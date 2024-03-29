/*
 * Copyright (c) 2015-2016, GreenPeak Technologies
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
 *
 */

//#define GP_LOCAL_LOG

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLEDATACOMMON

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBle.h"
#include "gpBleComps.h"
#include "gpBleDataCommon.h"
#include "gpBle_defs.h"
#include "gpSched.h"
#include "gpPoolMem.h"
#include "gpLog.h"

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
#include "gpBleLlcp.h"
#include "gpBleLlcpFramework.h"
#include "gpBleLlcpProcedures.h"
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define BLE_DATACOMMON_CODED_PHY_CONST_OVERHEAD_DURATION_US     (80 + 256 + 16 + 24) // preamble + AccessAddress + CI + TERM1

#define GPBLEDATACOMMON_FEC_SIZE_CODED          1

#define GPBLEDATACOMMON_CTE_INFO_SIZE BLE_CTE_INFO_SIZE
// 4 bytes AA, 2 bytes pdu header, 3 bytes CRC and 4 bytes MIC
#define GPBLEDATACOMMON_PDU_OVERHEAD_BYTES_COMMON   (BLE_ACCESS_ADDRESS_SIZE + BLE_PACKET_HEADER_SIZE + BLE_CRC_SIZE + BLE_SEC_MIC_LENGTH)

// Overhead bytes for all phy modes
#define GPBLEDATACOMMON_PDU_OVERHEAD_BYTES_1MBIT    (GPBLE_PREAMBLE_SIZE_1MBIT + GPBLEDATACOMMON_PDU_OVERHEAD_BYTES_COMMON)
#define GPBLEDATACOMMON_PDU_OVERHEAD_BYTES_2MBIT    (GPBLE_PREAMBLE_SIZE_2MBIT + GPBLEDATACOMMON_PDU_OVERHEAD_BYTES_COMMON)
#define GPBLEDATACOMMON_PDU_OVERHEAD_BYTES_CODED    (GPBLE_PREAMBLE_SIZE_CODED + GPBLEDATACOMMON_PDU_OVERHEAD_BYTES_COMMON + GPBLEDATACOMMON_FEC_SIZE_CODED)

#define BLEDATACOMMON_LENGTH_FIELD_MAX              (GPBLEDATACOMMON_OCTETS_SUPPORTED_MAX + BLE_SEC_MIC_LENGTH)

#define BLEDATACOMMON_SLOWEST_SUPPORTED_PHY_WITH_CODING         (gpHci_PhyWithCoding_1Mb)

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

#define BLE_EFFECTIVE_MAX_TX_TIME_UNCODED(connId)   min(Ble_DataLinkContext[connId].maxTxTimeLocal, Ble_DataLinkContext[connId].maxRxTimeRemote)
#define BLE_EFFECTIVE_MAX_RX_TIME_UNCODED(connId)   min(Ble_DataLinkContext[connId].maxRxTimeLocal, Ble_DataLinkContext[connId].maxTxTimeRemote)
#define BLE_INITIAL_MIN_TX_TIME_CODED               GPBLEDATACOMMON_TIME_CODED_SPEC_MIN

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct {
    UInt16 hostSuggestedMaxTxOctets;
    UInt16 hostSuggestedMaxTxTime;
    UInt16 hostSuggestedMaxTxOctetsUnmodified;
    UInt16 hostSuggestedMaxTxTimeUnmodified;
    gpHci_PhyMask_t preferredPhyModesTx;
    gpHci_PhyMask_t preferredPhyModesRx;
} Ble_DataGlobalContext_t;

typedef struct {
    Ble_IntConnId_t connId;
    UInt16 maxTxOctetsLocal;
    UInt16 maxRxOctetsLocal;
    UInt16 maxTxTimeLocal;
    UInt16 maxRxTimeLocal;
    UInt16 maxTxOctetsRemote;
    UInt16 maxRxOctetsRemote;
    UInt16 maxTxTimeRemote;
    UInt16 maxRxTimeRemote;
    // phyModeTx is stored as gpHci_PhyWithCoding_t type, to avoid the coding scheme byte to be stored separately.
    gpHci_PhyWithCoding_t phyModeTx;
    gpHci_Phy_t phyModeRx;
    gpHci_PhyMask_t preferredPhyModesTx;
    gpHci_PhyMask_t preferredPhyModesRx;
} Ble_DataLinkContext_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

// The data context for all connections
static Ble_DataGlobalContext_t Ble_DataGlobalContext;

// The data context per connection
static Ble_DataLinkContext_t Ble_DataLinkContext[BLE_LLCP_MAX_NR_OF_CONNECTIONS];

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static INLINE Bool Ble_DataIsConnectionIdValid(Ble_IntConnId_t connId);
static gpHal_BleRxPhy_t BleDataCommon_HciPhyToHalRxPhy(gpHci_Phy_t hciPhy);
static gpHci_Phy_t BleDataCommon_HalRxPhyToHciPhy(gpHal_BleRxPhy_t halPhy);
static UInt8 Ble_GetByteDurationUs(gpHci_PhyWithCoding_t phy);

#ifdef GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED
static Bool Ble_CheckIfLengthUpdateNeeded(Ble_IntConnId_t connId);
#endif //GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED
#if defined(GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED) &&  defined(GP_DIVERSITY_BLE_PERIPHERAL)
static Bool Ble_DataCommonPhyChangeRequested(gpHci_Phy_t currentPhy, gpHci_PhyMask_t possiblePhysMask);
static Bool Ble_CheckIfPhyUpdateNeeded(Ble_IntConnId_t connId);
static void Ble_DataTriggerStartupPhyUpdate(void* pArg);
static gpHci_PhyMask_t Ble_GetDefaultPhyModesTx(void);
static gpHci_PhyMask_t Ble_GetDefaultPhyModesRx(void);
#endif // GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED && (GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL)

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
static gpHci_Result_t Ble_DataTriggerLengthUpdate(Ble_IntConnId_t connId, Bool controllerInit, UInt16 txOctets, UInt16 txTime);
static void Ble_DataTriggerStartupLengthUpdate(void* pArg);
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL

