/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     PCIe Integration Control module.
 */

#include "pcie_integ_ctrl_reg.h"

#include <mod_apremap.h>
#include <mod_clock.h>
#include <mod_pcie_integ_ctrl.h>

#include <interface_io_block.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#define MOD_NAME "[PCIE_INTEG_CTRL] "

static const struct mod_apremap_cmn_atrans_api *apremap_cmn_atrans_api;

/* Module context */
struct mod_pcie_integ_ctrl_ctx {
    /* Pointer to the element configuration pointers (table) */
    const struct mod_pcie_integ_ctrl_config *config;
};

static struct mod_pcie_integ_ctrl_ctx *ctx_table;

/*
 * Static helpers
 */
static struct mod_pcie_integ_ctrl_ctx *get_ctx(fwk_id_t id)
{
    fwk_assert(fwk_module_is_valid_element_id(id));

    return (&ctx_table[fwk_id_get_element_idx(id)]);
}

static inline void pcie_integ_ctrl_configure_registers(
    struct pcie_ctrl_reg_set *reg,
    const struct pcie_ecam_mmio_mmap *cfg)
{
    reg->ECAM1_START_ADDR = PCIE_INTEG_CTRL_REG_START_ADDR_EN(
        cfg->ecam1_start_addr, cfg->allow_ns_access);
    reg->ECAM1_END_ADDR = PCIE_INTEG_CTRL_REG_END_ADDR(cfg->ecam1_end_addr);

    reg->MMIOL_START_ADDR = PCIE_INTEG_CTRL_REG_START_ADDR_EN(
        cfg->mmiol_start_addr, cfg->allow_ns_access);
    reg->MMIOL_END_ADDR = PCIE_INTEG_CTRL_REG_END_ADDR(cfg->mmiol_end_addr);

    reg->MMIOH_START_ADDR = PCIE_INTEG_CTRL_REG_START_ADDR_EN(
        cfg->mmioh_start_addr, cfg->allow_ns_access);
    reg->MMIOH_END_ADDR = PCIE_INTEG_CTRL_REG_END_ADDR(cfg->mmioh_end_addr);
}

static void configure_pcie_ecam_mmio_space(
    unsigned int index,
    const struct mod_pcie_integ_ctrl_config *config)
{
    const struct pcie_ecam_mmio_mmap *cfg;
    struct pcie_ctrl_reg *pcie_integ_ctrl_reg;
    struct pcie_ctrl_reg_set *reg;

    /* Disable CMN address translation to access the NCI GPV memory space */
    apremap_cmn_atrans_api->disable();

    pcie_integ_ctrl_reg = (struct pcie_ctrl_reg *)(config->reg_base);

    FWK_LOG_INFO(MOD_NAME "Configuring PCIe integ ctrl reg set: %d", index);

    reg = &pcie_integ_ctrl_reg->pcie_ctrl_x4_0;
    cfg = &config->x4_0_ecam_mmio_mmap;

    if (cfg->valid) {
        /* Configure address routing on x4_0 */
        pcie_integ_ctrl_configure_registers(reg, cfg);
    }

    reg = &pcie_integ_ctrl_reg->pcie_ctrl_x4_1;
    cfg = &config->x4_1_ecam_mmio_mmap;

    if (cfg->valid) {
        /* Configure address routing on x4_1 */
        pcie_integ_ctrl_configure_registers(reg, cfg);
    }

    reg = &pcie_integ_ctrl_reg->pcie_ctrl_x8;
    cfg = &config->x8_ecam_mmio_mmap;

    if (cfg->valid) {
        /* Configure address routing on x8 */
        pcie_integ_ctrl_configure_registers(reg, cfg);
    }

    reg = &pcie_integ_ctrl_reg->pcie_ctrl_x16;
    cfg = &config->x16_ecam_mmio_mmap;

    if (cfg->valid) {
        /* Configure address routing on x16 */
        pcie_integ_ctrl_configure_registers(reg, cfg);
    }
}

