/*
 *   Copyright (c) 2019, Qorvo Inc
 *
 *
 *   Implementation of BleModule_Config
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

/* <CodeGenerator Placeholder> Includes */
#include "wsf_types.h"
#include "app_api.h"
#include "svc_cfg.h"
#include "bstream.h"
#include "att_uuid.h"
#include "BleModule.h"
#include "gpBlePeripheralConnectionStm.h"
#include "smp_api.h"
/* </CodeGenerator Placeholder> Includes */


/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> Macro */
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
/* </CodeGenerator Placeholder> TypeDefinitions */

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticData */
static appSlaveCfg_t appSlaveCfgDefault = {
    .connMax = 3,
    .startAdvAfterConnectionClose = false,
    .securityRequestTimeout = 10
};

static appAdvCfg_t appAdvCfgDefault = {
    {30000,     0,     0},                              /*! Advertising durations in ms */
    {HCI_ADV_MIN_INTERVAL,  0,     0}                   /*! Advertising intervals in 0.625 ms units */
};

static appExtAdvCfg_t appExtAdvCfgDefault = {
    /* Extended advertisement is not used in current state of development.
       for easy integration in later stage, the default structure is already defined */
    {0},
    {0},
    {0},
    {0},
    {0}
};

static appUpdateCfg_t appUpdateCfgDefault = {

    .idlePeriod = 0,
    .connIntervalMin = 80,
    .connIntervalMax = 400,
    .connLatency = 5,
    .supTimeout = 601,
    .maxAttempts = 3
};

/*! SMP security parameter configuration */
static smpCfg_t smpCfgDefault =
{
  3000,                                   /*! 'Repeated attempts' timeout in msec */
  SMP_IO_NO_IN_NO_OUT,                    /*! I/O Capability */
  16,                                     /*! Minimum encryption key length */
  16,                                     /*! Maximum encryption key length */
  3,                                      /*! Attempts to trigger 'repeated attempts' timeout */
  0,                                      /*! Device authentication requirements */
};

static appSecCfg_t appSecCfgDefault = {
    .auth = DM_AUTH_BOND_FLAG | DM_AUTH_SC_FLAG,
    .iKeyDist = DM_KEY_DIST_LTK | DM_KEY_DIST_IRK,
    .rKeyDist = DM_KEY_DIST_LTK | DM_KEY_DIST_IRK,
    .oob = false,
    .initiateSec = false
};

static uint8_t appAdvDataDiscDefault[] = {
    /* General, length of the LEN-field should be the length of the AdvDataType + AdvData. So the lengthfield itself is not part of the lengthfield */
    /* Advertisement flags, length is two */
    0x02,
    DM_ADV_TYPE_FLAGS,
    DM_FLAG_LE_GENERAL_DISC | DM_FLAG_LE_BREDR_NOT_SUP,
    /* Prefered connection interval, len 5 */
    0x05,
    DM_ADV_TYPE_CONN_INTERVAL,
    /* Min prefered connection interval LSB, MSB*/
    0x10,
    0x00,
    /* Max prefered connection interval LSB, MSB*/
    0x20,
    0x00,
    /* Manufacturer, len 3 */
    0x03,
    DM_ADV_TYPE_MANUFACTURER,
    0x53,
    0x04,
    /* 128-bit UUID Service, len 17 */
    0x11,
    DM_ADV_TYPE_128_UUID,
    ATT_UUID_P1_SERVICE
};

static uint8_t appAdvDataConnDefault[] = {
    /* General, length of the LEN-field should be the length of the AdvDataType + AdvData. So the lengthfield itself is not part of the lengthfield */
    /* Advertisement flags, length is two */
    0x02,
    DM_ADV_TYPE_FLAGS,
    DM_FLAG_LE_BREDR_NOT_SUP,
    /* Prefered connection interval, len 5 */
    0x05,
    DM_ADV_TYPE_CONN_INTERVAL,
    /* Min prefered connection interval LSB, MSB*/
    0x10,
    0x00,
    /* Max prefered connection interval LSB, MSB*/
    0x20,
    0x00,
    /* Manufacturer, len 3 */
    0x03,
    DM_ADV_TYPE_MANUFACTURER,
    0x53,
    0x04,
    /* 128-bit UUID Service, len 17 */
    0x11,
    DM_ADV_TYPE_128_UUID,
    ATT_UUID_P1_SERVICE
};

static uint8_t appScanDataDefault[] = {
    /* General, length of the LEN-field should be the length of the ScanDataType + ScanData. So the lengthfield itself is not part of the lengthfield */
    /* Device local name */
    DEFAULT_SCAN_NAME_LEN + 1,
    DM_ADV_TYPE_LOCAL_NAME,
    DEFAULT_SCAN_NAME
};

static profileConfig_t appProfileConfig = {
    .enabledServices    = PROFILE_CONFIG_NONE,
    .advMode            = DM_ADV_NONE,
};

static BleModule_phyCfg_t appPhyCfgDefault = {
    .selectedPhy = BleModule_SelectedPhy_All,
    .txPhyTypeMask = BleModule_PhyTypeMask_1Mbit,
    .rxPhyTypeMask = BleModule_PhyTypeMask_1Mbit,
};
/* </CodeGenerator Placeholder> StaticData */

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticFunctionPrototypes */
/* </CodeGenerator Placeholder> StaticFunctionPrototypes */

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticFunctionDefinitions */
/* </CodeGenerator Placeholder> StaticFunctionDefinitions */

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void BleModule_LoadDefault(UInt8 numConnections, BleModule_Cfg_t* pConfig)
{
/* <CodeGenerator Placeholder> Implementation_BleModule_Init */
    UIntLoop i;

    MEMSET(pConfig, 0, (sizeof(pConfig)*numConnections));
    for(i=0; i < numConnections; i++)
    {
        pConfig[i].pAppSlaveCfg             = &appSlaveCfgDefault;
        pConfig[i].pAppAdvCfg               = &appAdvCfgDefault;
        pConfig[i].pAppExtAdvCfg            = &appExtAdvCfgDefault;
        pConfig[i].pAppUpdateCfg            = &appUpdateCfgDefault;
        pConfig[i].pAppSecCfg_Peripheral    = &appSecCfgDefault;
        pConfig[i].bondable                 = true;
        pConfig[i].pPhyConfig               = &appPhyCfgDefault;
        pConfig[i].appAdvDataConn           = appAdvDataConnDefault;
        pConfig[i].appAdvDataConnLen        = sizeof(appAdvDataConnDefault);
        pConfig[i].appScanDataConn          = appScanDataDefault;
        pConfig[i].appScanDataConnLen       = sizeof(appScanDataDefault);
        pConfig[i].appAdvDataDisc           = appAdvDataDiscDefault;
        pConfig[i].appAdvDataDiscLen        = sizeof(appAdvDataDiscDefault);
        pConfig[i].appScanDataDisc          = appScanDataDefault;
        pConfig[i].appScanDataDiscLen       = sizeof(appScanDataDefault);
        pConfig[i].pAppProfileCfg           = &appProfileConfig;
        pConfig[i].pSmpCfg                  = &smpCfgDefault;
        gpBlePeripheralConnectionStm_SetStateConf(gpBlePeripheralConnectionStm_ConfigAutoRecon, i);
    }
/* </CodeGenerator Placeholder> Implementation_BleModule_Init */
}
