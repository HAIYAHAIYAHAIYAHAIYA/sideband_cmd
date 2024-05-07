#include "pldm_redfish_gen.h"
#include "pldm_monitor.h"
#include "pldm_cjson.h"
#include "pldm_bej_resolve.h"
#include "pkt_gen.h"
#include "pldm_cjson.h"

pldm_gen_state_t gs_pldm_redfish_gen_state;
pldm_redfish_op_identify_t g_cur_op_identify;

u8 dict_etag[14][9];

static int dict_resource_id[] = {
        PLDM_BASE_NETWORK_ADAPTER_RESOURCE_ID,
        PLDM_BASE_PCIE_DEV_RESOURCE_ID,
        PLDM_BASE_NETWORK_INTERFACE_RESOURCE_ID,
        PLDM_BASE_PORTS_RESOURCE_ID,
        PLDM_BASE_NETWORK_DEV_FUNCS_RESOURCE_ID,
        PLDM_BASE_PCIE_FUNCS_RESOURCE_ID,
        PLDM_BASE_ETH_INTERFACE_COLLECTION_RESOURCE_ID,
        PLDM_BASE_PORT_RESOURCE_ID + 1,
        PLDM_BASE_NETWORK_DEV_FUNC_RESOURCE_ID + 1,
        PLDM_BASE_PCIE_FUNC_RESOURCE_ID + 1,
        PLDM_BASE_ETH_INTERFACE_RESOURCE_ID + 1,
        PLDM_BASE_ANNOTATION_DICT_RESOURCE_ID,
        PLDM_BASE_EVENT_DICT_RESOURCE_ID,
        PLDM_BASE_REGISTER_DICT_RESOURCE_ID
    };
static int dict_schemaclass[] = {
        SCHEMACLASS_MAJOR,
        SCHEMACLASS_MAJOR,
        SCHEMACLASS_MAJOR,
        SCHEMACLASS_MAJOR,
        SCHEMACLASS_MAJOR,
        SCHEMACLASS_MAJOR,
        SCHEMACLASS_MAJOR,
        SCHEMACLASS_MAJOR,
        SCHEMACLASS_MAJOR,
        SCHEMACLASS_MAJOR,
        SCHEMACLASS_MAJOR,
        SCHEMACLASS_ANNOTATION,
        SCHEMACLASS_EVENT,
        SCHEMACLASS_REGISTRY
    };
static void pldm_redfish_gen_cmd_01(u8 *buf)
{
    pldm_redfish_negotiate_redfish_parameters_req_dat_t *req_dat = (pldm_redfish_negotiate_redfish_parameters_req_dat_t *)buf;
    req_dat->mc_concurrency_support = 1;
    req_dat->mc_feature_support |= CBIT(8);
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_01;
}

static void pldm_redfish_gen_cmd_02(u8 *buf)
{
    pldm_redfish_negotiate_medium_parameters_req_dat_t *req_dat = (pldm_redfish_negotiate_medium_parameters_req_dat_t *)buf;
    req_dat->mc_maximum_xfer_chunksize_bytes = PLDM_REDFISH_XFER_CHUNKSIZE;
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_02;
}

static u8 idx = 0;
static void pldm_redfish_gen_cmd_03(u8 *buf)
{
    pldm_redfish_get_schema_dictionary_req_dat_t *req_dat = (pldm_redfish_get_schema_dictionary_req_dat_t *)buf;
    req_dat->resource_id = dict_resource_id[idx];
    g_cur_op_identify.op_id = 0;
    req_dat->requested_schemaclass = dict_schemaclass[idx++];
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_03;
    if (idx >= PLDM_REDFISH_DICT_NUM) idx = 0;
}

static void pldm_redfish_gen_cmd_04(u8 *buf)
{
    pldm_redfish_get_schema_uri_req_dat_t *req_dat = (pldm_redfish_get_schema_uri_req_dat_t *)buf;
    req_dat->resource_id = dict_resource_id[idx];
    req_dat->requested_schemaclass = dict_schemaclass[idx++];;

    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_04;
    if (idx >= PLDM_REDFISH_DICT_NUM) idx = 0;
}

