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

#ifndef _GPENCRYPTION_MARSHALLING_H_
#define _GPENCRYPTION_MARSHALLING_H_

//DOCUMENTATION ENCRYPTION: no @file required as all documented items are refered to a group

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include <global.h>
#include "gpEncryption.h"
/* <CodeGenerator Placeholder> AdditionalIncludes */
#include "gpPd_marshalling.h"
/* </CodeGenerator Placeholder> AdditionalIncludes */

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct {
    UInt8 pKey[16];
    UInt8 pNonce[13];
} gpEncryption_CCMOptions_t_pointer_marshall_t;

typedef struct {
    gpEncryption_CCMOptions_t data[1];
    gpEncryption_CCMOptions_t_pointer_marshall_t memberpointers[1];
} gpEncryption_CCMOptions_t_l1_pointer_marshall_t;


typedef struct {
    gpEncryption_AESOptions_t data[1];
} gpEncryption_AESOptions_t_l1_pointer_marshall_t;


typedef struct {
    gpEncryption_CCMOptions_t* pCCMOptions;
} gpEncryption_CCMEncrypt_Input_struct_t;

typedef struct {
    gpEncryption_CCMEncrypt_Input_struct_t data;
    gpEncryption_CCMOptions_t_l1_pointer_marshall_t pCCMOptions;
} gpEncryption_CCMEncrypt_Input_marshall_struct_t;

typedef struct {
    gpEncryption_Result_t result;
    UInt8* pDataOut;
} gpEncryption_CCMEncrypt_Output_struct_t;

typedef struct {
    gpEncryption_CCMEncrypt_Output_struct_t data;
    UInt8 pDataOut[128];
} gpEncryption_CCMEncrypt_Output_marshall_struct_t;


typedef struct {
    gpEncryption_CCMOptions_t* pCCMOptions;
} gpEncryption_CCMDecrypt_Input_struct_t;

typedef struct {
    gpEncryption_CCMDecrypt_Input_struct_t data;
    gpEncryption_CCMOptions_t_l1_pointer_marshall_t pCCMOptions;
} gpEncryption_CCMDecrypt_Input_marshall_struct_t;

typedef struct {
    gpEncryption_Result_t result;
    UInt8* pDataOut;
} gpEncryption_CCMDecrypt_Output_struct_t;

typedef struct {
    gpEncryption_CCMDecrypt_Output_struct_t data;
    UInt8 pDataOut[128];
} gpEncryption_CCMDecrypt_Output_marshall_struct_t;


typedef struct {
    UInt8* pInplaceBuffer;
    UInt8* pAesKey;
    gpEncryption_AESOptions_t AESOptions;
} gpEncryption_AESEncrypt_Input_struct_t;

typedef struct {
    gpEncryption_AESEncrypt_Input_struct_t data;
    UInt8 pInplaceBuffer[16];
    UInt8 pAesKey[32];
    gpEncryption_AESOptions_t_l1_pointer_marshall_t AESOptions;
} gpEncryption_AESEncrypt_Input_marshall_struct_t;

typedef struct {
    gpEncryption_Result_t result;
    // pInplaceBuffer used from input structure because it is an inout parameter
} gpEncryption_AESEncrypt_Output_struct_t;

typedef struct {
    gpEncryption_AESEncrypt_Output_struct_t data;
    // pInplaceBuffer used from input structure because it is an inout parameter
} gpEncryption_AESEncrypt_Output_marshall_struct_t;



typedef union {
    gpEncryption_CCMEncrypt_Input_marshall_struct_t gpEncryption_CCMEncrypt;
    gpEncryption_CCMDecrypt_Input_marshall_struct_t gpEncryption_CCMDecrypt;
    gpEncryption_AESEncrypt_Input_marshall_struct_t gpEncryption_AESEncrypt;
    UInt8 dummy; //ensure none empty union definition
} gpEncryption_Server_Input_union_t;

