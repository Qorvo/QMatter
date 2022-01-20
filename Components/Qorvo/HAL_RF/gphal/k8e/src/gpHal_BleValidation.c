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
 * $Header: //depot/release/Embedded/Components/Qorvo/HAL_RF/v2.10.2.1/comps/gphal/k8e/src/gpHal_BleValidation.c#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#define GP_LOCAL_LOG

#include "gpPd.h"
#include "gpHal.h"
#include "gpHal_DEFS.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_GPHAL

#define GPHAL_BLE_PREAMBLE_NR_OF_MATCHING_BITS      6
#define GPHAL_BLE_NUMBER_OF_SEQUENCE_BITS           7
#define GPHAL_BLE_SEQUENCE_MASK                     ((1 << GPHAL_BLE_NUMBER_OF_SEQUENCE_BITS) - 1)

// Validation constants provided by dig team
#define GPHAL_BLE_VALIDATION_SCORE_THRESHOLD        44 // Larger or equal to 45 (and thus larger than 44)
#define GPHAL_BLE_VALIDATION_SIMILARITY_THRESHOLD   6

#define GPHAL_BLE_VALIDATION_INDEX_INVALID          0xFF

#define GPHAL_BLE_PREAMBLE_MATCHING_ODD             0x55    // b01010101
#define GPHAL_BLE_PREAMBLE_MATCHING_EVEN            0xAA    // b10101010

/*****************************************************************************
 *                   Functional Macro Definitions
 *****************************************************************************/

// Based on preamble location algorithm from DSP team
// Set the preamble location to the index into the access address of the preamble minus 2.
// Note that the minus 2 comes from the fact that we use a 9 symbol correlation coefficients.
#define GP_HAL_BLE_VALIDATION_INDEX_CORRECTION(index)       ((index <= 2) ? 0 : (index - 2))

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef UInt8 gpHal_BleSequenceType_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

// Lookup table to obtain a score for a given sequence. The score indicates how similar a subsequence is from the preamble sequence
static const UInt8 gpHal_BleValidationSequenceLookupTable[] =
{
     0, 22, 26, 46, 28, 36, 47, 47, 29, 24, 20, 44, 48, 40, 48, 47,
    28, 24, 21, 45, 19,  9, 21, 40, 45, 27, 17, 42, 49, 42, 49, 46,
    26, 24, 25, 42, 21, 10, 18, 40, 20, 10,  0,  9, 17, 10, 25, 39,
    30, 38, 18, 23, 21, 10, 25, 40, 48, 45, 25, 21, 49, 31, 22, 22,
    43, 22, 24, 49, 34, 25, 45, 48, 23, 25, 10, 21, 23, 18, 45, 47,
    24, 25, 10, 17,  9,  0, 10, 20, 35, 18, 10, 21, 38, 25, 30, 26,
    32, 39, 35, 49, 41, 17, 23, 48, 40, 21,  9, 19, 42, 21, 42, 28,
    47, 48, 37, 48, 45, 20, 33, 29, 47, 47, 32, 28, 46, 26, 22,  0,
};

// If one of these patterns occur, in the (validated part of the) access address, we need to set the fake preamble present flag
static const UInt8 gpHal_BleFakePreambleSequences[] = { // 6 bit patterns
    42,     // b101010
    21,     // b010101
    63,     // b111111
    0,      // b000000
    36,     // b100100
    27,     // b011011
};

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

// The different metrics to collect
static void gpHal_BleCalculateFakePreambleIndex(gpHal_BleValidationParameters_t* pValidation, const gpHal_BleValidationInputParameters_t *inParams);
static void gpHal_BleCalculateValidationScores(gpHal_BleValidationParameters_t* pValidation, const gpHal_BleValidationInputParameters_t *inParams);
static void gpHal_BleCalculateSimilarityIndexes(gpHal_BleValidationParameters_t* pValidation, const gpHal_BleValidationInputParameters_t *inParams);
static UInt8 gpHal_BleCalculateSimilarityIndex(gpHal_BleSequenceType_t sequence1, gpHal_BleSequenceType_t sequence2);

