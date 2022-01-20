/*
 * Copyright (c) 2015-2016, GreenPeak Technologies
 * Copyright (c) 2021, Qorvo Inc
 *
 *   Hardware Abstraction Layer for ARM-based devices.
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
 * $Header: //depot/release/Embedded/Components/Qorvo/HAL_PLATFORM/v2.10.2.1/comps/halCortexM4/k8e/src/hal_CLK.c#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "hal.h"
#include "hal_defs.h"
#include "gpAssert.h"

/*****************************************************************************
 *                    CLK frequency
 *****************************************************************************/

void hal_SetMcuClockSpeed(UInt8 clockSpeed)
{
    if(clockSpeed > GP_WB_ENUM_CLOCK_SPEED_M32)
    {
        // Invalid clock speed
        GP_ASSERT_SYSTEM(false);
    }

#ifdef GP_HAL_EXPECTED_CHIP_EMULATED
    /* assert M32, because GP_WB_ENUM_CLOCK_SPEED_M64 is not supported on the FPGA */
    GP_ASSERT_DEV_INT(clockSpeed == GP_WB_ENUM_CLOCK_SPEED_M32);
#endif /* GP_HAL_EXPECTED_CHIP_EMULATED */

    GP_WB_WRITE_STANDBY_PRESCALE_UCCORE(clockSpeed);
}

UInt8 hal_GetMcuClockSpeed(void)
{
    return GP_WB_READ_STANDBY_PRESCALE_UCCORE();
}
