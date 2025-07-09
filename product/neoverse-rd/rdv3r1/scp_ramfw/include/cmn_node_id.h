/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     CMN node ID.
 */

#ifndef CMN_NODE_ID
#define CMN_NODE_ID

#include "platform_variant.h"

#if (PLATFORM_VARIANT == RD_V3_R1)
#define MEM_CNTRL00_ID 8
#define MEM_CNTRL01_ID 16
#define MEM_CNTRL02_ID 24
#define MEM_CNTRL03_ID 32
#define MEM_CNTRL04_ID 40
#define MEM_CNTRL05_ID 48
#define MEM_CNTRL06_ID 1032
#define MEM_CNTRL07_ID 1040
#define MEM_CNTRL08_ID 1048
#define MEM_CNTRL09_ID 1056
#define MEM_CNTRL10_ID 1064
#define MEM_CNTRL11_ID 1072

#define NODE_ID_HND 1084

#define NODE_ID_HNT1 1024

#define NODE_ID_HNP0 176
#define NODE_ID_HNP1 432
#define NODE_ID_HNP2 688

#define NODE_ID_HNI0 128
#define NODE_ID_HNI1 256
#define NODE_ID_HNI2 384
#define NODE_ID_HNI3 512
#define NODE_ID_HNI4 640
#define NODE_ID_HNI5 768
#define NODE_ID_HNI6 896
#define NODE_ID_HNI7 948
#define NODE_ID_HNI8 956

#define NODE_ID_SBSX 952

/* D2D */
#define NODE_ID_CCG00 4
#define NODE_ID_CCG01 12
#define NODE_ID_CCG06 52
#define NODE_ID_CCG07 132

#define MESH_SIZE_X 9
#define MESH_SIZE_Y 8

#define IOVB_NODE_ID0 NODE_ID_HNP0
#define IOVB_NODE_ID1 NODE_ID_HNP1
#define IOVB_NODE_ID2 NODE_ID_HNP2

#elif (PLATFORM_VARIANT == RD_V3_R1_CFG1)

#define MEM_CNTRL00_ID 32
#define MEM_CNTRL01_ID 64

#define NODE_ID_HND 68

#define NODE_ID_HNP0 2
#define NODE_ID_HNP1 3
#define NODE_ID_HNP2 26
#define NODE_ID_HNP3 27

#define NODE_ID_HNI0 0
#define NODE_ID_HNI1 70
#define NODE_ID_HNI2 60
#define NODE_ID_HNI3 88
#define NODE_ID_HNI4 92

#define NODE_ID_SBSX 66

/* D2D */
#define NODE_ID_CCG00 34
#define NODE_ID_CCG01 10

 /* S2S */
#define NODE_ID_CCG02 12
#define NODE_ID_CCG03 18
#define NODE_ID_CCG04 28

#define MESH_SIZE_X 3
#define MESH_SIZE_Y 4

#define IOVB_NODE_ID0 NODE_ID_HNP0
#define IOVB_NODE_ID1 NODE_ID_HNP2

#else
# error "Unsupported PLATFORM_VARIANT value"
#endif

#endif /* CMN_NODE_ID */
