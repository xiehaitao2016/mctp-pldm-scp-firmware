/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SYSTEM_PIK_H
#define MOD_SYSTEM_PIK_H

#include <fwk_attributes.h>
#include <fwk_id.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "system_pik.h"

#define TCU_L0GPTSZ_SHIFT       2UL
#define TCU_L0GPTSZ_MASK        0x0FUL
#define L0GPTSZ_16GIB           0x04UL

/*!
 * \brief System PIK module configuration data.
 */
struct mod_system_pik_ctx {

    struct system_pik_reg *system_pik_reg;
};

struct mod_system_pik_device_config {
    /*! Base address of the system PIK register */
    uintptr_t system_pik_base;

    uint32_t l0_gpt_size;
};

/*!
 * \}
 */

#endif /* MOD_SYSTEM_PIK_H */
