/*
* Copyright (c) 2015-2016, GreenPeak Technologies
* Copyright (c) 2017, Qorvo Inc
*
* gpHal_MAC.c
*   This file contains the implementation of the MAC functions
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
* $Header$
* $Change$
* $DateTime$
*
*/

/*****************************************************************************
*                    Includes Definitions
*****************************************************************************/
//#define GP_LOCAL_LOG

#include "gpPd.h"

#include "gpHal.h"
#include "gpHal_DEFS.h"
#include "gpHal_Ble.h"
#include "gpHal_Ble_DEFS.h"

//GP hardware dependent register definitions
#include "gpHal_HW.h"
#include "gpHal_reg.h"

#ifdef GP_COMP_TXMONITOR
#include "gpTxMonitor.h"
#endif //GP_COMP_TXMONITOR

#include "gpHal_RadioMgmt.h"
/*****************************************************************************
*                    Macro Definitions
*****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_GPHAL

// Test Event Info memory
#ifdef GP_COMP_CHIPEMU
extern UInt32 gpChipEmu_GetGpMicroStructBleTestInfoStart(UInt32 gp_mm_ram_linear_start);
#define GP_HAL_BLE_TEST_INFO_START                      gpChipEmu_GetGpMicroStructBleTestInfoStart(GP_MM_RAM_LINEAR_START)
#define GP_HAL_BLE_DIRECTION_FINDING_SAMPLES_START      gpChipEmu_GetGpMicroStructCteSamplesStart(GP_MM_RAM_LINEAR_START)
#else
#define GP_HAL_BLE_TEST_INFO_START           ((UIntPtr)&gpHal_BleTestInfo[0])
#define GP_HAL_BLE_DIRECTION_FINDING_SAMPLES_START           ((UIntPtr)&gpHal_BleTestModeSampleBuffer[0])
#endif // GP_COMP_CHIPEMU

#define GPHAL_BLE_DIRECT_TEST_MODE_CRC          0x555555

// Preamble definitions
#define GP_HAL_BLE_PREAMBLE_SYMBOL_ADV_CHANNEL_UNCODED  0x55
#define GP_HAL_BLE_PREAMBLE_SYMBOL_ADV_CHANNEL_CODED    0x3C

#define GPHAL_BLE_TIME_SLOT_DURATION_US                 625

#define GPHAL_BLE_TESTMODE_VALIDATION_THRESH_NDR                    GPHAL_BLE_VALIDATION_THRESHOLD_DEFAULT
#define GPHAL_BLE_TESTMODE_VALIDATION_START_IDX_NDR                 7
#define GPHAL_BLE_TESTMODE_VALIDATION_FAKE_PREAMBLE_PRESENT_NDR     0
#define GPHAL_BLE_TESTMODE_VALIDATION_FAKE_PREAMBLE_START_IDX_NDR   0

// Currently HDR settings are same as NDR, but this can always change
#define GPHAL_BLE_TESTMODE_VALIDATION_THRESH_HDR                    GPHAL_BLE_TESTMODE_VALIDATION_THRESH_NDR
#define GPHAL_BLE_TESTMODE_VALIDATION_START_IDX_HDR                 GPHAL_BLE_TESTMODE_VALIDATION_START_IDX_NDR
#define GPHAL_BLE_TESTMODE_VALIDATION_FAKE_PREAMBLE_PRESENT_HDR     GPHAL_BLE_TESTMODE_VALIDATION_FAKE_PREAMBLE_PRESENT_NDR
#define GPHAL_BLE_TESTMODE_VALIDATION_FAKE_PREAMBLE_START_IDX_HDR   GPHAL_BLE_TESTMODE_VALIDATION_FAKE_PREAMBLE_START_IDX_NDR

/*****************************************************************************
*                   Functional Macro Definitions
*****************************************************************************/

#define GP_HAL_BLE_TEST_INFO_TO_OFFSET_FROM_START()         GPHAL_BLE_RAM_ADDRESS_TO_START_OFFSET(GP_HAL_BLE_TEST_INFO_START)

