#include "pldm_monitor_gen.h"
#include "pldm_control.h"
#include "pkt_gen.h"

pldm_gen_state_t gs_pldm_monitor_gen_state;

static void pldm_monitor_gen_cmd_01(u8 *buf)
{
    pldm_set_tid_req_dat_t *req_dat = (pldm_set_tid_req_dat_t *)buf;
    req_dat->tid = 0x67;
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_ENTER_CMD_01;
}

static void pldm_monitor_gen_cmd_02(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_ENTER_CMD_02;
}

static void pldm_monitor_gen_cmd_03(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_ENTER_CMD_03;
}

static void pldm_monitor_gen_cmd_04(u8 *buf)
{
    pldm_set_event_receiver_req_dat_t *req_dat = (pldm_set_event_receiver_req_dat_t *)buf;
    req_dat->event_msg_global_en = PLDM_ENABLE_ASYNC;
    req_dat->trans_protocol_type = 0x00;
    req_dat->event_receiver_addr_info = 0xcc;
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_ENTER_CMD_04;
}

static void pldm_monitor_gen_cmd_05(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_ENTER_CMD_05;
}

static void pldm_monitor_gen_cmd_0a(u8 *buf)
{
    pldm_gen_req_hdr_update(buf, 0x0a);
    pldm_platform_event_msg_receive_t *req_dat = (pldm_platform_event_msg_receive_t *)buf;
    req_dat->cpl_code = MCTP_COMMAND_SUCCESS;
}

static void pldm_monitor_gen_cmd_0b(u8 *buf)
{
    pldm_gen_req_hdr_update(buf, 0x0b);
    pldm_poll_for_platform_event_msg_req_dat_t *req_dat = (pldm_poll_for_platform_event_msg_req_dat_t *)buf;
    pldm_poll_for_platform_event_msg_rsp_dat_t recv_rsp_dat = pldm_monitor_get_pull_event_rsp_dat();
    req_dat->data_transfer_handle = recv_rsp_dat.next_data_transfer_handle;
    req_dat->format_ver = PLDM_EVENT_FORMAT_VERSION;
    req_dat->transfer_op_flag = !(recv_rsp_dat.next_data_transfer_handle) ? PLDM_TRANSFER_OP_FLAG_GET_FIRST_PART : PLDM_TRANSFER_OP_FLAG_GET_NEXT_PART;
    req_dat->event_id_to_ack = !(recv_rsp_dat.next_data_transfer_handle) ? recv_rsp_dat.event_id : 0xFFFF;
}

static void pldm_monitor_gen_cmd_0c(u8 *buf)
{
    pldm_event_msg_supported_req_dat_t *req_dat = (pldm_event_msg_supported_req_dat_t *)buf;
    req_dat->format_ver = PLDM_EVENT_FORMAT_VERSION;
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_ENTER_CMD_0C;
}

static void pldm_monitor_gen_cmd_0d(u8 *buf)
{
    pldm_event_msg_buffersize_req_dat_t *req_dat = (pldm_event_msg_buffersize_req_dat_t *)buf;
    req_dat->event_receiver_max_buffersize = 256;
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_ENTER_CMD_0D;
}

static void pldm_monitor_gen_cmd_10(u8 *buf)
{
    pldm_set_numeric_sensor_enable_req_dat_t *req_dat = (pldm_set_numeric_sensor_enable_req_dat_t *)buf;
    req_dat->sensor_id = PLDM_BASE_NIC_TEMP_SENSOR_ID;
    req_dat->sensor_op_state = PLDM_OP_ENABLE;
    req_dat->sensor_event_msg_en = PLDM_EVENT_EN;
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_ENTER_CMD_10;
}

static void pldm_monitor_gen_cmd_11(u8 *buf)
{
    pldm_get_sensor_reading_req_dat_t *req_dat = (pldm_get_sensor_reading_req_dat_t *)buf;
    req_dat->sensor_id = PLDM_BASE_NIC_TEMP_SENSOR_ID;
    req_dat->rearm_event_state = 0;
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_ENTER_CMD_11;
}

static void pldm_monitor_gen_cmd_12(u8 *buf)
{
    pldm_get_sensor_thresholds_req_dat_t *req_dat = (pldm_get_sensor_thresholds_req_dat_t *)buf;
    req_dat->sensor_id = PLDM_BASE_NIC_TEMP_SENSOR_ID;
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_ENTER_CMD_12;
}

