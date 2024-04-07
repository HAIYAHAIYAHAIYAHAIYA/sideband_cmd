#include "pldm_monitor.h"
#include "pldm.h"
#include "mctp.h"
#include "pldm_monitor_event_rbuf.h"

static u8 gs_pdrs_buf[PDR_POOL_SIZE] = {0};

static u8 gs_entity_event_data[128] = {0};                                  /* for Platform Event Message cmd */
static pldm_event_data_t gs_pldm_event_data;

static u32 gs_event_previous_data_transfer_handle = 0x00000000;
static u8 gs_event_transfer_flag = PLDM_TRANSFER_FLAG_START_AND_END;

static pldm_pdr_record_t *gs_find_pdr = NULL;                               /* for get pdr cmd */
static pldm_pdr_hdr_t *gs_find_pdr_comm_hdr = NULL;

static u32 gs_pdr_previous_data_transfer_handle = 0x00000000;
static u8 gs_pdr_transfer_flag = PLDM_TRANSFER_FLAG_START_AND_END;

pldm_monitor_base_info_t g_pldm_monitor_info;
u8 g_pldm_need_rsp = 1;

extern pldm_temp_sensor_data_struct_t temp_sensors[NIC_TEMP_SENSOR + NC_TEMP_SENSOR + PLUG_TEMP_SENSOR][4];
extern pldm_link_speed_data_struct_t link_speed_sensors[PLDM_LINK_SPEED_SENSOR_NUM];
extern pldm_plug_power_data_struct_t plug_power_sensors[PLDM_PLUG_POWER_SENSOR_NUM];

extern pldm_nic_composite_state_sensor_data_struct_t nic_composite_state_sensors[4];
extern pldm_controller_composite_state_sensor_data_struct_t controller_composite_state_sensors[5];
extern pldm_plug_composite_state_sensor_data_struct_t plug_composite_state_sensors[12];
extern pldm_simple_sensor_data_struct_t simple_sensors[4];

extern pldm_temp_sensor_monitor_t temp_sensor_monitor[PLDM_NIC_TEMP_SENSOR_NUM + PLDM_NC_TEMP_SENSOR_NUM + PLDM_USED_PLUG_TEMP_SENSOR_NUM];

extern mctp_base_info g_mctp_ctrl_info[3];
extern u16 g_event_id;

extern void pldm_ctrl_set_tid(protocol_msg_t *pkt, int *pkt_len);
extern void pldm_ctrl_get_tid(protocol_msg_t *pkt, int *pkt_len);
extern void pldm_unsupport_cmd(protocol_msg_t *pkt, int *pkt_len);
extern u32 crc32_pldm(u32 init_crc, u8 *data, u32 len);

u8 crc8_pldm(u8 *data, u16 len);

static void pldm_monitor_get_uuid(protocol_msg_t *pkt, int *pkt_len)
{
    get_uuid_rsp_dat *rsp_dat = (get_uuid_rsp_dat *)(pkt->rsp_buf);

    cm_memcpy((u8 *)&(rsp_dat->time_low), (u8 *)&(g_mctp_ctrl_info[pkt->mctp_hw_id].uuid), sizeof(get_uuid_rsp_dat));

    *pkt_len += sizeof(get_uuid_rsp_dat);
    LOG("mctp get uuid cmd, uuid node : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x", rsp_dat->node[0], rsp_dat->node[1],
            rsp_dat->node[2], rsp_dat->node[3], rsp_dat->node[4], rsp_dat->node[5]);
}

static void pldm_monitor_set_event_receiver(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_set_event_receiver_req_dat_t *req_dat = (pldm_set_event_receiver_req_dat_t *)(pkt->req_buf);
    mctp_hdr_t *req_mctp_hdr = (mctp_hdr_t *)(pkt->req_buf - sizeof(pldm_request_t) - sizeof(mctp_hdr_t));
    // pcie_extcm_msg_t *req_vdm_hdr = (pcie_extcm_msg_t *)((u8 *)req_mctp_hdr - VDM_HDR_LEN);     /* default only support vdm. */
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    if (req_dat->event_msg_global_en == PLDM_DISABLE) {
        g_pldm_monitor_info.terminus_mode = PLDM_DISABLE;
        goto L_ERR;
    } else {
        if (req_dat->trans_protocol_type != 0x00) {                 /* 0x00 (MCTP) */
            rsp_hdr->cpl_code = PLDM_INVALID_PROTOCOL_TYPE;
            goto L_ERR;
        }

        g_pldm_monitor_info.hw_id = pkt->mctp_hw_id;

        if (req_dat->event_msg_global_en == PLDM_ENABLE_ASYNC || req_dat->event_msg_global_en == PLDM_ENABLE_POLLING) {
            if (req_mctp_hdr->src_eid == req_dat->event_receiver_addr_info) {
                g_pldm_monitor_info.terminus_mode = req_dat->event_msg_global_en;
                g_pldm_monitor_info.event_receiver_eid = req_dat->event_receiver_addr_info;
                // g_pldm_monitor_info.phy_addr.val = req_vdm_hdr->qw1.req_id;
                /* If the source EID in the packet is the same as the eventReceiverAddressInfo, use the requester ID
                   in the TLP as the B,D,F of the receiver(E810). */
            } else {
                LOG("send Resolve Endpoint ID MCTP command, sre_eid : %d, receiver_addr_info : %d", req_mctp_hdr->src_eid, req_dat->event_receiver_addr_info);
                mctp_resolve_eid_req(pkt->mctp_hw_id, req_dat->event_receiver_addr_info);
                /*  send a Resolve Endpoint ID MCTP command to get the B,D,F of the Event Receiver. If
                    the command fails four times, events are not sent.(E810) */
            }
        } else {
            LOG("Unsupport event_msg_global_en : %d", req_dat->event_msg_global_en);
            rsp_hdr->cpl_code = PLDM_ENABLE_METHOD_NOT_SUPPORTED;
        }
    }
L_ERR:
    return;
}

static void pldm_monitor_get_event_receiver(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_get_event_receiver_rsp_dat_t *rsp_dat = (pldm_get_event_receiver_rsp_dat_t *)(pkt->rsp_buf);

    rsp_dat->trans_protocol_type = 0x00;                            /* 0x00 (MCTP) */
    rsp_dat->event_receiver_addr_info = g_pldm_monitor_info.event_receiver_eid;

    *pkt_len += sizeof(pldm_get_event_receiver_rsp_dat_t);
}

static void pldm_monitor_platform_event_msg_rsp(protocol_msg_t *pkt, int *pkt_len)
{
    g_pldm_need_rsp = 0;
    pldm_platform_event_msg_receive_t *req_dat = (pldm_platform_event_msg_receive_t *)(pkt->req_buf);
    if (req_dat->cpl_code != MCTP_COMMAND_SUCCESS) {
        LOG("Need re-send event msg, cpl code : 0x%x", req_dat->cpl_code);
        return;
    }

    if (g_pldm_monitor_info.terminus_mode == PLDM_ENABLE_ASYNC) {
        pldm_event_rbuf_read_done(g_pldm_monitor_info.pldm_event_rbuf);
    }
}

