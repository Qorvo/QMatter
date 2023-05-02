# QPG6105 Bluetooth&reg; LE example application

Qorvo&reg; QPG6105 Bluetooth LE example shows how to create a sleepy Blutooth LE peripheral application.
This example can be used as reference for creating your own Bluetooth LE device by adding services on top.

Buttons are used as input to trigger device state changes, LEDs are used as output to visualize the device states.

Features of this application are:
1. Scannable and Connectable Bluetooth LE device
2. Bondable Bluetooth LE device
3. Perform OTA upgrade with our Qorvo Connect app


---

- [QPG6105 Bluetooth® LE example application](#qpg6105-bluetooth-le-example-application)
  - [Button control](#button-control)
  - [LED output](#led-output)
  - [Logging output](#logging-output)
  - [Building and flashing](#building-and-flashing)
  - [Generating OTA file](#generating-ota-file)
    - [OTA Image file creation process](#ota-image-file-creation-process)
    - [Command line use](#command-line-use)
    - [Integration in build flow](#integration-in-build-flow)
  - [Using Qorvo Connect app](#using-qorvo-connect-app)

---

## Button control

This application uses following buttons of the Qorvo IoT Dev Kit for QPG6105:

- `SW6 (RADIO RESET)`: Used to perform a HW reset for the full board.
- `SW5 (PB4)`: Used to start BLE advertisement for incoming connection
- `SW3 (PB3)`: Used to close and unbind connection

The buttons `SW1`, `SW2`, `SW4` and the slider switch `SW7` are unused.

## LED output

The following LEDs are used during the application:

- `LD4` - RED led - shows the device state and its connectivity. Following states are defined:
  - Fast blinks 3 times: Bluetooth LE is advertising
  - Slow blinks 3 times: Bluetooth LE connection established

## Logging output

For instructions to view the serial logging, refer to [Enable serial logging](../../../README.md#enable-serial-logging)

At startup you will see:

```
BleModule ready after reset
NRT ROM v1
============================
Qorvo BlePeripheral-app Launching
============================
BLE MAC address: 00:00:00:00:00:00
INFO: BleModule_LocalIrk restored value:
BleModule ready after reset

```

## Building and flashing

See [Building and flashing the example applications](../../../README.md#building-and-flashing-the-example-applications) section to get instructions how to build and program example application.


## Generating OTA file

### OTA Image file creation process

When calling `generate_ble_ota_img.py`, you need to specify a .hex file produced from the firmware .elf file.
First, it will run `signFirmware.py`, which will modify the hex file in-place.
This will modify the metadata in the userlicense section, allowing the hex file to be used as secure image, originating from the factory:

- The image-loaded-by-bootloader flag will be set.
- A cryptographic signature will be added for authentication by the bootloader
- The public key of the `.pem` file will be written to the native userlicense for use by the bootloader.

After this, `generate_ble_ota_img.py` will call `compressFirmware.py`:
- It will compress the firmware sections that contain upgradable data (code, data) using the Lempel–Ziv–Markov chain(LZMA) compression algorithm, after applying padding up to a page size multiple.
- A signature will be created and inserted into the compressed User License, this will enable the bootloader to perform an integrity check before inflating the payload.

As a last step, [BLE's ota\_image\_tool.py](../../../Tools/Ota/ota_gen.py) will be called to wrap the OTA payload with the Qorvo-specific OTA headers.

### Command line use

```
usage: generate_ble_ota_img.py [-h] [--in_file IN_FILE] [--out_file OUT_FILE] [--sw_ver SW_VER] [--sign] [--pem_file_path PEM_FILE_PATH] [--pem_password PEM_PASSWORD]
                               [-p FLASH_APP_START_OFFSET] [--compression {none,lzma}] [--prune_only]

Turn the application build hex-file into a bootable image and generate an ota image

optional arguments:
  -h, --help            show this help message and exit
  --in_file IN_FILE     Path to input file
  --out_file OUT_FILE   Path to output
  --sw_ver SW_VER       Software version
  --sign                sign firmware
  --pem_file_path PEM_FILE_PATH
                        PEM file path (string)
  --pem_password PEM_PASSWORD
                        PEM file password (string)
  -p FLASH_APP_START_OFFSET, --flash_app_start_offset FLASH_APP_START_OFFSET
                        Offset of the application in program flash
  --compression {none,lzma}
                        compression type (default to lzma)
  --prune_only          prune unneeded sections; don't add an upgrade user license (external storage scenario)
```

### Integration in build flow

When using QMatter to build, the `generate_ble_ota_img.py` script is called when the applications Makefile invokes its postbuild script (eg [ble_peripheral_qpg6105_postbuild.sh](ble_peripheral_qpg6105_postbuild.sh))


## Using Qorvo Connect app

The Qorvo Connect app provided by Qorvo is available on Apple Apps Store and Google Play Store.
 - For iPhone users: https://apps.apple.com/tr/app/qorvo-connect/id1588263604
 - For Android users: https://play.google.com/store/apps/details?id=com.qorvo.ble


 1. Start up the Qorvo Connect app then pull down to refresh the list of available device
<div align="center">
  <img src="Images/QConnect-list.png" alt="Qorvo Connect">
</div>

 2. Tap on CONNECT on the right side of the device listed to be connected.
 3. Tap on OPEN once the device moved up to the list of Connected devices
<div align="center">
  <img src="Images/QConnect-device.png" alt="Qorvo Connect">
</div>

 4. Tap on the LINK icon in the middle of the top list of icons to bond
 5. Tap on the N icon to enable notifications for all
 6. Tap on OTA to select the OTA image to be uploaded to the device
 7. Double confirm on the detail of the OTA image
<div align="center">
  <img src="Images/QConnect-confirm.png" alt="Qorvo Connect">
</div>

 8. Tap on EXECUTE to begin the OTA process
<div align="center">
  <img src="Images/QConnect-ota.png" alt="Qorvo Connect">
</div>

 9. The device will restart on its own once the OTA process has completed