// The different steps of the algorithm (as described in the presentation)
static void gpHal_BleCalculateValidationParameters(gpHal_BleValidationParameters_t* pValidation, const gpHal_BleValidationInputParameters_t *inParams);
static Bool gpHal_BleCalculateValidationParametersStep1(gpHal_BleValidationParameters_t* pValidation, const gpHal_BleValidationInputParameters_t *inParams);
static Bool gpHal_BleCalculateValidationParametersStep2(gpHal_BleValidationParameters_t* pValidation, const gpHal_BleValidationInputParameters_t *inParams);

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

// This functions checks if there is a subsequence in the access code that is identical to the preamble
void gpHal_BleCalculateFakePreambleIndex(gpHal_BleValidationParameters_t* pValidation, const gpHal_BleValidationInputParameters_t *inParams)
{
    UIntLoop i;
    UInt8 preambleIndex = GPHAL_BLE_PREAMBLE_MATCHING_INDEX_INVALID;

    if (inParams->isHighDataRate)
    {
        pValidation->fakePreambleFlag = true;
        pValidation->fakePreambleStartIndex = 1;
        return;
    }

    for(i = 0; i <= inParams->max_validation_n; i++)
    {
        gpHal_BleSequenceType_t sequence = ((inParams->accessAddress >> i) & 0x3F); // 0x3F selects lower 6 bits
        UIntLoop j;

        for(j = 0; j < number_of_elements(gpHal_BleFakePreambleSequences); j++)
        {
            if(sequence == gpHal_BleFakePreambleSequences[j])
            {
                preambleIndex = GP_HAL_BLE_VALIDATION_INDEX_CORRECTION(i);
                break;
            }
        }

        if(preambleIndex != GPHAL_BLE_PREAMBLE_MATCHING_INDEX_INVALID)
        {
            break;
        }
    }

    if(preambleIndex > pValidation->validationStartIndex)
    {
        pValidation->fakePreambleFlag = false;
        pValidation->fakePreambleStartIndex = GPHAL_BLE_PREAMBLE_MATCHING_INDEX_INVALID;
    }
    else
    {
        pValidation->fakePreambleFlag = true;
        pValidation->fakePreambleStartIndex = preambleIndex;
    }
}

// This function assigns a score to a subsequence, based on a table lookup (indicates similarity with preamble)
void gpHal_BleCalculateValidationScores(gpHal_BleValidationParameters_t* pValidation, const gpHal_BleValidationInputParameters_t *inParams)
{
    UIntLoop i;

    GP_ASSERT_DEV_INT(pValidation != NULL);

    for(i = 0; i <= inParams->max_validation_n; i++)
    {
        gpHal_BleSequenceType_t subSequence = ((inParams->accessAddress >> i) & GPHAL_BLE_SEQUENCE_MASK);

        // reverse subSequence in place
        subSequence = (subSequence & 0xF0) >> 4 | (subSequence & 0x0F) << 4;    // switch 4 bits
        subSequence = (subSequence & 0xCC) >> 2 | (subSequence & 0x33) << 2;    // switch 2 bits
        subSequence = (subSequence & 0xAA) >> 1 | (subSequence & 0x55) << 1;    // switch 1 bit
        subSequence >>= 1; // switch 1 bit to the right, because we started with a 7 bit number

        pValidation->scores[i] = gpHal_BleValidationSequenceLookupTable[subSequence];
    }
}

