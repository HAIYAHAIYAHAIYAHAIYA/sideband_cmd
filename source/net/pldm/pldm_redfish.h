#ifndef __PLDM_REDFISH_H__
#define __PLDM_REDFISH_H__

#include "protocol_device.h"

#define PLDM_REDFISH_CMD                                    (0x16 + 1)
#define MINIMAL_MCTP_PACKET                                 (64)
#define PLDM_REDFISH_DEV_MAXIMUM_XFER_CHUNKSIZE_BYTES       (2048)
#define PLDM_REDFISH_DICT_NUM                               (14)
#define PLDM_REDFISH_DICT_INFO_LEN                          ALIGN((sizeof(pldm_redfish_dict_hdr_t) + PLDM_REDFISH_DICT_NUM * sizeof(pldm_redfish_dict_info_t)), 4)

#define NETWORK_ADAPTER_SCHEMACLASS                         (BIT(SCHEMACLASS_MAJOR))
#define NETWORK_INTERFACE_SCHEMACLASS                       (BIT(SCHEMACLASS_MAJOR))
#define PCIE_DEVICE_SCHEMACLASS                             (BIT(SCHEMACLASS_MAJOR))
#define PORT_COLLECTION_SCHEMACLASS                         (BIT(SCHEMACLASS_MAJOR) | BIT(SCHEMACLASS_COLLECTION_MEMBER_TYPE))
#define NETWORK_DEVICE_FUNC_COLLECTION_SCHEMACLASS          (BIT(SCHEMACLASS_MAJOR) | BIT(SCHEMACLASS_COLLECTION_MEMBER_TYPE))
#define PCIE_FUNC_COLLECTION_SCHEMACLASS                    (BIT(SCHEMACLASS_MAJOR) | BIT(SCHEMACLASS_COLLECTION_MEMBER_TYPE))
#define PORT_IDENTIFY_SCHEMACLASS                           (BIT(SCHEMACLASS_MAJOR))
#define NETWORK_DEVICE_FUNC_SCHEMACLASS                     (BIT(SCHEMACLASS_MAJOR))
#define PCI_FUNC_SCHEMACLASS                                (BIT(SCHEMACLASS_MAJOR))
#define ETH_INTERFACE_SCHEMACLASS                           (BIT(SCHEMACLASS_MAJOR))
#define ETH_INTERFACE_COLLECTION_SCHEMACLASS                (BIT(SCHEMACLASS_MAJOR) | BIT(SCHEMACLASS_COLLECTION_MEMBER_TYPE))
#define ALL_SCHEMA_SCHEMACLASS                              (BIT(SCHEMACLASS_EVENT) | BIT(SCHEMACLASS_ANNOTATION) | BIT(SCHEMACLASS_ERROR))

#define SCHEMA_CLASS(schema_name)                           schema_name##_SCHEMACLASS

#define NETWORK_ADAPTER_ALLOWED_OP                          (BIT(READ) | BIT(HEAD) | BIT(UPDATE) | BIT(REPLACE)) | BIT(ACTION)
#define NETWORK_INTERFACE_ALLOWED_OP                        (BIT(READ) | BIT(HEAD) | BIT(UPDATE) | BIT(REPLACE))
#define PCIE_DEVICE_ALLOWED_OP                              (BIT(READ) | BIT(HEAD) | BIT(UPDATE) | BIT(REPLACE))
#define PORT_COLLECTION_ALLOWED_OP                          (BIT(READ) | BIT(HEAD))
#define NETWORK_DEVICE_FUNC_COLLECTION_ALLOWED_OP           (BIT(READ) | BIT(HEAD))
#define PCIE_FUNC_COLLECTION_ALLOWED_OP                     (0)
#define PORT_IDENTIFY_ALLOWED_OP                            (BIT(READ) | BIT(HEAD) | BIT(UPDATE) | BIT(REPLACE)) | BIT(ACTION)
#define NETWORK_DEVICE_FUNC_ALLOWED_OP                      (BIT(READ) | BIT(HEAD) | BIT(UPDATE) | BIT(REPLACE))
#define PCI_FUNC_ALLOWED_OP                                 (BIT(READ) | BIT(HEAD) | BIT(UPDATE) | BIT(REPLACE))
#define ETH_INTERFACE_ALLOWED_OP                            (BIT(READ) | BIT(HEAD) | BIT(UPDATE) | BIT(REPLACE))
#define ETH_INTERFACE_COLLECTION_ALLOWED_OP                 (BIT(READ) | BIT(HEAD))

