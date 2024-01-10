/*
 * Copyright (c) 2016, GreenPeak Technologies
 * Copyright (c) 2017-2021, Qorvo Inc
 *
 * This file gives an implementation of the Non Volatile Memory component for internal FLASH on Kx chips with random write possibility.
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
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

//#define GP_LOCAL_LOG

#include "global.h"
#ifdef GP_NVM_USE_ASSERT_SAFETY_NET
#include "gpNvm_AssertSafetyNet.h"
#endif // #ifdef GP_NVM_USE_ASSERT_SAFETY_NET
#include <limits.h>
#include "hal.h"
#include "gpNvm.h"
#include "gpNvm_defs.h"
#include "gpLog.h"
#include "gpAssert.h"
#include "gpHal.h"
#include "gpUtils.h"
#include "gpSched.h"
#ifdef GP_NVM_DIVERSITY_USE_POOLMEM
#include "gpPoolMem.h"
#endif  //GP_NVM_DIVERSITY_USE_POOLMEM
#include "gpNvm_RW_Kx_SubpagedFlash.h"
#include "gpHal_kx_Flash.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_NVM

/*
 * Need the following definitions from gpHal_kx_Flash.h
 *   FLASH_SECTOR_SIZE              physical sector size in bytes
 *   FLASH_PAGE_SIZE                physical page size in bytes
 *   FLASH_PAGES_PER_SECTOR         number of pages per sector
 *   FLASH_WRITE_UNIT               number of bytes written simultaneously
 *
 * Restrictions:
 *   - Flash erase targets one complete sector of FLASH_SECTOR_SIZE bytes.
 *   - Flash write targets a multiple of FLASH_WRITE_UNIT bytes, aligned on FLASH_WRITE_UNIT boundary.
 *   - Flash write may not cross a FLASH_PAGE_SIZE boundary.
 *   - On K8A, average flash write size must be at least 12 bytes to avoid exceeding the page write limit.
 *   - Flash read may not read more than FLASH_PAGE_SIZE bytes.
 */

#define NVM_PAYLOADLENGTH_INDICATING_TOKENREMOVED       (0)

#define NVM_START_ADDR                                  (0)   //GP_MM_FLASH_LINEAR_START
#define GP_NVM_NBR_OF_LOOKUPTABLE_HANDLES               (3)

#define NUMBER_OF_SECTORS_TO_ALLOCATE_SIMULTANEOUSLY    (2)

#define NVM_MAX_SIZE                                    (GP_DATA_SECTION_SIZE_NVM)
#define NVM_MAX_NUMBER_SECTORS                          ((NVM_MAX_SIZE / FLASH_PAGE_SIZE) / NVM_NUMBER_PAGES_PER_SECTOR)
#define NVM_MAX_NUMBER_LOGICAL_SECTORS                  (NVM_MAX_NUMBER_SECTORS / GP_NVM_NBR_OF_REDUNDANT_SECTORS)
#define NVM_MAX_NUMBER_POOLS                            (GP_NVM_NBR_OF_POOLS)

#define NVM_SECTORID_INVALID                            (0xFF)
#define PAGEID_UNUSED                                   (0xFF)
#define NVM_SEQNUM_INVALID                              (0xFFFF)
#define MARK_BROKEN_AFTER_REPLACEMENT                   (true)
#define DONT_MARK_BROKEN_AFTER_REPLACEMENT              (false)

#define NVM_DIV_ROUND_UP(n, k)                          (((n) + ((k) - 1)) / (k))
#define NVM_ROUND_UP_TO_MULTIPLE(n, k)                  (NVM_DIV_ROUND_UP((n), (k)) * (k))
#define EXACT_TOKEN_MATCH(token1, tokenLength1, token2, tokenLength2) \
    (tokenLength1 == tokenLength2 && 0 == MEMCMP(token1, token2, tokenLength1))

/* Record Header
 * -------------
 *
 *  2 bytes  : CRC
 *  1 byte   : tokenLen
 *  1 byte   : reserved
 *  1 byte   : updateFreq
 *  1 byte   : payloadLen
 *  2 bytes  : seqNr
 *  "tokenLen" bytes   : tokenData
 *  "payloadLen" bytes : payloadData
 *
 * Record Header size = 8 bytes
 * Record size = (8 + tokenLen + payloadLen) bytes, rounded up to multiple of FLASH_WRITE_UNIT.
 */
#define NVM_RECORD_HEADER_OFFSET_TO_CRC                     (0)
#define NVM_RECORD_HEADER_CRC_BYTES                         (2)
#define NVM_RECORD_HEADER_OFFSET_TO_TOKENLENGTH             (NVM_RECORD_HEADER_OFFSET_TO_CRC + NVM_RECORD_HEADER_CRC_BYTES)
#define NVM_RECORD_HEADER_TOKENLENGTH_BYTES                 (1)
#define NVM_RECORD_HEADER_OFFSET_TO_RESERVED                (NVM_RECORD_HEADER_OFFSET_TO_TOKENLENGTH + NVM_RECORD_HEADER_TOKENLENGTH_BYTES)
#define NVM_RECORD_HEADER_ID_RESERVED_BYTES                 (1)
#define NVM_RECORD_HEADER_OFFSET_TO_FREQUENCY               (NVM_RECORD_HEADER_OFFSET_TO_RESERVED + NVM_RECORD_HEADER_ID_RESERVED_BYTES)
#define NVM_RECORD_HEADER_FREQUENCY_BYTES                   (1)
#define NVM_RECORD_HEADER_OFFSET_TO_PAYLOADLENGTH           (NVM_RECORD_HEADER_OFFSET_TO_FREQUENCY + NVM_RECORD_HEADER_FREQUENCY_BYTES)
#define NVM_RECORD_HEADER_PAYLOADLENGTH_BYTES               (1)
#define NVM_RECORD_HEADER_OFFSET_TO_SEQNUM_COUNTER          (NVM_RECORD_HEADER_OFFSET_TO_PAYLOADLENGTH + NVM_RECORD_HEADER_PAYLOADLENGTH_BYTES)
#define NVM_RECORD_HEADER_SEQNUM_COUNTER_BYTES              (2)
#define NVM_RECORD_HEADER_OFFSET_TO_BODY                    (NVM_RECORD_HEADER_OFFSET_TO_SEQNUM_COUNTER + NVM_RECORD_HEADER_SEQNUM_COUNTER_BYTES)
#define NVM_RECORD_HEADER_SIZE                              (NVM_RECORD_HEADER_OFFSET_TO_BODY)
#define NVM_RECORD_HEADER_OFFSET_TO_TOKEN                   (NVM_RECORD_HEADER_OFFSET_TO_BODY)
#define NVM_RECORD_HEADER_OFFSET_TO_PAYLOAD(TokenLength)    (NVM_RECORD_HEADER_OFFSET_TO_BODY + (TokenLength))
#define NVM_RECORD_SIZE(TokenLength, PayloadLength)         (NVM_RECORD_HEADER_SIZE + (TokenLength) + (PayloadLength))

// Record size. Rounded up to a multiple of FLASH_WRITE_UNIT bytes.
#define NVM_ALIGNED_RECORD_SIZE(TokenLength, PayloadLength) (NVM_ROUND_UP_TO_MULTIPLE(NVM_RECORD_SIZE((TokenLength), (PayloadLength)), FLASH_WRITE_UNIT))
#define NVM_MAX_ALIGNED_RECORD_SIZE                         (NVM_ROUND_UP_TO_MULTIPLE(NVM_MAX_RECORD_SIZE, FLASH_WRITE_UNIT))


/* Sector Header
 * -------------
 *
 * offset   size
 *  0        1      validflag
 *  1        1      sectorId
 *  2        1      updateFreq
 *  3        1      erasePending
 *
 * Sector Header size = 4 bytes, rounded up to multiple of FLASH_WRITE_UNIT.
 */
#define NVM_SECTOR_HEADER_OFFSET_TO_VALIDFLAG           (0)
#define NVM_SECTOR_HEADER_VALIDFLAG_BYTES               (1)
#define NVM_SECTOR_HEADER_OFFSET_TO_ID                  (NVM_SECTOR_HEADER_OFFSET_TO_VALIDFLAG + NVM_SECTOR_HEADER_VALIDFLAG_BYTES)
#define NVM_SECTOR_HEADER_ID_BYTES                      (1)
#define NVM_SECTOR_HEADER_OFFSET_TO_FREQUENCY           (NVM_SECTOR_HEADER_OFFSET_TO_ID + NVM_SECTOR_HEADER_ID_BYTES)
#define NVM_SECTOR_HEADER_FREQUENCY_BYTES               (1)
#define NVM_SECTOR_HEADER_OFFSET_TO_ERASEPENDING        (NVM_SECTOR_HEADER_OFFSET_TO_FREQUENCY + NVM_SECTOR_HEADER_FREQUENCY_BYTES)
#define NVM_SECTOR_HEADER_ID_ERASEPENDING_BYTES         (1)
#define NVM_SECTOR_HEADER_OFFSET_TO_END                 (NVM_SECTOR_HEADER_OFFSET_TO_ERASEPENDING + NVM_SECTOR_HEADER_ID_ERASEPENDING_BYTES)

// Sector header size. Rounded up to a multiple of FLASH_WRITE_UNIT bytes.
#define NVM_SECTOR_HEADER_SIZE                          NVM_ROUND_UP_TO_MULTIPLE(NVM_SECTOR_HEADER_OFFSET_TO_END, FLASH_WRITE_UNIT)

#define NVM_SECTOR_HEADER_VALIDFLAG_VALUE               (0xC3)
#define NVM_SECTOR_HEADER_ERASEPENDING_VALUE            (0xEE)

#define NVM_BROKEN_SECTOR_MARKER                        (0x44454144L)  // "DEAD"
#define NVM_WRITE_OFFSET_WRITELOCK_MW                   (0xFFFF)

#define NVM_INITIAL_GLOBAL_COUNTER                      (1)

#define MINIMAL_RECORD_BODYLEN                          (4)

#if (FLASH_SECTOR_SIZE == 512)
#define GP_NVM_MAXLOGSTRING                             (127)
#elif (FLASH_SECTOR_SIZE==1024)
#define GP_NVM_MAXLOGSTRING                             (255)
#else
#error "Incorrect FLASH_SECTOR_SIZE"
#endif //FLASH_SECTOR_SIZE

#define NVM_MIN_TOKEN_SIZE                              (1)
#define NVM_MAX_RECORD_SIZE                             (NVM_RECORD_HEADER_SIZE + GP_NVM_MAX_TOKENLENGTH + GP_NVM_MAX_PAYLOADLENGTH)
#define NVM_MAX_RECORD_OFFSET                           (FLASH_PAGE_SIZE - NVM_RECORD_HEADER_SIZE)

#define GP_NVM_MAX_BODYLENGTH                           (FLASH_PAGE_SIZE - NVM_SECTOR_HEADER_SIZE - NVM_RECORD_HEADER_SIZE)

#define NVM_SECTOR_ADDROFFSET(sec)      ((UInt32)(sec) * FLASH_SECTOR_SIZE)
#define NVM_PAGE_ADDROFFSET(sec,page)   (NVM_SECTOR_ADDROFFSET(sec) + (page) * FLASH_PAGE_SIZE)

#define SECTOROFFSET_IS_PACKABLE(x)     (((x) &0x03) == 0)
#define SECTOROFFSET_PACK(x)            ((x) >> 2)
#define SECTOROFFSET_UNPACK(x)          (((UInt32)(x)) << 2)

#define NVM_TAGCACHE_HANDLE (0)

#define NVM_LUTHANDLE_IS_VALID(i) (((i) != gpNvm_LookupTable_Handle_Invalid) && ((i) < GP_NVM_NBR_OF_LOOKUPTABLE_HANDLES))
#define NVM_LUTHANDLE_SET(i,pLut) do {if (NVM_LUTHANDLE_IS_VALID(i)) { Nvm_TokenLuts_Handle[(i)] = pLut;} } while(false)
#define NVM_LUTHANDLE_SET_INVALID(i) NVM_LUTHANDLE_SET(i,NULL)
#define NVM_LUTHANDLE_IS_ACTIVE(i) (NVM_LUTHANDLE_IS_VALID(i) && (Nvm_TokenLuts_Handle[(i)] != NULL))
#define NVM_LUTHANDLE_TO_LUT(i) (NVM_LUTHANDLE_IS_VALID(i) ? Nvm_TokenLuts_Handle[(i)]: NULL)

#define NVM_MASKED_SEQNUM_MASK ((1<<15)-1)
#define NVM_MASKED_SEQNUM_MAX ((1<<15)-1)
#define NVM_SEQNUM_MSB 0x8000
#define NVM_MAX_SEQNUM_SPREAD (NVM_MASKED_SEQNUM_MAX/4)
#define NVM_NBR_OF_LOGICAL_SECTORS_REQUIRED_FOR_DEFRAG (2)

#define NVM_DEFAULT_BLOCK_SIZE          (64)

// NVM_DEFAULT_BLOCK_SIZE must be multiple of FLASH_WRITE_UNIT and must evenly divide FLASH_PAGE_SIZE
GP_COMPILE_TIME_VERIFY((NVM_DEFAULT_BLOCK_SIZE % FLASH_WRITE_UNIT == 0) && (FLASH_PAGE_SIZE % NVM_DEFAULT_BLOCK_SIZE == 0));

// Sectors larger than 1024 bytes not supported by 8-bit packed sector offsets
GP_COMPILE_TIME_VERIFY(FLASH_SECTOR_SIZE <= 1024);

// Number of pages per sector is dictated by gpHal_Flash implementation.
GP_COMPILE_TIME_VERIFY(NVM_NUMBER_PAGES_PER_SECTOR == FLASH_PAGES_PER_SECTOR);

#ifndef GP_NVM_BACKGROUND_DEFRAG_RATIO
#define GP_NVM_BACKGROUND_DEFRAG_RATIO 75
#endif

#ifndef GP_NVM_BACKGROUND_DEFRAG_SECTOR_INTERVAL_US
#define GP_NVM_BACKGROUND_DEFRAG_SECTOR_INTERVAL_US 0
#endif

/** @brief Number of tokens supported by subpaged flash implementation.
 *  Can be different then amount of TAGS if NVM is used directly through Token API.
**/
#ifndef GP_NVM_NBR_OF_UNIQUE_TOKENS
#define GP_NVM_NBR_OF_UNIQUE_TOKENS GP_NVM_NBR_OF_UNIQUE_TAGS
#endif //GP_NVM_NBR_OF_UNIQUE_TOKENS
#if GP_NVM_NBR_OF_UNIQUE_TOKENS < GP_NVM_NBR_OF_UNIQUE_TAGS
#error "GP_NVM_NBR_OF_UNIQUE_TOKENS must support at minimum the GP_NVM_NBR_OF_UNIQUE_TAGS"
#endif //

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/
typedef UInt8    Nvm_PhySectorId_t;
typedef UIntLoop Nvm_SectorPool_Loop_t;
typedef UInt8    Nvm_TagCache_Id_t;
typedef UInt8    Nvm_LogSectorId_t;
typedef UIntLoop Nvm_SectorLoop_t;
typedef UInt8    Nvm_PackedSectorOffset_t;

typedef UInt16 Nvm_MaskedSeqNum_t;
typedef UInt16 Nvm_SeqNum_t;

//------------------------ TOKEN LUT ------------------------

typedef PACKED_PRE struct Nvm_TokenLut_Item_s
{
    Nvm_LogSectorId_t         logSecId;
    Nvm_LogSectorId_t prevLogSecId; // used while counting nbrOfSectorReferences
    Nvm_LogSectorId_t nbrOfSectorReferences;
    Nvm_PackedSectorOffset_t  packed_sectorOffset; //Flash location - UInt8 packed
    /* Actual token data location will be calculated on the fly */
} PACKED_POST Nvm_Token_t;

//Poolmem container - filling as much as possible in 1 RAM poolmem
typedef struct Nvm_TokenItemBlock_s Nvm_TokenBlock_t  ;

PACKED_PRE struct Nvm_TokenItemBlock_s
{
    gpNvm_KeyIndex_t         nrOfItemsUsed;
    gpNvm_KeyIndex_t         nrOfItemsAllocated; //Max items possible in ItemBlock - allocated memory
    Nvm_TokenBlock_t        *nextBlock;         //Linked list
    Nvm_Token_t              items[];            //Structs with references to flash
} PACKED_POST;

typedef PACKED_PRE struct
{
    gpNvm_KeyIndex_t                     blockIndex;
    Nvm_TokenBlock_t        *block;
} PACKED_POST Nvm_TokenLut_Iterator_t;


#define INITIALIZE_TOKENLUT_ITERATOR(x) \
do {                                    \
    (x).blockIndex = 0;                 \
    (x).block = NULL;                   \
} while(false)

typedef PACKED_PRE struct Nvm_TokenLut_s
{
    gpNvm_PoolId_t            poolId;
    UInt8                     tokenMaskLength;
    UInt8                     TokenMask[GP_NVM_MAX_TOKENLENGTH];
    Nvm_TokenBlock_t        *firstItemBlock;
    Nvm_TokenLut_Iterator_t   readNext_Iterator;
    Bool                      trailingWildcard;
    Bool includeInactive;
    Bool writeLock;
    gpNvm_KeyIndex_t nrOfItemsAllocated; //Max items possible in chain
} PACKED_POST Nvm_TokenLut_t;


#ifndef GP_NVM_DIVERSITY_USE_POOLMEM
#ifdef GP_NVM_DIVERSITY_VARIABLE_SETTINGS
// Construct's using unique tokens as define can't be used with variable setting
#error GP_NVM_DIVERSITY_VARIABLE_SETTINGS cannot be used without GP_NVM_DIVERSITY_USE_POOLMEM.
#endif //GP_NVM_DIVERSITY_VARIABLE_SETTINGS
typedef PACKED_PRE struct {
    Bool inUse;
    Nvm_TokenLut_t lut;
    Nvm_TokenBlock_t   itemBlock;
    Nvm_Token_t tokenItems[GP_NVM_NBR_OF_UNIQUE_TOKENS];
} PACKED_POST Nvm_BssData_t;
#endif //#ifndef GP_NVM_DIVERSITY_USE_POOLMEM

typedef PACKED_PRE struct
{
    gpNvm_PoolId_t poolId;
    Nvm_PhySectorId_t logSec;
    UIntLoop pageId;
    UInt32 offsetInSector;
    /* For ease of use provide the record header+token data in this struct instead of passing it through the API */
    UInt8 RecordData[NVM_RECORD_HEADER_SIZE + GP_NVM_MAX_TOKENLENGTH];
    Bool isActive;
} PACKED_POST Nvm_Pool_Iterator_t;

//------------------------ SECTOR CACHE ------------------------

typedef struct Nvm_SectorCache_s
{
    Nvm_PhySectorId_t phySectorNbr[GP_NVM_NBR_OF_REDUNDANT_SECTORS];
    UInt16                pageWriteOffset[NVM_NUMBER_PAGES_PER_SECTOR];
    UInt16                InactiveSum;
} Nvm_SectorLut_t;


typedef struct prevVersion_s
{
    gpNvm_Result_t lookUpResult;
    UInt8 payloadLength;
    UInt8 logSecId;
} Nvm_Token_PreviousVersion_t;

/** @enum gpNvm_DryRun_t */
//@{
/** @brief The action should not be performed */
#define gpNvm_DryRun_yes    true
/** @brief The action should be performed */
#define gpNvm_DryRun_no     false
/** @typedef gpNvm_DryRun_t
    @brief Only check if an action would succeed
*/
typedef Bool    gpNvm_DryRun_t;
//@}

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

#ifdef GP_NVM_DIVERSITY_VARIABLE_SIZE
// NVM size can be changed by the application before NVM initialization.
static UInt32 Nvm_Flash_Size                  = NVM_MAX_SIZE;
static UInt16 Nvm_Flash_NbrOfPhySectors       = NVM_MAX_NUMBER_SECTORS;
static UInt8  Nvm_Flash_NbrOfPools            = NVM_MAX_NUMBER_POOLS;
#else
// NVM size is fixed at compile time.
static const UInt32 Nvm_Flash_Size            = NVM_MAX_SIZE;
static const UInt16 Nvm_Flash_NbrOfPhySectors = NVM_MAX_NUMBER_SECTORS;
static const UInt8  Nvm_Flash_NbrOfPools      = NVM_MAX_NUMBER_POOLS;
#endif //GP_NVM_DIVERSITY_VARIABLE_SIZE
#ifdef GP_NVM_DIVERSITY_VARIABLE_SETTINGS
static UInt32 Nvm_NumberOfUniqueTokens        = GP_NVM_NBR_OF_UNIQUE_TOKENS;
// Define should no longer be used throughout code - used to initialize default only
#undef GP_NVM_NBR_OF_UNIQUE_TOKENS
#else
static const UInt32 Nvm_NumberOfUniqueTokens  = GP_NVM_NBR_OF_UNIQUE_TOKENS;
#endif

static Nvm_TokenLut_t* Nvm_TokenLuts_Handle[GP_NVM_NBR_OF_LOOKUPTABLE_HANDLES];
#ifndef GP_NVM_DIVERSITY_USE_POOLMEM
Nvm_BssData_t Nvm_BssData;
#endif


static Nvm_PhySectorId_t Nvm_Pool_LastAllocatedPhySector[NVM_MAX_NUMBER_POOLS];
static gpNvm_KeyIndex_t Nvm_Pool_NumberOfUniqueTokens[NVM_MAX_NUMBER_POOLS];

static const UInt8 Nvm_PoolId_ScheduleTrick[1];

// Start address of NVM flash section. Obtained from linkerscript
extern const UIntPtr gpNvm_Start;

#if defined(GP_NVM_DIVERSITY_VARIABLE_SETTINGS)
// Variable start address of NVM flash section to use.
// gpNvm_SetVariableSettings must be called before Init() to change the pointer
#if defined(GP_COMP_CHIPEMU)
// External variable defined in test - can't be used as initializer.
UIntPtr Nvm_Start;
#else
UIntPtr Nvm_Start = ((UIntPtr)&gpNvm_Start);
#endif
#define Nvm_Base ((UIntPtr)Nvm_Start)
#else
#if defined(GP_COMP_CHIPEMU)
// External variable defined in test
#define Nvm_Base ((UIntPtr)gpNvm_Start)
#else
// Linkerscript symbol for NVM region
#define Nvm_Base ((UIntPtr)&gpNvm_Start)
#endif
#endif //GP_NVM_DIVERSITY_VARIABLE_SETTINGS

typedef struct {
    Nvm_MaskedSeqNum_t maskedSeqNum;
    Nvm_LogSectorId_t logSecId;
} Nvm_SeqNumTrackPair_t;

static Nvm_MaskedSeqNum_t Nvm_MaskedSeqNum_Highest[NVM_MAX_NUMBER_POOLS];
static Nvm_SeqNumTrackPair_t Nvm_MaskedSeqNum_Lowest[NVM_MAX_NUMBER_POOLS];
static Nvm_SeqNumTrackPair_t Nvm_MaskedSeqNum_LowestInUpperHalf[NVM_MAX_NUMBER_POOLS];
static Nvm_MaskedSeqNum_t Nvm_MaskedSeqNum_HighestInLowerHalf[NVM_MAX_NUMBER_POOLS];

#define NVM_IS_SEQNUM_VALID(x) (x != NVM_SEQNUM_INVALID)


#ifdef GP_NVM_DIVERSITY_VARIABLE_SIZE
static UInt8 Nvm_Pool_NbrOfSectors[NVM_MAX_NUMBER_POOLS] =
#else
static const UInt8 Nvm_Pool_NbrOfSectors[NVM_MAX_NUMBER_POOLS] =
#endif //GP_NVM_DIVERSITY_VARIABLE_SIZE
{
    GP_NVM_POOL_1_NBR_OF_PHY_SECTORS,
#if GP_NVM_NBR_OF_POOLS > 1
    GP_NVM_POOL_2_NBR_OF_PHY_SECTORS,
#endif
#if GP_NVM_NBR_OF_POOLS > 2
    GP_NVM_POOL_3_NBR_OF_PHY_SECTORS,
#endif
#if GP_NVM_NBR_OF_POOLS > 3
    GP_NVM_POOL_4_NBR_OF_PHY_SECTORS,
#endif
};


static UInt8 Nvm_DefragmentationDisableCounter = 0;

//------------------------ SECTOR CACHE ------------------------

static Nvm_SectorLut_t Nvm_SectorLuts[NVM_MAX_NUMBER_LOGICAL_SECTORS];

