/*
 * Copyright (c) 2014-2016, GreenPeak Technologies
 * Copyright (c) 2017-2018, Qorvo Inc
 *
 * gpHal_SEC.c
 *
 * Contains all security functionality of the HAL
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

// #define GP_LOCAL_LOG

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpHal.h"
#include "gpHal_DEFS.h"
#include "gpHal_SEC.h"
#include "gpHal_Pbm.h"

//GP hardware dependent register definitions
#include "gpHal_HW.h"
#include "gpHal_reg.h"

#include "gpAssert.h"

#ifdef GP_HAL_DIVERSITY_SEC_CRYPTOSOC
#include "sx_aes.h"
#include "cryptolib_def.h"
#include "sx_generic.h"
#include "sx_hash.h"
#endif //GP_HAL_DIVERSITY_SEC_CRYPTOSOC

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID GP_COMPONENT_ID_GPHAL

#define GP_HAL_KEY_128      16 // bytes
#define GP_HAL_DATA_LEN     16 // bytes
#define GP_HAL_KEY_256      32 // bytes
#define GP_HAL_MAXKEY_LEN   GP_HAL_KEY_256
#define GP_HAL_NONCE_104    13 // bytes


#define GP_HAL_SEC_USER_KEY_ID_TO_LOCATION(id)      (GP_WB_NVR_USER_KEY_0_LSB_ADDRESS    + (id                                )*(GP_WB_NVR_USER_KEY_0_LSB_LEN   +GP_WB_NVR_USER_KEY_0_MSB_LEN)   )
#define GP_HAL_SEC_PRODUCT_KEY_ID_TO_LOCATION(id)   (GP_WB_NVR_PRODUCT_KEY_0_LSB_ADDRESS + (id - gpEncryption_KeyIdProductKey0)*(GP_WB_NVR_PRODUCT_KEY_0_LSB_LEN+GP_WB_NVR_PRODUCT_KEY_0_MSB_LEN))



/*****************************************************************************
 *                   Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct {
    UInt8 key[GP_HAL_MAXKEY_LEN];
    UInt8 data[GP_HAL_DATA_LEN];
} gpHal_AESBuffer_t;

typedef struct {
    UInt8 key[GP_HAL_KEY_128];
    UInt8 nonce[GP_HAL_NONCE_104];
} gpHal_CCMBuffer_t;

typedef union {
    gpHal_AESBuffer_t aes;
    gpHal_CCMBuffer_t ccm;
} gpHal_SecurityRamBuffer_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

#define LINKER_SECTION_LOWER_RAM_RETAIN LINKER_SECTION(".lower_ram_retain")

// Key buffer must be 32-byte aligned.
COMPILER_ALIGNED(32) static gpHal_SecurityRamBuffer_t gpHal_SecRamBuffer LINKER_SECTION_LOWER_RAM_RETAIN;

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/


#define gpHalSec_Dump()



#ifdef GP_HAL_DIVERSITY_SEC_CRYPTOSOC

/** @brief Translate from Silex result code to gpHal result code. */
static INLINE gpHal_Result_t gpHalSec_MapSilexResult(UInt32 ret)
{
    switch (ret)
    {
        case CRYPTOLIB_SUCCESS:
            return gpHal_ResultSuccess;
        case CRYPTOLIB_INVALID_PARAM:
            return gpHal_ResultInvalidParameter;
        case CRYPTOLIB_INVALID_SIGN_ERR:
            // MAC mismatch
            return gpHal_ResultInvalidParameter;
        default:
            // hardware error ?
            return gpHal_ResultBusy;
    }
}

/** @brief Perform AES encryption in Silex CryptoSoc engine.
 *
 *  @param dataPtr   Pointer to 16-byte data buffer for input and output, non-compressed RAM address.
 *  @param keyPtr    Pointer to key buffer, non-compressed RAM address.
 *  @param keylen    Key length.
 *  @param hardened  True to use internal secret key.
 */
