# Commissioning Qorvo Matter device with POSIX CLI chip-tool

In this guide, step by step instructions are given to commission a Matter device onto the Matter network and control
it making use of the POSIX CLI chip-tool.

The setup to be achieved will look like the picture below:

<div align="center">
  <img src="../Images/cli_chiptool_setup.png" alt="CLI chip-tool setup">
</div>

To commission and control the Matter device in the network following actions will be done:
1) Based on the discriminator (12-bit value to find specific device between multiple commissionable device
advertisements), the chip-tool will perform a Bluetooth LE scan to find the Matter device.
2) A secure Bluetooth LE connection is setup and the setup pin code (27-bit PIN code) will be used for setting up the
Password-Authenticated Session Establishment (PASE).
3) Thread network credentials are passed to the Matter device.
4) The Matter device joins the Thread network.
5) The chip-tool can now communicate via IP over the QPG7015M based WiFi/Ethernet-Thread router and control the Matter
device.

Required Hardware:
- Qorvo's QPG7015M Gateway development kit.
- Qorvo's QPG6105 Matter development kit.
- PC or Raspberry Pi 4+ with Ubuntu 20.04 or newer
- Access point/Router to connect your PC/RPi and Gateway development kit to the same network. Note that a WiFi Access
point can be enabled on the Raspberry Pi that runs the OpenThread Border Router. See [How to setup the OpenThread Border Router](setup_ot_borderrouter.md) how this can be achieved.

## Step 1: Installing POSIX CLI chip-tool on RPi4 or PC.

The POSIX CLI chip-tool can be downloaded for [PC](Tools/MatterControllers/PC) or [Raspberry Pi](Tools/MatterControllers/RPi).
Transfer this binary to your PC or RPi and make sure Bluetooth LE is enabled on the device (BlueZ).

Using the chip-tool is always done in this way:
```
sudo ./chip-tool <cmd> <args>
```

Also make sure your PC/RPi is connected through WiFi or Ethernet in the same network as the OpenThread Border Router will
operate on.

## Step 2: Preparing OpenThread Border Router.

First make sure the OpenThread Border Router is set up as descibed in [How to setup the OpenThread Border Router](setup_ot_borderrouter.md). Also make sure it is connected through Ethernet or WiFi in the same network
as the chip-tool device. Once the OpenThread Border Router is up and running we can form a Thread network.
This can be achieved by browsing via the webbrowser to the ip-address of the Raspberry Pi on which the OpenThread
Border Router is running.

Navigate to 'Form' via the menu on the left. If the menu is not displayed click 'Home' in the top left first.

<div align="center">
  <img src="../Images/otbr_landing_page_form.png" alt="OpenThread Border Router Landing Page, Form">
</div>

This will bring you to a new page where you can do some configuration of the OpenThread network. Leave the defaults and
click 'FORM':

<div align="center">
  <img src="../Images/otbr_form_page.png" alt="OpenThread Border Router, forming network">
</div>

When prompted 'Are you sure you want to Form the Thread Network'. Click 'OKAY':

<div align="center">
  <img src="../Images/otbr_prompt.png" alt="OpenThread Border Router, forming network prompt">
</div>

After a few seconds a message should appear that the Thread network is formed correctly:

<div align="center">
  <img src="../Images/otbr_success.png" alt="OpenThread Border Router, forming network success">
</div>

Now a Thread network is created. Proceed to step 3 to prepare the Matter device for joining the network.

## Step 3: Preparing Matter device for commissioning.

