/*
 * Copyright (c) 2016, GreenPeak Technologies
 * Copyright (c) 2017, Qorvo Inc
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

/** @file "gpBleResPrAddr.c"
 *
 *  Implementation of gpBleResPrAddr
*/

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

// #define GP_LOCAL_LOG
#define GP_COMPONENT_ID GP_COMPONENT_ID_BLERESPRADDR

/* <CodeGenerator Placeholder> General */
/* </CodeGenerator Placeholder> General */


#include "gpBleResPrAddr.h"

/* <CodeGenerator Placeholder> Includes */
#include "global.h"
#include "gpHal.h"
#include "gpSched.h"
#include "gpAssert.h"
#include "gpHal_Ble_Manual.h"
#include "gpBle_defs.h"
#include "gpHci_Includes.h"
#include "gpHal_kx_public.h"

#include "gpLog.h"
#if defined (GP_DIVERSITY_BLE_PERIPHERAL) 
#include "gpBleAdvertiser.h"
#endif
/* </CodeGenerator Placeholder> Includes */
#include "gpRandom.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> Macro */
#define BLE_RES_PR_DEFAULT_PRAND_TO 0x0384 // 15min

#ifndef GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED
/* This check fixes the checkdflags "flag not in code" error. */
#error GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED should be set
#endif
/* </CodeGenerator Placeholder> Macro */

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> FunctionalMacro */
/* </CodeGenerator Placeholder> FunctionalMacro */

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> TypeDefinitions */
typedef UInt8 (*gpHal_BleGenerateRpa_t) (BtDeviceAddress_t *bdAddress, gpHal_BleRpaHandle_t rpHandle);
typedef Bool (*gpHal_BleRpa_IsAllZerosIrk_t) (gpHal_BleRpaHandle_t rpHandle);

/* </CodeGenerator Placeholder> TypeDefinitions */

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticData */
static UInt16 BleResPr_timeout;
/* </CodeGenerator Placeholder> StaticData */

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticFunctionPrototypes */
static void BleResPrAddr_SetRandomPartOfPrand(UInt32 randomPart);
static void BleResPrAddr_cbPrandTimeout(void *arg);
/* </CodeGenerator Placeholder> StaticFunctionPrototypes */

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticFunctionDefinitions */
void BleResPrAddr_SetRandomPartOfPrand(UInt32 randomPart)
{
    UInt32 prand;

    // prand format (lsb to msb)
    // || random part (22 bits) | 1 | 0 ||

    prand = randomPart | 0x400000;
    prand &= 0x7FFFFF;

    GP_LOG_PRINTF("random: %lx prand: %lx",0, (unsigned long)randomPart, (unsigned long)prand);

    gpHal_BleRpa_SetPrand(prand);

#ifdef GP_DIVERSITY_BLE_LEGACY_ADVERTISING
    gpResPrAddr_cbAdvPrandTimeout();
#endif // GP_DIVERSITY_BLE_LEGACY_ADVERTISING
}

void BleResPrAddr_cbPrandTimeout(void *arg)
{
    UInt32 prand = gpRandom_GenerateLargeRandom();
    BleResPrAddr_SetRandomPartOfPrand(prand);

    gpSched_ScheduleEventInSeconds(BleResPr_timeout, BleResPrAddr_cbPrandTimeout, NULL);
}

gpHci_Result_t gpBle_DeviceResolvingListChangeChecker(void)
{

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
    if(gpBleAdvertiser_IsEnabled())
    {
        GP_LOG_PRINTF("Cannot - advertiser active",0);
        return gpHci_ResultCommandDisallowed;
    }
#endif
    return gpHci_ResultSuccess;
}

static Bool gpBle_IsResolvingListChangeForbidden(void)
{
    return (gpHal_BleRpa_IsRpaEnabled() && (gpBle_DeviceResolvingListChangeChecker() != gpHci_ResultSuccess));
}



