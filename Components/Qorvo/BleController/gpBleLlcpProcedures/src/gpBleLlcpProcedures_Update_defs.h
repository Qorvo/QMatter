/*
 * Copyright (c) 2017, Qorvo Inc
 *
 * This software is owned by Qorvo Inc
 * and protected under applicable copyright laws.
 * It is delivered under the terms of the license
 * and is intended and supplied for use solely and
 * exclusively with products manufactured by\
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
 * $Header: //depot/release/Embedded/Components/Qorvo/BleController/v2.10.2.0/comps/gpBleLlcpProcedures/src/gpBleLlcpProcedures_Update_defs.h#1 $
 * $Change: 187624 $
 * $DateTime: 2021/12/20 10:58:50 $
 *
 */

#ifndef _GPBLELLCPPROCEDURES_UPDATE_DEFS_H_
#define _GPBLELLCPPROCEDURES_UPDATE_DEFS_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBleLlcpFramework.h"

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
 *                    Component Function Definitions
 *****************************************************************************/

UInt16 Ble_LlcpCalculateProcedureInstant(Ble_LlcpLinkContext_t* pContext, UInt16 currentEventCount);

// instant functions
void Ble_LlcpConfigureLastScheduledConnEvent(Ble_LlcpProcedureContext_t* pProcedure, UInt16 instant); // deprecated
void Ble_LlcpConfigureLastScheduledConnEventAfterCurrent(Ble_LlcpProcedureContext_t* pProcedure, UInt16 currentEventCount, UInt16 instant);
void Ble_LlcpConfigureLastScheduledConnEventAfterPassed(Ble_LlcpProcedureContext_t* pProcedure, UInt16 passedEventCount, UInt16 instant);
Bool Ble_LlcpInstantValid(UInt16 instant, UInt16 currentConnEventCount);
void Ble_LlcpInstantInvalidFollowUp(void* pArg);
UInt16 Ble_LlcpGetPduConnEventCount(Ble_LlcpLinkContext_t* pContext, gpPd_Loh_t* pPdLoh);

void Ble_LlcpConnectionUpdateInstantPassed(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure);
void Ble_LlcpChannelMapUpdateInstantPassed(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure);
void Ble_LlcpPhyUpdateInstantPassed(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure);

#endif //_GPBLELLCPPROCEDURES_UPDATE_DEFS_H_

