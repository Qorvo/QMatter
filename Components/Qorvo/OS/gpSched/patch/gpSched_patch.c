/*
 * Copyright (c) 2017, 2019, Qorvo Inc
 *
 * gpSched.c
 *   This file contains the implementation of the scheduler, which is the operating system.
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

#define GP_COMPONENT_ID GP_COMPONENT_ID_SCHED

#include "gpSched.h"
#include "gpSched_defs.h"
#include "hal.h"
#include "gpUtils.h"
#include "gpLog.h"

#include "gpJumpTables.h"

#if defined(GP_COMP_COM)
#include "gpCom.h"
#endif


/*****************************************************************************
 *                    Precompiler checks
 *****************************************************************************/

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/


/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    External Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/


/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/




#if     defined(GP_DIVERSITY_GPHAL_K8E)
#define ROMVERSION_FIXFORPATCH_SCHED_CANGOTOSLEEP  2
#define ROMVERSION_FIXFORPATCH_SCHED_GOTOSLEEP     2

#else
#define ROMVERSION_FIXFORPATCH_SCHED_CANGOTOSLEEP  1
#define ROMVERSION_FIXFORPATCH_SCHED_GOTOSLEEP     1
#define GPJUMPTABLES_MIN_ROMVERSION                0
#endif

UInt32 Sched_GetEventIdlePeriodPatched(void)
{
    gpSched_globals_t* sched_globals = GP_SCHED_GET_GLOBALS();
    UInt32 idleTime = HAL_SLEEP_INDEFINITE_SLEEP_TIME;

    gpSched_Event_t* pevt;

    gpUtils_LLLockAcquire((gpUtils_Links_t *)sched_globals->gpSched_EventList_p);
    pevt = (gpSched_Event_t*)gpUtils_LLGetFirstElem(sched_globals->gpSched_EventList_p);
    if (pevt)
    {
        if(hal_SleepGetGotoSleepThreshold() == GP_SCHED_NO_EVENTS_GOTOSLEEP_THRES) //Only sleep if no events are pending
        {
            idleTime = 0;
        }
        else
        {
            UInt32 time_now;
            HAL_TIMER_GET_CURRENT_TIME_1US(time_now);

            if(GP_SCHED_TIME_COMPARE_BIGGER(pevt->time,(time_now + hal_SleepGetGotoSleepThreshold())))
            {
                idleTime = GP_SCHED_GET_TIME_DIFF(time_now, pevt->time);
            }
            else
            {
                idleTime = 0;
            }
        }
    }
    gpUtils_LLLockRelease((gpUtils_Links_t *)sched_globals->gpSched_EventList_p);

    return (idleTime);
}
/*
* @brief Returns time to sleep (in microseconds).
* Will return 0 if no sleep is permitted at this point (pending actions, idle time below threshold)
*/
UInt32 Sched_CanGoToSleep_orgrom( void );
UInt32 Sched_CanGoToSleep(void)
{
#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_SCHED_CANGOTOSLEEP)
#if defined(GP_DIVERSITY_JUMPTABLES) && !defined(GP_DIVERSITY_KEEP_NRT_IN_FLASH) 
    if(gpJumpTables_GetRomVersion() < ROMVERSION_FIXFORPATCH_SCHED_CANGOTOSLEEP)
#else
    if(ROMVERSION_FIXFORPATCH_SCHED_CANGOTOSLEEP)
#endif
    {
        UInt32 result = 0;
        gpSched_globals_t* sched_globals = GP_SCHED_GET_GLOBALS();

        HAL_DISABLE_GLOBAL_INT();
        result = Sched_GetEventIdlePeriodPatched();

        if ((hal_SleepCheck(result) == false) ||
            (sched_globals->gpSched_cbGotoSleepCheck && !sched_globals->gpSched_cbGotoSleepCheck()))
        {
#ifdef GP_SCHED_FREE_CPU_TIME
        /* Even when sleep is disabled, sleep for GP_SCHED_FREE_CPU_TIME */
            if(result > GP_SCHED_FREE_CPU_TIME)
            {
                result = GP_SCHED_FREE_CPU_TIME;
            }
#else
            result = 0;
#endif //GP_SCHED_FREE_CPU_TIME
        }

#if defined(GP_COMP_COM) && !defined(TBC_GPCOM) && !defined(GP_COM_DIVERSITY_NO_RX)
        if (SCHED_APP_DIVERSITY_COM() && !SCHED_APP_DIVERSITY_COM_NO_RX())
        {
            //overrule if pending RX data on COM
            if(gpCom_IsReceivedPacketPending())
            {
                result = 0;
            }
        }
#endif //defined(GP_COMP_COM) && !defined(GP_COM_DIVERSITY_NO_RX)

#if defined(GP_COMP_COM) && !defined(TBC_GPCOM)
        //overrule if pending TX data on COM
        if(gpCom_TXDataPending())
        {
            result = 0;
        }
#endif //defined(GP_COMP_COM) && !defined(TBC_GPCOM)


#if !defined(HAL_LINUX_DIVERSITY_INTERRUPT_WAKES_IOTHREAD)
        //Don't go to sleep when Radio interrupt is pending
        if(HAL_RADIO_INT_CHECK_IF_OCCURED())
        {
            result = 0;
        }
#endif

// FIXME: should implement this callback also for other chipemu variants.

        HAL_ENABLE_GLOBAL_INT();

#ifdef GP_DIVERSITY_LINUXKERNEL
        // The Linux kernel driver wakes up "GoToSleepTreshold" before the
        // scheduled time and runs a short non-sleeping wait loop before
        // event execution to avoid over-sleeping.
        if (result != HAL_SLEEP_INDEFINITE_SLEEP_TIME)
        {
            if (result > hal_SleepGetGotoSleepThreshold())
            {
                result -= hal_SleepGetGotoSleepThreshold();
            }
            else
            {
                result = 0;
            }
        }
#endif

        return result;
    }
    else
#endif // GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_SCHED_CANGOTOSLEEP
    {
        /* Use original function */
        return Sched_CanGoToSleep_orgrom();
    }
}