#define SCHEMA_ALLOWED_OP(schema_name)                      schema_name##_ALLOWED_OP

/*  annotation.bin                          2560
    EthernetInterfaceCollection_v1.bin      164
    EthernetInterface_v1.bin                3068
    Event_v1.bin                            1084
    MessageRegistry_v1.bin                  276
    NetworkAdapter_v1.bin                   4072
    NetworkDeviceFunctionCollection_v1.bin  168
    NetworkDeviceFunction_v1.bin            3432
    NetworkInterface_v1.bin                 872
    PCIeDeviceCollection_v1.bin             156
    PCIeDevice_v1.bin                       4244
    PCIeFunction_v1.bin                     1944
    PortCollection_v1.bin                   148
    Port_v1.bin                             8076
 */

#define DICT_FMT_HDR_LEN                                    (sizeof(pldm_redfish_dict_fmt_t))

#define PLDM_REDFISH_ANNO_DICT_LEN                          (2560)
#define PLDM_REDFISH_ETH_INTERFACE_COLLECTION_DICT_LEN      (164)
#define PLDM_REDFISH_ETH_INTERFACE_DICT_LEN                 (3068)
#define PLDM_REDFISH_EVENT_DICT_LEN                         (1084)
#define PLDM_REDFISH_MSG_REGISTER_DICT_LEN                  (276)
#define PLDM_REDFISH_NETWORK_ADAPTER_DICT_LEN               (4072)
#define PLDM_REDFISH_NETWORK_DEV_FUNCS_DICT_LEN             (168)
#define PLDM_REDFISH_NETWORK_DEV_FUNC_DICT_LEN              (3432)
#define PLDM_REDFISH_NETWORK_INTERFACE_DICT_LEN             (872)
#define PLDM_REDFISH_PCIE_FUNCS_DICT_LEN                    (156)
#define PLDM_REDFISH_PCIE_DEV_DICT_LEN                      (4244)
#define PLDM_REDFISH_PCIE_FUNC_DICT_LEN                     (1944)
#define PLDM_REDFISH_PORTS_DICT_LEN                         (148)
#define PLDM_REDFISH_PORT_DICT_LEN                          (8076)

typedef u16                                                 rdeopid;
typedef u8                                                  schemaclass;

typedef enum {
    PLDM_REDFISH_TRANSFER_START = 0,
    PLDM_REDFISH_TRANSFER_MIDDLE,
    PLDM_REDFISH_TRANSFER_END,
    PLDM_REDFISH_TRANSFER_START_END
} pldm_redfish_transferflags_t;

typedef enum {
    XFER_FIRST_PART = 0,
    XFER_NEXT_PART,
    XFER_ABORT,
    XFER_COMPLETE
} pldm_redfish_transferoperation_t;

typedef enum {
    OPERATION_INACTIVE = 0,
    OPERATION_NEEDS_INPUT = 1,
    OPERATION_TRIGGERED= 2,
    OPERATION_RUNNING = 3,
    OPERATION_HAVE_RESULTS = 4,
    OPERATION_COMPLETED = 5,
    OPERATION_FAILED = 6,
    OPERATION_ABANDONED = 7
} pldm_redfish_state_definetions_t;

typedef enum {
    ETAG_IGNORE = 0,
    ETAG_IF_MATCH = 1,
    ETAG_IF_NONE_MATCH = 2
} etag_op_t;

typedef enum {
    HEAD = 0,
    READ,
    CREATE,
    DELETE,
    UPDATE,
    REPLACE,
    ACTION,
    EVENTS,
    OTHER_TYPE
} pldm_redfish_dev_feature_support_t;

typedef enum {
    EVENT = 0,
    REDFISH_PAYLOAD_ANNOTATIONS,
    REDFISH_ERROR,
    REGISTRY,

    NETWORK_ADAPTER,
    PCIE_DEVICE,
    NETWORK_INTERFACE,
    PORT_COLLECTION,
    PCIE_FUNC_COLLECTION,
    NETWORK_DEVICE_FUNC_COLLECTION,
    NETWORK_DEVICE_FUNC,
    PORT_IDENTIFY,
    PCI_FUNC,
    ETH_INTERFACE,
    ETH_INTERFACE_COLLECTION,
    ALL_SCHEMA,
    ALL_SCHEMA_IDENTIFY
} pldm_redfish_schema_identify_t;

