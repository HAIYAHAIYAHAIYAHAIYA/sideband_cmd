#ifndef __MCTP_DEFRAG_H__
#define __MCTP_DEFRAG_H__

#include "main.h"

#pragma pack(1)
typedef struct {
    u8 ver      : 4;
    u8 rsvd     : 4;
    u8 dest_eid;
    u8 src_eid;
    u8 msg_tag  : 3;
    u8 to       : 1;
    u8 pkt_seq  : 2;
    u8 eom      : 1;
    u8 som      : 1;
} mctp_defrag_hdr_t;
#pragma pack()

int mctp_defrag_process(protocol_msg_t *skb, int pkt_len, u8 max_payload, int hw_hdr_len, sendto_func send_to);

#endif /* __MCTP_DEFRAG_H__ */