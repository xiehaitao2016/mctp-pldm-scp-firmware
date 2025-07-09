/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_H
#define CONFIG_H

#define LIBPLDM_ABI_DEPRECATED __attribute__((visibility("default")))

#define LIBPLDM_ABI_STABLE __attribute__((visibility("default")))

#define LIBPLDM_ABI_DEPRECATED_UNSAFE __attribute__((visibility("default")))

#define LIBPLDM_ABI_TESTING

#define SSIZE_MAX  ((ssize_t)(SIZE_MAX >> 1))

#define ENOKEY     132  /* Required key not available */

#endif /* CONFIG_H */
