/*
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

/** @file Implementation of FreeRTOS specific sleep control */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID GP_COMPONENT_ID_HALCORTEXM4

#include "gpLog.h"
#include "gpAssert.h"
#include "gpHal.h"
#include "gpHal_DEFS.h"
#include "gpHal_Calibration.h"
#include "hal_timer.h"


#ifdef GP_COMP_COM
#include "gpCom.h"
#endif


#include "FreeRTOS.h"
#include "semphr.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define CPU_CLOCK_MHZ (configCPU_CLOCK_HZ / 1000000UL)

// Minimum time in which the systick can be directly scheduled,
// when the time is less than this value we must schedule the next systick
#define MINIMAL_DELAY_FOR_SYSTICK_US 200

/** @brief Sleep time compensation value due to wait for GP_WB_READ_ES_INIT_TIME_REFERENCE_BUSY() in wakeup procedur
 * the different length of the awakening time related to the maximum time HAL_WAIT_TIME_TIME_REF_UPDATE_US */
#define WAKEUP_TIME_COMPENSATION_US 1000

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Local Variable
 *****************************************************************************/
/** @brief Idle tick tracking */
static UInt32 halFreeRTOS_IdleCnt;

#ifdef GP_FREERTOS_DIVERSITY_SLEEP
static Int32 halFreeRTOS_TimerOffsetUs;
static UInt32 halFreeRTOS_EsTimerOverflowCnt;
static UInt32 halFreeRTOS_TimeUsEndWithOffsetLast;
static Bool halFreeRTOS_OffsetReady;
#endif //GP_FREERTOS_DIVERSITY_SLEEP
/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

#ifdef GP_FREERTOS_DIVERSITY_SLEEP
static Bool confirmSleepModeStatus(TickType_t xExpectedIdleTime)
{
    if (hal_SleepCheck(xExpectedIdleTime * MS_TO_US(1)) == false)
    {
        return false;
    }
#if defined(GP_COMP_COM) && !defined(TBC_GPCOM)
    if(gpCom_TXDataPending())
    {
        return false;
    }
#endif //defined(GP_COMP_COM) && !defined(GP_COM_DIVERSITY_NO_RX)
#if defined(GP_COMP_COM) && !defined(TBC_GPCOM) && !defined(GP_COM_DIVERSITY_NO_RX)
    if (gpCom_IsReceivedPacketPending())
    {
        return false;
    }
#endif //defined(GP_COMP_COM) && !defined(GP_COM_DIVERSITY_NO_RX)
#if !defined(HAL_LINUX_DIVERSITY_INTERRUPT_WAKES_IOTHREAD)
    if (HAL_RADIO_INT_CHECK_IF_OCCURED())
    {
        return false;
    }
#endif
    return true;
}
#endif
/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
#ifdef GP_FREERTOS_DIVERSITY_SLEEP
void hal_InitSleepFreeRTOS(void)
{
    halFreeRTOS_TimerOffsetUs = 0;
    halFreeRTOS_EsTimerOverflowCnt = 0;
    halFreeRTOS_TimeUsEndWithOffsetLast = 0;
    halFreeRTOS_OffsetReady = false;
}
#endif //GP_FREERTOS_DIVERSITY_SLEEP

/* Supress the tick interrupt and enter into sleep */
void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
    halFreeRTOS_IdleCnt++;
