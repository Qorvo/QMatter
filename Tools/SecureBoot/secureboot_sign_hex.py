"""
This tool will create a signature of the hash of the relevant part of a hex file from a given and
patch it into the Extended User License.
The size will be determined by the intelhex module by reading out the max address in the HEX file.
Next to hashing the application data, this module will also hash:
* The VPP in the user-license
* <entries to be added>
* <entries to be added>
* <entries to be added>
"""
import argparse
import getpass
import hashlib
import os
import struct
import codecs
import sys
import logging
from typing import Callable, Tuple
from dataclasses import dataclass, field

from ecdsa import NIST256p, NIST192p
from ecdsa.curves import Curve
from intelhex import IntelHex

if os.path.isfile(os.path.join(os.path.dirname(__file__), "crypto_utils.py")):
    # In the Matter DK, all python modules are exported to this script's directory
    import crypto_utils as crypto_utils
elif not getattr(sys, 'frozen', False):
    # Get exact version of Env
    # Determine if we are an .exe or not
    current_dir = os.path.dirname(os.path.realpath(__file__))
    parent_dir = os.path.dirname(current_dir)
    sys.path.append(os.path.join(parent_dir, "..", "..", "..", "..", "Env", "vless", "gppy_vless", "inf"))
    version_file = os.path.join(parent_dir, "..", 'gpVersion.xml')
    from getEnvVersion import getEnvVersion
    sys.path.append(os.path.join(parent_dir, "..", "..", "..", "..",
                                 "Env", getEnvVersion(version_file), "gppy", "tools"))

    import crypto_utils as crypto_utils
else:
    import crypto_utils as crypto_utils

# CONSTANTS
USER_LICENSE_CRC_VALUE_OFFSET = 0x10

EXTENDED_USER_LICENSE_OFFSET = 0x80
EXTENDED_USER_LICENSE_SIGNATURE_OFFSET = (EXTENDED_USER_LICENSE_OFFSET + 16)

EXTENDED_USER_LICENSE_SECTION_1_ADDRESS_OFFSET = (EXTENDED_USER_LICENSE_OFFSET + 4)
EXTENDED_USER_LICENSE_SECTION_1_SIZE_OFFSET = (EXTENDED_USER_LICENSE_SECTION_1_ADDRESS_OFFSET + 4)

UPGRADE_SECUREBOOT_PUBLICKEY_OFFSET = 0x1800


# Who is responsible for populating the following in the EUL?
# UInt8             signatureAlgorithm;
# UInt8             curveSelection;
# UInt8             hashAlgorithm;
# UInt8             signatureSize;
# UInt32            startAddress;
# UInt32            sectionSize;


def add_signature(intel_hex_file,
                  image,
                  pem,
                  pem_password: bytes,
                  signature_address,
                  deterministic_signature=False):
    """
    add_signature adds a signature over a specified image to the specified Intel HEX file object.
    """
    (signature, public_key, curve, hash_function) = calculate_signature(image,
                                                                        pem,
                                                                        pem_password,
                                                                        deterministic_signature)

    # Write generated signature to hex file
    crypto_utils.addSignatureToIntelHexObject(intel_hex_file,
                                              signature,
                                              signature_address)

    return signature, public_key, curve, hash_function


def calculate_signature(image, pemfile_path, password, deterministic_signature=False):
    """
    calculate_signature calculates the signature over a specified image using the private key
    contained in the specified file. The password for the file is also required.
    """
    # Retrieve information from PEM file
    ("Reading PEM file: %s" % os.path.basename(pemfile_path))

    (pem_curve, private_key, public_key) = crypto_utils.getPrivatePublicKeysFromPEMFile(pemfile_path,
                                                                                        password)

    # Assure correct curves and hashes are being used together

    if pem_curve == "secp192r1":
        curve = NIST192p
        hash_function = hashlib.sha1
    elif pem_curve == "secp256r1":
        curve = NIST256p
        hash_function = hashlib.sha256
    else:
        assert False

    logging.info("Using signing curve: %s" % curve.name)
    logging.info("PEM file has curve: %s" % pem_curve)

    logging.info("Hashing and signing image")
    signature = crypto_utils.signMessage(image, private_key, curve=curve, hashfunc=hash_function,
                                         deterministic=deterministic_signature)

    logging.info("=====================================")
    logging.info("Signature to be put in user license:")
    logging.info(crypto_utils.getCCodeBuffer(signature, "signature"))
    # crypto_utils.printCCodeBuffer(private_key, "private_key")
    logging.info("=====================================")
    logging.info("Public key to be put in bootloader:")
    logging.info(crypto_utils.getCCodeBuffer(public_key, "public_key"))
    logging.info("=====================================")

    return signature, public_key, curve, hash_function


