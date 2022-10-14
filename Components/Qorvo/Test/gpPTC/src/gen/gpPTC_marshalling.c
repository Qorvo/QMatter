/*
 * Copyright (c) 2016, GreenPeak Technologies
 * Copyright (c) 2017, Qorvo Inc
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
 * $Header$
 * $Change$
 * $DateTime$
 */

/** @file "gpPTC_marshalling.c"
 *
 *  gpPTC
 *
 *   Marshalling structures and functions.
*/

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

// General includes
#include "gpPTC.h"
#include "gpPTC_marshalling.h"

#ifdef GP_DIVERSITY_LOG
#include "gpLog.h"
#endif

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID GP_COMPONENT_ID_PTC

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

gpMarshall_AckStatus_t gpPTC_Attribute_t_buf2api(gpPTC_Attribute_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        gpPTC_AttributeId_t_buf2api_1(&(pDest->id), pSource, pIndex);
        Int32_buf2api_1(&(pDest->value), pSource, pIndex);
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpPTC_Attribute_t_api2buf(UInt8Buffer* pDest , const gpPTC_Attribute_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        gpPTC_AttributeId_t_api2buf_1(pDest , &(pSource->id), pIndex);
        Int32_api2buf_1(pDest , &(pSource->value), pIndex);
        pSource++;
    }
}

gpMarshall_AckStatus_t gpPTC_MACAddress_t_buf2api(gpPTC_MACAddress_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt8_buf2api_1(&(pDest->byte0), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte1), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte2), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte3), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte4), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte5), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte6), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte7), pSource, pIndex);
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpPTC_MACAddress_t_api2buf(UInt8Buffer* pDest , const gpPTC_MACAddress_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt8_api2buf_1(pDest , &(pSource->byte0), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte1), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte2), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte3), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte4), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte5), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte6), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte7), pIndex);
        pSource++;
    }
}

gpMarshall_AckStatus_t gpPTC_DeviceAddress_t_buf2api(gpPTC_DeviceAddress_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt8_buf2api_1(&(pDest->byte0), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte1), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte2), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte3), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte4), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte5), pSource, pIndex);
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpPTC_DeviceAddress_t_api2buf(UInt8Buffer* pDest , const gpPTC_DeviceAddress_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt8_api2buf_1(pDest , &(pSource->byte0), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte1), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte2), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte3), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte4), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte5), pIndex);
        pSource++;
    }
}

gpMarshall_AckStatus_t gpPTC_chipSerial_t_buf2api(gpPTC_chipSerial_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt8_buf2api_1(&(pDest->byte0), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte1), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte2), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte3), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte4), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte5), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte6), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte7), pSource, pIndex);
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpPTC_chipSerial_t_api2buf(UInt8Buffer* pDest , const gpPTC_chipSerial_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt8_api2buf_1(pDest , &(pSource->byte0), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte1), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte2), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte3), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte4), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte5), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte6), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte7), pIndex);
        pSource++;
    }
}

gpMarshall_AckStatus_t gpPTC_serialNumber_t_buf2api(gpPTC_serialNumber_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt8_buf2api_1(&(pDest->location), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->year0), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->year1), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->week0), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->week1), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->batch0), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->batch1), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->index0), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->index1), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->index2), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->index3), pSource, pIndex);
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpPTC_serialNumber_t_api2buf(UInt8Buffer* pDest , const gpPTC_serialNumber_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt8_api2buf_1(pDest , &(pSource->location), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->year0), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->year1), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->week0), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->week1), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->batch0), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->batch1), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->index0), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->index1), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->index2), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->index3), pIndex);
        pSource++;
    }
}

gpMarshall_AckStatus_t gpPTC_swVersionNumber_t_buf2api(gpPTC_swVersionNumber_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt8_buf2api_1(&(pDest->major), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->minor), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->revision), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->patch), pSource, pIndex);
        UInt32_buf2api_1(&(pDest->changelist), pSource, pIndex);
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpPTC_swVersionNumber_t_api2buf(UInt8Buffer* pDest , const gpPTC_swVersionNumber_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt8_api2buf_1(pDest , &(pSource->major), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->minor), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->revision), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->patch), pIndex);
        UInt32_api2buf_1(pDest , &(pSource->changelist), pIndex);
        pSource++;
    }
}

gpMarshall_AckStatus_t gpPTC_partNumber_t_buf2api(gpPTC_partNumber_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt8_buf2api_1(&(pDest->header0), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->header1), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte0), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte1), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte2), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte3), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->byte4), pSource, pIndex);
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpPTC_partNumber_t_api2buf(UInt8Buffer* pDest , const gpPTC_partNumber_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt8_api2buf_1(pDest , &(pSource->header0), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->header1), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte0), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte1), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte2), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte3), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->byte4), pIndex);
        pSource++;
    }
}

gpMarshall_AckStatus_t gpPTC_Parameter_t_buf2api(gpPTC_Parameter_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        Int32_buf2api_1(&(pDest->value), pSource, pIndex);
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpPTC_Parameter_t_api2buf(UInt8Buffer* pDest , const gpPTC_Parameter_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        Int32_api2buf_1(pDest , &(pSource->value), pIndex);
        pSource++;
    }
}

gpMarshall_AckStatus_t gpPTC_DiscoveryInfo_t_buf2api(gpPTC_DiscoveryInfo_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        gpPTC_MACAddress_t_buf2api(&(pDest->senderMacAddress), pSource, 1, pIndex);
        gpPTC_MACAddress_t_buf2api(&(pDest->DUTMacAddress), pSource, 1, pIndex);
        Int16_buf2api_1(&(pDest->RSSI), pSource, pIndex);
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpPTC_DiscoveryInfo_t_api2buf(UInt8Buffer* pDest , const gpPTC_DiscoveryInfo_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        gpPTC_MACAddress_t_api2buf(pDest , &(pSource->senderMacAddress), 1, pIndex);
        gpPTC_MACAddress_t_api2buf(pDest , &(pSource->DUTMacAddress), 1, pIndex);
        Int16_api2buf_1(pDest , &(pSource->RSSI), pIndex);
        pSource++;
    }
}

gpMarshall_AckStatus_t gpPTC_ProductName_t_buf2api(gpPTC_ProductName_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt8_buf2api((pDest->name), pSource, 40, pIndex);
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpPTC_ProductName_t_api2buf(UInt8Buffer* pDest , const gpPTC_ProductName_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt8_api2buf(pDest , (pSource->name), 40, pIndex);
        pSource++;
    }
}

gpMarshall_AckStatus_t gpPTC_ProductID_t_buf2api(gpPTC_ProductID_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt8_buf2api((pDest->name), pSource, 10, pIndex);
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpPTC_ProductID_t_api2buf(UInt8Buffer* pDest , const gpPTC_ProductID_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt8_api2buf(pDest , (pSource->name), 10, pIndex);
        pSource++;
    }
}


void gpPTC_InitMarshalling(void)
{
}


