#include "mctp_ctl.h"
#include "mctp.h"
#include "mctp_frag.h"

mctp_base_info g_mctp_ctrl_info[3];         /* 0 - smbus, 1 - vdm0, 2 - vdm1 */

//extern pldm_monitor_base_info_t g_pldm_monitor_info;
extern u8 g_instance_id;

extern void terminus_locator_pdr_chg(void);

static void mctp_init_rsp(protocol_msg_t *pkt, int *pkt_len)
{
    cm_memcpy(pkt->rsp_buf, pkt->req_buf, sizeof(mctp_ctrl_request_t));
    mctp_ctrl_response_t *rsp = (mctp_ctrl_response_t *)pkt->rsp_buf;
    rsp->cpl_code = MCTP_COMMAND_SUCCESS;
    rsp->rq = 0;
    *pkt_len += sizeof(mctp_ctrl_response_t);      /* mctp ctl rsp (3) */
}

static int mctp_unsupport_cmd(protocol_msg_t *pkt, int *pkt_len)
{
    LOG("the cmd is not supported");
    mctp_ctrl_response_t *rsp = (mctp_ctrl_response_t *)(pkt->rsp_buf - sizeof(mctp_ctrl_response_t));
    rsp->cpl_code = MCTP_COMMAND_UNSUPPORT;
    return SUCCESS;
}

static int mctp_set_eid(protocol_msg_t *pkt, int *pkt_len)
{
    set_eid_req_dat *req_dat = (set_eid_req_dat *)(pkt->req_buf);
    mctp_ctrl_response_t *rsp = (mctp_ctrl_response_t *)(pkt->rsp_buf - sizeof(mctp_ctrl_response_t));;
    LOG("mctp_set_eid cmd, operation : 0x%x eid : 0x%x", req_dat->operation, req_dat->eid);

    switch (req_dat->operation) {
        case SET_EID:
        case FORCE_EID:
            if (req_dat->eid == MCTP_EID_NULL || req_dat->eid == MCTP_EID_BROADCAST) {
                rsp->cpl_code = MCTP_COMMAND_INVALIDDATA;
                return SUCCESS;
            } else {
                g_mctp_ctrl_info[pkt->mctp_hw_id].dev_eid = req_dat->eid;
                g_mctp_ctrl_info[pkt->mctp_hw_id].discovered = TRUE;
            }
            break;

        case RESET_EID:
            rsp->cpl_code = MCTP_COMMAND_INVALIDDATA;
            return SUCCESS;

        case SET_DISC_FLAG:
            g_mctp_ctrl_info[pkt->mctp_hw_id].discovered = TRUE;
            break;
    }
    set_eid_rsp_dat *rsp_dat = (set_eid_rsp_dat *)(pkt->rsp_buf);
    rsp_dat->eid_alloc_status = 0;
    rsp_dat->resv1 = 0;
    rsp_dat->eid_assign_status = 0;
    rsp_dat->resv2 = 0;
    rsp_dat->eid_set = g_mctp_ctrl_info[pkt->mctp_hw_id].dev_eid;
    rsp_dat->eid_pool_sz = 0;

    terminus_locator_pdr_chg();

    *pkt_len += sizeof(set_eid_rsp_dat);
    return SUCCESS;
}

static int mctp_get_eid(protocol_msg_t *pkt, int *pkt_len)
{
    get_eid_rsp_dat *rsp_dat = (get_eid_rsp_dat *)(pkt->rsp_buf);

    rsp_dat->eid = (g_mctp_ctrl_info[pkt->mctp_hw_id].discovered == TRUE) ? g_mctp_ctrl_info[pkt->mctp_hw_id].dev_eid : 0;
    rsp_dat->eid_type = 0;
    rsp_dat->resv1 = 0;
    rsp_dat->e_type = 0;
    rsp_dat->resv2 = 0;
    rsp_dat->medium_spec_info = 0;

    *pkt_len += sizeof(get_eid_rsp_dat);
    LOG("mctp_get_eid cmd, rsp->eid %x",rsp_dat->eid);
    return SUCCESS;
}

