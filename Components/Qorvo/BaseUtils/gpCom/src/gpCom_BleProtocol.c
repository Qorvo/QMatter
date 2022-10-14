/*
 * Copyright (c) 2015-2016, GreenPeak Technologies
 * Copyright (c) 2017-2019, Qorvo Inc
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
 * Alternatively, this software may be distributed under the terms of the
 * modified BSD License or the 3-clause BSD License as published by the Free
 * Software Foundation @ https://directory.fsf.org/wiki/License:BSD-3-Clause
 *
 * $Header$
 * $Change$
 * $DateTime$
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
//#define GP_LOCAL_LOG
#define GP_COMPONENT_ID GP_COMPONENT_ID_COM

#include "hal.h"
#include "gpUtils.h"
#include "gpCom.h"
#include "gpCom_defs.h"

#include "gpLog.h"
#include "gpAssert.h"
#include "gpHci.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/* 'partOfPacket' state is initialzed externally using 'gpCom_ProtocolPacketHeader' */
#define gpCom_ProtocolHciHeader          gpCom_ProtocolPacketHeader
GP_COMPILE_TIME_VERIFY( 0 == gpCom_ProtocolHciHeader );
#define gpCom_ProtocolHciData            1
/* not an enum value but a special bit set in 'partOfPacket' to indicate hunt mode is active */
#define gpCom_ProtocolResetHuntMode      2

#define COUNT_TO_CASE(type, count)    ( (type << 2) | (count))
#define HCI_PACKET_TYPE               COUNT_TO_CASE(0, 0)
#define HCI_CMD_OPCODE_0              COUNT_TO_CASE(gpHci_CommandPacket, 0)
#define HCI_CMD_OPCODE_1              COUNT_TO_CASE(gpHci_CommandPacket, 1)
#define HCI_CMD_PARAM_LENGTH          COUNT_TO_CASE(gpHci_CommandPacket, 2)
#define HCI_DATA_HANDLE_0             COUNT_TO_CASE(gpHci_DataPacket, 0)
#define HCI_DATA_HANDLE_1             COUNT_TO_CASE(gpHci_DataPacket, 1)
#define HCI_DATA_LENGTH_0             COUNT_TO_CASE(gpHci_DataPacket, 2)
#define HCI_DATA_LENGTH_1             COUNT_TO_CASE(gpHci_DataPacket, 3)
#define HCI_ISODATA_HANDLE_0          COUNT_TO_CASE(gpHci_IsoDataPacket, 0)
#define HCI_ISODATA_HANDLE_1          COUNT_TO_CASE(gpHci_IsoDataPacket, 1)
#define HCI_ISODATA_LENGTH_0          COUNT_TO_CASE(gpHci_IsoDataPacket, 2)
#define HCI_ISODATA_LENGTH_1          COUNT_TO_CASE(gpHci_IsoDataPacket, 3)
#define HCI_EVENT_CODE                COUNT_TO_CASE(gpHci_EventPacket, 0)
#define HCI_EVENT_LENGTH              COUNT_TO_CASE(gpHci_EventPacket, 1)
// Note that HCI Synchronous Data packets (gpHci_SyncDataPacket) are not used in BLE: see BT spec HCI $5.4.3)


/* We should be able to buffer at least the header */
GP_COMPILE_TIME_VERIFY( (4 /* max header size */ ) <= GP_COM_MAX_PACKET_PAYLOAD_SIZE );

#define RESULT_CONTINUE ((Hci_ParseResult_t){.status=gpCom_ProtocolContinue,.errorCode=0})
#define RESULT_DONE ((Hci_ParseResult_t){.status=gpCom_ProtocolDone,.errorCode=0})
#define RESULT_ERROR(error_code) ((Hci_ParseResult_t){.status=gpCom_ProtocolError,.errorCode=error_code})

#define COM_ISO_DATA_LENGTH_MASK_MSB     0x3F

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct _parse_result {
    gpCom_ProtocolStatus_t status;
    UInt8 errorCode;
} Hci_ParseResult_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    External Data Definition
 *****************************************************************************/

