/*
 *   Copyright (c) 2018, Qorvo Inc
 *
 *   BleModule is the interface to the host-stack. This file is intended to be modified by the customer according to the customers needs.
 *   Implementation of BleModule
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

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLEMODULE

/* <CodeGenerator Placeholder> General */
#define _USE_BLE_
#ifdef GP_DIVERSITY_DEVELOPMENT
#ifndef GP_BLE_MULTI_ADV_SUPPORTED
#define GP_LOCAL_LOG
#endif //GP_BLE_MULTI_ADV_SUPPORTED
#endif //GP_DIVERSITY_DEVELOPMENT
/* </CodeGenerator Placeholder> General */

/* <CodeGenerator Placeholder> Includes */
#include "global.h"
#include "gpLog.h"
#include "gpSched.h"
#include "cordioBleHost.h"
#include "gpAssert.h"
#include "gpPoolMem.h"
#include "gpHci_types.h"
#include "BleModule.h"
#include "BleModule_Defs.h"
#include "gpUtils_RingBuffer.h"
#include "gpBleConnectionQueue.h"
#include "gpBlePeripheralConnectionStm.h"
#include "cordioAppFramework.h"

/* Includes from Host-stack are here needed to access app_db */
#include "wsf_types.h"
#include "wsf_msg.h"
#include "wsf_buf.h"

#include "app_api.h"
#include "app_main.h"
#include "att_api.h"

#include "app_db.h"
#ifdef GP_BLEPERIPHERAL_DIVERSITY_WDXS
#include "wdxs_api.h"
#endif
/* </CodeGenerator Placeholder> Includes */

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> Macro */

#ifndef GP_BLE_SECURITY_TIMEOUT
#define GP_BLE_SECURITY_TIMEOUT 10000000 //10s
#endif //GP_BLE_SECURITY_TIMEOUT

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
/* Provide shared prototype for DmDevWhiteListRemove and DmDevWhiteListAdd */
typedef void (*DmDevWhiteListHandler_t)(UInt8 addrType, UInt8* pAddr);

typedef struct ConfirmParams_s
{
    UInt8 linkId;
    BleModule_Result_t status;
} ConfirmParams_t;

/* </CodeGenerator Placeholder> TypeDefinitions */

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticData */
static wsfHandlerId_t BleModule_DmHandlerId;
static wsfHandlerId_t BleModule_AttHandlerId;

static BleModule_Cfg_t BleModule_Cfg[GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS];
static UInt8 BleModule_ConnIdList[GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS];
static UInt8 BleModule_LastReceivedNew = BLE_MODULE_LINK_ID_NONE;

static UInt8 BleModule_LocalStr[HCI_ADV_DATA_LEN];
static UInt8 BleModule_SlaveSecurityList[GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS];

#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT
static BleModule_Client_DiscEvtCbs_t serviceDiscCb;
#endif

/* Array to store disconnection request status for each linkId */
static Bool BleModulePeripheral_DisconnectionRequested[GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS];

appDbHdl_t static dbHdlsAddedToResolveList[GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS];

GP_UTILS_RING_BUFFER(BleModule_PrivacyModeQueuedHdl, appDbHdl_t, GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS);

/* </CodeGenerator Placeholder> StaticData */

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticFunctionPrototypes */
static void BleModuleItf_ProcAttMsg(attEvt_t* pMsg);
static void BleModuleItf_ProcDmMsg(dmEvt_t* pMsg);
static UInt8 BleModuleItf_GetLinkIdByAppDbHdl(appDbHdl_t dbHdl);
static UInt8 BleModuleItf_GetAdvertisingLinkId(void);
static void BleModuleItf_GetBondedDevices(void);
static BleModule_Result_t BleModuleItf_HandleWhiteList(UInt8 Length, UInt8* pLinkIdList, DmDevWhiteListHandler_t DmDevWhiteListHandler);
static void BleModuleItf_UpdateDpHdlInAppDb(dmConnId_t connId, appDbHdl_t dbHdl);

static void BleModuleItf_PopulateResolvingList(void);
static void BleModuleItf_SetDeviceFilteringPolicy(gpBlePeripheralConnectionStm_AdvType_t AdvType, UInt8 *pLinkIdList);
static void BleModuleItf_LoadCordioAppFrameworkPointers(UInt8 linkId);
static void BleModuleItf_regenerateECCKeys(void);
static void BleModuleItf_MarkDirectAdvertiseAsInvalid(void* pLinkId);

static void BleModuleItf_EnqueuePeerToRestorePrivacyMode(appDbHdl_t hdl);
static void BleModuleItf_ProcessPeersWaitingForRestoringPrivacyMode(void);
static void BleModuleItf_CheckAndEnqueueRestoringPrivacyMode(dmEvt_t* pMsg);

static Bool BleModuleItf_CheckIfDmConnCloseIndReasonIndicatesDirectAdvStop(UInt8 reason);

//Initialize application callbacks.
BleModule_Peripheral_Cb_t peripheralCb =
{
    .openConnCb = NULL,
    .closeConnCb = NULL,
    .authReqCb = NULL,
    .encCb = NULL,
    .pairCb = NULL,
    .secKeyCb = NULL,
    .unbindConnCb = NULL,
    .UpdateConnCb = NULL,
    .advStartedCb = NULL,
    .addrResolvingCb = NULL,
};



static void BleModuleItf_cbAdvStarted(UInt8 linkId, UInt8 status);
static void BleModuleItf_cbOpenConfirmResult(UInt8 linkId, UInt8 status);
static void BleModuleItf_cbCloseConfirmResult(UInt8 linkId, UInt8 status);
static void BleModuleItf_cbUnbindConfirmResult(UInt8 linkId, UInt8 status);
static void BleModuleItf_cbAuthenticationRequest(UInt8 linkId);
static void BleModuleItf_cbUpdateConfirmResult(UInt8 linkId, UInt8 status, BleModule_ConnectionParametersIndCnf_t connParams);
static void BleModuleItf_cbEncryptionConfirmResult(UInt8 linkId, UInt8 status, Bool usingLtk);
static void BleModuleItf_cbPairingConfirmResult(UInt8 linkId, UInt8 status, UInt8 authAndBondFlags);
static void BleModuleItf_UnscheduleSlaveSecurityRequest(UInt8 linkId);
static void BleModuleItf_SendSlaveSecurityRequest(void* schedLinkId);
static void BleModuleItf_cbSecurityKeyExchangedIndication(UInt8 linkId, BleModule_SecurityKeyExchangedInd_t keyExchangedInfo);
static void BleModuleItf_cbAddresResolvingConfiguredIndication(UInt8 status);

#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT
static void BleModule_Client_ServiceConfigurationFinishedCb(UInt8 linkId);
#endif
/* </CodeGenerator Placeholder> StaticFunctionPrototypes */

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/
/* <CodeGenerator Placeholder> StaticFunctionDefinitions */

/* Find the corresponding linkId of the given bond handle.
   If the given bond handle is NULL, then return BLE_MODULE_LINK_ID_NONE. */
static UInt8 BleModuleItf_GetLinkIdByAppDbHdl(appDbHdl_t dbHdl)
{
    appDbHdl_t dbHdlIndx = NULL;
    UIntLoop i;

    if(dbHdl == NULL)
    {
        return BLE_MODULE_LINK_ID_NONE;
    }

    /* dbHdlIndx is NULL if there is not any paired device */
    dbHdlIndx = AppDbGetNextRecordBare(APP_DB_HDL_NONE);

    /* iterate over the bonds but stop as soon as either bonds or links are all visited */
    for(i = 0; dbHdlIndx != APP_DB_HDL_NONE && i < GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS; i++)
    {
        if(dbHdlIndx == dbHdl)
        {
            /* Match, linkId == index */
            return i;
        }

        /* dbHdlIndx is NULL after the last entry in the bonds table */
        dbHdlIndx = AppDbGetNextRecordBare(dbHdlIndx);
    }

    return BLE_MODULE_LINK_ID_NONE;
}

/* Find the first advertising link or return BLE_MODULE_LINK_ID_NONE if none is found */
static UInt8 BleModuleItf_GetAdvertisingLinkId(void)
{
    UIntLoop i;

    for(i = 0; i < GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS; i++)
    {
        if(gpBlePeripheralConnectionStm_IsAdvertising(i) == gpBlePeripheralConnectionStmResult_Success)
        {
            return i;
        }
    }

    return BLE_MODULE_LINK_ID_NONE;
}

/* Function responsible for scanning appDb. Each record having InUse and Valid set will generate
   an EvBonded on the connection stateMachine */
static void BleModuleItf_GetBondedDevices(void)
{
    appDbHdl_t dbHdlIndx = NULL;
    UIntLoop i;

    dbHdlIndx = AppDbGetNextRecordBare(APP_DB_HDL_NONE);

    for(i = 0; i < GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS; i++)
    {
        if(AppDbRecordInUse(dbHdlIndx))
        {
            gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventBonded, i);
        }
        dbHdlIndx = AppDbGetNextRecordBare(dbHdlIndx);
    }
}

