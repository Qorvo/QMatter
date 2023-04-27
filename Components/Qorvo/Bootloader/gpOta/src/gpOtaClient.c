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
// #define GP_LOCAL_LOG

#include "gpOta.h"
#include "gpPoolMem.h"
#include "gpVersion.h"
#include "gpSched.h"
#include "gpLog.h"
#include "gpUpgrade.h"

#include "gpOtaDefs.h"
#include "gpUtils.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define OTA_HEADER_LENGTH_FIELD_INDEX               6
#define OTA_HEADER_LENGTH_FIELD_SIZE                2
#define OTA_SUBELEMENT_TAG_ID_INDEX                 0
#define OTA_SUBELEMENT_TAG_ID_SIZE                  2
#define OTA_SUBELEMENT_LENGTH_FIELD_INDEX           2
#define OTA_SUBELEMENT_LENGTH_FIELD_SIZE            4

/* OTA tag ids
   reuse Zigbee Cluster Library - 11.4.4 Tag Identifiers */
#define OTA_TAG_ID_MAIN_IMAGE                       0x0000
#define OTA_TAG_ID_SIGNATURE                        0x0001  /*< SHALL be the last sub-element in the file*/
#define OTA_TAG_ID_SIGNER_CERTIFICATE               0x0002  /*< information about the authority that generated the signature */
#define OTA_TAG_ID_IMAGE_INTEGRITY                  0x0003
#define OTA_TAG_ID_JUMPTABLE_IMAGE                  0xF000  /*< Manufacturer specific use 0xF000-0xFFFF*/


#define OTA_SUBELEMENT_HASH_CRC_PADDING_INDEX       0
#define OTA_SUBELEMENT_HASH_CRC_PADDING_SIZE        12
#define OTA_SUBELEMENT_HASH_CRC_VALUE_INDEX         12
#define OTA_SUBELEMENT_HASH_CRC_VALUE_SIZE          4
#define OTA_SUBELEMENT_SIGNATURE_IEEE_INDEX         0
#define OTA_SUBELEMENT_SIGNATURE_IEEE_SIZE          8
#define OTA_SUBELEMENT_SIGNATURE_DATA_INDEX         8

// Defines related to hold and resume
#define OTA_HOLD_MAX_DEPTH                          10
#define OTA_HOLD_MAX_TIMEOUT                        10000000UL      /* This timeout is set arbitrary, any reasonable value is oke */

// Defines to handle abort
#define OTA_TRIGGER_ABORT                           1
#define OTA_ABORT_CLEAR                             0

#define OTA_IMAGE_HASH_SIZE                         32

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/
typedef struct gpOta_OtaInfoEntry{
    //Current OTA state: idle, query image, image available, download header-tag-image, download compete, upgrade
    gpOta_OtaState_t otaState;
    //Vendor ID
    UInt16 manufCode;
    //Image type
    gpOta_ImageType_t imageType;
    //currenlty running image version
    UInt32 activeImageVersion;
    //application version that is available for download
    UInt32 newImageVersion;
    //active hardware version
    UInt32 activeHwVersion;
    //size of the full OTA file to download
    UInt32 fileSize;
    //size of the data after the tag + length field to download
    UInt32 subElementDataSize;
    //size of the actual application to upgrade (this value will be used by the bootloader for the copy action)
    UInt32 actualImageSize;
    //size of the OTA file header
    UInt16 otaFileHeaderSize;
    //offset in full OTA file
    UInt32 otaFileDataOffset;
    //offset in actual image block
    UInt32 subElemDataOffset;
    //Chunk length to download
    UInt8 downloadFragmentLength;
    //Delay between next block requests
    UInt16 blockRequestDelay;
    //holds crc calculated from the download chunks of this sub-element (will be compared against what is written in flash to observe flash corruption).
    UInt32 subElemDataCrcCalculated;
#if defined GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY
    //full ota file crc, read out from the downloaded file (will be compared against global otaFileCrcCalculated).
    UInt32 otaFileCrc;
#else
    //Context needed for building the hash of the downloaded image
    ota_hash_context* hash_ctx;
    //Context needed for signature validation
    ota_verify_context_t* verify_ctx;
    //Copy of the signature (part of the ota image)
    UInt8* signature;
    //Keep the hash of the ota image stored for the duration of the authentication process
    UInt8* ota_image_hash;
#endif
} gpOta_OtaInfoEntry_t;

typedef struct {
    UInt32 crcVal;
    UInt32 startAddr;
    UInt32 currentAddr;
    UInt32 totalSize;
} gpOta_CrcCtx_t;

static gpOta_OtaInfoEntry_t otaInfo;
//function pointer used for sending the data (RF4CE/BLE)
static gpOta_DataRequest_t Ota_DataRequest;
//function pointer used for reporting download status back to application
static gpOta_cbStatusReport_t Ota_cbStatus;
//Used as linkId for BLE or bindingId for RF4CE
static UInt8 pairingId = 0xFF;
//To indicate flash writes are started or not (trigger flash erase at the early start)
static Bool Ota_WriteStarted = false;
//Keep track of currentTagId, to know which part of the OTA file we are currenltly downloading
static UInt16 currentTagId = 0xFFFF;
#if defined GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY
//Used to calculate the CRC of the full OTA file that comes in, calculated as described in section 6.3.10.1 of the ZigBee OTA specifications.
static UInt32 otaFileCrcCalculated = 0;
#endif
// Used to hold the OtaDownload. Needed to allow e.g. a voice session or a key-press
static UInt8 OtaHold = 0;
// Used to flag an abort should be stated
static Bool Ota_TriggerAbort = OTA_ABORT_CLEAR;
// Used to flag if Ota Area is ready to be written
static Bool Ota_OtaAreaWiped = false;
// Used to keep context of entire flash CRC verification
static gpOta_CrcCtx_t flashIntegrityCrcCtx;

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/
// Helper functions for download flow
static void Ota_InitOtaSession(void);
static void Ota_StartUpgrade(void);
static void Ota_ImageDownload(void);
static void Ota_PrepareDownloadingSubelementData();
static void Ota_PrepareDownloadingSubelementTag();

// Functions to create the payload for the send function.
static void Ota_QueryNextImageRequest(void);
static void Ota_ImageBlockRequest(void);
static void Ota_UpgradeEndRequest(void *pOtaStatus);

// Validation functions
static void Ota_FlashIntegrityValidation_Start(void);
static void Ota_FlashIntegrityValidation_Step(void);
static void Ota_FlashIntegrityValidation_Done(void);
#if defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
static void Ota_OtaImageIntegrityValidation(void);
#else
static void Ota_OtaImageAuthenticityValidation_Start(void);
static void Ota_OtaImageAuthenticityValidation_Cleanup(void);
#endif

//Helper functions for scheduled actions
static void Ota_HoldTerminate(void);
static void Ota_cbHoldScheduled(void);
static void Ota_cbResumeScheduled(void);
static void Ota_HoldMaxTimeout(void);
static void Ota_RemoteResponseTimeout( void );
static void Ota_cbOtaAreaErased(void);
static void Ota_scheduleUpgradeEndRequest(gpOta_Status_t status);

//gpUpgrade wrappers
static void Ota_ChunkWrite(UInt32 flashAddress, UInt16 length, UInt8* payload);
static Bool Ota_SetPendingImage(UInt32 swVersion, UInt32 hwVersion, UInt32 imageSize);

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/
/*
    Resetting OTA state and init global variables
*/
static void Ota_InitOtaSession(void)
{
    MEMSET(&otaInfo, 0, sizeof(otaInfo));

    otaInfo.otaState                          = gpOta_OtaState_Idle;
    otaInfo.blockRequestDelay                 = (UInt16) GP_OTA_MINIMUMBLOCKPERIOD_DEFAULT;
    otaInfo.otaFileHeaderSize                 = OTA_MIN_OTA_FILE_HEADER_LENGTH;

    /* There can be a pending timeout, unschedule the time out */
    gpSched_UnscheduleEvent(Ota_HoldMaxTimeout);
    /* There can be a CRC calculation in progress, abort them */
    gpSched_UnscheduleEvent(Ota_FlashIntegrityValidation_Step);
    OtaHold = 0;
    Ota_TriggerAbort = OTA_ABORT_CLEAR;
    Ota_WriteStarted = false;
    currentTagId = 0xFFFF;
    pairingId = 0xFF;
    Ota_DataRequest = NULL;
    Ota_cbStatus = NULL;
#if defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
    otaFileCrcCalculated = 0;
    otaFileCrcCalculated ^= GP_UTILS_CRC32_FINAL_XOR_VALUE;
#else
    otaInfo.signature = NULL;
    otaInfo.ota_image_hash = NULL;
    otaInfo.hash_ctx = NULL;
    otaInfo.verify_ctx = NULL;
#endif
}

