/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  Application framework device database example, using simple RAM-based storage.
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

#define GP_COMPONENT_ID GP_COMPONENT_ID_QORVOBLEHOST

#include <string.h>
#include "wsf_types.h"
#include "wsf_assert.h"
#include "util/bda.h"
#include "app_api.h"
#include "app_main.h"
#include "app_db.h"
#include "app_cfg.h"

#include "global.h"
#include "gpLog.h"
#include "gpNvm.h"
#include "gpAssert.h"

#ifdef GP_DIVERSITY_CORDIO_QORVO_APP
#include "cordioAppFramework.h"
#endif // GP_DIVERSITY_CORDIO_QORVO_APP

/**************************************************************************************************
  Data Types
**************************************************************************************************/

#if (APP_DB_NUM_RECS > 3)
#error error: app_db NVM only implemented for maximum 3 links
#endif

#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
/*! Database record */
typedef struct
{
    /*! For slave local device */
    dmSecLtk_t  localLtk;                     /*! Local LTK */
    uint8_t     localLtkSecLevel;             /*! Local LTK security level */
    bool_t      peerAddrRes;                  /*! TRUE if address resolution's supported on peer device (master) */

    /*! For master local device */
    dmSecLtk_t  peerLtk;                      /*! Peer LTK */
    uint8_t     peerLtkSecLevel;              /*! Peer LTK security level */

#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_SERVER
    /*! for ATT server local device */
    uint16_t    cccTbl[APP_DB_NUM_CCCD];      /*! Client characteristic configuration descriptors */
    uint32_t    peerSignCounter;              /*! Peer Sign Counter */
#endif


#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT
    /*! for ATT client */
    uint16_t    hdlList[APP_DB_HDL_LIST_LEN]; /*! Cached handle list */
    uint8_t     discStatus;                   /*! Service discovery and configuration status */
#endif
} appDbRecExt_t;

typedef struct
{
  /*! Common for all roles */
  bdAddr_t     peerAddr;                      /*! Peer address */
  uint8_t      addrType;                      /*! Peer address type */
  dmSecIrk_t   peerIrk;                       /*! Peer IRK */
  dmSecCsrk_t  peerCsrk;                      /*! Peer CSRK */
  uint8_t      keyValidMask;                  /*! Valid keys in this record */
  bool_t       inUse;                         /*! TRUE if record in use */
  bool_t       valid;                         /*! TRUE if record is valid */
  bool_t       peerAddedToRl;                 /*! TRUE if peer device's been added to resolving list */
  bool_t       peerRpao;                      /*! TRUE if RPA Only attribute's present on peer device */

  /*! for ATT server local device */
  //uint16_t     cccTbl[APP_DB_NUM_CCCD];       /*! Client characteristic configuration descriptors */
  //uint32_t     peerSignCounter;               /*! Peer Sign Counter */
  uint8_t      changeAwareState;              /*! Peer client awareness to state change in database */
  uint8_t      csf[ATT_CSF_LEN];              /*! Peer client supported features record */

  /*! for ATT client */
  bool_t       cacheByHash;                   /*! TRUE if cached handles are maintained by comparing database hash */
  uint8_t      dbHash[ATT_DATABASE_HASH_LEN]; /*! Peer database hash */
  //uint16_t     hdlList[APP_DB_HDL_LIST_LEN];  /*! Cached handle list */
  //uint8_t      discStatus;                    /*! Service discovery and configuration status */

} appDbRec_t;
#else


/*! Database record */
typedef struct
{
    /*! Common for all roles */
    bdAddr_t    peerAddr;                     /*! Peer address */
    uint8_t     addrType;                     /*! Peer address type */
    dmSecIrk_t  peerIrk;                      /*! Peer IRK */
    dmSecCsrk_t peerCsrk;                     /*! Peer CSRK */
    uint8_t     keyValidMask;                 /*! Valid keys in this record */
    bool_t      inUse;                        /*! TRUE if record in use */
    bool_t      valid;                        /*! TRUE if record is valid */
    bool_t      peerAddedToRl;                /*! TRUE if peer device's been added to resolving list */
    bool_t      peerRpao;                     /*! TRUE if RPA Only attribute's present on peer device */
  /*! For slave local device */
  dmSecLtk_t   localLtk;                      /*! Local LTK */
  uint8_t      localLtkSecLevel;              /*! Local LTK security level */
  bool_t       peerAddrRes;                   /*! TRUE if address resolution's supported on peer device (master) */

  /*! For master local device */
  dmSecLtk_t   peerLtk;                       /*! Peer LTK */
  uint8_t      peerLtkSecLevel;               /*! Peer LTK security level */

  /*! for ATT server local device */
  uint16_t     cccTbl[APP_DB_NUM_CCCD];       /*! Client characteristic configuration descriptors */
  uint32_t     peerSignCounter;               /*! Peer Sign Counter */
  uint8_t      changeAwareState;              /*! Peer client awareness to state change in database */
  uint8_t      csf[ATT_CSF_LEN];              /*! Peer client supported features record */

  /*! for ATT client */
  bool_t       cacheByHash;                   /*! TRUE if cached handles are maintained by comparing database hash */
  uint8_t      dbHash[ATT_DATABASE_HASH_LEN]; /*! Peer database hash */
  uint16_t     hdlList[APP_DB_HDL_LIST_LEN];  /*! Cached handle list */
  uint8_t      discStatus;                    /*! Service discovery and configuration status */
} appDbRec_t;

#endif

/*! Database type */
typedef struct
{
  appDbRec_t  rec[APP_DB_NUM_RECS];               /*! Device database records */
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
  appDbRecExt_t  recExt[APP_DB_NUM_RECS];         /*! Device database records */
#endif
  char        devName[ATT_DEFAULT_PAYLOAD_LEN];   /*! Device name */
  uint8_t     devNameLen;                         /*! Device name length */
  uint8_t     dbHash[ATT_DATABASE_HASH_LEN];      /*! Device GATT database hash */
} appDb_t;

/**************************************************************************************************
  Local Variables
**************************************************************************************************/

/*! Database */
static appDb_t appDb;

#ifndef GP_DIVERSITY_CORDIO_QORVO_APP
/*! When all records are allocated use this index to determine which to overwrite */
static appDbRec_t *pAppDbNewRec = appDb.rec;
#endif //GP_DIVERSITY_CORDIO_QORVO_APP
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
#ifndef GP_DIVERSITY_CORDIO_QORVO_APP
static appDbRecExt_t *pAppDbNewRecExt = appDb.recExt;
#endif //GP_DIVERSITY_CORDIO_QORVO_APP

STATIC GP_NVM_CONST appDbRecExt_t ROM appDbDefaultBondRecordExt FLASH_PROGMEM = {


    .localLtk = { .key = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, .rand = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, .ediv = 0x0000 },
    .localLtkSecLevel = 0x00,
    .peerAddrRes = false,

    .peerLtk = { .key = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, .rand = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, .ediv = 0x0000 },
    .peerLtkSecLevel = 0x00,
#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT
    .discStatus = 0
#endif
};

