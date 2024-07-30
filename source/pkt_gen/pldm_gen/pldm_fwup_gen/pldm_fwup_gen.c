#include "pldm_fwup_gen.h"
#include "pkt_gen.h"

u8 b[9000];
pldm_gen_state_t gs_pldm_fwup_gen_state;

static void pldm_fwup_gen_cmd_01(u8 *buf)
{
    pldm_gen_req_hdr_update(buf, 0x01);
    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_NO_GET_FD_INDENTIFY;
}

static void pldm_fwup_gen_cmd_02(u8 *buf)
{
    pldm_gen_req_hdr_update(buf, 0x02);
}

static void pldm_fwup_gen_cmd_10(u8 *buf)
{
    pldm_gen_req_hdr_update(buf, 0x10);
    pldm_fwup_req_update_req_dat_t *req_dat = (pldm_fwup_req_update_req_dat_t *)buf;

    FILE *pd = NULL;
    // pd = fopen(PLDM_FWUP_IMG_NAME, "rb");
    pd = fopen(PLDM_FWUP_IMG_NAME, "rb");
    fread(&b, sizeof(u8), 1024, pd);
    fclose(pd);
    pldm_fwup_pkt_hdr_t *pkt_hdr = (pldm_fwup_pkt_hdr_t *)b;
    pldm_fwup_fw_dev_indentification_area_t *fw_dev_area = (pldm_fwup_fw_dev_indentification_area_t *)&(pkt_hdr->pkt_ver_str[pkt_hdr->pkt_ver_str_len]);
    pldm_fwup_fw_dev_id_records_first_part_t *fw_records_first_ptr = (pldm_fwup_fw_dev_id_records_first_part_t *)fw_dev_area->fw_dev_id_records;
    pldm_fwup_fw_dev_id_records_middle_part_t *fw_records_middle_ptr = (pldm_fwup_fw_dev_id_records_middle_part_t *)&(fw_records_first_ptr->comp_img_set_ver_str[fw_records_first_ptr->comp_img_set_ver_str_len]);
    pldm_add_descriptors_t *ptr = &(fw_records_middle_ptr->descriptor);
    for (u8 i = 0; i < fw_records_first_ptr->descriptor_cnt; i++) {
        ptr = (pldm_add_descriptors_t *)&(ptr->add_data[ptr->add_len]);
    }
    pldm_fwup_fw_dev_id_records_end_part_t *fw_records_end_ptr = (pldm_fwup_fw_dev_id_records_end_part_t *)ptr;
    pldm_fwup_comp_img_info_area_t *comp_img_area = (pldm_fwup_comp_img_info_area_t *)&(fw_records_end_ptr->fw_dev_pkt_data[fw_records_first_ptr->fw_dev_pkt_data_len]);
    pldm_fwup_comp_img_info_first_part_t *comp_first_part = (pldm_fwup_comp_img_info_first_part_t *)comp_img_area->comp_img_info;

    char *name = "test";
    req_dat->max_transfer_size = PLDM_FWUP_GEN_RECVBUF_MAX_SIZE;
    req_dat->num_of_comp = 1;
    req_dat->max_outstanding_transfer_req = 1;
    req_dat->pkt_data_len = fw_records_first_ptr->fw_dev_pkt_data_len;
    req_dat->comp_img_set_ver_str_type_and_len.type = PLDM_UD_TYPE_ASCII;
    req_dat->comp_img_set_ver_str_type_and_len.len = cm_strlen(name);
    cm_memcpy(req_dat->comp_img_set_ver_str, name, cm_strlen(name));

    if (req_dat->pkt_data_len) {
        gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_WITH_PKT_DATA;
    } else {
        gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_NO_PKT_DATA;
    }
}

