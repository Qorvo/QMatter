# Introduction

The Pulse Width Modulation (PWM) reference application demonstrates using
the PWM peripheral to control a Red Green Blue (RGB) Light Emitting
Diode (LED) supporting multiple PWM channels.

# Hardware Setup

<div align="center">
  <img src="images/pwm.png" alt="QPG6105 Smart Home and Lighting Carrier Board, PWM">
</div>

# QPG6105 GPIO Configurations

| GPIO Name| Direction| Connected To| Comments|
|:----------:|:----------:|:----------:|:---------|
| GPIO0 | Output| LD4 | Red LED|
| GPIO14 | Output| D1  | RGB red channel|
| GPIO15 | Output| D1  | RGB green channel|
| GPIO16 | Output| D1  | RGB blue channel|

Usage
=====

After loading the program to the board and running:

-   A defined list of colors is cycled at several intensities using the
    3 PWM channels on the RGB LED (D1).

-   The red LED (LD4) blink for every finished cycle.
