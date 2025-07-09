/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     CXL module for discovering CXL capable PCIe device.
 *     Execute DOE operation.
 */

#include <interface_cmn.h>

#include <mod_apremap.h>
#include <mod_cxl.h>
#include <mod_pcie_discovery.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_list.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#define MOD_NAME "[PCIE_CXL] "

#define MAX_REMOTE_MEM  5

/* AP remap Adress read write APIs */
static struct interface_address_remapper_rw_api *remapper_rw_api;

/* APIs to configure CMN700 moudle */
static struct interface_cmn_memmap_rnsam_api *cmn_api;

/* AP remap Address translation enable/disable APIs */
struct mod_apremap_cmn_atrans_api *apremap_cmn_atrans_api;

static struct remote_memory_config remote_mem[MAX_REMOTE_MEM];
static uint32_t next_remote_mem;
static struct cxl_mem_region host_cxl_mem;
static uint64_t remote_mem_size;

int is_doe_busy(uint64_t doe_stat_reg,
                struct pcie_discovery_rw_api rw_api)
{
    uint32_t doe_stat_val;

    doe_stat_val = rw_api.read32(doe_stat_reg);
    if (doe_stat_val & DOE_STAT_DOE_BUSY)
        return FWK_E_BUSY;

    return FWK_SUCCESS;
}

static void update_cdat_data(uint32_t *doe_resp_cdat_data, uint16_t length)
{
    uint32_t cur_index;
    struct cdat_dsmas *dsmas = NULL;
    uint64_t dev_phys_addr;

    /* skipping the CDAT header. */
    cur_index = 4;
    while (cur_index < length) {
        switch ((doe_resp_cdat_data[cur_index] & 0xff)) {
        case CDAT_STRUCTURE_DSMAS:
            dsmas = (struct cdat_dsmas *)(&(doe_resp_cdat_data[cur_index]));
            dev_phys_addr = host_cxl_mem.free_cxl_mem_base + dsmas->DPA_base;
            remote_mem[next_remote_mem].addressbaselow =
                dev_phys_addr & WORD_MASK;
            remote_mem[next_remote_mem].addressbasehigh =
                (dev_phys_addr >> WORD_SHIFT) & WORD_MASK;
            remote_mem[next_remote_mem].lengthlow =
                dsmas->DPA_length & WORD_MASK;
            remote_mem[next_remote_mem].lengthhigh =
                dsmas->DPA_length >> WORD_SHIFT;
            host_cxl_mem.free_cxl_mem_base += dsmas->DPA_length;
            next_remote_mem++;
            cur_index += CDAT_STRUCTURE_DSMAS_SIZE;
            remote_mem_size += dsmas->DPA_length;
            break;
        case CDAT_STRUCTURE_DSLBIS:
            cur_index += CDAT_STRUCTURE_DSLBIS_SIZE;
            break;
        default:
            break;
        }
    }

    return;
}

/* Receive DOE response data a DWORD at a time. */

static uint16_t doe_receive_response(uint64_t doe_base,
                             struct pcie_discovery_rw_api rw_api)
{
    uint32_t doe_resp_cdat_data[15] = {};
    uint32_t cur_index = 0;
    uint32_t data;
    uint32_t val;
    uint64_t reg;
    uint16_t length;
    uint16_t entry_handle = 0;

    reg = doe_base + DOE_STATUS_REG;
    val = rw_api.read32(reg);

    if (val & DOE_STAT_DATA_OBJ_READY) {
        cur_index = 0;
        reg = doe_base + DOE_READ_DATA_MAILBOX_REG;

        /* Read the DOE header. */
        data = rw_api.read32(reg);
        rw_api.write32(reg, 0x1);

        /* Read the DOE Header 2 for data length. */
        data = rw_api.read32(reg);
        length = data & DOE_DATA_OBJECT_LENGTH;
        rw_api.write32(reg, 0x1);

        if (length < 2) {
            FWK_LOG_WARN(MOD_NAME " DOE data read error \n");
            return 0;
        }

        /* Read DOE read entry response header. */
        data = rw_api.read32(reg);
        entry_handle = ((data & CXL_DOE_TABLE_ENTRY_HANDLE) >> 16);
        rw_api.write32(reg, 0x1);

        length -= 3;
        while (cur_index < length) {
            doe_resp_cdat_data[cur_index] = rw_api.read32(reg);
            cur_index++;
            rw_api.write32(reg, 0x1);
        }

        update_cdat_data(doe_resp_cdat_data, length);
    }

