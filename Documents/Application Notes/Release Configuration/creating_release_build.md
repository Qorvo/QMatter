# Adjustable application configurations for production builds

In this guide we will explain how you can adjust your application build configuration to become ready for production. The Matter light application will be used as example for this setup. However, these steps are considered to be generic for any application.

Within the build system of this SDK, a set of macro definitions are used as a hook to make the preprocessor either enable/disable parts of the code, or provide compile time parameters to the application build. These macro definitions are referred to as `DIVERSITY` within this SDK. The `DIVERSITY` mechanism will be used to separate the non-production code from the generic build.

In the sections below we will first describe how to move to a configuration that is intended for producation builds for your applications. In a second section we will give a detailed description of the configurations that are relevant for debug and release configurations.

In this guide you will find a description of the following parts:

---
[How to move from a debug to a production build](#How-to-move-from-a-debug-to-a-production-build)
- [Step 1: Adjust `logging` behaviour](#Step-1:-Adjust-logging-behaviour)
- [Step 2: Adjust `assert` behaviour](#Step-2:-Adjust-assert-behaviour)

[Detailed description of configurations](#Detailed-description-of-configurations)
- [Logging feature and how it is configured](#Logging-feature-and-how-it-is-configured)
- [Assert handling feature and how it is configured](#Assert-handling-feature-and-how-it-is-configured)
- [Debugging feature and how it is configured](#Debugging-feature-and-how-it-is-configured)
---

## How to move from a debug to a production build

By default the applications in this SDK, support a set of features that are useful for the development and testing of the application.
Because these features are enabled they will have an impact on performance, flash size, RAM usage, power consumption, etc.

To have the device performing with optimal parameters in your product, it is advised to follow below steps to disable some features in a production build. 

### Step 1: Disable the configurations for logging

To disable the application logging, the diversity `GP_DIVERSITY_LOG` needs to be disabled. This needs to be done for the application, but also for the library builds. To disable the diversity, it needs to be placed in comment as seen below:

```diff
-#define GP_DIVERSITY_LOG
+//#define GP_DIVERSITY_LOG
```

This needs to be done in:
* [*qorvo_config.h*](../../../Applications/Matter/light/gen/light_qpg6105/qorvo_config.h) of the Application.
* [*qorvo_internals.h*](../../../Libraries/Qorvo/QorvoStack/gen/QorvoStack_qpg6105_mrom/qorvo_internals.h) of the Qorvo stack library.
* [*qorvo_internals.h*](../../../Libraries/Qorvo/MatterQorvoGlue/gen/MatterQorvoGlue_qpg6105_libbuild/qorvo_internals.h) of the Matter-Qorvo glue library.

To disable all Matter stack logging, add below configurations as arguments in the gn build command for the Matter stack library build:
* chip_error_logging=false 
* chip_progress_logging=false 
* chip_detail_logging=false 
* chip_automation_logging=false

This needs to be done in [*Makefile.Matter_light_qpg6105*](../../../Libraries/ThirdParty/Matter/Makefile.Matter_light_qpg6105):

```diff
-    --args=" qpg_target_ic=\"$(QPG_TARGET_IC)\" qpg_sdk_root=\"$(QPG_SDK_ROOT)\" qpg_sdk_lib_dir=\"$(QPG_SDK_LIB_DIR)\" qpg_sdk_include_platform_libs=$(QPG_SDK_INCLUDE_PLATFORM_LIBS) chip_project_config_include_dirs=[\"$(QPG_APPLICATION_INCLUDE_PATH)\"]" \
+    --args="chip_error_logging=false chip_progress_logging=false chip_detail_logging=false chip_automation_logging=false qpg_target_ic=\"$(QPG_TARGET_IC)\" qpg_sdk_root=\"$(QPG_SDK_ROOT)\" qpg_sdk_lib_dir=\"$(QPG_SDK_LIB_DIR)\" qpg_sdk_include_platform_libs=$(QPG_SDK_INCLUDE_PLATFORM_LIBS) chip_project_config_include_dirs=[\"$(QPG_APPLICATION_INCLUDE_PATH)\"]" \
```

For debugging, some default handlers are coupled to a logging function. This is done to be able to analyze the occurrence of a hard fault for example. When a hard fault is triggered, it can dump debugging information over the serial interface. In a production build, these logs can be disabled and therefore these logging functions are not needed anymore. All these functions are implemented in the source file [default_log_handlers.c](../../../Components/Qorvo/HAL_PLATFORM/halCortexM4/src/default_log_handlers.c). So this file can be disabled in the [Makefile](../../../Libraries/Qorvo/QorvoStack/Makefile.QorvoStack_qpg6105_mrom) of the Qorvo stack Library:

```diff
-SRC_halCortexM4+=$(BASEDIR)/../../../Components/Qorvo/HAL_PLATFORM/halCortexM4/src/default_log_handlers.c
+# SRC_halCortexM4+=$(BASEDIR)/../../../Components/Qorvo/HAL_PLATFORM/halCortexM4/src/default_log_handlers.c
```

### Step 2: Adjust the assert behaviour

To have the application trigger an assert in a production ready product is not expected to happen. This 
could cause undefined behaviour. For production purposes, we will convert the behaviour of assert 
to perform a reset. Below diversities needs to set in the application Makefile, but also in the Makefiles of the libraries:

* in [*qorvo_config.h*](../../../Applications/Matter/light/gen/light_qpg6105/qorvo_config.h) of the Application.
* in [*qorvo_config.h*](../../../Libraries/Qorvo/QorvoStack/gen/QorvoStack_qpg6105_mrom/qorvo_config.h) of the Qorvo stack library.
* in [*qorvo_config.h*](../../../Libraries/Qorvo/MatterQorvoGlue/gen/MatterQorvoGlue_qpg6105_libbuild/qorvo_config.h) of the Matter-Qorvo glue library.
* in [*qorvo_config.h*](../../../Libraries/Qorvo/OpenThreadQorvoGlue/gen/OpenThreadQorvoGlue_qpg6105_libbuild/qorvo_config.h) of the Qorvo-OpenThread glue library.

```diff
+#define GP_DIVERSITY_ASSERT_ACTION_RESET
+#define GP_DIVERSITY_ASSERT_REPORTING_NOTHING
```

## Detailed description of configurations

### Logging feature and how it is configured
The following components are used to allow logging to work:
- [Matter stack logging](https://github.com/Qorvo/connectedhomeip/blob/v0.9.9.1/src/lib/support/logging/CHIPLogging.h): Can be used in matter source files to enable/disable the inclusion of the logging strings towards the compiler using preprocessor MACRO definitions. It also controls where these strings are sent towards and how they are buffered and flushed at runtime. This block also handles the string formatting of the parameters, etc.
- [gpLog](../../../Components/Qorvo/OS/gpLog): This block has the same functionality as the logging used in the Matter stack but for Qorvo specific source files. The MACRO definitions are also dedicated to this block. 
- [gpCom](../../../Components/Qorvo/BaseUtils/gpCom): Generic bus-alike component that handles packets to be transferred between different layers of the qorvo stack, such as gpLog/ChipLog to hal_UART.
- [HAL_PLATFORM](../../../Components/Qorvo/HAL_PLATFORM): This component defines the hardware specific dedicated parts such as [hal_UART.c](../../../Components/Qorvo/HAL_PLATFORM/halCortexM4/k8e/src/hal_UART.c). This part will specify how the hardware block is configured and how for example logging strings are buffered at hardware level (RAM / DMA / ...).
- [BSP](../../../Components/Qorvo/BSP): The Board Support Package will specify which hardware block instance will be used (for example UART1 / UART0), to which pins this will be connected, how these PIN io blocks are configured and what happens with the pins related to sleep and interrupt handling.

By default the application supports logging over UART. The logging strings are stored as ascii values in the hex after compilation, they are provided to the compiler through different MACRO definitions.

In this application build, logging strings are directed from 2 parts into gpCom:
- The Qorvo lib logging
```code
    GP_LOG_PRINTF("test string %u", 0, parameter);
    GP_LOG_SYSTEM_PRINTF("test string %u", 0, parameter);
```
- The application and Matter stack logging
```code
    ChipLogProgress(NotSpecified, "test string %u", parameter);
    ChipLogError(NotSpecified, "test string %u", parameter);
```

*Qorvo specific logging statements*:

In the qorvo specific files, the `GP_LOG_SYSTEM_PRINTF` and `GP_LOG_PRINTF` macro definitions are used to encapsulate the logging strings. The difference being that the latter can be disabled at file level by adding or removing `#define GP_LOCAL_LOG` at top level of the source file.

These macro definitions can be used by including the following header in your source files.
```code
#include "gpLog.h"
```

When all DIVERSITY settings are set to enable logging for a specific Qorvo source files in your application build, the following declaration will be made for each of these string MACRO definitions:
```code
#define GP_LOG_PRINTF(s, ...) gpLog_Printf(GP_COMPONENT_ID, true, GP_LOG_PSTR(s), __VA_ARGS__)
#define GP_LOG_SYSTEM_PRINTF(s, ...) gpLog_Printf(GP_COMPONENT_ID, true, GP_LOG_PSTR(s), __VA_ARGS__) 
```

When the logging is disabled for a specific qorvo source file in the application build, these MACRO's will become an empty definition and a such will be ignored by the compiler:
```code
#define GP_LOG_PRINTF(s, ...)
#define GP_LOG_SYSTEM_PRINTF(s, ...)
```
The decision tree for this mapping can be found in ([*gpLog.h*](../../../Components/Qorvo/OS/gpLog/inc/gpLog.h)).

These MACRO definitions will be converted to gpLog_Printf statements and are forwarded from gpLog to gpCom via:

```code
gpCom_DataRequest(GP_COMPONENT_ID_LOG, nbr_chars+1, (UInt8*)buf, GP_LOG_COMMUNICATION_ID);
```
in [gpLog_vsnprintf.c](../../../Components/Qorvo/OS/gpLog/src/gpLog_vsnprintf.c)



*Matter specific logging statements*:

In the Matter specific files, the `ChipLogError`, `ChipLogProgress`, `ChipLogDetail`, `ChipLogAutomation` MACRO definitions are used to encapsulate the logging strings. These used logging levels can be specified globally for the Matter stack at compile time.

These MACRO definitions can be used by including the following header in your source files.
```code
#include <lib/support/logging/CHIPLogging.h>
```

These MACRO definitions will be converted to qvCHIP_Printf which map to qvIO_UartTxData. This call hands over the logging strings to gpCom via:
```code
    gpCom_DataRequest(GP_COMPONENT_ID_QVIO, length, (UInt8*)txBuffer, GP_COM_DEFAULT_COMMUNICATION_ID);
```
in [qvIO.c](../../../Components/Qorvo/BSP/qvIO/src/qvIO.c)

The logging levels can get enabled in the gn build structure via:
```code
chip_error_logging = true
chip_progress_logging =  true
chip_detail_logging =  true
chip_automation_logging = true
```

The decision tree for this mapping can be found in ([*CHIPLogging.h*](https://github.com/Qorvo/connectedhomeip/blob/v0.9.9.1/src/lib/support/logging/CHIPLogging.h)).

These configurations can be added as arguments in the gn build command for the Matter stack library build. For example in [*Makefile.Matter_light_qpg6105*](../../../Libraries/ThirdParty/Matter/Makefile.Matter_light_qpg6105):

```diff
-    --args=" qpg_target_ic=\"$(QPG_TARGET_IC)\" qpg_sdk_root=\"$(QPG_SDK_ROOT)\" qpg_sdk_lib_dir=\"$(QPG_SDK_LIB_DIR)\" qpg_sdk_include_platform_libs=$(QPG_SDK_INCLUDE_PLATFORM_LIBS) chip_project_config_include_dirs=[\"$(QPG_APPLICATION_INCLUDE_PATH)\"]" \
+    --args="chip_error_logging=true chip_progress_logging=true chip_detail_logging=true chip_automation_logging=true qpg_target_ic=\"$(QPG_TARGET_IC)\" qpg_sdk_root=\"$(QPG_SDK_ROOT)\" qpg_sdk_lib_dir=\"$(QPG_SDK_LIB_DIR)\" qpg_sdk_include_platform_libs=$(QPG_SDK_INCLUDE_PLATFORM_LIBS) chip_project_config_include_dirs=[\"$(QPG_APPLICATION_INCLUDE_PATH)\"]" \
```

### Assert handling feature and how it is configured
To decrease the error handling code size of the applications, the build system is using an assert mechanism that can be used 
to highlight and identify issues on debug builds. This mechanism should be used to identify and resolve issues during the R&D
phase of the product before going into production stage. When the testing stage is passed and non of these asserts are triggered
they can be converted to any default behavior for production. 

Within this SDK the default behaviour of any subcomponent ASSERT implementation is assumed to be redirected to the gpAssert component.
This component can be controlled through the a setting for `GP_DIVERSITY_ASSERT_REPORTING` and to specify which assert was triggered.
Next to that another `GP_DIVERSITY_ASSERT_ACTION` that specifies what to do after the assert has occured.

- The list of settings for `GP_DIVERSITY_ASSERT_REPORTING`
```code
// define reporting behavior
#if defined(GP_DIVERSITY_ASSERT_REPORTING_CALLBACK)
#define GP_ASSERT_REPORT(info, compId, file, line)          gpAssert_CallCbAssertIndication(info,compId,file,line)
#elif defined(GP_DIVERSITY_ASSERT_REPORTING_LOG)
#define GP_ASSERT_REPORT(info, compId, file, line)          gpAssert_PrintLogString(compId,file,line)
#elif defined(GP_DIVERSITY_ASSERT_REPORTING_LED_DEBUG)
#define GP_ASSERT_REPORT(info, compId, file, line)          gpAssert_LedDebug(line)
#elif defined(GP_DIVERSITY_ASSERT_REPORTING_NOTHING)
#define GP_ASSERT_REPORT(info, compId, file, line)          do {} while(false)
#endif //GP_DIVERSITY_ASSERT_REPORTING_CALLBACK
```

- The list of settings for `GP_DIVERSITY_ASSERT_ACTION`:
```code
// define action behavior
#if defined(GP_DIVERSITY_ASSERT_ACTION_BLINK_LED)
#define GP_ASSERT_FOLLOW_UP_ACTION()                    gpAssert_BlinkLed()
#elif defined(GP_DIVERSITY_ASSERT_ACTION_EXIT)
#define GP_ASSERT_FOLLOW_UP_ACTION()                    gpAssert_Exit()
#elif defined(GP_DIVERSITY_ASSERT_ACTION_RESET)
#define GP_ASSERT_FOLLOW_UP_ACTION()                    gpAssert_ResetSystem()
#elif defined(GP_DIVERSITY_ASSERT_ACTION_NOTHING)
#define GP_ASSERT_FOLLOW_UP_ACTION()                    do {} while(false)
#endif
```

### Debugging feature and how it is configured
More info in ([debugging with Segger Ozone](../../../Documents/Guides/debugging_with_segger_ozone.md))
