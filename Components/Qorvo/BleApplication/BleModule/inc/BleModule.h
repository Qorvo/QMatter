/*
 * Copyright (c) 2018, Qorvo Inc
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

/** @file "BleModule.h"
 *
 *  BleModule is the interface to the host-stack. This file is intended to be modified by the customer according to the customers needs.
 *
 *  Declarations of the public functions and enumerations of BleModule.
*/

#ifndef _BLEMODULE_H_
#define _BLEMODULE_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"
/* Includes from Host-stack are here needed to access app_db */
#include "wsf_types.h"
#include "wsf_msg.h"
#include "wsf_buf.h"
#include "app_api.h"
#include "dm_api.h"
#include "att_api.h"
#include "smp_api.h"
#include "gpHci_types.h"
/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

/** @enum BleModule_BusyTask_t */
//@{
/** @brief BleModule is busy sending streaming voice data */
#define BleModule_BusyMaskVoice                                0x00000001
/** @brief BleModule Mitm user input ongoing */
#define BleModule_BusyMaskMitm                                 0x00000002
/** @brief BleModule general mask to set/clear/test all statuses at once */
#define BleModule_BusyMaskAll                                  0xFFFFFFFF
/** @typedef BleModule_BusyTask_t
    @brief Enum signaling a busy status to the application,
*/
typedef UInt32                            BleModule_BusyTask_t;
//@}

/** @enum BleModule_WhitelistOp_t */
//@{
/** @brief Add devices to the whitelist */
#define BleModule_WhiteListAdd                                 0x01
/** @brief Remove devices from the whitelist */
#define BleModule_WhiteListRemove                              0x02
/** @brief Remove all devices from the whitelist */
#define BleModule_WhiteListClear                               0x03
/** @typedef BleModule_WhitelistOp_t
    @brief Enum to define whitelist operations to be performed
*/
typedef UInt8                             BleModule_WhitelistOp_t;
//@}

/** @enum BleModule_Result_t */
//@{
/** @brief Result success, is returned in case of successful transaction or the result of a status request function is positive. */
#define BleModule_Success                                      0x00
/** @brief This is returned when public API is called with an invalid linkId */
#define BleModule_LinkIdInvalid                                0x01
/** @brief Value returned when API is called while linkId is not open. */
#define BleModule_LinkNotConnected                             0x02
/** @brief Returned when a the CCC of an attribute to be send is not set. */
#define BleModule_CccNotEnabled                                0x03
/** @brief Returned when an API with multiple operations on called with no valid operation selected. */
#define BleModule_NoOperation                                  0x04
/** @brief Returned when an API is called with an invalid length parameter. */
#define BleModule_LengthError                                  0x05
/** @brief Returned when an operation is not allowed due to advertising active. */
#define BleModule_AdvertisingInProgressError                   0x06
/** @brief Returned when an operation is not allowed due to scanning active. */
#define BleModule_ScanningInProgressError                      0x07
/** @brief Parameters received are invalid */
#define BleModule_InvalidParams                                0x08
/** @brief The request cannot be handled */
#define BleModule_RequestNotProcessed                          0x09
/** @brief This fault occurs when connecting state is left or a new device connect during reconnection */
#define BleModule_UnexpectedLinkOpenLinkDropped                0x0a
/** @brief This fault occurs when there is no response on advertising/scanning */
#define BleModule_OpenFailNoResponse                           0x0b
/** @brief Failed to establish security or pairing */
#define BleModule_SecurityPairingFailed                        0x0c
/** @brief Failed to start scanning */
#define BleModule_StartScanningFailed                          0x0d
/** @brief An unspecified error has occcured */
#define BleModule_UnknownError                                 0xff
/** @typedef BleModule_Result_t
    @brief BleModule result type
*/
typedef UInt8                             BleModule_Result_t;
//@}

/** @enum BleModule_EncryptionMethod_t */
//@{
/** @brief New LTK was generated */
#define BleModule_NewLtkGenerated 0x01
/** @brief Existing LTK was used */
#define BleModule_ExistingLtkUsed 0x02
/** @typedef BleModule_EncryptionMethod_t
    @brief Enum to define two types of BLE Encryption used during pairing
*/
typedef UInt8                             BleModule_EncryptionMethod_t;
//@}

