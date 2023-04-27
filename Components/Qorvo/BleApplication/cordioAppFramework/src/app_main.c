/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  Application framework main module.
 *
 *  Copyright (c) 2011-2019 Arm Ltd. All Rights Reserved.
 *  Arm Ltd. confidential and proprietary.
 *
 *  IMPORTANT.  Your use of this file is governed by a Software License Agreement
 *  ("Agreement") that must be accepted in order to download or otherwise receive a
 *  copy of this file.  You may not use or copy this file for any purpose other than
 *  as described in the Agreement.  If you do not agree to all of the terms of the
 *  Agreement do not use this file and delete all copies in your possession or control;
 *  if you do not have a copy of the Agreement, you must contact Arm Ltd. prior
 *  to any use, copying or further distribution of this software.
 */
/*************************************************************************************************/

#include <string.h>
#include "wsf_types.h"
#include "wsf_msg.h"
#include "sec_api.h"
#include "wsf_trace.h"
#include "wsf_timer.h"
#include "wsf_assert.h"
#include "util/bstream.h"
#include "dm_api.h"
#include "app_api.h"
#include "app_main.h"
#include "global.h"
#include "gpLog.h"
#include "app_db.h"
/* #include "app_ui.h" */

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/*! Configuration pointer for advertising */
appAdvCfg_t *pAppAdvCfg;

/*! Configuration pointer for extended and periodic advertising */
appExtAdvCfg_t *pAppExtAdvCfg;

/*! Configuration pointer for slave */
appSlaveCfg_t *pAppSlaveCfg;

/*! Configuration pointer for master */
appMasterCfg_t *pAppMasterCfg;

/*! Configuration pointer for extended master */
appExtMasterCfg_t *pAppExtMasterCfg;

/*! Configuration pointer for security */
appSecCfg_t *pAppSecCfg;

/*! Configuration pointer for connection parameter update */
appUpdateCfg_t *pAppUpdateCfg;

/*! Configuration pointer for discovery */
appDiscCfg_t *pAppDiscCfg;

/*! Configuration pointer for application */
appCfg_t *pAppCfg;

/*! Connection control block array */
appConnCb_t appConnCb[DM_CONN_MAX];

/*! WSF handler ID */
wsfHandlerId_t appHandlerId;

/*! Main control block */
appCb_t appCb;

/*! Configuration structure for incoming request actions */
const appReqActCfg_t appReqActCfg =
{
  APP_ACT_ACCEPT        /*! Action for the remote connection parameter request */
};

/*! Configuration pointer for incoming request actions on master */
appReqActCfg_t *pAppMasterReqActCfg = (appReqActCfg_t *) &appReqActCfg;

/*! Configurable pointer for incoming request actions on slave */
appReqActCfg_t *pAppSlaveReqActCfg = (appReqActCfg_t *) &appReqActCfg;

/*************************************************************************************************/
/*!
 *  \brief  Process messages from the event handler.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void appProcMsg(wsfMsgHdr_t *pMsg)
{
  switch(pMsg->event)
  {
    case APP_BTN_POLL_IND:
      /* appUiBtnPoll(); */
      break;

    case APP_UI_TIMER_IND:
      /* appUiTimerExpired(pMsg); */
      break;

    default:
      break;
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Check the bond-by-LTK state of a connection.
 *
 *  \param  connId      DM connection ID.
 *
 *  \return Bond-by-LTK state.
 */
/*************************************************************************************************/
bool_t appCheckBondByLtk(dmConnId_t connId)
{
  WSF_ASSERT((connId > 0) && (connId <= DM_CONN_MAX));

  return appConnCb[connId - 1].bondByLtk;
}

/*************************************************************************************************/
/*!
 *  \brief  Return the number of existing connections of the given role.
 *
 *  \param  role      Connection role
 *
 *  \return Number of connections.
 */
/*************************************************************************************************/
uint8_t appNumConns(uint8_t role)
{
  appConnCb_t   *pCcb = appConnCb;
  uint8_t       i, j;

  for (i = DM_CONN_MAX, j = 0; i > 0; i--, pCcb++)
  {
    if ((pCcb->connId != DM_CONN_ID_NONE) && (DmConnRole(pCcb->connId) == role))
    {
      j++;
    }
  }

  return j;
}

