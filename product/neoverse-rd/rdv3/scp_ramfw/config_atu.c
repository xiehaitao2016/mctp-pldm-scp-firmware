/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'atu'.
 */

#include "scp_css_mmap.h"

#include <mod_atu.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_module.h>

/* Indices for ATU module elements */
enum cfgd_mod_atu_element_idx {
    CFGD_MOD_ATU_EIDX_ATU0,
    CFGD_MOD_ATU_EIDX_COUNT
};

#define MOD_ATU_ELEMENT_COUNT (CFGD_MOD_ATU_EIDX_COUNT + 1)

struct sid_reg {
            uint8_t   RESERVED0[0x40 - 0];
    FWK_R   uint32_t  SYSTEM_ID;
            uint8_t   RESERVED1[0x50 - 0x44];
    FWK_R   uint32_t  SOC_ID;
            uint8_t   RESERVED2[0x60 - 0x54];
    FWK_R   uint32_t  NODE_ID;
            uint8_t   RESERVED3[0x70 - 0x64];
    FWK_R   uint32_t  SYSTEM_CFG;
            uint8_t   RESERVED4[0xFD0 - 0x74];
};

struct sid_reg *sid_reg = (struct sid_reg *)(0x2A4A0000);
#define SID_SYS_NODE_NUMBER_MASK UINT32_C(0xFF)

uint8_t chip_id = 0;

/* Indices for translation regions to be configured in the ATU */
enum atu_regions_idx {
    /* ATU region to access CMN CFGM */
    ATU_REGION_IDX_CMN = 0,
    /* ATU region to access AP Cluster Utility space */
    ATU_REGION_IDX_CLUSTER_UTIL,
    /* ATU region to access AP shared SRAM */
    ATU_REGION_IDX_SHARED_SRAM,
    /* ATU region to GPC SMMU register space */
    ATU_REGION_IDX_GPC_SMMU,
    /* ATU region to RSM SRAM region */
    ATU_REGION_IDX_RSM_SRAM,
    /* ATU region for Secure world shared SRAM */
    ATU_REGION_IDX_SHARED_SRAM_SECURE,
    /* ATU region for Non-secure world shared SRAM */
    ATU_REGION_IDX_SHARED_SRAM_NON_SECURE,
    /* ATU region for Realm world shared SRAM */
    ATU_REGION_IDX_SHARED_SRAM_REALM,
    /* ATU region for Non-secure world RSM SRAM */
    ATU_REGION_RSM_SCP_NON_SECURE,
    /* ATU region count */
    ATU_REGION_IDX_COUNT,
};

