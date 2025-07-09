/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SMCF Client Support
 */

#ifndef MOD_SMCF_CLIENT_H
#define MOD_SMCF_CLIENT_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 *  * \brief Indices of the interfaces exposed by the module.
 *   */
enum mod_smcf_client_api_idx {
    /*! Interface for Transport module */
    MOD_SMCF_CLIENT_API_IDX_TRANSPORT_SIGNAL = 1,
    /*! Number of exposed interfaces */
    MOD_SMCF_CLIENT_API_COUNT
};

#endif /* MOD_SMCF_CLIENT_H */
