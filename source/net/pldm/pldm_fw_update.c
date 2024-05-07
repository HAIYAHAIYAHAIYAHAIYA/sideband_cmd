#include "pldm_fw_update.h"
#include "pldm.h"
#include "mctp.h"

pldm_fwup_base_info_t g_pldm_fwup_info;

static u32 gs_data_transfer_handle = 0;
static u8 gs_event_id = UNKNOWN;

static pldm_fwup_pkt_data_t gs_pkg_data_buf;
static pldm_fwup_pkt_data_t gs_fw_data_buf;

/* ------------------------ */
static u8 gs_store_data[256] = {0};
static u32 gs_id = 1;
/* ------------------------ */

static u8 gs_getpackagedata_progress = 0;
static u8 gs_want_to_end_update = 0;
static u8 gs_component_index = 0;
static u32 gs_requestfwdata_once_size = 0;

static u8 gs_progress_flag = 0;                           /* 0: in progress; 1: finish */
static u8 gs_timeout_occur_flag = 0;                      /* 0: timeout cause update_cancel */
static u32 log_filter_temp = 0;

static pldm_fwup_time_def_t gs_cal_times;

extern u8 g_pldm_need_rsp;
extern sys_ctrl_t g_sys;
extern pldm_controller_composite_state_sensor_data_struct_t controller_composite_state_sensors[5];

extern void pldm_unsupport_cmd(protocol_msg_t *pkt, int *pkt_len);
extern u32 crc32_pldm(u32 init_crc, u8 *data, u32 len);

static pldm_fwup_upgrade_func_t upgrade_funcs[] = {
    // [0] = {
    //     (pldm_fw_upgrade_callback *)CM_PLDM_FWUP_UPGRADE_SLOT_CALLBACK, 
    //     (pldm_fw_upgrade_complete_callback *)CM_PLDM_FWUP_UPGRADE_SLOT_COMPLETE_CALLBACK, 
    //     (pldm_fw_upgrade_cancer_callback *)CM_PLDM_FWUP_UPGRADE_SLOT_CANCER_CALLBACK
    //     },
    // [1] = {
    //     (pldm_fw_upgrade_callback *)CM_PLDM_FWUP_UPGRADE_CHIP_CALLBACK, 
    //     (pldm_fw_upgrade_complete_callback *)CM_PLDM_FWUP_UPGRADE_CHIP_COMPLETE_CALLBACK, 
    //     (pldm_fw_upgrade_cancer_callback *)CM_PLDM_FWUP_UPGRADE_CHIP_CANCER_CALLBACK
    //     },
    // [2] = {
    //     (pldm_fw_upgrade_callback *)CM_PLDM_FWUP_UPGRADE_FACT_CALLBACK, 
    //     NULL,
    //     NULL
    //     },
};

static void pldm_fwup_sta_chg(u8 cur_state)
{
    g_pldm_fwup_info.prev_state = g_pldm_fwup_info.cur_state;
    g_pldm_fwup_info.cur_state = cur_state;
}

static void pldm_fwup_firmware_process_quit(void)
{
    if (g_pldm_fwup_info.cur_state >= PLDM_UD_DOWNLOAD) {
        u8 comp_identifier = g_pldm_fwup_info.fw_new_ud_comp[gs_component_index - 1].comp_class_msg.comp_identifier;
        pldm_fw_upgrade_cancer_callback *upgrade_cancer_callback = upgrade_funcs[comp_identifier].upgrade_cancer_callback_func;
            if (upgrade_cancer_callback)
                (*upgrade_cancer_callback)(&gs_store_data, 0);
    }

    gs_progress_flag = 0;
    gs_timeout_occur_flag = 0;

    gs_id = 1;
    cm_memset(&gs_fw_data_buf, 0, 6);
    cm_memset(&gs_pkg_data_buf, 0, 6);

    gs_component_index = 0;

    gs_getpackagedata_progress = 0;
    gs_want_to_end_update = 0;

    gs_data_transfer_handle = 0;
    gs_requestfwdata_once_size = 0;
    gs_event_id = UNKNOWN;
    g_pldm_fwup_info.update_mode = NON_UPDATE_MODE;
    CM_PLDM_FWUP_END_UPDATE(log_filter_temp);
}

static void pldm_fwup_component_process_quit(void)
{
    gs_progress_flag = 0;

    gs_id = 1;
    cm_memset(&gs_fw_data_buf, 0, 6);
    cm_memset(&gs_pkg_data_buf, 0, 6);

    gs_data_transfer_handle = 0;
    gs_requestfwdata_once_size = g_pldm_fwup_info.max_transfer_size;
    gs_event_id = UNKNOWN;
    u8 comp_identifier = g_pldm_fwup_info.fw_new_ud_comp[gs_component_index - 1].comp_class_msg.comp_identifier;
    pldm_fw_upgrade_cancer_callback *upgrade_cancer_callback = upgrade_funcs[comp_identifier].upgrade_cancer_callback_func;
    if (upgrade_cancer_callback)
        (*upgrade_cancer_callback)(&gs_store_data, 0);
    CM_PLDM_FWUP_END_UPDATE(log_filter_temp);
}

