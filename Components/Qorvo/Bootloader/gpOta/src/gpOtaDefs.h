/*
 *   Copyright (c) 2019, Qorvo Inc
 *
 *   OTA Implementation
 *   Declarations of the public functions and enumerations of gpOta.
 *
 *   This software is owned by Qorvo Inc
 *   and protected under applicable copyright laws.
 *   It is delivered under the terms of the license
 *   and is intended and supplied for use solely and
 *   exclusively with products manufactured by
 *   Qorvo Inc.
 *
 *
 *   THIS SOFTWARE IS PROVIDED IN AN "AS IS"
 *   CONDITION. NO WARRANTIES, WHETHER EXPRESS,
 *   IMPLIED OR STATUTORY, INCLUDING, BUT NOT
 *   LIMITED TO, IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *   QORVO INC. SHALL NOT, IN ANY
 *   CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 *   INCIDENTAL OR CONSEQUENTIAL DAMAGES,
 *   FOR ANY REASON WHATSOEVER.
 *
 *   $Header$
 *   $Change$
 *   $DateTime$
 */


#ifndef _GPOTADEFS_H_
#define _GPOTADEFS_H_

/// @file "gpOtaDefs.h"
/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include "global.h"

#if defined (GP_OTA_DIVERSITY_CLIENT) && !defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
#include "mbedtls/sha256.h"
#include "mbedtls/ecdsa.h"
#endif

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
/** @macro GP_OTA_MAXRESPONSEWAITTIME - time to wait for a response command in ms*/
#define GP_OTA_MAXRESPONSEWAITTIME                   10000
/** @macro GP_OTA_MAXIMAGEFRAGMENTLENGTH - Max length of data chunk*/
#ifndef GP_OTA_MAXIMAGEFRAGMENTLENGTH
#define GP_OTA_MAXIMAGEFRAGMENTLENGTH                128
#endif
/** @macro GP_OTA_MINIMUMBLOCKPERIOD_DEFAULT - time between ImageBlockRequest commands in ms*/
#ifndef GP_OTA_MINIMUMBLOCKPERIOD_DEFAULT
#define GP_OTA_MINIMUMBLOCKPERIOD_DEFAULT            10
#endif

/** @macro GP_OTA_UPGRADE_TIME_OFFSET_IN_SEC - Delay running newly downloaded firmware (server only) */
#ifndef GP_OTA_UPGRADE_TIME_OFFSET_IN_SEC
#define GP_OTA_UPGRADE_TIME_OFFSET_IN_SEC            10
#endif

/** @macro OTA_REMOTE_RESPONSE_TIMEOUT - Response timeout in seconds */
#ifndef GP_OTA_REMOTE_RESPONSE_TIMEOUT_IN_SEC
#define GP_OTA_REMOTE_RESPONSE_TIMEOUT_IN_SEC        30
#endif

/** Header defines **/
#define OTA_HDR_FILE_ID                             0x0BEEF11E
#define OTA_HDR_VERSION                             0x0100
#define OTA_MIN_OTA_FILE_HEADER_LENGTH              56

#define OTA_PROFILE_ID                              0xC3
#define OTA_CLUSTER_ID                              0x0019

#define OTA_HEADER_FIELD_LENGTH                     4
#define OTA_IMAGE_NOTIYFY_LENGTH                    10
#define OTA_QUERY_NEXT_IMAGE_REQUEST_LENGTH         11
#define OTA_QUERY_NEXT_IMAGE_REQUEST_EXT_HW_LENGTH  2
#define OTA_QUERY_NEXT_IMAGE_RESPONSE_LENGTH        13
#define OTA_IMAGE_BLOCK_REQUEST_LENGTH              14
#define OTA_IMAGE_BLOCK_RESPONSE_LENGTH             14
#define OTA_UPGRADE_END_REQUEST_LENGTH              9
#define OTA_UPGRADE_END_RESPONSE_LENGTH             16

