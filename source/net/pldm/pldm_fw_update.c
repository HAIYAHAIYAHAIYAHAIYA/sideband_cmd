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
static u32 gs_requestfwdata_once_size = 0;

static u8 gs_progress_flag = 0;                           /* 0: in progress; 1: finish */
static u8 gs_timeout_occur_flag = 0;                      /* 0: timeout cause update_cancel */

static pldm_fwup_time_def_t gs_cal_times;

FILE *pd = NULL;

extern u8 g_pldm_need_rsp;
extern pldm_controller_composite_state_sensor_data_struct_t controller_composite_state_sensors[5];

extern void pldm_unsupport_cmd(protocol_msg_t *pkt, int *pkt_len);
extern u32 crc32_pldm(u32 init_crc, u8 *data, u32 len);

extern void CM_FLASH_READ(u32 offset, u32 *buf, u32 size);
extern sts_t CM_FALSH_WRITE(u32 offset, u32 *buf, u32 size);

/* to be determind */
static pldm_fwup_upgrade_func_t upgrade_funcs[] = {
    [PLDM_UD_SLOT] = {
        NULL,
        NULL,
        NULL
        },
    [PLDM_UD_CHIP] = {
        NULL,
        NULL,
        NULL
        },
    [PLDM_UD_FACTORY] = {
        NULL,
        NULL,
        NULL
        },
};

static void pldm_fwup_sta_chg(u8 cur_state)
{
    g_pldm_fwup_info.prev_state = g_pldm_fwup_info.cur_state;
    g_pldm_fwup_info.cur_state = cur_state;
}

static void pldm_fwup_firmware_process_quit(void)
{
    if (g_pldm_fwup_info.cur_state >= PLDM_UD_DOWNLOAD) {
        u8 comp_identifier = g_pldm_fwup_info.fw_new_img_info.comp_identifier;
        PLDM_FW_UPDATE_CANCER_CALLBACK(comp_identifier);
    }

    gs_progress_flag = 0;
    gs_timeout_occur_flag = 0;

    gs_id = 1;
    cm_memset(&gs_fw_data_buf, 0, 6);
    cm_memset(&gs_pkg_data_buf, 0, 6);
    cm_memset(&g_pldm_fwup_info.fw_new_img_info, 0, sizeof(g_pldm_fwup_info.fw_new_img_info));
    cm_memset(&g_pldm_fwup_info.fw_new_set_info, 0, sizeof(g_pldm_fwup_info.fw_new_set_info));

    gs_getpackagedata_progress = 0;
    gs_want_to_end_update = 0;

    gs_data_transfer_handle = 0;
    gs_requestfwdata_once_size = 0;
    gs_event_id = UNKNOWN;
    g_pldm_fwup_info.update_mode = NON_UPDATE_MODE;
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
    u8 comp_identifier = g_pldm_fwup_info.fw_new_img_info.comp_identifier;
    PLDM_FW_UPDATE_CANCER_CALLBACK(comp_identifier);
}

static void pldm_fwup_querydeviceidentifiers(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_query_dev_identifier_rsp_dat_t *rsp_dat = (pldm_query_dev_identifier_rsp_dat_t *)(pkt->rsp_buf);

    rsp_dat->descriptor_cnt = 5;
    rsp_dat->descriptor.init_type = PLDM_PCI_VENDOR_ID;                                                           /* PCI Vendor ID */
    rsp_dat->descriptor.init_len  = sizeof(rsp_dat->descriptor.init_data);
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

    rsp_dat->dev_identifier_len = sizeof(pldm_add_descriptor_t) * rsp_dat->descriptor_cnt - 1;

    *pkt_len += rsp_dat->dev_identifier_len + sizeof(rsp_dat->descriptor_cnt) + sizeof(rsp_dat->dev_identifier_len);
}

