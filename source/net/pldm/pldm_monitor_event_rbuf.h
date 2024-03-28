#ifndef __PLDM_MONITOR_EVENT_RBUF_H__
#define __PLDM_MONITOR_EVENT_RBUF_H__

#include "main.h"

#define ALIGN_LEN(type, len)                      (((len) + (type) - 1) / (type))

typedef enum {
    SEVERITY_OK = 0,
    SEVERITY_WARNING,
    SEVERITY_CRITICAL
} pldm_msg_event_severity_t;

#pragma pack(1)

typedef struct {
    u16 event_id;
    u32 event_data_size;            /* not include event_id, event_data_size, event_class len */
    u8  event_class;
    u8  data[0];
} pldm_event_data_t;

/* Event msg */

typedef struct {
    u8 sensor_offset;
    u8 event_state;
    u8 previous_event_state;
} pldm_field_per_state_sensor_state_format_t;

typedef struct {
    u8 event_state;
    u8 previous_event_state;
    u8 sensor_datasize;
    u8 present_reading;
} pldm_field_per_numeric_sensor_state_format_t;

typedef struct {
    u8 present_op_state;
    u8 previous_op_state;
} pldm_field_per_sensor_op_state_format_t;

typedef struct {
    u16 sensor_id;
    u8 sensor_event_class;
    u8 field_per_event_class[0];
} pldm_sensor_event_class_event_data_format_t;

typedef struct {
    u32 resource_id;
    u16 op_id;
} pldm_redfish_task_executed_event_data_format_t;

typedef struct {
    u32 resource_id;
    u8 event_severity;
} pldm_redfish_id_and_severity_t;

typedef struct {
    u8 event_cnt;
    u16 event_data_len;
    pldm_redfish_id_and_severity_t id_and_severity[0];
    /* bejEncoding event_data; */
} pldm_redfish_msg_event_data_format_t;

typedef struct {
    u8 event_data_op;
    u8 num_of_chg_entries;
    u32 chg_entry[1];
} pldm_chg_record_format_t;

typedef struct {
    u8 event_data_format;
    u8 num_of_chg_records;
    pldm_chg_record_format_t chg_record[0];
} pldm_pdr_repo_chg_event_data_format_t;

typedef struct {
    u8 format_ver;
    u16 event_id;
    u32 data_transfer_handle;
} pldm_pldm_msg_poll_event_data_format_t;

#pragma pack()

void *pldm_event_rbuf_init(void);
int pldm_event_rbuf_is_empty(void *p);
int pldm_event_rbuf_try_read(void *p, void *dat, int len, int offset);
int pldm_event_rbuf_read_done(void *p);
int pldm_event_rbuf_write(void *p, void *dat, int len);

void pldm_pdr_chg_event_generate(void *p, u8 event_data_format, u8 event_data_op, u32 record_handle);
void pldm_sensor_event_generate(void *p, u8 sensor_event_class, u8 event_msg_en, void *data_struct);

void pldm_redfish_task_execute_event_generate(void *p, pldm_redfish_task_executed_event_data_format_t *op_indentify);
void pldm_redfish_msg_event_generate(void *p, u8 link_state);

#endif /* __PLDM_MONITOR_EVENT_RBUF_H__ */
