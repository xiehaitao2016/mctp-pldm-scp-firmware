/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_PLDM_FW_H
#define MOD_PLDM_FW_H

#include <fwk_id.h>

enum {
    PLDM_FW_BIND_BASE_API_IDX,
    PLDM_FW_BIND_MCTP_FW_API_IDX,
    PLDM_FW_BIND_PLATFORM_API_IDX,
    PLDM_FW_BIND_PDR_API_IDX,
    PLDM_FW_BIND_FRU_API_IDX,
    PLDM_FW_BIND_UTILS_API_IDX,
    PLDM_FW_BIND_API_IDX_COUNT,
};

enum mod_pldm_fw_event_idx {
    PLDM_FW_EVENT_IDX_PLDM_EVENT_MESSAGE,
    PLDM_FW_EVENT_IDX_COUNT,
};

enum {
    PLDM_FW_BIND_REQ_API_IDX,
    PLDM_FW_BIND_REQ_API_IDX_COUNT,
};

typedef struct pldm_fw_elem_config {
    fwk_id_t driver_id;
    fwk_id_t driver_api_id;
} pldm_fw_elem_config_t;

typedef struct pldm_fw_fw_api {
    void (*pldm_fw_receive_from_transport_layer)(
        void *data,
        void *pldm_packet,
        size_t len);
} pldm_fw_api_t;

#endif
