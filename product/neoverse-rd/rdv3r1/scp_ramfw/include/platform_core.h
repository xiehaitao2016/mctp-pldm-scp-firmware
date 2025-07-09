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

#include "platform_variant.h"

/* Maximum number of clusters supported */
#define MAX_NUM_CLUSTERS 70

/* Maximum number of LCP sub-system instances supported */
#define MAX_NUM_LCP 7

/*
* NUM_OF_LCP: Actual number of LCP sub-system instances implemented
* SHARED_NUM_CORE_PER_LCP: Number of AP cores that share an LCP
* NUM_PCIE_INTEG_CTRL: Number of PCIe Integration Control Register Blocks
*/
#if (PLATFORM_VARIANT == RD_V3_R1)
#   define NUM_OF_LCP 7
#   define SHARED_NUM_CORE_PER_LCP 2
#   define NUM_PCIE_INTEG_CTRL 2
/* Number of chips supported on the platform: 2 */
enum platform_chip_id { PLATFORM_CHIP_0, PLATFORM_CHIP_1, PLATFORM_CHIP_COUNT };
#elif (PLATFORM_VARIANT == RD_V3_R1_CFG1)
#   define NUM_OF_LCP 2
#   define SHARED_NUM_CORE_PER_LCP 4
#   define NUM_PCIE_INTEG_CTRL 1
/* Number of chips supported on the platform: 4 */
enum platform_chip_id { PLATFORM_CHIP_0, PLATFORM_CHIP_1, PLATFORM_CHIP_2, PLATFORM_CHIP_3, PLATFORM_CHIP_COUNT };
#else
#   error "Unsupported PLATFORM_VARIANT value"
#endif

/* Actual number of AP clusters implemented */
#define NUMBER_OF_CLUSTERS (NUM_OF_LCP * SHARED_NUM_CORE_PER_LCP)

/* Number of cores per cluster */
#define CORES_PER_CLUSTER 1

/* Number of PCIe root ports */
#define NUMBER_OF_ROOTPORTS 5

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