static gpHci_Result_t Ble_VsdGenerateResolvableAddressHelper(
                                                            gpBle_EventBuffer_t         *pEventBuf,
                                                            gpHci_AdvPeerAddressType_t  peerIdentityAddressType,
                                                            BtDeviceAddress_t *peerIdentityAddress,
                                                            gpHal_BleGenerateRpa_t halGenRpaFp,
                                                            gpHal_BleRpa_IsAllZerosIrk_t isAllZerosIrkFp)
{
/* <CodeGenerator Placeholder> Implementation_Ble_VsdGenerateResolvableAddressHelper */
    gpHci_Result_t hci_result = gpHci_ResultInvalid;
    gpHal_BleRpaHandle_t rpaHandle = BLE_RES_PR_INIT_RPA_HANDLE();
    Bool hal_result = false;

    hal_result = gpHal_BleRpa_MatchPeer(&rpaHandle, BLE_ADV_ADDR_TYPE_TO_HAL_ADDR_BIT(peerIdentityAddressType), peerIdentityAddress);

    if (hal_result && BLE_RES_PR_IS_VALID_HANDLE(rpaHandle) && !isAllZerosIrkFp(rpaHandle))
    {
        hal_result = halGenRpaFp(&pEventBuf->payload.commandCompleteParams.returnParams.bdAddress, rpaHandle);
        GP_ASSERT_DEV_INT(hal_result);
        hci_result = gpHci_ResultSuccess;
    }
    else
    {
        hci_result = gpHci_ResultInvalidHCICommandParameters;
    }

    return hci_result;
/* </CodeGenerator Placeholder> Implementation_Ble_VsdGenerateResolvableAddressHelper */
}