/* Handling of common DM events */
static void BleModuleItf_ProcDmMsg(dmEvt_t* pMsg)
{
    switch(pMsg->hdr.event)
    {
        case DM_ADV_START_IND:
        {
            UInt8 linkId;
            linkId = BleModuleItf_GetAdvertisingLinkId();
            if(pMsg->hdr.status == HCI_SUCCESS)
            {
                BleModuleItf_cbAdvStarted(linkId, BleModule_Success);
            }
            else
            {
                BleModuleItf_cbAdvStarted(linkId, BleModule_UnknownError);
            }
            GP_LOG_PRINTF("INFO: Advertising start", 0);
            break;
        }
        case DM_ADV_STOP_IND:
        {
            UInt8 linkId;
            UInt8 status;
            /* Note: this callback is only received if the advertising is stopped due to time out. */
            /* No response on advertisement, send connection fail to all connections */
            GP_LOG_PRINTF("INFO: Advertising stopped", 0);

            /*
            * When LinkLayer Privacy is enabled during disconnect we might want to restore peer privacy mode to device mode
            * Setting privacy mode is not allowed during advertising so we postpone the action until DM_ADV_STOP_IND occurs
            * This is safe place to set privacy mode because we are sure that we have just stopped undirected advertising
            */
            BleModuleItf_ProcessPeersWaitingForRestoringPrivacyMode();

            /* Questionable: First check if connectionStm is advertising or send ConnectionFail to each component
               For now first do the check, should be no problem to send ConnectFail to each STM, it only introduces more
               CPU load.
               Note: in this case, if the STM triggering the advetisement has left advertising state, the ConnFail Event is
               not send at all. Should be no issue. */
            linkId = BleModuleItf_GetAdvertisingLinkId();
            if(IS_VALID_LINK_ID(linkId))
            {
                BleModule_ConnIdList[linkId] = DM_CONN_ID_NONE;
                gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventConnFail, linkId);
                /* Disconnection requested while advertising. */
                if(BleModulePeripheral_DisconnectionRequested[linkId])
                {
                    status = BleModule_LinkNotConnected;
                    BleModuleItf_cbCloseConfirmResult(linkId, BleModule_Success);
                    /* Reset the variable BleModulePeripheral_DisconnectionRequested */
                    BleModulePeripheral_DisconnectionRequested[linkId] = false;
                }
                else
                {
                    status = BleModule_OpenFailNoResponse;
                }
                BleModuleItf_cbOpenConfirmResult(linkId, status);
            }
            break;
        }
       case DM_CONN_CLOSE_IND:
        {
            UInt8 linkId;
            linkId = BleModuleItf_GetLinkIdByConnId(pMsg->hdr.param);

            /*
            * Typically updating privacy mode is done in DM_CONN_CLOSE_IND in cordioAppFramework
            * but when advertise is active we need to process this in special way
            * Restoring privacy mode is enqueued here. Real setting will be done when advertise is stopped
            * Indication of advertise stop is done via:
            *   DM_ADV_STOP_IND for undirected advertisments
            *   DM_CONN_CLOSE_IND for direct advertisments (with reason == 0)
            * In case of this two events we process restoring privacy mode for enqueued peers
            */
            BleModuleItf_CheckAndEnqueueRestoringPrivacyMode(pMsg);

            if(IS_VALID_LINK_ID(linkId))
            {
                GP_LOG_SYSTEM_PRINTF("INFO: Connection %d (linkId: %u) closed, reason %d", 0, pMsg->hdr.param, linkId, pMsg->connClose.reason);
                BleModule_ConnIdList[linkId] = DM_CONN_ID_NONE;
                if(BleModuleItf_CheckIfDmConnCloseIndReasonIndicatesDirectAdvStop(pMsg->connClose.reason))
                {
                    /*
                     * When LinkLayer Privacy is enabled during disconnect we might want to restore privacy mode to device mode
                     * Setting privacy mode is not allowed during advertising so in case direct advertise was active
                     * we postpone the action until DM_CONN_CLOSE_IND with reason == 0 occurs
                     * This is safe place to set privacy mode because we have just stopped direct advertising
                     */
                    BleModuleItf_ProcessPeersWaitingForRestoringPrivacyMode();
                }
                /*              No need to distinguish here. If Closed by App, that event is already triggered
                if (pMsg->connClose.reason == gpHci_ResultConnectionTerminatedByLocalHost)
                {
                    gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventConnCloseApp, linkId);
                }
                else
                {
                    gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventConnCloseExt, linkId);
                }*/
                if(linkId == BleModule_LastReceivedNew)
                {
                    BleModule_LastReceivedNew = BLE_MODULE_LINK_ID_NONE;
                }
                gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventConnCloseExt, linkId);
                BleModuleItf_cbCloseConfirmResult(linkId, BleModule_Success);
            }
            else
            {
                GP_LOG_PRINTF("WARNING: Unknown connection %d closed, reason %d", 0, pMsg->hdr.param, pMsg->connClose.reason);
            }
            break;
        }
        case DM_RESET_CMPL_IND:
        {
            BleModuleItf_regenerateECCKeys();

            BleModule_RestoreAndSetLocalIrk();

            if(BleModule_Cfg[0].pLinkLayerConfig->useLinkLayerPrivacy)
            {
                BleModuleItf_PopulateResolvingList();
                BleModule_SetAddressResolving(true);
                BleModule_SetResolvableAddressGenerationTimeout(BleModule_Cfg[0].pLinkLayerConfig->rpaGenerationIntervalS);
            }
            else
            {
                BleModule_SetAddressResolving(false);
            }

            // start link state machines
            for(UInt8 linkId=0; linkId<GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS; linkId++)
            {
                /* Reset the variable BleModulePeripheral_DisconnectionRequested */
                BleModulePeripheral_DisconnectionRequested[linkId] = false;
                gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventBLEHostResetComplete, linkId);
            }
            BleModuleItf_GetBondedDevices();

        GP_ASSERT_DEV_EXT(BleModule_Cfg[0].pPhyConfig != NULL);
        BleModule_SetPreferredPhy(BleModule_Cfg[0].pPhyConfig->selectedPhy, BleModule_Cfg[0].pPhyConfig->txPhyTypeMask, BleModule_Cfg[0].pPhyConfig->rxPhyTypeMask);

        GP_LOG_PRINTF("INFO: BLE host Reset Complete", 0);
        //Callback to Application layer
        BleModule_cbResetDoneIndication();
        break;
    }

    case DM_PHY_UPDATE_IND:
    {
        GP_LOG_PRINTF("INFO: Preferred PHY updated. Status %x, TxMask %x, RxMask %x", 0, pMsg->phyUpdate.status, pMsg->phyUpdate.txPhy, pMsg->phyUpdate.rxPhy);
        break;
    }
    case DM_PHY_READ_IND:
    {
        GP_LOG_PRINTF("INFO: BleModule, connection %d PhyRead callback", 0, pMsg->hdr.param);
        break;
    }
    case DM_CONN_OPEN_IND:
    {
        /* Handled by callback cordioAppFramework_cbConnOpen triggered from AppSlave, AppSlaveSecDmProcMsg */
        break;
    }
    case DM_SEC_ENCRYPT_IND:
    {
        UInt8 linkId;

        linkId = BleModuleItf_GetLinkIdByConnId(pMsg->hdr.param);
        GP_LOG_PRINTF("INFO: Connection %d (linkId: %u) connection encrypted", 0, pMsg->hdr.param, linkId);
        if (IS_VALID_LINK_ID(linkId))
        {
            Bool usingLtk = pMsg->encryptInd.usingLtk;
            gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventSecurityEstablished, linkId);
            BleModuleItf_cbEncryptionConfirmResult(linkId, BleModule_Success, usingLtk);
        }
        break;
    }
    case DM_SEC_ENCRYPT_FAIL_IND:
    {
        UInt8 linkId = 0;

        linkId = BleModuleItf_GetLinkIdByConnId(pMsg->hdr.param);
        GP_LOG_PRINTF("WARNING: Connection %d (linkId: %u) encryption failed", 0, pMsg->hdr.param, linkId);
        if (IS_VALID_LINK_ID(linkId))
        {
            // Bluetooth SIG erratum 10734: 2.3.6 Repeated Attempts:
            // Note:
            // This is the most resource heavy implementation,
            // it is possible to relax this implementation to only regenerate
            // after 3 failed pairings
            gpSched_ScheduleEvent(0, BleModuleItf_regenerateECCKeys);
            gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventSecurityFail, linkId);
            /*DM_SEC_ENCRYPT_FAIL_IND does not provide any usable information so we use false here */
            BleModuleItf_cbEncryptionConfirmResult(linkId, BleModule_SecurityPairingFailed, USING_LTK_DATA_INVALID);
        }
        break;
    }
    case DM_SEC_PAIR_FAIL_IND:
    {
        UInt8 linkId = 0;
        linkId = BleModuleItf_GetLinkIdByConnId(pMsg->hdr.param);
        GP_LOG_PRINTF("WARNING: Connection %d (linkId: %u) pairing failed", 0, pMsg->hdr.param, linkId);
        if (IS_VALID_LINK_ID(linkId))
        {
            gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventPairFail, linkId);
            /* DM_SEC_PAIR_FAIL_IND is not providing this info about auth and bond flags */
            BleModuleItf_cbPairingConfirmResult(linkId, BleModule_SecurityPairingFailed, AUTH_AND_BOND_FLAGS_INVALID);
        }
        BleModule_LastReceivedNew = BLE_MODULE_LINK_ID_NONE;
        break;
    }
    case DM_SEC_AUTH_REQ_IND:
    {
        UInt8 linkId = BleModuleItf_GetLinkIdByConnId(pMsg->hdr.param);
        GP_LOG_PRINTF("INFO: DM_SEC_AUTH_REQ_IND received on linkId %u", 0, linkId);
        if (IS_VALID_LINK_ID(linkId))
        {
            BleModuleItf_cbAuthenticationRequest(linkId);
        }
        break;
    }
    case DM_SEC_COMPARE_IND:
    {
        /* Here user nummeric needs to be hooked up */
        GP_LOG_PRINTF("INFO: DM_SEC_COMPARE_IND", 0);
        break;
    }
    case DM_ERROR_IND:
    {
        GP_LOG_SYSTEM_PRINTF("ERROR: General error, status %d", 0, pMsg->hdr.status);
        break;
    }
    case DM_SEC_PAIR_CMPL_IND:
    {
        UInt8 linkId = 0;

        linkId = BleModuleItf_GetLinkIdByConnId(pMsg->hdr.param);
        GP_LOG_PRINTF("INFO: PairComplete callback received connection %d (linkId: %u)", 0, pMsg->hdr.param, linkId);
        if (IS_VALID_LINK_ID(linkId))
        {
            // Bluetooth SIG erratum 10734: 2.3.6 Repeated Attempts:
            // Note:
            // This is the most resource heavy implementation,
            // it is possible to relax this implementation to only regenerate
            // after 3 successful pairings
            gpSched_ScheduleEvent(0, BleModuleItf_regenerateECCKeys);

            gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventPairComplete, linkId);
            UInt8 authorizationAndBondingFlags = pMsg->pairCmpl.auth;
            BleModuleItf_cbPairingConfirmResult(linkId, BleModule_Success, authorizationAndBondingFlags);
#ifdef GP_DIVERSITY_DEVELOPMENT
            appDbHdl_t dbHdl = AppDbGetHdl(pMsg->hdr.param);
            if (APP_DB_HDL_NONE != dbHdl)
            {
                AppDbDumpKey(dbHdl, DM_KEY_LOCAL_LTK);
            }
#endif // GP_DIVERSITY_DEVELOPMENT
        }
            BleModule_LastReceivedNew = BLE_MODULE_LINK_ID_NONE;
            break;
        }

        case DM_PRIV_SET_ADDR_RES_ENABLE_IND:
        {
            /* Unfortunatelly here we only get information if pending operation (enable/disable) was successfull. We do not have information about final state */
            GP_LOG_PRINTF("INFO: Addres resolving configuration status: %u", 0, pMsg->setAddrResEnable.status);
            BleModuleItf_cbAddresResolvingConfiguredIndication(pMsg->setAddrResEnable.status);
            break;
        }
        case DM_CONN_UPDATE_IND:
        {
            BleModule_ConnectionParametersIndCnf_t ConnectParameters;
            UInt8 linkId = BleModuleItf_GetLinkIdByConnId(pMsg->hdr.param);

            if(IS_VALID_LINK_ID(linkId))
            {
                GP_LOG_PRINTF("INFO: Connection %d (linkId: %u) ParUpdated. Result %d, Interval %d, Latency %d, svTimeout %d", 0, pMsg->hdr.param, linkId,
                              pMsg->connUpdate.status, pMsg->connUpdate.connInterval, pMsg->connUpdate.connLatency, pMsg->connUpdate.supTimeout);

                /* Copy received parameters */
                ConnectParameters.Status = pMsg->connUpdate.status;
                ConnectParameters.ConnectionInterval = pMsg->connUpdate.connInterval;
                ConnectParameters.ConnectionLatency = pMsg->connUpdate.connLatency;
                ConnectParameters.ConnectionSupervisionTimeout = pMsg->connUpdate.supTimeout;

                BleModuleItf_cbUpdateConfirmResult(linkId, BleModule_Success, ConnectParameters);
            }
            break;
        }
        case DM_ADV_NEW_ADDR_IND:
        {
             GP_LOG_PRINTF("INFO: New RPA: %x:%x:%x:%x:%x:%x", 0, pMsg->advNewAddr.addr[5],
                          pMsg->advNewAddr.addr[4],
                          pMsg->advNewAddr.addr[3],
                          pMsg->advNewAddr.addr[2],
                          pMsg->advNewAddr.addr[1],
                          pMsg->advNewAddr.addr[0]);
            break;
        }
        case DM_READ_REMOTE_VER_INFO_IND:
        {
            if(pMsg->hdr.status == HCI_SUCCESS)
            {
                GP_LOG_PRINTF("INFO: Remote version indication", 0);
                GP_LOG_PRINTF("INFO: connId %d -> ver = 0x%2x | mfrName = 0x%4x | subversion = 0x%4x", 0, pMsg->hdr.param, pMsg->readRemVerInfo.version, pMsg->readRemVerInfo.mfrName, pMsg->readRemVerInfo.subversion);
            }
            else
            {
                GP_LOG_PRINTF("WARNING: Remote version indication failed", 0);
            }
            break;
        }
        case DM_SEC_KEY_IND:
        {
            BleModule_SecurityKeyExchangedInd_t exchangedKeyInfo;
            UInt8 linkId = BleModuleItf_GetLinkIdByConnId(pMsg->hdr.param);
            if(IS_VALID_LINK_ID(linkId))
            {
                exchangedKeyInfo.keyType = pMsg->keyInd.type;
                exchangedKeyInfo.securityLevel = pMsg->keyInd.secLevel;
                exchangedKeyInfo.keyLen = pMsg->keyInd.encKeyLen;
                BleModuleItf_cbSecurityKeyExchangedIndication(linkId, exchangedKeyInfo);
            }
            break;
        }
        case DM_SEC_PAIR_IND:
        {
            GP_LOG_PRINTF("INFO: DM callback 0x%x is handled by AppSlaveSecProcDmMsg", 0, pMsg->hdr.event);
            break;
        }
        case DM_PHY_SET_DEF_IND:
        {
            GP_LOG_PRINTF("WARNING: DM callback has no implementation DM_PHY_SET_DEF_IND", 0);
            break;
        }
        case DM_CONN_DATA_LEN_CHANGE_IND:
        {
            GP_LOG_PRINTF("WARNING: DM callback has no implementation DM_CONN_DATA_LEN_CHANGE_IND", 0);
            break;
        }
        case DM_CONN_READ_RSSI_IND:
        {
            GP_LOG_PRINTF("WARNING: DM callback has no implementation DM_CONN_READ_RSSI_IND", 0);
            break;
        }
        case DM_PRIV_ADD_DEV_TO_RES_LIST_IND:
        {
            /* Typically setting peer as added to resolve list is done in DM_PRIV_ADD_DEV_TO_RES_LIST_IND in cordioAppFramework
            * but for devices added to resolve list after POR (instead of during pairing) we need to process this here
            * because of missing connection context */
            appDbHdl_t dbHdl = dbHdlsAddedToResolveList[pMsg->hdr.param-1];
            if ((pMsg->hdr.status == HCI_SUCCESS) && (dbHdl != APP_DB_HDL_NONE))
            {
                AppDbSetPeerAddedToRl(dbHdl, TRUE);
            }
            GP_LOG_PRINTF("INFO: Link %d added to resolving list, result %u:", 0, pMsg->hdr.param, pMsg->hdr.status);
            break;
        }
        case DM_SEC_LTK_REQ_IND:
        {
            UInt8 linkId = 0;

            linkId = BleModuleItf_GetLinkIdByConnId(pMsg->hdr.param);
            GP_LOG_PRINTF("INFO: Security LTK Request callback received connection %d (linkId: %u)", 0, pMsg->hdr.param, linkId);
            if(IS_VALID_LINK_ID(linkId))
            {
                if(APP_DB_HDL_NONE == AppDbFindByLtkReq(pMsg->ltkReqInd.encDiversifier, pMsg->ltkReqInd.randNum))
                {
                    //reference BLUETOOTH SPECIFICATION Version 5.0 | Vol 6, Part D, Section 6.7 START ENCRYPTION WITHOUT LONG TERM KEY
                    //After LL_ENC_REQUEST/RESPONSE between two devices, on the slave device, LTK req is sent from the LL to the host
                    //with EDIV and Rand values. This allows the slave to calculate the LTK after reconnection in a fast way. At this point
                    //we know that we don't have the entry of the master (APP_DB_HDL_NONE) so the security establishment will fail anyway
                    //on the central side. The central gets notified as a LTK key request Negative Reply will be forwarded to the LL and this
                    //will in his turn trigger a LL_REJECT_IND to the master device. So this means we can tell the connection state machine
                    //that security has failed at this point on the slave side.
                    GP_LOG_SYSTEM_PRINTF("ERROR: No pairing information found to re-connect. Delete pairing information on BLE master first !", 0);
                    Bool usingLtk = true; /* DM_SEC_LTK_REQ_IND means that LTK was used */
                    gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventSecurityFail, linkId);
                    BleModuleItf_cbEncryptionConfirmResult(linkId, BleModule_SecurityPairingFailed, usingLtk);
                }
            }
        }
        case DM_CONN_IQ_REPORT_IND:
        case DM_CTE_REQ_FAIL_IND:
        case DM_CONN_CTE_RX_SAMPLE_START_IND:
        case DM_CONN_CTE_RX_SAMPLE_STOP_IND:
        case DM_CONN_CTE_TX_CFG_IND:
        case DM_CONN_CTE_REQ_START_IND:
        case DM_CONN_CTE_REQ_STOP_IND:
        case DM_CONN_CTE_RSP_START_IND:
        case DM_CONN_CTE_RSP_STOP_IND:
        {
            /* handled by BleModule_Client_ProcMsg() */
            break;
        }
        default:
        {
            GP_LOG_PRINTF("WARNING: Unprocessed DM Callback 0x%x", 0, pMsg->hdr.event);
            break;
        }
    }
    BleModule_cbDmCnf(pMsg);
}

