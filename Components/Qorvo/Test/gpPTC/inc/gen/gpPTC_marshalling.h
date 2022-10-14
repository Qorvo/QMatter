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

#ifndef _GPPTC_MARSHALLING_H_
#define _GPPTC_MARSHALLING_H_

//DOCUMENTATION PTC: no @file required as all documented items are refered to a group

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include <global.h>
#include "gpPTC.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct {
    gpPTC_Attribute_t data[1];
} gpPTC_Attribute_t_l1_pointer_marshall_t;

typedef struct {
    gpPTC_Attribute_t data[8];
} gpPTC_Attribute_t_l8_pointer_marshall_t;


typedef struct {
    gpPTC_MACAddress_t data[1];
} gpPTC_MACAddress_t_l1_pointer_marshall_t;


typedef struct {
    gpPTC_DeviceAddress_t data[1];
} gpPTC_DeviceAddress_t_l1_pointer_marshall_t;


typedef struct {
    gpPTC_chipSerial_t data[1];
} gpPTC_chipSerial_t_l1_pointer_marshall_t;


typedef struct {
    gpPTC_serialNumber_t data[1];
} gpPTC_serialNumber_t_l1_pointer_marshall_t;


typedef struct {
    gpPTC_swVersionNumber_t data[1];
} gpPTC_swVersionNumber_t_l1_pointer_marshall_t;


typedef struct {
    gpPTC_partNumber_t data[1];
} gpPTC_partNumber_t_l1_pointer_marshall_t;


typedef struct {
    gpPTC_Parameter_t data[8];
} gpPTC_Parameter_t_l8_pointer_marshall_t;


typedef struct {
    gpPTC_DiscoveryInfo_t data[1];
} gpPTC_DiscoveryInfo_t_l1_pointer_marshall_t;


typedef struct {
    gpPTC_ProductName_t data[1];
} gpPTC_ProductName_t_l1_pointer_marshall_t;


typedef struct {
    gpPTC_ProductID_t data[1];
} gpPTC_ProductID_t_l1_pointer_marshall_t;



typedef struct {
    UInt8 clientID;
} gpPTC_SetClientIDRequest_Input_struct_t;

typedef struct {
    gpPTC_SetClientIDRequest_Input_struct_t data;
} gpPTC_SetClientIDRequest_Input_marshall_struct_t;


typedef struct {
    UInt8 clientID;
    gpPTC_MACAddress_t senderMacAddress;
} gpPTC_Discover_Input_struct_t;

typedef struct {
    gpPTC_Discover_Input_struct_t data;
    gpPTC_MACAddress_t_l1_pointer_marshall_t senderMacAddress;
} gpPTC_Discover_Input_marshall_struct_t;



typedef struct {
    UInt32 version;
} gpPTC_GetDUTApiVersion_Output_struct_t;

typedef struct {
    gpPTC_GetDUTApiVersion_Output_struct_t data;
} gpPTC_GetDUTApiVersion_Output_marshall_struct_t;


typedef struct {
    UInt8 clientID;
} gpPTC_GetDUTInfoRequest_Input_struct_t;

typedef struct {
    gpPTC_GetDUTInfoRequest_Input_struct_t data;
} gpPTC_GetDUTInfoRequest_Input_marshall_struct_t;

typedef struct {
    gpPTC_Result_t result;
    UInt8* version;
    gpPTC_MACAddress_t* macAddr;
    gpPTC_DeviceAddress_t* bleAddr;
    gpPTC_swVersionNumber_t* appVersion;
    gpPTC_partNumber_t* partNumber;
    gpPTC_ProductName_t* productName;
    gpPTC_swVersionNumber_t* ptcVersion;
    gpPTC_ProductID_t* productID;
} gpPTC_GetDUTInfoRequest_Output_struct_t;

typedef struct {
    gpPTC_GetDUTInfoRequest_Output_struct_t data;
    UInt8 version[1];
    gpPTC_MACAddress_t_l1_pointer_marshall_t macAddr;
    gpPTC_DeviceAddress_t_l1_pointer_marshall_t bleAddr;
    gpPTC_swVersionNumber_t_l1_pointer_marshall_t appVersion;
    gpPTC_partNumber_t_l1_pointer_marshall_t partNumber;
    gpPTC_ProductName_t_l1_pointer_marshall_t productName;
    gpPTC_swVersionNumber_t_l1_pointer_marshall_t ptcVersion;
    gpPTC_ProductID_t_l1_pointer_marshall_t productID;
} gpPTC_GetDUTInfoRequest_Output_marshall_struct_t;


