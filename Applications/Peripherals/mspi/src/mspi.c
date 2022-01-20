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
 * $Header: //depot/release/Embedded/Applications/R005_PeripheralLib/v1.3.2.1/apps/mspi/src/mspi.c#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

 /** @file mspi.c
 *
 * Master SPI example application
 * This example shows configuration of SPI master to read/write/erase flash on the DB09 development board.
 * This example demonstrates write or read single byte, multiple bytes to or from flash, erase flash operations.
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "hal.h"
#include "gpHal.h"
#include "gpBaseComps.h"
#include "gpLog.h"
#include "gpCom.h"

#include "gpBsp.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_APP

/** @brief define slave select pin no for flash */
#define FLASH_SPI_SSN_PIN   MSPI_GPIO_SSN

/* Status register - Write in progress bit */
#define STATUS_WIP  0x01

/* read status register, WIP bit is 1 during PAGE PROGRAM cycle, and 0 when the cycle is completed */
#define FLASH_WAIT_WRITE_READY() while(Flash_ReadStatus() & STATUS_WIP){}

/* Test definitions */
/** @brief Define no of bytes to read and write */
#define TEST_NUM_BYTES   256

/** @brief Define flash size block to test - each page is 256 bytes */
#define TEST_FLASH_BLOCK_SIZE  (256 * 1024) // Bytes

/* SPI flash identification codes */
#define SPIFLASH_MANUFACTURER_ID_ADESTO     0x1F
#define SPIFLASH_DEVICE_ID_AT25SF081        0x85
#define SPIFLASH_CAPACITY_ID_AT25SF081      0x01

#define SPIFLASH_MANUFACTURER_ID_MICROCHIP  0x20
#define SPIFLASH_DEVICE_ID_M25P80           0x20
#define SPIFLASH_CAPACITY_ID_M25P80         0x14

#define SPIFLASH_MANUFACTURER_ID_AMIC       0x37
#define SPIFLASH_DEVICE_ID_A25L080          0x30
#define SPIFLASH_CAPACITY_ID_A25L080        0x14


/*****************************************************************************
 *                    Static global variables
 *****************************************************************************/

static UInt32 App_NrOfErrors;

/*****************************************************************************
 *                    SPI Flash Functions
 *****************************************************************************/

/** @brief Function to select or deselect spi slave */
void Flash_setSsn(Bool high)
{
    HAL_WAIT_US(1);

    if(high)
    {
        /* deselect slave */
        hal_gpioSet(gpios[FLASH_SPI_SSN_PIN]);
    }
    else
    {
        /* select slave */
        hal_gpioClr(gpios[FLASH_SPI_SSN_PIN]);
    }
}

/** @brief Function to read flash device identification data */
void Flash_ReadId(UInt8* id, UInt8* type, UInt8* cap)
{
    /* select slave */
    Flash_setSsn(0);

    /* send read_identification command */
    hal_WriteReadSPI(CMD_RDID);

#ifdef APP_DIVERSITY_WAIT_SPI_RX_READY
    /* add padding for waiting slave ready */
    hal_WriteReadSPI(0);
#endif

    /* read manufacture id */
    (*id) = hal_WriteReadSPI(0);

    /* read memory type */
    (*type) = hal_WriteReadSPI(0);

    /* read memory capacity */
    (*cap) = hal_WriteReadSPI(0);

    /* deselect slave */
    Flash_setSsn(1);
}

/**@brief Function to read device status register */
UInt8 Flash_ReadStatus(void)
{
    UInt8 status;

    /* select slave */
    Flash_setSsn(0);

    /* send read status register command */
    hal_WriteReadSPI(CMD_RDSR);

#ifdef APP_DIVERSITY_WAIT_SPI_RX_READY
    /* add padding for waiting slave ready */
    hal_WriteReadSPI(0);
#endif

    /* read status register */
    status = hal_WriteReadSPI(0);

    /* deslect slave */
    Flash_setSsn(1);

    return status;
}

/** @brief Function to enable or disable Write operation */
void Flash_WriteEnable(Bool enable)
{
    /* selece slave */
    Flash_setSsn(0);

    /* Send WRITE_ENABLE if true else WRITE_DISABLE */
    hal_WriteReadSPI(enable ? CMD_WREN : CMD_WRDI);

    /* deselect slave */
    Flash_setSsn(1);
}

