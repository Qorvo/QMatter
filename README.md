# Qorvo&reg; IoT Dev Kit for QPG6105

Welcome to Qorvo IoT Dev Kit for QPG6105 repository! This repository contains the Matter&trade; software development kit for [Qorvo QPG6105 SoC](https://www.qorvo.com/products/p/QPG6105). It supports you to develop applications that are compatible with Matter. The Qorvo development kit helps product developers bring Matter products to the market in a fast and easy way.

<div align="center">
  <img src="Images/qorvo_matter.png" alt="Qorvo Matter">
</div>

---
- [Qorvo® IoT Dev Kit for QPG6105](#qorvo-iot-dev-kit-for-qpg6105)
  - [Introduction](#introduction)
  - [Getting started with Qorvo IoT Development Kit for QPG6105](#getting-started-with-qorvo-iot-development-kit-for-qpg6105)
  - [Installation](#installation)
  - [How to use this repository?](#how-to-use-this-repository)
  - [Folder structure](#folder-structure)
  - [Matter architecture](#matter-architecture)
  - [Building and flashing the example applications](#building-and-flashing-the-example-applications)
    - [Prerequisites](#prerequisites)
    - [Building](#building)
    - [Flashing](#flashing)
  - [Enable serial logging](#enable-serial-logging)
  - [Debugging the example application](#debugging-the-example-application)
  - [Matter test setup](#matter-test-setup)
    - [OpenThread Border Router](#openthread-border-router)
    - [Matter Controller](#matter-controller)
    - [Matter OTA Provider](#matter-ota-provider)
  - [Product Test Component](#product-test-component)
  - [Interesting reads](#interesting-reads)
  - [FCC Notice](#fcc-notice)
  - [More information](#more-information)
---

## Introduction
The [Matter&trade; standard](https://buildwithmatter.com/) (formerly Project Connected Home over IP or Project CHIP) is an industry-unifying standard for IoT. It defines a unified application layer and data model for devices operating with different IP protocols including Wi-Fi and Thread. The Matter stack is developed and maintained as an open-source project. Matter technology is built around four core principles:
<ol>
    <li> Simplicity </li>
    <li> Interoperability </li>
    <li> Reliability </li>
    <li> Security </li>
</ol>

For more information about the Matter standard, please refer to [the Matter codebase used in this SDK](https://github.com/Qorvo/connectedhomeip/tree/v1.0.0.0-qorvo) and the [Connectivity standards alliance website on the Matter protocol](https://csa-iot.org/all-solutions/matter/).

The QPG6105 SoC is a future-proof multi-standard Smart Home communications controller supporting Matter standard. It features
ConcurrentConnect&trade; technology which enables multiple protocols (ZigBee, Thread and/or Bluetooth&reg; LE) to operate
simultaneously in a single chip design. This ensures compatibility with any open low-power standard ecosystem.

The [Qorvo IoT Dev Kit for QPG6105 Product Brief](https://www.qorvo.com/products/d/da008541) provides an overview of the
benefits and features of this development kit.


**Request a kit [here](https://www.qorvo.com/support/how-to-buy/request-a-sample?partNumber=QPG6105DK-01).**

For a complete system solution allowing to evaluate a network consisting of a Matter device and a Thread Border Router
check [Qorvo IoT Dev Kit Pro](Qorvo_IoT_Dev_Kit_Pro.md).

> This Qorvo evaluation board, development board, or development kit and all the features available through it is sold
> and/or licensed to you for evaluation purposes only. You may not use this evaluation board, development board, or
> development kit or any of its features in any revenue-generating products or services.

> **WARNING:** In the QPG6105 radio board design, the diversity antenna efficiency is not fully optimized. It is not
> recommended to evaluate the maximum range of the QPG6105 using the development kit hardware. Qorvo’s production boards
> and customer layouts typically achieve -2 dBm to -3 dBm antenna efficiency.

## Getting started with Qorvo IoT Development Kit for QPG6105

If you got the Qorvo IoT development kit for QPG6105, it comes preprogrammed with the Matter light application that is delivered in source [here](Applications/Matter/light/).

This can directly be used to commission this in a Matter network. If you have the Qorvo IoT Development Kit Pro which includes the Qorvo QPG7015M Gateway development kit, this QPG7015M Gateway can be used as OpenThread Border Router which is a mandatory device for bringing Thread based Matter nodes in the Matter Network. Instructions to set up a Matter network can be found in the guide     - [Commissioning Qorvo Matter™ device with POSIX CLI chip-tool](Documents/Guides/commissioning_posix_cli_chiptool.md).

If you have Qorvo IoT Development Kit for QPG6105 only, you can use a third-party OpenThread Border Router to bring your device in the Matter network. Commissioning guides for Apple and Google are provided in following guides:
- [Commissioning a Qorvo Matter™ end device in the Apple ecosystem](Documents/Guides/commissioning_with_apple.md)
- [Commissioning a Qorvo Matter™ end device in the Google ecosystem](Documents/Guides/commissioning_with_google.md)

Complete [user journey document](Qorvo_User_Journey.md) is made available as well to guide your through all the steps to take from evaluation to production of your Matter devices.

## Installation
The build environment for this SDK requires a Linux operating system. Instructions below are based on Ubuntu 20.04 Linux
distribution.

## How to use this repository?
Qorvo IoT Dev Kit for QPG6105 uses submodules for some of its subcomponents.
This respository contains git LFS files. To be able to clone it successfully the git lfs dependencies need to be installed:
```
sudo apt-get install git
sudo apt-get install git-lfs
```

To clone the repository, use the command-line below:

```
git clone https://github.com/Qorvo/QMatter -b v1.0.1
```

## Folder structure

The Qorvo IoT Dev Kit for QPG6105 repository is structured as follows:

| Folder                                 | Contents                                                                           |
| -------------------------------------- | ---------------------------------------------------------------------------------- |
| **[Applications](Applications/)**      | Contains the application level source code of the sample applications. Provided sample applications are a light, lock, light switch, thermostatic radiator valve and base Matter solution. Next, simple reference applications to demonstrate how to use the QPG6105 peripherals and their drivers are provided. Also, a user mode bootloader reference implementation is provided. Next a Bluetooth LE peripheral reference application is provided. Finally, a Product Test Component (PTC) application is provided for doing RF testing on QPG6105 platform. These can be found in the folders:  **[Matter](Applications/Matter)**, **[Bluetooth LE](Applications/Bluetooth_LE)**, **[Peripherals](Applications/Peripherals)**, **[Bootloader](Applications/Bootloader)** and **[PTC](Applications/PTC)** respectively.                                                |
| **[Binaries](Binaries/)**              | The precompiled and reference binaries of the example projects included in the development kit.                                                     |
| **[Components](Components/)**          | Contains the source code of the Qorvo SW stack (Base Components), Matter and OpenThread glue code and third parties. This source code is used to build the libraries that are used in the link step when building the sample applications.        |
| **[Documents](Documents/)**            | Comprehensive documentation such as user manuals, developer guides, API manuals, etc. can be found under this folder. The documentation that concerns application behavior has been placed with the source code of the corresponding applications as README.mds.                          |
| **[Hardware](Hardware/)** | This folder contains hardware design documents (layout and schematics) of the development board included in the development kit. |
| **[Libraries](Libraries/)**            | Contains the necessary MakeFiles to build the different libraries needed to link against during the building process of the reference applications. MakeFiles are provided for following Libraries: **Qorvo SW stack (Base Components)**, **Matter SW stack**, **Matter - Qorvo glue**, **OpenThread - Qorvo glue**, **Bootloader** and **Factory block**.                                   |
| **[Scripts](Scripts/)**                    | This folder contains the needed scripts for automatic setup of the build environment. It will make sure all needed dependencies for Matter development and testing are configured on the build machine.                                      |
| **[Tools](Tools/)**                    | General-purpose tools (such as the JadeLogger, Matter controllers, Matter OTA providers, OTA tools, Device attestation credentials generator, Factory block generator, Application Creator, ZAP tooling and PTC tooling) are located here.                                      |
| **[make](make/)**                      | Contains the common make rules to apply during building.                                  |

## Matter architecture

In the picture below you can find an overview of a Matter node architecture:

<div align="center">
  <img src="Images/architecture.png" alt="Matter application architecture" width="500">
</div>

A Matter node represents a single device in the Matter network. Each Matter node application layer (dark blue blocks) is
defined by Matter data model layer. The data model layer is defined by a set of clusters. These clusters contain
commands and attributes that can be accessed over the Matter network. There are two types of clusters. **Base clusters**
and **Application clusters**. Base clusters are clusters that do not implement an application specific feature set. These
are used for management and diagnostic purposes in the Matter network. These are common for all Matter devices.
Application clusters are used to enable specific functionalities that are application specific (e.g. Temperature
measurement, On/Off and level for lighting, etc.). The **Application layer** implements the actual product behavior such
as the toggling/dimming the light, temperature sensing, etc...

Next, there is the actual **Matter stack**. The Matter stack is built as a library and contains the software that is
hosted in the Matter open-source repository. As part of the Matter network layer, the **OpenThread stack** is used.
Its source code is hosted in the OpenThread open-source repository. This software stack is built as part of the
[QMatter examples builds that can be found in Applications/Matter](Applications/Matter).

Next, there are the glue components that implement a glue layer between the Matter/OpenThread stack and the Qorvo
platform code. These glue components are **Matter-Qorvo glue** and **OpenThread-Qorvo glue**. These are also delivered
as seperate libraries ([Matter-Qorvo glue](Libraries/Qorvo/MatterQorvoGlue) and
[OpenThread-Qorvo glue](Libraries/Qorvo/OpenThreadQorvoGlue)).

Finally, There is the **Qorvo stack** which implements the IEEE802.15.4 stack, Bluetooth® LE stack, Security code and
Hardware Abstraction Layer (HAL) towards the QPG6105. This library also contains the Board Support Package (BSP) that
can be updated to match the final product hardware. This [Qorvo stack is provided as a library build](Libraries/Qorvo/QorvoStack) as well.

## Building and flashing the example applications

### Prerequisites
Qorvo IoT Dev Kit for QPG6105 is based on Make.

The toolchain being used for this SDK is the GNU ARM Embedded Toolchain (version 9-2019-q4-major). This is a ready-to-use, open-source suite of tools for C, C++ and assembly programming.

Matter code requires the [GN](https://gn.googlesource.com/gn/) tool to be installed. This is a meta-build system that generates build files for [Ninja](https://ninja-build.org/).

To make sure all prerequisites are fulfilled and before running any build command, the `Scripts/activate.sh` environment setup script should be sourced at the top level. This script takes care of downloading the toolchain, GN, ninja, and setting up the build environment.

```
source Scripts/activate.sh
```

The first time this scripts gets executed, it will create the environment from scratch which takes some time for downloading and installing.

### Building
First make sure the environment and environment variables are set correctly as mentioned above:

```
source Scripts/activate.sh
```

All builds are based on Make

```
cd Applications/Matter/light
make -f Makefile.light_qpg6105_development clean
make -f Makefile.light_qpg6105_development
```

This will result in `.hex` in `Work/light_qpg6105/light_qpg6105.hex`

### Flashing
Make sure the development kit is connected to your computer using the USB-C port on the IoT Dev Kit for QPG6105.

<div align="center">
  <img src="Images/board_top.jpg" alt="QPG6105 Smart Home and Lighting Carrier Board" width="400">
</div>

The development kit has an onboard SEGGER J-Link debug probe. When the device is enumerated successfully, a virtual
COM port (`/dev/ttyACMx`) and a storage device (*JLINK*) will be available for use.

The J-Link Software and Documentation pack can be downloaded from
[https://www.segger.com/downloads/jlink/](https://www.segger.com/downloads/jlink/).
Install it to `/opt/SEGGER`, and create a symlink to make this latest version available under `JLink`:

```
sudo mkdir -p /opt/SEGGER/JLink
cd /opt/SEGGER
sudo tar xf /path/to/downloaded/JLink_Linux_V786h_x86_64.tgz
sudo ln -s JLink_Linux_V786h_x86_64 JLink
```

For easy programming of the `.hex` you can use the `program` target, as illustrated with following command:

```
make -f Makefile.light_qpg6105_development program
```

The same procedure can be followed for building the other Matter applications and the peripheral example applications.

In Windows, after installed the JLink software, open J-FLash Lite Vx.xxY (e.g. V7.80c). In J-Flash GUI, choose
Device: "QPG6105", Interface: "SWD" "4000kHz" and then press "OK". Erase chip before programming the device. Browse
the application image (base_qpg6105_xxxx.hex, light_qpg6105_xxxx.hex, lock_qpg6105_xxxx.hex, switch_qpg6105_xxxx.hex
or thermostaticRadiatorValve_qpg6105_xxxx.hex) you want to program, and then click "Program Device". This operation
will trigger the development board hardware to transfer the image file to the QPG6105's program flash.Finally, the
board gets flashed in a couple of seconds. Note that pre-build hex-files are provided under [Binaries/](Binaries/),
while the output of locally built applications will be found under Work/.


## Enable serial logging
First make sure the development kit is connected to your computer using the USB-C port on the IoT Dev Kit for QPG6105.
As mentioned above, a virtual COM port (`/dev/ttyACMx`) will be available for use.

Next you can use a serial terminal application to connect to the COM port. In this example, we will be using Minicom.
This tool can be installed using following command:

```
sudo apt-get install minicom
```
After the installation, start minicom using following options:

```
minicom -D /dev/ttyACM0 115200
```

Please note that the COM port number and device label **may differ** on your computer.

After resetting the programmed QPG6105 (press the button `SW6 RADIO RESET`), you will see similar output as below:

```
NRT ROM v1
qvCHIP <version> ROMv1/1 (CL:0) r:3
ResetCount[0]
[P][DL] BLEManagerImpl::Init() complete
[P][-] Initializing OpenThread stack
[P][DL] OpenThread started: OK
[P][DL] Setting OpenThread device type to ROUTER
[P][-] Starting OpenThread task
[P][-] Starting Platform Manager Event Loop
[P][-] ============================
[P][-] Qorvo <application>-app Launching
[P][-] ============================
```

## Debugging the example application

The IoT Dev Kit for QPG6105 has an onboard JLink based debugger which can be used to debug the reference applications in this SDK. Refer to [this debugging guide](Documents/Guides/debugging_with_segger_ozone.md) if you are interested in using Segger Ozone for debugging.

## Matter test setup
To start using the Matter applications, we need a 802.15.4-enabled Matter network to commission the device. This
requires a Thread Border Router and a Matter controller. For doing Over-the-Air download and upgrade also a Matter
OTA provider is needed. As part of the Matter SDK, these tools are provided as well.

### OpenThread Border Router
Implementation of the OpenThread Border Router is part of Qorvo IoT Dev Kit for QPG7015M. Refer to [QGateway](https://github.com/Qorvo/QGateway) if you are interested in building an OpenThread Border Router as part of the Matter network.

In this SDK, step-by-step instructions are given how to setup an Matter network based on this QPG7015M Dev Kit. See
[How to setup the OpenThread Border Router](Documents/Guides/setup_qpg7015m_ot_borderrouter.md)

### Matter Controller
Commissioning a device onto the Matter network is done by initiating a connection over Bluetooth LE between the Matter
controller and Matter device. Here the Matter controller has the commissioner role. The commissioner is responsible for
providing all necessary information to the Matter device to securely operate in the Matter network. This information
includes:
- **Onboarding information** (device discriminator, setup PIN code). This information needs to come from the Matter device itself (encoded in
a QR code or exposed through UART interface).
- **Thread credentials**. These are the Thread network credentials so it can join the 802.15.4 based Thread mesh network.

A terminal based Matter controller called **chip-tool** is provided for development and testing purposes and can be used
on a PC with Ubuntu 20.04+ or RPi model 4 running Ubuntu 20.04+.  This tool is developed by the CSA Matter development
team, and we provide [Instructions for commissioning and controlling the Matter device](Documents/Guides/commissioning_posix_cli_chiptool.md).

### Matter OTA Provider
To do a device upgrade over the Matter protocol, an OTA provider node needs to be available in the Matter network. An OTA provider
node is the node that can supply the available software update to the OTA requestor node. In this case the QPG6105 Matter
node will be the OTA requestor. This node will query the OTA provider node for a software update.

The OTA provider application is made available for testing purposes in this SDK (for Linux 64-bit PC/RPi). Step-by-step
instructions how to use the tool can be found [here](Documents/Guides/ota_device_firmware_upgrade.md).

## Product Test Component
Product Test Component (PTC) is a piece of software that is used to configure the radio chip in test modes enabling
measurements of the RF performance during design, validation, certification or production testing. To get started with PTC,
please refer to this [quick reference](Applications/PTC/README.md).

> **WARNING:** To control the PTC software, additional tooling, called Radio Control Console (RCC), on an external PC is
> needed. This RCC tooling is only supported for Windows machines. So make sure if you want to start using PTC software,
> a Windows machine is at hand.

> **WARNING:** In the QPG6105 radio board design, the diversity antenna efficiency is not fully optimized. It is not
> recommended to evaluate the maximum range of the QPG6105 using the development kit hardware. Qorvo’s production boards
> and customer layouts typically achieve -2 dBm to -3 dBm antenna efficiency.

## Interesting reads
- [Device Attestation integration in Qorvo QMatter SDK](Documents/Guides/device_attestation.md)
- [Matter API usage in Qorvo QMatter SDK](Documents/API%20Manuals/Matter/Matter_API_Manual.md)
- [How to create a Matter device](Applications/Matter/base/README.md#creating-matter-device)
- [How to configure for a production build](Documents/Production%20Setup%20Guides)
- [Make build flow in Qorvo QMatter SDK](Documents/Guides/make_build_flow.md)
- [Use the Matter multi-admin feature with a Qorvo Matter™ end device with the Apple Home and Google Home ecosystems](Documents/Guides/commission_with_multi_admin_feature.md)
- [Commissioning Qorvo Matter™ device with POSIX CLI chip-tool](Documents/Guides/commissioning_posix_cli_chiptool.md)
- [Commissioning a Qorvo Matter™ end device in the Apple ecosystem](Documents/Guides/commissioning_with_apple.md)
- [Commissioning a Qorvo Matter™ end device in the Google ecosystem](Documents/Guides/commissioning_with_google.md)
- [Designing with J-Link OB for prototyping](Documents/Guides/designing_with_jlinkob.md)
- [Debugging with Segger Ozone](Documents/Guides/debugging_with_segger_ozone.md)
- [ConcurrentConnect&trade; Technology configuration](Documents/Guides/concurrent_connect_technology_configuration.md)

## FCC Notice
This kit is designed to allow:

1. Product developers to evaluate electronic components, circuitry, or software associated with the kit to determine whether to incorporate such items in a finished product and
2. Software developers to write software applications for use with the end product. This kit is not a finished product and when assembled may not be resold or otherwise marketed unless all required FCC equipment authorizations are first obtained. Operation is subject to the condition that this product not cause harmful interference to licensed radio stations and that this product accept harmful interference. Unless the assembled kit is designed to operate under part 15, part 18 or part 95 of this chapter, the operator of the kit must operate under the authority of an FCC license holder or must secure an experimental authorization [under part 5 of this chapter](https://www.govinfo.gov/content/pkg/CFR-2013-title47-vol1/pdf/CFR-2013-title47-vol1-sec2-803.pdf).

## More information

Please visit www.qorvo.com for more information on our product line and support options.
