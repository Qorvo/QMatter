/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  Device Manager subsystem API.
 *
 *  Copyright (c) 2016-2019 Arm Ltd. All Rights Reserved.
 *
 *  Copyright (c) 2019-2021 Packetcraft, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*
 * Copyright (c) 2021, Qorvo Inc
 *
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
 */
/*************************************************************************************************/
#ifndef DM_API_H
#define DM_API_H

#include "hci_api.h"
#include "cfg_stack.h"
#include "smp_defs.h"
#include "sec_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \addtogroup STACK_DM_API
 *  \{ */

/**************************************************************************************************
  Macros
**************************************************************************************************/

/** \name GAP Device Role
 * Connectable GAP Roles.
 */
/**@{*/
#define DM_ROLE_MASTER              HCI_ROLE_MASTER  /*!< Role is master */
#define DM_ROLE_SLAVE               HCI_ROLE_SLAVE   /*!< Role is slave */
/**@}*/

/** \name GAP Discovery Mode
 * When setup as a discoverable device, these are the possible modes of discovery.
 */
/**@{*/
#define DM_DISC_MODE_NONE           0     /*!< GAP non-discoverable */
#define DM_DISC_MODE_LIMITED        1     /*!< GAP limited discoverable mode */
#define DM_DISC_MODE_GENERAL        2     /*!< GAP general discoverable mode */
/**@}*/

/** \name GAP Advertising Type
 * Type of connectable or disconverable advertising to perform.
 */
/**@{*/
#define DM_ADV_CONN_UNDIRECT        0     /*!< Connectable and scannable undirected advertising */
#define DM_ADV_CONN_DIRECT          1     /*!< Connectable directed advertising */
#define DM_ADV_SCAN_UNDIRECT        2     /*!< Scannable undirected advertising */
#define DM_ADV_NONCONN_UNDIRECT     3     /*!< Non-connectable and non-scannable undirected advertising */
#define DM_ADV_CONN_DIRECT_LO_DUTY  4     /*!< Connectable directed low duty cycle advertising */
/**@}*/

/** \name GAP AE Advertising Types
 * Advertising extension types - AE only.
 */
/**@{*/
#define DM_EXT_ADV_CONN_UNDIRECT    5     /*!< Connectable undirected advertising */
#define DM_EXT_ADV_NONCONN_DIRECT   6     /*!< Non-connectable and non-scannable directed advertising */
#define DM_EXT_ADV_SCAN_DIRECT      7     /*!< Scannable directed advertising */
#define DM_ADV_NONE                 255   /*!< For internal use only */
/**@}*/

/** \name GAP Advertising Report Type
 * Type of an advertising report observed while scanning.
 */
/**@{*/
#define DM_RPT_CONN_UNDIRECT        0    /*!< Connectable and scannable undirected advertising */
#define DM_RPT_CONN_DIRECT          1    /*!< Connectable directed advertising */
#define DM_RPT_SCAN_UNDIRECT        2    /*!< Scannable undirected advertising */
#define DM_RPT_NONCONN_UNDIRECT     3    /*!< Non-connectable undirected advertising */
#define DM_RPT_SCAN_RESPONSE        4    /*!< Scan response */
/**@}*/

/** \name GAP Advertising Data Location
 * Whether data is located in the advertising data or in the scan response data
 */
/**@{*/
#define DM_DATA_LOC_ADV             0     /*!< Locate data in the advertising data */
#define DM_DATA_LOC_SCAN            1     /*!< Locate data in the scan response data */
/**@}*/

/** \name GAP Scan Type
 * When setup as a connectable or observer device, this is the type of scanning to perform.
 */
/**@{*/
#define DM_SCAN_TYPE_PASSIVE        0     /*!< Passive scan */
#define DM_SCAN_TYPE_ACTIVE         1     /*!< Active scan */
/**@}*/

/** \name GAP Advertising Channel Map
 * Advertising channel map codes
 */
/**@{*/
#define DM_ADV_CHAN_37              HCI_ADV_CHAN_37  /*!< Advertising channel 37 */
#define DM_ADV_CHAN_38              HCI_ADV_CHAN_38  /*!< Advertising channel 38 */
#define DM_ADV_CHAN_39              HCI_ADV_CHAN_39  /*!< Advertising channel 39 */
/*! \brief All advertising channels */
#define DM_ADV_CHAN_ALL             (HCI_ADV_CHAN_37 | HCI_ADV_CHAN_38 | HCI_ADV_CHAN_39)
/**@}*/

/** \name DM Client IDs
 * The client ID parameter to function DmConnRegister()
 */
/**@{*/
#define DM_CLIENT_ID_ATT            0     /*!< Identifier for attribute protocol, for internal use only */
#define DM_CLIENT_ID_SMP            1     /*!< Identifier for security manager protocol, for internal use only */
#define DM_CLIENT_ID_DM             2     /*!< Identifier for device manager, for internal use only */
#define DM_CLIENT_ID_APP            3     /*!< Identifier for the application */
#define DM_CLIENT_ID_L2C            4     /*!< Identifier for L2CAP */
#define DM_CLIENT_ID_MAX            5     /*!< For internal use only */
/**@}*/

/** \name DM Unknown IDs
 * Values for unknown or unspecificed device identifiers.
 */
/**@{*/
/*! \brief Unknown connection ID or other error */
#define DM_CONN_ID_NONE             0

/*! \brief Unknown sync ID or other error */
#define DM_SYNC_ID_NONE             0

/*! \brief Unknown Connected Isochronous Group (CIG) ID or other error */
#define DM_CIG_ID_NONE              0xFF

/*! \brief Unknown Connected Isochronous Stream (CIS) ID or other error */
#define DM_CIS_ID_NONE              0xFF

/*! \brief Unknown Broadcast Isochronous Group (BIG) handle or other error */
#define DM_BIG_HANDLE_NONE          0xFF
/**@}*/

/** \name GAP Address Type
 * The address type to use over the air or that is associated with a received address.
 */
/**@{*/
#define DM_ADDR_PUBLIC              0x00  /*!< Public device address */
#define DM_ADDR_RANDOM              0x01  /*!< Random device address */
#define DM_ADDR_PUBLIC_IDENTITY     0x02  /*!< Public identity address (corresponds to resolved private address) */
#define DM_ADDR_RANDOM_IDENTITY     0x03  /*!< Random (static) identity address (corresponds to resolved private address) */
#define DM_ADDR_RANDOM_UNRESOLVED   0xFE  /*!< Random device address (Controller unable to resolve) */
#define DM_ADDR_NONE                0xFF  /*!< No address provided (anonymous) */
/**@}*/

/** \name GAP Advertising Data Types
 * Advertising data types flags.
 */
/**@{*/
#define DM_ADV_TYPE_FLAGS           0x01  /*!< Flag bits */
#define DM_ADV_TYPE_16_UUID_PART    0x02  /*!< Partial list of 16 bit UUIDs */
#define DM_ADV_TYPE_16_UUID         0x03  /*!< Complete list of 16 bit UUIDs */
#define DM_ADV_TYPE_32_UUID_PART    0x04  /*!< Partial list of 32 bit UUIDs */
#define DM_ADV_TYPE_32_UUID         0x05  /*!< Complete list of 32 bit UUIDs */
#define DM_ADV_TYPE_128_UUID_PART   0x06  /*!< Partial list of 128 bit UUIDs */
#define DM_ADV_TYPE_128_UUID        0x07  /*!< Complete list of 128 bit UUIDs */
#define DM_ADV_TYPE_SHORT_NAME      0x08  /*!< Shortened local name */
#define DM_ADV_TYPE_LOCAL_NAME      0x09  /*!< Complete local name */
#define DM_ADV_TYPE_TX_POWER        0x0A  /*!< TX power level */
#define DM_ADV_TYPE_SM_TK_VALUE     0x10  /*!< Security manager TK value */
#define DM_ADV_TYPE_SM_OOB_FLAGS    0x11  /*!< Security manager OOB flags */
#define DM_ADV_TYPE_CONN_INTERVAL   0x12  /*!< Peripheral connection interval range */
#define DM_ADV_TYPE_SIGNED_DATA     0x13  /*!< Signed data */
#define DM_ADV_TYPE_16_SOLICIT      0x14  /*!< Service soliticiation list of 16 bit UUIDs */
#define DM_ADV_TYPE_128_SOLICIT     0x15  /*!< Service soliticiation list of 128 bit UUIDs */
#define DM_ADV_TYPE_SERVICE_DATA    0x16  /*!< Service data - 16-bit UUID */
#define DM_ADV_TYPE_PUBLIC_TARGET   0x17  /*!< Public target address */
#define DM_ADV_TYPE_RANDOM_TARGET   0x18  /*!< Random target address */
#define DM_ADV_TYPE_APPEARANCE      0x19  /*!< Device appearance */
#define DM_ADV_TYPE_ADV_INTERVAL    0x1A  /*!< Advertising interval */
#define DM_ADV_TYPE_BD_ADDR         0x1B  /*!< LE Bluetooth device address */
#define DM_ADV_TYPE_ROLE            0x1C  /*!< LE role */
#define DM_ADV_TYPE_32_SOLICIT      0x1F  /*!< Service soliticiation list of 32 bit UUIDs */
#define DM_ADV_TYPE_SVC_DATA_32     0x20  /*!< Service data - 32-bit UUID */
#define DM_ADV_TYPE_SVC_DATA_128    0x21  /*!< Service data - 128-bit UUID */
#define DM_ADV_TYPE_LESC_CONFIRM    0x22  /*!< LE Secure Connections confirm value */
#define DM_ADV_TYPE_LESC_RANDOM     0x23  /*!< LE Secure Connections random value */
#define DM_ADV_TYPE_URI             0x24  /*!< URI */
#define DM_ADV_TYPE_INDOOR_POS      0x25  /*!< Indoor positioning service */
#define DM_ADV_TYPE_TRANS_DISC      0x26  /*!< Transport discovery service */
#define DM_ADV_TYPE_LE_SUP_FEAT     0x27  /*!< LE supported features */
#define DM_ADV_TYPE_CH_MAP_UPD_IND  0x28  /*!< Channel map update indication */
#define DM_ADV_TYPE_PB_ADV          0x29  /*!< PB-ADV */
#define DM_ADV_TYPE_MESH_MSG        0x2A  /*!< Mesh message */
#define DM_ADV_TYPE_MESH_BEACON     0x2B  /*!< Mesh beacon*/
#define DM_ADV_TYPE_BIG_INFO        0x2C  /*!< BIG Info */
#define DM_ADV_TYPE_BCAST_CODE      0x2D  /*!< Broadcast_Code */
#define DM_ADV_TYPE_3D_INFO_DATA    0x3D  /*!< 3D information data */
#define DM_ADV_TYPE_ADV_INTERVAL_LG 0x3E  /*!< Advertising interval - long */
#define DM_ADV_TYPE_MANUFACTURER    0xFF  /*!< Manufacturer specific data */
/**@}*/

/** \name GAP Advertising Data Flag Advertising Type
 * Bit mask for Advertising Type flag in advertising data.
 */
/**@{*/
#define DM_FLAG_LE_LIMITED_DISC     0x01  /*!< Limited discoverable flag */
#define DM_FLAG_LE_GENERAL_DISC     0x02  /*!< General discoverable flag */
#define DM_FLAG_LE_BREDR_NOT_SUP    0x04  /*!< BR/EDR not supported flag */
/**@}*/

/** \name GAP Advertising Data Element Indexes
 * Advertising data element indexes.
 */
/**@{*/
#define DM_AD_LEN_IDX               0     /*!< Advertising data element len */
#define DM_AD_TYPE_IDX              1     /*!< Advertising data element type */
#define DM_AD_DATA_IDX              2     /*!< Advertising data element data */
/**@}*/

/** \name GAP Advertising URI
 * Advertising URI Scheme
 */
/**@{*/
#define DM_URI_SCHEME_HTTP          0x16  /*!< URI HTTP Scheme */
#define DM_URI_SCHEME_HTTPS         0x17  /*!< URI HTTPS Scheme */
/**@}*/

/** \name GAP Timeouts
 * Timeouts defined by the GAP specification; in units of milliseconds.
 */
/**@{*/
#define DM_GAP_LIM_ADV_TIMEOUT      180000  /*!< Maximum advertising duration in limited discoverable mode */
#define DM_GAP_GEN_DISC_SCAN_MIN    10240   /*!< Minimum scan duration for general discovery */
#define DM_GAP_LIM_DISC_SCAN_MIN    10240   /*!< Minimum scan duration for limited discovery */
#define DM_GAP_CONN_PARAM_TIMEOUT   30000   /*!< Connection parameter update timeout */
#define DM_GAP_SCAN_FAST_PERIOD     30720   /*!< Minimum time to perform scanning when user initiated */
#define DM_GAP_ADV_FAST_PERIOD      30000   /*!< Minimum time to perform advertising when user initiated */
/**@}*/

/** \name GAP 1M PHY Timing
 * Advertising, scanning, and connection parameters defined in the GAP specification for the LE 1M PHY.
 * In units of 625 microseconds.
 */
/**@{*/
#define DM_GAP_SCAN_FAST_INT_MIN          48      /*!< Minimum scan interval when user initiated */
#define DM_GAP_SCAN_FAST_INT_MAX          96      /*!< Maximum scan interval when user initiated */
#define DM_GAP_SCAN_FAST_WINDOW           48      /*!< Scan window when user initiated */
#define DM_GAP_SCAN_SLOW_INT_1            2048    /*!< Scan interval 1 when background scannning */
#define DM_GAP_SCAN_SLOW_WINDOW_1         18      /*!< Scan window 1 when background scanning */
#define DM_GAP_SCAN_SLOW_INT_2            4096    /*!< Scan interval 2 when background scannning */
#define DM_GAP_SCAN_SLOW_WINDOW_2         36      /*!< Scan window 2 when background scanning */
#define DM_GAP_ADV_FAST_INT_MIN_1         48      /*!< Minimum advertising interval 1 when user initiated */
#define DM_GAP_ADV_FAST_INT_MAX_1         96      /*!< Maximum advertising interval 1 when user initiated */
#define DM_GAP_ADV_FAST_INT_MIN_2         160     /*!< Minimum advertising interval 2 when user initiated */
#define DM_GAP_ADV_FAST_INT_MAX_2         240     /*!< Maximum advertising interval 2 when user initiated */
#define DM_GAP_ADV_SLOW_INT_MIN           1600    /*!< Minimum advertising interval when background advertising */
#define DM_GAP_ADV_SLOW_INT_MAX           1920    /*!< Maximum advertising interval when background advertising */
/**@}*/

/** \name GAP Coded PHY Timing
 * Advertising, scanning, and connection parameters defined in the GAP specification for the LE Coded PHY.
 * In units of 625 microseconds.
 */
/**@{*/
#define DM_GAP_SCAN_CODED_FAST_INT_MIN    144     /*!< Minimum scan interval when user initiated on LE Coded PHY */
#define DM_GAP_SCAN_CODED_FAST_INT_MAX    288     /*!< Maximum scan interval when user initiated on LE Coded PHY */
#define DM_GAP_SCAN_CODED_FAST_WINDOW     144     /*!< Scan window when user initiated on LE Coded PHY */
#define DM_GAP_SCAN_CODED_SLOW_INT_1      6144    /*!< Scan interval 1 when background scannning on LE Coded PHY  */
#define DM_GAP_SCAN_CODED_SLOW_WINDOW_1   54      /*!< Scan window 1 when background scanning on LE Coded PHY */
#define DM_GAP_SCAN_CODED_SLOW_INT_2      12288   /*!< Scan interval 2 when background scannning on LE Coded PHY */
#define DM_GAP_SCAN_CODED_SLOW_WINDOW_2   108     /*!< Scan window 2 when background scanning on LE Coded PHY */
#define DM_GAP_ADV_CODED_FAST_INT_MIN_1   144     /*!< Minimum advertising interval 1 when user initiated on LE Coded PHY */
#define DM_GAP_ADV_CODED_FAST_INT_MAX_1   288     /*!< Maximum advertising interval 1 when user initiated on LE Coded PHY */
#define DM_GAP_ADV_CODED_FAST_INT_MIN_2   480     /*!< Minimum advertising interval 2 when user initiated on LE Coded PHY */
#define DM_GAP_ADV_CODED_FAST_INT_MAX_2   720     /*!< Maximum advertising interval 2 when user initiated on LE Coded PHY */
#define DM_GAP_ADV_CODED_SLOW_INT_MIN     4800    /*!< Minimum advertising interval when background advertising on LE Coded PHY */
#define DM_GAP_ADV_CODED_SLOW_INT_MAX     5760    /*!< Maximum advertising interval when background advertising on LE Coded PHY */
/**@}*/

/** \name GAP Connection Slave Latency
 *
 */
/**@{*/
/*! \brief GAP connection establishment slaves latency */
#define DM_GAP_CONN_EST_LATENCY     0
/**@}*/

/** \name GAP Connection Interval
 * GAP connection interval in 1.25ms units.
 */
/**@{*/
#define DM_GAP_INITIAL_CONN_INT_MIN 24      /*!< Minimum initial connection interval */
#define DM_GAP_INITIAL_CONN_INT_MAX 40      /*!< Maximum initial connection interval */
/**@}*/

/** \name GAP Connection Event Lengths
 * GAP connection establishment minimum and maximum connection event lengths.
 */
/**@{*/
#define DM_GAP_CONN_EST_MIN_CE_LEN  0  /*!< Connection establishment minimum event length */
#define DM_GAP_CONN_EST_MAX_CE_LEN  0  /*!< Connection establishment maximum event length */
/**@}*/

/** \name GAP Peripheral Privacy Characteristic Values
 *
 */
/**@{*/
#define DM_GAP_PRIV_DISABLED        0  /*!< Privacy Disabled */
#define DM_GAP_PRIV_ENABLED         1  /*!< Privacy Enabled */
/**@}*/

/** \name GAP Connection Supervision Timeout
 * Connection supervision timeout, in 10ms units
 */
/**@{*/
/*! \brief Connection establishment supervision timeout default, in 10ms units */
#ifndef DM_DEFAULT_EST_SUP_TIMEOUT
#define DM_DEFAULT_EST_SUP_TIMEOUT  2000
#endif //DM_DEFAULT_EST_SUP_TIMEOUT
/**@}*/

/** \name GAP Security Pairing Authentication Requirements
 * Pairing authentication/security properties bit mask.
 */
/**@{*/
#define DM_AUTH_BOND_FLAG           SMP_AUTH_BOND_FLAG  /*!< Bonding requested */
#define DM_AUTH_MITM_FLAG           SMP_AUTH_MITM_FLAG  /*!< MITM (authenticated pairing) requested */
#define DM_AUTH_SC_FLAG             SMP_AUTH_SC_FLAG    /*!< LE Secure Connections requested */
#define DM_AUTH_KP_FLAG             SMP_AUTH_KP_FLAG    /*!< Keypress notifications requested */
/**@}*/

/** \name GAP Key Distribution Flags
 * Key distribution bit mask
 */
/**@{*/
#define DM_KEY_DIST_LTK             SMP_KEY_DIST_ENC   /*!< Distribute LTK used for encryption */
#define DM_KEY_DIST_IRK             SMP_KEY_DIST_ID    /*!< Distribute IRK used for privacy */
#define DM_KEY_DIST_CSRK            SMP_KEY_DIST_SIGN  /*!< Distribute CSRK used for signed data */
/**@}*/

/** \name DM Security Key Indication Types
 * Type of key used in \ref DM_SEC_KEY_IND.
 */
/**@{*/
#define DM_KEY_LOCAL_LTK            0x01  /*!< LTK generated locally for this device */
#define DM_KEY_PEER_LTK             0x02  /*!< LTK received from peer device */
#define DM_KEY_IRK                  0x04  /*!< IRK and identity info of peer device */
#define DM_KEY_CSRK                 0x08  /*!< CSRK of peer device */
/**@}*/

/*! \brief Base value for HCI error status values for \ref DM_SEC_PAIR_CMPL_IND */
#define DM_SEC_HCI_ERR_BASE         0x20

/** \name GAP Security Level
 * GAP Mode 1 Security Levels
 */
/**@{*/
#define DM_SEC_LEVEL_NONE           0     /*!< Connection has no security */
#define DM_SEC_LEVEL_ENC            1     /*!< Connection is encrypted with unauthenticated key */
#define DM_SEC_LEVEL_ENC_AUTH       2     /*!< Connection is encrypted with authenticated key */
#define DM_SEC_LEVEL_ENC_LESC       3     /*!< Connection is encrypted with LE Secure Connections */
/**@}*/

/** \name GAP Broadcast Security Level
* GAP Mode 3 Security Levels
*/
/**@{*/
#define DM_SEC_LEVEL_BCAST_NONE     0     /*!< No security (no authentication and no encryption) */
#define DM_SEC_LEVEL_BCAST_UNAUTH   1     /*!< Use of unauthenticated Broadcast_Code */
#define DM_SEC_LEVEL_BCAST_AUTH     2     /*!< Use of authenticated Broadcast_Code */
/**@}*/

/** \name GAP Random Address Types
 * Random address type masks.
 */
/**@{*/
#define DM_RAND_ADDR_STATIC         0xC0  /*!< Static address */
#define DM_RAND_ADDR_RESOLV         0x40  /*!< Resolvable private address */
#define DM_RAND_ADDR_NONRESOLV      0x00  /*!< Non-resolvable private address */
/**@}*/

/** \name GAP Random Address Macros
 * Macros for identifying address type.
 */
/**@{*/
/*! \brief Get the type of random address */
#define DM_RAND_ADDR_GET(addr)          ((addr)[5] & 0xC0)

/*! \brief Set the type of random address */
#define DM_RAND_ADDR_SET(addr, type)    {(addr)[5] = ((addr)[5] & 0x3F) | (type);}

/*! \brief Check for Static Address */
#define DM_RAND_ADDR_SA(addr, type)     (((type) == DM_ADDR_RANDOM) && \
                                         (DM_RAND_ADDR_GET((addr)) == DM_RAND_ADDR_STATIC))

