/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_pldm.h>

#include <fwk_module.h>

#define MOD_NAME "[PLDM]: "

static int encode_get_event_receiver_resp(
    uint8_t instance_id,
    uint8_t completion_code,
    uint8_t transport_protocol_type,
    uint8_t event_receiver_address_info,
    struct pldm_msg *msg)
{
    if (msg == NULL) {
        return PLDM_ERROR_INVALID_DATA;
    }

    struct pldm_header_info header = { 0 };
    header.instance = instance_id;
    header.msg_type = PLDM_RESPONSE;
    header.pldm_type = PLDM_PLATFORM;
    header.command = PLDM_GET_EVENT_RECEIVER;

    uint8_t rc = pack_pldm_header(&header, &(msg->hdr));
    if (rc != PLDM_SUCCESS) {
        return rc;
    }

    struct pldm_get_event_receiver_resp *response =
        (struct pldm_get_event_receiver_resp *)msg->payload;
    response->completion_code = completion_code;
    response->transport_protocol_type = transport_protocol_type;
    response->event_receiver_address = event_receiver_address_info;

    return PLDM_SUCCESS;
}

static int decode_event_message_buffer_size_req(
    const struct pldm_msg *msg,
    size_t payload_length,
    uint16_t *event_receiver_max_buffer_size)
{
    if (msg == NULL || event_receiver_max_buffer_size == NULL) {
        return PLDM_ERROR_INVALID_DATA;
    }

    if (payload_length != PLDM_EVENT_MESSAGE_BUFFER_SIZE_REQ_BYTES) {
        return PLDM_ERROR_INVALID_LENGTH;
    }

    struct pldm_event_message_buffer_size_req *request =
        (struct pldm_event_message_buffer_size_req *)msg->payload;

    *event_receiver_max_buffer_size = request->event_receiver_max_buffer_size;

    return PLDM_SUCCESS;
}

static int encode_event_message_buffer_size_resp(
    uint8_t instance_id,
    uint8_t completion_code,
    uint16_t terminus_max_buffer_size,
    struct pldm_msg *msg)
{
    if (msg == NULL) {
        return PLDM_ERROR_INVALID_DATA;
    }

    struct pldm_header_info header = { 0 };
    header.msg_type = PLDM_RESPONSE;
    header.instance = instance_id;
    header.pldm_type = PLDM_PLATFORM;
    header.command = PLDM_EVENT_MESSAGE_BUFFER_SIZE;

    uint8_t rc = pack_pldm_header(&header, &(msg->hdr));
    if (rc != PLDM_SUCCESS) {
        return rc;
    }

    struct pldm_event_message_buffer_size_resp *response =
        (struct pldm_event_message_buffer_size_resp *)msg->payload;
    response->completion_code = completion_code;
    response->terminus_max_buffer_size = terminus_max_buffer_size;

    return PLDM_SUCCESS;
}

/*
 * pldm_base_api_t definition points to valid definitions for the the apis that
 * are exposed from this module via bind request
 */
static pldm_base_api_t pldm_base_api = {
    .encode_cc_only_resp = encode_cc_only_resp,
    .decode_get_tid_resp = decode_get_tid_resp,
    .encode_get_tid_resp = encode_get_tid_resp,
    .encode_get_tid_req = encode_get_tid_req,
    .decode_get_version_resp = decode_get_version_resp,
    .decode_get_version_req = decode_get_version_req,
    .encode_get_version_resp = encode_get_version_resp,
    .encode_get_version_req = encode_get_version_req,
    .decode_get_commands_resp = decode_get_commands_resp,
    .decode_get_types_resp = decode_get_types_resp,
    .encode_get_commands_resp = encode_get_commands_resp,
    .decode_get_commands_req = decode_get_commands_req,
    .encode_get_types_resp = encode_get_types_resp,
    .encode_get_commands_req = encode_get_commands_req,
    .encode_get_types_req = encode_get_types_req,
};