/** @enum BleModule_SecurityKeyType_t */
//@{
/** @brief LTK generated locally for this device */
#define BleModule_SecurityKeyTypeLocalLtk                       DM_KEY_LOCAL_LTK
/** @brief LTK received from peer device */
#define BleModule_SecurityKeyTypePeerLtk                        DM_KEY_PEER_LTK
/** @brief IRK and identity info of peer device */
#define BleModule_SecurityKeyTypeIrk                            DM_KEY_IRK
/** @brief CSRK of peer device */
#define BleModule_SecurityKeyTypeCsrk                           DM_KEY_CSRK
/** @typedef BleModule_SecurityKeyType_t
    @brief Enum to define security keys types exchanged during pairing
*/
typedef UInt8                             BleModule_SecurityKeyType_t;
//@}

/** @enum BleModule_SecurityLevel_t */
//@{
/** @brief Connection with no security */
#define BleModule_SecurityLevelNone                             DM_SEC_LEVEL_NONE
/** @brief Connection is encrypted with unauthenticated key */
#define BleModule_SecurityLevelEnc                              DM_SEC_LEVEL_ENC
/** @brief Connection is encrypted with authenticated key */
#define BleModule_SecurityLevelEncAuth                          DM_SEC_LEVEL_ENC_AUTH
/** @brief Connection is encrypted with LE Secure Connections */
#define BleModule_SecurityLevelEncLesc                          DM_SEC_LEVEL_ENC_LESC
/** @typedef BleModule_SecurityLevel
    @brief Enum to define Security level used during key exchange
*/
typedef UInt8                             BleModule_SecurityLevel_t;
//@}

/** @enum BleModule_PhyTypeMask_t */
//@{
/** @brief The Host prefers to use the LE 1M transmitter PHY (possibly among others) */
#define BleModule_PhyTypeMask_1Mbit                             GP_HCI_PHY_MASK_1MB
/** @brief The Host prefers to use the LE 2M transmitter PHY (possibly among others) */
#define BleModule_PhyTypeMask_2Mbit                             GP_HCI_PHY_MASK_2MB
/** @brief The Host prefers to use the LE Coded transmitter PHY (possibly among others) */
#define BleModule_PhyTypeMask_CodedBit                          GP_HCI_PHY_MASK_CODED
/** @typedef BleModule_PhyTypeMask_t
    @brief Enum to define selected LE PHYs types mask. Multiple preferrences can be set by using logical OR operator
*/
typedef UInt8                             BleModule_PhyTypeMask_t;
//@}

/** @enum BleModule_SelectedPhy_t */
//@{
/** @brief Both Phy (Tx and Rx) will be configured during setting preferred PHY procedure */
#define BleModule_SelectedPhy_All                               0x00
/** @brief Only Tx PHY will be configured during setting preferred PHY procedure */
#define BleModule_SelectedPhy_Tx                                0x01
/** @brief Only Rx PHY will be configured during setting preferred PHY procedure */
#define BleModule_SelectedPhy_Rx                                0x02
/** @typedef BleModule_SelectedPhy_t
    @brief Enum to define user friendly interface to select which PHY (Tx or RX) should be configured
*/
typedef UInt8                             BleModule_SelectedPhy_t;
//@}


/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define BLE_MODULE_LINK_ID_NONE                                0xFF
#define PAIRING_PIN_LEN                                        6
/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/


/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

#if defined GP_DIVERSITY_BLE_PERIPHERAL || defined GP_DIVERSITY_BLE_LEGACY_ADVERTISING
/** @struct profileConfig_t
 *  @brief Application level profile configuration
*/
typedef struct {
    UInt8  enabledServices;
    UInt8  advMode;
}profileConfig_t;
#endif //GP_DIVERSITY_BLE_PERIPHERAL || defined GP_DIVERSITY_BLE_LEGACY_ADVERTISING

