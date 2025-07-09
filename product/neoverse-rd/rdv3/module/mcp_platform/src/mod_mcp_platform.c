/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_notification.h>
#include <fwk_status.h>
#include <fmw_cmsis.h>

#include <mod_scmi.h>
#include <mod_scmi_system_power.h>
#include <mod_timer.h>

#include "mcp_cfgd_scmi.h"
#include "mcp_cfgd_timer.h"
#include "mod_mcp_platform.h"

#define MOD_NAME "[MCP_PLATFORM] "

/* Module context */
struct mcp_platform_ctx {
    /*! SCMI protocol API */
    const struct mod_scmi_from_protocol_req_api *scmi_protocol_req_api;

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    /* Module config data */
    const struct mod_mcp_platform_config *config;

    /* Alarm API */
    struct mod_timer_alarm_api *alarm_api;
#endif
};

/* Module context data */
struct mcp_platform_ctx mcp_platform_ctx;

/*
 * Framework handlers.
 */
static int mod_mcp_platform_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    const struct mod_mcp_platform_config *config;

    config = (struct mod_mcp_platform_config *)data;
    if (!fwk_id_type_is_valid(config->timer_id)) {
        return FWK_E_DATA;
    }

    /* Save the config data in the module context */
    mcp_platform_ctx.config = config;
#endif

    return FWK_SUCCESS;
}

static int mod_mcp_platform_bind(fwk_id_t id, unsigned int round)
{
    int status;

    /* Bind to SCMI module for SCP communication */
    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL_REQ),
        &mcp_platform_ctx.scmi_protocol_req_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    if (round == 0) {
        /* Bind to timer module for SCP boot complete timeout */
        if (!fwk_id_is_equal(mcp_platform_ctx.config->timer_id, FWK_ID_NONE)) {
            status = fwk_module_bind(
                mcp_platform_ctx.config->timer_id,
                FWK_ID_API(FWK_MODULE_IDX_TIMER, MOD_TIMER_API_IDX_ALARM),
                &mcp_platform_ctx.alarm_api);
            if (status != FWK_SUCCESS) {
                FWK_LOG_ERR(MOD_NAME "Failed to bind to timer module: %s",
                    fwk_status_str(status));
                return status;
            }
        }
    }
#endif

    return status;
}

/*
 * SCMI module -> MCP platform module interface
 */
static int platform_system_get_scmi_protocol_id(
    fwk_id_t protocol_id,
    uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = (uint8_t)MOD_SCMI_PROTOCOL_ID_SYS_POWER;

    return FWK_SUCCESS;
}

/*
 * Upon binding the mcp_platform module to the SCMI module, the SCMI module
 * will also bind back to the mcp_platform module, anticipating the presence of
 * .get_scmi_protocol_id() and .message_handler() APIs.
 *
 * In the current implementation of mcp_platform module, only sending SCMI
 * message is implemented, and the mcp_platform module is not intended to
 * receive any SCMI messages. Therefore, it is necessary to include a minimal
 * .message_handler() API to ensure the successful binding of the SCMI module.
 */
static int platform_system_scmi_message_handler(
    fwk_id_t protocol_id,
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id)
{
    return FWK_SUCCESS;
}

/* SCMI driver interface */
const struct mod_scmi_to_protocol_api platform_system_scmi_api = {
    .get_scmi_protocol_id = platform_system_get_scmi_protocol_id,
    .message_handler = platform_system_scmi_message_handler,
};

static int mod_mcp_platform_process_bind(
    fwk_id_t requester_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    int status;
    enum mod_mcp_platform_api_idx api_id_type;

    api_id_type = (enum mod_mcp_platform_api_idx)fwk_id_get_api_idx(api_id);

    switch (api_id_type) {
    case MOD_MCP_PLATFORM_API_IDX_SCMI_POWER_DOWN:
        *api = &platform_system_scmi_api;
        status = FWK_SUCCESS;
        break;

    default:
        status = FWK_E_PARAM;
    }

    return status;
}

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static void scp_boot_complete_timeout_callback(uintptr_t param)
{
    int status = FWK_SUCCESS;

    fwk_id_t mcp_scmi_prot_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI, MCP_CFGD_MOD_SCMI_EIDX_PSCI_SEND);
    struct mcp_cfgd_scmi_sys_power_state_notify_payload mcp_scmi_payload;

    mcp_scmi_payload.flags = 1;

    status = mcp_platform_ctx.scmi_protocol_req_api->scmi_send_message(
        MOD_SCMI_SYS_POWER_STATE_NOTIFY,
        MOD_SCMI_PROTOCOL_ID_SYS_POWER,
        0,
        mcp_scmi_prot_id,
        (void *)&mcp_scmi_payload,
        sizeof(mcp_scmi_payload),
        false);

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            MOD_NAME
            "Failed to subscribe SCMI power state change notofication");
    }
}
#endif

static int mod_mcp_platform_start(fwk_id_t id)
{
    int status = FWK_SUCCESS;

#ifdef BUILD_HAS_NOTIFICATION
    status = fwk_notification_subscribe(
        mod_scmi_system_power_notification_system_power_down,
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_SYSTEM_POWER),
        id);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Failed to subscribe to power down notification");
    }
#endif

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    /* Start the Alarm */
    if (mcp_platform_ctx.alarm_api != NULL) {
        status = mcp_platform_ctx.alarm_api->start(
            mcp_platform_ctx.config->timer_id,
            mcp_platform_ctx.config->scp_boot_complete_timeout_ms,
            MOD_TIMER_ALARM_TYPE_ONCE,
            scp_boot_complete_timeout_callback,
            0);

        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME "Failed to configure timer alarm: %s",
                fwk_status_str(status));
        }
    }
#endif

    FWK_LOG_INFO(MOD_NAME "MCP RAM firmware initialized");
    return status;
}

#ifdef BUILD_HAS_NOTIFICATION
static int mcp_platform_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    unsigned int *power_down;

    /* Notification handler for system wide power down. */
    if (fwk_id_is_equal(
            event->id, mod_scmi_system_power_notification_system_power_down)) {
        power_down = (unsigned int *)event->params;

        if (*power_down == MCP_SCMI_SYSTEM_STATE_SHUTDOWN) {
            FWK_LOG_INFO(MOD_NAME "System shutting down!");
        } else if (*power_down == MCP_SCMI_SYSTEM_STATE_COLD_RESET) {
            FWK_LOG_INFO(MOD_NAME "System rebooting!");
        } else {
            FWK_LOG_ERR(MOD_NAME "Invalid power mode");
        }

        __WFI();
    } else {
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}
#endif

const struct fwk_module module_mcp_platform = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = MOD_MCP_PLATFORM_API_COUNT,
    .init = mod_mcp_platform_init,
    .bind = mod_mcp_platform_bind,
    .process_bind_request = mod_mcp_platform_process_bind,
    .start = mod_mcp_platform_start,
#ifdef BUILD_HAS_NOTIFICATION
    .process_notification = mcp_platform_process_notification,
#endif
};
