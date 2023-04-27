/*
 *   Copyright (c) 2018, Qorvo Inc
 *
 *
 *   Implementation of BleModule_Services
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
#ifdef GP_DIVERSITY_DEVELOPMENT
#define GP_LOCAL_LOG
#endif //GP_DIVERSITY_DEVELOPMENT

#include "BleModule.h"
#include "BleModule_Defs.h"

/* <CodeGenerator Placeholder> Includes */
#include "global.h"
#include "gpLog.h"

/* Includes from Host-stack are here needed to access app_db */
#include "wsf_types.h"
#include "wsf_msg.h"
#include "wsf_buf.h"
#include "svc_core.h"
#include "svc_dis.h"







#ifdef  GP_BLEPERIPHERAL_DIVERSITY_WDXS
#include "wsf_efs.h"
#include "svc_wdxs.h"
#include "wdxs_api.h"
#include "wdxs_main.h"
#include "wdxs_stream.h"
#endif //GP_BLEPERIPHERAL_DIVERSITY_WDXS


#include "app_api.h"
#include "dm_api.h"
#include "att_api.h"

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

static attsCccSet_t BleModule_CccSet[NUM_CCC_IDX];


/* </CodeGenerator Placeholder> StaticData */

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticFunctionPrototypes */

static void BleModule_ATTS_CccCback(attsCccEvt_t *pEvt);

/* </CodeGenerator Placeholder> StaticFunctionPrototypes */


/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticFunctionDefinitions */

void BleModule_ATTS_CccCback(attsCccEvt_t *pEvt)
{
    appDbHdl_t dbHdl;

    GP_LOG_PRINTF("BleModule_ATTS_CccCback",0);
    GP_LOG_PRINTF("idx = %d | handle = 0x%x | connId %d | value = %d",0,pEvt->idx, pEvt->handle, pEvt->hdr.param, pEvt->value);

    /* if CCC not set from initialization and there's a device record */
    dbHdl = AppDbGetHdl((dmConnId_t) pEvt->hdr.param);

    if((pEvt->handle != ATT_HANDLE_NONE) && (dbHdl != APP_DB_HDL_NONE))
    {
        /* store value in device database */
        AppDbSetCccTblValue(dbHdl, pEvt->idx, pEvt->value);
    }

}

/* </CodeGenerator Placeholder> StaticFunctionDefinitions */

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void BleModule_Att_Server_Init(void)
{
    //needs to be called otherwise GAP characteristic reads are not supported
    AttsInit();
    AttsIndInit();

    /* GATT/GAP service */
    SvcCoreAddGroup();

    /* Device Information Service */
    SvcDisAddGroup();





#ifdef GP_BLE_ATT_SERVER_DAT
    SvcWpCbackRegister((attsReadCback_t) NULL,(attsWriteCback_t) BleModule_DatWpWriteCback);
    SvcWpAddGroup();
#endif //GP_BLE_ATT_SERVER_DAT
#ifdef GP_BLEPERIPHERAL_DIVERSITY_WDXS
    WdxsHandlerInit(WsfOsSetNextHandler(WdxsHandler));
    wdxsStreamInit();
    WdxsSetCccIdx(WDXS_DC_CH_CCC_IDX, WDXS_AU_CH_CCC_IDX, WDXS_FTC_CH_CCC_IDX, WDXS_FTD_CH_CCC_IDX);
#endif //GP_BLEPERIPHERAL_DIVERSITY_WDXS



    BleModule_InitCccTable();
}

void BleModule_InitCccTable(void)
{
    /* Gatt */
    BleModule_CccSet[GATT_SC_CCC_IDX].handle = GATT_SC_CH_CCC_HDL;
    BleModule_CccSet[GATT_SC_CCC_IDX].valueRange = ATT_CLIENT_CFG_INDICATE;
    BleModule_CccSet[GATT_SC_CCC_IDX].secLevel = DM_SEC_LEVEL_NONE;




#ifdef GP_BLE_ATT_SERVER_DAT
    BleModule_CccSet[DATS_WP_DAT_CCC_IDX].handle = WP_DAT_CH_CCC_HDL;
    BleModule_CccSet[DATS_WP_DAT_CCC_IDX].valueRange = ATT_CLIENT_CFG_NOTIFY;
    BleModule_CccSet[DATS_WP_DAT_CCC_IDX].secLevel = DM_SEC_LEVEL_NONE;
#endif // GP_BLE_ATT_SERVER_DAT

#ifdef GP_BLEPERIPHERAL_DIVERSITY_WDXS
    BleModule_CccSet[WDXS_DC_CH_CCC_IDX].handle = WDXS_DC_CH_CCC_HDL;
    BleModule_CccSet[WDXS_DC_CH_CCC_IDX].valueRange = ATT_CLIENT_CFG_NOTIFY;
    BleModule_CccSet[WDXS_DC_CH_CCC_IDX].secLevel = DM_SEC_LEVEL_NONE;
    BleModule_CccSet[WDXS_FTC_CH_CCC_IDX].handle = WDXS_FTC_CH_CCC_HDL;
    BleModule_CccSet[WDXS_FTC_CH_CCC_IDX].valueRange = ATT_CLIENT_CFG_NOTIFY;
    BleModule_CccSet[WDXS_FTC_CH_CCC_IDX].secLevel = DM_SEC_LEVEL_NONE;
    BleModule_CccSet[WDXS_FTD_CH_CCC_IDX].handle = WDXS_FTD_CH_CCC_HDL;
    BleModule_CccSet[WDXS_FTD_CH_CCC_IDX].valueRange = ATT_CLIENT_CFG_NOTIFY;
    BleModule_CccSet[WDXS_FTD_CH_CCC_IDX].secLevel = DM_SEC_LEVEL_NONE;
    BleModule_CccSet[WDXS_AU_CH_CCC_IDX].handle = WDXS_AU_CH_CCC_HDL;
    BleModule_CccSet[WDXS_AU_CH_CCC_IDX].valueRange = ATT_CLIENT_CFG_NOTIFY;
    BleModule_CccSet[WDXS_AU_CH_CCC_IDX].secLevel = DM_SEC_LEVEL_NONE;
#endif // GP_BLEPERIPHERAL_DIVERSITY_WDXS


    AttsCccRegister(NUM_CCC_IDX, BleModule_CccSet, BleModule_ATTS_CccCback);
}

void BleModule_cbAttsCnf(attEvt_t *pMsg)
{

#ifdef GP_BLEPERIPHERAL_DIVERSITY_WDXS
    WdxsAttCback(pMsg);
#endif //GP_BLEPERIPHERAL_DIVERSITY_WDXS
}

void BleModule_cbDmCnf(dmEvt_t *pMsg)
{
#ifdef GP_BLEPERIPHERAL_DIVERSITY_WDXS
    WdxsProcDmMsg(pMsg);
#endif
}