// Checker/action functions
#ifdef GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED
static gpHci_Result_t Ble_OctetAndTimeRangeChecker(UInt16 octets, UInt16 time);
#endif //GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED

#ifdef GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED
static gpHci_Result_t Ble_SetDataLengthChecker(gpHci_LeSetDataLengthCommand_t* pDataLength);
static gpHci_Result_t Ble_SetDataLengthAction(gpHci_LeSetDataLengthCommand_t* pDataLength);
static gpHci_Result_t Ble_WriteSuggestedDefDataLengthChecker(gpHci_LeWriteSuggestedDefDataLengthCommand_t* pDataLength);
static void Ble_WriteSuggestedDefDataLengthAction(gpHci_LeWriteSuggestedDefDataLengthCommand_t* pSuggestedLength);
#endif //GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED


/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

Bool Ble_DataIsConnectionIdValid(Ble_IntConnId_t connId)
{
    return (Ble_DataLinkContext[connId].connId == connId);
}

#ifdef GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED
Bool Ble_CheckIfLengthUpdateNeeded(Ble_IntConnId_t connId)
{
    {
        return ( (gpBle_GetMaxTxOctetsLocal(connId) > GPBLEDATACOMMON_OCTETS_SPEC_MIN) ||
            (gpBle_GetMaxRxOctetsLocal(connId) > GPBLEDATACOMMON_OCTETS_SPEC_MIN) ||
            (gpBle_GetMaxTxTimeLocal(connId)> GPBLEDATACOMMON_TIME_SPEC_MIN) ||
            (gpBle_GetMaxRxTimeLocal(connId)> GPBLEDATACOMMON_TIME_SPEC_MIN)
        );
    }
}
#endif //GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
#ifdef GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED
Bool Ble_DataCommonPhyChangeRequested(gpHci_Phy_t currentPhy, gpHci_PhyMask_t possiblePhysMask)
{
    GP_ASSERT_DEV_INT(GP_HCI_PHY_TYPE_VALID(currentPhy));

    GP_LOG_PRINTF("current phy %x en possbile ones %x",0, currentPhy, possiblePhysMask.mask);

    if(HCI_PHY_IN_PHYMASK(HCI_PHY_TO_PHYMASK(currentPhy), possiblePhysMask))
    {
        return false;
    }

    return true;
}
#endif // GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED
#endif // GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
#ifdef GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED
Bool Ble_CheckIfPhyUpdateNeeded(Ble_IntConnId_t connId)
{
    gpHci_Phy_t txPhy;
    gpHci_Phy_t rxPhy;

    txPhy = gpBle_GetEffectivePhyTxType(connId);
    rxPhy = gpBle_GetEffectivePhyRxType(connId);

    GP_LOG_PRINTF("Ble_CheckIfPhyUpdateNeeded: tx: %x (%x) || rx: %x (%x)",0,
        txPhy, gpBle_GetPreferredPhyModesTx(connId).mask,
        rxPhy, gpBle_GetPreferredPhyModesRx(connId).mask
    );

    if(Ble_DataCommonPhyChangeRequested(txPhy, gpBle_GetPreferredPhyModesTx(connId)))
    {
        return true;
    }

    if(Ble_DataCommonPhyChangeRequested(rxPhy, gpBle_GetPreferredPhyModesRx(connId)))
    {
        return true;
    }

    return false;
}
#endif // GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED
#endif // GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
gpHci_Result_t Ble_DataTriggerLengthUpdate(Ble_IntConnId_t connId, Bool controllerInit, UInt16 txOctets, UInt16 txTime)
{
    gpBleLlcpFramework_StartProcedureDescriptor_t startDescriptor;

    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    MEMSET(&startDescriptor, 0, sizeof(gpBleLlcpFramework_StartProcedureDescriptor_t));
    startDescriptor.procedureId = gpBleLlcp_ProcedureIdDataLengthUpdate;
    startDescriptor.controllerInit = controllerInit;
    startDescriptor.procedureData.dataLengthUpdate.localTxOctets = txOctets;
    startDescriptor.procedureData.dataLengthUpdate.localTxTime = txTime;

    return gpBleLlcpFramework_StartProcedure(connId, &startDescriptor);
}

void Ble_DataTriggerStartupLengthUpdate(void* pArg)
{
    Ble_DataLinkContext_t* pContext = (Ble_DataLinkContext_t*)pArg;

    Ble_DataTriggerLengthUpdate(pContext->connId, true, pContext->maxTxOctetsLocal,pContext->maxTxTimeLocal);
}
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
#ifdef GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED
void Ble_DataTriggerStartupPhyUpdate(void* pArg)
{
    Ble_DataLinkContext_t* pContext = (Ble_DataLinkContext_t*)pArg;
    gpHci_PhyMask_t txPhys = Ble_GetDefaultPhyModesTx();
    gpHci_PhyMask_t rxPhys = Ble_GetDefaultPhyModesRx();

    gpBleLlcpProcedures_ControllerTriggeredPhyUpdate(pContext->connId, txPhys, rxPhys);
}
#endif // GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED
#endif // GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL

#ifdef GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED
gpHci_Result_t Ble_OctetAndTimeRangeChecker(UInt16 octets, UInt16 time)
{
    if(!RANGE_CHECK(octets, GPBLEDATACOMMON_OCTETS_SPEC_MIN, GPBLEDATACOMMON_OCTETS_SPEC_MAX))
    {
        GP_LOG_PRINTF("octets not in range: %x <= %x <= %x",0, GPBLEDATACOMMON_OCTETS_SPEC_MIN, octets, GPBLEDATACOMMON_OCTETS_SPEC_MAX);
        return gpHci_ResultInvalidHCICommandParameters;
    }

    if(!RANGE_CHECK(time, GPBLEDATACOMMON_TIME_SPEC_MIN, GPBLEDATACOMMON_TIME_SPEC_MAX))
    {
        GP_LOG_PRINTF("time not in range: %x <= %x <= %x",0, GPBLEDATACOMMON_TIME_SPEC_MIN, time, GPBLEDATACOMMON_TIME_SPEC_MAX);
        return gpHci_ResultInvalidHCICommandParameters;
    }

    return gpHci_ResultSuccess;
}
#endif //GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED

#ifdef GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED
gpHci_Result_t Ble_SetDataLengthChecker(gpHci_LeSetDataLengthCommand_t* pDataLength)
{
    if(gpBleLlcp_IsHostConnectionHandleValid(pDataLength->connectionHandle) != gpHci_ResultSuccess)
    {
        GP_LOG_PRINTF("Invalid connection id: %x",0, pDataLength->connectionHandle);
        return gpHci_ResultUnknownConnectionIdentifier;
    }

    return Ble_OctetAndTimeRangeChecker(pDataLength->txOctets, pDataLength->txTime);
}

gpHci_Result_t Ble_SetDataLengthAction(gpHci_LeSetDataLengthCommand_t* pDataLength)
{
    Ble_IntConnId_t connId;

    connId = gpBleLlcp_HciHandleToIntHandle(pDataLength->connectionHandle);

    GP_ASSERT_DEV_INT(connId != BLE_CONN_HANDLE_INVALID);

    // The host is allowed to ask more than we support, make sure we correct these values when this is the case
    if(pDataLength->txOctets > GPBLEDATACOMMON_OCTETS_SUPPORTED_MAX)
    {
        GP_LOG_PRINTF("req octets (%u) > supported (%u) => adjust", 0, pDataLength->txOctets, GPBLEDATACOMMON_OCTETS_SUPPORTED_MAX);
        pDataLength->txOctets = GPBLEDATACOMMON_OCTETS_SUPPORTED_MAX;
    }

    if(pDataLength->txTime > GPBLEDATACOMMON_TIME_SUPPORTED_MAX)
    {
        GP_LOG_PRINTF("req time (%u) > supported (%u) => adjust", 0, pDataLength->txTime, GPBLEDATACOMMON_TIME_SUPPORTED_MAX);
        pDataLength->txTime = GPBLEDATACOMMON_TIME_SUPPORTED_MAX;
    }

    return Ble_DataTriggerLengthUpdate(connId, false, pDataLength->txOctets, pDataLength->txTime);
}

gpHci_Result_t Ble_WriteSuggestedDefDataLengthChecker(gpHci_LeWriteSuggestedDefDataLengthCommand_t* pDataLength)
{
    return Ble_OctetAndTimeRangeChecker(pDataLength->suggestedMaxTxOctets, pDataLength->suggestedMaxTxTime);
}

void Ble_WriteSuggestedDefDataLengthAction(gpHci_LeWriteSuggestedDefDataLengthCommand_t* pSuggestedLength)
{
    Ble_DataGlobalContext.hostSuggestedMaxTxOctetsUnmodified = pSuggestedLength->suggestedMaxTxOctets;
    Ble_DataGlobalContext.hostSuggestedMaxTxTimeUnmodified = pSuggestedLength->suggestedMaxTxTime;

    Ble_DataGlobalContext.hostSuggestedMaxTxOctets= pSuggestedLength->suggestedMaxTxOctets;
    if(pSuggestedLength->suggestedMaxTxOctets > GPBLEDATACOMMON_OCTETS_SUPPORTED_MAX)
    {
        GP_LOG_PRINTF("req octets (%u) > supported (%u) => adjust", 0, pSuggestedLength->suggestedMaxTxOctets, GPBLEDATACOMMON_OCTETS_SUPPORTED_MAX);
        Ble_DataGlobalContext.hostSuggestedMaxTxOctets = GPBLEDATACOMMON_OCTETS_SUPPORTED_MAX;
    }

    Ble_DataGlobalContext.hostSuggestedMaxTxTime = pSuggestedLength->suggestedMaxTxTime;
    if(pSuggestedLength->suggestedMaxTxTime > GPBLEDATACOMMON_TIME_SUPPORTED_MAX)
    {
        GP_LOG_PRINTF("req time (%u) > supported (%u) => adjust", 0, pSuggestedLength->suggestedMaxTxTime, GPBLEDATACOMMON_TIME_SUPPORTED_MAX);
        Ble_DataGlobalContext.hostSuggestedMaxTxTime = GPBLEDATACOMMON_TIME_SUPPORTED_MAX;
    }
}

#endif //GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED

gpHci_PhyWithCoding_t gpBleDataCommon_PhyToPhyWithCoding(gpHci_Phy_t hciPhy, gpHci_PhyOptions_t phyOptions)
{
    gpHci_PhyWithCoding_t phyWithCoding = gpHci_PhyWithCoding_Invalid;

    if(!GP_HCI_PHY_TYPE_VALID(hciPhy))
    {
        return phyWithCoding;
    }

    switch(hciPhy)
    {
        case gpHci_Phy_1Mb:
        {
            phyWithCoding = gpHci_PhyWithCoding_1Mb;
            break;
        }
#ifdef GP_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED
        case gpHci_Phy_2Mb:
        {
            phyWithCoding = gpHci_PhyWithCoding_2Mb;
            break;
        }
#endif // GP_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED
        default:
        {
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }

    return phyWithCoding;
}

gpHal_BleTxPhy_t BleDataCommon_HciPhyToHalTxPhy(gpHci_Phy_t hciPhy, gpHci_PhyOptions_t phyOptions)
{
    gpHal_BleTxPhy_t halPhy = gpHal_BleTxPhyInvalid;

    if(!GP_HCI_PHY_TYPE_VALID(hciPhy))
    {
        return halPhy;
    }

    switch(hciPhy)
    {
        case gpHci_Phy_1Mb:
        {
            halPhy = gpHal_BleTxPhy1Mb;
            break;
        }
#ifdef GP_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED
        case gpHci_Phy_2Mb:
        {
            halPhy = gpHal_BleTxPhy2Mb;
            break;
        }
#endif //.GP_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED
        default:
        {
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }

    return halPhy;
}

gpHal_BleRxPhy_t BleDataCommon_HciPhyToHalRxPhy(gpHci_Phy_t hciPhy)
{
    gpHal_BleRxPhy_t halPhy = gpHal_BleRxPhyInvalid;

    if(!GP_HCI_PHY_TYPE_VALID(hciPhy))
    {
        return halPhy;
    }

    switch(hciPhy)
    {
        case gpHci_Phy_1Mb:
        {
            halPhy = gpHal_BleTxPhy1Mb;
            break;
        }
#ifdef GP_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED
        case gpHci_Phy_2Mb:
        {
            halPhy = gpHal_BleTxPhy2Mb;
            break;
        }
#endif //.GP_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED
        default:
        {
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }

    return halPhy;
}


UInt8 Ble_GetByteDurationUs(gpHci_PhyWithCoding_t phy)
{
    UInt8 byteDurationsUs[] =
    {
        0,  /* gpHci_PhyWithCoding_Reserved */
        8,  /* gpHci_PhyWithCoding_1Mb */
        4,  /* gpHci_PhyWithCoding_2Mb */
        64, /* gpHci_PhyWithCoding_Coded125kb */
        16, /* gpHci_PhyWithCoding_Coded500kb */
    };

    GP_ASSERT_DEV_INT(GP_HCI_ENHANCED_PHY_TYPE_VALID(phy));

    return byteDurationsUs[phy];
}


/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBle_DataCommonInit(void)
{
    gpHal_EnableDataIndInterrupts(true);
    gpHal_EnableDataConfInterrupts(true);

    gpHal_BleSetMaxRxPayloadLength(BLEDATACOMMON_LENGTH_FIELD_MAX);
}

void gpBle_DataCommonReset(Bool firstReset)
{
    UIntLoop i;

    GP_LOG_PRINTF("Data Common Reset",0);

    MEMSET(&Ble_DataGlobalContext, 0, sizeof(Ble_DataGlobalContext_t));

    // It is acceptable to use our max supported Tx PDU sizes here
    // Even if the host did not set these yet (Host typcially reads these during initialization)
    Ble_DataGlobalContext.hostSuggestedMaxTxOctets = GPBLEDATACOMMON_OCTETS_SUPPORTED_MAX;
    Ble_DataGlobalContext.hostSuggestedMaxTxTime = GPBLEDATACOMMON_TIME_SUPPORTED_MAX;

    // Initialize host suggested values to their defaults.
    Ble_DataGlobalContext.hostSuggestedMaxTxOctetsUnmodified = GPBLEDATACOMMON_OCTETS_SPEC_DEFAULT;
    Ble_DataGlobalContext.hostSuggestedMaxTxTimeUnmodified = GPBLEDATACOMMON_TIME_SPEC_DEFAULT;

    Ble_DataGlobalContext.preferredPhyModesTx = GPBLEDATACOMMON_GET_SUPPORTED_PHYS_MASK();
    Ble_DataGlobalContext.preferredPhyModesRx = GPBLEDATACOMMON_GET_SUPPORTED_PHYS_MASK();

    // Reset link context
    for(i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        Ble_DataLinkContext[i].connId = BLE_CONN_HANDLE_INVALID;
    }
}

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
void gpBle_DataOpenConnection(Ble_IntConnId_t connId, gpHci_PhyWithCoding_t phy)
{
    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(connId));
    GP_ASSERT_DEV_INT(Ble_DataLinkContext[connId].connId == BLE_CONN_HANDLE_INVALID);

    GP_LOG_PRINTF("Data open conn: %x", 0, connId);

    Ble_DataLinkContext[connId].connId = connId;

    // Set defaults on connection start (Spec vol6, part B, section 4.5.10)
    Ble_DataLinkContext[connId].maxTxOctetsLocal = min(Ble_DataGlobalContext.hostSuggestedMaxTxOctets, (UInt16) GPBLEDATACOMMON_OCTETS_SUPPORTED_MAX);
    Ble_DataLinkContext[connId].maxTxOctetsLocal = min(Ble_DataLinkContext[connId].maxTxOctetsLocal, (UInt16) GP_BLE_DIVERSITY_HCI_BUFFER_SIZE_TX);
    /* maxRxOctetsLocal = min (buffer size of the Host, our supported buffer/PBM (smallest BLE) size) , Size of our HCI Rx buffers) */
    Ble_DataLinkContext[connId].maxRxOctetsLocal = min((UInt16)GPBLEDATACOMMON_OCTETS_SUPPORTED_MAX, (UInt16)GP_BLE_DIVERSITY_HCI_BUFFER_SIZE_RX);
    Ble_DataLinkContext[connId].maxTxTimeLocal = min(Ble_DataGlobalContext.hostSuggestedMaxTxTime, GPBLEDATACOMMON_TIME_SUPPORTED_MAX);
    Ble_DataLinkContext[connId].maxRxTimeLocal = gpBleDataCommon_GetPacketDurationUs(Ble_DataLinkContext[connId].maxRxOctetsLocal, BLEDATACOMMON_SLOWEST_SUPPORTED_PHY_WITH_CODING, true);

    // Remote times and octets shall be set to the spec minimum (will be updated if remote supports the length update procedure)
    Ble_DataLinkContext[connId].maxTxOctetsRemote = GPBLEDATACOMMON_OCTETS_SPEC_MIN;
    Ble_DataLinkContext[connId].maxRxOctetsRemote = GPBLEDATACOMMON_OCTETS_SPEC_MIN;
    Ble_DataLinkContext[connId].maxTxTimeRemote = GPBLEDATACOMMON_TIME_SPEC_MIN;
    Ble_DataLinkContext[connId].maxRxTimeRemote = GPBLEDATACOMMON_TIME_SPEC_MIN;

    // Always start in the specified mode -
    Ble_DataLinkContext[connId].phyModeTx = phy;
    Ble_DataLinkContext[connId].phyModeRx = gpBleDataCommon_PhyWithCodingToPhy(phy);
    Ble_DataLinkContext[connId].preferredPhyModesTx = Ble_DataGlobalContext.preferredPhyModesTx;
    Ble_DataLinkContext[connId].preferredPhyModesRx = Ble_DataGlobalContext.preferredPhyModesRx;


    // Do not trigger procedures at startup on lower tester
#ifdef GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED
    // Spec dictates we need to initiate a data length update in case we deviate from default minimum values
    if(Ble_CheckIfLengthUpdateNeeded(connId))
    {
        // Schedule this procedure ==> allow features to be exchanged first
        gpSched_ScheduleEventArg(0, Ble_DataTriggerStartupLengthUpdate, &Ble_DataLinkContext[connId]);
    }
#endif //GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED

#ifdef GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED
    if(Ble_CheckIfPhyUpdateNeeded(connId))
    {
        // Schedule this procedure ==> allow features to be exchanged first
        gpSched_ScheduleEventArg(0, Ble_DataTriggerStartupPhyUpdate, &Ble_DataLinkContext[connId]);
    }
#endif // GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED
}

void gpBle_DataCloseConnection(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    gpSched_UnscheduleEventArg(Ble_DataTriggerStartupLengthUpdate, &Ble_DataLinkContext[connId]);
    Ble_DataLinkContext[connId].connId = BLE_CONN_HANDLE_INVALID;
}

Bool gpBle_IsConnectionOpen(Ble_IntConnId_t connId)
{
    return (Ble_DataLinkContext[connId].connId == BLE_CONN_HANDLE_INVALID ? false : true);
}
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL

UInt16 gpBle_GetMaxTxOctetsLocal(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    return Ble_DataLinkContext[connId].maxTxOctetsLocal;
}

UInt16 gpBle_GetMaxRxOctetsLocal(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    return Ble_DataLinkContext[connId].maxRxOctetsLocal;
}

UInt16 gpBle_GetMaxTxTimeLocal(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    return Ble_DataLinkContext[connId].maxTxTimeLocal;
}

UInt16 gpBle_GetMaxRxTimeLocal(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    return Ble_DataLinkContext[connId].maxRxTimeLocal;
}

void gpBle_SetMaxTxOctetsLocal(Ble_IntConnId_t connId, UInt16 maxTxOctetsLocal)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    Ble_DataLinkContext[connId].maxTxOctetsLocal = maxTxOctetsLocal;
}

void gpBle_SetMaxTxTimeLocal(Ble_IntConnId_t connId, UInt16 maxTxTimeLocal)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    Ble_DataLinkContext[connId].maxTxTimeLocal = maxTxTimeLocal;
}