/*! \brief Check for Resolvable Private Address */
#define DM_RAND_ADDR_RPA(addr, type)    (((type) == DM_ADDR_RANDOM) && \
                                         (DM_RAND_ADDR_GET((addr)) == DM_RAND_ADDR_RESOLV))
/**@}*/

/** \name GAP Privacy Mode
 * Privacy Mode of this device in regards to a peer device.
 */
/**@{*/
#define DM_PRIV_MODE_NETWORK        0x00  /*!< Network privacy mode (default). */
#define DM_PRIV_MODE_DEVICE         0x01  /*!< Device privacy mode. */
/**@}*/

/** \name DM Internal State
 * Connection busy or idle state
 */
/**@{*/
#define DM_CONN_IDLE                0     /*!< Connection is idle. */
#define DM_CONN_BUSY                1     /*!< Connection is busy. */
/**@}*/

/** \name DM Internal State Flags
 * Connection busy/idle state bitmask.
 */
/**@{*/
#define DM_IDLE_SMP_PAIR            0x0001  /*!< SMP pairing in progress */
#define DM_IDLE_DM_ENC              0x0002  /*!< DM Encryption setup in progress */
#define DM_IDLE_ATTS_DISC           0x0004  /*!< ATTS service discovery in progress */
#define DM_IDLE_APP_DISC            0x0008  /*!< App framework service discovery in progress */
#define DM_IDLE_USER_1              0x0010  /*!< For use by user application */
#define DM_IDLE_USER_2              0x0020  /*!< For use by user application */
#define DM_IDLE_USER_3              0x0040  /*!< For use by user application */
#define DM_IDLE_USER_4              0x0080  /*!< For use by user application */
/**@}*/

/** \name GAP Filter Policy Modes
 * Filter policy modes.
 */
/**@{*/
#define DM_FILT_POLICY_MODE_ADV     0     /*!< Advertising filter policy mode */
#define DM_FILT_POLICY_MODE_SCAN    1     /*!< Scanning filter policy mode */
#define DM_FILT_POLICY_MODE_INIT    2     /*!< Initiator filter policy mode */
#define DM_FILT_POLICY_MODE_SYNC    3     /*!< Synchronization filter policy mode */
/**@}*/

/** \name DM Proprietary Error Codes
 * Internal error codes not sent in any PDU.
 */
/**@{*/
#define DM_ERR_SMP_RX_PDU_LEN_EXCEEDED    0x01  /*!< LESC key length exceeded maximum RX PDU length */
#define DM_ERR_ATT_RX_PDU_LEN_EXCEEDED    0x02  /*!< Configured ATT MTU exceeded maximum RX PDU length */
#define DM_ERR_L2C_RX_PDU_LEN_EXCEEDED    0x03  /*!< Registered COC MPS exceeded maximum RX PDU length */
/**@}*/

/** \name DM Conn CTE states
 * Internal states of the DM conn CTE.
 */
/**@{*/
enum
{
  DM_CONN_CTE_STATE_IDLE,                    /*!< Idle */
  DM_CONN_CTE_STATE_INITIATING,              /*!< Initiating CTE request */
  DM_CONN_CTE_STATE_RESPONDING,              /*!< Responding to CTE request */
  DM_CONN_CTE_STATE_SAMPLING,                /*!< Sampling received CTE */
  DM_CONN_CTE_STATE_STARTING,                /*!< Starting CTE request, CTE response or sampling received CTE */
  DM_CONN_CTE_STATE_STOPPING,                /*!< Stopping CTE request, CTE response or sampling received CTE */
};
/**@}*/

/** \name DM Legacy Advertising Handle
 * Default handle for legacy advertising when using legacy HCI interface.  In this case only one advertising
 * set is allowed so all activity uses the same handle.
 */
/**@{*/
/*! \brief Default Advertising handle for legacy advertising */
#define DM_ADV_HANDLE_DEFAULT       0
/**@}*/

/** \name DM ISO data path directions
* Number of ISO data path directions
*/
/**@{*/
#define DM_ISO_NUM_DIR              2
/**@}*/

/** \name DM Callback Events
 * Events handled by the DM state machine.
 */
/**@{*/
#define DM_CBACK_START              0x20  /*!< DM callback event starting value */

/*! \brief DM callback events */
enum
{
  DM_RESET_CMPL_IND = DM_CBACK_START,     /*!< Reset complete */
  DM_ADV_START_IND,                       /*!< Advertising started */
  DM_ADV_STOP_IND,                        /*!< Advertising stopped */
  DM_ADV_NEW_ADDR_IND,                    /*!< New resolvable address has been generated */
  DM_SCAN_START_IND,                      /*!< Scanning started */
  DM_SCAN_STOP_IND,                       /*!< Scanning stopped */
  DM_SCAN_REPORT_IND,                     /*!< Scan data received from peer device */
  DM_CONN_OPEN_IND,                       /*!< Connection opened */
  DM_CONN_CLOSE_IND,                      /*!< Connection closed */
  DM_CONN_UPDATE_IND,                     /*!< Connection update complete */
  DM_SEC_PAIR_CMPL_IND,                   /*!< Pairing completed successfully */
  DM_SEC_PAIR_FAIL_IND,                   /*!< Pairing failed or other security failure */
  DM_SEC_ENCRYPT_IND,                     /*!< Connection encrypted */
  DM_SEC_ENCRYPT_FAIL_IND,                /*!< Encryption failed */
  DM_SEC_AUTH_REQ_IND,                    /*!< PIN or OOB data requested for pairing */
  DM_SEC_KEY_IND,                         /*!< Security key indication */
  DM_SEC_LTK_REQ_IND,                     /*!< LTK requested for encyption */
  DM_SEC_PAIR_IND,                        /*!< Incoming pairing request from master */
  DM_SEC_SLAVE_REQ_IND,                   /*!< Incoming security request from slave */
  DM_SEC_CALC_OOB_IND,                    /*!< Result of OOB Confirm Calculation Generation */
  DM_SEC_ECC_KEY_IND,                     /*!< Result of ECC Key Generation */
  DM_SEC_COMPARE_IND,                     /*!< Result of Just Works/Numeric Comparison Compare Value Calculation */
  DM_SEC_KEYPRESS_IND,                    /*!< Keypress indication from peer in passkey security */
  DM_PRIV_RESOLVED_ADDR_IND,              /*!< Private address resolved */
  DM_PRIV_GENERATE_ADDR_IND,              /*!< Private resolvable address generated */
  DM_CONN_READ_RSSI_IND,                  /*!< Connection RSSI read */
  DM_PRIV_ADD_DEV_TO_RES_LIST_IND,        /*!< Device added to resolving list */
  DM_PRIV_REM_DEV_FROM_RES_LIST_IND,      /*!< Device removed from resolving list */
  DM_PRIV_CLEAR_RES_LIST_IND,             /*!< Resolving list cleared */
  DM_PRIV_READ_PEER_RES_ADDR_IND,         /*!< Peer resolving address read */
  DM_PRIV_READ_LOCAL_RES_ADDR_IND,        /*!< Local resolving address read */
  DM_PRIV_SET_ADDR_RES_ENABLE_IND,        /*!< Address resolving enable set */
  DM_REM_CONN_PARAM_REQ_IND,              /*!< Remote connection parameter requested */
  DM_CONN_DATA_LEN_CHANGE_IND,            /*!< Data length changed */
  DM_CONN_WRITE_AUTH_TO_IND,              /*!< Write authenticated payload complete */
  DM_CONN_AUTH_TO_EXPIRED_IND,            /*!< Authenticated payload timeout expired */
  DM_PHY_READ_IND,                        /*!< Read PHY */
  DM_PHY_SET_DEF_IND,                     /*!< Set default PHY */
  DM_PHY_UPDATE_IND,                      /*!< PHY update */
  DM_ADV_SET_START_IND,                   /*!< Advertising set(s) started */
  DM_ADV_SET_STOP_IND,                    /*!< Advertising set(s) stopped */
  DM_SCAN_REQ_RCVD_IND,                   /*!< Scan request received */
  DM_EXT_SCAN_START_IND,                  /*!< Extended scanning started */
  DM_EXT_SCAN_STOP_IND,                   /*!< Extended scanning stopped */
  DM_EXT_SCAN_REPORT_IND,                 /*!< Extended scan data received from peer device */
  DM_PER_ADV_SET_START_IND,               /*!< Periodic advertising set started */
  DM_PER_ADV_SET_STOP_IND,                /*!< Periodic advertising set stopped */
  DM_PER_ADV_SYNC_EST_IND,                /*!< Periodic advertising sync established */
  DM_PER_ADV_SYNC_EST_FAIL_IND,           /*!< Periodic advertising sync establishment failed */
  DM_PER_ADV_SYNC_LOST_IND,               /*!< Periodic advertising sync lost */
  DM_PER_ADV_SYNC_TRSF_EST_IND,           /*!< Periodic advertising sync transfer established */
  DM_PER_ADV_SYNC_TRSF_EST_FAIL_IND,      /*!< Periodic advertising sync transfer establishment failed */
  DM_PER_ADV_SYNC_TRSF_IND,               /*!< Periodic advertising sync info transferred */
  DM_PER_ADV_SET_INFO_TRSF_IND,           /*!< Periodic advertising set sync info transferred */
  DM_PER_ADV_REPORT_IND,                  /*!< Periodic advertising data received from peer device */
  DM_REMOTE_FEATURES_IND,                 /*!< Remote features from peer device */
  DM_READ_REMOTE_VER_INFO_IND,            /*!< Remote LL version information read */
  DM_CONN_IQ_REPORT_IND,                  /*!< IQ samples from CTE of received packet from peer device */
  DM_CTE_REQ_FAIL_IND,                    /*!< CTE request failed */
  DM_CONN_CTE_RX_SAMPLE_START_IND,        /*!< Sampling received CTE started */
  DM_CONN_CTE_RX_SAMPLE_STOP_IND,         /*!< Sampling received CTE stopped */
  DM_CONN_CTE_TX_CFG_IND,                 /*!< Connection CTE transmit parameters configured */
  DM_CONN_CTE_REQ_START_IND,              /*!< Initiating connection CTE request started */
  DM_CONN_CTE_REQ_STOP_IND,               /*!< Initiating connection CTE request stopped */
  DM_CONN_CTE_RSP_START_IND,              /*!< Responding to connection CTE request started */
  DM_CONN_CTE_RSP_STOP_IND,               /*!< Responding to connection CTE request stopped */
  DM_READ_ANTENNA_INFO_IND,               /*!< Antenna information read */
  DM_CIS_CIG_CONFIG_IND,                  /*!< CIS CIG configure complete */
  DM_CIS_CIG_REMOVE_IND,                  /*!< CIS CIG remove complete */
  DM_CIS_REQ_IND,                         /*!< CIS request */
  DM_CIS_OPEN_IND,                        /*!< CIS connection opened */
  DM_CIS_CLOSE_IND,                       /*!< CIS connection closed */
  DM_REQ_PEER_SCA_IND,                    /*!< Request peer SCA complete */
  DM_ISO_DATA_PATH_SETUP_IND,             /*!< ISO data path setup complete */
  DM_ISO_DATA_PATH_REMOVE_IND,            /*!< ISO data path remove complete */
  DM_DATA_PATH_CONFIG_IND,                /*!< Data path configure complete */
  DM_READ_LOCAL_SUP_CODECS_IND,           /*!< Local supported codecs read */
  DM_READ_LOCAL_SUP_CODEC_CAP_IND,        /*!< Local supported codec capabilities read */
  DM_READ_LOCAL_SUP_CTR_DLY_IND,          /*!< Local supported controller delay read */
  DM_BIG_START_IND,                       /*!< BIG started */
  DM_BIG_STOP_IND,                        /*!< BIG stopped */
  DM_BIG_SYNC_EST_IND,                    /*!< BIG sync established */
  DM_BIG_SYNC_EST_FAIL_IND,               /*!< BIG sync establishment failed */
  DM_BIG_SYNC_LOST_IND,                   /*!< BIG sync lost */
  DM_BIG_SYNC_STOP_IND,                   /*!< BIG sync stopped */
  DM_BIG_INFO_ADV_REPORT_IND,             /*!< BIG Info advertising data received from peer device */
  DM_SUBRATE_CHANGE_IND,                  /*!< Connection subrate changed */
  DM_L2C_CMD_REJ_IND,                     /*!< L2CAP Command Reject */
  DM_ERROR_IND,                           /*!< General error */
  DM_HW_ERROR_IND,                        /*!< Hardware error */
  DM_VENDOR_SPEC_IND,                     /*!< Vendor specific event */
  DM_VENDOR_SPEC_CMD_CMPL_IND             /*!< Vendor specific command complete event */
};

