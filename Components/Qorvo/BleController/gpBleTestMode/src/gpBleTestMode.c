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

//#define GP_LOCAL_LOG

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLETESTMODE

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBle.h"
#include "gpBleComps.h"
#include "gpBleTestMode.h"
#include "gpBleDataCommon.h"
#include "gpBle_defs.h"
#include "gpPoolMem.h"
#include "gpHal.h"
#include "gpLog.h"

#ifdef GP_COMP_BLEDIRECTIONFINDING
#include "gpBleDirectionFinding.h"
#endif //GP_COMP_BLEDIRECTIONFINDING

#if defined(GP_DIVERSITY_BLE_BROADCASTER) || defined(GP_DIVERSITY_BLE_SLAVE) || defined(GP_DIVERSITY_BLE_ADVERTISER)
#include "gpBleAdvertiser.h"
#endif //GP_DIVERSITY_BLE_BROADCASTER || GP_DIVERSITY_BLE_SLAVE || GP_DIVERSITY_BLE_ADVERTISER

#if defined(GP_DIVERSITY_BLE_OBSERVER) || defined(GP_DIVERSITY_BLE_MASTER) || defined(GP_DIVERSITY_BLE_SCANNER)
#include "gpBleScanner.h"
#endif //GP_DIVERSITY_BLE_OBSERVER || GP_DIVERSITY_BLE_MASTER || GP_DIVERSITY_BLE_SCANNER

#if defined(GP_DIVERSITY_BLE_MASTER) || defined(GP_DIVERSITY_BLE_SLAVE)
#include "gpBleInitiator.h"
#endif //GP_DIVERSITY_BLE_MASTER || GP_DIVERSITY_BLE_SLAVE

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define BLE_TEST_MODE_PACKET_PAYLOAD_INVALID    0x08

#ifndef GP_DIVERSITY_BLE_DIRECT_TEST_MODE_DEFAULT_ANTENNA
#define GP_DIVERSITY_BLE_DIRECT_TEST_MODE_DEFAULT_ANTENNA gpHal_AntennaSelection_Ant0
#endif //GP_DIVERSITY_BLE_DIRECT_TEST_MODE_DEFAULT_ANTENNA

// Default antenna to use for tests
#define BLE_TEST_MODE_ANTENNA   GP_DIVERSITY_BLE_DIRECT_TEST_MODE_DEFAULT_ANTENNA

#define BLE_TEST_MODE_UNLIMITED_PACKET_TX       0xFFFF

#define BLE_TESTMODE_PDU_HEADER_TYPE_IDX    0
#define BLE_TESTMODE_PDU_HEADER_RFU1_IDX    4
#define BLE_TESTMODE_PDU_HEADER_CP_IDX      5
#define BLE_TESTMODE_PDU_HEADER_RFU2_IDX    6

#define BLE_TESTMODE_PDU_HEADER_TYPE_BM     0x0F
#define BLE_TESTMODE_PDU_HEADER_RFU1_BM     0x10
#define BLE_TESTMODE_PDU_HEADER_CP_BM       0x20
#define BLE_TESTMODE_PDU_HEADER_RFU2_BM     0xC0

#define BLE_TESTMODE_RECEIVER_TEST_SYNC_HANDLE  0x0FFF

// Number of overhead bytes in a testmode packet
#define BLE_TESTMODE_NON_PAYLOAD_OVERHEAD   (GP_HAL_PBM_BLE_NR_RESERVED_BYTES + BLE_ACCESS_ADDRESS_SIZE + BLE_PACKET_HEADER_SIZE + BLE_CTE_INFO_SIZE)
#define BLE_TESTMODE_PAYLOAD_LENGTH_MAX_SUPPORTED       (GP_HAL_PBM_MAX_SIZE - BLE_TESTMODE_NON_PAYLOAD_OVERHEAD)
#define BLE_TESTMODE_PAYLOAD_LENGTH_MAX_SPEC            0xFF

#define BLE_TESTMODE_MAX_PAYLOAD_LENGTH                 min(BLE_TESTMODE_PAYLOAD_LENGTH_MAX_SUPPORTED, BLE_TESTMODE_PAYLOAD_LENGTH_MAX_SPEC)

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