void pldm_fwup_gen_cmd_11(u8 *buf)
{
    pldm_gen_req_hdr_update(buf, 0x11);
    pldm_fwup_get_pkt_data_rsp_dat_t *rsp_dat = (pldm_fwup_get_pkt_data_rsp_dat_t *)buf;
    rsp_dat->cpl_code = MCTP_COMMAND_SUCCESS;

    FILE *pd = NULL;
    // pd = fopen(PLDM_FWUP_IMG_NAME, "rb");
    pd = fopen(PLDM_FWUP_IMG_NAME, "rb");
    fread(&b, sizeof(u8), 1024, pd);
    fclose(pd);
    pldm_fwup_pkt_hdr_t *pkt_hdr = (pldm_fwup_pkt_hdr_t *)b;
    pldm_fwup_fw_dev_indentification_area_t *fw_dev_area = (pldm_fwup_fw_dev_indentification_area_t *)&(pkt_hdr->pkt_ver_str[pkt_hdr->pkt_ver_str_len]);
    pldm_fwup_fw_dev_id_records_first_part_t *fw_records_first_ptr = (pldm_fwup_fw_dev_id_records_first_part_t *)fw_dev_area->fw_dev_id_records;
    pldm_fwup_fw_dev_id_records_middle_part_t *fw_records_middle_ptr = (pldm_fwup_fw_dev_id_records_middle_part_t *)&(fw_records_first_ptr->comp_img_set_ver_str[fw_records_first_ptr->comp_img_set_ver_str_len]);
    pldm_add_descriptors_t *ptr = &(fw_records_middle_ptr->descriptor);
    for (u8 i = 0; i < fw_records_first_ptr->descriptor_cnt; i++) {
        ptr = (pldm_add_descriptors_t *)&(ptr->add_data[ptr->add_len]);
    }
    pldm_fwup_fw_dev_id_records_end_part_t *fw_records_end_ptr = (pldm_fwup_fw_dev_id_records_end_part_t *)ptr;

    u32 req_data_transfer_handle = pldm_fwup_gen_recv_get_pkt_data_req_dat();
    u32 remain_len = fw_records_first_ptr->fw_dev_pkt_data_len + req_data_transfer_handle;
    u16 cpy_len = remain_len > PLDM_FWUP_GEN_RECVBUF_MAX_SIZE ? PLDM_FWUP_GEN_RECVBUF_MAX_SIZE : remain_len;
    LOG("pkt data cpy_len : %d, remain_len : %d", cpy_len, remain_len);

    rsp_dat->next_data_transfer_handle = req_data_transfer_handle + cpy_len;
    rsp_dat->transfer_flag;

    pd = fopen(PLDM_FWUP_IMG_NAME, "rb");
    u32 offset = fw_records_end_ptr->fw_dev_pkt_data - b;
    fseek(pd, offset + req_data_transfer_handle, SEEK_SET);
    fread(&b, sizeof(u8), cpy_len, pd);
    fclose(pd);
    cm_memcpy(rsp_dat->portion_of_pkt_data, b, cpy_len);

    if (remain_len < PLDM_FWUP_GEN_RECVBUF_MAX_SIZE)
        gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_SEND_PKT_DATA_END;
}

static void pldm_fwup_gen_cmd_13(u8 *buf)
{
    pldm_gen_req_hdr_update(buf, 0x13);
    pldm_fwup_pass_comp_table_req_dat_t *req_dat = (pldm_fwup_pass_comp_table_req_dat_t *)buf;
    FILE *pd = NULL;
    // pd = fopen(PLDM_FWUP_IMG_NAME, "rb");
    pd = fopen(PLDM_FWUP_IMG_NAME, "rb");
    fread(&b, sizeof(u8), 1024, pd);
    fclose(pd);
    pldm_fwup_pkt_hdr_t *pkt_hdr = (pldm_fwup_pkt_hdr_t *)b;
    pldm_fwup_fw_dev_indentification_area_t *fw_dev_area = (pldm_fwup_fw_dev_indentification_area_t *)&(pkt_hdr->pkt_ver_str[pkt_hdr->pkt_ver_str_len]);
    pldm_fwup_fw_dev_id_records_first_part_t *fw_records_first_ptr = (pldm_fwup_fw_dev_id_records_first_part_t *)fw_dev_area->fw_dev_id_records;
    pldm_fwup_fw_dev_id_records_middle_part_t *fw_records_middle_ptr = (pldm_fwup_fw_dev_id_records_middle_part_t *)&(fw_records_first_ptr->comp_img_set_ver_str[fw_records_first_ptr->comp_img_set_ver_str_len]);
    pldm_add_descriptors_t *ptr = &(fw_records_middle_ptr->descriptor);
    for (u8 i = 0; i < fw_records_first_ptr->descriptor_cnt; i++) {
        ptr = (pldm_add_descriptors_t *)&(ptr->add_data[ptr->add_len]);
    }
    pldm_fwup_fw_dev_id_records_end_part_t *fw_records_end_ptr = (pldm_fwup_fw_dev_id_records_end_part_t *)ptr;
    pldm_fwup_comp_img_info_area_t *comp_img_area = (pldm_fwup_comp_img_info_area_t *)&(fw_records_end_ptr->fw_dev_pkt_data[fw_records_first_ptr->fw_dev_pkt_data_len]);
    pldm_fwup_comp_img_info_first_part_t *comp_first_part = (pldm_fwup_comp_img_info_first_part_t *)comp_img_area->comp_img_info;
    req_dat->transfer_flag = PLDM_TRANSFER_FLAG_START_AND_END;
    req_dat->comp_class_msg.comp_classification = comp_first_part->comp_classification;
    req_dat->comp_class_msg.comp_classification_idx = 0;
    req_dat->comp_class_msg.comp_identifier = comp_first_part->comp_identifier;

    req_dat->comp_ver_msg.comp_comparison_stamp = comp_first_part->comp_comparison_stamp;
    req_dat->comp_ver_msg.comp_ver_str_type = comp_first_part->comp_ver_str_type;
    req_dat->comp_ver_msg.comp_ver_str_len = comp_first_part->comp_ver_str_len;

    cm_memcpy(req_dat->comp_ver_str, comp_first_part->comp_ver_str, comp_first_part->comp_ver_str_len);
}

