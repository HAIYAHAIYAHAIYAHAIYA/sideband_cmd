#include "pldm_fru_data.h"
#include "pldm_fw_update.h"
#include "pldm.h"

static u8 gs_pldm_fru_table[PLDM_FRU_TABLE_BUF_LEN];

extern void pldm_unsupport_cmd(protocol_msg_t *pkt, int *pkt_len);
extern u32 crc32_pldm(u32 init_crc, u8 *data, u32 len);

static void pldm_fru_get_fru_record_table_metadata(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_fru_get_fru_record_table_metadata_rsp_dat_t *rsp_dat = (pldm_fru_get_fru_record_table_metadata_rsp_dat_t *)(pkt->rsp_buf);

    u8 rsp_len = sizeof(pldm_fru_get_fru_record_table_metadata_rsp_dat_t);
    cm_memcpy(rsp_dat, gs_pldm_fru_table, rsp_len);
    u32 crc32 = crc32_pldm(0xFFFFFFFFUL, &gs_pldm_fru_table[rsp_len], rsp_dat->fru_table_len);
    cm_memcpy(&((u8 *)rsp_dat)[rsp_len], &crc32, sizeof(u32));
    *pkt_len += rsp_len + sizeof(u32);
}

static u32 gs_prev_transfer_hdl = 0;
static void pldm_fru_get_fru_record_table(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_fru_get_fru_record_table_req_dat_t *req_dat = (pldm_fru_get_fru_record_table_req_dat_t *)(pkt->req_buf);
    pldm_fru_get_fru_record_table_rsp_dat_t *rsp_dat = (pldm_fru_get_fru_record_table_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    pldm_fru_record_table_fmt_t *table = (pldm_fru_record_table_fmt_t *)gs_pldm_fru_table;

    u16 table_len = table->head.fru_table_len;
    u32 data_transfer_hdl = req_dat->transfer_op_flg == PLDM_TRANSFER_OP_FLAG_GET_FIRST_PART ? 0 : req_dat->data_transfer_hdl;

    if (gs_prev_transfer_hdl != data_transfer_hdl) {
        rsp_hdr->cpl_code = PLDM_INVALID_DATA_TRANSFER_HANDLE;
        return;
    }

    if (req_dat->transfer_op_flg != PLDM_TRANSFER_OP_FLAG_GET_FIRST_PART && req_dat->transfer_op_flg != PLDM_TRANSFER_OP_FLAG_GET_NEXT_PART) {
        rsp_hdr->cpl_code = PLDM_INVALID_TRANSFER_OPERATION_FLAG;
        return;
    }

    u16 remain_len = table_len - data_transfer_hdl;
    u16 cpy_len = remain_len <= PLDM_FRU_TRANSFER_BUFFERSIZE ? remain_len : PLDM_FRU_TRANSFER_BUFFERSIZE;
    u8 *cpy_addr = (u8 *)&(table->field);

    if (remain_len <= PLDM_FRU_TRANSFER_BUFFERSIZE) {
        if (req_dat->data_transfer_hdl == PLDM_TRANSFER_OP_FLAG_GET_FIRST_PART) {
            rsp_dat->transfer_flg = PLDM_FRU_TRANSFRT_FLG_START_AND_END;
        } else {
            rsp_dat->transfer_flg = PLDM_FRU_TRANSFRT_FLG_END;
        }
        rsp_dat->next_data_transfer_hdl = 0;
    } else {
        rsp_dat->transfer_flg = PLDM_FRU_TRANSFRT_FLG_MIDDLE;
        rsp_dat->next_data_transfer_hdl = data_transfer_hdl + cpy_len;
    }
    cm_memcpy(rsp_dat->portion_of_data, &cpy_addr[data_transfer_hdl], cpy_len);
    gs_prev_transfer_hdl = rsp_dat->next_data_transfer_hdl;
    *pkt_len += sizeof(pldm_fru_get_fru_record_table_rsp_dat_t) + cpy_len;
}

static pldm_cmd_func pldm_fru_cmd_table[PLDM_FRU_DATA_CMD] =
{
    pldm_unsupport_cmd,
    pldm_fru_get_fru_record_table_metadata,
    pldm_fru_get_fru_record_table,
};

void pldm_fru_process(protocol_msg_t *pkt, int *pkt_len, u32 cmd_code)
{
    pldm_cmd_func cmd_proc = NULL;

    if (cmd_code < PLDM_FRU_DATA_CMD) {
        cmd_proc = pldm_fru_cmd_table[cmd_code];
    } else {
        cmd_proc = pldm_unsupport_cmd;
    }

    return cmd_proc(pkt, pkt_len);
}

/* refer to \\nfs\public\firmware\common\pldm\vendors\MCTPFRU-AN1281-100.pdf */
/* refer to \\nfs\public\firmware\common\pldm\vendors\MCTPFRU-AN1408-100.pdf */
/* to be determind */
u8 *pldm_fru_fill_general_part(u8 *buf, u8 *tlv_num)
{
    char vals[15][32];
    cm_memset(vals, '\0', sizeof(vals));

    snprintf(vals[FIELD_PART_NUM_TYPE], 32, "%s", "");
    snprintf(vals[FIELD_SERIAL_NUM_TYPE], 32, "%d", MAX_LAN_NUM);
    snprintf(vals[FIELD_MANUFACTURER_TYPE], 32, "%s", "");
    snprintf(vals[FIELD_MANUFACTURER_DATE_TYPE], 13, "%s", "");
    snprintf(vals[FIELD_VENDOR_TYPE], 32, "%s", "");
    snprintf(vals[FIELD_NAME_TYPE], 32, "%s", "");
    snprintf(vals[FIELD_VERSION_TYPE], 32, "Img Version is 0x%x", CM_GET_IMG_VERSION);
    snprintf(vals[FIELD_DESCRIPTION_TYPE], 32, "%s", "");
    snprintf(vals[FIELD_VENDOR_IANA_TYPE], 4, "%s", "");

    pldm_fru_tlv_fmt_t *tlv = (pldm_fru_tlv_fmt_t *)buf;
    *tlv_num = sizeof(vals) / sizeof(vals[0]);
    for (u8 i = 0; i < *tlv_num; i++) {
        tlv->type = i + 1;
        tlv->len = cm_strlen(vals[i]);
        cm_memcpy(tlv->val, vals[i], tlv->len);
        tlv = (pldm_fru_tlv_fmt_t *)&(tlv->val[tlv->len]);
    }
    return (u8 *)tlv;
}

/* to be determind */
u8 *pldm_fru_fill_chip_part(u8 *buf, u8 *tlv_num)
{
    char vals[7][32];
    // u8 mac[6];
    // u8 vpd_sn[25];
    cm_memset(vals, '\0', sizeof(vals));

    snprintf(vals[FIELD_OEM_VENDOR_IANA_TYPE], 4, "%s", CM_VENDOR_GET_VEMDOR_IANA);
    snprintf(vals[FIELD_OEM_FW_VERSION_TYPE], 48, "FW Version : 0x%x", CM_GET_FW_VERSION);
    snprintf(vals[FIELD_OEM_DID_TYPE], 48, "DID : 0x%x", CM_VENDOR_GET_PCI_DEV_ID);
    snprintf(vals[FIELD_OEM_VID_TYPE], 48, "VID : 0x%x", CM_VENDOR_GET_PCI_VENDOR_ID);
    snprintf(vals[FIELD_OEM_SSID_TYPE], 48, "SSID : 0x%x", CM_VENDOR_GET_PCI_SUBSYS_ID);
    snprintf(vals[FIELD_OEM_SVID_TYPE], 48, "SVID : 0x%x", CM_VENDOR_GET_PCI_SUBSYS_VENDOR_ID);
    snprintf(vals[FIELD_OEM_PCIE_LINK_SPD_TYPE], 48, "PCIe Link Speed : 0x%x", "");

    pldm_fru_tlv_fmt_t *tlv = (pldm_fru_tlv_fmt_t *)buf;
    *tlv_num = sizeof(vals) / sizeof(vals[0]);
    for (u8 i = 0; i < *tlv_num; i++) {
        tlv->type = i + 1;
        tlv->len = cm_strlen(vals[i]);
        cm_memcpy(tlv->val, vals[i], tlv->len);
        tlv = (pldm_fru_tlv_fmt_t *)&(tlv->val[tlv->len]);
    }
    return (u8 *)tlv;
}

/* to be determind */
u8 *pldm_fru_fill_portn_part(u8 *buf, u8 port, u8 *tlv_num)
{
    char vals[3][48];
    cm_memset(vals, '\0', sizeof(vals));

    snprintf(vals[FIELD_OEM_VENDOR_IANA_TYPE], 4, "%s", CM_VENDOR_GET_VEMDOR_IANA);
    snprintf(vals[FIELD_OEM_PORT_NUM_TYPE], 48, "Port Name : %d", port);
    snprintf(vals[FIELD_OEM_LINK_SPD_CAP_TYPE], 48, "Link Speed Capabilities : X%d", "");

    pldm_fru_tlv_fmt_t *tlv = (pldm_fru_tlv_fmt_t *)buf;
    *tlv_num = sizeof(vals) / sizeof(vals[0]);
    for (u8 i = 0; i < *tlv_num; i++) {
        tlv->type = !i ? 1 : (i | 0x80);
        tlv->len = cm_strlen(vals[i]);
        cm_memcpy(tlv->val, vals[i], tlv->len);
        tlv = (pldm_fru_tlv_fmt_t *)&(tlv->val[tlv->len]);
    }
    return (u8 *)tlv;
}

u8 *pldm_fru_fill_main_part(u8 *buf)
{
    if (!buf) return NULL;
    pldm_fru_record_table_field_fmt_t fields[] = {
        [0] = {FRU_GENERAL_SET_ID, RECORD_GENERAL_FRU_RECORD_TYPE, 0, PLDM_UD_TYPE_ASCII},
        [1] = {FRU_CHIP_SET_ID   , RECORD_OEM_FRU_RECORD_TYPE,     0, PLDM_UD_TYPE_ASCII},
    };

    pldm_fru_fill_table *fill_func[] = {
        pldm_fru_fill_general_part,
        pldm_fru_fill_chip_part
    };

    pldm_fru_record_table_field_fmt_t *record_data = (pldm_fru_record_table_field_fmt_t *)buf;
    u8 *next_part = NULL;
    for (u8 i = 0; i < 2; i++) {
        *record_data = fields[i];
        next_part = record_data->tlv;
        next_part = fill_func[i](next_part, &(record_data->num_of_fru_fields));
        record_data = (pldm_fru_record_table_field_fmt_t *)next_part;
    }
    return next_part;
}

u8 *pldm_fru_fill_sub_part(u8 *buf)
{
    if (!buf) return NULL;
    pldm_fru_record_table_field_fmt_t fields[] = {
        [0] = {FRU_PORT0_SET_ID  , RECORD_OEM_FRU_RECORD_TYPE,     0, PLDM_UD_TYPE_ASCII},
        [1] = {FRU_PORT1_SET_ID  , RECORD_OEM_FRU_RECORD_TYPE,     0, PLDM_UD_TYPE_ASCII},
        [2] = {FRU_PORT2_SET_ID  , RECORD_OEM_FRU_RECORD_TYPE,     0, PLDM_UD_TYPE_ASCII},
        [3] = {FRU_PORT3_SET_ID  , RECORD_OEM_FRU_RECORD_TYPE,     0, PLDM_UD_TYPE_ASCII},
    };

    pldm_fru_record_table_field_fmt_t *record_data = (pldm_fru_record_table_field_fmt_t *)buf;
    u8 *next_part = NULL;

    for (u8 i = 0; i < MAX_LAN_NUM; i++) {
        *record_data = fields[i];
        next_part = record_data->tlv;
        next_part = pldm_fru_fill_portn_part(next_part, i, &(record_data->num_of_fru_fields));
        record_data = (pldm_fru_record_table_field_fmt_t *)next_part;
    }
    return next_part;
}

static void pldm_fru_data_printf(pldm_fru_record_table_fmt_t *fru_data)
{
    LOG("fru_data_major_ver : %d", fru_data->head.fru_data_major_ver);
    LOG("fru_data_minor_ver : %d", fru_data->head.fru_data_minor_ver);
    LOG("fru_table_len : %d", fru_data->head.fru_table_len);
    LOG("fru_table_maxi_size : %d", fru_data->head.fru_table_maxi_size);
    LOG("num_of_records : %d", fru_data->head.num_of_records);
    LOG("num_of_records_set_identifiers : %d", fru_data->head.num_of_records_set_identifiers);
    LOG("");

    pldm_fru_record_table_field_fmt_t *field = &(fru_data->field);

    for (u8 i = 0; i < fru_data->head.num_of_records_set_identifiers; i++) {
        LOG("fru_record_set_identifier : %d", field->fru_record_set_identifier);
        LOG("fru_record_type : %d", field->fru_record_type);
        LOG("num_of_fru_fields : %d", field->num_of_fru_fields);
        LOG("encode_type_of_fru_field : %d", field->encode_type_of_fru_field);
        u8 *ptr = (u8 *)(field->tlv);
        for (u8 j = 0; j < field->num_of_fru_fields; j++) {
            pldm_fru_tlv_fmt_t *tlv = (pldm_fru_tlv_fmt_t *)ptr;
            LOG("type : %#x", tlv->type);
            LOG("len : %d", tlv->len);
            for (u8 k = 0; k < tlv->len; k++) {
                printf("%c", tlv->val[k]);
            }
            printf("\n");
            ptr = (u8 *)(&tlv->val[tlv->len]);
        }
        field = (pldm_fru_record_table_field_fmt_t *)ptr;
    }
}

/* to be determind */
void pldm_fru_init(void)
{
    pldm_fru_record_table_fmt_t *table = (pldm_fru_record_table_fmt_t *)gs_pldm_fru_table;
    table->head.fru_data_major_ver = 0x01;
    table->head.fru_data_minor_ver = 0;
    table->head.fru_table_maxi_size = 0;                                /* means that SetFRURecordTable command is not supported. */
    table->head.num_of_records_set_identifiers = 2 + MAX_LAN_NUM;
    table->head.num_of_records = 1;                                     /* may be equal to num_of_records_set_identifiers ! */
    u8 *next_part = (u8 *)&(table->field);
    next_part = pldm_fru_fill_main_part(next_part);
    next_part = pldm_fru_fill_sub_part(next_part);

    table->head.fru_table_len = next_part - gs_pldm_fru_table - sizeof(pldm_fru_get_fru_record_table_metadata_rsp_dat_t);
    LOG("used len : %d", next_part - gs_pldm_fru_table);
    // pldm_fru_data_printf(table);
}