#define BLE_TEST_IS_PACKET_PAYLOAD_VALID(payload)   (payload < BLE_TEST_MODE_PACKET_PAYLOAD_INVALID)

#define BLE_TEST_ENHANCED_TX_IS_CODED_PHY(phy)      (phy == gpHci_PhyWithCoding_Coded125kb || phy == gpHci_PhyWithCoding_Coded500kb)

#define BLE_TESTMODE_PDU_HEADER_TYPE_SET(header, type)  BLE_BM_SET(header, BLE_TESTMODE_PDU_HEADER_TYPE_BM, BLE_TESTMODE_PDU_HEADER_TYPE_IDX,type)
#define BLE_TESTMODE_PDU_HEADER_TYPE_GET(header)        BLE_BM_GET(header, BLE_TESTMODE_PDU_HEADER_TYPE_BM, BLE_TESTMODE_PDU_HEADER_TYPE_IDX)
#define BLE_TESTMODE_PDU_HEADER_CP_SET(header, type)    BLE_BM_SET(header, BLE_TESTMODE_PDU_HEADER_CP_BM, BLE_TESTMODE_PDU_HEADER_CP_IDX,type)
#define BLE_TESTMODE_PDU_HEADER_CP_GET(header)          BLE_BM_GET(header, BLE_TESTMODE_PDU_HEADER_CP_BM, BLE_TESTMODE_PDU_HEADER_CP_IDX)

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/
typedef struct {
    Bool testBusy;
    UInt8 testModeAntenna;
    UInt16 numberOfTxPackets;
#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
    UInt8 cteLengthUnit;
    gpHci_CteType_t cteType;
    UInt8 rxChannel;
    UInt8 AntSwitchSequenceLength;
    UInt16 *pAntSwitchNibbleSequence;
#endif //GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
} Ble_TestModeContext_t;

// Identification of version command from the HCI spec
#define BleTestMode_TestModeVersionId_v1      0
#define BleTestMode_TestModeVersionId_v2      1
#define BleTestMode_TestModeVersionId_v3      2
typedef UInt8 BleTestMode_TestModeVersionId_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

static Ble_TestModeContext_t Ble_TestModeContext;


/*****************************************************************************
 *                    External Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/


// Checker and action functions
static gpHci_Result_t Ble_TestStartChecker(UInt8 channel);
static gpHci_Result_t Ble_ReceiverTestStartChecker(gpHci_LeReceiverTest_v3Command_t* pTest);
static gpHci_Result_t Ble_ReceiverTestAction(gpHci_LeReceiverTest_v3Command_t* pTest);
static gpHci_Result_t Ble_TransmitterTestStartChecker(gpHci_LeTransmitterTest_v3Command_t* pTest);
static gpHci_Result_t Ble_TransmitterTestAction(gpHci_LeTransmitterTest_v3Command_t* pTest);
static gpHci_Result_t BleTestMode_GenericRxTestFunction(BleTestMode_TestModeVersionId_t versionId, gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);
static gpHci_Result_t BleTestMode_GenericTxTestFunction(BleTestMode_TestModeVersionId_t versionId, gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);
static UInt16 Ble_TestModeEndAction(void);
static Bool Ble_IsServiceEnabled(void);

// Other
static void Ble_PRBS9(UInt8* buf, UInt8 len);
static gpHal_BleTxPhy_t Ble_TransmitterTxPhyToHalPhy(gpHci_PhyWithCoding_t hciPhy);

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/


UInt16 Ble_TestModeEndAction(void)
{
    gpHal_BleTestModeEnd_t testModeEnd;

    MEMSET(&testModeEnd, 0, sizeof(gpHal_BleTestModeEnd_t));

    if(Ble_TestModeContext.testBusy)
    {
        gpHal_Result_t halResult;

        halResult = gpHal_BleEndTestMode(&testModeEnd);

        GP_ASSERT_DEV_INT(halResult == gpHal_ResultSuccess);

        if (testModeEnd.pdHandle != GP_PD_INVALID_HANDLE)
        {
            // Transmitter test
            Ble_RMFreeResource(BLE_CONN_HANDLE_INVALID, testModeEnd.pdHandle);

            // A transmitter test shall have the nr of packets set to zero
            testModeEnd.nrOfPackets = 0;
        }

        Ble_TestModeContext.testBusy = false;
    }

#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED

    if (Ble_TestModeContext.pAntSwitchNibbleSequence)
    {
        gpBleDirectionFinding_DestroyAntennaSwitchSequence(&Ble_TestModeContext.pAntSwitchNibbleSequence);
    }
#endif /* GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED */

    return testModeEnd.nrOfPackets;
}

