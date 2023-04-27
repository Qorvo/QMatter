# Performing current measurement on Qorvo QPG6105 Development Kit

The QPG6015 hardware development kit has the needed circuitry to be able to do current consumption measurement.
This can be seen in the image below:

<div align="center">
  <img src="Images/current_measurement.png" alt="Current measurement circuitry" width=300>
</div>

To measure the actual (sleep) current that goes towards the QPG6105 chip make sure to desolder resistor R10. Next, you can put current measurement equipment between the pins on J5.

To be able to capture power plots that represent the current consumption in the time domain, you can place a circuitry with a resistor between the pins of J5 and amplify the voltage. Based on this you can get power plots by using an oscilloscope or logic analyser.

After the measurements are completed, you can mount a jumper bridge on J5 to restore normal operation.
