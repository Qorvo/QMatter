/*
 * Copyright (c) 2021-2022, Qorvo Inc
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
 */

/** @file "qvCHIP_KVS.c"
 *
 *  CHIP KVS functionality
 *
 *  Implementation of qvCHIP KVS
 *
 *  The implementation will use 2 type of 'elements' stored in NVM.
 *  - The key coming from the KVS API will be stored as a key element.
 *  - The actual payload, possibly split up in chunks with max MAX_KVS_VALUE_LEN size, are separate elements.
 *    The list of payload indices is added to the key element as a reference.
 *    Tkey key element will also store the first (MAX_KVS_VALUE_LEN - MAX_KVS_KEY_LEN - MAX_KVS_PAYLOAD_EXTENSIONS)
 *    bytes of the payload. If payload length is larger than this, only then a data element will be created.
 *
 *  The token mask of the NVM is used in such a way to avoid use of many different keys inside the NVM.
 *  All KVS used elements will have following tokens:
 *      [ Component ID | type=key | index ]
 *
 *  A Key-Value stored will end up with:
 *  - head     = [ Component ID | type=head | index ] [ key Hash | payload index 0 | ... | payload index 9 ] [ payload[0..229]]
 *  - payload0 = [ Component ID | type=data | index ] [ payload byte 230 | 231 | 232 | ...]
 *  - payload1 = [ Component ID | type=data | index ] [ payload byte MAX_ | MAX_ + 1 | ...]
 *
 *  This implementation can store data sizes MAX_KVS_VALUE_LEN*MAX_KVS_PAYLOAD_EXTENSIONS.
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
/* <CodeGenerator Placeholder> Includes */

//#define DEBUG
//#define GP_LOCAL_LOG
#define GP_COMPONENT_ID GP_COMPONENT_ID_QVCHIP

#include "qvCHIP.h"

#include "hal.h"
#include <stdlib.h> // malloc()

#include "gpLog.h"
#include "gpNvm.h"
#include "gpNvm_NvmProtect.h"


#ifdef QVCHIP_DIVERSITY_KVS_HASH_KEYS
#include "mbedtls/sha256.h"
#endif // QVCHIP_DIVERSITY_KVS_HASH_KEYS
/* </CodeGenerator Placeholder> Includes */

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> Macro */

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/* Pool ID for KVS is the same as pool id for NVN implementation - they share the same pool */
#define KVS_POOL_ID (0)

/* Maximum length for values stored in KVS - reserve last byte for keys larger than maximum value size */
#define MAX_KVS_KEY_LEN (GP_NVM_MAX_TOKENLENGTH - 2)

/* Maximum length for the token mask when looking for a specific token */
#define MAX_KVS_TOKENMASK_LEN (GP_NVM_MAX_TOKENLENGTH - 1)

/* Maximum ID extensions to support - 251*10 */
#define MAX_KVS_PAYLOAD_EXTENSIONS 10

/* Denotes an invalid payload index, to avoid using a length for the number of payload indexes in the key element */
#define INVALID_KEY_INDEX (0xFF)

/* Maximum length for values stored in KVS */
#ifdef GP_DIVERSITY_ROMUSAGE_FOR_MATTER
#define MAX_KVS_VALUE_LEN (251)
#else //GP_DIVERSITY_ROMUSAGE_FOR_MATTER
#define MAX_KVS_VALUE_LEN (255)
#endif //GP_DIVERSITY_ROMUSAGE_FOR_MATTER

/* Maximum payload length that can be stored in a head element */
#define MAX_PAYLOAD_IN_HEAD_ELEM (MAX_KVS_VALUE_LEN - MAX_KVS_KEY_LEN - MAX_KVS_PAYLOAD_EXTENSIONS)

/* Variable settings definitions */
#ifndef GP_NVM_NBR_OF_UNIQUE_TOKENS
#define GP_NVM_NBR_OF_UNIQUE_TOKENS GP_NVM_NBR_OF_UNIQUE_TAGS
#endif //GP_NVM_NBR_OF_UNIQUE_TOKENS

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/** @brief Key information element type - will be populated with qvCHIP_KvsKeyPayload_t */
#define qvCHIP_KvsTypeHead 0x1
/** @brief Data element type */
#define qvCHIP_KvsTypeData 0x2
/** @typedef qvCHIP_KvsType_t
 *  @brief The qvCHIP_KvsType_t defines the types of NVM records used for KVS.
*/
typedef UInt8 qvCHIP_KvsType_t;

/** @brief Token payload for all KVS elements added in NVM.
    Struct used as representation of the byte stream to be written as NVM record
*/
typedef PACKED_PRE struct qvCHIP_KvsToken_s {
    /** @brief qvCHIP component ID - fixed */
    uint8_t componentId;
    /** @brief Type of KVS data */
    qvCHIP_KvsType_t type;
    uint8_t index;
} PACKED_POST qvCHIP_KvsToken_t;

/** @brief NVM record payload used for a KVS key element
    Strucct used as representation of the byte stream to be written as NVM record.
*/
typedef PACKED_PRE struct qvCHIP_KvsKeyPayload {
    /** @brief Key hash for the char key given at API level */
    uint8_t keyHash[MAX_KVS_KEY_LEN];
    /** @brief List of indices to payload elements.
    *   A payload can be split-up in multiple elements to fit a single NVM element.
    *   Each index is put in this list in sequence in which the pieces were broken down in.
    */
    uint8_t payloadIndices[MAX_KVS_PAYLOAD_EXTENSIONS];
    /** @brief First MAX_PAYLOAD_IN_HEAD_ELEM bytes of the key will be stored in the head element.
    */
    uint8_t payload[MAX_PAYLOAD_IN_HEAD_ELEM];
} PACKED_POST qvCHIP_KvsKeyPayload_t;

/* </CodeGenerator Placeholder> Macro */

/*****************************************************************************
 *                    Static Data
 *****************************************************************************/

/** @brief Mutex for all public API calls */
HAL_CRITICAL_SECTION_DEF(qvCHIP_KvsMutex)

/** @brief Start of NVM area - linkerscript defined. */
extern const UIntPtr gpNvm_Start;

/** @brief Persistent LUT handle for KVS storage, used for all NVM operations */
gpNvm_LookupTable_Handle_t qvCHIP_KvsLookupHandle = gpNvm_LookupTable_Handle_Invalid;


