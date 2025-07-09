/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_mm.h>
#include <mod_ras_handler.h>
#include <fwk_mmio.h>

#define MOD_NAME "[RAS_HANDLER]"
#define DYNAMIC_ATU_BASE 0x9F000000
#define TCM_ECC_BASE 0x50050000

static void default_handler()
{
unsigned int interrupt;
    fwk_interrupt_get_current(&interrupt);

    FWK_LOG_INFO ("[SRAM_INT] fwk_int number = %d\n",
	interrupt);
}

static void sram_handler()
{
    unsigned int interrupt;
    fwk_interrupt_get_current(&interrupt);
    uint32_t base = DYNAMIC_ATU_BASE;

    FWK_LOG_INFO ("[SRAM_INT] ErrStatus = 0x%lx\n",
	fwk_mmio_read_32(base + 0x010));

    /* Clear Status register */
    fwk_mmio_write_32((base + 0x010), 0xFFFFFFFF);

    FWK_LOG_INFO ("[SRAM_INT] fwk_int number = %d\n",
	interrupt);
    fwk_interrupt_clear_pending(interrupt);
    FWK_LOG_INFO ("[SRAM_INT] ErrAddr   = 0x%lx\n",
	fwk_mmio_read_32(base + 0x018));
}

static void tcm_handler()
{
    unsigned int interrupt;
    fwk_interrupt_get_current(&interrupt);
    uint32_t base = TCM_ECC_BASE;

    FWK_LOG_INFO ("[TCM_INT] ErrStatus = 0x%lx\n",
	fwk_mmio_read_32(base + 0x080));

    /* Clear Status register */
    fwk_mmio_write_32((base + 0x080), 0xFFFFFFFF);
 
    FWK_LOG_INFO ("[TCM_INT] fwk_int number = %d\n",
        interrupt);
    fwk_interrupt_clear_pending(interrupt);
    FWK_LOG_INFO ("[TCM_INT] ErrCode   = 0x%lx\n",
	fwk_mmio_read_32(base + 0x088));
    FWK_LOG_INFO ("[TCM_INT] ErrAddr   = 0x%lx\n",
	fwk_mmio_read_32(base + 0x08C));
}

static void rsm_handler()
{
    unsigned int interrupt;
    fwk_interrupt_get_current(&interrupt);
    uint32_t base = DYNAMIC_ATU_BASE;

    FWK_LOG_INFO ("[RSM_INT] ErrStatus = 0x%lx\n",
	 fwk_mmio_read_32(base + 0x010));

    /* Clear Status register */
    fwk_mmio_write_32((base + 0x010), 0xFFFFFFFF);

    FWK_LOG_INFO ("[RSM_INT] fwk_int number = %d\n",
        interrupt);
    fwk_interrupt_clear_pending(interrupt);
    FWK_LOG_INFO ("[RSM_INT] ErrAddr   = 0x%lx\n",
	fwk_mmio_read_32(base + 0x018));
}

struct ras_isr isr_list[] = {
	/* Shared SRAM Interrupts */
	[0]={ .interrupt_no = 24,
	      .isr = &sram_handler,
	    },
	[1]={ .interrupt_no = 25,
	      .isr = &sram_handler,
	    },
        [2]={ .interrupt_no = 26,
	      .isr = &sram_handler,
	    },
        [3]={ .interrupt_no = 27,
	      .isr = &sram_handler,
	    },
	/* SCP TCM Interrupts */
        [4]={ .interrupt_no = 21,
              .isr = &tcm_handler,
            },
        [5]={ .interrupt_no = 22,
              .isr = &tcm_handler,
            },
        [6]={ .interrupt_no = 23,
              .isr = &tcm_handler,
            },
	/* RSM SRAM Interrupt */
	[7]={ .interrupt_no = 29,
              .isr = &rsm_handler,
            },
};

static struct ras_context *ras_ctx;

static int mod_ras_handler_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *unused)
{

    FWK_LOG_INFO("%s Initializing RAS Handler ", MOD_NAME);
    if(element_count == 0)
        return FWK_E_DATA;

    ras_ctx = fwk_mm_calloc(element_count, sizeof(ras_ctx[0]));
 
    return FWK_SUCCESS;
}

bool check_if_irq_implemented(unsigned int irq)
{
    for(unsigned int i = 0; i < FWK_ARRAY_SIZE(isr_list); i++) {
    if (isr_list[i].interrupt_no == irq)
        return true;
    }

    return false;
}

void *find_isr(unsigned int irq )
{
    for(unsigned int i = 0; i < FWK_ARRAY_SIZE(isr_list); i++) {
    if(isr_list[i].interrupt_no == irq) {
	return (void*)isr_list[i].isr;
        }
    }

    return (void*)default_handler;
}

static int mod_ras_handler_start(fwk_id_t id)
{
    unsigned int irq;
    int status;
    /*
     * Go through the list  of interrupt and find matching
     * interrupt handler and initialize and enable interrupt
     * handler
     */
    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return FWK_SUCCESS;
    }

    FWK_LOG_INFO("%s Initializing RAS irq's. ", MOD_NAME);
    for(unsigned int i = 0; i < ras_ctx->config->interrupt_count; i++) {
	irq = ras_ctx->config->interrupt_id[i];
        if(check_if_irq_implemented(irq)) {
            status = fwk_interrupt_set_isr(irq,find_isr(irq));
        if (status != FWK_SUCCESS) {
            return status;
        }
           status = fwk_interrupt_enable(irq);
        if (status != FWK_SUCCESS) {
            return status;
	}
        } else {
          FWK_LOG_INFO(
	      "%s ISR not implemented for %d\n",
	      MOD_NAME,
	      irq);

	}
    }
    return FWK_SUCCESS;
}

static int ras_handler_element_init(
    fwk_id_t element_id,
    unsigned int element_count,
    const void *data)
{
    /* RAS handler context */
    if (fwk_id_get_element_idx(element_id) == 0) {
        ras_ctx[0].config = (struct mod_ras_config *)data;
    }

    return FWK_SUCCESS;
}


const struct fwk_module module_ras_handler = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .init = mod_ras_handler_init,
    .element_init = ras_handler_element_init,
    .start = mod_ras_handler_start,
};

