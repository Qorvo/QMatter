#!/usr/bin/python
"""
Helper classes to read, modify and generate OTA files
"""

import os
import argparse
import struct


class Header():
    MIN_LEN = 56
    FORMAT = "<IHHHHHIH32sI"
    FILE_ID = 0x0BEEF11E
    MANUFACTURER_CODE = 0x10D0
    HEADER_VERSION = 0x0100  # Major[1B MSB], Minor [1B LSB]
    ZIGBEE_VERSION = 0x0002  # Zigbee Pro

    FIELD_ID_SEC_CRED_BIT = 0
    FIELD_ID_DEVICE_SPECIFIC_BIT = 1
    FIELD_ID_HW_VERSION_BIT = 2

    def is_security_credential_version_present(self): return (
        self.field_control & (1 << Header.FIELD_ID_SEC_CRED_BIT)) != 0

    def is_device_specific_file_present(self): return (
        self.field_control & (1 << Header.FIELD_ID_DEVICE_SPECIFIC_BIT)) != 0

    def is_hardware_version_present(self): return (self.field_control & (1 << Header.FIELD_ID_HW_VERSION_BIT)) != 0

    def __init__(self, header_string=b"Qorvo OTA image", file_version=0x09090909, image_type=0x0000):
        """
            header_string [32B]
            file_version [4B]
            image_type [2B]
        """
        self.file_identifier = Header.FILE_ID
        self.header_version = Header.HEADER_VERSION
        self.header_length = Header.MIN_LEN
        self.field_control = 0x0000
        self.manufacturer_code = Header.MANUFACTURER_CODE
        self.image_type = image_type & 0xFFFF
        self.file_version = file_version & 0xFFFFFFFF
        self.zigbee_stack_version = Header.ZIGBEE_VERSION
        self.header_string = header_string[:32]
        self.ota_image_size = 0x00000000              # To be filled in later

    def from_file(self, filedescriptor):
        """
            Read from an active filedescriptor to fill in the class
        """
        raw_bytes = filedescriptor.read(Header.MIN_LEN)

        (self.file_identifier,
         self.header_version,
         self.header_length,
         self.field_control,
         self.manufacturer_code,
         self.image_type,
         self.file_version,
         self.zigbee_stack_version,
         self.header_string,
         self.ota_image_size) = struct.unpack(Header.FORMAT, raw_bytes)

        if self.is_security_credential_version_present():
            raw_bytes = filedescriptor.read(1)
            (self.security_credential_version,) = struct.unpack("<B", raw_bytes)

        if self.is_device_specific_file_present():
            raw_bytes = filedescriptor.read(8)
            (self.ieee_address,) = struct.unpack("<8c", raw_bytes)

        if self.is_hardware_version_present():
            raw_bytes = filedescriptor.read(2)
            (self.min_hw_version,) = struct.unpack("<H", raw_bytes)
            raw_bytes = filedescriptor.read(2)
            (self.max_hw_version,) = struct.unpack("<H", raw_bytes)

    def set_security_credential_version(self, version):
        """
            version [1B]
        """
        self.security_credential_version = version
        self.field_control |= (1 << Header.FIELD_ID_SEC_CRED_BIT)
        self.header_length += 1

    def set_hardware_version(self, min_hw_version, max_hw_version):
        """
            min_hw_version [2B]
            max_hw_version [2B]
        """
        self.min_hw_version = min_hw_version
        self.max_hw_version = max_hw_version
        self.field_control |= (1 << Header.FIELD_ID_HW_VERSION_BIT)
        self.header_length += 4

    def set_device_specific(self, ieee_address):
        """
            ieee_address [8B] (MAC address)
        """
        self.ieee_address = ieee_address
        self.field_control |= (1 << Header.FIELD_ID_DEVICE_SPECIFIC_BIT)
        self.header_length += 8

    def set_total_ota_file_size(self, size):
        """
            size [4B]
        """
        self.ota_image_size = size

    def serialize(self):
        byte_data = struct.pack(Header.FORMAT,
                                self.file_identifier,
                                self.header_version,
                                self.header_length,
                                self.field_control,
                                self.manufacturer_code,
                                self.image_type,
                                self.file_version,
                                self.zigbee_stack_version,
                                self.header_string,
                                self.ota_image_size)

        if self.is_security_credential_version_present():
            byte_data += struct.pack("<B", self.security_credential_version)

        if self.is_device_specific_file_present():
            byte_data += struct.pack("<8c", self.ieee_address)

        if self.is_hardware_version_present():
            byte_data += struct.pack("<H", self.min_hw_version)
            byte_data += struct.pack("<H", self.max_hw_version)

        return byte_data

    def __str__(self):
        return (f"file version:           0x{self.file_version:08X}{os.linesep}"
                f"file version:           0x{self.file_version:08X}{os.linesep}"
                f"field control:          0x{self.field_control:02X}-"
                f" sec:{self.is_security_credential_version_present():r}"
                f" specific:{self.is_device_specific_file_present():r}"
                f" hw:{self.is_hardware_version_present():r}{os.linesep}"
                f"image type:             0x{self.image_type:04X}{os.linesep}"
                f"header string:          \"{self.header_string.decode('ascii')}\"{os.linesep}"
                f"total size:             0x{self.ota_image_size:08X}")