def create_intel_hex_file_object(input_hex_filename, output_hex_filename):
    """
    create_intel_hex_file_object creates an Intel HEX object to read and modify
    without altering the HEX file.
    """
    intel_hex_file = IntelHex(input_hex_filename)
    intel_hex_file.padding = 0x00  # default is 0xFF
    intel_hex_file.writePath = output_hex_filename

    logging.info("Hex contains addresses 0x%08x to 0x%08x" % (intel_hex_file.minaddr(), intel_hex_file.maxaddr()))

    return intel_hex_file


def get_license_data_to_hash(intel_hex_file, start_addr_license):
    image = b''

    start_offset = start_addr_license + USER_LICENSE_CRC_VALUE_OFFSET

    end_offset = (start_addr_license + EXTENDED_USER_LICENSE_SIGNATURE_OFFSET)

    logging.info("adding license [0x%lx,0x%lx]" % (start_offset, end_offset))

    for i in range(start_offset, end_offset):
        # Skip signature

        image += struct.pack("B", intel_hex_file[i])

    return image


def get_section_data_to_hash(intel_hex_file, start_addr, size, add_padding):
    if add_padding:
        logging.info("adding section [0x%lx,0x%lx] (size 0x%lx bytes) padding: true" % (
            start_addr, start_addr + size, size))
    else:
        logging.info("adding section [0x%lx,0x%lx] (size 0x%lx bytes) padding: false" % (
            start_addr, start_addr + size, size))
    image = b''
    for i in range(start_addr, start_addr + size):
        if add_padding:
            # Add zero byte padding redundantly
            if intel_hex_file[i] == 0x00:
                intel_hex_file[i] = 0x00
        image += struct.pack("B", intel_hex_file[i])

    return image


def add_section(intel_hex_file, start_area, license_offset, section_number, argument, padding):
    if argument is None:
        offset = 0xFFFFFFFF
        size = 0xFFFFFFFF
    else:
        offset = int(argument.split(':')[0], 16)
        size = int(argument.split(':')[1], 16)
        logging.info("Adding section at offset 0x%x with size 0x%x" % (offset, size))

    image = b''

    if offset != 0xFFFFFFFF:
        if size == 0xFFFFFFFF:
            logging.info("Taking maximum address from HEX file to calculate and align size of section with offset 0x%lx"
                         % offset)
            # # Use maximum hex file address
            # size = align_section_size((intel_hex_file.maxaddr() - (start_area + offset)), ALIGNMENT_SIZE)
            size = intel_hex_file.maxaddr() - (start_area + offset)

        logging.info("Adding section %d - start = 0x%lx - size = %d" % (section_number, start_area + offset, size))

        image += get_section_data_to_hash(intel_hex_file, start_area + offset, size, padding)

    ihex_set_uint32(intel_hex_file,
                    start_area + license_offset + EXTENDED_USER_LICENSE_SECTION_1_ADDRESS_OFFSET + (section_number * 8),
                    offset)
    ihex_set_uint32(intel_hex_file,
                    start_area + license_offset + EXTENDED_USER_LICENSE_SECTION_1_SIZE_OFFSET + (section_number * 8),
                    size)

    return image


def ihex_set_uint32(intel_hex_file, offset, value):
    """
    ihex_set_uint32 sets the 32 bit value at a specified offset inside the Intel HEX file object.
    """
    first, second, third, fourth = struct.unpack('>BBBB', struct.pack('>I', value))

    intel_hex_file[offset + 0] = fourth
    intel_hex_file[offset + 1] = third
    intel_hex_file[offset + 2] = second
    intel_hex_file[offset + 3] = first


