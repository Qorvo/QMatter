/*
 * Copyright (c) 2020, Qorvo Inc
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
 */

#ifndef _GPRADIO_MARSHALLING_H_
#define _GPRADIO_MARSHALLING_H_

//DOCUMENTATION RADIO: no @file required as all documented items are refered to a group

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include <global.h>
#include "gpRadio.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/


typedef struct {
    Bool enableMultiStandard;
    Bool enableMultiChannel;
    Bool enableHighSensitivity;
} gpRadio_SetRxMode_Input_struct_t;

typedef struct {
    gpRadio_SetRxMode_Input_struct_t data;
} gpRadio_SetRxMode_Input_marshall_struct_t;

typedef struct {
    gpRadio_Status_t status;
} gpRadio_SetRxMode_Output_struct_t;

typedef struct {
    gpRadio_SetRxMode_Output_struct_t data;
} gpRadio_SetRxMode_Output_marshall_struct_t;


typedef struct {
    UInt8 rxAntenna;
} gpRadio_SetRxAntenna_Input_struct_t;

typedef struct {
    gpRadio_SetRxAntenna_Input_struct_t data;
} gpRadio_SetRxAntenna_Input_marshall_struct_t;

typedef struct {
    gpRadio_Status_t status;
} gpRadio_SetRxAntenna_Output_struct_t;

typedef struct {
    gpRadio_SetRxAntenna_Output_struct_t data;
} gpRadio_SetRxAntenna_Output_marshall_struct_t;


typedef struct {
    gpRadio_Status_t status;
    Bool* enableMultiStandard;
    Bool* enableMultiChannel;
    Bool* enableHighSensitivity;
} gpRadio_GetRxMode_Output_struct_t;

typedef struct {
    gpRadio_GetRxMode_Output_struct_t data;
    Bool enableMultiStandard[1];
    Bool enableMultiChannel[1];
    Bool enableHighSensitivity[1];
} gpRadio_GetRxMode_Output_marshall_struct_t;


typedef struct {
    UInt8 rxAntenna;
} gpRadio_GetRxAntenna_Output_struct_t;

typedef struct {
    gpRadio_GetRxAntenna_Output_struct_t data;
} gpRadio_GetRxAntenna_Output_marshall_struct_t;


typedef struct {
    UInt8 firFilter;
} gpRadio_SetRadioFirFilter_Input_struct_t;

typedef struct {
    gpRadio_SetRadioFirFilter_Input_struct_t data;
} gpRadio_SetRadioFirFilter_Input_marshall_struct_t;

typedef struct {
    gpRadio_Status_t status;
} gpRadio_SetRadioFirFilter_Output_struct_t;

typedef struct {
    gpRadio_SetRadioFirFilter_Output_struct_t data;
} gpRadio_SetRadioFirFilter_Output_marshall_struct_t;


typedef union {
    gpRadio_SetRxMode_Input_marshall_struct_t gpRadio_SetRxMode;
    gpRadio_SetRxAntenna_Input_marshall_struct_t gpRadio_SetRxAntenna;
    gpRadio_SetRadioFirFilter_Input_marshall_struct_t gpRadio_SetRadioFirFilter;
    UInt8 dummy; //ensure none empty union definition
} gpRadio_Server_Input_union_t;

typedef union {
    gpRadio_SetRxMode_Output_marshall_struct_t gpRadio_SetRxMode;
    gpRadio_SetRxAntenna_Output_marshall_struct_t gpRadio_SetRxAntenna;
    gpRadio_GetRxMode_Output_marshall_struct_t gpRadio_GetRxMode;
    gpRadio_GetRxAntenna_Output_marshall_struct_t gpRadio_GetRxAntenna;
    gpRadio_SetRadioFirFilter_Output_marshall_struct_t gpRadio_SetRadioFirFilter;
    UInt8 dummy; //ensure none empty union definition
} gpRadio_Server_Output_union_t;
/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Alias/enum copy macro's
#define gpRadio_Status_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpRadio_Status_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpRadio_Status_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpRadio_Status_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)

// Structure copy functions
// Server functions
gpMarshall_AckStatus_t gpRadio_SetRxMode_Input_buf2api(gpRadio_SetRxMode_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpRadio_SetRxMode_Output_api2buf(UInt8Buffer* pDest , gpRadio_SetRxMode_Output_marshall_struct_t* pSourceoutput , gpRadio_SetRxMode_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpRadio_SetRxAntenna_Input_buf2api(gpRadio_SetRxAntenna_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpRadio_SetRxAntenna_Output_api2buf(UInt8Buffer* pDest , gpRadio_SetRxAntenna_Output_marshall_struct_t* pSourceoutput , gpRadio_SetRxAntenna_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
void gpRadio_GetRxMode_Output_api2buf(UInt8Buffer* pDest , gpRadio_GetRxMode_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
void gpRadio_GetRxAntenna_Output_api2buf(UInt8Buffer* pDest , gpRadio_GetRxAntenna_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpRadio_SetRadioFirFilter_Input_buf2api(gpRadio_SetRadioFirFilter_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpRadio_SetRadioFirFilter_Output_api2buf(UInt8Buffer* pDest , gpRadio_SetRadioFirFilter_Output_marshall_struct_t* pSourceoutput , gpRadio_SetRadioFirFilter_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);

// Client functions
void gpRadio_SetRxMode_Input_par2buf(UInt8Buffer* pDest , Bool enableMultiStandard , Bool enableMultiChannel , Bool enableHighSensitivity , UInt16* pIndex);
void gpRadio_SetRxMode_Output_buf2par(gpRadio_Status_t* status , Bool enableMultiStandard , Bool enableMultiChannel , Bool enableHighSensitivity , UInt8Buffer* pSource , UInt16* pIndex);
void gpRadio_SetRxAntenna_Input_par2buf(UInt8Buffer* pDest , UInt8 rxAntenna , UInt16* pIndex);
void gpRadio_SetRxAntenna_Output_buf2par(gpRadio_Status_t* status , UInt8 rxAntenna , UInt8Buffer* pSource , UInt16* pIndex);
void gpRadio_GetRxMode_Output_buf2par(gpRadio_Status_t* status , Bool* enableMultiStandard , Bool* enableMultiChannel , Bool* enableHighSensitivity , UInt8Buffer* pSource , UInt16* pIndex);
void gpRadio_GetRxAntenna_Output_buf2par(UInt8* rxAntenna , UInt8Buffer* pSource , UInt16* pIndex);
void gpRadio_SetRadioFirFilter_Input_par2buf(UInt8Buffer* pDest , UInt8 firFilter , UInt16* pIndex);
void gpRadio_SetRadioFirFilter_Output_buf2par(gpRadio_Status_t* status , UInt8 firFilter , UInt8Buffer* pSource , UInt16* pIndex);

void gpRadio_InitMarshalling(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // _GPRADIO_MARSHALLING_H_