/*
    Functions to calculate CRC of images stored in flash. Output of this function will be compared with CRC
    calculated on the fly of received OTA chunks of that image. (Needed to detect flash corruption)
*/
static void Ota_FlashIntegrityValidation_Start(void)
{
    flashIntegrityCrcCtx.crcVal = 0;
    flashIntegrityCrcCtx.startAddr = 0;
    flashIntegrityCrcCtx.totalSize = 0;

    flashIntegrityCrcCtx.crcVal ^= GP_UTILS_CRC32_FINAL_XOR_VALUE;
    if (currentTagId == OTA_TAG_ID_JUMPTABLE_IMAGE)
    {
#if  defined(GP_DIVERSITY_GPHAL_K8E)
        //Verification of updated jumptables written in flash.
#if defined(GP_UPGRADE_DIVERSITY_COMPRESSION)
        flashIntegrityCrcCtx.startAddr = gpUpgrade_GetOtaAreaStartAddress() - GP_UPGRADE_APP_JUMP_TABLE_SIZE;
#else
        flashIntegrityCrcCtx.startAddr = GP_UPGRADE_APP_JUMP_TABLE_ADDR(gpUpgrade_GetOtaAreaStartAddress());
#endif
#endif //GP_DIVERSITY_GPHAL_K8C  || GP_DIVERSITY_GPHAL_K8D || GP_DIVERSITY_GPHAL_K8E
    }
    else if (currentTagId == OTA_TAG_ID_MAIN_IMAGE)
    {
        //Verification of updated application written in flash.
        flashIntegrityCrcCtx.startAddr = gpUpgrade_GetOtaAreaStartAddress();
    }
    flashIntegrityCrcCtx.currentAddr = flashIntegrityCrcCtx.startAddr;
    flashIntegrityCrcCtx.totalSize = otaInfo.subElementDataSize;
    gpSched_ScheduleEvent(0, Ota_FlashIntegrityValidation_Step);
}

static void Ota_FlashIntegrityValidation_Step(void)
{
    UInt8 imgData[GP_OTA_MAXIMAGEFRAGMENTLENGTH];
    UInt32 sz = flashIntegrityCrcCtx.currentAddr - flashIntegrityCrcCtx.startAddr;
    UInt16 imgBlockLen = min(GP_OTA_MAXIMAGEFRAGMENTLENGTH, (flashIntegrityCrcCtx.totalSize - sz));
    if (gpUpgrade_ReadChunk(flashIntegrityCrcCtx.currentAddr, imgBlockLen, imgData) == gpHal_FlashError_Success )
    {
        gpUtils_CalculatePartialCrc32(&flashIntegrityCrcCtx.crcVal, imgData, imgBlockLen);
    }
    flashIntegrityCrcCtx.currentAddr += imgBlockLen;
    if((flashIntegrityCrcCtx.currentAddr - flashIntegrityCrcCtx.startAddr) < flashIntegrityCrcCtx.totalSize)
    {
        gpSched_ScheduleEvent(0, Ota_FlashIntegrityValidation_Step);
    }
    else
    {
        flashIntegrityCrcCtx.crcVal ^= GP_UTILS_CRC32_FINAL_XOR_VALUE;
        Ota_FlashIntegrityValidation_Done();
    }
}

static void Ota_FlashIntegrityValidation_Done(void)
{
    if ( flashIntegrityCrcCtx.crcVal != otaInfo.subElemDataCrcCalculated )
    {
        GP_LOG_SYSTEM_PRINTF("flash corruption detected, mismatch between downloaded image and image in flash!", 0);
        if(Ota_cbStatus != NULL)
        {
            gpOta_StatusParam_t status = gpOta_OtaState_FlashVerifyFailure;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)&status);
        }
        Ota_scheduleUpgradeEndRequest(gpOta_Status_Abort);
        return;
    }
    // Return to the download state machine
    gpSched_ScheduleEvent(0, Ota_ImageDownload);
}


#if defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
static void Ota_OtaImageIntegrityValidation(void)
{
    // check CRC for errors on download, if CRC is part of the image.
    otaFileCrcCalculated ^= GP_UTILS_CRC32_FINAL_XOR_VALUE;
    GP_LOG_PRINTF("OTA File CRC: %lu, Calculated CRC: %lu", 0, otaInfo.otaFileCrc, otaFileCrcCalculated);
    if ((otaInfo.otaFileCrc != 0) && (otaFileCrcCalculated != otaInfo.otaFileCrc))
    {
        GP_LOG_PRINTF("CRC failure, mismatch between calculated CRC of OTA file download and CRC mentioned in the file!",0);
        if(Ota_cbStatus != NULL)
        {
            gpOta_StatusParam_t status = gpOta_OtaState_DownloadingFileCrcFailure;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete, (void*)&status);
        }
        Ota_scheduleUpgradeEndRequest(gpOta_Status_Abort);
        return;
    }
    // Return to the download state machine
    gpSched_ScheduleEvent(0, Ota_ImageDownload);
}
#endif //GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY

#if !defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
static void Ota_OtaImageAuthenticityValidation_Start(void)
{
    gpOta_StatusParam_t status;

    if (OtaHold)
    {
        /* Poll each 100ms until:
           - OtaHold is cleared
           - OtaHold times out
           - OTA session is aborted.
        */
        gpSched_ScheduleEvent(100000, Ota_OtaImageAuthenticityValidation_Start);
        return;
    }

    // Handle only for the first call to Ota_OtaImageAuthenticityValidation_Start()
    if (otaInfo.ota_image_hash == NULL)
    {
        // Finalize hash
        otaInfo.ota_image_hash = GP_POOLMEM_MALLOC(OTA_IMAGE_HASH_SIZE);
        if (otaInfo.ota_image_hash == NULL)
        {
            GP_LOG_PRINTF("[OTA] Failed to allocate ota_image_hash", 0);
            if (Ota_cbStatus != NULL)
            {
                status = gpOta_OtaState_DownloadingFileCrcFailure;
                Ota_cbStatus(gpOta_OtaState_SubProcComplete, (void*)&status);
            }
            Ota_scheduleUpgradeEndRequest(gpOta_Status_Abort);
            return;
        }

        MEMSET(otaInfo.ota_image_hash, 0, OTA_IMAGE_HASH_SIZE);
        if (Ota_CalculateHash_Finish(otaInfo.hash_ctx, otaInfo.ota_image_hash, OTA_IMAGE_HASH_SIZE) == false)
        {
            GP_LOG_PRINTF("Authentication failure, hashing finalization failed", 0);
            if(Ota_cbStatus != NULL)
            {
                status = gpOta_OtaState_DownloadingFileCrcFailure;
                Ota_cbStatus(gpOta_OtaState_SubProcComplete, (void*)&status);
            }
            gpPoolMem_Free(otaInfo.ota_image_hash);
            otaInfo.ota_image_hash = NULL;
            Ota_scheduleUpgradeEndRequest(gpOta_Status_Abort);
            return;
        }

        otaInfo.verify_ctx = GP_POOLMEM_MALLOC(sizeof(ota_verify_context_t));
        if (otaInfo.verify_ctx == NULL)
        {
            GP_LOG_PRINTF("[OTA] Failed to allocate the \"verify context\"", 0);
            if (Ota_cbStatus != NULL)
            {
                status = gpOta_OtaState_DownloadingFileCrcFailure;
                Ota_cbStatus(gpOta_OtaState_SubProcComplete, (void*)&status);
            }
            gpPoolMem_Free(otaInfo.ota_image_hash);
            otaInfo.ota_image_hash = NULL;
            Ota_scheduleUpgradeEndRequest(gpOta_Status_Abort);
            return;
        }
        Ota_AuthenticateImage_Init(otaInfo.verify_ctx);
    }

    // Handle for each call to Ota_OtaImageAuthenticityValidation_Start()
    Ota_Auth_Status_t result = Ota_AuthenticateImage(otaInfo.verify_ctx, otaInfo.ota_image_hash,
                                                             OTA_IMAGE_HASH_SIZE, otaInfo.signature);
    if (result == Ota_Auth_Status_Success)
    {
        // Return to the download state machine to indicate successful authentication
        gpSched_ScheduleEvent(0, Ota_ImageDownload);
    }
    else if (result == Ota_Auth_Status_In_Progress)
    {
        /* Continue validation until success or fail
         * Keep the scheduler available for at least 50% of the time for handling other tasks.
         * Delay the next authentication cycle with the duration of 1 cycle
         * (50ms @32MHz, see Ota_AuthenticateImage_Init())
         * =NOTE=
         * Audio processing can only buffer for GP_AUDIO_DIVERSITY_NUM_DEC_BUF_PER_CHANNEL x 2.5ms,
         * which will be smaller than 50ms, so instead of co-existing, audio should "Hold" the OTA task
         * while processing and "Resume" when finished.
         */
        gpSched_ScheduleEvent(50000, Ota_OtaImageAuthenticityValidation_Start);

        // Do not cleanup
        return;
    }
    else
    {
        GP_LOG_PRINTF("Authentication failed to start!", 0);
        if(Ota_cbStatus != NULL)
        {
            gpOta_StatusParam_t status = gpOta_OtaState_DownloadingFileSignatureFailure;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete, (void*)&status);
        }

        Ota_scheduleUpgradeEndRequest(gpOta_Status_Abort);
    }

    Ota_OtaImageAuthenticityValidation_Cleanup();
}

