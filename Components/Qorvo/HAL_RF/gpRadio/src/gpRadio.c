/*
 * Copyright (c) 2020-2023, Qorvo Inc
 *
 *   Radio
 *   Implementation of gpRadio
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

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_RADIO

#include "gpRadio.h"

#ifdef GP_RADIO_DIVERSITY_GPCOM_SERVER
#include "gpRadio_server.h"
#endif /* GP_RADIO_DIVERSITY_GPCOM_SERVER */
#include "gpAssert.h"
#include "gpLog.h"
#include "gpHal.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/** @brief The default RX antenna used when AD mode is disabled.
 *
 *  Antenna Diversity (AD mode) will be disabled when using multi-standard mode.
 *  Selects the default antenna to use in that case.
 **/
#ifndef GP_RADIO_DEFAULT_NO_AD_RX_ANTENNA
#define GP_RADIO_DEFAULT_NO_AD_RX_ANTENNA gpHal_AntennaSelection_Ant0
#endif //GP_RADIO_DEFAULT_NO_AD_RX_ANTENNA

#ifndef GP_RADIO_DEFAULT_RX_MODE
#if defined(GP_RADIO_DIVERSITY_ENABLE_MULTISTANDARD_LISTENING_MODE)
#define GP_RADIO_DEFAULT_RX_MODE /*enableMultiStandard = */ true, /* enableMultiChannel = */ false, /* enableHighSensitivity = */ false
#else
#if(GP_DIVERSITY_NR_OF_STACKS > 1)
// Enable multi-channel by default
#define GP_RADIO_DEFAULT_RX_MODE /*enableMultiStandard = */ false, /* enableMultiChannel = */ true, /* enableHighSensitivity = */ false
#else
#define GP_RADIO_DEFAULT_RX_MODE /*enableMultiStandard = */ false, /* enableMultiChannel = */ false, /* enableHighSensitivity = */ false
#endif // (GP_DIVERSITY_NR_OF_STACKS > 1)
#endif //defined(GP_HAL_DIVERSITY_MULTISTANDARD_RX_MODE)
#endif //GP_RADIO_DEFAULT_RX_MODE

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/** @brief Internal helper structure with all RxMode options */
typedef struct {
    Bool enableMultiStandard;
    Bool enableMultiChannel;
    Bool enableHighSensitivity;
} gpRadio_RxMode_t;

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpRadio_Init(void)
{
    const gpRadio_RxMode_t defaultSettings = {GP_RADIO_DEFAULT_RX_MODE};

    gpRadio_SetRxMode(defaultSettings.enableMultiStandard, defaultSettings.enableMultiChannel, defaultSettings.enableHighSensitivity);

    if(defaultSettings.enableMultiStandard)
    {
        gpHal_SetRxAntenna(GP_RADIO_DEFAULT_NO_AD_RX_ANTENNA);
    }
    else
    {
        gpHal_SetRxAntenna(gpHal_AntennaSelection_Auto);
    }

#ifdef GP_RADIO_DIVERSITY_GPCOM_SERVER
    /* Init the serial wrapper */
    gpRadio_InitServer();
#endif /* GP_RADIO_DIVERSITY_GPCOM_SERVER */
}

gpRadio_Status_t gpRadio_SetRxMode(Bool enableMultiStandard, Bool enableMultiChannel, Bool enableHighSensitivity)
{
#if defined(GP_COMP_GPHAL_MAC)
    if(gpHal_SetMacRxMode(enableMultiStandard, enableMultiChannel, enableHighSensitivity) != gpHal_ResultSuccess)
    {
        return gpRadio_StatusInvalidParameter;
    }
#endif //defined(GP_COMP_GPHAL_MAC)
#if defined(GP_COMP_GPHAL_BLE)
    if(gpHal_BleSetMultiStandard(enableMultiStandard) != gpHal_ResultSuccess)
    {
        return gpRadio_StatusInvalidParameter;
    }
#endif //defined(GP_COMP_GPHAL_BLE)
    return gpRadio_StatusSuccess;
}

gpRadio_Status_t gpRadio_GetRxMode(Bool* enableMultiStandard, Bool* enableMultiChannel, Bool* enableHighSensitivity)
{
    *enableMultiStandard = false;
    *enableMultiChannel = false;
    *enableHighSensitivity = false;
#if defined(GP_COMP_GPHAL_MAC)
    gpHal_GetMacRxMode(enableMultiStandard, enableMultiChannel, enableHighSensitivity);
#endif //defined(GP_COMP_GPHAL_MAC)
#if defined(GP_COMP_GPHAL_BLE)
    *enableMultiStandard = gpHal_BleGetMultiStandard();
#endif //defined(GP_COMP_GPHAL_BLE)
    return gpRadio_StatusSuccess;
}

gpRadio_Status_t gpRadio_SetRxAntenna(gpRadio_AntennaSelection_t rxAntenna)
{
    switch(rxAntenna)
    {
        case gpRadio_AntennaSelection_PortRF1:
        {
            gpHal_SetRxAntenna(gpHal_AntennaSelection_Ant0);
            break;
        }
        case gpRadio_AntennaSelection_PortRF2:
        {
            gpHal_SetRxAntenna(gpHal_AntennaSelection_Ant1);
            break;
        }
        case gpRadio_AntennaSelection_Auto:
        {
            gpHal_SetRxAntenna(gpHal_AntennaSelection_Auto);
            break;
        }
        case gpRadio_AntennaSelection_Unknown:
        {
            gpHal_SetRxAntenna(gpHal_AntennaSelection_Unknown);
            break;
        }
        default:
        {
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }
    return gpRadio_StatusSuccess;
}

gpRadio_AntennaSelection_t gpRadio_GetRxAntenna(void)
{
    gpRadio_AntennaSelection_t antsel = gpRadio_AntennaSelection_Unknown;

    switch(gpHal_GetRxAntenna())
    {
        case gpHal_AntennaSelection_Ant0:
        {
            antsel = gpRadio_AntennaSelection_PortRF1;
            break;
        }
        case gpHal_AntennaSelection_Ant1:
        {
            antsel = gpRadio_AntennaSelection_PortRF2;
            break;
        }
        case gpHal_AntennaSelection_Auto:
        {
            antsel = gpRadio_AntennaSelection_Auto;
            break;
        }
        case gpHal_AntennaSelection_Unknown:
        {
            antsel = gpRadio_AntennaSelection_Unknown;
            break;
        }
        default:
        {
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }
    return antsel;
}

gpRadio_Status_t gpRadio_SetRadioFirFilter(gpRadio_FirFilter_t firFilter)
{
    switch(firFilter)
    {
        case gpRadio_FirFilter_None:
        {
            gpHal_SetRadioFirFilter(gpHal_FirFilter_None);
            break;
        }
        case gpRadio_FirFilter_FIR25:
        {
            gpHal_SetRadioFirFilter(gpHal_FirFilter_FIR25);
            break;
        }
        default:
        {
            return gpRadio_StatusNotImplemented;
        }
    }
    return gpRadio_StatusSuccess;
}
