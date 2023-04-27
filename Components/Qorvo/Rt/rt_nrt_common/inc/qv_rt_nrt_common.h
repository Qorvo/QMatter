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
 * $Header$
 * $Change$
 * $DateTime$
 *
 */

#ifndef _QV_RT_NRT_COMMON_H_
#define _QV_RT_NRT_COMMON_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "qv_rt_nrt_platform.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define T_IFS               150

#define RT_EVENT_HANDLE_INVALID    0xFF
#define TTB_EVENT_HANDLE_INVALID    0xFF

#define RT_MAX_NR_OF_EVENTS          16

/*****************************************************************************
 *                   Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef UInt8 rt_event_handle_t;
typedef UInt8 rt_event_type_t;
typedef uintptr_t rt_event_ptr_t;

/*****************************************************************************
 *                    External Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#endif //_QV_RT_NRT_COMMON_H_
