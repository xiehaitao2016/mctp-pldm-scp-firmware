/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_mm.h>
#include <fwk_string.h>

#include <mod_transport.h>
#include <mod_mctp.h>

#include "mod_mctp_pcc.h"

#define MOD_NAME "[MCTP_PCC] "

struct mctp_pcc_ctx {
    struct mod_mctp_pcc_config *config;

    const struct mod_transport_mctp_pcc_api *transport_api;
    const mctp_api_t *mctp_api;

    fwk_id_t channel_id;
};

static struct mctp_pcc_ctx ctx;
static mctp_binding_pcc_t *pcc;

static int mod_mctp_pcc_tx_fn(void *data, void *buf, size_t len)
{
    uint32_t message_header = 0;
    int status;
    uint8_t tx_buf[PCC_TX_BUFFER_SIZE] = { 0 };
    struct mctp_pcc_pkt *pcc_pkt;
    uint32_t tx_buf_size = 0;


    pcc_pkt = (struct mctp_pcc_pkt *) tx_buf;
    pcc_pkt->signature = PCC_SUBSPACE_1;
    pcc_pkt->flags = 0;
    pcc_pkt->length = len + sizeof(pcc_pkt->command);
    pcc_pkt->command = PCC_MCTP_COMMAND;

    tx_buf_size = len + sizeof(struct mctp_pcc_pkt);

    if (tx_buf_size > sizeof(tx_buf)) {
        FWK_LOG_ERR(MOD_NAME "Tx Buffer size not enough");
        return FWK_E_NOMEM;
    }

    memcpy(pcc_pkt->mctp_payload, buf, len);

#if FWK_LOG_LEVEL < FWK_LOG_LEVEL_INFO
    size_t idx;
    uint8_t *c = (uint8_t *) tx_buf;

    for (idx = 0; idx < tx_buf_size; idx++) {
        FWK_LOG_INFO(MOD_NAME "TX : 0x%02x", *(c + idx));
    }
#endif

    status = ctx.transport_api->transmit(ctx.channel_id, message_header, tx_buf, tx_buf_size, false);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Send messagae failed.");
        return FWK_E_BUSY;
    }

    return FWK_SUCCESS;
}

static int mod_mctp_pcc_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *unused)
{
    /* No elements support */
    if (element_count == 0) {
        FWK_LOG_ERR(MOD_NAME " No element count.");
        fwk_unexpected();
        return FWK_E_PARAM;
    }

    ctx.config = fwk_mm_alloc(element_count, sizeof(struct mod_mctp_pcc_config));
    ctx.channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MCTP_PCC, MCTP_PCC_BIND_TRANSPORT_API_IDX);

    return FWK_SUCCESS;
}

static int mod_mctp_pcc_element_init (
    fwk_id_t element_id, unsigned int element_count, const void *data)
{
    const struct mod_mctp_pcc_config *config = data;
    struct mod_mctp_pcc_config *ctx_config;

    ctx_config = &ctx.config[fwk_id_get_element_idx(element_id)];

    ctx_config->driver_id = config->driver_id;
    ctx_config->driver_api_id = config->driver_api_id;

    return FWK_SUCCESS;
}

static int mod_mctp_pcc_bind(fwk_id_t id, unsigned int round)
{
    int status = FWK_SUCCESS;
    struct mod_mctp_pcc_config *config = NULL;

    if (round == 0) {
        if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
            return FWK_SUCCESS;
        }

        config = &ctx.config[fwk_id_get_element_idx(id)];

        switch (fwk_id_get_element_idx(id)) {
          case MCTP_PCC_BIND_TRANSPORT_API_IDX:
            status = fwk_module_bind(config->driver_id,
                                     config->driver_api_id, &ctx.transport_api);
            if (status != FWK_SUCCESS) {
                FWK_LOG_ERR(MOD_NAME "Transport APIs Binding Failed.");
                return status;
            }

            if ((ctx.transport_api->get_max_payload_size == NULL) ||
                (ctx.transport_api->get_payload == NULL) ||
                (ctx.transport_api->transmit == NULL) ||
                (ctx.transport_api->release_transport_channel_lock == NULL)) {
                FWK_LOG_ERR(MOD_NAME "Transport APIs Not Implemented.");
                return FWK_E_DATA;
            }

            break;
          case MCTP_PCC_BIND_SERVICE_API_IDX:
            status = fwk_module_bind(config->driver_id,
                                     config->driver_api_id, &ctx.mctp_api);
            if (status != FWK_SUCCESS) {
                FWK_LOG_ERR(MOD_NAME "MCTP API Binding Failed.");
                return status;
            }

            break;
          default:
            status = FWK_E_PARAM;
            break;
        }
    }

    return status;
}

