/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_mctp.h>

#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

struct fwk_module_config config_mctp = {
    .data = NULL,
};
