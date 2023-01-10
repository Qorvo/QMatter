# Commissioning a Qorvo Matter&trade; end device in the Apple ecosystem

In this guide, step by step instructions are given to commission a Matter device onto the Matter network and control it making use of the Apple HomePod mini.

Features demonstrated in this guide are:
1. [Validating hardware and software dependencies](#step-1-validating-hardware-and-software-dependencies)
2. [Commissioning a Qorvo Matter light app, using QR code](#step-2-commissioning-a-qorvo-matter-light-app-using-qr-code)
3. [Controlling the Qorvo Matter light device](#step-3-controlling-the-qorvo-matter-light-device)

## Step 1: Validating hardware and software dependencies
This section elaborates on hardware and software that can be used to validate comissioning and controlling of the Qorvo Matter devices using the Apple ecosystem.
- Apple HomePod mini:
  - This device functions as a Thread Border Router for the Apple ecosystem. It allows the matter network to communicate to devices over Thread.
  - Required software version : Apple HomePod OS 16.2 or higher

  - The Apple HomePod mini is available on the [Apple store](https://www.apple.com/shop/buy-homepod/homepod-mini).

- Apple IPad or iPhone:
  - Apple iPad OS / iOS 16.2 (or higher).
  - Latest version of the [Apple Home app](https://apps.apple.com/us/app/home/id1110145103)

## Step 2: Registering the Matter device as test device in the used Apple Ecosystem
To comply to the security aspect of matter, a commissionee can only get commissioned by a commissioner if it is registered as a thrusthworthy consumer device in a [Distributed Compliance Ledger (DCL)](https://csa-iot.org/certification/distributed-compliance-ledger/) or under certain constraints as a test device for development purposes. 

In below instructions, we will add the Qorvo Matter Device as a test device in Apple Home:
- Set the Product ID to a [supported accessory by Apple Home](https://www.apple.com/home-app/accessories/) in the factory block of the light application, more details on how to do this can be found in the [Factory block guide](../../Tools/FactoryData/README.md#usage).
- Set the Vendor ID to a test vendor id in the factory block of the light application. By default, you can use one of the VIDs allocated by the CSA for testing purposes. Select 0xFFF1 as one of the list of possible test VID's [_0xFFF1, 0xFFF2, 0xFFF3, 0xFFF4_].

## Step 3: Commissioning a Qorvo Matter light app, using QR code
1. Initiate the Bluetooth Low Energy (BLE) advertisements by factory resetting the light.
2. Open the QR code related to the commissionee, for the light application in debug mode the link is printed over uart at boot of the application.
3. Open the MyHome app and click “+” on the top right corner, next “Add Accessory”
4. Scan the QR code mentioned above to commissioning the Qorvo Matter Light device.
6. Now commissioning will start (takes 2 minutes or so).
7. When prompted that you are commissioning an uncertified device, press “Add anyway”.
8. Fill in the Location and a device name when asked.
7. After a while the bulb should be in the Apple ecosystem and you should be able to control it.

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
