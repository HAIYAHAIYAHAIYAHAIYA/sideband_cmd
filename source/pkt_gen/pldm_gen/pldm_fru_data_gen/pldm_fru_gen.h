#ifndef __PLDM_FRU_GEN_H__
#define __PLDM_FRU_GEN_H__

#include "main.h"
#include "pldm_fru_data.h"

#define PLDM_FRU_CMD_GEN(num)                  pldm_fru_gen_cmd_ ## num

typedef enum {
    PLDM_FRU_GEN_IDLE = 0,
} pldm_FRU_gen_gen_state_t;

typedef enum {
    PLDM_FRU_GEN_UNKNOW = 0,
    PLDM_FRU_GEN_ENTER_CMD_01 = 1,
    PLDM_FRU_GEN_ENTER_CMD_02,
    PLDM_FRU_GEN_ENTER_CMD_UNKNOW,
} pldm_FRU_gen_event_id_t;

void pldm_fru_gen(int cmd, u8 *buf);
void pldm_fru_gen_init(void);
u8 pldm_fru_state_transform_switch(u8 cnt, u8 *buf);
void pldm_fru_gen_recv(int cmd, u8 *buf);

pldm_fru_get_fru_record_table_rsp_dat_t pldm_fru_get_fru_record_table_rsp_dat(void);

#endif /* __PLDM_FRU_GEN_H__ */