static void pldm_redfish_gen_cmd_05(u8 *buf)
{
    pldm_redfish_get_resource_etag_req_dat_t *req_dat = (pldm_redfish_get_resource_etag_req_dat_t *)buf;
    g_cur_op_identify.resource_id = req_dat->resource_id = dict_resource_id[idx++];
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_05;
    if (idx >= PLDM_REDFISH_DICT_NUM) idx = 0;
    LOG("idx : %d", idx);
}

static void pldm_redfish_gen_cmd_06(u8 *buf)
{
    pldm_redfish_get_oem_count_req_dat_t *req_dat = (pldm_redfish_get_oem_count_req_dat_t *)buf;
    req_dat->resource_id = dict_resource_id[idx++];
    req_dat->requested_schemaclass = SCHEMACLASS_MAJOR;
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_06;
    if (idx >= PLDM_REDFISH_DICT_NUM) idx = 0;
}

static void pldm_redfish_gen_cmd_07(u8 *buf)
{
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_07;
}

static void pldm_redfish_gen_cmd_08(u8 *buf)
{
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_08;
}

static void pldm_redfish_gen_cmd_09(u8 *buf)
{
    pldm_redfish_get_registry_details_req_dat_t *req_dat = (pldm_redfish_get_registry_details_req_dat_t *)buf;
    req_dat->registry_idx = 0;
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_09;
}

static void pldm_redfish_gen_cmd_0b(u8 *buf)
{
    pldm_redfish_get_msg_registry_req_dat_t *req_dat = (pldm_redfish_get_msg_registry_req_dat_t *)buf;
    req_dat->registry_idx = 0;
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_0b;
}

pldm_payload_dat_t g_send_buf;
extern u8 g_dict_info[PLDM_REDFISH_DICT_INFO_LEN];
extern u8 g_anno_dict[PLDM_REDFISH_ANNO_DICT_LEN];
extern u8 g_needed_dict[PLDM_REDFISH_PORT_DICT_LEN];
extern schema_create g_schemas_update[11];
extern u16 pldm_redfish_get_dict_len(u32 resource_id);
extern u8 resource_id_to_resource_identity(u32 resource_id);
static void pldm_redfish_gen_cmd_10(u8 *buf)
{
    // if (g_cur_op_identify.resource_id == 0) {
        g_cur_op_identify.resource_id = dict_resource_id[idx];
        // g_cur_op_identify.resource_id = PLDM_BASE_PCIE_DEV_RESOURCE_ID;
        g_cur_op_identify.op_id = idx | CBIT(15);
        g_cur_op_identify.op_flg = CBIT(1);
        idx++;
        if (idx >= PLDM_REDFISH_DICT_NUM) idx = 0;
        LOG("idx : %d", idx);
    // }

    pldm_redfish_rde_operation_init_req_dat_t *req_dat = (pldm_redfish_rde_operation_init_req_dat_t *)buf;
    req_dat->op_type = REPLACE;
    req_dat->op_flg = g_cur_op_identify.op_flg;
    req_dat->op_identify = g_cur_op_identify;
    req_dat->op_locator_len = 0;
    req_dat->senddata_transfer_handle = 0;
    req_dat->req_payload_len = 0;

    if (g_cur_op_identify.op_flg & CBIT(1)) {
        pldm_cjson_t *root = NULL;
        u8 *anno_dict = &g_anno_dict[DICT_FMT_HDR_LEN];
        u8 *dict = &g_needed_dict[DICT_FMT_HDR_LEN];
        u8 ret = pldm_redfish_get_dict_data(g_cur_op_identify.resource_id, SCHEMACLASS_MAJOR,\
        g_needed_dict, pldm_redfish_get_dict_len(g_cur_op_identify.resource_id));
        if (ret == false) LOG("%s, pldm_redfish_get_dict_data err.", __FUNCTION__);

        u8 resourse_indenty = resource_id_to_resource_identity(g_cur_op_identify.resource_id);
        resourse_indenty -= NETWORK_ADAPTER;

        root = g_schemas_update[resourse_indenty](g_cur_op_identify.resource_id);
        pldm_cjson_cal_sf_to_root(root, anno_dict, dict);
        pldm_cjson_cal_len_to_root(root, req_dat->op_type);
        // pldm_cjson_printf_root(root);

        bejencoding_t *ptr = (bejencoding_t *)g_send_buf.data;
        pldm_redfish_dictionary_format_t *dict_ptr = (pldm_redfish_dictionary_format_t *)dict;
        ptr->ver = dict_ptr->schema_version;
        ptr->schema_class = SCHEMACLASS_MAJOR;
        u8 *end_ptr = pldm_bej_encode(root, &(g_send_buf.data[sizeof(bejencoding_t)]));
        g_send_buf.len = end_ptr - g_send_buf.data;
        LOG("g_send_buf.len : %d", g_send_buf.len);
        root = NULL;
        pldm_cjson_pool_reinit();
        u32 max_pkt_len = PLDM_REDFISH_XFER_CHUNKSIZE - PLDM_OP_INIT_FIELD_LEN;
        req_dat->req_payload_len = g_send_buf.len <= max_pkt_len ? g_send_buf.len : 0;
        u8 *payload = &(req_dat->op_locator[req_dat->op_locator_len]);
        if (req_dat->req_payload_len) {
            cm_memcpy(payload, g_send_buf.data, g_send_buf.len);
        }
    }

    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_10;
}

