#include "pdr.h"
#include "mctp.h"
#include "mctp_ctl.h"
#include "pldm_monitor_event_rbuf.h"
#include "pldm_redfish.h"
#include "pldm_monitor.h"
#include "pldm_fru_data.h"
#include "utlist.h"

static u32 *pdrs_pool;
static u32 pdrs_pool_wt;
static char *proposed_containing_resource_name;
static char *sub_uri;
static char *major_schema_name;

void pldm_get_link_spd_cap(pldm_speed_sensor_cap_t *speed_data, u8 port);
void pldm_get_temp_sensor_threshold(pldm_temp_sensor_threshold_data_t *temp_data, u8 sensor_type, u8 port);

pldm_temp_sensor_data_struct_t temp_sensors[3][4] = {
    [NIC_TEMP_SENSOR] = {
        /* op_state | previous_op_state | event_msg_en | entity type | sensor id | sensor event class | present val | previous val | data size | present reading */
        [0] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_EN, NIC_SENSOR, PLDM_BASE_NIC_TEMP_SENSOR_ID + 0, PLDM_NUMERIC_SENSOR_STATE, NORMAL, NORMAL, PLDM_DATASIZE_UINT8, 0}
    },
    [NC_TEMP_SENSOR] = {
        [0] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_EN, CONTROLLER_SENSOR, PLDM_BASE_NC_TEMP_SENSOR_ID + 0, PLDM_NUMERIC_SENSOR_STATE, NORMAL, NORMAL, PLDM_DATASIZE_UINT8, 0}
    },
    [PLUG_TEMP_SENSOR] = {
        [0] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_EN, UNKNOWN, PLDM_BASE_PLUG_TEMP_SENSOR_ID + 0, PLDM_NUMERIC_SENSOR_STATE, UNKNOWN, UNKNOWN, PLDM_DATASIZE_UINT8, 0},
        [1] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_EN, UNKNOWN, PLDM_BASE_PLUG_TEMP_SENSOR_ID + 1, PLDM_NUMERIC_SENSOR_STATE, UNKNOWN, UNKNOWN, PLDM_DATASIZE_UINT8, 0},
        [2] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_EN, UNKNOWN, PLDM_BASE_PLUG_TEMP_SENSOR_ID + 2, PLDM_NUMERIC_SENSOR_STATE, UNKNOWN, UNKNOWN, PLDM_DATASIZE_UINT8, 0},
        [3] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_EN, UNKNOWN, PLDM_BASE_PLUG_TEMP_SENSOR_ID + 3, PLDM_NUMERIC_SENSOR_STATE, UNKNOWN, UNKNOWN, PLDM_DATASIZE_UINT8, 0}
    }
};

pldm_link_speed_data_struct_t link_speed_sensors[PLDM_LINK_SPEED_SENSOR_NUM] = {
    [0] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_EN, COMMUNICATION_PORT, PLDM_BASE_LINK_SPEED_SENSOR_ID + 0, PLDM_NUMERIC_SENSOR_STATE, UNKNOWN, UNKNOWN, PLDM_DATASIZE_UINT32, 0},
    [1] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_EN, COMMUNICATION_PORT, PLDM_BASE_LINK_SPEED_SENSOR_ID + 1, PLDM_NUMERIC_SENSOR_STATE, UNKNOWN, UNKNOWN, PLDM_DATASIZE_UINT32, 0},
    [2] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_EN, COMMUNICATION_PORT, PLDM_BASE_LINK_SPEED_SENSOR_ID + 2, PLDM_NUMERIC_SENSOR_STATE, UNKNOWN, UNKNOWN, PLDM_DATASIZE_UINT32, 0},
    [3] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_EN, COMMUNICATION_PORT, PLDM_BASE_LINK_SPEED_SENSOR_ID + 3, PLDM_NUMERIC_SENSOR_STATE, UNKNOWN, UNKNOWN, PLDM_DATASIZE_UINT32, 0}
};

pldm_plug_power_data_struct_t plug_power_sensors[PLDM_PLUG_POWER_SENSOR_NUM] = {
    [0] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_EN, UNKNOWN, PLDM_BASE_PLUG_POWER_SENSOR_ID + 0, PLDM_NUMERIC_SENSOR_STATE, UNKNOWN, UNKNOWN, PLDM_DATASIZE_UINT32, 0},
    [1] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_EN, UNKNOWN, PLDM_BASE_PLUG_POWER_SENSOR_ID + 1, PLDM_NUMERIC_SENSOR_STATE, UNKNOWN, UNKNOWN, PLDM_DATASIZE_UINT32, 0},
    [2] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_EN, UNKNOWN, PLDM_BASE_PLUG_POWER_SENSOR_ID + 2, PLDM_NUMERIC_SENSOR_STATE, UNKNOWN, UNKNOWN, PLDM_DATASIZE_UINT32, 0},
    [3] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_EN, UNKNOWN, PLDM_BASE_PLUG_POWER_SENSOR_ID + 3, PLDM_NUMERIC_SENSOR_STATE, UNKNOWN, UNKNOWN, PLDM_DATASIZE_UINT32, 0}
};

pldm_nic_composite_state_sensor_data_struct_t nic_composite_state_sensors[4] = {
    /* op_state | previous_op_state | event_msg_en | sensor type | sensor id | sensor event class | offset | present val | previous val */
    [0] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, HEALTH_STATE, PLDM_BASE_CARD_COMPOSITE_STATE_SENSOR_ID + 0, PLDM_STATE_SENSOR_STATE, 0, NORMAL, NOT_FIELD},
    [1] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, CONFIGURATION, PLDM_BASE_CARD_COMPOSITE_STATE_SENSOR_ID + 0, PLDM_STATE_SENSOR_STATE, 1, VALID_CONFIG, VALID_CONFIG},
    [2] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, CONFIGURATION_CHG, PLDM_BASE_CARD_COMPOSITE_STATE_SENSOR_ID + 0, PLDM_STATE_SENSOR_STATE, 2, NORMAL, NORMAL},
    [3] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, THERMAL_TRIP, PLDM_BASE_CARD_COMPOSITE_STATE_SENSOR_ID + 0, PLDM_STATE_SENSOR_STATE, 3, NORMAL, NOT_FIELD}
};

pldm_controller_composite_state_sensor_data_struct_t controller_composite_state_sensors[5] = {
    [0] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, HEALTH_STATE, PLDM_BASE_NC_STATE_SENSOR_ID + 0, PLDM_STATE_SENSOR_STATE, 0, NORMAL, NOT_FIELD},
    [1] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, CONFIGURATION, PLDM_BASE_NC_STATE_SENSOR_ID + 0, PLDM_STATE_SENSOR_STATE, 1, VALID_CONFIG, VALID_CONFIG},
    [2] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, CONFIGURATION_CHG, PLDM_BASE_NC_STATE_SENSOR_ID + 0, PLDM_STATE_SENSOR_STATE, 2, NORMAL, NORMAL},
    [3] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, THERMAL_TRIP, PLDM_BASE_NC_STATE_SENSOR_ID + 0, PLDM_STATE_SENSOR_STATE, 3, NORMAL, NOT_FIELD},
    [4] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, FW_VERSION, PLDM_BASE_NC_STATE_SENSOR_ID + 0, PLDM_STATE_SENSOR_STATE, 4, NORMAL, NORMAL}
};

pldm_plug_composite_state_sensor_data_struct_t plug_composite_state_sensors[12] = {
        [0] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, HEALTH_STATE, PLDM_BASE_PLUG_COMPOSITE_SENSOR_ID + 0, PLDM_STATE_SENSOR_STATE, 0, UNKNOWN, UNKNOWN},
        [1] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, PRESENCE, PLDM_BASE_PLUG_COMPOSITE_SENSOR_ID + 0, PLDM_STATE_SENSOR_STATE, 1, NOT_PRESENCE, NOT_PRESENCE},
        [2] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, THERMAL_TRIP, PLDM_BASE_PLUG_COMPOSITE_SENSOR_ID + 0, PLDM_STATE_SENSOR_STATE, 2, UNKNOWN, UNKNOWN},

        [3] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, HEALTH_STATE, PLDM_BASE_PLUG_COMPOSITE_SENSOR_ID + 1, PLDM_STATE_SENSOR_STATE, 0, UNKNOWN, UNKNOWN},
        [4] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, PRESENCE, PLDM_BASE_PLUG_COMPOSITE_SENSOR_ID + 1, PLDM_STATE_SENSOR_STATE, 1, NOT_PRESENCE, NOT_PRESENCE},
        [5] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, THERMAL_TRIP, PLDM_BASE_PLUG_COMPOSITE_SENSOR_ID + 1, PLDM_STATE_SENSOR_STATE, 2, UNKNOWN, UNKNOWN},

        [6] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, HEALTH_STATE, PLDM_BASE_PLUG_COMPOSITE_SENSOR_ID + 2, PLDM_STATE_SENSOR_STATE, 0, UNKNOWN, UNKNOWN},
        [7] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, PRESENCE, PLDM_BASE_PLUG_COMPOSITE_SENSOR_ID + 2, PLDM_STATE_SENSOR_STATE, 1, NOT_PRESENCE, NOT_PRESENCE},
        [8] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, THERMAL_TRIP, PLDM_BASE_PLUG_COMPOSITE_SENSOR_ID + 2, PLDM_STATE_SENSOR_STATE, 2, UNKNOWN, UNKNOWN},

        [9] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, HEALTH_STATE, PLDM_BASE_PLUG_COMPOSITE_SENSOR_ID + 3, PLDM_STATE_SENSOR_STATE, 0, UNKNOWN, UNKNOWN},
        [10] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, PRESENCE, PLDM_BASE_PLUG_COMPOSITE_SENSOR_ID + 3, PLDM_STATE_SENSOR_STATE, 1, NOT_PRESENCE, NOT_PRESENCE},
        [11] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, THERMAL_TRIP, PLDM_BASE_PLUG_COMPOSITE_SENSOR_ID + 3, PLDM_STATE_SENSOR_STATE, 2, UNKNOWN, UNKNOWN}
};

pldm_simple_sensor_data_struct_t simple_sensors[4] = {
    [0] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, PORT, PLDM_BASE_LINK_STATE_SENSOR_ID + 0, PLDM_STATE_SENSOR_STATE, 0, DISCONNECTED, DISCONNECTED},
    [1] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, PORT, PLDM_BASE_LINK_STATE_SENSOR_ID + 1, PLDM_STATE_SENSOR_STATE, 0, DISCONNECTED, DISCONNECTED},
    [2] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, PORT, PLDM_BASE_LINK_STATE_SENSOR_ID + 2, PLDM_STATE_SENSOR_STATE, 0, DISCONNECTED, DISCONNECTED},
    [3] = {PLDM_OP_ENABLE, PLDM_OP_ENABLE, PLDM_EVENT_STATE_EN_ONLY, PORT, PLDM_BASE_LINK_STATE_SENSOR_ID + 3, PLDM_STATE_SENSOR_STATE, 0, DISCONNECTED, DISCONNECTED}
};

pldm_temp_sensor_monitor_t temp_sensor_monitor[PLDM_NIC_TEMP_SENSOR_NUM + PLDM_NC_TEMP_SENSOR_NUM + PLDM_USED_PLUG_TEMP_SENSOR_NUM] = {
    [0] = {NULL, PLDM_BASE_NIC_TEMP_SENSOR_ID},

    [1] = {NULL, PLDM_BASE_NC_TEMP_SENSOR_ID},

    [2] = {NULL, PLDM_BASE_PLUG_TEMP_SENSOR_ID + 0},
    [3] = {NULL, PLDM_BASE_PLUG_TEMP_SENSOR_ID + 1},
    #if PLDM_USED_PLUG_TEMP_SENSOR_NUM == 4
    [4] = {NULL, PLDM_BASE_PLUG_TEMP_SENSOR_ID + 2},
    [5] = {NULL, PLDM_BASE_PLUG_TEMP_SENSOR_ID + 3}
    #endif
};

extern mctp_base_info g_mctp_ctrl_info[3];
extern pldm_monitor_base_info_t g_pldm_monitor_info;

extern int is_temp_sensor(u16 sensor_id);

void pdrs_pool_init(u32 *addr)
{
    /* TODO: ALIGN table num and entry num addr */
    pdrs_pool = addr;
    pdrs_pool_wt = 0;

    LOG("%s addr 0x%X, total size 0x%X",
        __FUNCTION__, addr, PDR_POOL_SIZE);
}

