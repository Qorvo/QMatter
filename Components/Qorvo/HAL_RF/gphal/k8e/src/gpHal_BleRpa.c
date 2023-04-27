/*
 * Copyright (c) 2016, GreenPeak Technologies
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


/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

//#define GP_LOCAL_LOG

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLE

#include "global.h"
#include "gpHal.h"
#include "gpSched.h"
#include "gpAssert.h"
#include "gpHal_Ble.h"
#include "gpHal_Ble_Manual.h"
#include "gpLog.h"
#include "gpHal_kx_gpm.h"
#include "gpHal_Ble_DEFS.h"
#include "gpHal_kx_public.h"
#include "gpRandom.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_BLE_RSPRADDR_IDENTITY_ENTRY_SIZE 8
#define GP_BLE_RSPRADDR_IRK_SIZE 16
#define GP_BLE_RSPRADDR_ENTRIES GP_DIVERSITY_BLE_MAX_NR_OF_RESOLVINGLIST_ENTRIES

// Event Info memory
#ifndef GP_COMP_CHIPEMU
#define GP_BLE_RES_PR_ADDR_IDENTITY_LIST_START  (((UIntPtr)&gpHal_BleRsPrAddr_IdentityList[0]))
#define GP_BLE_RES_PR_ADDR_PEER_IRK_LIST_START  (((UIntPtr)&gpHal_BleRsPrAddr_IRK_List[0]))
#define GP_BLE_RES_PR_ADDR_LOCAL_IRK_LIST_START  (((UIntPtr)&gpHal_BleRsPrAddr_IRK_List[GP_BLE_RSPRADDR_IRK_SIZE*GP_BLE_RSPRADDR_ENTRIES]))
#else // GP_COMP_CHIPEMU
extern UInt32 gpChipEmu_GetGpMicroStructPeerIdentityListStart(UInt32 gp_mm_ram_linear_start);
extern UInt32 gpChipEmu_GetGpMicroStructResolvingListStart(UInt32 gp_mm_ram_linear_start);
#define GP_BLE_RES_PR_ADDR_IDENTITY_LIST_START  (gpChipEmu_GetGpMicroStructPeerIdentityListStart(GP_MM_RAM_LINEAR_START))
#define GP_BLE_RES_PR_ADDR_PEER_IRK_LIST_START  (gpChipEmu_GetGpMicroStructResolvingListStart(GP_MM_RAM_LINEAR_START))
#define GP_BLE_RES_PR_ADDR_LOCAL_IRK_LIST_START (gpChipEmu_GetGpMicroStructResolvingListStart(GP_MM_RAM_LINEAR_START) + GP_BLE_RSPRADDR_IRK_SIZE * GP_BLE_RSPRADDR_ENTRIES)
#endif // GP_COMP_CHIPEMU

#define GP_BLE_RES_PR_ADDR_IDENTITY_LIST_RAM_LINEAR_OFFSET  GPHAL_BLE_RAM_ADDRESS_TO_START_OFFSET(GP_BLE_RES_PR_ADDR_IDENTITY_LIST_START)
#define GP_BLE_RES_PR_ADDR_IRK_LIST_RAM_LINEAR_OFFSET   GPHAL_BLE_RAM_ADDRESS_TO_START_OFFSET(GP_BLE_RES_PR_ADDR_PEER_IRK_LIST_START)

/******************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

#ifndef GP_COMP_CHIPEMU
/*
 * compile time verification of info structures
 * NOTE: Identity Entry size is not written to RT, so changes to GP_BLE_RSPRADDR_IDENTITY_ENTRY_SIZE
 *       must be applied in RT too (rx_filter.c).
 */
GP_COMPILE_TIME_VERIFY((GP_BLE_RSPRADDR_IDENTITY_ENTRY_SIZE % GP_WB_MAX_MEMBER_SIZE) == 0);
GP_COMPILE_TIME_VERIFY(GP_BLE_RSPRADDR_IDENTITY_ENTRY_SIZE >= GP_WB_BLE_RES_PR_IDENTITY_ENTRY_SIZE);
/*
 * Info: No compile time verification on size and alignment of gpHal_BleRsPrAddr_IRK_List.
 *       sizes are fixed length on both ARM and GpMicro.
 */