/* </CodeGenerator Placeholder> StaticFunctionDefinitions */

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBleResPrAddr_Init(void)
{
/* <CodeGenerator Placeholder> Implementation_gpBleResPrAddr_Init */
/* </CodeGenerator Placeholder> Implementation_gpBleResPrAddr_Init */
}
void gpBleResPrAddr_Reset(Bool firstReset)
{
/* <CodeGenerator Placeholder> Implementation_gpBleResPrAddr_Reset */
    gpSched_UnscheduleEventArg(BleResPrAddr_cbPrandTimeout, NULL);
    gpHal_BleRpa_SetAddressResolutionEnable(false);
    gpHal_BleRpa_ClearResolvingList();

    BleResPr_timeout = BLE_RES_PR_DEFAULT_PRAND_TO;
    BleResPrAddr_cbPrandTimeout(NULL);
/* </CodeGenerator Placeholder> Implementation_gpBleResPrAddr_Reset */
}
gpHci_Result_t gpBle_LeAddDeviceToResolvingList(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
/* <CodeGenerator Placeholder> Implementation_gpBle_AddDeviceToResolvingList */
    gpHal_BleSetInResolvingList_t deviceInfo;
    gpHal_Result_t      hal_result;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    GP_LOG_PRINTF("Add device to resolving list", 0);
    if( gpBle_IsResolvingListChangeForbidden() )
    {
        GP_LOG_PRINTF("Adding disallowed, due to other activities", 0);
        return gpHci_ResultCommandDisallowed;
    }
    if (pParams->LeAddDeviceToResolvingList.peerIdentityAddressType > 1)
    {
        GP_LOG_PRINTF("Invalid peerIdentityAddressType", 0);
        return gpHci_ResultInvalidHCICommandParameters;
    }

    deviceInfo.peerAddrTypeBit = BLE_ADV_ADDR_TYPE_TO_HAL_ADDR_BIT(pParams->LeAddDeviceToResolvingList.peerIdentityAddressType);
    deviceInfo.peerIdentityAddress = &pParams->LeAddDeviceToResolvingList.peerIdentityAddress;
    deviceInfo.peerIRK = &pParams->LeAddDeviceToResolvingList.peerIRK.data[0];
    deviceInfo.localIRK = &pParams->LeAddDeviceToResolvingList.localIRK.data[0];
    deviceInfo.privacyMode = gpHci_PrivacyModeNetwork;

    hal_result = gpHal_BleRpa_SetDeviceInResolvingList(&deviceInfo);


    return hal_result == gpHal_ResultSuccess ? gpHci_ResultSuccess : gpHci_ResultMemoryCapacityExceeded;
/* </CodeGenerator Placeholder> Implementation_gpBle_AddDeviceToResolvingList */
}
gpHci_Result_t gpBle_LeRemoveDeviceFromResolvingList(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
/* <CodeGenerator Placeholder> Implementation_gpBle_RemoveDeviceFromResolvingList */
    gpHal_Result_t      hal_result;
    Bool                peerAddrTypeBit;
    BtDeviceAddress_t   *pPeerIdentityAddress;
    gpHal_BleRpaHandle_t rpaHandle;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    GP_LOG_PRINTF("Remove device from resolving list",0);

    if ( gpBle_IsResolvingListChangeForbidden() )
    {
        return gpHci_ResultCommandDisallowed;
    }

    peerAddrTypeBit = BLE_ADV_ADDR_TYPE_TO_HAL_ADDR_BIT(pParams->LeRemoveDeviceFromResolvingList.peerIdentityAddressType);
    pPeerIdentityAddress = &pParams->LeRemoveDeviceFromResolvingList.peerIdentityAddress;

    Bool matched = gpHal_BleRpa_MatchPeer(&rpaHandle, peerAddrTypeBit, pPeerIdentityAddress);
    if (!matched || !BLE_RES_PR_IS_VALID_HANDLE(rpaHandle))
    {
        /*
         * SPEC 5.2 Vol4, Part E, $7.8.39 LE Remove Device From Resolving List
         * When a Controller cannot remove a device from the resolving list because it is
         * not found, it shall return the error code Unknown Connection Identifier (0x02).
         */
        return gpHci_ResultUnknownConnectionIdentifier;
    }

    hal_result = gpHal_BleRpa_RemoveDeviceFromResolvingList(rpaHandle);

    return hal_result == gpHal_ResultSuccess ? gpHci_ResultSuccess : gpHci_ResultInvalid;
/* </CodeGenerator Placeholder> Implementation_gpBle_RemoveDeviceFromResolvingList */
}
gpHci_Result_t gpBle_LeClearResolvingList(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
/* <CodeGenerator Placeholder> Implementation_gpBle_ClearResolvingList */
    gpHal_Result_t      hal_result;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    GP_LOG_PRINTF("Clear resolving list",0);

    if ( gpBle_IsResolvingListChangeForbidden() )
    {
        return gpHci_ResultCommandDisallowed;
    }

    hal_result = gpHal_BleRpa_ClearResolvingList();
    return hal_result == gpHal_ResultSuccess ? gpHci_ResultSuccess : gpHci_ResultInvalid;
/* </CodeGenerator Placeholder> Implementation_gpBle_ClearResolvingList */
}
gpHci_Result_t gpBle_LeReadResolvingListSize(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
/* <CodeGenerator Placeholder> Implementation_gpBle_ReadResolvingListSize */
// Returns the max number of entries that can be stored in the resolving list
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    GP_LOG_PRINTF("read resolving list size",0);

    pEventBuf->payload.commandCompleteParams.returnParams.resolvingListSize = gpHal_BleRpaGetMaxListSize();

    return gpHci_ResultSuccess;

/* </CodeGenerator Placeholder> Implementation_gpBle_ReadResolvingListSize */
}