void gpSched_GoToSleep_orgrom( void );
void gpSched_GoToSleep( void )
{
#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_SCHED_GOTOSLEEP)
#if defined(GP_DIVERSITY_JUMPTABLES) && !defined(GP_DIVERSITY_KEEP_NRT_IN_FLASH) 
    // Check which NRT ROM version is present in this device
    if(gpJumpTables_GetRomVersion() < ROMVERSION_FIXFORPATCH_SCHED_GOTOSLEEP)
#else
    if(ROMVERSION_FIXFORPATCH_SCHED_GOTOSLEEP)
#endif
    {
        if (Sched_CanGoToSleep())
        {

            HAL_DISABLE_GLOBAL_INT();
            // Disable unneeded interrupts
            HAL_TIMER_STOP();   // Re-enabled by TIMER_RESTART()

            HAL_ENABLE_SLEEP_UC();
            HAL_ENABLE_GLOBAL_INT();

            while (true)
            {
                UInt32 timeTosleep;

                timeTosleep = Sched_CanGoToSleep();
                if(timeTosleep == 0)
                {
                    break;
                }
                HAL_SLEEP_UC_1US(timeTosleep);
                HAL_ENABLE_SLEEP_UC();
            }

            // Restart timer of uC without initialization
            // Note: if we have a HAL that supports going to sleep while an event is pending,
            // we have to forward the time of the scheduler when a new event is scheduled from an ISR or
            // when we wake-up to execute the pending event.
            HAL_TIMER_RESTART();
        }
    }
    else
#endif // #if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_SCHED_GOTOSLEEP)
    {
        gpSched_GoToSleep_orgrom();
    }
}




