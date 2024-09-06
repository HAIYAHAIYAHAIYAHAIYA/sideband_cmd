#include "pldm_fru_data.h"
#include "pldm_fw_update.h"
#include "pldm.h"

static u8 gs_pldm_fru_table[PLDM_FRU_TABLE_BUF_LEN];

extern void pldm_unsupport_cmd(protocol_msg_t *pkt, int *pkt_len);
extern u32 crc32_pldm(u32 init_crc, u8 *data, u32 len);

u8 *pldm_fru_find_next_fru_data(u8 *start_addr);
u8 pldm_fru_fill_pad(u8 *pad_pos, u8 data_len, u8 algin_len, u8 pad_val);

#define CM_VENDOR_GET_VEMDOR_IANA                   htonl(0x56575859)
#define CM_VENDOR_GET_PART_NUMBER                   "0"
#define CM_VENDOR_GET_MANUFACTURER                  "0"
#define CM_VENDOR_GET_MANUFACTURE_DATE              "2024-08-09-10"
#define CM_VENDOR_GET_VENDOR                        "0"
#define CM_VENDOR_GET_NAME                          "0"
#define CM_VENDOR_GET_DESCRIPTION                   "0"

#define CM_VENDOR_GET_VPD_SN                        NULL
#define CM_VENDOR_GET_MAC(port, mac) \
do { \
} while(0)

#define CM_VENDOR_GET_PCIE_LINK_SPD(val)            (void)val

#define CM_GET_FW_VERSION                           (0x56575859)
#define CM_GET_IMG_VERSION                          (0x56575859)

