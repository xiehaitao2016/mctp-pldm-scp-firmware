/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Platform generic definitions.
 */

#ifndef PLATFORM_CORE_H
#define PLATFORM_CORE_H

#include <fwk_assert.h>

#define PLATFORM_CORE_PER_CLUSTER_MAX 1

#define NUMBER_OF_ROOTPORTS 5

#define CORES_PER_CLUSTER  1
#if (PLATFORM_VARIANT == 0)
#define NUMBER_OF_CLUSTERS 16
#elif (PLATFORM_VARIANT == 1)
#define NUMBER_OF_CLUSTERS 8
#elif (PLATFORM_VARIANT == 2)
#define NUMBER_OF_CLUSTERS 4
#else
#error "Unsupported PLATFORM_VARIANT value"
#endif

#if (PLATFORM_VARIANT == 0)
#    define NUM_PCIE_INTEG_CTRL 4
#elif (PLATFORM_VARIANT == 1)
#    define NUM_PCIE_INTEG_CTRL 1
#elif (PLATFORM_VARIANT == 2)
#    define NUM_PCIE_INTEG_CTRL 1
#else
#    error "Unsupported PLATFORM_VARIANT value"
#endif

/* Number of chips supported on the platform. */
enum platform_chip_id {
    PLATFORM_CHIP_0,
#if (PLATFORM_VARIANT == 2)
    PLATFORM_CHIP_1,
    PLATFORM_CHIP_2,
    PLATFORM_CHIP_3,
#endif
    PLATFORM_CHIP_COUNT
};

static inline unsigned int platform_get_cluster_count(void)
{
    return NUMBER_OF_CLUSTERS;
}

static inline unsigned int platform_get_core_per_cluster_count(
    unsigned int cluster)
{
    fwk_assert(cluster < platform_get_cluster_count());

    return CORES_PER_CLUSTER;
}

static inline unsigned int platform_get_core_count(void)
{
    return platform_get_core_per_cluster_count(0) *
        platform_get_cluster_count();
}

#endif /* PLATFORM_CORE_H */
