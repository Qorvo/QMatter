/*
 * Copyright (c) 2015-2016, GreenPeak Technologies
 * Copyright (c) 2017, Qorvo Inc
 *
 *   Hardware Abstraction Layer for the UART on K8C devices.
 *
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
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "hal.h"
#include "hal_DMA.h"
#include "gpBsp.h"
#include "gpHal_reg.h"
#include "gpAssert.h"

#define GP_COMPONENT_ID     GP_COMPONENT_ID_HALCORTEXM4

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define HAL_UART_NR_OF_UARTS            2

#if defined(HAL_UART_DMA_MASK)
/* if DMA mask is defined that applies to both TX/RX DMA */
#if defined(HAL_UART_TX_DMA_MASK) || defined(HAL_UART_RX_DMA_MASK)
#error "Redundant definition of UART TX/RX DMA masks"
#endif
#define HAL_UART_TX_DMA_MASK HAL_UART_DMA_MASK
#define HAL_UART_RX_DMA_MASK HAL_UART_DMA_MASK
#endif

#if !defined(HAL_UART_TX_DMA_MASK)
#define HAL_UART_TX_DMA_MASK 0
#endif

#if !defined(HAL_UART_RX_DMA_MASK)
#define HAL_UART_RX_DMA_MASK 0
#endif

#define NO_RX_TIMER 0xFF

#if defined(HAL_UART_DMA_RX_BACKOFF) && HAL_UART_DMA_RX_BACKOFF == 0
#undef HAL_UART_DMA_RX_BACKOFF
#endif
#if !defined(HAL_UART_DMA_RX_BACKOFF)
#define HAL_UART_DMA_RX_BACKOFF 16
#endif

#if defined(HAL_UART0_DMA_RX_TIMER) || defined(HAL_UART1_DMA_RX_TIMER) || defined(HAL_UART2_DMA_RX_TIMER)
    #define HAL_UART_USE_DMA_RX_TIMER 1
#endif

#if !defined(HAL_UART0_DMA_RX_TIMER)
#define HAL_UART0_DMA_RX_TIMER NO_RX_TIMER
#endif

#if !defined(HAL_UART1_DMA_RX_TIMER)
#define HAL_UART1_DMA_RX_TIMER NO_RX_TIMER
#endif

#if !defined(HAL_UART2_DMA_RX_TIMER)
#define HAL_UART2_DMA_RX_TIMER NO_RX_TIMER
#endif

#define HAL_UART_COM_SYMBOL_PERIOD (((HAL_UART_BAUDRATE_GENERATOR_CLOCK+(8*GP_BSP_UART_COM_BAUDRATE/2)) / (8*GP_BSP_UART_COM_BAUDRATE))-1)
GP_COMPILE_TIME_VERIFY(HAL_UART_COM_SYMBOL_PERIOD <=  0x0FFF);

#if defined(GP_BSP_UART_COM2_BAUDRATE)
#define HAL_UART_COM2_SYMBOL_PERIOD (((HAL_UART_BAUDRATE_GENERATOR_CLOCK+(8*GP_BSP_UART_COM2_BAUDRATE/2)) / (8*GP_BSP_UART_COM2_BAUDRATE))-1)
GP_COMPILE_TIME_VERIFY(HAL_UART_COM2_SYMBOL_PERIOD <=  0x0FFF);
#endif

#define HAL_UART_SCOM_SYMBOL_PERIOD (((HAL_UART_BAUDRATE_GENERATOR_CLOCK+(8*GP_BSP_UART_SCOM_BAUDRATE/2)) / (8*GP_BSP_UART_SCOM_BAUDRATE))-1)
GP_COMPILE_TIME_VERIFY(HAL_UART_SCOM_SYMBOL_PERIOD <=  0x0FFF);

#if !defined(HAL_UART_RX_BUFFER_SIZE)
#define HAL_UART_RX_BUFFER_SIZE   64U
#endif //if !defined(HAL_UART_RX_BUFFER_SIZE)

#define UART_BASE_ADDR_FROM_NR(UartNr) halUart_baseAddr[UartNr]

/* Return 1 if the specified UART can use DMA, otherwise return 0. */
#define HAL_UART_TX_USE_DMA(uart)      ( (HAL_UART_TX_DMA_MASK >> (uart)) & 1 )
#define HAL_UART_RX_USE_DMA(uart)      ( (HAL_UART_RX_DMA_MASK >> (uart)) & 1 )

/* Number of UARTs with DMA enabled. */
#define HAL_UART_TX_NR_UARTS_WITH_DMA  ( HAL_UART_TX_USE_DMA(0) + HAL_UART_TX_USE_DMA(1) + HAL_UART_TX_USE_DMA(2) )
#define HAL_UART_RX_NR_UARTS_WITH_DMA  ( HAL_UART_RX_USE_DMA(0) + HAL_UART_RX_USE_DMA(1) + HAL_UART_RX_USE_DMA(2) )

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

// suppress bogus warning when compiling with gcc 12.2
#if(__GNUC__ == 12 && __GNUC_MINOR__ == 2)
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif

static const size_t halUart_baseAddr[HAL_UART_NR_OF_UARTS] = {
    GP_WB_UART_0_BASE_ADDRESS,
#if HAL_UART_NR_OF_UARTS > 1
    GP_WB_UART_1_BASE_ADDRESS,
#endif
#if HAL_UART_NR_OF_UARTS > 2
    GP_WB_UART_2_BASE_ADDRESS
#endif
};

static UInt8 halUart_TxEnabledMask = 0;
static hal_cbUartTx_t hal_cbUartTx[HAL_UART_NR_OF_UARTS] = { NULL, };
#define hal_UartStoreTx(uart, cb) { hal_cbUartTx[(uart)] = (cb); }

#if (HAL_UART_RX_DMA_MASK != 0)
#if !defined(HAL_UART_NO_RX)
static hal_DmaChannel_t halUart_DmaRxChannel[HAL_UART_RX_NR_UARTS_WITH_DMA];
static UInt8            halUart_DmaRxBuffer[HAL_UART_RX_NR_UARTS_WITH_DMA][HAL_UART_RX_BUFFER_SIZE];
static hal_DmaPointer_t halUart_DmaRxReadPtr[HAL_UART_RX_NR_UARTS_WITH_DMA];
#if defined(HAL_UART_USE_DMA_RX_TIMER)
static const halTimer_timerId_t halUart_DmaRxTimer[HAL_UART_NR_OF_UARTS] = {
    HAL_UART0_DMA_RX_TIMER,
#if HAL_UART_NR_OF_UARTS > 1
    HAL_UART1_DMA_RX_TIMER,
#endif
#if HAL_UART_NR_OF_UARTS > 2
    HAL_UART2_DMA_RX_TIMER
#endif
};
#endif /* defined(HAL_UART_USE_DMA_RX_TIMER) */
#endif /* !defined(HAL_UART_NO_RX) */
#endif /* (HAL_UART_RX_DMA_MASK != 0) */