void gpBle_SetMaxTxOctetsRemote(Ble_IntConnId_t connId, UInt16 maxTxOctetsRemote)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    Ble_DataLinkContext[connId].maxTxOctetsRemote = maxTxOctetsRemote;
}

void gpBle_SetMaxRxOctetsRemote(Ble_IntConnId_t connId, UInt16 maxRxOctetsRemote)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    Ble_DataLinkContext[connId].maxRxOctetsRemote = maxRxOctetsRemote;
}

void gpBle_SetMaxTxTimeRemote(Ble_IntConnId_t connId, UInt16 maxTxTimeRemote)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    Ble_DataLinkContext[connId].maxTxTimeRemote = maxTxTimeRemote;
}

void gpBle_SetMaxRxTimeRemote(Ble_IntConnId_t connId, UInt16 maxRxTimeRemote)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    Ble_DataLinkContext[connId].maxRxTimeRemote = maxRxTimeRemote;
}

UInt16 gpBle_GetEffectiveMaxTxOctets(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    return min(Ble_DataLinkContext[connId].maxTxOctetsLocal, Ble_DataLinkContext[connId].maxRxOctetsRemote);
}

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
UInt16 gpBle_GetEffectiveMaxTxTime(Ble_IntConnId_t connId)
{
    {
        // uncoded
        return BLE_EFFECTIVE_MAX_TX_TIME_UNCODED(connId);
    }
}

UInt16 gpBle_GetEffectiveMaxRxOctets(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    return min(Ble_DataLinkContext[connId].maxRxOctetsLocal, Ble_DataLinkContext[connId].maxTxOctetsRemote);
}

UInt16 gpBle_GetEffectiveMaxRxTime(Ble_IntConnId_t connId)
{
    {
       // uncoded
        return BLE_EFFECTIVE_MAX_RX_TIME_UNCODED(connId);
    }
}

gpHci_Phy_t gpBle_GetEffectivePhyTxType(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    // phyModeTx is stored as gpHci_PhyWithCoding_t ==> apply conversion
    return gpBleDataCommon_PhyWithCodingToPhy(Ble_DataLinkContext[connId].phyModeTx);
}

gpHci_PhyWithCoding_t gpBle_GetEffectivePhyTxTypeWithCoding(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    return Ble_DataLinkContext[connId].phyModeTx;
}

gpHci_Phy_t gpBle_GetEffectivePhyRxType(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    return Ble_DataLinkContext[connId].phyModeRx;
}

gpHci_PhyMask_t gpBle_GetPreferredPhyModesTx(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    return Ble_DataLinkContext[connId].preferredPhyModesTx;
}

gpHci_PhyMask_t gpBle_GetPreferredPhyModesRx(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    return Ble_DataLinkContext[connId].preferredPhyModesRx;
}

void gpBle_SetPreferredPhyModesTx(Ble_IntConnId_t connId, gpHci_PhyMask_t phyModesTx)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    Ble_DataLinkContext[connId].preferredPhyModesTx = phyModesTx;
}

void gpBle_SetPreferredPhyModesRx(Ble_IntConnId_t connId, gpHci_PhyMask_t phyModesRx)
{
    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    Ble_DataLinkContext[connId].preferredPhyModesRx = phyModesRx;
}

