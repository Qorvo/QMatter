/*
 * Copyright (c) 2017, Qorvo Inc
 *
 * gpHal_MAC.c
 *   This file contains the implementation of the MAC functions
 *
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
 * $Header: //depot/release/Embedded/Components/Qorvo/HAL_RF/v2.10.2.1/comps/gphal/k8e/src/gpHal_RapDsFix.c#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpHal.h"
#include "gpHal_DEFS.h"
#include "gpHal_kx_Rap.h"
#include "gpHal_kx_gpm.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

//#define GP_LOCAL_LOG

#define GP_COMPONENT_ID GP_COMPONENT_ID_GPHAL

/*****************************************************************************
 *                   Functional Macro Definitions
 *****************************************************************************/

#define GET_PROP_ADDR(prop)  (GP_WB_##prop##_ADDRESS + (GP_WB_##prop##_LSB/8))
#define GET_PROP_LSB(prop)   (GP_WB_##prop##_LSB     - (8*(GET_PROP_ADDR(prop)-GP_WB_##prop##_ADDRESS)))
#define GET_PROP_MASK(prop)  (GP_WB_##prop##_MASK   >> (8*(GET_PROP_ADDR(prop)-GP_WB_##prop##_ADDRESS)) )

// channel conversion map routing
#define CONVERT_BLE_CH(i) GP_WB_READ_U8(GP_WB_BLE_MGR_CH_CONV_BLE_CHANNEL_0_ADDRESS+(i))

#define LOCAL_BACKUP(prop) prop##_backup = GP_WB_READ_##prop()
#define LOCAL_RESTORE(prop) GP_WB_WRITE_##prop( prop##_backup )

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/*
 *  Searches the optimal set of parameters to counter the desense issue AD-1646
 *  **IMPORTANT** make sure that absolutely no BLE events are running! + NO ZigBee activity!
 *
 *  Parameters:
 *  -   channel: ble_channel to optimise for
 *  Returns:
 *  -   best_xo_ldo_refbits: the calibrated value to be used for PMUD_XO_LDO_REFBITS for the given channel
 *  -   best_rx_bpf_ldo_dig_refbits: the calibrated value to be used for RX_RX_BPF_LDO_DIG_REFBITS for the given channel
 *
 */
