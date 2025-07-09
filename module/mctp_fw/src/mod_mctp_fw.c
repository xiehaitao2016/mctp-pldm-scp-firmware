/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
 */

#include <mctp_fw_msg.h>

#include <mod_mctp.h>
#include <mod_mctp_fw_int.h>

#include <fwk_assert.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_log.h>

mctp_fw_ctx_t mctp_fw_ctx;

/* sends packet to transport layer from app layer */
static int process_mctp_fw_tx(
      uint32_t bus, bool tag_owner, uint8_t msg_tag,
      void *msg, size_t len, mctp_eid_t dst)
{
    mctp_api_t *mctp_api = mctp_fw_ctx.mctp_api;
    mctp_t *mctp = NULL;

    switch(bus) {
#ifdef BUILD_HAS_MOD_MCTP_SERIAL
      case MCTP_FW_BIND_SERIAL_API_IDX:
        mctp =
          mctp_fw_ctx.elem_ctx_table[MCTP_FW_BIND_SERIAL_API_IDX].mctp_fw_elem.serial_elem.mctp;
        break;
#endif
#ifdef BUILD_HAS_MOD_MCTP_PCC
      case MCTP_FW_BIND_PCC_API_IDX:
        mctp =
          mctp_fw_ctx.elem_ctx_table[MCTP_FW_BIND_PCC_API_IDX].mctp_fw_elem.pcc_elem.mctp;
        break;
#endif
      default:
        FWK_LOG_ERR(MOD_NAME "Unknown MCTP bus - %ld ", bus);
        break;
    }

    if (mctp == NULL) FWK_LOG_ERR(MOD_NAME "No mctp ... ");

    return mctp_api->mctp_message_tx(mctp, dst, tag_owner, msg_tag, msg, len);
}

/* data from application layer moves via this api */
static void mctp_fw_receive_from_app_layer(
      uint32_t bus, bool tag_owner, uint8_t msg_tag,
      void *data, void *msg, size_t len)
{
    size_t buffer_size = len + 1;
    uint8_t *buffer = fwk_mm_calloc(buffer_size, sizeof(uint8_t));
    uint8_t *type = data;

    // Data -> MCTP_MSG_TYPE
    buffer[0] = *type;
    memcpy(&buffer[1], msg, len);

    if (bus >= MCTP_FW_BIND_API_IDX_COUNT) {
      fwk_assert("Unknow MCTP Bus medium.");
    }
    int rc = process_mctp_fw_tx(
                bus, tag_owner, msg_tag, buffer, buffer_size,
                mctp_fw_ctx.elem_ctx_table[bus].remote_eid);

    if (rc != 0) {
        assert(false);
    }

    fwk_mm_free(buffer);
}

/*
 * mctp api defintion. Used by application layer (pldm) in our case to receieve
 * data from mctp layer.
 */
static const mctp_fw_api_t mctp_fw_api = {
    .mctp_fw_receive_from_app_layer = mctp_fw_receive_from_app_layer,
};

/* sends packet from transport layer to app layer */
static void mctp_fw_receive_from_mctp(
    uint8_t eid,
    bool tag_owner,
    uint8_t msg_tag,
    void *data,
    void *msg,
    size_t len)
{
    pldm_fw_api_t *pldm_fw_api;
    uint8_t *buffer = (uint8_t *)msg;
    uint32_t *bus = (uint32_t *) data;
    struct mctp_ctrl_msg_hdr_t *msg_hdr = msg;

    // Handle MCTP Request Only
    if (msg_hdr->ic_msg_type == MCTP_CTRL_HDR_MSG_TYPE &&
        msg_hdr->rq_dgram_inst & MCTP_CTRL_HDR_FLAG_REQUEST) {
        int rc = -1;

        rc = mctp_ctrl_cmd_handle(*bus, mctp_fw_ctx.txbuf, eid, msg, len);
        if (rc > 0) {
            process_mctp_fw_tx(*bus, 0, 0, (void *)mctp_fw_ctx.txbuf, rc, eid);
        }
        return;
    }

    // Handle PLDM
    pldm_fw_api = mctp_fw_ctx.pldm_fw_api;

    // Remove Msg Type
    pldm_fw_api->pldm_fw_receive_from_transport_layer(
        data, (void *)&buffer[1], len - 1);
}

