/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_PLDM_H
#define MOD_PLDM_H

#include <libpldm/base.h>
#include <libpldm/fru.h>
#include <libpldm/pdr.h>
#include <libpldm/platform.h>
#include <libpldm/state_set.h>

#include <endian.h>

#define MCTP_MSG_TYPE_PLDM 1

#define PLDM_SET_TID            1
#define PLDM_GET_EVENT_RECEIVER 0x05

/* common pldm mcros */
#define PLDM_SET_TID_RESP_BYTES 1

/* PLDM Event */
#define PLDM_CPER_EVENT 0x07

#define PLDM_PLATFORM_UNSUPPORTED_EVENT_FORMAT_VERSION 0x81
#define PLDM_PLATFORM_EVENT_ID_NOT_VALID               0x82

enum pldm_poll_for_platform_event_message_transfer_flag {
    PLDM_EVENT_START = 0x00,
    PLDM_EVENT_MIDDLE = 0x01,
    PLDM_EVENT_END = 0x04,
    PLDM_EVENT_START_AND_END = 0x05
};

typedef struct pldm_msg pldm_msg_t;
typedef struct pldm_msg_hdr pldm_msg_hdr_t;
typedef struct pldm_header_info pldm_header_info_t;

typedef struct pldm_pdr pldm_pdr_t;
typedef struct pldm_pdr_record pldm_pdr_record_t;

typedef struct pldm_get_tid_resp pldm_get_tid_resp_t;
typedef struct pldm_get_types_resp pldm_get_types_resp_t;
typedef struct pldm_get_version_resp pldm_get_version_resp_t;
typedef struct pldm_get_commands_resp pldm_get_commands_resp_t;
typedef struct pldm_get_pdr_resp pldm_get_pdr_resp_t;
typedef struct pldm_get_sensor_reading_resp pldm_get_sensor_reading_resp_t;
typedef struct pldm_get_state_sensor_readings_resp
    pldm_get_state_sensor_readings_resp_t;

typedef struct pldm_get_version_req pldm_get_version_req_t;
typedef struct pldm_get_commands_req pldm_get_commands_req_t;
typedef struct pldm_platform_event_message_req
    pldm_platform_event_message_req_t;
typedef struct pldm_platform_event_message_resp
    pldm_platform_event_message_resp_t;

typedef struct pldm_event_message_buffer_size_resp
    pldm_event_message_buffer_size_resp_t;

typedef struct pldm_get_fru_record_table_req pldm_get_fru_record_table_req_t;
typedef struct pldm_get_fru_record_table_resp pldm_get_fru_record_table_resp_t;

/** @struct pldm_compact_numeric_sensor_pdr
 *
 *  Structure representing PLDM compact numeric sensor PDR
 */
struct pldm_numeric_sensor_pdr {
    struct pldm_pdr_hdr hdr;
    uint16_t terminus_handle;
    uint16_t sensor_id;
    uint16_t entity_type;
    uint16_t entity_instance;
    uint16_t container_id;
    uint8_t sensor_init;
    bool8_t sensor_auxiliary_names_pdr;
    uint8_t base_unit;
    int8_t uint_modifier;
    uint8_t rate_unit;
    uint8_t base_oem_unit_handle;
    uint8_t aux_unit;
    uint8_t aux_unit_modifier;
    uint8_t aux_rate_unit;
    uint8_t rel;
    uint8_t aux_oem_uint_handle;
    bool8_t is_linear;
    uint8_t sensor_data_size;
    real32_t resolution;
    real32_t offset;
    uint16_t accuracy;
    uint8_t plus_tolerance;
    uint8_t minus_tolerance;
    uint8_t hysteresis; //
    uint8_t supported_thresholds;
    uint8_t threshold_and_hysteresis_volatility;
    real32_t state_transition_interval;
    real32_t update_interval;
    uint8_t max_readable; //
    uint8_t min_readable; //
    uint8_t range_field_format;
    uint8_t range_field_support;
    uint8_t nominal_value; //
    uint8_t normal_max; //
    uint8_t normal_min; //
    uint8_t warning_high; //
    uint8_t warning_low; //
    uint8_t critical_high; //
    uint8_t critical_low; //
    uint8_t fatal_high; //
    uint8_t fatal_low; //
} __attribute__((packed));

