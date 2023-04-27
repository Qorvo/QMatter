/*
 * Copyright (c) 2008-2016, GreenPeak Technologies
 * Copyright (c) 2017-2019, Qorvo Inc
 *
 * gpLog.c
 *
 * This file contains the implementation of the logging module (printfs and asserts).
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

#ifdef GP_DIVERSITY_LOG

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_LOG

#include "hal.h"

#include "gpLog.h"
#include "gpCom.h"

#include "gpAssert.h"

#include <stdarg.h>             // va_arg()
#include <string.h>             // memmove()
#include <stdio.h>              // memmove()
#include <stddef.h>

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#ifdef GP_BIG_ENDIAN
#define LOG_ENDIANESS gpLog_ParameterEndianessBig
#else
#define LOG_ENDIANESS gpLog_ParameterEndianessLittle
#endif

#define GP_LOG_LEN_DEFAULT              (80 + 3 /*CommandID + ModuleID + formatStringMode*/)

/* log string length can be overruled from application code. */
#ifndef GP_LOG_MAX_LEN
#define GP_LOG_MAX_LEN                  (GP_LOG_LEN_DEFAULT)
#endif /* GP_LOG_MAX_LEN */

#define GP_LOG_HEADER_LENGTH            (5+4)

//gcc does not like va_arg(short), even if short=int ->promote
//if va_arg(short) is reached, it will abort.
#define va_arg_promoted(args, type)      \
    (sizeof(type) > sizeof(unsigned int) \
     ? va_arg(args, type)                \
     : va_arg(args, unsigned int))

#ifndef GP_LOG_COMMUNICATION_ID
#define GP_LOG_COMMUNICATION_ID                  GP_COM_DEFAULT_COMMUNICATION_ID
#endif

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

#define gpLog_FormatStringModeString        0
#define gpLog_FormatStringModePointer       1
typedef UInt8 gpLog_FormatStringMode_t;

#define gpLog_ParameterEndianessLittle  0
#define gpLog_ParameterEndianessBig     1
typedef UInt8 gpLog_ParameterEndianess_t;

#define gpLog_CommandIDPrintfIndication         0x02
typedef UInt8 gpLog_CommandID_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

static UInt8 gpLog_PrintfTotalCnt = 0;

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/
/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/
/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
void gpLog_Init(void)
{
    HAL_DISABLE_GLOBAL_INT();
    gpLog_PrintfTotalCnt = 0;
    HAL_ENABLE_GLOBAL_INT();
}


static void Log_SizeErr(UInt8 componentID, FLASH_STRING format_str,Bool progmem)
{
    Char log_strng[21]="ExtSize:";
    if (progmem)
    {
        STRNCPY_P(&log_strng[8], format_str,sizeof(log_strng)-8-1);
    }
    else
    {
        const char * ramfmtstr = (const char *) format_str;
        STRNCPY(&log_strng[8], ramfmtstr,sizeof(log_strng)-8-1);
    }
    log_strng[sizeof(log_strng)-1]='\0';
    gpLog_Printf(componentID,false,(FLASH_STRING)log_strng, 0);
}