gpHci_Result_t Ble_TestStartChecker(UInt8 channel)
{
    if(Ble_TestModeContext.testBusy)
    {
        GP_LOG_PRINTF("Test mode already running",0);
        return gpHci_ResultCommandDisallowed;
    }

    if(Ble_IsServiceEnabled())
    {
        GP_LOG_PRINTF("Service(s) running, starting test mode not allowed",0);
        return gpHci_ResultCommandDisallowed;
    }

    if(!BLE_IS_CHANNEL_VALID(channel))
    {
        GP_LOG_PRINTF("channel %i invalid",0,channel);
        return gpHci_ResultInvalidHCICommandParameters;
    }

    return gpHci_ResultSuccess;
}

gpHci_Result_t Ble_ReceiverTestStartChecker(gpHci_LeReceiverTest_v3Command_t* pTest)
{
    gpHci_Result_t res;
#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
    // Check CTE related parameters
    if(pTest->expectedCteLengthUnit > 0)
    {
        if(!BLE_RANGE_CHECK(pTest->expectedCteLengthUnit, GP_BLEDIRECTIONFINDING_CTE_LENGTH_UNIT_MIN, GP_BLEDIRECTIONFINDING_CTE_LENGTH_UNIT_MAX))
        {
            GP_LOG_PRINTF("Expected CTE length unit invalid: %u <= %u <= %u",0, pTest->expectedCteLengthUnit, GP_BLEDIRECTIONFINDING_CTE_LENGTH_UNIT_MIN, GP_BLEDIRECTIONFINDING_CTE_LENGTH_UNIT_MAX);
            return gpHci_ResultInvalidHCICommandParameters;
        }

        if(pTest->phy == gpHci_Phy_Coded)
        {
            GP_LOG_PRINTF("CTE not allowed on phy %x",0, gpHci_Phy_Coded);
            return gpHci_ResultCommandDisallowed;
        }

        if(!GP_BLEDIRECTIONFINDING_IS_CTE_TYPE_VALID(pTest->expectedCteType))
        {
            GP_LOG_PRINTF("Invalid CTE type %x",0, pTest->expectedCteType);
            return gpHci_ResultInvalidHCICommandParameters;
        }

        if(pTest->expectedCteType == gpHci_CteTypeAoDConstantToneExt1us)
        {
            GP_LOG_PRINTF("Unsupported CTE type %x",0, pTest->expectedCteType);
            return gpHci_ResultUnsupportedFeatureOrParameterValue;
        }
        else
        if(pTest->expectedCteType == gpHci_CteTypeAoAConstantToneExt)
        {
            if(pTest->expectedSlotDurations != BLE_BLEDIRECTIONFINDING_SUPPORTED_CTE_SLOT_DURATION)
            {
                GP_LOG_PRINTF("Unsupported slot duration (%x us)",0, pTest->expectedSlotDurations);
                return gpHci_ResultUnsupportedFeatureOrParameterValue;
            }

            if(!gpBleDirectionFinding_CheckHciAntSwitchLenIsValid(pTest->switchingPatternLength, pTest->antennaIDs))
            {
                GP_LOG_PRINTF("Invalid ant switch length",0);
                return gpHci_ResultInvalidHCICommandParameters;
            }

            if ( (!gpBleDirectionFinding_CheckHciAntSwitchSequenceIsValid(pTest->switchingPatternLength, pTest->antennaIDs)) ||
                 (pTest->switchingPatternLength > GP_BLEDIRECTIONFINDING_SUPPORTED_ANTSWSEQLEN_MAX) )
            {
                GP_LOG_PRINTF("Invalid ant switch sequence OR too long ant switch sequence length",0);
                return gpHci_ResultUnsupportedFeatureOrParameterValue;
            }
        }
    }
#endif //GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED

    if(!BLE_IS_MODULATION_INDEX_VALID(pTest->modulationIndex))
    {
        GP_LOG_PRINTF("invalid modulation index %u", 0, pTest->modulationIndex);
        return gpHci_ResultInvalidHCICommandParameters;
    }

    if(!GP_HCI_PHY_TYPE_VALID(pTest->phy))
    {
        GP_LOG_PRINTF("Phy type %x not valid", 0, pTest->phy);
        return gpHci_ResultInvalidHCICommandParameters;
    }

    if(pTest->phy == gpHci_Phy_Coded)
    {
        GP_LOG_PRINTF("Phy type %x not supported", 0, pTest->phy);
        return gpHci_ResultUnsupportedFeatureOrParameterValue;
    }

    res = Ble_TestStartChecker(pTest->rxchannel);

    return res;
}

