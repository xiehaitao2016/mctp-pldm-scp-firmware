/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/mod_pldm_fw_int.h>

#include <mod_pldm.h>
#include <mod_pldm_fw.h>

#include <fwk_log.h>
#include <fwk_mm.h>

#define MOD_NAME "[MCP]: "
#define MCP_TID  0

pldm_fw_terminus_ctx_t pldm_fw_mcp_ctx;

pldm_cper_event_data_firmware_error_record_reference cper_event_data1 = {
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

pldm_event_data_numeric_sensor_state_uint8 numeric_sensor_event_data1 = {
    .sensor_id = 0x01,
    .sensor_event_class = PLDM_NUMERIC_SENSOR_STATE,
    .event_state = PLDM_SENSOR_WARNING,
    .previous_event_state = PLDM_SENSOR_NORMAL,
    .sensor_data_size = PLDM_SENSOR_DATA_SIZE_UINT8,
    .presentReading = 0x5b,
};

pldm_event_data_state_sensor_state state_sensor_event_data1 = {
    .sensor_id = 0x02,
    .sensor_event_class = PLDM_STATE_SENSOR_STATE,
    .sensor_offset = 1,
    .event_state = PLDM_SENSOR_WARNING,
    .previous_event_state = PLDM_SENSOR_NORMAL,
};

// Fru Record Table
const uint8_t fru_record_table[] = {
    0x0, // record_set_id
    0x0,
    0x01, // record_type:general FRU record
    0x03, // number of FRU fields
    0x01, // encoding type=ASCII
    // 5
    // FRU Field#1
    0x03, // type:part number
    0x06, // length
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    // 9
    // FRU field#2
    0x04, // type:serial number
    0x07, // length
    'S',
    'N',
    '1',
    '2',
    '3',
    '4',
    '5',
    // 7
    // FRU field#3
    0x08, // type:name
    0x05, // length
    'S',
    'a',
    't',
    'M',
    'C',
};

int get_fru_record_table_size()
{
    return sizeof(fru_record_table);
}

const uint8_t *get_fru_record_table_data()
{
    return fru_record_table;
}

int event_queue_put(
    uint8_t *event_data,
    uint32_t event_size,
    uint8_t event_class,
    uint32_t checksum)
{
    static uint16_t event_id = 1;
    struct pldm_event_queue *queue = &pldm_fw_mcp_ctx.event_queue;

    if (event_size > EVENT_MAX_SIZE) {
        return -1; // over max event size
    }

    if (queue->tail == queue->head && queue->length > 0) {
        // queue is full
        // DSP0248 13.4 PLDM event log clearing policies
        // Policy: FIFO, when queue full, discard oldest event
        queue->head = (queue->tail + 1) % EVENT_QUEUE_SIZE;
        ;
        FWK_LOG_INFO(MOD_NAME "event_queue_put 1");
    } else {
        FWK_LOG_INFO(MOD_NAME "event_queue_put 2");
        queue->length++;
    }
    queue->events[queue->tail].class = event_class;
    queue->events[queue->tail].id = event_id;
    queue->events[queue->tail].size = event_size;
    queue->events[queue->tail].checksum = checksum;
    memcpy(queue->events[queue->tail].data, event_data, event_size);
    queue->tail = (queue->tail + 1) % EVENT_QUEUE_SIZE;

    event_id++;
    if (event_id == 0xffff) {
        event_id = 1;
    }
    return 0;
}

int event_queue_get(
    uint8_t *event_data,
    uint32_t *event_size,
    uint16_t *event_id,
    uint8_t *event_class,
    uint32_t *checksum)
{
    struct pldm_event_queue *queue = &pldm_fw_mcp_ctx.event_queue;

    if (queue->length == 0) {
        *event_id = 0;
        *event_size = 0;
        *event_class = 0;
        *checksum = 0;
        FWK_LOG_INFO(MOD_NAME "event_queue_get 1, empty queue");
    } else {
        *event_id = queue->events[queue->head].id;
        *event_size = queue->events[queue->head].size;
        *event_class = queue->events[queue->head].class;
        *checksum = queue->events[queue->head].checksum;
        memcpy(
            event_data,
            queue->events[queue->head].data,
            queue->events[queue->head].size);
        queue->head = (queue->head + 1) % EVENT_QUEUE_SIZE;
        queue->length--;
        FWK_LOG_INFO(
            MOD_NAME
            "event_queue_get id=0x%04x, size=%ld, class=0x%02x checksum=%08lx",
            *event_id,
            *event_size,
            *event_class,
            *checksum);
    }
    return 0;
}

/*
 * mcp acts as a responder. BMC would implement the initializing agent which is
 * responsible for carrying out disccovery
 */
void mcp_set_ctx(pldm_fw_terminus_ctx_t *ctx)
{
    ver32_t versions[] = {
        { 0xF1, 0xF0, 0xF0, 0x00 },
        { 0xF1, 0xF2, 0xF0, 0x00 },
    };

    /*
     * Though we statically configure the TID. It is upto BMC to decide if this
     * TID is okay for this node. If not, it could change this field using set
     * tid command
     */
    ctx->tid = MCP_TID;
    ctx->types_count = 2;

    ctx->pldm_info = fwk_mm_calloc(ctx->types_count, sizeof(pldm_info_t));

    MM_ASSERT(ctx->pldm_info);

    ctx->pldm_info[0].type = PLDM_BASE;
    ctx->pldm_info[0].version_count = 1;
    ctx->pldm_info[0].version =
        fwk_mm_calloc(ctx->pldm_info[0].version_count, sizeof(version_t));

    MM_ASSERT(ctx->pldm_info[0].version);

    ctx->pldm_info[0].version[0].version = versions[0];
    ctx->pldm_info[0].version[0].commands_count = 4;
    ctx->pldm_info[0].version[0].commands = fwk_mm_calloc(
        ctx->pldm_info[0].version[0].commands_count, sizeof(uint8_t));

    MM_ASSERT(ctx->pldm_info[0].version[0].commands);

    ctx->pldm_info[0].version[0].commands[0] = PLDM_GET_TID;
    ctx->pldm_info[0].version[0].commands[1] = PLDM_GET_PLDM_VERSION;
    ctx->pldm_info[0].version[0].commands[2] = PLDM_GET_PLDM_TYPES;
    ctx->pldm_info[0].version[0].commands[3] = PLDM_GET_PLDM_COMMANDS;

    ctx->pldm_info[1].type = PLDM_PLATFORM;
    ctx->pldm_info[1].version_count = 1;
    ctx->pldm_info[1].version =
        fwk_mm_calloc(ctx->pldm_info[1].version_count, sizeof(version_t));

    MM_ASSERT(ctx->pldm_info[0].version);

    ctx->pldm_info[1].version[0].version = versions[1];
    ctx->pldm_info[1].version[0].commands_count = 5;
    ctx->pldm_info[1].version[0].commands = fwk_mm_calloc(
        ctx->pldm_info[1].version[0].commands_count, sizeof(uint8_t));

    MM_ASSERT(ctx->pldm_info[0].version[0].commands);

    ctx->pldm_info[1].version[0].commands[0] = PLDM_SET_NUMERIC_EFFECTER_VALUE;
    ctx->pldm_info[1].version[0].commands[1] = PLDM_SET_STATE_EFFECTER_STATES;
    ctx->pldm_info[1].version[0].commands[2] = PLDM_GET_PDR;
    ctx->pldm_info[1].version[0].commands[3] = PLDM_SET_EVENT_RECEIVER;
    ctx->pldm_info[1].version[0].commands[4] = PLDM_PLATFORM_EVENT_MESSAGE;

    ctx->event_terminus_max_buffer_size = EVENT_MAX_BUFFER_SIZE;
    ctx->event_receiver_max_buffer_size = EVENT_MAX_BUFFER_SIZE;
}

/*
 * this function would initialize pldm pdr objects and add them to pdr
 * repository.
 */
void init_pldm_state_sensor_pdr(pldm_state_sensor_pdr_t *pdr)
{
    pdr->hdr.record_handle = 0x01;
    pdr->hdr.version = 0x01;
    pdr->hdr.type = PLDM_STATE_SENSOR_PDR;
    pdr->hdr.record_change_num = 0x0;
    pdr->hdr.length =
        sizeof(pldm_state_sensor_pdr_t) - sizeof(struct pldm_pdr_hdr);

    pdr->terminus_handle = 0x0;
    pdr->sensor_id = 0x01;
    pdr->entity_type = 0x07;
    pdr->entity_instance = 0x0;
    pdr->container_id = 0x0;
    pdr->sensor_init = 0x0;
    pdr->sensor_auxiliary_names_pdr = false;
    pdr->composite_sensor_count = 0x1;
    pdr->possible_states[0] = PLDM_STATE_SET_HEALTH_STATE & 0xFF;
    pdr->possible_states[1] = (PLDM_STATE_SET_HEALTH_STATE >> 8) & 0xFF;
    pdr->possible_states[2] = 0x01;
    pdr->possible_states[3] = 0x0f;
}

void init_pldm_numeric_sensor_pdr(pldm_numeric_sensor_pdr_t *pdr)
{
    /* setup pdr object with dummy values */
    pdr->hdr.record_handle = 0x01;
    pdr->hdr.version = 0x01;
    pdr->hdr.type = PLDM_NUMERIC_SENSOR_PDR;
    pdr->hdr.record_change_num = 0x0;
    pdr->hdr.length =
        sizeof(pldm_numeric_sensor_pdr_t) - sizeof(struct pldm_pdr_hdr);

    pdr->terminus_handle = 0x1;
    pdr->sensor_id = 0x01;
    pdr->entity_type = 0x07;
    pdr->entity_instance = 0x0;
    pdr->container_id = 0x0;
    pdr->sensor_init = 0x0; // noInit
    pdr->sensor_auxiliary_names_pdr = true;
    pdr->base_unit = 2; // Degress c
    pdr->uint_modifier = 0;
    pdr->rate_unit = 0;
    pdr->aux_unit = 0;
    pdr->is_linear = true;
    pdr->sensor_data_size = PLDM_SENSOR_DATA_SIZE_UINT8;
    pdr->resolution = 1; // m
    pdr->offset = 0; // B
    pdr->accuracy = 100; // +- 1.00%
    pdr->plus_tolerance = 0;
    pdr->minus_tolerance = 0;
    pdr->hysteresis = 0; // not use hysteresis
    pdr->supported_thresholds = 0x0;
    pdr->threshold_and_hysteresis_volatility = 0x1f;
    pdr->state_transition_interval = 5;
    pdr->update_interval = 50;
    pdr->max_readable = 255;
    pdr->min_readable = 0;
    pdr->range_field_format = PLDM_SENSOR_DATA_SIZE_UINT8;
    pdr->range_field_support = 0x78;
    pdr->nominal_value = 0;
    pdr->normal_max = 0;
    pdr->normal_min = 0;
    pdr->warning_high = 90;
    pdr->warning_low = 5;
    pdr->critical_high = 100;
    pdr->critical_low = 0;
    pdr->fatal_high = 0;
    pdr->fatal_low = 0;
}

void init_pldm_sensor_auxiliary_names_pdr(
    pldm_sensor_auxiliary_names_pdr_t **aux_pdr,
    uint32_t *pdr_size)
{
    pldm_sensor_auxiliary_names_pdr_t *pdr;
    unsigned char tag[3] = { 'e', 'n', 0x0 }; // en
    unsigned char aux_name[18] = {
        0x0, 'C', 0x0, 'o', 0x0, 'r', 0x0, 'e', 0x0,
        'T', 0x0, 'e', 0x0, 'm', 0x0, 'p', 0x0, 0x0
    }; // CoreTemp
    int idx = 0;

    pdr = (pldm_sensor_auxiliary_names_pdr_t *)fwk_mm_calloc(
        1,
        sizeof(pldm_sensor_auxiliary_names_pdr_t) + sizeof(tag) +
            sizeof(aux_name));

    pdr->hdr.record_handle = 0x02;
    pdr->hdr.version = 0x01;
    pdr->hdr.type = PLDM_SENSOR_AUXILIARY_NAMES_PDR;
    pdr->hdr.record_change_num = 0x0;
    pdr->hdr.length = sizeof(pldm_sensor_auxiliary_names_pdr_t) + sizeof(tag) +
        sizeof(aux_name) - sizeof(struct pldm_pdr_hdr);

    pdr->terminus_handle = 0x01;
    pdr->sensor_id = 0x1;
    pdr->sensor_count = 0x1;
    pdr->names[0] = 0x1;

    idx = 1;
    memcpy(&pdr->names[idx], tag, sizeof(tag));

    idx += sizeof(tag);
    memcpy(&pdr->names[idx], aux_name, sizeof(aux_name));

    *aux_pdr = pdr;
    *pdr_size = sizeof(pldm_sensor_auxiliary_names_pdr_t) + sizeof(tag) +
        sizeof(aux_name);
}

void init_pldm_entity_auxiliary_names_pdr(
    pldm_entity_auxiliary_names_pdr_t **aux_pdr,
    uint32_t *pdr_size)
{
    pldm_entity_auxiliary_names_pdr_t *pdr;
    unsigned char tag[3] = { 'e', 'n', 0x0 }; // en
    unsigned char aux_name[12] = { 0x0, 'S', 0x0, 'a', 0x0, 't',
                                   0x0, 'M', 0x0, 'C', 0x0, 0x0 }; // SatMC
    int idx = 0;

    pdr = (pldm_entity_auxiliary_names_pdr_t *)fwk_mm_calloc(
        1,
        sizeof(pldm_entity_auxiliary_names_pdr_t) + sizeof(tag) +
            sizeof(aux_name));

    pdr->hdr.record_handle = 0x02;
    pdr->hdr.version = 0x01;
    pdr->hdr.type = PLDM_ENTITY_AUXILIARY_NAMES_PDR;
    pdr->hdr.record_change_num = 0x0;
    pdr->hdr.length = sizeof(pldm_entity_auxiliary_names_pdr_t) + sizeof(tag) +
        sizeof(aux_name) - sizeof(struct pldm_pdr_hdr);

    pdr->entity_type = 36; // Management Controller Firmware
    pdr->entity_instance_number = 1;
    pdr->entity_container_id = 0;
    pdr->shared_name_count = 0;
    pdr->names[0] = 1;

    idx = 1;
    memcpy(&pdr->names[idx], tag, sizeof(tag));

    idx += sizeof(tag);
    memcpy(&pdr->names[idx], aux_name, sizeof(aux_name));

    *aux_pdr = pdr;
    *pdr_size = sizeof(pldm_entity_auxiliary_names_pdr_t) + sizeof(tag) +
        sizeof(aux_name);
}

void mcp_setup_pdr(pldm_pdr_api_t *api)
{
    uint32_t record_handle = 1;
    uint32_t pdr_size = 0;
    pldm_pdr_t *repo;
    pldm_state_sensor_pdr_t pldm_state_sensor_pdr;
    pldm_numeric_sensor_pdr_t pldm_numeric_sensor_pdr;
    pldm_sensor_auxiliary_names_pdr_t *sensor_aux_name_pdr;
    pldm_entity_auxiliary_names_pdr_t *entity_aux_name_pdr;

    /* create master repository which would hold all pdrs */
    repo = api->pldm_pdr_init();
    pldm_fw_mcp_ctx.pdr_info.pdr_owner = 1;
    pldm_fw_mcp_ctx.pdr_info.repo = repo;

    /* add the first pdr object to repo */
    // Numeric Sensor PDR
    init_pldm_numeric_sensor_pdr(&pldm_numeric_sensor_pdr);
    api->pldm_pdr_add(
        repo,
        (uint8_t *)&pldm_numeric_sensor_pdr,
        sizeof(pldm_numeric_sensor_pdr_t),
        false,
        1,
        &record_handle);

    // State Sensor PDR
    init_pldm_state_sensor_pdr(&pldm_state_sensor_pdr);
    api->pldm_pdr_add(
        repo,
        (uint8_t *)&pldm_state_sensor_pdr,
        sizeof(pldm_state_sensor_pdr_t),
        false,
        1,
        NULL);

    // Sensor Auxiliary Name PDR
    init_pldm_sensor_auxiliary_names_pdr(&sensor_aux_name_pdr, &pdr_size);
    api->pldm_pdr_add(
        repo, (uint8_t *)sensor_aux_name_pdr, pdr_size, false, 1, NULL);
    fwk_mm_free(sensor_aux_name_pdr);

    // Entity Auxiliary Name PDR
    init_pldm_entity_auxiliary_names_pdr(&entity_aux_name_pdr, &pdr_size);
    api->pldm_pdr_add(
        repo, (uint8_t *)entity_aux_name_pdr, pdr_size, false, 1, NULL);
    fwk_mm_free(entity_aux_name_pdr);
}

/*
 * pldm_fw would call into mcp_init to pass control to mcp routines to have bmc
 * initialiazed.
 */
void mcp_init(pldm_pdr_api_t *api)
{
    /* setup context */
    mcp_set_ctx(&pldm_fw_mcp_ctx);
    /* setup pdr */
    mcp_setup_pdr(api);

    /* setup platform event message */
#if 1
    event_queue_put(
        (uint8_t *)&numeric_sensor_event_data1,
        sizeof(numeric_sensor_event_data1),
        PLDM_SENSOR_EVENT,
        0x12345678);
    event_queue_put(
        (uint8_t *)&state_sensor_event_data1,
        sizeof(state_sensor_event_data1),
        PLDM_SENSOR_EVENT,
        0x12345678);
    event_queue_put(
        (uint8_t *)&cper_event_data1,
        sizeof(cper_event_data1),
        PLDM_CPER_EVENT,
        0x38360c04);
#endif

    FWK_LOG_INFO(MOD_NAME "mcp context:");
    pldm_fw_terminus_info(&pldm_fw_mcp_ctx);
}
