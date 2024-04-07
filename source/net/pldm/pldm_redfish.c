#include "pldm_redfish.h"
#include "pldm_monitor.h"
#include "pldm_monitor_event_rbuf.h"
#include "pldm_bej_resolve.h"
#include "pldm_cjson.h"
#include "pldm.h"
#include "mctp.h"

pldm_redfish_base_info_t g_pldm_redfish_base_info;
static op_info_t gs_op_info = {.dev_status = OPERATION_INACTIVE};
static op_data_buf_t gs_op_buf;
static u8 etag[ALL_SCHEMA][256];
pldm_redfish_bej_t g_resourse_bej[11];
u8 g_dict_info[PLDM_REDFISH_DICT_INFO_LEN];
u8 g_anno_dict[PLDM_REDFISH_ANNO_DICT_LEN];
u8 g_needed_dict[PLDM_REDFISH_PORT_DICT_LEN];

u16 pldm_redfish_get_dict_len(u32 resource_id);

extern pldm_monitor_base_info_t g_pldm_monitor_info;
extern schema_create g_schemas[];

extern void pldm_unsupport_cmd(protocol_msg_t *pkt, int *pkt_len);
extern u32 crc32_pldm(u32 init_crc, u8 *data, u32 len);

pldm_redfish_schema_info_t schema_info[ALL_SCHEMA_IDENTIFY] = {
    [EVENT]                             = {SCHEMACLASS_EVENT,                               BIT(READ),                                          {"Event.json"}},
    [REDFISH_PAYLOAD_ANNOTATIONS]       = {SCHEMACLASS_ANNOTATION,                          BIT(READ),                                          {"redfish-payload-annotations.v1_0_2.json"}},
    [REDFISH_ERROR]                     = {SCHEMACLASS_ERROR,                               BIT(READ),                                          {"v1/redfish-error.v1_0_0.json"}},
    [REGISTRY]                          = {SCHEMACLASS_REGISTRY,                            BIT(READ),                                          {"Registry.v1_5_0.json"}},
    [NETWORK_ADAPTER]                   = {SCHEMA_CLASS(NETWORK_ADAPTER),                   SCHEMA_ALLOWED_OP(NETWORK_ADAPTER),                 {"NetworkAdapter.v1_5_0.json"}},
    [NETWORK_INTERFACE]                 = {SCHEMA_CLASS(NETWORK_INTERFACE),                 SCHEMA_ALLOWED_OP(NETWORK_INTERFACE),               {"NetWorkInterface.v1_2_0.json"}},
    [PCIE_DEVICE]                       = {SCHEMA_CLASS(PCIE_DEVICE),                       SCHEMA_ALLOWED_OP(PCIE_DEVICE),                     {"PCIeDevice.v1_4_0.json"}},
    [PORT_COLLECTION]                   = {SCHEMA_CLASS(PORT_COLLECTION),                   SCHEMA_ALLOWED_OP(PORT_COLLECTION),                 {"PortCollection.json", "Port.json"}},
    [NETWORK_DEVICE_FUNC_COLLECTION]    = {SCHEMA_CLASS(NETWORK_DEVICE_FUNC_COLLECTION),    SCHEMA_ALLOWED_OP(NETWORK_DEVICE_FUNC_COLLECTION),  {"NetWorkDeviceFunctionCollection.json", "NetWorkDeviceFunction.json"}},
    [PCIE_FUNC_COLLECTION]              = {SCHEMA_CLASS(PCIE_FUNC_COLLECTION),              SCHEMA_ALLOWED_OP(PCIE_FUNC_COLLECTION),            {"PCIeFunctionCollection.json", "PCIeFunction.json"}},
    [PORT_IDENTIFY]                     = {SCHEMA_CLASS(PORT_IDENTIFY),                     SCHEMA_ALLOWED_OP(PORT_IDENTIFY),                   {"NetworkPort.v1_3_1.json"}},
    [NETWORK_DEVICE_FUNC]               = {SCHEMA_CLASS(NETWORK_DEVICE_FUNC),               SCHEMA_ALLOWED_OP(NETWORK_DEVICE_FUNC),             {"NetWorkDeviceFunction.v1_3_3.json"}},
    [PCI_FUNC]                          = {SCHEMA_CLASS(PCI_FUNC),                          SCHEMA_ALLOWED_OP(PCI_FUNC),                        {"PCIeFunction.v1_2_3.json"}},
    [ETH_INTERFACE]                     = {SCHEMA_CLASS(ETH_INTERFACE),                     SCHEMA_ALLOWED_OP(ETH_INTERFACE),                   {"EthernetInterface.v1_5_1.json"}},
    [ETH_INTERFACE_COLLECTION]          = {SCHEMA_CLASS(ETH_INTERFACE_COLLECTION),          SCHEMA_ALLOWED_OP(ETH_INTERFACE_COLLECTION),        {"EthernetInterfaceCollection.json", "EthernetInterface.json"}},
    // [ALL_SCHEMA]                        = {SCHEMA_CLASS(ALL_SCHEMA),                        SCHEMA_ALLOWED_OP(PCIE_FUNC_COLLECTION),            {"Event.json", "redfish-payload-annotations.v1_0_2.json", "v1/redfish-error.v1_0_0.json"}}
};

static void pldm_redfish_clear_op_param(void)
{
    cm_memset(&gs_op_info, 0, sizeof(gs_op_info));
    gs_op_info.collection_top = 0xFFFF;           /* 0xFFFF means return all. */
    cm_memset(&gs_op_buf.multipartrecv_first_transfer_handle, 0, 16);
    gs_op_buf.buf_ptr.len = 0;
    gs_op_buf.buf_ptr.data = NULL;
    gs_op_buf.op_data.len = 0;
    gs_op_buf.op_result.len = 0;
    gs_op_buf.op_locator[0] = 0;   // the lengthbyte of length field
}

static u8 resource_id_to_resource_identity(u32 resource_id)
{
    u8 resource_identity = 0xFF;
    if (resource_id == PLDM_BASE_REGISTER_DICT_RESOURCE_ID) {
        resource_identity = ALL_SCHEMA;
    } else if (resource_id < 100) {
        switch (resource_id) {
            case PLDM_BASE_NETWORK_ADAPTER_RESOURCE_ID:
                resource_identity = NETWORK_ADAPTER;
                break;
            case PLDM_BASE_PCIE_DEV_RESOURCE_ID:
                resource_identity = PCIE_DEVICE;
                break;
            case PLDM_BASE_NETWORK_INTERFACE_RESOURCE_ID:
                resource_identity = NETWORK_INTERFACE;
                break;
            case PLDM_BASE_PORTS_RESOURCE_ID:
                resource_identity = PORT_COLLECTION;
                break;
            case PLDM_BASE_NETWORK_DEV_FUNCS_RESOURCE_ID:
                resource_identity = NETWORK_DEVICE_FUNC_COLLECTION;
                break;
            case PLDM_BASE_PCIE_FUNCS_RESOURCE_ID:
                resource_identity = PCIE_FUNC_COLLECTION;
                break;
            case PLDM_BASE_ETH_INTERFACE_COLLECTION_RESOURCE_ID:
                resource_identity = ETH_INTERFACE_COLLECTION;
                break;
            default:
                break;
        }
    } else if (((resource_id <= PLDM_MAX_PORT_RESOURCE_ID) && (resource_id >= PLDM_BASE_PORT_RESOURCE_ID)) || \
                ((resource_id <= PLDM_MAX_PORT_RESOURCE_ID_1) && (resource_id >= PLDM_BASE_PORT_RESOURCE_ID_1))) {
        resource_identity = PORT_IDENTIFY;
    } else if (((resource_id <= PLDM_MAX_NETWORK_DEV_FUNC_RESOURCE_ID) && (resource_id >= PLDM_BASE_NETWORK_DEV_FUNC_RESOURCE_ID)) || \
                ((resource_id <= PLDM_MAX_NETWORK_DEV_FUNC_RESOURCE_ID_1) && (resource_id >= PLDM_BASE_NETWORK_DEV_FUNC_RESOURCE_ID_1))) {
        resource_identity = NETWORK_DEVICE_FUNC;
    } else if ((resource_id <= PLDM_MAX_PCIE_FUNC_RESOURCE_ID) && (resource_id >= PLDM_BASE_PCIE_FUNC_RESOURCE_ID)) {
        resource_identity = PCI_FUNC;
    } else if (((resource_id <= PLDM_MAX_ETH_INTERFACE_RESOURCE_ID) && (resource_id >= PLDM_BASE_ETH_INTERFACE_RESOURCE_ID)) || \
                ((resource_id <= PLDM_MAX_ETH_INTERFACE_RESOURCE_ID_1) && (resource_id >= PLDM_BASE_ETH_INTERFACE_RESOURCE_ID_1))) {
        resource_identity = ETH_INTERFACE;
    }

    return resource_identity;
}

