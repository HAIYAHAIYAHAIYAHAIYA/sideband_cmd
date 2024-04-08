#include "pldm_monitor_event_rbuf.h"
#include "pldm_monitor.h"
#include "pldm_redfish.h"
#include "pldm_cjson.h"
#include "pldm_bej_resolve.h"
#include "pdr.h"

u16 g_event_id = 1;
extern pldm_monitor_base_info_t g_pldm_monitor_info;
extern u8 g_anno_dict[PLDM_REDFISH_ANNO_DICT_LEN];
extern u8 g_dict_info[PLDM_REDFISH_DICT_INFO_LEN];
typedef struct {
    u8 buf[PLDM_TERMINUS_MAX_BUFFERSIZE];
    u16 rd;
    u16 wt;
    u16 try_rd;
} pldm_event_buf_t;


static pldm_event_buf_t gs_pldm_event_rbuf;

void *pldm_event_rbuf_init(void)
{
    g_event_id = 1;
    gs_pldm_event_rbuf.rd = 0;
    gs_pldm_event_rbuf.wt = 0;
    gs_pldm_event_rbuf.try_rd = 0;
    return &gs_pldm_event_rbuf;
}

int pldm_event_rbuf_read_size(void *p)
{
    pldm_event_buf_t *pt = (pldm_event_buf_t *)p;
    int rbuf_sz = (pt->wt >= pt->rd) ? (pt->wt - pt->rd) : (pt->wt + PLDM_TERMINUS_MAX_BUFFERSIZE - pt->rd);
    return rbuf_sz;
}

int pldm_event_rbuf_is_empty(void *p)
{
    pldm_event_buf_t *pt = (pldm_event_buf_t *)p;
    return pt->rd == pt->wt;
}

int pldm_event_rbuf_try_read(void *p, void *dat, int len, int offset)
{
    //CHECK(p != NULL, TRUE, return STS_ERR);
    pldm_event_buf_t *pt = (pldm_event_buf_t *)p;
    u32 size = PLDM_TERMINUS_MAX_BUFFERSIZE;
    pt->try_rd = (pt->rd + offset) % size;

    if ((pt->try_rd + len) > size) {
        u16 first = size - pt->try_rd;
        cm_memcpy(dat, pt->buf + pt->try_rd, first);
        cm_memcpy(&(((u8 *)dat)[first]), pt->buf, len - first);
    } else {
        cm_memcpy(dat, pt->buf + pt->try_rd, len); 
    }

    pt->try_rd = (pt->try_rd + len) % size;

    //return rbuf_complex_read(p, dat, r_len, FALSE);
    return 0;
}

int pldm_event_rbuf_read_done(void *p)
{
    //CHECK(p != NULL, TRUE, return STS_ERR);
    pldm_event_buf_t *pt = (pldm_event_buf_t *)p;
    pt->rd = pt->try_rd;
    //return STS_SUC;
    return 0;
}

int pldm_event_rbuf_write(void *p, void *dat, int len)
{
    //CHECK(p != NULL, TRUE, return STS_ERR);
    pldm_event_buf_t *pt = (pldm_event_buf_t *)p;
    u32 size = PLDM_TERMINUS_MAX_BUFFERSIZE;
    u32 type = sizeof(u8);
    int valid = size - pldm_event_rbuf_read_size(p) - 1;
    u16 w_len = ALIGN_LEN(type, len);

    while (valid < w_len) {                                   /* delete the oldest event */
        u8 event_data_info[sizeof(pldm_event_data_t)] = {0};
        pldm_event_data_t *pldm_event_data = (pldm_event_data_t *)event_data_info;
        u32 rd = pt->rd;
        pldm_event_rbuf_try_read(p, event_data_info, sizeof(pldm_event_data_t), 0);
        u32 event_data_size = ALIGN_LEN(type, pldm_event_data->event_data_size + sizeof(pldm_event_data_t));
        pt->rd = pt->try_rd = (event_data_size + rd) % size;
        valid = size - pldm_event_rbuf_read_size(p) - 1;
    }

    if (pt->wt + w_len > size) {
        u16 first = size - pt->wt;
        cm_memcpy(pt->buf + pt->wt, dat, first);
        cm_memcpy(pt->buf, &(((u8 *)dat)[first]), w_len - first);
    } else {
        cm_memcpy(pt->buf + pt->wt, dat, w_len);
    }

    pt->wt = ( pt->wt + w_len) % size;
    return 0;
}

