/*
 * Copyright (c) 2015-2016, GreenPeak Technologies
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
 * $Header: //depot/release/Embedded/Components/Qorvo/BleController/v2.10.2.0/comps/gpBleDataCommon/inc/gpBle_PhyMask.h#1 $
 * $Change: 187624 $
 * $DateTime: 2021/12/20 10:58:50 $
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include "gpHci_Includes.h"
#include "gpHal_Ble.h"


/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
// HCI_PHYMASK_INIT should not be used execept for the defines below.
// It just provides an easy solution to set up some base masks
#define HCI_PHYMASK_INIT(maskVal)                       ((gpHci_PhyMask_t){.mask = maskVal})
#define BLE_PHYMASK_EMPTY                               HCI_PHYMASK_INIT(0x00)
#define BLE_PHYMASK_1MBIT                               HCI_PHYMASK_INIT(GP_HCI_PHY_MASK_1MB)
#define BLE_PHYMASK_2MBIT                               HCI_PHYMASK_INIT(GP_HCI_PHY_MASK_2MB)
#define BLE_PHYMASK_CODED                               HCI_PHYMASK_INIT(GP_HCI_PHY_MASK_CODED)

    #define BLE_SUPPORTED_PHYS_MASK                                 HCI_PHYMASK_INIT((GP_HCI_PHY_MASK_1MB))
    #define BLE_MAX_PHYS                                            (1)

    #define BLE_SUPPORTED_INIT_PHYS_MASK                            HCI_PHYMASK_INIT(GP_HCI_PHY_MASK_1MB)
    #define BLE_MAX_INIT_PHYS                                       (1)


/*****************************************************************************
 *                    Function Prototypes
 *****************************************************************************/
/*
 * @brief Check if gpHci_PhyMask_t is valid. (interal use only) Meaning:
 *      1. The mask is not zero
 *      2. In case of the scanner, 2Mbit phy is not set in the mask
 */
Bool BleMask_IsInitValid(gpHci_PhyMask_t mask);

/*
 * @brief Check if gpHci_PhyMask_t is valid. (interal use only) Meaning:
 *      The mask is not zero
 */
Bool BleMask_IsValid(gpHci_PhyMask_t mask);

/*
 * @brief Create a gpHci_PhyMask_t based on a single provided gpHci_Phy_t
 */
gpHci_PhyMask_t BleMask_Init(gpHci_Phy_t phy);

/*
 * @brief Compare a gpHci_PhyMask_t to a constant gpHci_PhyMask_t
 */
Bool BleMask_Equals(gpHci_PhyMask_t mask1, const gpHci_PhyMask_t mask2);

/*
 * @brief Check if the bit corresponding to a gpHci_Phy_t is set in a gpHci_PhyMask_t
 * Note that the current implementation asserts when called with a phy that is not suppported in BLE_SUPPORTED_PHYS_MASK
 * This might be too restrictive but can easily be relaxed.
 */
Bool BleMask_HasPhy(gpHci_PhyMask_t mask, gpHci_Phy_t phy);

/*
 * @brief Check if all phys set in gpHci_PhyMask_t are also supported as indicated by the BLE_SUPPORTED_PHYS_MASK
 */
Bool BleMask_IsSupported(gpHci_PhyMask_t mask);

/*
 * @brief Check if all phys set in gpHci_PhyMask_t are also supported as primarry phy as indicated by the BLE_SUPPORTED_INIT_PHYS_MASK
 */
Bool BleMask_IsInitSupported(gpHci_PhyMask_t mask);

/*
 * @brief extract from gpHci_PhyMask_t the gpHci_Phy_t corresponding to the loweest bit set.
 * Note that this function updates the gpHci_PhyMask_t so that on the next call the next lowest phy is returned
 */
gpHci_Phy_t BleMask_LowestPhy(gpHci_PhyMask_t* mask);

/*
 * @brief Set the bit corresponding to phy in mask
 */
void BleMask_SetPhy(gpHci_PhyMask_t* mask, gpHci_Phy_t phy);

/*
 * @brief Clear the bit corresponding to phy in mask
 */
void BleMask_ClearPhy(gpHci_PhyMask_t* mask, gpHci_Phy_t phy);

/*
 * @brief Check if a gpHci_PhyMask_t has only zero bits
 */
Bool BleMask_IsZero(gpHci_PhyMask_t mask);

/*
 * @brief Convert a gpHci_PhyMask_t to a gpHal_phyMask_t
 */
gpHal_phyMask_t BleMask_Hci2Hal(gpHci_PhyMask_t mask);

/*
 * @brief Check if a given gpHci_Phy_t is supported in BLE_SUPPORTED_PHYS_MASK
 */
Bool BlePhy_IsSupported(gpHci_Phy_t phy);

/*
 * @brief Check if a given gpHci_Phy_t is supported in BLE_SUPPORTED_INIT_PHYS_MASK
 */
Bool BlePhy_IsInitSupported(gpHci_Phy_t phy);

/*
 * @brief Convert a gpHci_Phy_t to a gpHal_BleRxPhy_t
 */
gpHal_BleRxPhy_t BlePhy_Hci2HalRx(gpHci_Phy_t phy);

/*
 * @brief Convert a gpHci_Phy_t to a gpHal_BleTxPhy_t
 */
gpHal_BleTxPhy_t BlePhy_Hci2HalTx(gpHci_Phy_t phy);

/*
 * @brief Convert a gpHal_BleRxPhy_t to a gpHci_Phy_t
 */
gpHci_Phy_t BlePhy_HalRx2Hci(gpHal_BleRxPhy_t phy);

/*
 * @brief Convert a gpHal_BleTxPhy_t to a gpHci_Phy_t
 */
gpHci_Phy_t BlePhy_HalTx2Hci(gpHal_BleTxPhy_t phy);

/*
 * @brief Convert a gpHal_BleTxPhy_t to a gpHci_PhyWithCoding_t
 */
gpHci_PhyWithCoding_t BlePhy_HalTx2HciWithCoding(gpHal_BleTxPhy_t phy);

/*
 * @brief Convert a gpHal_BleTxPhy_t to a gpHal_BleRxPhy_t
 */
gpHal_BleRxPhy_t BlePhy_HalTx2HalRx(gpHal_BleTxPhy_t phy);

/*
 * @brief Convert a gpHci_Phy_t to an index in array containing only entries set in BLE_SUPPORTED_PHYS_MASK (ordered 1Mbit, 2Mbit and Coded based)
 */
UInt8 BlePhy_ToIdx(gpHci_Phy_t phy);

/*
 * @brief Convert a gpHci_Phy_t to an index in array containing only entries set in BLE_SUPPORTED_INIT_PHYS_MASK (ordered 1Mbit, 2Mbit and Coded based)
 */
UInt8 BlePhy_ToInitIdx(gpHci_Phy_t phy);

/*
 * @brief Get the next supported phy in BLE_SUPPORTED_PHYS_MASK starting from gpHci_Phy_t
 */
gpHci_Phy_t BlePhy_NextPhy(gpHci_Phy_t phy);

/*
 * @brief Check whether the specified phymask contains an unsupported PHY
 */
Bool BlePhy_IsUnsupportedPhyPresent(gpHci_PhyMask_t mask);

/*
 * @brief Get the PHY with highest bitrate that was set in the mask
 */
gpHal_BleTxPhy_t BlePhy_FastestPhy(gpHci_PhyMask_t mask);

/*
 * @brief Check whether the specified phy is a supported/valid one
 */
Bool Ble_IsPhyUpdateFieldValid(gpHci_Phy_t phy);
