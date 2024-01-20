/*
 * Copyright (c) 2021, Qorvo Inc
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
#if defined(GP_COM_DIVERSITY_SERIAL_SCHEDULED_RX) || defined(GP_DIVERSITY_FREERTOS)
#include "gpSched.h"
#endif // defined(GP_COM_DIVERSITY_SERIAL_SCHEDULED_RX) || defined(GP_DIVERSITY_FREERTOS)

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#ifdef GP_COM_DIVERSITY_SERIAL_SCHEDULED_RX
#ifndef GP_COM_DIVERSITY_SERIAL_RESCHEDULE_SYMBOL_COUNT
#define GP_COM_DIVERSITY_SERIAL_RESCHEDULE_SYMBOL_COUNT     10
#endif // GP_COM_DIVERSITY_SERIAL_RESCHEDULE_PERIOD_COUNT

#define COM_SYMBOL_PERIOD           ((1000000UL * 11)/ GP_BSP_UART_COM_BAUDRATE )   // UART symbol period in us
#define COM_RCV_WINDOW              (GP_COM_DIVERSITY_SERIAL_RESCHEDULE_SYMBOL_COUNT * COM_SYMBOL_PERIOD)
#endif // GP_COM_DIVERSITY_SERIAL_SCHEDULED_RX

#if defined(GP_COM_DIVERSITY_SERIAL_NO_SYN_SENDTO_ID) && GP_COM_DIVERSITY_SERIAL_NO_SYN_SENDTO_ID > 0
#define COM_FIXED_MODULE_ID
#endif
/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

#if !defined(GP_COM_DIVERSITY_SERIAL_NO_SYN_SENDTO_ID) || GP_COM_DIVERSITY_SERIAL_NO_SYN_SENDTO_ID == 0
#define gpCom_ProtocolHeaderModuleId_x00 0
#define gpCom_ProtocolHeaderModuleId_0x0 1
#define gpCom_ProtocolHeaderModuleId_00x 2
#define gpCom_ProtocolHeaderPayload      3
typedef UInt8 gpCom_ProtocolPacketPart_t;
#endif // !defined(GP_COM_DIVERSITY_SERIAL_NO_SYN_SENDTO_ID) || GP_COM_DIVERSITY_SERIAL_NO_SYN_SENDTO_ID == 0

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
static void Com_PacketComplete(void* arg)
{
    gpCom_ProtocolState_t* state = (gpCom_ProtocolState_t*)arg;

    // Decouple packet from state
    gpCom_Packet_t* pPacket = state->pPacket;

    // reset state
    state->pPacket = NULL;
#ifndef COM_FIXED_MODULE_ID
    state->partOfPacket = gpCom_ProtocolHeaderModuleId_x00;
#endif // COM_FIXED_MODULE_ID

    Com_cbPacketReceived(pPacket);
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
// RX only function
gpCom_ProtocolStatus_t ComNoSynNoCrcProtocol_ParseByte(UInt8 rxbyte, gpCom_ProtocolState_t* state)
{
    return ComNoSynNoCrcProtocol_ParseBuffer(&rxbyte, 1, state);
}

// RX only function
gpCom_ProtocolStatus_t ComNoSynNoCrcProtocol_ParseBuffer(UInt8* rxbuf, UInt16 rxbuf_size, gpCom_ProtocolState_t* state)
{
    UInt16 rxbuf_idx;
#ifdef GP_COM_DIVERSITY_SERIAL_SCHEDULED_RX
    // Reschedule the `send` function one receive window in the future
    gpSched_UnscheduleEventArg(Com_PacketComplete, (void*)state);
    gpSched_ScheduleEventArg(COM_RCV_WINDOW, Com_PacketComplete, (void*)state);
#endif // GP_COM_DIVERSITY_SERIAL_SCHEDULED_RX

    for(rxbuf_idx = 0; rxbuf_idx < rxbuf_size; rxbuf_idx++)
    {
        UInt8 rxbyte;

        rxbyte = rxbuf[rxbuf_idx];

#ifdef COM_FIXED_MODULE_ID
        state->moduleID = GP_COM_DIVERSITY_SERIAL_NO_SYN_SENDTO_ID;
        // state->partOfPacket is unused in case of a fixed ModuleID
        state->partOfPacket = 0xFF;
#else
        if (
            (state->partOfPacket < gpCom_ProtocolHeaderPayload) &&
            ((rxbyte <'0' ) || (rxbyte > '9'))
           )
        {
            GP_LOG_SYSTEM_PRINTF("[0-9]", 0);
            continue;
        }

        switch(state->partOfPacket)
        {
            case gpCom_ProtocolHeaderModuleId_x00:
            {
                state->moduleID = rxbyte - '0';
                state->partOfPacket = gpCom_ProtocolHeaderModuleId_0x0;
                break;
            }
            case gpCom_ProtocolHeaderModuleId_0x0:
            {
                state->moduleID = (state->moduleID * 10) + (rxbyte - '0');
                state->partOfPacket = gpCom_ProtocolHeaderModuleId_00x;
                break;
            }
            case gpCom_ProtocolHeaderModuleId_00x:
            {
                state->moduleID = (state->moduleID * 10) + (rxbyte - '0');
                GP_LOG_SYSTEM_PRINTF("-> moduleID=%d", 0, state->moduleID);
                state->partOfPacket = gpCom_ProtocolHeaderPayload;
                break;
            }
            case gpCom_ProtocolHeaderPayload:
            {
#endif // COM_FIXED_MODULE_ID
                // Get packet to store payload
                if(NULL == state->pPacket)
                {
                    state->pPacket = Com_GetFreePacket();
                    if(NULL == state->pPacket)
                    {
#ifdef GP_COM_DIVERSITY_SERIAL_SCHEDULED_RX
                        gpSched_UnscheduleEventArg(Com_PacketComplete, (void*)state);
#endif // GP_COM_DIVERSITY_SERIAL_SCHEDULED_RX
                        return gpCom_ProtocolError;
                    }
                    else
                    {
                        state->pPacket->length = 0;
                        state->pPacket->commId = state->commId;
                        state->pPacket->moduleID = state->moduleID;
                    }
                }

                // Store byte
                state->pPacket->packet[state->pPacket->length] = rxbyte;
                state->pPacket->length++;

                if(state->pPacket->length == GP_COM_MAX_PACKET_PAYLOAD_SIZE)
                {
                    // packet will exceed its maximum length on the next iteration, send now
#ifdef GP_COM_DIVERSITY_SERIAL_SCHEDULED_RX
                    gpSched_UnscheduleEventArg(Com_PacketComplete, (void*)state);
#endif // GP_COM_DIVERSITY_SERIAL_SCHEDULED_RX
                    Com_PacketComplete((void*)state);
                }
#ifndef GP_COM_DIVERSITY_SERIAL_SCHEDULED_RX
                else if('\r' == rxbyte) // check for 'enter'
                {
                    Com_PacketComplete((void*)state);
                }
#endif // GP_COM_DIVERSITY_SERIAL_SCHEDULED_RX
#ifndef COM_FIXED_MODULE_ID
                break;
            }
            default:
            {
                break;
            }
        }
#endif // COM_FIXED_MODULE_ID
    }

    return gpCom_ProtocolDone;
}
