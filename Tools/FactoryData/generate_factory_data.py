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
import sys
import shutil
import logging
import argparse
import struct
import subprocess
import cryptography.x509
from typing import Tuple, List, Union
from enum import Enum, unique
from cryptography.hazmat.backends import default_backend
from dataclasses import dataclass


@dataclass
class FactoryDataGeneratorArguments:
    """helper to enforce type checking on argparse output"""
    passcode: int
    discriminator: int
    dac_cert: str
    dac_key: str
    pai_cert: str
    certification_declaration: str
    maximum_size: int
    out_file: str
    data: List[Tuple[int, str]]


INVALID_PASSCODES = [00000000, 11111111, 22222222, 33333333, 44444444, 55555555,
                     66666666, 77777777, 88888888, 99999999, 12345678, 87654321]

TOOLS = {}


def check_tools_exists():
    TOOLS['spake2p'] = shutil.which('spake2p')
    if TOOLS['spake2p'] is None:
        logging.error('spake2p not found, please add spake2p path to PATH environment variable')
        sys.exit(1)


def arguments_required_together(*arguments):
    """return True if all arguments are None or not None"""
    return len([arg for arg in arguments if arg is None]) in (0, len(arguments))


def validate_args(args: FactoryDataGeneratorArguments):
    # Validate the passcode
    if args.passcode is not None:
        if ((args.passcode < 0x0000001 and args.passcode > 0x5F5E0FE) or (args.passcode in INVALID_PASSCODES)):
            logging.error('Invalid passcode:' + str(args.passcode))
            sys.exit(1)

    # Validate the discriminator
    if (args.discriminator is not None) and (args.discriminator not in range(0x0000, 0x0FFF)):
        logging.error('Invalid discriminator:' + str(args.discriminator))
        sys.exit(1)

    assert arguments_required_together(
        args.discriminator, args.passcode), "Specify either no discriminator+passcode, or both"
    assert arguments_required_together(args.dac_cert, args.dac_key, args.pai_cert,
                                       args.certification_declaration), "Specify either no certificate configuration or a complete configuration"


def gen_spake2p_params(passcode):
    """generate spake2p parameters"""
    iter_count_max = 10000
    salt_len_max = 32

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
    with open(key_file, 'rb') as f:
        key_data = f.read()

    logging.warning('Leaking of DAC private keys may lead to attestation chain revokation')
    logging.warning('Please make sure the DAC private is key protected using a password')

    # WARNING: Below line assumes that the DAC private key is not protected by a password,
    #          please be careful and use the password-protected key if reusing this code
    key_der = cryptography.hazmat.primitives.serialization.load_der_private_key(key_data, None, default_backend())

    private_number_val = key_der.private_numbers().private_value
    privkey_raw_bytes = private_number_val.to_bytes(32, byteorder='big')

    public_key_first_byte = 0x04
    public_number_x = key_der.public_key().public_numbers().x
    public_number_y = key_der.public_key().public_numbers().y
    pubkey_raw_bytes = b"".join([public_key_first_byte.to_bytes(1, byteorder='big'),
                                 public_number_x.to_bytes(32, byteorder='big'),
                                 public_number_y.to_bytes(32, byteorder='big')])
    return (privkey_raw_bytes, pubkey_raw_bytes)


@unique
class TagId(Enum):
    """Reserved tag id's for the Qorvo Matter SDK"""
    END_MARKER = 0
    TEST_DAC_CERT = 1
    TEST_DAC_PRIVATE_KEY = 2
    TEST_DAC_PUBLIC_KEY = 3
    PAI_CERT = 4
    CERTIFICATION_DECLARATION = 5
    DISCRIMINATOR = 6
    ITERATION_COUNT = 7
    SALT = 8
    VERIFIER = 9


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
        return (f"tag id:       {self.tag_id:#x}{os.linesep}"
                f"data length:  {self.subelement_data_len:#x}{os.linesep}"
                f"data:         {self.subelement_data.hex()}")

    @staticmethod
    def create_uint32(tag_id, value):
        return FactoryDataElement(tag_id, struct.pack("<I", value))

    @staticmethod
    def from_file(tag_id, file_path):
        with open(file_path, 'rb') as file_descriptor:
            return FactoryDataElement(tag_id, file_descriptor.read(-1))

    @staticmethod
    def create_end_marker():
        return FactoryDataElement(TagId.END_MARKER, bytes())


