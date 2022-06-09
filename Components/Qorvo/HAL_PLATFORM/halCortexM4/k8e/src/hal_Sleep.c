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
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"
#include "hal.h"
#include "hal_defs.h"
#include "gpHal.h"
#include "gpHal_Calibration.h"
#include "gpLog.h"
#include "gpHal_ES.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_HALCORTEXM4

#if defined(__GNUC__)
#if !defined(__SES_ARM)
#define HAL_LOWERRAM_RETAIN_SIZE  ((UInt32)&__lowerram_retain_size)
#define HAL_HIGHERRAM_RETAIN_SIZE ((UInt32)&__higherram_retain_size)
#else
#define HAL_LOWERRAM_RETAIN_SIZE  ((UInt32)(&__RAM_Low_region_segment_used_end__) - (UInt32)(&lowerram_start))
#define HAL_HIGHERRAM_RETAIN_SIZE ((UInt32)(&__higher_ram_retain_end__) - (UInt32)(&higherram_start))
#endif
#endif

#if defined(__IAR_SYSTEMS_ICC__)
#pragma segment = "lower_ram_endretain"
#define HAL_LOWERRAM_RETAIN_END  ((UInt32)__section_begin("lower_ram_endretain"))
#define HAL_LOWERRAM_RETAIN_SIZE (HAL_LOWERRAM_RETAIN_END - (UInt32)(&lowerram_start))

#pragma segment = "higher_ram_endretain"
#define HAL_HIGHERRAM_RETAIN_END  ((UInt32)__section_begin("higher_ram_endretain"))
#define HAL_HIGHERRAM_RETAIN_SIZE (HAL_HIGHERRAM_RETAIN_END - ((UInt32)(&higherram_start)))
#endif

// Amount of RAM bytes (UC and SYS) that should be used for CRC calculation
// Total amount = 2^(HAL_CRC_MODE_CONFIG_RETENTION_SIZE_POWER + 1) bytes
#define HAL_CRC_MODE_CONFIG_RETENTION_SIZE_POWER 5

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef UInt32 TickType_t;

typedef struct {
    UInt32 disableCounter;
    TickType_t threshold;
} halFreeRTOS_SleepControlBlock_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

// event used for (timed) wake-up
static gpHal_AbsoluteEventId_t hal_wakeUpEventId;

// Indication whether the ARM is allowed to go to sleep (can be prevented by an interrupt that is not handled).
Bool hal_maySleep;


/*****************************************************************************
 *                    Static Function Declarations
 *****************************************************************************/

static void hal_ConfigureWakeUpEvent(void);
static void hal_RescheduleWakeUpEvent(UInt32 sleepTime);
static void hal_ConfigureRetention(void);
#ifdef GP_DIVERSITY_DEVELOPMENT
static Bool hal_CheckExternalEventConfigured(void);
#endif

/*****************************************************************************
 *                    Linkerscript Symbols Declarations
 *****************************************************************************/

#if defined(__GNUC__)
#if !defined(__SES_ARM)
//FIXME: SDP003-2798 Avoid using weak attributes for linker symbols
extern const UInt32 __attribute__((weak)) __lowerram_retain_size;
extern const UInt32 __attribute__((weak)) __higherram_retain_size;
#else
extern const UInt32 __RAM_Low_region_segment_used_end__;
extern const UInt32 lowerram_start;
extern const UInt32 __higher_ram_retain_end__;
extern const UInt32 higherram_start;
#endif
#endif
#if defined(__IAR_SYSTEMS_ICC__)
//extern const UIntPtr lowerram_retain_section_start ;
extern const UIntPtr lowerram_start;
extern const UIntPtr higherram_start;
#endif

extern const UIntPtr sw_retention_begin;
#define HAL_SW_RETENTION_BEGIN ((UIntPtr)&sw_retention_begin)

/*****************************************************************************
 *                    External Function Declarations
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

void hal_ConfigureWakeUpEvent(void)
{
    gpHal_AbsoluteEventDescriptor_t ev;

    // Need to wake up in 'sleeptime' * 32us. Wakeup event must be scheduled.
    MEMSET(&ev, 0, sizeof(ev));

    // Allocate event id, but never free it.
    // We keep the event info and only update execution time (performance optimization)
    hal_wakeUpEventId = gpHal_GetAbsoluteEvent();
    GP_ASSERT_DEV_EXT(GPHAL_ES_ABSOLUTE_EVENT_ID_INVALID != hal_wakeUpEventId);

    ev.type = GP_WB_ENUM_EVENT_TYPE_DUMMY;
    /* we need ex_itl when waking up very fast */
    ev.executionOptions = GP_ES_EXECUTION_OPTIONS_EXECUTE_IF_TOO_LATE;
    ev.interruptOptions = GP_ES_INTERRUPT_OPTIONS_MASK;

    if(HAL_BSP_IO_ACTIVITY_PENDING())
    {
        ev.executionOptions |= GP_ES_EXECUTION_OPTIONS_PROHIBIT_STANDBY;
    }

    gpHal_GetTime(&ev.exTime);

    ev.exTime += HAL_SLEEP_MAX_SLEEP_TIME;

    GP_ES_SET_EVENT_STATE(ev.control, gpHal_EventStateScheduled);
    gpHal_ScheduleAbsoluteEvent(&ev, hal_wakeUpEventId);
    gpHal_UnscheduleAbsoluteEvent(hal_wakeUpEventId);
    gpHal_EnableAbsoluteEventCallbackInterrupt(hal_wakeUpEventId, true);
}

