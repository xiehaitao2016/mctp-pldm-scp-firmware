/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Power capping module.
 */

#ifndef MOD_POWER_CAPPING_H
#define MOD_POWER_CAPPING_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupPOWER_CAPPING power capping module
 * \{
 */

/*!
 * \brief Power Capping interface.
 */
struct mod_power_capping_api {
    /*!
     * \brief Request power cap.
     *
     * \param domain_id Power Capping domain ID.
     * \param cap Requested cap to the system.
     *
     * \retval ::FWK_SUCCESS The requested cap was applied successfully.
     * \retval ::FWK_PENDING The requested cap was registered but not yet
     * applied. \return One of the standard framework error codes.
     */
    int (*request_cap)(fwk_id_t domain_id, uint32_t cap);

    /*!
     * \brief Get the applied power cap.
     *
     * \param domain_id Power Capping domain ID.
     * \param cap The applied cap to the system. Note that the applied cap
     *     is different from the requested where the applied cap is considered
     *     the real cap considered by the system and the requested cap is the
     *     cap that has been asked by an agent and is not guaranteed to be the
     *     the applied one.
     *
     * \retval ::FWK_SUCCESS The applied cap was returned successfully.
     * \return One of the standard framework error codes.
     */
    int (*get_applied_cap)(fwk_id_t domain_id, uint32_t *cap);
};

/*!
 * \brief Power Capping domain configuration.
 */
struct mod_power_capping_domain_config {
    /*!
     * \brief ID of the object that outputs a power limit.
     */
    fwk_id_t power_limiter_id;
    /*!
     * \brief API ID of the api that would return the power limit.
     */
    fwk_id_t power_limiter_api_id;
    /*!
     * \brief ID of the object that sets notification for the power
     *    limit change.
     */
    fwk_id_t power_limit_set_notifier_id;
    /*!
     * \brief Notification ID of the power limit change notification.
     */
    fwk_id_t power_limit_set_notification_id;
};

/*!
 * \brief Power Capping API indices.
 */
enum mod_power_capping_api_idx {
    /*! Power Cap API. */
    MOD_POWER_CAPPING_API_IDX_CAP,

    /*! Power Management API. */
    MOD_POWER_CAPPING_API_IDX_POWER_MANAGEMENT,

    /*! Number of defined APIs. */
    MOD_POWER_CAPPING_API_IDX_COUNT,
};

/*!
 * \brief Power Capping module notification indices.
 */
enum mod_power_capping_notification_idx {
    /*! Power cap change notification. */
    MOD_POWER_CAPPING_NOTIFICATION_IDX_CAP_CHANGE,

    /*! Number of defined notifications. */
    MOD_POWER_CAPPING_NOTIFICATION_IDX_COUNT,
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_POWER_CAPPING_H */
