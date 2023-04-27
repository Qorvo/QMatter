/*
 * Copyright (c) 2017, Qorvo Inc
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

/** @file "gpBleResPrAddr.h"
 *
 *  Declarations of the public functions and enumerations of gpBleResPrAddr.
*/

#ifndef _GPBLERESPRADDR_H_
#define _GPBLERESPRADDR_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"
#include "gpHci_Includes.h"
#include "gpBle.h"

/* <CodeGenerator Placeholder> AdditionalIncludes */
#include "gpHal_Ble_Manual.h"
/* </CodeGenerator Placeholder> AdditionalIncludes */

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/** @macro BLE_RES_PR_INIT_RPA_INFO() */
#define BLE_RES_PR_INIT_RPA_INFO()                   {.rpaHandle = BLE_RES_PR_INIT_RPA_HANDLE(), .srcAddrIsLLRPA = false, .dstAddrIsLLRPA = false}
/** @macro BLE_RES_PR_SRC_IS_LL_RESOLVED_RPA(rpaInfo) */
#define BLE_RES_PR_SRC_IS_LL_RESOLVED_RPA(rpaInfo)     ((rpaInfo)->srcAddrIsLLRPA)
/** @macro BLE_RES_PR_DST_IS_LL_RESOLVED_RPA(rpaInfo) */
#define BLE_RES_PR_DST_IS_LL_RESOLVED_RPA(rpaInfo)     ((rpaInfo)->dstAddrIsLLRPA)
/** @macro BLE_RES_PR_SRC_IS_LL_GENERATED_RPA(rpaInfo) */
#define BLE_RES_PR_SRC_IS_LL_GENERATED_RPA(rpaInfo)     (BLE_RES_PR_SRC_IS_LL_RESOLVED_RPA(rpaInfo))
/** @macro BLE_RES_PR_DST_IS_LL_GENERATED_RPA(rpaInfo) */
#define BLE_RES_PR_DST_IS_LL_GENERATED_RPA(rpaInfo)     (BLE_RES_PR_DST_IS_LL_RESOLVED_RPA(rpaInfo))
/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//Requests
void gpBleResPrAddr_Init(void);

void gpBleResPrAddr_Reset(Bool firstReset);

gpHci_Result_t gpBle_LeAddDeviceToResolvingList(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);

gpHci_Result_t gpBle_LeRemoveDeviceFromResolvingList(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);

gpHci_Result_t gpBle_LeClearResolvingList(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);

gpHci_Result_t gpBle_LeReadResolvingListSize(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);

gpHci_Result_t gpBle_VsdGeneratePeerResolvableAddress(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);

gpHci_Result_t gpBle_VsdGenerateLocalResolvableAddress(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);

gpHci_Result_t gpBle_LeSetAddressResolutionEnable(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);

gpHci_Result_t gpBle_LeSetResolvablePrivateAddressTimeout(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);

gpHci_Result_t gpBle_LeSetPrivacyMode(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);

gpHci_Result_t gpBle_VsdSetRpaPrand(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);

gpHci_Result_t gpBle_VsdReadResolvingListCurrentSize(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);

gpHci_Result_t gpBle_VsdSetResolvingListMaxSize(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);

//Indications

#if defined(GP_COMP_BLEADVERTISER)
gpHci_Result_t gpResPrAddr_cbAdvPrandTimeout(void);
#endif //defined(GP_COMP_BLEADVERTISER)

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_GPBLERESPRADDR_H_