/*****************************************************************************
 *                    Static Component Function Definitions
 *****************************************************************************/
#ifdef QVCHIP_DIVERSITY_KVS_HASH_KEYS

/** @brief Convert a char string to a hash to limit amount of bytes required
 *
 *  @param[in]  length Length of key.
 *  @param[in]  key    Array holding the key.
 *  @param[out] hash   Array to return calculated hash value from key.
*/
static qvStatus_t qvCHIP_KvsHashKey(uint8_t length, uint8_t* key, uint8_t* hash)
{
    int ret;
    mbedtls_sha256_context qvCHIP_Kvs_HashContext;
    mbedtls_sha256_init(&qvCHIP_Kvs_HashContext);
    ret = mbedtls_sha256_starts(&qvCHIP_Kvs_HashContext, 0);
    if(ret)
    {
        goto exit;
    }
    ret = mbedtls_sha256_update(&qvCHIP_Kvs_HashContext, key, length);
    if(ret)
    {
        goto exit;
    }
    ret = mbedtls_sha256_finish(&qvCHIP_Kvs_HashContext, hash);
    if(ret)
    {
        goto exit;
    }
exit:
    mbedtls_sha256_free(&qvCHIP_Kvs_HashContext);
    return (ret) ? QV_STATUS_INVALID_DATA : QV_STATUS_NO_ERROR;
}
#endif // QVCHIP_DIVERSITY_KVS_HASH_KEYS

/** @brief Create a byte based hash from a key as string
 *
 *  @param[in]  key         String key
 *  @param[out] hashBuffer  Byte buffer to return hash created from the kes string
*/
static qvStatus_t qvCHIP_KvsCreateHash(const char* key, uint8_t* hashBuffer)
{
    qvStatus_t qvStatus = QV_STATUS_NO_ERROR;
    uint8_t keyLen = strlen(key);
#ifdef QVCHIP_DIVERSITY_KVS_HASH_KEYS
    uint8_t hash[32];
    qvStatus = qvCHIP_KvsHashKey(keyLen, (uint8_t*)key, hash);
    if(QV_STATUS_NO_ERROR != qvStatus)
    {
        return qvStatus;
    }
    MEMCPY(hashBuffer, hash, MAX_KVS_KEY_LEN);
#else
    if(keyLen > MAX_KVS_KEY_LEN)
    {
        GP_LOG_PRINTF("WARNING: Key is too long");
        return QV_STATUS_INVALID_DATA;
    }
    /* make sure we only use the string part of the key and not the rest that
       follows after the string terminator */
    MEMSET(hashBuffer, 0x00, MAX_KVS_KEY_LEN);
    MEMCPY(hashBuffer, key, MIN(keyLen, MAX_KVS_KEY_LEN));
#endif // QVCHIP_DIVERSITY_KVS_HASH_KEYS

    GP_LOG_PRINTF("'%s' -> %02x%02x%02x%02x...", 0, key,
                  hashBuffer[0], hashBuffer[1], hashBuffer[2], hashBuffer[3]);

    return qvStatus;
}

/************************
* Lookup handling
*************************/

/** @brief Allocate Lookup handle for KVS use of NVM space
 *
 *  @return status Possible values from qvStatus_t:
 *                - QV_STATUS_NVM_ERROR - any failure in gpNvm call to build LUT
 *                - QV_STATUS_NO_ERROR
 */
static qvStatus_t qvCHIPKvs_BuildLookup(void)
{
    // Allocate once
    if(qvCHIP_KvsLookupHandle == gpNvm_LookupTable_Handle_Invalid)
    {
        // Allocate LUT for KVS storage
        UInt8 lookupMask[1] = {GP_COMPONENT_ID};
        UInt8 nrOfMatches;
        gpNvm_Result_t nvmResult;

        nvmResult = gpNvm_BuildLookupProtected(&qvCHIP_KvsLookupHandle, KVS_POOL_ID, gpNvm_UpdateFrequencyIgnore,
                                               sizeof(lookupMask), lookupMask,
                                               GP_NVM_NBR_OF_UNIQUE_TOKENS, &nrOfMatches);
        GP_LOG_PRINTF("KVS: LUT:%u allocated - %u elements in use", 0, qvCHIP_KvsLookupHandle, nrOfMatches);

        if(nvmResult != gpNvm_Result_DataAvailable)
        {
            return QV_STATUS_NVM_ERROR;
        }
    }
    return QV_STATUS_NO_ERROR;
}

/************************
* Element handling
*************************/

/** @brief Retrieve a free index for a given KVS element type
 *
 *  @param type Type of KVS element to look for (key/data)
*/
static UInt8 qvCHIPKvs_FindFreeIndex(qvCHIP_KvsType_t type)
{
    gpNvm_Result_t nvmResult;

    qvCHIP_KvsToken_t tokenMask = {
        .componentId = GP_COMPONENT_ID,
        .type = type,
        .index = 0};

    nvmResult = gpNvm_ResetIterator(qvCHIP_KvsLookupHandle);
    if(nvmResult != gpNvm_Result_DataAvailable)
    {
        goto _cleanup;
    }

    do
    {
        UInt8 dataLen;

        // Scrolling through LUT for key elements with index in tokenmask
        nvmResult = gpNvm_ReadUniqueProtected(qvCHIP_KvsLookupHandle, KVS_POOL_ID, gpNvm_UpdateFrequencyIgnore, NULL,
                                              sizeof(tokenMask), (uint8_t*)&tokenMask,
                                              MAX_KVS_VALUE_LEN,
                                              &dataLen,
                                              NULL);
        if(nvmResult == gpNvm_Result_DataAvailable)
        {
            // Index already in use, look for a new one
            tokenMask.index++;
        }
        else if(nvmResult == gpNvm_Result_NoDataAvailable)
        {
            // Index not yet written - use as free index
            break;
        }
        else
        {
            GP_LOG_PRINTF("key lookup failed:%x", 0, nvmResult);
            tokenMask.index = 0xFF;
            goto _cleanup;
        }
    } while(nvmResult == gpNvm_Result_DataAvailable);
    GP_LOG_PRINTF("0: |%x|%x found free token", 0, type, tokenMask.index);


_cleanup:
    return tokenMask.index;
}