static UInt8 Nvm_Pool_PhySectors_GetNbr(gpNvm_PoolId_t poolId)
{
    GP_ASSERT_SYSTEM(poolId < Nvm_Flash_NbrOfPools);
    return Nvm_Pool_NbrOfSectors[poolId];
}

static UInt8 Nvm_Pool_LogSectors_GetNbr(gpNvm_PoolId_t poolId)
{
    GP_ASSERT_SYSTEM(poolId < Nvm_Flash_NbrOfPools);
    return Nvm_Pool_NbrOfSectors[poolId] / GP_NVM_NBR_OF_REDUNDANT_SECTORS;
}

Nvm_LogSectorId_t Nvm_Pool_GetLogSectorOffset(gpNvm_PoolId_t poolId)
{
    UInt8 offset = 0;
    UInt8 i;
    for (i =0; i < poolId; i++ )
    {
        offset += Nvm_Pool_LogSectors_GetNbr(i);
    }
    return offset;
}

Nvm_PhySectorId_t Nvm_Pool_GetPhySectorOffset(gpNvm_PoolId_t poolId)
{
    return Nvm_Pool_GetLogSectorOffset(poolId) * GP_NVM_NBR_OF_REDUNDANT_SECTORS;
}

#if GP_NVM_NBR_OF_POOLS == 1
#define Nvm_Pool_SectorLut_Get(poolId) (&Nvm_SectorLuts[0])
#else
static Nvm_SectorLut_t *Nvm_Pool_SectorLut_Get(gpNvm_PoolId_t poolId)
{
    return &Nvm_SectorLuts[Nvm_Pool_GetLogSectorOffset(poolId)];
}
#endif

/*****************************************************************************
 *                    External Data Definition
 *****************************************************************************/

const UIntPtr gpNvm_NvmBaseAddr = (NVM_START_ADDR);

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static void Nvm_Pools_EraseCorrupted(void);
static void Nvm_LogSector_Init(Nvm_SectorLut_t *cacheEntry);
static void Nvm_SectorLuts_Init(void);
static Bool Nvm_LogSector_InUse(gpNvm_PoolId_t poolId, Nvm_LogSectorId_t id);
static void Nvm_Pool_SectorLut_Build(gpNvm_PoolId_t poolId);
static void Nvm_SectorLuts_LoadDetails(void);
static void Nvm_SectorLuts_Load(void);
static void Nvm_Pool_CalculateInactiveSums(gpNvm_PoolId_t poolId);
static void Nvm_LogSector_Load(gpNvm_PoolId_t poolId, Nvm_LogSectorId_t id);
static void Nvm_Pool_RegisterPhySect(gpNvm_PoolId_t poolId, Nvm_LogSectorId_t id, Nvm_PhySectorId_t phySecNbr);
#if GP_NVM_NBR_OF_REDUNDANT_SECTORS > 1
static void Nvm_Pool_AssureRedundancy(gpNvm_PoolId_t poolId);
static void Nvm_LogSector_ReplaceBroken(gpNvm_PoolId_t poolId, Nvm_LogSectorId_t id, UInt8 corruptedMirror, Bool markBroken);
#endif //GP_NVM_NBR_OF_REDUNDANT_SECTORS > 1
#if GP_NVM_NBR_OF_REDUNDANT_SECTORS == 1
static void Nvm_LogSector_SalvageEntriesFromBrokenSector(gpNvm_PoolId_t poolId, Nvm_PhySectorId_t phySecId);
#endif /* GP_NVM_NBR_OF_REDUNDANT_SECTORS == 1 */

static UInt16 Nvm_Flash_FreeSpaceLeftInPage(UInt16 freeStart);
static Nvm_LogSectorId_t Nvm_Pool_Record_Allocate(
    gpNvm_PoolId_t poolId,
    UInt16 lengthOfNewData,
    Nvm_LogSectorId_t *pLogSecId,
    UInt8 *pPageId,
    gpNvm_DryRun_t dryRun
);
static Bool Nvm_TokenLut_TokenBelongs(
    Nvm_TokenLut_t *lut,
    gpNvm_PoolId_t poolId,
    UInt8 tokenLength,
    UInt8 *pToken
);
static Bool Nvm_TokenLut_Allocate(
    Nvm_TokenLut_t **lut,
    gpNvm_KeyIndex_t maxNrMatches
);
static gpNvm_Result_t Nvm_TokenLut_Populate(
    Nvm_TokenLut_t *lut,
    gpNvm_PoolId_t poolIdSpec,
    UInt8 tokenMaskLengthSpec,
    UInt8* pTokenMask,
    gpNvm_KeyIndex_t maxNrMatches,
    gpNvm_KeyIndex_t *pNrOfMatches,
    Bool trailingWildcard,
    Bool includeInactive
);
static void Nvm_TokenLut_Free(Nvm_TokenLut_t *lut);
static gpNvm_Result_t Nvm_ReadUnique(
    Nvm_TokenLut_t *lut,
    gpNvm_PoolId_t poolId,
    UInt8 tokenMaskLength,
    UInt8* pTokenMask,
    UInt8 maxDataLength,
    UInt8* pDataLength,
    UInt8* pData
);

static void Nvm_Reset(void);

static gpNvm_Result_t Nvm_Pool_Record_Add(
                            gpNvm_PoolId_t poolId,
                            UInt8 tokenLength,
                            UInt8* pToken,
                            UInt8 dataLength,
                            UInt8* pData,
                            Nvm_Token_PreviousVersion_t prevVersion
);

static gpNvm_Result_t Nvm_Write(
                            gpNvm_PoolId_t poolId,
                            UInt8 tokenLength,
                            UInt8* pToken,
                            UInt8 dataLength,
                            UInt8* pData
);
static Bool Nvm_TokenLutIter_IsAtEmptySpot(
    Nvm_TokenLut_Iterator_t *iter
);

static gpNvm_Result_t Nvm_Token_GetRecordHeader(
    gpNvm_PoolId_t poolId,
    Nvm_Token_t* item,
    UInt8 header[NVM_RECORD_HEADER_SIZE]
);

static gpNvm_Result_t Nvm_TokenLut_Query(
    Nvm_Token_t** itemResult,
    Nvm_TokenLut_t *lut,
    UInt8 tokenMaskLength,
    const UInt8 *pTokenMask,
    Bool strictMatching,
    Nvm_TokenLut_Iterator_t *iter
);

static void Nvm_SectorPool_ScheduleDefrag(UInt32 rel_time, gpNvm_PoolId_t poolId);
static void Nvm_Pool_ReclaimOutdatedSpace_wrapper(void* x);
static UInt8 Nvm_Pool_ReclaimOutdatedSpace(gpNvm_PoolId_t poolId);

static Nvm_SectorLoop_t Nvm_LogSector_Create(
    gpNvm_PoolId_t poolId
);
static UInt32 Nvm_LogSector_GetPhyFlashOffset(gpNvm_PoolId_t poolId, Nvm_LogSectorId_t id);

static gpNvm_Result_t Nvm_TokenLut_AllocateHandle(
    gpNvm_LookupTable_Handle_t* h
);
static Bool Nvm_TokenLutIter_Start(
    Nvm_TokenLut_t *lut,
    Nvm_TokenLut_Iterator_t *iter
);

static Bool Nvm_TokenLutIter_IsAtEnd(
    Nvm_TokenLut_Iterator_t *iter
);
static INLINE Nvm_Token_t *Nvm_TokenLutIter_GetItem(
    Nvm_TokenLut_Iterator_t *iter
);
static gpNvm_Result_t Nvm_Token_GetRecordToken(
    gpNvm_PoolId_t poolId,
    Nvm_Token_t* item,
    UInt8 TokenSize,
    UInt8* pData
);
static gpNvm_Result_t Nvm_Token_GetRecord(
    gpNvm_PoolId_t poolId,
    Nvm_Token_t *item,
    UInt8 maxTokenLength,
    UInt8 *pToken,
    UInt8 *pTokenLength,
    UInt8 maxDataLength,
    UInt8* pDataLength,
    UInt8* pData
);
static Bool Nvm_TokenLutIter_Next(
    Nvm_TokenLut_Iterator_t *iter
);

static Bool Nvm_TokenLutIter_NextFree(Nvm_TokenLut_Iterator_t* iter);

static void Nvm_TokenLuts_Update(
    gpNvm_PoolId_t poolId,
    gpNvm_UpdateFrequency_t updateFrequency,
    UInt8 tokenLength,
    UInt8* pToken,
    Nvm_LogSectorId_t logSecId,
    UInt16 offsetInSector
);

static Nvm_LogSectorId_t Nvm_Pool_GetOldestSector(
    gpNvm_PoolId_t poolId
);
static Nvm_SeqNum_t Nvm_Pool_SeqNum_GetHighest(
    gpNvm_PoolId_t poolId
);
static void Nvm_SectorPool_ScheduleRenumber(
    UInt32 rel_time,
    gpNvm_PoolId_t poolId
);

static void Nvm_Pool_Relocate_wrapper(void* x);
static gpNvm_Result_t Nvm_LogSector_RelocateActiveData(
    gpNvm_PoolId_t poolId,
    Nvm_LogSectorId_t logSecId
);
static Bool Nvm_Pool_IsUrgentDefragRequired(gpNvm_PoolId_t poolId);

static void Nvm_Pool_SetErasePending(gpNvm_PoolId_t poolId);
static Bool Nvm_Pool_IsErasePending(gpNvm_PoolId_t poolId);
static void Nvm_Pools_Process_ErasePending(void);
static void Nvm_Flash_PhysicalErase(gpNvm_PoolId_t poolId);

static gpNvm_Result_t Nvm_AcquireLutHandle(
    gpNvm_LookupTable_Handle_t* pHandle,
    gpNvm_PoolId_t poolId,
    UInt8 tokenLength,
    UInt8* pToken,
    Bool* freeAfterUse,
    gpNvm_KeyIndex_t maxNbrOfMatches,
    Bool trailingWildcard,
    Bool includeInactive
);
gpNvm_Result_t gpNvm_BuildLookupMatch(
    gpNvm_LookupTable_Handle_t* pHandle,
    gpNvm_PoolId_t poolIdSpec,
    gpNvm_UpdateFrequency_t updateFrequencySpec,
    UInt8 tokenMaskLengthSpec,
    UInt8* pTokenMask,
    gpNvm_KeyIndex_t maxNrMatches,
    gpNvm_KeyIndex_t *pNrOfMatches,
    Bool trailingWildcard,
    Bool includeInactive
);
static gpNvm_Result_t Nvm_Token_IsDeleted(
    gpNvm_PoolId_t poolId,
    Nvm_Token_t* lutItem,
    Bool* isDeleted);

static Bool Nvm_Pool_Iterator_Read(Nvm_Pool_Iterator_t* iter);
static Bool Nvm_Pool_Iterator_NextPage(Nvm_Pool_Iterator_t* iter);
static Bool Nvm_Pool_Iterator_NextSector(Nvm_Pool_Iterator_t* iter);
static Bool Nvm_Pool_Iterator_Start(gpNvm_PoolId_t poolId, Nvm_Pool_Iterator_t* iter);
/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

//------------------------ MISC HELPER METHODS ------------------------

static INLINE UInt16 Nvm_Buf_Get_UInt16(const UInt8* pBuf, UIntPtr offset)
{
    UInt16 v;
    MEMCPY(&v, pBuf + offset, 2);
    return v;
}

static INLINE void Nvm_Buf_Put_UInt16(UInt8* pBuf, UIntPtr offset, UInt16 v)
{
    MEMCPY(pBuf + offset, &v, 2);
}

//------------------------ INITIALISATION ------------------------

static void Nvm_TokenLut_Init(Nvm_TokenLut_t* lut)
{
    lut->poolId = gpNvm_PoolId_AllPoolIds;
    lut->tokenMaskLength = 0;
    MEMSET(lut->TokenMask, 0, GP_NVM_MAX_TOKENLENGTH);
    lut->firstItemBlock = NULL;
    lut->readNext_Iterator.block = lut->firstItemBlock;
    lut->readNext_Iterator.blockIndex = 0;
    lut->trailingWildcard = false;
}

//------------------------ FLASH ACCESS ------------------------

/* Read bytes from flash.
 * NOTE: Can not read more than FLASH_PAGE_SIZE bytes
 */
static gpHal_FlashError_t Nvm_Flash_Read(UInt32 offset, UInt16 length_8, UInt8* pData_8)
{
    GP_ASSERT_DEV_EXT(offset < Nvm_Flash_Size && length_8 <= Nvm_Flash_Size - offset);
    GP_ASSERT_DEV_INT(length_8 < FLASH_PAGE_SIZE);
    return gpHal_FlashRead(Nvm_Base + offset, length_8, pData_8);
}

/* Write to flash.
 * NOTE: Write area may not cross FLASH_PAGE_SIZE boundary.
 * NOTE: Offset and length must be aligned to FLASH_WRITE_UNIT bytes.
 * NOTE: Already written bytes may not be re-written unless the sector is erased first.
 * NOTE: On K8A, all writes except the sector header are at least 12 bytes to avoid exceeding maximum page write time.
 */
static gpHal_FlashError_t Nvm_Flash_Write(UInt32 offset, UInt16 length_8, UInt32* pData_32)
{

    gpHal_FlashError_t result;
    GP_ASSERT_DEV_EXT(offset < Nvm_Flash_Size && length_8 <= Nvm_Flash_Size - offset);
    GP_ASSERT_DEV_INT(offset % FLASH_PAGE_SIZE + length_8 <= FLASH_PAGE_SIZE);
    GP_ASSERT_DEV_INT(offset % FLASH_WRITE_UNIT == 0 && length_8 % FLASH_WRITE_UNIT == 0);
    GP_ASSERT_DEV_INT(offset % FLASH_PAGE_SIZE == 0 || length_8 >= 12);

    result = gpHal_FlashWrite(Nvm_Base + offset, length_8 / 4, pData_32);

    return result;
}

static gpHal_FlashError_t Nvm_Flash_EraseSector(Nvm_PhySectorId_t sectorId)
{
    gpHal_FlashError_t result;
    GP_ASSERT_DEV_EXT(sectorId < Nvm_Flash_NbrOfPhySectors);

    result = gpHal_FlashEraseSector(Nvm_Base + NVM_SECTOR_ADDROFFSET(sectorId));

    return result;
}

//------------------------ PHYSICAL SECTOR POOL ------------------------

static INLINE Bool Nvm_PhySector_IsValidHeader(const UInt8* pHeader)
{
    return (pHeader[NVM_SECTOR_HEADER_OFFSET_TO_VALIDFLAG] == NVM_SECTOR_HEADER_VALIDFLAG_VALUE);
}

static INLINE Bool Nvm_PhySector_IsBroken(const void* dataBlock)
{
    const UInt32 pageBrokenLabel = NVM_BROKEN_SECTOR_MARKER;
    return (MEMCMP(dataBlock, &pageBrokenLabel, sizeof(pageBrokenLabel)) == 0);
}

static void Nvm_PhySector_MarkBroken(Nvm_PhySectorId_t phySecNbr)
{
    UInt32 writeBuf[NVM_DEFAULT_BLOCK_SIZE / 4];
    UInt32 i, offset;

    GP_LOG_PRINTF("MarkBroken sec:%d", 0, phySecNbr);
    for (i = 0; i < sizeof(writeBuf)/4; i++)
    {
        writeBuf[i] = NVM_BROKEN_SECTOR_MARKER;
    }
    gpHal_FlashEraseSectorNoVerify(Nvm_Base + NVM_SECTOR_ADDROFFSET(phySecNbr));

    for (offset = 0; offset < FLASH_SECTOR_SIZE; offset += sizeof(writeBuf))
    {
        // use dedicated write function, without verify
        gpHal_FlashWriteNoVerify(Nvm_Base + NVM_SECTOR_ADDROFFSET(phySecNbr) + offset, sizeof(writeBuf) / 4, writeBuf);

    }
}

static Bool Nvm_PhySector_InUse(Nvm_PhySectorId_t phySecId)
{
    UInt8 pData[NVM_SECTOR_HEADER_SIZE];

    GP_ASSERT_DEV_EXT(phySecId < Nvm_Flash_NbrOfPhySectors);

    Nvm_Flash_Read(NVM_SECTOR_ADDROFFSET(phySecId), NVM_SECTOR_HEADER_SIZE, pData);

    return Nvm_PhySector_IsValidHeader(pData);
}

static Bool Nvm_PhySector_IsFree(Nvm_PhySectorId_t phySecId)
{
    UInt8 pData[NVM_SECTOR_HEADER_SIZE];

    GP_ASSERT_DEV_EXT(phySecId < Nvm_Flash_NbrOfPhySectors);

    Nvm_Flash_Read(NVM_SECTOR_ADDROFFSET(phySecId), NVM_SECTOR_HEADER_SIZE, pData);

    //treat broken page as used pages
    if (Nvm_PhySector_IsBroken((void*)pData))
    {
        return false;
    }

    return (!Nvm_PhySector_IsValidHeader(pData));
}

static Bool Nvm_IsRecordHeaderCorrupted(const UInt8 tokenLength, const UInt8 payloadLength, const UInt16 bodyLength, UInt32 pageAddrOffset)
{
    if (pageAddrOffset + NVM_RECORD_HEADER_SIZE + bodyLength > FLASH_PAGE_SIZE)
    {
        return true;
    }

    if (tokenLength > GP_NVM_MAX_TOKENLENGTH)
    {
        return true;
    }
#if (GP_NVM_MAX_PAYLOADLENGTH < 255)
    if (payloadLength > GP_NVM_MAX_PAYLOADLENGTH)
    {
        return true;
    }
#endif

    return false;
}

static Bool Nvm_ReadRecordAndCheckCorrupted(UInt32 ptrTag, UInt32 pageAddrOffset, UInt8* pData, Bool* pIsLastRecord, UInt16* pRecordSize)
{
    GP_ASSERT_DEV_INT(pData != NULL);
    GP_ASSERT_DEV_INT(pIsLastRecord != NULL);
    GP_ASSERT_DEV_INT(pRecordSize != NULL);

    *pIsLastRecord = false;

    // Read record header.
    if (Nvm_Flash_Read(ptrTag, NVM_RECORD_HEADER_SIZE, pData) != gpHal_FlashError_Success)
    {
        return true;
    }

    const UInt8 payloadLength = pData[NVM_RECORD_HEADER_OFFSET_TO_PAYLOADLENGTH];
    const UInt8 tokenLength = pData[NVM_RECORD_HEADER_OFFSET_TO_TOKENLENGTH];
    const UInt16 bodyLength = tokenLength + payloadLength;
    UInt16 recordSize = NVM_RECORD_SIZE(tokenLength, payloadLength);

    GP_ASSERT_DEV_INT(pIsLastRecord != 0);

    if (tokenLength == 0)
    {
        *pIsLastRecord = true;
        return false;
    }

    if(Nvm_IsRecordHeaderCorrupted(tokenLength, payloadLength, bodyLength, pageAddrOffset))
    {
        return true;
    }

    // Read record body.
    if (Nvm_Flash_Read(ptrTag + NVM_RECORD_HEADER_SIZE, bodyLength, pData + NVM_RECORD_HEADER_SIZE) != gpHal_FlashError_Success)
    {
        return true;
    }

    // Check CRC.
    const UInt16 calculatedCrc = gpUtils_CalculateCrc(pData + NVM_RECORD_HEADER_CRC_BYTES, recordSize - NVM_RECORD_HEADER_CRC_BYTES);
    UInt16 givenCrc = Nvm_Buf_Get_UInt16(pData, NVM_RECORD_HEADER_OFFSET_TO_CRC);
    if (calculatedCrc != givenCrc)
    {
        GP_LOG_PRINTF("Wrong CRC stored %04x vs calc %04x",0, givenCrc, calculatedCrc);
        return true;
    }

    UInt16 alignedRecordSize = NVM_ALIGNED_RECORD_SIZE(tokenLength, payloadLength);
    /* initialize trailer padding */
    MEMSET(&pData[recordSize], 0x00, alignedRecordSize - recordSize);

    *pRecordSize = alignedRecordSize;
    return false;
}

/* Return true if the specified physical sector is corrupt.
 * A sector is corrupt if it is marked as valid but contains inconsistent data.
 */
static Bool Nvm_PhySector_IsCorrupt(Nvm_PhySectorId_t phySecId)
{
    UInt8 pHeader_0[NVM_SECTOR_HEADER_SIZE];
    UInt8 pHeader_1[NVM_SECTOR_HEADER_SIZE];
    UIntLoop pageId;

    GP_ASSERT_DEV_EXT(phySecId < Nvm_Flash_NbrOfPhySectors);

    Nvm_Flash_Read(NVM_PAGE_ADDROFFSET(phySecId,0), NVM_SECTOR_HEADER_SIZE, pHeader_0);

    if (!Nvm_PhySector_IsValidHeader(pHeader_0))
    {
        return false;
    }
    if (Nvm_PhySector_IsBroken(pHeader_0))
    {
        return false;
    }

    // All pages in the sector must have identical header.
    for (pageId = 1; pageId < NVM_NUMBER_PAGES_PER_SECTOR; pageId++)
    {
        Nvm_Flash_Read(NVM_PAGE_ADDROFFSET(phySecId,pageId), NVM_SECTOR_HEADER_SIZE, pHeader_1);
        if (MEMCMP(pHeader_0, pHeader_1, NVM_SECTOR_HEADER_SIZE) != 0)
        {
            return true;
        }
    }

    // Check all pages.
    for (pageId = 0; pageId < NVM_NUMBER_PAGES_PER_SECTOR; pageId++)
    {
        // Check all records in the page.
        UInt32 pageAddrOffset = NVM_SECTOR_HEADER_SIZE;
        UInt16 alignedRecordSize;
        while (pageAddrOffset <= NVM_MAX_RECORD_OFFSET)
        {
            /* pData must be a multiple of FLASH_WRITE_UNIT, because "Nvm_ReadRecordAndCheckCorrupted" will also
             * add the padding up to a multiple of FLASH_WRITE_UNIT in this array
             */
            UInt32 pData_32[NVM_DIV_ROUND_UP(NVM_MAX_ALIGNED_RECORD_SIZE, sizeof(UInt32))];
            UInt8* pData = (UInt8*)pData_32;
            UInt32 ptrTag = NVM_PAGE_ADDROFFSET(phySecId,pageId) + pageAddrOffset;

            Bool isCorruptedRecord;
            Bool isLastRecord;
            isCorruptedRecord = Nvm_ReadRecordAndCheckCorrupted(ptrTag, pageAddrOffset, pData, &isLastRecord, &alignedRecordSize);

            if(isCorruptedRecord)
            {
                return true;
            }
            if(isLastRecord)
            {
                break; // past last tag in this page
            }
            pageAddrOffset += alignedRecordSize;
        }
    }

    return false;
}

static Nvm_PhySectorId_t Nvm_Pool_GetFreePhySectorId(gpNvm_PoolId_t poolId)
{
    Nvm_SectorPool_Loop_t i;
    Nvm_PhySectorId_t offset = Nvm_Pool_GetPhySectorOffset(poolId);
    Nvm_PhySectorId_t nbrOfPhysicalSectors = Nvm_Pool_PhySectors_GetNbr(poolId);

    GP_ASSERT_DEV_INT(Nvm_Pool_LastAllocatedPhySector[poolId] >= offset);

    for (i=0; i < nbrOfPhysicalSectors; i++)
    {
        Nvm_PhySectorId_t phySecId;
        Nvm_PhySectorId_t lastAllocatedIndex = 0;

        lastAllocatedIndex = Nvm_Pool_LastAllocatedPhySector[poolId] - offset;
        phySecId = offset + ((lastAllocatedIndex + 1 + i) % (nbrOfPhysicalSectors));

        if (Nvm_PhySector_IsFree(phySecId))
        {
            Nvm_Pool_LastAllocatedPhySector[poolId] = phySecId;
            return phySecId;
        }
    }

    GP_LOG_SYSTEM_PRINTF("Could not find an empty sector to copy non-redundant sector",0);
    GP_ASSERT_SYSTEM(false);

    return NVM_SECTORID_INVALID;
}

static UInt8 Nvm_Pool_PhySectors_GetNbrUsed(gpNvm_PoolId_t poolId)
{
    UInt8 counter = 0;
    Nvm_SectorPool_Loop_t i;
    Nvm_PhySectorId_t offset = Nvm_Pool_GetPhySectorOffset(poolId);
    Nvm_PhySectorId_t nbrOfPhysicalSectors = Nvm_Pool_PhySectors_GetNbr(poolId);

    for (i = 0; i < nbrOfPhysicalSectors; i++)
    {
        Nvm_PhySectorId_t phySecId = offset + i;
        if (!Nvm_PhySector_IsFree(phySecId))
        {
           counter++;
        }
    }
    return counter;
}

