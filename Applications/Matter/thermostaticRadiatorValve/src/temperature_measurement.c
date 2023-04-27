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
 * $Header$
 * $Change$
 * $DateTime$
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

#include "gpBaseComps.h"
#include "gpHal.h"
#include "gpLog.h"
#include "gpSched.h"
#include "hal.h"

#include "temperature_measurement.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_APP

/** @brief Define macro to log temperature measurement - 2 byte adc value - LSB 8 bits define fractional part, MSB 8 bits define
 * whole number */
#define LOG_TEMPERATURE(x)                                                                                                         \
    GP_LOG_SYSTEM_PRINTF("TEMPERATURE: %u.%03luC", 0, HAL_ADC_TEMPERATURE_GET_INTEGER_PART(x),                                     \
                         HAL_ADC_TEMPERATURE_GET_FLOATING_PART(x));

UQ2_14 temperatureAdcData;
/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Application functions
 *****************************************************************************/

/** @brief Function to perform a single temperature measurement */
void temperatureMeasurement_MeasureTemperature(void)
{
    UQ2_14 adcData;

    /* start temperature measurement */
    /* MAX_HOLD and MIN_HOLD are not set, update buffer with posted result - no check for max and min values in the result buffer
    Differential mode enabled on temperature channel, scalar gain set to max and hence HAL_DONT_CARE_3V6  */
    hal_StartContinuousADCMeasurement(hal_AdcChannelTemperature, HAL_DISABLE_HOLD_MAX, HAL_DISABLE_HOLD_MIN, HAL_DONT_CARE_3V6);

    /* get adc temperature data */
    adcData = hal_GetContinuousADCMeasurement(hal_AdcChannelTemperature);
    /* save adc temperature data*/
    temperatureAdcData = adcData;
    /* Stop adc measurement on temperature channel */
    hal_StopContinuousADCMeasurement(hal_AdcChannelTemperature);
    LOG_TEMPERATURE(adcData);
}

/*****************************************************************************
 *                    Application Init
 *****************************************************************************/

static const UInt8 adc_ch_to_pin[] = GP_BSP_ADC_GPIO_MAP;

/** @brief Initialize application
 */
void temperatureMeasurement_Init(void)
{
    // init the temperature adc data
    temperatureAdcData = 0;

    /* Initialize adc */
    hal_InitADC();

    hal_gpioModePD(adc_ch_to_pin[ADC_CHANNEL_LIVE], true);
    hal_gpioModePP(gpios[adc_ch_to_pin[ADC_CHANNEL_LIVE]], false);
}

void temperatureMeasurement_GetTemperatureValue(int * pTemp)
{
    int temp_integerPart;
    int tmep_floatingPart;

    temperatureMeasurement_MeasureTemperature();

    if (pTemp != NULL)
    {
        temp_integerPart  = (int) HAL_ADC_TEMPERATURE_GET_INTEGER_PART(temperatureAdcData);
        tmep_floatingPart = (int) HAL_ADC_TEMPERATURE_GET_FLOATING_PART(temperatureAdcData); // in the unit of 0.001

        *pTemp = (int) (temp_integerPart * 100 + tmep_floatingPart / 10);
    }
}
