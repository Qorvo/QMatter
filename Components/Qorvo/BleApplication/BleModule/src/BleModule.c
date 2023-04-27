/*
 * Copyright (c) 2019, Qorvo Inc
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

/** @file "BleModule.c"
 *
 *  BleModule is the interface to the host-stack. This file is intended to be modified by the customer according to the customers needs.
 *
 *  Implementation of BleModule
*/

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLEMODULE

/* <CodeGenerator Placeholder> General */
/* </CodeGenerator Placeholder> General */

#include "cordioBleHost.h"
#include "dm_api.h"
#include "bstream.h"

#include "BleModule.h"
#include "BleModule_Defs.h"
/* <CodeGenerator Placeholder> Includes */
#include "gpLog.h"
#include "gpPoolMem.h"
#include "gpSched.h"
#include "gpNvm_ElemIf.h"
#include "gpAssert.h"
#ifdef GP_BLEMODULE_DIVERSITY_GPCOM_SERVER
#include "BleModule_server.h"
#endif // GP_BLEMODULE_DIVERSITY_GPCOM_SERVER

#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_SERVER
#include "svc_core.h"
#endif
/* </CodeGenerator Placeholder> Includes */

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/** @brief NVM Tags ID for the BleModule related NVM components */
#define BLE_MODULE_LOCAL_IRK_NVM_COMPONENT_ID GP_COMPONENT_ID
#define BLE_MODULE_LOCAL_IRK_TAG_ID 0
#define BLE_MODULE_LOCAL_IRK_UNIQUE_TAG_ID ((UInt16)(BLE_MODULE_LOCAL_IRK_NVM_COMPONENT_ID << 8) + BLE_MODULE_LOCAL_IRK_TAG_ID)

/* <CodeGenerator Placeholder> Macro */
/* </CodeGenerator Placeholder> Macro */

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> FunctionalMacro */
/* </CodeGenerator Placeholder> FunctionalMacro */

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> TypeDefinitions */
/* </CodeGenerator Placeholder> TypeDefinitions */

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticData */
static BleModule_BusyTask_t BLEBusy;
static BleModule_BsyCb_t busyCb = NULL;

static UInt8 BleModule_LocalIrk[16];
static const UInt8 BleModule_BleModule_LocalIrk_EmptyValuePattern[16]  = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/** @brief Definition of the NVM tags for the ActiveDeviceManager interface */
static const gpNvm_IdentifiableTag_t ROM BleModule_NvmSection[] FLASH_PROGMEM =
{
    {
        BLE_MODULE_LOCAL_IRK_UNIQUE_TAG_ID,
        (UInt8 *)&BleModule_LocalIrk,
        sizeof(BleModule_LocalIrk),
        gpNvm_UpdateFrequencyHigh,
        NULL,
        NULL
    }
};

/* </CodeGenerator Placeholder> StaticData */

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticFunctionPrototypes */
/* </CodeGenerator Placeholder> StaticFunctionPrototypes */

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticFunctionDefinitions */
/* </CodeGenerator Placeholder> StaticFunctionDefinitions */

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void BleModule_Init(void)
{
/* <CodeGenerator Placeholder> Implementation_BleModule_Init */
    cordioBleHost_Init();

    BleModule_ClearBusy(BleModule_BusyMaskAll);

#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_SERVER
    BleModule_Att_Server_Init();
#endif // CORDIO_APPFRAMEWORK_DIVERSITY_ATT_SERVER

#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT
    BleModule_Att_Client_Init();
#endif // CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT

#ifdef GP_BLEMODULE_CENTRAL
    BleModule_Central_Init();
#endif // GP_BLEMODULE_CENTRAL

#ifdef GP_BLEMODULE_PERIPHERAL
    BleModule_Peripheral_Init();
#endif // GP_BLEMODULE_PERIPHERAL

#ifdef GP_BLEMODULE_DIVERSITY_GPCOM_SERVER
    BleModule_InitServer();
#endif // GP_BLEMODULE_DIVERSITY_GPCOM_SERVER

    /* Start Cordio stack */
    DmDevReset();
/* </CodeGenerator Placeholder> Implementation_BleModule_Init */
}
#if !(defined(GP_COMP_UNIT_TEST))
void BleModule_RegisterCmnCb(BleModule_BsyCb_t callback)
{
/* <CodeGenerator Placeholder> Implementation_BleModule_RegisterCmnCb */
    busyCb = callback;
/* </CodeGenerator Placeholder> Implementation_BleModule_RegisterCmnCb */
}
#endif //!(defined(GP_COMP_UNIT_TEST))
Bool BleModule_GetBusy(BleModule_BusyTask_t busyTask)
{
/* <CodeGenerator Placeholder> Implementation_BleModule_GetBusy */
    return BLEBusy & busyTask;
/* </CodeGenerator Placeholder> Implementation_BleModule_GetBusy */
}
void BleModule_SetBusy(BleModule_BusyTask_t busyTask)
{
/* <CodeGenerator Placeholder> Implementation_BleModule_SetBusy */
    UInt32 BLEBusy_Previous = BLEBusy;

    BLEBusy |= busyTask;

    /* Trigger callback to application if not empty, and previous status was empty */
    if((BLEBusy_Previous == 0) && (BLEBusy != 0) && (busyCb != NULL))
    {
        (*busyCb)(true);
    }
/* </CodeGenerator Placeholder> Implementation_BleModule_SetBusy */
}
void BleModule_ClearBusy(BleModule_BusyTask_t busyTask)
{
/* <CodeGenerator Placeholder> Implementation_BleModule_ClearBusy */
    UInt32 BLEBusy_Previous = BLEBusy;

    BLEBusy &= ~busyTask;

    /* Trigger callback to application if not empty, and previous status was empty */
    if((BLEBusy_Previous != 0) && (BLEBusy == 0) && (busyCb != NULL))
    {
        (*busyCb)(false);
    }
/* </CodeGenerator Placeholder> Implementation_BleModule_ClearBusy */
}