static INLINE gpHal_Result_t gpHalSec_CryptoSocAesEncrypt(UInt32 dataPtr, UInt32 keyPtr, gpEncryption_AESKeyLen_t keylen, Bool hardened)
{

    if(keylen == gpEncryption_AESKeyLen192)
    {
        /* Key length not supported */
        return gpHal_ResultInvalidParameter;
    }

    if(keylen == gpEncryption_AESKeyLen256 && hardened)
    {
        return gpHal_ResultInvalidParameter;
    }



    sx_enable_clock();

    // Optional hardening with secret key.
    GP_WB_WRITE_SEC_PROC_CTRL_USE_SKEY(hardened ? 1 : 0);

    // Call Silex CryptoSoc AES function.
    UInt32 keyLen = (keylen == gpEncryption_AESKeyLen128) ? 16 : (keylen == gpEncryption_AESKeyLen256) ? 32 : 24;
    UInt32 ret = sx_aes_blk(
        ECB,                        // aes_fct
        ENC,                        // dir
        CTX_WHOLE,                  // ctx
        block_t_convert((void*)keyPtr, keyLen), // key
        block_t_convert(NULL, 0),   // xtskey
        block_t_convert(NULL, 0),   // iv
        block_t_convert((void*)dataPtr, GP_HAL_DATA_LEN), // data_in
        block_t_convert((void*)dataPtr, GP_HAL_DATA_LEN), // data_out
        block_t_convert(NULL, 0),   // aad
        block_t_convert(NULL, 0),   // tag
        block_t_convert(NULL, 0),   // ctx_ptr
        block_t_convert(NULL, 0)    // nonce_len_blk
    );

    // Disable hardening - other users of Silex library (TLS) will not expect it.
    GP_WB_WRITE_SEC_PROC_CTRL_USE_SKEY(0);

    sx_disable_clock();

    return gpHalSec_MapSilexResult(ret);
}

/** @brief Perform CCM encryption/decryption in CryptoSoc engine.
 *
 *  During encryption, MIC will be written immediately after encrypted data.
 *  During decryption, MIC is verified and verification reflected in result code. MIC check bytes are not written to the data buffer.
 *
 *  @param pKey      Pointer to 16-byte key.
 *  @param pNonce    Pointer to 13-byte nonce.
 *  @param ptrBuf    Pointer to buffer for input and output (non-compressed RAM address).
 *  @param ptrBufAux Pointer to buffer for additional authentication data (non-compressed RAM address).
 *  @param dataLen   Data length in bytes.
 *  @param micLen    MIC length in bytes.
 *  @param auxLen    Additional authentication data length in bytes.
 *  @param decrypt   True to decrypt, false to encrypt.
 */
static INLINE gpHal_Result_t gpHalSec_CryptoSocCcmOperation(const UInt8* pKey, const UInt8* pNonce, gpHal_Address_t ptrBuf, gpHal_Address_t ptrBufAux, UInt8 dataLen, UInt8 micLen, UInt8 auxLen, Bool modeDecrypt)
{
    sx_enable_clock();
    // Disable hardening (not used with CCM).
    GP_WB_WRITE_SEC_PROC_CTRL_USE_SKEY(0);

    // Call Silex CryptoSoc AES function.
    UInt32 ret = sx_aes_blk(
        CCM,                        // aes_fct
        (modeDecrypt) ? DEC : ENC,  // dir
        CTX_WHOLE,                  // ctx
        block_t_convert((void*)pKey, GP_HAL_KEY_128), // key
        block_t_convert(NULL, 0),   // xtskey
        block_t_convert(NULL, 0),   // iv
        block_t_convert((void*)ptrBuf, dataLen),   // data_in
        block_t_convert((void*)ptrBuf, dataLen),   // data_out
        block_t_convert((void*)ptrBufAux, auxLen), // aad
        block_t_convert((void*)(ptrBuf + dataLen), micLen), // tag
        block_t_convert(NULL, 0),   // ctx_ptr
        block_t_convert((void*)pNonce, GP_HAL_NONCE_104) // nonce_len_blk
    );
    sx_disable_clock();

    return gpHalSec_MapSilexResult(ret);
}

#else //GP_HAL_DIVERSITY_SEC_CRYPTOSOC

/** @brief Perform AES encryption in SSP engine.
 *
 *  @param dataPtr   Pointer to 16-byte data buffer for input and output, non-compressed RAM address.
 *  @param keyPtr    Pointer to key buffer, compressed RAM address.
 *  @param keylen    Key length.
 *  @param hardened  True to use internal secret key.
 */