gpHci_Result_t Ble_ReceiverTestAction(gpHci_LeReceiverTest_v3Command_t* pTest)
{
    gpHal_Result_t halResult;
    gpHal_TestInfo_t testInfo;

    MEMSET(&testInfo, 0, sizeof(gpHal_TestInfo_t));

    testInfo.tx = false;
    testInfo.channel = pTest->rxchannel;
    testInfo.accesscode = GPHAL_BLE_DIRECT_TEST_MODE_SYNCWORD;
    testInfo.antenna = Ble_TestModeContext.testModeAntenna;
    testInfo.phy.rxPhy = gpBleDataCommon_HciPhyToHalRxPhy(pTest->phy);
    testInfo.pdLoh.handle = GP_PD_INVALID_HANDLE;

#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
    testInfo.cteLengthUnit = pTest->expectedCteLengthUnit;
    testInfo.cteType = pTest->expectedCteType;
    if ( (gpHci_CteTypeAoAConstantToneExt == pTest->expectedCteType) &&
         (pTest->expectedCteLengthUnit > 0)
       )
    {
        Bool dummy;
        GP_ASSERT_DEV_INT(pTest->switchingPatternLength <= GP_BLEDIRECTIONFINDING_SUPPORTED_ANTSWSEQLEN_MAX);

        if (!gpBleDirectionFinding_BuildAntennaSwitchSequence(pTest->antennaIDs,
                                             pTest->switchingPatternLength,
                                             &Ble_TestModeContext.pAntSwitchNibbleSequence,
                                             &Ble_TestModeContext.AntSwitchSequenceLength,
                                             &dummy)
           )
        {
            return gpHci_ResultMemoryCapacityExceeded;
        }
        testInfo.pAntennaIDs = (UInt8*)Ble_TestModeContext.pAntSwitchNibbleSequence;
        testInfo.switchingPatternLength = Ble_TestModeContext.AntSwitchSequenceLength;
    }
    else
    {
        testInfo.switchingPatternLength = 0;
    }
#endif //GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED

    halResult = gpHal_BleStartTestMode(&testInfo);

    if(halResult != gpHal_ResultSuccess)
    {
        return gpHci_ResultHardwareFailure;
    }

#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
    Ble_TestModeContext.cteLengthUnit = pTest->expectedCteLengthUnit;
    Ble_TestModeContext.cteType = pTest->expectedCteType;
    Ble_TestModeContext.rxChannel = pTest->rxchannel;
#endif //GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED

    Ble_TestModeContext.testBusy = true;

    return gpHci_ResultSuccess;
}

