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

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_GPHAL

#include "gpPd.h"
#include "gpHal.h"
#include "gpHal_DEFS.h"
#include "gpHal_kx_Ipc.h"
#include "gpHal_RadioMgmt.h"
/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GPHAL_IPC_TIMEOUT_US                           10000
#define GPHAL_RT_SYSTEM_HALTED_WAIT_TIME_US            10000
#define GPHAL_IPC_HALT_TIMEOUT_US                      150000 //us

// Maximum timeout before TRC should become idle after disabling rx on when idle flags
#define GPHAL_TRC_IDLE_WAIT_TIME_US                    10000

// Offset from start of RT system where the version bytes are located
// This should only be used for RT from flash or when working with MF >= 8.
#define GPHAL_RT_SYSTEM_VERSION_INFO_START_OFFSET      0x80

// Offset for ROM RT system vector table
#define GPHAL_RT_SYSTEM_ROM_VTOR_START_OFFSET          0x80

#define GPM_WINDOW_GRANULARITY                         0x80

/*****************************************************************************
 *                   Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/******************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

#ifdef GP_DIVERSITY_RT_SYSTEM_PARTS_IN_ROM
static UInt8 gpHal_RtSystemVersion_Rom;
#endif

#if defined(GP_DIVERSITY_RT_SYSTEM_PARTS_IN_FLASH) || defined(GP_DIVERSITY_RT_SYSTEM_MACFILTER_IN_FLASH)
static UInt8 gpHal_RtSystemVersion_Flash;
#endif

#ifdef GP_DIVERSITY_RT_SYSTEM_PARTS_IN_FLASH
static Bool gphalIPC_radioClaimed = false;
#endif

/******************************************************************************
 *                    External Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

gpHal_Result_t gpHal_IpcTriggerCommand(UInt8 commandId, UInt8 argsLength, UInt8* pArgs)
{
    gpHal_Result_t result;
    Bool timeOut = false;

    // Write cmd
    GP_WB_WRITE_U16(BLE_MGR_CMD_REG_ADDRESS,commandId);
    // Write result to invalid value
    GP_WB_WRITE_U16(BLE_MGR_CMD_RESULT_REG_ADDRESS,0);

    if(argsLength > 0)
    {
        GP_ASSERT_DEV_INT(pArgs != NULL);
        // Write args
        GP_HAL_WRITE_BYTE_STREAM(BLE_MGR_CMD_ARGS_START_ADDRESS, pArgs, argsLength);
    }

    // Launch
    GP_WB_IPC_SET_X2GPM_CMD_INTERRUPT();

    __DSB();

    // Wait untill command has finished
    GP_DO_WHILE_TIMEOUT(!GP_WB_READ_IPC_UNMASKED_GPM2X_CMD_PROCESSED_INTERRUPT(), GPHAL_IPC_TIMEOUT_US, &timeOut);

    if(timeOut)
    {
        // Command was not processed by BLE event manager
        GP_LOG_SYSTEM_PRINTF("[WARN] Timeout for gpHal IPC command %u" ,0, commandId);
        result = gpHal_ResultBusy;
    }
    else
    {
        if(GP_WB_READ_U16(BLE_MGR_CMD_RESULT_REG_ADDRESS) != commandId)
        {
            GP_LOG_SYSTEM_PRINTF(" reg 0x%x commandId 0x%x ",2, GP_WB_READ_U16(BLE_MGR_CMD_RESULT_REG_ADDRESS), commandId);
        }
        // Check the status of the command, should equal the commandId
        GP_ASSERT_DEV_INT(GP_WB_READ_U16(BLE_MGR_CMD_RESULT_REG_ADDRESS) == commandId);
        // Cleanup
        GP_WB_IPC_CLR_GPM2X_CMD_PROCESSED_INTERRUPT();
        result = gpHal_ResultSuccess;
    }

    return result;
}

#ifdef GP_DIVERSITY_RT_SYSTEM_PARTS_IN_FLASH
#define IPC_ITRFLAG_IPCGPM2X_INTERRUPT 0
#define IPC_ITRFLAG_RCI_INTERRUPT      1

void gpHal_IpcStop(gpHal_IpcBackupRestoreFlags_t* pFlags)
{
    gpHal_Result_t result;

    GP_ASSERT_DEV_EXT(pFlags != NULL);

    // Backup all needed flags
    pFlags->interruptFlags = 0;

    if (GP_WB_READ_INT_CTRL_MASK_INT_IPCGPM2X_INTERRUPT())
    {
        BIT_SET(pFlags->interruptFlags,IPC_ITRFLAG_IPCGPM2X_INTERRUPT);
        GP_WB_WRITE_INT_CTRL_MASK_INT_IPCGPM2X_INTERRUPT(0);
    }

    if (GP_WB_READ_INT_CTRL_MASK_INT_RCI_INTERRUPT())
    {
        BIT_SET(pFlags->interruptFlags,IPC_ITRFLAG_RCI_INTERRUPT);
        GP_WB_WRITE_INT_CTRL_MASK_INT_RCI_INTERRUPT(0);
    }

    // Stop ZigBee activities
    // Just disable the macfilter interrupt to the RT system, this will cause all ZB RX packets to be dropped with the
    // GP_WB_ENUM_RX_DROP_REASON_PACKET_ENDED_BEFORE_PROCESSING_DONE status.
    GP_WB_WRITE_INT_CTRL_MASK_GPM_PARFCS_INTERRUPT(0);
    // The macfilter can accept packets regardless if its claim on the radio is granted for sending an ACK. Therefore it can accept a packet
    // and the software would get a dataindication without an ACK having been sent. Assuming there will be retries this is not a problem but
    // it's not optimal. Therefore we give the macfilter some time to finish it's current interrupt handler so that it can accept a packet and
    // claim the radio for the ACK.
    HAL_WAIT_US(100);
    // Afterwards claim the management interface, this will stop new ZB activities from starting. We could let the ZB activities continue
    // but then incoming packets would be dropped because they cannot be processed by the macfilter in time. Stopping activities altogether
    // might consume less power.
    // If a ZB TX activity had the possibility to start in the wait from before, it will not be able to receive ACK's.
    // But assuming the ZB retry mechanism will handle this.
    // We don't need to wait for the claim to be granted.
    if (!GP_WB_READ_RADIO_ARB_MGMT_CLAIM())
    {
        gphalIPC_radioClaimed = true;
        /*
        Here we assume IPC stop and IPC restart are synchronous. And since grant is not required, claim/release
        does not use the radio mgmt API
        */
        GP_WB_WRITE_RADIO_ARB_MGMT_CLAIM(1);
    }

    // After having claimed the radio (via the management-claim itf), tell RT to stop
    // so RT will release the BLE radio-claim (if raised) and expedite the management-grant
    result = gpHal_IpcTriggerCommand(BLE_MGR_HALT_GPMICRO, 0, NULL);
    GP_ASSERT_SYSTEM(gpHal_ResultSuccess == result);

    //Wait until gp micro goes into reset
    GP_DO_WHILE_TIMEOUT_ASSERT(!GP_WB_READ_STANDBY_RESET_GPMICRO(), GPHAL_IPC_HALT_TIMEOUT_US); //Timeout TBD
}


