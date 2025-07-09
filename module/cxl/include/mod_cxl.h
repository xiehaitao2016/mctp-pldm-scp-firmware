/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     CXL module header file.
 */

#ifndef MOD_CXL_H
#define MOD_CXL_H

#include <fwk_macros.h>

#include <stdbool.h>
#include <stdint.h>

#define WORD_MASK     UINT64_C(0xFFFFFFFF)
#define WORD_SHIFT    UINT64_C(32)

/* Extended Capabilities (PCI-X 2.0 and Express) */
#define PCI_EXT_CAP_ID(header)         ((header) & 0x0000ffff)
#define PCI_EXT_CAP_VER(header)        (((header) >> 16) & 0xf)
#define PCI_EXT_CAP_NEXT(header)       (((header) >> 20) & 0xfff)
#define PCI_CFG_SPACE_SIZE             256
#define PCI_CFG_SPACE_EXP_SIZE         4096

/* PCIe DVSEC for CXL Device */
#define DVSEC_CXL_HEADER1              0x4
#define DVSEC_VENDOR_ID_MASK           0xFFFF
#define DVSEC_CXL_VENDOR_ID            0x1E98
#define DVSEC_REVISION_MASK            0xF
#define DVSEC_REVISION_BIT_POS         16
#define DVSEC_CXL_REVISION             0x1

#define DVSEC_CXL_HEADER2              0x8
#define DVSEC_CXL_DEVICE_ID_MASK       0xFFFF
#define DVSEC_CXL_DEVICE_ID            0x0

#define DVSEC_CXL_CTRL_STATUS          0xC
#define DVSEC_CXL_CTRL_STATUS_2        0x10
#define DVSEC_CXL_LOCK_CAP2            0x14
#define DVSEC_CXL_RANGE_1_SIZE_HIGH    0x18
#define DVSEC_CXL_RANGE_1_SIZE_LOW     0x1C
#define DVSEC_CXL_RANGE_1_BASE_HIGH    0x20
#define DVSEC_CXL_RANGE_1_BASE_LOW     0x24
#define DVSEC_CXL_RANGE_2_SIZE_HIGH    0x28
#define DVSEC_CXL_RANGE_2_SIZE_LOW     0x2C
#define DVSEC_CXL_RANGE_2_BASE_HIGH    0x30
#define DVSEC_CXL_RANGE_2_BASE_LOW     0x34

#define CXL_DEVICE_CAP_ARRAY_REG1      0x0
#define CXL_DEVICE_CAP_ARRAY_REG2      0x4
#define CXL_DEVICE_CAP1_HEADER_REG1    0x10
#define CXL_DEVICE_CAP1_HEADER_REG2    0x14
#define CXL_DEVICE_CAP1_HEADER_REG3    0x18
#define CXL_DEVICE_CAP2_HEADER_REG1    0x20
#define CXL_DEVICE_CAP2_HEADER_REG2    0x24
#define CXL_DEVICE_CAP2_HEADER_REG3    0x28

#define MAILBOX_CAP_REG                0x0
#define MAILBOX_CONTROL_REG            0x4
#define MAILBOX_COMMAND_REG1           0x8
#define MAILBOX_COMMAND_REG2           0xC
#define MAILBOX_STATUS_REG1            0x10
#define MAILBOX_STATUS_REG2            0x14
#define MAILBOX_BG_CMD_STATUS_REG      0x18
#define MAILBOX_COMMAND_PAYLOAD_REG    0x20

#define CXLDEV_MBOX_CTRL_DOORBELL      BIT(0)

#define DOE_DATA_OBJ_HEADER_1          0x0
#define DOE_DATA_OBJ_HEADER_2          0x4

#define DOE_CAPABILITIES_REG           0x4
#define DOE_CONTROL_REG                0x8
#define DOE_STATUS_REG                 0xC
#define DOE_WRITE_DATA_MAILBOX_REG     0x10
#define DOE_READ_DATA_MAILBOX_REG      0x14

#define PCIE_DVSEC_ID                  0x23
#define PCIE_DOE_ID                    0x2E

#define DOE_DATA_OBJECT_LENGTH         0x0003ffff
#define CXL_DOE_TABLE_ENTRY_HANDLE     0xffff0000
#define CXL_DOE_TABLE_ENTRY_HANDLE_LAST 0xffff

#define   DOE_STAT_DOE_BUSY            UINT32_C(0x1)
#define   DOE_STAT_DATA_OBJ_READY      (UINT32_C(0x1) << 31)
#define   DOE_CTRL_DOE_GO              (UINT32_C(0x1) << 31)

