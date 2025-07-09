/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCP Platform Support
 */

#ifndef MOD_SCP_EINJ_UTIL_H
#define MOD_SCP_EINJ_UTIL_H

#include <cli.h>
#include <component_einj.h>

#include <stdlib.h>

#define ATU_EINJ_LOGIC_START 0x9F000000
#define ATU_EINJ_LOGIC_SIZE  0x100000
#define ATU_EINJ_LOGIC_MAX   (ATU_EINJ_LOGIC_START + ATU_EINJ_LOGIC_SIZE)

/* Help string */
const char einj_help[] =
    "Inject error into various components.\n"
    "Usage: einj-util -comp <n> -subcomp <n> -type <n>\n"
    "-comp: sram (0), tcm (1), cpu (2), rsm (3)\n"
    "-subcomp:\n"
    "	sram: root (0), secure (1)\n"
    "	tcm: itcm (0), dtcm (1)\n"
    "	cpu: always 0 for now\n"
    "	rsm: secure (0), non-secure (1)\n"
    "-type:\n"
    "	sram/tcm/rsm: correctable (0), uncorrectable (1)\n"
    "	cpu: correctable (0), uncorrectable (1), deferred (2)\n\n"
    "example:\n"
    "	1) ce into shared sram from secure world:\n"
    "		einj-util -comp 0 -subcomp 1 -type 0\n"

    "	2) ce into scp itcm:\n"
    "		einj-util -comp 1 -subcomp 0 -type 0\n"

    "	3) cpu ue:\n"
    "		einj-util -comp 2 -subcomp 0 -type 1\n"

    "	4) ce into rsm sram from secure world:\n"
    "		einj-util -comp 3 -subcomp 0 -type 0\n";

/* Modify einj_commands_list to add new commands */
enum einj_list {
    SCP_SHARED_SRAM,
    SCP_TCM,
    SCP_CPU,
    SCP_RSM,
    SCP_CMN,
    EINJ_LIST_COUNT
};
/* Modify this list to add new injection registers */
struct einj_config einj_config_list [] = {
	[SCP_SHARED_SRAM] = {
                .no_components = SCP_SHARED_SRAM_COUNT,
                .remote_trigger= false,
                .einj_mem_addr = scp_shared_einj_mem,
                .einj_register = scp_shared_einj_config,
        },
        [SCP_TCM] = {
                .no_components = SCP_TCM_COUNT,
                .remote_trigger= false,
                .einj_mem_addr = scp_tcm_einj_mem,
                .einj_register = scp_tcm_einj_config,
                },
	[SCP_CPU] = {
		.no_components = SCP_CPU_COUNT,
		.remote_trigger= false,
		.einj_mem_addr = scp_cpu_einj_mem,
		.einj_register = scp_cpu_einj_config,
	},
	[SCP_RSM] = {
                .no_components = SCP_RSM_COUNT,
                .remote_trigger= false,
                .einj_mem_addr = scp_rsm_einj_mem,
                .einj_register = scp_rsm_einj_config,
        },
	[SCP_CMN] = {0},
};

/* Modify this list to add new injection fucntion */
injection_function injectionlist[] = {
    [SCP_SHARED_SRAM] = scp_einj_function,
    [SCP_TCM] = scp_tcm_einj_function,
    [SCP_CPU] = scp_cpu_einj_function,
    [SCP_RSM] = scp_rsm_einj_function,
};

int parse_cli_params(
    int argc,
    char **argv,
    struct einj_cli_params *parse_params);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SCP_PLATFORM_H */
