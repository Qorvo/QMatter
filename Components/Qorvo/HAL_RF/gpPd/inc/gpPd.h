/*
 * Copyright (c) 2011-2016, GreenPeak Technologies
 * Copyright (c) 2017-2019, Qorvo Inc
 *
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
 * Alternatively, this software may be distributed under the terms of the
 * modified BSD License or the 3-clause BSD License as published by the Free
 * Software Foundation @ https://directory.fsf.org/wiki/License:BSD-3-Clause
 *
 * $Header: //depot/release/Embedded/Components/Qorvo/HAL_RF/v2.10.2.1/comps/gpPd/inc/gpPd.h#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

/** @file "gpPd.h"
 *
 *  Packet Descriptor Implementation
 *
 *  Declarations of the public functions of gpPd.
*/


#ifndef _GPPD_H_
#define _GPPD_H_

#if defined(GP_DIVERSITY_ROM_CODE)
#include "gpPd_RomCode.h"
#else //defined(GP_DIVERSITY_ROM_CODE)

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include <global.h>
#ifdef GP_COMP_GPHAL
// inclusion for number of pbms
#include "gpHal_Pbm.h"
#endif //GP_COMP_GPHAL

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#if defined (GP_DIVERSITY_PD_USE_PBM_VARIANT)
#ifndef GP_PD_NR_OF_HANDLES
#define GP_PD_NR_OF_HANDLES         GPHAL_NUMBER_OF_PBMS_USED
#endif
#endif //(GP_DIVERSITY_PD_USE_PBM_VARIANT)

#define GP_PD_INVALID_HANDLE           0xFF

#define GP_PD_BUFFER_SIZE_ZIGBEE            128
#define GP_PD_BUFFER_SIZE_BLE               256

#define gpPd_GetPdUnit(pdHandle)                    pdHandle = gpPd_GetPd()
#define gpPd_GetCustomPdUnit(pdHandle,type,size)    pdHandle = gpPd_GetCustomPd(type,size)

#define gpPd_FreePdUnit(pdHandle)         gpPd_FreePd(pdHandle)
#define gpPd_ReadByteStreamUnit(pdHandle, offset, length, pData)    gpPd_ReadByteStream(pdHandle, offset, length, pData)
#define gpPd_WriteByteStreamUnit(pdHandle, offset, length, pData)    gpPd_WriteByteStream(pdHandle, offset, length, pData)
#define gpPd_CheckPdValidUnit(pdHandle)   gpPd_CheckPdValid(pdHandle)

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

#define GP_PD_IS_BUFFERTYPE_VALID(type)     (type < gpPd_BufferTypeInvalid)

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef UInt8 gpPd_Handle_t;
typedef UInt16 gpPd_Length_t;
typedef UInt16 gpPd_Offset_t;
typedef UInt32 gpPd_TimeStamp_t;
typedef Int8 gpPd_Rssi_t;
typedef UInt8 gpPd_Lqi_t;


#define gpPd_ResultValidHandle        0x0
#define gpPd_ResultInvalidHandle      0x1
#define gpPd_ResultNotInUse           0x2
typedef UInt8 gpPd_Result_t;

#define gpPd_BufferTypeZigBee       0x00
#define gpPd_BufferTypeBle          0x01
#define gpPd_BufferTypeXLBle        0x02
#define gpPd_BufferTypeInvalid      0x03
typedef UInt8 gpPd_BufferType_t;

typedef struct gpPd_Loh_s{
    gpPd_Length_t   length;
    gpPd_Offset_t   offset;
    gpPd_Handle_t   handle;
}gpPd_Loh_t; // Length Offset Handle


/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#if defined(GP_DIVERSITY_JUMPTABLES) && defined(GP_DIVERSITY_ROM_CODE)
#include "gpPd_CodeJumpTableFlash_Defs.h"
#endif // defined(GP_DIVERSITY_JUMPTABLES) && defined(GP_DIVERSITY_ROM_CODE)
/* JUMPTABLE_FLASH_FUNCTION_DEFINITIONS_START */

void    gpPd_Init(void);
void    gpPd_DeInit(void);

