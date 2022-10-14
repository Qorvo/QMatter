/*
 * Copyright (c) 2015, GreenPeak Technologies
 * Copyright (c) 2017, Qorvo Inc
 *
 *
 *
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
#ifndef _GP_HCI_DEFS_H_
#define _GP_HCI_DEFS_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include "gpHci_Includes.h"
#if defined(GP_HCI_DIVERSITY_GPCOM_SERVER)
#include "gpCom.h"
#endif // GP_HCI_DIVERSITY_GPCOM_SERVER

typedef struct{
    UInt16 OpCode;
    UInt8 totalLength;
    UInt8 payload[64];
} gpHci_CrossOver_t;

#define SequencedEvent          ((UInt8)0x00)
#define SequencedRxData         ((UInt8)0x01)
#define SequencedRxIsoData      ((UInt8)0x02)
#define SequencedInvalid        ((UInt8)0xFF)
typedef UInt8 gpHci_SequencerElementType_t;

typedef struct {
    gpBle_EventBufferHandle_t handle;
} gpHci_SequencerEventElement_t;

typedef struct {
    gpBle_RxBufferHandle_t bufferIdx;
    gpHci_ConnectionHandle_t connHandle;
    gpHci_PacketBoundaryFlag_t pBoundary;
    UInt16 dataLength;
    UInt8 *pData;
} gpHci_SequencerDataElement_t;

typedef struct {
    gpHci_SequencerElementType_t elementType;
    union {
        gpHci_SequencerEventElement_t event;
        gpHci_SequencerDataElement_t  data;
    } element;
} gpHci_SequencerElement_t;

void gpHci_SequencerInit(void);
void gpHci_SequencerReset(void);
#if defined(GP_HCI_DIVERSITY_GPCOM_SERVER)
void gpHci_RegisterActivateTxCb(gpCom_cbActivateTx_t cb);
#endif // GP_HCI_DIVERSITY_GPCOM_SERVER

#endif //_GP_HCI_DEFS_H_
