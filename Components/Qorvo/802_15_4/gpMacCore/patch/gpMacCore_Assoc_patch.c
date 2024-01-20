/*
 * Copyright (c) 2017, 2019, Qorvo Inc
 *
 * gpMacCore_IndTx_patched.c
 *   This file contains patches for the indirect transmission functionality from the MAC protocol.
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

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID GP_COMPONENT_ID_MACCORE
//#define GP_LOCAL_LOG
// General includes
#include "gpMacCore.h"
#include "gpLog.h"
#include "gpSched.h"
#include "gpPd.h"
#include "gpPoolMem.h"
#include "gpMacCore.h"
#include "gpMacCore_defs.h"
#include "gpMacCore_defs_Main.h"
#include "gpJumpTables.h"


/*****************************************************************************
 *                    NRT ROM patch fix version numbers
 *****************************************************************************/
/* Rom versions where patch for the corresponding function is already included
 * while building ROM image (so no patch required when application is built with
 * the specified ROM version)
 */
#if   defined(GP_DIVERSITY_GPHAL_K8E)
#define ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT                         2
#endif

/*****************************************************************************
 *                    ROM Function prototypes
 *****************************************************************************/

void MacCore_StopRunningRequests_orgrom(gpMacCore_StackId_t stackId);
void MacCore_DelayedPollConfirm_orgrom(void);
void MacCore_AssociateSendCommandDataRequest_orgrom(void);
void MacCore_HandleAssocConf_orgrom(void);
void gpMacCore_AssociateRequest_orgrom(UInt8 logicalChannel , gpMacCore_AddressInfo_t* pCoordAddrInfo , UInt8 capabilityInformation , gpMacCore_StackId_t stackId );

void MacCore_HalDataIndication_orgrom(gpPd_Loh_t pdLoh, gpHal_RxInfo_t *rxInfo);

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Patched Function(s)
 *****************************************************************************/

#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT)
#ifdef GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR
void MacCore_StopRunningRequests_patched(gpMacCore_StackId_t stackId)
{
#if defined(GP_MACCORE_DIVERSITY_SCAN_ACTIVE_ORIGINATOR)  || \
    defined(GP_MACCORE_DIVERSITY_SCAN_ED_ORIGINATOR)      || \
    defined(GP_MACCORE_DIVERSITY_SCAN_ORPHAN_ORIGINATOR)
    if(GP_MACCORE_GET_GLOBALS()->gpMacCore_pScanState && (stackId == GP_MACCORE_GET_GLOBALS()->gpMacCore_pScanState->stackId))
    {
#ifdef GP_MACCORE_DIVERSITY_SCAN_ACTIVE_ORIGINATOR
        gpSched_UnscheduleEvent(MacCore_DoActiveScan);
#endif //GP_MACCORE_DIVERSITY_SCAN_ACTIVE_ORIGINATOR
#ifdef GP_MACCORE_DIVERSITY_SCAN_ORPHAN_ORIGINATOR
        gpSched_UnscheduleEvent(MacCore_DoOrphanScan);
#endif //GP_MACCORE_DIVERSITY_SCAN_ORPHAN_ORIGINATOR
#if (GP_MACCORE_SCAN_RXOFFWINDOW_TIME_US != 0)
        gpSched_UnscheduleEvent(MacCore_ScanRxOffWindow);
#endif //(GP_MACCORE_SCAN_RXOFFWINDOW_TIME_US != 0)
        gpPoolMem_Free(GP_MACCORE_GET_GLOBALS()->gpMacCore_pScanState);
        GP_MACCORE_GET_GLOBALS()->gpMacCore_pScanState = NULL;
    }
#endif

#ifdef GP_MACCORE_DIVERSITY_ASSOCIATION_RECIPIENT
    MacCore_AssocRespStopRequests(stackId);
#endif //GP_MACCORE_DIVERSITY_ASSOCIATION_RECIPIENT

#ifdef GP_MACCORE_DIVERSITY_POLL_ORIGINATOR
    if(GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs && stackId == GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->stackId)
    {
#ifdef GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR
        gpSched_UnscheduleEvent(MacCore_AssociateSendCommandDataRequest);
        gpSched_UnscheduleEvent(MacCore_AssociateTimeout);
#endif //GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR
        gpSched_UnscheduleEvent(MacCore_DelayedPollConfirm);
        gpPoolMem_Free(GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs);
        GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs = NULL;
    }
#endif //GP_MACCORE_DIVERSITY_POLL_ORIGINATOR
}
#endif //GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR
#endif //#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT)

