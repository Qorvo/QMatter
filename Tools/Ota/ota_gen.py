#!/usr/bin/python
""" OTA image generator
1. The application (binary) will be split in sub-elements
   * Application
   * Jumptable
2. An preliminary OTA header will be prepended to the sub-elements to create the "data of the OTA file"
   (preliminary, as the total size should be filled in later)
3. Process the "data of the OTA file" to provide
    A) Option 1: INTEGRITY CHECK + AUTHENTICATION (SIGNATURE)
       An ECDSA signature.
       Use data from the Privacy Enhanced Mail (PEM) file to decide on the hashing strategy (e.g. sha256sum)
       and the eliptic curve.
       Pack the signature along with the signer's IEEE address in the signature sub-element
       and append it to the OTA file.
    B) Option 2: INTEGRITY CHECK (CRC)
       A checksum.
       Add the checksum to the integrity sub-element and append it to the OTA file.
"""

from ota_file_format import Header, Subelement
# from pypkg import __version__
import argparse
from ecdsa import NIST256p
import codecs
import hashlib
import logging
import os
import re
import struct
import sys
import textwrap
import zlib

moduleroot = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
sys.path.append(os.path.join(moduleroot, "comps"))
if os.path.isfile(os.path.join(os.path.dirname(__file__), "crypto_utils.py")):
    # In the Matter DK, all python modules are exported to this script's directory
    import crypto_utils as crypto_utils
else:
    # When running from the Qorvo codebase, use the dependencies from original paths
    moduleroot = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
    sys.path.append(os.path.join(moduleroot, "comps"))
    from crypto_utils import crypto_utils
    sys.path.remove(os.path.join(moduleroot, "comps"))

    # Determine if we are an .exe or not
    if not getattr(sys, 'frozen', False):
        current_dir = os.path.dirname(os.path.realpath(__file__))
        parent_dir = os.path.dirname(current_dir)
        sys.path.append(os.path.join(parent_dir, "..", "..", "..", "..", "Env", "vless", "gppy_vless", "inf"))
        # pylint: disable-next=import-error
        from getEnvVersion import getEnvVersion

        try:
            envVersion = getEnvVersion()
            envPath = os.path.join(parent_dir, "..", "..", "..", "..", "Env", envVersion)
        except Exception as e:
            # Fallback to ENV_PATH
            logging.warning("getEnvVersion() failed, falling back to ENV_PATH")
            envPath = os.path.abspath(os.environ.get('ENV_PATH', ""))

        sec_deps_path = os.path.join(envPath, "gppy", "tools", "sec")
        sys.path.append(sec_deps_path)
sys.path.remove(os.path.join(moduleroot, "comps"))


logger = logging.getLogger()
logger.setLevel(level=logging.DEBUG)


