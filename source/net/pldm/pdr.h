#ifndef __PDR_H__
#define __PDR_H__

#include "main.h"

#define PDR_MIN_SIZE                    (1)
#define PDR_POOL_SIZE                   (MAX_LAN_NUM == 2 ? 3200 : 4608)
#define NOT_FIELD                       (0xFF)

typedef enum {
    TERMINUS_LOCATOR_PDR = 1,
    NUMERIC_SENSOR_PDR = 2,
    STATE_SENSOR_PDR = 4,
    ENTITY_ASSOC_PDR = 15,
    FRU_RECORD_SET_PDR = 20,
    REDFISH_RESOURCE_PDR = 22,
    REDFISH_ACTION_PDR = 24
} pldm_pdr_type_t;

typedef enum {
    DEGREES_C = 0x02,                   /* for Thermal sensor PDR */
    WATTS = 0x07,                       /* for Pluggable Module Power sensor PDR */
    BITS = 0x3c                         /* for Link Speed sensor PDR */
} pldm_base_unit_t;

typedef enum {
    UPPER_THRESHOLD_WARNING = 0,
    UPPER_THRESHOLD_CRITICAL,
    UPPER_THRESHOLD_FATAL,
    LOWER_THRESHOLD_WARNING,
    LOWER_THRESHOLD_CRITICAL,
    LOWER_THRESHOLD_FAYAL,
} pldm_sup_threshold_t;

typedef enum {
    INITIALIZATION_AGENT_CONTROLLER = 0,
    PLDM_SYS_POWER_UP,
    SYS_HARD_RESET,
    SYS_WARM_RESET,
    PLDM_TERMINUS_RET_TO_ONLINE_COND
} pldm_threshold_and_hysteresis_volatility_t;

typedef enum {
    NORMAL_VAL = 0,
    NORMAL_MAX,
    NORMAL_MIN,
    CRITICAL_HIGH,
    CRITICAL_LOW,
    FATAL_HIGH,
    FATAL_LOW
} pldm_range_field_support_t;

typedef enum {
    MAX_READABLE_100 = 100,
    MAX_READABLE_1G = 1000,
    MAX_READABLE_10G = 10000,
    MAX_READABLE_25G = 25000,
    MAX_READABLE_50G = 50000,
    MAX_READABLE_100G = 100000
} pldm_max_readable_t;

typedef enum {
    CONTROLLER_SENSOR = 144,    /* 0x90 (Physical | Network Controller (144)) */
    NIC_SENSOR = 68,            /* 0x44 (Physical | Add on card (68)) */
    CONNECTOR = 185,
    COMMUNICATION_PORT = 6,
    PORT = 189,
    CABLE = 187,
    ETH_PORT = 300,
    /* For Plug Sensor: */
    SFP28 = 206,
    SFP = 207,
    QSFP28 = 211,
    QSFP = 212
} pldm_sensor_type_t;

typedef enum {
    PLDM_ENTITY_ASSOCIAION_PHYSICAL = 0x00,
    PLDM_ENTITY_ASSOCIAION_LOGICAL = 0x01
} pldm_assoc_type_t;

typedef enum {
    TEMP_UPPER_WARNING = 0x8,
    TEMP_UPPER_CRITICAL = 0x9,
    TEMP_UPPER_FATAL = 0xA,
} pldm_temp_states_t;

typedef enum {
    HEALTH_STATE = 1,
    PRESENCE = 13,
    CONFIGURATION = 15,
    CONFIGURATION_CHG = 16,
    FW_VERSION = 18,
    THERMAL_TRIP = 21,
    LINK_STATE = 33
} pldm_composite_state_sensor_type_t;

typedef enum {
    UNKNOWN = 0,
    NORMAL = 1,
    CRITICAL = 3,
    FATAL = 4,
    UPPER_NON_CRITICAL = 5,
    UPPER_CRITICAL = 7
} pldm_health_states_t;

typedef enum {
    IS_PRESENCE = 1,
    NOT_PRESENCE = 2
} pldm_presence_states_t;

typedef enum {
    VALID_CONFIG = 1,
    INVALID_CONFIG = 2
} pldm_config_states_t;

