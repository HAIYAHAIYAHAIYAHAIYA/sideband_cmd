#ifndef __PLDM_MONITOR_H__
#define __PLDM_MONITOR_H__

#include "protocol_device.h"
#include "mctp_ctl.h"
#include "pdr.h"

#define PLDM_MONITOR_CMD                          (0x15 + 1)

#define PLDM_TERMINUS_MAX_BUFFERSIZE              (2 * 1024)
#define PLDM_TERMINUS_DEFAULT_BUFFERSIZE          (256)
#define PLDM_EVENT_RECEIVER_MIN_BUFFERSIZE        (14)
#define PLDM_EVENT_FORMAT_VERSION                 (0x01)
#define PLDM_TERMINUS_HANDLE                      (0x01)
#define PLDM_ERR_RECORD_HANDLE                    (0xFFFFFFFF)
/* sizeof(pldm_poll_for_platform_event_msg_rsp_dat_t) + sizeof(pldm_response_t) */
#define PLDM_SYNC_SEND_FIELD_LEN                  (17)
/* sizeof(pldm_platform_event_msg_req_dat_t) + sizeof(pldm_response_t) */
#define PLDM_ASYNC_SEND_FIELD_LEN                 (7)
#define PLDM_REDFISH_EXTERNAL                     (0)

/* NIC : Network Interface Card
   NC  : Network Controller
   E810 Table 12-80 */

/* Sensor num */

#define PLDM_NIC_NUM                              (1)
#define PLDM_CARD_COMPOSITE_STATE_SENSOR_NUM      (1)

#define PLDM_CONNECTOR_NUM                        (1)
#define PLDM_NIC_TEMP_SENSOR_NUM                  (1)

#define PLDM_NC_NUM                               (1)
#define PLDM_NC_STATE_SENSOR_NUM                  (1)
#define PLDM_PORT_OF_NC                           (4)           /* no sensor id ? */
#define PLDM_LINK_SPEED_SENSOR_NUM                (4)
#define PLDM_LINK_STATE_SENSOR_NUM                (4)
#define PLDM_NC_TEMP_SENSOR_NUM                   (1)

#define PLDM_PLUG_NUM                             (4)
#define PLDM_PLUG_POWER_SENSOR_NUM                (4)
#define PLDM_PLUG_TEMP_SENSOR_NUM                 (4)
#define PLDM_PLUG_COMPOSITE_SENSOR_NUM            (4)

#define PLDM_COMM_CHANNEL_NUM                     (4)

/* used */

#define PLDM_USED_PORT_OF_NC                      (MAX_LAN_NUM)           /* no sensor id ? */
#define PLDM_USED_LINK_SPEED_SENSOR_NUM           (MAX_LAN_NUM)
#define PLDM_USED_LINK_STATE_SENSOR_NUM           (MAX_LAN_NUM)

#define PLDM_USED_PLUG_NUM                        (MAX_LAN_NUM)
#define PLDM_USED_PLUG_POWER_SENSOR_NUM           (MAX_LAN_NUM)
#define PLDM_USED_PLUG_TEMP_SENSOR_NUM            (MAX_LAN_NUM)
#define PLDM_USED_PLUG_COMPOSITE_SENSOR_NUM       (MAX_LAN_NUM)

#define PLDM_USED_COMM_CHANNEL_NUM                (MAX_LAN_NUM)

/* Sensor id */

#define PLDM_BASE_CARD_COMPOSITE_STATE_SENSOR_ID  (5)
#define PLDM_BASE_NIC_TEMP_SENSOR_ID              (20)
#define PLDM_BASE_NC_STATE_SENSOR_ID              (60)
#define PLDM_BASE_LINK_SPEED_SENSOR_ID            (100)
#define PLDM_BASE_LINK_STATE_SENSOR_ID            (200)
#define PLDM_BASE_NC_TEMP_SENSOR_ID               (300)

#define PLDM_BASE_PLUG_POWER_SENSOR_ID            (400)
#define PLDM_BASE_PLUG_TEMP_SENSOR_ID             (500)
#define PLDM_BASE_PLUG_COMPOSITE_SENSOR_ID        (700)

