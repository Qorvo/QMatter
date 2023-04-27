/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  Service configuration.
 *
 *  Copyright (c) 2011-2017 Arm Ltd. All Rights Reserved.
 *
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

#ifndef SVC_CFG_H
#define SVC_CFG_H

#include "gpVersion.h"


#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! Default read security permissions for service characteristics */
#ifndef SVC_SEC_PERMIT_READ
#define SVC_SEC_PERMIT_READ ATTS_PERMIT_READ
#endif

/*! Default write security permissions for service characteristics */
/* See Bluetooth Security and Privacy Best Practices Guide (r02) - Section 4.2 - Advise is
 * to make all writeable characteristics protected with the encryption and/or authentication flag
 * individual properties of characteristics can be overruled within the characteristics definition as well
 */
#ifndef SVC_SEC_PERMIT_WRITE
#define SVC_SEC_PERMIT_WRITE (ATTS_PERMIT_WRITE | ATTS_PERMIT_WRITE_ENC)
#endif

/*! Default device name */
/*Obs: Make this the same as DEFAULT_MRF_NAME for consistency in display on vsarious central devices */
#define DEFAULT_SCAN_NAME       'Q','o','r','v','o',' ','I','n','c','.'
/*! Length of default device name */
#define DEFAULT_SCAN_NAME_LEN       10

/*! Default device name */
#define DEFAULT_DEVICE_NAME       "Qorvo Inc."
/*! Length of default device name */
#define DEFAULT_DEVICE_NAME_LEN   11

/*! Default manufacturer name */
#define DEFAULT_MFR_NAME        "QORVO Inc."

/*! Length of default manufacturer name */
#define DEFAULT_MFR_NAME_LEN    11

/*! Default model number */
#define DEFAULT_MODEL_NUM       "Qorvo model num"

/*! Length of default model number */
#define DEFAULT_MODEL_NUM_LEN   16

/*! Default serial number */
#define DEFAULT_SERIAL_NUM      "Qorvo serial num"

/*! Length of default serial number */
#define DEFAULT_SERIAL_NUM_LEN  17

/*! Default firmware revision */
#define DEFAULT_FW_REV          "Qorvo fw rev"

/*! Length of default firmware revision */
#define DEFAULT_FW_REV_LEN      13

/*! Default hardware revision */
#define DEFAULT_HW_REV          "Qorvo hw rev"

/*! Length of default hardware revision */
#define DEFAULT_HW_REV_LEN      13

/*! Default software revision */
#define DEFAULT_SW_REV          GP_VERSION_STRING

/*! Length of default software revision */
#define DEFAULT_SW_REV_LEN      sizeof(DEFAULT_SW_REV)

/*! PnP ID values */
#define DEFAULT_VENDOR_ID_SRC   0x01    /* Bluetooth SIG - assigned Device ID Vendor ID value from Assigned Numbers document */
#define DEFAULT_VENDOR_ID       HCI_ID_QORVO
#define DEFAULT_PRODUCT_ID      0x1234
#define DEFAULT_PRODUCT_VERSION 0x2030 /* 0xJJMN for version JJ.M.N */


/*! Default connection parameters  */
#define DEFAULT_CONN_INTERVAL_MIN   0x0010
#define DEFAULT_CONN_INTERVAL_MAX   0x0010
#define DEFAULT_CONN_LATENCY        0x0031
#define DEFAULT_SUPERVISION_TIMEOUT 0x01F4

#ifdef __cplusplus
};
#endif

#endif /* SVC_CFG_H */
