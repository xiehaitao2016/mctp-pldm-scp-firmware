/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform_core.h"
#include "scp_fw_mmap.h"
#include "smcf_utils.h"

#include <mod_smcf.h>

#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdio.h>

#define SMCF_MGI_NAME_SIZE        UINT8_C(16)
#define SMCF_NUM_CORE_TEMP_SENSOR UINT32_C(1)
#define SMCF_NUM_CORE_AMU         UINT32_C(1)
#define SMCF_TAG_LENGTH           UINT32_C(64)
#define SMCF_LCP_COMBINED_INT     UINT32_C(63)

#define LCP_PERIPHERALS_OFFSET        UINT32_C(0x60000000)
#define SMCF_MGI_REG_SPACE            UINT32_C(0X00200000)
#define SMCF_AMU_OFFSET               UINT32_C(0x000D0000)
#define SMCF_TEMP_SENSOR_OFFSET       UINT32_C(0x000D1000)
#define MGI_WR_SMCF_RAM_START_ADDRESS UINT32_C(0x70000000)
#define SMCF_READ_RAM_START_ADDRESS   UINT32_C(0x51000000)

/*
 * MGI_COUNTER_SIZE: the current size of counter in bytes
 */
#define MGI_COUNTER_SIZE UINT32_C(8)
/*
 * MGI_NUMBER_OF_COUNTER: the current number of counters
 */
#define MGI_NUMBER_OF_COUNTER UINT32_C(7)
/*
 * MGI_DEAD_PADDING: spacing between Archi and Aux counters
 */
#define MGI_DEAD_PADDING UINT32_C(224)

#define MGI_AMU_DATA_SIZE \
    ((MGI_COUNTER_SIZE * MGI_NUMBER_OF_COUNTER) + MGI_DEAD_PADDING)
#define MGI_SENSOR_DATA_SIZE UINT32_C(4)

/*
 * SMCF Context module configuration
 */
static void smcf_data_format_config(
    struct mod_smcf_data_config *const data_config)
{
    data_config->header_format = SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID |
        SMCF_SAMPLE_HEADER_FORMAT_DATA_VALID_BITS |
        SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID |
        SMCF_SAMPLE_HEADER_FORMAT_END_ID;

    data_config->data_location = SMCF_DATA_LOCATION_RAM;
}

static const struct fwk_element *smcf_get_element_table(fwk_id_t module_id)
{
    struct mod_smcf_element_config *mgi_config_table, *mgi_config;
    unsigned int const core_count = platform_get_core_count();
    struct fwk_element *element_table, *element;
    unsigned int core_idx;
    uint32_t smcf_ram_wr_address = MGI_WR_SMCF_RAM_START_ADDRESS;
    uint32_t smcf_ram_rd_address = SMCF_READ_RAM_START_ADDRESS;

    element_table = fwk_mm_calloc(
        core_count * SMCF_NUM_CORE_AMU + 1,
        sizeof(struct fwk_element));

    mgi_config_table = fwk_mm_calloc(
            core_count * SMCF_NUM_CORE_AMU + 1,
        sizeof(struct mod_smcf_element_config));

    /* AMU MGI Configuration */
    for (core_idx = 0; core_idx < core_count; core_idx++) {
        element = &element_table[core_idx];
        mgi_config = &mgi_config_table[core_idx];

        element->name = fwk_mm_alloc(SMCF_MGI_NAME_SIZE, 1);
        snprintf(
            (char *)element->name, SMCF_MGI_NAME_SIZE, "MGI_AMU_%u", core_idx);
        element->sub_element_count = SMCF_NUM_CORE_TEMP_SENSOR;
        element->data = mgi_config;
        mgi_config->reg_base = LCP_PERIPHERALS_OFFSET + SMCF_AMU_OFFSET +
            (core_idx * SMCF_MGI_REG_SPACE);
        mgi_config->sample_type = SMCF_SAMPLE_TYPE_TRIGGER_INPUT;
        mgi_config->irq = SMCF_LCP_COMBINED_INT;

        smcf_data_format_config(&mgi_config->data_config);

        mgi_config->data_config.write_addr = (uint64_t)(smcf_ram_wr_address);
        mgi_config->data_config.read_addr = (uint32_t *)(smcf_ram_rd_address);

        smcf_ram_wr_address = get_next_smcf_ram_offset(
            smcf_ram_wr_address,
            mgi_config->data_config.header_format,
            MGI_AMU_DATA_SIZE,
            SMCF_TAG_LENGTH);

        smcf_ram_rd_address = get_next_smcf_ram_offset(
            smcf_ram_rd_address,
            mgi_config->data_config.header_format,
            MGI_AMU_DATA_SIZE,
            SMCF_TAG_LENGTH);
    }

    return element_table;
}

struct fwk_module_config config_smcf = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(smcf_get_element_table),
};
