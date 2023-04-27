# Matter&trade; QPG6105 thermostat example application

Qorvo&reg; QPG6105 Matter thermostat example shows how to create a remotely controlled thermostatic radiator valve (TRV) project,
with Thread
connectivity, using Matter. It is using Bluetooth&trade; LE to perform Matter provisioning. This example can be used as reference for
creating your own TRV.

Buttons are used as input to trigger temperature display mode changes and factory reset, LEDs are used as output to visualize the device states.

Features of this application are:
1. Remote TRV control (e.g. system mode, heating/cooling setpoint) using a Matter controller.
2. Bluetooth LE for Matter commissioning procedure.
3. Factory reset implementation.


---

- [Matter™ QPG6105 thermostat example application](#matter-qpg6105-thermostat-example-application)
  - [Button control](#button-control)
  - [LED output](#led-output)
  - [Factory reset](#factory-reset)
  - [Logging output](#logging-output)
  - [Building and flashing](#building-and-flashing)
  - [Testing the example](#testing-the-example)
    - [POSIX CLI chip-tool](#posix-cli-chip-tool)
    - [Google Ecosystem](#google-ecosystem)
    - [Apple Ecosystem](#apple-ecosystem)

---

## Button control

This application uses following buttons of the Qorvo IoT Dev Kit for QPG6105:

- `SW6 (RADIO RESET)`: Used to perform a HW reset for the full board.
- `SW2 (PB2)`: Used to toggle temperature display mode, i.e. toggle between Celsius and Fahrenheit
- `SW5 (PB4)`: Used to perform, depending on the time the button is kept pressed,
  - Trigger OTA (released 0-3s)
  - Factory Reset (released after 6s)

The buttons `SW1`, `SW3`, `SW4` and the slider switch `SW7` are unused.

## LED output

The following LEDs are used during the application:

- `LD1` - WHITE led:
  - Off: Indicate the application is in idle mode
  - On:  Indicate the application is operating
- `LD4` - RED led - shows the device state and its connectivity. Following states are defined:
  - Short blink every 1s: The device is in idle state (not commissioned yet and not Bluetooth LE advertising).
  - Very fast binks: Bluetooth LE advertising.
  - Fast blinks: Bluetooth LE connected and subscribed but not yet commissioned.
  - On: Full service connectivity

## Factory reset

Factory reset of the Matter thermostat can be triggered by holding
`SW5` at least 6 seconds. If the factory reset is triggered by the reset sequence, the light will blink one second ON and
one second OFF, two times.

## Logging output

For instructions to view the serial logging, refer to [Enable serial logging](../../../README.md#enable-serial-logging)

At startup you will see:

```
NRT ROM v1
qvCHIP <version> ROMv1/1 (CL:0) r:3
ResetCount[0]
[P][DL] BLEManagerImpl::Init() complete
[P][-] Initializing OpenThread stack
[P][DL] OpenThread started: OK
[P][DL] Setting OpenThread device type to ROUTER
[P][-] Starting OpenThread task
[P][-] Starting Platform Manager Event Loop
[P][-] ============================
[P][-] Qorvo thermostat-app Launching
[P][-] ============================


```

## Building and flashing

See [Building and flashing the example applications](../../../README.md#building-and-flashing-the-example-applications) section to get instructions how to build and program any Matter example application such as the light application.

## Testing the example

The Matter thermostat will start Bluetooth LE advertising automatically at start-up if it is was not commissioned before
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

For a commissioning guide that makes use of the Google ecosystem, please refer to 
[Commissioning a Qorvo Matter™ end device in the Google ecosystem](../../../Documents/Guides/commissioning_with_google.md).

### Apple Ecosystem

For a commissioning guide that makes use of the Apple ecosytem, please refer to
[Commissioning a Qorvo Matter™ end device in the Apple ecosystem](../../../Documents/Guides/commissioning_with_apple.md).