typedef enum {
    SCHEMACLASS_MAJOR = 0,
    SCHEMACLASS_EVENT,
    SCHEMACLASS_ANNOTATION,
    SCHEMACLASS_COLLECTION_MEMBER_TYPE,
    SCHEMACLASS_ERROR,
    SCHEMACLASS_REGISTRY
} pldm_redfish_schemaclass_t;

typedef enum {
    PLDM_ERROR_NOT_READY = 0x04,

    PLDM_ERROR_BAD_CHECKSUM = 0x80,
    PLDM_ERROR_NOT_ALLOWED = 0X82,
    PLDM_ERROR_OPERATION_ABANDONED = 0x84,
    PLDM_ERROR_OPERATION_UNKILLABLE = 0x85,
    PLDM_ERROR_OPERATION_EXISTS = 0x86,
    PLDM_ERROR_OPERATION_FAILED = 0x87,
    PLDM_ERROR_UNEXPECTED = 0x88,
    PLDM_ERROR_UNSUPPORTED = 0x89,
    PLDM_ERROR_ETAG_MATCH = 0x91,
    PLDM_ERROR_NO_SUCH_RESOURCE = 0x92
} pldm_redfish_cpl_code_t;

typedef enum {
    /* Text format: */
    RAW_UTF_8 = 0,
    GZIP_UTF_8 = 1,
    /* Schema format: */
    JSON = 0x10,
    CSDL = 0x20,
    YAML = 0x30
} pldm_schema_fmt_t;

typedef enum {
    UNKNOW = 0,
    ASCII,
    UTF_8,
    UTF_16,
    UTF_16LE,
    UTF_16BE
} pldm_etag_fmt_t;

typedef enum {
    PLDM_TRANSFER_FLG_START = 0x00,
    PLDM_TRANSFER_FLG_MIDDLE = 0x01,
    PLDM_TRANSFER_FLG_END = 0x02,
    PLDM_TRANSFER_FLG_START_AND_END = 0x03,
} pldm_multi_recv_transfer_flag_t;

#pragma pack(1)
typedef struct {
    u8 format;  // UNKNOWN = 0, ASCII = 1, UTF-8 = 2, UTF-16 = 3, UTF-16LE = 4, UTF-16BE = 5, defalut UTF-8
    u8 len;     // in bytes, Including null terminator
    u8 val[0];     // Must be null terminated
} varstring;

typedef struct {
    u8 mc_concurrency_support;
    u16 mc_feature_support;
} pldm_redfish_negotiate_redfish_parameters_req_dat_t;

typedef struct {
    u8 dev_concurrency_support;
    u8 dev_capabilities_flg;
    u16 dev_feature_support;
    u32 dev_cfg_signature;
    varstring dev_provider_name;
} pldm_redfish_negotiate_redfish_parameters_rsp_dat_t;

typedef struct {
    u32 mc_maximum_xfer_chunksize_bytes;
} pldm_redfish_negotiate_medium_parameters_req_dat_t;

typedef struct {
    u32 mc_maximum_xfer_chunksize_bytes;
} pldm_redfish_negotiate_medium_parameters_rsp_dat_t;

typedef struct {
    u32 resource_id;
    schemaclass requested_schemaclass;           /* Table 12-89. Handle per Resource */
} pldm_redfish_get_schema_dictionary_req_dat_t;

typedef struct {
    u8 dictionary_fmt;
    u32 transfer_handle;
} pldm_redfish_get_schema_dictionary_rsp_dat_t;

typedef struct {
    u32 resource_id;
    schemaclass requested_schemaclass;
    u8 oem_extension_num;
} pldm_redfish_get_schema_uri_req_dat_t;

typedef struct {
    u8 string_frag_cnt;
    char schema_uri[0];                 /* Table 12-90. URI per Resource */
} pldm_redfish_get_schema_uri_rsp_dat_t;

typedef struct {
    u32 resource_id;
} pldm_redfish_get_resource_etag_req_dat_t;

typedef struct {
    u32 eTag;
} pldm_redfish_get_resource_etag_rsp_dat_t;

typedef struct {
    u32 resource_id;
    u8 requested_schemaclass;
} pldm_redfish_get_oem_count_req_dat_t;

