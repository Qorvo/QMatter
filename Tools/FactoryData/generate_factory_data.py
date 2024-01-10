#!/usr/bin/env python3
#
#    Copyright (c) 2022 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import os
import os.path
import sys
import shutil
import logging
import argparse
import struct
import subprocess
import base64
from typing import Tuple, List, Union, Optional, Any, Dict
from enum import Enum, unique
from dataclasses import dataclass
import cryptography.x509
import cryptography.hazmat.primitives.serialization
from cryptography.hazmat.backends import default_backend


@dataclass
class FactoryDataGeneratorArguments:
    """helper to enforce type checking on argparse output"""
    passcode: Optional[int]
    discriminator: Optional[int]
    dac_cert: str
    dac_key: str
    dac_pubkey: str
    pai_cert: str
    certification_declaration: str
    maximum_size: int
    out_file: str
    data: List[Tuple[int, str]]
    vendor_name: str
    vendor_id: Optional[int]
    product_name: str
    product_id: Optional[int]
    serial_num: str
    manuf_date: Optional[int]
    hw_ver: Optional[int]
    hw_ver_str: str
    unique_id: Optional[bytes]
    enable_key: bytes
    write_depfile_and_exit: bool
    empty: bool
    add_dac_private_key: bool
    use_spake2p_lookuptable: bool
    print_spake2p_output: bool
    magic: str


INVALID_PASSCODES = [00000000, 11111111, 22222222, 33333333, 44444444, 55555555,
                     66666666, 77777777, 88888888, 99999999, 12345678, 87654321]

TOOLS: Dict[str, Optional[str]] = {}


def check_tools_exists():
    """ determine the path to the spake2p tool """
    TOOLS['spake2p'] = shutil.which('spake2p')


def arguments_required_together(*arguments: Any):
    """return True if all arguments are None or not None"""
    return len([arg for arg in arguments if arg is None]) in (0, len(arguments))


def validate_args(args: FactoryDataGeneratorArguments):
    """ Validate all arguments
    """

    # Validate the passcode
    if args.passcode is not None:
        if ((args.passcode < 0x0000001 and args.passcode > 0x5F5E0FE) or (args.passcode in INVALID_PASSCODES)):
            logging.error('Invalid passcode: %s', args.passcode)
            sys.exit(1)

    # Validate the discriminator
    if (args.discriminator is not None) and (args.discriminator not in range(0x0000, 0x0FFF)):
        logging.error('Invalid discriminator: %s', args.discriminator)
        sys.exit(1)

    # Validate the unique Id -
    # The unique identifier SHALL consist of a randomly-generated 128-bit or longer octet string
    if (args.unique_id is not None) and len(args.unique_id) < 16:
        logging.error(f'Unique Id needs to be 16 bytes or longer: {len(args.unique_id)} < 16')
        sys.exit(1)

    assert arguments_required_together(
        args.discriminator, args.passcode), "Specify either no discriminator+passcode, or both"

    assert arguments_required_together(
        args.dac_cert, args.pai_cert), "items marked with (1) in --help need to be specified together"

    if args.dac_cert:
        assert args.dac_key or args.dac_pubkey, "DAC private or public key needs to be provided along with the DAC certificate"

    if args.write_depfile_and_exit is not None:
        assert args.out_file is not None, "Need output file to describe in depfile"
    assert arguments_required_together(args.vendor_name, args.vendor_id,
                                       args.product_name, args.product_id, args.serial_num, args.manuf_date, args.hw_ver, args.hw_ver_str, args.unique_id,
                                       args.enable_key), "items marked with (2) in --help need to be specified together"
    if args.empty:
        assert not any((args.dac_cert, args.dac_key, args.pai_cert,
                        args.certification_declaration, args.vendor_name, args.vendor_id,
                        args.product_name, args.product_id, args.serial_num, args.manuf_date,
                        args.hw_ver, args.hw_ver_str, args.unique_id, args.enable_key, args.passcode,
                        args.discriminator)), \
            "No arguments are allowed when --empty is used"


def gen_spake2p_params(passcode: int):
    """generate spake2p parameters"""
    iter_count_max = 10000
    salt_len_max = 32

    if TOOLS['spake2p'] is None:
        logging.error('spake2p not found, please add spake2p path to PATH environment variable')
        sys.exit(1)

    cmd = [
        TOOLS['spake2p'], 'gen-verifier',
        '--iteration-count', str(iter_count_max),
        '--salt-len', str(salt_len_max),
        '--pin-code', str(passcode),
        '--out', '-',
    ]

    output = subprocess.check_output(cmd)
    output = output.decode('utf-8').splitlines()
    return dict(zip(output[0].split(','), output[1].split(',')))


