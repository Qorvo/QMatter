/*!
 *  \file
 *
 *  \brief  Example GATT and GAP service implementations.
 *
 *  Copyright (c) 2009-2019 Arm Ltd. All Rights Reserved.
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

#ifndef SVC_CORE_H
#define SVC_CORE_H


#ifdef __cplusplus
extern "C" {
#endif

/*! \addtogroup GATT_AND_GAP_SERVICE
 *  \{ */


/**************************************************************************************************
 Handle Ranges
**************************************************************************************************/
/** \name GAP Service Handles
 */
/**@{*/
#define GAP_START_HDL               0x01               /*!< \brief GAP start handle */
#define GAP_END_HDL                 (GAP_MAX_HDL - 1)  /*!< \brief GAP end handle */
/**@}*/

/** \name GATT Service Handles
 *
 */
/**@{*/
#define GATT_START_HDL              0x10                /*!< \brief GATT start handle */
#define GATT_END_HDL                (GATT_MAX_HDL - 1)  /*!< \brief GATT end handle */
/**@}*/

/**************************************************************************************************
 Handles
**************************************************************************************************/

/** \name GAP Service Handles
 *
 */
/**@{*/
/*! \brief GAP service handle */
enum
{
  GAP_SVC_HDL = GAP_START_HDL,      /*!< \brief GAP service declaration */
  GAP_DN_CH_HDL,                    /*!< \brief Device name characteristic */
  GAP_DN_HDL,                       /*!< \brief Device name */
  GAP_AP_CH_HDL,                    /*!< \brief Appearance characteristic */
  GAP_AP_HDL,                       /*!< \brief Appearance */
  GAP_PPCP_CH_HDL,                  /*!< \brief Peripheral Preferred Connection Parameters characteristic */
  GAP_PPCP_HDL,                     /*!< \brief Peripheral Preferred Connection Parameters */
  GAP_CAR_CH_HDL,                   /*!< \brief Central address resolution characteristic */
  GAP_CAR_HDL,                      /*!< \brief Central address resolution */
#ifdef GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED
  GAP_RPAO_CH_HDL,                  /*!< \brief Resolvable private address only characteristic */
  GAP_RPAO_HDL,                     /*!< \brief Resolvable private address only */
#endif // GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED
  GAP_MAX_HDL                       /*!< \brief GAP maximum handle */
};
/**@}*/

/** \name GATT Service Handles
 *
 */
/**@{*/
/*! \brief GATT service handles */
enum
{
  GATT_SVC_HDL = GATT_START_HDL,    /*!< \brief GATT service declaration */
  GATT_SC_CH_HDL,                   /*!< \brief Service changed characteristic */
  GATT_SC_HDL,                      /*!< \brief Service changed */
  GATT_SC_CH_CCC_HDL,               /*!< \brief Service changed client characteristic configuration descriptor */
  GATT_CSF_CH_HDL,                  /*!< \brief Client supported features characteristic */
  GATT_CSF_HDL,                     /*!< \brief Client supported features */
#ifdef GP_BLE_ATT_SERVER_DATABASE_HASH
  GATT_DBH_CH_HDL,                  /*!< \brief Database hash characteristic */
  GATT_DBH_HDL,                     /*!< \brief Database hash */
#endif
  GATT_MAX_HDL                      /*!< \brief GATT maximum handle */
};
/**@}*/

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \brief  Add the services to the attribute server.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcCoreAddGroup(void);

/*************************************************************************************************/
/*!
 *  \brief  Remove the services from the attribute server.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcCoreRemoveGroup(void);

/*************************************************************************************************/
/*!
 *  \brief  Register callbacks for the service.
 *
 *  \param  readCback   Read callback function.
 *  \param  writeCback  Write callback function.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcCoreGattCbackRegister(attsReadCback_t readCback, attsWriteCback_t writeCback);

/*************************************************************************************************/
/*!
 *  \brief  Register callbacks for the service.
 *
 *  \param  readCback   Read callback function.
 *  \param  writeCback  Write callback function.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcCoreGapCbackRegister(attsReadCback_t readCback, attsWriteCback_t writeCback);

/*************************************************************************************************/
/*!
 *  \brief  Update the device name attribute value.
 *
 *  \param  name   Pointer to NULL terminated char string with device name
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcCoreGapDeviceNameUpdate(const char *name);

/*************************************************************************************************/
/*!
 *  \brief  Update the Appearance attribute value.
 *
 *  \param  appearance  Value of Appearance attribute (@see svc_ch.c)
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcCoreGapAppearanceUpdate(uint16_t appearance);

/*************************************************************************************************/
/*!
 *  \brief  Update the central address resolution attribute value.
 *
 *  \param  value   New value.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcCoreGapCentAddrResUpdate(bool_t value);

/*************************************************************************************************/
/*!
 *  \brief  Add the Resolvable Private Address Only (RPAO) characteristic to the GAP service.
 *          The RPAO characteristic should be added only when DM Privacy is enabled.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcCoreGapAddRpaoCh(void);

/*************************************************************************************************/
/*!
 *  \brief  Remove the Resolvable Private Address Only (RPAO) characteristic from the GAP service.
 *          The RPAO characteristic should be removed when DM Privacy is supported
 *          (defined GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED) but its not enabled in config
 *
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcCoreGapRemoveRpaoCh(void);

/*! \} */    /* GATT_AND_GAP_SERVICE */

#ifdef __cplusplus
};
#endif

#endif /* SVC_CORE_H */