typedef struct {
    u8 oem_cnt;                         /* For schema classes that do not support OEM extensions this value must be zero. */
} pldm_redfish_get_oem_count_rsp_dat_t;

typedef struct {
    u32 resource_id;
    u8 requested_schemaclass;
    u8 oem_idx;
} pldm_redfish_get_oem_name_req_dat_t;

typedef struct {
    char oem_mane[0];
} pldm_redfish_get_oem_name_rsp_dat_t;

typedef struct {
    u8 registry_cnt;
} pldm_redfish_get_registry_count_rsp_dat_t;

typedef struct {
    u8 registry_idx;
} pldm_redfish_get_registry_details_req_dat_t;

typedef struct {                      /* Currently, the lexicographic reverse numeric order is not an issue as only a single registry is supported. */
    char registry_prefix[20];
    char registry_uri[20];
    u8 registry_language[2];
    u8 ver_cnt;
    u32 version[0];
} pldm_redfish_get_registry_details_rsp_dat_t;

typedef struct {
    u8 registry_idx;
} pldm_redfish_get_msg_registry_req_dat_t;

typedef struct {
    u8 schema_fmt;
    u32 transfer_handle;
} pldm_redfish_get_msg_registry_rsp_dat_t;

typedef struct {
    u8 cnt;
    u8 val;     /* max 0xFF, DSP0218 5.3.3 nnint PLDM data type */
} nnint_t, bejtuples_t;

typedef struct {
    nnint_t nnint;
    bejtuples_t seq_num[0];
} bejlocator_t;

typedef struct {
    u32 resource_id;
    rdeopid op_id;
} pldm_redfish_op_identify_t;

typedef struct {
    pldm_redfish_op_identify_t op_identify;
    u8 op_type;
    u8 op_flg;
    u32 senddata_transfer_handle;
    u8 op_locator_len;
    u32 req_payload_len;
    u8 op_locator[0];
    /* null or bejEncoding RequestPayload; */
} pldm_redfish_rde_operation_init_req_dat_t;

typedef struct {
    u32 ver;
    u16 rsvd;
    schemaclass schema_class;
    u8 bejtuple[0];
    /* bejtuple_t */
} bejencoding_t;

typedef struct {
    u8 op_status;
    u8 cpl_percentage;
    u32 cpl_time_sec;                       /* Return 0xFFFF FFFF - no support */
    u8 op_execution_flg;
    u32 result_transfer_handle;
    u8 permission_flg;      // Indicates the access level granted to the resource targeted by the Operation.
    u32 rsp_payload_len;
    varstring etag;
    /* null or bejEncoding RequestPayload; */
} pldm_redfish_rde_operation_init_rsp_dat_t;

typedef struct {
    pldm_redfish_op_identify_t op_identify;
    u16 link_expand;
    u16 collection_skip;
    u16 collection_top;
    u16 pagination_offset;                  /* This value is ignored. The device does not do pagination. */
    u8 etag_op;
    u8 etag_cnt;
    varstring etag;
    // u8 header_cnt;                       Currently no support, so must be zero.
    // headername[0]
    // headerparameter[0]
} pldm_redfish_supply_custom_request_parameters_req_dat_t;

typedef struct {
    u8 op_status;
    u8 cpl_percentage;
    u32 cpl_time_sec;
    u8 op_execution_flg;
    u32 result_transfer_handle;
    u8 permission_flg;
    u32 rsp_payload_len;
    varstring etag;
} pldm_redfish_supply_custom_request_parameters_rsp_dat_t;

typedef struct {
    pldm_redfish_op_identify_t op_identify;
} pldm_redfish_retrieve_custom_response_parameters_req_dat_t, pldm_redfish_rde_operation_complete_req_dat_t, pldm_redfish_rde_operation_status_req_dat_t;

typedef struct {
    u32 deferral_timeframe;
    u32 new_resource_id;
    u8 rsp_hdr_cnt;                         /* Return 0 - no support for custom headers. */
    // hdr_name[0]
    // hdr_param[0]
} pldm_redfish_retrieve_custom_response_parameters_rsp_dat_t;

typedef struct {
    u8 op_status;
    u8 cpl_percentage;
    u32 cpl_time_sec;
    u8 op_execution_flg;
    u32 result_transfer_handle;
    u8 permission_flg;
    u32 rsp_payload_len;
    varstring etag;
    // bejEncoding rsp_payload[0];
} pldm_redfish_rde_operation_status_rsp_dat_t;

