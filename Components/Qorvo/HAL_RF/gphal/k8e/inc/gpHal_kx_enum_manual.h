/*
 * Copyright (c) 2014-2016, GreenPeak Technologies
 * Copyright (c) 2017, Qorvo Inc
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
 *  F
 */
#ifndef _ENUM_MANUAL_H
#define _ENUM_MANUAL_H


/*
 *  this is a file not only for enums but for all kind of stuff
 * that should be derived from regmap
 */

//Backwards compatibility
#define GPHAL_ENUM_EVENT_STATE_INVALID                              GP_WB_ENUM_EVENT_STATE_INVALID
#define GPHAL_ENUM_EVENT_STATE_SCHEDULED                            GP_WB_ENUM_EVENT_STATE_SCHEDULED
#define GPHAL_ENUM_EVENT_STATE_SCHEDULED_FOR_IMMEDIATE_EXECUTION    GP_WB_ENUM_EVENT_STATE_SCHEDULED_FOR_IMMEDIATE_EXECUTION
#define GPHAL_ENUM_EVENT_STATE_RESCHEDULED                          GP_WB_ENUM_EVENT_STATE_RESCHEDULED
#define GPHAL_ENUM_EVENT_STATE_DONE                                 GP_WB_ENUM_EVENT_STATE_DONE

#define GPHAL_ENUM_EVENT_RESULT_UNKNOWN                             GP_WB_ENUM_EVENT_RESULT_UNKNOWN
#define GPHAL_ENUM_EVENT_RESULT_EXECUTED_ON_TIME                    GP_WB_ENUM_EVENT_RESULT_EXECUTED_ON_TIME
#define GPHAL_ENUM_EVENT_RESULT_EXECUTED_TOO_LATE                   GP_WB_ENUM_EVENT_RESULT_EXECUTED_TOO_LATE
#define GPHAL_ENUM_EVENT_RESULT_MISSED_TOO_LATE                     GP_WB_ENUM_EVENT_RESULT_MISSED_TOO_LATE

//Frame type filter mask
#define GPHAL_ENUM_FRAME_TYPE_FILTER_BCN_MASK                       GP_WB_MACFILT_ACCEPT_FT_BCN_MASK
#define GPHAL_ENUM_FRAME_TYPE_FILTER_DATA_MASK                      GP_WB_MACFILT_ACCEPT_FT_DATA_MASK
#define GPHAL_ENUM_FRAME_TYPE_FILTER_ACK_MASK                       GP_WB_MACFILT_ACCEPT_FT_ACK_MASK
#define GPHAL_ENUM_FRAME_TYPE_FILTER_CMD_MASK                       GP_WB_MACFILT_ACCEPT_FT_CMD_MASK
#define GPHAL_ENUM_FRAME_TYPE_FILTER_RSV_MASK                      (GP_WB_MACFILT_ACCEPT_FT_RSV_4_MASK \
                                                                  | GP_WB_MACFILT_ACCEPT_FT_MP_MASK \
                                                                  | GP_WB_MACFILT_ACCEPT_FT_FRA_MASK \
                                                                  | GP_WB_MACFILT_ACCEPT_FT_EXT_MASK)

// BLE pdu types mask
#define GPHAL_ENUM_FILTER_ACCEPT_LIST_ENTRY_TYPE_MASK                        GP_WB_BLE_WHITELIST_ENTRY_S_ENTRY_TYPE_MASK
#define GPHAL_ENUM_FILTER_ACCEPT_LIST_ENTRY_ADVERTISING_VALID_MASK           GP_WB_BLE_WHITELIST_ENTRY_S_ADVERTISING_VALID_MASK
#define GPHAL_ENUM_FILTER_ACCEPT_LIST_ENTRY_SCANNING_VALID_MASK              GP_WB_BLE_WHITELIST_ENTRY_S_SCANNING_VALID_MASK
#define GPHAL_ENUM_FILTER_ACCEPT_LIST_ENTRY_INITIATING_VALID_MASK            GP_WB_BLE_WHITELIST_ENTRY_S_INTIATING_VALID_MASK
#define GPHAL_ENUM_FILTER_ACCEPT_LIST_ENTRY_PERSYNC_VALID_MASK               GP_WB_BLE_WHITELIST_ENTRY_S_PERSYNC_VALID_MASK

