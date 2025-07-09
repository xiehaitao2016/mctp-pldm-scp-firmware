/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCP platform sub-system initialization support.
 */

#include "core_manager.h"
#include "platform_core.h"

#include <internal/scp_platform.h>

#include <mod_power_domain.h>
#include <mod_scp_platform.h>
#include <mod_system_info.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

/* Module context data */
struct scp_platform_ctx scp_platform_ctx;

#ifdef AP_RESET_TO_BL31

/* SRAM address where TF-A BL31 binary will be preloaded in the FVP */
#    define ARM_TF_BL31_SRAM_ADDR (0x00063000)

/* Helper function to program the AP Core RVBAR */
void program_ap_rvbar()
{
    uint8_t core_idx;

    for (core_idx = 0; core_idx < platform_get_core_count(); core_idx++) {
        /* Set RVBAR to TF-A BL31 DRAM address */
        SCP_CLUSTER_UTILITY_CORE_MANAGER_PTR(core_idx)->PE_RVBARADDR_LW =
            ARM_TF_BL31_SRAM_ADDR;
        SCP_CLUSTER_UTILITY_CORE_MANAGER_PTR(core_idx)->PE_RVBARADDR_UP = 0;
    }
}

#endif /* AP_RESET_TO_BL31 */

static void platform_update_gpt_size(void)
{
    unsigned int n;
    volatile uint32_t *pe_static_config;

    /* Update L0GPTSZ for all cores */
    for (n = 0; n < platform_get_core_count(); n++) {
        pe_static_config =
            (volatile uint32_t *)(&SCP_CLUSTER_UTILITY_CORE_MANAGER_PTR(n)
                                       ->PE_STATIC_CONFIG);
        *pe_static_config |= (0x4 << 9);
    }
}

/*
 * Framework handlers
 */
static int scp_platform_mod_init(
    fwk_id_t module_id,
    unsigned int unused,
    const void *data)
{
    const struct mod_scp_platform_config *config;

    config = (struct mod_scp_platform_config *)data;

    if (!fwk_id_type_is_valid(config->timer_id) ||
        !fwk_id_type_is_valid(config->transport_id)) {
        return FWK_E_DATA;
    }

    /* Save the config data in the module context */
    scp_platform_ctx.config = config;

    return FWK_SUCCESS;
}

static int scp_platform_bind(fwk_id_t id, unsigned int round)
{
    int status;
    fwk_id_t mod_system_info_api_id_get_info;

    if (round > 0) {
        return FWK_SUCCESS;
    }

    /* Bind to modules required to handshake with RSS */
    status = platform_rss_bind(scp_platform_ctx.config);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /* Bind to modules required for power management */
    status = platform_power_mgmt_bind();
    if (status != FWK_SUCCESS) {
        return status;
    }

    mod_system_info_api_id_get_info =
        FWK_ID_API(FWK_MODULE_IDX_SYSTEM_INFO, MOD_SYSTEM_INFO_GET_API_IDX);

    /* Bind to System Info HAL API */
    return fwk_module_bind(
        fwk_module_id_system_info,
        mod_system_info_api_id_get_info,
        &scp_platform_ctx.system_info_api);
}

static int scp_platform_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    int status;
    enum mod_scp_platform_api_idx api_id_type;

    api_id_type = (enum mod_scp_platform_api_idx)fwk_id_get_api_idx(api_id);

    switch (api_id_type) {
    case MOD_SCP_PLATFORM_API_IDX_SCMI_POWER_DOWN:
        *api = get_platform_scmi_power_down_api();
        status = FWK_SUCCESS;
        break;

    case MOD_SCP_PLATFORM_API_IDX_SYSTEM_POWER_DRIVER:
        *api = get_platform_system_power_driver_api();
        status = FWK_SUCCESS;
        break;

    case MOD_SCP_PLATFORM_API_IDX_TRANSPORT_SIGNAL:
        *api = get_platform_transport_signal_api();
        status = FWK_SUCCESS;
        break;

    default:
        status = FWK_E_PARAM;
    }

    return status;
}