static int mctp_get_uuid(protocol_msg_t *pkt, int *pkt_len)
{
    get_uuid_rsp_dat *rsp_dat = (get_uuid_rsp_dat *)(pkt->rsp_buf);

    cm_memcpy((u8 *)&(rsp_dat->time_low), (u8 *)&(g_mctp_ctrl_info[pkt->mctp_hw_id].uuid), sizeof(get_uuid_rsp_dat));

    *pkt_len += sizeof(get_uuid_rsp_dat);
    LOG("mctp get uuid cmd, uuid node : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x", rsp_dat->node[0], rsp_dat->node[1],
            rsp_dat->node[2], rsp_dat->node[3], rsp_dat->node[4], rsp_dat->node[5]);
    return SUCCESS;
}

static int mctp_get_ver_sup(protocol_msg_t *pkt, int *pkt_len)
{
    get_ver_sup_req_dat *req_dat = (get_ver_sup_req_dat *)(pkt->req_buf);
    get_ver_sup_rsp_dat *rsp_dat = (get_ver_sup_rsp_dat *)(pkt->rsp_buf);

    rsp_dat->ver_entry_cnt = 1;
    switch (req_dat->msg_type_num) {
        case MCTP_MSG_TYPE_MCTPBASE:    /* DSP0236_1.3.1_MCTP_Base_Specification.pdf */
        case MCTP_MSG_TYPE_CONTROL:
            rsp_dat->alpha_byte = 0x00;
            rsp_dat->upd_ver_num = 0xF1;
            rsp_dat->minor_ver_num = 0xF3;
            rsp_dat->major_ver_num = 0xF1;
            break;

        case MCTP_MSG_TYPE_PLDM :       /* DSP0241_1.0.0_PLDM over MCTP Binding Spec.pdf */
        case MCTP_MSG_TYPE_SECURED :    /* https://www.dmtf.org/sites/default/files/standards/documents/DSP0276_1.0.pdf */
            rsp_dat->alpha_byte = 0x00;
            rsp_dat->upd_ver_num = 0xF0;
            rsp_dat->minor_ver_num = 0xF0;
            rsp_dat->major_ver_num = 0xF1;
            break;

        case MCTP_MSG_TYPE_NCSI :       /* DSP0261_1.2.3_NCSI_over_MCTP_Binding_Spec.pdf */
        case MCTP_MSG_TYPE_ETHERNET :
            rsp_dat->alpha_byte = 0x00;
            rsp_dat->upd_ver_num = 0xF3;
            rsp_dat->minor_ver_num = 0xF2;
            rsp_dat->major_ver_num = 0xF1;
            break;

        case MCTP_MSG_TYPE_NVMEMI :     /* https://www.dmtf.org/sites/default/files/standards/documents/DSP0235_1.0.pdf */
        case MCTP_MSG_TYPE_SPDM :       /* https://www.dmtf.org/sites/default/files/standards/documents/DSP0275_1.0.pdf */
            rsp_dat->alpha_byte = 0x00;
            rsp_dat->upd_ver_num = 0xF1;
            rsp_dat->minor_ver_num = 0xF0;
            rsp_dat->major_ver_num = 0xF1;
            break;

        case MCTP_MSG_TYPE_VDPCI :      /* https://www.dmtf.org/sites/default/files/standards/documents/DSP0238_1.2.pdf */
        case MCTP_MSG_TYPE_VDIANA :
            rsp_dat->alpha_byte = 0x00;
            rsp_dat->upd_ver_num = 0xF0;
            rsp_dat->minor_ver_num = 0xF2;
            rsp_dat->major_ver_num = 0xF1;
            break;

        default:
            LOG("mctp get ver support cmd unsupport %x!", req_dat->msg_type_num);
            mctp_ctrl_response_t *rsp = (mctp_ctrl_response_t *)(pkt->rsp_buf - sizeof(mctp_ctrl_response_t));
            rsp->cpl_code = MCTP_COMMAND_MSGTYPEUNSP;
            return SUCCESS;
    }

    *pkt_len += sizeof(get_ver_sup_rsp_dat);
    LOG("mctp get ver support cmd, alpha 0x%x, upd 0x%x, minor 0x%x, major 0x%x", rsp_dat->alpha_byte,
            rsp_dat->upd_ver_num, rsp_dat->minor_ver_num, rsp_dat->major_ver_num);
    return SUCCESS;
}

