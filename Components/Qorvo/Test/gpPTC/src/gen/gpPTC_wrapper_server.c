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

/** @file "gpPTC_wrapper_server.c"
 *
 *  gpPTC
 *
 *  Wrapper implementation
*/

/*****************************************************************************
 *                    Includes Definition
 *****************************************************************************/

#include "hal.h"
#include "gpUtils.h"
#include "gpLog.h"
#include "gpAssert.h"
#include "gpSched.h"
#include "gpCom.h"
#include "gpModule.h"
#include "gpPTC_clientServerCmdId.h"
#include "gpPTC.h"
#include "gpPTC_server.h"
#include "gpPTC_marshalling.h"

/*****************************************************************************
 *                    Typedef Definition
 *****************************************************************************/

/*****************************************************************************
 *                    Static Functions Declaration
 *****************************************************************************/

static void gpPTC_HandleCommandServer(UInt16 length, UInt8* pPayload, gpCom_CommunicationId_t communicationId);
static void PTC_HandleConnectionClose(gpCom_CommunicationId_t communicationId);
#ifndef GP_PTC_DIVERSITY_NO_LOCK
static UInt8 gpPTC_LockCheck(UInt16 length, UInt8* pPayload, gpCom_CommunicationId_t communicationId);
#endif //GP_PTC_DIVERSITY_NO_LOCK

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_PTC
#define GP_MODULE_ID GP_COMPONENT_ID

#ifndef GP_PTC_COMM_ID
#define GP_PTC_COMM_ID GP_COM_DEFAULT_COMMUNICATION_ID
#endif

#define REGISTER_MODULE(handle)        GP_COM_REGISTER_MODULE(handle)
#define DATA_REQUEST(len,buf,commId)   GP_COM_DATA_REQUEST(len,buf,commId)