/************************
* Key Element handling
*************************/
#ifdef GP_LOCAL_LOG
/** @brief Dump the information of a KVS key element
 *
 *  @param keyElementIndex Index of element used in NVM area
 *  @param pKeyPayload     Pointer to struct reprentation of key NVM element byte payload.
*/
static void qvCHIP_KvsDumpKeyElement(UInt8 keyElementIndex, qvCHIP_KvsKeyPayload_t* pKeyPayload)
{
    GP_LOG_SYSTEM_PRINTF("%u: %02x%02x%02x%02x... | %u, %u, ...", 0,
                         keyElementIndex,
                         pKeyPayload->keyHash[0], pKeyPayload->keyHash[1], pKeyPayload->keyHash[2], pKeyPayload->keyHash[3],
                         pKeyPayload->payloadIndices[0],
                         pKeyPayload->payloadIndices[1] != INVALID_KEY_INDEX ? pKeyPayload->payloadIndices[1] : INVALID_KEY_INDEX);
    gpLog_Flush();
}
#else
#define qvCHIP_KvsDumpKeyElement(keyElementIndex, keyPayload)
#endif

/** @brief Add a KVS key information element to NVM.
 *
 * @param[in] keyElementIndex Index to use for key storage
 * @param[in] keyHash Array of hash bytes to use as key in the element
 * @param[in] valueSize Size of full data block to store (in bytes)
 * @param[in] value Pointer to byte array to store
 * @param[in] payloadIndices List of indices to use to store the payload parts.
 *                           This list is used as ordered to store parts sequentally.
 *
 * @return status Possible values from qvStatus_t:
 *                - QV_STATUS_NVM_ERROR - any failure in gpNvm calls
 *                - QV_STATUS_NO_ERROR
*/
static qvStatus_t qvCHIP_KvsAddHeadElement(UInt8 keyElementIndex, const UInt8* keyHash, size_t valueSize, const void* value,
                                           const UInt8* payloadIndices)
{
    gpNvm_Result_t nvmResult;

    // Fill in index for key element
    qvCHIP_KvsToken_t keyTokenMask = {
        .componentId = GP_COMPONENT_ID,
        .type = qvCHIP_KvsTypeHead,
        .index = keyElementIndex};
    qvCHIP_KvsKeyPayload_t keyPayload;
    size_t writeLen = sizeof(keyPayload.keyHash);

    // Fill in data = key hash | payload indices | payload
    if(keyHash == NULL)
    {
        MEMSET(&keyPayload.keyHash, 0x0, MAX_KVS_KEY_LEN);
    }
    MEMCPY(&keyPayload.keyHash, keyHash, MAX_KVS_KEY_LEN);
    MEMCPY(&keyPayload.payloadIndices, payloadIndices, MAX_KVS_PAYLOAD_EXTENSIONS);
    writeLen += MAX_KVS_PAYLOAD_EXTENSIONS;

    if(value != NULL)
    {
        MEMCPY(&keyPayload.payload, (UInt8*)value, MIN(valueSize, MAX_PAYLOAD_IN_HEAD_ELEM));
        writeLen += MIN(valueSize, MAX_PAYLOAD_IN_HEAD_ELEM);
    }

    GP_LOG_PRINTF("Adding key", 0);
    qvCHIP_KvsDumpKeyElement(keyElementIndex, &keyPayload);

    // Write to NVM
    nvmResult = gpNvm_WriteProtected(KVS_POOL_ID, gpNvm_UpdateFrequencyIgnore, sizeof(keyTokenMask), (uint8_t*)&keyTokenMask,
                                     writeLen, (uint8_t*)&keyPayload);
    if(nvmResult != gpNvm_Result_DataAvailable)
    {
        // Write failed - NVM full ?
        return QV_STATUS_NVM_ERROR;
    }

    return QV_STATUS_NO_ERROR;
}

/** @brief Retrieve a certain KVS key information record based on the key hash.
 *
 * @param[in] keyHash
 * @param[out] pKeyIndex Pointer in which to return the index of the KVS key element
 * @param[out] pPayloadIndices Pointer to list to return payload indices
 * @param[in] maxReadSize Maximum size that can be read (in bytes)
 * @param[out] valueSize Size of payload data read (in bytes)
 * @param[out] value Pointer to data array where to store the data
 *
 * @return status Possible values from qvStatus_t:
 *                - QV_STATUS_NVM_ERROR - any failure in gpNvm calls
 *                - QV_STATUS_NO_ERROR
*/
static qvStatus_t qvCHIPVS_FindKeyHeadInfo(const UInt8* keyHash, UInt8* pKeyIndex, UInt8* pPayloadIndices,
                                           size_t maxReadSize, size_t* valueSize, UInt8* value)
{
    qvStatus_t qvStatus = QV_STATUS_INVALID_DATA;
    gpNvm_Result_t nvmResult;

    if(keyHash == NULL || pKeyIndex == NULL || pPayloadIndices == NULL)
    {
        return QV_STATUS_INVALID_DATA;
    }

    // Initialize in case not found
    *pKeyIndex = INVALID_KEY_INDEX;

    GP_LOG_PRINTF("Finding -> %02x%02x%02x%02x... l:%u", 0,
                  keyHash[0], keyHash[1], keyHash[2], keyHash[3], qvCHIP_KvsLookupHandle);

    nvmResult = gpNvm_ResetIterator(qvCHIP_KvsLookupHandle);
    if(nvmResult != gpNvm_Result_DataAvailable)
    {
        return QV_STATUS_NVM_ERROR;
    }

    do
    {
        qvCHIP_KvsToken_t lookupMask;
        UInt8 lookupLen;                // Only used for check
        qvCHIP_KvsKeyPayload_t dataKey; // Required for full info fetch
        UInt8 dataLen;                  // Used to determine length of indices

        nvmResult = gpNvm_ReadNextProtected(qvCHIP_KvsLookupHandle, KVS_POOL_ID, NULL, GP_NVM_MAX_TOKENLENGTH, &lookupLen,
                                            (uint8_t*)&lookupMask, sizeof(dataKey), &dataLen, (uint8_t*)&dataKey);
        GP_LOG_PRINTF("r:%u mask:%u %u|%u|%u data:%u %02x%02x%02x%02x...", 0, nvmResult,
                      lookupLen, lookupMask.componentId, lookupMask.type, lookupMask.index,
                      dataLen, dataKey.keyHash[0], dataKey.keyHash[1], dataKey.keyHash[2], dataKey.keyHash[3]);
        if(nvmResult == gpNvm_Result_NoDataAvailable)
        {
            // Nothing more found in LUT
            goto _cleanup;
        }

        if(lookupMask.type != qvCHIP_KvsTypeHead)
        {
            // Only look for head type elements
            // Result could be gpNvm_Result_Truncated due to the data size given.
            continue;
        }

        if(nvmResult != gpNvm_Result_DataAvailable)
        {
            // Real failure reasons
            qvStatus = QV_STATUS_NVM_ERROR;
            goto _cleanup;
        }

        // Found NVM record exceeds expected fixed key length
        // Or token used is inconsistent with KVS used structure
        if((dataLen < MAX_KVS_KEY_LEN) || (lookupLen != sizeof(lookupMask)))
        {
            qvStatus = QV_STATUS_NVM_ERROR;
            goto _cleanup;
        }

        // Check for key we're looking for
        if(MEMCMP(&dataKey.keyHash[0], keyHash, MAX_KVS_KEY_LEN) == 0)
        {
            qvStatus = QV_STATUS_NO_ERROR;
            *pKeyIndex = lookupMask.index;

            // Return Payload index information
            MEMCPY(pPayloadIndices, &dataKey.payloadIndices, MAX_KVS_PAYLOAD_EXTENSIONS);

            if(valueSize != NULL && value != NULL)
            {
                // Copy payload data
                *valueSize = MIN(maxReadSize, dataLen - MAX_KVS_PAYLOAD_EXTENSIONS - MAX_KVS_KEY_LEN);
                MEMCPY(value, &dataKey.payload, *valueSize);
                // Check if the provided buffer was too small to retrieve the existing key data
                if(maxReadSize < (dataLen - MAX_KVS_PAYLOAD_EXTENSIONS - MAX_KVS_KEY_LEN))
                {
                    qvStatus = QV_STATUS_BUFFER_TOO_SMALL;
                }
            }

            GP_LOG_PRINTF(">>>FK: Key found", 0);
            qvCHIP_KvsDumpKeyElement(*pKeyIndex, &dataKey);

            break;
        }
    } while(nvmResult != gpNvm_Result_NoDataAvailable);

_cleanup:
    return qvStatus;
}