void *pdr_malloc(int size)
{
    if (pdrs_pool_wt + size >= PDR_POOL_SIZE || !pdrs_pool) {
        LOG( "pdr_malloc failed");
        return NULL;
    }

    u8 *pt = (u8 *)pdrs_pool + pdrs_pool_wt;
    pdrs_pool_wt += size;
    cm_memset((void *)pt, 0, size);

    return pt;
}

u32 pldm_pdr_get_used(void)
{
    LOG("used space : %d, %d", ALIGN(pdrs_pool_wt, 128), pdrs_pool_wt);
    return pdrs_pool_wt;
}

int pldm_pdr_cmp(pldm_pdr_record_t *out, pldm_pdr_record_t *elt)
{
    if (!out || !elt) return -1;
    if (out->record_handle < elt->record_handle) {
        return -1;
    } else if (out->record_handle == elt->record_handle) {
        return 0;
    } else {
        return 1;
    }
}

void pldm_pdr_init(pldm_pdr_t *repo)
{
    if (!repo) return;
	repo->record_count = 0;
	repo->size = 0;
    repo->largest_pdr_size = 0;
    repo->repo_signature = 0;

    repo->update_time.utc_offset = 0;
    repo->update_time.utc_resolution = UTC_RSLV_MINUTE;
    repo->update_time.time_resolution = T_RSLV_MILLISECOND;
    repo->update_time.year = 0;
    repo->update_time.mon = 0;
    repo->update_time.day = 0;
    repo->update_time.hour = 0;
    repo->update_time.min = 0;
    repo->update_time.sec = 0;
    cm_memset(repo->update_time.microsecond, 0, sizeof(repo->update_time.microsecond));

	repo->head = NULL;
    repo->is_deleted_head = NULL;
}

int pldm_pdr_add(pldm_pdr_t *repo, u8 *pdr_data, u32 pdr_size, u32 record_handle)
{
    int ret = -1;
    if (!repo || !pdr_data) goto L_RET;

    pldm_pdr_record_t *add_pdr = (pldm_pdr_record_t *)pdr_malloc(sizeof(pldm_pdr_record_t));
    if (!add_pdr) goto L_RET;

    add_pdr->data = pdr_data;
    add_pdr->record_handle = record_handle;
    add_pdr->size = pdr_size;

    LL_INSERT_INORDER(repo->head, add_pdr, pldm_pdr_cmp);

    ++repo->record_count;
    repo->size += add_pdr->size;
    repo->largest_pdr_size = ALIGN(MAX(repo->largest_pdr_size, pdr_size), 64);
    ret = 0;

L_RET:
    return ret;
}

static u32 pldm_largest_pdrsize_get(pldm_pdr_t *repo)
{
    if (!repo || !(repo->head))
        return 0;

    u32 largets_pdr_size = 0;
    pldm_pdr_record_t *pdr = repo->head;
    for (int i = 0; i < repo->record_count; i++) {
        if (largets_pdr_size <= pdr->size) {
            largets_pdr_size = pdr->size;
            pdr = pdr->next;
        }
    }
    largets_pdr_size = ALIGN(largets_pdr_size, 64);
    return largets_pdr_size;
}

pldm_pdr_record_t *pldm_pdr_find(pldm_pdr_t *repo, u32 record_handle)
{
    if (!repo || record_handle == PLDM_ERR_RECORD_HANDLE || !(repo->head)) {
        return NULL;
    }

    pldm_pdr_record_t *out = NULL;
    pldm_pdr_record_t elt;
    elt.record_handle = record_handle;
    LL_SEARCH(repo->head, out, &elt, pldm_pdr_cmp);
    return out ? out : NULL;
}

int pldm_pdr_delete(pldm_pdr_t *repo, u32 record_handle)
{
    pldm_pdr_record_t *out = pldm_pdr_find(repo, record_handle);
    if (out) {
        /* clear record_change_num if pdr is deleted. */
        pldm_pdr_hdr_t *pdr_hdr = (pldm_pdr_hdr_t *)(out->data);
        pdr_hdr->record_change_num = 0;
        LL_DELETE(repo->head, out);
        LL_APPEND(repo->is_deleted_head, out);
        repo->size -= out->size;
        repo->largest_pdr_size = pldm_largest_pdrsize_get(repo);
        --repo->record_count;
    }
    return out ? 0 : -1;
}
pldm_pdr_record_t *pldm_pdr_is_exist(pldm_pdr_t *repo, u32 record_handle)
{
    if (!repo || record_handle == PLDM_ERR_RECORD_HANDLE) {
        return NULL;
    }

    pldm_pdr_record_t *out = NULL;
    pldm_pdr_record_t elt;
    elt.record_handle = record_handle;
    if (repo->head) {
        LL_SEARCH(repo->head, out, &elt, pldm_pdr_cmp);
        if (out) return out;
    }

    if (repo->is_deleted_head) {
        LL_SEARCH(repo->is_deleted_head, out, &elt, pldm_pdr_cmp);
        if (out) {
            LL_DELETE(repo->is_deleted_head, out);
            LL_INSERT_INORDER(repo->head, out, pldm_pdr_cmp);
            repo->largest_pdr_size = pldm_largest_pdrsize_get(repo);
            repo->size += out->size;
            ++repo->record_count;
            pldm_pdr_chg_event_generate(g_pldm_monitor_info.pldm_event_rbuf, PLDM_REPO_CHG_FORMAT_ID_PDR_HANDLE, PLDM_REPO_CHG_RECORDS_ADDED, out->record_handle);
        }
    }
    return out;
}

static void temp_sensor_monitor_handle(u16 sensor_id, u32 *pdr_addr)
{
    for (u8 i = 0; i < (PLDM_NIC_TEMP_SENSOR_NUM + PLDM_NC_TEMP_SENSOR_NUM + PLDM_USED_PLUG_TEMP_SENSOR_NUM); i++) {
        if (temp_sensor_monitor[i].sensor_id == sensor_id) {
            temp_sensor_monitor[i].pdr_addr = pdr_addr;
            return;
        }
    }
}

static int fill_common_thermal_sensor_pdr(void *buf, u16 sensor_id, u16 entity_type, u8 sensor_datasize)
{
    if (!buf) return FAILURE;
    pldm_numeric_sensor_pdr_t *thermal_pdr = (pldm_numeric_sensor_pdr_t *)buf;

    u32 record_handle = sensor_id_convert_to_record_handle(sensor_id);

    thermal_pdr->numeric_pdr_comm_part.hdr.record_handle = record_handle;
    thermal_pdr->numeric_pdr_comm_part.hdr.version = PLDM_EVENT_FORMAT_VERSION;
    thermal_pdr->numeric_pdr_comm_part.hdr.type = NUMERIC_SENSOR_PDR;
    // thermal_pdr->numeric_pdr_comm_part.hdr.record_change_num = 0;
    thermal_pdr->numeric_pdr_comm_part.hdr.length = sizeof(pldm_numeric_sensor_pdr_t) - sizeof(pldm_pdr_hdr_t); /* (67)? E810 */

    thermal_pdr->numeric_pdr_comm_part.terminus_handle = PLDM_TERMINUS_HANDLE;
    thermal_pdr->numeric_pdr_comm_part.sensor_id = sensor_id;
    thermal_pdr->numeric_pdr_comm_part.container.entity_type = entity_type;

    u8 port = 0;
    u8 sensor_type = NIC_TEMP_SENSOR;
    if (entity_type == NIC_SENSOR) {
        thermal_pdr->numeric_pdr_comm_part.container.entity_instance_num = sensor_id - PLDM_BASE_NIC_TEMP_SENSOR_ID + 1;
        thermal_pdr->numeric_pdr_comm_part.container.entity_container_id = PLDM_BASE_NIC_CONTAINER_ID;
    } else if (entity_type == CONTROLLER_SENSOR) {
        thermal_pdr->numeric_pdr_comm_part.container.entity_instance_num = sensor_id - PLDM_BASE_NC_TEMP_SENSOR_ID + 1;
        thermal_pdr->numeric_pdr_comm_part.container.entity_container_id = PLDM_BASE_NC_CONTAINER_ID;
        sensor_type = CONTROLLER_SENSOR;
    } else {    /* For Plug Sensor: */
        thermal_pdr->numeric_pdr_comm_part.container.entity_instance_num = sensor_id - PLDM_BASE_PLUG_TEMP_SENSOR_ID + 1;
        thermal_pdr->numeric_pdr_comm_part.container.entity_container_id = PLDM_BASE_PLUG_CONTAINER_ID + thermal_pdr->numeric_pdr_comm_part.container.entity_instance_num - 1;
        port = thermal_pdr->numeric_pdr_comm_part.container.entity_instance_num -1;
        sensor_type = PLUG_TEMP_SENSOR;
    }

    pldm_temp_sensor_threshold_data_t temp_data;
    pldm_get_temp_sensor_threshold(&temp_data, sensor_type, port);

    // thermal_pdr->numeric_pdr_comm_part.sensor_init = NO_INIT;
    thermal_pdr->numeric_pdr_comm_part.sensor_auxiliary_names_pdr = FALSE;
    thermal_pdr->numeric_pdr_comm_part.base_unit = DEGREES_C;
    // thermal_pdr->numeric_pdr_comm_part.unit_modifier = 0x00;
    // thermal_pdr->numeric_pdr_comm_part.rate_unit = 0x00;
    // thermal_pdr->numeric_pdr_comm_part.base_oem_unit_handle = 0x00;
    // thermal_pdr->numeric_pdr_comm_part.aux_unit = 0x00;
    // thermal_pdr->numeric_pdr_comm_part.aux_unit_modifier = 0x00;
    // thermal_pdr->numeric_pdr_comm_part.aux_rate_unit = 0x00;
    // thermal_pdr->numeric_pdr_comm_part.rel = 0x00;
    // thermal_pdr->numeric_pdr_comm_part.aux_oem_unit_handle = 0x00;
    thermal_pdr->numeric_pdr_comm_part.is_linear = TRUE;
    thermal_pdr->numeric_pdr_comm_part.sensor_data_size = sensor_datasize;
    thermal_pdr->numeric_pdr_comm_part.resolution.val = 0x3F800000;                      /* 1 */
    // thermal_pdr->numeric_pdr_comm_part.offset =
    // thermal_pdr->numeric_pdr_comm_part.accuracy =
    thermal_pdr->numeric_pdr_comm_part.plus_tolerace = 0x03;
    thermal_pdr->numeric_pdr_comm_part.minus_tolerance = 0x03;
    thermal_pdr->thermal_pdr.hysteresis = 1;
    thermal_pdr->thermal_pdr.supported_thresholds = BIT(UPPER_THRESHOLD_WARNING) | BIT(UPPER_THRESHOLD_CRITICAL) | BIT(UPPER_THRESHOLD_FATAL);
    thermal_pdr->thermal_pdr.threshold_and_hysteresis_volatility = BIT(PLDM_SYS_POWER_UP) | BIT(INITIALIZATION_AGENT_CONTROLLER);
    thermal_pdr->thermal_pdr.state_transition_interval.val = 0x3F800000;                 /* Return the internal firmware sensors polling interval (in real32). Current value is 1 second (0x3F800000). */
    thermal_pdr->thermal_pdr.update_interval.val = 0x3F800000;                           /* Return the internal firmware sensors polling interval (in real32). Current value is 1 second (0x3F800000). */
    thermal_pdr->thermal_pdr.max_readable = 127;
    thermal_pdr->thermal_pdr.min_readable = 0;
    thermal_pdr->thermal_pdr.range_field_format = 0x02;                     /* uint16 */
    thermal_pdr->thermal_pdr.range_field_support = BIT(NORMAL_MAX) | BIT(CRITICAL_HIGH) | BIT(FATAL_HIGH);
    thermal_pdr->thermal_pdr.normal_max = 127;
    thermal_pdr->thermal_pdr.nominal_value = 127;
    // thermal_pdr->thermal_pdr.normal_min = 0x00;

    thermal_pdr->thermal_pdr.warning_high = temp_data.warning_high;
    // thermal_pdr->thermal_pdr.warning_low = 0x00;
    thermal_pdr->thermal_pdr.critical_high = temp_data.critical_high;
    // thermal_pdr->thermal_pdr.critical_low = 0x00;
    thermal_pdr->thermal_pdr.fatal_high = temp_data.fatal_high;
    // thermal_pdr->thermal_pdr.fatal_low = 0x00;
    return SUCCESS;
}

