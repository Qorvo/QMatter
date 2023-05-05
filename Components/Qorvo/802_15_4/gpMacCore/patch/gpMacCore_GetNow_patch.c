/*
 * Copyright (c) 2017, 2019, Qorvo Inc
 *
 * gpMacCore_IndTx_patched.c
 *   This file contains patches for the indirect transmission functionality from the MAC protocol.
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
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID GP_COMPONENT_ID_MACCORE
// General includes
#include "gpMacCore.h"
#include "gpJumpTables.h"
#include "hal.h"

/*****************************************************************************
 *                    NRT ROM patch fix version numbers
 *****************************************************************************/
#if defined(GP_DIVERSITY_GPHAL_K8E)
#define ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION 2
#endif

#if(GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)
UInt32 gpMacCore_GetCurrentTimeUs(void)
{
    UInt32 now;
    HAL_TIMER_GET_CURRENT_TIME_1US(now);
    return now;
}
#endif // (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)
