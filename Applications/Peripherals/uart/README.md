# Introduction

The Universal Asynchronous Receiver-Transmitter (UART) reference
application demonstrates using the UART to send a message over the UART
Transmitter (TX) and receive data over the UART Receiver (RX).

# Hardware Setup

<div align="center">
  <img src="images/uart.png" alt="QPG6105 Smart Home and Lighting Carrier Board, UART">
</div>

# QPG6105 GPIO Configurations

| GPIO Name| Direction| Connected To| Comments|
|:----------:|:----------:|:----------:|:---------|
| GPIO0      | Output     |     LD4   | red LED      |
| GPIO9      | Output     |      -     | Configured as UART TX |
| GPIO8      | Input      |      -     | Configured as UART RX |

# Serial logging

To capture the serial logging, the tool minicom can be used:

```
minicom -D /dev/ttyACM0 115200
```
Please note that the COM port number and device label **may differ** on your computer.

After resetting the programmed QPG6105 with the application (press the button `SW6 RADIO RESET`), you will see similar output as below:

```
Hello, World!
Hello, World!
```

# Usage

After loading the program to the board and running:

-   The string “Hello, World!” will be sent at UART TX.

-   Bytes received via UART RX, will be echoed at UART TX.

-   The red LED (LD4) blinks for every byte sent.
