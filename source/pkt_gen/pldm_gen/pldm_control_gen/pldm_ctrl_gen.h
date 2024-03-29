#ifndef __PLDM_CTRL_GEN_H__
#define __PLDM_CTRL_GEN_H__

#include "main.h"

typedef enum {
    PLDM_CTRL_GEN_IDLE = 0,
} plfm_ctrl_gen_gen_state_t;

typedef enum {
    PLDM_CTRL_GEN_UNKNOW = 0,
    PLDM_CTRL_GEN_ENTER_CMD_01 = 1,
    PLDM_CTRL_GEN_ENTER_CMD_02,
    PLDM_CTRL_GEN_ENTER_CMD_03,
    PLDM_CTRL_GEN_ENTER_CMD_04,
    PLDM_CTRL_GEN_ENTER_CMD_05,
    PLDM_CTRL_GEN_ENTER_CMD_UNKNOW,
} plfm_ctrl_gen_event_id_t;

void pldm_ctrl_gen_init(void);
void pldm_ctrl_gen(int cmd, u8 *buf);

u8 pldm_ctrl_state_transform_switch(u8 cnt, u8 *buf);
void pldm_ctrl_gen_recv(int cmd, u8 *buf);

#endif /* __PLDM_CTRL_GEN_H__ */