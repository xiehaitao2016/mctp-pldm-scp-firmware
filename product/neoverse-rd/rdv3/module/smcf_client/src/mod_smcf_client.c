/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform_core.h"

#include <scp_cfgd_timer.h>
#include <scp_cfgd_transport.h>
#include <mod_amu_smcf_drv.h>
#include <mod_platform_smcf.h>
#include <mod_smcf.h>
#include <mod_smcf_client.h>
#include <mod_timer.h>
#include <mod_transport.h>

#include <interface_amu.h>

#include <interface_amu.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <string.h>

#define NUM_OF_CORE_COUNTERS 7
#define MOD_NAME "[SMCF_CLIENT] "
#define MODE_CONFIG_VALUE UINT32_C(0x51007000)
#define MODE_REQ_INDEX UINT8_C(2)

static unsigned int core_count;

static struct mod_timer_alarm_api *alarm_api;
static struct smcf_data_api *platform_smpling_api;
static struct smcf_control_api *control_api;
static struct smcf_monitor_group_interrupt_api *interrupt_api;
static struct amu_api *amu_data_api;
static struct mod_transport_firmware_api *transport_api;
static uint32_t amu_sensor_count = 0;

fwk_id_t alarm_id = FWK_ID_SUB_ELEMENT_INIT(
    FWK_MODULE_IDX_TIMER,
    0,
    SCP_CFGD_MOD_TIMER_SEIDX_SMCF_CLIENT_TIMER);

static int smcf_client_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    core_count = platform_get_core_count();
    /* one sensor and one amu per core */
    amu_sensor_count =
        fwk_module_get_element_count(FWK_ID_MODULE(FWK_MODULE_IDX_SMCF));

    return FWK_SUCCESS;
}

static void alarm_callback(uintptr_t module_idx)
{
    uint32_t mgi;
    uint32_t num_of_mgis = core_count; // Each core has 2 MGI (AMU, DTS)

    for (mgi = 0; mgi < num_of_mgis; mgi++) {
        platform_smpling_api->start_data_sampling(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_SMCF, mgi));
    }

    for (mgi = 0; mgi < num_of_mgis; mgi++) {
        interrupt_api->handle_interrupt(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_SMCF, mgi));
    }

    for (mgi = 0; mgi < num_of_mgis; mgi++) {
        platform_smpling_api->stop_data_sampling(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_SMCF, mgi));
    }
}

static int smcf_client_bind(fwk_id_t id, unsigned int round)
{
    int status;
    uint32_t mgi_idx;

    /* Only bind in the first round of calls */
    if (round > 0)
        return FWK_SUCCESS;

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_PLATFORM_SMCF),
        FWK_ID_API(
            FWK_MODULE_IDX_PLATFORM_SMCF,
            MOD_SMCF_PLATFORM_API_IDX_SAMPLING_API),
        &platform_smpling_api);

    if (status != FWK_SUCCESS) {
        return status;
    }

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_SMCF),
        FWK_ID_API(FWK_MODULE_IDX_SMCF, MOD_SMCF_API_IDX_CONTROL),
        &control_api);

    if (status != FWK_SUCCESS) {
        return status;
    }

    for (mgi_idx = 0; mgi_idx < amu_sensor_count; mgi_idx++) {
        status = fwk_module_bind(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_SMCF, mgi_idx),
            FWK_ID_API(FWK_MODULE_IDX_SMCF, MOD_SMCF_API_IDX_INTERRUPT),
            &interrupt_api);

        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_AMU_SMCF_DRV),
        FWK_ID_API(FWK_MODULE_IDX_AMU_SMCF_DRV, MOD_AMU_SMCF_DRV_API_IDX_DATA),
        &amu_data_api);

    if (status != FWK_SUCCESS) {
        return status;
    }

    status = fwk_module_bind(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TRANSPORT, SCP_CFGD_MOD_TRANSPORT_EIDX_SMCF_0),
        FWK_ID_API(FWK_MODULE_IDX_TRANSPORT, MOD_TRANSPORT_API_IDX_FIRMWARE),
        &transport_api);

    if (status != FWK_SUCCESS) {
        return status;
    }

    return fwk_module_bind(alarm_id, MOD_TIMER_API_ID_ALARM, &alarm_api);
}


/*
 * Transport signal API implementation
 */
static int signal_error(fwk_id_t unused)
{
    FWK_LOG_INFO(MOD_NAME "Error: message not delivered for AP-SCP NS MHU\n");

    return FWK_SUCCESS;
}

static int signal_message(fwk_id_t unused)
{
    alarm_callback(fwk_id_get_module_idx(unused));
    return FWK_SUCCESS;
}

const struct mod_transport_firmware_signal_api transport_signal_api = {
    .signal_error = signal_error,
    .signal_message = signal_message,
};

static int smcf_client_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t pd_id,
    fwk_id_t api_id,
    const void **api)
{
    enum mod_smcf_client_api_idx api_id_type;

    api_id_type = (enum mod_smcf_client_api_idx)fwk_id_get_api_idx(api_id);

    switch (api_id_type) {
    case MOD_SMCF_CLIENT_API_IDX_TRANSPORT_SIGNAL:
        *api = &transport_signal_api;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int smcf_client_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status = FWK_SUCCESS;
    unsigned int cntr_idx, mgi_idx;
    uint64_t counter_value[NUM_OF_CORE_COUNTERS];

    if (!fwk_id_is_equal(
            event->id, mod_smcf_notification_id_new_data_sample_ready)) {
        return FWK_E_BUSY;
    }

    mgi_idx = fwk_id_get_element_idx(event->source_id);

    FWK_LOG_INFO(MOD_NAME "Data successfully fetched for MGI[%u]", mgi_idx);

    if (mgi_idx < core_count) { // AMUs

        memset(counter_value, 0, 2 * NUM_OF_CORE_COUNTERS);
        status = amu_data_api->get_counters(
            FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_SMCF, mgi_idx, 0),
            counter_value,
            NUM_OF_CORE_COUNTERS);

        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME
                "Failed to get amu data for core %u ERROR %d", mgi_idx, status);
            return status;
        }
        for (cntr_idx = 0; cntr_idx < NUM_OF_CORE_COUNTERS; ++cntr_idx)
            FWK_LOG_INFO(MOD_NAME
                "MGI[%u] AMU_COUNTER[%u] data = %llu",
                mgi_idx,
                cntr_idx,
                counter_value[cntr_idx]);
    }
    return status;
}

static int smcf_client_start(fwk_id_t id)
{
    uint32_t mgi, num_of_mgis;

    if (!fwk_module_is_valid_module_id(id)) {
        return FWK_SUCCESS;
    }

    num_of_mgis = core_count;
    for (mgi = 0; mgi < num_of_mgis; mgi++) {
        fwk_notification_subscribe(
            mod_smcf_notification_id_new_data_sample_ready,
            FWK_ID_ELEMENT(FWK_MODULE_IDX_SMCF, mgi),
            id);
	/* Disabling the set monitor mode*/
	/*
	 * control_api->config_mode( (FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_SMCF, mgi, 0)),MODE_CONFIG_VALUE,MODE_REQ_INDEX);
	 */
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_smcf_client = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = MOD_SMCF_CLIENT_API_COUNT,
    .init = smcf_client_init,
    .bind = smcf_client_bind,
    .process_bind_request = smcf_client_process_bind_request,
    .start = smcf_client_start,
    .process_notification = smcf_client_process_notification,
};
const struct fwk_module_config config_smcf_client = { 0 };