#define CM_VENDOR_GET_PCI_DEV_ID                    0x56575859
#define CM_VENDOR_GET_PCI_VENDOR_ID                 0x56575859
#define CM_VENDOR_GET_PCI_SUBSYS_ID                 0x56575859
#define CM_VENDOR_GET_PCI_SUBSYS_VENDOR_ID          0x56575859
#define CM_VENDOR_GET_PCI_REVISION_ID               (0)

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

    u8 *cpy_addr = (u8 *)&(table->field);
    u8 *next_part = pldm_fru_find_next_fru_data(&cpy_addr[data_transfer_hdl]);
    u16 cpy_len = next_part - &cpy_addr[data_transfer_hdl];

    if (next_part >= &cpy_addr[table_len - 1]) {
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

    /* Table 7 – PLDM Representation of FRU Record Data */
    u8 pad_len = pldm_fru_fill_pad(&(rsp_dat->portion_of_data[cpy_len]), cpy_len, 4, 0x00);
    u32 fru_data_crc = crc32_pldm(0xFFFFFFFFUL, rsp_dat->portion_of_data, pad_len + cpy_len);
    cm_memcpy(&(rsp_dat->portion_of_data[pad_len + cpy_len]), &fru_data_crc, 4);

    gs_prev_transfer_hdl = rsp_dat->next_data_transfer_hdl;
    *pkt_len += sizeof(pldm_fru_get_fru_record_table_rsp_dat_t) + cpy_len + pad_len + sizeof(fru_data_crc);
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

u8 *pldm_fru_find_next_fru_data(u8 *start_addr)
{
    if (!start_addr) return NULL;
    pldm_fru_record_table_field_fmt_t *fru_data = (pldm_fru_record_table_field_fmt_t *)start_addr;
    u8 *next_tlv = fru_data->tlv;
    for (u8 i = 0; i < fru_data->num_of_fru_fields; ++i) {
        pldm_fru_tlv_fmt_t *tlv = (pldm_fru_tlv_fmt_t *)next_tlv;
        next_tlv = (u8 *)&(tlv->val[tlv->len]);
    }
    return next_tlv;
}

u8 pldm_fru_fill_pad(u8 *pad_pos, u8 data_len, u8 algin_len, u8 pad_val)
{
    if (!pad_pos) return 0;
    if (!(data_len % algin_len)) return 0;
    u8 pad_len = algin_len - data_len % algin_len;
    cm_memset(pad_pos, pad_val, pad_len);
    return pad_len;
}

/* refer to \\nfs\public\firmware\common\pldm\vendors\MCTPFRU-AN1281-100.pdf */
/* refer to \\nfs\public\firmware\common\pldm\vendors\MCTPFRU-AN1408-100.pdf */
/* to be determind */
static u8 *pldm_fru_fill_general_part(u8 *buf, u8 *tlv_num)
{
    char vals[15][48];
    u32 vendor_iana = CM_VENDOR_GET_VEMDOR_IANA;
    cm_memset(vals, '\0', sizeof(vals));

    cm_snprintf(vals[FIELD_PART_NUM_TYPE], 48, "%s", CM_VENDOR_GET_PART_NUMBER);
    cm_snprintf(vals[FIELD_SERIAL_NUM_TYPE], 48, "%d", MAX_LAN_NUM);
    cm_snprintf(vals[FIELD_MANUFACTURER_TYPE], 48, "%s", CM_VENDOR_GET_MANUFACTURER);
    cm_snprintf(vals[FIELD_MANUFACTURER_DATE_TYPE], 14, "%s", CM_VENDOR_GET_MANUFACTURE_DATE);
    cm_snprintf(vals[FIELD_VENDOR_TYPE], 48, "%s", CM_VENDOR_GET_VENDOR);
    cm_snprintf(vals[FIELD_NAME_TYPE], 48, "%s", CM_VENDOR_GET_NAME);
    cm_snprintf(vals[FIELD_VERSION_TYPE], 48, "Img Version is 0x%x", CM_GET_IMG_VERSION);
    cm_snprintf(vals[FIELD_DESCRIPTION_TYPE], 48, "%s", CM_VENDOR_GET_DESCRIPTION);
    cm_memcpy(vals[FIELD_VENDOR_IANA_TYPE], &vendor_iana, 4);

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
static u8 *pldm_fru_fill_chip_part(u8 *buf, u8 *tlv_num)
{
    char vals[8][32];
    u8 vpd_sn[21];
    u32 val = 0;
    u32 vendor_iana = CM_VENDOR_GET_VEMDOR_IANA;
    cm_memset(vals, '\0', sizeof(vals));

    // CM_VENDOR_GET_VPD_SN(vpd_sn);
    CM_VENDOR_GET_PCIE_LINK_SPD(&val);

    cm_memcpy(vals[FIELD_OEM_VENDOR_IANA_TYPE], &vendor_iana, 4);
    cm_snprintf(vals[FIELD_OEM_FW_VERSION_TYPE], 32, "FW Version : 0x%x", CM_GET_FW_VERSION);
    cm_snprintf(vals[FIELD_OEM_VPD_SN_TYPE], 32, "VPD SN : 0x%s", vpd_sn);
    cm_snprintf(vals[FIELD_OEM_DID_TYPE], 32, "DID : 0x%x", CM_VENDOR_GET_PCI_DEV_ID);
    cm_snprintf(vals[FIELD_OEM_VID_TYPE], 32, "VID : 0x%x", CM_VENDOR_GET_PCI_VENDOR_ID);
    cm_snprintf(vals[FIELD_OEM_SSID_TYPE], 32, "SSID : 0x%x", CM_VENDOR_GET_PCI_SUBSYS_ID);
    cm_snprintf(vals[FIELD_OEM_SVID_TYPE], 32, "SVID : 0x%x", CM_VENDOR_GET_PCI_SUBSYS_VENDOR_ID);
    cm_snprintf(vals[FIELD_OEM_PCIE_LINK_SPD_TYPE], 32, "PCIe Link Speed : 0x%x", val);

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
static u8 *pldm_fru_fill_portn_part(u8 *buf, u8 port_id, u8 *tlv_num)
{
    char vals[3][48];
    u8 mac[6];
    u32 vendor_iana = CM_VENDOR_GET_VEMDOR_IANA;
    cm_memset(vals, '\0', sizeof(vals));
    cm_memset(mac, 0, sizeof(mac));

    CM_VENDOR_GET_MAC(port_id, mac);

    cm_memcpy(vals[FIELD_OEM_VENDOR_IANA_TYPE], &vendor_iana, 4);
    cm_snprintf(vals[FIELD_OEM_PORT_NUM_TYPE], 48, "Port Name : %d", port_id);
    cm_snprintf(vals[FIELD_OEM_MAC_TYPE], 48, "mac : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

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

static u8 *pldm_fru_fill_main_part(u8 *buf)
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

static u8 *pldm_fru_fill_sub_part(u8 *buf)
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

void pldm_fru_data_printf(pldm_fru_record_table_fmt_t *fru_data)
{
    LOG("fru_data_major_ver : %d", fru_data->head.fru_data_major_ver);
    LOG("fru_data_minor_ver : %d", fru_data->head.fru_data_minor_ver);
    LOG("fru_table_len : %d", fru_data->head.fru_table_len);
    LOG("fru_table_maxi_size : %d", fru_data->head.fru_table_maxi_size);
    LOG("num_of_records : %d", fru_data->head.num_of_records);
    LOG("num_of_records_set_identifiers : %d", fru_data->head.num_of_records_set_identifiers);

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
            LOG("");
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
    next_part = pldm_fru_fill_main_part(next_part);         /* general part, chip part */
    next_part = pldm_fru_fill_sub_part(next_part);          /* port n part */

    table->head.fru_table_len = next_part - gs_pldm_fru_table - sizeof(pldm_fru_get_fru_record_table_metadata_rsp_dat_t);
#if PLDM_FRU_DUMP_EN
    pldm_fru_data_printf(table);
#endif
}