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
 */

/** @file "gpHci_subevents.h"
 *
 *  Host Controller Interface
 *
 *  Declarations of the public functions and enumerations of gpHci_subevents.
*/

#ifndef _GPHCI_SUBEVENTS_H_
#define _GPHCI_SUBEVENTS_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"
#include "gpHci_types.h"

/* <CodeGenerator Placeholder> AdditionalIncludes */
#include "gpHci_Includes.h"
/* </CodeGenerator Placeholder> AdditionalIncludes */

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//Requests
//Indications
Bool gpHci_LEConnectionCompleteEvent(gpHci_LEConnectionCompleteEventParams_t* LEConnectionCompleteEventParams);

Bool gpHci_LEAdvertisingReportEvent(gpHci_LeMetaAdvertisingReportParams_t* LEAdvertisingReportParams);

Bool gpHci_LEConnectionUpdateCompleteEvent(gpHci_LEConnectionUpdateCompleteEventParams_t* LEConnectionUpdateCompleteEvent);

Bool gpHci_LEReadRemoteFeaturesCompleteEvent(gpHci_LEReadRemoteFeaturesCompleteParams_t* LEReadRemoteFeaturesComplete);

Bool gpHci_LELongTermKeyRequestEvent(gpHci_LELongTermKeyRequestParams_t* LELongTermKeyRequestEvent);

Bool gpHci_LERemoteConnectionParameterRequest(gpHci_LERemoteConnectionParamsEventParams_t* LERemoteConnectionParameterRequest);

Bool gpHci_LEDataLengthChangeEvent(gpHci_LeMetaDataLengthChange_t* LeMetaDataLengthChangeEvent);


Bool gpHci_LEDirectAdvertisingReportEvent(gpHci_LeMetaAdvertisingReportParams_t* LEDirectAdvertisingReportParams);

#if defined(GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED)
Bool gpHci_LEPhyUpdateComplete(gpHci_LEPhyUpdateCompleteEventParams_t* LEPhyUpdateComplete);
#endif //defined(GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED)


#if defined(GP_DIVERSITY_PERIODIC_ADVERTISING_SYNC)
Bool gpHci_LePeriodicAdvertisingSyncEstablishedEvent(gpHci_LePeriodicAdvertisingSyncEstablishedEvent_t* LePeriodicAdvertisingSyncEstablished);
#endif //defined(GP_DIVERSITY_PERIODIC_ADVERTISING_SYNC)

#if defined(GP_DIVERSITY_PERIODIC_ADVERTISING_SYNC)
Bool gpHci_LePeriodicAdvertisingReportEvent(gpHci_LePeriodicAdvertisingReportEvent_t* LePeriodicAdvertisingReport);
#endif //defined(GP_DIVERSITY_PERIODIC_ADVERTISING_SYNC)

#if defined(GP_DIVERSITY_PERIODIC_ADVERTISING_SYNC)
Bool gpHci_LePeriodicAdvertisingSyncLostEvent(gpHci_LePeriodicAdvertisingSyncLostEvent_t* LePeriodicAdvertisingSyncLost);
#endif //defined(GP_DIVERSITY_PERIODIC_ADVERTISING_SYNC)





#if defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED)
Bool gpHci_LeConnectionlessIqReport(gpHci_LEConnectionlessIqReportEventParams_t* LeConnectionlessIqReport);
#endif //defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED)

#if defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED)
Bool gpHci_LeConnectionIqReport(gpHci_LEConnectionIqReportEventParams_t* LeConnectionIqReport);
#endif //defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED)

#if defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED)
Bool gpHci_LeCteRequestFailed(gpHci_LECteRequestFailedEventParams_t* LeCteRequestFailed);
#endif //defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED)




#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_GPHCI_SUBEVENTS_H_

