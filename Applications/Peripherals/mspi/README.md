# Introduction

The Master Serial Peripheral Interface (MSPI) peripheral reference
application demonstrates using the MSPI to read and write data to an SPI
flash device.

# Hardware Setup

<div align="center">
  <img src="images/mspi.png" alt="QPG6105 Smart Home and Lighting Carrier Board, MSPI">
</div>

# QPG6105 GPIO Configurations

| GPIO Name| Direction| Connected To| Test Pin| Comments|
|:----------:|:----------:|:----------:|:----------:|:---------|
| GPIO0| Output| LD4| -| Red LED|
| GPIO1| Output (SSn)| U5 (Pin 1)| TP19| Flash memory slave select|
| GPIO10| Output (SCLK)| U5 (Pin 6)| TP18| Flash memory SPI clock|
| GPIO11| Output (MOSI)| U5 (Pin 5)| TP20| Flash memory master out slave in (MOSI)|
| GPIO12| Output (MISO)| U5 (Pin 2)| TP21| Flash memory master in slave out (MISO)|

# Usage

After loading the program to the board and running:

-   The SPI flash device (AT25SF081) information is read back
    (Manufacturer ID, memory type and memory capacity).

-   A single byte is written to and read back from flash and logged.

-   A block of data is written to and read back from flash and logged.

-   The red LED (LD4) blinks for every flash read/write cycle.
