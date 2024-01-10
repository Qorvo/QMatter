/*
 *   Copyright (c) 2018, Qorvo Inc
 *
 *   Extenal Storage
 *   Implementation of gpExtStorage
 *
 *   This software is owned by Qorvo Inc
 *   and protected under applicable copyright laws.
 *   It is delivered under the terms of the license
 *   and is intended and supplied for use solely and
 *   exclusively with products manufactured by
 *   Qorvo Inc.
 *
 *
 *   THIS SOFTWARE IS PROVIDED IN AN "AS IS"
 *   CONDITION. NO WARRANTIES, WHETHER EXPRESS,
 *   IMPLIED OR STATUTORY, INCLUDING, BUT NOT
 *   LIMITED TO, IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *   QORVO INC. SHALL NOT, IN ANY
 *   CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 *   INCIDENTAL OR CONSEQUENTIAL DAMAGES,
 *   FOR ANY REASON WHATSOEVER.
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID GP_COMPONENT_ID_EXTSTORAGE
//#define GP_LOCAL_LOG

#include "gpBsp.h"
#include "hal.h"
#include "gpLog.h"
#include "gpExtStorage.h"
#include "gpSched.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/** @brief define slave select pin no for flash */
#define FLASH_SPI_SSN_PIN   MSPI_GPIO_SSN //Coming from SPI storage definition BSP

#if !defined(GP_EXTSTORAGE_DIVERSITY_WRITE_DISABLE)
/* Status register - Write in progress bit */
#define STATUS_WIP  0x01

/* read status register, WIP bit is 1 during PAGE PROGRAM cycle, and 0 when the cycle is completed */
#define FLASH_WAIT_WRITE_READY() while (ExtStorage_FlashReadStatus() & STATUS_WIP) { HAL_WDT_RESET();}
#endif // GP_EXTSTORAGE_DIVERSITY_WRITE_DISABLE

#define SSN_HIGH    true
#define SSN_LOW     false


#define IDLE_POLLING_INTERVAL_MS   50

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static void ExtStorage_FlashSetSsn(Bool high);
static void ExtStorage_FlashSendAddress(UInt32 address);

#if !defined(GP_EXTSTORAGE_DIVERSITY_WRITE_DISABLE)

static UInt8 ExtStorage_FlashReadStatus(void);
static void ExtStorage_FlashWriteEnable(Bool enable);
#endif //GP_EXTSTORAGE_DIVERSITY_WRITE_DISABLE


/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/
/** @brief Function to select or deselect spi slave
 */
static void ExtStorage_FlashSetSsn(Bool high)
{
    if (high)
    {
        /* deselect slave */
        hal_gpioSet(GPIO_PIN(FLASH_SPI_SSN_PIN));
    }
    else
    {
        /* select slave */
        hal_gpioClr(GPIO_PIN(FLASH_SPI_SSN_PIN));
    }
}

#if !defined(GP_EXTSTORAGE_DIVERSITY_WRITE_DISABLE)
/**@brief Function to read device status register
 */
static UInt8 ExtStorage_FlashReadStatus(void)
{
    UInt8 status;

    /* select slave */
    ExtStorage_FlashSetSsn(SSN_LOW);

    /* send read status register command */
    hal_WriteReadSPI(CMD_RDSR);

    /* read status register */
    status = hal_WriteReadSPI(0);

    /* deslect slave */
    ExtStorage_FlashSetSsn(SSN_HIGH);

    return status;
}

/** @brief Function to enable or disable Write operation
 */
static void ExtStorage_FlashWriteEnable(Bool enable)
{
    /* select slave */
    ExtStorage_FlashSetSsn(SSN_LOW);

    /* Send WRITE_ENABLE if true else WRITE_DISABLE */
    if (enable)
    {
        hal_WriteReadSPI(CMD_WREN);
    }
    else
    {
        hal_WriteReadSPI(CMD_WRDI);
    }

    /* deselect slave */
    ExtStorage_FlashSetSsn(SSN_HIGH);
}
#endif // GP_EXTSTORAGE_DIVERSITY_WRITE_DISABLE

