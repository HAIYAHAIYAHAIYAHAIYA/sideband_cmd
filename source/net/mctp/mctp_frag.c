/*
    MCTP fragmentation process functions
*/
#include "protocol_device.h"
#include "mctp.h"
#include "mctp_frag.h"

static mctp_pkt_error_type gs_mctp_merge_status = MERGEING;
static u16 gs_buf_len = 0;
static int gs_pre_msg_tag = -1;
static int gs_pre_pkt_seq = -1;
static u8 gs_mctp_frag_buf[MCTP_RECV_BUF_SIZE];

static void mctp_clear_gs_param(void)
{
    gs_pre_pkt_seq = -1;
    gs_pre_msg_tag = -1;
    gs_buf_len = 0;
}

/* pkt_len contain mctp hdr and payload */
int mctp_frag_process(protocol_msg_t *skb, int pkt_len, int hw_hdr_len)
{
    mctp_hdr_t *mctp = (mctp_hdr_t *)skb->req_buf;
    u16 payload_len = pkt_len - REAL_MCTP_HDR;
    u8 *merg_buf = gs_mctp_frag_buf;
    int status = MERGEING;

    if (!mctp) {
        status = NULL_PTR_ERR;
        goto L_ERR;
    }

    if (payload_len == 0) {
        status = BLEN_ERR;
        goto L_ERR;
    }

    if (mctp->som == 1) {
        // step1. 判断是否是第一个包，是的话copy hw hdr + mctp hdr
        if (gs_pre_msg_tag != -1) {            // S1E0 S1E0 S1E0 S1E0 S0E0 S0E1 情况 多S1E0乱序
            status = S1E0_S1E0_ERR;
        }
        // save tag, seq in mctp hdr
        gs_pre_msg_tag = mctp->msg_tag;
        gs_pre_pkt_seq = mctp->pkt_seq;

        int total_len = hw_hdr_len + pkt_len;
        u8 *hw_pt = skb->req_buf - hw_hdr_len;
        mctp_memcpy_fast(merg_buf, hw_pt, total_len);
        gs_buf_len = total_len;
    } else {
        // step2. copy mctp payload
        gs_pre_pkt_seq = (gs_pre_pkt_seq + 1) & 0x3;
        if (gs_pre_pkt_seq != mctp->pkt_seq) {      // pkt_seq 不匹配
            status = SEQ_ERR;
            goto L_ERR;
        }
        if (gs_pre_msg_tag != mctp->msg_tag) {      // msg_tag 不匹配
            status = TAG_ERR;
            goto L_ERR;
        }

        u8 *payload_pt = (u8 *)mctp + REAL_MCTP_HDR;
        mctp_memcpy_fast(merg_buf + gs_buf_len, payload_pt, payload_len);
        gs_buf_len += payload_len;
        // step3. 判断是否是最后一个包，是的话修改 skb->req_buf
        if (mctp->eom) {   // som = 0;eom = 1;
            status = MERGE_SUC;
            goto L_OK;
        }
    }

// step4. 最后一个包返回成功，其他返回pending or error
L_ERR:
    gs_mctp_merge_status = status;
    return status;
////////////////////////////////////////////////////
L_OK:
    mctp_clear_gs_param();
    gs_mctp_merge_status = status;
    skb->req_buf = gs_mctp_frag_buf + hw_hdr_len;
    return status;
}