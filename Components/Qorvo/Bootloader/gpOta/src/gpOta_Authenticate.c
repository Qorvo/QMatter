/*
 *   Copyright (c) 2019, Qorvo Inc
 *
 *   Shim layer to interface with mbedTls
 *   Verify a received signature and calculated sha256sum
 *   is provided by a trusted source (matching the gpOta_Authenticate_PublicKey)
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

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID GP_COMPONENT_ID_OTA
// #define GP_LOCAL_LOG

// Internal includes
#include "global.h"
#include "hal.h"
#include "gpAssert.h"
#include "gpLog.h"
#include "gpOtaDefs.h"
#include "gpPoolMem.h"

// mbedtls config includes
#include "config.h"
#include "gpOta_Authenticate_check_config.h" // Validate mbed_user_config.h
// mbedtls includes
#include "mbedtls/sha256.h"
#include "mbedtls/ecp.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/bignum.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/
/* Public EC key
 * Matches the private key (in PEM format) that comes with the SDK
 * Note: The SDK tools to sign images will print the public key in C-format to be copied here.
 */
const UInt8 gpOta_Authenticate_PublicKey[64] = {
    /*X point*/
    0x0c,0x24,0x2d,0xb4,0xaf,0x27,0xdd,0xd0,0xdf,0xe3,0x1e,0x3c,0x21,0x25,0x8b,0x97,0x06,0x17,0xf8,0xcc,0x11,0x15,0xad,
    0xf7,0x05,0xf3,0x82,0xff,0x8d,0xd9,0x2b,0x18,
    /*Y point*/
    0x39,0xae,0x7b,0x32,0x23,0x10,0xaa,0xa3,0xe1,0x9a,0x61,0x9b,0x86,0xd5,0xdb,0xf1,0xf0,0x81,0x5c,0x3f,0xba,0xcf,0x16,
    0x40,0x1d,0x7c,0x60,0x4c,0xf4,0xa8,0xde,0xdd
};

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/
#if !defined(MBEDTLS_ECDSA_VERIFY_ALT)
// Exposing the restartable variant of mbedtls_ecdsa_verify
extern int ecdsa_verify_restartable( mbedtls_ecp_group *grp,
                                     const unsigned char *buf, size_t blen,
                                     const mbedtls_ecp_point *Q,
                                     const mbedtls_mpi *r, const mbedtls_mpi *s,
                                     mbedtls_ecdsa_restart_ctx *rs_ctx );
#endif

static Int32 Ota_ecp_point_read_raw_binary(const mbedtls_ecp_group *grp, mbedtls_ecp_point *pt,
                                       const UInt8 *buf, UInt32 buf_len);
static Int32 Ota_ecdsa_read_raw_signature(ota_verify_context_t *ctx,
                                                      UInt8 *hash, UInt32 hlen,
                                                      UInt8 *sig, UInt32 slen);

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/
/**
 * @brief           This function imports a point from unsigned binary data in raw format (not DER).
 *
 * @note            Copied implementation from mbedtls_ecp_point_read_binary(), but stripped the DER format away
 *
 * @note            This function does not check that the point actually
 *                  belongs to the given group, see mbedtls_ecp_check_pubkey()
 *                  for that.
 *
 * @param grp       The group to which the point should belong.
 * @param P         The point to import.
 * @param buf       The input buffer.
 * @param ilen      The length of the input.
 *
 * @return          \c 0 on success.
 * @return          #MBEDTLS_ERR_ECP_BAD_INPUT_DATA if input is invalid.
 * @return          #MBEDTLS_ERR_MPI_ALLOC_FAILED on memory-allocation failure.
 */
static Int32 Ota_ecp_point_read_raw_binary(const mbedtls_ecp_group *grp, mbedtls_ecp_point *pt,
                                       const UInt8 *buf, UInt32 buf_len)
{
    Int32 ret;
    const UInt32 ec_point_length = mbedtls_mpi_size(&grp->P);
    GP_ASSERT_DEV_EXT(ec_point_length * 2 == buf_len);


    // Load public key (part X) into the context.Q (Public Key)
    if ((ret = mbedtls_mpi_read_binary(&pt->X, buf, ec_point_length)) != 0)
    {
        return ret;
    }

    // Load public key (part Y) into the context.Q (Public Key)
    if ((ret = mbedtls_mpi_read_binary(&pt->Y, buf + ec_point_length, ec_point_length)) != 0)
    {
        return ret;
    }

    // Set public key (part Z) in the context.Q (Public Key)
    if ((ret = mbedtls_mpi_lset(&pt->Z, 1)) != 0)
    {
        return ret;
    }

    return ret;
}

