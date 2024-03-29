/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief      FRC interface file.
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
/*************************************************************************************************/

#ifndef PAL_FRC_H
#define PAL_FRC_H

#include "pal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \addtogroup PAL_FRC
 *  \{ */
 
/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! \brief      Completion callback. */
typedef void (*PalFrcCompCback_t)(void);

/*! \brief      Timer states. */
typedef enum
{
  PAL_TIMER_STATE_UNINIT = 0,
  PAL_TIMER_STATE_ERROR = 0,
  PAL_TIMER_STATE_READY,
  PAL_TIMER_STATE_BUSY
} PalTimerState_t;

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/* Initialization */
void PalFrcInit(void);

/* Control and Status */
uint32_t PalFrcMsToTicks(uint32_t deltaMs);
uint32_t PalFrcDeltaMs(uint32_t endTime, uint32_t startTime);
uint32_t PalFrcDeltaUs(uint32_t endTime, uint32_t startTime);

void PalFrcHFTimerSet(uint32_t expTimeUsec, PalFrcCompCback_t expCback);
void PalFrcHFTimerClear(void);
uint32_t PalFrcHFTimerNextExpiration(void);
uint32_t PalFrcHFTimerGetCurrentTime(void);
/*! \} */    /* PAL_FRC */

#ifdef __cplusplus
};
#endif

#endif
