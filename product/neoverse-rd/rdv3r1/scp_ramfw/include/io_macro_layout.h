/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Base address definitions for the SCP's sub-system and access extending
 *     into the rest of the CSS.
 */

#ifndef IO_MACRO_LAYOUT
#define IO_MACRO_LAYOUT

#include "scp_css_mmap.h"

/* Calculate the base address of IO_NOC_S3 GPV for IO Block using its index. */
#define IO_NOC_S3_GPV_BLOCK_BASE(idx) \
    (SCP_IO_BLOCK_NOC_S3_GPV_BASE + (idx * SCP_IO_BLOCK_REG_SIZE))
/* Size of the IO Block IO_NOC_S3 GPV : 16MB */
#define IO_NOC_S3_GPV_BLOCK_SIZE (16 * FWK_MIB)

/* Calculate the base address for the TCU in IO block using IO block's index. */
#define TCU_REG_BASE(idx) \
    (SCP_IO_BLOCK_BASE + (idx * SCP_IO_BLOCK_REG_SIZE))
#define TCU_REG_SIZE (0x4000000UL)

/* Calculate the base address of IO control register bank. */
#define CTRL_REG_BASE(idx) \
    (SCP_IO_BLOCK_CTRL_REG_BASE + (idx * SCP_IO_BLOCK_REG_SIZE))
#define CTRL_REG_SIZE (0x10000)

/* Calculate the base address of RAS Agent register bank. */
#define RAS_AGENT_REG_BASE(idx) \
    (SCP_IO_BLOCK_RAS_AGENT_REG_BASE + (idx * SCP_IO_BLOCK_REG_SIZE))
#define RAS_AGENT_REG_SIZE (0x10000)

enum asni_domain_id {
    /*! CMN HNP0 completer Node ID. */
    ASNI_HNP0_CMN,
    /*! CMN HNP1 completer Node ID. */
    ASNI_HNP1_CMN,
    /*! Maximum number of completer Node IDs*/
    ASNI_DOMAIN_MAX_ID
};

enum amni_domain_id {
    /*! PCIe x16_0 requester Node ID. */
    AMNI_PCIEX16_0,
    /*! PCIe x16_1 requester Node ID. */
    AMNI_PCIEX16_1,
    /*! PCIe x1_0 requester Node ID. */
    AMNI_PCIEX1_0,
    /*! PCIe x4_0 requester Node ID. */
    AMNI_PCIEX4_0,
    /*! PCIe x4_1 requester Node ID. */
    AMNI_PCIEX4_1,
    /*! PCIe x8_0 requester Node ID. */
    AMNI_PCIEX8_0,
    /*! PMNI Control requester Node ID. */
    PMNI_CTRL_REG_APB,
    /*! PCIE x16_0 configurtion requester Node ID. */
    PMNI_PCIEX16_0_CFG,
    /*! PCIE x1_0 configurtion requester Node ID. */
    PMNI_PCIEX1_0_CFG,
    /*! PCIE x4_0 configurtion requester Node ID. */
    PNI_PCIEX4_0_CFG,
    /*! PCIE x4_1 configurtion requester Node ID. */
    PNI_PCIEX4_1_CFG,
    /*! PCIE x8_0 configurtion requester Node ID. */
    PMNI_PCIEX8_0_CFG,
    /*! TCU APB requester Node ID. */
    PMNI_TCU_APB,
    /*! RAS Agent Node ID. */
    PMNI_RAS_AGENT_APB,
    /*! Config Space interface ID. */
    CFGNI_ID = 0xF
};

#endif /* IO_MACRO_LAYOUT */