static const UInt8 hci_reset_cmd[4] = {gpHci_CommandPacket, 0x3, 0xC, 0x0};
static const UInt8 hci_reset_evp[7] = {gpHci_EventPacket, 0x0E, 0x04, 0x01, 0x03, 0x0C, 0x00};

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static UInt32 packetType_to_comm(gpHci_PacketType_t packetType);
static Hci_ParseResult_t Com_ParseHciFrame(UInt8 rxbyte, gpCom_ProtocolState_t* state);
static Hci_ParseResult_t Com_AppendByteToPacket(Hci_ParseResult_t tmp_result, gpCom_ProtocolState_t* state, UInt8 rxbyte);

static gpCom_ProtocolStatus_t Com_ParseBleProtocol_Normal(UInt8 rxbyte, gpCom_ProtocolState_t* state);
static UInt8 Com_GetHciHeaderLength(UInt8 packetType);
static Bool Com_FindResetBytes(UInt16 *pHuntIdx, UInt16 remaining_bytes, const gpCom_ProtocolState_t *state);

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

#ifndef GP_COMP_HCI
Bool gpHci_HardwareErrorEvent(UInt8 eventCode, UInt8 hwcode)
{
    // FIXME: if an error happens on a bridge it will just drop packets and the host won't get an error event
    return true;
}
#endif

UInt32 packetType_to_comm(gpHci_PacketType_t packetType)
{
    const UInt8 PacketTypeToCommId[6] = {0,                                  // undefined
                                          GP_COM_COMM_ID_BLE_COMMAND >> 16,  // 1: HCI command packet
                                          GP_COM_COMM_ID_BLE_DATA >> 16,     // 2: HCI Data packet
                                          0,                                 // 3: HCI synchronous data for BT Classic
                                          GP_COM_COMM_ID_BLE_EVENT >> 16,    // 4: HCI Event
                                          GP_COM_COMM_ID_BLE_ISODATA >> 16,  // 5: HCI ISO data
                                        };

    UInt32 commId = PacketTypeToCommId[packetType] << 16;
    return commId;
}

Bool Com_FindResetBytes(UInt16 *pHuntIdx, UInt16 size, const gpCom_ProtocolState_t *state)
{
    Bool inHuntMode = true;
    UInt16 idx;

    /* The idx moves forward until a pattern match is possible */
    for (idx=0; idx < size; ++idx)
    {
        /* reset command */
        UInt16 length = size < sizeof(hci_reset_cmd) ? (size - idx) : sizeof(hci_reset_cmd);
        if (0 == MEMCMP(&state->pPacket->packet[idx],hci_reset_cmd,length))
        {
            //GP_LOG_SYSTEM_PRINTF("stop hunt: length: %u",0,length);
            inHuntMode = (sizeof(hci_reset_cmd) == length) ? false : true;
            break;
        }
        /* command complete of reset */
        length = size < sizeof(hci_reset_evp) ? (size - idx) : sizeof(hci_reset_evp);
        if (0 == MEMCMP(&state->pPacket->packet[idx],hci_reset_evp,length))
        {
            //GP_LOG_SYSTEM_PRINTF("stop hunt: length: %u",0,length);
            inHuntMode = (sizeof(hci_reset_evp) == length) ? false : true;
            break;
        }
    }

    *pHuntIdx = idx;
    return inHuntMode;
}

