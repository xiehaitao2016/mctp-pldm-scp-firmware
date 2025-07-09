/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'cmn_cyprus'.
 */

#include "cmn_node_id.h"
#include "platform_core.h"
#include "scp_atw1_mmap.h"
#include "platform_variant.h"

#include <mod_cmn_cyprus.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// clang-format off

/* Coordinates of the bottom-left node in the mesh */
#define MESH_START_POS \
    { \
        .pos_x = 0, \
        .pos_y = 0, \
        .port_num = 0, \
        .device_num = 0 \
    }

/* Coordinates of the top-right node in the mesh */
#define MESH_END_POS \
    { \
        .pos_x = (MESH_SIZE_X - 1), \
        .pos_y = (MESH_SIZE_Y - 1), \
        .port_num = 1, \
        .device_num = 1 \
    }

// clang-format on

#if (PLATFORM_VARIANT == RD_V3_R1)
/*
 * List of nodes to be isolated in the mesh.
 */
struct isolated_hns_node_info isolated_hns_nodes[]= {
    {
        /* HN-S 64 NODE ID: 680 */
        .hns_pos = {
            .pos_x = 5,
            .pos_y = 5,
            .port_num = 0,
            .device_num = 0,
        },
        .hns_base = (uintptr_t)0xb5700000,
        .hns_mpam_s_base = (uintptr_t)0xb5500000,
        .hns_mpam_ns_base = (uintptr_t)0xb5600000,
    },
    {
        /* HN-S 65 MPAM_NS NODE ID: 681 */
        .hns_pos = {
            .pos_x = 5,
            .pos_y = 5,
            .port_num = 0,
            .device_num = 1,
        },
        .hns_base = (uintptr_t)0xb5740000,
        .hns_mpam_s_base = (uintptr_t)0xb5540000,
        .hns_mpam_ns_base = (uintptr_t)0xb5640000,
    },
    {
        /* HN-S 66 NODE ID: 808 */
        .hns_pos = {
            .pos_x = 6,
            .pos_y = 5,
            .port_num = 0,
            .device_num = 0,
        },
        .hns_base = (uintptr_t)0xb9700000,
        .hns_mpam_s_base = (uintptr_t)0xb9500000,
        .hns_mpam_ns_base = (uintptr_t)0xb9600000,
    },
    {
        /* HN-S 67 MPAM_NS NODE ID: 809 */
        .hns_pos = {
            .pos_x = 6,
            .pos_y = 5,
            .port_num = 0,
            .device_num = 1,
        },
        .hns_base = (uintptr_t)0xb9740000,
        .hns_mpam_s_base = (uintptr_t)0xb9540000,
        .hns_mpam_ns_base = (uintptr_t)0xb9640000,
    },    {
        /* HN-S 68 NODE ID: 936 */
        .hns_pos = {
            .pos_x = 7,
            .pos_y = 5,
            .port_num = 0,
            .device_num = 0,
        },
        .hns_base = (uintptr_t)0xbd700000,
        .hns_mpam_s_base = (uintptr_t)0xbd500000,
        .hns_mpam_ns_base = (uintptr_t)0xbd600000,
    },
    {
        /* HN-S 69 MPAM_NS NODE ID: 937 */
        .hns_pos = {
            .pos_x = 7,
            .pos_y = 5,
            .port_num = 0,
            .device_num = 1,
        },
        .hns_base = (uintptr_t)0xbd740000,
        .hns_mpam_s_base = (uintptr_t)0xbd540000,
        .hns_mpam_ns_base = (uintptr_t)0xbd640000,
    },
};
#endif

/*
 * HN-F to SN-F mapping table.
 */
// clang-format off
static const unsigned int snf_table[] = {
#if (PLATFORM_VARIANT == RD_V3_R1_CFG1)
    MEM_CNTRL00_ID, /* Maps to HN-S logical node 00 */
    MEM_CNTRL00_ID, /* Maps to HN-S logical node 01 */
    MEM_CNTRL00_ID, /* Maps to HN-S logical node 02 */
    MEM_CNTRL00_ID, /* Maps to HN-S logical node 03 */
    MEM_CNTRL01_ID, /* Maps to HN-S logical node 04 */
    MEM_CNTRL01_ID, /* Maps to HN-S logical node 05 */
    MEM_CNTRL01_ID, /* Maps to HN-S logical node 06 */
    MEM_CNTRL01_ID, /* Maps to HN-S logical node 07 */
#else
    MEM_CNTRL00_ID,
    MEM_CNTRL01_ID,
    MEM_CNTRL02_ID,
    MEM_CNTRL03_ID,
    MEM_CNTRL04_ID,
    MEM_CNTRL05_ID,
    MEM_CNTRL06_ID,
    MEM_CNTRL07_ID,
    MEM_CNTRL08_ID,
    MEM_CNTRL09_ID,
    MEM_CNTRL10_ID,
    MEM_CNTRL11_ID,
#endif
};
// clang-format on

/*
 * CCG ports in the mesh.
 */
enum rdv3r1_cmn_cyprus_ccg_port {
    CCG_PORT_00,
    CCG_PORT_01,
    CCG_PORT_02,
    CCG_PORT_03,
    CCG_PORT_04,
#if (PLATFORM_VARIANT == RD_V3_R1)
    CCG_PORT_05,
    CCG_PORT_06,
    CCG_PORT_07,
    CCG_PORT_08,
    CCG_PORT_09,
    CCG_PORT_10,
    CCG_PORT_11,
    CCG_PORT_12,
    CCG_PORT_13,
    CCG_PORT_14,
    CCG_PORT_15,
    CCG_PORT_16,
    CCG_PORT_17,
    CCG_PORT_18,
    CCG_PORT_19,
    CCG_PORT_20,
    CCG_PORT_21,
    CCG_PORT_22,
    CCG_PORT_23,
    CCG_PORT_24,
    CCG_PORT_25,
#endif
    CCG_PER_CHIP,
};

/*
 * Request Node System Address Map (RNSAM) configuration.
 */
