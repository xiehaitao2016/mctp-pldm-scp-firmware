/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Physical and remote address of various sub-components
 */

#ifndef COMPONENT_EINJ_H
#define COMPONENT_EINJ_H

#include <common_einj.h>

/* SCP-Shared SRAM */
struct einj_memory scp_shared_einj_config[] = {
	[SCP_SHARED_ROOT] = {
	 .pas_attribute = ATU_ENCODE_ATTRIBUTES_ROOT_PAS,
	 .remote_addr = true,
	 .region_size = (64 * FWK_KIB),
         .phy_addr = (0x2A510000),
	  },
	[SCP_SHARED_SECURE] = {
	 .pas_attribute = ATU_ENCODE_ATTRIBUTES_SECURE_PAS,
	 .remote_addr = true,
	 .region_size = (64 * FWK_KIB),
         .phy_addr = (0x2A4F0000),
        },

};

struct einj_memory scp_shared_einj_mem[] = {
	[SCP_SHARED_ROOT] = {
	 .pas_attribute = ATU_ENCODE_ATTRIBUTES_ROOT_PAS,
	 .remote_addr = false,
	 .region_size = (1 * FWK_KIB),
         .phy_addr = (0x70000000),
	  },
	[SCP_SHARED_SECURE] = {
	 .pas_attribute = ATU_ENCODE_ATTRIBUTES_SECURE_PAS,
	 .remote_addr = false,
	 .region_size = (1 * FWK_KIB),
         .phy_addr = (0x80400000),
        },
};

/* CPU */
struct einj_memory scp_cpu_einj_config[] = {
	 [SCP_CPU_ERR] = {
	  .pas_attribute = ATU_ENCODE_ATTRIBUTES_SECURE_PAS,
	  .remote_addr = false,
	  .region_size = (256 * FWK_MIB),
	  .phy_addr = (0x60000000),
	 },
 };

struct einj_memory scp_cpu_einj_mem[] = {
    [SCP_CPU_ERR] = { 0 },
};

/* SCP TCM */
struct einj_memory scp_tcm_einj_config[] = {
        [SCP_ITCM] = {
         .pas_attribute = ATU_ENCODE_ATTRIBUTES_SECURE_PAS,
         .remote_addr = false,
         .region_size = (64 * FWK_KIB),
         .phy_addr = (0x50050000),
          },
        [SCP_DTCM] = {
         .pas_attribute = ATU_ENCODE_ATTRIBUTES_SECURE_PAS,
         .remote_addr = false,
         .region_size = (64 * FWK_KIB),
         .phy_addr = (0x50050000),
        },
};

struct einj_memory scp_tcm_einj_mem[] = {
        [SCP_ITCM] = {
         .pas_attribute = ATU_ENCODE_ATTRIBUTES_SECURE_PAS,
         .remote_addr = false,
         .region_size = (1 * FWK_KIB),
         .phy_addr = (0x00000000),
          },
        [SCP_DTCM] = {
         .pas_attribute = ATU_ENCODE_ATTRIBUTES_SECURE_PAS,
         .remote_addr = false,
         .region_size = (1 * FWK_KIB),
         .phy_addr = (0x20000000),
        },
};

/* SCP RSM */
struct einj_memory scp_rsm_einj_config[] = {
        [SCP_RSM_SECURE] = {
         .pas_attribute = ATU_ENCODE_ATTRIBUTES_SECURE_PAS,
         .remote_addr = true,
         .region_size = (64 * FWK_KIB),
         .phy_addr = (0x2A5D0000),
        },
        [SCP_RSM_NON_SECURE] = {
         .pas_attribute = ATU_ENCODE_ATTRIBUTES_SECURE_PAS,
         .remote_addr = true,
         .region_size = (64 * FWK_KIB),
         .phy_addr = (0x2A5E0000),
        },
};

struct einj_memory scp_rsm_einj_mem[] = {
        [SCP_RSM_SECURE] = {
         .pas_attribute = ATU_ENCODE_ATTRIBUTES_SECURE_PAS,
         .remote_addr = false,
         .region_size = (1 * FWK_KIB),
         .phy_addr = (0x78100000),
        },
        [SCP_RSM_NON_SECURE] = {
         .pas_attribute = ATU_ENCODE_ATTRIBUTES_NON_SECURE_PAS,
         .remote_addr = false,
         .region_size = (1 * FWK_KIB),
         .phy_addr = (0x80700000),
        },
};

/* Injection function for scp tcm einj */
int scp_tcm_einj_function(
    struct einj_cli_params *params,
    struct einj_config *einj_config_list);

/* Injection function for scp shared sram einj */
int scp_einj_function(
    struct einj_cli_params *params,
    struct einj_config *einj_config_list);

/* Injection function for scp cpu einj */
int scp_cpu_einj_function(
    struct einj_cli_params *params,
    struct einj_config *einj_config_list);

/* Injection function for scp rsm einj */
int scp_rsm_einj_function(
    struct einj_cli_params *params,
    struct einj_config *einj_config_list);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SCP_PLATFORM_H */
