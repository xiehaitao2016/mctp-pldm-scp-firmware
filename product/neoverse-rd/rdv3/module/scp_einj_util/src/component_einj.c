/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Component einj : Component specific Error injection functions
 */

#include "fwk_mmio.h"

#include <common_einj.h>

/* Shared SRAM ECC RAS register offsets */
#define SRAM_ENABLE_ECC      (1U << 0)
#define SRAM_INJECT_CE_BIT   (1U << 8)
#define SRAM_INJECT_ERROR_CE (1U << 4)
#define SRAM_INJECT_UE_BIT   (1U << 3)
#define SRAM_INJECT_ERROR_UE (1U << 3)

#define SRAM_ERR_CTRL_OFFSET   0x08UL
#define SRAM_ERR_STATUS_OFFSET 0x10UL
#define SRAM_ERR_ADDR_OFFSET   0x18UL
#define SRAM_ERR_MISC1_OFFSET  0x28UL

/* TCM Error Injection */
#define TCM_TGT_RAM_IPATH (0x0 << 7)
#define TCM_TGT_RAM_DPATH (0x1 << 7)

#define TCM_INJECT_CE_ERROR  (0x1 << 5)
#define TCM_INJECT_UE_ERROR  (0x2 << 5)
#define TCM_UE_MASK          (1U << 3)
#define TCM_CE_MASK          (1U << 2)
#define TCM_ITCMRAM_ECC_EN   (1U << 1)
#define TCM_DTCMRAM_ECC_EN   (1U << 0)
#define TCM_ERRCTRL_OFFSET   0x084
#define TCM_ERRADDR_OFFSET   0x08C
#define TCM_ERRSTATUS_OFFSET 0x080

/* RAS base for CPU error injection */
#define RAS_BASE (0x1A0000)

/* SCP Shared-SRAM Error Inject function */
int scp_einj_function(
    struct einj_cli_params *params,
    struct einj_config *einj_config_list)
{
    uint32_t ecc_ras_reg_base;
    uint32_t action_address;
    enum ERROR_TYPE error_type = params->error_type;
    /* Fetch and convert remote address to local address */
    ecc_ras_reg_base = GET_REMOTE_CONFIG_ADDR(einj_config_list);
    action_address = GET_REMOTE_MEM_ADDR(einj_config_list);

    if (ecc_ras_reg_base > SCP_LOGICAL_MAX ||
        action_address > SCP_LOGICAL_MAX) {
        cli_printf(TEXT_GREEN, "Invalid address on Translation\n");
        return -1;
    }

    /*
     * Program control register to enable ECC checking
     * and error injection on shared SRAMs.
     */
    fwk_mmio_write_32(
        (ecc_ras_reg_base + SRAM_ERR_CTRL_OFFSET),
        SRAM_INJECT_CE_BIT | SRAM_INJECT_UE_BIT | SRAM_ENABLE_ECC);

    switch (error_type) {
    case EINJ_CE:
        cli_printf(TEXT_GREEN, "Injecting CE into Shared SRAM\n");
        fwk_mmio_write_32(
            (ecc_ras_reg_base + SRAM_ERR_MISC1_OFFSET), SRAM_INJECT_ERROR_CE);
        break;
    case EINJ_UE:
        cli_printf(TEXT_GREEN, "Injecting UE into Shared SRAM\n");
        fwk_mmio_write_32(
            (ecc_ras_reg_base + SRAM_ERR_MISC1_OFFSET), SRAM_INJECT_ERROR_UE);
        break;
    default:
        cli_printf(
            TEXT_RED,
            "Select the right error type. 0 for CE and 1 for UE \r\n");
    }

    /*
     * Now do a read transaction to a shared SRAM
     * address to trigger the error
     */
    uint32_t data_read = *(volatile uint32_t *)(action_address + 0x10);
    cli_printf(TEXT_GREEN, "Data : 0x%x \r\n", data_read);

    return FWK_SUCCESS;
}

/* RSM Error Inject function */
int scp_rsm_einj_function(
    struct einj_cli_params *params,
    struct einj_config *einj_config_list)
{
    uint32_t scp_rsm_ras_reg_base;
    uint32_t action_address;
    enum ERROR_TYPE error_type = params->error_type;
    /* Fetch and convert remote address to local address */
    scp_rsm_ras_reg_base = GET_REMOTE_CONFIG_ADDR(einj_config_list);
    action_address = GET_REMOTE_MEM_ADDR(einj_config_list);