#if (HAL_UART_TX_DMA_MASK != 0)
static hal_DmaChannel_t halUart_DmaTxChannel[HAL_UART_TX_NR_UARTS_WITH_DMA];
static hal_DmaPointer_t halUart_DmaTxWritePtr[HAL_UART_TX_NR_UARTS_WITH_DMA];
#endif /* (HAL_UART_TX_DMA_MASK != 0) */

#ifndef HAL_UART_NO_RX
static UInt8 halUart_RxEnabledMask = 0;
static hal_cbUartRx_t hal_cbUartRx[HAL_UART_NR_OF_UARTS] = { NULL, };
#define hal_UartStoreRx(uart,cb)     {hal_cbUartRx[(uart)] = (cb);}
#else
#define hal_UartStoreRx(uart,cb)     NOT_USED(cb)
#endif  /* HAL_UART_NO_RX */

static hal_cbUartEot_t  hal_cbUartOneShotEndOfTx;

static struct {
    volatile const UInt8* txStatus;
    UInt16 txLength;
    UInt16 txPos;
    Bool flushed;
} halUart_txCtx[HAL_UART_NR_OF_UARTS];

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/* Write interrupt enable bit INT_CTRL_MASK_INT_UART_x_INTERRUPT */
static INLINE void halUart_SetIntCtrlMaskUartInterrupt(UInt8 uart, Bool enable)
{
    switch (uart) {
        case 0: GP_WB_WRITE_INT_CTRL_MASK_INT_UART_0_INTERRUPT(enable); break;
#if HAL_UART_NR_OF_UARTS > 1
        case 1: GP_WB_WRITE_INT_CTRL_MASK_INT_UART_1_INTERRUPT(enable); break;
#endif
#if HAL_UART_NR_OF_UARTS > 2
        case 2: GP_WB_WRITE_INT_CTRL_MASK_INT_UART_2_INTERRUPT(enable); break;
#endif
        default: GP_ASSERT_DEV_INT(false); break;
    }
}

/* Write interrupt enable bit INT_CTRL_MASK_UART_x_TX_NOT_FULL_INTERRUPT */
static INLINE void halUart_SetIntCtrlMaskUartTxNotFull(UInt8 uart, Bool enable)
{
    switch (uart) {
        case 0: GP_WB_WRITE_INT_CTRL_MASK_UART_0_TX_NOT_FULL_INTERRUPT(enable); break;
#if HAL_UART_NR_OF_UARTS > 1
        case 1: GP_WB_WRITE_INT_CTRL_MASK_UART_1_TX_NOT_FULL_INTERRUPT(enable); break;
#endif
#if HAL_UART_NR_OF_UARTS > 2
        case 2: GP_WB_WRITE_INT_CTRL_MASK_UART_2_TX_NOT_FULL_INTERRUPT(enable); break;
#endif
        default: GP_ASSERT_DEV_INT(false); break;
    }
}

/* Write interrupt enable bit GP_WB_WRITE_INT_CTRL_MASK_UART_x_RX_NOT_EMPTY_INTERRUPT */
static INLINE void halUart_SetIntCtrlMaskUartRxNotEmpty(UInt8 uart, Bool enable)
{
    switch(uart) {
        case 0: GP_WB_WRITE_INT_CTRL_MASK_UART_0_RX_NOT_EMPTY_INTERRUPT(enable); break;
#if HAL_UART_NR_OF_UARTS > 1
        case 1: GP_WB_WRITE_INT_CTRL_MASK_UART_1_RX_NOT_EMPTY_INTERRUPT(enable); break;
#endif
#if HAL_UART_NR_OF_UARTS > 2
        case 2: GP_WB_WRITE_INT_CTRL_MASK_UART_2_RX_NOT_EMPTY_INTERRUPT(enable); break;
#endif
        default: GP_ASSERT_DEV_INT(false); break;
    }
}

static INLINE void halUart_SetStandbyResetEpiUart(UInt8 uart, Bool enable)
{
    switch(uart) {
        case 0: GP_WB_WRITE_STANDBY_RESET_EPI_UART_0(enable); break;
#if HAL_UART_NR_OF_UARTS > 1
        case 1: GP_WB_WRITE_STANDBY_RESET_EPI_UART_1(enable); break;
#endif
#if HAL_UART_NR_OF_UARTS > 2
        case 2: GP_WB_WRITE_STANDBY_RESET_EPI_UART_2(enable); break;
#endif
        default: GP_ASSERT_DEV_INT(false); break;
    }
}

/* Enable or disable UART TX GPIO alternate function. */
static INLINE void halUart_SetUartTxGpioEnabled(UInt8 uart, Bool enable)
{
    switch (uart)
    {
        case 0: GP_BSP_UART0_TX_ENABLE(enable); break;
#if HAL_UART_NR_OF_UARTS > 1
        case 1: GP_BSP_UART1_TX_ENABLE(enable); break;
#endif
#if HAL_UART_NR_OF_UARTS > 2
        case 2: GP_BSP_UART2_TX_ENABLE(enable); break;
#endif
        default: GP_ASSERT_DEV_INT(false); break;
    }
}

/* Enable or disable UART RX GPIO alternate function. */
static INLINE void halUart_SetUartRxGpioEnabled(UInt8 uart, Bool enable)
{
    switch(uart) {
        case 0: GP_BSP_UART0_RX_ENABLE(enable); break;
#if HAL_UART_NR_OF_UARTS > 1
        case 1: GP_BSP_UART1_RX_ENABLE(enable); break;
#endif
#if HAL_UART_NR_OF_UARTS > 2
        case 2: GP_BSP_UART2_RX_ENABLE(enable); break;
#endif
        default: GP_ASSERT_DEV_INT(false); break;
    }
}

#if HAL_UART_TX_DMA_MASK != 0
/* Map UART number to index into the list of UART DMA channels. */
static INLINE UInt32 halUart_UartTxToDmaIndex(UInt8 uart)
{
    UInt32 idx = 0;
    if (uart > 0)
    {
        idx += HAL_UART_TX_USE_DMA(0);
    }
    if (uart > 1)
    {
        idx += HAL_UART_TX_USE_DMA(1);
    }
    return idx;
}
#endif

#if (HAL_UART_RX_DMA_MASK != 0) && !defined(HAL_UART_NO_RX)
/* Map UART number to index into the list of UART DMA channels. */
static INLINE UInt32 halUart_UartRxToDmaIndex(UInt8 uart)
{
    UInt32 idx = 0;
    if (uart > 0)
    {
        idx += HAL_UART_RX_USE_DMA(0);
    }
    if (uart > 1)
    {
        idx += HAL_UART_RX_USE_DMA(1);
    }
    return idx;
}
#endif

#ifndef HAL_UART_NO_RX
static void halUart_RxCheckErrors(UInt8 uart)
{
}