static void pldm_fwup_getfirmwareparameters(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_get_fw_param_rsp_dat_t *rsp_dat = (pldm_get_fw_param_rsp_dat_t *)(pkt->rsp_buf);
    /* BIT2 : Device host functionality will be reduced, perhaps becoming inaccessible, during Firmware Update. */
    rsp_dat->cap_during_ud = CBIT(2) | CBIT(3) | CBIT(1);       // bit8
    rsp_dat->actv_comp_img_set_ver_str_type_and_len = g_pldm_fwup_info.fw_active_set_info.comp_img_set_ver_str_type_and_len;
    u32 str_len = rsp_dat->actv_comp_img_set_ver_str_type_and_len.len;
    cm_memcpy(rsp_dat->comp_img_set_ver_str, g_pldm_fwup_info.fw_active_set_info.comp_img_ver_str, str_len);

    u32 offset = str_len;
    if (g_pldm_fwup_info.pending_img_state) {
        rsp_dat->pending_comp_img_set_ver_str_type_and_len = g_pldm_fwup_info.fw_pending_set_info.comp_img_set_ver_str_type_and_len;
        str_len = rsp_dat->pending_comp_img_set_ver_str_type_and_len.len;
        cm_memcpy(&rsp_dat->comp_img_set_ver_str[offset], g_pldm_fwup_info.fw_pending_set_info.comp_img_ver_str, str_len);
        offset += str_len;
    } else {
        rsp_dat->pending_comp_img_set_ver_str_type_and_len.type = PLDM_UD_TYPE_UNKNOW;
        rsp_dat->pending_comp_img_set_ver_str_type_and_len.len = 0;
    }

    pldm_fwup_comp_param_table_t *comp_parm_table = (pldm_fwup_comp_param_table_t *)&(rsp_dat->comp_img_set_ver_str[offset]);
    for (u8 i = 0; i <= PLDM_UD_FACTORY; i++) {
        if (((g_pldm_fwup_info.active_img_state >> i) & 0x1) == 0)
            continue;

        rsp_dat->comp_cnt++;
        comp_parm_table->comp_class_msg.comp_classification = g_pldm_fwup_info.fw_cur_img_info[i].comp_classification;  /* DSP0267 953-959*/
        comp_parm_table->comp_class_msg.comp_identifier = g_pldm_fwup_info.fw_cur_img_info[i].comp_identifier;
        comp_parm_table->comp_class_msg.comp_classification_idx = g_pldm_fwup_info.fw_cur_img_info[i].comp_classification_idx;

        /* When ComponentOptions bit 1 is not set, this field should use the value of 0xFFFFFFFF. */
        comp_parm_table->actv_comp_ver_msg.comp_comparison_stamp = 0x0; // If the firmware component does not provide a component comparison stamp, this value should be set to 0x00000000
        comp_parm_table->actv_comp_ver_msg.comp_ver_str_type = g_pldm_fwup_info.fw_cur_img_info[i].comp_ver_str_type;
        comp_parm_table->actv_comp_ver_msg.comp_ver_str_len = g_pldm_fwup_info.fw_cur_img_info[i].comp_ver_str_len;
        cm_memset(comp_parm_table->actv_comp_release_date, 0, 8);
        u8 *component_ver_str = comp_parm_table->comp_ver_str;
        cm_memcpy(component_ver_str, g_pldm_fwup_info.fw_cur_img_info[i].comp_ver_str, g_pldm_fwup_info.fw_cur_img_info[i].comp_ver_str_len);
        offset += (sizeof(pldm_fwup_comp_param_table_t) + g_pldm_fwup_info.fw_cur_img_info[i].comp_ver_str_len);
        // rsp_data_len += ((sizeof(rsp_dat->comp_param_table[i].actv_comp_release_date) + rsp_dat->comp_param_table[i].pending_comp_ver_msg.comp_ver_str_len) * 2);

        comp_parm_table->pending_comp_ver_msg.comp_comparison_stamp = 0x0;
        cm_memset(comp_parm_table->pending_comp_release_date, 0, 8);
        if ((g_pldm_fwup_info.pending_img_state >> i) & 0x1) {
            comp_parm_table->pending_comp_ver_msg.comp_ver_str_type = g_pldm_fwup_info.fw_pending_img_info[i].comp_ver_str_type;
            comp_parm_table->pending_comp_ver_msg.comp_ver_str_len = g_pldm_fwup_info.fw_pending_img_info[i].comp_ver_str_len;
            cm_memcpy(&component_ver_str[g_pldm_fwup_info.fw_cur_img_info[i].comp_ver_str_len], g_pldm_fwup_info.fw_pending_img_info[i].comp_ver_str, g_pldm_fwup_info.fw_pending_img_info[i].comp_ver_str_len);
            offset += g_pldm_fwup_info.fw_pending_img_info[i].comp_ver_str_len;
        } else {
            comp_parm_table->pending_comp_ver_msg.comp_ver_str_type = PLDM_UD_TYPE_UNKNOW;
            comp_parm_table->pending_comp_ver_msg.comp_ver_str_len = 0;
        }

        comp_parm_table->comp_actv_meth = g_pldm_fwup_info.fw_cur_img_info[i].comp_actv_meth;  // TBD

        /* Bit 0 = 1: Firmware Device performs an ‘auto-apply’ during transfer phase and apply step will be completed immediately. */
        comp_parm_table->cap_during_ud = CBIT(0);
        comp_parm_table = (pldm_fwup_comp_param_table_t *)&(rsp_dat->comp_img_set_ver_str[offset]);
    }
    *pkt_len += sizeof(pldm_get_fw_param_rsp_dat_t) + offset;
}

