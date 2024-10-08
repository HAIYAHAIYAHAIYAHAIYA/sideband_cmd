#include "pldm_cjson.h"
#include "pldm_bej_resolve.h"
#include "pldm_monitor.h"

static u8 gs_pldm_cjson_buf[PLDM_CJSON_POLL_SIZE];
static u8 *gs_pldm_cjson_pool;
static u16 gs_pldm_cjson_wt;

extern pldm_nic_composite_state_sensor_data_struct_t nic_composite_state_sensors[4];
extern pldm_controller_composite_state_sensor_data_struct_t controller_composite_state_sensors[5];

void pldm_cjson_pool_init(void)
{
    gs_pldm_cjson_pool = gs_pldm_cjson_buf;
    gs_pldm_cjson_wt = 0;
}

void pldm_cjson_pool_reinit(void)
{
    gs_pldm_cjson_wt = 0;
}

void *pldm_cjson_malloc(u16 size)
{
    size = ALIGN(size, 1);
    if (gs_pldm_cjson_wt + size >= PLDM_CJSON_POLL_SIZE) {
        LOG("no more space");
        return NULL;
    }
    u8 *pt = gs_pldm_cjson_pool + gs_pldm_cjson_wt;
    gs_pldm_cjson_wt += size;
    cm_memset((void *)pt, 0, size);
    return pt;
}

void pldm_cjson_free(u8 *addr)
{
    if (!addr) return;
}

void pldm_cjson_delete_node(pldm_cjson_t *node)
{
    if (!node) return;
    // pldm_cjson_delete_node(node->next);
    // pldm_cjson_delete_node(node->child);
    // pldm_cjson_free((u8 *)node->name);
    // pldm_cjson_free((u8 *)node->sflv.val);
    // node->sflv.val = NULL;
    // node->name = NULL;
    // if (node->next) node->next = NULL;
    // if (node->child) node->child = NULL;
    // pldm_cjson_free((u8 *)node);
    // node = NULL;
}

u16 pldm_cjson_get_used_space(void)
{
    return gs_pldm_cjson_wt;
}

pldm_cjson_t *pldm_cjson_create_obj(void)
{
    pldm_cjson_t *obj = (pldm_cjson_t *)pldm_cjson_malloc(sizeof(pldm_cjson_t));
    if (!obj) return NULL;
    return obj;
}

static pldm_cjson_t *pldm_cjson_add_child(pldm_cjson_t *parent, pldm_cjson_t *child)
{
    if (!parent || !child) return NULL;
    parent->child = child;
    return child;
}

static pldm_cjson_t *pldm_cjson_add_next(pldm_cjson_t *parent, pldm_cjson_t *next)
{
    if (!parent || !next) return NULL;
    pldm_cjson_t *tmp = parent;
    while (tmp->next) {
        tmp = tmp->next;
    }
    tmp->next = next;
    next->next = NULL;
    return next;
}

static pldm_cjson_t *pldm_cjson_add_sflv_attr(pldm_cjson_t *item, pldm_bej_sflv_dat_t *sflv, char *name, char *val, u8 val_len)
{
    if (!item || !sflv || !val || !name) return NULL;
    item->sflv = *sflv;
    item->sflv.len = sflv->len + val_len;
    item->sflv.val = (char *)pldm_cjson_malloc(val_len + 1);
    cm_memcpy(item->sflv.val, val, val_len);
    item->name = (char *)pldm_cjson_malloc(cm_strlen(name) + 1);
    cm_memcpy(item->name, name, cm_strlen(name));
    return item;
}

pldm_redfish_dictionary_entry_t *pldm_cjson_dict_fill_sf(u8 *dict, pldm_redfish_dictionary_entry_t *entry, u16 entry_cnt, pldm_cjson_t *root, u8 name_idx)
{
    if (!dict || !entry) return NULL;
    pldm_redfish_dictionary_entry_t *tmp = entry;
    // LOG("%s, %d\n", &dict[tmp->name_off], entry_cnt);
    // LOG("need 0x%x, 0x%x, 0x%02x", sflv->fmt >> 4, sflv->seq >> 1, sflv->len);

    for (u16 k = 0; k < entry_cnt; k++) {
        char *name = "";
        if (tmp->name_off) {
            name = (char *)&dict[tmp->name_off];
        }
        if (cm_strcmp(name, &(root->name[name_idx])) == 0 && tmp->format >> 4 == root->sflv.fmt >> 4) {
            root->sflv.seq |= (tmp->sequence_num << 1);
            root->sflv.fmt = tmp->format;
            // LOG("%s\n", root->name);
            // LOG("%d\n", tmp->sequence_num);
            return tmp;
        }
        tmp += 1;
    }
    LOG("fmt err : %s", root->name);
    return entry;
}

void pldm_cjson_cal_sf_to_root_op(pldm_cjson_t *root, u8 *anno_dict, u8 *dict, pldm_redfish_dictionary_entry_t *entry, u16 entry_cnt, pldm_redfish_dictionary_entry_t *anno_entry, u16 anno_entry_cnt)
{
    pldm_cjson_t *tmp = root;
    while (tmp) {
        u8 fmt = tmp->sflv.fmt >> 4;
        u8 child_cnt = entry_cnt;
        u8 anno_child_cnt = anno_entry_cnt;
        pldm_redfish_dictionary_entry_t *new_entry = entry;
        pldm_redfish_dictionary_entry_t *new_anno_entry = anno_entry;

        if (tmp->sflv.seq & 1) {
            u8 name_len = cm_strlen(tmp->name) + 1;
            u8 is_find = 0;
            for (u8 j = 0; j < name_len; j++) {
                if (tmp->name[j] == '@') {
                    new_anno_entry = pldm_cjson_dict_fill_sf(anno_dict, anno_entry, anno_entry_cnt, tmp, j);
                    is_find = 1;
                    break;
                }
            }
            if (!is_find)
                new_anno_entry = pldm_cjson_dict_fill_sf(anno_dict, anno_entry, anno_entry_cnt, tmp, 0);
        } else {
            new_entry = pldm_cjson_dict_fill_sf(dict, entry, entry_cnt, tmp, 0);
        }
        if (fmt == BEJ_SET || fmt == BEJ_ARRAY) {
            child_cnt = new_entry->child_cnt;
            new_entry = (pldm_redfish_dictionary_entry_t *)&dict[new_entry->childpoint_off];

            if (new_anno_entry->childpoint_off && tmp->sflv.seq & 1) {
                anno_child_cnt = new_anno_entry->child_cnt;
                new_anno_entry = (pldm_redfish_dictionary_entry_t *)&anno_dict[new_anno_entry->childpoint_off];
            }
        }
        pldm_cjson_cal_sf_to_root_op(tmp->child, anno_dict, dict, new_entry, child_cnt, new_anno_entry, anno_child_cnt);
        tmp = tmp->next;
    }
}

void pldm_cjson_cal_sf_to_root(pldm_cjson_t *root, u8 *anno_dict, u8 *dict)
{
    if (!root || !anno_dict || !dict) return;
    pldm_redfish_dictionary_format_t *dict_ptr = (pldm_redfish_dictionary_format_t *)dict;
    pldm_redfish_dictionary_format_t *anno_dict_ptr = (pldm_redfish_dictionary_format_t *)anno_dict;
    pldm_cjson_cal_sf_to_root_op(root, anno_dict, dict, &(dict_ptr->entry[0]), dict_ptr->entry_cnt, &(anno_dict_ptr->entry[0]), anno_dict_ptr->entry_cnt);
}

u16 pldm_cjson_cal_len_to_root(pldm_cjson_t *root, u8 op_type)
{
    u16 len = 0;
    pldm_cjson_t *tmp = root;
    while (tmp) {
        u8 fmt = tmp->sflv.fmt >> 4;
        if (fmt == BEJ_SET || fmt == BEJ_ARRAY)
        {
            tmp->sflv.len = 0;
            tmp->sflv.len += 2;
        }
        tmp->sflv.len += pldm_cjson_cal_len_to_root(tmp->child, op_type);
        if (op_type != HEAD) {
            if (!(tmp->child) && fmt != BEJ_SET && fmt != BEJ_ARRAY) {
                // if (tmp->name) LOG("seq : %#x, fmt : 0x%02x, len : %d, name : %s : ", tmp->sflv.seq, tmp->sflv.fmt, tmp->sflv.len, tmp->name);
                if (fmt == BEJ_STR) tmp->sflv.len = cm_strlen(tmp->sflv.val);
                if (!(tmp->sflv.seq & 1) && fmt == BEJ_STR) tmp->sflv.len += 1;     /* major schema */
            }
        }
        len += tmp->sflv.len + 5;      /* sfl len */
        if (tmp->sflv.len > 0xFF)
            len += 1;
        tmp = tmp->next;
    }
    return len;
}

void pldm_cjson_printf_root(pldm_cjson_t *root)
{
    pldm_cjson_t *tmp = root;
    while (tmp) {
        LOG("seq : 0x%02x, fmt : 0x%02x, len : %d, name : %s : ", tmp->sflv.seq, tmp->sflv.fmt, tmp->sflv.len, tmp->name);
        pldm_cjson_printf_root(tmp->child);
        if (!tmp->child && tmp->sflv.val) {
            u8 fmt = tmp->sflv.fmt >> 4;
            for (u16 i = 0; i < tmp->sflv.len; i++) {
                switch (fmt) {
                    case BEJ_REAL:
                    case BEJ_INT:
                        printf("%d ", tmp->sflv.val[i]);
                        break;
                    case BEJ_ENUM:
                        printf("0x%x ", tmp->sflv.val[i]);
                        break;
                    default:
                        printf("%c", tmp->sflv.val[i]);
                        break;
                }
            }
            printf("\n");
        }
        tmp = tmp->next;
    }
}