typedef struct {
    UInt8 clientID;
    UInt8 numberOfAttr;
    gpPTC_Attribute_t* attributes;
} gpPTC_SetAttributeRequest_Input_struct_t;

typedef struct {
    gpPTC_SetAttributeRequest_Input_struct_t data;
    gpPTC_Attribute_t_l8_pointer_marshall_t attributes;
} gpPTC_SetAttributeRequest_Input_marshall_struct_t;

typedef struct {
    gpPTC_Result_t result;
} gpPTC_SetAttributeRequest_Output_struct_t;

typedef struct {
    gpPTC_SetAttributeRequest_Output_struct_t data;
} gpPTC_SetAttributeRequest_Output_marshall_struct_t;


typedef struct {
    UInt8 clientID;
    UInt8 numberOfAttr;
    gpPTC_Attribute_t* attributes;
} gpPTC_GetAttributeRequest_Input_struct_t;

typedef struct {
    gpPTC_GetAttributeRequest_Input_struct_t data;
    gpPTC_Attribute_t_l8_pointer_marshall_t attributes;
} gpPTC_GetAttributeRequest_Input_marshall_struct_t;

typedef struct {
    gpPTC_Result_t result;
    // numberOfAttr used from input structure because it is an inout parameter
    // attributes used from input structure because it is an inout parameter
} gpPTC_GetAttributeRequest_Output_struct_t;

typedef struct {
    gpPTC_GetAttributeRequest_Output_struct_t data;
    // attributes used from input structure because it is an inout parameter
} gpPTC_GetAttributeRequest_Output_marshall_struct_t;


typedef struct {
    UInt8 clientID;
    UInt8 modeID;
    UInt32 exectime;
    gpPTC_ModeExecution_t OnOff;
    UInt8 numberOfExtraParameters;
    gpPTC_Parameter_t* parameters;
} gpPTC_SetModeRequest_Input_struct_t;

typedef struct {
    gpPTC_SetModeRequest_Input_struct_t data;
    gpPTC_Parameter_t_l8_pointer_marshall_t parameters;
} gpPTC_SetModeRequest_Input_marshall_struct_t;

typedef struct {
    gpPTC_Result_t result;
    // numberOfExtraParameters used from input structure because it is an inout parameter
    // parameters used from input structure because it is an inout parameter
} gpPTC_SetModeRequest_Output_struct_t;

typedef struct {
    gpPTC_SetModeRequest_Output_struct_t data;
    // parameters used from input structure because it is an inout parameter
} gpPTC_SetModeRequest_Output_marshall_struct_t;


typedef struct {
    UInt8 clientID;
    gpPTC_AttributeId_t attributeID;
    UInt8 dataLen;
    UInt8* pData;
} gpPTC_SetByteDataForAttributeRequest_Input_struct_t;

typedef struct {
    gpPTC_SetByteDataForAttributeRequest_Input_struct_t data;
    UInt8 pData[100];
} gpPTC_SetByteDataForAttributeRequest_Input_marshall_struct_t;

typedef struct {
    gpPTC_Result_t result;
} gpPTC_SetByteDataForAttributeRequest_Output_struct_t;

typedef struct {
    gpPTC_SetByteDataForAttributeRequest_Output_struct_t data;
} gpPTC_SetByteDataForAttributeRequest_Output_marshall_struct_t;


typedef struct {
    UInt8 clientID;
    UInt8 modeID;
} gpPTC_GetModeRequest_Input_struct_t;

typedef struct {
    gpPTC_GetModeRequest_Input_struct_t data;
} gpPTC_GetModeRequest_Input_marshall_struct_t;

typedef struct {
    gpPTC_ModeExecution_t OnOff;
} gpPTC_GetModeRequest_Output_struct_t;

typedef struct {
    gpPTC_GetModeRequest_Output_struct_t data;
} gpPTC_GetModeRequest_Output_marshall_struct_t;


