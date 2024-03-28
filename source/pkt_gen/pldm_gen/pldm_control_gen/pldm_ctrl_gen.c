#include "pldm_ctrl_gen.h"
#include "pldm_control.h"
#include "pkt_gen.h"

extern void gen_cmd_unsupport(u8 *buf);

static void pldm_ctrl_gen_cmd_01(u8 *buf)
{
    pldm_set_tid_req_dat_t *req_dat = (pldm_set_tid_req_dat_t *)buf;
    req_dat->tid = 0x66;
}

static void pldm_ctrl_gen_cmd_02(u8 *buf)
{

}

static void pldm_ctrl_gen_cmd_03(u8 *buf)
{
    pldm_get_ver_req_dat_t *req_dat = (pldm_get_ver_req_dat_t *)buf;
    req_dat->trans_op_flag = GET_FIRST_PART;
    req_dat->pldm_type = PLDM_FOR_PLATFORM_MONITORING_AND_CONTROL;
}

static void pldm_ctrl_gen_cmd_04(u8 *buf)
{

}

static void pldm_ctrl_gen_cmd_05(u8 *buf)
{
    pldm_get_cmd_req_dat_t *req_dat = (pldm_get_cmd_req_dat_t *)buf;
    req_dat->pldm_type = MCTP_PLDM_MONITOR;
    req_dat->ver = PLDM_TYPE_2_VERSION;
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
        gen_cmd_unsupport(buf);
    }
}