/* Handling of common DM events */
static void BleModuleItf_ProcAttMsg(attEvt_t* pMsg)
{
    switch(pMsg->hdr.status)
    {
        case ATT_ERR_OVERFLOW:
        {
            GP_LOG_SYSTEM_PRINTF("ERROR: ATT OVERFLOW", 0);
            break;
        }
        case ATT_ERR_TIMEOUT:
        {
            GP_LOG_SYSTEM_PRINTF("ERROR: ATT TIMEOUT", 0);
            break;
        }
        default:
        {
            break;
        }
    }
    BleModule_cbAttsCnf(pMsg);
}

static void BleModule_DmHandler(wsfEventMask_t event, wsfMsgHdr_t* pMsg)
{
    if(pMsg != NULL)
    {
        /* process advertising and connection-related messages */
        AppSlaveProcDmMsg((dmEvt_t*)pMsg);

        /* process security-related messages */
        AppSlaveSecProcDmMsg((dmEvt_t*)pMsg);

#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT
        /* process client-related messages */
        BleModule_Client_ProcMsg(pMsg);
#endif // CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT

        /* Process all common messages here */
        BleModuleItf_ProcDmMsg((dmEvt_t*)pMsg);
    }
}

static void BleModule_AttHandler(wsfEventMask_t event, wsfMsgHdr_t* pMsg)
{
    if(pMsg != NULL)
    {
#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT
        /* process client-related messages */
        BleModule_Client_ProcMsg(pMsg);
#endif // CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT

        /* Process all common messages here */
        BleModuleItf_ProcAttMsg((attEvt_t*)pMsg);
    }
}