typedef struct {
    UInt8 clientID;
    UInt8 dataLenIn;
    UInt8* pDataIn;
} gpPTC_ExecuteCustomCommand_Input_struct_t;

typedef struct {
    gpPTC_ExecuteCustomCommand_Input_struct_t data;
    UInt8 pDataIn[100];
} gpPTC_ExecuteCustomCommand_Input_marshall_struct_t;

typedef struct {
    gpPTC_Result_t result;
    UInt8* dataLenOut;
    UInt8* pDataOut;
} gpPTC_ExecuteCustomCommand_Output_struct_t;

typedef struct {
    gpPTC_ExecuteCustomCommand_Output_struct_t data;
    UInt8 dataLenOut[1];
    UInt8 pDataOut[100];
} gpPTC_ExecuteCustomCommand_Output_marshall_struct_t;


typedef struct {
    gpPTC_DiscoveryInfo_t* pDiscoveryInfo;
} gpPTC_DiscoverIndication_Input_struct_t;

typedef struct {
    gpPTC_DiscoverIndication_Input_struct_t data;
    gpPTC_DiscoveryInfo_t_l1_pointer_marshall_t pDiscoveryInfo;
} gpPTC_DiscoverIndication_Input_marshall_struct_t;


typedef struct {
    UInt8 datalength;
    UInt8* payload;
} gpPTC_RXPacketIndication_Input_struct_t;

typedef struct {
    gpPTC_RXPacketIndication_Input_struct_t data;
    UInt8 payload[128];
} gpPTC_RXPacketIndication_Input_marshall_struct_t;


typedef struct {
    UInt8 status;
    UInt16 packetsSentOK;
    UInt16 packetsSentError;
} gpPTC_DataConfirm_Input_struct_t;

typedef struct {
    gpPTC_DataConfirm_Input_struct_t data;
} gpPTC_DataConfirm_Input_marshall_struct_t;


typedef struct {
    UInt8 result;
    UInt8 finished;
    UInt16 EDValue;
} gpPTC_EDConfirm_Input_struct_t;

typedef struct {
    gpPTC_EDConfirm_Input_struct_t data;
} gpPTC_EDConfirm_Input_marshall_struct_t;


typedef union {
    gpPTC_SetClientIDRequest_Input_marshall_struct_t gpPTC_SetClientIDRequest;
    gpPTC_Discover_Input_marshall_struct_t gpPTC_Discover;
    gpPTC_GetDUTInfoRequest_Input_marshall_struct_t gpPTC_GetDUTInfoRequest;
    gpPTC_SetAttributeRequest_Input_marshall_struct_t gpPTC_SetAttributeRequest;
    gpPTC_GetAttributeRequest_Input_marshall_struct_t gpPTC_GetAttributeRequest;
    gpPTC_SetModeRequest_Input_marshall_struct_t gpPTC_SetModeRequest;
    gpPTC_SetByteDataForAttributeRequest_Input_marshall_struct_t gpPTC_SetByteDataForAttributeRequest;
    gpPTC_GetModeRequest_Input_marshall_struct_t gpPTC_GetModeRequest;
    gpPTC_ExecuteCustomCommand_Input_marshall_struct_t gpPTC_ExecuteCustomCommand;
    UInt8 dummy; //ensure none empty union definition
} gpPTC_Server_Input_union_t;

typedef union {
    gpPTC_GetDUTApiVersion_Output_marshall_struct_t gpPTC_GetDUTApiVersion;
    gpPTC_GetDUTInfoRequest_Output_marshall_struct_t gpPTC_GetDUTInfoRequest;
    gpPTC_SetAttributeRequest_Output_marshall_struct_t gpPTC_SetAttributeRequest;
    gpPTC_GetAttributeRequest_Output_marshall_struct_t gpPTC_GetAttributeRequest;
    gpPTC_SetModeRequest_Output_marshall_struct_t gpPTC_SetModeRequest;
    gpPTC_SetByteDataForAttributeRequest_Output_marshall_struct_t gpPTC_SetByteDataForAttributeRequest;
    gpPTC_GetModeRequest_Output_marshall_struct_t gpPTC_GetModeRequest;
    gpPTC_ExecuteCustomCommand_Output_marshall_struct_t gpPTC_ExecuteCustomCommand;
    UInt8 dummy; //ensure none empty union definition
} gpPTC_Server_Output_union_t;