void hal_RescheduleWakeUpEvent(UInt32 sleepTime)
{
    UInt32 wakeUpTs;
    UInt8 wakeUpControl = 0;

    gpHal_GetTime(&wakeUpTs);

    if(sleepTime == HAL_SLEEP_INDEFINITE_SLEEP_TIME)
    {
        wakeUpTs += HAL_SLEEP_MAX_SLEEP_TIME; /* wake anyhow to be sure we increment the overflow counter after an overflow */
    }
    else
    {
        wakeUpTs += sleepTime;
    }

    GP_ES_SET_EVENT_RESULT(wakeUpControl, gpHal_EventResultInvalid);
    GP_ES_SET_EVENT_STATE(wakeUpControl, gpHal_EventStateScheduled);

    gpHal_RefreshAbsoluteEvent(hal_wakeUpEventId, wakeUpTs, wakeUpControl);
}

void hal_ConfigureRetention(void)
{
    UInt8 vddram = 0;
    UInt8 smram = 0;
    UInt8 ucram = 0;

    // Always apply crc on SYS RAM
    smram = HAL_CRC_MODE_CONFIG_RETENTION_SIZE_POWER;

    if(HAL_HIGHERRAM_RETAIN_SIZE > 0)
    {
        // if any part of higher RAM will be retained, add it to the crc mode config
        ucram = HAL_CRC_MODE_CONFIG_RETENTION_SIZE_POWER;
    }

    if(HAL_HIGHERRAM_RETAIN_SIZE <= (64 * 1024))
    {
        vddram |= GP_WB_PMUD_PMU_VDDRAM_SEL_UCRAM_64_96KB_MASK;
    }
    if(HAL_HIGHERRAM_RETAIN_SIZE <= (32 * 1024))
    {
        vddram |= GP_WB_PMUD_PMU_VDDRAM_SEL_UCRAM_32_64KB_MASK;
    }
    if(HAL_HIGHERRAM_RETAIN_SIZE == 0)
    {
        vddram |= GP_WB_PMUD_PMU_VDDRAM_SEL_UCRAM_0_32KB_MASK;
    }
    if((HAL_LOWERRAM_RETAIN_SIZE <= (8 * 1024)))
    {
        vddram |= GP_WB_PMUD_PMU_VDDRAM_SEL_SYSRAM_8_32KB_MASK;
    }

    /* 0-8KB is always required */
    GP_WB_WRITE_PMUD_PMU_VDDRAM_SEL(vddram);

    hal_set_crc_mode(smram, ucram);
}

