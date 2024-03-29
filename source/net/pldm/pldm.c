#include "protocol_device.h"
#include "pldm.h"
#include "mctp.h"
#include "pldm_control.h"
#include "pldm_monitor.h"
#include "pldm_redfish.h"
#include "pldm_fw_update.h"

#include "pkt_gen.h"

extern u8 g_pldm_need_rsp;

u8 g_instance_id = 0;

static void pldm_init_rsp(protocol_msg_t *pkt, int *pkt_len)
{
    memcpy(pkt->rsp_buf, pkt->req_buf, sizeof(pldm_request_t));
    pldm_response_t *rsp = (pldm_response_t *)pkt->rsp_buf;
    rsp->cpl_code = MCTP_COMMAND_SUCCESS;
    rsp->rq = 0;
    *pkt_len += sizeof(pldm_response_t);      /* mctp ctl rsp (3) */
}

static void pldm_req_fill_common_field(u8 *buf, u8 pldm_type, u8 cmd_code)
{
    pldm_request_t *pldm_req_hdr = (pldm_request_t *)buf;
    pldm_req_hdr->rq = 1;
    pldm_req_hdr->d = 0;
    pldm_req_hdr->instance_id = g_instance_id < 31 ? g_instance_id++ : 0;
    pldm_req_hdr->hdr_ver = 0;
    pldm_req_hdr->pldm_type = pldm_type;
    pldm_req_hdr->cmd_code = cmd_code;
}

void pldm_msg_send(u8 *msg, u16 msg_len, u8 pldm_type, u8 cmd_code, int hw_id, u8 dest_eid)
{
    if (!msg) return;
    // u8 buf_req[64] = { 0 };
    // u8 buf_rsp[64] = { 0 };
    // protocol_msg_t pkt = { 0 };

    // protocol_msg_init(hw_id, &pkt);
    // pkt.req_buf = buf_req + pkt.mctp_hw_hdr_len + sizeof(mctp_hdr_t);
    // pkt.rsp_buf = buf_rsp + pkt.mctp_hw_hdr_len + sizeof(mctp_hdr_t);

    // pldm_req_fill_common_field(pkt.rsp_buf, pldm_type, cmd_code);

    // fill mctp header
    // mctp_hdr_t *mctp_hdr = (mctp_hdr_t *)(pkt.req_buf - sizeof(mctp_hdr_t));
    // mctp_fill_common_field((u8 *)mctp_hdr, dest_eid, MCTP_MSG_TYPE_PLDM);
    // todo: fill mctp_hdr with api in mctp layer

    // fill vdm or smbus header
    // pcie_extcm_msg_t *hw_hdr = (pcie_extcm_msg_t *)buf_req;
    // hw_fill_common_field((u8 *)hw_hdr, hw_id, USE_BY_ID);
    // todo: fill hw_hdr with api in vdm or smbus layer
    // u8 *paylaod = (u8 *)pkt.rsp_buf + sizeof(pldm_request_t);

    // cm_memcpy(paylaod, msg, msg_len);

    // mctp_as_requester_send_to(&pkt, msg_len + sizeof(pldm_request_t));

    pldm_gen_recv(msg, pldm_type, cmd_code);
}

static void pldm_type_process(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_request_t *pldm_req = (pldm_request_t *)pkt->req_buf;
    u32 pldm_type  = pldm_req->pldm_type;

    pldm_init_rsp(pkt, pkt_len);

    // skip mctp ctl header
    pkt->req_buf += sizeof(pldm_request_t);
    pkt->rsp_buf += sizeof(pldm_response_t);

    LOG("\nSEND CMD : %#x", pldm_req->cmd_code);
    switch (pldm_type) {
        case MCTP_PLDM_CONTROL:
            LOG("MCTP_PLDM_CONTROL");
            pldm_control_process(pkt, pkt_len, pldm_req->cmd_code);
            break;

        case MCTP_PLDM_MONITOR:
            LOG("MCTP_PLDM_MONITOR");
            pldm_monitor_process(pkt, pkt_len, pldm_req->cmd_code);
            break;

        case MCTP_PLDM_UPDATE:
            LOG("MCTP_PLDM_UPDATE");
            pldm_fwup_process(pkt, pkt_len, pldm_req->cmd_code);
            break;

        case MCTP_PLDM_REDFISH:
            LOG("MCTP_PLDM_REDFISH");
            pldm_redfish_process(pkt, pkt_len, pldm_req->cmd_code);
            break;

        default:
            break;
    }
}

int pldm_pkt_process(protocol_msg_t *pkt)
{
    int pkt_len = 0;    /* mctp payload, no DW pad len */
    pldm_type_process(pkt, &pkt_len);

    // add header
    pkt->req_buf -= sizeof(pldm_request_t);
    pkt->rsp_buf -= sizeof(pldm_response_t);

    // LOG("pkt_len : %d", pkt_len);
    // for (u16 i = 0; i < (pkt_len + 3) / 4; i++) {
    //     LOG("data : 0x%08x", ((u32 *)(pkt->rsp_buf))[i]);
    // }

    pldm_request_t *pldm_req = (pldm_request_t *)pkt->req_buf;
    u32 pldm_type  = pldm_req->pldm_type;

    if (g_pldm_need_rsp) {                     /* In the case of actively sending pkt */
        // mctp_as_responser_send_to(pkt, pkt_len);
        pldm_gen_recv(pkt->rsp_buf, pldm_type, pldm_req->cmd_code);
    }
    g_pldm_need_rsp = 1;
    if (pldm_type == MCTP_PLDM_UPDATE)
        pldm_fwup_state_machine_switch();
    return SUCCESS;
}