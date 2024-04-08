import os
import sys
import struct
import binascii

parent_path = os.path.dirname(sys.path[0])
if parent_path not in sys.path:
    sys.path.append(parent_path)

from scripts_cfg import *

class macros:

    DICT_PATH = pldm_redfish_output_path + "pldm_redfish_dicts.bin"

    PLDM_BASE_NETWORK_ADAPTER_RESOURCE_ID = 1
    PLDM_BASE_NETWORK_INTERFACE_RESOURCE_ID = 5
    PLDM_BASE_PORTS_RESOURCE_ID = 10
    PLDM_BASE_NETWORK_DEV_FUNCS_RESOURCE_ID = 20
    PLDM_BASE_PORT_RESOURCE_ID = 100
    PLDM_BASE_NETWORK_DEV_FUNC_RESOURCE_ID = 200
    PLDM_BASE_NETWORK_DEV_FUNC_RESOURCE_ID_1 = 210
    PLDM_BASE_PCIE_DEV_RESOURCE_ID = 3
    PLDM_BASE_PCIE_FUNCS_RESOURCE_ID = 30
    PLDM_BASE_PCIE_FUNC_RESOURCE_ID = 300
    PLDM_BASE_ETH_INTERFACE_COLLECTION_RESOURCE_ID = 40
    PLDM_BASE_ETH_INTERFACE_RESOURCE_ID = 400
    PLDM_BASE_ETH_INTERFACE_RESOURCE_ID_1 = 410
    PLDM_BASE_RESET_SET2DEFAULE_RESOURCE_ID = 2001
    PLDM_BASE_PORT_RESET_RESOURCE_ID = 2002
    PLDM_BASE_ANNOTATION_DICT_RESOURCE_ID = 0xFFFFFFFF
    PLDM_BASE_EVENT_DICT_RESOURCE_ID = 0xFFFFFFFF
    PLDM_BASE_REGISTER_DICT_RESOURCE_ID = 0xFFFFFFFF

    MAJOR = 0
    EVENT = 1
    ANNOTATION = 2
    COLLECTION_MEMBER_TYPE = 3
    ERROR = 4
    REGISTRY = 5

    DICTS_ALIGNED_SIZE = 1024
    SINGLE_DICT_ALIGNED_SIZE = 4


resource_id = [
        macros.PLDM_BASE_ANNOTATION_DICT_RESOURCE_ID,
        macros.PLDM_BASE_ETH_INTERFACE_COLLECTION_RESOURCE_ID,
        macros.PLDM_BASE_ETH_INTERFACE_RESOURCE_ID,
        macros.PLDM_BASE_EVENT_DICT_RESOURCE_ID,
        macros.PLDM_BASE_REGISTER_DICT_RESOURCE_ID,
        macros.PLDM_BASE_NETWORK_ADAPTER_RESOURCE_ID,
        macros.PLDM_BASE_NETWORK_DEV_FUNCS_RESOURCE_ID,
        macros.PLDM_BASE_NETWORK_DEV_FUNC_RESOURCE_ID,
        macros.PLDM_BASE_NETWORK_INTERFACE_RESOURCE_ID,
        macros.PLDM_BASE_PCIE_FUNCS_RESOURCE_ID,
        macros.PLDM_BASE_PCIE_DEV_RESOURCE_ID,
        macros.PLDM_BASE_PCIE_FUNC_RESOURCE_ID,
        macros.PLDM_BASE_PORTS_RESOURCE_ID,
        macros.PLDM_BASE_PORT_RESOURCE_ID]

schema_class = [
    BIT(macros.ANNOTATION),
    BIT(macros.MAJOR) | BIT(macros.COLLECTION_MEMBER_TYPE),
    BIT(macros.MAJOR),
    BIT(macros.EVENT),
    BIT(macros.REGISTRY),
    BIT(macros.MAJOR),
    BIT(macros.MAJOR) | BIT(macros.COLLECTION_MEMBER_TYPE),
    BIT(macros.MAJOR),
    BIT(macros.MAJOR),
    BIT(macros.MAJOR) | BIT(macros.COLLECTION_MEMBER_TYPE),
    BIT(macros.MAJOR),
    BIT(macros.MAJOR),
    BIT(macros.MAJOR) | BIT(macros.COLLECTION_MEMBER_TYPE),
    BIT(macros.MAJOR)]

