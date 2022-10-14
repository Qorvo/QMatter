/*
 * Copyright (c) 2015, GreenPeak Technologies
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

#ifndef _GPBLEPRESCHEDULER_H_
#define _GPBLEPRESCHEDULER_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"
#include "gpBle.h"
#include "gpBle_defs.h"
#include "gpHal_Ble.h"
#ifdef GP_COMP_BLEADVERTISER
#include "gpBleAdvertiser.h"
#endif // GP_COMP_BLEADVERTISER
/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#ifndef GP_BLE_MAX_NUMBER_PRESCHEDULED_ACTIVITIES
#define GP_BLE_MAX_NUMBER_PRESCHEDULED_ACTIVITIES       (25)
#endif // GP_BLE_MAX_NUMBER_PRESCHEDULED_ACTIVITIES

// GP_BLE_PRESCHED_ACTCB_TRIGGER_MARGIN covers:
// - NRT ActCB processing time to schedule the sED
// - RT processing time to start executing the sED
#define GP_BLE_PRESCHED_ACTCB_TRIGGER_MARGIN            (1000)

#define GPBLEPRESCHED_MARGIN_BETWEEN_ACTIVITIES_DEFAULT_US       (700)

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/
#define BLE_SCHED_SAFETY_MARGIN         (MS_TO_US(10))
#define BLE_SCHED_NO_COLLISION          (false)
/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/
// the PAL only contains triggers for services to update the PDL, possibly in bursts
// So one trigger in the PAL may result in multiple sEDs being queued in the PDL
// table 2.3

// List of possible prescheduler (PAL) trigger types
#define GP_BLE_PRESCHED_TYPE_PRIMADVEVENT       0
#define GP_BLE_PRESCHED_TYPE_LEGACYADVEVENT     1
#define GP_BLE_PRESCHED_TYPE_NEWAUXSEGMENT      2
#define GP_BLE_PRESCHED_TYPE_CONTAUXSEGMENT     3
#define GP_BLE_PRESCHED_TYPE_PERIODICSYNC       6
#define GP_BLE_PRESCHED_TYPE_INVALID          255
typedef UInt8 Ble_cbTrigger_t;

#define TRIGGER_TYPE_INVALID   GP_BLE_PRESCHED_TYPE_INVALID
#define TRIGGER_TIME_INVALID   0x80000000
#define ANY_RESOURCE_INDEX     0xFF

typedef struct {
    UInt32              triggerTime;    // planned execution Time
    UInt32              interval;
    UInt16              duration;
    Ble_cbTrigger_t     triggerType;
    UInt8               next;
    UInt8               prev;
    UInt8               handle;       // a service specific reference << -- HCI Advertising handle
}gpBle_PreSchedPALEntry_t;
/** @pointer to function gpBle_PreSchedActCb_t
 *  @brief The gpBle_PreSchedActCb_t callback type definition defines the Activity Callback - .
*   @param preSchedEndTime
*   @param gpBle_PreSchedPALEntry_t
*/
typedef void (*gpBle_PreSchedActCb_t) (UInt32 preSchedEndTime, gpBle_PreSchedPALEntry_t *pal);


/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

void gpBlePreSched_Init(void);
void gpBlePreSched_Reset(Bool firstReset);
// Purpose: first time scheduling of an activity when a service was started by e.g. the Host
void gpBlePreSched_StartActivity(UInt8 handle, Ble_cbTrigger_t triggerType, UInt32 triggerTime, UInt16 duration, UInt32 interval);
void gpBlePreSched_StopActivity(UInt8 handle, Ble_cbTrigger_t triggerType, gpHal_BleSedType_t sedType, UInt8 index);
void gpBlePreSched_RemoveActivity(UInt8 handle, Ble_cbTrigger_t triggerType, gpHal_BleSedType_t sedType, UInt8 index);
// Purpose: queue a future activity in the PAL - only for activities that have been started already
void gpBlePreSched_ScheduleActivity(UInt8 handle, Ble_cbTrigger_t triggerType, UInt32 triggerTime, UInt16 duration, UInt32 interval);
Bool gpBlePreSched_RequestActivityPlanning(UInt32 *windowStart, UInt16 duration, UInt32 maxWindowEnd);
void gpBlePreSched_RegisterActCb(Ble_cbTrigger_t triggerType, gpBle_PreSchedActCb_t ActCb);
UInt16 gpBlePreSched_GetMinimalSubeventDistance(void);

#ifdef GP_DIVERSITY_DEVELOPMENT
gpHci_Result_t gpBle_VsdSetMinimalSubeventDistance(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf);
#endif //GP_DIVERSITY_DEVELOPMENT

//Indications
#ifdef GP_COMP_UNIT_TEST
void gpBlePreSched_cbTriggerActivity(UInt32 preSchedEndTime, gpBle_PreSchedPALEntry_t* pPal);
#endif //GP_COMP_UNIT_TEST

#ifdef __cplusplus
}
#endif

#endif //_GPBLEPRESCHEDULER_H_