static int mctp_get_msg_type_sup(protocol_msg_t *pkt, int *pkt_len)
{
    get_msg_type_sup_rsp_dat *rsp_dat = (get_msg_type_sup_rsp_dat *)(pkt->rsp_buf);

    rsp_dat->msg_type_cnt = 3;
    rsp_dat->msg_type_code[0] = MCTP_MSG_TYPE_PLDM;
    rsp_dat->msg_type_code[1] = MCTP_MSG_TYPE_NCSI;
    rsp_dat->msg_type_code[2] = MCTP_MSG_TYPE_VDPCI;

    *pkt_len += (sizeof(u8) * (rsp_dat->msg_type_cnt + 1));
    LOG("mctp mctp_get_msg_type_sup cmd, type 0x%x", rsp_dat->msg_type_code);
    return SUCCESS;
}

static int mctp_get_vendor_defined_msg_sup(protocol_msg_t *pkt, int *pkt_len)
{
    get_vendor_defined_msg_sup_req_dat *req_dat = (get_vendor_defined_msg_sup_req_dat *)(pkt->req_buf);
    get_vendor_defined_msg_sup_rsp_dat *rsp_dat = (get_vendor_defined_msg_sup_rsp_dat *)(pkt->rsp_buf);

    if (req_dat->vendor_id_set_selector == 0x00) {
        rsp_dat->vendor_id_set_selector = 0xff;     /* No more capability sets */
        rsp_dat->vendor_id = 0x0000;                /* 待定 */
        rsp_dat->pci_id_indicator = 0x00;           /* pci_id */
        rsp_dat->ver = 0x0000;                      /* 待定 */
        *pkt_len += sizeof(get_vendor_defined_msg_sup_rsp_dat);
        return SUCCESS;
    } else {
        LOG("mctp get vendor define msg support cmd unsupport %x!", req_dat->vendor_id_set_selector);
        return FAILURE;
    }
}

static int mctp_resolve_eid_rsp(protocol_msg_t *pkt, int *pkt_len)
{
    resolve_eid_rsp_dat *rsp_dat = (resolve_eid_rsp_dat *)(pkt->req_buf);
    if (rsp_dat->cpl_code != MCTP_COMMAND_SUCCESS) {
        LOG("mctp_discovery_notify_rsp err, cpl code : 0x%02x", rsp_dat->cpl_code);
        return FAILURE;
    }

    //g_pldm_monitor_info.event_receiver_eid = rsp_dat->bridge_eid;
    //g_pldm_monitor_info.phy_addr.val = rsp_dat->phy_addr.val;

    LOG("that is, the target EID is local to the bus that the Resolve Endpoint ID request was issued over) bridge eid : %02x, BDF : %d %d %d", \
    rsp_dat->bridge_eid, rsp_dat->phy_addr.bus_num, rsp_dat->phy_addr.dev_num, rsp_dat->phy_addr.func_num);

    return FAILURE;
}

static int mctp_prepare_for_eid_discovery(protocol_msg_t *pkt, int *pkt_len)
{
    mctp_ctrl_response_t *rsp_hdr = (mctp_ctrl_response_t *)(pkt->rsp_buf - sizeof(mctp_ctrl_response_t));
    if (pkt->mctp_max_payload != VDM_SPLIT_MAX_LEN) {   /* Only MCTP over PCIe Vendor Defined Messaging */
        rsp_hdr->cpl_code = MCTP_COMMAND_UNSUPPORT;
        LOG("Only MCTP over PCIe Vendor Defined Messaging, mctp_max_payload : %d", pkt->mctp_max_payload);
    } else {
        g_mctp_ctrl_info[pkt->mctp_hw_id].discovered = FALSE;
    }
    return SUCCESS;
}

static int mctp_eid_discovery(protocol_msg_t *pkt, int *pkt_len)
{
    int ret = SUCCESS;
    mctp_ctrl_response_t *rsp_hdr = (mctp_ctrl_response_t *)(pkt->rsp_buf - sizeof(mctp_ctrl_response_t));
    if (pkt->mctp_max_payload != VDM_SPLIT_MAX_LEN) {   /* Only MCTP over PCIe Vendor Defined Messaging */
        rsp_hdr->cpl_code = MCTP_COMMAND_UNSUPPORT;
        LOG("Only MCTP over PCIe Vendor Defined Messaging, mctp_max_payload : %d", pkt->mctp_max_payload);
    } else if (g_mctp_ctrl_info[pkt->mctp_hw_id].discovered == TRUE) {
        /* not respond */
        ret = FAILURE;
    }

    return ret;
}