typedef union {
    gpPTC_DiscoverIndication_Input_marshall_struct_t gpPTC_DiscoverIndication;
    gpPTC_RXPacketIndication_Input_marshall_struct_t gpPTC_RXPacketIndication;
    gpPTC_DataConfirm_Input_marshall_struct_t gpPTC_DataConfirm;
    gpPTC_EDConfirm_Input_marshall_struct_t gpPTC_EDConfirm;
    UInt8 dummy; //ensure none empty union definition
} gpPTC_Client_Input_union_t;

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Alias/enum copy macro's
#define gpPTC_Result_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpPTC_Result_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpPTC_Result_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpPTC_Result_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpPTC_PhyMode_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpPTC_PhyMode_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpPTC_PhyMode_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpPTC_PhyMode_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpPTC_AttributeId_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpPTC_AttributeId_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpPTC_AttributeId_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpPTC_AttributeId_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpPTC_ModeId_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpPTC_ModeId_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpPTC_ModeId_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpPTC_ModeId_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpPTC_ModeExecution_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpPTC_ModeExecution_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpPTC_ModeExecution_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpPTC_ModeExecution_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpPTC_CSMAMode_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpPTC_CSMAMode_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpPTC_CSMAMode_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpPTC_CSMAMode_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpPTC_SleepModes_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpPTC_SleepModes_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpPTC_SleepModes_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpPTC_SleepModes_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpPTC_CWMode_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpPTC_CWMode_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpPTC_CWMode_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpPTC_CWMode_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpPTC_MultiStandard_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpPTC_MultiStandard_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpPTC_MultiStandard_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpPTC_MultiStandard_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpPTC_HighSensitivity_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpPTC_HighSensitivity_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpPTC_HighSensitivity_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpPTC_HighSensitivity_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpPTC_MultiChannel_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpPTC_MultiChannel_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpPTC_MultiChannel_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpPTC_MultiChannel_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)

