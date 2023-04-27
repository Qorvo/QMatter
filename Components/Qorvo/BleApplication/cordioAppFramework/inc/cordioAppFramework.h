/*
 *   Copyright (c) 2018, Qorvo Inc
 *
 *   ARM Cordio App Framework.
 *   Declarations of the public functions and enumerations of cordioAppFramework.
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
 *   $Header$
 *   $Change$
 *   $DateTime$
 */


#ifndef _CORDIOAPPFRAMEWORK_H_
#define _CORDIOAPPFRAMEWORK_H_

/// @file "cordioAppFramework.h"
/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> AdditionalIncludes */

#include "wsf_types.h"
#include "app_api.h"
#include "global.h"

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
void cordioAppFramework_cbConnOpen(dmEvt_t* pMsg, appDbHdl_t hdl, UInt8 New);

/** @brief Get Connecting Link is a callback function to get LinkId of advertising Link, typical used to get index for new AppDb Record in cordioStack.
*   @return result                   LinkId of currently advertising Link. 0xFF if no link is advertising
*/
UInt8 cordioAppFramework_cbGetConnectingLink(void);

/** @brief Tell the application that a Slave Security Request came in.
*   @return void
*/
void cordioAppFramework_cbSlaveSecurityRequest(UInt8 connId);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_CORDIOAPPFRAMEWORK_H_
