/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for transport module configuration data in MCP
 *     firmware.
 */

#ifndef MCP_CFGD_TRANSPORT_H
#define MCP_CFGD_TRANSPORT_H

/* Module 'transport' element indexes */
enum mcp_cfgd_mod_transport_element_idx {
    MCP_CFGD_MOD_TRANSPORT_EIDX_SYSTEM_POWER_DOWN_SEND,
    MCP_CFGD_MOD_TRANSPORT_EIDX_SYSTEM_POWER_DOWN_RECV,
    MCP_CFGD_MOD_TRANSPORT_EIDX_COUNT,
};

#endif /* MCP_CFGD_TRANSPORT_H */
