/*
 * Copyright (c) 2017, Qorvo Inc
 *
 * This software is owned by Qorvo Inc
 * and protected under applicable copyright laws.
 * It is delivered under the terms of the license
 * and is intended and supplied for use solely and
 * exclusively with products manufactured by
 * Qorvo Inc.
 *
 *
 * THIS SOFTWARE IS PROVIDED IN AN "AS IS"
 * CONDITION. NO WARRANTIES, WHETHER EXPRESS,
 * IMPLIED OR STATUTORY, INCLUDING, BUT NOT
 * LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * QORVO INC. SHALL NOT, IN ANY
 * CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 * INCIDENTAL OR CONSEQUENTIAL DAMAGES,
 * FOR ANY REASON WHATSOEVER.
 *
 * $Header: //depot/release/Embedded/Applications/R005_PeripheralLib/v1.3.2.1/apps/adc/src/adc.c#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

 /** @file adc.c
 *
 * ADC example application.
 * This example shows reading ADC values of temperature monitor, battery monitor, ANIO_0, ANIO_1 and ANIO_2 channel.
 * ANIO0 and ANIO1 are connected to potmeter on DB09 development board.
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "hal.h"
#include "gpSched.h"
#include "gpHal.h"
#include "gpBaseComps.h"
#include "gpLog.h"

#include "adc.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_APP

#ifdef ADC_ANIO_WAKEUP_ENABLE

#define COMPARATOR_VOLTAGE_MAX_VALUE_MV    3000 //3v
#define COMPARATOR_VOLTAGE_MAX_STEP        64 //0 ~ 0x3F

#define COMPARATOR_THRESHOLD_VALUE         5

#define COMPARATOR_THRESHOLD_INTEGER_PART       ((COMPARATOR_VOLTAGE_MAX_VALUE_MV*COMPARATOR_THRESHOLD_VALUE/COMPARATOR_VOLTAGE_MAX_STEP)/1000)
#define COMPARATOR_THRESHOLD_FLOATING_PART      ((COMPARATOR_VOLTAGE_MAX_VALUE_MV*COMPARATOR_THRESHOLD_VALUE/COMPARATOR_VOLTAGE_MAX_STEP)%1000)

#endif



/** @brief Delay to start continuous adc measurements */
#define DELAY_ADC_MEASURE_US    5000000 //5s
#define DELAY_1S                1000000  //1s

/** @brief Define number of samples to read ANIO measurements */
#define NUMBER_OF_ANIO_MEASUREMENTS 5

/** @brief Define macro to log temperature measurement - 2 byte adc value - LSB 8 bits define fractional part, MSB 8 bits define whole number */
#define LOG_TEMPERATURE(x)  GP_LOG_SYSTEM_PRINTF("TEMPERATURE: %u.%03luC",0,HAL_ADC_TEMPERATURE_GET_INTEGER_PART(x), HAL_ADC_TEMPERATURE_GET_FLOATING_PART(x));

/** @brief Define macro to log voltage measurement - 2 byte adc value - MSB 2 bits define whole number, LSB 14 bits define fractional part of the voltage */
#define LOG_VOLTAGE(x, source)  GP_LOG_SYSTEM_PRINTF(#source " voltage : %u.%03luV",0,HAL_ADC_VOLTAGE_GET_INTEGER_PART(x), HAL_ADC_VOLTAGE_GET_FLOATING_PART(x));
#define LOG_VOLTAGE_ANIO(x, ch, source)  GP_LOG_SYSTEM_PRINTF(#source "%d voltage : %u.%03luV",0,ch, HAL_ADC_VOLTAGE_GET_INTEGER_PART(x), HAL_ADC_VOLTAGE_GET_FLOATING_PART(x));

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/
void Application_EnableCOMPEvInterrupt(void);
void Application_ContinuousADCMeasure(void);
/*****************************************************************************
 *                    Application functions
 *****************************************************************************/

#if defined(GP_DIVERSITY_GPHAL_K8E)
void enable_ints(void)
{
    halAdc_ToggleOutOfRangeInterrupts(true);
}

void callback(hal_AdcInterruptState_t state)
{
    static UInt8 lastInterruptState = 0xFF;

    if (state == hal_AdcBelowMinimum && state != lastInterruptState)
    {
        GP_LOG_SYSTEM_PRINTF("Value below minimum!",0);
    }
    else if (state == hal_AdcAboveMaximum && state != lastInterruptState)
    {
        GP_LOG_SYSTEM_PRINTF("Value above maximum!",0);
    }
    else
    {
        if (state != hal_AdcBelowMinimum && state != hal_AdcAboveMaximum)
        {
            GP_ASSERT_SYSTEM(false);
        }
    }

    lastInterruptState = state;

    halAdc_ToggleOutOfRangeInterrupts(false);

    gpSched_ScheduleEvent(1000000,enable_ints);

}
#endif