#ifdef GP_DIVERSITY_DEVELOPMENT
Bool hal_CheckExternalEventConfigured(void)
{
    Bool configured = true;

    //Check if all settings for an external wake-up are set
    //Can't wakeup from external trigger otherwise

    //external event settings
    //GP_LOG_SYSTEM_PRINTF("Ext ev t:%x v:%i", 0, GP_WB_READ_ES_EXTERNAL_EVENT_TYPE_TO_BE_EXECUTED(),
    //                                            GP_WB_READ_ES_EXTERNAL_EVENT_VALID());
    //gpLog_Flush();
    configured &= GP_WB_READ_ES_EXTERNAL_EVENT_TYPE_TO_BE_EXECUTED() == GP_WB_ENUM_EVENT_TYPE_DUMMY;
    configured &= GP_WB_READ_ES_EXTERNAL_EVENT_VALID() != 0;
    //masks
    //GP_LOG_SYSTEM_PRINTF("Ext ev mask:%x %x", 0, GP_WB_READ_INT_CTRL_MASK_ES_EXTERNAL_EVENT_INTERRUPT(),
    //                                             GP_WB_READ_INT_CTRL_MASK_INT_ES_INTERRUPT());
    //gpLog_Flush();
    configured &= GP_WB_READ_INT_CTRL_MASK_ES_EVENT_INTERRUPTS() != 0;
    configured &= GP_WB_READ_INT_CTRL_MASK_INT_ES_INTERRUPT() != 0;

    configured &= GP_WB_READ_U16(GP_WB_PMUD_WAKEUP_PIN_MODE_0_ADDRESS) != 0;
    /* check for interrupts from GPMICRO */
    if(!configured)
    {
        configured = GP_WB_READ_INT_CTRL_MASK_INT_RCI_INTERRUPT() != 0;
    }

    if(!configured)
    {
        configured = GP_WB_READ_INT_CTRL_MASK_INT_IPCGPM2X_INTERRUPT() != 0;
    }

#ifdef GP_DIGITAL_KEYBOARDSCAN
    if(!configured) /* No normal external event found */
    {
        //Check keyscan interrupt - will also wakeup ARM when scan is done
        //GP_LOG_SYSTEM_PRINTF("KeySc: %i", 0, GP_WB_READ_INT_CTRL_MASK_KPS_INT_INTERRUPT());
        configured = GP_WB_READ_INT_CTRL_MASK_INT_KEYPAD_INTERRUPT() != 0;
    }
#endif
    return configured;
}
#endif /* GP_DIVERSITY_DEVELOPMENT */

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void hal_InitSleep(void)
{
    hal_ConfigureWakeUpEvent();

    /* Enable deep sleep */
    gpHal_GoToSleepWhenIdle(true);
    hal_ConfigureRetention();

    hal_maySleep = true;

    /* Make sure that sw retention area does not overlap with hardware retention area */
    GP_ASSERT_SYSTEM(HAL_SW_RETENTION_BEGIN >= GP_MM_RAM_RETENTION_END);
}

#ifdef GP_DIVERSITY_JUMPTABLES
Bool hal_CanGotoSleep(void)
{
    return hal_maySleep;
}

void hal_EnableGotoSleep(void)
{
    hal_maySleep = true;
}

void hal_DisableGotoSleep(void)
{
    hal_maySleep = false;
}
#endif //def GP_DIVERSITY_JUMPTABLES

void hal_sleep_uc(UInt32 sleeptime)
{
#ifdef GP_DIVERSITY_DEVELOPMENT
    UInt32 t1, t2;
#endif

    GP_ASSERT_DEV_EXT(l_n_atomic == 0);

    if(sleeptime < 1000)
    {
        return;
    }

    if((sleeptime != HAL_SLEEP_INDEFINITE_SLEEP_TIME) && (sleeptime > HAL_SLEEP_MAX_SLEEP_TIME))
    {
        // ceil to maximum sleep time
        sleeptime = HAL_SLEEP_MAX_SLEEP_TIME;
    }

    /*
     * Interrupts are disabled again just before sleep instruction
     * such that all preparation steps have been done or none.
     */
    HAL_DISABLE_GLOBAL_INT();

    hal_RescheduleWakeUpEvent(sleeptime);

#ifdef GP_DIVERSITY_DEVELOPMENT
    if(sleeptime == HAL_SLEEP_INDEFINITE_SLEEP_TIME)
    {
        if(!hal_CheckExternalEventConfigured())
        {
            GP_LOG_SYSTEM_PRINTF("No Ext WKUP configured", 0);
            GP_ASSERT_SYSTEM(false);
        }
    }
#endif

#ifdef HAL_DIVERSITY_UART
    hal_UartBeforeSleep();
#endif

    /* finally, everything is ready for a healthy sleep */
    HAL_ENABLE_GLOBAL_INT();

#ifdef GP_DIVERSITY_DEVELOPMENT
    gpHal_GetTime(&t1);
#endif

    /* Actual sleep */
    hal_sleep();

    /*
     * At this point, any interrupt is executed and IntHandlerPrologue() is executed
     * OR we did not go to sleep at all...
     */

#ifdef HAL_DIVERSITY_UART
    hal_UartAfterSleep();
#endif // HAL_DIVERSITY_UART

    gpHal_UnscheduleAbsoluteEvent(hal_wakeUpEventId);

#ifdef GP_DIVERSITY_DEVELOPMENT
    gpHal_GetTime(&t2);

    if((t2 - t1) > 1000)
    {
        GP_LOG_PRINTF("slept %lu us(%lu->%lu)", 0, dist(t2, t1), t1, t2);
    }
#endif

}

void hal_SleepSetGotoSleepEnable(Bool enable)
{
    NOT_USED(enable);
}

void hal_SleepSetGotoSleepThreshold(TickType_t threshold)
{
    NOT_USED(threshold);
}

Bool hal_SleepCheck(uint32_t xExpectedIdleTime)
{
    return false;
}