static INLINE gpHal_Result_t gpHalSec_SspAesEncrypt(UInt32 dataPtr, UInt32 keyPtr, gpEncryption_AESKeyLen_t keylen, Bool hardened)
{
    //Set Data input and output Ptr
    GP_WB_WRITE_SSP_MSG_PTR(GP_MM_RAM_ADDR_TO_COMPRESSED(dataPtr));
    GP_WB_WRITE_SSP_MSG_OUT_PTR(GP_MM_RAM_ADDR_TO_COMPRESSED(dataPtr));

    //Set encryption key
    GP_WB_WRITE_SSP_KEY_PTR(keyPtr);

    //Harden with secret key
    if (hardened)
    {
        GP_WB_WRITE_SSP_USE_SKEY(1);
    }
    else
    {
        GP_WB_WRITE_SSP_USE_SKEY(0);
    }

    //Set the length
    GP_WB_WRITE_SSP_KEY_LEN( (keylen == gpEncryption_AESKeyLen128) ? GP_WB_ENUM_SSP_KEY_LEN_KEY_128 : (keylen == gpEncryption_AESKeyLen256) ? GP_WB_ENUM_SSP_KEY_LEN_KEY_256: GP_WB_ENUM_SSP_KEY_LEN_KEY_192 );
    GP_WB_WRITE_SSP_MSG_LEN(GP_HAL_DATA_LEN);

    //Set the mode
    GP_WB_WRITE_SSP_MODE(GP_WB_ENUM_SSP_MODE_AES);

    //Start encryption and poll for finish
    GP_WB_SSP_START_ENCRYPT();
    __DSB();

    // On high clock speeds, it is still possible we read out the status before the hardware was able to update.
    // Add an additional wait
    HAL_WAIT_US(1);

    GP_DO_WHILE_TIMEOUT_ASSERT(GP_WB_READ_SSP_BUSY(), GP_HAL_DEFAULT_TIMEOUT);

    //Assert when memory error occured
    GP_ASSERT_DEV_EXT(0 == GP_WB_READ_SSP_MEM_ERR());

    return gpHal_ResultSuccess;
}


/** @brief Perform CCM encryption/decryption in SSP engine.
 *
 *  During encryption, MIC will be written immediately after encrypted data.
 *  During decryption, MIC check bytes will be written immediately after decrypted data.
 *  Caller must check that these are all-zero bytes.
 *
 *  @param pKey      Pointer to 16-byte key.
 *  @param pNonce    Pointer to 13-byte nonce.
 *  @param ptrBuf    Pointer to buffer for input and output (non-compressed RAM address).
 *  @param ptrBufAux Pointer to buffer for additional authentication data (non-compressed RAM address).
 *  @param dataLen   Data length in bytes.
 *  @param micLen    MIC length in bytes.
 *  @param auxLen    Additional authentication data length in bytes.
 *  @param decrypt   True to decrypt, false to encrypt.
 */
