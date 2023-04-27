/*************************************************************************************************/
/*!
 *  \file   wsf_cs.c
 *
 *  \brief  Software foundation OS main module.
 *
 *  Copyright (c) 2009-2019 Arm Ltd. All Rights Reserved.
 *
 *  Copyright (c) 2019-2021 Packetcraft, Inc.
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*
 * Copyright (c) 2021, Qorvo Inc
 *
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
 */
/*************************************************************************************************/

#include "wsf_types.h"
#include "wsf_cs.h"

#include "wsf_assert.h"
#include "pal_sys.h"

#if (WSF_CS_STATS == TRUE)
#include "pal_frc.h"
#endif

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/*! \brief  Critical section nesting level. */
uint8_t wsfCsNesting = 0;

#if (WSF_CS_STATS == TRUE)

/*! \brief      Critical section start time. */
static uint32_t wsfCsStatsStartTime = 0;

/*! \brief  Critical section duration watermark level. */
uint16_t wsfCsStatsWatermarkUsec = 0;

#endif

#if (WSF_CS_STATS == TRUE)

/*************************************************************************************************/
/*!
 *  \brief  Get critical section duration watermark level.
 *
 *  \return Critical section duration watermark level.
 */
/*************************************************************************************************/
uint32_t WsfCsStatsGetCsWaterMark(void)
{
  return wsfCsStatsWatermarkUsec;
}

/*************************************************************************************************/
/*!
 *  \brief  Mark the beginning of a CS.
 */
/*************************************************************************************************/
static void wsfCsStatsEnter(void)
{
  /* N.B. Code path must not use critical sections. */

  wsfCsStatsStartTime = PalFrcHFTimerGetCurrentTime();
}

/*************************************************************************************************/
/*!
 *  \brief  Record the CS watermark.
 */
/*************************************************************************************************/
static void wsfCsStatsExit(void)
{
  /* N.B. Code path must not use critical sections. */

  uint32_t exitTime = PalFrcHFTimerGetCurrentTime();
  uint32_t durUsec = PalFrcDeltaUs(exitTime, wsfCsStatsStartTime);
  if (durUsec > wsfCsStatsWatermarkUsec)
  {
    wsfCsStatsWatermarkUsec = durUsec;
  }
}

#endif

/*************************************************************************************************/
/*!
 *  \brief  Enter a critical section.
 */
/*************************************************************************************************/
void WsfCsEnter(void)
{
  if (wsfCsNesting == 0)
  {
    PalSysCsEnter();

#if (WSF_CS_STATS == TRUE)
    wsfCsStatsEnter();
#endif
  }
  wsfCsNesting++;
}

/*************************************************************************************************/
/*!
 *  \brief  Exit a critical section.
 */
/*************************************************************************************************/
void WsfCsExit(void)
{
  WSF_ASSERT(wsfCsNesting != 0);

  wsfCsNesting--;
  if (wsfCsNesting == 0)
  {
#if (WSF_CS_STATS == TRUE)
    wsfCsStatsExit();
#endif
    PalSysCsExit();
  }
}
