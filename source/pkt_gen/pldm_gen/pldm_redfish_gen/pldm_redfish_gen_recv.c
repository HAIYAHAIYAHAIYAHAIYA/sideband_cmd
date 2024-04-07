#include "pldm_redfish_gen.h"
#include "pkt_gen.h"
#include "pldm_cjson.h"
#include "pldm_bej_resolve.h"

extern pldm_gen_state_t gs_pldm_redfish_gen_state;
extern pldm_redfish_op_identify_t g_cur_op_identify;
static void pldm_redfish_gen_recv_cmd_01(u8 *buf)
{
    pldm_redfish_negotiate_redfish_parameters_rsp_dat_t *rsp_dat = (pldm_redfish_negotiate_redfish_parameters_rsp_dat_t *)(buf + sizeof(pldm_response_t));
    LOG("dev_provider_name  : %s, dev_feature_support : %d, dev_feature_support : %#x", rsp_dat->dev_provider_name.val, rsp_dat->dev_concurrency_support, rsp_dat->dev_feature_support);
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_UNKNOW;
}

static void pldm_redfish_gen_recv_cmd_02(u8 *buf)
{
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_UNKNOW;
}

static pldm_redfish_rde_multipart_receive_req_dat_t multi_recv_rsp_dat; 
static void pldm_redfish_gen_recv_cmd_03(u8 *buf)
{
    pldm_redfish_get_schema_dictionary_rsp_dat_t *rsp_dat = (pldm_redfish_get_schema_dictionary_rsp_dat_t *)(buf + sizeof(pldm_response_t));
    multi_recv_rsp_dat.data_transfer_handle = rsp_dat->transfer_handle;
    multi_recv_rsp_dat.op_id = 0;
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_NEED_MULTI_RECV;
}

static void pldm_redfish_gen_recv_cmd_04(u8 *buf)
{
    pldm_redfish_get_schema_uri_rsp_dat_t *rsp_dat = (pldm_redfish_get_schema_uri_rsp_dat_t *)(buf + sizeof(pldm_response_t));
    LOG("schema_uri : %s", rsp_dat->schema_uri);
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_UNKNOW;
}

static void pldm_redfish_gen_recv_cmd_05(u8 *buf)
{
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_UNKNOW;
}

static void pldm_redfish_gen_recv_cmd_06(u8 *buf)
{
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_UNKNOW;
}

static void pldm_redfish_gen_recv_cmd_07(u8 *buf)
{
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_UNKNOW;
}

static void pldm_redfish_gen_recv_cmd_08(u8 *buf)
{
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_UNKNOW;
}

static void pldm_redfish_gen_recv_cmd_09(u8 *buf)
{
    pldm_redfish_get_registry_details_rsp_dat_t *rsp_dat = (pldm_redfish_get_registry_details_rsp_dat_t *)(buf + sizeof(pldm_response_t));
    LOG("registry_prefix : %s, registry_uri : %s, ver_cnt : %d", rsp_dat->registry_prefix, rsp_dat->registry_uri, rsp_dat->ver_cnt);
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_UNKNOW;
}

static void pldm_redfish_gen_recv_cmd_0b(u8 *buf)
{
    pldm_redfish_get_msg_registry_rsp_dat_t *rsp_dat = (pldm_redfish_get_msg_registry_rsp_dat_t *)(buf + sizeof(pldm_response_t));
    multi_recv_rsp_dat.data_transfer_handle = rsp_dat->transfer_handle;
    multi_recv_rsp_dat.op_id = 0;
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_NEED_MULTI_RECV;
}

static void pldm_redfish_gen_recv_cmd_10(u8 *buf)
{
    pldm_response_t *rsp_hdr = (pldm_response_t *)buf;

    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_UNKNOW;
    if (g_cur_op_identify.op_flg & CBIT(2)) {
        gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_11;
    } else if ((g_cur_op_identify.op_flg & CBIT(1)) && rsp_hdr->cpl_code == MCTP_COMMAND_SUCCESS) {
        gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_NEED_MULTI_SEND;
    }
}

