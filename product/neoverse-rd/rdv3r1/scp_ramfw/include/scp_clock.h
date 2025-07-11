/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCP clock definitions.
 */

#ifndef SCP_CLOCK_H
#define SCP_CLOCK_H

#include "css_common.h"
#include "platform_variant.h"

#include <fwk_macros.h>

#define CLOCK_RATE_SYSPLLCLK (2000UL * FWK_MHZ)

/*
 * PIK clock indexes.
 */
enum clock_pik_idx {
    CFGD_MOD_PIK_CLOCK_EIDX_CPU0,
    CFGD_MOD_PIK_CLOCK_EIDX_CPU1,
    CFGD_MOD_PIK_CLOCK_EIDX_CPU2,
    CFGD_MOD_PIK_CLOCK_EIDX_CPU3,
    CFGD_MOD_PIK_CLOCK_EIDX_CPU4,
    CFGD_MOD_PIK_CLOCK_EIDX_CPU5,
    CFGD_MOD_PIK_CLOCK_EIDX_CPU6,
    CFGD_MOD_PIK_CLOCK_EIDX_CPU7,
#if (PLATFORM_VARIANT == RD_V3_R1)
    CFGD_MOD_PIK_CLOCK_EIDX_CPU8,
    CFGD_MOD_PIK_CLOCK_EIDX_CPU9,
    CFGD_MOD_PIK_CLOCK_EIDX_CPU10,
    CFGD_MOD_PIK_CLOCK_EIDX_CPU11,
    CFGD_MOD_PIK_CLOCK_EIDX_CPU12,
    CFGD_MOD_PIK_CLOCK_EIDX_CPU13,
#endif
    CFGD_MOD_PIK_CLOCK_EIDX_CMN,
    CFGD_MOD_PIK_CLOCK_EIDX_SCP,
    CFGD_MOD_PIK_CLOCK_EIDX_GIC,
    CFGD_MOD_PIK_CLOCK_EIDX_SCP_PIK,
    CFGD_MOD_PIK_CLOCK_EIDX_SYSPERCLK,
    CFGD_MOD_PIK_CLOCK_EIDX_UARTCLK,
    CFGD_MOD_PIK_CLOCK_EIDX_COUNT
};

/*
 * Module 'clock' element indexes
 */
enum cfgd_mod_clock_element_idx {
    CFGD_MOD_CLOCK_EIDX_CPU0,
    CFGD_MOD_CLOCK_EIDX_CPU1,
    CFGD_MOD_CLOCK_EIDX_CPU2,
    CFGD_MOD_CLOCK_EIDX_CPU3,
    CFGD_MOD_CLOCK_EIDX_CPU4,
    CFGD_MOD_CLOCK_EIDX_CPU5,
    CFGD_MOD_CLOCK_EIDX_CPU6,
    CFGD_MOD_CLOCK_EIDX_CPU7,
#if (PLATFORM_VARIANT == RD_V3_R1)
    CFGD_MOD_CLOCK_EIDX_CPU8,
    CFGD_MOD_CLOCK_EIDX_CPU9,
    CFGD_MOD_CLOCK_EIDX_CPU10,
    CFGD_MOD_CLOCK_EIDX_CPU11,
    CFGD_MOD_CLOCK_EIDX_CPU12,
    CFGD_MOD_CLOCK_EIDX_CPU13,
#endif
    CFGD_MOD_CLOCK_EIDX_CMN,
    CFGD_MOD_CLOCK_EIDX_COUNT
};

#endif /* SCP_CLOCK_H */
