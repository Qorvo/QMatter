/*
 * Copyright (c) 2016, GreenPeak Technologies
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
 */

/** @file "gpPTC.h"
 *
 *  gpPTC
 *
 *  Declarations of the public functions and enumerations of gpPTC.
*/

#ifndef _GPPTC_H_
#define _GPPTC_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"

/* <CodeGenerator Placeholder> AdditionalIncludes */
/* referenced in XML by api.h */
#include "gpPTC_CustomCommand.h"
/* </CodeGenerator Placeholder> AdditionalIncludes */

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

/** @enum gpPTC_Result_t */
//@{
/** @brief The function returned successful. */
#define gpPTC_ResultSuccess                                    0x0
/** @brief The connected PC client is not the expected client. Last connected client takes ownership of the board, rendering previously connected clients invalid. */
#define gpPTC_ResultClientIDMismatch                           0x1
/** @brief Received an invalid argument. */
#define gpPTC_ResultInvalidArgument                            0x02
/** @brief Setting an attribute on the target failed. */
#define gpPTC_ResultSetAttributeFailed                         0x03
/** @brief Setting a mode on the target failed. */
#define gpPTC_ResultSetModeFailed                              0x04
/** @brief Received an invalid argument. */
#define gpPTC_ResultInvalidParameter                           0x05
/** @brief Component is busy. */
#define gpPTC_ResultBusy                                       0x06
/** @brief Indicates unsupported attribute or mode (e.g. differences between 15.4 and BLE) */
#define gpPTC_ResultUnsupported                                0x07
/** @brief Indicates that a wrong MF version is used */
#define gpPTC_ResultInvalidChip                                0x08
/** @brief Indicates that a wrong combination was selected while enabling the current RX mode */
#define gpPTC_ResultInvalidRXMode                              0x09
typedef UInt8                             gpPTC_Result_t;
//@}

/** @enum gpPTC_PhyMode_t */
//@{
/** @brief Enum value for 802.15.4 PHY */
#define gpPTC_RF4CE                                            0x00
/** @brief Enum value for BLE PHY */
#define gpPTC_BLE                                              0x01
typedef UInt8                             gpPTC_PhyMode_t;
//@}

/** @enum gpPTC_AttributeId_t */
//@{
/** @brief Channel that the device will transmit or listen on (for all PHYs). */
#define gpPTC_AttributeChannel                                 0x01
/** @brief Antenna on which the device will transmit or listen. */
#define gpPTC_AttributeAntenna                                 0x02
/** @brief Boolean flag controlling the antenna diversity feature. */
#define gpPTC_AttributeAntennaDiversity                        0x03
#define gpPTC_AttributeSleepMode                               0x04
#define gpPTC_AttributeCSMAMode                                0x05
#define gpPTC_AttributeMAXBE                                   0x06
#define gpPTC_AttributeMINBE                                   0x07
#define gpPTC_AttributeMaxCSMABackoff                          0x08
#define gpPTC_AttributeMaxMACRetries                           0x09
#define gpPTC_AttributeTXPower                                 0x10
#define gpPTC_AttributePanID                                   0x11
#define gpPTC_AttributeShortAddress                            0x12
#define gpPTC_AttributeCWMode                                  0x13
#define gpPTC_AttributePowerControlLoop                        0x14
/** @brief Value representing the address that will be used with the ReadRegister mode. */
#define gpPTC_AttributeReadRegisterAddress                     0x15
/** @brief Value representing the address that will be used with the WriteRegister mode. */
#define gpPTC_AttributeWriteRegisterAddress                    0x16
/** @brief Value that will be written to selected register with the WrigeRegister mode. */
#define gpPTC_AttributeWriteRegisterValue                      0x17
/** @brief Special case to set the byte array as the data payload for tx */
#define gpPTC_AttributeSetTxData                               0x18
/** @brief Sets the interval between packets in a packet train when transmitting multiple packets. */
#define gpPTC_AttributePacketIntervalInMS                      0x19
/** @brief Number of scans to be done by ED Scan */
#define gpPTC_AttributeScanCount                               0x20
/** @brief Number of packets that will be transmitted in a packet train. */
#define gpPTC_AttributePacketCount                             0x21
#define gpPTC_AttributePacketInPacket                          0x22
#define gpPTC_AttributePromiscuousMode                         0x23
#define gpPTC_AttributePacketLength                            0x24
/** @brief Write register values only for the specified bitmask */
#define gpPTC_AttributeWriteRegisterBitsMask                   0x25
#define gpPTC_AttributeWriteRegisterBitsValue                  0x26
#define gpPTC_AttributePhyMode                                 0x27
#define gpPTC_BLE_DTM_PacketType                               0x28
#define gpPTC_AttributeScanIntervalInMS                        0x29
#define gpPTC_BLE_DataRate                                     0x30
#define gpPTC_AttributeMapClockType                            0x31
#define gpPTC_AttributeMapClockIOPin                           0x32
#define gpPTC_AttributeRxLnaAttDuringTimeout                   0x33
#define gpPTC_AttributeRxMultiStandard                         0x34
#define gpPTC_AttributeRxHighSensitivity                       0x35
#define gpPTC_AttributeRxMultiChannel                          0x36
#define gpPTC_AttributePDMClkSrc                               0x37
#define gpPTC_AttributePDMClkFreq                              0x38
#define gpPTC_AttributePDMClkOutPin                            0x39
/** @brief This defines the number of elements defined in this enum. This can be used as the size of the buffer array. The value needs to be increased when a new attribute is defined.Not to be used as attribute ! */
#define PTC_ATTRIBUTES_MAX_NUMBER                              0x3A
typedef UInt8                             gpPTC_AttributeId_t;
//@}

