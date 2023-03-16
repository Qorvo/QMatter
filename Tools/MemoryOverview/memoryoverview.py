#!/usr/bin/env python3
'''Generate report of code sizes of relevant builds within the project
   Mapfiles from CHIP builds parsed stored in:
'''

import sys
import os
import io
from collections import namedtuple
from typing import Dict, List, Tuple, NamedTuple
from optparse import OptionParser

try:
    from prettytable import PrettyTable
except ImportError as pt_importerror:
    raise ImportError("Install PrettyTable to get table output - 'pip install PrettyTable'") from pt_importerror

moduleroot = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))

try:
    from memoryusage import parseMemory
    from layout import Memory, set_log      # noqa
except ImportError:
    try:
        projroot = os.path.abspath(os.path.join(moduleroot, "..", "..", "..", ".."))
        sys.path.append(os.path.join(projroot, "Env"))
        from vless.gppy_vless.inf.getEnvVersion import getEnvVersion        # noqa

        versionXml = os.path.join(moduleroot, "gpVersion.xml")
        envpath = os.path.abspath(os.path.join(projroot, "Env", getEnvVersion(versionXml)))
        sys.path.append(envpath)

        from gppy.tools.memory.memoryusage import parseMemory
        from gppy.tools.memory.compiler.layout import Memory, set_log      # noqa
    except ImportError:
        print("Using outside of Env ?")


class ApplicationInfo(NamedTuple):
    mapfile: str
    heap_size_definition: int
    appname: str


class Logger(object):
    class DummyStdout(io.TextIOBase):
        def __init__(self, logger):
            self.log = logger

        def write(self, text):
            self.log(text)

        def flush(self):
            pass

    def __init__(self):
        '''Create a logger instance to store output on multiple targets'''
        self.outputs = []
        self.stdout = self.DummyStdout(self.log_raw)
        set_log(self.log)

    def add_output(self, output):
        if output not in self.outputs:
            self.outputs += [output]

    def remove_output(self, output):
        if output in self.outputs:
            self.outputs.remove(output)

    def log(self, s):
        for output in self.outputs:
            output.write("%s\n" % str(s))

    def log_raw(self, s):
        for output in self.outputs:
            output.write("%s" % str(s))


def parse_arg():
    '''Parse the optional arguments from input.'''
    usage = "usage: %prog [options]"
    parser = OptionParser(usage)
    parser.add_option("-l", "--logfile",
                      action="store",
                      dest="logFileName",
                      default=None,
                      help="Output log file name")
    parser.add_option("-s", "--separate",
                      action="store_true",
                      dest="separateLogFiles",
                      default=False,
                      help="Generate log file per application. Usefull for diffs.")
    parser.add_option("-a", "--add-application",
                      action="append",
                      dest="app_map_path",
                      default=[],
                      help="Point to the map file of an application to add it to "
                           "the memory comparison")
    parser.add_option("-1", "--only-this",
                      action="append",
                      dest="only_this",
                      default=[],
                      help="Point to the map file of an application to analyze")
    (options, _args) = parser.parse_args()

    return options


def parse_mapfiles(options) -> List[ApplicationInfo]:
    '''Read the map files for application infos'''
    applications = []
    for mapfile in options.only_this:
        applications += [
            ApplicationInfo(mapfile=mapfile,
                            heap_size_definition=Memory.HEAP_SIZE_REMAINING_RAM,
                            appname=os.path.basename(mapfile))
        ]

    if options.app_map_path:
        applications += [
            ApplicationInfo(mapfile=path,
                            heap_size_definition=Memory.HEAP_SIZE_REMAINING_RAM,
                            appname=f"{os.path.basename(path)}", )
            for path in options.app_map_path
        ]

    return applications


def parse_applications(applications, logger) -> Dict[str, Memory]:
    '''Extract the memory info from the application contents.'''

    infos: Dict[str, Memory] = {}

    for application in applications:
        app = os.path.split(application.mapfile)[1].split(".")[0]
        if os.path.isabs(application.mapfile) or os.path.exists(application.mapfile):
            mapfilePath = application.mapfile
        else:
            # Mapfiles taken from CHIP example /build/ output
            mapdir = os.path.join(moduleroot, "scripts", "mapfiles")
            mapfilePath = os.path.join(mapdir, application.mapfile)
        name = application.appname or app
        if name in infos:
            for i in range(10):
                tmpname = f"{name}_{i}"
                if tmpname not in infos:
                    name = tmpname
                    break

        infos[name] = parseMemory(mapfilePath, "GCC", application.heap_size_definition)

    return infos


