/*
 * Copyright (c) 2015-2016, GreenPeak Technologies
 * Copyright (c) 2017, Qorvo Inc
 *
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
 * $Header: //depot/release/Embedded/Components/Qorvo/BleController/v2.10.2.0/comps/gpHci/src/gpHci.c#1 $
 * $Change: 187624 $
 * $DateTime: 2021/12/20 10:58:50 $
 *
 */


/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

//#define GP_LOCAL_LOG

#include "gpHci.h"
#include "gpBle.h"
#include "gpLog.h"
#include "gpHal.h"
#include "gpHci_defs.h"

#if defined(GP_DIVERSITY_BLE_MASTER) || defined(GP_DIVERSITY_BLE_SLAVE)
#include "gpBleDataTx.h"
#include "gpBleDataChannelTxQueue.h"
#endif //GP_DIVERSITY_BLE_MASTER || GP_DIVERSITY_BLE_SLAVE



/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID GP_COMPONENT_ID_HCI

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

#ifndef GP_COMP_UNIT_TEST
static gpHci_CrossOver_t gpHci_CrossOverBuffer;
#endif
static Bool Hci_CommandFlowEnabled;

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/


/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/


/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
void gpHci_CompReset(void)
{
    // Set all HCI Related parameters to it's default value
    Hci_CommandFlowEnabled = true;

    gpHci_SequencerReset();
}

void gpHci_Init(void)
{
    gpHci_SequencerInit();
}

#if defined(GP_DIVERSITY_BLE_MASTER) || defined(GP_DIVERSITY_BLE_SLAVE)
void gpHci_processData(gpHci_ConnectionHandle_t connHandle, UInt16 dataLength, UInt8* pData)
{

#ifndef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
    gpBle_DataTxRequest( connHandle, dataLength, pData);
#else
    gpBle_DataTxRequest( connHandle, dataLength, pData, NULL, NULL);
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */
}
#endif //GP_DIVERSITY_BLE_MASTER || GP_DIVERSITY_BLE_SLAVE


#ifndef GP_COMP_UNIT_TEST
Bool gpHci_commandsEnabled(void)
{
    return Hci_CommandFlowEnabled;
}

void gpHci_stopCommands(void)
{
    gpHci_EventCbPayload_t buf;

    Hci_CommandFlowEnabled = false;
    buf.commandCompleteParams.numHciCmdPackets = 0x00;
    buf.commandCompleteParams.opCode = gpHci_OpcodeNoOperation;
    buf.commandCompleteParams.result = 0;

#ifdef GP_HCI_DIVERSITY_HOST_SERVER
    WcBleHost_gpHci_CommandCompleteHandler(gpHci_EventCode_CommandComplete, &buf.commandCompleteParams);
#endif // GP_HCI_DIVERSITY_HOST_SERVER
}

void gpHci_resumeCommands(void)
{
    Hci_CommandFlowEnabled = true;

    if(!gpHci_ExecuteCrossOverCommand()) //Try executing stored command
    {
        gpHci_EventCbPayload_t buf;

        buf.commandCompleteParams.numHciCmdPackets = GP_HCI_COMMAND_CREDIT_NUMBER;
        buf.commandCompleteParams.opCode = gpHci_OpcodeNoOperation;

#ifdef GP_HCI_DIVERSITY_HOST_SERVER
    WcBleHost_gpHci_CommandCompleteHandler(gpHci_EventCode_CommandComplete, &buf.commandCompleteParams);
#endif // GP_HCI_DIVERSITY_HOST_SERVER
    }
}

Bool gpHci_StoreCrossOverCommand(gpHci_CommandOpCode_t opCode,UInt8 totalLength,UInt8 * pData)
{
    Bool result = false;

    if( gpHci_CrossOverBuffer.totalLength == 0xFF )
    {
        //buffer is free - store command
        gpHci_CrossOverBuffer.OpCode = opCode;
        gpHci_CrossOverBuffer.totalLength = totalLength;
        MEMCPY(gpHci_CrossOverBuffer.payload, pData, gpHci_CrossOverBuffer.totalLength);

        result = true;
    }

    return result;
}

Bool gpHci_ExecuteCrossOverCommand(void)
{
    Bool executed = false;

    if( gpHci_CrossOverBuffer.totalLength != 0xFF )
    {
#ifdef GP_DIVERSITY_BLE_EXECUTE_CMD_HCIWRAPPER
        gpHci_processCommand(gpHci_CrossOverBuffer.OpCode, gpHci_CrossOverBuffer.totalLength, gpHci_CrossOverBuffer.payload);
#else
        /* FIXME: is this only needed when using HCI serverwrapper? Why is this CrossOverCommand stuff needed? */
        GP_LOG_PRINTF("ExecuteCrossOverCommand skipped",0);
#endif // GP_DIVERSITY_BLE_EXECUTE_CMD_HCIWRAPPER
        gpHci_CrossOverBuffer.totalLength = 0xFF;

        executed = true;
    }

    return executed;
}
#endif //ndef GP_COMP_UNIT_TEST

