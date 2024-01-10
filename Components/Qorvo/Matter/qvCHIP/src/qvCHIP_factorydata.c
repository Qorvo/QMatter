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

/** @file "qvCHIP_factorydata.c"
 *
 *  factory-generated data retrieval
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
/* <CodeGenerator Placeholder> Includes */

#define GP_COMPONENT_ID GP_COMPONENT_ID_QVCHIP

#include "qvCHIP.h"
#include "gpHal.h"

#include "gpLog.h"

/* </CodeGenerator Placeholder> Includes */

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> Macro */
/** @brief Magic word placed at factory data block start */
#define FACTORY_DATA_MAGIC  (0x41444651)
#define FIRMWARE_DATA_MAGIC (0x44494651)

/** @brief Length of DAC private key
 *  handled specifically as it can be stored in a specific area. */
#define FACTORY_DATA_DAC_KEY_LENGTH 0x20

typedef PACKED_PRE struct {
    uint32_t tag_id;
    uint32_t data_length;
} PACKED_POST tag_t;
/* </CodeGenerator Placeholder> Macro */

/*****************************************************************************
 *                    Static Data
 *****************************************************************************/

/** @brief Linker symbol to factory data start location */
extern const uint32_t _binary_factory_data_bin_start;
extern const uint32_t _binary_firmware_data_bin_start;

/*****************************************************************************
 *                    Static Component Function Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Component Function Implementations
 *****************************************************************************/

/** @brief Lookup location of factory data with a given tag
 *
 * @param[in]  tag_id      Tag ID to look for
 * @param[out] data        Pointer to buffer pointer. Flash pointer to be returned here.
 * @param[out] data_length Length of located data returned in this pointer.
 *
 * @return status - QV_STATUS_INVALID_ARGUMENT - on null pointer
 *                - QV_STATUS_INVALID_DATA - no magic word found - no factory data linked in?
 *                                         - no data found for tag
 *                - QV_STATUS_NO_ERROR - data for tag located. data pointer and data_length valid.
 */
static qvStatus_t CHIP_LocateTag(uint32_t tag_id, const uint32_t** data, uint32_t* data_length, const uint32_t* section,
                                 const uint32_t magic)
{
    const uint32_t* cursor = section;

    if((data_length == NULL) || (data == NULL))
    {
        return QV_STATUS_INVALID_ARGUMENT;
    }
    if(*cursor != magic)
    {
        GP_LOG_PRINTF("exp %x act %lx", 0, magic, *cursor);
        return QV_STATUS_INVALID_DATA;
    }
    cursor++;

    while(((tag_t*)cursor)->tag_id != TAG_ID_END_MARKER)
    {
        GP_LOG_PRINTF("tag %lu", 0, ((tag_t*)cursor)->tag_id);
        GP_LOG_PRINTF("length %lu", 0, ((tag_t*)cursor)->data_length);
        if(((tag_t*)cursor)->tag_id == tag_id)
        {
            (*data_length) = ((tag_t*)cursor)->data_length;
            (*data) = cursor + 2; // data area
            return QV_STATUS_NO_ERROR;
        }

        // If tag length is not a multiple of 4, we need to add one more word for the alignment
        if((((tag_t*)cursor)->data_length % 4) != 0)
        {
            cursor += 2 /* tag id and tag length */ + ((tag_t*)cursor)->data_length / 4 /* tag contents */ + 1;
        }
        else
        {
            cursor += 2 /* tag id and tag length */ + ((tag_t*)cursor)->data_length / 4 /* tag contents */;
        }
    }

    return QV_STATUS_INVALID_DATA;
}

/** @brief Locate and fetch data for a given tag
 *
 * @param tag[in]          Tag ID for factory data to retrieve
 * @param dst[out]         Buffer for retrieved data.
 * @param buffer_size[in]  Available size of destination buffer.
 * @param data_length[out] Length of retrieved data
 *
 * @return status - QV_STATUS_INVALID_ARGUMENT - NULL pointers given.
 *                - QV_STATUS_BUFFER_TOO_SMALL - destination buffer can't hold tag.
 *                - QV_STATUS_NO_ERROR - data copied into dst buffer and data_length valid.
 */
