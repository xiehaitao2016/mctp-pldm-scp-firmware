/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     MCP Platform Support
 */

#ifndef MOD_MCP_PLATFORM_H
#define MOD_MCP_PLATFORM_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupPLATFORMModule PLATFORM Product Modules
 * @{
 */

/*!
 * \defgroup GroupMCPPlatform MCP Platform Support
 * @{
 */

/*!
 * \brief Indices of the interfaces exposed by the module.
 */
enum mod_mcp_platform_api_idx {
    /*! API index for the powerdown interface of SCMI module */
    MOD_MCP_PLATFORM_API_IDX_SCMI_POWER_DOWN,

    /*! Number of exposed interfaces */
    MOD_MCP_PLATFORM_API_COUNT
};

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
/*!
 * \brief MCP platform configuration data.
 */
struct mod_mcp_platform_config {
    /*! Timer identifier */
    fwk_id_t timer_id;

    /* Maximum time in milliseconds to wait for SCP boot complete */
    uint32_t scp_boot_complete_timeout_ms;
};
#endif

/*!
 * @}
 */

#endif /* MOD_MCP_PLATFORM_H */