# class param_t:
#     def __init__(self):
#         self.ANNO_DICT_NAME = "annotation.bin"
#         self.ETHERNETINTERFACE_V1_DICT_NAME = "EthernetInterface_v1.bin"
#         self.ETHERNETINTERFACECOLLECTION_V1_DICT_NAME = "EthernetInterfaceCollection_v1.bin"
#         self.EVENT_V1_DICT_NAME = "Event_v1.bin"
#         self.MESSAGEREGISTRY_V1_DICT_NAME = "MessageRegistry_v1.bin"
#         self.NETWORKADAPTER_V1_DICT_NAME = "NetworkAdapter_v1.bin"
#         self.NETWORKDEVICEFUNCTION_V1_DICT_NAME = "NetworkDeviceFunction_v1.bin"
#         self.NETWORKDEVICEFUNCTIONCOLLECTION_V1_DICT_NAME = "NetworkDeviceFunctionCollection_v1.bin"
#         self.NETWORKINTERFACE_V1_DICT_NAME = "NetworkInterface_v1.bin"
#         self.PCIEDEVICE_V1_DICT_NAME = "PCIeDevice_v1.bin"
#         self.PCIEDEVICECOLLECTION_V1_DICT_NAME = "PCIeDeviceCollection_v1.bin"
#         self.PCIEFUNCTION_V1_DICT_NAME = "PCIeFunction_v1.bin"
#         self.PORT_V1_DICT_NAME = "Port_v1.bin"
#         self.PORTCOLLECTION_V1_DICT_NAME = "PortCollection_v1.bin"

class dict_fmt_t:
    def __init__(self):
        self.LEN = 0

class dict_hdr_t:
    def __init__(self):
        self.TOTAL_LEN = 0
        self.NUM_OF_DICT = 0

def get_file_name(file_name):
    return cur_file_dir() + r"/dicts/" + file_name

def find_files_with_suffix(suffix):
    # 使用os模块获取文件夹中所有文件的路径
    all_files = os.listdir(cur_file_dir() + r"/dicts/")

    # 筛选以指定后缀名结尾的文件
    filtered_files = [file for file in all_files if file.endswith(suffix)]
    return filtered_files

if __name__ == '__main__':
    files = find_files_with_suffix(".bin")

    offset = []
    contents = bytes()

    for file in files:
        offset.append(len(contents))
        name = get_file_name(file)
        data = file_read_content(name)
        dict_fmt = dict_fmt_t()
        dict_fmt.LEN = len(data) + 6
        dict_sign = binascii.crc32(data)
        dict_fmt_hdr = struct.pack("H", dict_fmt.LEN)
        dict_sector = dict_fmt_hdr + dict_sign.to_bytes(4, "little") + data
        content, size = content_align(dict_sector, len(dict_sector), macros.SINGLE_DICT_ALIGNED_SIZE, 0xFF)
        contents += content

    num_of_dict = len(files)
    dict_hdr_len = num_of_dict * 4 + num_of_dict * 2 + num_of_dict * 2 + 2 + 2      #resource id(u32) schema_class(u16) offset(u16) total_len(u16) num_of_dict(u16)

    dict_hdr = dict_hdr_t()
    dict_hdr.TOTAL_LEN = len(contents) + dict_hdr_len
    dict_hdr.NUM_OF_DICT = num_of_dict

    dict = struct.pack("2H", 
        dict_hdr.TOTAL_LEN,
        dict_hdr.NUM_OF_DICT)
    for i in range(len(files)):
        dict += struct.pack("I", resource_id[i])
        dict += struct.pack("H", schema_class[i])
        dict += struct.pack("H", (dict_hdr_len + offset[i]))
    dict += contents

    final_content, final_size = content_align(dict, len(dict), macros.DICTS_ALIGNED_SIZE, 0xFF)

    file_write_content(macros.DICT_PATH, final_content)

    # print(len(final_content))