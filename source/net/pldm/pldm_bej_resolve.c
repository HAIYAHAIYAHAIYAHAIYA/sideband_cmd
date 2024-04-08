#include "pldm_bej_resolve.h"
#include "pldm_redfish.h"

static pldm_bej_key_t gs_key, gs_enum_key;

static u16 pldm_bej_get_len(u8 *buf)
{
    if (!buf) return 0;
    u16 len = 0;
    if (buf[0] == 1) {
        len = buf[1];
    } else {
        len = buf[2];
        len <<= 8;
        len |= buf[1];
    }
    return len;
}

static u8 *pldm_bej_seq_to_nnint(u8 *buf, u8 seq)
{
    if (!buf) return buf;
    u8 *tmp = buf;
    tmp[0] = 0x01;
    tmp[1] = seq;
    tmp += 2;
    return tmp;
}

static u8 *pldm_bej_len_to_nnint(u8 *buf, u16 len)
{
    if (!buf) return buf;
    u8 *tmp = buf;
    if (len <= 0xFF) {
        tmp[0] = 0x01;
    } else {
        tmp[0] = 0x02;
    }
    cm_memcpy(&tmp[1], &len, tmp[0]);
    tmp += tmp[0] + 1;
    return tmp;
}

static u8 *pldm_bej_sfl_to_bej(u8 *buf, pldm_cjson_t *node)
{
    if (!buf || !node) return buf;
    u8 *tmp = buf;
    tmp = pldm_bej_seq_to_nnint(tmp, node->sflv.seq);
    tmp[0] = node->sflv.fmt;
    tmp += 1;
    tmp = pldm_bej_len_to_nnint(tmp, node->sflv.len);
    u8 fmt = node->sflv.fmt >> 4;
    if (fmt == BEJ_ARRAY || fmt == BEJ_SET) {
        u8 cnt = 0;
        tmp[0] = 0x01;
        for (pldm_cjson_t *i = node->child; i; i = i->next) {
            cnt++;
        }
        tmp[1] = cnt;
        tmp += 2;
    }
    return tmp;
}

static u8 *pldm_bej_jsonval_to_bej(u8 *buf, pldm_cjson_t *node)
{
    if (!buf || !node) return buf;
    u8 *tmp = buf;
    u8 fmt = node->sflv.fmt >> 4;
        switch (fmt) {
            case BEJ_BOOLEAN:
                if (cm_strcmp(node->sflv.val, "t") == 0)
                    tmp[0] = 0xFF;
                else
                    tmp[0] = 0x00;
                break;
            case BEJ_ENUM:
                cm_memcpy(tmp, node->sflv.val, node->sflv.len);
                if (node->sflv.val[1] == 0xFF) tmp[1] = 0x00;
                break;
            default:
                cm_memcpy(tmp, node->sflv.val, node->sflv.len);
                break;
        }
        tmp += node->sflv.len;
    return tmp;
}

static void pldm_bej_get_sflv(u8 *buf, pldm_bej_sflv_t *sflv)
{
    if (!buf || !sflv) return;
    pldm_bej_nnint_t *nnint = (pldm_bej_nnint_t *)buf;
    u8 seq_len = nnint->len;        /* always 1 */
    sflv->seq = nnint->data[0];
    nnint = (pldm_bej_nnint_t *)&nnint->data[seq_len];

    sflv->fmt = nnint->len;
    nnint = (pldm_bej_nnint_t *)nnint->data;

    sflv->len = pldm_bej_get_len((u8 *)nnint);
    nnint = (pldm_bej_nnint_t *)&nnint->data[nnint->len];

    sflv->val = (u8 *)nnint;

    sflv->sflv_len = (u8 *)nnint - buf + sflv->len;
}

static pldm_redfish_dictionary_entry_t *pldm_bej_dict_search(pldm_bej_sflv_t *sflv, u8 *dict, pldm_redfish_dictionary_entry_t *entry, u16 entry_cnt)
{
    if (!dict || !sflv || !entry) return NULL;
    pldm_redfish_dictionary_entry_t *tmp = entry;
    u8 is_find = 0;
    gs_key.len = 0;
    gs_key.val = NULL;
    gs_enum_key.len = 0;
    gs_enum_key.val = NULL;
    // LOG("need 0x%x, 0x%x, 0x%02x", sflv->fmt >> 4, sflv->seq >> 1, sflv->len);

    for (u16 k = 0; k < entry_cnt; k++) {
        u8 dict_fmt = tmp->format >> 4;
        // LOG("0x%x, 0x%x", dict_fmt, tmp->sequence_num);
        if (dict_fmt == sflv->fmt >> 4 && tmp->sequence_num == sflv->seq >> 1) {
            gs_key.val = (char *)&dict[tmp->name_off];
            gs_key.len = tmp->name_len;
            // LOG("%s: ", gs_key.val);

            if (dict_fmt == BEJ_ENUM) {
                pldm_redfish_dictionary_entry_t *enum_ptr = (pldm_redfish_dictionary_entry_t *)&dict[tmp->childpoint_off];
                enum_ptr += sflv->val[1];
                gs_enum_key.len = enum_ptr->name_len;
                gs_enum_key.val = (char *)&dict[enum_ptr->name_off];
                // LOG("%s ", gs_enum_key.val);
            }
            is_find = 1;
            return tmp;
        }
        tmp += 1;
    }
    if (!is_find) {
        LOG("dict_search fmt err : fmt : seq : %#x, %#x", sflv->fmt, sflv->seq);
        gs_key.len = cm_strlen("fmt_err");
        gs_key.val = "fmt_err";
    }

    return entry;
}

void pldm_bej_init(void)
{
    pldm_cjson_pool_init();
}

