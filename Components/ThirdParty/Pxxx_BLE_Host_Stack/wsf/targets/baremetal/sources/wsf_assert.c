/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  Assert implementation.
 *
 *  Copyright (c) 2019-2020 Packetcraft, Inc.
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
#include "wsf_assert.h"

#include "wsf_trace.h"

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/*! \brief  Assertion callback. */
static void (*wsfAssertCback)(void) = NULL;

/*************************************************************************************************/
/*!
 *  \brief  Perform an assert action.
 *
 *  \param  pFile   Name of file originating assert.
 *  \param  line    Line number of assert statement.
 */
/*************************************************************************************************/
#if WSF_TOKEN_ENABLED == TRUE
void WsfAssert(uint16_t modId, uint16_t line)
#else
void WsfAssert(const char *pFile, uint16_t line)
#endif
{
  /* Possibly unused parameters */
  (void)line;
#if WSF_TOKEN_ENABLED == TRUE
  (void)modId;
  WSF_TRACE_ERR2("Assertion detected on %s:%u", modId, line);
#else
  (void)pFile;
  WSF_TRACE_ERR2("Assertion detected on %s:%u", pFile, line);
#endif

  if (wsfAssertCback)
  {
    wsfAssertCback();
  }
  else
  {
    volatile unsigned int forever = 1;
    while (forever);
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Register assert handler.
 *
 *  \param  cback   Callback called upon assert condition.
 */
/*************************************************************************************************/
void WsfAssertRegister(void (*cback)(void))
{
  wsfAssertCback = cback;
}
