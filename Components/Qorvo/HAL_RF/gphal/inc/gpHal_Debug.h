/*
 * Copyright (c) 2009-2014, 2016, GreenPeak Technologies
 * Copyright (c) 2017-2018, Qorvo Inc
 *
 * gpHal_Debug.h
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
 * $Header: //depot/release/Embedded/Components/Qorvo/HAL_RF/v2.10.2.1/comps/gphal/inc/gpHal_Debug.h#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

#ifndef _GP_HAL_DEBUG_H_
#define _GP_HAL_DEBUG_H_

/** @file gpHal_Debug.h
 *  @brief Usefull stuff for debugging and logging.
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#ifndef GP_HAL_REGISTER_ACCESS_BUFFER_SIZE
#define GP_HAL_REGISTER_ACCESS_BUFFER_SIZE   50
#endif

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

#define gphal_RegisterAccessTypeNone            0
#define gphal_RegisterAccessTypeRead            1
#define gphal_RegisterAccessTypeWrite           2
#define gphal_RegisterAccessTypeReadStream      3
#define gphal_RegisterAccessTypeWriteStream     4
#define gphal_RegisterAccessTypeReadModifyWrite 5
typedef UInt8 gphal_RegisterAccessType_t;


typedef struct gphal_RegisterAccess {
    gphal_RegisterAccessType_t  type;
    UInt8                       dataOrLength;
    UInt16                      address;
    UInt8                       mask;
} gphal_RegisterAccess_t;


/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#if defined(GP_DIVERSITY_DEVELOPMENT)
/* helper function: write out a memory region*/
void gphal_DumpBufferAsHex(UInt8 * pData, UInt8 length);
/** write out full pbm entry*/
void gphal_DumpTxPbmEntry(UInt8 i);
/** write out basic info on all pbm entries*/
void gphal_DumpPbmStates(void);
/** write out basic info on all events */
void gphal_DumpEventsSummary(void);
/** write out complete event*/
void gphal_DumpEvent(UInt8 event);
/** write out interrupt tree */
void gphal_DumpIrqConfig(void);
/** write out info about external event*/
void gphal_DumpExtEvent(void);
#else // GP_DIVERSITY_DEVELOPMENT && !GP_DIVERSITY_GPHAL_COPROC
#define gphal_DumpBufferAsHex(a,b) do {} while (false)
#define gphal_DumpTxPbmEntry(i) do {} while (false)
#define gphal_DumpPbmStates() do {} while (false)
#define gphal_DumpEventsSummary() do {} while (false)
#define gphal_DumpEvent(event) do {} while (false)
#define gphal_DumIrqConfig() do {} while (false)
#define gphal_DumpExtEvent() do {} while (false)
#endif // GP_DIVERSITY_DEVELOPMENT && !GP_DIVERSITY_GPHAL_COPROC

#define gphal_StartLogRegisterAccesses() do {} while (false)
#define gphal_AddLogRegisterAccess(type, address, data,Length, mask, retval) do {} while (false)
#define gphal_StopLogRegisterAccesses() do {} while (false)

#ifdef __cplusplus
}
#endif

#endif //_GP_HAL_DEBUG_H_