def gen_raw_ec_keypair_from_der(key_file: str) -> Tuple[bytes, bytes]:
    """Load an ec keypair"""
    with open(os.path.expandvars(key_file), 'rb') as file_descriptor:
        key_data = file_descriptor.read()

    logging.warning('Leaking of DAC private keys may lead to attestation chain revocation')
    logging.warning('Please make sure the DAC private is key protected using a password')

    # WARNING: Below line assumes that the DAC private key is not protected by a password,
    #          please be careful and use the password-protected key if reusing this code
    key_der = cryptography.hazmat.primitives.serialization.load_der_private_key(key_data, None, default_backend())

    privkey_raw_bytes = key_der.private_numbers().private_value.to_bytes(32, byteorder='big')

    pubkey_raw_bytes = key_der.public_key().public_bytes(
        cryptography.hazmat.primitives.serialization.Encoding.X962,
        cryptography.hazmat.primitives.serialization.PublicFormat.UncompressedPoint)

    return (privkey_raw_bytes, pubkey_raw_bytes)


def get_raw_ec_pubkey_from_der(key_file: str) -> bytes:
    with open(os.path.expandvars(key_file), 'rb') as file_desc:
        key_data = file_desc.read()
    pubkey = cryptography.hazmat.primitives.serialization.load_der_public_key(key_data, default_backend())
    return pubkey.public_bytes(
        cryptography.hazmat.primitives.serialization.Encoding.X962,
        cryptography.hazmat.primitives.serialization.PublicFormat.UncompressedPoint
    )


@unique
class TagId(Enum):
    """Reserved tag id's for the Qorvo Matter SDK"""
    END_MARKER = 0
    CERTIFICATION_DECLARATION = 1
    TEST_DAC_CERT = 3
    PAI_CERT = 4
    TEST_DAC_PRIVATE_KEY = 5
    TEST_DAC_PUBLIC_KEY = 6
    DISCRIMINATOR = 15
    ITERATION_COUNT = 16
    SALT = 17
    VERIFIER = 18
    SETUP_PASSCODE = 19
    VENDOR_NAME = 25
    VENDOR_ID = 26
    PRODUCT_NAME = 27
    PRODUCT_ID = 28
    SERIAL_NUMBER = 29
    MANUFACTURING_DATE = 30
    HARDWARE_VERSION = 31
    HARDWARE_VERSION_STRING = 32
    ROTATING_UNIQUE_ID = 33
    ENABLE_KEY = 40

    @staticmethod
    def repr_value(tag_id: int) -> str:
        """ return name from value"""
        try:
            return TagId(tag_id).name
        except ValueError:
            return 'UNKNOWN'


class FactoryDataElement:
    """A factory data element"""
    HEADER_LEN = 8
    FORMAT = "<II"

    def __init__(self, tag_id: Union[TagId, int], subelement_data: bytes):
        if isinstance(tag_id, TagId):
            self.tag_id = tag_id.value
        else:
            self.tag_id = tag_id
        assert self.tag_id < 0xFFFFFFFF
        assert len(subelement_data) < 0xFFFFFFFF
        self.subelement_data_len = len(subelement_data)
        self.subelement_data = subelement_data

    def serialize(self):
        """Return the data in binary form"""
        byte_data = struct.pack(FactoryDataElement.FORMAT,
                                self.tag_id,
                                self.subelement_data_len)
        if self.subelement_data_len:
            byte_data += self.subelement_data
        if len(byte_data) % 4:
            # add padding: align size up to multiple of 32-bit words
            byte_data += b'\0' * (4 - (len(byte_data) % 4))

        assert len(byte_data) % 4 == 0
        return byte_data

    def __str__(self):
        return (f"type={self.tag_id:#x} ({TagId.repr_value(self.tag_id)})"
                f" length={self.subelement_data_len:#x}"
                f" value={self.subelement_data.hex()}")

    @staticmethod
    def create_uint16(tag_id: Union[TagId, int], value: int) -> 'FactoryDataElement':
        """ Load data as little endian 16bit integer """
        return FactoryDataElement(tag_id, struct.pack("<H", value))

    @staticmethod
    def create_uint32(tag_id: Union[TagId, int], value: int) -> 'FactoryDataElement':
        """ Load data as little endian 32bit integer """
        return FactoryDataElement(tag_id, struct.pack("<I", value))

    @staticmethod
    def from_file(tag_id: Union[TagId, int], file_path: str) -> 'FactoryDataElement':
        """ Load data from a file, expands environment ${variables} """
        with open(os.path.expandvars(file_path), 'rb') as file_descriptor:
            return FactoryDataElement(tag_id, file_descriptor.read(-1))

    @staticmethod
    def from_utf8_string(tag_id: Union[TagId, int], string: str) -> 'FactoryDataElement':
        """ Load data as UTF-8 encoded argument string """
        return FactoryDataElement(tag_id, string.encode("utf-8") + b'\x00')

    @staticmethod
    def from_bytes(tag_id: Union[TagId, int], value: bytes) -> 'FactoryDataElement':
        """ Load data from bytes """
        return FactoryDataElement(tag_id, value)

    @staticmethod
    def create_end_marker() -> 'FactoryDataElement':
        """ factory method to create end of TLV stream marker """
        return FactoryDataElement(TagId.END_MARKER, bytes())


