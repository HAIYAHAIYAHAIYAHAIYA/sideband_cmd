#ifndef __MCTP_FRAG_H__
#define __MCTP_FRAG_H__

#include "main.h"

#define MCTP_RECV_BUF_SIZE                  (1024 + 5 + 4)      /* sizeof(mctp_hdr_t) + sizeof(pldm_response_t) for pldm fw update RequestFirmwareData (1k unit) */

typedef enum {
    MERGEING = 1,
    MERGE_SUC = 0,
    BLEN_ERR = -1,
    NULL_PTR_ERR = -2,
    TAG_ERR = -3,
    SEQ_ERR = -4,
    S1E0_S1E0_ERR = -5,
} mctp_pkt_error_type;

int mctp_frag_process(protocol_msg_t *skb, int pkt_len, int hw_hdr_len);

#endif /* __MCTP_FRAG_H__ */