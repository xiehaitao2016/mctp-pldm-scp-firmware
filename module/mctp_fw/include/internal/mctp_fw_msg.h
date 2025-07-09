/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MCTP_FW_MSG_H
#define MCTP_FW_MSG_H

#include <libmctp-cmds.h>

#include <mod_mctp.h>

#define MCTP_GET_ENDPOINT_ID_RESP_BYTES      4
#define MCTP_SET_ENDPOINT_ID_RESP_BYTES      4
#define MCTP_GET_MESSAGE_TYPE_MIN_RESP_BYTES 2
#define MCTP_GET_ENDPOINT_UUID_RESP_BYTES    17

struct mctp_ctrl_msg_hdr_t {
    uint8_t ic_msg_type;
    uint8_t rq_dgram_inst;
    uint8_t command_code;
    uint8_t data[];
};

int mctp_ctrl_cmd_handle(
    uint32_t bus,
    uint8_t *tx_buf,
    mctp_eid_t src,
    void *buffer,
    size_t length);

#endif /* MCTP_FW_MSG_H */
