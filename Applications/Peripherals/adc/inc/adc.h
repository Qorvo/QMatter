/*
 * Copyright (c) 2017-2021, Qorvo Inc
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

/**
 * @file adc.h
 *
 * Header to define the needed adc definitions
 *
 */

#ifndef _ADC_H_
#define _ADC_H_

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#if   \
    defined(GP_DIVERSITY_QPG6105DK_B01)
#define ADC_CHANNEL_LIVE    hal_AdcChannelANIO0

#else
// Using BSP definitions
#if defined(GP_BSP_ADC_CH_LIVE_ANIO)
#define ADC_CHANNEL_LIVE GP_BSP_ADC_CH_LIVE_ANIO
#endif
#if defined(GP_BSP_ADC_CH_MIN_HOLD_ANIO)
#define ADC_CHANNEL_MIN_HOLD GP_BSP_ADC_CH_MIN_HOLD_ANIO
#endif
#if defined(GP_BSP_ADC_CH_MAX_HOLD_ANIO)
#define ADC_CHANNEL_MAX_HOLD GP_BSP_ADC_CH_MAX_HOLD_ANIO
#endif

#endif

#endif //_ADC_H_