/**
 * @brief Callback function from BLE host device manager
 *
 * @param pDmEvt pointer to the event being passed to the application layer
 */
static void BleModule_DmCallback(dmEvt_t* pDmEvt)
{
    if(pDmEvt->hdr.event == DM_SEC_ECC_KEY_IND)
    {
        GP_LOG_PRINTF("Setting key", 0);
        DmSecSetEccKey(&pDmEvt->eccMsg.data.key);
        if((pAppSecCfg != NULL) && (pAppSecCfg->oob))
        {
            UInt8 oobLocalRandom[SMP_RAND_LEN];
            SecRand(oobLocalRandom, SMP_RAND_LEN);
            DmSecCalcOobReq(oobLocalRandom, pDmEvt->eccMsg.data.key.pubKey_x);
        }
    }
#ifdef GP_DIVERSITY_BLE_SILENT_CONNECTION_ACCEPTANCE
    else if(pDmEvt->hdr.event == DM_VENDOR_SPEC_IND)
    {
        GP_LOG_PRINTF("Receive Vendor specific HCI event : 0x%xl", 0, pDmEvt);

        // Note that dmEvt_t *pDmEvt  lives on stack
        if(gpHci_VsdSubEventCodeResolveRpaReq == pDmEvt->hdr.param)
        {
            if(CheckWsfBufAlloc(sizeof(gpHci_VsdSubEventResolveRpaRequestParams_t)))
            {
                gpHci_VsdSubEventResolveRpaRequestParams_t* pResolveRpaRequest;

                pResolveRpaRequest = WsfBufAlloc(sizeof(gpHci_VsdSubEventResolveRpaRequestParams_t));
                if(pResolveRpaRequest)
                {
                    MEMCPY(pResolveRpaRequest, pDmEvt->vendorSpec.param, sizeof(gpHci_VsdSubEventResolveRpaRequestParams_t));
                    gpSched_ScheduleEventArg(0, BleModule_TriggerResolveRpaReq, (void*)pResolveRpaRequest);
                }
                else
                {
                    // this should never happen
                    UInt8 SeqNbr = pDmEvt->vendorSpec.param[1];
                    GP_LOG_PRINTF("Unexpected WsfBufAlloc failure", 0);
                    // there must always be a response to a ResolveRpaReq
                    gpSched_ScheduleEventArg(0, BleModule_TriggerResolveRpaReq_NoBuff, (void*)(SeqNbr + 1));
                }
            }
            else
            {
                UInt8 SeqNbr = pDmEvt->vendorSpec.param[1];
                GP_LOG_PRINTF("  ResolveRpaReq not handled - insufficient memory", 0);
                // there must always be a response to a ResolveRpaReq
                gpSched_ScheduleEventArg(0, BleModule_TriggerResolveRpaReq_NoBuff, (void*)(SeqNbr + 1));
            }
        }
        else
        {
            // add more Vsd Subevents here
            GP_LOG_PRINTF("  vendor specific sub-event %d : not handled", 0, pDmEvt->hdr.param);
        }
    }
#endif /* GP_DIVERSITY_BLE_SILENT_CONNECTION_ACCEPTANCE */
    else
    {
        dmEvt_t* pMsg;
        uint16_t len;
        uint16_t extraLen = 0;

        /* Retrieve length of actual event that is received */
        len = DmSizeOfEvt(pDmEvt);

        switch(pDmEvt->hdr.event)
        {
            case DM_SCAN_REPORT_IND:
                extraLen = pDmEvt->scanReport.len;
                break;

            case DM_CONN_IQ_REPORT_IND:
                extraLen = 2 * pDmEvt->connIQReport.sampleCnt;
                break;
            default:
                break;
        }

        if ((CheckWsfMsgAlloc(len + extraLen))
            && ((pMsg = WsfMsgAlloc(len + extraLen)) != NULL))
        {
            MEMCPY(pMsg, pDmEvt, DmSizeOfEvt(pDmEvt));

            switch(pDmEvt->hdr.event)
            {
                case DM_SCAN_REPORT_IND:
                    pMsg->scanReport.pData = ((uint8_t*)pMsg + len);
                    MEMCPY(pMsg->scanReport.pData, pDmEvt->scanReport.pData, pDmEvt->scanReport.len);
                    break;

                case DM_CONN_IQ_REPORT_IND:
                    //DmEvent content copied above.
                    break;
                default:
                    break;
            }

            /* send to BleModule handler */
            WsfMsgSend(BleModule_DmHandlerId, pMsg);
        }
    }
}

/**
 * @brief function for processing ATT callbacks
 *
 * @param pEvt  pointer to ATT event structure
 */
static void BleModule_AttCback(attEvt_t* pEvt)
{
    attEvt_t* pMsg;

    //GP_LOG_PRINTF("BleModule_ATTS_AttCback",0);   //This print needs to be removed for audio quality testing!

    // Allocate buffer for attEvent and its data, as both may be overwritten/freed after this WSF msg handling
    if ((CheckWsfMsgAlloc(sizeof(attEvt_t) + pEvt->valueLen))
        && ((pMsg = WsfMsgAlloc(sizeof(attEvt_t) + pEvt->valueLen)) != NULL))
    {
        MEMCPY(pMsg, pEvt, sizeof(attEvt_t));
        // Copy the pValue buffer right after the attEvt_t
        pMsg->pValue = (uint8_t*)(pMsg + 1);
        MEMCPY(pMsg->pValue, pEvt->pValue, pEvt->valueLen);

        WsfMsgSend(BleModule_AttHandlerId, pMsg);
    }
}

BleModule_Result_t BleModule_PopulateWhiteList(BleModule_WhitelistOp_t BleModule_WhiteListOp, UInt8 Length, UInt8* pLinkIdList)
{
    /* <CodeGenerator Placeholder> Implementation_BleModule_PopulateWhiteList */
    BleModule_Result_t result = BleModule_NoOperation;
    UInt8 linkId;

    /* Check, if Length longer as number of connections return Length error */
    if(Length > GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS)
    {
        return BleModule_LengthError;
    }

    /* Check, manipulating Whitelist content while advertising is not allowed */
    linkId = BleModuleItf_GetAdvertisingLinkId();
    if(IS_VALID_LINK_ID(linkId))
    {
        GP_LOG_SYSTEM_PRINTF("ERROR: BleModule_PopulateWhiteList - advertising is in progress. Cannot manipulate whitelist", 0);
        return BleModule_AdvertisingInProgressError;
    }

    switch(BleModule_WhiteListOp)
    {
        case BleModule_WhiteListAdd:
            GP_LOG_SYSTEM_PRINTF("BleModuleItf_HandleWhiteList Add", 0);

            result = BleModuleItf_HandleWhiteList(Length, pLinkIdList, DmDevWhiteListAdd);
            break;

        case BleModule_WhiteListRemove:
            result = BleModuleItf_HandleWhiteList(Length, pLinkIdList, DmDevWhiteListRemove);
            break;

        case BleModule_WhiteListClear:
            DmDevWhiteListClear();
            result = BleModule_Success;
            break;

        default:
            break;
    }

    return result;
    /* </CodeGenerator Placeholder> Implementation_BleModule_PopulateWhiteList */
}

static void BleModuleItf_SetDeviceFilteringPolicy(gpBlePeripheralConnectionStm_AdvType_t AdvType, UInt8* pLinkIdList)
{
    /* <CodeGenerator Placeholder> BleModuleItf_SetDeviceFilteringPolicy */
    if(AdvType == gpBlePeripheralConnectionStm_ReconnectDirectedHigh ||
        AdvType == gpBlePeripheralConnectionStm_ReconnectDirectedLow ||
        AdvType == gpBlePeripheralConnectionStm_Reconnect)
    {
        /* First clear whitelist to ensure we reconnect to only one entry */
        BleModule_Result_t resultClear = BleModule_PopulateWhiteList(BleModule_WhiteListClear, 1, NULL);

        /* Load the given linkId into the whitelist */
        BleModule_Result_t resultPopulate = BleModule_PopulateWhiteList(BleModule_WhiteListAdd, 1, pLinkIdList);

        if(resultClear == BleModule_Success && resultPopulate == BleModule_Success)
        {
            /* Enable whitelist for both scan requests and connections */
            DmDevSetFilterPolicy(DM_FILT_POLICY_MODE_ADV, gpHci_AdvFilterPolicy_ScanWl_ConnWl);
        }
        else if(resultClear == BleModule_AdvertisingInProgressError && resultPopulate == BleModule_AdvertisingInProgressError)
        {
            /* This is expected to happen if we are not starting reconnection from WaitReconnect but iterate through
            configured reconnecting states. Otherwise extra attention is needed. */
            GP_LOG_SYSTEM_PRINTF("WARNING: Could not clear (result %u) or populate the whitelist (result: %u)", 0, resultClear, resultPopulate);
        }
        else
        {
            GP_LOG_SYSTEM_PRINTF("ERROR: Could not clear (result %u) or populate the whitelist (result: %u)", 0, resultClear, resultPopulate);
        }

    }
    else if(AdvType == gpBlePeripheralConnectionStm_NewConnection )
    {
        DmDevSetFilterPolicy(DM_FILT_POLICY_MODE_ADV, HCI_ADV_FILT_NONE);
    }
    /* </CodeGenerator Placeholder> BleModuleItf_SetDeviceFilteringPolicy */
}


static BleModule_Result_t BleModuleItf_HandleWhiteList(UInt8 Length, UInt8* pLinkIdList, DmDevWhiteListHandler_t DmDevWhiteListHandler)
{
    BleModule_Result_t result = BleModule_Success;
    UIntLoop i = 0;
    appDbHdl_t AppDbHdl = APP_DB_HDL_NONE;

    for(i = 0; i < Length; i++)
    {
        /* If wrong LinkId can be inserted, set error and continue with other entries */
        if(!IS_VALID_LINK_ID(*(pLinkIdList + i)))
        {
            result = BleModule_LinkIdInvalid;
            continue;
        }

        /* Valid LinkId already checked, no addtional check needed here */
        AppDbHdl = AppDbGetHdlByLinkId(*(pLinkIdList + i));
        /* Check AppDbHandle in use */
        if(AppDbRecordInUse(AppDbHdl))
        {
            UInt8* peerAddr = AppDbGetPeerAddress(AppDbHdl);
            UInt8 peerAddrType = AppDbGetPeerAddressType(AppDbHdl);

            DmDevWhiteListHandler(peerAddrType, peerAddr);
        }
        else
        {
            /* Invalid linkId error has higher priority */
            if(result != BleModule_LinkIdInvalid)
            {
                result = BleModule_UnknownError;
            }
        }
    }
    return result;
}


