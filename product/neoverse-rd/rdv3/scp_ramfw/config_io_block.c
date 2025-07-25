/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'io_block'.
 */

#include "io_macro_layout.h"
#include "scp_css_mmap.h"
#include "scp_fw_mmap.h"

#include <mod_atu.h>
#include <mod_io_block.h>
#include <mod_noc_s3.h>
#include <mod_scp_platform.h>

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#define IO_MACRO_ELEMENT_CONFIG(idx) \
    { \
        .name = "IO Macro " #idx, \
        .data = &((struct mod_io_block_element_config){ \
            .region_mem_maps_count = 2, \
            .region_mem_maps = ((struct interface_io_block_setup_mmap[]){ \
                { .io_block_address = NCI_GVP_BLOCK_BASE(idx), \
                  .source_id = ASNI_CMN, \
                  .region_count = 1, \
                  .carveout_info = \
                      &((struct interface_io_block_carveout_info){ \
                          .base = TCU_REG_BASE(idx), \
                          .size = TCU_REG_SIZE, \
                          .target_id = AMNI_PMNI_TCU_APB, \
                          .region_id = 0, \
                      }) }, \
                { .io_block_address = NCI_GVP_BLOCK_BASE(idx), \
                  .source_id = ASNI_CMN, \
                  .region_count = 1, \
                  .carveout_info = \
                      &((struct interface_io_block_carveout_info){ \
                          .base = CTRL_REG_BASE(idx), \
                          .size = CTRL_REG_SIZE, \
                          .target_id = AMNI_PMNI_CTRL_REG_APB, \
                          .region_id = 0, \
                      }) }, \
            }), \
        }) \
    }

static struct mod_io_block_config io_block_module_config = {
    .nci_id = FWK_ID_MODULE(FWK_MODULE_IDX_NOC_S3),
    .nci_api_id =
        FWK_ID_API(FWK_MODULE_IDX_NOC_S3, MOD_NOC_S3_API_SETUP_PSAM),
    .atu_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_ATU, 0),
    .atu_api_id = FWK_ID_API(FWK_MODULE_IDX_ATU, MOD_ATU_API_IDX_ATU),
    .plat_notification = {
        .notification_id = FWK_ID_NOTIFICATION_INIT(
                FWK_MODULE_IDX_SCP_PLATFORM,
                MOD_SCP_PLATFORM_NOTIFICATION_IDX_SUBSYS_INITIALIZED),
        .source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCP_PLATFORM),
    },
    .logical_region_base = SCP_ATW0_AP_IO_BLOCK_NCI_BASE,
    .logical_region_size = SCP_ATW0_AP_IO_BLOCK_NCI_SIZE
};

static const struct fwk_element io_block_element_table[] = {
    IO_MACRO_ELEMENT_CONFIG(0),
#if (PLATFORM_VARIANT == 0)
    IO_MACRO_ELEMENT_CONFIG(1),
    IO_MACRO_ELEMENT_CONFIG(2),
    IO_MACRO_ELEMENT_CONFIG(3),
#endif
    { 0 }
};

static void fix_address(struct mod_io_block_element_config *element_config)
{
    struct interface_io_block_setup_mmap *block_setup_mmap;
    struct interface_io_block_carveout_info *carveout_info;
    uint64_t chip_offset;
    uint8_t block_idx;
    uint8_t region_idx;

    block_setup_mmap = element_config->region_mem_maps;
    chip_offset = platform_get_chip_offset();
    for (block_idx = 0; block_idx < element_config->region_mem_maps_count; block_idx++) {
        block_setup_mmap[block_idx].io_block_address += chip_offset ;
        carveout_info = block_setup_mmap[block_idx].carveout_info;
        for (region_idx = 0; region_idx < block_setup_mmap[block_idx].region_count; region_idx++) {
            carveout_info[region_idx].base += chip_offset;
        }
    }
}

static const struct fwk_element *io_block_get_element_table(fwk_id_t module_id)
{
    uint8_t chip_id = platform_get_chip_id();

    FWK_LOG_INFO("[IO Block] chip_id: %d", chip_id);
    uint8_t idx;

    if (chip_id != 0) {
        for (idx = 0; io_block_element_table[idx].data != NULL; idx++) {
            fix_address((struct mod_io_block_element_config *)io_block_element_table[idx].data);
        }
    }

    return io_block_element_table;
}

struct fwk_module_config config_io_block = {
    .data = &io_block_module_config,
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(io_block_get_element_table),
};