static int fill_common_pluggable_module_power_sensor_pdr(void *buf, u16 sensor_id, u16 entity_type, u8 sensor_datasize)
{
    if (!buf) return FAILURE;
    pldm_numeric_sensor_pdr_t *pluggable_power_pdr = (pldm_numeric_sensor_pdr_t *)buf;

    u32 record_handle = sensor_id_convert_to_record_handle(sensor_id);

    pluggable_power_pdr->numeric_pdr_comm_part.hdr.record_handle = record_handle;             /* 1699 + plug number (1..n) + PLDM_Offset*8 */
    pluggable_power_pdr->numeric_pdr_comm_part.hdr.version = PLDM_EVENT_FORMAT_VERSION;
    pluggable_power_pdr->numeric_pdr_comm_part.hdr.type = NUMERIC_SENSOR_PDR;
    // pluggable_power_pdr->numeric_pdr_comm_part.hdr.record_change_num = 0;
    pluggable_power_pdr->numeric_pdr_comm_part.hdr.length = sizeof(pldm_numeric_sensor_pdr_t) - sizeof(pldm_pdr_hdr_t);  /* (71)? E810 */

    pluggable_power_pdr->numeric_pdr_comm_part.terminus_handle = PLDM_TERMINUS_HANDLE;
    pluggable_power_pdr->numeric_pdr_comm_part.sensor_id = sensor_id;                         /* 249 + plug number (1..n) */
    pluggable_power_pdr->numeric_pdr_comm_part.container.entity_type = entity_type;           /* 206 (SFP28 Module) / 207 (SFP+ Module) / 211 (QSFP28 Module) / 212 (QSFP+ Module) */
    u16 entity_instance_num = pluggable_power_pdr->numeric_pdr_comm_part.container.entity_instance_num = sensor_id - PLDM_BASE_PLUG_POWER_SENSOR_ID + 1;
    /* 1039 + Cage# + PLDM_Offset*8 (Connector) */
    pluggable_power_pdr->numeric_pdr_comm_part.container.entity_container_id = PLDM_BASE_CONNECTOR_CONTAINER_ID + entity_instance_num - 1;

    // pluggable_power_pdr->numeric_pdr_comm_part.sensor_init = NO_INIT;
    pluggable_power_pdr->numeric_pdr_comm_part.sensor_auxiliary_names_pdr = FALSE;
    pluggable_power_pdr->numeric_pdr_comm_part.base_unit = WATTS;
    pluggable_power_pdr->numeric_pdr_comm_part.unit_modifier = -1;
    // pluggable_power_pdr->numeric_pdr_comm_part.rate_unit = 0x00;
    // pluggable_power_pdr->numeric_pdr_comm_part.base_oem_unit_handle = 0x00;
    // pluggable_power_pdr->numeric_pdr_comm_part.aux_unit = 0x00;
    // pluggable_power_pdr->numeric_pdr_comm_part.aux_unit_modifier = 0x00;
    // pluggable_power_pdr->numeric_pdr_comm_part.aux_rate_unit = 0x00;
    // pluggable_power_pdr->numeric_pdr_comm_part.rel = 0x00;
    // pluggable_power_pdr->numeric_pdr_comm_part.aux_oem_unit_handle = 0x00;
    pluggable_power_pdr->numeric_pdr_comm_part.is_linear = TRUE;
    pluggable_power_pdr->numeric_pdr_comm_part.sensor_data_size = sensor_datasize;
    pluggable_power_pdr->numeric_pdr_comm_part.resolution.val = 0x3A83126E;                  /* 0.0000001 */
    // pluggable_power_pdr->numeric_pdr_comm_part.offset = 0x00;
    // pluggable_power_pdr->numeric_pdr_comm_part.accuracy = 0x00;
    pluggable_power_pdr->numeric_pdr_comm_part.plus_tolerace = 0x03;
    pluggable_power_pdr->numeric_pdr_comm_part.minus_tolerance = 0x03;
    // pluggable_power_pdr->pluggable_power_pdr.hysteresis = 0x00;
    // pluggable_power_pdr->pluggable_power_pdr.supported_thresholds = 0x00;
    // pluggable_power_pdr->pluggable_power_pdr.threshold_and_hysteresis_volatility = 0x00;
    pluggable_power_pdr->pluggable_power_pdr.state_transition_interval.val = 0x3DCCCCCC;   /* 100 ms - an upper limit on the firmware reaction time */
    // pluggable_power_pdr->pluggable_power_pdr.update_interval = 0x00;                    /* static value */
    pluggable_power_pdr->pluggable_power_pdr.max_readable = 65535;                         /* Actual power of plugged device == nominalValue == value reported by sensor. unit is mW */
    // pluggable_power_pdr->pluggable_power_pdr.min_readable = 0x00;
    pluggable_power_pdr->pluggable_power_pdr.range_field_format = 0x01;                    /* uint8 */
    pluggable_power_pdr->pluggable_power_pdr.range_field_support = BIT(NORMAL_VAL);
    pluggable_power_pdr->pluggable_power_pdr.nominal_value = 1;                            /* Actual power of plugged device. 1 watt */
    pluggable_power_pdr->pluggable_power_pdr.normal_max = 1;                               /* 1 watt */
    // pluggable_power_pdr->pluggable_power_pdr.normal_min = 0x00;

    // pluggable_power_pdr->pluggable_power_pdr.warning_high = 0x00;
    // pluggable_power_pdr->pluggable_power_pdr.warning_low = 0x00;
    // pluggable_power_pdr->pluggable_power_pdr.critical_high = 0x00;
    // pluggable_power_pdr->pluggable_power_pdr.critical_low = 0x00;
    // pluggable_power_pdr->pluggable_power_pdr.fatal_high = 0x00;
    // pluggable_power_pdr->pluggable_power_pdr.fatal_low = 0x00;
    return SUCCESS;
}

static int fill_common_link_speed_sensor_pdr(void *buf, u16 sensor_id, u16 entity_type, u8 sensor_datasize)
{
    if (!buf) return FAILURE;
    pldm_link_speed_sensor_pdr_t *link_speed_pdr = (pldm_link_speed_sensor_pdr_t *)buf;

    u32 record_handle = sensor_id_convert_to_record_handle(sensor_id);

    link_speed_pdr->numeric_pdr_comm_part.hdr.record_handle = record_handle;             /* 1299 + port number (1..n) + PLDM_Offset*8 */
    link_speed_pdr->numeric_pdr_comm_part.hdr.version = PLDM_EVENT_FORMAT_VERSION;
    link_speed_pdr->numeric_pdr_comm_part.hdr.type = NUMERIC_SENSOR_PDR;
    // link_speed_pdr->numeric_pdr_comm_part.hdr.record_change_num = 0;
    link_speed_pdr->numeric_pdr_comm_part.hdr.length = sizeof(pldm_link_speed_sensor_pdr_t) - sizeof(pldm_pdr_hdr_t);

    link_speed_pdr->numeric_pdr_comm_part.terminus_handle = PLDM_TERMINUS_HANDLE;
    link_speed_pdr->numeric_pdr_comm_part.sensor_id = sensor_id;                         /* 99 + port number (1..n) + PLDM Offset*8 */
    link_speed_pdr->numeric_pdr_comm_part.container.entity_type = entity_type;           /* (Physical | Port (6)) */
    link_speed_pdr->numeric_pdr_comm_part.container.entity_instance_num = sensor_id - PLDM_BASE_LINK_SPEED_SENSOR_ID + 1;
    link_speed_pdr->numeric_pdr_comm_part.container.entity_container_id = PLDM_BASE_NC_CONTAINER_ID; /* 1000 + PLDM Offset */

    // link_speed_pdr->numeric_pdr_comm_part.sensor_init = NO_INIT;
    link_speed_pdr->numeric_pdr_comm_part.sensor_auxiliary_names_pdr = FALSE;
    link_speed_pdr->numeric_pdr_comm_part.base_unit = BITS;
    link_speed_pdr->numeric_pdr_comm_part.unit_modifier = 0x6;                           /* mbps */
    link_speed_pdr->numeric_pdr_comm_part.rate_unit = 0x03;                              /* per second */
    // link_speed_pdr->numeric_pdr_comm_part.base_oem_unit_handle = 0x00;
    // link_speed_pdr->numeric_pdr_comm_part.aux_unit = 0x00;
    // link_speed_pdr->numeric_pdr_comm_part.aux_unit_modifier = 0x00;
    // link_speed_pdr->numeric_pdr_comm_part.aux_rate_unit = 0x00;
    // link_speed_pdr->numeric_pdr_comm_part.rel = 0x00;
    // link_speed_pdr->numeric_pdr_comm_part.aux_oem_unit_handle = 0x00;
    link_speed_pdr->numeric_pdr_comm_part.is_linear = TRUE;
    link_speed_pdr->numeric_pdr_comm_part.sensor_data_size = sensor_datasize;
    link_speed_pdr->numeric_pdr_comm_part.resolution.val = 0x3F800000;                  /* 1 */
    // link_speed_pdr->numeric_pdr_comm_part.offset = 0x00;
    // link_speed_pdr->numeric_pdr_comm_part.accuracy = 0x00;
    // link_speed_pdr->numeric_pdr_comm_part.plus_tolerace = 0x00;
    // link_speed_pdr->numeric_pdr_comm_part.minus_tolerance = 0x00;
    // link_speed_pdr->hysteresis = 0x00;
    // link_speed_pdr->supported_thresholds = 0x00;
    // link_speed_pdr->threshold_and_hysteresis_volatility = 0x00;
    link_speed_pdr->state_transition_interval.val = 0x3DCCCCCC;               /* 100 ms - an upper limit on the firmware reaction time */
    link_speed_pdr->update_interval.val = 0x3D4CCCCC;                         /* 50 ms - upper limit on the time from interrupt to event message */
    u8 port = link_speed_pdr->numeric_pdr_comm_part.container.entity_instance_num - 1;
    pldm_get_link_spd_cap((pldm_speed_sensor_cap_t *)&(link_speed_pdr->max_readable), port);
    // link_speed_pdr->max_readable = ;                                          /* The maximum speed supported by the media in Mb/s */
    // link_speed_pdr->min_readable = 0x00;                                   /* returned when link is down */
    link_speed_pdr->range_field_format = 0x04;                                /* uint32 */
    link_speed_pdr->range_field_support = BIT(NORMAL_VAL);
    link_speed_pdr->nominal_value = link_speed_pdr->max_readable;             /* The maximum speed supported by the media (in real32) */
    // link_speed_pdr->normal_max = 0x00;
    // link_speed_pdr->normal_min = 0x00;
    return SUCCESS;
}

static int fill_terminus_locator_pdr(pldm_terminus_locator_pdr_t *terminus_locator_pdr)
{
    if (!terminus_locator_pdr) return FAILURE;
    terminus_locator_pdr->hdr.record_handle = PLDM_TERMINUS_LOCATOR_PDR_HANDLE;
    terminus_locator_pdr->hdr.version = PLDM_EVENT_FORMAT_VERSION;
    terminus_locator_pdr->hdr.type = TERMINUS_LOCATOR_PDR;
    terminus_locator_pdr->hdr.record_change_num = 0;                /* no changes expected in Terminus Locator PDR */
    terminus_locator_pdr->hdr.length = sizeof(pldm_terminus_locator_pdr_t) - sizeof(pldm_pdr_hdr_t);

    terminus_locator_pdr->terminus_handle = PLDM_TERMINUS_HANDLE;
    terminus_locator_pdr->validity = 0x01;                          /* valid */
    terminus_locator_pdr->tid = g_pldm_monitor_info.tid;
    terminus_locator_pdr->container_id = PLDM_SYSTEM;               /* system */
    terminus_locator_pdr->terminus_locator_type = 0x01;             /* MCTP_EID */
    terminus_locator_pdr->terminus_locator_value_size = 0x01;       /* one uint8 */
    terminus_locator_pdr->eid = g_mctp_ctrl_info[1].dev_eid;        /* 待定 */

    return SUCCESS;
}

