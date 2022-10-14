# Jadelogger

For getting the serial logging for certain applications, the Qorvo tool Jadelogger needs to be used. It is used to send
and receive data from a Qorvo device. It is a serial/TCP terminal that uses a proprietary protocol to decode numbers,
offer additional context and increases robustness (using SYN). Jadelogger decodes this protocol and prints using
following format:

| Timestamp host | Module ID | Timestamp device | Log |
|:----------:|:----------:|:----------:|:---------|
| 2022-06-15 10:27:26.754| 01 | 0.051488| Reset Done |


To start Jadelogger, execute following command:

```
python3 QMatter/Tools/Jadelogger/gppy/tools/jadelogger.py /dev/ttyACM0:115200
```

Please note that the COM port number and device label **may differ** on your computer.

Note that this tool only needs to be used for getting serial logging of the [Peripherals applications](../../Applications/Peripherals)
All other applications use raw serial output which can be captured by any serial terminal application (for example, Minicom).
