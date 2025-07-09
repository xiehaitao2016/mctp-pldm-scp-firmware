/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_MCTP_FW_INT_H
#define MOD_MCTP_FW_INT_H

#include <mod_mctp.h>
#include <mod_mctp_pcc.h>
#include <mod_mctp_fw.h>
#include <mod_mctp_serial.h>
#include <mod_pldm_fw.h>

#define MOD_NAME       "[MCTPD]: "
#define TX_BUFFER_SIZE 256

/* holds types specific to mctp_pcc module */
typedef struct pcc_elem_ctx {
    mctp_t *mctp;
    mctp_pcc_api_t *pcc_api;
    mctp_binding_pcc_t *pcc_mctp_binding;
} mctp_pcc_elem_ctx_t;

/* holds types specific to mctp_serial module */
typedef struct serial_elem_ctx {
    mctp_t *mctp;
    mctp_serial_api_t *serial_api;
    mctp_binding_serial_t *serial_mctp_binding;
} mctp_serial_elem_ctx_t;

/* holds types specific to mctp module */
typedef struct mctp_elem_ctx {
    mctp_api_t *mctp_api;
} mctp_elem_ctx_t;

/* holds types specific to pldm fw */
typedef struct pldm_fw_elem_ctx {
    pldm_fw_api_t *pldm_fw_api;
} pldm_fw_elem_ctx_t;

/* mctp_fw elem ctx */
typedef struct mctp_fw_elem_ctx {
    fwk_id_t id;
    mctp_fw_elem_config_t *mctp_fw_config;

    uint8_t local_eid;
    uint8_t remote_eid;
    uint32_t bus;
    uint8_t bus_policy;
    // TODO: routing table

    union {
        mctp_serial_elem_ctx_t serial_elem;
        mctp_pcc_elem_ctx_t pcc_elem;
    } mctp_fw_elem;

} mctp_fw_elem_ctx_t;

/* mctp_fw ctx */
typedef struct mctp_fw_ctx {
    /* Number of channels */
    unsigned int elem_count;
    mctp_fw_elem_ctx_t *elem_ctx_table;

    /* MCTP core API */
    mctp_fw_elem_config_t *mctp_api_config;
    mctp_api_t *mctp_api;

    /* PLDM API */
    mctp_fw_elem_config_t *pldm_fw_api_config;
    pldm_fw_api_t *pldm_fw_api;

    uint8_t txbuf[TX_BUFFER_SIZE];
} mctp_fw_ctx_t;

#endif /* MOD_MCTP_FW_INT_H */
