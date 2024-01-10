#!/usr/bin/env python3
"""Python library to dump memory usage information
    memoryusage.py <mapfile> <compiler>
"""
from __future__ import print_function

import sys
from optparse import OptionParser

try:
    from gppy.tools.memory.compiler.layout import Memory, importModule
except ImportError:
    try:
        from layout import Memory, importModule
    except ImportError:
        print("Using outside of Env ?")


def parseMemory(mapfile, compiler, heap_size_definition=Memory.HEAP_SIZE_FIXED) -> Memory:
    module = importModule(compiler)
    info = module.CompilerInfo(mapfile, heap_size_definition)
    info.parseMapFile()
    return info


class dumpSizeInfoNotSupported(object):
    def dump(self):
        print("")
        print("-- memory overview --")
        print("    not supported")
        print("")


def checkCompilerSupport(compiler):
    try:
        eval(importModule(compiler))
    except Exception:
        return False
    else:
        return True


def memoryUsage(mapfile, compiler, heap_size_definition=Memory.HEAP_SIZE_FIXED):
    print(f"[{compiler}] Parsing: {mapfile}")

    if checkCompilerSupport(compiler):
        info = parseMemory(mapfile, compiler, heap_size_definition)
    else:
        info = dumpSizeInfoNotSupported()
    info.dump()


if __name__ == "__main__":
    usage = "usage: %prog [options]"
    parser = OptionParser(usage)
    parser.add_option("--heap_size_definition",
                      action="store",
                      dest="heap_size_definition",
                      default=Memory.HEAP_SIZE_FIXED,
                      help="Define how heap size is defined in linkerscript. Using all remaining RAM can not show up in map-file.\nHEAP_SIZE_FIXED - 0\nHEAP_SIZE_REMAINING_RAM - 1")

    (options, args) = parser.parse_args()

    if (len(args) < 2):
        targets = [cl.__name__ for cl in Memory.__subclasses__()]
        print(__doc__)
        print("    Supported compilers: %s" % targets)
        sys.exit(1)

    memoryUsage(args[0], args[1], options.heap_size_definition)
