/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  Application framework module for attribute server.
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
#include "wsf_buf.h"
#include "wsf_msg.h"
#include "wsf_trace.h"
#include "util/wstr.h"
#include "dm_api.h"
#include "att_api.h"
#include "app_api.h"
#include "app_main.h"
#include "svc_core.h"
#include "gatt/gatt_api.h"

/*************************************************************************************************/
/*!
*  \brief  Set the peer's CSRK and sign counter on this connection.
*
*  \param  connId      DM connection ID.
*
*  \return None.
*/
/*************************************************************************************************/
static void appServerSetSigningInfo(dmConnId_t connId)
{
  appDbHdl_t  dbHdl;
  dmSecKey_t  *pPeerKey;
  uint8_t     secLevel;

  /* if peer's CSRK is available */
  if (((dbHdl = AppDbGetHdl(connId)) != APP_DB_HDL_NONE) &&
      ((pPeerKey = AppDbGetKey(dbHdl, DM_KEY_CSRK, &secLevel)) != NULL))
  {
    bool_t authen = secLevel < DM_SEC_LEVEL_ENC_AUTH ? FALSE : TRUE;

    /* set peer's CSRK and sign counter on this connection */
    AttsSetCsrk(connId, pPeerKey->csrk.key, authen);
    AttsSetSignCounter(connId, AppDbGetPeerSignCounter(dbHdl));
  }
}

/*************************************************************************************************/
/*!
 *  \brief  ATT connection callback for app framework.
 *
 *  \param  pDmEvt  DM callback event.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppServerConnCback(dmEvt_t *pDmEvt)
{
  appDbHdl_t  dbHdl;
  dmConnId_t  connId = (dmConnId_t) pDmEvt->hdr.param;

  if (pDmEvt->hdr.event == DM_CONN_OPEN_IND)
  {
    /* apply the peer's CCC table - values are persistant across connection when bonded */
    if ((dbHdl = AppDbGetHdl(connId)) != APP_DB_HDL_NONE)
    {
      uint8_t changeAwareState;
      uint8_t *pCsf;

      AppDbGetCsfRecord(dbHdl, &changeAwareState, &pCsf);

      /* Apply peer's client supported features. */
      AttsCsfConnOpen(connId, changeAwareState, pCsf);

      AttsCccInitTable(connId, AppDbGetCccTbl(dbHdl));

      /* If database has changed and peer configured service indications, send one now. */
      if (changeAwareState == ATTS_CLIENT_CHANGE_UNAWARE)
      {
#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT
        GattSendServiceChangedInd(connId, ATT_HANDLE_START, ATT_HANDLE_MAX);
#endif // CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT
      }
    }
    else
    {
      /* set up CCC table with uninitialized (all zero) values. */
      AttsCccInitTable(connId, NULL);

      /* set CSF values to default */
      AttsCsfConnOpen(connId, TRUE, NULL);
    }

    /* set peer's data signing info */
    appServerSetSigningInfo(connId);
  }
#ifdef GP_DIVERSITY_CORDIO_QORVO_APP
  else if (pDmEvt->hdr.event == DM_SEC_ENCRYPT_IND)
  {
    /* When security is established update CCC table */
    if (pDmEvt->encryptInd.usingLtk && appCheckBondByLtk(connId))
    {
      if ((dbHdl = AppDbGetHdl(connId)) != APP_DB_HDL_NONE)
      {
        AttsCccInitTable(connId, AppDbGetCccTbl(dbHdl));
      }
    }
  }