class FactoryDataContainer:
    """A Factory Data Container"""
    magic = b"QFDA"

    def __init__(self, maximum_size: int = 0x1000):
        self.elements: List[FactoryDataElement] = []
        self.maximum_size = maximum_size

    def add(self, element: FactoryDataElement):
        self.elements.append(element)

    def serialize(self) -> bytes:
        assert len(self.elements) > 0, "An empty container is not allowed"

        if self.elements[-1].tag_id != TagId.END_MARKER:
            # add end marker
            self.add(FactoryDataElement.create_end_marker())

        for element in self.elements:
            print(element)
        factory_data = b"".join([self.magic] + [element.serialize() for element in self.elements])

        assert len(factory_data) < self.maximum_size
        return factory_data


def generate_factory_bin(args: FactoryDataGeneratorArguments) -> bytes:
    """main application"""
    container = FactoryDataContainer(maximum_size=args.maximum_size)
    if args.passcode:
        logging.info("Discriminator:%s Passcode:%s", args.discriminator, args.passcode)
        spake2p_params = gen_spake2p_params(args.passcode)
        container.add(FactoryDataElement.create_uint32(TagId.DISCRIMINATOR, args.discriminator))
        container.add(FactoryDataElement.create_uint32(TagId.ITERATION_COUNT, spake2p_params['Iteration Count']))
        container.add(FactoryDataElement(TagId.SALT, spake2p_params['Salt']))
        container.add(FactoryDataElement(TagId.VERIFIER, spake2p_params['Verifier']))

    if args.dac_key:
        (dac_raw_privkey, dac_raw_pubkey) = gen_raw_ec_keypair_from_der(args.dac_key)
        container.add(FactoryDataElement.from_file(TagId.TEST_DAC_CERT, args.dac_cert))
        container.add(FactoryDataElement(TagId.TEST_DAC_PRIVATE_KEY, dac_raw_privkey))
        container.add(FactoryDataElement(TagId.TEST_DAC_PUBLIC_KEY, dac_raw_pubkey))
        container.add(FactoryDataElement.from_file(TagId.PAI_CERT, args.pai_cert))
        container.add(FactoryDataElement.from_file(TagId.CERTIFICATION_DECLARATION, args.certification_declaration))

    if args.data:
        for (tag, filename) in args.data:
            container.add(FactoryDataElement.from_file(tag, filename))

    return container.serialize()


def parse_command_line_arguments() -> FactoryDataGeneratorArguments:
    """parse command line arguments"""
    def any_base_int(s):
        return int(s, 0)

    def int_double_colon_string(string):
        return (int(string.split(':')[0], 0), string.split(':')[1])

    parser = argparse.ArgumentParser(description='Chip Factory NVS binary generator tool')

    parser.add_argument('-p', '--passcode', type=any_base_int,
                        help='The discriminator for pairing, range: 0x01-0x5F5E0FE')
    parser.add_argument('-d', '--discriminator', type=any_base_int,
                        help='The passcode for pairing, range: 0x00-0x0FFF')
    parser.add_argument('--dac-cert', type=str,
                        help='The path to the DAC certificate in der format')
    parser.add_argument('--dac-key', type=str,
                        help='The path to the DAC private key in der format')
    parser.add_argument('--pai-cert', type=str,
                        help='The path to the PAI certificate in der format')
    parser.add_argument('--certification-declaration', type=str,
                        help='The path to the certificate declaration der format')
    parser.add_argument('-s', '--maximum-size', type=any_base_int, required=False, default=0x6000,
                        help='The maximum size of the factory blob, default: 0x6000')
    parser.add_argument("--out_file",
                        help="Path to output file (.bin file)")
    parser.add_argument('--data', action='append', type=int_double_colon_string,
                        help="extra element to add, specify tag_integer:filename")

    args = parser.parse_args()
    return FactoryDataGeneratorArguments(**vars(args))


def main():
    """entry point of the program"""
    args = parse_command_line_arguments()
    validate_args(args)
    # check_tools_exists()
    with open(args.out_file, 'wb') as file_descriptor:
        file_descriptor.write(generate_factory_bin(args))


if __name__ == "__main__":
    logging.basicConfig(format='[%(asctime)s] [%(levelname)7s] - %(message)s', level=logging.INFO)
    main()
