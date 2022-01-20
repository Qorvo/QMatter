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
 * $Header: //depot/release/Embedded/Components/Qorvo/BleController/v2.10.2.0/comps/gpBleDataCommon/src/gpBle_PhyMask.c#1 $
 * $Change: 187624 $
 * $DateTime: 2021/12/20 10:58:50 $
 *
 */
#define GP_LOCAL_LOG
#define GP_COMPONENT_ID GP_COMPONENT_ID_BLEDATACOMMON
/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBle_PhyMask.h"
#include "gpBleDataCommon.h"
#include "gpBleConfig.h"

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/*
 * @brief Check if gpHci_PhyMask_t is valid. (interal use only) Meaning:
 *      1. The mask is not zero
 *      2. Only bits set in BLE_SUPPORTED_(INIT)PHYS_MASK are set in the mask
 *      3. In case of the scanner, 2Mbit phy is not set in the mask
 */
static Bool _IsValid(gpHci_PhyMask_t mask, Bool Init)
{
    UInt8 supportedPhys = (Init) ? BLE_SUPPORTED_INIT_PHYS_MASK.mask : BLE_SUPPORTED_PHYS_MASK.mask;
    return (((supportedPhys | mask.mask) == supportedPhys) &&           // mask supported
            (mask.mask != 0x00) &&                                      // mask not zero
            ((Init) ? !BIT_TST(mask.mask, gpHci_Phy_2Mb - 1) : true));  // if Init, then 2Mbit is not allowed // This is a roll-out of BleMask_HasPhy, to avoid the asserts
}

static Bool _IsSupported(gpHci_PhyMask_t mask, gpHci_Phy_t phy)
{
    return BIT_TST(mask.mask, phy - 1);
}

/*****************************************************************************
*                    Function Definitions
*****************************************************************************/

// Will not assert on invalid input
gpHci_PhyMask_t BleMask_Init(gpHci_Phy_t phy)
{
    return ((gpHci_PhyMask_t){.mask=(gpHci_Phy_None==phy ? 0 : BM(phy - 1))});
}

Bool BleMask_IsZero(gpHci_PhyMask_t mask)
{
    return (mask.mask == 0x00);
}

Bool BleMask_IsInitValid(gpHci_PhyMask_t mask)
{
    return _IsValid(mask, true);
}

Bool BleMask_IsValid(gpHci_PhyMask_t mask)
{
    return _IsValid(mask, false);
}

Bool BlePhy_IsSupported(gpHci_Phy_t phy)
{
    return _IsSupported(BLE_SUPPORTED_PHYS_MASK, phy);
}

Bool BlePhy_IsInitSupported(gpHci_Phy_t phy)
{
    return _IsSupported(BLE_SUPPORTED_INIT_PHYS_MASK, phy);
}

Bool BleMask_IsSupported(gpHci_PhyMask_t mask)
{
    return ((BLE_SUPPORTED_PHYS_MASK.mask | mask.mask) == BLE_SUPPORTED_PHYS_MASK.mask);
}

Bool BleMask_IsInitSupported(gpHci_PhyMask_t mask)
{
    return ((BLE_SUPPORTED_INIT_PHYS_MASK.mask | mask.mask) == BLE_SUPPORTED_INIT_PHYS_MASK.mask);
}

// Will assert on invalid input, the second argument has to be valid and constant
Bool BleMask_Equals(gpHci_PhyMask_t mask1, const gpHci_PhyMask_t mask2)
{
    GP_ASSERT_DEV_INT(BleMask_IsValid(mask1));
    // GP_ASSERT_DEV_INT(BleMask_IsValid(mask2));
    return (mask1.mask == mask2.mask);
}

Bool BleMask_HasPhy(gpHci_PhyMask_t mask, gpHci_Phy_t phy)
{
    GP_ASSERT_DEV_INT(BleMask_IsValid(mask));
    GP_ASSERT_DEV_INT(BlePhy_IsSupported(phy));
    return BIT_TST(mask.mask, phy - 1);
}

gpHci_Phy_t BleMask_LowestPhy(gpHci_PhyMask_t* mask)
{
    GP_ASSERT_DEV_INT(BleMask_IsValid(*mask));
    gpHci_Phy_t phy = (gpHci_Phy_t)(((mask->mask & (0xFF - mask->mask + 1)) >> 1) + 1);
    mask->mask -= BM(phy-1);
    return phy;
}

void BleMask_SetPhy(gpHci_PhyMask_t* mask, gpHci_Phy_t phy)
{
    GP_ASSERT_DEV_INT(BlePhy_IsSupported(phy));
    mask->mask |= BM(phy - 1);
    GP_ASSERT_DEV_INT(BleMask_IsValid(*mask));
}