#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT)
#ifdef GP_MACCORE_DIVERSITY_POLL_ORIGINATOR
void MacCore_DelayedPollConfirm_patched(void)
{
    if(GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs)
    {
#ifdef GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR
        if (DIVERSITY_ASSOCIATION_ORIGINATOR())
        {
            //Path only used in case of failure cases
            //Association Confirm already triggered when receiving Assoc Response
            if(GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->assoc)
            {
                //if timeout has not been reached yet, re-schedule data poll
                if(gpSched_ExistsEvent(MacCore_AssociateTimeout))
                {
                    gpSched_ScheduleEvent((UInt32)((UInt32)GP_MACCORE_RESPONSE_WAIT_TIME * (UInt32)GP_MACCORE_SYMBOL_DURATION), MacCore_AssociateSendCommandDataRequest );
                }
                else //if timeout reached during in flight data poll, confirm fail poll
                {
                    //Result is filled in in pPollReqArgs -> used in Confirm handler
                    MacCore_HandleAssocConf();
                }
                return;
            }
        }
#endif //GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR
        {
            MacCore_DelayedPollConfirm_orgrom();
        }
    }
}
#endif //GP_MACCORE_DIVERSITY_POLL_ORIGINATOR
#endif //#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT)

#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT)
#if defined(GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR)
void MacCore_AssociateSendCommandDataRequest_patched(void)
{
    gpMacCore_Result_t result;

    //null pointer protection
    if(GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs == NULL)
    {
        GP_ASSERT_DEV_EXT(false);
        return;
    }

    result = MacCore_SendCommandDataRequest(&GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->coordAddrInfo, true, NULL, GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->stackId);

    if (result != gpMacCore_ResultSuccess)
    {
        //if we were not able to send a data request, and timeout has not been reach yet, re-schedule
        if(gpSched_ExistsEvent(MacCore_AssociateTimeout))
        {
            gpSched_ScheduleEvent((UInt32)((UInt32)GP_MACCORE_RESPONSE_WAIT_TIME * (UInt32)GP_MACCORE_SYMBOL_DURATION), MacCore_AssociateSendCommandDataRequest);
        }
        else//if we were not able to send data request, and timeout has been reached, confirm fail state
        {
            GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->result = result;
            GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->txTimestamp = 0;
            MacCore_HandleAssocConf();
        }
    }
}
#endif //defined(GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR)
#endif //#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT)

#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT)
#if defined(GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR)
void MacCore_HandleAssocConf_patched(void)
{
    /*
     *   According IEEE-802.15.4 paragraph 5.1.3.1 Association
     *   If the value of the Association Status field of the command is not “Association successful,” if there were a
     *   communication failure during the association process due to a missed acknowledgment, or if the association
     *   response command frame were not received, the device shall set macPANId to the default value (0xffff).
     */
    UInt16 assocShortAddress = GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->responseShortAddr;
    gpMacCore_Result_t status = GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->result;
    UInt32 txTime = GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->txTimestamp;

    //if the status of the last poll was not a success and timeout occurred, send timeout status
    if(!gpSched_ExistsEvent(MacCore_AssociateTimeout) && (status != gpMacCore_ResultSuccess))
    {
        status = gpMacCore_ResultTransactionExpired;
    }

    GP_LOG_PRINTF("handle assocconf",0);

    if(status != gpMacCore_ResultSuccess)
    {
        gpMacCore_SetPanId(GP_MACCORE_PANID_BROADCAST, GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->stackId);
    }
    gpPoolMem_Free(GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs);
    GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs = NULL;

    gpSched_UnscheduleEvent(MacCore_AssociateSendCommandDataRequest);
    gpSched_UnscheduleEvent(MacCore_AssociateTimeout);
    gpMacCore_cbAssociateConfirm(assocShortAddress, status, txTime);
}
#endif //defined(GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR)
#endif //#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT)