STATIC GP_NVM_CONST appDbRec_t ROM appDbDefaultBondRecord FLASH_PROGMEM =
{

    .peerAddr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .addrType = 0x00,
    .peerIrk = { .key = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, .bdAddr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, .addrType = 0x00 },
    .peerCsrk = { .key = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    .keyValidMask = 0x00,
    .inUse = false,
    .valid = false,
    .peerAddedToRl = false,
    .peerRpao = false


};
#else
STATIC GP_NVM_CONST appDbRec_t ROM appDbDefaultBondRecord FLASH_PROGMEM =
{

    .peerAddr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .addrType = 0x00,
    .peerIrk = { .key = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, .bdAddr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, .addrType = 0x00 },
    .peerCsrk = { .key = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    .keyValidMask = 0x00,
    .inUse = false,
    .valid = false,
    .peerAddedToRl = false,
    .peerRpao = false


};

#endif

#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
static Bool NvmElemIf_DefaultInitializer(const ROM void * pTag, uint8_t* pBuffer);
static Bool NvmElemIf_DefaultInitializerExt(const ROM void * pTag, uint8_t* pBuffer);

static Bool NvmElemIf_cbAppDbRecConsistent(const ROM void * pTag);
static Bool NvmElemIf_cbAppDbRecExtConsistent(const ROM void * pTag);

#define BLE_HOST_NVM_BASE_TAG_ID   ((UInt16)(GP_COMPONENT_ID<<8))
const gpNvm_IdentifiableTag_t ROM wcBleHost_NvmElements[] FLASH_PROGMEM = {
    {
        BLE_HOST_NVM_BASE_TAG_ID + 0,
        (uint8_t*)&(appDb.rec[0]),
        sizeof(appDbRec_t),
        gpNvm_UpdateFrequencyHigh,
        (gpNvm_cbDefaultValueInitializer_t)NvmElemIf_DefaultInitializer,
        NvmElemIf_cbAppDbRecConsistent
    },
#if APP_DB_NUM_RECS > 1
    {
        BLE_HOST_NVM_BASE_TAG_ID + 1,
        (uint8_t*)&(appDb.rec[1]),
        sizeof(appDbRec_t),
        gpNvm_UpdateFrequencyHigh,
        (gpNvm_cbDefaultValueInitializer_t)NvmElemIf_DefaultInitializer,
        NvmElemIf_cbAppDbRecConsistent
    },
#if APP_DB_NUM_RECS > 2
    {
        BLE_HOST_NVM_BASE_TAG_ID + 2,
        (uint8_t*)&(appDb.rec[2]),
        sizeof(appDbRec_t),
        gpNvm_UpdateFrequencyHigh,
        (gpNvm_cbDefaultValueInitializer_t)NvmElemIf_DefaultInitializer,
        NvmElemIf_cbAppDbRecConsistent
    },
#endif
#endif
    {
        BLE_HOST_NVM_BASE_TAG_ID + APP_DB_NUM_RECS,
        (uint8_t*)&(appDb.recExt[0]),
        sizeof(appDbRecExt_t),
        gpNvm_UpdateFrequencyHigh,
        (gpNvm_cbDefaultValueInitializer_t)NvmElemIf_DefaultInitializerExt,
        NvmElemIf_cbAppDbRecExtConsistent
    },
#if APP_DB_NUM_RECS > 1
    {
        BLE_HOST_NVM_BASE_TAG_ID + APP_DB_NUM_RECS + 1,
        (uint8_t*)&(appDb.recExt[1]),
        sizeof(appDbRecExt_t),
        gpNvm_UpdateFrequencyHigh,
        (gpNvm_cbDefaultValueInitializer_t)NvmElemIf_DefaultInitializerExt,
        NvmElemIf_cbAppDbRecExtConsistent
    },
#if APP_DB_NUM_RECS > 2
    {
        BLE_HOST_NVM_BASE_TAG_ID + APP_DB_NUM_RECS + 2,
        (uint8_t*)&(appDb.recExt[2]),
        sizeof(appDbRecExt_t),
        gpNvm_UpdateFrequencyHigh,
        (gpNvm_cbDefaultValueInitializer_t)NvmElemIf_DefaultInitializerExt,
        NvmElemIf_cbAppDbRecExtConsistent
    }
#endif
#endif
};

static Bool NvmElemIf_DefaultInitializer(const ROM void * pTag, uint8_t* pBuffer)
{
    gpNvm_IdentifiableTag_t tag;
    memcpy((uint8_t*)&tag, pTag, sizeof(gpNvm_IdentifiableTag_t));
    if(NULL == pBuffer)
    {
        pBuffer = tag.pRamLocation;
        if(NULL == pBuffer)
        {
            return false;
        }
    }


    memcpy(pBuffer, (uint8_t*) &appDbDefaultBondRecord, sizeof(appDbRec_t) );
    return true;
}

static Bool NvmElemIf_DefaultInitializerExt(const ROM void * pTag, uint8_t* pBuffer)
{
    gpNvm_IdentifiableTag_t tag;
    memcpy((uint8_t*)&tag, pTag, sizeof(gpNvm_IdentifiableTag_t));
    if(NULL == pBuffer)
    {
        pBuffer = tag.pRamLocation;
        if(NULL == pBuffer)
        {
            return false;
        }
    }

    memcpy(pBuffer, (uint8_t*) &appDbDefaultBondRecordExt, sizeof(appDbRecExt_t) );
    return true;
}

static Bool NvmElemIf_cbAppDbRecConsistent(const ROM void * pTag)
{
    gpNvm_IdentifiableTag_t tag;
    appDbRec_t *appDbRec;
    UInt8 componentId;
    UInt8 tagId;

    MEMCPY_P(&tag, pTag, sizeof(gpNvm_IdentifiableTag_t));

    componentId = ((UInt8)((tag.uniqueTagId)>>8));
    tagId = ((UInt8)((tag.uniqueTagId) & 0xFF));

    if (componentId != GP_COMPONENT_ID)
    {
        GP_LOG_PRINTF("componentId 0x%02x is not the expected component id 0x%02x", 0, componentId, GP_COMPONENT_ID);
        return false;
    }

    if (tagId >= APP_DB_NUM_RECS)
    {
        GP_LOG_PRINTF("tagId 0x%02x exceeds number of appDb records 0x%02x", 0, tagId, APP_DB_NUM_RECS);
        return false;
    }

    if (tag.size != sizeof(appDbRec_t))
    {
        GP_LOG_PRINTF("tag size %d does not match expected size %d", 0, tag.size, sizeof(appDbRec_t));
        return false;
    }

    appDbRec = (appDbRec_t *)(tag.pRamLocation);

    if (!appDbRec)
    {
        GP_LOG_PRINTF("tag ram location is NULL", 0);
        return false;
    }

    gpNvm_Restore(componentId, tagId, (UInt8*)appDbRec);

    // appDbBackupRecord makes sure this is <= 1
    if (appDbRec->inUse > 1)
    {
        GP_LOG_PRINTF("inUse flag value %d is invalid", 0, appDbRec->inUse);
        return false;
    }

    // appDbBackupRecord makes sure this is <= 1
    if (appDbRec->valid > 1)
    {
        GP_LOG_PRINTF("valid flag value %d is invalid", 0, appDbRec->inUse);
        return false;
    }

    // if not in use or not valid we don't bother looking at other attributes
    if ( (!appDbRec->inUse) || (!appDbRec->valid) )
    {
        return true;
    }

    if ( (appDbRec->addrType != DM_ADDR_PUBLIC) &&
         (appDbRec->addrType != DM_ADDR_PUBLIC_IDENTITY) &&
         (appDbRec->addrType != DM_ADDR_RANDOM_IDENTITY) &&
         (appDbRec->addrType != DM_ADDR_RANDOM_UNRESOLVED) &&
         (appDbRec->addrType != DM_ADDR_NONE)
       )
    {
        GP_LOG_PRINTF("address type 0x%02x is invalid", 0, appDbRec->addrType);
        return false;
    }

    uint8_t maskedKeyValid = appDbRec->keyValidMask & (DM_KEY_LOCAL_LTK | DM_KEY_PEER_LTK | DM_KEY_IRK | DM_KEY_CSRK);
    if ( maskedKeyValid != appDbRec->keyValidMask)
    {
        GP_LOG_PRINTF("key valid mask 0x%02x has invalid bits set", 0, appDbRec->keyValidMask);
        return false;
    }

    if ( (appDbRec->changeAwareState != ATTS_CLIENT_CHANGE_AWARE) &&
         (appDbRec->changeAwareState != ATTS_CLIENT_CHANGE_PENDING_AWARE) &&
         (appDbRec->changeAwareState != ATTS_CLIENT_CHANGE_AWARE_DB_READ_PENDING) &&
         (appDbRec->changeAwareState != ATTS_CLIENT_CHANGE_UNAWARE)
       )
    {
        GP_LOG_PRINTF("key valid mask 0x%02x has invalid bit(s) set", 0, appDbRec->keyValidMask);
        return false;
    }

    // compile-time check that the size of the `csf` member of appDbRec is 1 (if not, the definition of the
    // member has changed and the check must be revisited to make sure it still applies)
    // note: not using COMPILE_TIME_ASSERT because that is a no-op for IAR
    {enum { assert_value = 1/(!!(sizeof(appDbRec->csf)==1)) };}

    if ( (appDbRec->csf[0] & ATTS_CSF_ALL_FEATURES) != appDbRec->csf[0] )
    {
        GP_LOG_PRINTF("Client Supported Features 0x%02x has invalid bit(s) set", 0, appDbRec->csf[0]);
        return false;
    }

    return true;
}

static Bool NvmElemIf_cbAppDbRecExtConsistent(const ROM void * pTag)
{
    gpNvm_IdentifiableTag_t tag;
    appDbRecExt_t *appDbRecExt;
    UInt8 componentId;
    UInt8 tagId;

    MEMCPY_P(&tag, pTag, sizeof(gpNvm_IdentifiableTag_t));

    componentId = ((UInt8)((tag.uniqueTagId)>>8));
    tagId = ((UInt8)((tag.uniqueTagId) & 0xFF));

    if (componentId != GP_COMPONENT_ID)
    {
        GP_LOG_PRINTF("componentId 0x%02x is not the expected component id 0x%02x", 0, componentId, GP_COMPONENT_ID);
        return false;
    }

    if ( (tagId < APP_DB_NUM_RECS) || (tagId > (2*APP_DB_NUM_RECS)) )
    {
        GP_LOG_PRINTF("tagId 0x%02x exceeds number of appDbExt records 0x%02x", 0, tagId, APP_DB_NUM_RECS);
        return false;
    }

    if (tag.size != sizeof(appDbRecExt_t))
    {
        GP_LOG_PRINTF("tag size %d does not match expected size %d", 0, tag.size, sizeof(appDbRec_t));
        return false;
    }

    appDbRecExt = (appDbRecExt_t *)(tag.pRamLocation);

    if (!appDbRecExt)
    {
        GP_LOG_PRINTF("tag ram location is NULL", 0);
        return false;
    }

    gpNvm_Restore(componentId, tagId, (UInt8*)appDbRecExt);

    if ( (appDbRecExt->peerLtkSecLevel != DM_SEC_LEVEL_NONE) &&
         (appDbRecExt->peerLtkSecLevel != DM_SEC_LEVEL_ENC) &&
         (appDbRecExt->peerLtkSecLevel != DM_SEC_LEVEL_ENC_AUTH) &&
         (appDbRecExt->peerLtkSecLevel != DM_SEC_LEVEL_ENC_LESC)
       )
    {
        GP_LOG_PRINTF("peerLtkSecLevel %02x is invalid", 0, appDbRecExt->peerLtkSecLevel);
        return false;
    }

#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT
    if ( (appDbRecExt->discStatus != APP_DISC_INIT) &&
         (appDbRecExt->discStatus != APP_DISC_READ_DATABASE_HASH) &&
         (appDbRecExt->discStatus != APP_DISC_SEC_REQUIRED) &&
         (appDbRecExt->discStatus != APP_DISC_START) &&
         (appDbRecExt->discStatus != APP_DISC_CMPL) &&
         (appDbRecExt->discStatus != APP_DISC_FAILED) &&
         (appDbRecExt->discStatus != APP_DISC_CFG_START) &&
         (appDbRecExt->discStatus != APP_DISC_CFG_CONN_START) &&
         (appDbRecExt->discStatus != APP_DISC_CFG_CMPL)
       )
    {
        GP_LOG_PRINTF("discStatus %02x is invalid", 0, appDbRecExt->discStatus);
        return false;
    }
#endif

    return true;
}

#else /* GP_NVM_DIVERSITY_ELEMENT_IF */
appDbRec_t BleHost_TempDbRec;
const gpNvm_Tag_t ROM wcBleHost_NvmSection[] FLASH_PROGMEM = {
    {(uint8_t*)&BleHost_TempDbRec, sizeof(appDbRec_t), gpNvm_UpdateFrequencyHigh, (uint8_t *)&appDbDefaultBondRecord},
    {(uint8_t*)&BleHost_TempDbRec, sizeof(appDbRec_t), gpNvm_UpdateFrequencyHigh, (uint8_t *)&appDbDefaultBondRecord},
    {(uint8_t*)&BleHost_TempDbRec, sizeof(appDbRec_t), gpNvm_UpdateFrequencyHigh, (uint8_t *)&appDbDefaultBondRecord},
};
#endif /* GP_NVM_DIVERSITY_ELEMENT_IF */

void AppDbDumpKey(appDbHdl_t hdl, uint8_t type)
{
    uint8_t secLevel;
    dmSecKey_t *key;

    key = AppDbGetKey(hdl, type, &secLevel);
    if (key)
    {
        switch (type)
        {
            case DM_KEY_LOCAL_LTK:
                GP_LOG_SYSTEM_PRINTF("Local LTK: 0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",0,
                    key->ltk.key[0],key->ltk.key[1],key->ltk.key[2],key->ltk.key[3],key->ltk.key[4],
                    key->ltk.key[5],key->ltk.key[6],key->ltk.key[7],key->ltk.key[8],key->ltk.key[9],
                    key->ltk.key[10],key->ltk.key[11],key->ltk.key[12],key->ltk.key[13],key->ltk.key[14],key->ltk.key[15]);
                break;
            case DM_KEY_PEER_LTK:
                GP_LOG_SYSTEM_PRINTF("Peer LTK: 0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",0,
                    key->ltk.key[0],key->ltk.key[1],key->ltk.key[2],key->ltk.key[3],key->ltk.key[4],
                    key->ltk.key[5],key->ltk.key[6],key->ltk.key[7],key->ltk.key[8],key->ltk.key[9],
                    key->ltk.key[10],key->ltk.key[11],key->ltk.key[12],key->ltk.key[13],key->ltk.key[14],key->ltk.key[15]);
                break;
            case DM_KEY_IRK:
                GP_LOG_SYSTEM_PRINTF("Peer IRK: 0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",0,
                    key->irk.key[0],key->irk.key[1],key->irk.key[2],key->irk.key[3],key->irk.key[4],
                    key->irk.key[5],key->irk.key[6],key->irk.key[7],key->irk.key[8],key->irk.key[9],
                    key->irk.key[10],key->irk.key[11],key->irk.key[12],key->irk.key[13],key->irk.key[14],key->irk.key[15]);
                break;
            default:
                break;
        }
    } else {
        switch (type)
        {
            case DM_KEY_LOCAL_LTK:
                GP_LOG_SYSTEM_PRINTF("Local LTK invalid",0);
                break;
            case DM_KEY_PEER_LTK:
                GP_LOG_SYSTEM_PRINTF("Peer LTK invalid",0);
                break;
            case DM_KEY_IRK:
                GP_LOG_SYSTEM_PRINTF("Peer IRK invalid",0);
                break;
            default:
                break;
        }
    }
}

static void BleHost_DumpBondRecord(appDbRec_t *pRec)
{
#ifdef GP_LOCAL_LOG
    GP_LOG_PRINTF("-- Bond Record (size: %d bytes)--",0,sizeof(appDbRec_t));
    GP_LOG_PRINTF("peerAddr: %02x:%02x:%02x:%02x:%02x:%02x",0,pRec->peerAddr[0],pRec->peerAddr[1],pRec->peerAddr[2],pRec->peerAddr[3],pRec->peerAddr[4],pRec->peerAddr[5]);
    GP_LOG_PRINTF("addrType: 0x%02x",0,pRec->addrType);
    GP_LOG_PRINTF("inUse: %d",0,pRec->inUse);
    GP_LOG_PRINTF("valid: %d",0,pRec->valid);


    AppDbDumpKey((appDbHdl_t) pRec, DM_KEY_LOCAL_LTK);
    AppDbDumpKey((appDbHdl_t) pRec, DM_KEY_PEER_LTK);
    AppDbDumpKey((appDbHdl_t) pRec, DM_KEY_IRK);

    UInt8 i=0;
    UInt8 *ptr = (UInt8 *)pRec;
    UInt16 totalSum = 0;

    for(i=0;i<sizeof(appDbRec_t); i++)
    {
        totalSum += ptr[i];
    }

    GP_LOG_PRINTF("Sanity sum = 0x%04x",0,totalSum);
#endif //GP_LOCAL_LOG
}

#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
static void appDbBackupExtRecord(appDbRecExt_t *pRecExt)
{
    uint8_t i;

    for (i = 0; i < APP_DB_NUM_RECS; i++)
    {
        if(pRecExt == &appDb.recExt[i])
        {
            /* found record, backup */
            gpNvm_Backup(GP_COMPONENT_ID, APP_DB_NUM_RECS + i, (UInt8*)&appDb.recExt[i]);

            if(appDb.rec[i].valid > 1 || appDb.rec[i].inUse > 1)
            {
                GP_ASSERT_SYSTEM(false);
            }
            return;
        }
    }

    GP_ASSERT_SYSTEM(false);
}
#endif

static void appDbBackupRecord(appDbRec_t *pRec)
{
    uint8_t i;

    for (i = 0; i < APP_DB_NUM_RECS; i++)
    {
        if(pRec == &appDb.rec[i])
        {
            /* found record, backup */
            gpNvm_Backup(GP_COMPONENT_ID, i, (UInt8*)&appDb.rec[i]);

            BleHost_DumpBondRecord(&appDb.rec[i]);
            if(appDb.rec[i].valid > 1 || appDb.rec[i].inUse > 1)
            {
                GP_ASSERT_SYSTEM(false);
            }
            return;
        }
    }

    GP_ASSERT_SYSTEM(false);
}

void appDbRestoreBonds(void)
{
    uint8_t i;

    /* Restore bond records */
    for(i=0;i<APP_DB_NUM_RECS;i++)
    {
        gpNvm_Restore(GP_COMPONENT_ID, i, (UInt8*)&appDb.rec[i]);

#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
        gpNvm_Restore(GP_COMPONENT_ID, APP_DB_NUM_RECS + i, (UInt8*)&appDb.recExt[i]);
#endif

        BleHost_DumpBondRecord(&appDb.rec[i]);
        if(appDb.rec[i].valid > 1 || appDb.rec[i].inUse > 1)
        {
            GP_ASSERT_SYSTEM(false);
        }
    }
}

#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
static appDbRecExt_t* appDbFindExtRecord(appDbRec_t *pRec)
{
    uint8_t i;

    for (i = 0; i < APP_DB_NUM_RECS; i++)
    {
        if(pRec == &appDb.rec[i])
        {
            return &appDb.recExt[i];
        }
    }

    GP_ASSERT_DEV_INT(false);

    return NULL;

}
#endif

/*************************************************************************************************/
/*!
 *  \brief  Initialize the device database.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbInit(void)
{
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
    gpNvm_RegisterElements(wcBleHost_NvmElements, number_of_elements(wcBleHost_NvmElements));
#else /* GP_NVM_DIVERSITY_ELEMENT_IF */
    gpNvm_RegisterSection(GP_COMPONENT_ID, wcBleHost_NvmSection, APP_DB_NUM_RECS, NULL);
#endif /* GP_NVM_DIVERSITY_ELEMENT_IF */
}

/*************************************************************************************************/
/*!
 *  \brief  Create a new device database record.
 *
 *  \param  addrType  Address type.
 *  \param  pAddr     Peer device address.
 *
 *  \return Database record handle.
 */
/*************************************************************************************************/
appDbHdl_t AppDbNewRecord(uint8_t addrType, uint8_t *pAddr)
{
  appDbRec_t  *pRec = appDb.rec;
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
  appDbRecExt_t  *pRecExt = appDb.recExt;
#endif
#ifdef GP_DIVERSITY_CORDIO_QORVO_APP
  uint8_t linkId = 0xFF;

  linkId = cordioAppFramework_cbGetConnectingLink();
  if (linkId >= APP_DB_NUM_RECS)
  {
        return APP_DB_HDL_NONE;
  }

  pRec += linkId;
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
  pRecExt += linkId;
#endif

#else // GP_DIVERSITY_CORDIO_QORVO_APP
  uint8_t     i;

  /* find a free record */
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
  for (i = APP_DB_NUM_RECS; i > 0; i--, pRec++, pRecExt++)
#else
  for (i = APP_DB_NUM_RECS; i > 0; i--, pRec++)
#endif
  {
    if (!pRec->inUse)
    {
      break;
    }
  }

  /* if all records were allocated */
  if (i == 0)
  {
    /* overwrite a record */
    pRec = pAppDbNewRec;
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
    pRecExt = pAppDbNewRecExt;
#endif

    /* get next record to overwrite */
    pAppDbNewRec++;
    if (pAppDbNewRec == &appDb.rec[APP_DB_NUM_RECS])
    {
      pAppDbNewRec = appDb.rec;
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
      pAppDbNewRecExt = appDb.recExt;
#endif
    }
  }
#endif // GP_DIVERSITY_CORDIO_QORVO_APP

  /* initialize record */
  memset(pRec, 0, sizeof(appDbRec_t));
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
  memset(pRecExt, 0, sizeof(appDbRecExt_t));
#endif
  pRec->inUse = TRUE;
  pRec->addrType = addrType;
  BdaCpy(pRec->peerAddr, pAddr);
  pRec->peerAddedToRl = FALSE;
  pRec->peerRpao = FALSE;

  return (appDbHdl_t) pRec;
}

/*************************************************************************************************/
/*!
*  \brief  Get the next database record for a given record. For the first record, the function
*          should be called with 'hdl' set to 'APP_DB_HDL_NONE'.
*
*  \param  hdl  Database record handle.
*
*  \return Next record handle found. APP_DB_HDL_NONE, otherwise.
*/
/*************************************************************************************************/
appDbHdl_t AppDbGetNextRecord(appDbHdl_t hdl)
{
  appDbRec_t  *pRec;

  /* if first record is requested */
  if (hdl == APP_DB_HDL_NONE)
  {
    pRec = appDb.rec;
  }
  /* if valid record passed in */
  else if (AppDbRecordInUse(hdl))
  {
    pRec = (appDbRec_t *)hdl;
    pRec++;
  }
  /* invalid record passed in */
  else
  {
    return APP_DB_HDL_NONE;
  }

  /* look for next valid record */
  while (pRec < &appDb.rec[APP_DB_NUM_RECS])
  {
    /* if record is in use */
    if (pRec->inUse && pRec->valid)
    {
      /* record found */
      return (appDbHdl_t)pRec;
    }

    /* look for next record */
    pRec++;
  }

  /* end of records */
  return APP_DB_HDL_NONE;
}

#ifdef GP_DIVERSITY_CORDIO_QORVO_APP
/*************************************************************************************************/
/*!
 *  \fn     AppDbGetNextRecordBare
 *
 *  \brief  Get the next database record recardless of status InUse and Valid for a given record.
 *          For the first record, the function should be called with 'hdl' set to 'APP_DB_HDL_NONE'.
 *
 *  \param  hdl  Database record handle.
 *
 *  \return Next record handle found. APP_DB_HDL_NONE, otherwise.
 */
/*************************************************************************************************/
appDbHdl_t AppDbGetNextRecordBare(appDbHdl_t hdl)
{
    appDbRec_t *pRec = (appDbRec_t *)hdl;

    /* if first record is requested */
    if (hdl == APP_DB_HDL_NONE)
    {
        pRec = appDb.rec;
    }
    /* if valid record passed in */
    else if (pRec < appDb.rec + APP_DB_NUM_RECS - 1)
    {
        pRec++;
    }
    else
    {
        pRec = NULL;
    }

    return (appDbHdl_t)pRec;
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbGetHdlByLinkId
 *
 *  \brief  Get the handle of a LinkId
 *
 *  \param  LinkId.
 *
 *  \return Next record handle found. APP_DB_HDL_NONE, otherwise.
 */
/*************************************************************************************************/
appDbHdl_t AppDbGetHdlByLinkId(uint8_t LinkId)
{
    appDbRec_t *pRec = NULL;

    if (LinkId < APP_DB_NUM_RECS)
        pRec = appDb.rec + LinkId;

    return (appDbHdl_t)pRec;
}
#endif // GP_DIVERSITY_CORDIO_QORVO_APP

/*************************************************************************************************/
/*!
 *  \brief  Delete a new device database record.
 *
 *  \param  hdl       Database record handle.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbDeleteRecord(appDbHdl_t hdl)
{
  ((appDbRec_t *) hdl)->inUse = FALSE;

  appDbBackupRecord((appDbRec_t *) hdl);
}

/*************************************************************************************************/
/*!
 *  \brief  Validate a new device database record.  This function is called when pairing is
 *          successful and the devices are bonded.
 *
 *  \param  hdl       Database record handle.
 *  \param  keyMask   Bitmask of keys to validate.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbValidateRecord(appDbHdl_t hdl, uint8_t keyMask)
{
  ((appDbRec_t *) hdl)->valid = TRUE;
  ((appDbRec_t *) hdl)->keyValidMask = keyMask;

  appDbBackupRecord((appDbRec_t *) hdl);
}

/*************************************************************************************************/
/*!
 *  \brief  Check if a record has been validated.  If it has not, delete it.  This function
 *          is typically called when the connection is closed.
 *
 *  \param  hdl       Database record handle.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbCheckValidRecord(appDbHdl_t hdl)
{
  if (((appDbRec_t *) hdl)->valid == FALSE)
  {
    AppDbDeleteRecord(hdl);
  }
}

/*************************************************************************************************/
/*!
*  \brief  Check if a database record is in use.

*  \param  hdl       Database record handle.
*
*  \return TURE if record in use. FALSE, otherwise.
*/
/*************************************************************************************************/
bool_t AppDbRecordInUse(appDbHdl_t hdl)
{
  appDbRec_t  *pRec = appDb.rec;
  uint8_t     i;

  /* see if record is in database record list */
  for (i = APP_DB_NUM_RECS; i > 0; i--, pRec++)
  {
    if (pRec->inUse && pRec->valid && (pRec == ((appDbRec_t *)hdl)))
    {
      return TRUE;
    }
  }

  return FALSE;
}

/*************************************************************************************************/
/*!
 *  \brief  Check if there is a stored bond with any device.
 *
 *  \param  hdl       Database record handle.
 *
 *  \return TRUE if a bonded device is found, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t AppDbCheckBonded(void)
{
  appDbRec_t  *pRec = appDb.rec;
  uint8_t     i;

  /* find a record */
  for (i = APP_DB_NUM_RECS; i > 0; i--, pRec++)
  {
    if (pRec->inUse)
    {
      return TRUE;
    }
  }

  return FALSE;
}

/*************************************************************************************************/
/*!
 *  \brief  Delete all database records.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbDeleteAllRecords(void)
{
  appDbRec_t  *pRec = appDb.rec;
  uint8_t     i;

  /* set in use to false for all records */
  for (i = APP_DB_NUM_RECS; i > 0; i--, pRec++)
  {
    pRec->inUse = FALSE;
    appDbBackupRecord(pRec);
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Find a device database record by peer address.
 *
 *  \param  addrType  Address type.
 *  \param  pAddr     Peer device address.
 *
 *  \return Database record handle or APP_DB_HDL_NONE if not found.
 */
/*************************************************************************************************/
appDbHdl_t AppDbFindByAddr(uint8_t addrType, uint8_t *pAddr)
{
  appDbRec_t  *pRec = appDb.rec;
  uint8_t     peerAddrType = DmHostAddrType(addrType);
  uint8_t     i;

  /* find matching record */
  for (i = APP_DB_NUM_RECS; i > 0; i--, pRec++)
  {
    if (pRec->inUse && (pRec->addrType == peerAddrType) && BdaCmp(pRec->peerAddr, pAddr))
    {
      return (appDbHdl_t) pRec;
    }
  }

  return APP_DB_HDL_NONE;
}

/*************************************************************************************************/
/*!
 *  \brief  Find a device database record by data in an LTK request.
 *
 *  \param  encDiversifier  Encryption diversifier associated with key.
 *  \param  pRandNum        Pointer to random number associated with key.
 *
 *  \return Database record handle or APP_DB_HDL_NONE if not found.
 */
/*************************************************************************************************/
appDbHdl_t AppDbFindByLtkReq(uint16_t encDiversifier, uint8_t *pRandNum)
{
    appDbRec_t  *pRec = appDb.rec;
    uint8_t     i;

#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
    appDbRecExt_t *pRecExt = appDbFindExtRecord(pRec);

    GP_ASSERT_DEV_INT(pRecExt != NULL);
#endif

    /* find matching record */
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
    for (i = APP_DB_NUM_RECS; i > 0; i--, pRec++, pRecExt++)
#else
    for (i = APP_DB_NUM_RECS; i > 0; i--, pRec++)
#endif
    {
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
        if (pRec->inUse && (pRecExt->localLtk.ediv == encDiversifier) &&
                (memcmp(pRecExt->localLtk.rand, pRandNum, SMP_RAND8_LEN) == 0))
#else
            if (pRec->inUse && (pRec->localLtk.ediv == encDiversifier) &&
                    (memcmp(pRec->localLtk.rand, pRandNum, SMP_RAND8_LEN) == 0))
#endif
            {
                return (appDbHdl_t) pRec;
            }
    }

    return APP_DB_HDL_NONE;
}

/*************************************************************************************************/
/*!
 *  \brief  Get a key from a device database record.
 *
 *  \param  hdl       Database record handle.
 *  \param  type      Type of key to get.
 *  \param  pSecLevel If the key is valid, the security level of the key.
 *
 *  \return Pointer to key if key is valid or NULL if not valid.
 */
/*************************************************************************************************/
dmSecKey_t *AppDbGetKey(appDbHdl_t hdl, uint8_t type, uint8_t *pSecLevel)
{
    dmSecKey_t *pKey = NULL;
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
    appDbRecExt_t *pRecExt = appDbFindExtRecord((appDbRec_t *)hdl);

    GP_ASSERT_DEV_INT(pRecExt != NULL);
#endif

    /* if key valid */
    if ((type & ((appDbRec_t *) hdl)->keyValidMask) != 0)
    {
        switch(type)
        {
            case DM_KEY_LOCAL_LTK:
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
                *pSecLevel = pRecExt->localLtkSecLevel;
                pKey = (dmSecKey_t *) &pRecExt->localLtk;
#else
                *pSecLevel = ((appDbRec_t *) hdl)->localLtkSecLevel;
                pKey = (dmSecKey_t *) &((appDbRec_t *) hdl)->localLtk;
#endif
                break;

            case DM_KEY_PEER_LTK:
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
                *pSecLevel = pRecExt->peerLtkSecLevel;
                pKey = (dmSecKey_t *) &pRecExt->peerLtk;
#else
                *pSecLevel = ((appDbRec_t *) hdl)->peerLtkSecLevel;
                pKey = (dmSecKey_t *) &((appDbRec_t *) hdl)->peerLtk;
#endif
                break;

            case DM_KEY_IRK:
                pKey = (dmSecKey_t *)&((appDbRec_t *)hdl)->peerIrk;
                break;

#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_SERVER
            case DM_KEY_CSRK:
                pKey = (dmSecKey_t *)&((appDbRec_t *)hdl)->peerCsrk;
                break;
#endif

            default:
                break;
        }
    }

    return pKey;
}

/*************************************************************************************************/
/*!
 *  \brief  Set a key in a device database record.
 *
 *  \param  hdl       Database record handle.
 *  \param  pKey      Key data.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetKey(appDbHdl_t hdl, dmSecKeyIndEvt_t *pKey)
{
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
    appDbRecExt_t *pRecExt = appDbFindExtRecord((appDbRec_t *)hdl);

    GP_ASSERT_DEV_INT(pRecExt != NULL);
#endif

    switch(pKey->type)
    {
        case DM_KEY_LOCAL_LTK:
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
            pRecExt->localLtkSecLevel = pKey->secLevel;
            pRecExt->localLtk = pKey->keyData.ltk;
            appDbBackupExtRecord(pRecExt);
#else
            ((appDbRec_t *) hdl)->localLtkSecLevel = pKey->secLevel;
            ((appDbRec_t *) hdl)->localLtk = pKey->keyData.ltk;
#endif
            break;

        case DM_KEY_PEER_LTK:
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
            pRecExt->peerLtkSecLevel = pKey->secLevel;
            pRecExt->peerLtk = pKey->keyData.ltk;
            appDbBackupExtRecord(pRecExt);
#else
            ((appDbRec_t *) hdl)->peerLtkSecLevel = pKey->secLevel;
            ((appDbRec_t *) hdl)->peerLtk = pKey->keyData.ltk;
#endif
            break;

        case DM_KEY_IRK:
            ((appDbRec_t *)hdl)->peerIrk = pKey->keyData.irk;

            /* make sure peer record is stored using its identity address */
            ((appDbRec_t *)hdl)->addrType = pKey->keyData.irk.addrType;
            BdaCpy(((appDbRec_t *)hdl)->peerAddr, pKey->keyData.irk.bdAddr);
            break;

#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_SERVER
        case DM_KEY_CSRK:
            ((appDbRec_t *)hdl)->peerCsrk = pKey->keyData.csrk;
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
            /* sign counter must be initialized to zero when CSRK is generated */
            pRecExt->peerSignCounter = 0;
            appDbBackupRecord((appDbRec_t *) hdl);
#endif
            break;
#endif // CORDIO_APPFRAMEWORK_DIVERSITY_ATT_SERVER

        default:
            break;
    }
#ifndef GP_NVM_DIVERSITY_ELEMENT_IF
    appDbBackupRecord((appDbRec_t *) hdl);
#endif
}

