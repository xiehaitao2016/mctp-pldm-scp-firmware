/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCP Platform Support - Power Management
 */

#include "platform_core.h"
#include "scp_cfgd_scmi.h"

#include <internal/scp_platform.h>

#include <mod_power_domain.h>
#include <mod_scmi.h>
#include <mod_scmi_std.h>
#include <mod_system_power.h>

#include <fwk_core.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <fmw_cmsis.h>

/* Module context data */
extern struct scp_platform_ctx scp_platform_ctx;

/*! SCMI protocol API */
static const struct mod_scmi_from_protocol_req_api *scmi_protocol_req_api;

/* Module 'power_domain' restricted API pointer */
static struct mod_pd_restricted_api *pd_restricted_api;

/* System shutdown function */
static int platform_shutdown(enum mod_pd_system_shutdown system_shutdown)
{
    int status;
    fwk_id_t rss_scmi_prot_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI, SCP_CFGD_MOD_SCMI_RSS_POWER_DOWN_SEND);
#ifndef BUILD_HAS_SCMI_NOTIFICATIONS
    fwk_id_t mcp_scmi_prot_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI, SCP_CFGD_MOD_SCMI_MCP_POWER_DOWN_SEND);
#endif
    struct scp_cfgd_scmi_sys_power_state_set_payload scp_scmi_payload;

    scp_scmi_payload.flags = 0;
    scp_scmi_payload.system_state = (uint32_t)system_shutdown;

    /*
     * The current RSS firmware lacks SCMI support, preventing the RSS platform
     * firmware from sending SCMI power state notify messages to the SCP or
     * subscribing to SCMI notifications. To address this limitation, notify
     * the RSS about system power down using the SCMI system power state set
     * message. As this message can be decoded in the RSS firmware with minimal
     * SCMI message parsing logic.
     */
    status = scmi_protocol_req_api->scmi_send_message(
        MOD_SCMI_SYS_POWER_STATE_SET,
        MOD_SCMI_PROTOCOL_ID_SYS_POWER,
        0,
        rss_scmi_prot_id,
        (void *)&scp_scmi_payload,
        sizeof(scp_scmi_payload),
        false);

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            "[SCP_PLATFORM] ERROR! Unable to send shutdown request to RSS");
        return status;
    }

#ifndef BUILD_HAS_SCMI_NOTIFICATIONS
    status = scmi_protocol_req_api->scmi_send_message(
        MOD_SCMI_SYS_POWER_STATE_SET,
        MOD_SCMI_PROTOCOL_ID_SYS_POWER,
        0,
        mcp_scmi_prot_id,
        (void *)&scp_scmi_payload,
        sizeof(scp_scmi_payload),
        false);

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            "[SCP_PLATFORM] ERROR! Unable to send shutdown request to MCP");
        return status;
    }
#endif

    while (1) {
        __WFI();
    }

    return FWK_E_DEVICE;
}

/* Module 'system_power' driver interface */
const struct mod_system_power_driver_api platform_system_pwr_drv_api = {
    .system_shutdown = platform_shutdown,
};

const void *get_platform_system_power_driver_api(void)
{
    return &platform_system_pwr_drv_api;
}

/*
 * SCMI module -> SCP platform module interface
 */
static int platform_system_get_scmi_protocol_id(
    fwk_id_t protocol_id,
    uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = (uint8_t)MOD_SCMI_PROTOCOL_ID_SYS_POWER;

    return FWK_SUCCESS;
}

/*
 * Upon binding the scp_platform module to the SCMI module, the SCMI module
 * will also bind back to the scp_platform module, anticipating the presence of
 * .get_scmi_protocol_id() and .message_handler() APIs.
 *
 * In the current implementation of scp_platform module, only sending SCMI
 * message is implemented, and the scp_platform module is not intended to
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

const void *get_platform_scmi_power_down_api(void)
{
    return &platform_system_scmi_api;
}

int platform_power_mgmt_bind(void)
{
    int status;

    /* Bind to SCMI module for RSS communication */
    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL_REQ),
        &scmi_protocol_req_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return fwk_module_bind(
        fwk_module_id_power_domain,
        mod_pd_api_id_restricted,
        &pd_restricted_api);
}

