# Applications

Several example applications are provided to demonstrate a Matter&trade; device with Thread connectivity, using Bluetooth LE&reg; to perform Matter
provisioning. These examples are compatible with the Qorvo&reg; QPG6105 development boards. More information about each application can be found in
the `README.md` file.

## Matter
Following turn-key Matter solutions are provided as reference:
 - [Base](Applications/Matter/base) - This reference application is a Matter base application that is easy customizable to develop any custom Matter application.
    - *base_uart_rx* - A Matter base reference application variant with UART RX enabled.
 - [Light](Applications/Matter/light) - This reference application demonstrates a Matter dimmable color light.
 - [Lock](Applications/Matter/lock) - This reference application demonstrates a Matter door lock.
 - [Switch](Application/Matter/switch) - This reference application demonstrates a Matter light switch to control for example a Matter light device.
 - [Thermostatic Radiator Valve](Application/Matter/thermostaticRadiatorValve) - This reference application demonstrates a Matter thermostatic radiator valve.

## Bluetooth LE
Following BLE solutions are also provided as reference:
 - [Peripheral](Applications/BluetoothLE/peripheral) - This reference application demonstrate a BLE connectable/bondable peripheral device.

## Secure Bootloader
The example applications are all linked with a secure bootloader for OTA upgrade purpose.
 - [Bootloader](Bootloader) - This is the secure bootloader built to link with all applications.

## Peripheral Examples
The Qorvo IoT Dev Kit for QPG6105 also comes with seven different peripheral example applications. All sources and quick reference documentation for these example applications are listed as below.
- [ADC](Peripherals/adc) - Analog-to-digital conversion example. Temperature, battery voltage, and one analog I/O pin are read and converted to digital.
- [GPIO](Peripherals/gpio) - General purpose I/O example.
- [LED](Peripherals/led) - A LED dimming control example.
- [SPI](Peripherals/mspi) - Reads/write data from/to the external NOR flash through the SPI bus.
- [TWI](Peripherals/mtwi) - Reads/write data from/to the humidity sensor through the TWI bus.
- [PWM](Peripherals/pwm) - A Pulse Width Modulation (PWM) example to control RGB LEDs.
- [UART](Peripherals/uart) - "Hello, World" Example.

## PTC
This Dev Kit includes the Qorvo Product Test Component for validation purposes.
 - [PTC](PTC) - The Product Test Component(PTC) firmware
