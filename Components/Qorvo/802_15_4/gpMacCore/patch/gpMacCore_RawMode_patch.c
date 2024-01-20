/*
 * Copyright (c) 2017, 2019, Qorvo Inc
 *
 * gpMacCore_patch.c
 *   This file contains patches
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
 * $Header$
 * $Change$
 * $DateTime$
 *
 */

// #define GP_LOCAL_LOG
/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include "gpHal.h"
#include "gpLog.h"
#include "gpJumpTables.h"
#include "gpMacCore.h"
#include "gpMacCore_defs.h"
#include "gpMacCore_defs_Main.h"
#include "gpPoolMem.h"
#if defined(GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR) || defined(GP_MACCORE_DIVERSITY_ASSOCIATION_RECIPIENT) || defined(GP_MACCORE_DIVERSITY_POLL_ORIGINATOR) || defined(GP_MACCORE_DIVERSITY_SCAN_ORPHAN_ORIGINATOR)
#include "gpSched.h"
#endif

// Also add this one, for the case GP_MACCORE_DIVERSITY_SECURITY_ENABLED is set.
gpMacCore_Result_t MacCore_ValidateClearText(MacCore_HeaderDescriptor_t* dataIndicationMacValues);

/*****************************************************************************
 *                    NRT ROM patch fix version numbers
 *****************************************************************************/
#if   defined(GP_DIVERSITY_GPHAL_K8E)
#define ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION 2
#define ROMVERSION_FIXFORPATCH_MACCORE_RAWTX           2
#endif

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID GP_COMPONENT_ID_MACCORE

#define SHORT_ADDR_SIZE  sizeof(UInt16)
#define EXTENDED_ADDR_SIZE sizeof(MACAddress_t)

#define MAC_VS_IE_ID     0x00
#define THREAD_OUI_BYTE0 0x9B
#define THREAD_OUI_BYTE1 0xB8
#define THREAD_OUI_BYTE2 0xEA
#define THREAD_VS_IE_ID_LINKMETRICS  0x00

#define THREAD_LINK_METRIC_PDUCOUNT   0x01
#define THREAD_LINK_METRIC_LQI        0x02
#define THREAD_LINK_METRIC_LINKMARGIN 0x04
#define THREAD_LINK_METRIC_RSSI       0x08

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct MacCore_RxPduMetrics_s {
    UInt32 linkMetricConfiguration;
    Bool  dataCleared;
    UInt8 pduCount;
    UInt8 lqiAvg;
    UInt8 rssiMarginAvg;
    UInt8 rssiAvg;
} MacCore_RxPduMetrics_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

MacCore_RxPduMetrics_t MacCore_RxPduMetrics;

/*****************************************************************************
 *                    ROM Function prototypes
 *****************************************************************************/
#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)
void gpMacCore_DataRequest_orgrom(gpMacCore_AddressMode_t srcAddrMode, gpMacCore_AddressInfo_t* pDstAddrInfo, UInt8 txOptions, gpMacCore_Security_t *pSecOptions, gpMacCore_MultiChannelOptions_t multiChannelOptions, gpPd_Loh_t pdLoh, gpMacCore_StackId_t stackId);
void MacCore_GetRawModeEncryptionOffsets(gpPd_Loh_t currentPdLoh, UInt16* pAuxOffset, UInt16* pDataOffset, UInt16* pCslIeOffset);
void MacCore_HalDataIndication_orgrom(gpPd_Loh_t pdLoh, gpHal_RxInfo_t *rxInfo);
UInt8 MacCore_AnalyseMacHeader_orgrom(gpPd_Loh_t* p_PdLoh, MacCore_HeaderDescriptor_t* pMacHeaderDecoded );
gpMacCore_Result_t MacCore_SendCommand_orgrom(gpMacCore_AddressInfo_t* pDestAddrInfo, gpMacCore_AddressInfo_t* pSrcAddrInfo, UInt8 txOptions, gpMacCore_Security_t *pSecOptions, UInt8 *pData, UInt8 len , gpMacCore_StackId_t stackId, gpHal_MacScenario_t scenario);
gpMacCore_Result_t MacCore_SendCommandBeaconRequest_orgrom(UInt8 channel);
#ifdef GP_MACCORE_DIVERSITY_SCAN_ORPHAN_ORIGINATOR
gpMacCore_Result_t MacCore_SendCommandOrphanNotification_orgrom(UInt8 channel);
#endif //GP_MACCORE_DIVERSITY_SCAN_ORPHAN_ORIGINATOR
void gpMacCore_Init_orgrom(void);
void MacCore_HalDataConfirm_orgrom(gpHal_Result_t status, gpPd_Loh_t pdLoh, UInt8 lastChannelUsed);
#endif

/*****************************************************************************
 *                    Patched Function(s)
 *****************************************************************************/