static INLINE gpHal_Result_t gpHalSec_SspCcmOperation(const UInt8* pKey, const UInt8* pNonce, gpHal_Address_t ptrBuf, gpHal_Address_t ptrBufAux, UInt8 dataLen, UInt8 micLen, UInt8 auxLen, Bool modeDecrypt)
{
    GP_WB_WRITE_SSP_MSG_PTR(GP_MM_RAM_ADDR_TO_COMPRESSED(ptrBuf));
    GP_WB_WRITE_SSP_MSG_OUT_PTR(GP_MM_RAM_ADDR_TO_COMPRESSED(ptrBuf));
    GP_WB_WRITE_SSP_MSG_LEN(dataLen);

    //Write micPtr and length, mic will be placed after data
    GP_WB_WRITE_SSP_MIC_PTR(GP_MM_RAM_ADDR_TO_COMPRESSED(ptrBuf + dataLen));
    GP_WB_WRITE_SSP_MIC_LEN(micLen >> 1); //MIC length divide by 2!

    //Set the auxPtr and length, if needed linearize aux data
    //aux is only input for security engine so no need to delinearize and restore data later on
    GP_WB_WRITE_SSP_A_PTR(GP_MM_RAM_ADDR_TO_COMPRESSED(ptrBufAux));
    GP_WB_WRITE_SSP_A_LEN(auxLen);

    //Write key + keyPtr
    //Write Nonce + noncePtr
#ifdef GP_DIVERSITY_GPHAL_INTERN
    MEMCPY(gpHal_SecRamBuffer.ccm.key, pKey, GP_HAL_KEY_128);
    MEMCPY(gpHal_SecRamBuffer.ccm.nonce, pNonce, GP_HAL_NONCE_104);
    GP_WB_WRITE_SSP_KEY_PTR(GP_MM_RAM_ADDR_TO_COMPRESSED((gpHal_Address_t)gpHal_SecRamBuffer.ccm.key));
    GP_WB_WRITE_SSP_NONCE_PTR(GP_MM_RAM_ADDR_TO_COMPRESSED((gpHal_Address_t)gpHal_SecRamBuffer.ccm.nonce));
#else //GP_DIVERSITY_GPHAL_INTERN
    GP_HAL_WRITE_BYTE_STREAM(GP_HAL_SEC_KEY_STORAGE, pKey, GP_HAL_KEY_128);
    GP_HAL_WRITE_BYTE_STREAM(GP_HAL_SEC_NONCE_STORAGE, pNonce, GP_HAL_NONCE_104);
    GP_WB_WRITE_SSP_KEY_PTR(GP_MM_RAM_ADDR_TO_COMPRESSED(GP_HAL_SEC_KEY_STORAGE));
    GP_WB_WRITE_SSP_NONCE_PTR(GP_MM_RAM_ADDR_TO_COMPRESSED(GP_HAL_SEC_NONCE_STORAGE));
#endif //GP_DIVERSITY_GPHAL_INTERN

    GP_WB_WRITE_SSP_KEY_LEN(GP_WB_ENUM_SSP_KEY_LEN_KEY_128);

    //Set the mode & start encryption/decryption
    GP_WB_WRITE_SSP_USE_SKEY(0);
    GP_WB_WRITE_SSP_MODE(modeDecrypt ? GP_WB_ENUM_SSP_MODE_DECRYPT : GP_WB_ENUM_SSP_MODE_ENCRYPT);
    GP_WB_SSP_START_ENCRYPT();
    __DSB();

    // On high clock speeds, it is still possible we read out the status before the hardware was able to update.
    // Add an additional wait
    HAL_WAIT_US(1);

    //Polled wait for encryption to finish & assert when memory error occured
    GP_DO_WHILE_TIMEOUT_ASSERT(GP_WB_READ_SSP_BUSY(), GP_HAL_DEFAULT_TIMEOUT);
    GP_ASSERT_DEV_EXT(0 == GP_WB_READ_SSP_MEM_ERR());

    return gpHal_ResultSuccess;
}

#endif //GP_HAL_DIVERSITY_SEC_CRYPTOSOC

/** @brief Perform AES MMO encryption in SSP engine.
 *
 *  @param dataPtr   Pointer to 16-byte data buffer for input and output, non-compressed RAM address.
 *  @param keyPtr    Pointer to key buffer, compressed RAM address.
 *  @param keylen    Key length.
 *  @param hardened  True to use internal secret key.
 */
void gpHalSec_SspAesMMO(UInt32 compressedDataPtr,
    UInt32 compressedKeyPtr,
    gpEncryption_AESKeyLen_t keylen,
    UInt8 msgLengthBytes)
{
    GP_LOG_PRINTF("aesmmo: dp %lx kp %lx",0,compressedDataPtr, compressedKeyPtr);
    //Set input message pointer
    GP_WB_WRITE_SSP_MSG_PTR(compressedDataPtr);
    //Set encryption key
    GP_WB_WRITE_SSP_KEY_PTR(compressedKeyPtr);
    // generated digest value is stored back in keyptr
    GP_WB_WRITE_SSP_MSG_OUT_PTR(compressedKeyPtr);

    // hardening is disabled for aes mmo
    GP_WB_WRITE_SSP_USE_SKEY(0);

    //Set the length
    GP_WB_WRITE_SSP_KEY_LEN( (keylen == gpEncryption_AESKeyLen128) ? GP_WB_ENUM_SSP_KEY_LEN_KEY_128 : (keylen == gpEncryption_AESKeyLen256) ? GP_WB_ENUM_SSP_KEY_LEN_KEY_256: GP_WB_ENUM_SSP_KEY_LEN_KEY_192 );
    GP_ASSERT_DEV_EXT(msgLengthBytes % keylen == 0);

    GP_WB_WRITE_SSP_MSG_LEN(msgLengthBytes);

    //Set the mode
    GP_WB_WRITE_SSP_MODE(GP_WB_ENUM_SSP_MODE_AES);
    GP_WB_WRITE_SSP_AES_MMO(1);

    //Start encryption and poll for finish
    GP_WB_SSP_START_ENCRYPT();
    __DSB();

    // On high clock speeds, it is still possible we read out the status before the hardware was able to update.
    // Add an additional wait
    HAL_WAIT_US(1);

    GP_DO_WHILE_TIMEOUT_ASSERT(GP_WB_READ_SSP_BUSY(), GP_HAL_DEFAULT_TIMEOUT);

    //Assert when memory error occured
    GP_ASSERT_DEV_EXT(0 == GP_WB_READ_SSP_MEM_ERR());

    GP_WB_WRITE_SSP_AES_MMO(0);
    return;
}

