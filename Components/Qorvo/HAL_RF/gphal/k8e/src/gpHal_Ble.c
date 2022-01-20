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
 * $Header: //depot/release/Embedded/Components/Qorvo/HAL_RF/v2.10.2.1/comps/gphal/k8e/src/gpHal_Ble.c#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

//#define GP_LOCAL_LOG

#include "gpSched.h"
#include "gpPd.h"
#include "gpHal.h"
#include "gpHal_DEFS.h"
#include "gpHal_Ble_DEFS.h"
#include "gpHal_kx_Phy.h"

#ifdef GP_COMP_TXMONITOR
#include "gpTxMonitor.h"
#endif //GP_COMP_TXMONITOR



/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_GPHAL

#define GPHAL_BLE_TX_QUEUE_SIZE       8
#define GPHAL_BLE_MAX_NR_OF_QUEUED_PBMS     (GPHAL_BLE_TX_QUEUE_SIZE - 1)
#define GPHAL_BLE_QUEUE_WRITE_POINTER_WRAP  GPHAL_BLE_MAX_NR_OF_QUEUED_PBMS

// Offsets from start of packet where whitening and crc should start
#define GPHAL_BLE_WHITENING_START_OFFSET        5
#define GPHAL_BLE_CRC_START_OFFSET              5

#define GPHAL_BLE_CONNECTION_METRICS_UPDATE_TIMEOUT_US  100000

// Guard time related timings
#define T_SAFETY_MARGIN         (100 /* unit us */)
#define T_PROCESSING            (GP_WB_READ_BLE_MGR_EVENT_PROCESSING_DELAY() + T_SAFETY_MARGIN)


#define T_CAL_DELAY              ((GP_WB_READ_RIB_CAL_DELAY()+1) * 16)
#define T_OFF2TX_DELAY           32 //((GP_WB_READ_RIB_OFF2TX_DELAY()+1) * 16)
#define T_IFS                    150 //us
// Max lengths
#define LEN_MAX_ADV_IND                 (1 /*preamble*/ + 4 /*addr*/ + 2 /*header*/ +  37 /* max payload */      + 3 /*CRC*/)
#define LEN_SCAN_REQ                    (1 /*preamble*/ + 4 /*addr*/ + 2 /*header*/ +  12 /* fixed payload */    + 3 /*CRC*/)
#define LEN_MAX_SCAN_RSP                (1 /*preamble*/ + 4 /*addr*/ + 2 /*header*/ +  37 /* max payload */      + 3 /*CRC*/)
#define LEN_CON_REQ                     (1 /*preamble*/ + 4 /*addr*/ + 2 /*header*/ +  34 /* fixed payload */    + 3 /*CRC*/)
#define LEN_MIN_DATA                    (1 /*preamble*/ + 4 /*addr*/ + 2 /*header*/ +   0 /* data */             + 3 /*CRC*/)
#define GPHAL_BLE_MAX_PBM_DATA_LENGTH_SPEC   (1 /*preamble*/ + 4 /*addr*/ + 2 /*header*/ + 1 /* cteInfo */ + 251 /* data */ + 4 /*MIC*/ + 3 /*CRC*/)

/* Check if PBM is smaller than max BLE packet size */
#if GP_HAL_PBM_MAX_SIZE < GPHAL_BLE_MAX_PBM_DATA_LENGTH_SPEC
#define LEN_MAX_DATA             GP_HAL_PBM_MAX_SIZE
#else
#define LEN_MAX_DATA             GPHAL_BLE_MAX_PBM_DATA_LENGTH_SPEC
#endif

/* Convert length to tx/rx speed */
#define T_FROM_LEN_AND_BW(len_B, bw_Mbs)    ( ((UInt16)len_B * 8) / (bw_Mbs==gpHal_BleTxPhy2Mb? 2 : 1) )
#define T_FROM_LEN(len_B)                   T_FROM_LEN_AND_BW(len_B,gpHal_BleTxPhy1Mb)

/* max 1568us with 1120 for rx/tx */
#define T_ADV_GUARD_TIME(len_adv, len_scan_rsp) ((T_CAL_DELAY) + (T_OFF2TX_DELAY) + (T_FROM_LEN(len_adv)) + (T_IFS) + (T_FROM_LEN(LEN_CON_REQ)) + (T_IFS) + (T_FROM_LEN(len_scan_rsp)) + T_PROCESSING)
/* max 960us with 560 for rx/tx */
#define T_SCAN_GUARD_TIME                       ((T_IFS) + (T_FROM_LEN(LEN_SCAN_REQ)) + (T_IFS) + (T_FROM_LEN(LEN_MAX_SCAN_RSP)) + T_PROCESSING)
/* max 986us with 736 for rx/tx */
#define T_INIT_GUARD_TIME                       ((T_FROM_LEN(LEN_MAX_ADV_IND)) + (T_IFS) + (T_FROM_LEN(LEN_CON_REQ)) + T_PROCESSING)

#define T_BLE_TRANSACTION(len_data_tx, tx_phy, rx_phy)      ((T_IFS) + (T_FROM_LEN_AND_BW(len_data_tx,tx_phy)) + (T_IFS) + (T_FROM_LEN_AND_BW(LEN_MAX_DATA,rx_phy)))

#ifdef GP_COMP_GPHAL_MAC
/* We want to make sure a 15.4 TX event can't cause a BLE event to be skipped! :
 * we can send a cleanup trigger to the lower priority 15.4 TX event at least 4992us
 * before the BLE event starts since (CCA + 15.4 packet + ack) takes at most 4992us
 * and the event can only stop after this transanction.
 * Note that the RT subsystem automatically adds the processing delay */
extern UInt32 gpHal_MacGetMaxTransferTime(void);
//(5000 /* > 4992  == 192 + (6 + 127) * 32 + 12 * 16 + 11 * 32 */)
#define GP_HAL_T_CLEANUP_MARGIN     8
#define T_CLEANUP                                           max((gpHal_MacGetMaxTransferTime()+ GP_HAL_T_CLEANUP_MARGIN),T_BLE_TRANSACTION(LEN_MAX_DATA, gpHal_BleTxPhy1Mb, gpHal_BleTxPhy1Mb))
#else
/* If no 15.4 TX we only need to prevent a max length BLE transaction of another connection
 * from blocking us (Same as maximum guard time). */
/* TCleanup: max time for last transaction (MtoS + Tifs + StoM + Tifs) on a lower priority activity
 * when the lower priority is about to be interrupted by a higher priority activity
 * It is required that this timer needs to be configured according the slowest possible modulation
 * Note that the RT subsystem automatically adds the processing delay */
#define T_CLEANUP                                           T_BLE_TRANSACTION(LEN_MAX_DATA, gpHal_BleTxPhy1Mb, gpHal_BleTxPhy1Mb)
#endif //GP_COMP_GPHAL_MAC

/* Max 4640us with 4240 for rx/tx: since exchange time needed for smallest data limit (27B) is 992us
 * and the minimum window is 7500(6*1250), worst case we might only exchange 3 packets where we could
 * otherwise do 7. i.e. max 2.3x bandwith drop. Actually <2.0x because len_data_tx is taken into account! */
#define T_CONN_GUARD_TIME(len_data_tx, tx_phy, rx_phy)      (T_BLE_TRANSACTION(len_data_tx, tx_phy, rx_phy) + T_PROCESSING)

#define GP_HAL_BLE_PDL_HEAD_INDEX_INVALID               (255)

#define AUX_SCAN_REQ_TYPE       3

// Size of desense fix scratch array (should be at least 5*14 + 4*6 + 4*4 ==> 110 bytes)
#define GP_HAL_BLE_DSFIX_SCRATCH_ARRAY_SIZE             110


#define GP_HAL_BLE_FIXED_WD_DURATION_LIMIT_US                   100000

#ifndef GP_HAL_BLE_MAX_DEFAULT_TX_POWER
/* Constrained to 19 by default to pass BLE Phy certification tests which have an uncertainty of 0.98dB and must measure a value below 20dBm. */
#define GP_HAL_BLE_MAX_DEFAULT_TX_POWER 19
#endif

// Background scanning has the lowest priority of all BLE events. The lower the value, the lower the priority
#define GP_HAL_BLE_BGSC_EVENT_PRIORITY                              0

#define GP_HAL_BLE_ADV_ABORT_TIMEOUT_US                             1000

/*****************************************************************************
 *                    Checks
 *****************************************************************************/

//BLE channel index fixed to 3
GP_COMPILE_TIME_VERIFY(GPHAL_BLE_CHANNEL_IDX == 3);

/*****************************************************************************
 *                   Functional Macro Definitions
 *****************************************************************************/

#define GP_HAL_BLE_QUEUE_CIRC_INC(ptr)  ((ptr == GPHAL_BLE_QUEUE_WRITE_POINTER_WRAP) ? 0 : (ptr + 1))

#ifndef GP_HAL_DIVERSITY_SINGLE_ANTENNA
/* Threshold when an antenna swicth is needed. Initialized to ~ 3.51 %, see SW-4627 */
#define GP_HAL_BLE_ANT_SW_THRESHOLD                 0x0480
// RT expects log2(IIR_value). This corresponds to 64
#define GP_HAL_BLE_ANT_SW_IIR_FACTOR                6
#define GP_HAL_BLE_ANT_SW_NR_BACKOFF                6
#define GP_HAL_BLE_ANT_SW_USE_CRC                   1
#define GP_HAL_BLE_ANT_SW_USE_NACK                  1
#define GP_HAL_BLE_ANT_SW_USE_MISSED_FRAME          1
#endif // GP_HAL_DIVERSITY_SINGLE_ANTENNA

#define GP_HAL_BLE_GET_COMBINED_SCA_COMPENSATION(combinedSca)     ((combinedSca) * 105 / 100)
#define GP_HAL_BLE_GET_COMBINED_SCA_NO_COMPENSATION(combinedSca)  ((combinedSca) * 100 / 105)

#if defined(GP_HAL_EXPECTED_CHIP_EMULATED)
#define GP_HAL_IS_EMULATED() BIT_TST(gpHal_GetHWVersionId(), 11)
#endif //GP_HAL_EXPECTED_CHIP_EMULATED

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct {
    Bool allocated;
} gpHal_BleChannelMapMapping_t;


#define gpHal_BlePhyMode1Mb             GP_WB_ENUM_BLE_PHY_MODE_BLE
#define gpHal_BlePhyMode2Mb             GP_WB_ENUM_BLE_PHY_MODE_BLE_HDR

typedef UInt8 gpHal_BlePhyMode_t;

/******************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

#define LINKER_SECTION_LOWER_RAM_RETAIN_GPMICRO_ACCESSIBLE LINKER_SECTION(".lower_ram_retain_gpmicro_accessible")

/* compile time verification of info structures */
GP_COMPILE_TIME_VERIFY((GPHAL_BLE_SERVICE_EVENT_INFO_SIZE % GP_WB_MAX_MEMBER_SIZE) == 0);
GP_COMPILE_TIME_VERIFY(GPHAL_BLE_SERVICE_EVENT_INFO_SIZE >= GP_WB_SCAN_EV_INFO_SIZE);
GP_COMPILE_TIME_VERIFY(GPHAL_BLE_SERVICE_EVENT_INFO_SIZE >= GP_WB_INIT_EV_INFO_SIZE);
GP_COMPILE_TIME_VERIFY(GPHAL_BLE_SERVICE_EVENT_INFO_SIZE >= GP_WB_ADV_EV_INFO_SIZE);
GP_COMPILE_TIME_VERIFY(GPHAL_BLE_SERVICE_EVENT_INFO_SIZE >= GP_WB_BGSC_EV_INFO_SIZE);
GP_COMPILE_TIME_VERIFY((GPHAL_BLE_CONN_EVENT_INFO_SIZE % GP_WB_MAX_MEMBER_SIZE) == 0);
GP_COMPILE_TIME_VERIFY(GPHAL_BLE_CONN_EVENT_INFO_SIZE >= GP_WB_CONN_EV_INFO_SIZE);
GP_COMPILE_TIME_VERIFY((GPHAL_BLE_CHAN_MAP_INFO_SIZE % GP_WB_MAX_MEMBER_SIZE) == 0);
GP_COMPILE_TIME_VERIFY(GPHAL_BLE_CHAN_MAP_INFO_SIZE >= GP_WB_CONN_CH_MAP_SIZE);

// Placeholder to allocate space to store eventInfo in lower ram part
// Must be aligned to 8 bytes since aligned UInt64 reads/writes (LDRD) can be done on, for example, CONN_EV_INFO_TX_QUEUE
COMPILER_ALIGNED(GP_WB_MAX_MEMBER_SIZE) UInt8 gpHal_Ble_EventInfoMemory[GPHAL_BLE_NR_OF_SERVICE_EVENTS*GPHAL_BLE_SERVICE_EVENT_INFO_SIZE \
                                                                        + GPHAL_BLE_MAX_NR_OF_SUPPORTED_CONNECTIONS*GPHAL_BLE_CONN_EVENT_INFO_SIZE \
                                                                        + GPHAL_BLE_MAX_NR_OF_SUPPORTED_CHANNEL_MAPS*GPHAL_BLE_CHAN_MAP_INFO_SIZE \
                                                                       ] LINKER_SECTION_LOWER_RAM_RETAIN_GPMICRO_ACCESSIBLE;

COMPILER_ALIGNED(GP_WB_MAX_MEMBER_SIZE) UInt8 gpHal_Ble_ClosedLoop[GP_WB_TX_POWER_CONFIG_SIZE] LINKER_SECTION_LOWER_RAM_RETAIN_GPMICRO_ACCESSIBLE;

#ifndef GP_COMP_CHIPEMU
GP_COMPILE_TIME_VERIFY(GP_HAL_BLE_EXT_INIT_INFO_SIZE >= GP_WB_INIT_EXT_INFO_SIZE);

COMPILER_ALIGNED(GP_WB_MAX_MEMBER_SIZE) UInt8 gpHal_Ble_ExtInitEventInfoMemory[GP_HAL_BLE_EXT_INIT_INFO_SIZE] LINKER_SECTION_LOWER_RAM_RETAIN_GPMICRO_ACCESSIBLE;
#define GP_HAL_BLE_EXT_INIT_INFO_START                      ((UIntPtr)&gpHal_Ble_ExtInitEventInfoMemory[0])
#else
extern  UInt32 gpChipEmu_GetGpMicroStructExtInitInfoStart(UInt32 gp_mm_ram_linear_start);
#define GP_HAL_BLE_EXT_INIT_INFO_START                      gpChipEmu_GetGpMicroStructExtInitInfoStart(GP_MM_RAM_LINEAR_START)
#endif // GP_COMP_CHIPEMU

#ifndef GP_COMP_CHIPEMU
GP_COMPILE_TIME_VERIFY(GP_HAL_BLE_EXT_SCAN_INFO_SIZE >= GP_WB_SCAN_EXT_INFO_SIZE);

COMPILER_ALIGNED(GP_WB_MAX_MEMBER_SIZE) UInt8 gpHal_Ble_ExtScanInfoMemory[GP_HAL_BLE_EXT_SCAN_INFO_SIZE] LINKER_SECTION_LOWER_RAM_RETAIN_GPMICRO_ACCESSIBLE;
#define GP_HAL_BLE_EXT_SCAN_INFO_START               ((UIntPtr)&gpHal_Ble_ExtScanInfoMemory[0])
#else
extern  UInt32 gpChipEmu_GetGpMicroStructExtScanInfoStart(UInt32 gp_mm_ram_linear_start);
#define GP_HAL_BLE_EXT_SCAN_INFO_START               gpChipEmu_GetGpMicroStructExtScanInfoStart(GP_MM_RAM_LINEAR_START)
#endif // GP_COMP_CHIPEMU

// context for callbacks
static gpHal_BleCallbacks_t gpHal_BleCallbacks;

// Context for services
gpHal_BleServiceContext_t gpHal_BleServiceEventContext[GPHAL_BLE_NR_OF_SERVICE_EVENTS];

// Channel mapping
static gpHal_BleChannelMapMapping_t gpHal_BleChannelMapMapping[GPHAL_BLE_MAX_NR_OF_SUPPORTED_CHANNEL_MAPS];



static Int8 gpHal_BleTxPower; /* txPower at chip port */
static Int8 TxPathGaindBm = 0;
static Int8 RxPathGaindBm = 0;


COMPILER_ALIGNED(4) UInt8 gpHal_BleDsFixRtScratchArray[GP_HAL_BLE_DSFIX_SCRATCH_ARRAY_SIZE] LINKER_SECTION_LOWER_RAM_RETAIN_GPMICRO_ACCESSIBLE;


#ifndef GP_COMP_CHIPEMU
#endif //GP_COMP_CHIPEMU


/******************************************************************************
 *                    External Data Definitions
 *****************************************************************************/


/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

// Init
static void gpHal_BleGetChipDeviceAddress(BtDeviceAddress_t* pAddress);

// Communication over IPC (with BLE ev manager)
static gpHal_Result_t gpHal_BleStartEvent(UInt32 timeStamp, UInt8 type, UInt16 customData, UInt8 eventNr);
static gpHal_Result_t gpHal_BleRestartEvent(UInt32 timeStamp, UInt8 type, UInt16 customData, UInt8 eventNr);
static void gpHal_BleStartVirtualEvent(UInt32 timeStamp, UInt32 interval, gpHal_AbsoluteEventId_t eventNr);
static void gpHal_BleStopVirtualEvent(gpHal_AbsoluteEventId_t eventNr);
static void gpHal_BleWakeupEvent(UInt8 eventNr);
static gpHal_Result_t gpHal_BleStopEvent(UInt8 eventId);

// Info structure population
static gpHal_Result_t gpHal_BlePopulateAdvEventInfo(gpHal_AdvEventInfo_t* pInfo);
static gpHal_Result_t gpHal_BlePopulateScanEventInfo(gpHal_ScanEventInfo_t* pInfo);
static gpHal_Result_t gpHal_BlePopulateInitEventInfo(gpHal_InitEventInfo_t* pInfo);
static void gpHal_BlePopulateBgscEventInfo(gpHal_Address_t bgscInfoAddress, gpHal_BleServiceEventId_t serviceEvent, UInt8 service_flags, gpHal_phyMask_t phyMask);
static gpHal_Result_t gpHal_StartBgscEvent(void);
static gpHal_Result_t gpHal_RestartBgscEvent(void);
static gpHal_Result_t gpHal_StopBgscEvent(void);
static gpHal_Result_t gpHal_StartBleBgscService(gpHal_BleServiceEventId_t serviceEvent, gpHal_phyMask_t phyMask);
static gpHal_Result_t gpHal_StopBleBgscService(gpHal_BleServiceEventId_t serviceEvent);
static void gpHal_BlePopulateConnEventInfo(gpHal_ConnEventInfo_t* pInfo, gpHal_BleConnectionContext_t* pMapping, UInt32 firstConnEvtTs);
static void gpHal_BlePopulateUpdateConnectionInfo(gpHal_UpdateConnEventInfo_t* pInfo, gpHal_BleConnectionContext_t* pMapping);
static void gpHal_BlePopulateValidationParameters(gpHal_BleConnectionContext_t* pMapping, gpHal_BleValidationParameters_t* pValidation);
static void gpHal_BleUpdatePbmTxPower( gpHal_Address_t optsBase, gpHal_TxPower_t txPower );

