#include "pldm_cjson.h"
#include "pldm_bej_resolve.h"
#include "pldm_monitor.h"

extern pldm_cjson_schema_fmt_t *pldm_cjson_create_schema(pldm_cjson_t *obj, pldm_cjson_schema_fmt_t *fmt);
extern void pldm_cjson_fill_comm_field_in_schema_update(pldm_cjson_t *root, u8 is_collection, u32 id, char *type, u8 resource_identify);
/*-----------------------------------------------------------update--------------------------------------------------------------------*/
/* to be determind */
static pldm_cjson_t *pldm_cjson_create_port_v1_3_1_schema_update(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 9, "", ""},
            // {0, BEJ_SET, 9, "Port", ""},
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
                            {0, BEJ_REAL, 0, "", "123Gbs"},
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
                // {0, BEJ_INT, 0, "LinkTransitionIndicator", (char [3]){0x01, 0x01, 0x00}},
                // {0, BEJ_INT, 0, "MaxFrameSize", "?"},
                // {0, BEJ_REAL, 0, "MaxSpeedGbps", (char [3]){0x01, 50, 0x00}},
                // {0, BEJ_STR, 0, "Name", "AM_Port"},
                // {0, BEJ_SET, 1, "Status", ""},
                //     {0, BEJ_ENUM, 0, "State", (char [3]){0x01, 0x01, 0x00}},
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
    pldm_cjson_fill_comm_field_in_schema_update(new_root, 0, resource_id, "Port.1_3_1.Port", PORT_IDENTIFY);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    // pldm_cjson_printf_root(new_root->child);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_portcollection_schema_update(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 3, "", ""},
            // {0, BEJ_SET, 3, "PortCollection", ""},
                {0, BEJ_STR, 0, "Name", "Ports"},
                {1, BEJ_INT, 0, "Members@odata.count", (char [2]){MAX_LAN_NUM, 0x00}},
                {0, BEJ_ARRAY, 2, "Members", ""},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I100"},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I101"}
    };
    pldm_cjson_create_schema(root, fmt);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    pldm_cjson_fill_comm_field_in_schema_update(new_root, 1, PLDM_BASE_PORTS_RESOURCE_ID, "PortCollection.PortCollection", PORT_COLLECTION);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_networkinterface_v1_2_0_schema_update(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 5, "", ""},
            // {0, BEJ_SET, 5, "NetworkInterface", ""},
                {0, BEJ_SET, 1, "Links", ""},
                    {0, BEJ_SET, 0, "NetworkAdapter", ""},
                        // {0, BEJ_STR, 0, "", (char [2]){PLDM_BASE_NETWORK_ADAPTER_RESOURCE_ID, 0x00}},
                {0, BEJ_STR, 0, "Name", "1212AM_Network_Interface"},
                {0, BEJ_SET, 0, "NetworkDeviceFunctions", "Ports"},
                // {0, BEJ_SET, 0, "NetworkPorts", ""},
                    // {0, BEJ_STR, 0, "", (char [2]){PLDM_BASE_NETWORK_DEV_FUNCS_RESOURCE_ID, 0x00}},
                {0, BEJ_SET, 0, "Ports", ""},
                    // {0, BEJ_STR, 0, "", (char [2]){PLDM_BASE_PORTS_RESOURCE_ID, 0x00}},
                // {0, BEJ_SET, 1, "Status", ""},
                //     {0, BEJ_ENUM, 0, "State", (char [3]){0x01, 0x01, 0x00}},
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
    pldm_cjson_fill_comm_field_in_schema_update(new_root, 0, PLDM_BASE_NETWORK_INTERFACE_RESOURCE_ID, "NetworkInterface.1_2_1.NetworkInterface", NETWORK_INTERFACE);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_networkadapter_v1_5_0_schema_update(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        {0, BEJ_SET, 12, "", ""},
            // {0, BEJ_SET, 12, "NetworkAdapter", ""},
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
                            {0, BEJ_INT, 0, "LanesInUse", "?????????"},
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
                    {0, BEJ_ENUM, 5, "State", (char [3]){0x01, 0x01, 0x00}},
                        // {0, BEJ_STR, 0, "StandbyOffline", ""},
                        // {0, BEJ_STR, 0, "Starting", ""},
                        // {0, BEJ_STR, 0, "Updating", ""},
                        // {0, BEJ_STR, 0, "Enabled", ""},
                        // {0, BEJ_STR, 0, "Disabled", ""},
                {0, BEJ_STR, 0, "Id", "%I1"},                   /* PLDM_BASE_NETWORK_ADAPTER_RESOURCE_ID */
                // {0, BEJ_ARRAY, 1, "ControllerLinks", ""},
                //     {1, BEJ_INT, 0, "PCIeDevices@odata.count", (char [2]){MAX_LAN_NUM, 0x00}},
    };
    pldm_cjson_create_schema(root, fmt);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    pldm_cjson_fill_comm_field_in_schema_update(new_root, 0, PLDM_BASE_NETWORK_ADAPTER_RESOURCE_ID, "NetworkAdapter.1_5_0.NetworkAdapter", NETWORK_ADAPTER);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_networkdevicefunction_v1_3_3_schema_update(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 12, "", ""},
            // {0, BEJ_SET, 12, "NetworkDeviceFunction", ""},
                {0, BEJ_ARRAY, 1, "AssignablePhysicalPorts", ""},
                    {0, BEJ_SET, 0, "", ""},
                        // {0, BEJ_STR, 0, "", (char [2]){PLDM_BASE_PORT_RESOURCE_ID, 0x00}},
                        // {0, BEJ_STR, 0, "", (char [2]){PLDM_BASE_PORT_RESOURCE_ID + 1, 0x00}},
                {1, BEJ_INT, 0, "AssignablePhysicalPorts@odata.count", (char [2]){MAX_LAN_NUM, 0x00}},
                {0, BEJ_ENUM, 0, "BootMode", (char [3]){0x01, 0x01, 0x00}},
                    // {0, BEJ_STR, 0, "Disabled", ""},
                    // {0, BEJ_STR, 0, "PXE", ""},
                    // {0, BEJ_STR, 0, "iSCSI", ""},
                    // {0, BEJ_STR, 0, "FibreChannel", ""},
                    // {0, BEJ_STR, 0, "FibreChannelOverEthernet", ""},
                {0, BEJ_BOOLEAN, 0, "DeviceEnabled", "t"},
                {0, BEJ_SET, 4, "Ethernet", ""},
                    {0, BEJ_STR, 0, "MACAddress", "11:22:33:44:55:66"},
                    // {0, BEJ_INT, 0, "MTUSize", (char [4]){0x02, 0xEE, 0x25, 0x00}},
                    {0, BEJ_SET, 0, "VLAN", ""},
                    {0, BEJ_STR, 0, "PermanentMACAddress", "11:22:33:44:55:66"},
                    {0, BEJ_SET, 0, "VLANs", ""},
                {0, BEJ_INT, 0, "MaxVirtualFunctions", ""},
                // {0, BEJ_STR, 0, "Name", "NetworkDeviceFunction Current Settings?"},
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
                // {0, BEJ_SET, 1, "Status", ""},
                //     {0, BEJ_ENUM, 0, "State", (char [3]){0x01, 0x01, 0x00}},
                        // {0, BEJ_STR, 0, "StandbyOffline", ""},
                        // {0, BEJ_STR, 0, "Starting", ""},
                        // {0, BEJ_STR, 0, "Updating", ""},
                        // {0, BEJ_STR, 0, "Enabled", ""},
                        // {0, BEJ_STR, 0, "Disabled", ""},
                {0, BEJ_BOOLEAN, 0, "VirtualFunctionsEnabled", "f"},
                {1, BEJ_SET, 1, "@Redfish.Settings", ""},
                    // {1, BEJ_SET, 0, "SettingsObject", "Points to the next setting = Resource ID +10"},
                    {1, BEJ_ARRAY, 1, "SupportedApplyTimes", ""},
                        {1, BEJ_ENUM, 0, "", (char [3]){0x01, 0x03, 0x00}},
                        // {0, BEJ_STR, 0, "AtMaintenanceWindowStart", ""},
                        // {0, BEJ_STR, 0, "Immediate", ""},
                        // {0, BEJ_STR, 0, "InMaintenanceWindowOnReset", ""},
                        // {0, BEJ_STR, 0, "OnReset", ""},
                {0, BEJ_STR, 0, "Id", "dsdfsdfResource Offset"},
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
    pldm_cjson_fill_comm_field_in_schema_update(new_root, 0, resource_id, "PortCollection.1_3_1.PortCollection", NETWORK_DEVICE_FUNC);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_networkdevicefunctioncollection_schema_update(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 3, "", ""},
            // {0, BEJ_SET, 3, "NetworkDeviceFunctionCollection", ""},
                {0, BEJ_STR, 0, "Name", "NetworkDeviceFunctions"},
                {1, BEJ_INT, 0, "Members@odata.count", (char [2]){MAX_LAN_NUM, 0x00}},
                {0, BEJ_ARRAY, 2, "Members", ""},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I200"},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I201"}
    };
    pldm_cjson_create_schema(root, fmt);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    pldm_cjson_fill_comm_field_in_schema_update(new_root, 1, PLDM_BASE_NETWORK_DEV_FUNCS_RESOURCE_ID, "NetworkDeviceFunctionCollection.NetworkDeviceFunctionCollection", NETWORK_DEVICE_FUNC_COLLECTION);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_pciedevice_v1_4_0_schema_update(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 12, "", ""},
            // {0, BEJ_SET, 12, "PCIeDevice", ""},
                // {0, BEJ_STR, 0, "AssetTag", ""},
                {0, BEJ_ENUM, 0, "DeviceType", (char [3]){0x01, 0x01, 0x00}},  /* Config space of function 0 - Header type register */
                    // {0, BEJ_STR, 0, "SingleFunction", ""},
                    // {0, BEJ_STR, 0, "MultiFunction", ""},
                    // {0, BEJ_STR, 0, "Simulated", ""},
                {0, BEJ_STR, 0, "FirmwareVersion", "1.1.1?"},
                {0, BEJ_STR, 0, "Manufacturer", "dsfsdfsdWXKJ"},
                {0, BEJ_STR, 0, "Name", "AMBER"},
                {0, BEJ_SET, 3, "PCIeInterface", ""},
                    {0, BEJ_INT, 0, "LanesInUse", ""},                         /* Negotiated Link Width */
                    // {0, BEJ_INT, 0, "MaxLanes", ""},                           /* Maximum Link Width */
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
                {0, BEJ_STR, 0, "Model", "AMBER"},
                {0, BEJ_STR, 0, "PartNumber", ""},
                {0, BEJ_STR, 0, "SKU", "hfjkhdjhjsjd"},
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
    pldm_cjson_fill_comm_field_in_schema_update(new_root, 0, PLDM_BASE_PCIE_FUNC_RESOURCE_ID, "PCIeDevice.1_4_0.PCIeDevice", PCIE_DEVICE);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_pciefunctioncollection_schema_update(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 3, "", ""},
            // {0, BEJ_SET, 3, "PCIeDeviceCollection", ""},
                {0, BEJ_STR, 0, "Name", "PCIeFunctions"},
                {1, BEJ_INT, 0, "Members@odata.count", (char [2]){MAX_LAN_NUM, 0x00}},
                {0, BEJ_ARRAY, 2, "Members", ""},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I300"},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I301"}
    };
    pldm_cjson_create_schema(root, fmt);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    pldm_cjson_fill_comm_field_in_schema_update(new_root, 1, PLDM_BASE_PCIE_FUNCS_RESOURCE_ID, "PCIeFunctionCollection.PCIeFunctionCollection", PCIE_FUNC_COLLECTION);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_pciefunction_v1_2_3_schema_update(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 9, "", ""},
            // {0, BEJ_SET, 9, "PCIeFunction", ""},
                {0, BEJ_STR, 0, "ClassCode", "0x020000"},                  /* EthernetController */
                {0, BEJ_ENUM, 0, "DeviceClass", (char [3]){0x01, 'U', 0x00}},
                    // {0, BEJ_STR, 0, "NetworkController", ""},
                {0, BEJ_STR, 0, "DeviceId", ""},
                {0, BEJ_ENUM, 0, "FunctionType", (char [3]){0x01, 'U', 0x00}},
                    // {0, BEJ_STR, 0, "Physical", ""},
                    // {0, BEJ_STR, 0, "Virtual", ""},
                {0, BEJ_STR, 0, "Name", "AMBER"},
                {0, BEJ_STR, 0, "RevisionId", "fsdfsdf"},                         /* GLPCI_DREVID XOR GLPCI_REVID */
                {0, BEJ_SET, 1, "Status", ""},
                    {0, BEJ_ENUM, 0, "State", (char [3]){0x01, 0x01, 0x00}},
                        // {0, BEJ_STR, 0, "StandbyOffline", ""},
                        // {0, BEJ_STR, 0, "Starting", ""},
                        // {0, BEJ_STR, 0, "Updating", ""},
                        // {0, BEJ_STR, 0, "Enabled", ""},
                        // {0, BEJ_STR, 0, "Disabled", ""},
                // {0, BEJ_STR, 0, "SubsystemId", ""},                        /* PFPCI_SUBSYSID.PF_SUBSYS_ID */
                // {0, BEJ_STR, 0, "SubsystemVendorId", ""},                  /* GLPCI_SUBVENID */
                {0, BEJ_STR, 0, "VendorId", ""},                           /* GLPCI_VENDORID.VENDOR_D */
                {0, BEJ_STR, 0, "Id", ""},                                 /* Resource Offset */
    };
    pldm_cjson_create_schema(root, fmt);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    pldm_cjson_fill_comm_field_in_schema_update(new_root, 0, resource_id, "PCIeFunction.1_2_3.PCIeFunction", PCI_FUNC);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_ethernetinterface_v1_5_1_schema_update(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 10, "", ""},
            // {0, BEJ_SET, 10, "EthernetInterface", ""},
                {0, BEJ_BOOLEAN, 0, "InterfaceEnabled", ""},               /* PRTGEN_STATUS.PORT_VALID */
                {0, BEJ_BOOLEAN, 0, "FullDuplex", "t"},
                {0, BEJ_ENUM, 0, "LinkStatus", ""},
                    // {0, BEJ_STR, 0, "LinkDown", ""},
                    // {0, BEJ_STR, 0, "LinkUp", ""},
                    // {0, BEJ_STR, 0, "NoLink", ""},
                {0, BEJ_STR, 0, "MACAddress", "11:22:33:44:55:66"},
                {0, BEJ_INT, 0, "MTUSize", (char [3]){0xEE, 0x25, 0x00}},
                // {0, BEJ_STR, 0, "Name", "AM Ethernet Interface Current Settings"},
                {0, BEJ_ARRAY, 0, "NameServers", ""},
                {0, BEJ_STR, 0, "PermanentMACAddress", "ddddddddd"},
                // {0, BEJ_INT, 0, "SpeedMbps", ""},
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
    pldm_cjson_fill_comm_field_in_schema_update(new_root, 0, resource_id, "EthernetInterface.1_5_1.EthernetInterface", ETH_INTERFACE);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