/************************
* Data element handling
*************************/

/** @brief Adding KVS data parts to NVM
 *
 *  @param[in] valueSize Size of full data block to store (in bytes)
 *  @param[in] value Pointer to byte array to store
 *  @param[in,out] pPayloadIndices List of indices to use to store payload blocks.
 *                                 If no previous indices were given (set to 0xFF), new indices will be allocated.
 *
 *  @return status Possible values from qvStatus_t:
 *                - QV_STATUS_NVM_ERROR - any failure in gpNvm calls
 *                - QV_STATUS_NO_ERROR
*/
static qvStatus_t qvCHIP_KvsAddDataElements(size_t valueSize, const void* value, uint8_t* pPayloadIndices)
{
    gpNvm_Result_t nvmResult;
    UInt8 iterator = 0;

    while((iterator < MAX_KVS_PAYLOAD_EXTENSIONS) && (valueSize > 0))
    {
        qvCHIP_KvsToken_t dataTokenMask = {
            .componentId = GP_COMPONENT_ID,
            .type = qvCHIP_KvsTypeData,
            .index = pPayloadIndices[iterator]};

        // If key was already know, we'll be re-using the payload indices
        dataTokenMask.index = pPayloadIndices[iterator];
        if(dataTokenMask.index == INVALID_KEY_INDEX)
        {
            // No index known before - new element
            // Find a free element for a piece of data payload
            dataTokenMask.index = qvCHIPKvs_FindFreeIndex(qvCHIP_KvsTypeData);
            if(dataTokenMask.index == INVALID_KEY_INDEX)
            {
                GP_LOG_PRINTF("NVM error.", 0);
                return QV_STATUS_NVM_ERROR;
            }
            else
            {
                // Save in indices reference for key
                pPayloadIndices[iterator] = dataTokenMask.index;
            }
        }

        // Write Data payload element
        nvmResult = gpNvm_WriteProtected(KVS_POOL_ID, gpNvm_UpdateFrequencyIgnore,
                                         sizeof(dataTokenMask), (uint8_t*)&dataTokenMask,
                                         MIN(valueSize, MAX_KVS_VALUE_LEN), &((uint8_t*)value)[iterator * MAX_KVS_VALUE_LEN]);
        if(nvmResult != gpNvm_Result_DataAvailable)
        {
            // Write failed - NVM full ?
            GP_LOG_PRINTF("NVM add data fail r:%x", 0, nvmResult);
            return QV_STATUS_NVM_ERROR;
        }
        valueSize -= MIN(valueSize, MAX_KVS_VALUE_LEN);
        iterator += 1;
    }

    // Reset remaining payload indexes
    while(iterator < MAX_KVS_PAYLOAD_EXTENSIONS)
    {
        pPayloadIndices[iterator++] = INVALID_KEY_INDEX;
    }

    return QV_STATUS_NO_ERROR;
}