/** @brief Function to start Min/Max channel measurements */
void Application_StartMinMaxANIOChannel(void)
{
    GP_LOG_SYSTEM_PRINTF("== Resetting ANIO values", 0);

    /* To enable MIN_HOLD/MAX_HOLD, preset the buffer with a fixed value through GP_WB_WRITE_ADCIF_BUFFER_PRESET_VALUE().
    halADC sets preset value to 0 if MIN_HOLD is disabled,
    halADC sets preset value to 0x3FF if MIN_HOLD is enabled,
    set scalar gain - halADC sets scalar gain to 0.33 */

#ifdef ADC_CHANNEL_MIN_HOLD
    /* set MIN_HOLD, disable MAX_HOLD */
    hal_StartContinuousADCMeasurement(ADC_CHANNEL_MIN_HOLD, HAL_DISABLE_HOLD_MAX, HAL_ENABLE_HOLD_MIN, HAL_ENABLE_3V6);
#endif //ADC_CHANNEL_MIN_HOLD
#ifdef ADC_CHANNEL_MAX_HOLD
    /* set MAX_HOLD, disable MIN_HOLD */
    hal_StartContinuousADCMeasurement(ADC_CHANNEL_MAX_HOLD, HAL_ENABLE_HOLD_MAX, HAL_DISABLE_HOLD_MIN, HAL_ENABLE_3V6);
#endif //ADC_CHANNEL_MAX_HOLD
}

/** @brief Function to perform a single Battery and temperature measurement */
void Application_MeasureBatteryAndTemperature(void)
{
    UQ2_14 adcData;

    /* start battery voltage measurement */
    /* MAX_HOLD and MIN_HOLD are not set, update buffer with posted result - no check for max and min values in the result buffer
    set scalar gain - halADC sets scalar gain to 0.33  */
    hal_StartContinuousADCMeasurement(hal_AdcChannelBattery, HAL_DISABLE_HOLD_MAX, HAL_DISABLE_HOLD_MIN, HAL_ENABLE_3V6);

    /* get adc measurement on battery monitor channel */
    adcData = hal_GetContinuousADCMeasurement(hal_AdcChannelBattery);
    /* stop adc measurement on battery monitor channel */
    hal_StopContinuousADCMeasurement(hal_AdcChannelBattery);
    LOG_VOLTAGE(adcData, BAT);

    /* start temperature measurement */
    /* MAX_HOLD and MIN_HOLD are not set, update buffer with posted result - no check for max and min values in the result buffer
    Differential mode enabled on temperature channel, scalar gain set to max and hence HAL_DONT_CARE_3V6  */
    hal_StartContinuousADCMeasurement(hal_AdcChannelTemperature, HAL_DISABLE_HOLD_MAX, HAL_DISABLE_HOLD_MIN, HAL_DONT_CARE_3V6);

    /* get adc temperature data */
    adcData = hal_GetContinuousADCMeasurement(hal_AdcChannelTemperature);
    /* Stop adc measurement on temperature channel */
    hal_StopContinuousADCMeasurement(hal_AdcChannelTemperature);
    LOG_TEMPERATURE(adcData);
}


#if defined(GP_DIVERSITY_GPHAL_K8E)

void Application_EndOutOfRangeInterruptMeasurement(void)
{
    /* Stop last started Out Of Range Interrupt measurement on Live channel */
    hal_StopContinuousADCMeasurementWithOutOfRangeInterrupt(ADC_CHANNEL_LIVE);
    GP_LOG_SYSTEM_PRINTF("== End check Out Of Range Interrupts",0);

    /* Exit and reschedule function */
    gpSched_ScheduleEvent(DELAY_ADC_MEASURE_US, Application_ContinuousADCMeasure);
}
#endif
/** @brief Function to start continuous adc measurements and report adc data in temperature, ANIO0, ANIO1 and ANIO2 channel
 */