void gpHal_IpcRestart(gpHal_IpcBackupRestoreFlags_t* pFlags)
{
    UInt8 intmask;
    GP_ASSERT_DEV_EXT(pFlags != NULL);

    // Re-enable the macfilter interrupt, clear any pending IRQs
    GP_WB_PARFCS_CLR_LEVEL_TRIGGER_INTERRUPT();
    GP_WB_WRITE_INT_CTRL_MASK_GPM_PARFCS_INTERRUPT(1);

    // Allow gpMicro to run again
    GP_WB_WRITE_STANDBY_RESET_GPMICRO(0);

    // Free the arbiter
    if (gphalIPC_radioClaimed)
    {
        GP_WB_WRITE_RADIO_ARB_MGMT_CLAIM(0);
        gphalIPC_radioClaimed = false;
    }

    // Restore interrupt and rx on when idle flags
    intmask = (UInt8) (BIT_TST(pFlags->interruptFlags,IPC_ITRFLAG_IPCGPM2X_INTERRUPT));
    GP_WB_WRITE_INT_CTRL_MASK_INT_IPCGPM2X_INTERRUPT(intmask);
    intmask = (UInt8) (BIT_TST(pFlags->interruptFlags,IPC_ITRFLAG_RCI_INTERRUPT));
    GP_WB_WRITE_INT_CTRL_MASK_INT_RCI_INTERRUPT(intmask);
}
#endif //GP_DIVERSITY_RT_SYSTEM_PARTS_IN_FLASH

void gpHal_IpcInit(void)
{

#ifdef GP_COMP_CHIPEMU
    GP_WB_WRITE_STANDBY_RESET_GPMICRO(0);
#endif
    // Enable interrupts to the GPM
    GP_WB_WRITE_INT_CTRL_MASK_GPM_IPCX2GPM_INTERRUPT(1);
    // Enable cmd interrupts
    GP_WB_WRITE_INT_CTRL_MASK_IPCX2GPM_CMD_INTERRUPT(1);
#ifdef GP_DIVERSITY_RT_SYSTEM_PARTS_IN_FLASH
    gphalIPC_radioClaimed = false;
#endif
}

void gpHal_RtInitVersionInfo(void)
{
#if defined(GP_DIVERSITY_RT_SYSTEM_PARTS_IN_FLASH) || defined(GP_DIVERSITY_RT_SYSTEM_MACFILTER_IN_FLASH)
    gpHal_RtSystemVersion_Flash = (gpHal_RtSystem_FlashData[GPHAL_RT_SYSTEM_VERSION_INFO_START_OFFSET/2] & 0xFF);

    // When RT is running from flash, we know the version at compile time.
    // Check at runtime to make sure it still matches
    GP_ASSERT_SYSTEM(gpHal_RtSystemVersion_Flash == GP_DIVERSITY_RT_SYSTEM_IN_FLASH_VERSION);
#endif

#ifdef GP_DIVERSITY_RT_SYSTEM_PARTS_IN_ROM
    UInt32 rtStartAddress = GP_WB_READ_GPMICRO_PROGRAM_WINDOW_OFFSET_0() * GPM_WINDOW_GRANULARITY + GPHAL_RT_SYSTEM_ROM_VTOR_START_OFFSET;
    gpHal_RtSystemVersion_Rom = GP_WB_READ_U8(GP_MM_ROM_ADDR_FROM_COMPRESSED(rtStartAddress) + GPHAL_RT_SYSTEM_VERSION_INFO_START_OFFSET);
#endif
}

UInt8 gpHal_GetRtSystemVersion(gpHal_RtSubSystemId_t subsystem_id)
{
    UInt8 version = 0xFF;
    switch (subsystem_id)
    {
        case gpHal_RtSubSystem_BleMgr:
#ifdef GP_DIVERSITY_RT_SYSTEM_BLEMGR_IN_FLASH
            version = gpHal_RtSystemVersion_Flash;
#elif defined(GP_DIVERSITY_RT_SYSTEM_IN_ROM) || \
      defined(GP_DIVERSITY_RT_SYSTEM_PARTS_IN_ROM)
            version = gpHal_RtSystemVersion_Rom;
#else
#error "something is wrong with the diversities, need to define GP_DIVERSITY_RT_SYSTEM_BLEMGR_IN_FLASH or GP_DIVERSITY_RT_SYSTEM_IN_ROM"
#endif

            break;
        case gpHal_RtSubSystem_MacFilter:
        case gpHal_RtSubSystem_CalMgr:
#ifdef GP_DIVERSITY_RT_SYSTEM_MACFILTER_IN_ROM
            version = gpHal_RtSystemVersion_Rom;
#elif defined(GP_DIVERSITY_RT_SYSTEM_IN_FLASH) || \
      defined(GP_DIVERSITY_RT_SYSTEM_MACFILTER_IN_FLASH) || \
      defined(GP_DIVERSITY_RT_SYSTEM_PARTS_IN_FLASH)
            version = gpHal_RtSystemVersion_Flash;
#else
#error "something is wrong with the diversities"
#endif
            break;
        default:
            GP_ASSERT_DEV_EXT(false);
    }
    return version;
}
