#!/usr/bin/env python3
"""
This script generates a XML file with minimal Flash Programming Control Structure for the given HEX firmware file.
It is compliant with the XML version 0, which specifies the minimum fields required in the XML file.
These are: GP_Validation and Firmware_info

For the complete XML definition for each target, see the following documents:
    K7B: GP_P007_IS_08671_Flash_Programming
    K8A: GP_P008_IS_10851_Flash_Programming
    K8C: GP_P008_IS_16546_Flash_Programming_Control_Structure_XP3002
    K8D, K8E: GP_P008_IS_18032_Flash_Programming_Control_Structure_XP3004_XP3003
"""

from typing import List, Optional, Tuple
import os
import re
from enum import Enum, unique
from binascii import crc32
import struct
import unittest
from dataclasses import dataclass, field

import click
from Cheetah.Template import Template
from intelhex import IntelHex
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
import cryptography.x509
import cryptography.hazmat.primitives.serialization


@unique
class SecureBootMode(Enum):
    """Secure boot modes"""
    PRODUCT_KEY_0_AS_SYMMETRIC_KEY = 0x01
    USER_KEY_0_AS_SYMMETRIC_KEY = 0x02
    ROM_AS_ASYMETRIC_ECDSA_SECP256R1_PUBLIC_KEY = 0x10
    USER_KEYS_0_3_AS_ASYMMETRIC_ECDSA_SECP256R1_PUBLIC_KEY = 0x11

    @staticmethod
    def repr_value(mode: int) -> str:
        """return name from value"""
        try:
            return SecureBootMode(mode).name
        except ValueError:
            return 'UNKNOWN'


@dataclass
class UserKey:
    """Userkey details for use in the programmer XML template"""
    index: int
    comment: str
    source: str
    value: Optional[str]
    hash: Optional[str]


@dataclass
class EnrollmentField:
    """Details about an enrollment field for the XML"""
    name: str
    value: str


def find_hash_user_key(user_key: bytes, user_key_index: int):
    """ Calculate the key's hash the way the ROM bootloader does """
    # user key starts from compressed address 0x380180 and has length 16 bytes.
    user_key = bytearray(reversed(user_key))
    GP_MM_FLASH_NVR_SECT_0_KEY0_START = 0x00380180
    key_address = GP_MM_FLASH_NVR_SECT_0_KEY0_START + 16 * user_key_index
    address_key_in_bytes = struct.unpack('4B', struct.pack('<I', key_address))
    cipher_in = bytearray(address_key_in_bytes) + bytearray(12 * [0x00])
    cipher = Cipher(algorithms.AES(bytes(user_key)), modes.ECB())
    encryptor = cipher.encryptor()
    hash_ = encryptor.update(bytes(cipher_in)) + encryptor.finalize()
    return bytearray(reversed(hash_))


class TestHashUserKey(unittest.TestCase):
    """ Sanity check unittest for the hash key algorithm """
    SECURE_BOOT_PRODUCT_KEY_0_AS_SYMMETRIC_KEY_HASH = "6D15C8D4B5E3022B3453C9B16CB52911"
    SECURE_BOOT_PRODUCT_KEY_0_AS_SYMMETRIC_KEY_VALUE = "E5E5E5E5E5E5E5E5E5E5E5E5E5E5E501"

    def test_product_key_as_symmetric_key_value(self):
        inp = bytearray.fromhex(self.SECURE_BOOT_PRODUCT_KEY_0_AS_SYMMETRIC_KEY_VALUE)
        outp = bytearray.fromhex(self.SECURE_BOOT_PRODUCT_KEY_0_AS_SYMMETRIC_KEY_HASH)
        self.assertEqual(find_hash_user_key(inp, 7), outp)

    def test1(self):
        inp = bytearray.fromhex("FFEEDDCCBBAA99887766554433221100")
        outp = bytearray.fromhex("afd8f39fba986aa535556696995ea334")
        self.assertEqual(find_hash_user_key(inp, 0), outp)


def gen_raw_pubkey_from_der(key_file: str) -> bytes:
    """Load an ec keypair"""
    with open(os.path.expandvars(key_file), 'rb') as file_descriptor:
        key_data = file_descriptor.read()

    key_der = cryptography.hazmat.primitives.serialization.load_pem_public_key(key_data)

    return key_der.public_bytes(
        cryptography.hazmat.primitives.serialization.Encoding.X962,
        cryptography.hazmat.primitives.serialization.PublicFormat.UncompressedPoint)