/*****************************************************************************
 *                    Static Data
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

static void PTC_HandleConnectionClose(gpCom_CommunicationId_t communicationId)
{
}

#ifndef GP_PTC_DIVERSITY_NO_LOCK
static UInt8 gpPTC_LockCheck(UInt16 length, UInt8* pPayload, gpCom_CommunicationId_t communicationId)
{
    if (GP_UTILS_LOCK_CHECK_CLAIMED())
    {
        return gpMarshall_AckStatusBusy;
    }
    return gpMarshall_AckStatusSuccess;
}
#endif //GP_PTC_DIVERSITY_NO_LOCK



static void gpPTC_HandleCommandServer(UInt16 length, UInt8* pPayload, gpCom_CommunicationId_t communicationId)
{
#define commandId pPayload[0]
    gpMarshall_AckStatus_t marshall_result;
    UInt16 _index = 1;
    gpPTC_Server_Input_union_t input;
    gpPTC_Server_Output_union_t output;
    UInt8 ackBuffer[1 + 1 + 1+ max((4) * 1,max(1+1+1+(1 + 1 + 1 + 1 + 1 + 1 + 1 + 1)+1+(1 + 1 + 1 + 1 + 1 + 1)+1+(1 + 1 + 1 + 1 + 4*1)+1+(1 + 1 + 1 + 1 + 1 + 1 + 1)+1+(40)+1+(1 + 1 + 1 + 1 + 4*1)+1+(10)+1,max(1,max(1+1+8*(1 + 4*1)+1,max(1+1+8*(4*1)+1,1+1+1+100+1)))))];
    UInt16 ackBytes = 3;

    if(pPayload == NULL)
    {
        PTC_HandleConnectionClose(communicationId);
        return;
    }


    ackBuffer[0] = gpPTC_Acknowledge_CmdId;
    ackBuffer[1] = gpMarshall_AckStatusSuccess;
    ackBuffer[2] = commandId;

#ifndef GP_PTC_DIVERSITY_NO_LOCK
    if(gpPTC_Acknowledge_CmdId != commandId)
    {
        //Possibility to define extra check
        UInt8 status = gpPTC_LockCheck(length, pPayload, communicationId);
        if (status != gpMarshall_AckStatusSuccess)
        {
            ackBuffer[1] = status;
            goto gpPTC_HandleCommandServer_end;
        }
    }
#endif //GP_PTC_DIVERSITY_NO_LOCK

    switch(commandId)
    {
        case gpPTC_SetClientIDRequest_CmdId:
        {
            if(length != 2)
            {
                ackBuffer[1] = gpMarshall_AckStatusWrongParameterLength;
                break;
            }

            marshall_result = gpPTC_SetClientIDRequest_Input_buf2api(&(input.gpPTC_SetClientIDRequest) , pPayload , &_index);
            if (gpMarshall_AckStatusSuccess != marshall_result)
            {
                ackBuffer[1] = marshall_result;
                break;
            }

            gpPTC_SetClientIDRequest(
                  input.gpPTC_SetClientIDRequest.data.clientID
                );

            break;
        }
        case gpPTC_Discover_CmdId:
        {
            if(length != 10)
            {
                ackBuffer[1] = gpMarshall_AckStatusWrongParameterLength;
                break;
            }

            marshall_result = gpPTC_Discover_Input_buf2api(&(input.gpPTC_Discover) , pPayload , &_index);
            if (gpMarshall_AckStatusSuccess != marshall_result)
            {
                ackBuffer[1] = marshall_result;
                break;
            }

            gpPTC_Discover(
                  input.gpPTC_Discover.data.clientID
                , input.gpPTC_Discover.data.senderMacAddress
                );

            break;
        }
        case gpPTC_ResetDUT_CmdId:
        {
            if(length != 1)
            {
                ackBuffer[1] = gpMarshall_AckStatusWrongParameterLength;
                break;
            }


            gpPTC_ResetDUT(
                );

            break;
        }
        case gpPTC_GetDUTApiVersion_CmdId:
        {
            if(length != 1)
            {
                ackBuffer[1] = gpMarshall_AckStatusWrongParameterLength;
                break;
            }


            output.gpPTC_GetDUTApiVersion.data.version = gpPTC_GetDUTApiVersion(
                );
            gpPTC_GetDUTApiVersion_Output_api2buf(ackBuffer , &(output.gpPTC_GetDUTApiVersion) , &ackBytes);

            break;
        }
        case gpPTC_GetDUTInfoRequest_CmdId:
        {
            if(length != 2)
            {
                ackBuffer[1] = gpMarshall_AckStatusWrongParameterLength;
                break;
            }

            marshall_result = gpPTC_GetDUTInfoRequest_Input_buf2api(&(input.gpPTC_GetDUTInfoRequest) , pPayload , &_index);
            if (gpMarshall_AckStatusSuccess != marshall_result)
            {
                ackBuffer[1] = marshall_result;
                break;
            }
            // Initialize output pointers
            output.gpPTC_GetDUTInfoRequest.data.version = output.gpPTC_GetDUTInfoRequest.version;
            output.gpPTC_GetDUTInfoRequest.data.macAddr = output.gpPTC_GetDUTInfoRequest.macAddr.data;
            output.gpPTC_GetDUTInfoRequest.data.bleAddr = output.gpPTC_GetDUTInfoRequest.bleAddr.data;
            output.gpPTC_GetDUTInfoRequest.data.appVersion = output.gpPTC_GetDUTInfoRequest.appVersion.data;
            output.gpPTC_GetDUTInfoRequest.data.partNumber = output.gpPTC_GetDUTInfoRequest.partNumber.data;
            output.gpPTC_GetDUTInfoRequest.data.productName = output.gpPTC_GetDUTInfoRequest.productName.data;
            output.gpPTC_GetDUTInfoRequest.data.ptcVersion = output.gpPTC_GetDUTInfoRequest.ptcVersion.data;
            output.gpPTC_GetDUTInfoRequest.data.productID = output.gpPTC_GetDUTInfoRequest.productID.data;

            output.gpPTC_GetDUTInfoRequest.data.result = gpPTC_GetDUTInfoRequest(
                  input.gpPTC_GetDUTInfoRequest.data.clientID
                , output.gpPTC_GetDUTInfoRequest.data.version
                , output.gpPTC_GetDUTInfoRequest.data.macAddr
                , output.gpPTC_GetDUTInfoRequest.data.bleAddr
                , output.gpPTC_GetDUTInfoRequest.data.appVersion
                , output.gpPTC_GetDUTInfoRequest.data.partNumber
                , output.gpPTC_GetDUTInfoRequest.data.productName
                , output.gpPTC_GetDUTInfoRequest.data.ptcVersion
                , output.gpPTC_GetDUTInfoRequest.data.productID
                );
            gpPTC_GetDUTInfoRequest_Output_api2buf(ackBuffer , &(output.gpPTC_GetDUTInfoRequest) , &(input.gpPTC_GetDUTInfoRequest) , &ackBytes);

            break;
        }
        case gpPTC_SetAttributeRequest_CmdId:
        {
            if(length < 4)
            {
                ackBuffer[1] = gpMarshall_AckStatusWrongParameterLength;
                break;
            }

            marshall_result = gpPTC_SetAttributeRequest_Input_buf2api(&(input.gpPTC_SetAttributeRequest) , pPayload , &_index);
            if (gpMarshall_AckStatusSuccess != marshall_result)
            {
                ackBuffer[1] = marshall_result;
                break;
            }

            output.gpPTC_SetAttributeRequest.data.result = gpPTC_SetAttributeRequest(
                  input.gpPTC_SetAttributeRequest.data.clientID
                , input.gpPTC_SetAttributeRequest.data.numberOfAttr
                , input.gpPTC_SetAttributeRequest.data.attributes
                );
            gpPTC_SetAttributeRequest_Output_api2buf(ackBuffer , &(output.gpPTC_SetAttributeRequest) , &(input.gpPTC_SetAttributeRequest) , &ackBytes);

            break;
        }
        case gpPTC_GetAttributeRequest_CmdId:
        {
            if(length < 4)
            {
                ackBuffer[1] = gpMarshall_AckStatusWrongParameterLength;
                break;
            }

            marshall_result = gpPTC_GetAttributeRequest_Input_buf2api(&(input.gpPTC_GetAttributeRequest) , pPayload , &_index);
            if (gpMarshall_AckStatusSuccess != marshall_result)
            {
                ackBuffer[1] = marshall_result;
                break;
            }

            output.gpPTC_GetAttributeRequest.data.result = gpPTC_GetAttributeRequest(
                  input.gpPTC_GetAttributeRequest.data.clientID
                , input.gpPTC_GetAttributeRequest.data.numberOfAttr
                , input.gpPTC_GetAttributeRequest.data.attributes
                );
            gpPTC_GetAttributeRequest_Output_api2buf(ackBuffer , &(output.gpPTC_GetAttributeRequest) , &(input.gpPTC_GetAttributeRequest) , &ackBytes);

            break;
        }
        case gpPTC_SetModeRequest_CmdId:
        {
            if(length < 10)
            {
                ackBuffer[1] = gpMarshall_AckStatusWrongParameterLength;
                break;
            }

            marshall_result = gpPTC_SetModeRequest_Input_buf2api(&(input.gpPTC_SetModeRequest) , pPayload , &_index);
            if (gpMarshall_AckStatusSuccess != marshall_result)
            {
                ackBuffer[1] = marshall_result;
                break;
            }

            output.gpPTC_SetModeRequest.data.result = gpPTC_SetModeRequest(
                  input.gpPTC_SetModeRequest.data.clientID
                , input.gpPTC_SetModeRequest.data.modeID
                , input.gpPTC_SetModeRequest.data.exectime
                , input.gpPTC_SetModeRequest.data.OnOff
                , input.gpPTC_SetModeRequest.data.numberOfExtraParameters
                , input.gpPTC_SetModeRequest.data.parameters
                );
            gpPTC_SetModeRequest_Output_api2buf(ackBuffer , &(output.gpPTC_SetModeRequest) , &(input.gpPTC_SetModeRequest) , &ackBytes);

            break;
        }
        case gpPTC_SetByteDataForAttributeRequest_CmdId:
        {
            if(length < 5)
            {
                ackBuffer[1] = gpMarshall_AckStatusWrongParameterLength;
                break;
            }

            marshall_result = gpPTC_SetByteDataForAttributeRequest_Input_buf2api(&(input.gpPTC_SetByteDataForAttributeRequest) , pPayload , &_index);
            if (gpMarshall_AckStatusSuccess != marshall_result)
            {
                ackBuffer[1] = marshall_result;
                break;
            }

            output.gpPTC_SetByteDataForAttributeRequest.data.result = gpPTC_SetByteDataForAttributeRequest(
                  input.gpPTC_SetByteDataForAttributeRequest.data.clientID
                , input.gpPTC_SetByteDataForAttributeRequest.data.attributeID
                , input.gpPTC_SetByteDataForAttributeRequest.data.dataLen
                , input.gpPTC_SetByteDataForAttributeRequest.data.pData
                );
            gpPTC_SetByteDataForAttributeRequest_Output_api2buf(ackBuffer , &(output.gpPTC_SetByteDataForAttributeRequest) , &(input.gpPTC_SetByteDataForAttributeRequest) , &ackBytes);

            break;
        }
        case gpPTC_GetModeRequest_CmdId:
        {
            if(length != 3)
            {
                ackBuffer[1] = gpMarshall_AckStatusWrongParameterLength;
                break;
            }

            marshall_result = gpPTC_GetModeRequest_Input_buf2api(&(input.gpPTC_GetModeRequest) , pPayload , &_index);
            if (gpMarshall_AckStatusSuccess != marshall_result)
            {
                ackBuffer[1] = marshall_result;
                break;
            }

            output.gpPTC_GetModeRequest.data.OnOff = gpPTC_GetModeRequest(
                  input.gpPTC_GetModeRequest.data.clientID
                , input.gpPTC_GetModeRequest.data.modeID
                );
            gpPTC_GetModeRequest_Output_api2buf(ackBuffer , &(output.gpPTC_GetModeRequest) , &(input.gpPTC_GetModeRequest) , &ackBytes);

            break;
        }
        case gpPTC_ExecuteCustomCommand_CmdId:
        {
            if(length < 4)
            {
                ackBuffer[1] = gpMarshall_AckStatusWrongParameterLength;
                break;
            }

            marshall_result = gpPTC_ExecuteCustomCommand_Input_buf2api(&(input.gpPTC_ExecuteCustomCommand) , pPayload , &_index);
            if (gpMarshall_AckStatusSuccess != marshall_result)
            {
                ackBuffer[1] = marshall_result;
                break;
            }
            // Initialize output pointers
            output.gpPTC_ExecuteCustomCommand.data.dataLenOut = output.gpPTC_ExecuteCustomCommand.dataLenOut;
            output.gpPTC_ExecuteCustomCommand.data.pDataOut = output.gpPTC_ExecuteCustomCommand.pDataOut;

            output.gpPTC_ExecuteCustomCommand.data.result = gpPTC_ExecuteCustomCommand(
                  input.gpPTC_ExecuteCustomCommand.data.clientID
                , input.gpPTC_ExecuteCustomCommand.data.dataLenIn
                , input.gpPTC_ExecuteCustomCommand.data.pDataIn
                , output.gpPTC_ExecuteCustomCommand.data.dataLenOut
                , output.gpPTC_ExecuteCustomCommand.data.pDataOut
                );
            gpPTC_ExecuteCustomCommand_Output_api2buf(ackBuffer , &(output.gpPTC_ExecuteCustomCommand) , &(input.gpPTC_ExecuteCustomCommand) , &ackBytes);

            break;
        }
        default:
        {
            ackBuffer[1] = gpMarshall_AckStatusUnknownCommand;
            break;
        }
    }
#undef commandId
gpPTC_HandleCommandServer_end:
    if(0 < ackBytes)
    {
        DATA_REQUEST(ackBytes,ackBuffer, communicationId);

    }
}
/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpPTC_DeInitServer(void)
{
    gpCom_DeRegisterModule(GP_MODULE_ID);
}

void gpPTC_InitServer(void)
{

    REGISTER_MODULE(gpPTC_HandleCommandServer);
}

/*****************************************************************************
 *                    gpPTC Indications
 *****************************************************************************/

