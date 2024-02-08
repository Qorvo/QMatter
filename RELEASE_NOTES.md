# v1.0.2 Qorvo&reg; IoT Dev Kit for QPG6105 release
This Release Note describes the software release information for Qorvo IoT Dev Kit for QPG6105. The
SDK provides:
- Example Matter&trade; applications for light, light switch, lock and thermostatic radiator valve.
- Example Matter base application with the minimal mandatory Matter features. This gives a first basis to quickly
build custom products.
- Several simple examples to demonstrate how to use the QPG6105 peripherals.
- A bootloader supporting secure boot and enabling over-the-air (OTA) firmware upgrades.
- Bluetooth LE peripheral application with device firmware upgrade service for OTA upgrade capabilities.
- A Product Test Component (PTC) application is delivered to use as basis for RF testing of any QPG6105 platform.

## Changes compared to previous release
### Matter
- Updated patch of Thread power fix

### Bluetooth LE
- no updates.

### Product Test Component
- no updates.

### Generic
- Fixed missing standard libraries with a precompiled GN executable during bootstrapping.
- Fixed button issue in FTD
- Adopt minimal power consumption configuration with SPI CS pin
- Updated adjusting_xtal_trimcap_configuration.md
- Add a direct link to the certified protocol compatibility from the homepage:
  https://github.com/Qorvo/QMatter/blob/v1.0.2/RELEASE_NOTES.md#certified-components
- Updated README.md for LFS support
- Add a build target that uses precompiled openthread libs instead of building them from source
- Remove Node.js installation from the bootstrapping

### Qorvo stack changes
- Add some Thread 1.2/ Thread 1.3 features
- Link to CHIP v1.1.0.2_qorvo branch

## Known Issues
- SDP012-613: For the Matter light application, the color control is not accurate. This is seen that the color on the
Red-Green-Blue LED is not always accurate for a requested color.
- SDP012-922: For the Matter thermostatic radiator valve, in the Apple ecosystem degrees are only shown in Celcius, even
when switching to Fahrenheit.
- SDP012-919: For the Matter light switch, button control is not possible for Google and Apple because both ecosystems
does not support Matter bindings.
- SDP012-330: Debugging with secure bootloader is not possible. An option is foreseen in the AppConfigurator tool to
easy enable unsecure bootloader for debugging purposes.
- SDP013-364: When calling gpPTC_GetAttributeRequest() to read gpPTC_AttributeRxLnaAttDuringTimeout attribute, the wrong
register is read from the chip. Setting the attribute is working as expected.
- SDP011-1696: For the Matter light application, light toggling with button during BLE advertisement stops working when going from OFF to ON state.

## Release Management
- Released from https://gitlab.com/qorvo/wcon/lps_sw/depot/-/blob/pr/SDK_Matter/v1.0.2/Applications/P345_Matter_DK_Endnodes/v1.0.0.0/release/SDK.py

## Certified components

### Bluetooth Low Energy

**Certification overview:**

|  | QDID | Declaration ID | Link |
| --- | --- | --- | --- |
| BLE Controller QPG6105 | 181764 | D059395 | [https://launchstudio.bluetooth.com/ListingDetails/145366](https://launchstudio.bluetooth.com/ListingDetails/145366) |
| BLE Host Stack | 146344 | D049426 | [https://launchstudio.bluetooth.com/ListingDetails/103670](https://launchstudio.bluetooth.com/ListingDetails/103670) |
| BLE profiles subsystem | N/A | N/A | not certified but verified with the TCRL.2018-2 compliance tester using Bluetooth Profile Tuning Suites 7.3.0. **Please include testing for the BLE profiles included in your end-product listing.** |

### Thread

QPG6105 is Thread 1.3 certified: [https://www.threadgroup.org/What-is-Thread/Developers#dnn_ctr1464_Thread_CompDataDefault_rptrProductData_tdcn_51](https://www.threadgroup.org/What-is-Thread/Developers#dnn_ctr1464_Thread_CompDataDefault_rptrProductData_tdcn_51).

Certification Identification Number: 13A006.1_2023_06_12, Certification Date: 12/6/2023

### Matter standard

| Application | Version | Certification ID | Link |
| --- | --- | --- | --- |
| [QMatter light](Applications/Matter/light) | Matter v1.1 | CSA23835MAT41348-24 | [Qorvo QPG6105 DK Extended Color Light](https://csa-iot.org/csa_product/qorvo-qpg6105-dk-extended-color-light/) |
| [QMatter switch](Applications/Matter/switch) | Matter v1.1 | CSA23B16MAT41627-24 | [Qorvo QPG6105 DK Color Dimmer Switch](https://csa-iot.org/csa_product/qorvo-qpg6105-dk-color-dimmer-switch/) |
| [QMatter lock](Applications/Matter/lock) | Matter v1.1 | CSA23B62MAT41673-24 | [Qorvo QPG6105 DK Door Lock](https://csa-iot.org/csa_product/qorvo-qpg6105-dk-door-lock/) |
| [QMatter trv](Applications/Matter/thermostaticRadiatorValve) | Matter v1.1 | CSA23B63MAT41674-24 | [Qorvo QPG6105 DK Thermostatic Radiator Valve](https://csa-iot.org/csa_product/qorvo-qpg6105-dk-thermostatic-radiator-valve/) |

# Previous releases

Release notes of previous releases can be found here:
- [v1.0.1](https://github.com/Qorvo/QMatter/blob/v1.0.1/RELEASE_NOTES.md)
- [v1.0.0.0](https://github.com/Qorvo/QMatter/blob/v1.0.0.0/RELEASE_NOTES.md)
- [v0.9.1.0](https://github.com/Qorvo/QMatter/blob/v0.9.1.0/RELEASE_NOTES.md)
- [v0.9.0.0](https://github.com/Qorvo/QMatter/blob/v0.9.0.0/RELEASE_NOTES.md)