struct atu_region_map atu_regions[ATU_REGION_IDX_COUNT] = {
    [ATU_REGION_IDX_CMN] = {
        .region_owner_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCP_PLATFORM),
        .log_addr_base = SCP_ATW1_CMN_BASE,
        .phy_addr_base = 0x100000000,
        .region_size = SCP_ATW1_CMN_SIZE,
        .attributes = ATU_ENCODE_ATTRIBUTES_ROOT_PAS,
    },
    [ATU_REGION_IDX_CLUSTER_UTIL] = {
        .region_owner_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCP_PLATFORM),
        .log_addr_base = SCP_ATW0_LCP_AND_CLUSTER_UTILITY_BASE,
        .phy_addr_base = 0x200000000,
        .region_size = SCP_ATW0_LCP_AND_CLUSTER_UTILITY_SIZE,
        .attributes = ATU_ENCODE_ATTRIBUTES_ROOT_PAS,
    },
    [ATU_REGION_IDX_SHARED_SRAM] = {
        .region_owner_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCP_PLATFORM),
        .log_addr_base = SCP_ATW0_AP_PERIPHERAL_SRAM_BASE,
        .phy_addr_base = 0x00000000,
        .region_size = SCP_ATW0_AP_PERIPHERAL_SRAM_SIZE,
        .attributes = ATU_ENCODE_ATTRIBUTES_ROOT_PAS,
    },
    [ATU_REGION_IDX_GPC_SMMU] = {
        .region_owner_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCP_PLATFORM),
        .log_addr_base = SCP_ATW0_AP_PERIPHERAL_GPC_SMMU_BASE,
        .phy_addr_base = 0x300000000,
        .region_size = SCP_ATW0_AP_PERIPHERAL_GPC_SMMU_SIZE,
        .attributes = ATU_ENCODE_ATTRIBUTES_ROOT_PAS,
    },
    [ATU_REGION_IDX_RSM_SRAM] = {
        .region_owner_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCP_PLATFORM),
        .log_addr_base = SCP_ATW0_SHARED_SRAM_RSM_BASE,
        .phy_addr_base = 0x2F000000,
        .region_size = SCP_ATW0_SHARED_SRAM_RSM_SIZE,
        .attributes = ATU_ENCODE_ATTRIBUTES_SECURE_PAS,
    },
    [ATU_REGION_IDX_SHARED_SRAM_SECURE] = {
	.region_owner_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCP_PLATFORM),
	.log_addr_base = SCP_ATW0_SHARED_SRAM_SECURE_BASE,
	.phy_addr_base = 0x1A000,
	.region_size = SCP_ATW0_SHARED_SRAM_SECURE_SIZE,
	.attributes = ATU_ENCODE_ATTRIBUTES_SECURE_PAS,
    },
    [ATU_REGION_IDX_SHARED_SRAM_NON_SECURE] = {
        .region_owner_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCP_PLATFORM),
        .log_addr_base = SCP_ATW0_SHARED_SRAM_NON_SECURE_BASE,
        .phy_addr_base = 0x00000000,
        .region_size = SCP_ATW0_SHARED_SRAM_NON_SECURE_SIZE,
        .attributes = ATU_ENCODE_ATTRIBUTES_NON_SECURE_PAS,
    },
    [ATU_REGION_IDX_SHARED_SRAM_REALM] = {
        .region_owner_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCP_PLATFORM),
        .log_addr_base = SCP_ATW0_SHARED_SRAM_REALM_BASE,
        .phy_addr_base = 0x00000000,
        .region_size = SCP_ATW0_SHARED_SRAM_REALM_SIZE,
        .attributes = ATU_ENCODE_ATTRIBUTES_REALM_PAS,
    },
    [ATU_REGION_RSM_SCP_NON_SECURE] = {
        .region_owner_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCP_PLATFORM),
        .log_addr_base = SCP_ATW0_SHARED_SRAM_RSM_NS_BASE,
        .phy_addr_base = 0x002F000000,
        .region_size = SCP_ATW0_SHARED_SRAM_RSM_NS_SIZE,
        .attributes = ATU_ENCODE_ATTRIBUTES_NON_SECURE_PAS,
    },
};

static const struct fwk_element element_table[MOD_ATU_ELEMENT_COUNT] = {
    [CFGD_MOD_ATU_EIDX_ATU0] = {
        .name = "SCP_ATU",
        .data = &(struct mod_atu_device_config) {
            .is_atu_delegated = false,
            .atu_base = SCP_ATU_BASE,
            .atu_region_config_table = atu_regions,
            .atu_region_count = FWK_ARRAY_SIZE(atu_regions),
        },
    },
    [CFGD_MOD_ATU_EIDX_COUNT] = { 0 },
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    chip_id = sid_reg->NODE_ID & SID_SYS_NODE_NUMBER_MASK;
    FWK_LOG_INFO("[ATU] chip_id: %d", chip_id);
    for (unsigned int i = 0; i < FWK_ARRAY_SIZE(atu_regions); i++) {
        atu_regions[i].phy_addr_base += (chip_id * 64ULL * FWK_GIB);
    }
    return element_table;
}

struct fwk_module_config config_atu = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};
