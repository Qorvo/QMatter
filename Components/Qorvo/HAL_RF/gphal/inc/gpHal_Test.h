/*
 * Copyright (c) 2017-2023, Qorvo Inc
 *
 * gpHal_Test.h
 *
 *  This file defines all special functions used during development process only.
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
 * $Header$
 * $Change$
 * $DateTime$
 *
 */

#ifndef _HAL_GP_TEST_H_
#define _HAL_GP_TEST_H_

/** @file gpHal_Test.h
 *  This file defines all test functions used during development process of a chip.
 *
 *  @brief Functions for measurement cycles during development.
*/

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"
#include "gp_global.h"
#include "gpHal_reg.h"
#include "gpHal_HW.h"


/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/


/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/



/*****************************************************************************
 *                    Public function prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Disables or enables (= restore default) AGC on a chip.
*/
GP_API void gpHal_EnableAgc(Bool);

#ifdef __cplusplus
}
#endif

#endif  /* _HAL_GP_TEST_H_ */