/* OTA Header - Field Control */
#define OTA_HEADER_FIELD_CONTROL_SECURITY_CRED_VERSION_PRESENT_BIT  0
#define OTA_HEADER_FIELD_CONTROL_DEVICE_SPEC_FILE_PRESENT_BIT       1  /* Device specific (e.g. unique security key) */
#define OTA_HEADER_FIELD_CONTROL_HARDWARE_VERSION_PRESENT_BIT       2

/* Query Next Image Request Command - Field control*/
#define OTA_NEXT_IMAGE_FIELD_CONTROL_HARDWARE_VERSION_PRESENT_BIT   0

/*Image Block Request Command - Field control */
#define OTA_BLOCK_REQUEST_FIELD_CONTROL_IEEE_ADDR_PRESENT_BIT       0
#define OTA_BLOCK_REQUEST_FIELD_CONTROL_DELAY_PRESENT_BIT           1

#define OTA_IMAGE_NOTIFY_CMD_ID                     0x00
#define OTA_QUERY_NEXT_IMAGE_REQUEST_CMD_ID         0x01
#define OTA_QUERY_NEXT_IMAGE_RESPONSE_CMD_ID        0x02
#define OTA_IMAGE_BLOCK_REQUEST_CMD_ID              0x03
#define OTA_IMAGE_BLOCK_RESPONSE_CMD_ID             0x05
#define OTA_UPGRADE_END_REQUEST_CMD_ID              0x06
#define OTA_UPGRADE_END_RESPONSE_CMD_ID             0x07

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Enums
 *****************************************************************************/
#if !defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
/** @enum Ota_Auth_Status_t */
//@{
/** @brief when the signature has been successfully validated */
#define Ota_Auth_Status_Success                                0x00
/** @brief when the validation started, but needs to be restarted to complete it */
#define Ota_Auth_Status_In_Progress                            0x01
/** @brief when it signature validation failed */
#define gpOta_Auth_Status_Failed                               0x02
typedef UInt8                             Ota_Auth_Status_t;
//@}
#endif // !defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

#if defined (GP_OTA_DIVERSITY_CLIENT) && !defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
typedef mbedtls_sha256_context ota_hash_context;
typedef struct {
    mbedtls_ecdsa_context       ecdsa_ctxt;     /**< Context required for the authentication*/
    mbedtls_ecdsa_restart_ctx*  restart_ctxt;   /**< Context required for splitting the authentication over multiple calls*/
} ota_verify_context_t;
#endif

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/
#if defined(GP_OTA_DIVERSITY_CLIENT)
void Ota_ImageNotifyIndication(UInt8 pairingRef, UInt8* pData);
void Ota_QueryNextImageResponseIndication(UInt8 pairingRef, UInt8* pData);
void Ota_ImageBlockResponseIndication(UInt8 pairingRef, UInt8* pData);
void Ota_UpgradeEndResponseIndication(UInt8 pairingRef, UInt8* pData);
#endif //defined(GP_OTA_DIVERSITY_CLIENT)


void Ota_PrepareHeaderFields(UInt8 cmdId, UInt8* pData);
Bool Ota_CheckHeaderFields(UInt8* cmdId, UInt8* pData);

#if defined (GP_OTA_DIVERSITY_CLIENT) && !defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
void Ota_AuthenticateImage_Init(ota_verify_context_t* ctx);
void Ota_AuthenticateImage_Free(ota_verify_context_t* ctx);
Ota_Auth_Status_t Ota_AuthenticateImage(ota_verify_context_t* ctx, UInt8 hash_buffer[32], UInt8 hash_buffer_len, UInt8* signature);
UInt32 Ota_GetExpectedSignatureLength(void);

void Ota_CalculateHash_Init(ota_hash_context* ctx);
Bool Ota_CalculateHash_Start(ota_hash_context* ctx);
Bool Ota_CalculateHash_Update(ota_hash_context* ctx, const UInt8* image_chunk, UInt32 chunk_len);
Bool Ota_CalculateHash_Finish(ota_hash_context* ctx, UInt8 hash_buffer[32], UInt8 hash_buffer_len);
void Ota_CalculateHash_Free(ota_hash_context* ctx);
#endif // GP_OTA_DIVERSITY_CLIENT && !GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY


#endif //_GPOTADEFS_H_