def parse_command_line_arguments():
    """
    parse_command_line_arguments parses the command line arguments of the signfw
    application.
    """
    def str_as_bytes(string):
        return codecs.encode(string, 'UTF-8')

    def base_16_int(string):
        return int(string, 16)
    parser = argparse.ArgumentParser()

    parser.add_argument("--input_hex", required=True,
                        help="path to hex file to be signed")
    parser.add_argument("--output_hex",
                        help="output file to be written")
    parser.add_argument("--start_addr_area",
                        help="start address of flash area, typically 0x04000000")
    parser.add_argument("--asymkey",
                        help="<pem_file>:<pem_password | optional> asymmetric key")
    parser.add_argument("--pem_password",
                        type=str_as_bytes,
                        help="asymmetric key passphrase")
    parser.add_argument("--section1",
                        help="<offset>:<size> - Section 1 to hash")
    parser.add_argument('-sk', '--symkey', nargs='+', default=[])
    parser.add_argument('--force_deterministic', action='store_true',
                        help="force generated signature to be deterministic(for TESTING ONLY)")

    parser.add_argument("--write_application_public_key",
                        type=base_16_int,
                        help="base-16 offset to write the public key the application is signed with")

    parser.add_argument("--application_pem",
                        help="path to PEM file to get public key")

    parser.add_argument("--application_pem_password",
                        required=False,
                        type=str_as_bytes,
                        help="optional PEM file password")
    args = parser.parse_args()
    if not args.input_hex:
        logging.error("supply HEX file path")
        sys.exit()

    if not args.output_hex:
        args.output_hex = args.input_hex

    if not args.section1:
        # if omitted section starts from end of ENUL and goes till end of hex file, padded with zeros any gaps inbetween
        args.section1 = '0x100:0xFFFFFFFF'

    if not args.start_addr_area:
        args.start_addr_area = '0x04000000'

    if args.symkey:
        if len(args.symkey) == 1:
            args.symkey = args.symkey[0].split()
        if len(args.symkey) != 16:
            print("Expected 16 bytes for argument symkey")
            sys.exit(-1)

    return args


@dataclass
class SigningInformation:
    """Information from a PEM file needed to sign data"""
    curve: Curve
    hash_function: Callable
    private_key: bytes
    public_key: bytes


def load_pem_file(pem_file_path: str, pem_password: bytes) -> SigningInformation:
    """Load a PEM file and validate it is usable"""
    # Retrieve information from PEM file
    logging.info("Reading PEM file: %s", os.path.basename(pem_file_path))

    (pem_curve, private_key, public_key) = crypto_utils.getPrivatePublicKeysFromPEMFile(pem_file_path,
                                                                                        pem_password)

    # Assure correct curves and hashes are being used together

    logging.info("PEM file has curve: %s", pem_curve)

    if pem_curve == "secp256r1":
        curve = NIST256p
        hash_function = hashlib.sha256
    else:
        raise NotImplementedError(f"PEM file uses {pem_curve} instead of the expected 'secp256r1'")

    logging.info("Using signing curve: %s", curve.name)

    return SigningInformation(curve, hash_function, private_key, public_key)


def add_secureboot_public_key(intel_hex_file: IntelHex, sign_info: SigningInformation,
                              secureboot_public_key_offset: int) -> None:
    """Add a public key at an offset to an intel hex file object"""
    logging.info("populating secureboot public key for the bootloader to use at %#x with length %d",
                 secureboot_public_key_offset, len(sign_info.public_key))
    for index, value in enumerate(sign_info.public_key):
        intel_hex_file[secureboot_public_key_offset + index] = value


