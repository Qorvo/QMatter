/*!
 *  \file   wdx_defs.h
 *
 *  \brief  Wireless Data Exchange Protocol Definitions.
 *
 *  Copyright (c) 2013-2017 ARM Ltd. All Rights Reserved.
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

#ifndef WDX_DEFS_H
#define WDX_DEFS_H

#ifdef __cplusplus
extern "C"
{
#endif

/**************************************************************************************************
  Macros
**************************************************************************************************/

/* Base UUID:  005fXXXX-2ff2-4ed5-b045-4C7463617865 */
#define WDX_UUID_PART1                0x65, 0x78, 0x61, 0x63, 0x74, 0x4c, 0x45, 0xb0, \
                                      0xd5, 0x4e, 0xf2, 0x2f
#define WDX_UUID_PART2                0x5f, 0x00

/* Macro for building UUIDs */
#define WDX_UUID_BUILD(part)          WDX_UUID_PART1, UINT16_TO_BYTES(part), WDX_UUID_PART2

/* WDX Service */
#define WDX_SVC_UUID                  0xFEF6

/* WDX Device Configuration Characteristic */
#define WDX_DC_UUID                   WDX_UUID_BUILD(0x0002)

/* WDX File Transfer Control Characteristic */
#define WDX_FTC_UUID                  WDX_UUID_BUILD(0x0003)

/* WDX File Transfer Data Characteristic */
#define WDX_FTD_UUID                  WDX_UUID_BUILD(0x0004)

/* WDX Authentication Characteristic */
#define WDX_AU_UUID                   WDX_UUID_BUILD(0x0005)


/**************************************************************************************************
  Constant Definitions
**************************************************************************************************/

/*! WDXS File List Configuration */
#define WDX_FLIST_HANDLE               0             /*! File List handle */
#define WDX_FLIST_FORMAT_VER           1             /*! File List version */
#define WDX_FLIST_HDR_SIZE             7             /*! File List header length */
#define WDX_FLIST_RECORD_SIZE          40            /*! File List record length */
#define WDX_FLIST_MAX_LEN              (WDX_FLIST_HDR_SIZE + (WDX_FLIST_RECORD_SIZE * (WSF_EFS_MAX_FILES-1)))

/*! Device configuration characteristic message header length */
#define WDX_DC_HDR_LEN                 2

/*! Device configuration characteristic operations */
#define WDX_DC_OP_GET                  0x01         /*! Get a parameter value */
#define WDX_DC_OP_SET                  0x02         /*! Set a parameter value */
#define WDX_DC_OP_UPDATE               0x03         /*! Send an update of a parameter value */

/*! Device control characteristic parameter IDs */
#define WDX_DC_ID_CONN_UPDATE_REQ      0x01         /*! Connection Parameter Update Request */
#define WDX_DC_ID_CONN_PARAM           0x02         /*! Current Connection Parameters */
#define WDX_DC_ID_DISCONNECT_REQ       0x03         /*! Disconnect Request */
#define WDX_DC_ID_CONN_SEC_LEVEL       0x04         /*! Connection Security Level */
#define WDX_DC_ID_SECURITY_REQ         0x05         /*! Security Request */
#define WDX_DC_ID_SERVICE_CHANGED      0x06         /*! Service Changed */
#define WDX_DC_ID_DELETE_BONDS         0x07         /*! Delete Bonds */
#define WDX_DC_ID_ATT_MTU              0x08         /*! Current ATT MTU */
#define WDX_DC_ID_BATTERY_LEVEL        0x20         /*! Battery level */
#define WDX_DC_ID_MODEL_NUMBER         0x21         /*! Device Model */
#define WDX_DC_ID_FIRMWARE_REV         0x22         /*! Device Firmware Revision */
#define WDX_DC_ID_ENTER_DIAGNOSTICS    0x23         /*! Enter Diagnostic Mode */
#define WDX_DC_ID_DIAGNOSTICS_COMPLETE 0x24         /*! Diagnostic Complete */
#define WDX_DC_ID_DISCONNECT_AND_RESET 0x25         /*! Disconnect and Reset */

/*! Device control parameter lengths */
#define WDX_DC_LEN_DATA_FORMAT         1            /*! Data format */
#define WDX_DC_LEN_SEC_LEVEL           1            /*! Security Level */
#define WDX_DC_LEN_ATT_MTU             2            /*! ATT MTU */
#define WDX_DC_LEN_BATTERY_LEVEL       1            /*! Battery level */
#define WDX_DC_LEN_CONN_PARAM_REQ      8            /*! Connection parameter request */
#define WDX_DC_LEN_CONN_PARAM          7            /*! Current connection parameters */
#define WDX_DC_LEN_DIAG_COMPLETE       0            /*! Diagnostic complete */
#define WDX_DC_LEN_DEVICE_MODEL        18           /*! Device Model */
#define WDX_DC_LEN_FIRMWARE_REV        16           /*! Firmware Revision */