/** @enum gpPTC_ModeId_t */
//@{
#define gpPTC_ModeInfo                                         0x01
#define gpPTC_ModeCarrierWave                                  0x02
#define gpPTC_ModeRx                                           0x03
#define gpPTC_ModeReadRegister                                 0x04
#define gpPTC_ModeWriteRegister                                0x05
#define gpPTC_ModeSleep                                        0x06
#define gpPTC_ModeWakeUp                                       0x07
#define gpPTC_ModeEDScan                                       0x08
#define gpPTC_ModeTransmitPacket                               0x09
#define gpPTC_ModePrintPacketCount                             0x10
#define gpPTC_ModeResetCounters                                0x11
#define gpPTC_ModePrintReceivedPackets                         0x12
#define gpPTC_ModeTransmitRandomPackets                        0x13
#define gpPTC_ModeWriteRegisterBits                            0x14
#define gpPTC_ModeSetClockToGPIO                               0x15
#define gpPTC_ModeXtalSelfTest                                 0x16
#define gpPTC_ModeSetPdmClock                                  0x17
typedef UInt8                             gpPTC_ModeId_t;
//@}

/** @enum gpPTC_ModeExecution_t */
//@{
#define gpPTC_ModeExecution_On                                 0x1
#define gpPTC_ModeExecution_Off                                0x0
#define gpPTC_ModeExecution_Single                             0xFF
typedef UInt8                             gpPTC_ModeExecution_t;
//@}

/** @enum gpPTC_CSMAMode_t */
//@{
#define gpPTC_CSMAMode_NoCCA                                   0x0
#define gpPTC_CSMAMode_CCA                                     0x1
#define gpPTC_CSMAMode_CSMA                                    0x2
typedef UInt8                             gpPTC_CSMAMode_t;
//@}

/** @enum gpPTC_SleepModes_t */
//@{
#define gpPTC_SleepModeRC                                      0x0
#define gpPTC_SleepMode32KHz                                   0x1
#define gpPTC_SleepMode32MHz                                   0x2
typedef UInt8                             gpPTC_SleepModes_t;
//@}

/** @enum gpPTC_CWMode_t */
//@{
#define gpPTC_CWUnModulated                                    0x1
#define gpPTC_CWModulated                                      0x0
typedef UInt8                             gpPTC_CWMode_t;
//@}

/** @enum gpPTC_MultiStandard_t */
//@{
#define gpPTC_Enabled                                          0x1
#define gpPTC_Disabled                                         0x0
typedef UInt8                             gpPTC_MultiStandard_t;
//@}

/** @enum gpPTC_HighSensitivity_t */
//@{
#define gpPTC_Enabled                                          0x1
#define gpPTC_Disabled                                         0x0
typedef UInt8                             gpPTC_HighSensitivity_t;
//@}