static void pldm_fwup_gen_cmd_14(u8 *buf)
{
    pldm_gen_req_hdr_update(buf, 0x14);
    pldm_fwup_update_comp_req_dat_t *req_dat = (pldm_fwup_update_comp_req_dat_t *)buf;

    FILE *pd = NULL;
    // pd = fopen(PLDM_FWUP_IMG_NAME, "rb");
    pd = fopen(PLDM_FWUP_IMG_NAME, "rb");
    fread(&b, sizeof(u8), 1024, pd);
    fclose(pd);
    pldm_fwup_pkt_hdr_t *pkt_hdr = (pldm_fwup_pkt_hdr_t *)b;
    pldm_fwup_fw_dev_indentification_area_t *fw_dev_area = (pldm_fwup_fw_dev_indentification_area_t *)&(pkt_hdr->pkt_ver_str[pkt_hdr->pkt_ver_str_len]);
    pldm_fwup_fw_dev_id_records_first_part_t *fw_records_first_ptr = (pldm_fwup_fw_dev_id_records_first_part_t *)fw_dev_area->fw_dev_id_records;
    pldm_fwup_fw_dev_id_records_middle_part_t *fw_records_middle_ptr = (pldm_fwup_fw_dev_id_records_middle_part_t *)&(fw_records_first_ptr->comp_img_set_ver_str[fw_records_first_ptr->comp_img_set_ver_str_len]);
    pldm_add_descriptors_t *ptr = &(fw_records_middle_ptr->descriptor);
    for (u8 i = 0; i < fw_records_first_ptr->descriptor_cnt; i++) {
        ptr = (pldm_add_descriptors_t *)&(ptr->add_data[ptr->add_len]);
    }
    pldm_fwup_fw_dev_id_records_end_part_t *fw_records_end_ptr = (pldm_fwup_fw_dev_id_records_end_part_t *)ptr;
    pldm_fwup_comp_img_info_area_t *comp_img_area = (pldm_fwup_comp_img_info_area_t *)&(fw_records_end_ptr->fw_dev_pkt_data[fw_records_first_ptr->fw_dev_pkt_data_len]);
    pldm_fwup_comp_img_info_first_part_t *comp_first_part = (pldm_fwup_comp_img_info_first_part_t *)comp_img_area->comp_img_info;

    req_dat->comp_class_msg.comp_classification = comp_first_part->comp_classification;
    req_dat->comp_class_msg.comp_classification_idx = 0;
    req_dat->comp_class_msg.comp_identifier = comp_first_part->comp_identifier;

    req_dat->comp_comparison_stamp = comp_first_part->comp_comparison_stamp;
    req_dat->comp_ver_str_type = comp_first_part->comp_ver_str_type;
    req_dat->comp_ver_str_type_len = comp_first_part->comp_ver_str_len;

    cm_memcpy(req_dat->comp_ver_str, comp_first_part->comp_ver_str, comp_first_part->comp_ver_str_len);

    req_dat->comp_img_size = comp_first_part->comp_size;
    req_dat->ud_option_flag = comp_first_part->comp_options;

    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_STILL_HAVE_IMG;
    // gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_SEND_UP_COMP_END;
}

