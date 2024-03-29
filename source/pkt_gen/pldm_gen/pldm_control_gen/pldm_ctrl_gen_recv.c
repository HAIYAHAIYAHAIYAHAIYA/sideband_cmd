#include "pldm_ctrl_gen.h"
#include "pldm_control.h"
#include "pkt_gen.h"

extern pldm_gen_state_t gs_pldm_ctrl_gen_state;

static void pldm_ctrl_gen_recv_cmd_01(u8 *buf)
{
    gs_pldm_ctrl_gen_state.event_id = PLDM_CTRL_GEN_UNKNOW;
}

static void pldm_ctrl_gen_recv_cmd_02(u8 *buf)
{
    gs_pldm_ctrl_gen_state.event_id = PLDM_CTRL_GEN_UNKNOW;
}

static void pldm_ctrl_gen_recv_cmd_03(u8 *buf)
{
    gs_pldm_ctrl_gen_state.event_id = PLDM_CTRL_GEN_UNKNOW;
}

static void pldm_ctrl_gen_recv_cmd_04(u8 *buf)
{
    gs_pldm_ctrl_gen_state.event_id = PLDM_CTRL_GEN_UNKNOW;
}

static void pldm_ctrl_gen_recv_cmd_05(u8 *buf)
{
    gs_pldm_ctrl_gen_state.event_id = PLDM_CTRL_GEN_UNKNOW;
}

void pldm_ctrl_gen_recv(int cmd, u8 *buf)
{
    gen_cmd cmds[] = {
        gen_cmd_unsupport,
        pldm_ctrl_gen_recv_cmd_01,
        pldm_ctrl_gen_recv_cmd_02,
        pldm_ctrl_gen_recv_cmd_03,
        pldm_ctrl_gen_recv_cmd_04,
        pldm_ctrl_gen_recv_cmd_05,
    };

    if (cmd < PLDM_CTRL_CMD) {
        if (cmds[cmd])
            cmds[cmd](buf);
        LOG("RECV CMD : %#x\n", cmd);
    } else {
        gs_pldm_ctrl_gen_state.event_id = PLDM_CTRL_GEN_UNKNOW;
        LOG("ERR CMD : %#x\n", cmd);
    }
}