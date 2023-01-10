#  Use the Matter multi-admin feature with a Qorvo Matter&trade; end device with the Apple Home and Google Home ecosystems.

## What does multi-admin mean in context of Matter
The CSA created a [video](https://csa-iot.org/developer-resource/matter-multi-admin-video/) to introduce the multi-admin concept in Matter.

Multi-admin allows a user of a device to have different ecosystems control a specific device simultaneously in the matter network. Once the first ecosystem uses their commissionioner to commission a device, it has the capability to share it, or more likely reffered to as opening the commissioning window again, towards another ecosystems' commissioner. Each commissioned ecosystem will take up one of the reserved fabrics on the device. As these reservations take up resources, there will be a limit on how many fabrics can be supported for each device.

Once the commissioning window is opened on the specific the device, it can be commissioned in another ecosystem. Matter v1.0 is enforcing at least 5 fabrics to be supported on each device that gets certified for this version of the Matter specification.

In this guide, step by step instructions are given to commission a Matter device onto the Matter network using 2 different fabrics (Google and Apple) of the Qorvo Matter Light device.

Features demonstrated in this guide are:

1. [Prerequisites to get started with multi-admin](#step-1-prerequisites-to-get-started-with-multi-admin)
2. [Share a Matter device between ecosystems](#step-2-share-a-matter-device-between-ecosystems)


## Step 1: Prerequisites to get started with multi-admin
In order to start with multi-admin commissioning, the individual fabrics used in this guide should be able to commission. So first make sure you are able to commissioning single fabric with Apple and Google as done in below guides: 
- [Commissioning with Apple](commissioning_with_apple.md)
- [Commissioning with Google](commissioning_with_google.md) 

When both ecosystems are working as a single fabric, go to the next step to get started with the Matter multi-admin feature on the Qorvo Matter Light device.

## Step 2: Share a Matter device between ecosystems
Assuming the order in [step 1](#step-1-prerequisites-to-get-started-with-multifabric) was followed, Google should now be the single fabric used on the device. Follow the steps below to validate the multi-admin feature.

### Share a Matter device from the Google Home to the Apple Home ecosystem.
1. Make sure the device is commissioned in the Google ecosystem as described in [commissioning with Google](commissioning_with_google.md).
2. Make sure the Google Nest Hub 2nd generation and Android Tablet are in the same network as the Apple HomePod mini and iPad.
3. On the Android tablet open the Home App. Perform a long press on the tile to open the light controls. Next click on settings (gear symbol) > Linked Matter apps & services > Link apps & services > Share with QR code. This will display a QR code and the commissioning window is opened for another ecosystem to commission it.
4. [Use the iPad or iPhone to add an accessory](#commissioning_with_apple.md) using the QR code displayed by the previous step.
5. You should now be able to control the device with both the Apple Home and Google Home ecosystems.


### Share a Matter device from the Apple Home to the Google Home ecosystem.
> **WARNING:** This section is not working yet with the Google Ecosystem.
1. Make sure the device is commissioned in the Apple ecosystem as described in [commissioning with Apple](commissioning_with_apple.md).
2. Make sure the Google Nest Hub 2nd generation and Android Tablet are in the same network as the Apple HomePod mini and iPad.
3. On the iPad or iPhone and open the Apple Home App. Perform a long press on the tile to of the light device to open the option window. Next click on Accessory Details (gear symbol) > click the next gear symbol to get advanced settings > Scroll down and select _Turn On Pairing Mode_. This will display a setup code and the commissioning window is opened for another ecosystem to commission it.
4. [Use the iPad or iPhone to add an accessory](#commissioning_with_apple.md) using the original QR code of the device or the setup code indicated in the previous step.
5. You should now be able to control the device with both the Apple Home and Google Home ecosystems.