typedef enum {
    CONFIG_CHG = 2
} pldm_config_chg_states_t;

typedef enum {
    THERMAL_TRIPED = 2
} pldm_thermal_trip_states_t;

typedef enum {
    CONNECTED = 1,
    DISCONNECTED = 2
} pldm_link_states_t;

typedef enum {
    VER_CHG_DETECTED = 2
} pldm_fw_version_t;

typedef enum {
    PLDM_SENSOR_OP_STATE = 0x0,
    PLDM_STATE_SENSOR_STATE,
    PLDM_NUMERIC_SENSOR_STATE
} pldm_sensor_event_class_t;

typedef enum {
    NIC_TEMP_SENSOR = 0,
    NC_TEMP_SENSOR,
    PLUG_TEMP_SENSOR
} pldm_temp_sensor_type_t;

typedef enum {
    NO_INIT = 0,
    USE_INIT_PDR,
    EN_SENSOR,
    DISEN_SENSOR,
} pldm_sensor_init_t;

typedef enum {
    UTC_RSLV_UTCUNSPECIFIED = 0,
    UTC_RSLV_MINUTE,
    UTC_RSLV_MINUTE_10,
    UTC_RSLV_HOUR
} pldm_time_resolution_t;

typedef enum {
    T_RSLV_MICROSECOND = 0,
    T_RSLV_MICROSECOND_10,
    T_RSLV_MICROSECOND_100,
    T_RSLV_MILLISECOND,
    T_RSLV_MILLISECOND_10,
    T_RSLV_MILLISECOND_100,
    T_RSLV_SECOND,
    T_RSLV_SECOND_10,
    T_RSLV_MINUTE,
    T_RSLV_MINUTE_10,
    T_RSLV_HOUR,
    T_RSLV_DAY,
    T_RSLV_MONTH,
    T_RSLV_YEAR
} pldm_utc_resolution_t;

#pragma pack(1)
/* pdr list struct */
typedef struct pldm_pdr_record {
	u32 record_handle;
	u32 size;
	u8 *data;
	struct pldm_pdr_record *next;
} pldm_pdr_record_t;

typedef struct {
    s16 utc_offset;
    u8 microsecond[3];
    u8 sec;
    u8 min;
    u8 hour;
    u8 day;
    u8 mon;
    u16 year;
    u8 time_resolution      : 4;
    u8 utc_resolution       : 4;
} pldm_timestamp104_t;

typedef struct {
	u32 record_count;
	u32 size;
    u32 largest_pdr_size;
    pldm_timestamp104_t update_time;
    u32 repo_signature;
	pldm_pdr_record_t *head;
    pldm_pdr_record_t *is_deleted_head;              /* all deleted pdr is in here */
} pldm_pdr_t;

typedef struct {
    u8 num;
} pdr_data_ext_t;

typedef struct {
	u32 record_handle;
	u8 version;
	u8 type;
	u16 record_change_num;
	u16 length;
}pldm_pdr_hdr_t;

typedef struct {
    pldm_pdr_hdr_t hdr;
    u16 terminus_handle;
    u8 validity;
    u8 tid;
    u16 container_id;
    u8 terminus_locator_type;
	u8 terminus_locator_value_size;
    u8 eid;
} pldm_terminus_locator_pdr_t;

typedef struct {
	u16 entity_type;
	u16 entity_instance_num;
	u16 entity_container_id;
} pldm_entity_t;

typedef struct {
    pldm_pdr_hdr_t hdr;
    u16 pldm_terminus_handle;
    u8 fru_record_terminus_identifier;
    u16 entity_type;
    u16 entity_instance_num;
    u16 container_id;
} pldm_fru_record_set_pdr_t;

typedef union {
    struct {
        u32 mantissa      : 23;         /* [22:0] – mantissa as a binary integer (23 bits) */
        u32 exponent      : 8;          /* [30:23] – exponent as a binary integer (8 bits) */
        u32 sign          : 1;          /* [31] – S (sign) bit (1 = negative, 0 = positive) */
    };
    u32 val;
} real32_t;