/*************************************************************************************************/
/*!
 *  \brief  Check the bonded state of a connection.
 *
 *  \param  connId      DM connection ID.
 *
 *  \return Bonded state.
 */
/*************************************************************************************************/
bool_t AppCheckBonded(dmConnId_t connId)
{
  WSF_ASSERT((connId > 0) && (connId <= DM_CONN_MAX));

  return appConnCb[connId - 1].bonded;
}

/*************************************************************************************************/
/*!
 *  \brief  App framework handler init function called during system initialization.
 *
 *  \param  handlerID  WSF handler ID for App.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppHandlerInit(wsfHandlerId_t handlerId)
{
  appHandlerId = handlerId;

  AppDbInit();
}

/*************************************************************************************************/
/*!
 *  \brief  WSF event handler for app framework.
 *
 *  \param  event   WSF event mask.
 *  \param  pMsg    WSF message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
{
  if (pMsg != NULL)
  {
    APP_TRACE_INFO1("App got evt %d", pMsg->event);

    if (pMsg->event >= APP_MASTER_MSG_START)
    {
      /* pass event to master handler */
      (*appCb.masterCback)(pMsg);
    }
    else if (pMsg->event >= APP_SLAVE_MSG_START)
    {
      /* pass event to slave handler */
      (*appCb.slaveCback)(pMsg);
    }
    else
    {
      appProcMsg(pMsg);
    }
  }
  else
  {
    if (event & APP_BTN_DOWN_EVT)
    {
      /* AppUiBtnPressed(); */
    }
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Handle a passkey request during pairing.  If the passkey is to displayed, a
 *          random passkey is generated and displayed.  If the passkey is to be entered
 *          the user is prompted to enter the passkey.
 *
 *  \param  pAuthReq  DM authentication requested event structure.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppHandlePasskey(dmSecAuthReqIndEvt_t *pAuthReq)
{
  uint32_t passkey;
  uint8_t  buf[SMP_PIN_LEN];

  if (pAuthReq->display)
  {
    /* generate random passkey, limit to 6 digit max */
    SecRand((uint8_t *) &passkey, sizeof(uint32_t));
    passkey %= 1000000;

    /* convert to byte buffer */
    buf[0] = UINT32_TO_BYTE0(passkey);
    buf[1] = UINT32_TO_BYTE1(passkey);
    buf[2] = UINT32_TO_BYTE2(passkey);

    /* send authentication response to DM */
    DmSecAuthRsp((dmConnId_t) pAuthReq->hdr.param, SMP_PIN_LEN, buf);

    /* display passkey */
    /* AppUiDisplayPasskey(passkey); */
  }
  else
  {
    /* prompt user to enter passkey */
    /* AppUiAction(APP_UI_PASSKEY_PROMPT); */
  }
}

/*************************************************************************************************/
/*!
*  \brief  Handle a numeric comparison indication during pairing.  The confirmation value is
*          displayed and the user is prompted to verify that the local and peer confirmation
*          values match.
*
*  \param  pCnfInd  DM confirmation indication event structure.
*
*  \return None.
*/
/*************************************************************************************************/
void AppHandleNumericComparison(dmSecCnfIndEvt_t *pCnfInd)
{
  /* uint32_t confirm = */ DmSecGetCompareValue(pCnfInd->confirm);

  /* display confirmation value */
  /* AppUiDisplayConfirmValue(confirm); */

  /* TODO: Verify that local and peer confirmation values match */
  DmSecCompareRsp((dmConnId_t)pCnfInd->hdr.param, TRUE);
}

/*************************************************************************************************/
/*!
 *  \brief  Close the connection with the give connection identifier.
 *
 *  \param  connId    Connection identifier.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppConnClose(dmConnId_t connId)
{
  DmConnClose(DM_CLIENT_ID_APP, connId, HCI_ERR_REMOTE_TERMINATED);
}

/*************************************************************************************************/
/*!
 *  \brief  Get a list of connection identifiers of open connections.
 *
 *  \param  pConnIdList    Buffer to hold connection IDs (must be DM_CONN_MAX bytes).
 *
 *  \return Number of open connections.
 *
 */
/*************************************************************************************************/
uint8_t AppConnOpenList(dmConnId_t *pConnIdList)
{
  appConnCb_t   *pCcb = appConnCb;
  uint8_t       i;
  uint8_t       pos = 0;

  memset(pConnIdList, DM_CONN_ID_NONE, DM_CONN_MAX);

  for (i = DM_CONN_MAX; i > 0; i--, pCcb++)
  {
    if (pCcb->connId != DM_CONN_ID_NONE)
    {
      pConnIdList[pos++] = pCcb->connId;
    }
  }

  return pos;
}

/*************************************************************************************************/
/*!
 *  \brief  Check if a connection is open.
 *
 *  \return Connection ID of open connection or DM_CONN_ID_NONE if no open connections.
 */
/*************************************************************************************************/
dmConnId_t AppConnIsOpen(void)
{
  appConnCb_t   *pCcb = appConnCb;
  uint8_t       i;

  for (i = DM_CONN_MAX; i > 0; i--, pCcb++)
  {
    if (pCcb->connId != DM_CONN_ID_NONE)
    {
      return pCcb->connId;
    }
  }

  return DM_CONN_ID_NONE;
}

/*************************************************************************************************/
/*!
 *  \brief  Check if a connection is open.
 *
 *  \param  connId    Connection identifier.
 *
 *  \return true if connection Id is an open connection. False if connId is closed on non exiting.
 */
/*************************************************************************************************/
bool_t AppConnIdOpenCon(dmConnId_t connId)
{
  appConnCb_t   *pCcb = appConnCb;
  uint8_t       i;

  /* Avoid false positive when function is called with DM_CONN_ID_NONE */
  if(connId != DM_CONN_ID_NONE)
  {
    for (i = DM_CONN_MAX; i > 0; i--, pCcb++)
    {
      if (pCcb->connId == connId)
      {
        return true;
      }
    }
  }
  return false;
}

/*************************************************************************************************/
/*!
 *  \brief  Find active connId by dbHandle.
 *
 *  \param  appDbHdl_t dbHandle to find.
 *
 *  \return ConnId by dbHandle.
 */
/*************************************************************************************************/
dmConnId_t AppFindConnIdByDbHandle(appDbHdl_t dbHdl)
{
  appConnCb_t   *pCcb = appConnCb;
  uint8_t       i;

  if(dbHdl != APP_DB_HDL_NONE)
  {
    for (i = DM_CONN_MAX; i > 0; i--, pCcb++)
    {
      if (pCcb->dbHdl == dbHdl)
      {
        return pCcb->connId;
      }
    }
  }
  return DM_CONN_ID_NONE;
}

/*************************************************************************************************/
/*!
 *  \brief  Get the device database record handle associated with an open connection.
 *
 *  \param  connId    Connection identifier.
 *
 *  \return Database record handle or APP_DB_HDL_NONE.
 */
/*************************************************************************************************/
appDbHdl_t AppDbGetHdl(dmConnId_t connId)
{
  return appConnCb[connId-1].dbHdl;
}

/*************************************************************************************************/
/*!
 *  \brief  Add device to resolving list.
 *
 *  \param  pMsg    Pointer to DM callback event message.
 *  \param  connId  Connection identifier.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppAddDevToResList(dmEvt_t* pMsg, dmConnId_t connId)
{
    dmSecKey_t* pPeerKey = NULL;
    dmSecKey_t zeroKey;

    appDbHdl_t hdl = appConnCb[connId - 1].dbHdl;

    /* if LL Privacy is supported  */
    if(HciLlPrivacySupported())
    {

      /*If Link Layer Privacy is enabled host should populate resolving list with:
       stored local irk, peer irk, and peer identity address and address type */

      pPeerKey = AppDbGetKey(hdl, DM_KEY_IRK, NULL); /* get peer irk key */

      if(pPeerKey == NULL) /* NULL means that peers has not exchanged its IRK */
      {
        /* According to BLE Specs for connectable directed adv event:
          "The Link Layer shall use resolvable private addresses for the advertiserâ€™s device address (AdvA field).
          If an IRK is available in the Link Layer Resolving List for the peer device,then the targetâ€™s device address (TargetA field)
          shall use a resolvable private address. If _an_IRK_is_not_available_in_the_Link_Layer_Resolving_List_ or the IRK is set to zero
          for the peer device, then the targetâ€™s device address (TargetA field) shall use the Identity Address when entering the
          Advertising State and using connectable directed events"

          When there is no information about peer in resolving list. Qorvos Link Layer Privacy implementation will use Identity Addr
          for both AdvA and TargetA which _breaks_privacy_. When LL Privacy is enabled and peer has not exchanged its IRK we generate
          all-zeros IRK and add peer to resolving List with all-zeros key. It will cause generating Direct Advertisements using RPA for AdvA
          and using public identity for TargetA

          In case of reconnecting using undirected advertisements having peer in resolving list will cause using RPA as AdvA
        */
        GP_LOG_SYSTEM_PRINTF("WARN: Link Layer Privacy is enabled but peer has not exchanged its IRK key. Adding all-zero IRK to resolving list", 0);
        /* bonded device address and address type is always stored in AppDb, even when no IRK was exchanged */
        BdaCpy(zeroKey.irk.bdAddr, AppDbGetPeerAddress(hdl));
        zeroKey.irk.addrType = AppDbGetPeerAddressType(hdl);

        memset(zeroKey.irk.key, 0, 16); /* generate zero irk */
        pPeerKey = &zeroKey;      /* set generated zero irk with address and address type to be populated in resolving list */
      }

      /* In appDb peer irk key is null, but it will be added to resolve list */
      DmPrivAddDevToResList(pPeerKey->irk.addrType, pPeerKey->irk.bdAddr, pPeerKey->irk.key, DmSecGetLocalIrk(), FALSE, pMsg->hdr.param);
  }
}


