/*
 * Copyright (c) 2023, Qorvo Inc
 *
 * gpHal_TrimRxMode.h
 *
 * This is a header of file contains algorithm that changes rx mode based on temperature
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
 *
 */

#ifndef _GPHAL_TRIM_RX_MODE_H_
#define _GPHAL_TRIM_RX_MODE_H_

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

/** @brief Initialization of the Low Temperature mode switching algorithm
 *
 */
void gpHal_RxModeSwitchAlgoInit(void);

/** @brief Check if the chip is in the Low Temperature mode
 *
 *  @return isLowTemperature
 */
Bool gpHal_IsLowTemp(void);

#endif /* _GPHAL_TRIM_RX_MODE_H_ */