/*
* This function is called after POR (if address resolving is enabled) to recreate resolving list according to
* non volatile database of known (bonded) devices. dbHdlsAddedToResolveList needs to be populated here because
* cordioAppFramework is processing DM_PRIV_ADD_DEV_TO_RES_LIST_IND which is a result of DmPrivAddDevToResList
* with assumption that there is existing connection which can be used to read proper dbHdl.
* In this case we do not have connection context so its a workaround. Lack od proper dbHdl will cause
* that cordioAppFramework wont mark device as resolved so we do additional processing in BleModuleItf_ProcDmMsg
*/
static void BleModuleItf_PopulateResolvingList(void)
{
    appDbHdl_t dbHdl = APP_DB_HDL_NONE;
    dbHdl = AppDbGetNextRecordBare(dbHdl);

    UIntLoop i;
    /* iterate over the bonds but stop as soon as either bonds or links are all visited */
    for(i = 0; dbHdl != APP_DB_HDL_NONE && i < GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS; i++)
    {
        if(AppDbRecordInUse(dbHdl))
        {
            /* Record in use and device added to RPL before */
            dmSecKey_t* pKey = AppDbGetKey(dbHdl, DM_KEY_IRK, NULL);
            if(pKey != NULL) /* IRK was there */
            {
                UInt8* PeerIrkKey = pKey->irk.key;
                UInt8* PeerAddr = pKey->irk.bdAddr;
                UInt8  PeerAddrType = pKey->irk.addrType;
                GP_LOG_SYSTEM_PRINTF("INFO: Adding peer (dbHdl %lx) to resolve list - AddrType 0x%x, BdAddr %02x:%02x:%02x:%02x:%02x:%02x", 0, (unsigned long int)dbHdl, PeerAddrType, PeerAddr[5], PeerAddr[4], PeerAddr[3], PeerAddr[2], PeerAddr[1], PeerAddr[0]);
                GP_LOG_SYSTEM_PRINTF("INFO: PeerIrk %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", 0, PeerIrkKey[0], PeerIrkKey[1], PeerIrkKey[2], PeerIrkKey[3], PeerIrkKey[4], PeerIrkKey[5], PeerIrkKey[6], PeerIrkKey[7], PeerIrkKey[8], PeerIrkKey[9], PeerIrkKey[10], PeerIrkKey[11], PeerIrkKey[12], PeerIrkKey[13], PeerIrkKey[14], PeerIrkKey[15]);
                dbHdlsAddedToResolveList[i] = dbHdl;
                DmPrivAddDevToResList(PeerAddrType, PeerAddr, PeerIrkKey, DmSecGetLocalIrk(), FALSE, i+1); /* will set network privacy also */
                BleModule_RestorePrivacyModeToDevice(dbHdl); /* restore device privacy if necessary */
            }
            else if(AppDbGetPeerAddedToRl(dbHdl))
            {
                GP_LOG_SYSTEM_PRINTF("WARNING:Peer did not exchange IRK Key but was added to resolve list, will generate zero-irk and add to resolving list", 0);
                dmSecKey_t zeroKey;
                BdaCpy(zeroKey.irk.bdAddr, AppDbGetPeerAddress(dbHdl));
                zeroKey.irk.addrType = AppDbGetPeerAddressType(dbHdl);
                memset(zeroKey.irk.key, 0, 16); /* generate zero irk */
                dbHdlsAddedToResolveList[i] = dbHdl;
                DmPrivAddDevToResList(zeroKey.irk.addrType, zeroKey.irk.bdAddr, zeroKey.irk.key, DmSecGetLocalIrk(), FALSE, i+1); /* will set network privacy also */
                /* There is no need to restore device privacy even if RPAO was not present. If peer is added to resolve list with all-zero IRK
                * only Scan Requests/Connection Request containig identity address will be accepted regardless used privacy mode */
            }
        }
        /* dbHdl is APP_DB_HDL_NONE after the last entry in the bonds table */
        dbHdl = AppDbGetNextRecordBare(dbHdl);
    }
}

static void BleModuleItf_LoadCordioAppFrameworkPointers(UInt8 linkId)
{
    pAppSlaveCfg = BleModule_Cfg[linkId].pAppSlaveCfg;
    pAppAdvCfg = BleModule_Cfg[linkId].pAppAdvCfg;
    pAppExtAdvCfg = BleModule_Cfg[linkId].pAppExtAdvCfg;
    pAppUpdateCfg = BleModule_Cfg[linkId].pAppUpdateCfg;
    pAppSecCfg = BleModule_Cfg[linkId].pAppSecCfg_Peripheral;
    pSmpCfg = BleModule_Cfg[linkId].pSmpCfg;
}

static void BleModuleItf_regenerateECCKeys(void)
{
    DmSecGenerateEccKeyReq();
}

static void BleModuleItf_UpdateDpHdlInAppDb(dmConnId_t connId, appDbHdl_t dbHdl)
{
    /* extern to avoid changing packetcraft code */
    extern appConnCb_t appConnCb[DM_CONN_MAX];
    appConnCb[connId - 1].dbHdl = dbHdl;
}

UInt8 BleModuleItf_GetConnIdByLinkId(UInt8 linkId, dmConnId_t* connId)
{
    if(!IS_VALID_LINK_ID(linkId))
    {
        return BleModule_LinkIdInvalid;
    }

    *connId = BleModule_ConnIdList[linkId];
    if(*connId == DM_CONN_ID_NONE || !AppConnIdOpenCon(*connId))
    {
        *connId = DM_CONN_ID_NONE;
        return BleModule_LinkNotConnected;
    }

    return BleModule_Success;
}

UInt8 BleModuleItf_GetLinkIdByConnId(dmConnId_t connId)
{
    appDbHdl_t dbHdl = AppDbGetHdl(connId);
    UInt8 linkId = BLE_MODULE_LINK_ID_NONE;

    /* if the connection is from a paired device, retrieve its linkId */
    if(AppDbRecordInUse(dbHdl))
    {
        linkId = BleModuleItf_GetLinkIdByAppDbHdl(dbHdl);
    }

    /* if the connection is not from a paired device, check if it's from a
       device which has just connected but not paired yet (if any) */
    if(linkId == BLE_MODULE_LINK_ID_NONE)
    {
        for(UInt8 i = 0; i < GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS; i++)
        {
            if(BleModule_ConnIdList[i] == connId)
            {
                linkId = i;
                break;
            }
        }
    }

    if(linkId == BLE_MODULE_LINK_ID_NONE)
    {
        GP_LOG_PRINTF("Could not find the linkId matching connId %d", 0, connId);
    }

    return linkId;
}

void BleModuleItf_DirectAdvertise(UInt8 linkId, UInt8 advType)
{
    appDbHdl_t AppDbHdl = AppDbGetHdlByLinkId(linkId);

    UInt8 addrType = AppDbGetPeerAddressType(AppDbHdl);
    UInt8* addr = AppDbGetPeerAddress(AppDbHdl);


    dmConnId_t connId = AppConnAccept(advType, addrType, addr, AppDbHdl);
    if(connId == DM_CONN_ID_NONE)
    {
        GP_LOG_SYSTEM_PRINTF("ERROR: AppConnAccept returned DM_CONN_ID_NONE. Probably link layer privacy is enabled and peer is not supporting it", 0);
        UInt8* pLinkId= (UInt8*) GP_POOLMEM_MALLOC(sizeof(UInt8));
        *pLinkId = linkId;
        //Direct advetise has not started. We need to mark this in ConnectionStm and in BleModule_ConnIdList
        gpSched_ScheduleEventArg(0, BleModuleItf_MarkDirectAdvertiseAsInvalid, (void*)pLinkId);
        return;
    }
    if(AppDbGetHdl(connId) != AppDbHdl)
    {
        /*
        * This is necessary because in some cases connId connected
        * with dbHdl changes (AppConnAccept will return first free connId)
        * which can cause inconsistency between linkId and connId in layers
        * which needs to resolve linkId / connid relationship for paired devices
        * please check: SDP005-1624 for more details
        */
        GP_LOG_SYSTEM_PRINTF("Updating dbHdl connected to connId %u", 0, connId);
        BleModuleItf_UpdateDpHdlInAppDb(connId, AppDbHdl);
    }
    // This needs to be undone when the directed advertisement is stopped or unsuccessful
    BleModule_ConnIdList[linkId] = connId;
}

static void BleModuleItf_cbAdvStarted(UInt8 linkId, UInt8 status)
{
    if(peripheralCb.advStartedCb != NULL)
    {
        peripheralCb.advStartedCb(linkId, status);
    }
    else
    {
        GP_LOG_PRINTF("Application callback for advertising started confirmation is not registered!", 0);
    }
}
static void BleModuleItf_cbOpenConfirmResult(UInt8 linkId, UInt8 status)
{
    if(peripheralCb.openConnCb != NULL)
    {
        peripheralCb.openConnCb(linkId, status);
    }
    else
    {
        GP_LOG_PRINTF("Application callback for Open Connection confirmation is not registered!", 0);
    }
}
static void BleModuleItf_cbCloseConfirmResult(UInt8 linkId, UInt8 status)
{
    if(peripheralCb.closeConnCb != NULL)
    {
        peripheralCb.closeConnCb(linkId, status);
    }
    else
    {
        GP_LOG_PRINTF("Application callback for Close Connection confirmation is not registered!", 0);
    }
}

