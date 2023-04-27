/*
 *   Copyright (c) 2019, Qorvo Inc
 *
 *   BleModule_Dat is the interface Dat functionalities. This file is intended to be modified by the customer according to the customers needs.
 *   Implementation of BleModule_Dat
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

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLEMODULE_DAT

/* <CodeGenerator Placeholder> General */

#define GP_COMPONENT_ID_BLEMODULE_DAT GP_COMPONENT_ID_BLEMODULE

/* </CodeGenerator Placeholder> General */

/* <CodeGenerator Placeholder> Includes */

#include "BleModule.h"
#include "BleModule_Defs.h"
#include "gpLog.h"
#include "gpPoolMem.h"
#include "gpSched.h"
#include "svc_wp.h"

#ifdef  GP_BLEPERIPHERAL_DIVERSITY_WDXS
#include "wdxs_api.h"
#endif //GP_BLEPERIPHERAL_DIVERSITY_WDXS

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
/** @struct BleModule_cbDatConf_t
 *  @brief Dat Confirmation Parameters
 */
typedef struct
{
    UInt8 linkId;
    BleModule_Result_t     status;
} BleModule_cbDatConf_t;

/** @struct BleModule_cbDatInd_t
 *  @brief Dat Indication parameters
*/
typedef struct
{
    UInt8 linkId;
    UInt16 len;
    UInt8* pValue;
    BleModule_Result_t status;
} BleModule_cbDatInd_t;
/* </CodeGenerator Placeholder> TypeDefinitions */

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticData */
/* </CodeGenerator Placeholder> StaticData */

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticFunctionPrototypes */
static BleModule_Result_t BleModule_DatSendData(dmConnId_t connId, uint16_t length, uint8_t* pData);
static void BleModule_cbDatConfirm_Sched(void* datConfSched);
static void BleModule_cbDatIndication_Sched(void* datIndSched);
/* </CodeGenerator Placeholder> StaticFunctionPrototypes */

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticFunctionDefinitions */
static BleModule_Result_t BleModule_DatSendData(dmConnId_t connId, uint16_t length, uint8_t* pData)
{
    BleModule_Result_t result;

    if (AttsCccEnabled(connId, DATS_WP_DAT_CCC_IDX))
    {
        /* send notification only when CCC is enabled */
        GP_LOG_PRINTF("Send notification",0);

        AttsHandleValueNtf(connId, WP_DAT_HDL, length, pData);
        result = BleModule_Success;
    }
    else
    {
        result = BleModule_CccNotEnabled;
    }
    return result;
}

static void BleModule_cbDatConfirm_Sched(void* datConfSched)
{
    BleModule_cbDatConf_t datConf = *((BleModule_cbDatConf_t*)datConfSched);
    gpPoolMem_Free(datConfSched);

    BleModule_cbDatDataConfirm(datConf.linkId, datConf.status);
}

static void BleModule_cbDatIndication_Sched(void* datIndSched)
{
    BleModule_cbDatInd_t datInd = *((BleModule_cbDatInd_t*)datIndSched);
    BleModule_cbDatDataIndication(datInd.linkId,datInd.len,datInd.pValue,datInd.status);
    gpPoolMem_Free(datInd.pValue);
    gpPoolMem_Free(datIndSched);
}
/* </CodeGenerator Placeholder> StaticFunctionDefinitions */

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void BleModule_DatDataRequest(UInt8 linkId, UInt16 length, UInt8* pData)
{
/* <CodeGenerator Placeholder> Implementation_BleModule_DatDataRequest */

    //Get a list of connection identifiers of open connections
    dmConnId_t openConnList[DM_CONN_MAX];
    UInt8 openConn = AppConnOpenList(openConnList);
    BleModule_Result_t result;
    dmConnId_t connId;
    result = BleModule_LinkIdInvalid;

    if(openConn)
    {
        if (linkId != GP_BLEMODULE_LINKID_BROADCAST)
        {
            result = BleModuleItf_GetConnIdByLinkId(linkId, &connId);
        }

        UInt8 sent;
        UInt8 i;

        //loop over all open connections and send data
        sent = 0;
        for(i=0; i < openConn; i++)
        {
            if((GP_BLEMODULE_LINKID_BROADCAST == linkId) || /*Sending to all connections*/
               ((openConnList[i] == connId) && (result == BleModule_Success))) /*Send to specific connection*/
            {
                //Check if data was sent succesfully
                result = BleModule_DatSendData(openConnList[i], length, pData);
                sent++;
            }
        }
        if(sent == 0)
        {
            //No valid connection found
            result = BleModule_LinkIdInvalid;
        }
    }
    else
    {
        result = BleModule_LinkNotConnected;
    }
    BleModule_cbDatConf_t* datConf = GP_POOLMEM_MALLOC(sizeof(BleModule_cbDatConf_t));
    datConf->linkId=linkId;
    datConf->status=result;
    gpSched_ScheduleEventArg(0, BleModule_cbDatConfirm_Sched,(void*) datConf);

/* </CodeGenerator Placeholder> Implementation_BleModule_DatDataRequest */
}

/*****************************************************************************
 *                    Callback Function Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> CallbackFunctionDefinitions */

uint8_t BleModule_DatWpWriteCback(dmConnId_t connId, UInt16 handle,
                                UInt8 operation, UInt16 offset, UInt16 len,
                                UInt8* pValue, attsAttr_t* pAttr)
{
    /* print received data */
    //APP_TRACE_INFO0((const char*) pValue);
    BleModule_cbDatInd_t *datInd = GP_POOLMEM_MALLOC(sizeof( BleModule_cbDatInd_t));
    UInt8* recValue = GP_POOLMEM_MALLOC(len);
    MEMCPY(recValue, pValue, len);
    datInd->linkId=BleModuleItf_GetLinkIdByConnId(connId);
    datInd->len=len;
    datInd->pValue=recValue;
    datInd->status=BleModule_Success;
    gpSched_ScheduleEventArg(0, BleModule_cbDatIndication_Sched,(void*) datInd);

    return ATT_SUCCESS;
}

/* </CodeGenerator Placeholder> CallbackFunctionDefinitions */