static void pldm_redfish_negotiate_redfish_parameters(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_redfish_negotiate_redfish_parameters_req_dat_t *req_dat = (pldm_redfish_negotiate_redfish_parameters_req_dat_t *)(pkt->req_buf);
    pldm_redfish_negotiate_redfish_parameters_rsp_dat_t *rsp_dat = (pldm_redfish_negotiate_redfish_parameters_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));
    char *device_provider_name = "NAME";                            /* to be determind */

    if (req_dat->mc_concurrency_support == 0) {
        rsp_hdr->cpl_code = PLDM_ERROR_INVALID_DATA;
        return;
    }

    if ((req_dat->mc_feature_support & CBIT(8)) == 0){              /* Firmware checks that Bit 8 is set before using bejRegistryItem. */
        rsp_hdr->cpl_code = PLDM_ERROR_UNSUPPORTED;
        return;
    }

    rsp_dat->dev_concurrency_support = 1;                           /* a value of 1 indicates no support for concurrency. */
    rsp_dat->dev_capabilities_flg = CBIT(2);                        /* bit2 : BEJ v1.1 encoding and decoding supported — 1b = yes */
    rsp_dat->dev_feature_support = BIT(HEAD) | BIT(READ) | BIT(UPDATE) | BIT(REPLACE) | BIT(ACTION) | BIT(EVENTS);
    rsp_dat->dev_cfg_signature = 0xFFFFFFFF;                        /* Use OCS HCU engine with ALGORITHM_MODE = SHA256. */
    rsp_dat->dev_provider_name.format = ASCII;
    rsp_dat->dev_provider_name.len = cm_strlen(device_provider_name);
    /* If a VPD Identifier String (tag 0x82) exists, use it as DeviceProviderName. Otherwise, use “Name” property at NetworkInterface. */
    mctp_memcpy_fast(rsp_dat->dev_provider_name.val, device_provider_name, rsp_dat->dev_provider_name.len);
    *pkt_len += (sizeof(pldm_redfish_negotiate_redfish_parameters_rsp_dat_t) + rsp_dat->dev_provider_name.len);
}

