/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_scp_einj_util.h>

#define MOD_NAME "[SCP_EINJ_CLI] "
const char einj_call[] = "einj-util";

extern struct cli_ctx cli_ctx;
static const struct mod_atu_api *atu_api;
struct remote_manager_t remote_manager_ctx = { 0 };

/* Parse and populate input cli parameter */
int parse_cli_params(
    int argc,
    char **argv,
    struct einj_cli_params *parse_params)
{
    /*
     * TODO Add list all helper to list out all
     * the injection functions available
     */
    if (argc < 7) {
        cli_printf(TEXT_RED, "Missing parameter: Try einj-util help");
        return FWK_E_PARAM;
    }
    if (argc == 7) {
        if (cli_strncmp(argv[1], "-comp", 5) == 0) {
            parse_params->component = (uint8_t)strtoul(argv[2], 0, 0);
        }
        if (cli_strncmp(argv[3], "-subcomp", 8) == 0) {
            parse_params->subcomponent = (uint8_t)strtoul(argv[4], 0, 0);
        }
        if (cli_strncmp(argv[5], "-type", 5) == 0) {
            parse_params->error_type = (uint8_t)strtoul(argv[6], 0, 0);
        }
    }
    if (parse_params->component > FWK_ARRAY_SIZE(injectionlist)) {
        cli_printf(TEXT_RED, "Error Injection Fucntion Missing");
        return FWK_E_PARAM;
    }
    return FWK_SUCCESS;
}

/* Allocate logical address for remote addresses */
int remote_address_manager(
    enum ATU_MANAGER_ACTION action,
    struct einj_memory *reg)
{
    struct atu_region_map region;
    uint8_t region_idx;
    int status;
    /* Identifier of the ATU device */
    fwk_id_t atu_device_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_ATU, 0);

    /* Identifier of the entity that owns the ATU region */
    fwk_id_t atu_region_owner_id =
        FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCP_EINJ_UTIL);

    if (reg->remote_addr == true) {
        switch (action) {
        /*Add ATU Mapping */
        case ADD_REGION:
            if ((remote_manager_ctx.current_pos + reg->region_size) <
                ATU_EINJ_LOGIC_MAX) {
                region.region_owner_id = atu_region_owner_id;
                region.phy_addr_base = reg->phy_addr;
                region.region_size = reg->region_size;
                region.log_addr_base = remote_manager_ctx.current_pos;
                region.attributes = reg->pas_attribute;
                status =
                    atu_api->add_region(&region, atu_device_id, &region_idx);
                if (status != FWK_SUCCESS) {
                    return FWK_E_PANIC;
                }
                remote_manager_ctx.region_idx[remote_manager_ctx.count] =
                    region_idx;
                remote_manager_ctx.current_pos += reg->region_size;
                remote_manager_ctx.count += 1;

                return region.log_addr_base;
            } else {
                FWK_LOG_ERR("Logical address limit exceeded.");
                return FWK_E_PANIC;
            }
            break;
            /* Remove all atu mappings mapped by einj-util */
        case REMOVE_ALL:
            if (remote_manager_ctx.count > 0) {
                for (int i = remote_manager_ctx.count; i > 0; i--) {
                    status = atu_api->remove_region(
                        remote_manager_ctx
                            .region_idx[remote_manager_ctx.count - 1],
                        atu_device_id,
                        atu_region_owner_id);
                    if (status != FWK_SUCCESS) {
                        FWK_LOG_ERR("ATU Remove region Failed\n");
                        return FWK_E_PANIC;
                    }

                    remote_manager_ctx
                        .region_idx[remote_manager_ctx.count - 1] = 0;
                    remote_manager_ctx.count -= 1;
                }
                remote_manager_ctx.current_pos = ATU_EINJ_LOGIC_START;
            }
            break;

        default:
            FWK_LOG_ERR("Unknown Action: remote_address_manager");
            return FWK_E_PANIC;
        }
    } else {
        return (uint32_t)reg->phy_addr;
    }

    return FWK_SUCCESS;
}

int32_t einj_fun(int32_t argc, char **argv)
{
    int status;
    struct einj_cli_params cli_params = { 0 };

    /* parse the input arguments */
    status = parse_cli_params(argc, argv, &cli_params);
    if (status != FWK_SUCCESS)
        return FWK_E_PARAM;

    /* Match the injection fucntion from list */
    injectionlist[cli_params.component](
        &cli_params, &einj_config_list[cli_params.component]);

    /* Remove all allocated ATU regions before exiting */
    remote_address_manager(REMOVE_ALL, 0);

    return FWK_SUCCESS;
}

static int mod_scp_einj_util_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *unused)
{
    return FWK_SUCCESS;
}

static int mod_scp_einj_util_start(fwk_id_t id)
{
    int status;
    /* Registering einj-util with cli */
    cli_command_st einj_commands_list[] = {
        { einj_call, einj_help, &einj_fun, false },
        /* End of commands. */
        { 0, 0, 0 },
    };
    /* Register the command */
    for (int index = 0; einj_commands_list[index].command != 0; index++) {
        status = cli_command_register(einj_commands_list[index]);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR("Falied to Register einj-util command");
            return status;
        }
    }
    /* populate the remote_manager_ctx */
    remote_manager_ctx.logical_address_start = ATU_EINJ_LOGIC_START;
    remote_manager_ctx.size = ATU_EINJ_LOGIC_SIZE;
    remote_manager_ctx.current_pos = ATU_EINJ_LOGIC_START;
    remote_manager_ctx.count = 0;

    return FWK_SUCCESS;
}

static int mod_scp_einj_util_bind(fwk_id_t id, unsigned int round)
{
    int status;
    /* Bind to atu module atu-apis */
    status = fwk_module_bind(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_ATU, 0),
        FWK_ID_API(FWK_MODULE_IDX_ATU, MOD_ATU_API_IDX_ATU),
        &atu_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_scp_einj_util = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .init = mod_scp_einj_util_init,
    .start = mod_scp_einj_util_start,
    .bind = mod_scp_einj_util_bind,
};

const struct fwk_module_config config_scp_einj_util = { 0 };
