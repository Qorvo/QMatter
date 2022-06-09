# v0.8.0.0 QPG6105 Matter SDK Release

This Release Note describes the software release information for the QPG6105 Matter Software Development Kit (SDK). The
SDK provides an example Matter light and Matter lock application as well as a Matter template application to quickly build custom products. It also contains several simple examples to demonstrate how to use the QPG6105 peripherals. Also a user mode bootloader supporting secure boot and enabling over-the-air (OTA) firmware upgrades is provided as reference.

>This is an early stage SDK in which the functionality is limited tested. This SDK can be used for product development in
>engineering phase. This SDK should not be used for commercial products.

## Changes compared to release v0.7.0.0
- Added a Matter Base (template) example application. This application can be used as a template for creating any custom Matter
device. It makes use of the ZCL advanced platform (ZAP) tool for easy configurability of the application layers.

- Added support for secure bootloader. A reference implementation and all the needed tools are foreseen in the SDK to
allow signing of the appliction images to enable secure boot. The signing procedure is embedded in the build flow as a
post-build step.

- Added support for Over-The-Air (OTA) upgrade over Matter protocol. The provided Matter applications can now be upgraded
by using the Matter download protocol. Also the needed guides and tools are provided in this SDK.

- Integrated support for Segger Ozone debugger.

- Upgraded to more recent Matter stack: [https://github.com/Qorvo/connectedhomeip/tree/v0.9.9.0](https://github.com/Qorvo/connectedhomeip/tree/v0.9.9.0)

## Known Issues

- [SDP011-945](https://btap.qorvo.com/browse/SDP011-945): Restarting Bluetooth LE advertising can fail, causing the commissioning to fail. This issue is triggered when for example a wrong dataset is given of the OpenThread Border Router during commissioning. Workaround is to reset the device and start again the commissioning flow.
- [SDP011-946](https://btap.qorvo.com/browse/SDP011-946): A reset might be triggered in the middle of a commissioning flow, causing the commissioning to fail. This is rarely seen. Workaround is to restart the commissioning flow.
- [SDP011-947](https://btap.qorvo.com/browse/SDP011-947): A hardfault might get triggered after an OTA upgrade. Recovery can be done by resetting the device. The device will have successfully upgraded the firmware image after the reset.
- [SDP011-943](https://btap.qorvo.com/browse/SDP011-943): Defragmentation of the non-volatile memory component does not take into account any old and removed marker entries. Eventually the non-volatile memory area will fill up and an assert will be triggered. Full erase and reprogramming of the flash is needed when this occurs.
- [SDP012-329](https://btap.qorvo.com/browse/SDP012-329): The Program rule in Make does not program the hex file that results from the post-build step (image signing for secure bootloader). This causes the secure bootloader not accepting the application image and so fails to boot. Workaround is to use JFlashLite programmer or use Drag 'n drop programming.
- [SDP012-330](https://btap.qorvo.com/browse/SDP012-330): Debugging with secure bootloader is not possible. Workaround is to use a non-secure bootloader for debugging. Instructions can be found [here](Documents/Guides/debugging_with_segger_ozone.md).
- [SW-9628](https://btap.qorvo.com/browse/SW-9628): The example driver (peripheral) applications do not support FreeRTOS. As workaround the
peripheral applications are given as reference without FreeRTOS support integrated.
- [SDP012-332](https://btap.qorvo.com/browse/SDP012-332): For the Matter lock application, wrong LED behavior is seen after reset.
- [SDP012-333](https://btap.qorvo.com/browse/SDP012-333): For the Matter light application, when the light is toggled to OFF using the button on the development board, the level gets set to 1. As a result, upon the next toggle command, the light is ON with level 1 instead of the level it was before the OFF toggle.  
- [SDP011-980](https://btap.qorvo.com/browse/SDP011-980): Some features of following Matter clusters are missing still:
    - Binding cluster.
    - Unit localisation cluster.
    - User label cluster.
    - Fixed label cluster.

## Release Management
- This SDK release is based on Base Components v2.10.3.0. For release notes for these specific components, please refer
to the [Documents/Release Notes](Documents/Release%20Notes) folder.

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


## Certification

Not applicable for this release