static void Ota_OtaImageAuthenticityValidation_Cleanup(void)
{
    gpPoolMem_Free(otaInfo.ota_image_hash);
    otaInfo.ota_image_hash = NULL;
    Ota_AuthenticateImage_Free(otaInfo.verify_ctx);
    gpPoolMem_Free(otaInfo.verify_ctx);
    otaInfo.verify_ctx = NULL;
}
#endif // !defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)

/*
    Function to write OTA chunks to flash
*/
void Ota_ChunkWrite(UInt32 flashaddress, UInt16 length, UInt8* payload)
{
    gpUpgrade_Status_t status = gpUpgrade_StatusSuccess;

    if(!Ota_WriteStarted)
    {
        if(Ota_OtaAreaWiped)
        {
            Ota_WriteStarted = true;
            gpUpgrade_StartWrite();
            Ota_OtaAreaWiped = false;
        }
        else
        {
            GP_LOG_PRINTF("OTA area is not ready",0);
            status = gpUpgrade_StatusWriteError;
        }
    }

    if(status == gpUpgrade_StatusSuccess)
    {
        status = gpUpgrade_WriteChunk(flashaddress, length, payload);
    }
    if(status != gpUpgrade_StatusSuccess)
    {
        GP_LOG_PRINTF("OTA write failed %x 0x%lx l:%u",0,status, flashaddress, length);
        //stop OTA session, we are not able to write in flash!
        if(Ota_cbStatus != NULL)
        {
            gpOta_StatusParam_t status = gpOta_OtaState_DownloadFailed;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)&status);
        }
        Ota_scheduleUpgradeEndRequest(gpOta_Status_Abort);
    }
    else
    {
        if(Ota_cbStatus != NULL)
        {
            gpOta_StatusParam_t status = gpOta_OtaState_DownloadedChunk;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)&status);
        }
    }
}

/*
    Function to indicate that a new image is pending. User Mode Bootloader will take care of the upgrade.
*/
Bool Ota_SetPendingImage(UInt32 swVersion, UInt32 hwVersion, UInt32 imageSize)
{
    UInt32 otaAreaStartAddr = 0;
    gpUpgrade_Status_t status = gpUpgrade_StatusSuccess;
    UInt8 scheduleUpgrade = 0;

    otaAreaStartAddr = gpUpgrade_GetOtaAreaStartAddress();
    status = gpUpgrade_SetPendingImage(swVersion, hwVersion, otaAreaStartAddr, imageSize);

    if(status == gpUpgrade_StatusSuccess)
    {
        scheduleUpgrade = 1;
        gpSched_ScheduleEvent(1000*100, gpUpgrade_Reset);
    }
    else
    {
        scheduleUpgrade = 0;
        GP_LOG_PRINTF("Failed to set pending image %x",0,status);
    }
    if(Ota_cbStatus != NULL)
    {
        Ota_cbStatus(gpOta_OtaState_UpgradeImage,(void*)&scheduleUpgrade);
    }
    return (scheduleUpgrade == 1);
}

/*
    Trigger to start the upgrade process.
*/
static void Ota_StartUpgrade(void)
{
    if (false == Ota_SetPendingImage(otaInfo.newImageVersion, otaInfo.activeHwVersion, otaInfo.actualImageSize))
    {
        Ota_scheduleUpgradeEndRequest(gpOta_Status_Abort);
    }
    else
    {
        Ota_InitOtaSession();
    }
}

static void Ota_PrepareDownloadingSubelementTag()
{
    if(Ota_cbStatus != NULL)
    {
        Ota_cbStatus(gpOta_OtaState_DownloadingSubElemTag,NULL);
    }
    otaInfo.otaState = gpOta_OtaState_DownloadingSubElemTag;
    // Set the framelength to only download the subelement header
    otaInfo.downloadFragmentLength = OTA_SUBELEMENT_TAG_ID_SIZE + OTA_SUBELEMENT_LENGTH_FIELD_SIZE;
    // Keep track of how many data bytes (excl. header) of this sub-element have been downloaded.
    otaInfo.subElemDataOffset = 0;
    // Reset the size of this sub-element, will be provided in the "header" on BLOCK_RESPONSE
    otaInfo.subElementDataSize = 0;
}

static void Ota_PrepareDownloadingSubelementData()
{
    // Restart crc calculations of this sub-element.
    otaInfo.subElemDataCrcCalculated = 0;
    otaInfo.subElemDataCrcCalculated ^= GP_UTILS_CRC32_FINAL_XOR_VALUE;
    if(Ota_cbStatus != NULL)
    {
        Ota_cbStatus(gpOta_OtaState_DownloadingSubElemData,NULL);
    }
    // Prepare to download the first fragment of the sub-element data
    otaInfo.otaState = gpOta_OtaState_DownloadingSubElemData;
}

/*
    Image download control manager
*/
static void Ota_ImageDownload(void)
{
    GP_LOG_PRINTF("Ota_ImageDownload(state:0x%X)", 0, otaInfo.otaState);

    /* Check if AbortFlag is set. This is need to finalize pending transaction. */
    if(Ota_TriggerAbort == OTA_TRIGGER_ABORT)
    {
        Ota_scheduleUpgradeEndRequest(gpOta_Status_Abort);
        return;
    }

    if(OtaHold != 0)
    {
        /* Download is hold, stop downloading next frame */
        return;
    }

    switch (otaInfo.otaState)
    {
        case gpOta_OtaState_ImageAvailable:
        {
            if (Ota_OtaAreaWiped == false)
            {
                //Erase Ota Area
                gpUpgrade_StartEraseOtaArea(Ota_cbOtaAreaErased);
                // NOT DOWNLOADING A BLOCK, let the callback trigger Ota_ImageDownload
                return;
            }
            else
            {
                // Fall through to gpOta_OtaState_DownloadingHeader
            }
            // No break
        }
        case gpOta_OtaState_DownloadingHeader:
        {
            // Handle state transition
            if (otaInfo.otaState == gpOta_OtaState_ImageAvailable)
            {
                /* Move to next state, downloading the header of the OTA file*/
                otaInfo.otaState = gpOta_OtaState_DownloadingHeader;
                // Reset the offset to indicate we start downloading a new OTA file
                otaInfo.otaFileDataOffset = 0;
            }

            if(otaInfo.otaFileHeaderSize == 0)
            {
                otaInfo.downloadFragmentLength = OTA_MIN_OTA_FILE_HEADER_LENGTH;
                // Download first part of OTA header to set "otaInfo.otaFileHeaderSize"
                break;
            }
            else if(otaInfo.otaFileDataOffset < otaInfo.otaFileHeaderSize)
            {
                otaInfo.downloadFragmentLength = otaInfo.otaFileHeaderSize - otaInfo.otaFileDataOffset;
                // Download remaining OTA header
                break;
            }
            else
            {
                // Ota header received
                // Fall through to gpOta_OtaState_DownloadingSubElemTag
            }
            // No break
        }
        case gpOta_OtaState_DownloadingSubElemTag:
        {
            // Handle state transition
            if (otaInfo.otaState == gpOta_OtaState_DownloadingHeader)
            {
                Ota_PrepareDownloadingSubelementTag();
                // Download subelement header
                break;
            }
            else
            {
                // Subelement header downloaded
                // Fall through to gpOta_OtaState_DownloadingSubElemData
            }
            // No break
        }
            // Fall through to set downloadFragmentLength
        case gpOta_OtaState_DownloadingSubElemData:
        {
            // Handle state transition
            if (otaInfo.otaState == gpOta_OtaState_DownloadingSubElemTag)
            {
                Ota_PrepareDownloadingSubelementData();
            }

            if((otaInfo.subElemDataOffset + GP_OTA_MAXIMAGEFRAGMENTLENGTH) < otaInfo.subElementDataSize)
            {
                otaInfo.downloadFragmentLength = GP_OTA_MAXIMAGEFRAGMENTLENGTH;
                // Download fragment at maximum fragment size
                break;
            }
            else if(otaInfo.subElemDataOffset < otaInfo.subElementDataSize)
            {
                otaInfo.downloadFragmentLength = otaInfo.subElementDataSize - otaInfo.subElemDataOffset;
                // Download last fragment of the sub-element, only get remaining bytes
                break;
            }
            else
            {
                // All subelement data has been downloaded - check flash corruption errors.
                // Fall through to gpOta_OtaState_VerifyingSubElemData
            }
            // No break
        }
        case gpOta_OtaState_VerifyingSubElemData:
        {
            // Handle state transition
            if (otaInfo.otaState == gpOta_OtaState_DownloadingSubElemData)
            {
                otaInfo.otaState = gpOta_OtaState_VerifyingSubElemData;
                otaInfo.subElemDataCrcCalculated ^= GP_UTILS_CRC32_FINAL_XOR_VALUE;
                Ota_FlashIntegrityValidation_Start();
                // NOT DOWNLOADING A BLOCK, let the callback trigger Ota_ImageDownload
                return;
            }

            // Handle callback from Ota_FlashIntegrityValidation_Done
            if (otaInfo.otaFileDataOffset < otaInfo.fileSize)
            {
                // Only received part of the ota file

                Ota_PrepareDownloadingSubelementTag();
                // Download next subelement header
                break;
            }
            else
            {
                // Full ota file received - Verify total image
                // Fall through to gpOta_OtaState_VerifyingImage
            }
            // No break
        }
        case gpOta_OtaState_VerifyingImage:
        {
            // Handle state transition
            if (otaInfo.otaState == gpOta_OtaState_VerifyingSubElemData)
            {
                otaInfo.otaState = gpOta_OtaState_VerifyingImage;
                if(Ota_cbStatus != NULL)
                {
                    Ota_cbStatus(gpOta_OtaState_VerifyingImage, NULL);
                }
#if defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
                Ota_OtaImageIntegrityValidation();
#else
                Ota_OtaImageAuthenticityValidation_Start();
#endif
                // NOT DOWNLOADING A BLOCK, let the callers abover re-trigger Ota_ImageDownload
                return;
            }
            else
            {
#if !defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
                if (gpSched_ExistsEvent(Ota_OtaImageAuthenticityValidation_Start))
                {
                    // Handle resuming OTA while verifying authenticity
                    // Halt download state machine here until verification has finished
                    return;
                }
#endif
                // OTA file verified - Indicate download completed
                // Fall through to gpOta_OtaState_DownloadComplete
            }
            // No break
        }
        case gpOta_OtaState_DownloadComplete:
        {
            // Handle state transition
            if (otaInfo.otaState == gpOta_OtaState_VerifyingImage)
            {
                otaInfo.otaState = gpOta_OtaState_DownloadComplete;
            }

            Ota_scheduleUpgradeEndRequest(gpOta_Status_Success);
            // NOT DOWNLOADING A BLOCK, end of state machine
            return;
        }
        default:
        {
            GP_LOG_SYSTEM_PRINTF("invalid state!",0);
            break;
        }
    }

    Ota_ImageBlockRequest();
}

