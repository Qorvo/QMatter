# Matter&trade; protocol Application Programmer Interface

---
- [Matter™ protocol Application Programmer Interface](#matter-protocol-application-programmer-interface)
  - [Introduction](#introduction)
  - [Initialization flow](#initialization-flow)
    - [Step 1: Initialization of the Qorvo stack](#step-1-initialization-of-the-qorvo-stack)
    - [Step 2: Initialization of the Matter stack](#step-2-initialization-of-the-matter-stack)
      - [1. Memory initialization:](#1-memory-initialization)
      - [2. Core Matter initialization:](#2-core-matter-initialization)
      - [3. OpenThread stack initialization:](#3-openthread-stack-initialization)
      - [4. Configuration of Thread device type:](#4-configuration-of-thread-device-type)
      - [5. Start the OpenThread task:](#5-start-the-openthread-task)
      - [6. Starting platform manager event loop:](#6-starting-platform-manager-event-loop)
    - [Step 3: Start application task](#step-3-start-application-task)
    - [Step 4: Application specific initialization](#step-4-application-specific-initialization)
      - [1. Initialization of ZCL data model and starting the server:](#1-initialization-of-zcl-data-model-and-starting-the-server)
      - [2. Configure provider for device instance info:](#2-configure-provider-for-device-instance-info)
      - [3. Configure provider for commissionable data:](#3-configure-provider-for-commissionable-data)
      - [4. Configure the Device Attestation Credentials provider:](#4-configure-the-device-attestation-credentials-provider)
      - [5. Open commissioning window:](#5-open-commissioning-window)
      - [6. Get link for QR code printed:](#6-get-link-for-qr-code-printed)
  - [Interface towards the Matter data model implementation](#interface-towards-the-matter-data-model-implementation)
    - [Callback functions](#callback-functions)
      - [1. Cluster initialization callback:](#1-cluster-initialization-callback)
      - [2. Capture cluster attribute changes:](#2-capture-cluster-attribute-changes)
    - [Attribute getter/setter functions](#attribute-gettersetter-functions)
    - [Other](#other)
  - [Timers](#timers)
---

## Introduction

This document will describe the Application Programmer Interface (API) to be used for developing a Matter device. This document focusses on the software parts that are necessary to develop a Matter node.
First a description will be given of the initialization flow that needs to happen before the actual application can start. This flow includes initialization of the Qorvo stack, the Matter protocol stack and the application.
Next, a description will be given how to interface with the Matter data model. Finally, the API on how to use timers in your application is explained.

All these parts are implemented as reference in following example applications:
- [Matter light](../../../Applications/Matter/light)
- [Matter lock](../../../Applications/Matter/lock)
- [Matter switch](../../../Applications/Matter/switch)
- [Matter thermostatic radiator valve](../../../Applications/Matter/thermostaticRadiatorValve)
- [Matter base](../../../Applications/Matter/base)


## Initialization flow

Entry point of the Matter application is the main routine in [main.cpp](../../../Applications/Matter/shared/src/main.cpp).
In this file you can find a reference implementation of the initialization flow that is described below.

### Step 1: Initialization of the Qorvo stack

First step is to initialize the Qorvo stack. This is done with following function:

```
/** @brief Initialize Qorvo needed components for CHIP.
*   @return result                   0 if init was successful. -1 when failed
*/
int qvCHIP_init(application_init_callback_t application_init_callback);

```

As seen in the *qvCHIP_Init* function, it takes a function pointer as argument. This is a callback function that is
triggered once all the necessary Qorvo components are initialized. In this callback function, the next steps in the
initialization flow need to be started (see Step 2).


### Step 2: Initialization of the Matter stack

Once all the Qorvo components are initialized, we can initialize the Matter stack. The following initializations routines
needs to be triggered to do this:


#### 1. Memory initialization:
Initialization of memory and resources required for proper functioning of the Matter memory allocator. Below function of
the class *chip::Platform* is used for this:

```
/**
 * This function is called by CHIP layer to initialize memory and resources
 * required for proper functionality of the CHIP memory allocator.
 * This function is platform specific and might be empty in certain cases.
 * For example, this function is doing nothing when the C Standard Library malloc()
 * and free() functions are used for memory allocation.
 *
 * @param[in]  buf      A pointer to a dedicated memory buffer, which should be used as
 *                      a memory pool for CHIP memory allocation.
 *                      This input is optional (defaults to NULL) and shouldn't be used
 *                      if a dedicated memory buffer is not used.
 *
 * @param[in]  bufSize  Size of a dedicated memory buffer. This input is optional (defaults to 0)
 *                      and shouldn't be used if dedicated memory buffer is not used.
 *                      When a dedicated memory buffer is used the function checks and
 *                      generates an error if buffer size is not big enough to support
 *                      CHIP use cases.
 *
 * @retval  #CHIP_ERROR_BUFFER_TOO_SMALL  If dedicated input buffer size is not sufficient
 *                                         to support CHIP use cases.
 * @retval  #CHIP_NO_ERROR                On success.
 * @retval  other                          An error generated by platform-specific memory
 *                                         initialization function.
 *
 */
extern CHIP_ERROR MemoryInit(void * buf = nullptr, size_t bufSize = 0);

```

#### 2. Core Matter initialization:
Initialization of the base components needed for Matter. Below function is used for this:

```
inline CHIP_ERROR PlatformManager::InitChipStack()
{
    // NOTE: this is NOT thread safe and cannot be as the chip stack lock is prepared by
    // InitChipStack itself on many platforms.
    //
    // In the future, this could be moved into specific platform code (where it can
    // be made thread safe). In general however, init twice
    // is likely a logic error and we may want to avoid that path anyway. Likely to
    // be done once code stabilizes a bit more.
    if (mInitialized)
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR err = static_cast<ImplClass *>(this)->_InitChipStack();
    mInitialized   = (err == CHIP_NO_ERROR);
    return err;
}
```

#### 3. OpenThread stack initialization:
Initialization of the OpenThread stack that is needed as network layer for the Matter stack. Below function is used
for this:

```
inline CHIP_ERROR ThreadStackManager::InitThreadStack()
{
    return static_cast<ImplClass *>(this)->_InitThreadStack();
}
```

#### 4. Configuration of Thread device type:
Indicates which rule in the Matter network the device needs to take. Below function is used for this:

```
inline CHIP_ERROR ConnectivityManager::SetThreadDeviceType(ThreadDeviceType deviceType)
{
    return static_cast<ImplClass *>(this)->_SetThreadDeviceType(deviceType);
}
```

Possible Thread device types are mentioned below:

```
        kThreadDeviceType_NotSupported                = 0,
        kThreadDeviceType_Router                      = 1,
        kThreadDeviceType_FullEndDevice               = 2,
        kThreadDeviceType_MinimalEndDevice            = 3,
        kThreadDeviceType_SleepyEndDevice             = 4,
        kThreadDeviceType_SynchronizedSleepyEndDevice = 5,
```

#### 5. Start the OpenThread task:
Starting of the OpenThread stack. Below function is used for this:

```
inline CHIP_ERROR ThreadStackManager::StartThreadTask()
{
    return static_cast<ImplClass *>(this)->_StartThreadTask();
}
```

#### 6. Starting platform manager event loop:
Registration of an Event handler and starting of the task. Below function is used for the registration of the event
handler:

```
inline CHIP_ERROR PlatformManager::AddEventHandler(EventHandlerFunct handler, intptr_t arg)
{
    return static_cast<ImplClass *>(this)->_AddEventHandler(handler, arg);
}
```

To kick the actual task, below function is used:
```
/**
 * @brief
 *  Starts the stack on its own task with an associated event queue
 *  to dispatch and handle events posted to that task.
 *
 *  This is thread-safe.
 *  This is *NOT SAFE* to call from within the CHIP event loop since it can grab the stack lock.
 */
inline CHIP_ERROR PlatformManager::StartEventLoopTask()
{
    return static_cast<ImplClass *>(this)->_StartEventLoopTask();
}
```

### Step 3: Start application task

Now the Qorvo stack and the Matter stack components are all initialized, the main application task can start. A
reference implementation of such application task can be found in below function. An event queue buffer is allocated to
the task, and the application task gets started with its own main function *AppTaskMain*.

```
CHIP_ERROR AppTask::StartAppTask()
{
    sAppEventQueue = xQueueCreateStatic(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent), sAppEventQueueBuffer, &sAppEventQueueStruct);
    if (sAppEventQueue == nullptr)
    {
        ChipLogError(NotSpecified, "Failed to allocate app event queue");
        return CHIP_ERROR_NO_MEMORY;
    }

    // Start App task.
    sAppTaskHandle = xTaskCreateStatic(AppTaskMain, APP_TASK_NAME, ArraySize(appStack), nullptr, 1, appStack, &appTaskStruct);
    if (sAppTaskHandle == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

```

### Step 4: Application specific initialization

This initialization step is off course application specific and differs depending on the application you want to run.
But some application initialization steps will be needed for all Matter applications being implemented:

#### 1. Initialization of ZCL data model and starting the server:
Server in this context means an aggregate for all the resources needed to run a Matter node that is both commissionable
and mainly used as an end node with server clusters. In other words, it aggregates the state needed for the type of Node
used for most products that are not mainly controller/administrator role.

A reference implementation of this step is provided in the
function *AppTask::InitServer(intptr_t arg)* which is implemented in
[AppTask.cpp](../../../Applications/Matter/light/src/AppTask.cpp). Below a summary of the needed functions is listed.

Before actual initialization of the server, initialization of the internally owned resources needs to be done by using
the following member function of the class *chip::CommonCaseDeviceServerInitParams*:

```
    /**
     * Call this before Server::Init() to initialize the internally-owned resources.
     * Server::Init() will fail if this is not done, since several params required to
     * be non-null will be null without calling this method. ** See the transition method
     * in the outer comment of this class **.
     *
     * @return CHIP_NO_ERROR on success or a CHIP_ERROR value from APIs called to initialize
     *         resources on failure.
     */
    virtual CHIP_ERROR InitializeStaticResourcesBeforeServerInit()
```

Next, set the storage implementation used for non-volatile storage of device information data by using the following
member function of the class *chip::DeviceLayer::DeviceInfoProvider*:

```
    /**
     * @brief Set the storage implementation used for non-volatile storage of device information data.
     *
     * @param storage Pointer to storage instance to set. Cannot be nullptr, will assert.
     */
    void SetStorageDelegate(PersistentStorageDelegate * storage);
```

The device information provider to be used needs to be set using following member function of the class
*chip::DeviceLayer*:

```
/**
 * Instance setter for the global DeviceInfoProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * If the `provider` is nullptr, no change is done.
 *
 * @param[in] provider the Device Info Provider
 */
void SetDeviceInfoProvider(DeviceInfoProvider * provider);

```

The server gets initialized using the following member function of the class *chip::Server*:

```
CHIP_ERROR Init(const ServerInitParams & initParams);
```

#### 2. Configure provider for device instance info:
Device instance information is information like vendor identifier, product identifier, serial number, etc. To let the
software know where it can find this information a provider needs to be set by using below function of the class
*chip:DeviceLayer:DeviceInstanceInfoProvider*:

```
/**
 * Instance setter for the global DeviceInstanceInfoProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * If the `provider` is nullptr, no change is done.
 *
 * @param[in] provider the DeviceInstanceInfoProvider pointer to start returning with the getter
 */
void SetDeviceInstanceInfoProvider(DeviceInstanceInfoProvider * provider);

```

Note: In the reference application, a dedicated read-only persistent storage space (factory block) is used for this.


#### 3. Configure provider for commissionable data:
Commissionable data is all the data needed for being able to commission (discriminator, passcode, etc. ) a device. To
let the software know where it can find this information, a provider needs to be set by using the following member
function of the class *chip:DeviceLayer:CommissionableDataProvider*:

```
/**
 * Instance setter for the global CommissionableDataProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * If the `provider` is nullptr, no change is done.
 *
 * @param[in] provider the CommissionableDataProvider to start returning with the getter
 */
void SetCommissionableDataProvider(CommissionableDataProvider * provider);
```

Note: In the reference application, a dedicated read-only persistent storage space (factory block) is used for this.

#### 4. Configure the Device Attestation Credentials provider:
Device Attestation Credentials are all the data that are needed to complete the Device Attestation process. This includes
the certificates and the certification declaration. To let the software know where it can find this information a
provider needs to be set by using below function of the class *chip:Credentials:DeviceAttestationCredentialsProvider*:

```
/**
 * Instance setter for the global DeviceAttestationCredentialsProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * If the `provider` is nullptr, no change is done.
 *
 * @param[in] provider the DeviceAttestationCredentialsProvider to start returning with the getter
 */
void SetDeviceAttestationCredentialsProvider(DeviceAttestationCredentialsProvider * provider);
```
Note: In the reference application, a dedicated read-only persistent storage space (factory block) is used for this.

#### 5. Open commissioning window:
Now everything is initialized the last step is to open a commissioning window to start the Bluetooth LE advertisements.
This can be done with below function:

```
CHIP_ERROR CommissioningWindowManager::OpenBasicCommissioningWindow(Seconds16 commissioningTimeout,
                                                                    CommissioningWindowAdvertisement advertisementMode)
```

#### 6. Get link for QR code printed:
To get a visualization of the QR code to be able to commission the Matter device, below function can be used:

```
void PrintOnboardingCodes(chip::RendezvousInformationFlags aRendezvousFlags)
```

## Interface towards the Matter data model implementation

The clusters that are used in the application depend on the clusters that are selected by using the ZCL Advanced
Platform (ZAP) tool. Based on that, source and header files get generated when make builds the application. See
"Work/base_qpg6105_development/zap-generated/zap-generated" to see which files get generated. These generated files are
the interface towards the Matter data model.


### Callback functions
Callback functions from the Matter clusters are implemented in the generated file
"Work/base_qpg6105_development/zap-generated/zap-generated/app/callback-stub.cpp". These functions are defined as
*void \_\_attribute\_\_((weak))* which means these functions can be overloaded and implemented in your application. In the
Qorvo reference applications you can find some of these functions being implemented in
[ZclCallbacks.cpp](../../../Applications/Matter/light/src/ZclCallbacks.cpp). Following types of callback functions can be
useful to get implemented in your application:

#### 1. Cluster initialization callback:
These functions are called by the Matter cluster implementation after the cluster is initialized on a certain endpoint and the cluster is ready to be used. The functions to be used are defined below, where *cluster-name* needs to be filled in with the naming of cluster inititialization you want to target.


```
void __attribute__((weak)) emberAf<cluster-name>ClusterInitCallback(EndpointId endpoint)
{
    // To prevent warning
    (void) endpoint;
}

```

#### 2. Capture cluster attribute changes:
If you want to trigger application behavior based on the change of an attribute value, a generic callback is defined
where you can capture all the attribute changes and implement your custom application behavior. The callback definition
can be seen below:

```
void __attribute__((weak)) MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type,
                                                             uint16_t size, uint8_t * value)
{}
```

The *attributePath* argument contains the cluster identifier, endpoint and attribute identifier. So this can be used to
filter for a specific cluster attribute. Example logic to capture an attribute change of the OnOff attribute of the
OnOff cluster can be found below:

```
    if (clusterId == OnOff::Id && attributeId == OnOff::Attributes::OnOff::Id)
    {
        //Put the light On of Off based on the value that is given in the callback.
    }
```

### Attribute getter/setter functions
Sometimes it is needed that an application is able to update a cluster attribute due to an external trigger. An example
is a temperature sensor. If the temperature changes, the application needs to be able to write the corresponding
attribute. Therefore getter and setter functions are defined for each cluster in the class
*Clusters::\<cluster-name\>::Attributes::\<attribute-name\>*:

```
EmberAfStatus Get(chip::EndpointId endpoint, uint8_t * value)
EmberAfStatus Set(chip::EndpointId endpoint, uint8_t value)
```

An example for setting the OnOff attribute of the OnOff cluster:

```
    // Put the light ON
    EmberAfStatus status = Clusters::OnOff::Attributes::OnOff::Set(QPG_LIGHT_ENDPOINT_ID, 1);

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: updating on/off %x", status);
    }
```

### Other
There are a lot of clusters being implemented in the Matter data model, describing them all here would lead us to far. To get full visibility of the API of certain clusters, advise is to take a look at the header file of each cluster. Cluster implementations can be found [here](https://github.com/Qorvo/connectedhomeip/tree/v1.1.0.0_qorvo/src/app/clusters)

## Timers
The Matter stack also provides a timer API that can be used. To start a timer, the following function can be used from the class
*chip::DeviceLayer::SystemLayer*:

```
    /**
     * @brief
     *   This method starts a one-shot timer.
     *
     *   @note
     *       Only a single timer is allowed to be started with the same @a aComplete and @a aAppState
     *       arguments. If called with @a aComplete and @a aAppState identical to an existing timer,
     *       the currently-running timer will first be cancelled.
     *
     *   @param[in]  aDelay             Time before this timer fires.
     *   @param[in]  aComplete          A pointer to the function called when timer expires.
     *   @param[in]  aAppState          A pointer to the application state object used when timer expires.
     *
     *   @return CHIP_NO_ERROR On success.
     *   @return CHIP_ERROR_NO_MEMORY If a timer cannot be allocated.
     *   @return Other Value indicating timer failed to start.
     */
    virtual CHIP_ERROR StartTimer(Clock::Timeout aDelay, TimerCompleteCallback aComplete, void * aAppState) = 0;
```

For stopping a Timer, below function can be used from the class *chip::DeviceLayer::SystemLayer*:

```
    /**
     * @brief
     *   This method cancels a one-shot timer, started earlier through @p StartTimer().
     *
     *   @note
     *       The cancellation could fail silently in two different ways. If the timer specified by the combination of the callback
     *       function and application state object couldn't be found, cancellation could fail. If the timer has fired, then
     *       an event is queued and will be processed later.
     *
     *   WARNING: Timer handlers MUST assume that they may be hit even after CancelTimer due to cancelling an
     *            already fired timer that is queued in the event loop already.
     *
     *   @param[in]  aOnComplete   A pointer to the callback function used in calling @p StartTimer().
     *   @param[in]  aAppState     A pointer to the application state object used in calling @p StartTimer().
     *
     */
    virtual void CancelTimer(TimerCompleteCallback aOnComplete, void * aAppState) = 0;

```

Example implementation how to use the timers can be found in
[AppTask.cpp](../../../Applications/Matter/light/src/AppTask.cpp) or as seen in below reference implementation:

```
void AppTask::CancelTimer()
{
    chip::DeviceLayer::SystemLayer().CancelTimer(TimerEventHandler, this);
    mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    CHIP_ERROR err;

    chip::DeviceLayer::SystemLayer().CancelTimer(TimerEventHandler, this);
    err = chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(aTimeoutInMs), TimerEventHandler, this);
    SuccessOrExit(err);

    mFunctionTimerActive = true;
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "StartTimer failed %s: ", chip::ErrorStr(err));
    }
}
```
