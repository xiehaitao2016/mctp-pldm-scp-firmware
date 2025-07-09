/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_MCTP_SERIAL_H
#define MOD_MCTP_SERIAL_H

#include <mod_mctp.h>

#include <fwk_id.h>

#define PL011_ELEMENT_IDX_MCP_UART 1

#define PL011_ELEMENT_ID_MCP_UART \
    FWK_ID_ELEMENT(FWK_MODULE_IDX_PL011, PL011_ELEMENT_IDX_MCP_UART)

/* mctp serial binds to the following modules*/
enum {
    MCTP_SERIAL_BIND_MCTP_API_IDX,
    MCTP_SERIAL_BIND_API_IDX_COUNT,
};

/* mctp_serial supports the follwoing bind requests */
enum {
    MCTP_SERIAL_BIND_REQ_API_IDX,
    MCTP_SERIAL_BIND_REQ_API_IDX_COUNT,
};

/* basic type for mctp_serial elements */
typedef struct mctp_serial_elem_config {
    fwk_id_t driver_id;
    fwk_id_t driver_api_id;
} mctp_serial_elem_config_t;

/* type for mctp_serial apis exposed by the module */
typedef struct mctp_serial_api {
    mctp_binding_serial_t *(*mctp_serial_register_tx)(void);
} mctp_serial_api_t;

/*!
 * \brief mctp serial module configuration.
 */
struct mod_mctp_serial_module_config {
    /*!
     * Element identifier of the alarm used for polling the UART
     */
    fwk_id_t alarm_id;

    /*!
     * Time period to set for the poll alarm delay (milliseconds)
     */
    uint32_t poll_period;
};

#endif /* MOD_MCTP_SERIAL_H */