/** Perform CCM encryption or decryption, depending on mode parameter. */
static gpHal_Result_t gpHalSec_CCMbase(gpEncryption_CCMOptions_t* pCCMOptions, Bool modeDecrypt)
{
    UInt8 PBMhandle;
    UInt8 dataWithMicLength;
    gpHal_Address_t tempPtrBuf;
    gpHal_Address_t tempPtrBufAux;
    UInt8 checkHandle;
    Bool wrappeablePbm;
    gpHal_Result_t result;

    GP_ASSERT_DEV_EXT(NULL != pCCMOptions);
    GP_ASSERT_DEV_EXT(NULL != pCCMOptions->pKey);
    GP_ASSERT_DEV_EXT(NULL != pCCMOptions->pNonce);


    wrappeablePbm = (gpPd_GetPdType(pCCMOptions->pdHandle) == gpPd_BufferTypeZigBee)? true: false;



    if(wrappeablePbm)
    {
        if ((pCCMOptions->dataLength + pCCMOptions->auxLength + pCCMOptions->micLength) > 0x80)
        {
            // return Invalid Parameter(payload size excess limit) to upper layer
            return gpHal_ResultInvalidParameter;
        }
        pCCMOptions->auxOffset &= 0x7f;
        pCCMOptions->dataOffset &= 0x7f;
    }

    dataWithMicLength = pCCMOptions->dataLength + pCCMOptions->micLength;

    //Copy data and aux from Pd to Pbm in case of pdram
    PBMhandle = gpPd_SecRequest(pCCMOptions->pdHandle,
                                pCCMOptions->dataOffset,
                                //For decryption we also need to copy mic from pd to pbm !
                                (modeDecrypt) ? dataWithMicLength : pCCMOptions->dataLength,
                                pCCMOptions->auxOffset,
                                pCCMOptions->auxLength);
    if (!GP_HAL_CHECK_PBM_VALID(PBMhandle))
    {
        return gpHal_ResultBusy;
    }

    if (wrappeablePbm)
    {
        UInt16 pbmStartPtr = 0x0;
        GP_HAL_READ_REGS16(GP_WB_MM_PBM_0_DATA_BASE_ADDRESS_ADDRESS + PBMhandle*2, &pbmStartPtr); //Relative to RAM linear start

        // pbm window base is 4 bytes resolution, circ window base only 1 ==> convert
        pbmStartPtr <<= 2;
        // We can use the circular window approach
        GP_WB_WRITE_MM_CIRC_WINDOW_0_BASE_ADDRESS(pbmStartPtr); //address Relative to RAM linear start - already OK, we read it out the PBM window address register
        tempPtrBuf = GP_MM_RAM_CIRC_WINDOW_LEFT_0_START + pCCMOptions->dataOffset;
        tempPtrBufAux = GP_MM_RAM_CIRC_WINDOW_LEFT_0_START + pCCMOptions->auxOffset;
    }
    else
    {
        UInt32 pbmStartAddress = GP_HAL_PBM_ENTRY2ADDR(PBMhandle);
        // No wrappable pbm, use base address
        tempPtrBuf = pbmStartAddress + pCCMOptions->dataOffset;
        tempPtrBufAux = pbmStartAddress + pCCMOptions->auxOffset;
    }

#ifdef GP_HAL_DIVERSITY_SEC_CRYPTOSOC
    result = gpHalSec_CryptoSocCcmOperation(
        pCCMOptions->pKey,
        pCCMOptions->pNonce,
        tempPtrBuf,
        tempPtrBufAux,
        pCCMOptions->dataLength,
        pCCMOptions->micLength,
        pCCMOptions->auxLength,
        modeDecrypt
    );
#else //GP_HAL_DIVERSITY_SEC_CRYPTOSOC
    result = gpHalSec_SspCcmOperation(
        pCCMOptions->pKey,
        pCCMOptions->pNonce,
        tempPtrBuf,
        tempPtrBufAux,
        pCCMOptions->dataLength,
        pCCMOptions->micLength,
        pCCMOptions->auxLength,
        modeDecrypt
    );
#endif //GP_HAL_DIVERSITY_SEC_CRYPTOSOC

    //returned pdHandle should be te same, copy mic aswell for decryption !
    checkHandle = gpPd_cbSecConfirm(PBMhandle, pCCMOptions->dataOffset, dataWithMicLength);
    GP_ASSERT_DEV_EXT(checkHandle == pCCMOptions->pdHandle);

    if (result != gpHal_ResultSuccess)
    {
        return result;
    }

#ifdef GP_HAL_DIVERSITY_SEC_CRYPTOSOC
    // Do not verify the MIC after decryption.
    // The Silex library verifies it internally and zero MIC is not written to buffer.
#else
    //If decryption was succesfull, check MIC (should be all zero)
    if (modeDecrypt)
    {
        UInt8 micData[16] = {0}; //Maximum MIC length
        UIntLoop i;
        //Mic will be after data, check if all 0 after decryption
        gpPd_ReadByteStream(pCCMOptions->pdHandle, pCCMOptions->dataOffset + pCCMOptions->dataLength, pCCMOptions->micLength, micData);

        for (i = 0; i < pCCMOptions->micLength; ++i)
        {
            if (0 != micData[i])
            {
                //Decryption failed
                return gpHal_ResultInvalidParameter;
            }
        }
    }
#endif

    return gpHal_ResultSuccess;
}

