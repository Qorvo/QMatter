# Commissioning a Qorvo Matter&trade; end device in the Google ecosystem

In this guide, step by step instructions are given to commission a Matter device onto the Matter network and control it making use of the Google Nest Hub 2nd generation.

Features demonstrated in this guide are:
- [Commissioning a Qorvo Matter™ end device in the Google ecosystem](#commissioning-a-qorvo-matter-end-device-in-the-google-ecosystem)
  - [Step 1: Validating hardware and software dependencies](#step-1-validating-hardware-and-software-dependencies)
  - [Step 2: Registering the Matter device as test device in the used Google account](#step-2-registering-the-matter-device-as-test-device-in-the-used-google-account)
    - [Preparing a test device in the Google ecosystem](#preparing-a-test-device-in-the-google-ecosystem)
  - [Step 3: Commissioning a Qorvo Matter light app, using QR code](#step-3-commissioning-a-qorvo-matter-light-app-using-qr-code)
  - [Step 4: Controlling the Qorvo Matter light device](#step-4-controlling-the-qorvo-matter-light-device)
  - [Additional options](#additional-options)
    - [Onboarding to the Google Preview Program](#onboarding-to-the-google-preview-program)
  - [Future reads:](#future-reads)

## Step 1: Validating hardware and software dependencies
This section elaborates on hardware and software that can be used to validate comissioning and controlling of the Qorvo Matter devices.
For more details visit [the what you need section of Google](https://support.google.com/googlenest/answer/13127223?hl=en).
- Google Nest Hub 2nd generation:
  - This device functions as a Thread Border Router for the google ecosystem. It allows the matter network to communicate to devices over Thread.
  - Required software version : 48.9.447810048 (or higher)
  - The Google Nest Hub 2nd generation is available on the [Google store](https://store.google.com/be/product/nest_hub_2nd_gen).

- Android Tablet or Smartphone:
  - Android O (8.1, API level 27) or newer.
  - Google Play services version 22.48.14 or later
  - Bluetooth Low Energy (BLE) 4.2 or higher
  - Latest version of the [Google Home app](https://play.google.com/store/apps/details?id=com.google.android.apps.chromecast.app&hl=en_US)

  - To verify your mobile device has all the required Matter modules downloaded, you can follow [this](https://developers.home.google.com/matter/verify-services) guide.

## Step 2: Registering the Matter device as test device in the used Google account
To comply to the security aspect of matter, a commissionee can only get commissioned by a commissioner if it is registered as a thrusthworthy consumer device in a [Distributed Compliance Ledger (DCL)](https://csa-iot.org/certification/distributed-compliance-ledger/) or under certain constraints as a test device for development purposes.

In below instructions, we will add the Qorvo Matter Device as a test device in the [Google Home Developer Console](https://console.home.google.com/projects).

> **WARNING:** Google [does not support the onoff Light switch device type](https://developers.home.google.com/matter/supported-devices#onoff_light_switches). Because the Google Home ecosystem does not expose the Matter Binding cluster to its controllers, end users cannot configure an On/Off Light Switch to control other Matter devices using the GHA. The switch application in this repository is supporting the Matter Binding cluster and as a consequence will not be able to control any light device in the google ecosystem.

### Preparing a test device in the Google ecosystem

> In this release by default all example applications will have test vendor ID set to _0xFFF1_ which is mandatory for testing with Google. Also following product identifiers are used for the different types of applications:
>
> * Matter light: 0x8005
> * Matter door lock: 0x8006
> * Matter light switch: 0x8004
> * Matter thermostatic radiator valve: 0x8003

Create a project in the [Google Home Developer Console](https://console.home.google.com/projects) and link all devices to this account.
- Create a new Google account and make sure your tablet and Google Nest Hub 2nd generation are using this account.
  - Tablet: account switching can be done in settings > accounts and backup > Manage accounts.
  - Google Nest Hub: [Factory reset](https://support.google.com/googlenest/answer/7073477?hl=en#zippy=%2Cgoogle-nest-hub-nd-gen) and set it up from scratch once the account switching is done on the tablet.
- Go to the [Google Home Developer Console](https://console.home.google.com/projects) and login with the created Google account.
- Create a project “New Project” and give it a name. For example, “qorvo-matter-test”
- On the Home page you should be able to click “+ Add Matter Integration”
- click _Next: Develop_, which displays the Matter checklist page.
- click _Next: Setup_, which displays the Matter _Setup_ page:
  
  Enter your Product name. For example: “Qorvo Matter Light”
-	Click _Select device type*_ and select the device type from the dropdown menu. “Light”
-	VendorID: Test VID and use _0xFFF1_
-	ProductID: _0x8005_ for light
- click _Save & continue_, which displays the Matter branding page.
-	Leave the defaults
- click and click _Save_, which displays the Matter Develop page.
- click _Next:Test_, this accepts your integration and displays the Matter/Test page.
-	Now you get a summary of your integrations and status should be “Ready”.
- Your matter device is now _ready_ to be tested.

## Step 3: Commissioning a Qorvo Matter light app, using QR code
1. Power on Qorvo IoT Development Kit for QPG6105 by plugging in the USB-cable to your PC.
2. Initiate the Bluetooth Low Energy (BLE) advertisements by [factory resetting](../../Applications/Matter/light/README.md#factory-reset) the Qorvo Matter light device.
3. Open the QR code related to the commissionee, for the light application in debug mode the link is printed over uart at boot of the application (See [Enable serial logging](../../README.md#enable-serial-logging) to get the serial output). 
You can also use below QR code if you are commissioning the preprogrammed Matter Light application that comes with Qorvo IoT Development Kit for QPG6105:
   
<div align="center">
  <img src="Images/qrcode.png" alt="QR code" width=200>
</div>

4. Open the Google Home app on the Android tablet and click “+” in the upper left corner, “Set up device” > “New device” > “Select the correct Home”.
5. It will start looking for devices. Proceed with commissioning Matter-enabled device and scan the QR-code of the light device.
6. Now commissioning will start (takes 2 minutes or so).
7. After a while you need to add the Location of the device and Give the device a name.
8. Now it is part of the Google ecosystem, and you should be able to control it with Google (click on the tile).

## Step 4: Controlling the Qorvo Matter light device
To control the devices there are 3 interfaces made available by Google:
- Google assistant
- Google Home App
- Smart Displays

Details on these can be found via Googles' [Matter control guide](https://developers.home.google.com/matter/integration/control)

## Additional options
### Onboarding to the Google Preview Program

To verify the latest features, bugfixes and stack integrations you must register to the Google preview program for both the Google Home App as well as your specific devices from google.

> **WARNING:** The preview program is no longer a requirement for Matter compatibility [since 16 dec. 2022](https://blog.google/products/google-nest/matter-general-availability/) where it became generaly available through their updates on all Matter supported devices.

> **WARNING:** This update can take a few days before it is deployed to your devices after enrolling.

If you are still considering enrolling into their preview program, the links below are describing how to participate:

- [How to join Public Preview for Google Home app](https://support.google.com/googlenest/answer/12494697 ).

- [How to join Preview Program for a specific device](https://support.google.com/googlenest/answer/6343937?hl=en#zippy=%2Chow-do-i-join-the-preview-program)


## Future reads:
* [Get started with Matter in Google Home Ecosystem](https://developers.home.google.com/matter/get-started)
