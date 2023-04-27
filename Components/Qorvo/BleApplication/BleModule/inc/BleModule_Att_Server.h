/*
 *   Copyright (c) 2018, Qorvo Inc
 *
 *
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

#ifndef _BLEMODULE_ATT_SERVER_H_
#define _BLEMODULE_ATT_SERVER_H_

/// @file "BleModule.h"
/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"

#ifdef GP_BLE_ATT_SERVER_DAT
#include "server/BleModule_Dats.h"
#endif /* GP_BLE_ATT_SERVER_DAT */

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

/** @brief Index of ccc register call back array */
enum
{
    GATT_SC_CCC_IDX,                    /*! Service changed characteristic */
#ifdef GP_BLE_ATT_SERVER_DAT
    DATS_WP_DAT_CCC_IDX,
#endif // GP_BLE_ATT_SERVER_DAT
#ifdef GP_BLEPERIPHERAL_DIVERSITY_WDXS
    WDXS_DC_CH_CCC_IDX,             /*! WDXS DC service, service changed characteristic */
    WDXS_FTC_CH_CCC_IDX,            /*! WDXS FTC  service, service changed characteristic */
    WDXS_FTD_CH_CCC_IDX,            /*! WDXS FTD service, service changed characteristic */
    WDXS_AU_CH_CCC_IDX,             /*! WDXS AU service, service changed characteristic */
#endif
    NUM_CCC_IDX
};

/* </CodeGenerator Placeholder> Public TypeDefs */

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//Requests
void BleModule_Att_Server_Init(void);
void BleModule_InitCccTable(void);
#ifdef GP_BLEPERIPHERAL_DIVERSITY_WDXS
void BleModule_cbDatAtt(attEvt_t *pEvt);
#endif
#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_BLEMODULE_ATT_SERVER_H_

