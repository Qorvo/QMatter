# QPG6105 SDK for Matter

Welcome to the QPG6105 Matter SDK repository! This repository contains the Matter software development kit for Qorvo's QPG6105 SoC. It supports you to develop applications that are compatible with Matter. The Qorvo development kit helps product developers bring Matter products to the market in a fast and easy way.

<div align="center">
  <img src="Documents/Images/qorvo_matter.png" alt="Qorvo Matter">
</div>

---
- [Introduction](#introduction)
- [Installation](#installation)
- [How to use this repository?](#how-to-use-this-repository)
- [Folder structure](#folder-structure)
- [Matter architecture](#matter-architecture)
- [Example applications](#example-applications)
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
- [More information](#more-information)
---

## Introduction
[Matter](https://buildwithmatter.com/) (formerly Project Connected Home over IP or Project CHIP) is an industry-unifying standard for IoT. It defines a unified application layer and data model for devices operating with different IP protocols including Wi-Fi and Thread. The Matter stack is developed and maintained as an open-source project. Matter is built around four core principles:
<ol>
    <li> Simplicity </li>
    <li> Interoperability </li>
    <li> Reliability </li>
    <li> Security </li>
</ol>

For more information about the Matter standard, please refer to [Matter's GitHub](https://github.com/Qorvo/connectedhomeip/tree/v0.9.9.1).

> **WARNING:** Matter is still in a development stage and as a result the software in this development kit is not final.
> To not break dependencies between the HAL, the example applications and the Matter stack, this development kit
> repository adds a validated development branch of Matter as a GIT submodule.

The QPG6105 SoC is a future-proof multi-standard Smart Home communications controller supporting Matter. It features
ConcurrentConnect&trade; technology which enables multiple protocols (ZigBee, Thread and/or Bluetooth® LE) to operate
simultaneously in a single chip design. This ensures compatibility with any open low-power standard ecosystem.

To request your QPG6105 Matter Development Kit, contact Qorvo using this link: [Request a sample](https://www.qorvo.com/support/how-to-buy/request-a-sample?partNumber=Matter%20QPG6105DK)

> This Qorvo evaluation board, development board, or development kit and all the features available through it is sold
> and/or licensed to you for evaluation purposes only. You may not use this evaluation board, development board, or
> development kit or any of its features in any revenue-generating products or services.

> **WARNING:** In the QPG6105 radio board design, the diversity antenna efficiency is not fully optimized. It is not
> recommended to evaluate the maximum range of the QPG6105 using the development kit hardware. Qorvo’s production boards
> and customer layouts typically achieve -2 dBm to -3 dBm antenna efficiency.

## Installation
The build environment for this SDK requires a Linux operating system. Instructions below are based on Ubuntu 20.04 Linux
distribution.

## How to use this repository?
QPG6105 Matter SDK uses submodules for some of its subcomponents. To clone the repository, use the command-line below:

```
git clone https://github.com/Qorvo/QMatter -b v0.8.1.0
```

## Folder structure

The QPG6105 Matter repository is structured as follows:

| Folder                                 | Contents                                                                           |
| -------------------------------------- | ---------------------------------------------------------------------------------- |
| **[Documents](Documents/)**            | Comprehensive documentation such as user manuals, developer guides, API manuals and etc. can be found under this folder. The documentation that concerns application behavior has been placed with the source code of the corresponding applications as README.mds.                          |
| **[Hardware](Hardware/)** | This folder contains hardware design documents (layout and schematics) of the development board included in the development kit. |
| **[Applications](Applications/)**      | Contains the application level source code of the sample applications. Provided sample applications are a light, lock, base Matter solution. Next, simple reference applications to demonstrate how to use the QPG6105 peripherals and their drivers are provided. Finally, a user mode bootloader reference implementation is provided as well. These can be found in the folders:  **[Matter](Applications/Matter)**, **[Peripherals](Applications/Peripherals)** and **[Bootloader](Applications/Bootloader)** respectively.                                                |
| **[Libraries](Libraries/)**            | Contains the necessary MakeFiles to build the different libraries needed to link against during the building process of the reference applications. MakeFiles are foreseen for following Libraries: **Qorvo SW stack (Base Components)**, **Matter SW stack**, **Matter - Qorvo glue**, **OpenThread - Qorvo glue**.                                   |
| **[Components](Components/)**          | Contains the source code of the Qorvo SW stack (Base Components), Matter and OpenThread glue code and third parties. This source code is used to build the libraries that are used in the link step when building the sample applications.        |
| **[make](make/)**                      | Contains the common make rules to apply during building.                                  |
| **[Binaries](Binaries/)**              | The precompiled and reference binaries of the example projects included in the development kit.                                                     |
| **[Tools](Tools/)**                    | General-purpose tools (such as the JadeLogger, Matter controllers, Matter OTA providers, OTA tools) are located here.                                      |

## Matter architecture

In below picture you can find an overview of a Matter node architecture:

<div align="center">
  <img src="Documents/Images/architecture.png" alt="Matter application architecture" width="500">
</div>

A Matter node represents a single device in the Matter network. Each Matter node application layer (dark blue blocks) is
defined by Matter's data model layer. The data model layer is defined by a set of clusters. These clusters contain
commands and attributes that can be accessed over the Matter network. There are two types of clusters. **Base clusters**
and **Application clusters**. Base clusters are clusters that do not implement an application specific feature set. These
are used for management and diagnostic purposes in the Matter network. These are common for all Matter devices.
Application clusters are used to enable specific functionalities that are application specific (e.g. Temperature
measurement, On/Off and level for lighting, etc.). The **Application layer** implements the actual product behavior such
as the toggling/dimming the light, temperature sensing, etc...

Next, there is the actual **Matter stack**. The Matter stack is built as a library and contains the software that is
hosted in the Matter open-source repository. As part of the Matter network layer, the **OpenThread stack** is used of
which the software is hosted in the OpenThread open-source repository. This software stack is built as part of the
Matter library that can be found [here](Libraries/ThirdParty/Matter).

Next, there are the glue components that implement a glue layer between the Matter/OpenThread stack and the Qorvo
platform code. These glue components are **Matter-Qorvo glue** and **OpenThread-Qorvo glue**. These are also delivered
as seperate libraries ([Matter-Qorvo glue](Libraries/Qorvo/MatterQorvoGlue) and
[OpenThread-Qorvo glue](Libraries/Qorvo/OpenThreadQorvoGlue)).

Finally, There is the **Qorvo stack** which implements the IEEE802.15.4 stack, Bluetooth® LE stack, Security code and
Hardware Abstraction Layer (HAL) towards the QPG6105. This library also contains the Board Support Package (BSP) that
can be updated to match the final hardware product. This Qorvo stack is made available as library as well ([Qorvo stack](Libraries/Qorvo/QorvoStack))

## Example applications

Several example applications are provided to demonstrate a Matter device with Thread connectivity, using Bluetooth® LE to perform Matter
provisioning. These examples are compatible with the Qorvo QPG6105 development boards. More information about each application can be found in
the `README.md` file found in the `Applications/Matter/<app>` directories.

Following turn-key Matter solutions are provided as reference:
 - [light](Applications/Matter/light) - This reference application demonstrates a Matter dimmable color light.
 - [lock](Applications/Matter/lock) - This reference application demostrates a Matter door lock.
 - [base](Applications/Matter/base) - This reference application is a Matter base application that is easy customizable to develop any custom Matter application.

The QPG6105 SDK for Matter also comes with seven different peripheral example applications. All sources and quick reference documentation for these example applications can
be found [here](Applications/Peripherals). Applications that are provided are:

- [ADC](Applications/Peripherals/adc): Analog-to-digital conversion example. Temperature, battery voltage, and one analog I/O pin are read and converted to digital. <br/>
- [GPIO](Applications/Peripherals/gpio): General purpose I/O example. <br/>
- [LED](Applications/Peripherals/led): An LED dimming control example. <br/>
- [SPI](Applications/Peripherals/mspi): Reads/write data from/to the external NOR flash through the SPI bus. <br/>
- [TWI](Applications/Peripherals/mtwi): Reads/write data from/to the humidity sensor through the TWI bus. <br/>
- [PWM](Applications/Peripherals/pwm): A PWM example to control RGB LEDs. <br/>
- [UART](Applications/Peripherals/uart): "Hello, World" Example. <br/>

## Building and flashing the example applications

### Prerequisites
QPG6105 Matter SDK is based on Make.

The toolchain being used for this SDK is the GNU ARM Embedded Toolchain (version 9-2019-q4-major). This is a ready-to-use, open-source suite of tools for C, C++ and assembly programming.

Matter requires the [GN](https://gn.googlesource.com/gn/) tool to be installed. This is a meta-build system that generates build files for [Ninja](https://ninja-build.org/).

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
cd Applications/Matter/Light
make -f Makefile.light_qpg6105 clean
make -f Makefile.light_qpg6105
```

This will result in `.hex` in `Work/light_qpg6105/light_qpg6105.hex`

### Flashing
Make sure the development kit is connected to your computer using the USB-c port on the smart home & lighting carrier
board.

<div align="center">
  <img src="Documents/Images/board_top.png" alt="QPG6105 Smart Home and Lighting Carrier Board" width="400">
</div>

The development kit has an onboard SEGGER J-Link debug probe. When the device is enumerated successfully, a virtual
COM port (`/dev/ttyACMx`) and a storage device (*JLINK*) will be available for use.

For easy programming of the `.hex` you can use following command:

```
make -f Makefile.light_qpg6105 program DRIVE=/media/$USER/JLINK
```

The same procedure can be followed for building the other Matter applications and the peripheral example applications.

Note that during programming only the program space in the chip is re-flashed. So all non-volatile memory is kept intact.
This allows you to reflash the Matter device without losing the Thread network credentials. If you want to be in a
factory fresh state after programming, refer to [factory reset of the Matter Light](Applications/Matter/light/README.md#factory-reset),
[factory reset of the Matter Lock](Applications/Matter/lock/README.md#factory-reset) or [factory reset of the Matter Base application](Applications/Matter/base/README.md#factory-reset)

An alternative way of programming the device is by using drag 'n drop. Using a file browser, navigate to the hex-file
you want to program (for example, navigate to the Binaries/Matter folder). Next, highlight one of the reference
application images (light_qpg6105.hex, lock_qpg6105.hex or base_qpg6105.hex) and select copy. Next, go to JLink storage
device's directory and paste the file. This operation will trigger the development board hardware to transfer the image
file to the QPG6105's program flash. Finally, the board gets flashed in a couple of seconds.


## Enable serial logging
First make sure the development kit is connected to your computer using the USB-c port on the smart home & lighting carrier
board. As mentioned above, a virtual COM port (`/dev/ttyACMx`) will be available for use.

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

The QPG6105 Smart Home and Carrier Board has an onboard JLink Base debugger which can be used to debug the reference applications in this SDK. Refer to [this](Documents/Guides/debugging_with_segger_ozone.md) guide if you are interested in using Segger Ozone for debugging.

## Matter test setup
To start using the Matter applications, we need a 802.15.4-enabled Matter network to commission the device. This
requires a Thread Border Router and a Matter controller. For doing Over-the-Air download and upgrade also a Matter
OTA provider is needed. As part of the Matter SDK, these tools are provided as well.

### OpenThread Border Router
Implementation of the OpenThread Border Router is part of Qorvo's Gateway SDK based on QPG7015M. Refer to [QGateway](https://github.com/Qorvo/QGateway) if you are interested in building an OpenThread Border Router as part of the Matter network.

In this SDK, step-by-step instructions are given how to setup an Matter network based on this QPG7015M DK. See
[How to setup the OpenThread Border Router](Documents/Guides/setup_qpg7015m_ot_borderrouter.md)

### Matter Controller
Commissioning a device onto the Matter network is done by initiating a connection over Bluetooth LE between the Matter
controller and Matter device. Here the Matter controller has the commissioner role. The commissioner is responsible for
providing all necessary information to the Matter device to securely operate in the Matter network. This information
includes:
- **Onboarding information** (device descriminator, setup PIN code). This information needs to come from the Matter device itself (encoded in
a QR code or exposed through UART interface).
- **Thread credentials**. These are the Thread network credentials so it can join the 802.15.4 based Thread mesh network.

Several Matter controllers are available for use:
- Android&trade; chip-tool (smartphone with Android 8+) - Instructions for commissioning and controlling the Matter device
can be found [here](Documents/Guides/commissioning_android_chiptool.md)
- Terminal based chip-tool (PC with Ubuntu 20.04+ or RPi model 4 running Ubuntu 20.04+) - Instructions for commissioning
and controlling the Matter device can be found [here](Documents/Guides/commissioning_posix_cli_chiptool.md)

### Matter OTA Provider
To do a device upgrade over Matter, an OTA provider node needs to be available in the Matter network. An OTA provider
node is the node that can supply the available software update to the OTA requestor node. In this case the QPG6105 Matter
node will be the OTA requestor. This node will query the OTA provider node for a software update.

The OTA provider application is made available for testing purposes in this SDK (for Linux 64-bit PC/RPi). Step-by-step
instructions how to use the tool can be found [here](Documents/Guides/ota_device_firmware_upgrade.md).

## Product Test Component
Product Test Component (PTC) is a piece of software that is used to configure the radio chip in test modes enabling
measurements of the RF performance during design, validation, certification or production testing. To get started with PTC,
please refer to this [quick reference](Documents/User%20Manuals/PTC/GP_P345_UM_19411_Quick_Reference_PTC_DK.pdf).

> **WARNING:** To control the PTC software, additional tooling, called Radio Control Console (RCC), on an external PC is
> needed. This RCC tooling is only supported for Windows machines. So make sure if you want to start using PTC software,
> a Windows machine is at hand.

> **WARNING:** In the QPG6105 radio board design, the diversity antenna efficiency is not fully optimized. It is not
> recommended to evaluate the maximum range of the QPG6105 using the development kit hardware. Qorvo’s production boards
> and customer layouts typically achieve -2 dBm to -3 dBm antenna efficiency.

## More information

Please visit www.qorvo.com for more information on our product line and support options.