void BleModule_SendAuthenticationResponse(UInt8 connId, uint8_t authDataLen, uint8_t *pAuthData)
{
/* <CodeGenerator Placeholder> Implementation_BleModule_SendPinAuthenticationResponse */
    DmSecAuthRsp(connId, authDataLen, pAuthData);
/* </CodeGenerator Placeholder> Implementation_BleModule_SendPinAuthenticationResponse */
}

void BleModule_SetPreferredPhy(BleModule_SelectedPhy_t selectedPhy, BleModule_PhyTypeMask_t txPhysMask, BleModule_PhyTypeMask_t rxPhysMask)
{
/* <CodeGenerator Placeholder> Implementation_BleModule_SetDefaultPhy */
    GP_LOG_PRINTF("Seting default PHY Options. selectedPhy %x, Preferred txMask %x, rxMask %x",0,selectedPhy,txPhysMask,rxPhysMask);

    /* 0x00 means The host _has preferences_ among _both_ Tx and Rx PHY. No translation from BleModule_SelectedPhy_All is needed */
    UInt8 allPhys = 0x00;
    if(BleModule_SelectedPhy_Tx == selectedPhy)
    {
        /*
        * User selected Tx Phy to be configured. So according to HCI ALL_PHYS parameter specs allPhys will be set
        * to value which means that the Host has _no preference_ among the _receiver PHYs_
        */
        allPhys = BM(gpHci_PhyDirectionMask_Rx);
    }
    else if (BleModule_SelectedPhy_Rx==selectedPhy)
    {
        /*
        * User selected Rx Phy to be configured. So according to HCI ALL_PHYS parameter specs allPhys will be set
        * to value which means that the Host has _no preference_ among the _trasnmitter PHYs_
        */
        allPhys = BM(gpHci_PhyDirectionMask_Tx);
    }

    DmSetDefaultPhy(allPhys, txPhysMask, rxPhysMask);
/* </CodeGenerator Placeholder> Implementation_BleModule_SetDefaultPhy */
}

void BleModule_GenerateSmpPinFromNumber(const UInt32 passkeyNumber, UInt8 pPinBuffer[SMP_PIN_LEN])
{
    /* <CodeGenerator Placeholder> BleModule_GenerateSmpPinFromNumber */
    pPinBuffer[0] = UINT32_TO_BYTE0(passkeyNumber);
    pPinBuffer[1] = UINT32_TO_BYTE1(passkeyNumber);
    pPinBuffer[2] = UINT32_TO_BYTE2(passkeyNumber);
    /* </CodeGenerator Placeholder> BleModule_GenerateSmpPinFromNumber */
}