/** @brief Function to send 3 byte address */
void Flash_SendAddress(UInt32 address)
{
    if(FLASH_ADDRESS_SIZE >= 3)
    {
        hal_WriteReadSPI((address >> 16) & 0xFF);
    }
    if(FLASH_ADDRESS_SIZE >= 2)
    {
        hal_WriteReadSPI((address >> 8) & 0xFF);
    }
    hal_WriteReadSPI(address & 0xFF);
}

/** @brief Function to write a single byte - page program */
void Flash_WriteByte(UInt32 address, UInt8 value)
{
    /* Before PAGE_PROGRAM command, WRITE_ENABLE command must be executed */
    Flash_WriteEnable(true);

    /* select slave */
    Flash_setSsn(0);

    /* send PAGE_PROGRAM command code */
    hal_WriteReadSPI(CMD_WRITE);

    /* send address */
    Flash_SendAddress(address);

    /* send value to write */
    hal_WriteReadSPI(value);

    /* deselect slave */
    Flash_setSsn(1);

    /* check status register for WIP bit */
    FLASH_WAIT_WRITE_READY();
}

/** @brief Function to read data byte */
UInt8 Flash_ReadByte(UInt32 address)
{
    UInt8 value;

    /* select slave */
    Flash_setSsn(0);

    /* send READ DATA BYTES command */
    hal_WriteReadSPI(CMD_READ);

    /* send address to read data */
    Flash_SendAddress(address);

#ifdef APP_DIVERSITY_WAIT_SPI_RX_READY
    /* add padding for waiting slave ready */
    hal_WriteReadSPI(0);
#endif

    /* read data */
    value = hal_WriteReadSPI(0);

    /* deselect slave */
    Flash_setSsn(1);

    return value;
}

/** @brief Function to erase flash - Sector erase */
void Flash_SectorErase(UInt32 address)
{
    /* Before SECTOR_ERASE command, WRITE_ENABLE command must be executed */
    Flash_WriteEnable(true);

    /* select slave */
    Flash_setSsn(0);

    /* send BULK_ERASE command */
    hal_WriteReadSPI(CMD_SE);

    /* send address to read data */
    Flash_SendAddress(address);

    /* deselect slave */
    Flash_setSsn(1);

    /* wait till erase completes */
    FLASH_WAIT_WRITE_READY();
}

/** @brief Function to erase flash - Bulk erase */
void Flash_BulkErase(void)
{
    /* Before BULK_ERASE command, WRITE_ENABLE command must be executed */
    Flash_WriteEnable(true);

    /* select slave */
    Flash_setSsn(0);

    /* send BULK_ERASE command */
    hal_WriteReadSPI(CMD_BE);

    /* deselect slave */
    Flash_setSsn(1);

    /* wait till erase completes */
    FLASH_WAIT_WRITE_READY();
}

/** @brief Function to write multiple bytes */
void Flash_WriteBlock(UInt32 address, UInt16 length, UInt8* txBuffer)
{
    UInt16 i, writeLen;

    /* Flash can be programmed 1 to SPI_FLASH_PAGE_SIZE(256) bytes at a time using PAGE_PROGRAM command */

    while(length>0)
    {
        /* Calculate length of data to write at a time in selected page */
        writeLen = min(length, SPI_FLASH_PAGE_SIZE - (address % SPI_FLASH_PAGE_SIZE));

        /* Enable write operation */
        Flash_WriteEnable(true);

        /* disable all interrupts */
        HAL_DISABLE_GLOBAL_INT();

        /* select slave */
        Flash_setSsn(0);

        /* send PAGE_PROGRAM command */
        hal_WriteReadSPI(CMD_WRITE);

        /* send 3 bytes address */
        Flash_SendAddress(address);

        /* write data bytes */
        for(i = 0; i < writeLen; i++)
        {
            hal_WriteReadSPI(txBuffer[i]);
        }

        /* deselect slave */
        Flash_setSsn(1);

        /* re-enable intrrupts */
        HAL_ENABLE_GLOBAL_INT();

        /* wait till write completes */
        FLASH_WAIT_WRITE_READY();

        /* adjust length, address */
        length -= writeLen;
        address += writeLen;
        /* point write buffer to new location */
        txBuffer += writeLen;
    }
}

/** @brief This function reads multiple data */
void Flash_ReadBlock(UInt32 address, UInt8* rxBuffer)
{
    UInt16 i;

    /* select slave */
    Flash_setSsn(0);

    /* send READ_DATABYTES command */
    hal_WriteReadSPI(CMD_READ);

    /* send 3 bytes address */
    Flash_SendAddress(address);

#ifdef APP_DIVERSITY_WAIT_SPI_RX_READY
    /* add padding for waiting slave ready */
    hal_WriteReadSPI(0);
#endif

    /* read data byte to rxbuffer */
    for(i = 0; i < TEST_NUM_BYTES; i++)
    {
        rxBuffer[i] = hal_WriteReadSPI(0);
    }

    /* deselect slave */
    Flash_setSsn(1);
}

