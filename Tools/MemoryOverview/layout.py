#!/usr/bin/env python3

import os
import csv
from typing import Dict, List

try:
    from gppy.tools.memory.categorization.chip import CategorizationCHIP as categorization
except ImportError:
    try:
        from chip import CategorizationCHIP as categorization
    except ImportError:
        print("Using outside of Env ?")


global log
log = print


class Mempart(object):
    def __init__(self, name):
        self.name = name
        self.size = {}
        self.perc = {}
        for segment in ["Flash", "Ram", "Rom", "Default"]:
            self.size[segment] = 0
            self.perc[segment] = 0

    # Calculates the percentage within the given sizes
    def calcPerc(self, size):
        for segment in self.size.keys():
            if size[segment]:
                self.perc[segment] = self.size[segment] * 100 / size[segment]

    # Generic csv function
    def csv(self, level, segments):
        line = []
        line += [""] * level
        line += [self.name]
        line += [""] * (2 - level)

        for segment in sorted(self.size.keys()):
            if self.size[segment]:
                line += [""] * level
                line += [self.size[segment]]
                line += [""] * (2 - level)
                line += [""] * level
                line += [self.perc[segment]]
                line += [""] * (2 - level)
            elif segments[segment].size:
                line += [""] * 6

        return line

    # Generic dump function
    def dump(self, level, segments):
        global log
        line = " " * Memory.colSpacing * level
        line += self.name.ljust(Memory.colSpacing * (2 - level) + Memory.dumpSpacing)
        for segment in sorted(self.size.keys()):
            if self.size[segment]:
                line += " " * level
                line += "%6d (%3d%%)" % (self.size[segment], self.perc[segment])
                line += " " * (2 - level)
            elif segments[segment].size:
                line += " ".ljust(Memory.colSpacing)
        log(line)

# Module class


class Module(Mempart):
    def __init__(self, moduleName):
        Mempart.__init__(self, moduleName)
        self.components: Dict[str, 'Component'] = {}

    def addComponent(self, c: 'Component'):
        """ Adds a component to the module """
        if c.name not in self.components.keys():
            self.components[c.name] = c

    def calcSize(self, segments: List['Segment']):
        """ Calculates the size of the module based on the size of the contained components """
        for c in self.components.keys():
            self.components[c].calcSize(segments)
            for segment in self.size.keys():
                self.size[segment] += self.components[c].size[segment]
        self.calcPerc({key: segments[key].size for key in self.size})

    # Calculates the percentage this module takes within the given sizes (mostly of total used sizes)
    def calcPerc(self, size):
        Mempart.calcPerc(self, size)
        for c in self.components.keys():
            self.components[c].calcPerc(self.size)

    # Cleans the underlying components and removes the empty ones from the module
    def clean(self):
        for c in self.components:
            self.components[c].clean()
        self.components = {key: val for key, val in self.components.items() if val.objects}

    # Creates a csv compatible array
    def csv(self, segments):
        return Mempart.csv(self, 0, segments)

    # Dumps the module
    def dump(self, segments):
        Mempart.dump(self, 0, segments)
        for c in sorted(self.components.keys()):
            self.components[c].dump(segments)

# Component class = group of objects


class Component(Mempart):
    def __init__(self, componentName):
        Mempart.__init__(self, componentName)
        self.objects = {}

    # Adds an object to the component
    def addObject(self, o):
        if o.name not in self.objects.keys():
            self.objects[o.name] = o

    # Calculates the size of the component based on the size of the collected objects
    def calcSize(self, segments):
        for o in self.objects.keys():
            self.objects[o].calcSize(segments)
            for segment in self.size.keys():
                self.size[segment] += self.objects[o].size[segment]

    # Calculates the percentage this component takes within the given sizes (mostly of the module above)
    def calcPerc(self, size):
        Mempart.calcPerc(self, size)
        for o in self.objects.keys():
            self.objects[o].calcPerc(self.size)

    # Cleans out the underlying objects and then removes all empty objects
    def clean(self):
        for o in self.objects:
            self.objects[o].clean()
        self.objects = {key: val for key, val in self.objects.items() if val.sections}

    # Creates a csv compatible array
    def csv(self, segments):
        return Mempart.csv(self, 1, segments)

    # Dumps the component
    def dump(self, segments):
        Mempart.dump(self, 1, segments)
        for o in sorted(self.objects.keys()):
            self.objects[o].dump(segments)