static void pldm_monitor_poll_for_platform_event_msg(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_poll_for_platform_event_msg_req_dat_t *req_dat = (pldm_poll_for_platform_event_msg_req_dat_t *)(pkt->req_buf);
    pldm_poll_for_platform_event_msg_rsp_dat_t *rsp_dat = (pldm_poll_for_platform_event_msg_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    if (req_dat->format_ver != PLDM_EVENT_FORMAT_VERSION) {
        rsp_hdr->cpl_code = PLDM_UNSUPPORTED_EVENT_FORMAT_VERSION;
        goto L_ERR;
    }

    if (gs_pldm_event_data.event_id == req_dat->event_id_to_ack) {
        pldm_event_rbuf_read_done(g_pldm_monitor_info.pldm_event_rbuf);
    }

    rsp_dat->tid = g_pldm_monitor_info.tid;

    if (pldm_event_rbuf_is_empty(g_pldm_monitor_info.pldm_event_rbuf)) {
        g_event_id = 1;
        rsp_dat->event_id = 0x0000;
        *pkt_len += sizeof(rsp_dat->event_id) + sizeof(rsp_dat->tid);
        g_pldm_monitor_info.terminus_mode = PLDM_ENABLE_ASYNC;                        /* exit poll state */
        gs_event_previous_data_transfer_handle = 0x00000000;
        gs_event_transfer_flag = PLDM_TRANSFER_FLAG_END;
        return;
    }

    if (gs_event_transfer_flag == PLDM_TRANSFER_FLAG_END || gs_event_transfer_flag == PLDM_TRANSFER_FLAG_START_AND_END) {
        pldm_event_rbuf_try_read(g_pldm_monitor_info.pldm_event_rbuf, &gs_pldm_event_data, sizeof(pldm_event_data_t), 0);
        pldm_event_rbuf_try_read(g_pldm_monitor_info.pldm_event_rbuf, gs_entity_event_data, gs_pldm_event_data.event_data_size, sizeof(pldm_event_data_t));
    }

    rsp_dat->event_id = gs_pldm_event_data.event_id;               /* current event id */
    /* eventClass (E810)
       Values:
        pldmPDRRepositoryChgEvent — A change has been made to the repository.
        redfishTaskExecuteEvent   — A long running task spawned by an RDE Operation has completed execution.
        redfishMessageEvent       — A Redfish event has occurred. */
    rsp_dat->event_class = gs_pldm_event_data.event_class;

    if (req_dat->transfer_op_flag == PLDM_TRANSFER_OP_FLAG_ACK_ONLY) {
        rsp_dat->event_id = 0xFFFF;
        *pkt_len += sizeof(rsp_dat->event_id) + sizeof(rsp_dat->tid);
        g_pldm_monitor_info.terminus_mode = PLDM_ENABLE_ASYNC;                        /* exit poll state */
        gs_event_previous_data_transfer_handle = 0x00000000;
        gs_event_transfer_flag = PLDM_TRANSFER_FLAG_END;
        return;
    }

    u8 state = (req_dat->transfer_op_flag == PLDM_TRANSFER_OP_FLAG_GET_NEXT_PART && req_dat->event_id_to_ack != 0xFFFF);
    state |= (req_dat->transfer_op_flag == PLDM_TRANSFER_OP_FLAG_GET_FIRST_PART && req_dat->event_id_to_ack == 0xFFFF);
    if (state) {
        rsp_hdr->cpl_code = PLDM_EVENT_ID_NOT_VALID;
        goto L_ERR;
    }

    if (req_dat->data_transfer_handle != gs_event_previous_data_transfer_handle) {
        rsp_hdr->cpl_code = PLDM_INVALID_DATA_TRANSFER_HANDLE;
        goto L_ERR;
    }

    u16 max_pkt_len = g_pldm_monitor_info.terminus_max_buffersize - PLDM_SYNC_SEND_FIELD_LEN;
    u32 remain_bytes = gs_pldm_event_data.event_data_size - req_dat->data_transfer_handle;
    rsp_dat->event_datasize = ((remain_bytes + 4) <= max_pkt_len) ? remain_bytes : max_pkt_len;
    /*  If appending the DataIntegrityChecksum would cause this response message to exceed the 
        negotiated maximum transfer chunk size (clause 10.2), the DataIntegrityChecksum shall be sent as 
        the only data in another chunk. */
    if (remain_bytes < max_pkt_len && (remain_bytes + 4) > max_pkt_len) {
        rsp_dat->event_datasize = remain_bytes;
    }

    if ((remain_bytes + 4) <= max_pkt_len) {
        if (req_dat->transfer_op_flag == PLDM_TRANSFER_OP_FLAG_GET_FIRST_PART) {
            rsp_dat->transfer_flag = PLDM_TRANSFER_FLAG_START_AND_END;
        } else {
            rsp_dat->transfer_flag = PLDM_TRANSFER_FLAG_END;
        }
        rsp_dat->next_data_transfer_handle = 0x00000000;
        *(u32 *)&(rsp_dat->event_data[rsp_dat->event_datasize]) = crc32_pldm(0xFFFFFFFFUL, gs_entity_event_data, gs_pldm_event_data.event_data_size);
        *pkt_len += sizeof(u32);
    } else {
        if (req_dat->transfer_op_flag == PLDM_TRANSFER_OP_FLAG_GET_FIRST_PART) {
            rsp_dat->transfer_flag = PLDM_TRANSFER_FLAG_START;
        } else {
            rsp_dat->transfer_flag = PLDM_TRANSFER_FLAG_MIDDLE;
        }
        rsp_dat->next_data_transfer_handle = req_dat->data_transfer_handle + rsp_dat->event_datasize;
    }
    cm_memcpy(rsp_dat->event_data, (gs_entity_event_data + req_dat->data_transfer_handle), rsp_dat->event_datasize);
    gs_event_previous_data_transfer_handle = rsp_dat->next_data_transfer_handle;
    gs_event_transfer_flag = rsp_dat->transfer_flag;
    *pkt_len += sizeof(pldm_poll_for_platform_event_msg_rsp_dat_t) + rsp_dat->event_datasize;

L_ERR:
    LOG("cpl_code : %#x, event_datasize : %d, rsp_dat->event_datasize : %d, req_dat->data_transfer_handle : %d", rsp_hdr->cpl_code, gs_pldm_event_data.event_data_size, rsp_dat->event_datasize, req_dat->data_transfer_handle);
    return;
}

static void pldm_monitor_event_msg_supported(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_event_msg_supported_req_dat_t *req_dat = (pldm_event_msg_supported_req_dat_t *)(pkt->req_buf);
    pldm_event_msg_supported_rsp_dat_t *rsp_dat = (pldm_event_msg_supported_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    if (req_dat->format_ver != PLDM_EVENT_FORMAT_VERSION) {
        rsp_hdr->cpl_code = PLDM_UNSUPPORTED_EVENT_FORMAT_VERSION;
        return;
    }
    rsp_dat->sync_config = g_pldm_monitor_info.terminus_mode;
    rsp_dat->sync_config_supported = CBIT(1) | CBIT(2);             /* Synchronous and Asynchronous support without heartbeat */
    rsp_dat->num_event_class = 0x05;
    rsp_dat->event_class[0] = SENSOR_EVENT;
    rsp_dat->event_class[1] = REDFISH_TASK_EXCUTE_EVENT;
    rsp_dat->event_class[2] = REDFISH_MSG_EVENT;
    rsp_dat->event_class[3] = PLDM_PDR_REPO_CHG_EVENT;
    rsp_dat->event_class[4] = PLDM_MSG_POLL_EVENT;

    *pkt_len += sizeof(pldm_event_msg_supported_rsp_dat_t) + sizeof(u8) * rsp_dat->num_event_class;
}

static void pldm_monitor_event_msg_buffersize(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_event_msg_buffersize_req_dat_t *req_dat = (pldm_event_msg_buffersize_req_dat_t *)(pkt->req_buf);
    pldm_event_msg_buffersize_rsp_dat_t *rsp_dat = (pldm_event_msg_buffersize_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    if (req_dat->event_receiver_max_buffersize < PLDM_EVENT_RECEIVER_MIN_BUFFERSIZE) {
        g_pldm_monitor_info.terminus_max_buffersize = PLDM_TERMINUS_DEFAULT_BUFFERSIZE;
        rsp_hdr->cpl_code = PLDM_ERROR_INVALID_DATA;
        return;
    }

    rsp_dat->terminus_max_buffersize = MIN(req_dat->event_receiver_max_buffersize, PLDM_TERMINUS_MAX_BUFFERSIZE);                  /* contains pldm hdr and payload. */
    g_pldm_monitor_info.terminus_max_buffersize = rsp_dat->terminus_max_buffersize;                                                  /* event data len */

    *pkt_len += sizeof(pldm_event_msg_buffersize_rsp_dat_t);
}

static u8 is_link_speed_sensor(u16 sensor_id)
{
    if (sensor_id >= PLDM_BASE_LINK_SPEED_SENSOR_ID && sensor_id <= PLDM_MAX_LINK_SPEED_SENSOR_ID) {
        return TRUE;
    }
    return FALSE;
}

int is_temp_sensor(u16 sensor_id)
{
    if (sensor_id >= PLDM_BASE_NIC_TEMP_SENSOR_ID && sensor_id <= PLDM_MAX_NIC_TEMP_SENSOR_ID) {
        return NIC_TEMP_SENSOR;
    }
    if (sensor_id >= PLDM_BASE_NC_TEMP_SENSOR_ID && sensor_id <= PLDM_MAX_NC_TEMP_SENSOR_ID) {
        return NC_TEMP_SENSOR;
    }
    if (sensor_id >= PLDM_BASE_PLUG_TEMP_SENSOR_ID && sensor_id <= PLDM_MAX_PLUG_TEMP_SENSOR_ID) {
        return PLUG_TEMP_SENSOR;
    }
    return -1;
}

static u8 is_plug_power_comsumption_sensor(u16 sensor_id)
{
    if (sensor_id >= PLDM_BASE_PLUG_POWER_SENSOR_ID && sensor_id <= PLDM_MAX_PLUG_POWER_SENSOR_ID) {
        return TRUE;
    }
    return FALSE;
}

static u8 is_nic_composite_sensor(u16 sensor_id)
{
    if (sensor_id >= PLDM_BASE_CARD_COMPOSITE_STATE_SENSOR_ID && sensor_id <= PLDM_MAX_CARD_COMPOSITE_STATE_SENSOR_ID) {
        return TRUE;
    }
    return FALSE;
}

static u8 is_nc_composite_sensor(u16 sensor_id)
{
    if (sensor_id >= PLDM_BASE_NC_STATE_SENSOR_ID && sensor_id <= PLDM_MAX_NC_STATE_SENSOR_ID) {
        return TRUE;
    }
    return FALSE;
}

static u8 is_plug_composite_sensor(u16 sensor_id)
{
    if (sensor_id >= PLDM_BASE_PLUG_COMPOSITE_SENSOR_ID && sensor_id <= PLDM_MAX_PLUG_COMPOSITE_SENSOR_ID) {
        return TRUE;
    }
    return FALSE;
}

static u8 is_link_simple_sensor(u16 sensor_id)
{
    if (sensor_id >= PLDM_BASE_LINK_STATE_SENSOR_ID && sensor_id <= PLDM_MAX_LINK_STATE_SENSOR_ID) {
        return TRUE;
    }
    return FALSE;
}

static void pldm_monitor_set_numeric_sensor_enable(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_set_numeric_sensor_enable_rsp_dat_t *req_dat = (pldm_set_numeric_sensor_enable_rsp_dat_t *)(pkt->req_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    u16 sensor_id = req_dat->sensor_id;
    pldm_data_struct_t *sensor_data = NULL;
    pldm_data_struct_t *sensors = NULL;
    /* Only accepted values are the sensor IDs of Thermal sensors and Link Speed sensors */

    int is_temp = is_temp_sensor(sensor_id);

    if (is_temp != -1) {
        sensors = temp_sensors[is_temp];
    } else if (is_link_speed_sensor(sensor_id)) {
        sensors = link_speed_sensors;
    } else {
        LOG("Invalid sensor id : %d", sensor_id);
        rsp_hdr->cpl_code = PLDM_INVALID_SENSOR_ID;
        goto L_ERR;
    }

    for (u8 i = 0; i < MAX_LAN_NUM; i++) {
        if (sensor_id == sensors[i].sensor_id) {
            sensor_data = &sensors[i];
            break;
        }
    }

    if (req_dat->sensor_op_state > PLDM_OP_IN_TEST) {
        rsp_hdr->cpl_code = PLDM_INVALID_SENSOR_OPERATIONAL_STATE;
        goto L_ERR;
    }

    if (req_dat->sensor_event_msg_en > PLDM_EVENT_STATE_EN_ONLY) {
        rsp_hdr->cpl_code = PLDM_EVENT_GENERATION_NOT_SUPPORTED;
        goto L_ERR;
    }

    if (sensor_data) {
        if (sensor_data->event_msg_en != req_dat->sensor_event_msg_en) {
            sensor_data->event_msg_en = req_dat->sensor_event_msg_en;
            if (req_dat->sensor_event_msg_en == PLDM_EVENT_OP_EN_ONLY) {
                sensor_data->entity_type = PLDM_SENSOR_OP_STATE;
            } else if (req_dat->sensor_event_msg_en == PLDM_EVENT_EN) {
                sensor_data->entity_type = PLDM_NUMERIC_SENSOR_STATE;
            }
        }
        if (sensor_data->op_state != req_dat->sensor_op_state) {
            sensor_data->previous_op_state = sensor_data->op_state;
            sensor_data->op_state = req_dat->sensor_op_state;
            pldm_sensor_event_generate(g_pldm_monitor_info.pldm_event_rbuf, sensor_data->sensor_event_class, sensor_data->event_msg_en, (void *)sensor_data);
        }
    } else {
        LOG("Invalid sensor id : %d", sensor_id);
        rsp_hdr->cpl_code = PLDM_INVALID_SENSOR_ID;
    }

L_ERR:
    return;
}

static void pldm_monitor_get_sensor_reading(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_get_sensor_reading_req_dat_t *req_dat = (pldm_get_sensor_reading_req_dat_t *)(pkt->req_buf);
    pldm_get_sensor_reading_rsp_dat_t *rsp_dat = (pldm_get_sensor_reading_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    u16 sensor_id = req_dat->sensor_id;
    pldm_data_struct_t *sensor_data = NULL;
    pldm_data_struct_t *sensors = NULL;
    /* Only accepted values are the IDs of Temperature, Link Speed, and Power Consumption sensors */

    int is_temp = is_temp_sensor(sensor_id);

    if (is_temp != -1) {
        sensors = temp_sensors[is_temp];
    } else if (is_link_speed_sensor(sensor_id)) {
        sensors = link_speed_sensors;
    } else if (is_plug_power_comsumption_sensor(sensor_id)) {
        sensors = plug_power_sensors;
    } else {
        LOG("Invalid sensor id : %d", sensor_id);
        rsp_hdr->cpl_code = PLDM_INVALID_SENSOR_ID;
       goto L_ERR;
    }

    for (u8 i = 0; i < MAX_LAN_NUM; i++) {
        if (sensor_id == sensors[i].sensor_id) {
            sensor_data = &sensors[i];
            break;
        }
    }

    if (sensor_data) {
        if (sensor_data->present_state == PLDM_OP_UNAVAILABLE) {
            LOG("PLDM_OP_UNAVAILABLE");
            rsp_hdr->cpl_code = PLDM_EARM_UNAVAILABLE_IN_PRESENT_STATE;
            goto L_ERR;
        }
        rsp_dat->sensor_datasize = sensor_data->sensor_data_size;
        rsp_dat->sensor_op_state = sensor_data->op_state;   /*  must be ignored while the sensorOperationalState value of the sensor is Unavailable. */
        rsp_dat->sensor_event_msg_en = sensor_data->event_msg_en;
        rsp_dat->present_state = sensor_data->present_state;
        rsp_dat->previous_state = sensor_data->previous_state;
        rsp_dat->event_state = sensor_data->present_state;
        rsp_dat->present_reading = sensor_data->present_reading;

        *pkt_len += sizeof(pldm_get_sensor_reading_rsp_dat_t);
    } else {
        LOG("Invalid sensor id : %d", sensor_id);
        rsp_hdr->cpl_code = PLDM_INVALID_SENSOR_ID;
    }

L_ERR:
    return;
}

u32 sensor_id_convert_to_record_handle(u16 sensor_id)
{
    u32 record_handle = PLDM_ERR_RECORD_HANDLE;
    u32 max_record_handle = 0;
    if (sensor_id == PLDM_BASE_CARD_COMPOSITE_STATE_SENSOR_ID) {
        record_handle = PLDM_BASE_CARD_COMPOSITE_STATE_SENSOR_HANDLE;
        max_record_handle = PLDM_MAX_CARD_COMPOSITE_STATE_SENSOR_HANDLE;
    } else {
        switch (sensor_id / 10)
        {
            case PLDM_BASE_NIC_TEMP_SENSOR_ID / 10 :
                record_handle = PLDM_BASE_NIC_TEMP_SENSOR_HANDLE + sensor_id - PLDM_BASE_NIC_TEMP_SENSOR_ID;
                max_record_handle = PLDM_MAX_NIC_TEMP_SENSOR_HANDLE;
                break;
            case PLDM_BASE_NC_STATE_SENSOR_ID / 10 :
                record_handle = PLDM_BASE_NC_STATE_SENSOR_HANDLE + sensor_id - PLDM_BASE_NC_STATE_SENSOR_ID;
                max_record_handle = PLDM_MAX_NC_STATE_SENSOR_HANDLE;
                break;
            case PLDM_BASE_LINK_SPEED_SENSOR_ID / 10 :
                record_handle = PLDM_BASE_LINK_SPEED_SENSOR_HANDLE + sensor_id - PLDM_BASE_LINK_SPEED_SENSOR_ID;
                max_record_handle = PLDM_MAX_LINK_SPEED_SENSOR_HANDLE;
                break;
            case PLDM_BASE_LINK_STATE_SENSOR_ID / 10 :
                record_handle = PLDM_BASE_LINK_STATE_SENSOR_HANDLE + sensor_id - PLDM_BASE_LINK_STATE_SENSOR_ID;
                max_record_handle = PLDM_MAX_LINK_STATE_SENSOR_HANDLE;
                break;
            case PLDM_BASE_NC_TEMP_SENSOR_ID / 10 :
                record_handle = PLDM_BASE_NC_TEMP_SENSOR_HANDLE + sensor_id - PLDM_BASE_NC_TEMP_SENSOR_ID;
                max_record_handle = PLDM_MAX_NC_TEMP_SENSOR_HANDLE;
                break;
            case PLDM_BASE_PLUG_POWER_SENSOR_ID / 10 :
                record_handle = PLDM_BASE_PLUG_POWER_SENSOR_HANDLE + sensor_id - PLDM_BASE_PLUG_POWER_SENSOR_ID;
                max_record_handle = PLDM_MAX_PLUG_POWER_SENSOR_HANDLE;
                break;
            case PLDM_BASE_PLUG_TEMP_SENSOR_ID / 10 :
                record_handle = PLDM_BASE_PLUG_TEMP_SENSOR_HANDLE + sensor_id - PLDM_BASE_PLUG_TEMP_SENSOR_ID;
                max_record_handle = PLDM_MAX_PLUG_TEMP_SENSOR_HANDLE;
                break;
            case PLDM_BASE_PLUG_COMPOSITE_SENSOR_ID / 10 :
                record_handle = PLDM_BASE_PLUG_COMPOSITE_SENSOR_HANDLE + sensor_id - PLDM_BASE_PLUG_COMPOSITE_SENSOR_ID;
                max_record_handle = PLDM_MAX_PLUG_COMPOSITE_SENSOR_HANDLE;
                break;
            default :
                break;
        }
    }
    if (record_handle > max_record_handle) {
        record_handle = PLDM_ERR_RECORD_HANDLE;
    }
    return record_handle;
}

static void pldm_monitor_get_sensor_thresholds(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_get_sensor_thresholds_req_dat_t *req_dat = (pldm_get_sensor_thresholds_req_dat_t *)(pkt->req_buf);
    pldm_get_sensor_thresholds_rsp_dat_t *rsp_dat = (pldm_get_sensor_thresholds_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    u16 sensor_id = req_dat->sensor_id;
    int is_temp = is_temp_sensor(sensor_id);
    /* Only accepted values are the Temperature Sensor IDs */

    u32 record_handle = sensor_id_convert_to_record_handle(sensor_id);

    if (((is_temp == -1)) || (record_handle == PLDM_ERR_RECORD_HANDLE)) {
        rsp_hdr->cpl_code = PLDM_INVALID_SENSOR_ID;
        goto L_ERR;
    }

    pldm_pdr_record_t *find_pdr = pldm_pdr_find(&(g_pldm_monitor_info.pldm_repo), record_handle);

    if (!find_pdr) {
        rsp_hdr->cpl_code = PLDM_INVALID_SENSOR_ID;
        goto L_ERR;
    }

    rsp_dat->sensor_datasize = PLDM_DATASIZE_UINT8;

    pldm_numeric_sensor_pdr_t *temp_pdr = (pldm_numeric_sensor_pdr_t *)(find_pdr->data);

    rsp_dat->upper_threshold_warning = temp_pdr->thermal_pdr.warning_high;
    rsp_dat->upper_threshold_critical = temp_pdr->thermal_pdr.critical_high;
    rsp_dat->upper_threshold_fatal = temp_pdr->thermal_pdr.fatal_high;

    rsp_dat->lower_threshold_warning = 0;
    rsp_dat->lower_threshold_critical = 0;
    rsp_dat->lower_threshold_fatal = 0;

    *pkt_len += sizeof(pldm_get_sensor_thresholds_rsp_dat_t);

L_ERR:
    return;
}

static void pldm_monitor_set_sensor_thresholds(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_set_sensor_thresholds_req_dat_t *req_dat = (pldm_set_sensor_thresholds_req_dat_t *)(pkt->req_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    u16 sensor_id = req_dat->sensor_id;
    int is_temp = is_temp_sensor(sensor_id);
    /* Only accepted values are the Temperature Sensor IDs */

    if (req_dat->sensor_datasize != PLDM_DATASIZE_UINT8) {
        rsp_hdr->cpl_code = PLDM_ERROR_INVALID_DATA;
        goto L_ERR;
    }

    u32 record_handle = sensor_id_convert_to_record_handle(sensor_id);

    if ((is_temp == -1) || (record_handle == PLDM_ERR_RECORD_HANDLE)) {
        rsp_hdr->cpl_code = PLDM_INVALID_SENSOR_ID;
        goto L_ERR;
    }

    pldm_pdr_record_t *find_pdr = pldm_pdr_find(&(g_pldm_monitor_info.pldm_repo), record_handle);

    if (!find_pdr) {
        rsp_hdr->cpl_code = PLDM_INVALID_SENSOR_ID;
        goto L_ERR;
    }

    pldm_numeric_sensor_pdr_t *temp_pdr = (pldm_numeric_sensor_pdr_t *)(find_pdr->data);

    temp_pdr->thermal_pdr.warning_high = req_dat->upper_threshold_warning;
    temp_pdr->thermal_pdr.critical_high = req_dat->upper_threshold_critical;
    temp_pdr->thermal_pdr.fatal_high = req_dat->upper_threshold_fatal;

    temp_pdr->numeric_pdr_comm_part.hdr.record_change_num++;

    pldm_pdr_chg_event_generate(g_pldm_monitor_info.pldm_event_rbuf, PLDM_REPO_CHG_FORMAT_ID_PDR_HANDLE, PLDM_REPO_CHG_RECORDS_MODIFIED, record_handle);

L_ERR:
    return;
}

static void pldm_monitor_get_sensor_hysteresis(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_get_sensor_hysteresis_req_dat_t *req_dat = (pldm_get_sensor_hysteresis_req_dat_t *)(pkt->req_buf);
    pldm_get_sensor_hysteresis_rsp_dat_t *rsp_dat = (pldm_get_sensor_hysteresis_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    u16 sensor_id = req_dat->sensor_id;
    int is_temp = is_temp_sensor(sensor_id);
    /* Only accepted values are the Temperature Sensor IDs */

    u32 record_handle = sensor_id_convert_to_record_handle(sensor_id);

    if ((is_temp == -1) || (record_handle == PLDM_ERR_RECORD_HANDLE)) {
        rsp_hdr->cpl_code = PLDM_INVALID_SENSOR_ID;
        goto L_ERR;
    }

    pldm_pdr_record_t *find_pdr = pldm_pdr_find(&(g_pldm_monitor_info.pldm_repo), record_handle);

    if (!find_pdr) {
        rsp_hdr->cpl_code = PLDM_INVALID_SENSOR_ID;
        goto L_ERR;
    }

    pldm_numeric_sensor_pdr_t *temp_pdr = (pldm_numeric_sensor_pdr_t *)(find_pdr->data);

    rsp_dat->sensor_datasize = PLDM_DATASIZE_UINT8;
    rsp_dat->hysteresis_value = temp_pdr->thermal_pdr.hysteresis;

    *pkt_len += sizeof(pldm_get_sensor_hysteresis_rsp_dat_t);

L_ERR:
    return;
}

static void pldm_monitor_set_state_sensor_en(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_set_state_sensor_en_req_dat_t *req_dat = (pldm_set_state_sensor_en_req_dat_t *)(pkt->req_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    u16 sensor_id = req_dat->sensor_id;
    pldm_state_data_struct_t *sensor_data = NULL;
    pldm_state_data_struct_t *sensors = NULL;
    u8 cnt;
    u8 real_sensor_count = 0;

    /* supported for the link state, health state, presence, configuration change, thermal Trip, and power consumption sensors */

    if (is_nic_composite_sensor(sensor_id)) {
        sensors = nic_composite_state_sensors;
        real_sensor_count = sizeof(nic_composite_state_sensors) / sizeof(nic_composite_state_sensors[0]);
        cnt = PLDM_CARD_COMPOSITE_STATE_SENSOR_NUM;
    } else if (is_nc_composite_sensor(sensor_id)) {
        sensors = controller_composite_state_sensors;
        real_sensor_count = sizeof(controller_composite_state_sensors) / sizeof(controller_composite_state_sensors[0]);
        cnt = PLDM_NC_STATE_SENSOR_NUM;
    } else if (is_plug_composite_sensor(sensor_id)) {
        sensors = plug_composite_state_sensors;
        real_sensor_count = sizeof(plug_composite_state_sensors) / sizeof(plug_composite_state_sensors[0]) / 4;
        cnt = PLDM_USED_PLUG_COMPOSITE_SENSOR_NUM;
    } else if (is_link_simple_sensor(sensor_id)) {
        sensors = simple_sensors;
        real_sensor_count = sizeof(simple_sensors) / sizeof(simple_sensors[0]) / 4;
        cnt = PLDM_USED_LINK_STATE_SENSOR_NUM;
    } else {
        LOG("Invalid sensor id : %d", sensor_id);
        rsp_hdr->cpl_code = PLDM_INVALID_SENSOR_ID;
        goto L_ERR;
    }

    for (u8 i = 0; i < cnt; i++) {
        if (sensor_id == sensors[i * real_sensor_count].sensor_id) {
            sensor_data = &sensors[i * real_sensor_count];
            break;
        }
    }

    if (sensor_data) {
        if (req_dat->composite_sensor_cnt > real_sensor_count) {
            rsp_hdr->cpl_code = PLDM_ERROR_INVALID_DATA;
            goto L_ERR;
        }

        if (req_dat->composite_sensor_cnt < real_sensor_count) {
            req_dat->composite_sensor_cnt = 1;
        }

        for (u8 i = 0; i < req_dat->composite_sensor_cnt; i++) {
            sensor_data[i].event_msg_en = req_dat->opfield[i].event_msg_en;
            if (sensor_data[i].op_state != req_dat->opfield[i].sensor_op_state) {
                sensor_data[i].previous_op_state = sensor_data[i].op_state;
                sensor_data[i].op_state = req_dat->opfield[i].sensor_op_state;
                pldm_sensor_event_generate(g_pldm_monitor_info.pldm_event_rbuf, sensor_data[i].sensor_event_class, sensor_data[i].event_msg_en, (void *)&sensor_data[i]);
            }
        }
    } else {
        LOG("Invalid sensor id : %d", sensor_id);
        rsp_hdr->cpl_code = PLDM_INVALID_SENSOR_ID;
    }

L_ERR:
    return;
}

static void pldm_monitor_get_state_sensor_reading(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_get_state_sensor_reading_req_dat_t *req_dat = (pldm_get_state_sensor_reading_req_dat_t *)(pkt->req_buf);
    pldm_get_state_sensor_reading_rsp_dat_t *rsp_dat = (pldm_get_state_sensor_reading_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    u16 sensor_id = req_dat->sensor_id;
    pldm_state_data_struct_t *sensor_data = NULL;
    pldm_state_data_struct_t *sensors = NULL;
    u8 cnt = 0;
    u8 real_sensor_count = 0;

    /* supported for all the state sensors (Composite network controller sensor, Composite NIC sensor, Port link sensor, Cable Presence Sensor, Plug Composite Sensor) */

    if (is_nic_composite_sensor(sensor_id)) {
        sensors = nic_composite_state_sensors;
        real_sensor_count = sizeof(nic_composite_state_sensors) / sizeof(nic_composite_state_sensors[0]);
        cnt = PLDM_CARD_COMPOSITE_STATE_SENSOR_NUM;
    } else if (is_nc_composite_sensor(sensor_id)) {
        sensors = controller_composite_state_sensors;
        real_sensor_count = sizeof(controller_composite_state_sensors) / sizeof(controller_composite_state_sensors[0]);
        cnt = PLDM_NC_STATE_SENSOR_NUM;
    } else if (is_plug_composite_sensor(sensor_id)) {
        sensors = plug_composite_state_sensors;
        real_sensor_count = sizeof(plug_composite_state_sensors) / sizeof(plug_composite_state_sensors[0]) / 4;
        cnt = PLDM_USED_PLUG_COMPOSITE_SENSOR_NUM;
    } else if (is_link_simple_sensor(sensor_id)) {
        sensors = simple_sensors;
        real_sensor_count = sizeof(simple_sensors) / sizeof(simple_sensors[0]) / 4;
        cnt = PLDM_USED_LINK_STATE_SENSOR_NUM;
    } else {
        LOG("Invalid sensor id : %d", sensor_id);
        rsp_hdr->cpl_code = PLDM_INVALID_SENSOR_ID;
        goto L_ERR;
    }

    for (int i = 0; i < cnt; i++) {
        if (sensor_id == sensors[i * real_sensor_count].sensor_id) {
            sensor_data = &sensors[i * real_sensor_count];
            break;
        }
    }

    if (sensor_data) {
        rsp_dat->composite_sensor_cnt = real_sensor_count;
        for (int i = 0; i < rsp_dat->composite_sensor_cnt; i++) {
            rsp_dat->opfield[i].sensor_op_state = sensor_data[i].op_state;
            rsp_dat->opfield[i].present_state = sensor_data[i].present_state;
            rsp_dat->opfield[i].previous_state = sensor_data[i].previous_state;
            rsp_dat->opfield[i].event_state = sensor_data[i].present_state;
        }

        *pkt_len += sizeof(pldm_get_state_sensor_reading_rsp_dat_t) + sizeof(pldm_get_state_sensor_reading_composite_sensor_cnt_opfield_t) * rsp_dat->composite_sensor_cnt;
    } else {
        LOG("Invalid sensor id : %d", sensor_id);
        rsp_hdr->cpl_code = PLDM_INVALID_SENSOR_ID;
    }

L_ERR:
    return;
}

static void pldm_monitor_get_pdr_repo_info(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_get_pdr_repo_info_rsp_dat_t *rsp_dat = (pldm_get_pdr_repo_info_rsp_dat_t *)(pkt->rsp_buf);

    rsp_dat->repo_state = g_pldm_monitor_info.repo_state;
    rsp_dat->update_time = g_pldm_monitor_info.pldm_repo.update_time;                                  /* Returns the time in internal clock of the firmware of the last PDR update */
    rsp_dat->oem_update_time = 0x00;                                                                  /* Currently no OEM PDRs are defined, so return 0 */
    rsp_dat->record_count = g_pldm_monitor_info.pldm_repo.record_count;
    /* Return the size of the entire PDR data structure rounded up to 1K */
    rsp_dat->repo_size = (g_pldm_monitor_info.pldm_repo.size < 0x400) ? 0x400 : ((g_pldm_monitor_info.pldm_repo.size + 512) & (0xFFFFFC00));

    /* An implementation is allowed to round this number of up to the nearest 64-byte increment. */
    rsp_dat->largest_record_size = g_pldm_monitor_info.pldm_repo.largest_pdr_size;                    /* max byte is 153 bytes, */
    rsp_dat->data_transfer_handle_timeout = PLDM_REPO_DEFAULT_MINIMUM_TIMEOUT;                        /* default timeout of 30 seconds */

    LOG("record_count : %d", rsp_dat->record_count);

    *pkt_len += sizeof(pldm_get_pdr_repo_info_rsp_dat_t);
}

static void pldm_monitor_get_pdr(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_get_pdr_req_dat_t *req_dat = (pldm_get_pdr_req_dat_t *)(pkt->req_buf);
    pldm_get_pdr_rsp_dat_t *rsp_dat = (pldm_get_pdr_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    if (g_pldm_monitor_info.repo_state == PLDM_REPO_UPDATE_IN_PROGRESS) {
        rsp_hdr->cpl_code = PLDM_REPOSITORY_UPDATE_IN_PROGRESS;
        goto L_ERR;
    }

    if (gs_pdr_transfer_flag == PLDM_TRANSFER_FLAG_START_AND_END || gs_pdr_transfer_flag == PLDM_TRANSFER_FLAG_END) {
        gs_find_pdr = pldm_pdr_find(&(g_pldm_monitor_info.pldm_repo), req_dat->record_handle);
        if (!gs_find_pdr) {
            rsp_hdr->cpl_code = PLDM_INVALID_RECORD_HANDLE;
            goto L_ERR;
        }
        gs_find_pdr_comm_hdr = (pldm_pdr_hdr_t *)(gs_find_pdr->data);
    }

    if (gs_find_pdr_comm_hdr->record_handle != req_dat->record_handle) {
        rsp_hdr->cpl_code = PLDM_INVALID_RECORD_HANDLE;
        goto L_ERR;
    }

    if (req_dat->data_transfer_handle != gs_pdr_previous_data_transfer_handle) {
        rsp_hdr->cpl_code = PLDM_INVALID_DATA_TRANSFER_HANDLE;
        goto L_ERR;
    }

    u8 state = (req_dat->transfer_op_state == PLDM_TRANSFER_OP_FLAG_GET_NEXT_PART && req_dat->record_chg_num != gs_find_pdr_comm_hdr->record_change_num);
    state |= (req_dat->transfer_op_state == PLDM_TRANSFER_OP_FLAG_GET_FIRST_PART && req_dat->record_chg_num != 0x0000);
    if (state) {
        rsp_hdr->cpl_code = PLDM_INVALID_RECORD_CHANGE_NUMBER;
        goto L_ERR;
    }

    rsp_dat->rsp_cnt = (req_dat->req_cnt >= (gs_find_pdr->size - req_dat->data_transfer_handle)) ? (gs_find_pdr->size - req_dat->data_transfer_handle) : req_dat->req_cnt;
    if (req_dat->req_cnt >= (gs_find_pdr->size - req_dat->data_transfer_handle)) {
        if (req_dat->transfer_op_state == PLDM_TRANSFER_OP_FLAG_GET_FIRST_PART) {
            rsp_dat->transfer_flag = PLDM_TRANSFER_FLAG_START_AND_END;
        } else {
            rsp_dat->transfer_flag = PLDM_TRANSFER_FLAG_END;
            rsp_dat->req_data[rsp_dat->rsp_cnt] = crc8_pldm(gs_find_pdr->data, gs_find_pdr->size);
            *pkt_len += sizeof(u8);
        }
        rsp_dat->next_record_handle = (!gs_find_pdr->next) ? 0x00000000 : gs_find_pdr->next->record_handle;
        rsp_dat->next_data_transfer_handle = 0x00000000;            /*  returns 0x0000_0000 if there is no remaining data */
    } else {
        if (req_dat->transfer_op_state == PLDM_TRANSFER_OP_FLAG_GET_FIRST_PART) {
            rsp_dat->transfer_flag = PLDM_TRANSFER_FLAG_START;
        } else {
            rsp_dat->transfer_flag = PLDM_TRANSFER_FLAG_MIDDLE;
        }
        rsp_dat->next_record_handle = req_dat->record_handle;
        rsp_dat->next_data_transfer_handle = (req_dat->data_transfer_handle + rsp_dat->rsp_cnt);
    }
    cm_memcpy(rsp_dat->req_data, (gs_find_pdr->data + req_dat->data_transfer_handle), rsp_dat->rsp_cnt);
    gs_pdr_transfer_flag = rsp_dat->transfer_flag;
    gs_pdr_previous_data_transfer_handle = rsp_dat->next_data_transfer_handle;
    *pkt_len += sizeof(pldm_get_pdr_rsp_dat_t) + rsp_dat->rsp_cnt;
L_ERR:
    LOG("transfer_flag : %d, cpl code : %#x, req_dat->record_handle : %#x, rsp_cnt : %d", rsp_dat->transfer_flag, rsp_hdr->cpl_code, req_dat->record_handle, rsp_dat->rsp_cnt);
    return;
}

static void pldm_monitor_get_pdr_repo_signature(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_get_pdr_repo_signature_rsp_dat_t *rsp_dat = (pldm_get_pdr_repo_signature_rsp_dat_t *)(pkt->rsp_buf);

    /* This is a 32-bit value and remains persistent unless a change is detected in any record of the PDR repository.
       This field should be computed once (during device boot) as low 32 bits of SHA256 of PDRs.(E810) */
    rsp_dat->pdr_repo_signature = g_pldm_monitor_info.pldm_repo.repo_signature;               /* 待定 */

    *pkt_len += sizeof(pldm_get_pdr_repo_signature_rsp_dat_t);
}

static pldm_cmd_func pldm_cmd_table[PLDM_MONITOR_CMD] =
{
    pldm_unsupport_cmd,                                 /* 0x00 */
    pldm_ctrl_set_tid,                                  /* 0x01 */
    pldm_ctrl_get_tid,                                  /* 0x02 */
    pldm_monitor_get_uuid,                              /* 0x03 */
    pldm_monitor_set_event_receiver,                    /* 0x04 */
    pldm_monitor_get_event_receiver,                    /* 0x05 */
    pldm_unsupport_cmd,                                 /* 0x00 */
    pldm_unsupport_cmd,                                 /* 0x00 */
    pldm_unsupport_cmd,                                 /* 0x00 */
    pldm_unsupport_cmd,                                 /* 0x00 */
    pldm_monitor_platform_event_msg_rsp,                /* 0x0a */
    pldm_monitor_poll_for_platform_event_msg,           /* 0x0b */
    pldm_monitor_event_msg_supported,                   /* 0x0c */
    pldm_monitor_event_msg_buffersize,                  /* 0x0d */
    pldm_unsupport_cmd,                                 /* 0x00 */
    pldm_unsupport_cmd,                                 /* 0x00 */
    pldm_monitor_set_numeric_sensor_enable,             /* 0x10 */
    pldm_monitor_get_sensor_reading,                    /* 0x11 */
    pldm_monitor_get_sensor_thresholds,                 /* 0x12 */
    pldm_monitor_set_sensor_thresholds,                 /* 0x13 */
    pldm_unsupport_cmd,                                 /* 0x00 */
    pldm_monitor_get_sensor_hysteresis,                 /* 0x15 */
    // pldm_monitor_set_state_sensor_en,                   /* 0x20 */
    // pldm_monitor_get_state_sensor_reading,              /* 0x21 */
    // pldm_monitor_get_pdr_repo_info,                     /* 0x50 */
    // pldm_monitor_get_pdr,                               /* 0x51 */
    // pldm_monitor_get_pdr_repo_signature                 /* 0x53 */
};

/* polynomial x8 + x2 + x1 + 1 */
u8 crc8_pldm(u8 *data, u16 len)
{
    u8 crc8 = 0;
    while(len--){
        crc8 ^= *data++;
        for (int i = 0; i < 8; ++i) {
            crc8 = ( crc8 & 0x80 ) ? (crc8 << 1) ^ 0x07 : (crc8 << 1);
        }
    }
    return crc8;
}

void pldm_monitor_update_repo_signature(pldm_pdr_t *repo)
{
    if (!repo) return;
    pldm_pdr_record_t *pdr = repo->first;
    repo->repo_signature = 0;
    while (pdr) {
        repo->repo_signature = crc32_pldm(repo->repo_signature ^ 0xFFFFFFFFUL, pdr->data, pdr->size);
        pdr = pdr->next;
    }
}

static void pldm_monitor_base_info_init(pldm_monitor_base_info_t *pldm_monitor_base_info)
{
    pldm_monitor_base_info->tid = 0;
    pldm_monitor_base_info->phy_addr.val = 0;
    pldm_monitor_base_info->repo_state = PLDM_REPO_AVAILABLE;
    pldm_monitor_base_info->event_receiver_eid = 0;
    pldm_monitor_base_info->terminus_mode = PLDM_DISABLE;
    pldm_monitor_base_info->terminus_max_buffersize = 24;     /* default size. */
    pldm_monitor_base_info->pldm_event_rbuf = pldm_event_rbuf_init();
    pdrs_pool_init((u32 *)gs_pdrs_buf);
    pldm_pdr_init(&(pldm_monitor_base_info->pldm_repo));
}

void pldm_monitor_init(void)
{
    pldm_monitor_base_info_init(&g_pldm_monitor_info);
    g_pldm_monitor_info.terminus_mode = PLDM_ENABLE_ASYNC;
    g_pldm_monitor_info.event_receiver_eid = 0x66;
    g_pldm_monitor_info.tid = 0x67;

    pldm_assoc_pdr_init();
    pldm_terminus_locator_pdr_init();
    pldm_numeric_sensor_pdr_init();
    pldm_state_sensor_pdr_init();
    pldm_redfish_pdr_init();

    // g_pldm_monitor_info.pldm_repo.update_time =
    pldm_monitor_update_repo_signature(&(g_pldm_monitor_info.pldm_repo));
    for (u8 i = 0; i < MAX_LAN_NUM; i++) {
        pldm_link_handle(i, 1);
    }
}

void pldm_monitor_process(protocol_msg_t *pkt, int *pkt_len, u32 cmd_code)
{
    pldm_cmd_func cmd_proc = NULL;

    if (cmd_code < PLDM_MONITOR_CMD) {
        cmd_proc = pldm_cmd_table[cmd_code];
    } else if (cmd_code == 0x20) {
        cmd_proc = pldm_monitor_set_state_sensor_en;
    } else if (cmd_code == 0x21) {
        cmd_proc = pldm_monitor_get_state_sensor_reading;
    } else if (cmd_code == 0x50) {
        cmd_proc = pldm_monitor_get_pdr_repo_info;
    } else if (cmd_code == 0x51) {
        cmd_proc = pldm_monitor_get_pdr;
    } else if (cmd_code == 0x53) {
        cmd_proc = pldm_monitor_get_pdr_repo_signature;
    } else {
        cmd_proc = pldm_unsupport_cmd;
    }

    cmd_proc(pkt, pkt_len);
}

void pldm_temp_monitor_handle(void)
{
    for (u8 i = 0; i < (PLDM_NIC_TEMP_SENSOR_NUM + PLDM_NC_TEMP_SENSOR_NUM + PLDM_USED_PLUG_TEMP_SENSOR_NUM); i++) {
        if (temp_sensor_monitor[i].pdr_addr != NULL) {
            pldm_numeric_sensor_pdr_t *temp_pdr = (pldm_numeric_sensor_pdr_t *)(temp_sensor_monitor[i].pdr_addr);
            pldm_data_struct_t *sensor_data = NULL;

            u8 present_state = UNKNOWN;
            u16 sensor_id = temp_sensor_monitor[i].sensor_id;
            int is_temp = is_temp_sensor(sensor_id);
            u8 cnt = MAX_LAN_NUM;
            if (is_temp == -1) continue;

            for (u8 j = 0; j < cnt; j++) {
                if (sensor_id == temp_sensors[is_temp][j].sensor_id) {
                    sensor_data = &temp_sensors[is_temp][j];
                    cnt = 0;
                    break;
                }
            }

            if (cnt != 0) {
                LOG("temp sensor not find. sensor id : %d", sensor_id);
                continue;
            }
            if (sensor_data->present_reading < temp_pdr->thermal_pdr.warning_high) {
                present_state = NORMAL;
            }else if (sensor_data->present_reading < temp_pdr->thermal_pdr.critical_high) {
                present_state = TEMP_UPPER_WARNING;
            } else if (sensor_data->present_reading < temp_pdr->thermal_pdr.fatal_high) {
                present_state = TEMP_UPPER_CRITICAL;
            } else {
                present_state = TEMP_UPPER_FATAL;
            }

            if (sensor_data->present_state == present_state) {
                continue;
            }
            sensor_data->previous_state = sensor_data->present_state;
            sensor_data->present_state = present_state;
            pldm_sensor_event_generate(g_pldm_monitor_info.pldm_event_rbuf, sensor_data->sensor_event_class, sensor_data->event_msg_en, (void *)sensor_data);
        }
    }
}

void pldm_event_send_handle(void)
{
    if (g_pldm_monitor_info.terminus_mode == PLDM_ENABLE_POLLING)
        return;

    if (pldm_event_rbuf_is_empty(g_pldm_monitor_info.pldm_event_rbuf)) {
        g_event_id = 1;                                                                              /* event id reset */
        return;
    }

    u8 event_data_info[sizeof(pldm_event_data_t)] = {0};
    u8 paylaod[128];
    pldm_event_rbuf_try_read(g_pldm_monitor_info.pldm_event_rbuf, event_data_info, sizeof(pldm_event_data_t), 0);
    pldm_event_data_t *pldm_event_data = (pldm_event_data_t *)event_data_info;                       /* the oldest event */
    u8 event_cnt = g_event_id - pldm_event_data->event_id;

    if (g_pldm_monitor_info.event_receiver_eid) {
        u8 msg_len = 0;
        if ((event_cnt > 10) || (pldm_event_data->event_data_size > (g_pldm_monitor_info.terminus_max_buffersize - PLDM_ASYNC_SEND_FIELD_LEN))) {                                                                        /* pldm msg poll event */
            g_pldm_monitor_info.terminus_mode = PLDM_ENABLE_POLLING;                                   /* enter poll state, exit state is in poll msg cmd */
            pldm_platform_event_msg_req_dat_t *msg_ev_req_dat = (pldm_platform_event_msg_req_dat_t *)paylaod;
            pldm_pldm_msg_poll_event_data_format_t *msg_poll_event_dat = (pldm_pldm_msg_poll_event_data_format_t *)(msg_ev_req_dat->event_data);

            msg_ev_req_dat->format_ver = PLDM_EVENT_FORMAT_VERSION;
            msg_ev_req_dat->tid = g_pldm_monitor_info.tid;
            msg_ev_req_dat->event_class = PLDM_MSG_POLL_EVENT;

            msg_poll_event_dat->format_ver = PLDM_EVENT_FORMAT_VERSION;
            msg_poll_event_dat->data_transfer_handle = 0x00000000;
            msg_poll_event_dat->event_id = pldm_event_data->event_id;
            msg_len = sizeof(pldm_pldm_msg_poll_event_data_format_t) + sizeof(pldm_platform_event_msg_req_dat_t);
        } else {                                                                                     /* have some event */
            pldm_platform_event_msg_req_dat_t *msg_ev_req_dat = (pldm_platform_event_msg_req_dat_t *)paylaod;

            msg_ev_req_dat->format_ver = PLDM_EVENT_FORMAT_VERSION;
            msg_ev_req_dat->tid = g_pldm_monitor_info.tid;
            msg_ev_req_dat->event_class = pldm_event_data->event_class;

            pldm_event_rbuf_try_read(g_pldm_monitor_info.pldm_event_rbuf, msg_ev_req_dat->event_data, pldm_event_data->event_data_size, sizeof(pldm_event_data_t));
            msg_len = pldm_event_data->event_data_size + sizeof(pldm_platform_event_msg_req_dat_t);
        }
        pldm_msg_send(paylaod, msg_len, MCTP_PLDM_MONITOR, 0x0a, g_pldm_monitor_info.hw_id, g_pldm_monitor_info.event_receiver_eid);
    }
}