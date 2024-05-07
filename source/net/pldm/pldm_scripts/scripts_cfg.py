from my_module.my_module import *

AMBER = 0
AMLITE = 0

if AMBER:
    pldm_redfish_output_path = cur_file_dir() + "/../../../../../../build/"

    pldm_fwup_jsonfile_path = cur_file_dir()
    pldm_fwup_output_path = "./"
    pldm_fwup_img_path = "./"
    pldm_fwup_img_json_file     = pldm_fwup_jsonfile_path + "/amber_pldm_fwup.json"
    pldm_fwup_image_files = {
        0x0000: [pldm_fwup_img_path + "upgrade_slot.img",    0x01],
        0x0001: [pldm_fwup_img_path + "upgrade_chip.img",    0x02],
        0x0002: [pldm_fwup_img_path + "upgrade_factory.img", 0x04]
    }
    pldm_fwup_output_files = {
        0x0000: [pldm_fwup_output_path + "upgrade_pldm_fwup_slot.img",    0x01],
        0x0001: [pldm_fwup_output_path + "upgrade_pldm_fwup_chip.img",    0x02],
        0x0002: [pldm_fwup_output_path + "upgrade_pldm_fwup_factory.img", 0x04]
    }
    pldm_fwup_img_info_file = cur_file_dir() + "/../../../../../../build/" + "pldm_fwup_img_info.bin"

elif AMLITE:
    pldm_redfish_output_path = cur_file_dir() + "/../../../../../../build/"

    pldm_fwup_jsonfile_path = cur_file_dir()
    pldm_fwup_output_path = "./"
    pldm_fwup_img_path = "./build/"
    pldm_fwup_img_json_file = pldm_fwup_jsonfile_path + "/amlite_pldm_fwup.json"
    pldm_fwup_img_info_file = cur_file_dir() + "/../../../../../../build/" + "pldm_fwup_img_info.bin"
    pldm_fwup_image_files = {
        0x0000: [pldm_fwup_img_path + "fw_aml.bin",    0x01]
    }
    pldm_fwup_output_files = {
        0x0000: [pldm_fwup_output_path + "upgrade_pldm_fwup_fw_aml.bin",    0x01]
    }