void Application_ContinuousADCMeasure(void)
{
    UQ2_14 adcData;
    static UInt8 anioMeasureCount=0;

    /* Go back to ANIO measrements */
    if (anioMeasureCount == NUMBER_OF_ANIO_MEASUREMENTS)
    {
        /* restart ANIO measurements for next read cycle */
        Application_StartMinMaxANIOChannel();

        /* reset ANIO measurement count */
        anioMeasureCount=0;
    }

    GP_LOG_SYSTEM_PRINTF("-- Measuring cycle %u --",0, anioMeasureCount);

#ifdef ADC_CHANNEL_MIN_HOLD
    /* get adc measurement on MIN channel */
    adcData = hal_GetContinuousADCMeasurement(ADC_CHANNEL_MIN_HOLD);
    LOG_VOLTAGE(adcData, MIN);
#endif //ADC_CHANNEL_MIN_HOLD
#ifdef ADC_CHANNEL_MAX_HOLD
    /* get adc measurement on MAX channel */
    adcData = hal_GetContinuousADCMeasurement(ADC_CHANNEL_MAX_HOLD);
    LOG_VOLTAGE(adcData, MAX);
#endif //ADC_CHANNEL_MAX_HOLD

    /* Increment ANIO measure count */
    anioMeasureCount++;

    /* start LIVE measurement */
    /* MAX_HOLD and MIN_HOLD are not set, update buffer with posted result - no check for max and min values in the result buffer
    set scalar gain - halADC sets scalar gain to 0.33  */
    hal_StartContinuousADCMeasurement(ADC_CHANNEL_LIVE, HAL_DISABLE_HOLD_MAX, HAL_DISABLE_HOLD_MIN, HAL_ENABLE_3V6);
    /* get adc measurement on LIVE channel */
    adcData = hal_GetContinuousADCMeasurement(ADC_CHANNEL_LIVE);
    /* stop adc measurement on LIVE channel */
    hal_StopContinuousADCMeasurement(ADC_CHANNEL_LIVE);
#ifdef ADC_ANIO_WAKEUP_ENABLE
    LOG_VOLTAGE_ANIO(adcData, ADC_CHANNEL_LIVE, ANIO);
#else
    LOG_VOLTAGE(adcData, LIVE);
#endif //ADC_ANIO_WAKEUP_ENABLE

#ifdef ADC_ANIO_WAKEUP_ENABLE
    if((HAL_ADC_VOLTAGE_GET_INTEGER_PART(adcData)  >= COMPARATOR_THRESHOLD_INTEGER_PART) &&
       (HAL_ADC_VOLTAGE_GET_FLOATING_PART(adcData) >= COMPARATOR_THRESHOLD_FLOATING_PART))
    {
        LOG_VOLTAGE_ANIO(adcData, ADC_CHANNEL_LIVE, --(wakeup)-- ANIO);

        anioMeasureCount=0;
    }
#endif //ADC_ANIO_WAKEUP_ENABLE


    /* if anioMeasureCount exceeds NO_OF_ANIO_MEASUREMENTS, read temperature monitor and battery voltage */
    if (anioMeasureCount == NUMBER_OF_ANIO_MEASUREMENTS)
    {
        /* stop ANIO measurement on min/max channels */
#ifdef ADC_CHANNEL_MIN_HOLD
        hal_StopContinuousADCMeasurement(ADC_CHANNEL_MIN_HOLD);
#endif //ADC_CHANNEL_MIN_HOLD
#ifdef ADC_CHANNEL_MAX_HOLD
        hal_StopContinuousADCMeasurement(ADC_CHANNEL_MAX_HOLD);
#endif //ADC_CHANNEL_MAX_HOLD

        /* Measure battery and temperature */
        Application_MeasureBatteryAndTemperature();
#if defined(GP_DIVERSITY_GPHAL_K8E)

        GP_LOG_SYSTEM_PRINTF("== Check Out Of Range Interrupts",0);
        hal_StartContinuousADCMeasurementWithOutOfRangeInterrupt(ADC_CHANNEL_LIVE, HAL_ADC_VOLTAGE_TO_Q2_14_FORMAT(1,0), HAL_ADC_VOLTAGE_TO_Q2_14_FORMAT(2,0), HAL_ENABLE_3V6, callback);

        gpSched_ScheduleEvent(DELAY_ADC_MEASURE_US, Application_EndOutOfRangeInterruptMeasurement);

        return;
#endif
    }


#ifdef ADC_ANIO_WAKEUP_ENABLE

    gpSched_UnscheduleEvent(Application_EnableCOMPEvInterrupt);
    /* schedule an event to start continuous adc measurement on all channels after delay */
    gpSched_ScheduleEvent(DELAY_1S, Application_EnableCOMPEvInterrupt);

#endif //ADC_ANIO_WAKEUP_ENABLE

    /* schedule an event to start continuous adc measurement on all channels after delay */
    gpSched_ScheduleEvent(DELAY_ADC_MEASURE_US, Application_ContinuousADCMeasure);
}

