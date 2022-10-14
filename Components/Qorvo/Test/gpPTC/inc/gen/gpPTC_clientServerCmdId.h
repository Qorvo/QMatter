/*
 * Copyright (c) 2020, Qorvo Inc
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
 */

/** @file "gpPTC_clientServerCmdId.h"
 *
 *  gpPTC
 *
 *  Client Server Link Command IDs
*/

#ifndef _GPPTC_CLIENTSERVERCMDID_H_
#define _GPPTC_CLIENTSERVERCMDID_H_

/*****************************************************************************
 *                    Common timeout
 *****************************************************************************/

#ifndef GPPTC_GPCOMTIMEOUT_US
#define GPPTC_GPCOMTIMEOUT_US 10000000UL //10s
#endif //GPPTC_GPCOMTIMEOUT_US

/*****************************************************************************
 *                    Component Specific Command IDs
 *****************************************************************************/

#define gpPTC_SetClientIDRequest_CmdId                               0x02 /*02*/
#define gpPTC_Discover_CmdId                                         0x03 /*03*/
#define gpPTC_ResetDUT_CmdId                                         0x06 /*06*/
#define gpPTC_GetDUTApiVersion_CmdId                                 0x07 /*07*/
#define gpPTC_GetDUTInfoRequest_CmdId                                0x08 /*08*/
#define gpPTC_SetAttributeRequest_CmdId                              0x0a /*10*/
#define gpPTC_GetAttributeRequest_CmdId                              0x0b /*11*/
#define gpPTC_SetModeRequest_CmdId                                   0x0c /*12*/
#define gpPTC_SetByteDataForAttributeRequest_CmdId                   0x0d /*13*/
#define gpPTC_GetModeRequest_CmdId                                   0x0e /*14*/
#define gpPTC_ExecuteCustomCommand_CmdId                             0x12 /*18*/
#define gpPTC_DiscoverIndication_CmdId                               0x04 /*04*/
#define gpPTC_RXPacketIndication_CmdId                               0x0f /*15*/
#define gpPTC_DataConfirm_CmdId                                      0x10 /*16*/
#define gpPTC_EDConfirm_CmdId                                        0x11 /*17*/

/*****************************************************************************
 *                    Fixed Command IDs
 *****************************************************************************/

#define gpPTC_Acknowledge_CmdId                                   0xfe /*254*/
#define gpPTC_GetServerCompatibilityNumber_CmdId                  0xfd /*253*/

#endif //_GPPTC_CLIENTSERVERCMDID_H_