static void pldm_monitor_gen_cmd_13(u8 *buf)
{
    pldm_set_sensor_thresholds_req_dat_t *req_dat = (pldm_set_sensor_thresholds_req_dat_t *)buf;
    req_dat->sensor_id = PLDM_BASE_NIC_TEMP_SENSOR_ID;
    req_dat->sensor_datasize = PLDM_DATASIZE_UINT8;
    req_dat->upper_threshold_warning = 20;
    req_dat->upper_threshold_critical = 50;
    req_dat->upper_threshold_fatal = 80;
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_ENTER_CMD_13;
}

static void pldm_monitor_gen_cmd_15(u8 *buf)
{
    pldm_get_sensor_hysteresis_req_dat_t *req_dat = (pldm_get_sensor_hysteresis_req_dat_t *)buf;
    req_dat->sensor_id = PLDM_BASE_NIC_TEMP_SENSOR_ID;
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_ENTER_CMD_15;
}

static void pldm_monitor_gen_cmd_20(u8 *buf)
{
    pldm_set_state_sensor_en_req_dat_t *req_dat = (pldm_set_state_sensor_en_req_dat_t *)buf;
    req_dat->sensor_id = PLDM_BASE_NIC_TEMP_SENSOR_ID;
    req_dat->composite_sensor_cnt = 1;
    req_dat->opfield[0].event_msg_en = PLDM_EVENT_EN;
    req_dat->opfield[0].sensor_op_state = PLDM_OP_ENABLE;
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_ENTER_CMD_20;
}

static void pldm_monitor_gen_cmd_21(u8 *buf)
{
    pldm_get_state_sensor_reading_req_dat_t *req_dat = (pldm_get_state_sensor_reading_req_dat_t *)buf;
    req_dat->sensor_id = PLDM_BASE_NIC_TEMP_SENSOR_ID;
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_ENTER_CMD_21;
}

static void pldm_monitor_gen_cmd_50(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_ENTER_CMD_50;
}

static void pldm_monitor_gen_cmd_51(u8 *buf)
{
    pldm_get_pdr_req_dat_t *req_dat = (pldm_get_pdr_req_dat_t *)buf;
    pldm_get_pdr_rsp_dat_t rsp_dat = pldm_monitor_get_pdr_rsp_dat();
    req_dat->data_transfer_handle = rsp_dat.next_data_transfer_handle;
    req_dat->record_handle = !(rsp_dat.next_record_handle) ? 1 : rsp_dat.next_record_handle;
    req_dat->transfer_op_state = rsp_dat.next_data_transfer_handle ? PLDM_TRANSFER_OP_FLAG_GET_NEXT_PART : PLDM_TRANSFER_OP_FLAG_GET_FIRST_PART;
    req_dat->req_cnt = PLDM_MONITOR_GET_PDR_REQ_CNT;
    req_dat->record_chg_num = 0;
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_GET_PDR;
}

static void pldm_monitor_gen_cmd_53(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_ENTER_CMD_53;
}

static void pldm_monitor_gen_cmd_54(u8 *buf)
{
    pldm_event_send_handle();
}

void pldm_monitor_gen(int cmd, u8 *buf)
{
    gen_cmd cmds[] = {
        gen_cmd_unsupport,
        pldm_monitor_gen_cmd_01,
        pldm_monitor_gen_cmd_02,
        pldm_monitor_gen_cmd_03,
        pldm_monitor_gen_cmd_04,
        pldm_monitor_gen_cmd_05,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        pldm_monitor_gen_cmd_0a,
        pldm_monitor_gen_cmd_0b,
        pldm_monitor_gen_cmd_0c,
        pldm_monitor_gen_cmd_0d,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        pldm_monitor_gen_cmd_10,
        pldm_monitor_gen_cmd_11,
        pldm_monitor_gen_cmd_12,
        pldm_monitor_gen_cmd_13,
        gen_cmd_unsupport,
        pldm_monitor_gen_cmd_15,
    };

    if (cmd < PLDM_MONITOR_CMD) {
        if (cmds[cmd])
            cmds[cmd](buf);
    } else if (cmd == 0x20) {
        pldm_monitor_gen_cmd_20(buf);
    } else if (cmd == 0x21) {
        pldm_monitor_gen_cmd_21(buf);
    } else if (cmd == 0x50) {
        pldm_monitor_gen_cmd_50(buf);
    } else if (cmd == 0x51) {
        pldm_monitor_gen_cmd_51(buf);
    } else if (cmd == 0x53) {
        pldm_monitor_gen_cmd_53(buf);
    } else if (cmd == 0x54) {
        pldm_monitor_gen_cmd_54(buf);
    }else {
        gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_ENTER_CMD_UNKNOW;
        gen_cmd_unsupport(buf);
    }
}

