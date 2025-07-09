/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_mctp.h>
#include <mod_mctp_fw.h>
#include <mod_mctp_serial.h>
#include <mod_mctp_pcc.h>
#include <mod_pldm_fw.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

mctp_fw_elem_config_t mctp_serial_config;
mctp_fw_elem_config_t mctp_pcc_config;

static struct fwk_element mctp_fw_elem_table[] = {
#ifdef BUILD_HAS_MOD_MCTP_SERIAL
    [MCTP_FW_BIND_SERIAL_API_IDX] = {
        .name = "mctp_serial",
    },
#endif
#ifdef BUILD_HAS_MOD_MCTP_PCC
    [MCTP_FW_BIND_PCC_API_IDX] = {
        .name = "mctp_pcc",
    },
#endif
    [MCTP_FW_BIND_API_IDX_COUNT] = { 0 },
};

static const struct fwk_element *mctp_fw_get_elem_table(fwk_id_t module_id)
{
#ifdef BUILD_HAS_MOD_MCTP_SERIAL
    mctp_serial_config.driver_id = FWK_ID_MODULE(FWK_MODULE_IDX_MCTP_SERIAL);
    mctp_serial_config.driver_api_id.api.type = __FWK_ID_TYPE_API;
    mctp_serial_config.driver_api_id.api.module_idx =
        FWK_MODULE_IDX_MCTP_SERIAL;
    mctp_serial_config.driver_api_id.api.api_idx = MCTP_SERIAL_BIND_REQ_API_IDX;
    mctp_serial_config.local_eid = 18;
    mctp_serial_config.remote_eid = 8;
    mctp_serial_config.bus_policy = 0; // mctp endpoint
    mctp_fw_elem_table[MCTP_FW_BIND_SERIAL_API_IDX].data = &mctp_serial_config;
#endif

#ifdef BUILD_HAS_MOD_MCTP_PCC
    mctp_pcc_config.driver_id = FWK_ID_MODULE(FWK_MODULE_IDX_MCTP_PCC);
    mctp_pcc_config.driver_api_id.api.type = __FWK_ID_TYPE_API;
    mctp_pcc_config.driver_api_id.api.module_idx =
        FWK_MODULE_IDX_MCTP_PCC;
    mctp_pcc_config.driver_api_id.api.api_idx = MCTP_PCC_BIND_REQ_SERVICE_API_IDX;
    mctp_pcc_config.local_eid = 9;
    mctp_pcc_config.remote_eid = 8;
    mctp_pcc_config.bus_policy = 1; // mctp bus owner
    mctp_fw_elem_table[MCTP_FW_BIND_PCC_API_IDX].data = &mctp_pcc_config;
#endif

    return (const struct fwk_element *)mctp_fw_elem_table;
}

const struct fwk_module_config config_mctp_fw = {
    .data = &(mod_mctp_fw_config_t){
        .mctp_driver_api = {
            .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MCTP),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MCTP, MCTP_BIND_REQ_API_IDX),
        },
        .pldm_fw_driver_api = {
            .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_PLDM_FW),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PLDM_FW, PLDM_FW_BIND_REQ_API_IDX),
        },
    },
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(mctp_fw_get_elem_table),
};
