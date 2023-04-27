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
 * $Header$
 * $Change$
 * $DateTime$
 *
 */

#define GP_COMPONENT_ID GP_COMPONENT_ID_APP

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
//#define GP_LOCAL_LOG

#include "BleOtaClient.h"
#include "BleModule.h"
#include "BlePeripheral.h"

#include "gpLog.h"
#include "gpVersion.h"
#include "gpOta.h"

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
/* Control block */
static struct
{
    UInt8           linkId;
    gpOta_FuncPtr_t otaFuncPtr;
} bleOtaClCb;

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/
static void processImageNotification(UInt16 manufCode, gpOta_ImageType_t imageType, UInt32 newVersion)
{
    UInt32 activeVersion = 0;
    gpVersion_ReleaseInfo_t swVersion;

    gpVersion_GetSoftwareVersion(&swVersion);

    activeVersion = (UInt32)swVersion.patch ||
                    ((UInt32)swVersion.revision << 8) ||
                    ((UInt32)swVersion.minor << 16) ||
                    ((UInt32)swVersion.major << 24);

    //TODO: Check if update required
    GP_LOG_PRINTF("activeVersion:0x%08lx -> newVersion:0x%08lx", 0, activeVersion, newVersion);
    if (false == gpOta_StartOtaSession(bleOtaClCb.linkId,
                                       manufCode,
                                       imageType,
                                       activeVersion,
                                       0xFFFF,      // Hardware Version not Present
                                       &bleOtaClCb.otaFuncPtr))
    {
        GP_LOG_PRINTF("Failed to start OTA session", 0);
    }
    else
    {
        GP_LOG_PRINTF("OTA session started", 0);
    }
}

static void cbOtaStatus(gpOta_OtaState_t status, void *statusParam)
{
    switch (status)
    {
        case gpOta_OtaState_QueryImage:
            GP_LOG_PRINTF("Ota Query Image available", 0);
            break;

        case gpOta_OtaState_ImageAvailable:
            GP_LOG_PRINTF("Ota Image available", 0);
            break;

        case gpOta_OtaState_DownloadingHeader:
            GP_LOG_PRINTF("Downloading header", 0);
            break;

        case gpOta_OtaState_DownloadingSubElemTag:
            GP_LOG_PRINTF("Downloading (next) tag", 0);
            break;

        case gpOta_OtaState_DownloadingSubElemData:
            GP_LOG_PRINTF("Downloading (next) image", 0);
            break;

        case gpOta_OtaState_DownloadComplete:
            GP_LOG_PRINTF("Download complete, verify statics", 0);
            break;

        case gpOta_OtaState_UpgradeScheduled:
            GP_LOG_PRINTF("Verify Ok, upgrade scheduled in %d seconds", 0, *((UInt32 *)statusParam));
            break;

        case gpOta_OtaState_UpgradeImage:
            if (*((UInt8 *)statusParam))
            {
                GP_LOG_PRINTF("Upgrade started", 0);
            }
            else
            {
                GP_LOG_PRINTF("Failed to start", 0);
            }

            break;

        case gpOta_OtaState_SubProcComplete:
            switch (*((UInt8 *)statusParam))
            {
                case gpOta_OtaState_ImageAvailableInvalidState:
                    GP_LOG_PRINTF("Ota wrong state to recieve image", 0);
                    break;

                case gpOta_OtaState_ImageAvailableWrongPairId:
                    GP_LOG_PRINTF("Ota image from wrong pairing ID", 0);
                    break;

                case gpOta_OtaState_ImageAvailableStatusNoSuccess:
                    GP_LOG_PRINTF("Ota Image Query no success, reason %u", 0, *((UInt8 *)statusParam + 1));
                    break;

                case gpOta_OtaState_ImageAvailableWrongManufatorCode:
                    GP_LOG_PRINTF("Ota wrong Manufactorer code", 0);
                    break;

                case gpOta_OtaState_ImageAvailableWrongImageType:
                    GP_LOG_PRINTF("Ota wrong image type", 0);
                    break;

                case gpOta_OtaState_ImageAvailableImageOutdated:
                    GP_LOG_PRINTF("Ota image outdated, only newer image allowed", 0);
                    break;

                case gpOta_OtaState_FlashVerifyFailure:
                    GP_LOG_PRINTF("Flash corruption detected", 0);
                    break;

                case gpOta_OtaState_DownloadingFileCrcFailure:
                    GP_LOG_PRINTF("Crc failure on full image", 0);
                    break;

                case gpOta_OtaState_DownloadedChunk:
                    GP_LOG_PRINTF("Ota Chunk downloaded", 0);
                    break;

                case gpOta_OtaState_DownloadFailed:
                    GP_LOG_PRINTF("Ota Download failure! abort!", 0);
                    break;

                case gpOta_OtaState_RemoteAbort:
                    GP_LOG_PRINTF("Upgrade process aborted by peer!", 0);
                    break;

                case gpOta_OtaState_ImageOffsetWrong:
                    GP_LOG_PRINTF("Chunck recieved with wrong offset, download abort!", 0);
                    break;

                case gpOta_OtaState_ImageSizeTooLarge:
                    GP_LOG_PRINTF("Image size to large, download abort!", 0);
                    break;

                case gpOta_OtaState_DownloadHold:
                    GP_LOG_PRINTF("Download hold, hold depth: %u", 0, *((UInt8 *)statusParam + 1));
                    break;

                case gpOta_OtaState_DownloadResume:
                    GP_LOG_PRINTF("Download resume, hold depth: %u", 0, *((UInt8 *)statusParam + 1));
                    break;

                case gpOta_OtaState_DowloadHoldOverUnderflow:
                    GP_LOG_PRINTF("Overflow, underflow in Ota Hold-Resume pairs!", 0);
                    break;

                case gpOta_OtaState_DowloadHoldTimeout:
                    GP_LOG_PRINTF("Hold Max Timeout, download terminated", 0);
                    break;

                case gpOta_OtaState_Abort:
                    GP_LOG_PRINTF("Ota Aborted!", 0);
                    break;

                case gpOta_OtaState_NoDataRequestRegistered:
                    GP_LOG_PRINTF("Data Request called while no data request fuction registered!", 0);
                    break;

                case gpOta_OtaState_NoRemoteResponse:
                    GP_LOG_PRINTF("No remote response while downloading, download terminated!", 0);
                    break;

                case gpOta_OtaState_ReachedEndOfFile:
                    GP_LOG_PRINTF("Mismatch in OTA file length. No more OTA chunks available!", 0);
                    break;

                default:
                    GP_LOG_PRINTF("Ota Query Image failed, unknown reason %u", 0, *((UInt8 *)statusParam));
                    break;
            }
            break;

        case gpOta_OtaState_Idle:
            GP_LOG_PRINTF("Handling OtaStatus %u not yet implemented", 0, status);
            break;

        case gpOta_OtaState_ImageNotificationReceived:
            GP_LOG_PRINTF("Image is avaiable on Server...", 0);
            {
                gpOta_ImageNotification_t *imgData = (gpOta_ImageNotification_t *)statusParam;
                if (imgData->pairingRef == bleOtaClCb.linkId)
                {
                    if ((imgData->imageType == gpOta_ImageTypeMain) || (imgData->imageType == UINT16_MAX))
                    {
                        //TODO: Check if no OTA process in progress
                        processImageNotification(imgData->manufCode, gpOta_ImageTypeMain, imgData->fileVersion);
                    }
                    else
                    {
                        GP_LOG_PRINTF("Not supported Image Type (%d)", 0, imgData->imageType);
                    }
                }
                break;
            }

        default:
            GP_LOG_PRINTF("Unknown OtaStatus recieved %u", 0, status);
            break;
    }
}


