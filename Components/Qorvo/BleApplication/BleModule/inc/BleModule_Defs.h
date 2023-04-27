/*
 *   Copyright (c) 2018, Qorvo Inc
 *
 *   BleModule is the interface to the host-stack. This file is intended to be modified by the customer according to the customers needs.
 *   Declarations of the public functions and enumerations of BleModule.
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


#ifndef _BLEMODULE_DEFS_H_
#define _BLEMODULE_DEFS_H_

/// @file "BleModule_Defs.h"
/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"
#include "cordioAppFramework.h"
#include "BleModule.h"
/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Gets the Connection ID via the Link ID
*
*   @param linkId                    linkId related to the requested connId
*   @param connId                    Pointer to variable where to store the connId
*   @return result                   Returns succes if valid parameters where entered. If not an error code is returned
*/
BleModule_Result_t BleModuleItf_GetConnIdByLinkId(UInt8 linkId, dmConnId_t* connId);

/** @brief Gets the Link ID via the Connection ID
*
*   @param connId                    Connection ID
*   @return linkId                   linkId related to the requested connId
*/
BleModule_Result_t BleModuleItf_GetLinkIdByConnId(dmConnId_t connId);

/** @brief Load defaults for the Ble connection configuration.
*
*   This function should be defined in an application-specific _Config.c file
*
*   @param numConnections            Number of supported connections
*   @param pConfig                   Pointer to the list of configurations per connection
*/
void BleModule_LoadDefault(UInt8 numConnections, BleModule_Cfg_t* pConfig);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_BLEMODULE_DEFS_H_
