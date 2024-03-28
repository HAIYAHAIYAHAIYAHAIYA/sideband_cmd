/*
    MCTP defragmentation  process functions
*/
#include "protocol_device.h"
#include "mctp_defrag.h"
#include "mctp.h"

static u8 gs_mctp_defrag_buf[256];

int mctp_defrag_process(protocol_msg_t *skb, int pkt_len, u8 max_payload, int hw_hdr_len, sendto_func send_to)
{
    int ret = 0;
    // step1. 根据 pkt_len 以及 max_payload 计算分片个数
    int payload_len = pkt_len - REAL_MCTP_HDR;                                    // real mctp header is 4bytes
    int count = (payload_len + max_payload - 1) / max_payload;
    // step2. 对每个分片，预留 hw_hdr_len 用于底层 hw hdr
    mctp_defrag_hdr_t *mctp = (mctp_defrag_hdr_t *)(gs_mctp_defrag_buf + hw_hdr_len);
    u8 *src_paylaod_pt = skb->rsp_buf + REAL_MCTP_HDR;
    // step3. copy mctp hdr，修改 som、eom、pkt_seq
    u8 *old_rsp_buf = skb->rsp_buf;
    u8 *old_req_buf = skb->req_buf;
    // copy mctp header
    mctp_memcpy_fast(mctp, old_rsp_buf, REAL_MCTP_HDR);
    u8 *dst_paylaod_pt =  (u8 *)mctp + REAL_MCTP_HDR;

    for (int i = 0; i < count; i++) {
        mctp->som = (i == 0);                                                 // set som
        mctp->eom = (i == count - 1);                                         // set eom
        mctp->pkt_seq = (i & 0x3);                                            // seq 0~3循环
        // step4. copy payload
        u16 copy_len = (i != count - 1) ? max_payload : (payload_len - max_payload * (count - 1));
        mctp_memcpy_fast(dst_paylaod_pt, src_paylaod_pt, copy_len);
        // step5. send_to 发送分片
        skb->rsp_buf = (u8 *)mctp;
        skb->req_buf = old_req_buf;
        ret = send_to(skb, (copy_len + REAL_MCTP_HDR));
        if (ret < 0) {
            goto L_RET;
        }
        src_paylaod_pt += copy_len;                                                 //copy pos 偏移
    }

L_RET:
    // restore rsp_buf
    skb->rsp_buf = old_rsp_buf;
    return ret;
}