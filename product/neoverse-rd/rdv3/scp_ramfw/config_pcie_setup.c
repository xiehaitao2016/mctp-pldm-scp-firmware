/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024444m Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "cmn_node_id.h"
#include "io_macro_layout.h"
#include "scp_cfgd_sds.h"
#include "scp_fw_mmap.h"

#include <mod_atu.h>
#include <mod_atu_mmio.h>
#include <mod_cmn_cyprus.h>
#include <mod_io_block.h>
#include <mod_pcie_setup.h>
#include <mod_scp_platform.h>

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

struct sid_reg {
    uint8_t RESERVED0[0x40 - 0];
    FWK_R uint32_t SYSTEM_ID;
    uint8_t RESERVED1[0x50 - 0x44];
    FWK_R uint32_t SOC_ID;
    uint8_t RESERVED2[0x60 - 0x54];
    FWK_R uint32_t NODE_ID;
    uint8_t RESERVED3[0x70 - 0x64];
    FWK_R uint32_t SYSTEM_CFG;
    uint8_t RESERVED4[0xFD0 - 0x74];
};

extern struct sid_reg *sid_reg;
#define SID_SYS_NODE_NUMBER_MASK UINT32_C(0xFF)

extern uint8_t chip_id;

/* PCIe ECAM and MMIO addresses */
#define ECAM_SIZE                   (UINT64_C(256) * FWK_MIB)
#define CHIP_ECAM_OFFSET(idx)       (idx * ECAM_SIZE)
#define CHIP_ECAM_BASE_ADDRESS(idx) (0x4000000000ULL + CHIP_ECAM_OFFSET(idx))

#define MMIOH_SIZE                   (UINT64_C(64) * FWK_GIB)
#define CHIP_MMIOH_OFFSET(idx)       (idx * MMIOH_SIZE)
#define CHIP_MMIOH_BASE_ADDRESS(idx) (0x4040000000ULL + CHIP_MMIOH_OFFSET(idx))
#define CHIP_PCIE_BUS_SIZE           256
#if (PLATFORM_VARIANT == 2)
#    define CHIP_PCIE_MMIOL_SIZE (0x8000000UL)
#else
#    define CHIP_PCIE_MMIOL_SIZE (0x20000000UL)
#endif
#define CHIP0_PCIE_MMIOL_BASE (0x60000000UL)
#define CHIP1_PCIE_MMIOL_BASE (CHIP0_PCIE_MMIOL_BASE + CHIP_PCIE_MMIOL_SIZE)
#define CHIP2_PCIE_MMIOL_BASE (CHIP1_PCIE_MMIOL_BASE + CHIP_PCIE_MMIOL_SIZE)
#define CHIP3_PCIE_MMIOL_BASE (CHIP2_PCIE_MMIOL_BASE + CHIP_PCIE_MMIOL_SIZE)
#define NUMBER_OF_ENDPOINTS   5
#define NUMBER_OF_CHIPS       4

#define CHIP_MEMMAP(idx) \
    { \
        .mmap = { \
            .ecam = { \
                .start = CHIP_ECAM_BASE_ADDRESS(idx), \
                .size = ECAM_SIZE, \
            }, \
            .mmiol = { \
                .start = CHIP##idx##_PCIE_MMIOL_BASE, \
                .size = CHIP_PCIE_MMIOL_SIZE, \
            }, \
            .mmioh = { \
                .start = CHIP_MMIOH_BASE_ADDRESS(idx), \
                .size = MMIOH_SIZE \
            }, \
            .bus = { \
                .start = 0, \
                .size = CHIP_PCIE_BUS_SIZE \
            } \
        }, \
        .cmn_id = FWK_ID_MODULE(FWK_MODULE_IDX_CMN_CYPRUS), \
        .io_block_id = FWK_ID_MODULE(FWK_MODULE_IDX_IO_BLOCK), \
        .cmn_api_id = FWK_ID_API(FWK_MODULE_IDX_CMN_CYPRUS, \
                                 MOD_CMN_CYPRUS_API_IDX_MAP_IO_REGION), \
        .io_block_api_id = FWK_ID_API(FWK_MODULE_IDX_IO_BLOCK, \
                                      MOD_IO_BLOCK_API_IDX_MAP_REGION), \
        .address_remapper_id = FWK_ID_MODULE(FWK_MODULE_IDX_ATU_MMIO), \
        .address_remapper_api_id = \
                 FWK_ID_API(FWK_MODULE_IDX_ATU_MMIO, \
                            MOD_ATU_MMIO_API_IDX_MEM_RW),\
        .plat_notification = { \
            .notification_id = FWK_ID_NOTIFICATION_INIT( \
                    FWK_MODULE_IDX_SCP_PLATFORM, \
                    MOD_SCP_PLATFORM_NOTIFICATION_IDX_SUBSYS_INITIALIZED), \
            .source_id = FWK_ID_MODULE_INIT( \
                    FWK_MODULE_IDX_SCP_PLATFORM), \
        }, \
        .ep_count = NUMBER_OF_ENDPOINTS, \
        .ep_interrupt_ids = ((uint64_t [NUMBER_OF_ENDPOINTS]) { \
                0, \
                0x10000, \
                0x20000, \
                0x30000, \
                0x40000, \
         }), \
    }

