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
 */

/** @file "qvCHIP_OTA.c"
 *
 *  OTA Upgrade functionality
 *
 *  Implementation of qvCHIP OTA API
*/

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
/* <CodeGenerator Placeholder> Includes */

#define GP_COMPONENT_ID GP_COMPONENT_ID_QVCHIP

//#define GP_LOCAL_LOG

#include "qvCHIP.h"
#include "gpLog.h"

#include "gpAssert.h"
#include "gpUpgrade.h"

/* </CodeGenerator Placeholder> Includes */

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> Macro */
/* </CodeGenerator Placeholder> Macro */

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticData */
qvCHIP_OtaHeaderValidationCback_t pOtaHeaderValidationCb = NULL;
qvCHIP_OtaUpgradeHandledCback_t pOtaUpgradeHandledCb = NULL;

/* </CodeGenerator Placeholder> StaticData */

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticFunctionDefinitions */
static qvCHIP_OtaStatus_t OtaMapFileOffsetToMemoryMap(uint32_t chunk_offset, uint32_t length, uint32_t* memorymap_offset);
#if !defined(GP_UPGRADE_DIVERSITY_COMPRESSION) && !defined(GP_UPGRADE_DIVERSITY_USE_INTSTORAGE)
static bool OtaChunkExceedsJumpTableBoundary(uint32_t chunk_offset,
                                             uint32_t length,
                                             uint32_t* jump_table_data_size,
                                             uint32_t* ota_data_size);
static qvCHIP_OtaStatus_t OtaWritePartialData(uint32_t offset, uint16_t length, uint8_t* dataChunk);
#endif
/* </CodeGenerator Placeholder> StaticFunctionDefinitions */

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticFunctionDefinitions */
static qvCHIP_OtaStatus_t OtaMapFileOffsetToMemoryMap(uint32_t chunk_offset, uint32_t length, uint32_t* memorymap_offset)
{
    uint32_t jumptable_length = gpUpgrade_GetJumptableOtaAreaSize();

    if(!memorymap_offset)
    {
        GP_ASSERT_SYSTEM(memorymap_offset);
        return qvCHIP_OtaStatusInvalidParam;
    }

#if defined(GP_UPGRADE_DIVERSITY_COMPRESSION)
    if(chunk_offset + length > jumptable_length + gpUpgrade_GetOtaAreaSize())
    {
        return qvCHIP_OtaStatusInvalidAddress;
    }

    (*memorymap_offset) = chunk_offset + gpUpgrade_GetOtaAreaStartAddress() - jumptable_length;
    return qvCHIP_OtaStatusSuccess;
#else
    if(chunk_offset < jumptable_length)
    {
        /* Bridging the boundary is not accounted for in this function, so assert. */
        if(chunk_offset + length > jumptable_length)
        {
            return qvCHIP_OtaStatusInvalidAddress;
        }
        (*memorymap_offset) = chunk_offset + gpUpgrade_GetJumptableOtaAreaStartAddress();
        return qvCHIP_OtaStatusSuccess;
    }
    else
    {
        (*memorymap_offset) = chunk_offset + gpUpgrade_GetOtaAreaStartAddress() - jumptable_length;
        return qvCHIP_OtaStatusSuccess;
    }
#endif
}

#if !defined(GP_UPGRADE_DIVERSITY_COMPRESSION) && !defined(GP_UPGRADE_DIVERSITY_USE_INTSTORAGE)
static bool OtaChunkExceedsJumpTableBoundary(uint32_t chunk_offset,
                                             uint32_t length,
                                             uint32_t* jump_table_data_size,
                                             uint32_t* ota_data_size)
{
    uint32_t jumptable_length = gpUpgrade_GetJumptableOtaAreaSize();
    bool retVal = false;

    if(chunk_offset < jumptable_length)
    {
        if(chunk_offset + length > jumptable_length)
        {
            (*jump_table_data_size) = jumptable_length - chunk_offset;
            (*ota_data_size) = (chunk_offset + length) - jumptable_length;
            retVal = true;
        }
    }

    return retVal;
}