static void pldm_fwup_querydeviceidentifiers(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_query_dev_identifier_rsp_dat_t *rsp_dat = (pldm_query_dev_identifier_rsp_dat_t *)(pkt->rsp_buf);

    rsp_dat->descriptor_cnt = 4;
    rsp_dat->descriptor.init_type = PLDM_PCI_VENDOR_ID;                                                           /* PCI Vendor ID */
    rsp_dat->descriptor.init_len  = sizeof(rsp_dat->descriptor.init_type);
    rsp_dat->descriptor.init_data = 0x8086;                                                                       /* 0x8086 */

    rsp_dat->descriptor.add_descriptor[0].add_type = PLDM_PCI_DEV_ID;
    rsp_dat->descriptor.add_descriptor[0].add_len  = sizeof(rsp_dat->descriptor.add_descriptor[0].add_data);      /* 2 byte */
    rsp_dat->descriptor.add_descriptor[0].add_data = REG(PLDM_PCI_DEV_ID_REG);

    rsp_dat->descriptor.add_descriptor[1].add_type = PLDM_PCI_SUBSYS_VENDOR_ID;
    rsp_dat->descriptor.add_descriptor[1].add_len  = sizeof(rsp_dat->descriptor.add_descriptor[1].add_data);      /* 2 byte */
    rsp_dat->descriptor.add_descriptor[1].add_data = 0x8086;

    rsp_dat->descriptor.add_descriptor[2].add_type = PLDM_PCI_SUBSYS_ID;
    rsp_dat->descriptor.add_descriptor[2].add_len  = sizeof(rsp_dat->descriptor.add_descriptor[2].add_data);      /* 2 byte */
    rsp_dat->descriptor.add_descriptor[2].add_data = REG(PLDM_PCI_SUBSYS_ID_REG);

    rsp_dat->descriptor.add_descriptor[3].add_type = PLDM_PCI_REVISION_ID;
    rsp_dat->descriptor.add_descriptor[3].add_len  = sizeof(rsp_dat->descriptor.add_descriptor[3].add_data) - 1;  /* 1 byte */
    rsp_dat->descriptor.add_descriptor[3].add_data = 0x01;

    rsp_dat->dev_identifier_len = sizeof(pldm_descriptor_t) + sizeof(pldm_add_descriptor_t) * rsp_dat->descriptor_cnt - 1;

    *pkt_len += rsp_dat->dev_identifier_len + sizeof(rsp_dat->descriptor_cnt) + sizeof(rsp_dat->dev_identifier_len);
}

static void pldm_fwup_getfirmwareparameters(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_get_fw_param_rsp_dat_t *rsp_dat = (pldm_get_fw_param_rsp_dat_t *)(pkt->rsp_buf);
    u8 comp_identifier[] = {PLDM_UD_SLOT, PLDM_UD_CHIP, PLDM_UD_FACTORY};
    /*  18-character ASCII-encoded buffer as CCCCCCCC.SSSSSSSS<nul>, 
        where CCCCCCCC and SSSSSSSS are the 32-bit ComponentComparisonStamp and security revision (lad_srev) respectively, 
        rendered in hex with leading zeros as needed, terminated by a null byte. (E810)*/
    char *comp_ver_str[] = {"CCCCCCCC.SSSSSSSS", "CCCCCCCC.SSSSSSSS", "CCCCCCCC.SSSSSSSS"};
    char *comp_img_set_ver_str = "N:nnnnnnnnO:ooooooooT:tttttttttt";
    /* BIT2 : Device host functionality will be reduced, perhaps becoming inaccessible, during Firmware Update. */
    rsp_dat->cap_during_ud = CBIT(2);
    rsp_dat->comp_cnt = 3;                                                                   /* SLOT, CHIP, FACTORY */
    rsp_dat->actv_comp_img_set_ver_str_type_and_len.comp_img_set_ver_str_type = PLDM_UD_TYPE_ASCII;
    rsp_dat->actv_comp_img_set_ver_str_type_and_len.comp_img_set_ver_str_len = 32;

    rsp_dat->pending_comp_img_set_ver_str_type_and_len.comp_img_set_ver_str_type = PLDM_UD_TYPE_ASCII;
    rsp_dat->pending_comp_img_set_ver_str_type_and_len.comp_img_set_ver_str_len = 32;

    cm_memcpy(rsp_dat->comp_img_set_ver_str.actv_comp_img_set_ver_str, comp_img_set_ver_str, 32);
    cm_memcpy(rsp_dat->comp_img_set_ver_str.pending_comp_img_set_ver_str, comp_img_set_ver_str, 32);

    for (u8 i = 0; i < rsp_dat->comp_cnt; i++) {
        rsp_dat->comp_param_table[i].comp_class_msg.comp_classification = PLDM_UD_SW_BUNDLE_CLASSIFICATION;
        rsp_dat->comp_param_table[i].comp_class_msg.comp_identifier = comp_identifier[i];
        rsp_dat->comp_param_table[i].comp_class_msg.comp_classification_idx = 0;                  /* not used */

        /* When ComponentOptions bit 1 is not set, this field should use the value of 0xFFFFFFFF. */
        rsp_dat->comp_param_table[i].actv_comp_ver_msg.comp_comparison_stamp = 0xFFFFFFFF;
        rsp_dat->comp_param_table[i].actv_comp_ver_msg.comp_ver_str_type = PLDM_UD_TYPE_ASCII;
        rsp_dat->comp_param_table[i].actv_comp_ver_msg.comp_ver_str_len = 18;
        cm_memset(rsp_dat->comp_param_table[i].actv_comp_release_date, 0, 8);

        rsp_dat->comp_param_table[i].pending_comp_ver_msg.comp_comparison_stamp = 0xFFFFFFFF;
        rsp_dat->comp_param_table[i].pending_comp_ver_msg.comp_ver_str_type = PLDM_UD_TYPE_ASCII;
        rsp_dat->comp_param_table[i].pending_comp_ver_msg.comp_ver_str_len = 18;
        cm_memset(rsp_dat->comp_param_table[i].pending_comp_release_date, 0, 8);

        rsp_dat->comp_param_table[i].comp_actv_meth = CBIT(1);                           /* “DC power cycle” */
        /* Bit 0 = 1: Firmware Device performs an ‘auto-apply’ during transfer phase and apply step will be completed immediately. */
        rsp_dat->comp_param_table[i].cap_during_ud = CBIT(0);
        cm_memcpy(rsp_dat->comp_param_table[i].comp_ver_str.actv_comp_ver_str, comp_ver_str[i], 18);
        cm_memcpy(rsp_dat->comp_param_table[i].comp_ver_str.pending_comp_ver_str, comp_ver_str[i], 18);
    }
    *pkt_len += sizeof(pldm_get_fw_param_rsp_dat_t) + sizeof(pldm_fwup_comp_param_table_t) * rsp_dat->comp_cnt;
}

