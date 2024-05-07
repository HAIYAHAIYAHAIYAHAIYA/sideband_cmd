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
    LOG("CRC32 : %#x", crc32);
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
u8 *pldm_fru_fill_general_part(u8 *buf)
{
    char serial_num[2] = {MAX_LAN_NUM, 0x00};
    char *vals[] = {
        "",
        "",
        "Amber",
        serial_num,
        "WXKJ",
        "2024-04-22-18",
        "WXKJ",
        "Amber",
        "",
        "ver.xx.xx.xx",
        "",
        "Amber xx xx xx xx",
        "",
        "for WXKJ use"
    };
    pldm_fru_tlv_fmt_t *tlv = (pldm_fru_tlv_fmt_t *)buf;
    for (u8 i = 0; i < sizeof(vals) / sizeof(char *); i++) {
        tlv->type = i + 1;
        tlv->len = cm_strlen(vals[i]);
        cm_memcpy(tlv->val, vals[i], tlv->len);
        tlv = (pldm_fru_tlv_fmt_t *)&(tlv->val[tlv->len]);
    }
    return (u8 *)tlv;
}

u8 *pldm_fru_fill_chip_part(u8 *buf)
{
    char *vals[] = {
        "WXKJ",
        "FW Version:",
        "PCIe Link Speed: XXX",
    };
    pldm_fru_tlv_fmt_t *tlv = (pldm_fru_tlv_fmt_t *)buf;
    for (u8 i = 0; i < sizeof(vals) / sizeof(char *); i++) {
        tlv->type = i + 1;
        tlv->len = cm_strlen(vals[i]);
        cm_memcpy(tlv->val, vals[i], tlv->len);
        tlv = (pldm_fru_tlv_fmt_t *)&(tlv->val[tlv->len]);
    }
    return (u8 *)tlv;
}

u8 *pldm_fru_fill_port0_part(u8 *buf)
{
    char *vals[] = {
        "WXKJ",
        "Port Name: 0",
        "Link Type: FC",
        "WWNN: 20000000C9142356",
        "WWPN: 10000000C9142356",
        "Factory WWNN: 20000000C9142356",
        "Factory WWPN: 10000000C9142356",
        "FC Link Speed Capabilities: XXX",
    };
    pldm_fru_tlv_fmt_t *tlv = (pldm_fru_tlv_fmt_t *)buf;
    for (u8 i = 0; i < sizeof(vals) / sizeof(char *); i++) {
        tlv->type = i + 1;
        tlv->len = cm_strlen(vals[i]);
        cm_memcpy(tlv->val, vals[i], tlv->len);
        tlv = (pldm_fru_tlv_fmt_t *)&(tlv->val[tlv->len]);
    }
    return (u8 *)tlv;
}

u8 *pldm_fru_fill_port1_part(u8 *buf)
{
    char *vals[] = {
        "WXKJ",
        "Port Name: 1",
        "Link Type: FC",
        "WWNN: 20000000C9142356",
        "WWPN: 10000000C9142356",
        "Factory WWNN: 20000000C9142356",
        "Factory WWPN: 10000000C9142356",
        "FC Link Speed Capabilities: XXX",
    };
    pldm_fru_tlv_fmt_t *tlv = (pldm_fru_tlv_fmt_t *)buf;
    for (u8 i = 0; i < sizeof(vals) / sizeof(char *); i++) {
        tlv->type = i + 1;
        tlv->len = cm_strlen(vals[i]);
        cm_memcpy(tlv->val, vals[i], tlv->len);
        tlv = (pldm_fru_tlv_fmt_t *)&(tlv->val[tlv->len]);
    }
    return (u8 *)tlv;
}

