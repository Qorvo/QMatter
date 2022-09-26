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
 * $Change$
 * $DateTime$
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
 *  @param dst             pointer to write the requested data to
 *  @param buffer_size     size of the buffer to write to
 *  @param out data_length the number of bytes written to the buffer
 *  @return                QV_STATUS_INVALID_ARGUMENT if data is NULL, QV_STATUS_BUFFER_TOO_SMALL if the returned data is too
 *  large to fit the buffer, QV_STATUS_INVALID_DATA if the factory data is missing, otherwise QV_STATUS_NO_ERROR
*/
qvStatus_t qvCHIP_GetProductAttestationIntermediateCert(uint8_t* dst, uint32_t buffer_size, uint32_t* data_length);
qvStatus_t qvCHIP_GetCertificationDeclaration(uint8_t* dst, uint32_t buffer_size, uint32_t* data_length);
qvStatus_t qvCHIP_GetDeviceAttestationCert(uint8_t* dst, uint32_t buffer_size, uint32_t* data_length);
qvStatus_t qvCHIP_GetDeviceAttestationPrivateKey(uint8_t* dst, uint32_t buffer_size, uint32_t* data_length);
qvStatus_t qvCHIP_GetDeviceAttestationPublicKey(uint8_t* dst, uint32_t buffer_size, uint32_t* data_length);
#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_QVCHIP_FACTORYDATA_H_
