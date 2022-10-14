/*
 * Copyright (c) 2015-2016, GreenPeak Technologies
 * Copyright (c) 2017-2019, Qorvo Inc
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
 */

#ifndef _GPASSERT_MARSHALLING_H_
#define _GPASSERT_MARSHALLING_H_

//DOCUMENTATION ASSERT: no @file required as all documented items are refered to a group

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include <global.h>
#include "gpAssert.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/





typedef struct {
    UInt16 componentId;
    UInt16 lineNumber;
} gpAssert_cbIndication_Input_struct_t;

typedef struct {
    gpAssert_cbIndication_Input_struct_t data;
} gpAssert_cbIndication_Input_marshall_struct_t;


typedef union {
    gpAssert_cbIndication_Input_marshall_struct_t gpAssert_cbIndication;
    UInt8 dummy; //ensure none empty union definition
} gpAssert_Client_Input_union_t;

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Alias/enum copy macro's

// Structure copy functions
// Server functions
void gpAssert_cbIndication_Input_par2api(UInt8Buffer* pDest , UInt16 componentId , UInt16 lineNumber , UInt16* pIndex);

// Client functions
gpMarshall_AckStatus_t gpAssert_cbIndication_Input_buf2api(gpAssert_cbIndication_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);

void gpAssert_InitMarshalling(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // _GPASSERT_MARSHALLING_H_