void pldm_sensor_event_generate(void *p, u8 sensor_event_class, u8 event_msg_en, void *data_struct)
{
    if (!p || g_pldm_monitor_info.terminus_mode == PLDM_DISABLE) {
        goto L_RET;
    }

    if (event_msg_en == PLDM_EVENT_DIS_EN) {
        goto L_RET;
    }
    u8 event_buf[64] = {0};
    pldm_event_data_t *sensor_event = (pldm_event_data_t *)event_buf;
    pldm_sensor_event_class_event_data_format_t *sensor_event_dat = (pldm_sensor_event_class_event_data_format_t *)(sensor_event->data);

    sensor_event->event_id = g_event_id++;
    sensor_event->event_class = SENSOR_EVENT;

    sensor_event->event_data_size = sizeof(pldm_sensor_event_class_event_data_format_t);

    if (sensor_event_class == PLDM_SENSOR_OP_STATE) {                               /* change of the sensor's operational state */
        pldm_data_struct_t *op_state = (pldm_data_struct_t *)data_struct;
        sensor_event_dat->sensor_id = op_state->sensor_id;
        sensor_event_dat->sensor_event_class = PLDM_SENSOR_OP_STATE;

        pldm_field_per_sensor_op_state_format_t *op_state_field = (pldm_field_per_sensor_op_state_format_t *)(sensor_event_dat->field_per_event_class);
        op_state_field->present_op_state = op_state->op_state;
        op_state_field->previous_op_state = op_state->previous_op_state;

        sensor_event->event_data_size += sizeof(pldm_field_per_sensor_op_state_format_t);
    } else if (sensor_event_class == PLDM_STATE_SENSOR_STATE) {                     /* change in the present state */
        pldm_state_data_struct_t *sensor_state = (pldm_state_data_struct_t *)data_struct;
        sensor_event_dat->sensor_id = sensor_state->sensor_id;
        sensor_event_dat->sensor_event_class = PLDM_STATE_SENSOR_STATE;

        pldm_field_per_state_sensor_state_format_t *sensor_state_field = (pldm_field_per_state_sensor_state_format_t *)(sensor_event_dat->field_per_event_class);
        sensor_state_field->sensor_offset = sensor_state->sensor_offset;
        sensor_state_field->event_state = sensor_state->present_state;              /* same as presentState(E810) */
        sensor_state_field->previous_event_state = sensor_state->previous_state;    /* so */

        sensor_event->event_data_size += sizeof(pldm_field_per_state_sensor_state_format_t);
    } else if (sensor_event_class == PLDM_NUMERIC_SENSOR_STATE) {                   /* change in the present state */
        pldm_data_struct_t *sensor_numeric = (pldm_data_struct_t *)data_struct;
        sensor_event_dat->sensor_id = sensor_numeric->sensor_id;
        sensor_event_dat->sensor_event_class = PLDM_NUMERIC_SENSOR_STATE;

        pldm_field_per_numeric_sensor_state_format_t *sensor_numeric_field = (pldm_field_per_numeric_sensor_state_format_t *)(sensor_event_dat->field_per_event_class);
        sensor_numeric_field->sensor_datasize = sensor_numeric->sensor_data_size;
        sensor_numeric_field->event_state = sensor_numeric->present_state;              /* same as presentState(E810) */
        sensor_numeric_field->previous_event_state = sensor_numeric->previous_state;    /* so */
        sensor_numeric_field->present_reading = sensor_numeric->present_reading;

        sensor_event->event_data_size += sizeof(pldm_field_per_numeric_sensor_state_format_t);
    } else {
        LOG("error sensor event class : %d", sensor_event_class);
        goto L_RET;
    }
    pldm_event_rbuf_write(p, sensor_event, sizeof(pldm_event_data_t) + sensor_event->event_data_size);

L_RET:
    return;
}