static void pldm_fwup_requestupdate(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_fwup_req_update_req_dat_t *req_dat = (pldm_fwup_req_update_req_dat_t *)(pkt->req_buf);
    pldm_fwup_req_update_rsp_dat_t *rsp_dat = (pldm_fwup_req_update_rsp_dat_t *)(pkt->rsp_buf);
    mctp_hdr_t *req_mctp_hdr = (mctp_hdr_t *)(pkt->req_buf - sizeof(pldm_request_t) - sizeof(mctp_hdr_t));
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    if (g_pldm_fwup_info.update_mode == UPDATING_MODE || CM_IS_AT_UPGRADE_MODE()) {
        rsp_hdr->cpl_code = PLDM_UD_ALREADY_IN_UPDATE_MODE;
        return;
    }

    g_pldm_fwup_info.max_transfer_size = MIN(PLDM_RECVBUF_MAX_SIZE, req_dat->max_transfer_size);    /* max transfer size */
    g_pldm_fwup_info.update_mode = UPDATING_MODE;

    cm_memcpy(&(g_pldm_fwup_info.fw_new_ud_comp_img.comp_img_set_ver_str_type_and_len), &(req_dat->comp_img_set_ver_str_type_and_len), \
    sizeof(pldm_comp_img_set_ver_str_type_and_len_t) + req_dat->comp_img_set_ver_str_type_and_len.comp_img_set_ver_str_len);

    rsp_dat->fd_metadata_len = 0;
    if (req_dat->pkt_data_len != 0) {
        rsp_dat->fd_will_send_get_pkt_data_cmd = 0x02;
        rsp_dat->get_pkt_data_max_transfer_size = g_pldm_fwup_info.max_transfer_size;
        gs_pkg_data_buf.pkt_data_len = req_dat->pkt_data_len;
        *pkt_len += sizeof(pldm_fwup_req_update_rsp_dat_t);
        gs_event_id = PLDM_UD_ENTER_UD_WITH_PKTDATA;
    } else {
        rsp_dat->fd_will_send_get_pkt_data_cmd = 0x00;
        *pkt_len += sizeof(pldm_fwup_req_update_rsp_dat_t) - sizeof(rsp_dat->get_pkt_data_max_transfer_size);
        gs_event_id = PLDM_UD_ENTER_UD_NO_PKTDATA;
    }

    gs_requestfwdata_once_size = g_pldm_fwup_info.max_transfer_size;

    g_pldm_fwup_info.hw_id = pkt->mctp_hw_id;
    g_pldm_fwup_info.ua_eid = req_mctp_hdr->src_eid;
    log_filter_temp = CM_LOG_FILTER;
    CM_PLDM_FWUP_START_UPDATE();
    gs_cal_times.enter_upgrade_time = CM_GET_CUR_TIMER_MS();
}

static void pldm_fwup_getpackagedata_send(void)
{
    pldm_fwup_get_pkt_data_req_dat_t req_dat = {0};
    req_dat.data_transfer_handle = gs_data_transfer_handle;
    req_dat.transfer_op_flag = (gs_data_transfer_handle == 0) ? PLDM_TRANSFER_OP_FLAG_GET_FIRST_PART : PLDM_TRANSFER_OP_FLAG_GET_NEXT_PART;

    pldm_msg_send((u8 *)&req_dat, sizeof(pldm_fwup_get_pkt_data_req_dat_t), MCTP_PLDM_UPDATE, 0x11, g_pldm_fwup_info.hw_id, g_pldm_fwup_info.ua_eid);
}

static void pldm_fwup_getpackagedata_recv(protocol_msg_t *pkt, int *pkt_len)
{
    g_pldm_need_rsp = 0;
    pldm_fwup_get_pkt_data_rsp_dat_t *rsp_dat = (pldm_fwup_get_pkt_data_rsp_dat_t *)(pkt->req_buf);
    if (rsp_dat->cpl_code != MCTP_COMMAND_SUCCESS) {
        gs_data_transfer_handle = 0;
        LOG("pldm_fwup_getpackagedata_recv err, cpl_code : 0x%x !", rsp_dat->cpl_code);
        return;
    }

    u8 *buf = rsp_dat->portion_of_pkt_data;
    u16 offset = gs_pkg_data_buf.len;
    u16 remain_data_len = gs_pkg_data_buf.pkt_data_len - offset;
    u16 cpy_len = (remain_data_len <= g_pldm_fwup_info.max_transfer_size) ? remain_data_len : g_pldm_fwup_info.max_transfer_size;
    cm_memcpy(&(gs_pkg_data_buf.data[offset]), buf, cpy_len);
    gs_pkg_data_buf.len += cpy_len;

    if (gs_pkg_data_buf.len >= gs_pkg_data_buf.pkt_data_len) {
        u32 *crc32 = (u32 *)&(gs_pkg_data_buf.data[gs_pkg_data_buf.pkt_data_len - 4]);
        if (*crc32 != crc32_pldm(0xFFFFFFFFUL, gs_pkg_data_buf.data, gs_pkg_data_buf.pkt_data_len - 4)) {
            gs_want_to_end_update = 1;
        }
        gs_getpackagedata_progress = 1;
    } else {
        gs_data_transfer_handle = rsp_dat->next_data_transfer_handle;
        pldm_fwup_getpackagedata_send();
    }
    /* 考虑不接受UA传输的packsgedata，则置位gs_want_to_end_update，在下一个cmd的rsp_msg返回PACKAGE_DATA_ERROR并退出更新模式 */
}