gpHci_Result_t Ble_TransmitterTestStartChecker(gpHci_LeTransmitterTest_v3Command_t* pTest)
{
#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
    // Check CTE related parameters
    if(pTest->expectedCteLengthUnit > 0)
    {
        if(!BLE_RANGE_CHECK(pTest->expectedCteLengthUnit, GP_BLEDIRECTIONFINDING_CTE_LENGTH_UNIT_MIN, GP_BLEDIRECTIONFINDING_CTE_LENGTH_UNIT_MAX))
        {
            GP_LOG_PRINTF("Expected CTE length unit invalid: %u <= %u <= %u",0, pTest->expectedCteLengthUnit, GP_BLEDIRECTIONFINDING_CTE_LENGTH_UNIT_MIN, GP_BLEDIRECTIONFINDING_CTE_LENGTH_UNIT_MAX);
            return gpHci_ResultInvalidHCICommandParameters;
        }

        if(pTest->phy == gpHci_Phy_Coded)
        {
            GP_LOG_PRINTF("CTE not allowed on phy %x",0, gpHci_Phy_Coded);
            return gpHci_ResultCommandDisallowed;
        }

        if(!GP_BLEDIRECTIONFINDING_IS_CTE_TYPE_VALID(pTest->expectedCteType))
        {
            GP_LOG_PRINTF("Invalid CTE type %x",0, pTest->expectedCteType);
            return gpHci_ResultInvalidHCICommandParameters;
        }

        if(pTest->expectedCteType == gpHci_CteTypeAoDConstantToneExt1us)
        {
            GP_LOG_PRINTF("Unsupported CTE type %x",0, pTest->expectedCteType);
            return gpHci_ResultUnsupportedFeatureOrParameterValue;
        }
        else
        if (pTest->expectedCteType == gpHci_CteTypeAoDConstantToneExt2us)
        {
            if(!gpBleDirectionFinding_CheckHciAntSwitchLenIsValid(pTest->switchingPatternLength, pTest->antennaIDs))
            {
                GP_LOG_PRINTF("Invalid ant switch sequence",0);
                return gpHci_ResultInvalidHCICommandParameters;
            }

            if ( (!gpBleDirectionFinding_CheckHciAntSwitchSequenceIsValid(pTest->switchingPatternLength, pTest->antennaIDs)) ||
                 (pTest->switchingPatternLength > GP_BLEDIRECTIONFINDING_SUPPORTED_ANTSWSEQLEN_MAX) )
            {
                GP_LOG_PRINTF("Invalid ant switch sequence OR too long ant switch sequence length",0);
                return gpHci_ResultUnsupportedFeatureOrParameterValue;
            }
        }
    }
#endif //GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED

    if(!GP_HCI_ENHANCED_PHY_TYPE_VALID(pTest->phy))
    {
        GP_LOG_PRINTF("Phy type %x not valid", 0, pTest->phy);
        return gpHci_ResultInvalidHCICommandParameters;
    }

    if(BLE_TEST_ENHANCED_TX_IS_CODED_PHY(pTest->phy))
    {
        GP_LOG_PRINTF("Phy type %x not supported", 0, pTest->phy);
        return gpHci_ResultUnsupportedFeatureOrParameterValue;
    }

    if((UInt16)pTest->length > BLE_TESTMODE_MAX_PAYLOAD_LENGTH)
    {
        GP_LOG_PRINTF("Unsupported payload length %u for phy %u", 0, pTest->length, pTest->phy);
        return gpHci_ResultUnsupportedFeatureOrParameterValue;
    }

    if(!BLE_TEST_IS_PACKET_PAYLOAD_VALID(pTest->payload))
    {
        GP_LOG_PRINTF("packet payload invalid", 0);
        return gpHci_ResultInvalidHCICommandParameters;
    }

    return Ble_TestStartChecker(pTest->txchannel);
}

