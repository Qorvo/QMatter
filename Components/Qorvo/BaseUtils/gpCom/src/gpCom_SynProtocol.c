/*
 * Copyright (c) 2014-2016, GreenPeak Technologies
 * Copyright (c) 2017, 2019, Qorvo Inc
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
 * $Header: //depot/release/Embedded/Components/Qorvo/BaseUtils/v2.10.2.1/comps/gpCom/src/gpCom_SynProtocol.c#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_COM

#include "hal.h"
#include "gpUtils.h"
#include "gpCom.h"
#include "gpCom_defs.h"

#include "gpLog.h"
#include "gpAssert.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

#define gpCom_ProtocolHeaderS              0
#define gpCom_ProtocolHeaderY              1
#define gpCom_ProtocolHeaderN              2
#define gpCom_ProtocolHeaderLength         3
#define gpCom_ProtocolHeaderFrameControl   4
#define gpCom_ProtocolHeaderModule         5
typedef UInt8 gpCom_ProtocolHeader_t;

#define gpCom_ProtocolFooterCRCByteLSB     0
#define gpCom_ProtocolFooterCRCByteMSB     1
typedef UInt8 gpCom_ProtocolFooter_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    External Data Definition
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/


/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

//RX only function
gpCom_ProtocolStatus_t ComSynProtocol_ParseByte(UInt8 rxbyte, gpCom_ProtocolState_t* state)
{
    return ComSynProtocol_ParseBuffer(&rxbyte,1,state);
}

gpCom_ProtocolStatus_t ComSynProtocol_ParseBuffer(UInt8 *rxbuf, UInt16 rxbuf_size, gpCom_ProtocolState_t* state)
{
    Bool error = false;
    gpCom_ProtocolStatus_t status = gpCom_ProtocolContinue;
    UInt16 rxbuf_idx;

    for (rxbuf_idx = 0; rxbuf_idx < rxbuf_size; rxbuf_idx++)
    {
        UInt8 rxbyte = rxbuf[rxbuf_idx];
        switch (state->partOfPacket) {
        // Header section
        case gpCom_ProtocolPacketHeader:
        {
            //GP_LOG_SYSTEM_PRINTF("H[%i]:%x %i %c",0,state->counter, rxbyte, rxbyte, rxbyte);
            switch (state->counter)
            {
                case gpCom_ProtocolHeaderS:
                    if (rxbyte != 'S')
                    {
                        /* we can't discard the entire buffer since it can contain multiple SPI transfers */
#if defined(GP_COM_DIVERSITY_SERIAL_SPI)
                        UInt16 idx;
                        /* apply a speed optimisation by fast forward over 0xA5/NO_DATA in a local loop */
                        for (idx = rxbuf_idx; idx < rxbuf_size; idx++)
                        {
                            if (rxbuf[idx] != 0xA5)
                            {
                                break;
                            }
                        }
                        if (idx != rxbuf_idx)
                        {
                            rxbuf_idx = idx - 1;
                        }
