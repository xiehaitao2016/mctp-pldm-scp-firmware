/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_transport.h>
#include <mod_mctp_pcc.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct fwk_element service_table[] = {
    [MCTP_PCC_BIND_TRANSPORT_API_IDX] = {
        .name = "MCP_TRANSPORT",
        .data = &((struct mod_mctp_pcc_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TRANSPORT, 0),
            .driver_api_id =
                FWK_ID_API_INIT(FWK_MODULE_IDX_TRANSPORT, MOD_TRANSPORT_API_IDX_MCTP_PCC),
        }),
    },
    [MCTP_PCC_BIND_SERVICE_API_IDX] = {
        .name = "MCTP",
        .data = &((struct mod_mctp_pcc_config) {
            .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MCTP),
            .driver_api_id =
                FWK_ID_API_INIT(FWK_MODULE_IDX_MCTP, MCTP_BIND_REQ_API_IDX),
        }),
    },
    [MCTP_PCC_BIND_API_IDX_COUNT] = { 0 }
};

static const struct fwk_element *get_service_table(fwk_id_t module_id)
{
    return service_table;
}

const struct fwk_module_config config_mctp_pcc = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_service_table),
};
