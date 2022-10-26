# Matter&trade; QPG6105 base example application

Qorvo&reg; QPG6105 Matter base example can be used as basis for creating your own custom device, with Thread
connectivity, using the Matter protocol. It is using Bluetooth&trade; LE to perform Matter commissioning. This example contains a
minimal application layer. In this document, an overview of the Matter base example will be given, together
with step-by-step instructions to customize this application for your needs.

Features of this application are:
1. Stripped down application layer to be used as framework for custom Matter applications.
2. Bluetooth LE for Matter commissioning procedure.
3. Factory reset implementation.

---

- [Button control](#button-control)
- [LED output](#led-output)
- [Factory reset](#factory-reset)
- [Logging output](#logging-output)
- [Building and flashing](#building-and-flashing)
- [Testing the example](#testing-the-example)
  - [Android chip-tool](#android-chip-tool)
  - [POSIX CLI chip-tool](#posix-cli-chip-tool)
- [Creating Matter device](#creating-matter-device)
  - [Introduction](#introduction)
  - [ZAP tool usage](#zap-tool-usage)
  - [Application updates](#application-updates)
  - [Testing the temperature sensor](#testing-the-temperature-sensor)
  - [Further application development](#further-application-development)

---

## Button control

This application uses following buttons of the Qorvo IoT Dev Kit for QPG6105:

- `SW6[RADIORESET]`: Used to perform a HW reset for the full board
- `SW5[PB4]`: Used to perform, depending on the time the button is kept pressed,
  - Trigger Software update (released 0-3s)
  - Factory reset (released after 6s)

The buttons `SW1`, `SW2`, `SW3`, `SW4` and the slider switch `SW7` are unused.

## LED output

The following LEDs are used during the application:

- `LD4` - RED led - shows the device state and its connectivity. Following states are defined:
  - Short blink every 1s: The device is in idle state (not commissioned yet and not Bluetooth LE advertising).
  - Very fast binks: Bluetooth LE advertising.
  - Fast blinks: Bluetooth LE connected and subscribed but not yet commissioned.
  - On: Full service connectivity

## Factory reset

Factory reset of the Matter device can be triggered by holding `SW5` at least 6 seconds.

## Logging Output

For instructions to view the serial logging, refer to [Enable serial logging](../../../README.md#enable-serial-logging)

At startup you will see:

```
NRT ROM v1
qvCHIP <version> ROMv1/1 (CL:0) r:3
ResetCount[0]
[P][DL] BLEManagerImpl::Init() complete
[P][-] Initializing OpenThread stack
[P][DL] OpenThread started: OK
[P][DL] Setting OpenThread device type to ROUTER
[P][-] Starting OpenThread task
[P][-] Starting Platform Manager Event Loop
[P][-] ============================
[P][-] Qorvo Base-Matter-app Launching
[P][-] ============================
```

## Building and flashing

See [Building and flashing the example applications](../../../README.md#building-and-flashing-the-example-applications) section to get instructions how to build and program the Matter base example application.

## Testing the example

The Matter base application will start Bluetooth LE advertising automatically at start-up if it is was not commissioned before
in a fabric. If it is advertising, it is discoverable for a Matter controller to start the Matter commissioning over Bluetooth LE.

The commissioning procedure is done over Bluetooth LE where a connection is setup between a Matter device and a Matter
controller. This Matter controller takes the role of a commissioner.
The commissioner needs to get information from the Matter device to start the commissioning. This information can be
obtained by a QR code or from the serial output of the Matter device.

The final phase in the commissioning procedure is Thread provisioning. This involves sending the Thread network
credentials over Bluetooth LE to the Matter device. Once this is done, the device joins the Thread network and
communication with other Thread devices in the network can be achieved.

### Android chip-tool

For a commissioning guide that makes use of the Android chip-tool, please refer to [Commissioning Qorvo Matter device with Android chip-tool](../../../Documents/Guides/commissioning_android_chiptool.md)

### POSIX CLI chip-tool

For a commissioning guide that makes use of the POSIX cli chip-tool, please refer to [Commissioning Qorvo Matter device with POSIX CLI chip-tool](../../../Documents/Guides/commissioning_posix_cli_chiptool.md)

## Creating Matter device

### Introduction

To create your Matter device, a python tool [*AppCreator*](../../../Tools/AppCreator/) was made to setup the build infrastructure 
for a new application. The newly created structure should allow a user to compile the new application, which can be provisioned in the network.
This can act as a starting point to extend with the needed functionalities.
To demonstrate how to add such functionalities in practice, the below sections will show how the base application can be extended to create
a Matter Temperature Sensor. For this exercise the ADC peripheral will be used on the Qorvo IoT Dev Kit for QPG6105 to report
the measured temperature in the Matter network. In below picture you can find an overview of a Matter node architecture.

<div align="center">
  <img src="Images/architecture.png" alt="Matter application architecture" width="500">
</div>

A Matter node represents a single device in the Matter network. In this chapter we will focus on the application layers
(dark blue) of the Matter node. Each Matter node application layer is defined by the Matter data model layer. The data
model layer is defined by a set of clusters. These clusters contain commands and attributes that can be accessed over
the Matter network. There are two types of clusters. **Base clusters** and **Application clusters**. Base clusters are
clusters that do not implement an application specific feature set. These are used for management and diagnostic
purposes in the Matter network. These are common for all Matter devices.
Application clusters are used to enable specific functionalities that are application specific. In this example, we will
enable the Temperature Measurement cluster to create the Matter temperature sensor.

Clusters are grouped together in endpoints to seperate certain application blocks within the Matter node. In this base
example, a **Matter Root Node endpoint** is made that enables all mandatory clusters to perform Matter commissioning. This
endpoint needs to be available for all Matter devices to be created. Next to this endpoint, you can define a new endpoint
that contains the application specific features. In this example, we will create an endpoint that will enable the
temperature measurement.

To summarize, in this chapter we will:
- Create a Matter node that can commission the Matter network and report the Temperature
- Create an additional endpoint next to the Matter root node that enables the temperature measurements.
- Enable the needed clusters on the second endpoint for temperature measurements.

Make sure to get familiar with the base example first. Below we will extend this base example to create a Matter
temperature sensor. Matter [ZCL Advanced Platform](https://github.com/project-chip/zap) (ZAP) tooling will be used for this.

### ZAP tool usage

In this section, instructions will be given to add, edit and configure clusters using the ZAP tool. Modifying and adding
clusters can be done by modifying the `.zap` file. This `.zap` file defines all the endpoints and clusters to be enabled on
the Matter device. The `.zap` file for the base example can be found next to this README.md file ([base.zap](base.zap)).
This `.zap` file is not used by the Matter application itself, but it is used to generate the necessary source files.

The ZAP tool is a tool that can be used to modify this `.zap` file. Before using the ZAP tool, make sure your environment
is set up correclty. This can be achieved by executing following command:

```
source Scripts/activate.sh
```

To bring up the ZAP tool, execute following command:

```
python3 ./Tools/Zap/generate_zap_files.py --input Applications/Matter/base/base.zap --output Applications/Matter/base/src/zap-generated
```

Now, below window should appear:

<div align="center">
  <img src="Images/zap_cluster_config.png" alt="ZAP cluster configuration">
</div>

As seen in above screenshot, only one endpoint is defined: "Matter Root Node". This endpoint enables all mandatory clusters
to be able to operate in a Matter network. In this example we will not touch the configuration of this endpoint. To
enable Temperature Measurement functionality, we will create a new endpoint. Therefore, click `ADD NEW ENDPOINT` in the
ZAP tool. Below menu will appear:

<div align="center">
  <img src="Images/zap_new_endpoint.png" alt="ZAP adding a new enpoint">
</div>

Fill in the fields as represented in the screenshot to create an endpoint that represents the Matter Temperature Sensor
device type.
Once this is done, all mandatory clusters related to Matter Temperature Sensor device type will be enabled automatically.
You can filter in the Tool on `Only Enabled` clusters to see what clusters are enabled on that endpoint. As you can see
in below screenshot, the Temperature Measurement cluster is enabled as well by default.

<div align="center">
  <img src="Images/zap_temp_measurement_config.png" alt="ZAP Temperature Measurement config">
</div>

In this exercise, the device needs to report the measured temperature so make sure `MeasuredValue` attribute is enabled as
seen in below picture (should be done by default).

<div align="center">
  <img src="Images/zap_temp_measurement_config_detailed.png" alt="ZAP Temperature Measurement config">
</div>

Now, save the file and exit. The script will automatically generate the needed source files in the given output directory.


### Application updates

In this section updates that needs to be done in the Application layer that interact with the used clusters will be
described.

#### Source file additions


If additional clusters are added in the .zap file, you can check the corresponding cluster sources in
```Components/ThirdParty/Matter/repo/src/app/clusters/``` in the source tree.


To avoid having to add the cluster sources manually everytime new clusters are enabled, a script in `Makefile.base_qpg6105` was added to include these files automatically based on the used `.zap` file:
```
ZAP_FILE=$(BASEDIR)/../../../Applications/Matter/base/base.zap
ZAP_SCRIPT=$(BASEDIR)/../../../Components/ThirdParty/Matter/repo/src/app/zap_cluster_list.py
CLUSTERS = $(shell $(ZAP_SCRIPT) --zap_file=$(ZAP_FILE))
CLUSTER_BASE = $(BASEDIR)/../../../Components/ThirdParty/Matter/repo/src/app/clusters
SRC += $(foreach CLUSTER_NAME,$(CLUSTERS),$(wildcard $(CLUSTER_BASE)/$(CLUSTER_NAME)/*.cpp))
```



As we want to enable Temperature sensing in the application, we also need the sources of QPG6105 ADC peripheral to be
added. Copy ```QMatter/Applications/Peripherals/adc/src/adc.c``` to ```QMatter/Applications/Matter/base/src/adc.c```
and copy ```QMatter/Applications/Peripherals/adc/inc/adc.h``` to ```QMatter/Applications/Matter/base/include/adc.h```. Also
make sure that the sources are added in the build tree:

```
SRC_APP+=$(BASEDIR)/../../../Applications/Matter/base/src/adc.c
```

#### Enable Temperature sensor

First the QPG6105 ADC peripheral needs to be initialized when the application starts. This application init is done in
the function ```void Application_Init(void)``` in ```adc.c```. As this function definition is already used in the Matter
application, rename the function to ```void ADC_Init(void)```. Make sure to comment the ```gpBaseComps_StackInit()``` as
Qorvo's software stack will already be initialized as part of the Matter initialization flow. The function in ```adc.c```
shall now look like below:

```
/** @brief Initialize application
*/
void ADC_Init(void)
{
    /* Initialize stack */
    //gpBaseComps_StackInit();

    /* Initialize adc */
    hal_InitADC();

    ...

}
```

Also make sure to make this function publically available by adding its definition in ```adc.h```. Add following lines
in the file:

```
#ifdef __cplusplus
extern "C" {
#endif

void ADC_Init(void);

#ifdef __cplusplus
}
#endif
```

Now you can call the ADC initialization routine from ```CHIP_ERROR AppTask::Init()``` function in ```AppTask.cpp```:

```
CHIP_ERROR AppTask::Init()
{
    ...

    // Enable BLE advertisements
    chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
    ChipLogProgress(NotSpecified, "BLE advertising started. Waiting for Pairing.");

    ADC_Init();

    return err;
}
```

Make sure to add ```#include "adc.h"``` to the include lists in ```AppTask.cpp```.

Once this is done, QPG6105's ADC peripheral will be enabled at start-up of the Matter device. At this stage this will
only log the temperature every 25 seconds. We still need to have a callback function implemented so the application layer
receives the temperature from the temperature sensor.

To achieve this, add following function definition in ```adc.h```: ```void ADC_TempChangedCallback(UInt16 temp);```. This
callback needs to be called from the location where the temperature is logged and needs to be implemented in the
application layer (in this example, in ```AppTask.cpp```).

Snippet of ```adc.h```:

```
#ifdef __cplusplus
extern "C" {
#endif

void ADC_Init(void);
void ADC_TempChangedCallback(UInt16 temp);

#ifdef __cplusplus
}
#endif
```

At the bottom of the function ```Application_MeasureBatteryAndTemperature``` in ```adc.c``` add the following to call
the callback:

```
/** @brief Function to perform a single Battery and temperature measurement */
void Application_MeasureBatteryAndTemperature(void)
{
    ...

    uint16_t tempAdc;
    LOG_TEMPERATURE(adcData);
    tempAdc = HAL_ADC_TEMPERATURE_GET_INTEGER_PART(adcData) * 100 + HAL_ADC_TEMPERATURE_GET_FLOATING_PART(adcData) / 10;
    ADC_TempChangedCallback(tempAdc);
}
```

Implementation of ```void ADC_TempChangedCallback(UInt16 temp)``` belongs to the application layer. So add this function
in ```AppTask.cpp```:

```
void ADC_TempChangedCallback(UInt16 temp)
{
  //Here we need to link to the Temperature Measurement cluster
}
```

Now the only missing part is to link this reported temperature to the Temperature Measurement cluster so the temperature
can be distributed within the Matter network.

#### Link Temperature Measurement cluster

In this exercise we are interested in the `MeasuredValue` attribute of the Temperature Measurement cluster. So when an
update of the temperature sensor comes in, we need to make sure the corresponding attribute gets updated. To do this,
we need to trigger a write to the MeasuredValue attribute of the Temperature Measurement cluster:

```
void ADC_TempChangedCallback(UInt16 temp)
{
    EmberAfStatus status = Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(1, (uint8_t) temp);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
       ChipLogError(NotSpecified, "ERR: updating Temperature %x", status);
    }
}
```

### Testing the temperature sensor

To test the created Matter temperature sensor, complete following steps:
1. Make sure your device is commissioned in the Matter network. Follow the guide
   [Commissioning Qorvo Matter device with POSIX CLI chip-tool](../../../Documents/Guides/commissioning_posix_cli_chiptool.md)
2. Read out the temperature, using POSIX CLI chip-tool can be done by using this command:
```
sudo ./chip-tool.elf temperaturemeasurement read measured-value 1 1
```


Based on this command, similar temperature statements should be visible in the logs

from the base application:
```
TEMPERATURE: 33.363C
```

from the chiptool:
```
[1655468807.846370][8788:8793] CHIP:TOO:   MeasuredValue: 3336
```


### Further application development

Above example gives a very basic hands-on on how to enable clusters and configure them and how to write to its
corresponding attributes. This should also work the other way around. Sometimes attributes can get written due to a
command that comes in from another device in the Matter network. For example, a Matter light switch can send a toggle
command to a Matter light bulb. In that case the write attribute (OnOff attribute of the OnOff cluster in this case)
will be done as part of the OnOff implementation of Matter data model. This needs to be pushed through the application layer so the
application knows it needs to toggle the light.

This means a callback function needs to be implemented so the application is aware that the OnOff attribute is updated.
Implementation of this callback is done in ```MatterPostAttributeChangeCallback``` function. This function is defined
as ```__attribute__((weak))``` in the generated file ```src/zap-generated/callback-stub.cpp``` and can be implemented
as part of the application layer. Stubbed implementation is currently foreseen in ```src/ZclCallbacks.cpp```:

```
void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t mask, uint8_t type,
                                       uint16_t size, uint8_t * value)
{
    return;
}
```

In the ```ZclCallbacks.cpp``` implementation of the [Matter light reference](../light/src/ZclCallbacks.cpp) you can find
an example implementation of this function. This will push updates in the OnOff and LevelControl cluster attributes to
the application layer.

In the file ```src/zap-generated/callback-stub.cpp```, you can find other callback functions defined as
```__attribute__((weak))``` that can be implemented in the application if needed.

More information can also be found in the [Matter API Manual](../../../Documents/API%20Manuals/Matter/Matter_API_Manual.md).
