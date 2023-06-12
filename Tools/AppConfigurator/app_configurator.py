#!/usr/bin/env python3

import argparse
import sys
import os
import logging
import shutil
import subprocess
from dataclasses import dataclass
from typing import Tuple
from copy import deepcopy

sys.path.append('..')

from AppCreator.app_creator import AppCreator
from AppCreator.app_creator import AppCreatorArguments
from AppCreator.app_creator import matter_apps_path
from AppCreator.app_creator import work_path
from AppCreator.app_creator import matter_factory_configs_path
from AppCreator.app_creator import smart_home_and_lighting_bsp_path
from AppCreator.app_creator import jdebug_example_file_path


DESCRIPTION = """\
Generate a new instance of an existing Matter application and/or reconfigure it.
"""

ACTIVATE_SCRIPT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "Scripts", "activate.sh"))
activated_env_var = f"CHIP_ROOT"

assert os.getenv(f"{activated_env_var}"), \
    f"Failed to get {activated_env_var} !"\
    f"\nplease activate your environment with the following command and try again:"\
    f"\n$> cd {os.path.dirname(os.path.dirname(ACTIVATE_SCRIPT))} && "\
    f"source {os.path.join(os.path.basename(os.path.dirname(ACTIVATE_SCRIPT)), os.path.basename(ACTIVATE_SCRIPT))} && cd -"


def find_string_in_file(filepath: str, string_to_find: str) -> bool:
    with open(filepath) as f:
        s = f.read()
        if string_to_find in s:
            return True
        else:
            return False


def replace_string_in_file(filepath: str, source_string: str, target_string: str):
    update_str = f"{filepath} -> \n {source_string} \n with \n {target_string}"

    with open(filepath) as f:
        s = f.read()
        if source_string not in s:
            raise RuntimeError(f"Failed to update {update_str}")

        s = s.replace(f"{source_string}", f"{target_string}")

    with open(filepath, "w") as f:
        f.write(s)
        logging.info(f"Updated {update_str}")


def get_list_of_apps():
    for dirname, dirs, files in os.walk(matter_apps_path):
        dirs.remove("shared")
        return dirs


def get_list_of_factory_configs():

    factory_configs = list()
    for dirname, dirs, files in os.walk(matter_factory_configs_path):
        for file in files:
            if "test_" in file:
                # skip all tests configs
                continue

            if ".factory_data_config" in file:
                factory_configs.append(file)
    return factory_configs


@dataclass
class AppConfigurationArguments:
    """helper to enforce type checking on argparse output"""
    update_strategy: str
    reference_app_name: str
    high_temperature_support: bool
    external_sleep_crystal: bool
    sw_version: int
    openthread_devicetype: str
    factory_config: str
    debugging: str


