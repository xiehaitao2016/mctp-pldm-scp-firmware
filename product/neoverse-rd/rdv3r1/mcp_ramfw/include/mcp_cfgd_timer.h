/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for timer module configuration data in MCP firmware.
 */

#ifndef MCP_CFGD_TIMER_H
#define MCP_CFGD_TIMER_H

#define MCP_ALARM_ELEMENT_IDX    0

/* Sub-element indexes (alarms) for MCP timer device */
enum mcp_cfgd_mod_timer_subelement_idx {
    MCP_CFGD_MCTP_ALARM_IDX,
    SCP_CFGD_MOD_TIMER_SEIDX_DEBUGGER_CLI,
    MCP_CFGD_MOD_TIMER_SEIDX_ALARM_COUNT,
};

#endif /* MCP_CFGD_TIMER_H */