Hci_ParseResult_t Com_AppendByteToPacket(Hci_ParseResult_t tmp_result, gpCom_ProtocolState_t* state, UInt8 rxbyte)
{
    if (state->counter == 0)
    {
        if (!state->pPacket)
        {
            state->pPacket = Com_GetFreePacket();
            //GP_LOG_SYSTEM_PRINTF("claim %p",0,state->pPacket);
            if (!state->pPacket)
            {
                GP_LOG_SYSTEM_PRINTF("unable to claim gpcom packet",0);
                /* NOTE: We don't return an error and continue parsing until the
                *  end of the frame because we  don't want to start the hunt
                *  mode until after the entire HCI frame was received!
                */
            }
        }
    }

    if (state->counter < GP_COM_MAX_PACKET_PAYLOAD_SIZE)
    {
        /* allocating a packet could have failed */
        if (state->pPacket)
        {
            //GP_LOG_SYSTEM_PRINTF("write %u",0,state->counter);
            state->pPacket->packet[state->counter] = rxbyte;
        }
    }

    GP_ASSERT_SYSTEM (state->counter < 0xFFFF);
    state->counter++;

    return tmp_result;
}

UInt8 Com_GetHciHeaderLength(UInt8 packetType)
{
    switch(packetType)
    {
        case gpHci_DataPacket:
        case gpHci_IsoDataPacket:
            return 4;
        case gpHci_CommandPacket:
            return 3;
        case gpHci_EventPacket:
            return 2;
        default:
            GP_ASSERT_SYSTEM(false);
            return 0;
    }
}