pldm_cjson_t *g_is_find = NULL;
static void pldm_cjson_find_name(pldm_cjson_t *root, pldm_bej_sflv_dat_t *sflv, char *name)
{
    if (!root || !name) return;
    pldm_cjson_t *tmp = root;
    while (tmp) {
        u8 tmp_seq = tmp->sflv.seq >> 1;
        u8 tmp_fmt = tmp->sflv.fmt >> 4;
        if (cm_strcmp(tmp->name, name) == 0 && tmp_fmt == sflv->fmt >> 4 && tmp_seq == sflv->seq >> 1) {
            g_is_find = tmp;
            return;
        }
        pldm_cjson_find_name(tmp->child, sflv, name);
        if (g_is_find) {
            break;
        }
        tmp = tmp->next;
    }
}

/* skip and top param only for collection schema */
int pldm_cjson_read(pldm_cjson_t *root, u16 collection_skip, u16 collection_top)
{
    if (!root) return -1;
    if (collection_skip == 0 && collection_top == 0xFFFF) return 0;
    g_is_find = NULL;
    pldm_bej_sflv_dat_t sflv;
    sflv.fmt = BEJ_ARRAY << 4;
    sflv.seq = 1 << 1;
    pldm_cjson_find_name(root, &sflv, "Members");
    if (g_is_find) {
        if (collection_top == 0 || collection_skip >= MAX_LAN_NUM) {
            /* empty list */
            g_is_find->child = NULL;
            return 0;
        }
        pldm_cjson_t *start_member = g_is_find->child;
        pldm_cjson_t *end_member = NULL;
        g_is_find->child = NULL;
        for (u8 i = 0; i < collection_skip; i++) {
            if (start_member->next)
                start_member = start_member->next;
        }
        g_is_find->child = start_member;
        end_member = start_member;

        /* If the parameter for $top  exceeds the remaining number of members in a resource collection, the number returned shall be 
            truncated to those remaining. */
        u8 cnt = MIN((MAX_LAN_NUM - collection_skip), collection_top);

        for (u8 i = 0; i < cnt; i++) {
            if ( i != (cnt - 1)) {
                if (end_member->next)
                    end_member = end_member->next;
            } else {
                end_member->next = NULL;
            }

        }
    }
    return 0;
}

static void pldm_cjson_update_op(pldm_cjson_t *start_node, pldm_cjson_t *update_node)
{
    pldm_cjson_t *ori_node = start_node;
    pldm_cjson_t *new_node = update_node;
    while (new_node && ori_node) {
        pldm_cjson_t *tmp = ori_node;
        for (; tmp; tmp = tmp->next) {
            if (cm_strcmp(tmp->name, new_node->name) == 0) {
                u8 fmt = new_node->sflv.fmt >> 4;
                if (fmt == BEJ_SET || fmt == BEJ_ARRAY) break;
                u16 ori_val_len = tmp->sflv.len;
                u16 new_val_len = new_node->sflv.len;
                if ((tmp->sflv.fmt >> 4) == BEJ_STR) {
                    ori_val_len -= 1;
                    new_val_len -= 1;
                }
                if (ori_val_len < new_val_len)
                    tmp->sflv.val = pldm_cjson_malloc(new_val_len + 1);
                tmp->sflv.len = new_val_len;
                cm_memcpy(tmp->sflv.val, new_node->sflv.val, new_val_len);
                break;
            }
        }
        pldm_cjson_update_op(tmp->child, new_node->child);
        new_node = new_node->next;
    }
}

int pldm_cjson_update(pldm_cjson_t *root, pldm_cjson_t *match_node, pldm_cjson_t *update_node)
{
    if (!root || !update_node || !match_node) return -1;
    g_is_find = NULL;
    pldm_bej_sflv_dat_t sflv;
    sflv.fmt = BEJ_ARRAY << 4;
    sflv.seq = 1 << 1;
    pldm_cjson_find_name(root, &sflv, "Members");
    if (g_is_find)
        return -1;
    pldm_cjson_update_op(match_node, update_node);
    return 0;
}

int pldm_cjson_replace(pldm_cjson_t *root, pldm_cjson_t *replace_node)
{
    if (!root || !replace_node) return -1;
    cm_memcpy(root, replace_node, sizeof(pldm_cjson_t));
    // pldm_cjson_delete_node(root);
    return 0;
}

/* to be determind */
int pldm_cjson_action(pldm_cjson_t *root)
{
    if (!root) return -1;
    g_is_find = NULL;
    pldm_bej_sflv_dat_t sflv;
    sflv.fmt = BEJ_SET << 4;
    sflv.seq = 0 << 1;
    pldm_cjson_find_name(root, &sflv, "Actions");
    if (g_is_find) {
        /* Do action */
        /* Network adapter -> #NetworkAdapter.ResetSettingsToDefault */
        /* Port -> #Port.Reset */
        return 0;
    }
    return -1;
}

/* to be determind */
int pldm_cjson_head(pldm_cjson_t *root)
{
    if (!root) return -1;
    return 0;
}

pldm_cjson_t *pldm_cjson_add_item_to_obj(pldm_cjson_t *obj, pldm_bej_sflv_dat_t *sflv, char *name, char *val, u8 val_len)
{
    if (!obj || !sflv || !name || !val) return NULL;
    pldm_cjson_t *item = pldm_cjson_create_obj();
    pldm_cjson_add_sflv_attr(item, sflv, name, val, val_len);
    if (!(obj->child)) {
        pldm_cjson_add_child(obj, item);
    } else {
        pldm_cjson_add_next(obj->child, item);
    }
    return item;
}

void pldm_cjson_add_enum_to_obj(pldm_cjson_t *obj, u8 *dictionary, pldm_bej_sflv_dat_t *sflv, char *enum_name, char *enum_val)
{
    if (!obj || !sflv || !enum_name || !enum_val) return;
    if ((cm_strcmp("", enum_val) == 0)) {
        pldm_cjson_add_item_to_obj(obj, sflv, enum_name, "", 0);
        return;
    }
    pldm_redfish_dictionary_format_t *dict = (pldm_redfish_dictionary_format_t *)dictionary;
    for (u16 i = 0; i < dict->entry_cnt; i++) {
        u8 *name = &dictionary[dict->entry[i].name_off];
        if (cm_strcmp((char *)name, enum_name) == 0) {
            pldm_redfish_dictionary_entry_t *entry = (pldm_redfish_dictionary_entry_t *)&dictionary[dict->entry[i].childpoint_off];
            for (u8 j = 0; j < dict->entry[i].child_cnt; j++) {
                name = &dictionary[entry->name_off];
                if (cm_strcmp((char *)name, enum_val) == 0) {
                    char str[] = {0x01, 0x00, 0x00};
                    if (!j) {
                        str[1] = 'U';
                    } else {
                        str[1] = j;
                    }
                    pldm_cjson_add_item_to_obj(obj, sflv, enum_name, str, cm_strlen(str));
                    break;
                }
                entry++;
            }
            break;
        }
    }
}

extern u32 crc32_pldm(u32 init_crc, u8 *data, u32 len);
static void pldm_cjson_update_etag_op(pldm_cjson_t *root, u32 *etag)
{
    pldm_cjson_t *tmp = root;
    while (tmp) {
        u8 fmt = tmp->sflv.fmt >> 4;
        char *val = tmp->sflv.val;
        if (cm_strcmp(tmp->name, "@odata.etag") == 0) val = "etagetag";
        *etag = crc32_pldm(*etag ^ 0xFFFFFFFFUL, (u8 *)tmp->name, strlen(tmp->name));
        pldm_cjson_update_etag_op(tmp->child, etag);
        if (!(tmp->child) && fmt != BEJ_SET && fmt != BEJ_ARRAY) {
            *etag = crc32_pldm(*etag ^ 0xFFFFFFFFUL, (u8 *)val, strlen(val));
        }
        tmp = tmp->next;
    }
}

char *pldm_cjson_update_etag(pldm_cjson_t *root)
{
    if (!root) return NULL;
    u8 is_find = 0;
    u32 etag = 0;
    char etag_str[8];
    pldm_cjson_t *tmp = root;
    pldm_cjson_update_etag_op(tmp, &etag);
    LOG("pldm_cjson_update_etag : %#x", etag);
    cm_snprintf(etag_str, 8, "%08x", etag);

    for (u8 i = 0; i < 2; i++) {
        if (!i) tmp = root->child;
        else tmp = root;
        while (tmp) {
            // LOG("name : %s", tmp->name);
            if (cm_strcmp(tmp->name, "@odata.etag") == 0) {
                cm_memcpy(tmp->sflv.val, etag_str, 8);
                is_find = 1;
                break;
            }
            tmp = tmp->next;
        }
        if (is_find) break;
    }
    return is_find ? tmp->sflv.val : NULL;
}

extern pldm_redfish_bej_t g_resource_bej[PLDM_REDFISH_RESOURCE_NUM];
extern u32 pldm_redfish_resource_id_to_base(u32 resource_id);

schema_create g_schemas[11];

u8 pldm_cjson_first_get_etag(u8 resource_identify, u32 resource_id, u8 offset, u8 *etag_val)
{
    if (!etag_val) return false;
    pldm_cjson_t *root = NULL;

    root = g_schemas[resource_identify](resource_id);
    if (!root) {
        pldm_cjson_pool_reinit();
        return false;
    }
    char *val = pldm_cjson_update_etag(root);
    if (val) {
        cm_memcpy(etag_val, val, 8);
        g_resource_bej[resource_identify + offset].is_etag = 1;
        cm_memcpy(g_resource_bej[resource_identify + offset].etag, val, 8);
    }

    pldm_cjson_pool_reinit();
    return val ? true : false;
}

u8 pldm_cjson_get_etag(u8 resource_identify, u32 resource_id, varstring *etag)
{
    if (!etag || resource_identify > 10) return false;

    etag->format = UTF_8;
    etag->len = 9;

    u8 ret = true;
    u8 offset = 0;
    u8 new_resource_identify = resource_identify;
    offset = resource_id - pldm_redfish_resource_id_to_base(resource_id);

    /*  NETWORK_DEVICE_FUNC : identify = 6,
        PORT_IDENTIFY,
        PCI_FUNC,
        ETH_INTERFACE, */
    if (resource_identify > 6)
            new_resource_identify += (resource_identify - 6) * (MAX_LAN_NUM - 1);

    if (g_resource_bej[new_resource_identify + offset].is_etag)
        cm_memcpy(etag->val, g_resource_bej[new_resource_identify + offset].etag, 8);
    // else
    //     ret = pldm_cjson_first_get_etag(resource_identify, resource_id, offset, etag->val);
    etag->val[etag->len - 1] = '\0';
    return ret;
}