/*!
 * \brief Data object header
 *
 * \details Data Object Exchange(DOE) Header1 and Header2 put together.
 *      Reference taken from PCI-SIG ECN and
 *      CXL Specification (Revision 3.0, Version 0.7).
 */
struct DOEHeader {
    uint16_t vendor_id;
    uint8_t data_obj_type;
    uint8_t reserved;
    uint32_t length;
};

#define DOE_DATA_OBJ_TYPE_COMPLIANCE   0x0
#define DOE_DATA_OBJ_TYPE_CDAT         0x2

/*!
 * \brief DOE read request data
 *
 * \details DOE read request data structure. For CXL, DOE requests are made
 *      to read CDAT tables.
 *      Reference taken from CXL Specification (Revision 3.0, Version 0.7).
 */
struct cxl_cdat_read_entry_req {
    struct DOEHeader header;
    uint8_t req_code;
    uint8_t table_type;
    uint16_t entry_handle;
};

#define CXL_CDAT_DOE_ENTRYHANDLE_LAST_ENTRY  0xFFFF
/* Size in DW(4 Bytes) */
#define CDAT_READ_ENTRY_REQ_SIZE    3

/*!
 * \brief DOE read response data
 *
 * \details DOE read response data structure. For CXL, DOE responses carry
 *      information about CDAT tables.
 *      Reference taken from CXL Specification (Revision 3.0, Version 0.7).
 */
struct cxl_cdat_read_entry_resp {
    struct DOEHeader header;
    uint8_t rsp_code;
    uint8_t table_type;
    uint16_t entry_handle;
    uint32_t cdat_table[32];
};

/* Size in DW(4 Bytes) */
#define CDAT_READ_ENTRY_RESP_SIZE    3

/*!
 * \brief Coherent Device Attribute Table(CDAT) Header
 *
 * \details CDAT header, which is followed by variable number of CDAT structures.
 *      Reference taken from CDAT Specification (Revision 1.02).
 */
struct cdat_table_header {
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t reserved[6];
    uint32_t sequence;
};

/* Size in DW(4 Bytes) */
#define CDAT_TABLE_HEADER_SIZE    4

/*!
 * \brief Device Scoped Memory Affinity Structure (DSMAS)
 *
 * \details DSMAS returns Device Physical Address(DPA) range and it's attributes.
 *      Reference taken from CDAT Specification (Revision 1.02).
 */
struct cdat_dsmas {
    uint8_t type;
    uint8_t reserved_1;
    uint16_t length;
    uint8_t DSMADhandle;
    uint8_t flags;
    uint16_t reserved_2;
    uint64_t DPA_base;
    uint64_t DPA_length;
};

/* Size in DW(4 Bytes) */
#define CDAT_STRUCTURE_DSMAS_SIZE   6

/* Size in DW(4 Bytes) */
#define CDAT_STRUCTURE_DSLBIS_SIZE   6

struct cxl_cdat_dsmas_doe_resp {
    struct cxl_cdat_read_entry_resp resp;
    struct cdat_table_header header;
    struct cdat_dsmas       dsmas;
};

/*!
 * \brief Type of CDAT structures
 */
enum cdat_structure_type {
    CDAT_STRUCTURE_DSMAS = 0,
    CDAT_STRUCTURE_DSLBIS,
    CDAT_STRUCTURE_DSMSCIS,
    CDAT_STRUCTURE_DSIS,
    CDAT_STRUCTURE_DSEMTS,
    CDAT_STRUCTURE_SSLBIS,
    CDAT_STRUCTURE_COUNT
};

struct cxl_mem_region {
    uint64_t cxl_mem_base;
    uint64_t free_cxl_mem_base;
    uint64_t cxl_mem_size;
};

/*!
 * \brief Module interface.
 */
struct mod_cxl_api {
    /*!
     * \brief Find out whether a PCIe device supports CXL extended capability.
     *
     * \param Ecam base address of PCIe device, which will be  checked for
     *      device capability.
     */
    void (*find_cxl_cap)(uint64_t config_base);

    /*!
     * \brief Call CMN api to map host address space to CXL mem region.
     */
    void (*map_cxl_mem_region)(void);
};

/*!
 * \brief Remote memory details
 *
 * \details It will hold combined details of host address space and
 *      device address space.
 */
struct remote_memory_config {
    uint32_t  addressbaselow;
    uint32_t  addressbasehigh;
    uint32_t  lengthlow;
    uint32_t  lengthhigh;
};

void update_cxl_mem_region(uint64_t mem_base, uint64_t size);
#endif /* MOD_CXL_H */