// Service helper functions
static void gpHal_BlePurgeAllPendingPdus(gpHal_BleConnectionContext_t* pMapping);
static gpHal_Result_t gpHal_BleRestartService(gpHal_BleServiceEventId_t serviceEvent, UInt32 timeStamp, UInt8 type);

static gpHal_Result_t gpHal_InitBleEventManager(void);
static UInt32 gpHal_BleGetNextEventTs(gpHal_Address_t eventInfoAddress, UInt8 eType);

static void gpHal_BleConfigureEmptyPdu(void);
static void gpHal_BleOptimizeValidationParameters(void* pArg);
static gpHal_Result_t gpHalBle_PauseConnectionEvent(gpHal_BleConnectionContext_t *pMapping);

/* Phy helpers */
static gpHal_BleTxPhy_t gpHal_GetTxPhy(gpHal_Address_t eventInfoAddress);
static gpHal_BleRxPhy_t gpHal_GetRxPhy(gpHal_Address_t eventInfoAddress);
static void gpHal_SetTxPhy(gpHal_Address_t eventInfoAddress, gpHal_BleTxPhy_t tx_phy);
static void gpHal_SetRxPhy(gpHal_Address_t eventInfoAddress, gpHal_BleRxPhy_t rx_phy);


static void gpHal_BleUpdateCleanupTime(Bool eventRemoved);

static void gpHal_BleTriggerCbMasterCreateConn(UInt8 connRspPbm);


static gpHal_BleRxPhy_t gpHal_BleTxPhy2RxPhy(gpHal_BleTxPhy_t txPhy);
static UInt32 gpHal_BleRxTimestampCorrection(gpHal_BlePhyMode_t phy);

static void gpHal_BleUpdateSleepSettings(gpHal_Address_t connEvInfoAddress, UInt16 orignalLatency);

