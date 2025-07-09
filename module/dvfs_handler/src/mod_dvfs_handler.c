/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/dvfs_handler_reg.h>
#include <lcp_mhu3.h>

#include <mod_dvfs_handler.h>
#include <mod_transport.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#define MOD_NAME     "[DVFS HANDLER] "
#define EID          "[EIDX:%u] "
#define MOD_NAME_EID MOD_NAME EID
#define MAX_RETRY    10

/* Module element context */
struct dvfs_handler_element_ctx {
    /* Index of the element representing a DVFS domain */
    fwk_id_t element_idx;

    /* Module configuration data */
    struct mod_dvfs_handler_element_config *config;

    /* Current perf level */
    unsigned int current_level;

    /* Number of operating points */
    size_t opp_count;

    /* Transport fastchannel API */
    struct mod_transport_fast_channels_api *transport_fch_api;
};

/* Pointer to array of module element context data */
struct dvfs_handler_element_ctx *element_ctx;

/*
 * Update the DVFS handler 'policy frequency' register with the new frequency
 * in Hz.
 */
static inline void set_frequency(unsigned int eidx, unsigned int freq)
{
    struct dvfs_frame *dvfs_frame =
        (struct dvfs_frame *)element_ctx[eidx].config->dvfs_frame_addr;

    if (dvfs_frame->policy_frequency_status != freq) {
        dvfs_frame->policy_frequency = freq;
        FWK_LOG_INFO(MOD_NAME_EID "setting frequency to %uHz", eidx, freq);
    }
}

/*
 * Update the DVFS handler 'policy voltage' register with the new voltage
 * in micro-volts.
 */
static inline void set_voltage(unsigned int eidx, unsigned int vlt)
{
    struct dvfs_frame *dvfs_frame =
        (struct dvfs_frame *)element_ctx[eidx].config->dvfs_frame_addr;

    if (dvfs_frame->policy_voltage_status != vlt) {
        dvfs_frame->policy_voltage = vlt;
        FWK_LOG_INFO(MOD_NAME_EID "setting voltage to %uuV", eidx, vlt);
    }
}

/*
 * Check whether the frequency and voltage settings are reflected in status
 * register.
 */
static int check_dvfs_status(
    unsigned int eidx,
    unsigned int freq,
    unsigned int vlt)
{
    struct dvfs_frame *dvfs_frame =
        (struct dvfs_frame *)element_ctx[eidx].config->dvfs_frame_addr;

    if (dvfs_frame->policy_frequency_status != freq) {
        FWK_LOG_CRIT(
            MOD_NAME_EID "failed setting frequency to %uHz", eidx, freq);
        return FWK_PENDING;
    }

    if (dvfs_frame->policy_voltage_status != vlt) {
        FWK_LOG_CRIT(MOD_NAME_EID "failed setting voltage to %uuV", eidx, vlt);
        return FWK_PENDING;
    }

    return FWK_SUCCESS;
}

/*
 * Ensure OPP table is sorted in ascending order. Count the number of OPP
 * entries in the OPP table.
 */
static uint32_t validate_and_count_opps(const struct mod_dvfs_handler_opp *opps)
{
    const struct mod_dvfs_handler_opp *opp = &opps[0];
    unsigned int prev_opp_level = 0;
    uint32_t count = 0;

    while (opp->level != 0) {
        if ((opp->voltage == 0) || (opp->frequency == 0) ||
            (opp->level <= prev_opp_level)) {
            return 0;
        }
        prev_opp_level = opp->level;
        opp++;
        count++;
    }

    return count;
}

/*
 * Get the OPP table entry corresponding to the requested performance level.
 */
static const struct mod_dvfs_handler_opp *get_opp_for_level(
    unsigned int eidx,
    unsigned int level)
{
    struct dvfs_handler_element_ctx *ctx = &element_ctx[eidx];
    struct mod_dvfs_handler_opp *opp_table;
    unsigned int opp_idx;

    opp_table = ctx->config->opps;

    /*
     * Find an OPP level that is equal to or the nearest lower
     * OPP level that is supported.
     */

    if (level <= opp_table[0].level) {
        return &opp_table[0];
    }

    for (opp_idx = 1; opp_idx < ctx->opp_count; opp_idx++) {
        /*
         * The OPP level requested is not present in OPP table. Return
         * a valid OPP table entry just below the requested level.
         */
        if ((level < opp_table[opp_idx].level) &&
            (level >= opp_table[opp_idx - 1].level))
            return &opp_table[opp_idx - 1];
    }

    /*
     * Requested level is equal or higher than supported highest perf level.
     * So return the highest supported perf level.
     */
    return &opp_table[ctx->opp_count - 1];
}