/*****************************************************************************
 *                    Test Function Definitions
 *****************************************************************************/

/** @brief Function indicate something went wrong using the red led */
void indicateErrors(void)
{
    UInt32 i;
    for (i=0;i<App_NrOfErrors;i++)
    {
        HAL_LED_SET_RED();
        HAL_WAIT_MS(200);
        HAL_LED_CLR_RED();
        HAL_WAIT_MS(200);
    }

    GP_LOG_SYSTEM_PRINTF("NrOfErrors=%lu", 0, (unsigned long)App_NrOfErrors);
    gpLog_Flush();
}

/** @brief Function to test status of the device */
void testStatus(void)
{
    UInt8 status=0;

    /* read status register */
    status = Flash_ReadStatus();
    if(status & (STATUS_WIP != 0))
    {
        GP_LOG_SYSTEM_PRINTF("ERROR: Flash program/read/erase is in progress, SR : 0x%x", 0, status);
        App_NrOfErrors++;
    }
    else
    {
        GP_LOG_SYSTEM_PRINTF("No operation is in progress, SR : 0x%x",0,status);
    }
}

/** @brief Function to test write/read single byte */
void testSingleByte(UInt32 address)
{
    static UInt8 writeByte = 0;
    UInt8 readByte;

    GP_LOG_SYSTEM_PRINTF("Test SPI single byte read/write at address 0x%lx", 0, (unsigned long)address);

    /* Read back after erase */
    readByte = Flash_ReadByte(address);
    if (readByte != 0xff)
    {
        GP_LOG_SYSTEM_PRINTF("ERROR: Erased byte has value 0x%x", 0, readByte);
        App_NrOfErrors++;
    }

    /* change writeByte value */
    writeByte +=5;

    /* write single byte */
    Flash_WriteByte(address, writeByte);

    /* read single byte */
    readByte = Flash_ReadByte(address);

    GP_LOG_SYSTEM_PRINTF("Test Single Byte, WriteByte : 0x%x, ReadByte : 0x%x",0, writeByte, readByte);
    if (readByte != writeByte)
    {
        GP_LOG_SYSTEM_PRINTF("ERROR: Data mismatch", 0);
        App_NrOfErrors++;
    }

}

/** @brief Function to read/write blocks of data */
void testBlock(UInt32 address)
{
     /** @brief Write Buffer */
    static UInt8 writeBuffer[TEST_NUM_BYTES]={0};

    /** @brief Read Buffer */
    static UInt8 readBuffer[TEST_NUM_BYTES]={0};

    GP_LOG_SYSTEM_PRINTF("Test SPI block(multiple bytes) read/write at address 0x%lx", 0, (unsigned long)address);

    /* Reset read buffer. */
    MEMSET(readBuffer, 0, sizeof(readBuffer));

    /* Read back after erase. */
    Flash_ReadBlock(address, readBuffer);

    /* Verify erased to 0xff. */
    for (UInt16 i = 0; i < TEST_NUM_BYTES; i++)
    {
        if (readBuffer[i] != 0xff)
        {
            GP_LOG_SYSTEM_PRINTF("ERROR: After erase got readBuffer[%u] = 0x%x", 0, i, readBuffer[i]);
            App_NrOfErrors++;
            break;
        }
    }

    /* Scramble data in writeBuffer */
    writeBuffer[0] += 11;
    writeBuffer[1] += 3;
    for (UInt16 i = 2; i < TEST_NUM_BYTES; i++)
    {
        writeBuffer[i] += writeBuffer[i-1] ^ writeBuffer[i-2];
    }

    /* write multiple bytes at address */
    Flash_WriteBlock(address, TEST_NUM_BYTES, writeBuffer);

    /* read multiple data bytes */
    Flash_ReadBlock(address, readBuffer);

    for(UInt16 i = 0; i < TEST_NUM_BYTES; i++)
    {
        if(writeBuffer[i] != readBuffer[i])
        {
            GP_LOG_SYSTEM_PRINTF("ERROR: writeBuffer[%u]=0x%x, readBuffer[%u]=0x%x", 0, i, writeBuffer[i], i, readBuffer[i]);
            App_NrOfErrors++;
            break;
        }
    }
}

/*****************************************************************************
 *                    Application Init
 *****************************************************************************/