static void pldm_cjson_fill_comm_field_in_schema(pldm_cjson_t *root, u8 is_collection, u32 id, char *type, u8 resource_identify)
{
    if (!root || !type || resource_identify > 14) return;
    char str[16];
    char uri[128];
    char *schema_uri_prefix = "https://redfish.dmtf.org/schemas/";
    // memset(str, 0, cm_strlen(str));
    pldm_bej_sflv_dat_t sflv;
    str[0] = '%';
    str[1] = 'I';
    cm_snprintf(&str[2], 14, "%d", id);

    char uri_suffix[39];
    pldm_redfish_get_schema_uri_suffix(resource_identify, uri_suffix, SCHEMACLASS_MAJOR);

    u8 uri_prefix_len = cm_strlen(schema_uri_prefix);
    u8 uri_suffix_len = cm_strlen(uri_suffix);
    cm_memcpy(uri, schema_uri_prefix, uri_prefix_len);
    cm_memcpy(&(uri[uri_prefix_len]), uri_suffix, uri_suffix_len);
    uri[uri_prefix_len + uri_suffix_len] = '\0';

    char *key[6] = {
        "@odata.id",                        /* Returns bejResourceLink encoded resource ID. */
        "@odata.type",
        "@odata.etag",
        "@odata.context",
        "Status",
        "Status"
    };

    /* Normal = “OK” */
    char *healthrollupdescription = "OK";
    char *healthrollup = "OK";

    /* If the Card Composite State Sensor (sensorID = 5) is 
    uninitialized, Network Controller State sensor
    (sensorID = 50) shall be used instead. If this sensor 
    too is uninitialized, the operation fails. */

    pldm_state_data_struct_t *state_data = NULL;
    if (nic_composite_state_sensors[0].op_state == PLDM_OP_ENABLE) {
        state_data = &nic_composite_state_sensors[0];
    } else if (controller_composite_state_sensors[0].op_state == PLDM_OP_ENABLE) {
        state_data = &controller_composite_state_sensors[0];
    } else {
        LOG("all state sensor uninitialized, the operation fails.");
    }
    /* 0 : Critical, 1 : OK, 2 : Warning */
    char enum_str[3];
    enum_str[0] = 0x01;
    enum_str[1] = 0x01;
    enum_str[2] = 0x00;
    if (state_data) {
        switch (state_data->present_state) {
            case UPPER_NON_CRITICAL:
                enum_str[1] = 0x02;
                break;
            case UPPER_CRITICAL:
            case FATAL:
                enum_str[1] = 'U';
                break;
            default :
            break;
        }
    }
    healthrollupdescription = enum_str;
    healthrollup = enum_str;

    char *val[6] = {str, type, "etagetag", uri, healthrollupdescription, healthrollup};
    u8 cnt = is_collection ? 4 : 6;
    for (u8 i = 0; i < cnt; i++) {
        sflv.len = 0;
        if (i < 4) {
            sflv.seq = 1;
            sflv.fmt = BEJ_STR << 4;
            pldm_cjson_add_item_to_obj(root, &sflv, key[i], val[i], cm_strlen(val[i]));
        } else {
            sflv.seq = 0;
            pldm_bej_sflv_dat_t f_sflv;
            f_sflv.fmt = BEJ_SET << 4;
            f_sflv.seq = 0 << 1;
            g_is_find = NULL;
            pldm_cjson_find_name(root, &f_sflv, key[i]);
            if (g_is_find) {
                sflv.fmt = BEJ_ENUM << 4;
                pldm_cjson_add_item_to_obj(g_is_find, &sflv, "HealthRollup", val[i], 2);
            } else {
                sflv.fmt = BEJ_SET << 4;
                pldm_cjson_t *tmp = pldm_cjson_add_item_to_obj(root, &sflv, key[i], "", 0);
                sflv.fmt = BEJ_ENUM << 4;
                pldm_cjson_add_item_to_obj(tmp, &sflv, "HealthRollup", val[i], 2);
            }
        }
    }
}

void pldm_cjson_fill_comm_field_in_schema_update(pldm_cjson_t *root, u8 is_collection, u32 id, char *type, u8 resource_identify)
{
    if (!root || !type || resource_identify > 14) return;
    char str[16];
    char uri[128];
    char *schema_uri_prefix = "https://redfish.dmtf.org/schemas/";
    // memset(str, 0, cm_strlen(str));
    pldm_bej_sflv_dat_t sflv;
    str[0] = '%';
    str[1] = 'I';
    cm_snprintf(&str[2], 14, "%d", id);

    char uri_suffix[39];
    pldm_redfish_get_schema_uri_suffix(resource_identify, uri_suffix, SCHEMACLASS_MAJOR);

    u8 uri_prefix_len = cm_strlen(schema_uri_prefix);
    u8 uri_suffix_len = cm_strlen(uri_suffix);
    cm_memcpy(uri, schema_uri_prefix, uri_prefix_len);
    cm_memcpy(&(uri[uri_prefix_len]), uri_suffix, uri_suffix_len);
    uri[uri_prefix_len + uri_suffix_len] = '\0';

    char *key[6] = {
        "@odata.id",                        /* Returns bejResourceLink encoded resource ID. */
        "@odata.type",
        "@odata.etag",
        "@odata.context",
        "Status",
        "Status"
    };

    /* Normal = “OK” */
    char *healthrollupdescription = "OK";
    char *healthrollup = "OK";

    /* If the Card Composite State Sensor (sensorID = 5) is 
    uninitialized, Network Controller State sensor
    (sensorID = 50) shall be used instead. If this sensor 
    too is uninitialized, the operation fails. */

    pldm_state_data_struct_t *state_data = NULL;
    if (nic_composite_state_sensors[0].op_state == PLDM_OP_ENABLE) {
        state_data = &nic_composite_state_sensors[0];
    } else if (controller_composite_state_sensors[0].op_state == PLDM_OP_ENABLE) {
        state_data = &controller_composite_state_sensors[0];
    } else {
        LOG("all state sensor uninitialized, the operation fails.");
    }
    /* 0 : Critical, 1 : OK, 2 : Warning */
    char enum_str[3];
    enum_str[0] = 0x01;
    enum_str[1] = 0x01;
    enum_str[2] = 0x00;
    if (state_data) {
        switch (state_data->present_state) {
            case UPPER_NON_CRITICAL:
                enum_str[1] = 0x02;
                break;
            case UPPER_CRITICAL:
                enum_str[1] = 'U';
                break;
            case FATAL:
                enum_str[1] = 'U';
                break;
            default :
            break;
        }
    }
    healthrollupdescription = enum_str;
    healthrollup = enum_str;

    char *val[6] = {str, type, "etagetag", uri, healthrollupdescription, healthrollup};
    u8 cnt = is_collection ? 4 : 6;
    for (u8 i = 0; i < cnt; i++) {
        if (i < 4) {
            sflv.seq = 1;
            sflv.fmt = BEJ_STR << 4;
            pldm_cjson_add_item_to_obj(root, &sflv, key[i], val[i], cm_strlen(val[i]));
        } else {
            sflv.seq = 0;
            sflv.fmt = BEJ_SET << 4;
            pldm_cjson_t *tmp = pldm_cjson_add_item_to_obj(root, &sflv, key[i], "", 0);
            sflv.fmt = BEJ_ENUM << 4;
            pldm_cjson_add_item_to_obj(tmp, &sflv, "HealthRollup", val[i], 2);
        }
    }
}

pldm_cjson_schema_fmt_t *pldm_cjson_create_schema(pldm_cjson_t *obj, pldm_cjson_schema_fmt_t *fmt)
{
    if (!obj || !fmt)  return NULL;
    pldm_bej_sflv_dat_t sflv;
    pldm_cjson_t *tmp = obj;
    pldm_cjson_t *tmp1 = NULL;
    pldm_cjson_schema_fmt_t *buf = fmt;
    u8 cnt = 0;
    sflv.seq = buf[0].schema_type;
    sflv.fmt = buf[0].fmt << 4;
    sflv.len = 0;
    // LOG("fmt : %d, cnt : %d\n", buf[0].fmt, buf[0].child_cnt);
    if (buf[0].fmt == BEJ_SET || buf[0].fmt == BEJ_ARRAY) {
        cnt = buf[0].child_cnt;
        // LOG("name : %s\n", tmp->name);
        tmp1 = pldm_cjson_add_item_to_obj(tmp, &sflv, buf->key, "", 0);
        buf += 1;
    }
    for (u8 i = 0; i < cnt; i++) {
        buf = pldm_cjson_create_schema(tmp1, buf);
        // LOG("cnt %d\n", len);
    }
    if (!cnt && BEJ_SET != sflv.fmt >> 4 && BEJ_ARRAY != sflv.fmt >> 4) {
        pldm_cjson_add_item_to_obj(tmp, &sflv, buf->key, buf->val, cm_strlen(buf->val));
        buf += 1;
    }
    return buf;
}