void gpPTC_DiscoverIndication(gpPTC_DiscoveryInfo_t* pDiscoveryInfo)
{
    UInt16 _index = 1;
    UInt8 dataBuf[20];

#define commandId                                           dataBuf[0]


    commandId = gpPTC_DiscoverIndication_CmdId;


    gpPTC_DiscoverIndication_Input_par2api(dataBuf
        , pDiscoveryInfo
        , &_index);

    GP_ASSERT_SYSTEM(_index <= sizeof(dataBuf));
    DATA_REQUEST(_index, dataBuf, GP_PTC_COMM_ID);



#undef commandId
}

void gpPTC_RXPacketIndication(UInt8 datalength, UInt8* payload)
{
    UInt16 _index = 1;
    UInt8 dataBuf[131];

#define commandId                                           dataBuf[0]


    commandId = gpPTC_RXPacketIndication_CmdId;


    gpPTC_RXPacketIndication_Input_par2api(dataBuf
        , datalength
        , payload
        , &_index);

    GP_ASSERT_SYSTEM(_index <= sizeof(dataBuf));
    DATA_REQUEST(_index, dataBuf, GP_PTC_COMM_ID);



#undef commandId
}

void gpPTC_DataConfirm(UInt8 status, UInt16 packetsSentOK, UInt16 packetsSentError)
{
    UInt16 _index = 1;
    UInt8 dataBuf[6];

#define commandId                                           dataBuf[0]


    commandId = gpPTC_DataConfirm_CmdId;


    gpPTC_DataConfirm_Input_par2api(dataBuf
        , status
        , packetsSentOK
        , packetsSentError
        , &_index);

    GP_ASSERT_SYSTEM(_index <= sizeof(dataBuf));
    DATA_REQUEST(_index, dataBuf, GP_PTC_COMM_ID);



#undef commandId
}

void gpPTC_EDConfirm(UInt8 result, UInt8 finished, UInt16 EDValue)
{
    UInt16 _index = 1;
    UInt8 dataBuf[5];

#define commandId                                           dataBuf[0]


    commandId = gpPTC_EDConfirm_CmdId;


    gpPTC_EDConfirm_Input_par2api(dataBuf
        , result
        , finished
        , EDValue
        , &_index);

    GP_ASSERT_SYSTEM(_index <= sizeof(dataBuf));
    DATA_REQUEST(_index, dataBuf, GP_PTC_COMM_ID);



#undef commandId
}


