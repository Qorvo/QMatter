/*
 * Copyright (c) 2018, Qorvo Inc
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

/** @file "gpHci_subevents_vsd.h"
 *
 *  Host Controller Interface
 *
 *  Declarations of the public functions and enumerations of gpHci_subevents_vsd.
*/

#ifndef _GPHCI_SUBEVENTS_VSD_H_
#define _GPHCI_SUBEVENTS_VSD_H_

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
Bool gpHci_VsdForwardEventProcessedMessages(gpHci_VsdMetaEventProcessedParams_t* eventProcessedParams);

Bool gpHci_VsdMetaWhiteListModifiedEvent(gpHci_VsdMetaWhiteListModified_t* whiteListModified);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_GPHCI_SUBEVENTS_VSD_H_