#define GPHAL_ENUM_FILTER_ACCEPT_LIST_ENTRY_REGULAR_IDX                      GP_WB_BLE_WHITELIST_ENTRY_S_ENTRY_TYPE_LSB
#define GPHAL_ENUM_FILTER_ACCEPT_LIST_ENTRY_ADVERTISING_IDX                  GP_WB_BLE_WHITELIST_ENTRY_S_ADVERTISING_VALID_LSB
#define GPHAL_ENUM_FILTER_ACCEPT_LIST_ENTRY_INITIATING_IDX                   GP_WB_BLE_WHITELIST_ENTRY_S_INTIATING_VALID_LSB
#define GPHAL_ENUM_FILTER_ACCEPT_LIST_ENTRY_SCANNING_IDX                     GP_WB_BLE_WHITELIST_ENTRY_S_SCANNING_VALID_LSB
#define GPHAL_ENUM_FILTER_ACCEPT_LIST_ENTRY_PERSYNC_IDX                      GP_WB_BLE_WHITELIST_ENTRY_S_PERSYNC_VALID_LSB

// BLE advertising frame type accept mask
#define GPHAL_ENUM_ADV_FRAME_TYPE_ACCEPT_MASK_ADV_IND               GP_WB_ADV_EV_INFO_ACCEPT_FT_ADV_IND_MASK
#define GPHAL_ENUM_ADV_FRAME_TYPE_ACCEPT_MASK_ADV_DIRECT_IND        GP_WB_ADV_EV_INFO_ACCEPT_FT_ADV_DIRECT_IND_MASK
#define GPHAL_ENUM_ADV_FRAME_TYPE_ACCEPT_MASK_ADV_NONCONN_IND       GP_WB_ADV_EV_INFO_ACCEPT_FT_ADV_NONCONN_IND_MASK
#define GPHAL_ENUM_ADV_FRAME_TYPE_ACCEPT_MASK_SCAN_REQ              GP_WB_ADV_EV_INFO_ACCEPT_FT_SCAN_REQ_MASK
#define GPHAL_ENUM_ADV_FRAME_TYPE_ACCEPT_MASK_SCAN_RSP              GP_WB_ADV_EV_INFO_ACCEPT_FT_SCAN_RSP_MASK
#define GPHAL_ENUM_ADV_FRAME_TYPE_ACCEPT_MASK_CONNECT_REQ           GP_WB_ADV_EV_INFO_ACCEPT_FT_CONNECT_REQ_MASK
#define GPHAL_ENUM_ADV_FRAME_TYPE_ACCEPT_MASK_ADV_SCAN_IND          GP_WB_ADV_EV_INFO_ACCEPT_FT_ADV_SCAN_IND_MASK

// BLE advertising white list enable mask
#define GPHAL_ENUM_ADV_EV_INFO_FT_ADV_IND_WL_EN_MASK                GP_WB_ADV_EV_INFO_FT_ADV_IND_WL_EN_MASK
#define GPHAL_ENUM_ADV_EV_INFO_FT_ADV_DIRECT_IND_WL_EN_MASK         GP_WB_ADV_EV_INFO_FT_ADV_DIRECT_IND_WL_EN_MASK
#define GPHAL_ENUM_ADV_EV_INFO_FT_ADV_NONCONN_IND_WL_EN_MASK        GP_WB_ADV_EV_INFO_FT_ADV_NONCONN_IND_WL_EN_MASK
#define GPHAL_ENUM_ADV_EV_INFO_FT_SCAN_REQ_WL_EN_MASK               GP_WB_ADV_EV_INFO_FT_SCAN_REQ_WL_EN_MASK
#define GPHAL_ENUM_ADV_EV_INFO_FT_AUX_SCAN_REQ_WL_EN_MASK           GP_WB_ADV_EV_INFO_FT_AUX_SCAN_REQ_WL_EN_MASK
#define GPHAL_ENUM_ADV_EV_INFO_FT_SCAN_RSP_WL_EN_MASK               GP_WB_ADV_EV_INFO_FT_SCAN_RSP_WL_EN_MASK
#define GPHAL_ENUM_ADV_EV_INFO_FT_CONNECT_REQ_WL_EN_MASK            GP_WB_ADV_EV_INFO_FT_CONNECT_REQ_WL_EN_MASK
#define GPHAL_ENUM_ADV_EV_INFO_FT_AUX_CONNECT_REQ_WL_EN_MASK        GP_WB_ADV_EV_INFO_FT_AUX_CONNECT_REQ_WL_EN_MASK
#define GPHAL_ENUM_ADV_EV_INFO_FT_ADV_SCAN_IND_WL_EN_MASK           GP_WB_ADV_EV_INFO_FT_ADV_SCAN_IND_WL_EN_MASK
#define GPHAL_ENUM_ADV_EV_INFO_FT_RESERVED_WL_EN_MASK               GP_WB_ADV_EV_INFO_FT_RESERVED_WL_EN_MASK

