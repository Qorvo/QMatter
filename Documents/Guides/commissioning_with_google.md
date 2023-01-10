# Commissioning a Qorvo Matter&trade; end device in the Google ecosystem

In this guide, step by step instructions are given to commission a Matter device onto the Matter network and control it making use of the Google Nest Hub 2nd generation.

Features demonstrated in this guide are:
1. [Validating hardware and software dependencies](#step-1-validating-hardware-and-software-dependencies)
2. [Registering the Matter device as test device in the used Google account](#step-2-registering-the-matter-device-as-test-device-in-the-used-google-account)
3. [Commissioning a Qorvo Matter light app, using QR code](#step-3-commissioning-a-qorvo-matter-light-app-using-qr-code)
4. [Controlling the Qorvo Matter light device](#step-4-controlling-the-qorvo-matter-light-device)

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

In below instructions, we will add the Qorvo Matter Device as a test device in the Google Home Developer Console.

### Preparing a test device in the Google ecosystem
1. Configuring the device in development mode:
- Set the [Product ID](https://developers.home.google.com/matter/get-started#product_id) to a [supported product by google](https://developers.home.google.com/matter/supported-devices) in the factory  block of the light application, more details on how to do this can be found in the [Factory block guide](../../Tools/FactoryData/README.md#usage).
- Set the [Vendor ID](https://developers.home.google.com/matter/get-started#vendor_id) to a test vendor id in the factory  block of the light application. By default, you can use one of the VIDs allocated by the CSA for testing purposes. Select 0xFFF1 as one of the list of possible test VID's [_0xFFF1, 0xFFF2, 0xFFF3, 0xFFF4_].
2. Create a project in the Google Home Developer Console and link all devices to this account.
- Create a new Google account and make sure your tablet and Google Nest Hub 2nd generation are using this account.
  - Tablet: account switching can be done in settings > accounts and backup > Manage accounts.
  - Google Nest Hub: [Factory reset](https://support.google.com/googlenest/answer/7073477?hl=en#zippy=%2Cgoogle-nest-hub-nd-gen) and set it up from scratch once the account switching is done on the tablet. 
- Go to the [Google Home Developer Console](https://console.home.google.com/projects) and login with the created Google account.
- Create a project “New Project” and give it a name. For example, “qorvo-matter-test”
- On the Home page you should be able to click “+ Add Matter Integration”
- click Next: Develop, which displays the Matter checklist page. Click Next: Matter setup.
-	On the Matter setup page, enter your Product name. For example: “Qorvo Matter Light”
-	Click Select device type and select the device type from the dropdown menu. “Light”
-	VendorID: Test VID and use _0xFFF1_
-	ProductID: _0x8005_ for light
-	Click Save & Continue.
-	Configure setup and branding: Leave the defaults and click “Save”.
-	Click Next:Test.
-	Now you get a summary of your integrations and status should be “Ready”.


## Step 3: Commissioning a Qorvo Matter light app, using QR code
1. Initiate the Bluetooth Low Energy (BLE) advertisements by factory resetting the light.
2. Open the QR code related to the commissionee, for the light application in debug mode the link is printed over uart at boot of the application.
3. Open the Google Home app on the Android tablet and click “+” in the upper left corner, “Set up device” > “New device” > “Select the correct Home”.
4. It will start looking for devices. Proceed with commissioning Matter-enabled device and scan the QR-code of the light device.
5. Now commissioning will start (takes 2 minutes or so).
6. After a while you need to add the Location of the device and Give the device a name.
7. Now it is part of the Google ecosystem, and you should be able to control it with Google (click on the tile).

## Step 4: Controlling the Qorvo Matter light device
To control the devices there are 3 interfaces made available by Google:
- Voice assistant
- Google Home App
- Google Smart Displays

Details on these can be found via Googles' [Matter control guide](https://developers.home.google.com/matter/integration/control) 

## Additional options
### Onboarding to the Google Preview Program

To verify the latest features, bugfixes and stack integrations you must register to the Google preview program for both the Google Home App as well as your specific devices from google.

> **WARNING:** The preview program is no longer a requirement for Matter compatibility [since 16 dec. 2022](https://blog.google/products/google-nest/matter-general-availability/) where it became generaly available through their updates on all Matter supported devices.

> **WARNING:** This update can take a few days before it is deployed to your devices after enrolling.

If you are still considering enrolling into their preview program, the links below are describing how to participate:

- [How to join Public Preview for Google Home app](https://support.google.com/googlenest/answer/12494697 ).

- [How to join Preview Program for a specific device](https://support.google.com/googlenest/answer/6343937?hl=en#zippy=%2Chow-do-i-join-the-preview-program)


### Troubleshooting Google Matter devices

Google provides a tool called _Android Debug Bridge_(adb) in their [platform-tools](https://developer.android.com/studio/releases/platform-tools).

This tool can used to access both your Android phone/tablet and the Nest Hub (2nd gen).

for example to obtain your Thread credentials from your Google Nest Hub (2nd gen), you may run:
```
$ adb connect border_router_ip_address
$ adb -e shell ot-ctl dataset active -x
$ adb disconnect
```
For more information visit Google's [Troubleshooting guide](https://developers.home.google.com/matter/build/troubleshooting).

## Future reads: 
* [Get started with Matter in Google Home Ecosystem](https://developers.home.google.com/matter/get-started)