static int mctp_discovery_notify_rsp(protocol_msg_t *pkt, int *pkt_len)
{
    mctp_ctrl_response_t *rsp_hdr = (mctp_ctrl_response_t *)(pkt->req_buf - sizeof(mctp_ctrl_request_t));
    if (rsp_hdr->cpl_code != MCTP_COMMAND_SUCCESS) {
        LOG("mctp_discovery_notify_rsp err, cpl code : 0x%02x", rsp_hdr->cpl_code);
    }
    return FAILURE;
}

static int mctp_query_rate_limit(protocol_msg_t *pkt, int *pkt_len)
{
    query_rate_limit_rsp_dat *rsp_dat = (query_rate_limit_rsp_dat *)(pkt->rsp_buf);

    rsp_dat->recv_buf_size = MCTP_RECV_BUF_SIZE;
    rsp_dat->max_recv_data_rate_limit = 0x1DCD65;           /* Reflects a rate of 1 Gb/s in 64-bytes packets. (1000 * 1000 * 1000 / 8 / 64 )*/
    rsp_dat->max_sup_rate_limit = 0x1DCD65;                 /* Reflects a rate of 1 Gb/s in 64-bytes packets. */
    rsp_dat->min_sup_rate_limit = 0x7A1;                    /* Reflects a rate of 1 Mb/s in 64-bytes packets. (1000 * 1000 / 8 / 64 ) */
    u32 max_sup_brust_size = MCTP_RECV_BUF_SIZE / 64;
    cm_memcpy(rsp_dat->max_sup_brust_size, &max_sup_brust_size, 3);
    // rsp_dat->max_brust_size = ;                          /* According to current setting */
    // rsp_dat->eid_max_trans_data_rate_limit = ;           /* According to current setting */
    rsp_dat->trans_rate_limit_op_cap = 1;
    if (pkt->mctp_hw_id == 0)                               /* smbus */
        rsp_dat->rate_limit_sup_on_eid = 0;
    else
        rsp_dat->rate_limit_sup_on_eid = 1;

    *pkt_len += sizeof(query_rate_limit_rsp_dat);

    return SUCCESS;
}

static int mctp_request_tx_rate_limit(protocol_msg_t *pkt, int *pkt_len)
{
    request_tx_rate_limit_req_dat *req_dat = (request_tx_rate_limit_req_dat *)(pkt->req_buf);
    request_tx_rate_limit_rsp_dat *rsp_dat = (request_tx_rate_limit_rsp_dat *)(pkt->rsp_buf);
    mctp_ctrl_response_t *rsp_hdr = (mctp_ctrl_response_t *)(pkt->rsp_buf - sizeof(mctp_ctrl_response_t));

    if (pkt->mctp_hw_id == 0) {                             /* smbus */
        rsp_hdr->cpl_code = MCTP_COMMAND_INVALIDDATA;
    } else {
        u32 req_eid_trans_max_burst_size = 0;
        cm_memcpy(&req_eid_trans_max_burst_size, req_dat->eid_trans_max_burst_size, sizeof(req_dat->eid_trans_max_burst_size));
        /* TODO set its rate limit according to the requested values. */
        (void)rsp_dat;
        // rsp_dat->eid_trans_burst_size = ;
        // rsp_dat->eid_trans_data_rate_limit = ;
        *pkt_len += sizeof(request_tx_rate_limit_rsp_dat);
    }
    return SUCCESS;
}

static int mctp_update_rate_limit(protocol_msg_t *pkt, int *pkt_len)
{
    /* The device never sends this command and completes it without error, but ignores it when received.(E810) */
    return SUCCESS;
}