class AppConfigurator():
    """Application class generator that intakes a reference application and generates a new one"""

    def __init__(self, args: AppConfigurationArguments):
        """Constructor passing arguments as part of arguments class to self."""

        target_app_name = f"{args.reference_app_name}"

        if args.update_strategy == "create":
            if args.debugging:
                target_app_name += f"_dbg"

            if args.high_temperature_support == "enable":
                target_app_name += f"_HT"
            elif args.high_temperature_support == "disable":
                target_app_name += f"_noHT"

            if args.external_sleep_crystal != None:
                target_app_name += f"_xtal"

            if args.openthread_devicetype != None:
                target_app_name += f"_{args.openthread_devicetype}"

            app_creator_args = AppCreatorArguments(args.reference_app_name, target_app_name)
            app_creator = AppCreator(app_creator_args)
            app_creator.create_app()
        self.set_target_app_name(target_app_name)

        print(f"{args.update_strategy} {target_app_name} application".capitalize())

    def build_all_makesfiles(self):
        for makefile in self._makefiles:
            out = subprocess.run(["make", "-f", makefile, "clean"],
                                 cwd=os.path.join(matter_apps_path, self._target_app_name), capture_output=True)

            print(out.stdout.decode("utf-8"))
            print(out.stderr.decode("utf-8"))
            out.check_returncode()

            out = subprocess.run(["make", "-f", makefile], cwd=os.path.join(matter_apps_path,
                                 self._target_app_name), capture_output=True)
            print(out.stdout.decode("utf-8"))
            print(out.stderr.decode("utf-8"))
            out.check_returncode()

    def set_target_app_name(self, target_app_name: str):
        if target_app_name not in get_list_of_apps():
            raise RuntimeError(f"Can't select app {target_app_name} please select out of [{get_list_of_apps()}]")

        self._target_app_name = target_app_name
        self._update_application_tree()

    def _update_list_of_postbuilds(self):
        self._postbuilds = list()
        for dirname, dirs, files in os.walk(os.path.join(matter_apps_path, self._target_app_name)):
            for file in files:
                if "_postbuild.sh" in file:
                    self._postbuilds.append(os.path.join(f"{dirname}", file))

        if len(self._postbuilds) == 0:
            raise RuntimeError(f"Can't find any postbuilds for app {self._target_app_name}")

        logging.info(f"Found postbuilds {self._postbuilds}")

    def _update_list_of_makefiles(self):
        self._makefiles = list()
        for dirname, dirs, files in os.walk(os.path.join(matter_apps_path, self._target_app_name)):
            for file in files:
                if "Makefile" in file:
                    self._makefiles.append(os.path.join(f"{dirname}", file))

        if len(self._makefiles) == 0:
            raise RuntimeError(f"Can't find any makefiles for app {self._target_app_name}")

        logging.info(f"Found Makefiles {self._makefiles}")

    def _update_list_of_chip_config_headers(self):
        self._chip_config_headers = list()
        for dirname, dirs, files in os.walk(os.path.join(matter_apps_path, self._target_app_name)):
            for file in files:
                if "CHIPProjectConfig.h" in file:
                    self._chip_config_headers.append(os.path.join(f"{dirname}", file))
                    # logging.info(f"appending {os.path.join(dirname, file)}")

        if len(self._chip_config_headers) == 0:
            raise RuntimeError(f"Can't find any config headers for app {self._target_app_name}")

    def _update_list_of_qorvo_config_headers(self):
        self._qorvo_config_headers = list()
        for dirname, dirs, files in os.walk(os.path.join(matter_apps_path, self._target_app_name)):
            for file in files:
                if "qorvo_config.h" in file:
                    self._qorvo_config_headers.append(os.path.join(f"{dirname}", file))
                    # logging.info(f"appending {os.path.join(dirname, file)}")

        if len(self._qorvo_config_headers) == 0:
            raise RuntimeError(f"Can't find any config headers for app {self._target_app_name}")

    def _update_application_tree(self):
        self._update_list_of_makefiles()
        self._update_list_of_postbuilds()
        self._update_list_of_chip_config_headers()
        self._update_list_of_qorvo_config_headers()

    def _update_postbuild_scripts(self, source_string: str, target_string: str):
        for file in self._postbuilds:
            replace_string_in_file(file, source_string, target_string)

    def _update_qorvo_config_headers(self, source_string: str, target_string: str):
        for file in self._qorvo_config_headers:
            replace_string_in_file(file, source_string, target_string)

    def _update_chip_config_headers(self, source_string: str, target_string: str):
        for file in self._chip_config_headers:
            replace_string_in_file(file, source_string, target_string)

    def _update_makefiles(self, source_string: str, target_string: str, ignore_list=None):
        list_of_makefiles_to_update = deepcopy(self._makefiles)

        if None != ignore_list:
            for buildsuffix in ignore_list:
                for makefile in list_of_makefiles_to_update:
                    if buildsuffix in makefile:
                        list_of_makefiles_to_update.remove(makefile)

        for makefile in list_of_makefiles_to_update:
            replace_string_in_file(makefile, source_string, target_string)

    def _find_in_qorvo_config_headers(self, string_to_find: str) -> bool:
        for file in self._qorvo_config_headers:
            if find_string_in_file(file, string_to_find):
                logging.info(f"found {string_to_find} in {file}")
                return True

        return False

    def _find_in_makefiles(self, string_to_find: str) -> bool:
        for file in self._makefiles:
            if find_string_in_file(file, string_to_find):
                logging.info(f"found {string_to_find} in {file}")
                return True

        return False

    def set_high_temperature_support(self, setting) -> bool:

        if setting == "enable":
            if self._find_in_makefiles("_125degC"):
                logging.warning(f"125 degC dependencies already found in {self._makefiles}")
                return True

            self._update_makefiles("QorvoStack_qpg6105/QorvoStack_qpg6105.ld", "__PLACEHOLDER__")
            self._update_makefiles("QorvoStack_qpg6105", "QorvoStack_qpg6105_125degC")
            self._update_makefiles("__PLACEHOLDER__", "QorvoStack_qpg6105/QorvoStack_qpg6105.ld")

        else:
            if not self._find_in_makefiles("_125degC"):
                logging.warning(f"125 degC dependencies not found in {self._makefiles}")
                return True

            self._update_makefiles("QorvoStack_qpg6105/QorvoStack_qpg6105.ld", "__PLACEHOLDER__")
            self._update_makefiles("QorvoStack_qpg6105_125degC", "QorvoStack_qpg6105")
            self._update_makefiles("__PLACEHOLDER__", "QorvoStack_qpg6105/QorvoStack_qpg6105.ld")

        return False

    def set_debugging_support(self, setting) -> bool:

        if setting == "enable":
            if not self._find_in_makefiles("Bootloader_qpg6105_compr_secure"):
                logging.warning(f"no secure bootloader dependencies found in {self._makefiles}")
                return True

            self._update_makefiles(
                "Bootloader_qpg6105_compr_secure",
                "Bootloader_qpg6105",
                ignore_list=["production"])

            self._update_postbuild_scripts(
                '--pem_file_path "${SCRIPT_DIR}"/../../../Tools/Ota/example_private_key.pem.example', "")
            self._update_postbuild_scripts('--pem_password test1234', "")
            self._update_postbuild_scripts('--sign', "")

            replace_string_in_file(
                jdebug_example_file_path,
                "<path>/<to>/QMatter",
                os.path.join(os.path.dirname(jdebug_example_file_path), "QMatter"))

            replace_string_in_file(
                jdebug_example_file_path,
                "<path>/<to>/qmatter",
                os.path.join(os.path.dirname(jdebug_example_file_path), "qmatter"))

            for makefile in self._makefiles:
                if "development" in makefile:
                    targetname = f"{os.path.basename(makefile).replace('Makefile.', '')}"
                    replace_string_in_file(
                        jdebug_example_file_path,
                        "Work/light_qpg6105_development/light_qpg6105_development.elf",
                        f"Work/{targetname}/{targetname}.elf")

            replace_string_in_file(
                os.path.join(matter_apps_path, self._target_app_name, "src", "AppTask.cpp"),
                "CHIP_ERROR AppTask::Init()\n{",
                "CHIP_ERROR AppTask::Init()\n{\n    qvIO_EnableSleep(false);\n"
            )
        else:
            raise NotImplementedError(f"disable debugging not supported by {os.path.basename(__file__)}")

    def set_external_sleep_crystal_support(self, setting) -> bool:
        if setting == "enable":
            if self._find_in_qorvo_config_headers("gpBsp_QPG6105DK_B01_xtal_sleep.h"):
                logging.warning(f"crystal sleep dependencies already found")

                return True

            source_bsp = os.path.join(smart_home_and_lighting_bsp_path, "gpBsp_QPG6105DK_B01.h")
            target_bsp = os.path.join(smart_home_and_lighting_bsp_path, "gpBsp_QPG6105DK_B01_xtal_sleep.h")
            shutil.copy(source_bsp, target_bsp)

            self._update_qorvo_config_headers("gpBsp_QPG6105DK_B01.h", "gpBsp_QPG6105DK_B01_xtal_sleep.h")

            replace_string_in_file(target_bsp,
                                   "#define GP_BSP_32KHZ_CRYSTAL_AVAILABLE() 0",
                                   "#define GP_BSP_32KHZ_CRYSTAL_AVAILABLE() 1"
                                   )

            if find_string_in_file(target_bsp, "#define GP_BSP_32KHZ_CRYSTAL_AVAILABLE() 0"):
                replace_string_in_file(target_bsp,
                                       "#define GP_BSP_32KHZ_CRYSTAL_AVAILABLE() 0",
                                       "#define GP_BSP_32KHZ_CRYSTAL_AVAILABLE() 1"
                                       )

            assert(False == find_string_in_file(target_bsp, "#define GP_BSP_32KHZ_CRYSTAL_AVAILABLE() 0"))
            assert(True == find_string_in_file(target_bsp, "#define GP_BSP_32KHZ_CRYSTAL_AVAILABLE() 1"))

        else:
            if self._find_in_qorvo_config_headers("gpBsp_QPG6105DK_B01.h"):
                logging.warning(f"crystal sleep dependencies already disabled")

                return True

            self._update_qorvo_config_headers("gpBsp_QPG6105DK_B01_xtal_sleep.h", "gpBsp_QPG6105DK_B01.h")

        return False

    def set_sw_version(self, update_version_int: int) -> bool:
        if update_version_int < 1000:
            update_version_string = f'"0.{str(update_version_int)}"'
        else:
            update_version_string = f'"{str(update_version_int)[0]}.{str(update_version_int)[1:]}"'

        update_version = f"0x{str(update_version_int)}"

        self._update_chip_config_headers(
            f"#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION 0x0003",
            f"#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION {update_version}",
        )
        self._update_chip_config_headers(
            f'#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING "1.1"',
            f'#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING {update_version_string}',
        )
        return False

    def set_openthread_devicetype(self, setting: str) -> bool:
        raise NotImplementedError(f"setting openthread device type not yet supported by {os.path.basename(__file__)}")