/*****************************************************************************
 *                    Public functions
 *****************************************************************************/

//-------------------------------------------------------------------------------------------------------
//  SYNCHRONOUS SECURITY FUNCTIONS
//-------------------------------------------------------------------------------------------------------

gpHal_Result_t gpHal_AESEncrypt(UInt8* pInplaceBuffer, UInt8* pAESKey, gpEncryption_AESOptions_t AESOptions)
{
    UInt8* pPatchedAesKey;
    gpEncryption_KeyId_t keyId;
    Bool hardened;
    //24-bit address range
    UInt32 dataPtr;
    UInt32 keyPtr;
#ifndef GP_HAL_DIVERSITY_SEC_CRYPTOSOC
    UInt32 compressedKeyPtr;
#endif //GP_HAL_DIVERSITY_SEC_CRYPTOSOC
    gpHal_Result_t result;

    if( AESOptions.keylen != gpEncryption_AESKeyLen128 && AESOptions.keylen != gpEncryption_AESKeyLen256 && AESOptions.keylen != gpEncryption_AESKeyLen192 )
    {
        return gpHal_ResultInvalidParameter;
    }

    MEMCPY(gpHal_SecRamBuffer.aes.data, pInplaceBuffer, GP_HAL_DATA_LEN);

    keyId = GP_ENCRYPTION_OPTIONS_GET_KEYID(AESOptions.options);
    hardened = GP_ENCRYPTION_OPTIONS_IS_HARDENED(AESOptions.options);

    dataPtr = (UIntPtr)(gpHal_SecRamBuffer.aes.data);





    //Select encryption key
    if(GP_ENCRYPTION_KEYID_IS_USER(keyId))
    {
        keyPtr = GP_HAL_SEC_USER_KEY_ID_TO_LOCATION(keyId);
#ifndef GP_HAL_DIVERSITY_SEC_CRYPTOSOC
        compressedKeyPtr = GP_MM_FLASH_ALT_ADDR_TO_COMPRESSED(keyPtr);
#endif //GP_HAL_DIVERSITY_SEC_CRYPTOSOC
    }
    else if(GP_ENCRYPTION_KEYID_IS_PRODUCT(keyId))
    {
        keyPtr = GP_HAL_SEC_PRODUCT_KEY_ID_TO_LOCATION(keyId);
#ifndef GP_HAL_DIVERSITY_SEC_CRYPTOSOC
        compressedKeyPtr = GP_MM_FLASH_ALT_ADDR_TO_COMPRESSED(keyPtr);
#endif //GP_HAL_DIVERSITY_SEC_CRYPTOSOC
    }
    else
    {
        GP_ASSERT_DEV_EXT(GP_ENCRYPTION_KEYID_IS_KEYPTR(keyId));

        //resolve potential NULL key pointer
        if(NULL == pAESKey)
        {
            MEMSET(gpHal_SecRamBuffer.aes.key, 0, AESOptions.keylen);
        }
        else
        {
            MEMCPY(gpHal_SecRamBuffer.aes.key, pAESKey, AESOptions.keylen);
        }
        pPatchedAesKey = gpHal_SecRamBuffer.aes.key;

        keyPtr = (UInt32)pPatchedAesKey;
#ifndef GP_HAL_DIVERSITY_SEC_CRYPTOSOC
        compressedKeyPtr = GP_MM_RAM_ADDR_TO_COMPRESSED(keyPtr);
#endif //GP_HAL_DIVERSITY_SEC_CRYPTOSOC
    }

#ifdef GP_HAL_DIVERSITY_SEC_CRYPTOSOC
    result = gpHalSec_CryptoSocAesEncrypt(dataPtr, keyPtr, AESOptions.keylen, hardened);
#else //GP_HAL_DIVERSITY_SEC_CRYPTOSOC
    result = gpHalSec_SspAesEncrypt(dataPtr, compressedKeyPtr, AESOptions.keylen, hardened);
#endif //GP_HAL_DIVERSITY_SEC_CRYPTOSOC

    if (result != gpHal_ResultSuccess)
    {
        return result;
    }

    // Copy data back to the in place buffer
    MEMCPY(pInplaceBuffer, (UInt8*)dataPtr, GP_HAL_DATA_LEN);

    return gpHal_ResultSuccess;   //successfull request
}