#ifdef GP_BLE_MULTI_ADV_SUPPORTED
/** @struct advAddrConfig_t
 *  @brief advertising address configuration for multiple advertisement
*/
typedef struct {
    UInt8 address[BDA_ADDR_LEN];
    Bool  bUsePublicAddr;
}advAddrConfig_t;
#endif //GP_BLE_MULTI_ADV_SUPPORTED

/*! \brief Configurable parameters for PHY */
typedef struct
{
  BleModule_SelectedPhy_t      selectedPhy;                /*!< \brief Selected Phy to be configured */
  BleModule_PhyTypeMask_t      txPhyTypeMask;              /*!< \brief Preferred Tx PHY type mask */
  BleModule_PhyTypeMask_t      rxPhyTypeMask;              /*!< \brief Preferred Rx PHY type mask*/
} BleModule_phyCfg_t;

/*! \brief LinkLayer Privacy Configuration */
typedef struct
{
    Bool useLinkLayerPrivacy;       /*!< \brief link layer privacy feature should be used */
    UInt16 rpaGenerationIntervalS;  /*!< \brief Interval between address changes, in seconds */
} BleModule_linkLayerCfg_t;

/** @struct BleModule_Cfg_t
 *  @brief Application level connection configuration
*/
typedef struct BleModule_Cfg_s {
#if defined GP_DIVERSITY_BLE_PERIPHERAL || defined GP_DIVERSITY_BLE_LEGACY_ADVERTISING
    /* Configurations of application frame work, per connection */
    appSlaveCfg_t*              pAppSlaveCfg;
    appAdvCfg_t*                pAppAdvCfg;
    appExtAdvCfg_t*             pAppExtAdvCfg;
    appUpdateCfg_t*             pAppUpdateCfg;
    appSecCfg_t*                pAppSecCfg_Peripheral;
    bool_t                      bondable;
    profileConfig_t*            pAppProfileCfg;
#ifdef GP_BLE_MULTI_ADV_SUPPORTED
    advAddrConfig_t*            pAdvAddrCfg;
#endif //GP_BLE_MULTI_ADV_SUPPORTED
    /* Advertising data to be used in AppAdvSetData */
    UInt8*                      appAdvDataConn;
    UInt8                       appAdvDataConnLen;
    UInt8*                      appScanDataConn;
    UInt8                       appScanDataConnLen;
    UInt8*                      appAdvDataDisc;
    UInt8                       appAdvDataDiscLen;
    UInt8*                      appScanDataDisc;
    UInt8                       appScanDataDiscLen;
    UInt8                       advConfig;
#endif //GP_DIVERSITY_BLE_PERIPHERAL || defined GP_DIVERSITY_BLE_LEGACY_ADVERTISING
    smpCfg_t*                   pSmpCfg;
    BleModule_phyCfg_t*         pPhyConfig;
    BleModule_linkLayerCfg_t*   pLinkLayerConfig;
} BleModule_Cfg_t;

/** @struct BleModule_ConnectionParameters_t
 *  @brief Connection parameters requested
*/
typedef struct {
    /** @brief Minimum connection interval in 1.25 ms. Valid range  0x06 - 0x0C80 (7.5 ms - 4 S) and smaller or equal ConnectionMaxInterval.  */
    UInt16                         ConnectionMinInterval;
    /** @brief Maximum connection interval in 1.25 ms. Valid range  0x06 - 0x0C80 (7.5 ms - 4 S) and larger or equal ConnectionMinInterval. */
    UInt16                         ConnectionMaxInterval;
    /** @brief Slave latency, valid range 0-499 */
    UInt16                         ConnectionLatency;
    /** @brief Supervision timeout in 10 ms. Valid range 0x000A - 0x0C80 (100 ms - 32 s) and larger as (1 + connectionLatency) * connectionInterval * 2. For valid range check ConnectionMaxInterval will be used. */
    UInt16                         ConnectionSuperVisionTimeOut;
    /** @brief Minimum duration of a connection event in 0.625 ms. Valid range 0x0000 - 0xFFFF */
    UInt16                         ConnectionMinCeLen;
    /** @brief Maximum duration of a connection event in 0.625 ms. Valid range 0x0000 - 0xFFFF */
    UInt16                         ConnectionMaxCeLen;
} BleModule_ConnectionParameters_t;