void BleModule_RestoreAndSetLocalIrk(void)
{
    /* <CodeGenerator Placeholder> BleModule_RestoreAndSetLocalIrk */
    GP_LOG_PRINTF("INFO: BleModule_RestoreBleModule_LocalIrk", 0);

    gpNvm_RegisterElements(BleModule_NvmSection,
        sizeof(BleModule_NvmSection) / sizeof(gpNvm_IdentifiableTag_t));

    gpNvm_Restore(BLE_MODULE_LOCAL_IRK_NVM_COMPONENT_ID, BLE_MODULE_LOCAL_IRK_TAG_ID, BleModule_LocalIrk);

    if(MEMCMP(BleModule_LocalIrk, BleModule_BleModule_LocalIrk_EmptyValuePattern, sizeof(BleModule_LocalIrk))==0)
    {
        GP_LOG_SYSTEM_PRINTF("WARNING: BleModule_LocalIrk value was unknown in NVM Flash memory. Generating new value",0);
        SecRand(BleModule_LocalIrk, sizeof(BleModule_LocalIrk));
        gpNvm_Backup(BLE_MODULE_LOCAL_IRK_NVM_COMPONENT_ID, BLE_MODULE_LOCAL_IRK_TAG_ID, BleModule_LocalIrk);
    }

    GP_LOG_SYSTEM_PRINTF("INFO: BleModule_LocalIrk value: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", 0,
            BleModule_LocalIrk[0], BleModule_LocalIrk[1], BleModule_LocalIrk[2], BleModule_LocalIrk[3], BleModule_LocalIrk[4], BleModule_LocalIrk[5], BleModule_LocalIrk[6], BleModule_LocalIrk[7],
            BleModule_LocalIrk[8], BleModule_LocalIrk[9], BleModule_LocalIrk[10], BleModule_LocalIrk[11], BleModule_LocalIrk[12], BleModule_LocalIrk[13], BleModule_LocalIrk[14], BleModule_LocalIrk[15]);
    DmSecSetLocalIrk(BleModule_LocalIrk);
    /* </CodeGenerator Placeholder> BleModule_RestoreAndSetLocalIrk */
}

void BleModule_SetResolvableAddressGenerationTimeout(UInt16 timeout_s)
{
    /* <CodeGenerator Placeholder> BleModule_SetResolvableAddressGenerationTimeout */
    DmPrivSetResolvablePrivateAddrTimeout(timeout_s);
    /* </CodeGenerator Placeholder> BleModule_SetResolvableAddressGenerationTimeout */
}

void BleModule_SetAddressResolving(Bool enable)
{
    /* <CodeGenerator Placeholder> BleModule_SetAddressResolving */
    DmPrivSetAddrResEnable(enable);
#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_SERVER
#ifdef GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED
    //Enabling LLPrivacy (via GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED) is automatically adding Rpao characteristic
    //We need to remove this when useLinkLayerPrivacy is not enabled because otherwise connected peer
    //will detect RPAO and will configure Network Privacy Mode with us and will ignore any advertise
    //sent with public address (when llprivacy is not enabled we use only public addresses)
    if(enable)
    {
        SvcCoreGapAddRpaoCh();
    }
    else
    {
        SvcCoreGapRemoveRpaoCh();
    }
#endif
#endif
    /* </CodeGenerator Placeholder> BleModule_SetAddressResolving */
}

Bool BleModule_GetAddressResolvingStatus(void)
{
    /* <CodeGenerator Placeholder> BleModule_EnableAddressResolution */
    return DmLlPrivEnabled();
    /* </CodeGenerator Placeholder> BleModule_EnableAddressResolution */
}

void BleModule_RestorePrivacyModeToDevice(appDbHdl_t dbHdl)
{
    /* <CodeGenerator Placeholder> BleModule_RestorePrivacyModeToDevice */
    GP_ASSERT_DEV_EXT(dbHdl != APP_DB_HDL_NONE); /* should be checked before call */
    dmSecKey_t *pPeerKey = AppDbGetKey(dbHdl, DM_KEY_IRK, NULL);
    if (pPeerKey != NULL) /* we need to call AppDbGetKey anyway so lets check just to be extra safe */
    {
        GP_LOG_PRINTF("Setting privacy mode device for peer stored under dbHdl: %p", 0, dbHdl);
        DmPrivSetPrivacyMode(pPeerKey->irk.addrType, pPeerKey->irk.bdAddr, DM_PRIV_MODE_DEVICE);
    }
    else
    {
        GP_LOG_SYSTEM_PRINTF("ERROR: dbHdl (%p) with NULL IRK Key was requested to set privacy mode device", 0, dbHdl);
    }
    /* </CodeGenerator Placeholder> BleModule_RestorePrivacyModeToDevice */
}