COMPILER_ALIGNED(GP_WB_MAX_MEMBER_SIZE) static UInt8 gpHal_BleRsPrAddr_IdentityList[(GP_BLE_RSPRADDR_IDENTITY_ENTRY_SIZE*GP_BLE_RSPRADDR_ENTRIES)] LINKER_SECTION(".lower_ram_retain_gpmicro_accessible");
/* does need to be accessible by gpmicro */
COMPILER_ALIGNED(GP_BLE_RSPRADDR_IRK_SIZE) static UInt8 gpHal_BleRsPrAddr_IRK_List[(GP_BLE_RSPRADDR_IRK_SIZE*GP_BLE_RSPRADDR_ENTRIES*2)] LINKER_SECTION(".lower_ram_retain_gpmicro_accessible");
#endif // GP_COMP_CHIPEMU

/*
 * The size of the Resolving list can never exceed GP_BLE_RSPRADDR_ENTRIES
 * (compile time constant). Beside this boundary condition, the Resolving
 * list can be grown and shrunk at run time.
 */
static UInt8 Ble_MaxResolvingListSize;

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static UInt8 gpHal_BleRpa_MatchPeerIdentity(Bool addrType, const BtDeviceAddress_t *pIdentityAddr);
static void gpHal_BleRpaSetListSize(UInt8 length);
static UInt8 gpHal_BleRpa_ReadPeerIdentityAddr(BtDeviceAddress_t *pIdentityAddr, UIntPtr identityEntryAddr);
static void gpHal_BleRpa_WritePeerIdentityAddr(UIntPtr identityEntryAddr, const BtDeviceAddress_t *pIdentityAddr, UInt8 flags);
static void BleRpa_SetRandomKey(UIntPtr keyPtr);
static void gpHal_BleRpa_GenerateRpa(BtDeviceAddress_t *pRpaAddr, gpHal_BleRpaHandle_t handle, UInt32 prand, UInt32 IRK_listBaseAddr);
/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

ALWAYS_INLINE UIntPtr gpBle_ResPrIdentityIdxToAddr(UInt8 idx)
{
    return GP_BLE_RES_PR_ADDR_IDENTITY_LIST_START + idx * GP_BLE_RSPRADDR_IDENTITY_ENTRY_SIZE;
}
ALWAYS_INLINE UIntPtr gpBle_ResPrLocalIrkIdxToAddr(UInt8 idx)
{
    return GP_BLE_RES_PR_ADDR_LOCAL_IRK_LIST_START + idx * GP_BLE_RSPRADDR_IRK_SIZE;
}
ALWAYS_INLINE UIntPtr gpBle_ResPrPeerIrkIdxToAddr(UInt8 idx)
{
    return GP_BLE_RES_PR_ADDR_PEER_IRK_LIST_START + idx * GP_BLE_RSPRADDR_IRK_SIZE;
}

UInt8 gpHal_BleRpaGetListSize(void)
{
    return GP_WB_READ_BLEFILT_RES_PR_LIST_LENGTH();
}

void gpHal_BleRpaSetListSize(UInt8 length)
{
    GP_WB_WRITE_BLEFILT_RES_PR_LIST_LENGTH(length);
}


UInt8 gpHal_BleRpa_ReadPeerIdentityAddr(BtDeviceAddress_t *pIdentityAddr, UIntPtr identityEntryAddr)
{
    UIntPtr identityAddrAddr  = identityEntryAddr + GP_WB_BLE_RES_PR_IDENTITY_ENTRY_PEER_IDENTITY_ADDRESS_ADDRESS;
    GP_HAL_READ_BYTE_STREAM(identityAddrAddr,&pIdentityAddr->addr,GP_WB_BLE_RES_PR_IDENTITY_ENTRY_PEER_IDENTITY_ADDRESS_LEN);
    return GP_WB_READ_BLE_RES_PR_IDENTITY_ENTRY_FLAGS(identityEntryAddr);
}

void gpHal_BleRpa_WritePeerIdentityAddr(UIntPtr identityEntryAddr, const BtDeviceAddress_t *pIdentityAddr, UInt8 flags)
{
    UIntPtr identityAddrAddr  = identityEntryAddr + GP_WB_BLE_RES_PR_IDENTITY_ENTRY_PEER_IDENTITY_ADDRESS_ADDRESS;
    GP_HAL_WRITE_BYTE_STREAM(identityAddrAddr,&pIdentityAddr->addr,GP_WB_BLE_RES_PR_IDENTITY_ENTRY_PEER_IDENTITY_ADDRESS_LEN);
    GP_WB_WRITE_BLE_RES_PR_IDENTITY_ENTRY_FLAGS(identityEntryAddr, flags);
}