/** @brief Retrieve all data stored in several data elements.
 *
 * @param[in] pPayloadIndices Array of indices to collect the data from
 * @param[in] offsetBytes Offset to start reading in first found element
 * @param[in] maxBytesToRead Limit on bytes to read - Partial read out of the available data possible
 * @param[out] readBytesSize Amount of bytes read from existing data.
 *                           Could be less then maxBytesToRead when data is smaller then given read buffer
 * @param[out] pReadBuffer Buffer for the data to read back.
 *
 *  @return status Possible values from qvStatus_t:
 *                - QV_STATUS_NVM_ERROR - any failure in gpNvm calls
 *                - QV_STATUS_INVALID_ARGUMENT
 *                - QV_STATUS_BUFFER_TOO_SMALL - buffer in which can be returned is too small
 *                - QV_STATUS_NO_ERROR
*/
static qvStatus_t qvCHIP_KvsGetDataElements(const UInt8* pPayloadIndices, UInt8 offsetBytes, size_t maxBytesToRead,
                                            size_t* readBytesSize, UInt8* pReadBuffer)
{
    qvStatus_t qvStatus = QV_STATUS_NO_ERROR;
    gpNvm_Result_t nvmResult;
    UInt8 iterator = 0;
    UInt8 dataLen;

    nvmResult = gpNvm_ResetIterator(qvCHIP_KvsLookupHandle);
    if(nvmResult != gpNvm_Result_DataAvailable)
    {
        return QV_STATUS_NVM_ERROR;
    }

    while((iterator < MAX_KVS_PAYLOAD_EXTENSIONS) && (maxBytesToRead > 0))
    {
        qvCHIP_KvsToken_t tokenMask = {
            .componentId = GP_COMPONENT_ID,
            .type = qvCHIP_KvsTypeData,
            .index = pPayloadIndices[iterator]};

        iterator += 1;
        if(tokenMask.index == INVALID_KEY_INDEX)
        {
            break;
        }
        if(offsetBytes > MAX_KVS_VALUE_LEN)
        {
            offsetBytes -= MAX_KVS_VALUE_LEN;
            continue;
        }

        nvmResult = gpNvm_ReadUniqueProtected(qvCHIP_KvsLookupHandle, KVS_POOL_ID, gpNvm_UpdateFrequencyIgnore, NULL,
                                              sizeof(tokenMask), (uint8_t*)&tokenMask,
                                              MIN(MAX_KVS_VALUE_LEN, maxBytesToRead), &dataLen,
                                              (uint8_t*)&pReadBuffer[*readBytesSize]);
        if(nvmResult != gpNvm_Result_DataAvailable)
        {
            qvStatus = QV_STATUS_INVALID_DATA;
            goto _cleanup;
        }

        // At this point, if offsetBytes is greater than 0, it is surely less than MAX_KVS_VALUE_LEN, otherwise the entire element
        // would have been skipped; therefore, we need to skip the remaining offsetBytes
        // Also, if offsetBytes > 0 here, it means at this point, *readBytesSize is 0
        if(offsetBytes > 0)
        {
            MEMCPY_INPLACE((uint8_t*)&pReadBuffer[*readBytesSize], &pReadBuffer[*readBytesSize + offsetBytes],
                           MIN(maxBytesToRead, dataLen - offsetBytes));
            offsetBytes = 0;
            *readBytesSize += MIN(maxBytesToRead, dataLen - offsetBytes);
            maxBytesToRead -= MIN(maxBytesToRead, dataLen - offsetBytes);
        }
        else
        {
            *readBytesSize += MIN(maxBytesToRead, dataLen);
            maxBytesToRead -= MIN(maxBytesToRead, dataLen);
        }
    }

_cleanup:
    return qvStatus;
}

/** @brief Delete a KVS element - not protected by a mutex
*
*   @param[in] keyHash List of hash bytes to identify full (key + payload) KVS record to delete
*/
static qvStatus_t qvCHIP_KvsDeleteInternal(const uint8_t* keyHash)
{
    qvStatus_t qvStatus;
    gpNvm_Result_t nvmResult;
    qvCHIP_KvsToken_t tokenMask;
    UInt8 iterator = 0;

    /* check parameters*/
    if(keyHash == NULL)
    {
        return QV_STATUS_INVALID_ARGUMENT;
    }

    UInt8 payloadIndices[MAX_KVS_PAYLOAD_EXTENSIONS];
    UInt8 keyIndex;

    // Retrieve list of payload elements from head info structure (we're not interested in the data stored)
    qvStatus = qvCHIPVS_FindKeyHeadInfo(keyHash, &keyIndex, payloadIndices, MAX_KVS_VALUE_LEN, NULL, NULL);
    if(QV_STATUS_NO_ERROR != qvStatus)
    {
        goto _cleanup;
    }

    // Mark delete in progress - set hash to 0x0
    qvCHIP_KvsAddHeadElement(keyIndex, NULL, 0, NULL, payloadIndices);

    // Delete data elements
    tokenMask.componentId = GP_COMPONENT_ID;
    tokenMask.type = qvCHIP_KvsTypeData;
    while(iterator < MAX_KVS_PAYLOAD_EXTENSIONS)
    {
        tokenMask.index = payloadIndices[iterator];
        iterator += 1;

        if(tokenMask.index == INVALID_KEY_INDEX)
        {
            break;
        }

        nvmResult = gpNvm_RemoveProtected(KVS_POOL_ID, gpNvm_UpdateFrequencyIgnore, sizeof(tokenMask), (uint8_t*)&tokenMask);
        if(nvmResult != gpNvm_Result_DataAvailable)
        {
            qvStatus = QV_STATUS_INVALID_DATA;
            goto _cleanup;
        }
    }

    // Remove head element
    tokenMask.type = qvCHIP_KvsTypeHead;
    tokenMask.index = keyIndex;

    nvmResult = gpNvm_RemoveProtected(KVS_POOL_ID, gpNvm_UpdateFrequencyIgnore, sizeof(tokenMask), (uint8_t*)&tokenMask);
    if(nvmResult != gpNvm_Result_DataAvailable)
    {
        qvStatus = QV_STATUS_INVALID_DATA;
        goto _cleanup;
    }

_cleanup:
    return qvStatus;
}

/*****************************************************************************
 *                    Public Component Function Definitions
 *****************************************************************************/
