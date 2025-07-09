/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
 */

#include <mod_mctp.h>

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stdio.h>

/* Dummy function to make libmctp compile success */
size_t write(int fd, void *buf, size_t len)
{
    return -1;
}

/* internal-only allocation functions */
static void *mm_alloc(size_t size)
{
    void *ptr = malloc(size);

    if (ptr == NULL) {
        fwk_trap();
    }

    return ptr;
}

static void *mm_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

static void debug_print(int level, const char *fmt, va_list args)
{
#if FWK_LOG_LEVEL < FWK_LOG_LEVEL_INFO
    char buffer[FMW_LOG_COLUMNS];

    vsnprintf(buffer, sizeof(buffer), fmt, args);

    fwk_log_printf("[libmctp] %s", buffer);
#endif
}

/* mctp function pointer which gets passed during bind request*/
static const mctp_api_t mod_mctp_api = {
    /* mctp base apis */
    .mctp_ctx_init = mctp_init,
    .mctp_set_rx_all = mctp_set_rx_all,
    .mctp_register_bus = mctp_register_bus,
    .mctp_unregister_bus = mctp_unregister_bus,
    .mctp_bus_rx = mctp_bus_rx,
    .mctp_binding_set_tx_enabled = mctp_binding_set_tx_enabled,
    .mctp_message_tx = mctp_message_tx,

    /* mctp binding serial apis */
    .mctp_serial_init = mctp_serial_init,
    .mctp_serial_destroy = mctp_serial_destroy,
    .mctp_binding_serial_core = mctp_binding_serial_core,
    .mctp_serial_set_tx_fn = mctp_serial_set_tx_fn,
    .mctp_serial_rx = mctp_serial_rx,

    ///* mctp binding pcc apis */
    .mctp_pcc_init = mctp_pcc_init,
    .mctp_pcc_destroy = mctp_pcc_destroy,
    .mctp_binding_pcc_core = mctp_binding_pcc_core,
    .mctp_pcc_set_tx_fn = mctp_pcc_set_tx_fn,
    .mctp_pcc_rx = mctp_pcc_rx,
};

/* init */
static int mod_mctp_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    /* Module does not support elements */
    if (element_count > 0)
        return FWK_E_DATA;

    /* set dynamic memory management api's*/
    mctp_set_alloc_ops(mm_alloc, fwk_mm_free, mm_realloc);

    /* set debug log */
    mctp_set_log_custom(debug_print);

    return FWK_SUCCESS;
}

/* bind request */
static int mctp_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MCTP_BIND_REQ_API_IDX:
        *api = &mod_mctp_api;
        break;

    default:
        return FWK_E_PARAM;
    }
    return FWK_SUCCESS;
}

/* module descriptor */
const struct fwk_module module_mctp = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = MCTP_BIND_REQ_API_IDX_COUNT,
    .init = mod_mctp_init,
    .process_bind_request = mctp_process_bind_request,
};
