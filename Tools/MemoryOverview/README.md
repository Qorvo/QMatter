# Memory Overview

## Purpose

This tool is intended to give an overview of the Memory footprint that is used for the Matter applications. It gives a general overview of the used flash and ram of the QPG6105.
The tool has also the feature to give an overview of the specific memory footprint per software block (Application, Bluetooth LE, Matter (CHIP), MAC, OS/Libs, Security and Thread).

## Usage

```
    python3 memoryoverview.py --help
    Usage: memoryoverview.py [options] <mapfile>

    Options:
        -h, --help            show this help message and exit
        -l LOGFILENAME, --logfile=LOGFILENAME
                              Output log file name
        -s, --separate        Generate log file per application. Usefull for diffs.
        -a APP_MAP_PATH, --add-application=APP_MAP_PATH
                              Point to the map file of an application to add it to
                              the memory comparison
        -1 ONLY_THIS, --only-this=ONLY_THIS
                              Point to the map file of an application to analyze
```

## Example

Below an example is given to check the memory footprint status based on a map file of the Matter light bulb application.


```
    python3 memoryoverview.py -1 ../../Work/light_qpg6105_development/light_qpg6105_development.map

    ...

    +----------------+-------------------------------+
    |                | light_qpg6105_development.map |
    +----------------+-------------------------------+
    | Flash          |  634824/1048576 -    60.54 %  |
    | Flash+NVM+OTA  | 1032136/1048576 -    98.43 %  |
    | Ram            |  108934/ 131072 -    83.11 %  |
    | Stack          |     512/ 131072 -     0.39 %  |
    | Heap           |   21626/ 131072 -    16.50 %  |
    | Ram+Heap+Stack |  131072/ 131072 -   100.00 %  |
    | ------         | ------                        |
    | APP            | Flash:  96850 / RAM:  6124    |
    | BLE            | Flash:   4514 / RAM:   207    |
    | CHIP           | Flash: 246386 / RAM: 57503    |
    | MAC            | Flash:   8751 / RAM:   774    |
    | OS/Libs        | Flash: 101003 / RAM: 24215    |
    | Security       | Flash:  28722 / RAM:    66    |
    | Thread         | Flash: 143657 / RAM: 15128    |
    +----------------+-------------------------------+
```

## Integration into development flow

The memory overview tool is integrated as postbuild action in the make flow. This is seen in the corresponding Matter application postbuild script files. Example for [light_qpg6105_development_postbuild.sh](../../Applications/Matter/light/light_qpg6105_development_postbuild.sh):

```
"$PYTHON" "${SCRIPT_DIR}"/../../../Tools/MemoryOverview/memoryoverview.py --logfile "${SCRIPT_DIR}"/../../../Work/light_qpg6105_development/light_qpg6105_development.memoryoverview --only-this "${SCRIPT_DIR}"/../../../Work/light_qpg6105_development/light_qpg6105_development.map
```

Because of this a report of the memory footprint will be available for each build that is done.