// BLE scanning frame type accept mask
#define GPHAL_ENUM_SCAN_FRAME_TYPE_ACCEPT_MASK_ADV_IND              GP_WB_SCAN_EV_INFO_ACCEPT_FT_ADV_IND_MASK
#define GPHAL_ENUM_SCAN_FRAME_TYPE_ACCEPT_MASK_ADV_DIRECT_IND       GP_WB_SCAN_EV_INFO_ACCEPT_FT_ADV_DIRECT_IND_MASK
#define GPHAL_ENUM_SCAN_FRAME_TYPE_ACCEPT_MASK_ADV_NONCONN_IND      GP_WB_SCAN_EV_INFO_ACCEPT_FT_ADV_NONCONN_IND_MASK
#define GPHAL_ENUM_SCAN_FRAME_TYPE_ACCEPT_MASK_SCAN_REQ             GP_WB_SCAN_EV_INFO_ACCEPT_FT_SCAN_REQ_MASK
#define GPHAL_ENUM_SCAN_FRAME_TYPE_ACCEPT_MASK_SCAN_RSP             GP_WB_SCAN_EV_INFO_ACCEPT_FT_SCAN_RSP_MASK
#define GPHAL_ENUM_SCAN_FRAME_TYPE_ACCEPT_MASK_CONNECT_REQ          GP_WB_SCAN_EV_INFO_ACCEPT_FT_CONNECT_REQ_MASK
#define GPHAL_ENUM_SCAN_FRAME_TYPE_ACCEPT_MASK_ADV_SCAN_IND         GP_WB_SCAN_EV_INFO_ACCEPT_FT_ADV_SCAN_IND_MASK
#define GPHAL_ENUM_SCAN_FRAME_TYPE_ACCEPT_MASK_ADV_EXT_IND          GP_WB_SCAN_EV_INFO_ACCEPT_FT_ADV_EXT_IND_MASK
#define GPHAL_ENUM_SCAN_FRAME_TYPE_ACCEPT_MASK_AUX_CONNECT_RSP      GP_WB_SCAN_EV_INFO_ACCEPT_FT_AUX_CONNECT_RSP_MASK

// BLE scanning white list enable mask
#define GPHAL_ENUM_SCAN_EV_INFO_FT_ADV_IND_WL_EN_MASK                GP_WB_SCAN_EV_INFO_FT_ADV_IND_WL_EN_MASK
#define GPHAL_ENUM_SCAN_EV_INFO_FT_ADV_DIRECT_IND_WL_EN_MASK         GP_WB_SCAN_EV_INFO_FT_ADV_DIRECT_IND_WL_EN_MASK
#define GPHAL_ENUM_SCAN_EV_INFO_FT_ADV_NONCONN_IND_WL_EN_MASK        GP_WB_SCAN_EV_INFO_FT_ADV_NONCONN_IND_WL_EN_MASK
#define GPHAL_ENUM_SCAN_EV_INFO_FT_SCAN_REQ_WL_EN_MASK               GP_WB_SCAN_EV_INFO_FT_SCAN_REQ_WL_EN_MASK
#define GPHAL_ENUM_SCAN_EV_INFO_FT_SCAN_RSP_WL_EN_MASK               GP_WB_SCAN_EV_INFO_FT_SCAN_RSP_WL_EN_MASK
#define GPHAL_ENUM_SCAN_EV_INFO_FT_CONNECT_REQ_WL_EN_MASK            GP_WB_SCAN_EV_INFO_FT_CONNECT_REQ_WL_EN_MASK
#define GPHAL_ENUM_SCAN_EV_INFO_FT_ADV_SCAN_IND_WL_EN_MASK           GP_WB_SCAN_EV_INFO_FT_ADV_SCAN_IND_WL_EN_MASK
#define GPHAL_ENUM_SCAN_EV_INFO_FT_ADV_EXT_IND_WL_EN_MASK            GP_WB_SCAN_EV_INFO_FT_ADV_EXT_IND_WL_EN_MASK
#define GPHAL_ENUM_SCAN_EV_INFO_FT_RESERVED_WL_EN_MASK               GP_WB_SCAN_EV_INFO_FT_RESERVED_WL_EN_MASK