/** @struct BleModule_ConnectionParametersIndCnf_t
 *  @brief Connection parameters requested
*/
typedef struct {
    /** @brief Result of a connection update procedure. Can be either local or peer initiated. The statuses are defined in gpHci_Result_t */
    UInt8                          Status;
    /** @brief The selected connection interval in 1.25 ms */
    UInt16                         ConnectionInterval;
    /** @brief The slave latency received, */
    UInt16                         ConnectionLatency;
    /** @brief The actual supervision timeout in 10 ms. */
    UInt16                         ConnectionSupervisionTimeout;
} BleModule_ConnectionParametersIndCnf_t;

/** @struct BleModule_SecurityKeyExchangedInd_t
 *  @brief Information about exchanged security key
*/
typedef struct {
    /** @brief Type of exchanged key */
    BleModule_SecurityKeyType_t                         keyType;
    /** @brief Security level used during key exchange */
    BleModule_SecurityLevel_t                         securityLevel;
    /** @brief Exchanged key length in bytes */
    UInt8                         keyLen;
} BleModule_SecurityKeyExchangedInd_t;

/** @struct BleModule_AuthenticationInd_t
 *  @brief PIN authentication required indication. Provides information about generated PIN
*/
typedef struct {
    /** @brief Information if generated PIN should be displayed or we are expected to just provide PIN with keyboard */
    Bool shouldDisplayPin;
    /** @brief Generated PIN length */
    UInt8 pinLen;
    /** @brief PIN in unsigned integer format    */
    UInt32 pin;
} BleModule_AuthenticationInd_t;

/** @struct BleModule_EncryptionInd_t
 *  @brief Encryption method used indication
*/
typedef struct {
    /** @brief Encryption method used */
    BleModule_EncryptionMethod_t encryptionMethod;
} BleModule_EncryptionInd_t;
/**
 *  @brief Function pointer to the callback for busy functionality
*/
typedef void (*BleModule_BsyCb_t)(Bool busy);
/**
 *  @brief Function pointer to reset callback in the application
*/
typedef void (*BleModule_RstCb_t)(void);
/**
 *  @brief Function pointer to callbacks in the application
*/
typedef void (*BleModule_AppCb_t)(UInt8 linkId, UInt8 status);
/**
 *  @brief Function pointer to encryption event callbacks in the application
*/
typedef void (*BleModule_EncCb_t)(UInt8 linkId, UInt8 status, BleModule_EncryptionInd_t encryptionInd);
/**
 *  @brief Function pointer to pairing event callbacks in the application
*/
typedef void (*BleModule_PairCb_t)(UInt8 linkId, UInt8 status, UInt8 authAndBondFlags);
/**
 *  @brief Function pointer to security key exchange callbacks in the application
*/
typedef void (*BleModule_secKeyCb_t)(UInt8 linkId, BleModule_SecurityKeyExchangedInd_t keyExchangedInfo);
/**
 *  @brief Function pointer for scanning started callback in the application
*/
typedef void (*BleModule_ScanCb_t)(UInt8 status);
/**
 *  @brief Function pointer to authentication request even callback in application
*/
typedef void (*BleModule_AuthReqCb_t)(UInt8 linkId, BleModule_AuthenticationInd_t authInd);
/**
 *  @brief Function pointer to callbacks in the application
*/
typedef void (*BleModule_UpdConnCb_t)(UInt8 linkId, UInt8 status, BleModule_ConnectionParametersIndCnf_t connParams);
/**
 *  @brief Function pointer for address resolving configured indication
*/
typedef void (*BleModule_AddressResCb_t)(UInt8 status);

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//Requests
void BleModule_Init(void);

#if !(defined(GP_COMP_UNIT_TEST))
/** @brief Set callback for common functionality
*
*   @param callback                  Pointer to callback function
*/
void BleModule_RegisterCmnCb(BleModule_BsyCb_t callback);
#endif //!(defined(GP_COMP_UNIT_TEST))

