#include "pldm_rstr.h"
#include "pldm_bej_resolve.h"

static void pldm_rstr_fill_identify(char *name, u8 data_type, pldm_rstr_field_t *body)
{
    if (!body || !name) return;
    body->val.data_type = data_type;
    u8 name_len = cm_strlen(name);
    body->name = pldm_cjson_malloc(name_len + 1);
    if (body->name)
        cm_memcpy(body->name, name, name_len);
}

static void pldm_rstr_fill_val(void *val, u8 len, pldm_rstr_field_t *body)
{
    if (!body || !val) return;
        if (!val) return;
    switch (body->val.data_type) {
        case BEJ_INT:
        case BEJ_REAL:
        case BEJ_ENUM:
        case BEJ_BOOLEAN:
            body->val.i_val = *((u32 *)val);
            break;
        case BEJ_STR:
            body->val.string = pldm_cjson_malloc(len + 1);
            if (body->val.string)
                cm_memcpy(body->val.string, val, len);
            break;
    }
}

static void pldm_rstr_fill_str(pldm_rstr_field_t *fields, pldm_cjson_t *node, u8 field_cnt, u8 *idx)
{
    if (!fields || !node) return;
    u8 buf[32];
    u8 fmt = node->sflv.fmt >> 4;
    if ((cm_strcmp(fields[*idx].name, node->name) == 0) && (fmt == fields[*idx].val.data_type)) {
        u8 len = 0;
        switch (fmt) {
            case BEJ_INT:
                len = pldm_bej_u32_to_bejinteger(fields[*idx].val.i_val, buf, 0);
                break;
            case BEJ_BOOLEAN:
                if (fields[*idx].val.i_val) buf[0] = 't';
                else buf[0] = 'f';
                len = 1;
                break;
            case BEJ_STR:
                if (fields[*idx].val.string) {
                    len = cm_strlen(fields[*idx].val.string);
                    cm_memcpy(buf, fields[*idx].val.string, len);
                    len += 1;
                }
                break;
            case BEJ_REAL:
                len = pldm_bej_float_to_bejreal(fields[*idx].val.i_val, buf);
                break;
            case BEJ_ENUM:
                buf[0] = 0x1;
                len = pldm_bej_u32_to_bejinteger(fields[*idx].val.i_val, &buf[1], 0);
                len += 1;
                break;
        }
        node->sflv.len = len;
        node->sflv.val = pldm_cjson_malloc(len + 1);
        if (node->sflv.val)
            cm_memcpy(node->sflv.val, buf, len);
        *idx += 1;
    }
}

static u8 pldm_rstr_create_networkadapter_field(pldm_rstr_field_t *fields)
{
    if (!fields) return 0;
    char *names[] = {
        "Capable",
        "SRIOVVEPACapable",
        "DeviceMaxCount",
        "MinAssignmentGroupSize",
        "NetworkPortMaxCount",
        "FirmwarePackageVersion",
        "LanesInUse",
        "MaxLanes",
        "MaxPCIeType",
        "PCIeType",
        "Manufacturer",
        "Model",
        "Name",
        "PartNumber",
        "SKU",
        "SerialNumber",
        "State"
    };

    u8 type[] = {
        BEJ_BOOLEAN,
        BEJ_BOOLEAN,
        BEJ_INT,
        BEJ_INT,
        BEJ_INT,
        BEJ_STR,
        BEJ_INT,
        BEJ_INT,
        BEJ_ENUM,
        BEJ_ENUM,
        BEJ_STR,
        BEJ_STR,
        BEJ_STR,
        BEJ_STR,
        BEJ_STR,
        BEJ_STR,
        BEJ_ENUM,
    };
    u32 val = 's';
    u8 cnt = sizeof(type);
    for (u8 i = 0; i < cnt; ++i) {
        pldm_rstr_fill_identify(names[i], type[i], &fields[i]);
        pldm_rstr_fill_val(&val, sizeof(val), &fields[i]);
    }
    return cnt;
}