void Nvm_LogSector_Erase(Nvm_PhySectorId_t phySectorNbr[GP_NVM_NBR_OF_REDUNDANT_SECTORS])
{
    UIntLoop mirrorId;

    for (mirrorId = 0; mirrorId < GP_NVM_NBR_OF_REDUNDANT_SECTORS; mirrorId++)
    {
        if(Nvm_PhySector_InUse(phySectorNbr[mirrorId]))
        {
            Nvm_Flash_EraseSector(phySectorNbr[mirrorId]);
        }
    }
}

static Bool Nvm_Pool_Iterator_Start(gpNvm_PoolId_t poolId, Nvm_Pool_Iterator_t* iter)
{
    GP_ASSERT_DEV_INT(iter);
    iter->poolId = poolId;
    iter->logSec = 0;
    iter->pageId = 0;
    iter->offsetInSector = NVM_SECTOR_HEADER_SIZE;
    iter->isActive = true;

    if(!Nvm_LogSector_InUse(iter->poolId, iter->logSec))
    {
        if(!Nvm_Pool_Iterator_NextSector(iter))
        {
            iter->isActive = false;
            return false;
        }
    }

    return Nvm_Pool_Iterator_Read(iter);
}
static Bool Nvm_Pool_Iterator_IsActive(Nvm_Pool_Iterator_t* iter)
{
    return iter->isActive;
}

static Bool Nvm_Pool_Iterator_FromSector(Nvm_Pool_Iterator_t* iter, gpNvm_PoolId_t poolId, Nvm_LogSectorId_t logSec)
{
    iter->poolId = poolId;
    iter->pageId = 0;
    iter->offsetInSector = NVM_SECTOR_HEADER_SIZE;
    iter->logSec = logSec;
    iter->isActive = true;

    return Nvm_Pool_Iterator_Read(iter);
}

static Bool Nvm_Pool_Iterator_NextSector(Nvm_Pool_Iterator_t* iter)
{
    iter->pageId = 0;
    iter->offsetInSector = NVM_SECTOR_HEADER_SIZE;

    do
    {
        iter->logSec++;
    } while((iter->logSec < Nvm_Pool_LogSectors_GetNbr(iter->poolId)) && !Nvm_LogSector_InUse(iter->poolId, iter->logSec));

    if(iter->logSec == Nvm_Pool_LogSectors_GetNbr(iter->poolId))
    {
        iter->isActive = false;
        return false;
    }

    /* Reset watchdog because this loop could potentially take a long time */
    HAL_WDT_RESET();

    return true;
}

static Bool Nvm_Pool_Iterator_NextPage(Nvm_Pool_Iterator_t* iter)
{
    GP_ASSERT_DEV_INT(iter);
    iter->pageId++;
    iter->offsetInSector = (iter->pageId * FLASH_PAGE_SIZE) + NVM_SECTOR_HEADER_SIZE;

    if(iter->pageId == NVM_NUMBER_PAGES_PER_SECTOR)
    {
        return Nvm_Pool_Iterator_NextSector(iter);
    }
    return true;
}

static Bool Nvm_Pool_Iterator_Read(Nvm_Pool_Iterator_t* iter)
{
    do
    {
        UInt32 pageAddrOffset = iter->offsetInSector % FLASH_PAGE_SIZE;
        Nvm_Flash_Read(
            Nvm_LogSector_GetPhyFlashOffset(iter->poolId, iter->logSec) + iter->offsetInSector,
            NVM_RECORD_HEADER_SIZE,
            iter->RecordData);
        const UInt8 payloadLength = iter->RecordData[NVM_RECORD_HEADER_OFFSET_TO_PAYLOADLENGTH];
        const UInt8 poolIterTokenLength = iter->RecordData[NVM_RECORD_HEADER_OFFSET_TO_TOKENLENGTH];
        const UInt8 bodyLength = poolIterTokenLength + payloadLength;
        UInt8* poolIterToken = &iter->RecordData[NVM_RECORD_HEADER_OFFSET_TO_TOKEN];
        if(poolIterTokenLength == 0)
        {
            if(Nvm_Pool_Iterator_NextPage(iter))
            {
                continue;
            }
            iter->isActive = false;
            return false;
        }
        GP_ASSERT_DEV_INT(poolIterTokenLength <= GP_NVM_MAX_TOKENLENGTH);
        if(Nvm_IsRecordHeaderCorrupted(poolIterTokenLength, payloadLength, bodyLength, pageAddrOffset))
        {
            GP_LOG_SYSTEM_PRINTF("ERROR: Token header corrupted! tl=%u pl=%u bl=%u po=%lu", 0, poolIterTokenLength, payloadLength, bodyLength, (unsigned long int)pageAddrOffset);
            iter->isActive = false;
            return false;
        }
        Nvm_Flash_Read(
            Nvm_LogSector_GetPhyFlashOffset(iter->poolId, iter->logSec) + iter->offsetInSector + NVM_RECORD_HEADER_OFFSET_TO_TOKEN,
            poolIterTokenLength,
            poolIterToken);

        return true;
    } while(true);
}
Bool Nvm_Pool_Iterator_Next(Nvm_Pool_Iterator_t* iter)
{
    const UInt8 payloadLength = iter->RecordData[NVM_RECORD_HEADER_OFFSET_TO_PAYLOADLENGTH];
    const UInt8 tokenLength = iter->RecordData[NVM_RECORD_HEADER_OFFSET_TO_TOKENLENGTH];

    iter->offsetInSector += NVM_ALIGNED_RECORD_SIZE(tokenLength, payloadLength);
    if(iter->offsetInSector >= FLASH_SECTOR_SIZE - NVM_RECORD_HEADER_SIZE)
    {
        if(!Nvm_Pool_Iterator_NextPage(iter))
        {
            iter->isActive = false;
            return false;
        }
    }

    return Nvm_Pool_Iterator_Read(iter);
}

//------------------------ Housekeeping ------------------------

static Bool Nvm_MaskedSeqNum_InLowerHalf(
    Nvm_MaskedSeqNum_t maskedSeqNum
)
{
    return (maskedSeqNum <= NVM_MASKED_SEQNUM_MAX/2);
}
static Bool Nvm_MaskedSeqNum_InLowerQuarter(
    Nvm_MaskedSeqNum_t maskedSeqNum
)
{
    return (maskedSeqNum <= NVM_MASKED_SEQNUM_MAX/4);
}

static Bool Nvm_MaskedSeqNum_InUpperHalf(
    Nvm_MaskedSeqNum_t maskedSeqNum
)
{
    return (maskedSeqNum > NVM_MASKED_SEQNUM_MAX/2);
}
static Bool Nvm_MaskedSeqNum_InUpperQuarter(
    Nvm_MaskedSeqNum_t maskedSeqNum
)
{
    return (maskedSeqNum > 3*NVM_MASKED_SEQNUM_MAX/4);
}

static void Nvm_Pool_SeqNum_RegisterSeqNumceCtr(
    gpNvm_PoolId_t poolId,
    Nvm_MaskedSeqNum_t maskedSeqNum,
    Nvm_LogSectorId_t logSecId
)
{
    if (maskedSeqNum > Nvm_MaskedSeqNum_Highest[poolId]
    || !NVM_IS_SEQNUM_VALID(Nvm_MaskedSeqNum_Highest[poolId]))
    {
        Nvm_MaskedSeqNum_Highest[poolId] = maskedSeqNum;
    }

    if (maskedSeqNum < Nvm_MaskedSeqNum_Lowest[poolId].maskedSeqNum
    || !NVM_IS_SEQNUM_VALID(Nvm_MaskedSeqNum_Lowest[poolId].maskedSeqNum))
    {
        Nvm_MaskedSeqNum_Lowest[poolId].maskedSeqNum = maskedSeqNum;
        Nvm_MaskedSeqNum_Lowest[poolId].logSecId = logSecId;
    }

    if (Nvm_MaskedSeqNum_InLowerHalf(maskedSeqNum))
    {
        if (maskedSeqNum > Nvm_MaskedSeqNum_HighestInLowerHalf[poolId]
        || !NVM_IS_SEQNUM_VALID(Nvm_MaskedSeqNum_HighestInLowerHalf[poolId]))
        {
            Nvm_MaskedSeqNum_HighestInLowerHalf[poolId] = maskedSeqNum;
        }
    }
    if (Nvm_MaskedSeqNum_InUpperHalf(maskedSeqNum))
    {
        if (maskedSeqNum < Nvm_MaskedSeqNum_LowestInUpperHalf[poolId].maskedSeqNum
        || !NVM_IS_SEQNUM_VALID(Nvm_MaskedSeqNum_LowestInUpperHalf[poolId].maskedSeqNum))
        {
            Nvm_MaskedSeqNum_LowestInUpperHalf[poolId].maskedSeqNum= maskedSeqNum;
            Nvm_MaskedSeqNum_LowestInUpperHalf[poolId].logSecId = logSecId;
        }
    }
}

static Bool Nvm_Pool_SeqNum_UnregisterSeqNum(
    gpNvm_PoolId_t poolId,
    Nvm_MaskedSeqNum_t maskedSeqNum
)
{
    Bool result = false;
    if (maskedSeqNum == Nvm_MaskedSeqNum_Highest[poolId])
    {
        Nvm_MaskedSeqNum_Highest[poolId] = NVM_SEQNUM_INVALID;
        result = true;
    }
    if (maskedSeqNum == Nvm_MaskedSeqNum_LowestInUpperHalf[poolId].maskedSeqNum)
    {
        Nvm_MaskedSeqNum_LowestInUpperHalf[poolId].maskedSeqNum = NVM_SEQNUM_INVALID;
        result = true;
    }
    if (maskedSeqNum == Nvm_MaskedSeqNum_HighestInLowerHalf[poolId])
    {
        Nvm_MaskedSeqNum_HighestInLowerHalf[poolId] = NVM_SEQNUM_INVALID;
        result = true;
    }
    if (maskedSeqNum == Nvm_MaskedSeqNum_Lowest[poolId].maskedSeqNum)
    {
        Nvm_MaskedSeqNum_Lowest[poolId].maskedSeqNum = NVM_SEQNUM_INVALID;
        result = true;
    }
    return result;
}

/* return true if the sequence numbers need to be recalculated after the erase operation */
 static Bool Nvm_Pool_UnregisterLogSector(
    gpNvm_PoolId_t poolId,
    Nvm_LogSectorId_t logSecId
)
{
    Nvm_Pool_Iterator_t poolIter;
    Bool resetRequired = false;
    Bool poolIteratorInit;
    GP_ASSERT_DEV_EXT(poolId < Nvm_Flash_NbrOfPools);
    GP_ASSERT_DEV_EXT(logSecId < Nvm_Pool_LogSectors_GetNbr(poolId));

    for(poolIteratorInit = Nvm_Pool_Iterator_FromSector(&poolIter, poolId, logSecId);
        poolIteratorInit && Nvm_Pool_Iterator_IsActive(&poolIter) && poolIter.logSec == logSecId;
        Nvm_Pool_Iterator_Next(&poolIter))
    {
        const Nvm_MaskedSeqNum_t seqNum = Nvm_Buf_Get_UInt16(poolIter.RecordData, NVM_RECORD_HEADER_OFFSET_TO_SEQNUM_COUNTER);

        resetRequired |= Nvm_Pool_SeqNum_UnregisterSeqNum(poolId, seqNum);
    }

    if(!poolIteratorInit)
    {
        return false;
    }

    return resetRequired;
}

static Bool Nvm_Pool_SeqNum_IsSeqNumIntervalWrapping(gpNvm_PoolId_t poolId)
{
    if (!NVM_IS_SEQNUM_VALID(Nvm_MaskedSeqNum_LowestInUpperHalf[poolId].maskedSeqNum))
    {
        /* nothing in upper half */
        return false;
    }
    if (!NVM_IS_SEQNUM_VALID(Nvm_MaskedSeqNum_HighestInLowerHalf[poolId]))
    {
        /* no wrapping when only upper half has elements */
        return false;
    }
    return (
        Nvm_MaskedSeqNum_InUpperQuarter(Nvm_MaskedSeqNum_LowestInUpperHalf[poolId].maskedSeqNum)
    &&  Nvm_MaskedSeqNum_InLowerQuarter(Nvm_MaskedSeqNum_HighestInLowerHalf[poolId])
    );
}

static Nvm_SeqNum_t Nvm_Pool_SeqNum_Unmask(
    gpNvm_PoolId_t poolId,
    Nvm_MaskedSeqNum_t maskedSeqNum
)
{
    if (Nvm_Pool_SeqNum_IsSeqNumIntervalWrapping(poolId)
     && Nvm_MaskedSeqNum_InLowerHalf(maskedSeqNum)
    )
    {
        return NVM_SEQNUM_MSB | maskedSeqNum;
    }
    else
    {
        return maskedSeqNum;
    }
}

static Nvm_MaskedSeqNum_t Nvm_Pool_SeqNum_GetNext(
    gpNvm_PoolId_t poolId
)
{
    return NVM_MASKED_SEQNUM_MASK & (Nvm_Pool_SeqNum_GetHighest(poolId) + 1);
}

static Nvm_LogSectorId_t Nvm_Pool_GetOldestSector(
    gpNvm_PoolId_t poolId
)
{
    if (Nvm_Pool_SeqNum_IsSeqNumIntervalWrapping(poolId))
    {
        return Nvm_MaskedSeqNum_LowestInUpperHalf[poolId].logSecId;
    }
    else
    {
        return Nvm_MaskedSeqNum_Lowest[poolId].logSecId;
    }
}

static Nvm_SeqNum_t Nvm_Pool_SeqNum_GetLowestSeqNum(
    gpNvm_PoolId_t poolId
)
{
    if (Nvm_Pool_SeqNum_IsSeqNumIntervalWrapping(poolId))
    {
        return Nvm_MaskedSeqNum_LowestInUpperHalf[poolId].maskedSeqNum;
    }
    else
    {
        return Nvm_MaskedSeqNum_Lowest[poolId].maskedSeqNum;
    }
}

static Nvm_SeqNum_t Nvm_Pool_SeqNum_GetHighest(
    gpNvm_PoolId_t poolId
)
{
    if (Nvm_Pool_SeqNum_IsSeqNumIntervalWrapping(poolId))
    {
        return Nvm_Pool_SeqNum_Unmask(poolId, Nvm_MaskedSeqNum_HighestInLowerHalf[poolId]);
    }
    else
    {
        return Nvm_MaskedSeqNum_Highest[poolId];
    }
}

static void Nvm_Pool_SeqNum_EvaluateSpread(
    gpNvm_PoolId_t poolId
)
{
    Nvm_SeqNum_t minIc = Nvm_Pool_SeqNum_GetLowestSeqNum(poolId);
    Nvm_SeqNum_t maxIc = Nvm_Pool_SeqNum_GetHighest(poolId);

    if (!NVM_IS_SEQNUM_VALID(maxIc))
    {
        return;
    }

    if ((maxIc - minIc) > NVM_MAX_SEQNUM_SPREAD)
    {
        Nvm_SectorPool_ScheduleRenumber(0, poolId);
    }
}

static void Nvm_SectorPool_ScheduleRenumber(UInt32 rel_time, gpNvm_PoolId_t poolId)
{
    if (!gpSched_ExistsEventArg(Nvm_Pool_Relocate_wrapper,(void*)&Nvm_PoolId_ScheduleTrick[poolId]))
    {
        gpSched_ScheduleEventArg(rel_time, Nvm_Pool_Relocate_wrapper,(void*)&Nvm_PoolId_ScheduleTrick[poolId]);
    }
}

static void Nvm_Pool_Relocate_wrapper(void* x)
{
    UInt8* y = (UInt8*)x;
    GP_ASSERT_DEV_INT(x);
    UInt8 poolId = y - &Nvm_PoolId_ScheduleTrick[0];

    Nvm_LogSectorId_t logSecId = Nvm_Pool_GetOldestSector(poolId);

    (void)Nvm_LogSector_RelocateActiveData(poolId, logSecId);

    Nvm_Pool_SeqNum_EvaluateSpread(poolId);
}

static void Nvm_SectorPool_ScheduleDefrag(UInt32 rel_time, gpNvm_PoolId_t poolId)
{
    if (!gpSched_ExistsEventArg(Nvm_Pool_ReclaimOutdatedSpace_wrapper,(void*)&Nvm_PoolId_ScheduleTrick[poolId]))
    {
        gpSched_ScheduleEventArg(rel_time, Nvm_Pool_ReclaimOutdatedSpace_wrapper,(void*)&Nvm_PoolId_ScheduleTrick[poolId]);
    }
}
static void Nvm_Pool_ReclaimOutdatedSpace_wrapper(void* x)
{
    UInt8* y = (UInt8*)x;
    GP_ASSERT_DEV_INT(x);
    UInt8 poolId = y - &Nvm_PoolId_ScheduleTrick[0];

    UInt8 NbrOfDefragCandidates = Nvm_Pool_ReclaimOutdatedSpace(poolId);
    if (NbrOfDefragCandidates > 0)
    {
        /* Reschedule defragmentation from within scheduled function to prevent CPU monopolization */
        gpSched_ScheduleEventArg(GP_NVM_BACKGROUND_DEFRAG_SECTOR_INTERVAL_US, Nvm_Pool_ReclaimOutdatedSpace_wrapper,(void*)&Nvm_PoolId_ScheduleTrick[poolId]);
    }
}