/*************************************************************************************************/
/*!
 *  \brief  Get the peer's database hash.
 *
 *  \param  hdl       Database record handle.
 *
 *  \return Pointer to database hash.
 */
/*************************************************************************************************/
uint8_t *AppDbGetPeerDbHash(appDbHdl_t hdl)
{
  return ((appDbRec_t *) hdl)->dbHash;
}

/*************************************************************************************************/
/*!
 *  \brief  Set a new peer database hash.
 *
 *  \param  hdl       Database record handle.
 *  \param  pDbHash   Pointer to new hash.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetPeerDbHash(appDbHdl_t hdl, uint8_t *pDbHash)
{
  WSF_ASSERT(pDbHash != NULL);

  memcpy(((appDbRec_t *) hdl)->dbHash, pDbHash, ATT_DATABASE_HASH_LEN);
}

/*************************************************************************************************/
/*!
 *  \brief  Check if cached handles' validity is determined by reading the peer's database hash
 *
 *  \param  hdl       Database record handle.
 *
 *  \return \ref TRUE if peer's database hash must be read to verify handles have not changed.
 */
/*************************************************************************************************/
bool_t AppDbIsCacheCheckedByHash(appDbHdl_t hdl)
{
  return ((appDbRec_t *) hdl)->cacheByHash;
}