/** @enum gpPTC_MultiChannel_t */
//@{
#define gpPTC_Enabled                                          0x1
#define gpPTC_Disabled                                         0x0
typedef UInt8                             gpPTC_MultiChannel_t;
//@}

/** @enum gpPTC_PDMClkSrc_t */
//@{
#define gpPTC_PDMClkSrc_None                                   0x00
#define gpPTC_PDMClkSrc_2M                                     0x01
#define gpPTC_PDMClkSrc_PLL                                    0x02
typedef UInt8                             gpPTC_PDMClkSrc_t;
//@}

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/** @macro GP_PTC_API_VERSION */
#define GP_PTC_API_VERSION                           1
/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/** @struct gpPTC_Attribute_t */
typedef struct {
    gpPTC_AttributeId_t            id;
    Int32                          value;
} gpPTC_Attribute_t;

/** @struct gpPTC_MACAddress_t */
typedef struct {
    UInt8                          byte0;
    UInt8                          byte1;
    UInt8                          byte2;
    UInt8                          byte3;
    UInt8                          byte4;
    UInt8                          byte5;
    UInt8                          byte6;
    UInt8                          byte7;
} gpPTC_MACAddress_t;

/** @struct gpPTC_DeviceAddress_t */
typedef struct {
    UInt8                          byte0;
    UInt8                          byte1;
    UInt8                          byte2;
    UInt8                          byte3;
    UInt8                          byte4;
    UInt8                          byte5;
} gpPTC_DeviceAddress_t;

/** @struct gpPTC_chipSerial_t */
typedef struct {
    UInt8                          byte0;
    UInt8                          byte1;
    UInt8                          byte2;
    UInt8                          byte3;
    UInt8                          byte4;
    UInt8                          byte5;
    UInt8                          byte6;
    UInt8                          byte7;
} gpPTC_chipSerial_t;

/** @struct gpPTC_serialNumber_t */
typedef struct {
    UInt8                          location;
    UInt8                          year0;
    UInt8                          year1;
    UInt8                          week0;
    UInt8                          week1;
    UInt8                          batch0;
    UInt8                          batch1;
    UInt8                          index0;
    UInt8                          index1;
    UInt8                          index2;
    UInt8                          index3;
} gpPTC_serialNumber_t;

/** @struct gpPTC_swVersionNumber_t */
typedef struct {
    UInt8                          major;
    UInt8                          minor;
    UInt8                          revision;
    UInt8                          patch;
    UInt32                         changelist;
} gpPTC_swVersionNumber_t;

/** @struct gpPTC_partNumber_t */
typedef struct {
    UInt8                          header0;
    UInt8                          header1;
    UInt8                          byte0;
    UInt8                          byte1;
    UInt8                          byte2;
    UInt8                          byte3;
    UInt8                          byte4;
} gpPTC_partNumber_t;

/** @struct gpPTC_Parameter_t */
typedef struct {
    Int32                          value;
} gpPTC_Parameter_t;

/** @struct gpPTC_DiscoveryInfo_t */
typedef struct {
    gpPTC_MACAddress_t             senderMacAddress;
    gpPTC_MACAddress_t             DUTMacAddress;
    Int16                          RSSI;
} gpPTC_DiscoveryInfo_t;

/** @struct gpPTC_ProductName_t
 *  @brief The name that uniquely identifies the firmware and ptc dll. The RCC application will look for matching names so that the PTC dll and the firmware can communicate. Limited to 20 chars
*/
typedef struct {
    UInt8                          name[40];
} gpPTC_ProductName_t;

/** @struct gpPTC_ProductID_t */
typedef struct {
    UInt8                          name[10];
} gpPTC_ProductID_t;

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//Requests
void gpPTC_Init(void);

/** @brief Set the clientID, in case multiple clients are connected to the same ptc server
*
*   @param clientID
*/
void gpPTC_SetClientIDRequest(UInt8 clientID);

/** @brief Find out the macaddresses of the possible connected targets
*
*   @param clientID
*   @param senderMacAddress
*/
void gpPTC_Discover(UInt8 clientID, gpPTC_MACAddress_t senderMacAddress);

