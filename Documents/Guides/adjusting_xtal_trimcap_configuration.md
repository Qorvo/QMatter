# Configuring the Capacitor trimming algorithm
## Introduction
This guide describes how to configure the Capacitor trimming algorithm is used
in this repository to compensate the frequency offset error that the 32Mhz crystal
is resonating over its operating temperature range in light bulb products.
This algorithm modifying the load capacitance of the 32MHz crystal to correct
the frequency offset for temperature range between -40°C and +125°C.
The trimming mechanism modifies the values of the internal trimming capacitors in the QPG6105 in a 0 to 3 step range.

More information on the reference light application can be found [*here*](../../Applications/Matter/light/README.md).


## Illustration of frequency offset variations in crystal types
This section explains the issue that is addressed by the capacitance trimming algorithm.

Different crystals experience a different frequency stability characteristic in respect to temperature as can
be seen the figures below. Crystals that are capable to operate in temperature range of -40°C and
+125°C tend to have a horizontal "S" curved plot. In the illustrated plot below, the relative 0ppm frequency offset point
is at +25°C, and a variable point somewhere in the range 90°C-> 115°.

<div align="center">
  <img src="Images/taisaw_freq_vs_temp.png" alt="TAISAW 2.0x1.6 crystal frequency vs temperature plot" width=500>
</div>

<div align="center">
  <img src="Images/hosonic_freq_vs_temp.png" alt="Hosonic 2.5x2.0 crystal frequency vs temperature plot" width=500>
</div>

## Step 1: Use the crystal procurement tool to generate the crystal calibration settings

The graph below shows how the crystal procurement tool is used to generate the configuration of the trimming algorithm to correct with a 3 zones approach:

<div align="center">
  <img src="Images/crystal_procurement_tool_original_3zone_graph.png" alt="Crystal Procurement Tool - Frequency Error versus Temperature" width=500>
</div>

With the 3 zone granularity, some crystals will still be exceeding the boundaries of the allowed specification for the crystal frequency offset. Therefor by increasing the 3 zone approach to a more specific configuration for all transistions will allow these transistion temperatures to be configured smoother. The software trimming algorithm will by default be expecting 7 zones to cover all transistions between -40°C and +125°C as illustrated in the graph below.

<div align="center">
  <img src="Images/crystal_procurement_tool_multi_zone_compenstation_graph.png" alt="Crystal Procurement Tool - Optimized Multiple Zone Compensation" width=500>
</div>

More details on how to use the _Crystal Procurement Tool_ can be found in the [guide](../../Tools/CrystalProcurement/GP_P414_UM_16630_Crystal_Procurement_Tool.pdf).

This guide explains how to use the [Crystal Procurement Tool](../../Tools/CrystalProcurement/GP_P414_TR_16554_Xtal_Procurement_Tool_Python_Version.zip) to generate the header used in the [Step 2](#configuring-the-trimming-algorithm).

## Step 2: Configuring the trimming algorithm

The 32MHz crystal trimming algorithm is enabled in software by using the 125degC flavor of the [QorvoStack library](../../Libraries/Qorvo/QorvoStack/Makefile.QorvoStack_qpg6105_125degC):
```
Libraries/Qorvo/QorvoStack/Makefile.QorvoStack_qpg6105_125degC
```

This library will specifically enable the following [source file](../../Components/Qorvo/HAL_RF/gphal/k8e/src/gpHal_TrimXtal32M.c):
```
SRC_gphal+=$(BASEDIR)/../../../Components/Qorvo/HAL_RF/gphal/k8e/src/gpHal_TrimXtal32M.c
```

And enable the Board Support Package (BSP) header extention for the crystal configuration of the specified board:
[Components/Qorvo/BSP/gpBsp/inc/gpBsp_TrimXtal32M_default.h](../../Components/Qorvo/BSP/gpBsp/inc/gpBsp_TrimXtal32M_default.h)

The content of this header will be the configuration input of the algorithm,
specifying the maximum temperature(in °C) and trimcap setting of the 7 zones allong with the hysteresis.
The corresponding header of the illustrated in _Optimized Mulitple Zone Compensation_ graph above will look something like this:
```
/*****************************************************************************
* Macro Definitions
*****************************************************************************/
#define GP_BSP_TRIMCAP_ZONE1_MAX_TEMP (17)
#define GP_BSP_TRIMCAP_ZONE2_MAX_TEMP (21)
#define GP_BSP_TRIMCAP_ZONE3_MAX_TEMP (30)
#define GP_BSP_TRIMCAP_ZONE4_MAX_TEMP (100)
#define GP_BSP_TRIMCAP_ZONE5_MAX_TEMP (111)
#define GP_BSP_TRIMCAP_ZONE6_MAX_TEMP (119)
#define GP_BSP_TRIMCAP_ZONE1_SETTING (3)
#define GP_BSP_TRIMCAP_ZONE2_SETTING (2)
#define GP_BSP_TRIMCAP_ZONE3_SETTING (1)
#define GP_BSP_TRIMCAP_ZONE4_SETTING (0)
#define GP_BSP_TRIMCAP_ZONE5_SETTING (1)
#define GP_BSP_TRIMCAP_ZONE6_SETTING (2)
#define GP_BSP_TRIMCAP_ZONE7_SETTING (3)

#define GP_BSP_TRIMCAP_HYSTERESIS (3)
```

## Step 3: Verifying the trimming algorithm

When the algorithm is configured with the theorically ideal configuration as mentioned in [step 2](#step-2:-configuring-the-trimming-algorithm) in the trimcap header. The next step is to verify if this can be reproduced on the physical setup in a temperature chamber.

To verify this, the device under test(DUT) is subjected to a temperature sweep with 2 feedback parameters beeing the measured carrier frequency offset and the trimming capacitor setting.

The 2 graphs below are illustrating the result of the measurement for the QPG6105a:
<div align="center">
  <img src="Images/trim_cap_settings_over_temperature_measurement.png" alt="Trim capacitor setting vs temperature plot" width=500>
</div>

<div align="center">
  <img src="Images/carrier_frequency_over_temperature_measurement.png" alt="crystal frequency vs temperature plot" width=500>
</div>

More details on this measurement can be viewed in [Measuring 32 MHz Crystal Oscillator Frequency Compensation over Temperature Range](https://www.qorvo.com/products/p/QPG6105#documents).