// Send functions

static void Ota_QueryNextImageRequest(void)
{
    UInt8 pData[OTA_HEADER_FIELD_LENGTH+OTA_QUERY_NEXT_IMAGE_REQUEST_LENGTH];
    UInt8 dataOffset = OTA_HEADER_FIELD_LENGTH;
    UInt8 fieldControl = 0;
    UInt8 commandPayloadLength = OTA_HEADER_FIELD_LENGTH+OTA_QUERY_NEXT_IMAGE_REQUEST_LENGTH;

    Ota_PrepareHeaderFields(OTA_QUERY_NEXT_IMAGE_REQUEST_CMD_ID, pData);

    if ( otaInfo.activeHwVersion != 0xFFFF )
    {
        BIT_SET(fieldControl, OTA_NEXT_IMAGE_FIELD_CONTROL_HARDWARE_VERSION_PRESENT_BIT);
    }
    else
    {
        BIT_CLR(fieldControl, OTA_NEXT_IMAGE_FIELD_CONTROL_HARDWARE_VERSION_PRESENT_BIT);
        commandPayloadLength -= OTA_QUERY_NEXT_IMAGE_REQUEST_EXT_HW_LENGTH;
    }

    UInt8_api2buf(pData, &fieldControl, 1, &dataOffset);
    UInt16_api2buf(pData, &otaInfo.manufCode, 1, &dataOffset);
    UInt16_api2buf(pData, &otaInfo.imageType, 1, &dataOffset);
    UInt32_api2buf(pData, &otaInfo.activeImageVersion, 1, &dataOffset);

    if (BIT_TST(fieldControl, OTA_NEXT_IMAGE_FIELD_CONTROL_HARDWARE_VERSION_PRESENT_BIT))
    {
        UInt16_api2buf(pData, &otaInfo.activeHwVersion, 1, &dataOffset);
    }

    if(Ota_cbStatus != NULL)
    {
        Ota_cbStatus(gpOta_OtaState_QueryImage,NULL);
    }

    if(Ota_DataRequest != NULL)
    {
        gpSched_ScheduleEvent(GP_OTA_REMOTE_RESPONSE_TIMEOUT_IN_SEC * 1000 * 1000, Ota_RemoteResponseTimeout);
        Ota_DataRequest(pairingId, commandPayloadLength, pData);
    }
    else
    {
        GP_LOG_PRINTF("No data request function registered at line %u",0,__LINE__);
        if(Ota_cbStatus != NULL)
        {
            UInt8 statusParam = gpOta_OtaState_NoDataRequestRegistered;

            Ota_cbStatus(gpOta_OtaState_SubProcComplete, (void*)&statusParam);
        }
    }
}

static void Ota_ImageBlockRequest(void)
{
    UInt8 pData[OTA_HEADER_FIELD_LENGTH+OTA_IMAGE_BLOCK_REQUEST_LENGTH];
    UInt8 dataOffset = OTA_HEADER_FIELD_LENGTH;
    UInt8 fieldControl = 0;
    BIT_CLR(fieldControl, OTA_BLOCK_REQUEST_FIELD_CONTROL_IEEE_ADDR_PRESENT_BIT);
    BIT_CLR(fieldControl, OTA_BLOCK_REQUEST_FIELD_CONTROL_DELAY_PRESENT_BIT);

    Ota_PrepareHeaderFields(OTA_IMAGE_BLOCK_REQUEST_CMD_ID, pData);

    UInt8_api2buf(pData, &fieldControl, 1, &dataOffset);
    UInt16_api2buf(pData, &otaInfo.manufCode, 1, &dataOffset);
    UInt16_api2buf(pData, &otaInfo.imageType, 1, &dataOffset);
    UInt32_api2buf(pData, &otaInfo.newImageVersion, 1, &dataOffset);
    UInt32_api2buf(pData, &otaInfo.otaFileDataOffset, 1, &dataOffset);
    UInt8_api2buf(pData, &otaInfo.downloadFragmentLength, 1, &dataOffset);

    GP_LOG_PRINTF("BLOCK_REQUEST %lu/%lu, datalength=%u",
                  0,
                  otaInfo.otaState == gpOta_OtaState_DownloadingHeader ? otaInfo.otaFileDataOffset : otaInfo.subElemDataOffset,
                  otaInfo.otaState == gpOta_OtaState_DownloadingHeader ? otaInfo.otaFileHeaderSize : otaInfo.subElementDataSize,
                  otaInfo.downloadFragmentLength);

    if(Ota_DataRequest != NULL)
    {
        gpSched_ScheduleEvent(GP_OTA_REMOTE_RESPONSE_TIMEOUT_IN_SEC * 1000 * 1000, Ota_RemoteResponseTimeout);
        Ota_DataRequest(pairingId, OTA_HEADER_FIELD_LENGTH+OTA_IMAGE_BLOCK_REQUEST_LENGTH, pData);
    }
    else
    {
        GP_LOG_PRINTF("No data request function registered at line %u", 0, __LINE__);
        if(Ota_cbStatus != NULL)
        {
            UInt8 statusParam = gpOta_OtaState_NoDataRequestRegistered;

            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)&statusParam);
        }
    }
}

static void Ota_UpgradeEndRequest(void *pStatus)
{
    UInt8 pData[OTA_HEADER_FIELD_LENGTH+OTA_UPGRADE_END_REQUEST_LENGTH];
    UInt8 dataOffset = OTA_HEADER_FIELD_LENGTH;

    GP_ASSERT_DEV_INT(pStatus != NULL);
    gpOta_Status_t status = *((gpOta_Status_t*)pStatus);
    gpPoolMem_Free(pStatus);
#if !defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
    Ota_CalculateHash_Free(otaInfo.hash_ctx);
    gpPoolMem_Free(otaInfo.hash_ctx);
    otaInfo.hash_ctx = NULL;
    if (otaInfo.signature)
    {
        // May not yet be malloc'ed, check before freeing
        gpPoolMem_Free(otaInfo.signature);
        otaInfo.signature = NULL;
    }
#endif

    Ota_PrepareHeaderFields(OTA_UPGRADE_END_REQUEST_CMD_ID, pData);

    UInt8_api2buf(pData, &status, 1, &dataOffset);
    UInt16_api2buf(pData, &otaInfo.manufCode, 1, &dataOffset);
    UInt16_api2buf(pData, &otaInfo.imageType, 1, &dataOffset);
    UInt32_api2buf(pData, &otaInfo.newImageVersion, 1, &dataOffset);

    // Validate parameter (ZigBee Over-the-Air Upgrading Cluster rev23, 6.10.9.2.1)
    GP_ASSERT_DEV_INT(
                (status == gpOta_Status_Success) ||\
                (status == gpOta_Status_Invalid_Image) ||\
                (status == gpOta_Status_Require_More_Image) ||\
                (status == gpOta_Status_Abort));

    if (Ota_DataRequest != NULL)
    {
        gpSched_ScheduleEvent(GP_OTA_REMOTE_RESPONSE_TIMEOUT_IN_SEC * 1000 * 1000, Ota_RemoteResponseTimeout);
        Ota_DataRequest(pairingId, OTA_HEADER_FIELD_LENGTH+OTA_UPGRADE_END_REQUEST_LENGTH, pData);
    }
    else
    {
        GP_LOG_PRINTF("No data request function registered at line %u",0,__LINE__);
        if (Ota_cbStatus != NULL)
        {
            UInt8 statusParam = gpOta_OtaState_NoDataRequestRegistered;

            Ota_cbStatus(gpOta_OtaState_SubProcComplete, (void*)&statusParam);
        }
    }
    if (Ota_cbStatus != NULL)
    {
        if (status == gpOta_Status_Success)
        {
            Ota_cbStatus(gpOta_OtaState_DownloadComplete,NULL);
        }
        else
        {
            UInt8 statusParam = gpOta_OtaState_Abort;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)&statusParam);
        }
    }
    if (status != gpOta_Status_Success)
    {
        Ota_InitOtaSession();
    }
}

