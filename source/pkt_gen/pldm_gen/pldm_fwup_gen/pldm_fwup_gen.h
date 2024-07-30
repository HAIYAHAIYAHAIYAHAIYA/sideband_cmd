#ifndef __PLDM_FWUP_GEN_H__
#define __PLDM_FWUP_GEN_H__

#include "main.h"
#include "pldm_fw_update.h"

#define PLDM_FWUP_CMD(num)                  pldm_fwup_gen_cmd_ ## num

#define PLDM_FWUP_IMG_NAME                  "upgrade_pldm_fwup_slot.bin"
#define PLDM_FWUP_GEN_RECVBUF_MAX_SIZE      (666)

typedef enum {
    PLDM_FWUP_GEN_IDLE = 0,
    PLDM_FWUP_GEN_LEARN_COMP,
    PLDM_FWUP_GEN_READY_XFER,
    PLDM_FWUP_GEN_DOWNLOAD,
    PLDM_FWUP_GEN_VERIFY,
    PLDM_FWUP_GEN_APPLY,
    PLDM_FWUP_GEN_ACTIVATE
} pldm_fwup_gen_gen_state_t;

typedef enum {
    PLDM_FWUP_GEN_UNKNOW = 0,
    PLDM_FWUP_GEN_NO_GET_FD_INDENTIFY = 1,
    PLDM_FWUP_GEN_GET_FD_INDENTIFY,
    PLDM_FWUP_GEN_GET_FD_PARAM_AND_NOT_UPDATE,
    PLDM_FWUP_GEN_WITH_PKT_DATA,
    PLDM_FWUP_GEN_SEND_PKT_DATA,
    PLDM_FWUP_GEN_NO_PKT_DATA,
    PLDM_FWUP_GEN_SEND_PKT_DATA_END,
    PLDM_FWUP_GEN_SEND_PASS_COMP_END,
    PLDM_FWUP_GEN_SEND_UP_COMP_END,
    PLDM_FWUP_GEN_SEND_UP_COMP_END_PAUSE,
    PLDM_FWUP_GEN_SEND_FW_DATA,
    PLDM_FWUP_GEN_SEND_FW_DATA_PAUSE,
    PLDM_FWUP_GEN_TRANS_FW_DATA_END,
    PLDM_FWUP_GEN_FD_VERIFY_END,
    PLDM_FWUP_GEN_FD_APPLY_END,
    PLDM_FWUP_GEN_NO_IMG,
    PLDM_FWUP_GEN_STILL_HAVE_IMG,
    PLDM_FWUP_GEN_FD_IS_UPDATE,
    PLDM_FWUP_GEN_SEND_ACTIV_CMD,

    PLDM_FWUP_GEN_COMP_CANCEL_OR_TIMEOUT,
    PLDM_FWUP_GEN_IMG_CANCEL_OR_TIMEOUT,
    PLDM_FWUP_GEN_GET_STATUS,
} pldm_fwup_gen_event_id_t;

void pldm_fwup_gen_init(void);

void pldm_fwup_gen_recv(int cmd, u8 *buf);
void pldm_fwup_gen(int cmd, u8 *buf);
u8 pldm_fwup_state_transform_switch(u8 cnt, u8 *buf);

void pldm_fwup_gen_cmd_11(u8 *buf);
void pldm_fwup_gen_cmd_15(u8 *buf);

pldm_fwup_req_fw_data_req_dat_t pldm_fwup_gen_recv_get_fw_data_req_dat(void);
u32 pldm_fwup_gen_recv_get_pkt_data_req_dat(void);

#endif /* __PLDM_FWUP_GEN_H__ */