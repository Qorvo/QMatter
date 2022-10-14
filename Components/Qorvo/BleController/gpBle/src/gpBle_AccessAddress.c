/*
 * Copyright (c) 2017, Qorvo Inc
 *
 *   Bluetooth Low Energy interface
 *   Implementation of gpBle
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

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLE

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBle_defs.h"
#include "gpRandom.h"
#include "gpLog.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

// Access address generation requirements
#define BLE_ACCESS_ADDRESS_SIZE_BITS                        (BLE_ACCESS_ADDRESS_SIZE * 8)
#define BLE_ACCESS_ADDRESS_MAX_CONSECUTIVE_EQUAL_SYMBOLS    6
#define BLE_ACCESS_ADDRESS_MAX_BIT_TRANSITIONS              24
#define BLE_ACCESS_ADDRESS_RELEVANT_MSBS_FOR_TRANSITIONS    6
#define BLE_ACCESS_ADDRESS_MIN_BIT_TRANSITIONS_IN_RELEVANT_MSBS    2
#define BLE_ACCESS_ADDRESS_MIN_NR_OF_ADV_CH_DIFFERENCE_BITS 2

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static Bool Ble_LlcpValidAccessAddressBitRequirements(gpBle_AccessAddress_t accessAddress);
static Bool Ble_LlcpValidAccessAddressAdvDifference(gpBle_AccessAddress_t accessAddress);


/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

Bool Ble_LlcpValidAccessAddressBitRequirements(gpBle_AccessAddress_t accessAddress)
{
    UIntLoop i;
    UInt8 nrOfConsecutiveEquals = 1;
    UInt8 maxNrOfConsecutiveEquals = 0;
    UInt8 nrOfTransitions = 0;
    UInt8 nrOfTransitionsInRelevantMsbits = 0;

    for(i = 1; i < BLE_ACCESS_ADDRESS_SIZE_BITS; i++)
    {
        if(BIT_TST(accessAddress, i - 1) == BIT_TST(accessAddress, i))
        {
            // 2 consecutive bits are equal
            nrOfConsecutiveEquals++;
        }
        else
        {
            // 2 consecutive bits are different
            nrOfTransitions++;
            nrOfConsecutiveEquals = 1;

            if(i > (BLE_ACCESS_ADDRESS_SIZE_BITS - BLE_ACCESS_ADDRESS_RELEVANT_MSBS_FOR_TRANSITIONS))
            {
                nrOfTransitionsInRelevantMsbits++;
            }
        }

        if(nrOfConsecutiveEquals > maxNrOfConsecutiveEquals)
        {
            maxNrOfConsecutiveEquals = nrOfConsecutiveEquals;
        }

        if(maxNrOfConsecutiveEquals > BLE_ACCESS_ADDRESS_MAX_CONSECUTIVE_EQUAL_SYMBOLS)
        {
            // access address requirement 1 violated: max 6 consecutive zeros or ones
            GP_LOG_PRINTF("AA 0x%lx invalid: > 6 cons 0/1",0,(unsigned long int)accessAddress);
            return false;
        }

        if(nrOfTransitions > BLE_ACCESS_ADDRESS_MAX_BIT_TRANSITIONS)
        {
            // access address requirement 5 violated: max 24 bit transitions
            GP_LOG_PRINTF("AA 0x%lx invalid: > 24 transitions",0,(unsigned long int)accessAddress);
            return false;
        }
    }

    if(nrOfTransitionsInRelevantMsbits < BLE_ACCESS_ADDRESS_MIN_BIT_TRANSITIONS_IN_RELEVANT_MSBS)
    {
        // access address requirement 6 violated: min 2 bit transitions in 6 most significant bits
        GP_LOG_PRINTF("AA 0x%lx invalid: < 2 trans in 6 msbs",0,(unsigned long int)accessAddress);
        return false;
    }
    else
    {
        return true;
    }
}

Bool Ble_LlcpValidAccessAddressAdvDifference(gpBle_AccessAddress_t accessAddress)
{
    UIntLoop i;
    UInt8 nrOfDifferentBits = 0;
    gpBle_AccessAddress_t advChannelAccessAddress = 0x8E89BED6UL;

    for(i = 0; i < BLE_ACCESS_ADDRESS_SIZE_BITS; i++)
    {
        if(BIT_TST(accessAddress, i) != BIT_TST(advChannelAccessAddress, i))
        {
            nrOfDifferentBits++;
        }

        if(nrOfDifferentBits >= BLE_ACCESS_ADDRESS_MIN_NR_OF_ADV_CH_DIFFERENCE_BITS)
        {
            return true;
        }
    }

    // Access address requirement 2 or 3 violated: differs in less than 2 bits from advertising channel access address
    GP_LOG_PRINTF("AA 0x%lx invalid: < 2 bit diff with adv channel AA",0,(unsigned long int)accessAddress);
    return false;
}


/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

gpBle_AccessAddress_t gpBle_CreateAccessAddress(void)
{
    gpBle_AccessAddress_t accessAddress;

    // Pointer to accessAddress (for easier calculation)
    UInt8* pAccessAddress = (UInt8*)&accessAddress;
    Bool accessAddressValid = false;

    while(!accessAddressValid)
    {
        // Try a new random sequence
        gpRandom_GetNewSequence(BLE_ACCESS_ADDRESS_SIZE, (UInt8*)pAccessAddress);

        if((pAccessAddress[0] == pAccessAddress[1]) && (pAccessAddress[1] == pAccessAddress[2]) && (pAccessAddress[2] == pAccessAddress[3]))
        {
            // access address requirement 4 violated: Not all four octets equal
            GP_LOG_PRINTF("AA 0x%lx invalid: all octets equal",0,(unsigned long int)accessAddress);
            continue;
        }

        if(!Ble_LlcpValidAccessAddressBitRequirements(accessAddress))
        {
            // No more than six consecutive zeros or ones
            // No more than 24 transitions
            // Minimum of two transitions in most significant 6 bits
            continue;
        }

        if(!Ble_LlcpValidAccessAddressAdvDifference(accessAddress))
        {
            // Not equal to advertising channel access address
            // More than 1 difference bit with advertising channel access address
            continue;
        }


#ifndef GP_COMP_CHIPEMU
        if(gpHal_BleIsAccessAddressAcceptable(accessAddress) != gpHal_ResultSuccess)
        {
            // This access address is not preferred for the gpHal.
            // Try to obtain a new one
            continue;
        }
#endif //GP_COMP_CHIPEMU

        accessAddressValid = true;
    }

    GP_LOG_PRINTF("Using access code 0x%lx",0,(unsigned long int)accessAddress);

    return accessAddress;
}