static Bool Nvm_LogSector_IsDefragCandidate(
        gpNvm_PoolId_t poolId,
        Nvm_LogSectorId_t logSecId
)
{
    Nvm_SectorLut_t *sectorLut = Nvm_Pool_SectorLut_Get(poolId);

    if (logSecId == NVM_SECTORID_INVALID)
    {
        return false;
    }

    const UInt16 InactiveSum = sectorLut[logSecId].InactiveSum;
    if (InactiveSum > FLASH_SECTOR_SIZE/4)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/* Mark the sector as read-only in sectorLut (RAM)
 * This is implicitly cleared by erasing and marking the logical sector as unavailable */
static void Nvm_LogSector_WriteLock(gpNvm_PoolId_t poolId, Nvm_LogSectorId_t logSecId)
{
    Nvm_SectorLut_t* sectorLut = Nvm_Pool_SectorLut_Get(poolId);

    sectorLut[logSecId].pageWriteOffset[0] = NVM_WRITE_OFFSET_WRITELOCK_MW;
    sectorLut[logSecId].pageWriteOffset[1] = NVM_WRITE_OFFSET_WRITELOCK_MW;
}

/* Move up-to-date records outside a sector to reclaim the space occupied by
 * records with out-of-date values in the sector (by erase and reuse). */
static gpNvm_Result_t Nvm_LogSector_RelocateActiveData(
    gpNvm_PoolId_t poolId,
    Nvm_LogSectorId_t logSecId
)
{
    gpNvm_Result_t result;
    gpNvm_LookupTable_Handle_t lutHandle;
    Bool freeAfterUse;
    Bool resetRequired = false;
    Nvm_TokenLut_t *lut;

    GP_LOG_PRINTF("RelocateActiveData pool:%d logsec:%d", 0, poolId, logSecId);

    GP_ASSERT_DEV_INT(logSecId != NVM_SECTORID_INVALID);
    result = Nvm_AcquireLutHandle(&lutHandle, poolId, 0, NULL, &freeAfterUse, Nvm_NumberOfUniqueTokens, true, true);

    if (result != gpNvm_Result_DataAvailable)
    {
        GP_ASSERT_DEV_INT(false);
        if(freeAfterUse)
        {
            gpNvm_FreeLookup(lutHandle);
        }
        return result;
    }

    lut = NVM_LUTHANDLE_TO_LUT(lutHandle);

    Bool IteratorInit = Nvm_TokenLutIter_Start(lut, &lut->readNext_Iterator);
    GP_ASSERT_DEV_EXT(IteratorInit);

    /* avoid relocation to the same sector */
    Nvm_LogSector_WriteLock(poolId, logSecId);

    for(  /* BlockIterator_Start with assert */;
          Nvm_TokenLutIter_IsAtEnd(&lut->readNext_Iterator) != true;
          (void) Nvm_TokenLutIter_Next(&lut->readNext_Iterator)
    )
    {
        UInt8 Payload[GP_NVM_MAX_PAYLOADLENGTH];
        UInt8 Token[GP_NVM_MAX_TOKENLENGTH];
        Nvm_Token_t *lutItem;

        lutItem = Nvm_TokenLutIter_GetItem(&lut->readNext_Iterator);
        if (lutItem->logSecId != logSecId)
        {
            continue;
        }
        Nvm_Token_PreviousVersion_t prevVersion;
        UInt8 tokenLength;

        prevVersion.lookUpResult = gpNvm_Result_DataAvailable;
        prevVersion.logSecId = lutItem->logSecId;

        result = \
        Nvm_Token_GetRecord(
           poolId,
           lutItem,
           GP_NVM_MAX_TOKENLENGTH,
           Token,
           &tokenLength,
           GP_NVM_MAX_PAYLOADLENGTH,
           &prevVersion.payloadLength,
           Payload
        );

        /* When record deleted, Nvm_Token_GetRecord returns gpNvm_Result_NoDataAvailable */
        GP_ASSERT_DEV_EXT(result == gpNvm_Result_DataAvailable || result == gpNvm_Result_NoDataAvailable);

        Bool isDeleted = (prevVersion.payloadLength == NVM_PAYLOADLENGTH_INDICATING_TOKENREMOVED);

        if(isDeleted && lutItem->nbrOfSectorReferences == 1)
        {
            /* It makes no sense carrying over the active delete record,
             * no other sectors contain this token, we can drop it. */
            GP_LOG_PRINTF("reclaiming LUT entry!", 0);
            result = gpNvm_Result_DataAvailable;
            /* Trigger rebuild of LUTs,
             * Note that after during Nvm_Reset(), we will have a window of opportunity
             * where the LUT's have a broken reference to the sector we are now procesing.
             * (Nvm_LogSector_Erase() until Nvm_TokenLuts_Rebuild()).
             */
            resetRequired |= true;
        }
        else
        {
            GP_LOG_PRINTF("relocating token starting with %x", 0, Token[0]);
            result = Nvm_Pool_Record_Add(
                poolId,
                tokenLength,
                Token,
                prevVersion.payloadLength,
                Payload,
                prevVersion);
            if(result != gpNvm_Result_DataAvailable)
            {
                /* abort on addRecord issues */
                break;
            }
        }
    }

    if (result == gpNvm_Result_DataAvailable)
    {
        resetRequired |= Nvm_Pool_UnregisterLogSector(poolId, logSecId);

        Nvm_SectorLut_t *sectorLut = Nvm_Pool_SectorLut_Get(poolId);
        Nvm_LogSector_Erase(sectorLut[logSecId].phySectorNbr);
        Nvm_LogSector_Init(&sectorLut[logSecId]);
    }

    if (freeAfterUse)
    {
        gpNvm_FreeLookup(lutHandle);
    }
    if (resetRequired)
    {
        Nvm_Reset();
    }

    return result;
}

static UInt8 Nvm_Pool_ReclaimOutdatedSpace(gpNvm_PoolId_t poolId)
{
    /* Defragment one logical sector at a time */
    Nvm_PhySectorId_t logSecIter = 0;
    Nvm_SectorLut_t *sectorLut = Nvm_Pool_SectorLut_Get(poolId);

    UInt8 NbrOfDefragCandidates = 0;
    UInt8 logSecId = NVM_SECTORID_INVALID;
    UInt16 maxReclaimable = 0;
    for (logSecIter = 0; logSecIter < Nvm_Pool_LogSectors_GetNbr(poolId); logSecIter++)
    {
        if (!Nvm_LogSector_InUse(poolId, logSecIter))
        {
            continue;
        }
        if (Nvm_Pool_IsUrgentDefragRequired(poolId))
        {
            if (sectorLut[logSecIter].InactiveSum > maxReclaimable)
            {
                maxReclaimable = sectorLut[logSecIter].InactiveSum;
                if(logSecId == NVM_SECTORID_INVALID)
                {
                    logSecId = logSecIter;
                }
            }
            if (sectorLut[logSecIter].InactiveSum != 0)
            {
                NbrOfDefragCandidates++;
            }
        }
        else
        {
            if (Nvm_LogSector_IsDefragCandidate(poolId, logSecIter))
            {
                NbrOfDefragCandidates++;
                if(logSecId == NVM_SECTORID_INVALID)
                {
                    logSecId = logSecIter;
                }
            }
        }
    }

    if (NbrOfDefragCandidates > 0)
    {
        GP_ASSERT_SYSTEM(logSecId != NVM_SECTORID_INVALID);
        gpNvm_Result_t result;
        result = Nvm_LogSector_RelocateActiveData(poolId, logSecId);

        if (result == gpNvm_Result_DataAvailable)
        {
            NbrOfDefragCandidates--;
        }
        else
        {
            NbrOfDefragCandidates = 0; /* don't reschedule to keep running into errors */
        }
    }

    return NbrOfDefragCandidates;
}

/* Remove delete-records when they are not needed anymore but still take up a LUT entry.
 * This is the case when the token is not found in any other record of the pool.
 * Since every token in the pool is tracked, these obsolete records will otherwise
 * take up an entry in the LUT and prevent new tokens being added. */
static UInt8 Nvm_Pool_ReclaimNamespaceSpace(gpNvm_PoolId_t poolId)
{
    UInt8 NbrOfDefragCandidates = 0;
    UInt8 logSecIdSelected = NVM_SECTORID_INVALID;
    UInt8 logSecSkip = NVM_SECTORID_INVALID;
    gpNvm_Result_t Nvm_Result;
    gpNvm_LookupTable_Handle_t lutHandle;
    Bool freeAfterUse;
    Nvm_TokenLut_Iterator_t lutIter;
    Bool lutIteratorInit;

    Nvm_Result = Nvm_AcquireLutHandle(&lutHandle, poolId, 0, NULL, &freeAfterUse, Nvm_NumberOfUniqueTokens, true, true);

    if(Nvm_Result == gpNvm_Result_NoDataAvailable)
    {
        /* when the flash is empty, the LUT is empty */
        goto Nvm_Pool_ReclaimNamespaceSpace_cleanup;
    }

    if(Nvm_Result != gpNvm_Result_DataAvailable)
    {
        /* FIXME: must be sure the error is caused because of already over it? */
        goto Nvm_Pool_ReclaimNamespaceSpace_cleanup;
    }

    for(lutIteratorInit = Nvm_TokenLutIter_Start(NVM_LUTHANDLE_TO_LUT(lutHandle), &lutIter);
        lutIteratorInit && Nvm_TokenLutIter_IsAtEnd(&lutIter) != true;
        (void)Nvm_TokenLutIter_Next(&lutIter))
    {
        Nvm_Token_t* lutItem = Nvm_TokenLutIter_GetItem(&lutIter);
        UInt8 lutItemHeader[NVM_RECORD_HEADER_SIZE];
        UInt8 lutIterToken[GP_NVM_MAX_TOKENLENGTH];
        Nvm_Pool_Iterator_t poolIter;
        gpNvm_Result_t result;
        Bool poolIteratorInit;

        result = Nvm_Token_GetRecordHeader(poolId, lutItem, lutItemHeader);

        if(result != gpNvm_Result_DataAvailable)
        {
            GP_ASSERT_DEV_INT(false);
            goto Nvm_Pool_ReclaimNamespaceSpace_cleanup;
        }

        const UInt8 payloadLength = lutItemHeader[NVM_RECORD_HEADER_OFFSET_TO_PAYLOADLENGTH];
        const UInt8 lutIterTokenLength = lutItemHeader[NVM_RECORD_HEADER_OFFSET_TO_TOKENLENGTH];

        if(payloadLength != NVM_PAYLOADLENGTH_INDICATING_TOKENREMOVED)
        {
            /* skip active tokens in the LUT */
            continue;
        }

        Nvm_Token_GetRecordToken(poolId,
                                 lutItem,
                                 lutIterTokenLength,
                                 lutIterToken);

        GP_LOG_PRINTF("Found removed token starting with %x", 0, lutIterToken[0]);

        for(poolIteratorInit = Nvm_Pool_Iterator_Start(poolId, &poolIter);
            poolIteratorInit && Nvm_Pool_Iterator_IsActive(&poolIter);
            Nvm_Pool_Iterator_Next(&poolIter))
        {
            UInt8* poolIterToken = &poolIter.RecordData[NVM_RECORD_HEADER_OFFSET_TO_TOKEN];
            const UInt8 poolIterTokenLength = poolIter.RecordData[NVM_RECORD_HEADER_OFFSET_TO_TOKENLENGTH];
            if(poolIter.logSec == logSecSkip)
            {
                continue;
            }

            if(EXACT_TOKEN_MATCH(lutIterToken, lutIterTokenLength, poolIterToken, poolIterTokenLength))
            {
                // found an outdated reference to a removed token in this sector
                GP_LOG_PRINTF("Found outdated reference in sector %d", 0, poolIter.logSec);
                logSecSkip = poolIter.logSec; /* For this token, ignore the rest of the sector */

                if(logSecIdSelected == NVM_SECTORID_INVALID)
                {
                    /* first item found */
                    logSecIdSelected = poolIter.logSec;
                }
                if(logSecIdSelected == lutItem->logSecId)
                {
                    /* It is important to relocate sectors other than the one
                     containing the active record first */
                    logSecIdSelected = poolIter.logSec;
                }
                NbrOfDefragCandidates++;
            }
        }
        if(!poolIteratorInit)
        {
            /* We should not get into namespace cleanup with an empty flash */
            GP_ASSERT_DEV_INT(false);
            goto Nvm_Pool_ReclaimNamespaceSpace_cleanup;
        }
    }

    GP_ASSERT_DEV_EXT(lutIteratorInit);
    if(!lutIteratorInit)
    {
        goto Nvm_Pool_ReclaimNamespaceSpace_cleanup;
    }

    GP_LOG_PRINTF("%d namespace defrag candidates", 0, NbrOfDefragCandidates);
    if(NbrOfDefragCandidates > 0)
    {
        GP_ASSERT_SYSTEM(logSecIdSelected != NVM_SECTORID_INVALID);
        gpNvm_Result_t result;
        result = Nvm_LogSector_RelocateActiveData(poolId, logSecIdSelected);

        if(result == gpNvm_Result_DataAvailable)
        {
            NbrOfDefragCandidates--;
        }
        else
        {
            NbrOfDefragCandidates = 0; /* don't reschedule to keep running into errors */
        }
    }

Nvm_Pool_ReclaimNamespaceSpace_cleanup:

    if(freeAfterUse)
    {
        gpNvm_FreeLookup(lutHandle);
    }
    return NbrOfDefragCandidates;
}
static void Nvm_Pools_EraseCorrupted(void)
{
    Nvm_SectorLoop_t phySecId;

    /* loop over pools */
    for (uint8_t poolId = 0; poolId < Nvm_Flash_NbrOfPools; ++poolId)
    {
        for (phySecId = 0; phySecId < Nvm_Pool_PhySectors_GetNbr(poolId); phySecId++)
        {
            Nvm_PhySectorId_t globalPhySectorId = Nvm_Pool_GetPhySectorOffset(poolId) + phySecId;
            if (Nvm_PhySector_IsCorrupt(globalPhySectorId))
            {
#if GP_NVM_NBR_OF_REDUNDANT_SECTORS == 1
                /* only do this is there is no redundancy, because in case of redundancy */
                /* corrupted sectors are restored from the redundant sector by Nvm_Pool_AssureRedundancy */
                Nvm_LogSector_SalvageEntriesFromBrokenSector(poolId, globalPhySectorId);
#endif /* GP_NVM_NBR_OF_REDUNDANT_SECTORS == 1 */
                GP_LOG_PRINTF("Erase corrupted sector @ %08lx", 0, (unsigned long)NVM_SECTOR_ADDROFFSET(globalPhySectorId));
                Nvm_Flash_EraseSector(globalPhySectorId);
            }
        }
    }
}

//------------------------ SECTOR CACHE ------------------------

static void Nvm_LogSector_Init(Nvm_SectorLut_t *cacheEntry)
{
    UIntLoop i;

    for (i = 0; i < NVM_NUMBER_PAGES_PER_SECTOR; i++)
    {
        cacheEntry->pageWriteOffset[i] = NVM_SECTOR_HEADER_SIZE;
    }
    for (i = 0; i < GP_NVM_NBR_OF_REDUNDANT_SECTORS; i++)
    {
        cacheEntry->phySectorNbr[i] = NVM_SECTORID_INVALID;
    }
    cacheEntry->InactiveSum = 0;
}

static void Nvm_SectorLuts_Init(void)
{
    Nvm_SectorLoop_t logSecId;

    gpNvm_PoolId_t poolId;
    for (poolId = 0; poolId < Nvm_Flash_NbrOfPools; ++poolId)
    {
        Nvm_SectorLut_t *sectorLut = Nvm_Pool_SectorLut_Get(poolId);

        for (logSecId = 0; logSecId < Nvm_Pool_LogSectors_GetNbr(poolId); logSecId++)
        {
            Nvm_LogSector_Init(&sectorLut[logSecId]);
        }
        Nvm_MaskedSeqNum_Highest[poolId] = NVM_INITIAL_GLOBAL_COUNTER;  // might be overwritten during rebuild of the lookup tables
        Nvm_MaskedSeqNum_Lowest[poolId].maskedSeqNum = NVM_SEQNUM_INVALID;
        Nvm_MaskedSeqNum_LowestInUpperHalf[poolId].maskedSeqNum = NVM_SEQNUM_INVALID;
        Nvm_MaskedSeqNum_HighestInLowerHalf[poolId] = NVM_SEQNUM_INVALID;
        Nvm_Pool_NumberOfUniqueTokens[poolId] = 0;
    }
}

static void Nvm_LastPhySector_Init(void)
{
    for (uint8_t poolId = 0; poolId < Nvm_Flash_NbrOfPools; ++poolId)
    {
        Nvm_PhySectorId_t lastPhySectorOfPool = Nvm_Pool_GetPhySectorOffset(poolId) + Nvm_Pool_PhySectors_GetNbr(poolId);
        GP_ASSERT_DEV_INT(lastPhySectorOfPool > 0);
        /* defaults used with empty pool; make it so it will wrap to the first sector*/
        Nvm_Pool_LastAllocatedPhySector[poolId] = lastPhySectorOfPool - 1;
    }
}

static Bool Nvm_LogSector_InUse(gpNvm_PoolId_t poolId, Nvm_LogSectorId_t id)
{
    GP_ASSERT_DEV_EXT(id < Nvm_Pool_LogSectors_GetNbr(poolId));
    Nvm_SectorLut_t *sectorLut = Nvm_Pool_SectorLut_Get(poolId);

    UIntLoop mirrorId;

    for (mirrorId = 0; mirrorId < GP_NVM_NBR_OF_REDUNDANT_SECTORS; mirrorId++)
    {
        if (sectorLut[id].phySectorNbr[mirrorId] != NVM_SECTORID_INVALID)
        {
            return true;
        }
    }
    return false;
}

static UInt32 Nvm_LogSector_GetPhyFlashOffset(gpNvm_PoolId_t poolId, Nvm_LogSectorId_t id)
{
    GP_ASSERT_DEV_EXT(id < Nvm_Pool_LogSectors_GetNbr(poolId));
    Nvm_SectorLut_t *sectorLut = Nvm_Pool_SectorLut_Get(poolId);

    return (NVM_SECTOR_ADDROFFSET(sectorLut[id].phySectorNbr[0]));
}

static void Nvm_Pool_SectorLut_Build(gpNvm_PoolId_t poolId)
{
    Nvm_SectorLoop_t phySecId;

    UInt8 physicalOffset = Nvm_Pool_GetPhySectorOffset(poolId);
    for (phySecId = 0; phySecId < Nvm_Pool_PhySectors_GetNbr(poolId); phySecId++)
    {
        UInt8 pHeader[NVM_SECTOR_HEADER_SIZE];
        Nvm_Flash_Read(NVM_SECTOR_ADDROFFSET(physicalOffset+phySecId), NVM_SECTOR_HEADER_SIZE, pHeader);

        GP_LOG_PRINTF("Mapping %x -> %x", 0, pHeader[NVM_SECTOR_HEADER_OFFSET_TO_ID], phySecId);
        if (Nvm_PhySector_IsValidHeader(pHeader))
        {
            if(Nvm_PhySector_IsCorrupt(physicalOffset+phySecId)!=true)
            {
                Nvm_Pool_RegisterPhySect(poolId, pHeader[NVM_SECTOR_HEADER_OFFSET_TO_ID], physicalOffset +phySecId);
            }
        }
    }
}


static void Nvm_SectorLuts_LoadDetails(void)
{
    Nvm_SectorLoop_t logSecId;

    GP_LOG_PRINTF("Nvm_SectorLuts_LoadDetails->",0);

    gpNvm_PoolId_t poolId;
    for (poolId = 0; poolId < Nvm_Flash_NbrOfPools; poolId++)
    {
        for (logSecId = 0; logSecId < Nvm_Pool_LogSectors_GetNbr(poolId); logSecId++)
        {
            if (!Nvm_LogSector_InUse(poolId, logSecId)) {
                continue;
            }
            Nvm_LogSector_Load(poolId, logSecId);
        }


    }
    GP_LOG_PRINTF("Nvm_SectorLuts_LoadDetails<-",0);
}
static void Nvm_Pools_CalculateInactiveSums(void)
{
    gpNvm_PoolId_t poolId;
    for(poolId = 0; poolId < Nvm_Flash_NbrOfPools; poolId++)
    {
        /* Now we can build a tokenlut... */
        Nvm_Pool_CalculateInactiveSums(poolId);
    }
}
static void Nvm_Pool_CalculateInactiveSums(gpNvm_PoolId_t poolId)
{
    Nvm_TokenLut_t *lut;
    Bool freeAfterUse;
    UInt8 NbrOfLogicalSectors = Nvm_Pool_LogSectors_GetNbr(poolId);

    Nvm_SectorLut_t *sectorLut = Nvm_Pool_SectorLut_Get(poolId);
    gpNvm_LookupTable_Handle_t lutHandle;
    gpNvm_Result_t result;

    result = Nvm_AcquireLutHandle(&lutHandle, poolId, 0, NULL, &freeAfterUse, Nvm_NumberOfUniqueTokens, true, true);

    if ((result != gpNvm_Result_DataAvailable)
     && (result != gpNvm_Result_NoDataAvailable)) /* when the flash is empty, the LUT is empty */
    {
        /* most likely a gpNvm_Result_Truncated due to more unique keys in the
           flash than supported by the build configuration (Nvm_NumberOfUniqueTokens) */
        GP_ASSERT_DEV_INT(false);
        return;
    }
    lut = NVM_LUTHANDLE_TO_LUT(lutHandle);

    Nvm_LogSectorId_t    logSecId;
    for (logSecId = 0; logSecId < NbrOfLogicalSectors; logSecId++)
    {
        UIntLoop pageId;
        UInt16 SizeInActive = 0;

        if (!Nvm_LogSector_InUse(poolId, logSecId))
        {
            continue;
        }

        for (pageId = 0; pageId < NVM_NUMBER_PAGES_PER_SECTOR; pageId++)
        {
            UInt8 pData[NVM_RECORD_HEADER_SIZE + GP_NVM_MAX_TOKENLENGTH]; //Only header + token

            UInt32 pageAddrOffset = NVM_SECTOR_HEADER_SIZE;
            while (pageAddrOffset <= NVM_MAX_RECORD_OFFSET)
            {
                UInt32 offsetInSector = (UInt32)pageId * FLASH_PAGE_SIZE + pageAddrOffset;

                Nvm_Flash_Read(
                        Nvm_LogSector_GetPhyFlashOffset(poolId, logSecId) + offsetInSector,
                        NVM_RECORD_HEADER_SIZE,
                        pData
                );
                const UInt8 payloadLength = pData[ NVM_RECORD_HEADER_OFFSET_TO_PAYLOADLENGTH ];
                const UInt8 tokenLength = pData[ NVM_RECORD_HEADER_OFFSET_TO_TOKENLENGTH ];
                const UInt8 bodyLength = tokenLength + payloadLength ;
                UInt8 *token = &pData[NVM_RECORD_HEADER_OFFSET_TO_TOKEN];

                if (tokenLength == 0)
                {
                    break;  // past last tag in this page
                }

                GP_ASSERT_DEV_INT(tokenLength <= GP_NVM_MAX_TOKENLENGTH);
#if (GP_NVM_MAX_PAYLOADLENGTH < 255)
                GP_ASSERT_DEV_INT(payloadLength <= GP_NVM_MAX_PAYLOADLENGTH);
#endif
                if(Nvm_IsRecordHeaderCorrupted(tokenLength, payloadLength, bodyLength, pageAddrOffset))
                {
                    GP_LOG_SYSTEM_PRINTF("ERROR: Token header corrupted! tl=%u pl=%u bl=%u po=%lu",0, tokenLength, payloadLength, bodyLength, (unsigned long int)pageAddrOffset);
                    break;
                }

                Nvm_Flash_Read(
                        Nvm_LogSector_GetPhyFlashOffset(poolId, logSecId) + offsetInSector + NVM_RECORD_HEADER_OFFSET_TO_TOKEN,
                        tokenLength,
                        token
                );

                {
                    Nvm_Token_t *lutItem = NULL;
                    Nvm_TokenLut_Iterator_t lookupIter;
                    gpNvm_Result_t lookUpResult;

                    lookUpResult = Nvm_TokenLut_Query(&lutItem, lut, tokenLength, token , true, &lookupIter);

                    GP_ASSERT_DEV_INT(lookUpResult == gpNvm_Result_DataAvailable || lookUpResult == gpNvm_Result_NoDataAvailable);
                    if (lookUpResult == gpNvm_Result_DataAvailable)
                    {
                        GP_ASSERT_SYSTEM(lutItem);
                        if ((logSecId != lutItem->logSecId)
                         || (offsetInSector != SECTOROFFSET_UNPACK(lutItem->packed_sectorOffset)))
                        {
                            // Another logical sector contains this item; could be removed here.
                            SizeInActive += NVM_ALIGNED_RECORD_SIZE(tokenLength, payloadLength);
                        }
                        else
                        {
                            /* consider active removal records as obsolete if no other sectors
                             * contain this token */
                            Bool isDeleted = (payloadLength == NVM_PAYLOADLENGTH_INDICATING_TOKENREMOVED);

                            if(isDeleted && lutItem->nbrOfSectorReferences == 1)
                            {
                                SizeInActive += NVM_ALIGNED_RECORD_SIZE(tokenLength, payloadLength);
                            }
                        }
                    }
                }

                // Skip to next record in page.
                pageAddrOffset += NVM_ALIGNED_RECORD_SIZE(tokenLength, payloadLength);
            }
        }
        sectorLut[logSecId].InactiveSum = SizeInActive;
    }

    if (freeAfterUse)
    {
        gpNvm_FreeLookup(lutHandle);
    }
}

static Nvm_LogSectorId_t Nvm_Pool_GetFreeLogSectorId(gpNvm_PoolId_t poolId)
{
    Nvm_SectorLoop_t logSecId;

    for (logSecId = 0; logSecId < Nvm_Pool_LogSectors_GetNbr(poolId); logSecId++)
    {
        if (!Nvm_LogSector_InUse(poolId, logSecId))
        {
            return logSecId;
        }
    }
    /* Assert since if we come here we did not found a free logical sector */
    GP_ASSERT_SYSTEM(false);
    return NVM_SECTORID_INVALID;
}

static gpHal_FlashError_t Nvm_PhySector_Initialize(
    gpNvm_PoolId_t poolId,
    Nvm_PhySectorId_t phySecId,
    Nvm_SectorLoop_t logSecId
)
{
    gpHal_FlashError_t flasherror;
    UInt32 pData_32[NVM_SECTOR_HEADER_SIZE/sizeof(UInt32)];
    UInt8 *pData_8 = (UInt8*)pData_32;
    UIntLoop pageId;

    flasherror = gpHal_FlashBlankCheck(Nvm_Base + NVM_PAGE_ADDROFFSET(phySecId,0), FLASH_SECTOR_SIZE / 4 );
    if (flasherror == gpHal_FlashError_BlankFailure)
    {
        flasherror = gpHal_FlashEraseSector( Nvm_Base + NVM_PAGE_ADDROFFSET(phySecId,0));
    }
    if (flasherror != gpHal_FlashError_Success)
    {
        return flasherror;
    }

    MEMSET(pData_8, 0x00, NVM_SECTOR_HEADER_SIZE);
    pData_8[NVM_SECTOR_HEADER_OFFSET_TO_ID]        = logSecId;
    pData_8[NVM_SECTOR_HEADER_OFFSET_TO_FREQUENCY] = 0;
    pData_8[NVM_SECTOR_HEADER_OFFSET_TO_VALIDFLAG] = NVM_SECTOR_HEADER_VALIDFLAG_VALUE;
    pData_8[NVM_SECTOR_HEADER_OFFSET_TO_ERASEPENDING] = 0;

    for (pageId = 0; pageId < NVM_NUMBER_PAGES_PER_SECTOR; pageId++)
    {
        flasherror = Nvm_Flash_Write(NVM_PAGE_ADDROFFSET(phySecId,pageId), NVM_SECTOR_HEADER_SIZE, pData_32);
        if (flasherror != gpHal_FlashError_Success)
        {
            return flasherror;
        }
    }
    return flasherror;
}

static Nvm_SectorLoop_t Nvm_LogSector_Create(
    gpNvm_PoolId_t poolId
)
{
    Nvm_SectorLut_t *sectorLut = Nvm_Pool_SectorLut_Get(poolId);
    Nvm_SectorLoop_t logSecId;
    gpHal_FlashError_t flasherror;
    UIntLoop mirrorId;

    logSecId = Nvm_Pool_GetFreeLogSectorId(poolId);
    if (logSecId == NVM_SECTORID_INVALID)
    {
        return NVM_SECTORID_INVALID;
    }

    for (mirrorId = 0; mirrorId < GP_NVM_NBR_OF_REDUNDANT_SECTORS; mirrorId++)
    {
        Nvm_PhySectorId_t attempts = 0;
        Nvm_PhySectorId_t maxAttempts = Nvm_Pool_PhySectors_GetNbr(poolId);
        do
        {
            Nvm_PhySectorId_t phySecId = Nvm_Pool_GetFreePhySectorId(poolId);
            sectorLut[logSecId].phySectorNbr[mirrorId] = phySecId;

            flasherror = Nvm_PhySector_Initialize(poolId, phySecId, logSecId);
            if (flasherror != gpHal_FlashError_Success)
            {
                Nvm_PhySector_MarkBroken(phySecId);
            }
            attempts++;
        } while (flasherror != gpHal_FlashError_Success && (attempts < maxAttempts));
        GP_ASSERT_SYSTEM(attempts < maxAttempts);
    }

    sectorLut[logSecId].pageWriteOffset[0] = NVM_SECTOR_HEADER_SIZE;
    sectorLut[logSecId].pageWriteOffset[1] = NVM_SECTOR_HEADER_SIZE;
    return logSecId;
}

static void Nvm_LogSector_Load(gpNvm_PoolId_t poolId, Nvm_LogSectorId_t logSecId)
{
    UIntLoop pageId;
    Nvm_PhySectorId_t phySecId;
    Nvm_SectorLut_t *sectorLut = Nvm_Pool_SectorLut_Get(poolId);

    GP_ASSERT_DEV_EXT(poolId < Nvm_Flash_NbrOfPools);
    GP_ASSERT_DEV_EXT(logSecId < Nvm_Pool_LogSectors_GetNbr(poolId));

    phySecId  = sectorLut[ logSecId ].phySectorNbr[0];

    for (pageId = 0; pageId < FLASH_PAGES_PER_SECTOR; pageId++)
    {
        UInt8 pData[NVM_RECORD_HEADER_SIZE];

        UInt32 pageAddrOffset = NVM_SECTOR_HEADER_SIZE;
        while (pageAddrOffset <= NVM_MAX_RECORD_OFFSET)
        {
            UInt8 tokenLength;
            UInt8 payloadLength;
            Nvm_MaskedSeqNum_t seqNum;

            Nvm_Flash_Read(
                    NVM_PAGE_ADDROFFSET(phySecId,pageId) + pageAddrOffset,
                    NVM_RECORD_HEADER_SIZE,
                    pData
                );

            tokenLength   = pData[NVM_RECORD_HEADER_OFFSET_TO_TOKENLENGTH];
            payloadLength = pData[NVM_RECORD_HEADER_OFFSET_TO_PAYLOADLENGTH];

            if (tokenLength == 0)
            {
                break;  //we're done with this page: we've reached an empty length
            }

            seqNum = Nvm_Buf_Get_UInt16(pData, NVM_RECORD_HEADER_OFFSET_TO_SEQNUM_COUNTER);
            Nvm_Pool_SeqNum_RegisterSeqNumceCtr(poolId,seqNum,logSecId);

            pageAddrOffset += NVM_ALIGNED_RECORD_SIZE(tokenLength, payloadLength);
        }
        sectorLut[logSecId].pageWriteOffset[pageId] = pageAddrOffset;
    }
}

static void Nvm_Pool_RegisterPhySect(gpNvm_PoolId_t poolId, Nvm_LogSectorId_t id, Nvm_PhySectorId_t phySecNbr)
{
    GP_ASSERT_DEV_EXT(id < Nvm_Pool_LogSectors_GetNbr(poolId));
    Nvm_SectorLut_t *sectorLut = Nvm_Pool_SectorLut_Get(poolId);

    UIntLoop mirrorId;

    for (mirrorId = 0; mirrorId < GP_NVM_NBR_OF_REDUNDANT_SECTORS; mirrorId++)
    {
        if (sectorLut[id].phySectorNbr[mirrorId] == NVM_SECTORID_INVALID)
        {
            sectorLut[id].phySectorNbr[mirrorId] = phySecNbr;
            return;
        }
    }
    //register phy sect impossible as no free slots available
    GP_ASSERT_SYSTEM(false);
}

#if GP_NVM_NBR_OF_REDUNDANT_SECTORS > 1
static void Nvm_Pool_AssureRedundancy(gpNvm_PoolId_t poolId)
{
    Nvm_SectorLoop_t  logSecId;
    Nvm_SectorLut_t *sectorLut = Nvm_Pool_SectorLut_Get(poolId);

    for (logSecId = 0; logSecId < Nvm_Pool_LogSectors_GetNbr(poolId); logSecId++)
    {
        UInt8 validCnt = 0;
        UIntLoop mirrorId;

        // only handle valid data
        if (!Nvm_LogSector_InUse(poolId, logSecId))
        {
            continue;
        }

        for (mirrorId = 0; mirrorId < GP_NVM_NBR_OF_REDUNDANT_SECTORS; mirrorId++)
        {
            if (sectorLut[logSecId].phySectorNbr[mirrorId] != NVM_SECTORID_INVALID)
            {
                validCnt++;
            }
        }


        //nothing to re-duplicate
        if (validCnt == GP_NVM_NBR_OF_REDUNDANT_SECTORS)
        {
            continue;
        }

        for (mirrorId = 0; mirrorId < GP_NVM_NBR_OF_REDUNDANT_SECTORS; mirrorId++)
        {
            if (sectorLut[logSecId].phySectorNbr[mirrorId] == NVM_SECTORID_INVALID)
            {
                Nvm_LogSector_ReplaceBroken(poolId, logSecId, mirrorId, DONT_MARK_BROKEN_AFTER_REPLACEMENT);
            }
        }
    }
}

static void Nvm_LogSector_ReplaceBroken(gpNvm_PoolId_t poolId, Nvm_LogSectorId_t id, UInt8 corruptedMirror, Bool markBroken)
{
    Nvm_PhySectorId_t newPhySectorId    = NVM_SECTORID_INVALID;
    Nvm_PhySectorId_t corruptedPhySecId = NVM_SECTORID_INVALID;
    Nvm_PhySectorId_t knownGoodPhySecId = NVM_SECTORID_INVALID;
    UIntLoop mirrorId;
    Nvm_SectorLut_t *sectorLut = Nvm_Pool_SectorLut_Get(poolId);

    GP_LOG_PRINTF("ReplaceBroken pool:%d logsec:%d mirror:%d", 0, poolId, id, corruptedMirror);

    GP_ASSERT_DEV_EXT(id < Nvm_Pool_LogSectors_GetNbr(poolId));

    //search for known good phy and corrupted phys sectors
    for (mirrorId = 0; mirrorId < GP_NVM_NBR_OF_REDUNDANT_SECTORS; mirrorId++)
    {
        if (
             (mirrorId != corruptedMirror) &&
             (sectorLut[id].phySectorNbr[mirrorId] != NVM_SECTORID_INVALID)  &&
             (!Nvm_PhySector_IsCorrupt(sectorLut[id].phySectorNbr[mirrorId]))
           )
        {
            knownGoodPhySecId = sectorLut[id].phySectorNbr[mirrorId];
            break;
        }
    }

    GP_ASSERT_SYSTEM(knownGoodPhySecId != NVM_SECTORID_INVALID);

    corruptedPhySecId = sectorLut[id].phySectorNbr[corruptedMirror];

    if (corruptedPhySecId != NVM_SECTORID_INVALID)
    {
        if (markBroken == MARK_BROKEN_AFTER_REPLACEMENT)
        {
            Nvm_PhySector_MarkBroken(corruptedPhySecId);      // lost "forever"
        }
        else
        {   // sector was not really broken due to verify failure: retry (same) sector after erasure
            Nvm_Flash_EraseSector(corruptedPhySecId);
        }
    }

    newPhySectorId = Nvm_Pool_GetFreePhySectorId(poolId);

    // copy now from the knownGoodPhySecId, then we're safe again
    {
        UInt32 pData_32[NVM_DEFAULT_BLOCK_SIZE/sizeof(UInt32)];
        UInt8* pData_8 = (UInt8*)pData_32;
        UInt16 offset;

        for (offset = 0; offset < FLASH_SECTOR_SIZE; offset += sizeof(pData_32))
        {
            Nvm_Flash_Read(  NVM_SECTOR_ADDROFFSET(knownGoodPhySecId) + offset, sizeof(pData_32), pData_8);
            Nvm_Flash_Write( NVM_SECTOR_ADDROFFSET(newPhySectorId)    + offset, sizeof(pData_32), pData_32);
       }
    }

    sectorLut[id].phySectorNbr[corruptedMirror] = newPhySectorId;
}
#endif //GP_NVM_NBR_OF_REDUNDANT_SECTORS > 1

#if GP_NVM_NBR_OF_REDUNDANT_SECTORS == 1
static Bool Nvm_LogSector_SalvageEntriesFromBrokenPhySectorToLogicalSector(gpNvm_PoolId_t poolId, Nvm_PhySectorId_t phySecId, Nvm_LogSectorId_t logSecId)
{
    UIntLoop pageId;
    Nvm_PhySectorId_t newPhySectorId;
    Bool SectorRelocatedSuccessfully = true;

    /* retrieve the phy sector where the new logical sector is located */
    Nvm_SectorLut_t *sectorLut = Nvm_Pool_SectorLut_Get(poolId);
    newPhySectorId = sectorLut[logSecId].phySectorNbr[0]; /* hardcoded zero: GP_NVM_NBR_OF_REDUNDANT_SECTORS == 1 */

    /* Check all pages.*/
    for (pageId = 0; pageId < NVM_NUMBER_PAGES_PER_SECTOR; pageId++)
    {
        /* Check all records in the page.*/
        UInt32 pageAddrOffset = NVM_SECTOR_HEADER_SIZE;
        UInt16 alignedRecordSize;
        while (pageAddrOffset <= NVM_MAX_RECORD_OFFSET)
        {
            /* pData must be a multiple of FLASH_WRITE_UNIT, because "Nvm_ReadRecordAndCheckCorrupted" will also
             * add the padding up to a multiple of FLASH_WRITE_UNIT in this array
             */
            UInt32 pData_32[NVM_DIV_ROUND_UP(NVM_MAX_ALIGNED_RECORD_SIZE, sizeof(UInt32))];
            UInt8* pData_8 = (UInt8*)pData_32;
            /* corrupted sector pointer */
            UInt32 ptrTag       = NVM_PAGE_ADDROFFSET(phySecId,pageId)       + pageAddrOffset;
            /* new sector pointer */
            UInt32 ptrTagTarget = NVM_PAGE_ADDROFFSET(newPhySectorId,pageId) + pageAddrOffset;

            Bool isCorruptedRecord;
            Bool isLastRecord;
            isCorruptedRecord = Nvm_ReadRecordAndCheckCorrupted(ptrTag, pageAddrOffset, pData_8, &isLastRecord, &alignedRecordSize);

            if(isLastRecord == true)
            {
                break;
            }
            if(isCorruptedRecord == true)
            {
                break;
            }

            /* save to the new sector */
            gpHal_FlashError_t state;
            state = Nvm_Flash_Write(
                            ptrTagTarget,
                            alignedRecordSize,
                            &(pData_32[0])
                        );

            if (state == gpHal_FlashError_VerifyFailure)
            {
                Nvm_PhySector_MarkBroken(newPhySectorId);
                SectorRelocatedSuccessfully = false;
                return SectorRelocatedSuccessfully;
            }

            // Skip to next record in page (if any).
            pageAddrOffset += alignedRecordSize;
        }
    }

    return SectorRelocatedSuccessfully;
}

static void Nvm_LogSector_SalvageEntriesFromBrokenSector(gpNvm_PoolId_t poolId, Nvm_PhySectorId_t phySecId)
{
    UIntLoop pageId;
    Nvm_LogSectorId_t oldLogSecId = NVM_SECTORID_INVALID;
    Nvm_LogSectorId_t logSecId;
    UInt8 pHeader[NVM_SECTOR_HEADER_SIZE];
    Bool SectorRelocatedSuccessfully = false;

    /* read the logical sector Id of the sector which we're going to replace */
    for (pageId = 0; pageId < NVM_NUMBER_PAGES_PER_SECTOR; pageId++)
    {
        Nvm_Flash_Read(NVM_PAGE_ADDROFFSET(phySecId,pageId), NVM_SECTOR_HEADER_SIZE, pHeader);
        if (Nvm_PhySector_IsValidHeader(pHeader))
        {
            oldLogSecId = pHeader[NVM_SECTOR_HEADER_OFFSET_TO_ID];
        }
    }

    if(oldLogSecId == NVM_SECTORID_INVALID)
    {
        /* can't save anything here. */
        return;
    }

    /* prepare the logical sector lists */
    Nvm_SectorLuts_Init();
    Nvm_SectorLuts_Load();

    while(SectorRelocatedSuccessfully == false)
    {
        /* get a new logical sector */
        logSecId = Nvm_LogSector_Create(poolId);
        if(logSecId == NVM_SECTORID_INVALID)
        {
            /* I'm giving up. */
            GP_ASSERT_SYSTEM(false);
            return;
        }

        SectorRelocatedSuccessfully = Nvm_LogSector_SalvageEntriesFromBrokenPhySectorToLogicalSector(poolId, phySecId, logSecId);
    }

}
#endif /* GP_NVM_NBR_OF_REDUNDANT_SECTORS == 1 */

static Bool Nvm_Pool_IsFull(gpNvm_PoolId_t poolId)
{
    UInt8 nbrOfPhySectors = Nvm_Pool_PhySectors_GetNbr(poolId);
    UInt8 nbrOfPhySectorsUsed = Nvm_Pool_PhySectors_GetNbrUsed(poolId);

    /* "Check if pool is full": Verify to make sure there are always
     * "GP_NVM_NBR_OF_REDUNDANT_SECTORS" number of physical sectors available since this is
     * required to be able to always perform a proper defragementation action at all time */
    UInt8 nbrOfAvailableSectors = nbrOfPhySectors - nbrOfPhySectorsUsed;
    if (nbrOfAvailableSectors < GP_NVM_NBR_OF_REDUNDANT_SECTORS*NVM_NBR_OF_LOGICAL_SECTORS_REQUIRED_FOR_DEFRAG)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static Bool Nvm_Pool_IsNamespaceExhausted(gpNvm_PoolId_t poolId)
{
    return (Nvm_Pool_NumberOfUniqueTokens[poolId] == Nvm_NumberOfUniqueTokens);
}
static Bool Nvm_Pool_IsUrgentDefragRequired(gpNvm_PoolId_t poolId)
{
    UInt8 nbrOfPhySectors = Nvm_Pool_PhySectors_GetNbr(poolId);
    UInt8 nbrOfPhySectorsUsed = Nvm_Pool_PhySectors_GetNbrUsed(poolId);

    /* "Check to start urgent defragmentation": Verify to make sure there are always
     * "GP_NVM_NBR_OF_REDUNDANT_SECTORS" number of physical sectors available since this is
     * required to be able to always perform a proper defragementation action at all time */
    UInt8 nbrOfAvailableSectors = nbrOfPhySectors - nbrOfPhySectorsUsed;
    Bool tooFewSectorsAvailable = nbrOfAvailableSectors <= GP_NVM_NBR_OF_REDUNDANT_SECTORS * NVM_NBR_OF_LOGICAL_SECTORS_REQUIRED_FOR_DEFRAG;

    return (tooFewSectorsAvailable);
}

static Bool Nvm_Pool_IsExtendedDefragRequired(gpNvm_PoolId_t poolId)
{
    UInt16 nbrOfPhySectors = Nvm_Pool_PhySectors_GetNbr(poolId);
    UInt8 nbrOfPhySectorsUsed = Nvm_Pool_PhySectors_GetNbrUsed(poolId);
    /* "Check to start extended defragmentation": Verify to what extent we are reaching
     * the limits of the NVM memory. Schedule a extended defragmentation action if required */
    if (nbrOfPhySectorsUsed >= ((nbrOfPhySectors*GP_NVM_BACKGROUND_DEFRAG_RATIO)/100))
    {
        return true;
    }
    else
    {
        return false;
    }
}

static void Nvm_Pool_TryToStartExtendedDefrag(gpNvm_PoolId_t poolId)
{
    if ((Nvm_DefragmentationDisableCounter == 0) && Nvm_Pool_IsExtendedDefragRequired(poolId))
    {
        Nvm_SectorPool_ScheduleDefrag(0, poolId);
    }
}

//------------------------ MISC HELPER METHODS ------------------------

static UInt16 Nvm_Flash_FreeSpaceLeftInPage(UInt16 freeStart)
{
    UInt16 maxSize = FLASH_PAGE_SIZE;
    GP_ASSERT_SYSTEM(freeStart <= maxSize);
    return (maxSize - freeStart);
}

static Bool Nvm_Pool_Record_Allocate(
    gpNvm_PoolId_t poolId,
    UInt16 lengthOfNewData,
    Nvm_LogSectorId_t *pLogSecId,
    UInt8 *pPageId,
    gpNvm_DryRun_t dryRun
)
{
    Nvm_LogSectorId_t logSecId;
    Nvm_LogSectorId_t logSecIdPicked;
    UIntLoop pageId = 0;
    Bool allocateNewSector;
    Nvm_SectorLut_t *sectorLut = Nvm_Pool_SectorLut_Get(poolId);

    GP_ASSERT_DEV_EXT(pPageId);
    GP_ASSERT_DEV_EXT(pLogSecId);

    *pPageId = PAGEID_UNUSED;

    /* Find best fitting sector, starting from id 0, to allocate memory */
    allocateNewSector = true;
    UInt16 freeSpacePicked = 0xffff;
    UInt16 InactiveSpacePicked = 0xffff;
    for (logSecId = 0; logSecId < Nvm_Pool_LogSectors_GetNbr(poolId); logSecId++)
    {
        /* Skip sectors not in use */
        if (!Nvm_LogSector_InUse(poolId, logSecId))
        {
            continue;
        }

        if(sectorLut[logSecId].pageWriteOffset[0] == NVM_WRITE_OFFSET_WRITELOCK_MW || sectorLut[logSecId].pageWriteOffset[1] == NVM_WRITE_OFFSET_WRITELOCK_MW)
        {
            /* Skip sectors which are defragmenting */
            continue;
        }

        UInt16 freeSpacePageA = Nvm_Flash_FreeSpaceLeftInPage(sectorLut[logSecId].pageWriteOffset[0]);
        UInt16 freeSpacePageB = Nvm_Flash_FreeSpaceLeftInPage(sectorLut[logSecId].pageWriteOffset[1]);
        /* Skip sectors without space available */
        if (freeSpacePageA < lengthOfNewData && freeSpacePageB < lengthOfNewData)
        {
            continue;
        }
        /* Skip sectors which are candidate for defragmentation */
        if (Nvm_LogSector_IsDefragCandidate(poolId, logSecId))
        {
            continue;
        }
        UInt16 leastFreeSpace = 0xffff;

        if (freeSpacePageA >= lengthOfNewData)
        {
            leastFreeSpace = freeSpacePageA ;
        }
        if (freeSpacePageB >= lengthOfNewData && freeSpacePageB < leastFreeSpace)
        {
            leastFreeSpace = freeSpacePageB ;
        }

        if (!Nvm_Pool_IsUrgentDefragRequired(poolId))
        {
            /* Keep track of sector which has best fit for data */
            if (leastFreeSpace < freeSpacePicked)
            {
                allocateNewSector = false;
                logSecIdPicked = logSecId;
                freeSpacePicked = leastFreeSpace;
                pageId = (leastFreeSpace == freeSpacePageA) ? 0 : 1;
            }
        }
        else
        {
            /*  pick sector with least amount of inactive data */
            if (InactiveSpacePicked > sectorLut[logSecId].InactiveSum)
            {
                InactiveSpacePicked = sectorLut[logSecId].InactiveSum;
                allocateNewSector = false;
                logSecIdPicked = logSecId;
                freeSpacePicked = leastFreeSpace;
                pageId = (leastFreeSpace == freeSpacePageA) ? 0 : 1;
            }
        }
    }
    if (dryRun == gpNvm_DryRun_yes)
    {
        return (!allocateNewSector);
    }

    if (allocateNewSector)
    {
        logSecIdPicked = Nvm_LogSector_Create(poolId);
        GP_LOG_PRINTF("alloc sector pool:%d logsec:%d", 0, poolId, logSecIdPicked);

        GP_ASSERT_DEV_EXT(logSecIdPicked < Nvm_Pool_LogSectors_GetNbr(poolId));
        if (logSecIdPicked != NVM_SECTORID_INVALID)
        {
            //check which page to use
            for (pageId = 0; pageId < NVM_NUMBER_PAGES_PER_SECTOR; pageId++)
            {
                if (Nvm_Flash_FreeSpaceLeftInPage(sectorLut[logSecIdPicked].pageWriteOffset[pageId]) >= lengthOfNewData)
                {
                    break;
                }
            }
            GP_ASSERT_SYSTEM(pageId != NVM_NUMBER_PAGES_PER_SECTOR);
        }
    }

    (*pLogSecId) = logSecIdPicked;
    (*pPageId) = pageId;

    return (logSecIdPicked != NVM_SECTORID_INVALID);
}

static Bool Nvm_LogSector_StoreRecord(
                    gpNvm_PoolId_t       poolId,
                    Nvm_LogSectorId_t    logSecId,
                    UInt8                pageId,
                    UInt16               length_8,
                    UInt32               *pData_32,
                    UInt16*              pRetOffsetInSector
                )
{
    UIntLoop           mirrorId;
    gpHal_FlashError_t state;

    Nvm_SectorLut_t *sectorLut = Nvm_Pool_SectorLut_Get(poolId);
    UInt16 offsetInSector = (UInt16)pageId * FLASH_PAGE_SIZE + sectorLut[logSecId].pageWriteOffset[pageId];

    GP_ASSERT_DEV_EXT(logSecId < Nvm_Pool_LogSectors_GetNbr(poolId));
    GP_ASSERT_DEV_EXT(pageId < FLASH_PAGES_PER_SECTOR);
    GP_ASSERT_DEV_EXT(length_8 <=(FLASH_PAGE_SIZE));
    GP_ASSERT_DEV_EXT(length_8 != 0);

    GP_LOG_PRINTF("Nvm_LogSector_StoreRecord pool:%d logsec:%d page:%d", 0, (int)poolId, logSecId, pageId);

    for (mirrorId = 0; mirrorId < GP_NVM_NBR_OF_REDUNDANT_SECTORS; mirrorId++)
    {
        Int8 attempt;

        for(attempt = 3; attempt > 0; attempt--)
        {
            Nvm_PhySectorId_t phySecId = sectorLut[logSecId].phySectorNbr[mirrorId];

            GP_ASSERT_SYSTEM(phySecId != NVM_SECTORID_INVALID);

            state = Nvm_Flash_Write(
                            NVM_SECTOR_ADDROFFSET(phySecId) + offsetInSector,
                            length_8,
                            pData_32
                        );
#if GP_NVM_NBR_OF_REDUNDANT_SECTORS > 1
            if (state == gpHal_FlashError_VerifyFailure)
            {
                GP_LOG_PRINTF("VerifyFailure, ReplaceBroken", 0);
                Nvm_LogSector_ReplaceBroken(poolId, logSecId, mirrorId, MARK_BROKEN_AFTER_REPLACEMENT);
            }
#endif
            if (state == gpHal_FlashError_Success)
            {
                break;
            }
            GP_LOG_PRINTF("failed, %d attempts left", 0, attempt-1);
        }

        if (attempt == 0)
        {
            return false;
        }
    }

    //all redunant writes are ok so update caches
    if (Nvm_Flash_FreeSpaceLeftInPage(sectorLut[logSecId].pageWriteOffset[pageId]) == length_8)
    {
        /* boundaryTouch = true; */
        sectorLut[ logSecId ].pageWriteOffset[pageId] += (length_8 - 1);
    }
    else
    {
        sectorLut[ logSecId ].pageWriteOffset[pageId] += length_8;
    }

    if (pRetOffsetInSector)
    {
        (*pRetOffsetInSector) = offsetInSector;
    }
    return true;
}

static void Nvm_TokenLuts_Init(void)
{
#ifndef GP_NVM_DIVERSITY_USE_POOLMEM
    Nvm_BssData.inUse = false;
#endif

    UIntLoop i;
    for (i= 0; i< GP_NVM_NBR_OF_LOOKUPTABLE_HANDLES; i++)
    {
        NVM_LUTHANDLE_SET_INVALID(i);
    }
}

static void Nvm_SectorLuts_Load(void)
{
    gpNvm_PoolId_t poolId;
    for (poolId = 0; poolId < Nvm_Flash_NbrOfPools; poolId++)
    {
        Nvm_Pool_SectorLut_Build(poolId);
#if GP_NVM_NBR_OF_REDUNDANT_SECTORS > 1
        Nvm_Pool_AssureRedundancy(poolId);
#endif //GP_NVM_NBR_OF_REDUNDANT_SECTORS > 1
    }
}

void Nvm_BuildTagCache(void)
{
    gpNvm_KeyIndex_t nrOfMatches = 0;
    gpNvm_Result_t r;

    if (Nvm_TokenLuts_Handle[NVM_TAGCACHE_HANDLE])
    {
        Nvm_TokenLut_Free(Nvm_TokenLuts_Handle[NVM_TAGCACHE_HANDLE]);
        Nvm_TokenLuts_Handle[NVM_TAGCACHE_HANDLE] = NULL;
    }


    if (!Nvm_TokenLut_Allocate(&Nvm_TokenLuts_Handle[NVM_TAGCACHE_HANDLE], Nvm_NumberOfUniqueTokens))
    {
        GP_ASSERT_DEV_INT(false);
        return;
    }
    /* we set include-inactive-tokens to true so this LUT can be shared with
     * the CalculateInactiveSums function. There is no RAM impact
     * since the LUT size is fixed and inactive entries will be removed
     * before we hit the limits of the LUT.
     */
    r = Nvm_TokenLut_Populate(
        Nvm_TokenLuts_Handle[NVM_TAGCACHE_HANDLE],
        gpNvm_PoolId_Tag,
        0,
        NULL,
        Nvm_NumberOfUniqueTokens,
        &nrOfMatches,
        true,
        true /* includeInactive */
    );
    GP_ASSERT_DEV_INT(r == gpNvm_Result_DataAvailable);
}

void Nvm_FreeTagCache(void)
{
    if (NVM_LUTHANDLE_IS_ACTIVE(NVM_TAGCACHE_HANDLE))
    {
        gpNvm_FreeLookup(NVM_TAGCACHE_HANDLE);
    }
}

void Nvm_TokenLuts_Rebuild(void)
{
    UInt8 i;
    /* invalidate lookuptables */
    for (i=0; i < GP_NVM_NBR_OF_LOOKUPTABLE_HANDLES; i++)
    {
        Nvm_TokenLut_t *lut;
        Nvm_TokenBlock_t   *item;
        if (!NVM_LUTHANDLE_IS_ACTIVE(i))
        {
            continue;
        }

        lut = NVM_LUTHANDLE_TO_LUT(i);
        gpNvm_Result_t r;

        GP_ASSERT_DEV_EXT(lut->firstItemBlock);
        gpNvm_KeyIndex_t maxNrMatches = 0;

        for (item = lut->firstItemBlock; item; item = item->nextBlock)
        {
            item->nrOfItemsUsed = 0;
            maxNrMatches += item->nrOfItemsAllocated;
        }

        // zero out datastructure first!
        r = Nvm_TokenLut_Populate(
            lut,
            lut->poolId,
            lut->tokenMaskLength,
            lut->TokenMask,
            maxNrMatches,
            NULL,
            lut->trailingWildcard,
            lut->includeInactive);
        GP_ASSERT_DEV_INT(r == gpNvm_Result_DataAvailable|| r == gpNvm_Result_NoDataAvailable);
    }
}


gpNvm_Result_t gpNvm_AcquireLutHandle(
    gpNvm_LookupTable_Handle_t* pHandle,
    gpNvm_PoolId_t poolId,
    gpNvm_UpdateFrequency_t updateFrequencySpec,
    UInt8 tokenLength,
    UInt8* pToken,
    Bool* freeAfterUse,
    gpNvm_KeyIndex_t maxNbrOfMatches
)
{
    return Nvm_AcquireLutHandle(
        pHandle,
        poolId,
        tokenLength,
        pToken,
        freeAfterUse,
        maxNbrOfMatches,
        false,
        false
    );
}

/* Obtain a handle to a lut matching given requirements.
 * This allows re-use of LUT's, and related RAM savings */
static gpNvm_Result_t Nvm_AcquireLutHandle(
    gpNvm_LookupTable_Handle_t* pHandle,
    gpNvm_PoolId_t poolIdSpec,
    UInt8 tokenMaskLengthSpec,
    UInt8* pTokenMask,
    Bool* freeAfterUse,
    gpNvm_KeyIndex_t maxNbrOfMatches,
    Bool trailingWildcard,
    Bool includeInactive)
{
    gpNvm_Result_t result;
    UIntLoop i;
    GP_ASSERT_DEV_EXT(pHandle);
    GP_ASSERT_DEV_EXT(freeAfterUse);
    Nvm_TokenLut_t *pLut = NULL;

    (*freeAfterUse) = false;
    for (i = 0; i < GP_NVM_NBR_OF_LOOKUPTABLE_HANDLES; i++)
    {
        if (!NVM_LUTHANDLE_IS_ACTIVE(i))
        {
            continue;
        }

        if(NVM_LUTHANDLE_TO_LUT(i)->writeLock)
        {
            /* Nvm_TokenLut_Populate is reentrant */
            continue;
        }

        if(includeInactive && !NVM_LUTHANDLE_TO_LUT(i)->includeInactive)
        {
            /* don't use a lut missing inactive tokens for namespace cleanup */
            continue;
        }

        if(maxNbrOfMatches > NVM_LUTHANDLE_TO_LUT(i)->nrOfItemsAllocated)
        {
            continue;
        }

        if(Nvm_TokenLut_TokenBelongs(NVM_LUTHANDLE_TO_LUT(i), poolIdSpec, tokenMaskLengthSpec, pTokenMask))
        {
            /* reuse LUT */
            *pHandle = i;
            pLut = NVM_LUTHANDLE_TO_LUT(i);
            result = gpNvm_Result_DataAvailable;
            break;
        }
    }
    if (!pLut)
    {
        result = Nvm_TokenLut_AllocateHandle(pHandle);

        if(result != gpNvm_Result_DataAvailable)
        {
            return result;
        }

        if(!Nvm_TokenLut_Allocate(&pLut, maxNbrOfMatches))
        {
            GP_ASSERT_DEV_INT(false);
            NVM_LUTHANDLE_SET_INVALID(*pHandle);
            return gpNvm_Result_Error;
        }
        result = Nvm_TokenLut_Populate(
            pLut,
            poolIdSpec,
            tokenMaskLengthSpec,
            pTokenMask,
            maxNbrOfMatches,
            NULL,
            trailingWildcard,
            includeInactive);
        if(result == gpNvm_Result_DataAvailable || result == gpNvm_Result_Truncated)
        {
            NVM_LUTHANDLE_SET(*pHandle, pLut);
            (*freeAfterUse) = true;
        }
        else
        {
            gpNvm_FreeLookup(*pHandle);
        }
    }
    return result;
}

static gpNvm_Result_t Nvm_Token_IsDeleted(
    gpNvm_PoolId_t poolId,
    Nvm_Token_t *lutItem,
    Bool *isDeleted
)
{
    UInt8 ItemHeader[NVM_RECORD_HEADER_SIZE];
    gpNvm_Result_t result;
    GP_ASSERT_SYSTEM(isDeleted);
    (*isDeleted) = false;

    result = Nvm_Token_GetRecordHeader(poolId, lutItem, ItemHeader);
    if (result==gpNvm_Result_DataAvailable)
    {
        if (ItemHeader[NVM_RECORD_HEADER_OFFSET_TO_PAYLOADLENGTH] == NVM_PAYLOADLENGTH_INDICATING_TOKENREMOVED)
        {
            (*isDeleted) = true;
        }
    }
    return result;
}

static Bool Nvm_IsActiveToken(
    gpNvm_PoolId_t poolId,
    UInt8 tokenLength,
    UInt8* pToken
)
{
    Bool result = false;

    //buildup matching LUT
    gpNvm_Result_t             lutResult;
    gpNvm_LookupTable_Handle_t lutHandle;
    /* Note: will actually include removed tokens when nbrOfMatches == 1 and trailingWildcard == false */
    lutResult = gpNvm_BuildLookupMatch(
            &lutHandle,
            poolId,
            gpNvm_UpdateFrequencyIgnore,
            tokenLength,
            pToken,
            1,
            NULL,
            false,
            false
        );

    if (lutResult == gpNvm_Result_DataAvailable)
    {
        //find matchin attribute
        Nvm_Token_t*               pLutItem = NULL;
        Nvm_TokenLut_Iterator_t    lookUpIter;
        gpNvm_Result_t             lookUpResult;
        lookUpResult = Nvm_TokenLut_Query(&pLutItem,  NVM_LUTHANDLE_TO_LUT(lutHandle), tokenLength, pToken, true, &lookUpIter);

        if (lookUpResult == gpNvm_Result_DataAvailable)
        {
            //ensure latest available data on attribute is active and not marked as deleted
            Bool isDeleted;
            Nvm_Token_IsDeleted(poolId, pLutItem, &isDeleted);
            result = (!isDeleted);
        }
    }

    gpNvm_FreeLookup(lutHandle);
    return result;
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpNvm_SetBackgroundDefragmentationMode(Bool enable)
{
    if(enable)
    {
        GP_LOG_PRINTF("Background Defragmentation Enabled",0);
        /* Garbage collection enabled, decrease semaphore */

        /* Clip at underflow */
        if(Nvm_DefragmentationDisableCounter != 0)
        {
            Nvm_DefragmentationDisableCounter--;
            if(Nvm_DefragmentationDisableCounter==0)
            {
                gpNvm_PoolId_t poolId;

                GP_LOG_PRINTF("Background Defragmentation, all clear start",0);

                /* If enabled again, check, for Tag pool, if GC needs to be done */
                for (poolId = 0; poolId < Nvm_Flash_NbrOfPools; ++poolId)
                {
                    Nvm_Pool_TryToStartExtendedDefrag(poolId);
                }
            }
        }
        else
        {
            /* GP_ASSERT_EXT to detect underflow. */
            GP_ASSERT_DEV_EXT(false);
        }

    }
    else
    {
        GP_LOG_PRINTF("Background Defragmentation Disabled",0);

        /* Clip at overflow */
        if(Nvm_DefragmentationDisableCounter != 0xFF)
        {
            Nvm_DefragmentationDisableCounter++;
        }
        else
        {
            /* GP_ASSERT_EXT to detect overflow. */
            GP_ASSERT_DEV_EXT(false);
        }
    }
}

#ifdef GP_NVM_DIVERSITY_VARIABLE_SIZE
void gpNvm_SetVariableSize(UInt16 nrOfSectors, UInt8 nrOfPools, const UInt8 *sectorsPerPool)
{
    UIntLoop i;
    UInt32 totalPoolSectors;

    GP_ASSERT_SYSTEM(nrOfSectors <= NVM_MAX_NUMBER_SECTORS);
    GP_ASSERT_SYSTEM(nrOfSectors / GP_NVM_NBR_OF_REDUNDANT_SECTORS <= NVM_MAX_NUMBER_LOGICAL_SECTORS);
    GP_ASSERT_SYSTEM(nrOfPools > 0 && nrOfPools <= NVM_MAX_NUMBER_POOLS);

    totalPoolSectors = 0;
    for (i = 0; i < nrOfPools; i++)
    {
        totalPoolSectors += sectorsPerPool[i];
    }
    GP_ASSERT_SYSTEM(totalPoolSectors == nrOfSectors);

    Nvm_Flash_Size = (UInt32)nrOfSectors * NVM_NUMBER_PAGES_PER_SECTOR * FLASH_PAGE_SIZE;
    Nvm_Flash_NbrOfPhySectors = nrOfSectors;
    Nvm_Flash_NbrOfPools = nrOfPools;
    MEMCPY(Nvm_Pool_NbrOfSectors, sectorsPerPool, nrOfPools);
}
void gpNvm_GetVariableSize(UInt16* nrOfSectors, UInt8* nrOfPools, UInt8* sectorsPerPool)
{
    if(nrOfSectors)
    {
        *nrOfSectors = Nvm_Flash_NbrOfPhySectors;
    }
    if(nrOfPools)
    {
        *nrOfPools = Nvm_Flash_NbrOfPools;
    }
    if(sectorsPerPool)
    {
        MEMCPY(sectorsPerPool, Nvm_Pool_NbrOfSectors, Nvm_Flash_NbrOfPools);
    }

}
#endif //GP_NVM_DIVERSITY_VARIABLE_SIZE
#ifdef GP_NVM_DIVERSITY_VARIABLE_SETTINGS
void gpNvm_GetVariableSettings(UIntPtr* pNvmStart, gpNvm_KeyIndex_t* numberOfUniqueTokens, UInt8* maxTokenLength)
{
    if(pNvmStart)
    {
        *pNvmStart = Nvm_Start;
    }
    if(numberOfUniqueTokens)
    {
        *numberOfUniqueTokens = Nvm_NumberOfUniqueTokens;
    }
    //Fetchable for verification of internal setting
    if(maxTokenLength)
    {
        *maxTokenLength = GP_NVM_MAX_TOKENLENGTH;
    }
}

void gpNvm_SetVariableSettings(UIntPtr pNvmStart, gpNvm_KeyIndex_t numberOfUniqueTokens)
{
    Nvm_NumberOfUniqueTokens = numberOfUniqueTokens;
    Nvm_Start = pNvmStart;
}
#endif //GP_NVM_DIVERSITY_VARIABLE_SETTINGS

void Nvm_Init(void)
{
    GP_LOG_PRINTF("Nvm_Init->",0);

    GP_LOG_PRINTF("Nvm_Base %lx", 0, (unsigned long)Nvm_Base);

    Nvm_LastPhySector_Init();

    Nvm_TokenLuts_Init();

    Nvm_Reset();

    Nvm_DefragmentationDisableCounter = 0;
}
static void Nvm_Reset(void)
{
    Nvm_Pools_Process_ErasePending();

    Nvm_Pools_EraseCorrupted();

    Nvm_SectorLuts_Init();

    Nvm_SectorLuts_Load();

    Nvm_SectorLuts_LoadDetails();

    /* only use functions using LUT's after Nvm_TokenLuts_Rebuild() */
    Nvm_TokenLuts_Rebuild();

    Nvm_Pools_CalculateInactiveSums();

    GP_LOG_PRINTF("Nvm_Reset<-",0);
}

void Nvm_DeInit(void)
{
    UIntLoop i;
    for (i= 0; i< GP_NVM_NBR_OF_LOOKUPTABLE_HANDLES; i++)
    {
        gpNvm_FreeLookup(i);
        NVM_LUTHANDLE_SET_INVALID(i);
    }
}


void Nvm_Flush(void)
{
}


Bool Nvm_ReadTag(Nvm_TagCache_Id_t tagId, UInt16 maxSize, UInt8* data, gpNvm_UpdateFrequency_t updateFrequencySpec)
{
    NOT_USED(updateFrequencySpec);
    UInt8 DataLength = 0;
    gpNvm_Result_t result;
    Bool retval;
    Bool freeLutAfterUse;

    gpNvm_LookupTable_Handle_t lutHandle = 0;

    result = \
    Nvm_AcquireLutHandle(
        &lutHandle,
        gpNvm_PoolId_Tag,
        (UInt8) sizeof(Nvm_TagCache_Id_t),
        &tagId,
        &freeLutAfterUse,
        1,
        false,
        false
    );

    GP_ASSERT_SYSTEM(result == gpNvm_Result_DataAvailable || result == gpNvm_Result_NoDataAvailable);

    result = \
    Nvm_ReadUnique(
        NVM_LUTHANDLE_TO_LUT(lutHandle),
        gpNvm_PoolId_Tag,
        (UInt8) sizeof(Nvm_TagCache_Id_t),
        &tagId,
        maxSize,
        &DataLength,
        data
    );
    if (result == gpNvm_Result_DataAvailable
    || result == gpNvm_Result_Truncated /* backwards compatibility */)
    {
        retval = true;
    }
    else
    {
        MEMSET(data, 0xff, maxSize); /* backwards compat */
        retval = false;
    }

    if (freeLutAfterUse)
    {
        gpNvm_FreeLookup(lutHandle);
    }
    return retval;

}


static Bool Nvm_TokenLut_IsAvailable(
    Nvm_TokenLut_t *lut,
    gpNvm_PoolId_t poolId,
    UInt8 tokenMaskLength,
    UInt8 *pTokenMask
)
{
    if (!lut)
    {
        return false;
    }
    if (lut->poolId != poolId)
    {
        return false;
    }

    if (MEMCMP(lut->TokenMask, pTokenMask, min(tokenMaskLength, lut->tokenMaskLength)))
    {
        return false;
    }
    return true;
}

#ifdef GP_NVM_DIVERSITY_USE_POOLMEM
void Nvm_TokenLut_FreeData(Nvm_TokenBlock_t   *block)
{
    Nvm_TokenBlock_t   *nextBlock,*follow;
    GP_ASSERT_DEV_EXT(block);
    nextBlock = block;
    while (nextBlock)
    {
        follow = nextBlock->nextBlock;
        gpPoolMem_Free(nextBlock);
        nextBlock = follow;
    }
}
#endif // GP_NVM_DIVERSITY_USE_POOLMEM

static void Nvm_TokenLut_Free(Nvm_TokenLut_t *lut)
{
#ifdef GP_NVM_DIVERSITY_USE_POOLMEM
     if (lut)
     {
        if (lut->firstItemBlock)
        {
            Nvm_TokenLut_FreeData(lut->firstItemBlock);
        }
        gpPoolMem_Free(lut);
    }
#else
    if (lut == &Nvm_BssData.lut)
    {
        Nvm_BssData.inUse = false;
    }
#endif
}

void gpNvm_FreeLookup(gpNvm_LookupTable_Handle_t h)
{
    GP_ASSERT_SYSTEM(NVM_LUTHANDLE_IS_VALID(h));
    Nvm_TokenLut_Free(NVM_LUTHANDLE_TO_LUT(h));
    NVM_LUTHANDLE_SET_INVALID(h);
}
static Bool Nvm_TokenLutIter_Start(
    Nvm_TokenLut_t *lut,
    Nvm_TokenLut_Iterator_t *iter
)
{
    GP_ASSERT_DEV_INT(iter);
    GP_ASSERT_DEV_INT(lut);
    GP_ASSERT_DEV_INT(lut->firstItemBlock);
    iter->block = lut->firstItemBlock;
    iter->blockIndex = 0;
    return (iter->block!=NULL);
}

static Bool Nvm_TokenLutIter_IsAtEnd(
    Nvm_TokenLut_Iterator_t *iter
)
{
    if (iter->block->nrOfItemsUsed == 0)
    {
        return true;
    }
    if (iter->blockIndex < iter->block->nrOfItemsUsed)
    {
        return false;
    }
    if (Nvm_TokenLutIter_IsAtEmptySpot(iter))
    {
        return true;
    }
    if (iter->block->nextBlock && iter->block->nextBlock->nrOfItemsUsed)
    {
        return false;
    }
    return true;
}

/* @brief Get pointer to tokenlut item
 * @return pointer to a (potentially invalid struct or deleted) element
 */
static INLINE Nvm_Token_t *Nvm_TokenLutIter_GetItem(
    Nvm_TokenLut_Iterator_t *iter
)
{
    return &(iter->block->items[iter->blockIndex]);
}
/* @brief try to jump to next free entry in current or next block
 * @return true if a jump happened, false if at end of the chain
 */
static Bool Nvm_TokenLutIter_NextFree(
    Nvm_TokenLut_Iterator_t *iter
)
{
    GP_ASSERT_DEV_INT(iter);
    iter->blockIndex = iter->block->nrOfItemsUsed;

    if (iter->blockIndex < iter->block->nrOfItemsAllocated)
    {
        return true;
    }
    else if (iter->block->nextBlock)
    {
        iter->blockIndex = iter->block->nextBlock->nrOfItemsUsed;
        iter->block = iter->block->nextBlock;
        return true;
    }
    else
    {
        return false;
    }
}

static Bool Nvm_TokenLutIter_IsAtEmptySpot(
    Nvm_TokenLut_Iterator_t *iter
)
{
    GP_ASSERT_DEV_INT(iter->block->nrOfItemsAllocated >0);

    if ( (iter->blockIndex >= iter->block->nrOfItemsUsed)
     && (iter->blockIndex < iter->block->nrOfItemsAllocated))
    {
        return true;
    }
    return false;
}

static Bool Nvm_TokenLutIter_FirstFree(
    Nvm_TokenLut_t *lut,
    Nvm_TokenLut_Iterator_t *iter
)
{
    Bool iteratorInit;

    GP_ASSERT_DEV_INT(iter);
    Nvm_TokenLutIter_Start(lut, iter);
    iteratorInit = Nvm_TokenLutIter_Start(lut, iter);
    if(!iteratorInit)
    {
        return false;
    }

    while (Nvm_TokenLutIter_NextFree(iter))
    {
        if (Nvm_TokenLutIter_IsAtEmptySpot(iter))
        {
            return true;
        }
    }
    return false;
}

/* @brief iterate the used entries of the allocated blocks
 * @return false if the iterator is past the last used entry
 */
static Bool Nvm_TokenLutIter_Next(
    Nvm_TokenLut_Iterator_t *iter
)
{
    GP_ASSERT_DEV_INT(iter);
    iter->blockIndex++;
    if (iter->blockIndex < iter->block->nrOfItemsUsed)
    {
        /* not yet at end of used items */
        return true;
    }
    else if (iter->block->nrOfItemsUsed < iter->block->nrOfItemsAllocated)
    {
        /* empty blocks left means past end of continuous used region */
        return false;
    }
    else if (iter->block->nextBlock && iter->block->nextBlock->nrOfItemsUsed)
    {
        /* we have a next block to continue with */
        iter->blockIndex  = 0;
        iter->block = iter->block->nextBlock;
        return true;
    } else {
        return false;
    }
}

/* @brief write the itemblock entry the iter is pointing to
 * @return pointer to the item, NULL if the writer is not pointing to an empty spot
 */
static Nvm_Token_t* Nvm_TokenLut_Append(
    Nvm_TokenLut_Iterator_t *iter,
    UInt8 tokenLength,
    const UInt8*pToken
)
{
    Nvm_Token_t *item;

    GP_ASSERT_DEV_EXT(iter);
    GP_ASSERT_DEV_EXT(iter->block);
    GP_ASSERT_DEV_EXT(pToken);

    if (!Nvm_TokenLutIter_IsAtEmptySpot(iter))
    {
        GP_ASSERT_DEV_INT(false);
        return NULL;
    }

    item = Nvm_TokenLutIter_GetItem(iter);

    item->nbrOfSectorReferences = 0;
    item->prevLogSecId = NVM_SECTORID_INVALID;
    iter->block->nrOfItemsUsed++;

    /* ignore failure when we filled the last entry */
    (void)Nvm_TokenLutIter_NextFree(iter);

    return item;
}

static gpNvm_Result_t Nvm_TokenLut_Query(
    Nvm_Token_t** itemResult,
    Nvm_TokenLut_t *lut,
    UInt8 tokenMaskLength,
    const UInt8 *pTokenMask,
    Bool strictMatching,
    Nvm_TokenLut_Iterator_t *iter
)
{
    Nvm_Token_t *lutItem;
    gpNvm_Result_t result = gpNvm_Result_NoDataAvailable;

    GP_ASSERT_DEV_EXT(pTokenMask);

    GP_ASSERT_DEV_EXT(lut);

    Bool IteratorInit = Nvm_TokenLutIter_Start(lut, iter);
    GP_ASSERT_DEV_EXT(IteratorInit);
    if (!IteratorInit)
    {
        return gpNvm_Result_Error;
    }
    if (Nvm_TokenLutIter_IsAtEnd(iter))
    {
        return gpNvm_Result_NoDataAvailable;
    }

    do
    {
        UInt8 tokenContent[GP_NVM_MAX_TOKENLENGTH];
        UInt8 iterTokenLength;
        UInt8 ItemHeader[NVM_RECORD_HEADER_SIZE];

        lutItem = Nvm_TokenLutIter_GetItem(iter);

        gpNvm_Result_t getrecord_result = Nvm_Token_GetRecordHeader(lut->poolId, lutItem, ItemHeader);
        if(getrecord_result != gpNvm_Result_DataAvailable)
        {
            GP_ASSERT_SYSTEM(false);
            return getrecord_result;
        }

        iterTokenLength = ItemHeader[NVM_RECORD_HEADER_OFFSET_TO_TOKENLENGTH];

        Nvm_Token_GetRecordToken(lut->poolId,
                                 lutItem,
                                 iterTokenLength,
                                 tokenContent);

        if((iterTokenLength >= tokenMaskLength) && (0 == MEMCMP(tokenContent, pTokenMask, tokenMaskLength)))
        {
            result = gpNvm_Result_NoUniqueMaskMatch;
            if((!strictMatching) || (strictMatching && iterTokenLength == tokenMaskLength))
            {
                (*itemResult) = lutItem;
                return gpNvm_Result_DataAvailable;
            }
        }
    }
    while (Nvm_TokenLutIter_Next(iter));

    return result;
}

static void Nvm_Token_Update(Nvm_Token_t* lutItem, UInt16 offsetInSector, Nvm_LogSectorId_t logSecId)
{
    GP_ASSERT_DEV_EXT(lutItem);
    GP_ASSERT_DEV_INT(SECTOROFFSET_IS_PACKABLE(offsetInSector));
    lutItem->packed_sectorOffset = SECTOROFFSET_PACK(offsetInSector);
    lutItem->logSecId = logSecId;
}

#ifdef GP_NVM_DIVERSITY_USE_POOLMEM
static Bool Nvm_TokenLut_AllocateData(
    Nvm_TokenLut_t *lut,
    gpNvm_KeyIndex_t maxNrMatches
)
{
    Nvm_TokenBlock_t   *newBlock;

    GP_ASSERT_DEV_INT(lut);
    lut->firstItemBlock = NULL;

#ifdef GP_POOLMEM_DIVERSITY_MALLOC
    newBlock = GP_POOLMEM_MALLOC(sizeof(Nvm_TokenBlock_t) + (maxNrMatches*sizeof(Nvm_Token_t)));
    if (!newBlock)
    {
         return false;
    }
    newBlock->nrOfItemsAllocated = maxNrMatches;
    newBlock->nrOfItemsUsed = 0;
    newBlock->nextBlock = NULL;

    lut->firstItemBlock = newBlock;
#else
    gpNvm_KeyIndex_t nrOfItemsToAllocate;
    Nvm_TokenBlock_t   *iterBlock = NULL;
    UInt8 newBlockNrOfItems;

    for (nrOfItemsToAllocate = maxNrMatches; nrOfItemsToAllocate > 0; nrOfItemsToAllocate -= newBlockNrOfItems)
    {
        UInt16 maxBlockSize;
        UInt16 maxNrOfItemsInBlock;
        UInt16 newBlockSize;

        maxBlockSize = gpPoolMem_GetMaxAvailableChunkSize();
        GP_ASSERT_SYSTEM( maxBlockSize >= (sizeof(Nvm_TokenBlock_t  ) + sizeof(Nvm_Token_t)));

        maxNrOfItemsInBlock = (maxBlockSize - sizeof(Nvm_TokenBlock_t  )) / sizeof(Nvm_Token_t);

        newBlockNrOfItems = min(maxNrOfItemsInBlock,nrOfItemsToAllocate);
        newBlockSize = sizeof(Nvm_TokenBlock_t  ) + (newBlockNrOfItems *sizeof(Nvm_Token_t)) ;
        GP_ASSERT_SYSTEM(newBlockSize <= maxBlockSize);

        newBlock = GP_POOLMEM_MALLOC(newBlockSize);
        if (!newBlock)
        {
             return false;
        }

        newBlock->nrOfItemsAllocated = newBlockNrOfItems;
        newBlock->nrOfItemsUsed = 0;
        newBlock->nextBlock = NULL;

        if (lut->firstItemBlock == NULL)
        {
            lut->firstItemBlock = newBlock;
        }
        else
        {
            GP_ASSERT_SYSTEM(iterBlock);
            iterBlock->nextBlock = newBlock;
        }
        iterBlock = newBlock;
        if (newBlockNrOfItems > nrOfItemsToAllocate)
        {
            /* if we can only allocate fixed size chunks, break the loop before
             * the decrement */
            break;
        }
    }
#endif //GP_POOLMEM_DIVERSITY_MALLOC
    return true;
}
#else
static Bool Nvm_TokenLut_AllocateData(
    Nvm_TokenLut_t *lut,
    gpNvm_KeyIndex_t maxNrMatches
)
{
    GP_ASSERT_SYSTEM(maxNrMatches <= Nvm_NumberOfUniqueTokens);
    lut->firstItemBlock = &Nvm_BssData.itemBlock;
    Nvm_BssData.itemBlock.nrOfItemsAllocated = Nvm_NumberOfUniqueTokens;
    Nvm_BssData.itemBlock.nrOfItemsUsed = 0;
    Nvm_BssData.itemBlock.nextBlock = NULL;
    return true;
}
#endif// GP_NVM_DIVERSITY_USE_POOLMEM

static Bool Nvm_TokenLut_Allocate(
    Nvm_TokenLut_t **lut,
    gpNvm_KeyIndex_t maxNrMatches
)
{
    Nvm_TokenLut_t *newLut;

    GP_ASSERT_SYSTEM(maxNrMatches > 0);

    /* At startup, the Nvm_NumberOfUniqueTokens is allocated, this gives us
     * a feeling that -at least at start up-, this allocation will succeed.
     * To trigger this failure at startup, refuse larger requests at runtime.
     */
// avoid compiler warning in case GP_NVM_NBR_OF_UNIQUE_TOKENS is the maximum value of
// gpNvm_KeyIndex_t. (i.e. 255 when type(gpNvm_KeyIndex_t) == UInt8 and 2^32-1 when
// type(gpNvm_KeyIndex_t) == UInt32)
#ifndef GP_NVM_DIVERSITY_VARIABLE_SETTINGS
#if GP_NVM_NBR_OF_UNIQUE_TOKENS != UINT8_MAX && GP_NVM_NBR_OF_UNIQUE_TOKENS != UINT32_MAX
    //Check validity
    GP_ASSERT_SYSTEM(maxNrMatches <= Nvm_NumberOfUniqueTokens);
#endif
#endif

#ifdef GP_NVM_DIVERSITY_USE_POOLMEM
    newLut = GP_POOLMEM_MALLOC(sizeof(Nvm_TokenLut_t));
#else
    GP_ASSERT_DEV_INT(Nvm_BssData.inUse == false);
    Nvm_BssData.inUse = true;
    newLut = &Nvm_BssData.lut;
#endif

    if (!newLut)
    {
        return false;
    }

    Nvm_TokenLut_Init(newLut);

    if (!Nvm_TokenLut_AllocateData(newLut, maxNrMatches))
    {
         Nvm_TokenLut_Free(newLut);
         return false;
    }

    (*lut) = newLut;

    newLut->nrOfItemsAllocated = maxNrMatches;

    return true;
}




static Bool Nvm_TokenLut_TokenBelongs(
    Nvm_TokenLut_t *lut,
    gpNvm_PoolId_t poolId,
    UInt8 tokenLength,
    UInt8 *pToken
)
{
    if (!lut)
    {
        return false;
    }
    if (lut->poolId != poolId)
    {
        return false;
    }

    if (lut->tokenMaskLength == 0)
    {
        return true; /* accept any token */
    }

    if (tokenLength < lut->tokenMaskLength)
    {
        return false;
    }

    if (0 != MEMCMP(pToken, lut->TokenMask, lut->tokenMaskLength))
    {
        return false;
    }
    if (false == lut->trailingWildcard && tokenLength > lut->tokenMaskLength)
    {
        return false;
    }
    return true;
}



static gpNvm_Result_t Nvm_TokenLut_Populate(
    Nvm_TokenLut_t *lut,
    gpNvm_PoolId_t poolIdSpec,
    UInt8 tokenMaskLengthSpec,
    UInt8* pTokenMask,
    gpNvm_KeyIndex_t maxNrMatches,
    gpNvm_KeyIndex_t *pNrOfMatches,
    Bool trailingWildcard,
    Bool includeInactive
)
{
    Nvm_TokenLut_Iterator_t writeIter;
    gpNvm_KeyIndex_t nrOfMatches = 0;
    Nvm_Pool_Iterator_t poolIter;
    Bool poolIteratorInit;

    GP_ASSERT_SYSTEM(lut);

    if (pNrOfMatches)
    {
        (*pNrOfMatches) = nrOfMatches;
    }

    GP_ASSERT_DEV_EXT(tokenMaskLengthSpec <= GP_NVM_MAX_TOKENLENGTH);

    lut->poolId = poolIdSpec;
    lut->tokenMaskLength = tokenMaskLengthSpec;
    lut->trailingWildcard = trailingWildcard;
    lut->includeInactive = includeInactive;
    lut->writeLock = false;

    if (lut->TokenMask != pTokenMask)
    {
        MEMCPY(lut->TokenMask, pTokenMask, tokenMaskLengthSpec);
    }


    Bool IteratorStatus = Nvm_TokenLutIter_Start(lut, &writeIter);
    GP_ASSERT_DEV_EXT(IteratorStatus);

    if(tokenMaskLengthSpec == 0 && includeInactive)
    {
        Nvm_Pool_NumberOfUniqueTokens[poolIdSpec] = 0;
    }

    lut->writeLock = false;
    for(poolIteratorInit = Nvm_Pool_Iterator_Start(poolIdSpec, &poolIter);
        poolIteratorInit && Nvm_Pool_Iterator_IsActive(&poolIter);
        Nvm_Pool_Iterator_Next(&poolIter))
    {
        const UInt8 tokenLength = poolIter.RecordData[NVM_RECORD_HEADER_OFFSET_TO_TOKENLENGTH];
        Nvm_MaskedSeqNum_t seqNum = Nvm_Buf_Get_UInt16(poolIter.RecordData, NVM_RECORD_HEADER_OFFSET_TO_SEQNUM_COUNTER);
        ;
        UInt8* token = &poolIter.RecordData[NVM_RECORD_HEADER_OFFSET_TO_TOKEN];

        if(!Nvm_TokenLut_TokenBelongs(lut, poolIdSpec, tokenLength, token))
        {
            continue;
        }

        Nvm_Token_t* lutItem = NULL;
        gpNvm_Result_t lookUpResult;
        Nvm_TokenLut_Iterator_t lookupIter;
        Nvm_MaskedSeqNum_t lutItemseqNum = NVM_SEQNUM_INVALID;

        lookUpResult = Nvm_TokenLut_Query(&lutItem, lut, tokenLength, token, true, &lookupIter);

        if(lookUpResult == gpNvm_Result_DataAvailable)
        {
            UInt8 ItemHeader[NVM_RECORD_HEADER_SIZE];
            gpNvm_Result_t get_record_header_result = Nvm_Token_GetRecordHeader(lut->poolId, lutItem, ItemHeader);
            GP_ASSERT_SYSTEM(get_record_header_result == gpNvm_Result_DataAvailable);
            lutItemseqNum = Nvm_Buf_Get_UInt16(ItemHeader, NVM_RECORD_HEADER_OFFSET_TO_SEQNUM_COUNTER);
        }
        else
        {
            if(tokenMaskLengthSpec == 0 && includeInactive)
            {
                Nvm_Pool_NumberOfUniqueTokens[poolIdSpec]++;
            }

            if(
                ((maxNrMatches == 1) && (trailingWildcard == false)) || //search for a unique token
                (includeInactive == true) ||                            // this is for Nvm_IsActiveToken and diagnostics...
                Nvm_IsActiveToken(poolIdSpec, tokenLength, token))
            {
                nrOfMatches++;
                if(nrOfMatches > maxNrMatches)
                {
                    /* continue walking the flash to get the latest updates of the tags */
                    /* TO CHECK: there should be code coverage here! */
                    lutItem = NULL;
                }
                else
                {
                    lutItem = Nvm_TokenLut_Append(&writeIter, tokenLength, token);
                    GP_ASSERT_DEV_EXT(lutItem);
                }
            }
        }

        if(lutItem)
        {
            if(lutItem->prevLogSecId != poolIter.logSec)
            {
                lutItem->nbrOfSectorReferences++;
                lutItem->prevLogSecId = poolIter.logSec;
            }
        }
        if(lutItem && (
                          /* newly added to LUT */
                          (lutItemseqNum == NVM_SEQNUM_INVALID)
                          /* more recent record found than the one in the LUT */
                          || (Nvm_Pool_SeqNum_Unmask(poolIdSpec, lutItemseqNum) < Nvm_Pool_SeqNum_Unmask(poolIdSpec, seqNum))))
        {
            Nvm_Token_Update(lutItem, poolIter.offsetInSector, poolIter.logSec);
        }
    }

    lut->writeLock = false;

    if(!poolIteratorInit)
    {
        if(pNrOfMatches)
        {
            (*pNrOfMatches) = nrOfMatches;
        }
        return gpNvm_Result_DataAvailable;
    }
    if (nrOfMatches > maxNrMatches)
    {
        if (pNrOfMatches)
        {
            (*pNrOfMatches) = maxNrMatches;
        }
        return gpNvm_Result_Truncated;
    }
    else
    {
        if (pNrOfMatches)
        {
            (*pNrOfMatches) = nrOfMatches;
        }
        return gpNvm_Result_DataAvailable;
    }
}

static gpNvm_Result_t Nvm_TokenLut_AllocateHandle(
    gpNvm_LookupTable_Handle_t* h
)
{
    UIntLoop i;
    for (i=0; i < GP_NVM_NBR_OF_LOOKUPTABLE_HANDLES; i++)
    {
        if (!NVM_LUTHANDLE_IS_ACTIVE(i))
        {
            (*h) = i;
            return gpNvm_Result_DataAvailable;
        }
    }
    return gpNvm_Result_Error;
}

/* Build the user end lookuptable */
gpNvm_Result_t gpNvm_BuildLookup(
    gpNvm_LookupTable_Handle_t* pHandle,
    gpNvm_PoolId_t poolIdSpec,
    gpNvm_UpdateFrequency_t updateFrequencySpec,
    UInt8 tokenMaskLengthSpec,
    UInt8* pTokenMask,
    gpNvm_KeyIndex_t maxNrMatches,
    gpNvm_KeyIndex_t *pNrOfMatches
)
{
    return gpNvm_BuildLookupMatch(
        pHandle,
        poolIdSpec,
        updateFrequencySpec,
        tokenMaskLengthSpec,
        pTokenMask,
        maxNrMatches,
        pNrOfMatches,
        true,
        false
    );
}


gpNvm_Result_t gpNvm_BuildLookupMatch(
    gpNvm_LookupTable_Handle_t* pHandle,
    gpNvm_PoolId_t poolIdSpec,
    gpNvm_UpdateFrequency_t updateFrequencySpec,
    UInt8 tokenMaskLengthSpec,
    UInt8* pTokenMask,
    gpNvm_KeyIndex_t maxNrMatches,
    gpNvm_KeyIndex_t *pNrOfMatches,
    Bool trailingWildcard,
    Bool includeInactive
)
{
    Nvm_TokenLut_t *lut;
    gpNvm_KeyIndex_t nrOfMatches = 0;
    gpNvm_Result_t result;
    GP_ASSERT_SYSTEM(pHandle);

    if (pNrOfMatches)
    {
        (*pNrOfMatches) = nrOfMatches;
    }

    result = Nvm_TokenLut_AllocateHandle(pHandle);

    if (result != gpNvm_Result_DataAvailable)
    {
        return result;
    }

    if (!Nvm_TokenLut_Allocate(&lut, maxNrMatches))
    {
        GP_ASSERT_DEV_INT(false);
        return gpNvm_Result_Error;
    }
    NVM_LUTHANDLE_SET(*pHandle, lut);
    result = Nvm_TokenLut_Populate(
        lut,
        poolIdSpec,
        tokenMaskLengthSpec,
        pTokenMask,
        maxNrMatches,
        pNrOfMatches,
        trailingWildcard,
        includeInactive
    );

    return result;
}

gpNvm_Result_t gpNvm_BuildLookupOverAllUpdateFrequencies(
    gpNvm_LookupTable_Handle_t* handle,
    gpNvm_PoolId_t poolIdSpec,
    UInt8 tokenMaskLengthSpec,
    UInt8* pTokenMask,
    gpNvm_KeyIndex_t maxNrMatches,
    gpNvm_KeyIndex_t* pNrOfMatches
)
{
    return gpNvm_BuildLookup(
        handle,
        poolIdSpec,
        gpNvm_UpdateFrequencyIgnore,
        tokenMaskLengthSpec,
        pTokenMask,
        maxNrMatches,
        pNrOfMatches
    );
}

static gpNvm_Result_t Nvm_Token_GetRecordHeader(
    gpNvm_PoolId_t poolId,
    Nvm_Token_t* item,
    UInt8 header[NVM_RECORD_HEADER_SIZE]
)
{
    gpHal_FlashError_t state;

    GP_ASSERT_SYSTEM(item->logSecId != NVM_SECTORID_INVALID);
    UInt32 offset = (Nvm_LogSector_GetPhyFlashOffset(poolId, item->logSecId) + SECTOROFFSET_UNPACK(item->packed_sectorOffset));

    state = Nvm_Flash_Read(offset , NVM_RECORD_HEADER_SIZE, (UInt8*)header);

    if (state != gpHal_FlashError_Success)
    {
        return gpNvm_Result_Error;
    }
    return gpNvm_Result_DataAvailable;
}

static gpNvm_Result_t Nvm_Token_GetRecordPayload(
    gpNvm_PoolId_t poolId,
    Nvm_Token_t* item,
    UInt8 PayLoadSize,
    UInt8 tokenLength,
    UInt8* pData
)
{
    UInt32 offsetToTag = (Nvm_LogSector_GetPhyFlashOffset(poolId, item->logSecId) + SECTOROFFSET_UNPACK(item->packed_sectorOffset));
    UInt8 offsetToPayload = NVM_RECORD_HEADER_OFFSET_TO_PAYLOAD(tokenLength);
#if (GP_NVM_MAX_PAYLOADLENGTH < 255)
    GP_ASSERT_DEV_INT(PayLoadSize <= GP_NVM_MAX_PAYLOADLENGTH);
#endif
    UInt8 state = Nvm_Flash_Read(offsetToTag + offsetToPayload, PayLoadSize, pData);

    if (state != gpHal_FlashError_Success)
    {
        return gpNvm_Result_Error;
    }
    return gpNvm_Result_DataAvailable;
}

static gpNvm_Result_t Nvm_Token_GetRecordToken(
    gpNvm_PoolId_t poolId,
    Nvm_Token_t* item,
    UInt8 TokenSize,
    UInt8* pData
)
{
    UInt32 offsetToTag = (Nvm_LogSector_GetPhyFlashOffset(poolId, item->logSecId) + SECTOROFFSET_UNPACK(item->packed_sectorOffset));

    UInt8 state = Nvm_Flash_Read(offsetToTag + NVM_RECORD_HEADER_OFFSET_TO_TOKEN, TokenSize, pData);

    if (state != gpHal_FlashError_Success)
    {
        return gpNvm_Result_Error;
    }
    return gpNvm_Result_DataAvailable;
}

static gpNvm_Result_t Nvm_Token_GetRecord(
    gpNvm_PoolId_t poolId,
    Nvm_Token_t *item,
    UInt8 maxTokenLength,
    UInt8 *pToken,
    UInt8 *pTokenLength,
    UInt8 maxDataLength,
    UInt8* pDataLength,
    UInt8* pData
)
{
    gpNvm_Result_t result;
    UInt8 ItemHeader[NVM_RECORD_HEADER_SIZE];
    UInt8 ReturnedTokenLength;
    UInt8 ReturnedPayLoadLength;

    if (!item)
    {
        return gpNvm_Result_Error;
    }
    result = Nvm_Token_GetRecordHeader(poolId, item, ItemHeader);

    if (result!=gpNvm_Result_DataAvailable)
    {
        return result;
    }
#if (GP_NVM_MAX_PAYLOADLENGTH < 255)
    GP_ASSERT_DEV_INT(ItemHeader[NVM_RECORD_HEADER_OFFSET_TO_PAYLOADLENGTH] <= GP_NVM_MAX_PAYLOADLENGTH);
#endif
    ReturnedTokenLength = min(ItemHeader[NVM_RECORD_HEADER_OFFSET_TO_TOKENLENGTH],maxTokenLength);
    ReturnedPayLoadLength = min(ItemHeader[NVM_RECORD_HEADER_OFFSET_TO_PAYLOADLENGTH],maxDataLength);

    if (pTokenLength)
    {
        (*pTokenLength) = ReturnedTokenLength;
    }

    if (pDataLength)
    {
        (*pDataLength) = ReturnedPayLoadLength;
    }

    const UInt8 tokenLength = ItemHeader[NVM_RECORD_HEADER_OFFSET_TO_TOKENLENGTH];
    if (pToken)
    {

        result = Nvm_Token_GetRecordToken(poolId, item, ReturnedTokenLength, pToken);
        if (result != gpNvm_Result_DataAvailable)
        {
            return gpNvm_Result_Error;
        }
    }

    if (ItemHeader[NVM_RECORD_HEADER_OFFSET_TO_PAYLOADLENGTH] == NVM_PAYLOADLENGTH_INDICATING_TOKENREMOVED)
    {
        return gpNvm_Result_NoDataAvailable;
    }

    if (pToken)
    {
        if (tokenLength > maxTokenLength)
        {
            return gpNvm_Result_Error;
        }
    }
#if (GP_NVM_MAX_PAYLOADLENGTH < 255)
    GP_ASSERT_DEV_INT(ItemHeader[NVM_RECORD_HEADER_OFFSET_TO_PAYLOADLENGTH] <= GP_NVM_MAX_PAYLOADLENGTH);
#endif
    if (pData)
    {
        result = Nvm_Token_GetRecordPayload(poolId, item, ReturnedPayLoadLength, ItemHeader[NVM_RECORD_HEADER_OFFSET_TO_TOKENLENGTH], pData);
    }
    else
    {
        result = gpNvm_Result_DataAvailable;
    }

    if (result != gpNvm_Result_DataAvailable )
    {
        return result;
    }

    if (pData && (ReturnedPayLoadLength < ItemHeader[NVM_RECORD_HEADER_OFFSET_TO_PAYLOADLENGTH]))
    {
        return gpNvm_Result_Truncated;
    }

    return gpNvm_Result_DataAvailable;
}

gpNvm_Result_t gpNvm_ReadUnique(
    gpNvm_LookupTable_Handle_t handle,
    gpNvm_PoolId_t poolId,
    gpNvm_UpdateFrequency_t updateFrequencySpec,
    gpNvm_UpdateFrequency_t *pUpdateFrequency,
    UInt8 tokenMaskLength,
    UInt8* pTokenMask,
    UInt8 maxDataLength,
    UInt8* pDataLength,
    UInt8* pData
)
{
    Nvm_TokenLut_t *lut;
    GP_ASSERT_SYSTEM(handle < GP_NVM_NBR_OF_LOOKUPTABLE_HANDLES);
    if (pUpdateFrequency)
    {
        (*pUpdateFrequency) = updateFrequencySpec;
    }
    if (!NVM_LUTHANDLE_IS_ACTIVE(handle))
    {
        return gpNvm_Result_NoLookUpTable;
    }
    lut = NVM_LUTHANDLE_TO_LUT(handle);

    if (!pDataLength)
    {
        GP_ASSERT_DEV_EXT(pDataLength);
        return gpNvm_Result_Error;
    }

    return Nvm_ReadUnique(
        lut,
        poolId,
        tokenMaskLength,
        pTokenMask,
        maxDataLength,
        pDataLength,
        pData
    );
}

static gpNvm_Result_t Nvm_ReadUnique(
    Nvm_TokenLut_t *lut,
    gpNvm_PoolId_t poolId,
    UInt8 tokenMaskLength,
    UInt8* pTokenMask,
    UInt8 maxDataLength,
    UInt8* pDataLength,
    UInt8* pData
)
{
    if (poolId >= gpNvm_PoolId_AllPoolIds)
    {
        GP_ASSERT_DEV_INT(false);
        return gpNvm_Result_Error;
    }

    if (tokenMaskLength == 0 || tokenMaskLength > GP_NVM_MAX_TOKENLENGTH)
    {
        return gpNvm_Result_Error;
    }
    Nvm_Token_t *lutItem = NULL;
    Nvm_TokenLut_Iterator_t lookupIter;
    gpNvm_Result_t lookUpResult;

    if (!Nvm_TokenLut_IsAvailable(lut, poolId, tokenMaskLength, pTokenMask))
    {
        return gpNvm_Result_NoLookUpTable;
    }

    lookUpResult = Nvm_TokenLut_Query(&lutItem, lut, tokenMaskLength, pTokenMask, true, &lookupIter);

    if (lookUpResult == gpNvm_Result_DataAvailable)
    {
        return Nvm_Token_GetRecord(poolId, lutItem, 0, NULL, NULL,  maxDataLength, pDataLength, pData);
    }
    else
    {
        return lookUpResult;
    }
}

gpNvm_Result_t gpNvm_ReadNext(
    gpNvm_LookupTable_Handle_t handle,
    gpNvm_PoolId_t poolId,
    gpNvm_UpdateFrequency_t* pUpdateFrequency,
    UInt8 maxTokenLength,
    UInt8* pTokenLength,
    UInt8* pToken,
    UInt8 maxDataLength,
    UInt8* pDataLength,
    UInt8* pData
)
{
    gpNvm_Result_t result;
    Nvm_Token_t *lutItem;
    Nvm_TokenLut_t *lut;

    GP_ASSERT_SYSTEM(handle < GP_NVM_NBR_OF_LOOKUPTABLE_HANDLES);
    GP_ASSERT_DEV_EXT(pDataLength);
    if (pUpdateFrequency)
    {
        (*pUpdateFrequency) = gpNvm_UpdateFrequencyHigh;
    }
    if (!NVM_LUTHANDLE_IS_ACTIVE(handle))
    {
        return gpNvm_Result_NoLookUpTable;
    }
    lut = NVM_LUTHANDLE_TO_LUT(handle);

    if (lut->poolId != poolId)
    {
        return gpNvm_Result_Error;
    }

    if (!lut->readNext_Iterator.block)
    {
        Bool IteratorInit = Nvm_TokenLutIter_Start(lut, &lut->readNext_Iterator);
        GP_ASSERT_DEV_EXT(IteratorInit);
        if (!IteratorInit)
        {
            return gpNvm_Result_Error;
        }
    }

    do
    {
        /* loop to skip items that are removed: in those cases Nvm_Token_GetRecord
         * returns gpNvm_Result_NoDataAvailable */

        if (Nvm_TokenLutIter_IsAtEnd(&lut->readNext_Iterator))
        {
            return gpNvm_Result_NoDataAvailable;
        }

        lutItem = Nvm_TokenLutIter_GetItem(&lut->readNext_Iterator);

        result = Nvm_Token_GetRecord(poolId, lutItem, maxTokenLength, pToken, pTokenLength,  maxDataLength, pDataLength, pData);

        (void) Nvm_TokenLutIter_Next(&lut->readNext_Iterator);
    } while (result == gpNvm_Result_NoDataAvailable);

    return result;
}

gpNvm_Result_t gpNvm_ResetIterator(gpNvm_LookupTable_Handle_t handle)
{
    Nvm_TokenLut_t *lut;

    GP_ASSERT_SYSTEM(handle < GP_NVM_NBR_OF_LOOKUPTABLE_HANDLES);
    if (!NVM_LUTHANDLE_IS_ACTIVE(handle))
    {
        return gpNvm_Result_NoLookUpTable;
    }
    lut = NVM_LUTHANDLE_TO_LUT(handle);

    Bool IteratorInit = Nvm_TokenLutIter_Start(lut, &lut->readNext_Iterator);
    GP_ASSERT_DEV_EXT(IteratorInit);
    if (!IteratorInit)
    {
        return gpNvm_Result_Error;
    }
    return gpNvm_Result_DataAvailable;
}


gpNvm_Result_t gpNvm_ErasePool(gpNvm_PoolId_t poolId)
{
    GP_LOG_PRINTF("gpNvm_ErasePool poolId:%d", 0, poolId);

    if ((poolId >= GP_NVM_NBR_OF_POOLS) && (poolId != gpNvm_PoolId_AllPoolIds))
    {
        return gpNvm_Result_Error;
    }
    Nvm_Pool_PhysicalErase(poolId);
    Nvm_Reset();

    return gpNvm_Result_DataAvailable;
}
/** @brief Physical erase the NVM pool.
 *  Warning : This function should be used only inside Nvm_SafetyNetHandler
 *
 *  @param poolId    Pool Id to be erased
 */
gpNvm_Result_t gpNvm_PhysicalErasePool(gpNvm_PoolId_t poolId)
{
    GP_LOG_PRINTF("gpNvm_PhysicalErasePool poolId:%d", 0, poolId);

    if ((poolId >= GP_NVM_NBR_OF_POOLS) && (poolId != gpNvm_PoolId_AllPoolIds))
    {
        return gpNvm_Result_Error;
    }
    Nvm_Pool_PhysicalErase(poolId);

    return gpNvm_Result_DataAvailable;
}

Bool Nvm_WriteTag(Nvm_TagCache_Id_t tagId, UInt16 lengthOfNewData, UInt8* data, gpNvm_UpdateFrequency_t updateFrequency)
{
    gpNvm_Result_t r;
    NOT_USED(updateFrequency);

    r = Nvm_Write(
          gpNvm_PoolId_Tag,
          1,
          &tagId,
          lengthOfNewData,
          data
    );

    if (r == gpNvm_Result_DataAvailable)
    {
        return true;
    }
    else
    {
        return false;
    }
}

gpNvm_Result_t gpNvm_Write(
                            gpNvm_PoolId_t poolId,
                            gpNvm_UpdateFrequency_t updateFrequency,
                            UInt8 tokenLength,
                            UInt8* pToken,
                            UInt8 dataLength,
                            UInt8* pData
                        )
{
    gpNvm_Result_t r;

    if (dataLength == NVM_PAYLOADLENGTH_INDICATING_TOKENREMOVED || pData == NULL)
    {
        return gpNvm_Result_Error;
    }

    r = \
    Nvm_Write(
        poolId,
        tokenLength,
        pToken,
        dataLength,
        pData
    );

    return r;
}

static gpNvm_Result_t Nvm_TokenLut_GetSizeAndLocation(
    gpNvm_PoolId_t poolId,
    gpNvm_LookupTable_Handle_t lutHandle,
    UInt8 tokenLength,
    UInt8* pToken,
    UInt8 *pPayLoadLength,
    UInt8 *pLogSecId,
    Nvm_Token_t **pRetLutItem
)
{
    gpNvm_Result_t result;
    Nvm_Token_t *lutItem;
    Nvm_TokenLut_Iterator_t lookupIter;
    UInt8 header[NVM_RECORD_HEADER_SIZE];
    GP_ASSERT_DEV_EXT(pPayLoadLength);
    GP_ASSERT_DEV_EXT(pLogSecId);

    result = Nvm_TokenLut_Query(
        &lutItem,
        NVM_LUTHANDLE_TO_LUT(lutHandle),
        tokenLength,
        pToken,
        true,
        &lookupIter
    );
    if (result != gpNvm_Result_DataAvailable)
    {
        return result;
    }

    result = \
    Nvm_Token_GetRecordHeader(
        poolId,
        lutItem,
        header
    );
    GP_ASSERT_DEV_EXT(result == gpNvm_Result_DataAvailable);
    const UInt8 payloadLength = header[NVM_RECORD_HEADER_OFFSET_TO_PAYLOADLENGTH];

    (*pPayLoadLength) = payloadLength;
    (*pLogSecId) = lutItem->logSecId;
    (*pRetLutItem) = lutItem;
    return gpNvm_Result_DataAvailable;
}

static gpNvm_Result_t Nvm_Write(
                            gpNvm_PoolId_t poolId,
                            UInt8 tokenLength,
                            UInt8* pToken,
                            UInt8 dataLength,
                            UInt8* pData
                        )
{
    Nvm_Token_PreviousVersion_t prevVersion = {gpNvm_Result_Error};
    gpNvm_Result_t result = gpNvm_Result_DataAvailable;
    gpNvm_Result_t acqLutResult;
    Bool freeLutAfterUse;
    gpNvm_LookupTable_Handle_t lutHandle;
    Nvm_Token_t *lutItem = NULL;
    Bool lowOnSpace = false;

    if(Nvm_Pool_IsUrgentDefragRequired(poolId))
    {
        GP_LOG_PRINTF("Perform urgent defrag", 0);
        UInt8 nbrOfDefragCandidates;
        do
        {
            nbrOfDefragCandidates = Nvm_Pool_ReclaimOutdatedSpace(poolId);
        }
        while (nbrOfDefragCandidates > 0);

        GP_LOG_PRINTF("Defrag done",0);
        if (Nvm_Pool_IsUrgentDefragRequired(poolId))
        {
            GP_LOG_PRINTF("Set low on space flag",0);
            /* only one completely empty sector, need to do additional checks */
            lowOnSpace = true;
        }
    }

    /* lookup previous version
     * - for inactive record tracking
     * - to skip write if the value hasn't changed
     */
    acqLutResult =
        Nvm_AcquireLutHandle(
            &lutHandle,
            poolId,
            tokenLength,
            pToken,
            &freeLutAfterUse,
            1,
            false,
            false);
    GP_ASSERT_DEV_INT(acqLutResult == gpNvm_Result_DataAvailable || acqLutResult == gpNvm_Result_NoDataAvailable);
    if (acqLutResult == gpNvm_Result_DataAvailable)
    {
        prevVersion.lookUpResult = \
            Nvm_TokenLut_GetSizeAndLocation(
            poolId,
            lutHandle,
            tokenLength,
            pToken,
            &prevVersion.payloadLength,
            &prevVersion.logSecId,
            &lutItem
        );
    }

    if(acqLutResult == gpNvm_Result_NoDataAvailable || prevVersion.lookUpResult == gpNvm_Result_NoDataAvailable)
    {
        if(Nvm_Pool_IsNamespaceExhausted(poolId))
        {
            UInt8 nbrOfDefragCandidates;
            GP_LOG_PRINTF("Performing urgent namespace cleanup", 0);
            do
            {
                nbrOfDefragCandidates = Nvm_Pool_ReclaimNamespaceSpace(poolId);
            } while(nbrOfDefragCandidates > 0);

            if(Nvm_Pool_IsNamespaceExhausted(poolId))
            {
                result = gpNvm_Result_Error;
                goto Nvm_Write_Cleanup;
            }
        }
        Nvm_Pool_NumberOfUniqueTokens[poolId]++;
    }

    if ((prevVersion.lookUpResult == gpNvm_Result_DataAvailable)
     && (prevVersion.payloadLength == dataLength)
    )
    {
        UInt8 compareBuffer[GP_NVM_MAX_PAYLOADLENGTH]; /* can overlap with pData_32, explicitly left to compiler */
        result = Nvm_Token_GetRecordPayload(
            poolId,
            lutItem,
            prevVersion.payloadLength,
            tokenLength,
            compareBuffer
        );
        if (0 == MEMCMP(compareBuffer,pData, dataLength))
        {
            /* value has not changed */
            result = gpNvm_Result_DataAvailable;
            goto Nvm_Write_Cleanup;
        }
    }
    if (lowOnSpace)
    {
        if(prevVersion.lookUpResult == gpNvm_Result_DataAvailable && (dataLength <= prevVersion.payloadLength || prevVersion.payloadLength == 0))
        {
            /* update will invalidate old record, allowing defrag of old record */
        }
        else
        {
            /* avoid disasters by checking that the NVM will remain mutable after adding new token or having more active data */
            UInt8 pageId;
            Nvm_LogSectorId_t logSecId;
            UInt16 newRecordSize;
            newRecordSize = NVM_RECORD_SIZE(tokenLength, dataLength);

            /* can we still delete an existing key, or enlarge it to maximum length? */
            if ((Nvm_Pool_IsFull(poolId)) || (false == Nvm_Pool_Record_Allocate(poolId, newRecordSize, &logSecId, &pageId, gpNvm_DryRun_yes)))
            {
                result = gpNvm_Result_Error;
                goto Nvm_Write_Cleanup;
            }
        }
    }

    result = Nvm_Pool_Record_Add(poolId, tokenLength, pToken, dataLength, pData, prevVersion);
    if (result != gpNvm_Result_DataAvailable)
    {
        /* abort on addRecord issues */
        goto Nvm_Write_Cleanup;
    }

    Nvm_Pool_SeqNum_EvaluateSpread(poolId);
    Nvm_Pool_TryToStartExtendedDefrag(poolId);

Nvm_Write_Cleanup:
    if (freeLutAfterUse)
    {
        gpNvm_FreeLookup(lutHandle);
    }
    return result;
}

static gpNvm_Result_t Nvm_Pool_Record_Add(
                            gpNvm_PoolId_t poolId,
                            UInt8 tokenLength,
                            UInt8* pToken,
                            UInt8 dataLength,
                            UInt8* pData,
                            Nvm_Token_PreviousVersion_t prevVersion
                        )
{
    const UInt16 recordSize = NVM_RECORD_SIZE(tokenLength, dataLength);
    const UInt16 alignedRecordSize = NVM_ALIGNED_RECORD_SIZE(tokenLength, dataLength);
    Nvm_LogSectorId_t logSecId;
    UInt8 pageId;
    Bool status = false;
    UInt16 crcValue;
    UInt32 pData_32[NVM_DIV_ROUND_UP(NVM_MAX_ALIGNED_RECORD_SIZE, sizeof(UInt32))];
    UInt8* pData_8 = (UInt8*)pData_32;
    UInt16 offsetInSector;
    Nvm_MaskedSeqNum_t maskedSeqNum;
    gpNvm_Result_t result = gpNvm_Result_DataAvailable;

    if (tokenLength == 0)
    {
        return gpNvm_Result_Error;
    }
    if (pToken == NULL)
    {
        return gpNvm_Result_Error;
    }
#if (GP_NVM_MAX_PAYLOADLENGTH < 255)
    if (dataLength > GP_NVM_MAX_PAYLOADLENGTH)
    {
        return gpNvm_Result_Error;
    }
#endif
    if (tokenLength > GP_NVM_MAX_TOKENLENGTH)
    {
        return gpNvm_Result_Error;
    }

    if ((dataLength+tokenLength) > GP_NVM_MAX_BODYLENGTH)
    {
        return gpNvm_Result_Error;
    }

    if (false == Nvm_Pool_Record_Allocate(poolId, alignedRecordSize, &logSecId, &pageId, gpNvm_DryRun_no))
    {
        GP_LOG_SYSTEM_PRINTF("Cannot allocate NVM sector for token",0);
        GP_ASSERT_DEV_EXT(false);
        return gpNvm_Result_Error;
    }

    GP_ASSERT_SYSTEM(recordSize <= (FLASH_PAGE_SIZE - NVM_SECTOR_HEADER_SIZE));

    MEMSET(pData_8, 0x00, sizeof(pData_32));

    pData_8[NVM_RECORD_HEADER_OFFSET_TO_TOKENLENGTH]      = tokenLength;
    pData_8[NVM_RECORD_HEADER_OFFSET_TO_RESERVED]         = 0;
    pData_8[NVM_RECORD_HEADER_OFFSET_TO_FREQUENCY]        = 0;
    pData_8[NVM_RECORD_HEADER_OFFSET_TO_PAYLOADLENGTH]    = dataLength;

    maskedSeqNum = Nvm_Pool_SeqNum_GetNext(poolId);
    Nvm_Buf_Put_UInt16(pData_8, NVM_RECORD_HEADER_OFFSET_TO_SEQNUM_COUNTER, maskedSeqNum);

    MEMCPY(&(pData_8[NVM_RECORD_HEADER_OFFSET_TO_TOKEN]), pToken, tokenLength);
    MEMCPY(&(pData_8[NVM_RECORD_HEADER_OFFSET_TO_PAYLOAD(tokenLength)]), pData, dataLength);

    crcValue = gpUtils_CalculateCrc( &pData_8[NVM_RECORD_HEADER_OFFSET_TO_TOKENLENGTH], recordSize - NVM_RECORD_HEADER_CRC_BYTES);
    Nvm_Buf_Put_UInt16(pData_8, NVM_RECORD_HEADER_OFFSET_TO_CRC, crcValue);

    status = Nvm_LogSector_StoreRecord(poolId, logSecId, pageId, alignedRecordSize, pData_32, &offsetInSector);

    if (!status)
    {
        return gpNvm_Result_Error;
    }

    Nvm_Pool_SeqNum_RegisterSeqNumceCtr(poolId, maskedSeqNum, logSecId);

    if (prevVersion.lookUpResult == gpNvm_Result_DataAvailable)
    {
        /* update inactive sum variable in sector cache after writing new version */
        Nvm_SectorLut_t *sectorLut = Nvm_Pool_SectorLut_Get(poolId);
        sectorLut[prevVersion.logSecId].InactiveSum += NVM_ALIGNED_RECORD_SIZE(tokenLength, prevVersion.payloadLength);
    }

    Nvm_TokenLuts_Update(
        poolId,
        gpNvm_UpdateFrequencyIgnore,
        tokenLength,
        pToken,
        logSecId,
        offsetInSector
    );

    return result;
}

static void Nvm_TokenLuts_Update(
    gpNvm_PoolId_t poolId,
    gpNvm_UpdateFrequency_t updateFrequency,
    UInt8 tokenLength,
    UInt8* pToken,
    Nvm_LogSectorId_t logSecId,
    UInt16 offsetInSector
)
{
    UIntLoop i;
    NOT_USED(updateFrequency);
    for (i=0; i < GP_NVM_NBR_OF_LOOKUPTABLE_HANDLES; i++)
    {
        Nvm_TokenLut_t *lut;
        Bool LutSpecMatched;
        if (!NVM_LUTHANDLE_IS_ACTIVE(i))
        {
            continue;
        }

        lut = NVM_LUTHANDLE_TO_LUT(i);
        LutSpecMatched = Nvm_TokenLut_TokenBelongs(lut, poolId, tokenLength, pToken);

        if (LutSpecMatched)
        {
            Nvm_Token_t *lutItem = NULL;
            Nvm_TokenLut_Iterator_t lookupIter;
            gpNvm_Result_t lookUpResult;

            lookUpResult = Nvm_TokenLut_Query(&lutItem, lut, tokenLength, pToken, true, &lookupIter);

            if (lookUpResult != gpNvm_Result_DataAvailable)
            {
                lutItem = NULL;
                Nvm_TokenLut_Iterator_t writeIter;
                Bool IteratorStatus = Nvm_TokenLutIter_FirstFree(lut, &writeIter);
                if (!IteratorStatus){
                     /* should we allocate more space? */
                }
                else
                {
                    lutItem = Nvm_TokenLut_Append(&writeIter, tokenLength, pToken);
                    GP_ASSERT_DEV_EXT(lutItem);
                }
            }
            if (lutItem)
            {
                Nvm_Token_Update(lutItem, offsetInSector, logSecId);
            }
        }
    }

}

gpNvm_Result_t gpNvm_Remove(
    gpNvm_PoolId_t poolId,
    gpNvm_UpdateFrequency_t updateFrequencySpec,
    UInt8 tokenLength,
    UInt8* pToken
)
{
    NOT_USED(updateFrequencySpec);
    gpNvm_Result_t result;
    Bool freeLutAfterUse;
    gpNvm_LookupTable_Handle_t lutHandle;

    result = \
    Nvm_AcquireLutHandle(
      &lutHandle,
      poolId,
      tokenLength,
      pToken,
      &freeLutAfterUse,
      1,
      false,
      false
    );

    if (result != gpNvm_Result_DataAvailable)
    {
        /* When Lut was freshly built, NoDataAvailable error is possible */
        return result;
    }

    /* check if we need to return gpNvm_Result_NoDataAvailable if token is not
     * present in NVM */
    result = \
    Nvm_ReadUnique(
        NVM_LUTHANDLE_TO_LUT(lutHandle),
        poolId,
        tokenLength,
        pToken,
        0,
        NULL,
        NULL
    );

    if (result != gpNvm_Result_DataAvailable)
    {
        /* no item to delete */
        goto gpNvm_Remove_cleanup;
    }

    result = \
    Nvm_Write(
        poolId,
        tokenLength,
        pToken,
        NVM_PAYLOADLENGTH_INDICATING_TOKENREMOVED,
        NULL
    );

gpNvm_Remove_cleanup:
    if (freeLutAfterUse)
    {
        gpNvm_FreeLookup(lutHandle);
    }

    return result;
}

gpNvm_Result_t gpNvm_GetNextTokenKey(
    gpNvm_LookupTable_Handle_t handle,
    gpNvm_PoolId_t poolId,
    gpNvm_UpdateFrequency_t updateFrequencySpec,
    UInt8 tokenLength,
    UInt8* pToken,
    UInt8* pTokenKey
)
{
    NOT_USED(updateFrequencySpec);
    Nvm_Token_t *lutItem;
    Nvm_TokenLut_t *lut;
    Nvm_TokenLut_Iterator_t   iter;

#define NBR_OF_BITS          (sizeof(UInt32) * 8)
#define NBR_OF_IN_USE_WORDS  ((UINT8_MAX+1) / NBR_OF_BITS)


    UInt32 inUse[NBR_OF_IN_USE_WORDS];
    UInt8 wordIdx, bitIdx;

    GP_ASSERT_SYSTEM(handle < GP_NVM_NBR_OF_LOOKUPTABLE_HANDLES);
    if (!NVM_LUTHANDLE_IS_ACTIVE(handle))
    {
        return gpNvm_Result_NoLookUpTable;
    }
    lut = NVM_LUTHANDLE_TO_LUT(handle);


    Bool IteratorInit = Nvm_TokenLutIter_Start(lut, &iter);
    GP_ASSERT_DEV_EXT(IteratorInit);
    if (!IteratorInit)
    {
        return gpNvm_Result_Error;
    }

    if (Nvm_TokenLutIter_IsAtEnd(&iter))
    {
        return gpNvm_Result_NoDataAvailable;
    }

    GP_ASSERT_DEV_EXT(pTokenKey);
    (*pTokenKey) =  0;
    MEMSET(inUse, 0, sizeof(inUse));

    do
    {
        UInt8 tokenContent[GP_NVM_MAX_TOKENLENGTH];
        gpNvm_Result_t result;
        Bool isDeleted;
        UInt8 iterTokenLength;

        lutItem = Nvm_TokenLutIter_GetItem(&iter);

        UInt8 ItemHeader[NVM_RECORD_HEADER_SIZE];

        result = Nvm_Token_GetRecordHeader(lut->poolId, lutItem, ItemHeader);
        if(result != gpNvm_Result_DataAvailable)
        {
            return result;
        }

        iterTokenLength = ItemHeader[NVM_RECORD_HEADER_OFFSET_TO_TOKENLENGTH];

        Nvm_Token_GetRecordToken(lut->poolId,
                                 lutItem,
                                 iterTokenLength,
                                 tokenContent);

        if(!((iterTokenLength >= tokenLength) && (0 == MEMCMP(tokenContent, pToken, tokenLength))))
        {
            continue;
        }
        const UInt8 foundKey = tokenContent[tokenLength];
        result  = Nvm_Token_IsDeleted(poolId, lutItem, &isDeleted);
        if (result!=gpNvm_Result_DataAvailable)
        {
            return gpNvm_Result_Error;
        }
        wordIdx = foundKey /NBR_OF_BITS;
        bitIdx = foundKey % NBR_OF_BITS;
        if (isDeleted)
        {
            BIT_CLR(inUse[wordIdx], bitIdx);
            continue;
        }

        BIT_SET(inUse[wordIdx], bitIdx);
    }
    while (Nvm_TokenLutIter_Next(&iter));

    for (wordIdx = 0; wordIdx  < NBR_OF_IN_USE_WORDS; wordIdx++)
    {
        for (bitIdx = 0; bitIdx < NBR_OF_BITS; bitIdx++)
        {
            UInt8 key = (wordIdx * NBR_OF_BITS) + bitIdx;
            if (!BIT_TST(inUse[wordIdx], bitIdx))
            {
                (*pTokenKey) =  key;
                return gpNvm_Result_DataAvailable;
            }
        }
    }
    return gpNvm_Result_NoDataAvailable;
}

Bool Nvm_CheckAccessible(void)
{
    return true;
}

static void Nvm_Pools_Process_ErasePending(void)
{
    gpNvm_PoolId_t poolId;
    for (poolId = 0; poolId < Nvm_Flash_NbrOfPools; ++poolId)
    {
        if (Nvm_Pool_IsErasePending(poolId))
        {
            Nvm_Flash_PhysicalErase(poolId);
        }
    }
}

static void Nvm_Pool_SetErasePending(gpNvm_PoolId_t poolId)
{
    Nvm_PhySectorId_t offset;
    Nvm_PhySectorId_t nbrOfPhysicalSectors;
    UInt32 pData_32[NVM_SECTOR_HEADER_SIZE/sizeof(UInt32)];
    UInt8 *pData_8 = (UInt8*)pData_32;
    UIntLoop i;

    offset = Nvm_Pool_GetPhySectorOffset(poolId);
    nbrOfPhysicalSectors = Nvm_Pool_PhySectors_GetNbr(poolId);

    for (i=0; i < nbrOfPhysicalSectors; i++)
    {
        Nvm_PhySectorId_t phySecId = offset + i;
        Nvm_Flash_Read(NVM_SECTOR_ADDROFFSET(phySecId), NVM_SECTOR_HEADER_SIZE, pData_8);
        pData_8[NVM_SECTOR_HEADER_OFFSET_TO_ERASEPENDING] = NVM_SECTOR_HEADER_ERASEPENDING_VALUE;
        Nvm_Flash_Write(NVM_SECTOR_ADDROFFSET(phySecId), NVM_SECTOR_HEADER_SIZE, pData_32);
    }
}

static Bool Nvm_Pool_IsErasePending(gpNvm_PoolId_t poolId)
{
    Nvm_PhySectorId_t offset;
    Nvm_PhySectorId_t nbrOfPhysicalSectors;
    UInt8 pData_8[NVM_SECTOR_HEADER_SIZE];
    UIntLoop i;

    offset = Nvm_Pool_GetPhySectorOffset(poolId);
    nbrOfPhysicalSectors = Nvm_Pool_PhySectors_GetNbr(poolId);

    for (i=0; i < nbrOfPhysicalSectors; i++)
    {
        Nvm_PhySectorId_t phySecId = offset + i;
        Nvm_Flash_Read(NVM_SECTOR_ADDROFFSET(phySecId), NVM_SECTOR_HEADER_SIZE, pData_8);
        if (pData_8[NVM_SECTOR_HEADER_OFFSET_TO_ERASEPENDING] == NVM_SECTOR_HEADER_ERASEPENDING_VALUE)
        {
            return true;
        }
    }
    return false;
}

void Nvm_Pool_PhysicalErase(gpNvm_PoolId_t poolId)
{
    if (poolId == gpNvm_PoolId_AllPoolIds)
    {
        gpNvm_PoolId_t p;
        for (p=0; p < Nvm_Flash_NbrOfPools; p++)
        {
            Nvm_Pool_SetErasePending(p);
        }
        for (p=0; p < Nvm_Flash_NbrOfPools; p++)
        {
            Nvm_Flash_PhysicalErase(p);
        }
    }
    else
    {
        Nvm_Pool_SetErasePending(poolId);
        Nvm_Flash_PhysicalErase(poolId);
    }
}

static void Nvm_Flash_PhysicalErase(gpNvm_PoolId_t poolId)
{
    Nvm_PhySectorId_t i;
    Nvm_PhySectorId_t offset;
    Nvm_PhySectorId_t nbrOfPhysicalSectors;

    GP_ASSERT_SYSTEM(poolId < Nvm_Flash_NbrOfPools);

    offset = Nvm_Pool_GetPhySectorOffset(poolId);
    nbrOfPhysicalSectors = Nvm_Pool_PhySectors_GetNbr(poolId);

    for (i=0; i < nbrOfPhysicalSectors; i++)
    {
        Nvm_PhySectorId_t phySecId = offset + i;
        Nvm_Flash_EraseSector(phySecId) ;
    }
}

void Nvm_Erase(void)
{
    /* erase all pools */
    gpNvm_ErasePool(gpNvm_PoolId_AllPoolIds);
}

void Nvm_TagIf_Erase(void)
{
    gpNvm_ErasePool(gpNvm_PoolId_Tag);
}

/*****************************************************************************
 *                    Legacy Function Definitions
 *****************************************************************************/


void Nvm_DumpInfo(void)
{
}