#ifdef ADC_ANIO_WAKEUP_ENABLE
void Application_EnableCOMPEvInterrupt(void)
{
    HAL_DISABLE_GLOBAL_INT();
    #if defined(GP_DIVERSITY_GPHAL_K8E)
    GP_WB_WRITE_INT_CTRL_MASK_ES_COMP_EVENT_INTERRUPTS(0x0F);
    #else
    GP_WB_WRITE_INT_CTRL_MASK_ES_COMP_EVENT_INTERRUPT(0x0F);
    #endif
    HAL_ENABLE_GLOBAL_INT();
}

void Application_cbExternalEvent(void)
{
    /* Awake from interrupt */
    /* Disable interrupt event handling*/
    HAL_DISABLE_GLOBAL_INT();
    GP_WB_WRITE_ES_COMP_EVENT_VALID(0x00);
    GP_WB_WRITE_ES_CLR_COMP_EVENT_INTERRUPTS(0x0F);
    HAL_ENABLE_GLOBAL_INT();

    /* Disable interrupt untill handled */
    #if defined(GP_DIVERSITY_GPHAL_K8E)
    GP_WB_WRITE_INT_CTRL_MASK_ES_COMP_EVENT_INTERRUPTS(0x00);
    #else
    GP_WB_WRITE_INT_CTRL_MASK_ES_COMP_EVENT_INTERRUPT(0x00);
    #endif
    //gpHal_EnableExternalEventCallbackInterrupt(false);

    gpSched_UnscheduleEvent(Application_ContinuousADCMeasure);
    /* Delay check for debouncing of button/signal */
    gpSched_ScheduleEvent(0, Application_ContinuousADCMeasure);

    /* Enable interrupt event handling*/
    HAL_DISABLE_GLOBAL_INT();
    GP_WB_WRITE_ES_COMP_EVENT_VALID(0x0F);
    HAL_ENABLE_GLOBAL_INT();
}
#endif //ADC_ANIO_WAKEUP_ENABLE
/*****************************************************************************
 *                    Application Init
 *****************************************************************************/

