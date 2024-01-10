#!/usr/bin/env python3

import os
import re

try:
    from gppy.tools.memory.compiler.layout import Parts, Memory, Section, warn
except ImportError:
    try:
        from layout import Parts, Memory, Section, warn
    except ImportError:
        print("Using outside of Env ?")


class CompilerInfo(Memory):
    def __init__(self, mapfile, heap_size_definition):
        Memory.__init__(self, mapfile, heap_size_definition)

        # Remove preset sections, this will be filled when parsing the map file
        self.segments["Flash"].sections = {}
        self.segments["Ram"].sections = {}
        self.segments["Rom"].sections = {}

        # Set names used for segments
        self.segments["Flash"].refNames += ["FLASH", "FLASH2", "SD_FLASH", "FDS_FLASH", "OT_DATA_FLASH", "CODE_NRT"]
        self.segments["Ram"].refNames += ["SYSRAM", "UCRAM", "RAM", "SD_RAM", "SRAM", "AKRAM_NRT"]
        self.segments["Rom"].refNames += ["ROM"]
        self.segments["Default"].refNames += ["*default*"]
        self.segments["Stack"].refNames += []
        self.segments["Heap"].refNames += ["heap"]
        self.segments["Nvm"].refNames += ["gpNvm"]
        self.segments["Ota"].refNames += ["OTA", "JTOTA"]

        # SiLabs specific
        self.segments["Stack"].refNames += ["stack_dummy"]
        self.segments["Nvm"].refNames += ["nvm_dummy"]

        # version number regular expression
        self.versionNumber_re = re.compile(r"v(latest|less|\d\.\d\.\d\.\d)")

    def _addSectionInfo(self, name, address, size):
        if name in self.segments["Stack"].refNames:
            self.segments["Stack"].addSection(Section(name, size, address))
        elif name in self.segments["Heap"].refNames:
            self.segments["Heap"].addSection(Section(name, size, address))
        elif name in self.segments["Nvm"].refNames:
            self.segments["Nvm"].addSection(Section(name, size, address))
        elif name in self.segments["Ota"].refNames:
            self.segments["Ota"].addSection(Section(name, size, address))
        else:
            for s in Memory.printorder:
                for (start, end) in self.segments[s].address:
                    if (address >= start) and (address + size <= end):
                        self.segments[s].addSection(Section(name, size, address))
                        return

    def processPath(self, fullName):
        fullName = fullName.replace(self.path + os.sep, "")
        parts = fullName.split(os.sep)
        parts = [part for part in parts if part not in Parts.ignoreFolders]
        moduleName = None
        componentName = None
        objectFile = parts[-1]
        if parts[0] == "Work" or parts[0] == "gpHub":
            moduleName = parts[1]
            componentName = parts[-2]
            moduleName = self._getDefaultCategory(moduleName, componentName, objectFile)
        else:
            # Scrolling in path parts
            for index in range(0, len(parts) - 1):
                # Assuming a project version is part of the path
                if self.versionNumber_re.match(parts[index + 1]):
                    moduleName = parts[index]
                    componentName = parts[index + 2]
                    moduleName = self._getDefaultCategory(moduleName, componentName, objectFile)
                    break
            if (moduleName is None):
                moduleName = self._getObjectCategory(objectFile)
                componentName = "Unknown"
        return moduleName, componentName, objectFile

    def processArchive(self, fullName):
        return "Other", fullName.split(os.sep)[-1]

    def _initChapterArchive(self):
        # Archive regular expression:
        self.Archive_Entry_re = re.compile(r"^([\w/\.\-\+]+\.a)\(([\w\-\.]+)\.o(S|bj)?\)")

    def _parseChapterArchive(self, line):

        # Archive entry
        result = self.Archive_Entry_re.match(line)
        if (result is not None):
            fullName = result.group(1)
            componentName = os.path.split(fullName)[-1]
            objectFile = result.group(2)
            moduleName = self._getArchiveCategory(componentName, objectFile)
            self._addObj2Comp(moduleName, componentName, objectFile)
            return

    def _initChapterMemory(self):
        # Memory Configuration
        # text             0x00000000         0x00020000         xr
        self.Memory_Line_re = re.compile(r"([a-zA-Z_\*]+) +(0x[0-9a-fA-F]+) +(0x[0-9a-fA-F]+)")

    def _parseChapterMemory(self, line):
        # Memory Sizes
        result = self.Memory_Line_re.match(line)
        if result is not None:
            name = result.group(1)
            start = int(result.group(2), 16)
            size = int(result.group(3), 16)

            # Store the startaddress and the size of each memorytype
            for key, segment in self.segments.items():
                if name in segment.refNames:
                    self.segments[key].addPart(start, start + size)

    def _initChapterLinker(self):

        self.slower_retain = 0
        self.lower_retain_size = 0
        self.higher_retain_size = 0

        self.currentSection = ""
        self.currentAddress = None

        self.Linker_Module_Entry_re = re.compile(r"LOAD ([\w/\.\-\+]+)\.[s?o]")
        self.Linker_Section_Size_re = re.compile(r"^\.?([\w]+)[a-zA-Z0-9_\.]* +([0-9a-fx]+) +([0-9a-fx]+)")
        self.Linker_Section_Size_Header_re = re.compile(r"^\.?([\w]+)[a-zA-Z0-9_\.]* *$")
        self.Linker_Section_Size_Body_re = re.compile(r"^ +(0x[0-9a-fA-F]+) +(0x[0-9a-fA-F]+) *$")
        self.Linker_Object_Size_re = re.compile(
            r"^ \.([\w]+)[\w\.]* +(0x[0-9a-fA-F]+) +(0x[0-9a-fA-F]+) ([\w/\-\.\+]+)\.o$")
        self.Linker_Object_Size_Common_re = re.compile(
            r"^ ([COMN]+) +(0x[0-9a-fA-F]+) +(0x[0-9a-fA-F]+) ([/\w\-\.\+]+)\.o$")
        self.Linker_Object_Size_Complex_re = re.compile(
            r"^ \.([\w]+)[\w\.]* +(0x[0-9a-fA-F]+) +(0x[0-9a-fA-F]+) ([/\w\-\.\+]+\.a)\(([\w\-\.\+]+)\.o(S|bj)?\)$")
        self.Linker_Object_Size_Comn_Complex_re = re.compile(
            r"^ ([COMN]+) +(0x[0-9a-fA-F]+) +(0x[0-9a-fA-F]+) ([/\w\-\.\+]+\.a)\(([\w\-\.\+]+)\.o(S|bj)?\)$")
        self.Linker_Object_Size_Header_re = re.compile(r"^ \.([\w]+)[\w\.]* *$")
        self.Linker_Object_Size_Body_re = re.compile(r"^ +(0x[0-9a-fA-F]+) +(0x[0-9a-fA-F]+) ([\w/\-\.\+]+)\.o$")
        self.Linker_Object_Size_Complex_Body_re = re.compile(
            r"^ +(0x[0-9a-fA-F]+) +(0x[0-9a-fA-F]+) ([\w/\-\.\+]+\.a)\(([\w\-\.]+)\.o(S|bj)?\)$")
        self.Linker_Section_Fill_Mask_re = re.compile(r"^ +(0x[0-9a-fA-F]+) +(0x[0-9a-fA-F]+) BYTE (0x[0-9a-fA-F]+)")
        self.Linker_Section_Fill_Size_re = re.compile(r"^ \*fill\* +(0x[0-9a-fA-F]+) +(0x[0-9a-fA-F]+) ")

        Linker_Stack_re = re.compile(r"[ \t]+([\w\.]+)[ \t]+stack_size (.+)*")
        Linker_Stack_nrf_re = re.compile(r"[ \t]+.stack[ \t]+[\w\.]+[ \t]+([\w\.]+)")
        self.Linker_Stack_Size_re = [Linker_Stack_re, Linker_Stack_nrf_re]
        self.Linker_Stack_Start_re = re.compile(r"[ \t]+(0x[0-9a-fA-F]+)[ \t]*_sstack = .*$")
        self.Linker_Heap_Size_re = re.compile(r"[ \t]*.heap[ \t]+[\w\.]+[ \t]+([\w\.]+)")
        self.Linker_Ram_sretain_re = re.compile(r"[ \t]+0x([0-9a-fA-F]+)[ \t]*_sretain = .*$")
        self.Linker_Ram_eretain_re = re.compile(r"[ \t]+0x([0-9a-fA-F]+)[ \t]*_eretain =.*$")
        self.Linker_Ram_slower_retain_re = re.compile(r"[ \t]+0x([0-9a-fA-F]+)[ \t]*_slower_retain = .*$")
        self.Linker_Ram_elower_retain_size_re = re.compile(r"[ \t]+0x([0-9a-fA-F]+)[ \t]*__lowerram_retain_size = .*$")
        self.Linker_Ram_higher_retain_size_re = re.compile(r"[ \t]+0x([0-9a-fA-F]+)[ \t]*__higherram_retain_size = .*$")
        self.Linker_Ram_appuc_retain_size_re = re.compile(r"[ \t]+0x([0-9a-fA-F]+)[ \t]*__appuc_ram_retain_length = .*$")
        self.Linker_Ram_sys_retain_size_re = re.compile(r"[ \t]+0x([0-9a-fA-F]+)[ \t]*_sysram_length = .*$")

        # Test regular expressions with reference text
        # Section with multiple lines, body content
        SectionMultiLineBody_reference = "                0x0000000004000840      0xae4"
        assert self.Linker_Section_Size_Body_re.match(SectionMultiLineBody_reference)

        # Stack size - lookup linkerscript symbol (K8 linkerscripts - see template)
        stackSize_reference = "                0x000077cc                stack_size = (_estack - _sstack)"
        assert Linker_Stack_re.match(stackSize_reference)
        # Nordic stack/heap references
        stackSize_reference_nRf = " .stack         0x0000000020019b90     0x2000 /objs/gcc_startup_nrf52840.S.o"
        assert Linker_Stack_nrf_re.match(stackSize_reference_nRf)

        # Retention markers
        # XP3002
        sretain_reference = "                0x0000000020008000                _sretain = 0x2000800"
        assert self.Linker_Ram_sretain_re.match(sretain_reference)
        eretain_reference = "                0x000000002000b988                _eretain = ."
        assert self.Linker_Ram_eretain_re.match(eretain_reference)

        # K8.1
        slower_retain_reference = "                0x20008000                _slower_retain = ORIGIN (SYSRAM)"
        assert self.Linker_Ram_slower_retain_re.match(slower_retain_reference)

        # XP3002 / K8.1
        elower_retain_size_reference = "        0x0000000000003988                __lowerram_retain_size = (_eretain < eSYSRAM)?(_eretain - _sretain):(eSYSRAM - _sretain)"
        assert self.Linker_Ram_elower_retain_size_re.match(elower_retain_size_reference)

        higher_retain_size_reference = "        0x0000000000000000                __higherram_retain_size = (_eretain < eSYSRAM)?0x0:(_eretain - __lowerram_retain_size)"
        assert self.Linker_Ram_higher_retain_size_re.match(higher_retain_size_reference)

        heapSize_reference = ".heap           0x0000000020015dac     0xa000"
        heapSize_reference_nRf = ".heap          0x0000000020019b90     0xa000 /objs/gcc_startup_nrf52840.S.o"
        heapSize_reference_SiLabs = " .heap          0x0000000000000000      0xc00 /objs/startup_efr32mg12p.c.o"
        assert self.Linker_Heap_Size_re.match(heapSize_reference)
        assert self.Linker_Heap_Size_re.match(heapSize_reference_nRf)
        assert self.Linker_Heap_Size_re.match(heapSize_reference_SiLabs)

        # XP4001
        appuc_retain_size_reference = "                0x00000b14                        __appuc_ram_retain_length = (_eretain - _sretain)"
        sysram_retain_size_reference = "                0x000021dc                        _sysram_length = (sysram_end - _sysram_start)"
        assert self.Linker_Ram_appuc_retain_size_re.match(appuc_retain_size_reference)
        assert self.Linker_Ram_sys_retain_size_re.match(sysram_retain_size_reference)

    def _parseChapterLinker(self, line):

        # Parse module entry
        result = self.Linker_Module_Entry_re.match(line)
        if (result is not None):
            fullName = result.group(1)
            moduleName, componentName, objectFile = self.processPath(fullName)
            self._addObj2Comp(moduleName, componentName, objectFile)
            return

        # Find section size: single line case
        result = self.Linker_Section_Size_re.match(line)
        if (result is not None):
            address = int(result.group(2), 16)
            name = result.group(1)
            if (address != 0) or (name in Memory.ramCommonSections) or (name in Memory.romCommonSections) or (name in Memory.flashCommonSections):
                self.currentAddress = address
                self.currentSection = name
                size = int(result.group(3), 16)
                self._addSectionInfo(self.currentSection, address, size)
            else:
                self.currentSection = None
            return

        # Find section size: multi line case
        result = self.Linker_Section_Size_Header_re.match(line)
        if result is not None:
            self.currentSection = result.group(1)
            return
        result = self.Linker_Section_Size_Body_re.match(line)
        if result is not None:
            address = int(result.group(1), 16)
            name = self.currentSection
            if (address != 0) or (name in Memory.ramCommonSections) or (name in Memory.romCommonSections) or (name in Memory.flashCommonSections):
                self.currentAddress = address
                size = int(result.group(2), 16)
                self._addSectionInfo(self.currentSection, address, size)
            else:
                self.currentSection = None
            return

        if (self.currentSection is not None):
            # Find Object size: Single Line case
            result = self.Linker_Object_Size_re.match(line)
            if (result is not None):
                address = int(result.group(2), 16)
                size = result.group(3)
                fullName = result.group(4).strip()
                objectFile = os.path.split(fullName)[-1]
                if (self.currentAddress != address):
                    warn("WARNING: Address anomaly @ " + hex(address) + " - " + objectFile)
                self.currentAddress = address + int(size, 16)
                if objectFile not in self.objects:
                    moduleName, componentName, objectFile = self.processPath(fullName)
                    self._addObj2Comp(moduleName, componentName, objectFile)
                self._addObjInfo(objectFile, self.currentSection, size)
                return

            # Find Object size: COMMON entry case
            result = self.Linker_Object_Size_Common_re.match(line)
            if (result is not None):
                address = int(result.group(2), 16)
                size = result.group(3)
                objectFile = result.group(4).strip()
                objectFile = os.path.split(objectFile)[-1]
                if (self.currentAddress != address):
                    warn("WARNING: Address anomaly @ " + hex(address) + " - " + objectFile)
                self.currentAddress = address + int(size, 16)
                if objectFile not in self.objects:
                    moduleName, componentName, objectFile = self.processPath(fullName)
                    self._addObj2Comp(moduleName, componentName, objectFile)
                self._addObjInfo(objectFile, self.currentSection, size)
                return

            # Find Object size: Complex case for archives
            result = self.Linker_Object_Size_Complex_re.match(line)
            if (result is not None):
                address = int(result.group(2), 16)
                size = result.group(3)
                objectFile = result.group(5)
                if (self.currentAddress != address):
                    warn("WARNING: Address anomaly @ " + hex(address) + " - " + objectFile)
                self.currentAddress = address + int(size, 16)
                if objectFile not in self.objects:
                    archiveName = result.group(4).split(os.sep)[-1]
                    moduleName = self._getArchiveCategory(archiveName, objectFile)
                    self._addObj2Comp(moduleName, archiveName, objectFile)
                self._addObjInfo(objectFile, self.currentSection, size)
                return

            # Find Object size: COMMON entry with complex archive structure
            result = self.Linker_Object_Size_Comn_Complex_re.match(line)
            if (result is not None):
                address = int(result.group(2), 16)
                size = result.group(3)
                objectFile = result.group(5)
                if (self.currentAddress != address):
                    warn("WARNING: Address anomaly @ " + hex(address) + " - " + objectFile)
                self.currentAddress = address + int(size, 16)
                if objectFile not in self.objects:
                    archiveName = result.group(4).split(os.sep)[-1]
                    moduleName = self._getArchiveCategory(archiveName, objectFile)
                    self._addObj2Comp(moduleName, archiveName, objectFile)
                self._addObjInfo(objectFile, self.currentSection, size)
                return

            # Find Object size: multi line case
            result = self.Linker_Object_Size_Header_re.match(line)
            if result is not None:
                #self.currentSection = result.group(1)
                return
            result = self.Linker_Object_Size_Body_re.match(line)
            if result is not None:
                address = int(result.group(1), 16)
                size = result.group(2)
                fullName = result.group(3).strip()
                objectFile = os.path.split(fullName)[-1]
                if (self.currentAddress != address):
                    warn("WARNING: Address anomaly @ " + hex(address) + " - " + objectFile)
                self.currentAddress = address + int(size, 16)
                if objectFile not in self.objects:
                    moduleName, componentName, objectFile = self.processPath(fullName)
                    self._addObj2Comp(moduleName, componentName, objectFile)
                self._addObjInfo(objectFile, self.currentSection, size)
                return
            result = self.Linker_Object_Size_Complex_Body_re.match(line)
            if result is not None:
                address = int(result.group(1), 16)
                size = result.group(2)
                objectFile = result.group(4).strip()
                if (self.currentAddress != address):
                    warn("WARNING: Address anomaly @ " + hex(address) + " - " + objectFile)
                self.currentAddress = address + int(size, 16)
                if objectFile not in self.objects:
                    archiveName = result.group(3).split(os.sep)[-1]
                    moduleName = self._getArchiveCategory(archiveName, objectFile)
                    self._addObj2Comp(moduleName, archiveName, objectFile)
                self._addObjInfo(objectFile, self.currentSection, size)
                return

            # Find filler size
            result = self.Linker_Section_Fill_Mask_re.match(line)
            if result is not None:
                address = int(result.group(1), 16)
                size = result.group(2)
                if (self.currentAddress != address):
                    warn("WARNING: Address anomaly @ " + hex(address) + " - gap")
                self.currentAddress = address + int(size, 16)
                self._addObjInfo(Memory.gaps_object, self.currentSection, size)

            result = self.Linker_Section_Fill_Size_re.match(line)
            if result is not None:
                address = int(result.group(1), 16)
                size = result.group(2)
                if (self.currentAddress != address):
                    warn("WARNING: Address anomaly @ " + hex(address) + " - gap")
                self.currentAddress = address + int(size, 16)
                self._addObjInfo(Memory.gaps_object, self.currentSection, size)

        # RAM parts
        result = self.Linker_Ram_slower_retain_re.match(line)
        if (result is not None):
            self.slower_retain = int(result.group(1), 16)
            return
        result = self.Linker_Ram_elower_retain_size_re.match(line)
        if (result is not None):
            self.lower_retain_size = int(result.group(1), 16)
            return
        result = self.Linker_Ram_higher_retain_size_re.match(line)
        if (result is not None):
            self.higher_retain_size = int(result.group(1), 16)
            return
        # xp400x has sysram and appuc ram
        result = self.Linker_Ram_sys_retain_size_re.match(line)
        if (result is not None):
            self.lower_retain_size = int(result.group(1), 16)
            return
        result = self.Linker_Ram_appuc_retain_size_re.match(line)
        if (result is not None):
            self.higher_retain_size = int(result.group(1), 16)
            return

        # Heap size
        result = self.Linker_Heap_Size_re.match(line)
        if (result is not None):
            self.segments["Heap"].addSectionPart(Memory.heap_section, int(result.group(1), 16))
            self._addObjInfo(Memory.linker_object, Memory.heap_section, result.group(1))
            return

        # Stack size
        for reg_ex in self.Linker_Stack_Size_re:
            result = reg_ex.match(line)
            if result is not None:
                self.segments["Stack"].addSectionPart(Memory.stack_section, int(result.group(1), 16))
                self._addObjInfo(Memory.linker_object, Memory.stack_section, result.group(1))
                return

        # Stack address
        result = self.Linker_Stack_Start_re.match(line)
        if result is not None:
            address = int(result.group(1), 16)
            self.segments["Stack"].addSectionAddress(Memory.stack_section, address)

        return

    def parseMapFile(self):

        self._initChapterMemory()
        self._initChapterLinker()
        self._initChapterArchive()

        self.chapter = ""

        with open(self.mapfile, 'r') as fid:

            for line in fid:
                # Control
                if line.startswith('Archive member'):
                    self.chapter = "Archive"
                elif line.startswith('Allocating common symbols'):
                    self.chapter = "Symbols"
                elif line.startswith('Discarded input sections'):
                    self.chapter = "Discarded"
                elif line.startswith('Memory Configuration'):
                    self.chapter = "Memory"
                elif (line.startswith('Linker script and memory map') or line.startswith('Memory map')):
                    self.chapter = "Linker"
                elif (line.startswith('OUTPUT(')):
                    self.chapter = "Stubs"
                elif line.startswith('Cross Reference Table'):
                    self.chapter = "Cross"

                if self.chapter == "Linker":
                    self._parseChapterLinker(line)
                elif self.chapter == "Memory":
                    self._parseChapterMemory(line)
                elif self.chapter == "Archive":
                    self._parseChapterArchive(line)

        # Calculate the total size
        self._calculateSizes()

        # Clean up segments and modules
        self._clean()

        # Perform check
        self._checkSizes()

    def _checkSizes(self):
        check = ['Flash', 'Ram', 'Rom']
        size = {}
        gaps = {}

        for c in check:
            size[c] = sum([self.modules[module].size[c] for module in self.modules])
            gaps[c] = 0
            if (Memory.gaps_object in self.objects):
                gaps[c] = sum([self.objects[Memory.gaps_object].sections[section]
                               for section in self.objects[Memory.gaps_object].sections if section in self.segments[c].sections])

            if (size[c] + gaps[c] != self.segments[c].size):
                warn("WARNING: Size diff for %s: modules (%i) + gaps (%i) != segments (%i)" %
                     (c, size[c], gaps[c], self.segments[c].size))

            size[c] = 0
            for s in self.segments[c].sections:
                sizeSection = sum([self.objects[o].sections[s] for o in self.objects if s in self.objects[o].sections])
                size[c] += sizeSection
                if (sizeSection != self.segments[c].sections[s].size):
                    warn("WARNING: Size diff for %s, section %s: objects (%i) != segments (%i)" %
                         (c, s, sizeSection, self.segments[c].sections[s].size))

            if (size[c] != self.segments[c].size):
                warn("WARNING: Size diff for %s: objects (%i) != segments (%i)" % (c, size[c], self.segments[c].size))

    def dump(self):
        self._dumpObjects()
        self._dumpWithSectionMappings()
        # Memory.dump(self)
        self._dumpModules()
        self.dumpSizeInfo()