static qvCHIP_OtaStatus_t OtaWritePartialData(uint32_t offset, uint16_t length, uint8_t* dataChunk)
{
    qvCHIP_OtaStatus_t result = qvCHIP_OtaStatusSuccess;
    uint32_t memorymap_offset = 0;

    result = OtaMapFileOffsetToMemoryMap(offset, length, &memorymap_offset);

    if(result != qvCHIP_OtaStatusSuccess)
    {
        return result;
    }

    GP_LOG_PRINTF("gpUpgrade_WriteChunkaddr:%lx l:%u", 0, (unsigned long)memorymap_offset, length);
    return (qvCHIP_OtaStatus_t)gpUpgrade_WriteChunk(memorymap_offset, length, dataChunk);
}
#endif

/* </CodeGenerator Placeholder> StaticFunctionDefinitions */

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

bool qvCHIP_OtaValidateImage(qvCHIP_Ota_ImageHeader_t imageHeader)
{
    GP_LOG_PRINTF("qvCHIP_OtaValidateImage", 0);
    if(pOtaHeaderValidationCb == NULL)
    {
        return true;
    }

    return pOtaHeaderValidationCb(imageHeader);
}

void qvCHIP_OtaEraseArea(void)
{
    GP_LOG_PRINTF("qvCHIP_OtaEraseArea", 0);
    gpUpgrade_EraseOtaArea();
}

void qvCHIP_OtaStartEraseArea(qvCHIP_OtaEraseCompleteCback_t cb)
{
    GP_LOG_PRINTF("qvCHIP_OtaStartEraseArea", 0);
    gpUpgrade_StartEraseOtaArea(cb);
}

uint32_t qvCHIP_OtaGetAreaSize(void)
{
    GP_LOG_PRINTF("qvCHIP_OtaGetAreaSize", 0);
    return gpUpgrade_GetOtaAreaSize();
}

void qvCHIP_OtaStartWrite(void)
{
    GP_LOG_PRINTF("qvCHIP_OtaStartWrite", 0);
    gpUpgrade_StartWrite();
}

qvCHIP_OtaStatus_t qvCHIP_OtaWriteChunk(uint32_t offset, uint16_t length, uint8_t* dataChunk)
{
    qvCHIP_OtaStatus_t result = qvCHIP_OtaStatusSuccess;
    uint32_t memorymap_offset = 0;
    GP_LOG_PRINTF("qvCHIP_OtaWriteChunk addr:%lx l:%u", 0, (unsigned long)offset, length);
    if(NULL == dataChunk || 0 == length)
    {
        return qvCHIP_OtaStatusInvalidParam;
    }

#if !defined(GP_UPGRADE_DIVERSITY_COMPRESSION) && !defined(GP_UPGRADE_DIVERSITY_USE_INTSTORAGE)
    uint32_t jump_table_data_size = 0;
    uint32_t ota_data_size = 0;

    if(OtaChunkExceedsJumpTableBoundary(offset, length, &jump_table_data_size, &ota_data_size))
    {
        //We received a block that contains data for Jumptable and data for upgrade application. This needs to be split
        //and stored in seperate sections in flash. See Figure 2.2 of the document below to understand how the jumptables
        //and the application are stored in flash (similar layout for external OTA and internal non-compressed OTA)
        //Components/Qorvo/Bootloader/vlatest/apps/AppBootloader/doc/pdf/SW30236_AN_Vol_2_Secure_User_Mode_Bootloader_Implementation.pdf

        GP_ASSERT_SYSTEM(length == (jump_table_data_size + ota_data_size));

        result = OtaWritePartialData(offset, jump_table_data_size, dataChunk);
        if(result != qvCHIP_OtaStatusSuccess)
        {
            return result;
        }

        result = OtaWritePartialData(offset + jump_table_data_size, ota_data_size, dataChunk + jump_table_data_size);
        if(result != qvCHIP_OtaStatusSuccess)
        {
            return result;
        }

        return qvCHIP_OtaStatusSuccess;
    }
#endif //!defined(GP_UPGRADE_DIVERSITY_COMPRESSION) && !defined(GP_UPGRADE_DIVERSITY_USE_INTSTORAGE)

    result = OtaMapFileOffsetToMemoryMap(offset, length, &memorymap_offset);

    if(result != qvCHIP_OtaStatusSuccess)
    {
        return result;
    }

    GP_LOG_PRINTF("gpUpgrade_WriteChunkaddr:%lx l:%u", 0, (unsigned long)memorymap_offset, length);
    return (qvCHIP_OtaStatus_t)gpUpgrade_WriteChunk(memorymap_offset, length, dataChunk);
}

