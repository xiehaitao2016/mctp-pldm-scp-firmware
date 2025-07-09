/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/mod_pldm_fw_int.h>

#include <fwk_log.h>

#define MOD_NAME "[BMC]: "

/*
 * Object to represent BMC's view of MCP. If there are multiple nodes, BMC
 * should hold an array of contexts corresponding to each node. For current MCP
 * usecase, There are just 2 nodes (BMC and MCP). For sake of simplicity, we
 * have designated BMC as the master node that carries out all activities
 * related to discovery.
 */
pldm_fw_terminus_ctx_t pldm_fw_bmc_mcp_ctx;

/* routine to initialize bmc's ctx object. */
void bmc_init_ctx(pldm_fw_terminus_ctx_t *ctx)
{
    receiver_ev_state_t *ev_state;

    ev_state = &ctx->receiver_ev_state;

    /*
     * Event receivers for SBMR RAS capture depends only on polling. Set the
     * flag to capture polling events. This flag should be changed to ASYNC
     * or any other value as and when the use-cases change
     */
    ev_state->global_enable = PLDM_EVENT_MESSAGE_GLOBAL_ENABLE_POLLING;

    /* Use MCTP as the transport binding */
    ev_state->transport_protocol_type = PLDM_TRANSPORT_PROTOCOL_TYPE_MCTP;

    /*
     * From the specification. This field can change based on the transport
     * protocol type. For MCTP transport, this should be the same as the EID
     * of the event receiver.
     */
    ev_state->receiver_addr = 0x08;

    /*
     * Heart beat timer's elapsed time would be omitted in the case we set
     * event to polling only.
     */
    ev_state->heartbeat_timer = 0;

    ctx->pdr_info.pdr_owner = 0;
    ctx->pdr_info.repo = 0x0;
}

/*
 * pldm_fw would call into bmc_init to pass control to bmc routines to have bmc
 * initialiazed.
 */
void bmc_init()
{
    FWK_LOG_INFO(MOD_NAME "pldm discovery start ...");
    FWK_LOG_INFO(MOD_NAME "\n");

    bmc_init_ctx(&pldm_fw_bmc_mcp_ctx);
    /*
     * initializing agent as mentioned in the pldm spec is not explicitly
     * implemented. pldm_fw module takes care of implementing common
     * sequences as state machines with configurable terminal objects as
     * parameters/inputs. This way any node branching off pldm_fw can call
     * into discovery as and when needed.
     */
    pldm_fw_discovery(&pldm_fw_bmc_mcp_ctx);
}

void bmc_set_next_state()
{
}