/* Set a requested perf level */
static int dvfs_handler_set_level(unsigned int eidx, unsigned int level)
{
    struct dvfs_handler_element_ctx *ctx = &element_ctx[eidx];
    const struct mod_dvfs_handler_opp *new_opp;
    uint8_t retry = MAX_RETRY;
    int status;

    FWK_LOG_INFO(MOD_NAME_EID "requested level: %u", eidx, level);

    if (ctx->current_level == level) {
        FWK_LOG_INFO(
            MOD_NAME_EID
            "active level same as requested level, "
            "ignoring request",
            eidx);
        return FWK_SUCCESS;
    }

    /* Find an OPP for the performance level requested */
    new_opp = get_opp_for_level(eidx, level);
    if (new_opp == NULL) {
        FWK_LOG_ERR(MOD_NAME_EID "invalid level %u requested", eidx, level);
        return FWK_E_RANGE;
    } else if (new_opp->level == ctx->current_level) {
        FWK_LOG_INFO(
            MOD_NAME_EID
            "found opp that is same as active opp, "
            "ignoring request",
            eidx);
        return FWK_SUCCESS;
    }

    /* Program the hardware registers with frequency and voltage values */
    set_frequency(eidx, new_opp->frequency);
    set_voltage(eidx, new_opp->voltage);

    /*
     * Check the status registers to determine whether the frequency and
     * voltage updates are reflected.
     */
    do {
        status = check_dvfs_status(eidx, new_opp->frequency, new_opp->voltage);
        if (status == FWK_SUCCESS) {
            ctx->current_level = new_opp->level;
            FWK_LOG_INFO(
                MOD_NAME_EID "perf level set to %u", eidx, new_opp->level);
            return FWK_SUCCESS;
        }
    } while (--retry);

    FWK_LOG_ERR(MOD_NAME_EID "failed to set requested level %u", eidx, level);
    return status;
}

/*
 * Callback function registered with transport module and there by with MHU3
 * module. This function will be invoked by the MHU3 module on receiving a
 * data over fastchannel with performance level set request.
 */
static void dvfs_handler_fch_set_level_callback(uintptr_t dvfs_id)
{
    struct mod_transport_fast_channel_addr fch;
    struct dvfs_handler_element_ctx *ctx;
    fwk_id_t id = *(fwk_id_t *)dvfs_id;
    FWK_R unsigned int *perf_level;
    unsigned int eidx;
    int status;

    eidx = fwk_id_get_element_idx(id);
    ctx = &element_ctx[eidx];

    status = ctx->transport_fch_api->transport_get_fch_address(
        ctx->config->dvfs_fch_set_level, &fch);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            MOD_NAME_EID
            "failed to get fastchannel address required "
            "to find the new perf level requested, status: %d",
            eidx,
            status);
        return;
    }

    perf_level = (FWK_R unsigned int *)fch.local_view_address;
    status = dvfs_handler_set_level(eidx, *perf_level);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            MOD_NAME_EID "failed to set level %u, status: %d",
            eidx,
            *perf_level,
            status);
    }
}

/* Module framework handlers */

static int dvfs_handler_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    element_ctx =
        fwk_mm_calloc(element_count, sizeof(struct dvfs_handler_element_ctx));
    if (element_ctx == NULL) {
        FWK_LOG_CRIT(MOD_NAME "failed to allocate memory for module context");
        return FWK_E_NOMEM;
    }

    return FWK_SUCCESS;
}

static int dvfs_handler_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    unsigned int eidx = fwk_id_get_element_idx(element_id);
    struct dvfs_handler_element_ctx *ctx = &element_ctx[eidx];

    ctx->element_idx = element_id;
    ctx->config = (struct mod_dvfs_handler_element_config *)data;

    if (ctx->config->opps == NULL) {
        return FWK_E_PARAM;
    }

    ctx->opp_count = validate_and_count_opps(ctx->config->opps);
    if (ctx->opp_count == 0) {
        FWK_LOG_ERR(MOD_NAME_EID "invalid OPP table found", eidx);
        return FWK_E_PARAM;
    }

    FWK_LOG_INFO(
        MOD_NAME_EID "found %d supported operating points",
        eidx,
        ctx->opp_count);
    return FWK_SUCCESS;
}

static int dvfs_handler_bind(fwk_id_t id, unsigned int round)
{
    unsigned int eidx;
    int status;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_SUCCESS;
    }

    eidx = fwk_id_get_element_idx(id);

    /* Bind to transport channel */
    status = fwk_module_bind(
        FWK_ID_ELEMENT(
            FWK_MODULE_IDX_TRANSPORT, MHU3_DEVICE_IDX_LCP_AP_FCH_DVFS_SET_LVL),
        FWK_ID_API(
            FWK_MODULE_IDX_TRANSPORT, MOD_TRANSPORT_API_IDX_FAST_CHANNELS),
        &element_ctx[eidx].transport_fch_api);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            MOD_NAME_EID "bind with transport failed, status: %d",
            eidx,
            status);
        return status;
    }

    return FWK_SUCCESS;
}

static int dvfs_handler_start(fwk_id_t id)
{
    struct dvfs_handler_element_ctx *ctx;
    struct mod_dvfs_handler_opp *opp_table;
    unsigned sustain_level;
    unsigned int eidx;
    int status;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_SUCCESS;
    }

    eidx = fwk_id_get_element_idx(id);
    ctx = &element_ctx[eidx];
    opp_table = ctx->config->opps;
    sustain_level = opp_table[ctx->config->sustained_idx].level;

    /* Start by setting the sustained level */
    status = dvfs_handler_set_level(eidx, sustain_level);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            MOD_NAME_EID
            "failed to set sustained perf level, "
            "status: %d",
            eidx,
            status);
        return status;
    }

    /* Register callback function for performance level set */
    status = ctx->transport_fch_api->transport_fch_register_callback(
        ctx->config->dvfs_fch_set_level,
        (uintptr_t)&ctx->element_idx,
        &dvfs_handler_fch_set_level_callback);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            MOD_NAME_EID
            "failed to register 'set-level' fastchannel "
            " callback, status: %d",
            eidx,
            status);
        return status;
    }

    return FWK_SUCCESS;
}

/* Module description */
const struct fwk_module module_dvfs_handler = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = dvfs_handler_init,
    .element_init = dvfs_handler_element_init,
    .bind = dvfs_handler_bind,
    .start = dvfs_handler_start,
};
