/*!
 *  \file   svc_wdxs.h
 *
 *  \brief  Wireless Data Exchange service implementation.
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

#ifndef SVC_WDXS_H
#define SVC_WDXS_H

#include "util/bstream.h"
#include "att_api.h"
#include "wdx_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 Handle Ranges
**************************************************************************************************/

/* Proprietary Service */
#define WDXS_START_HDL               0x240
#define WDXS_END_HDL                 (WDXS_MAX_HDL - 1)

/**************************************************************************************************
 Handles
**************************************************************************************************/

/* Proprietary Service Handles */
enum
{
  WDXS_SVC_HDL = WDXS_START_HDL,     /* Proprietary Service Declaration */
  WDXS_DC_CH_HDL,                    /* WDX Device Configuration Characteristic Declaration */
  WDXS_DC_HDL,                       /* WDX Device Configuration Characteristic Value */
  WDXS_DC_CH_CCC_HDL,                /* WDX Device Configuration CCCD */
  WDXS_FTC_CH_HDL,                   /* WDX File Transfer Control Characteristic Declaration */
  WDXS_FTC_HDL,                      /* WDX File Transfer Control Characteristic Value */
  WDXS_FTC_CH_CCC_HDL,               /* WDX File Transfer Control CCCD */
  WDXS_FTD_CH_HDL,                   /* WDX File Transfer Data Characteristic Declaration */
  WDXS_FTD_HDL,                      /* WDX File Transfer Data Characteristic Value */
  WDXS_FTD_CH_CCC_HDL,               /* WDX File Transfer Data CCCD */
  WDXS_AU_CH_HDL,                    /* WDX Authentication Characteristic Declaration */
  WDXS_AU_HDL,                       /* WDX Authentication Characteristic Value */
  WDXS_AU_CH_CCC_HDL,                /* WDX Authentication CCCD */
  WDXS_MAX_HDL
};

/**************************************************************************************************
  Global Declarations
**************************************************************************************************/

extern const uint8_t wdxsDcUuid[ATT_128_UUID_LEN];          /* WDX Device Configuration Characteristic */
extern const uint8_t wdxsFtcUuid[ATT_128_UUID_LEN];         /* WDX File Transfer Control Characteristic */
extern const uint8_t wdxsFtdUuid[ATT_128_UUID_LEN];         /* WDX File Transfer Data Characteristic */
extern const uint8_t wdxsAuUuid[ATT_128_UUID_LEN];          /* WDX Authentication Characteristic */

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

void SvcWdxsAddGroup(void);
void SvcWdxsRemoveGroup(void);
void SvcWdxsRegister(attsWriteCback_t writeCback);

#ifdef __cplusplus
}
#endif

#endif /* SVC_WDXS_H */
