/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'pik_clock'.
 */

#include "core_manager.h"
#include "scp_clock.h"
#include "scp_pwrctrl.h"
#include "system_pik.h"
#include "platform_variant.h"

#include <mod_pik_clock.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdbool.h>

#define CFGD_MOD_PIK_CLOCK_ELEMENT_CPU(n) \
    [CFGD_MOD_PIK_CLOCK_EIDX_CPU##n] = { \
        .name = "PIK CLK CPU" #n, \
        .data = &((struct mod_pik_clock_dev_config){ \
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE, \
            .is_group_member = false, \
            .control_reg = \
                &SCP_CU_AP_PERIPH_CORE_MANAGER_PTR(n)->CORECLK_CTRL, \
            .divext_reg = &SCP_CU_AP_PERIPH_CORE_MANAGER_PTR(n)->CORECLK_DIV1, \
            .rate_table = rate_table_cpu_clk, \
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_clk), \
            .initial_rate = 0, \
        }), \
    }

/* CPU clock rate table */
static const struct mod_pik_clock_rate rate_table_cpu_clk[] = {
    {
        .rate = 0,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1,
    },
};

/* Cache Coherent Interconnect clock rate table */
static const struct mod_pik_clock_rate rate_table_int_clk[] = {
    {
        .rate = 2000 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_INTCLK_SOURCE_INTPLL,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1,
    },
};

/* SCP CORE clock rate table */
static const struct mod_pik_clock_rate rate_table_scp_clk[] = {
    {
        .rate = 800 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (800 * FWK_MHZ),
    },
};

/* GIC clock rate table */
static const struct mod_pik_clock_rate rate_table_gic_clk[] = {
    {
        .rate = 1000 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (1000 * FWK_MHZ),
    },
};

/* SCP PIK clock rate table */
static const struct mod_pik_clock_rate rate_table_scp_pik_clk[] = {
    {
        .rate = 400 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (400 * FWK_MHZ),
    },
};

/* System Peripheral clock rate table */
static const struct mod_pik_clock_rate rate_table_sysper_clk[] = {
    {
        .rate = 500 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (500 * FWK_MHZ),
    },
};

/* UART clock rate table */
static const struct mod_pik_clock_rate rate_table_uart_clk[] = {
    {
        .rate = 250 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (250 * FWK_MHZ),
    },
};

static const struct fwk_element pik_clock_table[] = {
    CFGD_MOD_PIK_CLOCK_ELEMENT_CPU(0),
    CFGD_MOD_PIK_CLOCK_ELEMENT_CPU(1),
    CFGD_MOD_PIK_CLOCK_ELEMENT_CPU(2),
    CFGD_MOD_PIK_CLOCK_ELEMENT_CPU(3),
    CFGD_MOD_PIK_CLOCK_ELEMENT_CPU(4),
    CFGD_MOD_PIK_CLOCK_ELEMENT_CPU(5),
    CFGD_MOD_PIK_CLOCK_ELEMENT_CPU(6),
    CFGD_MOD_PIK_CLOCK_ELEMENT_CPU(7),
#if (PLATFORM_VARIANT == RD_V3_R1)
    CFGD_MOD_PIK_CLOCK_ELEMENT_CPU(8),
    CFGD_MOD_PIK_CLOCK_ELEMENT_CPU(9),
    CFGD_MOD_PIK_CLOCK_ELEMENT_CPU(10),
    CFGD_MOD_PIK_CLOCK_ELEMENT_CPU(11),
    CFGD_MOD_PIK_CLOCK_ELEMENT_CPU(12),
    CFGD_MOD_PIK_CLOCK_ELEMENT_CPU(13),
#endif
    [CFGD_MOD_PIK_CLOCK_EIDX_CMN] = {
        .name = "PIK CLK CMN",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SYSTEM_PIK_PTR->INTCLK_CTRL,
            .divext_reg = &SYSTEM_PIK_PTR->INTCLK_DIV1,
            .rate_table = rate_table_int_clk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_int_clk),
            .initial_rate = 2000 * FWK_MHZ,
        }),
    },
    [CFGD_MOD_PIK_CLOCK_EIDX_SCP] = {
        .name = "PIK CLK SCP",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SCP_PWRCTRL_PTR->CORECLK_CTRL,
            .divsys_reg = &SCP_PWRCTRL_PTR->CORECLK_DIV1,
            .rate_table = rate_table_scp_clk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_scp_clk),
            .initial_rate = 800 * FWK_MHZ,
        }),
    },
    [CFGD_MOD_PIK_CLOCK_EIDX_GIC] = {
        .name = "PIK CLK GIC",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SYSTEM_PIK_PTR->GICCLK_CTRL,
            .divsys_reg = &SYSTEM_PIK_PTR->GICCLK_DIV1,
            .rate_table = rate_table_gic_clk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_gic_clk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CFGD_MOD_PIK_CLOCK_EIDX_SCP_PIK] = {
        .name = "PIK CLK SCP PIK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SYSTEM_PIK_PTR->SCPPIKCLK_CTRL,
            .divsys_reg = &SYSTEM_PIK_PTR->SCPPIKCLK_DIV1,
            .rate_table = rate_table_scp_pik_clk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_scp_pik_clk),
            .initial_rate = 400 * FWK_MHZ,
        }),
    },
    [CFGD_MOD_PIK_CLOCK_EIDX_SYSPERCLK] = {
        .name = "PIK CLK SYSPER",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SYSTEM_PIK_PTR->SYSPERCLK_CTRL,
            .divsys_reg = &SYSTEM_PIK_PTR->SYSPERCLK_DIV1,
            .rate_table = rate_table_sysper_clk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sysper_clk),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CFGD_MOD_PIK_CLOCK_EIDX_UARTCLK] = {
        .name = "PIK CLK UART",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SYSTEM_PIK_PTR->APUARTCLK_CTRL,
            .divsys_reg = &SYSTEM_PIK_PTR->APUARTCLK_DIV1,
            .rate_table = rate_table_uart_clk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_uart_clk),
            .initial_rate = 250 * FWK_MHZ,
        }),
    },
    [CFGD_MOD_PIK_CLOCK_EIDX_COUNT] = { 0 },
};

static const struct fwk_element *pik_clock_get_element_table(fwk_id_t module_id)
{
    return pik_clock_table;
}

const struct fwk_module_config config_pik_clock = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(pik_clock_get_element_table),
};