#endif
                        continue;
                    }
                    else
                    {
                        status = gpCom_ProtocolContinue;
                        state->crc = 0x0000;
                    }
                    break;
                case gpCom_ProtocolHeaderY :
                    if (rxbyte != 'Y')
                    {
                        error = true;
                        break;
                    }
                    break;
                case gpCom_ProtocolHeaderN :
                    if (rxbyte != 'N')
                    {
                        error = true;
                        break;
                    }
                    break;
                case gpCom_ProtocolHeaderLength:
                    //Add lower 8 bits of length
                    state->length = rxbyte;
                    break;

                case gpCom_ProtocolHeaderFrameControl:
                    //Add top 4 bits of length
                    state->length += ((UInt16)(rxbyte & 0x0F) << 8);

                    if (state->length > GP_COM_MAX_PACKET_PAYLOAD_SIZE)
                    {
                        error = true;
                        GP_LOG_SYSTEM_PRINTF("len: %u > %u",4,(UInt16)state->length,(UInt16)GP_COM_MAX_PACKET_PAYLOAD_SIZE);
                        break;
                    }
                    break;
                case gpCom_ProtocolHeaderModule:
                    state->moduleID = rxbyte;
                    break;
                default :
                    error = true;
                    break;
            }

            if (!error)
            {
                gpUtils_UpdateCrc(&(state->crc),(UInt8)rxbyte);
                if (++state->counter >= GP_COM_PACKET_HEADER_LENGTH)
                {
                    if (state->length > 0)
                    {
                        state->partOfPacket =  gpCom_ProtocolPacketData;
                    }
                    else
                    {
                        state->partOfPacket =  gpCom_ProtocolPacketFooter;
                    }
                    state->counter = 0;
                }
            }
            break;
        }
        case gpCom_ProtocolPacketData:
        {
            //GP_LOG_SYSTEM_PRINTF("D[%i]:%x %i",0,state->counter, rxbyte, rxbyte);
            if(NULL == state->pPacket)
            {
                //Allocate data buffer
                state->pPacket = Com_GetFreePacket();

                //No data available ?
                if(NULL == state->pPacket)
                {
                    GP_LOG_SYSTEM_PRINTF("No more RX pkts",0);
                    error = true;
                    break;
                }
            }

            //Fill in data
            state->pPacket->packet[state->counter] = rxbyte;
            gpUtils_UpdateCrc(&(state->crc),(UInt8)rxbyte);
            state->counter++;
            if (state->counter >= state->length)
            {
                state->partOfPacket = gpCom_ProtocolPacketFooter;
                state->counter      = gpCom_ProtocolFooterCRCByteLSB;
            }
            break;
        }
        case gpCom_ProtocolPacketFooter:
        {
            //GP_LOG_SYSTEM_PRINTF("F[%i]:%x %i",0,state->counter, rxbyte, rxbyte);
            // Receive footer
            switch (state->counter)
            {
                case gpCom_ProtocolFooterCRCByteLSB:
                {
                    UInt8 calculatedCRCByteMSB = (UInt8)(state->crc >> 0);
                    if (calculatedCRCByteMSB != rxbyte)
                    {
                        error = true;
                        break;
                    }
                    break;
                }
                case gpCom_ProtocolFooterCRCByteMSB :
                {
                    UInt8 calculatedCRCByteLSB = (UInt8)(state->crc >> 8);
                    if (calculatedCRCByteLSB != rxbyte)
                    {
                        error = true;
                        break;
                    }
                    break;
                }
                default :
                {
                    error = true;
                    break;
                }
            }

            if (!error)
            {
                if (++state->counter >= GP_COM_PACKET_FOOTER_LENGTH)
                {
                    {
                        if(state->length > 0)
                        {
                            //Add header info to currently allocated packet
                            state->pPacket->length   = state->length;
                            state->pPacket->moduleID = state->moduleID;
                            state->pPacket->commId   = state->commId;

                            Com_cbPacketReceived(state->pPacket);
                            //Clear temporary pointer
                            state->pPacket = NULL;
                        }
                    }
                    goto Com_ParseSynProtocol_done;
                }
                break;
            }

            break;
        }
        default:
        {
            error = true;
        }
        }

        if (error)
        {
            //Padding bytes before parsing not considered an error, just reset state machine
            if(!((state->counter == gpCom_ProtocolHeaderS) && (state->partOfPacket == gpCom_ProtocolPacketHeader)))
            {
#if defined(GP_COM_COMM_ID_BRIDGE_1) && defined(GP_COM_COMM_ID_BRIDGE_2)
                //Logging from bridging device
                GP_LOG_SYSTEM_PRINTF("?d:%x c:%i p:%i l:%u",8,(UInt16)rxbyte, (UInt16)state->counter,(UInt16)state->partOfPacket , (UInt16)state->length);
#else
                //Logging from non-bridge
                GP_LOG_SYSTEM_PRINTF("!d:%x c:%i p:%i l:%u",8,(UInt16)rxbyte, (UInt16)state->counter,(UInt16)state->partOfPacket , (UInt16)state->length);
#endif
            }

            // If an unknown byte is received
            //Free possible allocated memory
            if(state->pPacket != NULL)
            {
                Com_FreePacket(state->pPacket);
                state->pPacket = NULL;
            }

            goto Com_ParseSynProtocol_done;
        }

        //Not done parsing full packet yet
        continue;

Com_ParseSynProtocol_done:
        //Reset state
        state->counter       = gpCom_ProtocolHeaderS;
        state->partOfPacket  = gpCom_ProtocolPacketHeader;
        state->length        = 0;
        state->moduleID      = 0x0;

        //Done parsing - can throw away state if wanted
        status = error ? gpCom_ProtocolError : gpCom_ProtocolDone;
    }

    return status;
}