struct pldm_entity_auxiliary_names_pdr_ {
    struct pldm_pdr_hdr hdr;
    uint16_t entity_type;
    uint16_t entity_instance_number;
    uint16_t entity_container_id;
    uint8_t shared_name_count;
    uint8_t names[1];
} __attribute__((packed));

struct pldm_message_poll_event_data {
    uint8_t format_version;
    uint16_t event_id;
    uint32_t data_transfer_handle;
} __attribute__((packed));

/** @struct pldm_get_event_receiver_resq
 *
 * Structure representing GetEventReceiver response.
 * This structure applies only for MCTP as a transport type.
 */
struct pldm_get_event_receiver_resp {
    uint8_t completion_code;
    uint8_t transport_protocol_type;
    uint8_t event_receiver_address;
} __attribute__((packed));

typedef struct pldm_numeric_sensor_pdr pldm_numeric_sensor_pdr_t;
typedef struct pldm_state_sensor_pdr pldm_state_sensor_pdr_t;
typedef struct pldm_sensor_auxiliary_names_pdr
    pldm_sensor_auxiliary_names_pdr_t;
typedef struct pldm_entity_auxiliary_names_pdr_
    pldm_entity_auxiliary_names_pdr_t;
typedef struct pldm_message_poll_event_data pldm_message_poll_event_data_t;

/* index for bind requests supported by pldm_base */
enum mod_pldm_api_idx {
    PLDM_BASE_BIND_REQ_API_IDX,
    PLDM_PLATFORM_BIND_REQ_API_IDX,
    PLDM_PDR_BIND_REQ_API_IDX,
    PLDM_FRU_BIND_REQ_API_IDX,
    PLDM_UTILS_BIND_REQ_API_IDX,
    PLDM_API_BIND_REQ_API_IDX_COUNT,
};

/*
 * pldm_base_api_t holding function pointers to various apis shared by this
 * module
 */
typedef struct pldm_base_api {
    int (*encode_cc_only_resp)(
        uint8_t instance_id,
        uint8_t type,
        uint8_t command,
        uint8_t cc,
        pldm_msg_t *msg);

    int (*decode_get_tid_resp)(
        const pldm_msg_t *msg,
        size_t payload_length,
        uint8_t *completion_code,
        uint8_t *tid);

    int (*encode_get_tid_resp)(
        uint8_t instance_id,
        uint8_t completion_code,
        uint8_t tid,
        pldm_msg_t *msg);

    int (*encode_get_tid_req)(uint8_t instance_id, pldm_msg_t *msg);

    int (*decode_get_version_resp)(
        const pldm_msg_t *msg,
        size_t payload_length,
        uint8_t *completion_code,
        uint32_t *next_transfer_handle,
        uint8_t *transfer_flag,
        ver32_t *version);

    int (*decode_get_version_req)(
        const pldm_msg_t *msg,
        size_t payload_length,
        uint32_t *transfer_handle,
        uint8_t *transfer_opflag,
        uint8_t *type);

    int (*encode_get_version_resp)(
        uint8_t instance_id,
        uint8_t completion_code,
        uint32_t next_transfer_handle,
        uint8_t transfer_flag,
        const ver32_t *version_data,
        size_t version_size,
        pldm_msg_t *msg);

    int (*encode_get_version_req)(
        uint8_t instance_id,
        uint32_t transfer_handle,
        uint8_t transfer_opflag,
        uint8_t type,
        pldm_msg_t *msg);

    int (*decode_get_commands_resp)(
        const pldm_msg_t *msg,
        size_t payload_length,
        uint8_t *completion_code,
        bitfield8_t *commands);

    int (*decode_get_types_resp)(
        const pldm_msg_t *msg,
        size_t payload_length,
        uint8_t *completion_code,
        bitfield8_t *types);

    int (*encode_get_commands_resp)(
        uint8_t instance_id,
        uint8_t completion_code,
        const bitfield8_t *commands,
        pldm_msg_t *msg);

    int (*decode_get_commands_req)(
        const pldm_msg_t *msg,
        size_t payload_length,
        uint8_t *type,
        ver32_t *version);

    int (*encode_get_types_resp)(
        uint8_t instance_id,
        uint8_t completion_code,
        const bitfield8_t *types,
        pldm_msg_t *msg);

    int (*encode_get_commands_req)(
        uint8_t instance_id,
        uint8_t type,
        ver32_t version,
        pldm_msg_t *msg);

    int (*encode_get_types_req)(uint8_t instance_id, pldm_msg_t *msg);

} pldm_base_api_t;