#define PLDM_MAX_CARD_COMPOSITE_STATE_SENSOR_ID   (PLDM_BASE_CARD_COMPOSITE_STATE_SENSOR_ID + PLDM_CARD_COMPOSITE_STATE_SENSOR_NUM - 1)
#define PLDM_MAX_NIC_TEMP_SENSOR_ID               (PLDM_BASE_NIC_TEMP_SENSOR_ID + PLDM_NIC_TEMP_SENSOR_NUM - 1)
#define PLDM_MAX_NC_STATE_SENSOR_ID               (PLDM_BASE_NC_STATE_SENSOR_ID + PLDM_NC_STATE_SENSOR_NUM - 1)
#define PLDM_MAX_LINK_SPEED_SENSOR_ID             (PLDM_BASE_LINK_SPEED_SENSOR_ID + PLDM_LINK_SPEED_SENSOR_NUM - 1)
#define PLDM_MAX_LINK_STATE_SENSOR_ID             (PLDM_BASE_LINK_STATE_SENSOR_ID + PLDM_LINK_STATE_SENSOR_NUM - 1)
#define PLDM_MAX_NC_TEMP_SENSOR_ID                (PLDM_BASE_NC_TEMP_SENSOR_ID + PLDM_NC_TEMP_SENSOR_NUM - 1)

#define PLDM_MAX_PLUG_POWER_SENSOR_ID             (PLDM_BASE_PLUG_POWER_SENSOR_ID + PLDM_PLUG_POWER_SENSOR_NUM - 1)
#define PLDM_MAX_PLUG_TEMP_SENSOR_ID              (PLDM_BASE_PLUG_TEMP_SENSOR_ID + PLDM_PLUG_TEMP_SENSOR_NUM - 1)
#define PLDM_MAX_PLUG_COMPOSITE_SENSOR_ID         (PLDM_BASE_PLUG_COMPOSITE_SENSOR_ID + PLDM_PLUG_COMPOSITE_SENSOR_NUM - 1)

/* Handle */

#define PLDM_TERMINUS_LOCATOR_PDR_HANDLE              (0x10)
#define PLDM_FRU_RECORD_SET_PDR_HANDLE                (2200)

#define PLDM_BASE_NIC_HANDLE                          (1100)
#define PLDM_BASE_CARD_COMPOSITE_STATE_SENSOR_HANDLE  (1101)
#define PLDM_BASE_CONNECTOR_HANDLE                    (1110)
#define PLDM_BASE_NIC_TEMP_SENSOR_HANDLE              (1130)
#define PLDM_BASE_NC_HANDLE                           (1150)
#define PLDM_BASE_NC_STATE_SENSOR_HANDLE              (1170)
#define PLDM_BASE_PORT_OF_NC_HANDLE                   (1200)
#define PLDM_BASE_LINK_SPEED_SENSOR_HANDLE            (1300)
#define PLDM_BASE_LINK_STATE_SENSOR_HANDLE            (1400)
#define PLDM_BASE_NC_TEMP_SENSOR_HANDLE               (1500)

#define PLDM_BASE_PLUG_HANDLE                         (1600)
#define PLDM_BASE_PLUG_POWER_SENSOR_HANDLE            (1700)
#define PLDM_BASE_PLUG_TEMP_SENSOR_HANDLE             (1800)
#define PLDM_BASE_PLUG_COMPOSITE_SENSOR_HANDLE        (2000)
#define PLDM_BASE_COMM_CHANNEL_HANDLE                 (2100)

/* assosiation pdr handle */

#define PLDM_BASE_NIC_ASSOC_PDR_HANDLE                (1)
#define PLDM_BASE_NC_ASSOC_PDR_HANDLE                 (PLDM_BASE_NIC_ASSOC_PDR_HANDLE + PLDM_NC_NUM)
#define PLDM_BASE_CONNECTOR_ASSOC_PDR_HANDLE          (PLDM_BASE_NC_ASSOC_PDR_HANDLE + PLDM_CONNECTOR_NUM)
#define PLDM_BASE_PLUG_ASSOC_PDR_HANDLE               (PLDM_BASE_CONNECTOR_ASSOC_PDR_HANDLE + PLDM_PLUG_NUM)
#define PLDM_BASE_COMM_CHAN_ASSOC_PDR_HANDLE          (PLDM_BASE_PLUG_ASSOC_PDR_HANDLE + PLDM_PLUG_NUM)