// BLE init frame type accept mask
#define GPHAL_ENUM_INIT_FRAME_TYPE_ACCEPT_MASK_ADV_IND              GP_WB_INIT_EV_INFO_ACCEPT_FT_ADV_IND_MASK
#define GPHAL_ENUM_INIT_FRAME_TYPE_ACCEPT_MASK_ADV_DIRECT_IND       GP_WB_INIT_EV_INFO_ACCEPT_FT_ADV_DIRECT_IND_MASK
#define GPHAL_ENUM_INIT_FRAME_TYPE_ACCEPT_MASK_ADV_NONCONN_IND      GP_WB_INIT_EV_INFO_ACCEPT_FT_ADV_NONCONN_IND_MASK
#define GPHAL_ENUM_INIT_FRAME_TYPE_ACCEPT_MASK_SCAN_REQ             GP_WB_INIT_EV_INFO_ACCEPT_FT_SCAN_REQ_MASK
#define GPHAL_ENUM_INIT_FRAME_TYPE_ACCEPT_MASK_SCAN_RSP             GP_WB_INIT_EV_INFO_ACCEPT_FT_SCAN_RSP_MASK
#define GPHAL_ENUM_INIT_FRAME_TYPE_ACCEPT_MASK_CONNECT_REQ          GP_WB_INIT_EV_INFO_ACCEPT_FT_CONNECT_REQ_MASK
#define GPHAL_ENUM_INIT_FRAME_TYPE_ACCEPT_MASK_ADV_SCAN_IND         GP_WB_INIT_EV_INFO_ACCEPT_FT_ADV_SCAN_IND_MASK
#define GPHAL_ENUM_INIT_FRAME_TYPE_ACCEPT_MASK_ADV_EXT_IND          GP_WB_INIT_EV_INFO_ACCEPT_FT_ADV_EXT_IND_MASK
#define GPHAL_ENUM_INIT_FRAME_TYPE_ACCEPT_MASK_AUX_CONNECT_RSP      GP_WB_INIT_EV_INFO_ACCEPT_FT_AUX_CONNECT_RSP_MASK

// BLE init whitelist enable mask
#define GPHAL_ENUM_INIT_EV_INFO_FT_ADV_IND_WL_EN_MASK               GP_WB_INIT_EV_INFO_FT_ADV_IND_WL_EN_MASK
#define GPHAL_ENUM_INIT_EV_INFO_FT_ADV_DIRECT_IND_WL_EN_MASK        GP_WB_INIT_EV_INFO_FT_ADV_DIRECT_IND_WL_EN_MASK
#define GPHAL_ENUM_INIT_EV_INFO_FT_ADV_NONCONN_IND_WL_EN_MASK       GP_WB_INIT_EV_INFO_FT_ADV_NONCONN_IND_WL_EN_MASK
#define GPHAL_ENUM_INIT_EV_INFO_FT_SCAN_REQ_WL_EN_MASK              GP_WB_INIT_EV_INFO_FT_SCAN_REQ_WL_EN_MASK
#define GPHAL_ENUM_INIT_EV_INFO_FT_SCAN_RSP_WL_EN_MASK              GP_WB_INIT_EV_INFO_FT_SCAN_RSP_WL_EN_MASK
#define GPHAL_ENUM_INIT_EV_INFO_FT_CONNECT_REQ_WL_EN_MASK           GP_WB_INIT_EV_INFO_FT_CONNECT_REQ_WL_EN_MASK
#define GPHAL_ENUM_INIT_EV_INFO_FT_ADV_SCAN_IND_WL_EN_MASK          GP_WB_INIT_EV_INFO_FT_ADV_SCAN_IND_WL_EN_MASK
#define GPHAL_ENUM_INIT_EV_INFO_FT_ADV_EXT_IND_WL_EN_MASK           GP_WB_INIT_EV_INFO_FT_ADV_EXT_IND_WL_EN_MASK
#define GPHAL_ENUM_INIT_EV_INFO_FT_RESERVED_WL_EN_MASK              GP_WB_INIT_EV_INFO_FT_RESERVED_WL_EN_MASK

