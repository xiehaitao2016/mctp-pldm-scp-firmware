/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Base address definitions for the LCP's sub-system.
 */

#ifndef LCP_CSS_MMAP_H
#define LCP_CSS_MMAP_H

#include <fwk_macros.h>

// clang-format off

#define LCP_ITCM_S_BASE (0x10000000)
#define LCP_ITCM_SIZE   (128 * 1024)

#define LCP_DTCM_S_BASE (0x30000000)
#define LCP_DTCM_SIZE   (64 * 1024)

#define LCP_CORE_ITCM_REGION_START LCP_ITCM_S_BASE
#define LCP_CORE_ITCM_REGION_END   (LCP_ITCM_S_BASE + LCP_ITCM_SIZE - 1)

#define LCP_CORE_DTCM_REGION_START LCP_DTCM_S_BASE
#define LCP_CORE_DTCM_REGION_END   (LCP_DTCM_S_BASE + LCP_DTCM_SIZE - 1)

#define LCP_CORE_PERIPHERAL_REGION_START (0x30010000)
#define LCP_CORE_PERIPHERAL_REGION_END   (0x6FFFFFFF)

/*
 * SRAM region breakout
 *  ______________________________ (0xB007_FFFF)
 * |______________________________|
 * |                              |(0x7400_FFFF)
 * |  AP_LCP Shared SRAM (64 KB)  |
 * |______________________________|(0x7400_0000)
 * |______________________________|(0x7040_7FFF)
 * |                              |
 * |    LCP SMCF SRAM (32 KB)     |
 * |______________________________|(0x7040_0000)
 * |______________________________|(0x700F_FFFF)
 * |                              |
 * |     SCP SMCF SRAM (1 MB)     |
 * |______________________________|(0x7000_0000)
 *
 */

#define LCP_SRAM_REGION_START (0x70000000)
#define LCP_SRAM_REGION_END   (0xB007FFFF)

#define LCP_DEVICE_REGION_START (0xB0080000)
#define LCP_DEVICE_REGION_END   (0xFFFFFFFF)

/* LCP sub-system peripherals */
#define LCP_UART_BASE (0xB5080000)

// clang-format on

#endif /* LCP_CSS_MMAP_H */