#ifdef GP_FREERTOS_DIVERSITY_SLEEP
    HAL_DISABLE_GLOBAL_INT();
    Bool state = confirmSleepModeStatus(xExpectedIdleTime);
    HAL_ENABLE_GLOBAL_INT();

    if (state == true)
    {
        UInt32 beginSleepTimeUs = 0;
        UInt32 endSleepTimeUs = 0;
        UInt32 endSleepTimeUsWithOffset = 0;
        UInt32 systickLoad = SysTick->LOAD;
        TickType_t currentTicks = 0;

        UInt16 calibTimerVal = halTimer_getTimerValue(HAL_CALIBRATION_TIMER);
        UInt16 calibTimerEnable = BIT_TST(GP_WB_READ_TIMERS_TMR_ENABLES(), HAL_CALIBRATION_TIMER);
        UInt16 calibTimerThres = halTimer_getThreshold(HAL_CALIBRATION_TIMER);

        if(xExpectedIdleTime > HAL_SLEEP_MAX_SLEEP_TIME / MS_TO_US(1))
        {
            xExpectedIdleTime = HAL_SLEEP_MAX_SLEEP_TIME;
        }
        else
        {
            // ms tick to us conversion
            xExpectedIdleTime *= MS_TO_US(1);
        }

        if (xExpectedIdleTime >= WAKEUP_TIME_COMPENSATION_US)
        {
            xExpectedIdleTime -= WAKEUP_TIME_COMPENSATION_US;
        }

        hal_EnableGotoSleep();

        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
        HAL_TIMER_GET_CURRENT_TIME_1US(beginSleepTimeUs);

        // calculate initial time offset
        if (halFreeRTOS_OffsetReady == false)
        {
            halFreeRTOS_TimerOffsetUs = (xTaskGetTickCount() * portTICK_PERIOD_MS * MS_TO_US(1))
                            + (systickLoad - SysTick->VAL) / CPU_CLOCK_MHZ
                            - beginSleepTimeUs;
            halFreeRTOS_OffsetReady = true;
        }
        // Schedule a timer interrupt to wake from sleep.
        hal_sleep_uc(xExpectedIdleTime);

        HAL_TIMER_GET_CURRENT_TIME_1US(endSleepTimeUs);

        // Calculate the time to sleep avoiding round errors.
        // offset add to end time
        endSleepTimeUsWithOffset = endSleepTimeUs + halFreeRTOS_TimerOffsetUs;

        // ES timer overflow support
        if (endSleepTimeUsWithOffset < halFreeRTOS_TimeUsEndWithOffsetLast)
        {
            halFreeRTOS_EsTimerOverflowCnt++;
        }
        halFreeRTOS_TimeUsEndWithOffsetLast = endSleepTimeUsWithOffset;
        UInt32 longTimerMs = (UInt32)(((UInt64)halFreeRTOS_EsTimerOverflowCnt * 0x100000000ULL
                                       + endSleepTimeUsWithOffset) / MS_TO_US(1));
        UInt16 longTimerUs = (UInt16)(((UInt64)halFreeRTOS_EsTimerOverflowCnt * 0x100000000ULL
                                       + endSleepTimeUsWithOffset) % MS_TO_US(1));
        UInt16 longTimePartOfUs = longTimerUs % MS_TO_US(1);
        UInt16 longTimeComplementMs = MS_TO_US(1) - longTimePartOfUs;

        // compensate the freertos tick in full ms
        HAL_DISABLE_GLOBAL_INT();
        currentTicks = xTaskGetTickCount();
        vTaskStepTick(longTimerMs - currentTicks);
        HAL_ENABLE_GLOBAL_INT();

        if (longTimeComplementMs > MINIMAL_DELAY_FOR_SYSTICK_US)
        {
            /* Restart SysTick so it runs from SysTick->LOAD
            again, then set SysTick->LOAD back to its standard value. */
            SysTick->LOAD = CPU_CLOCK_MHZ * longTimeComplementMs;
        }
        else
        {
            // extra systick add
            vTaskStepTick(1);
            // schedule second systick
            SysTick->LOAD = CPU_CLOCK_MHZ * (2 * MS_TO_US(1) - longTimePartOfUs);

        }
        SysTick->VAL = 0;
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
        // restore load value
        SysTick->LOAD = systickLoad;


        if ((endSleepTimeUs - beginSleepTimeUs) > 1000) // deep sleep
        {
            GP_LOG_PRINTF("CT: val:%d, thr:%d, mask:0x%X", 0,
                                 halTimer_getTimerValue(HAL_CALIBRATION_TIMER),
                                 halTimer_getThreshold(HAL_CALIBRATION_TIMER),
                                 GP_WB_READ_INT_CTRL_MASKED_TIMERS_INTERRUPTS());
            if (calibTimerEnable)
            {
                halTimer_setThreshold(HAL_CALIBRATION_TIMER, calibTimerThres);
                halTimer_setTimerPreset(HAL_CALIBRATION_TIMER, calibTimerVal);
                halTimer_startTimer(HAL_CALIBRATION_TIMER);
            }
        }
    }
#endif // GP_FREERTOS_DIVERSITY_SLEEP
}
