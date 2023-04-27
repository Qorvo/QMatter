# How to create a Matter device based on Qorvo silicon

This document provides the a full user journey to develop a Matter end product based on Qorvo silicon. It provides
all the needed links from getting started with the kit, setting up the development environment, creating your custom
application, testing, certification and preparation of the device for production.

## Step 1: Getting started with the Software Development Kit

The [QMatter Landing  page](../../) explains how to install and use the QMatter SDK,
and provides all the information to get started with Qorvo IoT Development Kit for QPG6105 and Qorvo IoT Development Kit Pro.

## Step 2: Getting a basic understanding around Matter Standard

[Matter standard](https://buildwithmatter.com/) (formerly Project Connected Home over IP or Project CHIP) is an industry-unifying standard for IoT. It defines a unified application layer and data model for devices operating with different IP protocols including Wi-Fi and Thread. The Matter stack is developed and maintained as an open-source project. Matter technology is built around four core principles:
<ol>
    <li> Simplicity </li>
    <li> Interoperability </li>
    <li> Reliability </li>
    <li> Security </li>
</ol>

For more information about the Matter standard, please refer to [the Matter codebase used in this SDK](https://github.com/Qorvo/connectedhomeip/tree/v1.1.0.0_qorvo) and the [Connectivity standards alliance website on Matter protocol](https://csa-iot.org/all-solutions/matter/).

A very interesting read around the Matter Standard can be found on [Matter Introduction](https://groups.csa-iot.org/wg/members-all/document/24631?downloadRevision=49195).
This presentation is restricted for Connectivity Standards Alliance Members only. To get access to this page, your need
to register [here](https://csa-iot.org/become-member/).

Latest Matter specifications can be found [here](https://csa-iot.org/developer-resource/specifications-download-request/).

## Step 3: Development

The QMatter SDK provides several example Matter applications which you can explore.

On the hardware side, the SDK contains [schematics of the development kit hardware](Hardware)
to facilitate an early prototype stage while the [QPG6105 datasheet can be found on www.qorvo.com](https://www.qorvo.com/products/p/QPG6105#documents).

To start work on your own application you can use our [AppConfigurator tool](Tools/AppConfigurator)
to create a customized version of one of the [reference Matter applications](Applications/Matter).
AppCreator will handle copy and rename and allows you to set and reconfigure several application
specific features like enabling high temperature support and which `.factory_config` file (to set device attestation
certificates and vendorid/productid) is used.

Our [Matter Application Programmer Interface document](Documents/API_Manuals/Matter/Matter_API_Manual.md) describes
the API to be used for developing your Matter device.  It covers amongst other things the program initialization flow and the Matter
data model.

The [Peripheral example applications](Applications/Peripherals) can be used as a reference to using the
Qorvo silicon's on-chip peripherals that are of particular interest.

The [QMatter SDK build flow documentation](Documents/Guides/make_build_flow.md) explains how the different parts (Bootloader,
device specific data) that make up the application flash are put together. It explains which files are used to
configure the application and how to add extra files to the build.

To learn more about individual parts of the SDK, you can read the README files included in each folder of the SDK.

The QPG6105 DK's [on-board JLink probe can be used to flash the application](README.md#flashing) during development. You
can also learn more about [debugging the application using Segger Ozone](Documents/Guides/debugging_with_segger_ozone.md).

Device attestation is an important part of Matter, you can learn how [device attestation works](Documents/Guides/device_attestation.md),
and use the [credentials generator tool](Tools/CredentialsGenerator) that we provide to create your own
certificates.

## Step 4: Testing

Testing tools can help verify the Matter applications which are under development. The first tool you need is the Matter controller which can verify commissioning and controlling of the Matter device in the Matter network. There are few options of Matter controllers:

1. A POSIX CLI chip-tool - it is a client application running on Ubuntu platform. [Commissioning Qorvo Matter™ device with POSIX CLI chip-tool](Documents/Guides/commissioning_posix_cli_chiptool.md) provides instructions on how to use the chip-tool together with Qorvo® IoT Dev Kits to commission and control a Matter device onto the Matter network.
2. Apple's Matter implementation - Apple iPad/iPhone and Apple HomePod mini can work as a Matter controller and a Thread Border Router respectively. [Commissioning a Qorvo Matter™ end device in the Apple ecosystem](Documents/Guides/commissioning_with_apple.md) provide instructions on how to make use of Apple HomePod mini and Apple iPad/iPhone to commission and control a Matter device onto the Matter network in Apple ecosystem.
3. Google's Matter implementation - Android Tablet/Smartphone and Google Nest Hub (2nd generation) can work as a Matter controller and a Thread Border Router respectively. [Commissioning a Qorvo Matter™ end device in the Google ecosystem](Documents/Guides/commissioning_with_google.md) provides instructions on how to make use of Android Tablet/Smartphone and Google Nest Hub (2nd generation) to commission and control a Matter device onto the Matter network in Google ecosystem.

Another useful tool is the OTA provider application which can be used to verify OTA firmware upgrade. [Matter™ protocol Over-The-Air (OTA) Device firmware upgrade](Documents/Guides/ota_device_firmware_upgrade.md) provide a guideline to make use of OTA provider application together with chip-tool and Qorvo® IoT Dev Kits to setup a Matter network and verify OTA firmware upgrade. Please be reminded that both OTA provider application and chip-tool can coexist on a single Ubuntu platform, e.g. RPi4.

Besides, there is a Qorvo® Product Test Component(PTC) System which targets to enable RF validation, RF production test and nd PHY and MAC certification measurements of the Qorvo IEEE802.15.4 and Bluetooth LE Silicon. You can refer to the [guide](Applications/PTC/README.md) for more information of PTC system.

## Step 5: Certification

Before you can move to Matter certification, you are dependent on network connectivity
certification requirements which come with their own certification programs.

The underlying network transport layer that used on Qorvo silicon is Thread. Therefore, you
need to become a member of the [Thread Group](https://www.threadgroup.org/thread-group) and
Thread certification is required. The certification program for Thread is explained on this
website: [Thread certification](https://www.threadgroup.org/What-is-Thread/Certification). If
you use the certified Thread stack that comes with this Qorvo Software Development Kit, you can use
inheritance of Qorvo's Thread certification. See [Release Notes](RELEASE_NOTES.md) to find the
latest Certification Identification Number.

Bluetooth Low Energy technology is used for commissioning. Also for this protocol a
certification is needed. For this, you need to become a member of the
[Bluetooth SIG](https://www.bluetooth.com/develop-with-bluetooth/join/) and Bluetooth
certification is required. The certification program for Bluetooth is explained on this
website: [Bluetooth certification](https://www.bluetooth.com/develop-with-bluetooth/qualification-listing/).
If you use the certified Bluetooth stack that comes with this Qorvo Software Development Kit, you can use
inheritance of Qorvo's Bluetooth certification. See [Release Notes](RELEASE_NOTES.md) to find the
latest QID.

Now, your end product can be Matter certified. The Connectivity Standards Alliance (CSA) brings a full
certification program with test specification and certification framework.

High level overview of CSA's Matter certification program can be found on this website:
[CSA Matter Certification Process](https://csa-iot.org/certification/why-certify/).
More detailed documentation related to certification is available for Matter members on this
restricted website: [Matter Resource Kit](https://groups.csa-iot.org/wg/all-users/home/matter-resource-kit). Note
that you need to be a member of CSA to access this information.

To certify your end product for Matter you need:
1. Become a member of CSA.
2. Request a Vendor Identifier.
3. Select a Testing Provider from [this list](https://csa-iot.org/certification/testing-providers/).
4. Send product to the test lab together with the Protocol Implementation Conformance
Statement (PICS). Example PICS documents can be found in the relevant Matter application folder. Example for the
Matter extended color light can be found [here](Applications/Matter/light/pics). PICS documents for the
other application are available for reference as well. You can simply drag-and-drop these files in the
[PICS Tool](https://picstool.csa-iot.org/) for easy reading/modifying the PICS.
5. Submit Certification application.
6. Application Pending. The application is pending review by CSA Team.
7. Approval. Now you will receive a product certification. You will also get a Certification Declaration blob file
and a certified product record will get entered in the [Distributed Compliance Ledger (DCL)](https://webui.dcl.csa-iot.org/).

## Step 6: Set up production builds

When the application has matured, we can start configuring it to be flashed
in a production environment.

The following table provides an overview of features and tools that are
applied as we move from a debugging friendly build to a final build that
will be flashed at the factory:

| build type | verbose logging | flash method           | type of DAC certificates | secure boot |
| ---------- | --------------- | ---------------------- | ------------------------ | ----------- |
| debug      | enabled         | JLink (bench)          | testnet                  | not used    |
| release    | disabled        | JLink (bench)          | testnet                  | not used    |
| production | disabled        | Qorvo USBPOD (factory) | mainnet                  | enabled     |

1. build type: For each of the three build types of the Matter examples the SDK contains a Makefile, e.g.:  Makefile.base\_qpg6105*\_development*, Makefile.base\_qpg6105*\_release*.
2. verbose logging: Detailed error messages are very helpful during development, but they take up code size and print to
   a not-connected serial port in the final product.
3. flash method: While developing the application, we typically use the JLink for circuit programming/debugging. Qorvo
   developed a programming device for factory line programming that can be used to flash device-specific data.
4. Device Attestation certificates: During development, a test vendor ID or certificates on the testnet can be used,
   production builds ran on final devices will require a valid Matter compliant certificate chain and conformance
   declaration.
5. To protect device secrets like the Device Attestation private key, secure boot must be enabled. Once enabled, a
   device can only boot correctly signed application firmware.


The following Application Notes describe the steps needed to advance across the table above.

1. The document [Setting up a release build](Documents/Production%20Setup%20Guides/creating_release_build.md)
   describes how to disable verbose logging and update the assert behavior to map to a reset trigger instead of a halt
   of the application. This is the first step to take to move to a production build. The SDK provides example
   configurations identified by the \_release suffix in the filenames.

> WARNING: The changes described in `creating_release_builds.md` also need to be applied to the `_production` configuration
> files!!!

2. To mass produce your product, you will need to flash each device at the production line. To be Matter certified you
   will need to provision it with Device Attestation data and sign the firmware to perform Secure Boot. The application
   note on [creating production build for the factory](Documents/Production%20Setup%20Guides/creating_production_build.md)
   provides details on how you can do this.
   To get started with this, use the \_production suffix builds of the applications. These builds use a Secure-Boot
   enabled bootloader and expect device attestation data (private key, factory data) is added to the flash in your
   production environment.


## Step 7: Use the production programmer

### Customize the example Certificate Server

When programming a device in production, the Qorvo Programming Utility will contact an enrollment service to get factory
block and keys specified in the XML file. The enrollment service
- will generate a unique private key and create a device attestation certificate.
- will use `generate_factory_block.py` to generate a device specific factory block.

Qorvo provides an [example implementation](Tools/CertificateServer) of such an enrollment service in Python that
provides static values for testing purposes.

### Programming the production build using the USBPOD programmer

The Qorvo Programming Utility uses a [XML configuration file](Documentation/Production%20Setup%20Guides/GP_P008_IS_18032_Flash_Programming_Control_Structure_XP3004_XP3003.pdf)
that is [generated by the QMatter SDK](Tools/SecureBoot). This file contains (among
other things) connectivity details on the enrollment service used, where to put the factory block and which (eg. secure
boot) keys it needs to program into the device.

- Install the Qorvo Programming Utility (Contact your sales representative for a download link) and provide it
the needed input files for your product.
- Make sure the IP address or hostname of the enrollment host in the XML file is correct. You can edit the default
  value "--server-address http://localhost:8000" in your app's postbuild.sh script (e.g. `base_qpg6105_production_postbuild.sh`).
- Load the XML file in the Qorvo Programming Utility, the firmware hex file will be uploaded to the USBPOD programmer.
- Make sure the machine can access the certificate service, you can do this by opening the certificate service's HTTP
  page `http://ip_address:port/is_alive` in a web browser.
- Flash the device using the Qorvo Programming Utility.
- Verify the device is functional after disconnecting it from the programmer.

### RMA flow

To facilitate the RMA flow, it is possible to create a `rma_token.hex` file that
can be provided to Qorvo to erase the software on a specific QPG6105 sample or enable bare access
mode over SPI for inspection.  Such a file can be generated using `generateRMAToken.py`.

The postbuild script of secureboot from ROM bootloader can be found at `Applications/Bootloader/UMB_qpg6105_compr_secure_sbrom_asym_key_postbuild.sh`
and contains an example call to `generateRMAToken.py` for a dummy MAC address.