void pldm_fwup_gen_cmd_15(u8 *buf)
{
    pldm_gen_req_hdr_update(buf, 0x15);
    pldm_fwup_req_fw_data_rsp_dat_t *rsp_dat = (pldm_fwup_req_fw_data_rsp_dat_t *)buf;
    rsp_dat->cpl_code = MCTP_COMMAND_SUCCESS;

    pldm_fwup_req_fw_data_req_dat_t req_dat = pldm_fwup_gen_recv_get_fw_data_req_dat();

    FILE *pd = NULL;
    // pd = fopen(PLDM_FWUP_IMG_NAME, "rb");
    pd = fopen(PLDM_FWUP_IMG_NAME, "rb");
    fread(&b, sizeof(u8), 1024, pd);
    fclose(pd);
    pldm_fwup_pkt_hdr_t *pkt_hdr = (pldm_fwup_pkt_hdr_t *)b;
    pldm_fwup_fw_dev_indentification_area_t *fw_dev_area = (pldm_fwup_fw_dev_indentification_area_t *)&(pkt_hdr->pkt_ver_str[pkt_hdr->pkt_ver_str_len]);
    pldm_fwup_fw_dev_id_records_first_part_t *fw_records_first_ptr = (pldm_fwup_fw_dev_id_records_first_part_t *)fw_dev_area->fw_dev_id_records;
    pldm_fwup_fw_dev_id_records_middle_part_t *fw_records_middle_ptr = (pldm_fwup_fw_dev_id_records_middle_part_t *)&(fw_records_first_ptr->comp_img_set_ver_str[fw_records_first_ptr->comp_img_set_ver_str_len]);
    pldm_add_descriptors_t *ptr = &(fw_records_middle_ptr->descriptor);
    for (u8 i = 0; i < fw_records_first_ptr->descriptor_cnt; i++) {
        ptr = (pldm_add_descriptors_t *)&(ptr->add_data[ptr->add_len]);
    }
    pldm_fwup_fw_dev_id_records_end_part_t *fw_records_end_ptr = (pldm_fwup_fw_dev_id_records_end_part_t *)ptr;
    pldm_fwup_comp_img_info_area_t *comp_img_area = (pldm_fwup_comp_img_info_area_t *)&(fw_records_end_ptr->fw_dev_pkt_data[fw_records_first_ptr->fw_dev_pkt_data_len]);
    pldm_fwup_comp_img_info_first_part_t *comp_first_part = (pldm_fwup_comp_img_info_first_part_t *)comp_img_area->comp_img_info;

    u32 offset_base = comp_first_part->comp_local_offset;
    pd = fopen(PLDM_FWUP_IMG_NAME, "rb");
    fseek(pd, offset_base + req_dat.offset, SEEK_SET);

    u32 remain_len = comp_first_part->comp_size - req_dat.offset;
    u16 cpy_len = remain_len > PLDM_FWUP_GEN_RECVBUF_MAX_SIZE ? PLDM_FWUP_GEN_RECVBUF_MAX_SIZE : remain_len;
    LOG("fw data cpy_len : %d, remain_len : %d", cpy_len, remain_len);

    fread(&b, sizeof(u8), cpy_len, pd);
    fclose(pd);
    cm_memcpy(rsp_dat->comp_img_option, b, cpy_len);
    // gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_SEND_FW_DATA;
}

static void pldm_fwup_gen_cmd_16(u8 *buf)
{
    pldm_gen_req_hdr_update(buf, 0x16);
    pldm_fwup_transfer_cpl_rsp_dat_t *rsp_dat = (pldm_fwup_transfer_cpl_rsp_dat_t *)buf;
    rsp_dat->cpl_code = MCTP_COMMAND_SUCCESS;
    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_TRANS_FW_DATA_END;
}

static void pldm_fwup_gen_cmd_17(u8 *buf)
{
    pldm_gen_req_hdr_update(buf, 0x17);
    pldm_fwup_verify_cpl_rsp_dat_t *rsp_dat = (pldm_fwup_verify_cpl_rsp_dat_t *)buf;
    rsp_dat->cpl_code = MCTP_COMMAND_SUCCESS;
    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_FD_VERIFY_END;
}

