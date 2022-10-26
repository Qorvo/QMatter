# v0.9.0.0 Qorvo&reg; IoT Dev Kit for QPG6105 release

This Release Note describes the software release information for Qorvo IoT Dev Kit for QPG6105. The
SDK provides an example Matter&trade; light and Matter lock application as well as a Matter template application to quickly build custom products. It also contains several simple examples to demonstrate how to use the QPG6105 peripherals. Also, a bootloader supporting secure boot and enabling over-the-air (OTA) firmware upgrades is provided as reference. Finally, a Product Test Component (PTC) application is delivered to use as basis for RF testing of any QPG6105 platform.

>This is an early stage SDK of which the test coverage is limited to Matter certification tests. This SDK can be used for product development in
>engineering phase. This SDK should not be used for commercial products.

## Changes
- Support is added for Matter device attestation feature. QMatter provides all the needed tools from certificate generation to certificate deployment in the Matter firmware.
- A factory block section is added to the firmware hex file. This block stores all the needed certificates and keys to complete device attestation. It can also contain
information as passcode, discriminator, hardware version, serial number, etc. Tools to generate this factory block are provided.
- The tool *AppCreator* is added for easy setup of a new Matter application project. It generates the complete framework for a new project that can be used immediately for custom product development.
- Product Test Component (PTC) application is delivered in source. This allows easy porting of the application to any custom QPG6105 platform to do RF testing.
- Fixed LED behavior for color temperature and enhanced hue attribute updates.
- Removed lighting cluster state initialization in the application. This is done automatically inside the Matter stack when the cluster gets initialized.
- Updated the number of supported fabrics to 5.
- Migrated accessing/updating of the cluster attributes to the C++ access methods of the clusters instead of emberAf functions.
- Upgraded to the latest Matter stack: [https://github.com/Qorvo/connectedhomeip/tree/v1.0.0.0-qorvo](https://github.com/Qorvo/connectedhomeip/tree/v1.0.0.0-qorvo) - This Matter stack is used during Specification Validation Event (SVE) 2 and is Matter standard v1.0 certified for the lighting application.
- Matter stack updated to use QPG6105 Thread 1.3 certified code.

## Known Issues
- SDP012-330: Debugging with secure bootloader is not possible. Workaround is to use a non-secure bootloader for debugging. Instructions can be found [here](Documents/Guides/debugging_with_segger_ozone.md).
- SW-9628: The example driver (peripheral) applications do not support FreeRTOS. As workaround the
peripheral applications are given as reference without FreeRTOS support integrated.

## Release Management
- This SDK release is based on Base Components v2.10.3.1. For release notes for these specific components, please refer
to the [Documents/Release Notes](Documents/Release%20Notes) folder.
- Released from https://itgitlab.corp.qorvo.com/wcon/lps_sw/depot/-/blob/pr/SDK_Matter/v0.9.0.0/Applications/P345_Matter_DK_Endnodes/v0.9.0.0/release/SDK.py

## Certified components

### Bluetooth Low Energy

Certification overview:
|  | QDID | Declaration ID | Link |
| --- | --- | --- | --- |
| BLE Controller QPG6105 | 181764 | D059395 | [https://launchstudio.bluetooth.com/ListingDetails/145366](https://launchstudio.bluetooth.com/ListingDetails/145366) |
| BLE Host Stack | 146344 | D049426 | [https://launchstudio.bluetooth.com/ListingDetails/103670](https://launchstudio.bluetooth.com/ListingDetails/103670) |
| BLE Profiles and Services | 116593 | D041259 | [https://launchstudio.bluetooth.com/ListingDetails/66212](https://launchstudio.bluetooth.com/ListingDetails/66212) |

### Thread

QPG6105 is Thread 1.3 certified: [https://www.threadgroup.org/What-is-Thread/Developers#dnn_ctr1464_Thread_CompDataDefault_rptrProductData_tdcn_51](https://www.threadgroup.org/What-is-Thread/Developers#dnn_ctr1464_Thread_CompDataDefault_rptrProductData_tdcn_51).

Certification Identification Number: 13A006, Certification Date: 28/9/2022

### Matter standard

QPG6105 lighting application is Matter standard v1.0 certified.

Certification Identification Number: CSA22002MAT40002-24, Certification Date: 24/10/2022


# v0.8.1.0 Qorvo&reg; IoT Dev Kit for QPG6105 release

This Release Note describes the software release information for Qorvo IoT Dev Kit for QPG6105. The
SDK provides an example Matter light and Matter lock application as well as a Matter template application to quickly build custom products. It also contains several simple examples to demonstrate how to use the QPG6105 peripherals. Also a bootloader supporting secure boot and enabling over-the-air (OTA) firmware upgrades is provided as reference.

>This is an early stage SDK in which the functionality is limited tested. This SDK can be used for product development in
>engineering phase. This SDK should not be used for commercial products.

## Changes
- Upgraded to more recent Matter stack: [https://github.com/Qorvo/connectedhomeip/tree/v0.9.9.1](https://github.com/Qorvo/connectedhomeip/tree/v0.9.9.1) - This Matter stack is used during Specification Validation Event (SVE) 1.
- Matter lock application is updated to work with the Doorlock cluster instead of the OnOff cluster.
- Improvements done in the Over-The-Air upgrade flow to be compliant with the OTA test plan.
- Disabled Bluetooth LE advertising after reset when the Matter device is already part of a fabric. This change was needed to be Matter standard compliant.
- Implemented Identify cluster and OnOff cluster effects at application level for the Matter light.
- Added application note that describes how to move towards a production build configuration. Shows how logs can get disabled and how the assertion system can be updated to go into a reset sequence for production builds.
- Fixed SDP011-943: Defragmentation issue in the non-volatile memory component is fixed in the Key Value Storage glue layer (KVS).
- Fixed SDP011-980: Added further cluster support for Binding, Unit localisation, User label and Fixed label clusters.

## Known Issues
- SDP011-946: A reset might be triggered in the middle of a commissioning flow, causing the commissioning to fail. This is rarely seen. Workaround is to restart the commissioning flow.
- SDP011-947: A hardfault might get triggered after an OTA upgrade. Recovery can be done by resetting the device. The device will have successfully upgraded the firmware image after the reset.
- SDP012-330: Debugging with secure bootloader is not possible. Workaround is to use a non-secure bootloader for debugging. Instructions can be found [here](Documents/Guides/debugging_with_segger_ozone.md).
- SW-9628: The example driver (peripheral) applications do not support FreeRTOS. As workaround the
peripheral applications are given as reference without FreeRTOS support integrated.
- SDP012-333: For the Matter light application, when the light is toggled to OFF using the button on the development board, the level gets set to 1. As a result, upon the next toggle command, the light is ON with level 1 instead of the level it was before the OFF toggle.

## Release Management
- This SDK release is based on Base Components v2.10.3.1. For release notes for these specific components, please refer
to the [Documents/Release Notes](Documents/Release%20Notes) folder.
- Released from https://itgitlab.corp.qorvo.com/wcon/lps_sw/depot/-/blob/v2.10.3.0/Applications/P345_Matter_DK_Endnodes/v0.8.1.0/release/SDK.py - depot SHA: 994ad5b456387133ab6e14dbaa2e722bf277e437.

## Certified components
Not applicable for this release

# v0.8.0.0 Qorvo&reg; IoT Dev Kit for QPG6105 release

This Release Note describes the software release information for Qorvo IoT Dev Kit for QPG6105. The
SDK provides an example Matter light and Matter lock application as well as a Matter template application to quickly build custom products. It also contains several simple examples to demonstrate how to use the QPG6105 peripherals. Also a bootloader supporting secure boot and enabling over-the-air (OTA) firmware upgrades is provided as reference.

>This is an early stage SDK in which the functionality is limited tested. This SDK can be used for product development in
>engineering phase. This SDK should not be used for commercial products.

## Changes
- Added a Matter Base (template) example application. This application can be used as a template for creating any custom Matter
device. It makes use of the ZCL advanced platform (ZAP) tool for easy configurability of the application layers.

- Added support for secure bootloader. A reference implementation and all the needed tools are foreseen in the SDK to
allow signing of the application images to enable secure boot. The signing procedure is embedded in the build flow as a
post-build step.

- Added support for Over-The-Air (OTA) upgrade over Matter protocol. The provided Matter applications can now be upgraded
by using the Matter download protocol. Also the needed guides and tools are provided in this SDK.

- Integrated support for Segger Ozone debugger.

- Upgraded to more recent Matter stack: [https://github.com/Qorvo/connectedhomeip/tree/v0.9.9.0](https://github.com/Qorvo/connectedhomeip/tree/v0.9.9.0)

## Post v0.8.0.0 updates
In this section you will find information of patches that are committed after the initial v0.8.0.0 branch is pushed to GitHub. These patches
are visible as a separate commit in the branch.

- Fixed SDP011-945: Implemented a fix in the Bluetooth LE manager statemachine so Bluetooth LE
advertising can restart correctly.
- Fixed SDP012-329: Updated the program rule in Make so it programs the correct hex file after the
post-build steps are executed.
- Fixed SDP012-332: New LED behavior is implemented to indicate if the Matter device is in an
advertising state or not. During advertising, fast red led blinks will be seen.
- Fixed SDP012-361: GPIO, ADC and UART peripheral examples including sleep support are now added in the QMatter deliverable.
- Fixed SDP012-386: Update in activate.sh script to fetch the latest package information from all configured sources on the system.

## Known Issues
- SDP011-946: A reset might be triggered in the middle of a commissioning flow, causing the commissioning to fail. This is rarely seen. Workaround is to restart the commissioning flow.
- SDP011-947: A hardfault might get triggered after an OTA upgrade. Recovery can be done by resetting the device. The device will have successfully upgraded the firmware image after the reset.
- SDP011-943: Defragmentation of the non-volatile memory component does not take into account any old and removed marker entries. Eventually the non-volatile memory area will fill up and an assert will be triggered. Full erase and reprogramming of the flash is needed when this occurs.
- SDP012-330: Debugging with secure bootloader is not possible. Workaround is to use a non-secure bootloader for debugging. Instructions can be found [here](Documents/Guides/debugging_with_segger_ozone.md).
- SW-9628: The example driver (peripheral) applications do not support FreeRTOS. As workaround the
peripheral applications are given as reference without FreeRTOS support integrated.
- SDP012-333: For the Matter light application, when the light is toggled to OFF using the button on the development board, the level gets set to 1. As a result, upon the next toggle command, the light is ON with level 1 instead of the level it was before the OFF toggle.
- SDP011-980: Some features of following Matter clusters are missing still:
    - Binding cluster.
    - Unit localisation cluster.
    - User label cluster.
    - Fixed label cluster.

## Release Management
- This SDK release is based on Base Components v2.10.3.0. For release notes for these specific components, please refer
to the [Documents/Release Notes](Documents/Release%20Notes) folder.
- Released from https://itgitlab.corp.qorvo.com/wcon/lps_sw/depot/-/blob/v2.10.3.0/Applications/P345_Matter_DK_Endnodes/v0.8.0.0/release/SDK.py

## Certified components
Not applicable for this release

# v0.7.1.0 Qorvo&reg; IoT Dev Kit for QPG6105 Alpha Release
This Release Note describes the software release information for Qorvo IoT Dev Kit for QPG6105. The
SDK provides Matter light and Matter lock example applications. It also contains several simple reference applications
to demonstrate how to use the QPG6105 peripherals and their drivers.

>This is an early stage SDK in which the functionality is limited tested. This SDK can be used for product development in
>engineering phase. This SDK should not be used for commercial products.

## Changes
- Fixed SDP011-182 - Base Components (gpNvm, Bluetooth LE controller, Bluetooth LE Host) that are used for the Matter protocol and proven to be stable, are now pushed to ROM. Using QPG6105 ROM offloaded SW maximizes application flash availability.
- Fixed SDP011-600 and SDP011-636 - fixed buffer overflow issue when a write to NVM is done when the key-payload length exceeds 264 bytes.
- Fixed SDP011-625 - Added check at start-up to verify HW revision of QPG6105. This version of the DK is only compatible with QPG6105 containing ROM v1.

## Known Issues


## Release Management
- This SDK release is based on Base Components v2.10.2.0. For release notes for these specific components, please refer
to the [Documents/Release Notes](Documents/Release%20Notes) folder.
- Released from //depot/release/Embedded/Applications/P345_Matter_DK_Endnodes/v0.7.1.0/release/SDK.py

## Certified components
Not applicable for this release

# v0.7.0.0 Qorvo&reg; IoT Dev Kit for QPG6105 Release
This Release Note describes the software release information for Qorvo IoT Dev Kit for QPG6105. The
SDK provides Matter light and Matter lock example applications. It also contains several simple reference applications
to demonstrate how to use the QPG6105 peripherals and their drivers.

>This is an early stage SDK in which the functionality is limited tested. This SDK can be used for product development in
>engineering phase. This SDK should not be used for commercial products.

## Changes
- This is the initial release of Qorvo IoT Dev Kit for QPG6105. This version allows Qorvo customers to develop Matter standard based
applications on Qorvo state-of-the-art QPG6105 SoC.
- This SDK includes ready to use applications that are grouped like below:
    - Matter: Includes Matter light and Matter lock project examples.
    - Peripherals: Includes the peripheral example applications.

## Known Issues


## Release Management
- This SDK release is based on Base Components v2.10.2.0. For release notes for these specific components, please refer
to the [Documents/Release Notes](Documents/Release%20Notes) folder.
- Released from //depot/release/Embedded/Applications/P345_Matter_DK_Endnodes/v0.7.0.0/release/SDK.py

## Certified components
Not applicable for this release
