import argparse
import os
import sys
import shutil
import logging
from dataclasses import dataclass

matter_factory_configs_path = os.path.join(os.path.dirname(__file__), "..", "..", "Tools", "FactoryData", "Credentials")
matter_apps_path = os.path.join(os.path.dirname(__file__), "..", "..", "Applications", "Matter")
work_path = os.path.join(os.path.dirname(__file__), "..", "..", "Work")
smart_home_and_lighting_bsp_path = os.path.join(os.path.dirname(
    __file__), "..", "..", "Components", "Qorvo", "BSP", "gpBsp", "inc", "SmartHomeAndLighting")
jdebug_example_file_path = os.path.join(os.path.dirname(__file__), "..", "..", "qmatter.jdebug")

ACTIVATE_SCRIPT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "Scripts", "activate.sh"))
activated_env_var = f"CHIP_ROOT"

assert os.getenv(f"{activated_env_var}"), \
    f"Failed to get {activated_env_var} !"\
    f"\nplease activate your environment with the following command and try again:"\
    f"\n$> cd {os.path.dirname(os.path.dirname(ACTIVATE_SCRIPT))} && "\
    f"source {os.path.join(os.path.basename(os.path.dirname(ACTIVATE_SCRIPT)), os.path.basename(ACTIVATE_SCRIPT))} && cd -"


@dataclass
class AppCreatorArguments:
    """helper to enforce type checking on argparse output"""
    reference_app_name: str
    target_app_name: str