#define PLDM_MAX_NIC_HANDLE                           (PLDM_BASE_NIC_HANDLE + PLDM_NIC_NUM - 1)
#define PLDM_MAX_CARD_COMPOSITE_STATE_SENSOR_HANDLE   (PLDM_BASE_CARD_COMPOSITE_STATE_SENSOR_HANDLE + PLDM_CARD_COMPOSITE_STATE_SENSOR_NUM - 1)
#define PLDM_MAX_NIC_TEMP_SENSOR_HANDLE               (PLDM_BASE_NIC_TEMP_SENSOR_HANDLE + PLDM_NIC_TEMP_SENSOR_NUM - 1)
#define PLDM_MAX_NC_HANDLE                            (PLDM_BASE_NC_HANDLE + PLDM_NC_NUM - 1)
#define PLDM_MAX_NC_STATE_SENSOR_HANDLE               (PLDM_BASE_NC_STATE_SENSOR_HANDLE + PLDM_NC_STATE_SENSOR_NUM - 1)
#define PLDM_MAX_PORT_OF_NC_HANDLE                    (PLDM_BASE_PORT_OF_NC_HANDLE + PLDM_PORT_OF_NC_NUM - 1)
#define PLDM_MAX_LINK_SPEED_SENSOR_HANDLE             (PLDM_BASE_LINK_SPEED_SENSOR_HANDLE + PLDM_LINK_SPEED_SENSOR_NUM - 1)
#define PLDM_MAX_LINK_STATE_SENSOR_HANDLE             (PLDM_BASE_LINK_STATE_SENSOR_HANDLE + PLDM_LINK_STATE_SENSOR_NUM - 1)
#define PLDM_MAX_NC_TEMP_SENSOR_HANDLE                (PLDM_BASE_NC_TEMP_SENSOR_HANDLE + PLDM_NC_TEMP_SENSOR_NUM - 1)

#define PLDM_MAX_PLUG_HANDLE                          (PLDM_BASE_PLUG_HANDLE + PLDM_PLUG_NUM - 1)
#define PLDM_MAX_PLUG_POWER_SENSOR_HANDLE             (PLDM_BASE_PLUG_POWER_SENSOR_HANDLE + PLDM_PLUG_POWER_SENSOR_NUM - 1)
#define PLDM_MAX_PLUG_TEMP_SENSOR_HANDLE              (PLDM_BASE_PLUG_TEMP_SENSOR_HANDLE + PLDM_PLUG_TEMP_SENSOR_NUM - 1)
#define PLDM_MAX_PLUG_COMPOSITE_SENSOR_HANDLE         (PLDM_BASE_PLUG_COMPOSITE_SENSOR_HANDLE + PLDM_PLUG_COMPOSITE_SENSOR_NUM - 1)
#define PLDM_MAX_COMM_CHANNEL_HANDLE                  (PLDM_BASE_COMM_CHANNEL_HANDLE + PLDM_COMM_CHANNEL_NUM - 1)

/* redfish pdr handle */
#define PLDM_NETWORK_ADAPTER_PDR_HANDLE               (4001)
#define PLDM_NETWORK_INTERFACE_PDR_HANDLE             (4005)
#define PLDM_PORTS_PDR_HANDLE                         (4010)
#define PLDM_NETWORK_DEV_FUNCS_PDR_HANDLE             (4020)
#define PLDM_PORT_PDR_HANDLE                          (4100)
#define PLDM_NETWORK_DEV_FUNC_PDR_HANDLE              (4200)
#define PLDM_PCIE_FUNCS_PDR_HANDLE                    (4030)
#define PLDM_PCIE_FUNC_PDR_HANDLE                     (4300)
#define PLDM_ETH_INTERFACE_COLLECTION_PDR_HANDLE      (4040)
#define PLDM_ETH_INTERFACE_PDR_HANDLE                 (4400)
#define PLDM_RESET_SET2DEFAULE_PDR_HANDLE             (3001)
#define PLDM_PORT_RESET_PDR_HANDLE                    (3002)

