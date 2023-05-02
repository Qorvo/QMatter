# AppConfigurator Tool

## Introduction

The AppConfigurator tool is designed to facilitate the steps to customize one of the example Matter applications.
With this tool, the user of Qorvo&reg; IoT Dev Kit for QPG6105 will be able to create a new application starting from any of the SDK's reference applications and immediately pass the compilation stage.
To complete this process, the AppConfigurator will either first create a new application using the AppCreator tool, or update an existing application using the depending on the update strategy.
To perform the specified update, the AppConfigurator will crawl through the Application headers and make infrastructure and apply the necessary changes at file level.
To after the change is done all corresponding application builds will be triggered again.

## Example usage

```
(.python_venv) QMatter/Tools/AppConfigurator$ python app_configurator.py --update-strategy="update" --ref-app-name="lock" --debugging="enable"
```

## Validating the new application

When this Tools is executed as above, you should be able to find a new folder `lock_dbg` next to the example apps and the corresponding subbuild files for the modified instance of the lock application:

```
(.python_venv) QMatter/Tools/AppConfigurator$ python app_configurator.py --help
INFO:root:app_configurator.py -> provided args : ['--help']
usage: app_configurator.py [-h] --update-strategy {update,create,test} [--ref-app-name {light,thermostaticRadiatorValve,lock,switch,base}] [--debugging DEBUGGING] [--high-temperature-support {enable,disable}] [--external-sleep-crystal {enable,disable}] [--update-sw-version 0x<abcd>]
                           [--openthread-devicetype {ftd,mtd,sed}] [--factory-config {qorvo_switch.factory_data_config,qorvo_light.factory_data_config,qorvo_thermostaticRadiatorValve.factory_data_config,qorvo_base.factory_data_config,qorvo_lock.factory_data_config}] [--zapfile-strategy {reconfigure}]

Generate a new instance of an existing Matter application and/or reconfigure it.

optional arguments:
  -h, --help            show this help message and exit

required arguments:
  --update-strategy {update,create,test}
                        Specify whether to update an existing Matter application or generate a new application instance of the reference application

optional arguments:
  --ref-app-name {light,thermostaticRadiatorValve,lock,switch,base}
                        name of the reference application to be updated / created. By default the 'Base' application will be used as a reference
  --debugging {enable,disable}
                        Enable features needed to allow debugger to be functional
  --high-temperature-support {enable,disable}
                        Enable/disable mechanisms needed to operate in [-40:125]*Celcius environments.
  --external-sleep-crystal {enable,disable}
                        Enable/disable the configuration to enable the external sleep crystal support.
  --update-sw-version 0x<abcd>
                        Update software version.
  --openthread-devicetype {ftd,mtd,sed}
                        Configure the OpenThread device type.
  --factory-config {qorvo_switch.factory_data_config,qorvo_light.factory_data_config,qorvo_thermostaticRadiatorValve.factory_data_config,qorvo_base.factory_data_config,qorvo_lock.factory_data_config}
                        Use a different factory data configuration file than the one found in the reference application.
  --zapfile-strategy {reconfigure}
                        Reconfigure ZAP configuration file.
```

The new `lock_dbg` app can be compiled similar to the steps mentioned in [Building and flashing the example applications](../../README.md#building-and-flashing-the-example-applications) guide:
```
(.python_venv) QMatter/Tools/AppCreator$ cd ../../Applications/Matter/lock_dbg/
(.python_venv) QMatter/Applications/Matter/lock_dbg$ make -f Makefile.lock_dbg_qpg6105_development
```

Executing this build should result in the following successful postbuild statement
```
INFO:root:Combining all binary files to one output file
INFO:root: QMatter/Applications/Matter/lock_dbg/../../../Components/ThirdParty/Matter/repo/src/app/ota_image_tool.py create -v 0xFFF1 -p 0x8006 -vn 0x0003 -vs 1.1 -da sha256  QMatter/Applications/Matter/lock_dbg/../../../Work/lock_dbg_qpg6105_development/lock_dbg_qpg6105_development.compressed.bin QMatter/Applications/Matter/lock_dbg/../../../Work/lock_dbg_qpg6105_development/lock_dbg_qpg6105_development.ota
```

The resulting hex file of the application can be found in :
```
(.python_venv) QMatter/Work/lock_dbg_qpg6105_development$ ls -al lock_dbg_qpg6105_development.hex
-rw-rw-r-- 1 <user> <user> <size> <timestamp> lock_dbg_qpg6105_development.hex
```