void gpBle_SetEffectivePhys(Ble_IntConnId_t connId, gpHci_Phy_t txPhy, gpHci_Phy_t rxPhy, gpHci_PhyOptions_t phyOptions)
{
    gpHal_Result_t result;
    gpHal_BlePhyUpdateInfo_t updateInfo;

    MEMSET(&updateInfo, 0, sizeof(gpHal_BlePhyUpdateInfo_t));

    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    if(txPhy != gpHci_Phy_None)
    {
        Ble_DataLinkContext[connId].phyModeTx = gpBleDataCommon_PhyToPhyWithCoding(txPhy, phyOptions);
    }

    if(rxPhy != gpHci_Phy_None)
    {
        Ble_DataLinkContext[connId].phyModeRx = rxPhy;
    }

    // Convert HCI phy to gpHal tx phy
    updateInfo.phyIdTx = BleDataCommon_HciPhyToHalTxPhy(txPhy, phyOptions);
    // Convert HCI phy to gpHal rx phy
    updateInfo.phyIdRx = BleDataCommon_HciPhyToHalRxPhy(rxPhy);

    result = gpHal_BleUpdatePhy(connId, &updateInfo);

    GP_ASSERT_DEV_INT(result == gpHci_ResultSuccess);
}
#endif// GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
#ifdef GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED
gpHci_PhyMask_t Ble_GetDefaultPhyModesTx(void)
{
    return Ble_DataGlobalContext.preferredPhyModesTx;
}

gpHci_PhyMask_t Ble_GetDefaultPhyModesRx(void)
{
    return Ble_DataGlobalContext.preferredPhyModesRx;
}
#endif // GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED
#endif // GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL

void gpBle_SetDefaultPhyModesTx(gpHci_PhyMask_t phyModesTx)
{
    Ble_DataGlobalContext.preferredPhyModesTx = phyModesTx;
}

void gpBle_SetDefaultPhyModesRx(gpHci_PhyMask_t phyModesRx)
{
    Ble_DataGlobalContext.preferredPhyModesRx = phyModesRx;
}

Bool gpBle_DataOctetsAndTimesValidMinimum(UInt16 rxOctets, UInt16 rxTime, UInt16 txOctets, UInt16 txTime)
{
    if(rxOctets < GPBLEDATACOMMON_OCTETS_SPEC_MIN || txOctets < GPBLEDATACOMMON_OCTETS_SPEC_MIN)
    {
        return false;
    }

    if(rxTime < GPBLEDATACOMMON_TIME_SPEC_MIN || txTime < GPBLEDATACOMMON_TIME_SPEC_MIN)
    {
        return false;
    }

    return true;
}

UInt16 gpBle_GetEffectiveMaxRxPacketDuration(Ble_IntConnId_t connId)
{
    gpHci_PhyWithCoding_t rxPhy;
    gpHci_Phy_t effectivePhyRxType = gpBle_GetEffectivePhyRxType(connId);

    if ((gpHci_PhyWithCoding_1Mb != effectivePhyRxType) && (gpHci_PhyWithCoding_2Mb != effectivePhyRxType))
    {
        rxPhy = gpHci_PhyWithCoding_Coded125kb;
    }
    else
    {
        rxPhy = effectivePhyRxType;
    }

    // effectiveTime is time for payload AND overhead (full packet)
    UInt16 effectiveTime = gpBle_GetEffectiveMaxRxTime(connId);
    // effectiveOctets are only payload octets
    UInt16 effectiveRxOctets = gpBle_GetEffectiveMaxRxOctets(connId);
    UInt16 octetsInTime = gpBleDataCommon_GetPayloadDurationUs(effectiveRxOctets,rxPhy) + gpBleDataCommon_GetOverheadDurationUs(rxPhy);

    // The remote device will only send packets that are
    // - not exceeding the effectiveTime
    // - and not exceeding effectiveRxOctets (payload size)
    return min(effectiveTime, octetsInTime);
}

/* Given a PHY and payload length, return the total time in us to transmit payload bytes */
UInt16 gpBleDataCommon_GetPayloadDurationUs(UInt16 payloadLengthBytes, gpHci_PhyWithCoding_t phy)
{
    UInt8 multiplier;

    multiplier = Ble_GetByteDurationUs(phy);

    return (payloadLengthBytes * multiplier);
}

/* Given a PHY, return the total time in us to transmit all non-payload bytes (including a MIC) */
UInt16 gpBleDataCommon_GetOverheadDurationUs(gpHci_PhyWithCoding_t phy)
{
    UInt16 overheadDurationsUs[] =
    {
        0, /* gpHci_PhyWithCoding_Reserved */
        GPBLEDATACOMMON_PDU_OVERHEAD_BYTES_1MBIT * 8,    /* gpHci_PhyWithCoding_1Mb */
        GPBLEDATACOMMON_PDU_OVERHEAD_BYTES_2MBIT * 4,    /* gpHci_PhyWithCoding_2Mb */
        BLE_DATACOMMON_CODED_PHY_CONST_OVERHEAD_DURATION_US + ((BLE_PACKET_HEADER_SIZE + BLE_SEC_MIC_LENGTH) * 64) + ((24 + 3) * 8),  /* gpHci_PhyWithCoding_Coded125kb */
        BLE_DATACOMMON_CODED_PHY_CONST_OVERHEAD_DURATION_US + ((BLE_PACKET_HEADER_SIZE + BLE_SEC_MIC_LENGTH) * 16) + ((24 + 3) * 2),  /* gpHci_PhyWithCoding_Coded500kb */
        0 /* gpHci_PhyWithCoding_Invalid */
    };

    if(!GP_HCI_ENHANCED_PHY_TYPE_VALID(phy))
    {
        return 0;
    }

    return overheadDurationsUs[phy];
}

#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
/* Given a PHY and CteInfo presence, return the total time in us to transmit
   all non-payload bytes (including a MIC and 2 or 3 header bytes) ecluding the CTE */