static void pldm_redfish_negotiate_medium_parameters(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_redfish_negotiate_medium_parameters_req_dat_t *req_dat = (pldm_redfish_negotiate_medium_parameters_req_dat_t *)(pkt->req_buf);
    pldm_redfish_negotiate_medium_parameters_rsp_dat_t *rsp_dat = (pldm_redfish_negotiate_medium_parameters_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    if (req_dat->mc_maximum_xfer_chunksize_bytes < MINIMAL_MCTP_PACKET) {
        rsp_hdr->cpl_code = PLDM_ERROR_INVALID_DATA;
        return;
    }
    rsp_dat->mc_maximum_xfer_chunksize_bytes = PLDM_REDFISH_DEV_MAXIMUM_XFER_CHUNKSIZE_BYTES;   // PLDM_REDFISH_DEV_MAXIMUM_XFER_CHUNKSIZE_BYTES是否需要smbus和vdm各自保存一个
    g_pldm_redfish_base_info.mc_maximum_xfer_chunksize_bytes = MIN(g_pldm_redfish_base_info.mc_maximum_xfer_chunksize_bytes, req_dat->mc_maximum_xfer_chunksize_bytes);

    *pkt_len += sizeof(pldm_redfish_negotiate_medium_parameters_rsp_dat_t);
}

static int schema_transfer_handle_get(u32 resource_id, u8 schemaclass)
{
    if (schemaclass > SCHEMACLASS_REGISTRY)
        return PLDM_ERROR_UNSUPPORTED;

    int ret = PLDM_ERROR_NO_SUCH_RESOURCE;   // the source_id is not supported
    ret = resource_id_to_resource_identity(resource_id);
    if ((resource_id == PLDM_BASE_REGISTER_DICT_RESOURCE_ID) && (schemaclass == SCHEMACLASS_REGISTRY))
        ret = REGISTRY;

    u8 temp = !((schemaclass == SCHEMACLASS_MAJOR) || (schemaclass == SCHEMACLASS_COLLECTION_MEMBER_TYPE));
    u8 state = (ret == ALL_SCHEMA);
    state |= ((ret <= NETWORK_INTERFACE) && (ret >= NETWORK_ADAPTER) && (schemaclass != SCHEMACLASS_MAJOR));
    state |= ((ret <= NETWORK_DEVICE_FUNC_COLLECTION) && (ret >= PORT_COLLECTION) && temp);
    state |= ((ret <= ETH_INTERFACE) && (ret >= NETWORK_DEVICE_FUNC) && (schemaclass != SCHEMACLASS_MAJOR));
    state |= ((ret == ETH_INTERFACE_COLLECTION) && temp);
    if (state) {
        switch (schemaclass) {
            case SCHEMACLASS_EVENT:
                ret = EVENT;
                break;
            case SCHEMACLASS_ANNOTATION:
                ret = REDFISH_PAYLOAD_ANNOTATIONS;
                break;
            case SCHEMACLASS_ERROR:
                ret = REDFISH_ERROR;
                break;
            default:
                ret = PLDM_ERROR_INVALID_DATA;
                break;
        }
    }
    return ret;
}

static void pldm_redfish_get_schema_dictionary(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_redfish_get_schema_dictionary_req_dat_t *req_dat = (pldm_redfish_get_schema_dictionary_req_dat_t *)(pkt->req_buf);
    pldm_redfish_get_schema_dictionary_rsp_dat_t *rsp_dat = (pldm_redfish_get_schema_dictionary_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    int resource_identify = schema_transfer_handle_get(req_dat->resource_id, req_dat->requested_schemaclass);
    if ((resource_identify == PLDM_ERROR_NO_SUCH_RESOURCE) || (resource_identify == PLDM_ERROR_UNSUPPORTED) || (resource_identify == PLDM_ERROR_INVALID_DATA)) {
        rsp_hdr->cpl_code = resource_identify;
        return;
    }

    u8 ret = pldm_redfish_get_dict_data(req_dat->resource_id, g_needed_dict, pldm_redfish_get_dict_len(req_dat->resource_id));
    if (ret) {
        pldm_redfish_dictionary_format_t *dict_fmt = (pldm_redfish_dictionary_format_t *)&(g_needed_dict[DICT_FMT_HDR_LEN]);
        gs_op_buf.buf_ptr.len = dict_fmt->dictionary_size;
    } else {
        gs_op_buf.buf_ptr.len = 0;
    }

    gs_op_buf.multipartrecv_first_transfer_handle = 0;
    rsp_dat->dictionary_fmt = 0x00;
    rsp_dat->transfer_handle = gs_op_buf.multipartrecv_first_transfer_handle;    // 后续当MC将此handle传输回来后，RDE设备通过将此handle作为schema/dictionary地址数组索引转换成地址后，将地址对应数据返回给MC
    *pkt_len += sizeof(pldm_redfish_get_schema_dictionary_rsp_dat_t);
}

static void pldm_redfish_get_schema_uri(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_redfish_get_schema_uri_req_dat_t *req_dat = (pldm_redfish_get_schema_uri_req_dat_t *)(pkt->req_buf);
    pldm_redfish_get_schema_uri_rsp_dat_t *rsp_dat = (pldm_redfish_get_schema_uri_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));
    char *schema_uri_prefix = "https://redfish.dmtf.org/schemas/";

    if (req_dat->oem_extension_num != 0) {
        rsp_hdr->cpl_code = PLDM_ERROR_INVALID_DATA;
        goto L_ERR;
    }

    int resource_identify = schema_transfer_handle_get(req_dat->resource_id, req_dat->requested_schemaclass);
    if ((resource_identify == PLDM_ERROR_NO_SUCH_RESOURCE) || (resource_identify == PLDM_ERROR_UNSUPPORTED) || (resource_identify == PLDM_ERROR_INVALID_DATA)) {
        rsp_hdr->cpl_code = resource_identify;
        return;
    }

    rsp_dat->string_frag_cnt = 1;

    u8 uri_idx = (req_dat->requested_schemaclass == SCHEMACLASS_MAJOR) ? 0 : 1;
    if (resource_identify <= REGISTRY) uri_idx = 0;
    u8 uri_prefix_len = cm_strlen(schema_uri_prefix);
    u8 uri_suffix_len = cm_strlen(schema_info[resource_identify].uri[uri_idx]);
    mctp_memcpy_fast(rsp_dat->schema_uri, schema_uri_prefix, uri_prefix_len);
    mctp_memcpy_fast(&(rsp_dat->schema_uri[uri_prefix_len]), &(schema_info[resource_identify].uri[uri_idx]), uri_suffix_len);
    rsp_dat->schema_uri[uri_prefix_len + uri_suffix_len] = '\0';

    *pkt_len += sizeof(pldm_redfish_get_schema_uri_rsp_dat_t) + uri_prefix_len + uri_suffix_len;
L_ERR:
    LOG("resource_id : %lld, cpl_code : %#x, resource_identify : %d", req_dat->resource_id, rsp_hdr->cpl_code, resource_identify);
    return;
}

static void pldm_redfish_get_resource_etag(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_redfish_get_resource_etag_req_dat_t *req_dat = (pldm_redfish_get_resource_etag_req_dat_t *)(pkt->req_buf);
    pldm_redfish_get_resource_etag_rsp_dat_t *rsp_dat = (pldm_redfish_get_resource_etag_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    u8 resource_identify = resource_id_to_resource_identity(req_dat->resource_id);
    if (resource_identify == 0xFF) {
        rsp_hdr->cpl_code = PLDM_ERROR_NO_SUCH_RESOURCE;
        return;
    }
    (void)rsp_dat->eTag;
    *pkt_len += sizeof(pldm_redfish_get_resource_etag_rsp_dat_t);
}

static void pldm_redfish_get_oem_count(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_redfish_get_oem_count_req_dat_t *req_dat = (pldm_redfish_get_oem_count_req_dat_t *)(pkt->req_buf);
    pldm_redfish_get_oem_count_rsp_dat_t *rsp_dat = (pldm_redfish_get_oem_count_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    u8 resource_identify = resource_id_to_resource_identity(req_dat->resource_id);
    if (resource_identify == 0xFF) {
        rsp_hdr->cpl_code = PLDM_ERROR_NO_SUCH_RESOURCE;
        return;
    }
    // if (req_dat->requested_schemaclass == SCHEMACLASS_REGISTRY || req_dat->requested_schemaclass == SCHEMACLASS_ANNOTATION) {   /* Note: Redfish does not allow OEM extensions to Annotation and Registry schemas. */
    //     rsp_hdr->cpl_code = PLDM_ERROR_UNSUPPORTED;
    //     return;
    // }

    rsp_dat->oem_cnt = 0;
    /* The number of OEM extensions associated with the schema. For schema classes that do not support OEM 
       extensions this value must be zero. */
    *pkt_len = sizeof(pldm_redfish_get_oem_count_rsp_dat_t);
}

static void pldm_redfish_get_oem_name(protocol_msg_t *pkt, int *pkt_len)
{
    // pldm_redfish_get_oem_name_req_dat_t *req_dat = (pldm_redfish_get_oem_name_req_dat_t *)(pkt->req_buf);
    // pldm_redfish_get_oem_name_rsp_dat_t *rsp_dat = (pldm_redfish_get_oem_name_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));
    rsp_hdr->cpl_code = PLDM_ERROR_INVALID_DATA;
    /* A response code of ERROR_INVALID_DATA is used to indicate when the supplied index does not exist in the 
       schema or when the schema class does not support OEM schemas. */
}

static void pldm_redfish_get_registry_count(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_redfish_get_registry_count_rsp_dat_t *rsp_dat = (pldm_redfish_get_registry_count_rsp_dat_t *)(pkt->rsp_buf);

    rsp_dat->registry_cnt = 1;
    *pkt_len += sizeof(pldm_redfish_get_registry_count_rsp_dat_t);
}

static void pldm_redfish_get_registry_details(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_redfish_get_registry_details_req_dat_t *req_dat = (pldm_redfish_get_registry_details_req_dat_t *)(pkt->req_buf);
    pldm_redfish_get_registry_details_rsp_dat_t *rsp_dat = (pldm_redfish_get_registry_details_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    char *registry_prefix = "NetworkDeviceRegistry";

    if (req_dat->registry_idx != 0) {
        rsp_hdr->cpl_code = PLDM_ERROR_INVALID_DATA;
        return;
    }
    mctp_memcpy_fast(rsp_dat->registry_prefix, registry_prefix, cm_strlen(registry_prefix));
    (void)rsp_dat->registry_uri;
    rsp_dat->registry_language[0] = 'e';
    rsp_dat->registry_language[1] = 'n';                                /* Language in which the registry is published, as an ISO 639-1 two-letter code.to be determind. */
    rsp_dat->ver_cnt = 1;
    rsp_dat->version[0] = 0xF1F0F100;                                   /* E810 12.8.6.5.5.2 NetworkDevice Registry v1.0.1(E810) */
    rsp_dat->registry_prefix[cm_strlen(registry_prefix)] = '\0';

    *pkt_len += sizeof(pldm_redfish_get_registry_details_rsp_dat_t) + sizeof(u32);
}

static void pldm_redfish_get_msg_registry(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_redfish_get_msg_registry_req_dat_t *req_dat = (pldm_redfish_get_msg_registry_req_dat_t *)(pkt->req_buf);
    pldm_redfish_get_msg_registry_rsp_dat_t *rsp_dat = (pldm_redfish_get_msg_registry_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    if (req_dat->registry_idx != 0) {
        rsp_hdr->cpl_code = PLDM_ERROR_INVALID_DATA;
        return;
    }

    u8 ret = pldm_redfish_get_dict_data(PLDM_BASE_REGISTER_DICT_RESOURCE_ID, g_needed_dict, PLDM_REDFISH_MSG_REGISTER_DICT_LEN);
    if (ret) {
        pldm_redfish_dictionary_format_t *dict_fmt = (pldm_redfish_dictionary_format_t *)&(g_needed_dict[DICT_FMT_HDR_LEN]);
        gs_op_buf.buf_ptr.len = dict_fmt->dictionary_size;
    } else {
        gs_op_buf.buf_ptr.len = 0;
    }

    gs_op_buf.multipartrecv_first_transfer_handle = 0;
    rsp_dat->schema_fmt = JSON;
    rsp_dat->transfer_handle = gs_op_buf.multipartrecv_first_transfer_handle;
    *pkt_len += sizeof(pldm_redfish_get_msg_registry_rsp_dat_t);
}

static void pldm_redfish_rde_operation_init(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_redfish_rde_operation_init_req_dat_t *req_dat = (pldm_redfish_rde_operation_init_req_dat_t *)(pkt->req_buf);
    pldm_redfish_rde_operation_init_rsp_dat_t *rsp_dat = (pldm_redfish_rde_operation_init_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    u8 resource_identify = 0xFF;
    if (g_pldm_monitor_info.repo_state == PLDM_REPO_UPDATE_IN_PROGRESS) {
        rsp_hdr->cpl_code = PLDM_ERROR_NOT_READY;   // Firmware is in the middle of an operation preventing the access.
        goto L_ERR;
    }
    resource_identify = resource_id_to_resource_identity(req_dat->op_identify.resource_id);
    if (resource_identify == 0xFF) {
        rsp_hdr->cpl_code = PLDM_ERROR_NO_SUCH_RESOURCE;    // resource ID is not advertised by the device
        goto L_ERR;
    }
    if (!(BIT(req_dat->op_type) & schema_info[resource_identify].allowed_op)) {      /* if operation not allowed for the requested resource (e.g. Update a RO resource). */
        rsp_hdr->cpl_code = PLDM_ERROR_NOT_ALLOWED;
        goto L_ERR;
    }
    if (g_pldm_redfish_base_info.prev_op_identify.resource_id == req_dat->op_identify.resource_id && \
        g_pldm_redfish_base_info.prev_op_identify.op_id == req_dat->op_identify.op_id) {
        rsp_hdr->cpl_code = PLDM_ERROR_OPERATION_EXISTS;                        /* if attempt to re-create the same action with same {ResourceID, OperationID} */
        goto L_ERR;
    }
    if (!(req_dat->op_identify.op_id & CBIT(15))) {                             /* if OperationID MSB bit is cleared (device owned operation ID). */
        rsp_hdr->cpl_code = PLDM_ERROR_INVALID_DATA;
        goto L_ERR;
    }

    if (req_dat->op_flg & CBIT(1)) {    // operation is Update or Replace, and have req_data
        if (req_dat->req_payload_len > 0) {
            cm_memcpy(gs_op_buf.op_data.data, &(req_dat->op_locator[req_dat->op_locator_len]), req_dat->req_payload_len);
            gs_op_buf.op_data.len = req_dat->req_payload_len;
            req_dat->op_flg &= ~CBIT(1);
        } else {
            rsp_dat->op_status = OPERATION_NEEDS_INPUT;
            gs_op_buf.multipartsend_first_transfer_handle = req_dat->senddata_transfer_handle;
            gs_op_buf.multipartsend_next_transfer_handle = gs_op_buf.multipartsend_first_transfer_handle;
        }
    }

    g_pldm_redfish_base_info.prev_op_identify = req_dat->op_identify;
    gs_op_info.op_type = req_dat->op_type;
    gs_op_info.op_flag = req_dat->op_flg & (CBIT(1) | CBIT(2));
    if (req_dat->op_flg & CBIT(0)) {    // locator_valid bit
        gs_op_buf.op_locator[0] = req_dat->op_locator_len;
        cm_memcpy(&(gs_op_buf.op_locator[1]), req_dat->op_locator, req_dat->op_locator_len);
    }

    if (gs_op_info.op_flag & (CBIT(1) | CBIT(2))) {        // need SupplyCustomRequestParameters Cmd
        rsp_dat->op_status = OPERATION_NEEDS_INPUT;
        gs_op_info.dev_status = rsp_dat->op_status;
    } else {
        pldm_redfish_op_triggered();
    }

L_ERR:
    rsp_dat->permission_flg = CBIT(0) | CBIT(5);
    if (rsp_hdr->cpl_code == PLDM_ERROR_NOT_ALLOWED) {
        rsp_dat->permission_flg = 0;
    } else if (resource_identify != 0xFF) {
        if (schema_info[resource_identify].allowed_op & (BIT(UPDATE) | BIT(REPLACE))) {
            rsp_dat->permission_flg |= (CBIT(1) | CBIT(2)); /* 10 0111, bit 0 and 5 are always set; bit 1 and 2 are set if update and replace operation are allowed */
        }
    }

    rsp_dat->op_execution_flg = CBIT(0);                                        /* bit0 : TaskSpawned */
    if (rsp_hdr->cpl_code != MCTP_COMMAND_SUCCESS) {
        rsp_dat->cpl_percentage = 255;                                          /* Return 255 if operation is not valid. */
        rsp_dat->op_execution_flg = 0;
    } else if (gs_op_info.dev_status >= OPERATION_TRIGGERED) {
        rsp_dat->cpl_percentage = 254;
    } else {
        rsp_dat->cpl_percentage = 0;                                            /* Return zero if the Operation has not yet been triggered or if the Operation has failed. */
    }
    rsp_dat->cpl_time_sec = 0xFFFFFFFF;                                         /* Return 0xFFFF FFFF - no support */
    /* 0x0: no data to transfer from endpoint to MC; 0xFFFFFFFF: operation no complete; should not be a direct address in memory*/
    rsp_dat->result_transfer_handle = 0xFFFFFFFF;
    rsp_dat->rsp_payload_len = 0;                                               /* no rsp_data, the operation no finished */
    rsp_dat->op_status = gs_op_info.dev_status;
    rsp_dat->etag.format = UTF_8;                                               /* if operation has failed or not yet finished, the ETag field is skipped */
    rsp_dat->etag.len = 0;

    *pkt_len += sizeof(pldm_redfish_rde_operation_init_rsp_dat_t);
    LOG("cpl_code : %#x, resource_id : %lld, op_id : %d", rsp_hdr->cpl_code, req_dat->op_identify.resource_id, req_dat->op_identify.op_id);
}

static void pldm_redfish_supply_custom_request_parameters(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_redfish_supply_custom_request_parameters_req_dat_t *req_dat = (pldm_redfish_supply_custom_request_parameters_req_dat_t *)(pkt->req_buf);
    pldm_redfish_supply_custom_request_parameters_rsp_dat_t *rsp_dat = (pldm_redfish_supply_custom_request_parameters_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    u8 resource_identify = resource_id_to_resource_identity(req_dat->op_identify.resource_id);
    if (resource_identify == 0xFF) {
        rsp_hdr->cpl_code = PLDM_ERROR_NO_SUCH_RESOURCE;    // resource ID is not advertised by the device
        goto L_ERR;
    }
    if (g_pldm_redfish_base_info.prev_op_identify.resource_id != req_dat->op_identify.resource_id || \
        g_pldm_redfish_base_info.prev_op_identify.op_id != req_dat->op_identify.op_id || \
        !(gs_op_info.op_flag & CBIT(2))) {
        rsp_hdr->cpl_code = PLDM_ERROR_UNEXPECTED;
        goto L_ERR;
    }
    if (req_dat->etag_op >= 3) {
        rsp_hdr->cpl_code = PLDM_ERROR_UNSUPPORTED;
        goto L_ERR;
    }

    u8 state = (req_dat->etag_op == ETAG_IF_MATCH) && (req_dat->etag_cnt != 1);
    state = state || ((req_dat->etag_op == ETAG_IF_NONE_MATCH) && (req_dat->etag_cnt == 0));
    state = state || ((req_dat->etag_op == ETAG_IGNORE) && (req_dat->etag_cnt != 0));
    if (state) {
        rsp_hdr->cpl_code = PLDM_ERROR_INVALID_DATA;
        goto L_ERR;
    }

    u32 *ptr = (u32 *)&req_dat->etag;
    for (int i = 0; i < req_dat->etag_cnt; i++) {
        varstring *etag_val= (varstring *)ptr;
        cm_memcpy(etag[i], (void *)&(etag_val->len), etag_val->len + 1);
        ptr += (2 + etag_val->len);
    }

    if (req_dat->etag_op == ETAG_IF_MATCH) {
        // do the action only if the calculated ETag for the resource == ETag [0]
        u8 *etag_string = "etag"; // the calculated ETag
        int state = cm_memcmp(etag_string, &etag[0][1], etag[0][0]);
        if (state) {
            rsp_hdr->cpl_code = PLDM_ERROR_ETAG_MATCH;
            goto L_ERR;
        }
    } else if (req_dat->etag_op == ETAG_IF_NONE_MATCH) {
        // do the action only if the calculated ETag for the resource != all of {ETag [0] … ETag[ETagCount-1]. 
        u8 *etag_string = "etag";
        u32 etag_len = cm_strlen(etag_string);
        int state = 0;
        for (int i = 0; i < req_dat->etag_cnt; i++) {
            state = etag_len - etag[i][0];
            if (state) {
                continue;
            }
            state = cm_memcmp(etag_string, &etag[i][1], etag_len);
            if (!state) {
                rsp_hdr->cpl_code = PLDM_ERROR_ETAG_MATCH;
                goto L_ERR;
            }
        }
    }

    if (gs_op_info.op_type == READ) {   // op_task中检查op，如果是READ则需要用到下面两个参数
        gs_op_info.collection_skip = req_dat->collection_skip;         /* Zero means return all. */
        gs_op_info.collection_top = req_dat->collection_top;           /* 0xFFFF means return all. */
        if (gs_op_info.collection_skip > MAX_LAN_NUM) {
            rsp_hdr->cpl_code = PLDM_ERROR_OPERATION_FAILED;
            goto L_ERR;
        }
    }

    gs_op_info.op_flag &= ~CBIT(2);
    if (!(gs_op_info.op_flag & CBIT(1))) {
        pldm_redfish_op_triggered();
    }

L_ERR:
    rsp_dat->op_execution_flg = CBIT(0);
    rsp_dat->cpl_percentage = 254;
    if (rsp_hdr->cpl_code != MCTP_COMMAND_SUCCESS) {
        rsp_dat->op_execution_flg = 0;
    } else if (gs_op_info.dev_status < OPERATION_TRIGGERED) {
        rsp_dat->cpl_percentage = 0;                                            /* Return zero if the Operation has not yet been triggered or if the Operation has failed. */
    }
    rsp_dat->cpl_time_sec = 0xFFFFFFFF;                                         /* Return 0xFFFF FFFF - no support */
    /* 0x0: no data to transfer from endpoint to MC; 0xFFFFFFFF: operation no complete; should not be a direct address in memory*/
    rsp_dat->result_transfer_handle = 0xFFFFFFFF;
    rsp_dat->rsp_payload_len = 0;                                               /* no rsp_data, the operation no finished */
    rsp_dat->op_status = gs_op_info.dev_status;
    rsp_dat->etag.format = UTF_8;                                               /* if operation has failed or not yet finished, the ETag field is skipped */
    rsp_dat->etag.len = 0;

    rsp_dat->permission_flg = CBIT(0) | CBIT(5);
    if (resource_identify != 0xFF) {
        if (schema_info[resource_identify].allowed_op & (BIT(UPDATE) | BIT(REPLACE))) {
            rsp_dat->permission_flg |= (CBIT(1) | CBIT(2));
        }
    }

    *pkt_len += sizeof(pldm_redfish_supply_custom_request_parameters_rsp_dat_t);
    LOG("%s, cpl_code : %#x", __FUNCTION__, rsp_hdr->cpl_code);
}

/* This command is currently not supported, as the NIC does not provide any Custom Response parameters. */
static void pldm_redfish_retrieve_custom_response_parameters(protocol_msg_t *pkt, int *pkt_len)
{
    // pldm_redfish_retrieve_custom_response_parameters_req_dat_t *req_dat = (pldm_redfish_retrieve_custom_response_parameters_req_dat_t *)(pkt->req_buf);
    // pldm_redfish_retrieve_custom_response_parameters_rsp_dat_t *rsp_dat = (pldm_redfish_retrieve_custom_response_parameters_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    // if (g_pldm_redfish_base_info.prev_op_identify.resource_id != req_dat->op_identify.resource_id) {
    //     rsp_hdr->cpl_code = PLDM_ERROR_NO_SUCH_RESOURCE;
    //     return;
    // }else if (g_pldm_redfish_base_info.prev_op_identify.op_id != req_dat->op_identify.op_id) {
    //     rsp_hdr->cpl_code = PLDM_ERROR_UNEXPECTED;
    //     return;
    // }
    // rsp_dat->deferral_timeframe = 0xFF;                     /* Return 0xFF (unknown). */
    // rsp_dat->new_resource_id = 0;                           /* Return 0 - no support for Create command. */
    // rsp_dat->rsp_hdr_cnt = 0;                               /* Return 0 - no support for custom headers. */
    // *pkt_len += sizeof(pldm_redfish_retrieve_custom_response_parameters_rsp_dat_t);

    rsp_hdr->cpl_code = PLDM_ERROR_UNSUPPORTED;
}

static void pldm_redfish_rde_operation_complete(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_redfish_rde_operation_complete_req_dat_t *req_dat = (pldm_redfish_rde_operation_complete_req_dat_t *)(pkt->req_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    u8 resource_identify = resource_id_to_resource_identity(req_dat->op_identify.resource_id);
    if (resource_identify == 0xFF) {
        rsp_hdr->cpl_code = PLDM_ERROR_NO_SUCH_RESOURCE;    // resource ID is not advertised by the device
        goto L_ERR;
    }
    if (g_pldm_redfish_base_info.prev_op_identify.resource_id != req_dat->op_identify.resource_id || \
        g_pldm_redfish_base_info.prev_op_identify.op_id != req_dat->op_identify.op_id) {
        rsp_hdr->cpl_code = PLDM_ERROR_UNEXPECTED;
        goto L_ERR;
    }
    pldm_redfish_clear_op_param();
L_ERR:
    LOG("cpl code : %#x, resource_id : %lld, op_id : %d", rsp_hdr->cpl_code, req_dat->op_identify.resource_id, req_dat->op_identify.op_id);
}

static void pldm_redfish_rde_operation_status(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_redfish_rde_operation_status_req_dat_t *req_dat = (pldm_redfish_rde_operation_status_req_dat_t *)(pkt->req_buf);
    pldm_redfish_rde_operation_status_rsp_dat_t *rsp_dat = (pldm_redfish_rde_operation_status_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    u8 resource_identify = resource_id_to_resource_identity(req_dat->op_identify.resource_id);
    if (resource_identify == 0xFF) {
        rsp_hdr->cpl_code = PLDM_ERROR_NO_SUCH_RESOURCE;    // resource ID is not advertised by the device
        goto L_ERR;
    } else if (g_pldm_redfish_base_info.prev_op_identify.resource_id != req_dat->op_identify.resource_id || \
        g_pldm_redfish_base_info.prev_op_identify.op_id != req_dat->op_identify.op_id) {
        rsp_hdr->cpl_code = PLDM_ERROR_UNEXPECTED;
        goto L_ERR;
    }

    rsp_dat->op_status = gs_op_info.dev_status;
    rsp_dat->cpl_time_sec = 0xFFFFFFFF;
    if ((rsp_dat->op_status == OPERATION_NEEDS_INPUT) || (rsp_dat->op_status == OPERATION_FAILED)) {
        rsp_dat->cpl_percentage = 0;                       // the Operation has not yet been triggered
    } else {
        rsp_dat->cpl_percentage = 254;
    }

    rsp_dat->op_execution_flg = CBIT(0);
    if (rsp_dat->op_status == OPERATION_INACTIVE) {
        rsp_dat->op_execution_flg = 0;
    } else if ((rsp_dat->op_status == OPERATION_COMPLETED) && (gs_op_info.op_type == READ)) {
        rsp_dat->op_execution_flg |= CBIT(2) | CBIT(3);    // 1101
    } else if ((rsp_dat->op_status == OPERATION_COMPLETED) && (gs_op_info.op_type == HEAD)) {
        rsp_dat->op_execution_flg |= CBIT(3);
    }

    if (rsp_dat->op_status == OPERATION_HAVE_RESULTS) {
        // 判断操作结果bejencoding长度
            // 大于协商消息大小：rsp_dat->result_transfer_handle = 索引; rsp_dat->rsp_payload_len = 0; gs_op_buf.multipartrecv_first_transfer_handle = rsp_dat->result_transfer_handle;
            // 小于等于协商消息大小：拷贝数据到payload；rsp_dat->result_transfer_handle = 0; rsp_dat->rsp_payload_len = bejencoding_size; gs_op_info.dev_status = OPERATION_COMPLETED;
            if (gs_op_buf.op_result.len <= g_pldm_redfish_base_info.mc_maximum_xfer_chunksize_bytes) {
                u8 *cpy_ptr = &(rsp_dat->etag.val[rsp_dat->etag.len]);
                cm_memcpy(cpy_ptr, gs_op_buf.op_result.data, gs_op_buf.op_result.len);
                rsp_dat->rsp_payload_len = gs_op_buf.op_result.len;
                gs_op_info.dev_status = OPERATION_COMPLETED;
            } else {
                rsp_dat->result_transfer_handle = 0;
                rsp_dat->rsp_payload_len = 0;
                gs_op_buf.multipartrecv_first_transfer_handle = rsp_dat->result_transfer_handle;
            }
    } else if ((rsp_dat->op_status == OPERATION_INACTIVE) || (rsp_dat->op_status >= OPERATION_COMPLETED)) {
        rsp_dat->result_transfer_handle = 0;
        rsp_dat->rsp_payload_len = 0;
    } else {
        rsp_dat->result_transfer_handle = 0xFFFFFFFF;
        rsp_dat->rsp_payload_len = 0;
    }

    rsp_dat->permission_flg = CBIT(0) | CBIT(5);
    if (resource_identify != 0xFF) {
        if (schema_info[resource_identify].allowed_op & (BIT(UPDATE) | BIT(REPLACE))) {
            rsp_dat->permission_flg |= (CBIT(1) | CBIT(2));
        }
    }

    if (rsp_dat->op_status == OPERATION_COMPLETED) {
        // 补充rsp_dat->etag
    } else{
        rsp_dat->etag.format = UTF_8;
        rsp_dat->etag.len = 0;
    }

    *pkt_len += (sizeof(pldm_redfish_rde_operation_status_rsp_dat_t) + rsp_dat->rsp_payload_len + rsp_dat->etag.len);
    return;

L_ERR:
    rsp_dat->op_status = gs_op_info.dev_status;
    rsp_dat->cpl_percentage = 254;
    rsp_dat->cpl_time_sec = 0xFFFFFFFF;
    rsp_dat->op_execution_flg = 0;
    rsp_dat->result_transfer_handle = 0;
    rsp_dat->rsp_payload_len = 0;
    rsp_dat->etag.format = UTF_8;
    rsp_dat->etag.len = 0;

    rsp_dat->permission_flg = CBIT(0) | CBIT(5);
    if (rsp_hdr->cpl_code == PLDM_ERROR_NO_SUCH_RESOURCE) {
        rsp_dat->permission_flg = 0;
    } else if (resource_identify != 0xFF) {
        if (schema_info[resource_identify].allowed_op & (BIT(UPDATE) | BIT(REPLACE))) {
            rsp_dat->permission_flg |= (CBIT(1) | CBIT(2));
        }
    }

    *pkt_len += sizeof(pldm_redfish_rde_operation_status_rsp_dat_t);
}

static void pldm_redfish_rde_operation_kill(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_redfish_rde_operation_kill_req_dat_t *req_dat = (pldm_redfish_rde_operation_kill_req_dat_t *)(pkt->req_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    u8 resource_identify = resource_id_to_resource_identity(req_dat->op_identify.resource_id);
    if (resource_identify == 0xFF) {
        rsp_hdr->cpl_code = PLDM_ERROR_NO_SUCH_RESOURCE;    // resource ID is not advertised by the device
    } else if (g_pldm_redfish_base_info.prev_op_identify.resource_id != req_dat->op_identify.resource_id || \
        g_pldm_redfish_base_info.prev_op_identify.op_id != req_dat->op_identify.op_id) {
        rsp_hdr->cpl_code = PLDM_ERROR_UNEXPECTED;
    } else if (req_dat->kill_flg == 0x2) {                  /* bit1 : run_to_completion, bit0 : discard_record */
        rsp_hdr->cpl_code = PLDM_ERROR_INVALID_DATA;
    } else if (req_dat->kill_flg == 0x3) {
        switch (gs_op_info.dev_status) {
            case OPERATION_NEEDS_INPUT:
                rsp_hdr->cpl_code = PLDM_ERROR_UNEXPECTED;
                break;
            case OPERATION_HAVE_RESULTS:
                rsp_hdr->cpl_code = PLDM_ERROR_OPERATION_UNKILLABLE;
                break;
            case OPERATION_FAILED:
                rsp_hdr->cpl_code = PLDM_ERROR_OPERATION_FAILED;
                break;
            case OPERATION_ABANDONED:
                rsp_hdr->cpl_code = PLDM_ERROR_OPERATION_ABANDONED;
                break;
            default:
                break;
        }
    }
    if (rsp_hdr->cpl_code == MCTP_COMMAND_SUCCESS) {
        /* The RDE Device shall kill the Operation if the Operation can be killed;  */
        pldm_redfish_clear_op_param();
    }
}

static void pldm_redfish_rde_operation_enumerate(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_redfish_rde_operation_enumerate_rsp_dat_t *rsp_dat = (pldm_redfish_rde_operation_enumerate_rsp_dat_t *)(pkt->rsp_buf);
    rsp_dat->op_cnt = 0;
    if (g_pldm_redfish_base_info.prev_op_identify.resource_id != 0) {
        rsp_dat->op_cnt = 1;
        rsp_dat->field[0].op_identify = g_pldm_redfish_base_info.prev_op_identify;
        rsp_dat->field[0].op_type = gs_op_info.op_type;
    }
    *pkt_len += sizeof(pldm_redfish_rde_operation_enumerate_rsp_dat_t) + rsp_dat->op_cnt * sizeof(pldm_redfish_rde_operation_enumerate_field_t);
}

static void pldm_redfish_rde_multipart_send(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_redfish_rde_multipart_send_req_dat_t *req_dat = (pldm_redfish_rde_multipart_send_req_dat_t *)(pkt->req_buf);
    pldm_redfish_rde_multipart_send_rsp_dat_t *rsp_dat = (pldm_redfish_rde_multipart_send_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    if (g_pldm_redfish_base_info.prev_op_identify.op_id != req_dat->op_id) {
        rsp_hdr->cpl_code = PLDM_ERROR_UNEXPECTED;
        goto L_ERR;
    }

    if (req_dat->data_transfer_handle != gs_op_buf.multipartsend_next_transfer_handle) {
        rsp_hdr->cpl_code = PLDM_ERROR_INVALID_DATA;
        goto L_ERR;
    }
    gs_op_buf.multipartsend_next_transfer_handle = req_dat->next_transfer_handle;

    cm_memcpy(&gs_op_buf.op_data.data[gs_op_buf.op_data.len], req_dat->data, req_dat->data_len_bytes);
    gs_op_buf.op_data.len += req_dat->data_len_bytes;
    if ((req_dat->transfer_flg == PLDM_TRANSFER_FLG_START_AND_END) || (req_dat->transfer_flg == PLDM_TRANSFER_FLG_END)) {
        gs_op_buf.op_data.len -= sizeof(u32);
        u32 crc = crc32_pldm(0xFFFFFFFFUL, gs_op_buf.op_data.data, (gs_op_buf.op_data.len));
        LOG("crc result : %s", crc == *(u32 *)(&gs_op_buf.op_data.data[gs_op_buf.op_data.len]) ? "true" : "false");
        if (crc != *(u32 *)(&gs_op_buf.op_data.data[gs_op_buf.op_data.len])) {
            rsp_hdr->cpl_code = PLDM_ERROR_BAD_CHECKSUM;
            rsp_dat->transfer_op = XFER_FIRST_PART;
            gs_op_buf.multipartsend_next_transfer_handle = gs_op_buf.multipartsend_first_transfer_handle;
            gs_op_buf.op_data.len = 0;
        } else {
            rsp_dat->transfer_op = XFER_COMPLETE;
            gs_op_info.op_flag &= (~CBIT(1));
            if (!(gs_op_info.op_flag & CBIT(2))) {
                pldm_redfish_op_triggered();
            }
        }
    } else {
        rsp_dat->transfer_op = XFER_NEXT_PART;
    }
    *pkt_len += sizeof(pldm_redfish_rde_multipart_send_rsp_dat_t);

L_ERR:
    // LOG("%s, cpl_code : %#x", __FUNCTION__, rsp_hdr->cpl_code);
    return;
}

static void pldm_redfish_rde_multipart_receive(protocol_msg_t *pkt, int *pkt_len)
{
    pldm_redfish_rde_multipart_receive_req_dat_t *req_dat = (pldm_redfish_rde_multipart_receive_req_dat_t *)(pkt->req_buf);
    pldm_redfish_rde_multipart_receive_rsp_dat_t *rsp_dat = (pldm_redfish_rde_multipart_receive_rsp_dat_t *)(pkt->rsp_buf);
    pldm_response_t *rsp_hdr = (pldm_response_t *)(pkt->rsp_buf - sizeof(pldm_response_t));

    /* A value of zero is used for Dictionary transfer */
    if ((g_pldm_redfish_base_info.prev_op_identify.op_id != req_dat->op_id) && (0 != req_dat->op_id)) {
        rsp_hdr->cpl_code = PLDM_ERROR_UNEXPECTED;
        goto L_ERR;
    }

    u8 sta = (req_dat->transfer_op == XFER_FIRST_PART) && (req_dat->op_id == 0) && (req_dat->data_transfer_handle != gs_op_buf.multipartrecv_first_transfer_handle);
    sta = sta || ((req_dat->transfer_op == XFER_FIRST_PART) && (req_dat->op_id != 0) && (req_dat->data_transfer_handle != gs_op_buf.multipartrecv_first_transfer_handle));
    sta = sta || ((req_dat->transfer_op == XFER_NEXT_PART) && (req_dat->data_transfer_handle != gs_op_buf.multipartrecv_next_transfer_handle));
    if (sta) {
        rsp_hdr->cpl_code = PLDM_ERROR_INVALID_DATA;
        goto L_ERR;
    }

    if (req_dat->transfer_op == XFER_FIRST_PART) {
        if (req_dat->op_id == 0) {                          // schema transfer
            gs_op_buf.buf_ptr.data = &g_needed_dict[DICT_FMT_HDR_LEN];
        } else {                                            // op result transfer
            gs_op_buf.buf_ptr.data = gs_op_buf.op_result.data;
            gs_op_buf.buf_ptr.len = gs_op_buf.op_result.len;
        }
    }

    u16 max_pkt_len = g_pldm_redfish_base_info.mc_maximum_xfer_chunksize_bytes - PLDM_MULTI_RECV_FIELD_LEN;
    u32 remain_bytes = gs_op_buf.buf_ptr.len - gs_op_buf.multipartrecv_next_transfer_handle;
    u32 len = ((remain_bytes + 4) <= max_pkt_len) ? remain_bytes : max_pkt_len;
    /*  If appending the DataIntegrityChecksum would cause this response message to exceed the 
        negotiated maximum transfer chunk size (clause 10.2), the DataIntegrityChecksum shall be sent as 
        the only data in another chunk. */
    if (remain_bytes < max_pkt_len && (remain_bytes + 4) > max_pkt_len) {
        len = remain_bytes;
    }

    rsp_dat->data_len_bytes = len;
    if ((remain_bytes + 4) <= max_pkt_len) {
        if (gs_op_buf.multipartrecv_next_transfer_handle == 0) {
            rsp_dat->transfer_flg = PLDM_REDFISH_TRANSFER_START_END;
        } else {
            rsp_dat->transfer_flg = PLDM_REDFISH_TRANSFER_END;
        }
        rsp_dat->next_transfer_handle = 0;
        rsp_dat->data_len_bytes += sizeof(u32);         /* CRC32 */
        u32 crc = crc32_pldm(0xFFFFFFFFUL, gs_op_buf.buf_ptr.data, gs_op_buf.buf_ptr.len);
        cm_memcpy(&(rsp_dat->data[len]), &crc, sizeof(crc));
        *pkt_len += sizeof(u32);
        if (gs_op_info.dev_status == OPERATION_HAVE_RESULTS)
            gs_op_info.dev_status = OPERATION_COMPLETED;
    } else {
        if (gs_op_buf.multipartrecv_next_transfer_handle == gs_op_buf.multipartrecv_first_transfer_handle) {
            rsp_dat->transfer_flg = PLDM_REDFISH_TRANSFER_START;
        } else {
            rsp_dat->transfer_flg = PLDM_REDFISH_TRANSFER_MIDDLE;
        }
        rsp_dat->next_transfer_handle = req_dat->data_transfer_handle + len;
    }
    cm_memcpy(rsp_dat->data, &gs_op_buf.buf_ptr.data[gs_op_buf.multipartrecv_next_transfer_handle], len);
    gs_op_buf.multipartrecv_next_transfer_handle = rsp_dat->next_transfer_handle;
    *pkt_len += (sizeof(pldm_redfish_rde_multipart_receive_rsp_dat_t) + len);

L_ERR:
    LOG("cpl_code : %#x, op_id : %d, data_transfer_handle : %d", rsp_hdr->cpl_code, req_dat->op_id, req_dat->data_transfer_handle);
    return;
}

static pldm_cmd_func pldm_cmd_table[PLDM_REDFISH_CMD] =
{
    pldm_unsupport_cmd,                                     /* 0x00 */
    pldm_redfish_negotiate_redfish_parameters,              /* 0x01 */
    pldm_redfish_negotiate_medium_parameters,               /* 0x02 */
    pldm_redfish_get_schema_dictionary,                     /* 0x03 */
    pldm_redfish_get_schema_uri,                            /* 0x04 */
    pldm_redfish_get_resource_etag,                         /* 0x05 */
    pldm_redfish_get_oem_count,                             /* 0x06 */
    pldm_redfish_get_oem_name,                              /* 0x07 */
    pldm_redfish_get_registry_count,                        /* 0x08 */
    pldm_redfish_get_registry_details,                      /* 0x09 */
    pldm_unsupport_cmd,                                     /* 0x00 */
    pldm_redfish_get_msg_registry,                          /* 0x0b */
    pldm_unsupport_cmd,                                     /* 0x00 */
    pldm_unsupport_cmd,                                     /* 0x00 */
    pldm_unsupport_cmd,                                     /* 0x00 */
    pldm_unsupport_cmd,                                     /* 0x00 */
    pldm_redfish_rde_operation_init,                        /* 0x10 */
    pldm_redfish_supply_custom_request_parameters,          /* 0x11 */
    pldm_redfish_retrieve_custom_response_parameters,       /* 0x12 This command is currently not supported, as the NIC does not 
                                                                    provide any Custom Response parameters. */
    pldm_redfish_rde_operation_complete,                    /* 0x13 */
    pldm_redfish_rde_operation_status,                      /* 0x14 */
    pldm_redfish_rde_operation_kill,                        /* 0x15 */
    pldm_redfish_rde_operation_enumerate,                   /* 0x16 */
    // pldm_redfish_rde_multipart_send,                    /* 0x30 */
    // pldm_redfish_rde_multipart_receive                  /* 0x31 */
};

void pldm_redfish_process(protocol_msg_t *pkt, int *pkt_len, u32 cmd_code)
{
    pldm_cmd_func cmd_proc = NULL;

    if (cmd_code < PLDM_REDFISH_CMD) {
        cmd_proc = pldm_cmd_table[cmd_code];
    } else if (cmd_code == 0x30) {
        cmd_proc = pldm_redfish_rde_multipart_send;
    } else if (cmd_code == 0x31) {
        cmd_proc = pldm_redfish_rde_multipart_receive;
    } else {
        cmd_proc = pldm_unsupport_cmd;
    }

    return cmd_proc(pkt, pkt_len);
}

// #include "FreeRTOS.h"
// #include "task.h"
// static TaskHandle_t redfish_op_task_handle;

static void CM_FLASH_READ(u32 offset, u32 *buf, u32 size)
{
    FILE *fp = fopen("./build/pldm_redfish_dicts.bin", "r+b");
    if (!fp) {
        LOG("CM_FLASH_READ open file err!");
        return;
    }
    fseek(fp, offset, SEEK_SET);
    fread(buf, sizeof(u32), size, fp);
    fclose(fp);
}

// /* max dict is 8k, len is dw align */
u8 pldm_redfish_get_dict_data(u32 resource_id, u8 *dict, u16 len)
{
    u32 dict_addr = 0;
    pldm_redfish_dict_hdr_t *dict_info  = (pldm_redfish_dict_hdr_t *)g_dict_info;
    for (u8 i = 0; i < dict_info->num_of_dict; i++) {
        if (resource_id == dict_info->dict_info[i].resource_id) {
            dict_addr = dict_info->dict_info[i].offset;
            break;
        }
    }
    if (dict_addr) {
        // LOG("dict_addr : %d", dict_addr);
        CM_FLASH_READ(PLDM_REDFISH_DICT_BASE_ADDR + dict_addr, (u32 *)dict, len / sizeof(u32));
    } else {
        LOG("not found dict data, resource_id : %d", resource_id);
        return false;
    }
    return true;
}

u16 pldm_redfish_get_dict_len(u32 resource_id)
{
    u16 len = 0;
    u32 dict_resource_id[] = {
        PLDM_BASE_NETWORK_ADAPTER_RESOURCE_ID,
        PLDM_BASE_PCIE_DEV_RESOURCE_ID,
        PLDM_BASE_NETWORK_INTERFACE_RESOURCE_ID,
        PLDM_BASE_PORTS_RESOURCE_ID,
        PLDM_BASE_NETWORK_DEV_FUNCS_RESOURCE_ID,
        PLDM_BASE_PCIE_FUNCS_RESOURCE_ID,
        PLDM_BASE_ETH_INTERFACE_COLLECTION_RESOURCE_ID,
        PLDM_BASE_PORT_RESOURCE_ID,
        PLDM_BASE_NETWORK_DEV_FUNC_RESOURCE_ID,
        PLDM_BASE_PCIE_FUNC_RESOURCE_ID,
        PLDM_BASE_ETH_INTERFACE_RESOURCE_ID,
        PLDM_BASE_ANNOTATION_DICT_RESOURCE_ID,
        PLDM_BASE_EVENT_DICT_RESOURCE_ID,
        PLDM_BASE_REGISTER_DICT_RESOURCE_ID
    };
    u32 dict_len[] = {
        PLDM_REDFISH_NETWORK_ADAPTER_DICT_LEN,
        PLDM_REDFISH_PCIE_DEV_DICT_LEN,
        PLDM_REDFISH_NETWORK_INTERFACE_DICT_LEN,
        PLDM_REDFISH_PORTS_DICT_LEN,
        PLDM_REDFISH_NETWORK_DEV_FUNCS_DICT_LEN,
        PLDM_REDFISH_PCIE_FUNCS_DICT_LEN,
        PLDM_REDFISH_ETH_INTERFACE_COLLECTION_DICT_LEN,
        PLDM_REDFISH_PORT_DICT_LEN,
        PLDM_REDFISH_NETWORK_DEV_FUNC_DICT_LEN,
        PLDM_REDFISH_PCIE_FUNC_DICT_LEN,
        PLDM_REDFISH_ETH_INTERFACE_DICT_LEN,
        PLDM_REDFISH_ANNO_DICT_LEN,
        PLDM_REDFISH_EVENT_DICT_LEN,
        PLDM_REDFISH_MSG_REGISTER_DICT_LEN
    };
    u8 left = 0;
    u8 right = PLDM_REDFISH_DICT_NUM - 1;
    while (left <= right) {
        u8 mid = (left + right) / 2;
        if (dict_resource_id[mid] < resource_id) {
            left = mid + 1;
        } else if (dict_resource_id[mid] > resource_id) {
            right = mid - 1;
        } else {
            len = dict_len[mid];
            break;
        }
    }
    return len;
}

void pldm_redfish_op_triggered(void)
{
    gs_op_info.dev_status = OPERATION_TRIGGERED;
    // BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // (void)xHigherPriorityTaskWoken;
    // xTaskNotify(redfish_op_task_handle, 0x10000, eSetBits);
    pldm_redfish_op();
}

void pldm_redfish_init(void)
{
    g_pldm_redfish_base_info.mc_maximum_xfer_chunksize_bytes = PLDM_REDFISH_DEV_MAXIMUM_XFER_CHUNKSIZE_BYTES;
    g_pldm_redfish_base_info.prev_op_identify.resource_id = 0;
    g_pldm_redfish_base_info.prev_op_identify.op_id = 0;

    pldm_redfish_clear_op_param();

    pldm_bej_init();
    CM_FLASH_READ(PLDM_REDFISH_DICT_BASE_ADDR, (u32 *)g_dict_info, PLDM_REDFISH_DICT_INFO_LEN / sizeof(u32));

    pldm_redfish_get_dict_data(PLDM_BASE_ANNOTATION_DICT_RESOURCE_ID, g_anno_dict, sizeof(g_anno_dict));
}

void pldm_redfish_op(void)
{
        gs_op_info.dev_status = OPERATION_RUNNING;
        gs_op_buf.op_result.len = 0;

        pldm_cjson_t *bej_root = NULL;
        pldm_cjson_t *root = NULL;

        u8 *annc_dict = &g_anno_dict[DICT_FMT_HDR_LEN];
        u8 *dict = &g_needed_dict[DICT_FMT_HDR_LEN];

        u8 resourse_indenty = resource_id_to_resource_identity(g_pldm_redfish_base_info.prev_op_identify.resource_id);
        if (resourse_indenty > ETH_INTERFACE_COLLECTION) goto L_EXIT;

        resourse_indenty -= NETWORK_ADAPTER;

        u8 ret = pldm_redfish_get_dict_data(g_pldm_redfish_base_info.prev_op_identify.resource_id, \
        g_needed_dict, pldm_redfish_get_dict_len(g_pldm_redfish_base_info.prev_op_identify.resource_id));
        if (ret == false) goto L_EXIT;

        if (g_resourse_bej[resourse_indenty].is_bej)
            root = pldm_bej_decode(g_resourse_bej[resourse_indenty].data, g_resourse_bej[resourse_indenty].len, annc_dict, dict, root);
        else root = g_schemas[resourse_indenty](dict, annc_dict);

        if (!root) goto L_EXIT;

        if (gs_op_buf.op_data.len && gs_op_info.op_type != READ && gs_op_info.op_type != HEAD) {
            bej_root = pldm_bej_decode(&(gs_op_buf.op_data.data[sizeof(bejencoding_t)]), gs_op_buf.op_data.len - sizeof(bejencoding_t), annc_dict, dict, bej_root);
            if (!bej_root) goto L_EXIT;
        }

        switch (gs_op_info.op_type) {
            case READ:
                root = pldm_cjson_read(root, gs_op_info.collection_skip, gs_op_info.collection_top);
                break;
            case UPDATE:
                root = pldm_cjson_update(root, bej_root);
                break;
            case REPLACE:
                root = pldm_cjson_replace(root, bej_root);
                break;
            case ACTION:
                root = pldm_cjson_action(root);
                break;
            case HEAD:
                root = pldm_cjson_head(root);                   /*  not return message body information. */
                break;
            default :
                LOG("err op type : %d", gs_op_info.op_type);
                goto L_EXIT;
                break;
        }
        bej_root = NULL;
        pldm_cjson_cal_len_to_root(root, gs_op_info.op_type);
        // pldm_cjson_printf_root(root);
        LOG("op_type : %d", gs_op_info.op_type);
        if (gs_op_info.op_type != ACTION) {
            // cm_memcpy(gs_op_buf.op_result.data, gs_op_buf.op_data.data, sizeof(bejencoding_t));
            bejencoding_t *ptr = (bejencoding_t *)gs_op_buf.op_result.data;
            pldm_redfish_dictionary_format_t *dict_ptr = (pldm_redfish_dictionary_format_t *)dict;
            ptr->ver = dict_ptr->schema_version;
            ptr->schema_class = SCHEMACLASS_MAJOR;
            u8 *end_ptr = pldm_bej_encode(root, &(gs_op_buf.op_result.data[sizeof(bejencoding_t)]));
            gs_op_buf.op_result.len = end_ptr - gs_op_buf.op_result.data;
            // for (u16 i = 0; i < gs_op_buf.op_result.len; i++) {
                // printf("0x%02x, ", gs_op_buf.op_result.data[i]);
                // if (!((i + 1) % 8))
                //     printf("\n");
            // }
            LOG("encode len : %d", gs_op_buf.op_result.len);
        }
        // root = NULL;
        // pldm_cjson_pool_reinit();
        // bej_root = pldm_bej_decode(gs_op_buf.op_result.data, gs_op_buf.op_result.len, annc_dict, dict, bej_root);
        // if (bej_root) {
        //     // pldm_cjson_printf_root(bej_root);
        //     LOG("Decode success");
        // }
        if (root && gs_op_info.op_type != READ && gs_op_info.op_type != HEAD) {
            pldm_cjson_cal_len_to_root(root, OTHER_TYPE);
            u8 *ptr = pldm_bej_encode(root, g_resourse_bej[resourse_indenty].data);
            g_resourse_bej[resourse_indenty].is_bej = 1;
            g_resourse_bej[resourse_indenty].len = ptr - g_resourse_bej[resourse_indenty].data;
            // pldm_cjson_delete_node(root);
            root = NULL;
        }
L_EXIT:
        gs_op_info.dev_status = gs_op_buf.op_result.len ? OPERATION_HAVE_RESULTS : OPERATION_COMPLETED;
        pldm_cjson_pool_reinit();
        // pldm_redfish_task_execute_event_generate(g_pldm_monitor_info.pldm_event_rbuf, (pldm_redfish_task_executed_event_data_format_t *)&(g_pldm_redfish_base_info.prev_op_identify));
}

void pldm_redfish_op_task(void *param)
{
//     uint32_t ulNotifiedValue = 0x00;
//     BaseType_t xResult = pdFALSE;
//     redfish_op_task_handle = xTaskGetHandle("REDFISH");

//     // 所有op都在task中异步执行
//     while (1) {
//         xResult = xTaskNotifyWait( pdFALSE,     /* Don't clear bits on entry. */
//                         0xFFFF0000,              /* Clear all bits on exit. */
//                         &ulNotifiedValue,       /* Stores the notified value. */
//                         portMAX_DELAY );
//         if( xResult != pdPASS ) continue;

//         gs_op_info.dev_status = OPERATION_RUNNING;
//         gs_op_buf.op_result.len = 0;

//         pldm_cjson_t *bej_root = NULL;
//         pldm_cjson_t *root = NULL;

//         u8 *annc_dict = &g_anno_dict[DICT_FMT_HDR_LEN];
//         u8 *dict = &g_needed_dict[DICT_FMT_HDR_LEN];

//         u8 resourse_indenty = resource_id_to_resource_identity(g_pldm_redfish_base_info.prev_op_identify.resource_id);
//         if (resourse_indenty > ETH_INTERFACE_COLLECTION) goto L_EXIT;

//         resourse_indenty -= NETWORK_ADAPTER;

//         u8 ret = pldm_redfish_get_dict_data(g_pldm_redfish_base_info.prev_op_identify.resource_id, \
//         g_needed_dict, pldm_redfish_get_dict_len(g_pldm_redfish_base_info.prev_op_identify.resource_id));
//         if (ret == false) goto L_EXIT;

//         if (g_resourse_bej[resourse_indenty].is_bej) 
//             root = pldm_bej_decode(g_resourse_bej[resourse_indenty].data, annc_dict, dict, root);
//         else root = g_schemas[resourse_indenty](dict, annc_dict);

//         if (!root) goto L_EXIT;

//         if (gs_op_buf.op_data.len) {
//             bej_root = pldm_bej_decode(&(gs_op_buf.op_data.data[sizeof(bejencoding_t)]), annc_dict, dict, bej_root);
//             if (!bej_root) goto L_EXIT;
//         }

//         switch (gs_op_info.op_type) {
//             case READ:
//                 root = pldm_cjson_read(root, gs_op_info.collection_skip, gs_op_info.collection_top);
//                 break;
//             case UPDATE:
//                 root = pldm_cjson_update(root, bej_root);
//                 break;
//             case REPLACE:
//                 root = pldm_cjson_replace(root, bej_root);
//                 break;
//             case ACTION:
//                 root = pldm_cjson_action(root);
//                 break;
//             case HEAD:
//                 root = pldm_cjson_head(root);                   /*  not return message body information. */
//                 break;
//             default :
//                 LOG("err op type : %d", gs_op_info.op_type);
//                 goto L_EXIT;
//                 break;
//         }
//         bej_root = NULL;
//         pldm_cjson_cal_len_to_root(root, gs_op_info.op_type);
//         if (gs_op_info.op_type & (READ || UPDATE || REPLACE || HEAD)) {
//             // cm_memcpy(gs_op_buf.op_result.data, gs_op_buf.op_data.data, sizeof(bejencoding_t));
//             bejencoding_t *ptr = (bejencoding_t *)gs_op_buf.op_result.data;
//             pldm_redfish_dictionary_format_t *dict_ptr = (pldm_redfish_dictionary_format_t *)dict;
//             ptr->ver = dict_ptr->schema_version;
//             ptr->schema_class = SCHEMACLASS_MAJOR;
//             u8 *end_ptr = pldm_bej_encode(root, ((u8 *)ptr + sizeof(bejencoding_t)));
//             gs_op_buf.op_result.len = end_ptr - gs_op_buf.op_result.data;
//         }
//         if (root && gs_op_info.op_type != READ) {
//             pldm_cjson_cal_len_to_root(root, OTHER_TYPE);
//             pldm_bej_encode(root, g_resourse_bej[resourse_indenty].data);
//             g_resourse_bej[resourse_indenty].is_bej = 1;
//             // pldm_cjson_delete_node(root);
//             root = NULL;
//         }
// L_EXIT:
//         gs_op_info.dev_status = gs_op_buf.op_result.len ? OPERATION_HAVE_RESULTS : OPERATION_COMPLETED;
//         pldm_cjson_pool_reinit();
//         pldm_redfish_task_execute_event_generate(g_pldm_monitor_info.pldm_event_rbuf, (pldm_redfish_task_executed_event_data_format_t *)&(g_pldm_redfish_base_info.prev_op_identify));
//     }
}