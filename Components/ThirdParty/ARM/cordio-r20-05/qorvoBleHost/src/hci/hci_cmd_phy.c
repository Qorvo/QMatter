/*!
 *  \file   hci_cmd_phy.c
 *
 *  \brief  HCI PHY command module.
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

#include "wsf_types.h"
#include "wsf_msg.h"
#include "bstream.h"
#include "hci_cmd.h"
#include "hci_api.h"
#include "hci_main.h"


#include "gpBle.h"
/*************************************************************************************************/
/*!
*  \fn     HciLeReadPhyCmd
*
*  \brief  HCI read PHY command.
*
*  \return None.
*/
/*************************************************************************************************/
void HciLeReadPhyCmd(uint16_t handle)
{

    gpHci_CommandParameters_t cmd;

    cmd.LeReadPhy.connectionHandle = handle;
    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeReadPhy, gpBle_UnknownOpCode, &cmd);

}

/*************************************************************************************************/
/*!
*  \fn     HciLeSetDefaultPhyCmd
*
*  \brief  HCI set default PHY command.
*
*  \return None.
*/
/*************************************************************************************************/
void HciLeSetDefaultPhyCmd(uint8_t allPhys, uint8_t txPhys, uint8_t rxPhys)
{
  gpHci_CommandParameters_t cmd;

  cmd.LeSetDefaultPhy.allPhys = allPhys;
  cmd.LeSetDefaultPhy.txPhys.mask = txPhys;
  cmd.LeSetDefaultPhy.rxPhys.mask = rxPhys;
  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetDefaultPhy, gpBle_UnknownOpCode, &cmd);
}

/*************************************************************************************************/
/*!
*  \fn     HciLeSetPhyCmd
*
*  \brief  HCI set PHY command.
*
*  \return None.
*/
/*************************************************************************************************/
void HciLeSetPhyCmd(uint16_t handle, uint8_t allPhys, uint8_t txPhys, uint8_t rxPhys, uint16_t phyOptions)
{
  gpHci_CommandParameters_t cmd;

  cmd.LeSetPhy.connectionHandle = handle;
  cmd.LeSetPhy.allPhys = allPhys;
  cmd.LeSetPhy.txPhys.mask = txPhys;
  cmd.LeSetPhy.rxPhys.mask = rxPhys;
  cmd.LeSetPhy.phyOptions = phyOptions;
  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetPhy, gpBle_UnknownOpCode, &cmd);
}