// receive functions
void Ota_ImageNotifyIndication(UInt8 pairingRef, UInt8* pData)
{
    UInt8 dataOffset =0;

    UInt8 queryJitter = 0;
    gpOta_ImageNotifyType_t payloadType;
    gpOta_ImageNotification_t imgData = {
        .pairingRef = pairingRef,
        .manufCode = GP_OTA_MANUFACTURER_CODE_QORVO,
        .imageType = gpOta_ImageTypeUnknown,
        .fileVersion = UINT32_MAX //Wildcard
        };
    UInt8_buf2api(&payloadType, pData, 1, &dataOffset);
    UInt8_buf2api(&queryJitter, pData, 1, &dataOffset);

    if (payloadType > gpOta_ImgNotifyQueryJitterOnly)
    {
        UInt16_buf2api(&imgData.manufCode, pData, 1, &dataOffset);
        UInt16_buf2api(&imgData.imageType, pData, 1, &dataOffset);
        UInt32_buf2api(&imgData.fileVersion, pData, 1, &dataOffset);
    }
    if (Ota_cbStatus != NULL)
    {
        Ota_cbStatus(gpOta_OtaState_ImageNotificationReceived, (void*)&imgData);
    }
}

void Ota_QueryNextImageResponseIndication(UInt8 pairingRef, UInt8* pData)
{
    UInt8 status = gpOta_Status_Success;
    UInt16 manufCode = 0;
    UInt16 imageType = gpOta_ImageTypeMain;
    UInt8 dataOffset = 0;
    gpOta_StatusParam_t statusParam[2];

    if (otaInfo.otaState != gpOta_OtaState_QueryImage)
    {
        GP_LOG_PRINTF("Query Next Image Response received in invalid state, ignore!", 0);
        if (Ota_cbStatus != NULL)
        {
            statusParam[0] = gpOta_OtaState_ImageAvailableInvalidState;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
        }
        return;
    }

    if (pairingRef != pairingId)
    {
        GP_LOG_PRINTF("Command received from wrong pairing ID, ignore!", 0);
        if(Ota_cbStatus != NULL)
        {
            statusParam[0] = gpOta_OtaState_ImageAvailableWrongPairId;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
        }
        return;
    }

    /* Unschedule response timeout only when response is coming from expeted paringRef */
    gpSched_UnscheduleEvent(Ota_RemoteResponseTimeout);

    UInt8_buf2api(&status, pData, 1, &dataOffset);

    if (status != gpOta_Status_Success)
    {
        GP_LOG_PRINTF("Query Next Image Response received with status 0x%02X, stop OTA session", 1, status);
        if (Ota_cbStatus != NULL)
        {
            statusParam[0] = gpOta_OtaState_ImageAvailableStatusNoSuccess;
            statusParam[1] = status;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
        }
        Ota_InitOtaSession();
        return;
    }

    UInt16_buf2api(&manufCode, pData, 1, &dataOffset);

    if (manufCode != otaInfo.manufCode)
    {
        GP_LOG_PRINTF("Query Next Image Response received with mismatching manufacturer code (0x%04X <-> 0x%04X), stop OTA session", 2, manufCode, otaInfo.manufCode);
        //stop OTA session
        if (Ota_cbStatus != NULL)
        {
            statusParam[0] = gpOta_OtaState_ImageAvailableWrongManufatorCode;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
        }
        Ota_InitOtaSession();
        return;
    }

    UInt16_buf2api(&imageType, pData, 1, &dataOffset);

    if (imageType != otaInfo.imageType)
    {
        GP_LOG_SYSTEM_PRINTF("Query Next Image Response received with mismatching image type (0x%02X <-> 0x%02X)", 2, imageType, otaInfo.imageType);
        if (Ota_cbStatus != NULL)
        {
            statusParam[0] = gpOta_OtaState_ImageAvailableWrongImageType;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
        }
        Ota_InitOtaSession();
        return;
    }

    UInt32_buf2api(&otaInfo.newImageVersion, pData, 1, &dataOffset);

    if (otaInfo.newImageVersion <= otaInfo.activeImageVersion)
    {
        //Only upgrade to newer version is allowed!
        GP_LOG_PRINTF("Query Next Image Response received with equal or lower file version (0x%lx <= 0x%lx)", 1, otaInfo.newImageVersion, otaInfo.activeImageVersion);
        if (Ota_cbStatus != NULL)
        {
            statusParam[0] = gpOta_OtaState_ImageAvailableImageOutdated;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
        }
        Ota_InitOtaSession();
        return;
    }

    UInt32_buf2api(&otaInfo.fileSize, pData, 1, &dataOffset);

#if !defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
    // Only initialize the buffer when the ota session will commence
    // Freed in Ota_UpgradeEndRequest or on Ota_ImageBlockResponseIndication(gpOta_Status_Abort)
    otaInfo.hash_ctx = (ota_hash_context*) GP_POOLMEM_MALLOC(sizeof(ota_hash_context));
    if (otaInfo.hash_ctx == NULL)
    {
        GP_LOG_PRINTF("[OTA] Failed to allocate hash context", 0);
        if (Ota_cbStatus != NULL)
        {
            statusParam[0] = gpOta_OtaState_DownloadingFileCrcFailure;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete, (void*)statusParam);
        }
        Ota_InitOtaSession();
        return;
    }
    Ota_CalculateHash_Init(otaInfo.hash_ctx);
    if (Ota_CalculateHash_Start(otaInfo.hash_ctx) == false)
    {
        GP_LOG_PRINTF("[OTA] Failed to start hash calculation", 0);
        if (Ota_cbStatus != NULL)
        {
            statusParam[0] = gpOta_OtaState_DownloadingFileCrcFailure;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete, (void*)statusParam);
        }
        Ota_CalculateHash_Free(otaInfo.hash_ctx);
        gpPoolMem_Free(otaInfo.hash_ctx);
        Ota_InitOtaSession();
        return;
    }

#endif

    GP_LOG_PRINTF("Query Next Image Response received with valid image. Start downloading!", 0);

    otaInfo.otaState = gpOta_OtaState_ImageAvailable;
    if (Ota_cbStatus != NULL)
    {
        Ota_cbStatus(gpOta_OtaState_ImageAvailable,NULL);
    }

    // Start OTA download procedure
    gpSched_ScheduleEvent(0, Ota_ImageDownload);
}

