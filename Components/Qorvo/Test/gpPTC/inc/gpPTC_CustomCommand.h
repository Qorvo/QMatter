/*
 * Copyright (c) 2017, Qorvo Inc
 *
 * ptc_customcommand.h
 *
 *  The file contains the types of the customcommands
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
#ifndef _GPPTC_CUSTOM_COMMAND_H_
#define _GPPTC_CUSTOM_COMMAND_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
/* allow for 10 custom commands */
#ifndef GP_PTC_NUM_CUSTOM_COMMANDS
#define GP_PTC_NUM_CUSTOM_COMMANDS 10
#endif

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/
/** @brief The gpPTC_CustomCommandId_t provides a zero-based identifier for custom commands. */
typedef UInt8 gpPTC_CustomCommandId_t;

/** @brief The gpPTC_CustomCommand_t defines the function type for any custom commands registered by the application (see gpPtcDut_RegisterCustomCommand()). */
typedef Bool (*gpPTC_CustomCommand_t)(UInt8 lenIn, UInt8 *pDataIn, UInt8 *pLenOut, UInt8 *pDataOut);

/*****************************************************************************
*                    Public Function Definitions
*****************************************************************************/

/**
 * @brief Register a custom command.
 *
 * Register a custom command to extend standard PTC functionality.
 * This method should be called from the testmode initialization step in \c Application_Init(). The handler is implemented by the application.
 * Up to ::GP_PTC_NUM_CUSTOM_COMMANDS can be registered, identified by a 0-based index.
 *
 * @param id Id for the custom command to register.
 * @param handler Function to execute when the custom command is called.
 * @return True if the custom command was registered successfully, false otherwise.
 */
Bool gpPTC_RegisterCustomCommand(gpPTC_CustomCommandId_t id, gpPTC_CustomCommand_t handler);

Bool gpPTC_RunCustomCommand(gpPTC_CustomCommandId_t id, UInt8 lenIn, UInt8 *pDataIn, UInt8 *pLenOut, UInt8 *pDataOut);

#endif // //GP_PTC_CUSTOM_COMMAND_H_