#define DM_CBACK_END                DM_VENDOR_SPEC_CMD_CMPL_IND  /*!< DM callback event ending value */
/**@}*/

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! \brief Connection identifier. */
typedef uint8_t dmConnId_t;

/*! \brief Synchronization identifier. */
typedef uint8_t dmSyncId_t;

/*! \brief Configuration structure. */
typedef struct
{
  uint8_t dummy;  /*!< Placeholder variable. */
} dmCfg_t;

/*! \brief LTK data type. */
typedef struct
{
  uint8_t                   key[SMP_KEY_LEN];     /*!< LTK */
  uint8_t                   rand[SMP_RAND8_LEN];  /*!< Rand */
  uint16_t                  ediv;                 /*!< EDIV */
} dmSecLtk_t;

/*! \brief IRK data type. */
typedef struct
{
  uint8_t                   key[SMP_KEY_LEN];   /*!< IRK */
  bdAddr_t                  bdAddr;             /*!< BD Address */
  uint8_t                   addrType;           /*!< Address Type */
} dmSecIrk_t;

/*! \brief CSRK data type. */
typedef struct
{
  uint8_t                   key[SMP_KEY_LEN];   /*!< CSRK */
} dmSecCsrk_t;

/*! \brief Union of key types. */
typedef union
{
  dmSecLtk_t                ltk;   /*!< LTK */
  dmSecIrk_t                irk;   /*!< IRK */
  dmSecCsrk_t               csrk;  /*!< CSRK */
} dmSecKey_t;

/*! \brief Broadcast_Code data type. */
typedef struct
{
  uint8_t                   code[HCI_BC_LEN]; /*!< Broadcast_Code */
} dmSecBcastCode_t;

/*! \brief Data type for \ref DM_SEC_PAIR_CMPL_IND. */
typedef struct
{
  wsfMsgHdr_t               hdr;          /*!< Header */
  uint8_t                   auth;         /*!< Authentication and bonding flags */
} dmSecPairCmplIndEvt_t;

/*! \brief Data type for \ref DM_SEC_ENCRYPT_IND. */
typedef struct
{
  wsfMsgHdr_t               hdr;          /*!< Header */
  bool_t                    usingLtk;     /*!< TRUE if connection encrypted with LTK */
} dmSecEncryptIndEvt_t;

/*! \brief Data type for \ref DM_SEC_AUTH_REQ_IND. */
typedef struct
{
  wsfMsgHdr_t               hdr;          /*!< Header */
  bool_t                    oob;          /*!< Out-of-band data requested */
  bool_t                    display;      /*!< TRUE if pin is to be displayed */
} dmSecAuthReqIndEvt_t;

/*! \brief Data type for \ref DM_SEC_PAIR_IND. */
typedef struct
{
  wsfMsgHdr_t               hdr;          /*!< Header */
  uint8_t                   auth;         /*!< Authentication and bonding flags */
  bool_t                    oob;          /*!< Out-of-band pairing data present or not present */
  uint8_t                   iKeyDist;     /*!< Initiator key distribution flags */
  uint8_t                   rKeyDist;     /*!< Responder key distribution flags */
} dmSecPairIndEvt_t;

/*! \brief Data type for \ref DM_SEC_SLAVE_REQ_IND. */
typedef struct
{
  wsfMsgHdr_t               hdr;          /*!< Header */
  uint8_t                   auth;         /*!< Authentication and bonding flags */
} dmSecSlaveIndEvt_t;

/*! \brief Data type for \ref DM_SEC_KEY_IND. */
typedef struct
{
  wsfMsgHdr_t               hdr;          /*!< Header */
  dmSecKey_t                keyData;      /*!< Key data */
  uint8_t                   type;         /*!< Key type */
  uint8_t                   secLevel;     /*!< Security level of pairing when key was exchanged */
  uint8_t                   encKeyLen;    /*!< Length of encryption key used when data was transferred */
} dmSecKeyIndEvt_t;

/*! \brief Data type for \ref DM_SEC_COMPARE_IND. */
typedef struct
{
  wsfMsgHdr_t               hdr;                       /*!< Header */
  uint8_t                   confirm[SMP_CONFIRM_LEN];  /*!< Confirm value */
} dmSecCnfIndEvt_t;

/*! \brief Data type for \ref DM_SEC_KEYPRESS_IND. */
typedef struct
{
  wsfMsgHdr_t               hdr;                       /*!< Header */
  uint8_t                   notificationType;          /*!< Type of keypress notification */
} dmSecKeypressIndEvt_t;

/*! \brief Data type for \ref DM_PRIV_GENERATE_ADDR_IND. */
typedef struct
{
  wsfMsgHdr_t               hdr;                       /*!< Header */
  bdAddr_t                  addr;                      /*!< Resolvable private address */
} dmPrivGenAddrIndEvt_t;

/*! \brief Data type for \ref DM_SEC_CALC_OOB_IND. */
typedef struct
{
  wsfMsgHdr_t               hdr;                       /*!< Header */
  uint8_t                   confirm[SMP_CONFIRM_LEN];  /*!< Local confirm value */
  uint8_t                   random[SMP_RAND_LEN];      /*!< Local random value */
} dmSecOobCalcIndEvt_t;

/*! \brief Data type for \ref DM_ADV_NEW_ADDR_IND. */
typedef struct
{
  wsfMsgHdr_t               hdr;                       /*!< Header */
  bdAddr_t                  addr;                      /*!< New resolvable private address */
  bool_t                    firstTime;                 /*!< TRUE when address is generated for the first time */
} dmAdvNewAddrIndEvt_t;

/*! \brief Data structure for \ref DM_ADV_SET_START_IND. */
typedef struct
{
  wsfMsgHdr_t               hdr;                       /*!< Header */
  uint8_t                   numSets;                   /*!< Number of advertising sets */
  uint8_t                   advHandle[DM_NUM_ADV_SETS];/*!< Advertising handle array */
} dmAdvSetStartEvt_t;

/*! \brief Data structure for \ref DM_PER_ADV_SET_START_IND. */
typedef struct
{
  wsfMsgHdr_t               hdr;                       /*!< Header */
  uint8_t                   advHandle;                 /*!< Advertising handle */
} dmPerAdvSetStartEvt_t;

/*! \brief Data structure for \ref DM_PER_ADV_SET_STOP_IND. */
typedef struct
{
  wsfMsgHdr_t               hdr;                       /*!< Header */
  uint8_t                   advHandle;                 /*!< Advertising handle */
} dmPerAdvSetStopEvt_t;

/*! \brief Data structure for \ref DM_ISO_DATA_PATH_SETUP_IND. */
typedef struct
{
  wsfMsgHdr_t         hdr;                  /*!< Event header. */
  uint8_t             status;               /*!< Status. */
  uint16_t            handle;               /*!< Connection handle of the CIS or BIS. */
  uint8_t             dpDir;                /*!< Data path direction being set up. */
} dmSetupIsoDataPathEvt_t;

/*! \brief Data structure for \ref DM_ISO_DATA_PATH_REMOVE_IND. */
typedef struct
{
  wsfMsgHdr_t         hdr;                  /*!< Event header. */
  uint8_t             status;               /*!< Status. */
  uint16_t            handle;               /*!< Connection handle of the CIS or BIS. */
  uint8_t             directionBits;        /*!< Data path directions being removed. */
} dmRemoveIsoDataPathEvt_t;

/*! \brief Data structure for \ref DM_L2C_CMD_REJ_IND. */
typedef struct
{
  wsfMsgHdr_t               hdr;                       /*!< Header */
  uint16_t                  reason;                    /*!< Rejection reason */
  uint16_t                  handle;                    /*!< Connection handle */
} dmL2cCmdRejEvt_t;

/*! \brief Union of DM callback event data types.
 *
 *  \note the following events use only the common \ref wsfMsgHdr_t header:
 *  \ref DM_RESET_CMPL_IND,
 *  \ref DM_ADV_START_IND,
 *  \ref DM_ADV_STOP_IND,
 *  \ref DM_SCAN_START_IND,
 *  \ref DM_SCAN_STOP_IND,
 *  \ref DM_SEC_PAIR_FAIL_IND,
 *  \ref DM_SEC_ENCRYPT_FAIL_IND,
 *  \ref DM_PRIV_RESOLVED_ADDR_IND,
 *  \ref DM_EXT_SCAN_START_IND,
 *  \ref DM_EXT_SCAN_STOP_IND,
 *  \ref DM_ERROR_IND
 */