/** @brief Function to test a busy status of the BleModule
*
*   @param busyTask                  Test busy task
*   @return result                   return true if requested busy task is active
*/
Bool BleModule_GetBusy(BleModule_BusyTask_t busyTask);

/** @brief API to set specific busy task
*
*   @param busyTask                  Set busy task
*/
void BleModule_SetBusy(BleModule_BusyTask_t busyTask);

/** @brief API to clear a specific busy task
*
*   @param busyTask                  busy task to clear
*/
void BleModule_ClearBusy(BleModule_BusyTask_t busyTask);

//Indications
/** @brief Reports the transition of the internal state machine into busy state or back to idle mode.
*
*   @param busy                      If set to TRUE, indicates that the STM is busy.
*/
void BleModule_cbSetBusyMode(Bool busy);

/** @brief This function is called  to provide PIN or OOB data during pairing
*
*   @param connId                      Id of connection on which authentication data was requested
*   @param authDataLen                 Length of PIN or OOB data
*   @param pAuthData                   Pointer to PIN or OOB dat
*/
void BleModule_SendAuthenticationResponse(UInt8 connId, UInt8 authDataLen, UInt8 *pAuthData);

/** @brief API to set the preferred values for the transmitter PHY and receiver PHY for all subsequent connections
 * selectedPhy parameter provides user friendly interface to select which PHY should be configured.
 * In implementation it will be translated to HCI ALL_PHYS parameter which use inverted logic. Check implementation
 * and BLE Specs for more details
 *
 * txPhysMask and rxPhysMask allows to select multiple options (masking) by using logical OR
*
*   @param selectedPhy                  Selected PHY (Tx or Rx) to be configured
*   @param txPhysMask                   Preferred transmitter PHYs settings mask
*   @param rxPhysMask                   Preferred receiver PHYs settings mask
*/

void BleModule_SetPreferredPhy(BleModule_SelectedPhy_t selectedPhy, BleModule_PhyTypeMask_t txPhysMask, BleModule_PhyTypeMask_t rxPhysMask);

/** @brief API used to encode UInt32 passkey to fixed sized buffer containing PIN in SMP module expected format
*
*   @param passkeyNumber                Generated passkey in unsigned integer format
*   @param pPinBuffer                   Buffer to fill with PIN in SMP expected format
*/

void BleModule_GenerateSmpPinFromNumber(const UInt32 passkeyNumber, UInt8 pPinBuffer[SMP_PIN_LEN]);

/*! @brief API used restore and set local IRK value from NVM. If LocalIrk is not found in NVM new one will be generated, set and backuped */
void BleModule_RestoreAndSetLocalIrk(void);


/** @brief API used to set resolvable private address generation timeout.
*
*   @param enable               Timeout in seconds after which new RPA should be generated
*/
void BleModule_SetResolvableAddressGenerationTimeout(UInt16 timeout_s);

/** @brief API used to enable/disable RPA address resolution in Link Layer
*
*   @param enable                TRUE for enabling address resolution, FALSE for disabling
*/
void BleModule_SetAddressResolving(Bool enable);

/** @brief API used to check if RPA generation is enabled
*   @return                      TRUE for enabled, FALSE for disabled
*/
Bool BleModule_GetAddressResolvingStatus(void);

/** @brief API used to stop restore privacy to mode device for peer stored in AppDb
*   @param dbHdl   Handel under peer information is stored.
*                  APP_DB_HDL_NONE is not supported. Should be checked before call
*/
void BleModule_RestorePrivacyModeToDevice(appDbHdl_t dbHdl);

#ifdef __cplusplus
}
#endif //__cplusplus

#ifdef GP_BLEMODULE_CENTRAL
#include "BleModule_Central.h"
#endif /* GP_BLEMODULE_CENTRAL */

#ifdef GP_BLEMODULE_PERIPHERAL
#include "BleModule_Peripheral.h"
#endif /* GP_BLEMODULE_PERIPHERAL */

#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_SERVER
#include "BleModule_Att_Server.h"
#endif /* CORDIO_APPFRAMEWORK_DIVERSITY_ATT_SERVER */

#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT
#include "BleModule_Att_Client.h"
#endif /* CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT */

#endif //_BLEMODULE_H_