static void pldm_redfish_gen_cmd_11(u8 *buf)
{
    pldm_gen_req_hdr_update(buf, 0x11);
    pldm_redfish_supply_custom_request_parameters_req_dat_t *req_dat = (pldm_redfish_supply_custom_request_parameters_req_dat_t *)buf;
    req_dat->op_identify = g_cur_op_identify;
    req_dat->etag_op = ETAG_IF_MATCH;
    req_dat->etag_cnt = 1;
    req_dat->collection_skip = 0;
    req_dat->collection_top = 1;
    req_dat->etag.format = UTF_8;
    req_dat->etag.len = 9;
    u8 resourse_indenty = resource_id_to_resource_identity(g_cur_op_identify.resource_id);
    resourse_indenty -= NETWORK_ADAPTER;
    cm_memcpy(req_dat->etag.val, dict_etag[resourse_indenty], req_dat->etag.len);
    LOG("Send Etag : %s", req_dat->etag.val);
}

static void pldm_redfish_gen_cmd_12(u8 *buf)
{

}

static void pldm_redfish_gen_cmd_13(u8 *buf)
{
    pldm_redfish_rde_operation_complete_req_dat_t *req_dat = (pldm_redfish_rde_operation_complete_req_dat_t *)buf;
    req_dat->op_identify = g_cur_op_identify;
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_13;
}

static void pldm_redfish_gen_cmd_14(u8 *buf)
{
    pldm_redfish_rde_operation_status_req_dat_t *req_dat = (pldm_redfish_rde_operation_status_req_dat_t *)buf;
    req_dat->op_identify = g_cur_op_identify;
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_14;
}

static void pldm_redfish_gen_cmd_15(u8 *buf)
{
    pldm_redfish_rde_operation_kill_req_dat_t *req_dat = (pldm_redfish_rde_operation_kill_req_dat_t *)buf;
    req_dat->op_identify = g_cur_op_identify;
    req_dat->kill_flg = 0;

    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_15;
}

static void pldm_redfish_gen_cmd_16(u8 *buf)
{

}