def draw_table(infos: Dict[str, Memory]) -> Tuple[str, str]:
    """ render a table with a project specific grouped overview of memory use.
        Multiple applications can be given to compare sizes.

    :param infos: dictionary of analyzed configurations (memory class) to be printed side-by-side.
    """
    groups = [
        "APP",
        # "ZCL",
        "MAC",
        "Thread",
        "CHIP",
        "BLE",
        "Security",
        "OS/Libs",
    ]

    messages = ""
    table = PrettyTable()

    messages += ("-" * 40)
    messages += '\n'

    translation = {
        "APP": ["Application", "P345_Matter_DK_Endnodes"],
        "CHIP": ["P236_CHIP", "CHIP"],
        "BLE": ["BLE", "BLE Base"],
        "OS/Libs": ["Base",
                    "Other", "Library", "Thread/CHIP glue",
                    "Components/Qorvo/OS",
                    "Components/Qorvo/BaseUtils",
                    "Components/Qorvo/HAL_RF",
                    "Components/Qorvo/HAL_PLATFORM",
                    "Components/Qorvo/802_15_4",
                    "RTOS", "Debug"],
    }

    table.add_column("", ["Flash", "Flash+NVM+OTA", "Ram", "Stack",
                     "Heap", "Ram+Heap+Stack", "------"] + sorted(groups))
    table.align = "l"

    for app in sorted(infos.keys()):
        info: Memory = infos[app]
        for module in sorted(info.modules.keys()):
            info.modules[module].dump(info.segments)

        info_str = "%7u/%7u -   %6.2f %%"
        flash_info = info_str % (
            info.segments['Flash'].size,
            info.segments['Flash'].totalSize,
            100 * (1.0 * info.segments['Flash'].size) / (1.0 * info.segments['Flash'].totalSize)
        )
        ram_info = info_str % (
            info.segments['Ram'].size,
            info.segments['Ram'].totalSize,
            100 * (1.0 * info.segments['Ram'].size) / (1.0 * info.segments['Ram'].totalSize)
        )
        stack_info = info_str % (
            info.segments['Stack'].size,
            info.segments['Ram'].totalSize,
            100 * (1.0 * info.segments['Stack'].size) / (1.0 * info.segments['Ram'].totalSize)
        )
        heap_info = info_str % (
            info.segments['Heap'].size,
            info.segments['Ram'].totalSize,
            100 * (1.0 * info.segments['Heap'].size) / (1.0 * info.segments['Ram'].totalSize)
        )
        ram_with_stack_heap_info = info_str % (
            (info.segments['Heap'].size + info.segments['Ram'].size + info.segments['Stack'].size),
            info.segments['Ram'].totalSize,
            100 * (1.0 * info.segments['Heap'].size + info.segments['Ram'].size + info.segments['Stack'].size) /
            (1.0 * info.segments['Ram'].totalSize)
        )
        flash_with_nvm_ota_info = info_str % (
            info.segments['Flash'].size + info.segments["Nvm"].size + info.segments["Ota"].size,
            info.segments['Flash'].totalSize,
            100 * (1.0 * info.segments['Flash'].size + info.segments["Nvm"].size + info.segments["Ota"].size) /
            (1.0 * info.segments['Flash'].totalSize)
        )

        # Sanity check for changes in categories from memoryusage
        covered_categories = set(groups) - set(translation.keys())
        for trans in translation:
            # Add categories that are translated in a top-level categorization
            covered_categories = covered_categories | set(translation[trans])

        if not set(info.modules.keys()) <= covered_categories:
            print(set(info.modules.keys()) - covered_categories)
            raise RuntimeError("Not all modules are covered. Check:\n"
                               "Found in memoryoverview: %s\nCovered: %s" % (info.modules.keys(), covered_categories))

        groups_info = []

        # Loop over categories relevant for this project
        for group_name in sorted(groups):
            flash = 0
            ram = 0
            # Group all modules found in memoryusage
            for module in info.modules.keys():
                if group_name in translation:
                    if module in translation[group_name]:
                        messages += (f"Adding {module} to {group_name}\n")
                        flash += info.modules[module].size["Flash"]
                        ram += info.modules[module].size["Ram"]
                else:
                    # Name of our category matches the generic memusage name
                    if module == group_name:
                        flash += info.modules[module].size["Flash"]
                        ram += info.modules[module].size["Ram"]

            groups_info.append(f"Flash: {flash:6d} / RAM: {ram:5d}")

        table.add_column(app, [flash_info, flash_with_nvm_ota_info, ram_info, stack_info,
                               heap_info, ram_with_stack_heap_info, "------"] + groups_info)

    table.align = "l"
    return (messages, table)


''''''


def main():
    logger = Logger()
    log = logger.log

    options = parse_arg()

    if options.logFileName is not None:
        logfile = open(options.logFileName, "w", encoding='utf-8')
        logger.add_output(logfile)
    else:
        logger.add_output(sys.stdout)

    applications = parse_mapfiles(options)

    infos = parse_applications(applications, log)

    log("===========================")
    log("== Application overviews ==")
    log("===========================")
    for app, info in sorted(infos.items()):
        if options.separateLogFiles:
            separate_log_file = open(app + ".txt", "w")
            logger.add_output(separate_log_file)

        log("------------")
        log(f"== {app} ==")

        # Dump relevant memory usage info
        sys.stdout = logger.stdout
        info.dumpSizeInfo()
        info._dumpModules()
        sys.stdout = sys.__stdout__

        # Generate .csv files
        info._csvModules(file_name=app + ".csv")

        if options.separateLogFiles:
            separate_log_file.close()
            logger.remove_output(separate_log_file)

    # Draw the final table consolidating memory info
    (messages, table) = draw_table(infos)

    if options.logFileName and sys.stdout not in logger.outputs:
        logger.add_output(sys.stdout)

    log(messages)
    log(table)

    if options.logFileName is not None:
        logfile.close()


if __name__ == "__main__":
    main()