gpHci_Result_t Ble_TransmitterTestAction(gpHci_LeTransmitterTest_v3Command_t* pTest)
{
    gpHci_Result_t result;
    gpHal_TestInfo_t testInfo;
    UInt8 *testFrame=NULL;
    UInt32 syncword=GPHAL_BLE_DIRECT_TEST_MODE_SYNCWORD;
    UInt8 pduHeader = 0;
    UInt8 length;

    length = pTest->length;

    MEMSET(&testInfo, 0, sizeof(gpHal_TestInfo_t));

    testInfo.tx = true;
    testInfo.channel = pTest->txchannel;
    testInfo.antenna = Ble_TestModeContext.testModeAntenna;
    testInfo.accesscode = syncword;
    testInfo.phy.txPhy = Ble_TransmitterTxPhyToHalPhy(pTest->phy);
    testInfo.txPacketCount = Ble_TestModeContext.numberOfTxPackets;

    testFrame = GP_POOLMEM_MALLOC(length);

    if(testFrame)
    {
        switch(pTest->payload)
        {
            case 0: Ble_PRBS9(testFrame, length); break;
            case 1: MEMSET(testFrame, 0x0F, length); break;
            case 2: MEMSET(testFrame, 0x55, length); break;
            case 3: MEMSET(testFrame, 0xFF, length); break; // PRBS15 is only used on the tester side: not needed in IUT
            case 4: MEMSET(testFrame, 0xFF, length); break;
            case 5: MEMSET(testFrame, 0x00, length); break;
            case 6: MEMSET(testFrame, 0xF0, length); break;
            case 7: MEMSET(testFrame, 0xAA, length); break;
            default: break;
        }
    }
    else
    {
        return gpHci_ResultMemoryCapacityExceeded;
    }

    result = Ble_RMGetResource(&testInfo.pdLoh);

    if(result != gpHci_ResultSuccess)
    {
        gpPoolMem_Free(testFrame);
        return result;
    }

    testInfo.pdLoh.length = 0;
    testInfo.pdLoh.offset = GP_BLE_ADV_CHANNEL_PDU_MAX_OFFSET;

#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
    testInfo.cteLengthUnit = pTest->expectedCteLengthUnit;
    testInfo.cteType = pTest->expectedCteType;

    if ( (gpHci_CteTypeAoDConstantToneExt2us == pTest->expectedCteType) &&
         (pTest->expectedCteLengthUnit > 0)
       )
    {
        Bool dummy;
        GP_ASSERT_DEV_INT(pTest->switchingPatternLength <= GP_BLEDIRECTIONFINDING_SUPPORTED_ANTSWSEQLEN_MAX);

        if (!gpBleDirectionFinding_BuildAntennaSwitchSequence(pTest->antennaIDs,
                                             pTest->switchingPatternLength,
                                             &Ble_TestModeContext.pAntSwitchNibbleSequence,
                                             &Ble_TestModeContext.AntSwitchSequenceLength,
                                             &dummy)
           )
        {
            gpPoolMem_Free(testFrame);
            Ble_RMFreeResource(BLE_CONN_HANDLE_INVALID, testInfo.pdLoh.handle);
            return gpHci_ResultMemoryCapacityExceeded;
        }
        testInfo.pAntennaIDs = (UInt8*)Ble_TestModeContext.pAntSwitchNibbleSequence;
        testInfo.switchingPatternLength = Ble_TestModeContext.AntSwitchSequenceLength;
    }
    else
    {
        testInfo.switchingPatternLength = 0;
    }

    if(pTest->expectedCteLengthUnit > 0)
    {
        BLE_TESTMODE_PDU_HEADER_CP_SET(pduHeader, 0x01);
    }
#endif //GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED

    BLE_TESTMODE_PDU_HEADER_TYPE_SET(pduHeader, pTest->payload);

    // TestPattern
    gpPd_PrependWithUpdate(&testInfo.pdLoh, length, testFrame);

#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
    if(BLE_TESTMODE_PDU_HEADER_CP_GET(pduHeader))
    {
        UInt8 cteInfo = 0;

        BLE_CTEINFO_TIME_SET(cteInfo, pTest->expectedCteLengthUnit);
        BLE_CTEINFO_TYPE_SET(cteInfo, pTest->expectedCteType);

        // Add CTEInfo byte when CP bit is set
        gpPd_PrependWithUpdate(&testInfo.pdLoh, 1, &cteInfo);

        // Add CTE to the PBM
        gpHal_PbmSetCteLengthUs(testInfo.pdLoh.handle, pTest->expectedCteLengthUnit*8);
    }
#endif //GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED

    // length
    gpPd_PrependWithUpdate(&testInfo.pdLoh, 1, &pTest->length);
    //header
    gpPd_PrependWithUpdate(&testInfo.pdLoh, 1, &pduHeader);
    // sync
    gpPd_PrependWithUpdate(&testInfo.pdLoh, 4, (UInt8*)&syncword);

    result = gpHal_BleStartTestMode(&testInfo);

    if(result == gpHal_ResultSuccess)
    {
        Ble_TestModeContext.testBusy = true;
    }
    else
    {
        // Request failed, free pbm
        Ble_RMFreeResource(BLE_CONN_HANDLE_INVALID, testInfo.pdLoh.handle);
        result = gpHci_ResultHardwareFailure;
    }

    gpPoolMem_Free(testFrame);

    return result;
}

