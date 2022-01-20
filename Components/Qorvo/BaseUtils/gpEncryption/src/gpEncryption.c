/*
 * Copyright (c) 2013-2016, GreenPeak Technologies
 * Copyright (c) 2017-2018, Qorvo Inc
 *
 * gpEncryption.c
 *
 * Contains encryption API
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
 * $Header: //depot/release/Embedded/Components/Qorvo/BaseUtils/v2.10.2.1/comps/gpEncryption/src/gpEncryption.c#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpEncryption.h"
#include "gpHal.h"
#include "gpHal_SEC.h"
#include "gpEncryption_aes_mmo.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID GP_COMPONENT_ID_ENCRYPTION

/*****************************************************************************
 *                   Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static functions
 *****************************************************************************/

#ifdef GP_DIVERSITY_HOST_DECRYPTION
gpEncryption_Result_t Encryption_HostCCMDecrypt(gpEncryption_CCMOptions_t* pCCMOptions)
{
    Bool success;
    gpEncryption_Result_t result;
    unsigned char plaintext[128];
    unsigned char ciphertext[128];
    unsigned char aux[20];
    unsigned char mic[4];

    gpPd_ReadByteStream(pCCMOptions->pdHandle, pCCMOptions->auxOffset, pCCMOptions->auxLength, aux);
    gpPd_ReadByteStream(pCCMOptions->pdHandle, pCCMOptions->dataOffset, pCCMOptions->dataLength, plaintext);
    gpPd_ReadByteStream(pCCMOptions->pdHandle, pCCMOptions->dataOffset + pCCMOptions->dataLength, pCCMOptions->micLength, mic);

    success = hal_CCMDecrypt(ciphertext, pCCMOptions->dataLength, aux, pCCMOptions->auxLength, mic, pCCMOptions->micLength, pCCMOptions->pKey, pCCMOptions->pNonce, plaintext);

    if(success)
    {
        gpPd_WriteByteStream(pCCMOptions->pdHandle, pCCMOptions->dataOffset, pCCMOptions->dataLength, ciphertext);
        result = gpEncryption_ResultSuccess;
    }
    else
    {
        result = gpEncryption_ResultBusy;
    }
    return result;
}
#endif //GP_DIVERSITY_HOST_DECRYPTION



/*****************************************************************************
 *                    Public functions
 *****************************************************************************/

//-------------------------------------------------------------------------------------------------------
//  SYNCHRONOUS SECURITY FUNCTIONS
//-------------------------------------------------------------------------------------------------------
gpEncryption_Result_t gpEncryption_AESEncrypt(UInt8* pInplaceBuffer, UInt8* pAesKey, gpEncryption_AESOptions_t AESoptions)
{
    gpEncryption_Result_t result;
#if   defined(GP_DIVERSITY_HOST_DECRYPTION)
    result = Encryption_HostAESEncrypt(pInplaceBuffer, pAesKey, AESoptions);
#else
    result = gpHal_AESEncrypt(pInplaceBuffer, pAesKey, AESoptions);
#endif
    return result;
}

gpEncryption_Result_t gpEncryption_CCMEncrypt(gpEncryption_CCMOptions_t * pCCMOptions)
{
    gpEncryption_Result_t result;
    result = gpHal_CCMEncrypt(pCCMOptions);
    return result;
}

gpEncryption_Result_t gpEncryption_CCMDecrypt(gpEncryption_CCMOptions_t* pCCMOptions)
{
    gpEncryption_Result_t result;
#if   defined(GP_DIVERSITY_HOST_DECRYPTION)
    result = Encryption_HostCCMDecrypt(pCCMOptions);
#else
    result = gpHal_CCMDecrypt(pCCMOptions);
#endif

    return result;
}

void gpEncryption_Init(void)
{
    gpEncryptionAesMmo_Init();
}

