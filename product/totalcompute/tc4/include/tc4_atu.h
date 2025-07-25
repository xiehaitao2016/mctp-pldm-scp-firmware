/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TC4_ATU_H
#define TC4_ATU_H

/* ATU Region allocations */
#define START_REGION_FOR_MOD_ATU (0)
#define MAX_REGIONS_FOR_MOD_ATU  (8)
#define END_REGION_FOR_MOD_ATU \
    (START_REGION_FOR_MOD_ATU + MAX_REGIONS_FOR_MOD_ATU - 1)

/* ATU System Acccess Ports */
#define SCP_SYSTEM_ACCESS_ATU_DATA0_BASE_NS   UINT32_C(0x60000000)
#define SCP_SYSTEM_ACCESS_ATU_DATA0_BASE_S    UINT32_C(0x70000000)
#define SCP_SYSTEM_ACCESS_ATU_DATA1_BASE_NS   UINT32_C(0x80000000)
#define SCP_SYSTEM_ACCESS_ATU_DATA1_BASE_S    UINT32_C(0x90000000)
#define SCP_SYSTEM_ACCESS_ATU_INT_EXP_BASE_NS UINT32_C(0xA0000000)
#define SCP_SYSTEM_ACCESS_ATU_INT_EXP_BASE_S  UINT32_C(0xB0000000)
#define SCP_SYSTEM_ACCESS_ATU_DEV_BASE_NS     UINT32_C(0xC0000000)
#define SCP_SYSTEM_ACCESS_ATU_DEV_BASE_S      UINT32_C(0xD0000000)

/* ATU Logical Addresses */
#define SCP_ATU_LOG_ADDR_GTCLK_AP \
    (SCP_SYSTEM_ACCESS_ATU_DATA0_BASE_S + 0x0A810000)
#define SCP_ATU_LOG_ADDR_GTCLK (SCP_SYSTEM_ACCESS_ATU_DATA0_BASE_S + 0x0A430000)
#define SCP_ATU_LOG_ADDR_SHARED_SRAM \
    (SCP_SYSTEM_ACCESS_ATU_DATA0_BASE_S + 0x04000000)
#define SCP_ATU_LOG_ADDR_ROS_SOC_BASE \
    (SCP_SYSTEM_ACCESS_ATU_DATA0_BASE_S + 0x0FF70000)
#define SCP_ATU_LOG_ADDR_SHARED_NSRAM (SCP_SYSTEM_ACCESS_ATU_DATA0_BASE_NS)
#define SCP_ATU_LOG_ADDR_CLUSTER_UTIL (SCP_SYSTEM_ACCESS_ATU_INT_EXP_BASE_S)
#define SCP_ATU_LOG_ADDR_PLL          (SCP_SYSTEM_ACCESS_ATU_DEV_BASE_S)
#define SCP_ATU_LOG_ADDR_CCSM         (SCP_SYSTEM_ACCESS_ATU_DEV_BASE_S + 0x100000)

/* ATU Physical Addresses */
#define SCP_ATU_PHY_ADDR_GTCLK_AP     (0x2A810000)
#define SCP_ATU_PHY_ADDR_GTCLK        (0x47000000)
#define SCP_ATU_PHY_ADDR_CLUSTER_UTIL (0x5F000000)
#define SCP_ATU_PHY_ADDR_SHARED_SRAM  (0x04000000)
#define SCP_ATU_PHY_ADDR_SHARED_NSRAM (0x06000000)
#define SCP_ATU_PHY_ADDR_ROS_SOC      (0x7FF70000)
#define SCP_ATU_PHY_ADDR_PLL          (0x43000000)
#define SCP_ATU_PHY_ADDR_CCSM         (0x59800000)

/* ATU Region Sizes */
#define SCP_ATU_REG_SIZE_GTCLK_AP     (128 * FWK_KIB)
#define SCP_ATU_REG_SIZE_GTCLK        (128 * FWK_KIB)
#define SCP_ATU_REG_SIZE_SHARED_SRAM  (512 * FWK_KIB)
#define SCP_ATU_REG_SIZE_SHARED_NSRAM (512 * FWK_KIB)
#define SCP_ATU_REG_SIZE_ROS_SOC      (64 * FWK_KIB)
#define SCP_ATU_REG_SIZE_CLUSTER_UTIL (10 * FWK_MIB)
#define SCP_ATU_REG_SIZE_PLL          (64 * FWK_KIB)
#define SCP_ATU_REG_SIZE_CCSM         (8 * FWK_MIB)

#endif /* TC4_ATU_H */
