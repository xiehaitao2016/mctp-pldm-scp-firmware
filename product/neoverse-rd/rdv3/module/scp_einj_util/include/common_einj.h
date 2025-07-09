/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *
 */

#ifndef COMMON_EINJ_H
#define COMMON_EINJ_H

#include <cli.h>

#include <mod_atu.h>

#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <string.h>

/* Helper macros to extract components and subcomponet info */
#define CONFIG_REG(x) &(x->einj_register[params->subcomponent])
#define MEM_REG(x)    &(x->einj_mem_addr[params->subcomponent])

#define GET_REMOTE_CONFIG_ADDR(y) \
    remote_address_manager(ADD_REGION, CONFIG_REG(y))

#define GET_REMOTE_MEM_ADDR(y) remote_address_manager(ADD_REGION, MEM_REG(y))

#define READ_MM(address, type)   *((volatile type *)(address))
#define WRITE_MM(address, value) (*((volatile uint32_t *)(address)) = value)

#define SET_BIT(x, pos)   (x |= (1U << pos))
#define CLEAR_BIT(x, pos) (x &= (~(1U << pos)))

/* Max Logical address of SCP Memory Map */
#define SCP_LOGICAL_MAX (0xE00FFFFF)

enum PAS_ATTRIBUTES_EINJ {
    ROOT,
    SECURE,
    REALM,
    NON_SECURE,
};

enum ATU_MANAGER_ACTION {
    ADD_REGION,
    REMOVE_ALL,
};

/* SCP SHARED SRAM ERROR INJECTION REGISTERS */
enum SCP_SHARED_SRAM_LIST {
    SCP_SHARED_ROOT,
    SCP_SHARED_SECURE,
    SCP_SHARED_SRAM_COUNT,
};

/* SCP CPU ERROR INJECTION REGISTERS */
enum SCP_CPU_LIST {
    SCP_CPU_ERR,
    SCP_CPU_COUNT,
};

/* SCP TCM ERROR INJECTION REGISTERS */
enum SCP_TCM_LIST {
    SCP_ITCM,
    SCP_DTCM,
    SCP_TCM_COUNT,
};

/* SCP RSM ERROR INJECTION REGISTERS */
enum SCP_RSM_LIST {
    SCP_RSM_SECURE,
    SCP_RSM_NON_SECURE,
    SCP_RSM_COUNT,
};

enum ERROR_TYPE {
    EINJ_CE,
    EINJ_UE,
    EINJ_DE,
};

struct einj_memory {
    uint32_t pas_attribute;
    bool remote_addr;
    uint32_t region_size;
    uint64_t phy_addr;
    uint32_t log_addr;
};

struct einj_config {
    uint8_t no_components;
    bool remote_trigger;
    struct einj_memory *einj_mem_addr;
    struct einj_memory *einj_register;
};

struct einj_cli_params {
    uint8_t component;
    uint8_t subcomponent;
    uint8_t error_type;
};

struct remote_manager_t {
    uint64_t logical_address_start;
    uint64_t size;
    uint32_t current_pos;
    uint32_t pos;
    int count;
    uint8_t region_idx[4]; /* Concurrent allocation limit */
};

int remote_address_manager(
    enum ATU_MANAGER_ACTION action,
    struct einj_memory *reg);
typedef int (*injection_function)(
    struct einj_cli_params *params,
    struct einj_config *einj_config_list);

/*!
 * @}
 */

/*!
 * @}
 */

#endif