static void BleModuleItf_cbUnbindConfirmResult(UInt8 linkId, UInt8 status)
{
    if(peripheralCb.unbindConnCb != NULL)
    {
        peripheralCb.unbindConnCb(linkId, status);
    }
    else
    {
        GP_LOG_PRINTF("Application callback for Unbind confirmation is not registered!", 0);
    }
}

static void BleModuleItf_cbUpdateConfirmResult(UInt8 linkId, UInt8 status, BleModule_ConnectionParametersIndCnf_t connParams)
{
    if(peripheralCb.UpdateConnCb != NULL)
    {
        peripheralCb.UpdateConnCb(linkId, status, connParams);
    }
    else
    {
        GP_LOG_PRINTF("Application callback for Update Connection confirmation is not registered!", 0);
    }
}

static void BleModuleItf_UnscheduleSlaveSecurityRequest(UInt8 linkId)
{
    if(gpSched_ExistsEventArg(BleModuleItf_SendSlaveSecurityRequest, &BleModule_SlaveSecurityList[linkId]))
    {
        gpSched_UnscheduleEventArg(BleModuleItf_SendSlaveSecurityRequest, &BleModule_SlaveSecurityList[linkId]);
        BleModule_SlaveSecurityList[linkId] = BLE_MODULE_LINK_ID_NONE;
    }
}

static void BleModuleItf_cbEncryptionConfirmResult(UInt8 linkId, UInt8 status, Bool usingLtk)
{
    BleModuleItf_UnscheduleSlaveSecurityRequest(linkId);

    if(peripheralCb.encCb != NULL)
    {
        BleModule_EncryptionInd_t encryptionInd;
        if(usingLtk)
        {
            encryptionInd.encryptionMethod=BleModule_ExistingLtkUsed;
        }
        else
        {
            encryptionInd.encryptionMethod=BleModule_NewLtkGenerated;
        }
        peripheralCb.encCb(linkId, status, encryptionInd);
    }
}

static void BleModuleItf_cbPairingConfirmResult(UInt8 linkId, UInt8 status, UInt8 authAndBondFlags)
{
    BleModuleItf_UnscheduleSlaveSecurityRequest(linkId);

    if(peripheralCb.pairCb != NULL)
    {
        peripheralCb.pairCb(linkId, status, authAndBondFlags);
    }
    else
    {
        GP_LOG_PRINTF("Application callback for Pairing confirmation is not registered!", 0);
    }
}

static void BleModuleItf_cbAddresResolvingConfiguredIndication(UInt8 status)
{
    if(peripheralCb.addrResolvingCb != NULL)
    {
        peripheralCb.addrResolvingCb(status);
    }
}

static void BleModuleItf_cbAuthenticationRequest(UInt8 linkId)
{
    if(peripheralCb.authReqCb != NULL)
    {
        BleModule_AuthenticationInd_t authInd;
        authInd.shouldDisplayPin = false; //In peripheral we just enter the key displayed by central
        authInd.pin = 0;
        authInd.pinLen = 0;
        peripheralCb.authReqCb(linkId, authInd);
    }
    else
    {
        GP_LOG_PRINTF("Application callback for authentication request is not registered!", 0);
    }
}

static void BleModuleItf_SendSlaveSecurityRequest(void* schedLinkId)
{
    UInt8 connId = 0xFF;
    UInt8* linkId = (UInt8*)schedLinkId;

    if(BleModule_Success == BleModuleItf_GetConnIdByLinkId(*linkId, &connId))
    {
        AppSlaveSecurityReq(connId);
    }

    BleModule_SlaveSecurityList[*linkId] = BLE_MODULE_LINK_ID_NONE;
}

static void BleModuleItf_cbSecurityKeyExchangedIndication(UInt8 linkId, BleModule_SecurityKeyExchangedInd_t keyExchangedInfo)
{
    if(peripheralCb.secKeyCb != NULL)
    {
        peripheralCb.secKeyCb(linkId, keyExchangedInfo);
    }
}

static void BleModuleItf_MarkDirectAdvertiseAsInvalid(void* pLinkId)
{
    UInt8  linkId = *((UInt8*)pLinkId);
    gpPoolMem_Free(pLinkId);
    BleModule_ConnIdList[linkId] = DM_CONN_ID_NONE;
    gpBlePeripheralConnectionStm_Result_t result = gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventConnCloseExt, linkId);
    if(result != gpBlePeripheralConnectionStmResult_Success)
    {
        GP_LOG_SYSTEM_PRINTF("ERROR: BleModuleItf_MarkDirectAdvertiseAsInvalid - gpBlePeripheralConnectionStm_SendEvent returned unexpected result %u", 0,
                             result);
    }
}

#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT
static void BleModule_Client_ServiceConfigurationFinishedCb(UInt8 linkId)
{
    GP_LOG_PRINTF("INFO: Service Configuration finished. Will read CAR and RPAO",0);
    BleModule_Gap_CarAndRpaoRequest(linkId);
}
#endif

static void BleModuleItf_CheckAndEnqueueRestoringPrivacyMode(dmEvt_t* pMsg)
{
    /* If close reason is indicating that direct advertise has ended, GetAdvertisingLinkId is still indicating
    *  that device is advertising which might enqueue restoring device privacy mode which is uncessesary */
    if(! BleModuleItf_CheckIfDmConnCloseIndReasonIndicatesDirectAdvStop(pMsg->connClose.reason)) {

        /* Its not ended direct advertise but real disconnection. Check if there is advertise active on other link
        *  If there is active advertise we need to postpone changing peer privacy mode until advertise is stopped
        *  If advertise is not active updating privacy should have been done in cordioAppFramework */
        if(IS_VALID_LINK_ID(BleModuleItf_GetAdvertisingLinkId()))
        {
            appDbHdl_t hdl = AppDbGetHdl(pMsg->hdr.param);
            /* check if hdl is valid and  privacy should be restored */
            if ((hdl != APP_DB_HDL_NONE) && AppDbGetPeerAddedToRl(hdl) && !AppDbGetPeerRpao(hdl))
            {
                BleModuleItf_EnqueuePeerToRestorePrivacyMode(hdl);
            }
        }
    }
}

/* Enqueue peer which should have changed privacy mode when advertising is stopped */
static void BleModuleItf_EnqueuePeerToRestorePrivacyMode(appDbHdl_t hdl)
{
    GP_ASSERT_DEV_EXT(hdl != APP_DB_HDL_NONE); /* should be checked before call */
    if (!gpUtils_RingBufferFull(BleModule_PrivacyModeQueuedHdl))
    {
        GP_LOG_PRINTF("Adding peer stored by dbHdl %lx to have privacy mode restored",0, (unsigned long int)hdl);
        gpUtils_RingBufferPut(BleModule_PrivacyModeQueuedHdl, hdl);
    }
    else
    {
        GP_LOG_SYSTEM_PRINTF("ERROR: BleModule_PrivacyModeQueuedHdl is full. Wont add dbHdl %lx", 0, (unsigned long int)hdl);
    }
}

/* Process setting privacy mode for awaiting peers. Should be called when advertise is not active */
static void BleModuleItf_ProcessPeersWaitingForRestoringPrivacyMode(void)
{
    while (!gpUtils_RingBufferEmpty(BleModule_PrivacyModeQueuedHdl))
    {
        appDbHdl_t hdl = gpUtils_RingBufferHead(BleModule_PrivacyModeQueuedHdl);
        BleModule_RestorePrivacyModeToDevice(hdl);
        gpUtils_RingBufferDiscardTail(BleModule_PrivacyModeQueuedHdl);
    }
}

static Bool BleModuleItf_CheckIfDmConnCloseIndReasonIndicatesDirectAdvStop(UInt8 reason)
{
    /* reason == 0 means that DM_CONN_CLOSE_IND means that directed advertise has timeouted or was cancelled */
    return (reason == 0);
}

/* </CodeGenerator Placeholder> StaticFunctionDefinitions */

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void BleModule_Peripheral_Init(void)
{
    /* <CodeGenerator Placeholder> BleModule_Peripheral_Init */

    /* register callback to the DM connection manager */
    DmConnRegister(DM_CLIENT_ID_APP, BleModule_DmCallback);

    /* register callback to the device manager Scan and Advertisement messages */
    DmRegister(BleModule_DmCallback);

    AttRegister(BleModule_AttCback);

    /* register callbacks to the attribute server */
    AttConnRegister(AppServerConnCback);

    /* handle events */
    BleModule_DmHandlerId = WsfOsSetNextHandler(BleModule_DmHandler);
    BleModule_AttHandlerId = WsfOsSetNextHandler(BleModule_AttHandler);

    /* Init module */
    BleModule_LoadDefault(GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS, BleModule_Cfg);

    /* To avoid uninitialized pointers, always load the AppFrameWork pointers with linkId zero config. linkId zero is always present */
    BleModuleItf_LoadCordioAppFrameworkPointers(0);
    gpBleConnectionQueue_Init();

    /* Initialize connection and even list */
    MEMSET(BleModule_ConnIdList, DM_CONN_ID_NONE, sizeof(BleModule_ConnIdList));
    MEMSET(BleModule_SlaveSecurityList, BLE_MODULE_LINK_ID_NONE, sizeof(BleModule_SlaveSecurityList));

    for(UInt8 linkId=0; linkId<GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS; linkId++)
    {
        /* Reset the variable BleModulePeripheral_DisconnectionRequested */
        BleModulePeripheral_DisconnectionRequested[linkId] = false;
    }


#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT
    serviceDiscCb.confFinishedCb = BleModule_Client_ServiceConfigurationFinishedCb;
    BleModule_Client_RegisterServiceDiscoveryEventsCb(serviceDiscCb);
#endif
    /* </CodeGenerator Placeholder> BleModule_Peripheral_Init */
}

void BleModule_Peripheral_RegisterCb(BleModule_Peripheral_Cb_t* peripheralAppCb)
{
    MEMCPY(&peripheralCb, peripheralAppCb, sizeof(BleModule_Peripheral_Cb_t));
}

