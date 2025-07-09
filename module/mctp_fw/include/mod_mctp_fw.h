/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_MCTP_FW_H
#define MOD_MCTP_FW_H

#include <fwk_id.h>

#include <stddef.h>

#define PLDM_MSG_TYPE 1

/*
 * mctp_fw binds with the following modules inorder to use thier apis and other
 * types
 */
enum {
#ifdef BUILD_HAS_MOD_MCTP_SERIAL
    MCTP_FW_BIND_SERIAL_API_IDX,
#endif

#ifdef BUILD_HAS_MOD_MCTP_PCC
    MCTP_FW_BIND_PCC_API_IDX,
#endif
    MCTP_FW_BIND_API_IDX_COUNT,
};

/* ids representing bind requests supported by mctp_fw*/
enum {
    MCTP_FW_BIND_REQ_API_IDX,
    MCTP_FW_BIND_REQ_API_IDX_COUNT,
};

/*
 * each element we bind to from mctp_fw would have this type for holding fwk ids
 */
typedef struct mctp_fw_elem_config {
    fwk_id_t driver_id;
    fwk_id_t driver_api_id;

    uint8_t local_eid;
    uint8_t remote_eid;
    uint8_t bus_policy;
} mctp_fw_elem_config_t;

/* type for exposing mctp_fw apis to be used by other modules */
typedef struct mctp_fw_api_t {
    void (*mctp_fw_receive_from_app_layer)(
        uint32_t bus, bool tag_owner, uint8_t msg_tag,
        void *data, void *msg, size_t len);
} mctp_fw_api_t;

typedef struct mod_mctp_fw_config {
    mctp_fw_elem_config_t mctp_driver_api;
    mctp_fw_elem_config_t pldm_fw_driver_api;
} mod_mctp_fw_config_t;

#endif