u8 *pldm_fru_fill_port2_part(u8 *buf)
{
    char *vals[] = {
        "WXKJ",
        "Port Name: 2",
        "Link Type: FC",
        "WWNN: 20000000C9142356",
        "WWPN: 10000000C9142356",
        "Factory WWNN: 20000000C9142356",
        "Factory WWPN: 10000000C9142356",
        "FC Link Speed Capabilities: XXX",
    };
    pldm_fru_tlv_fmt_t *tlv = (pldm_fru_tlv_fmt_t *)buf;
    for (u8 i = 0; i < sizeof(vals) / sizeof(char *); i++) {
        tlv->type = i + 1;
        tlv->len = cm_strlen(vals[i]);
        cm_memcpy(tlv->val, vals[i], tlv->len);
        tlv = (pldm_fru_tlv_fmt_t *)&(tlv->val[tlv->len]);
    }
    return (u8 *)tlv;
}

u8 *pldm_fru_fill_port3_part(u8 *buf)
{
    char *vals[] = {
        "WXKJ",
        "Port Name: 3",
        "Link Type: FC",
        "WWNN: 20000000C9142356",
        "WWPN: 10000000C9142356",
        "Factory WWNN: 20000000C9142356",
        "Factory WWPN: 10000000C9142356",
        "FC Link Speed Capabilities: XXX",
    };
    pldm_fru_tlv_fmt_t *tlv = (pldm_fru_tlv_fmt_t *)buf;
    for (u8 i = 0; i < sizeof(vals) / sizeof(char *); i++) {
        tlv->type = i + 1;
        tlv->len = cm_strlen(vals[i]);
        cm_memcpy(tlv->val, vals[i], tlv->len);
        tlv = (pldm_fru_tlv_fmt_t *)&(tlv->val[tlv->len]);
    }
    return (u8 *)tlv;
}

void pldm_fru_init(void)
{
    pldm_fru_record_table_field_fmt_t fields[] = {
        [0] = {FRU_GENERAL_SET_ID, RECORD_GENERAL_FRU_RECORD_TYPE, 0, PLDM_UD_TYPE_ASCII},
        [1] = {FRU_CHIP_SET_ID   , RECORD_OEM_FRU_RECORD_TYPE,     0, PLDM_UD_TYPE_ASCII},
        [2] = {FRU_PORT0_SET_ID  , RECORD_OEM_FRU_RECORD_TYPE,     0, PLDM_UD_TYPE_ASCII},
        [3] = {FRU_PORT1_SET_ID  , RECORD_OEM_FRU_RECORD_TYPE,     0, PLDM_UD_TYPE_ASCII},
        [4] = {FRU_PORT2_SET_ID  , RECORD_OEM_FRU_RECORD_TYPE,     0, PLDM_UD_TYPE_ASCII},
        [5] = {FRU_PORT3_SET_ID  , RECORD_OEM_FRU_RECORD_TYPE,     0, PLDM_UD_TYPE_ASCII},
    };

    u8 num_of_fru_fields[] = {
        14,
        3,
        8,
        8,
        8,
        8,
    };

    pldm_fru_fill_table *fill_func[] = {
        pldm_fru_fill_general_part,
        pldm_fru_fill_chip_part,
        pldm_fru_fill_port0_part,
        pldm_fru_fill_port1_part,
        pldm_fru_fill_port2_part,
        pldm_fru_fill_port3_part
    };

    pldm_fru_record_table_fmt_t *table = (pldm_fru_record_table_fmt_t *)gs_pldm_fru_table;
    table->head.fru_data_major_ver = 0x01;
    table->head.fru_data_minor_ver = 0;
    table->head.fru_table_maxi_size = 0;                                /* means that SetFRURecordTable command is not supported. */
    table->head.num_of_records_set_identifiers = 2 + MAX_LAN_NUM;
    table->head.num_of_records = 1;
    u8 *next_part = NULL;
    pldm_fru_record_table_field_fmt_t *record_data = (pldm_fru_record_table_field_fmt_t *)&(table->field);
    for (u8 i = 0; i < table->head.num_of_records_set_identifiers; i++) {
        *record_data = fields[i];
        record_data->num_of_fru_fields = num_of_fru_fields[i];
        next_part = record_data->tlv;
        next_part = fill_func[i](next_part);
        record_data = (pldm_fru_record_table_field_fmt_t *)next_part;
    }
    table->head.fru_table_len = next_part - gs_pldm_fru_table - sizeof(pldm_fru_get_fru_record_table_metadata_rsp_dat_t);
}