void pldm_pdr_chg_event_generate(void *p, u8 event_data_format, u8 event_data_op, u32 record_handle)
{
    if (!p || g_pldm_monitor_info.terminus_mode == PLDM_DISABLE) {
        return;
    }

    u8 event_buf[64] = {0};
    pldm_event_data_t *pdr_chg_event = (pldm_event_data_t *)event_buf;
    pldm_pdr_repo_chg_event_data_format_t *pdr_chg_event_dat = (pldm_pdr_repo_chg_event_data_format_t *)(pdr_chg_event->data);

    pdr_chg_event->event_id = g_event_id++;
    pdr_chg_event->event_class = PLDM_PDR_REPO_CHG_EVENT;

    u8 num_of_chg_records = pdr_chg_event_dat->num_of_chg_records = 1;
    pdr_chg_event_dat->event_data_format = event_data_format;

    u8 num_of_chg_entries = pdr_chg_event_dat->chg_record[num_of_chg_records - 1].num_of_chg_entries = 1;
    pdr_chg_event_dat->chg_record[num_of_chg_records - 1].event_data_op = event_data_op;
    pdr_chg_event_dat->chg_record[num_of_chg_records - 1].chg_entry[num_of_chg_entries - 1] = record_handle;

    pdr_chg_event->event_data_size = sizeof(pldm_pdr_repo_chg_event_data_format_t) + num_of_chg_records * sizeof(pldm_chg_record_format_t);

    pldm_event_rbuf_write(p, pdr_chg_event, sizeof(pldm_event_data_t) + pdr_chg_event->event_data_size);
}

void pldm_redfish_task_execute_event_generate(void *p, pldm_redfish_task_executed_event_data_format_t *op_indentify)
{
    if (!p || g_pldm_monitor_info.terminus_mode == PLDM_DISABLE || !op_indentify) {
        return;
    }
    u8 event_buf[64] = {0};
    pldm_event_data_t *task_execute_event = (pldm_event_data_t *)event_buf;
    pldm_redfish_task_executed_event_data_format_t *task_execute_event_data = (pldm_redfish_task_executed_event_data_format_t *)(task_execute_event->data);

    task_execute_event->event_id = g_event_id++;
    task_execute_event->event_class = REDFISH_TASK_EXCUTE_EVENT;
    task_execute_event->event_data_size = sizeof(pldm_redfish_task_executed_event_data_format_t);

    task_execute_event_data->op_id = op_indentify->op_id;
    task_execute_event_data->resource_id = op_indentify->resource_id;

    pldm_event_rbuf_write(p, task_execute_event, sizeof(pldm_event_data_t) + task_execute_event->event_data_size);
}

void pldm_redfish_msg_event_generate(void *p, u8 link_state)
{
    if (!p || g_pldm_monitor_info.terminus_mode == PLDM_DISABLE) {
        return;
    }
    u8 event_dict[PLDM_REDFISH_EVENT_DICT_LEN];
    u8 event_buf[128] = {0};

    u8 ret = pldm_redfish_get_dict_data(PLDM_BASE_EVENT_DICT_RESOURCE_ID, SCHEMACLASS_EVENT, event_dict, PLDM_REDFISH_EVENT_DICT_LEN);
    if (ret == false) return;

    u8 *annc_dict = &g_anno_dict[DICT_FMT_HDR_LEN];
    pldm_event_data_t *msg_event = (pldm_event_data_t *)event_buf;
    pldm_redfish_msg_event_data_format_t *msg_event_data = (pldm_redfish_msg_event_data_format_t *)(msg_event->data);
    pldm_redfish_dictionary_format_t *event_dict_ptr = (pldm_redfish_dictionary_format_t *)&event_dict[DICT_FMT_HDR_LEN];

    msg_event->event_id = g_event_id++;
    msg_event->event_class = REDFISH_MSG_EVENT;

    msg_event_data->event_cnt = 1;
    msg_event_data->id_and_severity[0].resource_id = PLDM_BASE_EVENT_DICT_RESOURCE_ID;
    msg_event_data->id_and_severity[0].event_severity = SEVERITY_WARNING;
    bejencoding_t *ptr = (bejencoding_t *)&(msg_event_data->id_and_severity[1]);

    ptr->ver = event_dict_ptr->schema_version;
    ptr->schema_class = SCHEMACLASS_MAJOR;

    pldm_cjson_t *root = pldm_cjson_create_event_schema(PLDM_BASE_EVENT_DICT_RESOURCE_ID, (u8 *)event_dict_ptr, annc_dict, link_state);
    pldm_cjson_cal_len_to_root(root, OTHER_TYPE);
    u8 *end_ptr = pldm_bej_encode(root, ((u8 *)ptr + sizeof(bejencoding_t)));
    msg_event_data->event_data_len = end_ptr - (u8 *)ptr;
    pldm_cjson_pool_reinit();
    root = NULL;

    msg_event->event_data_size = msg_event_data->event_data_len + sizeof(pldm_redfish_msg_event_data_format_t) + sizeof(pldm_redfish_id_and_severity_t);
    pldm_event_rbuf_write(p, msg_event, sizeof(pldm_event_data_t) + msg_event->event_data_size);
}