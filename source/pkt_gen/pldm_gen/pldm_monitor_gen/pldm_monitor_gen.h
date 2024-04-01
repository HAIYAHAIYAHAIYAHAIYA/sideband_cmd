#ifndef __PLDM_MONITOR_GEN_H__
#define __PLDM_MONITOR_GEN_H__

#include "main.h"
#include "pldm_monitor.h"

#define PLDM_MONITOR_CMD_GEN(num)                  pldm_monitor_gen_cmd_ ## num
#define PLDM_MONITOR_GET_PDR_REQ_CNT               (12)

typedef enum {
    PLDM_MONITOR_GEN_IDLE = 0,
} pldm_monitor_gen_gen_state_t;

typedef enum {
    PLDM_MONITOR_GEN_UNKNOW = 0,
    PLDM_MONITOR_GEN_ENTER_CMD_01 = 1,
    PLDM_MONITOR_GEN_ENTER_CMD_02,
    PLDM_MONITOR_GEN_ENTER_CMD_03,
    PLDM_MONITOR_GEN_ENTER_CMD_04,
    PLDM_MONITOR_GEN_ENTER_CMD_05,
    PLDM_MONITOR_GEN_ENTER_CMD_0C,
    PLDM_MONITOR_GEN_ENTER_CMD_0D,
    PLDM_MONITOR_GEN_ENTER_CMD_10,
    PLDM_MONITOR_GEN_ENTER_CMD_11,
    PLDM_MONITOR_GEN_ENTER_CMD_12,
    PLDM_MONITOR_GEN_ENTER_CMD_13,
    PLDM_MONITOR_GEN_ENTER_CMD_15,
    PLDM_MONITOR_GEN_ENTER_CMD_20,
    PLDM_MONITOR_GEN_ENTER_CMD_21,
    PLDM_MONITOR_GEN_ENTER_CMD_50,
    PLDM_MONITOR_GEN_GET_PDR,
    PLDM_MONITOR_GEN_ENTER_CMD_53,
    PLDM_MONITOR_GEN_RECV_EVENT,
    PLDM_MONITOR_GEN_NEED_POLL_EVENT,
    PLDM_MONITOR_GEN_ENTER_CMD_UNKNOW,
} pldm_monitor_gen_event_id_t;

#pragma pack(1)

void pldm_monitor_gen(int cmd, u8 *buf);
void pldm_monitor_gen_recv(int cmd, u8 *buf);

void pldm_monitor_gen_init(void);
u8 pldm_monitor_state_transform_switch(u8 cnt, u8 *buf);

pldm_get_pdr_rsp_dat_t pldm_monitor_get_pdr_rsp_dat(void);
pldm_poll_for_platform_event_msg_rsp_dat_t pldm_monitor_get_pull_event_rsp_dat(void);

#endif /* __PLDM_MONITOR_GEN_H__ */