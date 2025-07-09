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
#include "platform_variant.h"

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
        .data = &((struct mod_io_block_element_config) \
        { \
            .region_mem_maps_count = 1, \
            .region_mem_maps = ((struct interface_io_block_setup_mmap[]) \
            { \
/*
* "Warning: The address region that corresponds to the internal configuration
* From NOC-S3 TRM:
* space must be programmed in the address map as one of the non-striped
* regions.
* The target for this non-striped region must be the
* Configuration Network Interface (CFGNI) interface ID.
* If this requirement is not followed, access to the internal configuration
* completer interface cannot be achieved after the address map has been
* programmed.”
*/ \
                { \
                    .io_block_address = IO_NOC_S3_GPV_BLOCK_BASE(idx), \
                    .source_id = ASNI_HNP0_CMN, \
                    .region_count = 4, \
                    .carveout_info = ((struct interface_io_block_carveout_info[]){ \
                        { \
                            .base = IO_NOC_S3_GPV_BLOCK_BASE(idx), \
                            .size = IO_NOC_S3_GPV_BLOCK_SIZE, \
                            .target_id = CFGNI_ID, \
                            .region_id = 0, \
                        }, \
                        { \
                            .base = TCU_REG_BASE(idx), \
                            .size = TCU_REG_SIZE, \
                            .target_id = PMNI_TCU_APB, \
                            .region_id = 1, \
                        }, \
                        { \
                            .base = CTRL_REG_BASE(idx), \
                            .size = CTRL_REG_SIZE, \
                            .target_id = PMNI_CTRL_REG_APB, \
                            .region_id = 2, \
                        }, \
                        { \
                            .base = RAS_AGENT_REG_BASE(idx), \
                            .size = RAS_AGENT_REG_SIZE, \
                            .target_id = PMNI_RAS_AGENT_APB, \
                            .region_id = 3, \
                        }, \
                    }) \
                }, \
                { \
                    .io_block_address = IO_NOC_S3_GPV_BLOCK_BASE(idx), \
                    .source_id = ASNI_HNP1_CMN, \
                    .region_count = 4, \
                    .carveout_info = ((struct interface_io_block_carveout_info[]){ \
                        { \
                            .base = IO_NOC_S3_GPV_BLOCK_BASE(idx), \
                            .size = IO_NOC_S3_GPV_BLOCK_SIZE, \
                            .target_id = CFGNI_ID, \
                            .region_id = 0, \
                        }, \
                        { \
                            .base = TCU_REG_BASE(idx), \
                            .size = TCU_REG_SIZE, \
                            .target_id = PMNI_TCU_APB, \
                            .region_id = 1, \
                        }, \
                        { \
                            .base = CTRL_REG_BASE(idx), \
                            .size = CTRL_REG_SIZE, \
                            .target_id = PMNI_CTRL_REG_APB, \
                            .region_id = 2, \
                        }, \
                        { \
                            .base = RAS_AGENT_REG_BASE(idx), \
                            .size = RAS_AGENT_REG_SIZE, \
                            .target_id = PMNI_RAS_AGENT_APB, \
                            .region_id = 3, \
                        }, \
                    }) \
                } \
            }) \
        }) \
    }

static struct mod_io_block_config io_block_module_config = {
    .noc_s3_id = FWK_ID_MODULE(FWK_MODULE_IDX_NOC_S3),
    .noc_s3_api_id =
        FWK_ID_API(FWK_MODULE_IDX_NOC_S3, MOD_NOC_S3_API_SETUP_PSAM),
    .atu_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_ATU, 0),
    .atu_api_id = FWK_ID_API(FWK_MODULE_IDX_ATU, MOD_ATU_API_IDX_ATU),
    .plat_notification = {
        .notification_id = FWK_ID_NOTIFICATION_INIT(
                FWK_MODULE_IDX_SCP_PLATFORM,
                MOD_SCP_PLATFORM_NOTIFICATION_IDX_SUBSYS_INITIALIZED),
        .source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCP_PLATFORM),
    },
    .logical_region_base = SCP_ATW0_AP_IO_BLOCK_NOC_S3_BASE,
    .logical_region_size = SCP_ATW0_AP_IO_BLOCK_NOC_S3_SIZE
};

static const struct fwk_element io_block_element_table[] = {
    IO_MACRO_ELEMENT_CONFIG(0),
#if (PLATFORM_VARIANT == RD_V3_R1)
    IO_MACRO_ELEMENT_CONFIG(1),
#endif
    { 0 }
};

void io_block_add_chip_offset(struct mod_io_block_element_config *element_config)
{
    uint8_t block_idx = 0;
    uint8_t region_idx = 0;
    struct interface_io_block_carveout_info *carveout_info = NULL;
    uint64_t chip_offset = platform_get_chip_offset();
    struct interface_io_block_setup_mmap *block_setup_mmap = element_config->region_mem_maps;

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
    uint8_t idx = 0;

    FWK_LOG_INFO("[IO Block] chip_id: %d", chip_id);

    for (idx = 0; io_block_element_table[idx].data != NULL; idx++) {
        io_block_add_chip_offset((struct mod_io_block_element_config *)io_block_element_table[idx].data);
    }

    return io_block_element_table;
}

struct fwk_module_config config_io_block = {
    .data = &io_block_module_config,
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(io_block_get_element_table),
};