    return entry_handle;
}

/*
 * Execute DOE command to fetch CDAT structures.
 *
 * The steps include -
 * 1. System firmware checks that the DOE Busy bit is Clear.
 * 2. System firmware writes entire data object a DWORD at a time via
 *    DOE Write Data Mailbox register.
 * 3. System firmware writes 1b to the DOE Go bit.
 * 4. The DOE instance consumes the DOE request from the DOE mailbox.
 * 5. The DOE instance generates a DOE Response and Sets Data Object Ready bit.
 * 6. System firmware polls the Data Object Ready bit and, provided it is Set,
 *    reads data from the DOE Read Data Mailbox and writes to the DOE Read Data
 *    Mailbox to indicate a successful read, a DWORD at a time until the entire
 *    DOE Response is read.
 *
 */
static void send_doe_command(uint64_t doe_base,
                             struct pcie_discovery_rw_api rw_api)
{
    uint32_t val;
    uint64_t reg;
    uint32_t cur_index = 0;
    struct cxl_cdat_read_entry_req cxl_doe_req;

    /* CDAT DOE Request header & Read entry request object */
    cxl_doe_req.header.vendor_id = DVSEC_CXL_VENDOR_ID;
    cxl_doe_req.header.data_obj_type = DOE_DATA_OBJ_TYPE_CDAT;
    cxl_doe_req.header.length = CDAT_READ_ENTRY_REQ_SIZE;

    /* 0 indicates this a request to read */
    cxl_doe_req.req_code = 0;

    /* 0 indicates table type as CDAT */
    cxl_doe_req.table_type = 0;

    /* 0 represents very first entry in the table */
    cxl_doe_req.entry_handle = 0;

    reg = doe_base + DOE_WRITE_DATA_MAILBOX_REG;

    do {
        if (is_doe_busy(doe_base + DOE_STATUS_REG, rw_api)) {
            FWK_LOG_WARN(MOD_NAME " DOE busy \n");
            return;
        }

        while (cur_index < CDAT_READ_ENTRY_REQ_SIZE) {
            rw_api.write32(
                reg,
                *((uint32_t *)(&cxl_doe_req) + cur_index));
            cur_index++;
        }

        reg = doe_base + DOE_CONTROL_REG;
        val = rw_api.read32(reg);
        val |= DOE_CTRL_DOE_GO;
        rw_api.write32(reg, val);

        cxl_doe_req.entry_handle = doe_receive_response(doe_base, rw_api);

    } while ( cxl_doe_req.entry_handle < CXL_DOE_TABLE_ENTRY_HANDLE_LAST);

    return;
}

/*
 * Find out whether the PCIe device supports CXL extended capability.
 */
static uint16_t find_pcie_ext_cap(uint64_t config_base,
                                  uint16_t ext_cap_off, uint16_t cap_id,
                                  struct pcie_discovery_rw_api rw_api)
{
    uint64_t cap_offset;
    uint32_t cap_val;

    cap_offset = config_base + ext_cap_off;

    do {
        cap_val =  rw_api.read32(cap_offset);
        if ((PCI_EXT_CAP_ID(cap_val) == cap_id)) {
            cap_val = rw_api.read32(cap_offset +
                DVSEC_CXL_HEADER1);
            if ((cap_val & 0xffff) == DVSEC_CXL_VENDOR_ID)
                return ext_cap_off;
        }

        ext_cap_off = PCI_EXT_CAP_NEXT(cap_val);
        cap_offset = config_base + ext_cap_off;
    } while (ext_cap_off);

   return 0;
}

/*
 * Find out whether the PCIe device supports DOE capability.
 */
static uint16_t find_pcie_doe_cap(uint64_t config_base,
                                  uint16_t ext_cap_off, uint16_t cap_id,
                                  struct pcie_discovery_rw_api rw_api)
{
    uint64_t cap_offset;
    uint32_t cap_val;

    cap_offset = config_base + ext_cap_off;

    do {
        cap_val =  rw_api.read32(cap_offset);
        if (PCI_EXT_CAP_ID(cap_val) == cap_id)
            return ext_cap_off;

        ext_cap_off = PCI_EXT_CAP_NEXT(cap_val);
        cap_offset = config_base + ext_cap_off;
    } while (ext_cap_off);

