/*
 * Copyright (c) 2015-2016, GreenPeak Technologies
 * Copyright (c) 2017, Qorvo Inc
 *
 * This software is owned by Qorvo Inc
 * and protected under applicable copyright laws.
 * It is delivered under the terms of the license
 * and is intended and supplied for use solely and
 * exclusively with products manufactured by
 * Qorvo Inc.
 *
 *
 * THIS SOFTWARE IS PROVIDED IN AN "AS IS"
 * CONDITION. NO WARRANTIES, WHETHER EXPRESS,
 * IMPLIED OR STATUTORY, INCLUDING, BUT NOT
 * LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * QORVO INC. SHALL NOT, IN ANY
 * CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 * INCIDENTAL OR CONSEQUENTIAL DAMAGES,
 * FOR ANY REASON WHATSOEVER.
 *
 * $Header$
 * $Change$
 * $DateTime$
 *
 */

//#define GP_LOCAL_LOG

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLESECURITYCOPROCESSOR

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBle.h"
#include "gpBleComps.h"
#include "gpBleSecurityCoprocessor.h"
#include "gpBle_defs.h"
#include "gpRandom.h"
#include "gpEncryption.h"
#include "gpLog.h"

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
#include "gpBleLlcp.h"
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL

#ifdef GP_DIVERSITY_DEVELOPMENT
#include "gpPoolMem.h"
#endif //GP_DIVERSITY_DEVELOPMENT


#ifdef GP_HAL_DIVERSITY_SEC_CRYPTOSOC
#include "gpRandom_HASH_DRBG.h"
#endif //GP_HAL_DIVERSITY_SEC_CRYPTOSOC

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define BLE_SEC_NONCE_LENGTH    13
#define BLE_PACKET_COUNTER_DIRECTION_BIT_MASK       0x80

#ifdef GP_DIVERSITY_DEVELOPMENT
#define GPRANDOM_CTR_DRBG    1
#define SILEX_HASH_DRBG      2
#define GPHAL_BA431_ENTROPY  3
#define GPHAL_GPENTROPY      4
#define USABLE_RNG_SOURCES   GPHAL_GPENTROPY
#endif /* GP_DIVERSITY_DEVELOPMENT */

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct {
    UInt32 lsbs;
    UInt8 msbAndDirection;
} Ble_PacketCounter_t;

typedef struct {
    Ble_PacketCounter_t packetCounterIn;
    Ble_PacketCounter_t packetCounterOut;
    UInt8 initVector[BLE_SEC_INIT_VECTOR_LENGTH];
    UInt8 sessionKey[GP_HCI_ENCRYPTION_KEY_LENGTH];
} Ble_SecLinkContext_t;

typedef gpEncryption_Result_t (*Ble_EncryptionFunc_t)(gpEncryption_CCMOptions_t * pCCMOptions);

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

// The security link context per connection
static Ble_SecLinkContext_t Ble_SecLinkContext[BLE_LLCP_MAX_NR_OF_CONNECTIONS];

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static void Ble_ReverseArray(UInt8* pDst, UInt8* pSrc, UInt8 length);
static gpHci_Result_t Ble_AESEncrypt(UInt8* pInplaceBuffer, UInt8* pKey);
static gpHci_Result_t Ble_PopulateCCMOptions(gpEncryption_CCMOptions_t* pOptions, UInt8* pKey, UInt8* pNonce, Ble_SecLinkContext_t* pContext, gpPd_Loh_t* pPdLoh, Bool encrypt);
static void Ble_SecConstructNonce(Ble_SecLinkContext_t* pContext, UInt8* pNonce, Ble_PacketCounter_t* pPacketCounter);
static gpHci_Result_t Ble_SecurityCoprocessorCcmCommon(Ble_IntConnId_t connId, gpPd_Loh_t* pPdLoh, Ble_SecLinkContext_t* pContext, Bool encrypt);
static void Ble_IncrementPacketCounter(Ble_PacketCounter_t* pPacketCounter);
static void Ble_UpdatePduLength(Bool encrypt, gpPd_Loh_t* pPdLoh);


/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

void Ble_ReverseArray(UInt8* pDst, UInt8* pSrc, UInt8 length)
{
    IntLoop i;

    for(i = length - 1; i >=0; i--)
    {
        pDst[length - i - 1] = pSrc[i];
    }
}

