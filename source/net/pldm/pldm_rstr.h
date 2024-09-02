#ifndef __PLDM_RSTR_H__
#define __PLDM_RSTR_H__

#include "main.h"
#include "pldm_cjson.h"

#pragma pack(1)

typedef struct {
    u8 data_type;
    u32 i_val;
    // pldm_real f_val;
    char *string;
} pldm_rstr_val_t;

typedef struct {
    char *name;
    pldm_rstr_val_t val;
} pldm_rstr_field_t;

#pragma pack()

void pldm_rstr_update_redfish_resource(pldm_cjson_t *root, u8 resource_identify);

#endif /* __PLDM_RSTR_H__ */