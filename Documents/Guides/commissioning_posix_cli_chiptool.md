# Commissioning Qorvo Matter&trade; device with POSIX CLI chip-tool

In this guide, step by step instructions are given to commission a Matter device onto the Matter network and control it making use of the POSIX CLI chip-tool. It can be used to test Matter applications using a PC running Ubuntu 20.04+ or RPi4 running Ubuntu 20.04+.

Features of the POSIX CLI chip-tool are:
1. Onboarding using setup code (no QR code).
2. Provisioning a Matter device.
3. Sending cluster commands to a Matter device including reading/writing cluster attributes.

The setup to be achieved will look like the picture below:

<div align="center">
  <img src="Images/cli_chiptool_setup.png" alt="CLI chip-tool setup" width=500>
</div>

To commission and control the Matter device in the network following actions will be done:
1) Based on the discriminator (12-bit value to find specific device between multiple commissionable device
advertisements), the chip-tool will perform a Bluetooth&trade; LE scan to find the Matter device.
2) A secure Bluetooth LE connection is setup and the setup pin code (27-bit PIN code) will be used for setting up the
Password Authenticated Session Establishment (PASE).
3) Thread network credentials are passed to the Matter device.
4) The Matter device joins the Thread network.
5) The chip-tool can now communicate via IP over the QPG7015M based WiFi/Ethernet-Thread router and control the Matter device.


