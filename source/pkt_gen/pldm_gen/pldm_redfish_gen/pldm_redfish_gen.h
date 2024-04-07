#ifndef __PLDM_REDFSIH_GEN_H__
#define __PLDM_REDFSIH_GEN_H__

#include "main.h"
#include "pldm_redfish.h"

#define PLDM_REDFISH_CMD_GEN(num)                 pldm_redfish_gen_cmd_ ## num
#define PLDM_REDFISH_XFER_CHUNKSIZE               (64)

typedef enum {
    PLDM_REDFISH_GEN_IDLE = 0
} pldm_redfish_gen_gen_state_t;

typedef enum {
    PLDM_REDFISH_GEN_UNKNOW = 0,
    PLDM_REDFISH_GEN_ENTER_CMD_01 = 1,
    PLDM_REDFISH_GEN_ENTER_CMD_02,
    PLDM_REDFISH_GEN_ENTER_CMD_03,
    PLDM_REDFISH_GEN_ENTER_CMD_04,
    PLDM_REDFISH_GEN_ENTER_CMD_05,
    PLDM_REDFISH_GEN_ENTER_CMD_06,
    PLDM_REDFISH_GEN_ENTER_CMD_07,
    PLDM_REDFISH_GEN_ENTER_CMD_08,
    PLDM_REDFISH_GEN_ENTER_CMD_09,
    PLDM_REDFISH_GEN_ENTER_CMD_0b,
    PLDM_REDFISH_GEN_ENTER_CMD_10,
    PLDM_REDFISH_GEN_ENTER_CMD_11,
    PLDM_REDFISH_GEN_ENTER_CMD_13,
    PLDM_REDFISH_GEN_ENTER_CMD_14,
    PLDM_REDFISH_GEN_ENTER_CMD_15,
    PLDM_REDFISH_GEN_NEED_MULTI_RECV,
    PLDM_REDFISH_GEN_NEED_MULTI_SEND,
    PLDM_REDFISH_GEN_ENTER_CMD_UNKNOW,
} pldm_redfish_gen_event_id_t;

void pldm_redfish_gen_init(void);
void pldm_redfish_gen(int cmd, u8 *buf);
u8 pldm_redfish_state_transform_switch(u8 cnt, u8 *buf);
void pldm_redfish_gen_recv(int cmd, u8 *buf);

pldm_redfish_rde_multipart_receive_req_dat_t pldm_redfish_get_multi_recv_rsp_dat(void);

#endif /* __PLDM_REDFSIH_GEN_H__ */