/** @brief Initialize application */
void Application_Init(void)
{
    HAL_INIT();

    gpBaseComps_StackInit();

    GP_LOG_SYSTEM_PRINTF("=====================", 0);
    GP_LOG_SYSTEM_PRINTF("SPI master test application starting", 0);
    GP_LOG_SYSTEM_PRINTF("=====================", 0);
    gpLog_Flush();

    App_NrOfErrors = 0;

    /* configure slave select pin - output */
    hal_gpioModePP(gpios[FLASH_SPI_SSN_PIN], 1);

    // Deselect slave.
    Flash_setSsn(1);
    HAL_WAIT_MS(1);

    /* Init spi - frequency - 1MHz, mode 0, msb first */
    hal_InitSPI(1000000UL, 0, false);
}

/*****************************************************************************
 *                    Main function
 *****************************************************************************/

/** @brief Main function */
MAIN_FUNCTION_RETURN_TYPE main(void)
{
    /* Intialize application */
    Application_Init();

    UInt32 address=0;

    /* Wait until flash is ready */
    HAL_WAIT_US(2000);

    /* read flash device information data */
    UInt8 manufId = 0;
    UInt8 memoryType = 0;
    UInt8 memoryCapacity = 0;
    Flash_ReadId(&manufId, &memoryType, &memoryCapacity);

    GP_LOG_SYSTEM_PRINTF("=====================",0);
    GP_LOG_SYSTEM_PRINTF("Manufacturer Id 0x%x",0,manufId);
    GP_LOG_SYSTEM_PRINTF("Memory Type 0x%x",0,memoryType);
    GP_LOG_SYSTEM_PRINTF("Memory Capacity 0x%x",0,memoryCapacity);

    if (manufId == SPIFLASH_MANUFACTURER_ID_ADESTO)
    {
        GP_LOG_SYSTEM_PRINTF("Recognized manufacturer Adesto", 0);
        if (memoryType == SPIFLASH_DEVICE_ID_AT25SF081 && memoryCapacity == SPIFLASH_CAPACITY_ID_AT25SF081)
        {
            GP_LOG_SYSTEM_PRINTF("Recognized device AT25SF081", 0);
        }
    }
    else if (manufId == SPIFLASH_MANUFACTURER_ID_MICROCHIP)
    {
        GP_LOG_SYSTEM_PRINTF("Recognized manufacturer Microchip", 0);
        if (memoryType == SPIFLASH_DEVICE_ID_M25P80 && memoryCapacity == SPIFLASH_CAPACITY_ID_M25P80)
        {
            GP_LOG_SYSTEM_PRINTF("Recognized device M25P80", 0);
        }
    }
    else if (manufId == SPIFLASH_MANUFACTURER_ID_AMIC)
    {
        GP_LOG_SYSTEM_PRINTF("Recognized manufacturer Amic", 0);
        if (memoryType == SPIFLASH_DEVICE_ID_A25L080 && memoryCapacity == SPIFLASH_CAPACITY_ID_A25L080)
        {
            GP_LOG_SYSTEM_PRINTF("Recognized device A25L080", 0);
        }
    }
    else
    {
        GP_LOG_SYSTEM_PRINTF("WARNING: Manufacturer ID not recognized", 0);
    }

    GP_LOG_SYSTEM_PRINTF("=====================",0);
    gpLog_Flush();

    do
    {
        HAL_LED_SET_GRN();
        HAL_WAIT_MS(50);
        HAL_LED_CLR_GRN();

        GP_LOG_SYSTEM_PRINTF("=====================",0);
        GP_LOG_SYSTEM_PRINTF("TESTING MSPI - FLASH",0);
        GP_LOG_SYSTEM_PRINTF("=====================",0);

        /* test status of the flash */
        testStatus();
        HAL_WAIT_MS(10);

        /* cycle through address */
        address += 2 * SPI_FLASH_PAGE_SIZE;
        address %= TEST_FLASH_BLOCK_SIZE;

        /* erase sector before using it for testing */
        GP_LOG_SYSTEM_PRINTF("Erasing sector at address 0x%lx", 0, (unsigned long)address);
        Flash_SectorErase(address);

        /* test single byte read/write */
        testSingleByte(address + (address / SPI_FLASH_PAGE_SIZE) % 11);
        HAL_WAIT_MS(10);

        /* test block read/write */
        testBlock(address + SPI_FLASH_PAGE_SIZE);
        HAL_WAIT_MS(10);

        indicateErrors();
        HAL_WAIT_MS(1000);


    } while (true);

    return 0;
}
