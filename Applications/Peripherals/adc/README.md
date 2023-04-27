# Introduction

The Analog-to-Digital Converter (ADC) peripheral reference application
demonstrates the use of the built-in ADC to log temperature, battery voltage and one
of the Analogue Input Output (ANIO) channel voltages.

# Hardware Setup

<div align="center">
  <img src="Images/adc.png" alt="QPG6105 Smart Home and Lighting Carrier Board, ADC">
</div>

# QPG6105 GPIO Configurations

The jumper J13 of Qorvo&reg; IoT Dev Kit for QPG6105 can be used to switch the ADC input ANIO0 between the potentiometer and the PIR sensor.
Verify the jumper is set to 1-2 ('POT').

| GPIO Name | Jumper setting | Connected To | Comments |
|:----------:|:----------:|:----------:|:---------|
| GPIO21| 1-2 | POT (R67)| ANIO0 jumper configured to use ADC input of the potentiometer. |
|| 2-3 | PIR Sensor (U4)| ANIO0 jumper configured to use ADC input of the PIR sensor. |


# Serial logging

For instructions to view the serial logging, refer to [Enable serial logging](../../../README.md#enable-serial-logging).

After resetting the programmed QPG6105 with the application (press the button `SW6 RADIO RESET`), you will see similar output as below:

```
2022-06-16-15:11:34.578  23 0.000480 NRT ROM v1
2022-06-16-15:11:35.557  01 1.001888 == Resetting ANIO values
2022-06-16-15:11:35.665  01 1.002080 -- Measuring cycle 0 --
2022-06-16-15:11:35.665  01 1.002368 LIVE voltage : 1.354V
```

# Usage

After loading the program onto the board, the program starts to run:
-   Every 5 seconds, the application reads the ANIO0 channels and logs
    the values to UART.
-   Every 5 ANIO readings, the application will read and log the
    internal temperature value and voltage.
-   Changing the resistance of the potentiometer will trigger changes in
    values of the live measurement.

# Chip sleep and wakeup example

Both application builds, adc and adc_wkup, will put the chip in a low-power sleep mode (indicated by LD3
being off).

While the application suffixed with _wkup will wake up when ANIO 0 reads a voltage larger than around 0.2V.
