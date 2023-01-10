# Introduction

This introduction manual provides an overview of the Qorvo&reg; Product Test Component(PTC) System and its intended usage.
The PTC system targets to enable RF validation, RF production test and PHY and MAC certification measurements of the Qorvo IEEE802.15.4 and Bluetooth LE Silicon.
After reading this guide you will have a general understanding of the different blocks in this system
and will be aware of the references towards detailed documentation of the individual blocks.

# The system overview
<div align="center">
  <img src="Images/ptc_overview.png" alt="Overview of PTC" width="500">
</div>

The Qorvo PTC system can be split up in a top level and (multiple) lower level building blocks.
On the top level a Radio Control Package enables to control the Qorvo silicon. The Package includes both a
Radio Control Console (RCC) PC application and a test component driver dll exposing the test API’s.

The lower level blocks implement test functionality for a specific use case. PTC enables to control the radio interface of the target hardware.
Features like Transmitting or Receiving radio frames are implemented.
Detailed info of the supported functionality is described in [RadioControlConsoleUserManual.pdf](../../Documents/User\ Manuals/PTC/GP_P864_UM_12253_RadioControlConsoleUserManual.pdf).
On top of this Qorvo proprietary implementation the PTC system also embeds the Direct Test Mode interface specified by the BLE SIG.


The Coexistence Test Component (CTC) enables to configure the coexistence interface of the target
hardware, however this is not required/supported on end node devices such as QPG6105.

The PTC System requires setting up a physical connection with the firmware running on the target hardware.
In this SDK the connection will be supported over UART.

# The interface

<div align="center">
  <img src="Images/ptc.png" alt="QPG6105 Smart Home and Lighting Carrier Board, Radiated Antenna setup" width="500">
</div>

Qorvo IoT Dev Kit for QPG6105 has an on board USB to serial convertor that allows the host PC to communicate to the target firmware, through the USB-C Connector(J11).

## QPG6105 GPIO Configurations
By default the PTC firmware application in this SDK are configured in the Board Support Package(BSP) to:

| GPIO Name| Direction| Connected To| Comments|
|:----------:|:----------:|:----------:|:---------|
| GPIO9      | Output     |      -     | Configured as UART TX |
| GPIO8      | Input      |      -     | Configured as UART RX |

## Radio Controle Console (RCC) output
When the Qorvo IoT Dev Kit for QPG6105 is connected to your PC, and opening RCC, you will be prompted to select the corresponding COM port of the board and specify the baudrate for this connection.
By default the baudrate will be 57600, configured by the GP_BSP_UART_COM_BAUDRATE.

<div align="center">
  <img src="Images/rcc_overview.png" alt="QPG6105 Smart Home and Lighting Carrier Board, Radiated Antenna setup" width="500">
</div>

Because all dll's support a standard interface contract, the firmware can be prompted by RCC to feedback its firmware version.
As a consequence the RCC goes looking for the correct dll version in its list of intalled Extentions.
When the DLL is selected by RCC, this means the uart connection between RCC and the target firmware is operating successfully.
This would look as follow's:

```
Checking .NET framework version...
.NET Framework Version: 4.6.2 or later
== Quick Edit Mode enabled ==
Looking for TestComps DLL in C:\Program Files (x86)\Qorvo\RadioControlConsole\Extensions
=> Using the following DLL to get target version
=> PTC_QPG6105_10DBM_CFG_A_DIF_v1.10.0.0.dll : (1.10.0.0, PTC_QPG6105_10DBM_CFG_A_DIF)
Found target version : 1.10.0.1 ; productname : PTC_QPG6105_10DBM_CFG_B
Looking for compatible DLL with productname PTC_QPG6105_10DBM_CFG_B and version 1.10.0.1...
=> Selected the following DLL :
=> PTC_QPG6105_10DBM_CFG_B_v1.10.0.1.dll : (1.10.0.1, PTC_QPG6105_10DBM_CFG_B)
Connected to UART COM6 baudrate : 57600
console>
```

# Usage
Now the connection is setup correctly, RCC can be used to send command to the target firmware. For a detailed list of commands use the "H" command or look at [PTC command overview](../../Documents/User%20Manuals/PTC/GP_P864_UM_16380_PTC_Overview.pdf).

To get started, a few commands like Energy Detection Scan, print a list of received packet, change the channel are illustrated below:
```
console> ed 4 1000
console> ED 0
ED 0
ED 0
ED 0
ED Scan finished

console> p
Avg RSSI        : 0
Avg LQI         : 0
RX Packet Count : 0
TX OK           : 0
TX FAIL         : 0
console> ch 15
Attrib changed : CH to 15
```
