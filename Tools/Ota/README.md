# OTA Firmware Upgrade Image/File Creation Tooling

This document describes the process of creating an image file for the over-the-air firmware upgrade process.
Such files can be used to perform a device firmware upgrade as described in our [Matter Over-The-Air Device firmware upgrade Guide](../../Documents/Guides/ota_device_firmware_upgrade.md).

# OTA Image file creation process

When calling `generate_ota_img.py`, you need to specify a .hex file produced from the firmware .elf file.
First, it will run `signFirmware.py`, which will modify the hex file in-place.
This will modify the metadata in the userlicense section, allowing the hex file to be used as secure image, originating from the factory:

- The image-loaded-by-bootloader flag will be set.
- A cryptographic signature will be added for authentication by the bootloader
- The public key of the `.pem` file will be written to the native userlicense for use by the bootloader.

After this, `generate_ota_img.py` will call `compressFirmware.py`:
- It will compress the firmware sections that contain upgradable data (code, data) using the Lempel–Ziv–Markov chain(LZMA) compression algorithm, after applying padding up to a page size multiple.
- A signature will be created and inserted into the compressed User License, this will enable the bootloader to perform an integrity check before inflating the payload.

As a last step, [Matter's ota\_image\_tool.py](../../Components/ThirdParty/Matter/repo/src/app/ota_image_tool.py) will be called to wrap the OTA payload
with the Matter-specific OTA headers.

# Command line use

	usage: generate_ota_img.py [-h] [--chip_config_header CHIP_CONFIG_HEADER] [--chip_root CHIP_ROOT] [--in_file IN_FILE] [--out_file OUT_FILE] [-vn VERSION] [-vs VERSION_STR] [-vid VENDOR_ID] [-pid PRODUCT_ID] [--sign]
							   [--pem_file_path PEM_FILE_PATH] [--pem_password PEM_PASSWORD] [--flash_app_start_offset FLASH_APP_START_OFFSET] [--compression {none,lzma}] [--prune_only]

	Turn a Matter application build hex-file into a bootable image and generate an ota image

	options:
	  -h, --help            show this help message and exit
	  --chip_config_header CHIP_CONFIG_HEADER
							path to Matter config header file
	  --chip_root CHIP_ROOT
							Path to root Matter directory
	  --in_file IN_FILE     Path to input file to format to Matter OTA fileformat
	  --out_file OUT_FILE   Path to output file (.ota file)
	  -vn VERSION, --version VERSION
							Software version (numeric)
	  -vs VERSION_STR, --version-str VERSION_STR
							Software version (string)
	  -vid VENDOR_ID, --vendor-id VENDOR_ID
							Vendor ID (string)
	  -pid PRODUCT_ID, --product-id PRODUCT_ID
							Product ID (string)
	  --sign                sign firmware
	  --pem_file_path PEM_FILE_PATH
							PEM file path (string)
	  --pem_password PEM_PASSWORD
							PEM file password (string)
	  --flash_app_start_offset FLASH_APP_START_OFFSET
							Offset of the application in program flash
	  --compression {none,lzma}
							compression type (default to none)
	  --prune_only          prune unneeded sections; don't add an upgrade user license (external storage scenario)


# Integration in our QMatter build flow

When using QMatter to build, the `generate_ota_img.py` script is called when the Matter&trade; applications Makefile invokes its postbuild script (eg [light_qpg6105_postbuild.sh](../../Applications/Matter/light/light_qpg6105_postbuild.sh))

An example invocation of the tool:

    "$PYTHON" "${SCRIPT_DIR}"/../../../Tools/Ota/generate_ota_img.py
    --chip_config_header "${SCRIPT_DIR}"/../../../Applications/Matter/base/include/CHIPProjectConfig.h
    --chip_root "${SCRIPT_DIR}"/../../../Components/ThirdParty/Matter/repo
    --compression lzma
    --in_file "${SCRIPT_DIR}"/../../../Work/base_qpg6105/base_qpg6105.hex
    --out_file "${SCRIPT_DIR}"/../../../Work/base_qpg6105/base_qpg6105.ota
    --pem_file_path "${SCRIPT_DIR}"/../../../Tools/Ota/example_private_key.pem.example --pem_password test1234 --sign