UInt16 gpBleDataCommon_GetOverheadDurationUs_ForDF(gpHci_PhyWithCoding_t phy, Bool CteInfoPresent)
{
    UInt16 overheadDurationsUs = gpBleDataCommon_GetOverheadDurationUs(phy);
    if (CteInfoPresent)
    {
        UInt16 CteInfoDurationsUs[] =
        {
            0, /* gpHci_PhyWithCoding_Reserved */
            GPBLEDATACOMMON_CTE_INFO_SIZE * 8,    /* gpHci_PhyWithCoding_1Mb */
            GPBLEDATACOMMON_CTE_INFO_SIZE * 4     /* gpHci_PhyWithCoding_2Mb */
        };
        GP_ASSERT_DEV_INT( (gpHci_PhyWithCoding_1Mb == phy) || (gpHci_PhyWithCoding_2Mb == phy) );
        overheadDurationsUs += CteInfoDurationsUs[phy];
    }
    return overheadDurationsUs;
}
#endif /* GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED */

/* Given a PHY and a number of payload bytes, return the total time in us to transmit a packet on that PHY */
UInt16 gpBleDataCommon_GetPacketDurationUs(UInt16 payloadLengthBytes, gpHci_PhyWithCoding_t phy, Bool includeMic)
{
    UInt16 totalDurationUs;

    totalDurationUs = gpBleDataCommon_GetOverheadDurationUs(phy) + gpBleDataCommon_GetPayloadDurationUs(payloadLengthBytes, phy);

    if(!includeMic)
    {
        totalDurationUs -= gpBleDataCommon_GetPayloadDurationUs(BLE_SEC_MIC_LENGTH, phy);
    }

    return totalDurationUs;
}

#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
/* Given a PHY, number of payload bytes and CteDuration, return the total time in us to transmit a packet on that PHY */
UInt16 gpBleDataCommon_GetPacketDurationUs_ForDF(UInt16 payloadLengthBytes, gpHci_PhyWithCoding_t phy, Bool includeMic, UInt8 CteDuration)
{
    return (gpBleDataCommon_GetPacketDurationUs(payloadLengthBytes, phy, includeMic) +
            gpBleDataCommon_GetPayloadDurationUs(1, phy) +
            CteDuration);
}
#endif /* GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED */

/* Given a total packet duration and a phy mode, this function calculates how many octets fit inside */
UInt16 gpBleDataCommon_GetOctetsFromDurationUs(UInt16 totalTimeUs, gpHci_PhyWithCoding_t phy, UInt8 CteDuration)
{
    UInt16 overheadDurationUs;
    UInt16 octetsTimeUs;
    UInt8 byteDurationUs;

    if(!GP_HCI_ENHANCED_PHY_TYPE_VALID(phy))
    {
        GP_LOG_PRINTF("Invalid phy type: %x",0, phy);
        // Invalid or unknown PHY
        return 0;
    }

#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
    overheadDurationUs = gpBleDataCommon_GetOverheadDurationUs_ForDF(phy, CteDuration>0?true:false);
    //GP_ASSERT_DEV_INT( totalTimeUs > overheadDurationUs + CteDuration );
    // do not assert - just check all is sane, else return 0
    if (totalTimeUs >= CteDuration)
    {
        totalTimeUs -= CteDuration;
    }
    else
    {
        totalTimeUs = 0;
    }
#else
    overheadDurationUs = gpBleDataCommon_GetOverheadDurationUs(phy);
#endif /* GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED */

    if(overheadDurationUs >= totalTimeUs)
    {
        GP_LOG_PRINTF("no time for octets (overhead: %u time: %u)",0,overheadDurationUs, totalTimeUs);
        // More or equal overhead than total duration, no octets can fit inside
        return 0;
    }

    octetsTimeUs = totalTimeUs - overheadDurationUs;

    byteDurationUs = Ble_GetByteDurationUs(phy);

    return (octetsTimeUs / byteDurationUs);
}

gpHal_BleTxPhy_t gpBleDataCommon_HciPhyToHalTxPhy(gpHci_Phy_t hciPhy)
{
    return BleDataCommon_HciPhyToHalTxPhy(hciPhy, gpHci_PhyOptions_NoPreference);
}

gpHal_BleRxPhy_t gpBleDataCommon_HciPhyToHalRxPhy(gpHci_Phy_t hciPhy)
{
    return BleDataCommon_HciPhyToHalRxPhy(hciPhy);
}

gpHci_Phy_t gpBleDataCommon_HalRxPhyToHciPhy(gpHal_BleRxPhy_t hciPhy)
{
    return BleDataCommon_HalRxPhyToHciPhy(hciPhy);
}

UInt32 gpBleDataCommon_GetIntervalPortionOccupiedUs(Ble_IntConnId_t connId)
{
    // portionOccupiedUs corresponds to C in the LL spec
    UInt32 portionOccupiedUs;
    UInt16 effectiveMaxRxTime;
    // total packet duration for the effective max rx octets
    UInt16 effectiveOctetsDuration;

    GP_ASSERT_DEV_INT(Ble_DataIsConnectionIdValid(connId));

    effectiveMaxRxTime = gpBle_GetEffectiveMaxRxTime(connId);
    effectiveOctetsDuration = gpBle_GetEffectiveMaxRxOctets(connId)*Ble_GetByteDurationUs(gpHci_PhyWithCoding_Coded125kb);
    effectiveOctetsDuration += gpBleDataCommon_GetOverheadDurationUs(gpHci_PhyWithCoding_Coded125kb);

    portionOccupiedUs = (2*T_IFS) + min(effectiveMaxRxTime, effectiveOctetsDuration);

    return portionOccupiedUs;
}


// PHY conversion functions