// Helper macro's to easy backup and restore registers
#define LOCAL_BACKUP(prop) prop##_backup = GP_WB_READ_##prop()
#define LOCAL_RESTORE(prop) GP_WB_WRITE_##prop( prop##_backup )

/*****************************************************************************
*                    Type Definitions
*****************************************************************************/

// Desense fix related state we need to backup and restore
typedef struct {
    Int8 xoLdoRefbits;
    Int8 bpfLdoDigRefbits;
} gpHal_BleTestModeDsFix_t;

/*****************************************************************************
*                    Static Data Definitions
*****************************************************************************/

// Info structure memory (shared with RT)
#ifndef GP_COMP_CHIPEMU
/* compile time verification of info structures */
GP_COMPILE_TIME_VERIFY(GPHAL_BLE_TEST_INFO_SIZE >= GP_WB_BLE_TEST_INFO_SIZE);

COMPILER_ALIGNED(GP_WB_MAX_MEMBER_SIZE) static UInt8 gpHal_BleTestInfo[GPHAL_BLE_TEST_INFO_SIZE] LINKER_SECTION(".lower_ram_retain_gpmicro_accessible");

#endif // GP_COMP_CHIPEMU

// Indicates RT is running in (direct) test mode
static Bool gpHal_BleTestModeActive;

#ifndef GP_COMP_CHIPEMU
static gpHal_BleTestModeDsFix_t gpHal_BleTestModeDsFixContext;
static UInt16 gpHal_DSFix_ChannelListPtr;
#endif //GP_COMP_CHIPEMU


/*****************************************************************************
*                    Static Function Prototypes
*****************************************************************************/

static UInt16 gpHalBle_GetPacketInterval(UInt8 pbmHandle, UInt16 pbmLength, gpHal_BleTxPhy_t phy);
static void gpHal_BlePopulateValidationSettings(gpHal_phyMask_t phyMask);
static gpHal_Result_t gpHal_BlePopulateTestInfo(gpHal_TestInfo_t* pInfo);
#ifndef GP_COMP_CHIPEMU
static void gpHal_BleDsFixStartTestModeHook(UInt8 phyChannel);
static void gpHal_BleDsFixEndTestModeHook(void);
#endif //GP_COMP_CHIPEMU
static gpHal_BleRxPhy_t rxPhyMaskToRxPhy(gpHal_phyMask_t mask);

/*****************************************************************************
*                    Static Function Definitions
*****************************************************************************/

