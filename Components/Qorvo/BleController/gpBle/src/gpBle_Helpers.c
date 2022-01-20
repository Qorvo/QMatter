/*
 * Copyright (c) 2017, Qorvo Inc
 *
 *   Bluetooth Low Energy interface
 *   Implementation of gpBle
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
 * $Header: //depot/release/Embedded/Components/Qorvo/BleController/v2.10.2.0/comps/gpBle/src/gpBle_Helpers.c#1 $
 * $Change: 187624 $
 * $DateTime: 2021/12/20 10:58:50 $
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

//#define GP_LOCAL_LOG

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLE

#include "gpBle.h"
#include "gpBleAddressResolver.h"
#include "gpBleConfig.h"
#include "gpBleSecurityCoprocessor.h"
#include "gpBle_defs.h"
#include "gpBleActivityManager.h"
#include "gpHci_Includes.h"
#include "gpLog.h"
#include "gpPd.h"
#include "gpHal.h"
#include "gpSched.h"
#include "gpPoolMem.h"
#include "gpAssert.h"

#if defined(GP_DIVERSITY_BLE_BROADCASTER) || defined(GP_DIVERSITY_BLE_SLAVE)
#include "gpBleAdvertiser.h"
#endif //GP_DIVERSITY_BLE_BROADCASTER || GP_DIVERSITY_BLE_SLAVE

#if defined(GP_DIVERSITY_BLE_OBSERVER) || defined(GP_DIVERSITY_BLE_MASTER)
#include "gpBleScanner.h"
#endif //GP_DIVERSITY_BLE_OBSERVER || GP_DIVERSITY_BLE_MASTER

#if defined(GP_DIVERSITY_BLE_MASTER) || defined(GP_DIVERSITY_BLE_SLAVE)
#include "gpBleInitiator.h"
#include "gpBleLlcp.h"
#include "gpBleLlcpProcedures.h"
#include "gpBleDataChannelRxQueue.h"
#include "gpBleDataChannelTxQueue.h"
#include "gpBleDataCommon.h"
#include "gpBleDataRx.h"
#include "gpBleDataTx.h"
#endif //GP_DIVERSITY_BLE_MASTER || GP_DIVERSITY_BLE_SLAVE




/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define BLE_CLOCK_WRAP_HALF         (0x80000000)
#define BLE_CLOCK_VALUE_MAX         (0xFFFFFFFF)

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

void Ble_ParseAdvPdHeader(gpPd_Loh_t* pPdLoh, Ble_AdvChannelPduHeader_t* pHeader)
{
    UInt16 header;

    gpPd_ReadWithUpdate(pPdLoh, 2, (UInt8*)&header);

    pHeader->pduType = BLE_ADV_PDU_HEADER_TYPE_GET(header);

    pHeader->txAdd = BLE_ADV_PDU_HEADER_TXADD_GET(header);
    pHeader->rxAdd = BLE_ADV_PDU_HEADER_RXADD_GET(header);
    pHeader->length = BLE_ADV_PDU_HEADER_LENGTH_GET(header);

}

void Ble_ParseDataChannelPduHeader(gpPd_Loh_t* pPdLoh, Ble_DataChannelHeader_t* pHeader)
{
    UInt16 headerAndLength;

    MEMSET(pHeader, 0, sizeof(Ble_DataChannelHeader_t));
    // do not assert on incoming packets - handle "corrupt" packets gracefully
    if (pPdLoh->length < BLE_PACKET_HEADER_SIZE)
    {
        pHeader->llid = Ble_LLID_Reserved;
        return;
    }

    gpPd_ReadWithUpdate(pPdLoh, BLE_PACKET_HEADER_SIZE, (UInt8*)&headerAndLength);

    // MD, SN and NESN are not needed in NRT
    pHeader->llid = BLE_DATA_PDU_HEADER_LLID_GET(headerAndLength);
    pHeader->length = BLE_DATA_PDU_HEADER_LENGTH_GET(headerAndLength);


#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
    pHeader->cp = BLE_DATA_PDU_HEADER_CP_GET(headerAndLength);
    if(pHeader->cp && (pPdLoh->length >= 1))
    {
        // If the CP bit is set, we expect an extra header byte with CTEInfo
        gpPd_ReadWithUpdate(pPdLoh, 1, (UInt8*)&pHeader->cteInfo);
    }
#endif /* GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED */
}