def parse_command_line_arguments():
    """Parse command-line arguments"""

    def parse_integer_string(string):
        return int(string, 0)

    def to_utf8(string):
        return codecs.encode(string, 'UTF-8')

    parser = argparse.ArgumentParser(
        description="Create an OTA image for testing Qorvo's OTA implementation.",
        formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("--binary_file", "-b",
                        required=True,
                        help=textwrap.dedent("""\
Path of the binary file with following format
0x00000000    +-----------------+
JT.OFFSET     |[opt.] jumptables|
JT.OFFSET+SIZE|[opt.] bootloader|
APP.OFFSET    |      application|
FILE END      +-----------------+"""))
    parser.add_argument("--image_version",
                        default=0x09090909,
                        help="Version of this OTA image [default: 0x09090909]",
                        type=parse_integer_string)
    parser.add_argument("--jumptable_offset",
                        default=0x00000000,
                        help="Start of the jumptable section in the \"binary_file\" [default: 0x00000000]",
                        type=parse_integer_string)
    parser.add_argument("--jumptable_size",
                        default=0,
                        help="Size of the jumptable (expected at the start of the binary_file) "
                             "[default: not available]",
                        type=parse_integer_string)
    parser.add_argument("--application_offset",
                        default=0x00000000,
                        help="Start of the application section in the \"binary_file\" [default: 0x00000000]",
                        type=parse_integer_string)
    parser.add_argument("--output_file", "-o",
                        default="ota_image.ota",
                        help="Output file name [default: \"ota_image.ota\"]")
    parser.add_argument("--pem",
                        help="[signing] Path to PEM file used to sign the image")
    parser.add_argument("--pem_password",
                        default=None,
                        help="[signing] Optional PEM file password",
                        type=to_utf8)
    # parser.add_argument("--version", "-v",
    #                     action='version',
    #                     version="%s - version %s" % (sys.argv[0], __version__),
    #                     help="Version of this tool")
    return parser.parse_args()


def create_header(image_string=b"Qorvo OTA image",
                  version=0x09090909,
                  image_type=0x0000,
                  min_hw_version=0x0000,
                  max_hw_version=0x0101):
    ota_filer_header = Header(header_string=image_string, file_version=version, image_type=image_type)
    ota_filer_header.set_hardware_version(min_hw_version=min_hw_version, max_hw_version=max_hw_version)
    # Note image_total_size not yet set!

    return ota_filer_header


def create_subelement_application(data):
    ota_file_subelement = Subelement(tag_id=Subelement.TAG_ID_MAIN_IMAGE, subelement_data=data)
    return ota_file_subelement


def create_subelement_jumptable(data):
    ota_file_subelement = Subelement(tag_id=Subelement.TAG_ID_JUMPTABLE_IMAGE, subelement_data=data)
    return ota_file_subelement


def create_subelement_signature(elements, ieee_address, pem, pem_password):
    """
        elements          List of Header or Subelements
        ieee_address [8B] Signer's virtual unique address (should be different from devices in the field)
        pem               Path to file containing the signers private key
        pem_password      String containing the password for the private key
    """
    address = struct.pack('<Q', ieee_address)
    image = b""
    for element in elements:
        image += element.serialize()
    # NOTE:
    # If ieee_address would be used, consider including the following to the image data
    # including Subelement.TAG_ID_SIGNATURE[tag_id, length, ieee_address]
    # excluding Subelement.TAG_ID_SIGNATURE[signature]

    (signature, public_key, curve, hash_function) = calculate_signature(image, pem, pem_password)
    signature = struct.pack('<%ss' % len(signature), signature)
    ota_file_subelement = Subelement(tag_id=Subelement.TAG_ID_SIGNATURE, subelement_data=address + signature)
    return ota_file_subelement


def create_subelement_hash(elements):
    """
        elements  List of Header or Subelements
    """
    crc_value = 0
    for element in elements:
        crc_value = zlib.crc32(element.serialize(), crc_value) & 0xFFFFFFFF

    # Pad CRC [4B] with 12 0's
    data = struct.pack('<12sI', b'\x00' * 12, crc_value)
    ota_file_subelement = Subelement(tag_id=Subelement.TAG_ID_IMAGE_INTEGRITY, subelement_data=data)
    return ota_file_subelement


def get_signature_length(pem, pem_password):
    """
    Get the length of the signature (based on the curve used for ECDSA (part of the PEM file))
    """
    (pem_curve, _private_key, _public_key) = crypto_utils.getPrivatePublicKeysFromPEMFile(pem,
                                                                                          pem_password)
    re_curve_names = re.compile(r"^[a-zA-Z]+([0-9]+)[a-z]+.*$")

    assert re_curve_names.match("secp256r1")

    regex_result = re_curve_names.match(pem_curve)
    if regex_result is not None:
        curve_bits = int(regex_result.group(1))
    else:
        raise AssertionError("Unable to derive ECDSA curve bits number from: {} file".format(pem))

    assert (curve_bits % 8 == 0), \
        "Failed to derive a valid number of bits used by the ECDSA curve - bits: %d" % curve_bits

    curve_length = curve_bits // 8
    signature_length = curve_length * 2  # 2 times more bytes in signature as the curve (as it contains r and s)

    return signature_length


def calculate_signature(image, pem, pem_password):
    """
    Calculates the signature over a specified image using the private key
    contained in the specified file. The password for the file is also required.
    """
    # Retrieve information from PEM file
    (pem_curve, private_key, public_key) = crypto_utils.getPrivatePublicKeysFromPEMFile(pem,
                                                                                        pem_password)

    # Assure correct curves and hashes are being used together

    if pem_curve == "secp256r1":
        curve = NIST256p
        hash_function = hashlib.sha256
    else:
        raise NotImplementedError("PEM file uses '%s' instead of the expected 'secp256r1'" % pem_curve)

    logger.debug("Using signing curve: %s" % curve.name)
    logger.debug("PEM file has curve: %s" % pem_curve)

    logger.debug("Hashing image and creating a signature")
    hash_output = crypto_utils.hashMessage(image, hashfunc=hash_function)
    logger.debug("=====================================")
    logger.debug("hash of the image:")
    logger.debug("%s" % hash_output)

    signature = crypto_utils.signMessage(image, private_key, curve=curve, hashfunc=hash_function)
    assert len(signature) == get_signature_length(pem, pem_password), \
        "Signature length does not match get_signature_length()"

    # Print signer's public key
    logger.debug("=====================================")
    logger.debug("Public key to authenticate OTA image")
    logger.debug("Compile your target with this C code")
    logger.debug(crypto_utils.getCCodeBuffer(public_key, "gpOta_authentication_key"))
    logger.debug("=====================================")

    return signature, public_key, curve, hash_function


def main():
    """
    main is the entry point of the signfw application.
    """
    arguments = parse_command_line_arguments()

    # Build OTA file via helper classes
    # Formats
    # |HEADER|SUBELEMENT_application|[opt.]SUBELEMENT_jumptable|SUBELEMENT_signature|
    # |HEADER|SUBELEMENT_application|[opt.]SUBELEMENT_jumptable|SUBELEMENT_crc|
    data_subelements = []
    helper_subelement = {}

    logger.info("Creating OTA image")

    # Application
    app_data = b""
    with open(arguments.binary_file, 'rb') as image:
        image.seek(arguments.application_offset)
        temp_data = image.read()
        while temp_data:
            app_data += temp_data
            temp_data = image.read()
    logger.info("Adding subelement: Application")
    data_subelements.append(create_subelement_application(app_data))

    # Jumptable
    if arguments.jumptable_size > 0:
        jumptable_data = b""
        with open(arguments.binary_file, 'rb') as jumptable:
            jumptable.seek(arguments.jumptable_offset)
            read_length = arguments.jumptable_size
            while read_length:
                jumptable_data += jumptable.read(read_length)
                read_length -= len(jumptable_data)
        logger.info("Adding subelement: Jumptable")
        data_subelements.append(create_subelement_jumptable(jumptable_data))

    # Prepare header (requires data_subelements)
    header = create_header(version=arguments.image_version)
    # Set image_total_size in header
    image_total_size = header.header_length
    for subelement in data_subelements:
        image_total_size += subelement.subelement_data_len + Subelement.HEADER_LEN
    if arguments.pem:
        image_total_size += Subelement.HEADER_LEN + 8 + get_signature_length(pem=arguments.pem,
                                                                             pem_password=arguments.pem_password)
    else:
        image_total_size += Subelement.HEADER_LEN + Subelement.CRC_DATA_LEN
    header.set_total_ota_file_size(size=image_total_size)

    # Calculate Signature or CRC
    elements_to_hash = [header] + data_subelements
    if arguments.pem:
        logger.info("Adding subelement: Signature")
        helper_subelement["signature"] = create_subelement_signature(elements=elements_to_hash,
                                                                     ieee_address=0xFFAB10D012345678,
                                                                     pem=arguments.pem,
                                                                     pem_password=arguments.pem_password)
    else:
        logger.info("Adding subelement: CRC")
        helper_subelement["crc"] = create_subelement_hash(elements_to_hash)

    # Write to file
    if os.path.exists(arguments.output_file):
        os.remove(arguments.output_file)

    with open(arguments.output_file, "wb+") as ota:
        ota.write(header.serialize())
        for subelement in data_subelements:
            # order = application first, optional jumptable second
            ota.write(subelement.serialize())
        if "signature" in helper_subelement:
            ota.write(helper_subelement["signature"].serialize())
        else:
            ota.write(helper_subelement["crc"].serialize())

    logger.info("--- %s file successfully created ---" % arguments.output_file)


if __name__ == "__main__":
    main()