/*! File transfer control characteristic message header length */
#define WDX_FTC_HDR_LEN                1
#define WDX_FTC_HANDLE_LEN             2

/*! File transfer control characteristic operations */
#define WDX_FTC_OP_NONE                0x00        /*! No operation */
#define WDX_FTC_OP_GET_REQ             0x01        /*! Get a file from the server */
#define WDX_FTC_OP_GET_RSP             0x02        /*! File get response */
#define WDX_FTC_OP_PUT_REQ             0x03        /*! Put a file to the server */
#define WDX_FTC_OP_PUT_RSP             0x04        /*! File put response */
#define WDX_FTC_OP_ERASE_REQ           0x05        /*! Erase a file on the server */
#define WDX_FTC_OP_ERASE_RSP           0x06        /*! File erase response */
#define WDX_FTC_OP_VERIFY_REQ          0x07        /*! Verify a file (e.g. check its CRC) */
#define WDX_FTC_OP_VERIFY_RSP          0x08        /*! File verify response */
#define WDX_FTC_OP_ABORT               0x09        /*! Abort a get, put, or list operation in progress */
#define WDX_FTC_OP_EOF                 0x0a        /*! End of file reached */

/*! File transfer control permissions */
#define WDX_FTC_GET_PERMITTED          0x01        /*! File Get Permitted */
#define WDX_FTC_PUT_PERMITTED          0x02        /*! File Put Permitted */
#define WDX_FTC_ERASE_PERMITTED        0x04        /*! File Erase Permitted */
#define WDX_FTC_VERIFY_PERMITTED       0x08        /*! File Verify Permitted */

/*! File transfer control characteristic status */
#define WDX_FTC_ST_SUCCESS             0           /*! Success */
#define WDX_FTC_ST_INVALID_OP_FILE     1           /*! Invalid operation for this file */
#define WDX_FTC_ST_INVALID_HANDLE      2           /*! Invalid file handle */
#define WDX_FTC_ST_INVALID_OP_DATA     3           /*! Invalid operation data */
#define WDX_FTC_ST_IN_PROGRESS         4           /*! Operation in progress */
#define WDX_FTC_ST_VERIFICATION        5           /*! Verification failure */

/*! File transfer control transport */
#define WDX_FTC_TRANSPORT_TYPE         0           /*! Transport Type */
#define WDX_FTC_TRANSPORT_ID           0x0030      /*! Transport ID */

/*! File transfer data characteristic message header length */
#define WDX_FTD_HDR_LEN                0

/*! Authentication message header length */
#define WDX_AU_HDR_LEN                 1

/*! Authentication characteristic operations */
#define WDX_AU_OP_START                0x01        /*! Authentication start */
#define WDX_AU_OP_CHALLENGE            0x02        /*! Authentication challenge */
#define WDX_AU_OP_REPLY                0x03        /*! Authentication reply */

/*! Proprietary ATT error codes */
#define WDX_APP_AUTH_REQUIRED          0x80        /*! Application authentication required */
#define WDX_AU_ST_INVALID_MESSAGE      0x81        /*! Authentication invalid message */
#define WDX_AU_ST_INVALID_STATE        0x82        /*! Authentication invalid state */
#define WDX_AU_ST_AUTH_FAILED          0x83        /*! Authentication failed */

/*! Authentication characteristic authentication level  */
#define WDX_AU_LVL_NONE                0x00        /*! None */
#define WDX_AU_LVL_USER                0x01        /*! User level */
#define WDX_AU_LVL_MAINT               0x02        /*! Maintenance level */
#define WDX_AU_LVL_DEBUG               0x03        /*! Debug level */

/*! Authenttication characteristic message parameter lengths */
#define WDX_AU_MSG_HDR_LEN             1           /*! Message header length */
#define WDX_AU_PARAM_LEN_START         2           /*! Authentication start */
#define WDX_AU_PARAM_LEN_CHALLENGE     16          /*! Authentication challenge */
#define WDX_AU_PARAM_LEN_REPLY         8           /*! Authentication reply */

/*! Authenttication characteristic random number and key lengths */
#define WDX_AU_RAND_LEN                16          /*! Authentication Random challenge length (bytes)*/
#define WDX_AU_KEY_LEN                 16          /*! Authentication Key length (bytes) */
#define WDX_AU_HASH_LEN                8           /*! Authentication Hash length (bytes) */

/* WDXS Media Types */
#define WDX_FLASH_MEDIA                0
#define WDX_OTA_MEDIA                  1
#define WDX_RAM_MEDIA                  2
#define WDX_STREAM_MEDIA               3

/* WDXS File Transfer Control Command Message Lengths */
#define WDX_FTC_ABORT_LEN              3
#define WDX_FTC_ERASE_LEN              3
#define WDX_FTC_VERIFY_LEN             3
#define WDX_FTC_PUT_LEN                16
#define WDX_FTC_GET_LEN                12


#ifdef __cplusplus
}
#endif

#endif /* WDX_DEFS_H */
