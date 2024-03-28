from pldm_fwup_pkg_creator import *

def create_pldm_fwup_img_info(metadata):
    components = metadata["ComponentImageInformationArea"]
    num_components = len(components)
    upgrade_data = bytes()
    upgrade_data += struct.pack("H", num_components)
    for component in components:
        upgrade_data +=struct.pack("H", component["ComponentClassification"])
        upgrade_data +=struct.pack("H", component["ComponentIdentifier"])
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
        upgrade_data +=struct.pack("H", ba2int(requested_component_activation_method))
        upgrade_data +=struct.pack("18s", component["ComponentVersionString"].encode("ascii"))
    upgrade_data_len = struct.pack("H", len(upgrade_data) + 4)
    upgrade_info = bytes()
    upgrade_info += upgrade_data_len + upgrade_data
    upgrade_info += binascii.crc32(upgrade_info).to_bytes(4, "little")
    upgrade_info_content, upgrade_info_content_size = content_align(upgrade_info, len(upgrade_info), macros.ALIGN_SIZE, 0xFF)
    file_write_content(pldm_fwup_img_info_file, upgrade_info_content)

if __name__ == "__main__":
    metadatafile = pldm_fwup_img_json_file
    with open(metadatafile) as file:
        try:
            metadata = json.load(file)
        except ValueError:
            sys.exit("ERROR: Invalid metadata JSON file")
    create_pldm_fwup_img_info(metadata)