/* resource id */
#define PLDM_BASE_NETWORK_ADAPTER_RESOURCE_ID         (1)
#define PLDM_BASE_NETWORK_INTERFACE_RESOURCE_ID       (5)
#define PLDM_BASE_PORTS_RESOURCE_ID                   (10)
#define PLDM_BASE_NETWORK_DEV_FUNCS_RESOURCE_ID       (20)
#define PLDM_BASE_PORT_RESOURCE_ID                    (100)
#define PLDM_BASE_PORT_RESOURCE_ID_1                  (110)
#define PLDM_BASE_NETWORK_DEV_FUNC_RESOURCE_ID        (200)
#define PLDM_BASE_NETWORK_DEV_FUNC_RESOURCE_ID_1      (210)
#define PLDM_BASE_PCIE_DEV_RESOURCE_ID                (3)
#define PLDM_BASE_PCIE_FUNCS_RESOURCE_ID              (30)
#define PLDM_BASE_PCIE_FUNC_RESOURCE_ID               (300)
#define PLDM_BASE_ETH_INTERFACE_COLLECTION_RESOURCE_ID (40)
#define PLDM_BASE_ETH_INTERFACE_RESOURCE_ID           (400)
#define PLDM_BASE_ETH_INTERFACE_RESOURCE_ID_1         (410)
#define PLDM_BASE_RESET_SET2DEFAULE_RESOURCE_ID       (2001)
#define PLDM_BASE_PORT_RESET_RESOURCE_ID              (2002)  /* ? E810 */

/* comm schema */
#define PLDM_BASE_ANNOTATION_DICT_RESOURCE_ID         (0xFFFFFFFF)
#define PLDM_BASE_EVENT_DICT_RESOURCE_ID              (0xFFFFFFFF)
#define PLDM_BASE_REGISTER_DICT_RESOURCE_ID           (0xFFFFFFFF)


#define PLDM_MAX_NETWORK_ADAPTER_RESOURCE_ID          (PLDM_BASE_NETWORK_ADAPTER_RESOURCE_ID)
#define PLDM_MAX_NETWORK_INTERFACE_RESOURCE_ID        (PLDM_BASE_NETWORK_INTERFACE_RESOURCE_ID)
#define PLDM_MAX_PORTS_RESOURCE_ID                    (PLDM_BASE_PORTS_RESOURCE_ID)
#define PLDM_MAX_NETWORK_DEV_FUNCS_RESOURCE_ID        (PLDM_BASE_NETWORK_DEV_FUNCS_RESOURCE_ID)
#define PLDM_MAX_PORT_RESOURCE_ID                     (PLDM_BASE_PORT_RESOURCE_ID + MAX_LAN_NUM - 1)
#define PLDM_MAX_PORT_RESOURCE_ID_1                   (PLDM_BASE_PORT_RESOURCE_ID_1 + MAX_LAN_NUM - 1)
#define PLDM_MAX_NETWORK_DEV_FUNC_RESOURCE_ID         (PLDM_BASE_NETWORK_DEV_FUNC_RESOURCE_ID + MAX_LAN_NUM - 1)
#define PLDM_MAX_NETWORK_DEV_FUNC_RESOURCE_ID_1       (PLDM_BASE_NETWORK_DEV_FUNC_RESOURCE_ID_1 + MAX_LAN_NUM - 1)
#define PLDM_MAX_PCIE_FUNCS_RESOURCE_ID               (PLDM_BASE_PCIE_FUNCS_RESOURCE_ID)
#define PLDM_MAX_PCIE_FUNC_RESOURCE_ID                (PLDM_BASE_PCIE_FUNC_RESOURCE_ID + MAX_LAN_NUM - 1)
#define PLDM_MAX_ETH_INTERFACE_COLLECTION_RESOURCE_ID (PLDM_BASE_ETH_INTERFACE_COLLECTION_RESOURCE_ID)
#define PLDM_MAX_ETH_INTERFACE_RESOURCE_ID            (PLDM_BASE_ETH_INTERFACE_RESOURCE_ID + MAX_LAN_NUM - 1)
#define PLDM_MAX_ETH_INTERFACE_RESOURCE_ID_1          (PLDM_BASE_ETH_INTERFACE_RESOURCE_ID_1 + MAX_LAN_NUM - 1)
#define PLDM_MAX_RESET_SET2DEFAULE_RESOURCE_ID        (PLDM_BASE_RESET_SET2DEFAULE_RESOURCE_ID)
#define PLDM_MAX_PORT_RESET_RESOURCE_ID               (PLDM_BASE_PORT_RESET_RESOURCE_ID)  /* "?" E810 */

/* Container id */

#define PLDM_SYSTEM                                   (0)