Required Hardware:
- Qorvo&reg; IoT Dev Kit for QPG7015M.
- Qorvo&reg; IoT Dev Kit for QPG6105.
- PC or Raspberry Pi 4+ with Ubuntu 20.04 or newer.
- Access point/Router to connect your PC/RPi and to the same network. Note that a WiFi Access
point can be enabled on the Raspberry Pi that runs the OpenThread Border Router. See [Enable a Wifi access point on RPi4](setup_qpg7015m_ot_borderrouter.md#enable-a-wifi-access-point-on-rpi4) how this can be achieved.

## Step 1: Installing POSIX CLI chip-tool on RPi4 or PC.

The POSIX CLI chip-tool can be downloaded for [PC](../../Tools/MatterControllers/x64_linux) or [Raspberry Pi](../../Tools/MatterControllers/RPi).
Transfer this binary to your PC or RPi and make sure Bluetooth LE is enabled on the device (BlueZ).

Using the chip-tool is always done in this way:
```
sudo ./chip-tool.elf <cmd> <args>
```

Also make sure your PC/RPi is connected through WiFi or Ethernet in the same network as the OpenThread Border Router will
operate on.

Alternatively, you can build the POSIX CLI chip-tool from source. Instructions how to do this can be found
[here](https://github.com/Qorvo/connectedhomeip/tree/v1.0.0.0-qorvo/examples/chip-tool)

## Step 2: OpenThread Border router is running and a Thread network is formed
See the guide [How to setup the OpenThread Border Router](setup_qpg7015m_ot_borderrouter.md).

Now a Thread network is created. Proceed to step 3 to prepare the Matter device for joining the network.


## Step 3: Preparing Matter device for commissioning.

Make sure the Matter device firmware is flashed on the development kit and the serial console application is running. See
[Building and flashing the example applications](../../README.md#flashing) and
[Enable serial logging](../../README.md#enable-serial-logging) for instructions.

If you already commissioned the device before, perform a factory reset first. Performing factory reset is dependent on
the Matter applications you are using:
- [Factory reset of the Matter Light](../../Applications/Matter/light/README.md#factory-reset)
- [Factory reset of the Matter Lock](../../Applications/Matter/lock/README.md#factory-reset)
- [Factory reset of Matter Base application](../../Applications/Matter/base/README.md#factory-reset)

After reset of the Matter light application or Matter base application, the device will start Bluetooth LE advertising automatically and is ready for commissioning in the Matter network.

The Matter lock application does not start advertising automatically because of security reasons for a lock device.
Therefore, a manual trigger is needed to initiate advertising. Shortly press `SW5` on the development board to trigger the
advertising. Now the Matter lock device is ready for commissioning in the Matter network.

## Step 3: Commissioning the Matter device in the Matter network.

Commissioning the Matter device in the Thread network is done through Bluetooth LE. The command in chip-tool to be used for commissioning is:

```
sudo ./chip-tool.elf pairing ble-thread <node-id> <operationalDataset> <setup-pin-code> <discriminator>
```

with:
- `<node-id>`: This is the node id to assign to the node being commissioned. Must be a decimal number or a 0x-prefixed
    hex number.
- `<operationalDataset>`: This is the current active operational dataset of the Thread network. This information can be
    retrieved from the running OpenThread Border Router, using below command on the Raspberry Pi that runs the Border
    Router (Also see [Step 6: Get active dataset of the running OpenThread Border Router](setup_qpg7015m_ot_borderrouter.md#step-6:-get-active-dataset-of-the-running-openthread-border-router)):
```
docker exec -it otbr_eth ot-ctl dataset active -x
0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8
Done
```
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
sudo ./chip-tool.elf pairing ble-thread 1 hex:0e080000000000010000000300000d35060004001fffe0020811111111222222220708fd7cc7fe41e171dc051000112233445566778899aabbccddeedf030e4f70656e54687265616444656d6f01021235041061e1206d2c2b46e079eb775f41fc72190c0402a0fff8 20202021 3840
```

> **NOTE:** The base application is using Qorvo vendor identifier and Qorvo certificates to complete the device
> attestation procedure. Because of this, above command will not work as a PAA certificate needs to be added to the
> trust store of the Matter controller (see instructions [Addendum 1: Using the chip-tool with non-default certificates for device attestation](#Addendum-1:-Using-the-chip-tool-with-non-default-certificates-for-device-attestation)). More information around device attestation can be found [here](device_attestation.md).



If the command was successful, following log will appear in the chip-tool:
```
[1641215294.652703][12343:12348] CHIP:TOO: Device commissioning completed with success
```

and following serial output will appear for the Matter device:
```
[P][SVR] Commissioning completed session establishment step

```

Now the Matter device is fully commissioned in the Matter network and you can start controlling the Matter device using the POSIX CLI chip-tool. This will be explained in the next step.

## Step 4: Sending commands to control the Matter device in the network.
### Matter light
To start controlling the Matter device, following command can be used for toggling the Matter light:

```
sudo ./chip-tool.elf onoff toggle <node-id> <endpoint-id>
```

with:
- `<node-id>`: This is the node id that was assigned during commissioning. Must be a decimal number or a 0x-prefixed
    hex number.
- `<endpoint-id>`: endpoint id that holds the OnOff cluster functionality.

Example of how the command looks like is:

```
sudo ./chip-tool.elf onoff toggle 1 1
```

If the command was successful, The RGB led on the development board will be toggled.

### Matter lock
To start controlling the Matter device, following command can be used for unlocking the Matter lock:

```
sudo ./chip-tool.elf doorlock unlock-door <node-id> <endpoint-id> --timedInteractionTimeoutMs 1000
```

and for locking the Matter lock:

```
sudo ./chip-tool.elf doorlock lock-door <node-id> <endpoint-id> --timedInteractionTimeoutMs 1000
```

with:
- `<node-id>`: This is the node id that was assigned during commissioning. Must be a decimal number or a 0x-prefixed
    hex number.
- `<endpoint-id>`: endpoint id that holds the doorlock cluster functionality.

Example of how the command looks like is:

```
sudo ./chip-tool.elf doorlock unlock-door 1 1
```

If the command was successful, the lock is emulated using the cool white led (LD1), this will be toggled.

## Addendum 1: Using the chip-tool with non-default certificates for device attestation

If non-default certificates are programmed on the Matter device, the Matter controller needs to know the PAA certificate
to complete the device attestation validation. In this section, instructions will be given how to do this. As example the
Qorvo certificates will be used (used in the base Matter application):

### Step 1: Make PAA certificate available on the Matter Controller device
Transfer the file [qorvo_paa_cert.der](../../Tools/FactoryData/Credentials/qorvo_paa_cert.der) to a folder on the machine
that runs chip-tool.elf executable. For example /home/ubuntu/cert/qorvo_paa_cert.der.

### Step 2: Make PAA certificate part of the PAA trust store
The trust store is the location where the Matter controller can find the root certificate to complete the certificate
chain validation. In this case this is a local trust store but in the future the Matter controller will need to fetch
a server to find the PAA. To instruct the Matter controller where to find the PAA certificate add following parameter
for the commissioning command: `--paa-trust-store-path /home/ubuntu/cert`. For example, a full command shall look like:

```
sudo ./chip-tool.elf pairing ble-thread 1 hex:0e080000000000010000000300000d35060004001fffe0020811111111222222220708fd7cc7fe41e171dc051000112233445566778899aabbccddeedf030e4f70656e54687265616444656d6f01021235041061e1206d2c2b46e079eb775f41fc72190c0402a0fff8 20202021 3840 --paa-trust-store-path /home/ubuntu/cert

```

Next to this, all other commands listed in this guide can be used without this argument.

## Addendum 2: Using the chip-tool to send Matter Commands

For more information and detailed list how to use the POSIX CLI chip-tool, please refer to the Matter repository
[here](https://github.com/Qorvo/connectedhomeip/tree/v1.0.0.0-qorvo/examples/chip-tool/README.md)
