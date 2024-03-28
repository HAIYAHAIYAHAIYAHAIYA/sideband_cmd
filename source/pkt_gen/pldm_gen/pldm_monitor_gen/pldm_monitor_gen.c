#include "pldm_monitor_gen.h"
#include "pldm_monitor.h"
#include "pldm_control.h"
#include "pkt_gen.h"

static void pldm_monitor_gen_cmd_01(u8 *buf)
{
    pldm_set_tid_req_dat_t *req_dat = (pldm_set_tid_req_dat_t *)buf;
    req_dat->tid = 0x67;
}

static void pldm_monitor_gen_cmd_02(u8 *buf)
{

}

static void pldm_monitor_gen_cmd_03(u8 *buf)
{

}

static void pldm_monitor_gen_cmd_04(u8 *buf)
{
    pldm_set_event_receiver_req_dat_t *req_dat = (pldm_set_event_receiver_req_dat_t *)buf;
    req_dat->event_msg_global_en = PLDM_ENABLE_ASYNC;
    req_dat->trans_protocol_type = 0x00;
    req_dat->event_receiver_addr_info = 0xcc;
}

static void pldm_monitor_gen_cmd_05(u8 *buf)
{

}

static void pldm_monitor_gen_cmd_0a(u8 *buf)
{

}

static void pldm_monitor_gen_cmd_0b(u8 *buf)
{

}

static void pldm_monitor_gen_cmd_0c(u8 *buf)
{
    pldm_event_msg_supported_req_dat_t *req_dat = (pldm_event_msg_supported_req_dat_t *)buf;
    req_dat->format_ver = PLDM_EVENT_FORMAT_VERSION;
}

static void pldm_monitor_gen_cmd_0d(u8 *buf)
{
    pldm_event_msg_buffersize_req_dat_t *req_dat = (pldm_event_msg_buffersize_req_dat_t *)buf;
    req_dat->event_receiver_max_buffersize = 256;
}

static void pldm_monitor_gen_cmd_10(u8 *buf)
{
    pldm_set_numeric_sensor_enable_rsp_dat_t *req_dat = (pldm_set_numeric_sensor_enable_rsp_dat_t *)buf;
    req_dat->sensor_id = PLDM_BASE_NIC_TEMP_SENSOR_ID;
    req_dat->sensor_op_state = PLDM_OP_ENABLE;
    req_dat->sensor_event_msg_en = PLDM_EVENT_EN;
}

static void pldm_monitor_gen_cmd_11(u8 *buf)
{
    pldm_get_sensor_reading_req_dat_t *req_dat = (pldm_get_sensor_reading_req_dat_t *)buf;
    req_dat->sensor_id = PLDM_BASE_NIC_TEMP_SENSOR_ID;
    req_dat->rearm_event_state = 0;
}

static void pldm_monitor_gen_cmd_12(u8 *buf)
{
    pldm_get_sensor_thresholds_req_dat_t *req_dat = (pldm_get_sensor_thresholds_req_dat_t *)buf;
    req_dat->sensor_id = PLDM_BASE_NIC_TEMP_SENSOR_ID;
}

static void pldm_monitor_gen_cmd_13(u8 *buf)
{
    pldm_set_sensor_thresholds_req_dat_t *req_dat = (pldm_set_sensor_thresholds_req_dat_t *)buf;
    req_dat->sensor_id = PLDM_BASE_NIC_TEMP_SENSOR_ID;
    req_dat->sensor_datasize = PLDM_DATASIZE_UINT8;
    req_dat->upper_threshold_warning = 20;
    req_dat->upper_threshold_critical = 50;
    req_dat->upper_threshold_fatal = 80;
}

static void pldm_monitor_gen_cmd_15(u8 *buf)
{
    pldm_get_sensor_hysteresis_req_dat_t *req_dat = (pldm_get_sensor_hysteresis_req_dat_t *)buf;
    req_dat->sensor_id = PLDM_BASE_NIC_TEMP_SENSOR_ID;
}

static void pldm_monitor_gen_cmd_20(u8 *buf)
{
    pldm_set_state_sensor_en_req_dat_t *req_dat = (pldm_set_state_sensor_en_req_dat_t *)buf;
    req_dat->sensor_id = PLDM_BASE_NIC_TEMP_SENSOR_ID;
    req_dat->composite_sensor_cnt = 1;
    req_dat->opfield[0].event_msg_en = PLDM_EVENT_EN;
    req_dat->opfield[0].sensor_op_state = PLDM_OP_ENABLE;
}

static void pldm_monitor_gen_cmd_21(u8 *buf)
{
    pldm_get_state_sensor_reading_req_dat_t *req_dat = (pldm_get_state_sensor_reading_req_dat_t *)buf;
    req_dat->sensor_id = PLDM_BASE_NIC_TEMP_SENSOR_ID;
}

static void pldm_monitor_gen_cmd_50(u8 *buf)
{

}

static void pldm_monitor_gen_cmd_51(u8 *buf)
{
    pldm_get_pdr_req_dat_t *req_dat = (pldm_get_pdr_req_dat_t *)buf;
    req_dat->data_transfer_handle = 0;
    req_dat->record_handle = 1;
    req_dat->transfer_op_state = PLDM_TRANSFER_OP_FLAG_GET_FIRST_PART;
    req_dat->req_cnt = 256;
    req_dat->record_chg_num = 0;
}

static void pldm_monitor_gen_cmd_53(u8 *buf)
{

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
    } else {
        gen_cmd_unsupport(buf);
    }
}