pldm_cjson_t *pldm_cjson_create_event_schema(u32 resource_id, u8 link_state)
{
    char str[10];
    str[0] = '%';
    str[1] = 'I';
    cm_snprintf(&(str[2]), 8, "%d", resource_id);
    u8 strlen_len = cm_strlen(str);
    str[strlen_len] = '\0';
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 4, "Event", ""},
            {0, BEJ_ARRAY, 1, "Events", ""},
                {0, BEJ_SET, 5, "", ""},
                    {0, BEJ_ENUM, 0, "EventType", (char [3]){0x01, 0x04, 0x00}},
                        // {0, BEJ_STR, 0, "StatusChange", ""},
                        // {0, BEJ_STR, 0, "ResourceUpdated", ""},
                        // {0, BEJ_STR, 0, "ResourceAdded", ""},
                        // {0, BEJ_STR, 0, "ResourceRemoved", ""},
                        // {0, BEJ_STR, 0, "Alert", ""},
                    {0, BEJ_STR, 0, "MemberId", "0"},                   /* Event member element ID within the Events array. */
                    {0, BEJ_ARRAY, 1, "MessageArgs", ""},               /* Array of strings containing the arguments needed for the Registry message */
                        {0, BEJ_STR, 0, "", ""},
                    {0, BEJ_STR, 0, "MessageId", "NetworkDevice.1.0.1."},
                    {0, BEJ_SET, 1, "OriginOfCondition", ""},
                        {1, BEJ_STR, 0, "@odata.id", str},                     /* Reference to related triggering resource. */
            {1, BEJ_INT, 0, "@odata.count", (char [2]){0x01, 0x00}},
            {0, BEJ_STR, 0, "Id", "1"},
            {0, BEJ_STR, 0, "Name", "Event"},
    };

    if (link_state) {
        fmt[7].val = "NetworkDevice.1.0.1.ConnectionEstablished";
    } else {
        fmt[7].val = "NetworkDevice.1.0.1.ConnectionDropped";
    }

    pldm_cjson_create_schema(root, fmt);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    pldm_cjson_fill_comm_field_in_schema(new_root, 1, resource_id, "Event.1_0_2.Event", EVENT);
    // pldm_cjson_cal_sf_to_root(new_root, anno_dict, dict);
    return new_root;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_port_v1_3_1_schema(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 1, "", ""},
            {0, BEJ_SET, 14, "Port", ""},
                {0, BEJ_SET, 2, "Actions", ""},
                    {0, BEJ_SET, 2, "#Port.Reset", ""},
                        {0, BEJ_STR, 0, "target", "%T10.0"},
                        {0, BEJ_STR, 0, "title", "#Port.Reset"},
                    {0, BEJ_SET, 1, "#Port.ResetPPB", ""},
                        {0, BEJ_STR, 0, "title", "#Port.Reset"},
                {0, BEJ_REAL, 0, "CurrentSpeedGbps", (char [3]){0x01, 0x20, 0x00}},
                {0, BEJ_SET, 3, "Ethernet", ""},
                    {0, BEJ_ENUM, 0, "FlowControlConfiguration", (char [3]){0x01, 0x01, 0x00}},
                        // {0, BEJ_STR, 0, "TX", ""},
                        // {0, BEJ_STR, 0, "RX", ""},
                        // {0, BEJ_STR, 0, "TX_RX", ""},
                    {0, BEJ_ENUM, 0, "FlowControlStatus", (char [3]){0x01, 0x01, 0x00}},
                        // {0, BEJ_STR, 0, "TX", ""},
                        // {0, BEJ_STR, 0, "RX", ""},
                        // {0, BEJ_STR, 0, "TX_RX", ""},
                    {0, BEJ_ARRAY, 1, "SupportedEthernetCapabilities", ""},
                        {0, BEJ_ENUM, 0, "", (char [3]){0x01, 0x01, 0x00}},
                            // {0, BEJ_STR, 0, "WakeOnLAN", ""},
                            // {0, BEJ_STR, 0, "EEE", ""},
                {0, BEJ_STR, 0, "Id", "Resource offset"},
                {0, BEJ_BOOLEAN, 0, "InterfaceEnabled", "t"},
                {0, BEJ_ARRAY, 1, "LinkConfiguration", ""},
                    {0, BEJ_SET, 3, "", ""},
                        {0, BEJ_BOOLEAN, 0, "AutoSpeedNegotiationCapable", "t"},
                        {0, BEJ_BOOLEAN, 0, "AutoSpeedNegotiationEnabled", "t"},
                        {0, BEJ_ARRAY, 2, "CapableLinkSpeedGbps", (char [3]){0x01, 0x20, 0x00}},
                            {0, BEJ_REAL, 0, "", "Gbs"},
                            {0, BEJ_REAL, 0, "", "Gbs"},
                {0, BEJ_ENUM, 0, "LinkNetworkTechnology", (char [3]){0x01, 'U', 0x00}},
                    // {0, BEJ_STR, 0, "Ethernet", ""},
                    // {0, BEJ_STR, 0, "InfiniBand", ""},
                    // {0, BEJ_STR, 0, "FibreChannel", ""},
                    // {0, BEJ_STR, 0, "GenZ", ""},
                {0, BEJ_ENUM, 0, "LinkState", (char [3]){0x01, 'U', 0x00}},
                    // {0, BEJ_STR, 0, "Enabled", ""},
                    // {0, BEJ_STR, 0, "Disabled", ""},
                {0, BEJ_ENUM, 0, "LinkStatus", (char [3]){0x01, 0x01, 0x00}},
                    // {0, BEJ_STR, 0, "LinkUp", ""},
                    // {0, BEJ_STR, 0, "Starting", ""},
                    // {0, BEJ_STR, 0, "Training", ""},
                    // {0, BEJ_STR, 0, "LinkDown", ""},
                    // {0, BEJ_STR, 0, "NoLink", ""},
                {0, BEJ_INT, 0, "LinkTransitionIndicator", (char [3]){0x01, 0x01, 0x00}},
                {0, BEJ_INT, 0, "MaxFrameSize", "?"},
                {0, BEJ_REAL, 0, "MaxSpeedGbps", (char [3]){0x01, 50, 0x00}},
                {0, BEJ_STR, 0, "Name", "AM_Port"},
                {0, BEJ_SET, 1, "Status", ""},
                    {0, BEJ_ENUM, 0, "State", (char [3]){0x01, 0x01, 0x00}},
                    // {0, BEJ_STR, 0, "Absent", ""},
                    // {0, BEJ_STR, 0, "Enabled", ""},
                    // {0, BEJ_STR, 0, "Disabled", ""},
    };
    pldm_cjson_create_schema(root, fmt);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    pldm_cjson_fill_comm_field_in_schema(new_root->child, 0, resource_id, "Port.1_3_1.Port", PORT_IDENTIFY);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_portcollection_schema(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 1, "", ""},
            {0, BEJ_SET, 3, "PortCollection", ""},
                {0, BEJ_STR, 0, "Name", "Ports"},
                {1, BEJ_INT, 0, "@odata.count", (char [2]){MAX_LAN_NUM, 0x00}},
                {0, BEJ_ARRAY, MAX_LAN_NUM, "Members", ""},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I100"},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I101"},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I102"},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I103"}
    };
    pldm_cjson_create_schema(root, fmt);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    pldm_cjson_fill_comm_field_in_schema(new_root->child, 1, PLDM_BASE_PORTS_RESOURCE_ID, "PortCollection.PortCollection", PORT_COLLECTION);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_networkinterface_v1_2_0_schema(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 1, "", ""},
            {0, BEJ_SET, 7, "NetworkInterface", ""},
                {0, BEJ_SET, 1, "Links", ""},
                    {0, BEJ_SET, 0, "NetworkAdapter", ""},
                        // {0, BEJ_STR, 0, "", (char [2]){PLDM_BASE_NETWORK_ADAPTER_RESOURCE_ID, 0x00}},
                {0, BEJ_STR, 0, "Name", "AM_Network_Interface"},
                {0, BEJ_SET, 0, "NetworkDeviceFunctions", "Ports"},
                {0, BEJ_SET, 0, "NetworkPorts", ""},
                    // {0, BEJ_STR, 0, "", (char [2]){PLDM_BASE_NETWORK_DEV_FUNCS_RESOURCE_ID, 0x00}},
                {0, BEJ_SET, 0, "Ports", ""},
                    // {0, BEJ_STR, 0, "", (char [2]){PLDM_BASE_PORTS_RESOURCE_ID, 0x00}},
                {0, BEJ_SET, 1, "Status", ""},
                    {0, BEJ_ENUM, 0, "State", (char [3]){0x01, 0x01, 0x00}},
                        // {0, BEJ_STR, 0, "StandbyOffline", ""},
                        // {0, BEJ_STR, 0, "Starting", ""},
                        // {0, BEJ_STR, 0, "Updating", ""},
                        // {0, BEJ_STR, 0, "Enabled", ""},
                        // {0, BEJ_STR, 0, "Disabled", ""},
                {0, BEJ_STR, 0, "Id", "%I5"}                        /* PLDM_BASE_NETWORK_INTERFACE_RESOURCE_ID */
    };
    pldm_cjson_create_schema(root, fmt);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    pldm_cjson_fill_comm_field_in_schema(new_root->child, 0, PLDM_BASE_NETWORK_INTERFACE_RESOURCE_ID, "NetworkInterface.1_2_1.NetworkInterface", NETWORK_INTERFACE);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_networkadapter_v1_5_0_schema(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        {0, BEJ_SET, 1, "", ""},
            {0, BEJ_SET, 12, "NetworkAdapter", ""},
                {0, BEJ_SET, 1, "Actions", ""},
                    {0, BEJ_SET, 1, "#NetworkAdapter.ResetSettingsToDefault", ""},
                        {0, BEJ_STR, 0, "target", "?"},
                {0, BEJ_ARRAY, 1, "Controllers", ""},
                    {0, BEJ_SET, 4, "", ""},
                        {0, BEJ_SET, 4, "ControllerCapabilities", ""},
                            {0, BEJ_SET, 1, "DataCenterBridging", ""},
                                {0, BEJ_BOOLEAN, 0, "Capable", "t"},
                            {0, BEJ_INT, 0, "NetworkDeviceFunctionCount", (char [2]){MAX_LAN_NUM, 0x00}},
                            {0, BEJ_INT, 0, "NetworkPortCount", (char [2]){MAX_LAN_NUM, 0x00}},
                            {0, BEJ_SET, 2, "VirtualizationOffload", ""},
                                {0, BEJ_SET, 1, "SRIOV", ""},
                                    {0, BEJ_BOOLEAN, 0, "SRIOVVEPACapable", "t"},
                                {0, BEJ_SET, 3, "VirtualFunction", ""},
                                    {0, BEJ_INT, 0, "DeviceMaxCount", (char [3]){0x00, 0x1, 0x00}},
                                    {0, BEJ_INT, 0, "MinAssignmentGroupSize", (char [2]){0x01, 0x00}},
                                    {0, BEJ_INT, 0, "NetworkPortMaxCount", (char [3]){0x00, 0x1, 0x00}},
                        {0, BEJ_STR, 0, "FirmwarePackageVersion", "1.1.1?"},
                        {0, BEJ_SET, 1, "Links", ""},
                            {0, BEJ_ARRAY, 1, "NetworkDeviceFunctions", ""},
                                {0, BEJ_SET, 0, "", (char [2]){PLDM_BASE_NETWORK_DEV_FUNCS_RESOURCE_ID, 0x00}},
                        {0, BEJ_SET, 4, "PCIeInterface", ""},
                            {0, BEJ_INT, 0, "LanesInUse", "?"},
                            {0, BEJ_INT, 0, "MaxLanes", ""},
                            {0, BEJ_ENUM, 0, "MaxPCIeType", (char [3]){0x01, 0x01, 0x00}},  /* Maximum Link Speed? */
                            {0, BEJ_ENUM, 0, "PCIeType", (char [3]){0x01, 0x01, 0x00}},
                                // {0, BEJ_STR, 0, "Gen1", ""},
                                // {0, BEJ_STR, 0, "Gen2", ""},
                                // {0, BEJ_STR, 0, "Gen3", ""},
                                // {0, BEJ_STR, 0, "Gen4", ""},
                {0, BEJ_STR, 0, "Manufacturer", "WXKJ"},
                {0, BEJ_STR, 0, "Model", "AMBER"},
                {0, BEJ_STR, 0, "Name", "AM Network Adapter"},
                {0, BEJ_SET, 0, "NetworkDeviceFunctions", "NetworkDeviceFunctionCollection"},
                {0, BEJ_SET, 0, "NetworkPorts", "NetworkPortCollection"},
                {0, BEJ_STR, 0, "PartNumber", "Part Number (PN) is 11 byte value maintained in VPD.?"},
                {0, BEJ_STR, 0, "SKU", "AMBER"},
                {0, BEJ_STR, 0, "SerialNumber", "Read from GLPCI_SERH/L.?"},
                {0, BEJ_SET, 1, "Status", ""},
                    {0, BEJ_ENUM, 0, "State", (char [3]){0x01, 0x01, 0x00}},
                        // {0, BEJ_STR, 0, "StandbyOffline", ""},
                        // {0, BEJ_STR, 0, "Starting", ""},
                        // {0, BEJ_STR, 0, "Updating", ""},
                        // {0, BEJ_STR, 0, "Enabled", ""},
                        // {0, BEJ_STR, 0, "Disabled", ""},
                {0, BEJ_STR, 0, "Id", "%I1"},                   /* PLDM_BASE_NETWORK_ADAPTER_RESOURCE_ID */
                // {0, BEJ_ARRAY, 1, "ControllerLinks", ""},
                //     {1, BEJ_INT, 0, "@odata.count", (char [2]){MAX_LAN_NUM, 0x00}},
    };
    pldm_cjson_create_schema(root, fmt);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    pldm_cjson_fill_comm_field_in_schema(new_root->child, 0, PLDM_BASE_NETWORK_ADAPTER_RESOURCE_ID, "NetworkAdapter.1_5_0.NetworkAdapter", NETWORK_ADAPTER);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_networkdevicefunction_v1_3_3_schema(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 1, "", ""},
            {0, BEJ_SET, 14, "NetworkDeviceFunction", ""},
                {0, BEJ_ARRAY, 1, "AssignablePhysicalPorts", ""},
                    {0, BEJ_SET, 0, "", ""},
                        // {0, BEJ_STR, 0, "", (char [2]){PLDM_BASE_PORT_RESOURCE_ID, 0x00}},
                        // {0, BEJ_STR, 0, "", (char [2]){PLDM_BASE_PORT_RESOURCE_ID + 1, 0x00}},
                {1, BEJ_INT, 0, "@odata.count", (char [2]){MAX_LAN_NUM, 0x00}},
                {0, BEJ_ENUM, 0, "BootMode", (char [3]){0x01, 0x01, 0x00}},
                    // {0, BEJ_STR, 0, "Disabled", ""},
                    // {0, BEJ_STR, 0, "PXE", ""},
                    // {0, BEJ_STR, 0, "iSCSI", ""},
                    // {0, BEJ_STR, 0, "FibreChannel", ""},
                    // {0, BEJ_STR, 0, "FibreChannelOverEthernet", ""},
                {0, BEJ_BOOLEAN, 0, "DeviceEnabled", "t"},
                {0, BEJ_SET, 5, "Ethernet", ""},
                    {0, BEJ_STR, 0, "MACAddress", "11:22:33:44:55:66"},
                    {0, BEJ_INT, 0, "MTUSize", (char [4]){0x02, 0xEE, 0x25, 0x00}},
                    {0, BEJ_STR, 0, "PermanentMACAddress", "11:22:33:44:55:66"},
                    {0, BEJ_SET, 0, "VLAN", ""},
                    {0, BEJ_SET, 0, "VLANs", ""},
                {0, BEJ_INT, 0, "MaxVirtualFunctions", ""},
                {0, BEJ_STR, 0, "Name", "NetworkDeviceFunction Current Settings?"},
                {0, BEJ_ARRAY, 1, "NetDevFuncCapabilities", ""},
                    {0, BEJ_ENUM, 0, "", (char [3]){0x01, 0x01, 0x00}},
                        // {0, BEJ_STR, 0, "Disabled", ""},
                        // {0, BEJ_STR, 0, "Ethernet", ""},
                        // {0, BEJ_STR, 0, "FibreChannel", ""},
                        // {0, BEJ_STR, 0, "iSCSI", ""},
                        // {0, BEJ_STR, 0, "FibreChannelOverEthernet", ""},
                {0, BEJ_ENUM, 0, "NetDevFuncType", (char [3]){0x01, 0x01, 0x00}},
                    // {0, BEJ_STR, 0, "Disabled", ""},
                    // {0, BEJ_STR, 0, "Ethernet", ""},
                    // {0, BEJ_STR, 0, "FibreChannel", ""},
                    // {0, BEJ_STR, 0, "iSCSI", ""},
                    // {0, BEJ_STR, 0, "FibreChannelOverEthernet", ""},
                {0, BEJ_SET, 1, "Status", ""},
                    {0, BEJ_ENUM, 0, "State", (char [3]){0x01, 0x01, 0x00}},
                        // {0, BEJ_STR, 0, "StandbyOffline", ""},
                        // {0, BEJ_STR, 0, "Starting", ""},
                        // {0, BEJ_STR, 0, "Updating", ""},
                        // {0, BEJ_STR, 0, "Enabled", ""},
                        // {0, BEJ_STR, 0, "Disabled", ""},
                {0, BEJ_BOOLEAN, 0, "VirtualFunctionsEnabled", "f"},
                {1, BEJ_SET, 2, "@Redfish.Settings", ""},
                    {1, BEJ_SET, 0, "SettingsObject", "Points to the next setting = Resource ID +10"},
                    {1, BEJ_ARRAY, 1, "SupportedApplyTimes", ""},
                        {1, BEJ_ENUM, 0, "", (char [3]){0x01, 0x03, 0x00}},
                        // {0, BEJ_STR, 0, "AtMaintenanceWindowStart", ""},
                        // {0, BEJ_STR, 0, "Immediate", ""},
                        // {0, BEJ_STR, 0, "InMaintenanceWindowOnReset", ""},
                        // {0, BEJ_STR, 0, "OnReset", ""},
                {0, BEJ_STR, 0, "Id", "Resource Offset"},
                {0, BEJ_SET, 2, "Links", ""},
                    {0, BEJ_SET, 0, "PCIeFunction", ""},
                        // {0, BEJ_STR, 0, "", (char [3]){0x0c, 0x12, 0x00}},
                    {0, BEJ_SET, 0, "PhysicalPortAssignment", ""},
                        // {0, BEJ_STR, 0, "", (char [2]){PLDM_BASE_PORT_RESOURCE_ID, 0x00}},
    };
    pldm_cjson_create_schema(root, fmt);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    pldm_cjson_fill_comm_field_in_schema(new_root->child, 0, resource_id, "PortCollection.1_3_1.PortCollection", NETWORK_DEVICE_FUNC);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_networkdevicefunctioncollection_schema(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 1, "", ""},
            {0, BEJ_SET, 3, "NetworkDeviceFunctionCollection", ""},
                {0, BEJ_STR, 0, "Name", "NetworkDeviceFunctions"},
                {1, BEJ_INT, 0, "@odata.count", (char [2]){MAX_LAN_NUM, 0x00}},
                {0, BEJ_ARRAY, MAX_LAN_NUM, "Members", ""},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I200"},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I201"},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I202"},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I203"}
    };
    pldm_cjson_create_schema(root, fmt);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    pldm_cjson_fill_comm_field_in_schema(new_root->child, 1, PLDM_BASE_NETWORK_DEV_FUNCS_RESOURCE_ID, "NetworkDeviceFunctionCollection.NetworkDeviceFunctionCollection", NETWORK_DEVICE_FUNC_COLLECTION);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_pciedevice_v1_4_0_schema(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 1, "", ""},
            {0, BEJ_SET, 13, "PCIeDevice", ""},
                {0, BEJ_STR, 0, "AssetTag", ""},
                {0, BEJ_ENUM, 0, "DeviceType", (char [3]){0x01, 0x01, 0x00}},  /* Config space of function 0 - Header type register */
                    // {0, BEJ_STR, 0, "SingleFunction", ""},
                    // {0, BEJ_STR, 0, "MultiFunction", ""},
                    // {0, BEJ_STR, 0, "Simulated", ""},
                {0, BEJ_STR, 0, "FirmwareVersion", "1.1.1?"},
                {0, BEJ_STR, 0, "Manufacturer", "WXKJ"},
                {0, BEJ_STR, 0, "Model", "AMBER"},
                {0, BEJ_STR, 0, "Name", "AMBER"},
                {0, BEJ_SET, 4, "PCIeInterface", ""},
                    {0, BEJ_INT, 0, "LanesInUse", ""},                         /* Negotiated Link Width */
                    {0, BEJ_INT, 0, "MaxLanes", ""},                           /* Maximum Link Width */
                    {0, BEJ_ENUM, 0, "MaxPCIeType", (char [3]){0x01, 0x01, 0x00}}, /* Maximum Link Speed */
                        // {0, BEJ_STR, 0, "Gen1", ""},
                        // {0, BEJ_STR, 0, "Gen2", ""},
                        // {0, BEJ_STR, 0, "Gen3", ""},
                        // {0, BEJ_STR, 0, "Gen4", ""},
                        // {0, BEJ_STR, 0, "Gen5", ""},
                    {0, BEJ_ENUM, 0, "PCIeType", (char [3]){0x01, 0x01, 0x00}},    /* Current Link Speed */
                        // {0, BEJ_STR, 0, "Gen1", ""},
                        // {0, BEJ_STR, 0, "Gen2", ""},
                        // {0, BEJ_STR, 0, "Gen3", ""},
                        // {0, BEJ_STR, 0, "Gen4", ""},
                        // {0, BEJ_STR, 0, "Gen5", ""},
                {0, BEJ_STR, 0, "PartNumber", ""},
                {0, BEJ_STR, 0, "SKU", "AMBER"},
                {0, BEJ_STR, 0, "SerialNumber", "11:22:33:FF:FF:44:55:66"},
                {0, BEJ_SET, 1, "Status", ""},
                    {0, BEJ_ENUM, 0, "State", (char [3]){0x01, 0x01, 0x00}},
                        // {0, BEJ_STR, 0, "StandbyOffline", ""},
                        // {0, BEJ_STR, 0, "Starting", ""},
                        // {0, BEJ_STR, 0, "Updating", ""},
                        // {0, BEJ_STR, 0, "Enabled", ""},
                        // {0, BEJ_STR, 0, "Disabled", ""},
                {0, BEJ_SET, 0, "PCIeFunctions", ""},
                    // {0, BEJ_STR, 0, "", (char [2]){PLDM_BASE_PCIE_FUNCS_RESOURCE_ID, 0x00}},
                {0, BEJ_STR, 0, "Id", ""},
    };
    pldm_cjson_create_schema(root, fmt);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    pldm_cjson_fill_comm_field_in_schema(new_root->child, 0, PLDM_BASE_PCIE_FUNC_RESOURCE_ID, "PCIeDevice.1_4_0.PCIeDevice", PCIE_DEVICE);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_pciefunctioncollection_schema(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 1, "", ""},
            {0, BEJ_SET, 3, "PCIeDeviceCollection", ""},
                {0, BEJ_STR, 0, "Name", "PCIeFunctions"},
                {1, BEJ_INT, 0, "@odata.count", (char [2]){MAX_LAN_NUM, 0x00}},
                {0, BEJ_ARRAY, MAX_LAN_NUM, "Members", ""},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I300"},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I301"},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I302"},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I303"}
    };
    pldm_cjson_create_schema(root, fmt);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    pldm_cjson_fill_comm_field_in_schema(new_root->child, 1, PLDM_BASE_PCIE_FUNCS_RESOURCE_ID, "PCIeFunctionCollection.PCIeFunctionCollection", PCIE_FUNC_COLLECTION);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_pciefunction_v1_2_3_schema(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 1, "", ""},
            {0, BEJ_SET, 11, "PCIeFunction", ""},
                {0, BEJ_STR, 0, "ClassCode", "0x020000"},                  /* EthernetController */
                {0, BEJ_ENUM, 0, "DeviceClass", (char [3]){0x01, 'U', 0x00}},
                    // {0, BEJ_STR, 0, "NetworkController", ""},
                {0, BEJ_STR, 0, "DeviceId", ""},
                {0, BEJ_ENUM, 0, "FunctionType", (char [3]){0x01, 'U', 0x00}},
                    // {0, BEJ_STR, 0, "Physical", ""},
                    // {0, BEJ_STR, 0, "Virtual", ""},
                {0, BEJ_STR, 0, "Name", "AMBER"},
                {0, BEJ_STR, 0, "RevisionId", ""},                         /* GLPCI_DREVID XOR GLPCI_REVID */
                {0, BEJ_SET, 1, "Status", ""},
                    {0, BEJ_ENUM, 0, "State", (char [3]){0x01, 0x01, 0x00}},
                        // {0, BEJ_STR, 0, "StandbyOffline", ""},
                        // {0, BEJ_STR, 0, "Starting", ""},
                        // {0, BEJ_STR, 0, "Updating", ""},
                        // {0, BEJ_STR, 0, "Enabled", ""},
                        // {0, BEJ_STR, 0, "Disabled", ""},
                {0, BEJ_STR, 0, "SubsystemId", ""},                        /* PFPCI_SUBSYSID.PF_SUBSYS_ID */
                {0, BEJ_STR, 0, "SubsystemVendorId", ""},                  /* GLPCI_SUBVENID */
                {0, BEJ_STR, 0, "VendorId", ""},                           /* GLPCI_VENDORID.VENDOR_D */
                {0, BEJ_STR, 0, "Id", ""},                                 /* Resource Offset */
    };
    pldm_cjson_create_schema(root, fmt);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    pldm_cjson_fill_comm_field_in_schema(new_root->child, 0, resource_id, "PCIeFunction.1_2_3.PCIeFunction", PCI_FUNC);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_ethernetinterface_v1_5_1_schema(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 1, "", ""},
            {0, BEJ_SET, 12, "EthernetInterface", ""},
                {0, BEJ_BOOLEAN, 0, "FullDuplex", "t"},
                {0, BEJ_BOOLEAN, 0, "InterfaceEnabled", ""},               /* PRTGEN_STATUS.PORT_VALID */
                {0, BEJ_ENUM, 0, "LinkStatus", ""},
                    // {0, BEJ_STR, 0, "LinkDown", ""},
                    // {0, BEJ_STR, 0, "LinkUp", ""},
                    // {0, BEJ_STR, 0, "NoLink", ""},
                {0, BEJ_STR, 0, "MACAddress", "11:22:33:44:55:66"},
                {0, BEJ_INT, 0, "MTUSize", (char [3]){0xEE, 0x25, 0x00}},
                {0, BEJ_STR, 0, "Name", "AM Ethernet Interface Current Settings"},
                {0, BEJ_ARRAY, 0, "NameServers", ""},
                {0, BEJ_STR, 0, "PermanentMACAddress", ""},
                {0, BEJ_INT, 0, "SpeedMbps", ""},
                {0, BEJ_SET, 1, "Status", ""},
                    {0, BEJ_ENUM, 0, "State", (char [3]){0x01, 0x03, 0x00}},
                        // {0, BEJ_STR, 0, "StandbyOffline", ""},
                        // {0, BEJ_STR, 0, "Starting", ""},
                        // {0, BEJ_STR, 0, "Updating", ""},
                        // {0, BEJ_STR, 0, "Enabled", ""},
                        // {0, BEJ_STR, 0, "Disabled", ""},
                {1, BEJ_SET, 2, "@Redfish.Settings", ""},
                    {1, BEJ_SET, 0, "SettingsObject", "Points to the next setting = Resource ID +10"},
                    {1, BEJ_ARRAY, 1, "SupportedApplyTimes", ""},
                        {1, BEJ_ENUM, 0, "", (char [3]){0x01, 0x03, 0x00}},
                        // {0, BEJ_STR, 0, "AtMaintenanceWindowStart", ""},
                        // {0, BEJ_STR, 0, "Immediate", ""},
                        // {0, BEJ_STR, 0, "InMaintenanceWindowOnReset", ""},
                        // {0, BEJ_STR, 0, "OnReset", ""},
                {0, BEJ_STR, 0, "Id", "Resource Offset"},
    };
    pldm_cjson_create_schema(root, fmt);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    pldm_cjson_fill_comm_field_in_schema(new_root->child, 0, resource_id, "EthernetInterface.1_5_1.EthernetInterface", ETH_INTERFACE);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_ethernetinterfacecollection_schema(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 1, "", ""},
            {0, BEJ_SET, 3, "EthernetInterfaceCollection", ""},
                {0, BEJ_STR, 0, "Name", "NetworkDeviceFunctions"},
                {1, BEJ_INT, 0, "@odata.count", (char [2]){MAX_LAN_NUM, 0x00}},
                {0, BEJ_ARRAY, MAX_LAN_NUM, "Members", ""},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I400"},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I401"},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I402"},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I403"}
    };
    pldm_cjson_create_schema(root, fmt);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    pldm_cjson_fill_comm_field_in_schema(new_root->child, 1, PLDM_BASE_ETH_INTERFACE_COLLECTION_RESOURCE_ID, "EthernetInterfaceCollection.EthernetInterfaceCollection", ETH_INTERFACE_COLLECTION);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/*  BEJ_NETWORK_ADAPTER = 0,
    BEJ_PCIE_DEVICE,
    BEJ_NETWORK_INTERFACE,
    BEJ_PORT_COLLECTION,
    BEJ_PCIE_FUNC_COLLECTION,
    BEJ_NETWORK_DEVICE_FUNC_COLLECTION,
    BEJ_NETWORK_DEVICE_FUNC,
    BEJ_PORT_IDENTIFY,
    BEJ_PCI_FUNC,
    BEJ_ETH_INTERFACE,
    BEJ_ETH_INTERFACE_COLLECTION, */