def parse_app_configurator_arguments() -> AppConfigurationArguments:
    """Parse command-line arguments"""
    parser = argparse.ArgumentParser(description=DESCRIPTION)

    requiredArgGroup = parser.add_argument_group('required arguments')
    requiredArgGroup.add_argument("--update-strategy",
                                  default="update",
                                  choices=['update', 'create', 'test'],
                                  help="Specify whether to update an existing Matter application or generate a new application instance of the reference application",
                                  required=True)

    optionalArgGroup = parser.add_argument_group('optional arguments')
    optionalArgGroup.add_argument("--ref-app-name",
                                  dest='reference_app_name',
                                  choices=get_list_of_apps(),
                                  default='base',
                                  help="name of the reference application to be updated / created. By default the 'Base' application will be used as a reference")

    optionalArgGroup.add_argument("--debugging",
                                  dest='debugging',
                                  help="Enable features needed to allow debugger to be functional")

    optionalArgGroup.add_argument("--high-temperature-support",
                                  choices=["enable", "disable"],
                                  dest='high_temperature_support',
                                  help="Enable/disable mechanisms needed to operate in [-40:125]*Celcius environments.")

    optionalArgGroup.add_argument("--external-sleep-crystal",
                                  choices=["enable", "disable"],
                                  dest='external_sleep_crystal',
                                  help="Enable/disable the configuration to enable the external sleep crystal support.")

    optionalArgGroup.add_argument("--update-sw-version",
                                  metavar='0x<abcd>',
                                  dest='sw_version',
                                  help="Update software version.")

    optionalArgGroup.add_argument("--openthread-devicetype",
                                  choices=['ftd', 'mtd', 'sed'],
                                  dest='openthread_devicetype',
                                  help="Configure the OpenThread device type.")

    optionalArgGroup.add_argument("--factory-config",
                                  choices=get_list_of_factory_configs(),
                                  help="Use a different factory data configuration file than the one found in the reference application.")

    logging.info(f"{sys.argv[0]} -> provided args : {sys.argv[1:]}")
    args = parser.parse_args()

    return AppConfigurationArguments(**vars(args))


