#include "pldm_fru_gen.h"
#include "pldm_monitor.h"
#include "pkt_gen.h"

pldm_gen_state_t gs_pldm_fru_gen_state;

static void pldm_fru_gen_cmd_01(u8 *buf)
{
    gs_pldm_fru_gen_state.event_id = PLDM_FRU_GEN_ENTER_CMD_01;
}

static void pldm_fru_gen_cmd_02(u8 *buf)
{
    pldm_fru_get_fru_record_table_req_dat_t *req_dat = (pldm_fru_get_fru_record_table_req_dat_t *)buf;
    pldm_fru_get_fru_record_table_rsp_dat_t rsp_dat = pldm_fru_get_fru_record_table_rsp_dat();
    req_dat->data_transfer_hdl = rsp_dat.next_data_transfer_hdl;
    req_dat->transfer_op_flg = rsp_dat.next_data_transfer_hdl == 0 ? PLDM_TRANSFER_OP_FLAG_GET_FIRST_PART : PLDM_TRANSFER_OP_FLAG_GET_NEXT_PART;
    gs_pldm_fru_gen_state.event_id = PLDM_FRU_GEN_ENTER_CMD_02;
}

void pldm_fru_gen(int cmd, u8 *buf)
{
    gen_cmd cmds[] = {
        gen_cmd_unsupport,
        pldm_fru_gen_cmd_01,
        pldm_fru_gen_cmd_02,
    };

    if (cmd < PLDM_FRU_DATA_CMD) {
        if (cmds[cmd])
            cmds[cmd](buf);
    } else {
        gen_cmd_unsupport(buf);
    }
}

void pldm_fru_gen_init(void)
{
    gs_pldm_fru_gen_state.cur_state = PLDM_FRU_GEN_IDLE;
    gs_pldm_fru_gen_state.prev_state = PLDM_FRU_GEN_IDLE;
    gs_pldm_fru_gen_state.event_id = PLDM_FRU_GEN_UNKNOW;
}

pkt_gen_state_transform_t pldm_fru_state_transform[] = {
    {PLDM_FRU_GEN_IDLE, PLDM_FRU_GEN_ENTER_CMD_01,     PLDM_FRU_GEN_IDLE, NULL},
    {PLDM_FRU_GEN_IDLE, PLDM_FRU_GEN_ENTER_CMD_02,     PLDM_FRU_GEN_IDLE, PLDM_FRU_CMD_GEN(02)},
    {PLDM_FRU_GEN_IDLE, PLDM_FRU_GEN_ENTER_CMD_UNKNOW, PLDM_FRU_GEN_IDLE, NULL},
};

u8 pldm_fru_state_transform_switch(u8 cnt, u8 *buf)
{
    u8 ret = 0xFF;
    for (u8 i = 0; i < sizeof(pldm_fru_state_transform) / sizeof(pkt_gen_state_transform_t); i++) {
        if (gs_pldm_fru_gen_state.cur_state == pldm_fru_state_transform[i].cur_state && gs_pldm_fru_gen_state.event_id == pldm_fru_state_transform[i].event_id) {
            gs_pldm_fru_gen_state.prev_state = gs_pldm_fru_gen_state.cur_state;
            gs_pldm_fru_gen_state.cur_state = pldm_fru_state_transform[i].next_state;

            if (gs_pldm_fru_gen_state.event_id == PLDM_FRU_GEN_ENTER_CMD_02)
                gs_pldm_fru_gen_state.event_id = PLDM_FRU_GEN_ENTER_CMD_UNKNOW;

            // LOG("gs_pldm_fru_gen prev state : %d, cur state : %d, event id : %d", gs_pldm_fru_gen_state.prev_state, gs_pldm_fru_gen_state.cur_state, gs_pldm_fru_gen_state.event_id);  /* for debug */
            if (pldm_fru_state_transform[i].action != NULL) {
                pldm_fru_state_transform[i].action(buf);
            }
            ret = 1;
            break;
        }
    }
    return ret;
}