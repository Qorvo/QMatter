/*
 * Copyright (c) 2022, Qorvo Inc
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
 */

/** @file "qvIO_UART.c"
 *
 *  IO UART functionality
 *
 *  Implementation of qvIO UART Tx/Rx.
 *  HW initialization of pins handled at HAL level.
 *  This implementation hooks up a callback for incoming UART bytes,
 *  storing it in a circular buffer for polled read-out.
*/

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
/* <CodeGenerator Placeholder> Includes */

#define GP_COMPONENT_ID GP_COMPONENT_ID_QVIO
#define GP_MODULE_ID    GP_COMPONENT_ID

#include "qvIO.h"

#include "hal.h"
#include "gpCom.h"
#ifndef GP_COM_DIVERSITY_NO_RX
#include "gpUtils.h"
#endif //GP_COM_DIVERSITY_NO_RX

/* </CodeGenerator Placeholder> Includes */

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

#ifndef GP_COM_DIVERSITY_NO_RX
static uint8_t IO_UartRxData[256];
static gpUtils_CircularBuffer_t IO_UartRxBuffer;
#endif //GP_COM_DIVERSITY_NO_RX

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/*****************************************************************************
 * --- UART handling
 *****************************************************************************/

#ifndef GP_COM_DIVERSITY_NO_RX
static void qvIO_cbRxData(UInt16 length, UInt8* pPayload, gpCom_CommunicationId_t communicationId)
{
    Bool writeSucces;

    HAL_DISABLE_GLOBAL_INT();
    writeSucces = gpUtils_CircBWriteData(&IO_UartRxBuffer, pPayload, length);
    HAL_ENABLE_GLOBAL_INT();

    GP_ASSERT_DEV_INT(writeSucces);
}
#endif //GP_COM_DIVERSITY_NO_RX

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

/*****************************************************************************
 * UART control
 *****************************************************************************/

#ifndef GP_COM_DIVERSITY_SERIAL_NO_SYN_NO_CRC
#error Raw gpCom output expected within IO builds. Generic serial terminal support required.
#endif //GP_COM_DIVERSITY_SERIAL_NO_SYN_NO_CRC

/** @brief Transmit data over the UART
 *
 *  @param length         Length of data to send
 *  @param txBuffer       Pointer to a buffer to transmit
*/
void qvIO_UartTxData(uint8_t length, const char* txBuffer)
{
    // Expecting gpCom to be used in a raw fashion
    gpCom_Flush();
    gpCom_DataRequest(GP_COMPONENT_ID_QVIO, length, (UInt8*)txBuffer, GP_COM_DEFAULT_COMMUNICATION_ID);
}

#ifndef GP_COM_DIVERSITY_NO_RX
/** @brief Get received UART data
 *
 *  @param length          Max length of data to retrieve
 *  @param txBuffer        Pointer to a buffer to received the data in.
 *                         It is assumed to be large enough to hold 'length' bytes
 *
 *  @return receivedLength Amount of bytes returned in the buffer.
 *                         If more data was pending, only 'length' bytes are returned.
*/
uint8_t qvIO_UartReadRxData(uint8_t length, char* rxBuffer)
{
    // Expected to be called from other thread
    uint8_t available;

    HAL_DISABLE_GLOBAL_INT();
    available = gpUtils_CircBAvailableData(&IO_UartRxBuffer);
    gpUtils_CircBReadData(&IO_UartRxBuffer, (UInt8*)rxBuffer, (available > length) ? length : available);
    HAL_ENABLE_GLOBAL_INT();

    return (available > length) ? length : available;
}
#endif // #ifndef GP_COM_DIVERSITY_NO_RX

/** @brief Initialize UART for use.
 *
*/
void qvIO_UartInit(void)
{
#ifndef GP_COM_DIVERSITY_NO_RX
    // Initialize Tx Circular buffer
    HAL_DISABLE_GLOBAL_INT();
    gpUtils_CircBInit(&IO_UartRxBuffer, IO_UartRxData, sizeof(IO_UartRxData));
    HAL_ENABLE_GLOBAL_INT();

    // Hook up callback
    gpCom_RegisterModule(GP_MODULE_ID, qvIO_cbRxData);
#endif // #ifndef GP_COM_DIVERSITY_NO_RX
}
