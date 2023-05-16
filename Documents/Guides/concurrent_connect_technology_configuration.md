# ConcurrentConnect&trade; Technology configuration

## Introduction

The QPG6105 its ConcurrentConnect&trade; Technology makes it a key differentiator in IoT solutions. In QMatter following
configurations can be selected for enabling ConcurrentConnect&trade; Technology:

1. **ConcurrentConnect&trade; Multi-Radio capability**
   * Single physical radio solution for concurrent Thread/Matter protocol + Bluetooth LE scanner use cases.
   * E.g. Bluetooth LE scanner device combined with Matter node.
   * If multiple IEEE802.15.4 stacks are enabled on the device (E.g. Thread and ZigBee), they must operate on the same
     channel!
2. **ConcurrentConnect&trade; Antenna Diversity**
   * Real time antenna selection, based on the best communication link for every packet received.
   * This is the default configuration in the reference applications.
3. **ConcurrentConnect&trade; Multi-Channel capability**
   * Single physical radio solution for concurrent Thread/Matter protocol + Other IEEE802.14.4 based stack (for example
     Zigbee).
   * Eg. ZigBee node + Matter node on a single device
   * All IEEE802.15.4 Stacks can operate on different channels!
   * Can only be used when no Bluetooth LE Scanning is required concurrently.

> Note that ConcurrentConnect&trade; Multi-Radio capability can't be combined with ConcurrentConnect&trade; Antenna 
> Diversity nor with ConcurrentConnect&trade; Multi-Channel capability. Compatibility Matrix can be found below:

|                   | **Antenna Diversity** | **Multi-Channel** |
|-------------------|-----------------------|-------------------|
| **Multi-Radio**   |     Not Compatible    | Not Compatible    |
| **Multi-Channel** |       Compatible      | N/A               |
| **Normal**        |       Compatible      | N/A               |

## Configuration

### ConcurrentConnect&trade; Multi-Radio capability

To enable ConcurrentConnect&trade; Multi-Radio capability configuration, the diversities 
`GP_RADIO_DIVERSITY_ENABLE_MULTISTANDARD_LISTENING_MODE` and `GP_HAL_DIVERSITY_MULTISTANDARD_LISTENING_MODE` need to be 
used at compile time. If the diversities are set in the build flags, the multi-radio capability will be selected.

Setting the diversity `GP_RADIO_DIVERSITY_ENABLE_MULTISTANDARD_LISTENING_MODE` and 
`GP_HAL_DIVERSITY_MULTISTANDARD_LISTENING_MODE`needs to be done in the configuration of the Qorvo Stack library. You can 
add the flags in [qorvo_config.h](../../Libraries/Qorvo/QorvoStack/gen/QorvoStack_qpg6105/qorvo_config.h) of the 
corresponding Qorvo Stack that is linked against in the application.

> Note that some flavors are provided of the Qorvo stack configuration (see 
[here](../../Libraries/Qorvo/QorvoStack/gen)). Make sure you update the qorvo_config.h of the stack you are using in 
your application! To check which flavor your application is using, you can check the Makefile of the corresponding 
Matter application. For example, for the Matter light bulb, you can find following lines in 
[Makefile.light_qpg6105_development](../../Applications/Matter/light/Makefile.light_qpg6105_development):

```
.PRECIOUS: $(BASEDIR)/../../../Work/QorvoStack_qpg6105_125degC/libQorvoStack_qpg6105_125degC.a
.PHONY: $(BASEDIR)/../../../Work/QorvoStack_qpg6105_125degC/libQorvoStack_qpg6105_125degC.a
$(BASEDIR)/../../../Work/QorvoStack_qpg6105_125degC/libQorvoStack_qpg6105_125degC.a:
	$(MAKE) -f $(BASEDIR)/../../../Libraries/Qorvo/QorvoStack/Makefile.QorvoStack_qpg6105_125degC
```

This means the QorvoStack with `_125degC` as suffix needs to be selected and updated if you want to configure another 
ConcurrentConnect&trade; Technology for the Matter light bulb. As example, we will enable ConcurrentConnect&trade; 
Multi-Radio capability for the light bulb below:

In [Libraries/Qorvo/QorvoStack/gen/QorvoStack_qpg6105_125degC/qorvo_config.h](../../Libraries/Qorvo/QorvoStack/gen/QorvoStack_qpg6105_125degC/qorvo_config.h)
we add following lines:

```
#define GP_RADIO_DIVERSITY_ENABLE_MULTISTANDARD_LISTENING_MODE
#define GP_HAL_DIVERSITY_MULTISTANDARD_LISTENING_MODE
```

Above configuration will enable ConcurrentConnect&trade; Multi-Radio capability.

### ConcurrentConnect&trade; Antenna Diversity

This is the default configuration of the Matter applications. Antenna Diversity gets enabled when the build flags
`GP_RADIO_DIVERSITY_ENABLE_MULTISTANDARD_LISTENING_MODE` and `GP_HAL_DIVERSITY_MULTISTANDARD_LISTENING_MODE` are **NOT**
set.

> Note that when you configured your application to use single antenna as described 
> [here](./bsp_configuration.md#2-configure-single-antenna), Antenna diversity can't be used. To be able to use Antenna 
> Diversity, both antennas needs to be available and usable.

### ConcurrentConnect&trade; Multi-Channel capability

To enable ConcurrentConnect&trade; Multi-Channel capability the build flags
`GP_RADIO_DIVERSITY_ENABLE_MULTISTANDARD_LISTENING_MODE` and `GP_HAL_DIVERSITY_MULTISTANDARD_LISTENING_MODE` should
**NOT** be set and `GP_DIVERSITY_NR_OF_STACKS` needs to be higher than 1.

By default `GP_DIVERSITY_NR_OF_STACKS` is set to 1 (Matter only application). If you want to add another IEEE802.15.4
stack that runs on another channel, you need to increase `GP_DIVERSITY_NR_OF_STACKS` to 2 (for 2 concurrent IEEE802.15.4
stacks), or to 3 (for 3 concurrent IEEE802.15.4 stacks).

This diversity can be updated in 
[qorvo_internals.h](../../Libraries/Qorvo/QorvoStack/gen/QorvoStack_qpg6105/qorvo_internals.h). Also here, make sure to 
select the correct flavor of Qorvo Stack for your application! See section
[ConcurrentConnect&trade; Multi-Radio capability](#concurrentconnect™-multi-radio-capability) to understand which flavor
needs to be updated.

```diff
-#define GP_DIVERSITY_NR_OF_STACKS      1
+#define GP_DIVERSITY_NR_OF_STACKS      2
```

Increasing the number of stacks and disabling Multi-standard capability, will automatically select the Multi-channel 
capability.