static void halUart_HandleIntRx(UInt8 uart)
{
    UInt32 uartBaseAddress;
    UInt8 data;

    halUart_RxCheckErrors(uart);

    uartBaseAddress = UART_BASE_ADDR_FROM_NR(uart);
    GP_ASSERT_DEV_INT(GP_WB_READ_UART_UNMASKED_RX_NOT_EMPTY_INTERRUPT(uartBaseAddress));
    data = GP_WB_READ_UART_RX_DATA_0(uartBaseAddress);
    GP_ASSERT_DEV_EXT(hal_cbUartRx[uart] != NULL);
#if defined(HAL_DIVERSITY_UART_RX_BUFFER_CALLBACK)
    hal_cbUartRx[uart](&data, 1);
#else
    hal_cbUartRx[uart](data);
#endif

    // Do not check or attempt to read multiple received bytes here.
    // - Reading multiple bytes is not needed. If another byte is available,
    //   uartX_handler_impl() will be called again to handle the new byte.
    // - Reading multiple bytes may leave UARTx_IRQ pending in NVIC without
    //   pending UART event. This triggers assert in uartX_handler_impl.
}

#if HAL_UART_RX_DMA_MASK != 0
static void halUart_RxHandleDma(UInt8 uart)
{
    GP_ASSERT_DEV_INT(HAL_UART_RX_USE_DMA(uart));
    UInt32 dmaIndex = halUart_UartRxToDmaIndex(uart);
    hal_DmaChannel_t dmaChannel = halUart_DmaRxChannel[dmaIndex];
    hal_DmaPointer_t readPtr;

    if (hal_cbUartRx[uart] == NULL)
    {
        return;
    }

    HAL_DISABLE_GLOBAL_INT();

    halUart_RxCheckErrors(uart);

    readPtr = halUart_DmaRxReadPtr[dmaIndex];
    do
    {
        hal_DmaPointer_t writePtr;
        UInt16 chunkSize;
        writePtr = hal_DmaGetInternalPointer(dmaChannel);
        if (HAL_DMA_POINTERS_EQUAL(writePtr,readPtr))
        {
            break;
        }
        chunkSize = hal_DmaBuffer_GetNextContinuousSize(writePtr, readPtr, HAL_UART_RX_BUFFER_SIZE);

#if defined(HAL_DIVERSITY_UART_RX_BUFFER_CALLBACK)
        hal_cbUartRx[uart](&halUart_DmaRxBuffer[dmaIndex][readPtr.offset], chunkSize);
#else
        UInt16 i;
        for (i = 0; i < chunkSize; i++)
        {
            hal_cbUartRx[uart](halUart_DmaRxBuffer[dmaIndex][readPtr.offset + i]);
        }
#endif

        readPtr.offset += chunkSize;
        if (readPtr.offset == HAL_UART_RX_BUFFER_SIZE)
        {
            readPtr.wrap = !readPtr.wrap;
            readPtr.offset = 0;
        }

#if(HAL_UART_RX_DMA_MASK != 0) && !defined(HAL_UART_NO_RX) && defined(HAL_UART_USE_DMA_RX_TIMER)
        if (halUart_DmaRxTimer[uart] != NO_RX_TIMER)
        {
            UInt32 dmaBaseAddress = HAL_DMA_GET_DMA_BASE(dmaChannel);
            UInt8 new_threshold = HAL_UART_DMA_RX_BACKOFF;
            GP_WB_WRITE_DMA_BUFFER_ALMOST_COMPLETE_THRESHOLD(dmaBaseAddress, new_threshold + 1);

            halTimer_resetTimer(halUart_DmaRxTimer[uart]);
            halTimer_startTimer(halUart_DmaRxTimer[uart]);
            break;
        }
#endif
    } while (true);

    halUart_DmaRxReadPtr[dmaIndex] = readPtr;
    hal_DmaUpdatePointers(dmaChannel, readPtr);

    HAL_ENABLE_GLOBAL_INT();
}

#if defined(HAL_UART_USE_DMA_RX_TIMER)
static void halUart_fallbackTimerHandler(UInt8 uart)
{
    if(!HAL_UART_RX_USE_DMA(uart))
    {
        return;
    }

    // RX timeout. Pull the data waiting in RX buffer.
    UInt32 dmaIndex = halUart_UartRxToDmaIndex(uart);
    hal_DmaChannel_t dmaChannel = halUart_DmaRxChannel[dmaIndex];
    halUart_RxHandleDma(uart);

    // Reset RX threshold back to a single byte and disable RX timer.
    UInt32 dmaBaseAddress = HAL_DMA_GET_DMA_BASE(dmaChannel);
    GP_WB_WRITE_DMA_BUFFER_ALMOST_COMPLETE_THRESHOLD(dmaBaseAddress, 0);
    halTimer_stopTimer(halUart_DmaRxTimer[uart]);
}

static void halUart_Uart0fallbackTimerHandler(void)
{
    halUart_fallbackTimerHandler(0);
}

#if HAL_UART_NR_OF_UARTS > 1
static void halUart_Uart1fallbackTimerHandler(void)
{
    halUart_fallbackTimerHandler(1);
}
#endif

#if HAL_UART_NR_OF_UARTS > 2
static void halUart_Uart2fallbackTimerHandler(void)
{
    halUart_fallbackTimerHandler(2);
}
#endif
#endif

/* Called from DMA interrupt handler when number of bytes in buffer exceeds threshold. */
static void halUart_cbDmaBufferAlmostComplete(hal_DmaChannel_t dmaChannel)
{
    UInt8 uart;
    for (uart = 0; uart < HAL_UART_NR_OF_UARTS; uart++)
    {
        if (HAL_UART_RX_USE_DMA(uart) && halUart_DmaRxChannel[halUart_UartRxToDmaIndex(uart)] == dmaChannel)
        {
            halUart_RxHandleDma(uart);
            return;
        }
    }

    // Got DMA callback for unknown DMA channel.
    GP_ASSERT_DEV_INT(false);
}

static void halUart_RxEnableDma(UInt8 uart)
{
    GP_ASSERT_DEV_INT(HAL_UART_RX_USE_DMA(uart));

    UInt32 dmaIndex = halUart_UartRxToDmaIndex(uart);

    MEMSET(&halUart_DmaRxReadPtr[dmaIndex],0,sizeof(hal_DmaPointer_t));

    hal_DmaDescriptor_t dmaDesc;
    MEMSET(&dmaDesc, 0, sizeof(dmaDesc));
    dmaDesc.channel = halUart_DmaRxChannel[dmaIndex];
    dmaDesc.cbAlmostComplete = halUart_cbDmaBufferAlmostComplete;
    dmaDesc.cbComplete = NULL;
    dmaDesc.wordMode = GP_WB_ENUM_DMA_WORD_MODE_BYTE;
    dmaDesc.bufferSize = HAL_UART_RX_BUFFER_SIZE;
    dmaDesc.circBufSel = GP_WB_ENUM_CIRCULAR_BUFFER_DEST_BUFFER;
    dmaDesc.srcAddrInRam = false;
    dmaDesc.destAddr = (UInt32) halUart_DmaRxBuffer[dmaIndex];
    dmaDesc.destAddrInRam = true;
    dmaDesc.bufCompleteIntMode = GP_WB_ENUM_DMA_BUFFER_COMPLETE_MODE_ERROR_MODE;

    // Get notification as soon as at least 1 character is pending in the buffer.
    dmaDesc.threshold = 1;

    switch (uart)
    {
        case 0:
            dmaDesc.srcAddr = GP_WB_UART_0_RX_DATA_0_ADDRESS;
            dmaDesc.dmaTriggerSelect = GP_WB_ENUM_DMA_TRIGGER_SRC_SELECT_UART_0_RX_NOT_EMPTY;
            break;
#if HAL_UART_NR_OF_UARTS > 1
        case 1:
            dmaDesc.srcAddr = GP_WB_UART_1_RX_DATA_0_ADDRESS;
            dmaDesc.dmaTriggerSelect = GP_WB_ENUM_DMA_TRIGGER_SRC_SELECT_UART_1_RX_NOT_EMPTY;
            break;
#endif
#if HAL_UART_NR_OF_UARTS > 2
        case 2:
            dmaDesc.srcAddr = GP_WB_UART_2_RX_DATA_0_ADDRESS;
            dmaDesc.dmaTriggerSelect = GP_WB_ENUM_DMA_TRIGGER_SRC_SELECT_UART_2_RX_NOT_EMPTY;
            break;
#endif
        default:
            GP_ASSERT_DEV_INT(false);
            break;
    }

    hal_DmaResult_t result = hal_DmaStart(&dmaDesc);
    GP_ASSERT_SYSTEM(result == HAL_DMA_RESULT_SUCCESS);
}

