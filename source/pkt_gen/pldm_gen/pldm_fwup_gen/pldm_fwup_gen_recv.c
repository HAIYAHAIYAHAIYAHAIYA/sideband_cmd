#include "pkt_gen.h"
#include "pldm_fwup_gen.h"

extern pldm_gen_state_t gs_pldm_fwup_gen_state;
static pldm_fwup_req_fw_data_req_dat_t gs_fw_data_req_dat;

static void pldm_fwup_gen_recv_cmd_01(u8 *buf)
{
    pldm_query_dev_identifier_rsp_dat_t *rsp_dat = (pldm_query_dev_identifier_rsp_dat_t *)(buf + sizeof(pldm_response_t));
    LOG("dev_identifier_len : %d", rsp_dat->dev_identifier_len);
    LOG("descriptor_cnt : %d", rsp_dat->descriptor_cnt);
    pldm_add_descriptor_t *add_d = (pldm_add_descriptor_t *)(rsp_dat->add_descriptor);
    for (u8 i = 0; i < rsp_dat->descriptor_cnt; i++) {
        LOG("add_type : %d", add_d[i].add_type);
        LOG("add_len : %d", add_d[i].add_len);
        LOG("add_data : 0x%x", add_d[i].add_data);
    }
    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_GET_FD_INDENTIFY;
}

static void pldm_fwup_gen_recv_cmd_02(u8 *buf)
{
    pldm_get_fw_param_rsp_dat_t *rsp_dat = (pldm_get_fw_param_rsp_dat_t *)(buf + sizeof(pldm_response_t));
    u32 offset = 0;
    LOG("cap_during_ud : 0x%x", rsp_dat->cap_during_ud);
    LOG("comp_cnt : %d", rsp_dat->comp_cnt);
    LOG("actv_t : %d", rsp_dat->actv_comp_img_set_ver_str_type_and_len.type);
    LOG("actv_l : %d", rsp_dat->actv_comp_img_set_ver_str_type_and_len.len);
    for (u8 i = 0; i < rsp_dat->actv_comp_img_set_ver_str_type_and_len.len; i++) {
        printf("%c", rsp_dat->comp_img_set_ver_str[offset + i]);
    }
    LOG("");
    LOG("pending_t : %d", rsp_dat->pending_comp_img_set_ver_str_type_and_len.type);
    LOG("pending_l : %d", rsp_dat->pending_comp_img_set_ver_str_type_and_len.len);
    offset += rsp_dat->actv_comp_img_set_ver_str_type_and_len.len;
    for (u8 i = 0; i < rsp_dat->pending_comp_img_set_ver_str_type_and_len.len; i++) {
        printf("%c", rsp_dat->comp_img_set_ver_str[offset + i]);
    }
    LOG("");
    offset += rsp_dat->pending_comp_img_set_ver_str_type_and_len.len;
    pldm_fwup_comp_param_table_t *comp_parm_table = (pldm_fwup_comp_param_table_t *)&(rsp_dat->comp_img_set_ver_str[offset]);
    for (u8 i = 0; i < rsp_dat->comp_cnt; i++) {
        LOG("comp_classification : %d", comp_parm_table->comp_class_msg.comp_classification);
        LOG("comp_identifier : %d", comp_parm_table->comp_class_msg.comp_identifier);
        LOG("comp_classification_idx : %d", comp_parm_table->comp_class_msg.comp_classification_idx);

        LOG("comp_comparison_stamp : %d", comp_parm_table->actv_comp_ver_msg.comp_comparison_stamp);
        LOG("comp_ver_str_type : %d", comp_parm_table->actv_comp_ver_msg.comp_ver_str_type);
        LOG("comp_ver_str_len : %d", comp_parm_table->actv_comp_ver_msg.comp_ver_str_len);

        for (u8 j = 0; j < 8; j++) {
            LOG("actv_comp_release_date[%d] : %d", j, comp_parm_table->actv_comp_release_date[j]);
        }

        LOG("comp_comparison_stamp : %d", comp_parm_table->pending_comp_ver_msg.comp_comparison_stamp);
        LOG("comp_ver_str_type : %d", comp_parm_table->pending_comp_ver_msg.comp_ver_str_type);
        LOG("comp_ver_str_len : %d", comp_parm_table->pending_comp_ver_msg.comp_ver_str_len);

        for (u8 j = 0; j < 8; j++) {
            LOG("actv_comp_release_date[%d] : %d", j, comp_parm_table->pending_comp_release_date[j]);
        }

        u8 *str = comp_parm_table->comp_ver_str;
        for (u8 j = 0; j < comp_parm_table->actv_comp_ver_msg.comp_ver_str_len; j++) {
            printf("%c", str[j]);
        }
        LOG("");

        str = &(comp_parm_table->comp_ver_str[comp_parm_table->actv_comp_ver_msg.comp_ver_str_len]);
        for (u8 j = 0; j < comp_parm_table->pending_comp_ver_msg.comp_ver_str_len; j++) {
            printf("%c", str[j]);
        }
        LOG("");

        LOG("comp_actv_meth : 0x%x", comp_parm_table->comp_actv_meth);
        LOG("cap_during_ud : 0x%x", comp_parm_table->cap_during_ud);
        comp_parm_table = (pldm_fwup_comp_param_table_t *)&(str[comp_parm_table->pending_comp_ver_msg.comp_ver_str_len]);
    }
    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_GET_FD_PARAM_AND_NOT_UPDATE;
}