Hci_ParseResult_t Com_ParseHciFrame(UInt8 rxbyte, gpCom_ProtocolState_t* state)
{
    Hci_ParseResult_t tmp_result = RESULT_CONTINUE;
    UInt8 packetType = state->moduleID;

    GP_LOG_PRINTF("bytes:[%u]%x",0,(unsigned int)state->counter,rxbyte);

    switch (state->partOfPacket)
    {
        // Header section
        case gpCom_ProtocolHciHeader:
        {
            UInt16 counter = state->counter;

            //GP_LOG_SYSTEM_PRINTF("bytes:%x %x",0,packetType,counter);

            switch(COUNT_TO_CASE(packetType, counter))
            {
                case HCI_PACKET_TYPE:
                {
                    state->length = 0;
                    switch(rxbyte)
                    {
                        case gpHci_DataPacket:
                        case gpHci_CommandPacket:
                        case gpHci_IsoDataPacket:
                        case gpHci_EventPacket:
                            /* Misusing the moduleID to store the packetType */
                            state->moduleID = (UInt8) rxbyte;

                            break;
                        default:
                        {
                            if(rxbyte == 0xFF)
                            {
                                GP_LOG_SYSTEM_PRINTF("ComBle_ProtErr type:%x - dropping",0,rxbyte);
                            }
                            else
                            {
                                GP_LOG_SYSTEM_PRINTF("ComBle_ProtErr type:%x",0,rxbyte);
                                tmp_result = RESULT_ERROR(gpHci_OutOfSync);
                            }
                        }
                    }
                    break;
                }
                case HCI_CMD_OPCODE_0:
                case HCI_CMD_OPCODE_1:
                case HCI_DATA_HANDLE_0:
                case HCI_DATA_HANDLE_1:
                case HCI_ISODATA_HANDLE_0:
                case HCI_ISODATA_HANDLE_1:
                case HCI_EVENT_CODE:
                {
                    tmp_result = Com_AppendByteToPacket(tmp_result, state, rxbyte);
                    break;
                }

                case HCI_CMD_PARAM_LENGTH:
                case HCI_EVENT_LENGTH:
                {
                    //GP_LOG_SYSTEM_PRINTF("e-length: %x",0,state->length);
                    state->partOfPacket = gpCom_ProtocolHciData;
                    /* length can be 0 */
                    if (0 == rxbyte)
                    {
                        tmp_result = RESULT_DONE;
                    }
                    //GP_LOG_SYSTEM_PRINTF(" PacketData length %d",2, rxbyte );
                    // no break because common code
                }
                case HCI_DATA_LENGTH_0:
                case HCI_ISODATA_LENGTH_0:
                {
                    state->length = rxbyte;
                    tmp_result = Com_AppendByteToPacket(tmp_result, state, rxbyte);
                    break;
                }
                case HCI_DATA_LENGTH_1:
                case HCI_ISODATA_LENGTH_1:
                {
                    if(state->moduleID == gpHci_IsoDataPacket)
                    {
                        // Ignore RFU bits
                        rxbyte &= COM_ISO_DATA_LENGTH_MASK_MSB;
                    }

                    state->length += (rxbyte << 8);
                    state->partOfPacket = gpCom_ProtocolHciData;
                    /* length can be 0 */
                    if (0 == state->length)
                    {
                        tmp_result = RESULT_DONE;
                    }
                    //GP_LOG_SYSTEM_PRINTF(" PacketData length %d",2, state->length );
                    tmp_result = Com_AppendByteToPacket(tmp_result, state, rxbyte);
                    break;
                }
                default:
                {
                    GP_ASSERT_SYSTEM(false);
                    break;
                }
            }
            break;
        }
        case gpCom_ProtocolHciData:
        {
            //GP_LOG_SYSTEM_PRINTF("c:%lu",0,(long unsigned int)state->counter);
            UInt32 total_size = Com_GetHciHeaderLength(packetType) + state->length;

            tmp_result = Com_AppendByteToPacket(tmp_result, state, rxbyte);

            if (state->counter == total_size || /* overflow protection */ state->counter == 0xFFFF)
            {
                if (state->counter > GP_COM_MAX_PACKET_PAYLOAD_SIZE)
                {
                    GP_LOG_SYSTEM_PRINTF("length too big:%u",0,(unsigned int)state->length);
                    tmp_result = RESULT_ERROR(gpHci_BufferOverflow);
                }
                else if (state->counter == total_size)
                {
                    //GP_LOG_SYSTEM_PRINTF("done:%u",0,(unsigned int)total_size);
                    tmp_result = RESULT_DONE;
                }
                else if (state->counter == 0xFFFF)
                {
                    GP_LOG_SYSTEM_PRINTF("counter overflow:%x",0,state->counter);
                    tmp_result = RESULT_ERROR(gpHci_BufferOverflow);
                }
                else
                {
                    GP_ASSERT_SYSTEM(false);
                }
            }

            break;
        }
        default:
        {
            GP_ASSERT_SYSTEM(false);
            break;
        }
    }

    if (gpCom_ProtocolDone == tmp_result.status)
    {
        if (state->pPacket)
        {
            GP_LOG_PRINTF("BLE gpCom_ProtocolDone ...",0);
            state->pPacket->length   = Com_GetHciHeaderLength(packetType) + state->length;
            state->pPacket->moduleID = 156; //state->moduleID; FIXME  - currently hardcoded gpHci
            state->pPacket->commId   = state->commId & ~GP_COM_COMM_ID_BLE;
            state->pPacket->commId  |= packetType_to_comm(packetType);
            state->partOfPacket      = gpCom_ProtocolHciHeader;
        }
        else
        {
            tmp_result = RESULT_ERROR(gpHci_NoBuffers);
        }
    }

    return tmp_result;
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

gpCom_ProtocolStatus_t Com_ParseBleProtocol_Normal(UInt8 rxbyte, gpCom_ProtocolState_t* state)
{
    gpCom_ProtocolStatus_t status = gpCom_ProtocolContinue;
    UInt8 ErrorCode=0x00;

    GP_LOG_PRINTF("Com_ParseBleProt part %d cnt %d 0x%x",6, state->partOfPacket, state->counter, rxbyte);

    // Parsing here need not be aware of "hunt-mode"
    Hci_ParseResult_t result = Com_ParseHciFrame(rxbyte, state);
    status = result.status;
    ErrorCode = result.errorCode;

    if (gpCom_ProtocolDone == status)
    {
        //GP_LOG_SYSTEM_PRINTF("BLE gpCom_ProtocolDone ...",0);


        if(Com_AddPendingPacket(state->pPacket))
        {
#ifndef GP_COMP_SCHED
            // FIXME: why is this here?
            gpCom_HandleRx();
#endif //!GP_COMP_SCHED
            state->pPacket = NULL;
        }
        else
        {
            GP_LOG_SYSTEM_PRINTF("No space for handling",0);
            ErrorCode = gpHci_NoBuffers;
            status = gpCom_ProtocolError;
        }
    }

    if (gpCom_ProtocolError == status)
    {
        GP_LOG_SYSTEM_PRINTF("BLE ERROR...",0);

        GP_ASSERT_DEV_INT(ErrorCode != 0x00);
        gpHci_HardwareErrorEvent(gpHci_EventCode_HardwareError, ErrorCode);
    }

    if (gpCom_ProtocolContinue != status)
    {
        //GP_LOG_SYSTEM_PRINTF("reset",0);
        /* FIXME: already reset by Error */
        state->counter       = 0;
        state->length        = 0;
        state->moduleID      = 0x0;
        state->partOfPacket  = gpCom_ProtocolHciHeader;
    }

    return status;
}

gpCom_ProtocolStatus_t Com_ParseBleProtocol(UInt8 rxbyte, gpCom_ProtocolState_t* state)
{
    gpCom_ProtocolStatus_t status = gpCom_ProtocolContinue;
    Bool inHuntMode;

    inHuntMode = gpCom_ProtocolResetHuntMode & state->partOfPacket ? true: false;
    // clear the hunt-mode part
    state->partOfPacket &= ~gpCom_ProtocolResetHuntMode;

    if (inHuntMode)
    {
        UInt16 remaining_bytes = state->counter;

        if (!state->pPacket)
        {
            //GP_LOG_SYSTEM_PRINTF("claim",0);
            state->pPacket = Com_GetFreePacket();
        }

        if (state->pPacket)
        {
            UInt16 hunt_idx;

            state->pPacket->packet[remaining_bytes++] = rxbyte;

            inHuntMode = Com_FindResetBytes(&hunt_idx, remaining_bytes, state);

            /* shift out all "discarded" bytes to the left */
            remaining_bytes -= hunt_idx;

            /* Keep bytes that match */
            if (remaining_bytes)
            {
                /* move forward */
                /* NOTE: MEMCPY does not work for overlapping memory segments */
                UInt16 idx;
                for (idx=0; idx<remaining_bytes; ++idx)
                {
                    state->pPacket->packet[idx] = state->pPacket->packet[hunt_idx+idx];
                }
            }
        }

        if ( /* still */ inHuntMode )
        {
            /* need more input -- retain the the remaining bytes for later */
            state->counter = remaining_bytes;
        }
        else
        {
            UInt16 idx;

            GP_LOG_SYSTEM_PRINTF("BLE reset",0);
            /* Do normal parsing of the found pattern */
            state->counter = 0;
            for (idx=0; idx < remaining_bytes; ++idx)
            {
                rxbyte = state->pPacket->packet[idx];
                //GP_LOG_SYSTEM_PRINTF("? %d %d %x %x", 0, idx, remaining_bytes, rxbyte, inHuntMode);
                status = Com_ParseBleProtocol_Normal(rxbyte, state);
                /* Can still fail because of out-of-memory issues */
                if (gpCom_ProtocolError == status)
                {
                    inHuntMode = true;
                    break;
                }
            }
        }
    }
    else
    {
         /* Normal parsing */
        status = Com_ParseBleProtocol_Normal(rxbyte, state);
        if (gpCom_ProtocolError == status)
        {
            GP_LOG_SYSTEM_PRINTF("BLE hunt",0);
            inHuntMode = true;
        }
    }

    // free
    if (0 == state->counter)
    {
        if(state->pPacket)
        {
            //GP_LOG_SYSTEM_PRINTF("free: %p",0,state->pPacket);
            Com_FreePacket(state->pPacket);
            state->pPacket       = NULL;
        }
    }

    // set the hunt-mode bit
    if (inHuntMode)
    {
        state->partOfPacket |= gpCom_ProtocolResetHuntMode;
    }

    return status;
}
