/*
 * Copyright (c) 2017, 2019, Qorvo Inc
 *
 * gpCom.c
 *
 * This file contains the implementation of the serial communication module.
 * It implements the GreenPeak serial protocol.
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
 * $Header: //depot/release/Embedded/Components/Qorvo/BaseUtils/v2.10.2.1/comps/gpCom/src/gpCom_serialUART.c#1 $
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

#if defined(HAL_DIVERSITY_UART_RX_BUFFER_CALLBACK)
static void Com_cbUartRx(UInt8*buffer, UInt16 size);
#else
static void Com_cbUartRx(Int16 rxbyte); //Rx only function
#endif




/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

static Int16 Com_cbUartGetTxData(gpCom_CommunicationId_t commId)
{
    Int16 returnValue = -1;

    HAL_DISABLE_GLOBAL_INT();

    if (Com_IsDataWaiting(commId))
    {
        returnValue = Com_GetData(commId);
    }

    HAL_ENABLE_GLOBAL_INT();

    return returnValue;
}

static Int16 Com_cbUart1GetTxData(void)
{
    return Com_cbUartGetTxData(GP_COM_COMM_ID_UART1);
}
#if GP_COM_NUM_UART == 2 
static Int16 Com_cbUart2GetTxData(void)
{
    return Com_cbUartGetTxData(GP_COM_COMM_ID_UART2);
}
#endif
//RX only function
#if defined(HAL_DIVERSITY_UART_RX_BUFFER_CALLBACK)
static void Com_cbUartRx(UInt8 *buffer, UInt16 size)
{
    gpCom_ProtocolState_t* const state = &gpComUart_RxState[0];
    state->commId = GP_COM_COMM_ID_UART1;
#ifdef GP_COM_DIVERSITY_SERIAL_NO_SYN_NO_CRC
    ComNoSynNoCrcProtocol_ParseBuffer(buffer, size, state);
#else
    ComSynProtocol_ParseBuffer(buffer, size, state);
#endif // GP_COM_DIVERSITY_SERIAL_NO_SYN_NO_CRC
}

#if (GP_COM_NUM_UART == 2)
static void Com_cbUart2Rx(UInt8 *buffer, UInt16 size)
{
    gpCom_ProtocolState_t* const state = &gpComUart_RxState[1];
    state->commId = GP_COM_COMM_ID_UART2;
#ifdef GP_COM_DIVERSITY_SERIAL_NO_SYN_NO_CRC
    ComNoSynNoCrcProtocol_ParseBuffer(buffer, size, state);
#else
    ComSynProtocol_ParseBuffer(buffer, size, state);
#endif // GP_COM_DIVERSITY_SERIAL_NO_SYN_NO_CRC

}
#endif
#else
static void Com_cbUartRx(Int16 rxbyte)
{
    Com_ParseProtocol(rxbyte, GP_COM_COMM_ID_UART1);
}

#if (GP_COM_NUM_UART == 2)
static void Com_cbUart2Rx(Int16 rxbyte)
{
    Com_ParseProtocol(rxbyte, GP_COM_COMM_ID_UART2);
}
#endif
#endif

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

/****************************************************************************
 ****************************************************************************
 **                       Initialization                                   **
 ****************************************************************************
 ****************************************************************************/

void gpComUart_Init(void)
{
    // Initialize the UART (serial port)
    HAL_UART_COM_START( Com_cbUartRx , Com_cbUart1GetTxData);
#if GP_COM_NUM_UART == 2 
    HAL_UART_COM2_START( Com_cbUart2Rx, Com_cbUart2GetTxData);
#endif

}

void gpComUart_DeInit(void)
{
    // De-Initialize the UART
    HAL_UART_COM_STOP();
    HAL_UART_COM_POWERDOWN();

#if GP_COM_NUM_UART == 2 
    HAL_UART_COM2_STOP();
    HAL_UART_COM2_POWERDOWN();
#endif
}

void gpComUart_Flush(void)
{
#ifdef HAVE_HAL_UART_FLUSH
    hal_UartComFlush(GP_BSP_UART_COM1);
#ifdef GP_BSP_UART_COM2
    hal_UartComFlush(GP_BSP_UART_COM2);
#endif //GP_BSP_UART_COM2
#endif
}

void ComUart_FlushRx(void)
{
#ifdef HAVE_HAL_UART_RX_FLUSH
    hal_UartRxComFlush(GP_BSP_UART_COM1);
#ifdef GP_BSP_UART_COM2
    hal_UartRxComFlush(GP_BSP_UART_COM2);
#endif //GP_BSP_UART_COM2
#endif
}


void ComUart_TriggerTx(UInt8 uart)
{
#if GP_COM_NUM_UART == 2
    if(uart == 1)
    {
        HAL_UART_COM2_TX_NEW_DATA();
    }
    else
#endif
    if(uart == 0)
    {
        HAL_UART_COM_TX_NEW_DATA();
    }
}