static void pldm_fwup_requestupdate(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_fwup_req_update_req_dat_t *req_dat = (pldm_fwup_req_update_req_dat_t *)(pkt->req_buf);
    pldm_fwup_req_update_rsp_dat_t *rsp_dat = (pldm_fwup_req_update_rsp_dat_t *)(pkt->rsp_buf);
    mctp_hdr_t *req_mctp_hdr = (mctp_hdr_t *)(pkt->req_buf - sizeof(pldm_request_t) - sizeof(mctp_hdr_t));
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    if (g_pldm_fwup_info.update_mode == UPDATING_MODE || CM_IS_AT_UPGRADE_MODE()) {
        rsp_hdr->cpl_code = PLDM_UD_ALREADY_IN_UPDATE_MODE;
        LOG("cpl_code : %#x", rsp_hdr->cpl_code);
        return;
    }

    g_pldm_fwup_info.max_transfer_size = MIN(PLDM_RECVBUF_MAX_SIZE, req_dat->max_transfer_size);    /* max transfer size */
    g_pldm_fwup_info.update_mode = UPDATING_MODE;

    cm_memcpy(&(g_pldm_fwup_info.fw_new_set_info.comp_img_set_ver_str_type_and_len), &(req_dat->comp_img_set_ver_str_type_and_len), \
    sizeof(pldm_comp_img_set_ver_str_type_and_len_t) + req_dat->comp_img_set_ver_str_type_and_len.len);

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
        LOG("cpl_code : %#x", rsp_dat->cpl_code);
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
    /* 如果接收的packagedata内容错误，或者FD不接受UA传输的packagedata，则置位gs_want_to_end_update，在下一个cmd的rsp_msg返回PACKAGE_DATA_ERROR并退出更新模式 */
}