    if (scp_rsm_ras_reg_base > SCP_LOGICAL_MAX ||
        action_address > SCP_LOGICAL_MAX) {
        cli_printf(TEXT_GREEN, "Invalid address on Translation\n");
        return -1;
    }
    /* Setup Interrupt for CE and UE and Enable ECC */
    fwk_mmio_write_32(
        (scp_rsm_ras_reg_base + SRAM_ERR_CTRL_OFFSET),
        SRAM_INJECT_CE_BIT | SRAM_INJECT_UE_BIT | SRAM_ENABLE_ECC);

    switch (error_type) {
    case EINJ_CE:
        cli_printf(TEXT_GREEN, "Injecting CE into RSM SRAM\n");
        fwk_mmio_write_32(
            (scp_rsm_ras_reg_base + SRAM_ERR_MISC1_OFFSET),
            SRAM_INJECT_ERROR_CE);
        break;
    case EINJ_UE:
        cli_printf(TEXT_GREEN, "Injecting UE into RSM SRAM\n");
        fwk_mmio_write_32(
            (scp_rsm_ras_reg_base + SRAM_ERR_MISC1_OFFSET),
            SRAM_INJECT_ERROR_UE);
        break;
    default:
        cli_printf(
            TEXT_RED,
            "Select the right error type. 0 for CE and 1 for UE \r\n");
    }

    /*
     * Now do a read transaction to a RSM SRAM
     * address to trigger the error
     */
    uint32_t data_read = *(volatile uint32_t *)(action_address + 0x10);
    cli_printf(TEXT_GREEN, "Data : 0x%x \r\n", data_read);

    return FWK_SUCCESS;
}

/* CPU Error Inject function */
void inject_error_cpu(uint32_t cluster0_base, uint8_t errtype)
{
    uint32_t cluster0_ras = cluster0_base + RAS_BASE;
    uint32_t core0_base = cluster0_ras;
    /* Enable CE and Error injection */
    fwk_mmio_write_32((cluster0_ras + 0x008), 0x10D);
    /* Clear ERXPDGCTRL Register */
    fwk_mmio_write_32((cluster0_ras + 0x808), 0x0);
    /* Clear ERXSTATUS Register */
    fwk_mmio_write_32((cluster0_ras + 0x010), 0xFFC00000);
    /* Set PFGCDN to 1 */
    fwk_mmio_write_32((cluster0_ras + 0x810), 0x1);

    switch (errtype) {
    case EINJ_CE:
        cli_printf(TEXT_GREEN, "Injecting CPU CE\r\n");
        /* Set PDGCTRL to Inject CE */
        fwk_mmio_write_32((cluster0_ras + 0x808), 0x80000940);
        break;
    case EINJ_UE:
        cli_printf(TEXT_GREEN, "Injecting CPU UE\r\n");
        /* Set PDGCTRL to Inject UE */
        fwk_mmio_write_32((cluster0_ras + 0x808), 0x80000002);
        break;
    case EINJ_DE:
        cli_printf(TEXT_GREEN, "Injecting CPU DE\r\n");
        /* Set PDGCTRL to Inject DE */
        fwk_mmio_write_32((cluster0_ras + 0x808), 0x80000020);
        break;
    default:
        cli_printf(
            TEXT_RED,
            "Select the right error type."
            "\n0 = CE, 1 = UE, 2 = DE\r\n");
    }
    /* Add some delay for the overflow to happen */
    for (int j = 0; j < 1000000; j++)
        ;

    cli_printf(
        TEXT_GREEN,
        "ErrStatus  0x%llx\r\n",
        READ_MM((core0_base + 0x010), uint64_t));
    cli_printf(
        TEXT_GREEN,
        "ErrAddress 0x%llx\r\n",
        READ_MM((cluster0_ras + 0x018), uint64_t));
}

