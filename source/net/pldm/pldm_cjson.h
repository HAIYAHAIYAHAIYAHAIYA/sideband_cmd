#ifndef __PLDM_CJSON_H__
#define __PLDM_CJSON_H__

#include "main.h"
#include "pldm_redfish.h"

#define PLDM_CJSON_POLL_SIZE                    (9 * 512)

/* Cable inserted into port. */
#define REGISTRY_CABLEINSERTED                  CBIT(0)             /* Currently not supported */
/* Cable removed from port.  */
#define REGISTRY_CABLEREMOVED                   CBIT(1)             /* Currently not supported */
/* Link down. */
#define REGISTRY_CONNECTIONDROPPED              CBIT(2)
/* Link UP. */
#define REGISTRY_CONNECTIONESTABLISHED          CBIT(3)
/* Link established on a lower speed than is supported by both the device and the Link Partner. */
#define REGISTRY_DEGRADEDCONNECTIONESTABLISHED  CBIT(4)             /* Currently not supported */
/* Network connection has repeatedly been established and dropped. */
#define REGISTRY_LINKFLAPDETECTED               CBIT(5)

#pragma pack(1)

typedef struct {
    u8 seq;
    u8 fmt;
    u16 len;
    char *val;
} pldm_bej_sflv_dat_t;

typedef struct {
    u8 schema_type;     /* is anno : 1, is major : 0 */
    u8 fmt;
    u8 child_cnt;
    char *key;
    char *val;
} pldm_cjson_schema_fmt_t;

typedef struct pldm_cjson {
    struct pldm_cjson *next;
    struct pldm_cjson *child;
    char *name;
    pldm_bej_sflv_dat_t sflv;
} pldm_cjson_t;

#pragma pack()

typedef pldm_cjson_t *(*schema_create)(u8 *dict, u8 *anno_dict);

void pldm_cjson_pool_init(void);
void pldm_cjson_pool_reinit(void);
pldm_cjson_t *pldm_cjson_create_obj(void);
void pldm_cjson_delete_node(pldm_cjson_t *node);

pldm_cjson_t *pldm_cjson_read(pldm_cjson_t *root, u16 collection_skip, u16 collection_top);
pldm_cjson_t *pldm_cjson_update(pldm_cjson_t *root, pldm_cjson_t *update_node);
pldm_cjson_t *pldm_cjson_replace(pldm_cjson_t *root, pldm_cjson_t *replace_node);
pldm_cjson_t *pldm_cjson_action(pldm_cjson_t *root);
pldm_cjson_t *pldm_cjson_head(pldm_cjson_t *root);
u16 pldm_cjson_cal_len_to_root(pldm_cjson_t *root, u8 op_type);
void pldm_cjson_printf_root(pldm_cjson_t *root);

pldm_cjson_t *pldm_cjson_add_item_to_obj(pldm_cjson_t *obj, pldm_bej_sflv_dat_t *sflv, char *name, char *val, u8 val_len);
void pldm_cjson_add_enum_to_obj(pldm_cjson_t *obj, u8 *dictionary, pldm_bej_sflv_dat_t *sflv, char *enum_name, char *enum_val);

pldm_cjson_t *pldm_cjson_create_event_schema(u32 resource_id, u8 *dict, u8 *anno_dict, u8 link_state);
#endif /* __PLDM_CJSON_H__ */