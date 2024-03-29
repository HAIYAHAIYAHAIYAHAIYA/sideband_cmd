#ifndef __PLDM_FW_UPDATE_H__
#define __PLDM_FW_UPDATE_H__

#include "pldm_monitor.h"

/*
 * ud   : update
 * comp : component
 */

#define PLDM_FW_UPDATE_CMD                          (0x1D + 1)

#define PLDM_UPDATE_BASELINE_SIZE                   (32)                    // transfer min size, in bytes
#define PLDM_RECVBUF_MAX_SIZE                       (1024)

#define PLDM_PCI_ID_BASE                            (0x10000)               /* to be determind, read by nvm or reg */

#define PLDM_PCI_VENDOR_ID_REG                      (PLDM_PCI_ID_BASE + 0x0)
#define PLDM_PCI_DEV_ID_REG                         (PLDM_PCI_ID_BASE + 0x2)
#define PLDM_PCI_SUBSYS_VENDOR_ID_REG               (PLDM_PCI_ID_BASE + 0x2c)
#define PLDM_PCI_SUBSYS_ID_REG                      (PLDM_PCI_ID_BASE + 0x2e)
#define PLDM_PCI_REVISION_ID_REG                    (PLDM_PCI_ID_BASE + 0x8)

#define UPDATING_MODE                               TRUE
#define NON_UPDATE_MODE                             FALSE

#define PLDM_FWUP_RECV_IMG_NAME                  "pldm_fwup_slot.bin"

typedef enum {
    PLDM_UD_IDLE = 0,
    PLDM_UD_LEARN_COMP,
    PLDM_UD_READY_XFER,
    PLDM_UD_DOWNLOAD,
    PLDM_UD_VERIFY,
    PLDM_UD_APPLY,
    PLDM_UD_ACTIVATE
} pldm_fwup_state_t;

typedef enum {
    PLDM_UD_ENTER_UD_WITH_PKTDATA = 1,
    PLDM_UD_ENTER_UD_NO_PKTDATA,
    PLDM_UD_PASSCOMP_END,
    PLDM_UD_UD_COMP_END,
    PLDM_UD_TRANSFER_PASS,
    PLDM_UD_VERIFY_PASS,
    PLDM_UD_APPLY_PASS,
    PLDM_UD_ENTER_ACTIVATE,
    PLDM_UD_ACTIVATE_DONE,
    PLDM_UD_CANCEL_UD_COMP,
    PLDM_UD_CANCEL_UD_OR_TIMEOUT
} pldm_fwup_event_id_t;

typedef enum {
    PLDM_UD_TYPE_UNKNOW = 0,
    PLDM_UD_TYPE_ASCII,
    PLDM_UD_TYPE_UTF_8,
    PLDM_UD_TYPE_UTF_16,
    PLDM_UD_TYPE_UTF_16LE,
    PLDM_UD_TYPE_UTF_16BE
} pldm_fwup_str_type_val_t;

typedef enum {
    PLDM_UD_CLASS_UNKNOW = 0,
    PLDM_UD_CLASS_OTHER,
    PLDM_UD_CLASS_DRIVER,
    PLDM_UD_CLASS_CFG_SW,
    PLDM_UD_CLASS_APPL_SW,
    PLDM_UD_CLASS_INSTRUMENTATION,
    PLDM_UD_CLASS_FW_BIOS,
    PLDM_UD_CLASS_DIAG_SW,
    PLDM_UD_CLASS_OP_SYS,
    PLDM_UD_CLASS_MW,
    PLDM_UD_CLASS_FW,
    PLDM_UD_CLASS_BIOS_FCODE,
    PLDM_UD_CLASS_SUP_SERV_PACK,
    PLDM_UD_CLASS_SW_BUNDLE,
    PLDM_UD_CLASS_DOWNSTREAM_DEV = 0xFFFF
} pldm_fwup_comp_classification_val_t;