static void pldm_fwup_passcomponenttable(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_fwup_pass_comp_table_req_dat_t *req_dat = (pldm_fwup_pass_comp_table_req_dat_t *)(pkt->req_buf);
    pldm_fwup_pass_comp_table_rsp_dat_t *rsp_dat = (pldm_fwup_pass_comp_table_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    /* The FD/FDP only expects this command in LEARN COMPONENTS state. */
    if ((gs_event_id == PLDM_UD_ENTER_UD_WITH_PKTDATA && gs_getpackagedata_progress == 0) || (g_pldm_fwup_info.cur_state != PLDM_UD_LEARN_COMP)) {
        rsp_hdr->cpl_code = PLDM_UD_INVALID_STATE_FOR_COMMAND;
        return;
    }

    if (gs_want_to_end_update == 1) {
        rsp_hdr->cpl_code = PLDM_UD_PACKAGE_DATA_ERROR;
        pldm_fwup_firmware_process_quit();
        return;
    }

    cm_memcpy(&g_pldm_fwup_info.fw_new_ud_comp[gs_component_index], &(req_dat->comp_class_msg), \
    sizeof(pldm_fwup_comp_class_msg_t) + sizeof(pldm_fwup_comp_ver_msg_t) + req_dat->comp_ver_msg.comp_ver_str_len);
    gs_component_index++;

    rsp_dat->comp_rsp = 0;
    rsp_dat->comp_rsp_code = 0;

    if (req_dat->transfer_flag == PLDM_TRANSFER_FLAG_END || req_dat->transfer_flag == PLDM_TRANSFER_FLAG_START_AND_END) {
        gs_event_id = PLDM_UD_PASSCOMP_END;
        LOG("component_index : %d", gs_component_index);
    }

    *pkt_len += sizeof(pldm_fwup_pass_comp_table_rsp_dat_t);
}

static void pldm_fwup_updatecomponent(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_fwup_update_comp_req_dat_t *req_dat = (pldm_fwup_update_comp_req_dat_t *)(pkt->req_buf);
    pldm_fwup_update_comp_rsp_dat_t *rsp_dat = (pldm_fwup_update_comp_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    /* The FD/FDP only expects this command in READY XFER state. */
    if (g_pldm_fwup_info.cur_state != PLDM_UD_READY_XFER) {
        rsp_hdr->cpl_code = PLDM_UD_INVALID_STATE_FOR_COMMAND;
        return;
    }

    rsp_dat->comp_compatibility_rsp = 0;             /* Component can be updated. */
    rsp_dat->comp_compatibility_rsp_code = 0;        /* No response code – used when component can be updated. */

    /* 组件信息验证 */
    int idx = 0;
    for (; idx < gs_component_index; idx++) {
        if (g_pldm_fwup_info.fw_new_ud_comp[idx].comp_class_msg.comp_classification_idx == req_dat->comp_class_msg.comp_classification_idx) {
            u32 new_ver_str_len = g_pldm_fwup_info.fw_new_ud_comp[idx].comp_ver_msg.comp_ver_str_type * g_pldm_fwup_info.fw_new_ud_comp[idx].comp_ver_msg.comp_ver_str_len;
            u32 req_ver_str_len = req_dat->comp_ver_str_type * req_dat->comp_ver_str_type_len;
            u32 stamp_result = g_pldm_fwup_info.fw_new_ud_comp[idx].comp_ver_msg.comp_comparison_stamp != req_dat->comp_comparison_stamp;
            int version_result = cm_memcmp(g_pldm_fwup_info.fw_new_ud_comp[idx].comp_ver_str, req_dat->comp_ver_str, new_ver_str_len);
            if ((stamp_result) || (new_ver_str_len != req_ver_str_len) || version_result) {
                rsp_dat->comp_compatibility_rsp = 1;
                rsp_dat->comp_compatibility_rsp_code = 3;   // Invalid component comparison stamp or version.
            }
            break;
        }
    }
    if (idx == gs_component_index) {
        rsp_dat->comp_compatibility_rsp = 1;
        rsp_dat->comp_compatibility_rsp_code = 8;       // Component cannot be updated as an Incomplete Component Image Set was received from the PassComponentTable commands
    }

    rsp_dat->ud_option_flag_en = 0;                     /* Force Update of component. If Bit 0 is set, firmware should allow downgrade only up to security version.*/
    rsp_dat->estimated_time_before_send_req_fw_data = 0;

    if (rsp_dat->comp_compatibility_rsp != 1) {
        gs_fw_data_buf.pkt_data_len = req_dat->comp_img_size;
        gs_event_id = PLDM_UD_UD_COMP_END;
        gs_data_transfer_handle = 0;
    }

    *pkt_len += sizeof(pldm_fwup_update_comp_rsp_dat_t);
}

static void pldm_fwup_requestfwdata_send(void)
{
    pldm_fwup_req_fw_data_req_dat_t req_dat = {0};
    req_dat.offset = gs_data_transfer_handle;
    req_dat.len = gs_requestfwdata_once_size;      /* Firmware should minimize the number of NVM Update commands by doing 4 KB updates.(E810) */

    pldm_msg_send((u8 *)&req_dat, sizeof(pldm_fwup_req_fw_data_req_dat_t), MCTP_PLDM_UPDATE, 0x15, g_pldm_fwup_info.hw_id, g_pldm_fwup_info.ua_eid);
}

static void pldm_fwup_transfercpl_send(u8 result)
{
    g_pldm_need_rsp = 0;
    pldm_fwup_transfer_cpl_req_dat_t req_dat = {0};
    req_dat.transfer_result = result;

    pldm_msg_send((u8 *)&req_dat, sizeof(pldm_fwup_transfer_cpl_req_dat_t), MCTP_PLDM_UPDATE, 0x16, g_pldm_fwup_info.hw_id, g_pldm_fwup_info.ua_eid);
}

static u8 transfer_result = 0;
FILE *pd = NULL;
static void pldm_fwup_requestfwdata_recv(protocol_msg_t *pkt, int *pkt_len)
{
    g_pldm_need_rsp = 0;
    pldm_fwup_req_fw_data_rsp_dat_t *rsp_dat = (pldm_fwup_req_fw_data_rsp_dat_t *)(pkt->req_buf);
    if (rsp_dat->cpl_code == MCTP_COMMAND_SUCCESS) {
        gs_id++;
        int upgrade_state = 0;
        u8 comp_identifier = g_pldm_fwup_info.fw_new_ud_comp[gs_component_index - 1].comp_class_msg.comp_identifier;
        pldm_fw_upgrade_callback *upgrade_callback = upgrade_funcs[comp_identifier].upgrade_callback_func;
        if (g_pldm_fwup_info.max_transfer_size != PLDM_RECVBUF_MAX_SIZE) {
            u16 need_len = PLDM_RECVBUF_MAX_SIZE - gs_fw_data_buf.len;
            u16 cpy_len = MIN(gs_requestfwdata_once_size, need_len);
            u16 remain_len = (gs_requestfwdata_once_size - need_len) ? gs_requestfwdata_once_size - need_len : 0;
            cm_memcpy(&(gs_fw_data_buf.data[gs_fw_data_buf.len]), rsp_dat->comp_img_option, cpy_len);
            gs_fw_data_buf.len += cpy_len;
            if (gs_fw_data_buf.len == PLDM_RECVBUF_MAX_SIZE) {
                if (upgrade_callback)
                    upgrade_state = (*upgrade_callback)(&gs_store_data, gs_id, gs_fw_data_buf.data, PLDM_RECVBUF_MAX_SIZE);
                if (remain_len)
                    cm_memcpy(gs_fw_data_buf.data, (u8 *)&(rsp_dat->comp_img_option[need_len]), remain_len);
                gs_fw_data_buf.len = remain_len;
                // pd = fopen("upgrade_pldm_fwup_slot.img", "rb");
                fwrite(gs_fw_data_buf.data, sizeof(u8), PLDM_RECVBUF_MAX_SIZE, pd);
            }
        } else {
            if (upgrade_callback)
                upgrade_state = (*upgrade_callback)(&gs_store_data, gs_id, (u8 *)(rsp_dat->comp_img_option), PLDM_RECVBUF_MAX_SIZE);
            // pd = fopen("upgrade_pldm_fwup_slot.img", "rb");
            fwrite(rsp_dat->comp_img_option, sizeof(u8), PLDM_RECVBUF_MAX_SIZE, pd);
        }
        upgrade_state = (upgrade_state > 0) ? -upgrade_state : upgrade_state;
        if (upgrade_state < 0) {  // flash erase failed
            transfer_result = 0x0D; // The FD/FDP has aborted the transfer due to an issue with storing the firmware data on the device.
            pldm_fwup_transfercpl_send(transfer_result);
            return;
        }

        gs_data_transfer_handle += gs_requestfwdata_once_size;
        u32 remain_data_len = gs_fw_data_buf.pkt_data_len - gs_data_transfer_handle;
        gs_requestfwdata_once_size = (remain_data_len <= g_pldm_fwup_info.max_transfer_size) ? remain_data_len : g_pldm_fwup_info.max_transfer_size;
        if (gs_data_transfer_handle >= gs_fw_data_buf.pkt_data_len) {
            /*  1 : Transfer has completed with error as the image received is corrupt.
                0 : Transfer has completed without error, no additional information on why is provided with this code. */
            transfer_result = (upgrade_state < 0) ? 1 : 0;
            pldm_fwup_transfercpl_send(transfer_result);
            gs_data_transfer_handle = 0;
            cm_memset(&gs_fw_data_buf, 0, 6);
            fclose(pd);
        } else {
            pldm_fwup_requestfwdata_send();
        }
    } else {
        LOG("pldm_fwup_requestfwdata_recv err, cpl_code : 0x%x !", rsp_dat->cpl_code);
    }
}

static u8 verify_result = 0;
static void pldm_fwup_verifycpl_send(void)
{
    g_pldm_need_rsp = 0;
    /* Verify has completed with error as the image failed the FD security checks */
    // verify_result = 3;
    pldm_fwup_verify_cpl_req_dat_t req_dat = {0};
    req_dat.verify_result = verify_result;
    pldm_msg_send((u8 *)&req_dat, sizeof(pldm_fwup_verify_cpl_req_dat_t), MCTP_PLDM_UPDATE, 0x17, g_pldm_fwup_info.hw_id, g_pldm_fwup_info.ua_eid);
}

static void pldm_fwup_applycpl_send(void)
{
    g_pldm_need_rsp = 0;
    pldm_fwup_apply_cpl_req_dat_t req_dat = {0};
    req_dat.apply_result = 0;                                              /* Apply has completed without error. */
    req_dat.comp_actv_meth_modification = CBIT(1);                         /* DC power cycle */

    pldm_msg_send((u8 *)&req_dat, sizeof(pldm_fwup_apply_cpl_req_dat_t), MCTP_PLDM_UPDATE, 0x18, g_pldm_fwup_info.hw_id, g_pldm_fwup_info.ua_eid);
}

static void pldm_fwup_transfercpl_recv(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_fwup_transfer_cpl_rsp_dat_t *rsp_dat = (pldm_fwup_transfer_cpl_rsp_dat_t *)(pkt->req_buf);
    if (rsp_dat->cpl_code != MCTP_COMMAND_SUCCESS) {
        LOG("transfer_cpl send err, cpl_code : 0x%x !", rsp_dat->cpl_code);
    } else {
        if (!transfer_result)
            gs_event_id = PLDM_UD_TRANSFER_PASS;
        gs_progress_flag = 1;
    }
}

static void pldm_fwup_verifycpl_recv(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_fwup_verify_cpl_rsp_dat_t *rsp_dat = (pldm_fwup_verify_cpl_rsp_dat_t *)(pkt->req_buf);
    if (rsp_dat->cpl_code != MCTP_COMMAND_SUCCESS) {
        LOG("verify_cpl send err, cpl_code : 0x%x !", rsp_dat->cpl_code);
    } else {
        if (!verify_result)
            gs_event_id = PLDM_UD_VERIFY_PASS;
        gs_progress_flag = 1;
    }
}

static void pldm_fwup_applycpl_recv(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_fwup_apply_cpl_rsp_dat_t *rsp_dat = (pldm_fwup_apply_cpl_rsp_dat_t *)(pkt->req_buf);
    if (rsp_dat->cpl_code != MCTP_COMMAND_SUCCESS) {
        LOG("apply_cpl send err, cpl_code : 0x%x !", rsp_dat->cpl_code);
    } else {
        gs_event_id = PLDM_UD_APPLY_PASS;
        gs_progress_flag = 1;
    }
}

static void pldm_fwup_activate_progress(void)
{
    /* Activate op */
    gs_event_id = PLDM_UD_ACTIVATE_DONE;
    gs_progress_flag = 1;
}

static void pldm_fwup_activatefw(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_fwup_actv_fw_req_dat_t *req_dat = (pldm_fwup_actv_fw_req_dat_t *)(pkt->req_buf);
    pldm_fwup_actv_fw_rsp_dat_t *rsp_dat = (pldm_fwup_actv_fw_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    if (g_pldm_fwup_info.cur_state != PLDM_UD_READY_XFER) {
        rsp_hdr->cpl_code = PLDM_UD_INVALID_STATE_FOR_COMMAND;
        goto L_ERR;
    }

    if (gs_event_id != PLDM_UD_APPLY_PASS) {
        rsp_hdr->cpl_code = PLDM_UD_INCOMPLETE_UPDATE;
        goto L_ERR;
    }

    /* 如果amber不支持自包含激活 */
    if (req_dat->self_contained_actv_req == true) { 
        rsp_hdr->cpl_code = PLDM_UD_SELF_CONTAINED_ACTIVATION_NOT_PERMITTED;
        g_pldm_fwup_info.cur_state = PLDM_UD_ACTIVATE;
        pldm_fwup_activate_progress();
        LOG("cpl_code : %#x", rsp_hdr->cpl_code);
    }

    /* 如果amber支持自包含激活 */
    // if (req_dat->self_contained_actv_req == true) { 
    //     /* 切换到active状态，自包含激活，激活成功后然后切换到IDLE状态，退出更新模式， */
    //     gs_event_id = PLDM_UD_ENTER_ACTIVATE;
    // }

    rsp_dat->estimated_time_for_self_contained_actv = 0;
    *pkt_len += sizeof(pldm_fwup_actv_fw_rsp_dat_t);

    u8 comp_identifier = g_pldm_fwup_info.fw_new_ud_comp[gs_component_index - 1].comp_class_msg.comp_identifier;
    pldm_fw_upgrade_complete_callback *upgrade_complete_callback = upgrade_funcs[comp_identifier].upgrade_complete_callback_func;
    if (upgrade_complete_callback)
        (*upgrade_complete_callback)(&gs_store_data, 0);
L_ERR:
    return;
}

static void pldm_fwup_getstatus(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_fwup_get_status_rsp_dat_t *rsp_dat = (pldm_fwup_get_status_rsp_dat_t *)(pkt->rsp_buf);

    rsp_dat->cur_state = g_pldm_fwup_info.cur_state;
    rsp_dat->prev_state = g_pldm_fwup_info.prev_state;
    if (g_pldm_fwup_info.cur_state <= PLDM_UD_READY_XFER) {
        if (g_pldm_fwup_info.cur_state == PLDM_UD_IDLE) {
            switch (g_pldm_fwup_info.prev_state) {
                case PLDM_UD_IDLE:
                    rsp_dat->reason_code = INITIALIZATION_OCCURRED;
                    break;

                case PLDM_UD_ACTIVATE:
                    rsp_dat->reason_code = ACTIVATEFIRMWARE_RECEIVED;
                    break;

                case PLDM_UD_LEARN_COMP:
                case PLDM_UD_READY_XFER:
                case PLDM_UD_DOWNLOAD:
                case PLDM_UD_VERIFY:
                case PLDM_UD_APPLY:
                    if (gs_timeout_occur_flag == 1)
                        rsp_dat->reason_code = g_pldm_fwup_info.prev_state + 2;
                    else
                        rsp_dat->reason_code = CANCELUPDATE_RECEIVED;
                    break;

                default:
                    break;
            }
        }
        rsp_dat->aux_state = PLDM_UD_OP_ELSE;
    } else if (g_pldm_fwup_info.cur_state <= PLDM_UD_ACTIVATE) {
        if (g_pldm_fwup_info.cur_state <= PLDM_UD_APPLY) {
            rsp_dat->progress_percent = 0x65;   /* 暂定不支持百分比 */
        }
        rsp_dat->aux_state = gs_progress_flag;
        rsp_dat->aux_state_status = 0x00;                   /* AuxState is In Progress or Success. */
    }
    rsp_dat->ud_option_flag_en = CBIT(0);
}

static void pldm_fwup_compupdate_cancel(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    if ((g_pldm_fwup_info.cur_state >= PLDM_UD_DOWNLOAD) && (g_pldm_fwup_info.cur_state <= PLDM_UD_APPLY)) {
        gs_event_id = PLDM_UD_CANCEL_UD_COMP;
    } else {
        rsp_hdr->cpl_code = PLDM_UD_INVALID_STATE_FOR_COMMAND;
    }
}

static void pldm_fwup_fwupdate_cancel(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_fwup_cancel_ud_rsp_dat_t *rsp_dat = (pldm_fwup_cancel_ud_rsp_dat_t *)(pkt->rsp_buf);

    rsp_dat->nonfunc_comp_indication = FALSE;
    rsp_dat->nonfunc_comp_bitmap = 0;

    gs_event_id = PLDM_UD_CANCEL_UD_OR_TIMEOUT;

    *pkt_len += sizeof(pldm_fwup_cancel_ud_rsp_dat_t);
}

static pldm_cmd_func pldm_cmd_table[PLDM_FW_UPDATE_CMD] =
{
    pldm_unsupport_cmd,                           /* 0x00 */
    pldm_fwup_querydeviceidentifiers,             /* 0x01 */
    pldm_fwup_getfirmwareparameters,              /* 0x02 */
    pldm_unsupport_cmd,                           /* 0x00 */
    pldm_unsupport_cmd,                           /* 0x00 */
    pldm_unsupport_cmd,                           /* 0x00 */
    pldm_unsupport_cmd,                           /* 0x00 */
    pldm_unsupport_cmd,                           /* 0x00 */
    pldm_unsupport_cmd,                           /* 0x00 */
    pldm_unsupport_cmd,                           /* 0x00 */
    pldm_unsupport_cmd,                           /* 0x00 */
    pldm_unsupport_cmd,                           /* 0x00 */
    pldm_unsupport_cmd,                           /* 0x00 */
    pldm_unsupport_cmd,                           /* 0x00 */
    pldm_unsupport_cmd,                           /* 0x00 */
    pldm_unsupport_cmd,                           /* 0x00 */
    pldm_fwup_requestupdate,                      /* 0x10 */
    pldm_fwup_getpackagedata_recv,                /* 0x11 */
    pldm_unsupport_cmd,                           /* 0x00 */
    pldm_fwup_passcomponenttable,                 /* 0x13 */
    pldm_fwup_updatecomponent,                    /* 0x14 */
    pldm_fwup_requestfwdata_recv,                 /* 0x15 */
    pldm_fwup_transfercpl_recv,                   /* 0x16 */
    pldm_fwup_verifycpl_recv,                     /* 0x17 */
    pldm_fwup_applycpl_recv,                      /* 0x18 */
    pldm_unsupport_cmd,                           /* 0x00 */
    pldm_fwup_activatefw,                         /* 0x1a */
    pldm_fwup_getstatus,                          /* 0x1b */
    pldm_fwup_compupdate_cancel,                  /* 0x1c */
    pldm_fwup_fwupdate_cancel                     /* 0x1d */
};

/* DSP0267 Figure 8 – Firmware Device State Transition Diagram */
pldm_fwup_state_transform_t pldm_fwup_transforms[] = {
    /* cur_state         event_id                      next_state          action */
    {PLDM_UD_IDLE,       PLDM_UD_ENTER_UD_WITH_PKTDATA,PLDM_UD_LEARN_COMP, pldm_fwup_getpackagedata_send},
    {PLDM_UD_IDLE,       PLDM_UD_ENTER_UD_NO_PKTDATA,  PLDM_UD_LEARN_COMP, NULL},
    {PLDM_UD_LEARN_COMP, PLDM_UD_PASSCOMP_END,         PLDM_UD_READY_XFER, NULL},
    {PLDM_UD_READY_XFER, PLDM_UD_UD_COMP_END,          PLDM_UD_DOWNLOAD,   pldm_fwup_requestfwdata_send},
    {PLDM_UD_DOWNLOAD,   PLDM_UD_TRANSFER_PASS,        PLDM_UD_VERIFY,     pldm_fwup_verifycpl_send},
    {PLDM_UD_VERIFY,     PLDM_UD_VERIFY_PASS,          PLDM_UD_APPLY,      pldm_fwup_applycpl_send},
    {PLDM_UD_APPLY,      PLDM_UD_APPLY_PASS,           PLDM_UD_READY_XFER, NULL},
    {PLDM_UD_READY_XFER, PLDM_UD_ENTER_ACTIVATE,       PLDM_UD_ACTIVATE,   pldm_fwup_activate_progress},
    {PLDM_UD_ACTIVATE,   PLDM_UD_ACTIVATE_DONE,        PLDM_UD_IDLE,       pldm_fwup_firmware_process_quit},

    {PLDM_UD_LEARN_COMP, PLDM_UD_CANCEL_UD_OR_TIMEOUT, PLDM_UD_IDLE,       pldm_fwup_firmware_process_quit},
    {PLDM_UD_READY_XFER, PLDM_UD_CANCEL_UD_OR_TIMEOUT, PLDM_UD_IDLE,       pldm_fwup_firmware_process_quit},
    {PLDM_UD_DOWNLOAD,   PLDM_UD_CANCEL_UD_OR_TIMEOUT, PLDM_UD_IDLE,       pldm_fwup_firmware_process_quit},
    {PLDM_UD_VERIFY,     PLDM_UD_CANCEL_UD_OR_TIMEOUT, PLDM_UD_IDLE,       pldm_fwup_firmware_process_quit},
    {PLDM_UD_APPLY,      PLDM_UD_CANCEL_UD_OR_TIMEOUT, PLDM_UD_IDLE,       pldm_fwup_firmware_process_quit},

    {PLDM_UD_DOWNLOAD,   PLDM_UD_CANCEL_UD_COMP,       PLDM_UD_READY_XFER, pldm_fwup_component_process_quit},
    {PLDM_UD_VERIFY,     PLDM_UD_CANCEL_UD_COMP,       PLDM_UD_READY_XFER, pldm_fwup_component_process_quit},
    {PLDM_UD_APPLY,      PLDM_UD_CANCEL_UD_COMP,       PLDM_UD_READY_XFER, pldm_fwup_component_process_quit}
};

void pldm_fwup_state_machine_switch(void)
{
    gs_timeout_occur_flag = 0;
    for (int i = 0; i < sizeof(pldm_fwup_transforms) / sizeof(pldm_fwup_transforms[0]); i++) {
        if (pldm_fwup_transforms[i].cur_state == g_pldm_fwup_info.cur_state && pldm_fwup_transforms[i].event_id == gs_event_id) {
            gs_progress_flag = 0;
            pldm_fwup_sta_chg(pldm_fwup_transforms[i].next_state);
            LOG("prev state : %d, cur state : %d, event id : %d", g_pldm_fwup_info.prev_state, g_pldm_fwup_info.cur_state, gs_event_id);  /* for debug */
            if (gs_event_id == PLDM_UD_UD_COMP_END) {
                pd = fopen(PLDM_FWUP_RECV_IMG_NAME, "w+b");
            }
            if (pldm_fwup_transforms[i].action != NULL) {
                pldm_fwup_transforms[i].action();
            }
            break;
        }
    }
}

void pldm_fwup_process(protocol_msg_t *pkt, int *pkt_len, u32 cmd_code)
{
    pldm_cmd_func cmd_proc = NULL;

    if (cmd_code < PLDM_FW_UPDATE_CMD) {
        cmd_proc = pldm_cmd_table[cmd_code];
    } else {
        cmd_proc = pldm_unsupport_cmd;
    }

    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    if ((g_pldm_fwup_info.update_mode == NON_UPDATE_MODE) && (g_pldm_fwup_info.cur_state != PLDM_UD_IDLE)) {
        rsp_hdr->cpl_code = PLDM_UD_NOT_IN_UPDATE_MODE;
        return;
    }
    gs_cal_times.req_time = CM_GET_CUR_TIMER_MS();
    cmd_proc(pkt, pkt_len);
}

void pldm_fwup_init(void)
{
    cm_memset(&g_pldm_fwup_info, 0, sizeof(pldm_fwup_base_info_t));
    cm_memset(&gs_fw_data_buf, 0, 6);
    cm_memset(&gs_pkg_data_buf, 0, 6);
    cm_memset(&gs_cal_times, 0, sizeof(pldm_fwup_time_def_t));
    // g_pldm_fwup_info.cur_state = PLDM_UD_IDLE;
    // g_pldm_fwup_info.prev_state = PLDM_UD_IDLE;
    // g_pldm_fwup_info.update_mode = FALSE;
}

static void pldm_fwup_timeout_process(void)
{
    if (!CM_IS_AT_UPGRADE_MODE() || g_pldm_fwup_info.cur_state == PLDM_UD_IDLE) return;
    /* ms */
    u64 cur_time = CM_GET_CUR_TIMER_MS();
    /* Implement “6.3.2 Requirements for Requesters” as specified in DSP0240. */
    u64 pt2_timeout = 4800;                 /* Time out waiting for a response -> min : 300ms, max : 4.8s */
    u64 upgrade_timeout = 15 * 60 * 1000;   /* upgrade must in 15 min.(E810) */
    u8 is_timeout = 0;

    if ((cur_time - gs_cal_times.req_time > pt2_timeout) && (gs_event_id <= PLDM_UD_APPLY_PASS)) {
        /* to be determind. */
        // is_timeout = 1;
        LOG("pldm fwup pt2_timeout : %lld, cur_state : %d", cur_time - gs_cal_times.req_time, g_pldm_fwup_info.cur_state);
    }
    if (cur_time - gs_cal_times.enter_upgrade_time > upgrade_timeout) {
        is_timeout = 1;
        LOG("pldm fwup upgrade_timeout : %lld", cur_time - gs_cal_times.enter_upgrade_time);
    }
    if (is_timeout) {
        gs_timeout_occur_flag = 1;
        pldm_fwup_firmware_process_quit();
    }
}

static void pldm_fwup_upgrade_is_detected_process(void)
{
    if (!CM_IS_AT_UPGRADE_MODE()) return;
    pldm_modify_state_datastruct(CONFIG_CHG, &controller_composite_state_sensors[2]);
    pldm_modify_state_datastruct(VER_CHG_DETECTED, &controller_composite_state_sensors[4]);
}

/* periodic monitoring in the task */
void pldm_fwup_upgrade_handle(void)
{
    pldm_fwup_upgrade_is_detected_process();
    pldm_fwup_timeout_process();
}