def add_secureboot(config):
    """ Add secureboot configuration key and public key chunked over several userkeys """
    # Add mode setting in key 7
    configuration_key = bytearray([0xe5] * 15 + [config.secure_boot_type])
    comment = f"This key value sets the SecureBoot mode to {SecureBootMode.repr_value(config.secure_boot_type)}"
    config.user_keys += [UserKey(
        7,
        comment,
        "XML",
        configuration_key.hex(),
        find_hash_user_key(configuration_key, 7).hex()
    )]

    if config.secure_boot_type == SecureBootMode.\
            USER_KEYS_0_3_AS_ASYMMETRIC_ECDSA_SECP256R1_PUBLIC_KEY.value:
        # load public key
        public_key = gen_raw_pubkey_from_der(config.user_public_key_path)
        # strip off type byte
        public_key = public_key[1:]

        chunks = [public_key[0 * 16:0 * 16 + 16],
                  public_key[1 * 16:1 * 16 + 16],
                  public_key[2 * 16:2 * 16 + 16],
                  public_key[3 * 16:3 * 16 + 16]]

        for i, chunk in enumerate(chunks):
            chunk = bytearray(reversed(bytearray(chunk)))
            config.user_keys.append(UserKey(
                i,
                f"part {i} of the public key the user application is signed with",
                "XML",
                chunk.hex(),
                find_hash_user_key(chunk, i).hex()
            ))

    else:
        raise ValueError(f"Unsupported mode: {SecureBootMode.repr_value(config.secure_boot_type)}")


class CRCSpecification:
    """Class that holds the required fields for the generated XML file. It also provides methods to calculate and update
    the required object values based on the calculation result and type.

    Note that the internal range is closed-open: [start, end)"""

    def __init__(self, start: int, end: Optional[int] = None, size: Optional[int] = None) -> None:
        self.start = start
        if end is not None:
            self.end = end
            self.size = self.end - self.start
        elif size is not None:
            self.size = size
            self.end = self.start + self.size
        else:
            self.size = "MAX_XML_SIZE"
            self.end = None
        # Check arguments
        if start % 4 != 0:
            raise ValueError("CRC_start_address should be 4 bytes aligned")
        if end is not None and end % 4 != 0:
            raise ValueError("CRC_end_address should be 4 bytes aligned")
        if size is not None and size % 4 != 0:
            raise ValueError("CRC_size should be 4 bytes aligned")
        self.value = None
        self.type = None

    def calculate_crc32_incremental(self, data: IntelHex) -> None:
        """Calculate the image's CRC based on the start and size/end values.
        Padding is determined by the given IntelHex object.

        :param data: data from which the CRC32 will be calculated.
        """
        # Determine size of CRC area based on highest address in image.
        # Round up to multiple of 4 bytes
        if self.size == "MAX_XML_SIZE":
            self.size = data.maxaddr() - self.start + 1
            self.size = ((self.size + 3) >> 2) << 2
            self.end = self.start + self.size

        # Convert dictionary to linear buffer, mind the different exclusive/inclusive ranges.
        bin_data = data.tobinarray(start=self.start, end=self.end - 1)
        # Calculate CRC over buffer and set the calculation type
        self.value = crc32(memoryview(bin_data)) & 0xFFFFFFFF
        self.type = 0x01

    @ property
    def is_valid(self):
        """ indicates the crc specification is valid """
        return (
            self.start is not None
            and self.size is not None
            and self.end is not None
            and self.type is not None
            and self.value is not None
        )


@dataclass
class ApplicationConfig:
    """Data class with application configuration details"""
    input_filename: str
    hex_root_path: str
    hex_list: List[str]
    gp_product_id: str
    crc: Optional[CRCSpecification] = None
    secure_boot_type: Optional[int] = None
    user_public_key_path: Optional[str] = None
    user_keys: Optional[List[UserKey]] = field(default_factory=list)
    enrollment: Optional[bool] = None
    factorydata_address: Optional[int] = None
    server_address: Optional[str] = None
    enrollment_fields: Optional[List[Tuple[str, str]]] = None


def calculate_crc(config: ApplicationConfig) -> None:
    """Calculate CRC and update the values in the supplied config. Calculation is only applicable to K8A targets.

    :raises ValueError: The start address must be aligned
    :param config: Application config from which the CRC data will be calculated."""
    if config.crc is None:
        return

    # validity check
    # With HEX file (K8), the start address must contain only the 24 most significant bits of the actual address.
    if (config.crc.start & 0xff) != 0:
        raise ValueError("--crcstart must be aligned to 256 bytes for HEX files")

    # Collect all values from the HEX files.
    hexdata = IntelHex()
    hexdata.padding = 0x00

    for linkref in config.hex_list or []:
        # Assume that the referred HEX file path is relative to the output directory
        linkfile = os.path.join(config.hex_root_path, linkref)
        datablock = IntelHex()
        datablock.fromfile(linkfile, 'hex')
        hexdata.merge(datablock)

    config.crc.calculate_crc32_incremental(hexdata)


def make_paths_relative(hexlink: List[str], output_path) -> List[str]:
    """ helper function to put paths relative to the XML in the XML """
    hex_list = []
    for hex_file in hexlink:
        if os.path.isabs(hex_file):
            hex_list.append(os.path.relpath(hex_file, output_path))
        else:
            hex_list.append(hex_file)
    return hex_list