static pldm_pdr_entity_assoc_t *assoc_pdr_create_container(pldm_entity_t *container, u32 record_handle, u16 container_id, u8 expected_contained_num, u8 assoc_type)
{
    if (!container) {
        return NULL;
    }

	pldm_pdr_entity_assoc_t *assoc_container = (pldm_pdr_entity_assoc_t *)pdr_malloc(sizeof(pldm_pdr_entity_assoc_t) + \
    expected_contained_num * sizeof(pldm_entity_t));

    if (!assoc_container) {
        LOG("no more space for malloc!, %s", __FUNCTION__);
        return NULL;
    }

    assoc_container->hdr.record_handle = record_handle;
    assoc_container->hdr.version = PLDM_EVENT_FORMAT_VERSION;
    assoc_container->hdr.type = ENTITY_ASSOC_PDR;
    assoc_container->hdr.record_change_num = 0;
    assoc_container->hdr.length = sizeof(pldm_pdr_entity_assoc_t) - sizeof(pldm_pdr_hdr_t);

    assoc_container->assoc_type = assoc_type;
    assoc_container->container_id = container_id;
    assoc_container->contained_entity_cnt = 0;
    mctp_memcpy_fast(&(assoc_container->container), container, sizeof(pldm_entity_t));

    return assoc_container;
}

static pldm_pdr_entity_assoc_t *assoc_pdr_add_contained(pldm_pdr_entity_assoc_t *container, pldm_entity_t *contained, u8 pos)
{
    if (!container || !contained) {
        return NULL;
    }

    container->hdr.record_change_num++;
    container->hdr.length += sizeof(pldm_entity_t);
    container->contained_entity_cnt++;

    contained->entity_container_id = container->container_id;
    mctp_memcpy_fast(&(container->contained[pos]), contained, sizeof(pldm_entity_t));
    return container;
}

static pldm_composite_state_sensor_pdr_t *composite_state_pdr_create_sensor(pldm_entity_t *composite_state_sensor, u16 sensor_id, u8 expected_sensor_count)
{
    if (!composite_state_sensor) {
        return NULL;
    }

	pldm_composite_state_sensor_pdr_t *composite_state_pdr = (pldm_composite_state_sensor_pdr_t *)pdr_malloc(sizeof(pldm_composite_state_sensor_pdr_t) + \
    expected_sensor_count * sizeof(pldm_composite_sensor_attr_t));

    if (!composite_state_pdr) {
        LOG("no more space for malloc!, %s", __FUNCTION__);
        return NULL;
    }

    composite_state_pdr->hdr.record_handle = sensor_id_convert_to_record_handle(sensor_id);
    composite_state_pdr->hdr.version = PLDM_EVENT_FORMAT_VERSION;
    composite_state_pdr->hdr.type = STATE_SENSOR_PDR;
    composite_state_pdr->hdr.record_change_num = 0;
    composite_state_pdr->hdr.length = sizeof(pldm_composite_state_sensor_pdr_t) - sizeof(pldm_pdr_hdr_t);

    composite_state_pdr->terminus_handle = PLDM_TERMINUS_HANDLE;
    composite_state_pdr->sensor_id = sensor_id;

    composite_state_pdr->sensor_init = EN_SENSOR;
    composite_state_pdr->sensor_auxi_names_pdr = FALSE;
    composite_state_pdr->composite_sensor_cnt = 0;

    mctp_memcpy_fast(&(composite_state_pdr->container), composite_state_sensor, sizeof(pldm_entity_t));

    return composite_state_pdr;
}

static pldm_composite_state_sensor_pdr_t *composite_state_pdr_add_sensor(pldm_composite_state_sensor_pdr_t *composite_state_sensor, pldm_composite_sensor_attr_t *add_sensor)
{
    if (!composite_state_sensor || !add_sensor) {
        return NULL;
    }
    // composite_state_sensor->hdr.record_change_num++;
    composite_state_sensor->hdr.length += sizeof(pldm_composite_sensor_attr_t);

    mctp_memcpy_fast(&(composite_state_sensor->sensors[composite_state_sensor->composite_sensor_cnt]), add_sensor, sizeof(pldm_composite_sensor_attr_t));
    composite_state_sensor->composite_sensor_cnt++;

    return composite_state_sensor;
}

static u8 possible_states_generate(u8 sensor_type)
{
    u8 possible_states = 0;
    switch (sensor_type)
    {
        case HEALTH_STATE :
            possible_states = BIT(UNKNOWN) | BIT(NORMAL) | BIT(CRITICAL) | BIT(UPPER_NON_CRITICAL) | BIT(UPPER_CRITICAL) | BIT(FATAL);
            break;
        case PRESENCE :
            possible_states = BIT(IS_PRESENCE) | BIT(NOT_PRESENCE);
            break;
        case CONFIGURATION :
            possible_states = BIT(VALID_CONFIG) | BIT(INVALID_CONFIG);
            break;
        case CONFIGURATION_CHG :
            possible_states = BIT(NORMAL) | BIT(CONFIG_CHG);
            break;
        case FW_VERSION :
            possible_states = BIT(NORMAL) | BIT(VER_CHG_DETECTED);
            break;
        case THERMAL_TRIP :
            possible_states = BIT(NORMAL) | BIT(UNKNOWN) | BIT(THERMAL_TRIPED);
            break;
        case LINK_STATE :
            possible_states = BIT(CONNECTED) | BIT(DISCONNECTED);
        break;
    }
    return possible_states;
}

static void pldm_add_state_sensor_pdr(u8 subsensor_count, u8 sensor_num, u16 entity_type, u16 entity_container_id, pldm_state_data_struct_t *datastruct, u8 is_plug_sensor)
{
	if (!datastruct) return;
    pldm_composite_state_sensor_pdr_t *composite_state_sensor_pdr = NULL;
    pldm_entity_t composite_state_sensor;
    pldm_composite_sensor_attr_t add_sensor;

    for (int i = 0; i < sensor_num; i++) {
        composite_state_sensor.entity_type = entity_type;
        composite_state_sensor.entity_instance_num = i + 1;
        composite_state_sensor.entity_container_id = entity_container_id;   /* entity_container_id + i ? */

        u32 record_handle = sensor_id_convert_to_record_handle(datastruct->sensor_id + i);
        if (record_handle == PLDM_ERR_RECORD_HANDLE) continue;
        pldm_pdr_record_t *is_exist = pldm_pdr_is_exist(&(g_pldm_monitor_info.pldm_repo), record_handle);
        if (is_exist) {
            goto L_OUT;
        }

        composite_state_sensor_pdr = composite_state_pdr_create_sensor(&composite_state_sensor, datastruct->sensor_id + i, subsensor_count);
        if (!composite_state_sensor_pdr) return;

        for (int j = 0; j < subsensor_count; j++) {
            add_sensor.state_setid = datastruct[i * subsensor_count * is_plug_sensor + j].sensor_type;
            add_sensor.possible_states_size = 1;
            add_sensor.possible_states = possible_states_generate(add_sensor.state_setid);
            composite_state_sensor_pdr = composite_state_pdr_add_sensor(composite_state_sensor_pdr, &add_sensor);
        }
        int ret = pldm_pdr_add(&(g_pldm_monitor_info.pldm_repo), (u8 *)composite_state_sensor_pdr, sizeof(pldm_composite_state_sensor_pdr_t) + \
        subsensor_count * sizeof(pldm_composite_sensor_attr_t), composite_state_sensor_pdr->hdr.record_handle);
        if (ret == 0)
            pldm_pdr_chg_event_generate(g_pldm_monitor_info.pldm_event_rbuf, PLDM_REPO_CHG_FORMAT_ID_PDR_HANDLE, PLDM_REPO_CHG_RECORDS_ADDED, record_handle);
L_OUT:
        pldm_sensor_event_generate(g_pldm_monitor_info.pldm_event_rbuf, datastruct->sensor_event_class, datastruct->event_msg_en, datastruct);
    }
}

static void pldm_add_numeric_sensor_pdr(u8 data_size, pldm_data_struct_t *sensor_datastruct, fill_common_sensor_pdr fill_common_func)
{
    if (!sensor_datastruct) return;
    u32 record_handle = PLDM_ERR_RECORD_HANDLE;

    if (sensor_datastruct->op_state == PLDM_OP_ENABLE) {
        u16 sensor_id = sensor_datastruct->sensor_id;

        record_handle = sensor_id_convert_to_record_handle(sensor_id);
        if (record_handle == PLDM_ERR_RECORD_HANDLE) return;
        pldm_pdr_record_t *is_exist = pldm_pdr_is_exist(&(g_pldm_monitor_info.pldm_repo), record_handle);
        void *buf = NULL;
        if (is_exist) {
            buf = is_exist->data;
        } else {
            buf = pdr_malloc(data_size);
        }

        if (!buf) {
            LOG("no more space for malloc!, %s", __FUNCTION__);
            return;
        }

        if (fill_common_func)
            fill_common_func(buf, sensor_datastruct->sensor_id, sensor_datastruct->entity_type, sensor_datastruct->sensor_data_size);

        if (is_temp_sensor(sensor_id) != -1) {
            temp_sensor_monitor_handle(sensor_id, (u32 *)buf);
        }

        if (!is_exist) {
            int ret = pldm_pdr_add(&(g_pldm_monitor_info.pldm_repo), (u8 *)buf, data_size, record_handle);
            if (ret == 0)
                pldm_pdr_chg_event_generate(g_pldm_monitor_info.pldm_event_rbuf, PLDM_REPO_CHG_FORMAT_ID_PDR_HANDLE, PLDM_REPO_CHG_RECORDS_ADDED, record_handle);
        }
        pldm_sensor_event_generate(g_pldm_monitor_info.pldm_event_rbuf, sensor_datastruct->sensor_event_class, sensor_datastruct->event_msg_en, \
        sensor_datastruct);

    }
}

static void pldm_delete_numeric_sensor_pdr(pldm_data_struct_t *sensor_datastruct)
{
    if (!sensor_datastruct) return;
    u32 record_handle = PLDM_ERR_RECORD_HANDLE;
    if (sensor_datastruct->op_state == PLDM_OP_UNAVAILABLE) {

        u16 sensor_id = sensor_datastruct->sensor_id;
        record_handle = sensor_id_convert_to_record_handle(sensor_id);
        if (record_handle == PLDM_ERR_RECORD_HANDLE) return;
        int ret = pldm_pdr_delete(&(g_pldm_monitor_info.pldm_repo), record_handle);
        if (ret != 0) {
            LOG("ERR SENSOR ID : %d", sensor_id);
            return;
        }

        if (is_temp_sensor(sensor_id) != -1) {
            temp_sensor_monitor_handle(sensor_id, NULL);
        }

        pldm_sensor_event_generate(g_pldm_monitor_info.pldm_event_rbuf, sensor_datastruct->sensor_event_class, sensor_datastruct->event_msg_en, \
        sensor_datastruct);
        pldm_pdr_chg_event_generate(g_pldm_monitor_info.pldm_event_rbuf, PLDM_REPO_CHG_FORMAT_ID_PDR_HANDLE, PLDM_REPO_CHG_RECORDS_DELETED, record_handle);
    }
}

static void pldm_add_assoc_pdr(pldm_entity_t *container, pldm_entity_t *contained, u32 record_handle, u16 container_id, u8 contained_cnt, u8 assoc_type)
{
    if (!container || !contained) return;
    pldm_pdr_entity_assoc_t *assoc_pdr = NULL;

    pldm_pdr_record_t *is_exist = pldm_pdr_is_exist(&(g_pldm_monitor_info.pldm_repo), record_handle);
    if (is_exist) return;

    assoc_pdr = assoc_pdr_create_container(container, record_handle, container_id, contained_cnt, assoc_type);
    if (!assoc_pdr) return;

    for (int i = 0; i < contained_cnt; i++) {
        assoc_pdr = assoc_pdr_add_contained(assoc_pdr, &contained[i], i);
    }
    assoc_pdr->hdr.record_change_num = 0;
    int ret = pldm_pdr_add(&(g_pldm_monitor_info.pldm_repo), (u8 *)assoc_pdr, sizeof(pldm_pdr_entity_assoc_t) + contained_cnt * sizeof(pldm_entity_t), assoc_pdr->hdr.record_handle);
    if (ret == 0)
        pldm_pdr_chg_event_generate(g_pldm_monitor_info.pldm_event_rbuf, PLDM_REPO_CHG_FORMAT_ID_PDR_HANDLE, PLDM_REPO_CHG_RECORDS_ADDED, assoc_pdr->hdr.record_handle);
}

