# ZCL Advanced Platform (ZAP) Tool

## Purpose

This tool is intended for generating the ZAP files and corresponding source files to be used for development of any
custom Matter&trade; application. This tool defines the clusters to be used in the Matter application and forms the basis for
Matter application development. More information how to create a Matter device can be found
[here](../../Applications/Matter/base/README.md#creating-matter-device).


## Implementation overview

This tool is implemented as a Python wrapper around the Matter zap tool. More details around this
Matter tool can be found [here](https://github.com/project-chip/zap/tree/master).

## Usage


```
      python3 generate_zap_files.py --help
      usage: generate_zap_files.py [-h] --input INPUT [--nogui]

      1. Starting the ZAP GUI 2. Regenerate the .zap file 3. Generate corresponding source/header files

      optional arguments:
        -h, --help       show this help message and exit
        --input INPUT    path to input .zap file
        --nogui          Add this option if it is not needed to do configuration in the gui
```

## Example

Before using the ZAP tool, make sure your environment is set up correctly. This can be achieved by executing
following command:

```
source Scripts/activate.sh
```

As an example the Matter base application will be used. To bring up the ZAP tool, execute following command:

```
      python3 ./Tools/Zap/generate_zap_files.py --input Applications/Matter/base/base.zap
```

Next, detailed instructions can be found [here](../../Applications/Matter/base/README.md#creating-matter-device).