static void pldm_fwup_gen_cmd_18(u8 *buf)
{
    pldm_gen_req_hdr_update(buf, 0x18);
    pldm_fwup_apply_cpl_rsp_dat_t *rsp_dat = (pldm_fwup_apply_cpl_rsp_dat_t *)buf;
    rsp_dat->cpl_code = MCTP_COMMAND_SUCCESS;
    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_FD_APPLY_END;
}

static void pldm_fwup_gen_cmd_1a(u8 *buf)
{
    LOG("pldm_fwup_gen_cmd_1a");
    pldm_gen_req_hdr_update(buf, 0x1a);
    pldm_fwup_actv_fw_req_dat_t *req_dat = (pldm_fwup_actv_fw_req_dat_t *)buf;
    req_dat->self_contained_actv_req = true;
    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_NO_IMG;
}

static void pldm_fwup_gen_cmd_1b(u8 *buf)
{
    pldm_gen_req_hdr_update(buf, 0x1b);
    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_GET_STATUS;
}

static void pldm_fwup_gen_cmd_1c(u8 *buf)
{
    pldm_gen_req_hdr_update(buf, 0x1c);
    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_COMP_CANCEL_OR_TIMEOUT;
}

static void pldm_fwup_gen_cmd_1d(u8 *buf)
{
    pldm_gen_req_hdr_update(buf, 0x1d);
    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_IMG_CANCEL_OR_TIMEOUT;
}

void pldm_fwup_gen(int cmd, u8 *buf)
{
    gen_cmd cmds[] = {
        gen_cmd_unsupport,
        pldm_fwup_gen_cmd_01,
        pldm_fwup_gen_cmd_02,
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
        pldm_fwup_gen_cmd_10,
        pldm_fwup_gen_cmd_11,
        gen_cmd_unsupport,
        pldm_fwup_gen_cmd_13,
        pldm_fwup_gen_cmd_14,
        pldm_fwup_gen_cmd_15,
        pldm_fwup_gen_cmd_16,
        pldm_fwup_gen_cmd_17,
        pldm_fwup_gen_cmd_18,
        gen_cmd_unsupport,
        pldm_fwup_gen_cmd_1a,
        pldm_fwup_gen_cmd_1b,
        pldm_fwup_gen_cmd_1c,
        pldm_fwup_gen_cmd_1d,
    };

    if (cmd < PLDM_FW_UPDATE_CMD) {
        if (cmds[cmd])
            cmds[cmd](buf);
    } else {
        gen_cmd_unsupport(buf);
    }
}

static void pldm_fwup_gen_clean_param(u8 *buf)
{
    // gs_pldm_fwup_gen_state.cur_state = PLDM_FWUP_GEN_IDLE;
    // gs_pldm_fwup_gen_state.prev_state = PLDM_FWUP_GEN_IDLE;
    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_UNKNOW;
}