typedef struct {
    pldm_redfish_op_identify_t op_identify;
    u8 kill_flg;
} pldm_redfish_rde_operation_kill_req_dat_t;

typedef struct {
    pldm_redfish_op_identify_t op_identify;
    u8 op_type;
} pldm_redfish_rde_operation_enumerate_field_t;

typedef struct {
    u16 op_cnt;
    pldm_redfish_rde_operation_enumerate_field_t field[0];
} pldm_redfish_rde_operation_enumerate_rsp_dat_t;

typedef struct {
    u32 data_transfer_handle;
    rdeopid op_id;
    u8 transfer_flg;
    u32 next_transfer_handle;
    u32 data_len_bytes;
    u8 data[0];
    /* u32 data_ic; TransferFlag = END or START_AND_END */
} pldm_redfish_rde_multipart_send_req_dat_t;

typedef struct {
    u8 transfer_op;
} pldm_redfish_rde_multipart_send_rsp_dat_t;

typedef struct {
    u32 data_transfer_handle;
    rdeopid op_id;
    u8 transfer_op;
} pldm_redfish_rde_multipart_receive_req_dat_t;

typedef struct {
    u8 transfer_flg;
    u32 next_transfer_handle;
    u32 data_len_bytes;
    u8 data[0];
    /* u32 data_ic; TransferFlag = END or START_AND_END */
} pldm_redfish_rde_multipart_receive_rsp_dat_t;

typedef struct {
    u8 schema_class;
    u8 allowed_op;
    char uri[2][39];
} pldm_redfish_schema_info_t;

typedef struct {
    u16 len;
    u8 data[PLDM_REDFISH_DEV_MAXIMUM_XFER_CHUNKSIZE_BYTES];
} pldm_payload_dat_t;

typedef struct {
    u16 len;	// transfer data len, unit is bytes
    u8 *data;
} pldm_send_dat_t;

typedef struct {
    u8 op_type;
    u8 op_flag;
    u8 dev_status;
    u16 collection_skip;
    u16 collection_top;
} op_info_t;    // Supported for Read, Update, and Action operations.

typedef struct {
    pldm_payload_dat_t op_data;
    pldm_payload_dat_t op_result;
    u8 op_locator[256];
    u32 multipartrecv_first_transfer_handle;
    u32 multipartrecv_next_transfer_handle;
    u32 multipartsend_first_transfer_handle;
    u32 multipartsend_next_transfer_handle;
    pldm_send_dat_t buf_ptr;
} op_data_buf_t;

typedef struct {
    pldm_redfish_op_identify_t prev_op_identify;
    u32 mc_maximum_xfer_chunksize_bytes;        /* for MultipartSend/Receive commands. */
} pldm_redfish_base_info_t;

typedef struct {
    u8 is_bej;
    u8 data[260];
} pldm_redfish_bej_t;

typedef struct {
    u8 format;
    u16 sequence_num;
    u16 childpoint_off;
    u16 child_cnt;
    u8 name_len;
    u16 name_off;
} pldm_redfish_dictionary_entry_t;

typedef struct {
    u8 version_tag;
    u8 dictionay_flags;
    u16 entry_cnt;  // name count
    u32 schema_version;
    u32 dictionary_size;
    pldm_redfish_dictionary_entry_t entry[0];
    // u8 name[0];
    // u8 copyright_len;
    // u8 copyright[0];
} pldm_redfish_dictionary_format_t;

typedef struct {
    u8 copyright_len;
    u8 copyright[0];
} pldm_redfish_dictionary_copyright_t;

typedef struct {
    u16 len;
    u32 dict_sign;
    u8 data[0];
} pldm_redfish_dict_fmt_t;

typedef struct {
    u32 resource_id;
    u16 schema_class;
    u16 offset;
} pldm_redfish_dict_info_t;

typedef struct {
    u16 total_len;
    u16 num_of_dict;
    pldm_redfish_dict_info_t dict_info[0];
} pldm_redfish_dict_hdr_t;

#pragma pack()

void pldm_redfish_init(void);
void pldm_redfish_op_triggered(void);
void pldm_redfish_op_task(void *param);
void pldm_redfish_process(protocol_msg_t *pkt, int *pkt_len, u32 cmd_code);
u8 pldm_redfish_get_dict_data(u32 resource_id, u8 *dict, u16 len);

#endif /* __PLDM_REDFISH_H__ */