typedef enum {
    PLDM_PCI_VENDOR_ID = 0x0000,
    PLDM_PCI_DEV_ID = 0x0100,
    PLDM_PCI_SUBSYS_VENDOR_ID = 0x0101,
    PLDM_PCI_SUBSYS_ID = 0x0102,
    PLDM_PCI_REVISION_ID = 0x0103
} pldm_add_type_t;

typedef enum {
    PLDM_UD_FW_CLASSIFICATION = 0x000A,
    PLDM_UD_BIOS_FCODE_CLASSIFICATION = 0x000B,
    PLDM_UD_FW_BIOS_CLASSIFICATION = 0x0006
} pldm_fwup_comp_classification_t;

typedef enum {
    PLDM_UD_SLOT = 0,
    PLDM_UD_CHIP,
    PLDM_UD_FACTORY
    // PLDM_UD_OROM = 0x5,
    // PLDM_UD_NETLIST = 0x8
} pldm_fwup_comp_identifier_t;

typedef enum {
    PLDM_UD_NOT_IN_UPDATE_MODE = 0x80,
    PLDM_UD_ALREADY_IN_UPDATE_MODE,
    PLDM_UD_DATA_OUT_OF_RANGE,
    PLDM_UD_INVALID_TRANSFER_LENGTH,
    PLDM_UD_INVALID_STATE_FOR_COMMAND,
    PLDM_UD_INCOMPLETE_UPDATE,
    PLDM_UD_BUSY_IN_BACKGROUND,
    PLDM_UD_CANCEL_PENDING,
    PLDM_UD_COMMAND_NOT_EXPECTED,
    PLDM_UD_RETRY_REQUEST_FW_DATA,
    PLDM_UD_UNABLE_TO_INITIATE_UPDATE,
    PLDM_UD_ACTIVATION_NOT_REQUIRED,
    PLDM_UD_SELF_CONTAINED_ACTIVATION_NOT_PERMITTED,
    PLDM_UD_NO_DEVICE_METADATA,
    PLDM_UD_RETRY_REQUEST_UPDATE,
    PLDM_UD_NO_PACKAGE_DATA,
    PLDM_UD_INVALID_TRANSFER_HANDLE,
    PLDM_UD_INVALID_TRANSFER_OPERATION_FLAG,
    PLDM_UD_ACTIVATE_PENDING_IMAGE_NOT_PERMITTED,
    PLDM_UD_PACKAGE_DATA_ERROR,
    PLDM_UD_NO_OPAQUE_DATA,
    PLDM_UD_UPDATE_SECURITY_REVISION_NOT_PERMITTED,
    PLDM_UD_DOWNSTREAM_DEVICE_LIST_CHANGED                 /* 0x96 */
} pldm_fwup_cpl_code_t;

typedef enum {
    PLDM_UD_OP_IN_PROGRESS = 0,
    PLDM_UD_OP_SUC,
    PLDM_UD_OP_FAIL,
    PLDM_UD_OP_ELSE
} pldm_fwup_aux_state_t;

typedef enum {
    INITIALIZATION_OCCURRED = 0,
    ACTIVATEFIRMWARE_RECEIVED,
    CANCELUPDATE_RECEIVED,
    TIMEOUT_IN_LEARN_COMP_STATE,
    TIMEOUT_IN_READY_XFER_STATE,
    TIMEOUT_IN_DOWNLOAD_STATE,
    TIMEOUT_IN_VERIFY_STATE,
    TIMEOUT_IN_APPLY_STATE
} pldm_fwup_reason_code_t;

#pragma pack(1)

typedef struct {
    u16 add_type;
    u16 add_len;
    u16 add_data;
} pldm_add_descriptor_t;

typedef struct {
    u16 init_type;
    u16 init_len;
    u16 init_data;
    pldm_add_descriptor_t add_descriptor[0];    /* E810 : PCI Device ID, PCI Subsystem Vendor ID, PCI Subsystem ID, PCI Revision ID (option). */
} pldm_descriptor_t;

