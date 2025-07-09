/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'mcp_platform'.
 */

#include "mcp_cfgd_timer.h"
#include "mod_mcp_platform.h"

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>


#define SCP_BOOT_COMPLETE_TIMEOUT_MS (10)

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static const struct mod_mcp_platform_config platform_config_data = {
#if (PLATFORM_VARIANT == 2)
    .timer_id = FWK_ID_NONE_INIT,
#else
    .timer_id = FWK_ID_SUB_ELEMENT_INIT(
        FWK_MODULE_IDX_TIMER,
        MCP_ALARM_ELEMENT_IDX,
        MCP_CFGD_SCP_BOOT_COMPLETE_ALARM_IDX),
    .scp_boot_complete_timeout_ms = SCP_BOOT_COMPLETE_TIMEOUT_MS,
#endif
};
#endif

struct fwk_module_config config_mcp_platform = {
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    .data = &platform_config_data,
#else
    0
#endif
};