// This is a wrapper around gpEncryption, to produce the correct output
gpHci_Result_t Ble_AESEncrypt(UInt8* pInplaceBuffer, UInt8* pKey)
{
    gpHal_Result_t result;
    gpEncryption_AESOptions_t AESoptions;
    UInt8 tmpKey[GP_HCI_ENCRYPTION_KEY_LENGTH];
    UInt8 tmpData[GP_HCI_ENCRYPTION_DATA_LENGTH];

    // We need to reverse both the key and the data in order for our encryption to work correctly
    Ble_ReverseArray(tmpKey, pKey, GP_HCI_ENCRYPTION_KEY_LENGTH);
    Ble_ReverseArray(tmpData, pInplaceBuffer, GP_HCI_ENCRYPTION_DATA_LENGTH);

    // Setup the encryption
    AESoptions.keylen = gpEncryption_AESKeyLen128;
    AESoptions.options = gpEncryption_KeyIdKeyPtr;
    result = gpEncryption_AESEncrypt(tmpData, tmpKey, AESoptions);

    if(result != gpHal_ResultSuccess)
    {
        return gpHci_ResultControllerBusy;
    }

    // Reverse output
    Ble_ReverseArray(pInplaceBuffer, tmpData, GP_HCI_ENCRYPTION_DATA_LENGTH);
    return gpHci_ResultSuccess;
}

// Debug code
#ifdef GP_LOCAL_LOG
void Ble_DumpCCMOptions(gpEncryption_CCMOptions_t* pOptions)
{
    GP_LOG_SYSTEM_PRINTF("pdHandle: %x",0, pOptions->pdHandle);
    GP_LOG_SYSTEM_PRINTF("datalen: %x dataoffset: %x",0,pOptions->dataLength, pOptions->dataOffset);
    GP_LOG_SYSTEM_PRINTF("auxlen: %x auxoffset: %x micLength: %x",0,pOptions->auxLength, pOptions->auxOffset, pOptions->micLength);
    GP_LOG_SYSTEM_PRINTF("Key:",0);
    gpLog_PrintBuffer(16, pOptions->pKey);
    GP_LOG_SYSTEM_PRINTF("Nonce",0);
    gpLog_PrintBuffer(13, pOptions->pNonce);
    gpLog_Flush();
}

#endif // GP_LOCAL_LOG

gpHci_Result_t Ble_PopulateCCMOptions(gpEncryption_CCMOptions_t* pOptions, UInt8* pKey, UInt8* pNonce, Ble_SecLinkContext_t* pContext, gpPd_Loh_t* pPdLoh, Bool encrypt)
{
    UInt16 pdLength = pPdLoh->length;

    if(!encrypt)
    {
        // In case of decrypt, do not take mic length into account for payload data
        if (pdLength > (BLE_SEC_MIC_LENGTH + BLE_PACKET_HEADER_SIZE))
        {
            pdLength -= BLE_SEC_MIC_LENGTH;
        }
        else
        {
            return gpHci_ResultAuthenticationFailure;
        }
    }
    // We need to reverse the key in order for the encryption to be correct
    Ble_ReverseArray(pKey, pContext->sessionKey, GP_HCI_ENCRYPTION_KEY_LENGTH);

    pOptions->pdHandle = pPdLoh->handle;
    pOptions->dataOffset = pPdLoh->offset + BLE_PACKET_HEADER_SIZE;     // Data starts after the 2 bytes header
    pOptions->dataLength = pdLength - BLE_PACKET_HEADER_SIZE;

#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
    UInt8 header = gpPd_ReadByte(pPdLoh->handle, pPdLoh->offset);
    if(header & BLE_DATA_PDU_HEADER_CP_BM)
    {
        // When AoA is supported, check if the CP bit is set.
        // If this is the case, encryption should start one byte later (CTEInfo is not encrypted)
        pOptions->dataOffset++;
        pOptions->dataLength--;
    }
#endif /* GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED */

    pOptions->auxOffset = pPdLoh->offset;                               // offset points to first header byte
    pOptions->auxLength = 1;                                            // a data is only the first byte of the header with NESN,SN and MD bits zero
    pOptions->micLength = BLE_SEC_MIC_LENGTH;
    pOptions->pKey = pKey;
    pOptions->pNonce = pNonce;

    return gpHci_ResultSuccess;
}

void Ble_SecConstructNonce(Ble_SecLinkContext_t* pContext, UInt8* pNonce, Ble_PacketCounter_t* pPacketCounter)
{
    // 32 packet counter lsbs
    MEMCPY(pNonce, &pPacketCounter->lsbs, sizeof(pPacketCounter->lsbs));

    // 7 packet counter msbs + 1 direction bit
    MEMCPY(&pNonce[4], &pPacketCounter->msbAndDirection, sizeof(pPacketCounter->msbAndDirection));

    // IV
    MEMCPY(&pNonce[5], pContext->initVector, BLE_SEC_INIT_VECTOR_LENGTH);
}