gpHal_Result_t gpHal_CCMEncrypt(gpEncryption_CCMOptions_t* pCCMOptions)
{
    //Call CCM base function
    return gpHalSec_CCMbase(pCCMOptions, false);
}
gpHal_Result_t gpHal_CCMEncrypt_RAM(UInt16 dataLength, UInt16 auxLength, UInt8 micLength, UInt8* dataPtr, UInt8* auxPtr, UInt8* micPtr, UInt8* pKey, UInt8* pNonce, UInt8* dataOutPtr)
{
    gpHal_Result_t result = gpHal_ResultInvalidRequest;

#ifdef GP_HAL_DIVERSITY_SEC_CRYPTOSOC
    sx_enable_clock();

    result = sx_aes_blk(
        CCM /*6*/,
        ENC /*1*/ /*encrypting*/,
        CTX_WHOLE, /*all in 1 packet*/
        block_t_convert((void*)pKey, GP_HAL_KEY_128),
        block_t_convert(NULL, 0),                           // xtskey
        block_t_convert(NULL, 0),                           // iv
        block_t_convert((void*)dataPtr, dataLength),        // data_in
        block_t_convert((void*)dataOutPtr, dataLength),     // data_out
        block_t_convert((void*)auxPtr, auxLength),          // aad
        block_t_convert((void*)(dataOutPtr + dataLength), micLength), // tag
        block_t_convert(NULL, 0),                           // ctx_ptr
        block_t_convert((void*)pNonce, GP_HAL_NONCE_104)    // nonce_len_blk
        ) == CRYPTOLIB_SUCCESS ? gpHal_ResultSuccess : gpHal_ResultInvalidRequest;

    sx_disable_clock();
#else
    gpEncryption_CCMOptions_t CCMOptions;
    gpPd_Handle_t pdHandle;

    if (dataLength + micLength + auxLength > GP_PD_BUFFER_SIZE_ZIGBEE)
    {
        return result;
    }

    pdHandle = gpPd_GetPd();
    if(gpPd_ResultValidHandle != gpPd_CheckPdValid(pdHandle))
    {
        return gpHal_ResultInvalidRequest;
    }

    CCMOptions.dataLength = dataLength;
    CCMOptions.auxLength = auxLength;
    CCMOptions.micLength = micLength;
    CCMOptions.pKey = pKey;
    CCMOptions.pNonce = pNonce;
    CCMOptions.pdHandle = pdHandle;
    CCMOptions.dataOffset = 0;
    CCMOptions.auxOffset = dataLength+micLength;
    gpPd_WriteByteStream(pdHandle, 0, dataLength + micLength, dataPtr);
    gpPd_WriteByteStream(pdHandle, dataLength+micLength, auxLength, auxPtr);

    //Call CCM base function
    result = gpHalSec_CCMbase(&CCMOptions,GP_WB_ENUM_SSP_MODE_ENCRYPT);
    gpPd_ReadByteStream(pdHandle, 0, dataLength + micLength, dataOutPtr);

    gpPd_FreePd(pdHandle);

#endif //GP_HAL_DIVERSITY_SEC_CRYPTOSOC
    return result;
}


