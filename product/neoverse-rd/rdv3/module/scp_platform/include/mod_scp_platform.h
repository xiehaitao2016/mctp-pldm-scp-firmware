/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCP Platform Support
 */

#ifndef MOD_SCP_PLATFORM_H
#define MOD_SCP_PLATFORM_H

#include <mod_power_domain.h>

#include <fwk_id.h>

#include <stdint.h>

#define WARM_RESET_MAX_RETRIES 10

/*!
 * \addtogroup GroupPLATFORMModule PLATFORM Product Modules
 * @{
 */

/*!
 * \defgroup GroupSCPPlatform SCP Platform Support
 * @{
 */

/*!
 * \brief Module context.
 */
struct scp_platform_ctx {
    /* Module config data */
    const struct mod_scp_platform_config *config;

    /* System Information HAL API pointer */
    struct mod_system_info_get_info_api *system_info_api;

    /* Count of number of warm reset completion check iterations */
    unsigned int warm_reset_check_cnt;
};

/*!
 * \brief Indices of the interfaces exposed by the module.
 */
enum mod_scp_platform_api_idx {
    /*! API index for the powerdown interface of SCMI module */
    MOD_SCP_PLATFORM_API_IDX_SCMI_POWER_DOWN,

    /*! API index for the driver interface of the SYSTEM POWER module */
    MOD_SCP_PLATFORM_API_IDX_SYSTEM_POWER_DRIVER,

    /*! Interface for Transport module */
    MOD_SCP_PLATFORM_API_IDX_TRANSPORT_SIGNAL,

    /*! Number of exposed interfaces */
    MOD_SCP_PLATFORM_API_COUNT
};

/*!
 * \brief Events used by scp platform module.
 */
enum mod_scp_platform_event_idx {
    /*! Event requesting check for power domain OFF */
    MOD_SCP_PLATFORM_CHECK_PD_OFF,

    /*! Number of defined events */
    MOD_SCP_PLATFORM_EVENT_COUNT
};

/*!
 * \brief Event to check all CPUs are powered off.
 */
static const fwk_id_t mod_scp_platform_event_check_ppu_off = FWK_ID_EVENT(
    FWK_MODULE_IDX_SCP_PLATFORM,
    MOD_SCP_PLATFORM_CHECK_PD_OFF);

/*!
 * \brief Notification indices.
 */
enum mod_scp_platform_notification_idx {
    /*! SCP subsystem initialization completion notification */
    MOD_SCP_PLATFORM_NOTIFICATION_IDX_SUBSYS_INITIALIZED,

    /*! Number of notifications defined by the module */
    MOD_SCP_PLATFORM_NOTIFICATION_COUNT,
};

/*!
 * \brief Identifier for the
 * ::MOD_SCP_PLATFORM_NOTIFICATION_IDX_SUBSYS_INITIALIZED notification.
 */
static const fwk_id_t mod_scp_platform_notification_subsys_init =
    FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_SCP_PLATFORM,
        MOD_SCP_PLATFORM_NOTIFICATION_IDX_SUBSYS_INITIALIZED);

/*!
 * \brief SCP platform configuration data.
 */
struct mod_scp_platform_config {
    /*! Timer identifier */
    fwk_id_t timer_id;

    /*! Transport channel identifier */
    fwk_id_t transport_id;

    /*!
     * Maximum amount of time, in microseconds, to wait for the RSS handshake
     * event.
     */
    uint32_t rss_sync_wait_us;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SCP_PLATFORM_H */