static void pldm_bej_val_search(u8 *dictionary, pldm_bej_sflv_t *sflv, pldm_cjson_t *ptr)
{
    if (!dictionary || !sflv || !ptr) return;
    u8 fmt = sflv->fmt >> 4;
    pldm_bej_sflv_dat_t tmp;
    tmp.seq = sflv->seq;
    tmp.fmt = sflv->fmt;
    tmp.len = sflv->len;
    switch (fmt) {
        // case BEJ_INT:
        //     pldm_cjson_add_item_to_obj(ptr, &tmp, gs_key.val, (char *)sflv->val, sflv->len);
        //     break;
        // case BEJ_ENUM:
        //     pldm_cjson_add_enum_to_obj(ptr, dictionary, &tmp, gs_key.val, gs_enum_key.val);
        //     break;
        case BEJ_BOOLEAN:
            if (sflv->val[0] == 0)
                pldm_cjson_add_item_to_obj(ptr, &tmp, gs_key.val, "f", sflv->len);
            else 
                pldm_cjson_add_item_to_obj(ptr, &tmp, gs_key.val, "t", sflv->len);
            break;
        default :
            pldm_cjson_add_item_to_obj(ptr, &tmp, gs_key.val, (char *)sflv->val, sflv->len);
            break;
    }
}

u8 *pldm_bej_encode(pldm_cjson_t *root, u8 *bej_buf)
{
    if (!root || !bej_buf) return bej_buf;
    pldm_cjson_t *tmp = root;
    u8 *buf = bej_buf;
    while (tmp) {
        u8 fmt = tmp->sflv.fmt >> 4;
        buf = pldm_bej_sfl_to_bej(buf, tmp);
        buf = pldm_bej_encode(tmp->child, buf);
        if (!(tmp->child) && fmt != BEJ_ARRAY && fmt != BEJ_SET) {
            buf = pldm_bej_jsonval_to_bej(buf, tmp);
        }
        tmp = tmp->next;
    }
    return buf;
}

static u16 pldm_bej_decode_op(u8 *buf, u8 *anno_dict, u8 *dict, pldm_redfish_dictionary_entry_t *entry, u16 entry_cnt, pldm_redfish_dictionary_entry_t *anno_entry, u16 anno_entry_cnt, pldm_cjson_t *root)
{
    if (!buf || !entry || !root || !dict || !anno_dict) return 0;
    pldm_bej_sflv_t sflv = {0, 0, 0, 0, NULL};
    pldm_redfish_dictionary_entry_t *new_entry = entry;
    pldm_redfish_dictionary_entry_t *new_anno_entry = anno_entry;
    u8 cnt = 0;
    pldm_cjson_t *ptr = root;
    pldm_cjson_t *add_ptr = NULL;
    pldm_bej_get_sflv(buf, &sflv);
    if (sflv.seq & 1)
        new_anno_entry = pldm_bej_dict_search(&sflv, anno_dict, anno_entry, anno_entry_cnt);
    else
        new_entry = pldm_bej_dict_search(&sflv, dict, entry, entry_cnt);

    /* see PropertyUnknown in the Redfish base message registry.(DSP0265) */
    if (cm_strcmp(gs_key.val, "fmt_err") == 0) return 0;

    u16 child_cnt = entry_cnt;
    u16 anno_child_cnt = anno_entry_cnt;
    u8 fmt = sflv.fmt >> 4;
    if ((fmt == BEJ_SET || fmt == BEJ_ARRAY)) {
        cnt = sflv.val[1];
        sflv.val = &(sflv.val[2]);

        child_cnt = new_entry->child_cnt;
        new_entry = (pldm_redfish_dictionary_entry_t *)&dict[new_entry->childpoint_off];

        if (new_anno_entry->childpoint_off && sflv.seq & 1) {
            anno_child_cnt = new_anno_entry->child_cnt;
            new_anno_entry = (pldm_redfish_dictionary_entry_t *)&anno_dict[new_anno_entry->childpoint_off];
        }

        pldm_bej_sflv_dat_t tmp;
        tmp.seq = sflv.seq;
        tmp.fmt = sflv.fmt;
        tmp.len = sflv.len;
        add_ptr = pldm_cjson_add_item_to_obj(ptr, &tmp, gs_key.val, "", 0);
    }
    for (u8 i = 0; i < cnt; i++) {
        u16 len = pldm_bej_decode_op(sflv.val, anno_dict, dict, new_entry, child_cnt, new_anno_entry, anno_child_cnt, add_ptr);
        sflv.val += len;
    }
    if (!cnt && fmt != BEJ_SET && fmt != BEJ_ARRAY) {
        pldm_bej_val_search(dict, &sflv, ptr);
    }
    return sflv.sflv_len;
}

pldm_cjson_t *pldm_bej_decode(u8 *buf, u16 buf_len, u8 *anno_dict, u8 *dict, pldm_cjson_t *root)
{
    if (!buf || !dict || !anno_dict) return NULL;
    root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_redfish_dictionary_format_t *dictionary = (pldm_redfish_dictionary_format_t *)dict;
    pldm_redfish_dictionary_format_t *anno_dictionary = (pldm_redfish_dictionary_format_t *)anno_dict;
    u16 total_len = 0;
    while (total_len < buf_len) {
        total_len += pldm_bej_decode_op(&buf[total_len], anno_dict, dict, &(dictionary->entry[0]), dictionary->entry_cnt, &(anno_dictionary->entry[0]), anno_dictionary->entry_cnt, root);
    }
    LOG("total_len : %d", total_len);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    // pldm_cjson_cal_len_to_root(new_root, OTHER_TYPE);
    return new_root;
}