/** @brief Initialize application
*/
void Application_Init(void)
{
    /* Initialize stack */
    gpBaseComps_StackInit();

    /* Initialize adc */
    hal_InitADC();

#if defined(ADC_CHANNEL_LIVE)
    switch (ADC_CHANNEL_LIVE)
    {
        #if defined(GP_DIVERSITY_GPHAL_K8E)
        case hal_AdcChannelANIO0:
            hal_gpioModePD(21, true);
            hal_gpioModePP(gpios[21], false);
            break;
        case hal_AdcChannelANIO1:
            hal_gpioModePD(22, true);
            hal_gpioModePP(gpios[22], false);
            break;
#endif

        case hal_AdcChannelANIO2:
        #if defined(GP_DIVERSITY_GPHAL_K8E)
            hal_gpioModePD(17, true);
            hal_gpioModePP(gpios[17], false);
        #else
            #error which chip are we talking to?
        #endif

            break;
        case hal_AdcChannelANIO3:
        #if defined(GP_DIVERSITY_GPHAL_K8E)
            hal_gpioModePD(18, true);
            hal_gpioModePP(gpios[18], false);
        #else
            #error which chip are we talking to?
        #endif
            break;
        #if !defined(GP_DIVERSITY_GPHAL_K8E)
        case hal_AdcChannelANIO4:
            hal_gpioModePD(25, true);
            hal_gpioModePP(gpios[25], false);
            break;
        case hal_AdcChannelANIO5:
            hal_gpioModePD(26, true);
            hal_gpioModePP(gpios[26], false);
            break;
#endif
        default:
            break;
    }
#endif

#if defined(ADC_CHANNEL_MIN_HOLD)
    switch (ADC_CHANNEL_MIN_HOLD)
    {
        #if defined(GP_DIVERSITY_GPHAL_K8E)
        case hal_AdcChannelANIO0:
            hal_gpioModePD(21, true);
            hal_gpioModePP(gpios[21], false);
            break;
        case hal_AdcChannelANIO1:
            hal_gpioModePD(22, true);
            hal_gpioModePP(gpios[22], false);
            break;
#endif
        case hal_AdcChannelANIO2:
        #if defined(GP_DIVERSITY_GPHAL_K8E)
            hal_gpioModePD(17, true);
            hal_gpioModePP(gpios[17], false);
        #else
            #error which chip are we talking to?
        #endif

            break;
        case hal_AdcChannelANIO3:
        #if defined(GP_DIVERSITY_GPHAL_K8E)
            hal_gpioModePD(18, true);
            hal_gpioModePP(gpios[18], false);
        #else
            #error which chip are we talking to?
        #endif
            break;
        #if !defined(GP_DIVERSITY_GPHAL_K8E)
        case hal_AdcChannelANIO4:
            hal_gpioModePD(25, true);
            hal_gpioModePP(gpios[25], false);
            break;
        case hal_AdcChannelANIO5:
            hal_gpioModePD(26, true);
            hal_gpioModePP(gpios[26], false);
            break;
#endif
        default:
            break;

    }
#endif

#if defined(ADC_CHANNEL_MAX_HOLD)
    switch (ADC_CHANNEL_MAX_HOLD)
    {
        #if defined(GP_DIVERSITY_GPHAL_K8E)
        case hal_AdcChannelANIO0:
            hal_gpioModePD(21, true);
            hal_gpioModePP(gpios[21], false);
            break;
        case hal_AdcChannelANIO1:
            hal_gpioModePD(22, true);
            hal_gpioModePP(gpios[22], false);
            break;
#endif

        case hal_AdcChannelANIO2:
        #if defined(GP_DIVERSITY_GPHAL_K8E)
            hal_gpioModePD(17, true);
            hal_gpioModePP(gpios[17], false);
        #else
            #error which chip are we talking to?
        #endif

            break;
        case hal_AdcChannelANIO3:
        #if defined(GP_DIVERSITY_GPHAL_K8E)
            hal_gpioModePD(18, true);
            hal_gpioModePP(gpios[18], false);
        #else
            #error which chip are we talking to?
        #endif
            break;
        #if !defined(GP_DIVERSITY_GPHAL_K8E)
        case hal_AdcChannelANIO4:
            hal_gpioModePD(25, true);
            hal_gpioModePP(gpios[25], false);
            break;
        case hal_AdcChannelANIO5:
            hal_gpioModePD(26, true);
            hal_gpioModePP(gpios[26], false);
            break;
#endif
        default:
            break;

    }
#endif

    HAL_WAIT_MS(1000);
#if defined(GP_DIVERSITY_GPHAL_K8E)
#endif
    /* start ANIO measurement */
    Application_StartMinMaxANIOChannel();

    /* Start continuous adc measurement */
    Application_ContinuousADCMeasure();

#ifdef ADC_ANIO_WAKEUP_ENABLE
    /* Enabling ANIO channel for wakeup interrupt,
       The chip will wakeup when selected ANIO channel > threshold (COMPARATOR_THRESHOLD_VALUE) */
    HAL_DISABLE_GLOBAL_INT();
    GP_WB_WRITE_ES_COMP_EVENT_VALID(0x00);
    HAL_ENABLE_GLOBAL_INT();

    /* Configure the slot */
    GP_WB_WRITE_PMUD_LPCMP_CLK_DIV_FACTOR(0);                       // default 0 - 32kHz
    GP_WB_WRITE_PMUD_LPCMP_CHANNEL_SEL_SLOT_0(ADC_CHANNEL_LIVE);    // ANIO channel
    GP_WB_WRITE_PMUD_LPCMP_LVL_SLOT_0(COMPARATOR_THRESHOLD_VALUE);        // Trigger if value is above threshold
    GP_WB_WRITE_PMUD_LPCMP_COMPARE_VALUE_SLOT_0(1);                 // Enable the measurement on the slot
    GP_WB_WRITE_PMUD_LPCMP_COMPARE_ENABLE_SLOT_0(1);

    /* Init external event interrupt*/
    GP_WB_WRITE_ES_CLR_COMP_EVENT_INTERRUPTS(0x0F);
    GP_WB_WRITE_ES_COMP_EVENT_TYPE_TO_BE_EXECUTED(GP_WB_ENUM_EVENT_TYPE_DUMMY);
    gpHal_RegisterExternalEventCallback(Application_cbExternalEvent);
    #if defined(GP_DIVERSITY_GPHAL_K8E)
    GP_WB_WRITE_INT_CTRL_MASK_ES_COMP_EVENT_INTERRUPTS(0x0F);
    #else
    GP_WB_WRITE_INT_CTRL_MASK_ES_COMP_EVENT_INTERRUPT(0x0F);
    #endif
    GP_WB_WRITE_ES_COMP_EVENT_VALID(0x0F);

#endif // ADC_ANIO_WAKEUP_ENABLE

#ifdef GP_SCHED_DIVERSITY_SLEEP
    /* Enable sleep behavior */
    gpHal_SetSleepMode(gpHal_SleepMode32kHz);

    /* Enable sleep behavior */
    gpSched_SetGotoSleepEnable(true);
#endif // GP_SCHED_DIVERSITY_SLEEP
}