typedef union
{
  wsfMsgHdr_t                         hdr;                  /*!< Common header */
  /* common header used by                                                           DM_RESET_CMPL_IND */
  /* common header used by                                                           DM_ADV_START_IND */
  /* common header used by                                                           DM_ADV_STOP_IND */
  dmAdvNewAddrIndEvt_t                advNewAddr;           /*!< handles \ref DM_ADV_NEW_ADDR_IND */
  /* common header used by                                                           DM_SCAN_START_IND */
  /* common header used by                                                           DM_SCAN_STOP_IND */
  hciLeAdvReportEvt_t                 scanReport;           /*!< handles \ref DM_SCAN_REPORT_IND */
  hciLeConnCmplEvt_t                  connOpen;             /*!< handles \ref DM_CONN_OPEN_IND */
  hciDisconnectCmplEvt_t              connClose;            /*!< handles \ref DM_CONN_CLOSE_IND */
  hciLeConnUpdateCmplEvt_t            connUpdate;           /*!< handles \ref DM_CONN_UPDATE_IND */
  dmSecPairCmplIndEvt_t               pairCmpl;             /*!< handles \ref DM_SEC_PAIR_CMPL_IND */
  /* common header used by                                                           DM_SEC_PAIR_FAIL_IND */
  dmSecEncryptIndEvt_t                encryptInd;           /*!< handles \ref DM_SEC_ENCRYPT_IND */
  /* common header used by                                                           DM_SEC_ENCRYPT_FAIL_IND */
  dmSecAuthReqIndEvt_t                authReq;              /*!< handles \ref DM_SEC_AUTH_REQ_IND */
  dmSecKeyIndEvt_t                    keyInd;               /*!< handles \ref DM_SEC_KEY_IND */
  hciLeLtkReqEvt_t                    ltkReqInd;            /*!< handles \ref DM_SEC_LTK_REQ_IND */
  dmSecPairIndEvt_t                   pairInd;              /*!< handles \ref DM_SEC_PAIR_IND */
  dmSecSlaveIndEvt_t                  slaveInd;             /*!< handles \ref DM_SEC_SLAVE_REQ_IND */
  dmSecOobCalcIndEvt_t                oobCalcInd;           /*!< handles \ref DM_SEC_CALC_OOB_IND */
  secEccMsg_t                         eccMsg;               /*!< handles \ref DM_SEC_ECC_KEY_IND */
  dmSecCnfIndEvt_t                    cnfInd;               /*!< handles \ref DM_SEC_COMPARE_IND */
  dmSecKeypressIndEvt_t               keypressInd;          /*!< handles \ref DM_SEC_KEYPRESS_IND */
  /*  common header used by                                                          DM_PRIV_RESOLVED_ADDR_IND */
  dmPrivGenAddrIndEvt_t               genAddr;              /*!< handles \ref DM_PRIV_GENERATE_ADDR_IND */
  hciReadRssiCmdCmplEvt_t             readRssi;             /*!< handles \ref DM_CONN_READ_RSSI_IND */
  hciLeAddDevToResListCmdCmplEvt_t    addDevToResList;      /*!< handles \ref DM_PRIV_ADD_DEV_TO_RES_LIST_IND */
  hciLeRemDevFromResListCmdCmplEvt_t  remDevFromResList;    /*!< handles \ref DM_PRIV_REM_DEV_FROM_RES_LIST_IND */
  hciLeClearResListCmdCmplEvt_t       clearResList;         /*!< handles \ref DM_PRIV_CLEAR_RES_LIST_IND */
  hciLeReadPeerResAddrCmdCmplEvt_t    readPeerResAddr;      /*!< handles \ref DM_PRIV_READ_PEER_RES_ADDR_IND */
  hciLeReadLocalResAddrCmdCmplEvt_t   readLocalResAddr;     /*!< handles \ref DM_PRIV_READ_LOCAL_RES_ADDR_IND */
  hciLeSetAddrResEnableCmdCmplEvt_t   setAddrResEnable;     /*!< handles \ref DM_PRIV_SET_ADDR_RES_ENABLE_IND */
  hciLeRemConnParamReqEvt_t           remConnParamReq;      /*!< handles \ref DM_REM_CONN_PARAM_REQ_IND */
  hciLeDataLenChangeEvt_t             dataLenChange;        /*!< handles \ref DM_CONN_DATA_LEN_CHANGE_IND */
  hciWriteAuthPayloadToCmdCmplEvt_t   writeAuthTo;          /*!< handles \ref DM_CONN_WRITE_AUTH_TO_IND */
  hciAuthPayloadToExpiredEvt_t        authToExpired;        /*!< handles \ref DM_CONN_AUTH_TO_EXPIRED_IND */
  hciLeReadPhyCmdCmplEvt_t            readPhy;              /*!< handles \ref DM_PHY_READ_IND */
  hciLeSetDefPhyCmdCmplEvt_t          setDefPhy;            /*!< handles \ref DM_PHY_SET_DEF_IND */
  hciLePhyUpdateEvt_t                 phyUpdate;            /*!< handles \ref DM_PHY_UPDATE_IND */
  dmAdvSetStartEvt_t                  advSetStart;          /*!< handles \ref DM_ADV_SET_START_IND */
  hciLeAdvSetTermEvt_t                advSetStop;           /*!< handles \ref DM_ADV_SET_STOP_IND */
  hciLeScanReqRcvdEvt_t               scanReqRcvd;          /*!< handles \ref DM_SCAN_REQ_RCVD_IND */
  /* common header used by                                                           DM_EXT_SCAN_START_IND */
  /* common header used by                                                           DM_EXT_SCAN_STOP_IND */
  hciLeExtAdvReportEvt_t              extScanReport;         /*!< handles \ref DM_EXT_SCAN_REPORT_IND */
  dmPerAdvSetStartEvt_t               perAdvSetStart;        /*!< handles \ref DM_PER_ADV_SET_START_IND */
  dmPerAdvSetStopEvt_t                perAdvSetStop;         /*!< handles \ref DM_PER_ADV_SET_STOP_IND */
  hciLePerAdvSyncEstEvt_t             perAdvSyncEst;         /*!< handles \ref DM_PER_ADV_SYNC_EST_IND */
  hciLePerAdvSyncEstEvt_t             perAdvSyncEstFail;     /*!< handles \ref DM_PER_ADV_SYNC_EST_FAIL_IND */
  hciLePerAdvSyncLostEvt_t            perAdvSyncLost;        /*!< handles \ref DM_PER_ADV_SYNC_LOST_IND */
  HciLePerAdvSyncTrsfRcvdEvt_t        perAdvSyncTrsfEst;     /*!< handles \ref DM_PER_ADV_SYNC_TRSF_EST_IND */
  HciLePerAdvSyncTrsfRcvdEvt_t        perAdvSyncTrsEstFail;  /*!< handles \ref DM_PER_ADV_SYNC_TRSF_EST_FAIL_IND */
  hciLePerAdvSyncTrsfCmdCmplEvt_t     perAdvSyncTrsf;        /*!< handles \ref DM_PER_ADV_SYNC_TRSF_IND */
  hciLePerAdvSetInfoTrsfCmdCmplEvt_t  perAdvSetInfoTrsf;     /*!< handles \ref DM_PER_ADV_SET_INFO_TRSF_IND */
  hciLePerAdvReportEvt_t              perAdvReport;          /*!< handles \ref DM_PER_ADV_REPORT_IND */
  hciLeReadRemoteFeatCmplEvt_t        readRemoteFeat;        /*!< handles \ref DM_REMOTE_FEATURES_IND */
  hciReadRemoteVerInfoCmplEvt_t       readRemVerInfo;        /*!< handles \ref DM_READ_REMOTE_VER_INFO_IND */
  hciLeConnIQReportEvt_t              connIQReport;          /*!< handles \ref DM_CONN_IQ_REPORT_IND */
  hciLeCteReqFailedEvt_t              cteReqFail;            /*!< handles \ref DM_CTE_REQ_FAIL_IND */
  hciLeSetConnCteRxParamsCmdCmplEvt_t connCteRxSampleStart;  /*!< handles \ref DM_CONN_CTE_RX_SAMPLE_START_IND */
  hciLeSetConnCteRxParamsCmdCmplEvt_t connCteRxSampleStop;   /*!< handles \ref DM_CONN_CTE_RX_SAMPLE_STOP_IND */
  hciLeSetConnCteTxParamsCmdCmplEvt_t connCteTxCfg;          /*!< handles \ref DM_CONN_CTE_TX_CFG_IND */
  hciLeConnCteReqEnableCmdCmplEvt_t   connCteReqStart;       /*!< handles \ref DM_CONN_CTE_REQ_START_IND */
  hciLeConnCteReqEnableCmdCmplEvt_t   connCteReqStop;        /*!< handles \ref DM_CONN_CTE_REQ_STOP_IND */
  hciLeConnCteRspEnableCmdCmplEvt_t   connCteRspStart;       /*!< handles \ref DM_CONN_CTE_RSP_START_IND */
  hciLeConnCteRspEnableCmdCmplEvt_t   connCteRspStop;        /*!< handles \ref DM_CONN_CTE_RSP_STOP_IND */
  hciLeReadAntennaInfoCmdCmplEvt_t    readAntennaInfo;       /*!< handles \ref DM_READ_ANTENNA_INFO_IND */
  hciLeSetCigParamsCmdCmplEvt_t       cisCigConfig;          /*!< handles \ref DM_CIS_CIG_CONFIG_IND */
  hciLeRemoveCigCmdCmplEvt_t          cisCigRemove;          /*!< handles \ref DM_CIS_CIG_REMOVE_IND */
  HciLeCisReqEvt_t                    cisReq;                /*!< handles \ref DM_CIS_REQ_IND */
  HciLeCisEstEvt_t                    cisOpen;               /*!< handles \ref DM_CIS_OPEN_IND */
  hciDisconnectCmplEvt_t              cisClose;              /*!< handles \ref DM_CIS_CLOSE_IND */
  HciLeReqPeerScaCmplEvt_t_t          reqPeerSca;            /*!< handles \ref DM_REQ_PEER_SCA_IND */
  dmSetupIsoDataPathEvt_t             isoDataPathSetup;      /*!< handles \ref DM_ISO_DATA_PATH_SETUP_IND */
  dmRemoveIsoDataPathEvt_t            isoDataPathRemove;     /*!< handles \ref DM_ISO_DATA_PATH_REMOVE_IND */
  hciConfigDataPathCmdCmplEvt_t       dataPathConfig;        /*!< handles \ref DM_DATA_PATH_CONFIG_IND */
  hciReadLocalSupCodecsCmdCmplEvt_t   readLocalSupCodecs;    /*!< handles \ref DM_READ_LOCAL_SUP_CODECS_IND */
  hciReadLocalSupCodecCapCmdCmplEvt_t readLocalSupCodecCap;  /*!< handles \ref DM_READ_LOCAL_SUP_CODEC_CAP_IND */
  hciReadLocalSupCtrDlyCmdCmplEvt_t   readLocalSupCtrDly;    /*!< handles \ref DM_READ_LOCAL_SUP_CTR_DLY_IND */
  HciLeCreateBigCmplEvt_t             bigStart;              /*!< handles \ref DM_BIG_START_IND */
  HciLeTerminateBigCmplEvt_t          bigStop;               /*!< handles \ref DM_BIG_STOP_IND */
  HciLeBigSyncEstEvt_t                bigSyncEst;            /*!< handles \ref DM_BIG_SYNC_EST_IND */
  HciLeBigSyncEstEvt_t                bigSyncEstFail;        /*!< handles \ref DM_BIG_SYNC_EST_FAIL_IND */
  HciLeBigSyncLostEvt_t               bigSyncLost;           /*!< handles \ref DM_BIG_SYNC_LOST_IND */
  HciLeBigTermSyncCmplEvt_t           bigSyncStop;           /*!< handles \ref DM_BIG_SYNC_STOP_IND */
  HciLeBigInfoAdvRptEvt_t             bigInfoAdvRpt;         /*!< handles \ref DM_BIG_INFO_ADV_REPORT_IND */
  hciLeSubrateChangeEvt_t             subrateChange;         /*!< handles \ref DM_SUBRATE_CHANGE_IND */
  dmL2cCmdRejEvt_t                    l2cCmdRej;             /*!< handles \ref DM_L2C_CMD_REJ_IND */
  /* common header used by                                                            DM_ERROR_IND */
  hciHwErrorEvt_t                     hwError;               /*!< handles \ref DM_HW_ERROR_IND */
  hciVendorSpecEvt_t                  vendorSpec;            /*!< handles \ref DM_VENDOR_SPEC_IND */
  hciVendorSpecCmdCmplEvt_t           vendorSpecCmdCmpl;     /*!< handles \ref DM_VENDOR_SPEC_CMD_CMPL_IND */
} dmEvt_t;

/*! \brief Data type for DmSecSetOob(). */
typedef struct
{
  uint8_t localRandom[SMP_RAND_LEN];      /*!< Random value of the local device */
  uint8_t localConfirm[SMP_CONFIRM_LEN];  /*!< Confirm value of the local device */
  uint8_t peerRandom[SMP_RAND_LEN];       /*!< Random value of the peer device */
  uint8_t peerConfirm[SMP_CONFIRM_LEN];   /*!< Confirm value of the peer device */
} dmSecLescOobCfg_t;

/*! \brief DM callback type. */
typedef void (*dmCback_t)(dmEvt_t *pDmEvt);

/*! \brief DM Setup ISO data path callback type. */
typedef void (*dmIsoDataPathSetupCback_t)(HciIsoSetupDataPath_t *pDataPathParam);

/*! \brief DM Remove ISO data path callback type. */
typedef void (*dmIsoDataPathRemoveCback_t)(uint16_t handle, uint8_t directionBits);

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/** \name DM App Callback Registration
 *
 */
/**@{*/
/*************************************************************************************************/
/*!
 *  \brief  Register a callback with DM for scan and advertising events.
 *
 *  \param  cback  Client callback function.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmRegister(dmCback_t cback);

/**@}*/

/** \name DM Advertising Functions
 * Functions used to control Legacy and Extended Advertising.
 */
/**@{*/

/*************************************************************************************************/
/*!
 *  \brief  Find an advertising data element in the given advertising or scan response data.
 *
 *  \param  adType  Advertising data element type to find.
 *  \param  dataLen Data length.
 *  \param  pData   Pointer to advertising or scan response data.
 *
 *  \return Pointer to the advertising data element byte array or NULL if not found.
 */
/*************************************************************************************************/
uint8_t *DmFindAdType(uint8_t adType, uint16_t dataLen, uint8_t *pData);

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM legacy advertising.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAdvInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM extended advertising.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmExtAdvInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Whether DM advertising is in legacy mode.
 *
 *  \return TRUE if DM advertising is in legacy mode. FALSE, otherwise.
 */
/*************************************************************************************************/
bool_t DmAdvModeLeg(void);

/*************************************************************************************************/
/*!
 *  \brief  Whether DM advertising is in extended mode.
 *
 *  \return TRUE if DM advertising is in extended mode. FALSE, otherwise.
 */
/*************************************************************************************************/
bool_t DmAdvModeExt(void);

/*************************************************************************************************/
/*!
 *  \brief  Set the advertising parameters using the given advertising type, and peer address.
 *
 *  \param  advHandle     Advertising handle.
 *  \param  advType       Advertising type.
 *  \param  peerAddrType  Peer address type.
 *  \param  pPeerAddr     Peer address.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAdvConfig(uint8_t advHandle, uint8_t advType, uint8_t peerAddrType, uint8_t *pPeerAddr);

/*************************************************************************************************/
/*!
 *  \brief  Set the advertising or scan response data to the given data.
 *
 *  \param  advHandle     Advertising handle.
 *  \param  op            Data operation.
 *  \param  location      Data location.
 *  \param  len           Length of the data.  Maximum length is 236 bytes.
 *  \param  pData         Pointer to the data.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAdvSetData(uint8_t advHandle, uint8_t op, uint8_t location, uint8_t len, uint8_t *pData);

/*************************************************************************************************/
/*!
 *  \brief  Start advertising using the given advertising set and duration.
 *
 *  \param  numSets       Number of advertising sets to enable.
 *  \param  pAdvHandles   Advertising handles array.
 *  \param  pDuration     Advertising duration (in milliseconds) array.
 *  \param  pMaxEaEvents  Maximum number of extended advertising events array.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAdvStart(uint8_t numSets, uint8_t *pAdvHandles, uint16_t *pDuration, uint8_t *pMaxEaEvents);

/*************************************************************************************************/
/*!
 *  \brief  Stop advertising for the given advertising set.  If the number of sets is set to 0
 *          then all advertising sets are disabled.
 *
 *  \param  numSets       Number of advertising sets to disable.
 *  \param  pAdvHandles   Advertising handles array.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAdvStop(uint8_t numSets, uint8_t *pAdvHandles);

/*************************************************************************************************/
/*!
 *  \brief  Remove an advertising set.
 *
 *  \param  advHandle     Advertising handle.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAdvRemoveAdvSet(uint8_t advHandle);

/*************************************************************************************************/
/*!
 *  \brief  Clear advertising sets.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAdvClearAdvSets(void);

/*************************************************************************************************/
/*!
 *  \brief  Set the random device address for a given advertising set.
 *
 *  \param  advHandle     Advertising handle.
 *  \param  pAddr         Random device address.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAdvSetRandAddr(uint8_t advHandle, const uint8_t *pAddr);

/*************************************************************************************************/
/*!
 *  \brief  Set the minimum and maximum advertising intervals.
 *
 *  \param  advHandle     Advertising handle.
 *  \param  intervalMin   Minimum advertising interval.
 *  \param  intervalMax   Maximum advertising interval.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAdvSetInterval(uint8_t advHandle, uint16_t intervalMin, uint16_t intervalMax);

/*************************************************************************************************/
/*!
 *  \brief  Include or exclude certain channels from the advertising channel map.
 *
 *  \param  advHandle     Advertising handle.
 *  \param  channelMap    Advertising channel map.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAdvSetChannelMap(uint8_t advHandle, uint8_t channelMap);

/*************************************************************************************************/
/*!
 *  \brief  Set the local address type used while advertising.  This function can be used to
 *          configure advertising to use a random address.
 *
 *  \param  addrType      Address type.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAdvSetAddrType(uint8_t addrType);

/*************************************************************************************************/
/*!
 *  \brief  Set the value of an advertising data element in the given advertising or
 *          scan response data.  If the element already exists in the data then it is replaced
 *          with the new value.  If the element does not exist in the data it is appended
 *          to it, space permitting.
 *
 *  \param  adType        Advertising data element type.
 *  \param  len           Length of the value.  Maximum length is 29 bytes.
 *  \param  pValue        Pointer to the value.
 *  \param  pAdvDataLen   Advertising or scan response data length.  The new length is returned
 *                        in this parameter.
 *  \param  pAdvData      Pointer to advertising or scan response data.
 *  \param  advDataBufLen Length of the advertising or scan response data buffer maintained by
 *                        Application.
 *
 *  \return TRUE if the element was successfully added to the data, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t DmAdvSetAdValue(uint8_t adType, uint8_t len, uint8_t *pValue, uint16_t *pAdvDataLen,
                       uint8_t *pAdvData, uint16_t advDataBufLen);

/*************************************************************************************************/
/*!
 *  \brief  Set the device name in the given advertising or scan response data.  If the
 *          name can only fit in the data if it is shortened, the name is shortened
 *          and the AD type is changed to DM_ADV_TYPE_SHORT_NAME.
 *
 *  \param  len           Length of the name.  Maximum length is 29 bytes.
 *  \param  pValue        Pointer to the name in UTF-8 format.
 *  \param  pAdvDataLen   Advertising or scan response data length.  The new length is returned
 *                        in this parameter.
 *  \param  pAdvData      Pointer to advertising or scan response data.
 *  \param  advDataBufLen Length of the advertising or scan response data buffer maintained by
 *                        Application.
 *
 *  \return TRUE if the element was successfully added to the data, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t DmAdvSetName(uint8_t len, uint8_t *pValue, uint16_t *pAdvDataLen, uint8_t *pAdvData,
                    uint16_t advDataBufLen);

/*************************************************************************************************/
/*!
 *  \brief  Initialize device privacy module.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmDevPrivInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Start using a private resolvable address.
 *
 *  \param  changeInterval  Interval between automatic address changes, in seconds.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmDevPrivStart(uint16_t changeInterval);

/*************************************************************************************************/
/*!
 *  \brief  Stop using a private resolvable address.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmDevPrivStop(void);

/*************************************************************************************************/
/*!
 *  \brief  Set whether or not to use legacy advertising PDUs with extended advertising.
 *
 *  \param  advHandle    Advertising handle.
 *  \param  useLegacyPdu Whether to use legacy advertising PDUs (default value is TRUE).
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAdvUseLegacyPdu(uint8_t advHandle, bool_t useLegacyPdu);

/*************************************************************************************************/
/*!
 *  \brief  Set whether or not to omit advertiser's address from all PDUs (anonymous advertising).
 *
 *  \param  advHandle    Advertising handle.
 *  \param  omitAdvAddr  Whether to omit advertiser's address from all PDUs (default value is FALSE).
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAdvOmitAdvAddr(uint8_t advHandle, bool_t omitAdvAddr);

/*************************************************************************************************/
/*!
 *  \brief  Set whether or not to include TxPower in extended header of advertising PDU.
 *
 *  \param  advHandle    Advertising handle.
 *  \param  incTxPwr     Whether to include TxPower in extended header of advertising PDU (default
 *                       value is FALSE).
 *  \param  advTxPwr     Advertising tx power (127 = no preference).
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAdvIncTxPwr(uint8_t advHandle, bool_t incTxPwr, int8_t advTxPwr);

/*************************************************************************************************/
/*!
 *  \brief  Set extended advertising PHY parameters.
 *
 *  \param  advHandle     Advertising handle.
 *  \param  priAdvPhy     Primary advertising Phy.
 *  \param  secAdvMaxSkip Maximum advertising events Controller can skip before sending AUX_ADV_IND
 *                        on secondary advertising channel (0 = AUX_ADV_IND will be sent prior to
 *                        next advertising event).
 *  \param  secAdvPhy     Secondary advertising Phy.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAdvSetPhyParam(uint8_t advHandle, uint8_t priAdvPhy, uint8_t secAdvMaxSkip, uint8_t secAdvPhy);

/*************************************************************************************************/
/*!
 *  \brief  Set scan request notification enable.
 *
 *  \param  advHandle       Advertising handle.
 *  \param  scanReqNotifEna Scan request notification enable.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAdvScanReqNotifEnable(uint8_t advHandle, bool_t scanReqNotifEna);

/*************************************************************************************************/
/*!
 *  \brief  Set fragment preference for advertising data.
 *
 *  \param  advHandle     Advertising handle.
 *  \param  fragPref      Fragment preference.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAdvSetFragPref(uint8_t advHandle, uint8_t fragPref);

/*************************************************************************************************/
/*!
 *  \brief  Set advertising SID for the given advertising handle.
 *
 *  \param  advHandle     Advertising handle.
 *  \param  advSid        Advertsing SID.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAdvSetSid(uint8_t advHandle, uint8_t advSid);

/*************************************************************************************************/
/*!
 *  \brief  Set the advertising parameters for periodic advertising.
 *
 *  \param  advHandle     Advertising handle.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPerAdvConfig(uint8_t advHandle);

/*************************************************************************************************/
/*!
 *  \brief  Set the advertising data to the given data for periodic advertising.
 *
 *  \param  advHandle     Advertising handle.
 *  \param  op            Data operation.
 *  \param  len           Length of the data.  Maximum length is 236 bytes.
 *  \param  pData         Pointer to the data.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPerAdvSetData(uint8_t advHandle, uint8_t op, uint8_t len, uint8_t *pData);

/*************************************************************************************************/
/*!
 *  \brief  Start periodic advertising for the advertising set specified by the advertising handle.
 *
 *  \param  advHandle     Advertising handle.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPerAdvStart(uint8_t advHandle);

/*************************************************************************************************/
/*!
 *  \brief  Stop periodic advertising for the advertising set specified by the advertising handle.
 *
 *  \param  advHandle     Advertising handle.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPerAdvStop(uint8_t advHandle);


/*************************************************************************************************/
/*!
 *  \brief  Set the minimum and maximum advertising intervals for periodic advertising.
 *
 *  \param  advHandle     Advertising handle.
 *  \param  intervalMin   Minimum advertising interval.
 *  \param  intervalMax   Maximum advertising interval.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPerAdvSetInterval(uint8_t advHandle, uint16_t intervalMin, uint16_t intervalMax);

/*************************************************************************************************/
/*!
 *  \brief  Set whether or not to include TxPower in extended header of advertising PDU for
 *          periodic advertising.
 *
 *  \param  advHandle    Advertising handle.
 *  \param  incTxPwr     Whether to include TxPower in extended header of advertising PDU (default
 *                       value is FALSE).
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPerAdvIncTxPwr(uint8_t advHandle, bool_t incTxPwr);

/*************************************************************************************************/
/*!
 *  \brief  Set whether or not to include AdvDataInfo (ADI) in periodic advertising.
 *
 *  \param  advHandle    Advertising handle.
 *  \param  incAdi       Whether to include ADI in periodic advertising (default value is FALSE).
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPerAdvIncAdi(uint8_t advHandle, bool_t incAdi);

/*************************************************************************************************/
/*!
 *  \brief  Get status of periodic advertising handle.
 *
 *  \param  advHandle    Advertising handle.
 *
 *  \return TRUE if periodic advertising is running on that handle. FALSE, otherwise.
*/
/*************************************************************************************************/
bool_t DmPerAdvEnabled(uint8_t advHandle);