Bool Ble_IsServiceEnabled(void)
{
#ifdef GP_DIVERSITY_BLE_SLAVE
    if(gpBleAdvertiser_IsEnabled())
    {
        return true;
    }
#endif //GP_DIVERSITY_BLE_SLAVE

#ifdef GP_DIVERSITY_BLE_MASTER
    if(gpBleScanner_IsEnabled())
    {
        return true;
    }
    if(gpBleInitiator_IsEnabled())
    {
        return true;
    }
#endif //GP_DIVERSITY_BLE_MASTER

    return false;
}

/* function to generate a PRBS9 testpattern */
void Ble_PRBS9(UInt8* buf, UInt8 len)
{
    UInt16 i,j;
    UInt16 a = 0x01ff;
    UInt16 newbit;
    UInt16 output;

    for(j=0; j<len; j++)
    {
        output = 0x00;
        for(i=0; i<8; i++)
        {
            //polynomial: x9+x5+1
            newbit = ((a >> (9-1)) ^ (a >> (9-5))) & 0x01;

            //output from the shift register
            output |= ((a >> 8) << i);

            //shift and update the register
            a = ((a << 1) | newbit) & 0x01ff;
        }
        buf[j] = output & 0xff;
    }
}

gpHal_BleTxPhy_t Ble_TransmitterTxPhyToHalPhy(gpHci_PhyWithCoding_t txTestPhy)
{
    gpHal_BleTxPhy_t rv = gpHal_BleTxPhyInvalid;

    if(!GP_HCI_ENHANCED_PHY_TYPE_VALID(txTestPhy))
    {
        return gpHal_BleTxPhyInvalid;
    }

    switch (txTestPhy)
    {
        case gpHci_PhyWithCoding_1Mb:
            rv = gpHal_BleTxPhy1Mb;
            break;
#ifdef GP_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED
        case gpHci_PhyWithCoding_2Mb:
            rv = gpHal_BleTxPhy2Mb;
            break;
#endif // GP_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED
        default:
            break;
    }

    return rv;
}

gpHci_Result_t BleTestMode_GenericRxTestFunction(BleTestMode_TestModeVersionId_t versionId, gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;
    gpHci_LeReceiverTest_v3Command_t rxTest;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    if(versionId == BleTestMode_TestModeVersionId_v3)
    {
        MEMCPY(&rxTest, &pParams->LeReceiverTest_v3, sizeof(gpHci_LeReceiverTest_v3Command_t));
    }
    else
    {
        // Set CTE parameters to defaults (not applicable)
        MEMSET(&rxTest, 0x00, sizeof(gpHci_LeReceiverTest_v3Command_t));

        if(versionId == BleTestMode_TestModeVersionId_v2)
        {
            rxTest.phy = pParams->LeEnhancedReceiverTest.phy;
            rxTest.modulationIndex = pParams->LeEnhancedReceiverTest.modulationIndex;
            rxTest.rxchannel = pParams->LeEnhancedReceiverTest.rxchannel;
        }
        else
        {
            rxTest.phy = gpHci_PhyWithCoding_1Mb;
            rxTest.modulationIndex = gpHci_ModulationIndex_Standard;
            rxTest.rxchannel = pParams->LeReceiverTest.rxchannel;
        }
    }

    result = Ble_ReceiverTestStartChecker(&rxTest);

    if(result == gpHci_ResultSuccess)
    {
        result = Ble_ReceiverTestAction(&rxTest);
    }

    return result;
}