gpHci_Result_t gpBle_VsdGeneratePeerResolvableAddress(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
/* <CodeGenerator Placeholder> Implementation_gpBle_VsdGeneratePeerResolvableAddress */

    gpHci_VsdGeneratePeerResolvableAddressCommand_t *params = NULL;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    GP_LOG_PRINTF("Gen peer resolvable address",0);

    params = &pParams->VsdGeneratePeerResolvableAddress;

    return Ble_VsdGenerateResolvableAddressHelper(pEventBuf, params->peerIdentityAddressType, &params->peerIdentityAddress, gpHal_BleRpa_GeneratePeerRpa, gpHal_BleRpa_IsAllZerosPeerIrk );

/* </CodeGenerator Placeholder> Implementation_gpBle_VsdGeneratePeerResolvableAddress */
}
gpHci_Result_t gpBle_VsdGenerateLocalResolvableAddress(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
/* <CodeGenerator Placeholder> Implementation_gpBle_VsdGenerateLocalResolvableAddress */
    gpHci_VsdGenerateLocalResolvableAddressCommand_t *params = NULL;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    GP_LOG_PRINTF("Gen local resolvable address",0);

    params = &pParams->VsdGenerateLocalResolvableAddress;

    return Ble_VsdGenerateResolvableAddressHelper(pEventBuf, params->peerIdentityAddressType, &params->peerIdentityAddress, gpHal_BleRpa_GenerateLocalRpa, gpHal_BleRpa_IsAllZerosLocalIrk );

/* </CodeGenerator Placeholder> Implementation_gpBle_VsdGenerateLocalResolvableAddress */
}
gpHci_Result_t gpBle_LeSetAddressResolutionEnable(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
/* <CodeGenerator Placeholder> Implementation_gpBle_SetAddressResolutionEnable */
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    GP_LOG_PRINTF("Set address resolution enable: %u",0, pParams->LeSetAddressResolutionEnable.enable);

    if(gpBle_DeviceResolvingListChangeChecker())
    {
        return gpHci_ResultCommandDisallowed;
    }
    if (pParams->LeSetAddressResolutionEnable.enable > 1)
    {
        GP_LOG_PRINTF("Enable value %x is not in range [0,1]",0,pParams->LeSetAddressResolutionEnable.enable);
        return gpHci_ResultInvalidHCICommandParameters;
    }

    gpHal_BleRpa_SetAddressResolutionEnable(pParams->LeSetAddressResolutionEnable.enable);

    return gpHci_ResultSuccess;
/* </CodeGenerator Placeholder> Implementation_gpBle_SetAddressResolutionEnable */
}
gpHci_Result_t gpBle_LeSetResolvablePrivateAddressTimeout(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
/* <CodeGenerator Placeholder> Implementation_gpBle_SetResolvablePrivateAddressTimeout */
    UInt16 newTimeout = pParams->LeSetResolvablePrivateAddressTimeout.rpa_timeout;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    GP_LOG_PRINTF("Set rpa address TO",0);

    if (newTimeout < 1 || 0xA1B8 < newTimeout)
    {
        return gpHci_ResultUnsupportedFeatureOrParameterValue;
    }

    /* In case gpBle_VsdSetRpaPrand disabled the timeout */
    if (!gpSched_ExistsEventArg(BleResPrAddr_cbPrandTimeout, NULL))
    {
        BleResPr_timeout = newTimeout;
        return gpHci_ResultSuccess;
    }

    UInt32 remainingTime = gpSched_GetRemainingTimeArg(BleResPrAddr_cbPrandTimeout, NULL);
    GP_ASSERT_DEV_INT(remainingTime < 0xffffffff);

    remainingTime /= 1000000; /* us to seconds */

    // If the remaining time is larger than the new timeout, we limit the remaining time
    // to said timeout, otherwise we leave the remaining time as is.
    remainingTime = (remainingTime > newTimeout) ? newTimeout : remainingTime;

    BleResPr_timeout = newTimeout;
    gpSched_UnscheduleEventArg(BleResPrAddr_cbPrandTimeout, NULL);
    gpSched_ScheduleEventInSeconds(remainingTime, BleResPrAddr_cbPrandTimeout, NULL);

    return gpHci_ResultSuccess;
/* </CodeGenerator Placeholder> Implementation_gpBle_SetResolvablePrivateAddressTimeout */
}
gpHci_Result_t gpBle_LeSetPrivacyMode(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
/* <CodeGenerator Placeholder> Implementation_gpBle_SetPrivacyMode */
    gpHci_Result_t hci_result = gpHci_ResultInvalid;
    gpHci_LeSetPrivacyModeCommand_t *params = NULL;
    BtDeviceAddress_t *peerIdentityAddress = NULL;
    Bool peerAddrTypeBit = false;
    gpHal_BleRpaHandle_t rpaHandle = BLE_RES_PR_INIT_RPA_HANDLE();
    Bool hal_result = false;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    GP_LOG_PRINTF("Set privacy mode",0);

    if (gpBle_IsResolvingListChangeForbidden())
    {
        GP_LOG_PRINTF("Cannot set privacy mode when resolving active",0);
        return gpHci_ResultCommandDisallowed;
    }

    params = &pParams->LeSetPrivacyMode;

    if (params->privacyMode >= gpHci_PrivacyModeInvalid)
    {
        GP_LOG_PRINTF("Invalid privacy mode: %x",0, params->privacyMode);
        return gpHci_ResultInvalidHCICommandParameters;
    }

    peerAddrTypeBit = BLE_ADV_ADDR_TYPE_TO_HAL_ADDR_BIT(params->peerIdentityAddressType);
    peerIdentityAddress = &params->peerIdentityAddress;

    hal_result = gpHal_BleRpa_MatchPeer(&rpaHandle, peerAddrTypeBit, peerIdentityAddress);
    if (hal_result && BLE_RES_PR_IS_VALID_HANDLE(rpaHandle))
    {
        hal_result = gpHal_BleRpa_SetPrivacyMode(rpaHandle, params->privacyMode);
        GP_ASSERT_DEV_INT(gpHal_ResultSuccess==hal_result);
        hci_result = gpHci_ResultSuccess;
    }
    else
    {
        hci_result = gpHci_ResultInvalidHCICommandParameters;
    }
    return hci_result;
/* </CodeGenerator Placeholder> Implementation_gpBle_SetPrivacyMode */
}
gpHci_Result_t gpBle_VsdSetRpaPrand(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
/* <CodeGenerator Placeholder> Implementation_gpBle_VsdSetRpaPrand */
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    GP_LOG_PRINTF("set vsd rpa",0);

    if(pParams->VsdSetRpaPrand.enableTimeout)
    {
        if (!gpSched_ExistsEventArg(BleResPrAddr_cbPrandTimeout, NULL))
        {
            gpSched_ScheduleEventInSeconds(BleResPr_timeout, BleResPrAddr_cbPrandTimeout, NULL);
        }
    }
    else
    {
        while(gpSched_ExistsEventArg(BleResPrAddr_cbPrandTimeout, NULL))
        {
            gpSched_UnscheduleEventArg(BleResPrAddr_cbPrandTimeout, NULL);
        }
    }

    BleResPrAddr_SetRandomPartOfPrand(pParams->VsdSetRpaPrand.prand);

    GP_LOG_PRINTF("pr:%lx to:%x",0,(unsigned long)pParams->VsdSetRpaPrand.prand, pParams->VsdSetRpaPrand.enableTimeout);

    return gpHci_ResultSuccess;
/* </CodeGenerator Placeholder> Implementation_gpBle_VsdSetRpaPrand */
}
gpHci_Result_t gpBle_VsdReadResolvingListCurrentSize(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
/* <CodeGenerator Placeholder> Implementation_gpBle_VsdReadResolvingListCurrentSize */
    // Returns the current number of used entries in the resolving list
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    GP_LOG_PRINTF("read current resolving list size",0);
    pEventBuf->payload.commandCompleteParams.returnParams.resolvingListSize = gpHal_BleRpaGetListSize();
    return gpHci_ResultSuccess;
/* </CodeGenerator Placeholder> Implementation_gpBle_VsdReadResolvingListCurrentSize */
}
gpHci_Result_t gpBle_VsdSetResolvingListMaxSize(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
/* <CodeGenerator Placeholder> Implementation_gpBle_VsdSetResolvingListMaxSize */
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    UInt8 size = pParams->VsdSetResolvingListMaxSize.maxSize;
    if ( gpBle_IsResolvingListChangeForbidden() )
    {
        GP_LOG_PRINTF("Due to another process, it is not allowed to changed the ResolvingList size at this moment", 0);
        return gpHci_ResultCommandDisallowed;
    }
    if(size > GP_DIVERSITY_BLE_MAX_NR_OF_RESOLVINGLIST_ENTRIES)
    {
        GP_LOG_PRINTF("Cannot increase size over the HW supported size", 0);
        return gpHci_ResultConnectionRejectedduetoLimitedResources;
    }
    if(size < gpHal_BleRpaGetListSize())
    {
        GP_LOG_PRINTF("Cannot shrink size below what is already in use", 0);
        return gpHci_ResultInvalidHCICommandParameters;
    }
    gpHal_BleRpaSetMaxListSize(size);
    return gpHci_ResultSuccess;

/* </CodeGenerator Placeholder> Implementation_gpBle_VsdSetResolvingListMaxSize */
}
