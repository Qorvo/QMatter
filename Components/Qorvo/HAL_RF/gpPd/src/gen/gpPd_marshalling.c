/*
 * Copyright (c) 2016, GreenPeak Technologies
 * Copyright (c) 2017-2018, Qorvo Inc
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
 */

/** @file "gpPd_marshalling.c"
 *
 *  Packet Descriptor interface
 *
 *   Marshalling structures and functions.
*/

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

// General includes
#include "gpPd.h"
#include "gpPd_marshalling.h"

#ifdef GP_DIVERSITY_LOG
#include "gpLog.h"
#endif

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID GP_COMPONENT_ID_PD

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

 /*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

 /*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

gpMarshall_AckStatus_t gpPd_Loh_t_buf2api(gpPd_Loh_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex , Bool storePdHandle )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        pDest->handle = gpPd_GetPd();
        if(gpPd_ResultValidHandle != gpPd_CheckPdValid(pDest->handle))
        {
            return gpMarshall_AckStatusExecutionFailed;/* Failure to allocate handle. */
        }
        gpPd_Length_t_buf2api_1(&(pDest->length), pSource, pIndex);
        // 0-length parameter offset
        pDest->offset = 0;
        gpPd_WriteByteStream(pDest->handle, pDest->offset, pDest->length, &(pSource[*pIndex]));
        *pIndex += pDest->length;
        if(storePdHandle && !gpPd_StorePdHandle(pDest->handle,pSource[*pIndex]))/* sticky client side handle p_PdLoh has to be aasciated with server side handle. */
        {
#ifdef GP_DIVERSITY_LOG
            GP_LOG_SYSTEM_PRINTF("handle assoc failed",0);
#endif
            /* Failure to associate client and server side handles causes freeing of server side handle... */
#ifdef GP_COMP_UNIT_TEST
            gpPd_FreeRealPd(pDest->handle);
#else
            gpPd_FreePd(pDest->handle);
#endif //GP_COMP_UNIT_TEST
            /* ...and causes server to indicate failure in acknowledge to client. */
            return gpMarshall_AckStatusExecutionFailed;
        }
        *pIndex += 1;
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpPd_Loh_t_api2buf(UInt8Buffer* pDest , const gpPd_Loh_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        gpPd_Length_t_api2buf_1(pDest , &(pSource->length), pIndex);
        // 0-length parameter offset
        gpPd_ReadByteStream(pSource->handle, pSource->offset, pSource->length, &(pDest[*pIndex]));
        *pIndex += pSource->length;
#ifdef GP_COMP_UNIT_TEST
        {
            gpPd_Handle_t uthandle = gpPd_GetUtPd(pSource->handle);
            gpPd_Handle_t_api2buf_1(pDest, &uthandle, pIndex);
        }
#else
        gpPd_Handle_t_api2buf_1(pDest, &pSource->handle, pIndex);
#endif //GP_COMP_UNIT_TEST
        pSource++;
    }
}


void gpPd_InitMarshalling(void)
{
/* <CodeGenerator Placeholder> gpPd_InitMarshalling */
    gpPd_InitPdHandleMapping();
/* </CodeGenerator Placeholder> gpPd_InitMarshalling */
}


