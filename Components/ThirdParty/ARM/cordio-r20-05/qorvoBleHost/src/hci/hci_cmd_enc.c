/*!
 *  \file   hci_cmd_enc.c
 *
 *  \brief  HCI command module for encryption.
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
#include "wsf_assert.h"
#include "wsf_queue.h"
#include "wsf_timer.h"
#include "wsf_msg.h"
#include "wsf_trace.h"
#include "bstream.h"
#include "calc128.h"
#include "hci_cmd.h"
#include "hci_tr.h"
#include "hci_api.h"
#include "hci_main.h"

#include "gpBle.h"
#include "gpBleSecurityCoprocessor.h"
#include "gpBleLlcpProcedures.h"
#include "gpHci.h"

/*************************************************************************************************/
/*!
 *  \fn     HciLeEncryptCmd
 *
 *  \brief  HCI LE encrypt command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeEncryptCmd(uint8_t* pKey, uint8_t* pData)
{
    hciEvt_t hciEvt;

    hciEvt.leEncryptCmdCmpl.hdr.status = gpHci_ResultUnknownHCICommand;
    hciEvt.leEncryptCmdCmpl.hdr.event = HCI_LE_ENCRYPT_CMD_CMPL_CBACK_EVT;

    hciEvt.leEncryptCmdCmpl.status = hciEvt.leEncryptCmdCmpl.hdr.status;

    hciCb.secCback(&hciEvt);
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeLtkReqNegReplCmd
 *
 *  \brief  HCI LE long term key request negative reply command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeLtkReqNegReplCmd(uint16_t handle)
{
    gpHci_CommandParameters_t cmd;

    cmd.LeLongTermKeyRequestNegativeReply.connectionHandle = handle;
    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeLongTermKeyRequestNegativeReply, gpBle_UnknownOpCode, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeLtkReqReplCmd
 *
 *  \brief  HCI LE long term key request reply command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeLtkReqReplCmd(uint16_t handle, uint8_t* pKey)
{
    gpHci_CommandParameters_t cmd;

    cmd.LeLongTermKeyRequestReply.connectionHandle = handle;
    cmd.LeLongTermKeyRequestReply.longTermKey = pKey;
    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeLongTermKeyRequestReply, gpBle_UnknownOpCode, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeStartEncryptionCmd
 *
 *  \brief  HCI LE start encryption command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeStartEncryptionCmd(uint16_t handle, uint8_t* pRand, uint16_t diversifier, uint8_t* pKey)
{
    gpHci_CommandParameters_t cmd;
#ifdef GP_DIVERSITY_BLE_MASTER
    cmd.LeStartEncryption.connectionHandle = handle;
    cmd.LeStartEncryption.encryptedDiversifier = diversifier;
    cmd.LeStartEncryption.randomNumber = pRand;
    cmd.LeStartEncryption.longTermKey = pKey;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeStartEncryption, gpBle_LeStartEncryption, &cmd);
#else
    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeStartEncryption, gpBle_UnknownOpCode, &cmd);
#endif // GP_DIVERSITY_BLE_MASTER
}