#define PLDM_BASE_NIC_CONTAINER_ID                    (100)
#define PLDM_BASE_CONNECTOR_CONTAINER_ID              (1040)
#define PLDM_BASE_NC_CONTAINER_ID                     (1000)
#define PLDM_BASE_PLUG_CONTAINER_ID                   (1010)
#define PLDM_BASE_COMM_CHAN_CONTAINER_ID              (1060)

#define PLDM_MAX_NIC_CONTAINER_ID                     (PLDM_BASE_NIC_CONTAINER_ID)
#define PLDM_MAX_CONNECTOR_CONTAINER_ID               (PLDM_BASE_CONNECTOR_CONTAINER_ID + PLDM_CONNECTOR_NUM - 1)
#define PLDM_MAX_NC_CONTAINER_ID                      (PLDM_BASE_NC_CONTAINER_ID)
#define PLDM_MAX_PLUG_CONTAINER_ID                    (PLDM_BASE_PLUG_CONTAINER_ID + PLDM_PLUG_NUM - 1)
#define PLDM_MAX_COMM_CHAN_CONTAINER_ID               (PLDM_BASE_COMM_CHAN_CONTAINER_ID + PLDM_COMM_CHANNEL_NUM - 1)

typedef enum {
	PLDM_NO_EVENT_GENERATION,
	PLDM_EVENTS_DISABLED,
	PLDM_EVENTS_ENABLED,
	PLDM_OP_EVENTS_ONLY_ENABLED,
	PLDM_STATE_EVENTS_ONLY_ENABLED
} pldm_monitor_sensor_event_msg_en_t;

typedef enum {
    PLDM_DISABLE = 0,
    PLDM_ENABLE_ASYNC,
    PLDM_ENABLE_POLLING
} pldm_monitor_event_msg_global_en_flag_t;

typedef enum {
    PLDM_ERROR_INVALID_DATA = 0x02,

    PLDM_INVALID_SENSOR_ID = 0X80,
    PLDM_INVALID_PROTOCOL_TYPE = 0x80,
    PLDM_INVALID_DATA_TRANSFER_HANDLE = 0x80,

    PLDM_INVALID_SENSOR_OPERATIONAL_STATE = 0X81,
    PLDM_INVALID_TRANSFER_OPERATION_FLAG = 0x81,
    PLDM_UNSUPPORTED_EVENT_FORMAT_VERSION = 0x81,
    PLDM_ENABLE_METHOD_NOT_SUPPORTED = 0x81,
    PLDM_EARM_UNAVAILABLE_IN_PRESENT_STATE = 0x81,

    PLDM_EVENT_GENERATION_NOT_SUPPORTED = 0x82,
    PLDM_EVENT_ID_NOT_VALID = 0x82,
    PLDM_INVALID_RECORD_HANDLE = 0x82,

    PLDM_INVALID_RECORD_CHANGE_NUMBER = 0x83,
    PLDM_TRANSFER_TIMEOUT = 0x84,
    PLDM_REPOSITORY_UPDATE_IN_PROGRESS = 0x85,
} pldm_monitor_cpl_code_t;

typedef enum {
    SENSOR_EVENT = 0x00,
    REDFISH_TASK_EXCUTE_EVENT = 0x02,
    REDFISH_MSG_EVENT = 0x03,
    PLDM_PDR_REPO_CHG_EVENT = 0x04,
    PLDM_MSG_POLL_EVENT = 0x05
} pldm_monitor_event_class_t;

typedef enum {
    PLDM_DATASIZE_UINT8 = 0x00,
    PLDM_DATASIZE_SINT8,
    PLDM_DATASIZE_UINT16,
    PLDM_DATASIZE_SINT16,
    PLDM_DATASIZE_UINT32,
    PLDM_DATASIZE_SINT32
} pldm_monitor_sensor_datasize_t;

typedef enum {
    PLDM_OP_ENABLE = 0x00,
    PLDM_OP_DESABLE,
    PLDM_OP_UNAVAILABLE,
    PLDM_OP_STATUS_UNKNOW,
    PLDM_OP_FAILED,
    PLDM_OP_INITIALIZING,
    PLDM_OP_SHUTTING_DOWN,
    PLDM_OP_IN_TEST
} pldm_monitor_sensor_op_state_t;

typedef enum {
    PLDM_EVENT_NO_CHG = 0x00,
    PLDM_EVENT_DIS_EN = 0x01,
    PLDM_EVENT_EN = 0x02,
    PLDM_EVENT_OP_EN_ONLY = 0x03,
    PLDM_EVENT_STATE_EN_ONLY = 0x04
} pldm_monitor_evevt_msg_en_t;

