# v0.8.0.0 QPG6105 Matter SDK Release

This Release Note describes the software release information for the QPG6105 Matter Software Development Kit (SDK). The
SDK provides an example Matter light and Matter lock application as well as a Matter template application to quickly build custom products. It also contains several simple examples to demonstrate how to use the QPG6105 peripherals. Also a user mode bootloader supporting secure boot and enabling over-the-air (OTA) firmware upgrades is provided as reference.

>This is an early stage SDK in which the functionality is limited tested. This SDK can be used for product development in
>engineering phase. This SDK should not be used for commercial products.

## Changes
- Added a Matter Base (template) example application. This application can be used as a template for creating any custom Matter
device. It makes use of the ZCL advanced platform (ZAP) tool for easy configurability of the application layers.

- Added support for secure bootloader. A reference implementation and all the needed tools are foreseen in the SDK to
allow signing of the appliction images to enable secure boot. The signing procedure is embedded in the build flow as a
post-build step.

- Added support for Over-The-Air (OTA) upgrade over Matter protocol. The provided Matter applications can now be upgraded
by using the Matter download protocol. Also the needed guides and tools are provided in this SDK.

- Integrated support for Segger Ozone debugger.

- Upgraded to more recent Matter stack: [https://github.com/Qorvo/connectedhomeip/tree/v0.9.9.0](https://github.com/Qorvo/connectedhomeip/tree/v0.9.9.0)

## Post v0.8.0.0 updates
In this section you will find information of patches that are committed after the initial v0.8.0.0 branch is pushed to GitHub. These patches
are visible as a seperate commit in the branch.

- Fixed [SDP011-945](https://btap.qorvo.com/browse/SDP011-945): Implemented a fix in the Bluetooth LE manager statemachine so Bluetooth LE
advertising can restart correctly.
- Fixed [SDP012-329](https://btap.qorvo.com/browse/SDP012-329): Updated the program rule in Make so it programs the correct hex file after the
post-build steps are executed.
- Fixed [SDP012-332](https://btap.qorvo.com/browse/SDP012-332): New LED behavior is implemented to indicate if the Matter device is in an
advertising state or not. During advertising, fast red led blinks will be seen.
- Fixed [SDP012-361](https://btap.qorvo.com/browse/SDP012-361): GPIO, ADC and UART peripheral examples including sleep support are now added in the QMatter deliverable.
- Fixed [SDP012-386](https://btap.qorvo.com/browse/SDP012-386): Update in activate.sh script to fetch the lastest package information from all configured sources on the system.

## Known Issues
- [SDP011-946](https://btap.qorvo.com/browse/SDP011-946): A reset might be triggered in the middle of a commissioning flow, causing the commissioning to fail. This is rarely seen. Workaround is to restart the commissioning flow.
- [SDP011-947](https://btap.qorvo.com/browse/SDP011-947): A hardfault might get triggered after an OTA upgrade. Recovery can be done by resetting the device. The device will have successfully upgraded the firmware image after the reset.
- [SDP011-943](https://btap.qorvo.com/browse/SDP011-943): Defragmentation of the non-volatile memory component does not take into account any old and removed marker entries. Eventually the non-volatile memory area will fill up and an assert will be triggered. Full erase and reprogramming of the flash is needed when this occurs.
- [SDP012-330](https://btap.qorvo.com/browse/SDP012-330): Debugging with secure bootloader is not possible. Workaround is to use a non-secure bootloader for debugging. Instructions can be found [here](Documents/Guides/debugging_with_segger_ozone.md).
- [SW-9628](https://btap.qorvo.com/browse/SW-9628): The example driver (peripheral) applications do not support FreeRTOS. As workaround the
peripheral applications are given as reference without FreeRTOS support integrated.
- [SDP012-333](https://btap.qorvo.com/browse/SDP012-333): For the Matter light application, when the light is toggled to OFF using the button on the development board, the level gets set to 1. As a result, upon the next toggle command, the light is ON with level 1 instead of the level it was before the OFF toggle.  
- [SDP011-980](https://btap.qorvo.com/browse/SDP011-980): Some features of following Matter clusters are missing still:
    - Binding cluster.
    - Unit localisation cluster.
    - User label cluster.
    - Fixed label cluster.

## Release Management
- This SDK release is based on Base Components v2.10.3.0. For release notes for these specific components, please refer
to the [Documents/Release Notes](Documents/Release%20Notes) folder.
- Released from https://itgitlab.corp.qorvo.com/wcon/lps_sw/depot/-/blob/v2.10.3.0/Applications/P345_Matter_DK_Endnodes/v0.8.0.0/release/SDK.py

## Certification
Not applicable for this release

# v0.7.1.0 QPG6105 Matter SDK Alpha Release
This Release Note describes the software release information for the QPG6105 Matter Software Development Kit (SDK). The
SDK provides Matter light and Matter lock example applications. It also contains several simple reference applications
to demonstrate how to use the QPG6105 peripherals and their drivers.

>This is an early stage SDK in which the functionality is limited tested. This SDK can be used for product development in
>engineering phase. This SDK should not be used for commercial products.

## Changes
- Fixed [SDP011-182](https://btap.qorvo.com/browse/SDP011-182) - Base Components (gpNvm, Bluetooth LE controller, Bluetooth LE Host) that are used for Matter and proven to be stable, are now pushed to ROM. Using QPG6105 ROM offloaded SW maximizes application flash availability.
- Fixed [SDP011-600](https://btap.qorvo.com/browse/SDP011-600) and [SDP011-636](https://btap.qorvo.com/browse/SDP011-636) - fixed buffer overflow issue when a write to NVM is done when the key-payload length exceeds 264 bytes.
- Fixed [SDP011-625](https://btap.qorvo.com/browse/SDP011-625) - Added check at start-up to verify HW revision of QPG6105. This version of the DK is only compatible with QPG6105 containing ROM v1.

## Known Issues


## Release Management
- This SDK release is based on Base Components v2.10.2.0. For release notes for these specific components, please refer
to the [Documents/Release Notes](Documents/Release%20Notes) folder.
- Released from //depot/release/Embedded/Applications/P345_Matter_DK_Endnodes/v0.7.1.0/release/SDK.py

## Certification
Not applicable for this release

# v0.7.0.0 QPG6105 Matter SDK Release
This Release Note describes the software release information for the QPG6105 Matter Software Development Kit (SDK). The
SDK provides Matter light and Matter lock example applications. It also contains several simple reference applications
to demonstrate how to use the QPG6105 peripherals and their drivers.

>This is an early stage SDK in which the functionality is limited tested. This SDK can be used for product development in
>engineering phase. This SDK should not be used for commercial products.

## Changes
- This is the initial release of the QPG6105 Matter SDK. This version allows Qorvo's customers to develop Matter based
applications on Qorvo's state-of-the-art QPG6105 SoC.
- This SDK includes ready to use applications that are grouped like below:
    - Matter: Includes Matter light and Matter lock project examples.
    - Peripherals: Includes the peripheral example applications.

## Known Issues


## Release Management
- This SDK release is based on Base Components v2.10.2.0. For release notes for these specific components, please refer
to the [Documents/Release Notes](Documents/Release%20Notes) folder.
- Released from //depot/release/Embedded/Applications/P345_Matter_DK_Endnodes/v0.7.0.0/release/SDK.py

## Certification
Not applicable for this release