/*
 * Framework handlers
 */

static int pcie_integ_ctrl_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *unused)
{
    if (element_count == 0) {
        /* There must be at least one pcie integ config data */
        return FWK_SUCCESS;
    }

    ctx_table = fwk_mm_calloc(element_count, sizeof(ctx_table[0]));

    return FWK_SUCCESS;
}

static int pcie_integ_ctrl_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    const struct mod_pcie_integ_ctrl_config *config;
    struct mod_pcie_integ_ctrl_ctx *ctx;

    config = (struct mod_pcie_integ_ctrl_config *)data;
    if ((config == NULL) || (config->reg_base == 0)) {
        fwk_unexpected();
        return FWK_E_DATA;
    }

    ctx = &ctx_table[fwk_id_get_element_idx(element_id)];
    ctx->config = config;

    return FWK_SUCCESS;
}

static int pcie_integ_ctrl_start(fwk_id_t id)
{
    const struct mod_pcie_integ_ctrl_config *config;
    struct mod_pcie_integ_ctrl_ctx *ctx;

    /* Nothing to be done for module start call */
    if (fwk_id_get_type(id) == FWK_ID_TYPE_MODULE) {
        return FWK_SUCCESS;
    }

    ctx = get_ctx(id);

    config = ctx->config;

    if (fwk_id_is_equal(config->clock_id, FWK_ID_NONE)) {
        /* Configure the PCIe integration config registers */
        configure_pcie_ecam_mmio_space(fwk_id_get_element_idx(id), config);
        return FWK_SUCCESS;
    }

    /* Register the element for clock state notifications */
    return fwk_notification_subscribe(
        mod_clock_notification_id_state_changed, config->clock_id, id);
}

static int pcie_integ_ctrl_bind(fwk_id_t id, unsigned int round)
{
    return fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_APREMAP),
        FWK_ID_API(FWK_MODULE_IDX_APREMAP, MOD_APREMAP_API_IDX_CMN_ATRANS),
        &apremap_cmn_atrans_api);
}

static int pcie_integ_ctrl_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct mod_pcie_integ_ctrl_ctx *ctx;
    struct clock_notification_params *params;

    fwk_assert(
        fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed));
    fwk_assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT));

    params = (struct clock_notification_params *)event->params;

    if (params->new_state == MOD_CLOCK_STATE_RUNNING) {
        ctx = get_ctx(event->target_id);

        /* Configure the PCIe integration config registers */
        configure_pcie_ecam_mmio_space(
            fwk_id_get_element_idx(event->target_id), ctx->config);

        /* Unsubscribe to the notification */
        return fwk_notification_unsubscribe(
            event->id, event->source_id, event->target_id);
    }

    return FWK_SUCCESS;
}

static struct pcie_ctrl_reg_set *get_register_set(
    uintptr_t block_address,
    enum mod_pcie_integ_ctrl_target_node_id node_id)
{
    struct pcie_ctrl_reg *pcie_integ_ctrl_reg;
    struct pcie_ctrl_reg_set *reg;

    pcie_integ_ctrl_reg = (struct pcie_ctrl_reg *)(uintptr_t)(block_address);
    switch (node_id) {
    case MOD_PCIE_INTEG_CTRL_NODE_X4_0:
        reg = &pcie_integ_ctrl_reg->pcie_ctrl_x4_0;
        break;
    case MOD_PCIE_INTEG_CTRL_NODE_X4_1:
        reg = &pcie_integ_ctrl_reg->pcie_ctrl_x4_1;
        break;
    case MOD_PCIE_INTEG_CTRL_NODE_X8:
        reg = &pcie_integ_ctrl_reg->pcie_ctrl_x8;
        break;
    case MOD_PCIE_INTEG_CTRL_NODE_X16:
        reg = &pcie_integ_ctrl_reg->pcie_ctrl_x16;
        break;
    default:
        FWK_LOG_ERR("Invalid Target Node ID: %d", node_id);
        fwk_unexpected();
        reg = NULL;
        break;
    };