static void pldm_delete_assoc_pdr(u32 assoc_record_handle)
{
    int ret = -1;
    ret = pldm_pdr_delete(&(g_pldm_monitor_info.pldm_repo), assoc_record_handle);
    if (ret != 0) {
        return;
    }
    pldm_pdr_chg_event_generate(g_pldm_monitor_info.pldm_event_rbuf, PLDM_REPO_CHG_FORMAT_ID_PDR_HANDLE, PLDM_REPO_CHG_RECORDS_DELETED, assoc_record_handle);
}

void pldm_modify_state_datastruct(u8 present_state, pldm_state_data_struct_t *datastruct)
{
    if (!datastruct || datastruct->op_state != PLDM_OP_ENABLE) return;
    datastruct->previous_state = datastruct->present_state;
    datastruct->present_state = present_state;
    if (datastruct->previous_state != present_state) {
        pldm_sensor_event_generate(g_pldm_monitor_info.pldm_event_rbuf, datastruct->sensor_event_class, datastruct->event_msg_en, datastruct);
    }
}

#define PLDM_REDFISH_RESOURCE_PDR_BASE_LEN   (sizeof(pldm_redfish_resource_pdr_first_part_t) + sizeof(pldm_redfish_resource_pdr_middle0_part_t) + \
    sizeof(pldm_redfish_resource_pdr_middle1_part_t) + sizeof(pldm_redfish_resource_pdr_middle2_part_t) + sizeof(pldm_redfish_resource_pdr_end_part_t))

static u8 *pldm_redfish_resource_pdr_fill_first_part(u8 *first_part_pdr, u8 malloc_len, u32 record_handle, u32 resource_id, u8 resource_flg, u32 containing_resource_id, char *proposed_containing_resource_name)
{
    if (!first_part_pdr || !proposed_containing_resource_name) return NULL;
    pldm_redfish_resource_pdr_first_part_t *first_part = (pldm_redfish_resource_pdr_first_part_t *)first_part_pdr;
    u8 proposed_containing_resource_name_len = cm_strlen(proposed_containing_resource_name) + 1;
    first_part->hdr.record_handle = record_handle;
    first_part->hdr.version = PLDM_EVENT_FORMAT_VERSION;
    first_part->hdr.type = REDFISH_RESOURCE_PDR;
    // first_part->hdr.record_change_num = 0;
    first_part->hdr.length = malloc_len - sizeof(pldm_pdr_hdr_t);

    first_part->resource_id = resource_id;
    /* bit0 : is_device_root
     * bit1 : is_contained_in_collection
     * bit2 : is_collection
     */
    first_part->resource_flg = resource_flg;
    first_part->containing_resource_id = containing_resource_id;
    first_part->proposed_containing_resource_byte_len = proposed_containing_resource_name_len;
    cm_memcpy(first_part->proposed_containing_resource_name, proposed_containing_resource_name, proposed_containing_resource_name_len);
    u8 *next_part = (u8 *)&(first_part->proposed_containing_resource_name[proposed_containing_resource_name_len]);
    return next_part;
}

static u8 *pldm_redfish_resource_pdr_fill_middle0_part(u8 *middle0_part_pdr, char *sub_uri)
{
    if (!middle0_part_pdr || !sub_uri) return NULL;
    pldm_redfish_resource_pdr_middle0_part_t *middle0_part = (pldm_redfish_resource_pdr_middle0_part_t *)middle0_part_pdr;
    middle0_part->suburi_byte_len = cm_strlen(sub_uri) + 1;
    cm_memcpy(middle0_part->suburi, sub_uri, middle0_part->suburi_byte_len);
    u8 *next_part = (u8 *)&(middle0_part->suburi[middle0_part->suburi_byte_len]);
    return next_part;
}

static u8 *pldm_redfish_resource_pdr_fill_middle1_part(u8 *middle1_part_pdr, u16 add_resource_id_cnt, void *add_info, void *add_suburi)
{
    if (!middle1_part_pdr) return NULL;
    pldm_redfish_add_info_t *resource_info = (pldm_redfish_add_info_t *)add_info;
    pldm_redfish_resource_pdr_middle1_part_t *middle1_part = (pldm_redfish_resource_pdr_middle1_part_t *)middle1_part_pdr;
    middle1_part->add_resource_id_cnt = add_resource_id_cnt;
    pldm_redfish_add_resource_info_t *ptr = (pldm_redfish_add_resource_info_t *)(middle1_part->add_resource_info);
    u8 offset = sizeof(pldm_redfish_resource_pdr_middle1_part_t);
    char *suburi_ptr = (char *)add_suburi;
    for (u16 i = 0; i < add_resource_id_cnt; i++) {
        ptr->add_resource_id = resource_info[i].add_resource_id;
        ptr->add_suburi_byte_len = resource_info[i].add_suburi_byte_len;
        cm_memcpy(ptr->add_suburi, suburi_ptr, ptr->add_suburi_byte_len);
        suburi_ptr += ptr->add_suburi_byte_len;
        offset += (sizeof(pldm_redfish_add_info_t) + ptr->add_suburi_byte_len);
        ptr = (pldm_redfish_add_resource_info_t *)&(ptr->add_suburi[ptr->add_suburi_byte_len]);
    }

    u8 *next_part = middle1_part_pdr + offset;
    return next_part;
}

static u8 *pldm_redfish_resource_pdr_fill_middle2_part(u8 *middle2_part_pdr, u32 resource_id, u8 requested_schemaclass, char *major_schema_name)
{
    if (!middle2_part_pdr || !major_schema_name) return NULL;
    pldm_redfish_resource_pdr_middle2_part_t *middle2_part = (pldm_redfish_resource_pdr_middle2_part_t *)middle2_part_pdr;
    u8 major_schema_name_len = cm_strlen(major_schema_name) + 1;
    u8 dict[ALIGN(sizeof(pldm_redfish_dict_fmt_t) + sizeof(pldm_redfish_dictionary_format_t), 4)];
    u8 ret = pldm_redfish_get_dict_data(resource_id, requested_schemaclass, dict, sizeof(dict));
    if (ret == true) {
        pldm_redfish_dict_fmt_t *dict_fmt = (pldm_redfish_dict_fmt_t *)dict;
        pldm_redfish_dictionary_format_t *dict_ptr = (pldm_redfish_dictionary_format_t *)dict_fmt->data;
        middle2_part->major_schema_dict_byte_len = dict_ptr->dictionary_size;
        middle2_part->major_schema_dict_sign = dict_fmt->dict_sign;
        middle2_part->major_schema_ver = dict_ptr->schema_version;
    } else {
        LOG("can not found need dict");
    }

    // middle2_part->major_schema_dict_byte_len = ;
    // middle2_part->major_schema_dict_sign = ;
    middle2_part->major_schema_name_len = major_schema_name_len;
    cm_memcpy(middle2_part->major_schema_name, major_schema_name, major_schema_name_len);

    u8 *next_part = (u8 *)&(middle2_part->major_schema_name[major_schema_name_len]);
    return next_part;
}

static void pldm_redfish_resource_pdr_fill_end_part(u8 *end_part_pdr, u16 oem_cnt)
{
    if (!end_part_pdr) return;
    pldm_redfish_resource_pdr_end_part_t *end_part = (pldm_redfish_resource_pdr_end_part_t *)end_part_pdr;
    end_part->oem_cnt = oem_cnt;    /* currently not support oem. */
}

static void pldm_redfish_resource_pdr_fill(u8 *buf, u8 malloc_len, u32 record_handle, u32 resource_id, u8 resource_flg, u32 containing_resource_id, u16 add_resource_id_cnt, void *add_info, void *add_suburi)
{
    u8 *next_part = pldm_redfish_resource_pdr_fill_first_part(buf, malloc_len, record_handle, resource_id, resource_flg, containing_resource_id, proposed_containing_resource_name);
    next_part = pldm_redfish_resource_pdr_fill_middle0_part(next_part, sub_uri);
    next_part = pldm_redfish_resource_pdr_fill_middle1_part(next_part, add_resource_id_cnt, add_info, add_suburi);
    next_part = pldm_redfish_resource_pdr_fill_middle2_part(next_part, resource_id, SCHEMACLASS_MAJOR, major_schema_name);
    pldm_redfish_resource_pdr_fill_end_part(next_part, 0);
    if (!next_part)
        return ;
    pldm_pdr_add(&(g_pldm_monitor_info.pldm_repo), buf, malloc_len, record_handle);
}

static u8 pldm_redfish_pdr_malloc_size_get(void)
{
    u8 malloc_len = PLDM_REDFISH_RESOURCE_PDR_BASE_LEN;
    malloc_len += cm_strlen(proposed_containing_resource_name) + sizeof(u8);
    malloc_len += cm_strlen(sub_uri) + sizeof(u8);
    malloc_len += cm_strlen(major_schema_name) + sizeof(u8);

    return malloc_len;
}

static int pldm_redfish_pdr_add(u32 record_handle, u32 resource_id, u8 resource_flg, u32 containing_resource_id)
{
    pldm_pdr_record_t *is_exist = pldm_pdr_is_exist(&(g_pldm_monitor_info.pldm_repo), record_handle);
    if (is_exist)
        return -1;

    u8 malloc_len = pldm_redfish_pdr_malloc_size_get();
    u8 *buf = (u8 *)pdr_malloc(malloc_len);
    if (!buf) {
        return -1;
    }
    pldm_redfish_resource_pdr_fill(buf, malloc_len, record_handle, resource_id, resource_flg, containing_resource_id, 0, NULL, NULL);

    return 0;
}

static void pldm_redfish_add_network_adapter_pdr(void)
{
    proposed_containing_resource_name = "NetworkAdapterCollection.NetworkAdapterCollection";
    sub_uri = "";
    major_schema_name = "NetworkAdapter.NetworkAdapter";
    pldm_redfish_pdr_add(PLDM_NETWORK_ADAPTER_PDR_HANDLE, PLDM_BASE_NETWORK_ADAPTER_RESOURCE_ID, CBIT(0), PLDM_REDFISH_EXTERNAL);
}

static void pldm_redfish_add_network_interface_pdr(void)
{
    proposed_containing_resource_name = "NetworkInterfaceCollection.NetworkInterfaceCollection";
    sub_uri = "";
    major_schema_name = "NetworkInterface.NetworkInterface";
    pldm_redfish_pdr_add(PLDM_NETWORK_INTERFACE_PDR_HANDLE, PLDM_BASE_NETWORK_INTERFACE_RESOURCE_ID, CBIT(0), PLDM_REDFISH_EXTERNAL);
}

static void pldm_redfish_add_ports_pdr(void)
{
    proposed_containing_resource_name = "";
    sub_uri = "Ports";
    major_schema_name = "PortCollection.PortCollection";
    pldm_redfish_pdr_add(PLDM_PORTS_PDR_HANDLE, PLDM_BASE_PORTS_RESOURCE_ID, CBIT(2), PLDM_REDFISH_EXTERNAL);
}

static void pldm_redfish_add_network_dev_funcs_pdr(void)
{
    proposed_containing_resource_name = "";
    sub_uri = "NetworkDeviceFunctions";
    major_schema_name = "NetworkDeviceFunctionCollection.NetworkDeviceFunctionCollection";
    pldm_redfish_pdr_add(PLDM_NETWORK_DEV_FUNCS_PDR_HANDLE, PLDM_BASE_NETWORK_DEV_FUNCS_RESOURCE_ID, CBIT(2), PLDM_BASE_NETWORK_ADAPTER_RESOURCE_ID);
}

