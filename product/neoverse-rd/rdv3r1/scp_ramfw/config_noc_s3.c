/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'io_block'.
 */

#include <mod_noc_s3.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

#include <fmw_cmsis.h>

struct fwk_module_config config_noc_s3 = {
    .elements = FWK_MODULE_STATIC_ELEMENTS({ { 0 } })
};
