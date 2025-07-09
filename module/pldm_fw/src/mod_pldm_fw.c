/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifdef BUILD_HAS_DEBUGGER
#include <cli.h>
#endif

#include <internal/mod_pldm_fw_int.h>

#include <mod_mctp_fw.h>
#include <mod_pldm.h>
#include <mod_pldm_fw.h>

#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_core.h>

#define MOD_NAME "[PLDM_FW]: "

/*
 * spaces from the left most end of the terminal to the right most end of the
 * terminal for right alignment
 */
#define ALIGN_SIZE 48

/*
 * wrapper macro over FWK_LOG_INFO. Use this macro to align and format spacing
 * __VA_ARGS__ by ALIGN_SIZE. This macro can keep all parameters right aligned
 */
#define FWK_LOG_INFO_ALIGN_P1(fmt, ...) \
    FWK_LOG_INFO(fmt, ALIGN_SIZE - strlen(fmt), __VA_ARGS__)

/*
 * wrapper macro over FWK_LOG_INFO. Use this macro to align and format spacing
 * __VA_ARGS__ by ALIGN_SIZE. This macro can additionally take one parameter to
 * the left and still keep the remaining parameters right aligned
 */
#define FWK_LOG_INFO_ALIGN_P2(fmt, p1, ...) \
    FWK_LOG_INFO(fmt, p1, ALIGN_SIZE - strlen(fmt), __VA_ARGS__)

/* state setter and getter macros */
#define pldm_fw_get_state() pldm_fw_state
#define pldm_fw_set_state(s) \
    do { \
        pldm_fw_state = s; \
    } while (0);

/*
 * simple hash with 1 byte right shifted pldm_type (at 1'st byte position)
 * followed by command in the 0'th byte position.
 */
#define PLDM_HASH(type, cmd) \
    (uint16_t)((cmd | (type << BITS_PER_BYTE)) & 0xFFFF)

/*
 * wrapper macro around assert to be used for checking if a pldm command was
 * successfully processed
 */
#define PLDM_ASSERT(x) assert(PLDM_SUCCESS == (x))

/* commonly used pldm struct sizes */
#define PLDM_MSG_HDR_T_SIZE (sizeof(pldm_msg_hdr_t))
#define PLDM_MSG_T_SIZE     (sizeof(pldm_msg_t))

/* Helper macros to simplify nested access */
#define PLDM_BASE_API  pldm_base_api
#define PLDM_BASE_ELEM pldm_base_elem

#define PLDM_PLATFORM_API  pldm_platform_api
#define PLDM_PLATFORM_ELEM pldm_platform_elem

#define PLDM_PDR_API  pldm_pdr_api
#define PLDM_PDR_ELEM pldm_pdr_elem

#define PLDM_FRU_API  pldm_fru_api
#define PLDM_FRU_ELEM pldm_fru_elem

#define PLDM_UTILS_API  pldm_utils_api
#define PLDM_UTILS_ELEM pldm_utils_elem

#define MCTP_FW_API  mctp_fw_api
#define MCTP_FW_ELEM mctp_fw_elem

#define PLDM_FW_ELEM pldm_fw_elem

#define GET_PLDM_PDR_API(ctx) ctx->PLDM_FW_ELEM.PLDM_PDR_ELEM.PLDM_PDR_API

#define GET_PLDM_PLATFORM_API(ctx) \
    ctx->PLDM_FW_ELEM.PLDM_PLATFORM_ELEM.PLDM_PLATFORM_API

#define GET_PLDM_BASE_API(ctx) ctx->PLDM_FW_ELEM.PLDM_BASE_ELEM.PLDM_BASE_API

#define GET_PLDM_FRU_API(ctx) ctx->PLDM_FW_ELEM.PLDM_FRU_ELEM.PLDM_FRU_API

#define GET_PLDM_UTILS_API(ctx) ctx->PLDM_FW_ELEM.PLDM_UTILS_ELEM.PLDM_UTILS_API

#define GET_MCTP_FW_API(ctx) ctx->PLDM_FW_ELEM.MCTP_FW_ELEM.MCTP_FW_API

/* utility macros */
#define BITS_PER_BYTE    (8U)
#define PLDM_INSTANCE_ID (0U)
#define PLDM_TYPES_COUNT (8U)

/* type for pldm_base element */
typedef struct pldm_base_elem {
    pldm_base_api_t *pldm_base_api;
} pldm_base_elem_t;

/* type for mctp_fw element */
typedef struct mctp_fw_elem {
    mctp_fw_api_t *mctp_fw_api;
} mctp_fw_elem_t;

/* type for pldm_platform element */
typedef struct pldm_platform_elem {
    pldm_platform_api_t *pldm_platform_api;
} pldm_platform_elem_t;

/* type for pldm_pdr element */
typedef struct pldm_pdr_elem {
    pldm_pdr_api_t *pldm_pdr_api;
} pldm_pdr_elem_t;

/* type for pldm_fru element */
typedef struct pldm_fru_elem {
    pldm_fru_api_t *pldm_fru_api;
} pldm_fru_elem_t;

/* type for pldm utils element */
typedef struct pldm_utils_elem {
    pldm_utils_api_t *pldm_utils_api;
} pldm_utils_elem_t;

/* Generic type for pldm_fw elements */
typedef struct pldm_fw_elem_ctx_t {
    fwk_id_t id;
    pldm_fw_elem_config_t *pldm_fw_config;
    fwk_id_t driver_id;

    union {
        pldm_base_elem_t pldm_base_elem;
        mctp_fw_elem_t mctp_fw_elem;
        pldm_platform_elem_t pldm_platform_elem;
        pldm_pdr_elem_t pldm_pdr_elem;
        pldm_fru_elem_t pldm_fru_elem;
        pldm_utils_elem_t pldm_utils_elem;
    } pldm_fw_elem;

} pldm_fw_elem_ctx_t;

/* pldm_fw_ctx type */
typedef struct pldm_fw_ctx {
    pldm_fw_elem_ctx_t *elem_ctx_table;
    /* Number of channels */
    unsigned int elem_count;
} pldm_fw_ctx_t;

/*
 * enumeration to help with pldm_fw discovery states. Discovery has been
 * designed as a state machine.
 */
typedef enum {
    PLDM_FW_REQ_TID = 0,
    PLDM_FW_SET_EVENT_RECEIVER,
    PLDM_FW_GET_PDR,
    PLDM_FW_REQ_TYPES,
    PLDM_FW_REQ_VERSION,
    PLDM_FW_REQ_CMDS,
    PLDM_FW_DISCOVERY_DONE,
} pldm_discovery_t;

/* global object definitions */
static pldm_fw_state_t pldm_fw_state;
static pldm_fw_ctx_t pldm_fw_ctx;
extern pldm_fw_terminus_ctx_t pldm_fw_mcp_ctx;
extern pldm_fw_terminus_ctx_t pldm_fw_bmc_mcp_ctx;

/* pldm requests and response needs to travel down the pldm<->mctp stack before
 * it reaches the corresponding terminal for decoding and preocessing. This api
 * helps in pushing packets down the stack.
 */
static int send_pldm_packet(uint32_t bus, bool tag_owner, uint8_t msg_tag, pldm_msg_t *pkt, size_t size)
{
    uint8_t mctp_msg_type = MCTP_MSG_TYPE_PLDM;

    pldm_fw_elem_ctx_t *elem_ctx =
        &pldm_fw_ctx.elem_ctx_table[PLDM_FW_BIND_MCTP_FW_API_IDX];

    /* Call mctp_fw api to send packet */
    GET_MCTP_FW_API(elem_ctx)->mctp_fw_receive_from_app_layer(
        bus, tag_owner, msg_tag, &mctp_msg_type, pkt, size);

    return FWK_SUCCESS;
}

/* returns the elements ctx */
static pldm_fw_elem_ctx_t *pldm_fw_get_elem_ctx(unsigned int elem_id)
{
    return &pldm_fw_ctx.elem_ctx_table[elem_id];
}

int send_pldm_platform_event_message(uint16_t tid, uint32_t data_transfer_handle)
{
    size_t size = 0;
    int rc = 0;
    pldm_platform_api_t *api = NULL;
    pldm_msg_t *request = NULL;
    pldm_message_poll_event_data_t event_data;

    size =
        PLDM_PLATFORM_EVENT_MESSAGE_MIN_REQ_BYTES + PLDM_MSG_POLL_EVENT_LENGTH;
    request = (pldm_msg_t *)fwk_mm_calloc(1, PLDM_MSG_HDR_T_SIZE + size);

    api = GET_PLDM_PLATFORM_API(
        pldm_fw_get_elem_ctx(PLDM_FW_BIND_PLATFORM_API_IDX));

    event_data.format_version = 0x1;
    event_data.event_id = tid;
    event_data.data_transfer_handle = data_transfer_handle;

    rc = api->encode_platform_event_message_req(
        PLDM_INSTANCE_ID,
        0x1,
        tid,
        PLDM_MESSAGE_POLL_EVENT,
        (uint8_t *)&event_data,
        sizeof(pldm_message_poll_event_data_t),
        request,
        size);

    PLDM_ASSERT(rc);

    send_pldm_packet(MCTP_FW_BIND_SERIAL_API_IDX, 1, 0, request, PLDM_MSG_HDR_T_SIZE + size);
    fwk_mm_free(request);

    return FWK_SUCCESS;
}