void Ota_ImageBlockResponseIndication(UInt8 pairingRef, UInt8* pData)
{
    static UInt32 flashaddress = 0;
    UInt8 status = gpOta_Status_Success;
    UInt16 manufCode = 0;
    UInt16 imageType = gpOta_ImageTypeMain;
    UInt32 currentTime = 0;
    UInt32 requestTime = 0;
    UInt16 blockRequestDelay = 0;
    UInt8 dataOffset = 0;
    UInt32 newFileVersion = 0;
    UInt32 fileOffset = 0;
    UInt8 dataSize = 0;
    UInt8 statusParam[2];

    if (pairingRef == pairingId)
    {
        /* Unschedule response timeout only when response is coming form an expected pairingRef */
        gpSched_UnscheduleEvent(Ota_RemoteResponseTimeout);
    }
    else
    {
        GP_LOG_PRINTF("Command received from wrong pairing ID, ignore!", 0);
        if(Ota_cbStatus != NULL)
        {
            statusParam[0] = gpOta_OtaState_ImageAvailableWrongPairId;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
        }
        return;
    }

    /* Validate received status */
    UInt8_buf2api(&status, pData, 1, &dataOffset);
    if (status == gpOta_Status_Abort)
    {
        GP_LOG_PRINTF("Image block response with status ABORT received", 0);
        if(Ota_cbStatus != NULL)
        {
            statusParam[0] = gpOta_OtaState_RemoteAbort;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
        }
#if !defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
        Ota_CalculateHash_Free(otaInfo.hash_ctx);
        gpPoolMem_Free(otaInfo.hash_ctx);
        // No need to free otaInfo.verify_ctx, as it does not live while "downloading"
        // No need to free otaInfo.ota_image_hash, as it does not live while "downloading"
#endif
        Ota_InitOtaSession(); //no further actions need to be taken.
        return;
    }
    else if (status == gpOta_Status_Wait_For_Data)
    {
        GP_LOG_SYSTEM_PRINTF("Image block response with status WAIT FOR DATA received", 0);
        UInt32_buf2api(&currentTime, pData, 1, &dataOffset);
        UInt32_buf2api(&requestTime, pData, 1, &dataOffset);
        UInt16_buf2api(&blockRequestDelay, pData, 1, &dataOffset);
        if (blockRequestDelay > otaInfo.blockRequestDelay)
        {
            otaInfo.blockRequestDelay = blockRequestDelay;
        }

        // Retry the same request later
        gpSched_UnscheduleEvent(Ota_ImageBlockRequest);
        gpSched_ScheduleEvent((requestTime - currentTime)*1000000UL, Ota_ImageBlockRequest);
        return;
    }
    else if (status != gpOta_Status_Success)
    {
        GP_LOG_PRINTF("Image Block Response received with status 0x%02X, abort!", 1, status);
        if(Ota_cbStatus != NULL)
        {
            statusParam[0] = gpOta_OtaState_ImageAvailableStatusNoSuccess;
            statusParam[1] = status;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
        }
        Ota_scheduleUpgradeEndRequest(gpOta_Status_Abort);
        return;
    }

    /* Validate manufacturer code */
    UInt16_buf2api(&manufCode, pData, 1, &dataOffset);
    if (manufCode != otaInfo.manufCode)
    {
        GP_LOG_SYSTEM_PRINTF("Image Block Response received with mismatching manufacturer code (0x%04X)", 1, manufCode);
        if(Ota_cbStatus != NULL)
        {
            statusParam[0] = gpOta_OtaState_ImageAvailableWrongManufatorCode;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
        }
        Ota_scheduleUpgradeEndRequest(gpOta_Status_Abort); //no further actions need to be taken.
        return;
    }

    /* Validate image type */
    UInt16_buf2api(&imageType, pData, 1, &dataOffset);
    if (imageType != otaInfo.imageType)
    {
        GP_LOG_PRINTF("Image Block Response received with mismatching image type (0x%02X)", 1, imageType);
        if (Ota_cbStatus != NULL)
        {
            statusParam[0] = gpOta_OtaState_ImageAvailableWrongImageType;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
        }
        Ota_scheduleUpgradeEndRequest(gpOta_Status_Abort); //no further actions need to be taken.
        return;
    }

    /* Validate file version */
    UInt32_buf2api(&newFileVersion, pData, 1, &dataOffset);
    if (newFileVersion != otaInfo.newImageVersion)
    {
        GP_LOG_PRINTF("Image Block Response received with mismatch on file version (0x%lx)", 1, newFileVersion);
        if (Ota_cbStatus != NULL)
        {
            statusParam[0] = gpOta_OtaState_ImageAvailableImageOutdated;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
        }
        Ota_scheduleUpgradeEndRequest(gpOta_Status_Abort); //no further actions need to be taken.
        return;
    }

    /* Validate file offset */
    UInt32_buf2api(&fileOffset, pData, 1, &dataOffset);
    if (fileOffset != otaInfo.otaFileDataOffset)
    {
        GP_LOG_PRINTF("Image Block Response received with mismatch on offset (0x%lx)", 1, fileOffset);
        if (Ota_cbStatus != NULL)
        {
            statusParam[0] = gpOta_OtaState_ImageOffsetWrong;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
        }
        Ota_scheduleUpgradeEndRequest(gpOta_Status_Invalid_Image); //no further actions need to be taken.
        return;
    }

    /* Validate data size */
    UInt8_buf2api(&dataSize, pData, 1, &dataOffset);
#ifdef GP_LOCAL_LOG
    // gpLog_PrintBuffer(dataSize, pData+OTA_IMAGE_BLOCK_RESPONSE_LENGTH);
#endif
    if (dataSize == 0)
    {
        GP_LOG_PRINTF("Empty OTA chunk received, probably mismatch in OTA file length specified in OTA header?",0);
        if (Ota_cbStatus != NULL)
        {
            statusParam[0] = gpOta_OtaState_ReachedEndOfFile;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
        }
        Ota_scheduleUpgradeEndRequest(gpOta_Status_Invalid_Image);
        return;
    }

    /* Handle image data */
    if (otaInfo.otaState == gpOta_OtaState_DownloadingHeader)
    {
        // Check header length is part of the received packet
        if ( (otaInfo.otaFileDataOffset <= OTA_HEADER_LENGTH_FIELD_INDEX) &&
             (otaInfo.otaFileDataOffset + dataSize >= (OTA_HEADER_LENGTH_FIELD_INDEX + OTA_HEADER_LENGTH_FIELD_SIZE))
           )
        {
            dataOffset += (OTA_HEADER_LENGTH_FIELD_INDEX - otaInfo.otaFileDataOffset);
            UInt16_buf2api(&otaInfo.otaFileHeaderSize, pData, 1, &dataOffset);
        }
    }
    else if (otaInfo.otaState == gpOta_OtaState_DownloadingSubElemTag)
    {
        UInt16 tagId = 0;
        UInt16_buf2api(&tagId, pData, 1, &dataOffset);
        UInt32_buf2api(&otaInfo.subElementDataSize, pData, 1, &dataOffset);
        switch(tagId)
        {
        case OTA_TAG_ID_MAIN_IMAGE:
            if (otaInfo.subElementDataSize > gpUpgrade_GetOtaAreaSize())
            {
                GP_LOG_SYSTEM_PRINTF("image size too big to store (%ld)", 1, otaInfo.subElementDataSize);
                if (Ota_cbStatus != NULL)
                {
                    statusParam[0] = gpOta_OtaState_ImageSizeTooLarge;
                    Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
                }
                Ota_scheduleUpgradeEndRequest(gpOta_Status_Invalid_Image);
                return;
            }
            otaInfo.actualImageSize = otaInfo.subElementDataSize;
#if defined(GP_UPGRADE_DIVERSITY_COMPRESSION)
            flashaddress = gpUpgrade_GetOtaAreaStartAddress();
#else
            flashaddress = gpUpgrade_GetOtaAreaStartAddress();
#endif
            break;
        case OTA_TAG_ID_JUMPTABLE_IMAGE:
#if  defined(GP_DIVERSITY_GPHAL_K8E)
            if (otaInfo.subElementDataSize != GP_UPGRADE_APP_JUMP_TABLE_SIZE)
            {
                GP_LOG_PRINTF("Wrong jumptable size in OTA file: 0x%lx!!!", 0, otaInfo.subElementDataSize);
                if (Ota_cbStatus != NULL)
                {
                    statusParam[0] = gpOta_OtaState_ImageSizeTooLarge;
                    Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
                }
                Ota_scheduleUpgradeEndRequest(gpOta_Status_Invalid_Image); //no further actions need to be taken.
                return;
            }
#if defined(GP_UPGRADE_DIVERSITY_COMPRESSION)
            flashaddress = gpUpgrade_GetOtaAreaStartAddress() - GP_UPGRADE_APP_JUMP_TABLE_SIZE;
#else
            flashaddress = GP_UPGRADE_APP_JUMP_TABLE_ADDR(gpUpgrade_GetOtaAreaStartAddress());
#endif

#endif // GP_DIVERSITY_GPHAL_K8C || GP_DIVERSITY_GPHAL_K8D || GP_DIVERSITY_GPHAL_K8E
            break;
        case OTA_TAG_ID_IMAGE_INTEGRITY:
            if (otaInfo.subElementDataSize != 0x00000010)
            {
                GP_LOG_PRINTF("Size of 'Image integrity' sub-element is invalid", 0);
                if (Ota_cbStatus != NULL)
                {
                    statusParam[0] = gpOta_OtaState_ImageSizeTooLarge;
                    Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
                }
                Ota_scheduleUpgradeEndRequest(gpOta_Status_Invalid_Image); //no further actions need to be taken.
                return;
            }
            break;
#if !defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
        case OTA_TAG_ID_SIGNATURE:
            if (Ota_GetExpectedSignatureLength() != (otaInfo.subElementDataSize - OTA_SUBELEMENT_SIGNATURE_DATA_INDEX))
            {
                GP_LOG_PRINTF("Size of 'Image signature' sub-element is invalid", 0);
                if (Ota_cbStatus != NULL)
                {
                    statusParam[0] = gpOta_OtaState_ImageSizeTooLarge;
                    Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
                }
                Ota_scheduleUpgradeEndRequest(gpOta_Status_Invalid_Image); //no further actions need to be taken.
                return;
            }
            break;
        case OTA_TAG_ID_SIGNER_CERTIFICATE:
            // Not implemented
            // Could be used to retrieve the signer's public key, input would be CA's public key + signer's IEEE address
            /* No break */
#endif /* !GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY */
        default:
            // Legacy OTA stops here when receiving a signed image
            GP_LOG_PRINTF("Unknown SubElement TAG -> 0x%04x", 0, tagId);
            if (Ota_cbStatus != NULL)
            {
                statusParam[0] = gpOta_OtaState_UnknownSubElementTag;
                Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
            }
            Ota_scheduleUpgradeEndRequest(gpOta_Status_Invalid_Image); //no further actions need to be taken.
            return;
        }
        currentTagId = tagId;
    }

#if defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
    if (currentTagId != OTA_TAG_ID_IMAGE_INTEGRITY)
    {
        // Calculate CRC of entire file = OTA header + each sub-elements before the
        // "hash sub-element" (which is considered the last sub-element).
        // Reference: ZigBee OTA specifications - 6.3.10.1 Hash Value Calculation
        gpUtils_CalculatePartialCrc32(&otaFileCrcCalculated, pData+OTA_IMAGE_BLOCK_RESPONSE_LENGTH, dataSize);
    }
#else
    if (currentTagId != OTA_TAG_ID_SIGNATURE)
    {
        /* Calculate hash of entire file = OTA header + each sub-elements before the
         * "signature sub-element" (which is considered the last sub-element).*/
        if (Ota_CalculateHash_Update(otaInfo.hash_ctx, pData+OTA_IMAGE_BLOCK_RESPONSE_LENGTH, dataSize) == false)
        {
            GP_LOG_PRINTF("Authentication failure, hash calculation failed", 0);
            if(Ota_cbStatus != NULL)
            {
                statusParam[0] = gpOta_OtaState_DownloadingFileCrcFailure;
                Ota_cbStatus(gpOta_OtaState_SubProcComplete, (void*)&status);
            }
            Ota_scheduleUpgradeEndRequest(gpOta_Status_Abort);
            return;
        }
#if defined(GP_LOCAL_LOG)
        static UInt32 processed_image_bytes = 0;
        processed_image_bytes += dataSize;
        GP_LOG_PRINTF("Ota_CalculateHash_Update (processed %ld bytes)", 0, processed_image_bytes);
#endif
    }
#endif

    if (otaInfo.otaState == gpOta_OtaState_DownloadingSubElemData)
    {
        switch(currentTagId)
        {
#if  defined(GP_DIVERSITY_GPHAL_K8E)
        case OTA_TAG_ID_JUMPTABLE_IMAGE:
            // Handle jumptables like OTA_TAG_ID_MAIN_IMAGE
            // No break
#endif
        case OTA_TAG_ID_MAIN_IMAGE:
            //CRC for flash verification only needs to be done for main image and jumptable
            gpUtils_CalculatePartialCrc32(&otaInfo.subElemDataCrcCalculated, pData+OTA_IMAGE_BLOCK_RESPONSE_LENGTH, dataSize);
            //Write chunk to flash
            Ota_ChunkWrite(flashaddress, dataSize, pData+OTA_IMAGE_BLOCK_RESPONSE_LENGTH);
            flashaddress += dataSize;
            break;
#if defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
        case OTA_TAG_ID_IMAGE_INTEGRITY:
            // Check OTA file crc value is part of the received packet
            if ( (otaInfo.subElemDataOffset <= OTA_SUBELEMENT_HASH_CRC_VALUE_INDEX) &&
                 (otaInfo.subElemDataOffset + dataSize >= (OTA_SUBELEMENT_HASH_CRC_VALUE_INDEX + OTA_SUBELEMENT_HASH_CRC_VALUE_SIZE))
               )
            {
                dataOffset += (OTA_SUBELEMENT_HASH_CRC_VALUE_INDEX - otaInfo.subElemDataOffset);
                UInt32_buf2api(&otaInfo.otaFileCrc, pData, 1, &dataOffset);
            }
            break;
#else
        case OTA_TAG_ID_SIGNATURE:
        {
            // Ignore "IEEE address" [8B] as it is only useful when the certificate is provided

            // Check signature data is part of the received packet
            UInt32 signature_length = otaInfo.subElementDataSize - OTA_SUBELEMENT_SIGNATURE_DATA_INDEX;
            if ( (otaInfo.subElemDataOffset <= OTA_SUBELEMENT_SIGNATURE_DATA_INDEX) &&
                 (otaInfo.subElemDataOffset + dataSize >= (OTA_SUBELEMENT_SIGNATURE_DATA_INDEX + signature_length))
               )
            {
                dataOffset += (OTA_SUBELEMENT_SIGNATURE_DATA_INDEX - otaInfo.subElemDataOffset);
                otaInfo.signature = GP_POOLMEM_MALLOC(signature_length);
                if (otaInfo.signature == NULL)
                {
                    if(Ota_cbStatus != NULL)
                    {
                        statusParam[0] = gpOta_OtaState_ImageAvailableStatusNoSuccess;
                        statusParam[1] = 0xDE;
                        Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
                    }
                    Ota_scheduleUpgradeEndRequest(gpOta_Status_Abort);
                    return;
                }
                UInt8_buf2api(otaInfo.signature, pData, signature_length, &dataOffset);
                GP_LOG_PRINTF("Received signature data", 0);
            }
            break;
        }
        case OTA_TAG_ID_SIGNER_CERTIFICATE:
            // Not implemented
            // Could be used to retrieve the signer's public key, input would be CA's public key + signer's IEEE address
            /* No break*/
#endif //GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY
        default:
            // Ignore this sub-element
            break;
        }

        otaInfo.subElemDataOffset += dataSize;
    }

    otaInfo.otaFileDataOffset += dataSize;

    // Continue download procedure
    gpSched_ScheduleEvent(MS_TO_US(otaInfo.blockRequestDelay), Ota_ImageDownload);
}