int scp_cpu_einj_function(
    struct einj_cli_params *params,
    struct einj_config *einj_config_list)
{
    enum SCP_CPU_LIST cpu_list = params->subcomponent;
    uint8_t errtype = params->error_type;
    uint32_t cpu_einj_reg = GET_REMOTE_CONFIG_ADDR(einj_config_list);

    switch (cpu_list) {
    case SCP_CPU_ERR:
        inject_error_cpu(cpu_einj_reg, errtype);
        break;
    default:
        cli_printf(TEXT_RED, "Check scp-einj -h for help \r\n");
        break;
    }
    return 0;
}

/* SCP TCM Error Inject function */
void inject_tcm_error(
    uint32_t einj_reg,
    uint32_t action_addr,
    enum ERROR_TYPE type)
{
    uint32_t reg_read = 0;
    /* Select error type */
    switch (type) {
    case EINJ_CE:
        cli_printf(TEXT_GREEN, "Injecting CE\n");
        reg_read = fwk_mmio_read_32(einj_reg + TCM_ERRCTRL_OFFSET);
        CLEAR_BIT(reg_read, 6);
        SET_BIT(reg_read, 5);
        fwk_mmio_write_32(einj_reg + TCM_ERRCTRL_OFFSET, reg_read);
        break;
    case EINJ_UE:
        cli_printf(TEXT_GREEN, "Injecting UE\n");
        reg_read = fwk_mmio_read_32(einj_reg + TCM_ERRCTRL_OFFSET);
        CLEAR_BIT(reg_read, 5);
        SET_BIT(reg_read, 6);
        fwk_mmio_write_32(einj_reg + TCM_ERRCTRL_OFFSET, reg_read);
        break;
    default:
        cli_printf(
            TEXT_RED,
            "Select the right error. \n"
            "0 for CE and 1 for UE.\n");
        return;
    }
    /* Perform the Action */
    reg_read = *(volatile uint32_t *)action_addr;
    cli_printf(TEXT_GREEN, "Data : 0x%x \r\n", reg_read);
}

int scp_tcm_einj_function(
    struct einj_cli_params *params,
    struct einj_config *einj_config_list)
{
    enum SCP_TCM_LIST tcm_list = params->subcomponent;
    enum ERROR_TYPE type = params->error_type;
    uint32_t reg_read = 0;
    uint32_t tcm_einj_reg = GET_REMOTE_CONFIG_ADDR(einj_config_list);
    uint32_t action_addr = GET_REMOTE_MEM_ADDR(einj_config_list);
    if (tcm_einj_reg > SCP_LOGICAL_MAX || action_addr > SCP_LOGICAL_MAX) {
        cli_printf(TEXT_GREEN, "Invalid address on Translation\n");
        return -1;
    }

    /* Enable Error CE, UE error detection and Interrupts */
    fwk_mmio_write_32(
        tcm_einj_reg + TCM_ERRCTRL_OFFSET,
        (TCM_UE_MASK | TCM_CE_MASK | TCM_ITCMRAM_ECC_EN | TCM_DTCMRAM_ECC_EN |
         TCM_INJECT_CE_ERROR | TCM_TGT_RAM_IPATH));

    switch (tcm_list) {
    case SCP_ITCM:
        cli_printf(TEXT_GREEN, "ITCM\n");
        /* Select ITCM */
        reg_read = fwk_mmio_read_32(tcm_einj_reg + TCM_ERRCTRL_OFFSET);
        CLEAR_BIT(reg_read, 7);
        fwk_mmio_write_32(tcm_einj_reg + TCM_ERRCTRL_OFFSET, reg_read);
        inject_tcm_error(tcm_einj_reg, action_addr, type);
        break;
    case SCP_DTCM:
        cli_printf(TEXT_GREEN, "DTCM\n");
        /* Select DTCM */
        reg_read = fwk_mmio_read_32(tcm_einj_reg + TCM_ERRCTRL_OFFSET);
        SET_BIT(reg_read, 7);
        fwk_mmio_write_32(tcm_einj_reg + TCM_ERRCTRL_OFFSET, reg_read);
        inject_tcm_error(tcm_einj_reg, action_addr, type);
        break;
    default:
        cli_printf(
            TEXT_RED,
            "Select the right component\n"
            "try scp-einj -h");
    }
    return FWK_SUCCESS;
}
