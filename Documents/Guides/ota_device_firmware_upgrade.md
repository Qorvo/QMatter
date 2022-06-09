# Matter Over-The-Air (OTA) Device firmware upgrade

In this guide we will explain how you can trigger a download and upgrade of a Matter node in the Matter network. Matter
compliant OTA protocol will be used for this.


<div align="center">
  <img src="../Images/chip_ota_provider_app_setup.png" alt="chip-ota-provider-app setup" width=700>
</div>

**Prerequisites:**
- Qorvo's QPG7015M Gateway development kit is used as OpenThread Border Router. See
[How to setup the OpenThread Border Router](setup_qpg7015m_ot_borderrouter.md).
- Host for the _chip-tool_ such as Linux 64-bit PC running Ubuntu 20.04+ or RPi4 running Ubuntu 20.04+.
- Host for the _ota-provider-app_ such as Linux 64-bit PC running Ubuntu 20.04+ or RPi4 running Ubuntu 20.04+.

_In the example below a RaspberryPi4 will be used as host device for these tools and refered to as RPi._
_If an other platform is selected for one or all of the tools/apps please use the respective folder versions of the tools._

**_Note: Android chip tool has only a subset of the commands below available, as such it isn't supported for ota purposes._**

To do a device upgrade over Matter, two nodes are needed in the Matter network: OTA provider and OTA requestor node.
An OTA provider node is the node that can supply the available software update to the OTA requestor node. In this case
the QPG6105 Matter light node will be the OTA requestor. This node will query the OTA provider node for a software
update. The OTA provider node is for this test your Linux 64-bit PC that runs the OTA provider tool from Matter.

## Step 1: OpenThread Border router is running and a Thread network is formed
See the guide [How to setup the OpenThread Border Router](setup_qpg7015m_ot_borderrouter.md).


## Step 2: Rebuild the application to obtain an update .ota file
A prerequisite for starting the Matter OTA download process is that the version of the application firmware needs to be updated. Else, it will ignore the image. To update the version of the software, and as a consequence also the version of the ota image, the following actions need to be taken:

1. Update the version in the [CHIPProjectConfig.h](../../Applications/Matter/light/include/CHIPProjectConfig.h) CHIPProjectConfig.h as shown below. This will update the application firmware version.

``` diff
 #ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
-#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION 0x0001
+#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION 0x0002
...
 #ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
-#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING "0.1ALPHA"
+#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING "0.2ALPHA"
 #endif
```
2. The version is also part of the OTA header when the .ota file gets generated. Therefore, another version needs to be given to the script that generates this .ota file [generate_ota_img.py](../../Tools/Ota/generate_ota_img.py):
``` diff
-    parser.add_argument('-vn', '--version', type=any_base_int, help='Software version (numeric)', default=1)
-    parser.add_argument('-vs', '--version-str', help='Software version (string)', default="1.0")
+    parser.add_argument('-vn', '--version', type=any_base_int, help='Software version (numeric)', default=2)
+    parser.add_argument('-vs', '--version-str', help='Software version (string)', default="2.0")
```

Once these lines are modified restart the application build to get the updated ota file.

```
cd QMatter/
source Scripts/activate.sh
cd Applications/Matter/light
make -f Makefile.light_qpg6105
```
After successful compilation you can tell the OTA provider tool where it can find the .ota file.

## Step 3: Provide the OTA provider node with the needed .ota file

```
cd QMatter/Tools/MatterOtaProviders/RPi/
sudo ./chip-ota-provider-app.elf -f <path>/<to>/QMatter/Work/light_qpg6105/light_qpg6105.ota
```

Now make sure to keep this application running. Open a new terminal window to complete the next steps.

## Step 4: Commission the OTA provider node in the Matter network
For this the Matter controller tool is needed that can run on the same machine as the chip-ota-provider-app. The OTA
provider will be commissioned as node id 1 in the network:

```
cd QMatter/Tools/MatterControllers/RPi/
sudo ./chip-tool.elf pairing onnetwork 1 20202021
```


## Step 5: Commission the QPG6105 Matter light (OTA requestor) node in the Matter network
Also for this commissioning, the Matter controller tool is needed. First make sure to get the active dataset from the
running OpenThread border router. Details how to retrieve this can be found in the guide
[How to setup the OpenThread Border Router](setup_qpg7015m_ot_borderrouter.md#step-6:-get-active-dataset-of-the-running-openthread-border-router). With this information we can commission the Matter light node with node identifier 2 in the network:

```
sudo ./chip-tool.elf pairing ble-thread 2 hex:<otbr active dataset> 20202021 3840
```

## Step 6: Configure the default OTA provider on the OTA requestor (Matter light)
To do this we need to instruct the Matter controller to write the default OTA provider in the Matter light. We need to
provide the fabricIndex on which the OTA provider lives, also the node identifier we assigned to it and the endpoint of
the OTA software update provider cluster. The last arguments in the command contain the node identifier of the OTA
requestor and the endpoint where the OTA software update requestor cluster lives.

```
sudo ./chip-tool.elf otasoftwareupdaterequestor write default-ota-providers '[{"fabricIndex": 1, "providerNodeID": 1, "endpoint": 0}]' 2 0
```

## Step 7: Configuration Access Control List (ACL) on the OTA provider node
This step is necessary to allow nodes to send commands to the OTA provider node. With below command, you grant Operation
priviledges to all the nodes that are part of the fabric.

```
sudo ./chip-tool.elf accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": null}]' 1 0
```

## Step 8: Trigger download and device upgrade
The final step in the flow is the announcement that needs to be made that a software update is available for the Matter
node. This will trigger the download step of the full .ota image and once the image is taken in, a reboot of the device
will take place to allow the bootloader to upgrade to the new firmware.

```
sudo ./chip-tool.elf otasoftwareupdaterequestor announce-ota-provider 1 0 0 0 2 0
```
Arguments in this command are:
- OTA Provider node identifier [1]
- OTA Provider vendor identifier [0]
- Announcement Reason [0 - SimpleAnnouncement]
- OTA Requestor node endpoint [0]
- OTA Requestor node identifier [2]
- Endpoint to be ignored for group commands [0]

if all goes well, you should now have your application rebooted with the updated version ```0.2ALPHA``` on the ota requestor node:

```
[P][DL] _OnPlatformEvent default: event->Type = 32780
[P][-] Current Software Version: 0.2ALPHA
```
