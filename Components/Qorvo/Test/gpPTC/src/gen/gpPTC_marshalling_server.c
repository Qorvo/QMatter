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

/** @file "gpPTC_marshalling_server.c"
 *
 *  gpPTC
 *
 *  Marshalling structures and functions
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

gpMarshall_AckStatus_t gpPTC_SetClientIDRequest_Input_buf2api(gpPTC_SetClientIDRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex)
{
    UInt8_buf2api_1(&(pDest->data.clientID), pSource, pIndex);
    return gpMarshall_AckStatusSuccess;
}


gpMarshall_AckStatus_t gpPTC_Discover_Input_buf2api(gpPTC_Discover_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex)
{
    UInt8_buf2api_1(&(pDest->data.clientID), pSource, pIndex);
    {
        gpMarshall_AckStatus_t marshall_result = gpPTC_MACAddress_t_buf2api(&(pDest->data.senderMacAddress), pSource, 1, pIndex );
        if (gpMarshall_AckStatusSuccess != marshall_result) { return marshall_result; }
    }
    return gpMarshall_AckStatusSuccess;
}



void gpPTC_GetDUTApiVersion_Output_api2buf(UInt8Buffer* pDest , gpPTC_GetDUTApiVersion_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex)
{
    UInt32_api2buf_1(pDest, &(pSourceoutput->data.version), pIndex);
}

gpMarshall_AckStatus_t gpPTC_GetDUTInfoRequest_Input_buf2api(gpPTC_GetDUTInfoRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex)
{
    UInt8_buf2api_1(&(pDest->data.clientID), pSource, pIndex);
    return gpMarshall_AckStatusSuccess;
}

void gpPTC_GetDUTInfoRequest_Output_api2buf(UInt8Buffer* pDest , gpPTC_GetDUTInfoRequest_Output_marshall_struct_t* pSourceoutput , gpPTC_GetDUTInfoRequest_Input_marshall_struct_t* pSourceinput , UInt16* pIndex)
{
    gpPTC_Result_t_api2buf_1(pDest, &(pSourceoutput->data.result), pIndex);
    pDest[(*pIndex)++] = (pSourceoutput->data.version == NULL); // Add IsNull byte for output pointer
    if (pSourceoutput->data.version != NULL)
    {
        UInt8_api2buf_1(pDest, pSourceoutput->data.version,pIndex);
    }
    pDest[(*pIndex)++] = (pSourceoutput->data.macAddr == NULL); // Add IsNull byte for output pointer
    if (pSourceoutput->data.macAddr != NULL)
    {
        gpPTC_MACAddress_t_api2buf(pDest , pSourceoutput->data.macAddr , 1 , pIndex);
    }
    pDest[(*pIndex)++] = (pSourceoutput->data.bleAddr == NULL); // Add IsNull byte for output pointer
    if (pSourceoutput->data.bleAddr != NULL)
    {
        gpPTC_DeviceAddress_t_api2buf(pDest , pSourceoutput->data.bleAddr , 1 , pIndex);
    }
    pDest[(*pIndex)++] = (pSourceoutput->data.appVersion == NULL); // Add IsNull byte for output pointer
    if (pSourceoutput->data.appVersion != NULL)
    {
        gpPTC_swVersionNumber_t_api2buf(pDest , pSourceoutput->data.appVersion , 1 , pIndex);
    }
    pDest[(*pIndex)++] = (pSourceoutput->data.partNumber == NULL); // Add IsNull byte for output pointer
    if (pSourceoutput->data.partNumber != NULL)
    {
        gpPTC_partNumber_t_api2buf(pDest , pSourceoutput->data.partNumber , 1 , pIndex);
    }
    pDest[(*pIndex)++] = (pSourceoutput->data.productName == NULL); // Add IsNull byte for output pointer
    if (pSourceoutput->data.productName != NULL)
    {
        gpPTC_ProductName_t_api2buf(pDest , pSourceoutput->data.productName , 1 , pIndex);
    }
    pDest[(*pIndex)++] = (pSourceoutput->data.ptcVersion == NULL); // Add IsNull byte for output pointer
    if (pSourceoutput->data.ptcVersion != NULL)
    {
        gpPTC_swVersionNumber_t_api2buf(pDest , pSourceoutput->data.ptcVersion , 1 , pIndex);
    }
    pDest[(*pIndex)++] = (pSourceoutput->data.productID == NULL); // Add IsNull byte for output pointer
    if (pSourceoutput->data.productID != NULL)
    {
        gpPTC_ProductID_t_api2buf(pDest , pSourceoutput->data.productID , 1 , pIndex);
    }
}

gpMarshall_AckStatus_t gpPTC_SetAttributeRequest_Input_buf2api(gpPTC_SetAttributeRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex)
{
    UInt8_buf2api_1(&(pDest->data.clientID), pSource, pIndex);
    UInt8_buf2api_1(&(pDest->data.numberOfAttr), pSource, pIndex);
    if (pSource[(*pIndex)++] == 0x0)  // Check IsNull byte
    {
        pDest->data.attributes = pDest->attributes.data;
        {
            gpMarshall_AckStatus_t marshall_result = gpPTC_Attribute_t_buf2api(pDest->data.attributes , pSource , pDest->data.numberOfAttr, pIndex );
            if (gpMarshall_AckStatusSuccess != marshall_result) { return marshall_result; }
        }
    }
    else
    {
        pDest->data.attributes = NULL;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpPTC_SetAttributeRequest_Output_api2buf(UInt8Buffer* pDest , gpPTC_SetAttributeRequest_Output_marshall_struct_t* pSourceoutput , gpPTC_SetAttributeRequest_Input_marshall_struct_t* pSourceinput , UInt16* pIndex)
{
    gpPTC_Result_t_api2buf_1(pDest, &(pSourceoutput->data.result), pIndex);
}

gpMarshall_AckStatus_t gpPTC_GetAttributeRequest_Input_buf2api(gpPTC_GetAttributeRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex)
{
    UInt8_buf2api_1(&(pDest->data.clientID), pSource, pIndex);
    UInt8_buf2api_1(&(pDest->data.numberOfAttr), pSource, pIndex);
    if (pSource[(*pIndex)++] == 0x0)  // Check IsNull byte
    {
        pDest->data.attributes = pDest->attributes.data;
        {
            gpMarshall_AckStatus_t marshall_result = gpPTC_Attribute_t_buf2api(pDest->data.attributes , pSource , pDest->data.numberOfAttr, pIndex );
            if (gpMarshall_AckStatusSuccess != marshall_result) { return marshall_result; }
        }
    }
    else
    {
        pDest->data.attributes = NULL;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpPTC_GetAttributeRequest_Output_api2buf(UInt8Buffer* pDest , gpPTC_GetAttributeRequest_Output_marshall_struct_t* pSourceoutput , gpPTC_GetAttributeRequest_Input_marshall_struct_t* pSourceinput , UInt16* pIndex)
{
    gpPTC_Result_t_api2buf_1(pDest, &(pSourceoutput->data.result), pIndex);
    UInt8_api2buf_1(pDest, &(pSourceinput->data.numberOfAttr), pIndex);
    pDest[(*pIndex)++] = (pSourceinput->data.attributes == NULL); // Add IsNull byte for output pointer
    if (pSourceinput->data.attributes != NULL)
    {
        gpPTC_Attribute_t_api2buf(pDest , pSourceinput->data.attributes , pSourceinput->data.numberOfAttr , pIndex);
    }
}

gpMarshall_AckStatus_t gpPTC_SetModeRequest_Input_buf2api(gpPTC_SetModeRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex)
{
    UInt8_buf2api_1(&(pDest->data.clientID), pSource, pIndex);
    UInt8_buf2api_1(&(pDest->data.modeID), pSource, pIndex);
    UInt32_buf2api_1(&(pDest->data.exectime), pSource, pIndex);
    gpPTC_ModeExecution_t_buf2api_1(&(pDest->data.OnOff), pSource, pIndex);
    UInt8_buf2api_1(&(pDest->data.numberOfExtraParameters), pSource, pIndex);
    if (pSource[(*pIndex)++] == 0x0)  // Check IsNull byte
    {
        pDest->data.parameters = pDest->parameters.data;
        {
            gpMarshall_AckStatus_t marshall_result = gpPTC_Parameter_t_buf2api(pDest->data.parameters , pSource , pDest->data.numberOfExtraParameters, pIndex );
            if (gpMarshall_AckStatusSuccess != marshall_result) { return marshall_result; }
        }
    }
    else
    {
        pDest->data.parameters = NULL;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpPTC_SetModeRequest_Output_api2buf(UInt8Buffer* pDest , gpPTC_SetModeRequest_Output_marshall_struct_t* pSourceoutput , gpPTC_SetModeRequest_Input_marshall_struct_t* pSourceinput , UInt16* pIndex)
{
    gpPTC_Result_t_api2buf_1(pDest, &(pSourceoutput->data.result), pIndex);
    UInt8_api2buf_1(pDest, &(pSourceinput->data.numberOfExtraParameters), pIndex);
    pDest[(*pIndex)++] = (pSourceinput->data.parameters == NULL); // Add IsNull byte for output pointer
    if (pSourceinput->data.parameters != NULL)
    {
        gpPTC_Parameter_t_api2buf(pDest , pSourceinput->data.parameters , pSourceinput->data.numberOfExtraParameters , pIndex);
    }
}

gpMarshall_AckStatus_t gpPTC_SetByteDataForAttributeRequest_Input_buf2api(gpPTC_SetByteDataForAttributeRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex)
{
    UInt8_buf2api_1(&(pDest->data.clientID), pSource, pIndex);
    gpPTC_AttributeId_t_buf2api_1(&(pDest->data.attributeID), pSource, pIndex);
    UInt8_buf2api_1(&(pDest->data.dataLen), pSource, pIndex);
    if (pSource[(*pIndex)++] == 0x0)  // Check IsNull byte
    {
        pDest->data.pData = pDest->pData;
        UInt8_buf2api(pDest->data.pData, pSource, pDest->data.dataLen, pIndex );
    }
    else
    {
        pDest->data.pData = NULL;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpPTC_SetByteDataForAttributeRequest_Output_api2buf(UInt8Buffer* pDest , gpPTC_SetByteDataForAttributeRequest_Output_marshall_struct_t* pSourceoutput , gpPTC_SetByteDataForAttributeRequest_Input_marshall_struct_t* pSourceinput , UInt16* pIndex)
{
    gpPTC_Result_t_api2buf_1(pDest, &(pSourceoutput->data.result), pIndex);
}

gpMarshall_AckStatus_t gpPTC_GetModeRequest_Input_buf2api(gpPTC_GetModeRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex)
{
    UInt8_buf2api_1(&(pDest->data.clientID), pSource, pIndex);
    UInt8_buf2api_1(&(pDest->data.modeID), pSource, pIndex);
    return gpMarshall_AckStatusSuccess;
}

void gpPTC_GetModeRequest_Output_api2buf(UInt8Buffer* pDest , gpPTC_GetModeRequest_Output_marshall_struct_t* pSourceoutput , gpPTC_GetModeRequest_Input_marshall_struct_t* pSourceinput , UInt16* pIndex)
{
    gpPTC_ModeExecution_t_api2buf_1(pDest, &(pSourceoutput->data.OnOff), pIndex);
}

gpMarshall_AckStatus_t gpPTC_ExecuteCustomCommand_Input_buf2api(gpPTC_ExecuteCustomCommand_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex)
{
    UInt8_buf2api_1(&(pDest->data.clientID), pSource, pIndex);
    UInt8_buf2api_1(&(pDest->data.dataLenIn), pSource, pIndex);
    if (pSource[(*pIndex)++] == 0x0)  // Check IsNull byte
    {
        pDest->data.pDataIn = pDest->pDataIn;
        UInt8_buf2api(pDest->data.pDataIn, pSource, pDest->data.dataLenIn, pIndex );
    }
    else
    {
        pDest->data.pDataIn = NULL;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpPTC_ExecuteCustomCommand_Output_api2buf(UInt8Buffer* pDest , gpPTC_ExecuteCustomCommand_Output_marshall_struct_t* pSourceoutput , gpPTC_ExecuteCustomCommand_Input_marshall_struct_t* pSourceinput , UInt16* pIndex)
{
    gpPTC_Result_t_api2buf_1(pDest, &(pSourceoutput->data.result), pIndex);
    pDest[(*pIndex)++] = (pSourceoutput->data.dataLenOut == NULL); // Add IsNull byte for output pointer
    if (pSourceoutput->data.dataLenOut != NULL)
    {
        UInt8_api2buf_1(pDest, pSourceoutput->data.dataLenOut,pIndex);
    }
    pDest[(*pIndex)++] = (pSourceoutput->data.pDataOut == NULL); // Add IsNull byte for output pointer
    if (pSourceoutput->data.pDataOut != NULL)
    {
        UInt8_api2buf(pDest, pSourceoutput->data.pDataOut,*pSourceoutput->data.dataLenOut, pIndex);
    }
}


void gpPTC_DiscoverIndication_Input_par2api(UInt8Buffer* pDest , gpPTC_DiscoveryInfo_t* pDiscoveryInfo , UInt16* pIndex)
{
    pDest[(*pIndex)++] = (NULL == pDiscoveryInfo);
    if (NULL != pDiscoveryInfo)
    {
        gpPTC_DiscoveryInfo_t_api2buf(pDest , pDiscoveryInfo , 1 , pIndex);
    }
}


void gpPTC_RXPacketIndication_Input_par2api(UInt8Buffer* pDest , UInt8 datalength , UInt8* payload , UInt16* pIndex)
{
    UInt8_api2buf_1(pDest , &(datalength) , pIndex);
    pDest[(*pIndex)++] = (NULL == payload);
    if (NULL != payload)
    {
        UInt8_api2buf(pDest , payload , datalength , pIndex);
    }
}


void gpPTC_DataConfirm_Input_par2api(UInt8Buffer* pDest , UInt8 status , UInt16 packetsSentOK , UInt16 packetsSentError , UInt16* pIndex)
{
    UInt8_api2buf_1(pDest , &(status) , pIndex);
    UInt16_api2buf_1(pDest , &(packetsSentOK) , pIndex);
    UInt16_api2buf_1(pDest , &(packetsSentError) , pIndex);
}


void gpPTC_EDConfirm_Input_par2api(UInt8Buffer* pDest , UInt8 result , UInt8 finished , UInt16 EDValue , UInt16* pIndex)
{
    UInt8_api2buf_1(pDest , &(result) , pIndex);
    UInt8_api2buf_1(pDest , &(finished) , pIndex);
    UInt16_api2buf_1(pDest , &(EDValue) , pIndex);
}