static UInt8 gpHal_Ble_GetScanAntennaSwitching(void);
static UInt8 gpHal_Ble_GetScanAntenna(void);

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/
void gpHal_BleGetChipDeviceAddress(BtDeviceAddress_t* pAddress)
{
    const BtDeviceAddress_t noAddress = {.addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    const BtDeviceAddress_t ffAddress = {.addr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
    UInt64 deviceAddress;

    // Read from user license
    GP_HAL_READ_BYTE_STREAM(GP_MM_FLASH_ALT_START, pAddress->addr, sizeof(BtDeviceAddress_t));

    if((MEMCMP(pAddress->addr, noAddress.addr, sizeof(BtDeviceAddress_t)) != 0) &&
       (MEMCMP(pAddress->addr, ffAddress.addr, sizeof(BtDeviceAddress_t)) != 0))
    {
        // BLE address in user page is set
        return;
    }

    // Read from user area in inf page
    deviceAddress = GP_WB_READ_NVR_USER_AREA_0();

    MEMCPY(pAddress->addr, &deviceAddress, sizeof(BtDeviceAddress_t));

    if((MEMCMP(pAddress->addr, noAddress.addr, sizeof(BtDeviceAddress_t)) != 0) &&
       (MEMCMP(pAddress->addr, ffAddress.addr, sizeof(BtDeviceAddress_t)) != 0))
    {
        // BLE address in user area of inf page is set
        return;
    }

    // Read from info page
    deviceAddress = GP_WB_READ_NVR_BLE_DEVICE_ADDRESS();

    MEMCPY(pAddress->addr, &deviceAddress, sizeof(BtDeviceAddress_t));
}

// Use this function when a new event is started
gpHal_Result_t gpHal_BleStartEvent(UInt32 timeStamp, UInt8 type, UInt16 customData, UInt8 eventNr)
{
    gpHal_Result_t result;

    // Update cleanup time before starting the event, to let RT use the updated time for the new occurence
    gpHal_BleUpdateCleanupTime(false);

    result = gpHal_BleRestartEvent(timeStamp, type, customData, eventNr);

    if(result != gpHal_ResultSuccess)
    {
        // Activity could not be added. Update cleanup time again
        gpHal_BleUpdateCleanupTime(true);
    }

    return result;
}

// Use this function when an already existing event is restarted
gpHal_Result_t gpHal_BleRestartEvent(UInt32 timeStamp, UInt8 type, UInt16 customData, UInt8 eventNr)
{
    gpHal_Result_t result;
    ble_mgr_start_event_args_t eventArgs;

    // Populate IPC arguments for BLE event manager
    eventArgs.event_nr = eventNr;
    eventArgs.event_type = type;
    eventArgs.info_ptr = customData;
    eventArgs.schedule_time = timeStamp;

    // Let the BLE ev mgr start and validate this event
    result = gpHal_IpcTriggerCommand(BLE_MGR_CMD_START_EVENT, sizeof(ble_mgr_start_event_args_t), (UInt8*)&eventArgs);

    return result;
}

void gpHal_BleStartVirtualEvent(UInt32 timeStamp, UInt32 interval, gpHal_AbsoluteEventId_t eventNr)
{
    gpHal_AbsoluteEventDescriptor_t eventDescriptor;

    eventDescriptor.exTime = timeStamp;
    eventDescriptor.recPeriod = interval;
    eventDescriptor.recAmount = 0xFFFF;
    eventDescriptor.customData = 0x00;
    eventDescriptor.executionOptions = GP_WB_EVENT_EXECUTE_IF_TOO_LATE_MASK;
    eventDescriptor.interruptOptions = 0x0;
    eventDescriptor.control = 0x0;

    GP_ES_SET_EVENT_RESULT(eventDescriptor.control, GP_WB_ENUM_EVENT_STATE_INVALID);
    GP_ES_SET_EVENT_STATE(eventDescriptor.control, gpHal_EventStateScheduled);
    eventDescriptor.type = GPHAL_ENUM_EVENT_TYPE_VIRTUAL;

    // Make sure no interrupts are pending before start
    GP_WB_WRITE_IPC_CLR_GPM2X_EVENT_PROCESSED_INTERRUPTS(1 << eventNr);

    // Start the virtual connection, should be done directly through ES
    gpHal_ScheduleAbsoluteEvent(&eventDescriptor, eventNr);
}

void gpHal_BleStopVirtualEvent(gpHal_AbsoluteEventId_t eventNr)
{
    gpHal_UnscheduleAbsoluteEvent(eventNr);
}

void gpHal_BleWakeupEvent(UInt8 eventNr)
{
    gpHal_Result_t result;

    result = gpHal_IpcTriggerCommand(BLE_MGR_CMD_WAKEUP_EVENT, 1, &eventNr);

    GP_ASSERT_DEV_INT(result == gpHal_ResultSuccess);
}

gpHal_Result_t gpHal_BleStopEvent(UInt8 eventId)
{
    gpHal_Result_t result;

    // trigger IPC command to BLE ev mgr
    result = gpHal_IpcTriggerCommand(BLE_MGR_CMD_STOP_EVENT, sizeof(eventId), &eventId);

    if(result == gpHal_ResultSuccess)
    {

        // Activity was successfully removed, update cleanup time.
        gpHal_BleUpdateCleanupTime(true);

        // If we stop an event, we should also clear any pending interrupt for this event
        GP_WB_WRITE_IPC_CLR_GPM2X_EVENT_PROCESSED_INTERRUPTS(1 << eventId);
        GP_WB_WRITE_IPC_CLR_GPM2X_LAST_CONN_EVENT_CNT_INTERRUPTS(1 << eventId);
    }

    return result;
}

UInt8 gpHal_Ble_GetScanAntennaSwitching(void)
{
    /* antenna switching does not mix with multistandard listening */
    if( (!(gpHal_GetRxAntennaDiversity())) || gpHal_BleGetMultiStandard() )
    {
        return GP_HAL_BLE_ANTENNASWITCHINGDISABLED;
    }
    else
    {
        return GP_HAL_BLE_ANTENNASWITCHINGENABLED;
    }
}

UInt8 gpHal_Ble_GetScanAntenna(void)
{

    if( (!(gpHal_GetRxAntennaDiversity())) || gpHal_BleGetMultiStandard() )
    {
        return (UInt8) gpHal_GetBleAntenna();
    }
    else
    {
        return gpHal_AntennaSelection_Ant0;
    }
}

gpHal_Result_t gpHal_BlePopulateAdvEventInfo(gpHal_AdvEventInfo_t* pInfo)
{
    UInt8 pbmHandle;
    gpHal_Address_t eventInfoAddress;
    UInt8 len_adv   = 0;
    UInt8 len_scan_rsp = 0;

    pbmHandle = gpHal_BlePdToPbm(pInfo->pdLohAdv, true);

    if(!GP_HAL_CHECK_PBM_VALID(pbmHandle))
    {
        return gpHal_ResultInvalidParameter;
    }

    len_adv = pInfo->pdLohAdv.length;

    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_SERVICE_TO_EVENT_INFO_ADDRESS(gpHal_BleServiceEventIdAdvertising);

    GP_WB_WRITE_ADV_EV_INFO_ADV_PBM(eventInfoAddress, pbmHandle);

    if(gpPd_CheckPdValid(pInfo->pdLohScan.handle) == gpPd_ResultValidHandle)
    {
        // We have a valid scan resp pd
        pbmHandle = gpHal_BlePdToPbm(pInfo->pdLohScan, false); // no Tx if too late for scan rsp

        if(!GP_HAL_CHECK_PBM_VALID(pbmHandle))
        {
            return gpHal_ResultInvalidParameter;
        }

        len_scan_rsp = pInfo->pdLohScan.length;

        GP_WB_WRITE_ADV_EV_INFO_SCAN_RSP_PBM(eventInfoAddress, pbmHandle);
    }
    else
    {
        // Make sure we always set invalid pbm in case none is specified
        GP_WB_WRITE_ADV_EV_INFO_SCAN_RSP_PBM(eventInfoAddress, GP_PBM_INVALID_HANDLE);
    }

    GP_WB_WRITE_ADV_EV_INFO_PRIORITY(eventInfoAddress, pInfo->priority);
    GP_WB_WRITE_ADV_EV_INFO_EXTENDED_PRIO_EN(eventInfoAddress, pInfo->enableExtPriority);
    GP_WB_WRITE_ADV_EV_INFO_INTERVAL(eventInfoAddress, pInfo->interval);
    GP_WB_WRITE_ADV_EV_INFO_INTRA_EV_TX_SPACING(eventInfoAddress, 0 /* intraEventTxSpacing can be minimized, since we have Tx if too late set for adv PDUs*/);
    GP_WB_WRITE_ADV_EV_INFO_ADV_DELAY_MASK(eventInfoAddress, pInfo->advDelayMax);
    GP_WB_WRITE_ADV_EV_INFO_ADV_CH0(eventInfoAddress, pInfo->channelMap[0]);
    GP_WB_WRITE_ADV_EV_INFO_ADV_CH1(eventInfoAddress, pInfo->channelMap[1]);
    GP_WB_WRITE_ADV_EV_INFO_ADV_CH2(eventInfoAddress, pInfo->channelMap[2]);
    GP_WB_WRITE_ADV_EV_INFO_FRAME_TYPE_ACCEPT_MASK(eventInfoAddress, pInfo->frameTypeAcceptMask);
    GP_WB_WRITE_ADV_EV_INFO_FT_WHITELIST_ENABLE_MASK(eventInfoAddress, pInfo->whitelistEnableMask);
    GP_ASSERT_DEV_INT(len_adv+3 <= LEN_MAX_ADV_IND);
    GP_ASSERT_DEV_INT(len_scan_rsp+3 <= LEN_MAX_SCAN_RSP);
    GP_WB_WRITE_ADV_EV_INFO_GUARD_TIME(eventInfoAddress, T_ADV_GUARD_TIME(len_adv, len_scan_rsp));
    GP_WB_WRITE_ADV_EV_INFO_EN_ANT_SWITCHING(eventInfoAddress, gpHal_Ble_GetAdvAntennaSwitching());
    GP_WB_WRITE_ADV_EV_INFO_CURR_ANTENNA(eventInfoAddress, gpHal_Ble_GetAdvAntenna());
    GP_WB_WRITE_ADV_EV_INFO_ACCEPT_UNRESOLVED_RPA_SRC(eventInfoAddress, 1);
    GP_WB_WRITE_ADV_EV_INFO_ACCEPT_UNRESOLVED_ID_SRC(eventInfoAddress, 1);

    GP_WB_WRITE_SCAN_EXT_INFO_PRI_PHY_MASK(GP_HAL_BLE_EXT_SCAN_INFO_START, GPHAL_BLE_PHY_MASK_1MB);

    return gpHal_ResultSuccess;
}

gpHal_Result_t gpHal_BlePopulateScanEventInfo(gpHal_ScanEventInfo_t* pInfo)
{
    UInt8 pbmHandle=0xFF;
    gpHal_Address_t eventInfoAddress;

    if(pInfo->activeScanning)
    {
        pbmHandle = gpHal_BlePdToPbm(pInfo->pdLoh, false);

        if(!GP_HAL_CHECK_PBM_VALID(pbmHandle))
        {
            return gpHal_ResultInvalidParameter;
        }
    }
    else
    {
        pbmHandle = GP_PBM_INVALID_HANDLE;
    }

    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_SERVICE_TO_EVENT_INFO_ADDRESS(gpHal_BleServiceEventIdScanning);

    // Write the event info
    GP_WB_WRITE_SCAN_EV_INFO_SCAN_REQ_PBM(eventInfoAddress, pbmHandle);
    GP_WB_WRITE_SCAN_EV_INFO_PRIORITY(eventInfoAddress, pInfo->priority);
    GP_WB_WRITE_SCAN_EV_INFO_EXTENDED_PRIO_EN(eventInfoAddress, pInfo->enableExtPriority);
    GP_WB_WRITE_SCAN_EV_INFO_INTERVAL(eventInfoAddress, pInfo->interval);
    GP_WB_WRITE_SCAN_EV_INFO_SCAN_WINDOW_DURATION(eventInfoAddress, pInfo->scanDuration);
    GP_WB_WRITE_SCAN_EV_INFO_CURRENT_CH_MAP_IDX(eventInfoAddress, pInfo->channelMapIndex);
    GP_WB_WRITE_SCAN_EV_INFO_SCAN_CH0(eventInfoAddress, pInfo->channelMap[0]);
    GP_WB_WRITE_SCAN_EV_INFO_SCAN_CH1(eventInfoAddress, pInfo->channelMap[1]);
    GP_WB_WRITE_SCAN_EV_INFO_SCAN_CH2(eventInfoAddress, pInfo->channelMap[2]);
    GP_WB_WRITE_SCAN_EV_INFO_FRAME_TYPE_ACCEPT_MASK(eventInfoAddress, pInfo->frameTypeAcceptMask);
    GP_WB_WRITE_SCAN_EV_INFO_FT_WHITELIST_ENABLE_MASK(eventInfoAddress, pInfo->whitelistEnableMask);

    GP_HAL_WRITE_BYTE_STREAM(eventInfoAddress + GP_WB_SCAN_EV_INFO_OWN_DEVICE_ADDRESS_ADDRESS, pInfo->ownAddress.addr, sizeof(BtDeviceAddress_t));
    GP_WB_WRITE_SCAN_EV_INFO_OWN_DEVICE_ADDRESS_TYPE(eventInfoAddress, pInfo->ownAddressType);
    GP_WB_WRITE_SCAN_EV_INFO_GENERATE_RES_PR(eventInfoAddress, pInfo->generateRpa);
    GP_WB_WRITE_SCAN_EV_INFO_ACTIVE_SCANNING(eventInfoAddress, pInfo->activeScanning);
    if(pInfo->activeScanning)
    {
        GP_WB_WRITE_SCAN_EV_INFO_BACKOFF_CNT(eventInfoAddress, pInfo->scanBackoffCount);
        GP_WB_WRITE_SCAN_EV_INFO_UPPER_LIMIT_MASK(eventInfoAddress, pInfo->upperLimitMask);
        GP_WB_WRITE_SCAN_EV_INFO_SUCCESS_CNT(eventInfoAddress, pInfo->successCount);
        GP_WB_WRITE_SCAN_EV_INFO_FAILURE_CNT(eventInfoAddress, pInfo->failureCount);
    }

    GP_WB_WRITE_SCAN_EV_INFO_ACCEPT_UNRESOLVED_RPA_DST(eventInfoAddress, pInfo->acceptUnresolvedRpaDst);
    GP_WB_WRITE_SCAN_EV_INFO_ACCEPT_UNRESOLVED_RPA_SRC(eventInfoAddress, 1);
    GP_WB_WRITE_SCAN_EV_INFO_ACCEPT_UNRESOLVED_ID_SRC(eventInfoAddress, 1);
    GP_WB_WRITE_SCAN_EV_INFO_GUARD_TIME(eventInfoAddress, T_SCAN_GUARD_TIME);

    GP_WB_WRITE_SCAN_EV_INFO_EN_ANT_SWITCHING(eventInfoAddress, gpHal_Ble_GetScanAntennaSwitching());
    GP_WB_WRITE_SCAN_EV_INFO_CURR_ANTENNA(eventInfoAddress, gpHal_Ble_GetScanAntenna());

    GP_WB_WRITE_SCAN_EXT_INFO_PRI_PHY_MASK(GP_HAL_BLE_EXT_SCAN_INFO_START, pInfo->phyMask.mask & 0xFD); // mask out 2Mbit

    return gpHal_ResultSuccess;
}

#ifdef GP_DIVERSITY_DEVELOPMENT
extern UInt8 Ble_GetVsdInitiatorVirtualConnId(UInt8 connId);
#endif /* GP_DIVERSITY_DEVELOPMENT */

gpHal_Result_t gpHal_BlePopulateInitEventInfo(gpHal_InitEventInfo_t* pInfo)
{
    UInt8 pbmHandle;
    gpHal_BleConnectionContext_t* pMapping;
    gpHal_Address_t eventInfoAddress;

    pbmHandle = gpHal_BlePdToPbm(pInfo->pdLohConn, false);

    if(!GP_HAL_CHECK_PBM_VALID(pbmHandle))
    {
        return gpHal_ResultInvalidParameter;
    }

    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_SERVICE_TO_EVENT_INFO_ADDRESS(gpHal_BleServiceEventIdInitiating);

#if defined(GP_DIVERSITY_DEVELOPMENT) && defined(GP_DIVERSITY_GPHAL_INTERN)
    pMapping = gpHal_BleGetConnMappingFromId(Ble_GetVsdInitiatorVirtualConnId(pInfo->virtualConnId));
#else
    pMapping = gpHal_BleGetConnMappingFromId(pInfo->virtualConnId);
#endif /* GP_DIVERSITY_DEVELOPMENT */

    if(pMapping == NULL)
    {
        // Provided conn id is not a valid one
        return gpHal_ResultInvalidParameter;
    }

    // Write the event info
    GP_WB_WRITE_INIT_EV_INFO_PRIORITY(eventInfoAddress, pInfo->priority);
    GP_WB_WRITE_INIT_EV_INFO_EXTENDED_PRIO_EN(eventInfoAddress, pInfo->enableExtPriority);
    GP_WB_WRITE_INIT_EV_INFO_INTERVAL(eventInfoAddress, pInfo->interval);
    GP_WB_WRITE_INIT_EV_INFO_INIT_WINDOW_DURATION(eventInfoAddress, pInfo->initWindowDuration);
    GP_WB_WRITE_INIT_EV_INFO_CONN_REQ_PBM(eventInfoAddress, pbmHandle);
    GP_WB_WRITE_INIT_EV_INFO_CURRENT_CH_MAP_IDX(eventInfoAddress, pInfo->channelMapIndex);
    GP_WB_WRITE_INIT_EV_INFO_INIT_CH0(eventInfoAddress, pInfo->channelMap[0]);
    GP_WB_WRITE_INIT_EV_INFO_INIT_CH1(eventInfoAddress, pInfo->channelMap[1]);
    GP_WB_WRITE_INIT_EV_INFO_INIT_CH2(eventInfoAddress, pInfo->channelMap[2]);
    GP_WB_WRITE_INIT_EV_INFO_FRAME_TYPE_ACCEPT_MASK(eventInfoAddress, pInfo->frameTypeAcceptMask);
    GP_WB_WRITE_INIT_EV_INFO_FT_WHITELIST_ENABLE_MASK(eventInfoAddress, pInfo->whitelistEnableMask);
    GP_HAL_WRITE_BYTE_STREAM(eventInfoAddress + GP_WB_INIT_EV_INFO_OWN_DEVICE_ADDRESS_ADDRESS, pInfo->ownAddress.addr, sizeof(BtDeviceAddress_t));
    GP_WB_WRITE_INIT_EV_INFO_OWN_DEVICE_ADDRESS_TYPE(eventInfoAddress, pInfo->ownAddressType);
    GP_WB_WRITE_INIT_EV_INFO_GENERATE_RES_PR(eventInfoAddress, pInfo->generateRpa);
    GP_WB_WRITE_INIT_EV_INFO_ACCEPT_UNRESOLVED_RPA_SRC(eventInfoAddress, 1);
    GP_WB_WRITE_INIT_EV_INFO_ACCEPT_UNRESOLVED_ID_SRC(eventInfoAddress, 1);

    // check connect request length (HW adds preamble and crc)
    GP_ASSERT_DEV_INT(LEN_CON_REQ == pInfo->pdLohConn.length + 3 + 1);

    GP_WB_WRITE_INIT_EV_INFO_GUARD_TIME(eventInfoAddress, T_INIT_GUARD_TIME);
    GP_WB_WRITE_INIT_EV_INFO_EN_ANT_SWITCHING(eventInfoAddress, gpHal_Ble_GetScanAntennaSwitching());
    GP_WB_WRITE_INIT_EV_INFO_CURR_ANTENNA(eventInfoAddress, gpHal_Ble_GetScanAntenna());

    if((pInfo->virtualConnId > GPHAL_BLE_MAX_NR_OF_SUPPORTED_CONNECTIONS) && !(pInfo->virtualConnId & GPHAL_BLE_VIRTUAL_CONN_MASK))
    {
        GP_WB_WRITE_INIT_EV_INFO_VIRT_EV_NR(eventInfoAddress, GPHAL_ES_ABSOLUTE_EVENT_ID_INVALID);
    }
    else
    {
        GP_WB_WRITE_INIT_EV_INFO_VIRT_EV_NR(eventInfoAddress, pMapping->eventNr);
    }
    GP_HAL_WRITE_BYTE_STREAM(eventInfoAddress + GP_WB_INIT_EV_INFO_CONN_DST_ADDRESS_ADDRESS, pInfo->connDstAddress.addr, sizeof(BtDeviceAddress_t));


    GP_WB_WRITE_INIT_EXT_INFO_PRI_PHY_MASK(GP_HAL_BLE_EXT_INIT_INFO_START, pInfo->initPhyMask.mask & 0xFD); // mask out 2Mbit

    return gpHal_ResultSuccess;
}

Bool gpHal_BleCompareEventParams(gpHal_BleServiceEventId_t serviceEvent, gpHal_phyMask_t mask_new)
{
    gpHal_Address_t event_info_addr = (gpHal_Address_t)GP_HAL_BLE_SERVICE_TO_EVENT_INFO_ADDRESS(serviceEvent);
    gpHal_Address_t bgsc_info_addr = (gpHal_Address_t)GP_HAL_BLE_SERVICE_TO_EVENT_INFO_ADDRESS(gpHal_BleServiceEventIdBgsc);

    UInt32 iv_new  = 0x00;
    UInt32 wd_new  = 0x00;
    UInt8 mask_old = 0x00;

    switch (serviceEvent)
    {
        case gpHal_BleServiceEventIdScanning:
            iv_new   = GP_WB_READ_SCAN_EV_INFO_INTERVAL(event_info_addr);
            wd_new   = GP_WB_READ_SCAN_EV_INFO_SCAN_WINDOW_DURATION(event_info_addr);
            mask_old = GP_WB_READ_SCAN_EXT_INFO_PRI_PHY_MASK(GP_HAL_BLE_EXT_SCAN_INFO_START);
            break;
        case gpHal_BleServiceEventIdInitiating:
            iv_new   = GP_WB_READ_INIT_EV_INFO_INTERVAL(event_info_addr);
            wd_new   = GP_WB_READ_INIT_EV_INFO_INIT_WINDOW_DURATION(event_info_addr);
            mask_old = GP_WB_READ_INIT_EXT_INFO_PRI_PHY_MASK(GP_HAL_BLE_EXT_INIT_INFO_START);
            break;
        default:
            GP_ASSERT_SYSTEM(false); /* invalid param */
    }

    UInt32 iv_old = GP_WB_READ_BGSC_EV_INFO_INTERVAL(bgsc_info_addr);
    UInt32 wd_old = GP_WB_READ_BGSC_EV_INFO_WINDOW_DURATION(bgsc_info_addr);

    return (iv_new > iv_old || wd_new > wd_old || mask_new.mask != mask_old);
}

void gpHal_BlePopulateIntervalAndWindow(UInt8 service_flags, UInt32* interval, UInt32* wd_duration)
{
    gpHal_Address_t init_info_addr = (gpHal_Address_t)GP_HAL_BLE_SERVICE_TO_EVENT_INFO_ADDRESS(gpHal_BleServiceEventIdInitiating);
    gpHal_Address_t scan_info_addr = (gpHal_Address_t)GP_HAL_BLE_SERVICE_TO_EVENT_INFO_ADDRESS(gpHal_BleServiceEventIdScanning);

    Bool init_enabled = GP_WB_GET_BGSC_EV_INFO_ENABLE_INIT_FROM_SCAN_FLAGS(service_flags);
    Bool scan_enabled = GP_WB_GET_BGSC_EV_INFO_ENABLE_SCAN_FROM_SCAN_FLAGS(service_flags);
    *interval = max(GP_WB_READ_INIT_EV_INFO_INTERVAL(init_info_addr) * init_enabled,
                    GP_WB_READ_SCAN_EV_INFO_INTERVAL(scan_info_addr) * scan_enabled);
    *wd_duration = max(GP_WB_READ_INIT_EV_INFO_INIT_WINDOW_DURATION(init_info_addr) * init_enabled,
                       GP_WB_READ_SCAN_EV_INFO_SCAN_WINDOW_DURATION(scan_info_addr) * scan_enabled);
}

void gpHal_BlePopulateBgscEventInfo(gpHal_Address_t bgscInfoAddress, gpHal_BleServiceEventId_t serviceEvent, UInt8 service_flags, gpHal_phyMask_t phyMask)
{
    gpHal_Address_t scanInfoAddress = (gpHal_Address_t)GP_HAL_BLE_SERVICE_TO_EVENT_INFO_ADDRESS(gpHal_BleServiceEventIdScanning);
    gpHal_Address_t initInfoAddress = (gpHal_Address_t)GP_HAL_BLE_SERVICE_TO_EVENT_INFO_ADDRESS(gpHal_BleServiceEventIdInitiating);

    GP_WB_WRITE_BGSC_EV_INFO_PRIORITY(bgscInfoAddress, GP_HAL_BLE_BGSC_EVENT_PRIORITY);
    GP_WB_WRITE_BGSC_EV_INFO_SUSPEND_EVENT(bgscInfoAddress,0);
    GP_WB_WRITE_BGSC_EV_INFO_EXTENDED_PRIO_EN(bgscInfoAddress, 0);
    GP_WB_WRITE_BGSC_EV_INFO_NR_CONSEC_SKIPPED_EVENTS(bgscInfoAddress,0);

    UInt32 interval=0;
    UInt32 wd_duration=0;
    UInt8 ch_map_idx=0;
    UInt8 ch0=0, ch1=0, ch2=0;
    UInt8 ant_switching = 0;
    UInt8 curr_antenna = 0;
    gpHal_BlePopulateIntervalAndWindow(service_flags, &interval, &wd_duration);
    switch (serviceEvent)
    {
        case gpHal_BleServiceEventIdScanning:
            ch_map_idx = GP_WB_READ_SCAN_EV_INFO_CURRENT_CH_MAP_IDX(scanInfoAddress);
            ch0 = GP_WB_READ_SCAN_EV_INFO_SCAN_CH0(scanInfoAddress);
            ch1 = GP_WB_READ_SCAN_EV_INFO_SCAN_CH1(scanInfoAddress);
            ch2 = GP_WB_READ_SCAN_EV_INFO_SCAN_CH2(scanInfoAddress);
            ant_switching = GP_WB_READ_SCAN_EV_INFO_EN_ANT_SWITCHING(scanInfoAddress);
            curr_antenna = GP_WB_READ_SCAN_EV_INFO_CURR_ANTENNA(scanInfoAddress);
            GP_WB_WRITE_SCAN_EXT_INFO_PRI_PHY_MASK(GP_HAL_BLE_EXT_SCAN_INFO_START, phyMask.mask & 0xFD); // mask out 2Mbit
            break;
        case gpHal_BleServiceEventIdInitiating:
            ch_map_idx = GP_WB_READ_INIT_EV_INFO_CURRENT_CH_MAP_IDX(initInfoAddress);
            ch0 = GP_WB_READ_INIT_EV_INFO_INIT_CH0(initInfoAddress);
            ch1 = GP_WB_READ_INIT_EV_INFO_INIT_CH1(initInfoAddress);
            ch2 = GP_WB_READ_INIT_EV_INFO_INIT_CH2(initInfoAddress);
            ant_switching = GP_WB_READ_INIT_EV_INFO_EN_ANT_SWITCHING(initInfoAddress);
            curr_antenna = GP_WB_READ_INIT_EV_INFO_CURR_ANTENNA(initInfoAddress);
            GP_WB_WRITE_INIT_EXT_INFO_PRI_PHY_MASK(GP_HAL_BLE_EXT_INIT_INFO_START, phyMask.mask & 0xFD); // mask out 2Mbit
            break;
        default:
            GP_ASSERT_SYSTEM(false); /* invalid param */
    }

    // Background scanning is not soft-aborted anymore, so we can use the processing delay as guard time
    GP_WB_WRITE_BGSC_EV_INFO_GUARD_TIME(bgscInfoAddress, GP_WB_READ_BLE_MGR_EVENT_PROCESSING_DELAY());
    GP_WB_WRITE_BGSC_EV_INFO_INTERVAL(bgscInfoAddress, interval);
    GP_WB_WRITE_BGSC_EV_INFO_WINDOW_DURATION(bgscInfoAddress, wd_duration);
    GP_WB_WRITE_BGSC_EV_INFO_CURRENT_CH_MAP_IDX(bgscInfoAddress, ch_map_idx);
    GP_WB_WRITE_BGSC_EV_INFO_SCAN_CH0(bgscInfoAddress, ch0);
    GP_WB_WRITE_BGSC_EV_INFO_SCAN_CH1(bgscInfoAddress, ch1);
    GP_WB_WRITE_BGSC_EV_INFO_SCAN_CH2(bgscInfoAddress, ch2);
    GP_WB_WRITE_BGSC_EV_INFO_SCAN_CH3(bgscInfoAddress, 0xFF);
    GP_WB_WRITE_BGSC_EV_INFO_SCAN_CH4(bgscInfoAddress, 0xFF);

    GP_WB_WRITE_BGSC_EV_INFO_EN_ANT_SWITCHING(bgscInfoAddress, ant_switching);
    GP_WB_WRITE_BGSC_EV_INFO_CURR_ANTENNA(bgscInfoAddress, curr_antenna);
}


void gpHal_BlePopulateConnEventInfo(gpHal_ConnEventInfo_t* pInfo, gpHal_BleConnectionContext_t* pMapping, UInt32 firstConnEvtTs)
{
#ifdef GP_DIVERSITY_GPHAL_INTERN
    UInt64 tx_queue = 0;
#else
    UInt64Struct_t tx_queue = {.LSB=0,.MSB=0};
#endif
    if(pInfo != NULL)
    {
        gpHal_Address_t eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId);
        UInt16 combinedSca;
        UInt16 combinedScaWorst;

        // Write the event info
        GP_WB_WRITE_CONN_EV_INFO_PRIORITY(eventInfoAddress, pInfo->priority);
        GP_WB_WRITE_CONN_EV_INFO_ACCESS_ADDRESS(eventInfoAddress, pInfo->accessAddress);
        GP_WB_WRITE_CONN_EV_INFO_EXTENDED_PRIO_EN(eventInfoAddress, pInfo->enableExtPriority);
        GP_WB_WRITE_CONN_EV_INFO_INTERVAL(eventInfoAddress, pInfo->interval);
        GP_WB_WRITE_CONN_EV_INFO_HOP_INCREMENT(eventInfoAddress, pInfo->hopIncrement);
        GP_WB_WRITE_CONN_EV_INFO_CH_MAP_PTR(eventInfoAddress, GP_HAL_BLE_CHAN_MAP_TO_OFFSET_FROM_START(pInfo->channelMapHandle));
        GP_WB_WRITE_CONN_EV_INFO_PREAMBLE_THRESH(eventInfoAddress, GPHAL_BLE_PREAMBLE_THRESHOLD_DEFAULT);
        GP_WB_WRITE_CONN_EV_INFO_CRC_INIT(eventInfoAddress, pInfo->crcInit);
        GP_WB_WRITE_CONN_EV_INFO_WINDOW_DURATION(eventInfoAddress, pInfo->windowDuration);

        combinedSca = pInfo->masterSca + gpHal_GetSleepClockAccuracy();

        if(pInfo->masterSca == 0)
        {
            combinedScaWorst = 0;
        }
        else
        {
            combinedScaWorst = pInfo->masterSca + gpHal_GetWorstSleepClockAccuracy();
        }

        // Compensating these values here is only done because it simplifies calculations in the RT subsystem
        GP_WB_WRITE_CONN_EV_INFO_COMBINED_SCA(eventInfoAddress, GP_HAL_BLE_GET_COMBINED_SCA_COMPENSATION(combinedSca));
        GP_WB_WRITE_CONN_EV_INFO_COMBINED_SCA_WORST(eventInfoAddress, GP_HAL_BLE_GET_COMBINED_SCA_COMPENSATION(combinedScaWorst));

        // CONN_EV_INFO_SLAVE_LATENCY_EN is controlled by RT subsystem (no need to set here)

        // Default behavior is to sleep during latency events (only used by RT for slave events)
        GP_WB_WRITE_CONN_EV_INFO_SLAVE_LATENCY_SLEEP_ENABLE(eventInfoAddress, 1);
        // Do not write latency yet (as it is not enabled in the early connection phase), will be done later by gpHal_BleUpdateSleepSettings
        GP_WB_WRITE_CONN_EV_INFO_SLAVE_LATENCY(eventInfoAddress, 0);
        GP_WB_WRITE_CONN_EV_INFO_EVENT_CNT(eventInfoAddress, pInfo->eventCounter);
        GP_WB_WRITE_CONN_EV_INFO_UNMAPPED_CH_PTR(eventInfoAddress, pInfo->unmappedChannelPtr);
        GP_WB_WRITE_CONN_EV_INFO_TX_SN(eventInfoAddress, pInfo->txSeqNr);
        GP_WB_WRITE_CONN_EV_INFO_TX_NESN(eventInfoAddress, pInfo->txNextSeqNr);
        GP_WB_WRITE_CONN_EV_INFO_RX_SN(eventInfoAddress, pInfo->rxSeqNr);
        GP_WB_WRITE_CONN_EV_INFO_RX_NESN(eventInfoAddress, pInfo->rxNextSeqNr);
        GP_WB_WRITE_CONN_EV_INFO_CURRENT_TX_PBM(eventInfoAddress, 0xFF);
        GP_WB_WRITE_CONN_EV_INFO_LAST_SCH_EVENT_CNT(eventInfoAddress, pInfo->lastConnEventCount);
        GP_WB_WRITE_CONN_EV_INFO_LAST_SCH_EVENT_CNT_VALID(eventInfoAddress, pInfo->lastConnEventCountValid);
        GP_WB_WRITE_CONN_EV_INFO_RX_FLOW_CTRL_FLAG(eventInfoAddress, pInfo->rxFlowCtrlFlag);
        GP_WB_WRITE_CONN_EV_INFO_TX_QUEUE_RD_PTR(eventInfoAddress, pInfo->txQueueReadPtr);
        GP_WB_WRITE_CONN_EV_INFO_TX_QUEUE_WR_PTR(eventInfoAddress, pInfo->txQueueWritePtr);
        GP_WB_WRITE_CONN_EV_INFO_TX_QUEUE(eventInfoAddress, tx_queue);
        GP_WB_WRITE_CONN_EV_INFO_T_NEXT_EXP_ANCHOR_POINT(eventInfoAddress, firstConnEvtTs);
        GP_WB_WRITE_CONN_EV_INFO_T_LAST_CORRELATION(eventInfoAddress, pInfo->tsLastValidPacketReceived);
        GP_WB_WRITE_CONN_EV_INFO_T_LAST_PEER_PACKET(eventInfoAddress, pInfo->tsLastPacketReceived);
        GP_WB_WRITE_CONN_EV_INFO_NR_CONSEC_SKIPPED_EVENTS(eventInfoAddress, pInfo->nrOfConsecSkippedEvents);
        /* Write default guard time: will be updated dynamically by the NRT subsystem */
        GP_WB_WRITE_CONN_EV_INFO_GUARD_TIME(eventInfoAddress,T_CONN_GUARD_TIME(LEN_MIN_DATA, gpHal_BleTxPhy1Mb, gpHal_BleTxPhy1Mb));
        GP_WB_WRITE_CONN_EV_INFO_PREAMBLE(eventInfoAddress, pInfo->preamble);
        GP_WB_WRITE_CONN_EV_INFO_TX_PHY_MODE(eventInfoAddress, pInfo->phy);
        GP_WB_WRITE_CONN_EV_INFO_RX_PHY_MODE(eventInfoAddress, gpHal_BleTxPhy2RxPhy(pInfo->phy));


        // Populate default values for validation, will be optimized later on (see gpHal_BlePopulateValidationParameters)
        GP_WB_WRITE_CONN_EV_INFO_VALIDATION_THRESH(eventInfoAddress, GPHAL_BLE_VALIDATION_THRESHOLD_DEFAULT);
        GP_WB_WRITE_CONN_EV_INFO_VALIDATION_START_IDX(eventInfoAddress, GPHAL_BLE_VALIDATION_INDEX_DEFAULT);
        GP_WB_WRITE_CONN_EV_INFO_FAKE_PREAMBLE_PRESENT(eventInfoAddress, 0);
        GP_WB_WRITE_CONN_EV_INFO_FAKE_PREAMBLE_START_IDX(eventInfoAddress, 0);


        GP_WB_WRITE_CONN_EV_INFO_EN_ANT_SWITCHING(eventInfoAddress, gpHal_Ble_GetAdvAntennaSwitching());
#ifndef GP_HAL_DIVERSITY_SINGLE_ANTENNA
        GP_WB_WRITE_CONN_EV_INFO_ANT_SW_USE_CRC(eventInfoAddress, GP_HAL_BLE_ANT_SW_USE_CRC);
        GP_WB_WRITE_CONN_EV_INFO_ANT_SW_USE_NACK(eventInfoAddress, GP_HAL_BLE_ANT_SW_USE_NACK);
        GP_WB_WRITE_CONN_EV_INFO_ANT_SW_USE_MISSED(eventInfoAddress, GP_HAL_BLE_ANT_SW_USE_MISSED_FRAME);
        GP_WB_WRITE_CONN_EV_INFO_ANT_SW_THRESHOLD(eventInfoAddress, GP_HAL_BLE_ANT_SW_THRESHOLD);
        GP_WB_WRITE_CONN_EV_INFO_ANT_SW_IIR_FACTOR(eventInfoAddress, GP_HAL_BLE_ANT_SW_IIR_FACTOR);
        GP_WB_WRITE_CONN_EV_INFO_ANT_SW_NR_BACKOFF(eventInfoAddress, GP_HAL_BLE_ANT_SW_NR_BACKOFF);

        // Make sure to default initialize these fields, as they are immediately used by RT
        GP_WB_WRITE_CONN_EV_INFO_PER_CURR_ANT(eventInfoAddress, 0);
        GP_WB_WRITE_CONN_EV_INFO_PER_OTHER_ANT(eventInfoAddress, 0);
        GP_WB_WRITE_CONN_EV_INFO_CURR_BACKOFF_CNT(eventInfoAddress, GP_HAL_BLE_ANT_SW_NR_BACKOFF);
#endif // GP_HAL_DIVERSITY_SINGLE_ANTENNA
        GP_WB_WRITE_CONN_EV_INFO_CURR_ANTENNA(eventInfoAddress, gpHal_Ble_GetAdvAntenna());


        // Configure aggressive window widening
        GP_WB_WRITE_CONN_EV_INFO_FIXED_WD_THRESHOLD(eventInfoAddress, pInfo->fixedWDThreshold);

        // This function reads back from the info structure, so should be executed after interval and guard have been written
        gpHal_BleUpdateSleepSettings(eventInfoAddress, 0);

    }
}

void gpHal_BlePopulateUpdateConnectionInfo(gpHal_UpdateConnEventInfo_t* pInfo, gpHal_BleConnectionContext_t* pMapping)
{
    gpHal_Address_t eventInfoAddress;

    GP_ASSERT_DEV_INT(pInfo != NULL);

    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId);

    GP_WB_WRITE_CONN_EV_INFO_INTERVAL(eventInfoAddress, pInfo->interval);
    GP_WB_WRITE_CONN_EV_INFO_WINDOW_DURATION(eventInfoAddress, pInfo->windowDuration);
    // Only write the value for slave latency and let RT enable it when needed
    GP_WB_WRITE_CONN_EV_INFO_SLAVE_LATENCY(eventInfoAddress, pInfo->latency);
    GP_WB_WRITE_CONN_EV_INFO_SLAVE_LATENCY_EN(eventInfoAddress, 0);
    GP_WB_WRITE_CONN_EV_INFO_EVENT_CNT(eventInfoAddress, pInfo->eventCount);
    GP_WB_WRITE_CONN_EV_INFO_UNMAPPED_CH_PTR(eventInfoAddress, pInfo->unmappedChannelPtr);
    GP_WB_WRITE_CONN_EV_INFO_T_LAST_CORRELATION(eventInfoAddress, pInfo->tsLastValidPacketReceived);
    GP_WB_WRITE_CONN_EV_INFO_NR_NO_RX_EVENTS(eventInfoAddress, pInfo->nrNoRXEvents);

    // This function reads back from the info structure, so should be executed after interval has been written
    gpHal_BleUpdateSleepSettings(eventInfoAddress, pInfo->latency);
}

