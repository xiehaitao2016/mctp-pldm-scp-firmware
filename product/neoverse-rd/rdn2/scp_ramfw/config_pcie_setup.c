/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "config_cmn700.h"
#include "platform_def.h"
#include "platform_sds.h"
#include "scp_css_mmap.h"
#include "scp_software_mmap.h"

#include <mod_apremap.h>
#include <mod_cmn700.h>
#include <mod_pcie_integ_ctrl.h>
#include <mod_pcie_setup.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#define ECAM_BASE_ADDRESS              (0x1010000000ULL)
#define MMIOH_BASE_ADDRESS             (0x4000000000ULL)
#define CHIP_ADDRESS_SPACE_SIZE        (UINT64_C(64) * FWK_TIB)
#define CHIP_ADDRESS_SPACE_OFFSET(idx) (idx * CHIP_ADDRESS_SPACE_SIZE)
/* PCIe ECAM and MMIO addresses */
#define CHIP_PCIE_ECAM_SIZE      (0x40000000UL)
#define CHIP_PCIE_MMIOH_SIZE     (0x4000000000ULL)
#define CHIP_PCIE_BUS_SIZE       256
#define CHIP_PCIE_MMIOL_SIZE     (0x8000000UL)
#define CHIP0_PCIE_MMIOL_BASE    (0x60000000UL)
#define CHIP1_PCIE_MMIOL_BASE    (CHIP0_PCIE_MMIOL_BASE + CHIP_PCIE_MMIOL_SIZE)
#define CHIP2_PCIE_MMIOL_BASE    (CHIP1_PCIE_MMIOL_BASE + CHIP_PCIE_MMIOL_SIZE)
#define CHIP3_PCIE_MMIOL_BASE    (CHIP2_PCIE_MMIOL_BASE + CHIP_PCIE_MMIOL_SIZE)
#define NUMBER_OF_ENDPOINTS      4
#define NUMBER_OF_CHIPS          4
#define SMMU_REGISTER_BLOCK_BASE 0x40000000
#define SMMU_REGISTER_BLOCK_SIZE 0x2000000
#define SMMU_REGISTER_BLOCK_ADDRESS(idx) \
    (SMMU_REGISTER_BLOCK_BASE + (idx * SMMU_REGISTER_BLOCK_SIZE))
#define CHIP_MEMMAP(idx) \
    { \
        .translation = CHIP_ADDRESS_SPACE_SIZE * idx, \
        .mmap = { \
            .ecam = { \
                .start = ECAM_BASE_ADDRESS, \
                .size = CHIP_PCIE_ECAM_SIZE, \
            }, \
            .mmiol = { \
                .start = CHIP##idx##_PCIE_MMIOL_BASE, \
                .size = CHIP_PCIE_MMIOL_SIZE, \
            }, \
            .mmioh = { \
                .start = MMIOH_BASE_ADDRESS, \
                .size = CHIP_PCIE_MMIOH_SIZE \
            }, \
            .bus = { \
                .start = 0, \
                .size = CHIP_PCIE_BUS_SIZE \
            } \
        }, \
        .cmn_id = FWK_ID_MODULE(FWK_MODULE_IDX_CMN700), \
        .io_block_id = FWK_ID_MODULE(FWK_MODULE_IDX_PCIE_INTEG_CTRL), \
        .cmn_api_id = FWK_ID_API(FWK_MODULE_IDX_CMN700, \
                                 MOD_CMN700_API_IDX_MAP_IO_REGION), \
        .io_block_api_id = FWK_ID_API(FWK_MODULE_IDX_PCIE_INTEG_CTRL, \
                                      MOD_PCIE_INTEG_CTRL_API_IDX_MAP_REGION), \
        .address_remapper_id = FWK_ID_MODULE(FWK_MODULE_IDX_APREMAP), \
        .address_remapper_api_id = FWK_ID_API(FWK_MODULE_IDX_APREMAP, \
                                          MOD_APREMAP_API_IDX_AP_MEM_RW),\
        .ep_interrupt_ids = ((uint64_t [NUMBER_OF_ENDPOINTS]) { \
                0, \
                0x10000, \
                0x20000, \
                0x30000, \
                }), \
        .ep_count = NUMBER_OF_ENDPOINTS \
    }

