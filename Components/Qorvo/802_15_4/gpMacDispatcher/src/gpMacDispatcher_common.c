/*
 * Copyright (c) 2017-2018, Qorvo Inc
 *
 * gpMacDispatcher_common.c
 *   Parts of MAC Dispatcher that are common between multi-stack and single-stack implementations.
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
#define GP_COMPONENT_ID GP_COMPONENT_ID_MACDISPATCHER
//#define GP_LOCAL_LOG

#include "gpMacCore.h"
#include "gpMacDispatcher.h"
#include "gpMacDispatcher_def.h"
#include "gpLog.h"
#include "gpAssert.h"

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Internal Non-Static Function Definitions
 *****************************************************************************/


/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#ifdef GP_MACCORE_DIVERSITY_SCAN_ED_ORIGINATOR
void gpMacDispatcher_SetMinInterferenceLevels(Int8* pInterferenceLevels)
{
    gpMacCore_SetMinInterferenceLevels(pInterferenceLevels) ;
}
#endif //GP_MACCORE_DIVERSITY_SCAN_ED_ORIGINATOR