class AppCreator():
    """Application class generator that intakes a reference application and generates a new one"""

    def __init__(self, args: AppCreatorArguments):
        """Constructor passing arguments as part of arguments class to self."""
        self._args = args
        self._app_path = matter_apps_path

        self._ref_app_name = args.reference_app_name
        self._ref_app_path = os.path.abspath(os.path.join(self._app_path, f"{self._ref_app_name}"))

        self._tgt_app_name = args.target_app_name
        self._tgt_app_path = os.path.abspath(os.path.join(self._app_path, f"{self._tgt_app_name}"))

        self._validate_appname()
        self._populate_files_to_update()

    def _populate_files_to_update(self):
        """Called to populate a list of files to update based on the Application name."""

        self._files_to_update = list()
        self._files_to_update.append(os.path.join(self._ref_app_path, f"README.md"))

        # Append all Application Makefiles
        for dirname, dirs, files in os.walk(self._ref_app_path):
            for file in files:
                if "Makefile." in file:
                    self._files_to_update.append(os.path.join(self._ref_app_path, file))

                if "_postbuild.sh" in file:
                    self._files_to_update.append(os.path.join(self._ref_app_path, file))

        # Append all Application source files
        for dirname, dirs, files in os.walk(os.path.join(self._ref_app_path, "src")):
            for file in files:
                print(os.path.join(f"{dirname}", file))
                self._files_to_update.append(os.path.join(f"{dirname}", file))

        # Append all Application header files
        for dirname, dirs, files in os.walk(os.path.join(self._ref_app_path, "include")):
            for file in files:
                print(os.path.join(f"{dirname}", file))
                self._files_to_update.append(os.path.join(f"{dirname}", file))

        # Append all Application generated config files
        for dirname, dirs, files in os.walk(os.path.join(self._ref_app_path, "gen")):
            # exclude the top level directory
            if self._ref_app_name not in dirname.split("gen")[1]:
                continue
            self._files_to_update.append(os.path.join(f"{dirname}", "qorvo_internals.h"))
            self._files_to_update.append(os.path.join(f"{dirname}", "qorvo_config.h"))

        self._files_to_update.append(os.path.join(self._ref_app_path, "..", "..", "..", "Libraries",
                                                  "Qorvo", "FactoryData", f"Makefile.FactoryData_{self._ref_app_name}"))

        self._files_to_update.append(os.path.join(matter_factory_configs_path,
                                     f"qorvo_{self._ref_app_name}.factory_data_config"))
        self._files_to_update.append(os.path.join(matter_factory_configs_path,
                                     f"test_{self._ref_app_name}.factory_data_config"))

        self._word_list_to_ignore = list()
        self._word_list_to_ignore.append("basename")
        self._word_list_to_ignore.append("Component: base")
        self._word_list_to_ignore.append(f"_{self._ref_app_name}_dac_cert_0.der")
        self._word_list_to_ignore.append(f"_{self._ref_app_name}_dac_key_0.der")
        self._word_list_to_ignore.append(f"_{self._ref_app_name}_pai_cert.der")
        self._word_list_to_ignore.append(f"_{self._ref_app_name}_cd.bin")
        self._word_list_to_ignore.append(f"app/clusters/switch-server/switch-server.h")
        self._word_list_to_ignore.append(f"door-lock-server/door-lock-server.h")
        self._word_list_to_ignore.append(f"switch (btnIdx)")
        self._word_list_to_ignore.append(f"switch (event->Type)")
        self._word_list_to_ignore.append(f"switch (commandId)")
        self._word_list_to_ignore.append(f"switch (data->clusterId)")
        self._word_list_to_ignore.append(f"kUnlocked")
        self._word_list_to_ignore.append(f"UnlockThreadStack")
        self._word_list_to_ignore.append(f"nativeParams.lockCb")
        self._word_list_to_ignore.append(f"nativeParams.unlockCb")
        self._word_list_to_ignore.append(f"chip::System::Clock::Milliseconds32")

    def _validate_appname(self):
        """validate if app path already exists"""

        if not os.path.isdir(self._ref_app_path):
            raise RuntimeError(f"Can't create new app because reference app {self._ref_app_path} doesn't exist")

        if os.path.isdir(self._tgt_app_path):
            raise RuntimeError(f"Can't create new app because {self._tgt_app_path} already exists")

    def _copy_to_target_app(self):
        """Copy the application contents of the Base application to the new folder structure"""

        # -- Copy the Matter application layer
        logging.debug("Copying {self._ref_app_path} -> {self._tgt_app_path}")
        shutil.copytree(self._ref_app_path, self._tgt_app_path)

        # -- update all de directory names
        # Directories are moved first before files to avoid duplicates and
        # missing directories.
        for dirname, dirs, files in os.walk(self._tgt_app_path):
            logging.info("dn:%s, d:%s, f:%s" % (dirname, dirs, files))
            for directoryname in dirs:
                if f"{self._ref_app_name}" not in directoryname:
                    continue
                ref_path = os.path.abspath(os.path.join(dirname, directoryname))
                tgt_path = os.path.abspath(os.path.join(dirname, directoryname.replace(
                    f"{self._ref_app_name}", f"{self._tgt_app_name}")))

                if not os.path.isdir(ref_path):
                    raise RuntimeError(f"Dir {ref_path} doesn't exist")

                shutil.move(ref_path, tgt_path)

        # -- update all de file names
        for dirname, dirs, files in os.walk(self._tgt_app_path):
            # logging.info("dn:%s, d:%s, f:%s" %(dirname, dirs,files))
            for filename in files:
                if f"{self._ref_app_name}" not in filename:
                    continue
                ref_path = os.path.abspath(os.path.join(dirname, filename))
                tgt_path = os.path.abspath(os.path.join(dirname, filename.replace(
                    f"{self._ref_app_name}", f"{self._tgt_app_name}")))
                # tgt_path=ref_path.replace(f"{self._ref_app_name}", f"{self._tgt_app_name}")

                if not os.path.isfile(ref_path):
                    raise RuntimeError(f"File {ref_path} doesn't exist")

                shutil.move(ref_path, tgt_path)

        # create new instance of the files to update
        for ref_path_of_file_to_update in self._files_to_update:
            # replace base / example in original file with new app_name
            tgt_path_of_file_to_update = ref_path_of_file_to_update.replace(
                f"{self._ref_app_name}", f"{self._tgt_app_name}")
            logging.info("Copying %s -> %s" % (ref_path_of_file_to_update, tgt_path_of_file_to_update))
            shutil.copy(ref_path_of_file_to_update, tgt_path_of_file_to_update)

    def _update_files_with_new_appname(self):
        """Called to update the list of files with new target application name."""
        # update the file contents
        for filepath in self._files_to_update:

            # replace base / example in original file with new app_name
            target_filepath = filepath.replace(f"{self._ref_app_name}", f"{self._tgt_app_name}")

            logging.debug(f"updating: {target_filepath}")

            # replace all references of "{app_name}" the file with the "<target_app_name>"
            with open(filepath) as f:

                s = f.read()
                for name in self._word_list_to_ignore:
                    placeholdername = name.replace(f"{self._ref_app_name}", "__placeholder__")
                    s = s.replace(name, placeholdername)

                s = s.replace(f"{self._ref_app_name}", f"{self._tgt_app_name}")
                s = s.replace(f"{self._ref_app_name}-Matter-app".capitalize(),
                              f"{self._tgt_app_name}-Matter-app".capitalize())
                s = s.replace(f"Qorvo Matter {self._ref_app_name}", f"Qorvo Matter {self._tgt_app_name}")

                for name in self._word_list_to_ignore:
                    placeholdername = name.replace(f"{self._ref_app_name}", "__placeholder__")
                    s = s.replace(placeholdername, name)

            with open(target_filepath, "w") as f:
                f.write(s)

    def create_app(self):
        self._copy_to_target_app()
        self._update_files_with_new_appname()


def parse_app_creator_arguments() -> AppCreatorArguments:
    """Parse command-line arguments"""
    parser = argparse.ArgumentParser(description='Create new application from reference application')

    parser.add_argument('--target-app-name', required=True, metavar='<targetAppName>',
                        dest='target_app_name', type=str, help='Name of the new application')

    parser.add_argument('--reference-app-name', required=True, metavar='<referenceAppName>',
                        dest='reference_app_name', type=str, help='Name of the application that is used as a reference')

    logging.info(f"{sys.argv[0]} -> provided args : {sys.argv[1:]}")
    args = parser.parse_args()

    return AppCreatorArguments(**vars(args))


def main():

    app_creator = AppCreator(parse_app_creator_arguments())
    app_creator.create_app()

    return


if __name__ == "__main__":
    main()
