/*
 * Copyright (c) 2019, Qorvo Inc
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

/** @file "gpOta.h"
 *
 *  OTA Implementation
 *
 *  Declarations of the public functions and enumerations of gpOta.
*/

#ifndef _GPOTA_H_
#define _GPOTA_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

/** @enum gpOta_Status_t */
//@{
/** @brief Success Operation */
#define gpOta_Status_Success                                   0x00
/** @brief Failed case when a client or a server decides to abort the upgrade process */
#define gpOta_Status_Abort                                     0x95
/** @brief Server is not authorized to upgrade the client */
#define gpOta_Status_Not_Authorized                            0x7E
/** @brief Invalid OTA upgrade image (ex. failed signature validation or signer information check or CRC check */
#define gpOta_Status_Invalid_Image                             0x96
/** @brief Server does not have data block available yet */
#define gpOta_Status_Wait_For_Data                             0x97
/** @brief No OTA upgrade image available for a particular client */
#define gpOta_Status_No_Image_Available                        0x98
/** @brief The command received is badly formatted. It usually means the command is missing certain fields or values included in the fields are invalid */
#define gpOta_Status_Malformed_Command                         0x80
/** @brief Such command is not supported on the device */
#define gpOta_Status_Unsup_Cluster_Command                     0x81
/** @brief The client still requires more OTA upgrade image files in order to successfully upgrade */
#define gpOta_Status_Require_More_Image                        0x99
/** @brief OTA link is no active */
#define gpOta_Status_LinkNotReady                              0xFD
/** @brief Ota control API accessed while there is no active Ota session */
#define gpOta_Status_NoOtaInProgress                           0xFE
typedef UInt8                             gpOta_Status_t;
//@}

/** @enum gpOta_OtaState_t */
//@{
/** @brief Downloading image in Idle state */
#define gpOta_OtaState_Idle                                    0x00
/** @brief Query host/target for available OTA image */
#define gpOta_OtaState_QueryImage                              0x01
/** @brief State entered after success feedback from query image */
#define gpOta_OtaState_ImageAvailable                          0x02
/** @brief Downloading image header */
#define gpOta_OtaState_DownloadingHeader                       0x03
/** @brief Downloading image tags, tags are used to identify different sections inside an image */
#define gpOta_OtaState_DownloadingSubElemTag                   0x04
/** @brief Downloading image section identified by previous tag */
#define gpOta_OtaState_DownloadingSubElemData                  0x05
/** @brief Image fully downloaded */
#define gpOta_OtaState_DownloadComplete                        0x06
/** @brief Image upgrade started */
#define gpOta_OtaState_UpgradeImage                            0x07
/** @brief This is not a real state, used for status reporting to application */
#define gpOta_OtaState_UpgradeScheduled                        0x08
/** @brief The SubProcComplete is used to report changes/updates inside a state that requires reporting to application. e.g. while downloading a new chunk is recieved. */
#define gpOta_OtaState_SubProcComplete                         0x09
/** @brief This is not a real state, used for reporting new image availability on Server to Client's application */
#define gpOta_OtaState_ImageNotificationReceived               0x0a
/** @brief Verifying the downloaded subelement was written correctly into flash */
#define gpOta_OtaState_VerifyingSubElemData                    0x0b
/** @brief Verifying the full ota image */
#define gpOta_OtaState_VerifyingImage                          0x0c
typedef UInt8                             gpOta_OtaState_t;
//@}

/** @enum gpOta_StatusParam_t */
//@{
#define gpOta_OtaState_ImageAvailableInvalidState              0x01
#define gpOta_OtaState_ImageAvailableWrongPairId               0x02
/** @brief This error code will be followed by a gpOta_Status_t, so a void pointer with to an UInt8 array[2] */
#define gpOta_OtaState_ImageAvailableStatusNoSuccess           0x03
#define gpOta_OtaState_ImageAvailableWrongManufatorCode        0x04
#define gpOta_OtaState_ImageAvailableWrongImageType            0x05
#define gpOta_OtaState_ImageAvailableImageOutdated             0x06
#define gpOta_OtaState_FlashVerifyFailure                      0x07
#define gpOta_OtaState_DownloadingFileCrcFailure               0x08
#define gpOta_OtaState_DownloadedChunk                         0x09
#define gpOta_OtaState_DownloadFailed                          0x0A
#define gpOta_OtaState_RemoteAbort                             0x0B
#define gpOta_OtaState_ImageOffsetWrong                        0x0C
#define gpOta_OtaState_ImageSizeTooLarge                       0x0D
#define gpOta_OtaState_DownloadHold                            0x0E
#define gpOta_OtaState_DownloadResume                          0x0F
#define gpOta_OtaState_DowloadHoldOverUnderflow                0x10
#define gpOta_OtaState_DowloadHoldTimeout                      0x11
#define gpOta_OtaState_Abort                                   0x12
#define gpOta_OtaState_NoDataRequestRegistered                 0x13
#define gpOta_OtaState_NoRemoteResponse                        0x14
#define gpOta_OtaState_ReachedEndOfFile                        0x15
#define gpOta_OtaState_UnknownSubElementTag                    0x16
#define gpOta_OtaState_DownloadingFileSignatureFailure         0x17
/** @typedef gpOta_StatusParam_t
 *  @brief Sub-types passed as parametr if cb... is called with gpOta_OtaState_SubProcComplete
*/
typedef UInt8                             gpOta_StatusParam_t;
//@}