/**
 * @brief           This function reads and verifies an raw binary signature (R and S).
 *
 * @note            Copied from mbedtls_ecdsa_read_signature_restartable(), but stripped the ASN1 tags from the
 *                  signature
 *
 * @note            If the bitlength of the message hash is larger than the
 *                  bitlength of the group order, then the hash is truncated as
 *                  defined in <em>Standards for Efficient Cryptography Group
 *                  (SECG): SEC1 Elliptic Curve Cryptography</em>, section
 *                  4.1.4, step 3.
 *
 * @param ctx       The ota verify context.
 * @param hash      The message hash.
 * @param hlen      The size of the hash.
 * @param sig       The signature to read and verify.
 * @param slen      The size of \p sig.
 *
 * @return          \c 0 on success.
 * @return          An \c MBEDTLS_ERR_ECP_IN_PROGRESS when ecp_max_ops has been reached
 * @return          #MBEDTLS_ERR_ECP_BAD_INPUT_DATA if signature is invalid.
 * @return          #MBEDTLS_ERR_ECP_SIG_LEN_MISMATCH if there is a valid
 *                  signature in \p sig, but its length is less than \p siglen.
 * @return          An \c MBEDTLS_ERR_ECP_XXX or \c MBEDTLS_ERR_MPI_XXX
 *                  error code on failure for any other reason.
 */
static Int32 Ota_ecdsa_read_raw_signature(ota_verify_context_t *ctx,
                                          UInt8 *hash, UInt32 hlen,
                                          UInt8 *sig, UInt32 slen)
{
    Int32 ret;
    mbedtls_mpi r;
    mbedtls_mpi s;
    const UInt32 ec_point_length = mbedtls_mpi_size(&ctx->ecdsa_ctxt.grp.P);
    GP_ASSERT_DEV_EXT(ec_point_length * 2 == slen);

    // Split signature in <r, s> multi-precision integers (bignums)
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);
    if ((ret = mbedtls_mpi_read_binary(&r, sig, ec_point_length)) != 0)
    {
        GP_LOG_PRINTF("[OTA] Failed to retrieve the r-part of the signature - error: -0x%X", 0, 0-ret);
        goto cleanup;
    }
    if ((ret = mbedtls_mpi_read_binary(&s, sig + ec_point_length, ec_point_length)) != 0)
    {
        GP_LOG_PRINTF("[OTA] Failed to retrieve the s-part of the signature - error: -0x%X", 0, 0-ret);
        goto cleanup;
    }

    // Verify the signature and SHA256 are compatible with the public key
#if defined(MBEDTLS_ECDSA_VERIFY_ALT)
    if ((ret = mbedtls_ecdsa_verify(&ctx->ecdsa_ctxt.grp, hash, hlen, &ctx->ecdsa_ctxt.Q, &r, &s)) != 0)
    {
        GP_LOG_PRINTF("[OTA] Failed to verify the signature - error: -0x%X", 0, 0-ret);
        goto cleanup;
    }
#else

#if !defined(MBEDTLS_ECP_RESTARTABLE)
    // Long blocking function, disable watchdog
    HAL_WDT_DISABLE();
#endif
    ret = ecdsa_verify_restartable(&ctx->ecdsa_ctxt.grp, hash, hlen, &ctx->ecdsa_ctxt.Q, &r, &s, ctx->restart_ctxt);
#if !defined(MBEDTLS_ECP_RESTARTABLE)
    HAL_WDT_ENABLE(GP_WB_READ_WATCHDOG_TIMEOUT());
#endif
    if (ret != 0 &&
        ret != MBEDTLS_ERR_ECP_IN_PROGRESS)
    {
        GP_LOG_PRINTF("[OTA] Failed to verify the signature - error: -0x%X", 0, 0-ret);
        goto cleanup;
    }