typedef enum {
    PLDM_REPO_AVAILABLE = 0x00,
    PLDM_REPO_UPDATE_IN_PROGRESS,
    PLDM_REPO_FAILD
} pldm_repo_state_t;

typedef enum {
    PLDM_REPO_NO_TIMEOUT = 0x00,
    PLDM_REPO_DEFAULT_MINIMUM_TIMEOUT = 0x01,
    PLDM_REPO_TIMEOUT_EXCEED_255 = 0xFF
} pldm_data_transfer_handle_timeout_t;

typedef enum {
    PLDM_TRANSFER_FLAG_START = 0x00,
    PLDM_TRANSFER_FLAG_MIDDLE = 0x01,
    PLDM_TRANSFER_FLAG_END = 0x04,
    PLDM_TRANSFER_FLAG_START_AND_END = 0x05,
} pldm_get_pdr_transfer_flag_t;

typedef enum {
    PLDM_TRANSFER_OP_FLAG_GET_NEXT_PART = 0x00,
    PLDM_TRANSFER_OP_FLAG_GET_FIRST_PART,
    PLDM_TRANSFER_OP_FLAG_ACK_ONLY
} pldm_transfer_op_flag_t;

typedef enum {
    EVENT_ID_FOR_START = 0x0000,            /* TransferOperationFlag is GetFirstPart */
    EVENT_ID_FOR_MIDDLE = 0xFFFF            /* TransferOperationFlag is GetNextPart */
} pldm_event_id_to_ack_t;

typedef enum {
    PLDM_REPO_CHG_REFRESH_ENTIRE_REPO = 0x00,
    PLDM_REPO_CHG_FORMAT_IS_PDR_TYPE,
    PLDM_REPO_CHG_FORMAT_ID_PDR_HANDLE
} pldm_repo_chg_event_data_format_t;

typedef enum {
    PLDM_REPO_CHG_REFRESH_ALL_RECORDS = 0x00,
    PLDM_REPO_CHG_RECORDS_DELETED,
    PLDM_REPO_CHG_RECORDS_ADDED,
    PLDM_REPO_CHG_RECORDS_MODIFIED
} pldm_repo_chg_event_data_op_t;

#pragma pack(1)

typedef struct {
    u8 event_msg_global_en;
    u8 trans_protocol_type;
    u8 event_receiver_addr_info;
} pldm_set_event_receiver_req_dat_t;

typedef struct {
    u8 trans_protocol_type;
    u8 event_receiver_addr_info;
} pldm_get_event_receiver_rsp_dat_t;

typedef struct {
    u8 format_ver;
} pldm_event_msg_supported_req_dat_t;

typedef struct {
    u8 sync_config;
    u8 sync_config_supported;
    u8 num_event_class;
    u8 event_class[0];
} pldm_event_msg_supported_rsp_dat_t;

typedef struct {
    u16 event_receiver_max_buffersize;
} pldm_event_msg_buffersize_req_dat_t;

typedef struct {
    u16 terminus_max_buffersize;
} pldm_event_msg_buffersize_rsp_dat_t;

typedef struct {
    u16 sensor_id;
    u8  sensor_op_state;
    u8  sensor_event_msg_en;
} pldm_set_numeric_sensor_enable_req_dat_t;

typedef struct {
    u16 sensor_id;
    u8  rearm_event_state;         /* Ignored. All sensors are auto-rearm */
} pldm_get_sensor_reading_req_dat_t;

typedef struct {
    u8 sensor_datasize;
    u8 sensor_op_state;
    u8 sensor_event_msg_en;
    u8 present_state;
    u8 previous_state;
    u8 event_state;
    u8 present_reading[0];
} pldm_get_sensor_reading_rsp_dat_t;

typedef struct {
    u16 sensor_id;
} pldm_get_sensor_thresholds_req_dat_t, pldm_get_sensor_hysteresis_req_dat_t;

typedef struct {
    u8 sensor_datasize;
    u8 upper_threshold_warning;
    u8 upper_threshold_critical;
    u8 upper_threshold_fatal;
    u8 lower_threshold_warning;         /* value : 0 */
    u8 lower_threshold_critical;        /* value : 0 */
    u8 lower_threshold_fatal;           /* value : 0 */
} pldm_get_sensor_thresholds_rsp_dat_t;