/*************************************************************************************************/
/*!
 *  \brief  Set if cached handles' validity is determined by reading the peer's database hash.
 *
 *  \param  hdl           Database record handle.
 *  \param  cacheByHash   \ref TRUE if peer's database must be read to verify cached handles.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetCacheByHash(appDbHdl_t hdl, bool_t cacheByHash)
{
  ((appDbRec_t *) hdl)->cacheByHash = cacheByHash;
}

#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_SERVER
/*************************************************************************************************/
/*!
 *  \brief  Get the client characteristic configuration descriptor table.
 *
 *  \param  hdl       Database record handle.
 *
 *  \return Pointer to client characteristic configuration descriptor table.
 */
/*************************************************************************************************/
uint16_t *AppDbGetCccTbl(appDbHdl_t hdl)
{
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
    appDbRecExt_t *pRecExt = appDbFindExtRecord((appDbRec_t *)hdl);

    GP_ASSERT_DEV_INT(pRecExt != NULL);
    return pRecExt->cccTbl;
#else
  return ((appDbRec_t *) hdl)->cccTbl;
#endif
}

/*************************************************************************************************/
/*!
 *  \brief  Set a value in the client characteristic configuration table.
 *
 *  \param  hdl       Database record handle.
 *  \param  idx       Table index.
 *  \param  value     client characteristic configuration value.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetCccTblValue(appDbHdl_t hdl, uint16_t idx, uint16_t value)
{
    WSF_ASSERT(idx < APP_DB_NUM_CCCD);

#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
    appDbRecExt_t *pRecExt = appDbFindExtRecord((appDbRec_t *)hdl);

    GP_ASSERT_DEV_INT(pRecExt != NULL);

    pRecExt->cccTbl[idx] = value;

    appDbBackupExtRecord(pRecExt);
#else
    ((appDbRec_t *) hdl)->cccTbl[idx] = value;

    appDbBackupRecord((appDbRec_t *) hdl);
#endif

}
#endif // CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT

/*************************************************************************************************/
/*!
 *  \brief  Get the client supported features record.
 *
 *  \param  hdl                Database record handle.
 *  \param  pChangeAwareState  Pointer to peer client's change aware status to a change in the database.
 *  \param  pCsf               Pointer to csf value pointer.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbGetCsfRecord(appDbHdl_t hdl, uint8_t *pChangeAwareState, uint8_t **pCsf)
{
  *pChangeAwareState = ((appDbRec_t *)hdl)->changeAwareState;
  *pCsf = ((appDbRec_t *) hdl)->csf;
}

/*************************************************************************************************/
/*!
 *  \brief  Set a client supported features record.
 *
 *  \param  hdl               Database record handle.
 *  \param  changeAwareState  The state of awareness to a change, see ::attClientAwareStates.
 *  \param  pCsf              Pointer to the client supported features value.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetCsfRecord(appDbHdl_t hdl, uint8_t changeAwareState, uint8_t *pCsf)
{
  if ((pCsf != NULL) && (hdl != APP_DB_HDL_NONE))
  {
    ((appDbRec_t *) hdl)->changeAwareState = changeAwareState;
    memcpy(&((appDbRec_t *) hdl)->csf, pCsf, ATT_CSF_LEN);
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Set client's state of awareness to a change in the database.
 *
 *  \param  hdl        Database record handle. If \ref hdl == \ref NULL, state is set for all
 *                     clients.
 *  \param  state      The state of awareness to a change, see ::attClientAwareStates.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetClientsChangeAwareState(appDbHdl_t hdl, uint8_t state)
{
  if (hdl == APP_DB_HDL_NONE)
  {
    appDbRec_t  *pRec = appDb.rec;
    uint8_t     i;

    /* Set all clients status to change-unaware. */
    for (i = APP_DB_NUM_RECS; i > 0; i--, pRec++)
    {
      pRec->changeAwareState = state;
    }
  }
  else
  {
    ((appDbRec_t *) hdl)->changeAwareState = state;
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Get device's GATT database hash.
 *
 *  \return Pointer to database hash.
 */
/*************************************************************************************************/
uint8_t *AppDbGetDbHash(void)
{
  return appDb.dbHash;
}

/*************************************************************************************************/
/*!
 *  \brief  Set device's  GATT database hash.
 *
 *  \param  pHash    GATT database hash to store.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetDbHash(uint8_t *pHash)
{
  if (pHash != NULL)
  {
    memcpy(appDb.dbHash, pHash, ATT_DATABASE_HASH_LEN);
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Get the discovery status.
 *
 *  \param  hdl       Database record handle.
 *
 *  \return Discovery status.
 */
/*************************************************************************************************/
#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT
/*************************************************************************************************/
/*!
 *  \fn     AppDbGetDiscStatus
 *
 *  \brief  Get the discovery status.
 *
 *  \param  hdl       Database record handle.
 *
 *  \return Discovery status.
 */
/*************************************************************************************************/
uint8_t AppDbGetDiscStatus(appDbHdl_t hdl)
{
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
    appDbRecExt_t *pRecExt = appDbFindExtRecord((appDbRec_t *)hdl);

    GP_ASSERT_DEV_INT(pRecExt != NULL);

    return pRecExt->discStatus;
#else
    return ((appDbRec_t *) hdl)->discStatus;
#endif //GP_NVM_DIVERSITY_ELEMENT_IF
}

/*************************************************************************************************/
/*!
 *  \brief  Set the discovery status.
 *
 *  \param  hdl       Database record handle.
 *  \param  state     Discovery status.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetDiscStatus(appDbHdl_t hdl, uint8_t status)
{
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
    appDbRecExt_t *pRecExt = appDbFindExtRecord((appDbRec_t *)hdl);

    GP_ASSERT_DEV_INT(pRecExt != NULL);

    pRecExt->discStatus = status;

    appDbBackupExtRecord(pRecExt);
#else
    ((appDbRec_t *) hdl)->discStatus = status;

    appDbBackupRecord((appDbRec_t *) hdl);
#endif //GP_NVM_DIVERSITY_ELEMENT_IF
}
/*************************************************************************************************/
/*!
 *  \brief  Get the cached handle list.
 *
 *  \param  hdl       Database record handle.
 *
 *  \return Pointer to handle list.
 */
/*************************************************************************************************/
uint16_t *AppDbGetHdlList(appDbHdl_t hdl)
{
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
    appDbRecExt_t *pRecExt = appDbFindExtRecord((appDbRec_t *)hdl);

    GP_ASSERT_DEV_INT(pRecExt != NULL);

    return pRecExt->hdlList;
#else
    return ((appDbRec_t *) hdl)->hdlList;
#endif //GP_NVM_DIVERSITY_ELEMENT_IF
}

/*************************************************************************************************/
/*!
 *  \brief  Set the cached handle list.
 *
 *  \param  hdl       Database record handle.
 *  \param  pHdlList  Pointer to handle list.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetHdlList(appDbHdl_t hdl, uint16_t *pHdlList)
{
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
    appDbRecExt_t *pRecExt = appDbFindExtRecord((appDbRec_t *)hdl);

    GP_ASSERT_DEV_INT(pRecExt != NULL);

    memcpy(pRecExt->hdlList, pHdlList, sizeof(pRecExt->hdlList));

    appDbBackupExtRecord(pRecExt);
#else
    memcpy(((appDbRec_t *) hdl)->hdlList, pHdlList, sizeof(((appDbRec_t *) hdl)->hdlList));

    appDbBackupRecord((appDbRec_t *) hdl);
#endif //GP_NVM_DIVERSITY_ELEMENT_IF

}
#endif //CORDIO_APPFRAMEWORK_DIVERSITY_ATT_CLIENT

/*************************************************************************************************/
/*!
 *  \brief  Get the device name.
 *
 *  \param  pLen      Returned device name length.
 *
 *  \return Pointer to UTF-8 string containing device name or NULL if not set.
 */
/*************************************************************************************************/
char *AppDbGetDevName(uint8_t *pLen)
{
    /* if first character of name is NULL assume it is uninitialized */
    if (appDb.devName[0] == 0)
    {
        *pLen = 0;
        return NULL;
    }
    else
    {
        *pLen = appDb.devNameLen;
        return appDb.devName;
    }
}

/*************************************************************************************************/
/*!
 *  \brief  Set the device name.
 *
 *  \param  len       Device name length.
 *  \param  pStr      UTF-8 string containing device name.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetDevName(uint8_t len, char *pStr)
{
    /* check for maximum device length */
    len = (len <= sizeof(appDb.devName)) ? len : sizeof(appDb.devName);

    memcpy(appDb.devName, pStr, len);
}

