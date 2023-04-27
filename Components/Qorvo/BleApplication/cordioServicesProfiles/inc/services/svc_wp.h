/*!
 *  \file   svc_wp.h
 *
 *  \brief  ARM Ltd. proprietary service implementation.
 *
 *  Copyright (c) 2011-2017 ARM Ltd., all rights reserved.
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

#ifndef SVC_WP_H
#define SVC_WP_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  Macros
**************************************************************************************************/

/**************************************************************************************************
 Handle Ranges
**************************************************************************************************/

/* Proprietary Service */
#define WP_START_HDL               0x200
#define WP_END_HDL                 (WP_MAX_HDL - 1)

/**************************************************************************************************
 Handles
**************************************************************************************************/

/* Proprietary Service Handles */
enum
{
  WP_SVC_HDL = WP_START_HDL,       /* Proprietary service declaration */
  WP_DAT_CH_HDL,                   /* Proprietary data characteristic */
  WP_DAT_HDL,                      /* Proprietary data */
  WP_DAT_CH_CCC_HDL,               /* Proprietary data client characteristic configuration */
  WP_MAX_HDL
};

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

void SvcWpAddGroup(void);
void SvcWpRemoveGroup(void);
void SvcWpCbackRegister(attsReadCback_t readCback, attsWriteCback_t writeCback);

#ifdef __cplusplus
};
#endif

#endif /* SVC_WP_H */
