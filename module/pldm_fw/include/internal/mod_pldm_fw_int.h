/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_PLDM_FW_INT_H
#define MOD_PLDM_FW_INT_H

#include <internal/Cper.h>
#include <internal/UefiBaseTypes.h>

#include <mod_pldm.h>
#include <mod_pldm_fw.h>

#include <fwk_assert.h>

/*
 * pldm_fw uses quite a bit of memory allocations throughout its lifecycle. This
 * macro helps in checking for allocation failures.
 */
#define MM_ASSERT(x) assert(NULL != x)

#define PLDM_MAX_PDR 5U

#define EVENT_MAX_SIZE        256
#define EVENT_QUEUE_SIZE      5
#define EVENT_MAX_BUFFER_SIZE 32

/* Platform Event Message Data */
struct event {
    uint32_t size;
    uint16_t id;
    uint8_t class;
    uint32_t checksum;
    uint8_t data[EVENT_MAX_SIZE];
};

struct pldm_event_queue {
    int head;
    int tail;
    int length;
    struct event events[EVENT_QUEUE_SIZE];
};

struct _pldm_cper_event_data_firmware_error_record_reference {
    uint8_t format_version;
    uint8_t format_type;
    uint16_t event_data_length;
    EFI_ERROR_SECTION_DESCRIPTOR section_descriptor;
    EFI_FIRMWARE_ERROR_DATA section_data;
} __attribute__((packed));

struct _pldm_event_data_numeric_sensor_state_uint8 {
    uint16_t sensor_id;
    uint8_t sensor_event_class;
    uint8_t event_state;
    uint8_t previous_event_state;
    uint8_t sensor_data_size;
    uint8_t presentReading;
} __attribute__((packed));

struct _pldm_event_data_state_sensor_state {
    uint16_t sensor_id;
    uint8_t sensor_event_class;
    uint8_t sensor_offset;
    uint8_t event_state;
    uint8_t previous_event_state;
} __attribute__((packed));

typedef struct _pldm_cper_event_data_firmware_error_record_reference
    pldm_cper_event_data_firmware_error_record_reference;

typedef struct _pldm_event_data_numeric_sensor_state_uint8
    pldm_event_data_numeric_sensor_state_uint8;

typedef struct _pldm_event_data_state_sensor_state
    pldm_event_data_state_sensor_state;

/* each pdr would be represented by a pointer to hold the base address and a
 * size_t object to hold the size of that pdr */
typedef struct pdr {
    uint8_t *p;
    size_t sz;
} pdr_t;

/* representation of a pldm pdr object */
typedef struct pdr_info {
    /* if the owner this object is a pdr owner, set this flag to true */
    bool pdr_owner;
    /* if pdr owner, initialize repo with proper pdr repo address. If the holder
     * of this object is not the pdr owner, this pointer holds garbage and
     * should not be accessed. For safety it is recommended that the owner of
     * such an object should set this pointer to zero */
    pldm_pdr_t *repo;
    /* index to keep track of pdr updates */
    uint8_t idx;
    /* an array of pdr elements pointers. For simplicity, the maximum pdrs have
     * been limited to PLDM_MAX_PDR */
    pdr_t pdr[PLDM_MAX_PDR];
    /**/
} pdr_info_t;

/*
 * each pldm type can support multiple versions. Each version can again support
 * n number of commands. This type is held as an aggregate within pldm_info
 * struct to store the aforementioned information.
 */
typedef struct version {
    uint8_t idx;
    uint8_t commands_count;
    uint8_t *commands;
    ver32_t version;
} version_t;

/*
 * pldm info type holds a pointer to version type. It is a map of one pldm type
 * obejct (represented by type field) to n number of version object.
 */
typedef struct pldm_info {
    uint8_t type;
    uint8_t idx;
    uint8_t version_count;
    version_t *version;
} pldm_info_t;

/*
 * terminals which have the capability to send events can be configured with the
 * address and other information of terminals that would require those events.
 * In pldm specs these receivers are called event receiver. ctx for terminals in
 * case of this framework holds an object of the below type.
 */
typedef struct receiver_ev_state {
    uint8_t global_enable;
    uint8_t receiver_addr;
    uint8_t heartbeat_timer;
    uint8_t transport_protocol_type;
} receiver_ev_state_t;

/*
 * Any pldm terminus which supports pldm_base type should use the following type
 * to define basic pldm_base params supported by that terminus.
 */
typedef struct pldm_fw_terminus_ctx {
    uint8_t tid;
    uint8_t types_count;
    uint8_t idx;
    pldm_info_t *pldm_info;
    receiver_ev_state_t receiver_ev_state;
    pdr_info_t pdr_info;

    struct pldm_event_queue event_queue;
    uint16_t event_terminus_max_buffer_size;
    uint16_t event_receiver_max_buffer_size;
} pldm_fw_terminus_ctx_t;

/* different handshakes/sequences supported by the firmware */
typedef enum {
    PLDM_FW_DISCOVERY,
} pldm_fw_state_t;

/*
 * pldm_fw would call into bmc_init to pass control to bmc routines to have bmc
 * initialiazed.
 */
void bmc_init();

/*
 * pldm_fw would call into mcp_init to pass control to mcp routines to have bmc
 * initialiazed.
 */
void mcp_init(pldm_pdr_api_t *api);

/*
 * Implements discovery state machine. Any terminus which needs to do a
 * discovery on one of the other terminus should invoke this function,
 */
void pldm_fw_discovery(pldm_fw_terminus_ctx_t *ctx);

/* Prints the ctx object onto the terminal */
void pldm_fw_terminus_info(pldm_fw_terminus_ctx_t *ctx);

int event_queue_put(
    uint8_t *event_data,
    uint32_t event_size,
    uint8_t event_class,
    uint32_t checksum);

int event_queue_get(
    uint8_t *event_data,
    uint32_t *event_size,
    uint16_t *event_id,
    uint8_t *event_class,
    uint32_t *checksum);

int get_fru_record_table_size();
const uint8_t *get_fru_record_table_data();

#endif
