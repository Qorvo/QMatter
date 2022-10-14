# Commissioning Qorvo Matter device with Android chip-tool

In this guide, step-by-step instructions are given to commission a Matter device onto the Matter network and control
it using the Android&trade; chip-tool. It can be used to test Matter applications using an Android&trade; 8+ Smartphone.

Features of the Android&trade; chip-tool are:
1. Scan a Matter QR code and display payload information to the user.
2. Commission a Matter device.
3. Send echo requests to the Matter echo server.
4. Send on/off cluster, level control cluster requests to a Matter device.
5. Sensor cluster read outs (Temperature, Pressure and Humidity).
6. Multi admin cluster functionality (basic commissioning and enhanced commissioning)
7. Reading out the supported/commissioned fabric count.
8. Basic cluster information read outs.

> **WARNING:** the CHIPTool Android application is a development tool developed and maintained within the Matter
> community, its features are limited and do not represent a final product. It is still in a development phase so
> some instability while using the application might be seen.

> **WARNING:** the CHIPTool Android application can only commission Matter devices with default certificates with vendor
> identifier 0xFFF1. As the Matter base reference application uses Qorvo vendor identifier and Qorvo certificates, it
> can't be commissioned with this tool. Please use the CLI `chiptool` for testing this application
[here](commissioning_posix_cli_chiptool.md).

The setup to be achieved will look like the picture below:

<div align="center">
  <img src="Images/android_chiptool_setup.png" alt="Android chip-tool setup" width=500>
</div>

To commission and control the Matter device in the network following actions will be done:
1) The smartphone scans the Matter device's QR code.
2) A secure Bluetooth LE connection is setup using the QR code information.
3) Thread network credentials are passed to the Matter device.
4) The Matter device joins the Thread network.
5) The smartphone can now communicate via IP over the QPG7015M based WiFi/Ethernet-Thread router and control the Matter
device.

Required Hardware:
- Qorvo's QPG7015M Gateway development kit.
- Qorvo's QPG6105 Matter development kit.
- Smartphone with Android 8 or later.
- Access point/Router to connect your Smartphone and Gateway development kit to the same network. Note that a WiFi Access
point can be enabled on the Raspberry Pi that runs the OpenThread Border Router. See [Enable a Wifi access point on RPi4](setup_qpg7015m_ot_borderrouter.md#enable-a-wifi-access-point-on-rpi4) how this can be achieved.

## Step 1: Installing Android chip-tool.

The Android chip-tool can be downloaded from [here](../../Tools/MatterControllers/Android). Transfer this apk to your smartphone and
install it. You may need to grant permission to install unknown applications.

Also make sure your smartphone is connected through WiFi in the same network as the OpenThread Border Router will
operate on.

Alternatively, you can build the Android chip-tool from source. Instructions how to do this can be found
[here](https://github.com/Qorvo/connectedhomeip/blob/v1.0.0.0-qorvo/docs/guides/android_building.md).

## Step 2: OpenThread Border router is running and a Thread network is formed
See the guide [How to setup the OpenThread Border Router](setup_qpg7015m_ot_borderrouter.md).

Now a Thread network is created. Proceed to step 3 to prepare the Matter device for joining the network.

## Step 3: Preparing Matter device for commissioning.

Make sure the Matter software is flashed on the development kit and the serial console application is running. See
[Building and flashing the example applications](../../README.md#flashing) and
[Enable serial logging](../../README.md#enable-serial-logging) for instructions.

If you already commissioned the device before, perform a factory reset first. Performing factory reset is dependent on
the Matter applications you are using:
- [Factory reset of the Matter Light](../../Applications/Matter/light/README.md#factory-reset)
- [Factory reset of the Matter Lock](../../Applications/Matter/lock/README.md#factory-reset)

After reset of the Matter light application, the device will start Bluetooth LE advertising
automatically and is ready for commissioning in the Matter network.

The Matter lock application does not start advertising automatically because of security reasons for a lock device.
Therefore, a manual trigger is needed to initiate advertising. Shortly press `SW5` on the development board to trigger the
advertising. Now the Matter lock device is ready for commissioning in the Matter network.

## Step 4: Commissioning the Matter device in the Matter network.

Open Android chip-tool on the smartphone and click `PROVISION CHIP DEVICE WITH THREAD`:

<div align="center">
  <img src="Images/android_chiptool_provision.png" alt="Android provision with thread">
</div>

This will open the camera to scan the QR code. To retrieve the QR code you need to check the serial output of the
Matter device. This will print a URL at start-up where you can find the QR code. Output should be something similar as:

```
[P][SVR] https://dhrishi.github.io/connectedhomeip/qrcode.html?data=MT%3AY3.13AUB00KA0648G00
```

To display the QR code, open a web browser and navigate to the URL. This website generates the required QR code to
commission the device with.

Now, make sure your Matter device is advertising (for the Matter light, it starts advertising automatically after reset,
for the Matter lock, you need to press `SW5` short to trigger the advertising). Next scan the QR code (action 1 in the
figure on top) and the chip-tool will ask to enter the credentials of the Thread network. If you have used
the defaults during the formation of the Thread network on the Border Router, you can use the defaults here as well.
Otherwise make sure to update them accordingly.

<div align="center">
  <img src="Images/android_chiptool_threadnetwork.png" alt="Android provision with thread, enter thread network">
</div>

Next, the chip-tool will start Bluetooth LE scanning to find the Matter device. If it finds the device, a
Bluetooth LE connection will be triggered (action 2 in the figure on top), and commissioning will start automatically.
During the process, the Matter device will receive the Thread network credentials via Bluetooth LE (action 3 in the
figure on top). Finally, the Matter device will join the Thread network and the message `commissioning completed`
appears shortly at the bottom of the screen.

Now the Matter device is fully commissioned in the Matter network and you can start controlling the Matter device using
the Android chip-tool. This will be explained in the next step.

## Step 5: Sending commands to control the Matter device in the network.

To start controlling the Matter device, navigate to `LIGHT ON/OFF & LEVEL CLUSTER`.

<div align="center">
  <img src="Images/android_chiptool_onoff.png" alt="Android controlling">
</div>

A slider, `Toggle` and `ON`/`OFF` controls are shown:

<div align="center">
  <img src="Images/android_chiptool_onoffcontrol.png" alt="Android controlling onoff">
</div>

You can now operate your Matter device using the controls:
- For Matter Light, it controls the RGB led on the development board (Dimming using the slider, `Toggle` and `On`/`Off`
commands can be used).

You can also read the status of the Matter light. Therefore you can use the `Read` button. The `Subscribe` button
can be used to poll the status of the Matter light at a defined time interval.

> Note: For the Matter lock example application no native support for the door lock cluster is added in this Matter controller.
## Troubleshooting

- if the Android chip-tool crashes during commissioning, disable Bluetooth on the smartphone, then re-enable it again
before retrying. You can also try killing all applications running in the background using Android task manager.
- After a reset of the Matter device, trigger 'Update address' on the control screen. This will update the address again
in the Thread network.
- After a reset of the OpenThread border router, it is advised to reset the SRP server by executing following commands
in RPi's terminal:
```
docker exec -it otbr_eth sudo ot-ctl srp server disable
docker exec -it otbr_eth sudo ot-ctl srp server disable
```
Also trigger 'Update Address' on the control screen. This will update the address again in the Thread network.
- After a force kill of the application, trigger 'Update Address' on the light control screen. This will update the
address again in the Thread network.