/* mctp_fw apis are exposed via bind requests*/
static int mctp_fw_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MCTP_FW_BIND_REQ_API_IDX:
        *api = &mctp_fw_api;
        break;

    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

/* mctp_fw binds with pldm_fw, mctp and mctp_serial modules  */
static int mctp_fw_bind(fwk_id_t id, unsigned int round)
{
    int status;
    mctp_fw_elem_ctx_t *elem_ctx;

    if (round == 0) {
        if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
            status = fwk_module_bind(
                mctp_fw_ctx.mctp_api_config->driver_id,
                mctp_fw_ctx.mctp_api_config->driver_api_id,
                &mctp_fw_ctx.mctp_api);
            if (status != FWK_SUCCESS) {
                return status;
            }
            FWK_LOG_INFO(MOD_NAME "MCTP API Binding Successfully");

            status = fwk_module_bind(
                mctp_fw_ctx.pldm_fw_api_config->driver_id,
                mctp_fw_ctx.pldm_fw_api_config->driver_api_id,
                &mctp_fw_ctx.pldm_fw_api);
            if (status != FWK_SUCCESS) {
                return status;
            }
            FWK_LOG_INFO(MOD_NAME "PLDM FW API Binding Successfully");
        }

        if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
            elem_ctx = &mctp_fw_ctx.elem_ctx_table[fwk_id_get_element_idx(id)];

            switch (fwk_id_get_element_idx(id)) {
#ifdef BUILD_HAS_MOD_MCTP_SERIAL
            case MCTP_FW_BIND_SERIAL_API_IDX:
                status = fwk_module_bind(
                    elem_ctx->mctp_fw_config->driver_id,
                    elem_ctx->mctp_fw_config->driver_api_id,
                    &elem_ctx->mctp_fw_elem.serial_elem.serial_api);
                FWK_LOG_INFO(MOD_NAME "MCTP Serial API Binding Successfully");
                break;
#endif
#ifdef BUILD_HAS_MOD_MCTP_PCC
            case MCTP_FW_BIND_PCC_API_IDX:
                status = fwk_module_bind(
                    elem_ctx->mctp_fw_config->driver_id,
                    elem_ctx->mctp_fw_config->driver_api_id,
                    &elem_ctx->mctp_fw_elem.pcc_elem.pcc_api);
                FWK_LOG_INFO(MOD_NAME "MCTP Pcc API Binding Successfully");
                break;
#endif
            default:
                status = FWK_E_PARAM;
                break;
            }

            if (status != FWK_SUCCESS) {
                return FWK_E_HANDLER;
            }
        }
    }
    return FWK_SUCCESS;
}

/* init for mctp_fw */
static int mctp_fw_init(
    fwk_id_t module_id,
    unsigned int elem_count,
    const void *data)
{
    mod_mctp_fw_config_t *config = (mod_mctp_fw_config_t *) data;

    mctp_fw_ctx.elem_ctx_table =
        fwk_mm_calloc(elem_count, sizeof(mctp_fw_ctx.elem_ctx_table[0]));

    if (mctp_fw_ctx.elem_ctx_table == NULL) {
        return FWK_E_NOMEM;
    }

    mctp_fw_ctx.elem_count = elem_count;

    mctp_fw_ctx.mctp_api_config = &config->mctp_driver_api;
    mctp_fw_ctx.pldm_fw_api_config = &config->pldm_fw_driver_api;

    return FWK_SUCCESS;
}

/* element init for mctp_fw */
static int mctp_fw_elem_init(
    fwk_id_t elem_id,
    unsigned int unused,
    const void *data)
{
    mctp_fw_elem_ctx_t *mctp_fw_elem_ctx;

    mctp_fw_elem_ctx =
        &mctp_fw_ctx.elem_ctx_table[fwk_id_get_element_idx(elem_id)];
    mctp_fw_elem_ctx->mctp_fw_config = (mctp_fw_elem_config_t *)data;
    mctp_fw_elem_ctx->id = elem_id;

    mctp_fw_elem_ctx->local_eid = mctp_fw_elem_ctx->mctp_fw_config->local_eid;
    mctp_fw_elem_ctx->remote_eid = mctp_fw_elem_ctx->mctp_fw_config->remote_eid;
    mctp_fw_elem_ctx->bus_policy = mctp_fw_elem_ctx->mctp_fw_config->bus_policy;

    return FWK_SUCCESS;
}