qvCHIP_OtaStatus_t qvCHIP_OtaReadChunk(uint32_t offset, uint16_t length, uint8_t* dataChunk)
{
    qvCHIP_OtaStatus_t result;
    uint32_t memorymap_offset;
    GP_LOG_PRINTF("qvCHIP_OtaReadChunk addr:%lx l:%u", 0, (unsigned long)offset, length);
    if(NULL == dataChunk || 0 == length)
    {
        return qvCHIP_OtaStatusInvalidParam;
    }

    result = OtaMapFileOffsetToMemoryMap(offset, length, &memorymap_offset);

    if(result != qvCHIP_OtaStatusSuccess)
    {
        return result;
    }

    return (qvCHIP_OtaStatus_t)gpUpgrade_ReadChunk(memorymap_offset, length, dataChunk);
}

uint32_t qvCHIP_OtaGetCrc(void)
{
    GP_LOG_PRINTF("qvCHIP_OtaGetCrc", 0);
    return gpUpgrade_GetCrc();
}

void qvCHIP_OtaSetCrc(uint32_t crcValue)
{
    GP_LOG_PRINTF("qvCHIP_OtaSetCrc", 0);
    gpUpgrade_SetCrc(crcValue);
}

qvCHIP_OtaStatus_t qvCHIP_OtaSetPendingImage(void)
{
    uint32_t swVer = 0;
    uint32_t hwVer = 0;
    uint32_t startAddr = 0;
    uint32_t imgSz = 0;
    GP_LOG_PRINTF("qvCHIP_OtaSetPendingImage", 0);

    // NOTE : In this implemenation the arguments provided in this API are unused.
    // Instead the License area based approach is used where the versions are stored
    // a dedicated programmable memory area during the factory flow.
    return (qvCHIP_OtaStatus_t)gpUpgrade_SetPendingImage(swVer, hwVer, startAddr, imgSz);
}

void qvCHIP_OtaReset(void)
{
    GP_LOG_PRINTF("qvCHIP_OtaReset", 0);
    gpUpgrade_Reset();
}

void gpUpgrade_cbUpgradeHandled(bool upgradeHandled, qvCHIP_OtaStatus_t upgradeStatus)
{
    GP_LOG_PRINTF("gpUpgrade_cbUpgradeHandled", 0);
    if(pOtaUpgradeHandledCb != NULL)
    {
        pOtaUpgradeHandledCb(upgradeHandled, upgradeStatus);
    }
}

void qvCHIP_OtaSetUpgradeHandledCb(qvCHIP_OtaUpgradeHandledCback_t upgradeHandledCb)
{
    GP_LOG_PRINTF("qvCHIP_OtaSetUpgradeHandledCb", 0);
    pOtaUpgradeHandledCb = upgradeHandledCb;
}

void qvCHIP_OtaSetHeaderValidationCb(qvCHIP_OtaHeaderValidationCback_t headerValidationCb)
{
    GP_LOG_PRINTF("qvCHIP_OtaSetHeaderValidationCb", 0);
    pOtaHeaderValidationCb = headerValidationCb;
}
