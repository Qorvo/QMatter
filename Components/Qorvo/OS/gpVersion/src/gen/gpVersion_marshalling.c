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

/** @file "gpVersion_marshalling.c"
 *
 *  GPVERSION
 *
 *   Marshalling structures and functions.
*/

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

// General includes
#include "gpVersion.h"
#include "gpVersion_marshalling.h"

#ifdef GP_DIVERSITY_LOG
#include "gpLog.h"
#endif

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID GP_COMPONENT_ID_VERSION

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

gpMarshall_AckStatus_t gpVersion_ReleaseInfo_t_buf2api(gpVersion_ReleaseInfo_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        gpVersion_VersionNumber_t_buf2api_1(&(pDest->major), pSource, pIndex);
        gpVersion_VersionNumber_t_buf2api_1(&(pDest->minor), pSource, pIndex);
        gpVersion_VersionNumber_t_buf2api_1(&(pDest->revision), pSource, pIndex);
        gpVersion_VersionNumber_t_buf2api_1(&(pDest->patch), pSource, pIndex);
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpVersion_ReleaseInfo_t_api2buf(UInt8Buffer* pDest , const gpVersion_ReleaseInfo_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        gpVersion_VersionNumber_t_api2buf_1(pDest , &(pSource->major), pIndex);
        gpVersion_VersionNumber_t_api2buf_1(pDest , &(pSource->minor), pIndex);
        gpVersion_VersionNumber_t_api2buf_1(pDest , &(pSource->revision), pIndex);
        gpVersion_VersionNumber_t_api2buf_1(pDest , &(pSource->patch), pIndex);
        pSource++;
    }
}

gpMarshall_AckStatus_t gpVersion_SoftwareInfo_t_buf2api(gpVersion_SoftwareInfo_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        gpVersion_ReleaseInfo_t_buf2api(&(pDest->version), pSource, 1, pIndex);
        UInt8_buf2api_1(&(pDest->ctrl), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->reserved), pSource, pIndex);
        UInt16_buf2api_1(&(pDest->number), pSource, pIndex);
        UInt32_buf2api_1(&(pDest->changeList), pSource, pIndex);
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpVersion_SoftwareInfo_t_api2buf(UInt8Buffer* pDest , const gpVersion_SoftwareInfo_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        gpVersion_ReleaseInfo_t_api2buf(pDest , &(pSource->version), 1, pIndex);
        UInt8_api2buf_1(pDest , &(pSource->ctrl), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->reserved), pIndex);
        UInt16_api2buf_1(pDest , &(pSource->number), pIndex);
        UInt32_api2buf_1(pDest , &(pSource->changeList), pIndex);
        pSource++;
    }
}


void gpVersion_InitMarshalling(void)
{
}


