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

#ifndef _GPBLELLCPPROCEDURES_DEFS_H_
#define _GPBLELLCPPROCEDURES_DEFS_H_

#if defined(GP_DIVERSITY_ROM_CODE)
#include "gpBleLlcpProcedures_RomCode_defs.h"
#else //defined(GP_DIVERSITY_ROM_CODE)

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

void gpBleLlcpProcedures_EncryptionInit(void);
void gpBleLlcpProcedures_UpdateCommonInit(gpHal_BleCallbacks_t* pCallbacks);
void gpBleLlcpProcedures_ConnectionUpdateInit(void);
void gpBleLlcpProcedures_ChannelMapUpdateInit(void);
void gpBleLlcpProcedures_PhyUpdateInit(void);
void gpBleLlcpProcedures_DataLengthExchangeInit(void);
void gpBleLlcpProcedures_FeatureExchangeInit(void);
void gpBleLlcpProcedures_PingInit(void);
void gpBleLlcpProcedures_VersionExchangeInit(void);
void gpBleLlcpProcedures_AclTerminationInit(void);
void gpBleLlcpProcedures_MinUsedChannelsInit(void);

#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
void gpBleLlcpProcedures_CteInit(void);
#endif //GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED



void gpBleLlcpProcedures_ChannelMapUpdateReset(Bool firstReset);
void gpBleLlcpProcedures_AclTerminationReset(void);
void gpBleLlcpProcedures_DataLengthExchangeReset(void);

Ble_LlcpFrameworkAction_t Ble_LlcpCommonUnexpectedPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode, gpPd_Loh_t* pPdLoh);
void Ble_LlcpStopLastScheduledConnEventCount(Ble_LlcpLinkContext_t* pContext);
void gpBleLlcpProcedures_MinUsedChannelsChannelMapUpdated(Ble_LlcpLinkContext_t* pContext);
/*****************************************************************************
 *                    Static Function Prototypes
*****************************************************************************/

#if defined(GP_DIVERSITY_JUMPTABLES) && defined(GP_DIVERSITY_ROM_CODE)
#include "gpBleLlcpProcedures_CodeJumpTableFlash_Defs_defs.h"
#include "gpBleLlcpProcedures_CodeJumpTableRom_Defs_defs.h"
#endif // defined(GP_DIVERSITY_JUMPTABLES) && defined(GP_DIVERSITY_ROM_CODE)

/* JUMPTABLE_FLASH_FUNCTION_DEFINITIONS_START */
/* JUMPTABLE_ROM_FUNCTION_DEFINITIONS_START */

void Ble_LlcpTerminationGetCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t* pOpcode, UInt8* ctrDataLength, UInt8* pCtrData);
Ble_LlcpFrameworkAction_t Ble_LlcpTerminationStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode);

void Ble_LlcpVersionGetCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t* pOpcode, UInt8* ctrDataLength, UInt8* pCtrData);
Ble_LlcpFrameworkAction_t Ble_LlcpVersionStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode);


/* JUMPTABLE_FLASH_FUNCTION_DEFINITIONS_END */
/* JUMPTABLE_ROM_FUNCTION_DEFINITIONS_END */

/*****************************************************************************
 *                    Service Function Definitions
 *****************************************************************************/

#endif //defined(GP_DIVERSITY_ROM_CODE)

#endif //_GPBLELLCPPROCEDURES_DEFS_H_