static void pldm_redfish_gen_recv_cmd_11(u8 *buf)
{
    pldm_redfish_supply_custom_request_parameters_rsp_dat_t *rsp_dat = (pldm_redfish_supply_custom_request_parameters_rsp_dat_t *)(buf + sizeof(pldm_response_t));
    pldm_response_t *rsp_hdr = (pldm_response_t *)buf;

    LOG("op_status : %d", rsp_dat->op_status);
    LOG("cpl_percentage : %d", rsp_dat->cpl_percentage);
    LOG("cpl_time_sec : %lld", rsp_dat->cpl_time_sec);
    LOG("op_execution_flg : %#x", rsp_dat->op_execution_flg);
    LOG("result_transfer_handle : %lld", rsp_dat->result_transfer_handle);
    LOG("permission_flg : %#x", rsp_dat->permission_flg);
    LOG("rsp_payload_len : %d", rsp_dat->rsp_payload_len);
    LOG("etag.val : %s", rsp_dat->etag.val);

    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_UNKNOW;
    if ((g_cur_op_identify.op_flg & CBIT(1)) && rsp_hdr->cpl_code == MCTP_COMMAND_SUCCESS) {
        gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_NEED_MULTI_SEND;
    }
}

static void pldm_redfish_gen_recv_cmd_12(u8 *buf)
{

}

static void pldm_redfish_gen_recv_cmd_13(u8 *buf)
{
    g_cur_op_identify.resource_id = 0;
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_UNKNOW;
}

extern u8 g_needed_dict[PLDM_REDFISH_PORT_DICT_LEN];
extern u8 g_anno_dict[PLDM_REDFISH_ANNO_DICT_LEN];
static void pldm_redfish_gen_recv_cmd_14(u8 *buf)
{
    pldm_redfish_rde_operation_status_rsp_dat_t *rsp_dat = (pldm_redfish_rde_operation_status_rsp_dat_t *)(buf + sizeof(pldm_response_t));

    LOG("op_status : %d", rsp_dat->op_status);
    LOG("cpl_percentage : %d", rsp_dat->cpl_percentage);
    LOG("cpl_time_sec : %#x", rsp_dat->cpl_time_sec);
    LOG("op_execution_flg : %#x", rsp_dat->op_execution_flg);
    LOG("result_transfer_handle : %lld", rsp_dat->result_transfer_handle);
    LOG("permission_flg : %#x", rsp_dat->permission_flg);
    LOG("rsp_payload_len : %d", rsp_dat->rsp_payload_len);
    if (rsp_dat->op_status == OPERATION_HAVE_RESULTS) {
        if (rsp_dat->rsp_payload_len) {
            gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_UNKNOW;
            u8 *annc_dict = &g_anno_dict[DICT_FMT_HDR_LEN];
            u8 *dict = &g_needed_dict[DICT_FMT_HDR_LEN];
            pldm_cjson_t *root = NULL;
            u16 len = rsp_dat->rsp_payload_len - sizeof(bejencoding_t);

            root = pldm_bej_decode(&(rsp_dat->etag.val[rsp_dat->etag.len + sizeof(bejencoding_t)]), len, annc_dict, dict, root);
            if (root) {
                pldm_cjson_printf_root(root);
            }
            pldm_cjson_pool_reinit();
            root = NULL;
        } else {
            gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_NEED_MULTI_RECV;
        }
        multi_recv_rsp_dat.data_transfer_handle = rsp_dat->result_transfer_handle;
    } else {
        gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_UNKNOW;
    }
}

static void pldm_redfish_gen_recv_cmd_15(u8 *buf)
{
    g_cur_op_identify.resource_id = 0;
    gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_UNKNOW;
}

static void pldm_redfish_gen_recv_cmd_16(u8 *buf)
{

}

static void pldm_redfish_gen_recv_cmd_30(u8 *buf)
{
    pldm_redfish_rde_multipart_send_rsp_dat_t *rsp_dat = (pldm_redfish_rde_multipart_send_rsp_dat_t *)(buf + sizeof(pldm_response_t));
    if (rsp_dat->transfer_op != XFER_COMPLETE) {
        gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_NEED_MULTI_SEND;
    } else {
        gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_UNKNOW;
    }

}