class FactoryDataContainer:
    """A Factory Data Container"""

    def __init__(self, magic: bytes, maximum_size: int = 0x1000):
        self.elements: List[FactoryDataElement] = []
        self.maximum_size = maximum_size
        self.magic = magic

    def add(self, element: FactoryDataElement):
        """ add an element to the container """
        self.elements.append(element)

    def serialize(self) -> bytes:
        """ turn the contained elements into a binary file """
        assert len(self.elements) > 0, "An empty container is not allowed"

        if self.elements[-1].tag_id != TagId.END_MARKER:
            # add end marker
            self.add(FactoryDataElement.create_end_marker())

        assert len(self.magic) == 4, "Magic should be four bytes"
        factory_data = b"".join([self.magic] + [element.serialize() for element in self.elements])

        assert len(factory_data) < self.maximum_size
        return factory_data


spake2p_lookuptable = {
    20202021: {
        'Index': '0',
        'PIN Code': '20202021',
        'Iteration Count': '10000',
        'Salt': 'NU2z6q7/SsuKgg1sXe0YWCl7jmY31Xx6sUt1M75thbs=',
        'Verifier':
            'ARnsTipvGgE08mxFIcyidQch34P2WL95vLgi0zECABAEz/ON7VBv++X+764H8lQlRnufauVnn6pZcAwC9izVcPADt/tXI2rXlHay6exFWxgsQJYkjU1/NZfyVQOL9/x0fw=='
    }
}


def generate_factory_bin(args: FactoryDataGeneratorArguments) -> bytes:
    """main application"""

    if args.empty:
        return b'\0' * args.maximum_size
    container = FactoryDataContainer(args.magic, maximum_size=args.maximum_size)
    if args.passcode is not None and args.discriminator is not None:
        logging.info("Discriminator:%s Passcode:%s", args.discriminator, args.passcode)
        if args.use_spake2p_lookuptable:
            logging.warning("Using lookuptable to obtain spake2p values, use for DEVELOPMENT ONLY")
            try:
                spake2p_params = spake2p_lookuptable[args.passcode]
            except KeyError:
                logging.error("The given passcode %s is not present in the lookuptable", args.passcode)
                sys.exit(1)

        else:
            spake2p_params = gen_spake2p_params(args.passcode)
            if args.print_spake2p_output:
                print({str(args.passcode): spake2p_params})
        container.add(FactoryDataElement.create_uint32(TagId.SETUP_PASSCODE, args.passcode))
        container.add(FactoryDataElement.create_uint16(TagId.DISCRIMINATOR, args.discriminator))
        container.add(FactoryDataElement.create_uint32(TagId.ITERATION_COUNT, int(spake2p_params['Iteration Count'])))
        container.add(FactoryDataElement(TagId.SALT, base64.decodebytes(spake2p_params['Salt'].encode('ascii'))))
        container.add(FactoryDataElement(TagId.VERIFIER, base64.decodebytes(
            spake2p_params['Verifier'].encode('ascii'))))

    if args.dac_key:
        (dac_raw_privkey, dac_raw_pubkey) = gen_raw_ec_keypair_from_der(args.dac_key)
        if args.add_dac_private_key:
            logging.warning("Adding dac private key to factorydata (use this for development only!)")
            container.add(FactoryDataElement(TagId.TEST_DAC_PRIVATE_KEY, dac_raw_privkey))
        container.add(FactoryDataElement(TagId.TEST_DAC_PUBLIC_KEY, dac_raw_pubkey))
    elif args.dac_pubkey:
        container.add(FactoryDataElement(TagId.TEST_DAC_PUBLIC_KEY, get_raw_ec_pubkey_from_der(args.dac_pubkey)))
    if args.dac_cert:
        container.add(FactoryDataElement.from_file(TagId.TEST_DAC_CERT, args.dac_cert))
        container.add(FactoryDataElement.from_file(TagId.PAI_CERT, args.pai_cert))
    if args.certification_declaration:
        container.add(FactoryDataElement.from_file(TagId.CERTIFICATION_DECLARATION, args.certification_declaration))

    if args.vendor_name:
        container.add(FactoryDataElement.from_utf8_string(TagId.VENDOR_NAME, args.vendor_name))
    if args.vendor_id is not None:
        container.add(FactoryDataElement.create_uint16(TagId.VENDOR_ID, args.vendor_id))
    if args.product_name:
        container.add(FactoryDataElement.from_utf8_string(TagId.PRODUCT_NAME, args.product_name))
    if args.product_id is not None:
        container.add(FactoryDataElement.create_uint16(TagId.PRODUCT_ID, args.product_id))
    if args.serial_num:
        container.add(FactoryDataElement.from_utf8_string(TagId.SERIAL_NUMBER, args.serial_num))
    if args.manuf_date is not None:
        container.add(FactoryDataElement.create_uint32(TagId.MANUFACTURING_DATE, args.manuf_date))
    if args.hw_ver is not None:
        container.add(FactoryDataElement.create_uint16(TagId.HARDWARE_VERSION, args.hw_ver))
    if args.hw_ver_str:
        container.add(FactoryDataElement.from_utf8_string(TagId.HARDWARE_VERSION_STRING, args.hw_ver_str))
    if args.unique_id:
        container.add(FactoryDataElement.from_bytes(TagId.ROTATING_UNIQUE_ID, args.unique_id))
    if args.enable_key:
        container.add(FactoryDataElement.from_bytes(TagId.ENABLE_KEY, args.enable_key))

    if args.data:
        for (tag, value) in args.data:
            if value.startswith("@"):
                container.add(FactoryDataElement.from_file(tag, value[1:]))
            else:
                container.add(FactoryDataElement.from_bytes(tag, bytes.fromhex(value)))

    if not args.write_depfile_and_exit:
        for element in container.elements:
            logging.info("%s", element)
    return container.serialize()


