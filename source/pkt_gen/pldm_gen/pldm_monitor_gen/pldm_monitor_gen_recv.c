#include "pldm_monitor_gen.h"
#include "pldm_control.h"
#include "pldm_monitor_event_rbuf.h"
#include "pkt_gen.h"

extern pldm_gen_state_t gs_pldm_monitor_gen_state;

static void pldm_monitor_gen_recv_cmd_01(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
}

static void pldm_monitor_gen_recv_cmd_02(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
}

static void pldm_monitor_gen_recv_cmd_03(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
}

static void pldm_monitor_gen_recv_cmd_04(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
}

static void pldm_monitor_gen_recv_cmd_05(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
}

static pldm_poll_for_platform_event_msg_rsp_dat_t get_pull_event_rsp_dat;
static void pldm_monitor_gen_recv_cmd_0a(u8 *buf)
{
    pldm_platform_event_msg_req_dat_t *req_dat = (pldm_platform_event_msg_req_dat_t *)buf;
    pldm_pldm_msg_poll_event_data_format_t *pull_dat = (pldm_pldm_msg_poll_event_data_format_t *)(req_dat->event_data);
    LOG("format_ver : %d, tid : %#x, event_class : %d", req_dat->format_ver, req_dat->tid, req_dat->event_class);
    if (req_dat->event_class == PLDM_MSG_POLL_EVENT) {
        gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_NEED_POLL_EVENT;
        get_pull_event_rsp_dat.event_id = pull_dat->event_id;
        get_pull_event_rsp_dat.next_data_transfer_handle = pull_dat->data_transfer_handle;
    } else {
        gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_RECV_EVENT;
    }
}

extern u16 g_event_id;
u8 op_cnt = 1;
static void pldm_monitor_gen_recv_cmd_0b(u8 *buf)
{
    pldm_poll_for_platform_event_msg_rsp_dat_t *rsp_dat = (pldm_poll_for_platform_event_msg_rsp_dat_t *)(buf + sizeof(pldm_response_t));
    if (rsp_dat->event_id) {
        get_pull_event_rsp_dat = *rsp_dat;
        gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_NEED_POLL_EVENT;
        LOG("event_id : %d, event_class : %d, tid : %#x, transfer_flag : %d", rsp_dat->event_id, rsp_dat->event_class, rsp_dat->tid, rsp_dat->transfer_flag);
    } else {
        op_cnt = op_cnt ^ 1;
        for (u8 i = 0; i < MAX_LAN_NUM; i++) {
            pldm_link_handle(i, op_cnt);
        }
        LOG("more event! %d", op_cnt);
        LOG("g_event_id : %d", g_event_id);
    }
}

pldm_poll_for_platform_event_msg_rsp_dat_t pldm_monitor_get_pull_event_rsp_dat(void)
{
    return get_pull_event_rsp_dat;
}

static void pldm_monitor_gen_recv_cmd_0c(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
}

static void pldm_monitor_gen_recv_cmd_0d(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
}

static void pldm_monitor_gen_recv_cmd_10(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
}

static void pldm_monitor_gen_recv_cmd_11(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
}

static void pldm_monitor_gen_recv_cmd_12(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
}

static void pldm_monitor_gen_recv_cmd_13(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
}

static void pldm_monitor_gen_recv_cmd_15(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
}

static void pldm_monitor_gen_recv_cmd_20(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
}

static void pldm_monitor_gen_recv_cmd_21(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
}

static void pldm_monitor_gen_recv_cmd_50(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
}

static pldm_get_pdr_rsp_dat_t get_pdr_rsp_dat;
u8 pdr_size = 0;
u8 pdr_cnt = 0;

static void pldm_monitor_gen_recv_cmd_51(u8 *buf)
{
    pldm_get_pdr_rsp_dat_t *rsp_dat = (pldm_get_pdr_rsp_dat_t *)(buf + sizeof(pldm_response_t));
    pldm_response_t *rsp_hdr = (pldm_response_t *)(buf);

    if (rsp_hdr->cpl_code == MCTP_COMMAND_SUCCESS) {
        // LOG("next record hdl : %d, rsp_cnt : %d, next_data_transfer_handle : %d", rsp_dat->next_record_handle, rsp_dat->rsp_cnt, rsp_dat->next_data_transfer_handle);
        pdr_size += rsp_dat->rsp_cnt;
        if (rsp_dat->transfer_flag == PLDM_TRANSFER_FLAG_END || rsp_dat->transfer_flag == PLDM_TRANSFER_FLAG_START_AND_END) {
            pdr_cnt++;
            LOG("pdr_cnt : %d, pdr_size : %d", pdr_cnt, pdr_size);
            pdr_size = 0;
        }
        get_pdr_rsp_dat = *rsp_dat;
        if (rsp_dat->next_record_handle != 0) {
            gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_GET_PDR;
        } else {
            get_pdr_rsp_dat.next_record_handle = 1;
            gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
            pdr_cnt = 0;
        }
    } else {
        gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
        LOG("pldm_monitor_gen_recv_cmd_51 : %#x", rsp_hdr->cpl_code);
    }
}

pldm_get_pdr_rsp_dat_t pldm_monitor_get_pdr_rsp_dat(void)
{
    return get_pdr_rsp_dat;
}

static void pldm_monitor_gen_recv_cmd_53(u8 *buf)
{
    gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
}

void pldm_monitor_gen_recv(int cmd, u8 *buf)
{
    gen_cmd cmds[] = {
        gen_cmd_unsupport,
        pldm_monitor_gen_recv_cmd_01,
        pldm_monitor_gen_recv_cmd_02,
        pldm_monitor_gen_recv_cmd_03,
        pldm_monitor_gen_recv_cmd_04,
        pldm_monitor_gen_recv_cmd_05,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        pldm_monitor_gen_recv_cmd_0a,
        pldm_monitor_gen_recv_cmd_0b,
        pldm_monitor_gen_recv_cmd_0c,
        pldm_monitor_gen_recv_cmd_0d,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        pldm_monitor_gen_recv_cmd_10,
        pldm_monitor_gen_recv_cmd_11,
        pldm_monitor_gen_recv_cmd_12,
        pldm_monitor_gen_recv_cmd_13,
        gen_cmd_unsupport,
        pldm_monitor_gen_recv_cmd_15,
    };

    if (cmd < PLDM_MONITOR_CMD) {
        if (cmds[cmd])
            cmds[cmd](buf);
        LOG("RECV CMD : %#x\n", cmd);
    } else if (cmd == 0x20) {
        pldm_monitor_gen_recv_cmd_20(buf);
        LOG("RECV CMD : %#x\n", cmd);
    } else if (cmd == 0x21) {
        pldm_monitor_gen_recv_cmd_21(buf);
        LOG("RECV CMD : %#x\n", cmd);
    } else if (cmd == 0x50) {
        pldm_monitor_gen_recv_cmd_50(buf);
        LOG("RECV CMD : %#x\n", cmd);
    } else if (cmd == 0x51) {
        pldm_monitor_gen_recv_cmd_51(buf);
        LOG("RECV CMD : %#x\n", cmd);
    } else if (cmd == 0x53) {
        pldm_monitor_gen_recv_cmd_53(buf);
        LOG("RECV CMD : %#x\n", cmd);
    } else {
        gs_pldm_monitor_gen_state.event_id = PLDM_MONITOR_GEN_UNKNOW;
        LOG("ERR CMD : %#x\n", cmd);
    }
}