static u32 dict_size = 0;
static u8 op_result[8192];
extern u32 crc32_pldm(u32 init_crc, u8 *data, u32 len);
static void pldm_redfish_gen_recv_cmd_31(u8 *buf)
{
    pldm_redfish_rde_multipart_receive_rsp_dat_t *rsp_dat = (pldm_redfish_rde_multipart_receive_rsp_dat_t *)(buf + sizeof(pldm_response_t));
    LOG("transfer_flg : %d, next_transfer_handle : %d, data_len_bytes : %d", rsp_dat->transfer_flg, rsp_dat->next_transfer_handle, rsp_dat->data_len_bytes);
    cm_memcpy(&op_result[dict_size], rsp_dat->data, rsp_dat->data_len_bytes);
    dict_size += rsp_dat->data_len_bytes;
    if (rsp_dat->next_transfer_handle != 0) {
        multi_recv_rsp_dat.data_transfer_handle = rsp_dat->next_transfer_handle;
        gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_NEED_MULTI_RECV;
    } else {
        if (g_cur_op_identify.op_id == 0) {
            LOG("dictionary size : %d", dict_size);
        } else {
            u32 *crc32 = (u32 *)&(rsp_dat->data[rsp_dat->data_len_bytes - 4]);
            u32 cal_crc = crc32_pldm(0xFFFFFFFFUL, op_result, dict_size - 4);
            LOG("result size : %d, crc result : %s", dict_size, cal_crc == *crc32 ? "true" : "false");
            u8 *annc_dict = &g_anno_dict[DICT_FMT_HDR_LEN];
            u8 *dict = &g_needed_dict[DICT_FMT_HDR_LEN];
            pldm_cjson_t *root = NULL;
            u16 len = dict_size - sizeof(bejencoding_t) - 4;

            root = pldm_bej_decode(&(op_result[sizeof(bejencoding_t)]), len, annc_dict, dict, root);
            if (root) {
                pldm_cjson_printf_root(root);
            }
            pldm_cjson_pool_reinit();
            root = NULL;
        }
        dict_size = 0;
        gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_ENTER_CMD_UNKNOW;
    }
}

pldm_redfish_rde_multipart_receive_req_dat_t pldm_redfish_get_multi_recv_rsp_dat(void)
{
    return multi_recv_rsp_dat;
}

void pldm_redfish_gen_recv(int cmd, u8 *buf)
{
    gen_cmd cmds[] = {
        gen_cmd_unsupport,
        pldm_redfish_gen_recv_cmd_01,
        pldm_redfish_gen_recv_cmd_02,
        pldm_redfish_gen_recv_cmd_03,
        pldm_redfish_gen_recv_cmd_04,
        pldm_redfish_gen_recv_cmd_05,
        pldm_redfish_gen_recv_cmd_06,
        pldm_redfish_gen_recv_cmd_07,
        pldm_redfish_gen_recv_cmd_08,
        pldm_redfish_gen_recv_cmd_09,
        gen_cmd_unsupport,
        pldm_redfish_gen_recv_cmd_0b,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        gen_cmd_unsupport,
        pldm_redfish_gen_recv_cmd_10,
        pldm_redfish_gen_recv_cmd_11,
        pldm_redfish_gen_recv_cmd_12,
        pldm_redfish_gen_recv_cmd_13,
        pldm_redfish_gen_recv_cmd_14,
        pldm_redfish_gen_recv_cmd_15,
        pldm_redfish_gen_recv_cmd_16,
        // pldm_redfish_gen_recv_cmd_30,
        // pldm_redfish_gen_recv_cmd_31,
    };

    if (cmd < PLDM_REDFISH_CMD) {
        if (cmds[cmd])
            cmds[cmd](buf);
        LOG("RECV CMD : %#x\n", cmd);
    } else if (cmd == 0x30) {
        pldm_redfish_gen_recv_cmd_30(buf);
        LOG("RECV CMD : %#x\n", cmd);
    } else if (cmd == 0x31) {
        pldm_redfish_gen_recv_cmd_31(buf);
        LOG("RECV CMD : %#x\n", cmd);
    } else {
        gs_pldm_redfish_gen_state.event_id = PLDM_REDFISH_GEN_UNKNOW;
        LOG("ERR CMD : %#x\n", cmd);
    }
}