/* type for exposing mctp_fw apis to be used by other modules */
typedef struct pldm_platform_api {
    int (*decode_set_event_receiver_req)(
        const pldm_msg_t *msg,
        size_t payload_length,
        uint8_t *event_message_global_enable,
        uint8_t *transport_protocol_type,
        uint8_t *event_receiver_address_info,
        uint16_t *heartbeat_timer);

    int (*decode_set_event_receiver_resp)(
        const pldm_msg_t *msg,
        size_t payload_length,
        uint8_t *completion_code);

    int (*encode_set_event_receiver_resp)(
        uint8_t instance_id,
        uint8_t completion_code,
        pldm_msg_t *msg);

    int (*encode_set_event_receiver_req)(
        uint8_t instance_id,
        uint8_t event_message_global_enable,
        uint8_t transport_protocol_type,
        uint8_t event_receiver_address_info,
        uint16_t heartbeat_timer,
        struct pldm_msg *msg);

    int (*decode_platform_event_message_resp)(
        const pldm_msg_t *msg,
        size_t payload_length,
        uint8_t *completion_code,
        uint8_t *platform_event_status);

    int (*encode_platform_event_message_req)(
        uint8_t instance_id,
        uint8_t format_version,
        uint8_t tid,
        uint8_t event_class,
        const uint8_t *event_data,
        size_t event_data_length,
        pldm_msg_t *msg,
        size_t payload_length);

    int (*encode_platform_event_message_resp)(
        uint8_t instance_id,
        uint8_t completion_code,
        uint8_t platform_event_status,
        pldm_msg_t *msg);

    int (*decode_platform_event_message_req)(
        const pldm_msg_t *msg,
        size_t payload_length,
        uint8_t *format_version,
        uint8_t *tid,
        uint8_t *event_class,
        size_t *event_data_offset);

    int (*decode_get_pdr_req)(
        const pldm_msg_t *msg,
        size_t payload_length,
        uint32_t *record_hndl,
        uint32_t *data_transfer_hndl,
        uint8_t *transfer_op_flag,
        uint16_t *request_cnt,
        uint16_t *record_chg_num);

    int (*encode_get_pdr_resp)(
        uint8_t instance_id,
        uint8_t completion_code,
        uint32_t next_record_hndl,
        uint32_t next_data_transfer_hndl,
        uint8_t transfer_flag,
        uint16_t resp_cnt,
        const uint8_t *record_data,
        uint8_t transfer_crc,
        pldm_msg_t *msg);

    int (*encode_get_pdr_req)(
        uint8_t instance_id,
        uint32_t record_hndl,
        uint32_t data_transfer_hndl,
        uint8_t transfer_op_flag,
        uint16_t request_cnt,
        uint16_t record_chg_num,
        pldm_msg_t *msg,
        size_t payload_length);

    int (*decode_get_pdr_resp)(
        const pldm_msg_t *msg,
        size_t payload_length,
        uint8_t *completion_code,
        uint32_t *next_record_hndl,
        uint32_t *next_data_transfer_hndl,
        uint8_t *transfer_flag,
        uint16_t *resp_cnt,
        uint8_t *record_data,
        size_t record_data_length,
        uint8_t *transfer_crc);

    int (*decode_get_sensor_reading_req)(
        const struct pldm_msg *msg,
        size_t payload_length,
        uint16_t *sensor_id,
        uint8_t *rearm_event_state);

    int (*encode_get_sensor_reading_resp)(
        uint8_t instance_id,
        uint8_t completion_code,
        uint8_t sensor_data_size,
        uint8_t sensor_operational_state,
        uint8_t sensor_event_message_enable,
        uint8_t present_state,
        uint8_t previous_state,
        uint8_t event_state,
        const uint8_t *present_reading,
        struct pldm_msg *msg,
        size_t payload_length);

    int (*decode_get_state_sensor_readings_req)(
        const struct pldm_msg *msg,
        size_t payload_length,
        uint16_t *sensor_id,
        bitfield8_t *sensor_rearm,
        uint8_t *reserved);

    int (*encode_get_state_sensor_readings_resp)(
        uint8_t instance_id,
        uint8_t completion_code,
        uint8_t comp_sensor_count,
        get_sensor_state_field *field,
        struct pldm_msg *msg);

    int (*decode_poll_for_platform_event_message_req)(
        const struct pldm_msg *msg,
        size_t payload_length,
        uint8_t *format_version,
        uint8_t *transfer_operation_flag,
        uint32_t *data_transfer_handle,
        uint16_t *event_id_to_acknowledge);

    int (*encode_poll_for_platform_event_message_resp)(
        uint8_t instance_id,
        uint8_t completion_code,
        uint8_t tid,
        uint16_t event_id,
        uint32_t next_data_transfer_handle,
        uint8_t transfer_flag,
        uint8_t event_class,
        uint32_t event_data_size,
        uint8_t *event_data,
        uint32_t checksum,
        struct pldm_msg *msg,
        size_t payload_length);

    int (*encode_get_event_receiver_resp)(
        uint8_t instance_id,
        uint8_t completion_code,
        uint8_t transport_protocol_type,
        uint8_t event_receiver_address_info,
        struct pldm_msg *msg);

    int (*decode_event_message_buffer_size_req)(
        const struct pldm_msg *msg,
        size_t payload_length,
        uint16_t *event_receiver_max_buffer_size);

    int (*encode_event_message_buffer_size_resp)(
        uint8_t instance_id,
        uint8_t completion_code,
        uint16_t terminus_max_buffer_size,
        struct pldm_msg *msg);

} pldm_platform_api_t;

