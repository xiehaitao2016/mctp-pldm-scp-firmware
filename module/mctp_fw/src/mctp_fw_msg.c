/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
 */

#include <mctp_fw_msg.h>

#include <mod_mctp.h>
#include <mod_mctp_fw_int.h>

#include <fwk_log.h>

#include <stddef.h>

extern mctp_fw_ctx_t mctp_fw_ctx;

static int mctp_init_bus_register(uint32_t bus, mctp_eid_t eid)
{
    mctp_api_t *mctp_api = mctp_fw_ctx.mctp_api;
    mctp_t *mctp = NULL;
    mctp_binding_t *binding = NULL;

    switch(bus) {
      case MCTP_FW_BIND_SERIAL_API_IDX:
        mctp =
          mctp_fw_ctx.elem_ctx_table[bus].mctp_fw_elem.serial_elem.mctp;
        binding =
          mctp_api->mctp_binding_serial_core(
                mctp_fw_ctx.elem_ctx_table[bus].mctp_fw_elem.serial_elem.serial_mctp_binding);
        break;
#ifdef BUILD_HAS_MOD_MCTP_PCC
      case MCTP_FW_BIND_PCC_API_IDX:
        mctp =
          mctp_fw_ctx.elem_ctx_table[bus].mctp_fw_elem.pcc_elem.mctp;
        binding =
          mctp_api->mctp_binding_pcc_core(
                mctp_fw_ctx.elem_ctx_table[bus].mctp_fw_elem.pcc_elem.pcc_mctp_binding);
        break;
#endif
      default:
        FWK_LOG_ERR("Unknown MCTP bus - %ld ", bus);
        break;
    }

    if (mctp == NULL) {
      FWK_LOG_ERR("No mctp bus found ... ");
      return 0;
    }

    // Unregister Bus
    mctp_api->mctp_unregister_bus(mctp, binding);

    // Register Bus
    mctp_api->mctp_register_bus(mctp, binding, eid);

    return 0;
}

static int mctp_cmd_get_endpoint_uuid(
    void *buffer,
    size_t length,
    void *rspn_buf)
{
    struct mctp_ctrl_msg_hdr_t *rspn = rspn_buf;

    rspn->data[0] = MCTP_CTRL_CC_SUCCESS;
    for (int i = 1; i < MCTP_GET_ENDPOINT_UUID_RESP_BYTES; i++) {
        rspn->data[i] = i;
    }

    return MCTP_GET_ENDPOINT_UUID_RESP_BYTES;
}

static int mctp_cmd_get_endpoint_id(uint32_t bus, void *buffer, size_t length, void *rspn_buf)
{
    struct mctp_ctrl_msg_hdr_t *rspn = rspn_buf;

    rspn->data[0] = MCTP_CTRL_CC_SUCCESS;
    if (bus >= MCTP_FW_BIND_API_IDX_COUNT) {
      fwk_assert("Unknow MCTP Bus medium.");
    }
    rspn->data[1] = mctp_fw_ctx.elem_ctx_table[bus].local_eid;
    rspn->data[2] = 0;
    rspn->data[3] = 0;

    return MCTP_GET_ENDPOINT_ID_RESP_BYTES;
}

static int mctp_cmd_set_endpoint_id(uint32_t bus, void *buffer, size_t length, void *rspn_buf)
{
    struct mctp_ctrl_msg_hdr_t *req = buffer;
    struct mctp_ctrl_msg_hdr_t *rspn = rspn_buf;

    mctp_eid_t eid = req->data[1];
    if (eid == MCTP_EID_NULL || eid == MCTP_EID_BROADCAST) {
        rspn->data[0] = MCTP_CTRL_CC_ERROR_INVALID_DATA;
        if (bus >= MCTP_FW_BIND_API_IDX_COUNT) {
          fwk_assert("Unknow MCTP Bus medium.");
        }
        rspn->data[2] = mctp_fw_ctx.elem_ctx_table[bus].local_eid;
    } else {
        // Update EID and reinit Bus registration
        mctp_fw_ctx.elem_ctx_table[bus].local_eid = eid;
        mctp_init_bus_register(bus, eid);

        rspn->data[0] = MCTP_CTRL_CC_SUCCESS;
        rspn->data[2] = eid;
    }

    rspn->data[1] = 0;
    rspn->data[3] = 0;

    return MCTP_SET_ENDPOINT_ID_RESP_BYTES;
}

