# Commissioning a Qorvo Matter&trade; end device in the Apple ecosystem

In this guide, step by step instructions are given to commission a Matter device onto the Matter network and control it making use of the Apple HomePod mini.

Features demonstrated in this guide are:
- [Commissioning a Qorvo Matter™ end device in the Apple ecosystem](#commissioning-a-qorvo-matter-end-device-in-the-apple-ecosystem)
  - [Step 1: Validating hardware and software dependencies](#step-1-validating-hardware-and-software-dependencies)
  - [Step 2: Registering the Matter device as test device in the used Apple Ecosystem](#step-2-registering-the-matter-device-as-test-device-in-the-used-apple-ecosystem)
  - [Step 3: Commissioning a Qorvo Matter light app, using QR code](#step-3-commissioning-a-qorvo-matter-light-app-using-qr-code)
  - [Step 4: Controlling the Qorvo Matter light device](#step-4-controlling-the-qorvo-matter-light-device)
  - [Additional options](#additional-options)
    - [Onboarding to the Apple Beta Program](#onboarding-to-the-apple-beta-program)
    - [Troubleshooting Apple Matter devices](#troubleshooting-apple-matter-devices)
  - [Future reads:](#future-reads)

## Step 1: Validating hardware and software dependencies
This section elaborates on hardware and software that can be used to validate commissioning and controlling of the Qorvo Matter devices using the Apple ecosystem.
- A thread enabled Apple Home Hub such as the Apple HomePod mini:
  - This device functions as a Thread Border Router for the Apple ecosystem. It allows the matter network to communicate to devices over Thread.
  - Required software version : Apple HomePod OS 16.4 or higher

  - The Apple HomePod mini is available on the [Apple store](https://www.apple.com/shop/buy-homepod/homepod-mini).
  - For more details or other border router options follow the [Setup your Apple device as home hub guide](https://support.apple.com/HT207057).

- Apple IPad or iPhone:
  - Apple iPad OS / iOS 16.4 (or higher).
  - Latest version of the [Apple Home app](https://apps.apple.com/us/app/home/id1110145103)

## Step 2: Registering the Matter device as test device in the used Apple Ecosystem
To comply to the security aspect of matter, a commissionee can only get commissioned by a commissioner if it is registered as a thrusthworthy consumer device in a [Distributed Compliance Ledger (DCL)](https://csa-iot.org/certification/distributed-compliance-ledger/) or under certain constraints as a test device for development purposes.

> **WARNING:** Because the Apple Home ecosystem does not support the Matter Binding cluster, end users cannot configure an On/Off Light Switch to control other Matter devices using the Apple Home app. The switch application in this repository is supporting the Matter Binding cluster and as a consequence will not be able to control any other device in the Apple ecosystem.
In below instructions, we will add a Qorvo Matter Device as a test device in Apple Home:

In this release by default all example applications will have test vendor ID set to _0xFFF1_ which is mandatory for testing. Also following product identifiers are used for the different types of applications:
>
> * Matter light: 0x8005
> * Matter door lock: 0x8006
> * Matter light switch: 0x8004
> * Matter thermostatic radiator valve: 0x8003

## Step 3: Commissioning a Qorvo Matter light app, using QR code
1. Power on Qorvo IoT Development Kit for QPG6105 by plugging in the USB-cable to your PC.
2. Initiate the Bluetooth Low Energy (BLE) advertisements by [factory resetting](../../Applications/Matter/light/README.md#factory-reset) the Qorvo Matter light device.
3. Open the QR code related to the commissionee, for the light application in debug mode the link is printed over uart at boot of the application (See [Enable serial logging](../../README.md#enable-serial-logging) to get the serial output). You can also use below QR code if you are commissioning the preprogrammed Matter Light application that comes with Qorvo IoT Development Kit for QPG6105:
   
<div align="center">
  <img src="Images/qrcode.png" alt="QR code" width=200>
</div>

4. Open the MyHome app and click “+” on the top right corner, next “Add Accessory”.
5. Scan the QR code mentioned above to commission the Qorvo Matter light device.
6. Now commissioning will start (takes 2 minutes or so).
7. When prompted that you are commissioning an uncertified device, press “Add anyway”.
8. Fill in the Location and a device name when asked.
9. After a while the corresponding icon of the Application, in this example a light bulb, should appear in the Apple ecosystem and you should be able to control it.

## Step 4: Controlling the Qorvo Matter light device
Once added, Apple Home transparently supports Matter accessories in the Home app, Siri, Control Center, and in third-party HomeKit apps.

More details can be found on [Apple's introduction to Home](https://support.apple.com/guide/iphone/intro-to-home-iph22d98bbca/ios).

## Additional options
### Onboarding to the Apple Beta Program

To verify the latest features, bugfixes and stack integrations you must register to the Apple Beta program.

> **WARNING:** The Apple Beta program is no longer a requirement for Matter compatibility [since Oct 24, 2022](https://developer.apple.com/apple-home/matter/) where it became generaly available through their iOS 16.1 updates on all Matter supported devices.

> **WARNING:** This update can be started immediately. Rollback to a previous version is not possible for apple devices.

If you are still considering enrolling into their preview program, the links below are describing how to participate:
- [How to join Beta program](https://beta.apple.com/sp/betaprogram).

### Troubleshooting Apple Matter devices

Apple provides a framework called [MatterSupport framework](https://developer.apple.com/documentation/mattersupport) to allow users to investigate issues.

## Future reads:
* [Get started with Matter in Apple Home Ecosystem](https://developer.apple.com/apple-home/matter/)
