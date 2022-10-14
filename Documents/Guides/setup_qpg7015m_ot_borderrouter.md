# Setup OpenThread Border Router on Qorvo's QPG7015M Gateway development kit

In this guide, step by step instructions are given to setup an OpenThread Border Router, using Qorvo's QPG7015M Gateway
Development Kit. An OpenThread Border Router connects a Thread network to other IP based networks, such as WiFi or
Ethernet. A Thread network requires a Border Router to connect to other networks.

Required Hardware:
- Access point/router to connect your QPG7015M Gateway development kit to a local area network (LAN).
- Qorvo's QPG7015M Gateway development kit based on Raspberry Pi 4.

**Note**: If no external access point or router is available to connect the Raspberry Pi to the network, you can setup a
local access point on the RPi4 as well. To do this, see the section
[Enable a Wifi access point on RPi4](#enable-a-wifi-access-point-on-rpi4)

In this guide you will go through following steps:
1. Accessing the QPG7015M Gateway Development Kit's terminal
2. Configure the QPG7015M Gateway Development Kit to run OpenThread Border Router
3. Start QPG7015M OpenThread Border Router
4. Forming an OpenThread network
5. Collecting logs of the OpenThread Border Router
6. Get active dataset of the running OpenThread Border Router

Also some additional instructions are given to:
- launch an access point on the RPi4,
- stop the OpenThread Border Router,
- factory reset the OpenThread Border Router.

### Step 1: Accessing the QPG7015M Gateway Development Kit's terminal
The QPG7015M Gateway DK comes with a power supply to power the device. Insert the power supply in the USB C port on the
front of the QPG7015M Gateway DK. Access to the QPG7015M Gateway Development Kit terminal is required to configure,
start and stop applications. There are two options to access the terminal of the QPG7015M Gateway Development Kit:
Secure shell or keyboard and screen.

#### Secure shell
Connect the QPG7015M DK to your local area network using an ethernet cable.  From a computer that
has an SSH client application installed the QPG7015M DK can be accessed over SSH by running from a command
prompt (Windows) or terminal (Linux):

```shell
ssh pi@raspberrypi-<last 4 digits of the radio board its serial number>
```
The hostname can be found on the casing. The user of the QPG7015M DK is *pi*, the password is *raspberry*. An example
hostname is `raspberrypi-db99`.

It might be that there is no Domain Name Server (DNS) running in your local setup, in that case it is only possible to
access the RPi over ssh by using its IP address. This can be retrieved by using a keyboard and screen as described in
below bullet point. If this is not available, use the nmap tool on your local PC to retrieve the IP address:

```shell
nmap raspberrypi-db99.local
Starting Nmap 7.80 ( https://nmap.org ) at 2021-11-10 10:49 CET
Nmap scan report for raspberrypi-db99.local (192.168.1.38)
Host is up (0.0024s latency).
Not shown: 997 closed ports
PORT STATE SERVICE
22/tcp open ssh
80/tcp open http
8081/tcp open blackice-icecap
Nmap done: 1 IP address (1 host up) scanned in 0.39 seconds
```

If nmap is not installed, it can be installed by using:
```shell
sudo apt-get install nmap
```

#### Keyboard and screen
Connect a USB keyboard to any of the USB A ports of the QPG7015M DK and attach a screen to a
High-Definition Multimedia Interface (HDMI) Micro port. Please ensure the screen is connected before applying power to
the QPG7015M DK. The user of the QPG7015M DK is *pi*, the password is *raspberry*. If for some reason the hostname
cannot be accessed over ethernet, this is an option to retrieve the IP address running:
```shell
ifconfig
```

### Step 2: Configure the QPG7015M Gateway Development Kit to run OpenThread Border Router
The QPG7015M Gateway Development Kit comes pre installed with several communication stacks and example applications.
These can each be enabled or disabled pending on the user preference using a configuration file: `qorvo_stack_config`.
The configuration file is located in the home directory: `/home/pi`. From a terminal, the user can modify the
configuration file by running:

```shell
sudo nano qorvo_stack_config
```
For running only the OpenThread Border Router make sure to do following changes on `qorvo_stack_config` file:

1. Disable ZigBee stack: Assign `QORVO_ZIGBEE` to `0`
2. Disable Bluetooth LE stack: Assign `QORVO_BLUETOOTHLE` to `0`
3. Disable Bluetooth LE scanning: Assign `QORVO_BLUETOOTHLE_SCANNING` to `0`
4. Enable OpenThread Border Router: Assign `QORVO_OT_BR` to `1`

To save the file run `ctrl + o` followed by `ctrl + x`.

### Step 3: Start QPG7015M OpenThread Border Router
To start the QPG7015M OpenThread Border Router with the configuration set in Step 2, Qorvo provides a script called
`start_gateway.sh`. When executing the script, expect following logging:

```shell
./start_gateway.sh
CONFIGURING GATEWAY SERVER...
Please don't stop the process, This can take a few minutes.
Run time configuration:
QORVO_CHIP=QPG7015M
QORVO_DRIVER_EXT=_RPi4
QORVO_ZB3=ZigBee3.0
QORVO_HOST_INTERFACE=SPI
QORVO_BLE=Bluetooth_LE
QORVO_FIRMWAREUPDATER=FirmwareUpdater
QORVO_FW_IMAGE=FirmwareUpdater/Firmware_QPG7015M.hex
QORVO_PTC=0
QORVO_CTC=0
QORVO_PTC_APP=Ptc
QORVO_CTC_APP=Ctc
QORVO_BOARDCONFIG_DIR=/home/pi/gateway_20220113-031718-QPG7015M/BoardConfigTool
QORVO_BOARDCONFIG_BIN=BoardConfigTool_RPi.elf
QORVO_DRIVERS=Drivers
QORVO_DRIVER_PATH=Drivers/RPi4/5.10.17-v7l+-qorvo/QPG7015M_RPi4
OSAL_DRIVER=OsalDriver_RPi4
OSALOEM_DRIVER=OsalOemDriver_QPG7015M_SPI_RPi4
APP_DRIVER=DrvComKernel_QPG7015M_SPI_RPi4
QORVO_ZIGBEE=0
QORVO_OT_CLI=0
QORVO_OT_BR=1
QORVO_OT_BRBB_INTERFACE=
QORVO_OPENTHREAD=OpenThread
OPENTHREAD_CLI=qpg7015m-ot-cli-ftd.elf
OPENTHREAD_RCP=qpg7015m-ot-rcp.elf
QORVO_COMDUMP=
QORVO_CONFIG_PATH=
QORVO_STARTUP_XML=
QORVO_SUDO=sudo
QORVO_DEBUG=0
QORVO_INTERFERENCE_THRESHOLD=192
QORVO_GW_AT_BOOT=0
QORVO_SOCKET_IPC=/dev/socket
QORVO_MAC_NVM_PATH=/etc/mac/macNvm.dat
APP_DRIVER_DEFAULT=DrvComKernel_QPG7015M_SPI_RPi4
STARTING GATEWAY SERVER...
  PID TTY      STAT   TIME COMMAND
 5533 ?        Ss     0:00 /lib/systemd/systemd --user
 5534 ?        S      0:00 (sd-pam)
 9790 ?        S      0:00 sshd: pi@pts/1
 9796 ?        S      0:00 sshd: pi@notty
 9797 ?        Ss     0:00 /usr/lib/openssh/sftp-server
 9798 pts/1    Ss+    0:00 -bash
10275 ?        S      0:00 sshd: pi@pts/0
10281 ?        S      0:00 sshd: pi@notty
10282 ?        Ss     0:00 /usr/lib/openssh/sftp-server
10283 pts/0    Ss     0:00 -bash
10305 pts/0    S+     0:00 /bin/sh ./start_gateway.sh
10308 pts/0    S+     0:00 /bin/sh ./qorvo_stack_init start
10391 pts/0    S+     0:00 /bin/sh ./qorvo_stack_init start
10392 pts/0    S+     0:00 sleep 1
10394 pts/0    R+     0:00 ps x
DrvComKernel_QPG7015M_SPI_RPi4 module is not loaded.
OsalOemDriver_QPG7015M_SPI_RPi4 module is not loaded.
OsalDriver_RPi4 module is not loaded.
Loading OsalDriver_RPi4 ...
Loading GreenPeak Drivers/RPi4/5.10.17-v7l+-qorvo/QPG7015M_RPi4/OsalDriver_RPi4.ko for  (/dev/gp) kernel module...
No dev node created for module
Loading OsalOemDriver_QPG7015M_SPI_RPi4 ...
Loading GreenPeak Drivers/RPi4/5.10.17-v7l+-qorvo/QPG7015M_RPi4/OsalOemDriver_QPG7015M_SPI_RPi4.ko for  (/dev/gp) kernel module...
No dev node created for module
Loading DrvComKernel_QPG7015M_SPI_RPi4 ...
Loading GreenPeak Drivers/RPi4/5.10.17-v7l+-qorvo/QPG7015M_RPi4/DrvComKernel_QPG7015M_SPI_RPi4.ko for  (/dev/gp) kernel module...
13:35:53:697 01 ============================
13:35:53:698 01 FirmwareUpdater
13:35:53:698 01 v1.5.0.0 Change:188084
13:35:53:698 01 ============================
13:35:53:698 01 loading FirmwareUpdater/Firmware_QPG7015M.hex
13:35:53:861 01 INFO: Bootloader reports ready and valid
13:35:53:862 01 Bootloader is active
13:35:53:862 01 Bootloader Stage 2: v0.9.2.6 Change:151145
13:35:53:862 01 Bootloader Stage1: version data unavailable (request not supported by this stage2 bootloader)
13:35:53:862 01 ProductId: QPG7015
13:35:53:862 01 IEEE 15.4 MAC: 00:15:5f:02:00:42:9c:54
13:35:53:862 01 BLE MAC: 00:15:5f:09:6d:4c
13:35:53:874 01 Verifying CRC...
13:35:54:477 01 image matches
13:35:54:477 01 Starting application...
13:35:59:506 01 Received cbDeviceReady from firmware
13:35:59:506 01 Firmware update SUCCEEDED v1.5.0.0 Change:188084
Firmware update finished successfully (0) ...
Firmware STARTING ...
gateway not running - starting!
NOTICE: Enabling normal mode
13:35:59:522 01 Settings applied
13:35:59:522 01 Enabling normal mode
Loading ot-br docker image from file!
3a7cc06ad581: Loading layer [==================================================>]  48.07MB/48.07MB
57fa7918522d: Loading layer [==================================================>]  81.91MB/81.91MB
d626ab464e19: Loading layer [==================================================>]  197.7MB/197.7MB
Loaded image: connectedhomeip/otbr:te7
d5c744a109086a27a471e8c4b4714dec128ca82ba8ed6d3227d3a3503e0700aa
 * rsyslogd is running
 * dbus is running
Avahi mDNS/DNS-SD Daemon is running
 * otbr-agent is running
 * otbr-web is running
Gateway does not start automatically at boot

-------

Docker container otbr_eth is running and Up 22 seconds.

You can now start ot-ctl command-line utility, and get logs by entering the following commands:

docker exec -it otbr_eth ot-ctl
docker logs otbr_eth [--follow]

-------
GATEWAY STARTED AND READY TO USE!
```

### Step 4: Forming an OpenThread network
Once the OpenThread Border Router is up and running, we can form a Thread network.
This can be achieved by browsing via the webbrowser to the IP address of the Raspberry Pi on which the OpenThread
Border Router is running.

Navigate to 'Form' via the menu on the left. If the menu is not displayed click 'Home' in the top left first.

<div align="center">
  <img src="Images/otbr_landing_page_form.png" alt="OpenThread Border Router Landing Page, Form">
</div>

This will bring you to a new page where you can do some configuration of the OpenThread network. Leave the defaults and
click 'FORM':

<div align="center">
  <img src="Images/otbr_form_page.png" alt="OpenThread Border Router, forming network">
</div>

When prompted 'Are you sure you want to Form the Thread Network'. Click 'OKAY':

<div align="center">
  <img src="Images/otbr_prompt.png" alt="OpenThread Border Router, forming network prompt">
</div>

After a few seconds a message should appear that the Thread network is formed correctly:

<div align="center">
  <img src="Images/otbr_success.png" alt="OpenThread Border Router, forming network success">
</div>

Now a Thread network is created.


### Step 5: Collecting logs of the OpenThread Border Router
For debugging or investigations it is always interesting to be able to collect logs of a running OpenThread Border
Router. This can be done by running following command on the RPi4:

```shell
docker logs otbr_eth
```

To do a live capture of the logging, use the `--follow` parameter as shown below:

```shell
docker logs otbr_eth --follow
```

### Step 6: Get active dataset of the running OpenThread Border Router
To do onboarding of a device in the Thread network, the active dataset is needed. This is for example used during the
commissioning process of a Matter device that runs over Thread. To get the active dataset, following command needs to be
used:

```shell
docker exec -it otbr_eth ot-ctl dataset active -x
0e080000000000010000000300000f35060004001fffe0020811111111222222220708fd791f1a49a1fc8e051000112233445566778899aabbccddeeff030e4f70656e54687265616444656d6f01021234041061e1206d2c2b46e079eb775f41fc72190c0402a0fff8
Done
```

## Stopping the OpenThread Border Router
To stop the QPG7015M Gateway Development Kit, Qorvo provides a script called `stop_gateway.sh`.
Running this script will **NOT** delete the info of the running Thread network.

```shell
./stop_gateway.sh
STOPPING GATEWAY...
Unloading DrvComKernel_QPG7015M_SPI_RPi4 ...
found Drivers/RPi4/5.10.17-v7l+-qorvo/QPG7015M_RPi4/DrvComKernel_QPG7015M_SPI_RPi4.ko
Unloading GreenPeak 'Drivers/RPi4/5.10.17-v7l+-qorvo/QPG7015M_RPi4/DrvComKernel_QPG7015M_SPI_RPi4.ko' kernel module...
Unloading OsalOemDriver_QPG7015M_SPI_RPi4 ...
found Drivers/RPi4/5.10.17-v7l+-qorvo/QPG7015M_RPi4/OsalOemDriver_QPG7015M_SPI_RPi4.ko
Unloading GreenPeak 'Drivers/RPi4/5.10.17-v7l+-qorvo/QPG7015M_RPi4/OsalOemDriver_QPG7015M_SPI_RPi4.ko' kernel module...
Unloading OsalDriver_RPi4 ...
found Drivers/RPi4/5.10.17-v7l+-qorvo/QPG7015M_RPi4/OsalDriver_RPi4.ko
Unloading GreenPeak 'Drivers/RPi4/5.10.17-v7l+-qorvo/QPG7015M_RPi4/OsalDriver_RPi4.ko' kernel module...
```

## Factory resetting the OpenThread Border Router
To stop and factory reset the QPG7015M Gateway Development Kit, Qorvo provides a script called `factory_reset.sh`.
Running this script **WILL** delete the info of the running Thread network.

```
FACTORY RESETTING GATEWAY...
STOPPING GATEWAY...
Unloading DrvComKernel_QPG7015M_SPI_RPi4 ...
found Drivers/RPi4/5.10.17-v7l+-qorvo/QPG7015M_RPi4/DrvComKernel_QPG7015M_SPI_RPi4.ko
Unloading GreenPeak 'Drivers/RPi4/5.10.17-v7l+-qorvo/QPG7015M_RPi4/DrvComKernel_QPG7015M_SPI_RPi4.ko' kernel module...
Unloading OsalOemDriver_QPG7015M_SPI_RPi4 ...
found Drivers/RPi4/5.10.17-v7l+-qorvo/QPG7015M_RPi4/OsalOemDriver_QPG7015M_SPI_RPi4.ko
Unloading GreenPeak 'Drivers/RPi4/5.10.17-v7l+-qorvo/QPG7015M_RPi4/OsalOemDriver_QPG7015M_SPI_RPi4.ko' kernel module...
Unloading OsalDriver_RPi4 ...
found Drivers/RPi4/5.10.17-v7l+-qorvo/QPG7015M_RPi4/OsalDriver_RPi4.ko
Unloading GreenPeak 'Drivers/RPi4/5.10.17-v7l+-qorvo/QPG7015M_RPi4/OsalDriver_RPi4.ko' kernel module...
GATEWAY IS FACTORY RESET!
```

## Enable a Wifi access point on RPi4
By default Ethernet is used, in that case the OpenThread Border Router is part of the network the RPi is wired to. This
is seen in `qorvo_stack_config` file option `QORVO_OT_BRBB_INTERFACE=eth0`.

It is possible to setup a local access point on the RPi as well if there is no wired connection possible or if
you want to set up an isolated network. In that case the OpenThread Border Router needs to be connected through that
wireless access point being set up. To achieve this, you need to assign `QORVO_OT_BRBB_INTERFACE` to `wlan0`. If
`QORVO_OT_BRBB_INTERFACE=wlan0` is set in `qorvo_stack_config` and the script `start_gateway.sh` is run, an access
point will be automatically launched with the SSID `BorderRouter-AP` and password `Qorvo_QPG7015M`. In this case the
OpenThread Border Router can be accessed by connecting over WiFi to this new network.

Note when changing the `wlan0/eth0` interface the docker name in which the OpenThread Border Router runs will also
change. Therefore if you want to access the container, the correct name needs to be used. If `wlan0` is used:
- For collecting the logs, use this command:
```shell
docker logs otbr_wlan0 [--follow]
```
- For getting the active dataset of the OpenThread network:
```shell
docker exec -it otbr_wlan0 ot-ctl dataset active -x
```