gpHal_Result_t gpHal_CCMDecrypt_RAM(UInt16 dataLength, UInt16 auxLength, UInt8 micLength, UInt8* dataPtr, UInt8* auxPtr, UInt8* micPtr, UInt8* pKey, UInt8* pNonce, UInt8* dataOutPtr)
{
    gpHal_Result_t result = gpHal_ResultInvalidRequest;

#ifdef GP_HAL_DIVERSITY_SEC_CRYPTOSOC
    sx_enable_clock();
    result = sx_aes_blk(
        CCM /*6*/,
        DEC /*decrypting*/,
        CTX_WHOLE, /*all in 1 packet*/
        block_t_convert((void*)pKey, GP_HAL_KEY_128),
        block_t_convert(NULL, 0),                                   // xtskey
        block_t_convert(NULL, 0),                                   // iv
        block_t_convert((void*)dataPtr, dataLength),                // data_in
        block_t_convert((void*)dataOutPtr, dataLength),             // data_out
        block_t_convert((void*)auxPtr, auxLength),                  // aad
        block_t_convert((void*)micPtr, micLength),                  // tag
        block_t_convert(NULL, 0),                                   // ctx_ptr
        block_t_convert((void*)pNonce, GP_HAL_NONCE_104)            // nonce_len_blk
        ) == CRYPTOLIB_SUCCESS ? gpHal_ResultSuccess : gpHal_ResultInvalidRequest;

    sx_disable_clock();
#else
    gpEncryption_CCMOptions_t CCMOptions;
    gpPd_Handle_t pdHandle;

    if (dataLength + micLength + auxLength > GP_PD_BUFFER_SIZE_ZIGBEE)
    {
        return result;
    }
    pdHandle = gpPd_GetPd();
    if(gpPd_ResultValidHandle != gpPd_CheckPdValid(pdHandle))
    {
        return gpHal_ResultInvalidRequest;
    }

    CCMOptions.pdHandle = pdHandle;

    CCMOptions.dataLength = dataLength;
    CCMOptions.auxLength = auxLength;
    CCMOptions.micLength = micLength;
    CCMOptions.pKey = pKey;
    CCMOptions.pNonce = pNonce;
    CCMOptions.dataOffset = 0;
    CCMOptions.auxOffset = dataLength+micLength;

    if (dataLength)
    {
        gpPd_WriteByteStream(pdHandle, 0, dataLength, dataPtr);
    }
    gpPd_WriteByteStream(pdHandle, dataLength, micLength, micPtr);
    gpPd_WriteByteStream(pdHandle, dataLength+micLength, auxLength, auxPtr);

    //Call CCM base function
    result = gpHalSec_CCMbase(&CCMOptions,GP_WB_ENUM_SSP_MODE_DECRYPT);
    if (dataLength > 0)
    {
        gpPd_ReadByteStream(pdHandle, 0, dataLength, dataOutPtr);
    }

    gpPd_FreePd(pdHandle);
#endif //GP_HAL_DIVERSITY_SEC_CRYPTOSOC
    return result;
}


gpHal_Result_t gpHal_CCMDecrypt(gpEncryption_CCMOptions_t* pCCMOptions)
{
    //Call CCM base function
    return gpHalSec_CCMbase(pCCMOptions, true);
}

gpHal_Result_t gpHal_HMAC(UInt8 hashFct, UInt16 keyLength, UInt16 msgLength, UInt8 resultLength, UInt8* pKey, UInt8* pMsg, UInt8* pResult)
{
    gpHal_Result_t halResult = gpHal_ResultUnsupported;
    /* gpHal_HMAC not supported */
    /* Please use gpTls_HMACAuth or mbedtls_md_hmac API */


    return halResult;
}