/** @brief Function to send 3 byte address
 */
static void ExtStorage_FlashSendAddress(UInt32 address)
{
    if (FLASH_ADDRESS_SIZE >= 3)
    {
        hal_WriteReadSPI((address >> 16) & 0xFF);
    }
    if (FLASH_ADDRESS_SIZE >= 2)
    {
        hal_WriteReadSPI((address >> 8) & 0xFF);
    }
    hal_WriteReadSPI(address & 0xFF);
}

static void ExtStorage_Init(void)
{
    /* configure slave select pin - output */
    hal_gpioModePP(GPIO_PIN(FLASH_SPI_SSN_PIN), 1);

    /* set SSN high (inactive state) for starters */
    ExtStorage_FlashSetSsn(true);

    /* Init spi - frequency - 1MHz, mode 0, msb first */
    hal_InitSPI(1000000UL, 0, false);

}

static void ExtStorage_DeInit(void)
{
    hal_DeInitSPI();
}

#if !defined(GP_EXTSTORAGE_DIVERSITY_WRITE_DISABLE)
static void ExtStorage_CheckIdle(void *pParam)
{
    if(ExtStorage_FlashReadStatus() & STATUS_WIP)
    {
        gpSched_ScheduleEventArg(1000*IDLE_POLLING_INTERVAL_MS, ExtStorage_CheckIdle, pParam);
    }
    else
    {
        if(pParam)
        {
            ((gpExtStorage_cbEraseComplete_t)pParam)();
        }
        ExtStorage_DeInit();
    }
}
#endif //GP_EXTSTORAGE_DIVERSITY_WRITE_DISABLE
/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

gpExtStorage_Result_t gpExtStorage_ReadBlock(UInt32 address, UInt32 length, UInt8* pData)
{
    UInt16 index;

    GP_LOG_PRINTF("Reading: addr = 0x%08lx, length = %lu", 0, (unsigned long int)address, (unsigned long int)length);


    ExtStorage_Init();

    /* select slave */
    ExtStorage_FlashSetSsn(SSN_LOW);

    /* send READ_DATABYTES command */
    hal_WriteReadSPI(CMD_READ);

    /* send 3 bytes address */
    ExtStorage_FlashSendAddress(address);


    /* read actual data byte to pData */
    for (index = 0; index < length; index++)
    {
        pData[index] = hal_WriteReadSPI(0);
    }


    /* deselect slave */
    ExtStorage_FlashSetSsn(SSN_HIGH);

    ExtStorage_DeInit();


    return gpExtStorage_Success;
}

#if !defined(GP_EXTSTORAGE_DIVERSITY_WRITE_DISABLE)
gpExtStorage_Result_t gpExtStorage_WriteBlock(UInt32 address, UInt32 length, UInt8* pData)
{
    UInt16 i, writeLen;

    GP_LOG_PRINTF("Writing: addr = 0x%08lx, length = %lu", 0, (unsigned long int)address, (unsigned long int)length);


    ExtStorage_Init();

    /* Flash can be programmed 1 to SPI_FLASH_PAGE_SIZE(256) bytes at a time using PAGE_PROGRAM command */

    while (length > 0)
    {
        /* Calculate length of data to write at a time in selected page */
        writeLen = min(length, SPI_FLASH_PAGE_SIZE - (address % SPI_FLASH_PAGE_SIZE));

        /* Enable write operation */
        ExtStorage_FlashWriteEnable(true);

        /* disable all interrupts */
        HAL_DISABLE_GLOBAL_INT();

        /* select slave */
        ExtStorage_FlashSetSsn(SSN_LOW);

        /* send PAGE_PROGRAM command */
        hal_WriteReadSPI(CMD_WRITE);

        /* send 3 bytes address */
        ExtStorage_FlashSendAddress(address);

        /* write data byte */
        for (i = 0; i < writeLen; i++)
        {
            hal_WriteReadSPI(pData[i]);
        }

        /* deselect slave */
        ExtStorage_FlashSetSsn(SSN_HIGH);

        /* re-enable interrupts */
        HAL_ENABLE_GLOBAL_INT();

        /* wait till write completes */
        FLASH_WAIT_WRITE_READY();

        /* adjust length, address */
        length  -= writeLen;
        address += writeLen;
        /* point write buffer to new location */
        pData   += writeLen;

    }

    ExtStorage_DeInit();

    return gpExtStorage_Success;
}