/* event, Port, NetworkDeviceFunction, PCIFunction, EthernetInterface schema use resource id, other schema ignore. */
schema_create g_schemas[11] = {
    pldm_cjson_create_networkadapter_v1_5_0_schema,
    pldm_cjson_create_pciedevice_v1_4_0_schema,
    pldm_cjson_create_networkinterface_v1_2_0_schema,
    pldm_cjson_create_portcollection_schema,
    pldm_cjson_create_pciefunctioncollection_schema,
    pldm_cjson_create_networkdevicefunctioncollection_schema,
    pldm_cjson_create_networkdevicefunction_v1_3_3_schema,
    pldm_cjson_create_port_v1_3_1_schema,
    pldm_cjson_create_pciefunction_v1_2_3_schema,
    pldm_cjson_create_ethernetinterface_v1_5_1_schema,
    pldm_cjson_create_ethernetinterfacecollection_schema
};

#if 0

u8 g_is_replace = 0;
void pldm_cjson_replace_val(pldm_cjson_t *root, pldm_bej_sflv_dat_t *sflv, char *replace_val)
{
    g_is_replace = 0;
    if (!root || !sflv || !replace_val) return;
    pldm_cjson_t *tmp = root;
    while (tmp) {
        if (tmp->child) {
            pldm_cjson_replace_val(tmp->child, sflv, replace_val);
        } else {
            if (tmp->sflv.fmt == sflv->fmt && tmp->sflv.seq == sflv->seq) {
                pldm_cjson_free((u8 *)(tmp->sflv.val));
                u8 len = cm_strlen(replace_val);
                tmp->sflv.val = (char *)pldm_cjson_malloc(len + 1);
                cm_memcpy(tmp->sflv.val, replace_val, len);
                g_is_replace = 1;
            }
        }
        if (g_is_replace) {
            break;
        }
        tmp = tmp->next;
    }
}