static const struct mod_cmn_cyprus_mem_region_map mmap[] = {
    {
        /*
         * System cache backed region
         * Map: 0x0000_0000_0000 - 0xFFFF_FFFF_FFFF (256 TiB)
         */
        .base = UINT64_C(0x000000000000),
        .size = UINT64_C(256) * FWK_TIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE,
        .hns_pos_start = MESH_START_POS,
        .hns_pos_end = MESH_END_POS,
    },
    {
        /*
         * Shared SRAM
         * Map: 0x0000_0000_0000 - 0x0000_07FF_FFFF (128 MB)
         */
        .base = UINT64_C(0x000000000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE_SUB,
        .node_id = NODE_ID_SBSX,
    },
    {
        /*
         * Boot Flash
         * Map: 0x00_0800_0000 - 0x00_0FFF_FFFF (128 MB)
         */
        .base = UINT64_C(0x0008000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
#if (PLATFORM_VARIANT == RD_V3_R1_CFG1)
        .node_id = NODE_ID_HNI0,
#else
        .node_id = NODE_ID_HNT1,
#endif
    },
    {
        /*
         * Peripherals
         * Map: 0x00_1000_0000 - 0x00_1EFF_FFFF (240 MB)
         */
        .base = UINT64_C(0x0010000000),
        .size = UINT64_C(240) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Shared RSM SRAM
         * Map: 0x00_1F00_0000 - 0x00_1F3F_FFFF (4 MB)
         */
        .base = UINT64_C(0x001F000000),
        .size = UINT64_C(4) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE_SUB,
        .node_id = NODE_ID_SBSX,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_1F40_0000 - 0x00_5FFF_FFFF (1036 MB)
         */
        .base = UINT64_C(0x001F400000),
        .size = UINT64_C(1036) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * CMN_CYPRUS GPV
         * Map: 0x01_0000_0000 - 0x01_3FFF_FFFF (1 GB)
         */
        .base = UINT64_C(0x0100000000),
        .size = UINT64_C(1) * FWK_GIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Cluster Utility Memory region
         * Map: 0x1_4000_0000 - 0x1_5FFF_FFFF (512 MB)
         */
        .base = UINT64_C(0x140000000),
        .size = UINT64_C(512) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
#if (PLATFORM_VARIANT == RD_V3_R1)
    {
        /*
         * LCP0
         * Map: 0x1_6000_0000 - 0x1_601F_FFFF (2 MB)
         */
        .base = UINT64_C(0x160000000),
        .size = UINT64_C(2) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNI0,
    },
    {
        /*
         * LCP1
         * Map: 0x1_6020_0000 - 0x1_603F_FFFF (2 MB)
         */
        .base = UINT64_C(0x160200000),
        .size = UINT64_C(2) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNI1,
    },
    {
        /*
         * LCP2
         * Map: 0x1_6040_0000 - 0x1_605F_FFFF (2 MB)
         */
        .base = UINT64_C(0x160400000),
        .size = UINT64_C(2) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNI2,
    },
    {
        /*
         * LCP3
         * Map: 0x1_6060_0000 - 0x1_607F_FFFF (2 MB)
         */
        .base = UINT64_C(0x160600000),
        .size = UINT64_C(2) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNI3,
    },
    {
        /*
         * LCP4
         * Map: 0x1_6080_0000 - 0x1_609F_FFFF (2 MB)
         */
        .base = UINT64_C(0x160800000),
        .size = UINT64_C(2) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNI4,
    },
    {
        /*
         * LCP5
         * Map: 0x1_60A0_0000 - 0x1_60BF_FFFF (2 MB)
         */
        .base = UINT64_C(0x160A00000),
        .size = UINT64_C(2) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNI5,
    },
    {
        /*
         * LCP6
         * Map: 0x1_60C0_0000 - 0x1_60DF_FFFF (2 MB)
         */
        .base = UINT64_C(0x160C00000),
        .size = UINT64_C(2) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNI6,
    },
#elif (PLATFORM_VARIANT == RD_V3_R1_CFG1)
    {
        /*
         * LCP0
         * Map: 0x1_6000_0000 - 0x1_601F_FFFF (2 MB)
         */
        .base = UINT64_C(0x160000000),
        .size = UINT64_C(2) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNI2,
    },
    {
        /*
         * LCP1
         * Map: 0x1_6020_0000 - 0x1_603F_FFFF (2 MB)
         */
        .base = UINT64_C(0x160200000),
        .size = UINT64_C(2) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNI4,
    },
#endif
    {
        /*
         * Peripherals - Memory Controller
         * Map: 0x1_8000_0000 - 0x1_8FFF_FFFF (256 MB)
         */
        .base = UINT64_C(0x180000000),
        .size = UINT64_C(256) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals, NCI GPV Memory Map 0
         * Map: 0x01_C000_0000 - 0x01_C7FF_FFFF (128 MB)
         */
        .base = UINT64_C(0x01C0000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = IOVB_NODE_ID0,
    },
    {
        /*
         * Peripherals, NCI GPV Memory Map 1
         * Map: 0x01_C800_0000 - 0x01_CFFF_FFFF (128 MB)
         */
        .base = UINT64_C(0x01C8000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = IOVB_NODE_ID1,
    },
#if (PLATFORM_VARIANT == RD_V3_R1)
    {
        /*
         * Peripherals, NCI GPV Memory Map 2
         * Map: 0x01_D000_0000 - 0x01_D7FF_FFFF (128 MB)
         */
        .base = UINT64_C(0x01D0000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = IOVB_NODE_ID2,
    },
#endif
    {
        /*
         * GPC_SMMU region
         * Map: 0x02_4000_0000 - 0x02_47FF_FFFF (128 MB)
         */
        .base = UINT64_C(0x240000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Non Secure NOR Flash 0/1
         * Map: 0x06_5000_0000 - 0x06_57FF_FFFF (128 MB)
         */
        .base = UINT64_C(0x0650000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Ethernet Controller PL91x
         * Map: 0x06_5C00_0000 - 0x06_5FFF_FFFF (64 MB)
         */
        .base = UINT64_C(0x065C000000),
        .size = UINT64_C(64) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
};

/* Number of CCG nodes per CML Port Aggregation group (CPAG) */
#define CCG_PER_CPAG 2

/* List of LDIDs of CCG nodes in the CPAG */
#define CCG_LDID_LIST(...) ((unsigned int[]){ __VA_ARGS__ })

/* Home Agent ID */
#define GET_HAID(chip_id, ccg_port) ((CCG_PER_CHIP * chip_id) + ccg_port)

/* List of Home Agent IDs of CCG nodes in the CPAG */
#define CCG_HAID_LIST(chip_id, ccg_port_x, ccg_port_y) \
    ((unsigned int[]){ GET_HAID(chip_id, ccg_port_x), \
                       GET_HAID(chip_id, ccg_port_y) })

/*!
 * \brief Define a Remote hashed region.
 *
 * \param pri_base Primary HTG region base address.
 * \param pri_size Primary HTG region size.
 * \param sec_base Secondary HTG region base address.
 * \param sec_size Secondary HTG region size.
 * \param start_pos HN-S node start position of the HTG.
 * \param end_pos HN-S node end position of the HTG.
 */
// clang-format off
#define REMOTE_HASHED_REGION( \
    pri_base, pri_size, sec_base, sec_size, start_pos, end_pos) \
    { \
        .base = (pri_base), \
        .size = (pri_size), \
        .sec_region_base = (sec_base), \
        .sec_region_size = (sec_size), \
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_REMOTE_HASHED, \
        .hns_pos_start = start_pos, \
        .hns_pos_end = end_pos, \
    }

#define REMOTE_NH_REGION(region_start, region_size, target) \
{ \
    .base = (region_start), \
    .size = (region_size), \
    .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_REMOTE_NON_HASHED, \
    .node_id = target, \
}
// clang-format on

/* Peripheral memory size for a chip (64 GiB) */
#define PERIPH_MEM_SIZE (UINT64_C(1) << 36)
/* Peripheral memory base for a chip */
#define CHIP_PERIPH_MEM_BASE(chip_id) (chip_id * PERIPH_MEM_SIZE)

/* PCIe ECAM Base */
#define PCI_ECAM_BASE UINT64_C(0x10000000000)
/* PCIe ECAM size (256 MiB) */
#define PCI_ECAM_SIZE (UINT64_C(256) * FWK_MIB)
/* PCIe ECAM Base for a chip*/
#define CHIP_PCIE_ECAM_BASE(chip_id) (PCI_ECAM_BASE + (PCI_ECAM_SIZE * (chip_id)))

/* PCIe MMIO_L Base */
#define PCI_MMIOL_BASE UINT64_C(0x60000000)
/* PCIe MMIO_L size (256 MiB) */
#if (PLATFORM_VARIANT == RD_V3_R1_CFG1)
#define PCI_MMIOL_SIZE (UINT64_C(128) * FWK_MIB)
#else
#define PCI_MMIOL_SIZE (UINT64_C(256) * FWK_MIB)
#endif
/* PCIe MMIO_L Base for a chip*/
#define CHIP_PCIE_MMIOL_BASE(chip_id) (PCI_MMIOL_BASE + (PCI_MMIOL_SIZE * (chip_id)))

/* PCIe MMIO_H Base */
#define PCI_MMIOH_BASE UINT64_C(0x10100000000)
/* PCIe MMIO_H size (64 GiB) */
#define PCI_MMIOH_SIZE (UINT64_C(64) * FWK_GIB)
/* PCIe MMIO_H Base for a chip*/
#define CHIP_PCIE_MMIOH_BASE(chip_id) (PCI_MMIOH_BASE + (PCI_MMIOH_SIZE * (chip_id)))

/* Second DRAM block base */
#define DRAM2_BASE UINT64_C(0x20080000000)
/* Second DRAM block size (1 TiB) */
#define DRAM2_SIZE (UINT64_C(1) << 40)
/* Second DRAM block base for a chip */
#define CHIP_DRAM2_BASE(chip_id) (DRAM2_BASE + (DRAM2_SIZE * chip_id))

#if (PLATFORM_VARIANT == RD_V3_R1)
/*
 * RD-V3-R1 Multichip configuration data
 *
 * Cross chip CCG connections between the chips:
 *
 *  +-------------------------------------------------+
 *  |       CCG20 CCG21 CCG22 CCG23 CCG24 CCG25       |
 *  |                                                 |
 *  |CCG08                                       CCG14|
 *  |                                                 |
 *  |CCG09                                       CCG15|
 *  |                                                 |
 *  |CCG10                                       CCG16|
 *  |                      CHIP 0                     |
 *  |CCG11                                       CCG17|
 *  |                                                 |
 *  |CCG12                                       CCG18|
 *  |                                                 |
 *  |CCG13                                       CCG19|
 *  |                                                 |
 *  | CCG00 CCG01 CCG02 CCG03 CCG04 CCG05 CCG06 CCG07 |
 *  +---+-----+-----+-----+-----+-----+-----+-----+---+
 *      |     |     |     |     |     |     |     |
 *      |     |     |     |     |     |     |     |
 *  +---+-----+-----+-----+-----+-----+-----+-----+---+
 *  | CCG07 CCG06 CCG05 CCG04 CCG03 CCG02 CCG01 CCG00 |
 *  |                                                 |
 *  |CCG08                                       CCG14|
 *  |                                                 |
 *  |CCG09                                       CCG15|
 *  |                                                 |
 *  |CCG10                                       CCG16|
 *  |                      CHIP 1                     |
 *  |CCG11                                       CCG17|
 *  |                                                 |
 *  |CCG12                                       CCG18|
 *  |                                                 |
 *  |CCG13                                       CCG19|
 *  |                                                 |
 *  |       CCG20 CCG21 CCG22 CCG23 CCG24 CCG25       |
 *  +-------------------------------------------------+
 */

/* Chip 0 CML configuration */
static const struct mod_cmn_cyprus_cml_config chip0_cml_config[] = {
    {
        /* Chip 0 to Chip 1 */
        .ccg_ldid = CCG_LDID_LIST(CCG_PORT_00, CCG_PORT_01),
        .haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_00, CCG_PORT_01),
        .remote_mmap_table = {
            {
                /*
                 * Chip 1 peripheral address space and DRAM.
                 * Map: 0x10_0000_0000 - 0x1F_FFFF_FFFF (primary)
                 *      0x300_0000_0000 - 0x3FF_FFFF_FFFF (secondary)
                 */
                .region_mmap = REMOTE_HASHED_REGION(CHIP_PERIPH_MEM_BASE(1),
                                PERIPH_MEM_SIZE, CHIP_DRAM2_BASE(1),
                                DRAM2_SIZE, MESH_START_POS, MESH_END_POS),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_07,
                                CCG_PORT_06),
            },
            {
                /*
                 * Chip 1 PCIe MMIO_L space.
                 * Map: 0x00_7000_0000 - 0x00_7FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOL_BASE(1),
                                PCI_MMIOL_SIZE, NODE_ID_CCG00),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_07,
                                CCG_PORT_06),
            },
            {
                /*
                 * Chip 1 PCIe ECAM space.
                 * Map: 0x100_1000_0000 - 0x100_1FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_ECAM_BASE(1),
                                PCI_ECAM_SIZE, NODE_ID_CCG00),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_07,
                                CCG_PORT_06),
            },
            {
                /*
                 * Chip 1 PCIe MMIO_H space.
                 * Map: 0x111_0000_0000 - 0x120_FFFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOH_BASE(1),
                                PCI_MMIOH_SIZE, NODE_ID_CCG00),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_07,
                                CCG_PORT_06),
            },
        },
        .remote_chip_id = PLATFORM_CHIP_1,
        .enable_smp_mode = true,
        .enable_direct_connect_mode = true,
        .enable_cpa_mode = true,
        .cpag_config = {
            .cpag_id = 0,
            .ccg_count = CCG_PER_CPAG,
        },
    },
};

/* Chip 1 CML configuration */
static const struct mod_cmn_cyprus_cml_config chip1_cml_config[] = {
    {
        /* Chip 1 to Chip 0 */
        .ccg_ldid = CCG_LDID_LIST(CCG_PORT_07, CCG_PORT_06),
        .haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_07, CCG_PORT_06),
        .remote_mmap_table = {
            {
                /*
                 * Chip 0 peripheral address space and DRAM.
                 * Map: 0x0 - 0x0F_FFFF_FFFF (primary)
                 *      0x200_0000_0000 - 0x2FF_FFFF_FFFF (secondary)
                 */
                .region_mmap = REMOTE_HASHED_REGION(CHIP_PERIPH_MEM_BASE(0),
                                PERIPH_MEM_SIZE, CHIP_DRAM2_BASE(0),
                                DRAM2_SIZE, MESH_START_POS, MESH_END_POS),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_00,
                                CCG_PORT_01),
            },
            {
                /*
                 * Chip 0 PCIe MMIO_L space.
                 * Map: 0x00_6000_0000 - 0x00_6FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOL_BASE(0),
                                PCI_MMIOL_SIZE, NODE_ID_CCG07),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_00,
                                CCG_PORT_01),
            },
            {
                /*
                 * Chip 0 PCIe ECAM space.
                 * Map: 0x100_0000_0000 - 0x100_0FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_ECAM_BASE(0),
                                PCI_ECAM_SIZE, NODE_ID_CCG07),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_00,
                                CCG_PORT_01),
            },
            {
                /*
                 * Chip 0 PCIe MMIO_H space.
                 * Map: 0x101_0000_0000 - 0x110_FFFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOH_BASE(0),
                                PCI_MMIOH_SIZE, NODE_ID_CCG07),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_00,
                                CCG_PORT_01),
            },
        },
        .remote_chip_id = PLATFORM_CHIP_0,
        .enable_smp_mode = true,
        .enable_direct_connect_mode = true,
        .enable_cpa_mode = true,
        .cpag_config = {
            .cpag_id = 0,
            .ccg_count = CCG_PER_CPAG,
        },
    },
};
#elif (PLATFORM_VARIANT == RD_V3_R1_CFG1)

/*
 * Note: Re-defined followings for all CPAGs as unequal CPAG CCG count
 * is not supported
 */
#undef CCG_PER_CPAG
/* Number of CCG nodes per CML Port Aggregation group (CPAG) */
#define CCG_PER_CPAG 1

#undef CCG_HAID_LIST
/* List of Home Agent IDs of CCG nodes in the CPAG */
#define CCG_HAID_LIST(chip_id, ccg_port_x) \
    ((unsigned int[]){ GET_HAID(chip_id, ccg_port_x) })

/*
 * RD-V3-R1-Cfg1 Multichip configuration data
 *
 * Cross chip CCG connections between the chips:
 *
 *  +------------------+        +------------------+
 *  |       CCG2       |        |      CCG4        |
 *  |              CCG3+--------+CCG3              |
 *  |                  |        |                  |
 *  |      CHIP 0      |        |      CHIP 2      |
 *  |              CCG4+--+     |                  |
 *  |                  |  |  +--+CCG2              |
 *  |   CCG0    CCG1   |  |  |  |   CCG0    CCG1   |
 *  +-----+------+-----+  |  |  +-----+------+-----+
 *        |      |        |  |        |      |
 *        |      |        |  |        |      |
 *  +-----+------+-----+  |  |  +-----+------+-----+
 *  |   CCG1    CCG0   |  |  |  |  CCG1    CCG0    |
 *  |              CCG2|--|--+  |                  |
 *  |                  |  +-----+CCG4              |
 *  |      CHIP 1      |        |     CHIP 3       |
 *  |                  |        |                  |
 *  |              CCG3+--------+CCG3              |
 *  |       CCG4       |        |      CCG2        |
 *  +------------------+        +------------------+
 *
 */

/* Chip 0 CML configuration */
static const struct mod_cmn_cyprus_cml_config chip0_cml_config[] = {
    {
        /* Chip 0 to Chip 1 */
        .ccg_ldid = CCG_LDID_LIST(CCG_PORT_00),
        .haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_00),
        .remote_mmap_table = {
            {
                /*
                 * Chip 1 peripheral address space and DRAM.
                 * Map: 0x10_0000_0000 - 0x1F_FFFF_FFFF (primary)
                 *      0x300_0000_0000 - 0x3FF_FFFF_FFFF (secondary)
                 */
                .region_mmap = REMOTE_HASHED_REGION(CHIP_PERIPH_MEM_BASE(1),
                                PERIPH_MEM_SIZE, CHIP_DRAM2_BASE(1),
                                DRAM2_SIZE, MESH_START_POS, MESH_END_POS),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_01),
            },
            {
                /*
                 * Chip 1 PCIe MMIO_L space.
                 * Map: 0x00_6800_0000 - 0x00_67FF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOL_BASE(1),
                                PCI_MMIOL_SIZE, NODE_ID_CCG00),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_01),
            },
            {
                /*
                 * Chip 1 PCIe ECAM space.
                 * Map: 0x100_1000_0000 - 0x100_1FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_ECAM_BASE(1),
                                PCI_ECAM_SIZE, NODE_ID_CCG00),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_01),
            },
            {
                /*
                 * Chip 1 PCIe MMIO_H space.
                 * Map: 0x111_0000_0000 - 0x120_FFFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOH_BASE(1),
                                PCI_MMIOH_SIZE, NODE_ID_CCG00),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_01),
            },
        },
        .remote_chip_id = PLATFORM_CHIP_1,
        .enable_smp_mode = true,
        .enable_direct_connect_mode = true,
        .enable_cpa_mode = true,
        .cpag_config = {
            .cpag_id = 0,
            .ccg_count = CCG_PER_CPAG,
        },
    },
    {
        /* Chip 0 to Chip 2 */
        .ccg_ldid = CCG_LDID_LIST(CCG_PORT_03),
        .haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_03),
        .remote_mmap_table = {
            {
                /*
                 * Chip 2 peripheral address space and DRAM.
                 * Map: 0x20_0000_0000 - 0x2F_FFFF_FFFF (primary)
                 *      0x400_0000_0000 - 0x4FF_FFFF_FFFF (secondary)
                 */
                .region_mmap = REMOTE_HASHED_REGION(CHIP_PERIPH_MEM_BASE(2),
                                PERIPH_MEM_SIZE, CHIP_DRAM2_BASE(2),
                                DRAM2_SIZE, MESH_START_POS, MESH_END_POS),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_2, CCG_PORT_03),
            },
            {
                /*
                 * Chip 2 PCIe MMIO_L space.
                 * Map: 0x00_7000_0000 - 0x00_77FF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOL_BASE(2),
                                PCI_MMIOL_SIZE, NODE_ID_CCG03),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_2, CCG_PORT_03),
            },
            {
                /*
                 * Chip 2 PCIe ECAM space.
                 * Map: 0x100_2000_0000 - 0x100_2FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_ECAM_BASE(2),
                                PCI_ECAM_SIZE, NODE_ID_CCG03),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_2, CCG_PORT_03),
            },
            {
                /*
                 * Chip 2 PCIe MMIO_H space.
                 * Map: 0x121_0000_0000 - 0x130_FFFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOH_BASE(2),
                                PCI_MMIOH_SIZE, NODE_ID_CCG03),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_2, CCG_PORT_03),
            },
        },
        .remote_chip_id = PLATFORM_CHIP_2,
        .enable_smp_mode = true,
        .enable_direct_connect_mode = true,
        .enable_cpa_mode = true,
        .cpag_config = {
            .cpag_id = 1,
            .ccg_count = CCG_PER_CPAG,
        },
    },
    {
        /* Chip 0 to Chip 3 */
        .ccg_ldid = CCG_LDID_LIST(CCG_PORT_04),
        .haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_04),
        .remote_mmap_table = {
            {
                /*
                 * Chip 3 peripheral address space and DRAM.
                 * Map: 0x30_0000_0000 - 0x3F_FFFF_FFFF (primary)
                 *      0x500_0000_0000 - 0x5FF_FFFF_FFFF (secondary)
                 */
                .region_mmap = REMOTE_HASHED_REGION(CHIP_PERIPH_MEM_BASE(3),
                                PERIPH_MEM_SIZE, CHIP_DRAM2_BASE(3),
                                DRAM2_SIZE, MESH_START_POS, MESH_END_POS),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_3, CCG_PORT_04),
            },
            {
                /*
                 * Chip 3 PCIe MMIO_L space.
                 * Map: 0x00_7800_0000 - 0x00_7FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOL_BASE(3),
                                PCI_MMIOL_SIZE, NODE_ID_CCG04),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_3, CCG_PORT_04),
            },
            {
                /*
                 * Chip 3 PCIe ECAM space.
                 * Map: 0x100_3000_0000 - 0x100_3FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_ECAM_BASE(3),
                                PCI_ECAM_SIZE, NODE_ID_CCG04),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_3, CCG_PORT_04),
            },
            {
                /*
                 * Chip 3 PCIe MMIO_H space.
                 * Map: 0x131_0000_0000 - 0x140_FFFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOH_BASE(3),
                                PCI_MMIOH_SIZE, NODE_ID_CCG04),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_3, CCG_PORT_04),
            },
        },
        .remote_chip_id = PLATFORM_CHIP_3,
        .enable_smp_mode = true,
        .enable_direct_connect_mode = true,
        .enable_cpa_mode = true,
        .cpag_config = {
            .cpag_id = 2,
            .ccg_count = CCG_PER_CPAG,
        },
    },
};

/* Chip 1 CML configuration */
static const struct mod_cmn_cyprus_cml_config chip1_cml_config[] = {
    {
        /* Chip 1 to Chip 0 */
        .ccg_ldid = CCG_LDID_LIST(CCG_PORT_01),
        .haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_01),
        .remote_mmap_table = {
            {
                /*
                 * Chip 0 peripheral address space and DRAM.
                 * Map: 0x0 - 0x0F_FFFF_FFFF (primary)
                 *      0x200_0000_0000 - 0x2FF_FFFF_FFFF (secondary)
                 */
                .region_mmap = REMOTE_HASHED_REGION(CHIP_PERIPH_MEM_BASE(0),
                                PERIPH_MEM_SIZE, CHIP_DRAM2_BASE(0),
                                DRAM2_SIZE, MESH_START_POS, MESH_END_POS),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_00),
            },
            {
                /*
                 * Chip 0 PCIe MMIO_L space.
                 * Map: 0x00_6000_0000 - 0x00_6FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOL_BASE(0),
                                PCI_MMIOL_SIZE, NODE_ID_CCG01),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_00),
            },
            {
                /*
                 * Chip 0 PCIe ECAM space.
                 * Map: 0x100_0000_0000 - 0x100_0FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_ECAM_BASE(0),
                                PCI_ECAM_SIZE, NODE_ID_CCG01),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_00),
            },
            {
                /*
                 * Chip 0 PCIe MMIO_H space.
                 * Map: 0x101_0000_0000 - 0x110_FFFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOH_BASE(0),
                                PCI_MMIOH_SIZE, NODE_ID_CCG01),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_00),
            },
        },
        .remote_chip_id = PLATFORM_CHIP_0,
        .enable_smp_mode = true,
        .enable_direct_connect_mode = true,
        .enable_cpa_mode = true,
        .cpag_config = {
            .cpag_id = 0,
            .ccg_count = CCG_PER_CPAG,
        },
    },
    {
        /* Chip 1 to Chip 2 */
        .ccg_ldid = CCG_LDID_LIST(CCG_PORT_02),
        .haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_02),
        .remote_mmap_table = {
            {
                /*
                 * Chip 2 peripheral address space and DRAM.
                 * Map: 0x20_0000_0000 - 0x2F_FFFF_FFFF (primary)
                 *      0x400_0000_0000 - 0x4FF_FFFF_FFFF (secondary)
                 */
                .region_mmap = REMOTE_HASHED_REGION(CHIP_PERIPH_MEM_BASE(2),
                                PERIPH_MEM_SIZE, CHIP_DRAM2_BASE(2),
                                DRAM2_SIZE, MESH_START_POS, MESH_END_POS),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_2, CCG_PORT_02),
            },
            {
                /*
                 * Chip 2 PCIe MMIO_L space.
                 * Map: 0x00_7000_0000 - 0x00_77FF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOL_BASE(2),
                                PCI_MMIOL_SIZE, NODE_ID_CCG02),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_2, CCG_PORT_02),
            },
            {
                /*
                 * Chip 2 PCIe ECAM space.
                 * Map: 0x100_2000_0000 - 0x100_2FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_ECAM_BASE(2),
                                PCI_ECAM_SIZE, NODE_ID_CCG02),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_2, CCG_PORT_02),
            },
            {
                /*
                 * Chip 2 PCIe MMIO_H space.
                 * Map: 0x121_0000_0000 - 0x130_FFFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOH_BASE(2),
                                PCI_MMIOH_SIZE, NODE_ID_CCG02),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_2, CCG_PORT_02),
            },
        },
        .remote_chip_id = PLATFORM_CHIP_2,
        .enable_smp_mode = true,
        .enable_direct_connect_mode = true,
        .enable_cpa_mode = true,
        .cpag_config = {
            .cpag_id = 1,
            .ccg_count = CCG_PER_CPAG,
        },
    },
    {
        /* Chip 1 to Chip 3 */
        .ccg_ldid = CCG_LDID_LIST(CCG_PORT_03),
        .haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_03),
        .remote_mmap_table = {
            {
                /*
                 * Chip 3 peripheral address space and DRAM.
                 * Map: 0x30_0000_0000 - 0x3F_FFFF_FFFF (primary)
                 *      0x500_0000_0000 - 0x5FF_FFFF_FFFF (secondary)
                 */
                .region_mmap = REMOTE_HASHED_REGION(CHIP_PERIPH_MEM_BASE(3),
                                PERIPH_MEM_SIZE, CHIP_DRAM2_BASE(3),
                                DRAM2_SIZE, MESH_START_POS, MESH_END_POS),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_3, CCG_PORT_03),
            },
            {
                /*
                 * Chip 3 PCIe MMIO_L space.
                 * Map: 0x00_7800_0000 - 0x00_7FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOL_BASE(3),
                                PCI_MMIOL_SIZE, NODE_ID_CCG03),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_3, CCG_PORT_03),
            },
            {
                /*
                 * Chip 3 PCIe ECAM space.
                 * Map: 0x100_3000_0000 - 0x100_3FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_ECAM_BASE(3),
                                PCI_ECAM_SIZE, NODE_ID_CCG03),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_3, CCG_PORT_03),
            },
            {
                /*
                 * Chip 3 PCIe MMIO_H space.
                 * Map: 0x131_0000_0000 - 0x140_FFFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOH_BASE(3),
                                PCI_MMIOH_SIZE, NODE_ID_CCG03),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_3, CCG_PORT_03),
            },
        },
        .remote_chip_id = PLATFORM_CHIP_3,
        .enable_smp_mode = true,
        .enable_direct_connect_mode = true,
        .enable_cpa_mode = true,
        .cpag_config = {
            .cpag_id = 2,
            .ccg_count = CCG_PER_CPAG,
        },
    },
};

/* Chip 2 CML configuration */
static const struct mod_cmn_cyprus_cml_config chip2_cml_config[] = {
    {
        /* Chip 2 to Chip 0 */
        .ccg_ldid = CCG_LDID_LIST(CCG_PORT_03),
        .haid = CCG_HAID_LIST(PLATFORM_CHIP_2, CCG_PORT_03),
        .remote_mmap_table = {
            {
                /*
                 * Chip 0 peripheral address space and DRAM.
                 * Map: 0x0 - 0x0F_FFFF_FFFF (primary)
                 *      0x200_0000_0000 - 0x2FF_FFFF_FFFF (secondary)
                 */
                .region_mmap = REMOTE_HASHED_REGION(CHIP_PERIPH_MEM_BASE(0),
                                PERIPH_MEM_SIZE, CHIP_DRAM2_BASE(0),
                                DRAM2_SIZE, MESH_START_POS, MESH_END_POS),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_03),
            },
            {
                /*
                 * Chip 0 PCIe MMIO_L space.
                 * Map: 0x00_6000_0000 - 0x00_6FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOL_BASE(0),
                                PCI_MMIOL_SIZE, NODE_ID_CCG03),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_03),
            },
            {
                /*
                 * Chip 0 PCIe ECAM space.
                 * Map: 0x100_0000_0000 - 0x100_0FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_ECAM_BASE(0),
                                PCI_ECAM_SIZE, NODE_ID_CCG03),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_03),
            },
            {
                /*
                 * Chip 0 PCIe MMIO_H space.
                 * Map: 0x101_0000_0000 - 0x110_FFFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOH_BASE(0),
                                PCI_MMIOH_SIZE, NODE_ID_CCG03),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_03),
            },
        },
        .remote_chip_id = PLATFORM_CHIP_0,
        .enable_smp_mode = true,
        .enable_direct_connect_mode = true,
        .enable_cpa_mode = true,
        .cpag_config = {
            .cpag_id = 0,
            .ccg_count = CCG_PER_CPAG,
        },
    },
    {
        /* Chip 2 to Chip 1 */
        .ccg_ldid = CCG_LDID_LIST(CCG_PORT_02),
        .haid = CCG_HAID_LIST(PLATFORM_CHIP_2, CCG_PORT_02),
        .remote_mmap_table = {
            {
                /*
                 * Chip 1 peripheral address space and DRAM.
                 * Map: 0x10_0000_0000 - 0x1F_FFFF_FFFF (primary)
                 *      0x300_0000_0000 - 0x3FF_FFFF_FFFF (secondary)
                 */
                .region_mmap = REMOTE_HASHED_REGION(CHIP_PERIPH_MEM_BASE(1),
                                PERIPH_MEM_SIZE, CHIP_DRAM2_BASE(1),
                                DRAM2_SIZE, MESH_START_POS, MESH_END_POS),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_02),
            },
            {
                /*
                 * Chip 1 PCIe MMIO_L space.
                 * Map: 0x00_6800_0000 - 0x00_67FF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOL_BASE(1),
                                PCI_MMIOL_SIZE, NODE_ID_CCG02),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_02),
            },
            {
                /*
                 * Chip 1 PCIe ECAM space.
                 * Map: 0x100_1000_0000 - 0x100_1FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_ECAM_BASE(1),
                                PCI_ECAM_SIZE, NODE_ID_CCG02),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_02),
            },
            {
                /*
                 * Chip 1 PCIe MMIO_H space.
                 * Map: 0x111_0000_0000 - 0x120_FFFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOH_BASE(1),
                                PCI_MMIOH_SIZE, NODE_ID_CCG02),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_02),
            },
        },
        .remote_chip_id = PLATFORM_CHIP_1,
        .enable_smp_mode = true,
        .enable_direct_connect_mode = true,
        .enable_cpa_mode = true,
        .cpag_config = {
            .cpag_id = 1,
            .ccg_count = CCG_PER_CPAG,
        },
    },
    {
        /* Chip 2 to Chip 3 */
        .ccg_ldid = CCG_LDID_LIST(CCG_PORT_00),
        .haid = CCG_HAID_LIST(PLATFORM_CHIP_2, CCG_PORT_00),
        .remote_mmap_table = {
            {
                /*
                 * Chip 3 peripheral address space and DRAM.
                 * Map: 0x30_0000_0000 - 0x3F_FFFF_FFFF (primary)
                 *      0x500_0000_0000 - 0x5FF_FFFF_FFFF (secondary)
                 */
                .region_mmap = REMOTE_HASHED_REGION(CHIP_PERIPH_MEM_BASE(3),
                                PERIPH_MEM_SIZE, CHIP_DRAM2_BASE(3),
                                DRAM2_SIZE, MESH_START_POS, MESH_END_POS),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_3, CCG_PORT_01),
            },
            {
                /*
                 * Chip 3 PCIe MMIO_L space.
                 * Map: 0x00_7800_0000 - 0x00_7FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOL_BASE(3),
                                PCI_MMIOL_SIZE, NODE_ID_CCG00),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_3, CCG_PORT_01),
            },
            {
                /*
                 * Chip 3 PCIe ECAM space.
                 * Map: 0x100_3000_0000 - 0x100_3FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_ECAM_BASE(3),
                                PCI_ECAM_SIZE, NODE_ID_CCG00),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_3, CCG_PORT_01),
            },
            {
                /*
                 * Chip 3 PCIe MMIO_H space.
                 * Map: 0x131_0000_0000 - 0x140_FFFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOH_BASE(3),
                                PCI_MMIOH_SIZE, NODE_ID_CCG00),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_3, CCG_PORT_01),
            },
        },
        .remote_chip_id = PLATFORM_CHIP_3,
        .enable_smp_mode = true,
        .enable_direct_connect_mode = true,
        .enable_cpa_mode = true,
        .cpag_config = {
            .cpag_id = 2,
            .ccg_count = CCG_PER_CPAG,
        },
    },
};

/* Chip 3 CML configuration */
static const struct mod_cmn_cyprus_cml_config chip3_cml_config[] = {
    {
        /* Chip 3 to Chip 0 */
        .ccg_ldid = CCG_LDID_LIST(CCG_PORT_04),
        .haid = CCG_HAID_LIST(PLATFORM_CHIP_3, CCG_PORT_04),
        .remote_mmap_table = {
            {
                /*
                 * Chip 0 peripheral address space and DRAM.
                 * Map: 0x0 - 0x0F_FFFF_FFFF (primary)
                 *      0x200_0000_0000 - 0x2FF_FFFF_FFFF (secondary)
                 */
                .region_mmap = REMOTE_HASHED_REGION(CHIP_PERIPH_MEM_BASE(0),
                                PERIPH_MEM_SIZE, CHIP_DRAM2_BASE(0),
                                DRAM2_SIZE, MESH_START_POS, MESH_END_POS),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_04),
            },
            {
                /*
                 * Chip 0 PCIe MMIO_L space.
                 * Map: 0x00_6000_0000 - 0x00_6FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOL_BASE(0),
                                PCI_MMIOL_SIZE, NODE_ID_CCG04),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_04),
            },
            {
                /*
                 * Chip 0 PCIe ECAM space.
                 * Map: 0x100_0000_0000 - 0x100_0FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_ECAM_BASE(0),
                                PCI_ECAM_SIZE, NODE_ID_CCG04),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_04),
            },
            {
                /*
                 * Chip 0 PCIe MMIO_H space.
                 * Map: 0x101_0000_0000 - 0x110_FFFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOH_BASE(0),
                                PCI_MMIOH_SIZE, NODE_ID_CCG04),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_0, CCG_PORT_04),
            },
        },
        .remote_chip_id = PLATFORM_CHIP_0,
        .enable_smp_mode = true,
        .enable_direct_connect_mode = true,
        .enable_cpa_mode = true,
        .cpag_config = {
            .cpag_id = 0,
            .ccg_count = CCG_PER_CPAG,
        },
    },
    {
        /* Chip 3 to Chip 1 */
        .ccg_ldid = CCG_LDID_LIST(CCG_PORT_03),
        .haid = CCG_HAID_LIST(PLATFORM_CHIP_3, CCG_PORT_03),
        .remote_mmap_table = {
            {
                /*
                 * Chip 1 peripheral address space and DRAM.
                 * Map: 0x10_0000_0000 - 0x1F_FFFF_FFFF (primary)
                 *      0x300_0000_0000 - 0x3FF_FFFF_FFFF (secondary)
                 */
                .region_mmap = REMOTE_HASHED_REGION(CHIP_PERIPH_MEM_BASE(1),
                                PERIPH_MEM_SIZE, CHIP_DRAM2_BASE(1),
                                DRAM2_SIZE, MESH_START_POS, MESH_END_POS),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_03),
            },
            {
                /*
                 * Chip 1 PCIe MMIO_L space.
                 * Map: 0x00_6800_0000 - 0x00_67FF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOL_BASE(1),
                                PCI_MMIOL_SIZE, NODE_ID_CCG03),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_03),
            },
            {
                /*
                 * Chip 1 PCIe ECAM space.
                 * Map: 0x100_1000_0000 - 0x100_1FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_ECAM_BASE(1),
                                PCI_ECAM_SIZE, NODE_ID_CCG03),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_03),
            },
            {
                /*
                 * Chip 1 PCIe MMIO_H space.
                 * Map: 0x111_0000_0000 - 0x120_FFFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOH_BASE(1),
                                PCI_MMIOH_SIZE, NODE_ID_CCG03),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_1, CCG_PORT_03),
            },
        },
        .remote_chip_id = PLATFORM_CHIP_1,
        .enable_smp_mode = true,
        .enable_direct_connect_mode = true,
        .enable_cpa_mode = true,
        .cpag_config = {
            .cpag_id = 1,
            .ccg_count = CCG_PER_CPAG,
        },
    },
    {
        /* Chip 3 to Chip 2 */
        .ccg_ldid = CCG_LDID_LIST(CCG_PORT_01),
        .haid = CCG_HAID_LIST(PLATFORM_CHIP_3, CCG_PORT_01),
        .remote_mmap_table = {
            {
                /*
                 * Chip 2 peripheral address space and DRAM.
                 * Map: 0x20_0000_0000 - 0x2F_FFFF_FFFF (primary)
                 *      0x400_0000_0000 - 0x4FF_FFFF_FFFF (secondary)
                 */
                .region_mmap = REMOTE_HASHED_REGION(CHIP_PERIPH_MEM_BASE(2),
                                PERIPH_MEM_SIZE, CHIP_DRAM2_BASE(2),
                                DRAM2_SIZE, MESH_START_POS, MESH_END_POS),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_2, CCG_PORT_00),
            },
            {
                /*
                 * Chip 2 PCIe MMIO_L space.
                 * Map: 0x00_7000_0000 - 0x00_77FF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOL_BASE(2),
                                PCI_MMIOL_SIZE, NODE_ID_CCG01),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_2, CCG_PORT_00),
            },
            {
                /*
                 * Chip 2 PCIe ECAM space.
                 * Map: 0x100_2000_0000 - 0x100_2FFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_ECAM_BASE(2),
                                PCI_ECAM_SIZE, NODE_ID_CCG01),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_2, CCG_PORT_00),
            },
            {
                /*
                 * Chip 2 PCIe MMIO_H space.
                 * Map: 0x121_0000_0000 - 0x130_FFFF_FFFF
                 */
                .region_mmap = REMOTE_NH_REGION(CHIP_PCIE_MMIOH_BASE(2),
                                PCI_MMIOH_SIZE, NODE_ID_CCG01),
                .target_haid = CCG_HAID_LIST(PLATFORM_CHIP_2, CCG_PORT_00),
            },
        },
        .remote_chip_id = PLATFORM_CHIP_2,
        .enable_smp_mode = true,
        .enable_direct_connect_mode = true,
        .enable_cpa_mode = true,
        .cpag_config = {
            .cpag_id = 2,
            .ccg_count = CCG_PER_CPAG,
        },
    },
};
#endif

static struct mod_cmn_cyprus_config cmn_config_table[] = {
    [PLATFORM_CHIP_0] = {
        .periphbase = SCP_CMN_BASE,
        .mesh_size_x = MESH_SIZE_X,
        .mesh_size_y = MESH_SIZE_Y,
        .hnf_sam_config = {
            .snf_table = snf_table,
            .snf_count = FWK_ARRAY_SIZE(snf_table),
#if (PLATFORM_VARIANT == RD_V3_R1_CFG1)
            .hnf_sam_mode = MOD_CMN_CYPRUS_HNF_SAM_MODE_DIRECT_MAPPING,
        },
        .rnsam_scg_config = {
            .scg_hashing_mode = MOD_CMN_CYPRUS_RNSAM_SCG_POWER_OF_TWO_HASHING,
        },
#else
            .hnf_sam_mode = MOD_CMN_CYPRUS_HNF_SAM_MODE_RANGE_BASED_HASHING,
            .hashed_mode_config = {
                .sn_mode = MOD_CMN_CYPRUS_HNF_SAM_HASHED_MODE_3_SN,
                .top_address_bit0 = 32,
                .top_address_bit1 = 33,
            },
        },
        .isolated_hns_table = isolated_hns_nodes,
        .isolated_hns_count = FWK_ARRAY_SIZE(isolated_hns_nodes),
        .rnsam_scg_config = {
            .scg_hashing_mode = MOD_CMN_CYPRUS_RNSAM_SCG_HIERARCHICAL_HASHING,
            .hier_hash_cfg = {
                .num_cluster_groups = 4,
            },
        },
#endif
        .hns_cal_mode = true,
        .mmap_table = mmap,
        .mmap_count = FWK_ARRAY_SIZE(mmap),
        .chip_addr_space = UINT64_C(64) * FWK_GIB,
        .cml_config_table = chip0_cml_config,
        .cml_table_count =
            FWK_ARRAY_SIZE(chip0_cml_config),
        .cml_poll_timeout_us = UINT32_C(100),
        .enable_lcn = true,
    },
    [PLATFORM_CHIP_1] = {
        .periphbase = SCP_CMN_BASE,
        .mesh_size_x = MESH_SIZE_X,
        .mesh_size_y = MESH_SIZE_Y,
        .hnf_sam_config = {
            .snf_table = snf_table,
            .snf_count = FWK_ARRAY_SIZE(snf_table),
#if (PLATFORM_VARIANT == RD_V3_R1_CFG1)
            .hnf_sam_mode = MOD_CMN_CYPRUS_HNF_SAM_MODE_DIRECT_MAPPING,
        },
        .rnsam_scg_config = {
            .scg_hashing_mode = MOD_CMN_CYPRUS_RNSAM_SCG_POWER_OF_TWO_HASHING,
        },
#else
            .hnf_sam_mode = MOD_CMN_CYPRUS_HNF_SAM_MODE_RANGE_BASED_HASHING,
            .hashed_mode_config = {
                .sn_mode = MOD_CMN_CYPRUS_HNF_SAM_HASHED_MODE_3_SN,
                .top_address_bit0 = 32,
                .top_address_bit1 = 33,
            },
        },
        .isolated_hns_table = isolated_hns_nodes,
        .isolated_hns_count = FWK_ARRAY_SIZE(isolated_hns_nodes),
        .rnsam_scg_config = {
            .scg_hashing_mode = MOD_CMN_CYPRUS_RNSAM_SCG_HIERARCHICAL_HASHING,
            .hier_hash_cfg = {
                .num_cluster_groups = 4,
            },
        },
#endif
        .hns_cal_mode = true,
        .mmap_table = mmap,
        .mmap_count = FWK_ARRAY_SIZE(mmap),
        .chip_addr_space = UINT64_C(64) * FWK_GIB,
        .cml_config_table = chip1_cml_config,
        .cml_table_count =
            FWK_ARRAY_SIZE(chip1_cml_config),
        .cml_poll_timeout_us = UINT32_C(100),
        .enable_lcn = true,
    },
#if (PLATFORM_VARIANT == RD_V3_R1_CFG1)
    [PLATFORM_CHIP_2] = {
        .periphbase = SCP_CMN_BASE,
        .mesh_size_x = MESH_SIZE_X,
        .mesh_size_y = MESH_SIZE_Y,
        .hnf_sam_config = {
            .snf_table = snf_table,
            .snf_count = FWK_ARRAY_SIZE(snf_table),
            .hnf_sam_mode = MOD_CMN_CYPRUS_HNF_SAM_MODE_DIRECT_MAPPING,
        },
        .rnsam_scg_config = {
            .scg_hashing_mode = MOD_CMN_CYPRUS_RNSAM_SCG_POWER_OF_TWO_HASHING,
        },
        .hns_cal_mode = true,
        .mmap_table = mmap,
        .mmap_count = FWK_ARRAY_SIZE(mmap),
        .chip_addr_space = UINT64_C(64) * FWK_GIB,
        .cml_config_table = chip2_cml_config,
        .cml_table_count =
            FWK_ARRAY_SIZE(chip2_cml_config),
        .cml_poll_timeout_us = UINT32_C(100),
        .enable_lcn = true,
    },
    [PLATFORM_CHIP_3] = {
        .periphbase = SCP_CMN_BASE,
        .mesh_size_x = MESH_SIZE_X,
        .mesh_size_y = MESH_SIZE_Y,
        .hnf_sam_config = {
            .snf_table = snf_table,
            .snf_count = FWK_ARRAY_SIZE(snf_table),
            .hnf_sam_mode = MOD_CMN_CYPRUS_HNF_SAM_MODE_DIRECT_MAPPING,
        },
        .rnsam_scg_config = {
            .scg_hashing_mode = MOD_CMN_CYPRUS_RNSAM_SCG_POWER_OF_TWO_HASHING,
        },
        .hns_cal_mode = true,
        .mmap_table = mmap,
        .mmap_count = FWK_ARRAY_SIZE(mmap),
        .chip_addr_space = UINT64_C(64) * FWK_GIB,
        .cml_config_table = chip3_cml_config,
        .cml_table_count =
            FWK_ARRAY_SIZE(chip3_cml_config),
        .cml_poll_timeout_us = UINT32_C(100),
        .enable_lcn = true,
    },
#endif
};

struct mod_cmn_cyprus_config_table cmn_config = {
    .chip_config_data = cmn_config_table,
    .chip_count = PLATFORM_CHIP_COUNT,
    .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
};

const struct fwk_module_config config_cmn_cyprus = {
    .data = (void *)&cmn_config,
};
