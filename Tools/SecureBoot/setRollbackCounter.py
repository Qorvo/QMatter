import os
import sys
import argparse
import logging
import struct

from intelhex import IntelHex

# User license layout is described in SW30236_AN_Vol_2_Secure_User_Mode_Bootloader_Implementation.pdf
USER_LICENSE_ROLLBACK_COUNTER_OFFSET = 0x84


def auto_int(x):
    """auto_int converts any format to int."""
    return int(x, 0)


def parse_command_line_arguments():
    """parse_command_line_arguments parses setRollbackCounter arguments."""

    parser = argparse.ArgumentParser()

    parser.add_argument("--hex", required=True, help="path to hex file to be modified")
    parser.add_argument("--rollback_counter", required=True, type=auto_int,
                        help="new rollback protection counter value")
    parser.add_argument("--license_address", required=True, type=auto_int, help="loaded user license address")

    args = parser.parse_args()

    return args


def main():
    """main opens hex file and overwrites rollback counter in user license."""

    logging.basicConfig(level=logging.INFO)

    args = parse_command_line_arguments()

    # Rollback counter is located under fixed offset in user license
    rollback_counter_address = args.license_address + USER_LICENSE_ROLLBACK_COUNTER_OFFSET

    # Open hex file
    image = IntelHex()
    image.loadhex(args.hex)

    # Check if user license is present
    if image.todict().get(args.license_address) == None:
        logging.error(
            "User license not found at %s in %s",
            "0x{:08X}".format(args.license_address),
            os.path.basename(args.hex))
        sys.exit()

    # Read rollback counter (little endian)
    old_rollback_counter = struct.unpack("<I", image.gets(rollback_counter_address, 4))[0]

    # Overwrite rollback counter (little endian)
    image.puts(rollback_counter_address, struct.pack('<I', args.rollback_counter))

    # Save modified image
    image.write_hex_file(args.hex)

    logging.info("Rollback protection counter overwritten:")
    logging.info("Old value : %s", "0x{:08X}".format(old_rollback_counter))
    logging.info("New value : %s", "0x{:08X}".format(args.rollback_counter))
    logging.info("Address   : %s", "0x{:08X}".format(rollback_counter_address))
    logging.info("File      : %s", os.path.basename(args.hex))


if __name__ == "__main__":
    main()