static u32 data_hdl = 0;
extern u32 crc32_pldm(u32 init_crc, u8 *data, u32 len);
static void pldm_redfish_gen_cmd_30(u8 *buf)
{
    pldm_gen_req_hdr_update(buf, 0x30);
    pldm_redfish_rde_multipart_send_req_dat_t *req_dat = (pldm_redfish_rde_multipart_send_req_dat_t *)buf;
    req_dat->op_id = g_cur_op_identify.op_id;
    req_dat->data_transfer_handle = data_hdl;
    u32 max_pkt_len = PLDM_REDFISH_XFER_CHUNKSIZE - PLDM_MULTI_SEND_FIELD_LEN;
    u32 remain_bytes = g_send_buf.len - data_hdl;
    u32 cpy_len = (remain_bytes + 4) <= max_pkt_len ? remain_bytes : max_pkt_len;
    if (remain_bytes < max_pkt_len && (remain_bytes + 4) > max_pkt_len) {
        cpy_len = remain_bytes;
    }
    req_dat->data_len_bytes = cpy_len;

    if ((remain_bytes + 4) <= max_pkt_len) {
        if (!data_hdl) {
            req_dat->transfer_flg = PLDM_REDFISH_TRANSFER_START_END;
        } else {
            req_dat->transfer_flg = PLDM_TRANSFER_FLG_END;
        }
        req_dat->data_len_bytes += 4;
        u32 crc32 = crc32_pldm(0xFFFFFFFFUL, g_send_buf.data, g_send_buf.len);
        cm_memcpy(&(req_dat->data[cpy_len]), &crc32, sizeof(crc32));
    } else {
        if (!data_hdl) {
            req_dat->transfer_flg = PLDM_REDFISH_TRANSFER_START;
        } else {
            req_dat->transfer_flg = PLDM_TRANSFER_FLG_MIDDLE;
        }
        req_dat->next_transfer_handle = data_hdl + cpy_len;
    }
    cm_memcpy(req_dat->data, &(g_send_buf.data[data_hdl]), cpy_len);
    data_hdl += cpy_len;
    if ((remain_bytes + 4) <= max_pkt_len)
        data_hdl = 0;
    LOG("%s, send data part : %d", __FUNCTION__, cpy_len);
}

static void pldm_redfish_gen_cmd_31(u8 *buf)
{
    pldm_gen_req_hdr_update(buf, 0x31);
    pldm_redfish_rde_multipart_receive_req_dat_t rsp_dat = pldm_redfish_get_multi_recv_rsp_dat();
    pldm_redfish_rde_multipart_receive_req_dat_t *req_dat = (pldm_redfish_rde_multipart_receive_req_dat_t *)buf;
    *req_dat = rsp_dat;
    req_dat->op_id = g_cur_op_identify.op_id;
    req_dat->transfer_op = !(rsp_dat.data_transfer_handle) ? XFER_FIRST_PART : XFER_NEXT_PART;
}

void pldm_redfish_gen_init(void)
{
    g_cur_op_identify.resource_id = 0;

    gs_pldm_redfish_gen_state.cur_state = PLDM_REDFISH_GEN_IDLE;
    gs_pldm_redfish_gen_state.prev_state = PLDM_REDFISH_GEN_IDLE;
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_UNKNOW;
}

pkt_gen_state_transform_t pldm_redfish_state_transform[] = {
    {PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_GEN_ENTER_CMD_01,    PLDM_REDFISH_GEN_IDLE, NULL},
    {PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_GEN_ENTER_CMD_02,    PLDM_REDFISH_GEN_IDLE, NULL},
    {PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_GEN_ENTER_CMD_03,    PLDM_REDFISH_GEN_IDLE, NULL},
    {PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_GEN_ENTER_CMD_04,    PLDM_REDFISH_GEN_IDLE, NULL},
    {PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_GEN_ENTER_CMD_05,    PLDM_REDFISH_GEN_IDLE, NULL},
    {PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_GEN_ENTER_CMD_06,    PLDM_REDFISH_GEN_IDLE, NULL},
    {PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_GEN_ENTER_CMD_07,    PLDM_REDFISH_GEN_IDLE, NULL},
    {PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_GEN_ENTER_CMD_08,    PLDM_REDFISH_GEN_IDLE, NULL},
    {PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_GEN_ENTER_CMD_09,    PLDM_REDFISH_GEN_IDLE, NULL},
    {PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_GEN_ENTER_CMD_0b,    PLDM_REDFISH_GEN_IDLE, NULL},

    {PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_GEN_NEED_MULTI_RECV, PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_CMD_GEN(31)},
    {PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_GEN_NEED_MULTI_SEND, PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_CMD_GEN(30)},

    {PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_GEN_ENTER_CMD_10,    PLDM_REDFISH_GEN_IDLE, NULL},
    {PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_GEN_ENTER_CMD_11,    PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_CMD_GEN(11)},

    {PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_GEN_ENTER_CMD_14,    PLDM_REDFISH_GEN_IDLE, NULL},
    {PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_GEN_ENTER_CMD_13,    PLDM_REDFISH_GEN_IDLE, NULL},
    {PLDM_REDFISH_GEN_IDLE, PLDM_REDFISH_GEN_ENTER_CMD_15,    PLDM_REDFISH_GEN_IDLE, NULL},
};