void BleMask_ClearPhy(gpHci_PhyMask_t* mask, gpHci_Phy_t phy)
{
    GP_ASSERT_DEV_INT(BlePhy_IsSupported(phy));
    GP_ASSERT_DEV_INT(BleMask_IsValid(*mask));
    mask->mask &= ~BM(phy - 1);
}

gpHal_phyMask_t BleMask_Hci2Hal(gpHci_PhyMask_t mask)
{
    GP_ASSERT_DEV_INT(BleMask_IsValid(mask));
    return ((gpHal_phyMask_t){.mask=mask.mask});
}

gpHal_BleRxPhy_t BlePhy_Hci2HalRx(gpHci_Phy_t phy)
{
    GP_ASSERT_DEV_INT(BlePhy_IsSupported(phy));
    return (gpHal_BleRxPhy_t)(phy - 1);
}

gpHal_BleTxPhy_t BlePhy_Hci2HalTx(gpHci_Phy_t phy)
{
    gpHal_BleTxPhy_t txPhy = (gpHal_BleTxPhy_t)(phy - 1);


    return txPhy;
}

gpHci_Phy_t BlePhy_HalRx2Hci(gpHal_BleRxPhy_t phy)
{
    gpHci_Phy_t p = phy + (phy < gpHal_BleRxPhyInvalid); // This takes also coding into account
    GP_ASSERT_DEV_INT(BlePhy_IsSupported(p));
    return p;
}

gpHci_Phy_t BlePhy_HalTx2Hci(gpHal_BleTxPhy_t phy)
{
    gpHci_Phy_t p = phy + 1;
    GP_ASSERT_DEV_INT(BlePhy_IsSupported(p));
    return p;
}

gpHci_PhyWithCoding_t BlePhy_HalTx2HciWithCoding(gpHal_BleRxPhy_t phy)
{
    return (gpHci_PhyWithCoding_t)(phy + 1);
}

gpHal_BleRxPhy_t BlePhy_HalTx2HalRx(gpHal_BleTxPhy_t phy)
{
    gpHal_BleRxPhy_t p = phy;


    return p;
}


UInt8 BlePhy_ToIdx(gpHci_Phy_t phy)
{
    GP_ASSERT_DEV_INT(BlePhy_IsSupported(phy));
    UInt8 idx = 0;
    switch(phy){
        case gpHci_Phy_1Mb: break;
        default: GP_ASSERT_DEV_INT(0);
    }
    return idx;
}

UInt8 BlePhy_ToInitIdx(gpHci_Phy_t phy)
{
    GP_ASSERT_DEV_INT(BlePhy_IsInitSupported(phy));
    return (phy - 1 - (phy > gpHci_Phy_2Mb));
}

gpHci_Phy_t BlePhy_NextPhy(gpHci_Phy_t phy)
{
    GP_ASSERT_DEV_INT(BlePhy_IsSupported(phy));
    // This is a a variation on 'LowestPhy'

    UInt8 mask = (BLE_SUPPORTED_PHYS_MASK.mask | 0x08) - (BM(phy) - 1);
    return(gpHci_Phy_t)(((mask & (0xFF - mask + 1)) >> 1) + 1);
}

Bool BlePhy_IsUnsupportedPhyPresent(gpHci_PhyMask_t mask)
{
    gpHci_PhyMask_t supported_phys = GPBLEDATACOMMON_GET_SUPPORTED_PHYS_MASK();

    return ((supported_phys.mask | mask.mask) != supported_phys.mask);
}

Bool Ble_IsPhyUpdateFieldValid(gpHci_Phy_t phy)
{
    Bool retval = false;
    if( phy == gpHci_Phy_None)
    {
        retval = true;
    }
    else if(phy > gpHci_Phy_None && phy < gpHci_Phy_Invalid)
    {
        phy--;
        gpHci_PhyMask_t supported_phys = GPBLEDATACOMMON_GET_SUPPORTED_PHYS_MASK();
        retval = BIT_TST(supported_phys.mask, phy);
    }
    return retval;
}

gpHal_BleTxPhy_t BlePhy_FastestPhy(gpHci_PhyMask_t mask)
{
    gpHci_PhyMask_t supported_phys = GPBLEDATACOMMON_GET_SUPPORTED_PHYS_MASK();
    UInt8 all_phys = mask.mask & supported_phys.mask;


    if(all_phys & GP_HCI_PHY_MASK_1MB)
    {
        return gpHal_BleTxPhy1Mb;
    }


    return gpHal_BleTxPhyInvalid;
}

