/*
 * Copyright (c) 2021, Qorvo Inc
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

#ifndef _GPPTC_SERVER_H_
#define _GPPTC_SERVER_H_

/** @file "gpPTC_server.h"
 *
 *  gpPTC
 *
 *  gpPTC server definition
*/

/*****************************************************************************
 *                    Parameter checking stubs for requests
 *****************************************************************************/


/*****************************************************************************
 *                    Public Function Declarations
 *****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

void gpPTC_InitServer(void);
void gpPTC_DeInitServer(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_GPPTC_SERVER_H_