# Object class


class obj(Mempart):
    def __init__(self, objectName):
        Mempart.__init__(self, objectName)
        self.sections = {}

    # Adds a section with name and size to the object
    def addSection(self, name, size):
        if name not in self.sections.keys():
            self.sections[name] = size
        else:
            self.sections[name] += size

    # Calculates the size of the object based on the section sizes for the object
    def calcSize(self, segments):
        for section, size in self.sections.items():
            for segment in self.size.keys():
                if section in segments[segment].sections:
                    self.size[segment] += size

    # Calculates the percentage this object takes within the given sizes (mostly of the component above)
    def calcPerc(self, size):
        Mempart.calcPerc(self, size)

    # Cleans out empty sections
    def clean(self):
        self.sections = {key: val for key, val in self.sections.items() if val > 0}

    # Creates a csv compatible array
    def csv(self, segments):
        return Mempart.csv(self, 2, segments)

    # Dumps the object
    def dump(self, segments):
        Mempart.dump(self, 2, segments)

# Segment class


class Segment(object):
    def __init__(self, name):
        self.name = name

        # List of names by which this memory segment can be addressed
        self.refNames = [name]

        # Used Size and total available size
        self.size = 0
        self.totalSize = 0

        # Start and end address in the addressing space (can be multiple parts), append as tuples
        self.address = []

        # Sections in the segment
        self.sections = {}

    # Adds a part of memory to the segment
    def addPart(self, startAddress, endAddress):
        self.address.append((startAddress, endAddress))
        self.totalSize += endAddress - startAddress

    # Adds a section to the segment, must be section class object
    def addSection(self, section):
        if section.name not in self.sections.keys():
            self.sections[section.name] = section

    # Adds size to a section by name, if the section does not yet exist, it is created in the process
    def addSectionPart(self, name, size):
        if name not in self.sections.keys():
            self.sections[name] = Section(name, size=size)
        else:
            self.sections[name].size += size

    # Adds a startaddress to a section by name, if the section does not yet exist, it is created in the process
    def addSectionAddress(self, name, address):
        if name not in self.sections.keys():
            self.sections[name] = Section(name, address=address)
        else:
            self.sections[name].address = address

    # Calculates the size of the segment based on the sizes of the sections
    def calcSize(self):
        self.size = sum([self.sections[section].size for section in self.sections])

    # Cleans out empty sections
    def clean(self):
        self.sections = {key: val for key, val in self.sections.items() if val.size}

    # Dumps the segment
    def dump(self, force=False):
        global log
        if (self.size or force):
            line = self.name.ljust(2 * Memory.colSpacing) + "%8d" % self.size
            if self.totalSize:
                line += " / %8d " % self.totalSize
                line += " (%3.1f%%) " % (self.size * 1.0 / self.totalSize * 100.0)
                line += " (free: %8d)" % (self.totalSize - self.size)
            log(line)

# Section class


class Section(object):
    def __init__(self, name, size=0, address=None):
        self.name = name

        # Start address and size
        self.address = address
        self.size = size

        # Boolean to denote const data -> used in iar parsing
        self.constData = False

    def dump(self, segment):
        global log
        line = self.name.ljust(Memory.dumpSpacing) + str(self.size).ljust(Memory.colSpacing)
        if self.constData:
            line += ("as " + segment + " - const data").ljust(2 * Memory.colSpacing)
        else:
            line += ("as " + segment).ljust(2 * Memory.colSpacing)
        if (self.address is not None):
            line += " [ 0x" + '{:010x}'.format(self.address) + " - 0x" + \
                '{:010x}'.format(self.address + self.size) + " ]"
        log(line)


