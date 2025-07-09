/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_CMN700_H
#define CONFIG_CMN700_H

/* CCIX Related macros */
#define CHIP_0 0
#define CHIP_1 1
#define CHIP_2 2
#define CHIP_3 3

#define RNF_PER_CHIP 8
#define CHIP_COUNT   4

/* CCG ports available on the variant 2. */
enum rdn2cfg2_cmn700_ccg_port {
    CCG_PORT_0,
    CCG_PORT_1,
    CCG_PORT_2,
    CCG_PORT_3,
    CCG_PORT_4,
    CCG_PER_CHIP,
};

/* Total RN-Fs (N2 CPUs) per chips for variant 2. */
#define RNF_PER_CHIP_CFG2 4

/*
 * CMN700 nodes
 */
#if (PLATFORM_VARIANT == 0)
#    define MEM_CNTRL0_ID 64
#    define MEM_CNTRL1_ID 128
#    define MEM_CNTRL2_ID 192
#    define MEM_CNTRL3_ID 256
#    define MEM_CNTRL4_ID 108
#    define MEM_CNTRL5_ID 172
#    define MEM_CNTRL6_ID 234
#    define MEM_CNTRL7_ID 298

#    define NODE_ID_HND  260
#    define NODE_ID_HNI0 0
#    define NODE_ID_HNP0 324
#    define NODE_ID_HNP1 340
#    define NODE_ID_HNP2 348
#    define NODE_ID_HNP3 362
#    define NODE_ID_HNP4 364
#    define NODE_ID_SBSX 196

#    define NODE_ID_NON_PCIE_IO_MACRO NODE_ID_HNP4

#    define MESH_SIZE_X 6
#    define MESH_SIZE_Y 6

#elif (PLATFORM_VARIANT == 1)
#    define MEM_CNTRL0_ID 32
#    define MEM_CNTRL1_ID 64

#    define NODE_ID_HND  68
#    define NODE_ID_HNI0 0
#    define NODE_ID_HNP0 2
#    define NODE_ID_HNP1 3
#    define NODE_ID_SBSX 66

#    define NODE_ID_NON_PCIE_IO_MACRO NODE_ID_HNP1

#    define MESH_SIZE_X 3
#    define MESH_SIZE_Y 3

#elif (PLATFORM_VARIANT == 2)
#    define MEM_CNTRL0_ID 108
#    define MEM_CNTRL1_ID 172
#    define MEM_CNTRL2_ID 234
#    define MEM_CNTRL3_ID 298

#    define NODE_ID_HND  256
#    define NODE_ID_HNI0 0
#    define NODE_ID_HNP0 0x144
#    define NODE_ID_HNP1 0x154
#    define NODE_ID_HNP2 0x15c
#    define NODE_ID_HNP3 0x16a
#    define NODE_ID_SBSX 196

#    define NODE_ID_NON_PCIE_IO_MACRO NODE_ID_HNP1

#    define MESH_SIZE_X 6
#    define MESH_SIZE_Y 6

#elif (PLATFORM_VARIANT == 3)
#    define MEM_CNTRL0_ID 16
#    define MEM_CNTRL1_ID 1168
#    define MEM_CNTRL2_ID 8
#    define MEM_CNTRL3_ID 1160
#    define MEM_CNTRL4_ID 24
#    define MEM_CNTRL5_ID 1176
#    define MEM_CNTRL6_ID 32
#    define MEM_CNTRL7_ID 1184

#    define NODE_ID_HND  42
#    define NODE_ID_HNI0 256
#    define NODE_ID_HNP0 44
#    define NODE_ID_HNP1 298
#    define NODE_ID_HNP2 554
#    define NODE_ID_HNP3 682
#    define NODE_ID_HNP4 938
#    define NODE_ID_SBSX 172

#    define NODE_ID_NON_PCIE_IO_MACRO NODE_ID_HNP4

#    define MESH_SIZE_X 10
#    define MESH_SIZE_Y 6
#endif

#endif /* CONFIG_CMN700_H */
