/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024 Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'pcie'.
 */

#include "platform_variant.h"
#include "cmn_node_id.h"
#include "io_macro_layout.h"
#include "scp_cfgd_sds.h"
#include "scp_fw_mmap.h"
#include "platform_variant.h"

#include <mod_atu_mmio.h>
#include <mod_cmn_cyprus.h>
#include <mod_io_block.h>
#include <mod_pcie_setup.h>
#include <mod_scp_platform.h>

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

/* PCIe ECAM Base */
#define ECAM_BASE UINT64_C(0x10000000000)
/* PCIe ECAM size (256 MiB) */
#define ECAM_SIZE (UINT64_C(256) * FWK_MIB)
/* PCIe ECAM base for a chip */
#define CHIP_ECAM_BASE(chip_id) (ECAM_BASE + (chip_id * ECAM_SIZE))

/* PCIe MMIO_L Base */
#define MMIOL_BASE UINT64_C(0x60000000)
/* PCIe MMIO_L size (256 MiB) */
#if (PLATFORM_VARIANT == RD_V3_R1_CFG1)
#define MMIOL_SIZE (UINT64_C(128) * FWK_MIB)
#else
#define MMIOL_SIZE (UINT64_C(256) * FWK_MIB)
#endif
/* PCIe MMIO_L base for a chip */
#define CHIP_MMIOL_BASE(chip_id) (MMIOL_BASE + (chip_id * MMIOL_SIZE))

/* PCIe MMIO_H Base */
#define MMIOH_BASE UINT64_C(0x10100000000)
/* PCIe MMIO_H size (64 GiB) */
#define MMIOH_SIZE (UINT64_C(64) * FWK_GIB)
/* PCIe MMIO_H base for a chip */
#define CHIP_MMIOH_BASE(chip_id) (MMIOH_BASE + (chip_id * MMIOH_SIZE))

/* PCIe bus size for a chip */
#define CHIP_PCIE_BUS_SIZE 256

#define NUMBER_OF_ENDPOINTS 5

/* Number of Chiplets */
#if (PLATFORM_VARIANT == RD_V3_R1_CFG1)
#define NUMBER_OF_CHIPS 4
#else
#define NUMBER_OF_CHIPS 2
#endif

#define CHIP_MEMMAP(chip_id) \
    { \
        .mmap = { \
            .ecam = { \
                .start = CHIP_ECAM_BASE(chip_id), \
                .size = ECAM_SIZE, \
            }, \
            .mmiol = { \
                .start = CHIP_MMIOL_BASE(chip_id), \
                .size = MMIOL_SIZE, \
            }, \
            .mmioh = { \
                .start = CHIP_MMIOH_BASE(chip_id), \
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
        .address_remapper_api_id = FWK_ID_API(FWK_MODULE_IDX_ATU_MMIO, \
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

#define IO_MACRO_EP_CONFIG(x16_0_en, x1_0_en, x4_0_en, x4_1_en, x8_0_en) \
    ((struct mod_pcie_setup_ep_config[NUMBER_OF_ENDPOINTS]){ \
        { .valid = x16_0_en, \
          .allow_ns_access = true, \
          .rp_node_id = AMNI_PCIEX16_0 }, \
        { .valid = x1_0_en, \
          .allow_ns_access = true, \
          .rp_node_id = AMNI_PCIEX1_0 }, \
        { .valid = x4_0_en, \
          .allow_ns_access = true, \
          .rp_node_id = AMNI_PCIEX4_0 }, \
        { .valid = x4_1_en, \
          .allow_ns_access = true, \
          .rp_node_id = AMNI_PCIEX4_1 }, \
        { .valid = x8_0_en, \
          .allow_ns_access = true, \
          .rp_node_id = AMNI_PCIEX8_0 }, \
    })

#define IO_MACRO_PCIE_ELEMENT_CONFIG(idx, x16_0_en, x1_0_en, x4_0_en, x4_1_en, x8_0_en) \
    { \
        .name = "IO Macro " #idx, \
        .data = &((struct mod_pcie_setup_config){ \
            .reg_base = IO_NOC_S3_GPV_BLOCK_BASE(idx), \
            .cmn_node_id = IOVB_NODE_ID##idx, \
            .nci_source_node_id = ASNI_HNP0_CMN, \
            .block_id = idx, \
            .smmu_base = TCU_REG_BASE(idx), \
            .sds_struct_id = SDS_PCIE_MMAP, \
            .ep_config = \
                IO_MACRO_EP_CONFIG(x16_0_en, x1_0_en, x4_0_en, x4_1_en, x8_0_en) }) \
    }

static const struct fwk_element pcie_setup_element_table[] = {
    IO_MACRO_PCIE_ELEMENT_CONFIG(0, false, false, false, false, true),
#if (PLATFORM_VARIANT == RD_V3_R1)
    IO_MACRO_PCIE_ELEMENT_CONFIG(1, false, false, false, false, true),
#endif
    { 0 }
};

static const struct mod_pcie_setup_module_config module_config = {
    .resource_info_count = NUMBER_OF_CHIPS,
    .resource_info = ((struct mod_pcie_setup_resource_info[NUMBER_OF_CHIPS]) {
        CHIP_MEMMAP(0),
        CHIP_MEMMAP(1),
#if (PLATFORM_VARIANT == RD_V3_R1_CFG1)
        CHIP_MEMMAP(2),
        CHIP_MEMMAP(3),
#endif

/*
 * Keep the basic memory map regions identical. The module during runtime
 * will detect the chip number and update the translation fields
 * accordingly.
 */
    }),
};

void pcie_setup_add_chip_offset(struct mod_pcie_setup_config *pci_setup_config)
{
    uint64_t chip_offset = platform_get_chip_offset();

    pci_setup_config->reg_base += chip_offset;
    pci_setup_config->smmu_base += chip_offset;
}

static const struct fwk_element *pcie_setup_get_element_table(
    fwk_id_t module_id)
{
    uint8_t chip_id = 0;

    chip_id = platform_get_chip_id();

    FWK_LOG_INFO("[PCIe Setup] chip_id: %d", chip_id);

    uint8_t idx;
    for (idx = 0; pcie_setup_element_table[idx].data != NULL; idx++) {
        pcie_setup_add_chip_offset((struct mod_pcie_setup_config *)pcie_setup_element_table[idx].data);
    }

    return pcie_setup_element_table;
}

struct fwk_module_config config_pcie_setup = {
    .data = &module_config,
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(pcie_setup_get_element_table),
};
