#include "mctp.h"
#include "mctp_ctl.h"
#include "mctp_frag.h"
#include "mctp_defrag.h"
#include "pldm.h"
#include "ncsi.h"

extern mctp_base_info g_mctp_ctrl_info[3];
static u8 gs_msg_tag = 0;

void *mctp_memcpy_fast(void *dest, const void *src, size_t count)
{
    //if (((u32)dest & 0x3) || ((u32)src & 0x3) || (count & 0x3)) { // NOT 4bytes align
        return cm_memcpy(dest, src, count);
    //} else {
    //    return memcpy32(dest, src, count/DWORD_SIZE);
    //}
}

void mctp_fill_common_field(u8 *buf, u8 src_eid, u8 msg_type)
{
    mctp_hdr_t *mctp_hdr = (mctp_hdr_t *)(buf);
    mctp_hdr->ver = 0x1;
    mctp_hdr->src_eid = src_eid;
    mctp_hdr->msg_type = msg_type;
    mctp_hdr->msg_tag = gs_msg_tag < 7 ? gs_msg_tag++ : 0;
}

static int mctp_send_to(protocol_msg_t *pkt, int pkt_len, u8 to)
{
    // todo: add header and send pkt to next protocol device
    pkt->req_buf -= sizeof(mctp_hdr_t);
    pkt->rsp_buf -= sizeof(mctp_hdr_t);
    pkt_len += sizeof(mctp_hdr_t);

    mctp_hdr_t *req_hdr_ptr = (mctp_hdr_t *)pkt->req_buf;
    mctp_hdr_t *rsp_hdr_ptr = (mctp_hdr_t *)pkt->rsp_buf;

    rsp_hdr_ptr->dest_eid = req_hdr_ptr->src_eid;
    rsp_hdr_ptr->src_eid = g_mctp_ctrl_info[pkt->mctp_hw_id].dev_eid;
    rsp_hdr_ptr->ver = req_hdr_ptr->ver;
    rsp_hdr_ptr->ic = 0;
    rsp_hdr_ptr->msg_type = req_hdr_ptr->msg_type;
    rsp_hdr_ptr->msg_tag = req_hdr_ptr->msg_tag;
    rsp_hdr_ptr->to = to;
    rsp_hdr_ptr->som = 1;
    rsp_hdr_ptr->eom = 1;
    rsp_hdr_ptr->pkt_seq = 0;

    int payload = pkt_len - REAL_MCTP_HDR; // real mctp header is 4bytes
    sendto_func send_to = pkt->send_to_1;
    if (payload > pkt->mctp_max_payload) {
        return mctp_defrag_process(pkt, pkt_len, pkt->mctp_max_payload, pkt->mctp_hw_hdr_len, send_to);
    }

    return send_to(pkt, pkt_len);
}

int mctp_as_requester_send_to(protocol_msg_t *pkt, int pkt_len) {
    return mctp_send_to(pkt, pkt_len, 1);
}

int mctp_as_responser_send_to(protocol_msg_t *pkt, int pkt_len) {
    return mctp_send_to(pkt, pkt_len, 0);
}

int mctp_pkt_process(protocol_msg_t *skb, int pkt_len)
{
    int ret = 0;
    u8 *old_req_buf = skb->req_buf;
    volatile mctp_hdr_t *mctp = (volatile mctp_hdr_t *)old_req_buf;
    if ((mctp->som != 1) || (mctp->eom != 1)) { // fragment pkt
        ret = mctp_frag_process(skb, pkt_len, skb->mctp_hw_hdr_len);
        if (ret != 0) return ret;
        // using new req_buf;
        mctp = (volatile mctp_hdr_t *)skb->req_buf;
    }

    u8 msg_type = mctp->msg_type;
    // skip mctp header
    skb->req_buf += sizeof(mctp_hdr_t);
    skb->rsp_buf += sizeof(mctp_hdr_t);

    switch (msg_type) {
        case MCTP_MSG_TYPE_CONTROL:     // MCTP Control msg，用于在MCTP网络中支持初始化和配置MCTP通信的消息
            LOG("MCTP Control msg");
            ret = mctp_ctl_process(skb);
            break;

        case MCTP_MSG_TYPE_PLDM:     // PLDM msg，用于在MCTP上传输平台级数据模型(PLDM)流量的消息
            LOG("Platform Level Data Model msg");
            ret = pldm_pkt_process(skb);
            break;

        case MCTP_MSG_TYPE_NCSI:    // NC-SI over MCTP，用于通过MCTP传输NC-SI控制流量的消息，暂时不理会ncsi模块，
            LOG("NC-SI over MCTP msg");
            skb->send_to_2 = mctp_as_responser_send_to;
            // ncsi_rx2(skb);
            break;

        case MCTP_MSG_TYPE_ETHERNET:     // Ethernet over MCTP，用于通过MCTP传输以太网流量的消息。此消息类型也可以由其他规范单独使用。
            LOG("Ethernet over MCTP msg");
            break;

        default:
            LOG("unknow msg: %d", msg_type);
            break;
    }

    // restore old req_buf
    if (old_req_buf != skb->req_buf)
        skb->req_buf = old_req_buf;

    return ret;
}
