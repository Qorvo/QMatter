/*
 *   Copyright (c) 2020, Qorvo Inc
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

#ifndef _BLEOTACLIENT_H_
#define _BLEOTACLIENT_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

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
 *                    Global Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

void BleOtaClient_Init(void);
void BleOtaClient_cbOpenConnection(UInt8 linkId);
void BleOtaClient_cbCloseConnection(UInt8 linkId);
void BleOtaClient_cbBondedConnection(UInt8 linkId);
Bool BleOtaClient_cbDataIndication(UInt8 linkId, UInt16 length, UInt8* pData);

#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  //_BLEOTACLIENT_H_