#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT)
#if defined(GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR)
void gpMacCore_AssociateRequest_patched(UInt8 logicalChannel , gpMacCore_AddressInfo_t* pCoordAddrInfo , UInt8 capabilityInformation , gpMacCore_StackId_t stackId )
{
    gpMacCore_AddressInfo_t srcAddrInfo;
    gpMacCore_Result_t result;
    UInt8 pMsdu[GP_MACCORE_ASSOCIATION_REQUEST_CMD_LEN]={gpMacCore_CommandAssociationRequest,capabilityInformation};

    if(GP_MACCORE_CHECK_IF_ADDRESSINFO_INVALID(pCoordAddrInfo))
    {
        gpMacCore_cbAssociateConfirm(GP_MACCORE_SHORT_ADDR_BROADCAST, gpMacCore_ResultInvalidParameter, 0);
        return;
    }

    if(GP_MACCORE_CHECK_IF_ADDRESSMODE_NOT_PRESENT(pCoordAddrInfo->addressMode))
    {
        gpMacCore_cbAssociateConfirm(GP_MACCORE_SHORT_ADDR_BROADCAST, gpMacCore_ResultInvalidParameter, 0);
        return;
    }

    if(GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs != NULL)
    {
        gpMacCore_cbAssociateConfirm(GP_MACCORE_SHORT_ADDR_BROADCAST, gpMacCore_ResultTransactionOverflow, 0 );
        return;
    }

    gpRxArbiter_SetStackChannel(logicalChannel, stackId);
    gpMacCore_SetPanId(pCoordAddrInfo->panId, stackId);

    if(pCoordAddrInfo->addressMode == gpMacCore_AddressModeShortAddress)
    {
        gpMacCore_SetCoordShortAddress(pCoordAddrInfo->address.Short, stackId);
    }
    else
    {
        gpMacCore_SetCoordExtendedAddress(&(pCoordAddrInfo->address.Extended), stackId);
    }

    // fill in assoc args
    GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs = (MacCore_PollReqArgs_t*)GP_POOLMEM_MALLOC(sizeof(MacCore_PollReqArgs_t));
    GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->stackId = stackId;
    GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->coordAddrInfo = *pCoordAddrInfo;
    //Init response result
    GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->result = gpMacCore_ResultNoData;
    GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->responseShortAddr = GP_MACCORE_SHORT_ADDR_BROADCAST;
    GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->assoc = true;

    // prepare src address info
    MacCore_InitSrcAddressInfo(&srcAddrInfo, gpMacCore_AddressModeExtendedAddress, stackId);
    // overrule pan ID
    srcAddrInfo.panId = GP_MACCORE_PANID_BROADCAST;

    // send Association Request
    result = MacCore_SendCommand(pCoordAddrInfo, &srcAddrInfo, GP_MACCORE_TX_OPT_ACK_REQ , NULL, pMsdu, GP_MACCORE_ASSOCIATION_REQUEST_CMD_LEN , stackId, gpHal_MacDefault);

    //need to schedule timeout here, otherwise MacCore_HandleAssocConf will overwrite the info from the assoc request status to timeout
    //timeout is around 7 seconds if gpMacCore_GetTransactionPersistenceTime is GP_MACCORE_DEFAULT_TRANSACTION_PERSISTENCE_TIME
    gpSched_ScheduleEvent((UInt32)((UInt32)gpMacCore_GetTransactionPersistenceTime(stackId) * (UInt32)GP_MACCORE_BASE_SUPERFRAME_DURATION*GP_MACCORE_SYMBOL_DURATION),
                                MacCore_AssociateTimeout);

    // if success continue in dataconfirm
    // if not success, end association attempt.
    if(result != gpMacCore_ResultSuccess)
    {
        GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->result = result;
        GP_MACCORE_GET_GLOBALS()->gpMacCore_pPollReqArgs->txTimestamp = 0;
        MacCore_HandleAssocConf();
    }
}
#endif //defined(GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR)
#endif //#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT)

/*****************************************************************************
 *                    Patch Function Multiplexer(s)
 *****************************************************************************/

#ifdef GP_ROM_PATCHED_MacCore_StopRunningRequests
void MacCore_StopRunningRequests(gpMacCore_StackId_t stackId)
{
#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT) && defined(GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR)
    if(gpJumpTables_GetRomVersion() < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT)
    {
        MacCore_StopRunningRequests_patched(stackId);
        return;
    }
#endif

    MacCore_StopRunningRequests_orgrom(stackId);
}
#endif //GP_ROM_PATCHED_MacCore_StopRunningRequests

#ifdef GP_ROM_PATCHED_MacCore_DelayedPollConfirm
#ifdef GP_MACCORE_DIVERSITY_POLL_ORIGINATOR
void MacCore_DelayedPollConfirm(void)
{
#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT)
    if(gpJumpTables_GetRomVersion() < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT)
    {
        MacCore_DelayedPollConfirm_patched();
        return;
    }
#endif

    MacCore_DelayedPollConfirm_orgrom();
}
#endif //def GP_MACCORE_DIVERSITY_POLL_ORIGINATOR
#endif //GP_ROM_PATCHED_MacCore_DelayedPollConfirm

#ifdef GP_ROM_PATCHED_MacCore_AssociateSendCommandDataRequest
void MacCore_AssociateSendCommandDataRequest(void)
{
#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT) && defined(GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR)
    if(gpJumpTables_GetRomVersion() < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT)
    {
        MacCore_AssociateSendCommandDataRequest_patched();
        return;
    }
#endif

    MacCore_AssociateSendCommandDataRequest_orgrom();
}
#endif //GP_ROM_PATCHED_MacCore_AssociateSendCommandDataRequest