UInt16 gpHalBle_GetPacketInterval(UInt8 pbmHandle, UInt16 pbmLength, gpHal_BleTxPhy_t phy)
{
    UInt16 totalDurationUs;
    UInt16 packetIntervalUnit;
    UInt16 extraOverheadUs;
    UInt8 byteDurationUs;


    switch(phy)
    {
        case gpHal_BleTxPhy1Mb:
        {
            byteDurationUs = 8;
            // preamble + crc
            extraOverheadUs = 8 + 24;
            break;
        }
        case gpHal_BleTxPhy2Mb:
        {
            byteDurationUs = 4;
            // preamble + crc
            extraOverheadUs = 8 + 12;
            break;
        }
        // case gpHal_BleTxPhyCoded125kb:
        // {
        //     byteDurationUs = 64;
        //     // preamble + CI + TERM1 + CRC + TERM2
        //     extraOverheadUs = 80 + 16 + 24 + 192 + 24;
        //     break;
        // }
        // case gpHal_BleTxPhyCoded500kb:
        // {
        //     byteDurationUs = 16;
        //     // preamble + remaining from sync word + CI + TERM1 + CRC + TERM2
        //     // the sync word remainder comes from the fact that the sync word is present in the pbm (and incorporated by using pdLoh.length).
        //     // For all other phys, the sync word has the same PHY as the rest of the packet, but in this case, the sync word uses S=8 coding.
        //     // This means that we have 64 us (4bytes sync word * 16us per byte) added to the calculation, but the syncword duration is 256.
        //     // Therefor, we need to add an extra 192 us (256-64) to the overhead calculation
        //     extraOverheadUs = 80 + 192 + 16 + 24 + 48 + 6;
        //     break;
        // }
        default:
        {
            // Should not happen
            extraOverheadUs = 0;
            byteDurationUs = 0;
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }

    totalDurationUs = (pbmLength * byteDurationUs) + extraOverheadUs;


    packetIntervalUnit = (totalDurationUs + 249) / GPHAL_BLE_TIME_SLOT_DURATION_US;

    if((totalDurationUs + 249) % GPHAL_BLE_TIME_SLOT_DURATION_US != 0)
    {
        packetIntervalUnit++;
    }

    return (packetIntervalUnit * GPHAL_BLE_TIME_SLOT_DURATION_US);
}

void gpHal_BlePopulateValidationSettings(gpHal_phyMask_t phyMask)
{
    gpHal_Address_t infoAddress = (gpHal_Address_t)GP_HAL_BLE_TEST_INFO_START;

    if(phyMask.mask & GPHAL_BLE_PHY_MASK_1MB)
    {
        GP_WB_WRITE_BLE_TEST_INFO_VALIDATION_THRESH(infoAddress, GPHAL_BLE_TESTMODE_VALIDATION_THRESH_NDR);
        GP_WB_WRITE_BLE_TEST_INFO_VALIDATION_START_IDX(infoAddress, GPHAL_BLE_TESTMODE_VALIDATION_START_IDX_NDR);
        GP_WB_WRITE_BLE_TEST_INFO_FAKE_PREAMBLE_PRESENT(infoAddress, GPHAL_BLE_TESTMODE_VALIDATION_FAKE_PREAMBLE_PRESENT_NDR);
        GP_WB_WRITE_BLE_TEST_INFO_FAKE_PREAMBLE_START_IDX(infoAddress, GPHAL_BLE_TESTMODE_VALIDATION_FAKE_PREAMBLE_START_IDX_NDR);
    }
    else if (phyMask.mask & GPHAL_BLE_PHY_MASK_2MB)
    {
        // Use optimized settings for high data rate
        GP_WB_WRITE_BLE_TEST_INFO_VALIDATION_THRESH(infoAddress, GPHAL_BLE_TESTMODE_VALIDATION_THRESH_HDR);
        GP_WB_WRITE_BLE_TEST_INFO_VALIDATION_START_IDX(infoAddress, GPHAL_BLE_TESTMODE_VALIDATION_START_IDX_HDR);
        GP_WB_WRITE_BLE_TEST_INFO_FAKE_PREAMBLE_PRESENT(infoAddress, GPHAL_BLE_TESTMODE_VALIDATION_FAKE_PREAMBLE_PRESENT_HDR);
        GP_WB_WRITE_BLE_TEST_INFO_FAKE_PREAMBLE_START_IDX(infoAddress, GPHAL_BLE_TESTMODE_VALIDATION_FAKE_PREAMBLE_START_IDX_HDR);
    }
    else
    {
        // Currently no special settings needed for coded PHY
    }
}

gpHal_BleRxPhy_t rxPhyMaskToRxPhy(gpHal_phyMask_t mask)
{
    if (GPHAL_BLE_PHY_MASK_2MB == mask.mask)
    {
        return gpHal_BleRxPhy2Mb;
    }
    else if (GPHAL_BLE_PHY_MASK_1MB == mask.mask)
    {
        return gpHal_BleRxPhy1Mb;
    }
    else if (0 == mask.mask)
    {
        return gpHal_BleRxPhy1Mb;
    }
    else
    {
        GP_LOG_PRINTF("Can't convert rxPhyMask %u to phy", 0, mask.mask);
        GP_ASSERT_DEV_INT(false);
        return gpHal_BleRxPhy1Mb;
    }
}

gpHal_Result_t gpHal_BlePopulateTestInfo(gpHal_TestInfo_t* pInfo)
{
    gpHal_Address_t infoAddress = (gpHal_Address_t)GP_HAL_BLE_TEST_INFO_START;
    UInt8 preambleSymbol = GP_HAL_BLE_PREAMBLE_SYMBOL_ADV_CHANNEL_UNCODED;

    // if( pInfo->tx && ((pInfo->phy.txPhy == gpHal_BleTxPhyCoded125kb) || (pInfo->phy.txPhy == gpHal_BleTxPhyCoded500kb)) )
    // {
    //     preambleSymbol = GP_HAL_BLE_PREAMBLE_SYMBOL_ADV_CHANNEL_CODED;
    // }

    UInt8 rxPhyMask = pInfo->phy.rxPhyMask.mask;
    if (!pInfo->tx && rxPhyMask != GPHAL_BLE_PHY_MASK_1MB && rxPhyMask != GPHAL_BLE_PHY_MASK_2MB)
    {
        // Only 1Mb and 2Mb (high data rate) are available and not concurrently
        return gpHal_ResultInvalidParameter;
    }

    GP_WB_WRITE_BLE_TEST_INFO_PREAMBLE(infoAddress, preambleSymbol);
    GP_WB_WRITE_BLE_TEST_INFO_RX_NOT_TX(infoAddress, (pInfo->tx ? false: true));
    if (pInfo->tx)
    {
        GP_WB_WRITE_BLE_TEST_INFO_TX_PHY_MODE(infoAddress, pInfo->phy.txPhy);
    }
    else
    {
        // If supported by the RT then BLE_TEST_INFO_RX_ALLPHY_MASK will overrule this
        gpHal_BleRxPhy_t rxPhy = rxPhyMaskToRxPhy(pInfo->phy.rxPhyMask);
        GP_WB_WRITE_BLE_TEST_INFO_RX_PHY_MODE(infoAddress, rxPhy);
    }
    GP_WB_WRITE_BLE_TEST_INFO_CHANNEL(infoAddress, pInfo->channel);
    GP_WB_WRITE_BLE_TEST_INFO_ANTENNA(infoAddress, pInfo->antenna);
    GP_WB_WRITE_BLE_TEST_INFO_WHITENING_ENABLE(infoAddress, 0);
    GP_WB_WRITE_BLE_TEST_INFO_ACCESS_ADDRESS(infoAddress, pInfo->accesscode);
    GP_WB_WRITE_BLE_TEST_INFO_CRC_INIT(infoAddress, GPHAL_BLE_DIRECT_TEST_MODE_CRC);
    GP_WB_WRITE_BLE_TEST_INFO_WHITENING_INIT_REV(infoAddress, 0);
    GP_WB_WRITE_BLE_TEST_INFO_PREAMBLE_THRESH(infoAddress, GPHAL_BLE_PREAMBLE_THRESHOLD_DEFAULT);

    if(pInfo->tx)
    {
        UInt8 pbmHandle;

        pbmHandle = gpHal_BlePdToPbm(pInfo->pdLoh, false);

        if(!GP_HAL_CHECK_PBM_VALID(pbmHandle))
        {
            return gpHal_ResultInvalidParameter;
        }

        GP_WB_WRITE_BLE_TEST_INFO_TX_PACKET_INTERVAL(infoAddress, gpHalBle_GetPacketInterval(pbmHandle, pInfo->pdLoh.length, pInfo->phy.txPhy));
        GP_WB_WRITE_BLE_TEST_INFO_TX_PACKET_COUNT(infoAddress, pInfo->txPacketCount);
        GP_WB_WRITE_BLE_TEST_INFO_TX_PBM(infoAddress, pbmHandle);
    }
    else
    {
        GP_WB_WRITE_BLE_TEST_INFO_TX_PBM(infoAddress, GP_PD_INVALID_HANDLE);
        GP_WB_WRITE_BLE_TEST_INFO_RX_PACKET_COUNT(infoAddress, 0);
        GP_WB_WRITE_BLE_TEST_INFO_FORWARD_RX_PDUS(infoAddress, pInfo->forwardRxPdus);

        GP_WB_WRITE_BLE_TEST_INFO_RX_ALLPHY_MASK(infoAddress, 0); //pInfo->phy.rxPhyMask.mask);
        // An RX test also requires that validation settings are applied
        gpHal_BlePopulateValidationSettings(pInfo->phy.rxPhyMask);
    }


    return gpHal_ResultSuccess;
}

#ifndef GP_COMP_CHIPEMU
void gpHal_BleDsFixStartTestModeHook(UInt8 phyChannel)
{
    UInt8 bleChannel = GP_HAL_CONVERT_PHY_TO_BLE_CHANNEL(phyChannel);
    gpHal_DSFix_ChannelListPtr =  GP_WB_READ_BLE_MGR_DSFIX_CHANNEL_LIST_PTR();
    // make sure RT system is bypassed
    GP_WB_WRITE_BLE_MGR_DSFIX_CHANNEL_LIST_PTR(0x0);

    gpHalRadioMgmtSynch_claimRadio();

    gpHal_BleTestModeDsFixContext.xoLdoRefbits = GP_WB_READ_PMUD_XO_LDO_REFBITS();
    gpHal_BleTestModeDsFixContext.bpfLdoDigRefbits = GP_WB_READ_RX_RX_BPF_LDO_DIG_REFBITS();

    rap_dsfix_cal_desense_ch(&bleChannel);

    gpHalRadioMgmtSynch_releaseRadio();
}

void gpHal_BleDsFixEndTestModeHook(void)
{
        // write back RT DSFix pointer
    GP_WB_WRITE_BLE_MGR_DSFIX_CHANNEL_LIST_PTR(gpHal_DSFix_ChannelListPtr);

    UInt8 LOCAL_BACKUP(PMUD_XO_LDO_RDY_OVERRIDE);
    UInt8 LOCAL_BACKUP(PMUD_CLK_32M_RDY_OVERRIDE);
    GP_WB_WRITE_PMUD_XO_LDO_RDY_OVERRIDE(1);
    GP_WB_WRITE_PMUD_CLK_32M_RDY_OVERRIDE(1);

    GP_WB_WRITE_PMUD_XO_LDO_REFBITS(gpHal_BleTestModeDsFixContext.xoLdoRefbits);
    GP_WB_WRITE_RX_RX_BPF_LDO_DIG_REFBITS(gpHal_BleTestModeDsFixContext.bpfLdoDigRefbits);

    HAL_WAIT_US(100); // wait until parameters are settled
    LOCAL_RESTORE(PMUD_XO_LDO_RDY_OVERRIDE);
    LOCAL_RESTORE(PMUD_CLK_32M_RDY_OVERRIDE);
}
#endif //GP_COMP_CHIPEMU

/*****************************************************************************
*                    Public Function Definitions
*****************************************************************************/

void gpHal_BleTestModeInit(void)
{
    gpHal_BleTestModeActive = false;

    // RT can forward testmode PDUs to NRT and uses the scan_req_rx interrupt for this.
    GP_WB_WRITE_INT_CTRL_MASK_IPCGPM2X_SCAN_REQ_RX_INTERRUPT(1);
}

gpHal_Result_t gpHal_BleStartTestMode(gpHal_TestInfo_t* pInfo)
{
    gpHal_Result_t result;
    UInt16 testModeArgs;

    if(gpHal_BleTestModeActive)
    {
        // Should not happen (blocked in higher layer)
        GP_ASSERT_DEV_INT(false);
        return gpHal_ResultBusy;
    }

    if(pInfo == NULL)
    {
        return gpHal_ResultInvalidParameter;
    }

    result = gpHal_BlePopulateTestInfo(pInfo);

    if(result != gpHal_ResultSuccess)
    {
        return result;
    }

#ifndef GP_COMP_CHIPEMU
    if(!pInfo->tx)
    {
        // For rx tests, we need to perform some exta steps to improve the sensitivity on desense channels
        gpHal_BleDsFixStartTestModeHook(pInfo->channel);
    }
#endif //GP_COMP_CHIPEMU

    // Convert to address that RT understands
    testModeArgs = (UInt16) (GP_HAL_BLE_TEST_INFO_TO_OFFSET_FROM_START() & 0xFFFF);

    result = gpHal_IpcTriggerCommand(BLE_MGR_START_DIRECT_TEST_MODE, sizeof(testModeArgs), (UInt8*)&testModeArgs);

    if(result == gpHal_ResultSuccess)
    {
        gpHal_BleTestModeActive = true;
#ifdef GP_COMP_TXMONITOR
        gpTxMonitor_AnnounceTxStart();
#endif //GP_COMP_TXMONITOR
    }

    return result;
}

gpHal_Result_t gpHal_BleTestMode_EnableSampleCollection(void)
{
    gpHal_Address_t infoAddress;

    infoAddress = (gpHal_Address_t) GP_HAL_BLE_TEST_INFO_START;

    if(GP_WB_READ_BLE_TEST_INFO_RAW_PHASE_MEASUREMENT_ARMED(infoAddress))
    {
        // Already armed, nothing to do
        return gpHal_ResultBusy;
    }

    // Arm again, allow new measurements to be taken
    GP_WB_WRITE_BLE_TEST_INFO_RAW_PHASE_MEASUREMENT_ARMED(infoAddress, 1);

    return gpHal_ResultSuccess;
}

gpHal_Result_t gpHal_BleEndTestMode(gpHal_BleTestModeEnd_t* pInfo)
{
    gpHal_Address_t address;
    gpHal_Result_t result;
    UInt8 pbmEntry;

    address = (gpHal_Address_t) GP_HAL_BLE_TEST_INFO_START;

    result = gpHal_IpcTriggerCommand(BLE_MGR_STOP_DIRECT_TEST_MODE, 0, NULL);

    if(result == gpHal_ResultSuccess)
    {
        gpHal_BleTestModeActive = false;

        if(!GP_WB_READ_BLE_TEST_INFO_RX_NOT_TX(address))
        {
            #ifdef GP_COMP_TXMONITOR
            gpTxMonitor_AnnounceTxFinished();
            #endif //GP_COMP_TXMONITOR
        }
    }

    if(GP_WB_READ_BLE_TEST_INFO_RX_NOT_TX(address))
    {
#ifndef GP_COMP_CHIPEMU
        // Make sure to restore desense related fixes to state from before the receiver test was started
        gpHal_BleDsFixEndTestModeHook();
#endif //GP_COMP_CHIPEMU
        pInfo->nrOfPackets = GP_WB_READ_BLE_TEST_INFO_RX_PACKET_COUNT(address);
    }
    else
    {
        pInfo->nrOfPackets = GP_WB_READ_BLE_TEST_INFO_TX_PACKET_COUNT(address);
    }

    pbmEntry = GP_WB_READ_BLE_TEST_INFO_TX_PBM(address);

    if(GP_HAL_IS_PBM_ALLOCATED(pbmEntry))
    {
        gpPd_Loh_t pdLoh;

        gpHal_BleConfPbmToPd(pbmEntry, &pdLoh);
        pInfo->pdHandle = pdLoh.handle;
    }
    else
    {
        pInfo->pdHandle = GP_PD_INVALID_HANDLE;
    }

    return result;
}

Bool gpHal_BleTestModeIsActive(void)
{
    return gpHal_BleTestModeActive;
}

gpHal_Result_t gpHal_BleTestModeGetPhaseSamplesBuffer(UInt32* pSamples)
{
    gpHal_Address_t infoAddress;
    UInt32 samplesPtr;

    if(!gpHal_BleTestModeIsActive())
    {
        return gpHal_ResultInvalidRequest;
    }

    infoAddress = (gpHal_Address_t) GP_HAL_BLE_TEST_INFO_START;

    if(GP_WB_READ_BLE_TEST_INFO_RAW_PHASE_MEASUREMENT_ARMED(infoAddress))
    {
        // Armed, buffer can still be used by HW
        return gpHal_ResultBusy;
    }

    samplesPtr = GP_WB_READ_BLE_TEST_INFO_RAW_PHASE_SAMPLE_PTR(infoAddress);
    *pSamples = (UInt32)(GP_MM_RAM_ADDR_FROM_COMPRESSED(samplesPtr));

    return gpHal_ResultSuccess;
}