static void halUart_RxDisableDma(UInt8 uart)
{
    GP_ASSERT_DEV_INT(HAL_UART_RX_USE_DMA(uart));

    UInt32 dmaIndex = halUart_UartRxToDmaIndex(uart);
    hal_DmaResult_t result = hal_DmaStop(halUart_DmaRxChannel[dmaIndex]);
    GP_ASSERT_SYSTEM(result == HAL_DMA_RESULT_SUCCESS);
}

#endif // HAL_UART_DMA_MASK != 0
#endif // HAL_UART_NO_RX

#if (HAL_UART_TX_DMA_MASK != 0)

static void halUart_TxCbDmaComplete(hal_DmaChannel_t channel)
{
    UInt8 uart;

    for(uart = 0U; uart < HAL_UART_NR_OF_UARTS; uart++)
    {
        if(HAL_UART_TX_USE_DMA(uart) && halUart_DmaTxChannel[halUart_UartTxToDmaIndex(uart)] == channel)
        {
            hal_DmaEnableCompleteInterruptMask(channel, false);

            // Clear busy flag before calling user callback, so the next transfer
            // could be triggered from there.
            Bool call_cb = (halUart_txCtx[uart].txStatus && hal_cbUartTx[uart]) ? true : false;
            const UInt8* buffer = (const UInt8*) halUart_txCtx[uart].txStatus;
            halUart_txCtx[uart].txStatus = NULL;

            if(call_cb)
            {
                UInt16 bytesTransferred = halUart_txCtx[uart].txLength;
                halUart_txCtx[uart].txLength = 0;
                hal_cbUartTx[uart](buffer, bytesTransferred);
            }

            return;
        }
    }

    // Unknown instance. This should not happen.
    GP_ASSERT_DEV_INT(false);
}

static void halUart_TxDisableDma(UInt8 uart)
{
    GP_ASSERT_DEV_INT(HAL_UART_TX_USE_DMA(uart));

    UInt32 dmaIndex = halUart_UartTxToDmaIndex(uart);
    hal_DmaResult_t result = hal_DmaStop(halUart_DmaTxChannel[dmaIndex]);
    GP_ASSERT_SYSTEM(result == HAL_DMA_RESULT_SUCCESS);
}

static void halUart_TxEnableDma(UInt8 uart)
{
    hal_DmaDescriptor_t dmaDesc;
    hal_DmaResult_t result;
    UInt32 dmaIndex;

    GP_ASSERT_DEV_INT(HAL_UART_TX_USE_DMA(uart));
    dmaIndex = halUart_UartTxToDmaIndex(uart);

    MEMSET(&halUart_DmaTxWritePtr[dmaIndex], 0, sizeof(hal_DmaPointer_t));
    MEMSET(&dmaDesc, 0, sizeof(dmaDesc));

    dmaDesc.channel = halUart_DmaTxChannel[dmaIndex];
    dmaDesc.cbComplete = halUart_TxCbDmaComplete;
    dmaDesc.wordMode = GP_WB_ENUM_DMA_WORD_MODE_BYTE;
    dmaDesc.circBufSel = GP_WB_ENUM_CIRCULAR_BUFFER_SRC_BUFFER;
    // This is a dummy address which will be set when TX requested,
    // but the driver requires specifying valid address in RAM space.
    dmaDesc.srcAddr = (UInt32) &halUart_TxEnabledMask;
    dmaDesc.srcAddrInRam = true;
    dmaDesc.bufferSize = 2; // Dummy size. This will be altered when TX is requested.
    dmaDesc.threshold = 0;
    dmaDesc.bufCompleteIntMode = GP_WB_ENUM_DMA_BUFFER_COMPLETE_MODE_STATUS_MODE;
    dmaDesc.writePtr.offset = 0;
    dmaDesc.writePtr.wrap = 0;

    switch (uart)
    {
        case 0:
            dmaDesc.destAddr = GP_WB_UART_0_TX_DATA_0_ADDRESS;
            dmaDesc.dmaTriggerSelect = GP_WB_ENUM_DMA_TRIGGER_SRC_SELECT_UART_0_TX_NOT_FULL;
            break;
#if HAL_UART_NR_OF_UARTS > 1
        case 1:
            dmaDesc.destAddr = GP_WB_UART_1_TX_DATA_0_ADDRESS;
            dmaDesc.dmaTriggerSelect = GP_WB_ENUM_DMA_TRIGGER_SRC_SELECT_UART_1_TX_NOT_FULL;
            break;
#endif
#if HAL_UART_NR_OF_UARTS > 2
        case 2:
            dmaDesc.destAddr = GP_WB_UART_2_TX_DATA_0_ADDRESS;
            dmaDesc.dmaTriggerSelect = GP_WB_ENUM_DMA_TRIGGER_SRC_SELECT_UART_2_TX_NOT_FULL;
            break;
#endif
        default:
            GP_ASSERT_DEV_INT(false);
            break;
    }

    /* Disabling interrupts till dma complete interrupt is disabled
    otherwise it will get stuck in interrupt handling */
    HAL_DISABLE_GLOBAL_INT();
    result = hal_DmaStart(&dmaDesc);
    GP_ASSERT_SYSTEM(result == HAL_DMA_RESULT_SUCCESS);

    // disabling buffer complete interrupts since we use the almost
    // complete interrupt to re-fill buffer
    // while flushing buffer we check the unmasked buffer complete
    // interrupt to make sure all data is emptied
    hal_DmaEnableAlmostCompleteInterruptMask(dmaDesc.channel, false);
    hal_DmaEnableCompleteInterruptMask(dmaDesc.channel, false);
    HAL_ENABLE_GLOBAL_INT();
}
#endif

