/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

//#include <libmctp.h>

#include <mod_mctp.h>
#include <mod_mctp_serial.h>
#include <mod_timer.h>

#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>

#define MOD_NAME "[MCTP_SERIAL]: "

/* mctp elem type - part of mctp_serial elem ctx*/
typedef struct mctp_elem_ctx {
    mctp_api_t *mctp_api;
} mctp_elem_ctx_t;

/* mctp_serial elem ctx used for binding to other modules*/
typedef struct mctp_serial_elem_ctx {
    fwk_id_t id;
    mctp_serial_elem_config_t *mctp_serial_elem_config;
    fwk_id_t driver_id;

    union {
        mctp_elem_ctx_t mctp_elem;
    } mctp_serial_elem;

} mctp_serial_elem_ctx_t;

/* mctp_serial driver ctx */
typedef struct mctp_serial_ctx {
    mctp_serial_elem_ctx_t *elem_ctx_table;
    /* Number of channels */
    unsigned int elem_count;
} mctp_serial_ctx_t;

static mctp_serial_ctx_t mctp_serial_ctx;
static mctp_binding_serial_t *serial;
static struct mod_timer_alarm_api *alarm_api;
static struct fwk_io_stream *fwk_io_mctp;

static int mod_mctp_serial_tx_fn(void *data, void *buf, size_t len)
{
    size_t idx;
    char *ch = (char *)buf;

    FWK_LOG_DEBUG("[MCTP Serial] Sending packet ... ");
    for (idx = 0; idx < len; idx++) {
        FWK_LOG_DEBUG("Tx : 0x%02x", (char)ch[idx]);
        fwk_io_putch(fwk_io_mctp, ch[idx]);
    }

    return len;
}

static mctp_api_t *get_mctp_api()
{
    mctp_serial_elem_ctx_t *mctp_elem_ctx;

    mctp_elem_ctx =
        &mctp_serial_ctx.elem_ctx_table[MCTP_SERIAL_BIND_MCTP_API_IDX];

    return mctp_elem_ctx->mctp_serial_elem.mctp_elem.mctp_api;
}

/*
 * Protocol layers making use of mctp_serial (mctp in our case) should bind to
 * mctp_serial module to make use of the following function, exposed via
 * function pointer. This api internally sets mctp's binding fp's with
 * appropriate mctp_serial tx api
 */
mctp_binding_serial_t *mctp_serial_register_tx(void)
{
    return serial;
}

/* function pointers passed via bind request */
static mctp_serial_api_t mctp_serial_api = {
    .mctp_serial_register_tx = mctp_serial_register_tx,
};

/* initialize mctp_serial ctx */
static int mod_mctp_serial_init(
    fwk_id_t module_id,
    unsigned int elem_count,
    const void *config)
{
    mctp_serial_ctx.elem_ctx_table =
        fwk_mm_calloc(elem_count, sizeof(mctp_serial_ctx.elem_ctx_table[0]));

    if (mctp_serial_ctx.elem_ctx_table == NULL) {
        return FWK_E_NOMEM;
    }

    mctp_serial_ctx.elem_count = elem_count;

    /* Initialize mctp serial uart */
    static struct fwk_io_stream mctp_stream;
    bool configure_mctp =
        !fwk_id_is_equal(PL011_ELEMENT_ID_MCP_UART, FWK_ID_NONE);
    int status = FWK_SUCCESS;

    if (configure_mctp) {
        status = fwk_io_open(
            &mctp_stream,
            PL011_ELEMENT_ID_MCP_UART,
            (enum fwk_io_mode)(((unsigned int)FWK_IO_MODE_READ) | ((unsigned int)FWK_IO_MODE_WRITE) | ((unsigned int)FWK_IO_MODE_BINARY)));
        if (fwk_expect(status == FWK_SUCCESS)) {
            fwk_io_mctp = &mctp_stream;
        }
    }
    FWK_LOG_INFO("[MCTP Serial] Configured\n");

    if (status != FWK_SUCCESS) {
        return FWK_E_DEVICE;
    }

    return FWK_SUCCESS;
}

