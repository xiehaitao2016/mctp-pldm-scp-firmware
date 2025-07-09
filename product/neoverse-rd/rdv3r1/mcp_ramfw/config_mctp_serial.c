/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mcp_cfgd_timer.h"

#include <mod_mctp.h>
#include <mod_mctp_serial.h>

#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static mctp_serial_elem_config_t mctp_config;

static struct fwk_element mctp_serial_elem_table[] = {
    [MCTP_SERIAL_BIND_MCTP_API_IDX] = {
        .name = "mctp",
    },
    [MCTP_SERIAL_BIND_API_IDX_COUNT] = { 0 },
};

static const struct fwk_element *mctp_serial_get_elem_table(fwk_id_t module_id)
{
    mctp_config.driver_id = FWK_ID_MODULE(FWK_MODULE_IDX_MCTP);
    mctp_config.driver_api_id.api.type = __FWK_ID_TYPE_API;
    mctp_config.driver_api_id.api.module_idx = FWK_MODULE_IDX_MCTP;
    mctp_config.driver_api_id.api.api_idx = MCTP_BIND_REQ_API_IDX;
    mctp_serial_elem_table[MCTP_SERIAL_BIND_MCTP_API_IDX].data = &mctp_config;

    return (const struct fwk_element *)mctp_serial_elem_table;
}

/*
 * Data for the mctp serial module configuration
 */
static const struct mod_mctp_serial_module_config mctp_serial_data = {
    .alarm_id =
        FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, MCP_ALARM_ELEMENT_IDX, MCP_CFGD_MCTP_ALARM_IDX),
    .poll_period = 1
};

const struct fwk_module_config config_mctp_serial = {
    .data = &mctp_serial_data,
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(mctp_serial_get_elem_table),
};
