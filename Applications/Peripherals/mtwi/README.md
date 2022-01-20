# Introduction

The Master Two Wire Interface (TWI) peripheral reference application
demonstrates using the TWI to read and write data to a TWI humidity
sensor device.

# Hardware Setup

<div align="center">
  <img src="images/mtwi.png" alt="QPG6105 Smart Home and Lighting Carrier Board, MTWI">
</div>

# QPG6105 GPIO Configurations

| GPIO Name| Direction| Connected To| Comments|
|:----------:|:----------:|:----------:|:---------|
| GPIO0| Output| LD4| Red LED|
| GPIO3| Output| U11| TWI clock (SCL)|
| GPIO2| Bi-direction| U11| TWI data (SDA)|

Usage
=====

After loading the program to the board and running:

-   On startup, the application reads humidity sensor device
    (Si7020-A20) information (Firmware version, serial number).

-   Every 5 seconds the relative humidity and temperature are read back
    and printed to log.

-   The red LED (LD4) blinks every 5 seconds.
