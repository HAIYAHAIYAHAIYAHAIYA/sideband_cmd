#ifndef __PLDM_FRU_DATA_H__
#define __PLDM_FRU_DATA_H__

#include "protocol_device.h"
#include "pldm_monitor.h"

#define PLDM_FRU_DATA_CMD               (0x02 + 1)
#define PLDM_FRU_TABLE_BUF_LEN          (1024)              /* 4 LAN for 892 bytes, 2 LAN for 522 bytes */
/* Attention ! DSP0257 pldm fru data spec do not have transfer buffersize. pldm_fru_get_fru_record_table rsp cmd's behavior may be err.  */
#define PLDM_FRU_TRANSFER_BUFFERSIZE    PLDM_TERMINUS_DEFAULT_BUFFERSIZE

typedef enum {
    RECORD_GENERAL_FRU_RECORD_TYPE = 0x01,
    RECORD_OEM_FRU_RECORD_TYPE = 0xFE
} pldm_fru_record_type_t;

typedef enum {
    FIELD_CLASSIS_TYPE = 0x01,
    FIELD_VENDOR_IANA_TYPE = 0x01,      /* RECORD_OEM_FRU_RECORD_TYPE */
    FIELD_MODULE_TYPE,
    FIELD_PART_NUM_TYPE,
    FIELD_SERIAL_NUM_TYPE,
    FIELD_MANUFACTURER_TYPE,
    FIELD_MANUFACTURER_DATE_TYPE,
    FIELD_VENDOR_TYPE,
    FIELD_NAME_TYPE,
    FIELD_SKU_TYPE,
    FIELD_VERSION_TYPE,
    FIELD_ASSET_TAG_TYPE,
    FIELD_DESCRIPTION_TYPE,
    FIELD_ENG_CHG_LVL_TYPE,
    FIELD_OTHER_INFO_TYPE,
    // FIELD_VENDOR_IANA_TYPE = 0xF,             /* RECORD_GENERAL_FRU_RECORD_TYPE */
} pldm_fru_field_type_t;

typedef enum {
    FRU_GENERAL_SET_ID = 0x01,
    FRU_CHIP_SET_ID = 0x03,
    FRU_PORT0_SET_ID,
    FRU_PORT1_SET_ID,
    FRU_PORT2_SET_ID,
    FRU_PORT3_SET_ID,
} pldm_fru_record_set_id_t;

typedef enum {
    PLDM_FRU_TRANSFRT_FLG_START = 0x01,
    PLDM_FRU_TRANSFRT_FLG_MIDDLE = 0x02,
    PLDM_FRU_TRANSFRT_FLG_END = 0x04,
    PLDM_FRU_TRANSFRT_FLG_START_AND_END = 0x05
} pldm_fru_multi_transfer_flg_t;

#pragma pack(1)

typedef struct {
    u8 fru_data_major_ver;
    u8 fru_data_minor_ver;
    u32 fru_table_maxi_size;
    u32 fru_table_len;
    u16 num_of_records_set_identifiers;
    u16 num_of_records;
    // u32 crc32;
} pldm_fru_get_fru_record_table_metadata_rsp_dat_t;

typedef struct {
    u32 data_transfer_hdl;
    u8 transfer_op_flg;
} pldm_fru_get_fru_record_table_req_dat_t;

typedef struct {
    u32 next_data_transfer_hdl;
    u8 transfer_flg;
    u8 portion_of_data[0];
} pldm_fru_get_fru_record_table_rsp_dat_t;

typedef struct {
    u8 type;
    u8 len;
    char val[0];
} pldm_fru_tlv_fmt_t;

typedef struct {
    u16 fru_record_set_identifier;
    u8 fru_record_type;
    u8 num_of_fru_fields;
    u8 encode_type_of_fru_field;
    u8 tlv[0];
} pldm_fru_record_table_field_fmt_t;

typedef struct {
    pldm_fru_get_fru_record_table_metadata_rsp_dat_t head;
    pldm_fru_record_table_field_fmt_t field;
} pldm_fru_record_table_fmt_t;

#pragma pack()

typedef u8 *(pldm_fru_fill_table)(u8 *);

void pldm_fru_init(void);
void pldm_fru_process(protocol_msg_t *pkt, int *pkt_len, u32 cmd_code);

#endif /* __PLDM_FRU_DATA_H__ */