/* actually not used */
static void pldm_cjson_replace_enum_val(pldm_cjson_t *root, pldm_bej_sflv_dat_t *sflv, char *enum_name, char *enum_val)
{
    if (!root || !sflv || !enum_name || !enum_val) return;
    g_is_find = NULL;
    pldm_cjson_find_name(root, enum_name);
    if (g_is_find) {
        u8 idx = 0;
        for (g_is_find = g_is_find->child; g_is_find; g_is_find = g_is_find->next) {
            if (cm_strcmp(g_is_find->name, enum_val) == 0) {
                char str[] = {0x01, 0x00, 0x00};
                if (!idx) {
                    str[1] = 'U';
                } else {
                    str[1] = idx;
                }
                pldm_cjson_replace_val(root, sflv, str);
                // pldm_cjson_cal_len_to_root(root, OTHER_TYPE);
                break;
            }
            idx++;
        }
    }
}

static void pldm_cjson_fill_dict_hdr(u8 *dictionary)
{
    if (!dictionary) return;
    pldm_redfish_dictionary_format_t *dict = (pldm_redfish_dictionary_format_t *)dictionary;
    dict->version_tag = 0;       /* 0x00 for DSP0218 v1.0.0, v1.1.0, v1.1.1 */
    /* bit0 : truncation_flag; if 1b, the dictionary is truncated and provides entries for a subset of the full Redfish schema */
    dict->dictionay_flags = 0;
    // dict->schema_version = 0;
}