static void pldm_fwup_passcomponenttable(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_fwup_pass_comp_table_req_dat_t *req_dat = (pldm_fwup_pass_comp_table_req_dat_t *)(pkt->req_buf);
    pldm_fwup_pass_comp_table_rsp_dat_t *rsp_dat = (pldm_fwup_pass_comp_table_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    /* The FD/FDP only expects this command in LEARN COMPONENTS state. */
    if ((gs_event_id == PLDM_UD_ENTER_UD_WITH_PKTDATA && gs_getpackagedata_progress == 0) || (g_pldm_fwup_info.cur_state != PLDM_UD_LEARN_COMP)) {
        rsp_hdr->cpl_code = PLDM_UD_INVALID_STATE_FOR_COMMAND;
        LOG("cpl_code : %#x", rsp_hdr->cpl_code);
        return;
    }

    if (gs_want_to_end_update == 1) {
        rsp_hdr->cpl_code = PLDM_UD_PACKAGEDATA_ERROR;
        gs_event_id = PLDM_UD_PACKAGEDATA_ERROR;
        LOG("cpl_code : %#x", rsp_hdr->cpl_code);
        return;
    }

    cm_memcpy(&g_pldm_fwup_info.fw_new_img_info, &(req_dat->comp_class_msg), sizeof(pldm_fwup_comp_class_msg_t));
    cm_memcpy(&g_pldm_fwup_info.fw_new_img_info.comp_ver_str_type, &(req_dat->comp_ver_msg.comp_ver_str_type), sizeof(pldm_comp_img_set_ver_str_type_and_len_t) + req_dat->comp_ver_msg.comp_ver_str_len);

    rsp_dat->comp_rsp = 0;
    rsp_dat->comp_rsp_code = 0;

    if (req_dat->transfer_flag == PLDM_TRANSFER_FLAG_END || req_dat->transfer_flag == PLDM_TRANSFER_FLAG_START_AND_END) {
        gs_event_id = PLDM_UD_PASSCOMP_END;
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
        LOG("cpl_code : %#x", rsp_hdr->cpl_code);
        return;
    }

    rsp_dat->comp_compatibility_rsp = 0;             /* Component can be updated. */
    rsp_dat->comp_compatibility_rsp_code = 0;        /* No response code – used when component can be updated. */

    /* 组件信息验证 */
    if (g_pldm_fwup_info.fw_new_img_info.comp_identifier == req_dat->comp_class_msg.comp_identifier) {
        u32 new_ver_str_len = g_pldm_fwup_info.fw_new_img_info.comp_ver_str_len;
        u32 req_ver_str_len = req_dat->comp_ver_str_type_len;
        u32 str_compare_len = MIN(new_ver_str_len, req_ver_str_len);
        int error_result = (new_ver_str_len != req_ver_str_len);
        error_result |= (g_pldm_fwup_info.fw_new_img_info.comp_ver_str_type != req_dat->comp_ver_str_type);
        error_result |= cm_memcmp(g_pldm_fwup_info.fw_new_img_info.comp_ver_str, req_dat->comp_ver_str, str_compare_len);
        if (error_result) {
            rsp_dat->comp_compatibility_rsp = 1;        // Component will not be updated
            rsp_dat->comp_compatibility_rsp_code = 3;   // Invalid component comparison stamp or version.
        }
    } else {
        rsp_dat->comp_compatibility_rsp = 1;            // Component will not be updated
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
    pldm_fwup_transfer_cpl_req_dat_t req_dat = {0};
    req_dat.transfer_result = result;

    pldm_msg_send((u8 *)&req_dat, sizeof(pldm_fwup_transfer_cpl_req_dat_t), MCTP_PLDM_UPDATE, 0x16, g_pldm_fwup_info.hw_id, g_pldm_fwup_info.ua_eid);
}

static u8 transfer_result = 0;
static void pldm_fwup_requestfwdata_recv(protocol_msg_t *pkt, int *pkt_len)
{
    g_pldm_need_rsp = 0;
    pldm_fwup_req_fw_data_rsp_dat_t *rsp_dat = (pldm_fwup_req_fw_data_rsp_dat_t *)(pkt->req_buf);
    if (rsp_dat->cpl_code != MCTP_COMMAND_SUCCESS) {
        LOG("cpl_code : %#x", rsp_dat->cpl_code);
        return;
    }

    sts_t upgrade_state = 0;

    u8 comp_identifier = g_pldm_fwup_info.fw_new_img_info.comp_identifier;
    u16 free_len = PLDM_RECVBUF_MAX_SIZE - gs_fw_data_buf.len;
    u16 cpy_len = MIN(gs_requestfwdata_once_size, free_len);
    u16 remain_len = (gs_requestfwdata_once_size > free_len) ? gs_requestfwdata_once_size - free_len : 0;
    u8 *data_ptr = NULL;
    if (g_pldm_fwup_info.max_transfer_size != PLDM_RECVBUF_MAX_SIZE) {
        cm_memcpy(&(gs_fw_data_buf.data[gs_fw_data_buf.len]), rsp_dat->comp_img_option, cpy_len);
        data_ptr = gs_fw_data_buf.data;
    } else {
        data_ptr = rsp_dat->comp_img_option;
    }

    gs_fw_data_buf.len += cpy_len;

    if (gs_fw_data_buf.len == PLDM_RECVBUF_MAX_SIZE) {
        upgrade_state = PLDM_FW_UPDATE_CALLBACK(comp_identifier, data_ptr);
        fwrite(data_ptr, sizeof(u8), PLDM_RECVBUF_MAX_SIZE, pd);
        if (upgrade_state != 0x1) {  // flash erase failed
            transfer_result = 0x0D;  // The FD/FDP has aborted the transfer due to an issue with storing the firmware data on the device.
            pldm_fwup_transfercpl_send(transfer_result);
            return;
        }
        if (remain_len)
            cm_memcpy(gs_fw_data_buf.data, &(rsp_dat->comp_img_option[cpy_len]), remain_len);
        gs_fw_data_buf.len = remain_len;
    }

    gs_data_transfer_handle += gs_requestfwdata_once_size;

    if (gs_data_transfer_handle >= gs_fw_data_buf.pkt_data_len) {
        /*  1 : Transfer has completed with error as the image received is corrupt.
            0 : Transfer has completed without error, no additional information on why is provided with this code. */
        transfer_result = 0;
        pldm_fwup_transfercpl_send(transfer_result);
        gs_data_transfer_handle = 0;
        cm_memset(&gs_fw_data_buf, 0, 6);
        fclose(pd);
    } else {
        u32 remain_data_len = gs_fw_data_buf.pkt_data_len - gs_data_transfer_handle;
        gs_requestfwdata_once_size = (remain_data_len < g_pldm_fwup_info.max_transfer_size) ? remain_data_len : g_pldm_fwup_info.max_transfer_size;
        pldm_fwup_requestfwdata_send();
    }
}

static u8 verify_result = 0;
static void pldm_fwup_verifycpl_send(void)
{
    /* Verify has completed with error as the image failed the FD security checks */
    // verify_result = 3;
    pldm_fwup_verify_cpl_req_dat_t req_dat = {0};
    req_dat.verify_result = verify_result;
    pldm_msg_send((u8 *)&req_dat, sizeof(pldm_fwup_verify_cpl_req_dat_t), MCTP_PLDM_UPDATE, 0x17, g_pldm_fwup_info.hw_id, g_pldm_fwup_info.ua_eid);
}

static void pldm_fwup_applycpl_send(void)
{
    pldm_fwup_apply_cpl_req_dat_t req_dat = {0};

    /* 0x01:Apply has completed with success and has modified its activation method. Values shall be provided in the ComponentActivationMethodsModifications field */
    req_dat.apply_result = 0x01;

    req_dat.comp_actv_meth_modification = PLDM_IMG_ACTIVE_METHOD;
    pldm_msg_send((u8 *)&req_dat, sizeof(pldm_fwup_apply_cpl_req_dat_t), MCTP_PLDM_UPDATE, 0x18, g_pldm_fwup_info.hw_id, g_pldm_fwup_info.ua_eid);
}

static void pldm_fwup_transfercpl_recv(protocol_msg_t *pkt, int *pkt_len)
{
    g_pldm_need_rsp = 0;
    pldm_fwup_transfer_cpl_rsp_dat_t *rsp_dat = (pldm_fwup_transfer_cpl_rsp_dat_t *)(pkt->req_buf);
    if (rsp_dat->cpl_code != MCTP_COMMAND_SUCCESS) {
        LOG("cpl_code : %#x", rsp_dat->cpl_code);
    } else {
        if (!transfer_result)
            gs_event_id = PLDM_UD_TRANSFER_PASS;
        gs_progress_flag = 1;
    }
}

static void pldm_fwup_verifycpl_recv(protocol_msg_t *pkt, int *pkt_len)
{
    g_pldm_need_rsp = 0;
    pldm_fwup_verify_cpl_rsp_dat_t *rsp_dat = (pldm_fwup_verify_cpl_rsp_dat_t *)(pkt->req_buf);
    if (rsp_dat->cpl_code != MCTP_COMMAND_SUCCESS) {
        LOG("cpl_code : %#x", rsp_dat->cpl_code);
    } else {
        if (!verify_result)
            gs_event_id = PLDM_UD_VERIFY_PASS;
        gs_progress_flag = 1;
    }
}

static void pldm_fwup_verify_process(void)
{
    u8 comp_identifier = g_pldm_fwup_info.fw_new_img_info.comp_identifier;
    sts_t upgrade_result = PLDM_FW_UPDATE_COMPLETE_CALLBACK(comp_identifier);
    verify_result = 0x0;
    if (upgrade_result != 0x1) {
        LOG("pldm firmware update img crc error");
        verify_result = 0x01;   // Verify has completed with a verification failure – FD will not transition to APPLY state to apply the component
    }
    pldm_fwup_verifycpl_send();
}

static void pldm_fwup_applycpl_recv(protocol_msg_t *pkt, int *pkt_len)
{
    g_pldm_need_rsp = 0;
    pldm_fwup_apply_cpl_rsp_dat_t *rsp_dat = (pldm_fwup_apply_cpl_rsp_dat_t *)(pkt->req_buf);
    if (rsp_dat->cpl_code != MCTP_COMMAND_SUCCESS) {
        LOG("cpl_code : %#x", rsp_dat->cpl_code);
    } else {
        gs_event_id = PLDM_UD_APPLY_PASS;
        gs_progress_flag = 1;
    }
}

void pldm_fwup_info_printf(pldm_fwup_base_info_t *fwup_info);

static void pldm_fwup_activatefw(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_fwup_actv_fw_req_dat_t *req_dat = (pldm_fwup_actv_fw_req_dat_t *)(pkt->req_buf);
    pldm_fwup_actv_fw_rsp_dat_t *rsp_dat = (pldm_fwup_actv_fw_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    if (g_pldm_fwup_info.cur_state != PLDM_UD_READY_XFER) {
        rsp_hdr->cpl_code = PLDM_UD_INVALID_STATE_FOR_COMMAND;
        LOG("cpl_code : %#x", rsp_hdr->cpl_code);
        goto L_ERR;
    }

    if (gs_event_id != PLDM_UD_APPLY_PASS) {
        rsp_hdr->cpl_code = PLDM_UD_INCOMPLETE_UPDATE;
        LOG("cpl_code : %#x", rsp_hdr->cpl_code);
        goto L_ERR;
    }

    /* amber no support self_contain_active */
    if (req_dat->self_contained_actv_req == TRUE) {    /* 如果BMC想自激活，自复位？ */
        rsp_hdr->cpl_code = PLDM_UD_SELF_CONTAINED_ACTIVATION_NOT_PERMITTED;
    }

    gs_event_id = PLDM_UD_ACTIVATE_DONE;
    gs_progress_flag = 1;
    rsp_dat->estimated_time_for_self_contained_actv = 0;

    u8 comp_identifier = g_pldm_fwup_info.fw_new_img_info.comp_identifier;
    cm_memcpy(&g_pldm_fwup_info.fw_pending_img_info[comp_identifier], &g_pldm_fwup_info.fw_new_img_info, sizeof(g_pldm_fwup_info.fw_new_img_info));
    cm_memcpy(&g_pldm_fwup_info.fw_pending_set_info, &g_pldm_fwup_info.fw_new_set_info, sizeof(g_pldm_fwup_info.fw_new_set_info));
    if (comp_identifier == PLDM_UD_SLOT) {
        g_pldm_fwup_info.pending_img_state |= 1;
    } else {
        g_pldm_fwup_info.pending_img_state = (0x1 << comp_identifier);
    }

    pldm_fwup_comp_img_info_t fw_active_set_info = g_pldm_fwup_info.fw_active_set_info;
    g_pldm_fwup_info.fw_active_set_info = g_pldm_fwup_info.fw_pending_set_info;
    pldm_component_info_t fw_cur_img_info = g_pldm_fwup_info.fw_cur_img_info[comp_identifier];
    g_pldm_fwup_info.fw_cur_img_info[comp_identifier] = g_pldm_fwup_info.fw_pending_img_info[comp_identifier];
    u32 len = sizeof(u32) + sizeof(pldm_fwup_comp_img_info_t) + sizeof(pldm_component_info_t) * PLDM_FWUP_COMP_TYPE_NUM;
    u32 active_img_state = g_pldm_fwup_info.active_img_state;
    g_pldm_fwup_info.active_img_state = g_pldm_fwup_info.pending_img_state;
    sts_t sta = CM_FALSH_WRITE(g_pldm_fwup_info.nvm_fwup_info_addr, &g_pldm_fwup_info.active_img_state, (len / sizeof(u32)));
    // pldm_fwup_info_printf(&g_pldm_fwup_info);
    if (sta != 0x1) {
        LOG("flash write error");
    }
    g_pldm_fwup_info.fw_active_set_info = fw_active_set_info;
    g_pldm_fwup_info.fw_cur_img_info[comp_identifier] = fw_cur_img_info;
    g_pldm_fwup_info.active_img_state = active_img_state;

    *pkt_len += sizeof(pldm_fwup_actv_fw_rsp_dat_t);
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
    *pkt_len += sizeof(pldm_fwup_get_status_rsp_dat_t);
}

static void pldm_fwup_compupdate_cancel(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    if ((g_pldm_fwup_info.cur_state >= PLDM_UD_DOWNLOAD) && (g_pldm_fwup_info.cur_state <= PLDM_UD_APPLY)) {
        gs_event_id = PLDM_UD_CANCEL_UD_COMP;
    } else {
        rsp_hdr->cpl_code = PLDM_UD_INVALID_STATE_FOR_COMMAND;
        LOG("cpl_code : %#x", rsp_hdr->cpl_code);
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

static pldm_cmd_func pldm_update_cmd_table[PLDM_FW_UPDATE_CMD] =
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
static pldm_fwup_state_transform_t pldm_fwup_transforms[] = {
    /* cur_state         event_id                      next_state          action */
    {PLDM_UD_IDLE,       PLDM_UD_ENTER_UD_WITH_PKTDATA,PLDM_UD_LEARN_COMP, pldm_fwup_getpackagedata_send},
    {PLDM_UD_IDLE,       PLDM_UD_ENTER_UD_NO_PKTDATA,  PLDM_UD_LEARN_COMP, NULL},
    {PLDM_UD_LEARN_COMP, PLDM_UD_PACKAGEDATA_ERROR,    PLDM_UD_IDLE,       pldm_fwup_firmware_process_quit},
    {PLDM_UD_LEARN_COMP, PLDM_UD_PASSCOMP_END,         PLDM_UD_READY_XFER, NULL},
    {PLDM_UD_READY_XFER, PLDM_UD_UD_COMP_END,          PLDM_UD_DOWNLOAD,   pldm_fwup_requestfwdata_send},
    {PLDM_UD_DOWNLOAD,   PLDM_UD_TRANSFER_PASS,        PLDM_UD_VERIFY,     pldm_fwup_verify_process},
    {PLDM_UD_VERIFY,     PLDM_UD_VERIFY_PASS,          PLDM_UD_APPLY,      pldm_fwup_applycpl_send},
    {PLDM_UD_APPLY,      PLDM_UD_APPLY_PASS,           PLDM_UD_READY_XFER, NULL},
    {PLDM_UD_READY_XFER, PLDM_UD_ACTIVATE_DONE,        PLDM_UD_IDLE,       pldm_fwup_firmware_process_quit},    /* ACTIVATE -> IDLE (FD moves through ACTIVATE step to IDLE) */

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
        cmd_proc = pldm_update_cmd_table[cmd_code];
    } else {
        cmd_proc = pldm_unsupport_cmd;
    }

    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    if ((g_pldm_fwup_info.update_mode == NON_UPDATE_MODE) && (g_pldm_fwup_info.cur_state != PLDM_UD_IDLE)) {
        rsp_hdr->cpl_code = PLDM_UD_NOT_IN_UPDATE_MODE;
        LOG("cpl_code : %#x, cur_state : %d", rsp_hdr->cpl_code, g_pldm_fwup_info.cur_state);
        return;
    }
    gs_cal_times.req_time = CM_GET_CUR_TIMER_MS();
    cmd_proc(pkt, pkt_len);
}

void pldm_fwup_info_printf(pldm_fwup_base_info_t *fwup_info)
{
    LOG("active_img_state : %d", fwup_info->active_img_state);
    LOG("type : %d", fwup_info->fw_active_set_info.comp_img_set_ver_str_type_and_len.type);
    LOG("len : %d", fwup_info->fw_active_set_info.comp_img_set_ver_str_type_and_len.len);
    for (u8 i = 0; i < fwup_info->fw_active_set_info.comp_img_set_ver_str_type_and_len.len; i++) {
        printf("%c", fwup_info->fw_active_set_info.comp_img_ver_str[i]);
    }
    LOG("");

    for (u8 i = 0; i < 3; i++) {
        LOG("comp_ver_str_type : %d", fwup_info->fw_cur_img_info[i].comp_ver_str_type);
        LOG("comp_ver_str_len : %d", fwup_info->fw_cur_img_info[i].comp_ver_str_len);
        for (u8 j = 0; j < fwup_info->fw_cur_img_info[i].comp_ver_str_len; j++) {
            printf("%c", fwup_info->fw_cur_img_info[i].comp_ver_str[j]);
        }
        LOG("");
    }
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

    pldm_data_hdr_t pldm_data_hdr = {0};
    CM_FLASH_READ(0, (void *)&pldm_data_hdr, (sizeof(pldm_data_hdr_t) / sizeof(u32)));

    g_pldm_fwup_info.nvm_fwup_info_addr = 0 + pldm_data_hdr.pldm_fwup_info_off;
    LOG("fwup info init from nvm, total len : %d, cnt : %d", pldm_data_hdr.pldm_fwup_info_off, pldm_data_hdr.pldm_fwup_info_size);
    CM_FLASH_READ(g_pldm_fwup_info.nvm_fwup_info_addr, (void *)&g_pldm_fwup_info.active_img_state, (sizeof(pldm_component_info_t) * PLDM_FWUP_COMP_TYPE_NUM + sizeof(pldm_fwup_comp_img_info_t) + sizeof(u32)) / sizeof(u32));
    // pldm_fwup_info_printf(&g_pldm_fwup_info);
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
        pldm_fwup_sta_chg(PLDM_UD_IDLE);
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