//Descriptor handling
/** @brief  Return a handle to use for packet construction and transmission
 *          This function will return a gpPd_BufferTypeZigBee.
 *          See gpPd_GetCustomPd() for other buffer types (BLE).
 *
 *  @return pdHandle    Handle as reference for use with the packet memory througout other APIs.
 *                      GP_PD_INVALID_HANDLE will be returned if no handles are available.
*/
gpPd_Handle_t       gpPd_GetPd(void);
/** @brief  Return a handle to use for a specific packet buffer type.
 *
 *  @param  type        Buffer type requested. Possible types listed in gpPd_BufferType_t.
 *  @param  size        Buffer size needed for the requested packet. Size must align with the request type (ZB/BLE/BLE XL).
 *
 *  @return pdHandle    Handle as reference for use with the packet memory througout other APIs.
 *                      Will return GP_PD_INVALID_HANDLE in the following cases:
 *                      - no handles are available.
 *                      - no buffers with the requested size are available
 *                      - requested size is not valid for the selected type.
*/
gpPd_Handle_t       gpPd_GetCustomPd(gpPd_BufferType_t type, UInt16 size);
/** @brief Release a Pd handle and it's associated memory after use.
 *
 *  Pd handle can be freed, allowing it and his associated memory to be re-used by the application (Tx) or HW (Rx).
 *  It is expected the handles are not kept for long durations, not to block the ability to send or receive new packets.
 *  The recommentation is the copy all needed information in a short lifespan.
 *  Note that accessing the handle after freeing is an illegal operation.
 *
 *  @param pdHandle  Handle referring to Tx or Rx buffer used.
*/
void                gpPd_FreePd(gpPd_Handle_t pdHandle);
/** @brief Lookup pd handle by PBM.
 *
 *
 *  @param pdHandle  Handle referring to Tx or Rx buffer used.
*/
gpPd_Handle_t       gpPd_GetPdFromPBM(UInt8 pbmHandle);
/** @brief Check if the Pd is valid.
 *  To be used to check a fetched Pd.
 *  This function is advised to be used opposed to comparing against GP_PD_INVALID_HANDLE as more checks are performed.
 *
 *  @param pdHandle  Handle fetched or received.
 *  @return result   Can return the following:
                     - gpPd_ResultValidHandle if it's in range of possible handle values.
                     - gpPd_ResultInvalidHandle if the handle is outside the possible range.
                     - gpPd_ResultNotInUse if the handle is not considered claimed (fi when feeding an unfetched but valid handle value).
*/
gpPd_Result_t       gpPd_CheckPdValid(gpPd_Handle_t pdHandle);
/** @brief Check if the Pd is valid.
 *
 *  @param  pdHandle Handle referring to buffer.
 *  @return type     Buffer type of gpPd_BufferType_t pertaining to the given handle.
*/
gpPd_BufferType_t   gpPd_GetPdType(gpPd_Handle_t pdHandle);

//Data handling
/** @brief  Read one byte from a Pd buffer.
 *
 *  @param  pdHandle    Handle associated with the packet buffer.
 *  @param  offset      Offset to location in buffer to read from.
 *  @return byte        Byte read out @ offset.
*/
UInt8         gpPd_ReadByte(gpPd_Handle_t pdHandle, gpPd_Offset_t offset);
/** @brief  Write one byte to a Pd buffer.
 *
 *  @param  pdHandle    Handle associated with the packet buffer.
 *  @param  offset      Offset to location in buffer to write to.
 *  @param  byte        Byte to write @ offset.
*/
void          gpPd_WriteByte(gpPd_Handle_t pdHandle, gpPd_Offset_t offset, UInt8 byte);
/** @brief  Read a bytestream from a Pd buffer.
 *
 *  @param  pdHandle    Handle associated with the packet buffer.
 *  @param  offset      Offset to location in buffer to start read from.
 *  @param  length      Length to read.
 *  @return pData       Pointer to buffer for bytes read out @ offset.
*/
void          gpPd_ReadByteStream(gpPd_Handle_t pdHandle, gpPd_Offset_t offset, UInt8 length, UInt8* pData);
/** @brief  Write a bytestream to a Pd buffer.
 *
 *  @param  pdHandle    Handle associated with the packet buffer.
 *  @param  offset      Offset to location in buffer to start write operation.
 *  @param  length      Length to write.
 *  @return pData       Pointer to buffer of bytes written @ offset.
*/
void          gpPd_WriteByteStream(gpPd_Handle_t pdHandle, gpPd_Offset_t offset, UInt8 length, UInt8* pData);

