/*
 * Copyright (c) 2015, GreenPeak Technologies
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

#ifndef _GPRANDOM_MARSHALLING_H_
#define _GPRANDOM_MARSHALLING_H_

//DOCUMENTATION RANDOM: no @file required as all documented items are refered to a group

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include <global.h>
#include "gpRandom.h"
/* <CodeGenerator Placeholder> AdditionalIncludes */
// manual
#include "gpRandom_CTR_DRBG.h"
/* </CodeGenerator Placeholder> AdditionalIncludes */

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/


typedef struct {
    UInt8 nmbrRandomBytes;
} gpRandom_GetNewSequence_Input_struct_t;

typedef struct {
    gpRandom_GetNewSequence_Input_struct_t data;
} gpRandom_GetNewSequence_Input_marshall_struct_t;

typedef struct {
    UInt8* pBuffer;
} gpRandom_GetNewSequence_Output_struct_t;

typedef struct {
    gpRandom_GetNewSequence_Output_struct_t data;
    UInt8 pBuffer[25];
} gpRandom_GetNewSequence_Output_marshall_struct_t;







typedef struct {
    UInt8 nmbrRandomBytes;
} gpRandom_GetFromDRBG_Input_struct_t;

typedef struct {
    gpRandom_GetFromDRBG_Input_struct_t data;
} gpRandom_GetFromDRBG_Input_marshall_struct_t;

typedef struct {
    UInt8* pBuffer;
} gpRandom_GetFromDRBG_Output_struct_t;

typedef struct {
    gpRandom_GetFromDRBG_Output_struct_t data;
    UInt8 pBuffer[255];
} gpRandom_GetFromDRBG_Output_marshall_struct_t;


typedef union {
    gpRandom_GetNewSequence_Input_marshall_struct_t gpRandom_GetNewSequence;
    gpRandom_GetFromDRBG_Input_marshall_struct_t gpRandom_GetFromDRBG;
    UInt8 dummy; //ensure none empty union definition
} gpRandom_Server_Input_union_t;

typedef union {
    gpRandom_GetNewSequence_Output_marshall_struct_t gpRandom_GetNewSequence;
    gpRandom_GetFromDRBG_Output_marshall_struct_t gpRandom_GetFromDRBG;
    UInt8 dummy; //ensure none empty union definition
} gpRandom_Server_Output_union_t;
/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Alias/enum copy macro's
#define gpRandom_Result_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpRandom_Result_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpRandom_Result_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpRandom_Result_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)

// Structure copy functions
// Server functions
gpMarshall_AckStatus_t gpRandom_GetNewSequence_Input_buf2api(gpRandom_GetNewSequence_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpRandom_GetNewSequence_Output_api2buf(UInt8Buffer* pDest , gpRandom_GetNewSequence_Output_marshall_struct_t* pSourceoutput , gpRandom_GetNewSequence_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpRandom_GetFromDRBG_Input_buf2api(gpRandom_GetFromDRBG_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpRandom_GetFromDRBG_Output_api2buf(UInt8Buffer* pDest , gpRandom_GetFromDRBG_Output_marshall_struct_t* pSourceoutput , gpRandom_GetFromDRBG_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);

// Client functions
void gpRandom_GetNewSequence_Input_par2buf(UInt8Buffer* pDest , UInt8 nmbrRandomBytes , UInt16* pIndex);
void gpRandom_GetNewSequence_Output_buf2par(UInt8 nmbrRandomBytes , UInt8* pBuffer , UInt8Buffer* pSource , UInt16* pIndex);
void gpRandom_GetFromDRBG_Input_par2buf(UInt8Buffer* pDest , UInt8 nmbrRandomBytes , UInt16* pIndex);
void gpRandom_GetFromDRBG_Output_buf2par(UInt8 nmbrRandomBytes , UInt8* pBuffer , UInt8Buffer* pSource , UInt16* pIndex);

void gpRandom_InitMarshalling(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // _GPRANDOM_MARSHALLING_H_


