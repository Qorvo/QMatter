/*
 * Copyright (c) 2017, Qorvo Inc
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

 /** @file mtwi.c
 *
 * TWI example application
 * This example shows an example of TWI communication.
 * This example access humidity sensor on SDK board through TWI - i2c interface.
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "hal.h"
#include "gpSched.h"
#include "gpHal.h"
#include "gpBaseComps.h"
#include "gpLog.h"

#include "app_common.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_APP

/** @brief Blinking led timing set to 50ms on every 5s */
#define LED_DELAY_MS            50 //ms - 50ms
#define LED_PATTERN_PERIOD_US   5000000 //us - 5s

/** @brief Define i2c address of humidity sensor on sdk board(Si7020) */
#define I2C_SLAVE_ADDRESS  0x40
#define I2C_DEVICE_ADDRESS  (I2C_SLAVE_ADDRESS << 1)

/** @brief Define i2c command codes */
#define CMD_HUMIDITY        0xF5
#define CMD_TEMPERATURE     0xE0
#define CMD_RESET           0xFE
#define CMD_ELE_ID1_ONE     0xFA
#define CMD_ELE_ID1_TWO     0x0F
#define CMD_ELE_ID2_ONE     0xFC
#define CMD_ELE_ID2_TWO     0xC9
#define CMD_FIRM_REV_ONE    0x84
#define CMD_FIRM_REV_TWO    0xB8

/** @brief Define application definitions */
#define APP_MEASUREMENT_PERIOD      5000000 //us - 5s
#define APP_MAX_RELATIVE_HUMIDITY   100 //%

/*****************************************************************************
 *                    Test Function Definitions
 *****************************************************************************/


void Application_ReadDevInfo(void)
{
    UInt8 txBuffer[2];
    UInt8 rxBuffer[16];

    txBuffer[0] = CMD_RESET;
    if(hal_WriteReadTWI(I2C_DEVICE_ADDRESS, 1,txBuffer, 0, rxBuffer))
    {
       GP_LOG_SYSTEM_PRINTF("Reset Done",0);
       /* Wait time after reset */
       HAL_WAIT_MS(15);
    }
    else
    {
        /* a TWI access failed */
        GP_ASSERT_SYSTEM(false);
    }

    /* Write read firmware revision command code on i2c bus */
    txBuffer[0] = CMD_FIRM_REV_ONE;
    txBuffer[1] = CMD_FIRM_REV_TWO;

    /* read back the firmware revision value */
    if(hal_WriteReadTWI(I2C_DEVICE_ADDRESS, 2,txBuffer, 1, rxBuffer))
    {
       /* expected firmware revision for Si7020 is : 0x20, 0xFF */
       GP_LOG_SYSTEM_PRINTF("Firmware Revision %x",0, rxBuffer[0]);
    }
    else
    {
        /* a TWI access failed */
        GP_ASSERT_SYSTEM(false);
    }

    /* Read electronic serial number */

    /* Read back the first part of the serial number */
    txBuffer[0] = CMD_ELE_ID1_ONE;
    txBuffer[1] = CMD_ELE_ID1_TWO;
    if(!hal_WriteReadTWI(I2C_DEVICE_ADDRESS, 2,txBuffer, 8, rxBuffer) )
    {
        /* a TWI access failed */
        GP_ASSERT_SYSTEM(false);
    }

    /* Read back the second part of the serial number */
    txBuffer[0] = CMD_ELE_ID2_ONE;
    txBuffer[1] = CMD_ELE_ID2_TWO;
    if(!hal_WriteReadTWI(I2C_DEVICE_ADDRESS, 2,txBuffer, 8, &rxBuffer[8]) )
    {
        /* a TWI access failed */
        GP_ASSERT_SYSTEM(false);
    }

    GP_LOG_SYSTEM_PRINTF("Serial number: %x:%x:%x:%x:%x:%x:%x:%x",0, rxBuffer[0], rxBuffer[2], rxBuffer[4], rxBuffer[6],
                                                                     rxBuffer[8], rxBuffer[10], rxBuffer[12], rxBuffer[14]);
}

void Application_ReadHumiditySensor(void)
{
    UInt8 txBuffer[1];
    UInt8 rxBuffer[2];

    /* Use polled ack method to read the current humidity */

    txBuffer[0] = CMD_HUMIDITY;

    if(!hal_PolledAckWriteReadTWI(I2C_DEVICE_ADDRESS, 1, txBuffer, 2, rxBuffer))
    {
        /* a TWI access failed */
        GP_ASSERT_SYSTEM(false);
    }
    else
    {
        UInt16 relativeHumidityCode = (rxBuffer[0] << 8) | rxBuffer[1];

        /* See Si7020-A20 datasheet for humidity calculation */
        UInt8 relativeHumidityPercentage = (UInt8)((UInt32)(((125UL * relativeHumidityCode) / 65536UL) - 6UL));

        /* Truncate at 100% */
        if(relativeHumidityPercentage > APP_MAX_RELATIVE_HUMIDITY)
        {
            relativeHumidityPercentage = APP_MAX_RELATIVE_HUMIDITY;
        }

        GP_LOG_SYSTEM_PRINTF("Relative Humidity: %i Percent", 0, relativeHumidityPercentage);
    }

    /* Read the temperature measured at the previous humidity measurement */
    txBuffer[0] = CMD_TEMPERATURE;

    if(!hal_WriteReadTWI(I2C_DEVICE_ADDRESS, 1, txBuffer, 2, rxBuffer))
    {
        /* a TWI access failed */
        GP_ASSERT_SYSTEM(false);
    }
    else
    {
        UInt16 temperatureCode = (rxBuffer[0] << 8) | rxBuffer[1];

        /* See Si7020-A20 datasheet for temperature calculation*/
        UInt16 temperature = (UInt16)((UInt32)((((17572UL * (temperatureCode)) / 65536UL) - 4685UL)/10));

        GP_LOG_SYSTEM_PRINTF("Temperature:       %i.%i Degrees Celcius", 0, temperature/10, temperature%10);
    }

    gpSched_ScheduleEvent(APP_MEASUREMENT_PERIOD, Application_ReadHumiditySensor);

}

 /** @brief Generate a square waveform by toggling the IO
*/
void Application_LedPattern(void)
{
    /* Blink an LED */
    LED_INDICATOR_ON();
    HAL_WAIT_MS(LED_DELAY_MS);
    LED_INDICATOR_OFF();

    /* schedule event */
    gpSched_ScheduleEvent(LED_PATTERN_PERIOD_US, Application_LedPattern);
}

/*****************************************************************************
 *                    Application Init
 *****************************************************************************/

/** @brief Initialize application
*/

void Application_Init(void)
{
    /* Initialize stack */
    gpBaseComps_StackInit();

    /* Init twi */
    hal_InitTWI();

    /* show led pattern */
    gpSched_ScheduleEvent(0, Application_LedPattern);

    /* read device information - firmware revision & serial number */
    gpSched_ScheduleEvent(0, Application_ReadDevInfo);

    /* read humidity and temperature */
    gpSched_ScheduleEvent(0, Application_ReadHumiditySensor);
}