class Subelement():
    HEADER_LEN = 6
    FORMAT = "<HI"
    TAG_ID_MAIN_IMAGE = 0x0000
    TAG_ID_SIGNATURE = 0x0001
    TAG_ID_SIGNER_CERTIFICATE = 0x0002
    TAG_ID_IMAGE_INTEGRITY = 0x0003
    TAG_ID_JUMPTABLE_IMAGE = 0xF000

    CRC_DATA_LEN = 16

    def __init__(self, tag_id, subelement_data):
        """
            tag_id [2B]
            subelement_data
        """
        self.tag_id = tag_id & 0xFFFF
        self.subelement_data_len = len(subelement_data) & 0xFFFFFFFF
        self.subelement_data = subelement_data

    def from_file(self, filedescriptor):
        raw_bytes = filedescriptor.read(Subelement.HEADER_LEN)

        (self.tag_id,
         self.subelement_data_len) = struct.unpack(Subelement.FORMAT, raw_bytes)

        self.subelement_data = filedescriptor.read(self.subelement_data_len)

    def serialize(self):
        byte_data = struct.pack(Subelement.FORMAT,
                                self.tag_id,
                                self.subelement_data_len)
        if self.subelement_data_len:
            byte_data += self.subelement_data

        return byte_data

    def __str__(self):
        return (f"tag id:       0x{self.tag_id:04X}{os.linesep}"
                f"data length:  0x{self.subelement_data_len:08X}{os.linesep}"
                f"data [{', '.join([hex(d) for d in self.subelement_data[0:min(len(self.subelement_data), 4)]])}]")


def parse_command_line_arguments():
    """Parse command-line arguments"""

    def parse_integer_string(string):
        return int(string, 0)

    parser = argparse.ArgumentParser(description="Analyze an OTA file")
    parser.add_argument("--ota_file",
                        help="ota file to analyze",
                        required=True)
    return parser.parse_args()


def main():
    """
    main is the entry point of the application.
    """
    arguments = parse_command_line_arguments()

    ota_file_header = Header()
    ota_file_subelement = Subelement()

    ota_file_header.set_hardware_version(0x0000, 0x0101)
    ota_file_header.set_total_ota_file_size(0x00037237)
    print("default header:")
    print(ota_file_header)
    print("")

    with open(arguments.ota_file, 'rb') as fd:

        ota_file_header.from_file(fd)
        print("header file decoded:")
        print(ota_file_header)
        print("")

        buffer = ota_file_header.serialize()
        print("header serialized:")
        print(buffer)
        print("")

        while(fd.tell() != ota_file_header.ota_image_size):

            ota_file_subelement.from_file(fd)
            print("subelement decoded:")
            print(ota_file_subelement)
            print("")

            buffer = ota_file_subelement.serialize()
            print("subelement serialized:")
            print(buffer[0:100])
            print("")


if __name__ == "__main__":
    main()
