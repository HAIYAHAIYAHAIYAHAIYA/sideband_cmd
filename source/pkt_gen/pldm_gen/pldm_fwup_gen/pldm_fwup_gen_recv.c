#include "pkt_gen.h"
#include "pldm_fwup_gen.h"

extern volatile u8 g_pldm_fwup_gen_event_id;
extern pldm_fwup_gen_state_t gs_pldm_fwup_gen_state;
static pldm_fwup_req_fw_data_req_dat_t gs_fw_data_req_dat;

static void pldm_fwup_gen_recv_cmd_01(u8 *buf)
{
    g_pldm_fwup_gen_event_id = PLDM_FWUP_GEN_GET_FD_INDENTIFY;
}

static void pldm_fwup_gen_recv_cmd_02(u8 *buf)
{
    g_pldm_fwup_gen_event_id = PLDM_FWUP_GEN_GET_FD_PARAM_AND_NOT_UPDATE;
}

static void pldm_fwup_gen_recv_cmd_10(u8 *buf)
{

}

static void pldm_fwup_gen_recv_cmd_11(u8 *buf)
{
    g_pldm_fwup_gen_event_id = PLDM_FWUP_GEN_SEND_PKT_DATA_END;
}

static void pldm_fwup_gen_recv_cmd_13(u8 *buf)
{
    g_pldm_fwup_gen_event_id = PLDM_FWUP_GEN_SEND_PASS_COMP_END;
}

static void pldm_fwup_gen_recv_cmd_14(u8 *buf)
{
    g_pldm_fwup_gen_event_id = PLDM_FWUP_GEN_SEND_UP_COMP_END;
}

static void pldm_fwup_gen_recv_cmd_15(u8 *buf)
{
    pldm_fwup_req_fw_data_req_dat_t *req_dat = (pldm_fwup_req_fw_data_req_dat_t *)buf;
    gs_fw_data_req_dat.offset = req_dat->offset;
    gs_fw_data_req_dat.len = req_dat->len;
    g_pldm_fwup_gen_event_id = PLDM_FWUP_GEN_SEND_FW_DATA;
}

pldm_fwup_req_fw_data_req_dat_t pldm_fwup_gen_recv_get_fw_data_req_dat(void)
{
    return gs_fw_data_req_dat;
}

static void pldm_fwup_gen_recv_cmd_16(u8 *buf)
{
    g_pldm_fwup_gen_event_id = PLDM_FWUP_GEN_TRANS_FW_DATA_END;
}

static void pldm_fwup_gen_recv_cmd_17(u8 *buf)
{
    g_pldm_fwup_gen_event_id = PLDM_FWUP_GEN_FD_VERIFY_END;
}

static void pldm_fwup_gen_recv_cmd_18(u8 *buf)
{
    g_pldm_fwup_gen_event_id = PLDM_FWUP_GEN_FD_APPLY_END;
    // g_pldm_fwup_gen_event_id = PLDM_FWUP_GEN_NO_IMG;
}

static void pldm_fwup_gen_recv_cmd_1a(u8 *buf)
{
    g_pldm_fwup_gen_event_id = PLDM_FWUP_GEN_SEND_ACTIV_CMD;
}

static void pldm_fwup_gen_recv_cmd_1b(u8 *buf)
{
    g_pldm_fwup_gen_event_id = PLDM_FWUP_GEN_FD_IS_UPDATE;
}

static void pldm_fwup_gen_recv_cmd_1c(u8 *buf)
{

}

static void pldm_fwup_gen_recv_cmd_1d(u8 *buf)
{

}

void pldm_fwup_gen_recv(int cmd, u8 *buf)
{
    gen_cmd cmds[] = {
        gen_cmd_unsupport,
        pldm_fwup_gen_recv_cmd_01,
        pldm_fwup_gen_recv_cmd_02,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        pldm_fwup_gen_recv_cmd_10,
        pldm_fwup_gen_recv_cmd_11,
        gen_cmd_unsupport,
        pldm_fwup_gen_recv_cmd_13,
        pldm_fwup_gen_recv_cmd_14,
        pldm_fwup_gen_recv_cmd_15,
        pldm_fwup_gen_recv_cmd_16,
        pldm_fwup_gen_recv_cmd_17,
        pldm_fwup_gen_recv_cmd_18,
        gen_cmd_unsupport,
        pldm_fwup_gen_recv_cmd_1a,
        pldm_fwup_gen_recv_cmd_1b,
        pldm_fwup_gen_recv_cmd_1c,
        pldm_fwup_gen_recv_cmd_1d,
    };

    if (cmd < PLDM_FW_UPDATE_CMD) {
        if (cmds[cmd])
            cmds[cmd](buf);
            LOG("RECV CMD : %#x", cmd);
            // LOG("pldm_fwup_gen prev state : %d, cur state : %d, event id : %d", gs_pldm_fwup_gen_state.prev_state, gs_pldm_fwup_gen_state.cur_state, g_pldm_fwup_gen_event_id);  /* for debug */
    } else {
        LOG("ERR CMD : %#x", cmd);
    }
}