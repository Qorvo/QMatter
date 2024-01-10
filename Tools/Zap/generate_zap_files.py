#!/usr/bin/env python3

import argparse
import sys
import os
import logging
import shutil
import subprocess

DESCRIPTION = """\
1. Starting the ZAP GUI
2. Regenerate the .zap file
3. Generate corresponding source/header files
"""

SCRIPT_PATH = os.path.dirname(__file__)
ZAP_TOOLS_PATH = f"{SCRIPT_PATH}/../../Components/ThirdParty/Matter/repo/scripts/tools/zap"

# Check if we are in the package or in the Qorvo Env
if not os.path.isfile(os.path.join(SCRIPT_PATH, "..", "ota", "crypto_utils.py")):
    ZAP_TOOLS_PATH = os.getenv("MATTER_ZAP_TOOLS_PATH", ZAP_TOOLS_PATH)


def parse_command_line_arguments():
    """Parse command-line arguments"""
    def any_base_int(string):
        return int(string, 0)
    parser = argparse.ArgumentParser(description=DESCRIPTION)

    parser.add_argument("--input",
                        help="path to input .zap file",
                        required=True)

    parser.add_argument("--nogui",
                        help="Add this option if it is not needed to do configuration in the gui",
                        action='store_true')

    args = parser.parse_args()

    return args


def run_script(command: str):
    """ run a python script using the current interpreter """
    subprocess.check_output(f"{sys.executable} {command}", shell=True)


def main():
    """ Main """

    args = parse_command_line_arguments()

    input_zap = os.path.abspath(args.input)

    script_args = [f"{input_zap}"]

    if not args.nogui:
        logging.info("=========================================================================")
        logging.info("A popup GUI window is expected to open now. Please modify the zap content\n"
                     "according to your needs, click save and close the window to continue.\n")

        logging.warning("The source files related to ZAP will be gerenated by\n"
                        "the build system according to the makefiles of the app.\n"
                        "This means there is no need to press the generate button on the GUI.")
        logging.info("=========================================================================")
        subprocess.call([f"{ZAP_TOOLS_PATH}/run_zaptool.sh"] + script_args)

    run_script(f"{ZAP_TOOLS_PATH}/generate.py {input_zap}")


if __name__ == "__main__":
    main()