#endif // MBEDTLS_ECDSA_VERIFY_ALT

cleanup:
    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&s);

    return ret;
}
/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
/**
 * @brief Return expected signature size
 * @note signature should hold 2 points (R and S)
 *       signature should hold raw values only, it should not contain an ASN1 header
 */
UInt32 Ota_GetExpectedSignatureLength(void)
{
    const mbedtls_ecp_curve_info* curve_info;
    UInt16 ec_point_length;

    // Retrieve the supported curves
    curve_info = mbedtls_ecp_curve_list();

    // Check if at least 1 curve is supported
    GP_ASSERT_DEV_EXT(curve_info->grp_id != MBEDTLS_ECP_DP_NONE);
    // Verify only 1 curve is supported (last item is always MBEDTLS_ECP_DP_NONE)
    GP_ASSERT_DEV_EXT((curve_info+1)->grp_id == MBEDTLS_ECP_DP_NONE);

    // calculate the expected length of 1 point on the curve
    ec_point_length = curve_info[0].bit_size / 8;

    // signature contains 2 points (r and s) on the curve
    return (UInt32)ec_point_length * 2;
}

/**
 * @brief Initialize the contexts required for signature authentication
 */
void Ota_AuthenticateImage_Init(ota_verify_context_t* ctx)
{
    // Init ECDSA context
    mbedtls_ecdsa_init(&ctx->ecdsa_ctxt);
#if defined(MBEDTLS_ECP_RESTARTABLE)
    ctx->restart_ctxt = GP_POOLMEM_MALLOC(sizeof(mbedtls_ecdsa_restart_ctx));
    if (ctx->restart_ctxt == NULL)
    {
        GP_LOG_PRINTF("[OTA] Failed to allocate \"restart context\", running blocking verification instead", 0);
        GP_ASSERT_DEV_EXT(false);
    }
    else
    {
        mbedtls_ecdsa_restart_init(ctx->restart_ctxt);
        mbedtls_ecp_set_max_ops(10);    //   1 operations = 528 restarts, each  +-50ms @32MHz (smaller than minimum)
                                        //  10 operations = 528 restarts, each  +-50ms @32MHz
                                        // 100 operations =  68 restarts, each +-360ms @32MHz
    }
#else
    ctx->restart_ctxt = NULL;
#endif
}

/**
 * @brief Frees the authentication context.
 */
void Ota_AuthenticateImage_Free(ota_verify_context_t* ctx)
{
    if (ctx)
    {
#if defined(MBEDTLS_ECP_RESTARTABLE)
        mbedtls_ecdsa_restart_free(ctx->restart_ctxt);
        gpPoolMem_Free(ctx->restart_ctxt);
#endif
        ctx->restart_ctxt = NULL;
        mbedtls_ecdsa_free(&ctx->ecdsa_ctxt);
    }
}


/**
 * @brief Verify the provided hash matches the @p signature
 * @note  Should be called again when it return
 *
 * @param[in] ctx                       Context used to perform the authentication, caller should keep it available
 *                                      until Ota_cbAuthenticateImageDone() is called
 * @param[in] hash_buffer               Pointer to the hash
 * @param[in] hash_buffer_len           Size of the hash array
 * @param[in] signature                 Signature to compare against
 * @return Ota_Auth_Status_t
 */