int send_pldm_platform_event_message_cper(uint16_t tid, uint32_t data_transfer_handle)
{
    size_t size = 0;
    int rc = 0;
    pldm_platform_api_t *api = NULL;
    pldm_msg_t *request = NULL;
    pldm_cper_event_data_firmware_error_record_reference cper_event_data = {
        .format_version = 0x01,
        .format_type = 0x01,
        .event_data_length = sizeof(EFI_ERROR_SECTION_DESCRIPTOR) + sizeof(EFI_FIRMWARE_ERROR_DATA),
        .section_descriptor = {
            .SectionOffset = 72,
            .SectionLength = sizeof(EFI_FIRMWARE_ERROR_DATA),
            .Revision = 0x0100,
            .SecValidMask = 0x0,
            .SectionFlags = 0x0,
            .SectionType = {0xe19e3d16, 0xbc11, 0x11e4, {0x9c, 0xaa, 0xc2, 0x05, 0x1d, 0x5d, 0x46, 0xb0}},
            .Severity = 3,
        },
        .section_data = {
            .ErrorType = 2,
            .Revision = 2,
            .RecordIdGuid = {0x12345678, 0xabcd, 0xdcba, {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}},
        },
    };

    size =
        PLDM_PLATFORM_EVENT_MESSAGE_MIN_REQ_BYTES +
        sizeof(pldm_cper_event_data_firmware_error_record_reference);
    request = (pldm_msg_t *)fwk_mm_calloc(1, PLDM_MSG_HDR_T_SIZE + size);

    api = GET_PLDM_PLATFORM_API(
        pldm_fw_get_elem_ctx(PLDM_FW_BIND_PLATFORM_API_IDX));

    rc = api->encode_platform_event_message_req(
        PLDM_INSTANCE_ID,
        0x1,
        tid,
        PLDM_CPER_EVENT,
        (uint8_t *)&cper_event_data,
        sizeof(pldm_cper_event_data_firmware_error_record_reference),
        request,
        size);

    PLDM_ASSERT(rc);

    send_pldm_packet(MCTP_FW_BIND_SERIAL_API_IDX, 1, 0, request, PLDM_MSG_HDR_T_SIZE + size);
    fwk_mm_free(request);

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_DEBUGGER
static const char pldm_call[] = "pldm";
static const char pldm_help[] =
    "  Test pldm send platform event message request\n"
    "    Usage: pldm event\n";

static int32_t pldm_f(int32_t argc, char **argv)
{
    uint32_t i = 0;

    if (argc == 1) {
        cli_printf(NONE, "%s\n", pldm_help);
        return FWK_SUCCESS;
    } else if (argc != 2) {
        return FWK_E_PARAM;
    }

    if (cli_strncmp(argv[1], "event", 5) == 0) {
        cli_print("[PLDM FW] Send Platform Event Message Request ... \n");

        return send_pldm_platform_event_message(1, 0);
    } else if (cli_strncmp(argv[1], "cper", 4) == 0) {
        cli_print("[PLDM FW] Send Platform Event Message - CPER ... \n");

        return send_pldm_platform_event_message_cper(1, 0);
    }

    cli_print("CLI: Invalid command received:\n");
    for (i = 0; i < (uint32_t)argc; i++) {
        cli_printf(NONE, "Parameter %d is %s\n", i, argv[i]);
    }

    return FWK_E_PARAM;
}

static cli_command_st cli_commands[] = {
    { pldm_call, pldm_help, &pldm_f, false },
    { 0, 0, 0, 0 }
};
#endif

/*
 * returns pldm request field which indicates if the packet is a request or
 * response
 */
uint8_t get_pldm_request(pldm_msg_t *msg)
{
    return msg->hdr.request;
}

/*
 * Returns a simple hash. This hash would later be used for finding out the
 * proper PLDM command that needs to be handled.
 */
uint16_t pldm_get_hash(pldm_msg_t *pldm_req)
{
    uint8_t command = pldm_req->hdr.command;
    uint8_t type = pldm_req->hdr.type;

    return PLDM_HASH(type, command);
}

/* prints out data received from responder as part of discovery */
void pldm_fw_terminus_info(pldm_fw_terminus_ctx_t *ctx)
{
    pldm_info_t *info = NULL;
    version_t *version = NULL;
    receiver_ev_state_t *ev_state = NULL;
    uint8_t *v = NULL;
    pdr_info_t *pdr_info = NULL;

    ev_state = &ctx->receiver_ev_state;

    FWK_LOG_INFO(MOD_NAME "\n");
    FWK_LOG_INFO_ALIGN_P1(MOD_NAME "pldm tid: %*lu", (uint32_t)ctx->tid);
    FWK_LOG_INFO_ALIGN_P1(
        MOD_NAME "pldm type count: %*lu", (uint32_t)ctx->types_count);

    FWK_LOG_INFO_ALIGN_P1(
        MOD_NAME "global enable: %*lu", (uint32_t)ev_state->global_enable);
    FWK_LOG_INFO_ALIGN_P1(
        MOD_NAME "receiver addr: %*lu", (uint32_t)ev_state->receiver_addr);
    FWK_LOG_INFO_ALIGN_P1(
        MOD_NAME "heartbeat timer: %*lu", (uint32_t)ev_state->heartbeat_timer);
    FWK_LOG_INFO_ALIGN_P1(
        MOD_NAME "transport protocol type: %*lu",
        (uint32_t)ev_state->transport_protocol_type);

    for (size_t i = 0; i < ctx->types_count; i++) {
        info = &ctx->pldm_info[i];
        FWK_LOG_INFO_ALIGN_P1(MOD_NAME "pldm type: %*lu", (uint32_t)info->type);
        FWK_LOG_INFO_ALIGN_P1(
            MOD_NAME "version count: %*lu", (uint32_t)info->version_count);

        for (size_t j = 0; j < info->version_count; j++) {
            version = &info->version[j];
            v = (uint8_t *)&version->version;

            FWK_LOG_INFO_ALIGN_P2(
                MOD_NAME "version[%u] : %*x.%x.%x.%x",
                j,
                (int)v[0],
                (int)v[1],
                (int)v[2],
                (int)v[3]);

            FWK_LOG_INFO_ALIGN_P1(
                MOD_NAME "commands count: %*lu",
                (uint32_t)version->commands_count);

            for (size_t k = 0; k < version->commands_count; k++) {
                FWK_LOG_INFO_ALIGN_P2(
                    MOD_NAME "command[%u]: %*lu",
                    k,
                    (uint32_t)version->commands[k]);
            }
        }
    }

    pdr_info = &ctx->pdr_info;
    for (size_t i = 0; i < pdr_info->idx; i++) {
        FWK_LOG_INFO(MOD_NAME "pdr [%lu]: ", (uint32_t)i);
        for (size_t j = 0; j < pdr_info->pdr[i].sz; j++)
            FWK_LOG_INFO_ALIGN_P1(
                MOD_NAME "%*lu ", (uint32_t)pdr_info->pdr[i].p[j]);
    }
}

/* api is designed based on DSP0240's discovery example.*/
void pldm_fw_discovery(pldm_fw_terminus_ctx_t *ctx)
{
    static pldm_discovery_t pldm_discovery = PLDM_FW_REQ_TID;

    uint8_t idx;
    uint8_t version_idx;
    int rc;
    size_t size = 0;

    pldm_info_t *info;
    pldm_msg_t *request = NULL;

    pldm_base_api_t *pldm_base_api;
    pldm_platform_api_t *pldm_platform_api;

    /* Fetch api object using GET_*_API macro */
    pldm_base_api =
        GET_PLDM_BASE_API(pldm_fw_get_elem_ctx(PLDM_FW_BIND_BASE_API_IDX));
    pldm_platform_api = GET_PLDM_PLATFORM_API(
        pldm_fw_get_elem_ctx(PLDM_FW_BIND_PLATFORM_API_IDX));

    /* pre-set values */
    idx = ctx->idx;
    info = &ctx->pldm_info[idx];
    version_idx = info->idx;

    /* pldm discovery - handshake */
    switch (pldm_discovery) {
    case PLDM_FW_REQ_TID:

        size = PLDM_MSG_T_SIZE;
        request = (pldm_msg_t *)fwk_mm_calloc(1, size);

        MM_ASSERT(request);

        rc = pldm_base_api->encode_get_tid_req(PLDM_INSTANCE_ID, request);
        pldm_discovery = PLDM_FW_SET_EVENT_RECEIVER;
        break;

    case PLDM_FW_SET_EVENT_RECEIVER:

        size = PLDM_MSG_HDR_T_SIZE + PLDM_SET_EVENT_RECEIVER_REQ_BYTES;
        request = (pldm_msg_t *)fwk_mm_calloc(1, size);
        receiver_ev_state_t *ev_state;

        ev_state = &ctx->receiver_ev_state;

        uint8_t event_message_global_enable = ev_state->global_enable;
        uint8_t transport_protocol_type = ev_state->transport_protocol_type;
        uint8_t event_receiver_address_info = ev_state->receiver_addr;
        uint16_t heartbeat_timer = ev_state->heartbeat_timer;

        rc = pldm_platform_api->encode_set_event_receiver_req(
            PLDM_INSTANCE_ID,
            event_message_global_enable,
            transport_protocol_type,
            event_receiver_address_info,
            heartbeat_timer,
            request);
        pldm_discovery = PLDM_FW_GET_PDR;
        break;

    case PLDM_FW_GET_PDR:

        size = PLDM_GET_PDR_REQ_BYTES;
        request = (pldm_msg_t *)fwk_mm_calloc(1, size);

        MM_ASSERT(request);

        /* transfer_op_flag denoting either FIRST_PART or NEXT_PART or the PDR
         * to be retrieved. */
        uint8_t transfer_op_flag = PLDM_GET_FIRSTPART;

        /* The maximum number of record bytes requested to be returned in the
         * response to this instance of the GetPDR command. */
        uint16_t request_cnt = sizeof(pldm_state_sensor_pdr_t);

        /* From pldm platform specification - "If the transferOperationFlag
         * field is set to GetFirstPart, set this value to 0x0000" */
        uint16_t record_chg_num = 0;

        /* record_hndl value for the PDR to be retrieved. 0 is a special value
         * denoting the first PDR from the repository*/
        uint32_t record_hndl = 0;

        /* data_transfer_hndl is used to identify multipart messages. 0 stands
         * for the first part */
        uint32_t data_transfer_hndl = 0;

        /* form get_pdr request */
        rc = pldm_platform_api->encode_get_pdr_req(
            PLDM_INSTANCE_ID,
            record_hndl,
            data_transfer_hndl,
            transfer_op_flag,
            request_cnt,
            record_chg_num,
            request,
            PLDM_GET_PDR_REQ_BYTES);

        pldm_discovery = PLDM_FW_REQ_TYPES;
        break;

    case PLDM_FW_REQ_TYPES:

        size = PLDM_MSG_T_SIZE;
        request = (pldm_msg_t *)fwk_mm_calloc(1, size);

        MM_ASSERT(request);

        rc = pldm_base_api->encode_get_types_req(PLDM_INSTANCE_ID, request);
        pldm_discovery = PLDM_FW_REQ_VERSION;
        break;

    case PLDM_FW_REQ_VERSION:

        if (idx < ctx->types_count) {
            uint32_t transfer_handle = 0;
            size = PLDM_MSG_HDR_T_SIZE + PLDM_GET_VERSION_REQ_BYTES;

            request = (pldm_msg_t *)fwk_mm_calloc(1, size);

            MM_ASSERT(request);

            rc = pldm_base_api->encode_get_version_req(
                PLDM_INSTANCE_ID,
                transfer_handle,
                PLDM_GET_FIRSTPART,
                info->type,
                request);

            pldm_discovery = PLDM_FW_REQ_CMDS;
        } else {
            pldm_discovery = PLDM_FW_DISCOVERY_DONE;
        }
        break;

    case PLDM_FW_REQ_CMDS:

        /* Request commands for each versions separately */
        if (version_idx < info->version_count) {
            size = PLDM_MSG_HDR_T_SIZE + PLDM_GET_COMMANDS_REQ_BYTES;

            request = (pldm_msg_t *)fwk_mm_calloc(1, size);

            MM_ASSERT(request);

            rc = pldm_base_api->encode_get_commands_req(
                PLDM_INSTANCE_ID,
                info->type,
                info->version[version_idx].version,
                request);

            if ((version_idx + 1) < info->version_count) {
                pldm_discovery = PLDM_FW_REQ_CMDS;
            } else {
                if ((idx + 1) < ctx->types_count) {
                    pldm_discovery = PLDM_FW_REQ_VERSION;
                } else {
                    pldm_discovery = PLDM_FW_DISCOVERY_DONE;
                }
            }
        }
        break;

    case PLDM_FW_DISCOVERY_DONE:

        pldm_fw_terminus_info(ctx);

        return;

    default:
        PLDM_ASSERT(PLDM_ERROR_UNSUPPORTED_PLDM_CMD);
    }

    PLDM_ASSERT(rc);
    send_pldm_packet(MCTP_FW_BIND_SERIAL_API_IDX, 1, 0, request, size);
    fwk_mm_free(request);
}

/* pldm unsupport packet processing */
/* api to process pldm unsupport request packets */
void pldm_response_unsupport(
    pldm_msg_t *pldm_req,
    pldm_msg_t **pldm_resp_ptr,
    size_t *resp_len_ptr)
{
    // 1 for CompleteCode
    *resp_len_ptr = PLDM_MSG_HDR_T_SIZE + 1;
    *pldm_resp_ptr = (pldm_msg_t *)fwk_mm_calloc(1, *resp_len_ptr);

    MM_ASSERT(*pldm_resp_ptr);

    /* encode response packet */
    (*pldm_resp_ptr)->hdr.instance_id = pldm_req->hdr.instance_id;
    (*pldm_resp_ptr)->hdr.reserved = 0;
    (*pldm_resp_ptr)->hdr.datagram = 0;
    (*pldm_resp_ptr)->hdr.request = 0;
    (*pldm_resp_ptr)->hdr.type = pldm_req->hdr.type;
    (*pldm_resp_ptr)->hdr.header_ver = pldm_req->hdr.header_ver;
    (*pldm_resp_ptr)->hdr.command = pldm_req->hdr.command;
    (*pldm_resp_ptr)->payload[0] = PLDM_ERROR_UNSUPPORTED_PLDM_CMD;
}

/* pldm set version packet processing */
/* api to process pldm base get tid request packets */
void handle_pldm_set_tid_req(
    pldm_msg_t *pldm_req,
    pldm_msg_t **pldm_resp_ptr,
    size_t *resp_len_ptr,
    pldm_fw_terminus_ctx_t *ctx)
{
    uint8_t completion_code = PLDM_SUCCESS;
    int rc;

    pldm_base_api_t *pldm_base_api;
    pldm_base_api =
        GET_PLDM_BASE_API(pldm_fw_get_elem_ctx(PLDM_FW_BIND_BASE_API_IDX));

    ctx->tid = pldm_req->payload[0];

    /* populate size for response packet and allocate memory */
    *resp_len_ptr = PLDM_MSG_HDR_T_SIZE + PLDM_SET_TID_RESP_BYTES;
    *pldm_resp_ptr = (pldm_msg_t *)fwk_mm_calloc(1, *resp_len_ptr);

    MM_ASSERT(*pldm_resp_ptr);

    /* encode response packet */
    rc = pldm_base_api->encode_cc_only_resp(
        pldm_req->hdr.instance_id,
        pldm_req->hdr.type,
        pldm_req->hdr.command,
        completion_code,
        *pldm_resp_ptr);

    PLDM_ASSERT(rc);
}

/* pldm get version packet processing */
/* api to process pldm base get tid request packets */
void handle_pldm_get_tid_req(
    pldm_msg_t *pldm_req,
    pldm_msg_t **pldm_resp_ptr,
    size_t *resp_len_ptr,
    pldm_fw_terminus_ctx_t *ctx)
{
    uint8_t tid = ctx->tid;
    uint8_t completion_code = PLDM_SUCCESS;
    int rc;

    pldm_base_api_t *pldm_base_api;
    pldm_base_api =
        GET_PLDM_BASE_API(pldm_fw_get_elem_ctx(PLDM_FW_BIND_BASE_API_IDX));

    /* populate size for response packet and allocate memory */
    *resp_len_ptr = PLDM_MSG_HDR_T_SIZE + sizeof(pldm_get_tid_resp_t);
    *pldm_resp_ptr = (pldm_msg_t *)fwk_mm_calloc(1, *resp_len_ptr);

    MM_ASSERT(*pldm_resp_ptr);

    /* encode response packet */
    rc = pldm_base_api->encode_get_tid_resp(
        pldm_req->hdr.instance_id, completion_code, tid, *pldm_resp_ptr);

    PLDM_ASSERT(rc);
}

/* api to process pldm base get tid response packets */
void handle_pldm_get_tid_resp(
    pldm_msg_t *pldm_resp,
    pldm_fw_terminus_ctx_t *ctx)
{
    uint8_t completion_code = PLDM_SUCCESS;
    pldm_base_api_t *pldm_base_api;
    pldm_base_api =
        GET_PLDM_BASE_API(pldm_fw_get_elem_ctx(PLDM_FW_BIND_BASE_API_IDX));

    /* decode response packet */
    int rc = pldm_base_api->decode_get_tid_resp(
        pldm_resp, sizeof(pldm_get_tid_resp_t), &completion_code, &ctx->tid);

    PLDM_ASSERT(rc);
}

/* pldm set event receiver packet processing */
/* api to process pldm set event receiver request packet */
void handle_pldm_set_event_receiver_req(
    pldm_msg_t *pldm_req,
    size_t req_len,
    pldm_msg_t **pldm_resp_ptr,
    size_t *resp_len_ptr,
    pldm_fw_terminus_ctx_t *ev_gen_ctx)
{
    uint8_t event_message_global_enable;
    uint8_t transport_protocol_type;
    uint8_t event_receiver_address_info = 0x08;
    uint8_t completion_code = PLDM_SUCCESS;
    uint8_t instance_id = PLDM_INSTANCE_ID;
    uint16_t heartbeat_timer = 0;
    int rc;

    receiver_ev_state_t *ev_state;
    pldm_platform_api_t *pldm_platform_api;

    /* get pldm_base_api type using GET_*_API macro */
    pldm_platform_api = GET_PLDM_PLATFORM_API(
        pldm_fw_get_elem_ctx(PLDM_FW_BIND_PLATFORM_API_IDX));

    rc = pldm_platform_api->decode_set_event_receiver_req(
        pldm_req,
        PLDM_SET_EVENT_RECEIVER_REQ_BYTES,
        &event_message_global_enable,
        &transport_protocol_type,
        &event_receiver_address_info,
        &heartbeat_timer);

    PLDM_ASSERT(rc);

    /*
     * ctx object passed into this function is that of the event generator.
     * Therefor rather than retrieving information from the context, event
     * receiver ctx is saved into the ev_gen_ctx object.
     */
    ev_state = &ev_gen_ctx->receiver_ev_state;
    ev_state->global_enable = event_message_global_enable;
    ev_state->transport_protocol_type = transport_protocol_type;
    ev_state->receiver_addr = event_receiver_address_info;
    ev_state->heartbeat_timer = heartbeat_timer;

    /* set_event_receiver response packets don't have any additional payload */
    *resp_len_ptr = PLDM_MSG_HDR_T_SIZE;
    *pldm_resp_ptr = fwk_mm_calloc(1, *resp_len_ptr);

    rc = pldm_platform_api->encode_set_event_receiver_resp(
        instance_id, completion_code, *pldm_resp_ptr);

    PLDM_ASSERT(rc);
}

/* pldm get event receiver packet processing */
/* api to process pldm get event receiver request packet */
void handle_pldm_get_event_receiver_req(
    pldm_msg_t *pldm_req,
    pldm_msg_t **pldm_resp_ptr,
    size_t *resp_len_ptr,
    pldm_fw_terminus_ctx_t *ev_gen_ctx)
{
    receiver_ev_state_t *ev_state;
    pldm_platform_api_t *pldm_platform_api;
    uint8_t completion_code = PLDM_SUCCESS;
    int rc;

    /* get pldm_base_api type using GET_*_API macro */
    pldm_platform_api = GET_PLDM_PLATFORM_API(
        pldm_fw_get_elem_ctx(PLDM_FW_BIND_PLATFORM_API_IDX));

    /* populate size for response packet and allocate memory */
    *resp_len_ptr =
        PLDM_MSG_HDR_T_SIZE + sizeof(struct pldm_get_event_receiver_resp);
    *pldm_resp_ptr = fwk_mm_calloc(1, *resp_len_ptr);

    MM_ASSERT(*pldm_resp_ptr);

    /* get ev_state */
    ev_state = &ev_gen_ctx->receiver_ev_state;

    /* encode response packet */
    rc = pldm_platform_api->encode_get_event_receiver_resp(
        pldm_req->hdr.instance_id,
        completion_code,
        ev_state->transport_protocol_type,
        ev_state->receiver_addr,
        *pldm_resp_ptr);

    PLDM_ASSERT(rc);
}

/* api to process pldm set event receiver response packet */
void handle_pldm_set_event_receiver_resp(
    pldm_msg_t *pldm_resp,
    size_t resp_len,
    pldm_fw_terminus_ctx_t *ctx)
{
    uint8_t completion_code;
    int rc;

    pldm_platform_api_t *pldm_platform_api;

    /* get pldm_base_api type using GET_*_API macro */
    pldm_platform_api = GET_PLDM_PLATFORM_API(
        pldm_fw_get_elem_ctx(PLDM_FW_BIND_PLATFORM_API_IDX));

    rc = pldm_platform_api->decode_set_event_receiver_resp(
        pldm_resp, resp_len, &completion_code);

    /* Make sure that both rc and completion code are valid */
    PLDM_ASSERT(rc & completion_code);
}

/* pldm get pldm types packet processing */
/* api to process pldm base get pldm types request packets */
void handle_pldm_get_pldm_types_req(
    pldm_msg_t *pldm_req,
    pldm_msg_t **pldm_resp_ptr,
    size_t *resp_ptr_len,
    pldm_fw_terminus_ctx_t *ctx)
{
    uint8_t completion_code = PLDM_SUCCESS;
    int rc;

    bitfield8_t types[PLDM_TYPES_COUNT];
    pldm_info_t *info;
    pldm_base_api_t *pldm_base_api;

    pldm_base_api =
        GET_PLDM_BASE_API(pldm_fw_get_elem_ctx(PLDM_FW_BIND_BASE_API_IDX));

    memset(&types[0], 0, sizeof(types));

    for (size_t i = 0; i < ctx->types_count; i++) {
        /*
         * encode supported types the particular terminus supports to types
         * object
         */
        info = &ctx->pldm_info[i];
        types[info->type / BITS_PER_BYTE].byte |=
            (1 << info->type % BITS_PER_BYTE);
    }

    /* populate size for response packet and allocate memory */
    *resp_ptr_len = PLDM_MSG_HDR_T_SIZE + sizeof(pldm_get_types_resp_t);
    *pldm_resp_ptr = (pldm_msg_t *)fwk_mm_calloc(1, *resp_ptr_len);

    MM_ASSERT(*pldm_resp_ptr);

    /* encode response packet */
    rc = pldm_base_api->encode_get_types_resp(
        pldm_req->hdr.instance_id, completion_code, &types[0], *pldm_resp_ptr);

    PLDM_ASSERT(rc);
}

/* api to process pldm base get pldm types response packets */
void handle_pldm_get_pldm_types_resp(
    pldm_msg_t *pldm_resp,
    pldm_fw_terminus_ctx_t *ctx)
{
    uint8_t completion_code = PLDM_SUCCESS;
    int rc;

    bitfield8_t types[PLDM_TYPES_COUNT];
    pldm_base_api_t *pldm_base_api;

    pldm_base_api =
        GET_PLDM_BASE_API(pldm_fw_get_elem_ctx(PLDM_FW_BIND_BASE_API_IDX));

    memset(&types[0], 0, sizeof(types));

    /* decode response packet */
    rc = pldm_base_api->decode_get_types_resp(
        pldm_resp, sizeof(pldm_get_types_resp_t), &completion_code, &types[0]);

    PLDM_ASSERT(rc);

    /* Find out the number of types supported using the first pass */
    int k = 0;
    for (size_t i = 0; i < PLDM_TYPES_COUNT; i++) {
        if (types[i].byte != 0) {
            for (size_t j = 0; j < BITS_PER_BYTE; j++) {
                if (types[i].byte & (1 << j)) {
                    k++;
                }
            }
        }
    }

    /* allocate region according the number of types the responder supports */
    ctx->types_count = k;
    ctx->pldm_info = fwk_mm_calloc(k, sizeof(pldm_info_t));

    MM_ASSERT(ctx->pldm_info);

    /*
     * During the second pass populate the newly allocated region with the types
     * that are supported
     */
    k = 0;
    for (size_t i = 0; i < PLDM_TYPES_COUNT; i++) {
        if (types[i].byte != 0) {
            for (size_t j = 0; j < BITS_PER_BYTE; j++) {
                if (types[i].byte & (1 << j)) {
                    ctx->pldm_info[k++].type = i * PLDM_TYPES_COUNT + j;
                }
            }
        }
    }
}

/* pldm get version packet processing */
/* api to process pldm base get version request packets */
void handle_pldm_get_version_req(
    pldm_msg_t *pldm_req,
    size_t req_len,
    pldm_msg_t **pldm_resp_ptr,
    size_t *resp_len_ptr,
    pldm_fw_terminus_ctx_t *ctx)
{
    uint8_t transfer_opflag;
    uint8_t type;
    uint8_t transfer_flag = PLDM_START_AND_END;
    uint8_t completion_code = PLDM_SUCCESS;
    uint32_t transfer_handle;
    uint32_t next_transfer_handle = 1;
    int rc;

    pldm_base_api_t *pldm_base_api;
    pldm_info_t *info;

    pldm_base_api =
        GET_PLDM_BASE_API(pldm_fw_get_elem_ctx(PLDM_FW_BIND_BASE_API_IDX));

    ver32_t *version = NULL;

    /* decode request packet */
    rc = pldm_base_api->decode_get_version_req(
        pldm_req,
        PLDM_GET_VERSION_REQ_BYTES,
        &transfer_handle,
        &transfer_opflag,
        &type);

    PLDM_ASSERT(rc);

    for (size_t i = 0; i < ctx->types_count; i++) {
        if (ctx->pldm_info[i].type == type) {
            info = &ctx->pldm_info[i];
            // TODO add support for multipart versions
            version = &info->version[0].version;
        }
    }

    /* populate size for response packet and allocate memory */
    *resp_len_ptr =
        PLDM_MSG_HDR_T_SIZE + sizeof(pldm_get_version_resp_t) + sizeof(ver32_t);
    *pldm_resp_ptr = (pldm_msg_t *)fwk_mm_calloc(1, *resp_len_ptr);

    MM_ASSERT(*pldm_resp_ptr);

    /* encode response packet */
    rc = pldm_base_api->encode_get_version_resp(
        pldm_req->hdr.instance_id,
        completion_code,
        next_transfer_handle,
        transfer_flag,
        version,
        sizeof(ver32_t),
        *pldm_resp_ptr);

    PLDM_ASSERT(rc);
}

/* api to process pldm base get version response packets */
void handle_pldm_get_version_resp(
    pldm_msg_t *pldm_resp,
    size_t resp_len,
    pldm_fw_terminus_ctx_t *ctx)
{
    uint8_t completion_code;
    uint8_t transfer_flag;
    uint32_t next_transfer_handle;
    int rc;

    pldm_info_t *info;
    ver32_t *version;
    pldm_base_api_t *pldm_base_api;

    pldm_base_api =
        GET_PLDM_BASE_API(pldm_fw_get_elem_ctx(PLDM_FW_BIND_BASE_API_IDX));

    info = &ctx->pldm_info[ctx->idx];

    info->version_count = 1;
    info->version = fwk_mm_calloc(info->version_count, sizeof(version_t));

    MM_ASSERT(info->version);

    info->idx = 0;
    version = &info->version[info->idx].version;

    /* decode response packet */
    rc = pldm_base_api->decode_get_version_resp(
        pldm_resp,
        resp_len,
        &completion_code,
        &next_transfer_handle,
        &transfer_flag,
        version);

    PLDM_ASSERT(rc);
}

/* pldm get commands packet processing */
/* api to process pldm base get commands request packets */
void handle_pldm_get_commands_req(
    pldm_msg_t *pldm_req,
    size_t req_len,
    pldm_msg_t **pldm_resp_ptr,
    size_t *resp_len_ptr,
    pldm_fw_terminus_ctx_t *ctx)
{
    uint8_t type;
    uint8_t completion_code = PLDM_SUCCESS;
    int rc;

    ver32_t version;
    ver32_t any_version = {
      0xFF, 0xFF, 0xFF, 0xFF
    };
    bitfield8_t commands[32];
    pldm_base_api_t *pldm_base_api;
    pldm_info_t *info;
    version_t *ver;

    pldm_base_api =
        GET_PLDM_BASE_API(pldm_fw_get_elem_ctx(PLDM_FW_BIND_BASE_API_IDX));

    memset(&commands[0], 0, sizeof(commands));

    /* decode request packet */
    rc = pldm_base_api->decode_get_commands_req(
        pldm_req, PLDM_GET_COMMANDS_REQ_BYTES, &type, &version);

    PLDM_ASSERT(rc);

    /* Fetch the commands supported at ctx */
    for (size_t i = 0; i < ctx->types_count; i++) {
        info = &ctx->pldm_info[i];
        if (info->type == type) {
            // Any version - 0xFF 0xFF 0xFF 0xFF
            if (memcmp(&any_version, &version, sizeof(ver32_t)) == 0) {
              ver = &info->version[0];
              for (size_t k = 0; k < ver->commands_count; k++) {
                  commands[ver->commands[k] / BITS_PER_BYTE].byte |=
                      (1 << ver->commands[k] % BITS_PER_BYTE);
              }
            } else {
              for (size_t j = 0; j < info->version_count; j++) {
                  ver = &info->version[j];
                  if (memcmp(&ver->version, &version, sizeof(ver32_t)) == 0) {
                      for (size_t k = 0; k < ver->commands_count; k++) {
                          commands[ver->commands[k] / BITS_PER_BYTE].byte |=
                              (1 << ver->commands[k] % BITS_PER_BYTE);
                      }
                  }
              }
            }
        }
    }

    /* populate size for response packet and allocate memory */
    *resp_len_ptr = PLDM_MSG_HDR_T_SIZE + sizeof(pldm_get_commands_resp_t);
    *pldm_resp_ptr = (pldm_msg_t *)fwk_mm_calloc(1, *resp_len_ptr);

    MM_ASSERT(*pldm_resp_ptr);

    /* encode response packet */
    rc = pldm_base_api->encode_get_commands_resp(
        pldm_req->hdr.instance_id,
        completion_code,
        &commands[0],
        *pldm_resp_ptr);

    PLDM_ASSERT(rc);
}

/* api to process pldm base get commands response packets */
void handle_pldm_get_commands_resp(
    pldm_msg_t *pldm_resp,
    size_t resp_len,
    pldm_fw_terminus_ctx_t *ctx)
{
    uint8_t completion_code;
    uint8_t *commands_ptr = NULL;
    uint8_t *commands_count = NULL;
    uint8_t version_idx;
    int rc;

    bitfield8_t commands[32];
    pldm_base_api_t *pldm_base_api = NULL;
    pldm_info_t *info;

    /* pre-set ctx and related pointers */
    info = &ctx->pldm_info[ctx->idx];
    version_idx = info->idx;

    /* get pldm base api to access required fps */
    pldm_base_api =
        GET_PLDM_BASE_API(pldm_fw_get_elem_ctx(PLDM_FW_BIND_BASE_API_IDX));

    memset(&commands[0], 0, sizeof(commands));

    /* decode response packet */
    rc = pldm_base_api->decode_get_commands_resp(
        pldm_resp,
        PLDM_GET_COMMANDS_RESP_BYTES,
        &completion_code,
        &commands[0]);

    /* update ctx with commands count */
    commands_count = &info->version[version_idx].commands_count;

    /*
     * find number of commands that are supported for this particular version of
     * pldm_type
     */
    *commands_count = 0;
    for (size_t i = 0; i < 32; i++) {
        if (commands[i].byte != 0) {
            for (size_t j = 0; j < BITS_PER_BYTE; j++) {
                if (commands[i].byte & (1 << j)) {
                    (*commands_count)++;
                }
            }
        }
    }

    /* allocate memory to hold these commands */
    commands_ptr = fwk_mm_calloc(*commands_count, sizeof(uint8_t));

    MM_ASSERT(commands_ptr);

    /* populate commands to allocated region */
    int k = 0;
    for (size_t i = 0; i < 32; i++) {
        if (commands[i].byte != 0) {
            for (size_t j = 0; j < 8; j++) {
                if (commands[i].byte & (1 << j)) {
                    commands_ptr[k++] = i * PLDM_TYPES_COUNT + j;
                }
            }
        }
    }

    /* assign pointer back to requester ctx */
    info->version[version_idx].commands = commands_ptr;

    PLDM_ASSERT(rc);

    /*
     * increment version idx to indicate that we are done with getting commands
     * for this particular version
     */
    ctx->pldm_info[ctx->idx].idx++;

    if ((ctx->pldm_info[ctx->idx].idx) >=
        ctx->pldm_info[ctx->idx].version_count) {
        /* get commands is the last state/command as part of discovery. It is
         * mandatory to increment the idx if there are no more versions
         * supported under this particular type to indicate that it is safe to
         * proceed to the next pldm type if any.
         */
        ctx->idx++;
    }
}

/* pldm get pdr packet processing */
/* api to process pldm_platform get_pdr request packets */
void handle_pldm_get_pdr_req(
    pldm_msg_t *pldm_req,
    size_t req_len,
    pldm_msg_t **pldm_resp_ptr,
    size_t *resp_len_ptr,
    pldm_fw_terminus_ctx_t *ctx)
{
    uint8_t *record_data;
    uint8_t transfer_op_flag;
    /* Start and end of transfer */
    uint8_t transfer_flag = PLDM_EVENT_START_AND_END;
    /* we don't have a CRC engine to do this. Add in a dummy value */
    uint8_t transfer_crc = 0xFF;
    uint16_t request_cnt;
    uint16_t record_chg_num;
    uint32_t record_hndl;
    uint32_t data_transfer_hndl;
    uint32_t next_record_hndl;
    uint32_t next_data_transfer_hndl = 0x0;
    uint32_t resp_cnt;

    int rc;

    pldm_platform_api_t *pldm_platform_api;
    pldm_pdr_api_t *pldm_pdr_api;

    /* get pldm pdr api to access required fps */
    pldm_pdr_api =
        GET_PLDM_PDR_API(pldm_fw_get_elem_ctx(PLDM_FW_BIND_PDR_API_IDX));
    /* get pldm platform api to access required fps */
    pldm_platform_api = GET_PLDM_PLATFORM_API(
        pldm_fw_get_elem_ctx(PLDM_FW_BIND_PLATFORM_API_IDX));

    /* decode pldm pdr request */
    rc = pldm_platform_api->decode_get_pdr_req(
        pldm_req,
        PLDM_GET_PDR_REQ_BYTES,
        &record_hndl,
        &data_transfer_hndl,
        &transfer_op_flag,
        &request_cnt,
        &record_chg_num);

    PLDM_ASSERT(rc);

    /* find appropriate pdr from pdr repo */
    pldm_pdr_api->pldm_pdr_find_record(
        ctx->pdr_info.repo,
        record_hndl,
        &record_data,
        &resp_cnt,
        &next_record_hndl);

    /* populate size for response packet and allocate memory */
    *resp_len_ptr = PLDM_MSG_HDR_T_SIZE + PLDM_GET_PDR_MIN_RESP_BYTES + resp_cnt;
    *pldm_resp_ptr = (pldm_msg_t *)fwk_mm_calloc(1, *resp_len_ptr);

    MM_ASSERT(*pldm_resp_ptr);

    /* form get_pdr response to send to the requesting terminus */
    rc = pldm_platform_api->encode_get_pdr_resp(
        pldm_req->hdr.instance_id,
        PLDM_SUCCESS,
        next_record_hndl,
        next_data_transfer_hndl,
        transfer_flag,
        (uint16_t)resp_cnt,
        record_data,
        transfer_crc,
        *pldm_resp_ptr);

    PLDM_ASSERT(rc);
}

/* api to process pldm_platform get_pdr response packets */
void handle_pldm_get_pdr_resp(
    pldm_msg_t *pldm_resp,
    size_t resp_len,
    pldm_fw_terminus_ctx_t *ctx)
{
    uint8_t idx;
    uint8_t completion_code;
    uint32_t next_record_hndl;
    uint32_t next_data_transfer_hndl;
    uint8_t transfer_flag;
    uint16_t resp_cnt;
    uint8_t transfer_crc;
    int rc;

    pdr_info_t *pdr_info;

    pdr_info = &ctx->pdr_info;
    idx = pdr_info->idx;

    if (idx >= PLDM_MAX_PDR) {
        return;
    }

    pldm_platform_api_t *pldm_platform_api;

    /* get pldm base api to access required fps */
    pldm_platform_api = GET_PLDM_PLATFORM_API(
        pldm_fw_get_elem_ctx(PLDM_FW_BIND_PLATFORM_API_IDX));

    pdr_info->pdr[idx].sz =
        ((pldm_get_pdr_resp_t *)pldm_resp->payload)->response_count;

    pdr_info->pdr[idx].p = (uint8_t *)fwk_mm_calloc(1, pdr_info->pdr[idx].sz);

    MM_ASSERT(pdr_info->pdr[idx].p);

    /* decode get_pdr response and store in ctx object */
    rc = pldm_platform_api->decode_get_pdr_resp(
        pldm_resp,
        PLDM_GET_PDR_MIN_RESP_BYTES + pdr_info->pdr[idx].sz,
        &completion_code,
        &next_record_hndl,
        &next_data_transfer_hndl,
        &transfer_flag,
        &resp_cnt,
        pdr_info->pdr[idx].p,
        pdr_info->pdr[idx].sz,
        &transfer_crc);

    PLDM_ASSERT(rc);

    ctx->pdr_info.idx++;
}

/* pldm get state sensor reading packet processing */
/* api to process pldm_platform get state sensor reading request packets */
void handle_pldm_get_state_sensor_reading_req(
    pldm_msg_t *pldm_req,
    size_t req_len,
    pldm_msg_t **pldm_resp_ptr,
    size_t *resp_len_ptr,
    pldm_fw_terminus_ctx_t *ctx)
{
    uint16_t sensor_id = 0;
    bitfield8_t rearm_event_state;
    int rc;
    get_sensor_state_field field;

    pldm_platform_api_t *pldm_platform_api;

    /* get pldm platform api to access required fps */
    pldm_platform_api = GET_PLDM_PLATFORM_API(
        pldm_fw_get_elem_ctx(PLDM_FW_BIND_PLATFORM_API_IDX));

    /* decode pldm get sensor reading request */
    rc = pldm_platform_api->decode_get_state_sensor_readings_req(
        pldm_req,
        PLDM_GET_STATE_SENSOR_READINGS_REQ_BYTES,
        &sensor_id,
        &rearm_event_state,
        NULL);

    PLDM_ASSERT(rc);

    /* populate size for response packet and allocate memory */
    *resp_len_ptr =
        PLDM_MSG_HDR_T_SIZE + sizeof(pldm_get_state_sensor_readings_resp_t);
    *pldm_resp_ptr = (pldm_msg_t *)fwk_mm_calloc(1, *resp_len_ptr);

    MM_ASSERT(*pldm_resp_ptr);

    /* TODO: state sensor reading */
    field.sensor_op_state = PLDM_SENSOR_ENABLED;
    field.present_state = PLDM_STATE_SET_HEALTH_STATE_NORMAL;
    field.previous_state = PLDM_STATE_SET_HEALTH_STATE_NON_CRITICAL;
    field.event_state = PLDM_SENSOR_NORMAL;

    /* form get_state_sensor_reading response to send to the requesting terminus
     */
    rc = pldm_platform_api->encode_get_state_sensor_readings_resp(
        pldm_req->hdr.instance_id, PLDM_SUCCESS, 1, &field, *pldm_resp_ptr);

    PLDM_ASSERT(rc);
}

/* pldm get sensor reading packet processing */
/* api to process pldm_platform get_sensor reading request packets */
void handle_pldm_get_sensor_reading_req(
    pldm_msg_t *pldm_req,
    size_t req_len,
    pldm_msg_t **pldm_resp_ptr,
    size_t *resp_len_ptr,
    pldm_fw_terminus_ctx_t *ctx)
{
    uint8_t resp_cnt = 0; // Sensor Reading Uint8
    static uint8_t sensor_value = 0;
    uint16_t sensor_id = 0;
    bool8_t rearm_event_state = 0;
    int rc;

    pldm_platform_api_t *pldm_platform_api;

    /* get pldm platform api to access required fps */
    pldm_platform_api = GET_PLDM_PLATFORM_API(
        pldm_fw_get_elem_ctx(PLDM_FW_BIND_PLATFORM_API_IDX));

    /* decode pldm get sensor reading request */
    rc = pldm_platform_api->decode_get_sensor_reading_req(
        pldm_req,
        PLDM_GET_SENSOR_READING_REQ_BYTES,
        &sensor_id,
        &rearm_event_state);

    PLDM_ASSERT(rc);

    /* populate size for response packet and allocate memory */
    *resp_len_ptr =
        PLDM_MSG_HDR_T_SIZE + sizeof(pldm_get_sensor_reading_resp_t) + resp_cnt;
    *pldm_resp_ptr = (pldm_msg_t *)fwk_mm_calloc(1, *resp_len_ptr);

    MM_ASSERT(*pldm_resp_ptr);

    /* TODO: sensor reading */
    sensor_value++;

    /* form get_pdr response to send to the requesting terminus */
    rc = pldm_platform_api->encode_get_sensor_reading_resp(
        pldm_req->hdr.instance_id,
        PLDM_SUCCESS,
        PLDM_EFFECTER_DATA_SIZE_UINT8,
        PLDM_SENSOR_ENABLED,
        PLDM_NO_EVENT_GENERATION,
        PLDM_SENSOR_NORMAL,
        PLDM_SENSOR_NORMAL,
        PLDM_SENSOR_NORMAL,
        &sensor_value,
        *pldm_resp_ptr,
        sizeof(pldm_get_sensor_reading_resp_t) + resp_cnt);

    PLDM_ASSERT(rc);
}

/* pldm poll for platform event packet processing */
/* api to process pldm_platform poll for platform event request packets */
void handle_pldm_poll_for_platform_event_message(
    pldm_msg_t *pldm_req,
    size_t req_len,
    pldm_msg_t **pldm_resp_ptr,
    size_t *resp_len_ptr,
    pldm_fw_terminus_ctx_t *ctx)
{
    pldm_platform_api_t *pldm_platform_api;
    uint8_t format_version;
    uint8_t transfer_operation_flag;
    uint32_t data_transfer_handle;
    uint16_t event_id_to_acknowledge;
    int rc;

    /* get pldm platform api to access required fps */
    pldm_platform_api = GET_PLDM_PLATFORM_API(
        pldm_fw_get_elem_ctx(PLDM_FW_BIND_PLATFORM_API_IDX));

    /* decode pldm get sensor reading request */
    rc = pldm_platform_api->decode_poll_for_platform_event_message_req(
        pldm_req,
        PLDM_POLL_FOR_PLATFORM_EVENT_MESSAGE_REQ_BYTES,
        &format_version,
        &transfer_operation_flag,
        &data_transfer_handle,
        &event_id_to_acknowledge);

    PLDM_ASSERT(rc);

    static uint8_t event_data[EVENT_MAX_SIZE];
    static uint16_t event_id = 0;
    static uint32_t event_size = 0;
    static uint32_t checksum = 0;
    static uint8_t event_class = 0;

    uint8_t transfer_flag = PLDM_EVENT_START;
    uint8_t completion_code = PLDM_SUCCESS;
    uint16_t xfer_id = 0;
    uint32_t max_xfer_size = (pldm_fw_mcp_ctx.event_terminus_max_buffer_size >
                              pldm_fw_mcp_ctx.event_receiver_max_buffer_size) ?
        pldm_fw_mcp_ctx.event_receiver_max_buffer_size :
        pldm_fw_mcp_ctx.event_terminus_max_buffer_size;
    uint32_t xfer_size = 0;
    uint32_t next_data_transfer_handle = 0;
    size_t payload_length = 0;

    switch (transfer_operation_flag) {
    case PLDM_ACKNOWLEDGEMENT_ONLY:
        if (pldm_fw_mcp_ctx.event_queue.length > 0) {
            xfer_id = 0xffff;
        } else {
            xfer_id = 0;
        }

        if (event_id == event_id_to_acknowledge) {
            event_id = 0; // mark event_data is empty
            completion_code = PLDM_SUCCESS;
        } else {
            completion_code = PLDM_PLATFORM_EVENT_ID_NOT_VALID;
        }

        data_transfer_handle = 0;
        next_data_transfer_handle = 0;
        xfer_size = 0;
        payload_length += PLDM_POLL_FOR_PLATFORM_EVENT_MESSAGE_MIN_RESP_BYTES;
        break;

    case PLDM_GET_FIRSTPART:
        if (event_id == 0) {
            event_queue_get(
                event_data, &event_size, &event_id, &event_class, &checksum);
        }

        if (event_id == 0) {
            // Empty Queue
            xfer_id = event_id;
            payload_length +=
                PLDM_POLL_FOR_PLATFORM_EVENT_MESSAGE_MIN_RESP_BYTES;
        } else {
            // Get first event in queue
            if (event_size >= max_xfer_size) {
                transfer_flag = PLDM_EVENT_START;
                xfer_size = max_xfer_size;
            } else {
                transfer_flag = PLDM_EVENT_START_AND_END;
                xfer_size = event_size;
                // Need checksum at start_and_end op
                payload_length += sizeof(checksum);
            }

            xfer_id = event_id;
            data_transfer_handle = 0;
            next_data_transfer_handle = xfer_size;
            payload_length +=
                (PLDM_POLL_FOR_PLATFORM_EVENT_MESSAGE_RESP_BYTES + xfer_size);
        }
        break;

    case PLDM_GET_NEXTPART:
    default:
        transfer_flag = PLDM_EVENT_MIDDLE;
        xfer_id = event_id;
        if (event_id_to_acknowledge != 0xffff) {
            completion_code = PLDM_PLATFORM_EVENT_ID_NOT_VALID;
            xfer_size = 0;
            data_transfer_handle = 0;
            next_data_transfer_handle = 0;
            break;
        }

        if (event_id == 0x0 || data_transfer_handle > event_size) {
            completion_code = PLDM_PLATFORM_INVALID_DATA_TRANSFER_HANDLE;
            xfer_size = 0;
            data_transfer_handle = 0;
            next_data_transfer_handle = 0;
            break;
        }

        xfer_size = event_size - data_transfer_handle;
        if (xfer_size > max_xfer_size) {
            xfer_size = max_xfer_size;
        }
        next_data_transfer_handle = data_transfer_handle + xfer_size;
        payload_length +=
            (PLDM_POLL_FOR_PLATFORM_EVENT_MESSAGE_RESP_BYTES + xfer_size);

        if ((xfer_size + sizeof(checksum)) <= max_xfer_size) {
            transfer_flag = PLDM_EVENT_END;
            payload_length += sizeof(checksum);
        }
        break;
    }

    /* populate size for response packet and allocate memory */
    *resp_len_ptr = PLDM_MSG_HDR_T_SIZE + payload_length;
    *pldm_resp_ptr = (pldm_msg_t *)fwk_mm_calloc(1, *resp_len_ptr);

    MM_ASSERT(*pldm_resp_ptr);

    rc = pldm_platform_api->encode_poll_for_platform_event_message_resp(
        pldm_req->hdr.instance_id,
        completion_code,
        ctx->tid,
        xfer_id,
        next_data_transfer_handle,
        transfer_flag,
        event_class,
        xfer_size,
        &event_data[data_transfer_handle],
        checksum,
        *pldm_resp_ptr,
        payload_length);

    PLDM_ASSERT(rc);
}

void handle_pldm_platform_event_message_req(
    pldm_msg_t *pldm_req,
    size_t req_len,
    pldm_msg_t **pldm_resp_ptr,
    size_t *resp_len_ptr,
    pldm_fw_terminus_ctx_t *ctx)
{
    uint8_t completion_code = PLDM_SUCCESS;
    uint8_t format_version;
    uint8_t tid;
    uint8_t event_class;
    uint8_t *event_data;
    uint32_t event_data_size = 0;
    size_t event_data_offset;
    uint8_t status;
    uint8_t *sensor_event_class = NULL;
    int rc;
    uint32_t crc32_calc;

    pldm_platform_api_t *pldm_platform_api =
        GET_PLDM_PLATFORM_API(pldm_fw_get_elem_ctx(PLDM_FW_BIND_PLATFORM_API_IDX));
    pldm_utils_api_t *pldm_utils_api =
        GET_PLDM_UTILS_API(pldm_fw_get_elem_ctx(PLDM_FW_BIND_UTILS_API_IDX));

    /* decode request packet */
    rc = pldm_platform_api->decode_platform_event_message_req(
            pldm_req,
            req_len - PLDM_MSG_HDR_T_SIZE,
            &format_version, &tid, &event_class, &event_data_offset);

    PLDM_ASSERT(rc);

    // TODO: Format version
    // TODO: tid

    // Log event message
    event_data = (uint8_t *) &pldm_req->payload[0] + event_data_offset;
    switch(event_class) {
      case PLDM_SENSOR_EVENT:
        // TODO : Check if numeric / state sensor event by SensorEventClass
        sensor_event_class = (uint8_t *) event_data + 2;
        if (*sensor_event_class == 0x1) { // StatSensorState
            event_data_size = sizeof(pldm_event_data_state_sensor_state);
        } else if (*sensor_event_class == 0x2) { // NumericSensorState
            // TODO: data size for int8/uint8, int16/uint16 and int32/uint32
            event_data_size = sizeof(pldm_event_data_numeric_sensor_state_uint8);
        }
        break;
      case PLDM_CPER_EVENT:
        event_data_size = sizeof(pldm_cper_event_data_firmware_error_record_reference);
        break;
      default:
        event_data_size = 0;
        FWK_LOG_ERR("Unsupporteed PLDM Event Class.");
        break;
    };

    crc32_calc = pldm_utils_api->crc32(event_data, event_data_size);
    event_queue_put(event_data, event_data_size, event_class, crc32_calc);

    /* encode response packet */
    *resp_len_ptr =
        PLDM_MSG_HDR_T_SIZE + sizeof(pldm_platform_event_message_resp_t);
    *pldm_resp_ptr = (pldm_msg_t *)fwk_mm_alloc(1, *resp_len_ptr);

    MM_ASSERT(*pldm_resp_ptr);

    completion_code = PLDM_SUCCESS;
    status = PLDM_EVENT_LOGGED;
    rc = pldm_platform_api->encode_platform_event_message_resp(
        pldm_req->hdr.instance_id, completion_code, status, *pldm_resp_ptr);

    PLDM_ASSERT(rc);

    // Send a PLDM event to notify receiver
    struct fwk_event event;
    event = (struct fwk_event) {
        .target_id = fwk_module_id_pldm_fw,
        .source_id = fwk_module_id_pldm_fw,
        .id = FWK_ID_EVENT_INIT(FWK_MODULE_IDX_PLDM_FW, PLDM_FW_EVENT_IDX_PLDM_EVENT_MESSAGE),
    };

    rc = fwk_put_event(&event);
    PLDM_ASSERT(rc);
}

void handle_pldm_platform_event_message_resp(
    pldm_msg_t *pldm_resp,
    size_t resp_len)
{
    uint8_t completion_code = PLDM_SUCCESS;
    uint8_t status = PLDM_EVENT_NO_LOGGING;
    pldm_platform_api_t *pldm_platform_api;

    pldm_platform_api = GET_PLDM_PLATFORM_API(
        pldm_fw_get_elem_ctx(PLDM_FW_BIND_PLATFORM_API_IDX));

    /* decode response packet */
    int rc = pldm_platform_api->decode_platform_event_message_resp(
        pldm_resp, resp_len, &completion_code, &status);

    PLDM_ASSERT(rc);
}

/* pldm event message buffer packet processing */
/* api to process pldm platform event message buffer request packets */
void handle_pldm_event_message_buffer_req(
    pldm_msg_t *pldm_req,
    size_t req_len,
    pldm_msg_t **pldm_resp_ptr,
    size_t *resp_len_ptr,
    pldm_fw_terminus_ctx_t *ctx)
{
    uint8_t completion_code = PLDM_SUCCESS;
    int rc;
    uint16_t max_buffer_size = 0;

    pldm_platform_api_t *pldm_platform_api;
    pldm_platform_api = GET_PLDM_PLATFORM_API(
        pldm_fw_get_elem_ctx(PLDM_FW_BIND_PLATFORM_API_IDX));

    /* decode request packet */
    rc = pldm_platform_api->decode_event_message_buffer_size_req(
        pldm_req, PLDM_EVENT_MESSAGE_BUFFER_SIZE_REQ_BYTES, &max_buffer_size);

    PLDM_ASSERT(rc);

    /* update event state */
    ctx->event_receiver_max_buffer_size = max_buffer_size;

    /* populate size for response packet and allocate memory */
    *resp_len_ptr =
        PLDM_MSG_HDR_T_SIZE + sizeof(pldm_event_message_buffer_size_resp_t);
    *pldm_resp_ptr = (pldm_msg_t *)fwk_mm_calloc(1, *resp_len_ptr);

    MM_ASSERT(*pldm_resp_ptr);

    /* encode response packet */
    rc = pldm_platform_api->encode_event_message_buffer_size_resp(
        pldm_req->hdr.instance_id,
        completion_code,
        ctx->event_terminus_max_buffer_size,
        *pldm_resp_ptr);

    PLDM_ASSERT(rc);
}

/* pldm get fru record table processing */
/* api to process pldm get fru record table request packets */
void handle_pldm_get_fru_record_table(
    pldm_msg_t *pldm_req,
    size_t req_len,
    pldm_msg_t **pldm_resp_ptr,
    size_t *resp_len_ptr,
    pldm_fw_terminus_ctx_t *ctx)
{
    uint8_t completion_code = PLDM_SUCCESS;
    uint32_t data_transfer_handle = 0;
    uint8_t transfer_operation_flag = 0;
    int rc = 0;
    const uint8_t *fru_record_table = NULL;
    int fru_record_table_size = 0;
    int pad = 4 - (sizeof(fru_record_table) % 4); // pad
    pldm_get_fru_record_table_resp_t *resp;
    int ret = 0;

    pldm_fru_api_t *pldm_fru_api;
    pldm_fru_api =
        GET_PLDM_FRU_API(pldm_fw_get_elem_ctx(PLDM_FW_BIND_FRU_API_IDX));

    /* decode request packet */
    rc = pldm_fru_api->decode_get_fru_record_table_req(
        pldm_req,
        req_len - PLDM_MSG_HDR_T_SIZE,
        &data_transfer_handle,
        &transfer_operation_flag);

    PLDM_ASSERT(rc);

    /* get fru record information */
    fru_record_table = get_fru_record_table_data();
    fru_record_table_size = get_fru_record_table_size();
    pad = 4 - (fru_record_table_size % 4);

    /* populate size for response packet and allocate memory */
    *resp_len_ptr = PLDM_MSG_HDR_T_SIZE +
        sizeof(pldm_get_fru_record_table_resp_t) - 1 + fru_record_table_size +
        pad + 4; // 4 for crc
    *pldm_resp_ptr = (pldm_msg_t *)fwk_mm_calloc(1, *resp_len_ptr);

    MM_ASSERT(*pldm_resp_ptr);

    /* encode response packet */
    rc = pldm_fru_api->encode_get_fru_record_table_resp(
        pldm_req->hdr.instance_id,
        completion_code,
        0,
        PLDM_START_AND_END,
        *pldm_resp_ptr);

    /* Fru Record Table */
    resp = (pldm_get_fru_record_table_resp_t *)(*pldm_resp_ptr)->payload;
    memcpy(
        resp->fru_record_table_data, fru_record_table, fru_record_table_size);
    ret += fru_record_table_size;

    /* Pad */
    memset(&resp->fru_record_table_data[ret], 0x0, pad);
    ret += pad;

    /* TODO: Calculate CRC */
    resp->fru_record_table_data[ret++] = 0x1;
    resp->fru_record_table_data[ret++] = 0x2;
    resp->fru_record_table_data[ret++] = 0x3;
    resp->fru_record_table_data[ret++] = 0x4;
    ret += 4;

    PLDM_ASSERT(rc);
}

/*
 * identify the current state machine being used by pldm_fw and call into the
 * right procedure. If states other than discovery is under process, the
 * function that handles this particular state machine needs to be invoked to
 * carry on with the next state/command within the state machine.
 */
void pldm_fw_send_next_request(pldm_fw_terminus_ctx_t *ctx)
{
    switch (pldm_fw_get_state()) {
    case PLDM_FW_DISCOVERY:
        pldm_fw_discovery(ctx);
        break;
    }
}

/*
 * implements request - response framework (From here on rr fwk). rr fwk helps
 * identify a received packet to be either a request or a response. Once
 * identified, they are passed onto their corresponding handlers. For request
 * packets, a well formed response packet is expected at the end of processing.
 * This response packet is layer passed onto the transport layer to carry on
 * with pldm transactions.
 */
void process_pldm_packet(
    pldm_msg_t *pldm_packet,
    size_t len,
    pldm_msg_t **pldm_resp_ptr,
    uint16_t hash,
    size_t *resp_len_ptr,
    pldm_fw_terminus_ctx_t *ctx)
{
    switch (hash) {
    case PLDM_HASH(PLDM_BASE, PLDM_SET_TID):
        if (pldm_resp_ptr != NULL) {
            handle_pldm_set_tid_req(
                pldm_packet, pldm_resp_ptr, resp_len_ptr, ctx);
        }
        break;

    case PLDM_HASH(PLDM_BASE, PLDM_GET_TID):

        if (pldm_resp_ptr != NULL) {
            handle_pldm_get_tid_req(
                pldm_packet, pldm_resp_ptr, resp_len_ptr, ctx);
        } else {
            handle_pldm_get_tid_resp(pldm_packet, ctx);
        }
        break;

    case PLDM_HASH(PLDM_PLATFORM, PLDM_SET_EVENT_RECEIVER):

        if (pldm_resp_ptr != NULL) {
            handle_pldm_set_event_receiver_req(
                pldm_packet, len, pldm_resp_ptr, resp_len_ptr, ctx);
        } else {
            handle_pldm_set_event_receiver_resp(pldm_packet, len, ctx);
        }
        break;

    case PLDM_HASH(PLDM_BASE, PLDM_GET_PLDM_TYPES):

        if (pldm_resp_ptr != NULL) {
            handle_pldm_get_pldm_types_req(
                pldm_packet, pldm_resp_ptr, resp_len_ptr, ctx);
        } else {
            handle_pldm_get_pldm_types_resp(pldm_packet, ctx);
        }
        break;

    case PLDM_HASH(PLDM_BASE, PLDM_GET_PLDM_VERSION):

        if (pldm_resp_ptr != NULL) {
            handle_pldm_get_version_req(
                pldm_packet, len, pldm_resp_ptr, resp_len_ptr, ctx);
        } else {
            handle_pldm_get_version_resp(pldm_packet, len, ctx);
        }
        break;

    case PLDM_HASH(PLDM_BASE, PLDM_GET_PLDM_COMMANDS):

        if (pldm_resp_ptr != NULL) {
            handle_pldm_get_commands_req(
                pldm_packet, len, pldm_resp_ptr, resp_len_ptr, ctx);
        } else {
            handle_pldm_get_commands_resp(pldm_packet, len, ctx);
        }
        break;

    case PLDM_HASH(PLDM_PLATFORM, PLDM_GET_PDR):

        if (pldm_resp_ptr != NULL) {
            handle_pldm_get_pdr_req(
                pldm_packet, len, pldm_resp_ptr, resp_len_ptr, ctx);
        } else {
            handle_pldm_get_pdr_resp(pldm_packet, len, ctx);
        }
        break;

    case PLDM_HASH(PLDM_PLATFORM, PLDM_GET_SENSOR_READING):

        if (pldm_resp_ptr != NULL) {
            handle_pldm_get_sensor_reading_req(
                pldm_packet, len, pldm_resp_ptr, resp_len_ptr, ctx);
        }
        break;

    case PLDM_HASH(PLDM_PLATFORM, PLDM_GET_STATE_SENSOR_READINGS):
        if (pldm_resp_ptr != NULL) {
            handle_pldm_get_state_sensor_reading_req(
                pldm_packet, len, pldm_resp_ptr, resp_len_ptr, ctx);
        }
        break;

    case PLDM_HASH(PLDM_PLATFORM, PLDM_POLL_FOR_PLATFORM_EVENT_MESSAGE):
        if (pldm_resp_ptr != NULL) {
            handle_pldm_poll_for_platform_event_message(
                pldm_packet, len, pldm_resp_ptr, resp_len_ptr, ctx);
        }
        break;

    case PLDM_HASH(PLDM_PLATFORM, PLDM_PLATFORM_EVENT_MESSAGE):
        if (pldm_resp_ptr != NULL) {
            handle_pldm_platform_event_message_req(
                pldm_packet, len, pldm_resp_ptr, resp_len_ptr, ctx);
        } else {
            handle_pldm_platform_event_message_resp(pldm_packet, len);
        }
        break;

    case PLDM_HASH(PLDM_PLATFORM, PLDM_GET_EVENT_RECEIVER):
        if (pldm_resp_ptr != NULL) {
            handle_pldm_get_event_receiver_req(
                pldm_packet, pldm_resp_ptr, resp_len_ptr, ctx);
        }
        break;

    case PLDM_HASH(PLDM_PLATFORM, PLDM_EVENT_MESSAGE_BUFFER_SIZE):
        if (pldm_resp_ptr != NULL) {
            handle_pldm_event_message_buffer_req(
                pldm_packet, len, pldm_resp_ptr, resp_len_ptr, ctx);
        }
        break;

    case PLDM_HASH(PLDM_FRU, PLDM_GET_FRU_RECORD_TABLE):
        if (pldm_resp_ptr != NULL) {
            handle_pldm_get_fru_record_table(
                pldm_packet, len, pldm_resp_ptr, resp_len_ptr, ctx);
        }
        break;

    default:
        FWK_LOG_INFO(MOD_NAME "PLDM_ERROR_UNSUPPORTED_PLDM_CMD");
        if (pldm_resp_ptr != NULL) {
            pldm_response_unsupport(pldm_packet, pldm_resp_ptr, resp_len_ptr);
        }
        break;
    }
}

/* receive's data from transport layer and processes it accordingly */
static void pldm_fw_receive_from_transport_layer(
    void *data,
    void *pldm_packet,
    size_t len)
{
    uint8_t request;
    uint16_t hash;
    size_t size = 0;
    pldm_msg_t *pldm_resp = NULL;
    pldm_msg_t *msg = (pldm_msg_t *)pldm_packet;
    char *buf = (char *)pldm_packet;
    uint32_t *bus = (uint32_t *)data;

    request = get_pldm_request(pldm_packet);
    hash = pldm_get_hash(pldm_packet);

    (void)msg;
    (void)buf;

    FWK_LOG_INFO(MOD_NAME "Request : %d from %ld", msg->hdr.request, *bus);
    FWK_LOG_INFO(MOD_NAME "Instance_id : %d ", msg->hdr.instance_id);
    FWK_LOG_INFO(MOD_NAME "Type : %d ", msg->hdr.type);
    FWK_LOG_INFO(MOD_NAME "Ver : %d ", msg->hdr.header_ver);
    FWK_LOG_INFO(MOD_NAME "Command : %d ", msg->hdr.command);

#if FWK_LOG_LEVEL < FWK_LOG_LEVEL_INFO
    FWK_LOG_DEBUG(MOD_NAME "Rx Len : %d ", len);
    for (size = 0; size < len; size++) {
        FWK_LOG_DEBUG("Rx: 0x%02x ", buf[size]);
    }
#endif

    /*
     * If the received packet is a request, pass the response pointer and
     * response len pointers along to the handling api. The handlers would
     * process the request, take the corresponding action and populates the
     * response and response length pointers. These are then pushed to the
     * transport layer.
     *
     * If the received packet is a response, it could be passed on directly with
     * no addtional fields.
     * */
    if (request == PLDM_REQUEST) {
        process_pldm_packet(
            pldm_packet, len, &pldm_resp, hash, &size, &pldm_fw_mcp_ctx);
        send_pldm_packet(*bus, 0, 0, pldm_resp, size);

        fwk_mm_free(pldm_resp);
    } else {
        process_pldm_packet(
            pldm_packet, len, NULL, hash, &size, &pldm_fw_bmc_mcp_ctx);
    }
}

/* pldm_fw_api_t definition for valid functions exposed */
static const pldm_fw_api_t mod_pldm_fw_api = {
    .pldm_fw_receive_from_transport_layer =
        pldm_fw_receive_from_transport_layer,
};

/* apis exposed by this module would be shared via bind request */
static int pldm_fw_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case PLDM_FW_BIND_REQ_API_IDX:
        *api = &mod_pldm_fw_api;
        break;

    default:
        return FWK_E_PARAM;
    }
    return FWK_SUCCESS;
}