static u8 pldm_rstr_create_pciedevice_field(pldm_rstr_field_t *fields)
{
    if (!fields) return 0;
    char *names[] = {
        "AssetTag",
        "DeviceType",
        "FirmwareVersion",
        "Manufacturer",
        "Model",
        "Name",
        "LanesInUse",
        "MaxLanes",
        "MaxPCIeType",
        "PCIeType",
        "PartNumber",
        "SKU",
        "SerialNumber",
        "State",
    };

    u8 type[] = {
        BEJ_STR,
        BEJ_ENUM,
        BEJ_STR,
        BEJ_STR,
        BEJ_STR,
        BEJ_STR,
        BEJ_INT,
        BEJ_INT,
        BEJ_ENUM,
        BEJ_ENUM,
        BEJ_STR,
        BEJ_STR,
        BEJ_STR,
        BEJ_ENUM,
    };
    u32 val = 's';
    u8 cnt = sizeof(type);
    for (u8 i = 0; i < cnt; ++i) {
        pldm_rstr_fill_identify(names[i], type[i], &fields[i]);
        pldm_rstr_fill_val(&val, sizeof(val), &fields[i]);
    }
    return cnt;
}

static u8 pldm_rstr_create_networkinterface_field(pldm_rstr_field_t *fields)
{
    if (!fields) return 0;
    char *names[] = {
        "Name",
        "State",
    };

    u8 type[] = {
        BEJ_STR,
        BEJ_ENUM,
    };
    u32 val = 's';
    u8 cnt = sizeof(type);
    for (u8 i = 0; i < cnt; ++i) {
        pldm_rstr_fill_identify(names[i], type[i], &fields[i]);
        pldm_rstr_fill_val(&val, sizeof(val), &fields[i]);
    }
    return cnt;
}

static u8 pldm_rstr_create_networkdevicefunction_field(pldm_rstr_field_t *fields)
{
    if (!fields) return 0;
    char *names[] = {
        "BootMode",
        "DeviceEnabled",
        "MACAddress",
        "MTUSize",
        "PermanentMACAddress",
        "MaxVirtualFunctions",
        "Name",
        "",                                 /* NetDevFuncCapabilities */
        "NetDevFuncType",
        "State",
        "VirtualFunctionsEnabled",
        "",                                 /* SupportedApplyTimes */
    };

    u8 type[] = {
        BEJ_ENUM,
        BEJ_BOOLEAN,
        BEJ_STR,
        BEJ_INT,
        BEJ_STR,
        BEJ_INT,
        BEJ_STR,
        BEJ_ENUM,
        BEJ_ENUM,
        BEJ_ENUM,
        BEJ_BOOLEAN,
        BEJ_ENUM,
    };
    u32 val = 's';
    u8 cnt = sizeof(type);
    for (u8 i = 0; i < cnt; ++i) {
        pldm_rstr_fill_identify(names[i], type[i], &fields[i]);
        pldm_rstr_fill_val(&val, sizeof(val), &fields[i]);
    }
    return cnt;
}

static u8 pldm_rstr_create_port_field(pldm_rstr_field_t *fields)
{
    if (!fields) return 0;
    char *names[] = {
        "CurrentSpeedGbps",
        "FlowControlConfiguration",
        "FlowControlStatus",
        "",                                             /* SupportedEthernetCapabilities */
        "InterfaceEnabled",
        "AutoSpeedNegotiationCapable",
        "AutoSpeedNegotiationEnabled",
        "",                                             /* CapableLinkSpeedGbps */
        "",
        "",
        "LinkNetworkTechnology",
        "LinkState",
        "LinkStatus",
        "LinkTransitionIndicator",
        "MaxFrameSize",
        "MaxSpeedGbps",
        "Name",
        "State",
    };

    u8 type[] = {
        BEJ_REAL,
        BEJ_ENUM,
        BEJ_ENUM,
        BEJ_ENUM,
        BEJ_BOOLEAN,
        BEJ_BOOLEAN,
        BEJ_BOOLEAN,
        BEJ_REAL,
        BEJ_REAL,
        BEJ_REAL,
        BEJ_ENUM,
        BEJ_ENUM,
        BEJ_ENUM,
        BEJ_INT,
        BEJ_INT,
        BEJ_REAL,
        BEJ_STR,
        BEJ_ENUM,
    };
    u32 val = 's';
    u8 cnt = sizeof(type);
    for (u8 i = 0; i < cnt; ++i) {
        pldm_rstr_fill_identify(names[i], type[i], &fields[i]);
        pldm_rstr_fill_val(&val, sizeof(val), &fields[i]);
    }
    return cnt;
}