static pldm_platform_api_t pldm_platform_api = {
    .decode_set_event_receiver_req = decode_set_event_receiver_req,
    .decode_set_event_receiver_resp = decode_set_event_receiver_resp,
    .encode_set_event_receiver_resp = encode_set_event_receiver_resp,
    .encode_set_event_receiver_req = encode_set_event_receiver_req,
    .decode_platform_event_message_resp = decode_platform_event_message_resp,
    .encode_platform_event_message_req = encode_platform_event_message_req,
    .encode_platform_event_message_resp = encode_platform_event_message_resp,
    .decode_platform_event_message_req = decode_platform_event_message_req,
    .encode_get_pdr_req = encode_get_pdr_req,
    .encode_get_pdr_resp = encode_get_pdr_resp,
    .decode_get_pdr_req = decode_get_pdr_req,
    .decode_get_pdr_resp = decode_get_pdr_resp,
    .decode_get_sensor_reading_req = decode_get_sensor_reading_req,
    .encode_get_sensor_reading_resp = encode_get_sensor_reading_resp,
    .decode_get_state_sensor_readings_req =
        decode_get_state_sensor_readings_req,
    .encode_get_state_sensor_readings_resp =
        encode_get_state_sensor_readings_resp,
    .decode_poll_for_platform_event_message_req =
        decode_poll_for_platform_event_message_req,
    .encode_poll_for_platform_event_message_resp =
        encode_poll_for_platform_event_message_resp,
    .encode_get_event_receiver_resp = encode_get_event_receiver_resp,
    .decode_event_message_buffer_size_req =
        decode_event_message_buffer_size_req,
    .encode_event_message_buffer_size_resp =
        encode_event_message_buffer_size_resp,
};

/* api's exposed by pldm_pdr library */
static pldm_pdr_api_t pldm_pdr_api = {
    .pldm_pdr_init = pldm_pdr_init,
    .pldm_pdr_add = pldm_pdr_add,
    .pldm_pdr_destroy = pldm_pdr_destroy,
    .pldm_pdr_find_record = pldm_pdr_find_record,
    .pldm_pdr_get_next_record = pldm_pdr_get_next_record,
    .pldm_pdr_find_record_by_type = pldm_pdr_find_record_by_type,
    .pldm_pdr_get_record_count = pldm_pdr_get_record_count,
    .pldm_pdr_get_repo_size = pldm_pdr_get_repo_size,
    .pldm_pdr_get_record_handle = pldm_pdr_get_record_handle,
    .pldm_pdr_fru_record_set_find_by_rsi = pldm_pdr_fru_record_set_find_by_rsi,
};

static pldm_fru_api_t pldm_fru_api = {
    .decode_get_fru_record_table_req = decode_get_fru_record_table_req,
    .encode_get_fru_record_table_resp = encode_get_fru_record_table_resp,
};

static pldm_utils_api_t pldm_utils_api = {
    .crc32 = crc32,
    .crc8 = crc8,
};

/* api takes care of bind requests for the module */
static int pldm_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case PLDM_BASE_BIND_REQ_API_IDX:
        *api = &pldm_base_api;
        break;
    case PLDM_PLATFORM_BIND_REQ_API_IDX:
        *api = &pldm_platform_api;
        break;
    case PLDM_PDR_BIND_REQ_API_IDX:
        *api = &pldm_pdr_api;
        break;
    case PLDM_FRU_BIND_REQ_API_IDX:
        *api = &pldm_fru_api;
        break;
    case PLDM_UTILS_BIND_REQ_API_IDX:
        *api = &pldm_utils_api;
        break;

    default:
        /* Invalid API */
        assert(false);
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int pldm_base_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    /* Module does not support elements */
    if (element_count > 0)
        return FWK_E_DATA;

    return FWK_SUCCESS;
}

/* Module descriptor */
const struct fwk_module module_pldm = {
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .api_count = PLDM_API_BIND_REQ_API_IDX_COUNT,
    .init = pldm_base_init,
    .process_bind_request = pldm_process_bind_request,
};
