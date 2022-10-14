/*
 * Copyright (c) 2015-2016, GreenPeak Technologies
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

/** @file "gpEncryption_marshalling.c"
 *
 *  GPENCRYPTION
 *
 *   Marshalling structures and functions.
*/

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

// General includes
#include "gpEncryption.h"
#include "gpEncryption_marshalling.h"

#ifdef GP_DIVERSITY_LOG
#include "gpLog.h"
#endif

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID GP_COMPONENT_ID_ENCRYPTION

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

gpMarshall_AckStatus_t gpEncryption_CCMOptions_t_buf2api(gpEncryption_CCMOptions_t* pDest ,gpEncryption_CCMOptions_t_pointer_marshall_t* pDestPointers , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex , Bool storePdHandle )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        pDest->pdHandle = gpPd_GetPd();
        if(gpPd_ResultValidHandle != gpPd_CheckPdValid(pDest->pdHandle))
        {
            return gpMarshall_AckStatusExecutionFailed;/* Failure to allocate handle. */
        }
        if(storePdHandle && !gpPd_StorePdHandle(pDest->pdHandle,pSource[*pIndex]))/* sticky client side handle p_PdLoh has to be aasciated with server side handle. */
        {
#ifdef GP_DIVERSITY_LOG
            GP_LOG_SYSTEM_PRINTF("handle assoc failed",0);
#endif
            /* Failure to associate client and server side handles causes freeing of server side handle... */
#ifdef GP_COMP_UNIT_TEST
            gpPd_FreeRealPd(pDest->pdHandle);
#else
            gpPd_FreePd(pDest->pdHandle);
#endif //GP_COMP_UNIT_TEST
            /* ...and causes server to indicate failure in acknowledge to client. */
            return gpMarshall_AckStatusExecutionFailed;
        }
        *pIndex += 1;
        gpPd_Offset_t_buf2api_1(&(pDest->dataOffset), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->dataLength), pSource, pIndex);
        gpPd_Offset_t_buf2api_1(&(pDest->auxOffset), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->auxLength), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->micLength), pSource, pIndex);
        if (NULL != pDestPointers)  // No IsNull byte check
        {
            pDest->pKey = pDestPointers->pKey;
            UInt8_buf2api(pDest->pKey, pSource, 16, pIndex);
        }
        else
        {
            pDest->pKey = NULL;
        }
        if (NULL != pDestPointers)  // No IsNull byte check
        {
            pDest->pNonce = pDestPointers->pNonce;
            UInt8_buf2api(pDest->pNonce, pSource, 13, pIndex);
        }
        else
        {
            pDest->pNonce = NULL;
        }
        pDest++;
        if (NULL != pDestPointers) {
            pDestPointers++;
        }
    }
    return gpMarshall_AckStatusSuccess;
}

void gpEncryption_CCMOptions_t_api2buf(UInt8Buffer* pDest , const gpEncryption_CCMOptions_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
#ifdef GP_COMP_UNIT_TEST
        {
            gpPd_Handle_t uthandle = gpPd_GetUtPd(pSource->pdHandle);
            gpPd_Handle_t_api2buf_1(pDest, &uthandle, pIndex);
        }
#else
        gpPd_Handle_t_api2buf_1(pDest, &pSource->pdHandle, pIndex);
#endif //GP_COMP_UNIT_TEST
        gpPd_Offset_t_api2buf_1(pDest , &(pSource->dataOffset), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->dataLength), pIndex);
        gpPd_Offset_t_api2buf_1(pDest , &(pSource->auxOffset), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->auxLength), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->micLength), pIndex);
        // No IsNull byte check
        if (NULL != pSource->pKey)
        {
            UInt8_api2buf(pDest , pSource->pKey , 16, pIndex);
        }
        // No IsNull byte check
        if (NULL != pSource->pNonce)
        {
            UInt8_api2buf(pDest , pSource->pNonce , 13, pIndex);
        }
        pSource++;
    }
}

gpMarshall_AckStatus_t gpEncryption_AESOptions_t_buf2api(gpEncryption_AESOptions_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        gpEncryption_AESKeyLen_t_buf2api_1(&(pDest->keylen), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->options), pSource, pIndex);
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpEncryption_AESOptions_t_api2buf(UInt8Buffer* pDest , const gpEncryption_AESOptions_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        gpEncryption_AESKeyLen_t_api2buf_1(pDest , &(pSource->keylen), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->options), pIndex);
        pSource++;
    }
}


void gpEncryption_InitMarshalling(void)
{
}