void BleModule_OpenConnectionRequest(UInt8 linkId)
{
    /* <CodeGenerator Placeholder> Implementation_BleModule_OpenConnectionRequest */
    gpBlePeripheralConnectionStm_Result_t result;

    if(!IS_VALID_LINK_ID(linkId))
    {
        BleModuleItf_cbOpenConfirmResult(linkId, BleModule_LinkIdInvalid);
        return;
    }

    {
        result = gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventConnReq, linkId);
        if(result != gpBlePeripheralConnectionStmResult_Success)
        {
            BleModuleItf_cbOpenConfirmResult(linkId, BleModule_RequestNotProcessed);
            return;
        }
    }
    /* </CodeGenerator Placeholder> Implementation_BleModule_OpenConnectionRequest */
}
void BleModule_CloseConnectionRequest(UInt8 linkId)
{
    /* <CodeGenerator Placeholder> Implementation_BleModule_CloseConnectionRequest */
    if(IS_VALID_LINK_ID(linkId))
    {
        gpBlePeripheralConnectionStm_Result_t result;
        result = gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventConnCloseApp, linkId);
        if(result != gpBlePeripheralConnectionStmResult_Success)
        {
            BleModuleItf_cbCloseConfirmResult(linkId, BleModule_RequestNotProcessed);
        }
    }
    else
    {
        GP_LOG_PRINTF("WARNING: BleModule_CloseConnectionRequest with invalid linkId received", 0);
        BleModuleItf_cbCloseConfirmResult(linkId, BleModule_LinkIdInvalid);
    }
    /* </CodeGenerator Placeholder> Implementation_BleModule_CloseConnectionRequest */
}
void BleModule_UnbindConnectionRequest(UInt8 linkId)
{
    /* <CodeGenerator Placeholder> Implementation_BleModule_UnbindConnectionRequest */
    gpBlePeripheralConnectionStm_Result_t result;
    appDbHdl_t hdl = AppDbGetHdlByLinkId(linkId);

    if(IS_VALID_LINK_ID(linkId))
    {
        result = gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventUnbindApp, linkId);
        if(result == gpBlePeripheralConnectionStmResult_Success)
        {
            if(hdl != APP_DB_HDL_NONE)
            {
                //BleModule_RestorePrivacyModeToDevice(hdl);
                AppDbDeleteRecord(hdl);
            }
            else
            {
                GP_LOG_PRINTF("Warning: BleModule_UnbindConnectionRequest has invalid AppDb Handle", 0);
            }
            BleModuleItf_cbUnbindConfirmResult(linkId, BleModule_Success);
        }
        else
        {
            GP_LOG_PRINTF("Warning: BleModule_UnbindConnectionRequest is not processed", 0);
            BleModuleItf_cbUnbindConfirmResult(linkId, BleModule_RequestNotProcessed);
        }
    }
    else
    {
        GP_LOG_PRINTF("Warning: BleModule_UnbindConnectionRequest with invalid linkId received", 0);
        BleModuleItf_cbUnbindConfirmResult(linkId, BleModule_LinkIdInvalid);
    }

    /* </CodeGenerator Placeholder> Implementation_BleModule_UnbindConnectionRequest */
}
void BleModule_SendDataRequest(UInt8 linkId, UInt16 length, UInt8* pData)
{
    /* <CodeGenerator Placeholder> Implementation_BleModule_SendDataRequest */
    //    BleModule_Result_t result = BleModule_LinkIdInvalid;

    if(IS_VALID_LINK_ID(linkId))
    {
        /* Replace by send general and check for valid Key report pointer */
        //result = gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventConnReq, linkId);
    }

    //    return result;
    /* </CodeGenerator Placeholder> Implementation_BleModule_SendDataRequest */
}

void BleModule_UpdateConnectionParametersRequest(UInt8 linkId, const BleModule_ConnectionParameters_t* pConPar)
{
    /* <CodeGenerator Placeholder> Implementation_BleModule_UpdateConnectionParametersRequest */
    BleModule_ConnectionParametersIndCnf_t ConnectParameters;

    //invalidate
    MEMSET(&ConnectParameters, 0, sizeof(ConnectParameters));

    if(IS_VALID_LINK_ID(linkId))
    {
        if(BleModule_ConnIdList[linkId] != DM_CONN_ID_NONE)
        {
            /* Rely on parameter check in LinkLayer, if invalid parameters status in callback != success */
            DmConnUpdate(BleModule_ConnIdList[linkId], (hciConnSpec_t*)pConPar);
        }
        else
        {
            BleModuleItf_cbUpdateConfirmResult(linkId, BleModule_LinkNotConnected, ConnectParameters);
        }
    }
    else
    {
        BleModuleItf_cbUpdateConfirmResult(linkId, BleModule_LinkIdInvalid, ConnectParameters);
    }
    /* </CodeGenerator Placeholder> Implementation_BleModule_UpdateConnectionParametersRequest */
}

void BleModule_SetAdvertisingTimeoutMs(UInt8 linkId, UInt16 advTimeoutMS)
{
    /* <CodeGenerator Placeholder> Implementation_BleModule_SetAdvertisingTimeoutMs */
    BleModule_Cfg[linkId].pAppAdvCfg->advDuration[0] = advTimeoutMS;
    /* </CodeGenerator Placeholder> Implementation_BleModule_SetAdvertisingTimeoutMs */
}

void BleModule_SetAdvertisingIntervalMs(UInt8 linkId, UInt16 advIntervalMs)
{
    /* <CodeGenerator Placeholder> BleModule_SetAdvertisingIntervalMs */
    BleModule_Cfg[linkId].pAppAdvCfg->advInterval[0] = (advIntervalMs * 8) / 5;
    /* </CodeGenerator Placeholder> BleModule_SetAdvertisingIntervalMs */
}

void BleModule_SetLocalName(UInt8 linkId, char* localName)
{
    /* <CodeGenerator Placeholder> BleModule_SetLocalName */
    uint8_t localNameLen = strlen(localName);

    if(localNameLen + 2 > HCI_ADV_DATA_LEN)
    {
        localNameLen = HCI_ADV_DATA_LEN - 2;
    }
    BleModule_LocalStr[0] = localNameLen + 1;
    BleModule_LocalStr[1] = DM_ADV_TYPE_LOCAL_NAME;
    memcpy(&BleModule_LocalStr[2], localName, localNameLen);
    BleModule_Cfg[linkId].appScanDataConn = BleModule_LocalStr;
    BleModule_Cfg[linkId].appScanDataConnLen = localNameLen + 2;
    BleModule_Cfg[linkId].appScanDataDisc = BleModule_LocalStr;
    BleModule_Cfg[linkId].appScanDataDiscLen = localNameLen + 2;
    /* </CodeGenerator Placeholder> BleModule_SetLocalName */
}


UInt8 BleModule_GetConnectionIdleWithSmpPairing(void)
{
    /* <CodeGenerator Placeholder> Implementation_BleModule_GetConnectionIdleWithSmpPairing */
    dmConnId_t connId = DM_CONN_ID_NONE;
    for(UIntLoop i = 1; i <= GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS; i++)
    {
        if (DmConnInUse(i) && (DmConnCheckIdle(i) & DM_IDLE_SMP_PAIR))
        {
            connId = i;
            GP_LOG_PRINTF("Detected Connection Idle with Smp Pairing (connId: %u)", 0, connId);
            break;
        }
    }
    return connId;
    /* </CodeGenerator Placeholder> Implementation_BleModule_GetConnectionIdleWithSmpPairing */
}


Bool BleModule_IsBonded(UInt8 linkId)
{
    /* <CodeGenerator Placeholder> Implementation_BleModule_IsBonded */
    appDbHdl_t hdl = AppDbGetHdlByLinkId(linkId);
    return AppDbRecordInUse(hdl);
    /* </CodeGenerator Placeholder> Implementation_BleModule_IsBonded */
}

/*****************************************************************************
 *                    Callback Function Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> CallbackFunctionDefinitions */

UInt8 cordioAppFramework_cbGetConnectingLink(void)
{
    return BleModule_LastReceivedNew;
}

