
/*
 * Copyright (c) 2017, Qorvo Inc
 *
 *   This file contains the implementation of the gpPd API protocol.
 *
 *   Packet Descriptor interface
 *   Handler Implementations
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
 * Alternatively, this software may be distributed under the terms of the
 * modified BSD License or the 3-clause BSD License as published by the Free
 * Software Foundation @ https://directory.fsf.org/wiki/License:BSD-3-Clause
 *
 * $Header$
 * $Change$
 * $DateTime$
 *
 */


/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"
#include "gpLog.h"
#include "gpAssert.h"
#include "gpPd.h"

#define GP_COMPONENT_ID GP_COMPONENT_ID_APP

#ifndef GP_PD_NR_OF_HANDLES
/* workaround for when gpPd is not configured properly, but marshalling code
 included anyhow */
#define GP_PD_NR_OF_HANDLES (1)
#endif // ifndef GP_PD_NR_OF_HANDLES
/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define GPPD_HANDLE_ASSOCIATION_NUMBER (GP_PD_NR_OF_HANDLES + 1)  //Place for all available pd's + 1 invalid

/*****************************************************************************
 *                    Typedef Definition
 *****************************************************************************/

typedef struct {
    UInt8   oldHandle;
    UInt8   newHandle;
} Pd_pdHandleMapping_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/
static Pd_pdHandleMapping_t Pd_handleAssociations[GPPD_HANDLE_ASSOCIATION_NUMBER];

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpPd_InitPdHandleMapping(void)
{
    UIntLoop i;
    for(i=0; i < GPPD_HANDLE_ASSOCIATION_NUMBER; i++)
    {
        Pd_handleAssociations[i].newHandle = GP_PD_INVALID_HANDLE;
        Pd_handleAssociations[i].oldHandle = GP_PD_INVALID_HANDLE;
    }
}

Bool gpPd_StorePdHandle(gpPd_Handle_t newPdHandle, gpPd_Handle_t pdHandle)
{
    UIntLoop i;
    for(i=0; i < GPPD_HANDLE_ASSOCIATION_NUMBER; i++)
    {
        if( GP_PD_INVALID_HANDLE == Pd_handleAssociations[i].oldHandle )
        {
            Pd_handleAssociations[i].newHandle = newPdHandle;
            Pd_handleAssociations[i].oldHandle = pdHandle;
            return true;
        }
    }
    return false;
}

/* This function will check if pdHandle exist in list, if not it will return GP_INVALID_HANDLE for storedPdHandle and return true
   If no free entry in list for GP_INVALID_HANDLE it will return false */
Bool gpPd_GetStoredPdHandle(gpPd_Handle_t* storedPdHandle, gpPd_Handle_t pdHandle)
{
    UIntLoop i;
    for(i=0; i < GPPD_HANDLE_ASSOCIATION_NUMBER; i++)
    {
        if(Pd_handleAssociations[i].oldHandle == pdHandle)
        {
            *storedPdHandle = Pd_handleAssociations[i].newHandle;
            return true;
        }
    }
    *storedPdHandle = GP_PD_INVALID_HANDLE;

    //We can add only 1 invalid entry, check if already exist
    for(i=0; i < GPPD_HANDLE_ASSOCIATION_NUMBER; i++)
    {
        if(GP_PD_INVALID_HANDLE == Pd_handleAssociations[i].newHandle
            && GP_PD_INVALID_HANDLE != Pd_handleAssociations[i].oldHandle)
        {
            return false;
        }
    }
    //No invalid found, add to list
    return gpPd_StorePdHandle(GP_PD_INVALID_HANDLE,pdHandle);
}

gpPd_Handle_t gpPd_RestorePdHandle(gpPd_Handle_t newPdHandle, Bool remove)
{
    UIntLoop i;
    gpPd_Handle_t oldPdHandle = GP_PD_INVALID_HANDLE;

    for(i=0; i < GPPD_HANDLE_ASSOCIATION_NUMBER; i++)
    {
        if(Pd_handleAssociations[i].newHandle == newPdHandle
            && GP_PD_INVALID_HANDLE != Pd_handleAssociations[i].oldHandle)
        {
            oldPdHandle = Pd_handleAssociations[i].oldHandle;
            if(remove)
            {
                Pd_handleAssociations[i].newHandle = GP_PD_INVALID_HANDLE;
                Pd_handleAssociations[i].oldHandle = GP_PD_INVALID_HANDLE;
            }
            break;
        }
    }
    return oldPdHandle;
}