void Ota_UpgradeEndResponseIndication(UInt8 pairingRef, UInt8* pData)
{
    UInt16 manufCode = 0;
    gpOta_ImageType_t imageType = gpOta_ImageTypeMain;
    UInt32 currentTime = 0;
    UInt32 upgradeTime = 0;
    UInt8 dataOffset = 0;
    UInt32 newFileVersion = 0;
    gpOta_StatusParam_t statusParam = 0;

    if (pairingRef != pairingId)
    {
        GP_LOG_PRINTF("Command received from wrong pairing ID, ignore!", 0);
        if (Ota_cbStatus != NULL)
        {
            statusParam = gpOta_OtaState_ImageAvailableWrongPairId;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)&statusParam);
        }
        return;
    }

    /* Unschedule response timeout only when response is coming from expeted paringRef */
    gpSched_UnscheduleEvent(Ota_RemoteResponseTimeout);

    UInt16_buf2api(&manufCode, pData, 1, &dataOffset);
    if (manufCode != otaInfo.manufCode)
    {
        GP_LOG_PRINTF("Upgrade End Response received with mismatching manufacturer code (0x%04X)", 1, manufCode);
        if (Ota_cbStatus != NULL)
        {
            statusParam = gpOta_OtaState_ImageAvailableWrongManufatorCode;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)&statusParam);
        }
        Ota_InitOtaSession(); //no further actions need to be taken.
        return;
    }

    UInt16_buf2api(&imageType, pData, 1, &dataOffset);
    if (imageType != otaInfo.imageType)
    {
        GP_LOG_PRINTF("Upgrade End Response received with mismatching image type (0x%02X)", 1, imageType);
        if (Ota_cbStatus != NULL)
        {
            statusParam = gpOta_OtaState_ImageAvailableWrongImageType;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)&statusParam);
        }
        Ota_InitOtaSession(); //no further actions need to be taken.
        return;
    }

    UInt32_buf2api(&newFileVersion, pData, 1, &dataOffset);
    if (newFileVersion != otaInfo.newImageVersion)
    {
        GP_LOG_PRINTF("Upgrade End Response received with mismatch on file version (0x%08X)", 1, newFileVersion);
        if (Ota_cbStatus != NULL)
        {
            statusParam = gpOta_OtaState_ImageAvailableImageOutdated;
            Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)&statusParam);
        }
        Ota_InitOtaSession(); //no further actions need to be taken.
        return;
    }

    UInt32_buf2api(&currentTime, pData, 1, &dataOffset);
    UInt32_buf2api(&upgradeTime, pData, 1, &dataOffset);

    if (Ota_cbStatus != NULL)
    {
        UInt32 UpgradeOffset = upgradeTime - currentTime;

        Ota_cbStatus(gpOta_OtaState_UpgradeScheduled,(void*)&UpgradeOffset);
    }

    gpSched_ScheduleEvent((upgradeTime-currentTime)*1000000, Ota_StartUpgrade);
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
Bool gpOta_StartOtaSession(UInt8 pairingRef, UInt16 manufCode, gpOta_ImageType_t imageType, UInt32 activeVersion, UInt16 hardwareVersion, gpOta_FuncPtr_t *funcPtr)
{
    if (otaInfo.otaState != gpOta_OtaState_Idle)
    {
        GP_LOG_PRINTF("Already ongoing OTA session",0);
        return false;
    }

    //Prepare everything to start the OTA session.
    Ota_InitOtaSession();

    pairingId = pairingRef;
    otaInfo.manufCode = manufCode;
    otaInfo.imageType = imageType;
    otaInfo.activeImageVersion = activeVersion;
    otaInfo.activeHwVersion = hardwareVersion;
    if (funcPtr) //Callbacks might be assigned using gpOta_Register function
    {
        Ota_DataRequest = funcPtr->sendFunction;
        Ota_cbStatus = funcPtr->cbStatus;
    }
    GP_ASSERT_DEV_INT(Ota_DataRequest != NULL);
    GP_ASSERT_DEV_INT(Ota_cbStatus != NULL);
    //move to next state and trigger Query Next Image Request.
    otaInfo.otaState = gpOta_OtaState_QueryImage;
    gpSched_ScheduleEvent(0, Ota_QueryNextImageRequest);

    return true;
}