#ifdef GP_ROM_PATCHED_MacCore_HandleAssocConf
void MacCore_HandleAssocConf(void)
{
#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT) && defined(GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR)
    if(gpJumpTables_GetRomVersion() < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT)
    {
        MacCore_HandleAssocConf_patched();
        return;
    }
#endif

    MacCore_HandleAssocConf_orgrom();
}
#endif //GP_ROM_PATCHED_MacCore_HandleAssocConf

#ifdef GP_ROM_PATCHED_gpMacCore_AssociateRequest
void gpMacCore_AssociateRequest(UInt8 logicalChannel , gpMacCore_AddressInfo_t* pCoordAddrInfo , UInt8 capabilityInformation , gpMacCore_StackId_t stackId )
{
#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT) && defined(GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR)
    if(gpJumpTables_GetRomVersion() < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT)
    {
        gpMacCore_AssociateRequest_patched(logicalChannel, pCoordAddrInfo, capabilityInformation, stackId);
        return;
    }
#endif

    gpMacCore_AssociateRequest_orgrom(logicalChannel, pCoordAddrInfo, capabilityInformation, stackId);
}
#endif //GP_ROM_PATCHED_gpMacCore_AssociateRequest

#ifdef GP_ROM_PATCHED_MacCore_AssociateTimeout
#if defined(GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR)
void MacCore_AssociateTimeout(void)
{
    // just needs to be empty
}
#endif //defined(GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR)
#endif //GP_ROM_PATCHED_MacCore_AssociateTimeout

#if (!(defined(GP_MACCORE_DIVERSITY_SECURITY_ENABLED)) || !(defined(GP_MACCORE_DIVERSITY_RAW_FRAMES)))
void MacCore_HalDataIndication_patched(gpPd_Loh_t pdLoh, gpHal_RxInfo_t *rxInfo)
{
    MacCore_HeaderDescriptor_t mdi;
    UInt8 macHeaderLength;
#ifdef GP_MACCORE_DIVERSITY_RAW_FRAMES
    gpPd_Loh_t pdLoh_backup = pdLoh;
#endif
    gpPd_Loh_t pdLoh_backup_for_rom = pdLoh;


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
        case gpMacCore_FrameTypeCommand:
        {
            UInt8 result;
            result = MacCore_ValidateClearText(&mdi);
            if(result != gpMacCore_ResultSuccess)
            {
                GP_LOG_SYSTEM_PRINTF("dec fail:%x",0,result);
                gpMacCore_cbSecurityFailureCommStatusIndication(&mdi.srcAddrInfo, &mdi.dstAddrInfo, result, mdi.stackId, gpPd_GetRxTimestamp(pdLoh.handle));
                gpPd_FreePd(pdLoh.handle);
                // no need to fall back to the rom version.
                return;
            }
            else
            {
                gpMacCore_Command_t command;
    #if defined(GP_MACCORE_DIVERSITY_ASSOCIATION_RECIPIENT) || defined(GP_MACCORE_DIVERSITY_POLL_RECIPIENT) || defined(GP_MACCORE_DIVERSITY_SCAN_ORPHAN_RECIPIENT)
                // gpPd_TimeStamp_t rxTime = gpPd_GetRxTimestamp(pdLoh.handle);
    #endif // defined(GP_MACCORE_DIVERSITY_ASSOCIATION_RECIPIENT) || defined(GP_MACCORE_DIVERSITY_POLL_RECIPIENT)
                command = gpPd_ReadByte(pdLoh.handle, pdLoh.offset);
                pdLoh.offset++;
                switch(command)
                {
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

                        gpPd_FreePd( pdLoh.handle );
                        return;

                        break;
                    }
    #endif //GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR
                    default:
                    {
                        break;
                    }
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
    pdLoh = pdLoh_backup_for_rom; // restore the offset

    MacCore_HalDataIndication_orgrom(pdLoh, rxInfo);
}

void MacCore_HalDataIndication(gpPd_Loh_t pdLoh, gpHal_RxInfo_t *rxInfo)
{
#if (GPJUMPTABLES_MIN_ROMVERSION < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT)
    if(gpJumpTables_GetRomVersion() < ROMVERSION_FIXFORPATCH_MACCORE_ASSOCTIMEOUT)
    {
        MacCore_HalDataIndication_patched(pdLoh, rxInfo);
        return;
    }
#endif
    MacCore_HalDataIndication_orgrom(pdLoh, rxInfo);
}
#endif
