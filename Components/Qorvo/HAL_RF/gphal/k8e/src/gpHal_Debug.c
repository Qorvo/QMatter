/*
 * Copyright (c) 2014-2016, GreenPeak Technologies
 * Copyright (c) 2017, Qorvo Inc
 *
 * gpHal_Debug.c
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
 * $Header: //depot/release/Embedded/Components/Qorvo/HAL_RF/v2.10.2.1/comps/gphal/k8e/src/gpHal_Debug.c#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpHal.h"
#include "gpHal_Debug.h"
#include "gpHal_DEFS.h"
#include "gpHal_ES.h"
#include "gpAssert.h"
#include "gpLog.h"


/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID GP_COMPONENT_ID_GPHAL

/*****************************************************************************
 *                   Functional Macro Definitions
 *****************************************************************************/

/** returns the pbm (utc) state. For it's meaning, see HW documentation*/
#define gpHal_GetPbmState(PBMentry) GP_WB_READ_PBM_FORMAT_Z_RETURN_CODE(GP_HAL_PBM_ENTRY2ADDR_OPT_BASE(PBMentry))

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#ifdef GP_DIVERSITY_DEVELOPMENT
void gphal_DumpBufferAsHex(UInt8 * pData, UInt8 length)
{
    gpLog_PrintBuffer(length, pData);
}

void gphal_DumpTxPbmEntry(UInt8 i)
{
    GP_ASSERT_SYSTEM(false);
}
#ifdef GP_COMP_GPHAL_MAC
void gphal_DumpPbmStates(void)
{
   UInt8 alloc = 0;
    int i;

    for (i=0;i<GPHAL_NUMBER_OF_PBMS_USED;i++)
    {
        if (GP_HAL_IS_PBM_ALLOCATED(i))
        {
            alloc |= BM(i);
        }
    }
    GP_LOG("Pbm a:%x", 2, (UInt16) alloc);
#define STATE(i) ((i<GPHAL_NUMBER_OF_PBMS_USED)?((UInt16)gpHal_GetPbmState(i)):0xFFFF)
    GP_LOG("Pbm st0-3: %i %i %i %i", 8, STATE(0), STATE(1), STATE(2), STATE(3));
    GP_LOG("Pbm st4-6: %i %i %i", 8, STATE(4), STATE(5), STATE(6));
#undef STATE
//#endif
//    GP_ASSERT_SYSTEM(false);
}
#endif //GP_COMP_GPHAL_MAC

void gpHal_DumpEventsSummary(void)
{
    GP_ASSERT_SYSTEM(false);
}
void gphal_DumpEvent(UInt8 eventNbr)
{
    GP_ASSERT_SYSTEM(false);
}
void gphal_DumpIrqConfig(void)
{
    GP_ASSERT_SYSTEM(false);
}


void gphal_DumpExtEvent(void)
{
    GP_ASSERT_SYSTEM(false);
}
void gphal_DumpIO(void)
{
    GP_ASSERT_SYSTEM(false);
}
#endif // GP_DIVERSITY_DEVELOPMENT