typedef struct {
    u8 hysteresis;
    u8 supported_thresholds;
    u8 threshold_and_hysteresis_volatility;
    real32_t state_transition_interval;
    real32_t update_interval;
    u8 max_readable;
    u8 min_readable;
    u8 range_field_format;
    u8 range_field_support;
    u16 nominal_value;
	u16 normal_max;
	u16 normal_min;
	u16 warning_high;
	u16 warning_low;
	u16 critical_high;
	u16 critical_low;
	u16 fatal_high;
	u16 fatal_low;
} pldm_thermal_sensor_pdr_t;

typedef struct {
    u32 hysteresis;
    u8 supported_thresholds;
    u8 threshold_and_hysteresis_volatility;
    real32_t state_transition_interval;
    real32_t update_interval;
    u32 max_readable;
    u32 min_readable;
    u8 range_field_format;
    u8 range_field_support;
    u8 nominal_value;
	u8 normal_max;
	u8 normal_min;
	u8 warning_high;
	u8 warning_low;
	u8 critical_high;
	u8 critical_low;
	u8 fatal_high;
	u8 fatal_low;
} pldm_pluggable_module_power_sensor_pdr_t;

typedef struct {
    pldm_pdr_hdr_t hdr;
    u16 terminus_handle;
    u16 sensor_id;
    pldm_entity_t container;
    u8 sensor_init;
    u8 sensor_auxiliary_names_pdr;  /* bool */
    u8 base_unit;
    s8 unit_modifier;
    u8 rate_unit;
    u8 base_oem_unit_handle;
    u8 aux_unit;
    s8 aux_unit_modifier;
    u8 aux_rate_unit;
    u8 rel;
    u8 aux_oem_unit_handle;
    u8 is_linear;                   /* bool */
    u8 sensor_data_size;
    real32_t resolution;
    real32_t offset;
    u16 accuracy;
    u8 plus_tolerace;
    u8 minus_tolerance;
} pldm_numeric_sensor_pdr_comm_part_t;

typedef struct {
    pldm_numeric_sensor_pdr_comm_part_t numeric_pdr_comm_part;
    union {
        pldm_thermal_sensor_pdr_t thermal_pdr;
        pldm_pluggable_module_power_sensor_pdr_t pluggable_power_pdr;
    };
} pldm_numeric_sensor_pdr_t;                        /* only thermal_sensor or pluggable_module_power_sensor pdr */

typedef struct {
    pldm_numeric_sensor_pdr_comm_part_t numeric_pdr_comm_part;
    u32 hysteresis;
    u8 supported_thresholds;
    u8 threshold_and_hysteresis_volatility;
    real32_t state_transition_interval;
    real32_t update_interval;
    u32 max_readable;
    u32 min_readable;
    u8 range_field_format;
    u8 range_field_support;
    u32 nominal_value;
	u32 normal_max;
	u32 normal_min;
} pldm_link_speed_sensor_pdr_t;

typedef struct {
    pldm_pdr_hdr_t hdr;
    u32 resource_id;
    u8 resource_flg;
    u32 containing_resource_id;
    u16 proposed_containing_resource_byte_len;
    char proposed_containing_resource_name[0];
} pldm_redfish_resource_pdr_first_part_t;

typedef struct {
    u16 suburi_byte_len;
    char suburi[0];
} pldm_redfish_resource_pdr_middle0_part_t;

typedef struct {
    u32 add_resource_id;
    u16 add_suburi_byte_len;
    char add_suburi[0];
} pldm_redfish_add_resource_info_t;

typedef struct {
    u32 add_resource_id;
    u16 add_suburi_byte_len;
} pldm_redfish_add_info_t;

typedef struct {
    u16 add_resource_id_cnt;
    u8 add_resource_info[0];
} pldm_redfish_resource_pdr_middle1_part_t;

typedef struct {
    u32 major_schema_ver;
    u16 major_schema_dict_byte_len;
    u32 major_schema_dict_sign;
    u8 major_schema_name_len;
    char major_schema_name[0];
} pldm_redfish_resource_pdr_middle2_part_t;

typedef struct {
    u16 oem_cnt;                /* always 0 */
    // u16 oem_name_byte_len;
    // char oem_name;
} pldm_redfish_resource_pdr_end_part_t;