def validate_productid(gpproductid: str):
    """Validate the product ID argument"""
    PartnumberGpFormat_re_str = r"^[A-Z]{2}\d{3}[a-zA-Z0-9]{4}$"
    PartnumberQpgFormat_re_str = r"^[A-Z]{3}\d{4}[J,F,G,M]?$"
    if (
        not re.search(PartnumberGpFormat_re_str, gpproductid.strip())
        and not re.search(PartnumberQpgFormat_re_str, gpproductid.strip())
        and not gpproductid == 'UNKNOWN'
    ):
        raise click.BadArgumentUsage(
            f"GP_Product_Id {gpproductid} doesn't have one of the expected formats: "
            + f"{PartnumberGpFormat_re_str} OR {PartnumberQpgFormat_re_str}"
        )


def add_crc(config: ApplicationConfig, crcstart, crcend, crcsize):
    """Calculate CRC value and add to ApplicationConfig"""
    if crcend is not None and crcsize is not None:
        raise click.BadArgumentUsage("Only one of the CRC_end_address and CRC_size options can be specified")
    crc_start = int(crcstart, 0)
    crc_end = int(crcend, 0) if crcend else None
    crc_size = int(crcsize, 0) if crcsize else None
    config.crc = CRCSpecification(crc_start, crc_end, crc_size)
    calculate_crc(config)


@ click.command()
@ click.option('-o', '--output', 'outputfilename', required=True, help="Output XML filename.")
@ click.option('-i', '--input', 'inputfilename', required=True, help="Input XML filename or template.")
@ click.option(
    '--hexlink',
    type=click.STRING,
    required=True,
    multiple=True,
    help="HEX filename which will be added to the XML. There can be multiple filenames supplied."
)
@ click.option('--gpproductid', default='UNKNOWN', help="GP Product ID.")
@ click.option('--crcstart', type=click.STRING, help="Start of CRC protected area, should be 4 bytes aligned.")
@ click.option(
    '--crcend',
    type=click.STRING,
    help="Advanced mode to explicitly specify the end of CRC protected area, should be 4 bytes aligned. "
    + "By default the maximum address in the specified XML files will be used."
)
@ click.option(
    '--crcsize',
    type=click.STRING,
    help="Advanced mode to explicitly specify the size of CRC protected area, should be 4 bytes aligned. "
    + "If not specified, it will be calculated with the maximum address in the specified XML files."
)
@ click.option(
    '--secure-boot-type',
    type=click.INT,
    help="Enable secure boot from rom bootloader, specifying the type"
)
@ click.option(
    '--user-public-key-path',
    type=click.STRING,
    help="path to the pem file with public key used to sign the application."
)
@ click.option(
    '--enrollment',
    is_flag=True,
    default=False,
    help="Add enrollment information"
)
@ click.option(
    '--factorydata-address',
    type=click.STRING,
    help="set factorydata address"
)
@ click.option(
    '--enroll-key',
    type=click.INT,
    help="enroll this key using the service",
    multiple=True
)
@ click.option(
    '--server-address',
    type=click.STRING,
    default="http://localhost:8000",
    help="Set the enrollment server address"
)
@ click.option(
    '--enrollment-field',
    type=(str, str),
    multiple=True,
    help="Add enrollment fields"
)
def main(
    outputfilename: str,
    inputfilename: str,
    hexlink: List[str],
    gpproductid: str,
    crcstart: Optional[str],
    crcend: Optional[str],
    crcsize: Optional[str],
    secure_boot_type: Optional[int],
    user_public_key_path: Optional[str],
    enrollment: bool,
    factorydata_address: Optional[str],
    enroll_key: Optional[List[int]],
    server_address: Optional[str],
    enrollment_field: Optional[List[Tuple[str, str]]]
) -> None:
    """This script generates a basic programmer XML file for the suppliex HEX files."""
    # argument checks
    validate_productid(gpproductid)

    output_path = os.path.dirname(outputfilename)

    config = ApplicationConfig(
        input_filename=inputfilename,
        hex_root_path=output_path,
        hex_list=make_paths_relative(hexlink, output_path),
        gp_product_id=gpproductid,
        enrollment=enrollment,
        crc=None,
        secure_boot_type=secure_boot_type,
        user_public_key_path=user_public_key_path,
        factorydata_address=factorydata_address,
        server_address=server_address,
        enrollment_fields=[EnrollmentField(name, value) for name, value in enrollment_field]
    )

    if crcstart is not None:
        add_crc(config, crcstart, crcend, crcsize)
    if enroll_key is not None:
        for key_index in enroll_key:
            config.user_keys.append(UserKey(
                key_index,
                "A key from the enrollment service",
                "FactoryBlockGeneratorService",
                None,
                None
            ))

    if config.secure_boot_type is not None:
        add_secureboot(config)

    gen = Template(file=config.input_filename, searchList={'config': config})
    # Call str outside of context manager for the case when the template rendering fails the XML file won't be updated.
    xml_data = str(gen)
    with open(outputfilename, 'w', encoding='utf-8') as file_handle:
        file_handle.write(xml_data)


if __name__ == "__main__":
    # ignore error caused by click
    # pylint: disable=no-value-for-parameter
    main()