void qvCHIP_NvmSetVariableSettings(void)
{
#ifdef GP_NVM_DIVERSITY_VARIABLE_SETTINGS
    UInt8 availableMaxTokenLength;
    UIntPtr currentNvmStart;
    gpNvm_KeyIndex_t currentNumberOfUniqueTokens;

    gpNvm_GetVariableSettings(&currentNvmStart, &currentNumberOfUniqueTokens, &availableMaxTokenLength);
    GP_LOG_PRINTF("Settings old/new: Nvm Start:%lx/%lx #Uniq Token:%u/%u Token Length:%u/%u", 0,
                  (unsigned long)currentNvmStart, (unsigned long)&gpNvm_Start,
                  currentNumberOfUniqueTokens, GP_NVM_NBR_OF_UNIQUE_TOKENS,
                  availableMaxTokenLength, GP_NVM_MAX_TOKENLENGTH);

    if(availableMaxTokenLength < GP_NVM_MAX_TOKENLENGTH)
    {
        GP_LOG_SYSTEM_PRINTF("Max token length %u < %u", 0, availableMaxTokenLength, GP_NVM_MAX_TOKENLENGTH);
        GP_ASSERT_DEV_INT(false);
    }

    gpNvm_SetVariableSettings((UIntPtr)&gpNvm_Start, GP_NVM_NBR_OF_UNIQUE_TOKENS);
#endif //GP_NVM_DIVERSITY_VARIABLE_SETTINGS
#ifdef GP_NVM_DIVERSITY_VARIABLE_SIZE
    UInt16 currentNrOfSectors;
    UInt8 currentNrOfPools;
    UInt8 currentSectorsPerPool[4]; // Max from NVM implementation

    UInt16 newNrOfSectors = (GP_NVM_POOL_1_NBR_OF_PHY_SECTORS
#if GP_NVM_NBR_OF_POOLS > 1
                             + GP_NVM_POOL_2_NBR_OF_PHY_SECTORS
#endif
#if GP_NVM_NBR_OF_POOLS > 2
                             + GP_NVM_POOL_3_NBR_OF_PHY_SECTORS
#endif
#if GP_NVM_NBR_OF_POOLS > 3
                             + GP_NVM_POOL_4_NBR_OF_PHY_SECTORS
#endif
    );
    UInt8 newNrOfPools = GP_NVM_NBR_OF_POOLS;
    UInt8 newSectorsPerPool[GP_NVM_NBR_OF_POOLS] = {
        GP_NVM_POOL_1_NBR_OF_PHY_SECTORS,
#if GP_NVM_NBR_OF_POOLS > 1
        GP_NVM_POOL_2_NBR_OF_PHY_SECTORS,
#endif
#if GP_NVM_NBR_OF_POOLS > 2
        GP_NVM_POOL_3_NBR_OF_PHY_SECTORS,
#endif
#if GP_NVM_NBR_OF_POOLS > 3
        GP_NVM_POOL_4_NBR_OF_PHY_SECTORS,
#endif
    };

    gpNvm_GetVariableSize(&currentNrOfSectors, &currentNrOfPools, currentSectorsPerPool);
    GP_LOG_PRINTF("Sizes old/new: #sectors:%u/%u #pools:%u/%u", 0, currentNrOfSectors, newNrOfSectors, currentNrOfPools,
                  newNrOfPools);

    gpNvm_SetVariableSize(newNrOfSectors, newNrOfPools, newSectorsPerPool);
#endif //GP_NVM_DIVERSITY_VARIABLE_SIZE
}

qvStatus_t qvCHIP_KvsInit(void)
{
    // Add protect to double init call by having a check on the status of the mutex
    if(!hal_MutexIsValid(qvCHIP_KvsMutex))
    {
        hal_MutexCreate(&qvCHIP_KvsMutex);
    }
    if(!hal_MutexIsValid(qvCHIP_KvsMutex))
    {
        return QV_STATUS_NVM_ERROR;
    }

    // LUT handle will be initialized on first use
    qvCHIP_KvsLookupHandle = gpNvm_LookupTable_Handle_Invalid;


    return QV_STATUS_NO_ERROR;
}