static void pldm_redfish_add_port_pdr(void)
{
    pldm_pdr_record_t *is_exist = pldm_pdr_is_exist(&(g_pldm_monitor_info.pldm_repo), PLDM_PORT_PDR_HANDLE);
    if (is_exist) return;

    proposed_containing_resource_name = "";
    sub_uri = "0";
    major_schema_name = "Port.Port";

    pldm_redfish_add_info_t add_info[MAX_LAN_NUM];
    char add_suburi[MAX_LAN_NUM][2];
    u8 malloc_len = pldm_redfish_pdr_malloc_size_get() + sizeof(add_suburi) + sizeof(add_info);
    u8 *buf = (u8 *)pdr_malloc(malloc_len);
    if (!buf) {
        return;
    }
    char *num = "1234";
    cm_memset(add_suburi, '\0', sizeof(add_suburi));
    for (u8 i = 0; i < MAX_LAN_NUM; i++) {
        cm_memcpy(add_suburi[i], &num[i], 1);
        add_info[i].add_resource_id = PLDM_BASE_PORT_RESOURCE_ID + i;
        add_info[i].add_suburi_byte_len = cm_strlen(add_suburi[i]) + 1;
    }

    /* no ContainingResourceID ? E810 */
    pldm_redfish_resource_pdr_fill(buf, malloc_len, PLDM_PORT_PDR_HANDLE, PLDM_BASE_PORT_RESOURCE_ID, CBIT(1), PLDM_REDFISH_EXTERNAL, MAX_LAN_NUM, (void *)add_info, (void *)add_suburi);
}

static void pldm_redfish_add_network_dev_func_pdr(void)
{
    pldm_pdr_record_t *is_exist = pldm_pdr_is_exist(&(g_pldm_monitor_info.pldm_repo), PLDM_NETWORK_DEV_FUNC_PDR_HANDLE);
    if (is_exist) return;

    proposed_containing_resource_name = "";
    sub_uri = "0";
    major_schema_name = "NetworkDeviceFunction.NetworkDeviceFunction";
    pldm_redfish_add_info_t add_info[2 * MAX_LAN_NUM];
    char add_suburi[2 * MAX_LAN_NUM][11];

    u8 malloc_len = pldm_redfish_pdr_malloc_size_get() + sizeof(add_suburi) + sizeof(add_info);
    u8 *buf = (u8 *)pdr_malloc(malloc_len);
    if (!buf) {
        return;
    }

    char *add_suburi_suffix = "/Settings";
    char *num = "1234";
    cm_memset(add_suburi, '\0', sizeof(add_suburi));
    for (u8 i = 0; i < MAX_LAN_NUM; i++) {
        cm_memcpy(add_suburi[i], &num[i], 1);
        add_info[i].add_resource_id = PLDM_BASE_NETWORK_DEV_FUNC_RESOURCE_ID + i;
        add_info[i].add_suburi_byte_len = cm_strlen(add_suburi[i]) + 1;
        cm_memcpy(add_suburi[MAX_LAN_NUM + i], &num[i], 1);
        cm_memcpy(&add_suburi[MAX_LAN_NUM + i][1], add_suburi_suffix, cm_strlen(add_suburi_suffix));
        add_info[MAX_LAN_NUM + i].add_resource_id = PLDM_BASE_NETWORK_DEV_FUNC_RESOURCE_ID_1 + i;
        add_info[MAX_LAN_NUM + i].add_suburi_byte_len = cm_strlen(add_suburi[MAX_LAN_NUM + i]) + 1;
    }

    pldm_redfish_resource_pdr_fill(buf, malloc_len, PLDM_NETWORK_DEV_FUNC_PDR_HANDLE, PLDM_BASE_NETWORK_DEV_FUNC_RESOURCE_ID, CBIT(1), PLDM_BASE_NETWORK_DEV_FUNCS_RESOURCE_ID, 2 * MAX_LAN_NUM, (void *)add_info, (void *)add_suburi);
}

static void pldm_redfish_add_pcie_funcs_pdr(void)
{
    proposed_containing_resource_name = "";
    sub_uri = "NetworkDeviceFunctions";
    major_schema_name = "NetworkDeviceFunctionCollection.NetworkDeviceFunctionCollection";
    pldm_redfish_pdr_add(PLDM_PCIE_FUNCS_PDR_HANDLE, PLDM_BASE_PCIE_FUNCS_RESOURCE_ID, CBIT(2), PLDM_BASE_NETWORK_ADAPTER_RESOURCE_ID);
}

static void pldm_redfish_add_pcie_func_pdr(void)
{
    pldm_pdr_record_t *is_exist = pldm_pdr_is_exist(&(g_pldm_monitor_info.pldm_repo), PLDM_PCIE_FUNC_PDR_HANDLE);
    if (is_exist) return;

    proposed_containing_resource_name = "";
    sub_uri = "/0";
    major_schema_name = "PCIeFunction.PCIeFunction";
    pldm_redfish_add_info_t add_info[MAX_LAN_NUM];
    char add_suburi[MAX_LAN_NUM][3];

    u8 malloc_len = pldm_redfish_pdr_malloc_size_get() + sizeof(add_suburi) + sizeof(add_info);
    u8 *buf = (u8 *)pdr_malloc(malloc_len);
    if (!buf) {
        return;
    }

    char *add_suburi_prefix = "/";
    char *num = "1234";
    cm_memset(add_suburi, '\0', sizeof(add_suburi));
    for (u8 i = 0; i < MAX_LAN_NUM; i++) {
        cm_memcpy(&add_suburi[i][0], add_suburi_prefix, cm_strlen(add_suburi_prefix));
        cm_memcpy(&add_suburi[i][1], &num[i], 1);
        add_info[i].add_resource_id = PLDM_BASE_PCIE_FUNC_RESOURCE_ID + i;
        add_info[i].add_suburi_byte_len = cm_strlen(add_suburi[i]) + 1;
    }

    pldm_redfish_resource_pdr_fill(buf, malloc_len, PLDM_PCIE_FUNC_PDR_HANDLE, PLDM_BASE_PCIE_FUNC_RESOURCE_ID, CBIT(1), PLDM_BASE_PCIE_FUNCS_RESOURCE_ID, MAX_LAN_NUM, (void *)add_info, (void *)add_suburi);
}

static void pldm_redfish_add_eth_interface_collection_pdr(void)
{
    proposed_containing_resource_name = "System";
    sub_uri = "";
    major_schema_name = "EthernetInterfaceCollection.EthernetInterfaceCollection";
    pldm_redfish_pdr_add(PLDM_ETH_INTERFACE_COLLECTION_PDR_HANDLE, PLDM_BASE_ETH_INTERFACE_COLLECTION_RESOURCE_ID, CBIT(2) | CBIT(0), PLDM_REDFISH_EXTERNAL);
}

static void pldm_redfish_add_eth_interface_pdr(void)
{
    pldm_pdr_record_t *is_exist = pldm_pdr_is_exist(&(g_pldm_monitor_info.pldm_repo), PLDM_ETH_INTERFACE_PDR_HANDLE);
    if (is_exist) return;

    proposed_containing_resource_name = "";
    sub_uri = "0";
    major_schema_name = "EthernetInterface.EthernetInterface";
    pldm_redfish_add_info_t add_info[2 * MAX_LAN_NUM] = {0};
    char add_suburi[2 * MAX_LAN_NUM][11];

    u8 malloc_len = pldm_redfish_pdr_malloc_size_get();
    u8 *buf = (u8 *)pdr_malloc(malloc_len);
    if (!buf) {
        return;
    }

    char *add_suburi_suffix = "/Settings";
    char *num = "1234";
    cm_memset(add_suburi, '\0', sizeof(add_suburi));
    for (u8 i = 0; i < MAX_LAN_NUM; i++) {
        cm_memcpy(add_suburi[i], &num[i], 1);
        add_info[i].add_resource_id = PLDM_BASE_ETH_INTERFACE_RESOURCE_ID + i;
        add_info[i].add_suburi_byte_len = cm_strlen(add_suburi[i]) + 1;
        cm_memcpy(add_suburi[MAX_LAN_NUM + i], &num[i], 1);
        cm_memcpy(&add_suburi[MAX_LAN_NUM + i][1], add_suburi_suffix, cm_strlen(add_suburi_suffix));
        add_info[MAX_LAN_NUM + i].add_resource_id = PLDM_BASE_ETH_INTERFACE_RESOURCE_ID_1 + i;
        add_info[MAX_LAN_NUM + i].add_suburi_byte_len = cm_strlen(add_suburi[MAX_LAN_NUM + i]) + 1;
    }

    pldm_redfish_resource_pdr_fill(buf, malloc_len, PLDM_ETH_INTERFACE_PDR_HANDLE, PLDM_BASE_ETH_INTERFACE_RESOURCE_ID, CBIT(1), PLDM_BASE_ETH_INTERFACE_COLLECTION_RESOURCE_ID, 2 * MAX_LAN_NUM, (void *)add_info, (void *)add_suburi);
}

#define PLDM_REDFISH_ACTION_PDR_BASE_LEN (sizeof(pldm_redfish_action_pdr_first_part_t) + sizeof(pldm_redfish_action_pdr_end_part_t))

static u8 *pldm_redfish_action_pdr_fill_first_part(u8 *first_part_pdr, u8 malloc_len, u32 record_handle, u32 resource_id, u8 action_pdr_idx, u16 related_resource_cnt, u32 *related_resource_id)
{
    if (!first_part_pdr || !related_resource_id) return NULL;
    pldm_redfish_action_pdr_first_part_t *first_part = (pldm_redfish_action_pdr_first_part_t *)first_part_pdr;
    first_part->hdr.record_handle = record_handle;
    first_part->hdr.version = PLDM_EVENT_FORMAT_VERSION;
    first_part->hdr.type = REDFISH_ACTION_PDR;
    // first_part->hdr.record_change_num = 0;
    first_part->hdr.length = malloc_len - sizeof(pldm_pdr_hdr_t);

    first_part->action_pdr_idx = action_pdr_idx;
    first_part->related_resource_cnt = related_resource_cnt;

    for (u8 i = 0; i < related_resource_cnt; i++) {
        first_part->related_resource_id[i] = related_resource_id[i];
    }
    u8 *next_part = (u8 *)&(first_part->related_resource_id[first_part->related_resource_cnt]);
    return next_part;
}

static void pldm_redfish_action_pdr_fill_end_part(u8 *end_part_pdr, u8 action_cnt, char *action_name, char *action_path)
{
    if (!end_part_pdr || !action_name || !action_path) return;
    pldm_redfish_action_pdr_end_part_t *end_part = (pldm_redfish_action_pdr_end_part_t *)end_part_pdr;
    end_part->action_cnt = action_cnt;
    pldm_redfish_action_info_t *ptr = (pldm_redfish_action_info_t *)(end_part->action_info);
    char *name_ptr = action_name;
    char *path_ptr = action_path;
    for (u8 i = 0; i < action_cnt; i++) {
        for (u8 j = 0; j < 2; j++) {
            char *action_str = (j % 2) ? path_ptr : name_ptr;
            u8 action_len = cm_strlen(action_str) + 1;
            ptr->action_byte_len = action_len;
            cm_memcpy(ptr->action_str, action_str, ptr->action_byte_len);
            ptr = (pldm_redfish_action_info_t *)((u8 *)ptr + ptr->action_byte_len);
            if (j)
                path_ptr += action_len;
            else 
                name_ptr += action_len;
        }
    }
}

static void pldm_redfish_add_reset_settings_to_default_pdr(void)
{
    pldm_pdr_record_t *is_exist = pldm_pdr_is_exist(&(g_pldm_monitor_info.pldm_repo), PLDM_RESET_SET2DEFAULE_PDR_HANDLE);
    if (is_exist) return;

    char *action_name = "ResetSettingsToDefault";
    char *action_path = "";
    u8 action_name_len = cm_strlen(action_name) + 1;
    u8 action_path_len = cm_strlen(action_path) + 1;
    u32 related_resource_id = PLDM_BASE_RESET_SET2DEFAULE_RESOURCE_ID;

    u8 malloc_len = PLDM_REDFISH_ACTION_PDR_BASE_LEN + action_name_len + action_path_len + sizeof(related_resource_id);
    u8 *buf = (u8 *)pdr_malloc(malloc_len);
    if (!buf) {
        return;
    }

    u8 *next_part = pldm_redfish_action_pdr_fill_first_part(buf, malloc_len, PLDM_RESET_SET2DEFAULE_PDR_HANDLE, PLDM_BASE_RESET_SET2DEFAULE_RESOURCE_ID, \
    0, 1, &related_resource_id);
    pldm_redfish_action_pdr_fill_end_part(next_part, 1, action_name, action_path);
    pldm_pdr_add(&(g_pldm_monitor_info.pldm_repo), buf, malloc_len, PLDM_RESET_SET2DEFAULE_PDR_HANDLE);
}