void pldm_monitor_gen_init(void)
{
    gs_pldm_monitor_gen_state.cur_state = PLDM_MONITOR_GEN_IDLE;
    gs_pldm_monitor_gen_state.prev_state = PLDM_MONITOR_GEN_IDLE;
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
}

pkt_gen_state_transform_t pldm_monitor_state_transform[] = {
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_ENTER_CMD_01,     PLDM_MONITOR_GEN_IDLE, NULL},
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_ENTER_CMD_02,     PLDM_MONITOR_GEN_IDLE, NULL},
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_ENTER_CMD_03,     PLDM_MONITOR_GEN_IDLE, NULL},
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_ENTER_CMD_04,     PLDM_MONITOR_GEN_IDLE, NULL},
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_ENTER_CMD_05,     PLDM_MONITOR_GEN_IDLE, NULL},
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_ENTER_CMD_0C,     PLDM_MONITOR_GEN_IDLE, NULL},
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_ENTER_CMD_0D,     PLDM_MONITOR_GEN_IDLE, NULL},
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_ENTER_CMD_10,     PLDM_MONITOR_GEN_IDLE, NULL},
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_ENTER_CMD_11,     PLDM_MONITOR_GEN_IDLE, NULL},
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_ENTER_CMD_12,     PLDM_MONITOR_GEN_IDLE, NULL},
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_ENTER_CMD_13,     PLDM_MONITOR_GEN_IDLE, NULL},
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_ENTER_CMD_15,     PLDM_MONITOR_GEN_IDLE, NULL},
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_ENTER_CMD_20,     PLDM_MONITOR_GEN_IDLE, NULL},
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_ENTER_CMD_21,     PLDM_MONITOR_GEN_IDLE, NULL},
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_ENTER_CMD_50,     PLDM_MONITOR_GEN_IDLE, NULL},
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_ENTER_CMD_53,     PLDM_MONITOR_GEN_IDLE, NULL},

    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_ENTER_CMD_UNKNOW, PLDM_MONITOR_GEN_IDLE, NULL},
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_GET_PDR,          PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_CMD_GEN(51)},
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_RECV_EVENT,       PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_CMD_GEN(0a)},
    {PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_GEN_NEED_POLL_EVENT,  PLDM_MONITOR_GEN_IDLE, PLDM_MONITOR_CMD_GEN(0b)},
};

u8 pldm_monitor_state_transform_switch(u8 cnt, u8 *buf)
{
    u8 ret = 0xFF;
    // LOG("gs_pldm_monitor_gen_state.event_id : %d", gs_pldm_monitor_gen_state.event_id);
    for (u8 i = 0; i < sizeof(pldm_monitor_state_transform) / sizeof(pkt_gen_state_transform_t); i++) {
        if (gs_pldm_monitor_gen_state.cur_state == pldm_monitor_state_transform[i].cur_state && gs_pldm_monitor_gen_state.event_id == pldm_monitor_state_transform[i].event_id) {
            gs_pldm_monitor_gen_state.prev_state = gs_pldm_monitor_gen_state.cur_state;
            gs_pldm_monitor_gen_state.cur_state = pldm_monitor_state_transform[i].next_state;
            LOG("gs_pldm_monitor_gen prev state : %d, cur state : %d, event id : %d", gs_pldm_monitor_gen_state.prev_state, gs_pldm_monitor_gen_state.cur_state, gs_pldm_monitor_gen_state.event_id);  /* for debug */

            if (pldm_monitor_state_transform[i].action != NULL) {
                pldm_monitor_state_transform[i].action(buf);
            }

            if (gs_pldm_monitor_gen_state.event_id == PLDM_MONITOR_GEN_GET_PDR)
                gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;

            if (gs_pldm_monitor_gen_state.event_id == PLDM_MONITOR_GEN_RECV_EVENT)
                gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;

            if (gs_pldm_monitor_gen_state.event_id == PLDM_MONITOR_GEN_NEED_POLL_EVENT)
                gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
            ret = 1;
            break;
        }
    }
    return ret;
}