/* initialize mctp_serial element */
static int mctp_serial_elem_init(
    fwk_id_t elem_id,
    unsigned int unused,
    const void *data)
{
    mctp_serial_elem_ctx_t *mctp_serial_elem_ctx;

    mctp_serial_elem_ctx =
        &mctp_serial_ctx.elem_ctx_table[fwk_id_get_element_idx(elem_id)];
    mctp_serial_elem_ctx->mctp_serial_elem_config =
        (mctp_serial_elem_config_t *)data;
    mctp_serial_elem_ctx->id = elem_id;
    return FWK_SUCCESS;
}

/*
 * mctp_serial driver binds to MCTP layer. This is mainly to complete the rx
 * chain. Any event received from Tx is send back to MCTP via these apis
 */
static int mctp_serial_bind(fwk_id_t id, unsigned int round)
{
    int status;
    mctp_serial_elem_ctx_t *elem_ctx;
    const struct mod_mctp_serial_module_config *mod_config;

    if (round == 0) {
        if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
            elem_ctx =
                &mctp_serial_ctx.elem_ctx_table[fwk_id_get_element_idx(id)];

            switch (fwk_id_get_element_idx(id)) {
            case MCTP_SERIAL_BIND_MCTP_API_IDX:
                status = fwk_module_bind(
                    elem_ctx->mctp_serial_elem_config->driver_id,
                    elem_ctx->mctp_serial_elem_config->driver_api_id,
                    &elem_ctx->mctp_serial_elem.mctp_elem.mctp_api);
                break;

            default:
                status = FWK_E_PARAM;
                break;
            }

            if (status != FWK_SUCCESS) {
                return FWK_E_HANDLER;
            }
            elem_ctx->driver_id = elem_ctx->mctp_serial_elem_config->driver_id;
        }

        if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
            /* Retrieve the module config as specified by the platform config.c
             * file */
            mod_config = fwk_module_get_data(id);

            /* Bind to the specified alarm in order to poll the UART */
            status = fwk_module_bind(
                mod_config->alarm_id, MOD_TIMER_API_ID_ALARM, &alarm_api);
            if (status != FWK_SUCCESS) {
                return FWK_E_HANDLER;
            }
        }
    }
    return FWK_SUCCESS;
}

/* mctp_serial function pointers are passed via bind requests */
static int mctp_serial_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MCTP_SERIAL_BIND_REQ_API_IDX:
        *api = &mctp_serial_api;
        break;

    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static void alarm_callback(uintptr_t module_idx)
{
    int status;
    char ch = 0;
    mctp_api_t *mctp_api;

    /* Get the pending character (if any) from the UART without blocking */
    status = fwk_io_getch(fwk_io_mctp, &ch);

    if (status == FWK_SUCCESS) {
        mctp_api = get_mctp_api();
        mctp_api->mctp_serial_rx(serial, (const void *)&ch, 1);
    }
}

static int start_alarm(fwk_id_t id)
{
    const struct mod_mctp_serial_module_config *module_config;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return FWK_SUCCESS;
    }

    /* Retrieve the module config as specified by the platform config.c file */
    module_config = fwk_module_get_data(id);

    /* Start the UART polling alarm, recurring with the given time period */
    return alarm_api->start(
        module_config->alarm_id,
        module_config->poll_period,
        MOD_TIMER_ALARM_TYPE_PERIODIC,
        alarm_callback,
        fwk_id_get_module_idx(id));
}

static int mctp_serial_start(fwk_id_t id)
{
    mctp_api_t *mctp_api;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return FWK_SUCCESS;
    }

    /* Initialize mctp serial binding */
    mctp_api = get_mctp_api();

    serial = mctp_api->mctp_serial_init();
    fwk_assert(serial);

    mctp_api->mctp_serial_set_tx_fn(serial, mod_mctp_serial_tx_fn, 0);

    return start_alarm(id);
}

/* Module descriptor */
const struct fwk_module module_mctp_serial = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = MCTP_SERIAL_BIND_REQ_API_IDX_COUNT,
    .event_count = 1,
    .init = mod_mctp_serial_init,
    .element_init = mctp_serial_elem_init,
    .bind = mctp_serial_bind,
    .start = mctp_serial_start,
    .process_bind_request = mctp_serial_bind_request,
};