void gpHal_BlePopulateValidationParameters(gpHal_BleConnectionContext_t* pMapping, gpHal_BleValidationParameters_t* pValidation)
{
    gpHal_Address_t eventInfoAddress;
    UInt8 fakePreambleIndex;

    GP_ASSERT_DEV_INT(pMapping != NULL);

    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId);

    // Now populate all relevant parameters
    GP_WB_WRITE_CONN_EV_INFO_VALIDATION_START_IDX(eventInfoAddress, pValidation->validationStartIndex);
    GP_WB_WRITE_CONN_EV_INFO_VALIDATION_THRESH(eventInfoAddress, pValidation->validationThresh);
    GP_WB_WRITE_CONN_EV_INFO_FAKE_PREAMBLE_PRESENT(eventInfoAddress, pValidation->fakePreambleFlag);

    if(pValidation->fakePreambleStartIndex == GPHAL_BLE_PREAMBLE_MATCHING_INDEX_INVALID)
    {
        fakePreambleIndex = 0;
    }
    else
    {
        fakePreambleIndex = pValidation->fakePreambleStartIndex;
    }

    GP_WB_WRITE_CONN_EV_INFO_FAKE_PREAMBLE_START_IDX(eventInfoAddress, fakePreambleIndex);
}

void gpHal_BleUpdatePbmTxPower( gpHal_Address_t optsBase, gpHal_TxPower_t txPower )
{
#if defined(GP_HAL_DIVERSITY_EXT_MODE_SIGNALS)
    GP_WB_WRITE_PBM_BLE_FORMAT_T_MODE_CTRL(optsBase, gpHalPhy_GetTxMode(txPower));
#elif defined(GP_BSP_RF_DEBUG_TX_SUPPORTED)
    // Allow to display TX enable signal on GPIO
    GP_WB_WRITE_PBM_BLE_FORMAT_T_MODE_CTRL(optsBase, BM(GP_BSP_RF_DEBUG_TX_MODE_CTRL_BIT));
#else
    /* not used by hw */
#endif // GP_HAL_DIVERSITY_EXT_MODE_SIGNALS

    GP_WB_WRITE_PBM_BLE_FORMAT_T_PA_POWER_SETTINGS(optsBase, gpHalPhy_GetTxPowerSetting(txPower, GPHAL_TX_OPTIONS_FOR_TRANSMIT));

    GP_WB_WRITE_PBM_BLE_FORMAT_T_PA_POWER_TABLE_INDEX(optsBase, BLE_MGR_PA_POWER_TABLE_INDEX_INVALID);

}

gpHal_Result_t gpHal_BleSetRfPathCompensation(Int8 newTxPathGaindBm, Int8 newRxPathGaindBm)
{
    TxPathGaindBm = newTxPathGaindBm;
    RxPathGaindBm = newRxPathGaindBm;
    return gpHal_ResultSuccess;
}

void gpHal_BleGetRfPathCompensation(Int8 *pTxPathGaindBm, Int8 *pRxPathGaindBm)
{
    *pTxPathGaindBm = TxPathGaindBm;
    *pRxPathGaindBm = RxPathGaindBm;
}

Int8 gpHal_BleCalculateTxPowerAtAntenna(gpHal_TxPower_t chipPortTxPower)
{
    return chipPortTxPower + TxPathGaindBm;
}

void gpHal_BleGetMinMaxPowerLevels(Int8 *minTxPower, Int8 *maxTxPower)
{
    *minTxPower = gpHal_BleCalculateTxPowerAtAntenna(GPHAL_MIN_TRANSMIT_POWER);
    *maxTxPower = gpHal_BleCalculateTxPowerAtAntenna(GPHAL_MAX_TRANSMIT_POWER);
}

gpHal_TxPower_t gpHal_BleCalculateTxPowerAtChipPort(Int8 requested_txPower_dBm_at_Antenna)
{
    gpHal_TxPower_t chipPortTxPower = requested_txPower_dBm_at_Antenna - TxPathGaindBm;

    if(chipPortTxPower < GPHAL_MIN_TRANSMIT_POWER)
    {
        chipPortTxPower = GPHAL_MIN_TRANSMIT_POWER;
    }
    else if(chipPortTxPower > GPHAL_MAX_TRANSMIT_POWER)
    {
        chipPortTxPower = GPHAL_MAX_TRANSMIT_POWER;
    }
    return chipPortTxPower;
}

Int8 gpHal_BleGetNearestSupportedTxPower(Int8 requested_txPower_dBm_at_Antenna)
{
    gpHal_TxPower_t TxPowerAtChipPort = gpHal_BleCalculateTxPowerAtChipPort(requested_txPower_dBm_at_Antenna);
    return gpHal_BleCalculateTxPowerAtAntenna(TxPowerAtChipPort); // note AntennaTxPower may be lower (or higher) than requested_txPower_dBm_at_Antenna
}

gpHal_Result_t gpHal_BleSetTxPower(Int8 requested_txPower_dBm)
{
    // This function configures the BLE default Tx Power - applicable for legacy advertising and connections
    // it also configures the external RF Path (e.g. including an external PA - ouside the chip/package) applicable for all BLE activities (incl ext adv)
    // All BLE Tx Power settings (incl those for for Ext Advertising sets) will be
    // restricted to the Tx Power range corresponding to the RF Path selected by the default Tx Power

    // set dBm Tx power at output of antenna path
    gpHal_Result_t ChipTxPower = gpHal_BleCalculateTxPowerAtChipPort(requested_txPower_dBm);

    gpHal_BleTxPower = ChipTxPower;

    // PA and ANTSELINT were determined from the Tx power required at the antenna, PBM Tx power is what the chip needs to do
    gpHal_BleUpdatePbmTxPower(GP_HAL_PBM_ENTRY2ADDR_OPT_BASE(GP_WB_READ_BLE_MGR_EMPTY_PBM_NR()), ChipTxPower);

    return gpHal_ResultSuccess;
}

gpHal_TxPower_t gpHal_BleGetTxPower(void)
{
    return gpHal_BleCalculateTxPowerAtAntenna(gpHal_BleTxPower);
}

void gpHal_BlePopulateDefaultPbmOptions(UInt8 pbmHandle, Bool TxIfTooLate)
{
    gpHal_Address_t optsBase;

    GP_ASSERT_DEV_EXT(GP_HAL_CHECK_PBM_VALID(pbmHandle));

    optsBase = GP_HAL_PBM_ENTRY2ADDR_OPT_BASE(pbmHandle);

    // Send when too late (optional for advertising, forbidden other services)
    GP_WB_WRITE_PBM_BLE_FORMAT_T_TX_IF_TOO_LATE(optsBase, TxIfTooLate ? 1 : 0);
    GP_WB_WRITE_PBM_BLE_FORMAT_T_TX_ON_TIMESTAMP(optsBase, 1);
    GP_WB_WRITE_PBM_BLE_FORMAT_T_WHITENING_ENABLE(optsBase, 1);
    GP_WB_WRITE_PBM_BLE_FORMAT_T_CHANNEL_IDX(optsBase, GPHAL_BLE_CHANNEL_IDX);
    GP_WB_WRITE_PBM_BLE_FORMAT_T_SKIP_CAL(optsBase, 0x01);
    GP_WB_WRITE_PBM_BLE_FORMAT_T_TX_INTERRUPT_RX(optsBase, 0x01); // workaround for AD-1853

    // Do not use gpHal_BleGetTxPower() here sine that returns the Tx Power at the antenna
    // Here, we need to populate the PBM Tx power with what is needed at the chip port
    gpHal_BleUpdatePbmTxPower(optsBase, gpHal_BleTxPower);
}

void gpHal_BleSetPbmTxPower(UInt8 pbmHandle, Int8 requested_txPower_dBm)
{
    gpHal_Address_t optsBase;
    optsBase = GP_HAL_PBM_ENTRY2ADDR_OPT_BASE(pbmHandle);

    gpHal_BleUpdatePbmTxPower(optsBase, gpHal_BleCalculateTxPowerAtChipPort(requested_txPower_dBm));
}

void gpHal_BlePurgePbmToPd(UInt8 pbmEntry, gpPd_Loh_t* pPdLoh)
{
    gpHal_Address_t pbmOptAddress;
    UInt8 length;
    UInt8 offset;

    pbmOptAddress = GP_HAL_PBM_ENTRY2ADDR_OPT_BASE(pbmEntry);
    offset = GP_WB_READ_PBM_BLE_FORMAT_T_FRAME_PTR(pbmOptAddress);
    length = GP_WB_READ_PBM_BLE_FORMAT_T_FRAME_LEN(pbmOptAddress);

    gpPd_cbDataConfirm(pbmEntry, offset, length, pPdLoh);
}

void gpHal_BleConfPbmToPd(UInt8 pbmEntry, gpPd_Loh_t* pPdLoh)
{
    gpHal_Address_t pbmOptAddress;
    UInt16 length;
    UInt16 offset;
    UInt32 correctedTs;

    pbmOptAddress = GP_HAL_PBM_ENTRY2ADDR_OPT_BASE(pbmEntry);

    GP_ASSERT_SYSTEM(pbmEntry <  GPHAL_NUMBER_OF_PBMS_USED);

    // TX timestamp in PBM deviates from actual ==> apply correction
    correctedTs = GP_WB_READ_PBM_BLE_FORMAT_T_TIMESTAMP(pbmOptAddress) + BLE_TX_TIMESTAMP_OFFSET;

    GP_WB_WRITE_PBM_BLE_FORMAT_T_TIMESTAMP(pbmOptAddress, correctedTs);

    offset = GP_WB_READ_PBM_BLE_FORMAT_T_FRAME_PTR(pbmOptAddress);
    length = GP_WB_READ_PBM_BLE_FORMAT_T_FRAME_LEN(pbmOptAddress);

    // Cut off access address and crc

    offset += 4;
    length -= (4 + 3);

    gpPd_cbDataConfirm(pbmEntry, offset, length, pPdLoh);
}

void gpHal_BleIndPbmToPd(UInt8 pbmEntry, gpPd_Loh_t* pPdLoh)
{
    gpHal_Address_t pbmOptAddress;
    UInt16 length;
    UInt16 offset;
    UInt32 correctedTs;

    pbmOptAddress = GP_HAL_PBM_ENTRY2ADDR_OPT_BASE(pbmEntry);

    GP_ASSERT_SYSTEM(pbmEntry <  GPHAL_NUMBER_OF_PBMS_USED);

    // RX timestamp in PBM deviates from actual (dependent on the PHY) ==> apply correction
    correctedTs = GP_WB_READ_PBM_BLE_FORMAT_R_TIMESTAMP(pbmOptAddress) + gpHal_BleRxTimestampCorrection(GP_WB_READ_PBM_BLE_FORMAT_R_PHY_MODE(pbmOptAddress));

    GP_WB_WRITE_PBM_BLE_FORMAT_R_TIMESTAMP(pbmOptAddress, correctedTs);

    offset = GP_WB_READ_PBM_BLE_FORMAT_R_FRAME_PTR(pbmOptAddress);
    length = GP_WB_READ_PBM_BLE_FORMAT_R_FRAME_LEN(pbmOptAddress);

    /* For indication PDU's, the preamble is not written into the pd.
     * This means we only need to cut off access address and crc (because the LL does not use these fields).
     */

    offset += 4;
    length -= (4 + 3);

    gpPd_DataIndication(pbmEntry, offset, length, pPdLoh, gpPd_BufferTypeBle);
}

void gpHal_BlePurgeAllPendingPdus(gpHal_BleConnectionContext_t* pMapping)
{
    UInt8 rdPointer;
    UInt8 wrPointer;

    rdPointer = GP_WB_READ_CONN_EV_INFO_TX_QUEUE_RD_PTR(GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId));
    wrPointer = GP_WB_READ_CONN_EV_INFO_TX_QUEUE_WR_PTR(GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId));

    while(wrPointer != rdPointer)
    {
        UInt8 pbmHandle;
        gpPd_Loh_t pdLoh;

        // Get a pending pdm
        pbmHandle = GP_HAL_READ_REG(GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId) + GP_WB_CONN_EV_INFO_TX_QUEUE_ADDRESS + rdPointer);
        gpHal_BlePurgePbmToPd(pbmHandle, &pdLoh);
        // handle will be freed in cbPurgeConf
        gpHal_BleCallbacks.cbPurgeConf(pMapping->connId, pdLoh.handle);
        rdPointer = GP_HAL_BLE_QUEUE_CIRC_INC(rdPointer);
    }

    // Queue is empty!
    GP_WB_WRITE_CONN_EV_INFO_TX_QUEUE_RD_PTR(GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId), rdPointer);
}

gpHal_Result_t gpHal_BleStartService(gpHal_BleServiceEventId_t serviceEvent, UInt32 timeStamp, UInt8 type)
{
    gpHal_Result_t result;
    UInt16 customData;

    GP_ASSERT_DEV_INT(gpHal_BleServiceEventContext[serviceEvent].eventNr == GPHAL_ES_ABSOLUTE_EVENT_ID_INVALID);
    // Allocate and schedule the absolute event
    gpHal_BleServiceEventContext[serviceEvent].eventNr = gpHal_GetAbsoluteEvent();

    // Custom data holds the address of the event info structure (convert to compressed, because RT will use this)
    customData = GP_HAL_BLE_SERVICE_TO_OFFSET_FROM_START(serviceEvent);

#ifdef GP_COMP_TXMONITOR
    gpTxMonitor_AnnounceTxStart();
#endif //GP_COMP_TXMONITOR

    result = gpHal_BleStartEvent(timeStamp, type, customData, gpHal_BleServiceEventContext[serviceEvent].eventNr);
    return result;
}

gpHal_Result_t gpHal_BleRestartService(gpHal_BleServiceEventId_t serviceEvent, UInt32 timeStamp, UInt8 type)
{
    UInt16 customData;

    // Custom data holds the address of the event info structure (convert to compressed, because RT will use this)
    customData = GP_HAL_BLE_SERVICE_TO_OFFSET_FROM_START(serviceEvent);

    return gpHal_BleRestartEvent(timeStamp, type, customData, gpHal_BleServiceEventContext[serviceEvent].eventNr);
}

gpHal_Result_t gpHal_BleStopService(gpHal_BleServiceEventId_t serviceEvent)
{
    gpHal_Result_t result;

    result = gpHal_BleStopEvent(gpHal_BleServiceEventContext[serviceEvent].eventNr);

    if(result == gpHal_ResultSuccess)
    {
        gpHal_FreeAbsoluteEvent(gpHal_BleServiceEventContext[serviceEvent].eventNr);
        gpHal_BleServiceEventContext[serviceEvent].eventNr = GPHAL_ES_ABSOLUTE_EVENT_ID_INVALID;
    }

    return result;
}

