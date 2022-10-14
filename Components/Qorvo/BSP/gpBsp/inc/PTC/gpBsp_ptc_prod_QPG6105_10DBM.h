
/*
#   Copyright (c) 2011-2016, GreenPeak Technologies
#   Copyright (c) 2017-2021, Qorvo Inc
 *
 *
 *   QPG6105 PTC 10DBM production
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
 *
 */


#ifndef _GPBSP_PTC_PROD_QPG6105_10DBM_H_
#define _GPBSP_PTC_PROD_QPG6105_10DBM_H_

/*****************************************************************************
 *                    BSP configuration interface
 *****************************************************************************/

#define GP_BSP_INTERNAL_UC
// Is a 32kHz crystal mounted?
#define GP_BSP_32KHZ_CRYSTAL_AVAILABLE() 1
// Is the watchdog timer used?
#define GP_BSP_USE_WDT_TIMER() 0
// Has the board an differential antenna?
#define GP_BSP_HAS_DIFFERENTIAL_SINGLE_ANTENNA() 0
// Settling time for tx monitor in symbols
#define GP_BSP_TXMONITOR_SETTLING_TIME_IN_SYMBOLS() 0

/*****************************************************************************
 *                    GPIO - LED
 *****************************************************************************/


/* Green LED: active low, no fading, using GPIO peripheral */
#define GP_BSP_LED_GRN_PIN 1
#define GP_BSP_LED_GRN_LOGIC_LEVEL 0
// HAL helpers
#define GRN 1 // GPIO1 - LED Active when low
#define HAL_LED_SET_GRN() GP_WB_WRITE_GPIO_GPIO1_DIRECTION(1)
#define HAL_LED_CLR_GRN() GP_WB_WRITE_GPIO_GPIO1_DIRECTION(0)
#define HAL_LED_TST_GRN() GP_WB_READ_GPIO_GPIO1_DIRECTION()
#define HAL_LED_TGL_GRN() do{ if (HAL_LED_TST_GRN()) { HAL_LED_CLR_GRN(); } else { HAL_LED_SET_GRN(); }; }while(false)

/* Red LED: active low, no fading, using GPIO peripheral */
#define GP_BSP_LED_RED_PIN 2
#define GP_BSP_LED_RED_LOGIC_LEVEL 0
// HAL helpers
#define RED 2 // GPIO2 - LED Active when low
#define HAL_LED_SET_RED() GP_WB_WRITE_GPIO_GPIO2_DIRECTION(1)
#define HAL_LED_CLR_RED() GP_WB_WRITE_GPIO_GPIO2_DIRECTION(0)
#define HAL_LED_TST_RED() GP_WB_READ_GPIO_GPIO2_DIRECTION()
#define HAL_LED_TGL_RED() do{ if (HAL_LED_TST_RED()) { HAL_LED_CLR_RED(); } else { HAL_LED_SET_RED(); }; }while(false)

#define HAL_LED_INIT_LEDS()                         do{ \
    /*Initialize output value - switching input/output will toggle LED*/ \
    GP_WB_WRITE_GPIO_GPIO1_OUTPUT_VALUE(0); \
    GP_WB_WRITE_IOB_GPIO_1_CFG(GP_WB_ENUM_GPIO_MODE_PULLUP); \
    HAL_LED_CLR_GRN(); \
    /*Initialize output value - switching input/output will toggle LED*/ \
    GP_WB_WRITE_GPIO_GPIO2_OUTPUT_VALUE(0); \
    GP_WB_WRITE_IOB_GPIO_2_CFG(GP_WB_ENUM_GPIO_MODE_PULLUP); \
    HAL_LED_CLR_RED(); \
    /*Drive strength*/ \
    GP_WB_WRITE_IOB_GPIO_0_3_DRIVE_STRENGTH(GP_WB_ENUM_DRIVE_STRENGTH_DRIVE_18MA); \
}while(0)