#endif //GP_DIVERSITY_CORDIO_QORVO_APP
  else if (pDmEvt->hdr.event == DM_SEC_PAIR_CMPL_IND)
  {
    bool_t bonded;

    bonded = ((pDmEvt->pairCmpl.auth & DM_AUTH_BOND_FLAG) == DM_AUTH_BOND_FLAG);

    /* if a record exists but it is a new record, synchronize ATT CCC info into record. */
    if (bonded && (AppCheckBonded(connId) == FALSE) &&
        ((dbHdl = AppDbGetHdl(connId)) != APP_DB_HDL_NONE))
    {
      uint8_t tableLen, idx;
      uint8_t csf[ATT_CSF_LEN];

      tableLen = AttsGetCccTableLen();

      for (idx = 0; idx < tableLen; idx++)
      {
        uint16_t cccValue;

        if ((cccValue = AttsCccGet(connId, idx)) != 0)
        {
          AppDbSetCccTblValue(dbHdl, idx, cccValue);
        }
      }

      /* Store cached CSF information. */
      AttsCsfGetFeatures(connId, csf, sizeof(csf));
      AppDbSetCsfRecord(dbHdl, AttsCsfGetClientChangeAwareState(connId), csf);
    }

    /* set peer's data signing info */
    appServerSetSigningInfo(connId);
  }
  else if (pDmEvt->hdr.event == DM_CONN_CLOSE_IND)
  {
    /* clear CCC table on connection close */
    AttsCccClearTable(connId);

    if ((dbHdl = AppDbGetHdl(connId)) != APP_DB_HDL_NONE)
    {
      /* remember peer's sign counter */
      AppDbSetPeerSignCounter(dbHdl, AttsGetSignCounter(connId));
    }
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Handle a database hash update.
 *
 *  \param  pMsg    message containing new hash.
 *
 *  \return None.
 */
/*************************************************************************************************/
void appServerHandleDbHashUpdate(attEvt_t *pMsg)
{
  uint8_t *pCurrentHash = AppDbGetDbHash();

  /* Compare new hash with old. */
  if (pCurrentHash != NULL)
  {
    if (memcmp(pMsg->pValue, pCurrentHash, ATT_DATABASE_HASH_LEN))
    {
      /* hash has changed, set to NULL. */
      pCurrentHash = NULL;
    }
  }

  if (pCurrentHash == NULL)
  {
    /* Update App database. */
    AppDbSetDbHash(pMsg->pValue);

    /* Make all bonded clients change-unaware. */
    AppDbSetClientsChangeAwareState(APP_DB_HDL_NONE, ATTS_CLIENT_CHANGE_UNAWARE);

    /* Make all active clients change-unaware. */
    AttsCsfSetClientChangeAwareState(DM_CONN_ID_NONE, ATTS_CLIENT_CHANGE_UNAWARE);

    APP_TRACE_INFO0("Database hash updated");

#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT
    /* Send all connect clients configured to receive Service Changed Indications one now. */
    GattSendServiceChangedInd(DM_CONN_ID_NONE, ATT_HANDLE_START, ATT_HANDLE_MAX);
#endif // CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Handle a service change confirmation.
 *
 *  \param  pMsg    message containing handle value confirmation.
 *
 *  \return None.
 */
/*************************************************************************************************/
void appServerHandleSvcChangeCnf(attEvt_t *pMsg)
{
  /* Check if this is a confirmation on the Service Changed Indication. */
  if (pMsg->handle == GATT_SC_HDL)
  {
    appDbHdl_t  dbHdl;
    dmConnId_t  connId = (dmConnId_t)pMsg->hdr.param;

    if ((dbHdl = AppDbGetHdl(connId)) != APP_DB_HDL_NONE)
    {
      /* store update in device database */
      AppDbSetClientsChangeAwareState(dbHdl, ATTS_CLIENT_CHANGE_AWARE);
    }

    /* Client is now change-aware. */
    AttsCsfSetClientChangeAwareState(connId, ATTS_CLIENT_CHANGE_AWARE);
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Application ATTS client supported features changed callback.
 *
 *  \param  connId            DM Connection ID
 *  \param  changeAwareState  The state of awareness to a change, see ::attClientAwareStates.
 *  \param  pCsf              Pointer to the client supported features value.
 *
 *  \return None.
 */
/*************************************************************************************************/
void appServerCsfWriteCback(dmConnId_t connId, uint8_t changeAwareState, uint8_t *pCsf)
{
  appDbHdl_t dbHdl;

  if ((dbHdl = AppDbGetHdl(connId)) != APP_DB_HDL_NONE)
  {
    /* store update in device database */
    AppDbSetCsfRecord(dbHdl,  changeAwareState, pCsf);
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Process ATT messages.
 *
 *  \param  pMsg    message containing event.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppServerProcAttMsg(wsfMsgHdr_t *pMsg)
{
  switch(pMsg->event)
  {
    case ATTS_DB_HASH_CALC_CMPL_IND:
      appServerHandleDbHashUpdate((attEvt_t *)pMsg);
      break;

    case ATTS_HANDLE_VALUE_CNF:
      appServerHandleSvcChangeCnf((attEvt_t *)pMsg);
      break;

    default:
      break;
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Application Server initialization routine.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppServerInit(void)
{
  /* register callback with caching state machine */
  AttsCsfRegister(appServerCsfWriteCback);
}
