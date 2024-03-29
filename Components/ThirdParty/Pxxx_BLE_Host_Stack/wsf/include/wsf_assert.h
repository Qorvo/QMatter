/*************************************************************************************************/
/*!
 *  \file   wsf_assert.h
 *
 *  \brief  Assert macro.
 *
 *  Copyright (c) 2009-2018 Arm Ltd. All Rights Reserved.
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
#ifndef WSF_ASSERT_H
#define WSF_ASSERT_H

#include "wsf_trace.h"
#include "gpAssert.h"

#ifndef GP_COMPONENT_ID
#define GP_COMPONENT_ID GP_COMPONENT_ID_QORVOBLEHOST
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*! \addtogroup WSF_ASSERT_API
 *  \{ */

/**************************************************************************************************
  Function Prototypes
**************************************************************************************************/

#if WSF_TOKEN_ENABLED == TRUE
/*************************************************************************************************/
/*!
 *  \brief  Perform an assert action.
 *
 *  \param  modId   Name of file originating assert.
 *  \param  line    Line number of assert statement.
 */
/*************************************************************************************************/
void WsfAssert(uint16_t modId, uint16_t line);
#else
void WsfAssert(const char *pFile, uint16_t line);
#endif

/*************************************************************************************************/
/*!
 *  \brief  Get number of asserts.
 *
 *  \return Number of asserts.
 */
/*************************************************************************************************/
uint16_t WsfAssertNum(void);

/*************************************************************************************************/
/*!
 *  \brief  Enable assert trap.
 *
 *  \param  enaAssertTrap     TRUE to enable assert trap.
 */
/*************************************************************************************************/
void WsfAssertTrapEnable(bool_t enaAssertTrap);

/**************************************************************************************************
  Macros
**************************************************************************************************/

#ifndef WSF_ASSERT_ENABLED
/*! \brief  Enable assertion statements. */
#define WSF_ASSERT_ENABLED  FALSE
#endif

#ifndef WSF_TOKEN_ENABLED
/*! \brief  Enable tokenized tracing. */
#define WSF_TOKEN_ENABLED   FALSE
#endif

/*************************************************************************************************/
/*!
 *  \brief  Run-time assert macro.  The assert executes when the expression is FALSE.
 *
 *  \param  expr    Boolean expression to be tested.
 */
/*************************************************************************************************/
#if WSF_ASSERT_ENABLED == TRUE
#if WSF_TOKEN_ENABLED == TRUE
#define WSF_ASSERT(expr)      if (!(expr)) {WsfAssert(MODULE_ID, (uint16_t) __LINE__);}
#else
//#define WSF_ASSERT(expr)      if (!(expr)) {WsfAssert(__FILE__, (uint16_t) __LINE__);}
#define WSF_ASSERT(expr)      GP_ASSERT_SYSTEM(expr);
#endif
#else
#define WSF_ASSERT(expr)
#endif

/*************************************************************************************************/
/*!
 *  \brief  Compile-time assert macro.  This macro causes a compiler error when the
 *          expression is FALSE.  Note that this macro is generally used at file scope to
 *          test constant expressions.  Errors may result of it is used in executing code.
 *
 *  \param  expr    Boolean expression to be tested.
 */
/*************************************************************************************************/
#define WSF_CT_ASSERT(expr)     extern char wsf_ct_assert[(expr) ? 1 : -1]

/*************************************************************************************************/
/*!
 *  \brief  Register assert handler.
 *
 *  \param  cback   Callback called upon assert condition.
 *
 *  \return None
 */
/*************************************************************************************************/
void WsfAssertRegister(void (*cback)(void));

/*! \} */    /* WSF_ASSERT_API */

#ifdef __cplusplus
};
#endif

#endif /* WSF_ASSERT_H */