typedef struct {
    u16 sensor_id;
    u8  sensor_datasize;
    u8  upper_threshold_warning;
    u8  upper_threshold_critical;
    u8  upper_threshold_fatal;
    u8  lower_threshold_warning;         /* Ignored */
    u8  lower_threshold_critical;        /* Ignored */
    u8  lower_threshold_fatal;           /* Ignored */
} pldm_set_sensor_thresholds_req_dat_t;

typedef struct {
    u8 sensor_datasize;
    u8 hysteresis_value;
} pldm_get_sensor_hysteresis_rsp_dat_t;

typedef struct {
    u8 sensor_op_state;
    u8 event_msg_en;
} pldm_set_state_sensor_en_composite_sensor_cnt_opfield_t;

typedef struct {
    u16 sensor_id;
    u8  composite_sensor_cnt;
    pldm_set_state_sensor_en_composite_sensor_cnt_opfield_t opfield[0];
} pldm_set_state_sensor_en_req_dat_t;

typedef struct {
    u16 sensor_id;
    u8  sensor_rearm;
    u8  rsvd;
} pldm_get_state_sensor_reading_req_dat_t;

typedef struct {
    u8 sensor_op_state;
    u8 present_state;
    u8 previous_state;
    u8 event_state;
} pldm_get_state_sensor_reading_composite_sensor_cnt_opfield_t;

typedef struct {
    u8 composite_sensor_cnt;
    pldm_get_state_sensor_reading_composite_sensor_cnt_opfield_t opfield[0];
} pldm_get_state_sensor_reading_rsp_dat_t;

typedef struct {
    u8 repo_state;
    pldm_timestamp104_t update_time;
    u32 oem_update_time;                     /* Currently no OEM PDRs are defined, so return 0, timestamp104 ? */
    u32 record_count;
    u32 repo_size;
    u32 largest_record_size;
    u8 data_transfer_handle_timeout;
} pldm_get_pdr_repo_info_rsp_dat_t;

typedef struct {
    u32 record_handle;
    u32 data_transfer_handle;
    u8 transfer_op_state;
    u16 req_cnt;
    u16 record_chg_num;
} pldm_get_pdr_req_dat_t;

typedef struct {
    u32 next_record_handle;
    u32 next_data_transfer_handle;
    u8 transfer_flag;
    u16 rsp_cnt;
    u8 req_data[0];
    // u8 transfer_crc;                        /* If transferFlag = End */
} pldm_get_pdr_rsp_dat_t;

typedef struct {
    u32 pdr_repo_signature;
} pldm_get_pdr_repo_signature_rsp_dat_t;

typedef struct {
    u8 cpl_code;
    u8 status;                              /* can be ignored */
} pldm_platform_event_msg_receive_t;

typedef struct {
    u8 format_ver;
    u8 transfer_op_flag;
    u32 data_transfer_handle;
    u16 event_id_to_ack;
} pldm_poll_for_platform_event_msg_req_dat_t;

typedef struct {
    u8 tid;
    u16 event_id;
    u32 next_data_transfer_handle;
    u8 transfer_flag;
    u8 event_class;
    u32 event_datasize;                     /* Size in byes of eventData */
    u8 event_data[0];
    // u32 event_data_integrity_checksum;
} pldm_poll_for_platform_event_msg_rsp_dat_t;

typedef struct {
    u8 format_ver;
    u8 tid;
    u8 event_class;
    u8 event_data[0];
} pldm_platform_event_msg_req_dat_t;

typedef struct {
    u8 tid;
    u8 hw_id;
    phy_addr_t phy_addr;        /* vdm target id */
    u8 event_receiver_eid;
    u8 terminus_mode;
    u8 repo_state;
    u16 terminus_max_buffersize;
    pldm_pdr_t pldm_repo;
    void *pldm_event_rbuf;
} pldm_monitor_base_info_t;

#pragma pack()

void pldm_monitor_init(void);
void pldm_monitor_process(protocol_msg_t *pkt, int *pkt_len, u32 cmd_code);
void pldm_monitor_update_repo_signature(pldm_pdr_t *repo);

int is_temp_sensor(u16 sensor_id);
u32 sensor_id_convert_to_record_handle(u16 sensor_id);

void pldm_event_send_handle(void);
void pldm_temp_monitor_handle(void);

#endif /* __PLDM_MONITOR_H__ */