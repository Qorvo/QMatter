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

# Usage

After loading the program to the board and running:

-   The string “Hello, World!” will be sent at UART TX.

-   Bytes received via UART RX, will be echoed at UART TX.

-   The red LED (LD4) blinks for every byte sent.

For the application that supports wake-up:

-  Bytes received via UART RX, will be echoed at UART TX with '>' and S characters. **(e.g., >aS)**
    - '>' character indicates that the device has been woken up.
    - The 'S' gets printed out right before the device sleeps again.

-   The red LED remains turned off.
