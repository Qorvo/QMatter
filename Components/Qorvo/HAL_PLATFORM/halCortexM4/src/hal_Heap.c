/*
 * Copyright (c) 2023, Qorvo Inc
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

/** @file Implementation of non chip specific heap utility functions */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID GP_COMPONENT_ID_HALCORTEXM4

#include "hal.h"

/*****************************************************************************
 *                    HEAP
 *****************************************************************************/

#ifdef GP_KX_HEAP_SIZE
#ifdef __ICCARM__
        struct internal_malloc_stat_struct {
    size_t maxfp;
    size_t fp;
    size_t used;
};
void __iar_internal_malloc_stats(struct internal_malloc_stat_struct*);

void hal_GetHeapInUse(UInt32* pInUse, UInt32* pReserved, UInt32* pMax)
{
    struct internal_malloc_stat_struct imss;
    __iar_internal_malloc_stats(&imss);

    *pInUse = imss.used;
    *pReserved = imss.fp;
    *pMax = imss.maxfp;
    GP_ASSERT_DEV_INT(GP_KX_HEAP_SIZE == imss.maxfp);
}
#elif defined(__GNUC__)
#if !defined(__SES_ARM)
#include <malloc.h>
// Linker symbol for size of heap
extern const unsigned long _lheap;

void hal_GetHeapInUse(UInt32* pInUse, UInt32* pReserved, UInt32* pMax)
{
    struct mallinfo mi;

    mi = mallinfo();


    *pInUse = mi.uordblks;
    *pReserved = mi.arena;
    *pMax = (UInt32)(UIntPtr)&_lheap;
}
#else
void hal_GetHeapInUse(UInt32* pInUse, UInt32* pReserved, UInt32* pMax)
{
/* Implement hal_GetHeapInUse for the Embedded Studio */
    if(pInUse)
        *pInUse = 0;
    if(pReserved)
        *pReserved = 0;
    if(pMax)
        *pMax = 0;
}
#endif
#else
#error No known heap implementation for other compiler families then IAR/GCC
#endif
#else
void hal_GetHeapInUse(UInt32* pInUse, UInt32* pReserved, UInt32* pMax)
{
    //No Heap
    *pInUse = 0x0;
    *pReserved = 0x0;
    *pMax = 0x0;
}
#endif //GP_KX_HEAP_SIZE
