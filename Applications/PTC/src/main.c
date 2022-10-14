/*
 * Copyright (c) 2016, GreenPeak Technologies
 * Copyright (c) 2017, Qorvo Inc
 *
 * main.c
 *
 *  The file contains the functions used to do testing of the RF performance
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

 /*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID GP_COMPONENT_ID_APP
#include "gpPTC.h"
#include "gpBsp.h"
#include "gpTest.h"
#include "gpHal.h"
#include "gpSched.h"
#include "gpBaseComps.h"
#if defined(GP_COMP_GPHAL_BLE)
#include "gpBleComps.h"
#endif
#include "gpCom.h"
#ifdef GP_DIVERSITY_LOG
#include "gpLog.h"
#endif
#include "main.h"

 /*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/
#define PTC_PAN_ID                    (UInt16)0xCAFE
#define PTC_RFCOMMUNICATION_CHANNEL    17
#define PTC_SHORT_ADDRESS             (UInt16)0xFC7C

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/
void EnableSPIInterface(void);
void Application_Init( void );
void CommonInit(void);
static Bool IncrementalEchoCommand(UInt8 dataLenIn, UInt8* pDataIn, UInt8* pDataLenOut, UInt8* pDataOut);
static Bool Execute32KhzTest(UInt8 dataLenIn, UInt8* pDataIn, UInt8* pDataLenOut, UInt8* pDataOut);


/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

void heartbeat(void)
{
    HAL_LED_SET(RED);
    HAL_LED_SET(GRN);
#ifdef GP_DIVERSITY_LOG
    GP_LOG_SYSTEM_PRINTF("hb !!",0);
#endif

    HAL_LED_CLR(GRN);
    HAL_LED_CLR(RED);
    gpSched_ScheduleEvent(10000000,heartbeat);
}

void EnableSPIInterface(void)
{
}

void Application_Init( void )
{
#if defined(GP_PTC_SPI_DUMP)
    EnableSPIInterface();
#endif /* GP_PTC_SPI_DUMP */

    gpBaseComps_StackInit();

#if defined(GP_COMP_GPHAL_BLE)
    gpBleComps_StackInit();
#endif

    gpPTC_Init();
    /* workaround for SW-7826, can be removed when basecomps has this fix? */
#if defined(GP_COMP_GPHAL_BLE)
    gpHal_BleSetTxPower(GPHAL_MAX_TRANSMIT_POWER);
#endif

    gpSched_ScheduleEvent(0, CommonInit);
    gpSched_ScheduleEvent(10000000,heartbeat);
    gpPTC_RegisterCustomCommand(0, IncrementalEchoCommand);
    gpPTC_RegisterCustomCommand(1, Execute32KhzTest);

}


void CommonInit(void)
{
   // gpTest_Start();

}


/**
 *  @brief Function that increments the input byte array and outputs this
 *
 * @param dataLenIn Length of the input byte array
 * @param pDataIn pointer to the input byte array
 * @param pDataLenOut pointer to the output byte array (defined by the functionality)
 * @param pDataOut pointer to the output byte array
*/
Bool IncrementalEchoCommand(UInt8 dataLenIn, UInt8* pDataIn, UInt8* pDataLenOut, UInt8* pDataOut)
{
  UIntLoop i;

  /* Return the same data with each byte incremented by 1,
  except for the commandid */
  *pDataLenOut = dataLenIn-1;
  for(i = 1; i < dataLenIn; i++)
      pDataOut[i-1] = pDataIn[i]+1;
  return true;
}

/*
* Implementation of the 32kHz crystal test
* The returned data is the result of this test
 * @param dataLenIn Length of the input byte array
 * @param pDataIn pointer to the input byte array
 * @param pDataLenOut pointer to the output byte array (defined by the functionality)
 * @param pDataOut pointer to the output byte array
*/
Bool Execute32KhzTest(UInt8 dataLenIn, UInt8* pDataIn, UInt8* pDataLenOut, UInt8* pDataOut)
{
  return gpPTC_Execute32KhzTest(dataLenIn,pDataIn,pDataLenOut,pDataOut);
}