class Memory(object):
    HEAP_SIZE_FIXED = 0
    HEAP_SIZE_REMAINING_RAM = 1

    # List of sections that will be used when outputting the objects to the prompt
    common_sections = ["text", "rodata", "data", "bss", "bss_uc", "rom", "romjumptable", "flashjumptable",
                       "datajumptable", "lower_ram_retain", "lower_ram_retain_gpmicro_accessible"]

    # Common used names and spacings within the script
    heap_section = "HEAP"
    stack_section = "CSTACK"
    linker_object = "linker"
    gaps_object = "LINKER_GAPS"
    dumpSpacing = 50
    colSpacing = 15

    # The order to list memory parts
    printorder = ["Flash", "Nvm", "Ota", "Rom", "Ram", "Stack", "Heap", "Default", "Unknown"]

    # Classificaton certain named sections
    flashCommonSections = ["text", "vpp", "version_sw", "mw", "rt_flash", "rodata",
                           "flashjumptable", "datajumptable", "m_flashjumptable", "gpNvm",
                           "native_user_license", "OTA"]
    ramCommonSections = ["lower_ram_retain", "lower_ram", "higher_ram_noretain",
                         "lower_ram_retain_gpmicro_accessible", "bss", "bss_uc", "data", "events", "heap",
                         "ret_hw", "ret_sw", "crc", "mw_crc", "pufr", "pkey",
                         "rom_m_bss", "rom_m_data"]
    romCommonSections = ["rom", "romjumptable", "rom_m_text"]

    def __init__(self, mapfile, heap_size_definition):
        # Store the mapfile and its path
        self.mapfile = mapfile
        self.path = os.path.split(mapfile)[0]

        # Init the dictionaries of objects, modules and segments
        self.objects = {}
        self.modules = {}
        self.segments = {}

        # Init all of the segments
        self.segments["Flash"] = Segment("Flash")
        self.segments["Ram"] = Segment("Ram")
        self.segments["Rom"] = Segment("Rom")
        self.segments["Stack"] = Segment("Stack")
        self.segments["Heap"] = Segment("Heap")
        self.segments["Nvm"] = Segment("Nvm")
        self.segments["Ota"] = Segment("Ota")
        self.segments["Default"] = Segment("Default")
        self.segments["Unknown"] = Segment("Unknown")

        for name in Memory.flashCommonSections:
            self.segments["Flash"].addSection(Section(name))
        for name in Memory.ramCommonSections:
            self.segments["Ram"].addSection(Section(name))
        for name in Memory.romCommonSections:
            self.segments["Rom"].addSection(Section(name))

        self.lower_retain_size = 0
        self.higher_retain_size = 0
        # Allow further customization - heap approach/category additions
        self.heap_size_definition = heap_size_definition

    def parseMapFile(self):
        print("Need implementation to population all sections/object info")

    # Adds a section with name and size to an object in the dictionary of the memory
    # This represents the space this component uses within this section
    def _addObjInfo(self, objectFile, section, size):
        if objectFile not in self.objects.keys():
            self.objects[objectFile] = obj(objectFile)
        self.objects[objectFile].addSection(section, int(size, 16))

    def _getDefaultCategory(self, moduleName, componentName, objectFile):
        for key in categorization.order:
            if moduleName in categorization.default[key]:
                if len(categorization.default[key][moduleName]):
                    for c in categorization.default[key][moduleName]:
                        if componentName.startswith(c):
                            if len(categorization.default[key][moduleName][c]):
                                for k in categorization.default[key][moduleName][c]:
                                    if objectFile.startswith(k):
                                        return key
                            else:
                                return key
                else:
                    return key
        return moduleName

    def _getArchiveCategory(self, archiveName, objectFile):
        for archivename_start in categorization.combined_archives:
            if archiveName.startswith(archivename_start):
                module_by_filename = self._getObjectCategory(objectFile)
                if module_by_filename != "Other":
                    return module_by_filename
        for key in categorization.order:
            for archivename_start in categorization.archives[key]:
                if archiveName.startswith(archivename_start):
                    if len(categorization.archives[key][archivename_start]):
                        for objectname_start in categorization.archives[key][archivename_start]:
                            if objectFile.startswith(objectname_start):
                                return key
                    else:
                        return key
        return "Other"

    def _getObjectCategory(self, objectFile):
        for key in categorization.order:
            for o in categorization.files[key]:
                if objectFile.startswith(o):
                    return key
        return "Other"

    # Adds an object to a component within a module. This will categorize according to
    # the dictionary 'categories' in the memory class
    def _addObj2Comp(self, moduleName, componentName, objectFile):
        if moduleName not in self.modules:
            self.modules[moduleName] = Module(moduleName)
        self.modules[moduleName].addComponent(Component(componentName))
        if objectFile not in self.objects.keys():
            self.objects[objectFile] = obj(objectFile)
        self.modules[moduleName].components[componentName].addObject(self.objects[objectFile])

    def _calculateSizes(self):

        # Calculate the size of each segment
        for s in self.segments:
            self.segments[s].calcSize()

        # Calculate the sizes for the modules
        for m in self.modules:
            self.modules[m].calcSize(self.segments)

        # If a heap is used as remaining RAM, it can not show up in the map-file
        if self.heap_size_definition is self.HEAP_SIZE_REMAINING_RAM:
            self.segments["Heap"].size = self.segments["Ram"].totalSize - \
                self.segments["Ram"].size - self.segments["Stack"].size

    # Clean all dictionaries, remove empty sections, objects and modules
    def _clean(self):
        for s in self.segments:
            self.segments[s].clean()
        for o in self.objects:
            self.objects[o].sections = {key: val for key, val in self.objects[o].sections.items() if val > 0}
        self.objects = {key: val for key, val in self.objects.items() if val.sections}
        for m in self.modules:
            self.modules[m].clean()
        self.modules = {key: val for key, val in self.modules.items() if val.components}

    # Dumps the modules showing all modules, the containted components and the underlying
    # objects with their sizes and percentages
    def _dumpModules(self):
        top = "*" * self.dumpSpacing
        print(top)
        top = "Dumping Modules"
        print(top)
        top = "*" * self.dumpSpacing
        print(top)

        top = "Module".ljust(self.colSpacing)
        top += "Component".ljust(self.colSpacing)
        top += "Object".ljust(self.dumpSpacing)
        for s in ["Flash", "Ram", "Rom", "Default"]:
            if self.segments[s].size:
                top += s.ljust(self.colSpacing)
        print(top)

        for module in sorted(self.modules.keys()):
            self.modules[module].dump(self.segments)
        print("\n")

    def _csvModules(self, file_name="memory_modules.csv"):
        """Write module information to a csv file.

        :param file_name: csv full file name for dump. Defaults to 'memory_modules.csv'.
        """
        with open(file_name, 'w') as csvfile:
            writer = csv.writer(csvfile, delimiter=',', quoting=csv.QUOTE_MINIMAL)

            # Header row
            top = ["Module", "Component", "Object"]
            for s in ["Flash", "Ram", "Rom"]:
                if self.segments[s].size:
                    top += [s, "", "", "", "", ""]
            writer.writerow(top)

            # Content
            for m in sorted(self.modules):
                writer.writerow(self.modules[m].csv(self.segments))
                for c in sorted(self.modules[m].components):
                    writer.writerow(self.modules[m].components[c].csv(self.segments))
                    for o in sorted(self.modules[m].components[c].objects):
                        writer.writerow(self.modules[m].components[c].objects[o].csv(self.segments))

    # Dumps the objects and lists the size each of the represent in the common sections
    def _dumpObjects(self):

        # Limit overview to common sections
        section_headers = sorted(set(Memory.common_sections) & set(self.segments["Flash"].sections.keys()))
        section_headers += sorted(set(Memory.common_sections) & set(self.segments["Ram"].sections.keys()))

        # Dump some ascii
        top = "*" * self.dumpSpacing
        for header in section_headers:
            top += "*" * self.colSpacing
        print(top)
        top = "Dumping Objects"
        print(top)
        top = "*" * self.dumpSpacing
        for header in section_headers:
            top += "*" * self.colSpacing
        print(top)

        # Limit overview to common sections
        # warn("WARNING: This table only contains common sections")

        # Dump header
        top = "object".ljust(self.dumpSpacing)
        for header in section_headers:
            if len(header) >= self.colSpacing:
                header = header[:self.colSpacing - 6] + ".." + header[-3:]
            top += header.ljust(self.colSpacing)
        print(top)

        # Dump per object info
        for o in sorted(self.objects.keys()):
            line = self.objects[o].name[:self.dumpSpacing - 2].ljust(self.dumpSpacing)
            content = False
            for section in section_headers:
                if section in self.objects[o].sections:
                    line += ("%d" % self.objects[o].sections[section]).ljust(self.colSpacing)
                    if self.objects[o].sections[section]:
                        content = True
                else:
                    line += "0".ljust(self.colSpacing)
            if content:
                print(line.strip())
        print("")

    # Dumps the sections in each of the segments
    def _dumpWithSectionMappings(self):

        # Dump some ascii
        top = "*" * (self.dumpSpacing + 4 * self.colSpacing)
        print(top)
        top = "Dumping Sections"
        print(top)
        top = "*" * (self.dumpSpacing + 4 * self.colSpacing)
        print(top)

        for s in Memory.printorder:
            section_list = []
            for sec in self.segments[s].sections:
                if self.segments[s].sections[sec].address:
                    section_list.append(self.segments[s].sections[sec])
            for section in section_list:
                section.dump(s)
        print("\n")

    def dumpSizeInfo(self):
        print("*" * (5 * self.colSpacing))
        print("Memory Overview")
        print("*" * (5 * self.colSpacing))

        for segment_name in Memory.printorder:
            self.segments[segment_name].dump()
            # Ordering expected - Nvm, Ota
            # Dump joint info after both segments
            if segment_name == "Ota":
                if self.segments["Nvm"].size or self.segments["Ota"].size:
                    flash_total = self.segments["Flash"].size
                    flash_total += self.segments["Nvm"].size if self.segments["Nvm"].size else 0
                    flash_total += self.segments["Ota"].size if self.segments["Ota"].size else 0

                    extra_flash_consumers = ["Nvm"] if self.segments["Nvm"].size else []
                    extra_flash_consumers += ["Ota"] if self.segments["Ota"].size else []
                    extra_flash_consumers = "/".join(extra_flash_consumers)

                    line = f"Flash with {extra_flash_consumers}".ljust(2 * self.colSpacing) + "%8d" % (flash_total)
                    if self.segments["Flash"].totalSize:
                        line += " / %8d " % self.segments["Flash"].totalSize
                        line += " (%3.1f%%) " % (flash_total * 1.0 / self.segments["Flash"].totalSize * 100.0)
                        line += " (free: %8d)" % (self.segments["Flash"].totalSize - flash_total)
                    print(line)
                print("*" * (5 * self.colSpacing))
            if segment_name == "Heap":
                # Dumping joint RAM overviews after last RAM segment
                if (self.segments["Stack"].size + self.segments["Heap"].size):
                    ram_total = self.segments["Ram"].size + self.segments["Stack"].size + self.segments["Heap"].size
                    line = "RAM with stack/heap".ljust(2 * self.colSpacing) + "%8d" % (ram_total)
                    if self.segments["Ram"].totalSize:
                        line += " / %8d " % self.segments["Ram"].totalSize
                        line += " (%3.1f%%) " % (ram_total * 1.0 / self.segments["Ram"].totalSize * 100.0)
                        line += " (free: %8d)" % (self.segments["Ram"].totalSize - ram_total)
                    print(line)
                if self.lower_retain_size or self.higher_retain_size:
                    print("RAM (retained):".ljust(2 * self.colSpacing) + "%8d" %
                          (self.lower_retain_size + self.higher_retain_size))
                    print("  Lower RAM (retained):".ljust(2 * self.colSpacing) + "%8d" % (self.lower_retain_size))
                    print("  Higher RAM (retained):".ljust(2 * self.colSpacing) + "%8d" % (self.higher_retain_size))

    def dump(self):
        for seg in self.segments:
            for sec in self.segments[seg].sections:
                print(self.segments[seg].sections[sec].name.ljust(
                    Memory.dumpSpacing), self.segments[seg].sections[sec].size)

        self.dumpSizeInfo()


class Parts(object):
    ignoreFolders = categorization.ignore_folders


def importModule(compiler):
    try:
        py_file = "gppy.tools.memory.compiler.%s" % compiler.lower()
        module = __import__(py_file, globals(), locals(), ['CompilerInfo'], 0)
    except ImportError:
        try:
            py_file = "%s" % compiler.lower()
            module = __import__(py_file, globals(), locals(), ['CompilerInfo'], 0)
        except ImportError:
            raise RuntimeError("CompilerInfo not found")
    return module


def set_log(f):
    global log
    log = f


def warn(s):
    # global log
    # log(s)
    print(s)
