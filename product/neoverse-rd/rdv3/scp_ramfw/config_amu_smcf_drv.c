/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform_core.h"
#include "scp_fw_mmap.h"

#include <mod_amu_smcf_drv.h>

#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdio.h>

#define SMCF_AMU_DRV_CORE_NAME_SIZE UINT8_C(16)
#define SMCF_AMU_TAG_BUFFER_SIZE UINT32_C(0)

/* AMU counters config */
#define CORE_TYPE_AUX_MEMORY_GAP UINT32_C(0xE0)

enum core_counters_layout {
    CORE_COUNTER_CORE,
    CORE_COUNTER_CONST,
    CORE_COUNTER_INST_RET,
    CORE_COUNTER_MEM_STALL,
    CORE_COUNTER_AUX0,
    CORE_COUNTER_AUX1,
    CORE_COUNTER_AUX2,
    NUM_OF_CORE_COUNTERS,
};

static uint32_t amu_counter_offsets[] = {
    [CORE_COUNTER_CORE] = 0,
    [CORE_COUNTER_CONST] = sizeof(uint64_t) * CORE_COUNTER_CONST,
    [CORE_COUNTER_INST_RET] = sizeof(uint64_t) * CORE_COUNTER_INST_RET,
    [CORE_COUNTER_MEM_STALL] = sizeof(uint64_t) * CORE_COUNTER_MEM_STALL,
    [CORE_COUNTER_AUX0] =
        CORE_TYPE_AUX_MEMORY_GAP + sizeof(uint64_t) * CORE_COUNTER_AUX0,
    [CORE_COUNTER_AUX1] =
        CORE_TYPE_AUX_MEMORY_GAP + sizeof(uint64_t) * CORE_COUNTER_AUX1,
    [CORE_COUNTER_AUX2] =
        CORE_TYPE_AUX_MEMORY_GAP + sizeof(uint64_t) * CORE_COUNTER_AUX2,
};

static const struct fwk_element *amu_drv_get_element_table(fwk_id_t module_id)
{
    unsigned int const core_count = platform_get_core_count();
    unsigned int core_idx;
    struct fwk_element *element_table, *element;
    struct amu_smcf_drv_element_config *amu_smcf_drv_core_config_table, *config;
    element_table = fwk_mm_calloc(core_count, sizeof(struct fwk_element));
    amu_smcf_drv_core_config_table =
        fwk_mm_calloc(core_count, sizeof(struct amu_smcf_drv_element_config));

    for (core_idx = 0; core_idx < core_count; ++core_idx) {
        element = &element_table[core_idx];
        config = &amu_smcf_drv_core_config_table[core_idx];

        element->name = fwk_mm_alloc(SMCF_AMU_DRV_CORE_NAME_SIZE, 1);
        element->sub_element_count = NUM_OF_CORE_COUNTERS;
        element->data = config;
        snprintf(
            (char *)element->name,
            SMCF_AMU_DRV_CORE_NAME_SIZE,
            "CORE_IDX_%u",
            core_idx);

        /* Init config */
        config->smcf_mli_id =
            FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_SMCF, core_idx, 0);
        config->counter_offsets = amu_counter_offsets;
        config->amu_tag_buffer_size = SMCF_AMU_TAG_BUFFER_SIZE;
    }

    return element_table;
}

struct fwk_module_config config_amu_smcf_drv = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(amu_drv_get_element_table),
};
