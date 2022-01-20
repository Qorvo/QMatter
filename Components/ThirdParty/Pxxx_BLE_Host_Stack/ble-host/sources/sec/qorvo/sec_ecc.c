/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  Security ECC implementation using uECC.
 *
 *  Copyright (c) 2011-2018 Arm Ltd. All Rights Reserved.
 *
 *  Copyright (c) 2019-2020 Packetcraft, Inc.
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

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "wsf_types.h"
#include "wsf_queue.h"
#include "wsf_msg.h"
#include "wsf_trace.h"
#include "sec_api.h"
#include "sec_main.h"
#include "wsf_buf.h"
#include "hci_api.h"
#include "util/calc128.h"
#include "gpECC.h"

#ifdef GP_ECC_DIVERSITY_USE_SLICING
#include "wsf_assert.h"
#endif /* GP_ECC_DIVERSITY_USE_SLICING  */
#include "gpLog.h"

#ifndef SEC_ECC_CFG
#define SEC_ECC_CFG SEC_ECC_CFG_UECC
#endif

#if SEC_ECC_CFG == SEC_ECC_CFG_UECC

/**************************************************************************************************
  External Variables
**************************************************************************************************/

extern secCb_t secCb;

#ifdef GP_ECC_DIVERSITY_USE_SLICING
secEccMsg_t *pSharedSecretReturnMsg = NULL;
wsfHandlerId_t SharedSecretReturn_handlerId;

secEccMsg_t *pMakeKeyReturnMsg = NULL;
wsfHandlerId_t MakeKeyReturn_handlerId;
#endif /* GP_ECC_DIVERSITY_USE_SLICING  */

/*************************************************************************************************/
/*!
 *  \brief  Random number generator used by uECC.
 *
 *  \param  p_dest      Buffer to hold random number
 *  \param  p_size      Size of p_dest in bytes .
 *
 *  \return TRUE if successful.
 */
/*************************************************************************************************/
static int secEccRng(uint8_t *p_dest, unsigned p_size)
{
  SecRand(p_dest, p_size);
  return TRUE;
}

/*************************************************************************************************/
/*!
 *  \brief  Callback for HCI encryption for ECC operations.
 *
 *  \param  pBuf        Pointer to sec queue element.
 *  \param  pEvent      Pointer to HCI event.
 *  \param  handlerId   WSF handler ID.
 *
 *  \return none.
 */
/*************************************************************************************************/
void SecEccHciCback(secQueueBuf_t *pBuf, hciEvt_t *pEvent, wsfHandlerId_t handlerId)
{
  /* TBD */
}

/*************************************************************************************************/
/*!
 *  \brief  Generate an ECC key.
 *
 *  \param  handlerId   WSF handler ID for client.
 *  \param  param       Optional parameter sent to client's WSF handler.
 *  \param  event       Event for client's WSF handler.
 *
 *  \return TRUE if successful, else FALSE.
 */
/*************************************************************************************************/
#ifdef GP_ECC_DIVERSITY_USE_SLICING
void SecEccGenMakeKey_Result(uint8_t **pBuffs)
{
    WsfBufFree(pBuffs[GP_ECC_API_MAKE_KEY_CTX_NUM]);
    WsfBufFree(pBuffs[GP_ECC_SLICE_MAKE_KEY_CTX_NUM]);

    if (pMakeKeyReturnMsg)
    {
      WsfMsgSend(MakeKeyReturn_handlerId, pMakeKeyReturnMsg);
      pMakeKeyReturnMsg = NULL;
    }
}
#endif /* GP_ECC_DIVERSITY_USE_SLICING */

bool_t SecEccGenKey(wsfHandlerId_t handlerId, uint16_t param, uint8_t event)
{
  secEccMsg_t *pMsg = WsfMsgAlloc(sizeof(secEccMsg_t));

  if (pMsg)
  {
#ifndef GP_ECC_DIVERSITY_USE_SLICING
    /* Generate the keys */
    //uECC_make_key(pMsg->data.key.pubKey_x, pMsg->data.key.privKey);

    gpECC_make_key(pMsg->data.key.pubKey_x, pMsg->data.key.privKey, gpECC_secp256r1());

    /* Send local key to handler */
    pMsg->hdr.event = event;
    pMsg->hdr.param = param;
    pMsg->hdr.status = HCI_SUCCESS;
    WsfMsgSend(handlerId, pMsg);

#else
      if (pMakeKeyReturnMsg)
      {
        WsfMsgFree(pMsg);
        WSF_ASSERT(FALSE);
        return FALSE;
      }
      else
      {
        uint8_t *pBuffs[GP_ECC_NUM_CTX];
        uint16_t Sizes[GP_ECC_NUM_CTX];
        pBuffs[GP_ECC_API_MAKE_KEY_CTX_NUM] = WsfBufAlloc(GP_ECC_API_MAKE_KEY_CTX_SIZE);
        Sizes[GP_ECC_API_MAKE_KEY_CTX_NUM] = GP_ECC_API_MAKE_KEY_CTX_SIZE;
        pBuffs[GP_ECC_SLICE_MAKE_KEY_CTX_NUM] = WsfBufAlloc(GP_ECC_SLICE_MAKE_KEY_CTX_SIZE);
        Sizes[GP_ECC_SLICE_MAKE_KEY_CTX_NUM] = GP_ECC_SLICE_MAKE_KEY_CTX_SIZE;
        MakeKeyReturn_handlerId = handlerId;

        gpECC_make_key_sliced(pMsg->data.key.pubKey_x, pMsg->data.key.privKey, gpECC_secp256r1(), SecEccGenMakeKey_Result, pBuffs, Sizes);
      }

      /* Prepare return message*/
      pMsg->hdr.event = event;
      pMsg->hdr.param = param;
      pMsg->hdr.status = HCI_SUCCESS; // TODO fs035088: Set status at end of sliced calculation

      pMakeKeyReturnMsg = pMsg;
#endif /* GP_ECC_DIVERSITY_USE_SLICING */

    return TRUE;
  }

  return FALSE;
}

