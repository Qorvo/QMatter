/*
 * Copyright (c) 2015-2016, GreenPeak Technologies
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
 *
 */

/**
 * @file gpHal_Ble_patch.c
 *
 * @brief This file gives an implementation of some BLE functionality on Kx chips.
 * Only required patch functions are captured here.
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

//#define GP_LOCAL_LOG

#include "gpHal.h"
#include "gpHal_DEFS.h"
#include "gpHal_Ble_DEFS.h"
#include "gpHal_kx_Phy.h"

/*****************************************************************************
 *                   Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/******************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/******************************************************************************
 *                    External Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/
void gpHal_BleSetClockAccuracy(UInt16 caPpm)
{
    if(gpHal_GetRtSystemVersion(gpHal_RtSubSystem_BleMgr) >= GP_HAL_BLE_FIRST_RT_VERSION_SUPPORTING_WORST_SCA_WIDENING)
    {
        GP_WB_WRITE_BLE_MGR_OWN_CLOCK_ACCURACY_PPM(caPpm);
    }
}

gpHal_Result_t gpHal_BleSetTxPower(Int8 requested_txPower_dBm)
{
    // This function configures the BLE default Tx Power - applicable for legacy advertising and connections
    // it also configures the external RF Path (e.g. including an external PA - ouside the chip/package) applicable for all BLE activities (incl ext adv)
    // All BLE Tx Power settings (incl those for for Ext Advertising sets) will be
    // restricted to the Tx Power range corresponding to the RF Path selected by the default Tx Power

    //gpHal_BleTxPower is a static variable in the context of gpHal_ble ROM code. So we need to manipulate the variable
    //by referencing with its address directly.
    gpHal_TxPower_t* gpHal_BleTxPower_ptr = (gpHal_TxPower_t*)0x2003a216;
    gpHal_Address_t optsBase = GP_HAL_PBM_ENTRY2ADDR_OPT_BASE(GP_WB_READ_BLE_MGR_EMPTY_PBM_NR());

    // set dBm Tx power at output of antenna path
    gpHal_TxPower_t ChipTxPower = gpHal_BleCalculateTxPowerAtChipPort(requested_txPower_dBm);

    *gpHal_BleTxPower_ptr = ChipTxPower;

    // PA and ANTSELINT were determined from the Tx power required at the antenna, PBM Tx power is what the chip needs to do
    GP_WB_WRITE_PBM_BLE_FORMAT_T_PA_POWER_SETTINGS(optsBase, gpHalPhy_GetTxPowerSetting(ChipTxPower, GPHAL_TX_OPTIONS_FOR_TRANSMIT));
    GP_WB_WRITE_PBM_BLE_FORMAT_T_PA_POWER_TABLE_INDEX(optsBase, BLE_MGR_PA_POWER_TABLE_INDEX_INVALID);

    return gpHal_ResultSuccess;
}

gpHal_TxPower_t gpHal_BleCalculateTxPowerAtChipPort(Int8 requested_txPower_dBm_at_Antenna)
{
    Int8 TxPathCompensation, RxPathCompensation;
    gpHal_TxPower_t chipPortTxPower;

    gpHal_BleGetRfPathCompensation(&TxPathCompensation, &RxPathCompensation);

    chipPortTxPower = requested_txPower_dBm_at_Antenna - TxPathCompensation;


    if(chipPortTxPower < GPHAL_MIN_TRANSMIT_POWER)
    {
        chipPortTxPower = GPHAL_MIN_TRANSMIT_POWER;
    }
    else if(chipPortTxPower > GPHAL_MAX_TRANSMIT_POWER)
    {
        chipPortTxPower = GPHAL_MAX_TRANSMIT_POWER;
    }
    return chipPortTxPower;
}

Bool gpHal_BleGetMultiStandard(void) { return (GP_WB_READ_BLE_MGR_MS_ENABLED() == 1); }