pkt_gen_state_transform_t pldm_fwup_state_transform[] = {
    {PLDM_FWUP_GEN_IDLE,        PLDM_FWUP_GEN_NO_GET_FD_INDENTIFY,         PLDM_FWUP_GEN_IDLE,       PLDM_FWUP_CMD(01)},
    {PLDM_FWUP_GEN_IDLE,        PLDM_FWUP_GEN_GET_FD_INDENTIFY,            PLDM_FWUP_GEN_IDLE,       PLDM_FWUP_CMD(02)},
    {PLDM_FWUP_GEN_IDLE,        PLDM_FWUP_GEN_GET_FD_PARAM_AND_NOT_UPDATE, PLDM_FWUP_GEN_LEARN_COMP, PLDM_FWUP_CMD(10)},

    {PLDM_FWUP_GEN_LEARN_COMP,  PLDM_FWUP_GEN_WITH_PKT_DATA,               PLDM_FWUP_GEN_LEARN_COMP, NULL},
    {PLDM_FWUP_GEN_LEARN_COMP,  PLDM_FWUP_GEN_SEND_PKT_DATA,               PLDM_FWUP_GEN_LEARN_COMP, PLDM_FWUP_CMD(11)},
    {PLDM_FWUP_GEN_LEARN_COMP,  PLDM_FWUP_GEN_NO_PKT_DATA,                 PLDM_FWUP_GEN_LEARN_COMP, PLDM_FWUP_CMD(13)},
    {PLDM_FWUP_GEN_LEARN_COMP,  PLDM_FWUP_GEN_SEND_PKT_DATA_END,           PLDM_FWUP_GEN_LEARN_COMP, PLDM_FWUP_CMD(13)},

    {PLDM_FWUP_GEN_LEARN_COMP,  PLDM_FWUP_GEN_SEND_PASS_COMP_END,          PLDM_FWUP_GEN_DOWNLOAD,   PLDM_FWUP_CMD(14)},
    // {PLDM_FWUP_GEN_DOWNLOAD,    PLDM_FWUP_GEN_SEND_FW_DATA,                PLDM_FWUP_GEN_DOWNLOAD,     NULL},
    {PLDM_FWUP_GEN_DOWNLOAD,    PLDM_FWUP_GEN_SEND_FW_DATA,                PLDM_FWUP_GEN_DOWNLOAD,   PLDM_FWUP_CMD(15)},

    // {PLDM_FWUP_GEN_DOWNLOAD,    PLDM_FWUP_GEN_TRANS_FW_DATA_END,           PLDM_FWUP_GEN_VERIFY,     PLDM_FWUP_CMD(16)},
    // {PLDM_FWUP_GEN_VERIFY,      PLDM_FWUP_GEN_FD_VERIFY_END,               PLDM_FWUP_GEN_APPLY,      PLDM_FWUP_CMD(17)},
    // {PLDM_FWUP_GEN_APPLY,       PLDM_FWUP_GEN_FD_APPLY_END,                PLDM_FWUP_GEN_READY_XFER, PLDM_FWUP_CMD(18)},

    // {PLDM_FWUP_GEN_READY_XFER,  PLDM_FWUP_GEN_NO_IMG,                      PLDM_FWUP_GEN_ACTIVATE,   PLDM_FWUP_CMD(1a)},
    // {PLDM_FWUP_GEN_READY_XFER,  PLDM_FWUP_GEN_STILL_HAVE_IMG,              PLDM_FWUP_GEN_DOWNLOAD,   PLDM_FWUP_CMD(14)},

    {PLDM_FWUP_GEN_DOWNLOAD,    PLDM_FWUP_GEN_TRANS_FW_DATA_END,           PLDM_FWUP_GEN_VERIFY,     NULL},
    {PLDM_FWUP_GEN_VERIFY,      PLDM_FWUP_GEN_FD_VERIFY_END,               PLDM_FWUP_GEN_APPLY,      NULL},
    {PLDM_FWUP_GEN_APPLY,       PLDM_FWUP_GEN_FD_APPLY_END,                PLDM_FWUP_GEN_READY_XFER, NULL},

    {PLDM_FWUP_GEN_READY_XFER,  PLDM_FWUP_GEN_NO_IMG,                      PLDM_FWUP_GEN_ACTIVATE,   NULL},
    {PLDM_FWUP_GEN_READY_XFER,  PLDM_FWUP_GEN_STILL_HAVE_IMG,              PLDM_FWUP_GEN_DOWNLOAD,   NULL},

    {PLDM_FWUP_GEN_ACTIVATE,    PLDM_FWUP_GEN_SEND_ACTIV_CMD,              PLDM_FWUP_GEN_ACTIVATE,   PLDM_FWUP_CMD(1b)},
    {PLDM_FWUP_GEN_ACTIVATE,    PLDM_FWUP_GEN_FD_IS_UPDATE,                PLDM_FWUP_GEN_IDLE,       pldm_fwup_gen_clean_param},

    {PLDM_FWUP_GEN_DOWNLOAD,    PLDM_FWUP_GEN_GET_STATUS,                  PLDM_FWUP_GEN_DOWNLOAD,   NULL},
    {PLDM_FWUP_GEN_VERIFY,      PLDM_FWUP_GEN_GET_STATUS,                  PLDM_FWUP_GEN_VERIFY,     NULL},
    {PLDM_FWUP_GEN_APPLY,       PLDM_FWUP_GEN_GET_STATUS,                  PLDM_FWUP_GEN_APPLY,      NULL},
    {PLDM_FWUP_GEN_READY_XFER,  PLDM_FWUP_GEN_GET_STATUS,                  PLDM_FWUP_GEN_READY_XFER, NULL},
    {PLDM_FWUP_GEN_ACTIVATE,    PLDM_FWUP_GEN_GET_STATUS,                  PLDM_FWUP_GEN_ACTIVATE,   NULL},

    {PLDM_FWUP_GEN_DOWNLOAD,    PLDM_FWUP_GEN_IMG_CANCEL_OR_TIMEOUT,       PLDM_FWUP_GEN_IDLE,       pldm_fwup_gen_clean_param},
    {PLDM_FWUP_GEN_VERIFY,      PLDM_FWUP_GEN_IMG_CANCEL_OR_TIMEOUT,       PLDM_FWUP_GEN_IDLE,       pldm_fwup_gen_clean_param},
    {PLDM_FWUP_GEN_APPLY,       PLDM_FWUP_GEN_IMG_CANCEL_OR_TIMEOUT,       PLDM_FWUP_GEN_IDLE,       pldm_fwup_gen_clean_param},
    {PLDM_FWUP_GEN_READY_XFER,  PLDM_FWUP_GEN_IMG_CANCEL_OR_TIMEOUT,       PLDM_FWUP_GEN_IDLE,       pldm_fwup_gen_clean_param},
    {PLDM_FWUP_GEN_ACTIVATE,    PLDM_FWUP_GEN_IMG_CANCEL_OR_TIMEOUT,       PLDM_FWUP_GEN_IDLE,       pldm_fwup_gen_clean_param},

    {PLDM_FWUP_GEN_DOWNLOAD,    PLDM_FWUP_GEN_COMP_CANCEL_OR_TIMEOUT,      PLDM_FWUP_GEN_READY_XFER, NULL},
    {PLDM_FWUP_GEN_VERIFY,      PLDM_FWUP_GEN_COMP_CANCEL_OR_TIMEOUT,      PLDM_FWUP_GEN_READY_XFER, NULL},
    {PLDM_FWUP_GEN_APPLY,       PLDM_FWUP_GEN_COMP_CANCEL_OR_TIMEOUT,      PLDM_FWUP_GEN_READY_XFER, NULL},
};

