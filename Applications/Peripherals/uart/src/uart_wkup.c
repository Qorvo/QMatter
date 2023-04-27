/*
 * Copyright (c) 2017-2021, Qorvo Inc
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

/** @file uart_wkup.c
 *
 * UART example application.
 * This example puts the chip to sleep and will wake up when a character is received on UART1.
 * After one second goes back to sleep.
 *
 */


/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "hal.h"
#include "gpHal.h"
#include "gpLog.h"
#include "gpSched.h"
#include "gpHal_kx_ES.h"
#include "gpBaseComps.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_APP

/** @brief Macro to calculate symbol period setting from baudrate*/
#define UART_CALCULATE_SYMBOL_PERIOD(baudrate) (16000000L / (8*baudrate+1))

/** @brief Symbolic value to return to UART Tx callback when no data is left to send */
#define UART_NO_DATA_TO_TX (-1)

/** @brief Define one second in microseconds for the scheduling*/
#define ONE_SECOND  (1000000)

/** @brief GPIO number of pin used as UART Rx*/
#if   \
      defined (GP_DIVERSITY_QPG6105DK_B01)
#define UART_RX_PIN GP_BSP_UART0_RX_GPIO
#else
#error Board not supported for this application
#endif

#if defined(GP_DIVERSITY_FREERTOS)
#ifndef GP_COMP_GPHAL
#define GP_COMP_GPHAL
#endif
#ifndef GP_COMP_SCHED
#define GP_COMP_SCHED
#endif
#endif


/*****************************************************************************
 *                    Static Data
 *****************************************************************************/
static Int16 App_TxData1 = UART_NO_DATA_TO_TX;

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/
#if defined(HAL_DIVERSITY_UART_RX_BUFFER_CALLBACK)
static void Application_UART1callbackRX(UInt8* buffer, UInt16 length);
#else
static void Application_UART1callbackRX(Int16 data);
#endif
static Int16 Application_UART1callbackTX(void);

static void Application_GoToSleep( void ) ;
void Application_InitGPIOWakeUp(void);

volatile static Bool extend_wakeup = false;
/*****************************************************************************
 *                    Application functions
 *****************************************************************************/
#if defined(HAL_DIVERSITY_UART_RX_BUFFER_CALLBACK)
void Application_UART1callbackRX(UInt8* buffer, UInt16 length)
{
    /* Echo data back */
    UInt8 i;

    HAL_DISABLE_GLOBAL_INT();

    for(i=0; i<length; i++)
    {
        App_TxData1 = buffer[i];
        hal_UartTxNewData(GP_BSP_UART_COM1);
    }

    extend_wakeup = true;

    HAL_ENABLE_GLOBAL_INT();
}
#else
/** @brief Callback triggered by UART1 receiving a byte. Will echo byte back through Tx
 */
void Application_UART1callbackRX(Int16 data)
{
    /* Echo data back */
    HAL_DISABLE_GLOBAL_INT();

    App_TxData1 = data;
    hal_UartTxNewData(GP_BSP_UART_COM1);

    HAL_ENABLE_GLOBAL_INT();
}
#endif /* HAL_DIVERSITY_UART_RX_BUFFER_CALLBACK */

/** @brief Callback triggered by UART1 to fetch a byte to TX.
 */
Int16 Application_UART1callbackTX(void)
{
    if(App_TxData1 != UART_NO_DATA_TO_TX)
    {
        Int16 data = App_TxData1;

        //Reset character to send
        App_TxData1 = UART_NO_DATA_TO_TX;
        return data;
    }
    else
    {
        return UART_NO_DATA_TO_TX;
    }
}

/*****************************************************************************
 *                    Application functions
 *****************************************************************************/

/** @brief Registered Callback from Qorvo stack to signal chip wakeup
*/
void Application_cbExternalEvent(void)
{
    /* Disable interrupt untill handled */
    gpHal_EnableExternalEventCallbackInterrupt(false);
    hal_SleepSetGotoSleepEnable(false);

    /* Send character signaling it's ready to receive */
    HAL_DISABLE_GLOBAL_INT();

    App_TxData1 = '>';
    hal_UartTxNewData(GP_BSP_UART_COM1);

    HAL_ENABLE_GLOBAL_INT();

    /* Delay check for debouncing of button/signal */
    if (!gpSched_ExistsEvent(Application_GoToSleep))
    {
        gpSched_ScheduleEvent(ONE_SECOND, Application_GoToSleep);
    }
}