static void pldm_cjson_fill_dict_str(pldm_cjson_t *root, u8 *dictionary)
{
	if (!root || !dictionary) return;

	pldm_cjson_t *queue[64];
    pldm_cjson_t *q = NULL;

    pldm_redfish_dictionary_format_t *dict = (pldm_redfish_dictionary_format_t *)dictionary;
    u8 *dict_name = (u8 *)&(dict->entry[dict->entry_cnt]);
	//front作为输出索引，rear作为存储索引
	int front = -1, rear = -1;
    int have_child = -1;
    u8 offset = 0;
    u16 entry_cnt = 0;
    u8 is_same = 0;

    for (; root; root = root->next) {
        queue[++rear] = root;
    }
    have_child = front;
	while (front != rear)
	{
		q = queue[++front];
        // LOG("seq : %d, fmt : %d, len : %d, name : %s", q->sflv.seq, q->sflv.fmt, q->sflv.len, q->name);
        if (!(q->sflv.seq & 1)) {
            u8 fmt = dict->entry[entry_cnt].format >> 4;
            if (fmt == q->sflv.fmt >> 4 && dict->entry[entry_cnt].sequence_num == q->sflv.seq >> 1) {
                is_same = 0;
                for (u8 i = 0; i < entry_cnt; i++) {
                    if (cm_strcmp((char *)&dictionary[dict->entry[i].name_off], q->name) == 0) {
                        dict->entry[entry_cnt].name_off = dict->entry[i].name_off;
                        is_same = 1;
                        break;
                    }
                }
                if (!is_same) {
                    if (!(cm_strlen(q->name))) {
                        dict->entry[entry_cnt].name_off = 0;
                        continue;
                    }
                    cm_memcpy(dict_name + offset, q->name, cm_strlen(q->name) + 1);   /* terminal is '\0' */
                    dict->entry[entry_cnt].name_off = (dict_name - dictionary) + offset;
                    offset += cm_strlen(q->name) + 1;
                }
                entry_cnt++;
                // LOG("\n%s %d", name, entry_cnt);
            }
        }

        if (front == rear) {
            u8 cnt = rear - have_child;
            // LOG("cnt : %d", cnt);
            for (u8 i = 0; i < cnt; i++) {
                pldm_cjson_t *tmp = queue[have_child + i + 1];
                if (tmp->child) {
                    for (pldm_cjson_t *tmp1 = tmp->child; tmp1; tmp1 = tmp1->next) {
                        queue[++rear] = tmp1;
                    }
                }
            }
            have_child = front;
        }
	}
}

static void pldm_cjson_fill_dict_entry_data(pldm_cjson_t *root, u8 *dictionary)
{
	if (!root || !dictionary) return;

	pldm_cjson_t *queue[64];
    pldm_cjson_t *q = NULL;
	//front作为输出索引，rear作为存储索引
	int front = -1, rear = -1;
    int have_child = -1;
    u16 entry_cnt = 0;
    u8 is_same = 0;

    pldm_redfish_dictionary_format_t *dict = (pldm_redfish_dictionary_format_t *)dictionary;

    for (; root; root = root->next) {
        queue[++rear] = root;
    }
    have_child = front;
	while (front != rear)
	{
		q = queue[++front];
		// LOG("seq : %d, fmt : %d, len : %d, name : %s", q->sflv.seq, q->sflv.fmt, q->sflv.len, q->name);
        if (!(q->sflv.seq & 1)) {
            is_same = 0;
            for (u8 i = 0; i < front; i++) {
                u8 fmt = queue[i]->sflv.fmt >> 4;
                if ((cm_strcmp(queue[i]->name, q->name) == 0) && (fmt == q->sflv.fmt >> 4) && (queue[i]->sflv.seq == q->sflv.seq)) {
                    is_same = 1;
                    break;
                }
            }
            if (!is_same) {
                dict->entry[entry_cnt].format = q->sflv.fmt;
                dict->entry[entry_cnt].sequence_num = (q->sflv.seq >> 1);
                dict->entry[entry_cnt].name_len = cm_strlen(q->name);
                entry_cnt++;
            }
        }

        if (front == rear) {
            u8 cnt = rear - have_child;
            // LOG("cnt : %d", cnt);
            for (u8 i = 0; i < cnt; i++) {
                pldm_cjson_t *tmp = queue[have_child + i + 1];
                if (tmp->child) {
                    for (pldm_cjson_t *tmp1 = tmp->child; tmp1; tmp1 = tmp1->next) {
                        queue[++rear] = tmp1;
                    }
                }
            }
            have_child = front;
        }
	}
    dict->entry_cnt = entry_cnt;
}

void pldm_cjson_fill_anno_dict_entry_data(pldm_cjson_t *root, u8 *dictionary, u8 *name_buf, u16 *name_off)
{
	if (!root || !dictionary || !name_buf || !name_off) return;

	pldm_cjson_t *queue[64];
    pldm_cjson_t *q = NULL;
	//front作为输出索引，rear作为存储索引
	int front = -1, rear = -1;
    int have_child = -1;
    u8 entry_cnt = 0;
    u8 is_same = 0;

    pldm_redfish_dictionary_format_t *dict = (pldm_redfish_dictionary_format_t *)dictionary;

    entry_cnt = dict->entry_cnt;

    for (; root; root = root->next) {
        queue[++rear] = root;
    }
    have_child = front;
	while (front != rear)
	{
		q = queue[++front];
		// LOG("seq : %d, fmt : %d, len : %d, name : %s\n", q->sflv.seq >> 1, q->sflv.fmt, q->sflv.len, q->name);
        if (q->sflv.seq & 1) {
            is_same = 0;
            u8 used_name_idx = 0;
            for (u8 i = 0; i < cm_strlen(q->name); i++) {
                if (q->name[i] == '@') {
                    used_name_idx = i;
                    break;
                }
            }
            for (u8 i = 0; i < entry_cnt; i++) {
                if ((cm_strcmp((char *)&name_buf[dict->entry[i].name_off], &(q->name[used_name_idx])) == 0)) {
                    is_same = 1;
                    break;
                }
            }
            if (!is_same) {
                char *name = &(q->name[used_name_idx]);
                dict->entry[entry_cnt].format = q->sflv.fmt;
                dict->entry[entry_cnt].sequence_num = (q->sflv.seq >> 1);
                dict->entry[entry_cnt].name_len = cm_strlen(name);
                cm_memcpy(name_buf + *name_off, name, dict->entry[entry_cnt].name_len + 1);
                dict->entry[entry_cnt].name_off = *name_off;
                *name_off += dict->entry[entry_cnt].name_len + 1;
                entry_cnt++;
            }
        }

        if (front == rear) {
            u8 cnt = rear - have_child;
            // LOG("cnt : %d\n", cnt);
            for (u8 i = 0; i < cnt; i++) {
                pldm_cjson_t *tmp = queue[have_child + i + 1];
                if (tmp->child) {
                    for (pldm_cjson_t *tmp1 = tmp->child; tmp1; tmp1 = tmp1->next) {
                        queue[++rear] = tmp1;
                    }
                }
            }
            have_child = front;
        }
	}
    dict->entry_cnt = entry_cnt;
    // LOG("entry cnt : %d\n", entry_cnt);
}