void pldm_redfish_gen(int cmd, u8 *buf)
{
    gen_cmd cmds[] = {
        gen_cmd_unsupport,
        pldm_redfish_gen_cmd_01,
        pldm_redfish_gen_cmd_02,
        pldm_redfish_gen_cmd_03,
        pldm_redfish_gen_cmd_04,
        pldm_redfish_gen_cmd_05,
        pldm_redfish_gen_cmd_06,
        pldm_redfish_gen_cmd_07,
        pldm_redfish_gen_cmd_08,
        pldm_redfish_gen_cmd_09,
        gen_cmd_unsupport,
        pldm_redfish_gen_cmd_0b,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        pldm_redfish_gen_cmd_10,
        pldm_redfish_gen_cmd_11,
        pldm_redfish_gen_cmd_12,
        pldm_redfish_gen_cmd_13,
        pldm_redfish_gen_cmd_14,
        pldm_redfish_gen_cmd_15,
        pldm_redfish_gen_cmd_16,
        // pldm_redfish_gen_cmd_30,
        // pldm_redfish_gen_cmd_31,
    };

    if (cmd < PLDM_REDFISH_CMD) {
        if (cmds[cmd])
            cmds[cmd](buf);
    } else if (cmd == 0x30) {
        pldm_redfish_gen_cmd_30(buf);
    } else if (cmd == 0x31) {
        pldm_redfish_gen_cmd_31(buf);
    } else {
        gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_UNKNOW;
        gen_cmd_unsupport(buf);
    }
}

u8 pldm_redfish_state_transform_switch(u8 cnt, u8 *buf)
{
    u8 ret = 0xFF;
    // LOG("gs_pldm_redfish_gen_state.event_id : %d", gs_pldm_redfish_gen_state.event_id);
    for (u8 i = 0; i < sizeof(pldm_redfish_state_transform) / sizeof(pkt_gen_state_transform_t); i++) {
        if (gs_pldm_redfish_gen_state.cur_state == pldm_redfish_state_transform[i].cur_state && gs_pldm_redfish_gen_state.event_id == pldm_redfish_state_transform[i].event_id) {
            gs_pldm_redfish_gen_state.prev_state = gs_pldm_redfish_gen_state.cur_state;
            gs_pldm_redfish_gen_state.cur_state = pldm_redfish_state_transform[i].next_state;
            LOG("gs_pldm_redfish_gen prev state : %d, cur state : %d, event id : %d", gs_pldm_redfish_gen_state.prev_state, gs_pldm_redfish_gen_state.cur_state, gs_pldm_redfish_gen_state.event_id);  /* for debug */

            if (gs_pldm_redfish_gen_state.event_id == PLDM_REDFISH_GEN_NEED_MULTI_RECV)
                gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_UNKNOW;
            if (gs_pldm_redfish_gen_state.event_id == PLDM_REDFISH_GEN_NEED_MULTI_SEND)
                gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_UNKNOW;
            if (pldm_redfish_state_transform[i].action != NULL) {
                pldm_redfish_state_transform[i].action(buf);
            }
            ret = 1;
            break;
        }
    }
    return ret;
}