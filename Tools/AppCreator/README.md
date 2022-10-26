# AppCreator Tool

## Introduction

The AppCreator tool is designed to facilitate the steps to create a custom application.
With this tool, the user of Qorvo$reg; IoT Dev Kit for QPG6105 will be able to create a new application starting from the SDK's reference application "base" and immediately pass the compilation stage.
To complete this process, the AppCreator Tool will create the target application directory structure, create new makefiles in the subbuild systems and modify the "base" references in a required list of files.

## Example usage

```
(.python_venv) QMatter/Tools/AppCreator$ python3 app_creator.py --app-name switch
```

## Validating the new application

When this Tools is executed as above, you should be able to find a new folder `switch` next to the example apps and the corresponding subbuild files for the new app:
```
(.python_venv) QMatter/Tools/AppCreator$ git status
On branch <branch_reference>

Untracked files:
  (use "git add <file>..." to include in what will be committed)
        ../../Applications/Matter/switch/
        ../../Libraries/Qorvo/FactoryData/Makefile.FactoryData_switch
        ../../Libraries/ThirdParty/Matter/Makefile.Matter_switch_qpg6105
        ../FactoryData/Credentials/switch.factory_data_config
```

The new `switch` app can be compiled similar to the steps mentioned in [Building and flashing the example applications](../../README.md#building-and-flashing-the-example-applications) guide:
```
(.python_venv) QMatter/Tools/AppCreator$ cd ../../Applications/Matter/switch/
(.python_venv) QMatter/Applications/Matter/switch$ make -f Makefile.switch_qpg6105
```

Executing this build should result in the following successful postbuild statement
```
INFO:root:Combining all binary files to one output file
INFO:root: QMatter/Applications/Matter/switch/../../../Components/ThirdParty/Matter/repo/src/app/ota_image_tool.py create -v 0xFFF1 -p 0x8005 -vn 1 -vs 1.0 -da sha256  QMatter/Applications/Matter/switch/../../../Work/switch_qpg6105/switch_qpg6105.compressed.bin QMatter/Applications/Matter/switch/../../../Work/switch_qpg6105/switch_qpg6105.ota
QMatter/Applications/Matter/switch
```

The resulting hex file of the application can be found in :
```
(.python_venv) QMatter/Work/switch_qpg6105$ ls -al switch_qpg6105.hex
-rw-rw-r-- 1 <user> <user> 1591876 Okt  3 16:08 switch_qpg6105.hex
```

## Customising the new application

### Modify Board Support Package (BSP)
In order to align the newly created application to a custom board, the default BSP file needs to be modified.
By default, the reference applications in this SDK are using the Smart Home and Lighting board with QPG6105 which is defined in [gpBsp_Smart_Home_and_Lighting_CB_1_x_QPG6105.h ](../../Components/Qorvo/BSP/gpBsp/inc/SmartHomeAndLighting/gpBsp_Smart_Home_and_Lighting_CB_1_x_QPG6105.h)

The file is included in the build as follows:
```
(.python_venv) QMatter$ grep -r "SmartHomeAndLighting" .  | grep switch
./Applications/Matter/switch/Makefile.switch_qpg6105_release:INC_gpBsp+=-I$(BASEDIR)/../../../Components/Qorvo/BSP/gpBsp/inc/SmartHomeAndLighting
./Applications/Matter/switch/Makefile.switch_qpg6105:INC_gpBsp+=-I$(BASEDIR)/../../../Components/Qorvo/BSP/gpBsp/inc/SmartHomeAndLighting
```
and a new one can be selected by modifying the following line (depending on the selected build):
```
(.python_venv) QMatter/Tools/AppCreator$ grep -r "gpBsp_Smart_Home_and_Lighting_CB_1_x_QPG6105.h" ../../  | grep switch
../../Applications/Matter/switch/gen/switch_qpg6105_release/qorvo_config.h:#define GP_BSP_FILENAME                                                          "gpBsp_Smart_Home_and_Lighting_CB_1_x_QPG6105.h"
../../Applications/Matter/switch/gen/switch_qpg6105/qorvo_config.h:#define GP_BSP_FILENAME                                                          "gpBsp_Smart_Home_and_Lighting_CB_1_x_QPG6105.h"
```

### Adjust the cluster implementation
To adjust the functionality of the application by adding, editing and configuring clusters, you can follow the [zap tooling guide](../../Applications/Matter/base/README.md#zap-tool-usage).