void gpOta_Register(gpOta_FuncPtr_t* funcPtr)
{
    GP_ASSERT_DEV_INT(funcPtr != NULL);
    Ota_DataRequest = funcPtr->sendFunction;
    Ota_cbStatus = funcPtr->cbStatus;
}

/* Ota session will be halted till the API gpOta_ResumeOtaSession is accessed. Multiple entries (OTA_HOLD_MAX_DEPTH) of hold are allowed.
   In case of overflow / underflow of the entry counter, the Ota SESSION is terminated. If the maximum hold time out expires, the
   OTA session is terminated. Maximum Hold is measured from the first access till last resume. Once the resume counter becomes zero
   the time out is reset.
*/
gpOta_Status_t gpOta_HoldOtaSession( void )
{
    if (otaInfo.otaState != gpOta_OtaState_Idle)
    {
        if (OtaHold == 0)
        {
            /* First hold, set hold, trigger callback and schedule time out */
            OtaHold++;
            GP_LOG_PRINTF("Ota hold: %u", 0, OtaHold);
            /* Schedule callback */
            gpSched_ScheduleEvent(0, Ota_cbHoldScheduled);
            /* Schedule timeout */
            gpSched_ScheduleEvent(OTA_HOLD_MAX_TIMEOUT, Ota_HoldMaxTimeout);

            /* Unschedule response timeout, time out will expire during hold */
            gpSched_UnscheduleEvent(Ota_RemoteResponseTimeout);
        }
        else if (OtaHold < OTA_HOLD_MAX_DEPTH)
        {
            OtaHold++;
            GP_LOG_PRINTF("Ota hold: %u", 0, OtaHold);
            /* Schedule callback */
            gpSched_ScheduleEvent(0, Ota_cbHoldScheduled);
        }
        else
        {
            /* Max OTA hold depth, terminate download */
            /* re-initilaization of OtaHold is handled in Ota_InitOtaSession()*/
            GP_LOG_SYSTEM_PRINTF("Ota hold overflow, download terminated!",0);
            gpSched_ScheduleEvent(0, Ota_HoldTerminate);
        }
    }
    else
    {
        GP_LOG_PRINTF("Ota hold while no Ota in progress",0);
        return gpOta_Status_NoOtaInProgress;
    }

    return gpOta_Status_Success;
}

gpOta_Status_t gpOta_ResumeOtaSession( void )
{
    if (otaInfo.otaState != gpOta_OtaState_Idle)
    {
        if (OtaHold == 0)
        {
            /* OTA underflow, terminate download */
            /* re-initilaization of OtaHold is handled in Ota_InitOtaSession()*/
            GP_LOG_SYSTEM_PRINTF("Ota hold overflow, download terminated!",0);
            gpSched_ScheduleEvent(0, Ota_HoldTerminate);
        }
        else if (OtaHold == 1)
        {
            OtaHold--;
            GP_LOG_PRINTF("Ota resumed!",0);
            /* Unschedule Time out */
            gpSched_UnscheduleEvent(Ota_HoldMaxTimeout);
            /* Schedule callback */
            gpSched_ScheduleEvent(0, Ota_cbResumeScheduled);
            /* Resume download procedure */
            gpSched_ScheduleEvent(0, Ota_ImageDownload);
        }
        else
        {
            /* Decrease OTA download */
            OtaHold--;
            GP_LOG_SYSTEM_PRINTF("Ota resume requires %u more call(s)", 0, OtaHold);
        }
    }
    else
    {
        GP_LOG_PRINTF("Ota_Resume while no Ota in progress",0);
        return gpOta_Status_NoOtaInProgress;
    }
    return gpOta_Status_Success;
}

gpOta_Status_t gpOta_AbortOtaSession(void)
{
    gpSched_UnscheduleEvent(Ota_RemoteResponseTimeout);
#if !defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
    if (gpSched_UnscheduleEvent(Ota_OtaImageAuthenticityValidation_Start))
    {
        Ota_OtaImageAuthenticityValidation_Cleanup();
    }
#endif

    if (Ota_DataRequest != NULL)
    {
        gpOta_Status_t *pStatus = GP_POOLMEM_MALLOC(sizeof(gpOta_Status_t));
        if (pStatus)
        {
            *pStatus = gpOta_Status_Abort;
            Ota_UpgradeEndRequest((void *)pStatus);     //pStatus is freed inside
            return gpOta_Status_Success;
        }
    }

    Ota_InitOtaSession();

    return gpOta_Status_Success;
}

gpOta_Status_t gpOta_scheduleUpgradeEndRequestOtaSession( void )
{
    if (otaInfo.otaState != gpOta_OtaState_Idle)
    {
        /* Unschedule response timeout, avoid time out on aborted process */
        gpSched_UnscheduleEvent(Ota_RemoteResponseTimeout);

        /* Set flag to abort, abort will be handled Ota_ImageDownload */
        Ota_TriggerAbort = OTA_TRIGGER_ABORT;

        /* If in hold state schedule Ota_ImageDownload */
        if (OtaHold != 0)
        {
            /* First unschedule, to avoid multiple entries */
            gpSched_UnscheduleEvent(Ota_ImageDownload);
            gpSched_ScheduleEvent(0, Ota_ImageDownload);
        }

    }
    else
    {
        GP_LOG_PRINTF("Ota_scheduleUpgradeEndRequest while no Ota in progress",0);
        return gpOta_Status_NoOtaInProgress;
    }
    return gpOta_Status_Success;
}

void Ota_HoldMaxTimeout(void)
{
    if (Ota_cbStatus != NULL)
    {
        UInt8 statusParam = gpOta_OtaState_DowloadHoldTimeout;
        Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)&statusParam);
    }
#if !defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
    if (gpSched_UnscheduleEvent(Ota_OtaImageAuthenticityValidation_Start))
    {
        Ota_OtaImageAuthenticityValidation_Cleanup();
    }
#endif
    Ota_scheduleUpgradeEndRequest(gpOta_Status_Abort);
}

void Ota_HoldTerminate(void)
{
    if (Ota_cbStatus != NULL)
    {
        UInt8 statusParam = gpOta_OtaState_DowloadHoldOverUnderflow;
        Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)&statusParam);
    }

#if !defined(GP_OTA_VALIDATE_INTEGRITY_WO_AUTHENTICITY)
    if (gpSched_UnscheduleEvent(Ota_OtaImageAuthenticityValidation_Start))
    {
        Ota_OtaImageAuthenticityValidation_Cleanup();
    }
#endif

    /* There can be a pending action if this is scheduled because of OtaHold underflow,
       go through abort flow if OtaHold not zero */
    if (OtaHold != 0)
    {
        Ota_scheduleUpgradeEndRequest(gpOta_Status_Abort);
    }
    else
    {
        gpOta_scheduleUpgradeEndRequestOtaSession();
    }
}

void Ota_cbHoldScheduled(void)
{
    if (Ota_cbStatus != NULL)
    {
        UInt8 statusParam[2];
        statusParam[0] = gpOta_OtaState_DownloadHold;
        statusParam[1] = OtaHold;
        Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
    }
}

void Ota_cbResumeScheduled(void)
{
    if (Ota_cbStatus != NULL)
    {
        UInt8 statusParam[2];
        statusParam[0] = gpOta_OtaState_DownloadResume;
        statusParam[1] = OtaHold;
        Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)statusParam);
    }
}

void Ota_RemoteResponseTimeout( void )
{
    if (Ota_cbStatus != NULL)
    {
        UInt8 statusParam = gpOta_OtaState_NoRemoteResponse;
        Ota_cbStatus(gpOta_OtaState_SubProcComplete,(void*)&statusParam);
    }

    /* Remote side not responsive, don't send UpgradeEndRequest with abort status! */
    Ota_InitOtaSession();
}
void Ota_cbOtaAreaErased(void)
{
    GP_LOG_PRINTF("OTA area erased",0);
    Ota_OtaAreaWiped = true;

    // Return to the download state machine
    gpSched_ScheduleEvent(0, Ota_ImageDownload);
}

static void Ota_scheduleUpgradeEndRequest(gpOta_Status_t status)
{
    gpOta_Status_t *pStatus = GP_POOLMEM_MALLOC(sizeof(status));

    GP_LOG_PRINTF("Schedule upgrade end request w/ status %02x", 0, status);

    *pStatus = status;
    gpSched_ScheduleEventArg(0, Ota_UpgradeEndRequest, (void*)pStatus);
}