/* to be determind */
static pldm_cjson_t *pldm_cjson_create_ethernetinterfacecollection_schema_update(u32 resource_id)
{
    pldm_cjson_t *root = pldm_cjson_create_obj();
    if (!root) return NULL;
    pldm_cjson_schema_fmt_t fmt[] = {
        /* schema_type | fmt | child_cnt | name | val */
        {0, BEJ_SET, 3, "", ""},
            // {0, BEJ_SET, 3, "EthernetInterfaceCollection", ""},
                {0, BEJ_STR, 0, "Name", "NetworkDeviceFunctions"},
                {1, BEJ_INT, 0, "Members@odata.count", (char [2]){MAX_LAN_NUM, 0x00}},
                {0, BEJ_ARRAY, 2, "Members", ""},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I400"},
                    {0, BEJ_SET, 1, "", ""},
                        {1, BEJ_STR, 0, "@odata.id", "%I401"}
    };
    pldm_cjson_create_schema(root, fmt);
    pldm_cjson_t *new_root = NULL;
    new_root = root->child;
    root->child = NULL;
    // pldm_cjson_delete_node(root);
    root = NULL;
    pldm_cjson_fill_comm_field_in_schema_update(new_root, 1, PLDM_BASE_ETH_INTERFACE_COLLECTION_RESOURCE_ID, "EthernetInterfaceCollection.EthernetInterfaceCollection", ETH_INTERFACE_COLLECTION);
    // pldm_cjson_cal_sf_to_root(new_root->child, anno_dict, dict);
    return new_root->child;
}

schema_create g_schemas_update[11] = {
    pldm_cjson_create_networkadapter_v1_5_0_schema_update,
    pldm_cjson_create_pciedevice_v1_4_0_schema_update,
    pldm_cjson_create_networkinterface_v1_2_0_schema_update,
    pldm_cjson_create_portcollection_schema_update,
    pldm_cjson_create_pciefunctioncollection_schema_update,
    pldm_cjson_create_networkdevicefunctioncollection_schema_update,
    pldm_cjson_create_networkdevicefunction_v1_3_3_schema_update,
    pldm_cjson_create_port_v1_3_1_schema_update,
    pldm_cjson_create_pciefunction_v1_2_3_schema_update,
    pldm_cjson_create_ethernetinterface_v1_5_1_schema_update,
    pldm_cjson_create_ethernetinterfacecollection_schema_update
};