static int mod_mctp_pcc_start(fwk_id_t id)
{
    if (!fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return FWK_SUCCESS;
    }

    pcc = ctx.mctp_api->mctp_pcc_init();
    fwk_assert(pcc);

    ctx.mctp_api->mctp_pcc_set_tx_fn(pcc, mod_mctp_pcc_tx_fn, 0);

    return FWK_SUCCESS;
}

static int mctp_pcc_signal_error(fwk_id_t channel_id)
{
    FWK_LOG_ERR(MOD_NAME "Signal error");
    return FWK_SUCCESS;
}

static int mctp_pcc_signal_message(fwk_id_t channel_id)
{
    uint8_t *msg;
    size_t size = 0;
    struct mctp_pcc_pkt *pcc_pkt;
    uint32_t pcc_subspace = PCC_SUBSPACE_0;
    uint32_t mctp_command = PCC_MCTP_COMMAND;
    uint32_t payload_length = 0;

    ctx.transport_api->get_payload(channel_id, (const void **) &msg, &size);

    pcc_pkt = (struct mctp_pcc_pkt *) msg;
    if (memcmp(&pcc_subspace, &pcc_pkt->signature, sizeof(pcc_pkt->signature)) != 0) {
        FWK_LOG_ERR(MOD_NAME "PCC Subspace Signature Error.");
        return FWK_SUCCESS;
    }

    // TODO: Check pcc flags
    (void) pcc_pkt->flags;

    // TODO: Check pcc payload length
    payload_length = pcc_pkt->length - 4; // Remove command size
    (void) payload_length;


    if (memcmp(&mctp_command, &pcc_pkt->command, sizeof(pcc_pkt->command)) == 0) {
#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
        uint32_t idx = 0;

        FWK_LOG_INFO(MOD_NAME "Receive a incoming request (%ld) - ", payload_length);
        for (idx = 0; idx < payload_length; idx++) {
            FWK_LOG_INFO(MOD_NAME "Rx : 0x%02x", pcc_pkt->mctp_payload[idx]);
        }
#endif
        ctx.mctp_api->mctp_pcc_rx(pcc, (const void *) pcc_pkt->mctp_payload, payload_length);
    } else {
        FWK_LOG_ERR(MOD_NAME "Unknown request type ... ");
    }

    // Done
    ctx.transport_api->release_transport_channel_lock(channel_id);

    return FWK_SUCCESS;
}

struct mod_transport_mctp_pcc_signal_api mctp_pcc_signal_api = {
    .signal_error = mctp_pcc_signal_error,
    .signal_message = mctp_pcc_signal_message,
};

mctp_binding_pcc_t *mctp_pcc_register_tx(void)
{
    return pcc;
}

static mctp_pcc_api_t mctp_pcc_api = {
    .mctp_pcc_register_tx = mctp_pcc_register_tx,
};

static int mod_mctp_pcc_bind_request(
    fwk_id_t source_id, fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
      case MCTP_PCC_BIND_REQ_TRANSPORT_API_IDX:
        *api = &mctp_pcc_signal_api;
        break;
      case MCTP_PCC_BIND_REQ_SERVICE_API_IDX:
        *api = &mctp_pcc_api;
        break;
      default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_mctp_pcc = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .init = mod_mctp_pcc_init,
    .element_init = mod_mctp_pcc_element_init,
    .bind = mod_mctp_pcc_bind,
    .start = mod_mctp_pcc_start,
    .api_count = MCTP_PCC_BIND_REQ_API_IDX_COUNT,
    .process_bind_request = mod_mctp_pcc_bind_request,
};
