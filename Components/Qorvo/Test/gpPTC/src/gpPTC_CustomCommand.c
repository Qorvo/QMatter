/*
 * Copyright (c) 2016, GreenPeak Technologies
 * Copyright (c) 2017, Qorvo Inc
 *
 *   gpPTC_customcommand
 *   Implementation of gpPTC CustomCommands
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
 *  F
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpPTC.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_PTC

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

static gpPTC_CustomCommand_t gpPTC_CustomCommands[GP_PTC_NUM_CUSTOM_COMMANDS] = {0};

/*****************************************************************************
 *                    External Data Definition
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

Bool gpPTC_RegisterCustomCommand(gpPTC_CustomCommandId_t id, gpPTC_CustomCommand_t handler)
{
    if(id >= GP_PTC_NUM_CUSTOM_COMMANDS)
        return false;

    gpPTC_CustomCommands[id] = handler;
    return true;
}

Bool gpPTC_RunCustomCommand(gpPTC_CustomCommandId_t id, UInt8 lenIn, UInt8 *pDataIn, UInt8 *pLenOut, UInt8 *pDataOut)
{
    gpPTC_CustomCommand_t handler;

    if(id >= GP_PTC_NUM_CUSTOM_COMMANDS)
        return false;

    handler = gpPTC_CustomCommands[id];

    /* Pass execution to callback */
    if(handler)
        return handler(lenIn, pDataIn, pLenOut, pDataOut);
    /* Requested test is not implemented, simply return false */
    else
        return false;
}

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