/*************************************************************************************************/
/*!
 *  \brief  Generate an ECC key.
 *
 *  \param  pKey        ECC Key structure.
 *  \param  handlerId   WSF handler ID for client.
 *  \param  param       Optional parameter sent to client's WSF handler.
 *  \param  event       Event for client's WSF handler.
 *
 *  \return TRUE if successful, else FALSE.
 */
/*************************************************************************************************/
#ifdef GP_ECC_DIVERSITY_USE_SLICING
void SecEccGenSharedSecret_Result(uint8_t **pBuffs)
{
    WsfBufFree(pBuffs[GP_ECC_API_CTX_NUM]);
    WsfBufFree(pBuffs[GP_ECC_SLICE_CTX_NUM]);
    if (pSharedSecretReturnMsg)
    {
      WsfMsgSend(SharedSecretReturn_handlerId, pSharedSecretReturnMsg);
      pSharedSecretReturnMsg = NULL;
    }
}
#endif /* GP_ECC_DIVERSITY_USE_SLICING */

bool_t SecEccGenSharedSecret(secEccKey_t *pKey, wsfHandlerId_t handlerId, uint16_t param, uint8_t event)
{
  secEccMsg_t *pMsg = WsfMsgAlloc(sizeof(secEccMsg_t));

  if (pMsg)
  {
    bool_t keyValid = gpECC_valid_public_key(pKey->pubKey_x);
    if (keyValid)
    {
#ifndef GP_ECC_DIVERSITY_USE_SLICING
      /* uECC_shared_secret(pKey->pubKey_x, pKey->privKey, pMsg->data.sharedSecret.secret); */
      gpECC_shared_secret(pKey->pubKey_x, pKey->privKey, pMsg->data.sharedSecret.secret, gpECC_secp256r1());
#else
      if (pSharedSecretReturnMsg)
      {
        WsfMsgFree(pMsg);
        WSF_ASSERT(FALSE);
        return FALSE;
      }
      else
      {
        uint8_t *pBuffs[GP_ECC_NUM_CTX];
        uint16_t Sizes[GP_ECC_NUM_CTX];
        pBuffs[GP_ECC_API_CTX_NUM] = WsfBufAlloc(GP_ECC_API_CTX_SIZE);
        Sizes[GP_ECC_API_CTX_NUM] = GP_ECC_API_CTX_SIZE;
        pBuffs[GP_ECC_SLICE_CTX_NUM] = WsfBufAlloc(GP_ECC_SLICE_CTX_SIZE);
        Sizes[GP_ECC_SLICE_CTX_NUM] = GP_ECC_SLICE_CTX_SIZE;
        SharedSecretReturn_handlerId = handlerId;
        gpECC_shared_secret_sliced(pKey->pubKey_x, pKey->privKey, pMsg->data.sharedSecret.secret, gpECC_secp256r1(), SecEccGenSharedSecret_Result, pBuffs, Sizes);
      }
#endif /* GP_ECC_DIVERSITY_USE_SLICING */
    }
    else
    {
       memset(pMsg->data.sharedSecret.secret, 0xFF, SEC_ECC_KEY_LEN);
    }

    /* Send shared secret to handler. */
    pMsg->hdr.event = event;
    pMsg->hdr.param = param;
    pMsg->hdr.status = keyValid ? HCI_SUCCESS : HCI_ERR_INVALID_PARAM;
#ifndef GP_ECC_DIVERSITY_USE_SLICING
    WsfMsgSend(handlerId, pMsg);
#else
    if(keyValid)
    {
      pSharedSecretReturnMsg = pMsg;
    }
    else
    {
      /* If key is not valid, SecEccGenSharedSecret_Result will not be called - message needs to be sent here to trigger the callback */
      WsfMsgSend(handlerId, pMsg);
    }
#endif /* GP_ECC_DIVERSITY_USE_SLICING  */
    return TRUE;
  }

  return FALSE;
}

/*************************************************************************************************/
/*!
 *  \brief  Called to initialize ECC security.
 *
 *  \param  None.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SecEccInit()
{
  /* srand((unsigned int)time(NULL)); */
  /* uECC_set_rng(secEccRng); */
  gpECC_set_rng(secEccRng);
}

#endif /* SEC_ECC_CFG */
