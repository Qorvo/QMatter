/*
 *   Copyright (c) 2019, Qorvo Inc
 *
 *   BLE Peripheral API
 *   Implementation of BLE Peripheral
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
 *   $Header$
 *   $Change$
 *   $DateTime$
 */

/** @file "BlePeripheral.c"
 *
 * BLE stack integration file.
 * Handles BLE connection and incoming BLE commands
*/

#define GP_COMPONENT_ID GP_COMPONENT_ID_APP

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpLog.h"
#include "gpCom.h"
#include "gpBleComps.h"
#include "gpSched.h"

#include "BleModule.h"
#include "BleModule_Defs.h"
#include "BlePeripheral.h"
#include "gpBlePeripheralConnectionStm.h"
#include "l2c_defs.h"

#ifdef GP_OTA_DIVERSITY_CLIENT
#include "BleOtaClient.h"
#endif

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#ifdef GP_OTA_DIVERSITY_CLIENT
#define BLE_LINK_ID_OTA         0
#endif //GP_OTA_DIVERSITY_CLIENT

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/
static void (*startConfirm_callback)(Ble_Status_t) = NULL;
static void (*dataIndication_callback)(UInt8, UInt16, UInt8*) = NULL;

/*****************************************************************************
 *                    Static functions
 *****************************************************************************/

/*****************************************************************************
 *                    Ble Module callbacks
 *****************************************************************************/

void BleModule_cbOpenConnectionConfirm(UInt8 linkId, BleModule_Result_t status)
{
    if(status == BleModule_Success)
    {
        GP_LOG_SYSTEM_PRINTF("Ble linkId %u open success",0,linkId);
#ifdef GP_OTA_DIVERSITY_CLIENT
        if(linkId == BLE_LINK_ID_OTA)
        {
            BleOtaClient_cbOpenConnection(linkId);
        }
#endif //GP_OTA_DIVERSITY_CLIENT
    }
    else
    {
        GP_LOG_SYSTEM_PRINTF("Ble linkId %u open failed. st:0x%x",0,linkId,status);
    }

    if(startConfirm_callback != NULL)
    {
        startConfirm_callback(status);
    }
}

void BleModule_cbCloseConnectionConfirm(UInt8 linkId, BleModule_Result_t status)
{
    if(status == BleModule_Success)
    {
        GP_LOG_SYSTEM_PRINTF("Ble linkId %u close success",0,linkId);
#ifdef GP_OTA_DIVERSITY_CLIENT
        if(linkId == BLE_LINK_ID_OTA)
        {
            BleOtaClient_cbCloseConnection(linkId);
        }    
#endif //GP_OTA_DIVERSITY_CLIENT

    }
    else
    {
        GP_LOG_SYSTEM_PRINTF("Ble linkId %u close failed. st:0x%x",0,linkId,status);
    }
}

void BleModule_cbSecurityConfirm(UInt8 linkId, BleModule_SecurityKeyExchangedInd_t keyExchangedInfo)
{
    GP_LOG_SYSTEM_PRINTF("Ble linkId %u encrypted",0,linkId);
    GP_LOG_SYSTEM_PRINTF("Type: %u Level: %u",0,keyExchangedInfo.keyType,keyExchangedInfo.securityLevel);
#ifdef GP_OTA_DIVERSITY_CLIENT
    if(linkId == BLE_LINK_ID_OTA)
    {
        BleOtaClient_cbBondedConnection(linkId);
    }
#endif //GP_OTA_DIVERSITY_CLIENT
}

void BleModule_cbPairingConfirm(UInt8 linkId, UInt8 status, UInt8 authAndBondFlags)
{
    if(status == BleModule_Success)
    {
        GP_LOG_SYSTEM_PRINTF("Ble linkId %u pairing success",0,linkId);
    }
    else
    {
        GP_LOG_SYSTEM_PRINTF("Ble linkId %u pairing failed. st:0x%x",0,linkId,status);
    }
}

void BleModule_cbUnbindConnectionConfirm(UInt8 linkId, BleModule_Result_t status)
{
    if(status == BleModule_Success)
    {
        GP_LOG_SYSTEM_PRINTF("Ble linkId %u removed",0,linkId);
    }
    else
    {
        GP_LOG_SYSTEM_PRINTF("Ble linkId %u unbind fail. st:0x%x",0,linkId,status);
    }
}

void BleModule_cbSetBusyMode(Bool busy)
{
    if (busy == true)
    {
        GP_LOG_SYSTEM_PRINTF("Ble module busy",0);
    }
    else
    {
        GP_LOG_SYSTEM_PRINTF("Ble module done",0);
    }
}

void BleModule_cbUpdateConnectionParametersConfirm(UInt8 linkId, UInt8 status, BleModule_ConnectionParametersIndCnf_t ConPar)
{
    if(status == BleModule_Success)
    {
        UInt32 connectionInterval = ConPar.ConnectionInterval;
        UInt16 supervisionTimeout = ConPar.ConnectionSupervisionTimeout;

        /* scale connection interval to ms */
        connectionInterval *= 125;
        connectionInterval /= 100;
        /* Should be oke, but just for safety, explicit set MSB to zero before cast to UInt16 */
        connectionInterval &= 0x0000FFFF;

        /* scale supervision timeout to ms */
        supervisionTimeout *= 10;

        GP_LOG_SYSTEM_PRINTF("Link %d update connection: status 0x%x, Interval %d ms, Latency %d svTimeout %d ms",0,
                              linkId, ConPar.Status, (UInt16)connectionInterval, ConPar.ConnectionLatency, supervisionTimeout);
    }
    else
    {
        GP_LOG_SYSTEM_PRINTF("Link %d update connection failed: status 0x%x",0, linkId, status);
    }
}