qvStatus_t qvCHIP_KvsConsistencyCheck(void)
{
    gpNvm_Result_t nvmResult;

    qvStatus_t qvStatus = QV_STATUS_NO_ERROR;


    if(!hal_MutexIsValid(qvCHIP_KvsMutex))
    {
        return QV_STATUS_WRONG_STATE;
    }
    hal_MutexAcquire(qvCHIP_KvsMutex);

    uint8_t keySize;
    qvCHIP_KvsToken_t tokenMask = {.componentId = GP_COMPONENT_ID, .type = 0, .index = 0};
    uint8_t tokenMaskLen;
    qvCHIP_KvsKeyPayload_t* kvsKey = NULL;
    uint8_t* headKeyIndexes = NULL;
    uint8_t* pHeadKeyPayloadIndices[GP_NVM_NBR_OF_UNIQUE_TOKENS];
    uint8_t* dataKeyIndexes = NULL;
    uint8_t nrOfHeadKeys = 0;
    uint8_t nrOfDataKeys = 0;
    uint8_t allZeroesHash[MAX_KVS_KEY_LEN] = {0};

    kvsKey = malloc(sizeof(qvCHIP_KvsKeyPayload_t));
    headKeyIndexes = malloc(GP_NVM_NBR_OF_UNIQUE_TOKENS);
    dataKeyIndexes = malloc(GP_NVM_NBR_OF_UNIQUE_TOKENS);
    if(kvsKey == NULL || headKeyIndexes == NULL || dataKeyIndexes == NULL)
    {
        // !!! Problem - cannot allocate memory
        qvStatus = QV_STATUS_NVM_ERROR;
        goto _cleanup;
    }

    qvStatus = qvCHIPKvs_BuildLookup();
    if(QV_STATUS_NO_ERROR != qvStatus)
    {
        goto _cleanup;
    }

    // First delete all keys that were not deleted properly - search for keys with hash all zeroes
    // Note: this can be moved to a separate clean-up function to decrease total run-time for consistency check
    qvStatus_t deleteStatus;
    do
    {
        deleteStatus = qvCHIP_KvsDeleteInternal(allZeroesHash);
    } while(deleteStatus != QV_STATUS_INVALID_DATA);

    nvmResult = gpNvm_ResetIterator(qvCHIP_KvsLookupHandle);
    if(nvmResult != gpNvm_Result_DataAvailable)
    {
        qvStatus = QV_STATUS_NVM_ERROR;
        goto _cleanup;
    }

    // Now go through the remaining keys and check structural integrity
    do
    {
        nvmResult = gpNvm_ReadNextProtected(qvCHIP_KvsLookupHandle, KVS_POOL_ID, NULL, sizeof(tokenMask), &tokenMaskLen,
                                            (uint8_t*)&tokenMask, MAX_KVS_VALUE_LEN, &keySize, (uint8_t*)kvsKey);
        // If a token has been found and it is a KVS record
        if(nvmResult == gpNvm_Result_DataAvailable)
        {
            if(tokenMask.type == qvCHIP_KvsTypeHead)
            {
                headKeyIndexes[nrOfHeadKeys] = tokenMask.index;
                // Allocate memory and copy payload indices - only if there are any
                pHeadKeyPayloadIndices[nrOfHeadKeys] = NULL;
                if(kvsKey->payloadIndices[0] != INVALID_KEY_INDEX)
                {
                    pHeadKeyPayloadIndices[nrOfHeadKeys] = malloc(MAX_KVS_PAYLOAD_EXTENSIONS);
                    if(pHeadKeyPayloadIndices[nrOfHeadKeys] == NULL)
                    {
                        // !!! Problem - cannot allocate memory
                        qvStatus = QV_STATUS_NVM_ERROR;
                        goto _cleanup;
                    }
                    MEMCPY((uint8_t*)pHeadKeyPayloadIndices[nrOfHeadKeys], (uint8_t*)kvsKey->payloadIndices, MAX_KVS_PAYLOAD_EXTENSIONS);
                }

                // Check datalen and data payloads
                if(kvsKey->payloadIndices[0] != INVALID_KEY_INDEX && keySize < MAX_KVS_KEY_LEN)
                {
                    GP_LOG_SYSTEM_PRINTF("!!! KVS inconsistency: key shorter than maximum, should fit in the head element", 0);
                    GP_ASSERT_DEV_INT(false);
                }
                nrOfHeadKeys += 1;
            }
            else if(tokenMask.type == qvCHIP_KvsTypeData)
            {
                dataKeyIndexes[nrOfDataKeys] = tokenMask.index;
                nrOfDataKeys += 1;
            }
            else
            {
                GP_LOG_SYSTEM_PRINTF("!!! KVS inconsistency: unexpected key type (%d)", 0, tokenMask.type);
                GP_ASSERT_DEV_INT(false);
            }
        }
    } while(nvmResult == gpNvm_Result_DataAvailable);

    // Check that all data elements referenced from head elements are found
    for(uint8_t i = 0; i < nrOfHeadKeys; i++)
    {
        if(pHeadKeyPayloadIndices[i] == NULL)
        {
            continue;
        }
        for(uint8_t j = 0; j < MAX_KVS_PAYLOAD_EXTENSIONS; j++)
        {
            if(pHeadKeyPayloadIndices[i][j] != INVALID_KEY_INDEX)
            {
                // Look for the index in data indexes
                bool found = false;
                for(uint8_t k = 0; k < nrOfDataKeys; k++)
                {
                    if(dataKeyIndexes[k] == pHeadKeyPayloadIndices[i][j])
                    {
                        found = true;
                        // Invalidate found index, to make sure it is not double refferenced
                        dataKeyIndexes[k] = INVALID_KEY_INDEX;
                        break;
                    }
                }
                if(!found)
                {
                    GP_LOG_SYSTEM_PRINTF("!!! KVS inconsistency: data element index was not found (%d)", 0,
                                         pHeadKeyPayloadIndices[i][j]);
                    GP_ASSERT_DEV_INT(false);
                }
            }
        }
    }
    // At this point, all data indexes should have been invalidated
    for(uint8_t k = 0; k < nrOfDataKeys; k++)
    {
        if(dataKeyIndexes[k] != INVALID_KEY_INDEX)
        {
            GP_LOG_SYSTEM_PRINTF("!!! KVS inconsistency: removing orphan data element (%d)", 0, dataKeyIndexes[k]);
            // Remove invalid token
            tokenMaskLen = sizeof(qvCHIP_KvsToken_t);
            tokenMask.componentId = GP_COMPONENT_ID;
            tokenMask.type = qvCHIP_KvsTypeData;
            tokenMask.index = dataKeyIndexes[k];
            nvmResult = gpNvm_RemoveProtected(KVS_POOL_ID, gpNvm_UpdateFrequencyIgnore, tokenMaskLen, (uint8_t*)&tokenMask);
            if(nvmResult != gpNvm_Result_DataAvailable)
            {
                qvStatus = QV_STATUS_NVM_ERROR;
                goto _cleanup;
            }
        }
    }

_cleanup:
    if(kvsKey != NULL)
    {
        free(kvsKey);
    }
    if(headKeyIndexes != NULL)
    {
        free(headKeyIndexes);
    }
    if(dataKeyIndexes != NULL)
    {
        free(dataKeyIndexes);
    }
    for(uint8_t i = 0; i < nrOfHeadKeys; i++)
    {
        if(pHeadKeyPayloadIndices[i] != NULL)
        {
            free(pHeadKeyPayloadIndices[i]);
        }
    }

    // Release Mutex
    hal_MutexRelease(qvCHIP_KvsMutex);


    return qvStatus;
}

qvStatus_t qvCHIP_KvsPut(const char* key, const void* value, size_t valueSize)
{
    qvStatus_t qvStatus;
    UInt8 keyIndex;
    uint8_t keyHash[MAX_KVS_KEY_LEN];

    if((key == NULL) || (value == NULL))
    {
        return QV_STATUS_INVALID_ARGUMENT;
    }

    if((valueSize > MAX_PAYLOAD_IN_HEAD_ELEM) &&
       (((valueSize - MAX_PAYLOAD_IN_HEAD_ELEM) / MAX_KVS_VALUE_LEN) + 1) > MAX_KVS_PAYLOAD_EXTENSIONS)
    {
        // Element too large to be split up in extension parts
        return QV_STATUS_INVALID_ARGUMENT;
    }

    qvStatus = qvCHIP_KvsCreateHash(key, keyHash);
    if(QV_STATUS_NO_ERROR != qvStatus)
    {
        return qvStatus;
    }

    if(!hal_MutexIsValid(qvCHIP_KvsMutex))
    {
        return QV_STATUS_WRONG_STATE;
    }
    hal_MutexAcquire(qvCHIP_KvsMutex);

    qvStatus = qvCHIPKvs_BuildLookup();
    if(QV_STATUS_NO_ERROR != qvStatus)
    {
        goto _cleanup;
    }

    UInt8 payloadIndices[MAX_KVS_PAYLOAD_EXTENSIONS];

    // Retrieve list of payload data elements that come with the key
    qvStatus = qvCHIPVS_FindKeyHeadInfo(keyHash, &keyIndex, payloadIndices, 0, NULL, NULL);
    if(QV_STATUS_INVALID_DATA == qvStatus && keyIndex == INVALID_KEY_INDEX)
    {
        //No key found yet in NVM - initialize:
        keyIndex = qvCHIPKvs_FindFreeIndex(qvCHIP_KvsTypeHead);
        if(keyIndex == INVALID_KEY_INDEX)
        {
            GP_LOG_SYSTEM_PRINTF("NVM error - no free index", 0);
            qvStatus = QV_STATUS_NVM_ERROR;
            goto _cleanup;
        }
        MEMSET(payloadIndices, INVALID_KEY_INDEX, MAX_KVS_PAYLOAD_EXTENSIONS);
    }
    else if(QV_STATUS_NO_ERROR != qvStatus)
    {
        GP_LOG_PRINTF("KvsPut FindKey '%s' r:%x", 0, key, qvStatus);
        goto _cleanup;
    }

    if(valueSize > MAX_PAYLOAD_IN_HEAD_ELEM)
    {
        // Store data parts first - in case operation is interrupted these can be flagged as invalid
        qvStatus = qvCHIP_KvsAddDataElements(valueSize - MAX_PAYLOAD_IN_HEAD_ELEM,
                                             (const void*)&((uint8_t*)value)[MAX_PAYLOAD_IN_HEAD_ELEM],
                                             payloadIndices);
        if(QV_STATUS_NO_ERROR != qvStatus)
        {
            goto _cleanup;
        }
    }
    // Finish by writing key reference
    qvStatus = qvCHIP_KvsAddHeadElement(keyIndex, keyHash, MIN(valueSize, MAX_PAYLOAD_IN_HEAD_ELEM), value, payloadIndices);
    if(QV_STATUS_NO_ERROR != qvStatus)
    {
        goto _cleanup;
    }

_cleanup:
    hal_MutexRelease(qvCHIP_KvsMutex);

    return qvStatus;
}