// event types
#define GPHAL_ENUM_EVENT_TYPE_ADVERTISE                             GP_WB_ENUM_EVENT_TYPE_BLE_ADVERTISING
#define GPHAL_ENUM_EVENT_TYPE_SCAN                                  GP_WB_ENUM_EVENT_TYPE_BLE_SCANNING
#define GPHAL_ENUM_EVENT_TYPE_INITIATING                            GP_WB_ENUM_EVENT_TYPE_BLE_INITIATING
#define GPHAL_ENUM_EVENT_TYPE_VIRTUAL                               GP_WB_ENUM_EVENT_TYPE_BLE_VIRTUAL
#define GPHAL_ENUM_EVENT_TYPE_CONNECTION_M                          GP_WB_ENUM_EVENT_TYPE_BLE_CONNECTION_M
#define GPHAL_ENUM_EVENT_TYPE_CONNECTION_S                          GP_WB_ENUM_EVENT_TYPE_BLE_CONNECTION_S
#define GPHAL_ENUM_EVENT_TYPE_SUBEVENT                              GP_WB_ENUM_EVENT_TYPE_BLE_SUBEVENT
#define GPHAL_ENUM_EVENT_TYPE_BG_SCANNING                           GP_WB_ENUM_EVENT_TYPE_BLE_BG_SCANNING


#define GPHAL_ENUM_ADCIF_CHANNEL_VBATT                              GP_WB_ENUM_ADC_CHANNEL_VBAT
#define GPHAL_ENUM_ADCIF_VOLTAGE_RANGE_0V_2V                        255 /* dummy value - not used */

#define GP_MM_RAM_PBM_ZIGBEE_DATA_SIZE                              0x80

// Phy modes
#define GPHAL_ENUM_PHY_MODE_TX_BLE                                  GP_WB_ENUM_BLE_TRANSMITTER_MODE_BLE
#define GPHAL_ENUM_PHY_MODE_TX_BLE_HDR                              GP_WB_ENUM_BLE_TRANSMITTER_MODE_BLE_HDR
//#define GPHAL_ENUM_PHY_MODE_TX_BLE_LR125                            GP_WB_ENUM_BLE_TRANSMITTER_MODE_BLE_LR125
//#define GPHAL_ENUM_PHY_MODE_TX_BLE_LR500                            GP_WB_ENUM_BLE_TRANSMITTER_MODE_BLE_LR500
#define GPHAL_ENUM_PHY_MODE_TX_BLE_INVALID                          GP_WB_ENUM_BLE_TRANSMITTER_MODE_BLE_HDR + 1

#define GPHAL_ENUM_PHY_MODE_RX_BLE                                  GP_WB_ENUM_BLE_RECEIVER_MODE_BLE
#define GPHAL_ENUM_PHY_MODE_RX_BLE_HDR                              GP_WB_ENUM_BLE_RECEIVER_MODE_BLE_HDR
//#define GPHAL_ENUM_PHY_MODE_RX_BLE_LR                               GP_WB_ENUM_BLE_RECEIVER_MODE_BLE_LR
#define GPHAL_ENUM_PHY_MODE_RX_BLE_INVALID                          GP_WB_ENUM_BLE_RECEIVER_MODE_BLE_HDR + 1

#endif //_ENUM_MANUAL_H

