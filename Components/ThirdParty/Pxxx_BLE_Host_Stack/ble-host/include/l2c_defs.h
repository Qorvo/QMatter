/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  L2CAP constants and definitions from the Bluetooth specification.
 *
 *  Copyright (c) 2009-2018 Arm Ltd. All Rights Reserved.
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
/*************************************************************************************************/
#ifndef L2C_DEFS_H
#define L2C_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/*! \addtogroup STACK_L2CAP_API
 *  \{ */

/**************************************************************************************************
  Macros
**************************************************************************************************/

/** \name L2CAP Packet Constants
 *
 */
/**@{*/
#define L2C_HDR_LEN                   4         /*!< L2CAP packet header length */
#define L2C_MIN_MTU                   23        /*!< Minimum packet payload MTU for LE */
#define L2C_SIG_HDR_LEN               4         /*!< L2CAP signaling command header length */
#define L2C_LE_SDU_HDR_LEN            2         /*!< L2CAP LE SDU data header length */
/**@}*/

/*! \brief Max number of channels per enhanced connection request */
#define L2C_MAX_EN_CHAN               5

/*! \brief Start of L2CAP payload in an HCI ACL packet buffer */
#define L2C_PAYLOAD_START             (HCI_ACL_HDR_LEN + L2C_HDR_LEN)

/*! \brief L2CAP signaling packet base length, including HCI header */
#define L2C_SIG_PKT_BASE_LEN          (HCI_ACL_HDR_LEN + L2C_HDR_LEN + L2C_SIG_HDR_LEN)

/*! \brief L2CAP LE SDU packet base length, including HCI header */
#define L2C_LE_SDU_PKT_BASE_LEN       (HCI_ACL_HDR_LEN + L2C_HDR_LEN + L2C_LE_SDU_HDR_LEN)

/** \name L2CAP Parameter Lengths
 * Signaling packet parameter lengths
 */
/**@{*/
#define L2C_SIG_CONN_UPDATE_REQ_LEN   8   /*!< Connection update request length. */
#define L2C_SIG_CONN_UPDATE_RSP_LEN   2   /*!< Connection update response length. */
#define L2C_SIG_CMD_REJ_LEN           2   /*!< Command reject length. */
#define L2C_SIG_DISCONN_REQ_LEN       4   /*!< Disconnection request length. */
#define L2C_SIG_DISCONN_RSP_LEN       4   /*!< Disconnection response length. */
#define L2C_SIG_LE_CONN_REQ_LEN       10  /*!< LE connection request length. */
#define L2C_SIG_LE_CONN_RSP_LEN       10  /*!< LE connection response length. */
#define L2C_SIG_FLOW_CTRL_CREDIT_LEN  4   /*!< Flow control credit lenghth. */
#define L2C_SIG_EN_CONNECT_REQ_LEN    8   /*!< Enhanced credit based connection request */
#define L2C_SIG_EN_CONNECT_RSP_LEN    8   /*!< Enhanced credit based connection response */
#define L2C_SIG_EN_RECONFIG_REQ_LEN   4   /*!< Enhanced credit based reconfiguration request */
#define L2C_SIG_EN_RECONFIG_RSP_LEN   2   /*!< Enhanced credit based reconfiguration response */
/**@}*/

/** \name L2CAP Connection Identifiers
 * BLE Defined Connection Identifiers (CID)
 */
/**@{*/
#define L2C_CID_ATT                   0x0004    /*!< CID for attribute protocol */
#define L2C_CID_LE_SIGNALING          0x0005    /*!< CID for LE signaling */
#define L2C_CID_SMP                   0x0006    /*!< CID for security manager protocol */
/**@}*/

/** \name L2CAP Signaling Codes
 *
 */
/**@{*/
#define L2C_SIG_CMD_REJ               0x01      /*!< Comand reject */
#define L2C_SIG_DISCONNECT_REQ        0x06      /*!< Disconnect request */
#define L2C_SIG_DISCONNECT_RSP        0x07      /*!< Disconnect response */
#define L2C_SIG_CONN_UPDATE_REQ       0x12      /*!< Connection parameter update request */
#define L2C_SIG_CONN_UPDATE_RSP       0x13      /*!< Connection parameter update response */
#define L2C_SIG_LE_CONNECT_REQ        0x14      /*!< LE credit based connection request */
#define L2C_SIG_LE_CONNECT_RSP        0x15      /*!< LE credit based connection response */
#define L2C_SIG_FLOW_CTRL_CREDIT      0x16      /*!< LE flow control credit */
#define L2C_SIG_EN_CONNECT_REQ        0x17      /*!< Enhanced credit based connection request */
#define L2C_SIG_EN_CONNECT_RSP        0x18      /*!< Enhanced credit based connection response */
#define L2C_SIG_EN_RECONFIG_REQ       0x19      /*!< Enhanced credit based reconfiguration request */
#define L2C_SIG_EN_RECONFIG_RSP       0x1A      /*!< Enhanced credit based reconfiguration response */
/**@}*/

/*! \brief Signaling response code flag */
#define L2C_SIG_RSP_FLAG              0x01

/** \name L2CAP Command Rejection Codes
 * BLE defined Command rejection reason codes
 */
/**@{*/
#define L2C_REJ_NOT_UNDERSTOOD        0x0000    /*!< Command not understood */
#define L2C_REJ_MTU_EXCEEDED          0x0001    /*!< Signaling MTU exceeded */
#define L2C_REJ_INVALID_CID           0x0002    /*!< Invalid CID in request */
/**@}*/

/** \name L2CAP Connection Parameter Update Result Codes
 * BLE defined result codes
 */
/**@{*/
#define L2C_CONN_PARAM_ACCEPTED       0x0000    /*!< Connection parameters accepted */
#define L2C_CONN_PARAM_REJECTED       0x0001    /*!< Connection parameters rejected */
/**@}*/

/** \name L2CAP Connection Result Codes
 * BLE defined result codes
 */
/**@{*/
#define L2C_CONN_SUCCESS              0x0000    /*!< Connection successful */
#define L2C_CONN_NONE                 0x0001    /*!< No connection result value available */
#define L2C_CONN_FAIL_PSM             0x0002    /*!< Connection refused LE_PSM not supported */
#define L2C_CONN_FAIL_RES             0x0004    /*!< Connection refused no resources available */
#define L2C_CONN_FAIL_AUTH            0x0005    /*!< Connection refused insufficient authentication */
#define L2C_CONN_FAIL_AUTHORIZ        0x0006    /*!< Connection refused insufficient authorization */
#define L2C_CONN_FAIL_KEY_SIZE        0x0007    /*!< Connection refused insufficient encryption key size */
#define L2C_CONN_FAIL_ENC             0x0008    /*!< Connection Refused insufficient encryption */
#define L2C_CONN_FAIL_INVALID_SCID    0x0009    /*!< Connection refused invalid source CID */
#define L2C_CONN_FAIL_ALLOCATED_SCID  0x000A    /*!< Connection refused source CID already allocated */
#define L2C_CONN_FAIL_UNACCEPT_PARAM  0x000B    /*!< Connection refused unacceptable parameters */
#define L2C_CONN_FAIL_INVALID_PARAM   0x000C    /*!< Connection refused invalid parameters */
/**@}*/

/** \name L2CAP Interal Connection Result Codes
 * Proprietary codes not sent in any L2CAP packet.
 */
/**@{*/
#define L2C_CONN_FAIL_TIMEOUT         0xF000    /*!< Request timeout */
/**@}*/

/** \name L2CAP Signaling Parameter Value Ranges
 *
 */
/**@{*/
#define L2C_PSM_MIN                   0x0001  /*!< PSM minimum. */
#define L2C_PSM_MAX                   0x00FF  /*!< PSM maximum. */
#define L2C_CID_DYN_MIN               0x0040  /*!< CID dynamic minimum. */
#define L2C_CID_DYN_MAX               0x007F  /*!< CID dynamic maximum. */
#define L2C_MTU_MIN                   0x0017  /*!< MTU minimum. */
#define L2C_MPS_MIN                   0x0017  /*!< MPS minimum. */
#define L2C_MPS_MAX                   0xFFFD  /*!< MPS maximum. */
#define L2C_CREDITS_MAX               0xFFFF  /*!< Credits maximum. */
/**@}*/

/** \name L2CAP Enhanced Connection Reconfigure Result Codes
 *
 */
/**@{*/
#define L2C_RECONFIG_FAIL_MTU         0x0001  /*!< Enhanced Reconfiguration refuded - cannot reduce MTU. */
#define L2C_RECONFIG_FAIL_MPS         0x0002  /*!< Enhanced Reconfiguration refuded - cannot reduce MPS on more than one channel. */
#define L2C_RECONFIG_FAIL_CID         0x0003  /*!< Enhanced Reconfiguration refuded - invalid CID. */
#define L2C_RECONFIG_FAIL_PARAM       0x0004  /*!< Enhanced Reconfiguration refuded - unacceptable parameters. */
/**@}*/

/*! \} */    /*! STACK_L2CAP_API */

#ifdef __cplusplus
};
#endif

#endif /* L2C_DEFS_H */