void BleRpa_SetRandomKey(UIntPtr keyPtr)
{
    GP_WB_WRITE_U32(keyPtr+0,gpRandom_GenerateLargeRandom());
    GP_WB_WRITE_U32(keyPtr+4,gpRandom_GenerateLargeRandom());
    GP_WB_WRITE_U32(keyPtr+8,gpRandom_GenerateLargeRandom());
    GP_WB_WRITE_U32(keyPtr+12,gpRandom_GenerateLargeRandom());
}

void gpHal_BleRpa_GenerateRpa(BtDeviceAddress_t *pRpaAddr, gpHal_BleRpaHandle_t handle, UInt32 prand, UInt32 IRK_listBaseAddr)
{
    UInt8 idx;
    UInt8 pDataBuffer[16];
    UInt8* pKeyBase;
    UInt8 pKey[16];
    gpEncryption_AESOptions_t aesOptions;
    UInt32 hash;

    idx = handle.idx;

    GP_ASSERT_DEV_INT(BLE_RES_PR_IS_VALID_HANDLE(handle) && idx < Ble_MaxResolvingListSize);

    MEMSET(pDataBuffer, 0, sizeof(pDataBuffer));

    // Databuffer should be big-endian, copy prand to back of array (MSB first)
    pDataBuffer[13] = (prand >> 16) & 0xFF;
    pDataBuffer[14] = (prand >> 8) & 0xFF;
    pDataBuffer[15] = (prand) & 0xFF;

    intptr_t ptr = (GP_MM_RAM_ADDR_FROM_COMPRESSED((GP_BLE_RSPRADDR_IRK_SIZE)*IRK_listBaseAddr)) + idx*GP_BLE_RSPRADDR_IRK_SIZE;
    pKeyBase = (UInt8*) (ptr);
    // Keys are stored big endian, so no need to change this (aes also expects big-endian key)
    GP_HAL_READ_BYTE_STREAM((UIntPtr)pKeyBase, pKey, 16);
    aesOptions.keylen = gpEncryption_AESKeyLen128;
    aesOptions.options = gpEncryption_KeyIdKeyPtr;

    gpHal_AESEncrypt(pDataBuffer, pKey, aesOptions);

    // hash is 24 lsbits from databuffer
    hash = (pDataBuffer[15] | pDataBuffer[14] << 8 | pDataBuffer[13] << 16);

    GP_LOG_PRINTF("hash:%lx prand:%lx", 0, (unsigned long)hash, (unsigned long)prand);

    pRpaAddr->addr[0] = (hash & 0xFF); hash>>=8;
    pRpaAddr->addr[1] = (hash & 0xFF); hash>>=8;
    pRpaAddr->addr[2] = (hash & 0xFF);
    pRpaAddr->addr[3] = ((prand>>0)  & 0xFF);
    pRpaAddr->addr[4] = ((prand>>8)  & 0xFF);
    pRpaAddr->addr[5] = ((prand>>16) & 0xFF);

    GP_LOG_PRINTF("Generated rpa: %02x:%02x:%02x:%02x:%02x:%02x",0,
        pRpaAddr->addr[0],pRpaAddr->addr[1],pRpaAddr->addr[2],pRpaAddr->addr[3],pRpaAddr->addr[4],pRpaAddr->addr[5]);
}

