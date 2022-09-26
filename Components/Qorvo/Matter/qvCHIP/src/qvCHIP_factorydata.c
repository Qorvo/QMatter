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
 * $Header$
 * $Change$
 * $DateTime$
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

#include "gpLog.h"

/* </CodeGenerator Placeholder> Includes */

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> Macro */
#define FACTORY_DATA_MAGIC (0x41444651)

#define NVM_DIV_ROUND_UP(n, k)         (((n) + ((k)-1)) / (k))
#define NVM_ROUND_UP_TO_MULTIPLE(n, k) (NVM_DIV_ROUND_UP((n), (k)) * (k))

typedef enum {
    TAG_ID_END_MARKER = 0,
    TAG_ID_TEST_DAC_CERT = 1,
    TAG_ID_TEST_DAC_PRIVATE_KEY = 2,
    TAG_ID_TEST_DAC_PUBLIC_KEY = 3,
    TAG_ID_PAI_CERT = 4,
    TAG_ID_CERTIFICATION_DECLARATION = 5,
    TAG_ID_DISCRIMINATOR = 6,
    TAG_ID_ITERATION_COUNT = 7,
    TAG_ID_SALT = 8,
    TAG_ID_VERIFIER = 9
} factory_data_tag_id_t;

typedef PACKED_PRE struct {
    uint32_t tag_id;
    uint32_t data_length;
} PACKED_POST tag_t;
/* </CodeGenerator Placeholder> Macro */

/*****************************************************************************
 *                    Static Data
 *****************************************************************************/
extern const uint32_t _binary_factory_data_bin_start;

/*****************************************************************************
 *                    Static Component Function Definitions
 *****************************************************************************/
static qvStatus_t qvCHIP_LocateTag(uint32_t tag_id, const uint32_t** data, uint32_t* data_length)
{
    const uint32_t* cursor = &_binary_factory_data_bin_start;
    if(!data_length || !data)
    {
        return QV_STATUS_INVALID_ARGUMENT;
    }
    if(*cursor != FACTORY_DATA_MAGIC)
    {
        GP_LOG_PRINTF("exp %x act %lx", 0, FACTORY_DATA_MAGIC, *cursor);
        return QV_STATUS_INVALID_DATA;
    }
    cursor++;

    for(;
        ((tag_t*)cursor)->tag_id != TAG_ID_END_MARKER;
        cursor += (2 + NVM_ROUND_UP_TO_MULTIPLE(((tag_t*)cursor)->data_length, 4) / 4))
    {
        GP_LOG_PRINTF("tag %lu", 0, ((tag_t*)cursor)->tag_id);
        GP_LOG_PRINTF("length %lu", 0, ((tag_t*)cursor)->data_length);
        if(((tag_t*)cursor)->tag_id == tag_id)
        {
            (*data_length) = ((tag_t*)cursor)->data_length;
            (*data) = cursor;
            return QV_STATUS_NO_ERROR;
        }
    }
    return QV_STATUS_INVALID_DATA;
}
static qvStatus_t qvCHIP_GetData(factory_data_tag_id_t tag, uint8_t* dst, uint32_t buffer_size, uint32_t* data_length)
{
    /* <CodeGenerator Placeholder> Implementation_qvCHIP_GetData */
    qvStatus_t status;
    const uint32_t* src;
    uint32_t src_size;

    if(dst == NULL || data_length == NULL)
    {
        return QV_STATUS_INVALID_ARGUMENT;
    }

    status = qvCHIP_LocateTag(tag, &src, &src_size);

    if(status != QV_STATUS_NO_ERROR)
    {
        return status;
    }
    if(buffer_size < src_size)
    {
        return QV_STATUS_BUFFER_TOO_SMALL;
    }
    MEMCPY(dst, src, src_size);
    (*data_length) = src_size;

    return QV_STATUS_NO_ERROR;
    /* </CodeGenerator Placeholder> Implementation_qvCHIP_GetData */
}
/*****************************************************************************
 *                    Public Component Function Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

qvStatus_t qvCHIP_GetProductAttestationIntermediateCert(uint8_t* dst, uint32_t buffer_size, uint32_t* data_length)
{
    return qvCHIP_GetData(TAG_ID_PAI_CERT, dst, buffer_size, data_length);
}
qvStatus_t qvCHIP_GetCertificationDeclaration(uint8_t* dst, uint32_t buffer_size, uint32_t* data_length)
{
    return qvCHIP_GetData(TAG_ID_CERTIFICATION_DECLARATION, dst, buffer_size, data_length);
}
qvStatus_t qvCHIP_GetDeviceAttestationCert(uint8_t* dst, uint32_t buffer_size, uint32_t* data_length)
{
    return qvCHIP_GetData(TAG_ID_TEST_DAC_CERT, dst, buffer_size, data_length);
}
qvStatus_t qvCHIP_GetDeviceAttestationPrivateKey(uint8_t* dst, uint32_t buffer_size, uint32_t* data_length)
{
    return qvCHIP_GetData(TAG_ID_TEST_DAC_PRIVATE_KEY, dst, buffer_size, data_length);
}
qvStatus_t qvCHIP_GetDeviceAttestationPublicKey(uint8_t* dst, uint32_t buffer_size, uint32_t* data_length)
{
    return qvCHIP_GetData(TAG_ID_TEST_DAC_PUBLIC_KEY, dst, buffer_size, data_length);
}
