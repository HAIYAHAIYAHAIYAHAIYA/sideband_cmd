#ifndef __PLDM_BEJ_RESOLVE_H__
#define __PLDM_BEJ_RESOLVE_H__

#include "main.h"
#include "pldm_cjson.h"

typedef float pldm_real;

#pragma pack(1)

typedef struct {
    u8 seq;
    u8 fmt;
    u16 len;
    u16 sflv_len;
    u8 *val;
} pldm_bej_sflv_t;

typedef struct {
    u8 len;         /* max len is 0xFF */
    u8 data[0];     /* LSB -> MSB */
} pldm_bej_nnint_t;

typedef struct {
    u8 len;
    char *val;
} pldm_bej_key_t;

#pragma pack()

typedef enum {
    BEJ_SET = 0,
    BEJ_ARRAY,
    BEJ_NULL,
    BEJ_INT,
    BEJ_ENUM,
    BEJ_STR,
    BEJ_REAL,
    BEJ_BOOLEAN,
    BEJ_BYTE_STR,
    BEJ_CHOICE,
    BEJ_PROPERTY_ANNO,
    BEJ_REGISTRY_ITEM,
    BEJ_RESOURCE_LINK = 0xE,
    BEJ_RESOURCE_LINK_EXPANSION = 0xF,
} pldm_bej_fmt_t;

void pldm_bej_init(void);
u8 *pldm_bej_encode(pldm_cjson_t *root, u8 *bej_buf);
pldm_cjson_t *pldm_bej_decode(u8 *buf, u16 buf_len, u8 *anno_dict, u8 *dict, pldm_cjson_t *root, u8 is_full_schema);
pldm_cjson_t *pldm_bej_get_match_node(void);
void pldm_bej_fill_name(pldm_cjson_t *schema_root, pldm_cjson_t *bej_root);

u8 pldm_bej_u32_to_bejinteger(u32 num, u8 *buf, u8 is_nnint);
u8 pldm_bej_float_to_bejreal(pldm_real num, u8 *buf);

#endif /* __PLDM_BEJ_RESOLVE_H__ */