def run_app_configurator_app(args: AppConfigurationArguments) -> bool:

    configurator = AppConfigurator(args)

    if args.high_temperature_support and configurator.set_high_temperature_support(args.high_temperature_support):
        logging.error(f"Failed to update high_temperature_support for {args}")
        return True

    if args.external_sleep_crystal and configurator.set_external_sleep_crystal_support(args.external_sleep_crystal):
        logging.error(f"Failed to update sleep crystal support for {args}")
        return True

    if args.debugging and configurator.set_debugging_support(args.debugging):
        logging.error(f"Failed to update debugging support for {args}")
        return True

    if args.sw_version and configurator.set_sw_version(args.sw_version):
        logging.error(f"Failed to update software version for {args}")
        return True

    if args.openthread_devicetype and configurator.set_openthread_devicetype(args.set_openthread_devicetype):
        logging.error(f"Failed set openthread device type for {args}")
        return True

    configurator.build_all_makesfiles()

    return False


def run_test_loop():
    EXPECT_FAIL = True
    EXPECT_PASS = False

    default_args = AppConfigurationArguments(update_strategy='create', reference_app_name="light",
                                             high_temperature_support=None,
                                             external_sleep_crystal=None,
                                             sw_version=None,
                                             openthread_devicetype=None,
                                             factory_config=None,
                                             debugging=None,
                                             )

    # Create a new instance of the light app without high temperature support
    args = deepcopy(default_args)
    args.high_temperature_support = "disable"
    assert(EXPECT_PASS == run_app_configurator_app(args))

    # Create a new instance of the switch app with high temperature support
    args = deepcopy(default_args)
    args.reference_app_name = "switch"
    args.high_temperature_support = "enable"
    assert(EXPECT_PASS == run_app_configurator_app(args))

    # Try to enable light app with high temperature support which is enable by default
    args = deepcopy(default_args)
    args.reference_app_name = "light"
    args.update_strategy = "update"
    args.high_temperature_support = "enable"
    assert(EXPECT_FAIL == run_app_configurator_app(args))

    # Try to disable switch app with high temperature support which is disabled by default
    args = deepcopy(default_args)
    args.reference_app_name = "switch"
    args.update_strategy = "update"
    args.high_temperature_support = "disable"
    assert(EXPECT_FAIL == run_app_configurator_app(args))

    # Try to disable switch app with high temperature support which is disabled by default
    args = deepcopy(default_args)
    args.reference_app_name = "switch"
    args.update_strategy = "update"
    args.high_temperature_support = "disable"
    assert(EXPECT_FAIL == run_app_configurator_app(args))

    # Try to enable lock app with sleep crystal support which is disabled by default
    args = deepcopy(default_args)
    args.reference_app_name = "lock"
    args.update_strategy = "update"
    args.external_sleep_crystal = "enable"
    assert(EXPECT_PASS == run_app_configurator_app(args))

    # Try to disable lock app with sleep crystal support which has been updated above
    args = deepcopy(default_args)
    args.reference_app_name = "lock"
    args.update_strategy = "update"
    args.external_sleep_crystal = "disable"
    assert(EXPECT_PASS == run_app_configurator_app(args))

    # Try to update lock app with debugging enabled
    args = deepcopy(default_args)
    args.reference_app_name = "lock"
    args.update_strategy = "update"
    args.debugging = "enable"
    assert(EXPECT_PASS == run_app_configurator_app(args))

    # Try to disable lock app with sleep crystal support which has been updated above
    args = deepcopy(default_args)
    args.reference_app_name = "lock"
    args.update_strategy = "update"
    args.debugging = "disable"
    try:
        run_app_configurator_app(args)
    except NotImplementedError as e:
        logging.info(e)

    # Try to update lock app with debugging enabled
    args = deepcopy(default_args)
    args.reference_app_name = "lock"
    args.update_strategy = "update"
    args.sw_version = 1001
    assert(EXPECT_PASS == run_app_configurator_app(args))

    # Try to update lock app with debugging enabled
    args = deepcopy(default_args)
    args.reference_app_name = "light"
    args.update_strategy = "update"
    args.sw_version = 31
    assert(EXPECT_PASS == run_app_configurator_app(args))


def main():
    """ Main """
    logging.basicConfig(level=logging.INFO)

    args = parse_app_configurator_arguments()

    if args.update_strategy == "test":
        run_test_loop()
        return

    run_app_configurator_app(args)


if __name__ == "__main__":
    main()