static void halUart_HandleIntTxData(UInt8 uart)
{
    if(!halUart_txCtx[uart].txStatus)
    {
        // Unexpected Interrupt. Ignore it if there is no transfer in progress.
        return;
    }

    if(halUart_txCtx[uart].txPos >= halUart_txCtx[uart].txLength)
    {
        // Transfer complete. Clear busy flag before calling user callback,
        // so the next transfer could be triggered from there.
        halUart_SetIntCtrlMaskUartTxNotFull(uart, false);
        Bool call_cb = hal_cbUartTx[uart] ? true : false;
        const UInt8* buffer = (const UInt8*) halUart_txCtx[uart].txStatus;
        halUart_txCtx[uart].txStatus = NULL;

        if(call_cb)
        {
            UInt16 bytesTransferred = halUart_txCtx[uart].txLength;
            halUart_txCtx[uart].txLength = 0;
            hal_cbUartTx[uart](buffer, bytesTransferred);
        }

        return;
    }

    UInt8 dataToTx = halUart_txCtx[uart].txStatus[halUart_txCtx[uart].txPos];
    ++halUart_txCtx[uart].txPos;

    switch(uart)
    {
        case 0: GP_WB_WRITE_UART_0_TX_DATA_0(dataToTx); break;
#if HAL_UART_NR_OF_UARTS > 1
        case 1: GP_WB_WRITE_UART_1_TX_DATA_0(dataToTx); break;
#endif
#if HAL_UART_NR_OF_UARTS > 2
        case 2: GP_WB_WRITE_UART_2_TX_DATA_0(dataToTx); break;
#endif
        default: GP_ASSERT_DEV_INT(false); break;
    }
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
void hal_InitUart(void)
{
#if (HAL_UART_RX_DMA_MASK != 0) && !defined(HAL_UART_NO_RX)
    MEMSET(halUart_DmaRxChannel,HAL_DMA_CHANNEL_INVALID,sizeof(halUart_DmaRxChannel));
#endif
#if (HAL_UART_TX_DMA_MASK != 0)
    MEMSET(halUart_DmaTxChannel,HAL_DMA_CHANNEL_INVALID,sizeof(halUart_DmaTxChannel));
#endif

    halUart_TxEnabledMask = 0;

#ifndef HAL_UART_NO_RX
    halUart_RxEnabledMask = 0;
#endif
}

void hal_UartStart(hal_cbUartRx_t cbRx, hal_cbUartTx_t cbTx, UInt16 symbolPeriod, UInt16 flags, UInt8 uart)
{
    GP_ASSERT_SYSTEM(uart < HAL_UART_NR_OF_UARTS);

    UInt8 stopbits = (flags >> 2) & 0x03;
    UInt8 parity   = (flags >> 0) & 0x03;
    UInt8 databits = (flags >> 4) & 0x0F;
    UInt32 UartBaseAddress = UART_BASE_ADDR_FROM_NR(uart);

#ifdef GP_BSP_UART_INIT
    if (uart == 0)
    {
        GP_BSP_UART_INIT();
    }
#endif

#if (HAL_UART_RX_DMA_MASK != 0) && !defined(HAL_UART_NO_RX)
    if (HAL_UART_RX_USE_DMA(uart))
    {
        if (halUart_DmaRxChannel[halUart_UartRxToDmaIndex(uart)]==HAL_DMA_CHANNEL_INVALID)
        {
            halUart_DmaRxChannel[halUart_UartRxToDmaIndex(uart)] = hal_DmaClaim();
        }

#if defined(HAL_UART_USE_DMA_RX_TIMER)
        halTimer_timerId_t timerIdx = halUart_DmaRxTimer[uart];

        if (timerIdx != NO_RX_TIMER)
        {
            halTimer_cbTimerWrapInterruptHandler_t cbTimer = NULL;

            switch(uart)
            {
                case 0:
                    cbTimer = halUart_Uart0fallbackTimerHandler;
                    break;
#if HAL_UART_NR_OF_UARTS > 1
                case 1:
                    cbTimer = halUart_Uart1fallbackTimerHandler;
                    break;
#endif
#if HAL_UART_NR_OF_UARTS > 2
                case 2:
                    cbTimer = halUart_Uart2fallbackTimerHandler;
                    break;
#endif
                default:
                    GP_ASSERT_DEV_INT(false);
                    break;
            }

            // According to datasheet, symbol period + 1 is equal to time in microseconds/2.
            // The timeout window is set to 150% of transfer time of HAL_UART_DMA_RX_BACKOFF
            // (10 symbols per byte).
            UInt32 timeout_us = ((1UL + symbolPeriod) * HAL_UART_DMA_RX_BACKOFF) / 2UL * 15UL;
            UInt8 prescaler = 4;

            // Increase the prescaler if timer threshold value exceeds 16-bit value.
            while ((timeout_us > 0xFFFF) && (prescaler < 7))
            {
                timeout_us >>= 1;
                ++prescaler;
            }

            if (timeout_us > 0xFFFF)
            {
                timeout_us = 0xFFFF;
            }

            halTimer_initTimer(timerIdx, prescaler, GP_WB_ENUM_TMR0_CLK_SEL_INT_CLK,
                               (UInt16)timeout_us, cbTimer, true);
        }
#endif
    }
#endif

#if (HAL_UART_TX_DMA_MASK != 0)
    if (HAL_UART_TX_USE_DMA(uart))
    {
        if (halUart_DmaTxChannel[halUart_UartTxToDmaIndex(uart)]==HAL_DMA_CHANNEL_INVALID)
        {
            halUart_DmaTxChannel[halUart_UartTxToDmaIndex(uart)] = hal_DmaClaim();
        }
    }
#endif

    hal_UartStoreRx(uart, cbRx);
    hal_UartStoreTx(uart, cbTx);
    hal_cbUartOneShotEndOfTx = (hal_cbUartEot_t) NULL;

    GP_WB_WRITE_UART_BAUD_RATE(UartBaseAddress, symbolPeriod);

    //we only support up to one byte for now.
    GP_ASSERT_DEV_EXT(databits <= 8);
    //GP_LOG_SYSTEM_PRINTF("bits: %i",2, databits);
    //GP_LOG_SYSTEM_PRINTF("parity: %i",2, parity);
    //GP_LOG_SYSTEM_PRINTF("stop: %i",2, stopbits);

    GP_WB_WRITE_UART_DATA_BITS(UartBaseAddress, databits-1);
    GP_WB_WRITE_UART_PARITY(UartBaseAddress, parity);
    GP_WB_WRITE_UART_STOP_BITS(UartBaseAddress, stopbits-1);

    hal_UartEnable(uart);
}
void hal_UartSetClockDivider(UInt8 uart, UInt16 value)
{
    GP_ASSERT_DEV_INT(uart < HAL_UART_NR_OF_UARTS);

    UInt32 UartBaseAddress = UART_BASE_ADDR_FROM_NR(uart);
    GP_WB_WRITE_UART_BAUD_RATE(UartBaseAddress, value);
}
UInt16 hal_UartGetClockDivider(UInt8 uart)
{
    GP_ASSERT_DEV_INT(uart < HAL_UART_NR_OF_UARTS);

    UInt32 UartBaseAddress = UART_BASE_ADDR_FROM_NR(uart);
    return GP_WB_READ_UART_BAUD_RATE(UartBaseAddress);
}

void hal_UartComStart(hal_cbUartRx_t cbRx, hal_cbUartTx_t cbTx, UInt8 uart)
{
    hal_UartStart(cbRx, cbTx,
#if defined(GP_BSP_UART_COM2_BAUDRATE)
                  (uart == GP_BSP_UART_COM1) ? HAL_UART_COM_SYMBOL_PERIOD : HAL_UART_COM2_SYMBOL_PERIOD,
#else
                  HAL_UART_COM_SYMBOL_PERIOD,
#endif
                  (HAL_UART_OPT_8_BITS_PER_CHAR | HAL_UART_OPT_NO_PARITY | HAL_UART_OPT_ONE_STOP_BIT), uart);
}
void hal_UartSComStart(hal_cbUartRx_t cbRx, hal_cbUartTx_t cbTx)
{
    hal_UartStart(cbRx, cbTx, HAL_UART_SCOM_SYMBOL_PERIOD,
                  (HAL_UART_OPT_8_BITS_PER_CHAR | HAL_UART_OPT_NO_PARITY | HAL_UART_OPT_ONE_STOP_BIT), 0);
}

void hal_UartComStop(UInt8 uart)
{
    /* this function is used to deinitialize before handover between stage2 bootloader and application */
    hal_UartDisable(uart);

    hal_UartStoreRx(uart, NULL);
    hal_UartStoreTx(uart, NULL);
    hal_cbUartOneShotEndOfTx = (hal_cbUartEot_t) NULL;

    /* can't call hal_Dma Release -- not implemented */
}

void hal_UartDisable(UInt8 uart)
{
    halUart_SetIntCtrlMaskUartInterrupt(uart, false);

    switch (uart) {
        case 0: NVIC_DisableIRQ(UART0_IRQn); break;
#if HAL_UART_NR_OF_UARTS > 1
        case 1: NVIC_DisableIRQ(UART1_IRQn); break;
#endif
#if HAL_UART_NR_OF_UARTS > 2
        case 2: NVIC_DisableIRQ(UART2_IRQn); break;
#endif
        default: GP_ASSERT_DEV_INT(false); break;
    }

    //Flush remaing bytes
    hal_UartComFlush(uart);
    hal_UartRxComFlush(uart);

    //Disconnect pins and mapping
    halUart_SetIntCtrlMaskUartTxNotFull(uart, false);
    halUart_SetUartTxGpioEnabled(uart, false);
#if (HAL_UART_TX_DMA_MASK != 0)
    if (HAL_UART_TX_USE_DMA(uart) && BIT_TST(halUart_TxEnabledMask, uart))
    {
        halUart_TxDisableDma(uart);
    }
#endif
    BIT_CLR(halUart_TxEnabledMask, uart);

#ifndef HAL_UART_NO_RX
#if HAL_UART_RX_DMA_MASK != 0
    if (HAL_UART_RX_USE_DMA(uart) && BIT_TST(halUart_RxEnabledMask, uart))
    {
        halUart_RxDisableDma(uart);
    }
#endif

    halUart_SetIntCtrlMaskUartRxNotEmpty(uart, false);
    halUart_SetUartRxGpioEnabled(uart, false);

    GP_WB_WRITE_UART_RX_ENABLE(UART_BASE_ADDR_FROM_NR(uart), false);

    BIT_CLR(halUart_RxEnabledMask, uart);
#endif
}

void hal_UartEnable(UInt8 uart)
{
    GP_ASSERT_DEV_INT(uart < HAL_UART_NR_OF_UARTS);

#ifndef HAL_UART_NO_RX
    Bool rxDefined = false;
    Bool useRxDma = false;
#endif
    Bool txDefined = false;
    UInt32 UartBaseAddress = UART_BASE_ADDR_FROM_NR(uart);
    Bool useTxDma = false;

    if (HAL_UART_TX_USE_DMA(uart))
    {
        useTxDma = true;
    }
#ifndef HAL_UART_NO_RX
    if (HAL_UART_RX_USE_DMA(uart))
    {
        useRxDma = true;
    }
#endif
    GP_WB_UART_CLR_RX_OVERRUN_INTERRUPT(UartBaseAddress);
    GP_WB_UART_CLR_RX_PARITY_ERROR_INTERRUPT(UartBaseAddress);
    GP_WB_UART_CLR_RX_FRAMING_ERROR_INTERRUPT(UartBaseAddress);
    (void) GP_WB_READ_UART_RX_DATA_0(UartBaseAddress);

    /* UART TX */
    switch (uart)
    {
        case 0: txDefined = GP_BSP_UART0_TX_DEFINED(); break;
#if HAL_UART_NR_OF_UARTS > 1
        case 1: txDefined = GP_BSP_UART1_TX_DEFINED(); break;
#endif
#if HAL_UART_NR_OF_UARTS > 2
        case 2: txDefined = GP_BSP_UART2_TX_DEFINED(); break;
#endif
        default: GP_ASSERT_DEV_INT(false); break;
    }

    //Put UART block in reset during configuration
    halUart_SetStandbyResetEpiUart(uart, true);

    if (!txDefined)
    {
        /* no TX will be used, why explicitly disable-> in case of reinit? */
        halUart_SetUartTxGpioEnabled(uart, false);
        halUart_SetIntCtrlMaskUartTxNotFull(uart, false);
        BIT_CLR(halUart_TxEnabledMask, uart);
    }
    else
    {
        /* TX int enable: will be enabled when data needs to be sent */

        /* configure GPIO */
        switch(uart)
        {
            case 0: GP_BSP_UART0_TX_GPIO_CFG(); break;
#if HAL_UART_NR_OF_UARTS > 1
            case 1: GP_BSP_UART1_TX_GPIO_CFG(); break;
#endif
#if HAL_UART_NR_OF_UARTS > 2
            case 2: GP_BSP_UART2_TX_GPIO_CFG(); break;
#endif
            default: GP_ASSERT_DEV_INT(false); break;
        }
        halUart_SetUartTxGpioEnabled(uart, true);
        BIT_SET(halUart_TxEnabledMask, uart);
    }

#if (HAL_UART_TX_DMA_MASK != 0)
    if (txDefined && useTxDma)
    {
        halUart_TxEnableDma(uart);
    }
#endif

#ifndef HAL_UART_NO_RX
    /* UART RX */

    switch (uart)
    {
        case 0: rxDefined = GP_BSP_UART0_RX_DEFINED(); break;
#if HAL_UART_NR_OF_UARTS > 1
        case 1: rxDefined = GP_BSP_UART1_RX_DEFINED(); break;
#endif
#if HAL_UART_NR_OF_UARTS > 2
        case 2: rxDefined = GP_BSP_UART2_RX_DEFINED(); break;
#endif
        default: GP_ASSERT_DEV_INT(false); break;
    }

    //Rx interrupt should be enabled if there is a handler, tx not yet.
    if ((hal_cbUartRx[uart] != NULL) && rxDefined)
    {
        if (useRxDma)
        {
#if HAL_UART_RX_DMA_MASK != 0
            halUart_RxEnableDma(uart);
#endif
        }

        switch (uart) {
            case 0: GP_BSP_UART0_RX_GPIO_CFG(); break;
#if HAL_UART_NR_OF_UARTS > 1
            case 1: GP_BSP_UART1_RX_GPIO_CFG(); break;
#endif
#if HAL_UART_NR_OF_UARTS > 2
            case 2: GP_BSP_UART2_RX_GPIO_CFG(); break;
#endif
            default: GP_ASSERT_DEV_INT(false); break;
        }
        if (!useRxDma)
        {
            halUart_SetIntCtrlMaskUartRxNotEmpty(uart, true);
        }
        halUart_SetUartRxGpioEnabled(uart, true);
        BIT_SET(halUart_RxEnabledMask, uart);
        //Enable when reset of block is lifted
    }
    else
#endif  /* HAL_UART_NO_RX */
    {
        GP_WB_WRITE_UART_RX_ENABLE(UartBaseAddress, false);

        halUart_SetIntCtrlMaskUartRxNotEmpty(uart, false);
        if (uart == 1)
        {
        }
        else
        {
            halUart_SetUartRxGpioEnabled(uart, false);
        }
    }

    /* Enable UART block mask */
    halUart_SetIntCtrlMaskUartInterrupt(uart, true);

    if ((txDefined && !useTxDma)
#ifndef HAL_UART_NO_RX
       || ((hal_cbUartRx[uart] != NULL) && rxDefined && !useRxDma)
#endif  /* HAL_UART_NO_RX */
       )
    {
        switch(uart) {
            case 0: NVIC_EnableIRQ(UART0_IRQn); break;
#if HAL_UART_NR_OF_UARTS > 1
            case 1: NVIC_EnableIRQ(UART1_IRQn); break;
#endif
#if HAL_UART_NR_OF_UARTS > 2
            case 2: NVIC_EnableIRQ(UART2_IRQn); break;
#endif
            default: GP_ASSERT_DEV_INT(false); break;
        }
    }

    //Release reset of block
    halUart_SetStandbyResetEpiUart(uart, false);

#ifndef HAL_UART_NO_RX
    if ((hal_cbUartRx[uart] != NULL) && rxDefined)
    {
        //Enable Rx when UART block is out of reset
        GP_WB_WRITE_UART_RX_ENABLE(UartBaseAddress, true);
    }
#endif  /* HAL_UART_NO_RX */
}

Bool hal_UartTxEnabled(UInt8 uart)
{
    GP_ASSERT_DEV_INT(uart < HAL_UART_NR_OF_UARTS);
    return (uart < HAL_UART_NR_OF_UARTS) && BIT_TST(halUart_TxEnabledMask, uart);
}

Bool hal_UartRxEnabled(UInt8 uart)
{
#ifndef HAL_UART_NO_RX
    GP_ASSERT_DEV_INT(uart < HAL_UART_NR_OF_UARTS);
    return (uart < HAL_UART_NR_OF_UARTS) && BIT_TST(halUart_RxEnabledMask, uart);
#else
    return false;
#endif  /* HAL_UART_NO_RX */
}

Bool hal_UartTx(UInt8 uart, const UInt8* data, UInt16 length)
{
    GP_ASSERT_DEV_INT(uart < HAL_UART_NR_OF_UARTS);

    if (halUart_txCtx[uart].txStatus || !length)
    {
        // Transmission currently in progress or invalid length.
        return false;
    }

    halUart_txCtx[uart].txStatus = data;
    halUart_txCtx[uart].txLength = length;
    halUart_txCtx[uart].txPos = 0;

#if (HAL_UART_TX_DMA_MASK != 0)
    if (HAL_UART_TX_USE_DMA(uart))
    {
        UInt32 dmaIndex = halUart_UartTxToDmaIndex(uart);
        hal_DmaChannel_t dmaChannel = halUart_DmaTxChannel[dmaIndex];
        UInt32 dmaBaseAddress = HAL_DMA_GET_DMA_BASE(dmaChannel);

        GP_WB_DMA_RESET_POINTERS(dmaBaseAddress);
        if (length > 1)
        {
            GP_WB_WRITE_DMA_SRC_ADDR(dmaBaseAddress, GP_MM_RAM_ADDR_TO_COMPRESSED((UInt32)data));
            GP_WB_WRITE_DMA_BUFFER_SIZE(dmaBaseAddress, length - 1);
            hal_DmaPointer_t wptr = {.offset = 0, .wrap = 1};
            hal_DmaUpdatePointers(dmaChannel, wptr);
        }
        else
        {
            // DMA controller does not allow 1-byte transfers, so the workaround must be applied.
            // Buffer size is set to 2 and read pointer is shifted by, so the next transfer will
            // result in a single byte being transmitted.
            GP_WB_WRITE_DMA_SRC_ADDR(dmaBaseAddress, GP_MM_RAM_ADDR_TO_COMPRESSED((UInt32)data - 1));
            GP_WB_WRITE_DMA_BUFFER_SIZE(dmaBaseAddress, 1);
            GP_WB_WRITE_DMA_BUFFER_PTR_VALUE(dmaBaseAddress, 1);
            GP_WB_WRITE_DMA_BUFFER_PTR_WRAP_VALUE(dmaBaseAddress, 1);
            GP_WB_DMA_SET_READ_PTR(dmaBaseAddress);
        }

        hal_DmaEnableCompleteInterruptMask(dmaChannel, true);
        return true;
    }
#endif

    halUart_SetIntCtrlMaskUartTxNotFull(uart, true);
    return true;
}

Bool hal_UartTxBusy(UInt8 uart)
{
    GP_ASSERT_DEV_INT(uart < HAL_UART_NR_OF_UARTS);
    return halUart_txCtx[uart].txStatus != NULL;
}

void hal_UartWaitEndOfTransmission(UInt8 uart)
{
    GP_ASSERT_DEV_INT(uart < HAL_UART_NR_OF_UARTS);
    if (uart < HAL_UART_NR_OF_UARTS)
    {
        UInt32 UartBaseAddress = UART_BASE_ADDR_FROM_NR(uart);
        while (!GP_WB_READ_UART_UNMASKED_TX_NOT_BUSY_INTERRUPT(UartBaseAddress)) { }
    }
}

void hal_UartRegisterOneShotEndOfTxCb(hal_cbUartEot_t cbEot)
{
    GP_WB_WRITE_INT_CTRL_MASK_UART_0_TX_NOT_BUSY_INTERRUPT(false);
    hal_cbUartOneShotEndOfTx = cbEot;
    GP_WB_WRITE_INT_CTRL_MASK_UART_0_TX_NOT_BUSY_INTERRUPT(true);
}

static Bool hal_UartTxFlushFunc(UInt8 uart)
{
    if (halUart_txCtx[uart].txStatus != NULL)
    {
        Bool txNotFull = false;
        switch(uart)
        {
            case 0: txNotFull = (GP_WB_READ_UART_0_UNMASKED_TX_NOT_FULL_INTERRUPT() && GP_WB_READ_UART_0_UNMASKED_TX_NOT_BUSY_INTERRUPT()); break;
    #if HAL_UART_NR_OF_UARTS > 1
            case 1: txNotFull = (GP_WB_READ_UART_1_UNMASKED_TX_NOT_FULL_INTERRUPT() && GP_WB_READ_UART_1_UNMASKED_TX_NOT_BUSY_INTERRUPT()); break;
    #endif
    #if HAL_UART_NR_OF_UARTS > 2
            case 2: txNotFull = (GP_WB_READ_UART_2_UNMASKED_TX_NOT_FULL_INTERRUPT() && GP_WB_READ_UART_2_UNMASKED_TX_NOT_BUSY_INTERRUPT()); break;
    #endif
            default: GP_ASSERT_DEV_INT(false); break;
        }

        if(txNotFull)
        {
            halUart_HandleIntTxData(uart);
        }
    }

    return halUart_txCtx[uart].txStatus != NULL;
}

void hal_UartComFlush(UInt8 uart)
{
    // Wait in a loop and keep feeding WDT (GP_DO_WHILE_TIMEOUT_ASSERT calls HAL_WDT_RESET).
    GP_DO_WHILE_TIMEOUT_ASSERT(hal_UartTxFlushFunc(uart), 200000);
    hal_UartWaitEndOfTransmission(uart);
    halUart_txCtx[uart].flushed = true;
}

void hal_UartRxComFlush(UInt8 uart)
{
#if !defined(HAL_UART_NO_RX) && (HAL_UART_RX_DMA_MASK != 0)
    GP_ASSERT_DEV_INT(uart < HAL_UART_NR_OF_UARTS);
    if (HAL_UART_RX_USE_DMA(uart) && hal_UartRxEnabled(uart))
    {
        halUart_RxHandleDma(uart);
    }
#endif
}

/* Called before going to sleep to stop DMA. */
void hal_UartBeforeSleep(void)
{
    UInt8 uart;

    for (uart = 0; uart < HAL_UART_NR_OF_UARTS; uart++)
    {
#if (HAL_UART_RX_DMA_MASK != 0) && !defined(HAL_UART_NO_RX)
        if (HAL_UART_RX_USE_DMA(uart))
        {
            if (hal_UartRxEnabled(uart))
            {
                halUart_RxDisableDma(uart);
                halUart_RxHandleDma(uart);
            }
        }
#endif

#if (HAL_UART_TX_DMA_MASK != 0)
        if (HAL_UART_TX_USE_DMA(uart))
        {
            if (hal_UartTxEnabled(uart))
            {
                hal_UartComFlush(uart);
                halUart_TxDisableDma(uart);
            }
        }
#endif
    }
}

/* Called after waking up from sleep to restart DMA. */
void hal_UartAfterSleep(void)
{
    UInt8 uart;

    for (uart = 0; uart < HAL_UART_NR_OF_UARTS; uart++)
    {
#if (HAL_UART_RX_DMA_MASK != 0) && !defined(HAL_UART_NO_RX)
        if (HAL_UART_RX_USE_DMA(uart))
        {
            if (hal_UartRxEnabled(uart))
            {
                halUart_RxEnableDma(uart);
            }
        }
#endif

#if (HAL_UART_TX_DMA_MASK != 0)
        if (HAL_UART_TX_USE_DMA(uart))
        {
            if (hal_UartTxEnabled(uart))
            {
                halUart_TxEnableDma(uart);
            }
        }
#endif
    }
}

/*****************************************************************************
 *                    Global interrupt handlers
 *****************************************************************************/

void uart0_handler_impl(void)
{
#ifndef HAL_UART_NO_RX
    if (!HAL_UART_RX_USE_DMA(0) && GP_WB_READ_INT_CTRL_MASKED_UART_0_RX_NOT_EMPTY_INTERRUPT())
    {
        halUart_HandleIntRx(0);
    }
    else
#endif
    if(GP_WB_READ_INT_CTRL_MASKED_UART_0_TX_NOT_FULL_INTERRUPT())
    {
        halUart_HandleIntTxData(0);
    }
    else if(GP_WB_READ_INT_CTRL_MASKED_UART_0_TX_NOT_BUSY_INTERRUPT())
    {
        if ((hal_cbUartEot_t)NULL != hal_cbUartOneShotEndOfTx)
        {
            hal_cbUartOneShotEndOfTx();
        }
        GP_WB_WRITE_INT_CTRL_MASK_UART_0_TX_NOT_BUSY_INTERRUPT(false);
        hal_cbUartOneShotEndOfTx = (hal_cbUartEot_t)NULL;
    }
    else
    {
        if(!halUart_txCtx[0].flushed)
        {
            GP_ASSERT_DEV_INT(false);
        }
        else
        {
            // we just flushed the uart, ignore spurious interrupt
            halUart_txCtx[0].flushed = false;
        }
    }
}

#if HAL_UART_NR_OF_UARTS > 1
void uart1_handler_impl(void)
{
#ifndef HAL_UART_NO_RX
    if (!HAL_UART_RX_USE_DMA(1) && GP_WB_READ_INT_CTRL_MASKED_UART_1_RX_NOT_EMPTY_INTERRUPT())
    {
        halUart_HandleIntRx(1);
    }
    else
#endif
    if(GP_WB_READ_INT_CTRL_MASKED_UART_1_TX_NOT_FULL_INTERRUPT())
    {
        halUart_HandleIntTxData(1);
    }
    else if(GP_WB_READ_INT_CTRL_MASKED_UART_1_TX_NOT_BUSY_INTERRUPT())
    {
        GP_WB_WRITE_INT_CTRL_MASK_UART_1_TX_NOT_BUSY_INTERRUPT(false);
    }
    else
    {
        if(!halUart_txCtx[1].flushed)
        {
            GP_ASSERT_DEV_INT(false);
        }
        else
        {
            // we just flushed the uart, ignore spurious interrupt
            halUart_txCtx[1].flushed = false;
        }
    }
}
#endif

#if HAL_UART_NR_OF_UARTS > 2
void uart2_handler_impl(void)
{
#ifndef HAL_UART_NO_RX
    if (!HAL_UART_RX_USE_DMA(2) && GP_WB_READ_INT_CTRL_MASKED_UART_2_RX_NOT_EMPTY_INTERRUPT())
    {
        halUart_HandleIntRx(2);
    }
    else
#endif
    if (GP_WB_READ_INT_CTRL_MASKED_UART_2_TX_NOT_FULL_INTERRUPT())
    {
        halUart_HandleIntTxData(2);
    }
    else if (GP_WB_READ_INT_CTRL_MASKED_UART_2_TX_NOT_BUSY_INTERRUPT())
    {
        GP_WB_WRITE_INT_CTRL_MASK_UART_2_TX_NOT_BUSY_INTERRUPT(false);
    }
    else
    {
        if(!halUart_txCtx[2].flushed)
        {
            GP_ASSERT_DEV_INT(false);
        }
        else
        {
            // we just flushed the uart, ignore spurious interrupt
            halUart_txCtx[2].flushed = false;
        }
    }
}
#endif