/*************************************************************************************************/
/*!
 *  \brief  Get address resolution attribute value read from a peer device.
 *
 *  \param  hdl        Database record handle.
 *
 *  \return TRUE if address resolution is supported in peer device. FALSE, otherwise.
 */
/*************************************************************************************************/
bool_t AppDbGetPeerAddrRes(appDbHdl_t hdl)
{
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
    appDbRecExt_t *pRecExt = appDbFindExtRecord((appDbRec_t *)hdl);

    GP_ASSERT_DEV_INT(pRecExt != NULL);
    return pRecExt->peerAddrRes;
#else
    return ((appDbRec_t *)hdl)->peerAddrRes;
#endif //GP_NVM_DIVERSITY_ELEMENT_IF

}

/*************************************************************************************************/
/*!
 *  \brief  Set address resolution attribute value for a peer device.
 *
 *  \param  hdl        Database record handle.
 *  \param  addrRes    Address resolution attribue value.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetPeerAddrRes(appDbHdl_t hdl, uint8_t addrRes)
{
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
    appDbRecExt_t *pRecExt = appDbFindExtRecord((appDbRec_t *)hdl);

    GP_ASSERT_DEV_INT(pRecExt != NULL);
    pRecExt->peerAddrRes = addrRes;

    appDbBackupExtRecord(pRecExt);
#else
    ((appDbRec_t *)hdl)->peerAddrRes = addrRes;

    appDbBackupRecord((appDbRec_t *) hdl);
#endif //GP_NVM_DIVERSITY_ELEMENT_IF
}

#ifdef CORDIO_APPFRAMEWORK_DIVERSITY_ATT_SERVER
/*************************************************************************************************/
/*!
 *  \brief  Get sign counter for a peer device.
 *
 *  \param  hdl        Database record handle.
 *
 *  \return Sign counter for peer device.
 */