/*
 * mctp_fw takes care of initializing mctp object, enabling mctp tx and
 * registering mctp rx and chainin mctp to pldm_fw.
 */
static int mctp_fw_start(fwk_id_t id)
{
    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE) &&
        fwk_id_get_module_idx(id) == FWK_MODULE_IDX_MCTP_FW) {
        mctp_t *mctp = NULL;
        mctp_api_t *mctp_api = NULL;
#ifdef BUILD_HAS_MOD_MCTP_PCC
        mctp_fw_elem_ctx_t *ctx_mctp_pcc;
        mctp_pcc_api_t *mctp_pcc_api = NULL;
        mctp_binding_pcc_t *pcc = NULL;
#endif
        mctp_fw_elem_ctx_t *ctx_mctp_serial;
        mctp_serial_api_t *mctp_serial_api = NULL;
        mctp_binding_serial_t *serial = NULL;

        mctp_api = mctp_fw_ctx.mctp_api;

#ifdef BUILD_HAS_MOD_MCTP_SERIAL
        // Serial Binding
        ctx_mctp_serial = &mctp_fw_ctx.elem_ctx_table[MCTP_FW_BIND_SERIAL_API_IDX];
        if (ctx_mctp_serial) {
          mctp_serial_api = ctx_mctp_serial->mctp_fw_elem.serial_elem.serial_api;

          mctp = mctp_api->mctp_ctx_init();
          if (mctp == NULL) {
              return FWK_E_PARAM;
          }
          ctx_mctp_serial->mctp_fw_elem.serial_elem.mctp = mctp;

          serial = mctp_serial_api->mctp_serial_register_tx();
          if (serial == NULL) {
            return FWK_E_PARAM;
          }
          ctx_mctp_serial->mctp_fw_elem.serial_elem.serial_mctp_binding = serial;

          mctp_api->mctp_register_bus(
              mctp,
              mctp_api->mctp_binding_serial_core(serial),
              ctx_mctp_serial->local_eid);

          ctx_mctp_serial->bus = MCTP_FW_BIND_SERIAL_API_IDX;
          mctp_api->mctp_set_rx_all(mctp, mctp_fw_receive_from_mctp, &ctx_mctp_serial->bus);

          mctp_api->mctp_binding_set_tx_enabled(
              mctp_api->mctp_binding_serial_core(serial), true);
        }
#endif

#ifdef BUILD_HAS_MOD_MCTP_PCC
        // PCC Binding
        ctx_mctp_pcc =
            &mctp_fw_ctx.elem_ctx_table[MCTP_FW_BIND_PCC_API_IDX];
        if (ctx_mctp_pcc) {
          mctp_pcc_api = ctx_mctp_pcc->mctp_fw_elem.pcc_elem.pcc_api;

          mctp = mctp_api->mctp_ctx_init();
          if (mctp == NULL) {
              return FWK_E_PARAM;
          }
          ctx_mctp_pcc->mctp_fw_elem.pcc_elem.mctp = mctp;

          pcc = mctp_pcc_api->mctp_pcc_register_tx();
          if (pcc == NULL) {
              return FWK_E_PARAM;
          }
          ctx_mctp_pcc->mctp_fw_elem.pcc_elem.pcc_mctp_binding = pcc;

          mctp_api->mctp_register_bus(
              mctp,
              mctp_api->mctp_binding_pcc_core(pcc),
              ctx_mctp_pcc->local_eid);

          ctx_mctp_pcc->bus = MCTP_FW_BIND_PCC_API_IDX;
          mctp_api->mctp_set_rx_all(mctp, mctp_fw_receive_from_mctp, &ctx_mctp_pcc->bus);

          mctp_api->mctp_binding_set_tx_enabled(
              mctp_api->mctp_binding_pcc_core(pcc), true);
        }
#endif
    }

    return FWK_SUCCESS;
}

/* module definition */
const struct fwk_module module_mctp_fw = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = MCTP_FW_BIND_REQ_API_IDX_COUNT,
    .init = mctp_fw_init,
    .element_init = mctp_fw_elem_init,
    .bind = mctp_fw_bind,
    .start = mctp_fw_start,
    .process_bind_request = mctp_fw_process_bind_request,
};