UInt8 gpHal_BleRpa_MatchPeerIdentity(Bool addrTypeBit, const BtDeviceAddress_t *pIdentityAddr)
{
    UInt8 idx;
    UInt8 matchIdx = GP_BLE_RSPRADDR_ENTRIES;
    UInt8 listSize = gpHal_BleRpaGetListSize();

    for (idx=0; idx < listSize; ++idx)
    {
        UInt8 flags;
        Bool entry_addrType;
        UIntPtr identityAddr;
        UIntPtr identityAddrAddr;

        identityAddr = gpBle_ResPrIdentityIdxToAddr(idx);
        flags = GP_WB_READ_BLE_RES_PR_IDENTITY_ENTRY_FLAGS(identityAddr);

        entry_addrType = GP_WB_GET_BLE_RES_PR_IDENTITY_ENTRY_IS_RANDOM_PEER_ADDRESS_FROM_FLAGS(flags);

        identityAddrAddr = identityAddr + GP_WB_BLE_RES_PR_IDENTITY_ENTRY_PEER_IDENTITY_ADDRESS_ADDRESS;
        if ( entry_addrType == addrTypeBit &&
             0 == gpHal_wb_memcmp(pIdentityAddr->addr,identityAddrAddr,GP_WB_BLE_RES_PR_IDENTITY_ENTRY_PEER_IDENTITY_ADDRESS_LEN))
        {
            matchIdx = idx;
            break;
        }
    }
    return matchIdx;
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpHal_BleRpa_Init(void)
{
    UInt32 peer_irk_list_addr;
    UInt32 local_irk_list_addr;

    Ble_MaxResolvingListSize = GP_BLE_RSPRADDR_ENTRIES;

#ifndef GP_COMP_CHIPEMU
    COMPILE_TIME_ASSERT( (GP_BLE_RES_PR_ADDR_IDENTITY_LIST_RAM_LINEAR_OFFSET+sizeof(gpHal_BleRsPrAddr_IdentityList)) < 16384);
    COMPILE_TIME_ASSERT( (GP_BLE_RES_PR_ADDR_IRK_LIST_RAM_LINEAR_OFFSET+sizeof(gpHal_BleRsPrAddr_IRK_List)) < 16384);

    MEMSET(gpHal_BleRsPrAddr_IdentityList,0,sizeof(gpHal_BleRsPrAddr_IdentityList));
    MEMSET(gpHal_BleRsPrAddr_IRK_List,0,sizeof(gpHal_BleRsPrAddr_IRK_List));
#endif

    GP_WB_WRITE_BLEFILT_RES_PR_IDENTITY_LIST_BASE_ADDRESS((UInt16)GP_BLE_RES_PR_ADDR_IDENTITY_LIST_RAM_LINEAR_OFFSET);

    peer_irk_list_addr = GP_MM_RAM_ADDR_TO_COMPRESSED(GP_BLE_RES_PR_ADDR_PEER_IRK_LIST_START)/GP_BLE_RSPRADDR_IRK_SIZE;
    local_irk_list_addr = GP_MM_RAM_ADDR_TO_COMPRESSED(GP_BLE_RES_PR_ADDR_LOCAL_IRK_LIST_START)/GP_BLE_RSPRADDR_IRK_SIZE;
    GP_WB_WRITE_BLEFILT_RES_PR_PEER_IRK_LIST_BASE_ADDRESS(peer_irk_list_addr);
    GP_WB_WRITE_BLE_MGR_RES_PR_LOCAL_IRK_LIST_BASE_ADDRESS(local_irk_list_addr);

    // total entries in list
    gpHal_BleRpaSetListSize(0);
}

Bool gpHal_BleRpa_MatchPeer(gpHal_BleRpaHandle_t *pHandle, Bool addrTypeBit, const BtDeviceAddress_t *pIdentityAddr)
{
    UInt8 idx;

    idx = gpHal_BleRpa_MatchPeerIdentity(addrTypeBit, pIdentityAddr);
    if (idx == GP_BLE_RSPRADDR_ENTRIES)
    {
        pHandle->idx = GP_BLE_RSPRADDR_ENTRIES;
        pHandle->idx_is_valid = false;
        return false;
    }
    pHandle->idx = idx;
    pHandle->idx_is_valid = true;

    return true;
}

Bool gpHal_BleRpa_IsAllZerosLocalIrk(gpHal_BleRpaHandle_t handle)
{
    UInt8 idx;
    UIntPtr identityAddr;
    UInt8 flags;

    idx = handle.idx;
    if (!BLE_RES_PR_IS_VALID_HANDLE(handle) || Ble_MaxResolvingListSize <= idx)
    {
        GP_ASSERT_DEV_INT(false);
        return true;
    }

    identityAddr = gpBle_ResPrIdentityIdxToAddr(idx);
    flags = GP_WB_READ_BLE_RES_PR_IDENTITY_ENTRY_FLAGS(identityAddr);

    return GP_WB_GET_BLE_RES_PR_IDENTITY_ENTRY_ALL_ZEROS_LOCAL_IRK_FROM_FLAGS(flags);
}

Bool gpHal_BleRpa_IsAllZerosPeerIrk(gpHal_BleRpaHandle_t handle)
{
    UInt8 idx;
    UIntPtr identityAddr;
    UInt8 flags;

    idx = handle.idx;
    if (!BLE_RES_PR_IS_VALID_HANDLE(handle) || Ble_MaxResolvingListSize <= idx)
    {
        GP_ASSERT_DEV_INT(false);
        return true;
    }

    identityAddr = gpBle_ResPrIdentityIdxToAddr(idx);
    flags = GP_WB_READ_BLE_RES_PR_IDENTITY_ENTRY_FLAGS(identityAddr);

    return GP_WB_GET_BLE_RES_PR_IDENTITY_ENTRY_ALL_ZEROS_PEER_IRK_FROM_FLAGS(flags);
}

Bool gpHal_BleRpa_GeneratePeerRpa(BtDeviceAddress_t *pRpaAddr, gpHal_BleRpaHandle_t handle)
{
    UInt8 idx;
    UInt8 flags;
    UInt32 prand;
    UIntPtr identityAddr;

    idx = handle.idx;

    GP_ASSERT_DEV_INT(BLE_RES_PR_IS_VALID_HANDLE(handle) && idx < Ble_MaxResolvingListSize);

    identityAddr = gpBle_ResPrIdentityIdxToAddr(idx);
    flags = GP_WB_READ_BLE_RES_PR_IDENTITY_ENTRY_FLAGS(identityAddr);

    if (GP_WB_GET_BLE_RES_PR_IDENTITY_ENTRY_ALL_ZEROS_PEER_IRK_FROM_FLAGS(flags))
    {
        /* Means the higher software layers didn't check this */
        GP_ASSERT_DEV_INT(false);
        return false;
    }

    prand = gpHal_BleRpa_GetPrand();
    gpHal_BleRpa_GenerateRpa(pRpaAddr, handle, prand, GP_WB_READ_BLEFILT_RES_PR_PEER_IRK_LIST_BASE_ADDRESS());
    return true;
}

Bool gpHal_BleRpa_GenerateLocalRpa(BtDeviceAddress_t *pRpaAddr, gpHal_BleRpaHandle_t handle)
{
    UInt8 idx;
    UInt8 flags;
    UInt32 prand;
    UIntPtr identityAddr;

    idx = handle.idx;

    GP_ASSERT_DEV_INT(BLE_RES_PR_IS_VALID_HANDLE(handle) && idx < Ble_MaxResolvingListSize);

    identityAddr = gpBle_ResPrIdentityIdxToAddr(idx);
    flags = GP_WB_READ_BLE_RES_PR_IDENTITY_ENTRY_FLAGS(identityAddr);

    if (GP_WB_GET_BLE_RES_PR_IDENTITY_ENTRY_ALL_ZEROS_LOCAL_IRK_FROM_FLAGS(flags))
    {
        /* Means the higher software layers didn't check this */
        GP_ASSERT_DEV_INT(false);
        return false;
    }

    prand = gpHal_BleRpa_GetPrand();
    gpHal_BleRpa_GenerateRpa(pRpaAddr, handle, prand, GP_WB_READ_BLE_MGR_RES_PR_LOCAL_IRK_LIST_BASE_ADDRESS());
    return true;
}

Bool gpHal_BleRpa_GetPeerIdentity(BtDeviceAddress_t *pIdentityAddr, Bool *pAddrTypeBit, gpHal_BleRpaHandle_t rpaHandle)
{
    /*
     * This function should only be called if the address in @pIdentityAddr was successfully resolved.
     */
    UInt8 idx = rpaHandle.idx;

    if (!BLE_RES_PR_IS_VALID_HANDLE(rpaHandle) || Ble_MaxResolvingListSize <= idx)
    {
        GP_ASSERT_DEV_INT(false);
        return false;
    }

    UIntPtr identityEntryAddr = gpBle_ResPrIdentityIdxToAddr(idx);
    UInt8 identity_flags = gpHal_BleRpa_ReadPeerIdentityAddr(pIdentityAddr,identityEntryAddr);

    *pAddrTypeBit = GP_WB_GET_BLE_RES_PR_IDENTITY_ENTRY_IS_RANDOM_PEER_ADDRESS_FROM_FLAGS(identity_flags);

    return true;
}

gpHal_Result_t gpHal_BleRpa_SetDeviceInResolvingList(
    gpHal_BleSetInResolvingList_t *deviceInfo)
{
    UInt8 listIdx = 0;
    UIntPtr identityEntryAddr;
    UIntPtr peerIrkAddr;
    UIntPtr localIrkAddr;
    gpHal_BleRpaHandle_t handle;
    UInt8 zero_irk[GP_BLE_RSPRADDR_IRK_SIZE] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    UInt8 tmp_irk[GP_BLE_RSPRADDR_IRK_SIZE];
    UInt8 listLength = gpHal_BleRpaGetListSize();

    /* If there is already an entry for this peer we don't add a new one */
    if(gpHal_BleRpa_MatchPeer(&handle, deviceInfo->peerAddrTypeBit, deviceInfo->peerIdentityAddress))
    {
        listIdx = handle.idx;
    }
    else
    {
        listIdx = listLength;

        if (Ble_MaxResolvingListSize <= listIdx)
        {
            GP_LOG_PRINTF("listIdx %u exceeds the maximum resolving list size %u", 0, listIdx, Ble_MaxResolvingListSize);
            return gpHal_ResultInvalidRequest;
        }
        listLength++;
    }

    identityEntryAddr = gpBle_ResPrIdentityIdxToAddr(listIdx);
    peerIrkAddr = gpBle_ResPrPeerIrkIdxToAddr(listIdx);
    localIrkAddr = gpBle_ResPrLocalIrkIdxToAddr(listIdx);

    Bool localIrkAllZeros = (0==MEMCMP(zero_irk, deviceInfo->localIRK, GP_BLE_RSPRADDR_IRK_SIZE));
    Bool peerIrkAllZeros = (0==MEMCMP(zero_irk, deviceInfo->peerIRK, GP_BLE_RSPRADDR_IRK_SIZE));

    if (localIrkAllZeros)
    {
        /* Make sure to fill in random values for allzero peer irks -> so decryption fails when attackers use key 000... */
        /* TODO: check if zero key after resolving */
        BleRpa_SetRandomKey(localIrkAddr);
    }
    else
    {
        UInt8 idx;
        for (idx=0;idx<GP_BLE_RSPRADDR_IRK_SIZE;++idx)
        {
            /* hardware uses big endian keys*/
            tmp_irk[idx] = deviceInfo->localIRK[GP_BLE_RSPRADDR_IRK_SIZE-1-idx];
        }
        GP_HAL_WRITE_BYTE_STREAM(localIrkAddr, tmp_irk, GP_BLE_RSPRADDR_IRK_SIZE);
    }
    if (peerIrkAllZeros)
    {
        BleRpa_SetRandomKey(peerIrkAddr);
    }
    else
    {
        UInt8 idx;
        for (idx=0;idx<GP_BLE_RSPRADDR_IRK_SIZE;++idx)
        {
            /* hardware uses big endian keys*/
            tmp_irk[idx] = deviceInfo->peerIRK[GP_BLE_RSPRADDR_IRK_SIZE-1-idx];
        }
        GP_HAL_WRITE_BYTE_STREAM(peerIrkAddr, tmp_irk, GP_BLE_RSPRADDR_IRK_SIZE);
    }

    UInt8 identity_flags = 0;
    /* We set the address type as two individual bits so we can "mem_part_match" entries efficiently in RT */
    GP_WB_SET_BLE_RES_PR_IDENTITY_ENTRY_IS_PUBLIC_PEER_ADDRESS_TO_FLAGS(identity_flags, deviceInfo->peerAddrTypeBit ? 0 : 1);
    GP_WB_SET_BLE_RES_PR_IDENTITY_ENTRY_IS_RANDOM_PEER_ADDRESS_TO_FLAGS(identity_flags, deviceInfo->peerAddrTypeBit ? 1 : 0);
    GP_ASSERT_DEV_INT(identity_flags);
    GP_WB_SET_BLE_RES_PR_IDENTITY_ENTRY_PRIVACY_MODE_TO_FLAGS(identity_flags,deviceInfo->privacyMode);
    GP_WB_SET_BLE_RES_PR_IDENTITY_ENTRY_ALL_ZEROS_LOCAL_IRK_TO_FLAGS(identity_flags,localIrkAllZeros);
    GP_WB_SET_BLE_RES_PR_IDENTITY_ENTRY_ALL_ZEROS_PEER_IRK_TO_FLAGS(identity_flags,peerIrkAllZeros);

#ifdef GP_DIVERSITY_GPHAL_WHITELIST_UPDATE_CALLBACK
    UInt8 wlIndex = gpHal_BleWl_FindEntry(deviceInfo->peerAddrTypeBit?1:0, deviceInfo->peerIdentityAddress);
    GP_WB_SET_BLE_RES_PR_IDENTITY_ENTRY_WL_IDX_VALID_TO_FLAGS(identity_flags, GP_HAL_BLE_WL_ID_INVALID == wlIndex ? 0 : 1);
    GP_LOG_PRINTF("wlIndex: %x",0, wlIndex);
    GP_WB_WRITE_BLE_RES_PR_IDENTITY_ENTRY_WL_IDX(identityEntryAddr, wlIndex);
#endif // GP_DIVERSITY_GPHAL_WHITELIST_UPDATE_CALLBACK

    gpHal_BleRpa_WritePeerIdentityAddr(identityEntryAddr, deviceInfo->peerIdentityAddress, identity_flags);


    gpHal_BleRpaSetListSize(listLength);

    return gpHal_ResultSuccess;
}

gpHal_Result_t gpHal_BleRpa_SetPrivacyMode(gpHal_BleRpaHandle_t handle, UInt8 privacyMode)
{
    UInt8 identity_flags;
    UIntPtr identityEntryAddr;
    UInt8 idx;

    idx = handle.idx;
    if (!BLE_RES_PR_IS_VALID_HANDLE(handle) || Ble_MaxResolvingListSize <= idx)
    {
        GP_ASSERT_DEV_INT(false);
        return gpHal_ResultInvalidParameter;
    }

    identityEntryAddr = gpBle_ResPrIdentityIdxToAddr(idx);
    identity_flags = GP_WB_READ_BLE_RES_PR_IDENTITY_ENTRY_FLAGS(identityEntryAddr);
    GP_WB_SET_BLE_RES_PR_IDENTITY_ENTRY_PRIVACY_MODE_TO_FLAGS(identity_flags,privacyMode&0x01);
    GP_WB_WRITE_BLE_RES_PR_IDENTITY_ENTRY_FLAGS(identityEntryAddr, identity_flags);

    return gpHal_ResultSuccess;
}

gpHal_Result_t gpHal_BleRpa_RemoveDeviceFromResolvingList(gpHal_BleRpaHandle_t rpaHandle)
{
    UInt8 idx;

    idx = rpaHandle.idx;

    if (!BLE_RES_PR_IS_VALID_HANDLE(rpaHandle) || gpHal_BleRpaGetListSize() <= idx)
    {
        return gpHal_ResultInvalidParameter;
    }

    // move the last entry to the free spot:
    UInt8 length = gpHal_BleRpaGetListSize() - 1;
    if(length) // only when more than one address exists in the list
    {
        // move the Local IRK
        UInt8 buff[GP_BLE_RSPRADDR_IRK_SIZE];
        UIntPtr thisaddr = gpBle_ResPrLocalIrkIdxToAddr(idx);
        UIntPtr nextAddr = gpBle_ResPrLocalIrkIdxToAddr(length);
        GP_HAL_READ_BYTE_STREAM (nextAddr, buff, GP_BLE_RSPRADDR_IRK_SIZE);
        GP_HAL_WRITE_BYTE_STREAM(thisaddr, buff, GP_BLE_RSPRADDR_IRK_SIZE);

        // move the Peer IRK
        thisaddr = gpBle_ResPrPeerIrkIdxToAddr(idx);
        nextAddr = gpBle_ResPrPeerIrkIdxToAddr(length);
        GP_HAL_READ_BYTE_STREAM (nextAddr, buff, GP_BLE_RSPRADDR_IRK_SIZE);
        GP_HAL_WRITE_BYTE_STREAM(thisaddr, buff, GP_BLE_RSPRADDR_IRK_SIZE);

        // move the Identity address
        thisaddr = gpBle_ResPrIdentityIdxToAddr(idx);
        nextAddr = gpBle_ResPrIdentityIdxToAddr(length);
        GP_HAL_READ_BYTE_STREAM (nextAddr, buff, GP_BLE_RSPRADDR_IDENTITY_ENTRY_SIZE);
        GP_HAL_WRITE_BYTE_STREAM(thisaddr, buff, GP_BLE_RSPRADDR_IDENTITY_ENTRY_SIZE);
    }

    gpHal_BleRpaSetListSize(length);
    return gpHal_ResultSuccess;
}

gpHal_Result_t gpHal_BleRpa_ClearResolvingList(void)
{
    gpHal_BleRpaSetListSize(0);
    return gpHal_ResultSuccess;
}

UInt8 gpHal_BleRpa_ReadResolvingListSize(void)
{
    return gpHal_BleRpaGetListSize();
}

void gpHal_BleRpa_SetAddressResolutionEnable(Bool enable)
{
    GP_LOG_PRINTF("Enable RPA resolver: %u", 0, enable);
    GP_WB_WRITE_BLEFILT_RESOLVE_RES_PR_SRC(enable);
}

Bool gpHal_BleRpa_IsRpaEnabled(void)
{
    return GP_WB_READ_BLEFILT_RESOLVE_RES_PR_SRC();
}

gpHal_Result_t gpHal_BleRpa_SetPrand(UInt32 prand)
{
    GP_WB_WRITE_BLE_MGR_RES_PR_PRAND(prand);

    return gpHal_ResultSuccess;
}

UInt32 gpHal_BleRpa_GetPrand(void)
{
    return GP_WB_READ_BLE_MGR_RES_PR_PRAND();
}

#ifdef GP_DIVERSITY_GPHAL_WHITELIST_UPDATE_CALLBACK
void gpHal_BleWl_cbUpdateIndex(UInt8 addressType, const BtDeviceAddress_t* pAddress, UInt8 wlIndex)
{
    gpHal_BleRpaHandle_t handle;
    if(gpHal_BleRpa_MatchPeer(&handle, addressType==1 ? true:false, pAddress))
    {
        UInt8 listIdx = handle.idx;
        GP_LOG_PRINTF("gpHal_BleWl_cbUpdateIndex: %x %x",0,listIdx,wlIndex);
        UIntPtr identityEntryAddr = gpBle_ResPrIdentityIdxToAddr(listIdx);
        GP_WB_WRITE_BLE_RES_PR_IDENTITY_ENTRY_WL_IDX(identityEntryAddr, wlIndex);
        GP_WB_WRITE_BLE_RES_PR_IDENTITY_ENTRY_WL_IDX_VALID(identityEntryAddr, GP_HAL_BLE_WL_ID_INVALID == wlIndex ? 0 : 1);
    }
}
#endif //GP_DIVERSITY_GPHAL_WHITELIST_UPDATE_CALLBACK

UInt8 gpHal_BleRpaGetMaxListSize(void)
{
    return Ble_MaxResolvingListSize;
}

void gpHal_BleRpaSetMaxListSize(UInt8 size)
{
    if(size > GP_BLE_RSPRADDR_ENTRIES)
    {
        GP_ASSERT_DEV_INT(false);
        return;
    }
    Ble_MaxResolvingListSize = size;
}

/*
 *  The input arguments are also used as return values.
 *  This function is completely transparent for non- or unresolved RPA addresses
 *  Note: addressType is potentially in the range [0-3], but the path to get here
 *        only will set [0-1]
 */
UInt8 gpHal_BleRpaResolveAddress(BtDeviceAddress_t* rpaAddress, UInt8* addressType)
{
    UInt8 identityAddress = 0x02;
    if(*addressType >= identityAddress)
    {
        GP_ASSERT_DEV_INT(false);
        return BLE_RPA_IDX_INVALID;
    }
    if(! ((*addressType) && (0x40 == (rpaAddress->addr[5] & 0xC0))))
    { // Only resolve when the provided address is actually an RPA
        return BLE_RPA_IDX_INVALID;
    }
    UInt8 resolvingListSize = gpHal_BleRpa_ReadResolvingListSize();
    UInt32 rand = rpaAddress->addr[3] | rpaAddress->addr[4] << 8 | rpaAddress->addr[5] << 16;

    BtDeviceAddress_t tmpAddress;
    gpHal_BleRpaHandle_t handle = {0, true};

    for(handle.idx = 0; handle.idx < resolvingListSize; handle.idx++)
    {
        gpHal_BleRpa_GenerateRpa(&tmpAddress, handle, rand, GP_WB_READ_BLEFILT_RES_PR_PEER_IRK_LIST_BASE_ADDRESS());
        Bool match = !MEMCMP(&tmpAddress, rpaAddress, 3); // The hash is in the first 3 bytes
        if(match)
        {
            UIntPtr identityAddr_base = gpBle_ResPrIdentityIdxToAddr(handle.idx);
            *addressType = GP_WB_READ_BLE_RES_PR_IDENTITY_ENTRY_IS_RANDOM_PEER_ADDRESS(identityAddr_base) | identityAddress;
            GP_HAL_READ_BYTE_STREAM(identityAddr_base + 2, rpaAddress, sizeof(BtDeviceAddress_t));
            return handle.idx;
        }
    }
    // Resolving failed, we return the @argument rpaAddress unchanged
    return BLE_RPA_IDX_INVALID;
}
