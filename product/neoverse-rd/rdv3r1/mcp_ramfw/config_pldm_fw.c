/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_mctp_fw.h>
#include <mod_pldm.h>
#include <mod_pldm_fw.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

pldm_fw_elem_config_t base_data_config;
pldm_fw_elem_config_t mctp_fw_config;
pldm_fw_elem_config_t platform_config;
pldm_fw_elem_config_t pdr_config;
pldm_fw_elem_config_t fru_config;
pldm_fw_elem_config_t utils_config;

static struct fwk_element pldm_fw_elem_table[] = {
    [PLDM_FW_BIND_BASE_API_IDX] = {
    .name = "pldm_fw_base",
    },

    [PLDM_FW_BIND_MCTP_FW_API_IDX] = {
    .name = "pldm_fw_mctp_fw",
    },

    [PLDM_FW_BIND_PLATFORM_API_IDX] = {
    .name = "pldm_fw_platform",
    },

    [PLDM_FW_BIND_PDR_API_IDX] = {
    .name = "pldm_pdr_platform",
    },

    [PLDM_FW_BIND_FRU_API_IDX] = {
    .name = "pldm_fw_fru"
    },

    [PLDM_FW_BIND_UTILS_API_IDX] = {
    .name = "pldm_fw_utils"
    },

    [PLDM_FW_BIND_API_IDX_COUNT] = { 0 },

};

static const struct fwk_element *pldm_fw_get_elem_table(fwk_id_t module_id)

{
    base_data_config.driver_id = FWK_ID_MODULE(FWK_MODULE_IDX_PLDM);
    base_data_config.driver_api_id.api.type = __FWK_ID_TYPE_API;
    base_data_config.driver_api_id.api.module_idx = FWK_MODULE_IDX_PLDM;
    base_data_config.driver_api_id.api.api_idx = PLDM_BASE_BIND_REQ_API_IDX;
    pldm_fw_elem_table[PLDM_FW_BIND_BASE_API_IDX].data = &base_data_config;

    mctp_fw_config.driver_id = FWK_ID_MODULE(FWK_MODULE_IDX_MCTP_FW);
    mctp_fw_config.driver_api_id.api.type = __FWK_ID_TYPE_API;
    mctp_fw_config.driver_api_id.api.module_idx = FWK_MODULE_IDX_MCTP_FW;
    mctp_fw_config.driver_api_id.api.api_idx = MCTP_FW_BIND_REQ_API_IDX;
    pldm_fw_elem_table[PLDM_FW_BIND_MCTP_FW_API_IDX].data = &mctp_fw_config;

    platform_config.driver_id = FWK_ID_MODULE(FWK_MODULE_IDX_PLDM);
    platform_config.driver_api_id.api.type = __FWK_ID_TYPE_API;
    platform_config.driver_api_id.api.module_idx = FWK_MODULE_IDX_PLDM;
    platform_config.driver_api_id.api.api_idx = PLDM_PLATFORM_BIND_REQ_API_IDX;
    pldm_fw_elem_table[PLDM_FW_BIND_PLATFORM_API_IDX].data = &platform_config;

    pdr_config.driver_id = FWK_ID_MODULE(FWK_MODULE_IDX_PLDM);
    pdr_config.driver_api_id.api.type = __FWK_ID_TYPE_API;
    pdr_config.driver_api_id.api.module_idx = FWK_MODULE_IDX_PLDM;
    pdr_config.driver_api_id.api.api_idx = PLDM_PDR_BIND_REQ_API_IDX;
    pldm_fw_elem_table[PLDM_FW_BIND_PDR_API_IDX].data = &pdr_config;

    fru_config.driver_id = FWK_ID_MODULE(FWK_MODULE_IDX_PLDM);
    fru_config.driver_api_id.api.type = __FWK_ID_TYPE_API;
    fru_config.driver_api_id.api.module_idx = FWK_MODULE_IDX_PLDM;
    fru_config.driver_api_id.api.api_idx = PLDM_FRU_BIND_REQ_API_IDX;
    pldm_fw_elem_table[PLDM_FW_BIND_FRU_API_IDX].data = &fru_config;

    utils_config.driver_id = FWK_ID_MODULE(FWK_MODULE_IDX_PLDM);
    utils_config.driver_api_id.api.type = __FWK_ID_TYPE_API;
    utils_config.driver_api_id.api.module_idx = FWK_MODULE_IDX_PLDM;
    utils_config.driver_api_id.api.api_idx = PLDM_UTILS_BIND_REQ_API_IDX;
    pldm_fw_elem_table[PLDM_FW_BIND_UTILS_API_IDX].data = &utils_config;

    return (const struct fwk_element *)pldm_fw_elem_table;
}

struct fwk_module_config config_pldm_fw = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(pldm_fw_get_elem_table),

};
