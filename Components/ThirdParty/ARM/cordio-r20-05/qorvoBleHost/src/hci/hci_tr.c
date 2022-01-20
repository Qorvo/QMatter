/*!
 *  \file   hci_tr.c
 *
 *  \brief  HCI transport module.
 *
 *  Copyright (c) 2018 Arm Ltd. All Rights Reserved.
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

#include <string.h>
#include "wsf_types.h"
#include "wsf_msg.h"
#include "bstream.h"
#include "hci_api.h"
#include "hci_core.h"
#include "hci_tr.h"

#include "gpBle.h"
#include "gpBleDataTx.h"

/*************************************************************************************************/
/*!
 *  \fn     hciTrSendAclData
 *
 *  \brief  Send a complete HCI ACL packet to the transport.
 *
 *  \param  pContext Connection context.
 *  \param  pData    WSF msg buffer containing an ACL packet.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciTrSendAclData(void *pContext, uint8_t *pData, uint16_t hciFraglen, uint16_t hciFragPb)
{
    /* send to LL - skipping HCI ACL header bytes */
#ifndef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
    gpBle_DataTxRequest(((hciCoreConn_t*)pContext)->handle | hciFragPb, hciFraglen, &pData[4]);
#else
    gpBle_DataTxRequest(((hciCoreConn_t*)pContext)->handle | hciFragPb, hciFraglen, &pData[4], (void*)pContext, (void*)pData);
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */

#ifndef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
    /* free HCI buffer */
    hciCoreTxAclComplete(pContext, pData);
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */
}
