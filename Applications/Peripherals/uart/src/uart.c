/*
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
 *
 */

/** @file uart.c
 *
 * UART example application.
 * This example repeats a print of a test string every second - 200ms per char.
 * It will also echo any byte sent to it.
 *
 */


/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "hal.h"
#include "gpLog.h"
#include "gpSched.h"
#include "gpBaseComps.h"
#include "gpHal.h"

#include "app_common.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_APP

/** @brief Symbolic value to return to UART Tx callback when no data is left to send */
#define UART_NO_DATA_TO_TX (-1)

#if defined(GP_DIVERSITY_FREERTOS)
#ifndef GP_COMP_GPHAL
#define GP_COMP_GPHAL
#endif
#ifndef GP_COMP_SCHED
#define GP_COMP_SCHED
#endif
#endif

#define HAL_UART_COM_SYMBOL_PERIOD (((32000000L+(8*GP_BSP_UART_COM_BAUDRATE/2)) / (8*GP_BSP_UART_COM_BAUDRATE))-1)

/*****************************************************************************
 *                    Static Data
 *****************************************************************************/

static const UInt8 App_StringBuffer[] = "Hello, World!\r\n";
static UInt8 App_StringMarker = 0;
static Int16 App_TxData = UART_NO_DATA_TO_TX;

#if defined(GP_DIVERSITY_FREERTOS)
#define MAIN_STACK_SIZE (4 * 1024)
static StaticTask_t xAppUartTaskPCB;
static StackType_t xAppUartTaskStack[MAIN_STACK_SIZE];

#define mainTASK_PRIORITY ( tskIDLE_PRIORITY + 2 )
#endif

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

#if defined(HAL_DIVERSITY_UART_RX_BUFFER_CALLBACK)
static void Application_UARTcallbackRX(UInt8* buffer, UInt16 length);
#else
static void Application_UARTcallbackRX(Int16 data);
#endif
static Int16 Application_UARTcallbackTX(void);
static void Application_SendNextChar(void);
static void Application_SendTestString(void);

/*****************************************************************************
 *                    Application functions
 *****************************************************************************/

/** @brief Callback triggered by UART receiving a byte. Will echo byte back through UART Tx
 */
#if defined(HAL_DIVERSITY_UART_RX_BUFFER_CALLBACK)
void Application_UARTcallbackRX(UInt8* buffer, UInt16 length)
{
    /* Echo data back */
    UInt8 i;

    HAL_DISABLE_GLOBAL_INT();

    for(i=0; i<length; i++)
    {
        App_TxData = buffer[i];
        hal_UartTxNewData(GP_BSP_UART_COM1);
    }

    HAL_ENABLE_GLOBAL_INT();
}
#else
void Application_UARTcallbackRX(Int16 data)
{
    /* Echo data back */

    HAL_DISABLE_GLOBAL_INT();

    App_TxData = data;
    hal_UartTxNewData(GP_BSP_UART_COM1);

    HAL_ENABLE_GLOBAL_INT();
}
#endif

/** @brief Callback triggered by UART to fetch a byte to TX.
 */
Int16 Application_UARTcallbackTX(void)
{
    if(App_TxData != UART_NO_DATA_TO_TX)
    {
        Int16 data = App_TxData;

        //Reset character to send
        App_TxData = UART_NO_DATA_TO_TX;
        return data;
    }
    else
    {
        return UART_NO_DATA_TO_TX;
    }
}

/** @brief Function to send a single character of the string
 */
void Application_SendNextChar(void)
{
    LED_INDICATOR_ON();
#ifdef GP_DIVERSITY_FREERTOS
    vTaskDelay(100);
#else
    HAL_WAIT_MS(100);
#endif
    LED_INDICATOR_OFF();

    if(App_StringMarker < sizeof(App_StringBuffer) - 1)
    {
        HAL_DISABLE_GLOBAL_INT();

        App_TxData = App_StringBuffer[App_StringMarker];
        App_StringMarker++;
        hal_UartTxNewData(GP_BSP_UART_COM1);

        HAL_ENABLE_GLOBAL_INT();

        gpSched_ScheduleEvent(MS_TO_US(200), Application_SendNextChar);
    }
    else
    {
        gpSched_ScheduleEvent(MS_TO_US(1000), Application_SendTestString);
    }
}

/** @brief Function to kick off transmission of a test string
 */
void Application_SendTestString(void)
{
    HAL_LED_SET(RED);
#ifdef GP_DIVERSITY_FREERTOS
    vTaskDelay(100);
#else
    HAL_WAIT_MS(100);
#endif
    HAL_LED_CLR(RED);

    //Tx from start of string
    App_StringMarker = 0;
    gpSched_ScheduleEvent(0, Application_SendNextChar);
}

#if defined(GP_DIVERSITY_FREERTOS)
static void Application_MainLoop(void *pvParameters)
{
    (void)pvParameters;

//    GP_LOG_PRINTF("Application start...", 0);

    gpSched_ScheduleEvent(0, Application_SendTestString);
    vTaskSuspend(NULL);
}
#endif
/*****************************************************************************
 *                    Application Init
 *****************************************************************************/

/** @brief Initialize application
*/
void Application_Init(void)
{
    HAL_ENABLE_GLOBAL_INT();

    HAL_WDT_DISABLE();

#ifndef GP_BASECOMPS_DIVERSITY_NO_GPCOM_INIT
#error GP_BASECOMPS_DIVERSITY_NO_GPCOM_INIT should keep gpcom from claiming the uart
#endif
    /* Initialize whole stack */
    gpBaseComps_StackInit();

    HAL_DISABLE_GLOBAL_INT();

    /* Start the UART peripheral (see BSP for mapping and UART block selection) */

    /* Settings: Baudrate GP_BSP_UART_COM_BAUDRATE, 8-bits, No Parity, 1 Stop Bit */
    hal_UartStart(Application_UARTcallbackRX,
                  Application_UARTcallbackTX,
                  HAL_UART_COM_SYMBOL_PERIOD,
                 (HAL_UART_OPT_8_BITS_PER_CHAR | HAL_UART_OPT_NO_PARITY | HAL_UART_OPT_ONE_STOP_BIT),
                  GP_BSP_UART_COM1);

    HAL_ENABLE_GLOBAL_INT();

#if defined(GP_DIVERSITY_FREERTOS)
    TaskHandle_t TaskHandle;

    TaskHandle = xTaskCreateStatic(
            Application_MainLoop,                   /* The function that implements the task. */
            "app uart",                             /* The text name assigned to the task - for debug only as it is not used by the kernel. */
            MAIN_STACK_SIZE,                        /* The size of the stack to allocate to the task. */
            NULL,                                   /* The parameter passed to the task */
            mainTASK_PRIORITY,                      /* The priority assigned to the task. */
            xAppUartTaskStack,                      /* The task stack memory */
            &xAppUartTaskPCB);                      /* The task PCB memory */
    GP_ASSERT (GP_DIVERSITY_ASSERT_LEVEL_SYSTEM, TaskHandle!=NULL);
#else
    gpSched_ScheduleEvent(0, Application_SendTestString);
#endif
}