static void pldm_redfish_add_port_reset_pdr(void)
{
    pldm_pdr_record_t *is_exist = pldm_pdr_is_exist(&(g_pldm_monitor_info.pldm_repo), PLDM_PORT_RESET_PDR_HANDLE);
    if (is_exist) return;

    char *action_name = "#Port.Reset";
    char *action_path = "";
    u8 action_name_len = cm_strlen(action_name) + 1;
    u8 action_path_len = cm_strlen(action_path) + 1;
    u32 related_resource_id[MAX_LAN_NUM];
    for (u8 i = 0; i < MAX_LAN_NUM; i++) {
        related_resource_id[i] = PLDM_BASE_PORT_RESOURCE_ID + i;
    }

    u8 malloc_len = PLDM_REDFISH_ACTION_PDR_BASE_LEN + action_name_len + action_path_len + sizeof(related_resource_id);
    u8 *buf = (u8 *)pdr_malloc(malloc_len);
    if (!buf) {
        return;
    }

    u8 *next_part = pldm_redfish_action_pdr_fill_first_part(buf, malloc_len, PLDM_PORT_RESET_PDR_HANDLE, PLDM_BASE_PORT_RESET_RESOURCE_ID, \
    0, MAX_LAN_NUM, related_resource_id);
    pldm_redfish_action_pdr_fill_end_part(next_part, 1, action_name, action_path);
    pldm_pdr_add(&(g_pldm_monitor_info.pldm_repo), buf, malloc_len, PLDM_PORT_RESET_PDR_HANDLE);
}

static int pldm_fru_add_fru_record_set_pdr(pldm_fru_record_set_pdr_t *fru_record_set_pdr)
{
    fru_record_set_pdr->hdr.record_handle = PLDM_FRU_RECORD_SET_PDR_HANDLE;
    fru_record_set_pdr->hdr.version = PLDM_EVENT_FORMAT_VERSION;
    fru_record_set_pdr->hdr.type = FRU_RECORD_SET_PDR;
    fru_record_set_pdr->hdr.record_change_num = 0;
    fru_record_set_pdr->hdr.length = sizeof(pldm_fru_record_set_pdr_t) - sizeof(pldm_pdr_hdr_t);

    pldm_pdr_record_t *is_find = pldm_pdr_find(&(g_pldm_monitor_info.pldm_repo), PLDM_BASE_NIC_ASSOC_PDR_HANDLE);
    if (!is_find) return FAILURE;

    pldm_pdr_entity_assoc_t *assoc_pdr = (pldm_pdr_entity_assoc_t *)(is_find->data);

    fru_record_set_pdr->pldm_terminus_handle = PLDM_TERMINUS_HANDLE;
    fru_record_set_pdr->fru_record_terminus_identifier = FRU_GENERAL_SET_ID;
    fru_record_set_pdr->entity_type = assoc_pdr->container.entity_type;
    fru_record_set_pdr->entity_instance_num = assoc_pdr->container.entity_instance_num;
    fru_record_set_pdr->container_id = assoc_pdr->container_id;
    return SUCCESS;
}

void pldm_fru_pdr_init(void)
{
    pldm_pdr_record_t *is_exist = pldm_pdr_is_exist(&(g_pldm_monitor_info.pldm_repo), PLDM_FRU_RECORD_SET_PDR_HANDLE);
    if (is_exist) return;
    pldm_fru_record_set_pdr_t *fru_record_set_pdr = (pldm_fru_record_set_pdr_t *)pdr_malloc(sizeof(pldm_fru_record_set_pdr_t));
    if (!fru_record_set_pdr) {
        LOG("no more space for malloc!, %s", __FUNCTION__);
        return;
    }

    int ret = pldm_fru_add_fru_record_set_pdr(fru_record_set_pdr);
    if (ret == FAILURE) {
        LOG("pldm_fru_add_fru_record_set_pdr err, %s", __FUNCTION__);
        return;
    }
    pldm_pdr_add(&(g_pldm_monitor_info.pldm_repo), (u8 *)fru_record_set_pdr, sizeof(pldm_fru_record_set_pdr_t), fru_record_set_pdr->hdr.record_handle);
}

void pldm_redfish_pdr_init(void)
{
    pldm_redfish_add_network_adapter_pdr();
    pldm_redfish_add_network_interface_pdr();
    pldm_redfish_add_ports_pdr();
    pldm_redfish_add_network_dev_funcs_pdr();
    pldm_redfish_add_port_pdr();
    pldm_redfish_add_network_dev_func_pdr();
    pldm_redfish_add_pcie_funcs_pdr();
    pldm_redfish_add_pcie_func_pdr();
    pldm_redfish_add_eth_interface_collection_pdr();
    pldm_redfish_add_eth_interface_pdr();
    pldm_redfish_add_reset_settings_to_default_pdr();
    pldm_redfish_add_port_reset_pdr();
}

void pldm_terminus_locator_pdr_init(void)
{
    pldm_pdr_record_t *is_exist = pldm_pdr_is_exist(&(g_pldm_monitor_info.pldm_repo), PLDM_TERMINUS_LOCATOR_PDR_HANDLE);
    if (is_exist) return;

	pldm_terminus_locator_pdr_t *terminus_locator_pdr = (pldm_terminus_locator_pdr_t *)pdr_malloc(sizeof(pldm_terminus_locator_pdr_t));
    if (!terminus_locator_pdr) {
        LOG("no more space for malloc!, %s", __FUNCTION__);
        return;
    }

    fill_terminus_locator_pdr(terminus_locator_pdr);
    pldm_pdr_add(&(g_pldm_monitor_info.pldm_repo), (u8 *)terminus_locator_pdr, sizeof(pldm_terminus_locator_pdr_t), terminus_locator_pdr->hdr.record_handle);
}

void pldm_state_sensor_pdr_init(void)
{
    u8 subsensor_count = 0;

    /* NIC Composite State Sensor PDR */

    subsensor_count = sizeof(nic_composite_state_sensors) / sizeof(nic_composite_state_sensors[0]);

    pldm_add_state_sensor_pdr(subsensor_count, PLDM_CARD_COMPOSITE_STATE_SENSOR_NUM, NIC_SENSOR, PLDM_SYSTEM, nic_composite_state_sensors, 0);

    /* Controller Composite State Sensor PDR */

    subsensor_count = sizeof(controller_composite_state_sensors) / sizeof(controller_composite_state_sensors[0]);

    pldm_add_state_sensor_pdr(subsensor_count, PLDM_NC_STATE_SENSOR_NUM, NIC_SENSOR, PLDM_BASE_NIC_CONTAINER_ID, controller_composite_state_sensors, 0);

    /* Plug Composite State Sensor PDR */

    subsensor_count = sizeof(plug_composite_state_sensors) / sizeof(plug_composite_state_sensors[0]) / 4;
    /* PLDM_BASE_CONNECTOR_CONTAINER_ID E810 ? */
    pldm_add_state_sensor_pdr(subsensor_count, PLDM_USED_PLUG_COMPOSITE_SENSOR_NUM, PORT, PLDM_BASE_PLUG_CONTAINER_ID, plug_composite_state_sensors, 1);

    /* Simple State Sensors PDRs */

    subsensor_count = sizeof(simple_sensors) / sizeof(simple_sensors[0]) / 4;

    pldm_add_state_sensor_pdr(subsensor_count, PLDM_USED_LINK_STATE_SENSOR_NUM, COMMUNICATION_PORT, PLDM_BASE_NC_CONTAINER_ID, simple_sensors, 1);
}

void pldm_numeric_sensor_pdr_init(void)
{
    /* temp sensor */

    for (int i = 0; i < PLDM_NIC_TEMP_SENSOR_NUM; i++) {
        pldm_add_numeric_sensor_pdr(sizeof(pldm_numeric_sensor_pdr_t), &temp_sensors[NIC_TEMP_SENSOR][i], fill_common_thermal_sensor_pdr);
    }

    for (int i = 0; i < PLDM_NC_TEMP_SENSOR_NUM; i++) {
        pldm_add_numeric_sensor_pdr(sizeof(pldm_numeric_sensor_pdr_t), &temp_sensors[NC_TEMP_SENSOR][i], fill_common_thermal_sensor_pdr);
    }
}

void pldm_assoc_pdr_init(void)
{
    pldm_entity_t container, contained[MAX_LAN_NUM];

    /* NIC assoc PDR */

    container.entity_type = NIC_SENSOR;
    container.entity_instance_num = 1;
    container.entity_container_id = PLDM_SYSTEM;

    contained[0].entity_type = CONTROLLER_SENSOR;
    contained[0].entity_instance_num = 1;

    contained[1].entity_type = CONNECTOR;
    contained[1].entity_instance_num = 1;

    pldm_add_assoc_pdr(&container, contained, PLDM_BASE_NIC_ASSOC_PDR_HANDLE, PLDM_BASE_NIC_CONTAINER_ID, PLDM_CONNECTOR_NUM + PLDM_NC_NUM, PLDM_ENTITY_ASSOCIAION_PHYSICAL);

    /* Controller assoc PDR */

    for (int i = 0; i < PLDM_NC_NUM; i++) {
        container.entity_type = CONTROLLER_SENSOR;
        container.entity_instance_num = i + 1;
        container.entity_container_id = PLDM_BASE_NIC_CONTAINER_ID;
        for (int j = 0; j < PLDM_USED_COMM_CHANNEL_NUM; j++) {
            contained[j].entity_type = COMMUNICATION_PORT;
            contained[j].entity_instance_num = j + 1;
        }
        pldm_add_assoc_pdr(&container, contained, PLDM_BASE_NC_ASSOC_PDR_HANDLE + i, PLDM_BASE_NC_CONTAINER_ID, PLDM_USED_COMM_CHANNEL_NUM, PLDM_ENTITY_ASSOCIAION_PHYSICAL);
    }
    /* Dynamic add pdr */
    /* Connector assoc PDR                    : This PDR always exists only if a module is plugged. */
    /* Pluggable Module assoc PDR             : This PDR exists only if a module is plugged in and a cable is plugged in it. */
    /* Communication Channel Entity assoc PDR : This PDR exists only if a module is plugged in and a cable is plugged into it, and the MAC is associated
       with a specific cable. */
}

pldm_data_struct_t *numeric_sensors[3] = {
    temp_sensors[PLUG_TEMP_SENSOR],
    link_speed_sensors,
    plug_power_sensors
};

static void pldm_add_numeric_pdr(u8 port)
{
    u8 data_size[3] = {sizeof(pldm_numeric_sensor_pdr_t), sizeof(pldm_link_speed_sensor_pdr_t), sizeof(pldm_numeric_sensor_pdr_t)};
    fill_common_sensor_pdr func[3] = {fill_common_thermal_sensor_pdr, fill_common_link_speed_sensor_pdr, fill_common_pluggable_module_power_sensor_pdr};
    for (int i = 0; i < sizeof(numeric_sensors) / sizeof(numeric_sensors[0]); i++) {
        numeric_sensors[i][port].previous_state = numeric_sensors[i][port].present_state;
        numeric_sensors[i][port].previous_op_state = numeric_sensors[i][port].op_state;
        numeric_sensors[i][port].present_state = NORMAL;
        numeric_sensors[i][port].op_state = PLDM_OP_ENABLE;
        pldm_add_numeric_sensor_pdr(data_size[i], &numeric_sensors[i][port], func[i]);
    }
}

static void pldm_delete_numeric_pdr(u8 port)
{
    for (int i = 0; i < sizeof(numeric_sensors) / sizeof(numeric_sensors[0]); i++) {
        numeric_sensors[i][port].previous_state = numeric_sensors[i][port].present_state;
        numeric_sensors[i][port].previous_op_state = numeric_sensors[i][port].op_state;
        numeric_sensors[i][port].present_state = UNKNOWN;
        numeric_sensors[i][port].op_state = PLDM_OP_UNAVAILABLE;
        // numeric_sensors[i][port].entity_type = UNKNOWN;
        pldm_delete_numeric_sensor_pdr(&numeric_sensors[i][port]);
    }
}

static void pldm_state_sensor_link_handle(u8 port)
{
    u8 persent_state[3] = {NORMAL, PRESENCE, NORMAL};
    u8 subsensor_cnt = sizeof(plug_composite_state_sensors) / sizeof(plug_composite_state_sensors[0]) / 4;

    for (int i = 0; i < subsensor_cnt; i++) {
        pldm_modify_state_datastruct(persent_state[i], &plug_composite_state_sensors[port * subsensor_cnt + i]);
    }
    pldm_modify_state_datastruct(CONNECTED, &simple_sensors[port]);
}