static int mctp_cmd_get_msg_type(void *buffer, size_t length, void *rspn_buf)
{
    struct mctp_ctrl_msg_hdr_t *rspn = rspn_buf;

    rspn->data[0] = MCTP_CTRL_CC_SUCCESS;
    rspn->data[1] = 2; // MCTP CTRL + PLDM
    rspn->data[2] = MCTP_CTRL_HDR_MSG_TYPE;
    rspn->data[3] = PLDM_MSG_TYPE;

    return MCTP_GET_MESSAGE_TYPE_MIN_RESP_BYTES + 2;
}

int mctp_ctrl_cmd_handle(
    uint32_t bus,
    uint8_t *tx_buf,
    mctp_eid_t src,
    void *buffer,
    size_t length)
{
    struct mctp_ctrl_msg_hdr_t *msg_hdr = buffer;
    struct mctp_ctrl_msg_hdr_t *rspn_hdr = (struct mctp_ctrl_msg_hdr_t *)tx_buf;
    int rc = -1;

    switch (msg_hdr->command_code) {
    case MCTP_CTRL_CMD_SET_ENDPOINT_ID:
        FWK_LOG_INFO(MOD_NAME "MCTP Set Endpoint ID");
        rc = mctp_cmd_set_endpoint_id(bus, buffer, length, tx_buf);

        // Only Bus Owner can assign EID
        if (bus >= MCTP_FW_BIND_API_IDX_COUNT) {
          fwk_assert("Unknow MCTP Bus medium.");
        }
        mctp_fw_ctx.elem_ctx_table[bus].remote_eid = src;
        break;
    case MCTP_CTRL_CMD_GET_ENDPOINT_ID:
        FWK_LOG_INFO(MOD_NAME "MCTP Get Endpoint ID");
        rc = mctp_cmd_get_endpoint_id(bus, buffer, length, tx_buf);
        break;
    case MCTP_CTRL_CMD_GET_ENDPOINT_UUID:
        FWK_LOG_INFO(MOD_NAME "MCTP Get Endpoint UUID");
        rc = mctp_cmd_get_endpoint_uuid(buffer, length, tx_buf);
        break;
    case MCTP_CTRL_CMD_GET_VERSION_SUPPORT:
        FWK_LOG_INFO(MOD_NAME "TODO: MCTP Get Version Support");
        break;
    case MCTP_CTRL_CMD_GET_MESSAGE_TYPE_SUPPORT:
        FWK_LOG_INFO(MOD_NAME "MCTP Get Message Type Support");
        rc = mctp_cmd_get_msg_type(buffer, length, tx_buf);
        break;
    case MCTP_CTRL_CMD_PREPARE_ENDPOINT_DISCOVERY:
        FWK_LOG_INFO(MOD_NAME "TODO: MCTP Prepare Endpoint Discovery");
        break;
    case MCTP_CTRL_CMD_ENDPOINT_DISCOVERY:
        FWK_LOG_INFO(MOD_NAME "TODO: MCTP Endpoint Discovery");
        break;
    default:
        FWK_LOG_INFO(MOD_NAME "MCTP 0x%02X Command Unsupported", msg_hdr->command_code);
        break;
    }

    if (rc > 0) {
        rspn_hdr->ic_msg_type = msg_hdr->ic_msg_type;
        rspn_hdr->command_code = msg_hdr->command_code;
        rspn_hdr->rq_dgram_inst =
            MCTP_CTRL_HDR_INSTANCE_ID_MASK & msg_hdr->rq_dgram_inst;
        rc += sizeof(struct mctp_ctrl_msg_hdr_t);
    }

    return rc;
}