/* start off pldm handshakes starting from discovery */
static int pldm_fw_start(fwk_id_t id)
{
    int status;
    uint32_t index;

    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT) &&
        fwk_id_get_element_idx(id) == PLDM_FW_BIND_BASE_API_IDX) {
        pldm_fw_elem_ctx_t *elem_ctx =
            &pldm_fw_ctx.elem_ctx_table[PLDM_FW_BIND_PDR_API_IDX];

        /* Fwk initially starts off with discovery */
        pldm_fw_set_state(PLDM_FW_DISCOVERY);

        /*
         * mcp needs to be initialized first as bmc init would start off the
         * discovery.
         */
        mcp_init(GET_PLDM_PDR_API(elem_ctx));

#ifdef BUILD_HAS_DEBUGGER
        // Debugger command
        for (index = 0; cli_commands[index].command != 0; index++) {
            status = cli_command_register(cli_commands[index]);
            if (status != FWK_SUCCESS) {
                FWK_LOG_ERR(MOD_NAME "Debugger command register failed.");
                return status;
            }
        }
#endif
    }

    return FWK_SUCCESS;
}

/* pldm_fw binds with other modules within this api */
static int pldm_fw_bind(fwk_id_t id, unsigned int round)
{
    int status = FWK_SUCCESS;
    pldm_fw_elem_ctx_t *elem_ctx;

    if (round == 0 && fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        elem_ctx = pldm_fw_get_elem_ctx(fwk_id_get_element_idx(id));

        switch (fwk_id_get_element_idx(id)) {
        /* bind to pldm_base spec - required for most of discovery */
        case PLDM_FW_BIND_BASE_API_IDX:
            status = fwk_module_bind(
                elem_ctx->pldm_fw_config->driver_id,
                elem_ctx->pldm_fw_config->driver_api_id,
                &(GET_PLDM_BASE_API(elem_ctx)));
            break;

        /* bind to mctp fw to get fp for pushing data to mctp layer */
        case PLDM_FW_BIND_MCTP_FW_API_IDX:
            status = fwk_module_bind(
                elem_ctx->pldm_fw_config->driver_id,
                elem_ctx->pldm_fw_config->driver_api_id,
                &(GET_MCTP_FW_API(elem_ctx)));
            break;

        case PLDM_FW_BIND_PLATFORM_API_IDX:
            status = fwk_module_bind(
                elem_ctx->pldm_fw_config->driver_id,
                elem_ctx->pldm_fw_config->driver_api_id,
                &(GET_PLDM_PLATFORM_API(elem_ctx)));
            break;

        case PLDM_FW_BIND_PDR_API_IDX:
            status = fwk_module_bind(
                elem_ctx->pldm_fw_config->driver_id,
                elem_ctx->pldm_fw_config->driver_api_id,
                &(GET_PLDM_PDR_API(elem_ctx)));
            break;

        case PLDM_FW_BIND_FRU_API_IDX:
            status = fwk_module_bind(
                elem_ctx->pldm_fw_config->driver_id,
                elem_ctx->pldm_fw_config->driver_api_id,
                &(GET_PLDM_FRU_API(elem_ctx)));
            break;

        case PLDM_FW_BIND_UTILS_API_IDX:
            status = fwk_module_bind(
                elem_ctx->pldm_fw_config->driver_id,
                elem_ctx->pldm_fw_config->driver_api_id,
                &(GET_PLDM_UTILS_API(elem_ctx)));
            break;

        default:
            return FWK_E_HANDLER;
        }

        if (status != FWK_SUCCESS) {
            return FWK_E_HANDLER;
        }
        elem_ctx->driver_id = elem_ctx->pldm_fw_config->driver_id;
    }
    return status;
}