void BleModule_cbResetDoneIndication(void)
{    
    uint16_t rxAclLen;

    rxAclLen = HciGetMaxRxAclLen();
    if(CORDIO_BLE_HOST_ATT_MAX_MTU > (rxAclLen - L2C_HDR_LEN))
    {
        HciSetMaxRxAclLen(CORDIO_BLE_HOST_ATT_MAX_MTU + L2C_HDR_LEN);
    }

    GP_LOG_SYSTEM_PRINTF("BleModule ready after reset",0);
}

#ifdef GP_BLE_ATT_SERVER_DAT
void BleModule_cbDatDataIndication(UInt8 linkId, UInt16 length, UInt8* pData, BleModule_Result_t status)
{
    if (status == BleModule_Success)
    {
#ifdef GP_OTA_DIVERSITY_CLIENT
        if(linkId == BLE_LINK_ID_OTA && BleOtaClient_cbDataIndication(linkId, length, pData))
        {
            return;
        }
#endif //GP_OTA_DIVERSITY_CLIENT
        GP_LOG_SYSTEM_PRINTF("Received data from linkId %u",0,linkId);
        gpLog_PrintBuffer(length,pData);

        //Handle incoming data as commands for application
        if(dataIndication_callback != NULL)
        {
            dataIndication_callback(linkId, length, pData);
        }
    }
}

void BleModule_cbDatDataConfirm(UInt8 linkId, BleModule_Result_t status)
{
    if(status == BleModule_Success)
    {
        GP_LOG_SYSTEM_PRINTF("Sent data to linkId %u",0,linkId);
    }
    else
    {
        GP_LOG_SYSTEM_PRINTF("Sending data to linkId %d failed - st:%x",0,linkId, status);
    }
}
#endif //GP_BLE_ATT_SERVER_DAT

/*****************************************************************************
 *                    Public functions
 *****************************************************************************/

/** @brief Inits the Ble stack
*/
void BlePeripheral_Init(void (*startConfirm)(Ble_Status_t), void (*dataIndication)(UInt8, UInt16, UInt8*))
{
    gpBleComps_StackInit();
    BleModule_Init();

    //Register callbacks
    BleModule_Peripheral_Cb_t peripheralAppCb =
    {
        .openConnCb = BleModule_cbOpenConnectionConfirm,
        .closeConnCb = BleModule_cbCloseConnectionConfirm,
        .secKeyCb = BleModule_cbSecurityConfirm,
        .pairCb = BleModule_cbPairingConfirm,
        .unbindConnCb = BleModule_cbUnbindConnectionConfirm,
        .UpdateConnCb = BleModule_cbUpdateConnectionParametersConfirm,
    };

    BleModule_Peripheral_RegisterCb(&peripheralAppCb);

#ifdef GP_OTA_DIVERSITY_CLIENT
    BleOtaClient_Init();
#endif //GP_OTA_DIVERSITY_CLIENT

    startConfirm_callback = startConfirm;
    dataIndication_callback = dataIndication;
}

/** @brief Starts the Ble stack
*/
void BlePeripheral_Start(UInt8 linkId)
{
    GP_LOG_SYSTEM_PRINTF("BLE: Starting connection (LinkId %u)", 0, linkId);
    BleModule_OpenConnectionRequest(linkId);
}

/** @brief To be called to send data over the BLE service
*/
void BlePeripheral_DataRequest(UInt8 linkId, UInt16 length, UInt8* pData)
{
#ifdef GP_BLE_ATT_SERVER_DAT
    BleModule_DatDataRequest(linkId, length, pData);
#endif //GP_BLE_ATT_SERVER_DAT
}

/** @brief Close connection
*/
void BlePeripheral_CloseConnection(UInt8 linkId)
{
    BleModule_CloseConnectionRequest(linkId);
}

/** @brief Close all opened connections - called before factory reset
*/
void BlePeripheral_CloseConnections(void)
{
    UInt8 linkId;
    for(linkId=0; linkId<GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS; linkId++)
    {
        BleModule_CloseConnectionRequest(linkId);
    }
}

/** @brief Unbind connection
*/
void BlePeripheral_UnbindConnection(UInt8 linkId)
{
    BleModule_UnbindConnectionRequest(linkId);
}

/** @brief Clear all binding information - called in factory reset
*/
void BlePeripheral_FactoryReset(void)
{
    UInt8 linkId;
    for(linkId=0; linkId<GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS; linkId++)
    {
        BleModule_UnbindConnectionRequest(linkId);
    }
}

/** @brief Dump the BLE connection information
*/
void BlePeripheral_DumpConnInfo(void)
{
    UInt8 linkId;
    appDbHdl_t AppDbHdl;
    UInt8 *addr;
    UInt8 cnt = 0;

    GP_LOG_SYSTEM_PRINTF("BLE Centrals :", 0);
    for(linkId=0; linkId<GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS; linkId++)
    {
        AppDbHdl = AppDbGetHdlByLinkId(linkId);
        if(AppDbRecordInUse(AppDbHdl))
        {
            addr = AppDbGetPeerAddress(AppDbHdl);
            if(gpBlePeripheralConnectionStm_IsConnected(linkId) == gpBlePeripheralConnectionStmResult_NoConnection)
            {
                GP_LOG_SYSTEM_PRINTF("  LinkId %u : %02X:%02X:%02X:%02X:%02X:%02X (Disconnected)", 0,
                                     linkId, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
            }
            else
            {
                GP_LOG_SYSTEM_PRINTF("  LinkId %u : %02X:%02X:%02X:%02X:%02X:%02X (Connected)", 0,
                                     linkId, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
            }
            cnt++;
        }
    }
    if(!cnt)
    {
        GP_LOG_SYSTEM_PRINTF("  None",0);
    }
}