/*************************************************************************************************/
/*!
 *  \brief  Get the maximum advertising data length supported by Controller for a given advertising
 *          type.
 *
 *  \param  advType      Advertising type.
 *  \param  useLegacyPdu Whether to use legacy advertising PDUs with extended advertising.
 *
 *  \return Maximum advertising data length.
 */
/*************************************************************************************************/
uint16_t DmExtMaxAdvDataLen(uint8_t advType, bool_t useLegacyPdu);

/**@}*/

/** \name DM Privacy Functions
 * Functions for controlling Privacy.
 */
/**@{*/

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM privacy module.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPrivInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Resolve a private resolvable address.  When complete the client's callback function
 *          is called with a DM_PRIV_RESOLVED_ADDR_IND event.  The client must wait to receive
 *          this event before executing this function again.
 *
 *  \param  pAddr     Peer device address.
 *  \param  pIrk      The peer's identity resolving key.
 *  \param  param     Client-defined parameter returned with callback event.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPrivResolveAddr(uint8_t *pAddr, uint8_t *pIrk, uint16_t param);

/*************************************************************************************************/
/*!
 *  \brief  Add device to resolving list.  When complete the client's callback function
 *          is called with a DM_PRIV_ADD_DEV_TO_RES_LIST_IND event.  The client must wait
 *          to receive this event before executing this function again.
 *
 *  \param  addrType      Peer identity address type.
 *  \param  pIdentityAddr Peer identity address.
 *  \param  pPeerIrk      The peer's identity resolving key.
 *  \param  pLocalIrk     The local identity resolving key.
 *  \param  enableLlPriv  Set to TRUE to enable address resolution in LL.
 *  \param  param         client-defined parameter returned with callback event.
 *
 *  \return None.
 *
 *  \Note   This command cannot be used when address resolution is enabled in the Controller and:
 *          - Advertising (other than periodic advertising) is enabled,
 *          - Scanning is enabled, or
 *          - (Extended) Create connection or Create Sync command is outstanding.
 *
 *  \Note   If the local or peer IRK associated with the peer Identity Address is all zeros then
 *          the Controller will use or accept the local or peer Identity Address respectively.
 *
 *  \Note   Parameter 'enableLlPriv' should be set to TRUE when the last device is being added
 *          to resolving list to enable address resolution in the Controller.
 */
/*************************************************************************************************/
void DmPrivAddDevToResList(uint8_t addrType, const uint8_t *pIdentityAddr, uint8_t *pPeerIrk,
                           uint8_t *pLocalIrk, bool_t enableLlPriv, uint16_t param);

/*************************************************************************************************/
/*!
 *  \brief  Remove device from resolving list.  When complete the client's callback function
 *          is called with a DM_PRIV_REM_DEV_FROM_RES_LIST_IND event.  The client must wait to
 *          receive this event before executing this function again.
 *
 *  \param  addrType      Peer identity address type.
 *  \param  pIdentityAddr Peer identity address.
 *  \param  param         client-defined parameter returned with callback event.
 *
 *  \return None.
 *
 *  \Note   This command cannot be used when address resolution is enabled in the Controller and:
 *          - Advertising (other than periodic advertising) is enabled,
 *          - Scanning is enabled, or
 *          - (Extended) Create connection or Create Sync command is outstanding.
 */
/*************************************************************************************************/
void DmPrivRemDevFromResList(uint8_t addrType, const uint8_t *pIdentityAddr, uint16_t param);

/*************************************************************************************************/
/*!
 *  \brief  Clear resolving list.  When complete the client's callback function is called with a
 *          DM_PRIV_CLEAR_RES_LIST_IND event.  The client must wait to receive this event before
 *          executing this function again.
 *
 *  \return None.
 *
 *  \Note   This command cannot be used when address resolution is enabled in the Controller and:
 *          - Advertising (other than periodic advertising) is enabled,
 *          - Scanning is enabled, or
 *          - (Extended) Create connection or Create Sync command is outstanding.
 *
 *  \Note   Address resolution in Controller will be disabled when resolving list's cleared
 *          successfully.
 */
/*************************************************************************************************/
void DmPrivClearResList(void);

/*************************************************************************************************/
/*!
 *  \brief  HCI read peer resolvable address command.  When complete the client's callback
 *          function is called with a DM_PRIV_READ_PEER_RES_ADDR_IND event.  The client must
 *          wait to receive this event before executing this function again.
 *
 *  \param  addrType        Peer identity address type.
 *  \param  pIdentityAddr   Peer identity address.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPrivReadPeerResolvableAddr(uint8_t addrType, const uint8_t *pIdentityAddr);

/*************************************************************************************************/
/*!
 *  \brief  Read local resolvable address command.  When complete the client's callback
 *          function is called with a DM_PRIV_READ_LOCAL_RES_ADDR_IND event.  The client must
 *          wait to receive this event before executing this function again.
 *
 *  \param  addrType        Peer identity address type.
 *  \param  pIdentityAddr   Peer identity address.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPrivReadLocalResolvableAddr(uint8_t addrType, const uint8_t *pIdentityAddr);

/*************************************************************************************************/
/*!
 *  \brief  Enable or disable address resolution in LL.  When complete the client's callback
 *          function is called with a DM_PRIV_SET_ADDR_RES_ENABLE_IND event.  The client must
 *          wait to receive this event before executing this function again.
 *
 *  \param  enable   Set to TRUE to enable address resolution or FALSE to disable it.
 *
 *  \return None.
 *
 *  \Note   This command can be used at any time except when:
 *          - Advertising (other than periodic advertising) is enabled,
 *          - Scanning is enabled, or
 *          - (Extended) Create connection or Create Sync command is outstanding.
 */
/*************************************************************************************************/
void DmPrivSetAddrResEnable(bool_t enable);

/*************************************************************************************************/
/*!
 *  \brief  Set resolvable private address timeout command.
 *
 *  \param  rpaTimeout    Timeout measured in seconds.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPrivSetResolvablePrivateAddrTimeout(uint16_t rpaTimeout);

/*************************************************************************************************/
/*!
 *  \brief  Set privacy mode for a given entry in the resolving list.
 *
 *  \param  addrType      Peer identity address type.
 *  \param  pIdentityAddr Peer identity address.
 *  \param  mode          Privacy mode (by default, network privacy mode is used).
 *
 *  \return None.
 *
 *  \Note   This command can be used at any time except when:
 *          - Advertising (other than periodic advertising) is enabled,
 *          - Scanning is enabled, or
 *          - (Extended) Create connection or Create Sync command is outstanding.
 */
/*************************************************************************************************/
void DmPrivSetPrivacyMode(uint8_t addrType, const uint8_t *pIdentityAddr, uint8_t mode);

/*************************************************************************************************/
/*!
 *  \brief  Generate a Resolvable Private Address (RPA).
 *
 *  \param  pIrk      The identity resolving key.
 *  \param  param     Client-defined parameter returned with callback event.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPrivGenerateAddr(uint8_t *pIrk, uint16_t param);

/*************************************************************************************************/
/*!
 *  \brief  Whether LL Privacy is enabled.
 *
 *  \return TRUE if LL Privacy is enabled. FALSE, otherwise.
 */
/*************************************************************************************************/
bool_t DmLlPrivEnabled(void);

/**@}*/

/** \name DM Scanner Functions
 * Functions for controlling Legacy and Extended Scanner behavior.
 */
/**@{*/

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM legacy scanning.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmScanInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM extended scanning.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmExtScanInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM Periodic Advertising Sync Transfer (PAST) module.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPastInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM Connection Constant Tone Extension (CTE) module.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnCteInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Whether DM scanning is in legacy mode.
 *
 *  \return TRUE if DM scanning is in legacy mode. FALSE, otherwise.
 */
/*************************************************************************************************/
bool_t DmScanModeLeg(void);

/*************************************************************************************************/
/*!
 *  \brief  Whether DM scanning is in extended mode.
 *
 *  \return TRUE if DM scanning is in extended mode. FALSE, otherwise.
 */
/*************************************************************************************************/
bool_t DmScanModeExt(void);

/*************************************************************************************************/
/*!
 *  \brief  Start scanning on the given PHYs.
 *
 *  \param  scanPhys  Scanner PHYs.
 *  \param  mode      Discoverability mode.
 *  \param  pScanType Scan type array.
 *  \param  filterDup Filter duplicates.  Set to TRUE to filter duplicate responses received
 *                    from the same device.  Set to FALSE to receive all responses.
 *  \param  duration  The scan duration, in milliseconds.  If set to zero or both duration and
 *                    period set to non-zero, scanning will continue until DmScanStop() is called.
 *  \param  period    The scan period, in 1.28 sec units (only applicable to AE).  If set to zero,
 *                    periodic scanning is disabled.
 *
 *  \return None.
 */
 /*************************************************************************************************/
void DmScanStart(uint8_t scanPhys, uint8_t mode, const uint8_t *pScanType, bool_t filterDup,
                 uint16_t duration, uint16_t period);

/*************************************************************************************************/
/*!
 *  \brief  Stop scanning.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmScanStop(void);

/*************************************************************************************************/
/*!
 *  \brief  Set the scan interval and window for the specified PHYs.
 *
 *  \param  scanPhys      Scanning PHYs.
 *  \param  pScanInterval Scan interval array.
 *  \param  pScanWindow   Scan window array.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmScanSetInterval(uint8_t scanPhys, uint16_t *pScanInterval, uint16_t *pScanWindow);

/*************************************************************************************************/
/*!
 *  \brief  Set the local address type used while scanning.  This function can be used to
 *          configure scanning to use a random address.
 *
 *  \param  addrType  Address type.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmScanSetAddrType(uint8_t addrType);

/*************************************************************************************************/
/*!
 *  \brief  Synchronize with periodic advertising from the given advertiser, and start receiving
 *          periodic advertising packets.
 *
 *          Note: The synchronization filter policy is used to determine whether the periodic
 *                advertiser list is used. If the periodic advertiser list is not used, the
 *                advertising SID, advertiser address type, and advertiser address parameters
 *                specify the periodic advertising device to listen to; otherwise these parameters
 *                are ignored.
 *
 *  \param  advSid        Advertising SID.
 *  \param  advAddrType   Advertiser address type.
 *  \param  pAdvAddr      Advertiser address.
 *  \param  skip          Number of periodic advertising packets that can be skipped after
 *                        successful receive.
 *  \param  syncTimeout   Synchronization timeout, in the units of 10ms.
 *  \param  syncCteType   Whether to only synchronize to periodic advertising with certain types
 *                        of Constant Tone Extension (0 indicates that the presence or absence of
 *                        a Constant Tone Extension is irrelevant).
 *
 *  \return Sync indentifier.
 */
/*************************************************************************************************/
dmSyncId_t DmSyncStart(uint8_t advSid, uint8_t advAddrType, const uint8_t *pAdvAddr, uint16_t skip,
                       uint16_t syncTimeout, uint8_t syncCteType);

/*************************************************************************************************/
/*!
 *  \brief  Stop reception of the periodic advertising identified by the given sync identifier.
 *
 *  \param  syncId        Sync identifier.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSyncStop(dmSyncId_t syncId);

/*************************************************************************************************/
/*!
 *  \brief  Set the encryption mode of the Broadcast Isochronous Group (BIG) corresponding to the
 *          periodic advertising train identified by the sync handle.
 *
 *  \param  syncHandle   Synch handle.
 *  \param  encrypt      FALSE (Unencrypted) or FALSE (Encrypted).
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSyncSetEncrypt(uint16_t syncHandle, bool_t encrypt);

/*************************************************************************************************/
/*!
 *  \brief  Get the encryption mode of the Broadcast Isochronous Group (BIG) corresponding to the
 *          periodic advertising train identified by the sync handle.
 *
 *  \param  syncHandle   Synch handle.
 *
 *  \return TRUE if sync encrypted. FALSE, otherwise.
 */
