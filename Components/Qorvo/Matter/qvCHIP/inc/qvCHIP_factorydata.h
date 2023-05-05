/*
 * Copyright (c) 2020, Qorvo Inc
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

/** @file "qvCHIP_KVS.h"
 *
 *  CHIP wrapper KVS API
 *
 *  Declarations of the KVS specific public functions and enumerations of qvCHIP.
*/

#ifndef _QVCHIP_FACTORYDATA_H_
#define _QVCHIP_FACTORYDATA_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include <stdint.h>

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/** @name qvCHIP_FactoryDataTagId_t */
//@{
#define TAG_ID_END_MARKER 0x00000000
// DeviceAttestationCredentialsProvider
#define TAG_ID_CERTIFICATION_DECLARATION                    0x00000001
#define TAG_ID_FIRMWARE_INFORMATION                         0x00000002
#define TAG_ID_DEVICE_ATTESTATION_CERTIFICATE               0x00000003
#define TAG_ID_PRODUCT_ATTESTATION_INTERMEDIATE_CERTIFICATE 0x00000004
#define TAG_ID_DEVICE_ATTESTATION_PRIVATE_KEY               0x00000005
#define TAG_ID_DEVICE_ATTESTATION_PUBLIC_KEY                0x00000006
// CommissionableDataProvider
#define TAG_ID_SETUP_DISCRIMINATOR    0x0000000F
#define TAG_ID_SPAKE2_ITERATION_COUNT 0x00000010
#define TAG_ID_SPAKE2_SALT            0x00000011
#define TAG_ID_SPAKE2_VERIFIER        0x00000012
#define TAG_ID_SPAKE2_SETUP_PASSCODE  0x00000013
// DeviceInstanceInfoProvider
#define TAG_ID_VENDOR_NAME             0x00000019
#define TAG_ID_VENDOR_ID               0x0000001A
#define TAG_ID_PRODUCT_NAME            0x0000001B
#define TAG_ID_PRODUCT_ID              0x0000001C
#define TAG_ID_SERIAL_NUMBER           0x0000001D
#define TAG_ID_MANUFACTURING_DATE      0x0000001E
#define TAG_ID_HARDWARE_VERSION        0x0000001F
#define TAG_ID_HARDWARE_VERSION_STRING 0x00000020
#define TAG_ID_ROTATING_DEVICE_ID      0x00000021
// Platform specific
#define TAG_ID_ENABLE_KEY 0x00000028
// Manufacturer specific
#define TAG_ID_MANUFACTURER_SPECIFIC_1 0x40000000
/** @typedef qvCHIP_FactoryDataTagId_t
 *  @brief TLV Tag IDs for various factory data
*/
typedef uint32_t qvCHIP_FactoryDataTagId_t;
//@}

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 *                    NVM API
 *****************************************************************************/
/** @brief Get Factory data
 *  @param tag             ID of the tag in the TLV structure holding the data of interest
 *  @param dst             pointer to destination data
 *  @param buffer_size     size of the destination buffer
 *  @param out data_length the number of bytes written to the buffer
 *  @return                QV_STATUS_INVALID_ARGUMENT if data is NULL,
 *                         QV_STATUS_BUFFER_TOO_SMALL if the returned data is too large to fit the buffer,
 *                         QV_STATUS_INVALID_DATA if the factory data is missing,
 *                         QV_STATUS_NO_ERROR otherwise
*/
qvStatus_t qvCHIP_FactoryDataGetValue(qvCHIP_FactoryDataTagId_t tag, uint8_t* dst, uint32_t buffer_size, uint32_t* data_length);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_QVCHIP_FACTORYDATA_H_