qvStatus_t qvCHIP_KvsGet(const char* key, void* value, size_t valueSize, size_t* readBytesSize,
                         size_t offsetBytes)
{
    uint8_t keyHash[MAX_KVS_KEY_LEN];
    qvStatus_t qvStatus = QV_STATUS_NO_ERROR;

    /* check parameters*/
    if((key == NULL) || (value == NULL) || (readBytesSize == NULL) ||
       (valueSize > (MAX_KVS_PAYLOAD_EXTENSIONS * MAX_KVS_VALUE_LEN) + MAX_PAYLOAD_IN_HEAD_ELEM))
    {
        return QV_STATUS_INVALID_ARGUMENT;
    }

    // Caller expects this to be initialized.
    *readBytesSize = 0;

    qvStatus = qvCHIP_KvsCreateHash(key, keyHash);
    if(QV_STATUS_NO_ERROR != qvStatus)
    {
        return qvStatus;
    }

    if(!hal_MutexIsValid(qvCHIP_KvsMutex))
    {
        return QV_STATUS_WRONG_STATE;
    }
    hal_MutexAcquire(qvCHIP_KvsMutex);

    qvStatus = qvCHIPKvs_BuildLookup();
    if(QV_STATUS_NO_ERROR != qvStatus)
    {
        goto _cleanup;
    }

    UInt8 payloadIndices[MAX_KVS_PAYLOAD_EXTENSIONS];
    UInt8 keyIndex;
    NOT_USED(keyIndex);

    // Retrieve list of payload elements and payload data from head info structure
    qvStatus = qvCHIPVS_FindKeyHeadInfo(keyHash, &keyIndex, payloadIndices, valueSize, readBytesSize, (uint8_t*)value);
    if(QV_STATUS_NO_ERROR != qvStatus)
    {
        goto _cleanup;
    }

    // If we don't have any more payload data and offset is larger than what we read
    if(offsetBytes != 0 && offsetBytes >= *readBytesSize && *readBytesSize < MAX_PAYLOAD_IN_HEAD_ELEM)
    {
        qvStatus = QV_STATUS_INVALID_DATA;
        goto _cleanup;
    }
    // If offset is between 0 and data read length, we need to shift the data to the left and update offset and readBytesSize
    if(offsetBytes > 0 && offsetBytes < *readBytesSize)
    {
        MEMCPY_INPLACE(value, (void*)&((uint8_t*)value)[offsetBytes], *readBytesSize - offsetBytes);
        *readBytesSize -= offsetBytes;
        offsetBytes = 0;
    }
    else if(offsetBytes >= MAX_PAYLOAD_IN_HEAD_ELEM)
    {
        offsetBytes -= MAX_PAYLOAD_IN_HEAD_ELEM;
        *readBytesSize = 0;
    }
    valueSize -= *readBytesSize;

    // If we have some more payload elements and still room left in the caller buffer
    if(payloadIndices[0] != INVALID_KEY_INDEX && valueSize > 0)
    {
        // Fill up the read value with all payload elements listed
        // readBytesSize will be used inside the function as offset in value array
        qvStatus = qvCHIP_KvsGetDataElements(payloadIndices, offsetBytes, valueSize, readBytesSize, (uint8_t*)value);
        if(QV_STATUS_NO_ERROR != qvStatus)
        {
            goto _cleanup;
        }
    }

_cleanup:
    hal_MutexRelease(qvCHIP_KvsMutex);

    return qvStatus;
}

/** @brief Delete a KVS element - protected by a mutex */
qvStatus_t qvCHIP_KvsDelete(const char* key)
{
    qvStatus_t qvStatus;
    UInt8 keyHash[MAX_KVS_KEY_LEN];


    if((key == NULL))
    {
        return QV_STATUS_INVALID_ARGUMENT;
    }

    qvStatus = qvCHIP_KvsCreateHash(key, keyHash);
    if(QV_STATUS_NO_ERROR != qvStatus)
    {
        return qvStatus;
    }

    if(!hal_MutexIsValid(qvCHIP_KvsMutex))
    {
        return QV_STATUS_WRONG_STATE;
    }
    hal_MutexAcquire(qvCHIP_KvsMutex);

    qvStatus = qvCHIPKvs_BuildLookup();
    if(QV_STATUS_NO_ERROR != qvStatus)
    {
        goto _cleanup;
    }
    qvStatus = qvCHIP_KvsDeleteInternal(keyHash);

_cleanup:
    hal_MutexRelease(qvCHIP_KvsMutex);


    return qvStatus;
}
qvStatus_t qvCHIP_KvsErasePartition(void)
{
    gpNvm_Result_t nvmResult;
    qvStatus_t qvStatus = QV_STATUS_NO_ERROR;


    if(!hal_MutexIsValid(qvCHIP_KvsMutex))
    {
        return QV_STATUS_WRONG_STATE;
    }
    hal_MutexAcquire(qvCHIP_KvsMutex);

    // Note: Pool can contain more then only KVS data if configured so at build-time
    nvmResult = gpNvm_ErasePool(KVS_POOL_ID);
    if(nvmResult != gpNvm_Result_DataAvailable)
    {
        qvStatus = QV_STATUS_NVM_ERROR;
        goto _cleanup;
    }

_cleanup:
    // Release Mutex
    hal_MutexRelease(qvCHIP_KvsMutex);


    return qvStatus;
}
