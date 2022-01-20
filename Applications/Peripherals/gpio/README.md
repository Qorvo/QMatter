# Introduction

The General-Purpose Inputs and Output (GPIO) reference application
demonstrates using GPIOs as inputs and outputs using buttons and Light
Emitting Diodes (LEDs). One application supports waking up from sleep
using a GPIO interrupt.

# Hardware Setup

<div align="center">
  <img src="images/gpio.png" alt="QPG6105 Smart Home and Lighting Carrier Board, GPIO">
</div>

# QPG6105 GPIO Configurations

| GPIO Name| Direction| Connected To| Comments|
|:----------:|:----------:|:----------:|:---------|
| GPIO0| Input (Pull-up)| SW1| Button (Controlling red LED) |
| GPIO22| Input (Pull-up)| SW2| Button (Controlling white (warm) LED) |
| GPIO3| Input (Pull-up)| SW3| Button (Controlling white (warm) LED) |
| GPIO5| Input (Pull-up)| SW4| Button (Controlling white (warm) LED) |
| GPIO2| Input (Pull-up)| SW5| Button (Controlling white (warm) LED) |
| GPIO4| Input (Pull-up)| SW7| Sliding switch (Controlling white (cool) LED) |


# Usage


After loading the program to the board and running:

-   Switching the SW7 slider on and off will switch the (cool) white LED
    (LD1) on and off.

-   Pressing the SW1 button will turn on the red LED (LD4), releasing it will turn off the red LED (LD4).

-   Pressing and releasing the SW2, SW3, SW4 or SW5 button will toggle the (warm) white
    LED (LD2).

For the application supporting wake-up:

-   Pressing and releasing SW2 will toggle the (warm) white LED LD2

-   Pressing and releasing SW3 will toggle the (cool) white LED LD1

-   When the white leds are turned off, the device will wake up every 5 seconds to blink the (warm) white LED (LD2).

-   The VDDDIG LED (LD3) will be off while the device is in sleep mode.
