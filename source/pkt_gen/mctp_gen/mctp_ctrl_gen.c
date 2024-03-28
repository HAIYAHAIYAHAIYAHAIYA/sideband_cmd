#include "mctp_ctrl_gen.h"
#include "mctp_ctl.h"
#include "mctp.h"
#include "pkt_gen.h"

static void mctp_ctrl_gen_cmd_01(u8 *buf)
{
    set_eid_req_dat *req_dat = (set_eid_req_dat *)buf;
    req_dat->operation = SET_EID;
    req_dat->eid = 0xA5;
}

static void mctp_ctrl_gen_cmd_02(u8 *buf)
{

}

static void mctp_ctrl_gen_cmd_03(u8 *buf)
{

}

static void mctp_ctrl_gen_cmd_04(u8 *buf)
{
    get_ver_sup_req_dat *req_dat = (get_ver_sup_req_dat *)buf;
    req_dat->msg_type_num = MCTP_MSG_TYPE_CONTROL;
}

static void mctp_ctrl_gen_cmd_05(u8 *buf)
{

}

static void mctp_ctrl_gen_cmd_06(u8 *buf)
{
    get_vendor_defined_msg_sup_req_dat *req_dat = (get_vendor_defined_msg_sup_req_dat *)buf;
    req_dat->vendor_id_set_selector == 0x00;
}

static void mctp_ctrl_gen_cmd_07(u8 *buf)
{
    resolve_eid_rsp_dat *rsp_dat = (resolve_eid_rsp_dat *)buf;
    rsp_dat->bridge_eid = 0x55;
    rsp_dat->phy_addr.bus_num = 1;
    rsp_dat->phy_addr.dev_num = 2;
    rsp_dat->phy_addr.func_num = 3;
}

static void mctp_ctrl_gen_cmd_0b(u8 *buf)
{

}

static void mctp_ctrl_gen_cmd_0c(u8 *buf)
{

}

static void mctp_ctrl_gen_cmd_0d(u8 *buf)
{

}

static void mctp_ctrl_gen_cmd_11(u8 *buf)
{

}

static void mctp_ctrl_gen_cmd_12(u8 *buf)
{

}

static void mctp_ctrl_gen_cmd_13(u8 *buf)
{

}

static void mctp_ctrl_gen_cmd_14(u8 *buf)
{

}

void mctp_ctrl_gen(int cmd, u8 *buf)
{
    gen_cmd cmds[] = {
        gen_cmd_unsupport,
        mctp_ctrl_gen_cmd_01,
        mctp_ctrl_gen_cmd_02,
        mctp_ctrl_gen_cmd_03,
        mctp_ctrl_gen_cmd_04,
        mctp_ctrl_gen_cmd_05,
        mctp_ctrl_gen_cmd_06,
        mctp_ctrl_gen_cmd_07,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        mctp_ctrl_gen_cmd_0b,
        mctp_ctrl_gen_cmd_0c,
        mctp_ctrl_gen_cmd_0d,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        mctp_ctrl_gen_cmd_11,
        mctp_ctrl_gen_cmd_12,
        mctp_ctrl_gen_cmd_13,
        mctp_ctrl_gen_cmd_14,
    };

    mctp_ctrl_request_t *req_hdr = (mctp_ctrl_request_t *)buf;
    req_hdr->cmd_code = cmd;
    buf = buf + sizeof(mctp_ctrl_request_t);

    if (cmd < MCTP_CTL_CMD) {
        if (cmds[cmd])
            cmds[cmd](buf);
    } else {
        gen_cmd_unsupport(buf);
    }
}