/* type for exposing pldm_pdr apis to be used by other modules */
typedef struct pldm_pdr_api {
    pldm_pdr_t *(*pldm_pdr_init)();
    int (*pldm_pdr_add)(
        pldm_pdr_t *repo,
        const uint8_t *data,
        uint32_t size,
        bool is_remote,
        uint16_t terminus_handle,
        uint32_t *record_handle);

    void (*pldm_pdr_destroy)(pldm_pdr_t *repo);

    const pldm_pdr_record_t *(*pldm_pdr_find_record)(
        const pldm_pdr_t *repo,
        uint32_t record_handle,
        uint8_t **data,
        uint32_t *size,
        uint32_t *next_record_handle);

    const pldm_pdr_record_t *(*pldm_pdr_get_next_record)(
        const pldm_pdr_t *repo,
        const pldm_pdr_record_t *curr_record,
        uint8_t **data,
        uint32_t *size,
        uint32_t *next_record_handle);

    const pldm_pdr_record_t *(*pldm_pdr_find_record_by_type)(
        const pldm_pdr_t *repo,
        uint8_t pdr_type,
        const pldm_pdr_record_t *curr_record,
        uint8_t **data,
        uint32_t *size);

    uint32_t (*pldm_pdr_get_record_count)(const pldm_pdr_t *repo);
    uint32_t (*pldm_pdr_get_repo_size)(const pldm_pdr_t *repo);
    uint32_t (*pldm_pdr_get_record_handle)(
        const pldm_pdr_t *repo,
        const pldm_pdr_record_t *record);

    const pldm_pdr_record_t *(*pldm_pdr_fru_record_set_find_by_rsi)(
        const pldm_pdr_t *repo,
        uint16_t fru_rsi,
        uint16_t *terminus_handle,
        uint16_t *entity_type,
        uint16_t *entity_instance_num,
        uint16_t *container_id);

} pldm_pdr_api_t;

/* type for exposing pldm_fru apis to be used by other modules */
typedef struct pldm_fru_api {
    int (*decode_get_fru_record_table_req)(
        const struct pldm_msg *msg,
        size_t payload_length,
        uint32_t *data_transfer_handle,
        uint8_t *transfer_operation_flag);

    int (*encode_get_fru_record_table_resp)(
        uint8_t instance_id,
        uint8_t completion_code,
        uint32_t next_data_transfer_handle,
        uint8_t transfer_flag,
        struct pldm_msg *msg);

} pldm_fru_api_t;

/* type for exposing pldm_util apis to be used by other modules */
typedef struct pldm_utils_api {
    uint32_t (*crc32)(const void *data, size_t size);

    uint8_t (*crc8)(const void *data, size_t size);
} pldm_utils_api_t;
#endif /* MOD_PLDM_H */