static int mctp_query_supported_interfaces(protocol_msg_t *pkt, int *pkt_len)
{
    query_supported_interface_rsp_dat *rsp_dat = (query_supported_interface_rsp_dat *)(pkt->rsp_buf);

    rsp_dat->sup_interface_cnt = 3;

    for (u8 i = 0; i < 3; i++) {
        rsp_dat->interface_info[i].eid = g_mctp_ctrl_info[i].dev_eid;
        if (!i)
            rsp_dat->interface_info[i].type = SMBUS_SPD_100K;
        else
            rsp_dat->interface_info[i].type = PCIE_4_0_DEV;     /* to be determind */
    }

    *pkt_len += sizeof(query_supported_interface_rsp_dat) + rsp_dat->sup_interface_cnt * sizeof(interface_info_dat);
    return SUCCESS;
}

void mctp_ctl_fill_commmon_field(u8 *buf, u8 cmd_code)
{
    mctp_ctrl_request_t *mctp_ctl_hdr = (mctp_ctrl_request_t *)buf;
    mctp_ctl_hdr->instance_id = g_instance_id < 31 ? g_instance_id++ : 0;
    mctp_ctl_hdr->rq = 1;
    mctp_ctl_hdr->d = 0;
    mctp_ctl_hdr->rsvd = 0;
    mctp_ctl_hdr->cmd_code = cmd_code;
}

void mctp_resolve_eid_req(int hw_id, u8 target_eid)
{
    u8 buf_req[64] = { 0 };
    u8 buf_rsp[64] = { 0 };
    protocol_msg_t pkt = { 0 };

    // protocol_msg_init(hw_id, &pkt);
    pkt.req_buf = buf_req + pkt.mctp_hw_hdr_len + sizeof(mctp_hdr_t);
    pkt.rsp_buf = buf_rsp + pkt.mctp_hw_hdr_len + sizeof(mctp_hdr_t);

    // fill mctp ctrl header and payload
    mctp_ctl_fill_commmon_field(pkt.rsp_buf, 0x7);

    u8 *paylaod = (u8 *)pkt.rsp_buf + sizeof(mctp_ctrl_request_t);
    *paylaod = target_eid;

    // fill mctp header
    mctp_hdr_t *mctp_hdr = (mctp_hdr_t *)(pkt.req_buf - sizeof(mctp_hdr_t));
    mctp_fill_common_field((u8 *)mctp_hdr, 0, MCTP_MSG_TYPE_CONTROL);
    // todo: fill mctp_hdr with api in mctp layer

    // fill vdm or smbus header
    // pcie_extcm_msg_t *hw_hdr = (pcie_extcm_msg_t *)buf_req;
    // hw_fill_common_field((u8 *)hw_hdr, hw_id, USE_BY_BROADCAST);
    // todo: fill hw_hdr with api in vdm or smbus layer

    // mctp_as_requester_send_to(&pkt, 1 + sizeof(mctp_ctrl_request_t));
}

void mctp_discovery_notify_req(int hw_id)
{
    u8 buf_req[64] = { 0 };
    u8 buf_rsp[64] = { 0 };
    protocol_msg_t pkt = { 0 };

    // protocol_msg_init(hw_id, &pkt);
    pkt.req_buf = buf_req + pkt.mctp_hw_hdr_len + sizeof(mctp_hdr_t);
    pkt.rsp_buf = buf_rsp + pkt.mctp_hw_hdr_len + sizeof(mctp_hdr_t);

    // fill mctp ctrl header and payload
    mctp_ctl_fill_commmon_field(pkt.rsp_buf, 0xd);

    // fill mctp header
    mctp_hdr_t *mctp_hdr = (mctp_hdr_t *)(pkt.req_buf - sizeof(mctp_hdr_t));
    mctp_fill_common_field((u8 *)mctp_hdr, 0, MCTP_MSG_TYPE_CONTROL);
    // todo: fill mctp_hdr with api in mctp layer

    // fill vdm or smbus header
    // pcie_extcm_msg_t *hw_hdr = (pcie_extcm_msg_t *)buf_req;
    // hw_fill_common_field((u8 *)hw_hdr, hw_id, USE_BY_BROADCAST);
    // todo: fill hw_hdr with api in vdm or smbus layer

    // mctp_as_requester_send_to(&pkt, sizeof(mctp_ctrl_request_t));
}