/*************************************************************************************************/
uint32_t AppDbGetPeerSignCounter(appDbHdl_t hdl)
{
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
    appDbRecExt_t *pRecExt = appDbFindExtRecord((appDbRec_t *)hdl);

    GP_ASSERT_DEV_INT(pRecExt != NULL);
    return pRecExt->peerSignCounter;
#else
    return ((appDbRec_t *)hdl)->peerSignCounter;
#endif //GP_NVM_DIVERSITY_ELEMENT_IF
}

/*************************************************************************************************/
/*!
 *  \brief  Set sign counter for a peer device.
 *
 *  \param  hdl          Database record handle.
 *  \param  signCounter  Sign counter for peer device.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetPeerSignCounter(appDbHdl_t hdl, uint32_t signCounter)
{
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
    appDbRecExt_t *pRecExt = appDbFindExtRecord((appDbRec_t *)hdl);

    GP_ASSERT_DEV_INT(pRecExt != NULL);
    pRecExt->peerSignCounter = signCounter;

    appDbBackupExtRecord(pRecExt);
#else
    ((appDbRec_t *)hdl)->peerSignCounter = signCounter;

    appDbBackupRecord((appDbRec_t *) hdl);
#endif //GP_NVM_DIVERSITY_ELEMENT_IF
}
#endif //CORDIO_APPFRAMEWORK_DIVERSITY_ATT_SERVER

/*************************************************************************************************/
/*!
 *  \brief  Get the peer device added to resolving list flag value.
 *
 *  \param  hdl        Database record handle.
 *
 *  \return TRUE if peer device's been added to resolving list. FALSE, otherwise.
 */