/*************************************************************************************************/
bool_t DmSyncEncrypted(uint16_t syncHandle);

/*************************************************************************************************/
/*!
 *  \brief  Get status of sync identified by the handle.
 *
 *  \param  syncHandle   Synch handle.
 *
 *  \return TRUE if sync is enabled for that handle. FALSE, otherwise.
 */
/*************************************************************************************************/
bool_t DmSyncEnabled(uint16_t syncHandle);

/*************************************************************************************************/
/*!
 *  \brief  DM enable or disable initial periodic advertisement reporting.
 *
 *  \param  enable        TRUE to enable initial reporting, FALSE to disable initial reporting.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSyncInitialRptEnable(bool_t enable);

/*************************************************************************************************/
/*!
 *  \brief  DM enable or disable initial periodic advertising duplicate filtering once synchronized.
 *
 *  \param  enable        TRUE to enable initial duplicate filtering, FALSE to disable initial
 *                        duplicate filtering.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSyncInitialDupFiltEnable(bool_t enable);

/*************************************************************************************************/
/*!
 *  \brief  Synchronize to a Broadcast Isochronous Group (BIG) described in the periodic
 *          advertising train specified by the sync handle.
 *
 *  \param  bigHandle       BIG handle.
 *  \param  syncHandle      Periodic advertising train handle.
 *  \param  mse             Maximum number of subevents.
 *  \param  bigSyncTimeout  Synchronization timeout for the BIS, in the units of 10ms.
 *  \param  numBis          Total number of BISes in the BIG.
 *  \param  pBis            List of indices of BISes (in ascending order).
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmBigSyncStart(uint8_t bigHandle, uint16_t syncHandle, uint8_t mse, uint16_t bigSyncTimeout,
                    uint8_t numBis, uint8_t *pBis);

/*************************************************************************************************/
/*!
 *  \brief  Stop synchronizing or cancel the process of synchronizing to the Broadcast Isochronous
 *           Group (BIG) identified by the handle.
 *
 *  \note   The command also terminates the reception of BISes in the BIG specified in \ref
 *          DmBigSyncStart, destroys the associated connection handles of the BISes in the BIG
 *          and removes the data paths for all BISes in the BIG.
 *
 *  \param  bigHandle       BIG handle.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmBigSyncStop(uint8_t bigHandle);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Return TRUE if the BIS sync is in use.
 *
 *  \param  handle  BIS connection handle.
 *
 *  \return TRUE if the BIS sync is in use, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t DmBisSyncInUse(uint16_t handle);

/*************************************************************************************************/
/*!
 *  \brief  Set the Broadcast Code for the given Broadcast Isochronous Group (BIG).
 *
 *  \param  bigHandle    BIG handle.
 *  \param  encrypt      FALSE (Unencrypted) or TRUE (Encrypted).
 *  \param  authen       FALSE (Unauthenticated) or TRUE (Authenticated).
 *  \param  pBcastCode   Broadcast code.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmBigSyncSetBcastCode(uint8_t bigHandle, bool_t encrypt, bool_t authen, uint8_t *pBcastCode);

/*************************************************************************************************/
/*!
 *  \brief  Set the security level of the LE Security Mode 3 for the given Broadcast Isochronous
 *          Group (BIG).
 *
 *  \param  bigHandle    BIG handle.
 *  \param  secLevel     Security level.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmBigSyncSetSecLevel(uint8_t bigHandle, uint8_t secLevel);

/*************************************************************************************************/
/*!
 *  \brief  Get the security level of the LE Security Mode 3 for the given Broadcast Isochronous
 *          Group (BIG) connection handle.
 *
 *  \param  handle  BIS connection handle.
 *
 *  \return Security level.
 */
/*************************************************************************************************/
uint8_t DmBigSyncGetSecLevel(uint16_t handle);

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM BIS manager for operation as master.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmBisMasterInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Add device to periodic advertiser list.
 *
 *  \param  advAddrType   Advertiser address type.
 *  \param  pAdvAddr      Advertiser address.
 *  \param  advSid        Advertising SID.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmAddDeviceToPerAdvList(uint8_t advAddrType, uint8_t *pAdvAddr, uint8_t advSid);

/*************************************************************************************************/
/*!
 *  \brief  DM remove device from periodic advertiser list.
 *
 *  \param  advAddrType   Advertiser address type.
 *  \param  pAdvAddr      Advertiser address.
 *  \param  advSid        Advertising SID.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmRemoveDeviceFromPerAdvList(uint8_t advAddrType, uint8_t *pAdvAddr, uint8_t advSid);

/*************************************************************************************************/
/*!
 *  \brief  DM clear periodic advertiser list.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmClearPerAdvList(void);

/*************************************************************************************************/
/*!
 *  \brief  Enable or disable reports and duplicate filtering for the periodic advertising identified
 *          by the sync id.
 *
 *  \param  syncId           Sync identifier.
 *  \param  enableBits       Whether to enable or disable reporting and duplicate filtering.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPastRptRcvEnable(dmSyncId_t syncId, bool_t enableBits);

/*************************************************************************************************/
/*!
 *  \brief  Send synchronization information about the periodic advertising identified by the
 *          sync id to a connected device.
 *
 *  \param  connId           Connection identifier.
 *  \param  serviceData      Value provided by the Host.
 *  \param  syncId           Sync identifier.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPastSyncTrsf(dmConnId_t connId, uint16_t serviceData, dmSyncId_t syncId);

/*************************************************************************************************/
/*!
 *  \brief  Send synchronization information about the periodic advertising in an advertising
 *          set to a connected device.
 *
 *  \param  connId           Connection identifier.
 *  \param  serviceData      Value provided by the Host.
 *  \param  advHandle        Advertising handle.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPastSetInfoTrsf(dmConnId_t connId, uint16_t serviceData, uint8_t advHandle);

/*************************************************************************************************/
/*!
 *  \brief  Specify how the Controller should process periodic advertising synchronization
 *          information received from the device identified by the connnection handle.
 *
 *  \param  connId           Connection identifier.
 *  \param  mode             Action to be taken when periodic advertising info is received.
 *  \param  skip             Number of consecutive periodic advertising packets that the receiver
 *                           may skip after successfully receiving a periodic advertising packet.
 *  \param  syncTimeout      Maximum permitted time between successful receives. If this time is
 *                           exceeded, synchronization is lost.
 *  \param  cteType          Whether to only synchronize to periodic advertising with certain
 *                           types of Constant Tone Extension (0 indicates that the presence or
 *                           absence of a Constant Tone Extension is irrelevant).
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPastConfig(dmConnId_t connId, uint8_t mode, uint16_t skip, uint16_t syncTimeout,
                  uint8_t cteType);

/*************************************************************************************************/
/*!
 *  \brief  Specify the initial value for the mode, skip, timeout, and Constant Tone Extension type
 *          to be used for all subsequent connections over the LE transport.
 *
 *  \param  mode             Action to be taken when periodic advertising info is received.
 *  \param  skip             Number of consecutive periodic advertising packets that the receiver
 *                           may skip after successfully receiving a periodic advertising packet.
 *  \param  syncTimeout      Maximum permitted time between successful receives. If this time is
 *                           exceeded, synchronization is lost.
 *  \param  cteType          Whether to only synchronize to periodic advertising with certain
 *                           types of Constant Tone Extension (0 indicates that the presence or
 *                           absence of a Constant Tone Extension is irrelevant).
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPastDefaultConfig(uint8_t mode, uint16_t skip, uint16_t syncTimeout, uint8_t cteType);

/*************************************************************************************************/
/*!
 *  \brief  Enable sampling received CTE fields on the specified connection, and configure the
 *          antenna switching pattern, and switching and sampling slot durations to be used.
 *
 *  \param  connId           Connection identifier.
 *  \param  slotDurations    Switching and sampling slot durations to be used while receiving CTE.
 *  \param  switchPatternLen Number of Antenna IDs in switching pattern.
 *  \param  pAntennaIDs      List of Antenna IDs in switching pattern.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnCteRxSampleStart(dmConnId_t connId, uint8_t slotDurations, uint8_t switchPatternLen,
                            uint8_t *pAntennaIDs);

/*************************************************************************************************/
/*!
 *  \brief  Disable sampling received CTE fields on the specified connection.
 *
 *  \param  connId           Connection identifier.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnCteRxSampleStop(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  Configure the antenna switching pattern, and permitted CTE types used for transmitting
 *          CTEs requested by the peer device on the specified connection.
 *
 *  \param  connId           Connection identifier.
 *  \param  cteTypeBits      Permitted CTE type bits used for transmitting CTEs requested by peer.
 *  \param  switchPatternLen Number of Antenna IDs in switching pattern.
 *  \param  pAntennaIDs      List of Antenna IDs in switching pattern.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnCteTxConfig(dmConnId_t connId, uint8_t cteTypeBits, uint8_t switchPatternLen,
                       uint8_t *pAntennaIDs);

/*************************************************************************************************/
/*!
 *  \brief  Initiate the CTE Request procedure on the specified connection.
 *
 *  \param  connId           Connection identifier.
 *  \param  cteReqInt        CTE request interval.
 *  \param  reqCteLen        Minimum length of CTE being requested in 8 us units.
 *  \param  reqCteType       Requested CTE type.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnCteReqStart(dmConnId_t connId, uint16_t cteReqInt, uint8_t reqCteLen,
                       uint8_t reqCteType);

/*************************************************************************************************/
/*!
 *  \brief  Stop initiating the CTE Request procedure on the specified connection.
 *
 *  \param  connId           Connection identifier.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnCteReqStop(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  Start responding to LL_CTE_REQ PDUs with LL_CTE_RSP PDUs on the specified connection.
 *
 *  \param  connId           Connection identifier.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnCteRspStart(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  Stop responding to LL_CTE_REQ PDUs with LL_CTE_RSP PDUs on the specified connection.
 *
 *  \param  connId           Connection identifier.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnCteRspStop(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  Returns the device manager's CTE request state for a given connection.
 *
 *  \param  connId           Connection identifier.
 *
 *  \return The CTE request state.
 */