Ota_Auth_Status_t Ota_AuthenticateImage(ota_verify_context_t* ctx,
                                    UInt8 hash_buffer[32],
                                    UInt8 hash_buffer_len,
                                    UInt8* signature)
{
    Int32 ret;
    const mbedtls_ecp_curve_info* curve_info;
#if defined(GP_LOCAL_LOG)
    static UInt32 counter = 0;
#endif

    if (ctx == NULL)
    {
        GP_ASSERT_DEV_EXT(false);
        return gpOta_Auth_Status_Failed;
    }

    // Retrieve the supported curves
    curve_info = mbedtls_ecp_curve_list();
    // Check if at least 1 curve is supported (configure via mbedtls_user_config.h)
    GP_ASSERT_DEV_EXT(curve_info->grp_id != MBEDTLS_ECP_DP_NONE);
    // Verify only 1 curve is supported (last item is always MBEDTLS_ECP_DP_NONE)
    GP_ASSERT_DEV_EXT((curve_info+1)->grp_id == MBEDTLS_ECP_DP_NONE);

    // calculate the expected length of 1 point on the curve
    const UInt16 ec_point_length = curve_info[0].bit_size / 8;

    /* set up our own sub-context if needed (that is, on first run) */
    if(ctx->ecdsa_ctxt.grp.pbits == 0)
    {
        // Configure group: Set defaults for the supported EC curve type
        if ((ret = mbedtls_ecp_group_load(&ctx->ecdsa_ctxt.grp, curve_info->grp_id)) != 0)
        {
            GP_LOG_PRINTF("[OTA] Failed to configure the group - error: -0x%X", 0, 0-ret);
            return gpOta_Auth_Status_Failed;
        }

        // Load public key
        if ((ret = Ota_ecp_point_read_raw_binary(&ctx->ecdsa_ctxt.grp,
                                                 &ctx->ecdsa_ctxt.Q,
                                                 gpOta_Authenticate_PublicKey,
                                                 sizeof(gpOta_Authenticate_PublicKey))) != 0)
        {
            GP_LOG_PRINTF("[OTA] Failed to load public key - error: -%X", 0, 0-ret);
            return gpOta_Auth_Status_Failed;
        }

        // Check public key is valid: Should be located on the EC curve
        if ((ret = mbedtls_ecp_check_pubkey(&ctx->ecdsa_ctxt.grp, &ctx->ecdsa_ctxt.Q)) != 0)
        {
            GP_LOG_PRINTF("[OTA] Invalid public key point - error: -0x%X", 0, 0-ret);
            return gpOta_Auth_Status_Failed;
        }
    }

    /* (Continue) verifying the signature */
    GP_LOG_PRINTF("[OTA] auth %ld", 0, counter++);
    ret = Ota_ecdsa_read_raw_signature(ctx, hash_buffer, (UInt32)hash_buffer_len,
                                      signature, ec_point_length * 2);
    if (ret == 0)
    {
        return Ota_Auth_Status_Success;
    }
    else if (ret == MBEDTLS_ERR_ECP_IN_PROGRESS)
    {
        return Ota_Auth_Status_In_Progress;
    }
    else
    {
        GP_LOG_PRINTF("[OTA] Failed to verify signature - error: -0x%X", 0, 0-ret);
        return gpOta_Auth_Status_Failed;
    }
}

/**
 * @brief Initialize the contexts required for hashing a data buffer
 */
void Ota_CalculateHash_Init(ota_hash_context* ctx)
{
    mbedtls_sha256_init((mbedtls_sha256_context*)ctx);
}

/**
 * @brief Start a new hash
 */
Bool Ota_CalculateHash_Start(ota_hash_context* ctx)
{
    return (mbedtls_sha256_starts_ret((mbedtls_sha256_context*)ctx, 0) == 0);
}

/**
 * @brief Append more data to be included in the hash
 */
Bool Ota_CalculateHash_Update(ota_hash_context* ctx, const UInt8* image_chunk, UInt32 chunk_len)
{
    return (mbedtls_sha256_update_ret((mbedtls_sha256_context*)ctx, image_chunk, chunk_len) == 0);
}

/**
 * @brief Finalize and retrieve the hash for the given hash context
 */
Bool Ota_CalculateHash_Finish(ota_hash_context* ctx, UInt8 hash_buffer[32], UInt8 hash_buffer_len)
{
    Int32 ret;

    GP_ASSERT_DEV_EXT(hash_buffer_len >= 32);

    ret = mbedtls_sha256_finish_ret((mbedtls_sha256_context*)ctx, hash_buffer);

    GP_LOG_PRINTF("sha256sum calculated from OTA file:", 0);
#if defined(GP_LOCAL_LOG)
    gpLog_PrintBuffer(32, hash_buffer);
#endif

    return (ret == 0);
}

/**
 * @brief Free the contexts required for hashing a data buffer
 */
void Ota_CalculateHash_Free(ota_hash_context* ctx)
{
    mbedtls_sha256_free((mbedtls_sha256_context*)ctx);
}