    return reg;
}

static void program_node(
    struct pcie_ctrl_reg_set *reg,
    enum interface_io_block_carveout_type carveout_type,
    uint64_t start,
    uint64_t end,
    bool allow_ns_access)
{
    switch (carveout_type) {
    case INTERFACE_IO_BLOCK_CARVEOUT_TYPE_ECAM:
        reg->ECAM1_START_ADDR =
            PCIE_INTEG_CTRL_REG_START_ADDR_EN(start, allow_ns_access);
        reg->ECAM1_END_ADDR = PCIE_INTEG_CTRL_REG_END_ADDR(end);
        break;
    case INTERFACE_IO_BLOCK_CARVEOUT_TYPE_MMIOL:
        reg->MMIOL_START_ADDR =
            PCIE_INTEG_CTRL_REG_START_ADDR_EN(start, allow_ns_access);
        reg->MMIOL_END_ADDR = PCIE_INTEG_CTRL_REG_END_ADDR(end);
        break;
    case INTERFACE_IO_BLOCK_CARVEOUT_TYPE_MMIOH:
        reg->MMIOH_START_ADDR =
            PCIE_INTEG_CTRL_REG_START_ADDR_EN(start, allow_ns_access);
        reg->MMIOH_END_ADDR = PCIE_INTEG_CTRL_REG_END_ADDR(end);
        break;
    default:
        FWK_LOG_ERR("Invalid Carveout type: %d", carveout_type);
        fwk_unexpected();
        break;
    };
}

static int map_region(struct interface_io_block_setup_mmap *mmap)
{
    struct pcie_ctrl_reg_set *reg;
    unsigned int idx;
    uint64_t start;
    uint64_t end;
    bool allow_ns_access;

    if (mmap == NULL) {
        return FWK_E_PARAM;
    }

    apremap_cmn_atrans_api->disable();

    /* The register base for PCIe Integ ctrl is within 32 bit address range. */
    for (idx = 0; idx < mmap->region_count; idx++) {
        reg = get_register_set(
            mmap->io_block_address, mmap->carveout_info[idx].target_id);

        start = mmap->carveout_info[idx].base;
        end =
            (mmap->carveout_info[idx].base + mmap->carveout_info[idx].size) - 1;
        allow_ns_access = mmap->carveout_info[idx].access_type ==
            INTERFACE_IO_BLOCK_ACCESS_TYPE_NS;
        program_node(
            reg,
            mmap->carveout_info[idx].carveout_type,
            start,
            end,
            allow_ns_access);
    }

    apremap_cmn_atrans_api->enable();

    return FWK_SUCCESS;
}

/*
 * There are no regions in PCIe Integ Ctrl. Mapping always map the same region.
 */
int unmap_region(struct interface_io_block_setup_mmap *mmap)
{
    return FWK_SUCCESS;
}

static struct interface_io_block_memmap_api memmap_api = {
    .map_region = map_region,
    .unmap_region = unmap_region,
};

static int pcie_integ_ctrl_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t id,
    fwk_id_t api_id,
    const void **api)
{
    enum mod_pcie_integ_ctrl_api_idx api_idx;

    api_idx = (enum mod_pcie_integ_ctrl_api_idx)fwk_id_get_api_idx(api_id);
    if (api_idx != MOD_PCIE_INTEG_CTRL_API_IDX_MAP_REGION) {
        return FWK_E_PARAM;
    }

    *api = &memmap_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_pcie_integ_ctrl = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = pcie_integ_ctrl_init,
    .api_count = MOD_PCIE_INTEG_CTRL_API_IDX_MAX,
    .element_init = pcie_integ_ctrl_element_init,
    .bind = pcie_integ_ctrl_bind,
    .start = pcie_integ_ctrl_start,
    .process_bind_request = pcie_integ_ctrl_process_bind_request,
    .process_notification = pcie_integ_ctrl_process_notification,
};