gpHal_Result_t gpHal_InitBleEventManager(void)
{
    gpHal_Result_t result;
    UInt32 validationSettings = 0;
    Int16 firstTxTsCorrection;
    BtDeviceAddress_t deviceAddress;

    // Init BLE ev manager is only needed when performing BLE activities
    result = gpHal_IpcTriggerCommand(BLE_MGR_CMD_INIT_BLE_MGR, 0, NULL);
    if (result != gpHal_ResultSuccess)
    {
        GP_LOG_PRINTF("MEM_EXCEPTION_TYPE: %x, GPMICRO_LOGICAL_ADDRESS_ON_ERR: 0x%04x", 0, (unsigned int)GP_WB_READ_GPMICRO_MEM_EXCEPTION_TYPE(), GP_WB_READ_GPMICRO_LOGICAL_ADDRESS_ON_ERR());
        GP_ASSERT_SYSTEM(false);
    }

    // This source uses BLE
    GP_WB_WRITE_BLE_MGR_BLE_CHANNEL_IDX(GPHAL_BLE_CHANNEL_IDX);

#ifdef GP_HAL_EXPECTED_CHIP_EMULATED
    if(GP_HAL_IS_EMULATED())
    {
        GP_WB_WRITE_BLE_MGR_EVENT_PROCESSING_DELAY(600);
    }
#endif

    // SDP004-716: Needed when we want T_MAFS
    // GP_WB_WRITE_RIB_CAL_DELAY(6);

    // Compensate for first TX (take calibration delay into account)
    firstTxTsCorrection = -(T_CAL_DELAY + T_OFF2TX_DELAY + T_TX_PAEN_DELAY);
    GP_WB_WRITE_BLE_MGR_FIRST_TX_TIMESTAMP_COR(firstTxTsCorrection);

    // Access address validation settings
    GP_WB_SET_BLE_MGR_ADV_VALIDATION_START_IDX_TO_ADV_VALIDATION_SETTINGS(validationSettings, GP_HAL_BLE_ADV_VALIDATION_START_INDEX);
    GP_WB_SET_BLE_MGR_ADV_FAKE_PREAMBLE_START_IDX_TO_ADV_VALIDATION_SETTINGS(validationSettings, 0);
    GP_WB_SET_BLE_MGR_ADV_FAKE_PREAMBLE_PRESENT_TO_ADV_VALIDATION_SETTINGS(validationSettings, 0);
    GP_WB_WRITE_BLE_MGR_ADV_VALIDATION_SETTINGS(validationSettings);

    GP_WB_WRITE_BLE_MGR_ADV_PREAMBLE_THRESH(GPHAL_BLE_PREAMBLE_THRESHOLD_DEFAULT);
    GP_WB_WRITE_BLE_MGR_ADV_VALIDATION_THRESH(GPHAL_BLE_VALIDATION_THRESHOLD_DEFAULT);

    // Configure BLE event manager with the MAC address from the chip
    gpHal_BleGetChipDeviceAddress(&deviceAddress);
    gpHal_BleSetDeviceAddress(&deviceAddress);

    gpHal_BleConfigureEmptyPdu();

    // Initialize the cleanup time (depending on 15/4 or other connections)
    gpHal_BleUpdateCleanupTime(false);

#if defined(GP_HAL_EXPECTED_CHIP_EMULATED)
    if(GP_HAL_IS_EMULATED())
    {
        // old MF2 Colibri 10 boards have problems with very high phase noise.
        // Limiting the tx_power (hal) setting to 15 (-3dBm) instead of 63 (+8dBm),
        // in combination with the settings below largely avoids the problem.
        // These settings are not needed on MF4 colibri 10, but also do not harm.
        GP_WB_WRITE_FLL_SX_LDO_REFBITS(1);   //dev.hal.write_prop(dev, dev.props.fll.sx_ldo_refbits, 1)
        GP_WB_WRITE_FLL_FLL_LDO_REFBITS(1);   //dev.hal.write_prop(dev, dev.props.fll.fll_ldo_refbits, 1)
        GP_WB_WRITE_TX_TX_DCA_P(7);          //dev.hal.write_prop(dev, dev.props.tx.tx_dca_p, 7)
        GP_WB_WRITE_TX_TX_DCA_N(7);          //dev.hal.write_prop(dev, dev.props.tx.tx_dca_n, 7)
    }
#endif

    GP_WB_WRITE_BLE_MGR_TX_ANTSEL_INT(GP_BSP_TX_ANTSEL_INT_DEFAULT);
    /* Not used by hw */

    return result;
}

UInt32 gpHal_BleGetNextEventTs(gpHal_Address_t eventInfoAddress, UInt8 eType)
{
    UInt32 newTs;

    newTs = GP_WB_READ_CONN_EV_INFO_T_NEXT_EXP_ANCHOR_POINT(eventInfoAddress);

    if (eType == GPHAL_ENUM_EVENT_TYPE_CONNECTION_S)
    {
        /* For a slave the start of the event corresponds to an RX window open while NEXT_EXP_ANCHOR_POINT is the master TX time.
         * Master TX should start in the middle of the slave RX window. */
        newTs -= GP_WB_READ_CONN_EV_INFO_WINDOW_DURATION(eventInfoAddress) / 2;
    }
    return newTs;
}

void gpHal_BleConfigureEmptyPdu(void)
{
    UInt8 pbmHandle;

    // Claim a pbm to use for transmitting empty pdus (should be kept in use as long as the ble event manager is active)
    pbmHandle = gpHal_GetHandle(GP_HAL_RESERVED_PBM_SIZE);
    GP_ASSERT_DEV_EXT(GP_HAL_CHECK_PBM_VALID(pbmHandle));

    gpHal_BlePopulateDefaultPbmOptions(pbmHandle, false);

    // Tell the ble ev manager to use this pbm for empty pdus
    GP_WB_WRITE_BLE_MGR_EMPTY_PBM_NR(pbmHandle);
}

void gpHal_BleOptimizeValidationParameters(void* pArg)
{
    gpHal_BleConnectionContext_t* pMapping = (gpHal_BleConnectionContext_t*)pArg;

    if(pMapping != NULL)
    {
        UInt32 accessAddress;
        gpHal_BleValidationInputParameters_t input;
        gpHal_BleValidationParameters_t validationParameters;
        gpHal_Address_t eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId);
        gpHal_BleTxPhy_t phy;

        phy = gpHal_GetRxPhy(eventInfoAddress);

        // if(phy == gpHal_BleTxPhyCoded125kb || phy == gpHal_BleTxPhyCoded500kb)
        // {
        //     // No validation stuff needed when receiving phy is a coded one
        //     GP_LOG_PRINTF("Skip opt validation params",0);
        //     return;
        // }

        accessAddress = GP_WB_READ_CONN_EV_INFO_ACCESS_ADDRESS(eventInfoAddress);

        if (phy == gpHal_BleTxPhy2Mb)
        {
            GPHAL_BLE_INIT_VALIDATION_INPUT_HDR(input, accessAddress);
        }
        else
        {
            GPHAL_BLE_INIT_VALIDATION_INPUT_NDR(input, accessAddress);
        }

        gpHal_BleGetValidationParameters(&validationParameters, &input);
        gpHal_BlePopulateValidationParameters(pMapping, &validationParameters);
    }
    else
    {
        GP_ASSERT_DEV_INT(false);
    }
}


gpHal_BleTxPhy_t gpHal_GetTxPhy(gpHal_Address_t eventInfoAddress)
{
    return GP_WB_READ_CONN_EV_INFO_TX_PHY_MODE(eventInfoAddress);
}

gpHal_BleRxPhy_t gpHal_GetRxPhy(gpHal_Address_t eventInfoAddress)
{
    return GP_WB_READ_CONN_EV_INFO_RX_PHY_MODE(eventInfoAddress);
}

void gpHal_SetTxPhy(gpHal_Address_t eventInfoAddress, gpHal_BleTxPhy_t tx_phy)
{
    //GP_ASSERT_DEV_INT(tx_phy <= GP_WB_ENUM_BLE_TRANSMITTER_MODE_BLE_LR500);

    GP_WB_WRITE_CONN_EV_INFO_TX_PHY_MODE(eventInfoAddress, tx_phy);
}

void gpHal_SetRxPhy(gpHal_Address_t eventInfoAddress, gpHal_BleRxPhy_t rx_phy)
{
    //GP_ASSERT_DEV_INT(rx_phy <= GP_WB_ENUM_BLE_RECEIVER_MODE_BLE_LR);

    GP_WB_WRITE_CONN_EV_INFO_RX_PHY_MODE(eventInfoAddress, rx_phy);
}

void gpHal_BleTestDisableBleMgr(void)
{
    UInt8 result;
    // test setup
    result = gpHal_IpcTriggerCommand(BLE_MGR_CMD_DISABLE_BLE_MGR, 0, NULL);
    GP_ASSERT_SYSTEM(result == gpHal_ResultSuccess);

    GP_ASSERT_SYSTEM(GP_WB_READ_BLE_MGR_STATE() == GP_WB_ENUM_BLE_MGR_STATE_STANDBY);

    if(GP_WB_READ_PMUD_CLK_32M_PUP())
    {
        while(!GP_WB_READ_PMUD_CLK_32M_RDY()) {}
    }

    // setup to be able to use the direct interface
    GP_WB_WRITE_RIB_MGMT_PAT_SELECT_BLE(true);
}

void gpHal_BleTestReEnableBleMgr(void)
{
    gpHal_Result_t result;

    result = gpHal_IpcTriggerCommand(BLE_MGR_CMD_INIT_BLE_MGR, 0, NULL);

    GP_ASSERT_SYSTEM(result == gpHal_ResultSuccess);

    GP_WB_WRITE_BLE_MGR_BLE_CHANNEL_IDX(GPHAL_BLE_CHANNEL_IDX);
}

void gpHal_BleTestSetChannel(UInt8 phychannel)
{
    gpHal_CalibrateFllChannel(GPHAL_BLE_CHANNEL_IDX, GP_HAL_CONVERT_BLEPHY_TO_FLL_CHANNEL(phychannel));
}

/*****************************************************************************
 *                    Callbacks from gpHal_ISR
 *****************************************************************************/

void gpHal_cbBleDataIndication(UInt8 pbmEntry)
{
    gpPd_Loh_t pdLoh;

    gpHal_BleIndPbmToPd(pbmEntry, &pdLoh);
    {
        UInt8 eventNr;
        UInt8 connId;
        gpHal_BleConnectionContext_t* pMapping;
        gpHal_Address_t pbmOptAddress;

        GP_ASSERT_SYSTEM(gpHal_BleCallbacks.cbDataInd);

        pbmOptAddress = GP_HAL_PBM_ENTRY2ADDR_OPT_BASE(pbmEntry);
        eventNr = GP_WB_READ_PBM_BLE_FORMAT_R_RELATED_EVENT(pbmOptAddress);
        pMapping = gpHal_BleGetConnMappingFromEvent(eventNr);

        if(pMapping != NULL)
        {
            connId = pMapping->connId;
        }
        else
        {
            connId = GPHAL_BLE_CONN_ID_INVALID;
        }

        gpHal_BleCallbacks.cbDataInd(connId, pdLoh);
    }
}

void gpHal_cbBleDataConfirm(UInt8 pbmEntry)
{
    UInt8 eventNr;
    UInt8 connId;
    gpPd_Loh_t pdLoh;
    gpHal_Address_t pbmOptAddress;
    gpHal_BleConnectionContext_t* pMapping;

    GP_ASSERT_SYSTEM(gpHal_BleCallbacks.cbDataConf);

    pbmOptAddress = GP_HAL_PBM_ENTRY2ADDR_OPT_BASE(pbmEntry);

    gpHal_BleConfPbmToPd(pbmEntry, &pdLoh);

    eventNr = GP_WB_READ_PBM_BLE_FORMAT_R_RELATED_EVENT(pbmOptAddress);

    pMapping = gpHal_BleGetConnMappingFromEvent(eventNr);

    if(pMapping == NULL)
    {
        connId = GPHAL_BLE_CONN_ID_INVALID;
    }
    else
    {
        connId = pMapping->connId;
    }

    gpHal_BleCallbacks.cbDataConf(connId, pdLoh);
}

void gpHal_cbBleSubEvConfirm(UInt8 pbmEntry, gpPd_Loh_t *pPdLoh)
{
    gpHal_BleConfPbmToPd(pbmEntry, pPdLoh);
}


void gpHal_cbBleAdvertisingIndication(UInt8 pbmEntry)
{
    gpHal_BleAdvIndInfo_t advIndInfo;
    if ((GP_HAL_CHECK_PBM_VALID(pbmEntry) && GP_HAL_IS_PBM_ALLOCATED(pbmEntry)) == 0)
    {
        GP_LOG_SYSTEM_PRINTF(" invalid pbm cbBleAdv h %u", 0, pbmEntry);
        GP_ASSERT_DEV_EXT(0);
    }


    gpHal_BleIndPbmToPd(pbmEntry, &advIndInfo.pdLoh);
    {
        gpHal_BleCallbacks.cbAdvInd(&advIndInfo);
    }
}

void gpHal_cbBleConnectionRequestIndication(UInt8 pbmEntry)
{
    gpHal_BleSlaveCreateConnInfo_t slaveInfo;

    MEMSET(&slaveInfo, 0, sizeof(gpHal_BleSlaveCreateConnInfo_t));



    gpHal_BleIndPbmToPd(pbmEntry, &slaveInfo.pdLoh);

    GP_ASSERT_SYSTEM(gpHal_BleCallbacks.cbSlaveCreateConn);
    gpHal_BleCallbacks.cbSlaveCreateConn(&slaveInfo);
}

void gpHal_BleTriggerCbMasterCreateConn(UInt8 connRspPbm)
{
    gpHal_Address_t eventInfoAddress;
    UInt8 connReqPbm;
    gpHal_BleMasterCreateConnInfo_t masterInfo;

    MEMSET(&masterInfo, 0, sizeof(gpHal_BleMasterCreateConnInfo_t));
    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_SERVICE_TO_EVENT_INFO_ADDRESS(gpHal_BleServiceEventIdInitiating);
    connReqPbm = GP_WB_READ_INIT_EV_INFO_CONN_REQ_PBM(eventInfoAddress);

    if(connRspPbm == GP_PBM_INVALID_HANDLE)
    {
        // Legacy case
    }
    else
    {
        // Not expecting a connRspPbm when EXTENDED_INITIATING is disabled
        GP_ASSERT_DEV_INT(false);
    }



    gpHal_BleConfPbmToPd(connReqPbm, &masterInfo.pdLoh);

    GP_ASSERT_SYSTEM(gpHal_BleCallbacks.cbMasterCreateConn!= NULL);
    gpHal_BleCallbacks.cbMasterCreateConn(&masterInfo);
}

// Only triggered when a connection was created on a primary advertising channel (legacy, non AE)
void gpHal_cbBleConnectionRequestConfirm(void)
{
    gpHal_BleTriggerCbMasterCreateConn(GP_PBM_INVALID_HANDLE);
}

// Only triggered when a connection was created on a secondary advertising channel (AE)
void gphal_cbBleConnectionResponseIndication(UInt8 pbmEntry)
{
    gpHal_BleTriggerCbMasterCreateConn(pbmEntry);
}

void gpHal_cbBleEventProcessed(UInt8 eventId)
{
    if(eventId == gpHal_BleServiceEventContext[gpHal_BleServiceEventIdAdvertising].eventNr)
    {
        // End of Scan Event handling
        if(gpHal_BleCallbacks.cbAdvEvDone)
        {
            gpHal_BleCallbacks.cbAdvEvDone();
        }
    }
    else if(eventId == gpHal_BleServiceEventContext[gpHal_BleServiceEventIdScanning].eventNr)
    {
        // End of Scan Event handling
        if(gpHal_BleCallbacks.cbScanEvDone)
        {
            gpHal_BleCallbacks.cbScanEvDone();
        }
    }
    else if(eventId == gpHal_BleServiceEventContext[gpHal_BleServiceEventIdInitiating].eventNr)
    {
        // End of Scan Event handling
        if(gpHal_BleCallbacks.cbInitEvDone)
        {
            gpHal_BleCallbacks.cbInitEvDone();
        }
    }
    else
    {
        // Check if it is a connection event done callback
        gpHal_BleConnectionContext_t* pMapping;

        pMapping = gpHal_BleGetConnMappingFromEvent(eventId);

        if (pMapping != NULL)
        {
            if(gpHal_BleCallbacks.cbConnEventDone)
            {
                // invoke the End of connection event handler for this connection
                gpHal_BleCallbacks.cbConnEventDone(pMapping->connId);
            }
        }
    }
}

void gpHal_cbBleLastConnEventCountReached(UInt8 eventId)
{
    gpHal_BleConnectionContext_t* pMapping;
    gpHal_Address_t eventInfoAddress;

    pMapping = gpHal_BleGetConnMappingFromEvent(eventId);

    if(pMapping == NULL)
    {
        // We received an interrupt from a connection that is already terminated ==> drop
        return;
    }

    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId);

    // If we get this interrupt, the BLE ev manager has stopped (paused) the running schedule
    pMapping->running = false;

    // Invalidate and allow new requests
    GP_WB_WRITE_CONN_EV_INFO_LAST_SCH_EVENT_CNT_VALID(eventInfoAddress, 0);

    if(gpHal_BleCallbacks.cbLastSchedEventPassed)
    {
        gpHal_BleCallbacks.cbLastSchedEventPassed(pMapping->connId);
    }
}

void gpHal_cbBleScanRequestReceived(void)
{
    {
        // Implement handling
    }
}




void gpHal_BleUpdateCleanupTime(Bool eventRemoved)
{
#ifndef GP_COMP_MACCORE
    UIntLoop i;
    UInt8 nrOfActivities;
    UInt8 nrOfActivitiesForZeroCleanup;
#endif
    UInt16 cleanupTime;

    // Use default cleanup, unless there only is one connection
    cleanupTime = T_CLEANUP;

#ifndef GP_COMP_MACCORE
    nrOfActivities = 0;
    nrOfActivitiesForZeroCleanup = 1;

    if(eventRemoved)
    {
        // The event is removed, but the context is not freed yet. Take into account
        nrOfActivitiesForZeroCleanup++;
    }

    for(i = 0; i < GPHAL_BLE_MAX_NR_OF_SUPPORTED_CONNECTIONS; i++)
    {
        gpHal_BleConnectionContext_t* pMapping;

        pMapping = gpHal_BleGetConnMappingFromId(i);

        if(pMapping != NULL && !pMapping->virtual)
        {
            // Only take non-virtual connections into account
            nrOfActivities++;
        }
    }

    for(i = 0; i < GPHAL_BLE_NR_OF_SERVICE_EVENTS; i++)
    {
        if(gpHal_BleServiceEventContext[i].eventNr != GPHAL_ES_ABSOLUTE_EVENT_ID_INVALID)
        {
            nrOfActivities++;
        }
    }

    if(nrOfActivities <= nrOfActivitiesForZeroCleanup)
    {
        cleanupTime = 0;
    }

    GP_LOG_PRINTF("#act: %u change cleanup %u-> %u",0, nrOfActivities-eventRemoved, GP_WB_READ_BLE_MGR_CLEANUP_TIME(), cleanupTime);
#endif //!GP_COMP_MACCORE

    GP_WB_WRITE_BLE_MGR_CLEANUP_TIME(cleanupTime);
}



gpHal_BleRxPhy_t gpHal_BleTxPhy2RxPhy(gpHal_BleTxPhy_t txPhy)
{
    gpHal_BleRxPhy_t rxPhy = gpHal_BleRxPhyInvalid;

    switch(txPhy)
    {
        case gpHal_BleTxPhy1Mb:
            rxPhy = gpHal_BleRxPhy1Mb;
            break;
        case gpHal_BleTxPhy2Mb:
            rxPhy = gpHal_BleRxPhy2Mb;
            break;
        // case gpHal_BleTxPhyCoded125kb:
        // case gpHal_BleTxPhyCoded500kb:
        //     rxPhy = gpHal_BleRxPhyCoded;
        //     break;
        case gpHal_BleTxPhyInvalid:
        default:
            GP_ASSERT_DEV_INT(false);
            break;
    }

    return rxPhy;
}

UInt32 gpHal_BleRxTimestampCorrection(UInt8 phy)
{
    Int16 correction;

    switch(phy)
    {
        case gpHal_BlePhyMode1Mb:
        {
            correction = BLE_RX_TIMESTAMP_OFFSET;
            break;
        }
        case gpHal_BlePhyMode2Mb:
        {
            correction = BLE_HDR_RX_TIMESTAMP_OFFSET;
            break;
        }
        default:
        {
            correction = 0;
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }

    return correction;
}

void gpHal_BleUpdateSleepSettings(gpHal_Address_t connEvInfoAddress, UInt16 slaveLatency)
{
    UInt32 connIntervalUs = GP_WB_READ_CONN_EV_INFO_INTERVAL(connEvInfoAddress);
    UInt32 nonRxPortionUs = GP_WB_READ_CONN_EV_INFO_GUARD_TIME(connEvInfoAddress) + T_SAFETY_MARGIN;
    UInt16 combinedScaWorst = GP_HAL_BLE_GET_COMBINED_SCA_NO_COMPENSATION(GP_WB_READ_CONN_EV_INFO_COMBINED_SCA_WORST(connEvInfoAddress));
    UInt32 fixedWdDuration = 0;
    UInt32 maxAllowedSleepTimeUs = 0;
    UInt16 maxAllowedLatency;

    if(connIntervalUs > nonRxPortionUs)
    {
        // If window size exceeds connIntervalUs-guard_time, 1 out of 2 events will be skipped.
        // To avoid this, set the max (fixed) window duration to a smaller value.
        // Also, apply a max window duration to prevent overflows in the calculation of maxAllowedSleepTimeUs.
        fixedWdDuration = min((connIntervalUs-nonRxPortionUs), GP_HAL_BLE_FIXED_WD_DURATION_LIMIT_US);
    }

    if(combinedScaWorst == 0)
    {
        // Master link, no need to continue
        return;
    }

    // Given the current window duration and total (master + slave) worst-case SCA, we can calculate the max total sleep time
    maxAllowedSleepTimeUs = (fixedWdDuration / 2) * (1000000 / combinedScaWorst);

    if(maxAllowedSleepTimeUs < connIntervalUs)
    {
        // A sleep time smaller than an interval does not make sense, prevent underflow and return
        GP_LOG_SYSTEM_PRINTF("WARN: max allowed sleep time %lu < interval %lu",0, (unsigned long)maxAllowedSleepTimeUs, (unsigned long)connIntervalUs);
        return;
    }

    maxAllowedLatency = (maxAllowedSleepTimeUs / connIntervalUs) - 1;

    GP_WB_WRITE_CONN_EV_INFO_SLAVE_LATENCY(connEvInfoAddress, min(slaveLatency, maxAllowedLatency));

}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpHal_InitBle(void)
{
    gpHal_Result_t result;
    UIntLoop i;

    NOT_USED(gpHal_Ble_EventInfoMemory);
    // gpHal_BleTxPower tx power is used in gpHal_BleConfigureEmptyPdu invoked by gpHal_InitBleEventManager
    gpHal_BleTxPower = GPHAL_DEFAULT_TRANSMIT_POWER > GP_HAL_BLE_MAX_DEFAULT_TX_POWER ? GP_HAL_BLE_MAX_DEFAULT_TX_POWER : GPHAL_DEFAULT_TRANSMIT_POWER; /* txPower at chip port */

    MEMSET(&gpHal_BleCallbacks, 0, sizeof(gpHal_BleCallbacks_t));

    MEMSET(gpHal_BleChannelMapMapping, 0, sizeof(gpHal_BleChannelMapMapping));

    // Reset service event context
    for(i = 0; i < GPHAL_BLE_NR_OF_SERVICE_EVENTS; i++)
    {
        gpHal_BleServiceEventContext[i].eventNr = GPHAL_ES_ABSOLUTE_EVENT_ID_INVALID;
    }

    MEMSET(gpHal_Ble_ClosedLoop, 0, sizeof(gpHal_Ble_ClosedLoop));


    GP_WB_WRITE_BLE_MGR_TX_POWER_CONFIG_PTR(TO_GPM_ADDR((UIntPtr)gpHal_Ble_ClosedLoop));

#ifdef GP_HAL_DIVERSITY_BLE_AE_RX_PRECALIBRATION
    gpHal_BlePreCalibrationInit();
#endif //GP_HAL_DIVERSITY_BLE_AE_RX_PRECALIBRATION

    // Initialize BLE event manager (RT subsystem)
    result = gpHal_InitBleEventManager();

    if(result != gpHal_ResultSuccess)
    {
        GP_ASSERT_SYSTEM(false);
    }

    GP_WB_WRITE_RIB_IS_BLE_CH3(1);

#ifdef GP_HAL_DIVERSITY_MULTISTANDARD_LISTENING_MODE
    GP_WB_WRITE_BLE_MGR_MS_ENABLED(1);
#else
    GP_WB_WRITE_BLE_MGR_MS_ENABLED(0);
#endif // GP_HAL_DIVERSITY_MULTISTANDARD_LISTENING_MODE
    GP_WB_WRITE_INT_CTRL_MASK_IPCGPM2X_EVENT_PROCESSED_INTERRUPT(1);
    GP_WB_WRITE_INT_CTRL_MASK_IPCGPM2X_LAST_CONN_EVENT_CNT_INTERRUPT(1);
    GP_WB_WRITE_INT_CTRL_MASK_IPCGPM2X_STAT_INTERRUPT(1);
#ifdef GP_COMP_CHIPEMU
    GP_WB_WRITE_INT_CTRL_MASK_IPCGPM2X_UNEXPECTED_COND_INTERRUPT(1);
#endif



    GP_WB_WRITE_BLE_MGR_SCAN_EXT_INFO_BASE_PTR(TO_GPM_ADDR(GP_HAL_BLE_EXT_SCAN_INFO_START));
    GP_WB_WRITE_BLE_MGR_INIT_EXT_INFO_BASE_PTR(TO_GPM_ADDR(GP_HAL_BLE_EXT_INIT_INFO_START));

    gpHal_BleCommonInit();
    gpHal_BleWlInit();
    gpHal_BleValidationInit();



    gpHal_BleSetTxPower(gpHal_BleTxPower);

#ifndef GP_COMP_CHIPEMU
    rap_dsfix_setup((UIntPtr)gpHal_BleDsFixRtScratchArray);
#endif //GP_COMP_CHIPEMU


            }


void gpHal_BleGetDeviceAddress(BtDeviceAddress_t* pAddress)
{
    GP_HAL_READ_BYTE_STREAM(GP_WB_BLE_MGR_DEVICE_ADDRESS_ADDRESS, pAddress->addr, sizeof(BtDeviceAddress_t));
}

void gpHal_BleSetDeviceAddress(BtDeviceAddress_t* pAddress)
{
    GP_HAL_WRITE_BYTE_STREAM(GP_WB_BLE_MGR_DEVICE_ADDRESS_ADDRESS, pAddress->addr, sizeof(BtDeviceAddress_t));
}

gpHal_BleChannelMapHandle_t gpHal_BleAllocateChannelMapHandle(void)
{
    UIntLoop i;

    for(i = 0; i < GPHAL_BLE_MAX_NR_OF_SUPPORTED_CHANNEL_MAPS; i++)
    {
        if(!gpHal_BleChannelMapMapping[i].allocated)
        {
            gpHal_BleChannelMapMapping[i].allocated = true;
            return i;
        }
    }

    return GP_HAL_BLE_CHANNEL_MAP_INVALID;
}

Bool gpHal_BleIsChannelMapValid(gpHal_BleChannelMapHandle_t handle)
{
    if(handle < GPHAL_BLE_MAX_NR_OF_SUPPORTED_CHANNEL_MAPS)
    {
        if(gpHal_BleChannelMapMapping[handle].allocated)
        {
            return true;
        }
    }

    return false;
}

gpHal_Result_t gpHal_BleIsAccessAddressAcceptable(UInt32 accessAddress)
{
    gpHal_BleValidationInputParameters_t input;
    gpHal_BleValidationParameters_t validationParameters;

    GPHAL_BLE_INIT_VALIDATION_INPUT_NDR(input, accessAddress);

    gpHal_BleGetValidationParameters(&validationParameters, &input);

    if(validationParameters.isReliableAccessAddress)
    {
        return gpHal_ResultSuccess;
    }

    return gpHal_ResultInvalidParameter;
}


void gpHal_BleFreeChannelMapHandle(gpHal_BleChannelMapHandle_t handle)
{
    GP_ASSERT_DEV_INT(gpHal_BleIsChannelMapValid(handle));

    gpHal_BleChannelMapMapping[handle].allocated = false;
}


void gpHal_BleSetChannelMap(gpHal_BleChannelMapHandle_t handle, gpHal_ChannelMap_t* pChanMap)
{
    Bool valid = gpHal_BleIsChannelMapValid(handle);
    GP_ASSERT_DEV_INT(valid);
    if(valid)
    {
        gpHal_Address_t chanMapPtr;

        chanMapPtr = (gpHal_Address_t)GP_HAL_BLE_CHAN_MAP_TO_EVENT_INFO_ADDRESS(handle);

        GP_HAL_WRITE_BYTE_STREAM(chanMapPtr, pChanMap->usedChanIds, sizeof(pChanMap->usedChanIds));
        GP_HAL_WRITE_BYTE_STREAM(chanMapPtr + GP_WB_CONN_CH_MAP_HOP_REMAP_0_ADDRESS, pChanMap->remapTable, sizeof(pChanMap->remapTable));
        GP_WB_WRITE_CONN_CH_MAP_HOP_REMAP_TABLE_LEN(chanMapPtr, pChanMap->hopRemapTableLength);
    }
}

void gpHal_BleGetChannelMap(gpHal_BleChannelMapHandle_t handle, gpHal_ChannelMap_t* pChanMap)
{
    Bool valid = gpHal_BleIsChannelMapValid(handle);
    GP_ASSERT_DEV_INT(valid);
    if(valid)
    {
        gpHal_Address_t chanMapPtr;

        chanMapPtr = (gpHal_Address_t)GP_HAL_BLE_CHAN_MAP_TO_EVENT_INFO_ADDRESS(handle);

        GP_HAL_READ_BYTE_STREAM(chanMapPtr, pChanMap->usedChanIds, sizeof(pChanMap->usedChanIds));
        GP_HAL_READ_BYTE_STREAM(chanMapPtr + GP_WB_CONN_CH_MAP_HOP_REMAP_0_ADDRESS, pChanMap->remapTable, sizeof(pChanMap->remapTable));
        pChanMap->hopRemapTableLength = GP_WB_READ_CONN_CH_MAP_HOP_REMAP_TABLE_LEN(chanMapPtr);
    }
}

void gpHal_EnableMasterCreateConnInterrupts(Bool enable)
{
    GP_WB_WRITE_INT_CTRL_MASK_IPCGPM2X_CONN_REQ_TX_INTERRUPT(enable);
}

void gpHal_EnableSlaveCreateConnInterrupts(Bool enable)
{
    GP_WB_WRITE_INT_CTRL_MASK_RCI_BLE_CONN_REQ_IND_INTERRUPT(enable);
}


void gpHal_EnableAdvIndInterrupts(Bool enable)
{
    GP_WB_WRITE_INT_CTRL_MASK_RCI_BLE_ADV_IND_INTERRUPT(enable);
}

void gpHal_EnableDataIndInterrupts(Bool enable)
{
    GP_WB_WRITE_INT_CTRL_MASK_RCI_BLE_DATA_IND_INTERRUPT(enable);
}

void gpHal_EnableDataConfInterrupts(Bool enable)
{
    GP_WB_WRITE_INT_CTRL_MASK_RCI_BLE_DATA_CNF_INTERRUPT(enable);
}



// event start stop functions
gpHal_Result_t gpHal_BleStartAdvertising(UInt32 firstAdvTs, gpHal_AdvEventInfo_t* pInfo)
{
    gpHal_Result_t result;

    // write away the event info structure
    result = gpHal_BlePopulateAdvEventInfo(pInfo);

    if(result != gpHal_ResultSuccess)
    {
        return result;
    }

    result = gpHal_BleStartService(gpHal_BleServiceEventIdAdvertising, firstAdvTs, GPHAL_ENUM_EVENT_TYPE_ADVERTISE);

    return result;
}

gpHal_Result_t gpHal_BleRestartAdvertising(UInt32 advTs)
{
    return gpHal_BleRestartService(gpHal_BleServiceEventIdAdvertising, advTs, GPHAL_ENUM_EVENT_TYPE_ADVERTISE);
}


gpHal_Result_t gpHal_BleStopAdvertising(gpPd_Loh_t* pdLohAdv, gpPd_Loh_t* pdLohScan)
{
    UInt8 pbmHandle;

    gpHal_Address_t eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_SERVICE_TO_EVENT_INFO_ADDRESS(gpHal_BleServiceEventIdAdvertising);

    // Read back pbm information
    pbmHandle = GP_WB_READ_ADV_EV_INFO_ADV_PBM(eventInfoAddress);

    gpHal_BleConfPbmToPd(pbmHandle, pdLohAdv);

    pbmHandle = GP_WB_READ_ADV_EV_INFO_SCAN_RSP_PBM(eventInfoAddress);

    if(GP_HAL_CHECK_PBM_VALID(pbmHandle))
    {
        gpHal_BleConfPbmToPd(pbmHandle, pdLohScan);
    }

    gpHal_Result_t status = gpHal_BleStopService(gpHal_BleServiceEventIdAdvertising);
    GP_DO_WHILE_TIMEOUT_ASSERT(GP_WB_READ_BLE_MGR_STATE() == GP_WB_ENUM_BLE_MGR_STATE_ABORT, GP_HAL_BLE_ADV_ABORT_TIMEOUT_US);

    return status;
}

gpHal_Result_t gpHal_StartBgscEvent(void)
{
    gpHal_Result_t result;
    UInt32 firstScanStartTs;

    HAL_TIMER_GET_CURRENT_TIME_1US(firstScanStartTs);

    // It is not strictly needed to add T_PROCESSING here, but in practice, scanning will not start any earlier, so it does not harm
    firstScanStartTs += T_PROCESSING;

    result = gpHal_BleStartService(gpHal_BleServiceEventIdBgsc, firstScanStartTs, GPHAL_ENUM_EVENT_TYPE_BG_SCANNING);

    if(result == gpHal_ResultSuccess)
    {
        // When we were able to start bg scanning, also enable the resume functionality
        GP_WB_WRITE_BLE_MGR_BGSCAN_EV_NR(gpHal_BleServiceEventContext[gpHal_BleServiceEventIdBgsc].eventNr);
    }

    return result;
}

gpHal_Result_t gpHal_RestartBgscEvent(void)
{
    gpHal_Result_t result;

    result = gpHal_StopBgscEvent();

    if(result != gpHal_ResultSuccess)
    {
        return result;
    }

    return gpHal_StartBgscEvent();
}

gpHal_Result_t gpHal_StopBgscEvent(void)
{
    gpHal_Result_t result;

    // Make sure we will not trigger resumes while our event is being invalidated
    GP_WB_WRITE_BLE_MGR_BGSCAN_EV_NR(GPHAL_ES_ABSOLUTE_EVENT_ID_INVALID);

    result = gpHal_BleStopService(gpHal_BleServiceEventIdBgsc);

    if(result != gpHal_ResultSuccess)
    {
        // We were not able to stop background scanning, restore the resume state
        GP_WB_WRITE_BLE_MGR_BGSCAN_EV_NR(gpHal_BleServiceEventContext[gpHal_BleServiceEventIdBgsc].eventNr);
    }

    return result;
}

gpHal_Result_t gpHal_StartBleBgscService(gpHal_BleServiceEventId_t serviceEvent, gpHal_phyMask_t phyMask)
{
    gpHal_Result_t result = gpHal_ResultInvalidRequest;
    gpHal_Address_t bgscInfoAddress = (gpHal_Address_t)GP_HAL_BLE_SERVICE_TO_EVENT_INFO_ADDRESS(gpHal_BleServiceEventIdBgsc);
    UInt8 old_flags = GP_WB_READ_BGSC_EV_INFO_SCAN_FLAGS(bgscInfoAddress);
    UInt8 new_flags = old_flags;

    switch (serviceEvent)
    {
        case gpHal_BleServiceEventIdScanning:
            GP_WB_WRITE_BGSC_EV_INFO_SCAN_EV_INFO_PTR(bgscInfoAddress, GP_HAL_BLE_SERVICE_TO_OFFSET_FROM_START(gpHal_BleServiceEventIdScanning));
            GP_WB_SET_BGSC_EV_INFO_ENABLE_SCAN_TO_SCAN_FLAGS(new_flags, 1);
            break;
        case gpHal_BleServiceEventIdInitiating:
            GP_WB_WRITE_BGSC_EV_INFO_INIT_EV_INFO_PTR(bgscInfoAddress, GP_HAL_BLE_SERVICE_TO_OFFSET_FROM_START(gpHal_BleServiceEventIdInitiating));
            GP_WB_SET_BGSC_EV_INFO_ENABLE_INIT_TO_SCAN_FLAGS(new_flags, 1);
            break;
        default:
            GP_ASSERT_SYSTEM(false); /* invalid param */
    }

    GP_ASSERT_DEV_INT(old_flags || new_flags);
    if (old_flags)
    {
        // Already enabled
        // We need to update the parameters when initiating is added to scanning or vice versa
        // it is impossible to receive new scan parameters while scanning is enabled
        if (gpHal_BleCompareEventParams(serviceEvent, phyMask))
        {
            // We need to update the bgsc parameters
            gpHal_BlePopulateBgscEventInfo(bgscInfoAddress, serviceEvent, new_flags, phyMask);
            GP_WB_WRITE_BGSC_EV_INFO_SCAN_FLAGS(bgscInfoAddress, new_flags);

            result = gpHal_RestartBgscEvent();
        }
        else
        {
            // We don't need to update the bgsc parameters
            GP_WB_WRITE_BGSC_EV_INFO_SCAN_FLAGS(bgscInfoAddress, new_flags);
            result = gpHal_ResultSuccess;
        }
    }
    else //if (new_flags)
    {
        gpHal_BlePopulateBgscEventInfo(bgscInfoAddress, serviceEvent, new_flags, phyMask);
        GP_WB_WRITE_BGSC_EV_INFO_SCAN_FLAGS(bgscInfoAddress, new_flags);

        result = gpHal_StartBgscEvent();
    }

    return result;
}

gpHal_Result_t gpHal_StopBleBgscService(gpHal_BleServiceEventId_t serviceEvent)
{
    gpHal_Result_t result = gpHal_ResultInvalidRequest;
    gpHal_Address_t bgscInfoAddress = (gpHal_Address_t)GP_HAL_BLE_SERVICE_TO_EVENT_INFO_ADDRESS(gpHal_BleServiceEventIdBgsc);

    UInt8 old_flags = GP_WB_READ_BGSC_EV_INFO_SCAN_FLAGS(bgscInfoAddress);
    UInt8 new_flags = old_flags;
    gpHal_phyMask_t phyMask = {.mask = 0x00};

    switch (serviceEvent)
    {
        case gpHal_BleServiceEventIdScanning:
            GP_WB_SET_BGSC_EV_INFO_ENABLE_SCAN_TO_SCAN_FLAGS(new_flags, 0);
            break;
        case gpHal_BleServiceEventIdInitiating:
            GP_WB_SET_BGSC_EV_INFO_ENABLE_INIT_TO_SCAN_FLAGS(new_flags, 0);
            break;
        default:
            GP_ASSERT_SYSTEM(false); /* invalid param */
    }

    if (new_flags)
    {
        // some services still enabled
        // There are only two services (x, y), so if we disabled x and one is still running it must be y
        switch (serviceEvent)
        {
            case gpHal_BleServiceEventIdScanning:
                serviceEvent = gpHal_BleServiceEventIdInitiating;
                phyMask.mask = GP_WB_READ_SCAN_EXT_INFO_PRI_PHY_MASK(GP_HAL_BLE_EXT_SCAN_INFO_START); // we can only copy the existing mask, we cannot restore the one originally set with the service
                break;
            case gpHal_BleServiceEventIdInitiating:
                serviceEvent = gpHal_BleServiceEventIdScanning;

                phyMask.mask = GP_WB_READ_INIT_EXT_INFO_PRI_PHY_MASK(GP_HAL_BLE_EXT_INIT_INFO_START); // we can only copy the existing mask, we cannot restore the one originally set with the service
                break;
            default:
                GP_ASSERT_SYSTEM(false); /* invalid param */
        }

        gpHal_BlePopulateBgscEventInfo(bgscInfoAddress, serviceEvent, new_flags, phyMask);
        GP_WB_WRITE_BGSC_EV_INFO_SCAN_FLAGS(bgscInfoAddress, new_flags);

        result = gpHal_RestartBgscEvent();
    }
    else
    {
        GP_WB_WRITE_BGSC_EV_INFO_SCAN_FLAGS(bgscInfoAddress, new_flags);

        result = gpHal_StopBgscEvent();
    }

    return result;
}

gpHal_Result_t gpHal_BleStartScanning(UInt32 firstScanTs, gpHal_ScanEventInfo_t* pInfo)
{
    gpHal_Result_t result;

    // write away the event info structure
    result = gpHal_BlePopulateScanEventInfo(pInfo);

    if(result != gpHal_ResultSuccess)
    {
        return result;
    }

    result = gpHal_StartBleBgscService(gpHal_BleServiceEventIdScanning, pInfo->phyMask);

    return result;
}

gpHal_Result_t gpHal_BleStopScanning(gpPd_Loh_t* pdLohScan)
{
    UInt8 pbmHandle;

    gpHal_Address_t eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_SERVICE_TO_EVENT_INFO_ADDRESS(gpHal_BleServiceEventIdScanning);

    pbmHandle = GP_WB_READ_SCAN_EV_INFO_SCAN_REQ_PBM(eventInfoAddress);

    if(GP_HAL_CHECK_PBM_VALID(pbmHandle))
    {
        gpHal_BleConfPbmToPd(pbmHandle, pdLohScan);
    }
    return gpHal_StopBleBgscService(gpHal_BleServiceEventIdScanning);
}

gpHal_Result_t gpHal_BleStartInitScanning(UInt32 firstInitScanTs, gpHal_InitEventInfo_t* pInitEventInfo)
{
    gpHal_Result_t result;


    // write away the event info structure
    result = gpHal_BlePopulateInitEventInfo(pInitEventInfo);

    if(result != gpHal_ResultSuccess)
    {
        return result;
    }

    result = gpHal_StartBleBgscService(gpHal_BleServiceEventIdInitiating, pInitEventInfo->initPhyMask);

    return result;
}

gpHal_Result_t gpHal_BleRestartInitScanning(UInt32 firstInitScanTs, gpPd_Loh_t ConnectReqPdu)
{
    gpHal_Result_t result;
    UInt8 pbmHandle;
    gpHal_Address_t eventInfoAddress;
    gpHal_phyMask_t phyMask;

    phyMask.mask = GP_WB_READ_INIT_EXT_INFO_PRI_PHY_MASK(GP_HAL_BLE_EXT_INIT_INFO_START);

    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_SERVICE_TO_EVENT_INFO_ADDRESS(gpHal_BleServiceEventIdInitiating);

    pbmHandle = gpHal_BlePdToPbm(ConnectReqPdu, false);

    if(!GP_HAL_CHECK_PBM_VALID(pbmHandle))
    {
        return gpHal_ResultInvalidParameter;
    }

    GP_WB_WRITE_INIT_EV_INFO_CONN_REQ_PBM(eventInfoAddress, pbmHandle);

    result = gpHal_StartBleBgscService(gpHal_BleServiceEventIdInitiating, phyMask);

    return result;
}


gpHal_Result_t gpHal_BleStartConnection(UInt8 connId, UInt32 firstConnTs, gpHal_ConnEventInfo_t* pConnEventInfo)
{
    gpHal_Result_t result;
    gpHal_BleConnectionContext_t* pMapping;
    UInt16 infoAddress;

    // Check if there isn't already an event running
    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping != NULL)
    {
        return gpHal_ResultBusy;
    }

    pMapping = gpHal_BleAddConnIdMapping(connId);

    if(pMapping == NULL)
    {
        return gpHal_ResultBusy;
    }

    infoAddress = GP_HAL_BLE_CONNECTION_TO_OFFSET_FROM_START(pMapping->connId);


    // write away the event info structure
    gpHal_BlePopulateConnEventInfo(pConnEventInfo, pMapping, firstConnTs);


    result = gpHal_BleStartEvent(firstConnTs, GPHAL_ENUM_EVENT_TYPE_CONNECTION_S, infoAddress, pMapping->eventNr);

    if(result != gpHal_ResultSuccess)
    {
        gpHal_BleRemoveConnIdMapping(pMapping);
        return result;
    }

    pMapping->running = true;

    // Schedule the calculation for the optimal validation settings (should be delayed, because it takes some time to complete)
    gpSched_ScheduleEventArg(0, gpHal_BleOptimizeValidationParameters, (void*)pMapping);

    return result;
}

gpHal_Result_t gpHal_BleStartVirtualConnection(UInt8 connId, UInt32 firstConnTs, UInt32 interval)
{
    gpHal_BleConnectionContext_t* pMapping;

    GP_ASSERT_DEV_INT(connId & GPHAL_BLE_VIRTUAL_CONN_MASK);

    // Check if there isn't already an event running
    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping != NULL)
    {
        GP_LOG_PRINTF("Mapping for connId %u already exists", 0, connId);
        return gpHal_ResultInvalidParameter;
    }

    pMapping = gpHal_BleAddConnIdMapping(connId);

    if(pMapping == NULL)
    {
        GP_LOG_PRINTF("No mapping found for connId %u", 0, connId);
        return gpHal_ResultBusy;
    }

    gpHal_BleStartVirtualEvent(firstConnTs, interval, pMapping->eventNr);

    pMapping->virtual = true;
    pMapping->running = true;

    return gpHal_ResultSuccess;
}

gpHal_Result_t gpHal_BleStopVirtualConnection(UInt8 connId)
{
    gpHal_BleConnectionContext_t* pMapping;

    GP_ASSERT_DEV_INT(connId & GPHAL_BLE_VIRTUAL_CONN_MASK);

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        return gpHal_ResultInvalidParameter;
    }

    if(GP_WB_READ_EVENT_TYPE_FIELD(GPHAL_ES_EVENT_NR_TO_START_OFFSET(pMapping->eventNr)) != GPHAL_ENUM_EVENT_TYPE_VIRTUAL)
    {
        // Event number should correspond to a virtual one
        return gpHal_ResultInvalidParameter;
    }

    gpHal_BleStopVirtualEvent(pMapping->eventNr);

    pMapping->running = false;

    gpHal_BleRemoveConnIdMapping(pMapping);

    return gpHal_ResultSuccess;
}

gpHal_Result_t gpHal_BleEstablishMasterConnection(UInt8 virtualConnId, UInt8 masterConnId, UInt32* nextEventTs)
{
    gpHal_BleConnectionContext_t* pMapping;

    GP_ASSERT_DEV_INT((masterConnId & GPHAL_BLE_VIRTUAL_CONN_MASK) == 0);
    GP_ASSERT_DEV_INT(virtualConnId & GPHAL_BLE_VIRTUAL_CONN_MASK);

    pMapping = gpHal_BleGetConnMappingFromId(virtualConnId);

    if(pMapping == NULL)
    {
        return gpHal_ResultInvalidParameter;
    }

    *nextEventTs = GP_WB_READ_EVENT_EXECUTION_TIME(GPHAL_ES_EVENT_NR_TO_START_OFFSET(pMapping->eventNr));

    pMapping->connId = masterConnId;
    pMapping->virtual = false;

    return gpHal_ResultSuccess;
}

gpHal_Result_t gpHal_UpdateConnection(UInt8 connId, UInt32 firstConnTs, gpHal_UpdateConnEventInfo_t* pInfo)
{
    gpHal_Result_t result;
    gpHal_BleConnectionContext_t* pMapping;
    UInt8 eType;
    UInt16 infoAddress;

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        GP_LOG_PRINTF("No mapping for connId: %x",0,connId);
        return gpHal_ResultInvalidParameter;
    }

    if(pMapping->running)
    {
        GP_LOG_PRINTF("Update failed. Conn %x still running",0,connId);
        return gpHal_ResultBusy;
    }

    gpHal_BlePopulateUpdateConnectionInfo(pInfo, pMapping);

    eType = GP_WB_READ_EVENT_TYPE_FIELD(GPHAL_ES_EVENT_NR_TO_START_OFFSET(pMapping->eventNr));
    infoAddress = GP_HAL_BLE_CONNECTION_TO_OFFSET_FROM_START(pMapping->connId);

    result = gpHal_BleRestartEvent(firstConnTs, eType, infoAddress, pMapping->eventNr);

    if(result == gpHal_ResultSuccess)
    {
        pMapping->running = true;
    }

    return result;
}

gpHal_Result_t gpHal_BleUpdateMasterConnection(UInt8 connId, UInt32 firstConnTs, gpHal_ConnEventInfo_t* pConnEventInfo)
{
    gpHal_Result_t result;
    gpHal_BleConnectionContext_t* pMapping;
    UInt16 infoAddress;

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        GP_LOG_PRINTF("No mapping found for connId %u", 0, connId);
        return gpHal_ResultInvalidParameter;
    }

    if(GP_WB_READ_EVENT_TYPE_FIELD(GPHAL_ES_EVENT_NR_TO_START_OFFSET(pMapping->eventNr)) != GPHAL_ENUM_EVENT_TYPE_VIRTUAL)
    {
        // Event number should correspond to a virtual one
        GP_LOG_PRINTF("Event number does not correspond to a virtual event", 0);
        return gpHal_ResultInvalidParameter;
    }

    gpHal_BleStopVirtualEvent(pMapping->eventNr);


        gpHal_BlePopulateConnEventInfo(pConnEventInfo, pMapping, firstConnTs);


    infoAddress = GP_HAL_BLE_CONNECTION_TO_OFFSET_FROM_START(pMapping->connId);

    // Use start event to make sure the cleanup time is updated
    result = gpHal_BleStartEvent(firstConnTs, GPHAL_ENUM_EVENT_TYPE_CONNECTION_M, infoAddress, pMapping->eventNr);

    if(result == gpHal_ResultSuccess)
    {
        gpSched_ScheduleEventArg(0, gpHal_BleOptimizeValidationParameters, (void*)pMapping);
    }

    return result;
}

gpHal_Result_t gpHal_BleUpdateChannelMap(UInt8 connId, gpHal_BleChannelMapHandle_t newChannelMapHandle)
{
    gpHal_Result_t result;
    UInt32 newTs;
    UInt8 eType;
    gpHal_BleConnectionContext_t* pMapping;
    gpHal_Address_t eventInfoAddress;
    UInt16 infoAddressCompressed;

    // Make sure there is an event that belongs to this connHandle
    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        GP_LOG_PRINTF("No mapping for connId: %x",0,connId);
        return gpHal_ResultInvalidParameter;
    }

    if(pMapping->running)
    {
        GP_LOG_PRINTF("Update failed. Conn %x still running",0,connId);
        return gpHal_ResultBusy;
    }

    // Check if the channel map handle is known
    if(!gpHal_BleIsChannelMapValid(newChannelMapHandle))
    {
        GP_LOG_PRINTF("Invalid channel map: %x",0,newChannelMapHandle);
        return gpHal_ResultInvalidParameter;
    }

    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(connId);
    infoAddressCompressed = GP_HAL_BLE_CONNECTION_TO_OFFSET_FROM_START(pMapping->connId);

    // Update to the new channel map handle
    GP_WB_WRITE_CONN_EV_INFO_CH_MAP_PTR(eventInfoAddress, GP_HAL_BLE_CHAN_MAP_TO_OFFSET_FROM_START(newChannelMapHandle));

    eType = GP_WB_READ_EVENT_TYPE_FIELD(GPHAL_ES_EVENT_NR_TO_START_OFFSET(pMapping->eventNr));
    newTs = gpHal_BleGetNextEventTs(eventInfoAddress, eType);

    result = gpHal_BleRestartEvent(newTs, eType, infoAddressCompressed, pMapping->eventNr);

    if(result == gpHal_ResultSuccess)
    {
        pMapping->running = true;
    }

    return result;
}

gpHal_Result_t gpHal_BleUpdatePhy(UInt8 connId, gpHal_BlePhyUpdateInfo_t* pInfo)
{
    gpHal_Result_t result;
    gpHal_BleConnectionContext_t* pMapping;
    UInt32 newTs;
    gpHal_Address_t eventInfoAddress;
    UInt8 eType;
    UInt16 infoAddressCompressed;

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        GP_LOG_PRINTF("gpHal_BleUpdatePhy(%i): no mapping",0,connId);
        return gpHal_ResultInvalidParameter;
    }

    if(pMapping->running)
    {
        GP_LOG_PRINTF("Update failed. Conn %x still running",0,connId);
        return gpHal_ResultBusy;
    }

    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId);

    if (gpHal_BleTxPhyInvalid == pInfo->phyIdTx)
    {
        pInfo->phyIdTx = gpHal_GetTxPhy(eventInfoAddress);
    }
    else
    {
        gpHal_SetTxPhy(eventInfoAddress, pInfo->phyIdTx);
    }

    if (gpHal_BleRxPhyInvalid == pInfo->phyIdRx)
    {
        pInfo->phyIdRx = gpHal_GetRxPhy(eventInfoAddress);
    }
    else
    {
        gpHal_SetRxPhy(eventInfoAddress, pInfo->phyIdRx);
    }

    GP_ASSERT_DEV_INT(gpHal_BleIsTxQueueEmpty(connId)); /* Tx should have been paused */

    eType = GP_WB_READ_EVENT_TYPE_FIELD(GPHAL_ES_EVENT_NR_TO_START_OFFSET(pMapping->eventNr));
    newTs = gpHal_BleGetNextEventTs(eventInfoAddress, eType);
    infoAddressCompressed = GP_HAL_BLE_CONNECTION_TO_OFFSET_FROM_START(pMapping->connId);

    result = gpHal_BleRestartEvent(newTs, eType, infoAddressCompressed, pMapping->eventNr);

    if(result == gpHal_ResultSuccess)
    {
        pMapping->running = true;
        gpSched_ScheduleEventArg(0, gpHal_BleOptimizeValidationParameters, (void*)pMapping);
    }

    return result;
}

UInt16 gpHal_BleGetCurrentConnEventCount(UInt8 connId)
{
    UInt16 currentEventCount = 0;
    gpHal_BleConnectionContext_t* pMapping;

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping != NULL)
    {
        gpHal_Address_t eventInfoAddress;

        eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(connId);

        do
        {
            // Make sure we are not busy when reading the event counter
            GP_DO_WHILE_TIMEOUT_ASSERT(GP_WB_READ_CONN_EV_INFO_METRICS_UPDATE_BUSY(eventInfoAddress), GPHAL_BLE_CONNECTION_METRICS_UPDATE_TIMEOUT_US);

            // Clear flag (should be an atomic write)
            GP_WB_WRITE_CONN_EV_INFO_METRICS_UPDATE_CHANGE(eventInfoAddress, 0);

            // Read out event counter
            currentEventCount = GP_WB_READ_CONN_EV_INFO_EVENT_CNT(eventInfoAddress);
        } while(GP_WB_READ_CONN_EV_INFO_METRICS_UPDATE_BUSY(eventInfoAddress) || GP_WB_READ_CONN_EV_INFO_METRICS_UPDATE_CHANGE(eventInfoAddress));
    }
    else
    {
        GP_LOG_PRINTF("No mapping for connId: %x",0,connId);
        // do not assert: the calling procedure will find out that the connection has gone
    }

    return currentEventCount;
}

gpHal_Result_t gpHal_BleSetLastScheduledConnEventCount(UInt8 connId, UInt16 lastEventCount)
{
    gpHal_BleConnectionContext_t* pMapping;
    gpHal_Address_t eventInfoAddress;

    GP_LOG_PRINTF("SetLastScheduledConnEventCount %d ",2, lastEventCount);

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        return gpHal_ResultInvalidParameter;
    }

    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId);

    // Check if we haven't already a request pending
    if(GP_WB_READ_CONN_EV_INFO_LAST_SCH_EVENT_CNT_VALID(eventInfoAddress))
    {
        GP_LOG_SYSTEM_PRINTF("We already have a last conn event pending",0);
        return gpHal_ResultBusy;
    }

    GP_WB_WRITE_CONN_EV_INFO_LAST_SCH_EVENT_CNT(eventInfoAddress, lastEventCount);
    GP_WB_WRITE_CONN_EV_INFO_LAST_SCH_EVENT_CNT_VALID(eventInfoAddress, 1);

    return gpHal_ResultSuccess;
}

gpHal_Result_t gpHal_BleStopLastScheduledConnEventCount(UInt8 connId)
{
    gpHal_BleConnectionContext_t* pMapping;
    gpHal_Address_t eventInfoAddress;

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        GP_LOG_SYSTEM_PRINTF("Stop last sched conn %x: no mapping",0,connId);
        return gpHal_ResultInvalidParameter;
    }

    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(connId);

    GP_WB_WRITE_CONN_EV_INFO_LAST_SCH_EVENT_CNT_VALID(eventInfoAddress, 0);

    return gpHal_ResultSuccess;
}

gpHal_Result_t gpHalBle_PauseConnectionEvent(gpHal_BleConnectionContext_t *pMapping)
{
    gpHal_Result_t result;

    result = gpHal_BleStopEvent(pMapping->eventNr);

    if(result == gpHal_ResultSuccess)
    {
        pMapping->running = false;
    }

    return result;
}

gpHal_Result_t gpHal_BlePauseConnectionEvent(UInt8 connId)
{
    gpHal_BleConnectionContext_t *pMapping;

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        GP_LOG_SYSTEM_PRINTF("Pause connection %x: no mapping",0,connId);
        return gpHal_ResultInvalidParameter;
    }

    return gpHalBle_PauseConnectionEvent(pMapping);
}

gpHal_Result_t gpHal_BleStopConnection(UInt8 connId)
{
    gpHal_Result_t result;
    gpHal_BleConnectionContext_t* pMapping;

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        GP_LOG_SYSTEM_PRINTF("Stop connection %x: no mapping",0,connId);
        return gpHal_ResultInvalidParameter;
    }

    result = gpHalBle_PauseConnectionEvent(pMapping);

    if(result == gpHal_ResultSuccess)
    {
        gpSched_UnscheduleEventArg(gpHal_BleOptimizeValidationParameters, pMapping);
        gpHal_BlePurgeAllPendingPdus(pMapping);
        gpHal_BleRemoveConnIdMapping(pMapping);
    }

    return result;
}

gpHal_Result_t gpHal_BleSetFlowCtrl(UInt16 connMask)
{
    UInt8 connId;

    for (connId=0;connId<GPHAL_BLE_MAX_NR_OF_SUPPORTED_CONNECTIONS;++connId)
    {
        gpHal_BleConnectionContext_t *pMapping;
        pMapping = gpHal_BleGetConnMappingFromId(connId);
        if(pMapping != NULL)
        {
            gpHal_Address_t offset = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId);
            UInt8 enabled = (connMask>>connId)&0x01;
            GP_WB_WRITE_CONN_EV_INFO_RX_FLOW_CTRL_FLAG(offset, enabled);
        }
    }

    return gpHal_ResultSuccess;
}

gpHal_Result_t gpHal_SetSuspendConnectionEvent(UInt8 connId, Bool suspend)
{
    gpHal_BleConnectionContext_t *pMapping;
    gpHal_Address_t eventInfoAddress;

    GP_LOG_PRINTF("SetSuspendConnectionEvent %d %d",2, connId, suspend);

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        return gpHal_ResultInvalidParameter;
    }

    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId);

    GP_WB_WRITE_CONN_EV_INFO_SUSPEND_EVENT(eventInfoAddress, suspend);

    return gpHal_ResultSuccess;
}

// Find the length of the longest packet (incl preamble, excl crc) in the queue
UInt16 gpHal_BleGetMaxQueuedPacketLength(UInt8 connId)
{
    gpHal_BleConnectionContext_t* pMapping;
    gpHal_Address_t eventInfoAddress;
    UInt8 queueWritePointer;
    UInt8 QueueIndex;
    UInt16 maxQueuedLength = 0;

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        return 0;
    }

    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId);
    queueWritePointer = GP_WB_READ_CONN_EV_INFO_TX_QUEUE_WR_PTR(eventInfoAddress);
    QueueIndex = GP_WB_READ_CONN_EV_INFO_TX_QUEUE_RD_PTR(eventInfoAddress);

    while (QueueIndex != queueWritePointer)
    {
        UInt8 queuedPbmHandle = GP_HAL_READ_REG(eventInfoAddress + GP_WB_CONN_EV_INFO_TX_QUEUE_ADDRESS + QueueIndex);
        gpHal_Address_t optsBase = GP_HAL_PBM_ENTRY2ADDR_OPT_BASE(queuedPbmHandle);
        UInt8 queuedPbmLength = GP_WB_READ_PBM_BLE_FORMAT_T_FRAME_LEN(optsBase);

        if (queuedPbmLength > maxQueuedLength)
        {
            maxQueuedLength = queuedPbmLength;
        }
        QueueIndex = GP_HAL_BLE_QUEUE_CIRC_INC(QueueIndex);
    }

    return maxQueuedLength;
}

gpHal_Result_t gpHal_BleAddPduToQueue(UInt8 connId, gpPd_Loh_t pdLoh, gpHal_BleTxOptions_t* pTxOptions)
{
    UInt8 pbmHandle;
    UInt8 queueWritePointer;
    UInt8 queueReadPointer;
    gpHal_BleConnectionContext_t* pMapping;
    gpHal_Address_t eventInfoAddress;

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        return gpHal_ResultInvalidHandle;
    }

    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId);

    // AD-1695: TxIfTooLate should be set to zero for a master. For a slave, preferably to one, but it is not always
    // possible to do as the empty PDU is potentially shared between a master and slave.
    pbmHandle = gpHal_BlePdToPbm(pdLoh, false);


    if(!GP_HAL_CHECK_PBM_VALID(pbmHandle))
    {
        return gpHal_ResultInvalidParameter;
    }

    queueWritePointer = GP_WB_READ_CONN_EV_INFO_TX_QUEUE_WR_PTR(eventInfoAddress);
    queueReadPointer = GP_WB_READ_CONN_EV_INFO_TX_QUEUE_RD_PTR(eventInfoAddress);

    if(GP_HAL_BLE_QUEUE_CIRC_INC(queueWritePointer) == queueReadPointer)
    {
        // Do not add when queue is full
        return gpHal_ResultBusy;
    }

    // Note: guard time is managed by the NRT subsystem, guard updates must occur before queueing the new packet

    GP_HAL_WRITE_REG(eventInfoAddress + GP_WB_CONN_EV_INFO_TX_QUEUE_ADDRESS + queueWritePointer, pbmHandle);

#ifdef GP_COMP_TXMONITOR
    gpTxMonitor_AnnounceTxStart();
#endif //GP_COMP_TXMONITOR

    GP_WB_WRITE_CONN_EV_INFO_TX_QUEUE_WR_PTR(eventInfoAddress, GP_HAL_BLE_QUEUE_CIRC_INC(queueWritePointer));

    if(GP_WB_READ_CONN_EV_INFO_SLAVE_LATENCY_ASLEEP(eventInfoAddress))
    {
        gpHal_BleWakeupEvent(pMapping->eventNr);
    }

    return gpHal_ResultSuccess;
}

Bool gpHal_BleIsTxQueueEmpty(UInt8 connId)
{
    gpHal_BleConnectionContext_t* pMapping;
    gpHal_Address_t eventInfoAddress;

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        GP_LOG_PRINTF("no mapping",0);
        return false;
    }

    GP_ASSERT_DEV_INT(pMapping);

    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(connId);

    return (GP_WB_READ_CONN_EV_INFO_TX_QUEUE_RD_PTR(eventInfoAddress) == GP_WB_READ_CONN_EV_INFO_TX_QUEUE_WR_PTR(eventInfoAddress));
}

UInt32 gpHal_BleGetLastRxTimestamp(UInt8 connId)
{
    gpHal_BleConnectionContext_t* pMapping;
    UInt32 ts = 0xFFFFFFFF;

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping != NULL)
    {
        ts = GP_WB_READ_CONN_EV_INFO_T_LAST_CORRELATION(GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(connId));
    }

    return ts;
}

gpHal_Result_t gpHal_BleGetNextExpectedEventTimestamp(UInt8 connId, UInt32 *ts)
{
    gpHal_BleConnectionContext_t* pMapping;

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        return gpHal_ResultInvalidParameter;
    }

    *ts = GP_WB_READ_CONN_EV_INFO_T_NEXT_EXP_ANCHOR_POINT(GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(connId));
    return gpHal_ResultSuccess;
}

gpHal_Result_t gpHal_BleGetUnmappedChannelPtr(UInt8 connId, UInt8 *unmappedChannelPtr)
{
    gpHal_BleConnectionContext_t* pMapping;

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        return gpHal_ResultInvalidParameter;
    }

    // Note that the UnmappedChannelPtr is the value of the last scheduled connection event
    // : the RT subsystem calculates the new value at the beginning of each connection event
    *unmappedChannelPtr = GP_WB_READ_CONN_EV_INFO_UNMAPPED_CH_PTR(GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(connId));
    return gpHal_ResultSuccess;
}

UInt8 gpHal_BleGetNrOfAvailableLinkQueueEntries(UInt8 connId)
{
    gpHal_BleConnectionContext_t* pMapping;
    UInt8 rdPointer;
    UInt8 wdPointer;
    UInt8 nrOfAvailableEntries = 0;

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        return 0;
    }

    rdPointer = GP_WB_READ_CONN_EV_INFO_TX_QUEUE_RD_PTR(GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId));
    wdPointer = GP_WB_READ_CONN_EV_INFO_TX_QUEUE_WR_PTR(GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId));

    if(rdPointer > wdPointer)
    {
        nrOfAvailableEntries = GPHAL_BLE_MAX_NR_OF_QUEUED_PBMS - (GPHAL_BLE_TX_QUEUE_SIZE - (rdPointer - wdPointer));
    }
    else
    {
        nrOfAvailableEntries = GPHAL_BLE_MAX_NR_OF_QUEUED_PBMS - (wdPointer - rdPointer);
    }

    return nrOfAvailableEntries;
}

void gpHal_BleRegisterCallbacks(gpHal_BleCallbacks_t* pCallbacks)
{
    GP_ASSERT_DEV_EXT(pCallbacks != NULL);


    MEMCPY(&gpHal_BleCallbacks, pCallbacks, sizeof(gpHal_BleCallbacks_t));
}


gpHal_Result_t gpHal_BleSetSlaveLatency(UInt8 connId, UInt16 slaveLatency)
{
    gpHal_BleConnectionContext_t* pMapping;
    gpHal_Address_t eventInfoAddress;

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        return gpHal_ResultInvalidHandle;
    }

    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(connId);

    GP_WB_WRITE_CONN_EV_INFO_SLAVE_LATENCY(eventInfoAddress, slaveLatency);

    gpHal_BleUpdateSleepSettings(eventInfoAddress, slaveLatency);

    if(slaveLatency == 0)
    {
        if(GP_WB_READ_CONN_EV_INFO_SLAVE_LATENCY_ASLEEP(eventInfoAddress))
        {
            gpHal_BleWakeupEvent(pMapping->eventNr);
        }
    }

    return gpHal_ResultSuccess;
}




gpHal_Result_t gpHal_BleGetConnectionMetrics(UInt8 connId, gpHal_ConnEventMetrics_t* pMetrics)
{
    gpHal_BleConnectionContext_t* pMapping;
    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        return gpHal_ResultInvalidParameter;
    }

    gpHal_Address_t eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(connId);

    do
    {
        // Make sure we are not busy when reading the metrics
        GP_DO_WHILE_TIMEOUT_ASSERT(GP_WB_READ_CONN_EV_INFO_METRICS_UPDATE_BUSY(eventInfoAddress), GPHAL_BLE_CONNECTION_METRICS_UPDATE_TIMEOUT_US);

        // Clear flag (should be an atomic write)
        GP_WB_WRITE_CONN_EV_INFO_METRICS_UPDATE_CHANGE(eventInfoAddress, 0);

        // Read out metrics
        pMetrics->eventCounterNext = GP_WB_READ_CONN_EV_INFO_EVENT_CNT(eventInfoAddress);
        pMetrics->windowDuration = GP_WB_READ_CONN_EV_INFO_WINDOW_DURATION(eventInfoAddress);
        pMetrics->tsLastValidPacketReceived = GP_WB_READ_CONN_EV_INFO_T_LAST_CORRELATION(eventInfoAddress);
        pMetrics->tsLastPacketReceived = GP_WB_READ_CONN_EV_INFO_T_LAST_PEER_PACKET(eventInfoAddress);
        pMetrics->nextAnchorTime = GP_WB_READ_CONN_EV_INFO_T_NEXT_EXP_ANCHOR_POINT(eventInfoAddress);
        pMetrics->nrNoRXEvents = GP_WB_READ_CONN_EV_INFO_NR_NO_RX_EVENTS(eventInfoAddress);
        pMetrics->anchorTimeLastRxEvent = GP_WB_READ_CONN_EV_INFO_T_ANCHOR_POINT_LAST_RX_EVENT(eventInfoAddress);
        pMetrics->eventCounterLastRx = GP_WB_READ_CONN_EV_INFO_EVENT_CNT_LAST_RX(eventInfoAddress);

    } while(GP_WB_READ_CONN_EV_INFO_METRICS_UPDATE_BUSY(eventInfoAddress) || GP_WB_READ_CONN_EV_INFO_METRICS_UPDATE_CHANGE(eventInfoAddress));

    // This metric is not always updated while busy is set, so read it outside the polling loop
    pMetrics->nrOfConsecSkippedEvents = GP_WB_READ_CONN_EV_INFO_NR_CONSEC_SKIPPED_EVENTS(eventInfoAddress);

    return gpHal_ResultSuccess;
}

gpHal_Result_t gpHal_SetConnectionPriority(UInt8 connId, UInt8 priority)
{
    gpHal_BleConnectionContext_t* pMapping;
    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        return gpHal_ResultInvalidHandle;
    }

    gpHal_Address_t eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId);
    GP_WB_WRITE_CONN_EV_INFO_PRIORITY(eventInfoAddress, priority);
    return gpHal_ResultSuccess;
}

gpHal_Result_t gpHal_GetConnectionGuardTime(UInt8 connId, UInt32 *pGuard)
{
    gpHal_BleConnectionContext_t* pMapping;
    gpHal_Address_t eventInfoAddress;

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL || pGuard == NULL)
    {
        return gpHal_ResultInvalidHandle;
    }

    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId);

    *pGuard = GP_WB_READ_CONN_EV_INFO_GUARD_TIME(eventInfoAddress) - T_PROCESSING;

    return gpHal_ResultSuccess;
}

gpHal_Result_t gpHal_SetConnectionGuardTime(UInt8 connId, UInt32 MaxDurationPacketPair, UInt32 ExtraIdleTime, UInt16 slaveLatency)
{
    gpHal_BleConnectionContext_t* pMapping;
    gpHal_Address_t eventInfoAddress;

    pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        return gpHal_ResultInvalidHandle;
    }

    eventInfoAddress = (gpHal_Address_t)GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(pMapping->connId);

    // In a connection event, there must be sufficient time left to finish a last transaction (a packet pair) : MaxDurationPacketPair
    // This packet pair is guaranteed to finish before the beginning of the next connection event (i.e. before the RT "processing" of the next connection event)
    // In addition, we can create some idle time in connection events : ExtraIdleTime
    // ExtraIdleTime is typcially derived from the maximum allowed duration of the connection events
    // Note that this additional idle time does overlap with the RT "processing" time

    if (ExtraIdleTime > T_PROCESSING)
    {
        ExtraIdleTime = ExtraIdleTime - T_PROCESSING;
    }
    else
    {
        ExtraIdleTime = 0;
    }

    GP_WB_WRITE_CONN_EV_INFO_GUARD_TIME(eventInfoAddress, MaxDurationPacketPair + ExtraIdleTime + T_PROCESSING);

    // This function reads back from the info structure, so should be executed after guard has been written
    gpHal_BleUpdateSleepSettings(eventInfoAddress, slaveLatency);

    return gpHal_ResultSuccess;
}


UInt8 gpHal_PhyToBleChannel(UInt8 phyChannel)
{
    UInt8 fllChannel;
    UIntLoop i;

    // Go through fll channel, as table exists in ble ev mgr
    fllChannel = GP_HAL_CONVERT_BLEPHY_TO_FLL_CHANNEL(phyChannel);

    for(i = 0; i < GP_HAL_NR_OF_BLE_CHANNELS; i++)
    {
        if(GP_HAL_CONVERT_BLE_TO_FLL_CHANNEL(i) == fllChannel)
        {
            return i;
        }
    }

    // Not a valid channel provided
    GP_ASSERT_DEV_INT(false);
    return 0xFF;
}

UInt8 gpHal_BleGetBleChannelIndex(UInt8 pbmEntry)
{
    return GP_HAL_CONVERT_PHY_TO_BLE_CHANNEL(
             GP_HAL_CONVERT_FLL_TO_BLEPHY_CHANNEL(
               GP_HAL_CONVERT_MACPHY_TO_FLL_CHANNEL(
                 gpHal_GetRxedChannel(pbmEntry)
               )
             )
           );
}

UInt16 gpHal_BleCalcWaveLengthInmm(UInt8 BleChannelIndex)
{
    UInt8 FLLChannel = GP_HAL_CONVERT_BLE_TO_FLL_CHANNEL(BleChannelIndex);
    UInt32 FreqMHz = GPHAL_BLE_PHY_CHAN_BASE_FREQ + GP_HAL_CONVERT_FLL_TO_BLEPHY_CHANNEL(FLLChannel) * GPHAL_BLE_PHY_CHAN_BW;

    return (UInt16)((GPHAL_BLE_PHY_SPEEDC + (FreqMHz>>1)) / FreqMHz);
}

Bool gpHal_BleHasFullBgScanSupport(void)
{
    return true;
}

void gpHal_BleSetMaxRxPayloadLength(UInt8 payloadLength)
{
    GP_WB_WRITE_BLE_MGR_MAX_ALLOWED_PDU_PAYLOAD_LENGTH(payloadLength);
}


gpHal_Result_t gpHal_BleSetMultiStandard(Bool enable)
{
    GP_WB_WRITE_BLE_MGR_MS_ENABLED(enable);
    return gpHal_ResultSuccess;
}

Bool gpHal_BleGetMultiStandard(void)
{
    return (GP_WB_READ_BLE_MGR_MS_ENABLED() == 1);
}

UInt8 gpHal_Ble_GetAdvAntennaSwitching(void)
{
    /* We don't care about GP_HAL_DIVERSITY_MULTISTANDARD_LISTENING_MODE here because advertising
     * only RX on channels which it transmits advertisements on. */
    if(!gpHal_GetRxAntennaDiversity())
    {
        return GP_HAL_BLE_ANTENNASWITCHINGDISABLED;
    }
    else
    {
        return GP_HAL_BLE_ANTENNASWITCHINGENABLED;
    }
}

UInt8 gpHal_Ble_GetAdvAntenna(void)
{
    if( !gpHal_GetRxAntennaDiversity() )
    {
        return gpHal_GetBleAntenna();
    }
    else
    {
        return gpHal_AntennaSelection_Ant0;
    }
}

Bool gpHal_BleIsChanSelAlgo2Used(UInt8 connId)
{
    gpHal_BleConnectionContext_t* pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        return false;
    }

    return GP_WB_READ_CONN_EV_INFO_USE_CHAN_SEL_ALGO2(GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(connId));
}

UInt8 gpHal_BleGetHopIncrement(UInt8 connId)
{
    gpHal_BleConnectionContext_t* pMapping = gpHal_BleGetConnMappingFromId(connId);

    if(pMapping == NULL)
    {
        return 0;
    }

    return GP_WB_READ_CONN_EV_INFO_HOP_INCREMENT(GP_HAL_BLE_CONNECTION_TO_EVENT_INFO_ADDRESS(connId));
}

UInt8 gpHal_BleGetRtBleMgrVersion(void)
{
    return gpHal_GetRtSystemVersion(gpHal_RtSubSystem_BleMgr);
}
