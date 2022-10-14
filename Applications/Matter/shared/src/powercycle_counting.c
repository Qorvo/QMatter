

/*
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
 * $Header:
 * //depot/main/Embedded/Applications/P345_Matter_DK_Endnodes/vlatest/apps/matter/shared/src/powercycle_counting.c#none
 * $ $Change$ $DateTime$
 */

/** @file "gpAppFramework_Reset.c"
 *
 *  Application API
 *
 *  Implementation of gpAppFramework Reset
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID_APPFRAMEWORK 56
#define GP_COMPONENT_ID GP_COMPONENT_ID_APPFRAMEWORK

#include "powercycle_counting.h"
#include "global.h"
#include "gpLog.h"
#include "gpNvm.h"
#include "gpReset.h"
#include "gpSched.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define RESET_COUNTS_TAG_ID 0
#define GP_APP_NVM_BASE_TAG_ID (UInt16)(GP_COMPONENT_ID << 8)
#define RESET_COUNTING_PERIOD_US 2000000 // 2s

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/
static Bool Application_NvmResetCounts_DefaultInitializer(const ROM void *pTag,
                                                          UInt8 *pBuffer);
/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

const gpNvm_IdentifiableTag_t ROM gpApplication_NvmElements[] FLASH_PROGMEM = {
    {(GP_APP_NVM_BASE_TAG_ID + RESET_COUNTS_TAG_ID), NULL, sizeof(UInt8),
     gpNvm_UpdateFrequencyLow, Application_NvmResetCounts_DefaultInitializer,
     NULL},
};
const gpNvm_Tag_t ROM gpApplication_NvmSection[] FLASH_PROGMEM = {
    {NULL, sizeof(UInt8), gpNvm_UpdateFrequencyLow, NULL}, // ResetCount element
};

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/
static Bool Application_NvmResetCounts_DefaultInitializer(const ROM void *pTag,
                                                          UInt8 *pBuffer) {
  gpNvm_IdentifiableTag_t tag;
  UInt8 value = 0;

  MEMCPY_P((UInt8 *)&tag, pTag, sizeof(gpNvm_IdentifiableTag_t));
  if (NULL == pBuffer) {
    pBuffer = tag.pRamLocation;
    if (NULL == pBuffer) {
      return false;
    }
  }

  MEMCPY_P(pBuffer, (UInt8 *)&value, sizeof(UInt8));
  return true;
}

static void gpAppFramework_HardwareResetTriggered(void) {
  UInt8 resetCounts;
  // read number of reset counts
  gpNvm_Restore(GP_COMPONENT_ID, RESET_COUNTS_TAG_ID, &resetCounts);
  GP_LOG_SYSTEM_PRINTF("ResetCount[%d]", 0, resetCounts);

  // increment reset counts
  resetCounts ++;

  // write back updated value
  gpNvm_Backup(GP_COMPONENT_ID, RESET_COUNTS_TAG_ID, &resetCounts);

  // schedule check after 2 seconds
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
UInt8 gpAppFramework_Reset_GetResetCount(void) {
  UInt8 resetCounts;
  UInt8 resetCountsClear;

  gpNvm_Restore(GP_COMPONENT_ID, RESET_COUNTS_TAG_ID, &resetCounts);
  GP_LOG_PRINTF("Processing reset counts: %u", 0, resetCounts);

  resetCountsClear = 0;

  gpNvm_Backup(GP_COMPONENT_ID, RESET_COUNTS_TAG_ID, &resetCountsClear);

  return resetCounts;
}

void gpAppFramework_Reset_Init(void) {
  gpNvm_RegisterElements(gpApplication_NvmElements, number_of_elements(gpApplication_NvmElements));

  if (gpReset_GetResetReason() == gpReset_ResetReason_HW_Por) {
    gpAppFramework_HardwareResetTriggered();
  }
  gpSched_ScheduleEvent(RESET_COUNTING_PERIOD_US, gpAppFramework_Reset_cbTriggerResetCountCompleted);
}