#define GP_BSP_LED_GPIO_MAP                         { 0xff, 0xff, 0xff, 0xff }
#define GP_BSP_LED_ALTERNATE_MAP                    { 0, 0, 0, 0 }

/*****************************************************************************
 *                    GPIO - BTN - No buttons used
 *****************************************************************************/

#define HAL_BTN_INIT_BTNS()                         do{ \
}while(0)

/*****************************************************************************
 *                    GPIO - ALTERNATIVE - not used
 *****************************************************************************/


/*****************************************************************************
 *                    UART
 *****************************************************************************/

// Pin 27 - GPIO 8 - UART0_RX
#define GP_BSP_UART0_RX_GPIO                        8
#define GP_BSP_UART0_RX_ALTERNATE                   GP_WB_ENUM_GPIO_8_ALTERNATES_UART_0_RX
#define GP_BSP_UART0_RX_INIT()                      do{ GP_WB_WRITE_IOB_GPIO_8_ALTERNATE(GP_WB_ENUM_GPIO_8_ALTERNATES_UART_0_RX); GP_WB_WRITE_IOB_GPIO_8_ALTERNATE_ENABLE(1); }while(0)
#define GP_BSP_UART0_RX_DEINIT()                    GP_WB_WRITE_IOB_GPIO_8_ALTERNATE_ENABLE(0);
#define GP_BSP_UART0_RX_DEFINED()                   (1)
#define GP_BSP_UART0_RX_GPIO_CFG()                  GP_WB_WRITE_IOB_GPIO_8_CFG(GP_WB_ENUM_GPIO_MODE_PULLUP)
#define GP_BSP_UART0_RX_ENABLE(en)                  do{ if (en) { GP_WB_WRITE_IOB_GPIO_8_ALTERNATE(GP_WB_ENUM_GPIO_8_ALTERNATES_UART_0_RX); } GP_WB_WRITE_IOB_GPIO_8_ALTERNATE_ENABLE((en)); }while(0)
#define GP_BSP_UART0_RX_ENABLED()                   GP_WB_READ_IOB_GPIO_8_ALTERNATE_ENABLE()

// Pin 28 - GPIO 9 - UART0_TX
#define GP_BSP_UART0_TX_GPIO                        9
#define GP_BSP_UART0_TX_ALTERNATE                   GP_WB_ENUM_GPIO_9_ALTERNATES_UART_0_TX
#define GP_BSP_UART0_TX_INIT()                      do{ GP_WB_WRITE_IOB_GPIO_9_ALTERNATE(GP_WB_ENUM_GPIO_9_ALTERNATES_UART_0_TX); GP_WB_WRITE_IOB_GPIO_9_ALTERNATE_ENABLE(1); }while(0)
#define GP_BSP_UART0_TX_DEINIT()                    GP_WB_WRITE_IOB_GPIO_9_ALTERNATE_ENABLE(0);
#define GP_BSP_UART0_TX_DEFINED()                   (1)
#define GP_BSP_UART0_TX_GPIO_CFG()                  GP_WB_WRITE_IOB_GPIO_9_CFG(GP_WB_ENUM_GPIO_MODE_FLOAT)
#define GP_BSP_UART0_TX_ENABLE(en)                  do{ if (en) { GP_WB_WRITE_IOB_GPIO_9_ALTERNATE(GP_WB_ENUM_GPIO_9_ALTERNATES_UART_0_TX); } GP_WB_WRITE_IOB_GPIO_9_ALTERNATE_ENABLE((en)); }while(0)
#define GP_BSP_UART0_TX_ENABLED()                   GP_WB_READ_IOB_GPIO_9_ALTERNATE_ENABLE()

#define GP_BSP_UART1_RX_DEFINED()                   (0)
#define GP_BSP_UART1_RX_GPIO_CFG()                  do { } while(0)
#define GP_BSP_UART1_RX_ENABLE(en)                  do { } while(0)
#define GP_BSP_UART1_RX_ENABLED()                   (0)