/*************************************************************************************************/
uint8_t DmConnCteGetReqState(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  Returns the device manager's CTE response state for a given connection.
 *
 *  \param  connId           Connection identifier.
 *
 *  \return The CTE response state.
 */
/*************************************************************************************************/
uint8_t DmConnCteGetRspState(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  Read the switching rates, the sampling rates, the number of antennae, and the maximum
 *          length of a transmitted Constant Tone Extension supported by the Controller.
 *
 *  \return None.
 *
 *  \note   The antenna info will be returned with DM indication \ref DM_READ_ANTENNA_INFO_IND.
 */
/*************************************************************************************************/
void DmReadAntennaInfo(void);

/**@}*/

/** \name DM Connection Functions
 * Functions for forming connections and managing connection behavior and parameter updates.
 */
/**@{*/

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM connection manager.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM connection manager for operation as legacy master.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnMasterInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM connection manager for operation as extended master.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmExtConnMasterInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM connection manager for operation as legacy slave.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnSlaveInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM connection manager for operation as extended slave.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmExtConnSlaveInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Register with the DM connection manager.
 *
 *  \param  clientId  The client identifier.
 *  \param  cback     Client callback function.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnRegister(uint8_t clientId, dmCback_t cback);

/*************************************************************************************************/
/*!
 *  \brief  Open a connection to a peer device with the given address.
 *
 *  \param  clientId  The client identifier.
 *  \param  initPhys  Initiator PHYs.
 *  \param  addrType  Address type.
 *  \param  pAddr     Peer device address.
 *
 *  \return Connection identifier.
 */
/*************************************************************************************************/
dmConnId_t DmConnOpen(uint8_t clientId, uint8_t initPhys, uint8_t addrType, uint8_t *pAddr);

/*************************************************************************************************/
/*!
 *  \brief  Abort connection open operation
 */
/*************************************************************************************************/
void DmConnCancelOpen(void);

/*************************************************************************************************/
/*!
 *  \brief  Close the connection with the give connection identifier.
 *
 *  \param  clientId  The client identifier.
 *  \param  connId    Connection identifier.
 *  \param  reason    Reason connection is being closed.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnClose(uint8_t clientId, dmConnId_t connId, uint8_t reason);

/*************************************************************************************************/
/*!
 *  \brief  Accept a connection from the given peer device by initiating directed advertising.
 *
 *  \param  clientId     The client identifier.
 *  \param  advHandle    Advertising handle.
 *  \param  advType      Advertising type.
 *  \param  duration     Advertising duration (in ms).
 *  \param  maxEaEvents  Maximum number of extended advertising events.
 *  \param  addrType     Address type.
 *  \param  pAddr        Peer device address.
 *
 *  \return Connection identifier.
 */
/*************************************************************************************************/
dmConnId_t DmConnAccept(uint8_t clientId, uint8_t advHandle, uint8_t advType, uint16_t duration,
                        uint8_t maxEaEvents, uint8_t addrType, uint8_t *pAddr);

/*************************************************************************************************/
/*!
 *  \brief  Update the connection parameters of an open connection
 *
 *  \param  connId      Connection identifier.
 *  \param  pConnSpec   Connection specification.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnUpdate(dmConnId_t connId, hciConnSpec_t *pConnSpec);

/*************************************************************************************************/
/*!
 *  \brief  Set the scan interval and window for connections to be created with DmConnOpen().
 *
 *  \param  scanInterval  The scan interval.
 *  \param  scanWindow    The scan window.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnSetScanInterval(uint16_t scanInterval, uint16_t scanWindow);

/*************************************************************************************************/
/*!
 *  \brief  Set the scan interval and window for extended connections to be created with
 *          DmConnOpen().
 *
 *  \param  initPhys      Initiator PHYs.
 *  \param  pScanInterval Scan interval array.
 *  \param  pScanWindow   Scan window array.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmExtConnSetScanInterval(uint8_t initPhys, uint16_t *pScanInterval, uint16_t *pScanWindow);

/*************************************************************************************************/
/*!
 *  \brief  Set the connection spec parameters for connections to be created with DmConnOpen().
 *
 *  \param  pConnSpec   Connection spec parameters.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnSetConnSpec(hciConnSpec_t *pConnSpec);

/*************************************************************************************************/
/*!
 *  \brief  Set the extended connection spec parameters for extended connections to be created
 *          with DmConnOpen().
 *
 *  \param  initPhys    The initiator PHYs.
 *  \param  pConnSpec   Connection spec parameters array.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmExtConnSetConnSpec(uint8_t initPhys, hciConnSpec_t *pConnSpec);

/*************************************************************************************************/
/*!
 *  \brief  Set the local address type used for connections created with DmConnOpen().
 *
 *  \param  addrType  Address type.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnSetAddrType(uint8_t addrType);

/*************************************************************************************************/
/*!
 *  \brief  Configure a bit in the connection idle state mask as busy or idle.
 *
 *  \param  connId      Connection identifier.
 *  \param  idleMask    Bit in the idle state mask to configure.
 *  \param  idle        DM_CONN_BUSY or DM_CONN_IDLE.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnSetIdle(dmConnId_t connId, uint16_t idleMask, uint8_t idle);

/*************************************************************************************************/
/*!
 *  \brief  Check if a connection is idle.
 *
 *  \param  connId      Connection identifier.
 *
 *  \return Zero if connection is idle, nonzero if busy.
 */
/*************************************************************************************************/
uint16_t DmConnCheckIdle(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  Read RSSI of a given connection.
 *
 *  \param  connId      Connection identifier.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnReadRssi(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  Reply to the HCI remote connection parameter request event.  This command is used to
 *          indicate that the Host has accepted the remote device's request to change connection
 *          parameters.
 *
 *  \param  connId      Connection identifier.
 *  \param  pConnSpec   Connection specification.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmRemoteConnParamReqReply(dmConnId_t connId, hciConnSpec_t *pConnSpec);

/*************************************************************************************************/
/*!
 *  \brief  Negative reply to the HCI remote connection parameter request event.  This command
 *          is used to indicate that the Host has rejected the remote device's request to change
 *          connection parameters.
 *
 *  \param  connId      Connection identifier.
 *  \param  reason      Reason for rejection.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmRemoteConnParamReqNegReply(dmConnId_t connId, uint8_t reason);

/*************************************************************************************************/
/*!
 *  \brief  Set data length for a given connection.
 *
 *  \param  connId      Connection identifier.
 *  \param  txOctets    Maximum number of payload octets for a Data PDU.
 *  \param  txTime      Maximum number of microseconds for a Data PDU.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnSetDataLen(dmConnId_t connId, uint16_t txOctets, uint16_t txTime);

/*************************************************************************************************/
/*!
 *  \brief  Return the connection role indicating master or slave.
 *
 *  \param  connId      Connection identifier.
 *
 *  \return Device role.
 */
/*************************************************************************************************/
uint8_t DmConnRole(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  Set authenticated payload timeout for a given connection.
 *
 *  \param  connId      Connection identifier.
 *  \param  timeout     Timeout period in units of 10ms.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmWriteAuthPayloadTimeout(dmConnId_t connId, uint16_t timeout);

/*************************************************************************************************/
/*!
 *  \brief  Request the Sleep Clock Accuracy (SCA) of a peer device.
 *
 *  \param  connId      Connection identifier.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmConnRequestPeerSca(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM Enhanced Connection Update (ECU) module.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmEcuInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Set the initial values for the acceptable parameters for subrating requests for all
 *          future ACL connections where the local device is the Central.
 *
 *  \param  pSubrate    Subrate parameter.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmEcuSetDefSubrate(HciSubrateParam_t *pSubrate);

/*************************************************************************************************/
/*!
 *  \brief  Request a change to the subrating factor and other parameters applied to the specified
 *          connection.
 *
 *  \param  connId      Connection identifier.
 *  \param  pSubrate    Subrate parameter.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmEcuUpdateSubrate(dmConnId_t connId, HciSubrateParam_t *pSubrate);

/**@}*/

/** \name DM CIS Functions
* Functions for forming and managing Connected Isochronous Stream (CIS) streams.
*/
/**@{*/

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM Connected Isochronous Stream (CIS) manager.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmCisInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM Connected Isochronous Stream (CIS) manager for operation as master.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmCisMasterInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM Connected Isochronous Stream (CIS) manager for operation as slave.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmCisSlaveInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Set the interval, in microseconds, of periodic SDUs for the given Connected Isochronous
 *          Group (CIG).
 *
 *  \param  cigId            CIG ID.
 *  \param  sduIntervalMToS  Time interval between start of consecutive SDUs from master Host.
 *  \param  sduIntervalSToM  Time interval between start of consecutive SDUs from slave Host.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmCisCigSetSduInterval(uint8_t cigId, uint32_t sduIntervalMToS, uint32_t sduIntervalSToM);

/*************************************************************************************************/
/*!
 *  \brief  Set the slaves clock accuracy for the given Connected Isochronous Group (CIG).
 *
 *  \param  cigId   CIG identifier.
 *  \param  sca     Slaves clck accuracy (0 if unknown).
 *
 *  \return None.
 *
 *  \note   The slaves clock accuracy must which must be the worst-case sleep clock accuracy of the
 *          slaves that will participate in the CIG.
 */
/*************************************************************************************************/
void DmCisCigSetSca(uint8_t cigId, uint8_t sca);

/*************************************************************************************************/
/*!
 *  \brief  Set the packing scheme and framing format for the given Connected Isochronous Group
 *          (CIG).
 *
 *  \param  cigId       CIG identifier.
 *  \param  packing     Packing scheme.
 *  \param  framing     Indicates format of CIS Data PDUs.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmCisCigSetPackingFraming(uint8_t cigId, uint8_t packing, uint8_t framing);

/*************************************************************************************************/
/*!
 *  \brief  Set the maximum transport latency, in microseconds, for the given Connected Isochronous
 *          Group (CIG).
 *
 *  \param  cigId        CIG identifier.
 *  \param  transLatMToS Maximum time for SDU to be transported from master Controller to slave Controller.
 *  \param  transLatSToM Maximum time for SDU to be transported from slave Controller to master Controller.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmCisCigSetTransLatInterval(uint8_t cigId, uint16_t transLatMToS, uint16_t transLatSToM);

/*************************************************************************************************/
/*!
 *  \brief  Set the parameters of one or more Connected Isochronous Streams (CISes) that are
 *          associated with the given Connected Isochronous Group (CIG).
 *
 *  \param  cigId       CIG identifier.
 *  \param  numCis      Number of CIS to be configured.
 *  \param  pCisParam   CIS parameters.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmCisCigConfig(uint8_t cigId, dmConnId_t numCis, HciCisCisParams_t *pCisParam);

/*************************************************************************************************/
/*!
 *  \brief  Remove all the Connected Isochronous Streams (CISes) associated with the given
 *          Connected Isochronous Group (CIG).
 *
 *  \param  cigId       CIG identifier.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmCisCigRemove(uint8_t cigId);

/*************************************************************************************************/
/*!
 *  \brief  Create one or more Connected Isochronous Streams (CISes) using the connections
 *          identified by the ACL connection handles.
 *
 *  \param  numCis      Total number of CISes to be created.
 *  \param  pCisHandle  List of connection handles of CISes.
 *  \param  pConnId     List of DM connection identifiers.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmCisOpen(uint8_t numCis, uint16_t *pCisHandle, dmConnId_t *pConnId);

/*************************************************************************************************/
/*!
 *  \brief  Inform the Controller to accept the request for the Connected Isochronous Stream (CIS)
 *          that is identified by the connection handle.
 *
 *  \param  handle    Connection handle of the CIS.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmCisAccept(uint16_t handle);

/*************************************************************************************************/
/*!
 *  \brief  Inform the Controller to reject the request for the Connected Isochronous Stream (CIS)
 *          that is identified by the connection handle.
 *
 *  \param  handle    Connection handle of the CIS to be rejected.
 *  \param  reason    Reason the CIS request was rejected.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmCisReject(uint16_t handle, uint8_t reason);

/*************************************************************************************************/
/*!
 *  \brief  Close the Connected Isochronous Stream (CIS) connection with the given handle.
 *
 *  \param  handle    CIS connection handle.
 *  \param  reason    Reason connection is being closed.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmCisClose(uint16_t handle, uint8_t reason);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Find the Connected Isochronous Stream (CIS) ID with matching
 *          handle.
 *
 *  \param  handle  CIS connection handle.
 *
 *  \return CIS identifier or DM_CIS_ID_NONE if error.
 */
/*************************************************************************************************/
uint8_t DmCisIdByHandle(uint16_t handle);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Find the Connected Isochronous Stream (CIS) handle with matching
 *          CIG and CIS identifiers.
 *
 *  \param  handle  CIG ID.
 *  \param  handle  CIS ID.
 *
 *  \return CIS connection handle or DM_CONN_HCI_HANDLE_NONE if error.
 */
/*************************************************************************************************/
uint16_t DmCisHandleById(uint8_t cigId, uint8_t cisId);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Return TRUE if the Connected Isochronous Stream (CIS)
 *          connection is in use.
 *
 *  \param  handle  CIS connection handle.
 *
 *  \return TRUE if the CIS connection is in use, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t DmCisConnInUse(uint16_t handle);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Return the CIS connection role indicating master or slave.
 *
 *  \param  handle  CIS connection handle.
 *
 *  \return CIS connection role.
 */
/*************************************************************************************************/
uint8_t DmCisConnRole(uint16_t handle);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Return TRUE if Connected Isochronous Group (CIG) is in use.
 *
 *  \param  cigId   CIG identifier.
 *
 *  \return TRUE if CIG is in use, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t DmCisCigInUse(uint8_t cigId);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Return TRUE if the Connected Isochronous Stream (CIS)
 *          connection is in use.
 *
 *  \param  cigId   CIG identifier.
 *  \param  cisId   CIS identifier.
 *
 *  \return TRUE if the CIS connection is in use, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t DmCisInUse(uint8_t cigId, uint8_t cisId);

/**@}*/

/** \name DM BIS Functions
* Functions for forming and managing Broadcast Isochronous Stream (BIS) streams and synchronization.
*/
/**@{*/

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM BIS manager for operation as slave.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmBisSlaveInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Start a Broadcast Isochronous Group (BIG) with one or more Broadcast Isochronous
 *          Streams (BISes).
 *
 *  \param  bigHandle     CIG identifier.
 *  \param  advHandle     Used to identify the periodic advertising train.
 *  \param  numBis;       Total number of BISes in the BIG.
 *  \param  sduInterUsec  Interval, in microseconds, of BIG SDUs.
 *  \param  maxSdu        Maximum size of SDU
 *  \param  mtlMs         Maximum time, in milliseconds, for transmitting SDU.
 *  \param  rtn           Retransmission number.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmBigStart(uint8_t bigHandle, uint8_t advHandle, uint8_t numBis, uint32_t sduInterUsec,
                uint16_t maxSdu, uint16_t mtlMs, uint8_t rtn);

/*************************************************************************************************/
/*!
 *  \brief  Stop a Broadcast Isochronous Group (BIG) identified for the given handle.
 *
 *  \param  bigHandle   BIG identifier.
 *  \param  reason      Reason BIG is terminated.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmBigStop(uint8_t bigHandle, uint8_t reason);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Return TRUE if the BIS is in use.
 *
 *  \param  handle  BIS connection handle.
 *
 *  \return TRUE if the BIS connection is in use, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t DmBisInUse(uint16_t handle);

/*************************************************************************************************/
/*!
 *  \brief  Set the PHYs used for transmission of PDUs of Broadcast Isochronous Streams (BISes) in
 *          Broadcast Isochronous Group (BIG).
 *
 *  \param  bigHandle   BIG handle.
 *  \param  phyBits     PHY bit field.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmBigSetPhy(uint8_t bigHandle, uint8_t phyBits);

/*************************************************************************************************/
/*!
 *  \brief  Set the packing scheme and framing format for the given Broadcast Isochronous Group
 *          (BIG).
 *
 *  \param  bigHandle   BIG handle.
 *  \param  packing     Packing scheme.
 *  \param  framing     Indicates format of BIS Data PDUs.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmBigSetPackingFraming(uint8_t bigHandle, uint8_t packing, uint32_t framing);

/*************************************************************************************************/
/*!
 *  \brief  Set the Broadcast Code for the given Broadcast Isochronous Group (BIG).
 *
 *  \param  bigHandle    BIG handle.
 *  \param  encrypt      FALSE (Unencrypted) or TRUE (Encrypted).
 *  \param  authen       FALSE (Unauthenticated) or TRUE (Authenticated).
 *  \param  pBcastCode   Broadcast code.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmBigSetBcastCode(uint8_t bigHandle, bool_t encrypt, bool_t authen, uint8_t *pBcastCode);

/*************************************************************************************************/
/*!
 *  \brief  Set the security level of the LE Security Mode 3 for the given Broadcast Isochronous
 *          Group (BIG).
 *
 *  \param  bigHandle    BIG handle.
 *  \param  secLevel     Security level.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmBigSetSecLevel(uint8_t bigHandle, uint8_t secLevel);

/*************************************************************************************************/
/*!
 *  \brief  Get the security level of the LE Security Mode 3 for the given Broadcast Isochronous
 *          Group (BIG) connection handle.
 *
 *  \param  handle  BIS connection handle.
 *
 *  \return Security level.
 */
/*************************************************************************************************/
uint8_t DmBigGetSecLevel(uint16_t handle);

/** \name DM Isochronous (ISO) Functions
* Functions for setting up and managing isochronous data path between the Host and the Controller.
*/
/**@{*/
/*************************************************************************************************/
/*!
 *  \brief  Initialize DM ISO manager.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmIsoInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Register CIS, BIS and setup callbacks for the HCI ISO data path.
 *
 *  \param  cisCback    CIS data callback function (may be NULL).
 *  \param  bisCback    BIS data callback function (may be NULL).
 *  \param  setupCback  ISO data path setup callback function (NULL when the codec in the LL).
 *  \param  removeCback ISO data path remove callback function (NULL when the codec in the LL).
 *
 *  \return None.
 */
 /*************************************************************************************************/
void DmIsoRegister(hciIsoCback_t cisCback, hciIsoCback_t bisCback,
                   dmIsoDataPathSetupCback_t setupCback, dmIsoDataPathRemoveCback_t removeCback);

/*************************************************************************************************/
/*!
 *  \brief  Setup the isochronous data path between the Host and the Controller for an established
 *          Connected Isochronous Stream (CIS) or Broadcast Isochronous Stream (BIS) identified by
 *          the connection handle parameter.
 *
 *  \param  pDataPathParam Parameters to setup ISO data path.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmIsoDataPathSetup(HciIsoSetupDataPath_t *pDataPathParam);

/*************************************************************************************************/
/*!
 *  \brief  Remove the input and/or output data path(s) associated with a Connected Isochronous
 *          Stream (CIS) or Broadcast Isochronous Stream (BIS) identified by the connection handle
 *          parameter.
 *
 *  \param  handle         Connection handle of CIS or BIS.
 *  \param  directionBits  Data path direction bits.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmIsoDataPathRemove(uint16_t handle, uint8_t directionBits);

/*************************************************************************************************/
/*!
 *  \brief  Request the Controller to configure the data transport path in a given direction
 *          between the Controller and the Host.
 *
 *  \param  pDataPathParam  Parameters for configuring data path.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmDataPathConfig(HciConfigDataPath_t *pDataPathParam);

/*************************************************************************************************/
/*!
 *  \brief  Read a list of the codecs supported by the Controller, as well as vendor specific
 *          codecs, which are defined by an individual manufacturer.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmReadLocalSupCodecs(void);

/*************************************************************************************************/
/*!
 *  \brief  Read a list of codec capabilities supported by the Controller for a given codec.
 *
 *  \param  pCodecParam  Parameters for reading local supported codec capabilities.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmReadLocalSupCodecCap(HciReadLocalSupCodecCaps_t *pCodecParam);

/*************************************************************************************************/
/*!
 *  \brief  Read the range of supported Controller delays for the codec specified by Codec ID on
 *          a given transport type specified by Logical Transport Type, in the direction specified
 *          by Direction, and with the codec configuration specified by Codec Configuration.
 *
 *  \param  pDelayParam  Parameters for reading local supported controller delay.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmReadLocalSupCtrDly(HciReadLocalSupControllerDly_t *pDelayParam);

/*************************************************************************************************/
/*!
 *  \brief  Send ISO Data packet.
 *
 *  \param  pIsoParam  ISO data packet parameters.
 */
/*************************************************************************************************/
void DmSendIsoData(uint16_t handle, uint16_t len, uint8_t *pData, bool_t useTs, uint32_t ts);

/**@}*/

/** \name DM PHY Control Functions
 * Functions for setting PHY preferences.
 */
/**@{*/

/*************************************************************************************************/
/*!
 *  \brief  Set the preferred values for the transmitter PHY and receiver PHY for all subsequent
 *          connections.
 *
 *  \param  allPhys     All PHYs preferences.
 *  \param  txPhys      Preferred transmitter PHYs.
 *  \param  rxPhys      Preferred receiver PHYs.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSetDefaultPhy(uint8_t allPhys, uint8_t txPhys, uint8_t rxPhys);

/*************************************************************************************************/
/*!
 *  \brief  Read the current transmitter PHY and receiver PHY for a given connection.
 *
 *  \param  connId      Connection identifier.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmReadPhy(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  Set the PHY preferences for a given connection.
 *
 *  \param  connId      Connection identifier.
 *  \param  allPhys     All PHYs preferences.
 *  \param  txPhys      Preferred transmitter PHYs.
 *  \param  rxPhys      Preferred receiver PHYs.
 *  \param  phyOptions  PHY options.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSetPhy(dmConnId_t connId, uint8_t allPhys, uint8_t txPhys, uint8_t rxPhys, uint16_t phyOptions);

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM PHY.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmPhyInit(void);

/**@}*/


/** \name DM Device Functions
 *  Device control functions
 */
/**@{*/

/*************************************************************************************************/
/*!
 *  \brief  Reset the device.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmDevReset(void);

/*************************************************************************************************/
/*!
 *  \brief  Set the random address to be used by the local device.
 *
 *  \param  pAddr     Random address.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmDevSetRandAddr(uint8_t *pAddr);

/*************************************************************************************************/
/*!
 *  \brief  Add a peer device to the white list.  Note that this function cannot be called
 *          while advertising, scanning, or connecting with white list filtering active.
 *
 *  \param  addrType  Address type.
 *  \param  pAddr     Peer device address.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmDevWhiteListAdd(uint8_t addrType, uint8_t *pAddr);

/*************************************************************************************************/
/*!
 *  \brief  Remove a peer device from the white list.  Note that this function cannot be called
 *          while advertising, scanning, or connecting with white list filtering active.
 *
 *  \param  addrType  Address type.
 *  \param  pAddr     Peer device address.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmDevWhiteListRemove(uint8_t addrType, uint8_t *pAddr);

/*************************************************************************************************/
/*!
 *  \brief  Clear the white list.  Note that this function cannot be called while
 *          advertising, scanning, or connecting with white list filtering active.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmDevWhiteListClear(void);

/*************************************************************************************************/
/*!
 *  \brief  Set the Advertising, Scanning or Initiator filter policy.
 *
 *  \param  mode     Policy mode.
 *  \param  policy   Filter policy.
 *
 *  \return TRUE if the filter policy was successfully set, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t DmDevSetFilterPolicy(uint8_t mode, uint8_t policy);

/*************************************************************************************************/
/*!
 *  \brief  Set the Advertising filter policy for the given advertising, Scanning or Initiator
 *          filter policy.
 *
 *  \param  advHandle  Advertising handle (only applicable to advertising).
 *  \param  mode       Policy mode.
 *  \param  policy     Filter policy.
 *
 *  \return TRUE if the filter policy was successfully set, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t DmDevSetExtFilterPolicy(uint8_t advHandle, uint8_t mode, uint8_t policy);

/*************************************************************************************************/
/*!
 *  \brief  Vendor-specific controller initialization function.
 *
 *  \param  param    Vendor-specific parameter.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmDevVsInit(uint8_t param);

/**@}*/

/** \name DM Security Functions
 * Functions for accessing and controlling security configuration of device.
 */
/**@{*/

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM security.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSecInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Initialize DM LE Secure Connections security.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSecLescInit(void);

/*************************************************************************************************/
/*!
 *  \brief  This function is called by a master device to initiate pairing.
 *
 *  \param  connId    DM connection ID.
 *  \param  oob       Out-of-band pairing data present or not present.
 *  \param  auth      Authentication and bonding flags.
 *  \param  iKeyDist  Initiator key distribution flags.
 *  \param  rKeyDist  Responder key distribution flags.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSecPairReq(dmConnId_t connId, uint8_t oob, uint8_t auth, uint8_t iKeyDist, uint8_t rKeyDist);

/*************************************************************************************************/
/*!
 *  \brief  This function is called by a slave device to proceed with pairing after a
 *          DM_SEC_PAIR_IND event is received.
 *
 *  \param  connId    DM connection ID.
 *  \param  oob       Out-of-band pairing data present or not present.
 *  \param  auth      Authentication and bonding flags.
 *  \param  iKeyDist  Initiator key distribution flags.
 *  \param  rKeyDist  Responder key distribution flags.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSecPairRsp(dmConnId_t connId, uint8_t oob, uint8_t auth, uint8_t iKeyDist, uint8_t rKeyDist);

/*************************************************************************************************/
/*!
 *  \brief  This function is called to cancel the pairing process.
 *
 *  \param  connId    DM connection ID.
 *  \param  reason    Failure reason.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSecCancelReq(dmConnId_t connId, uint8_t reason);

/*************************************************************************************************/
/*!
 *  \brief  This function is called in response to a DM_SEC_AUTH_REQ_IND event to provide
 *          PIN or OOB data during pairing.
 *
 *  \param  connId      DM connection ID.
 *  \param  authDataLen Length of PIN or OOB data.
 *  \param  pAuthData   pointer to PIN or OOB data.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSecAuthRsp(dmConnId_t connId, uint8_t authDataLen, uint8_t *pAuthData);

/*************************************************************************************************/
/*!
 *  \brief  This function is called by a slave device to request that the master initiates
 *          pairing or link encryption.
 *
 *  \param  connId    DM connection ID.
 *  \param  auth      Authentication flags.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSecSlaveReq(dmConnId_t connId, uint8_t auth);

/*************************************************************************************************/
/*!
 *  \brief  This function is called by a master device to initiate link encryption.
 *
 *  \param  connId    DM connection ID.
 *  \param  secLevel  Security level of pairing when LTK was exchanged.
 *  \param  pLtk      Pointer to LTK parameter structure.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSecEncryptReq(dmConnId_t connId, uint8_t secLevel, dmSecLtk_t *pLtk);

/*************************************************************************************************/
/*!
 *  \brief  This function is called by a slave in response to a DM_SEC_LTK_REQ_IND event
 *          to provide the long term key used for encryption.
 *
 *  \param  connId    DM connection ID.
 *  \param  keyFound  TRUE if key found.
 *  \param  secLevel  Security level of pairing when key was exchanged.
 *  \param  pKey      Pointer to the key, if found.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSecLtkRsp(dmConnId_t connId, bool_t keyFound, uint8_t secLevel, uint8_t *pKey);

/*************************************************************************************************/
/*!
 *  \brief  This function sets the local CSRK used by the device.
 *
 *  \param  pCsrk     Pointer to CSRK.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSecSetLocalCsrk(uint8_t *pCsrk);

/*************************************************************************************************/
/*!
 *  \brief  This function sets the local IRK used by the device.
 *
 *  \param  pIrk     Pointer to IRK.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSecSetLocalIrk(uint8_t *pIrk);

/*************************************************************************************************/
/*!
 *  \brief  This function sets the local identity address used by the device.
 *
 *  \param  identityAddr  Local identity address.
 *  \param  addrType      Local identity address type.
 *
 *  \return None.
 */
 /*************************************************************************************************/
void DmSecSetLocalIdentityAddr(const bdAddr_t identityAddr, uint8_t addrType);

/*************************************************************************************************/
/*!
 *  \brief  This function generates an ECC key for use with LESC security.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSecGenerateEccKeyReq(void);

/*************************************************************************************************/
/*!
 *  \brief  This function sets the ECC key for use with LESC security.
 *
 *  \param  pKey      Pointer to key.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSecSetEccKey(secEccKey_t *pKey);

/*************************************************************************************************/
/*!
 *  \brief  This function gets the local ECC key for use with LESC security.
 *
 *  \return Pointer to local ECC key.
 */
/*************************************************************************************************/
secEccKey_t *DmSecGetEccKey(void);

/*************************************************************************************************/
/*!
 *  \brief  This function sets the ECC key for use with LESC security to standard debug keys values.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSecSetDebugEccKey(void);

/*************************************************************************************************/
/*!
 *  \brief  This function configures the DM to use OOB pairing for the given connection.
 *          The pRand and pConfirm contain the Random and Confirm values exchanged via
 *          out of band methods.
 *
 *  \param  connId      ID of the connection.
 *  \param  pConfig     Pointer to OOB configuration.
 *
 *  \return Pointer to IRK.
 */
/*************************************************************************************************/
void DmSecSetOob(dmConnId_t connId, dmSecLescOobCfg_t *pConfig);

/*************************************************************************************************/
/*!
 *  \brief  This function calculates the local random and confirm values used in LESC OOB pairing.
 *          The operation's result is posted as a DM_SEC_CALC_OOB_IND event to the application's DM
 *          callback handler.  The local rand and confirm values are exchanged with the peer via
 *          out-of-band (OOB) methods and passed into the DmSecSetOob after DM_CONN_OPEN_IND.
 *
 *  \param  pRand       Random value used in calculation.
 *  \param  pPubKeyX    X component of the local public key.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSecCalcOobReq(uint8_t *pRand, uint8_t *pPubKeyX);

/*************************************************************************************************/
/*!
 *  \brief  This function is called by the application in response to a DM_SEC_COMPARE_IND event.
 *          The valid parameter indicates if the compare value of the DM_SEC_COMPARE_IND was valid.
 *
 *  \param  connId      ID of the connection.
 *  \param  valid       TRUE if compare value was valid
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSecCompareRsp(dmConnId_t connId, bool_t valid);

/*************************************************************************************************/
/*!
 *  \brief  This function returns the 6-digit compare value for the specified 128-bit confirm value.
 *
 *  \param  pConfirm    Pointer to 128-bit comfirm value.
 *
 *  \return Six-digit compare value.
 */
/*************************************************************************************************/
uint32_t DmSecGetCompareValue(uint8_t *pConfirm);

/**@}*/

/** \name DM Internal Functions
 * Functions called internally by the stack.
 */
/**@{*/

/*************************************************************************************************/
/*!
 *  \brief  Map an address type to a type used by LL.
 *
 *  \param  addrType   Address type used by Host.
 *
 *  \return Address type used by LL.
 */
/*************************************************************************************************/
uint8_t DmLlAddrType(uint8_t addrType);

/*************************************************************************************************/
/*!
 *  \brief  Map an address type to a type used by Host.
 *
 *  \param  addrType   Address type used by LL.
 *
 *  \return Address type used by Host.
 */
/*************************************************************************************************/
uint8_t DmHostAddrType(uint8_t addrType);

/*************************************************************************************************/
/*!
 *  \brief  Return size of a DM callback event.
 *
 *  \param  pDmEvt  DM callback event.
 *
 *  \return Size of DM callback event.
 */
/*************************************************************************************************/
uint16_t DmSizeOfEvt(dmEvt_t *pDmEvt);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  L2C calls this function to send the result of an L2CAP
 *          connection update response to DM.
 *
 *  \param  handle  Connection handle.
 *  \param  reason  Connection update response reason code.
 *  \return None.
 */
/*************************************************************************************************/
void DmL2cConnUpdateCnf(uint16_t handle, uint16_t reason);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  L2C calls this function to send the result of an L2CAP
 *          Command Reject up to the application.
 *
 *  \param  handle  Connection handle.
 *  \param  result  Connection update result code.
 *  \return None.
 */
/*************************************************************************************************/
void DmL2cCmdRejInd(uint16_t handle, uint16_t result);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  L2C calls this function when it receives a connection update
 *          request from a peer device.
 *
 *  \param  identifier  Identifier value.
 *  \param  handle      Connection handle.
 *  \param  pConnSpec   Connection spec parameters.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmL2cConnUpdateInd(uint8_t identifier, uint16_t handle, hciConnSpec_t *pConnSpec);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Find the connection ID with matching handle.
 *
 *  \param  handle  Handle to find.
 *
 *  \return Connection ID or DM_CONN_ID_NONE if error.
 */
/*************************************************************************************************/
dmConnId_t DmConnIdByHandle(uint16_t handle);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Return TRUE if the connection is in use.
 *
 *  \param  connId  Connection ID.
 *
 *  \return TRUE if the connection is in use, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t DmConnInUse(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  Count active connections
 * *
 *  \return Number of active connections.
 */
/*************************************************************************************************/
uint8_t DmConnActiveCount(void);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Return the peer address type.
 *
 *  \param  connId  Connection ID.
 *
 *  \return Peer address type.
 */
/*************************************************************************************************/
uint8_t DmConnPeerAddrType(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Return the peer device address.
 *
 *  \param  connId  Connection ID.
 *
 *  \return Pointer to peer device address.
 */
/*************************************************************************************************/
uint8_t *DmConnPeerAddr(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Return the local address type.
 *
 *  \param  connId  Connection ID.
 *
 *  \return Local address type.
 */
/*************************************************************************************************/
uint8_t DmConnLocalAddrType(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Return the local address.
 *
 *  \param  connId  Connection ID.
 *
 *  \return Pointer to local address.
 */
/*************************************************************************************************/
uint8_t *DmConnLocalAddr(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Return the peer resolvable private address (RPA).
 *
 *  \param  connId  Connection ID.
 *
 *  \return Pointer to peer RPA.
 */
/*************************************************************************************************/
uint8_t *DmConnPeerRpa(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Return the local resolvable private address (RPA).
 *
 *  \param  connId  Connection ID.
 *
 *  \return Pointer to local RPA.
 */
/*************************************************************************************************/
uint8_t *DmConnLocalRpa(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Return the security level of the connection.
 *
 *  \param  connId  Connection ID.
 *
 *  \return Security level of the connection.
 */
/*************************************************************************************************/
uint8_t DmConnSecLevel(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  This function is called by SMP to request encryption.
 *
 *  \param  connId    DM connection ID.
 *  \param  secLevel  Security level of pairing when key was exchanged.
 *  \param  pKey      Pointer to key.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSmpEncryptReq(dmConnId_t connId, uint8_t secLevel, uint8_t *pKey);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Execute DM callback from SMP procedures.
 *
 *  \param  pDmEvt    Pointer to callback event data.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmSmpCbackExec(dmEvt_t *pDmEvt);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  This function gets the local CSRK used by the device.
 *
 *  \return Pointer to CSRK.
 */
/*************************************************************************************************/
uint8_t *DmSecGetLocalCsrk(void);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  This function gets the local IRK used by the device.
 *
 *  \return Pointer to IRK.
 */
/*************************************************************************************************/
uint8_t *DmSecGetLocalIrk(void);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  This function gets the local identity address used by the device.
 *
 *  \return Pointer to the identity address.
 */
/*************************************************************************************************/
uint8_t *DmSecGetLocalIdentityAddr(void);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  This function gets the local identity address type used by the
 *          device.
 *
 *  \return The identity address type.
 */
/*************************************************************************************************/
uint8_t DmSecGetLocalIdentityAddrType(void);

/*************************************************************************************************/
/*!
 *  \brief  For internal use only.  Read the features of the remote device.
 *
 *  \param  connId      Connection identifier.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmReadRemoteFeatures(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  Read the version info of the remote device.
 *
 *  \param  connId      Connection identifier.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmReadRemoteVerInfo(dmConnId_t connId);

/*************************************************************************************************/
/*!
 *  \brief  Disable Slave Latency.
 *
 *  \param  connId      Connection identifier.
 *  \param  disabled    disable/enable
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmDisableSlaveLatency(dmConnId_t connId, bool_t disabled);

/*************************************************************************************************/
/*!
 *  \brief  Over rule Remote Maximum Rx octets.
 *
 *  \param  connId               Connection identifier.
 *  \param  maxRxOctetsRemote    Remote maximum receive octets.
 *  \param  maxRxTimeRemote      Remote maximum Recieve Time.
 *
 *  \return None.
 */
/*************************************************************************************************/
void DmOverruleRemoteMaxRxOctetsAndTime(dmConnId_t connId, uint16_t maxRxOctetsRemote, uint16_t maxRxTimeRemote);

/*************************************************************************************************/
/*!
 *  \brief  Set device address.
 *
 *  \param  pAddr    pointer to the address.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciVsdSetDeviceAddress(uint8_t *pAddr);

/*************************************************************************************************/
/*!
 *  \brief  Set transmit power.
 *
 *  \param  transmitPower    TX Power value.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciVsdSetTransmitPower(int8_t transmitPower);

/*************************************************************************************************/
/*!
 *  \brief  Set event notification bit.
 *
 *  \param  event    bit to receive vsd notifications in the host stack.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciCmndVsdSetLeMetaVSDEvent(uint8_t event);

/*************************************************************************************************/
/*!
 *  \brief  Reset event notification bit.
 *
 *  \param  event    bit to disable vsd notifications in the host stack.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciCmndVsdResetLeMetaVSDEvent(uint8_t event);

/**@}*/

/*! \} */    /* STACK_DM_API */

#ifdef __cplusplus
};
#endif

#endif /* DM_API_H */