gpHci_Phy_t gpBleDataCommon_PhyWithCodingToPhy(gpHci_PhyWithCoding_t phyWithCoding)
{
    gpHci_Phy_t hciPhy = gpHci_Phy_Invalid;

    switch(phyWithCoding)
    {
        case gpHci_PhyWithCoding_1Mb:
        {
            hciPhy = gpHci_Phy_1Mb;
            break;
        }
#ifdef GP_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED
        case gpHci_PhyWithCoding_2Mb:
        {
            hciPhy = gpHci_Phy_2Mb;
            break;
        }
#endif // GP_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED
        case gpHci_PhyWithCoding_Invalid:
        default:
        {
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }

    GP_ASSERT_DEV_INT(GP_HCI_ENHANCED_PHY_TYPE_VALID(phyWithCoding));

    return hciPhy;
}

gpHci_Phy_t gpBleDataCommon_HalTxPhyToHciPhy(gpHal_BleTxPhy_t halPhy)
{
    gpHci_Phy_t hciPhy = gpHci_Phy_Invalid;

    switch (halPhy)
    {
        case gpHal_BleTxPhy1Mb:
            hciPhy = gpHci_Phy_1Mb;
            break;
#ifdef GP_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED
        case gpHal_BleTxPhy2Mb:
            hciPhy = gpHci_Phy_2Mb;
            break;
#endif // GP_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED
        case gpHal_BleTxPhyInvalid:
        default:
            GP_ASSERT_DEV_INT(false);
            break;
    }
    return hciPhy;
}

gpHci_PhyWithCoding_t gpBleDataCommon_HalPhyWithCodingToHciPhyWithCoding(gpHal_BleTxPhy_t halPhy)
{
    COMPILE_TIME_ASSERT(gpHci_PhyWithCoding_1Mb        == (gpHal_BleTxPhy1Mb + 1));
    COMPILE_TIME_ASSERT(gpHci_PhyWithCoding_2Mb        == (gpHal_BleTxPhy2Mb + 1));

    return (gpHci_PhyWithCoding_t)(halPhy + 1);
}

gpHal_BleTxPhy_t gpBleDataCommon_HciPhyWithCodingToHalPhyWithCoding(gpHci_PhyWithCoding_t hciPhy)
{
    COMPILE_TIME_ASSERT(gpHci_PhyWithCoding_1Mb        == (gpHal_BleTxPhy1Mb + 1));
    COMPILE_TIME_ASSERT(gpHci_PhyWithCoding_2Mb        == (gpHal_BleTxPhy2Mb + 1));

    return (gpHci_PhyWithCoding_t)(hciPhy - 1);
}

gpHci_Phy_t BleDataCommon_HalRxPhyToHciPhy(gpHal_BleRxPhy_t halPhy)
{
    gpHci_Phy_t hciPhy = gpHci_Phy_Invalid;

    switch (halPhy)
    {
        case gpHal_BleRxPhy1Mb:
            hciPhy = gpHci_Phy_1Mb;
            break;
#ifdef GP_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED
        case gpHal_BleRxPhy2Mb:
            hciPhy = gpHci_Phy_2Mb;
            break;
#endif // GP_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED
        case gpHal_BleRxPhyInvalid:
        default:
            GP_ASSERT_DEV_INT(false);
            break;
    }
    return hciPhy;
}

/*****************************************************************************
 *                    Public Service Function Definitions
 *****************************************************************************/

#ifdef GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED
gpHci_Result_t gpBle_LeSetDataLength(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;
    gpHci_LeSetDataLengthCommand_t* pDataLength = &pParams->LeSetDataLength;

    GP_LOG_PRINTF("Set data length",0);

    // Use Command complete event to reply to a set data length command
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    // set connection Id into the command complete
    pEventBuf->payload.commandCompleteParams.returnParams.connectionHandle = pDataLength->connectionHandle;

    result = Ble_SetDataLengthChecker(pDataLength);

    if(result == gpHci_ResultSuccess)
    {
        result = Ble_SetDataLengthAction(pDataLength);
    }

    return result;
}

gpHci_Result_t gpBle_LeReadSuggestedDefDataLength(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_LeReadSuggestedDefDataLengthManual_t* pSuggestedLength;

    GP_LOG_PRINTF("Read suggested def data length",0);

    pSuggestedLength = &pEventBuf->payload.commandCompleteParams.returnParams.readSuggestedDefDataLength;

    // Use Command complete event to reply to a read suggested def data length command
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    pSuggestedLength->suggestedMaxTxOctets = Ble_DataGlobalContext.hostSuggestedMaxTxOctetsUnmodified;
    pSuggestedLength->suggestedMaxTxTime = Ble_DataGlobalContext.hostSuggestedMaxTxTimeUnmodified;

    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBle_LeWriteSuggestedDefDataLength(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;
    gpHci_LeWriteSuggestedDefDataLengthCommand_t* pSuggestedLength = &pParams->LeWriteSuggestedDefDataLength;

    GP_LOG_PRINTF("Write suggested def data length",0);

    // Use Command complete event to reply to a write suggested def data length command
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    result = Ble_WriteSuggestedDefDataLengthChecker(pSuggestedLength);

    if(result == gpHci_ResultSuccess)
    {
        Ble_WriteSuggestedDefDataLengthAction(pSuggestedLength);
    }

    return result;
}

gpHci_Result_t gpBle_LeReadMaxDataLength(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_ReadMaxDataLength_t* pReadMaxDataLength;
    GP_LOG_PRINTF("Read max data length",0);

    pReadMaxDataLength = &pEventBuf->payload.commandCompleteParams.returnParams.readMaxDataLength;

    // Use Command complete event to reply to a read max data length command
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    // Fill in return data
    pReadMaxDataLength->supportedMaxTxOctets = GPBLEDATACOMMON_OCTETS_SUPPORTED_MAX;
    pReadMaxDataLength->supportedMaxTxTime   = GPBLEDATACOMMON_TIME_SUPPORTED_MAX;
    pReadMaxDataLength->supportedMaxRxOctets = GPBLEDATACOMMON_OCTETS_SUPPORTED_MAX;
    pReadMaxDataLength->supportedMaxRxTime   = GPBLEDATACOMMON_TIME_SUPPORTED_MAX;

    return gpHci_ResultSuccess;
}
#endif //GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED

