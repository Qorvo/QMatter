/*
 * Copyright (c) 2016, GreenPeak Technologies
 * Copyright (c) 2017-2021, Qorvo Inc
 *
 *   gpPTC
 *   Implementation of gpPTC
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
#define GP_COMPONENT_ID GP_COMPONENT_ID_PTC
//#define GP_PTC_COMM_ID GP_COM_COMM_ID_RF
// #define GP_LOCAL_LOG

#include "gpTest.h"
#ifdef GP_PTC_ENABLE_BLE
#include "gpTestBle.h"
#endif
#include "gpLog.h"
#include "gpCom.h"
#include "gpSched.h"
#include "gpVersion.h"
#include "gpPoolMem.h"
#include "gpBaseComps.h"
#include "gpPTC_server.h"
#include "gpPTC.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define PTC_VERSION  1
#define UINT16_MAXVALUE 65535

#define PTC_PAN_ID                    (UInt16)0xCAFE
#define PTC_RFCOMMUNICATION_CHANNEL    17
#define PTC_SHORT_ADDRESS             (UInt16)0xFC7C

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct _ModeParams
{
    UInt8 clientid;
    UInt32 exectime;
    UInt8 modeid;
    gpPTC_ModeExecution_t onoff;
    UInt8 numberofextraargs;
    gpPTC_Parameter_t* parameters;

} gpPTC_ModeParams_t;


//@}
/** @name gpPTC_BLE_DataRate_t
//@{
 */
/** @brief 1 MB/s (default) */
#define GPPTC_BLE_DATARATE_1MBS           1
/** @brief 2 MB/s */
#define GPPTC_BLE_DATARATE_2MBS           2
/*
 *  This typedef defines the possible BLE Data Rates.
 *  @typedef gpPTC_BLE_DataRate_t
 */
typedef UInt8 gpPTC_BLE_DataRate_t;


/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

static gpTest_ContinuousWaveMode_t gpPTC_CWMode = 0;
static UInt8 gpPTC_MapClockIOPin_Enabled = 0;
static Bool gpPTC_IsBusy = false;
static UInt32 gpPTC_RRAddress = 0;
static UInt32 gpPTC_WRAddress = 0;
static UInt8 gpPTC_WRValue = 0;
static UInt32 gpPTC_WRBitMask = 0;
static Bool gpPTC_PrintRXPackets = false;
static UInt16 gpPTC_PacketCount =10;
static UInt16 gpPTC_ScanCount =0;
static UInt16 gpPTC_PacketInterValInMS =5;
static UInt16 gpPTC_ScanInterValInMS = 3;
static UInt16 gpPTC_PacketLength =5;

static UInt8* gpPTC_pTxData = NULL;
static UInt8 gpPTC_pTxDataLength = 0;

static Bool gpPTC_OldRxState;
static Int32 gpPTC_AllRSSI=0;
static UInt32 gpPTC_AllLQI=0;
static UInt16 gpPTC_RXPacketCount=0;
//static UInt16 gpPTC_RXRetryCount=0;
static UInt16 gpPTC_TXPacketCount=0;
static UInt8 gpPTC_DirectTestMode_PacketType = 0;
static gpPTC_BLE_DataRate_t gpPTC_BLE_DataRateValue = GPPTC_BLE_DATARATE_1MBS;

static UInt8 gpPTC_MapIOClockPin = 0;
static gpHal_SleepMode_t gpPTC_MapIOClockType = 0;
static UInt8 gpPTC_DBG_Sel_OrigValue = 0;
static UInt8 gpPTC_DBG_En_OrigValue = 0;
static gpHal_SleepMode_t gpPTC_SleepMode_OrigValue = 0;
static UInt8 gpPTC_EDScanBusy = false;
static UInt8 gpPTC_RXMultiStandard = gpPTC_Disabled;
static UInt8 gpPTC_RXHighSensitivity = gpPTC_Disabled;
static UInt8 gpPTC_RXMultichannel = gpPTC_Disabled;

static struct {
    Bool state;
    gpPTC_PDMClkSrc_t              src;
    UInt32                         freqHz;
    UInt8                          gpio;
} gpPTC_PDMClkCfg =
{false, gpPTC_PDMClkSrc_2M, 2000, 3};

static gpPTC_Attribute_t gpPTC_attributeBuffer[PTC_ATTRIBUTES_MAX_NUMBER] ;

static Bool UseOTAProtocol = true;

GP_COMPILE_TIME_VERIFY((gpHal_CollisionAvoidanceModeNoCCA == gpPTC_CSMAMode_NoCCA) &&
                       (gpHal_CollisionAvoidanceModeCCA   == gpPTC_CSMAMode_CCA) &&
                       (gpHal_CollisionAvoidanceModeCSMA  == gpPTC_CSMAMode_CSMA));

/*GP_COMPILE_TIME_VERIFY((gpHal_SleepModeEvent == gpPTC_SleepModeEvent) &&
                       (gpHal_SleepModeRC   == gpPTC_SleepModeRC) &&
                       (gpHal_SleepMode16MHz  == gpPTC_SleepMode16MHz));
                       //(gpHal_SleepModeReset  == gpPTC_SleepModeReset)); */

GP_COMPILE_TIME_VERIFY((gpPTC_RF4CE == gpTest_PhyModeMac) &&
                       (gpPTC_BLE == gpTest_PhyModeBle));

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static void gpPTC_mainReEnableCw(gpTest_ContinuousWaveMode_t CWState);
static void gpPTC_RestoreAttributes(Bool skipPhyMode);
static Bool gpPTC_CheckCompatibleHWVersion(void);
static gpPTC_Result_t gpPTC_ExecAttribute(gpPTC_Attribute_t attribute);
static void gpPTC_SetModeRequestFromStruct(void* pArg);
static void gpPTC_SetModeRequestToStruct(UInt8 clientID, UInt8 modeID, UInt32 exectime, gpPTC_ModeExecution_t OnOff, UInt8 numberOfExtraParameters, gpPTC_Parameter_t* parameters, gpPTC_ModeParams_t* pParam);
static void gpPTC_enableCWMode_M(void);
static void gpPTC_enableCWMode_U(void);


#if defined(GP_DIVERSITY_GPHAL_K8E)
// move this to seperate header file and make implementation for chip variants ?
// or move this to gpTest ?
static gpPTC_Result_t restoreClkToGPIORegisters(void);
static gpPTC_Result_t mapClkToGPIORegisters(void);
#endif

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

Bool gpPTC_CheckCompatibleHWVersion(void)
{
#ifdef GPHAL_CHIP_ID_K8A
    // if (gpHal_GetChipVersion() < 4)
    // {
    //   GP_LOG_PRINTF("Id %i V %i",0,GPHAL_CHIP_ID_K8A,gpHal_ChipVersion);
    //   return false;
    // }
#endif
  // for now : allow all other variants ?
  return true;
}

void gpPTC_enableCWMode_M(void)
{
    if (gpTest_GetPhyMode() == gpTest_PhyModeBle
        && gpPTC_BLE_DataRateValue == GPPTC_BLE_DATARATE_2MBS)
    {
        gpTest_SetContinuousWaveMode( CW_BLE_HDRMODULATED );
    }
    else {
        gpTest_SetContinuousWaveMode( CW_MODULATED );
    }
    GP_LOG_PRINTF("CW M ON",0);
}

void gpPTC_enableCWMode_U(void)
{
    gpTest_SetContinuousWaveMode( CW_UNMODULATED );
    GP_LOG_PRINTF("CW U ON",0);
}

void gpPTC_mainReEnableCw(gpTest_ContinuousWaveMode_t CWState)
{
    void_func contWaveFunc = gpPTC_enableCWMode_U;
    switch(CWState)
    {
        case CW_MODULATED:
        {
            contWaveFunc = gpPTC_enableCWMode_M;
            break;
        }
        case CW_UNMODULATED:
        {
            contWaveFunc = gpPTC_enableCWMode_U;
            break;
        }
        case CW_OFF :
        {
          // if it was already off, do nothing
          return;
        }
        default:
        {
          GP_ASSERT_DEV_INT(false);
          return;
        }
    }

    gpSched_UnscheduleEvent(contWaveFunc);
      gpSched_ScheduleEvent( 0, contWaveFunc );
}

gpPTC_Result_t gpPTC_ExecAttribute(gpPTC_Attribute_t attribute)
{
    if (!gpPTC_CheckCompatibleHWVersion())
    {
    return gpPTC_ResultInvalidChip;
    }
    gpPTC_Result_t result = gpPTC_ResultSuccess;
    GP_LOG_PRINTF("SetAttr 0x%x to 0x%lx",0,attribute.id, attribute.value);
    switch (attribute.id)
    {
    case gpPTC_AttributeChannel :
    {
        gpTest_ContinuousWaveMode_t CWState = gpTest_GetContinuousWaveMode();
        if (CWState != CW_OFF) {
            gpTest_SetContinuousWaveMode(CW_OFF);
        }
        result = gpTest_SetChannel((UInt8)attribute.value) ;
        if (result != gpPTC_ResultSuccess)
        {
            result = gpPTC_ResultSetAttributeFailed;
        }
        else
        {
            gpPTC_mainReEnableCw(CWState);
        }
        break;
    }
    case  gpPTC_AttributeTXPower :
    {
        gpTest_ContinuousWaveMode_t CWState = gpTest_GetContinuousWaveMode();
        if (CWState != CW_OFF) {
            gpTest_SetContinuousWaveMode(CW_OFF);
        }
        gpTest_SetTxPower(attribute.value);
        result = gpPTC_ResultSuccess;
        Int8 powerset = gpTest_GetTxPower();

        if (attribute.value != powerset)
        {
            GP_LOG_PRINTF("Set %i, expected : %i",0,powerset,(int)attribute.value);
            result = gpPTC_ResultSetAttributeFailed;
        }
        else
        {
            gpPTC_mainReEnableCw(CWState);
        }
        break;
    }
    case gpPTC_AttributeAntenna :
    {
#if defined(GP_BSP_HAS_DIFFERENTIAL_SINGLE_ANTENNA)
#if GP_BSP_HAS_DIFFERENTIAL_SINGLE_ANTENNA()
        result = gpPTC_ResultUnsupported;
        break;
#endif
#endif
        gpTest_ContinuousWaveMode_t CWState = gpTest_GetContinuousWaveMode();
        if (CWState != CW_OFF) {
            gpTest_SetContinuousWaveMode(CW_OFF);
        }
        gpTest_SetAntennaDiversity( false );
        result = gpTest_SetAntenna((UInt8)attribute.value);
        if (result != gpPTC_ResultSuccess)
        {
            result = gpPTC_ResultSetAttributeFailed;
        }
        else
        {
            gpPTC_mainReEnableCw(CWState);
        }
        break;
    }
    case gpPTC_AttributeSleepMode :
    {
        UInt32 frequencymHz;
        gpTest_SleepClockMeasurementStatus_t status = gpTest_GetMeasuredSleepClockFrequency(attribute.value, &frequencymHz);
        if ( (status != gpHal_SleepClockMeasurementStatusStable) || (status == gpHal_SleepClockMeasurementStatusNotStarted) )
            result = gpPTC_ResultSetAttributeFailed;
        else
            gpTest_SetSleepMode(attribute.value);
        break;
    }
    case gpPTC_AttributeAntennaDiversity:
    {
#if defined(GP_BSP_HAS_DIFFERENTIAL_SINGLE_ANTENNA)
#if GP_BSP_HAS_DIFFERENTIAL_SINGLE_ANTENNA()
        result = gpPTC_ResultUnsupported;
        break;
#endif
#endif
        gpTest_SetAntennaDiversity(attribute.value == 1 ? true : false);
        if (attribute.value != gpTest_GetAntennaDiversity())
        {
            GP_LOG_PRINTF("Set %i, expected : %i",0,gpTest_GetAntennaDiversity(),(int)attribute.value);
            result = gpPTC_ResultSetAttributeFailed;
        }
        else
        {
            result = gpPTC_ResultSuccess;
        }
        break;
    }
    case gpPTC_AttributeCWMode :
    {
        if (gpTest_GetContinuousWaveMode() != CW_OFF)
            gpTest_SetContinuousWaveMode( CW_OFF );
        gpPTC_CWMode = ((UInt8)attribute.value == 1) ? CW_UNMODULATED : CW_MODULATED;
        result = gpPTC_ResultSuccess;
        break;
    }
    case gpPTC_AttributeCSMAMode :
    {
        gpTest_SetCollisionAvoidanceModeToUse(attribute.value);
        result = gpPTC_ResultSuccess;
        break;
    }
    case gpPTC_AttributeMAXBE :
    {
        gpTest_SetMaxBE((UInt8) attribute.value);
        if (gpTest_GetMaxBE() == (UInt8) attribute.value)
        {
            result = gpPTC_ResultSuccess;
        }
        else
        {
            result = gpPTC_ResultSetAttributeFailed;
        }
        break;
    }
    case gpPTC_AttributeMINBE :
    {
        gpTest_SetMinBE((UInt8) attribute.value);
        if (gpTest_GetMinBE() == (UInt8) attribute.value)
        {
            result = gpPTC_ResultSuccess;
        }
        else
        {
            result = gpPTC_ResultSetAttributeFailed;
        }
        break;
    }
    case gpPTC_AttributeMaxCSMABackoff :
    {
        gpTest_SetMaxCSMABackoffs((UInt8) attribute.value);
        if (gpTest_GetMaxCSMABackoffs() == (UInt8) attribute.value)
        {
            result = gpPTC_ResultSuccess;
        }
        else
        {
            result = gpPTC_ResultSetAttributeFailed;
        }
        break;
    }
    case gpPTC_AttributeMaxMACRetries :
    {
        gpTest_SetNumberOfRetries((UInt8) attribute.value);
        if (gpTest_GetNumberOfRetries() == (UInt8) attribute.value)
        {
            result = gpPTC_ResultSuccess;
        }
        else
        {
            result = gpPTC_ResultSetAttributeFailed;
        }
        break;
    }
    case gpPTC_AttributePanID :
    {
        gpTest_SetPanId((UInt16) attribute.value,gpHal_SourceIdentifier_0);

        if (gpTest_GetPanId(gpHal_SourceIdentifier_0) == (UInt16) attribute.value)
        {
            result = gpPTC_ResultSuccess;
        }
        else
        {
            result = gpPTC_ResultSetAttributeFailed;
        }
        break;
    }
    case gpPTC_AttributeShortAddress :
    {
        gpTest_SetShortAddress((UInt16) attribute.value,gpHal_SourceIdentifier_0);
        if (gpTest_GetShortAddress(gpHal_SourceIdentifier_0) == (UInt16) attribute.value)
        {
            result = gpPTC_ResultSuccess;
        }
        else
        {
            result = gpPTC_ResultSetAttributeFailed;
        }
        break;
    }

    case gpPTC_AttributePacketInPacket :
    {
        gpTest_SetPacketInPacketMode((UInt8) attribute.value);
        if (attribute.value != gpTest_GetPacketInPacketMode())
        {
            GP_LOG_PRINTF("Set %i, expected : %i",0,gpTest_GetPacketInPacketMode(),(int)attribute.value);
            result = gpPTC_ResultSetAttributeFailed;
        }
        else
        {
            result = gpPTC_ResultSuccess;
        }
        break;
    }
    case gpPTC_AttributePromiscuousMode :
    {
        gpTest_SetPromiscuousMode((Bool) attribute.value);
        if (attribute.value != gpTest_GetPromiscuousMode())
        {
            GP_LOG_PRINTF("Set %i, expected : %i",0,gpTest_GetPromiscuousMode(),(int)attribute.value);
            result = gpPTC_ResultSetAttributeFailed;
        }
        else
        {
            result = gpPTC_ResultSuccess;
        }
        break;
    }
    case  gpPTC_AttributeReadRegisterAddress :
    {
        gpPTC_RRAddress = attribute.value;
        result = gpPTC_ResultSuccess;
        break;
    }
    case gpPTC_AttributeWriteRegisterAddress :
    {
        gpPTC_WRAddress = attribute.value;
        result = gpPTC_ResultSuccess;
        break;
    }
    case gpPTC_AttributeWriteRegisterValue :
    {
        // in case of only a number of bits that should be set : first read out the value, then change it
        gpPTC_WRValue = (UInt8) attribute.value;
        result = gpPTC_ResultSuccess;
        break;
    }
    case gpPTC_AttributeWriteRegisterBitsMask :
    {
        gpPTC_WRBitMask = (UInt32) attribute.value;
        result = gpPTC_ResultSuccess;
        break;
    }
    case gpPTC_AttributeScanCount :
    {
        if (attribute.value > UINT16_MAXVALUE)
        {
            result = gpPTC_ResultInvalidArgument;
        }
        else
        {
            gpPTC_ScanCount = (UInt16) attribute.value;
            result = gpPTC_ResultSuccess;
        }
        break;
    }
    case gpPTC_AttributePacketCount :
    {
        // prevent truncation !!
        if (attribute.value > UINT16_MAXVALUE)
        {
            result = gpPTC_ResultInvalidArgument;
        }
        else if (attribute.value == 0)
        {
            return gpPTC_ResultInvalidArgument;
        }
        else
        {
            gpPTC_PacketCount = (UInt16) attribute.value;
            result = gpPTC_ResultSuccess;
        }
        break;
    }
    case gpPTC_AttributePacketIntervalInMS :
    {
        if (attribute.value > UINT16_MAXVALUE)
        {
            result = gpPTC_ResultInvalidArgument;
        }
        else
        {
           gpPTC_PacketInterValInMS = (UInt16) attribute.value;
           result = gpPTC_ResultSuccess;
        }
        break;
    }
    case gpPTC_AttributeScanIntervalInMS :
    {
        if (attribute.value > UINT16_MAXVALUE)
        {
            result = gpPTC_ResultInvalidArgument;
        }
        else
        {
            gpPTC_ScanInterValInMS = (UInt16) attribute.value;
            result = gpPTC_ResultSuccess;
        }
        break;
    }
    case gpPTC_AttributePacketLength :
    {
        if (attribute.value > UINT16_MAXVALUE)
        {
            result = gpPTC_ResultInvalidArgument;
        }
        else
        {
            /* add some packet size limitation check here */
            if (gpTest_GetPhyMode() == gpTest_PhyModeMac && attribute.value > 125)
            {
                return gpPTC_ResultInvalidArgument;
            }
            else if (gpTest_GetPhyMode() == gpTest_PhyModeBle && attribute.value > 241)
            {
                return gpPTC_ResultInvalidArgument;
            }
            gpPTC_PacketLength = (UInt16) attribute.value;
            result = gpPTC_ResultSuccess;
        }
        break;
    }
    case gpPTC_AttributePhyMode :
    {
        if (gpTest_GetRxState() == 1)
        {
            // change of this attribute is only valid if RX mode is off
            result = gpPTC_ResultSetAttributeFailed;
        }
        else
        {
            // turn off running modes...
            gpTest_SetContinuousWaveMode(CW_OFF);
            gpSched_UnscheduleEvent(gpPTC_enableCWMode_M);
            gpSched_UnscheduleEvent(gpPTC_enableCWMode_U);
            gpTest_Stop();
            gpTest_SetPhyMode((UInt8) attribute.value);
            gpTest_Start();
            gpPTC_RestoreAttributes(true);

            result = gpPTC_ResultSuccess;
        }
        break;
    }
    case gpPTC_BLE_DTM_PacketType :
    {
        gpPTC_DirectTestMode_PacketType = (UInt8) attribute.value;
        break;
    }
    case gpPTC_BLE_DataRate :
    {
#if defined(GP_PTC_ENABLE_BLE)
    if (gpTest_GetPhyMode() != gpTest_PhyModeBle)
    {
        result = gpPTC_ResultUnsupported;
    }
    else
    {
        gpPTC_BLE_DataRateValue = (UInt8) attribute.value;
        /* TODO : remove constant values and replace with chip dependent ones */
        /* temp workaround for use with rpi (which has no chip dependency) is to use constant values */
        gpTest_BleTxPhy_t phy = gpHal_BleTxPhyInvalid;
        switch(gpPTC_BLE_DataRateValue)
        {
        case GPPTC_BLE_DATARATE_1MBS:
            phy = gpHal_BleTxPhy1Mb;
            break;
        case GPPTC_BLE_DATARATE_2MBS:
            phy = gpHal_BleTxPhy2Mb;
            break;
        default:
            // No support for coded phy yet
            GP_ASSERT_DEV_INT(false);
            break;
        }
        gpTest_SetModulation(phy);
    }
#else
        result = gpPTC_ResultUnsupported;
#endif /* defined(GP_PTC_ENABLE_BLE) */
        break;
    }
    case gpPTC_AttributeMapClockIOPin :
    {
        gpPTC_MapIOClockPin = (UInt8) attribute.value;
        break;
    }
    case gpPTC_AttributeMapClockType :
    {
        gpPTC_MapIOClockType = (gpHal_SleepMode_t) attribute.value;
        break;
    }
    case gpPTC_AttributeRxMultiStandard :
    {
        if (gpTest_GetRxState() == 1)
        {
            // change of this attribute is only valid if RX mode is off
            result = gpPTC_ResultSetAttributeFailed;
        }
        else
        {
            gpPTC_RXMultiStandard = (UInt8) attribute.value;
        }
        break;
    }
    case gpPTC_AttributeRxHighSensitivity :
    {
        if (gpTest_GetRxState() == 1)
        {
            // change of this attribute is only valid if RX mode is off
            result = gpPTC_ResultSetAttributeFailed;
        }
        else
        {
            gpPTC_RXHighSensitivity = (UInt8) attribute.value;
        }
        break;
    }
    case gpPTC_AttributeRxMultiChannel :
    {
        if (gpTest_GetRxState() == 1)
        {
            // change of this attribute is only valid if RX mode is off
            result = gpPTC_ResultSetAttributeFailed;
        }
        else
        {
            gpPTC_RXMultichannel = (UInt8) attribute.value;
        }
        break;
    }
    // APPHW-3412: Only needed for products that expect WiFi interference - Is blocked by dll if not exposable
    case gpPTC_AttributeRxLnaAttDuringTimeout :
    {
        // gpTest api is available for all chips, but will be stubbed if no implementation is available
        gpTest_SetRxLnaAttDuringTimeoutForRssiBasedAgcMode((Bool)attribute.value);
        break;
    }
    case gpPTC_AttributePDMClkSrc:
    {
        gpPTC_PDMClkCfg.src = (gpPTC_PDMClkSrc_t) attribute.value;
        break;
    }
    case gpPTC_AttributePDMClkFreq:
    {
        if (!GP_TEST_PDM_CLK_FREQ_VALIDATE((UInt32) attribute.value))
        {
            result = gpPTC_ResultInvalidParameter;
            break;
        }

        gpPTC_PDMClkCfg.freqHz = (UInt32) attribute.value;
        break;
    }
    case gpPTC_AttributePDMClkOutPin:
    {
        gpPTC_PDMClkCfg.gpio = (UInt8) attribute.value;
        break;
    }
    default :
    {
        GP_LOG_PRINTF("Unknown attribute %x" ,0,attribute.id);
        attribute.value = 0xFFFF;
        result = gpPTC_ResultInvalidArgument;
        break;
    }
    }
    return result;
}

/* exectime is in ms */
void gpPTC_SetModeRequestToStruct(UInt8 clientID, UInt8 modeID, UInt32 exectime, gpPTC_ModeExecution_t OnOff, UInt8 numberOfExtraParameters, gpPTC_Parameter_t* parameters, gpPTC_ModeParams_t* pParam)
{
    pParam->clientid = clientID;
    pParam->modeid = modeID;
    pParam->exectime = exectime;
    pParam->onoff = OnOff;
    pParam->numberofextraargs = numberOfExtraParameters;
    MEMCPY(&pParam->parameters, parameters, sizeof(gpPTC_Parameter_t));
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpPTC_Init(void)
{

#if defined(GP_PTC_DIVERSITY_GPCOM_SERVER)
    gpPTC_InitServer();
#endif
    gpPTC_IsBusy = false;
    GP_LOG_PRINTF("App Going to test mode",0);
    // gpTest_Start contains registration of DI, DC callbacks...
    gpTest_Start();
    gpTest_WakeUpGP();

   // gpHal_RegisterEDConfirmCallback(gpTest_cbEDConfirm);
}

void gpPTC_Discover(UInt8 clientID, gpPTC_MACAddress_t senderMacAddress)
{
    (void)clientID;
    (void)senderMacAddress;
    
    GP_LOG_SYSTEM_PRINTF("NO SUPPORT FOR MULTIPLE CONNECTIONS", 0);
}

gpPTC_ModeExecution_t gpPTC_GetModeRequest(UInt8 clientID, UInt8 modeID)
{
    gpPTC_ModeExecution_t result = gpPTC_ModeExecution_Single;
    switch (modeID)
    {
      case gpPTC_ModeCarrierWave :
        if (gpTest_GetContinuousWaveMode() == CW_OFF)
        {
          result = gpPTC_ModeExecution_Off;
        }
        else
        {
          result = gpPTC_ModeExecution_On;
        }
        break;
      case gpPTC_ModeRx :

        if (gpTest_GetPhyMode() == gpTest_PhyModeMac)
        {
            if (gpTest_GetRxState() == gpPTC_ModeExecution_On )
            {
              result = gpPTC_ModeExecution_On;
            }
            else
            {
              result = gpPTC_ModeExecution_Off;
            }
        }
        else if (gpTest_GetPhyMode() == gpTest_PhyModeBle)
        {
          // special case for BLE because we are using Directtestmode rx...
          // should be handled in gpTest...

            if (gpPTC_OldRxState)
            {
                 result = gpPTC_ModeExecution_On;
            }
            else
            {
              result = gpPTC_ModeExecution_Off;
            }
        }
        break;
      default :
        GP_LOG_PRINTF("Not implemented getmode %i",0,modeID);
        result = gpPTC_ModeExecution_Single;
        break;
    }
    return result;
}


/********************************************************************
* Set the cached values of the attributes                           *
* skipPhyMode : restoration can be called when switching phy mode,  *
*         so when this is set, do not restore the phy mode attribute*
*********************************************************************/
static void gpPTC_RestoreAttributes(Bool skipPhyMode)
{
    UIntLoop i;
    for (i = 1; i < PTC_ATTRIBUTES_MAX_NUMBER; i++)
    {
        if (gpPTC_attributeBuffer[i].id != 0)
        {
            // skip restoring the phy mode, as the restore has been triggered by the setting of the phy mode to a new value
            if (skipPhyMode && gpPTC_attributeBuffer[i].id == gpPTC_AttributePhyMode)
            {
                continue;
            }
            GP_LOG_PRINTF("r id : 0x%x val : %i ",0,gpPTC_attributeBuffer[i].id,(int)gpPTC_attributeBuffer[i].value);
            gpPTC_ExecAttribute(gpPTC_attributeBuffer[i]);
        }
    }
}

void gpPTC_SetClientIDRequest(UInt8 clientID)
{

}

void gpPTC_ResetDUT(void)
{
#ifdef GP_COMP_GPHAL
    gpSched_ScheduleEvent(1000, hal_Reset);
#endif
}

UInt32 gpPTC_GetDUTApiVersion(void)
{
    return GP_PTC_API_VERSION;
}

gpPTC_Result_t gpPTC_GetDUTInfoRequest(UInt8 clientID, UInt8* version, gpPTC_MACAddress_t* macAddr, gpPTC_DeviceAddress_t* bleAddr, gpPTC_swVersionNumber_t* appVersion, gpPTC_partNumber_t* partNumber, gpPTC_ProductName_t* productName, gpPTC_swVersionNumber_t* ptcVersion, gpPTC_ProductID_t* productID)
{

  GP_LOG_PRINTF("Dut Info",0);

  gpVersion_ReleaseInfo_t releaseInfo;
  MACAddress_t mac;
  gpTest_GetExtendedAddress(&mac);
  MEMCPY(macAddr,&mac,sizeof(MACAddress_t));

#ifdef GP_PTC_ENABLE_BLE
  BtDeviceAddress_t ble;
  // this call will read from the info/user page, this should be the one to use
  // assuming that the device address should already be written
  // for testing purposes, this will read from what is in the register
  // this is written by the setDeviceAddress call in main.c
  gpTest_BleGetDeviceAddress(&ble);
  MEMCPY(bleAddr,&ble.addr,sizeof(BtDeviceAddress_t));
#endif

  gpVersion_GetSoftwareVersion(&releaseInfo);
  appVersion->major = releaseInfo.major;
  appVersion->minor = releaseInfo.minor;
  appVersion->revision = releaseInfo.revision;
  appVersion->patch = releaseInfo.patch;
  appVersion->changelist = gpVersion_GetChangelist();

#ifdef GP_PTC_VERSION
  const ROM gpVersion_ReleaseInfo_t FLASH_PROGMEM gpVersionPTC = {GP_PTC_VERSION};
  MEMCPY_P(ptcVersion, &gpVersionPTC, sizeof(gpVersion_ReleaseInfo_t));
  ptcVersion->changelist = 0;
#else
  ptcVersion->major = releaseInfo.major;
  ptcVersion->minor = releaseInfo.minor;
  ptcVersion->revision = releaseInfo.revision;
  ptcVersion->patch = releaseInfo.patch;
  // changelist information is not known for the ptc component
  ptcVersion->changelist = 0;
#endif

  partNumber->header0 = gpTest_GetChipId();
  partNumber->header1 = gpTest_GetChipVersion();

  *version = PTC_VERSION;
  MEMSET(productName->name,'\0',sizeof(productName->name));

  gpTest_ReadProductId((UInt8*)productID);

#ifdef GP_PTC_PRODUCTNAME
  char product[] = {XSTRINGIFY(GP_PTC_PRODUCTNAME)};
  /* check to make sure productname is smaller than 40 chars */
  COMPILE_TIME_ASSERT(sizeof(product) < 40);
  MEMCPY(productName->name,&product,sizeof(product));
#else
  #error "No PTC Productname defined !!"
#endif
 return gpPTC_ResultSuccess;
}


/* Use this call to set some payload data that can be used by a command */
gpPTC_Result_t gpPTC_SetByteDataForAttributeRequest(UInt8 clientID, gpPTC_AttributeId_t attributeID, UInt8 dataLen, UInt8* pData)
{
  if (!gpPTC_CheckCompatibleHWVersion())
  {
    return gpPTC_ResultInvalidChip;
  }
  if (dataLen <=3 || dataLen > 125)
  {
    return gpPTC_ResultInvalidParameter;
  }
  if (attributeID == gpPTC_AttributeSetTxData) {
    if (gpPTC_pTxData != NULL)
    {
       gpPoolMem_Free(gpPTC_pTxData);
    }
    gpPTC_pTxData = (UInt8*) GP_POOLMEM_MALLOC(dataLen);
    MEMCPY(gpPTC_pTxData, pData, dataLen);

    gpPTC_pTxDataLength = dataLen;
  }
  return gpPTC_ResultSuccess ;
}


gpPTC_Result_t gpPTC_ExecuteCustomCommand(UInt8 clientID, UInt8 dataLenIn, UInt8* pDataIn, UInt8* dataLenOut, UInt8* pDataOut)
{
   if (!gpPTC_CheckCompatibleHWVersion())
  {
    return gpPTC_ResultInvalidChip;
  }
  MEMSET(pDataOut,0,sizeof(*dataLenOut));
// first byte of the data is the command id
#define commandId (pDataIn[0])
#define command (pDataIn + 1)
  if (!gpPTC_RunCustomCommand(commandId,dataLenIn,pDataIn,dataLenOut,pDataOut))
  {
        GP_LOG_PRINTF("Unsupported custom command 0x%x",0,commandId);
        return gpPTC_ResultInvalidParameter;
  }
#undef commandId
#undef command


    return gpPTC_ResultSuccess;
}

/*
* Implementation of the 32kHz crystal test
* The returned data is the result of this test
* @param lenIn Length of the input byte array
* @param pDataIn pointer to the input byte array
* @param pLenOut pointer to the output byte array (defined by the functionality)
* @param pDataOut pointer to the output byte array
* byte 0   : 00 - status : not started = 0; pending = 1; not stable = 2; stable = 3
* byte 1-4 : frequency in mHz
*/
Bool gpPTC_Execute32KhzTest(UInt8 dataLenIn, UInt8* pDataIn, UInt8* dataLenOut, UInt8* pDataOut)
{
  UInt32 frequencymHz = 0;

  gpHal_SleepClockMeasurementStatus_t status = gpTest_GetMeasuredSleepClockFrequency(gpHal_SleepMode32kHz, &frequencymHz);
  *dataLenOut = 5;
  pDataOut[0] = status;
  // reverse the order
  UInt32 newValue = (frequencymHz & 0x000000FFU) << 24 | (frequencymHz & 0x0000FF00U) << 8 |
         (frequencymHz & 0x00FF0000U) >> 8 | (frequencymHz & 0xFF000000U) >> 24;
  MEMCPY(&pDataOut[1], &newValue, sizeof(newValue));
  GP_LOG_PRINTF("status %i pDataOut 0x%i",0,status,pDataOut[1]);
  return true;
}


/* Setting of attributes : assuming checking of possible values has already been done at client side
 RF Settings are only being applied when actually send a tx packet.
*/
gpPTC_Result_t gpPTC_SetAttributeRequest(UInt8 clientID, UInt8 numberOfAttr, gpPTC_Attribute_t* attributes)
{
    gpPTC_Result_t result = gpPTC_ResultSuccess;
    UIntLoop i;

    for (i = 0; i < numberOfAttr; i++)
    {
        gpPTC_Attribute_t attribute = attributes[i];
        result = gpPTC_ExecAttribute(attribute);
        if (result != gpPTC_ResultSuccess)
        {
            break;
        }
        else
        {
            // Store all attributes so they can be restored later
            gpPTC_attributeBuffer[attribute.id] = attribute;
        }
    }

    return result;
}

gpPTC_Result_t gpPTC_GetAttributeRequest(UInt8 clientID, UInt8 numberOfAttr, gpPTC_Attribute_t* attributes)
{
   if (!gpPTC_CheckCompatibleHWVersion())
  {
    return gpPTC_ResultInvalidChip;
  }
    gpPTC_Result_t result = gpPTC_ResultSuccess ;
    UIntLoop i = 0;

    // look for the corresponding attribute based on id
    for (i = 0; i < numberOfAttr; i++)
    {
        switch (attributes[i].id)
        {
           case gpPTC_AttributeChannel :
           {
                attributes[i].value = gpTest_GetChannel();
                break;
           }
           case  gpPTC_AttributeTXPower :
           {
                attributes[i].value = gpTest_GetTxPower();
                break;
           }
           case gpPTC_AttributeAntenna :
           {
#if defined(GP_BSP_HAS_DIFFERENTIAL_SINGLE_ANTENNA)
#if GP_BSP_HAS_DIFFERENTIAL_SINGLE_ANTENNA()
                attributes[i].value = 0xFF;
                break;
#endif
#endif
                attributes[i].value = gpTest_GetRxAntenna();
                break;
           }
           case  gpPTC_AttributeAntennaDiversity:
           {
#if defined(GP_BSP_HAS_DIFFERENTIAL_SINGLE_ANTENNA)
#if GP_BSP_HAS_DIFFERENTIAL_SINGLE_ANTENNA()
                attributes[i].value = 0xFF;
                break;
#endif
#endif
                attributes[i].value = gpTest_GetAntennaDiversity();
                break;
           }
           case gpPTC_AttributeSleepMode :
           {
                attributes[i].value = gpTest_GetSleepMode();
                break;
           }
           case gpPTC_AttributeCWMode :
           {
                attributes[i].value = (gpPTC_CWMode == CW_UNMODULATED ? 1 : 0);
                break;
           }
           case gpPTC_AttributeCSMAMode :
           {
                attributes[i].value = gpTest_GetCollisionAvoidanceModeInUse();
                break;
           }
           case gpPTC_AttributeMAXBE :
           {
                attributes[i].value = gpTest_GetMaxBE();
                break;
           }
           case gpPTC_AttributeMINBE :
           {
                attributes[i].value = gpTest_GetMinBE();
                break;
           }
           case gpPTC_AttributeMaxCSMABackoff :
           {
                attributes[i].value = gpTest_GetMaxCSMABackoffs();
                break;
           }
           case gpPTC_AttributeMaxMACRetries :
           {
                attributes[i].value = gpTest_GetNumberOfRetries();
                break;
           }
           case gpPTC_AttributePanID :
           {
                attributes[i].value = gpTest_GetPanId(gpHal_SourceIdentifier_0);
                break;
           }
           case gpPTC_AttributeShortAddress :
           {
                attributes[i].value = gpTest_GetShortAddress(gpHal_SourceIdentifier_0);
                break;
           }
           case gpPTC_AttributePowerControlLoop :
           {
                attributes[i].value = false;
                break;
           }
           case gpPTC_AttributePacketInPacket :
           {
                attributes[i].value = gpTest_GetPacketInPacketMode();
                break;
           }
          case gpPTC_AttributePromiscuousMode :
           {
                attributes[i].value = gpTest_GetPromiscuousMode();
                break;
           }
             case gpPTC_AttributePacketCount :
           {
               attributes[i].value =  gpPTC_PacketCount;
               break;
           }
              case gpPTC_AttributeScanCount :
           {
                attributes[i].value =  gpPTC_ScanCount;
               break;
           }
           case gpPTC_AttributePacketIntervalInMS :
           {
                attributes[i].value =  gpPTC_PacketInterValInMS ;
               break;
           }
           case gpPTC_AttributeScanIntervalInMS :
           {
                attributes[i].value =  gpPTC_ScanInterValInMS ;
               break;
           }
              case gpPTC_AttributePacketLength :
           {
                attributes[i].value =  gpPTC_PacketLength ;
               break;
           }
         //   other attributes are not relevant ?
           case  gpPTC_AttributeReadRegisterAddress :
           {
                attributes[i].value = gpPTC_RRAddress;
                break;
           }
           case gpPTC_AttributeWriteRegisterAddress :
           {
                attributes[i].value = gpPTC_WRAddress;
                break;
           }
           case gpPTC_AttributeWriteRegisterValue :
           {
                attributes[i].value = gpPTC_WRValue;
               break;
           }
            case gpPTC_AttributeWriteRegisterBitsMask :
           {
                attributes[i].value = gpPTC_WRBitMask;
               break;
           }
           case gpPTC_AttributePhyMode :
           {
                attributes[i].value = gpTest_GetPhyMode();
               break;
           }
            case gpPTC_BLE_DTM_PacketType :
           {
               attributes[i].value = gpPTC_DirectTestMode_PacketType;
            break;
           }
            case gpPTC_BLE_DataRate :
           {
              attributes[i].value = gpPTC_BLE_DataRateValue;
              break;
           }
           case gpPTC_AttributeMapClockType :
           {
              attributes[i].value = (UInt8) gpPTC_MapIOClockType;
              break;
           }
           case gpPTC_AttributeMapClockIOPin :
           {
              attributes[i].value = gpPTC_MapIOClockPin;
              break;
           }
           case gpPTC_AttributeRxMultiStandard :
           {
              attributes[i].value = gpPTC_RXMultiStandard;
              break;
           }
           case gpPTC_AttributeRxHighSensitivity :
           {
              attributes[i].value = gpPTC_RXHighSensitivity;
              break;
           }
           case gpPTC_AttributeRxMultiChannel :
           {
              attributes[i].value = gpPTC_RXMultichannel;
              break;
           }
#if defined(GP_DIVERSITY_GPHAL_K8E)
           // todo getter should be in gpTest
           case gpPTC_AttributeRxLnaAttDuringTimeout :
           {
            attributes[i].value = gpTest_ReadReg(0x0258) & 0x2;
            break;
           }
#endif
           case gpPTC_AttributePDMClkSrc:
           {
               attributes[i].value = gpPTC_PDMClkCfg.src;
               break;
           }
           case gpPTC_AttributePDMClkFreq:
           {
               attributes[i].value =  gpPTC_PDMClkCfg.freqHz;
               break;
           }
           case gpPTC_AttributePDMClkOutPin:
           {
               attributes[i].value =  gpPTC_PDMClkCfg.gpio;
               break;
           }
           default :
           {
               break;
           }
        }
        GP_LOG_PRINTF("GetAttr  %x %lx",0,attributes[i].id,(UInt32)attributes[i].value);
        if (result != gpPTC_ResultSuccess)
            break;
    }
    return result;
}



void gpPTC_SetModeRequestFromStruct(void* pArg)
{
    gpPTC_ModeParams_t* pParam = (gpPTC_ModeParams_t*)pArg;
    gpPTC_SetModeRequest(pParam->clientid,pParam->modeid,pParam->exectime,pParam->onoff,pParam->numberofextraargs,pParam->parameters);
    gpPoolMem_Free(pParam);
}

void gpTest_cbDataConfirm(UInt8 status, UInt16 packetsSentOK, UInt16 packetsSentError)
{
  GP_LOG_PRINTF("gpTest_cbDataConfirm %x",0,status);
  // this should not be shown in case of OTA communication... but in this case callbacks should be assigned to other
  if (!UseOTAProtocol)
  {
  //    gpTest_cbDataConfirm(status,packetsSentOK,packetsSentError);
  }
}

gpPTC_Result_t gpPTC_SetModeRequest(UInt8 clientID, UInt8 modeID, UInt32 exectime, gpPTC_ModeExecution_t OnOff, UInt8 numberOfExtraParameters, gpPTC_Parameter_t* parameters)
{
    if (!gpPTC_CheckCompatibleHWVersion())
    {
        return gpPTC_ResultInvalidChip;
    }
    gpPTC_Result_t result = gpPTC_ResultSuccess;
    GP_LOG_PRINTF("setmode %x %x exectime=%lu",0,(UInt16)modeID,(UInt8)OnOff, (UInt32)exectime);

    if (exectime != 0 && OnOff == gpPTC_ModeExecution_On)
    {
        // what happens with the re-init in case of
        // schedule to off command for the command
        gpPTC_ModeParams_t* params = (gpPTC_ModeParams_t*) GP_POOLMEM_MALLOC(sizeof(gpPTC_ModeParams_t));
        gpPTC_SetModeRequestToStruct(clientID,modeID,0,gpPTC_ModeExecution_Off,numberOfExtraParameters,parameters,params);
        // schedule the set mode off function
        gpSched_ScheduleEventArg(exectime*1000,gpPTC_SetModeRequestFromStruct,params);
    }

    switch (modeID) {
      case gpPTC_ModeInfo :
      {
          // Prevent On or Off setting for these modes
          if (OnOff != gpPTC_ModeExecution_Single)
          {
            return gpPTC_ResultSetModeFailed;
          }
        /* this is a placeholder since this should be handled in a seperate request because of the different return value structure */
        /* keeping this here for OTA information ge*/
        break;
      }
      case gpPTC_ModeCarrierWave :
          {
            if (OnOff == gpPTC_ModeExecution_Off && gpTest_GetContinuousWaveMode() != CW_OFF)
            {
                gpTest_SetContinuousWaveMode( CW_OFF );

                gpSched_UnscheduleEvent(gpPTC_enableCWMode_M);
                gpSched_UnscheduleEvent(gpPTC_enableCWMode_U);

                if(gpPTC_OldRxState != gpTest_GetRxState()) gpTest_SetRxState(gpPTC_OldRxState);

            }
            else if (OnOff == gpPTC_ModeExecution_On)
            {

                gpPTC_OldRxState = gpTest_GetRxState();
                if(gpPTC_OldRxState != 0) gpTest_SetRxState(0);

                gpPTC_mainReEnableCw(gpPTC_CWMode);
            }
            break;
          }
        case gpPTC_ModeEDScan :
        {
          UInt16 channelMask = 0;
          if (gpTest_GetPhyMode() != gpTest_PhyModeMac)
          {
            result = gpPTC_ResultUnsupported;
          }
          else
          {
            if (gpTest_GetContinuousWaveMode() != CW_OFF) {
              result = gpPTC_ResultBusy;
              break;
            }
            else
            {
              if (OnOff == gpPTC_ModeExecution_On)
              {

              /* Don't expect user to input channelMask, get current channel and bit shift this into the channelmask */
                 BIT_SET(channelMask, (gpTest_GetChannel() - 11));
                 gpTest_EDScan(gpPTC_ScanCount, gpPTC_PacketInterValInMS,channelMask);
                 gpPTC_EDScanBusy = true;
               }
               else
               {
                /* no way of interrupting the ed scan once started, not foreseen in gphal, so try to do something meaningfull */
                  if (gpPTC_EDScanBusy)
                    result = gpPTC_ResultBusy;
                  else
                    result = gpPTC_ResultUnsupported;
               }
            }
          }
            break;
        }
        case gpPTC_ModeRx :
        {
          if (OnOff == gpPTC_ModeExecution_On)
          {
              gpTest_Result_t result = gpTest_SetRxModeOptions(gpPTC_RXMultiStandard == gpPTC_Enabled, gpPTC_RXMultichannel == gpPTC_Enabled, gpPTC_RXHighSensitivity == gpPTC_Enabled);
              if (result == gpHal_ResultSuccess)
        {
                gpTest_SetRxState(1);
                gpPTC_OldRxState = gpTest_GetRxState();
              }
              else
        {
                return gpPTC_ResultInvalidRXMode;
              }
          }
          else
          {
              gpTest_SetRxState(0);
              if (gpTest_GetPhyMode() == gpTest_PhyModeBle)
              {
#ifdef GP_PTC_ENABLE_BLE
                 gpPTC_RXPacketCount += gpTest_BleGetNumberOfRxPackets();
#endif
              }
              gpPTC_OldRxState = gpTest_GetRxState();
          }
          break;
        }
        case gpPTC_ModeTransmitPacket :
        {
         switch (OnOff)
          {
            case  gpPTC_ModeExecution_On :
            {
              if (gpPTC_PacketCount == 0)
              {
                return gpPTC_ResultInvalidArgument;
              }
              if (gpTest_GetPhyMode() == gpTest_PhyModeMac)
               {
                if((gpPTC_pTxDataLength < 3) || (gpPTC_pTxDataLength> 125))
                {
                    return gpPTC_ResultInvalidArgument;
                }

                if(gpPTC_pTxData == NULL)
                {
                    return gpPTC_ResultInvalidArgument;
                }
                else
                {
                    gpTest_TxPacket(gpPTC_PacketCount, gpPTC_PacketInterValInMS, gpPTC_pTxDataLength, gpPTC_pTxData, 0);
                }
                gpPTC_IsBusy = true;
              }
              else  if (gpTest_GetPhyMode() == gpTest_PhyModeBle)
              {
#ifdef GP_PTC_ENABLE_BLE
                if (gpPTC_PacketLength < 3 || (gpPTC_PacketLength > 241))
                {
                   return gpPTC_ResultInvalidArgument;
                }
                gpTest_BleTestEnd();
                /// TODO : cleanup
                gpTest_BleSetNumberTxPacketsInTestMode(gpPTC_PacketCount);
                gpTest_BleTransmitterTest(gpPTC_PacketLength,gpPTC_DirectTestMode_PacketType);
#endif
              }

            break;
            }
            case gpPTC_ModeExecution_Off :
            {
              if (gpTest_GetPhyMode() == gpTest_PhyModeMac)
              {
                gpTest_TxPacket(0, 0, 0, NULL, false);
                gpPTC_IsBusy = false;
              }
              else if (gpTest_GetPhyMode() == gpTest_PhyModeBle)
              {
#ifdef GP_PTC_ENABLE_BLE
                // normally this returns the number of sent packets, but this is now always 0xFFFF
                gpPTC_TXPacketCount += gpTest_BleTestEnd();
#endif
              }
              break;
            }
            default :
            {
              break;
            }
          }

          break;
        }
        case gpPTC_ModeTransmitRandomPackets :
        {
           switch (OnOff)
          {
            case  gpPTC_ModeExecution_On :
            {
              if (gpTest_GetPhyMode() == gpTest_PhyModeBle)
              {
                  return gpPTC_ResultUnsupported;
              }
              else
              {
                if(gpPTC_PacketInterValInMS<0)
                {
                    return gpPTC_ResultInvalidArgument;
                }
                if(gpPTC_PacketCount <= 0)
                {
                    return gpPTC_ResultInvalidArgument;
                }
                if( (gpPTC_PacketLength <= 3) || (gpPTC_PacketLength > 125) )
                {
                    return gpPTC_ResultInvalidArgument;
                }
                gpTest_TxPacket(gpPTC_PacketCount, gpPTC_PacketInterValInMS, gpPTC_PacketLength, NULL, true);
                gpPTC_IsBusy = true;
              }
              break;
            }
             case gpPTC_ModeExecution_Off :
            {
              if (gpTest_GetPhyMode() == gpTest_PhyModeBle)
              {
                  return gpPTC_ResultUnsupported;
              }
              else
              {
                gpTest_TxPacket(0, 0, 0, NULL, false);
                gpPTC_IsBusy = false;
              }
              break;
            }
             default :
            {
              break;
            }
          }

          break;
        }
        case gpPTC_ModePrintPacketCount :
        {
         // Prevent On or Off setting for these modes
          if (OnOff != gpPTC_ModeExecution_Single)
          {
            return gpPTC_ResultSetModeFailed;
          }
          /* send back the counters as extraparameters */
          GP_ASSERT_DEV_INT(numberOfExtraParameters == 5);
          gpTest_Statistics_t stats;
          gpTest_GetStatistics(&stats);

          if (gpTest_GetPhyMode() == gpTest_PhyModeBle)
          {
            parameters[2].value = gpPTC_RXPacketCount;
            parameters[3].value = gpPTC_TXPacketCount;
            parameters[4].value = 0;
          }
          else
          {
            parameters[2].value = stats.ReceivedPacketCounterC;
            parameters[3].value = stats.PacketsSentOK;
            parameters[4].value = stats.PacketsSentError;
          }
          if (gpPTC_RXPacketCount)
          {
            parameters[0].value = gpPTC_AllRSSI/gpPTC_RXPacketCount;
            parameters[1].value = gpPTC_AllLQI /gpPTC_RXPacketCount;
          }
          else
          {
            parameters[0].value = 0;
            parameters[1].value = 0;
          }
         break;
        }
        case gpPTC_ModeResetCounters :
        {
          // Prevent On or Off setting for these modes
          if (OnOff != gpPTC_ModeExecution_Single)
          {
            return gpPTC_ResultSetModeFailed;
          }
          gpTest_ResetStatistics();
          gpPTC_RXPacketCount = 0;
          gpPTC_TXPacketCount = 0;
          gpPTC_AllRSSI = 0;
          gpPTC_AllLQI = 0;
          break;
        }
        case gpPTC_ModePrintReceivedPackets :
        {
            // BLE mode is way faster than UART
            if (gpTest_GetPhyMode() != gpTest_PhyModeMac)
            {
                gpPTC_PrintRXPackets = false;
                result = gpPTC_ResultUnsupported;
            }
            else
            {
                if (OnOff == gpPTC_ModeExecution_On) {
                    gpPTC_PrintRXPackets = true;
                }
                else {
                    gpPTC_PrintRXPackets = false;
                }
            }
            break;
        }
        case gpPTC_ModeSleep :
        {

            if (OnOff == gpPTC_ModeExecution_On)
            {
                GP_LOG_PRINTF("Configuring for sleep... ",0);
#if defined(GP_DIVERSITY_GPHAL_K8E)
                // make sure clock GPIO mappings are restored, this can have impact !
                if (gpPTC_MapClockIOPin_Enabled != 0)
                {
                    restoreClkToGPIORegisters();
                }
#endif
                gpTest_SetAsleep();
            }
            else
            {
                hal_SleepSetGotoSleepEnable(false);
                if(!gpTest_IsAwake())
                {
                    gpTest_WakeUpGP();
                }
            }
            break;
        }
        case gpPTC_ModeWakeUp :
        {
            // can be removed....
            if(!gpTest_IsAwake())
            {
                gpTest_WakeUpGP();
            }
            break;
        }
        case gpPTC_ModeReadRegister :
        {
            // Prevent On or Off setting for these modes
            if (OnOff != gpPTC_ModeExecution_Single)
            {
                return gpPTC_ResultSetModeFailed;
            }
            GP_ASSERT_DEV_INT(numberOfExtraParameters == 1);

            UInt8 regVal = gpTest_ReadReg(gpPTC_RRAddress);
            parameters[0].value = regVal;
            result = gpPTC_ResultSuccess;
            break;
        }
        case gpPTC_ModeWriteRegister :
        {
            // Prevent On or Off setting for these modes
            if (OnOff != gpPTC_ModeExecution_Single)
            {
                return gpPTC_ResultSetModeFailed;
            }
            Bool retVal = gpTest_WriteReg(gpPTC_WRAddress,gpPTC_WRValue);
            if (retVal) {
                result = gpPTC_ResultSuccess;
            }
            else {
                result = gpPTC_ResultSetModeFailed;
            }
            break;
        }
        case gpPTC_ModeWriteRegisterBits :
        {
            // Prevent On or Off setting for these modes
            if (OnOff != gpPTC_ModeExecution_Single)
            {
                return gpPTC_ResultSetModeFailed;
            }
            //   GP_LOG_PRINTF("gpPTC_WRAddress %x",0,(UInt32)gpPTC_WRAddress);
            //  GP_LOG_PRINTF("gpPTC_WRValue %x",0,(UInt32)gpPTC_WRValue);
            // first readout the register
            UInt8 readVal = gpTest_ReadReg(gpPTC_WRAddress);
            // GP_LOG_PRINTF("readreg %x",0,readVal);
            //GP_LOG_PRINTF("gpPTC_WRBitMask %i",0,(readVal & ~gpPTC_WRBitMask));
            UInt8 bitmask = (UInt8) gpPTC_WRBitMask;
            int i;
            for (i = 0; i < 8; i++) {
                int res = bitmask & 1;
                if (res == 1)
                    break;
                bitmask >>= 1;
            }
            // apply inverse bit mask with the value to set
            // value needs to be shifted to  the correct position
            UInt8 newValue = (readVal & ~gpPTC_WRBitMask) | (gpPTC_WRValue <<i);
            // get the first set bit from the bitmask to get the starting position

            //GP_LOG_PRINTF("newValue %x",0,newValue);
            Bool retVal = gpTest_WriteReg(gpPTC_WRAddress,newValue);

            if (retVal) {
                result = gpPTC_ResultSuccess;
            }
            else {
                result = gpPTC_ResultSetModeFailed;
            }
            break;
        }
#if defined(GP_DIVERSITY_GPHAL_K8E)
        case gpPTC_ModeSetClockToGPIO :
        {
            if (gpPTC_MapIOClockType != gpHal_SleepMode32kHz)
            {
                return gpPTC_ResultUnsupported;
            }

            if (OnOff == gpPTC_ModeExecution_On)
            {
                // make sure no other mode is running !
                if (gpPTC_IsBusy)
                {
                    // what to do ?
                    return gpPTC_ResultBusy;
                }

                gpPTC_IsBusy = true;
                gpPTC_MapClockIOPin_Enabled = gpPTC_MapIOClockPin;

                result = mapClkToGPIORegisters();
            }
            else if (OnOff == gpPTC_ModeExecution_Off)
            {
                if (gpPTC_MapClockIOPin_Enabled == 0)
                {
                    return gpPTC_ResultSetModeFailed;
                }

                restoreClkToGPIORegisters();
            }
            break;
        }
#endif // GP_DIVERSITY_GPHAL_K8A || defined(GP_DIVERSITY_GPHAL_K8C) || defined(GP_DIVERSITY_GPHAL_K8D) || defined(GP_DIVERSITY_GPHAL_K8E)
        case gpPTC_ModeSetPdmClock :
        {
            gpTest_PDMClkSrc_t src = gpPTC_PDMClkSrc_None;

            gpPTC_PDMClkCfg.state = (OnOff == gpPTC_ModeExecution_On);

            if(!gpPTC_PDMClkCfg.state)
            {
                src = gpPTC_PDMClkSrc_None;
            }   
            else if (gpPTC_PDMClkCfg.src == gpPTC_PDMClkSrc_2M)
            {
                src = gpTest_PDMClkSrc_2M;
            }
            else if (gpPTC_PDMClkCfg.src == gpPTC_PDMClkSrc_PLL)
            {
                src = gpTest_PDMClkSrc_PLL;
            }

            result = gpTest_SetPdmClk(src, gpPTC_PDMClkCfg.freqHz, gpPTC_PDMClkCfg.gpio);
            break;
        }
        default :
        {
            result = gpPTC_ResultUnsupported;
            break;
        }
    } // switch
    return result;
}

gpPTC_Result_t mapClkToGPIORegisters(void)
{
    gpPTC_Result_t result = gpPTC_ResultSuccess;
    // Set the standby mode by writing the register
    // Using the gpTest_SetSleepMode function will not work, because this function
    // will only set a variable that will be used when actually going to sleep
    gpPTC_SleepMode_OrigValue = gpHal_GetSleepMode();
    gpHal_SetSleepMode(gpPTC_MapIOClockType);

    switch (gpPTC_MapIOClockPin) {
#if !defined(GP_DIVERSITY_GPHAL_K8E)
    case 18 :
    {
        gpPTC_DBG_Sel_OrigValue =  GP_WB_READ_IOB_DBG0_SEL();
        GP_WB_WRITE_IOB_DBG0_SEL(158); // stbclk - used for stand by configuration
#if   defined(GP_DIVERSITY_GPHAL_K8E)
        gpPTC_DBG_En_OrigValue = GP_WB_READ_IOB_GPIO_18_ALTERNATE();
        GP_WB_WRITE_IOB_GPIO_18_ALTERNATE_ENABLE(0);
        GP_WB_WRITE_IOB_GPIO_18_ALTERNATE(GP_WB_ENUM_GPIO_18_ALTERNATES_DEBUG_DBG_0);
        GP_WB_WRITE_IOB_GPIO_18_ALTERNATE_ENABLE(1);
#endif //GP_DIVERSITY_GPHAL_K8A
        break;
    }
    case 17 :
    {
        gpPTC_DBG_Sel_OrigValue =  GP_WB_READ_IOB_DBG1_SEL();
        GP_WB_WRITE_IOB_DBG1_SEL(158); // stbclk - used for stand by configuration
#if   defined(GP_DIVERSITY_GPHAL_K8E)
        gpPTC_DBG_En_OrigValue = GP_WB_READ_IOB_GPIO_17_ALTERNATE();
        GP_WB_WRITE_IOB_GPIO_17_ALTERNATE_ENABLE(0);
        GP_WB_WRITE_IOB_GPIO_17_ALTERNATE(GP_WB_ENUM_GPIO_17_ALTERNATES_DEBUG_DBG_1);
        GP_WB_WRITE_IOB_GPIO_17_ALTERNATE_ENABLE(1);
#endif //GP_DIVERSITY_GPHAL_K8A
        break;
    }
#endif //!defined(GP_DIVERSITY_GPHAL_K8D) && !defined(GP_DIVERSITY_GPHAL_K8E)
    case 16 :
    {
        gpPTC_DBG_Sel_OrigValue =  GP_WB_READ_IOB_DBG2_SEL();
        GP_WB_WRITE_IOB_DBG2_SEL(158); // stbclk - used for stand by configuration
#if   defined(GP_DIVERSITY_GPHAL_K8E)
        gpPTC_DBG_En_OrigValue = GP_WB_READ_IOB_GPIO_16_ALTERNATE();
        GP_WB_WRITE_IOB_GPIO_16_ALTERNATE_ENABLE(0);
        GP_WB_WRITE_IOB_GPIO_16_ALTERNATE(GP_WB_ENUM_GPIO_16_ALTERNATES_DEBUG_DBG_2);
        GP_WB_WRITE_IOB_GPIO_16_ALTERNATE_ENABLE(1);
#endif //GP_DIVERSITY_GPHAL_K8A
        break;
    }
    case 15 :
    {
        gpPTC_DBG_Sel_OrigValue =  GP_WB_READ_IOB_DBG3_SEL();
        GP_WB_WRITE_IOB_DBG3_SEL(158); // stbclk - used for stand by configuration
#if   defined(GP_DIVERSITY_GPHAL_K8E)
        gpPTC_DBG_En_OrigValue = GP_WB_READ_IOB_GPIO_15_ALTERNATE();
        GP_WB_WRITE_IOB_GPIO_15_ALTERNATE_ENABLE(0);
        GP_WB_WRITE_IOB_GPIO_15_ALTERNATE(GP_WB_ENUM_GPIO_15_ALTERNATES_DEBUG_DBG_3);
        GP_WB_WRITE_IOB_GPIO_15_ALTERNATE_ENABLE(1);
#endif //GP_DIVERSITY_GPHAL_K8A
        break;
    }
    case 14 :
    {
        gpPTC_DBG_Sel_OrigValue =  GP_WB_READ_IOB_DBG4_SEL();
        GP_WB_WRITE_IOB_DBG4_SEL(158); // stbclk - used for stand by configuration
#if   defined(GP_DIVERSITY_GPHAL_K8E)
        gpPTC_DBG_En_OrigValue = GP_WB_READ_IOB_GPIO_14_ALTERNATE();
        GP_WB_WRITE_IOB_GPIO_14_ALTERNATE_ENABLE(0);
        GP_WB_WRITE_IOB_GPIO_14_ALTERNATE(GP_WB_ENUM_GPIO_14_ALTERNATES_DEBUG_DBG_4);
        GP_WB_WRITE_IOB_GPIO_14_ALTERNATE_ENABLE(1);
#endif //GP_DIVERSITY_GPHAL_K8A
        break;
    }
#if defined(GP_DIVERSITY_GPHAL_K8E)
    case 13 :
    {
        gpPTC_DBG_Sel_OrigValue =  GP_WB_READ_IOB_DBG0_SEL();
        GP_WB_WRITE_IOB_DBG0_SEL(158); // stbclk - used for stand by configuration
        gpPTC_DBG_En_OrigValue = GP_WB_READ_IOB_GPIO_13_ALTERNATE();
        GP_WB_WRITE_IOB_GPIO_13_ALTERNATE_ENABLE(0);
        GP_WB_WRITE_IOB_GPIO_13_ALTERNATE(GP_WB_ENUM_GPIO_13_ALTERNATES_DEBUG_DBG_0);
        GP_WB_WRITE_IOB_GPIO_13_ALTERNATE_ENABLE(1);
        break;
    }
    case 12 :
    {
        gpPTC_DBG_Sel_OrigValue =  GP_WB_READ_IOB_DBG1_SEL();
        GP_WB_WRITE_IOB_DBG1_SEL(158); // stbclk - used for stand by configuration
        gpPTC_DBG_En_OrigValue = GP_WB_READ_IOB_GPIO_12_ALTERNATE();
        GP_WB_WRITE_IOB_GPIO_12_ALTERNATE_ENABLE(0);
        GP_WB_WRITE_IOB_GPIO_12_ALTERNATE(GP_WB_ENUM_GPIO_12_ALTERNATES_DEBUG_DBG_1);
        GP_WB_WRITE_IOB_GPIO_12_ALTERNATE_ENABLE(1);
        break;
    }
#endif //defined(GP_DIVERSITY_GPHAL_K8D) || defined(GP_DIVERSITY_GPHAL_K8E)
    default :
        result = gpPTC_ResultInvalidArgument;
        break;
    }
    return result;
}

gpPTC_Result_t restoreClkToGPIORegisters(void)
{
    gpPTC_Result_t result = gpPTC_ResultSuccess;
#if defined(GP_DIVERSITY_GPHAL_K8E)
    if (gpHal_SetSleepMode(gpPTC_SleepMode_OrigValue) != gpHal_ResultSuccess)
    {
        result = gpPTC_ResultBusy;
    }


    switch (gpPTC_MapClockIOPin_Enabled){
#if !defined(GP_DIVERSITY_GPHAL_K8E)
    case 18 :
    {
        GP_WB_WRITE_IOB_DBG0_SEL(gpPTC_DBG_Sel_OrigValue);
        break;
    }
    case 17 :
    {
        GP_WB_WRITE_IOB_DBG1_SEL(gpPTC_DBG_Sel_OrigValue);
        break;
    }
#endif //!defined(GP_DIVERSITY_GPHAL_K8D) && !defined(GP_DIVERSITY_GPHAL_K8E)
    case 16 :
    {
        GP_WB_WRITE_IOB_DBG2_SEL(gpPTC_DBG_Sel_OrigValue);
#if   defined(GP_DIVERSITY_GPHAL_K8E)
        GP_WB_WRITE_IOB_GPIO_16_ALTERNATE_ENABLE(0);
        GP_WB_WRITE_IOB_GPIO_16_ALTERNATE(gpPTC_DBG_En_OrigValue);
#endif
        break;
    }
    case 15 :
    {
        GP_WB_WRITE_IOB_DBG3_SEL(gpPTC_DBG_Sel_OrigValue);
#if   defined(GP_DIVERSITY_GPHAL_K8E)
        GP_WB_WRITE_IOB_GPIO_15_ALTERNATE_ENABLE(0);
        GP_WB_WRITE_IOB_GPIO_15_ALTERNATE(gpPTC_DBG_En_OrigValue);
#endif
        break;
    }
    case 14 :
    {
        GP_WB_WRITE_IOB_DBG4_SEL(gpPTC_DBG_Sel_OrigValue);
#if   defined(GP_DIVERSITY_GPHAL_K8E)
        GP_WB_WRITE_IOB_GPIO_14_ALTERNATE_ENABLE(0);
        GP_WB_WRITE_IOB_GPIO_14_ALTERNATE(gpPTC_DBG_En_OrigValue);
#endif
        break;
    }
#if defined(GP_DIVERSITY_GPHAL_K8E)
    case 13 :
    {
        GP_WB_WRITE_IOB_DBG0_SEL(gpPTC_DBG_Sel_OrigValue);
        GP_WB_WRITE_IOB_GPIO_13_ALTERNATE_ENABLE(0);
        GP_WB_WRITE_IOB_GPIO_13_ALTERNATE(gpPTC_DBG_En_OrigValue);
        break;
    }
    case 12 :
    {
        GP_WB_WRITE_IOB_DBG1_SEL(gpPTC_DBG_Sel_OrigValue);
        GP_WB_WRITE_IOB_GPIO_12_ALTERNATE_ENABLE(0);
        GP_WB_WRITE_IOB_GPIO_12_ALTERNATE(gpPTC_DBG_En_OrigValue);
        break;
    }
#endif //defined(GP_DIVERSITY_GPHAL_K8D) || defined(GP_DIVERSITY_GPHAL_K8E)
    default :
    {
        result = gpPTC_ResultInvalidArgument;
        break;
    }
    }
    if (result == gpPTC_ResultSuccess) {
        gpPTC_IsBusy = false;
        gpPTC_MapClockIOPin_Enabled = 0;
    }
#endif
    return result;

}


void gpTest_cbDataIndication(UInt8 length, gpPd_Offset_t dataOffset, gpPd_Handle_t handle)
{
    UInt8 psdu[GP_TEST_MAX_LENGTH_PACKET];

    length = ((length <= GP_TEST_MAX_LENGTH_PACKET) ? length : GP_TEST_MAX_LENGTH_PACKET); //Clip length

    gpPTC_RXPacketCount++;
    Int8 rssi = (Int8)gpPd_GetRssi(handle);
    gpPTC_AllRSSI += rssi;

    gpPTC_AllLQI += gpPd_GetLqi(handle);

    if (gpPTC_PrintRXPackets)
    {
        gpPd_ReadByteStream(handle,dataOffset,length,psdu);
        gpPTC_RXPacketIndication(length,psdu);
    }
    gpPd_FreePd(handle);
}

void gpTest_cbEDConfirm(UInt8 result, UInt16 channelMask, UInt8 *pData, Bool Finished)
{
    UInt16 channelIndex = (gpTest_GetChannel() - 11);
    if (Finished)
    {
      gpPTC_EDScanBusy = false;
    }
    gpPTC_EDConfirm(result, Finished, pData[channelIndex]);
}