static qvStatus_t CHIP_FactoryDataCopyValue(const uint32_t* src, uint8_t* dst, uint32_t buffer_size, uint32_t src_size,
                                            uint32_t* data_length)
{
    if((data_length == NULL) || (dst == NULL))
    {
        return QV_STATUS_INVALID_ARGUMENT;
    }

    (*data_length) = src_size;
    if(buffer_size < src_size)
    {
        return QV_STATUS_BUFFER_TOO_SMALL;
    }
    MEMCPY(dst, (uint8_t*)src, src_size);
    return QV_STATUS_NO_ERROR;
}
static qvStatus_t CHIP_FactoryDataGetValue_internal(qvCHIP_FactoryDataTagId_t tag, uint8_t* dst, uint32_t buffer_size,
                                                    uint32_t* data_length)
{
    qvStatus_t status;
    const uint32_t* src;
    uint32_t src_size;

    status = CHIP_LocateTag(tag, &src, &src_size, &_binary_firmware_data_bin_start, FIRMWARE_DATA_MAGIC);
    if(status == QV_STATUS_NO_ERROR)
    {
        return CHIP_FactoryDataCopyValue(src, dst, buffer_size, src_size, data_length);
    }
    status = CHIP_LocateTag(tag, &src, &src_size, &_binary_factory_data_bin_start, FACTORY_DATA_MAGIC);
    if(status == QV_STATUS_NO_ERROR)
    {
        return CHIP_FactoryDataCopyValue(src, dst, buffer_size, src_size, data_length);
    }

    return status;
}

/** @brief Retrieve Private DAC key from HW location.
 *         If not found, revert back to factory data blob for development.
 *
 * @param[out] dst Buffer for key - expected to be 32 bytes - to checked in caller function.
 *
 * @return QV_STATUS_NOT_IMPLEMENTED - no key found in HW storage. No valid data in dst buffer.
 *         QV_STATUS_NO_ERROR        - key found and copied into dst buffer.
 *         QV_STATUS_INVALID_ARGUMENT - NULL pointer given.
 */
static qvStatus_t CHIP_GetDacPrivateKeyFromHW(uint8_t dst[FACTORY_DATA_DAC_KEY_LENGTH])
{
    uint8_t* key_in_infopage;
    uint8_t i;

    if(dst == NULL)
    {
        return QV_STATUS_INVALID_ARGUMENT;
    }

    // We use the area of userkey 8 and 9
    key_in_infopage = (uint8_t*)GP_WB_NVR_USER_KEY_0_LSB_ADDRESS +
                      (8) * (GP_WB_NVR_USER_KEY_0_LSB_LEN + GP_WB_NVR_USER_KEY_0_MSB_LEN);

    // Check existence - all 0 = empty
    for(i = 0; i < FACTORY_DATA_DAC_KEY_LENGTH; i++)
    {
        if(key_in_infopage[i] != 0x00)
        {
            // Active key found - copy over
            MEMCPY(dst, key_in_infopage, FACTORY_DATA_DAC_KEY_LENGTH);
            return QV_STATUS_NO_ERROR;
        }
    }

    // No key found - all zero's
    return QV_STATUS_NOT_IMPLEMENTED;
}

static qvStatus_t CHIP_GetDacPrivateKey(uint8_t* dst, uint32_t buffer_size, uint32_t* data_length)
{
    qvStatus_t status;

    if((dst == NULL) || (data_length == NULL))
    {
        return QV_STATUS_INVALID_ARGUMENT;
    }

    if(buffer_size < FACTORY_DATA_DAC_KEY_LENGTH)
    {
        return QV_STATUS_BUFFER_TOO_SMALL;
    }

    status = CHIP_GetDacPrivateKeyFromHW(dst);
    if(status == QV_STATUS_NO_ERROR)
    {
        *data_length = FACTORY_DATA_DAC_KEY_LENGTH;
    }
    else
    {
        // Use factory block storage as fallback
        status = CHIP_FactoryDataGetValue_internal(TAG_ID_DEVICE_ATTESTATION_PRIVATE_KEY, dst,
                                                   FACTORY_DATA_DAC_KEY_LENGTH, data_length);
    }

    return status;
}

/*****************************************************************************
 *                    Public Function Implementations
 *****************************************************************************/

qvStatus_t qvCHIP_FactoryDataGetValue(qvCHIP_FactoryDataTagId_t tag, uint8_t* dst, uint32_t buffer_size,
                                      uint32_t* data_length)
{
    if(tag == TAG_ID_DEVICE_ATTESTATION_PRIVATE_KEY)
    {
        return CHIP_GetDacPrivateKey(dst, buffer_size, data_length);
    }
    return CHIP_FactoryDataGetValue_internal(tag, dst, buffer_size, data_length);
}