//Data handling with update of pdLoh
/** @brief  Append a bytestream to a Pd buffer and update the pdLoh structure.
 *
 *  @param  pPdLoh    Pointer to Length,Offset,Handle associated with the packet buffer.
 *  @param  length    Length to write.
 *  @param  pData     Pointer to buffer of bytes written @ Offset (in pdLoh structure).
*/
void          gpPd_AppendWithUpdate(gpPd_Loh_t *pPdLoh, UInt8 length, UInt8 const *pData);
/** @brief  Prepend a bytestream to a Pd buffer and update the pdLoh structure.
 *          Typically used to add header information in front of payload bytes of higher layers.
 *
 *  @param  pPdLoh    Pointer to Length,Offset,Handle associated with the packet buffer.
 *  @param  length    Length to write.
 *  @param  pData     Pointer to buffer of bytes written @ Offset (in pdLoh structure).
*/
void          gpPd_PrependWithUpdate(gpPd_Loh_t *pPdLoh, UInt8 length, UInt8 const *pData);
/** @brief  Read a bytestream from a Pd buffer and update the pdLoh structure.
 *          Typically used to scroll through bytes in a Pd.
 *
 *  @param  pPdLoh    Pointer to Length,Offset,Handle associated with the packet buffer.
 *  @param  length    Length to read.
 *  @param  pData     Pointer to buffer for bytes read out @ Offset (in pdLoh structure).
*/
void          gpPd_ReadWithUpdate(gpPd_Loh_t *pPdLoh, UInt8 length, UInt8 *pData);

//Properties handling
//Rx
/** @brief  Return the Relative Signal Strength Indication (RSSI) of the received packet.
 *
 *  @param  pdHandle    Handle associated with the packet buffer.
 *  @return rssi        RSSI value in dBm.
*/
gpPd_Rssi_t         gpPd_GetRssi(gpPd_Handle_t pdHandle);
/** @brief  Return the Link Quality Indication (LQI) of the received packet.
 *
 *  @param  pdHandle    Handle associated with the packet buffer.
 *  @return lqi         LQI value (as specified in 802.15.4 PHY specification).
*/
gpPd_Lqi_t          gpPd_GetLqi(gpPd_Handle_t pdHandle);
/** @brief  Return the timestamp of the received packet.
 *
 *  @param  pdHandle    Handle associated with the packet buffer.
 *  @return timestamp   Timestamp in us, taken from the timebase of the OS (hal timing).
*/
gpPd_TimeStamp_t    gpPd_GetRxTimestamp(gpPd_Handle_t pdHandle);
/** @brief  Return the timestamp of the received packet taken from the chip HW timer.
 *
 *  @param  pdHandle    Handle associated with the packet buffer.
 *  @return timestamp   Timestamp in us, taken from the timer of the chip HW (gpHal timing).
 *                      The timestamp will be equivalent to gpPd_GetRxTimestamp() when running fully embedded (no transceiver setup).
*/
gpPd_TimeStamp_t    gpPd_GetRxTimestampChip(gpPd_Handle_t pdHandle);
/** @brief  Return the channel on which the packet was received.
 *
 *  @param  pdHandle    Handle associated with the packet buffer.
 *  @return rxChannel   Channel on which the packet was received.
 *
*/
UInt8               gpPd_GetRxChannel(gpPd_Handle_t pdHandle);
/** @brief  Return the buffer of BLE phase samples of the received packet.
 *
 *  @param  pdHandle    Handle associated with the packet buffer.
 *  @return pSamples    Pointer to the received phase samples.
 *                      NULL if not available and valid untill free of the pdHandle.
*/
UInt16*             gpPd_GetPhaseSamplesBuffer(gpPd_Handle_t pdHandle);