/* pldm_fw module init */
static int pldm_fw_init(
    fwk_id_t module_id,
    unsigned int elem_count,
    const void *config)
{
    pldm_fw_ctx.elem_ctx_table =
        fwk_mm_calloc(elem_count, sizeof(pldm_fw_ctx.elem_ctx_table[0]));

    MM_ASSERT(pldm_fw_ctx.elem_ctx_table);

    if (pldm_fw_ctx.elem_ctx_table == NULL) {
        return FWK_E_NOMEM;
    }

    pldm_fw_ctx.elem_count = elem_count;
    return FWK_SUCCESS;
}

/* pldm_fw element init */
static int pldm_fw_elem_init(
    fwk_id_t elem_id,
    unsigned int unused,
    const void *data)
{
    pldm_fw_elem_ctx_t *elem_ctx;

    elem_ctx = &pldm_fw_ctx.elem_ctx_table[fwk_id_get_element_idx(elem_id)];
    elem_ctx->pldm_fw_config = (pldm_fw_elem_config_t *)data;
    elem_ctx->id = elem_id;

    return FWK_SUCCESS;
}

static int pldm_fw_process_event(const struct fwk_event *event,
                                  struct fwk_event *resp)
{
    int rc = 0;
    switch (fwk_id_get_event_idx(event->id)) {
    case PLDM_FW_EVENT_IDX_PLDM_EVENT_MESSAGE:
        rc = send_pldm_platform_event_message(1, 0);
        if (rc != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME "Sending pldm platform event message failed : %d.", rc);
        }
        return FWK_SUCCESS;
    default:
        FWK_LOG_ERR(MOD_NAME "Invalid event request: %s.", FWK_ID_STR(event->id));
        return FWK_E_PARAM;
    }
}

/* module description */
const struct fwk_module module_pldm_fw = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .init = pldm_fw_init,
    .api_count = PLDM_FW_BIND_REQ_API_IDX_COUNT,
    .event_count = PLDM_FW_EVENT_IDX_COUNT,
    .element_init = pldm_fw_elem_init,
    .bind = pldm_fw_bind,
    .start = pldm_fw_start,
    .process_bind_request = pldm_fw_process_bind_request,
    .process_event = pldm_fw_process_event,
};