static int scp_platform_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp)
{
    int status;

    switch (fwk_id_get_event_idx(event->id)) {
    case MOD_SCP_PLATFORM_CHECK_PD_OFF:
        status = check_and_reboot_ap_cores();
        fwk_assert(status == FWK_SUCCESS);

        break; /* MOD_SCP_PLATFORM_CHECK_PD_OFF */
    default:
        FWK_LOG_WARN(MOD_NAME "unrecognized event received, event ignored\n");
        status = FWK_E_PARAM;
    }

    return status;
}

int scp_platform_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status;

    /* Event for checking power domain status */
    struct fwk_event_light check_pd_off_event = {
        .id = mod_scp_platform_event_check_ppu_off,
        .target_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCP_PLATFORM),
    };

    fwk_assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_MODULE));

    if (fwk_id_is_equal(event->id, mod_pd_notification_id_pre_warm_reset)) {
        /*
         * Notification handler for warm reset.
         *
         * On receiving warm reset request, the power domain (PD) module will
         * issue the warm reset notification. The notification is handled by
         * the scp platform module. The notification handler requests PD to
         * power off all the CPU cores through the HAL API exposed by PD. After
         * requesting for power off for all the cores, the platform system
         * module will send an event to itself to check whether all core power
         * domains are powered off. After all the core power domains are powered
         * off, the event handler will then power up the boot CPU.
         */
        power_off_all_cores();

        scp_platform_ctx.warm_reset_check_cnt = 0;
        status = fwk_put_event(&check_pd_off_event);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME "PD power off check event failed with %d",
                status);
            FWK_LOG_ERR(MOD_NAME "Issuing cold reboot to recover.");
            status = reboot_system();
            fwk_assert(status == FWK_SUCCESS);
        }
    } else {
        status = FWK_E_PARAM;
    }

    return status;
}

static int scp_platform_start(fwk_id_t id)
{
    int status;
    const struct mod_system_info *system_info;
    struct fwk_event event = { 0 };
    unsigned int event_count;

    /* Notify RSS that SYSTOP is powered up and wait for RSS doorbell */
    status = notify_rss_and_wait_for_response();
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Error! SCP-RSS handshake failed");
        return FWK_E_PANIC;
    }

    /* SCP subsystem initialization completion notification */
    event.id = mod_scp_platform_notification_subsys_init;
    event.source_id = id;

    /*
     * RSS has now setup GPC bypass in the system control block. Notify other
     * modules that are waiting to access memory regions outside the SCP
     * subsystem (which otherwise would have generated a fault).
     */
    status = fwk_notification_notify(&event, &event_count);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Error! Subsystem init notification failed");
        return FWK_E_PANIC;
    }

    /* Configure LCP0 UART access and release all LCPs */
    status = platform_setup_lcp();
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Error! LCP setup failed");
        return FWK_E_PANIC;
    }

    /* Update L0GPTSZ for all cores */
    platform_update_gpt_size();

#ifdef AP_RESET_TO_BL31

    /* Set the AP RVBAR to DRAM address where the TF-A BL31 binary is preloaded
     */
    program_ap_rvbar();

#endif /* AP_RESET_TO_BL31 */

    /* Determine the chip information */
    status = scp_platform_ctx.system_info_api->get_system_info(&system_info);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Error! Failed to obtain system info");
        return FWK_E_PANIC;
    }

    if (system_info->chip_id != 0) {
        /* Nothing to be done for secondary chips */
        return FWK_SUCCESS;
    }

    FWK_LOG_INFO(MOD_NAME "Initializing the primary core...");

    status = init_ap_core(0);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Error! Failed to initialize primary core");
        fwk_trap();
    }

    /* Subscribe to warm reset notifications */
    status = fwk_notification_subscribe(
        mod_pd_notification_id_pre_warm_reset,
        FWK_ID_MODULE(FWK_MODULE_IDX_POWER_DOMAIN),
        id);
    if (status != FWK_SUCCESS) {
        FWK_LOG_WARN(MOD_NAME "failed to subscribe to warm reset notification");
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_scp_platform = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_SCP_PLATFORM_API_COUNT,
    .notification_count = MOD_SCP_PLATFORM_NOTIFICATION_COUNT,
    .event_count = (unsigned int)MOD_SCP_PLATFORM_EVENT_COUNT,
    .init = scp_platform_mod_init,
    .bind = scp_platform_bind,
    .process_bind_request = scp_platform_process_bind_request,
    .process_event = scp_platform_process_event,
    .process_notification = scp_platform_process_notification,
    .start = scp_platform_start,
};
