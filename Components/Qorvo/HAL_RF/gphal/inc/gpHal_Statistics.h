/*
 * Copyright (c) 2009-2014, 2016, GreenPeak Technologies
 * Copyright (c) 2017-2018, Qorvo Inc
 *
 * gpHal_Statistics.h
 *
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
 * Alternatively, this software may be distributed under the terms of the
 * modified BSD License or the 3-clause BSD License as published by the Free
 * Software Foundation @ https://directory.fsf.org/wiki/License:BSD-3-Clause
 *
 * $Header: //depot/release/Embedded/Components/Qorvo/HAL_RF/v2.10.2.1/comps/gphal/inc/gpHal_Statistics.h#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

#ifndef _GP_HAL_STATISTICS_H_
#define _GP_HAL_STATISTICS_H_

/** @file gpHal_Statistics.h
 *  @brief Getters and Setters of gpHal.
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/* COEX signals data structure */
typedef struct cntPrio_s {
    UInt16 prio0;
    UInt16 prio1;
    UInt16 prio2;
    UInt16 prio3;
}  gpHal_StatisticsCntPrio_t;

typedef struct gpHal_StatisticsCoexCounter_s {
    gpHal_StatisticsCntPrio_t coexReq;
    gpHal_StatisticsCntPrio_t coexGrant;
} gpHal_StatisticsCoexCounter_t;

typedef struct gpHal_StatisticsMacCounter_s {
    UInt16 ccaFails;
    UInt16 txRetries;
    UInt16 failTxNoAck;
    UInt16 failTxChannelAccess;
    UInt16 successTx;
    UInt16 totalRx;
    UInt16 pbmOverflow;
} gpHal_StatisticsMacCounter_t;


/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* clear all debug counters */
void gpHal_StatisticsCountersClear(void);
/* get debug counters */
void gpHal_StatisticsCountersGet(gpHal_StatisticsMacCounter_t* pStatisticsMacCounters, gpHal_StatisticsCoexCounter_t* pStatisticsCoexCounters);

#ifdef __cplusplus
}
#endif

#endif //_GP_HAL_STATISTICS_H_
