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
 * $Header: //depot/release/Embedded/Components/Qorvo/HAL_PLATFORM/v2.10.2.1/comps/halCortexM4/src/hal_Mutex.c#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

/** @file Implementation of OS specific Mutex wrappers */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "hal.h"

#ifdef GP_DIVERSITY_FREERTOS
#include "FreeRTOS.h"
#include "semphr.h"
#endif //GP_DIVERSITY_FREERTOS

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

HAL_CRITICAL_SECTION_TYPE hal_MutexCreate(void* mutexMemory)
{
#ifdef GP_DIVERSITY_FREERTOS
#if configSUPPORT_DYNAMIC_ALLOCATION
    return xSemaphoreCreateMutex();
#else    // Create static buffer for mutex memory outside of function
    return xSemaphoreCreateMutexStatic((StaticSemaphore_t*)mutexMemory);
#endif //configSUPPORT_DYNAMIC_ALLOCATION

#else
    // No return
#endif
}

void hal_MutexDestroy(HAL_CRITICAL_SECTION_PARAM(pMutex))
{
#if defined(GP_DIVERSITY_FREERTOS)
#if configSUPPORT_DYNAMIC_ALLOCATION    
    vSemaphoreDelete(pMutex);
#endif //configSUPPORT_DYNAMIC_ALLOCATION
#endif //GP_DIVERSITY_FREERTOS
}

Bool hal_MutexIsValid(HAL_CRITICAL_SECTION_PARAM(pMutex))
{
#if defined(GP_DIVERSITY_FREERTOS)
    return pMutex != 0;
#else
    // Not implemented
    return true;
#endif
}

Bool hal_MutexIsAcquired(HAL_CRITICAL_SECTION_PARAM(pMutex))
{
#if defined(GP_DIVERSITY_FREERTOS)
    xPSR_Type psr;
    psr.w = __get_xPSR();

    // Use ISR safe function
    if(psr.b.ISR != 0)
    {
        return (uxQueueMessagesWaitingFromISR((QueueHandle_t)(pMutex)) == 0);
    }
    else
    {
        return (uxSemaphoreGetCount((pMutex)) == 0);
    }
#else
    // Using full ISR disabling as mutex
    return !HAL_GLOBAL_INT_ENABLED();
#endif
}

void hal_MutexAcquire(HAL_CRITICAL_SECTION_PARAM(pMutex))
{
#if defined(GP_DIVERSITY_FREERTOS)
    xPSR_Type psr;
    psr.w = __get_xPSR();

    if(psr.b.ISR != 0)
    {
        BaseType_t taken;
        xSemaphoreTakeFromISR((pMutex), &taken);
        NOT_USED(taken);
    }
    else
    {
        xSemaphoreTake((pMutex), portMAX_DELAY);
    }
#else
    HAL_DISABLE_GLOBAL_INT();
#endif
}

void hal_MutexRelease(HAL_CRITICAL_SECTION_PARAM(pMutex))
{
#if defined(GP_DIVERSITY_FREERTOS)
    xPSR_Type psr;
    psr.w = __get_xPSR();

    if(psr.b.ISR != 0)
    {
        BaseType_t woken;
        xSemaphoreGiveFromISR((pMutex), &woken);
        NOT_USED(woken);
    }
    else
    {
        xSemaphoreGive((pMutex));
    }
#else
    HAL_ENABLE_GLOBAL_INT();
#endif
}