/*************************************************************************************************/
bool_t AppDbGetPeerAddedToRl(appDbHdl_t hdl)
{
    return ((appDbRec_t *)hdl)->peerAddedToRl;
}

/*************************************************************************************************/
/*!
 *  \brief  Set the peer device added to resolving list flag to a given value.
 *
 *  \param  hdl           Database record handle.
 *  \param  peerAddedToRl Peer device added to resolving list flag value.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetPeerAddedToRl(appDbHdl_t hdl, bool_t peerAddedToRl)
{
    ((appDbRec_t *)hdl)->peerAddedToRl = peerAddedToRl;
    appDbBackupRecord((appDbRec_t *) hdl);
}

/*************************************************************************************************/
/*!
 *  \brief  Get the resolvable private address only attribute flag for a given peer device.
 *
 *  \param  hdl        Database record handle.
 *
 *  \return TRUE if RPA Only attribute is present on peer device. FALSE, otherwise.
 */
/*************************************************************************************************/
bool_t AppDbGetPeerRpao(appDbHdl_t hdl)
{
    return ((appDbRec_t *)hdl)->peerRpao;
}

/*************************************************************************************************/
/*!
 *  \brief  Set the resolvable private address only attribute flag for a given peer device.
 *
 *  \param  hdl        Database record handle.
 *  \param  peerRpao   Resolvable private address only attribute flag value.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AppDbSetPeerRpao(appDbHdl_t hdl, bool_t peerRpao)
{
    ((appDbRec_t *)hdl)->peerRpao = peerRpao;
    appDbBackupRecord((appDbRec_t *) hdl);
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbGetPeerAddress
 *
 *  \brief  Get address of a peer device.
 *
 *  \param  hdl        Database record handle.
 *
 *  \return Pointer to peer address.
 */
/*************************************************************************************************/
uint8_t *AppDbGetPeerAddress(appDbHdl_t hdl)
{
    if(((appDbRec_t *) hdl)->valid == TRUE)
    {
        return ((appDbRec_t *) hdl)->peerAddr;
    }
    else
    {
        return NULL;
    }
}

/*************************************************************************************************/
/*!
 *  \fn     AppDbGetPeerAddressType
 *
 *  \brief  Get address type of a peer device.
 *
 *  \param  hdl        Database record handle.
 *
 *  \return Peer address type.
 */
/*************************************************************************************************/
uint8_t AppDbGetPeerAddressType(appDbHdl_t hdl)
{
    if(((appDbRec_t *) hdl)->valid == TRUE)
    {
        return ((appDbRec_t *) hdl)->addrType;
    }
    else
    {
        return 0;
    }
}