typedef union {
    gpEncryption_CCMEncrypt_Output_marshall_struct_t gpEncryption_CCMEncrypt;
    gpEncryption_CCMDecrypt_Output_marshall_struct_t gpEncryption_CCMDecrypt;
    gpEncryption_AESEncrypt_Output_marshall_struct_t gpEncryption_AESEncrypt;
    UInt8 dummy; //ensure none empty union definition
} gpEncryption_Server_Output_union_t;
/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Alias/enum copy macro's
#define gpEncryption_Result_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpEncryption_Result_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpEncryption_Result_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpEncryption_Result_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpEncryption_SecLevel_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpEncryption_SecLevel_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpEncryption_SecLevel_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpEncryption_SecLevel_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpEncryption_KeyId_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpEncryption_KeyId_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpEncryption_KeyId_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpEncryption_KeyId_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpEncryption_AESKeyLen_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpEncryption_AESKeyLen_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpEncryption_AESKeyLen_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpEncryption_AESKeyLen_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)

// Structure copy functions
gpMarshall_AckStatus_t gpEncryption_CCMOptions_t_buf2api(gpEncryption_CCMOptions_t* pDest ,gpEncryption_CCMOptions_t_pointer_marshall_t* pDestPointers , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex , Bool storePdHandle );
void gpEncryption_CCMOptions_t_api2buf(UInt8Buffer* pDest , const gpEncryption_CCMOptions_t* pSource , UInt16 length , UInt16* pIndex);
gpMarshall_AckStatus_t gpEncryption_AESOptions_t_buf2api(gpEncryption_AESOptions_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex );
void gpEncryption_AESOptions_t_api2buf(UInt8Buffer* pDest , const gpEncryption_AESOptions_t* pSource , UInt16 length , UInt16* pIndex);
// Server functions
gpMarshall_AckStatus_t gpEncryption_CCMEncrypt_Input_buf2api(gpEncryption_CCMEncrypt_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpEncryption_CCMEncrypt_Output_api2buf(UInt8Buffer* pDest , gpEncryption_CCMEncrypt_Output_marshall_struct_t* pSourceoutput , gpEncryption_CCMEncrypt_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpEncryption_CCMDecrypt_Input_buf2api(gpEncryption_CCMDecrypt_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpEncryption_CCMDecrypt_Output_api2buf(UInt8Buffer* pDest , gpEncryption_CCMDecrypt_Output_marshall_struct_t* pSourceoutput , gpEncryption_CCMDecrypt_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpEncryption_AESEncrypt_Input_buf2api(gpEncryption_AESEncrypt_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpEncryption_AESEncrypt_Output_api2buf(UInt8Buffer* pDest , gpEncryption_AESEncrypt_Output_marshall_struct_t* pSourceoutput , gpEncryption_AESEncrypt_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);

// Client functions
void gpEncryption_CCMEncrypt_Input_par2buf(UInt8Buffer* pDest , gpEncryption_CCMOptions_t* pCCMOptions , UInt16* pIndex);
void gpEncryption_CCMEncrypt_Output_buf2par(gpEncryption_Result_t* result , gpEncryption_CCMOptions_t* pCCMOptions , UInt8Buffer* pSource , UInt16* pIndex);
void gpEncryption_CCMDecrypt_Input_par2buf(UInt8Buffer* pDest , gpEncryption_CCMOptions_t* pCCMOptions , UInt16* pIndex);
void gpEncryption_CCMDecrypt_Output_buf2par(gpEncryption_Result_t* result , gpEncryption_CCMOptions_t* pCCMOptions , UInt8Buffer* pSource , UInt16* pIndex);
void gpEncryption_AESEncrypt_Input_par2buf(UInt8Buffer* pDest , UInt8* pInplaceBuffer , UInt8* pAesKey , gpEncryption_AESOptions_t AESOptions , UInt16* pIndex);
void gpEncryption_AESEncrypt_Output_buf2par(gpEncryption_Result_t* result , UInt8* pInplaceBuffer , UInt8* pAesKey , gpEncryption_AESOptions_t AESOptions , UInt8Buffer* pSource , UInt16* pIndex);

void gpEncryption_InitMarshalling(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // _GPENCRYPTION_MARSHALLING_H_


