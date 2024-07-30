from pldm_fwup_pkg_creator import *

PLDM_UD_TYPE_UNKNOW = 0
PLDM_UD_TYPE_ASCII = 1
PLDM_UD_TYPE_UTF_8 = 2
PLDM_UD_TYPE_UTF_16 = 3
PLDM_UD_TYPE_UTF_16LE = 4
PLDM_UD_TYPE_UTF_16BE = 5

def create_pldm_fwup_img_info(metadata):
    fw_img = metadata["FirmwareDeviceIdentificationArea"][0]
    components = metadata["ComponentImageInformationArea"]

    upgrade_info = bytes()
    active_img_state = BIT(2)                                                             # default is factory img state
    upgrade_info += struct.pack("I", active_img_state)

    format_string = "2B" + str(len(fw_img["ComponentImageSetVersionString"])) + "s"        # 2B34s
    upgrade_info += struct.pack(
        format_string,
        PLDM_UD_TYPE_ASCII,
        len(fw_img["ComponentImageSetVersionString"]),
        fw_img["ComponentImageSetVersionString"].encode("ascii")
    )

    for component in components:
        upgrade_info += struct.pack(
            "2HB",
            component["ComponentClassification"],
            component["ComponentIdentifier"],
            0                                                                             # not used
        )

        requested_component_activation_method = bitarray(16, endian="little")
        requested_component_activation_method.setall(0)
        supported_requested_component_activation_method = [0, 1, 2, 3, 4, 5]
        for option in component["RequestedComponentActivationMethod"]:
            if option not in supported_requested_component_activation_method:
                sys.exit(
                    "ERROR: unsupported RequestedComponent                    "
                    "    ActivationMethod entry"
                )
            requested_component_activation_method[option] = 1
        upgrade_info += struct.pack("H", ba2int(requested_component_activation_method))

        format_string = "2B" + str(len(component["ComponentVersionString"])) + "s"        # 2B19s
        upgrade_info += struct.pack(
            format_string,
            PLDM_UD_TYPE_ASCII,
            len(component["ComponentVersionString"]),
            component["ComponentVersionString"].encode("ascii")
        )

    upgrade_data_content, upgrade_data_content_size = content_align(upgrade_info, len(upgrade_info), macros.ALIGN_SIZE, 0xFF)
    file_write_content(pldm_fwup_img_info_file, upgrade_data_content)

if __name__ == "__main__":
    metadatafile = pldm_fwup_img_json_file
    with open(metadatafile) as file:
        try:
            metadata = json.load(file)
        except ValueError:
            sys.exit("ERROR: Invalid metadata JSON file")
    create_pldm_fwup_img_info(metadata)