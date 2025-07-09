/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
 */

#ifndef MCTP_H
#define MCTP_H

#include <libmctp-alloc.h>
#include <libmctp-serial.h>
#include <libmctp-pcc.h>
#include <libmctp.h>

#include <fwk_id.h>

#include <stddef.h>

/* typedefs for common exposed mctp types */
typedef uint8_t mctp_eid_t;
typedef struct mctp mctp_t;
typedef struct mctp_bus mctp_bus_t;
typedef struct mctp_binding mctp_binding_t;

/* MCTP api index */
enum mod_mctp_api_idx {
    MCTP_BIND_REQ_API_IDX,
    MCTP_BIND_REQ_API_IDX_COUNT,
};

/* MCTP packet definitions */
typedef struct mctp_hdr mctp_hdr_t;

/* MCTP pktbuf definition */
typedef struct mctp_pktbuf mctp_pktbuf_t;

/* mctp bus type */
typedef struct mctp_bus mctp_bus_t;

/* hardware bindings */
typedef struct mctp_binding mctp_binding_t;

/* mctp serial type definition */
typedef struct mctp_binding_serial mctp_binding_serial_t;

/* mctp pcc type definition */
typedef struct mctp_binding_pcc mctp_binding_pcc_t;

/* mctp api type */
typedef struct mctp_api {
    /* mctp base apis */
    mctp_t *(*mctp_ctx_init)(void);
    int (*mctp_set_rx_all)(mctp_t *mctp, mctp_rx_fn fn, void *data);
    int (*mctp_register_bus)(
        mctp_t *mctp,
        mctp_binding_t *binding,
        mctp_eid_t eid);
    void (*mctp_unregister_bus)(mctp_t *mctp, mctp_binding_t *binding);

    void (*mctp_bus_rx)(mctp_binding_t *binding, mctp_pktbuf_t *pkt);
    void (*mctp_binding_set_tx_enabled)(mctp_binding_t *binding, bool enable);

    int (*mctp_message_tx)(
        mctp_t *mctp,
        mctp_eid_t eid,
        bool tag_owner,
        uint8_t msg_tag,
        void *msg,
        size_t msg_len);

    /* mctp binding serial apis */
    mctp_binding_serial_t *(*mctp_serial_init)(void);
    void (*mctp_serial_destroy)(mctp_binding_serial_t *serial);
    mctp_binding_t *(*mctp_binding_serial_core)(mctp_binding_serial_t *b);
    void (*mctp_serial_set_tx_fn)(
        mctp_binding_serial_t *serial,
        mctp_serial_tx_fn fn,
        void *data);
    int (*mctp_serial_rx)(
        mctp_binding_serial_t *serial,
        const void *buf,
        size_t len);

    /* mctp binding pcc apis */
    mctp_binding_pcc_t *(*mctp_pcc_init)(void);
    void (*mctp_pcc_destroy)(mctp_binding_pcc_t *pcc);
    mctp_binding_t *(*mctp_binding_pcc_core)(mctp_binding_pcc_t *b);
    void (*mctp_pcc_set_tx_fn)(
        mctp_binding_pcc_t *pcc, mctp_pcc_tx_fn fn, void *data);
    int (*mctp_pcc_rx)(
        mctp_binding_pcc_t *pcc, const void *buf, size_t len);
} mctp_api_t;

/* Dummy function to make libmctp compile success */
size_t write(int fd, void *buf, size_t len);

#endif /* _MCTP_H */
