# QPG6105 SDK for Matter

Welcome to the QPG6105 Matter SDK repository! This repository contains the Matter software development kit for Qorvo's QPG6105 SoC. It supports you to develop applications that are compatible with Matter. The Qorvo development kit helps product developers bring Matter products to the market in a fast and easy way.

<div align="center">
  <img src="Documents/Images/qorvo_matter.png" alt="Qorvo Matter">
</div>

---

- [QPG6105 SDK for Matter](#qpg6105-sdk-for-matter)
    - [Introduction](#introduction)
    - [Installation](#installation)
    - [How to use this repository?](#how-to-use-this-repository)
    - [Folder structure](#folder-structure)
    - [Example applications](#example-applications)
        - [Matter examples](#matter-examples)
            - [Matter light and lock](#matter-light-and-lock)
            - [Test setup](#test-setup)
                - [Thread Border Router](#thread-border-router)
                - [Matter Controller](#matter-controller)
                    - [Android chip-tool](#android-chip-tool)
                    - [POSIX CLI chip-tool](#posix-cli-chip-tool)
    - [Compilation and programming](#compilation-and-programming)
        - [Prerequisites](#prerequisites)
        - [Building](#building)
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

For more information about the Matter standard, please refer to [Matter's GitHub](https://github.com/Qorvo/connectedhomeip/tree/v0.9.7.1).

> **WARNING:** Matter is still in a development stage and as a result the software in this development kit is not final.
> To not break dependencies between the HAL, the example applications and the Matter stack, this development kit
> repository adds a validated development branch of Matter as a GIT submodule.

The QPG6105 SoC is a future-proof multi-standard Smart Home communications controller supporting Matter. It features
ConcurrentConnect&trade; technology which enables multiple protocols (ZigBee, Thread and/or Bluetooth® LE) to operate
simultaneously in a single chip design. This ensures compatibility with any open low-power standard ecosystem.

To request your QPG6105 Matter Development Kit, contact Qorvo using below link:
https://www.qorvo.com/support/how-to-buy/request-a-sample?partNumber=Matter%20QPG6105DK

## Installation
Setting up the build environment and building the example applications has been tested on:
<ol>
    <li> Ubuntu 20.04 LTS </li>
    <li> Windows Subsystem for Linux (WSL) - Ubuntu 18.04 LTS </li>
</ol>

It is strongly advised to run one of both options mentioned above.

## How to use this repository?
QPG6105 Matter SDK uses submodules for some of its subcomponents. To clone the repository, use the command-line below:

```
git clone --recurse-submodules -j8 https://github.com/Qorvo/QMatter -b v0.7.0.0
```

## Folder structure

The QPG6105 Matter repository is structured as follows:

| Folder                                 | Contents                                                                           |
| -------------------------------------- | ---------------------------------------------------------------------------------- |
| **[Documents](Documents/)**            | Comprehensive documentation such as user manuals, developer guides, API manuals and etc. can be found under this folder. The documentation that concerns application behavior has been placed with the source code of the corresponding applications as README.mds.                          |
| **[Hardware](Hardware/)** | This folder contains hardware design documents (layout and schematics) of the development board included in the development kit. |
| **[Applications](Applications/)**      | Contains the application level source code of the sample applications. Provided sample applications are a light, lock Matter solution and also several simple reference applications to demonstrate how to use the QPG6105 peripherals and their drivers. These can be found in the folders:  **[Matter](Applications/Matter)** and **[Peripherals](Applications/Peripherals)** respectively.                                                |
| **[Libraries](Libraries/)**            | Contains the necessary MakeFiles to build the different libraries needed to link against during the building process of the reference applications. MakeFiles are foreseen for following Libraries: **Qorvo SW stack (Base Components)**, **Matter SW stack**, **Matter - Qorvo glue**, **OpenThread - Qorvo glue**.                                   |
| **[Components](Components/)**          | Contains the source code of the Qorvo SW stack (Base Components), Matter and OpenThread glue code and third parties. This source code is used to build the libraries that are used in the link step when building the sample applications.        |
| **[make](make/)**                      | Contains the common make rules to apply during building, also includes programmer rule to flash the binary to the Qorvo development board.                                  |
| **[Binaries](Binaries/)**              | The precompiled and reference binaries of the example projects included in the development kit.                                                     |
| **[Tools](Tools/)**                    | General-purpose tools (such as the JadeLogger, Matter controllers) are located here.                                      |

## Example applications

Several example applications are provided to demonstrate a Matter device with Thread connectivity, using Bluetooth® LE to perform Matter
provisioning. These examples are compatible with the Qorvo QPG6105 development boards. More information about each application can be found in
the `README.md` file found in the `Applications/Matter/<app>` directories.

Following turn-key Matter solutions are provided as reference:
 - [light](Applications/Matter/light)
 - [lock](Applications/Matter/lock)

The QPG6105 SDK for Matter also comes with seven different peripheral example applications. All sources and quick reference documentation for these example applications can
be found [here](Applications/Peripherals). Applications that are provided are:

- [ADC](Applications/Peripherals/adc): Analog-to-digital conversion example. Temperature, battery voltage, and one analog I/O pin are read and converted to digital. <br/>
- [GPIO](Applications/Peripherals/gpio): General purpose I/O example. <br/>
- [LED](Applications/Peripherals/led): An LED dimming control example. <br/>
- [SPI](Applications/Peripherals/mspi): Reads/write data from/to the external NOR flash through the SPI bus. <br/>
- [TWI](Applications/Peripherals/mtwi): Reads/write data from/to the humidity sensor through the TWI bus. <br/>
- [PWM](Applications/Peripherals/pwm): A PWM example to control RGB LEDs. <br/>
- [UART](Applications/Peripherals/uart): "Hello, World" Example. <br/>

### Matter examples

#### Matter light and lock

1) Connect the development kit to your computer using the USB-C port on the smart home & lighting carrier board.

<div align="center">
  <img src="Documents/Images/board_top.png" alt="QPG6105 Smart Home and Lighting Carrier Board">
</div>

2) The development kit has an onboard SEGGER J-Link debug probe. When the device is enumerated successfully, a virtual
COM port (`/dev/ttyACMx` on Unix or `/dev/ttySx` on WSL) and a storage device (*JLINK*) will be available for use.

3) Use a serial terminal application to connect to the COM port. In this example, we will be using Minicom. This tool can be installed using following command:

```
sudo apt-get install minicom
```
After the installation, start minicom using following options:

```
minicom -D /dev/ttyACM0 115200
```

Please note that the COM port number and device label **may differ** on your computer.

4) The QPG6105 Matter SDK provides precompiled binaries. Using a file browser, navigate to the Binaries/Matter folder, highlight one of the reference application images (light_qpg6105.hex or lock_qpg6105.hex) and select copy. Next, go to JLink storage device's directory and paste the file.  This operation will trigger the development board hardware to transfer the image file to the QPG6015's program flash.

5) The board gets flashed in a couple of seconds, and the Matter light/lock logging should become visible in the minicom console. Also, the red-led (LD4) on the board starts blinking.

```
qvCHIP <version> ROM<vx> (CL:xxxxxx) r:x
[P][-] Init CHIP Stack
[P][DL] BLEManagerImpl::Init() complete
[P][-] Initializing OpenThread stack
[P][DL] OpenThread ifconfig up and thread start
[P][DL] OpenThread started: OK
[P][DL] Setting OpenThread device type to MINIMAL END DEVICE
[P][-] Starting OpenThread task
[P][-] Starting Platform Manager Event Loop
[P][-] ============================
[P][-] Qorvo <application>-app Launching
[P][-] ============================
```
To start using the Matter applications, we need a 802.15.4-enabled Matter network to commission the device. This
requires a Thread Border Router and a Matter controller.

#### Test setup

##### Thread Border Router

Setting up a Matter Network can be achieved by forming an OpenThread network based on Qorvo's QPG7015M OpenThread SDK. Please refer to
[How to setup the OpenThread Border Router](Documents/Guides/setup_ot_borderrouter.md).

##### Matter Controller

Commissioning a device onto the Matter network is done by initiating a connection over Bluetooth LE between the Matter
controller and Matter device. Here the Matter controller has the commissioner role. The commissioner is responsible for
providing all necessary information to the Matter device to securely operate in the Matter network. This information
includes:
- **Onboarding information** (device descriminator, setup PIN code). This information needs to come from the Matter device itself (encoded in
a QR code or exposed through UART interface).
- **Thread credentials**. These are the Thread network credentials so it can join the 802.15.4 based Thread mesh network.

Several Matter controllers are available for use:
- Android&trade; chip-tool (smartphone with Android 8+)
- Terminal based chip-tool (PC with Ubuntu 20.04+ or RPi model 4 running Ubuntu 20.04+ instead of Raspbian OS)

###### Android chip-tool

This Matter controller allows you to test Matter applications using an Android 8+ smartphone. Features of this tool are:
- Scan a Matter QR code and display payload information to the user
- Commission a Matter device
- Send echo requests to the Matter echo server
- Send on/off cluster requests to a Matter device

To start using this application you can install the [Android chip-tool apk on your smartphone from this location](Tools/MatterControllers/Android).
Another option is to build it from source. Instructions how to do this can be found [here](https://github.com/Qorvo/connectedhomeip/tree/v0.9.7.1/docs/guides/android_building.md).

To start using this tool as commissioner, please refer to [Commissioning Qorvo Matter device with Android chip-tool](Documents/Guides/commissioning_android_chiptool.md). In this guide following steps will be explained:
- Installing the Android chip-tool.
- Preparing the OpenThread Border Router.
- Preparing the Matter device for commissioning.
- Commissioning the Matter device onto the Matter network.
- Sending commands to control the Matter device.

###### POSIX CLI chip-tool

This Matter controller allows you to test Matter applications using a PC running Ubuntu 20.04+ or RPi4 running Ubuntu 20.04+. Features are:
- Onboarding using setup code (no QR code)
- Commission a Matter device
- Send echo requests to the Matter echo server
- Send Cluster commands to a Matter device including reading/writing cluster attributes

To start using this application you can install the binary of chip-tool on your PC or RPi from these locations: [POSIX CLI chip-tool for PC](Tools/MatterControllers/PC) or [POSIX CLI chip-tool for RPi](Tools/MatterControllers/RPi).
Another option is to build it from source. Instructions how to do this can be found [here](https://github.com/Qorvo/connectedhomeip/tree/v0.9.7.1/examples/chip-tool)

To start using this tool as commissioner, please refer to [Commissioning Qorvo Matter device with POSIX CLI chip-tool](Documents/Guides/commissioning_posix_cli_chiptool.md). In this guide following steps will be explained:
- Installing the POSIX CLI chip-tool on RPi4 or PC.
- Preparing the OpenThread Border Router.
- Preparing the Matter device for commissioning.
- Commissioning the Matter device onto the Matter network.
- Sending commands to control the Matter device.

After your Matter device is commissioned, you can find some more details on the actual device behavior on following pages,
dependent on the Matter device you used for commissioning:
- To start using the Matter light application please refer to [light](Applications/Matter/light/README.md).
- To use the Matter lock application refer to [lock](Applications/Matter/lock/README.md)

To start building the Matter examples please keep reading the document.

## Compilation and programming

### Prerequisites
QPG6105 Matter SDK is based on Make.

The toolchain being used for this SDK is the GNU ARM Embedded Toolchain (version 9-2019-q4-major). This is a ready-to-use, open-source suite of tools for C, C++ and assembly programming.

Matter requires the [GN](https://gn.googlesource.com/gn/) tool to be installed. This is a meta-build system that generates build files for [Ninja](https://ninja-build.org/).

To make sure all prerequisites are fulfilled and before running any build command, the `Scripts/activate.sh` environment setup script should be sourced at the top level. This script takes care of downloading the toolchain, GN, ninja, and setting up the build environment.

```
source Scripts/activate.sh
```

The first time this scripts gets executed, it will create the environment from scratch which takes some time for downloading and intalling.

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

For easy programming of the `.hex` an additional make rule is added

- For Linux:
```
make -f Makefile.light.qpg6105 program DRIVE=/media/<user>/JLINK
```
- For WSL:
```
make -f Makefile.light.qpg6105 program DRIVE=/mnt/<JLINK mount point>
```

The same procedure can be followed for building the Matter Lock application and the peripheral example applications.

Note that during programming only the program space in the chip is re-flashed. So all non-volatile memory is kept intact.
This allows you to reflash the Matter device without losing the Thread network credentials. If you want to be in a
factory fresh state after programming, refer to [factory reset of the Matter Light](Applications/Matter/light/README.md#factory-reset)
or [factory reset of the Matter Lock](Applications/Matter/lock/README.md#factory-reset)

## Product Test Component

Product Test Component (PTC) is a piece of software that is used to configure the radio chip in test modes enabling
measurements of the RF performance during design, validation, certification or production testing. To get started with PTC,
please refer to this [quick reference](Documents/User%20Manuals/PTC/GP_P345_UM_19411_Quick_Reference_PTC_DK.pdf).

> **WARNING:** To control the PTC software, additional tooling, called Radio Control Console (RCC), on an external PC is
> needed. This RCC tooling is only supported for Windows machines. So make sure if you want to start using PTC software,
> a Windows machine is at hand.

## More information

Please visit www.qorvo.com for more information on our product line and support options.










