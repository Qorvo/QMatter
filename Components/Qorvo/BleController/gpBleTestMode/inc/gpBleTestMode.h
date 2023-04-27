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
 * $Header$
 * $Change$
 * $DateTime$
 *
 */

#ifndef _GPBLETESTMODE_H_
#define _GPBLETESTMODE_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpHci_Includes.h"
#include "gpBle.h"

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

void gpBle_TestModeInit(gpHal_BleCallbacks_t* pCallbacks);
void gpBle_TestModeReset(Bool firstReset);

/*****************************************************************************
 *                    Service Function Definitions
 *****************************************************************************/

gpHci_Result_t gpBle_LeReceiverTest(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);
gpHci_Result_t gpBle_LeTransmitterTest(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);
gpHci_Result_t gpBle_LeEnhancedReceiverTest(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);
gpHci_Result_t gpBle_LeEnhancedTransmitterTest(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);
gpHci_Result_t gpBle_LeReceiverTest_v3(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);
gpHci_Result_t gpBle_LeTransmitterTest_v3(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);
gpHci_Result_t gpBle_LeTestEnd(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);
gpHci_Result_t gpBle_SetVsdDirectTestTxPacketCountHelper(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);
gpHci_Result_t gpBle_SetVsdDirectTestModeAntennaHelper(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);

gpHci_Result_t gpBle_VsdEnhancedReceiverTest(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);
gpHci_Result_t gpBle_VsdLeReceiverTest_v3(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);

#endif // _GPBLETESTMODE_H_