typedef struct {
    u32 dev_identifier_len;
    u8 descriptor_cnt;
    pldm_descriptor_t descriptor;
} pldm_query_dev_identifier_rsp_dat_t;

// typedef struct {
//     u32 comp_ud_fail_recovery_cap        : 1;
//     u32 comp_ud_fail_retry_cap           : 1;
//     u32 fw_dev_host_func_during_fw_ud    : 1;
//     u32 fw_dev_partial_ud                : 1;
//     u32 fw_dev_ud_mode_restriction       : 1;
//     u32 rsvd                             : 3;
//     u32 fw_dev_downgrade_restriction     : 1;
//     u32 security_revision_num_ud_req_sud : 1;
//     u32 rsvd1                            : 22;
// } pldm_fwup_cap_during_ud_t;

typedef struct {
    u32 comp_comparison_stamp;
    u8 comp_ver_str_type;
    u8 comp_ver_str_len;
} pldm_fwup_comp_ver_msg_t;

typedef struct {
    char actv_comp_img_set_ver_str[32];
    char pending_comp_img_set_ver_str[32];
} pldm_fwup_comp_img_set_ver_str_t;

typedef struct {
    char actv_comp_ver_str[18];
    char pending_comp_ver_str[18];
} pldm_fwup_nvm_comp_ver_str_t;

typedef struct {
    u8 comp_img_set_ver_str_type;
    u8 comp_img_set_ver_str_len;
} pldm_comp_img_set_ver_str_type_and_len_t;

typedef struct {
    u16 comp_classification;
    u16 comp_identifier;
    u8 comp_classification_idx;         /* not used */
} pldm_fwup_comp_class_msg_t;

typedef struct {
    pldm_fwup_comp_class_msg_t comp_class_msg;
    pldm_fwup_comp_ver_msg_t actv_comp_ver_msg;
    u8 actv_comp_release_date[8];
    pldm_fwup_comp_ver_msg_t pending_comp_ver_msg;
    u8 pending_comp_release_date[8];
    u16 comp_actv_meth;
    u32 cap_during_ud;
    pldm_fwup_nvm_comp_ver_str_t comp_ver_str;
} pldm_fwup_comp_param_table_t;

typedef struct {
    u32 cap_during_ud;
    u16 comp_cnt;
    pldm_comp_img_set_ver_str_type_and_len_t actv_comp_img_set_ver_str_type_and_len;
    pldm_comp_img_set_ver_str_type_and_len_t pending_comp_img_set_ver_str_type_and_len;
    pldm_fwup_comp_img_set_ver_str_t comp_img_set_ver_str;
    pldm_fwup_comp_param_table_t comp_param_table[0];
} pldm_get_fw_param_rsp_dat_t;

typedef struct {
    u32 max_transfer_size;
    u16 num_of_comp;
    u8 max_outstanding_transfer_req;
    u16 pkt_data_len;
    pldm_comp_img_set_ver_str_type_and_len_t comp_img_set_ver_str_type_and_len;
    char comp_img_set_ver_str[0];
} pldm_fwup_req_update_req_dat_t;

typedef struct {
    u16 fd_metadata_len;                        /*  Set to 0, as GetMetaData is not supported. */
    u8 fd_will_send_get_pkt_data_cmd;
    u16 get_pkt_data_max_transfer_size;
} pldm_fwup_req_update_rsp_dat_t;

typedef struct {
    u32 data_transfer_handle;
    u8 transfer_op_flag;
} pldm_fwup_get_pkt_data_req_dat_t;

typedef struct {
    u8 cpl_code;
    u32 next_data_transfer_handle;
    u8 transfer_flag;
    u8 portion_of_pkt_data[0];
} pldm_fwup_get_pkt_data_rsp_dat_t;

typedef struct {
    u8 transfer_flag;
    pldm_fwup_comp_class_msg_t comp_class_msg;
    pldm_fwup_comp_ver_msg_t comp_ver_msg;
    char comp_ver_str[0];
} pldm_fwup_pass_comp_table_req_dat_t;