#define IO_MACRO_EP_CONFIG(x1_en, x20_en, x21_en, x4_en, x8_en) \
    ((struct mod_pcie_setup_ep_config[NUMBER_OF_ENDPOINTS]){ \
        { .valid = x1_en, \
          .allow_ns_access = true, \
          .rp_node_id = AMNI_PCIEX1_0 }, \
        { .valid = x20_en, \
          .allow_ns_access = true, \
          .rp_node_id = AMNI_PCIEX2_0 }, \
        { .valid = x21_en, \
          .allow_ns_access = true, \
          .rp_node_id = AMNI_PCIEX2_1 }, \
        { .valid = x4_en, \
          .allow_ns_access = true, \
          .rp_node_id = AMNI_PCIEX4_0 }, \
        { .valid = x8_en, \
          .allow_ns_access = true, \
          .rp_node_id = AMNI_PCIEX8_0 }, \
    })

#define IO_MACRO_PCIE_ELEMENT_CONFIG(idx, x1_en, x20_en, x21_en, x4_en, x8_en) \
    { \
        .name = "IO Macro " #idx, \
        .data = &((struct mod_pcie_setup_config){ \
            .reg_base = NCI_GVP_BLOCK_BASE(idx), \
            .cmn_node_id = IOVB_NODE_ID##idx, \
            .nci_source_node_id = ASNI_CMN, \
            .block_id = idx, \
            .smmu_base = TCU_REG_BASE(idx), \
            .sds_struct_id = SDS_PCIE_MMAP, \
            .ep_config = \
                IO_MACRO_EP_CONFIG(x1_en, x20_en, x21_en, x4_en, x8_en) }) \
    }

static const struct fwk_element pcie_setup_element_table[] = {
#if (PLATFORM_VARIANT == 1)
    IO_MACRO_PCIE_ELEMENT_CONFIG(0, true, true, true, true, true),
#else
    IO_MACRO_PCIE_ELEMENT_CONFIG(0, false, false, false, false, true),
#endif

#if (PLATFORM_VARIANT == 0)
    IO_MACRO_PCIE_ELEMENT_CONFIG(1, false, false, false, false, true),

    IO_MACRO_PCIE_ELEMENT_CONFIG(2, false, false, false, false, true),

    IO_MACRO_PCIE_ELEMENT_CONFIG(3, false, false, false, false, true),
#endif

    { 0 }
};

static const struct mod_pcie_setup_module_config module_config = {
    .resource_info_count = NUMBER_OF_CHIPS,
    .resource_info = ((struct mod_pcie_setup_resource_info[NUMBER_OF_CHIPS]) {
        CHIP_MEMMAP(0),
/*
 * Keep the basic memory map regions identical. The module during runtime
 * will detect the chip number and update the translation fields
 * accordingly.
 */
#if (PLATFORM_VARIANT == 2)
            CHIP_MEMMAP(1), CHIP_MEMMAP(2), CHIP_MEMMAP(3),
#endif
    }),
};

static void fix_address(
    struct mod_pcie_setup_config *element_config,
    uint64_t chip_offset)
{
    element_config->reg_base += chip_offset;
    element_config->smmu_base += chip_offset;
}

static const struct fwk_element *pcie_setup_get_element_table(
    fwk_id_t module_id)
{
    uint64_t chip_offset;
    uint8_t chip_id;
    uint8_t idx;

    chip_offset = platform_get_chip_offset();
    chip_id = platform_get_chip_id();

    FWK_LOG_INFO("[PCIe Setup] chip_id: %d", chip_id);

    if (chip_id != 0) {
        for (idx = 0; pcie_setup_element_table[idx].data != NULL; idx++) {
            fix_address(
                (struct mod_pcie_setup_config *)pcie_setup_element_table[idx]
                    .data,
                chip_offset);
        }
    }
    return pcie_setup_element_table;
}

struct fwk_module_config config_pcie_setup = {
    .data = &module_config,
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(pcie_setup_get_element_table),
};
