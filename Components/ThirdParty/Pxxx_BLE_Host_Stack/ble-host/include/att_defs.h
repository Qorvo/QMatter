/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  Attribute protocol constants and definitions from the Bluetooth specification.
 *
 *  Copyright (c) 2009-2019 Arm Ltd. All Rights Reserved.
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
#ifndef ATT_DEFS_H
#define ATT_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/*! \addtogroup STACK_ATT_API
 *  \{ */

/**************************************************************************************************
  Macros
**************************************************************************************************/

/** \name ATT PDU Format
 * ATT PDU defaults and constants
 */
/**@{*/
#define ATT_HDR_LEN                   1         /*!< Attribute PDU header length */
#define ATT_AUTH_SIG_LEN              12        /*!< Authentication signature length */
#define ATT_DEFAULT_MTU               23        /*!< Default value of ATT_MTU */
#define ATT_MAX_MTU                   124       /*!< Maximum value of ATT_MTU */
#define ATT_DEFAULT_PAYLOAD_LEN       20        /*!< Default maximum payload length for most PDUs */
/**@}*/

/** \name ATT Maximum Value Parameters
 * maximum values for ATT attribute length and offset
 */
/**@{*/
#define ATT_VALUE_MAX_LEN             512       /*!< Maximum attribute value length */
#define ATT_VALUE_MAX_OFFSET          511       /*!< Maximum attribute value offset */
/**@}*/

/** \name ATT Transaction Timeout
 *  Maximum time allowed between transaction request and response.
 */
/**@{*/
#define ATT_MAX_TRANS_TIMEOUT         30        /*!< Maximum transaction timeout in seconds */
/**@}*/

/** \name ATT Error Codes
 * ATT Protocol operation status codes found in PDUs
 */
/**@{*/
#define ATT_SUCCESS                   0x00      /*!< Operation successful */
#define ATT_ERR_HANDLE                0x01      /*!< Invalid handle */
#define ATT_ERR_READ                  0x02      /*!< Read not permitted */
#define ATT_ERR_WRITE                 0x03      /*!< Write not permitted */
#define ATT_ERR_INVALID_PDU           0x04      /*!< Invalid pdu */
#define ATT_ERR_AUTH                  0x05      /*!< Insufficient authentication */
#define ATT_ERR_NOT_SUP               0x06      /*!< Request not supported */
#define ATT_ERR_OFFSET                0x07      /*!< Invalid offset */
#define ATT_ERR_AUTHOR                0x08      /*!< Insufficient authorization */
#define ATT_ERR_QUEUE_FULL            0x09      /*!< Prepare queue full */
#define ATT_ERR_NOT_FOUND             0x0A      /*!< Attribute not found */
#define ATT_ERR_NOT_LONG              0x0B      /*!< Attribute not long */
#define ATT_ERR_KEY_SIZE              0x0C      /*!< Insufficient encryption key size */
#define ATT_ERR_LENGTH                0x0D      /*!< Invalid attribute value length */
#define ATT_ERR_UNLIKELY              0x0E      /*!< Other unlikely error */
#define ATT_ERR_ENC                   0x0F      /*!< Insufficient encryption */
#define ATT_ERR_GROUP_TYPE            0x10      /*!< Unsupported group type */
#define ATT_ERR_RESOURCES             0x11      /*!< Insufficient resources */
#define ATT_ERR_DATABASE_OUT_OF_SYNC  0x12      /*!< Client out of synch with database */
#define ATT_ERR_VALUE_NOT_ALLOWED     0x13      /*!< Value not allowed */
#define ATT_ERR_WRITE_REJ             0xFC      /*!< Write request rejected */
#define ATT_ERR_CCCD                  0xFD      /*!< CCCD improperly configured */
#define ATT_ERR_IN_PROGRESS           0xFE      /*!< Procedure already in progress */
#define ATT_ERR_RANGE                 0xFF      /*!< Value out of range */
/**@}*/

/** \name Proprietary Internal Error Codes
 * These codes may be sent to application but are not present in any ATT PDU.
 */
/**@{*/
#define ATT_ERR_MEMORY                0x70      /*!< Out of memory */
#define ATT_ERR_TIMEOUT               0x71      /*!< Transaction timeout */
#define ATT_ERR_OVERFLOW              0x72      /*!< Transaction overflow */
#define ATT_ERR_INVALID_RSP           0x73      /*!< Invalid response PDU */
#define ATT_ERR_CANCELLED             0x74      /*!< Request cancelled */
#define ATT_ERR_UNDEFINED             0x75      /*!< Other undefined error */
#define ATT_ERR_REQ_NOT_FOUND         0x76      /*!< Required characteristic not found */
#define ATT_ERR_MTU_EXCEEDED          0x77      /*!< Attribute PDU length exceeded MTU size */
#define ATT_ERR_NO_CHANNEL            0x78      /*!< No enhanced channel available */
#define ATT_CONTINUING                0x79      /*!< Procedure continuing */
#define ATT_RSP_PENDING               0x7A      /*!< Response delayed pending higher layer */
/**@}*/

/** \name ATT Application Error Codes
 * These codes may be sent to application but are not present in any ATT PDU.
 */
/**@{*/
#define ATT_ERR_VALUE_RANGE           0x80      /*!< Value out of range */
/**@}*/

/** \name ATT HCI Error Status
 *
 */
/**@{*/
/*! \brief Base value for HCI error status values passed through ATT.
 * Since the values of HCI and ATT error codes overlap, the constant
 * \ref ATT_HCI_ERR_BASE is added to HCI error codes before being passed through ATT.
 * See \ref HCI_SUCCESS for HCI error code values.
 */
#define ATT_HCI_ERR_BASE              0x20
/**@}*/

/** \name ATT PDU Types
 *  PDU Types for all possible over-the-air ATT operations.
 */
/**@{*/
#define ATT_PDU_ERR_RSP               0x01      /*!< Error response */
#define ATT_PDU_MTU_REQ               0x02      /*!< Exchange mtu request */
#define ATT_PDU_MTU_RSP               0x03      /*!< Exchange mtu response */
#define ATT_PDU_FIND_INFO_REQ         0x04      /*!< Find information request */
#define ATT_PDU_FIND_INFO_RSP         0x05      /*!< Find information response */
#define ATT_PDU_FIND_TYPE_REQ         0x06      /*!< Find by type value request */
#define ATT_PDU_FIND_TYPE_RSP         0x07      /*!< Find by type value response */
#define ATT_PDU_READ_TYPE_REQ         0x08      /*!< Read by type request */
#define ATT_PDU_READ_TYPE_RSP         0x09      /*!< Read by type response */
#define ATT_PDU_READ_REQ              0x0A      /*!< Read request */
#define ATT_PDU_READ_RSP              0x0B      /*!< Read response */
#define ATT_PDU_READ_BLOB_REQ         0x0C      /*!< Read blob request */
#define ATT_PDU_READ_BLOB_RSP         0x0D      /*!< Read blob response */
#define ATT_PDU_READ_MULT_REQ         0x0E      /*!< Read multiple request */
#define ATT_PDU_READ_MULT_RSP         0x0F      /*!< Read multiple response */
#define ATT_PDU_READ_GROUP_TYPE_REQ   0x10      /*!< Read by group type request */
#define ATT_PDU_READ_GROUP_TYPE_RSP   0x11      /*!< Read by group type response */
#define ATT_PDU_WRITE_REQ             0x12      /*!< Write request */
#define ATT_PDU_WRITE_RSP             0x13      /*!< Write response */
#define ATT_PDU_WRITE_CMD             0x52      /*!< Write command */
#define ATT_PDU_SIGNED_WRITE_CMD      0xD2      /*!< Signed write command */
#define ATT_PDU_PREP_WRITE_REQ        0x16      /*!< Prepare write request */
#define ATT_PDU_PREP_WRITE_RSP        0x17      /*!< Prepare write response */
#define ATT_PDU_EXEC_WRITE_REQ        0x18      /*!< Execute write request */
#define ATT_PDU_EXEC_WRITE_RSP        0x19      /*!< Execute write response */
#define ATT_PDU_VALUE_NTF             0x1B      /*!< Handle value notification */
#define ATT_PDU_VALUE_IND             0x1D      /*!< Handle value indication */
#define ATT_PDU_VALUE_CNF             0x1E      /*!< Handle value confirmation */
#define ATT_PDU_READ_MULT_VAR_REQ     0x20      /*!< Read multiple variable length request */
#define ATT_PDU_READ_MULT_VAR_RSP     0x21      /*!< Read multiple variable length response */
#define ATT_PDU_MULT_VALUE_NTF        0x23      /*!< Handle value multiple notification */
/**@}*/

/** \name ATT PDU Length Fields
 * Length constants of PDU fixed length fields
 */
/**@{*/
#define ATT_ERR_RSP_LEN               5 /*!< Error response length. */
#define ATT_MTU_REQ_LEN               3 /*!< MTU request length. */
#define ATT_MTU_RSP_LEN               3 /*!< MTU response length. */
#define ATT_FIND_INFO_REQ_LEN         5 /*!< Find information request length. */
#define ATT_FIND_INFO_RSP_LEN         2 /*!< Find information response length. */
#define ATT_FIND_TYPE_REQ_LEN         7 /*!< Find type request length. */
#define ATT_FIND_TYPE_RSP_LEN         1 /*!< Find type response length. */
#define ATT_READ_TYPE_REQ_LEN         5 /*!< Read type request length. */
#define ATT_READ_TYPE_RSP_LEN         2 /*!< Read type response length. */
#define ATT_READ_REQ_LEN              3 /*!< Read request length. */
#define ATT_READ_RSP_LEN              1 /*!< Read response length. */
#define ATT_READ_BLOB_REQ_LEN         5 /*!< Read blob request legnth. */
#define ATT_READ_BLOB_RSP_LEN         1 /*!< Read blob response length. */
#define ATT_READ_MULT_REQ_LEN         1 /*!< Read multiple request length. */
#define ATT_READ_MULT_RSP_LEN         1 /*!< Read multiple response length. */
#define ATT_READ_GROUP_TYPE_REQ_LEN   5 /*!< Read group type request length. */
#define ATT_READ_GROUP_TYPE_RSP_LEN   2 /*!< Read group type response length. */
#define ATT_WRITE_REQ_LEN             3 /*!< Write request length. */
#define ATT_WRITE_RSP_LEN             1 /*!< Write response length. */
#define ATT_WRITE_CMD_LEN             3 /*!< Write command length. */
#define ATT_SIGNED_WRITE_CMD_LEN      (ATT_WRITE_CMD_LEN + ATT_AUTH_SIG_LEN) /*!< Signed write command length. */
#define ATT_PREP_WRITE_REQ_LEN        5 /*!< Prepared write command length. */
#define ATT_PREP_WRITE_RSP_LEN        5 /*!< Prepared write response length. */
#define ATT_EXEC_WRITE_REQ_LEN        2 /*!< Execute write request length. */
#define ATT_EXEC_WRITE_RSP_LEN        1 /*!< Execute write response length. */
#define ATT_VALUE_NTF_LEN             3 /*!< Value notification length. */
#define ATT_VALUE_IND_LEN             3 /*!< Value indication length. */
#define ATT_VALUE_CNF_LEN             1 /*!< Value confirmation length. */
#define ATT_READ_MULT_VAR_REQ_LEN     1 /*!< Base read multiple variable request length. */
#define ATT_READ_MULT_VAR_RSP_LEN     1 /*!< Base read multiple variable response length. */
#define ATT_PDU_MULT_VALUE_NTF_LEN    1 /*!< Base multiple variable notification length. */
/**@}*/

/** \name ATT Find Information Response Format
 *
 */
/**@{*/
#define ATT_FIND_HANDLE_16_UUID       0x01      /*!< Handle and 16 bit UUID */
#define ATT_FIND_HANDLE_128_UUID      0x02      /*!< Handle and 128 bit UUID */
/**@}*/

/** \name ATT Execute Write Request Flags
 *
 */
/**@{*/
#define ATT_EXEC_WRITE_CANCEL         0x00      /*!< Cancel all prepared writes */
#define ATT_EXEC_WRITE_ALL            0x01      /*!< Write all pending prepared writes */
/**@}*/

/** \name ATT PDU Masks
 *
 */
/**@{*/
#define ATT_PDU_MASK_SERVER           0x01      /*!< Server bit mask */
#define ATT_PDU_MASK_COMMAND          0x40      /*!< Command bit mask */
#define ATT_PDU_MASK_SIGNED           0x80      /*!< Auth signature bit mask */
/**@}*/

/** \name ATT Handle Constants
 * Invalid, minimum and maximum handle values.
 */
/**@{*/
#define ATT_HANDLE_NONE               0x0000    /*!< Handle none. */
#define ATT_HANDLE_START              0x0001    /*!< Handle start. */
#define ATT_HANDLE_MAX                0xFFFF    /*!< Handle max. */
/**@}*/

/** \name ATT UUID Lengths
 *
 */
/**@{*/
#define ATT_NO_UUID_LEN               0         /*!< Length when no UUID is present ;-) */
#define ATT_16_UUID_LEN               2         /*!< Length in bytes of a 16 bit UUID */
#define ATT_128_UUID_LEN              16        /*!< Length in bytes of a 128 bit UUID */
/**@}*/

/** \name GATT Characteristic Properties
 * Properties for how a characteristic may be interacted with through the ATT Protocol.
 */
/**@{*/
#define ATT_PROP_BROADCAST            0x01      /*!< Permit broadcasts */
#define ATT_PROP_READ                 0x02      /*!< Permit reads */
#define ATT_PROP_WRITE_NO_RSP         0x04      /*!< Permit writes without response */
#define ATT_PROP_WRITE                0x08      /*!< Permit writes with response */
#define ATT_PROP_NOTIFY               0x10      /*!< Permit notifications */
#define ATT_PROP_INDICATE             0x20      /*!< Permit indications */
#define ATT_PROP_AUTHENTICATED        0x40      /*!< Permit signed writes */
#define ATT_PROP_EXTENDED             0x80      /*!< More properties defined in extended properties */
/**@}*/

/** \name GATT Characteristic Extended Properties
 *
 */
/**@{*/
#define ATT_EXT_PROP_RELIABLE_WRITE   0x0001    /*!< Permit reliable writes */
#define ATT_EXT_PROP_WRITEABLE_AUX    0x0002    /*!< Permit write to characteristic descriptor */
/**@}*/

/** \name GATT Client Charactertic Configuration
 * Configures a characteristic to send notifications or indications, if applicable.
 */
/**@{*/
#define ATT_CLIENT_CFG_NOTIFY         0x0001    /*!< Notify the value */
#define ATT_CLIENT_CFG_INDICATE       0x0002    /*!< Indicate the value */
/**@}*/

/** \name GATT Server Characteristic Configuration
 *
 */
/**@{*/
#define ATT_SERVER_CFG_BROADCAST      0x0001    /*!< Broadcast the value */
/**@}*/

/** \name GATT Characteristic Format
 * GATT Format descriptor values
 */
/**@{*/
#define ATT_FORMAT_BOOLEAN            0x01      /*!< Boolean */
#define ATT_FORMAT_2BIT               0x02      /*!< Unsigned 2 bit integer */
#define ATT_FORMAT_NIBBLE             0x03      /*!< Unsigned 4 bit integer */
#define ATT_FORMAT_UINT8              0x04      /*!< Unsigned 8 bit integer */
#define ATT_FORMAT_UINT12             0x05      /*!< Unsigned 12 bit integer */
#define ATT_FORMAT_UINT16             0x06      /*!< Unsigned 16 bit integer */
#define ATT_FORMAT_UINT24             0x07      /*!< Unsigned 24 bit integer */
#define ATT_FORMAT_UINT32             0x08      /*!< Unsigned 32 bit integer */
#define ATT_FORMAT_UINT48             0x09      /*!< Unsigned 48 bit integer */
#define ATT_FORMAT_UINT64             0x0A      /*!< Unsigned 64 bit integer */
#define ATT_FORMAT_UINT128            0x0B      /*!< Unsigned 128 bit integer */
#define ATT_FORMAT_SINT8              0x0C      /*!< Signed 8 bit integer */
#define ATT_FORMAT_SINT12             0x0D      /*!< Signed 12 bit integer */
#define ATT_FORMAT_SINT16             0x0E      /*!< Signed 16 bit integer */
#define ATT_FORMAT_SINT24             0x0F      /*!< Signed 24 bit integer */
#define ATT_FORMAT_SINT32             0x10      /*!< Signed 32 bit integer */
#define ATT_FORMAT_SINT48             0x11      /*!< Signed 48 bit integer */
#define ATT_FORMAT_SINT64             0x12      /*!< Signed 64 bit integer */
#define ATT_FORMAT_SINT128            0x13      /*!< Signed 128 bit integer */
#define ATT_FORMAT_FLOAT32            0x14      /*!< IEEE-754 32 bit floating point */
#define ATT_FORMAT_FLOAT64            0x15      /*!< IEEE-754 64 bit floating point */
#define ATT_FORMAT_SFLOAT             0x16      /*!< IEEE-11073 16 bit SFLOAT */
#define ATT_FORMAT_FLOAT              0x17      /*!< IEEE-11073 32 bit FLOAT */
#define ATT_FORMAT_DUINT16            0x18      /*!< IEEE-20601 format */
#define ATT_FORMAT_UTF8               0x19      /*!< UTF-8 string */
#define ATT_FORMAT_UTF16              0x1A      /*!< UTF-16 string */
#define ATT_FORMAT_STRUCT             0x1B      /*!< Opaque structure */
/**@}*/

/** \name GATT Database Hash
* GATT database hash values
*/
/**@{*/
#define ATT_DATABASE_HASH_LEN         16        /*!< Database hash length. */
/**@}*/

/** \name GATT Client Supported Features
* Flags of features supported by the GATT Client
*/
/**@{*/
#define ATTS_CSF_ROBUST_CACHING      (1<<0)                  /*!< Robust caching. */
#define ATTS_CSF_EATT_BEARER         (1<<1)                  /*!< Enhanced ATT Bearer. */
#define ATTS_CSF_MULTI_VAL_NTF       (1<<2)                  /*!< Multiple Handle Value Notifications. */

#define ATTS_CSF_ALL_FEATURES        (0x7)                   /*!< Mask of all client supported features. */

#define ATT_CSF_LEN                  1                       /*!< Length of client supported features array. */
/**@}*/

/** \name GATT Server Supported Features
* Flags of features supported by the GATT Server
*/
/**@{*/
#define ATTS_SSF_EATT                (1<<0)                  /*!< Enhanced ATT supported. */
/**@}*/

/*! \} */    /* STACK_ATT_API */

#ifdef __cplusplus
};
#endif

#endif /* ATT_DEFS_H */