/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
void BleOtaClient_Init(void)
{
    bleOtaClCb.linkId                   = UINT8_MAX;
    bleOtaClCb.otaFuncPtr.sendFunction  = NULL;
    bleOtaClCb.otaFuncPtr.cbStatus      = cbOtaStatus;
}

void BleOtaClient_cbOpenConnection(UInt8 linkId)
{
    bleOtaClCb.linkId = linkId;
}

void BleOtaClient_cbCloseConnection(UInt8 linkId)
{
    if(bleOtaClCb.linkId == linkId)
    {
        bleOtaClCb.linkId = UINT8_MAX;
        bleOtaClCb.otaFuncPtr.sendFunction = NULL;
        gpOta_Register(&bleOtaClCb.otaFuncPtr);
    }
}

void BleOtaClient_cbBondedConnection(UInt8 linkId)
{
    if(bleOtaClCb.linkId == linkId)
    {
        GP_LOG_PRINTF("OTAU Link created", 0);
        bleOtaClCb.otaFuncPtr.sendFunction = BlePeripheral_DataRequest;
        gpOta_Register(&bleOtaClCb.otaFuncPtr);
    }
}

Bool BleOtaClient_cbDataIndication(UInt8 linkId, UInt16 length, UInt8* pData)
{
#define OTA_PROFILE_ID      0xC3
#define OTA_CLUSTER_ID      0x0019

    UInt8 profileId =  0;
    UInt16 clusterId = 0;
    UInt8 dataOffset = 0;

    if(linkId != bleOtaClCb.linkId)
    {
        return false;
    }

    UInt8_buf2api(&profileId, pData, 1, &dataOffset);
    UInt16_buf2api(&clusterId, pData, 1, &dataOffset);

    if ( (profileId == OTA_PROFILE_ID) && (clusterId == OTA_CLUSTER_ID) )
    {
        gpOta_cbDataIndication(linkId, length, pData);
        return true;
    }

    return false;
}