typedef struct {
    u8 comp_rsp;
    u8 comp_rsp_code;
} pldm_fwup_pass_comp_table_rsp_dat_t;

typedef struct {
    pldm_fwup_comp_class_msg_t comp_class_msg;
    u32 comp_comparison_stamp;
    u32 comp_img_size;
    u32 ud_option_flag;
    u8 comp_ver_str_type;
    u8 comp_ver_str_type_len;
    char comp_ver_str[0];
} pldm_fwup_update_comp_req_dat_t;

typedef struct {
    u8 comp_compatibility_rsp;
    u8 comp_compatibility_rsp_code;
    u32 ud_option_flag_en;
    u16 estimated_time_before_send_req_fw_data;
    // u16 get_comp_opaque_data_max_transfer_size;          This field is only present if Bit 1 in UpdateOptionFlagsEnabled is set to 1.
} pldm_fwup_update_comp_rsp_dat_t;

typedef struct {
    u32 offset;
    u32 len;
} pldm_fwup_req_fw_data_req_dat_t;

typedef struct {
    u8 cpl_code;
    u32 comp_img_option[0];
} pldm_fwup_req_fw_data_rsp_dat_t;

typedef struct {
    union {
        u8 transfer_result;
        u8 verify_result;
    };
} pldm_fwup_transfer_cpl_req_dat_t, pldm_fwup_verify_cpl_req_dat_t;

typedef struct {
    u8 apply_result;
    u16 comp_actv_meth_modification;
} pldm_fwup_apply_cpl_req_dat_t;

typedef struct {
    u8 cpl_code;
} pldm_fwup_apply_cpl_rsp_dat_t, pldm_fwup_transfer_cpl_rsp_dat_t, pldm_fwup_verify_cpl_rsp_dat_t;

typedef struct {
    s8 self_contained_actv_req;                 /* Self contained activate is not supported. */
} pldm_fwup_actv_fw_req_dat_t;

typedef struct {
    u16 estimated_time_for_self_contained_actv; /* Should be set to 0 if no self-contained components. */
} pldm_fwup_actv_fw_rsp_dat_t;

typedef struct {
    u8 cur_state;
    u8 prev_state;
    u8 aux_state;
    u8 aux_state_status;
    u8 progress_percent;                        /*  IDLE – 0%
                                                    LEARN COMPONENTS – 0%
                                                    READY XFER – 0%
                                                    DOWNLOAD – 100% * num_downloaded / total_num_of_comp
                                                    VERIFY – 100% * num_verified / total_num_of_comp
                                                    APPLY – 100% * num_applied / total_num_of_comp
                                                    ACTIVATE – 0% */
    u8 reason_code;
    u32 ud_option_flag_en;
} pldm_fwup_get_status_rsp_dat_t;

typedef struct {
    u8 nonfunc_comp_indication;                 /* Always False meaning that all components will be functional. */
    u64 nonfunc_comp_bitmap;                    /* All zeros */
} pldm_fwup_cancel_ud_rsp_dat_t;

/* pldm fwup requested pkt hdr fmt */

typedef struct {
    u8 uuid[16];
    u8 pkt_hdr_fmt_reversion;
    u16 pkt_hdr_size;
    pldm_timestamp104_t pkt_release_datetime;
    u16 comp_bitmap_bitlen;
    u8 pkt_ver_str_type;
    u8 pkt_ver_str_len;
    u8 pkt_ver_str[0];
}pldm_fwup_pkt_hdr_t;

typedef struct {
    u16 record_len;
    u8 descriptor_cnt;
    u32 dev_up_op_flgs;
    u8 comp_img_set_ver_str_type;
    u8 comp_img_set_ver_str_len;
    u16 fw_dev_pkt_data_len;
    u8 applicable_comps;
    u8 comp_img_set_ver_str[0];
} pldm_fwup_fw_dev_id_records_first_part_t;