static u8 pldm_rstr_create_pciefunction_field(pldm_rstr_field_t *fields)
{
    if (!fields) return 0;
    char *names[] = {
        "ClassCode",
        "DeviceClass",
        "DeviceId",
        "FunctionType",
        "Name",
        "RevisionId",
        "State",
        "SubsystemId",
        "SubsystemVendorId",
        "VendorId",
    };

    u8 type[] = {
        BEJ_STR,
        BEJ_ENUM,
        BEJ_STR,
        BEJ_ENUM,
        BEJ_STR,
        BEJ_STR,
        BEJ_ENUM,
        BEJ_STR,
        BEJ_STR,
        BEJ_STR,
    };
    u32 val = 's';
    u8 cnt = sizeof(type);
    for (u8 i = 0; i < cnt; ++i) {
        pldm_rstr_fill_identify(names[i], type[i], &fields[i]);
        pldm_rstr_fill_val(&val, sizeof(val), &fields[i]);
    }
    return cnt;
}

static u8 pldm_rstr_create_ethernetinterface_field(pldm_rstr_field_t *fields)
{
    if (!fields) return 0;
    char *names[] = {
        "FullDuplex",
        "InterfaceEnabled",
        "LinkStatus",
        "MACAddress",
        "MTUSize",
        "Name",
        "PermanentMACAddress",
        "SpeedMbps",
        "State",
        "",                         /* SupportedApplyTimes */
    };

    u8 type[] = {
        BEJ_BOOLEAN,
        BEJ_BOOLEAN,
        BEJ_ENUM,
        BEJ_STR,
        BEJ_INT,
        BEJ_STR,
        BEJ_STR,
        BEJ_INT,
        BEJ_ENUM,
        BEJ_ENUM,
    };
    u32 val = 's';
    u8 cnt = sizeof(type);
    for (u8 i = 0; i < cnt; ++i) {
        pldm_rstr_fill_identify(names[i], type[i], &fields[i]);
        pldm_rstr_fill_val(&val, sizeof(val), &fields[i]);
    }
    return cnt;
}

static void pldm_rstr_process_redfish_resource(pldm_rstr_field_t *fields, pldm_cjson_t *root, u8 field_cnt, u8 *idx)
{
    if (!fields || !root) return;
    pldm_cjson_t *tmp = root;

    while (tmp) {
        pldm_rstr_process_redfish_resource(fields, tmp->child, field_cnt, idx);
        if (*idx == field_cnt) return;
        pldm_rstr_fill_str(fields, tmp, field_cnt, idx);
        tmp = tmp->next;
    }
}

void pldm_rstr_update_redfish_resource(pldm_cjson_t *root, u8 resource_identify)
{
    if (!root) return;
    pldm_rstr_field_t fields[20];
    u8 field_cnt = 0;
    u8 identify = resource_identify + NETWORK_ADAPTER;

    switch (identify) {
        case NETWORK_ADAPTER:
            field_cnt = pldm_rstr_create_networkadapter_field(fields);
            break;
        case PCIE_DEVICE:
            field_cnt = pldm_rstr_create_pciedevice_field(fields);
            break;
        case NETWORK_INTERFACE:
            field_cnt = pldm_rstr_create_networkinterface_field(fields);
            break;
        case NETWORK_DEVICE_FUNC:
            field_cnt = pldm_rstr_create_networkdevicefunction_field(fields);
            break;
        case PORT_IDENTIFY:
            field_cnt = pldm_rstr_create_port_field(fields);
            break;
        case PCI_FUNC:
            field_cnt = pldm_rstr_create_pciefunction_field(fields);
            break;
        case ETH_INTERFACE:
            field_cnt = pldm_rstr_create_ethernetinterface_field(fields);
            break;
    }
    u8 idx = 0;
    if (field_cnt)
        pldm_rstr_process_redfish_resource(fields, root, field_cnt, &idx);
}