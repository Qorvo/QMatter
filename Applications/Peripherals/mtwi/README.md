# Introduction

The Master Two Wire Interface (TWI) peripheral reference application
demonstrates using the TWI to read and write data to a TWI humidity
sensor device.

# Hardware Setup

<div align="center">
  <img src="Images/mtwi.png" alt="QPG6105 Smart Home and Lighting Carrier Board, MTWI">
</div>

# QPG6105 GPIO Configurations and required jumper settings

|   GPIO Name|    Direction| Required jumper setting| Connected To| Comments|
|:----------:|:-----------:|:----------------------:|:-----------:|:---------|
|       GPIO0|       Output|                     N/A|          LD4| Red LED|
|       GPIO3|       Output|                 J19 1-2|          U11| TWI clock (SCL)|
|       GPIO2| Bi-direction|                 J20 1-2|          U11| TWI data (SDA)|


# Serial logging

For getting the serial logging for this application, the Qorvo tool Jadelogger needs to be used. It is used to send
and receive data from a Qorvo device. It is a serial/TCP terminal that uses a proprietary protocol to decode numbers,
offer additional context and increases robustness (using SYN). Jadelogger decodes this protocol and prints using
following format:

| Timestamp host | Module ID | Timestamp device | Log |
|:----------:|:----------:|:----------:|:---------|
| 2022-06-15 10:27:26.754| 01 | 0.051488| Reset Done |


To start Jadelogger, execute following command:

```
python3 QMatter/Tools/Jadelogger/gppy/tools/jadelogger.py /dev/ttyACM0:115200
```

Please note that the COM port number and device label **may differ** on your computer.

After resetting the programmed QPG6105 with the application (press the button `SW6 RADIO RESET`), you will see similar output as below:

```
2022-06-17-15:45:15.711  00 2.675919 NRT ROM v1
2022-06-17-15:45:15.735  00 2.699969 Reset Done
2022-06-17-15:45:15.735  00 2.700072 Firmware Revision 20
2022-06-17-15:45:15.753  00 2.718390 Serial number: c2:66:22:c1:14:41:ff:ff
2022-06-17-15:45:15.753  00 2.718618 Relative Humidity: 37 Percent
2022-06-17-15:45:15.859  00 2.824212 Temperature:       25.6 Degrees Celcius
```

# Usage


After loading the program to the board and running:

-   On startup, the application reads humidity sensor device
    (Si7020-A20) information (Firmware version, serial number).

-   Every 5 seconds the relative humidity and temperature are read back
    and printed to log.

-   The red LED (LD4) blinks every 5 seconds.