/*************************************************************************************************/
/*!
 *  \brief  Clear all bonding information.
 *
 *  \return None.
 *
 *  \Note   This API should not be used when:
 *          - Advertising (other than periodic advertising) is enabled,
 *          - Scanning is enabled, or
 *          - (Extended) Create connection or Create Sync command is outstanding.
 *
 *          Otherwise, clearing the resolving list in the Controller may fail.
 */
/*************************************************************************************************/
void AppClearAllBondingInfo(void)
{
  APP_TRACE_INFO0("Clear bonding info");

  /* clear bonded device info */
  AppDbDeleteAllRecords();

  /* if LL Privacy is supported */
  if (HciLlPrivacySupported())
  {
    /* if LL Privacy has been enabled */
    if (DmLlPrivEnabled())
    {
      /* make sure LL Privacy is disabled before clearing resolving list */
      DmPrivSetAddrResEnable(FALSE);
    }

    /* clear resolving list */
    DmPrivClearResList();
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Update privacy mode for a given peer device.
 *
 *  \param  hdl     Database record handle.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppUpdatePrivacyMode(appDbHdl_t hdl)
{
  /* if peer device's been added to resolving list but RPA Only attribute not found on peer device */
  if ((hdl != APP_DB_HDL_NONE) && AppDbGetPeerAddedToRl(hdl) && !AppDbGetPeerRpao(hdl))
  {
    dmSecKey_t *pPeerKey = AppDbGetKey(hdl, DM_KEY_IRK, NULL);
    if (pPeerKey != NULL)
    {
      /* set device privacy mode for this peer device */
      DmPrivSetPrivacyMode(pPeerKey->irk.addrType, pPeerKey->irk.bdAddr, DM_PRIV_MODE_DEVICE);

      /* make sure resolving list flag cleared */
      /* note: setting device privacy does not mean that device should be marked as not added to RL */
      /* AppDbSetPeerAddedToRl(hdl, FALSE); */
    }
  }
}