def main():
    """
    main is the entry point of the signfw application.
    """
    logging.basicConfig(level=logging.INFO)

    args = parse_command_line_arguments()

    ###############################################

    logging.info("Reading Hex file: %s" % os.path.basename(args.input_hex))

    intel_hex_file = create_intel_hex_file_object(args.input_hex, args.output_hex)
    start_addr_area = int(args.start_addr_area, 16)

    # Check that hex file contains ENUL MW placeholder
    if intel_hex_file[start_addr_area + 0x80] != 0xBC or \
            intel_hex_file[start_addr_area + 0x81] != 0x3F or \
            intel_hex_file[start_addr_area + 0x82] != 0x24 or \
            intel_hex_file[start_addr_area + 0x83] != 0xA3:
        logging.error("Supplied hex file does not contain ENUL MW, abort")
        exit()

    image = b''

    license_offset = 0
    license_address = start_addr_area + license_offset

    logging.info("Start Address = 0x%08x" % start_addr_area)
    logging.info("License located @ 0x%lx" % license_address)

    assert intel_hex_file.maxaddr() > (start_addr_area + license_offset)

    if args.write_application_public_key is not None:
        if args.application_pem_password is None:
            # Newlines were added to accentuate the password prompt in the middle of
            # make output
            prompt = "\n\n" + f"PEM password for {os.path.abspath(args.application_pem)}:" + "\n\n"
            args.application_pem_password = getpass.getpass(prompt=prompt)
            args.application_pem_password = codecs.encode(args.application_pem_password, 'UTF-8')
        sign_info: SigningInformation = load_pem_file(args.application_pem, args.application_pem_password)

        logging.info("Application signing public key inserted in bootloader:")
        logging.info(crypto_utils.getCCodeBuffer(sign_info.public_key, "public_key"))
        add_secureboot_public_key(intel_hex_file, sign_info,
                                  start_addr_area + args.write_application_public_key)

    logging.info("Hashing application")
    image += add_section(intel_hex_file, start_addr_area, license_offset, 0, args.section1, True)

    # Add Qorvo License to hash
    logging.info("Hashing license")
    image += get_license_data_to_hash(intel_hex_file, license_address)

    signature_address = license_address + EXTENDED_USER_LICENSE_SIGNATURE_OFFSET
    logging.info("Signature located @ 0x%lx" % signature_address)

    if args.asymkey:
        pem = None
        pem_password = None

        if not os.path.exists(args.asymkey):
            # Backward compatibility:
            # --> Trying to handle a combined command-line argument.
            # ---> e.g., C:\\test\\dir:test1234
            result = args.asymkey.rsplit(":", 1)
            pem = result[0]
            pem_password = codecs.encode(result[1], 'UTF-8')
        else:
            # This is a path-only argument: e.g., C:\\test\\dir
            pem = args.asymkey

        if args.pem_password is not None:
            if pem_password is not None:
                logging.error("Too many passphrases are provided for the asymmetric key. (%s and %s)" %
                              (pem_password, args.pem_password))
                exit(-1)
            pem_password = args.pem_password

        if pem_password is None:
            logging.info("No passphrase provided for the"
                         " asymmetric key.")
            pem_password = codecs.encode(getpass.getpass(prompt=f"PEM password for {pem}:"), 'UTF-8')

        (signature, public_key, curve, hash_function) = add_signature(intel_hex_file,
                                                                      image,
                                                                      pem,
                                                                      pem_password,
                                                                      signature_address,
                                                                      args.force_deterministic)

        # Verify that image is signed correctly
        if crypto_utils.verifyMessage(image,
                                      public_key,
                                      signature,
                                      curve=curve,
                                      hashfunc=hash_function):
            logging.info("SUCCESS: Message OK")
        else:
            logging.error("Message NOK")
            exit()

    elif args.symkey:
        import hashlib
        from cryptography.hazmat.backends import default_backend
        from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes

        m = hashlib.sha256()
        m.update(image)
        sha256_hash = m.digest()
        # AES encrypt sha-256 hash
        symmetric_key = bytearray([int(x, 16) for x in args.symkey])

        logging.info("Length of hash = %d" % len(sha256_hash))
        logging.info("sha-256 digest")
        logging.info(', '.join('0x{:02x}'.format(x) for x in bytearray(sha256_hash)))

        # Do AES encryption
        cipher_in = bytearray(sha256_hash)
        cipher = Cipher(algorithms.AES(bytes(symmetric_key)), modes.ECB(), default_backend())
        encryptor = cipher.encryptor()
        result = encryptor.update(bytes(cipher_in)) + encryptor.finalize()
        output = bytearray(result)

        logging.info("output")
        logging.info(', '.join('0x{:02x}'.format(x) for x in output))
        logging.info("length of AES output = %d" % len(output))

        for idx, byte_value in enumerate(output):
            intel_hex_file[signature_address + idx] = byte_value

        logging.info("Writing encrypted sha-256 hash to ENUL")

    # Overwrite the hex file with the modified (signed) intel hex object
    intel_hex_file.tofile(intel_hex_file.writePath, format='hex')


if __name__ == "__main__":
    main()
