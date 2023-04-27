/*
 *   Copyright (c) 2019, Qorvo Inc
 *
 *   OTA Implementation
 *   Implementation of gpOta
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
//#define GP_LOCAL_LOG

#include "gpOta.h"
#include "gpLog.h"
#include "gpOtaDefs.h"

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

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
void Ota_PrepareHeaderFields(UInt8 cmdId, UInt8* pData)
{
    UInt8 profileId =  OTA_PROFILE_ID;
    UInt16 clusterId = OTA_CLUSTER_ID;
    UInt8 dataOffset = 0;

    UInt8_api2buf(pData, &profileId, 1, &dataOffset);
    UInt16_api2buf(pData, &clusterId, 1, &dataOffset);
    UInt8_api2buf(pData, &cmdId, 1, &dataOffset);
}

Bool Ota_CheckHeaderFields(UInt8* cmdId, UInt8* pData)
{
    UInt8 profileId =  0;
    UInt16 clusterId = 0;
    UInt8 commandId = 0xFF;
    UInt8 dataOffset = 0;
    Bool returnVal = false;

    UInt8_buf2api(&profileId, pData, 1, &dataOffset);
    UInt16_buf2api(&clusterId, pData, 1, &dataOffset);

    if ( (profileId == OTA_PROFILE_ID) && (clusterId == OTA_CLUSTER_ID) )
    {
        UInt8_buf2api(&commandId, pData, 1, &dataOffset);
        *cmdId = commandId;
        returnVal = true;
    }

    return returnVal;
}

void gpOta_cbDataIndication (UInt8 pairingRef, UInt16 dataLength, UInt8* pData)
{
    UInt8 cmdId = 0xFF;

    if ( ! Ota_CheckHeaderFields (&cmdId, pData) )
    {
        GP_LOG_SYSTEM_PRINTF("invalid OTA command received!",0);
    }

    switch (cmdId)
    {
#if defined(GP_OTA_DIVERSITY_CLIENT)
        case OTA_IMAGE_NOTIFY_CMD_ID:
            GP_LOG_PRINTF("OTA_IMAGE_NOTIFY_CMD_ID received",0);
            Ota_ImageNotifyIndication(pairingRef, pData + OTA_HEADER_FIELD_LENGTH);
            break;
        case OTA_QUERY_NEXT_IMAGE_RESPONSE_CMD_ID:
        {
            GP_LOG_PRINTF("OTA_QUERY_NEXT_IMAGE_RESPONSE_CMD_ID received",0);
            Ota_QueryNextImageResponseIndication(pairingRef, pData + OTA_HEADER_FIELD_LENGTH);
            break;
        }
        case OTA_IMAGE_BLOCK_RESPONSE_CMD_ID:
        {
            GP_LOG_PRINTF("OTA_IMAGE_BLOCK_RESPONSE_CMD_ID received",0);
            Ota_ImageBlockResponseIndication(pairingRef, pData + OTA_HEADER_FIELD_LENGTH);
            break;
        }
        case OTA_UPGRADE_END_RESPONSE_CMD_ID:
        {
            GP_LOG_PRINTF("OTA_UPGRADE_END_RESPONSE_CMD_ID received",0);
            Ota_UpgradeEndResponseIndication(pairingRef, pData + OTA_HEADER_FIELD_LENGTH);
            break;
        }
#endif //defined(GP_OTA_DIVERSITY_CLIENT)
        default:
        {
            GP_LOG_SYSTEM_PRINTF("invalid ota command identifier!",0);
        }
    }
}

#ifdef GP_DIVERSITY_LOG
const char *gpOta_OtaState2String(gpOta_OtaState_t val)
{
    switch (val)
    {
        case gpOta_OtaState_Idle:
            return "OtaState_Idle";

        case gpOta_OtaState_QueryImage:
            return "OtaState_QueryImage";

        case gpOta_OtaState_ImageAvailable:
            return "OtaState_ImageAvailable";

        case gpOta_OtaState_DownloadingHeader:
            return "OtaState_DownloadingHeader";

        case gpOta_OtaState_DownloadingSubElemTag:
            return "OtaState_DownloadingTag";

        case gpOta_OtaState_DownloadingSubElemData:
            return "OtaState_DownloadingImage";

        case gpOta_OtaState_UpgradeImage:
            return "OtaState_UpgradeImage";

        case gpOta_OtaState_UpgradeScheduled:
            return "OtaState_UpgradeScheduled";

        case gpOta_OtaState_DownloadComplete:
            return "OtaState_DownloadComplete";

        case gpOta_OtaState_SubProcComplete:
            return "OtaState_SubProcComplete";

        case gpOta_OtaState_ImageNotificationReceived:
            return "OtaState_ImageNotificationReceived";

        case gpOta_OtaState_VerifyingSubElemData:
            return "OtaState_VerifyingSubElemData";

        case gpOta_OtaState_VerifyingImage:
            return "OtaState_VerifyingImage";

        default:
            return "?";
    }
}

const char *gpOta_StatusParam2String(gpOta_StatusParam_t val)
{
    switch (val)
    {
        case gpOta_OtaState_ImageAvailableInvalidState:
            return "OtaState_ImageAvailableInvalidState";
            break;

        case gpOta_OtaState_ImageAvailableWrongPairId:
            return "OtaState_ImageAvailableWrongPairId";
            break;

        case gpOta_OtaState_ImageAvailableStatusNoSuccess:
            return "OtaState_ImageAvailableStatusNoSuccess";
            break;

        case gpOta_OtaState_ImageAvailableWrongManufatorCode:
            return "OtaState_ImageAvailableWrongManufatorCode";
            break;

        case gpOta_OtaState_ImageAvailableWrongImageType:
            return "OtaState_ImageAvailableWrongImageType";
            break;

        case gpOta_OtaState_ImageAvailableImageOutdated:
            return "OtaState_ImageAvailableImageOutdated";
            break;

        case gpOta_OtaState_FlashVerifyFailure:
            return "OtaState_FlashVerifyFailure";
            break;

        case gpOta_OtaState_DownloadingFileCrcFailure:
            return "OtaState_DownloadingFileCrcFailure";
            break;

        case gpOta_OtaState_DownloadedChunk:
            return "OtaState_DownloadedChunk";
            break;

        case gpOta_OtaState_DownloadFailed:
            return "OtaState_DownloadFailed";
            break;

        case gpOta_OtaState_RemoteAbort:
            return "OtaState_RemoteAbort";
            break;

        case gpOta_OtaState_ImageOffsetWrong:
            return "OtaState_ImageOffsetWrong";
            break;

        case gpOta_OtaState_ImageSizeTooLarge:
            return "OtaState_ImageSizeTooLarge";
            break;

        case gpOta_OtaState_DownloadHold:
            return "OtaState_DownloadHold";
            break;

        case gpOta_OtaState_DownloadResume:
            return "OtaState_DownloadResume";
            break;

        case gpOta_OtaState_DowloadHoldOverUnderflow:
            return "OtaState_DowloadHoldOverUnderflow";
            break;

        case gpOta_OtaState_DowloadHoldTimeout:
            return "OtaState_DowloadHoldTimeout";
            break;

        case gpOta_OtaState_Abort:
            return "OtaState_Abort";
            break;

        case gpOta_OtaState_NoDataRequestRegistered:
            return "OtaState_NoDataRequestRegistered";
            break;

        case gpOta_OtaState_NoRemoteResponse:
            return "OtaState_NoRemoteResponse";
            break;

        case gpOta_OtaState_ReachedEndOfFile:
            return "OtaState_ReachedEndOfFile";
            break;

        case gpOta_OtaState_UnknownSubElementTag:
            return "OtaState_UnknownSubElementTag";
            break;

        default:
            return "?";
            break;
    }
}

const char *gpOta_Status2String(gpOta_Status_t val)
{
    switch (val)
    {
        case gpOta_Status_Success:
            return "Status_Success";
            break;

        case gpOta_Status_Malformed_Command:
            return "Status_Malformed_Command";
            break;

        case gpOta_Status_Unsup_Cluster_Command:
            return "Status_Unsup_Cluster_Command";
            break;

        case gpOta_Status_Abort:
            return "Status_Abort";
            break;

        case gpOta_Status_Invalid_Image:
            return "Status_Invalid_Image";
            break;

        case gpOta_Status_Wait_For_Data:
            return "Status_Wait_For_Data";
            break;

        case gpOta_Status_No_Image_Available:
            return "Status_No_Image_Available";
            break;

        case gpOta_Status_Require_More_Image:
            return "Status_Require_More_Image";
            break;

        case gpOta_Status_NoOtaInProgress:
            return "Status_NoOtaInProgress";
            break;

        case gpOta_Status_LinkNotReady:
            return "Status_LinkNotReady";
            break;

        case gpOta_Status_Not_Authorized:
            return "Status_Not_Authorized";
            break;

        default:
            return "?";
            break;
    }
}
#endif

