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
 * $Header: //depot/release/Embedded/Components/Qorvo/BleController/v2.10.2.0/comps/gpBleLlcpProcedures/src/gpBleLlcpProcedures.c#1 $
 * $Change: 187624 $
 * $DateTime: 2021/12/20 10:58:50 $
 *
 */

//#define GP_LOCAL_LOG

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLELLCPPROCEDURES

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBle.h"
#include "gpBleComps.h"
#include "gpBleLlcpProcedures.h"
#include "gpBleLlcpProcedures_defs.h"

#include "gpBle_defs.h"
#include "gpSched.h"
#include "gpLog.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

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

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

Ble_LlcpFrameworkAction_t Ble_LlcpCommonUnexpectedPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode, gpPd_Loh_t* pPdLoh)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;

    switch(rxOpcode)
    {
        case gpBleLlcp_OpcodeUnknownRsp:
        {
            if(pPdLoh->length >= 1)
            {
                gpBleLlcp_Opcode_t unknownOpcode = gpPd_ReadByte(pPdLoh->handle, pPdLoh->offset);

                if(unknownOpcode == pProcedure->currentTxPdu)
                {
                    pProcedure->result = gpHci_ResultUnsupportedRemoteFeatureUnsupportedLmpFeature;
                    action = Ble_LlcpFrameworkActionStop;
                }
            }

            break;
        }
        case gpBleLlcp_OpcodeRejectExtInd:
        {
            if(pPdLoh->length >= 1)
            {
                gpBleLlcp_Opcode_t rejectOpcode = gpPd_ReadByte(pPdLoh->handle, pPdLoh->offset);

                if(rejectOpcode == pProcedure->currentTxPdu)
                {
                    if(pPdLoh->length >= 2)
                    {
                        pProcedure->result = gpPd_ReadByte(pPdLoh->handle, pPdLoh->offset+1);
                    }
                    else
                    {
                        pProcedure->result = gpHci_ResultUnspecifiedError;
                    }
                    action = Ble_LlcpFrameworkActionStop;
                }
            }

            break;
        }
        case gpBleLlcp_OpcodeRejectInd:
        {
            if(pPdLoh->length >= 1)
            {
                pProcedure->result = gpPd_ReadByte(pPdLoh->handle, pPdLoh->offset);
                action = Ble_LlcpFrameworkActionStop;
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return action;
}

void gpBleLlcpProcedures_Init(gpHal_BleCallbacks_t* pCallbacks)
{
    // Initialize mandatory procedures
    gpBleLlcpProcedures_UpdateCommonInit(pCallbacks);
    gpBleLlcpProcedures_ConnectionUpdateInit();
    gpBleLlcpProcedures_ChannelMapUpdateInit();
    gpBleLlcpProcedures_FeatureExchangeInit();
    gpBleLlcpProcedures_VersionExchangeInit();
    gpBleLlcpProcedures_AclTerminationInit();

    // Initialize optional procedures





#if defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED)
    gpBleLlcpProcedures_CteInit();
#endif /* GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED */


}

void gpBleLlcpProcedures_Reset(Bool firstReset)
{
    gpBleLlcpProcedures_ChannelMapUpdateReset(firstReset);


    gpBleLlcpProcedures_AclTerminationReset();
}