void gpHal_BleCalculateSimilarityIndexes(gpHal_BleValidationParameters_t* pValidation, const gpHal_BleValidationInputParameters_t *inParams)
{
    UIntLoop i;
    UIntLoop j;
    UInt8 preamble_repeat = inParams->isHighDataRate ? 1 : 0;

    UInt64 matchBuffer = 0;
    UInt8* pMatchBuffer = (UInt8*)&matchBuffer;
    pMatchBuffer[0] = (inParams->accessAddress & 0x01) ? GPHAL_BLE_PREAMBLE_MATCHING_ODD : GPHAL_BLE_PREAMBLE_MATCHING_EVEN;
    MEMCPY(&pMatchBuffer[1], &inParams->accessAddress, sizeof(inParams->accessAddress));

    for(i = 0; i <= inParams->max_validation_n; i++)
    {
        UInt8 nrSimil = 0;
        UInt8 firstSim = GPHAL_BLE_VALIDATION_INDEX_INVALID;
        UInt8 highestSimilarIndex = 0;

        gpHal_BleSequenceType_t sequenceVal = ((inParams->accessAddress >> i) & GPHAL_BLE_SEQUENCE_MASK);

        for(j = 0; j < 8+i; j++)
        {
            gpHal_BleSequenceType_t sequence2 = ((matchBuffer >> j) & GPHAL_BLE_SEQUENCE_MASK);
            UInt8 simIndex = gpHal_BleCalculateSimilarityIndex(sequenceVal,sequence2);

            if(simIndex > highestSimilarIndex)
            {
                highestSimilarIndex = simIndex;
            }

            if(simIndex > 4)
            {
                nrSimil++;
                /* the pattern for the first two bits repeats (4*preamble_repeat) times */
                if (j < 2)
                {
                    nrSimil += (preamble_repeat * 4);
                }

                if(firstSim == GPHAL_BLE_VALIDATION_INDEX_INVALID)
                {
                    firstSim = j;
                    /* take preamble repeats into account */
                    if (2 <= j)
                    {
                        firstSim += (preamble_repeat * 8);
                    }
                }
            }
        }

        pValidation->firstSimilar[i] = firstSim;
        pValidation->numberOfSimilarities[i] = nrSimil;
        pValidation->similarScore[i] = highestSimilarIndex;
    }
}

UInt8 gpHal_BleCalculateSimilarityIndex(gpHal_BleSequenceType_t sequence1, gpHal_BleSequenceType_t sequence2)
{
    UInt8 nrOfDifferences = 0;
    sequence1 &= GPHAL_BLE_SEQUENCE_MASK;
    sequence2 &= GPHAL_BLE_SEQUENCE_MASK;
    gpHal_BleSequenceType_t differenceBits = (sequence1^sequence2);

    // count bits in diffenceBits
    gpHal_BleSequenceType_t tmp = differenceBits;
    for (nrOfDifferences = 0; tmp; nrOfDifferences++)
    {
        tmp &= tmp - 1; // clear the least significant bit set
    }

    if(nrOfDifferences == 0)
    {
        return 7;
    }
    else if(nrOfDifferences == 1)
    {
        if(BIT_TST(differenceBits,0) || BIT_TST(differenceBits,(GPHAL_BLE_NUMBER_OF_SEQUENCE_BITS - 1)))
        {
            // only first or last different
            return 6;
        }
    }
    else if(nrOfDifferences == 2)
    {
        UIntLoop j;
        UInt8 firstDiff = 0xFF;
        UInt8 secondDiff = 0xFF;

        if(((differenceBits & 0x03) == 0x03) || ((differenceBits >> (GPHAL_BLE_NUMBER_OF_SEQUENCE_BITS - 2)) == 0x03))
        {
            // first two or last two different
            return 5;
        }

        for(j = 0; j < GPHAL_BLE_NUMBER_OF_SEQUENCE_BITS; j++)
        {
            if(BIT_TST(differenceBits,j))
            {
                if(firstDiff == 0xFF)
                {
                    firstDiff = j;
                }
                else if(secondDiff == 0xFF)
                {
                    secondDiff = j;
                    break;
                }
            }
        }

        if(((((firstDiff + 1) == secondDiff) && BIT_TST(sequence1,firstDiff) != BIT_TST(sequence1,secondDiff))|| (firstDiff == 0 && secondDiff == 6)))
        {
            // bit reversal
            return 4;
        }
    }

    // No similarity
    return 0;
}

