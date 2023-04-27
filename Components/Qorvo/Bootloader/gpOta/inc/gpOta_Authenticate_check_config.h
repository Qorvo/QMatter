/*
 * Copyright (c) 2021, Qorvo Inc
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

/** @file "gpOta_Authenticate_check_config.h"
 *
 *  OTA Implementation
 *
 *  Declarations of the public functions and enumerations of gpOta.
*/

#ifndef _GPOTA_AUTHENTICATE_CHECK_CONFIG_H_
#define _GPOTA_AUTHENTICATE_CHECK_CONFIG_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

#if !defined(GP_TLS_DIVERSITY_USER_DEFINED_MBEDTLS_CONFIG)
#error "gpOta requires a custom mbed_user_config.h, to use it, set GP_TLS_DIVERSITY_USER_DEFINED_MBEDTLS_CONFIG"
#endif

#if !defined(MBEDTLS_ASN1_PARSE_C)
#error "gpOta requires MBEDTLS_ASN1_PARSE_C to be defined in mbed_user_config.h"
#endif

#if !defined(MBEDTLS_ASN1_WRITE_C)
#error "gpOta requires MBEDTLS_ASN1_WRITE_C to be defined in mbed_user_config.h"
#endif

#if !defined(MBEDTLS_BIGNUM_C)
#error "gpOta requires MBEDTLS_BIGNUM_C to be defined in mbed_user_config.h"
#endif

#if !defined(MBEDTLS_ECP_C)
#error "gpOta requires MBEDTLS_ECP_C to be defined in mbed_user_config.h"
#endif

#if !defined(MBEDTLS_ECP_DP_SECP256R1_ENABLED)
#error "gpOta requires MBEDTLS_ECP_DP_SECP256R1_ENABLED to be defined in mbed_user_config.h"
#endif

#if !defined(MBEDTLS_PLATFORM_C)
#error "gpOta requires MBEDTLS_PLATFORM_C to be defined in mbed_user_config.h"
#endif

#if !defined(MBEDTLS_PLATFORM_MEMORY)
#error "gpOta requires MBEDTLS_PLATFORM_MEMORY to be defined in mbed_user_config.h"
#endif

#if !defined(MBEDTLS_SHA256_C)
#error "gpOta requires MBEDTLS_SHA256_C to be defined in mbed_user_config.h"
#endif

#if !defined(MBEDTLS_ECDSA_C)
#error "gpOta requires MBEDTLS_ECDSA_C to be defined in mbed_user_config.h"
#endif

#if !defined(MBEDTLS_PLATFORM_NO_STD_FUNCTIONS)
#error "gpOta requires MBEDTLS_PLATFORM_NO_STD_FUNCTIONS to be defined in mbed_user_config.h"
#endif

#if !defined(MBEDTLS_PLATFORM_SNPRINTF_MACRO)
#error "gpOta requires MBEDTLS_PLATFORM_SNPRINTF_MACRO to be defined in mbed_user_config.h"
#endif

#ifdef GP_TLS_DIVERSITY_USE_MBEDTLS_ALT
#if !defined(MBEDTLS_AES_ALT)
#error "gpOta requires MBEDTLS_AES_ALT to be defined in mbed_user_config.h"
#endif

#if !defined(MBEDTLS_ECP_ALT)
#error "gpOta requires MBEDTLS_ECP_ALT to be defined in mbed_user_config.h"
#endif

#if !defined(MBEDTLS_SHA256_ALT)
#error "gpOta requires MBEDTLS_SHA256_ALT to be defined in mbed_user_config.h"
#endif

#if !defined(MBEDTLS_ECDSA_VERIFY_ALT)
#error "gpOta requires MBEDTLS_ECDSA_VERIFY_ALT to be defined in mbed_user_config.h"
#endif

#if !defined(MBEDTLS_ENTROPY_HARDWARE_ALT)
#error "gpOta requires MBEDTLS_ENTROPY_HARDWARE_ALT to be defined in mbed_user_config.h"
#endif
#endif /* GP_TLS_DIVERSITY_USE_MBEDTLS_ALT */

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/


#endif //_GPOTA_AUTHENTICATE_CHECK_CONFIG_H_