int init_ap_core(uint8_t core_idx)
{
    bool resp_requested;
    uint32_t pd_state;
    fwk_id_t pd_id;

    pd_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, core_idx);

    /* Notification event at the end of request processing is not required */
    resp_requested = false;

    /* Composite Power Domain state to be set for the AP */
    pd_state = MOD_PD_COMPOSITE_STATE(
        MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON, MOD_PD_STATE_ON, MOD_PD_STATE_ON);

    return pd_restricted_api->set_state(pd_id, resp_requested, pd_state);
}

/* Poweroff all AP cores */
void power_off_all_cores(void)
{
    unsigned int pd_idx;
    unsigned int core_count;
    int status;

    core_count = platform_get_core_count();

    for (pd_idx = 0; pd_idx < core_count; pd_idx++) {
        FWK_LOG_INFO(MOD_NAME "Powering down %s",
            fwk_module_get_element_name(
                FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, pd_idx)));

        status = pd_restricted_api->set_state(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, pd_idx),
            false,
            MOD_PD_COMPOSITE_STATE(MOD_PD_LEVEL_0, 0, 0, 0, MOD_PD_STATE_OFF));

        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME "Power down of %s failed",
                fwk_module_get_element_name(
                    FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, pd_idx)));
        }
        fwk_assert(status == FWK_SUCCESS);
    }
}

/* Check weather all AP cores are powered OFF */
static int check_power_off_all_cores(void)
{
    unsigned int core_count;
    unsigned int power_state;
    unsigned int pd_idx;
    int status = 0;

    core_count = platform_get_core_count();

    /* Check if all the CPU power domain are powered down */
    for (pd_idx = 0; pd_idx < core_count; pd_idx++) {
        status = pd_restricted_api->get_state(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, pd_idx), &power_state);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME "failed to get state of %s",
                fwk_module_get_element_name(
                    FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, pd_idx)));
            return status;
        }

        /* Exit if any core is not powered down */
        if ((power_state &
             (MOD_PD_CS_STATE_MASK << MOD_PD_CS_LEVEL_0_STATE_SHIFT)) !=
            MOD_PD_STATE_OFF) {
            FWK_LOG_INFO(MOD_NAME "%s not yet powered down",
                fwk_module_get_element_name(
                    FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, pd_idx)));
            return FWK_PENDING;
        }
    }
    return status;
}

/*
 * Power ON the primary core only after all AP cores have powered OFF to
 * complete the warm reboot.
 */
int check_and_reboot_ap_cores()
{
    int status;

    /* Event for checking power domain status */
    struct fwk_event_light check_pd_off_event = {
        .id = mod_scp_platform_event_check_ppu_off,
        .target_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCP_PLATFORM),
    };

    status = check_power_off_all_cores();

    if (status != FWK_SUCCESS) {
        /*
         * Increment the retry count. The count is initialized in the warm
         * reset notification handler
         */
        scp_platform_ctx.warm_reset_check_cnt++;
        if (scp_platform_ctx.warm_reset_check_cnt >=
            WARM_RESET_MAX_RETRIES) {
            FWK_LOG_ERR(MOD_NAME "Fail: warm reboot reached maximum retries");
            fwk_assert(
                scp_platform_ctx.warm_reset_check_cnt <
                WARM_RESET_MAX_RETRIES);
        }

        /*
         * Continue monitoring core PPUs until all the core power domains
         * are powered down.
         */
        status = fwk_put_event(&check_pd_off_event);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME "Failed to send event, returned %d", status);
        }
        fwk_assert(status == FWK_SUCCESS);
    } else {
        /*
         * All the CPU power domain are powered off. Start the process to
         * power on the first application core to complete the warm reboot
         * sequence.
         */
        status = init_ap_core(0);
        fwk_assert(status == FWK_SUCCESS);
    }

    return status;
}

/* Issue system wide cold reboot. */
int reboot_system(void)
{
    return pd_restricted_api->system_shutdown(MOD_PD_SYSTEM_COLD_RESET);
}
