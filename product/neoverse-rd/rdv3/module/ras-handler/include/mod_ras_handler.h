/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Module for configuring PCIe rootports and its memory map.
 */

#ifndef MOD_RAS_HANDLER_H
#define MOD_RAS_HANDLER_H

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <fwk_interrupt.h>

struct ras_isr {

    /*! Interrupt number*/
    unsigned int interrupt_no;

    /*! Interrupt service routine */
    void (*isr)(void);
};

struct mod_ras_config {

    /*! Number of RAS interrupts supported */
    unsigned int interrupt_count;

    /*! Array of interrupt id */
    const unsigned int *interrupt_id;

};

struct ras_context {
    struct mod_ras_config *config;
};

#endif /* MOD_RAS_HANDLER_H */
