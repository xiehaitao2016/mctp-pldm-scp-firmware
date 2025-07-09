/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'scmi'.
 */

#include "mcp_cfgd_scmi.h"
#include "mcp_cfgd_transport.h"

#include <mod_scmi.h>
#include <mod_transport.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

/* Module 'scmi' element count */
#define MOD_SCMI_ELEMENT_COUNT (MCP_CFGD_MOD_SCMI_EIDX_COUNT + 1)

static const struct fwk_element service_table[MOD_SCMI_ELEMENT_COUNT] = {
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    [MCP_CFGD_MOD_SCMI_EIDX_PSCI_SEND] = {
        .name = "SERVICE_SCP_SCMI_NOTIFICATION",
        .data = &((struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MCP_CFGD_MOD_TRANSPORT_EIDX_SYSTEM_POWER_DOWN_SEND),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT),
            .transport_notification_init_id = FWK_ID_NOTIFICATION_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_NOTIFICATION_IDX_INITIALIZED),
            .scmi_agent_id = MCP_SCMI_AGENT_IDX_PSCI,
            .scmi_p2a_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_SCMI,
                MCP_CFGD_MOD_SCMI_EIDX_PSCI_RECV),
        }),
    },
#endif
    [MCP_CFGD_MOD_SCMI_EIDX_PSCI_RECV] = {
        .name = "SERVICE_SCP",
        .data = &((struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MCP_CFGD_MOD_TRANSPORT_EIDX_SYSTEM_POWER_DOWN_RECV),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT),
            .transport_notification_init_id = FWK_ID_NOTIFICATION_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_NOTIFICATION_IDX_INITIALIZED),
            .scmi_agent_id = MCP_SCMI_AGENT_IDX_PSCI,
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
            .scmi_p2a_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_SCMI,
                MCP_CFGD_MOD_SCMI_EIDX_PSCI_SEND),
#else
            .scmi_p2a_id = FWK_ID_NONE_INIT,
#endif
        }),
    },
    [MCP_CFGD_MOD_SCMI_EIDX_COUNT] = { 0 }
};

static const struct fwk_element *get_service_table(fwk_id_t module_id)
{
    return service_table;
}

static struct mod_scmi_agent agent_table[MCP_SCMI_AGENT_IDX_COUNT] = {
    [MCP_SCMI_AGENT_IDX_PSCI] = {
        .type = SCMI_AGENT_TYPE_PSCI,
        .name = "PSCI",
    },
};

const struct fwk_module_config config_scmi = {
    .data =
        &(struct mod_scmi_config){
            .protocol_count_max = 1,
            .protocol_requester_count_max = 2,
            .agent_count = FWK_ARRAY_SIZE(agent_table) - 1,
            .agent_table = agent_table,
            .vendor_identifier = "arm",
            .sub_vendor_identifier = "arm",
        },

    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_service_table),
};