static void pldm_cjson_fill_dict_child(pldm_cjson_t *root, u8 *dictionary)
{
	if (!root || !dictionary) return;
    pldm_redfish_dictionary_format_t *dict = (pldm_redfish_dictionary_format_t *)dictionary;
    pldm_cjson_t *queue[64];
	//front作为输出索引，rear作为存储索引
	int front = -1, rear = -1;
    int have_child = -1;
    u8 child_cnt = 0;
    u16 i = 0;

    for (; root; root = root->next) {
        queue[++rear] = root;
    }
    have_child = front;
	while (front != rear)
	{
		++front;
		// LOG("seq : %d, fmt : %d, len : %d, name : %s", q->sflv.seq, q->sflv.fmt, q->sflv.len, q->name);
        if (front == rear) {
            u8 cnt = rear - have_child;
            // LOG("cnt : %d", cnt);
            for (u8 k = 0; k < cnt; k++) {
                pldm_cjson_t *tmp = queue[have_child + k + 1];
                if (tmp->child) {
                    for (; i < dict->entry_cnt; i++) {
                        if (cm_strcmp((char *)&dictionary[dict->entry[i].name_off], tmp->name) == 0) {
                            break;
                        }
                    }
                    for (u16 j = i; j < dict->entry_cnt; j++) {
                        if (cm_strcmp((char *)&dictionary[dict->entry[j].name_off], tmp->child->name) == 0) {
                            dict->entry[i].childpoint_off = (u8 *)&(dict->entry[j]) - dictionary;
                            break;
                        }
                    }
                    child_cnt = 0;
                    for (pldm_cjson_t *tmp1 = tmp->child; tmp1; tmp1 = tmp1->next) {
                        queue[++rear] = tmp1;
                        child_cnt++;
                    }
                    if (i < dict->entry_cnt) {
                        dict->entry[i].child_cnt = child_cnt;
                        i++;
                    }
                }
            }
            have_child = front;
        }
	}
    // for (u8 k = 0; k < dict->entry_cnt; k++) {
    //     pldm_redfish_dictionary_entry_t *entry = (pldm_redfish_dictionary_entry_t *)&dictionary[dict->entry[k].childpoint_off];
    //     LOG("child_cnt : %d, child_off : %s, name : %s", dict->entry[k].child_cnt, &dictionary[entry->name_off], &dictionary[dict->entry[k].name_off]);
    // }
}

static void pldm_cjson_fill_dict_copyright(u8 *dictionary, char *copyright_name)
{
    if (!dictionary || !copyright_name) return;
    pldm_redfish_dictionary_format_t *dict = (pldm_redfish_dictionary_format_t *)dictionary;
    pldm_redfish_dictionary_copyright_t *copyright;
    if (dict->entry_cnt) {
        u16 name_total_len = 0;
        for (u16 i = 0; i < dict->entry_cnt; i++) {
            name_total_len += dict->entry[i].name_len + 1;
        }
        copyright = (pldm_redfish_dictionary_copyright_t *)(((u8 *)&(dict->entry[dict->entry_cnt])) + name_total_len);
    } else {
        copyright = (pldm_redfish_dictionary_copyright_t *)&(dictionary[sizeof(pldm_redfish_dictionary_format_t)]);
    }
    copyright->copyright_len = cm_strlen(copyright_name) + 1;
    cm_memcpy(copyright->copyright, copyright_name, copyright->copyright_len);
    dict->dictionary_size = copyright->copyright + copyright->copyright_len - dictionary;
}

void pldm_cjson_printf_dict(u8 *dictionary)
{
    pldm_redfish_dictionary_format_t *dict = (pldm_redfish_dictionary_format_t *)dictionary;
    LOG("version_tag : %d", dict->version_tag);
    LOG("schema_version : %d", dict->schema_version);
    LOG("entry_cnt : %d", dict->entry_cnt);
    LOG("dictionay_flags : %d", dict->dictionay_flags);
    LOG("dictionary_size : %d", dict->dictionary_size);
    for (u8 i = 0; i < dict->entry_cnt; i++) {
        LOG("child_cnt : %d, seq : %d, fmt : %d name : ", dict->entry[i].child_cnt, dict->entry[i].sequence_num, dict->entry[i].format >> 4);
        for (u8 j = 0; j < dict->entry[i].name_len; j++) {
            u8 *name = &dictionary[dict->entry[i].name_off];
            LOG("%c", name[j]);
        }
        LOG(" child off : %d, offset : %d" , dict->entry[i].childpoint_off, (u8 *)&(dict->entry[i]) - dictionary);
    }
    pldm_redfish_dictionary_copyright_t *copyright;
    u16 name_total_len = 0;
    if (dict->entry_cnt) {
        for (u16 i = 0; i < dict->entry_cnt; i++) {
            name_total_len += dict->entry[i].name_len + 1;
        }
        copyright = (pldm_redfish_dictionary_copyright_t *)(((u8 *)&(dict->entry[dict->entry_cnt])) + name_total_len);
    } else {
        copyright = (pldm_redfish_dictionary_copyright_t *)&(dictionary[sizeof(pldm_redfish_dictionary_format_t)]);
    }
    LOG("name_total_len : %d, copyright len : %d, copyright : %s\n", name_total_len, copyright->copyright_len, copyright->copyright);
    // for (u16 i = 0; i < dict->dictionary_size; i++) {
    //     LOG("0x%02x, ", dictionary[i]);
    //     if (!((i + 1) % 8)) {
    //         LOG("");
    //     }
    // }
}

/* maybe not used */
static void pldm_cjson_create_dict(pldm_cjson_t *root, u8 *dictionary)
{
    if (!root || !dictionary) return;
    pldm_cjson_fill_dict_hdr(dictionary);
    pldm_cjson_fill_dict_entry_data(root, dictionary);
    pldm_cjson_fill_dict_str(root, dictionary);
    pldm_cjson_fill_dict_child(root, dictionary);
    pldm_cjson_fill_dict_copyright(dictionary, "Copyright (c) 2018 DMTF");
    // pldm_cjson_fill_dict_copyright(root, dictionary, "made in china");
    // pldm_cjson_printf_dict(dictionary);
}

u8 anno_dict[512];

void pldm_cjson_init(void)
{
    pldm_cjson_t *root = NULL;
    pldm_cjson_pool_init();

    u8 name_buf[512];
    u16 name_off = 0;
    /* create annotation dictionary */
    pldm_cjson_fill_dict_hdr(anno_dict);
    for (u8 i = 0; i < (sizeof(g_schemas) / sizeof(schema_create)); i++) {
        root = (g_schemas[i]());
        pldm_cjson_fill_anno_dict_entry_data(root, anno_dict, name_buf, &name_off);
        pldm_cjson_pool_reinit();
        root = NULL;
    }
    pldm_redfish_dictionary_format_t *dict = (pldm_redfish_dictionary_format_t *)anno_dict;
    u8 *dict_name = (u8 *)&(dict->entry[dict->entry_cnt]);
    u16 offset = dict_name - anno_dict;
    for (u8 i= 0; i < dict->entry_cnt; i++) {
        dict->entry[i].name_off += offset;
        dict->entry[i].sequence_num = i;
    }
    cm_memcpy(dict_name, name_buf, name_off);
    pldm_cjson_fill_dict_copyright(anno_dict, "Copyright (c) 2018 DMTF");
}

/* for test */
void pldm_cjson_test(void)
{
    u8 dict_test[1024];
    pldm_bej_sflv_dat_t sflv;
    pldm_cjson_t *root = NULL;
    pldm_cjson_t *obj = NULL;
    pldm_cjson_t *obj1 = NULL;
    pldm_cjson_t *obj2 = NULL;

    pldm_cjson_init();
    // root = create_port_v1_3_1_dict_test();

    root = pldm_cjson_create_obj();

    sflv.seq = 0;
    sflv.fmt = BEJ_SET << 4;
    pldm_cjson_add_sflv_attr(root, &sflv, "DummySimple", "", cm_strlen(""));

    sflv.seq = 0;
    sflv.fmt = BEJ_ARRAY << 4;
    obj2 = pldm_cjson_add_item_to_obj(root, &sflv, "ChildArrayProperty", "", cm_strlen(""));

    sflv.seq = 0;
    sflv.fmt = BEJ_SET << 4;
    obj1 = pldm_cjson_add_item_to_obj(obj2, &sflv, "", "", cm_strlen(""));

    sflv.seq = 0;
    sflv.fmt = BEJ_BOOLEAN << 4;
    pldm_cjson_add_item_to_obj(obj1, &sflv, "AnotherBoolean", "true", cm_strlen("true"));

    sflv.seq = 1;
    sflv.fmt = BEJ_ENUM << 4;
    obj = pldm_cjson_add_item_to_obj(obj1, &sflv, "LinkStatus", "LinkUp", cm_strlen("LinkUp"));

    sflv.seq = 0;
    sflv.fmt = BEJ_STR << 4;
    pldm_cjson_add_item_to_obj(obj, &sflv, "LinkDown", "", cm_strlen(""));

    sflv.seq = 1;
    sflv.fmt = BEJ_STR << 4;
    pldm_cjson_add_item_to_obj(obj, &sflv, "LinkUp", "", cm_strlen(""));

    sflv.seq = 2;
    sflv.fmt = BEJ_STR << 4;
    pldm_cjson_add_item_to_obj(obj, &sflv, "NoLink", "", cm_strlen(""));

    sflv.seq = 1;
    sflv.fmt = BEJ_STR << 4;
    pldm_cjson_add_item_to_obj(root, &sflv, "Id", "Dummy ID", cm_strlen("Dummy ID"));

    sflv.seq = 2;
    sflv.fmt = BEJ_BOOLEAN << 4;
    pldm_cjson_add_item_to_obj(root, &sflv, "SampleEnabledProperty", "false", cm_strlen("false"));

    sflv.seq = 3;
    sflv.fmt = BEJ_INT << 4;
    pldm_cjson_add_item_to_obj(root, &sflv, "SampleIntegerProperty", "12", cm_strlen("12"));

    sflv.seq = 1;
    sflv.fmt = BEJ_ENUM << 4;
    pldm_cjson_replace_enum_val(root, &sflv, "LinkStatus", "LinkUp");

    pldm_cjson_create_dict(root, dict_test);
    // pldm_cjson_cal_len_to_root(root, OTHER_TYPE);
    pldm_cjson_printf_root(root);
    LOG("\nused space : %d, max_space : %d", pldm_cjson_get_used_space(), PLDM_CJSON_POLL_SIZE);
}

#endif