void gpHal_BleCalculateValidationParameters(gpHal_BleValidationParameters_t* pValidation, const gpHal_BleValidationInputParameters_t *inParams)
{
    Bool resultSuccess;

    if(gpHal_BleCalculateValidationParametersStep1(pValidation, inParams))
    {
        return;
    }

    resultSuccess = gpHal_BleCalculateValidationParametersStep2(pValidation, inParams);

    // It is an error if last step was not successful
    GP_ASSERT_DEV_INT(resultSuccess);
}

Bool gpHal_BleCalculateValidationParametersStep1(gpHal_BleValidationParameters_t* pValidation, const gpHal_BleValidationInputParameters_t* inParams)
{
    UIntLoop i = 0;
    Bool matchFound = false;
    UInt8 score = 0;

    pValidation->isReliableAccessAddress = true;

    // Flow chart 1
    for(i = 0; i <= inParams->max_validation_n; i++)
    {
        if(pValidation->similarScore[i] == 0 && pValidation->scores[i] > GPHAL_BLE_VALIDATION_SCORE_THRESHOLD)
        {
            matchFound = true;
            pValidation->validationThresh = inParams->threshHold_hig;
            if(pValidation->scores[i] > score)
            {
                pValidation->validationStartIndex = i;
                score = pValidation->scores[i];
            }
        }
    }

    if(!matchFound)
    {
        pValidation->isReliableAccessAddress = false;
    }

    return matchFound;
}

Bool gpHal_BleCalculateValidationParametersStep2(gpHal_BleValidationParameters_t* pValidation, const gpHal_BleValidationInputParameters_t *inParams)
{
    UIntLoop i;
    UInt8 score = 0;
    Bool lessThanSix = false;

    // Flow chart 2
    for(i = 0; i <= inParams->max_validation_n; i++)
    {
        if(pValidation->similarScore[i] < GPHAL_BLE_VALIDATION_SIMILARITY_THRESHOLD)
        {
            if (!lessThanSix)
            {
                score = pValidation->scores[i];
                pValidation->validationStartIndex = i;
                lessThanSix = true;
            }
            else if(pValidation->scores[i] > score)
            {
                score = pValidation->scores[i];
                pValidation->validationStartIndex = i;
            }
        }
        else
        {
            if((pValidation->scores[i] > score) && !lessThanSix)
            {
                score = pValidation->scores[i];
                pValidation->validationStartIndex = i;
            }
        }
    }
    pValidation->validationThresh = inParams->threshHold_low;
    return true;
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpHal_BleValidationInit(void)
{
    COMPILE_TIME_ASSERT(8*sizeof(gpHal_BleSequenceType_t) >= GPHAL_BLE_NUMBER_OF_SEQUENCE_BITS);
}

// Given an access address as input, this function calculates all relevant parameters for the validation algorithm
void gpHal_BleGetValidationParameters(gpHal_BleValidationParameters_t* pValidation, const gpHal_BleValidationInputParameters_t *inParams)
{
    GP_ASSERT_DEV_INT(pValidation != NULL);

    // First, calculate the score for all subsequences. These are obtained by a simple table lookup
    gpHal_BleCalculateValidationScores(pValidation, inParams);

    // Now calculate the similarity index (for each combination of validation sequence and previous sequence)
    gpHal_BleCalculateSimilarityIndexes(pValidation, inParams);

    // Now calculate the actual validation settings for the hardware
    gpHal_BleCalculateValidationParameters(pValidation, inParams);

    //As a last step, calculate if a fake preamble is present or not (needs validationStartIndex value)
    gpHal_BleCalculateFakePreambleIndex(pValidation, inParams);
}