void gpLog_Printf(UInt8 componentID, Bool progmem , FLASH_STRING format_str, UInt8 length , ...)
{
    Char  log_strng[GP_LOG_MAX_LEN];

    UInt8  nbr_chars = 0;
    UInt8 componentId = GP_COMPONENT_ID_LOG;
    gpCom_CommunicationId_t communicationId = GP_LOG_COMMUNICATION_ID;
    UInt32 time = 0;

    UInt8* pData;
    Char * pFmt;
    UInt8  formatLoopCnt = 0;

    UInt8  format_str_length;

    va_list  args;

    // When gpCom module is disabled, then return directly (avoid build of the printf)
    if (!gpCom_GetTXEnable())
    {
        return;
    }

    HAL_TIMER_GET_CURRENT_TIME(time);
    HOST_TO_LITTLE_UINT32(&time);
    //Get length of format string - can be done non-atomic
    if (progmem)
    {
        format_str_length = STRLEN_P(format_str) + 1 /* NULL character */;
    }
    else
    {
        const char * ramfmtstr = (const char *) format_str;
        format_str_length = STRLEN(ramfmtstr) + 1 /* NULL character */;
    }

    //Check if initial header + format string still fits
    if ((GP_LOG_HEADER_LENGTH + format_str_length) > sizeof(log_strng))
        goto overflow;

    //Construction of header
    log_strng[nbr_chars++] = gpLog_CommandIDPrintfIndication;
    log_strng[nbr_chars++] = componentID;
    log_strng[nbr_chars++] = gpLog_FormatStringModeString | (LOG_ENDIANESS << 1);
    log_strng[nbr_chars++] = 0; //Former interval counter
    HAL_DISABLE_GLOBAL_INT();
    gpLog_PrintfTotalCnt++;
    log_strng[nbr_chars++] = gpLog_PrintfTotalCnt;
    HAL_ENABLE_GLOBAL_INT();
    MEMCPY(&(log_strng[nbr_chars]),&time,4);
    nbr_chars += 4;

    //Copy format string
    if (progmem)
    {
        MEMCPY_P(&(log_strng[nbr_chars]), format_str, format_str_length);
    }
    else
    {
        const char * ramfmtstr = (const char *) format_str;
        MEMCPY(&(log_strng[nbr_chars]), ramfmtstr, format_str_length);
    }

    //Copy variables to be filled in
    pFmt = (Char*)&(log_strng[nbr_chars]);
    nbr_chars += format_str_length;

    va_start(args, length);
    formatLoopCnt = 0;
    //local pointer pData as UInt8* because casting log_strng[..] to LOG_TYPE_FROM_STACK aligns potential unaligned address
    //copy args to the pData location done via a local variable of type LOG_TYPE_FROM_STACK and a MEMCPY
    pData = (UInt8*)&(log_strng[nbr_chars]);
    while (pFmt[formatLoopCnt])
    {
        if (pFmt[formatLoopCnt] == '%')
        {
            while (pFmt[formatLoopCnt+1] >= '0' && pFmt[formatLoopCnt+1] <= '9')
            {
                /* ignore length indicator */
                formatLoopCnt++;
            }
            if (pFmt[formatLoopCnt+1] == 'l')
            {
                UInt32 arg32;
                if ((nbr_chars + sizeof(UInt32)) > sizeof(log_strng))
                {
                    goto overflow;
                }
                arg32= va_arg_promoted(args, UInt32);
                MEMCPY(pData,&arg32,sizeof(UInt32));
                pData += sizeof(UInt32);
                nbr_chars += sizeof(UInt32);
            }
            else if ((pFmt[formatLoopCnt+1] == 'x') ||
                     (pFmt[formatLoopCnt+1] == 'X') ||
                     (pFmt[formatLoopCnt+1] == 'u') ||
                     (pFmt[formatLoopCnt+1] == 'i') ||
                     (pFmt[formatLoopCnt+1] == 'd') ||
                     (pFmt[formatLoopCnt+1] == 'c'))
            {
                UInt16 arg16;
                if ((nbr_chars + sizeof(UInt16)) > sizeof(log_strng))
                {
                    goto overflow;
                }
                /*Note: we do it like this because %d is 2 bytes; va_arg will promote a short to
                        an int and thus generate a warning if we try to use a UInt16 variable */
                arg16 = (UInt16)(va_arg_promoted(args, int) & 0xFFFF);
                MEMCPY(pData, &arg16, sizeof(UInt16));
                pData += sizeof(UInt16);
                nbr_chars += sizeof(UInt16);
            }
            else
            {
                //Part of the formatting string containing % characters but no actual formatting after it
                //can come here. Fi an URL print with substituted special characters
            }
        }
        formatLoopCnt++;
    }
    va_end(args);

    //Tranmsit log string


    gpCom_DataRequest(componentId, nbr_chars, (UInt8*)log_strng, communicationId);
    return;

overflow:
    Log_SizeErr(componentID, format_str, progmem);
    GP_ASSERT_DEV_INT(false);
    return;
}

void gpLog_Flush(void)
{
    gpCom_Flush();
}

/*
 * @brief Print a buffer, byte by byte
 * @note  This printed buffer will be padded up to a multiple of 4 with values from the stack.
 */
void gpLog_PrintBuffer(UInt16 length, UInt8* pData)
{
    UInt16 k;
    for (k = 0; k < length; k+=4)
    {
        GP_LOG_SYSTEM_PRINTF("%i: %x %x %x %x",0,k, pData[k+0],pData[k+1],pData[k+2],pData[k+3]);
        gpLog_Flush();
    }
}

#endif //ifdef GP_DIVERSITY_LOG