gpExtStorage_Result_t gpExtStorage_Erase(void)
{
    ExtStorage_Init();

    /* Before BULK_ERASE command, WRITE_ENABLE command must be executed */
    ExtStorage_FlashWriteEnable(true);

    /* select slave */
    ExtStorage_FlashSetSsn(SSN_LOW);

    /* send BULK_ERASE command */
    hal_WriteReadSPI(CMD_BE);

    /* deselect slave */
    ExtStorage_FlashSetSsn(SSN_HIGH);

    /* wait till erase completes */
    FLASH_WAIT_WRITE_READY();

    ExtStorage_DeInit();

    return gpExtStorage_Success;
}

gpExtStorage_Result_t gpExtStorage_EraseNoBlock(gpExtStorage_cbEraseComplete_t cb)
{
    ExtStorage_Init();

    /* Before BULK_ERASE command, WRITE_ENABLE command must be executed */
    ExtStorage_FlashWriteEnable(true);

    /* select slave */
    ExtStorage_FlashSetSsn(SSN_LOW);

    /* send BULK_ERASE command */
    hal_WriteReadSPI(CMD_BE);

    /* deselect slave */
    ExtStorage_FlashSetSsn(SSN_HIGH);

    /* schedule event to poll WIP bit in status register */
    ExtStorage_CheckIdle((void *)cb);

    return gpExtStorage_Success;
}

gpExtStorage_Result_t gpExtStorage_EraseSectorNoBlock(uint32_t address, gpExtStorage_cbEraseComplete_t cb)
{
    ExtStorage_Init();

    /* Before BULK_ERASE command, WRITE_ENABLE command must be executed */
    ExtStorage_FlashWriteEnable(true);

    /* select slave */
    ExtStorage_FlashSetSsn(SSN_LOW);

    /* send SECTOR_ERASE command */
    hal_WriteReadSPI(CMD_SE);

    /* send address withing sector to erase  */
    hal_WriteReadSPI(0xFF & (address >> 16));
    hal_WriteReadSPI(0xFF & (address >> 8));
    hal_WriteReadSPI(0xFF & address);

    /* deselect slave */
    ExtStorage_FlashSetSsn(SSN_HIGH);

    /* schedule event to poll WIP bit in status register */
    ExtStorage_CheckIdle((void *)cb);

    return gpExtStorage_Success;
}

gpExtStorage_Result_t gpExtStorage_EraseSector(uint32_t address)
{
    ExtStorage_Init();

    /* Before BULK_ERASE command, WRITE_ENABLE command must be executed */
    ExtStorage_FlashWriteEnable(true);

    /* select slave */
    ExtStorage_FlashSetSsn(SSN_LOW);

    /* send SECTOR_ERASE command */
    hal_WriteReadSPI(CMD_SE);

    /* send address withing sector to erase  */
    hal_WriteReadSPI(0xFF & (address >> 16));
    hal_WriteReadSPI(0xFF & (address >> 8));
    hal_WriteReadSPI(0xFF & address);

    /* deselect slave */
    ExtStorage_FlashSetSsn(SSN_HIGH);

    /* wait till erase completes */
    FLASH_WAIT_WRITE_READY();

    return gpExtStorage_Success;
}

#endif //GP_EXTSTORAGE_DIVERSITY_WRITE_DISABLE