void Ble_DumpAddress(BtDeviceAddress_t* pAddr)
{
    GP_LOG_SYSTEM_PRINTF("Address = %x:%x:%x:%x:%x:%x",0, pAddr->addr[5], pAddr->addr[4], pAddr->addr[3], pAddr->addr[2], pAddr->addr[1], pAddr->addr[0]);
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

Bool Ble_IsAdvPduHeaderValid(Ble_AdvChannelPduHeader_t* pAdvHeader)
{
    // Note that RT does not implement strict type checking on legacy adv PDUs
    if(!BLE_IS_ADV_PDU_TYPE_VALID(pAdvHeader->pduType))
    {
        GP_LOG_PRINTF("Adv pdu type invalid: %x",0,pAdvHeader->pduType);
        return false;
    }

    // It is unclear what we need to do with adv channel pdus with a length > 37 bytes.
    // According to the spec, these are invalid packets and Vol 6, part B, section 3.1 states that only valid packets shall be processed.
    // Vol 1 PArt E $2.7 "RESPONDING TO INVALID BEHAVIOR" allows a.o. : ignoring hte PDU, Attempt to recover the situation ... while not violating the spec
    // However, there are rumours that this length restriction of 37 bytes will be relaxed in future versions of the spec.
    // Therefore, for future compatibility, it would be handy if we do not drop packets with a length > 37 bytes.
    // This means, we would need to change our implementation (truncate advData to what fits in a legacy HCI LE Advertising Report event)
    // So we currently have chosen to check the length and drop it when it is invalid.
    if(!BLE_RANGE_CHECK(pAdvHeader->length, BLE_ADV_PDU_PAYLOAD_LENGTH_MIN, BLE_ADV_PDU_PAYLOAD_LENGTH_MAX))
    {
        GP_LOG_PRINTF("Adv pdu length not in range: %x <= %x <= %x",0,BLE_ADV_PDU_PAYLOAD_LENGTH_MIN, pAdvHeader->length, BLE_ADV_PDU_PAYLOAD_LENGTH_MAX);
        return false;
    }

    return true;
}


Int8 gpBle_ec_cmp(UInt16 commonPreceding, UInt16 first, UInt16 second)
{
    Int8 result;
    UInt16 d_first;
    UInt16 d_second;

    /* Note that we test this by comparing the differences to a know preceding event count
     * so we are indifferent to counter overflows! */
    d_first  = BLE_GET_EC_DIFF(commonPreceding, first);
    d_second = BLE_GET_EC_DIFF(commonPreceding, second);

    result = d_first < d_second ? -1 : (d_first > d_second ? 1 : 0);

    return result;
}

// Calculates the difference between two event counters
// It is assumed that the smallest difference is requested (meaning that the event counters are not further apart than 0xFFFF/2)
UInt16 gpBle_GetEcDifference(UInt16 ec1, UInt16 ec2)
{
    UInt16 smallestEcDifference = BLE_GET_EC_DIFF(ec1, ec2);

    if(smallestEcDifference > GP_BLE_EC_MAX_VALUE/2)
    {
        smallestEcDifference = BLE_GET_EC_DIFF(ec2, ec1);
    }

    return smallestEcDifference;
}

// Check if expectedEarliestEc occured before expectedLastEc (by calculating the EC distance)
Bool gpBle_IsEcEarlier(UInt16 expectedEarliestEc, UInt16 expectedLastEc)
{
    if(BLE_GET_EC_DIFF(expectedEarliestEc, expectedLastEc) <= GP_BLE_EC_MAX_VALUE/2 )
    {
        return true;
    }

    return false;
}

Bool gpBle_IsTimestampEarlier(UInt32 tsFirst, UInt32 tsSecond)
{
    // We assume that the timestamps are not spaced more than half the range of the clock
    if( (tsSecond - tsFirst) && (tsSecond - tsFirst < BLE_CLOCK_WRAP_HALF) )
    {
        // tsFirst is earlier
        return true;
    }
    else
    {
        // tsSecond is earlier or equal
        return false;
    }
}

Bool gpBle_IsTimestampEarlierOrEqual(UInt32 tsFirst, UInt32 tsSecond)
{
    // We assume that the timestamps are not spaced more than half the range of the clock
    if( (tsSecond == tsFirst) || (tsSecond - tsFirst < BLE_CLOCK_WRAP_HALF) )
    {
        // tsFirst is earlier or equal
        return true;
    }
    else
    {
        // tsSecond is earlier
        return false;
    }
}

UInt32 gpBle_GetTimeDifference(UInt32 tsFirst, UInt32 tsSecond)
{
    UInt32 diff;
    // We assume that tsFirst is earlier than tsSecond
    if (tsFirst > tsSecond)
    {   // in case of wrap
        diff = (BLE_CLOCK_VALUE_MAX - tsFirst) + tsSecond;
    }
    else
    {
        diff = tsSecond - tsFirst;
    }
    return diff;
}

UInt32 gpBle_GetGcd(UInt32 a, UInt32 b)
{
    UInt32 tmp;

    while(b > 0)
    {
        tmp = b;
        b = a % b;
        a = tmp;
    }

    return a;
}

UInt8 Ble_CountChannels(gpHci_ChannelMap_t* pChannelMap)
{
    UIntLoop i;
    UInt8 nrOfUsedChannels = 0;

    for(i = 0; i < BLE_DATA_NUMBER_OF_CHANNELS; i++)
    {
        UInt8 arrayIndex = (i >> 3);
        UInt8 arrayBitIndex = (UInt8)(i - (arrayIndex << 3));

        // Check if bit i is set in channel map
        if((pChannelMap->channels[arrayIndex] & BM(arrayBitIndex)) != 0)
        {
            nrOfUsedChannels++;
        }
    }
    return nrOfUsedChannels;
}

void gpBle_AppendWithUpdate(UInt8* pBuffer, UInt8* pValue, UInt8* pIndex, UInt8 length)
{
    MEMCPY(pBuffer, pValue, length);
    *pIndex += length;
}

#if defined(GP_DIVERSITY_JUMPTABLES)
Bool Ble_DiversityBleInitiatorEnabledInFlash(void)
{
    return BLE_DIVERSITY_BLE_INITIATOR_ENABLED_IN_FLASH();
}

Bool Ble_DiversityBleScannerEnabledInFlash(void)
{
    return BLE_DIVERSITY_BLE_SCANNER_ENABLED_IN_FLASH();
}

Bool Ble_DiversityBleExtAdvEnabledInFlash(void)
{
    return BLE_DIVERSITY_BLE_EXTADV_ENABLED_IN_FLASH();
}
#endif // defined(GP_DIVERSITY_JUMPTABLES)


gpHci_InitPeerAddressType_t Ble_GetHCiAdvAType(Ble_AdvChannelPduHeader_t *pAdvHeader, BtDeviceAddress_t* pAdvA, gpHal_BleAdvIndInfo_t* advIndInfo)
{
    gpHci_InitPeerAddressType_t PeerAddrType;

    {
        PeerAddrType = BLE_ADVPEER_ADDR_TYPE_TO_INITPEER_ADDR_TYPE(BLE_HAL_ADDR_BIT_TO_ADVPEER_ADDR_TYPE(pAdvHeader->txAdd), false);
    }
    return PeerAddrType;
}
