# Board Support Package configurations
## Introduction
This guide describes the common hardware configurations.
<br /><br />

## 1. Enable 32kHz Crystal Oscillator
The 32KHz crystal oscillator is optional. It will only be used when sleep mode is enabled for having more accurate timing result. The 32KHz crystal is by default disabled in this SDK, the internal low jitter RC oscillator is used instead. If 32kHz crystal is needed in the project, please be reminded to update the definition of diversity **GP_BSP_32KHZ_CRYSTAL_AVAILABLE()** from *0* to *1* in [*BSP header file*](../../Components/Qorvo/BSP/gpBsp/inc/SmartHomeAndLighting/gpBsp_QPG6105DK_B01.h).

```c
#define GP_BSP_32KHZ_CRYSTAL_AVAILABLE() 1 //1 - enable 32KHz crystal, 0 - disable 32KHz crystal
```
<br />

## 2. Configure Single Antenna
The QPG6105 has 2 antenna ports with integrated switch, matching and RF filters. Both **RF Port 1** and **RF Port 2** are enabled by default in this SDK. If a single antenna is used in the project, please be reminded to add the diversity **GP_HAL_DIVERSITY_SINGLE_ANTENNA** in *qorvo_config.h* in QMATTER/Libraries/Qorvo/QorvoStack/gen/{module} and specify the antenna used to this diversity as below:

```c
#define GP_HAL_DIVERSITY_SINGLE_ANTENNA  0 //Force to use RF Port 1
```
OR
```c
#define GP_HAL_DIVERSITY_SINGLE_ANTENNA  1 //Force to use RF Port 2
```

For example, if it is needed to force the light application to use single antenna, please add the above diversity definition in [*125degC qorvo_config header file*](../../Libraries/Qorvo/QorvoStack/gen/QorvoStack_qpg6105_125degC/qorvo_config.h). For other applications, e.g. lock application, please update [*qorvo_config header file*](../../Libraries/Qorvo/QorvoStack/gen/QorvoStack_qpg6105/qorvo_config.h).


Note that the antenna configuration can be read at runtime by calling gpRadio_GetRxAntenna(), so to verify the antenna selection you can call:

```c
gpRadio_AntennaSelection_t antsel = gpRadio_GetRxAntenna();
```


## 3. Enable sleep mode in a high-temperature environment (>85 degrees Celsius)
Please be reminded that keeping the flash component in active state in high-temperature environments (>85 degrees Celsius) has a negative impact and will reduce product lifetime. Therefore, it is recommended to enable sleep in all high temperature applications with below API defined in [*qvIO.h*](../../Components/Qorvo/BSP/qvIO/inc/qvIO.h):

```c
qvIO_EnableSleep(true); //true - enable sleep mode, false - disable sleep mode.
```

Different crystals experience a different frequency stability characteristic in respect to temperature. The [*Adjusting xtal trimcap configuration guide*](./adjusting_xtal_trimcap_configuration.md) explains the issue and describes how to compensate the frequency offset error of the 32MHz crystal over its operating temperature.