//TxConfirm
/** @brief  Return the timestamp of a transmitted packet.
 *
 *  @param  pdHandle    Handle associated with the packet.
 *  @return timestamp   Timestamp in us at which the packet was sent, taken from the timebase of the OS (hal timing).
*/
gpPd_TimeStamp_t    gpPd_GetTxTimestamp(gpPd_Handle_t pdHandle);
/** @brief  Return the Link Quality Indication (LQI) of the ACK for the transmitted packet.
 *
 *  @param  pdHandle    Handle associated with the packet.
 *  @return lqi         LQI value of the received ACK - only valid when there was an ACK present.
*/
gpPd_Lqi_t          gpPd_GetTxAckLqi(gpPd_Handle_t pdHandle);
/** @brief  Return the amount of CCA retries of the transmitted packet
 *
 *  @param  pdHandle    Handle associated with the packet.
 *  @return retries     Amount of CCA retries performed.
*/
UInt8               gpPd_GetTxCCACntr(gpPd_Handle_t pdHandle);
/** @brief  Return the amount of MAC retries of the transmitted packet
 *
 *  @param  pdHandle    Handle associated with the packet.
 *  @return retries     Amount of MAC retries. 0 if the first attempt succeeded.
*/
UInt8               gpPd_GetTxRetryCntr(gpPd_Handle_t pdHandle);
/** @brief  Return the framepending bit from the ACK of a transmitted packet.
 *
 *  @param  pdHandle     Handle associated with the packet.
 *  @return framePending True if the frame pending bit was set in the associated ACK.
*/
UInt8               gpPd_GetFramePendingAfterTx(gpPd_Handle_t pdHandle);

//Internal Helper functions
void                gpPd_SetRssi(gpPd_Handle_t pdHandle, gpPd_Rssi_t rssi);
void                gpPd_SetLqi(gpPd_Handle_t pdHandle, gpPd_Lqi_t lqi);
void                gpPd_SetRxTimestamp(gpPd_Handle_t pdHandle, gpPd_TimeStamp_t timestamp);
void                gpPd_SetTxTimestamp(gpPd_Handle_t pdHandle, gpPd_TimeStamp_t timestamp);
void                gpPd_SetTxRetryCntr(gpPd_Handle_t pdHandle, UInt8 txRetryCntr);
void                gpPd_SetFramePendingAfterTx(gpPd_Handle_t pdHandle, UInt8 framePending);
void                gpPd_SetRxChannel(gpPd_Handle_t pdHandle, UInt8 rxChannel);

gpPd_Handle_t       gpPd_CopyPd(gpPd_Handle_t pdHandle);

//Data
UInt8         gpPd_DataRequest(gpPd_Loh_t *p_PdLoh);
void          gpPd_cbDataConfirm(UInt8 pbmHandle, UInt16 pbmOffset, UInt16 pbmLength, gpPd_Loh_t *p_PdLoh);
void          gpPd_DataIndication(UInt8 pbmHandle, UInt16 pbmOffset, UInt16 pbmLength, gpPd_Loh_t *p_PdLoh, gpPd_BufferType_t type);

//Security
UInt8         gpPd_SecRequest(gpPd_Handle_t pdHandle, UInt8 dataOffset, UInt8 dataLength, UInt8 auxOffset, UInt8 auxLength );
gpPd_Handle_t gpPd_cbSecConfirm(UInt8 pbmHandle, UInt8 dataOffset, UInt8 dataLength);

//Purge
UInt8         gpPd_PurgeRequest(gpPd_Handle_t pdHandle);
void          gpPd_cbPurgeConfirm(UInt8 pbmHandle);


#if defined (GP_DIVERSITY_PD_USE_PBM_VARIANT)
#include "gpPd_pbm.h"
#else // defined (GP_DIVERSITY_PD_USE_RAM_VARIANT)
#include "gpPd_ram.h"
#endif //(GP_DIVERSITY_PD_USE_PBM_VARIANT)

//Serialization helper functions
void gpPd_InitPdHandleMapping(void);
Bool gpPd_StorePdHandle(gpPd_Handle_t newPdHandle, gpPd_Handle_t pdHandle);
Bool gpPd_GetStoredPdHandle(gpPd_Handle_t* storedPdHandle, gpPd_Handle_t pdHandle);
gpPd_Handle_t gpPd_RestorePdHandle(gpPd_Handle_t newPdHandle, Bool remove);

#define gpPd_InitUnit()
#define gpPd_GetRealPd(handle)          handle
#define gpPd_GetUtPd(handle)            handle
#define gpPd_FreeRealPd(handle)         gpPd_FreePd(handle)
#define gpPd_FreeUnitPd(realHandle)
#define gpPd_CheckPdFreeUnit()          true

void gpPd_GetCustomPdUnitWithoutCB( gpPd_Handle_t *pUtHandle, gpPd_BufferType_t type, UInt16 size ); // backward compatibility


/* JUMPTABLE_FLASH_FUNCTION_DEFINITIONS_END */

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //defined(GP_DIVERSITY_ROM_CODE)

#endif // _GPPD_H_

