/*
 * Copyright (c) 2020, Qorvo Inc
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

/** @file "gpTest_marshalling.c"
 *
 *  Low level Test functions
 *
 *   Marshalling structures and functions.
*/

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

// General includes
#include "gpTest.h"
#include "gpTest_marshalling.h"

#ifdef GP_DIVERSITY_LOG
#include "gpLog.h"
#endif

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID GP_COMPONENT_ID_TEST

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

 /*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

 /*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

gpMarshall_AckStatus_t gpTest_Statistics_t_buf2api(gpTest_Statistics_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt16_buf2api_1(&(pDest->ReceivedPacketCounterC), pSource, pIndex);
        UInt16_buf2api_1(&(pDest->ReceivedPacketCounterL), pSource, pIndex);
        UInt16_buf2api_1(&(pDest->ReceivedPacketCounterD), pSource, pIndex);
        UInt16_buf2api_1(&(pDest->PacketsSentOK), pSource, pIndex);
        UInt16_buf2api_1(&(pDest->PacketsSentError), pSource, pIndex);
        Int32_buf2api_1(&(pDest->CumulativeRssi), pSource, pIndex);
        UInt32_buf2api_1(&(pDest->CumulativeLQI), pSource, pIndex);
        UInt16_buf2api_1(&(pDest->RxAntenna0), pSource, pIndex);
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpTest_Statistics_t_api2buf(UInt8Buffer* pDest , const gpTest_Statistics_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt16_api2buf_1(pDest , &(pSource->ReceivedPacketCounterC), pIndex);
        UInt16_api2buf_1(pDest , &(pSource->ReceivedPacketCounterL), pIndex);
        UInt16_api2buf_1(pDest , &(pSource->ReceivedPacketCounterD), pIndex);
        UInt16_api2buf_1(pDest , &(pSource->PacketsSentOK), pIndex);
        UInt16_api2buf_1(pDest , &(pSource->PacketsSentError), pIndex);
        Int32_api2buf_1(pDest , &(pSource->CumulativeRssi), pIndex);
        UInt32_api2buf_1(pDest , &(pSource->CumulativeLQI), pIndex);
        UInt16_api2buf_1(pDest , &(pSource->RxAntenna0), pIndex);
        pSource++;
    }
}

gpMarshall_AckStatus_t gpTest_Settings_t_buf2api(gpTest_Settings_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt8_buf2api_1(&(pDest->AntennaMode), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->SelectedAntenna), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->SelectedChannel), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->ContinuousWave), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->MaxBE), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->MinBE), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->MaxCSMABackoffs), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->MaxRetries), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->CSMAMode), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->PacketInPacket), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->RxOnWhenIdle), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->SleepMode), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->PowerSetting), pSource, pIndex);
        UInt8_buf2api_1(&(pDest->PromiscuousMode), pSource, pIndex);
        UInt16_buf2api_1(&(pDest->PanID), pSource, pIndex);
        UInt16_buf2api_1(&(pDest->ShortAddress), pSource, pIndex);
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpTest_Settings_t_api2buf(UInt8Buffer* pDest , const gpTest_Settings_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt8_api2buf_1(pDest , &(pSource->AntennaMode), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->SelectedAntenna), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->SelectedChannel), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->ContinuousWave), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->MaxBE), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->MinBE), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->MaxCSMABackoffs), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->MaxRetries), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->CSMAMode), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->PacketInPacket), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->RxOnWhenIdle), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->SleepMode), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->PowerSetting), pIndex);
        UInt8_api2buf_1(pDest , &(pSource->PromiscuousMode), pIndex);
        UInt16_api2buf_1(pDest , &(pSource->PanID), pIndex);
        UInt16_api2buf_1(pDest , &(pSource->ShortAddress), pIndex);
        pSource++;
    }
}

gpMarshall_AckStatus_t gpTest_CntPrio_t_buf2api(gpTest_CntPrio_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt16_buf2api_1(&(pDest->prio0), pSource, pIndex);
        UInt16_buf2api_1(&(pDest->prio1), pSource, pIndex);
        UInt16_buf2api_1(&(pDest->prio2), pSource, pIndex);
        UInt16_buf2api_1(&(pDest->prio3), pSource, pIndex);
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpTest_CntPrio_t_api2buf(UInt8Buffer* pDest , const gpTest_CntPrio_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt16_api2buf_1(pDest , &(pSource->prio0), pIndex);
        UInt16_api2buf_1(pDest , &(pSource->prio1), pIndex);
        UInt16_api2buf_1(pDest , &(pSource->prio2), pIndex);
        UInt16_api2buf_1(pDest , &(pSource->prio3), pIndex);
        pSource++;
    }
}

gpMarshall_AckStatus_t gpTest_StatisticsCounter_t_buf2api(gpTest_StatisticsCounter_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex )
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt16_buf2api_1(&(pDest->ccaFails), pSource, pIndex);
        UInt16_buf2api_1(&(pDest->txRetries), pSource, pIndex);
        UInt16_buf2api_1(&(pDest->failTxNoAck), pSource, pIndex);
        UInt16_buf2api_1(&(pDest->failTxChannelAccess), pSource, pIndex);
        UInt16_buf2api_1(&(pDest->successTx), pSource, pIndex);
        gpTest_CntPrio_t_buf2api(&(pDest->coexReq), pSource, 1, pIndex);
        gpTest_CntPrio_t_buf2api(&(pDest->coexGrant), pSource, 1, pIndex);
        UInt16_buf2api_1(&(pDest->totalRx), pSource, pIndex);
        UInt16_buf2api_1(&(pDest->pbmOverflow), pSource, pIndex);
        pDest++;
    }
    return gpMarshall_AckStatusSuccess;
}

void gpTest_StatisticsCounter_t_api2buf(UInt8Buffer* pDest , const gpTest_StatisticsCounter_t* pSource , UInt16 length , UInt16* pIndex)
{
    UIntLoop i;
    for(i = 0; i < length; i++)
    {
        UInt16_api2buf_1(pDest , &(pSource->ccaFails), pIndex);
        UInt16_api2buf_1(pDest , &(pSource->txRetries), pIndex);
        UInt16_api2buf_1(pDest , &(pSource->failTxNoAck), pIndex);
        UInt16_api2buf_1(pDest , &(pSource->failTxChannelAccess), pIndex);
        UInt16_api2buf_1(pDest , &(pSource->successTx), pIndex);
        gpTest_CntPrio_t_api2buf(pDest , &(pSource->coexReq), 1, pIndex);
        gpTest_CntPrio_t_api2buf(pDest , &(pSource->coexGrant), 1, pIndex);
        UInt16_api2buf_1(pDest , &(pSource->totalRx), pIndex);
        UInt16_api2buf_1(pDest , &(pSource->pbmOverflow), pIndex);
        pSource++;
    }
}


void gpTest_InitMarshalling(void)
{
}