#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)
void gpMacCore_DataRequest_patched(gpMacCore_AddressMode_t srcAddrMode, gpMacCore_AddressInfo_t* pDstAddrInfo, UInt8 txOptions, gpMacCore_Security_t *pSecOptions, gpMacCore_MultiChannelOptions_t multiChannelOptions, gpPd_Loh_t pdLoh, gpMacCore_StackId_t stackId)
{
    gpHal_DataReqOptions_t  dataReqOptions;
    gpMacCore_Result_t      result = gpMacCore_ResultSuccess;
    gpMacCore_AddressInfo_t srcAddressInfo;
#ifdef GP_MACCORE_DIVERSITY_RAW_FRAMES
#endif //GP_MACCORE_DIVERSITY_RAW_FRAMES

    GP_LOG_PRINTF("Mac DR sId:%u h:%u l:%u txOpt:%x",0, stackId, pdLoh.handle, pdLoh.length, txOptions);

    //Check correct channel setting
    if (((multiChannelOptions.channel[0] != GP_MACCORE_INVALID_CHANNEL) && !GP_MACCORE_CHECK_CHANNEL_VALID(multiChannelOptions.channel[0])) ||
        ((multiChannelOptions.channel[1] != GP_MACCORE_INVALID_CHANNEL) && !GP_MACCORE_CHECK_CHANNEL_VALID(multiChannelOptions.channel[1])) ||
        ((multiChannelOptions.channel[2] != GP_MACCORE_INVALID_CHANNEL) && !GP_MACCORE_CHECK_CHANNEL_VALID(multiChannelOptions.channel[2])))
    {
        GP_LOG_PRINTF("wrong ch: %u %u %u", 0, multiChannelOptions.channel[0], multiChannelOptions.channel[1], multiChannelOptions.channel[2]);
        MacCore_cbDataConfirm(gpMacCore_ResultInvalidParameter, pdLoh.handle);
        return;
    }

    if (GP_MACCORE_TIMEDTX_ENABLED(txOptions))
    {
#ifdef GP_MACCORE_DIVERSITY_TIMEDTX
        if (DIVERSITY_TIMEDTX())
        {
            if (GP_MACCORE_INDIRECT_TRANSMISSION_ENABLED(txOptions))
            {
                GP_LOG_PRINTF("Can not use timed TX with indirect TX", 0);
                MacCore_cbDataConfirm(gpMacCore_ResultInvalidParameter, pdLoh.handle);
                return;
            }

            if (GP_MACCORE_GET_GLOBALS()->MacCore_TimedTx_State != gpMacCore_TimedTxState_Idle)
            {
                GP_LOG_PRINTF("Scheduled TX queue already in use (max 1 packet)", 0);
                MacCore_cbDataConfirm(gpMacCore_ResultTransactionOverflow, pdLoh.handle);
                return;
            }

            // Allocate event descriptor.
            if (GP_MACCORE_GET_GLOBALS()->MacCore_TimedTx_EventId == GPHAL_ES_ABSOLUTE_EVENT_ID_INVALID)
            {
                GP_MACCORE_GET_GLOBALS()->MacCore_TimedTx_EventId = gpHal_GetAbsoluteEvent();
                GP_LOG_PRINTF("timedtx eventid %d", 0, GP_MACCORE_GET_GLOBALS()->MacCore_TimedTx_EventId);
            }
            if (GP_MACCORE_GET_GLOBALS()->MacCore_TimedTx_EventId == GPHAL_ES_ABSOLUTE_EVENT_ID_INVALID)
            {
                // Allocation of event descriptor failed.
                MacCore_cbDataConfirm(gpMacCore_ResultTransactionOverflow, pdLoh.handle);
                return;
            }
        }
#endif //GP_MACCORE_DIVERSITY_TIMEDTX
        if (!DIVERSITY_TIMEDTX())
        {
            GP_LOG_PRINTF("Timed TX not supported", 0);
            MacCore_cbDataConfirm(gpMacCore_ResultInvalidParameter, pdLoh.handle);
            return;
        }
    }

    if (GP_MACCORE_INDIRECT_TRANSMISSION_ENABLED(txOptions))
    {
        //Add to queue and return
#ifdef GP_MACCORE_DIVERSITY_INDIRECT_TRANSMISSION
        if (DIVERSITY_INDIRECT_TRANSMISSION())
        {
            GP_MACCORE_INDIRECT_TRANSMISSION_CLEAR(txOptions);
            if (!MacCore_IndTxAddElement( srcAddrMode, pDstAddrInfo, txOptions, pSecOptions, multiChannelOptions, pdLoh, stackId ))
            {
                MacCore_cbDataConfirm(gpMacCore_ResultTransactionOverflow, pdLoh.handle);
            }
        }
#endif //GP_MACCORE_DIVERSITY_INDIRECT_TRANSMISSION
        if (!DIVERSITY_INDIRECT_TRANSMISSION())
        {
            GP_LOG_PRINTF("IndTx not supported",0);
            MacCore_cbDataConfirm(gpMacCore_ResultInvalidParameter, pdLoh.handle);
        }
        return;
    }

    if (GP_MACCORE_INJECT_ENABLED(txOptions))
    {
#ifdef GP_MACCORE_DIVERSITY_POLL_RECIPIENT
        if (DIVERSITY_POLL_RECIPIENT())
        {
            UInt8 index;

            GP_MACCORE_INJECT_CLEAR(txOptions);
            // Check Indirect Tx queue
            index = MacCore_IndTxBufferFindElemForTx(pDstAddrInfo, stackId);
            if(index != 0xFF)
            {
                txOptions |= GP_MACCORE_TX_OPT_MORE_DATA_PENDING;
            }
        }
#endif //def GP_MACCORE_DIVERSITY_POLL_RECIPIENT
        if (!DIVERSITY_POLL_RECIPIENT())
        {
            GP_LOG_PRINTF("Inject not supported",0);
            MacCore_cbDataConfirm(gpMacCore_ResultInvalidParameter, pdLoh.handle);
            return;
        }
    }

    GP_STAT_SAMPLE_TIME();

    // check parameters
    if(GP_MACCORE_CHECK_IF_ADDRESSMODE_NOT_NONE(srcAddrMode) && ((pDstAddrInfo == NULL) || GP_MACCORE_CHECK_IF_ADDRESSMODE_NOT_NONE(pDstAddrInfo->addressMode)))
    {
        GP_LOG_PRINTF("Inv addr",0);
        MacCore_cbDataConfirm(gpMacCore_ResultInvalidAddress, pdLoh.handle);
        return;
    }

    if(GP_MACCORE_CHECK_IF_ADDRESSMODE_INVALID(srcAddrMode))
    {
        GP_LOG_PRINTF("inv src addr mode",0);
        MacCore_cbDataConfirm(gpMacCore_ResultInvalidParameter, pdLoh.handle);
        return;
    }

    if(GP_MACCORE_CHECK_IF_ADDRESSINFO_INVALID(pDstAddrInfo))
    {
        GP_LOG_PRINTF("Inv dst info",0);
        MacCore_cbDataConfirm(gpMacCore_ResultInvalidParameter, pdLoh.handle);
        return;
    }

    if((multiChannelOptions.channel[0] == GP_MACCORE_INVALID_CHANNEL) &&
        (gpMacCore_GetCurrentChannel(stackId) == GP_MACCORE_INVALID_CHANNEL))
    {
        GP_LOG_PRINTF("MacCore Data req invalid channel",0);
        MacCore_cbDataConfirm(gpMacCore_ResultInvalidParameter, pdLoh.handle);
        return;
    }

#ifdef GP_MACCORE_DIVERSITY_RAW_FRAMES
#endif //def GP_MACCORE_DIVERSITY_RAW_FRAMES

    MacCore_InitSrcAddressInfo(&srcAddressInfo, srcAddrMode, stackId);

#ifdef GP_MACCORE_DIVERSITY_RAW_FRAMES
    /*
     *
     * patch ROM code to add setting of rawEncryptionEnable and the calculation
     * of the aux and data offsets used by the encryption.
     * Included in this patch is also the function to calculate those offsets.
     *
     */
#if defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)
    dataReqOptions.rawEncryptionEnable = false;
    dataReqOptions.rawKeepFrameCounter = false;
#endif // defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)
    GP_LOG_PRINTF("Raw gpMacCore_DataRequest opts=%x",0, txOptions);

    if(DIVERSITY_RAW_FRAMES() && ((txOptions & GP_MACCORE_TX_OPT_RAW) != 0))
    {
        /* the mac header is already present in the higher layer data */
        GP_LOG_PRINTF("Raw gpMacCore_DataRequest st=%d pd=%d",0, stackId, pdLoh.handle);
        GP_MACCORE_GET_GLOBALS_CONST()->MacCore_RawFrameInfoPtr->raw[pdLoh.handle] = true;
#if defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)
        if(GP_MACCORE_NO_SECURITY_SPECIFIED(pSecOptions))
        {
            // pass. No further action needed.
        }
        else
        {
            // set up the encryption config.
            GP_LOG_PRINTF("Raw encryption opt=%04xd",0, (UInt16)((UInt32) pSecOptions & 0xFFFF));
            dataReqOptions.rawEncryptionEnable = true;
            if(txOptions & GP_MACCORE_TX_OPT_RAW_KEEP_FRAMECOUNTER)
            {
                dataReqOptions.rawKeepFrameCounter = true;
            }
            MacCore_GetRawModeEncryptionOffsets(pdLoh, &dataReqOptions.rawAuxOffset, &dataReqOptions.rawDataOffset, &dataReqOptions.rawCslIeOffset);
        }
#endif // defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)
    }
    else
    {
        GP_LOG_PRINTF("Norm1 gpMacCore_DataRequest st=%d pd=%d",0, stackId, pdLoh.handle);
        GP_MACCORE_GET_GLOBALS_CONST()->MacCore_RawFrameInfoPtr->raw[pdLoh.handle] = false;
#else
    {
        GP_LOG_PRINTF("Norm2 gpMacCore_DataRequest st=%d pd=%d",0, stackId, pdLoh.handle);
#endif //GP_MACCORE_DIVERSITY_RAW_FRAMES
        // can also be optimized by converting to assert if
        if(GP_MACCORE_NO_SECURITY_SPECIFIED(pSecOptions))
        {
            // no security ==> only write normal MAC header
            MacCore_WriteMacHeaderInPd(gpMacCore_FrameTypeData, &srcAddressInfo, pDstAddrInfo, txOptions, gpEncryption_SecLevelNothing, &pdLoh , stackId );
            if(pdLoh.length > GP_MACCORE_MAX_PHY_PACKET_SIZE_NO_FCS)
            {
                // If any parameter in the MCPS-DATA.request primitive is not supported or is out of range, the MAC sublayer will issue the MCPS-DATA.confirm primitive with a status of INVALID_PARAMETER.
                result = gpMacCore_ResultInvalidParameter;
            }
        }
        else
        {
#ifdef GP_MACCORE_DIVERSITY_SECURITY_ENABLED
            GP_ASSERT_DEV_EXT(false); // ROM doesn't support normal MAC security (only in Raw mode via this patch)
#else
            result = gpMacCore_ResultUnsupportedSecurity;
#endif // GP_MACCORE_DIVERSITY_SECURITY_ENABLED
        }
    }

    if(result != gpMacCore_ResultSuccess)
    {
        GP_LOG_PRINTF("MacCore Data req unsuccessful MAC result %x",0, result);
        MacCore_cbDataConfirm(result, pdLoh.handle);
        return;
    }

    if( multiChannelOptions.channel[0] == GP_MACCORE_INVALID_CHANNEL)
    {
        multiChannelOptions.channel[0] = gpMacCore_GetCurrentChannel(stackId);
        multiChannelOptions.channel[1] = GP_MACCORE_INVALID_CHANNEL;
        multiChannelOptions.channel[2] = GP_MACCORE_INVALID_CHANNEL;
    }

    // set channels
    gpPad_SetTxChannels(MacCore_GetPad(stackId), multiChannelOptions.channel);

    dataReqOptions.macScenario = GP_MACCORE_TIMEDTX_ENABLED(txOptions) ? gpHal_MacTimedTx : gpHal_MacDefault;
    dataReqOptions.srcId = stackId;

#ifdef GP_MACCORE_DIVERSITY_RAW_FRAMES
#endif //GP_MACCORE_DIVERSITY_RAW_FRAMES

    {
        result = MacCore_TxDataRequest(&dataReqOptions, pdLoh, stackId);
    }
    if(result != gpHal_ResultSuccess)
    {
        GP_LOG_PRINTF("MacCore Data req unsuccessful HAL result %x",2, result);
        MacCore_cbDataConfirm(result, pdLoh.handle);
        return;
    }

#ifdef GP_MACCORE_DIVERSITY_TIMEDTX
    if (DIVERSITY_TIMEDTX())
    {
        if (GP_MACCORE_TIMEDTX_ENABLED(txOptions))
        {
            // Mark scheduled TX queue busy.
            GP_MACCORE_GET_GLOBALS()->MacCore_TimedTx_PdHandle = pdLoh.handle;
            GP_MACCORE_GET_GLOBALS()->MacCore_TimedTx_State = gpMacCore_TimedTxState_Queued;
        }
    }
#endif //GP_MACCORE_DIVERSITY_TIMEDTX

}

void MacCore_GetRawModeEncryptionOffsets(gpPd_Loh_t currentPdLoh, UInt16* pAuxOffset, UInt16* pDataOffset, UInt16* pCslIeOffset)
{
    UInt16 frameControl;
    UInt8 currentOffset = 0;

    gpPd_ReadByteStream(currentPdLoh.handle, currentPdLoh.offset, GP_MACCORE_FC_SIZE, (UInt8*) &(frameControl));
    currentOffset += GP_MACCORE_FC_SIZE;
    currentOffset += GP_MACCORE_SN_SIZE;

    if(MACCORE_FRAMECONTROL_DSTADDRMODE_GET(frameControl) == gpMacCore_AddressModeShortAddress)
    {
        currentOffset += GP_MACCORE_SHORT_ADDR_SIZE + GP_MACCORE_PANID_SIZE;
    }
    if(MACCORE_FRAMECONTROL_SRCADDRMODE_GET(frameControl) == gpMacCore_AddressModeShortAddress)
    {
        currentOffset += GP_MACCORE_SHORT_ADDR_SIZE + GP_MACCORE_PANID_SIZE;
    }
    if(MACCORE_FRAMECONTROL_DSTADDRMODE_GET(frameControl) == gpMacCore_AddressModeExtendedAddress)
    {
        currentOffset += GP_MACCORE_EXT_ADDR_SIZE + GP_MACCORE_PANID_SIZE;
    }
    if(MACCORE_FRAMECONTROL_SRCADDRMODE_GET(frameControl) == gpMacCore_AddressModeExtendedAddress)
    {
        currentOffset += GP_MACCORE_EXT_ADDR_SIZE + GP_MACCORE_PANID_SIZE;
    }

    if( (MACCORE_FRAMECONTROL_FRAMEVERSION_GET(frameControl) == gpMacCore_MacVersion2003) ||
        (MACCORE_FRAMECONTROL_FRAMEVERSION_GET(frameControl) == gpMacCore_MacVersion2006) )
    {
        if(MACCORE_FRAMECONTROL_PANCOMPRESSION_GET(frameControl) == 0x01)
        {
            currentOffset -= GP_MACCORE_PANID_SIZE; /* subtract the size of one of the two panids again */
        }
    }
    else if (MACCORE_FRAMECONTROL_FRAMEVERSION_GET(frameControl) == gpMacCore_MacVersion2015)
    {
        // if no addressing information is present, the pan_id_compression field indicates wheter there is a
        // dst pan_id included
        if((MACCORE_FRAMECONTROL_DSTADDRMODE_GET(frameControl) == gpMacCore_AddressModeNoAddress) &&
           (MACCORE_FRAMECONTROL_SRCADDRMODE_GET(frameControl) == gpMacCore_AddressModeNoAddress) )
        {
            if(MACCORE_FRAMECONTROL_PANCOMPRESSION_GET(frameControl) == 0x01)
            {
                currentOffset += GP_MACCORE_PANID_SIZE; /* add a panid without any other address info */
            }
        }
        // If only either the dst or the src addressing is present, the pan_id_compression field indicates
        // wheter the pan_id of the respective address is NOT present
        else if( ( (MACCORE_FRAMECONTROL_DSTADDRMODE_GET(frameControl) != gpMacCore_AddressModeNoAddress) &&
                   (MACCORE_FRAMECONTROL_SRCADDRMODE_GET(frameControl) == gpMacCore_AddressModeNoAddress) ) ||
                 ( (MACCORE_FRAMECONTROL_DSTADDRMODE_GET(frameControl) == gpMacCore_AddressModeNoAddress) &&
                   (MACCORE_FRAMECONTROL_SRCADDRMODE_GET(frameControl) != gpMacCore_AddressModeNoAddress) ) )
        {
            if(MACCORE_FRAMECONTROL_PANCOMPRESSION_GET(frameControl) == 0x01)
            {
                currentOffset -= GP_MACCORE_PANID_SIZE; /* subtract the single panid size again */
            }
        }
        // if both addresses are long addresses, src pan_id is never sent. And the dst pan_id is present only
        // when the pan_id_compression is 0
        else if((MACCORE_FRAMECONTROL_DSTADDRMODE_GET(frameControl) == gpMacCore_AddressModeExtendedAddress) &&
                (MACCORE_FRAMECONTROL_SRCADDRMODE_GET(frameControl) == gpMacCore_AddressModeExtendedAddress) )
        {
            currentOffset -= GP_MACCORE_PANID_SIZE;     /* only 1 panid is added, so subtract the previous addition*/
            if(MACCORE_FRAMECONTROL_PANCOMPRESSION_GET(frameControl) == 0x01)
            {
                currentOffset -= GP_MACCORE_PANID_SIZE; /* in this case also remove the 2nd panid size */
            }
        }
        // in all other cases, the pan_id_compression field indicates wheter the src pan_id is present or not
        else
        {
            if(MACCORE_FRAMECONTROL_PANCOMPRESSION_GET(frameControl) == 0x01)
            {
                currentOffset -= GP_MACCORE_PANID_SIZE; /* subtract the size of one of the two panids again */
            }
        }
    }
    else
    {
        GP_ASSERT_DEV_EXT(false); /* unknown frame version */
    }

    // store offset to the Aux header in the frame.
    *pAuxOffset = currentPdLoh.offset + currentOffset;

    if(MACCORE_FRAMECONTROL_SECURITY_GET(frameControl))
    {
        UInt8 securityControl;
        UInt8 keyIdMode;
        securityControl = gpPd_ReadByte(currentPdLoh.handle, currentPdLoh.offset + currentOffset);
        currentOffset += sizeof(UInt8);  // security control
        currentOffset += sizeof(UInt32); // framecounter
        keyIdMode = MACCORE_SECCONTROL_KEYIDMODE_GET(securityControl);

        switch(keyIdMode)
        {
            case gpMacCore_KeyIdModeImplicit:       {currentOffset += 0; break;}
            case gpMacCore_KeyIdModeExplicit1Octet: {currentOffset += 1; break;}
            case gpMacCore_KeyIdModeExplicit4Octet: {currentOffset += 5; break;}
            case gpMacCore_KeyIdModeExplicit8Octet: {currentOffset += 9; break;}
            default:                                {GP_ASSERT_DEV_INT(0); break;}
        }
    }

    /* skip past the ie header fields to get to the to-be-encrypted data. */
    if(MACCORE_FRAMECONTROL_IE_PRESENT_GET(frameControl))
    {
        UInt16 ie_header;
        UInt8 ie_length, ie_type, ie_id;

        while(currentOffset + sizeof(UInt16) < currentPdLoh.length)
        {
            gpPd_ReadByteStream(currentPdLoh.handle, currentPdLoh.offset + currentOffset, sizeof(UInt16), (UInt8*)&ie_header);
            ie_length = MACCORE_IEHEADER_LENGTH_GET(ie_header);
            ie_id     = MACCORE_IEHEADER_ELEMENTID_GET(ie_header);
            ie_type   = MACCORE_IEHEADER_TYPE_GET(ie_header);

            if((ie_type == MACCORE_IE_TYPE_HEADER) && (ie_id == MACCORE_IE_ID_CSL))
            {
                /* remember the CLS IE offset */
                *pCslIeOffset = currentPdLoh.offset + currentOffset;
            }

            currentOffset += sizeof(UInt16);
            currentOffset += ie_length;

            if((ie_type == MACCORE_IE_TYPE_HEADER) && ((ie_id == MACCORE_IE_ID_HT1) || (ie_id == MACCORE_IE_ID_HT2)))
            {
                /* we're done with the header IEs. */
                /* exit the loop */
                break;
            }
        }

    }

    /* check if it's a command */
    if(MACCORE_FRAMECONTROL_FRAMETYPE_GET(frameControl) == 0x3)
    {
        // store offset to the to-be-encrypted data in the frame.
        *pDataOffset = currentPdLoh.offset + currentOffset + 1;
    }
    else
    {
        // store offset to the to-be-encrypted data in the frame.
        *pDataOffset = currentPdLoh.offset + currentOffset;
    }
}

#if defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)
STATIC_FUNC void MacCore_UpdateEnhAckVsIe(void)
{
        UInt8 vsIe[8];
        UInt8 index = 1; /* skip the length field now and complete it at the end */
        vsIe[index++] = MAC_VS_IE_ID;
        vsIe[index++] = THREAD_OUI_BYTE0;
        vsIe[index++] = THREAD_OUI_BYTE1;
        vsIe[index++] = THREAD_OUI_BYTE2;
        vsIe[index++] = THREAD_VS_IE_ID_LINKMETRICS;


        if(MacCore_RxPduMetrics.linkMetricConfiguration & THREAD_LINK_METRIC_PDUCOUNT)
        {
            vsIe[index] = MacCore_RxPduMetrics.pduCount;
            index++;
        }
        if(MacCore_RxPduMetrics.linkMetricConfiguration & THREAD_LINK_METRIC_LQI)
        {
            vsIe[index] = MacCore_RxPduMetrics.lqiAvg;
            index++;
        }
        if(MacCore_RxPduMetrics.linkMetricConfiguration & THREAD_LINK_METRIC_RSSI)
        {
            vsIe[index] = MacCore_RxPduMetrics.rssiAvg;
            index++;
        }
        if(MacCore_RxPduMetrics.linkMetricConfiguration & THREAD_LINK_METRIC_LINKMARGIN)
        {
            vsIe[index] = MacCore_RxPduMetrics.rssiMarginAvg;
            index++;
        }

#ifdef GP_LOCAL_LOG
        GP_LOG_SYSTEM_PRINTF("setting vsie len %d config=0x%x", 0, index, (UInt16)(MacCore_RxPduMetrics.linkMetricConfiguration&0xFFFF));
        gpLog_PrintBuffer(index, vsIe);
#endif // GP_LOCAL_LOG

        GP_ASSERT_DEV_INT(index < 9);
        vsIe[0] = index - 2; // subtract the length of the IE header */

        gpHal_SetEnhAckVSIE(index, &vsIe[0]);
}

STATIC_FUNC void MacCore_DisableEnhAckVsIe(void)
{
        gpHal_SetEnhAckVSIE(0, 0);
}

STATIC_FUNC void MacCore_UpdateMetricWeight8(UInt8 *pMetric, UInt8 newSample, Bool dataCleared)
{
    if(dataCleared)
    {
        *pMetric = newSample;
    }
    else
    {
        *pMetric = (UInt8)((((UInt16)(*pMetric)) * 7 + (UInt16)newSample + 4 /* rounding */)/8);
    }
}


STATIC_FUNC UInt8 MacCore_ScaleToUint8(Int16 value, Int16 expectedMinValue, Int16 expectedMaxValue)
{
    UInt8 ret = 0;

    if(value <= expectedMinValue)
    {
        ret = 0;
    }
    else if(value >= expectedMaxValue)
    {
        ret = UINT8_MAX;
    }
    else
    {
        /* Scale the value to use the complete range of a UInt8, for values between the epxected min and max.
         * This formula only works for the case where a small range is expanded to a larger range.
         * This is valid for the relevant use-cases: 0..130 and -130..0 => expand to 0..255.
         * If ever this is not valid anymore, rounding may need to be added to ensure equal usage off all values
         * in the final range.
         */
        ret = (UInt8) ((value - expectedMinValue) * UINT8_MAX / (expectedMaxValue - expectedMinValue));
    }
    GP_LOG_PRINTF("val: %d ret: %d emin: %d emax: %d", 0, value, ret, expectedMinValue, expectedMaxValue);
    return ret;
}

void MacCore_StoreLinkMetrics(MacCore_HeaderDescriptor_t *pmdi, gpPd_Loh_t pdLoh)
{
    if(MacCore_RxPduMetrics.linkMetricConfiguration > 0)
    {
        Int8 signed_rssi, signed_sens_level;
        UInt8 rssi, lqi, rssiMargin;

        MacCore_RxPduMetrics.pduCount++;

        lqi = gpPd_GetLqi(pdLoh.handle);
        MacCore_UpdateMetricWeight8(&(MacCore_RxPduMetrics.lqiAvg), lqi, MacCore_RxPduMetrics.dataCleared);

        signed_rssi = gpPd_GetRssi(pdLoh.handle);
        /* Thread spec v1.2.1RC2 paragraph 4.11.3.4.3 Link Metrics Data and Sub-TLV Formats */
        rssi = MacCore_ScaleToUint8(signed_rssi, -130, 0);
        MacCore_UpdateMetricWeight8(&(MacCore_RxPduMetrics.rssiAvg),  rssi, MacCore_RxPduMetrics.dataCleared);

        signed_sens_level = gpHal_GetSensitivityLevel();
        if(signed_rssi > signed_sens_level)
        {
            GP_LOG_PRINTF("rssi %d sens %d", 0, signed_rssi, signed_sens_level);
            /* Thread spec v1.2.1RC2 paragraph 4.11.3.4.3 Link Metrics Data and Sub-TLV Formats */
            rssiMargin = MacCore_ScaleToUint8(signed_rssi - signed_sens_level, 0, 130);
        }
        else
        {
            rssiMargin = 0;
        }
        MacCore_UpdateMetricWeight8(&(MacCore_RxPduMetrics.rssiMarginAvg), rssiMargin, MacCore_RxPduMetrics.dataCleared);

        MacCore_UpdateEnhAckVsIe();

        /* now we don't have an empty measurement set anymore */
        MacCore_RxPduMetrics.dataCleared = false;
    }
}
#endif // defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)

gpMacCore_Result_t gpMacCore_ConfigureEnhAckProbing_STACKID(UInt8 linkMetrics, MACAddress_t* pExtendedAddress, UInt16 shortAddress MACCORE_STACKID_ARG_2)
{
#if defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)
    /* perform some checks on the validity of the linkMetrics configuration */
    UInt8 supportedMetrics = THREAD_LINK_METRIC_PDUCOUNT | THREAD_LINK_METRIC_LQI | THREAD_LINK_METRIC_RSSI | THREAD_LINK_METRIC_LINKMARGIN;
    UInt8 index, metricCount;

    if(linkMetrics & (~supportedMetrics))
    {
        GP_LOG_PRINTF("unsupported metrics: 0x%x", 0, linkMetrics);
        return gpMacCore_ResultInvalidParameter;
    }

    /* count the requested metrics */
    metricCount = 0;
    for(index = 0; index < 8*sizeof(UInt8); index++)
    {
        if((linkMetrics >> index) & 0x01)
        {
            metricCount++;
        }
    }

    if(metricCount > 2)
    {
        GP_LOG_PRINTF("too many metrics: 0x%x", 0, linkMetrics);
        return gpMacCore_ResultInvalidParameter;
    }

    MacCore_RxPduMetrics.linkMetricConfiguration = linkMetrics;
    MacCore_RxPduMetrics.pduCount = 0;
    MacCore_RxPduMetrics.lqiAvg = 0;
    MacCore_RxPduMetrics.rssiMarginAvg = 0;
    MacCore_RxPduMetrics.rssiAvg = 0;

    /* do not perform the averaging when the configuration has changed and the data set is cleared */
    MacCore_RxPduMetrics.dataCleared = true;

    if(linkMetrics>0)
    {
        MacCore_UpdateEnhAckVsIe();
    }
    else
    {
        MacCore_DisableEnhAckVsIe();
    }

    return gpMacCore_ResultSuccess;
#else
    GP_LOG_PRINTF("Link metrics not enabled", 0);
    return gpMacCore_ResultInvalidParameter;
#endif // defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)
}

#ifdef GP_ROM_PATCHED_MacCore_AnalyseMacHeader
static gpMacCore_Result_t MacCore_ValidAddrModes(gpMacCore_AddressMode_t srcAddrMode, gpMacCore_AddressMode_t dstAddrMode, gpMacCore_FrameType_t frameType)
{
    //
    // The check on the frame type is only addition compared to the original rom code:
    //
    if((srcAddrMode == gpMacCore_AddressModeNoAddress) && (dstAddrMode == gpMacCore_AddressModeNoAddress) && (frameType != gpMacCore_FrameTypeAcknowledge))
    {
        return gpMacCore_ResultInvalidAddress;
    }

    if( (srcAddrMode == gpMacCore_AddressModeReserved || srcAddrMode > gpMacCore_AddressModeExtendedAddress) ||
        (dstAddrMode == gpMacCore_AddressModeReserved || dstAddrMode > gpMacCore_AddressModeExtendedAddress))
    {
        return gpMacCore_ResultInvalidParameter;
    }

    return gpMacCore_ResultSuccess;
}
#endif //GP_ROM_PATCHED_MacCore_AnalyseMacHeader

#ifdef GP_ROM_PATCHED_MacCore_AnalyseMacHeader
UInt8 MacCore_AnalyseMacHeader_patched(gpPd_Loh_t* p_PdLoh, MacCore_HeaderDescriptor_t* pMacHeaderDecoded )
{
    UInt8 startMacHdr = p_PdLoh->offset;
    UInt8 packetLength = p_PdLoh->length;

    // read frame control
    MacCore_ReadStreamAndUpdatePd((UInt8*)&pMacHeaderDecoded->frameControl, 2, p_PdLoh);
    // read sequence number
    gpPd_ReadWithUpdate(p_PdLoh, 1, &pMacHeaderDecoded->sequenceNumber);
    // get frametype
    pMacHeaderDecoded->frameType = (gpMacCore_FrameType_t)MACCORE_FRAMECONTROL_FRAMETYPE_GET(pMacHeaderDecoded->frameControl);
    // get destination address mode
    pMacHeaderDecoded->dstAddrInfo.addressMode = (gpMacCore_AddressMode_t)MACCORE_FRAMECONTROL_DSTADDRMODE_GET(pMacHeaderDecoded->frameControl);
    // get source addressing mode
    pMacHeaderDecoded->srcAddrInfo.addressMode = (gpMacCore_AddressMode_t)MACCORE_FRAMECONTROL_SRCADDRMODE_GET(pMacHeaderDecoded->frameControl);

    if( MacCore_ValidAddrModes(pMacHeaderDecoded->srcAddrInfo.addressMode, pMacHeaderDecoded->dstAddrInfo.addressMode, pMacHeaderDecoded->frameType) != gpMacCore_ResultSuccess )
    {
        GP_LOG_PRINTF("Invalid header seen",0);
        return GP_MACCORE_INVALID_HEADER_LENGTH;
    }

    Bool dstPanIdPresent, srcPanIdPresent;

    if(MACCORE_FRAMECONTROL_FRAMEVERSION_GET(pMacHeaderDecoded->frameControl) < gpMacCore_MacVersion2015)
    {
        dstPanIdPresent = (pMacHeaderDecoded->dstAddrInfo.addressMode >= gpMacCore_AddressModeShortAddress);
        srcPanIdPresent = (pMacHeaderDecoded->srcAddrInfo.addressMode >= gpMacCore_AddressModeShortAddress);
        if( (pMacHeaderDecoded->dstAddrInfo.addressMode >= gpMacCore_AddressModeShortAddress) &&
            (pMacHeaderDecoded->srcAddrInfo.addressMode >= gpMacCore_AddressModeShortAddress)
          )
        {
            srcPanIdPresent = !(MACCORE_FRAMECONTROL_PANCOMPRESSION_GET(pMacHeaderDecoded->frameControl));
        }
    }
    else
    {
        dstPanIdPresent = (pMacHeaderDecoded->dstAddrInfo.addressMode >= gpMacCore_AddressModeShortAddress);
        srcPanIdPresent = (pMacHeaderDecoded->srcAddrInfo.addressMode >= gpMacCore_AddressModeShortAddress);
        if( (pMacHeaderDecoded->dstAddrInfo.addressMode == gpMacCore_AddressModeNoAddress) &&
            (pMacHeaderDecoded->srcAddrInfo.addressMode == gpMacCore_AddressModeNoAddress)
          )
        {
            dstPanIdPresent = (MACCORE_FRAMECONTROL_PANCOMPRESSION_GET(pMacHeaderDecoded->frameControl));
        }
        if( (pMacHeaderDecoded->dstAddrInfo.addressMode >= gpMacCore_AddressModeShortAddress) &&
            (pMacHeaderDecoded->srcAddrInfo.addressMode == gpMacCore_AddressModeNoAddress)
          )
        {
            dstPanIdPresent = !(MACCORE_FRAMECONTROL_PANCOMPRESSION_GET(pMacHeaderDecoded->frameControl));
        }
        if( (pMacHeaderDecoded->dstAddrInfo.addressMode == gpMacCore_AddressModeNoAddress) &&
            (pMacHeaderDecoded->srcAddrInfo.addressMode >= gpMacCore_AddressModeShortAddress)
          )
        {
            srcPanIdPresent = !(MACCORE_FRAMECONTROL_PANCOMPRESSION_GET(pMacHeaderDecoded->frameControl));
        }
        if( (pMacHeaderDecoded->dstAddrInfo.addressMode == gpMacCore_AddressModeExtendedAddress) &&
            (pMacHeaderDecoded->srcAddrInfo.addressMode == gpMacCore_AddressModeExtendedAddress)
          )
        {
            dstPanIdPresent = !(MACCORE_FRAMECONTROL_PANCOMPRESSION_GET(pMacHeaderDecoded->frameControl));
            srcPanIdPresent = false;
        }
        else
        {
            srcPanIdPresent = !(MACCORE_FRAMECONTROL_PANCOMPRESSION_GET(pMacHeaderDecoded->frameControl));
        }
    }

    if(pMacHeaderDecoded->dstAddrInfo.addressMode >= gpMacCore_AddressModeShortAddress)
    {
        if(dstPanIdPresent)
        {
            // Get PAN ID
            MacCore_ReadStreamAndUpdatePd((UInt8*)&pMacHeaderDecoded->dstAddrInfo.panId, 2, p_PdLoh);
        }
        // Get address
        if (pMacHeaderDecoded->dstAddrInfo.addressMode == gpMacCore_AddressModeExtendedAddress )
        {
            // Get extended address
            MacCore_ReadStreamAndUpdatePd((UInt8*)&pMacHeaderDecoded->dstAddrInfo.address.Extended, EXTENDED_ADDR_SIZE, p_PdLoh);
        }
        else
        {
            // Get short address
            MacCore_ReadStreamAndUpdatePd((UInt8*)&pMacHeaderDecoded->dstAddrInfo.address.Short, SHORT_ADDR_SIZE, p_PdLoh);
        }
    }

    if (pMacHeaderDecoded->srcAddrInfo.addressMode >= gpMacCore_AddressModeShortAddress )
    {
        if(srcPanIdPresent)
        {
            // Get PAN ID
            MacCore_ReadStreamAndUpdatePd((UInt8*)&pMacHeaderDecoded->srcAddrInfo.panId, 2, p_PdLoh);
        }
        else
        {
            pMacHeaderDecoded->srcAddrInfo.panId = pMacHeaderDecoded->dstAddrInfo.panId;
        }

        // Get address
        if (pMacHeaderDecoded->srcAddrInfo.addressMode == gpMacCore_AddressModeExtendedAddress)
        {
            // Get extended address
            MacCore_ReadStreamAndUpdatePd((UInt8*)&pMacHeaderDecoded->srcAddrInfo.address.Extended, EXTENDED_ADDR_SIZE, p_PdLoh);
        }
        else
        {
            // Get short address
            MacCore_ReadStreamAndUpdatePd((UInt8*)&pMacHeaderDecoded->srcAddrInfo.address.Short, SHORT_ADDR_SIZE, p_PdLoh);
        }
    }

    // Get security properties
    if(MACCORE_FRAMECONTROL_SECURITY_GET(pMacHeaderDecoded->frameControl))
    {
        // get security control
        UInt8 secControl;

        gpPd_ReadWithUpdate(p_PdLoh, 1, &secControl);
        pMacHeaderDecoded->secOptions.securityLevel =MACCORE_SECCONTROL_SECLEVEL_GET(secControl);
        pMacHeaderDecoded->secOptions.keyIdMode =MACCORE_SECCONTROL_KEYIDMODE_GET(secControl);
        // get framecounter
        MacCore_ReadStreamAndUpdatePd((UInt8*)&pMacHeaderDecoded->frameCounter, 4, p_PdLoh);
        switch(pMacHeaderDecoded->secOptions.keyIdMode)
        {
            case gpMacCore_KeyIdModeExplicit1Octet:
            {
                break;
            }
            case gpMacCore_KeyIdModeExplicit4Octet:
            {
                // get key source
                MacCore_ReadStreamAndUpdatePd(pMacHeaderDecoded->secOptions.pKeySource.pKeySource4, 4, p_PdLoh);
                break;
            }
            case gpMacCore_KeyIdModeExplicit8Octet:
            {
                // get key source
                MacCore_ReadStreamAndUpdatePd(pMacHeaderDecoded->secOptions.pKeySource.pKeySource8, 8, p_PdLoh);
                break;
            }
            case gpMacCore_KeyIdModeImplicit:
            {
                // do nothing
                break;
            }
            default:
            {
                GP_LOG_PRINTF("Invalid header keyIdMode: %x",0,pMacHeaderDecoded->secOptions.keyIdMode);
                return GP_MACCORE_INVALID_HEADER_LENGTH;
            }
        }
        if(pMacHeaderDecoded->secOptions.keyIdMode != gpMacCore_KeyIdModeImplicit)
        {
            // get key index
            gpPd_ReadWithUpdate(p_PdLoh, 1, &pMacHeaderDecoded->secOptions.keyIndex);
        }
    }
    else
    {
        pMacHeaderDecoded->secOptions.keyIndex = 0;
    }
    if( packetLength < p_PdLoh->offset - startMacHdr )
    {
        GP_LOG_PRINTF("Invalid header length: %i < %i-%i",0,packetLength, p_PdLoh->offset, startMacHdr);
        return GP_MACCORE_INVALID_HEADER_LENGTH;
    }
    return p_PdLoh->offset - startMacHdr;

}
#endif //GP_ROM_PATCHED_MacCore_AnalyseMacHeader

void MacCore_HalDataIndication_patched(gpPd_Loh_t pdLoh, gpHal_RxInfo_t *rxInfo)
{
    MacCore_HeaderDescriptor_t mdi;
    UInt8 macHeaderLength;
#ifdef GP_MACCORE_DIVERSITY_RAW_FRAMES
    gpPd_Loh_t pdLoh_backup = pdLoh;
#endif


    // analyze the MAC header and check if it contains invalid data in framecontrol
    MEMSET((UInt8*)&mdi, 0, sizeof(MacCore_HeaderDescriptor_t));

    macHeaderLength = MacCore_AnalyseMacHeader(&pdLoh, &mdi);
    mdi.stackId = MacCore_GetStackId(&mdi.dstAddrInfo);


    if(macHeaderLength == GP_MACCORE_INVALID_HEADER_LENGTH)
    {
        gpPd_FreePd(pdLoh.handle);
        return;
    }

    //
    // Below are the additional/updated code lines when compared to the original rom code:
    //

    //fixed for inderect tx,not yet for assoc, when reworking assoc, fix this. (merge GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs and GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs
    // drop packets which are received on another channel than the stackchannel.
    // drop packet if a packet is received even though the receiver for the stack was not on.
    if( mdi.stackId != GP_MACCORE_STACK_UNDEFINED )
    {
        if( (!MacCore_RxForThisStack(mdi.stackId,mdi.dstAddrInfo.addressMode,rxInfo->rxChannel)) &&
            (mdi.frameType != gpMacCore_FrameTypeAcknowledge) ) // ACK frames can be received if RxOnWhenIdle is not set.
        {
            GP_LOG_PRINTF("drop %i %i %i %i",0,mdi.stackId,gpRxArbiter_GetStackRxOn(mdi.stackId) , gpRxArbiter_GetStackChannel(mdi.stackId), gpRxArbiter_GetCurrentRxChannel() );
            gpPd_FreePd(pdLoh.handle);
            return;
        }
    }

#ifdef GP_MACCORE_DIVERSITY_RAW_FRAMES
    if (DIVERSITY_RAW_FRAMES())
    {
        // Check if we're receiving a standard ACK frame. If so, drop it.
        if(mdi.stackId == GP_MACCORE_STACK_UNDEFINED)
        {
            if((mdi.frameType == gpMacCore_FrameTypeAcknowledge) && (MACCORE_FRAMECONTROL_FRAMEVERSION_GET(mdi.frameControl) != gpMacCore_MacVersion2015))
            {
                // drop non-enhanced ACK frame
                gpPd_FreePd( pdLoh.handle );
                return;
            }
        }
    }
#endif //GP_MACCORE_DIVERSITY_RAW_FRAMES

#ifdef GP_MACCORE_DIVERSITY_RAW_FRAMES
    if (DIVERSITY_RAW_FRAMES())
    {
        // if the mdi.stackId is GP_MACCORE_STACK_UNDEFINED or is equal to the OpenThread stackId
        // then send a copy to the openthread stack

        // if the mdi.stackId is GP_MACCORE_STACK_UNDEFINED or is equal to any other stackid
        // then send a copy to the all stacks, except the OpenThread stack !!!!

        if(mdi.stackId == GP_MACCORE_STACK_UNDEFINED)
        {
            UInt8 stackid = MacCoreGetRawStack();
            // use the backup pdloh to be able to return the frame with the mac header indcluded.
            if((mdi.frameType == gpMacCore_FrameTypeAcknowledge) && (MACCORE_FRAMECONTROL_FRAMEVERSION_GET(mdi.frameControl) != gpMacCore_MacVersion2015))
            {
                // drop non-enhanced ACK frame
                gpPd_FreePd( pdLoh.handle );
                return;
            }

            if(stackid != GP_MACCORE_STACK_UNDEFINED)
            {
                pdLoh_backup.handle = gpPd_CopyPd( pdLoh.handle );
                GP_LOG_PRINTF("Raw1 gpMacCore_cbDataIndication st=%d seq=%d",0, stackid, mdi.sequenceNumber);
#if defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)
                MacCore_StoreLinkMetrics(&mdi, pdLoh_backup);
#endif // defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)
                gpMacCore_cbDataIndication(&mdi.srcAddrInfo, &mdi.dstAddrInfo,mdi.sequenceNumber, &mdi.secOptions, pdLoh_backup, stackid);
            }
        }
        if((mdi.stackId != GP_MACCORE_STACK_UNDEFINED) && (gpMacCore_GetBeaconPayloadLength(mdi.stackId) == 0xFF))
        {
            //GP_LOG_SYSTEM_PRINTF("Raw2 gpMacCore_cbDataIndication st=%d seq=%d",0, mdi.stackId, mdi.sequenceNumber);
#if defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)
            MacCore_StoreLinkMetrics(&mdi, pdLoh_backup);
#endif // defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)
            gpMacCore_cbDataIndication(&mdi.srcAddrInfo, &mdi.dstAddrInfo,mdi.sequenceNumber, &mdi.secOptions, pdLoh_backup, mdi.stackId);
            return;
        }
    }
#endif //GP_MACCORE_DIVERSITY_RAW_FRAMES

    switch(mdi.frameType)
    {
#if defined(GP_MACCORE_DIVERSITY_SCAN_ACTIVE_ORIGINATOR)
        case gpMacCore_FrameTypeBeacon:
        {
            if (DIVERSITY_SCAN_ACTIVE_ORIGINATOR())
            {
                MacCore_ProcessBeacon(pdLoh, &mdi, rxInfo);
            }
            gpPd_FreePd( pdLoh.handle );
            break;
        }
#endif // GP_MACCORE_DIVERSITY_SCAN_ACTIVE_ORIGINATOR
        case gpMacCore_FrameTypeData:
        {
            MacCore_ProcessData(pdLoh, &mdi,macHeaderLength, rxInfo);
            break;
        }
        case gpMacCore_FrameTypeCommand:
        {
            UInt8 result;
// ROM doesn't support normal MAC security (only in Raw mode via this patch)
// #ifdef GP_MACCORE_DIVERSITY_SECURITY_ENABLED
//             GP_ASSERT_DEV_EXT(false);
// #else
            result = MacCore_ValidateClearText(&mdi);
//#endif //GP_MACCORE_DIVERSITY_SECURITY_ENABLED
            if(result != gpMacCore_ResultSuccess)
            {
                GP_LOG_SYSTEM_PRINTF("dec fail:%x",0,result);
                gpMacCore_cbSecurityFailureCommStatusIndication(&mdi.srcAddrInfo, &mdi.dstAddrInfo, result, mdi.stackId, gpPd_GetRxTimestamp(pdLoh.handle));
                gpPd_FreePd(pdLoh.handle);
            }
            else
            {
                gpMacCore_Command_t command;
    #if defined(GP_MACCORE_DIVERSITY_ASSOCIATION_RECIPIENT) || defined(GP_MACCORE_DIVERSITY_POLL_RECIPIENT) || defined(GP_MACCORE_DIVERSITY_SCAN_ORPHAN_RECIPIENT)
                gpPd_TimeStamp_t rxTime = gpPd_GetRxTimestamp(pdLoh.handle);
    #endif // defined(GP_MACCORE_DIVERSITY_ASSOCIATION_RECIPIENT) || defined(GP_MACCORE_DIVERSITY_POLL_RECIPIENT)
                command = gpPd_ReadByte(pdLoh.handle, pdLoh.offset);
                pdLoh.offset++;
                switch(command)
                {
    #ifdef GP_MACCORE_DIVERSITY_ASSOCIATION_RECIPIENT
                    case gpMacCore_CommandAssociationRequest:
                    {
                        if (DIVERSITY_ASSOCIATION_RECIPIENT())
                        {
                            // assoc request is always 2 bytes
                            // mdi.stackId has to be valid here -> stack is based on pan id
                            if(    pdLoh.length == GP_MACCORE_ASSOCIATION_REQUEST_CMD_LEN
                                && mdi.stackId != GP_MACCORE_STACK_UNDEFINED )
                            {
                                // only respond to association request commands when association permit is true
                                if(gpMacCore_GetAssociationPermit(mdi.stackId))
                                {
                                    UInt8 capabilityInformation = gpPd_ReadByte(pdLoh.handle, pdLoh.offset);
                                    gpMacCore_cbAssociateIndication(&mdi.srcAddrInfo.address, capabilityInformation, mdi.stackId, rxTime);
                                }
                            }
                        }
                        break;
                    }
    #endif //GP_MACCORE_DIVERSITY_ASSOCIATION_RECIPIENT
    #ifdef GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR
                    case gpMacCore_CommandAssociationResponse:
                    {
                        if (DIVERSITY_ASSOCIATION_ORIGINATOR())
                        {
                            // mdi.stackId not relevant here -> use GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->stackId
                            // Assoc request pending, if response here, we received data
                            if(     GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs
                                 && GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->assoc == true
                                 && pdLoh.length == GP_MACCORE_ASSOCIATION_RESPONSE_CMD_LEN )
                            {
                                gpPd_ReadByteStream(pdLoh.handle, pdLoh.offset, 2, (UInt8*)&GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->responseShortAddr);
                                pdLoh.offset += 2;
                                LITTLE_TO_HOST_UINT16(&GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->responseShortAddr);
                                GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->result = gpPd_ReadByte(pdLoh.handle, pdLoh.offset);
                                if( GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->result == gpMacCore_ResultSuccess )
                                {
                                    gpMacCore_SetCoordExtendedAddress(&mdi.srcAddrInfo.address.Extended, GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->stackId);
                                    gpMacCore_SetShortAddress(GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->responseShortAddr, GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->stackId);
                                }
                                //if success or not, always send the callback up to upper layer, when assoc response is correctly received
                                //just keep pooling if data poll is not MAC ACKed or if no packet is received on the RX window.
                                MacCore_HandleAssocConf();
                            }
                        }
                        break;
                    }
    #endif //GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR
    #ifdef GP_MACCORE_DIVERSITY_POLL_RECIPIENT
                    case gpMacCore_CommandDataRequest:
                    {
                        if (DIVERSITY_POLL_RECIPIENT())
                        {
                            // mdi.stackId has to be valid here -> if not, drop
                            if(    pdLoh.length == GP_MACCORE_DATA_REQUEST_CMD_LEN
                                && mdi.stackId != GP_MACCORE_STACK_UNDEFINED )
                            {
        #ifdef GP_MACCORE_DIVERSITY_ASSOCIATION_RECIPIENT
                                MacCore_AssocRsp_t *pAssocRsp = MacCore_GetAssocRespPointer(mdi.srcAddrInfo.address.Extended);
                                if( DIVERSITY_ASSOCIATION_RECIPIENT()
                                    && mdi.srcAddrInfo.addressMode == gpMacCore_AddressModeExtendedAddress
                                    && pAssocRsp != NULL
                                )
                                {
                                    gpSched_ScheduleEventArg(0,MacCore_cbAssocResp, (void*)pAssocRsp);
                                }
                                else
        #endif //GP_MACCORE_DIVERSITY_ASSOCIATION_RECIPIENT
                                {
                                    Bool framePending;
                                    framePending = MacCore_CheckIfDataPending(&mdi.srcAddrInfo, mdi.stackId);
                                    MacCore_cbPollIndication(&mdi.srcAddrInfo, mdi.stackId, rxTime, framePending);
                                }
                            }
                        }
                        break;
                    }
    #endif //GP_MACCORE_DIVERSITY_POLL_RECIPIENT
    #ifdef GP_MACCORE_DIVERSITY_SCAN_ORPHAN_RECIPIENT
                    case gpMacCore_CommandOrphanNotification:
                    {
                        if (DIVERSITY_SCAN_ORPHAN_RECIPIENT())
                        {
                            // mdi.stackId not valid here (other side lost his stack) -> ignore
                            //orphan has extended src address
                            if(  mdi.srcAddrInfo.addressMode == gpMacCore_AddressModeExtendedAddress
                              && pdLoh.length == GP_MACCORE_ORPHAN_NOTIFICATION_CMD_LEN )
                            {
                                UInt8 stackId;
                                for( stackId=0;stackId<DIVERSITY_NR_OF_STACKS;stackId++)
                                {
                                    if( gpMacCore_cbValidStack(stackId) && MacCore_RxForThisStack( stackId,gpMacCore_AddressModeExtendedAddress,rxInfo->rxChannel) )
                                    {
                                        gpMacCore_cbOrphanIndication(&mdi.srcAddrInfo.address.Extended, stackId, rxTime);
                                    }
                                }
                            }
                        }
                        break;
                    }
    #endif //GP_MACCORE_DIVERSITY_SCAN_ORPHAN_RECIPIENT
    #ifdef GP_MACCORE_DIVERSITY_SCAN_ORPHAN_ORIGINATOR
                    case gpMacCore_CommandCoordinatorRealignment:
                    {
                        if (DIVERSITY_SCAN_ORPHAN_ORIGINATOR())
                        {
                            gpMacCore_GlobalVars_t* maccore_globals = GP_MACCORE_GET_GLOBALS();
                            // mdi.stackId not relevant here -> use maccore_globals->gpMacCore_pScanState->stackId
                            if(    pdLoh.length == GP_MACCORE_COORDINATOR_REALIGNMENT_CMD_LEN
                                && gpSched_UnscheduleEvent(MacCore_DoOrphanScan) && maccore_globals->gpMacCore_pScanState )
                            {
                                UInt8 temp8;
                                UInt16 temp16;
                                gpPd_ReadByteStream( pdLoh.handle, pdLoh.offset, 2, (UInt8*)&temp16);
                                LITTLE_TO_HOST_UINT16(&temp16);
                                gpMacCore_SetPanId(temp16,maccore_globals->gpMacCore_pScanState->stackId);
                                pdLoh.offset += 2;

                                gpPd_ReadByteStream( pdLoh.handle, pdLoh.offset, 2, (UInt8*)&temp16);
                                LITTLE_TO_HOST_UINT16(&temp16);
                                gpMacCore_SetCoordShortAddress(temp16,maccore_globals->gpMacCore_pScanState->stackId);
                                pdLoh.offset += 2;

                                gpPd_ReadByteStream( pdLoh.handle, pdLoh.offset, 1, (UInt8*)&temp8);
                                gpRxArbiter_SetStackChannel(temp8,maccore_globals->gpMacCore_pScanState->stackId);
                                pdLoh.offset += 1;

                                gpPd_ReadByteStream( pdLoh.handle, pdLoh.offset, 2, (UInt8*)&temp16);
                                LITTLE_TO_HOST_UINT16(&temp16);
                                gpMacCore_SetShortAddress(temp16,maccore_globals->gpMacCore_pScanState->stackId);

                                gpMacCore_SetCoordExtendedAddress(&mdi.srcAddrInfo.address.Extended,maccore_globals->gpMacCore_pScanState->stackId);

                                MacCore_HandleOrphanScanEnd( gpMacCore_ResultSuccess);
                            }
                        }
                        break;
                    }
    #endif //GP_MACCORE_DIVERSITY_SCAN_ORPHAN_ORIGINATOR
    #ifdef GP_MACCORE_DIVERSITY_SCAN_ACTIVE_RECIPIENT
                    case gpMacCore_CommandBeaconRequest: // Beacon request received
                    {
                        if (DIVERSITY_SCAN_ACTIVE_RECIPIENT())
                        {
                            // mdi.stackId not valid here (no src addressing) -> ignore
                            // according to the standard, beacon requests must have a broadcast panID address in them
                            if (  mdi.dstAddrInfo.panId == GP_MACCORE_PANID_BROADCAST
                                && pdLoh.length == GP_MACCORE_BEACON_REQUEST_CMD_LEN )
                            {
                                gpPd_Loh_t BeaconPd ;
                                gpMacCore_GlobalVars_t* maccore_globals = GP_MACCORE_GET_GLOBALS();

                                maccore_globals->MacCore_BeaconRequestStackId = 0 ;
                                // initialize pd - allocate memory for beacons
                                if(MacCore_AllocatePdLoh(&BeaconPd))
                                {
                                    // check whether to transmit is done inside MacCore_TransmitBeacon()
                                    // the memory slot is released once all beacons have been transmitted
                                    MacCore_TransmitBeacon(&BeaconPd,rxInfo) ;
                                }
                            }
                        }
                        break;
                    }
    #endif //GP_MACCORE_DIVERSITY_SCAN_ACTIVE_RECIPIENT
                    default:
                    {
                        GP_LOG_PRINTF("Unknown cmd:%x",0,command);
                        break;
                    }
                }
                gpPd_FreePd( pdLoh.handle );
            }
            break;
        }
        default:
        {
            gpPd_FreePd( pdLoh.handle );
            //GP_LOG_SYSTEM_PRINTF("unkn frametype %x", 2, mdi.frameType);
            break;
        }
    }
}

/* these functions need to be pached because the original rom code does not initialize the */
/* new dataReqOptions which are needed now in case raw+encryption is enabled */

gpMacCore_Result_t MacCore_SendCommand_patched(gpMacCore_AddressInfo_t* pDestAddrInfo, gpMacCore_AddressInfo_t* pSrcAddrInfo, UInt8 txOptions, gpMacCore_Security_t *pSecOptions, UInt8 *pData, UInt8 len , gpMacCore_StackId_t stackId, gpHal_MacScenario_t scenario)
{
    gpPd_Loh_t              pdLoh;
    gpHal_DataReqOptions_t  dataReqOptions;
    UInt8                   channels[3];
    gpMacCore_Result_t      result;

    if(!MacCore_AllocatePdLoh(&pdLoh))
    {
        return gpMacCore_ResultTransactionOverflow;
    }

    channels[0] = gpRxArbiter_GetStackChannel(stackId);
    channels[1] = GP_HAL_MULTICHANNEL_INVALID_CHANNEL;
    channels[2] = GP_HAL_MULTICHANNEL_INVALID_CHANNEL;
    dataReqOptions.macScenario = scenario;
    dataReqOptions.srcId = stackId;
    /* here's the fix which is not in rom: */
#if defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)
    dataReqOptions.rawEncryptionEnable = false;
    dataReqOptions.rawKeepFrameCounter = false;
#endif // defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)

    if(!GP_MACCORE_CHECK_CHANNEL_VALID(channels[0]))
    {
        //Channel not set yet ?
        GP_LOG_SYSTEM_PRINTF("Ch inv: %x", 0, channels[0]);
        return gpMacCore_ResultInvalidParameter;
    }

    // set transmit attributes for this stack
    gpPad_SetTxChannels(MacCore_GetPad(stackId), channels);

    // Write Data
    gpPd_WriteByteStream(pdLoh.handle, pdLoh.offset, len, pData);
    pdLoh.length += len;

    // can also be optimized by converting to assert if
    if(GP_MACCORE_NO_SECURITY_SPECIFIED(pSecOptions))
    {
        // no security ==> only write normal MAC header
        MacCore_WriteMacHeaderInPd(gpMacCore_FrameTypeCommand, pSrcAddrInfo, pDestAddrInfo, txOptions, gpEncryption_SecLevelNothing, &pdLoh , stackId);
        if(pdLoh.length > GP_MACCORE_MAX_PHY_PACKET_SIZE_NO_FCS)
        {
            // If any parameter in the MCPS-DATA.request primitive is not supported or is out of range, the MAC sublayer will issue the MCPS-DATA.confirm primitive with a status of INVALID_PARAMETER.
            result = gpMacCore_ResultInvalidParameter;
        }
    }
#ifdef GP_MACCORE_DIVERSITY_SECURITY_ENABLED
    else
    {
        GP_ASSERT_DEV_EXT(false); // ROM doesn't support normal MAC security (only in Raw mode via this patch)
        //result = MacCore_PrepareOutgoingSecuredPacket(pSrcAddrInfo, pDestAddrInfo, &pdLoh, txOptions, pSecOptions , gpMacCore_FrameTypeCommand, stackId);
    }
#endif // GP_MACCORE_DIVERSITY_SECURITY_ENABLED

    // send command
    result = MacCore_TxDataRequest(&dataReqOptions, pdLoh, stackId);

    if(result != gpMacCore_ResultSuccess)
    {
        gpPd_FreePd(pdLoh.handle);
    }
    else
    {
#if defined(GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR) || defined(GP_MACCORE_DIVERSITY_POLL_ORIGINATOR)
        if(DIVERSITY_POLL_ORIGINATOR() || DIVERSITY_ASSOCIATION_ORIGINATOR())
        {
            if(GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs && scenario == gpHal_MacPollReq)
            {
                //Store handle as tracker (used in Poll DataConfirms)
                GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->pdHandle = pdLoh.handle;
            }
        }
#endif
    }
    return result;
}

gpMacCore_Result_t MacCore_SendCommandBeaconRequest_patched(UInt8 channel)
{
    gpHal_Result_t result;
    gpPd_Loh_t pdLoh;
    gpMacCore_AddressInfo_t srcAddrInfo;
    gpMacCore_AddressInfo_t dstAddrInfo;
    gpHal_DataReqOptions_t  dataReqOptions;
    UInt8 channels[3] = {channel, GP_HAL_MULTICHANNEL_INVALID_CHANNEL, GP_HAL_MULTICHANNEL_INVALID_CHANNEL};
    gpMacCore_GlobalVars_t* maccore_globals = GP_MACCORE_GET_GLOBALS();

    // initialize pd
    if(!MacCore_AllocatePdLoh(&pdLoh))
    {
        return gpMacCore_ResultTransactionOverflow;
    }

    dataReqOptions.macScenario = gpHal_MacDefault;
    dataReqOptions.srcId = maccore_globals->gpMacCore_pScanState->stackId;
#if defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)
    dataReqOptions.rawEncryptionEnable = false;
    dataReqOptions.rawKeepFrameCounter = false;
#endif // defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)

    // set transmit attributes for this stack
    gpPad_SetTxChannels(MacCore_GetPad(maccore_globals->gpMacCore_pScanState->stackId), channels);

    srcAddrInfo.addressMode = gpMacCore_AddressModeNoAddress;
    srcAddrInfo.panId = gpMacCore_GetPanId(maccore_globals->gpMacCore_pScanState->stackId);

    dstAddrInfo.addressMode = gpMacCore_AddressModeShortAddress;
    dstAddrInfo.address.Short = GP_MACCORE_SHORT_ADDR_BROADCAST;
    dstAddrInfo.panId = GP_MACCORE_PANID_BROADCAST;

    MacCore_WriteMacHeaderInPd(gpMacCore_FrameTypeCommand, &srcAddrInfo, &dstAddrInfo,0,0,&pdLoh,maccore_globals->gpMacCore_pScanState->stackId);

    gpPd_WriteByte(pdLoh.handle, GP_MACCORE_MAX_MAC_HEADER_SIZE, gpMacCore_CommandBeaconRequest);
    pdLoh.length++;

    // send beacon request
    result = MacCore_TxDataRequest(&dataReqOptions, pdLoh, maccore_globals->gpMacCore_pScanState->stackId);
    if (result != gpHal_ResultSuccess)
    {
        gpPd_FreePd(pdLoh.handle);
    }
    return result;
}


#ifdef GP_MACCORE_DIVERSITY_SCAN_ORPHAN_ORIGINATOR
gpMacCore_Result_t MacCore_SendCommandOrphanNotification_patched(UInt8 channel)
{
    gpHal_Result_t result;
    gpPd_Loh_t pdLoh;
    gpMacCore_AddressInfo_t srcAddrInfo;
    gpMacCore_AddressInfo_t dstAddrInfo;
    gpHal_DataReqOptions_t  dataReqOptions;
    UInt8 channels[3] = {channel, GP_HAL_MULTICHANNEL_INVALID_CHANNEL, GP_HAL_MULTICHANNEL_INVALID_CHANNEL};
    gpMacCore_GlobalVars_t* maccore_globals = GP_MACCORE_GET_GLOBALS();

    // initialize pd
    if(!MacCore_AllocatePdLoh(&pdLoh))
    {
        return gpMacCore_ResultTransactionOverflow;
    }
    dataReqOptions.macScenario = gpHal_MacDefault;
    dataReqOptions.srcId = maccore_globals->gpMacCore_pScanState->stackId;
#if defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)
    dataReqOptions.rawEncryptionEnable = false;
    dataReqOptions.rawKeepFrameCounter = false;
#endif // defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)

    // set transmit attributes for this stack
    gpPad_SetTxChannels(MacCore_GetPad(maccore_globals->gpMacCore_pScanState->stackId), channels);

    srcAddrInfo.addressMode = gpMacCore_AddressModeExtendedAddress;
    gpMacCore_GetExtendedAddress(&srcAddrInfo.address.Extended, maccore_globals->gpMacCore_pScanState->stackId);
    srcAddrInfo.panId = GP_MACCORE_PANID_BROADCAST;

    dstAddrInfo.addressMode = gpMacCore_AddressModeShortAddress;
    dstAddrInfo.address.Short = GP_MACCORE_SHORT_ADDR_BROADCAST;
    dstAddrInfo.panId = GP_MACCORE_PANID_BROADCAST;

    MacCore_WriteMacHeaderInPd(gpMacCore_FrameTypeCommand, &srcAddrInfo, &dstAddrInfo, 0, 0, &pdLoh,maccore_globals->gpMacCore_pScanState->stackId);

    gpPd_WriteByte(pdLoh.handle, GP_MACCORE_MAX_MAC_HEADER_SIZE, gpMacCore_CommandOrphanNotification);
    pdLoh.length++;

    // send orphan notification
    result = MacCore_TxDataRequest(&dataReqOptions, pdLoh, maccore_globals->gpMacCore_pScanState->stackId);
    if (result != gpHal_ResultSuccess)
    {
        gpPd_FreePd(pdLoh.handle);
    }
    return result;
}
#endif //GP_MACCORE_DIVERSITY_SCAN_ORPHAN_ORIGINATOR

void gpMacCore_Init_patched(void)
{
    // first call the original rom function
    gpMacCore_Init_orgrom();

    // now update the MaxTransferTime
#if defined(GP_HAL_DIVERSITY_RAW_ENHANCED_ACK_RX)
    gpHal_MacSetMaxTransferTime(GP_MACCORE_MAX_TXFRAME_PLUS_RXENHACK_TIME);
#else
    gpHal_MacSetMaxTransferTime(GP_MACCORE_MAX_TXFRAME_PLUS_RXNORMALACK_TIME);
#endif // defined(GP_HAL_DIVERSITY_RAW_ENHANCED_ACK_RX)

}

#endif // (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)

#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_RAWTX)
void MacCore_HalDataConfirm_patched(gpHal_Result_t status, gpPd_Loh_t pdLoh, UInt8 lastChannelUsed)
{
    MacCore_HeaderDescriptor_t  mdi;

    GP_STAT_SAMPLE_TIME();

#ifdef GP_MACCORE_DIVERSITY_TIMEDTX
    if (DIVERSITY_TIMEDTX())
    {
        if (pdLoh.handle == GP_MACCORE_GET_GLOBALS()->MacCore_TimedTx_PdHandle)
        {
            // Packet from scheduled TX queue now confirmed.
            GP_MACCORE_GET_GLOBALS()->MacCore_TimedTx_State = gpMacCore_TimedTxState_Idle;
            GP_MACCORE_GET_GLOBALS()->MacCore_TimedTx_PdHandle = GP_PD_INVALID_HANDLE;
            if (GP_MACCORE_GET_GLOBALS()->MacCore_TimedTx_EventId != GPHAL_ES_ABSOLUTE_EVENT_ID_INVALID)
            {
                gpHal_FreeAbsoluteEvent(GP_MACCORE_GET_GLOBALS()->MacCore_TimedTx_EventId);
                GP_MACCORE_GET_GLOBALS()->MacCore_TimedTx_EventId = GPHAL_ES_ABSOLUTE_EVENT_ID_INVALID;
            }
        }
    }
#endif //GP_MACCORE_DIVERSITY_TIMEDTX

    MacCore_AnalyseMacHeader(&pdLoh, &mdi);
    mdi.stackId = MacCore_GetStackId(&mdi.srcAddrInfo);

#ifdef GP_MACCORE_DIVERSITY_RAW_FRAMES
    if (DIVERSITY_RAW_FRAMES())
    {
        if(GP_MACCORE_GET_GLOBALS_CONST()->MacCore_RawFrameInfoPtr->raw[pdLoh.handle])
        {

            {
                GP_LOG_PRINTF("Raw gpMacCore_cbDataConfirm  pd=%d: l:%u ch:%u",0, pdLoh.handle, pdLoh.length, lastChannelUsed);
#if defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)
                if(MACCORE_FRAMECONTROL_SECURITY_GET(mdi.frameControl))
                {
                    gpMacCore_cbSecurityFrameCounterIndication(mdi.frameCounter, mdi.stackId);
                }
#endif // defined(GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION)
                MacCore_cbDataConfirm(status, pdLoh.handle);
                GP_MACCORE_GET_GLOBALS_CONST()->MacCore_RawFrameInfoPtr->raw[pdLoh.handle] = false;
            }
            return;
        }
    }
#endif

    switch(mdi.frameType)
    {
        case gpMacCore_FrameTypeData:
        {
            if(lastChannelUsed != GP_HAL_MULTICHANNEL_INVALID_CHANNEL)
            {
                // it was a gpHal multichannel retransmit
                // => adapt the current stack channel
                gpRxArbiter_SetStackChannel(lastChannelUsed, MacCore_GetStackId( &mdi.srcAddrInfo ) );
            }

            MacCore_cbDataConfirm(status, pdLoh.handle);
            return;
        }
        case gpMacCore_FrameTypeCommand:
        {
            gpMacCore_Command_t command;
#if defined(GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR) || defined(GP_MACCORE_DIVERSITY_ASSOCIATION_RECIPIENT) || defined(GP_MACCORE_DIVERSITY_POLL_ORIGINATOR) || defined(GP_MACCORE_DIVERSITY_SCAN_ORPHAN_RECIPIENT)
            gpPd_TimeStamp_t    txTime;
            txTime = gpPd_GetTxTimestamp(pdLoh.handle);
#endif
            command = gpPd_ReadByte(pdLoh.handle, pdLoh.offset);

            switch(command)
            {
                case gpMacCore_CommandBeaconRequest:
                {
                    break;
                }
#ifdef GP_MACCORE_DIVERSITY_SCAN_ORPHAN_ORIGINATOR
                case gpMacCore_CommandOrphanNotification:
                {
                    break;
                }
#endif //GP_MACCORE_DIVERSITY_SCAN_ORPHAN_ORIGINATOR
#ifdef GP_MACCORE_DIVERSITY_ASSOCIATION_RECIPIENT
                case gpMacCore_CommandAssociationResponse:
                {
                    if (DIVERSITY_ASSOCIATION_RECIPIENT())
                    {
                        mdi.stackId = MacCore_GetStackId( &mdi.srcAddrInfo );
                        MacCore_cbAssocConfirm(&mdi.srcAddrInfo, &mdi.dstAddrInfo, status, mdi.stackId, txTime);
                    }
                    break;
                }
#endif //GP_MACCORE_DIVERSITY_ASSOCIATION_RECIPIENT
#ifdef GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR
                case gpMacCore_CommandAssociationRequest:
                {
                    if (DIVERSITY_ASSOCIATION_ORIGINATOR())
                    {
                        if(GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs)
                        {
                            if(status == gpHal_ResultSuccess)
                            {
                                gpSched_ScheduleEvent((UInt32)((UInt32)GP_MACCORE_RESPONSE_WAIT_TIME * (UInt32)GP_MACCORE_SYMBOL_DURATION), MacCore_AssociateSendCommandDataRequest );
                            }
                            else
                            {
                                GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->result = status;
                                GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->txTimestamp = txTime;
                                MacCore_HandleAssocConf();
                            }
                        }
                    }
                    break;
                }
#endif //GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR
#ifdef GP_MACCORE_DIVERSITY_POLL_ORIGINATOR
                case gpMacCore_CommandDataRequest:
                {
                    if (DIVERSITY_POLL_ORIGINATOR())
                    {
                        // FP bit is not correctly read on K5 and status is always successful
                        if(GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs && \
                          (GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->pdHandle == pdLoh.handle)) //Skipping old dataconfirms (expected only from a Association poll request)
                        {
                            /* Do not use the result here from the poll confirm.
                            GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->result is initially set to no data
                            and is updated by sw if we received correct packet. */
                            if(status == gpHal_ResultNoAck)
                            {
                                GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->result = gpMacCore_ResultNoAck;
                            }
                            GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->txTimestamp = txTime;
                            //schedule poll/assoc confirm as we want to make sure all dataindication interrupts are handled.
                            gpSched_ScheduleEvent(0,MacCore_DelayedPollConfirm);
                        }
                    }
                    break;
                }
#endif //GP_MACCORE_DIVERSITY_POLL_ORIGINATOR
#ifdef GP_MACCORE_DIVERSITY_SCAN_ORPHAN_RECIPIENT
                case gpMacCore_CommandCoordinatorRealignment:
                {
                    if (DIVERSITY_SCAN_ORPHAN_RECIPIENT())
                    {
                        mdi.stackId = MacCore_GetStackId( &mdi.srcAddrInfo );
                        gpMacCore_cbOrphanCommStatusIndication(&mdi.srcAddrInfo, &mdi.dstAddrInfo, status, mdi.stackId, txTime);
                    }
                    break;
                }
#endif //GP_MACCORE_DIVERSITY_SCAN_ORPHAN_RECIPIENT
                default:
                {
                    break;
                }
            }
            break;
        }
#ifdef GP_MACCORE_DIVERSITY_SCAN_ACTIVE_RECIPIENT
        case gpMacCore_FrameTypeBeacon: // Beacon confirmed
        {
            if (DIVERSITY_SCAN_ACTIVE_RECIPIENT())
            {
                // send out the next beacon if needed - beacon confirmed
                gpHal_RxInfo_t rxInfo;
                rxInfo.rxChannel = lastChannelUsed;
                MacCore_TransmitBeacon(&pdLoh, &rxInfo);
                return;
            }
        }
#endif //GP_MACCORE_DIVERSITY_SCAN_ACTIVE_RECIPIENT
        default:
        {
            break;
        }
    }

    // In all cases free the handle
    gpPd_FreePd(pdLoh.handle);
}
#endif // (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_RAWTX)

/*****************************************************************************
 *                    Patch Function Multiplexer(s)
 *****************************************************************************/

#ifdef GP_ROM_PATCHED_gpMacCore_DataRequest
void gpMacCore_DataRequest(gpMacCore_AddressMode_t srcAddrMode, gpMacCore_AddressInfo_t* pDstAddrInfo, UInt8 txOptions, gpMacCore_Security_t *pSecOptions, gpMacCore_MultiChannelOptions_t multiChannelOptions, gpPd_Loh_t pdLoh, gpMacCore_StackId_t stackId)
{
#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)
    if(gpJumpTables_GetRomVersion() < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)
    {
        gpMacCore_DataRequest_patched(srcAddrMode, pDstAddrInfo, txOptions, pSecOptions, multiChannelOptions, pdLoh, stackId);
        return;
    }
#endif

    gpMacCore_DataRequest_orgrom(srcAddrMode, pDstAddrInfo, txOptions, pSecOptions, multiChannelOptions, pdLoh, stackId);
}
#endif //GP_ROM_PATCHED_gpMacCore_DataRequest


void MacCore_HalDataIndication(gpPd_Loh_t pdLoh, gpHal_RxInfo_t *rxInfo)
{
#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)
    if(gpJumpTables_GetRomVersion() < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)
    {
        MacCore_HalDataIndication_patched(pdLoh, rxInfo);
        return;
    }
#endif

    MacCore_HalDataIndication_orgrom(pdLoh, rxInfo);
}

#ifdef GP_ROM_PATCHED_MacCore_AnalyseMacHeader
UInt8 MacCore_AnalyseMacHeader(gpPd_Loh_t* p_PdLoh, MacCore_HeaderDescriptor_t* pMacHeaderDecoded )
{
#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)
    if(gpJumpTables_GetRomVersion() < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)
    {
        return MacCore_AnalyseMacHeader_patched(p_PdLoh, pMacHeaderDecoded);
    }
#endif
    return MacCore_AnalyseMacHeader_orgrom(p_PdLoh, pMacHeaderDecoded);
}
#endif //GP_ROM_PATCHED_MacCore_AnalyseMacHeader

#ifdef GP_MACCORE_DIVERSITY_RX_WINDOWS
void gpMacCore_EnableRxWindows(UInt32 dutyCycleOnTime, UInt32 dutyCyclePeriod, UInt16 recurrenceAmount, UInt32 startTime, gpMacCore_StackId_t stackId)
{
    UInt8 channel = gpMacCore_GetCurrentChannel(stackId);
    gpRxArbiter_EnableRxWindows(channel, dutyCycleOnTime, dutyCyclePeriod, recurrenceAmount, startTime, stackId);
}

void gpMacCore_DisableRxWindows(gpMacCore_StackId_t stackId)
{
    gpRxArbiter_DisableRxWindows(stackId);
}

void gpMacCore_EnableCsl(UInt16 dutyCyclePeriod, gpMacCore_StackId_t stackId)
{
    GP_ASSERT_DEV_INT(dutyCyclePeriod <= 0xFFFF);
    gpHal_EnableCsl(dutyCyclePeriod);
}

void gpMacCore_UpdateCslSampleTime(UInt32 nextCslSampleTime, gpMacCore_StackId_t stackId)
{
    gpHal_UpdateCslSampleTime(nextCslSampleTime);
}
#endif //GP_MACCORE_DIVERSITY_RX_WINDOWS

#ifdef GP_ROM_PATCHED_MacCore_SendCommand
gpMacCore_Result_t MacCore_SendCommand(gpMacCore_AddressInfo_t* pDestAddrInfo, gpMacCore_AddressInfo_t* pSrcAddrInfo, UInt8 txOptions, gpMacCore_Security_t *pSecOptions, UInt8 *pData, UInt8 len , gpMacCore_StackId_t stackId, gpHal_MacScenario_t scenario)
{
#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)
    if(gpJumpTables_GetRomVersion() < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)
    {
        return MacCore_SendCommand_patched(pDestAddrInfo, pSrcAddrInfo, txOptions, pSecOptions, pData, len , stackId, scenario);
    }
#endif
    return MacCore_SendCommand_orgrom(pDestAddrInfo, pSrcAddrInfo, txOptions, pSecOptions, pData, len , stackId, scenario);
}
#endif //GP_ROM_PATCHED_MacCore_SendCommand

#ifdef GP_ROM_PATCHED_MacCore_SendCommandBeaconRequest
gpMacCore_Result_t MacCore_SendCommandBeaconRequest(UInt8 channel)
{
#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)
    if(gpJumpTables_GetRomVersion() < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)
    {
        return MacCore_SendCommandBeaconRequest_patched(channel);
    }
#endif
    return MacCore_SendCommandBeaconRequest_orgrom(channel);
}
#endif //GP_ROM_PATCHED_MacCore_SendCommandBeaconRequest

#ifdef GP_ROM_PATCHED_MacCore_SendCommandOrphanNotification
#ifdef GP_MACCORE_DIVERSITY_SCAN_ORPHAN_ORIGINATOR
gpMacCore_Result_t MacCore_SendCommandOrphanNotification(UInt8 channel)
{
#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)
    if(gpJumpTables_GetRomVersion() < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)
    {
        return MacCore_SendCommandOrphanNotification_patched(channel);
    }
#endif
    return MacCore_SendCommandOrphanNotification_orgrom(channel);
}
#endif //GP_MACCORE_DIVERSITY_SCAN_ORPHAN_ORIGINATOR
#endif //GP_ROM_PATCHED_MacCore_SendCommandOrphanNotification

#ifdef GP_ROM_PATCHED_gpMacCore_Init
void gpMacCore_Init(void)
{
#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)
    if(gpJumpTables_GetRomVersion() < ROMVERSION_FIXFORPATCH_MACCORE_RAWTXENCRYPTION)
    {
        gpMacCore_Init_patched();
        return;
    }
#else
    gpMacCore_Init_orgrom();
#endif
}
#endif  //GP_ROM_PATCHED_gpMacCore_Init

#ifdef GP_ROM_PATCHED_MacCore_HalDataConfirm
void MacCore_HalDataConfirm(gpHal_Result_t status, gpPd_Loh_t pdLoh, UInt8 lastChannelUsed)
{
#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_RAWTX)
    if(gpJumpTables_GetRomVersion() < ROMVERSION_FIXFORPATCH_MACCORE_RAWTX)
    {
        MacCore_HalDataConfirm_patched(status, pdLoh, lastChannelUsed);
        return;
    }
#endif
    MacCore_HalDataConfirm_orgrom(status, pdLoh, lastChannelUsed);
}
#endif //GP_ROM_PATCHED_MacCore_HalDataConfirm