class ArgumentParserWithEnvVarExpanding(argparse.ArgumentParser):
    """ Expand environment variables in 'fromfile' statements
    Example: call this program with @specific-config-settings as argument
    when specific-config-settings contains:
      @${variablewithpathprefix}/common-config-settings)
      --some-extra-argument1=a
      --some-extra-argument2=b
    Here the system environment variable $variablewithpathprefix will
    be resolved with os.path.expandvars, allowing reuse of a common
    factorydata settings file.
    """

    def _read_args_from_files(self, arg_strings):
        arg_strings = [
            os.path.expandvars(arg_str) if arg_str and arg_str.startswith(self.fromfile_prefix_chars)
            else arg_str
            for arg_str in arg_strings
        ]
        return super()._read_args_from_files(arg_strings)


def parse_command_line_arguments(cli_args: List[str]) -> FactoryDataGeneratorArguments:
    """parse command line arguments"""
    def any_base_int(string: str) -> int:
        """ convert string (with possible 0x hex indicator) to int """
        return int(string, 0)

    def hex_string(string: str) -> bytes:
        """ convert a hexadecimal string like deadbeef to bytes """
        return bytes.fromhex(string)

    def int_double_colon_string(string: str) -> Tuple[int, str]:
        """ convert 1:foo to (int(1), "foo") """
        return (int(string.split(':')[0], 0), string.split(':')[1])

    def calendar_date(string: str) -> int:
        """ convert a string date in format M/D/Y to packed structure containing 2 bytes for year, byte month, byte day """
        from datetime import datetime
        manuf_date = datetime.strptime(string, "%m/%d/%Y")
        return manuf_date.day << 24 | (manuf_date.month << 16) | manuf_date.year

    def str_to_magic(string: str) -> bytes:
        """ convert four char string to magic """
        magic = string.encode("utf-8")
        assert len(magic) == 4, "Magic should be four bytes"
        return magic

    parser = ArgumentParserWithEnvVarExpanding(description='Chip Factory NVS binary generator tool',
                                               fromfile_prefix_chars='@')

    parser.add_argument('-p', '--passcode', type=any_base_int, default=None,
                        help='The discriminator for pairing, range: 0x01-0x5F5E0FE',)
    parser.add_argument('-d', '--discriminator', type=any_base_int, default=None,
                        help='The passcode for pairing, range: 0x00-0x0FFF')
    parser.add_argument('--dac-cert', type=str,
                        help='(1) The path to the DAC certificate in der format')
    parser.add_argument('--dac-key', type=str,
                        help='(1A) The path to the DAC private key in der format (required if public key is not supplied)')
    parser.add_argument('--dac-pubkey', type=str,
                        help="(1B) The path to the DAC public key in der format (required if private key is not supplied)")
    parser.add_argument('--pai-cert', type=str,
                        help='(1) The path to the PAI certificate in der format')
    parser.add_argument('--certification-declaration', type=str,
                        help='The path to the certificate declaration der format')
    parser.add_argument('-s', '--maximum-size', type=any_base_int, required=False, default=0x6000,
                        help='The maximum size of the factory blob, default: 0x6000')
    parser.add_argument("--out_file", type=str,
                        help="Path to output file (.bin file)")
    parser.add_argument('--data', action='append', type=int_double_colon_string, default=None,
                        help="extra element to add, specify "
                        "tag_integer:@filename_with_binary_data or tag_integer:hex_string")
    parser.add_argument('--vendor-name', type=str, help='(2) Vendor name')
    parser.add_argument('--vendor-id', type=any_base_int, default=None, help='(2) Vendor ID')
    parser.add_argument('--product-name', type=str, help='(2) Product name')
    parser.add_argument('--product-id', type=any_base_int, default=None, help='(2) Product ID')
    parser.add_argument('--serial-num', type=str, help='(2) Serial number (string)')
    parser.add_argument('--manuf-date', type=calendar_date, default=None, help='Manufacturing date')
    parser.add_argument('--hw-ver', type=any_base_int, default=None, help='(2) Hardware version')
    parser.add_argument('--hw-ver-str', type=str, help='(2) Hardware version string')
    parser.add_argument('--unique-id', type=hex_string, default=None,
                        help='(2) Rotating device ID unique ID, must be a randomly-generated 128-bit (or longer) hex string')
    parser.add_argument('--enable-key', type=hex_string, default=None, help='(2) Enable key (hex_string)')
    parser.add_argument('--write-depfile-and-exit', type=str, help='Write make depfile to disk and exit')
    parser.add_argument('--empty', action='store_true', default=False, help='Write an all-zeroes file')
    parser.add_argument('--add-dac-private-key', action='store_true', default=False,
                        help='Add the DAC private key to factorydata file')
    parser.add_argument('--use-spake2p-lookuptable', action='store_true', default=False,
                        help='Use static values to avoid calling spake2p')
    parser.add_argument('--print-spake2p-output', action='store_true', default=False,
                        help='print spake2p output to copy/paste into spake2p_lookuptable')
    parser.add_argument('--magic', type=str_to_magic, default=b"QFDA",
                        help='use custom magic')

    args = parser.parse_args(cli_args)
    return FactoryDataGeneratorArguments(**vars(args))