static void pldm_state_sensor_unlink_handle(u8 port)
{
    u8 persent_state[3] = {UNKNOWN, NOT_PRESENCE, UNKNOWN};
    u8 subsensor_cnt = sizeof(plug_composite_state_sensors) / sizeof(plug_composite_state_sensors[0]) / 4;

    pldm_modify_state_datastruct(DISCONNECTED, &simple_sensors[port]);
    for (int i = 0; i < subsensor_cnt; i++) {
        pldm_modify_state_datastruct(persent_state[i], &plug_composite_state_sensors[port * subsensor_cnt + i]);
    }
}

static void pldm_add_connector_assoc_pdr(u8 port)
{
    pldm_entity_t container, contained;

    /* Connector assoc PDR */

    container.entity_type = CONNECTOR;
    container.entity_instance_num = 1;
    container.entity_container_id = PLDM_BASE_NIC_CONTAINER_ID;

    contained.entity_type = plug_power_sensors[port].entity_type;
    contained.entity_instance_num = 1;

    pldm_add_assoc_pdr(&container, &contained, PLDM_BASE_CONNECTOR_ASSOC_PDR_HANDLE + port, PLDM_BASE_CONNECTOR_CONTAINER_ID + port, 1, \
    PLDM_ENTITY_ASSOCIAION_PHYSICAL);
}

static void pldm_add_pluggable_module_assoc_pdr(u8 port)
{
    pldm_pdr_entity_assoc_t *container_assoc_pdr = NULL;
    pldm_pdr_record_t *is_find = NULL;
    pldm_entity_t contained;

    is_find = pldm_pdr_is_exist(&(g_pldm_monitor_info.pldm_repo), PLDM_BASE_CONNECTOR_ASSOC_PDR_HANDLE + port);

    if (!is_find) {
        LOG("Not find assoc pdr. record handle : %d", PLDM_BASE_CONNECTOR_ASSOC_PDR_HANDLE + port);
        return;
    }

    container_assoc_pdr = (pldm_pdr_entity_assoc_t *)(is_find->data);

    /* Pluggable Module assoc PDR */
    contained.entity_type = CABLE;
    contained.entity_instance_num = 1;
    pldm_add_assoc_pdr(&(container_assoc_pdr->contained[0]), &contained, PLDM_BASE_PLUG_ASSOC_PDR_HANDLE + port, PLDM_BASE_PLUG_CONTAINER_ID + port, 1, \
    PLDM_ENTITY_ASSOCIAION_PHYSICAL);
}

static void pldm_add_comm_chan_assoc_pdr(u8 port)
{
    pldm_pdr_entity_assoc_t *container_assoc_pdr = NULL;
    pldm_pdr_record_t *is_find = NULL;
    pldm_entity_t contained[2];

    is_find = pldm_pdr_is_exist(&(g_pldm_monitor_info.pldm_repo), PLDM_BASE_PLUG_ASSOC_PDR_HANDLE + port);

    if (!is_find) {
        LOG("Not find assoc pdr. record handle : %d", PLDM_BASE_PLUG_ASSOC_PDR_HANDLE + port);
        return;
    }

    container_assoc_pdr = (pldm_pdr_entity_assoc_t *)(is_find->data);

    /* Communication Channel Entity assoc PDR */
    contained[0].entity_type = ETH_PORT;
    contained[0].entity_instance_num = 1;

    contained[1].entity_type = CABLE;
    contained[1].entity_instance_num = 1;
    pldm_add_assoc_pdr(&(container_assoc_pdr->contained[0]), contained, PLDM_BASE_COMM_CHAN_ASSOC_PDR_HANDLE + port, PLDM_BASE_COMM_CHAN_CONTAINER_ID + port, 2, \
    PLDM_ENTITY_ASSOCIAION_LOGICAL);
}

static void pldm_delete_connector_assoc_pdr(u8 port)
{
    /* Connector assoc PDR */
    pldm_delete_assoc_pdr(PLDM_BASE_CONNECTOR_ASSOC_PDR_HANDLE + port);
}

static void pldm_delete_pluggable_module_assoc_pdr(u8 port)
{
    /* Pluggable Module assoc PDR */
    pldm_delete_assoc_pdr(PLDM_BASE_PLUG_ASSOC_PDR_HANDLE + port);
}

static void pldm_delete_comm_chan_assoc_pdr(u8 port)
{
    /* Communication Channel Entity assoc PDR */
    pldm_delete_assoc_pdr(PLDM_BASE_COMM_CHAN_ASSOC_PDR_HANDLE + port);
}

void terminus_locator_pdr_chg(void)
{
    pldm_pdr_record_t *is_find = (pldm_pdr_record_t *)pldm_pdr_find(&(g_pldm_monitor_info.pldm_repo), PLDM_TERMINUS_LOCATOR_PDR_HANDLE);
    if (!is_find)
        return;
    pldm_terminus_locator_pdr_t *terminus_locator_pdr = (pldm_terminus_locator_pdr_t *)(is_find->data);
    if ((g_pldm_monitor_info.tid != terminus_locator_pdr->tid) || (g_mctp_ctrl_info[1].dev_eid != terminus_locator_pdr->eid)) {
        g_pldm_monitor_info.repo_state = PLDM_REPO_UPDATE_IN_PROGRESS;
        terminus_locator_pdr->hdr.record_change_num++;
        terminus_locator_pdr->tid = g_pldm_monitor_info.tid;
        terminus_locator_pdr->eid = g_mctp_ctrl_info[1].dev_eid;      /* 待定 */
        pldm_pdr_chg_event_generate(g_pldm_monitor_info.pldm_event_rbuf, PLDM_REPO_CHG_FORMAT_ID_PDR_HANDLE, PLDM_REPO_CHG_RECORDS_MODIFIED, PLDM_TERMINUS_LOCATOR_PDR_HANDLE);
        g_pldm_monitor_info.repo_state = PLDM_REPO_AVAILABLE;
        pldm_monitor_update_repo_signature(&(g_pldm_monitor_info.pldm_repo));
        pldm_monitor_update_repo_time(&(g_pldm_monitor_info.pldm_repo.update_time));
    }
}

static pldm_monitor_handle pldm_link_func[][5] = {
    [0] = {
        pldm_delete_numeric_pdr,

        pldm_state_sensor_unlink_handle,

        pldm_delete_connector_assoc_pdr,
        pldm_delete_pluggable_module_assoc_pdr,
        pldm_delete_comm_chan_assoc_pdr
    },
    [1] = {
        pldm_add_numeric_pdr,

        pldm_state_sensor_link_handle,

        pldm_add_connector_assoc_pdr,
        pldm_add_pluggable_module_assoc_pdr,
        pldm_add_comm_chan_assoc_pdr
    }
};

void pldm_link_handle(u8 port, u8 link_state)
{
    if (port >= MAX_LAN_NUM || ((link_state != LINK_DOWN) && (link_state != LINK_UP))) return;
    g_pldm_monitor_info.repo_state = PLDM_REPO_UPDATE_IN_PROGRESS;
    for (int i = 0; i < (sizeof(pldm_link_func[link_state]) / sizeof(pldm_link_func[link_state][0])); i++) {
        if (pldm_link_func[link_state][i] != NULL) {
            pldm_link_func[link_state][i](port);
        }
    }
    pldm_redfish_msg_event_generate(g_pldm_monitor_info.pldm_event_rbuf, PLDM_BASE_NETWORK_DEV_FUNC_RESOURCE_ID + port, link_state);
    /* 12.8.2.3 PDR Dynamic Changes Flow */
    pldm_modify_state_datastruct(CONFIG_CHG, &nic_composite_state_sensors[2]);

    g_pldm_monitor_info.repo_state = PLDM_REPO_AVAILABLE;

    pldm_monitor_update_repo_signature(&(g_pldm_monitor_info.pldm_repo));
    pldm_monitor_update_repo_time(&(g_pldm_monitor_info.pldm_repo.update_time));
}

/* 初始化更新pdr */
void pldm_thermal_sensor_pdr_update(u16 sensor_id, u8 sensor_type, u8 port)
{
    pldm_temp_sensor_threshold_data_t temp_data;
    pldm_pdr_record_t *is_find = NULL;
    u32 record_hanle = sensor_id_convert_to_record_handle(sensor_id);

    is_find = pldm_pdr_find(&(g_pldm_monitor_info.pldm_repo), record_hanle);
    if (!is_find) {
        LOG("[pldm_thermal_sensor_pdr_update], err record handle : %d", record_hanle);
        return;
    }
    pldm_get_temp_sensor_threshold(&temp_data, sensor_type, port);

    pldm_numeric_sensor_pdr_t *thermal_pdr = (pldm_numeric_sensor_pdr_t *)(is_find->data);
    thermal_pdr->thermal_pdr.warning_high = temp_data.warning_high;
    thermal_pdr->thermal_pdr.critical_high = temp_data.critical_high;
    thermal_pdr->thermal_pdr.fatal_high = temp_data.fatal_high;
}

/* refer to SFF-8024 Rev 4.6, https://www.gigalight.com/downloads/standards/sff-8024.pdf */
static void pldm_identifier_update(u8 port)
{
    u16 raw_val = CM_MODULE_GET_IDENTIFIER(port);
    switch (raw_val) {
        case 0x03:  /* SFP/SFP+/SFP28 */
            temp_sensors[PLUG_TEMP_SENSOR][port].entity_type = SFP;
            plug_power_sensors[port].entity_type = SFP;
            break;
        case 0x0C:  /* QSFP (INF-8438) */
            temp_sensors[PLUG_TEMP_SENSOR][port].entity_type = QSFP;
            plug_power_sensors[port].entity_type = QSFP;
            break;
        case 0x11:  /* QSFP28 or later with SFF-8636 management interface (SFF-8665 et al.) *2  */
            temp_sensors[PLUG_TEMP_SENSOR][port].entity_type = QSFP28;
            plug_power_sensors[port].entity_type = QSFP28;
            break;
        default:
            break;
    }
}

/* 摄氏度 */
void pldm_get_temp_sensor_threshold(pldm_temp_sensor_threshold_data_t *temp_data, u8 sensor_type, u8 port)
{
    if (!temp_data || port >= MAX_LAN_NUM) return;
    switch(sensor_type) {
        case PLUG_TEMP_SENSOR:
            /* 保留整数 (0 ~ 127) */
            temp_data->warning_high = CM_MODULE_GET_WARN_DATA(port);
            temp_data->critical_high = CM_MODULE_GET_CRITICAL_DATA(port);
            temp_data->fatal_high = CM_MODULE_GET_FATAL_DATA(port);
            temp_data->warning_high >>= 8;
            temp_data->critical_high >>= 8;
            temp_data->fatal_high >>= 8;
            break;
        case NIC_TEMP_SENSOR:
        case NC_TEMP_SENSOR:
            temp_data->warning_high = CM_NIC_GET_WARN_DATA;
            temp_data->critical_high = CM_NIC_GET_CRITICAL_DATA;
            temp_data->fatal_high = CM_NIC_GET_FATAL_DATA;
            break;
    }
}

void pldm_get_link_spd_cap(pldm_speed_sensor_cap_t *speed_data, u8 port)
{
    if (!speed_data || port >= MAX_LAN_NUM) return;
    speed_data->max_readable = CM_MODULE_GET_SIGNAL_RATE_DATA(port) * 1000;
    speed_data->min_readable = 0;
}

/* in period monitor task in the future */
void pldm_module_info_update(u8 port)
{
    if (port >= MAX_LAN_NUM) return;
    pldm_identifier_update(port);
    /* 摄氏度, 保留整数 (0 ~ 127) */
    u16 raw_val = CM_MODULE_GET_TEMPERATURE_DATE(port);
    temp_sensors[PLUG_TEMP_SENSOR][port].present_reading = raw_val & CBIT(15) ? 0 : raw_val >> 8;
    /* 0.1微瓦 */
    plug_power_sensors[port].present_reading = CM_MODULE_GET_POWER_DATA(port);
    /* MBd */
    link_speed_sensors[port].present_reading = CM_MODULE_GET_SIGNAL_RATE_DATA(port) * 1000;
}