void gpBlePeripheralConnectionStm_cbStartAdvertising(UInt8 linkId, gpBlePeripheralConnectionStm_AdvType_t AdvType)
{
    BleModuleItf_LoadCordioAppFrameworkPointers(linkId);
    BleModule_Cfg_t* linkCfg = &BleModule_Cfg[linkId];
    if(linkCfg->pLinkLayerConfig->useLinkLayerPrivacy)
    {
        UInt8 linkIdList[] = {linkId};
        BleModuleItf_SetDeviceFilteringPolicy(AdvType, linkIdList);
    }
    else
    {
        //just in case link layer privacy was disabled in runtime
        DmDevSetFilterPolicy(DM_FILT_POLICY_MODE_ADV, HCI_ADV_FILT_NONE);
        BleModule_PopulateWhiteList(BleModule_WhiteListClear, 1, NULL);
    }

    switch(AdvType)
    {
        case gpBlePeripheralConnectionStm_ReconnectDirectedHigh:
        {
            GP_LOG_SYSTEM_PRINTF("INFO: starting direct (high duty) advertising on link %u", 0, linkId);
            BleModuleItf_DirectAdvertise(linkId, DM_ADV_CONN_DIRECT);
            break;
        }

        case gpBlePeripheralConnectionStm_ReconnectDirectedLow:
        {
            GP_LOG_SYSTEM_PRINTF("INFO: starting direct (low duty) advertising on link %u", 0, linkId);
            BleModuleItf_DirectAdvertise(linkId, DM_ADV_CONN_DIRECT_LO_DUTY);
            break;
        }

        case gpBlePeripheralConnectionStm_Reconnect:
        {
            /* Load advertising data for connection linkId to AppFramework */
            AppSetBondable(false);
            {
#ifdef GP_BLE_MULTI_ADV_SUPPORTED
                /*Configure the address and address type*/
                uint8_t* pAddr = BleModule_Cfg[linkId].pAdvAddrCfg->address;

                if(BleModule_Cfg[linkId].pAdvAddrCfg->bUsePublicAddr)
                {
                    DmAdvSetAddrType(DM_ADDR_PUBLIC);
                }
                else
                {
                    DmDevSetRandAddr(pAddr);
                    DmAdvSetAddrType(DM_ADDR_RANDOM);
                }
#endif //GP_BLE_MULTI_ADV_SUPPORTED
                GP_LOG_SYSTEM_PRINTF("INFO: starting undirect advertising for reconnection on link %u", 0, linkId);
                AppAdvSetData(APP_ADV_DATA_CONNECTABLE, linkCfg->appAdvDataConnLen, linkCfg->appAdvDataConn);
                AppAdvSetData(APP_SCAN_DATA_CONNECTABLE, linkCfg->appScanDataConnLen, linkCfg->appScanDataConn);

                if(BleModule_Cfg[linkId].pLinkLayerConfig->useLinkLayerPrivacy)
                {
                    /* to enable undirected advertise using RPA generated by LLPrivacy.
                    If set peer is added to resolving list RPA will be used as AdvA */
                    appDbHdl_t AppDbHdl = AppDbGetHdlByLinkId(linkId);
                    UInt8 addrType = AppDbGetPeerAddressType(AppDbHdl);
                    UInt8* addr = AppDbGetPeerAddress(AppDbHdl);
                    AppSetAdvPeerAddr(addrType, addr);
                }
                AppAdvStart(APP_MODE_CONNECTABLE);
            }
        }
        break;

        case gpBlePeripheralConnectionStm_NewConnection:
        {
            /* Load advertising data for connection linkId to AppFramework */
            AppSetBondable(linkCfg->bondable);

            {
#ifdef GP_BLE_MULTI_ADV_SUPPORTED
                /*Configure the address and address type*/
                uint8_t* pAddr = BleModule_Cfg[linkId].pAdvAddrCfg->address;

                /* Public address is the default configure */
                if(BleModule_Cfg[linkId].pAdvAddrCfg->bUsePublicAddr)
                {
                    DmAdvSetAddrType(DM_ADDR_PUBLIC);
                }
                else
                {
                    DmDevSetRandAddr(pAddr);
                    DmAdvSetAddrType(DM_ADDR_RANDOM);
                }
#endif //GP_BLE_MULTI_ADV_SUPPORTED
                GP_LOG_SYSTEM_PRINTF("INFO: starting undirect advertising for new connection on link %u", 0, linkId);
                AppAdvSetData(APP_ADV_DATA_DISCOVERABLE, linkCfg->appAdvDataDiscLen, linkCfg->appAdvDataDisc);
                AppAdvSetData(APP_SCAN_DATA_DISCOVERABLE, linkCfg->appScanDataDiscLen, linkCfg->appScanDataDisc);
                if(BleModule_Cfg[linkId].pLinkLayerConfig->useLinkLayerPrivacy)
                {
                    //in case undirected reconnect was used before - restore zero values to avoid using RPA
                    UInt8 zeroAddr[BDA_ADDR_LEN];
                    MEMSET(zeroAddr, 0, BDA_ADDR_LEN);
                    AppSetAdvPeerAddr(0, zeroAddr);
                }
                AppAdvStart(APP_MODE_DISCOVERABLE);
            }
        }
        break;

        default:
            GP_LOG_PRINTF("ERROR: Invalid advertisment type", 0);
            break;
    }
}

void gpBlePeripheralConnectionStm_cbStopAdvertising(UInt8 linkId, gpBlePeripheralConnectionStm_AdvType_t AdvType, Bool disconnectingRequested)
{
    dmConnId_t connId = BleModule_ConnIdList[linkId];

    GP_LOG_PRINTF("INFO: stopping advertisement on link %d", 0, linkId);

    switch(AdvType)
    {
        case gpBlePeripheralConnectionStm_ReconnectDirectedHigh:
            /* Fall through */
        case gpBlePeripheralConnectionStm_ReconnectDirectedLow:
            if(connId != DM_CONN_ID_NONE)
            {
                AppConnClose(connId);
                // Need to remove this link id from the "active" connections
                BleModule_ConnIdList[linkId] = DM_CONN_ID_NONE;
            }
            break;

        default:
            /* Store the reason for stopping the advertisement, to be used on DM_ADV_STOP_IND.*/
            BleModulePeripheral_DisconnectionRequested[linkId]= disconnectingRequested;
            AppAdvStop();
            break;
    }
}

void gpBlePeripheralConnectionStm_cbCloseConnection(UInt8 linkId)
{
    appDbHdl_t hdl = AppDbGetHdlByLinkId(linkId);

    if(BleModule_ConnIdList[linkId] != DM_CONN_ID_NONE)
    {
        AppConnClose(BleModule_ConnIdList[linkId]);

        /* Sanity check, APP_DB_HDL_NONE on an open connection should not be possible */
        if(hdl != APP_DB_HDL_NONE)
        {
            AppDbCheckValidRecord(hdl);
        }
    }

}

void gpBlePeripheralConnectionStm_cbScheduleSecurityTimeout(UInt8 linkId)
{
    BleModule_SlaveSecurityList[linkId] = linkId;
    if(!gpSched_ExistsEventArg(BleModuleItf_SendSlaveSecurityRequest, &BleModule_SlaveSecurityList[linkId]))
    {
        gpSched_ScheduleEventArg(GP_BLE_SECURITY_TIMEOUT, BleModuleItf_SendSlaveSecurityRequest, &BleModule_SlaveSecurityList[linkId]);
    }
}

void cordioAppFramework_cbConnOpen(dmEvt_t* pMsg, appDbHdl_t handleOpen, UInt8 New)
{
    gpBlePeripheralConnectionStm_Result_t result;

    UInt8 linkIdOpen = BleModuleItf_GetLinkIdByConnId(pMsg->hdr.param);
    UInt8 linkIdAdv = BleModuleItf_GetAdvertisingLinkId();

    appDbHdl_t handleAdv = AppDbGetHdlByLinkId(linkIdAdv);

#ifdef GP_LOCAL_LOG
    UInt8 addrType = DmConnPeerAddrType((UInt8)pMsg->hdr.param);
    UInt8* pAddr = DmConnPeerAddr((UInt8)pMsg->hdr.param);

    GP_LOG_PRINTF("INFO: Connection Indication - connId: %u, linkIdOpen: %u, linkIdAdv: %u, handleOpen: 0x%lx, handleAdv: 0x%lx, new: %u", 0,
                  (UInt8)pMsg->hdr.param, linkIdOpen, linkIdAdv, (unsigned long)handleOpen, (unsigned long)handleAdv, New);
    GP_LOG_PRINTF("INFO: Connection Indication - addrType: 0x%x, addr %02x:%02x:%02x:%02x:%02x:%02x", 0,
                  addrType, pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5]);
#endif //GP_LOCAL_LOG

    /* If no linkId is advertising, close the connection and return immediately */
    if(!IS_VALID_LINK_ID(linkIdAdv))
    {
        GP_LOG_PRINTF("WARNING: dropping new device connection, no link was advertising", 0);
        AppConnClose(pMsg->hdr.param);
        BleModuleItf_cbOpenConfirmResult(BLE_MODULE_LINK_ID_NONE, BleModule_UnexpectedLinkOpenLinkDropped);
        return;
    }
    /* Add here to identify connect is from a known or a new device, AppFrameWork has aleady infrastructure for that
       investigate reusability of that.
       If connection from a new device use ConnOpenNewEvent */

    if(New) // Replace by connection from a new Device
    {
        result = gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventConnOpenNew, linkIdAdv);
        if(result == gpBlePeripheralConnectionStmResult_Success)
        {
            /* Store ConnId for later use */
            BleModule_LastReceivedNew = linkIdAdv;
            BleModule_ConnIdList[linkIdAdv] = pMsg->hdr.param;
            /* Connection opend and kept open, call DmFeatureRequest */
            DmReadRemoteFeatures(BleModule_ConnIdList[linkIdAdv]);
            BleModuleItf_cbOpenConfirmResult(linkIdAdv, BleModule_Success);
        }
        else
        {
            /* New connection not processed, this can occur if an unkown device connects while reconnecting. Drop connection */
            GP_LOG_PRINTF("WARNING: dropping new device connection, linkIdAdv is in the wrong state", 0);
            gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventConnFail, linkIdAdv);
            BleModuleItf_cbOpenConfirmResult(linkIdAdv, BleModule_UnexpectedLinkOpenLinkDropped);
            BleModule_LastReceivedNew = BLE_MODULE_LINK_ID_NONE;
        }
    }
    else // Replace by connect from a known device
    {
        /* Add, if known device and not from advertising connection, send conn open to linkId of received ConnOpen,
           Send ConnFail to advertising device. */

        /* If it was not paired using the advertising linkId, notify a failure to the advertising STM first */
        if(handleOpen != handleAdv)
        {
            gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventConnFail, linkIdAdv);
        }
        else if(linkIdOpen != linkIdAdv)
        {
            GP_LOG_PRINTF("WARNING: linkIdOpen != linkIdAdv (%u != %u)", 0, linkIdOpen, linkIdAdv);
        }

        result = gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_EventConnOpen, linkIdOpen);
        if(result == gpBlePeripheralConnectionStmResult_Success)
        {
            /* Store ConnId for later use */
            BleModule_ConnIdList[linkIdOpen] = pMsg->hdr.param;
            /* Connection opend and kept open, call DmFeatureRequest */
            DmReadRemoteFeatures(BleModule_ConnIdList[linkIdOpen]);
            BleModuleItf_cbOpenConfirmResult(linkIdOpen, BleModule_Success);
        }
        else
        {
            /* New connection not processed, Should not occur. Drop connection to avoid to have an unmanaged connection open. */
            GP_LOG_PRINTF("WARNING: dropping known device connection, linkIdOpen is in the wrong state", 0);
            AppConnClose(pMsg->hdr.param);
            BleModuleItf_cbOpenConfirmResult(BLE_MODULE_LINK_ID_NONE, BleModule_UnexpectedLinkOpenLinkDropped);
        }

        /* in case we are coming from a directed advertisement */
        BleModule_LastReceivedNew = BLE_MODULE_LINK_ID_NONE;
    }
}

/* </CodeGenerator Placeholder> CallbackFunctionDefinitions */
