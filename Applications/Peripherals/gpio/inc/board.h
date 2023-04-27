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

/** @file "board.h"
 *
 *  Expose board components as feature components to the application.
 *
 *  Note:
 *  This file is application specific, as each application requires unique
 *  features
*/

#ifndef _BOARD_H_
#define _BOARD_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include "global.h"
#include "gpBsp.h"

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> Macro */
/** @brief Map button configuration */

/**
*   Porting note:
*   Map your application features on buttons, exposed by the development board.
*/

/** @brief Macro to define switch/button numbers on the hardware */
#if   \
      defined(GP_DIVERSITY_QPG6105DK_B01)

    #define GP_APP_BOARD_BUTTON_LED_1                   GP_BSP_BUTTON_GP_PB2_PIN
    #define GP_APP_BOARD_BUTTON_LED_1_LOGIC_LEVEL       GP_BSP_BUTTON_GP_PB2_LOGIC_LEVEL
    #define GP_APP_BOARD_BUTTON_LED_2                   GP_BSP_BUTTON_GP_PB3_PIN
    #define GP_APP_BOARD_BUTTON_LED_2_LOGIC_LEVEL       GP_BSP_BUTTON_GP_PB3_LOGIC_LEVEL
    #define GP_APP_BOARD_BUTTON_LED_3                   GP_BSP_BUTTON_GP_PB1_PIN
    #define GP_APP_BOARD_BUTTON_LED_3_LOGIC_LEVEL       GP_BSP_BUTTON_GP_PB1_LOGIC_LEVEL

#else
#error  "Define Button/Switch number for your board configuration"
#endif


/** @brief Map LED configuration */
/**
*   Porting note:
*   Map your application features on LEDs, exposed by the development board.
*/

/** @brief Macro to define switch/button numbers on the hardware */
#if   \
      defined(GP_DIVERSITY_QPG6105DK_B01)

    #define GP_APP_BOARD_LED_1                      GP_BSP_LED_RED_PIN
    #define GP_APP_BOARD_LED_1_LOGIC_LEVEL          GP_BSP_LED_RED_LOGIC_LEVEL
    #define GP_APP_BOARD_LED_2                      17 /*GPIO 17*/
    #define GP_APP_BOARD_LED_2_LOGIC_LEVEL          1  /*Active high*/
    #define GP_APP_BOARD_LED_3                      18 /*GPIO 18*/
    #define GP_APP_BOARD_LED_3_LOGIC_LEVEL          1  /*Active high*/

#else
#error  "Define LED ids for your board configuration"
#endif

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/
/** @brief Functional macro's to deinitialize the pin usage that is automatically
 *         configured via gpBaseComps_StackInit(), by calling the BSP functional macro's:
 *         - HAL_LED_INIT_LEDS()
 *         - HAL_BTN_INIT_BTNS()
 *
 *         No need to undo all configurations, only those that would interfere with
 *         standard GPIO functionality (like peripheral mappings) for the buttons and LEDs
 */
/** @brief Placeholder for BSP that require no specific deinitialization of the gpio pins connected to a button*/
#define HAL_BTN_DEINIT_BTNS()
/** @brief Placeholder for BSP that require no specific deinitialization of the gpio pins used to drive an LED*/
#define HAL_LED_DEINIT_LEDS()

#if   \
      defined(GP_DIVERSITY_QPG6105DK_B01)

    #undef HAL_LED_DEINIT_LEDS
    #define HAL_LED_DEINIT_LEDS()                         do{ \
        /*Disable the alternate use for the pin used by LED WHITE_COOL*/ \
        GP_WB_WRITE_IOB_GPIO_17_ALTERNATE_ENABLE(0); \
        /*Disable the alternate use for the pin used by LED WHITE_WARM*/ \
        GP_WB_WRITE_IOB_GPIO_18_ALTERNATE_ENABLE(0); \
    }while(0)

#endif

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/



/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_BOARD_H_