#define GP_BSP_UART1_TX_DEFINED()                   (0)
#define GP_BSP_UART1_TX_GPIO_CFG()                  do { } while(0)
#define GP_BSP_UART1_TX_ENABLE(en)                  do { } while(0)
#define GP_BSP_UART1_TX_ENABLED()                   (0)

#define GP_BSP_UART_TX_GPIO_MAP                     { 9, 0xff }
#define GP_BSP_UART_TX_ALTERNATE_MAP                { GP_WB_ENUM_GPIO_9_ALTERNATES_UART_0_TX, 0 }
#define GP_BSP_UART_RX_GPIO_MAP                     { 8, 0xff }
#define GP_BSP_UART_RX_ALTERNATE_MAP                { GP_WB_ENUM_GPIO_8_ALTERNATES_UART_0_RX, 0 }

#define GP_BSP_UART_COM1                            0

#ifndef GP_BSP_UART_COM_BAUDRATE
#define GP_BSP_UART_COM_BAUDRATE                    57600
#endif

#ifndef GP_BSP_UART_SCOM_BAUDRATE
#define GP_BSP_UART_SCOM_BAUDRATE                   57600
#endif

/*****************************************************************************
 *                    GPIO - Unused
 *****************************************************************************/

/* Pull down unused pins */
#define GP_BSP_UNUSED_INIT()                        do{ \
    GP_WB_WRITE_IOB_GPIO_0_CFG(GP_WB_ENUM_GPIO_MODE_PULLDOWN); \
    GP_WB_WRITE_IOB_GPIO_3_CFG(GP_WB_ENUM_GPIO_MODE_PULLDOWN); \
    GP_WB_WRITE_IOB_GPIO_4_CFG(GP_WB_ENUM_GPIO_MODE_PULLDOWN); \
    GP_WB_WRITE_IOB_GPIO_6_CFG(GP_WB_ENUM_GPIO_MODE_PULLDOWN); \
    GP_WB_WRITE_IOB_GPIO_7_CFG(GP_WB_ENUM_GPIO_MODE_PULLDOWN); \
    GP_WB_WRITE_IOB_GPIO_10_CFG(GP_WB_ENUM_GPIO_MODE_PULLDOWN); \
    GP_WB_WRITE_IOB_GPIO_11_CFG(GP_WB_ENUM_GPIO_MODE_PULLDOWN); \
    GP_WB_WRITE_IOB_GPIO_17_CFG(GP_WB_ENUM_GPIO_MODE_PULLDOWN); \
    GP_WB_WRITE_IOB_GPIO_18_CFG(GP_WB_ENUM_GPIO_MODE_PULLDOWN); \
    GP_WB_WRITE_IOB_GPIO_21_CFG(GP_WB_ENUM_GPIO_MODE_PULLDOWN); \
    GP_WB_WRITE_IOB_GPIO_22_CFG(GP_WB_ENUM_GPIO_MODE_PULLDOWN); \
}while(0)

/*****************************************************************************
 *                    IO Pending
 *****************************************************************************/

#define HAL_BSP_IO_ACTIVITY_PENDING()               (false)

/*****************************************************************************
 *                    Generic
 *****************************************************************************/


/* Enable one of the defines below to enable the correct antenna tuning parameters and TX power table*/
/* #define GP_BSP_ANTENNATUNECONFIG_10DBM_DIFFERENTIAL */
 #define GP_BSP_ANTENNATUNECONFIG_10DBM_SINGLE_ENDED
/* #define GP_BSP_ANTENNATUNECONFIG_7DBM_DIFFERENTIAL */
/* #define GP_BSP_ANTENNATUNECONFIG_7DBM_SINGLE_ENDED */
/* #define GP_BSP_ANTENNATUNECONFIG_FEM_SINGLE_ENDED */

#include "gpBsp_k8e_antenna_tune_parameters.h"