typedef struct {
    u16 add_type;
    u16 add_len;
    u8 add_data[0];
} pldm_add_descriptors_t;

typedef struct {
    pldm_add_descriptors_t descriptor;
} pldm_fwup_fw_dev_id_records_middle_part_t;

typedef struct {
    u8 fw_dev_pkt_data[0];
} pldm_fwup_fw_dev_id_records_end_part_t;

typedef struct {
    u8 dev_id_record_cnt;
    u8 fw_dev_id_records[0];
} pldm_fwup_fw_dev_indentification_area_t;

typedef struct {
    u16 comp_classification;
    u16 comp_identifier;
    u32 comp_comparison_stamp;
    u16 comp_options;
    u16 req_comp_acti_method;
    u32 comp_local_offset;
    u32 comp_size;
    u8 comp_ver_str_type;
    u8 comp_ver_str_len;
    u8 comp_ver_str[0];
} pldm_fwup_comp_img_info_first_part_t;

typedef struct {
    u32 pkt_hdr_crc32;
} pldm_fwup_comp_img_info_end_part_t;

// typedef struct {
//     u32 comp_opaque_data_len;
//     u8 comp_opaque_data[0];
// } pldm_fwup_comp_img_info_end_part_t;

typedef struct {
    u16 comp_img_cnt;
    u8 comp_img_info[0];
} pldm_fwup_comp_img_info_area_t;

/* pldm fwup requested pkt hdr fmt */

typedef struct {
    pldm_fwup_comp_class_msg_t comp_class_msg;
    pldm_fwup_comp_ver_msg_t comp_ver_msg;
    char comp_ver_str[18];
} pldm_fwup_comp_info_t;

typedef struct {
    pldm_comp_img_set_ver_str_type_and_len_t comp_img_set_ver_str_type_and_len;
    char comp_img_ver_str[32];
} pldm_fwup_comp_img_info_t;

typedef struct {
    int hw_id;
    u8 ua_eid;          /* update agent : bmc */
    u8 cur_state;
    u8 prev_state;
    u8 update_mode;
    u32 max_transfer_size;
    pldm_fwup_comp_info_t fw_new_ud_comp[3];
    // pldm_fwup_comp_info_t fw_cur_ud_comp[3];
    pldm_fwup_comp_img_info_t fw_new_ud_comp_img;
    // pldm_fwup_comp_img_info_t fw_cur_ud_comp_img;
} pldm_fwup_base_info_t;

typedef struct {
    u16 len;
    u32 pkt_data_len;
    u8 data[PLDM_RECVBUF_MAX_SIZE];
} pldm_fwup_pkt_data_t;

typedef void (*do_action)(void);

typedef struct {
    pldm_fwup_state_t cur_state;
    pldm_fwup_event_id_t event_id;
    pldm_fwup_state_t next_state;
    do_action action;
} pldm_fwup_state_transform_t;

typedef int (*pldm_fw_upgrade_callback)(void *store, int id, u8 *dat, int len);
typedef int (*pldm_fw_upgrade_complete_callback)(void *store, int total_len);
typedef int (*pldm_fw_upgrade_cancer_callback)(void *store, int total_len);

typedef struct {
    pldm_fw_upgrade_callback *upgrade_callback_func;
    pldm_fw_upgrade_complete_callback *upgrade_complete_callback_func;
    pldm_fw_upgrade_cancer_callback *upgrade_cancer_callback_func;
} pldm_fwup_upgrade_func_t;

typedef struct {
    u64 enter_upgrade_time;
    u64 req_time;
} pldm_fwup_time_def_t;

#pragma pack()

void pldm_fwup_process(protocol_msg_t *pkt, int *pkt_len, u32 cmd_code);
void pldm_fwup_init(void);
void pldm_fwup_state_machine_switch(void);
void pldm_fwup_upgrade_handle(void);

#endif /* __PLDM_FW_UPDATE_H__ */