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
| GPIO22| Input (Pull-up)| SW2| Button (Controlling red LED) |
| GPIO3| Input (Pull-up)| SW3| Button (Controlling white (cool) LED) |
| GPIO5| Input (Pull-up)| SW4| Button (Controlling white (warm) LED) |

# Usage

After loading the program to the board and running:
-   Pressing the SW1 button will turn on the red LED (LD4), releasing it will turn off the red LED (LD4).
-   Pressing the SW4 button will turn on the (warm) white LED (LD2), releasing it will turn off the (warm) white LED
    (LD2).
-   Pressing and releasing SW2 button will toggle the red LED (LD4).
-   Pressing and releasing SW3 button will toggle the (cool) white LED (LD1).

# Chip sleep and wakeup example

The application build configuration suffixed with _wkup will put the chip in a low-power sleep mode (indicated by LD3
being off).

Behavior of the buttons in the application with sleep enabled is identical as the application without sleep. Running
the application with sleep will schedule a short wake-up each 5 seconds to flash the warm white LED (LD2).
