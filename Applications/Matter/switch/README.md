# Matter&trade; QPG6105 switch example application

Qorvo&reg; QPG6105 Matter switch example shows how to create a switch to remotely control dimmable color light bulb, with Thread
connectivity, using the Matter protocol. It is using Bluetooth&trade; LE to perform Matter commissioning. This example can be used as reference for
creating your own Matter switch.

Buttons are used as input to trigger device state changes, LEDs are used as output to visualize the device states.

Features of this application are:
1. Remote lighting control (dimming, color, on/off) using a Matter controller.
2. Bluetooth LE for Matter commissioning procedure.
3. Factory reset implementation based on number of reset cycles.


---

- [Matter™ QPG6105 switch example application](#matter-qpg6105-switch-example-application)
  - [Button control](#button-control)
  - [LED output](#led-output)
  - [Factory reset](#factory-reset)
  - [Logging output](#logging-output)
  - [Building and flashing](#building-and-flashing)
  - [Testing the example](#testing-the-example)
    - [POSIX CLI chip-tool](#posix-cli-chip-tool)
    - [Google Ecosystem](#google-ecosystem)
    - [Apple Ecosystem](#apple-ecosystem)
  - [Binding with Matter light device](#binding-with-matter-light-device)

---

## Button control

This application uses following buttons of the Qorvo IoT Dev Kit for QPG6105:

- `SW2 (PB2)`: Used to toggle the off/on state of the binded light
- `SW6 (RADIO RESET)`: Used to perform a HW reset for the full board.
- `SW5 (PB4)`: Used to perform, depending on the time the button is kept pressed,
  - Trigger OTA (released 0-3s)
  - Factory Reset (released after 6s)

The buttons `SW1`, `SW3`, `SW4` and the slider switch `SW7` are unused.

## LED output

The following LEDs are used during the application:

- `LD4` - RED led - shows the device state and its connectivity. Following states are defined:
  - Short blink every 1s: The device is in idle state (not commissioned yet and not Bluetooth LE advertising).
  - Very fast binks: Bluetooth LE advertising.
  - Fast blinks: Bluetooth LE connected and subscribed but not yet commissioned.
  - On: Full service connectivity

## Factory reset

Factory reset of the Matter switch can be triggered by holding `SW5` at least 6 seconds.
During factory reset, the RED led `LD4` will be off for a short period of time.

## Logging output

For instructions to view the serial logging, refer to [Enable serial logging](../../../README.md#enable-serial-logging)

At startup you will see:

```
NRT ROM v1
qvCHIP v0.0.0.0 ROMv1/1 (CL:0) r:3
[P][DL] BLEManagerImpl::Init() complete
[P][-] Initializing OpenThread stack
[P][DL] OpenThread started: OK
[P][DL] Setting OpenThread device type to ROUTER
[P][-] Starting OpenThread task
[P][-] Starting Platform Manager Event Loop
[P][-] ============================
[P][-] Qorvo Light-Switch-app Launching
[P][-] ============================
```

## Building and flashing

See [Building and flashing the example applications](../../../README.md#building-and-flashing-the-example-applications) section to get instructions how to build and program the Matter switch example application.

## Testing the example

The Matter switch will start Bluetooth LE advertising automatically at start-up if it is was not commissioned before
in a fabric. If it is advertising, it is discoverable for a Matter controller to start the Matter commissioning over Bluetooth LE.

The commissioning procedure is done over Bluetooth LE where a connection is setup between a Matter device and a Matter
controller. This Matter controller takes the role of a commissioner.
The commissioner needs to get information from the Matter device to start the commissioning. This information can be
obtained by a QR code or from the serial output of the Matter device.

The final phase in the commissioning procedure is Thread provisioning. This involves sending the Thread network
credentials over Bluetooth LE to the Matter device. Once this is done, the device joins the Thread network and
communication with other Thread devices in the network can be achieved.

### POSIX CLI chip-tool

For a commissioning guide that makes use of the POSIX cli chip-tool, please refer to [Commissioning Qorvo Matter device with POSIX CLI chip-tool](../../../Documents/Guides/commissioning_posix_cli_chiptool.md)

### Google Ecosystem

Light switch is not support in Google's Ecosystem. See
[Google's Supported devices page](https://developers.home.google.com/matter/supported-devices#onoff_light_switches) for
more information.

### Apple Ecosystem

Light switch is not supported in Apples's Ecosystem as binding support is not included at the time of writing.

## Binding with Matter light device

Make sure both Light and Switch device are commissioned to the same network.

Assuming that the NodeID of the devices are set to the followings
- Matter Light device: NodeID = `#LightNodeId#`
- Matter Switch device: NodeID = `#SwitchNodeId#`

1. Write binding info to the Switch app
```
./chip-tool.elf binding write binding '[{"fabricIndex": 1, "node": #LightNodeId#, "endpoint": 1, "cluster": 6}, {"fabricIndex": 1, "node": #LightNodeId#, "endpoint": 1, "cluster": 8}]' #SwitchNodeId# 1
```

2. Confirm that the binding info is written successfully
```
./chip-tool.elf binding read binding #SwitchNodeId# 1
```
Output:
```
[1666874049.111186][11048:11053] CHIP:TOO: Endpoint: 1 Cluster: 0x0000_001E Attribute 0x0000_0000 DataVersion: 1511515605
[1666874049.111440][11048:11053] CHIP:TOO:   Binding: 2 entries
[1666874049.111458][11048:11053] CHIP:TOO:     [1]: {
[1666874049.111467][11048:11053] CHIP:TOO:       Node: #LightNodeId#
[1666874049.111472][11048:11053] CHIP:TOO:       Endpoint: 1
[1666874049.111475][11048:11053] CHIP:TOO:       Cluster: 6
[1666874049.111480][11048:11053] CHIP:TOO:       FabricIndex: 1
[1666874049.111484][11048:11053] CHIP:TOO:      }
[1666874049.111488][11048:11053] CHIP:TOO:     [2]: {
[1666874049.111492][11048:11053] CHIP:TOO:       Node: #LightNodeId#
[1666874049.111494][11048:11053] CHIP:TOO:       Endpoint: 1
[1666874049.111497][11048:11053] CHIP:TOO:       Cluster: 8
[1666874049.111502][11048:11053] CHIP:TOO:       FabricIndex: 1
[1666874049.111506][11048:11053] CHIP:TOO:      }
```

3. Write access control info to the Light app
```
./chip-tool.elf accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": [#SwitchNodeId#], "targets": [{"cluster": 6, "endpoint": 1, "deviceType": null}, {"cluster": 8, "endpoint": 1, "deviceType": null}]}]' #LightNodeId# 0
```

4. Confirm that the access control is written successfully
```
./chip-tool.elf accesscontrol read acl #LightNodeId# 0
```
Output:
```
[1666874500.590545][11235:11240] CHIP:TOO: Endpoint: 0 Cluster: 0x0000_001F Attribute 0x0000_0000 DataVersion: 2344203767
[1666874500.590628][11235:11240] CHIP:TOO:   ACL: 2 entries
[1666874500.590658][11235:11240] CHIP:TOO:     [1]: {
[1666874500.590670][11235:11240] CHIP:TOO:       Privilege: 5
[1666874500.590680][11235:11240] CHIP:TOO:       AuthMode: 2
[1666874500.590692][11235:11240] CHIP:TOO:       Subjects: 1 entries
[1666874500.590710][11235:11240] CHIP:TOO:         [1]: 112233
[1666874500.590723][11235:11240] CHIP:TOO:       Targets: null
[1666874500.590733][11235:11240] CHIP:TOO:       FabricIndex: 1
[1666874500.590743][11235:11240] CHIP:TOO:      }
[1666874500.590773][11235:11240] CHIP:TOO:     [2]: {
[1666874500.590783][11235:11240] CHIP:TOO:       Privilege: 3
[1666874500.590792][11235:11240] CHIP:TOO:       AuthMode: 2
[1666874500.590803][11235:11240] CHIP:TOO:       Subjects: 1 entries
[1666874500.590815][11235:11240] CHIP:TOO:         [1]: #SwitchNodeId#
[1666874500.590833][11235:11240] CHIP:TOO:       Targets: 2 entries
[1666874500.590870][11235:11240] CHIP:TOO:         [1]: {
[1666874500.590879][11235:11240] CHIP:TOO:           Cluster: 6
[1666874500.590889][11235:11240] CHIP:TOO:           Endpoint: 1
[1666874500.590899][11235:11240] CHIP:TOO:           DeviceType: null
[1666874500.590908][11235:11240] CHIP:TOO:          }
[1666874500.590925][11235:11240] CHIP:TOO:         [2]: {
[1666874500.590933][11235:11240] CHIP:TOO:           Cluster: 8
[1666874500.590942][11235:11240] CHIP:TOO:           Endpoint: 1
[1666874500.590950][11235:11240] CHIP:TOO:           DeviceType: null
[1666874500.590958][11235:11240] CHIP:TOO:          }
[1666874500.590970][11235:11240] CHIP:TOO:       FabricIndex: 1
[1666874500.590993][11235:11240] CHIP:TOO:      }
```

5. Press `SW2` will toggle the LEDs on the Light device.