static void pldm_fwup_gen_recv_cmd_10(u8 *buf)
{

}

u32 gs_data_transfer_handle = 0;
static void pldm_fwup_gen_recv_cmd_11(u8 *buf)
{
    pldm_fwup_get_pkt_data_req_dat_t *req_dat = (pldm_fwup_get_pkt_data_req_dat_t *)buf;
    gs_data_transfer_handle = req_dat->data_transfer_handle;
    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_SEND_PKT_DATA;
}

u32 pldm_fwup_gen_recv_get_pkt_data_req_dat(void)
{
    return gs_data_transfer_handle;
}

static void pldm_fwup_gen_recv_cmd_13(u8 *buf)
{
    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_SEND_PASS_COMP_END;
}

static void pldm_fwup_gen_recv_cmd_14(u8 *buf)
{
    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_SEND_UP_COMP_END;
}

static void pldm_fwup_gen_recv_cmd_15(u8 *buf)
{
    pldm_fwup_req_fw_data_req_dat_t *req_dat = (pldm_fwup_req_fw_data_req_dat_t *)buf;
    gs_fw_data_req_dat.offset = req_dat->offset;
    gs_fw_data_req_dat.len = req_dat->len;
    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_SEND_FW_DATA;
}

pldm_fwup_req_fw_data_req_dat_t pldm_fwup_gen_recv_get_fw_data_req_dat(void)
{
    return gs_fw_data_req_dat;
}

static void pldm_fwup_gen_recv_cmd_16(u8 *buf)
{
    pldm_fwup_transfer_cpl_req_dat_t *req_dat = (pldm_fwup_transfer_cpl_req_dat_t *)buf;
    if (!(req_dat->transfer_result))
        // gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_TRANS_FW_DATA_END;
        ;
    else {
        LOG("pldm_fwup_transfer_cpl_req_dat_t err : %#x", req_dat->transfer_result);
    }
}

static void pldm_fwup_gen_recv_cmd_17(u8 *buf)
{
    pldm_fwup_verify_cpl_req_dat_t *req_dat = (pldm_fwup_verify_cpl_req_dat_t *)buf;
    if (!(req_dat->verify_result))
        // gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_FD_VERIFY_END;
        ;
    else
        LOG("pldm_fwup_verify_cpl_req_dat_t err : %#x", req_dat->verify_result);
}

static void pldm_fwup_gen_recv_cmd_18(u8 *buf)
{
    pldm_fwup_apply_cpl_req_dat_t *req_dat = (pldm_fwup_apply_cpl_req_dat_t *)buf;
    if (!(req_dat->apply_result))
        // gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_FD_APPLY_END;
        ;
    else
        LOG("pldm_fwup_apply_cpl_req_dat_t err : %#x", req_dat->apply_result);
    LOG("comp_actv_meth_modification : %#x", req_dat->comp_actv_meth_modification);
}

static void pldm_fwup_gen_recv_cmd_1a(u8 *buf)
{
    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_SEND_ACTIV_CMD;
}

static void pldm_fwup_gen_recv_cmd_1b(u8 *buf)
{
    pldm_fwup_get_status_rsp_dat_t *rsp_dat = (pldm_fwup_get_status_rsp_dat_t *)(buf + sizeof(pldm_response_t));
    LOG("%s", __FUNCTION__);
    LOG("cur_state : %d", rsp_dat->cur_state);
    LOG("prev_state : %d", rsp_dat->prev_state);
    LOG("aux_state : %d", rsp_dat->aux_state);
    LOG("aux_state_status : %d", rsp_dat->aux_state_status);
    LOG("progress_percent : %#x", rsp_dat->progress_percent);
    LOG("reason_code : %d", rsp_dat->reason_code);
    LOG("ud_option_flag_en : %#x", rsp_dat->ud_option_flag_en);
    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_FD_IS_UPDATE;
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
            LOG("RECV CMD : %#x\n", cmd);
            // LOG("pldm_fwup_gen prev state : %d, cur state : %d, event id : %d", gs_pldm_fwup_gen_state.prev_state, gs_pldm_fwup_gen_state.cur_state, gs_pldm_fwup_gen_state.event_id);  /* for debug */
    } else {
        LOG("ERR CMD : %#x\n", cmd);
    }
}