/** @enum gpOta_ImageType_t */
//@{
/** @brief The image is a main firmware image. */
#define gpOta_ImageTypeMain                                    0x0000
/** @brief The image contains a new public key for signature validation. */
#define gpOta_ImageTypeSecurityCredentials                     0xFFC0
/** @brief Image type is unknown */
#define gpOta_ImageTypeUnknown                                 0xFFFF
typedef UInt16                            gpOta_ImageType_t;
//@}

/** @enum gpOta_ImageNotifyType_t */
//@{
/** @brief The image is a main firmware image. */
#define gpOta_ImgNotifyQueryJitterOnly                         0x00
/** @brief Image type is unknown */
#define gpOta_ImgNotifyAll                                     0x03
typedef UInt8                             gpOta_ImageNotifyType_t;
//@}

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/** @macro GP_OTA_MANUFACTURER_CODE_QORVO */
#define GP_OTA_MANUFACTURER_CODE_QORVO               0x10D0
/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/** @pointer to function gpOta_DataRequest_t
 *  @brief Typedef to register Ota datarequest function to send the commands.
*   @param pairingRef                Reference to Pairing id. This can be a linkId for BLE or a pairingId for RF4CE. This reference is needed to understand to which device the request needs to be sent.
*   @param length                    Length of the command to be sent.
*   @param pData                     Command to send over-the-air
*/
typedef void (*gpOta_DataRequest_t) (UInt8 pairingRef, UInt16 length, UInt8* pData);

/** @pointer to function gpOta_cbStatusReport_t
 *  @brief Typedef to register Ota status callback to application
*   @param status                    Status of Ota Statemachine, with some addtional status
*   @param statusParam               void pointer used to pass through some sub-state or status parameters
*/
typedef void (*gpOta_cbStatusReport_t) (gpOta_StatusParam_t status, void* statusParam);

/** @struct gpOta_FuncPtr_t
 *  @brief Struct to pass through function pointers required to run gpOta
*/
typedef struct {
    /** @brief Function pointer to hook up a dataRequest function used by the download carrier */
    gpOta_DataRequest_t            sendFunction;
    /** @brief Function pointer to application callback to report OTA status */
    gpOta_cbStatusReport_t         cbStatus;
} gpOta_FuncPtr_t;

/** @struct gpOta_ImageNotification_t
 *  @brief Struct to pass Image Notification Data
*/
typedef struct {
    /** @brief Reference to paired device (linkId or PairingId) */
    UInt8                          pairingRef;
    /** @brief Vendor ID */
    UInt16                         manufCode;
    /** @brief Image Type */
    UInt16                         imageType;
    /** @brief OTA upgrade file version that the server tries to upgrade client devices to */
    UInt32                         fileVersion;
} gpOta_ImageNotification_t;

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//Requests


#if defined(GP_OTA_DIVERSITY_CLIENT)
/** @brief Start OTA Session with querying the gpOta Server for a new image
*
*   @param pairingRef                Reference to paired device (linkId or PairingId)
*   @param manufCode                 Look for images with matching Manufacturer code
*   @param imageType                 Look for images with matching imageType
*   @param activeVersion             Current running application version
*   @param hardwareVersion           Current HW version that is in used
*   @param funcPtr                   Function pointers to hook gpOta on to
*   @return result                   FALSE if OTA process is ongoing, TRUE otherwise
*/
Bool gpOta_StartOtaSession(UInt8 pairingRef, UInt16 manufCode, gpOta_ImageType_t imageType, UInt32 activeVersion, UInt16 hardwareVersion, gpOta_FuncPtr_t* funcPtr);
#endif //defined(GP_OTA_DIVERSITY_CLIENT)

/** @brief Register OTA Session when ready to serve new image
*
*   @param funcPtr                   Function pointers to hook gpOta on to
*/
void gpOta_Register(gpOta_FuncPtr_t* funcPtr);

/** @brief Callback function of data that comes in from other gpOta Device.
*
*   @param pairingRef                Reference to paired device (linkId or PairingId)
*   @param dataLength                Length of the command that is received.
*   @param pData                     Pointer to the data that is received.
*/
void gpOta_cbDataIndication(UInt8 pairingRef, UInt16 dataLength, UInt8* pData);

/** @brief Function to allow the application to put an OTA session on hold
 *  @return result
*/
gpOta_Status_t gpOta_HoldOtaSession(void);

/** @brief Function to allow the application to resume an OTA session.
 *  @return result
*/
gpOta_Status_t gpOta_ResumeOtaSession(void);

/** @brief Function to allow the application to abort an OTA session.
 *  @return result
*/
gpOta_Status_t gpOta_AbortOtaSession(void);




#if defined(GP_DIVERSITY_LOG)
/** @brief Convert gpOta_Status_t to string
*
*   @param val                       gpOta_OtaState_t value
*   @return str                      gpOta_OtaState_t string
*/
const char* gpOta_OtaState2String(gpOta_OtaState_t val);
#endif //defined(GP_DIVERSITY_LOG)

#if defined(GP_DIVERSITY_LOG)
/** @brief Convert gpOta_Status_t to string
*
*   @param val                       gpOta_StatusParam_t value
*   @return str                      gpOta_StatusParam_t string
*/
const char* gpOta_StatusParam2String(gpOta_StatusParam_t val);
#endif //defined(GP_DIVERSITY_LOG)

#if defined(GP_DIVERSITY_LOG)
/** @brief Convert gpOta_Status_t to string
*
*   @param val                       gpOta_Status_t value
*   @return str                      gpOta_Status_t string
*/
const char* gpOta_Status2String(gpOta_Status_t val);
#endif //defined(GP_DIVERSITY_LOG)




//Indications



#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_GPOTA_H_

