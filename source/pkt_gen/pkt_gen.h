#ifndef __PKT_GEN_H__
#define __PKT_GEN_H__

#include "main.h"
#include "pldm.h"
#include "mctp.h"

typedef void (*gen_cmd)(u8 *buf);
#pragma pack(1)

typedef struct {
    u8 cur_state;
    u8 event_id;
    u8 next_state;
    gen_cmd action;
} pkt_gen_state_transform_t;

#pragma pack()

void pldm_gen_init(void);

void pldm_gen_req_hdr_update(u8 *buf, int cmd);

void pldm_gen_recv(u8 *msg, u8 type, u8 cmd_code);
void gen_cmd_unsupport(u8 *buf);
void mctp_gen(int cmd, u8 *buf);
u8 pldm_gen(int type, int cmd, u8 *buf);

#endif /* __PKT_GEN_H__ */