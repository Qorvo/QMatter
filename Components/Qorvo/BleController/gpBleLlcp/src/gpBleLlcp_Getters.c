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
 *
 */

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLELLCP

#include "gpBle.h"
#include "gpBleLlcp.h"
#include "gpBle_LLCP_getters.h"
#ifdef GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
#include "gpBleConfig.h"
#endif // GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
#include "gpBleDataCommon.h"

UInt32 gpBleLlcp_GetConnIntervalUnit(Ble_IntConnId_t connId)
{
    Ble_LlcpLinkContext_t* pContext;

    pContext = Ble_GetLinkContext(connId);

    GP_ASSERT_DEV_INT(pContext);

    return pContext->intervalUnit;
}

UInt32 gpBleLlcp_GetConnIntervalUs(Ble_IntConnId_t connId)
{
    Ble_LlcpLinkContext_t* pContext;

    pContext = Ble_GetLinkContext(connId);

    GP_ASSERT_DEV_INT(pContext);

    return BLE_TIME_UNIT_1250_TO_US(pContext->intervalUnit);
}

UInt16 gpBleLlcp_GetLatency(Ble_IntConnId_t connId)
{
    Ble_LlcpLinkContext_t* pContext;

    pContext = Ble_GetLinkContext(connId);

    GP_ASSERT_DEV_INT(pContext);

    return pContext->latency;
}

Bool gpBleLlcp_IsMasterConnection(Ble_IntConnId_t connId)
{
    Ble_LlcpLinkContext_t* pContext;

    pContext = Ble_GetLinkContext(connId);

    GP_ASSERT_DEV_INT(pContext);

    return pContext->masterConnection;
}

UInt16 gpBleLlcp_GetMinCeUnit(Ble_IntConnId_t connId)
{
    Ble_LlcpLinkContext_t* pContext;
    UInt16 minCELength = 0;

    pContext = Ble_GetLinkContext(connId);

    if(pContext == NULL)
    {
        GP_ASSERT_DEV_INT(false);
        return minCELength;
    }

    minCELength = pContext->ccParams.minCELength;

    if(gpBle_GetEffectivePhyRxType(connId) == gpHci_Phy_Coded && gpBle_GetEffectivePhyTxTypeWithCoding(connId) == gpHci_PhyWithCoding_Coded125kb)
    {
        if(minCELength == 1)
        {
            // In this case, the host specified a minCe length that is too short for a coded PHY transaction.
            // Increase it manually and allow at least the exchange of an empty PDU pair.
            minCELength += 1;
        }
    }

    return minCELength;
}
