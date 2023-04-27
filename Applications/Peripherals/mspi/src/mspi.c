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

#include "app_common.h"

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

#define GPIO_PIN(pin) gpios[pin]

#if defined(GP_DIVERSITY_FREERTOS)
#define mainQUEUE_RECEIVE_TASK_PRIORITY        ( tskIDLE_PRIORITY + 2 )
#endif
#if defined(GP_DIVERSITY_FREERTOS)
#define APP_WAIT_MS(x) do { \
    while(((x) / portTICK_PERIOD_MS) == 0);\
    vTaskDelay((x) / portTICK_PERIOD_MS); \
} while(0)
#else
#define APP_WAIT_MS HAL_WAIT_MS
#endif


#ifdef APP_DIVERSITY_WAIT_SPI_RX_READY

#define MSPI_GPIO_SSN   3
/* define command id's */
/* read identification */
#define CMD_RDID    0x9F
/* read status register */
#define CMD_RDSR    0x05
/* Write enable */
#define CMD_WREN    0x06
/* Write disable */
#define CMD_WRDI    0x04
/* Read data bytes */
#define CMD_READ    0x03
/* page program */
#define CMD_WRITE   0x02
/* Bulk erase */
#define CMD_BE      0xC7
/* Sector erase */
#define CMD_SE      0xD8

#define FLASH_SPI_SLAVE_REQUEST_TO_SEND_PIN   2
#define NB_OF_SLAVE_LATENCY_BYTES   16

#define WAIT_SPI_SLAVE_READY_TO_TRANSFER()     while(hal_gpioGet(GPIO_PIN(FLASH_SPI_SLAVE_REQUEST_TO_SEND_PIN))){}

#define FLASH_ADDRESS_SIZE          ((UInt8)3)
#define SPI_FLASH_PAGE_SIZE         ((UInt16)256)
#define SPI_FLASH_PAGE_READWRITE    ((UInt8)0x01)
#define SPI_FLASH_BYTE_READWRITE    ((UInt8)0x00)
#endif

/* number of runs for this test */
#define NUMBER_OF_RUNS  5
/*****************************************************************************
 *                    Static global variables
 *****************************************************************************/

static UInt32 App_NrOfErrors;

#if defined(GP_DIVERSITY_FREERTOS)
static StaticTask_t xMainTaskPCB;
#define TASK_STACK_SIZE (4*1024)
static StackType_t xMainTaskStack[TASK_STACK_SIZE];
#endif

/*****************************************************************************
 *                    SPI Flash Functions
 *****************************************************************************/

#ifdef APP_DIVERSITY_WAIT_SPI_RX_READY
/** @brief Function to send slave information about length of data to be send/receive */
static void Flash_SendPageReadWriteFlag(Bool flag)
{
    if(flag)
    {
        hal_WriteReadSPI(SPI_FLASH_PAGE_READWRITE);
    }
    else
    {
        hal_WriteReadSPI(SPI_FLASH_BYTE_READWRITE);
    }
}

/** @brief Function to send Slave Tx latency frames - see datasheet for details */
static void Flash_SendSlaveTxLatency(UInt8 nbOfLatencyBytes) 
{
    UInt8 i = 0;
    while(i < nbOfLatencyBytes)
    {
        hal_WriteReadSPI(0);
        i++;
    } 
}
#endif

/** @brief Function to select or deselect spi slave */
static void Flash_setSsn(Bool high)
{
    if(high)
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

/** @brief Function to read flash device identification data */
static void Flash_ReadId(UInt8* id, UInt8* type, UInt8* cap)
{
    /* select slave */
    Flash_setSsn(0);
    GP_ASSERT_DEV_EXT(id);
    GP_ASSERT_DEV_EXT(type);
    GP_ASSERT_DEV_EXT(cap);

    /* send read_identification command */
    hal_WriteReadSPI(CMD_RDID);

#ifdef APP_DIVERSITY_WAIT_SPI_RX_READY
    /* deselect slave until it is ready to send data */
    Flash_setSsn(1);

    /*wait until slave sets FLASH_SPI_SLAVE_REQUEST_TO_SEND_PIN to active level */
    WAIT_SPI_SLAVE_READY_TO_TRANSFER();

    /* select slave */
    Flash_setSsn(0);

    /* send slave tx latency bytes */
    Flash_SendSlaveTxLatency(NB_OF_SLAVE_LATENCY_BYTES);
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
static UInt8 Flash_ReadStatus(void)
{
    UInt8 status;

    /* select slave */
    Flash_setSsn(0);

    /* send read status register command */
    hal_WriteReadSPI(CMD_RDSR);

#ifdef APP_DIVERSITY_WAIT_SPI_RX_READY
    /* deselect slave until it is ready to send data */
    Flash_setSsn(1);

    /*wait until slave sets FLASH_SPI_SLAVE_REQUEST_TO_SEND_PIN to active level */
    WAIT_SPI_SLAVE_READY_TO_TRANSFER();

    /* select slave */
    Flash_setSsn(0);

    /* send slave tx latency bytes */
    Flash_SendSlaveTxLatency(NB_OF_SLAVE_LATENCY_BYTES);
#endif

    /* read status register */
    status = hal_WriteReadSPI(0);

    /* deslect slave */
    Flash_setSsn(1);

    return status;
}

/** @brief Function to enable or disable Write operation */
static void Flash_WriteEnable(Bool enable)
{
    /* selece slave */
    Flash_setSsn(0);

    /* Send WRITE_ENABLE if true else WRITE_DISABLE */
    hal_WriteReadSPI(enable ? CMD_WREN : CMD_WRDI);

    /* deselect slave */
    Flash_setSsn(1);
}

/** @brief Function to send 3 byte address */
static void Flash_SendAddress(UInt32 address)
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
static void Flash_WriteByte(UInt32 address, UInt8 value)
{
    /* Before PAGE_PROGRAM command, WRITE_ENABLE command must be executed */
    Flash_WriteEnable(true);

    /* select slave */
    Flash_setSsn(0);

    /* send PAGE_PROGRAM command code */
    hal_WriteReadSPI(CMD_WRITE);

    /* send address */
    Flash_SendAddress(address);
#ifdef APP_DIVERSITY_WAIT_SPI_RX_READY
    /* signal byte-long read */
    Flash_SendPageReadWriteFlag(false);
#endif
    /* send value to write */
    hal_WriteReadSPI(value);

    /* deselect slave */
    Flash_setSsn(1);

    /* check status register for WIP bit */
    FLASH_WAIT_WRITE_READY();
}

/** @brief Function to read data byte */
static UInt8 Flash_ReadByte(UInt32 address)
{
    UInt8 value;

    /* select slave */
    Flash_setSsn(0);

    /* send READ DATA BYTES command */
    hal_WriteReadSPI(CMD_READ);

    /* send address to read data */
    Flash_SendAddress(address);

#ifdef APP_DIVERSITY_WAIT_SPI_RX_READY
    /* signal page-long read*/
    Flash_SendPageReadWriteFlag(false);

    /* deselect slave until it is ready to send data */
    Flash_setSsn(1);

    /*wait until slave sets FLASH_SPI_SLAVE_REQUEST_TO_SEND_PIN to active level */
    WAIT_SPI_SLAVE_READY_TO_TRANSFER();

    /* select slave */
    Flash_setSsn(0);

    /* send slave tx latency bytes */
    Flash_SendSlaveTxLatency(NB_OF_SLAVE_LATENCY_BYTES);
#endif

    /* read data */
    value = hal_WriteReadSPI(0);

    /* deselect slave */
    Flash_setSsn(1);

    return value;
}

/** @brief Function to erase flash - Sector erase */
static void Flash_SectorErase(UInt32 address)
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
static void Flash_WriteBlock(UInt32 address, UInt16 length, UInt8* txBuffer)
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

#ifdef APP_DIVERSITY_WAIT_SPI_RX_READY
        /* signal page-long read*/
        Flash_SendPageReadWriteFlag(true);
#endif

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
static void Flash_ReadBlock(UInt32 address, UInt8* rxBuffer)
{
    UInt16 i;

    /* select slave */
    Flash_setSsn(0);

    /* send READ_DATABYTES command */
    hal_WriteReadSPI(CMD_READ);

    /* send 3 bytes address */
    Flash_SendAddress(address);

#ifdef APP_DIVERSITY_WAIT_SPI_RX_READY
    /* signal page-long read*/
    Flash_SendPageReadWriteFlag(true);

    /* deselect slave until it is ready to send data */
    Flash_setSsn(1);

    /*wait until slave sets FLASH_SPI_SLAVE_REQUEST_TO_SEND_PIN to active level */
    WAIT_SPI_SLAVE_READY_TO_TRANSFER();

    /* select slave */
    Flash_setSsn(0);

    /* send slave tx latency bytes */
    Flash_SendSlaveTxLatency(NB_OF_SLAVE_LATENCY_BYTES);
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
static void indicateErrors(void)
{
    UInt32 i;
    for (i=0;i<App_NrOfErrors;i++)
    {
        HAL_LED_SET_RED();
        APP_WAIT_MS(200);
        HAL_LED_CLR_RED();
        APP_WAIT_MS(200);
    }

    GP_LOG_SYSTEM_PRINTF("NrOfErrors=%lu", 0, (unsigned long)App_NrOfErrors);
    gpLog_Flush();
}

/** @brief Function to test status of the device */
static void testStatus(void)
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
static void testSingleByte(UInt32 address)
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
static void testBlock(UInt32 address)
{
     /** @brief Write Buffer */
    static UInt8 writeBuffer[TEST_NUM_BYTES]={0};

    /** @brief Read Buffer */
    static UInt8 readBuffer[TEST_NUM_BYTES]={0};

    GP_LOG_SYSTEM_PRINTF("Test SPI block(multiple bytes) read/write at address 0x%lx", 0, (unsigned long)address);

    /* Reset read buffer. */
    MEMSET(readBuffer, 0, sizeof(readBuffer));

#ifdef APP_DIVERSITY_WAIT_SPI_RX_READY
    /* erase memory */
    Flash_BulkErase();
#endif

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

static void Application_MainLoop(void *pvParameters)
{
    NOT_USED(pvParameters);
    UInt32 address=0;

    /* Wait until flash is ready */
    APP_WAIT_MS(2);

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
    gpLog_Flush();

    LED_INDICATOR_ON();
    APP_WAIT_MS(50);
    LED_INDICATOR_OFF();

    GP_LOG_SYSTEM_PRINTF("=====================",0);
    GP_LOG_SYSTEM_PRINTF("TESTING MSPI - FLASH",0);
    GP_LOG_SYSTEM_PRINTF("=====================",0);

    for (UInt8 run = 0; run < NUMBER_OF_RUNS; run++)
    {
        /* test status of the flash */
        testStatus();
        APP_WAIT_MS(10);

        /* cycle through address */
        address += 2 * SPI_FLASH_PAGE_SIZE;
        address %= TEST_FLASH_BLOCK_SIZE;

        /* erase sector before using it for testing */
        GP_LOG_SYSTEM_PRINTF("Erasing sector at address 0x%lx", 0, (unsigned long)address);
        Flash_SectorErase(address);

        /* test single byte read/write */
        testSingleByte(address + (address / SPI_FLASH_PAGE_SIZE) % 11);
        APP_WAIT_MS(10);

        /* test block read/write */
        testBlock(address + SPI_FLASH_PAGE_SIZE);
        APP_WAIT_MS(10);

        indicateErrors();
        APP_WAIT_MS(1000);

    }

    if (App_NrOfErrors == 0)
    {
        HAL_LED_SET_GRN();
        GP_LOG_SYSTEM_PRINTF("Test TstSSPI FINISHED >>>>> Pass ", 0);
    }
    else
    {
        HAL_LED_SET_RED();
        GP_LOG_SYSTEM_PRINTF("Test TstSSPI FINISHED >>>>> Fail ", 0);
    }
#if defined(GP_DIVERSITY_FREERTOS)
    vTaskDelete(NULL); //Delete task when test is finished
#endif

}

/*****************************************************************************
 *                    Application Init
 *****************************************************************************/

/** @brief Initialize application */
void Application_Init(void)
{

/* 
If Basecomps is not initialising the gpCom component, 
it needs to be enabled the application side in order to to use it.
*/
#ifdef GP_BASECOMPS_DIVERSITY_NO_GPCOM_INIT
#define GP_APP_DIVERSITY_GPCOM_INIT
#endif //GP_BASECOMPS_DIVERSITY_NO_GPCOM_INIT
#ifdef GP_APP_DIVERSITY_GPCOM_INIT
   gpCom_Init();
#endif // GP_APP_DIVERSITY_GPCOM_INIT


/* 
If Basecomps is not initialising the gpLog component,
it needs to be enabled on the application side in order to to use it.
*/
#ifdef GP_BASECOMPS_DIVERSITY_NO_GPLOG_INIT
#define GP_APP_DIVERSITY_GPLOG_INIT
#endif// GP_BASECOMPS_DIVERSITY_NO_GPLOG_INIT
#ifdef GP_APP_DIVERSITY_GPLOG_INIT
   gpLog_Init();
#endif // GP_APP_DIVERSITY_GPLOG_INIT

    gpBaseComps_StackInit();
    hal_DisableWatchdog();

    GP_LOG_SYSTEM_PRINTF("=====================", 0);
    GP_LOG_SYSTEM_PRINTF("SPI master test application starting", 0);
    GP_LOG_SYSTEM_PRINTF("=====================", 0);
    gpLog_Flush();

    App_NrOfErrors = 0;

    /* configure slave select pin - output */
    hal_gpioModePP(GPIO_PIN(FLASH_SPI_SSN_PIN), 1);
#ifdef APP_DIVERSITY_WAIT_SPI_RX_READY
    hal_gpioModePU(FLASH_SPI_SLAVE_REQUEST_TO_SEND_PIN, 1);
#endif
    // Deselect slave.
    Flash_setSsn(1);
    APP_WAIT_MS(1);

    /* Init spi - frequency - 1MHz, mode 0, msb first */
    hal_InitSPI(1000000UL, 0, false);
#if defined(GP_DIVERSITY_FREERTOS)
    TaskHandle_t TaskHandle;
    /* Start the two tasks as described in the comments at the top of this file. */

    TaskHandle = xTaskCreateStatic(
            Application_MainLoop,                   /* The function that implements the task. */
            "spi_example",                           /* The text name assigned to the task - for debug only as it is not used by the kernel. */
            TASK_STACK_SIZE,               /* The size of the stack to allocate to the task. */
            NULL,                                   /* The parameter passed to the task */
            mainQUEUE_RECEIVE_TASK_PRIORITY,        /* The priority assigned to the task. */
            xMainTaskStack,                         /* The task stack memory */
            &xMainTaskPCB);                         /* The task PCB memory */
    GP_ASSERT (GP_DIVERSITY_ASSERT_LEVEL_SYSTEM, TaskHandle!=NULL);
#else
    Application_MainLoop(NULL);
#endif

}

/*****************************************************************************
 *                    Main function
 *****************************************************************************/

/** @brief Main function */
#if defined(GP_SCHED_EXTERNAL_MAIN) && !defined(GP_DIVERSITY_FREERTOS)
MAIN_FUNCTION_RETURN_TYPE main(void)
{
    HAL_INIT();
    /* Intialize application */
    Application_Init();

    Application_MainLoop(NULL);
    return 0;
}
#endif