#define GP_BSP_GENERIC_INIT()                       do{ \
    /*Disable bus keeper/PU/PD on UART0_RX*/ \
    GP_WB_WRITE_IOB_GPIO_8_CFG(GP_WB_ENUM_GPIO_MODE_FLOAT); \
    /*Disable bus keeper/PU/PD on UART0_TX*/ \
    GP_WB_WRITE_IOB_GPIO_9_CFG(GP_WB_ENUM_GPIO_MODE_FLOAT); \
    GP_BSP_ANTENNATUNECONFIG_INIT(); \
}while(0)

#define GP_BSP_SLEEPMODERETENTIONLIST \
    /* *** plme *** */\
    /* *** trx *** */\
    0x0080, /* ext_mode_ctrl_for_trx_off|ext_mode_ctrl_for_rx_on_att_ctrl_low|ext_mode_ctrl_for_rx_on_att_ctrl_high|ext_channel_ctrl_for_trx_off|mask_channel_ctrl_for_trx_off */ \
    0x0081, /* external_mode_control_1 */ \
    0x0082, /* external_mode_control_2 */ \
    /* *** rx *** */\
    0x0222, /* bt_sample_rssi_delay|bt_sample_rssi_on_early_sfd */ \
    0x0223, /* unnamedpb_rx_0x0022_1 */ \
    0x028A, /* bt_delay_early_birdy|en_zb_packet_found */ \
    0x02BC, /* fail_timeout_value|fail_timeout_en|cw_det_depends_on_fine */ \
    0x02BD, /* dbg_22_1 */ \
    /* *** tx *** */\
    0x0307, /* tx_pa_biastrim_mult */ \
    0x0309, /* pa_slope_zb|pa_slope_ble|fll_start_of_ramp|enable_external_pa_biasing */ \
    0x030C, /* tx_ldo_refbits */ \
    0x030E, /* tx_dca_n|tx_dca_p|tx_ldo_bleed_off|tx_ldo_pup|tx_pa_biastrim|tx_peak_lvl|tx_ldo_res_bypass */ \
    0x030F, /* unnamedpb_tx_0x000e_1 */ \
    /* *** rib *** */\
    /* *** qta *** */\
    /* *** prg *** */\
    /* *** pmud *** */\
    /* *** iob *** */\
    0x0710, /* gpio_0_3_drive_strength|gpio_0_3_schmitt_trigger|gpio_4_7_drive_strength|gpio_4_7_schmitt_trigger|gpio_8_11_drive_strength|gpio_8_11_schmitt_trigger|gpio_12_15_drive_strength|gpio_12_15_schmitt_trigger|gpio_16_19_drive_strength|gpio_16_19_schmitt_trigger|gpio_20_23_schmitt_trigger|gpio_20_23_drive_strength */ \
    0x0711, /* gpio_pin_config_b_1 */ \
    0x0712, /* gpio_pin_config_b_2 */ \
    /* *** standby *** */\
    /* *** es *** */\
    /* *** msi *** */\
    /* *** int_ctrl *** */\
    /* *** cortexm4 *** */\
    0x0C48, /* icode_dcode_block_0_remap|icode_dcode_block_1_remap|icode_dcode_block_2_remap|icode_dcode_block_3_remap */ \
    0x0C49, /* unnamedpb_cortexm4_0x0008_1 */ \
    0x0C4C, /* flash_virt_window_0_offset|flash_virt_window_1_offset|flash_virt_window_2_offset|flash_virt_window_3_offset */ \
    0x0C4D, /* unnamedpb_cortexm4_0x000c_1 */ \
    0x0C4E, /* unnamedpb_cortexm4_0x000c_2 */ \
    0x0C4F, /* unnamedpb_cortexm4_0x000c_3 */ \
    0x0C50, /* flash_virt_window_4_offset|flash_virt_window_5_offset|flash_virt_window_6_offset|flash_virt_window_7_offset */ \
    0x0C51, /* unnamedpb_cortexm4_0x0010_1 */ \
    0x0C52, /* unnamedpb_cortexm4_0x0010_2 */ \
    0x0C53, /* unnamedpb_cortexm4_0x0010_3 */ \
    /* *** mm *** */\
    0x0E94, /* conv_0_buffer_disable|conv_1_buffer_disable|conv_2_buffer_disable|conv_3_buffer_disable|conv_4_buffer_disable|conv_4_prefetcher_disable|conv_4_brchstat1_halt_prefetch_disable|conv_4_brchstat2_halt_prefetch_disable|conv_4_brchstat3_halt_prefetch_disable|conv_4_replacement_config */ \
    0x0E95, /* flash_conv_config_1 */ \
    /* *** pbm_adm *** */\
    /* *** gpio *** */\
    0x100A, /* exti0_port_sel|exti1_port_sel|exti2_port_sel|exti3_port_sel|exti4_port_sel|exti5_port_sel|exti6_port_sel|exti7_port_sel */ \
    0x100B, /* exti_port_sel_1 */ \
    0x100C, /* exti0_expected_value|exti1_expected_value|exti2_expected_value|exti3_expected_value|exti4_expected_value|exti5_expected_value|exti6_expected_value|exti7_expected_value */ \
    /* *** adcif *** */\
    0x1068, /* adc_ldo_refbits|adc_vref_refbits|adc_scaler_bias_cgm_res|adc_scaler_vcm_refbits|adc_spare|smux_resload|adc_clk_mux_pup|adc_clk_select|adc_clk_speed|adc_comp_bias_boost|adc_comp_bias_pup|adc_ldo_bleed_off|adc_ldo_pup|adc_ldo_resbypass|adc_ovp_pup|adc_scaler_bypass|adc_scaler_filter_enable|adc_scaler_pup|adc_test_vref|adc_vcm_buf_pup|adc_vref_buf_pup|adc_vref_resbypass|smux_adc_buf_n_fullscale|smux_adc_buf_n_pup|smux_adc_buf_p_fullscale|smux_adc_buf_p_pup|smux_adc_channel_sel_pup|smux_external_reference|smux_resload_en_n|smux_resload_en_p|smux_selftest_mode|smux_tsensor_pup|xo_clk_4m_pup */ \
    0x1069, /* unnamedpb_adcif_0x0028_1 */ \
    0x106A, /* unnamedpb_adcif_0x0028_2 */ \
    0x106B, /* unnamedpb_adcif_0x0028_3 */ \
    0x106C, /* unnamedpb_adcif_0x0028_4 */ \
    0x106D, /* unnamedpb_adcif_0x0028_5 */ \
    /* *** uart_0 *** */\
    /* *** watchdog *** */\
    /* *** timers *** */\
    /* 0x1380 tmr0_prescaler_div|tmr0_clk_sel */ \
    0x1382, /* tmr0_threshold */ \
    0x1383, /* unnamedpb_timers_0x0002_1 */ \
    /* 0x138C tmr1_prescaler_div|tmr1_clk_sel */ \
    0x138E, /* tmr1_threshold */ \
    0x138F, /* unnamedpb_timers_0x000e_1 */ \
    0x1398, /* tmr2_prescaler_div|tmr2_clk_sel */ \
    0x139A, /* tmr2_threshold */ \
    0x139B, /* unnamedpb_timers_0x001a_1 */ \
    /* 0x13A4 tmr3_prescaler_div|tmr3_clk_sel */ \
    0x13A6, /* tmr3_threshold */ \
    0x13A7, /* unnamedpb_timers_0x0026_1 */ \
    /* 0x13B0 tmr4_prescaler_div|tmr4_clk_sel */ \
    0x13B2, /* tmr4_threshold */ \
    0x13B3, /* unnamedpb_timers_0x0032_1 */ \
    0
#endif //_GPBSP_PTC_PROD_QPG6105_10DBM_H_