   return 0;
}

/*
 * This function gets the details about the Host physical address space, which
 * is reserved and configured for  CXL Memory.
 */
void update_cxl_mem_region(uint64_t mem_base, uint64_t size)
{
    host_cxl_mem.cxl_mem_base = mem_base;
    host_cxl_mem.free_cxl_mem_base = mem_base;
    host_cxl_mem.cxl_mem_size = size;

}

void map_cxl_mem_region(void)
{
   int status;

   apremap_cmn_atrans_api->enable();
   status = cmn_api->map_ccg_for_cxl_mem (remote_mem_size);
   if (status != FWK_SUCCESS) {
       FWK_LOG_ERR(MOD_NAME "CXL Mem region mapping failed, Status %d", status);
       return;
   }
   apremap_cmn_atrans_api->disable();
}

void find_cxl_capability(uint64_t config_base)
{
    uint32_t offset = 0;
    uint64_t cap_offset;
    uint32_t vendor_id;
    uint32_t cxl_id;
    uint64_t cap_val;
    struct pcie_discovery_rw_api rw_api;

    rw_api.read8 = remapper_rw_api->read8;
    rw_api.write8 = remapper_rw_api->write8;
    rw_api.read32 = remapper_rw_api->read32;
    rw_api.write32 = remapper_rw_api->write32;

    offset = find_pcie_ext_cap(config_base,
                               PCI_CFG_SPACE_SIZE,
                               PCIE_DVSEC_ID,
                               rw_api);
    if (!offset) {
        return;
    }

    cap_offset = config_base + offset + DVSEC_CXL_HEADER1;
    cap_val =  rw_api.read32(cap_offset);
    vendor_id = cap_val & DVSEC_VENDOR_ID_MASK;

    cap_offset = config_base + offset + DVSEC_CXL_HEADER2;
    cap_val =  rw_api.read32(cap_offset);
    cxl_id = cap_val & DVSEC_CXL_DEVICE_ID_MASK;

    if ((vendor_id != DVSEC_CXL_VENDOR_ID) || (cxl_id != DVSEC_CXL_DEVICE_ID)) {
        FWK_LOG_WARN(MOD_NAME "No CXL Device found");
        return;
    }

    offset = find_pcie_doe_cap(config_base,
                               PCI_CFG_SPACE_SIZE,
                               PCIE_DOE_ID,
                               rw_api);
    if (!offset) {
        FWK_LOG_WARN(MOD_NAME "DOE Capability Not Found \n");
        return;
    }

    send_doe_command(config_base + offset, rw_api);

    return;
}

static int cxl_bind(fwk_id_t id, unsigned int round)
{
    int status;

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_APREMAP),
        FWK_ID_API(FWK_MODULE_IDX_APREMAP, MOD_APREMAP_API_IDX_AP_MEM_RW),
        &remapper_rw_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = fwk_module_bind(
                 FWK_ID_MODULE(FWK_MODULE_IDX_CMN700),
                 FWK_ID_API(FWK_MODULE_IDX_CMN700, 0),
                 &cmn_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = fwk_module_bind(
                 FWK_ID_MODULE(FWK_MODULE_IDX_APREMAP),
                 FWK_ID_API(FWK_MODULE_IDX_APREMAP,
                 MOD_APREMAP_API_IDX_CMN_ATRANS),
                 &apremap_cmn_atrans_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return FWK_SUCCESS;
}

static int cxl_init(fwk_id_t module_id,
                    unsigned int element_count,
                    const void *unused)
{
    return FWK_SUCCESS;
}

static struct mod_cxl_api cxl_api = {
    .find_cxl_cap = find_cxl_capability,
    .map_cxl_mem_region = map_cxl_mem_region,
};

static int cxl_bind_request(fwk_id_t requester_id,
                            fwk_id_t id,
                            fwk_id_t api_id,
                            const void **api)
{

    if (!fwk_module_is_valid_module_id(requester_id)) {
        return FWK_E_ACCESS;
    }

    *api = &cxl_api;
    return FWK_SUCCESS;
}

const struct fwk_module_config config_cxl = { 0 };

const struct fwk_module module_cxl = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = 1,
    .event_count = 0,
    .init = cxl_init,
    .process_bind_request = cxl_bind_request,
    .bind = cxl_bind,
};