typedef struct {
    pldm_pdr_hdr_t hdr;
    u8 action_pdr_idx;
    u16 related_resource_cnt;
    u32 related_resource_id[0];
} pldm_redfish_action_pdr_first_part_t;

// char action_name_byte_len;
// char action_name[0];
// u8 action_path_byte_len;
// char action_path;
typedef struct {
    u8 action_byte_len;
    char action_str[0];
} pldm_redfish_action_info_t;

typedef struct {
    u8 action_cnt;
    u8 action_info[0];
} pldm_redfish_action_pdr_end_part_t;

typedef struct {
    pldm_pdr_hdr_t hdr;
	u16 container_id;
	u8 assoc_type;
	pldm_entity_t container;
    u8 contained_entity_cnt;
	pldm_entity_t contained[0];
} pldm_pdr_entity_assoc_t;

typedef struct {
    u16 state_setid;
    u8 possible_states_size;
    u8 possible_states;
} pldm_composite_sensor_attr_t;

typedef struct {
    pldm_pdr_hdr_t hdr;
    u16 terminus_handle;
    u16 sensor_id;
    pldm_entity_t container;
    u8 sensor_init;
    u8 sensor_auxi_names_pdr;           /* bool */
    u8 composite_sensor_cnt;
    pldm_composite_sensor_attr_t sensors[0];
} pldm_composite_state_sensor_pdr_t;

/* data structure list struct */

typedef struct {
    u8 op_state;
    u8 previous_op_state;
    u8 event_msg_en;
    u16 entity_type;
    u16 sensor_id;
    u8 sensor_event_class;
    u8 present_state;
    u8 previous_state;
    u8 sensor_data_size;
    u16 present_reading;
} pldm_temp_sensor_data_struct_t, pldm_link_speed_data_struct_t, pldm_plug_power_data_struct_t, pldm_data_struct_t;

typedef struct {
    u8 op_state;
    u8 previous_op_state;
    u8 event_msg_en;
    u8 sensor_type;
    u16 sensor_id;
    u8 sensor_event_class;
    u8 sensor_offset;
    u8 present_state;
    u8 previous_state;
} pldm_nic_composite_state_sensor_data_struct_t, pldm_controller_composite_state_sensor_data_struct_t, \
pldm_plug_composite_state_sensor_data_struct_t, pldm_simple_sensor_data_struct_t, pldm_state_data_struct_t;

typedef struct {
    u32 *pdr_addr;
    u16 sensor_id;
} pldm_temp_sensor_monitor_t;

typedef struct {
    u16 warning_high;
	u16 critical_high;
	u16 fatal_high;
} pldm_temp_sensor_threshold_data_t;

typedef struct {
    u32 max_readable;
    u32 min_readable;
} pldm_speed_sensor_cap_t;

typedef struct {
    u16 total_size;
    u16 cnt;
} pldm_pdr_data_hdr_t, pldm_redfish_schema_data_hdr_t;

#pragma pack()

typedef void (*pldm_monitor_handle)(u8 port);
typedef int (*fill_common_sensor_pdr)(void *buf, u16 sensor_id, u16 entity_type, u8 data_size);

void pdrs_pool_init(u32 *addr);
void *pdr_malloc(int size);
u32 pldm_pdr_get_used(void);

void pldm_pdr_init(pldm_pdr_t *repo);
pldm_pdr_record_t *pldm_find_insert(pldm_pdr_t *repo, u32 record_handle);
int pldm_pdr_add(pldm_pdr_t *repo, u8 *pdr_data, u32 pdr_size, u32 record_handle);
int pldm_pdr_delete(pldm_pdr_t *repo, u32 record_handle);
pldm_pdr_record_t *pldm_pdr_find(pldm_pdr_t *repo, u32 record_handle);

void pldm_fru_pdr_init(void);
void pldm_redfish_pdr_init(void);
void pldm_terminus_locator_pdr_init(void);
void pldm_assoc_pdr_init(void);
void pldm_state_sensor_pdr_init(void);
void pldm_numeric_sensor_pdr_init(void);

void pldm_link_handle(u8 port, u8 link_state);

void terminus_locator_pdr_chg(void);
void pldm_modify_state_datastruct(u8 present_state, pldm_state_data_struct_t *datastruct);

#endif /* __PDR_H__ */