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
    defined(GP_DIVERSITY_SMART_HOME_AND_LIGHTING_CB_QPG6105)
#define ADC_CHANNEL_LIVE    hal_AdcChannelANIO0

#else

#error "Board diversity not supported!"

#endif

#endif //_ADC_H_
