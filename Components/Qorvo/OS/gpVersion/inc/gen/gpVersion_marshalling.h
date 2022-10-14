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

#ifndef _GPVERSION_MARSHALLING_H_
#define _GPVERSION_MARSHALLING_H_

//DOCUMENTATION VERSION: no @file required as all documented items are refered to a group

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include <global.h>
#include "gpVersion.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct {
    gpVersion_ReleaseInfo_t data[1];
} gpVersion_ReleaseInfo_t_l1_pointer_marshall_t;

typedef struct {
    gpVersion_ReleaseInfo_t data[6];
} gpVersion_ReleaseInfo_t_l6_pointer_marshall_t;


typedef struct {
    gpVersion_SoftwareInfo_t data[1];
} gpVersion_SoftwareInfo_t_l1_pointer_marshall_t;



typedef struct {
    gpVersion_ReleaseInfo_t clientInfo;
} Version_ExchangeGlobalVersion_Input_struct_t;

typedef struct {
    Version_ExchangeGlobalVersion_Input_struct_t data;
    gpVersion_ReleaseInfo_t_l1_pointer_marshall_t clientInfo;
} Version_ExchangeGlobalVersion_Input_marshall_struct_t;

typedef struct {
    gpVersion_VersionStatusResult_t result;
    gpVersion_ReleaseInfo_t* serverInfo;
} Version_ExchangeGlobalVersion_Output_struct_t;

typedef struct {
    Version_ExchangeGlobalVersion_Output_struct_t data;
    gpVersion_ReleaseInfo_t_l1_pointer_marshall_t serverInfo;
} Version_ExchangeGlobalVersion_Output_marshall_struct_t;


typedef struct {
    UInt8 moduleId;
    UInt8 nrOfClientVersions;
    gpVersion_ReleaseInfo_t* pClientVersions;
} Version_NegotiateModuleVersion_Input_struct_t;

typedef struct {
    Version_NegotiateModuleVersion_Input_struct_t data;
    gpVersion_ReleaseInfo_t_l6_pointer_marshall_t pClientVersions;
} Version_NegotiateModuleVersion_Input_marshall_struct_t;

typedef struct {
    gpVersion_VersionStatusResult_t result;
    gpVersion_ReleaseInfo_t* serverInfo;
} Version_NegotiateModuleVersion_Output_struct_t;

typedef struct {
    Version_NegotiateModuleVersion_Output_struct_t data;
    gpVersion_ReleaseInfo_t_l1_pointer_marshall_t serverInfo;
} Version_NegotiateModuleVersion_Output_marshall_struct_t;


typedef struct {
    gpVersion_VersionStatusResult_t result;
} gpVersion_ExchangeCompatibility_Output_struct_t;

typedef struct {
    gpVersion_ExchangeCompatibility_Output_struct_t data;
} gpVersion_ExchangeCompatibility_Output_marshall_struct_t;



typedef union {
    Version_ExchangeGlobalVersion_Input_marshall_struct_t Version_ExchangeGlobalVersion;
    Version_NegotiateModuleVersion_Input_marshall_struct_t Version_NegotiateModuleVersion;
    UInt8 dummy; //ensure none empty union definition
} gpVersion_Server_Input_union_t;

typedef union {
    Version_ExchangeGlobalVersion_Output_marshall_struct_t Version_ExchangeGlobalVersion;
    Version_NegotiateModuleVersion_Output_marshall_struct_t Version_NegotiateModuleVersion;
    gpVersion_ExchangeCompatibility_Output_marshall_struct_t gpVersion_ExchangeCompatibility;
    UInt8 dummy; //ensure none empty union definition
} gpVersion_Server_Output_union_t;
/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Alias/enum copy macro's
#define gpVersion_VersionNumber_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpVersion_VersionNumber_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpVersion_VersionNumber_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpVersion_VersionNumber_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpVersion_VersionStatusResult_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpVersion_VersionStatusResult_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpVersion_VersionStatusResult_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpVersion_VersionStatusResult_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)

// Structure copy functions
gpMarshall_AckStatus_t gpVersion_ReleaseInfo_t_buf2api(gpVersion_ReleaseInfo_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex );
void gpVersion_ReleaseInfo_t_api2buf(UInt8Buffer* pDest , const gpVersion_ReleaseInfo_t* pSource , UInt16 length , UInt16* pIndex);
gpMarshall_AckStatus_t gpVersion_SoftwareInfo_t_buf2api(gpVersion_SoftwareInfo_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex );
void gpVersion_SoftwareInfo_t_api2buf(UInt8Buffer* pDest , const gpVersion_SoftwareInfo_t* pSource , UInt16 length , UInt16* pIndex);
// Server functions
gpMarshall_AckStatus_t Version_ExchangeGlobalVersion_Input_buf2api(Version_ExchangeGlobalVersion_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void Version_ExchangeGlobalVersion_Output_api2buf(UInt8Buffer* pDest , Version_ExchangeGlobalVersion_Output_marshall_struct_t* pSourceoutput , Version_ExchangeGlobalVersion_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t Version_NegotiateModuleVersion_Input_buf2api(Version_NegotiateModuleVersion_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void Version_NegotiateModuleVersion_Output_api2buf(UInt8Buffer* pDest , Version_NegotiateModuleVersion_Output_marshall_struct_t* pSourceoutput , Version_NegotiateModuleVersion_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
void gpVersion_ExchangeCompatibility_Output_api2buf(UInt8Buffer* pDest , gpVersion_ExchangeCompatibility_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);

// Client functions
void Version_ExchangeGlobalVersion_Input_par2buf(UInt8Buffer* pDest , gpVersion_ReleaseInfo_t clientInfo , UInt16* pIndex);
void Version_ExchangeGlobalVersion_Output_buf2par(gpVersion_VersionStatusResult_t* result , gpVersion_ReleaseInfo_t clientInfo , gpVersion_ReleaseInfo_t* serverInfo , UInt8Buffer* pSource , UInt16* pIndex);
void Version_NegotiateModuleVersion_Input_par2buf(UInt8Buffer* pDest , UInt8 moduleId , UInt8 nrOfClientVersions , gpVersion_ReleaseInfo_t* pClientVersions , UInt16* pIndex);
void Version_NegotiateModuleVersion_Output_buf2par(gpVersion_VersionStatusResult_t* result , UInt8 moduleId , UInt8 nrOfClientVersions , gpVersion_ReleaseInfo_t* pClientVersions , gpVersion_ReleaseInfo_t* serverInfo , UInt8Buffer* pSource , UInt16* pIndex);
void gpVersion_ExchangeCompatibility_Output_buf2par(gpVersion_VersionStatusResult_t* result , UInt8Buffer* pSource , UInt16* pIndex);

void gpVersion_InitMarshalling(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // _GPVERSION_MARSHALLING_H_