gpHci_Result_t Ble_SecurityCoprocessorCcmCommon(Ble_IntConnId_t connId, gpPd_Loh_t* pPdLoh, Ble_SecLinkContext_t* pContext, Bool encrypt)
{
    gpEncryption_CCMOptions_t ccmOptions;
    gpHal_Result_t result;
    Ble_EncryptionFunc_t encryptionFunction;
    Ble_PacketCounter_t* pPacketCounter;

    // Provide storage for key and nonce (will be filled in by populate function)
    UInt8 nonce[BLE_SEC_NONCE_LENGTH];
    UInt8 tmpKey[GP_HCI_ENCRYPTION_KEY_LENGTH];

    if(!BLE_IS_INT_CONN_HANDLE_VALID(connId))
    {
        GP_ASSERT_DEV_INT(false);
        return gpHci_ResultUnknownConnectionIdentifier;
    }

    if(pContext == NULL)
    {
        // In case no Context was provided, take the default one (from the ACL connection)
        pContext = &Ble_SecLinkContext[connId];
    }

    if(encrypt)
    {
        encryptionFunction = gpEncryption_CCMEncrypt;
        pPacketCounter = &pContext->packetCounterOut;
    }
    else
    {
        encryptionFunction = gpEncryption_CCMDecrypt;
        pPacketCounter = &pContext->packetCounterIn;
    }

    Ble_SecConstructNonce(pContext, nonce, pPacketCounter);
    Ble_IncrementPacketCounter(pPacketCounter);

    if(Ble_PopulateCCMOptions(&ccmOptions, tmpKey, nonce, pContext, pPdLoh, encrypt) != gpHci_ResultSuccess)
    {
        return gpHci_ResultAuthenticationFailure;
    }

    // Ble_DumpCCMOptions(&ccmOptions);

    result = encryptionFunction(&ccmOptions);

    if(result != gpHal_ResultSuccess)
    {
        return gpHci_ResultAuthenticationFailure;
    }

    // Encryption / decryption adds/removes a MIC - take this into account
    Ble_UpdatePduLength(encrypt, pPdLoh);

    return gpHci_ResultSuccess;
}

// Packet counter is 39 bit, does not fit proper into standard type ==> use increment function
void Ble_IncrementPacketCounter(Ble_PacketCounter_t* pPacketCounter)
{
    if(pPacketCounter->lsbs == 0xFFFFFFFF)
    {
        pPacketCounter->lsbs = 0;

        if((pPacketCounter->msbAndDirection & 0x7F) == 0x7F)
        {
            pPacketCounter->msbAndDirection &= BLE_PACKET_COUNTER_DIRECTION_BIT_MASK;
        }
        else
        {
            pPacketCounter->msbAndDirection++;
        }
    }
    else
    {
        pPacketCounter->lsbs++;
    }
}

void Ble_UpdatePduLength(Bool encrypt, gpPd_Loh_t* pPdLoh)
{
    UInt8 lengthByte = gpPd_ReadByte(pPdLoh->handle, pPdLoh->offset + 1);

    if(encrypt)
    {
        lengthByte += BLE_SEC_MIC_LENGTH;
        pPdLoh->length += BLE_SEC_MIC_LENGTH;
    }
    else
    {
        lengthByte -= BLE_SEC_MIC_LENGTH;
        pPdLoh->length -= BLE_SEC_MIC_LENGTH;
    }

    gpPd_WriteByte(pPdLoh->handle, pPdLoh->offset + 1, lengthByte);
}


/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBle_SecurityCoprocessorReset(Bool firstReset)
{
    // Reset local link context
    MEMSET(Ble_SecLinkContext, 0, sizeof(Ble_SecLinkContext_t)*BLE_LLCP_MAX_NR_OF_CONNECTIONS);
}

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
gpHci_Result_t gpBle_SecurityCoprocessorCalculateSessionKey(Ble_IntConnId_t connId, UInt8* pLtk, UInt8* pSkd, UInt8* pIv)
{
    gpHci_Result_t result;
    Ble_PacketCounter_t* pPacketCounterCToP;

    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(connId));

    MEMCPY(Ble_SecLinkContext[connId].initVector, pIv, sizeof(Ble_SecLinkContext[connId].initVector));

    // Reset packet counter
    MEMSET(&Ble_SecLinkContext[connId].packetCounterIn, 0, sizeof(Ble_PacketCounter_t));
    MEMSET(&Ble_SecLinkContext[connId].packetCounterOut, 0, sizeof(Ble_PacketCounter_t));

    // Add direction bit for the C to P direction
    if(Ble_LlcpIsMasterConnection(connId))
    {
        pPacketCounterCToP = &Ble_SecLinkContext[connId].packetCounterOut;
    }
    else
    {
        pPacketCounterCToP = &Ble_SecLinkContext[connId].packetCounterIn;
    }

    pPacketCounterCToP->msbAndDirection |= BLE_PACKET_COUNTER_DIRECTION_BIT_MASK;

    // Copy the key (will be modified in place)
    MEMCPY(Ble_SecLinkContext[connId].sessionKey, pSkd, GP_HCI_ENCRYPTION_KEY_LENGTH);

    result = Ble_AESEncrypt(Ble_SecLinkContext[connId].sessionKey, pLtk);
    return result;
}
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL

gpHci_Result_t gpBle_SecurityCoprocessorCcmEncryptAcl(Ble_IntConnId_t connId, gpPd_Loh_t* pPdLoh)
{
    return Ble_SecurityCoprocessorCcmCommon(connId, pPdLoh, NULL, true);
}

gpHci_Result_t gpBle_SecurityCoprocessorCcmDecryptAcl(Ble_IntConnId_t connId, gpPd_Loh_t* pPdLoh)
{
    return Ble_SecurityCoprocessorCcmCommon(connId, pPdLoh, NULL, false);
}


/*****************************************************************************
 *                    Public Service Function Definitions
 *****************************************************************************/

gpHci_Result_t gpBle_LeEncrypt(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;

    GP_LOG_PRINTF("Encrypt request",0);

    // Use Command complete event to reply to an encrypt command
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    // Copy data (encryption will be done in place)
    MEMCPY(pEventBuf->payload.commandCompleteParams.returnParams.encryptedData.encryptedData, pParams->LeEncrypt.data, GP_HCI_ENCRYPTION_DATA_LENGTH);

    result = Ble_AESEncrypt(pEventBuf->payload.commandCompleteParams.returnParams.encryptedData.encryptedData, pParams->LeEncrypt.key);

    return result;
}

gpHci_Result_t gpBle_LeRand(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    GP_LOG_PRINTF("Rand", 0);

    // Use Command complete event to reply to a rand command
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    gpRandom_GetFromDRBG(GP_HCI_RANDOM_DATA_LENGTH, pEventBuf->payload.commandCompleteParams.returnParams.randData.randomNumber);

    return gpHci_ResultSuccess;
}

#ifdef GP_DIVERSITY_DEVELOPMENT
gpHci_Result_t gpBle_VsdRNG(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    // Use Command Complete event to reply to a LeVsdRNG command
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    gpHci_Result_t res = gpHal_ResultSuccess;
    UInt8 numbytes = pParams->VsdRNG.numRandomBytes;
    UInt8 RngSource = pParams->VsdRNG.RNG_Source;

    pEventBuf->payload.commandCompleteParams.returnParams.VsdRandData.RNG_Source = RngSource;
    pEventBuf->payload.commandCompleteParams.returnParams.VsdRandData.numRandomBytes = 0;
    pEventBuf->payload.commandCompleteParams.returnParams.VsdRandData.pRandomData = NULL;

    if (numbytes > (255 - (2 /*cmnd returns params*/ + 4 /*standard HCI_CC evt params*/)))
    {
        return gpHci_ResultInvalidHCICommandParameters;
    }

    UInt8* pRandData = NULL;

    if ( (0 < numbytes) && (RngSource <= USABLE_RNG_SOURCES) )
    {
        pRandData = GP_POOLMEM_TRYMALLOC(numbytes + 1);

        if (NULL == pRandData)
        {
            return gpHci_ResultMemoryCapacityExceeded;
        }
    }
    else
    {
        return gpHci_ResultInvalidHCICommandParameters;
    }

    switch (RngSource)
    {
#ifdef GP_HAL_DIVERSITY_SEC_CRYPTOSOC
        case SILEX_HASH_DRBG:
        {
            res = gpRandom_HASH_DRBG_Generate(numbytes, pRandData);
            break;
        }

        case GPHAL_BA431_ENTROPY:
        {
            gpRandom_FillEntropyBuffer(pRandData, (UInt16)numbytes);
            break;
        }
#endif /* GP_HAL_DIVERSITY_SEC_CRYPTOSOC */
        case GPHAL_GPENTROPY:
        {
            UInt8 i;
            for(i=0;i < numbytes; i++)
            {
               pRandData[i] = gpHal_GetRandomSeed();
            }
            break;
        }
        default :
        {
            res = gpHci_ResultInvalidHCICommandParameters;
            break;
        }
    }

    if ( (gpHal_ResultSuccess != res) && (pRandData) )
    {
        gpPoolMem_Free(pRandData);
    }
    else
    {
        pEventBuf->payload.commandCompleteParams.returnParams.VsdRandData.numRandomBytes = numbytes;
        pEventBuf->payload.commandCompleteParams.returnParams.VsdRandData.pRandomData = pRandData;
    }

    return res;
}
#endif /* GP_DIVERSITY_DEVELOPMENT */
