/*************************************************************************************************/
/*!
 *  \file   app_cfg.h
 *
 *  \brief  Application framework configuration.
 *
 *          $Date$
 *          $Revision$
 *
 *  Copyright (c) 2011-2017 ARM Ltd., all rights reserved.
 *  ARM Ltd. confidential and proprietary.
 *
 *  IMPORTANT.  Your use of this file is governed by a Software License Agreement
 *  ("Agreement") that must be accepted in order to download or otherwise receive a
 *  copy of this file.  You may not use or copy this file for any purpose other than
 *  as described in the Agreement.  If you do not agree to all of the terms of the
 *  Agreement do not use this file and delete all copies in your possession or control;
 *  if you do not have a copy of the Agreement, you must contact ARM Ltd. prior
 *  to any use, copying or further distribution of this software.
 */
/*************************************************************************************************/
#ifndef APP_CFG_H
#define APP_CFG_H


#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! Number of application database device records */
#if defined(CORDIO_APPFRAMEWORK_DIVERSITY_NUM_RECS)
#define APP_DB_NUM_RECS                  CORDIO_APPFRAMEWORK_DIVERSITY_NUM_RECS
#elif defined(GP_DIVERSITY_BLE_PERIPHERAL)
#define APP_DB_NUM_RECS                  GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS
#else
#define APP_DB_NUM_RECS                  3
#endif

#ifdef GP_DIVERSITY_BLE_PERIPHERAL
#if APP_DB_NUM_RECS != GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS
#error CORDIO_APPFRAMEWORK_DIVERSITY_NUM_RECS must be equal to GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS
#endif
#endif // GP_DIVERSITY_BLE_PERIPHERAL

/*! Number of client characteristic configuration descriptor handles per record */
#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_NUM_CCCD
#define APP_DB_NUM_CCCD                  CORDIO_APPFRAMEWORK_DIVERSITY_NUM_CCCD
#else
#define APP_DB_NUM_CCCD                  6
#endif

/*! Number of ATT client cached handles per record */
#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_DB_HDL_LIST_LEN
#define APP_DB_HDL_LIST_LEN              CORDIO_APPFRAMEWORK_DIVERSITY_DB_HDL_LIST_LEN
#else
#define APP_DB_HDL_LIST_LEN              20
#endif

/*! Number of scan results to store (used only when operating as master) */
#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_SCAN_RESULTS_MAX
#define APP_SCAN_RESULT_MAX              CORDIO_APPFRAMEWORK_DIVERSITY_SCAN_RESULTS_MAX
#else
#define APP_SCAN_RESULT_MAX              10
#endif

#ifdef __cplusplus
};
#endif

#endif /* APP_CFG_H */