def write_depfile(args: FactoryDataGeneratorArguments):
    """ Write a make dependency file with filenames mentioned in the conf file """
    file_names = [item for item in [args.dac_cert, args.pai_cert,
                                    args.certification_declaration] if item is not None]

    if args.data:
        file_names += [value[1:] for (tag, value) in args.data if value.startswith("@")]

    file_names = [os.path.expandvars(file_name) for file_name in file_names]
    # fromfiles (config files)
    file_names += [file_name[1:] for file_name in sys.argv if file_name.startswith("@")]

    # include all command arguments in the file comments to cover changes
    new_content = "# " + "\n# ".join(sys.argv) + "\n"
    # add input file names so make can trigger the rule if any file changes
    new_content += args.out_file + ": \\\n" + " \\\n".join(file_names)

    if os.path.isfile(args.write_depfile_and_exit):
        with open(args.write_depfile_and_exit, 'r', encoding='utf-8') as file_descriptor:
            old_content = file_descriptor.read(-1)
            if old_content == new_content:
                new_content = None

    if new_content is not None:
        with open(args.write_depfile_and_exit, 'w', encoding='utf-8') as file_descriptor:
            file_descriptor.write(new_content)


def main():
    """entry point of the program"""
    args = parse_command_line_arguments(sys.argv[1:])
    validate_args(args)
    check_tools_exists()
    if args.write_depfile_and_exit:
        write_depfile(args)
        sys.exit(0)
    with open(args.out_file, 'wb') as file_descriptor:
        file_descriptor.write(generate_factory_bin(args))


if __name__ == "__main__":
    logging.basicConfig(format='[%(asctime)s] [%(levelname)7s] - %(message)s', level=logging.INFO)
    main()