// Structure copy functions
gpMarshall_AckStatus_t gpPTC_Attribute_t_buf2api(gpPTC_Attribute_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex );
void gpPTC_Attribute_t_api2buf(UInt8Buffer* pDest , const gpPTC_Attribute_t* pSource , UInt16 length , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_MACAddress_t_buf2api(gpPTC_MACAddress_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex );
void gpPTC_MACAddress_t_api2buf(UInt8Buffer* pDest , const gpPTC_MACAddress_t* pSource , UInt16 length , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_DeviceAddress_t_buf2api(gpPTC_DeviceAddress_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex );
void gpPTC_DeviceAddress_t_api2buf(UInt8Buffer* pDest , const gpPTC_DeviceAddress_t* pSource , UInt16 length , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_chipSerial_t_buf2api(gpPTC_chipSerial_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex );
void gpPTC_chipSerial_t_api2buf(UInt8Buffer* pDest , const gpPTC_chipSerial_t* pSource , UInt16 length , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_serialNumber_t_buf2api(gpPTC_serialNumber_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex );
void gpPTC_serialNumber_t_api2buf(UInt8Buffer* pDest , const gpPTC_serialNumber_t* pSource , UInt16 length , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_swVersionNumber_t_buf2api(gpPTC_swVersionNumber_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex );
void gpPTC_swVersionNumber_t_api2buf(UInt8Buffer* pDest , const gpPTC_swVersionNumber_t* pSource , UInt16 length , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_partNumber_t_buf2api(gpPTC_partNumber_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex );
void gpPTC_partNumber_t_api2buf(UInt8Buffer* pDest , const gpPTC_partNumber_t* pSource , UInt16 length , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_Parameter_t_buf2api(gpPTC_Parameter_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex );
void gpPTC_Parameter_t_api2buf(UInt8Buffer* pDest , const gpPTC_Parameter_t* pSource , UInt16 length , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_DiscoveryInfo_t_buf2api(gpPTC_DiscoveryInfo_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex );
void gpPTC_DiscoveryInfo_t_api2buf(UInt8Buffer* pDest , const gpPTC_DiscoveryInfo_t* pSource , UInt16 length , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_ProductName_t_buf2api(gpPTC_ProductName_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex );
void gpPTC_ProductName_t_api2buf(UInt8Buffer* pDest , const gpPTC_ProductName_t* pSource , UInt16 length , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_ProductID_t_buf2api(gpPTC_ProductID_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex );
void gpPTC_ProductID_t_api2buf(UInt8Buffer* pDest , const gpPTC_ProductID_t* pSource , UInt16 length , UInt16* pIndex);
// Server functions
gpMarshall_AckStatus_t gpPTC_SetClientIDRequest_Input_buf2api(gpPTC_SetClientIDRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_Discover_Input_buf2api(gpPTC_Discover_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPTC_GetDUTApiVersion_Output_api2buf(UInt8Buffer* pDest , gpPTC_GetDUTApiVersion_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_GetDUTInfoRequest_Input_buf2api(gpPTC_GetDUTInfoRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPTC_GetDUTInfoRequest_Output_api2buf(UInt8Buffer* pDest , gpPTC_GetDUTInfoRequest_Output_marshall_struct_t* pSourceoutput , gpPTC_GetDUTInfoRequest_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_SetAttributeRequest_Input_buf2api(gpPTC_SetAttributeRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPTC_SetAttributeRequest_Output_api2buf(UInt8Buffer* pDest , gpPTC_SetAttributeRequest_Output_marshall_struct_t* pSourceoutput , gpPTC_SetAttributeRequest_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_GetAttributeRequest_Input_buf2api(gpPTC_GetAttributeRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPTC_GetAttributeRequest_Output_api2buf(UInt8Buffer* pDest , gpPTC_GetAttributeRequest_Output_marshall_struct_t* pSourceoutput , gpPTC_GetAttributeRequest_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_SetModeRequest_Input_buf2api(gpPTC_SetModeRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPTC_SetModeRequest_Output_api2buf(UInt8Buffer* pDest , gpPTC_SetModeRequest_Output_marshall_struct_t* pSourceoutput , gpPTC_SetModeRequest_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_SetByteDataForAttributeRequest_Input_buf2api(gpPTC_SetByteDataForAttributeRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPTC_SetByteDataForAttributeRequest_Output_api2buf(UInt8Buffer* pDest , gpPTC_SetByteDataForAttributeRequest_Output_marshall_struct_t* pSourceoutput , gpPTC_SetByteDataForAttributeRequest_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_GetModeRequest_Input_buf2api(gpPTC_GetModeRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPTC_GetModeRequest_Output_api2buf(UInt8Buffer* pDest , gpPTC_GetModeRequest_Output_marshall_struct_t* pSourceoutput , gpPTC_GetModeRequest_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_ExecuteCustomCommand_Input_buf2api(gpPTC_ExecuteCustomCommand_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPTC_ExecuteCustomCommand_Output_api2buf(UInt8Buffer* pDest , gpPTC_ExecuteCustomCommand_Output_marshall_struct_t* pSourceoutput , gpPTC_ExecuteCustomCommand_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
void gpPTC_DiscoverIndication_Input_par2api(UInt8Buffer* pDest , gpPTC_DiscoveryInfo_t* pDiscoveryInfo , UInt16* pIndex);
void gpPTC_RXPacketIndication_Input_par2api(UInt8Buffer* pDest , UInt8 datalength , UInt8* payload , UInt16* pIndex);
void gpPTC_DataConfirm_Input_par2api(UInt8Buffer* pDest , UInt8 status , UInt16 packetsSentOK , UInt16 packetsSentError , UInt16* pIndex);
void gpPTC_EDConfirm_Input_par2api(UInt8Buffer* pDest , UInt8 result , UInt8 finished , UInt16 EDValue , UInt16* pIndex);

// Client functions
void gpPTC_SetClientIDRequest_Input_par2buf(UInt8Buffer* pDest , UInt8 clientID , UInt16* pIndex);
void gpPTC_Discover_Input_par2buf(UInt8Buffer* pDest , UInt8 clientID , gpPTC_MACAddress_t senderMacAddress , UInt16* pIndex);
void gpPTC_GetDUTApiVersion_Output_buf2par(UInt32* version , UInt8Buffer* pSource , UInt16* pIndex);
void gpPTC_GetDUTInfoRequest_Input_par2buf(UInt8Buffer* pDest , UInt8 clientID , UInt16* pIndex);
void gpPTC_GetDUTInfoRequest_Output_buf2par(gpPTC_Result_t* result , UInt8 clientID , UInt8* version , gpPTC_MACAddress_t* macAddr , gpPTC_DeviceAddress_t* bleAddr , gpPTC_swVersionNumber_t* appVersion , gpPTC_partNumber_t* partNumber , gpPTC_ProductName_t* productName , gpPTC_swVersionNumber_t* ptcVersion , gpPTC_ProductID_t* productID , UInt8Buffer* pSource , UInt16* pIndex);
void gpPTC_SetAttributeRequest_Input_par2buf(UInt8Buffer* pDest , UInt8 clientID , UInt8 numberOfAttr , gpPTC_Attribute_t* attributes , UInt16* pIndex);
void gpPTC_SetAttributeRequest_Output_buf2par(gpPTC_Result_t* result , UInt8 clientID , UInt8 numberOfAttr , gpPTC_Attribute_t* attributes , UInt8Buffer* pSource , UInt16* pIndex);
void gpPTC_GetAttributeRequest_Input_par2buf(UInt8Buffer* pDest , UInt8 clientID , UInt8 numberOfAttr , gpPTC_Attribute_t* attributes , UInt16* pIndex);
void gpPTC_GetAttributeRequest_Output_buf2par(gpPTC_Result_t* result , UInt8 clientID , UInt8* numberOfAttr , gpPTC_Attribute_t* attributes , UInt8Buffer* pSource , UInt16* pIndex);
void gpPTC_SetModeRequest_Input_par2buf(UInt8Buffer* pDest , UInt8 clientID , UInt8 modeID , UInt32 exectime , gpPTC_ModeExecution_t OnOff , UInt8 numberOfExtraParameters , gpPTC_Parameter_t* parameters , UInt16* pIndex);
void gpPTC_SetModeRequest_Output_buf2par(gpPTC_Result_t* result , UInt8 clientID , UInt8 modeID , UInt32 exectime , gpPTC_ModeExecution_t OnOff , UInt8* numberOfExtraParameters , gpPTC_Parameter_t* parameters , UInt8Buffer* pSource , UInt16* pIndex);
void gpPTC_SetByteDataForAttributeRequest_Input_par2buf(UInt8Buffer* pDest , UInt8 clientID , gpPTC_AttributeId_t attributeID , UInt8 dataLen , UInt8* pData , UInt16* pIndex);
void gpPTC_SetByteDataForAttributeRequest_Output_buf2par(gpPTC_Result_t* result , UInt8 clientID , gpPTC_AttributeId_t attributeID , UInt8 dataLen , UInt8* pData , UInt8Buffer* pSource , UInt16* pIndex);
void gpPTC_GetModeRequest_Input_par2buf(UInt8Buffer* pDest , UInt8 clientID , UInt8 modeID , UInt16* pIndex);
void gpPTC_GetModeRequest_Output_buf2par(gpPTC_ModeExecution_t* OnOff , UInt8 clientID , UInt8 modeID , UInt8Buffer* pSource , UInt16* pIndex);
void gpPTC_ExecuteCustomCommand_Input_par2buf(UInt8Buffer* pDest , UInt8 clientID , UInt8 dataLenIn , UInt8* pDataIn , UInt16* pIndex);
void gpPTC_ExecuteCustomCommand_Output_buf2par(gpPTC_Result_t* result , UInt8 clientID , UInt8 dataLenIn , UInt8* pDataIn , UInt8* dataLenOut , UInt8* pDataOut , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_DiscoverIndication_Input_buf2api(gpPTC_DiscoverIndication_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_RXPacketIndication_Input_buf2api(gpPTC_RXPacketIndication_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_DataConfirm_Input_buf2api(gpPTC_DataConfirm_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPTC_EDConfirm_Input_buf2api(gpPTC_EDConfirm_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);

void gpPTC_InitMarshalling(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // _GPPTC_MARSHALLING_H_