#define IO_MACRO_EP_CONFIG(x40_en, x41_en, x8_en, x16_en) \
    ((struct mod_pcie_setup_ep_config[4]){ \
        { .valid = x40_en, \
          .allow_ns_access = true, \
          .rp_node_id = MOD_PCIE_INTEG_CTRL_NODE_X4_0 }, \
        { .valid = x41_en, \
          .allow_ns_access = true, \
          .rp_node_id = MOD_PCIE_INTEG_CTRL_NODE_X4_1 }, \
        { .valid = x8_en, \
          .allow_ns_access = true, \
          .rp_node_id = MOD_PCIE_INTEG_CTRL_NODE_X8 }, \
        { .valid = x16_en, \
          .allow_ns_access = true, \
          .rp_node_id = MOD_PCIE_INTEG_CTRL_NODE_X16 }, \
    })

#define IO_MACRO_PCIE_ELEMENT_CONFIG(idx, x40_en, x41_en, x8_en, x16_en) \
    { \
        .name = "IO Macro " #idx, \
        .data = &((struct mod_pcie_setup_config){ \
            .reg_base = PCIE_INTEG_CTRL_REG_BASE(idx), \
            .cmn_node_id = NODE_ID_HNP##idx, \
            .block_id = idx, \
            .smmu_base = SMMU_REGISTER_BLOCK_ADDRESS(idx), \
            .sds_struct_id = PLATFORM_SDS_PCIE_MMAP, \
            .ep_config = IO_MACRO_EP_CONFIG(x40_en, x41_en, x8_en, x16_en) }) \
    }

#if (PLATFORM_VARIANT == 0 || PLATFORM_VARIANT == 3)
#    define NON_PCIE_IO_MACRO_REG_BASE PCIE_INTEG_CTRL_REG_BASE(4)
#elif (PLATFORM_VARIANT == 1) || \
    (PLATFORM_VARIANT == 2) /* RD-N2 Cfg1 and RD-N2-Cfg2 */
#    define NON_PCIE_IO_MACRO_REG_BASE PCIE_INTEG_CTRL_REG_BASE(1)
#endif

static const struct fwk_element pcie_setup_element_table[] = {
#if (PLATFORM_VARIANT == 1)
    IO_MACRO_PCIE_ELEMENT_CONFIG(0, true, true, true, true),
#else
    IO_MACRO_PCIE_ELEMENT_CONFIG(0, false, false, false, true),
#endif

#if (PLATFORM_VARIANT == 0 || PLATFORM_VARIANT == 3)
    IO_MACRO_PCIE_ELEMENT_CONFIG(1, false, false, false, true),

    IO_MACRO_PCIE_ELEMENT_CONFIG(2, false, false, false, true),

    IO_MACRO_PCIE_ELEMENT_CONFIG(3, false, false, false, true),
#endif
    { 0 }
};

static const struct mod_pcie_setup_module_config module_config = {
    .resource_info_count = NUMBER_OF_CHIPS,
    .resource_info = ((struct mod_pcie_setup_resource_info[NUMBER_OF_CHIPS]) {
        CHIP_MEMMAP(0),
#if (PLATFORM_VARIANT == 2)
            CHIP_MEMMAP(1), CHIP_MEMMAP(2), CHIP_MEMMAP(3),
#endif
    }),
};

static const struct fwk_element *pcie_setup_get_element_table(
    fwk_id_t module_id)
{
    return pcie_setup_element_table;
}

struct fwk_module_config config_pcie_setup = {
    .data = &module_config,
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(pcie_setup_get_element_table),
};