/** @brief Request DUT to execute a software reset.
*/
void gpPTC_ResetDUT(void);

/** @brief Get API version (supported attributes and modes).
            This command's id should not be changed, as it will break API version verification compatibility.
            
*   @return version                  Integer representing the device's API version
*/
UInt32 gpPTC_GetDUTApiVersion(void);

/** @brief Get more detailed information about the dut
*
*   @param clientID
*   @param version
*   @param macAddr
*   @param bleAddr
*   @param appVersion
*   @param partNumber
*   @param productName
*   @param ptcVersion
*   @param productID
 *  @return result
*/
gpPTC_Result_t gpPTC_GetDUTInfoRequest(UInt8 clientID, UInt8* version, gpPTC_MACAddress_t* macAddr, gpPTC_DeviceAddress_t* bleAddr, gpPTC_swVersionNumber_t* appVersion, gpPTC_partNumber_t* partNumber, gpPTC_ProductName_t* productName, gpPTC_swVersionNumber_t* ptcVersion, gpPTC_ProductID_t* productID);

/** @brief Set a possible number of attributes with a max of 8
*
*   @param clientID
*   @param numberOfAttr
*   @param attributes
 *  @return result
*/
gpPTC_Result_t gpPTC_SetAttributeRequest(UInt8 clientID, UInt8 numberOfAttr, gpPTC_Attribute_t* attributes);

/** @brief Get a possible number of attributes with a max of 8
*
*   @param clientID
*   @param numberOfAttr
*   @param attributes
 *  @return result
*/
gpPTC_Result_t gpPTC_GetAttributeRequest(UInt8 clientID, UInt8 numberOfAttr, gpPTC_Attribute_t* attributes);

/** @brief Start a certain mode
*
*   @param clientID
*   @param modeID
*   @param exectime
*   @param OnOff
*   @param numberOfExtraParameters
*   @param parameters
 *  @return result
*/
gpPTC_Result_t gpPTC_SetModeRequest(UInt8 clientID, UInt8 modeID, UInt32 exectime, gpPTC_ModeExecution_t OnOff, UInt8 numberOfExtraParameters, gpPTC_Parameter_t* parameters);

/** @brief Set the byte data to be used for TX packets
*
*   @param clientID
*   @param attributeID
*   @param dataLen
*   @param pData
 *  @return result
*/
gpPTC_Result_t gpPTC_SetByteDataForAttributeRequest(UInt8 clientID, gpPTC_AttributeId_t attributeID, UInt8 dataLen, UInt8* pData);

/** @brief Start a certain mode
*
*   @param clientID
*   @param modeID
 *  @return OnOff
*/
gpPTC_ModeExecution_t gpPTC_GetModeRequest(UInt8 clientID, UInt8 modeID);

/** @brief Pass a command to be executed as byte array and return the result as a byte array
*
*   @param clientID
*   @param dataLenIn
*   @param pDataIn
*   @param dataLenOut
*   @param pDataOut
 *  @return result
*/
gpPTC_Result_t gpPTC_ExecuteCustomCommand(UInt8 clientID, UInt8 dataLenIn, UInt8* pDataIn, UInt8* dataLenOut, UInt8* pDataOut);

//Indications
/** @brief Response from the target with it macaddress and rssi (in case of RF)
*
*   @param pDiscoveryInfo
*/
void gpPTC_DiscoverIndication(gpPTC_DiscoveryInfo_t* pDiscoveryInfo);

/** @brief Send back the payload of the received packets
*
*   @param datalength
*   @param payload
*/
void gpPTC_RXPacketIndication(UInt8 datalength, UInt8* payload);

void gpPTC_DataConfirm(UInt8 status, UInt16 packetsSentOK, UInt16 packetsSentError);

void gpPTC_EDConfirm(UInt8 result, UInt8 finished, UInt16 EDValue);

/* <CodeGenerator Placeholder> AdditionalPublicFunctionDefinitions */
/* referenced in XML by api.h */
Bool gpPTC_Execute32KhzTest(UInt8 dataLenIn, UInt8* pDataIn, UInt8* dataLenOut, UInt8* pDataOut);
/* </CodeGenerator Placeholder> AdditionalPublicFunctionDefinitions */
#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_GPPTC_H_