void pldm_fwup_gen_init(void)
{
    gs_pldm_fwup_gen_state.cur_state = PLDM_FWUP_GEN_IDLE;
    gs_pldm_fwup_gen_state.prev_state = PLDM_FWUP_GEN_IDLE;
    gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_UNKNOW;
}

u8 pldm_fwup_state_transform_switch(u8 cnt, u8 *buf)
{
    u8 ret = 0xFF;
    for (u8 i = 0; i < sizeof(pldm_fwup_state_transform) / sizeof(pkt_gen_state_transform_t); i++) {
        if ((gs_pldm_fwup_gen_state.cur_state == pldm_fwup_state_transform[i].cur_state) && (gs_pldm_fwup_gen_state.event_id == pldm_fwup_state_transform[i].event_id)) {
            gs_pldm_fwup_gen_state.prev_state = gs_pldm_fwup_gen_state.cur_state;
            gs_pldm_fwup_gen_state.cur_state = pldm_fwup_state_transform[i].next_state;
            LOG("pldm_fwup_gen prev state : %d, cur state : %d, event id : %d", gs_pldm_fwup_gen_state.prev_state, gs_pldm_fwup_gen_state.cur_state, gs_pldm_fwup_gen_state.event_id);  /* for debug */
            if (gs_pldm_fwup_gen_state.event_id == PLDM_FWUP_GEN_SEND_FW_DATA)
                gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_SEND_FW_DATA_PAUSE;

            // if (gs_pldm_fwup_gen_state.event_id == PLDM_FWUP_GEN_FD_APPLY_END)
                // gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_NO_IMG;

            if (gs_pldm_fwup_gen_state.event_id == PLDM_FWUP_GEN_SEND_PKT_DATA)
                gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_WITH_PKT_DATA;

            if (gs_pldm_fwup_gen_state.event_id == PLDM_FWUP_GEN_SEND_UP_COMP_END) {
                gs_pldm_fwup_gen_state.event_id = PLDM_FWUP_GEN_SEND_UP_COMP_END_PAUSE;
            }

            if (gs_pldm_fwup_gen_state.event_id == PLDM_FWUP_GEN_FD_IS_UPDATE)
                ret = 0;
            else
                ret = 1;
            if (pldm_fwup_state_transform[i].action != NULL) {
                pldm_fwup_state_transform[i].action(buf);
            }
            break;
        }
    }
    return ret;
}