void dsfix_cal(UInt8 channel, Int8 *best_xo_ldo_refbits, Int8 *best_rx_bpf_ldo_dig_refbits)
{
    if (
        (channel == 6)  ||
        (channel == 13) ||
        (channel == 21) ||
        (channel == 29) ||
        (channel == 39)
        )
    {
        // first do a calibaration
        // can't we use some RAP functions?
        GP_WB_WRITE_RIB_CHANNEL_NR(CONVERT_BLE_CH(channel));
        GP_WB_WRITE_RIB_CHANNEL_IDX( GP_WB_READ_BLE_MGR_BLE_CHANNEL_IDX() );
        GP_WB_WRITE_RIB_START_TIMESTAMP_VALID(false); // immediate
        GP_WB_RIB_CHANNEL_CHANGE_REQUEST();
        HAL_WAIT_US(100);
        while(GP_WB_READ_RIB_TRC_STATE() == GP_WB_ENUM_SIMPLE_TRC_STATE_CAL_RX) {}

        // backup some used settings
        // locals, compiler will decide if they get pushed onto the stack or not, and we still can use clear names!
        {
            UInt8 LOCAL_BACKUP(RADIOITF_MATCH_ANT1_SELECT_OVERRULE);
            UInt8 LOCAL_BACKUP(RADIOITF_MATCH_ANT2_SELECT_OVERRULE);
            UInt8 LOCAL_BACKUP(RADIOITF_MATCH_ANT1_SELECT_OVERRULE_ENA);
            UInt8 LOCAL_BACKUP(RADIOITF_MATCH_ANT2_SELECT_OVERRULE_ENA);
            UInt8 LOCAL_BACKUP(RADIOITF_RX_BPF_BLE_OVERRULE);
            UInt8 LOCAL_BACKUP(RADIOITF_RX_BPF_BLE_OVERRULE_ENA);
            UInt8 LOCAL_BACKUP(RX_EN_LNA_AGC);
            UInt8 LOCAL_BACKUP(PMUD_XO_LDO_RDY_OVERRIDE);
            UInt8 LOCAL_BACKUP(PMUD_CLK_32M_RDY_OVERRIDE);
            UInt8 LOCAL_BACKUP(RX_PREAMBLE_THRESH_BT);
            UInt8 LOCAL_BACKUP(RIB_RX_ON_WHEN_IDLE_CH);

            // backup the active parameters
            Int8 LOCAL_BACKUP(PMUD_XO_LDO_REFBITS);
            Int8 LOCAL_BACKUP(RX_RX_BPF_LDO_DIG_REFBITS);

            UInt8 best_rssi = 255;
            Int8 xo_ldo_refbits;

            // some overrules to enhance detection/resistance against noize
            // decouple the antennas, to make the spurs bigger
            GP_WB_WRITE_RADIOITF_MATCH_ANT1_SELECT_OVERRULE(0);
            GP_WB_WRITE_RADIOITF_MATCH_ANT2_SELECT_OVERRULE(0);
            GP_WB_WRITE_RADIOITF_MATCH_ANT1_SELECT_OVERRULE_ENA(1);
            GP_WB_WRITE_RADIOITF_MATCH_ANT2_SELECT_OVERRULE_ENA(1);
            // switch to small bandpass filter
            GP_WB_WRITE_RADIOITF_RX_BPF_BLE_OVERRULE(0);
            GP_WB_WRITE_RADIOITF_RX_BPF_BLE_OVERRULE_ENA(1);
            // disable the AGC
            GP_WB_WRITE_RX_EN_LNA_AGC(0);
            // overrides to keep the chip from resetting
            GP_WB_WRITE_PMUD_XO_LDO_RDY_OVERRIDE(1);
            GP_WB_WRITE_PMUD_CLK_32M_RDY_OVERRIDE(1);
            // make sure we don't receive frames
            GP_WB_WRITE_RX_PREAMBLE_THRESH_BT(0xFFFF);

            // enable the receiver for BLE
            GP_WB_WRITE_PLME_RX_MODE_BLE(GP_WB_ENUM_BLE_RECEIVER_MODE_BLE);
            GP_WB_WRITE_RIB_RX_ON_WHEN_IDLE_CH(1<<GP_WB_READ_BLE_MGR_BLE_CHANNEL_IDX());

            // loop over all parameters and keep track of the lowest score

            *best_xo_ldo_refbits = 0;
            *best_rx_bpf_ldo_dig_refbits = 0;

            for (xo_ldo_refbits=-16; xo_ldo_refbits<16; xo_ldo_refbits++)
            {
                Int8 rx_bpf_ldo_dig_refbits;

                GP_WB_WRITE_PMUD_XO_LDO_REFBITS(xo_ldo_refbits); // highest bit may not be used
                //
                for (rx_bpf_ldo_dig_refbits=-8; rx_bpf_ldo_dig_refbits<8; rx_bpf_ldo_dig_refbits++)
                {
                    UInt32 i;
                    UInt8 lowest_rssi = 255;

                    GP_WB_WRITE_RX_RX_BPF_LDO_DIG_REFBITS(rx_bpf_ldo_dig_refbits);
                    // take into account settling
                    HAL_WAIT_US(64);
                    // take the samples
                    // UInt32 t_now = GP_WB_READ_ES_AUTO_SAMPLED_SYMBOL_COUNTER();

                    for (i=0; i<1000; i++)
                    {
                        UInt8 rssi = GP_WB_READ_RX_RSSI_BBPRX();
                        if (rssi < lowest_rssi)
                            lowest_rssi = rssi;
                    }
                    // LOG(0, "sample_dur=%lu", GP_WB_READ_ES_AUTO_SAMPLED_SYMBOL_COUNTER()-t_now);
                    // LOG(0, "ldo=%d, agc=%d, bgt=%d, score=%d",xo_ldo_refbits, xo_agc_level, bg_tune, lowest_rssi);
                    // update the best parameter
                    if (lowest_rssi < best_rssi)
                    {
                        best_rssi = lowest_rssi;
                        *best_xo_ldo_refbits = xo_ldo_refbits;
                        *best_rx_bpf_ldo_dig_refbits = rx_bpf_ldo_dig_refbits;
                    }
                }
            }

            // disable forces
            // restore backup
            LOCAL_RESTORE(RIB_RX_ON_WHEN_IDLE_CH);

            LOCAL_RESTORE(RADIOITF_MATCH_ANT1_SELECT_OVERRULE);
            LOCAL_RESTORE(RADIOITF_MATCH_ANT2_SELECT_OVERRULE);
            LOCAL_RESTORE(RADIOITF_MATCH_ANT1_SELECT_OVERRULE_ENA);
            LOCAL_RESTORE(RADIOITF_MATCH_ANT2_SELECT_OVERRULE_ENA);
            LOCAL_RESTORE(RADIOITF_RX_BPF_BLE_OVERRULE);
            LOCAL_RESTORE(RADIOITF_RX_BPF_BLE_OVERRULE_ENA);
            LOCAL_RESTORE(RX_EN_LNA_AGC);

            // restore previous active parameters
            LOCAL_RESTORE(PMUD_XO_LDO_REFBITS);
            LOCAL_RESTORE(RX_RX_BPF_LDO_DIG_REFBITS);
            HAL_WAIT_US(500);
            LOCAL_RESTORE(PMUD_XO_LDO_RDY_OVERRIDE);
            LOCAL_RESTORE(PMUD_CLK_32M_RDY_OVERRIDE);
            LOCAL_RESTORE(RX_PREAMBLE_THRESH_BT);
        }
    }
    else
    {
        // no fix needed for this channel, just use the nominal settings
        *best_xo_ldo_refbits = 0;
        *best_rx_bpf_ldo_dig_refbits = 0;
    }
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void rap_dsfix_cal_desense_ch(UInt8 *channel)
{
    // check for the most optimal parameters
    Int8 best_xo_ldo_refbits;
    Int8 best_rx_bpf_ldo_dig_refbits;
    dsfix_cal(*channel, &best_xo_ldo_refbits, &best_rx_bpf_ldo_dig_refbits);

    // apply the found parameters
    {
        UInt8 LOCAL_BACKUP(PMUD_XO_LDO_RDY_OVERRIDE);
        UInt8 LOCAL_BACKUP(PMUD_CLK_32M_RDY_OVERRIDE);
        GP_WB_WRITE_PMUD_XO_LDO_RDY_OVERRIDE(1);
        GP_WB_WRITE_PMUD_CLK_32M_RDY_OVERRIDE(1);

        GP_WB_WRITE_PMUD_XO_LDO_REFBITS(best_xo_ldo_refbits);
        GP_WB_WRITE_RX_RX_BPF_LDO_DIG_REFBITS(best_rx_bpf_ldo_dig_refbits);

        HAL_WAIT_US(100); // wait until parameters are settled
        LOCAL_RESTORE(PMUD_XO_LDO_RDY_OVERRIDE);
        LOCAL_RESTORE(PMUD_CLK_32M_RDY_OVERRIDE);
    }
}

void rap_dsfix_setup(UInt32 dsfix_mem_ptr)
{
    UInt8 i;

    // LOG(0, "[DSFIX] Configuring dsfix_ch's");
    const UInt8 dsfix_channels[] = {6,13,21,29,39};
    UInt32 dsfix_param_addr = dsfix_mem_ptr + sizeof(dsfix_channels)*BLE_MGR_DSFIX_CH_LIST_SPACING;
    UInt32 dsfix_pre_addr = dsfix_mem_ptr + sizeof(dsfix_channels)*BLE_MGR_DSFIX_CH_LIST_SPACING + 4*BLE_MGR_DSFIX_PARAM_LIST_SPACING;

    for (i=0; i<sizeof(dsfix_channels); i++)
    {
        UInt32 dsfix_ch_addr = dsfix_mem_ptr+i*BLE_MGR_DSFIX_CH_LIST_SPACING;
        GP_WB_WRITE_DSFIX_CH_CHANNEL(dsfix_ch_addr, dsfix_channels[i]);
        GP_WB_WRITE_DSFIX_CH_LOWEST_RSSI(dsfix_ch_addr,     255);
        GP_WB_WRITE_DSFIX_CH_RSSI_AGEING_CNT(dsfix_ch_addr, 0);
        GP_WB_WRITE_DSFIX_CH_BEST_POLL_CNT(dsfix_ch_addr,   0);
        GP_WB_WRITE_DSFIX_CH_PARAM_SWEEP_CNT(dsfix_ch_addr, 0);
        GP_WB_WRITE_DSFIX_CH_CUR_PARAM0(dsfix_ch_addr,  -16); // xo_ldo_refbits
        GP_WB_WRITE_DSFIX_CH_CUR_PARAM1(dsfix_ch_addr,  -8); // rx_bpf_ldo_dig_refbits
        GP_WB_WRITE_DSFIX_CH_CUR_PARAM2(dsfix_ch_addr,  0);
        GP_WB_WRITE_DSFIX_CH_CUR_PARAM3(dsfix_ch_addr,  0);
        GP_WB_WRITE_DSFIX_CH_BEST_PARAM0(dsfix_ch_addr, -16); // xo_ldo_refbits
        GP_WB_WRITE_DSFIX_CH_BEST_PARAM1(dsfix_ch_addr, -8); // rx_bpf_ldo_dig_refbits
        GP_WB_WRITE_DSFIX_CH_BEST_PARAM2(dsfix_ch_addr, 0);
        GP_WB_WRITE_DSFIX_CH_BEST_PARAM3(dsfix_ch_addr, 0);
    }

    GP_WB_WRITE_BLE_MGR_DSFIX_CHANNEL_LIST_PTR(TO_GPM_ADDR(dsfix_mem_ptr));
    GP_WB_WRITE_BLE_MGR_DSFIX_CHANNEL_LIST_SIZE(sizeof(dsfix_channels));

    // LOG(0, "[DSFIX] Configuring dsfix_params's");
    // the params we want to sweep

    GP_WB_WRITE_BLE_MGR_DSFIX_PARAM_LIST_PTR(TO_GPM_ADDR(dsfix_param_addr));
    GP_WB_WRITE_BLE_MGR_DSFIX_PARAM_LIST_SIZE(2);

    GP_WB_WRITE_DSFIX_PARAM_ADDRESS(dsfix_param_addr,   (UInt16) GET_PROP_ADDR(PMUD_XO_LDO_REFBITS));
    GP_WB_WRITE_DSFIX_PARAM_LSB(dsfix_param_addr,       GET_PROP_LSB(PMUD_XO_LDO_REFBITS));
    GP_WB_WRITE_DSFIX_PARAM_MASK(dsfix_param_addr,      GET_PROP_MASK(PMUD_XO_LDO_REFBITS));
    GP_WB_WRITE_DSFIX_PARAM_MIN(dsfix_param_addr,       -16);
    GP_WB_WRITE_DSFIX_PARAM_MAX(dsfix_param_addr,       15);
    dsfix_param_addr += BLE_MGR_DSFIX_PARAM_LIST_SPACING;
    GP_WB_WRITE_DSFIX_PARAM_ADDRESS(dsfix_param_addr,   (UInt16) GET_PROP_ADDR(RX_RX_BPF_LDO_DIG_REFBITS));
    GP_WB_WRITE_DSFIX_PARAM_LSB(dsfix_param_addr,       GET_PROP_LSB(RX_RX_BPF_LDO_DIG_REFBITS));
    GP_WB_WRITE_DSFIX_PARAM_MASK(dsfix_param_addr,      GET_PROP_MASK(RX_RX_BPF_LDO_DIG_REFBITS)); // special case, we may not use the upper bit
    GP_WB_WRITE_DSFIX_PARAM_MIN(dsfix_param_addr,       -8);
    GP_WB_WRITE_DSFIX_PARAM_MAX(dsfix_param_addr,       7);

    // nominal parameter values
    GP_WB_WRITE_BLE_MGR_DSFIX_NOM_PARAM0(0);
    GP_WB_WRITE_BLE_MGR_DSFIX_NOM_PARAM1(0);

    // LOG(0, "[DSFIX] Configuring dsfix_pre's");
    // measurement parameters, forces needed to be on while the dsfix is enabled, could otherswise reset the chip
    GP_WB_WRITE_RADIOITF_MATCH_ANT1_SELECT_OVERRULE(0);
    GP_WB_WRITE_RADIOITF_MATCH_ANT2_SELECT_OVERRULE(0);
    GP_WB_WRITE_RADIOITF_RX_BPF_BLE_OVERRULE(0);
    GP_WB_WRITE_PMUD_XO_LDO_RDY_OVERRIDE(1);
    GP_WB_WRITE_PMUD_CLK_32M_RDY_OVERRIDE(1);


    GP_WB_WRITE_BLE_MGR_DSFIX_PRE_LIST_PTR(TO_GPM_ADDR(dsfix_pre_addr));
    // decouple the antennas, to make the spurs bigger
    GP_WB_WRITE_U16(dsfix_pre_addr, (UInt16) GET_PROP_ADDR(RADIOITF_MATCH_ANT1_SELECT_OVERRULE_ENA)); dsfix_pre_addr+=2;
    GP_WB_WRITE_U8(dsfix_pre_addr,  0); dsfix_pre_addr+=1; // placeholder for backup
    GP_WB_WRITE_U8(dsfix_pre_addr,  GET_PROP_MASK(RADIOITF_MATCH_ANT1_SELECT_OVERRULE_ENA) | GET_PROP_MASK(RADIOITF_MATCH_ANT2_SELECT_OVERRULE_ENA)); dsfix_pre_addr+=1;
    // switch to small bandpass filter
    GP_WB_WRITE_U16(dsfix_pre_addr, (UInt16) GET_PROP_ADDR(RADIOITF_RX_BPF_BLE_OVERRULE_ENA)); dsfix_pre_addr+=2;
    GP_WB_WRITE_U8(dsfix_pre_addr,  0); dsfix_pre_addr+=1; // placeholder for backup
    GP_WB_WRITE_U8(dsfix_pre_addr,  GET_PROP_MASK(RADIOITF_RX_BPF_BLE_OVERRULE)); dsfix_pre_addr+=1;
    // disable the AGC
    GP_WB_WRITE_U16(dsfix_pre_addr, (UInt16) GET_PROP_ADDR(RX_EN_LNA_AGC)); dsfix_pre_addr+=2;
    GP_WB_WRITE_U8(dsfix_pre_addr,  0); dsfix_pre_addr+=1; // placeholder for backup
    GP_WB_WRITE_U8(dsfix_pre_addr,  GET_PROP_MASK(RX_EN_LNA_AGC)); dsfix_pre_addr+=1;
    // empty entry
    GP_WB_WRITE_U16(dsfix_pre_addr, 0); dsfix_pre_addr+=2;
    GP_WB_WRITE_U8(dsfix_pre_addr,  0); dsfix_pre_addr+=1; // placeholder for backup
    GP_WB_WRITE_U8(dsfix_pre_addr,  0); dsfix_pre_addr+=1;

    // general stuff
    GP_WB_WRITE_BLE_MGR_DSFIX_RX_WD_OFFSET(60);
    GP_WB_WRITE_BLE_MGR_DSFIX_RSSI_SAMPLE_HOLDOFF(50);

    GP_WB_WRITE_BLE_MGR_DSFIX_RSSI_AGEING_THRESHOLD(15); //
    GP_WB_WRITE_BLE_MGR_DSFIX_BEST_POLL_THRESHOLD(2); // 2 new param measurements before between each best param check
    // LOG(0, "[DSFIX] Done");
}