Make sure the Matter software is flashed on the development kit and the serial console application is running. See
[Matter examples](../../README.md#matter-examples) if this is not done yet.

If you already commissioned the device before, perform a factory reset first. Performing factory reset is dependent on
the Matter device you are using:
- [Factory reset of the Matter Light](../../Applications/Matter/light/README.md#factory-reset)
- [Factory reset of the Matter Lock](../../Applications/Matter/lock/README.md#factory-reset)

After reset of the Matter light application, the device will start Bluetooth LE advertising automatically and is ready
for commissioning in the Matter network.

The Matter lock application does not start advertising automatically because of security reasons for a lock device,
therefore a manual trigger is needed to initiate advertising. Shortly press SW5 on the development board to trigger the
advertising. Now the Matter lock device is ready for commissioning in the Matter network.

## Step 3: Commissioning the Matter device in the Matter network.

Commissioning the Matter device in the Thread network is done through Bluetooth LE. The command in chip-tool to be used
for commissioning is:

```
sudo ./chip-tool pairing ble-thread <node-id> <operationalDataset> <fabric-id> <setup-pin-code> <discriminator>
```

with:
- `<node-id>`: This is the node-id to assign to the node being commissioned. Must be a decimal number or a 0x-prefixed
    hex number.
- `<operationalDataset>`: This is the current active operational dataset of the Thread network. This information can be
    retrieved from the running OpenThread Border Router, using below command on the Raspberry Pi that runs the Border
    Router:
```
docker exec -it otbr_docker_0 sudo ot-ctl dataset active -x
0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8
Done
```
- `<fabric-id>`: This is the identifier of the fabric where the Matter device will be commissionioned. This needs to be set to 0.
- `<setup-pin-code>`: This is 27-bit PIN code that will be used for PASE. This PIN code can be retrieved from the Matter
    device. The retrieve this, you need to check the serial output of the Matter device and look for Device configuration
    that is printed at startup of the device. In this device configuration list you will find the Setup Pin Code.
```
[P][DL] Device Configuration:
[P][DL]   Serial Number: TEST_SN
[P][DL]   Vendor Id: 9050 (0x235A)
[P][DL]   Product Id: 20812 (0x514C)
[P][DL]   Product Revision: 0
[P][DL]   Setup Pin Code: 20202021
[P][DL]   Setup Discriminator: 3840 (0xF00)
[P][DL]   Manufacturing Date: (not set)
[P][DL]   Device Type: 65535 (0xFFFF)
```
- `<discriminator>`: This is a 12-bit value that is used to find the corresponding Matter device between multiple
    commissionable advertisements. This value also belongs to the Matter device and is listed in the device configuration
    as well.

Example of how the command looks like is:
```
sudo ./chip-tool pairing ble-thread 1 hex:0e080000000000010000000300000d35060004001fffe0020811111111222222220708fd7cc7fe41e171dc051000112233445566778899aabbccddeedf030e4f70656e54687265616444656d6f01021235041061e1206d2c2b46e079eb775f41fc72190c0402a0fff8 0 20202021 3840
```

If the command was successfull, following log will appear in the chip-tool:
```
[1641215294.652703][12343:12348] CHIP:TOO: Device commissioning completed with success
```

and following serial output will appear for the Matter device:
```
[P][IN] CASE Session established. Setting up the secure channel.
[P][IN] CASE secure channel is available now.
[P][IN] CASE Server enabling CASE session setups
```

Now the Matter device is fully commissioned in the Matter network and you can start controlling the Matter device using
the POSIX CLI chip-tool. This will be explained in the next step.

## Step 4: Sending commands to control the Matter device in the network.

To start controlling the Matter device, following command can be used for toggling the Matter device (Light/Lock):

```
sudo ./chip-tool onoff toggle <node-id> <endpoint-id>
```

with:
- `<node-id>`: This is the node-id that was assigned during commissioning. Must be a decimal number or a 0x-prefixed
    hex number.
- `<endpoint-id`: Only one endpoint is defined for cluster communications, this can be set to 1.

Example of how the command looks like is:

```
sudo ./chip-tool onoff toggle 1 1
```

If the command was successfull,
- for Matter Light, The RGB led on the development board will be toggled.
- for Matter Lock, the lock is emulated using the cool white led (LD1), this will be toggled.

## Addendum: Using the chip-tool to send Matter Commands

### How to get the list of supported clusters

To get the list of supported clusters, run the built executable without any
arguments.

    $ chip-tool

Example output:

```bash
Usage:
  sudo ./chip-tool cluster_name command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Clusters:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * barriercontrol                                                                    |
  | * basic                                                                             |
  | * colorcontrol                                                                      |
  | * doorlock                                                                          |
  | * groups                                                                            |
  | * iaszone                                                                           |
  | * identify                                                                          |
  | * levelcontrol                                                                      |
  | * onoff                                                                             |
  | * pairing                                                                           |
  | * payload                                                                           |
  | * scenes                                                                            |
  | * temperaturemeasurement                                                            |
  +-------------------------------------------------------------------------------------+
```

### How to get the list of supported commands for a specific cluster

To get the list of commands for a specific cluster, run the built executable
with the target cluster name.

```
sudo ./chip-tool onoff
```

### How to get the list of supported attributes for a specific cluster

To the the list of attributes for a specific cluster, run the built executable
with the target cluster name and the `read` command name.

```
chip-tool onoff read
```

### How to get the list of parameters for a command

To get the list of parameters for a specific command, run the built executable
with the target cluster name and the target command name

```
chip-tool onoff on
```

### Command Reference

#### Command List

-   [basic](#basic)
-   [colorcontrol](#colorcontrol)
-   [groups](#groups)
-   [identify](#identify)
-   [levelcontrol](#levelcontrol)
-   [onoff](#onoff)
-   [pairing](#pairing)
-   [scenes](#scenes)
-   [temperaturemeasurement](#temperaturemeasurement)

#### Command Details

##### basic

```bash
Usage:
  sudo ./chip-tool basic command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * reset-to-factory-defaults                                                         |
  | * ping                                                                              |
  | * discover                                                                          |
  | * read                                                                              |
  +-------------------------------------------------------------------------------------+
```

##### colorcontrol

```bash
Usage:
  sudo ./chip-tool colorcontrol command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * move-color                                                                        |
  | * move-color-temperature                                                            |
  | * move-hue                                                                          |
  | * move-saturation                                                                   |
  | * move-to-color                                                                     |
  | * move-to-color-temperature                                                         |
  | * move-to-hue                                                                       |
  | * move-to-hue-and-saturation                                                        |
  | * move-to-saturation                                                                |
  | * step-color                                                                        |
  | * step-color-temperature                                                            |
  | * step-hue                                                                          |
  | * step-saturation                                                                   |
  | * stop-move-step                                                                    |
  | * discover                                                                          |
  | * read                                                                              |
  | * report                                                                            |
  +-------------------------------------------------------------------------------------+
```

##### groups

```bash
Usage:
  sudo ./chip-tool groups command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * add-group                                                                         |
  | * add-group-if-identifying                                                          |
  | * get-group-membership                                                              |
  | * remove-all-groups                                                                 |
  | * remove-group                                                                      |
  | * view-group                                                                        |
  | * discover                                                                          |
  | * read                                                                              |
  +-------------------------------------------------------------------------------------+
```

##### identify

```bash
Usage:
  sudo ./chip-tool identify command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * identify                                                                          |
  | * identify-query                                                                    |
  | * discover                                                                          |
  | * read                                                                              |
  +-------------------------------------------------------------------------------------+
```

##### levelcontrol

```bash
Usage:
  sudo ./chip-tool levelcontrol command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * move                                                                              |
  | * move-to-level                                                                     |
  | * move-to-level-with-on-off                                                         |
  | * move-with-on-off                                                                  |
  | * step                                                                              |
  | * step-with-on-off                                                                  |
  | * stop                                                                              |
  | * stop-with-on-off                                                                  |
  | * discover                                                                          |
  | * read                                                                              |
  | * report                                                                            |
  +-------------------------------------------------------------------------------------+
```

##### onoff

```bash
Usage:
  sudo ./chip-tool onoff command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * off                                                                               |
  | * on                                                                                |
  | * toggle                                                                            |
  | * discover                                                                          |
  | * read                                                                              |
  | * report                                                                            |
  +-------------------------------------------------------------------------------------+
```

##### scenes

```bash
Usage:
  sudo ./chip-tool scenes command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * add-scene                                                                         |
  | * get-scene-membership                                                              |
  | * recall-scene                                                                      |
  | * remove-all-scenes                                                                 |
  | * remove-scene                                                                      |
  | * store-scene                                                                       |
  | * view-scene                                                                        |
  | * discover                                                                          |
  | * read                                                                              |
  +-------------------------------------------------------------------------------------+
```

##### temperaturemeasurement

```bash
Usage:
  sudo ./chip-tool temperaturemeasurement command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * discover                                                                          |
  | * read                                                                              |
  | * report                                                                            |
  +-------------------------------------------------------------------------------------+
```




