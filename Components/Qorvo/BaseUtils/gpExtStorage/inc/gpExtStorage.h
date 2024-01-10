/*
 *   Copyright (c) 2018, Qorvo Inc
 *
 *   Extenal Storage
 *   Declarations of the public functions and enumerations of gpExtStorage.
 *
 *   This software is owned by Qorvo Inc
 *   and protected under applicable copyright laws.
 *   It is delivered under the terms of the license
 *   and is intended and supplied for use solely and
 *   exclusively with products manufactured by
 *   Qorvo Inc.
 *
 *
 *   THIS SOFTWARE IS PROVIDED IN AN "AS IS"
 *   CONDITION. NO WARRANTIES, WHETHER EXPRESS,
 *   IMPLIED OR STATUTORY, INCLUDING, BUT NOT
 *   LIMITED TO, IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *   QORVO INC. SHALL NOT, IN ANY
 *   CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 *   INCIDENTAL OR CONSEQUENTIAL DAMAGES,
 *   FOR ANY REASON WHATSOEVER.
 *
 */


#ifndef _GPEXTSTORAGE_H_
#define _GPEXTSTORAGE_H_

/// @file "gpExtStorage.h"
/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

/** @enum gpExtStorage_Result_t */
//@{
#define gpExtStorage_Success                                    0
#define gpExtStorage_OutOfRange                                 1
#define gpExtStorage_UnalignedAddress                           2
#define gpExtStorage_BlankFailure                               3
#define gpExtStorage_VerifyFailure                              4
#define gpExtStorage_BlockOverFlow                              5
typedef UInt8                               gpExtStorage_Result_t;
//@}

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/** @pointer to function gpExtStorage_cbEraseComplete_t
 *  @brief Pointer to callback for erase complete
*/
typedef void (*gpExtStorage_cbEraseComplete_t) (void);


/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Function to read blocks of data from external storage device
*
*   @param address                   Address to read data from external storage device
*   @param length                    Length of the data to read from external storage device
*   @param pData                     Pointer to data
*   @return result                   Status of read block
*/
gpExtStorage_Result_t gpExtStorage_ReadBlock(UInt32 address, UInt32 length, UInt8* pData);

/** @brief Function to write blocks of data to external storage device
*
*   @param address                   Address to write data to external storage device
*   @param length                    Length of the data to write to external storage device
*   @param pData                     Pointer to data
*   @return result                   Status of write block
*/
gpExtStorage_Result_t gpExtStorage_WriteBlock(UInt32 address, UInt32 length, UInt8* pData);

/** @brief Erase external storage device
*   @return result                   Status of erase operation
*/
gpExtStorage_Result_t gpExtStorage_Erase(void);

/** @brief Erase external storage device
*
*   @param cb                        Pointer to erase complete callback
*   @return result                   Status of erase operation
*/
gpExtStorage_Result_t gpExtStorage_EraseNoBlock(gpExtStorage_cbEraseComplete_t cb);

/** @brief Erase selected 4kB sector of external storage device
*
*   @param address                   Address within sector to erase
*   @return result                   Status of erase operation
*/
gpExtStorage_Result_t gpExtStorage_EraseSector(uint32_t address);

/** @brief Erase selected 4kB sector of external storage device
*
*   @param address                   Address within sector to erase
*   @param cb                        Pointer to erase complete callback
*   @return result                   Status of erase operation
*/
gpExtStorage_Result_t gpExtStorage_EraseSectorNoBlock(uint32_t address, gpExtStorage_cbEraseComplete_t cb);

//Indications

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_GPEXTSTORAGE_H_