/** @brief Function to handle GPIO changes - Button press
*/
void Application_GoToSleep(void)
{
    if (extend_wakeup)
    {
        extend_wakeup = false;
        gpSched_ScheduleEvent(ONE_SECOND, Application_GoToSleep);
        return;
    }
    HAL_DISABLE_GLOBAL_INT();

    App_TxData1 = 'S';
    hal_UartTxNewData(GP_BSP_UART_COM1);

    HAL_ENABLE_GLOBAL_INT();
    hal_SleepSetGotoSleepEnable(true);

    /* Clear flag and enable interrupt */
    GP_WB_ES_CLR_EXTERNAL_EVENT_INTERRUPT();
    gpHal_EnableExternalEventCallbackInterrupt(true);
}

/** @brief Extend initialization of GPIOs to allow wakeup from sleep on GPIO trigger
 */
void Application_InitGPIOWakeUp(void)
{
    gpHal_ExternalEventDescriptor_t eventDesc;

    /* Configure pins for wakeup */
    hal_gpioSetWakeUpMode(UART_RX_PIN, hal_WakeUpModeBoth);

    /* Configure External event block */
    /* Only ISR generation */
    eventDesc.type = gpHal_EventTypeDummy;
    gpHal_ScheduleExternalEvent(&eventDesc);

    /* Register handler function */
    gpHal_RegisterExternalEventCallback(Application_cbExternalEvent);

    /* Clear flag and enable interrupt mask */
    GP_WB_ES_CLR_EXTERNAL_EVENT_INTERRUPT();
    gpHal_EnableExternalEventCallbackInterrupt(true);
}

/*****************************************************************************
 *                    Application Init
 *****************************************************************************/

 /** @brief Initialize application
 */
void Application_Init(void)
{
    /* Initialize stack */
#if !defined(GP_DIVERSITY_FREERTOS)
    gpBaseComps_StackInit();
#else
    // Skip gpBaseComps_StackInit since it initializes gpCom

#ifdef GP_COMP_GPHAL
    gpHal_EnableInterrupts(true);
#endif //GP_COMP_GPHAL

#ifdef GP_COMP_SCHED
#ifndef GP_DIVERSITY_FREERTOS
    gpSched_Init();
#endif
#if defined(GP_DIVERSITY_GPHAL_INTERN) &&  defined(GP_DIVERSITY_GPHAL_K8E)
#ifdef GP_SCHED_DIVERSITY_SLEEP
#ifdef HAL_DEFAULT_GOTOSLEEP_THRES
    hal_SleepSetGotoSleepThreshold(HAL_DEFAULT_GOTOSLEEP_THRES);
#endif //HAL_DEFAULT_GOTOSLEEP_THRES
#endif //def GP_SCHED_DIVERSITY_SLEEP
#endif //defined(GP_DIVERSITY_GPHAL_INTERN) && (defined(GP_DIVERSITY_GPHAL_K8C) || defined(GP_DIVERSITY_GPHAL_K8D)) || defined(GP_DIVERSITY_GPHAL_K8E))
#endif //GP_COMP_SCHED

#ifdef GP_COMP_SCHED
   gpSched_StartTimeBase();
#ifdef GP_SCHED_DIVERSITY_SLEEP
#ifndef GP_SCHED_FREE_CPU_TIME
    hal_SleepSetGotoSleepEnable(false);
#endif //GP_SCHED_FREE_CPU_TIME
#endif
#endif //GP_COMP_SCHED


#endif
    /* initialize GPIO to allow wake up from sleep */
    Application_InitGPIOWakeUp();

    hal_UartStart(Application_UART1callbackRX,
                  Application_UART1callbackTX,
                  HAL_UART_SYMBOL_PERIOD(GP_BSP_UART_COM_BAUDRATE),
                 (HAL_UART_OPT_8_BITS_PER_CHAR | HAL_UART_OPT_NO_PARITY | HAL_UART_OPT_ONE_STOP_BIT),
                  GP_BSP_UART_COM1);

    /* Enable sleep behavior */
    gpHal_SetSleepMode(gpHal_SleepModeRC);

    /* Enable sleep behavior */
    hal_SleepSetGotoSleepEnable(true);
}
