#include "pldm_ctrl_gen.h"
#include "pldm_control.h"
#include "pkt_gen.h"

pldm_gen_state_t gs_pldm_ctrl_gen_state;

static void pldm_ctrl_gen_cmd_01(u8 *buf)
{
    pldm_set_tid_req_dat_t *req_dat = (pldm_set_tid_req_dat_t *)buf;
    req_dat->tid = 0x66;
    gs_pldm_ctrl_gen_state.event_id = PLDM_CTRL_GEN_ENTER_CMD_01;
}

static void pldm_ctrl_gen_cmd_02(u8 *buf)
{
    gs_pldm_ctrl_gen_state.event_id = PLDM_CTRL_GEN_ENTER_CMD_02;
}

static void pldm_ctrl_gen_cmd_03(u8 *buf)
{
    pldm_get_ver_req_dat_t *req_dat = (pldm_get_ver_req_dat_t *)buf;
    req_dat->trans_op_flag = GET_FIRST_PART;
    req_dat->pldm_type = PLDM_FOR_PLATFORM_MONITORING_AND_CONTROL;
    gs_pldm_ctrl_gen_state.event_id = PLDM_CTRL_GEN_ENTER_CMD_03;
}

static void pldm_ctrl_gen_cmd_04(u8 *buf)
{
    gs_pldm_ctrl_gen_state.event_id = PLDM_CTRL_GEN_ENTER_CMD_04;
}

static void pldm_ctrl_gen_cmd_05(u8 *buf)
{
    pldm_get_cmd_req_dat_t *req_dat = (pldm_get_cmd_req_dat_t *)buf;
    req_dat->pldm_type = MCTP_PLDM_MONITOR;
    req_dat->ver = PLDM_TYPE_2_VERSION;
    gs_pldm_ctrl_gen_state.event_id = PLDM_CTRL_GEN_ENTER_CMD_05;
}

void pldm_ctrl_gen(int cmd, u8 *buf)
{
    gen_cmd cmds[] = {
        gen_cmd_unsupport,
        pldm_ctrl_gen_cmd_01,
        pldm_ctrl_gen_cmd_02,
        pldm_ctrl_gen_cmd_03,
        pldm_ctrl_gen_cmd_04,
        pldm_ctrl_gen_cmd_05,
    };

    if (cmd < PLDM_CTRL_CMD) {
        if (cmds[cmd])
            cmds[cmd](buf);
    } else {
        gs_pldm_ctrl_gen_state.event_id = PLDM_CTRL_GEN_ENTER_CMD_UNKNOW;
        gen_cmd_unsupport(buf);
    }
}

void pldm_ctrl_gen_init(void)
{
    gs_pldm_ctrl_gen_state.cur_state = PLDM_CTRL_GEN_IDLE;
    gs_pldm_ctrl_gen_state.prev_state = PLDM_CTRL_GEN_IDLE;
    gs_pldm_ctrl_gen_state.event_id = PLDM_CTRL_GEN_UNKNOW;
}

pkt_gen_state_transform_t pldm_ctrl_state_transform[] = {
    {PLDM_CTRL_GEN_IDLE, PLDM_CTRL_GEN_ENTER_CMD_01,     PLDM_CTRL_GEN_IDLE, NULL},
    {PLDM_CTRL_GEN_IDLE, PLDM_CTRL_GEN_ENTER_CMD_02,     PLDM_CTRL_GEN_IDLE, NULL},
    {PLDM_CTRL_GEN_IDLE, PLDM_CTRL_GEN_ENTER_CMD_03,     PLDM_CTRL_GEN_IDLE, NULL},
    {PLDM_CTRL_GEN_IDLE, PLDM_CTRL_GEN_ENTER_CMD_04,     PLDM_CTRL_GEN_IDLE, NULL},
    {PLDM_CTRL_GEN_IDLE, PLDM_CTRL_GEN_ENTER_CMD_05,     PLDM_CTRL_GEN_IDLE, NULL},
    {PLDM_CTRL_GEN_IDLE, PLDM_CTRL_GEN_ENTER_CMD_UNKNOW, PLDM_CTRL_GEN_IDLE, NULL},
};

u8 pldm_ctrl_state_transform_switch(u8 cnt, u8 *buf)
{
    u8 ret = 0xFF;
    for (u8 i = 0; i < sizeof(pldm_ctrl_state_transform) / sizeof(pkt_gen_state_transform_t); i++) {
        if (gs_pldm_ctrl_gen_state.cur_state == pldm_ctrl_state_transform[i].cur_state && gs_pldm_ctrl_gen_state.event_id == pldm_ctrl_state_transform[i].event_id) {
            gs_pldm_ctrl_gen_state.prev_state = gs_pldm_ctrl_gen_state.cur_state;
            gs_pldm_ctrl_gen_state.cur_state = pldm_ctrl_state_transform[i].next_state;
            // LOG("gs_pldm_ctrl_gen prev state : %d, cur state : %d, event id : %d", gs_pldm_ctrl_gen_state.prev_state, gs_pldm_ctrl_gen_state.cur_state, gs_pldm_ctrl_gen_state.event_id);  /* for debug */
            if (pldm_ctrl_state_transform[i].action != NULL) {
                pldm_ctrl_state_transform[i].action(buf);
            }
            ret = 1;
            break;
        }
    }
    return ret;
}