gpHci_Result_t BleTestMode_GenericTxTestFunction(BleTestMode_TestModeVersionId_t versionId, gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;
    gpHci_LeTransmitterTest_v3Command_t txTest;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    if(versionId == BleTestMode_TestModeVersionId_v3)
    {
        MEMCPY(&txTest, &pParams->LeTransmitterTest_v3, sizeof(gpHci_LeTransmitterTest_v3Command_t));
    }
    else
    {
        // Set CTE parameters to defaults (not applicable)
        MEMSET(&txTest, 0x00, sizeof(gpHci_LeTransmitterTest_v3Command_t));

        txTest.expectedCteType = gpHci_CteTypeAoAConstantToneExt;
        txTest.switchingPatternLength = 0x02;

        if(versionId == BleTestMode_TestModeVersionId_v2)
        {
            txTest.txchannel = pParams->LeEnhancedTransmitterTest.txchannel;
            txTest.length = pParams->LeEnhancedTransmitterTest.length;
            txTest.payload = pParams->LeEnhancedTransmitterTest.payload;
            txTest.phy = pParams->LeEnhancedTransmitterTest.phy;
        }
        else
        {
            // Set PHY to default
            txTest.phy = gpHci_PhyWithCoding_1Mb;
            txTest.txchannel = pParams->LeTransmitterTest.txchannel;
            txTest.length = pParams->LeTransmitterTest.length;
            txTest.payload = pParams->LeTransmitterTest.payload;
        }
    }

    result = Ble_TransmitterTestStartChecker(&txTest);

    if(result == gpHci_ResultSuccess)
    {
        result = Ble_TransmitterTestAction(&txTest);
    }

    return result;
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBle_TestModeInit(gpHal_BleCallbacks_t* pCallbacks)
{
    COMPILE_TIME_ASSERT(BLE_TEST_MODE_ANTENNA == gpHal_AntennaSelection_Ant0 || BLE_TEST_MODE_ANTENNA == gpHal_AntennaSelection_Ant1);

}

/*****************************************************************************
 *                    Service Function Definitions
 *****************************************************************************/

gpHci_Result_t gpBle_LeReceiverTest(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    return BleTestMode_GenericRxTestFunction(BleTestMode_TestModeVersionId_v1, pParams, pEventBuf);
}

gpHci_Result_t gpBle_LeTransmitterTest(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    return BleTestMode_GenericTxTestFunction(BleTestMode_TestModeVersionId_v1, pParams, pEventBuf);
}

gpHci_Result_t gpBle_LeEnhancedReceiverTest(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    return BleTestMode_GenericRxTestFunction(BleTestMode_TestModeVersionId_v2, pParams, pEventBuf);
}

gpHci_Result_t gpBle_LeEnhancedTransmitterTest(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    return BleTestMode_GenericTxTestFunction(BleTestMode_TestModeVersionId_v2, pParams, pEventBuf);
}

gpHci_Result_t gpBle_LeReceiverTest_v3(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    return BleTestMode_GenericRxTestFunction(BleTestMode_TestModeVersionId_v3, pParams, pEventBuf);
}

gpHci_Result_t gpBle_LeTransmitterTest_v3(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    return BleTestMode_GenericTxTestFunction(BleTestMode_TestModeVersionId_v3, pParams, pEventBuf);
}

gpHci_Result_t gpBle_LeTestEnd(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    pEventBuf->payload.commandCompleteParams.returnParams.testResult = Ble_TestModeEndAction();

    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBle_SetVsdDirectTestTxPacketCountHelper( gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    MEMCPY((UInt8*)&Ble_TestModeContext.numberOfTxPackets, pParams->SetVsdTestParams.value, sizeof(Ble_TestModeContext.numberOfTxPackets));

    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBle_SetVsdDirectTestModeAntennaHelper( gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    if(Ble_TestModeContext.testBusy)
        return gpHci_ResultControllerBusy;

    MEMCPY((UInt8*)&Ble_TestModeContext.testModeAntenna, pParams->SetVsdTestParams.value, sizeof(Ble_TestModeContext.testModeAntenna));

    GP_LOG_PRINTF("Switch to ant: %d", 0, Ble_TestModeContext.testModeAntenna);

    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBle_VsdSetTransmitPower(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHal_Result_t halResult;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    halResult = gpHal_BleSetTxPower(pParams->VsdSetTransmitPower.transmitPower);

    if(halResult != gpHal_ResultSuccess)
    {
        return gpHci_ResultInvalidHCICommandParameters;
    }

    return gpHci_ResultSuccess;
}

void gpBle_TestModeReset(Bool firstReset)
{
    if(firstReset)
    {
        Ble_TestModeContext.testBusy = false;
        Ble_TestModeContext.numberOfTxPackets = BLE_TEST_MODE_UNLIMITED_PACKET_TX;
    }

    // End test mode if pending
    Ble_TestModeEndAction();

    //Initialize testMode antenna to default
    Ble_TestModeContext.testModeAntenna = BLE_TEST_MODE_ANTENNA;
}