static void mctp_base_info_init(mctp_base_info *dev)
{
//    u8 mac[6] = {0};
    u32 id = 0xFFFF;              /* DEVICE_UUID 待定, 暂时可以随意写一个 */;
    cm_memset(dev, 0, sizeof(mctp_base_info));

    dev->discovered = FALSE;  // dev discovered flag
    dev->dev_eid = 0;  // eid null, for mctp set eid cmd

    /* uuid self define */
    dev->uuid.time_low = 2303170950;
    dev->uuid.time_mid = 'B';
    //dev->uuid.time_high = (FW_VERSION_MAIN << 8) | (FW_VERSION_SUB << 4) | FW_VERSION_UPDATE;
    dev->uuid.time_high = 0xA55A; // todo: generate uuid by sn number
    dev->uuid.clk_seq_hi =  (id >> 8) & 0xF;
    dev->uuid.clk_seq_low = id & 0xF;
//    mng_get_port_mac(port, mac);
    for (int i = 0; i < 6; i++) {
//        dev->uuid.node[i] = mac[i];
        dev->uuid.node[i] = 0;  /* amber获取mac地址待定，暂时uuid配置为0 */
    }
}

static mctp_cmd_func mctp_cmd_table[MCTP_CTL_CMD] =
{
    mctp_unsupport_cmd,                 /* 0x00 */
    mctp_set_eid,                       /* 0x01 */
    mctp_get_eid,                       /* 0x02 */
    mctp_get_uuid,                      /* 0x03 */
    mctp_get_ver_sup,                   /* 0x04 */
    mctp_get_msg_type_sup,              /* 0x05 */
    mctp_get_vendor_defined_msg_sup,    /* 0x06 */
    mctp_resolve_eid_rsp,               /* 0x07 */
    mctp_unsupport_cmd,                 /* 0x00 */
    mctp_unsupport_cmd,                 /* 0x00 */
    mctp_unsupport_cmd,                 /* 0x00 */
    mctp_prepare_for_eid_discovery,     /* 0x0b */
    mctp_eid_discovery,                 /* 0x0c */
    mctp_discovery_notify_rsp,          /* 0x0d */
    mctp_unsupport_cmd,                 /* 0x00 */
    mctp_unsupport_cmd,                 /* 0x00 */
    mctp_unsupport_cmd,                 /* 0x00 */
    mctp_query_rate_limit,              /* 0x11 */
    mctp_request_tx_rate_limit,         /* 0x12 */
    mctp_update_rate_limit,             /* 0x13 */
    mctp_query_supported_interfaces,    /* 0x14 */
};

static int mctp_ctrl_cmd_process(protocol_msg_t *pkt, int *pkt_len)
{
    mctp_ctrl_request_t *ctl_req_ptr = (mctp_ctrl_request_t *)pkt->req_buf;
    mctp_cmd_func cmd_proc = NULL;
    u8 cmd = ctl_req_ptr->cmd_code;

    LOG("mctp cmd 0x%x", cmd);
    if (cmd < MCTP_CTL_CMD) {
        cmd_proc = mctp_cmd_table[cmd];
    } else {
        cmd_proc = mctp_unsupport_cmd;
    }

    mctp_init_rsp(pkt, pkt_len);

    // skip mctp ctl header
    pkt->req_buf += sizeof(mctp_ctrl_request_t);
    pkt->rsp_buf += sizeof(mctp_ctrl_response_t);

    return cmd_proc(pkt, pkt_len);
}

void mctp_ctrl_init(void)
{
    for (u8 i = 0; i < sizeof(g_mctp_ctrl_info) / sizeof(mctp_base_info); i++) {
        mctp_base_info_init(&g_mctp_ctrl_info[i]);
    }
}

int mctp_ctl_process(protocol_msg_t *pkt)
{
    // todo: process msg and send pkt to next protocol device
    int pkt_len = 0;    /* mctp payload, no DW pad len */
    int flag = SUCCESS;
    flag = mctp_ctrl_cmd_process(pkt, &pkt_len);
    if (FAILURE == flag) {
        return FAILURE;
    }
    // add header
    pkt->req_buf -= sizeof(mctp_ctrl_request_t);
    pkt->rsp_buf -= sizeof(mctp_ctrl_response_t);

    LOG("pkt_len : %d", pkt_len);
    for (u16 i = 0; i < (pkt_len + 3) / 4; i++) {
        LOG("data : 0x%08x", ((u32 *)(pkt->rsp_buf))[i]);
    }

    // mctp_as_responser_send_to(pkt, pkt_len);
    return SUCCESS;
}