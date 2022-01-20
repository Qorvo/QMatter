/*
 * Copyright (c) 2014-2016, GreenPeak Technologies
 * Copyright (c) 2017, Qorvo Inc
 *
 * gphal_dump_layout_code.c
 *   This file contains the implementation of the MAC functions
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
 * $Header: //depot/release/Embedded/Components/Qorvo/HAL_RF/v2.10.2.1/comps/gphal/k8e/src/gpHal_dump_layout_code.c#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */


/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_GPHAL

#include "gpHal.h"
#include "gpHal_DEFS.h"

// include the generated code:
#include "gpHal_dump_layout_structs.h"
#include "gpHal_dump_layout_code.h"

void gphal_dump_register(UInt16 layout, UInt16 RelBlockIndex, gpHal_Address_t relAddress)
{
    Bool compressed;
    UInt16 val;
    struct rangelist*  rl = layoutlist[layout].rlp;
    gpHal_Address_t address = relAddress;

    compressed = true;
    if(rl == pbm_format_t)
    {
        address += GPHAL_MM_PBMS_START + RelBlockIndex * GPHAL_MM_PBM_OFFSET;
    }
    else if (rl == event)
    {
        address += GP_MM_RAM_EVENT_START + RelBlockIndex * GP_MM_RAM_EVENT_OFFSET;
    }
    else if ((rl == ble_mgr) || (rl == fll_table))
    {
        compressed = false;
    }

    if (compressed)
    {
        val = GP_HAL_READ_REG(GP_MM_WISHB_ADDR_FROM_COMPRESSED(address));
    }
    else
    {
        val = GP_HAL_READ_REG(address);
    }
    GP_LOG_SYSTEM_PRINTF("a: 0x%lx v: 0x%x",0, (unsigned long int)address, val);
    gpLog_Flush();
}

void gphal_dump_block(UInt16 layout, UInt16 RelBlockIndex)
{
    gpHal_Address_t reg;
    UInt16 i;
    struct rangelist*  rl = layoutlist[layout].rlp;
    for(reg = 0; reg < layoutlist[layout].rangesize; reg++)
    {
        if (rl == event)
        {
            GP_LOG_SYSTEM_PRINTF("Event %i register range 0x%x - 0x%x",0, RelBlockIndex, rl[reg].startAddress, rl[reg].endAddress);
            gpLog_Flush();
            for(i=0; i < (rl[reg].endAddress - rl[reg].startAddress) + 1; i++)
            {
                gphal_dump_register(layout, RelBlockIndex, rl[reg].startAddress +i);
            }
        }
        else
        {
            GP_LOG_SYSTEM_PRINTF("register range 0x%x - 0x%x",0, rl[reg].startAddress, rl[reg].endAddress);
            gpLog_Flush();
            for(i=0; i < (rl[reg].endAddress - rl[reg].startAddress) + 1; i++)
            {
                gphal_dump_register(layout, RelBlockIndex, rl[reg].startAddress +i);
            }
        }
    }

}

void gphal_dump_allregisters(void)
{
    UInt16 layout;
    UInt16 RelBlockIndex = 0;
    GP_LOG_SYSTEM_PRINTF("Dumping",0);
    gpLog_Flush();
    for(layout = 0; layout < (sizeof(layoutlist) / sizeof(struct rangedescription)); layout++)
    {
        struct rangelist*  rl = layoutlist[layout].rlp;
        if(rl == pbm_format_t)
        {
            for(RelBlockIndex = 0; RelBlockIndex < 6 ; RelBlockIndex ++)
            {
                gphal_dump_block(layout, RelBlockIndex);
            }
        }
        else if (rl == event)
        {
            for(RelBlockIndex = 0; RelBlockIndex < 5 ; RelBlockIndex ++)
            {
                gphal_dump_block(layout, RelBlockIndex);
            }
        }
        else
        {
            gphal_dump_block(layout, RelBlockIndex);
        }
    }
}

