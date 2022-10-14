/*
 * Copyright (c) 2009-2016, GreenPeak Technologies
 * Copyright (c) 2017-2019, Qorvo Inc
 *
 *   Low level Test functions
 *   Declarations of the public functions and enumerations of gpTest.
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
 * Alternatively, this software may be distributed under the terms of the
 * modified BSD License or the 3-clause BSD License as published by the Free
 * Software Foundation @ https://directory.fsf.org/wiki/License:BSD-3-Clause
 *
 * $Header$
 * $Change$
 * $DateTime$
 *
 */


#ifndef _GP_TEST_H_
#define _GP_TEST_H_


/**
 * @file gpTest.h
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpHal.h"
#include "gpHal_Ble.h"

#include "gpPd.h"
#include "gpHal_MAC.h"
#include "gpVersion.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#ifndef GP_TEST_MAX_LENGTH_PACKET
#define GP_TEST_MAX_LENGTH_PACKET           125UL
#endif

#define GP_TEST_TXOPTIONS_RANDOMDATA        0x1
#define GP_TEST_TXOPTIONS_INCREMENTINGCTR   0x2
#define GP_TEST_TXOPTIONS_AUTOMATICALLY_CORRECT_PACKET 0x4
#define GP_TEST_INTERVAL_NO_DELAY 0xFFFE

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/
typedef gpHal_TxPower_t gpTest_TxPower_t;

typedef gpHal_SleepMode_t gpTest_SleepMode_t;

typedef gpHal_AntennaSelection_t gpTest_AntennaSelection_t;

typedef gpHal_SourceIdentifier_t gpTest_SourceIdentifier_t;

typedef gpHal_CollisionAvoidanceMode_t gpTest_CollisionAvoidanceMode_t;

typedef gpHal_BleTxPhy_t gpTest_BleTxPhy_t;

typedef gpHal_BleRxPhy_t gpTest_BleRxPhy_t;

typedef gpHal_Result_t gpTest_Result_t;

typedef gpHal_SleepClockMeasurementStatus_t gpTest_SleepClockMeasurementStatus_t;

//@}
/** @name gpTest_ContinuousWaveMode_t
//@{
 */
/** @brief Unmodulated Continuous Wave Mode */
#define CW_UNMODULATED              'U'
/** @brief Unmodulated Continuous Wave Mode on BLE channelindex */
#define CW_BLE_UNMODULATED          'D'
/** @brief Modulated Continuous Wave Mode */
#define CW_MODULATED                'M'
/** @brief Continuous Wave Mode is BLE */
#define CW_BLE_MODULATED            'B'
/** @brief Continuous Wave Mode is HDR BLE */
#define CW_BLE_HDRMODULATED         'H'
/** @brief Continuous Wave Mode is off */
#define CW_OFF                      'O'
/*
 *  This typedef defines the possible continuous wave modes.
 *  @typedef gpTest_ContinuousWaveMode_t
 */
typedef UInt8 gpTest_ContinuousWaveMode_t;
//@}

/** @name gpTest_PhyMode_t
//@{
 */
/** @brief gpTest in IEEE 802.15.4 mode */
#define gpTest_PhyModeMac           0
/** @brief gpTest in BLE mode */
#define gpTest_PhyModeBle           1
/*
 *  This typedef defines the possible PHY modes to test with (if available on the silicon).
 *  @typedef gpTest_PhyMode_t
 */
typedef UInt8 gpTest_PhyMode_t;
//@}

/** @struct gpTest_Statistics
 *  The gpTest_Statistics structure holds the statistics.
 *  @typedef gpTest_Statistics_t
 *  The gpTest_Statistics_t type specifies the statistics structure.
*/
typedef struct gpTest_Statistics_s {
/** Received packet counter */
    UInt16                          ReceivedPacketCounterC;
    UInt16                          ReceivedPacketCounterL;
/** Received packet counter with matching data */
    UInt16                          ReceivedPacketCounterD;
/** Successfully transmitted packet counter */
    UInt16                          PacketsSentOK;
/** Failed transmitted packet counter */
    UInt16                          PacketsSentError;
/** Cumulative rssi from indications with matching data **/
    Int32                           CumulativeRssi;
/** Cumulative lqi from indications with matching data **/
    UInt32                          CumulativeLQI;
/** Number of packets received on Antenna0. **/
    UInt16                           RxAntenna0;
} gpTest_Statistics_t;


/** @struct gpTest_Settings
 *  This structure holds the current settings.
 *  @typedef gpTest_Settings_t
 *  The gpTest_Settings_t type specifies the settings structure.
*/
typedef struct gpTest_Settings_s {
/** Antenna diversity setting */
    UInt8                           AntennaMode;
/** Selected antenna setting */
    UInt8                           SelectedAntenna;
/** Selected channel setting */
    UInt8                           SelectedChannel;
/** Continuous wave setting */
    UInt8                           ContinuousWave;
/** Maximum Backoff Exponent setting */
    UInt8                           MaxBE;
/** Minimum Backoff Exponent setting */
    UInt8                           MinBE;
/** Maximum CSMA backoffs setting */
    UInt8                           MaxCSMABackoffs;
/** Maximum retries setting */
    UInt8                           MaxRetries;
/** CSMA mode setting */
    UInt8                           CSMAMode;
/** Packet in packet setting */
    UInt8                           PacketInPacket;
/** RxOnWhenIdle setting setting */
    UInt8                           RxOnWhenIdle;
/** Sleepmode setting */
    UInt8                           SleepMode;
///** PER mode setting */
//    UInt8                           PERMode;
/** Tx power setting */
    Int8                            PowerSetting;
/** Promiscuous mode setting */
    UInt8                           PromiscuousMode;
/** PAN ID setting */
    UInt16                          PanID;
/** Short address setting */
    UInt16                          ShortAddress;
    //

} gpTest_Settings_t;


/** @struct gpTest_FrameHeader
 *  The gpTest_FrameHeader structure will hold the header of frames that are transmitted.
 *  @typedef gpTest_FrameHeader_t
 *  The gpTest_FrameHeader_t type specifies the frameheader structure.
*/
typedef struct gpTest_FrameHeader
{
/** First byte */
    UInt8                           byteOne;
/** Second byte */
    UInt8                           byteTwo;
} gpTest_FrameHeader_t;


/** @struct gpTest_StatisticsHeader
 *  The gpTest_StatisticsHeader structure will hold the additional header of statistics frames that are transmitted.
 *  @typedef gpTest_StatisticsHeader_t
 *  The gpTest_StatisticsHeader_t type specifies the additinal header structure in a statistics packet.
*/
typedef struct gpTest_StatisticsHeader
{
/** header length */
    UInt8                           headerLength;
/** identification */
    UInt8                           identifier[4];
/** major version */
    UInt8                           majorVersion;
/** minor version */
    UInt8                           minorVersion;

} gpTest_StatisticsHeader_t;


typedef struct gpTest_ChipIdentity
{
    UInt8       ChipID;
    UInt8       ChipVersion;
} gpTest_ChipIdentity_t;


#define gpTest_VersioningIndexChipId        0
typedef UInt8 gpTest_VersioningIndex_t;

/** @struct gpTest_CntPrio_t */
typedef struct {
    UInt16                         prio0;
    UInt16                         prio1;
    UInt16                         prio2;
    UInt16                         prio3;
} gpTest_CntPrio_t;

/** @struct gpTest_StatisticsCounter_t */
typedef struct {
    UInt16                         ccaFails;
    UInt16                         txRetries;
    UInt16                         failTxNoAck;
    UInt16                         failTxChannelAccess;
    UInt16                         successTx;
    gpTest_CntPrio_t               coexReq;
    gpTest_CntPrio_t               coexGrant;
    UInt16                         totalRx;
    UInt16                         pbmOverflow;
} gpTest_StatisticsCounter_t;

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/** @brief Returns the antenna used to transmit a packet.
 *
 *  This function returns the antenna used to transmit a packet.
 *
 *  @return the antenna used to transmit a packet, possible values defined in enumeration gpTest_AntennaSelection
 */
gpTest_AntennaSelection_t gpTest_GetAntenna(void);

/** @brief Returns the channel currently used for transmit and receive.
 *
 *  This function returns the channel currently used for transmit and receive.
 *  Possible values: 11..26
 *
 *  @return UInt8
 */
UInt8 gpTest_GetChannel(void);

/** @brief Returns the current maxBE setting.
 *
 *  This function returns the current maxBE setting.
 *  Possible values: 1...5
 *
 *  @return UInt8
 */
UInt8 gpTest_GetMaxBE(void);

/** @brief Returns the current minBE setting.
 *
 *  This function returns the current minBE setting.
 *  Possible values: 1...5
 *
 *  @return UInt8
 */
UInt8 gpTest_GetMinBE(void);

/** @brief Returns the current maximum CSMA backoffs.
 *
 *  This function returns the current maximum CSMA backoffs setting.
 *  Possible values: 1...5
 *
 *  @return UInt8
 */
UInt8 gpTest_GetMaxCSMABackoffs(void);

/** @brief Returns the current number of MAC retries.
 *
 *  This function returns the current maximum retries setting.
 *  Possible values: 1...5
 *
 *  @return UInt8
 */
UInt8 gpTest_GetNumberOfRetries(void);

/** @brief Returns the current PAN ID.
 *
 *  This function returns the current PAN ID setting.
 *  Possible values: 0x0000...0xffff
 *
 *  @return UInt16
 */
UInt16 gpTest_GetPanId(gpTest_SourceIdentifier_t srcId);

/** @brief Returns the current Promiscuous mode setting.
 *
 *  This function returns the current promiscuous mode setting.
 *  Possible values:
 *  - true                      Promiscuous mode is enabled
 *  - false                     Promiscuous mode is disabled
 *
 *  @return Bool
 */
Bool gpTest_GetPromiscuousMode(void);

/** @brief Returns the antenna used to receive packets.
 *
 *  This function returns the current antenna used for reception.
 *  Possible values:
 *  - defined in enumeration gpTest_AntennaSelection
 *
 *  @return gpTest_AntennaSelection_t
 */
gpTest_AntennaSelection_t gpTest_GetRxAntenna(void);

/** @brief Set the Rx antenna to use
 *
 * @param value of gpTest_AntennaSelection_t, (possible values: gpTest_AntennaSelection_Ant0 or gpTest_AntennaSelection_Ant1)
**/
void gpTest_SetRxAntenna(gpTest_AntennaSelection_t antenna);

/** @brief Set the Tx antenna to use
 *
 * @param value of gpTest_AntennaSelection_t, (possible values: gpTest_AntennaSelection_Ant0 or gpTest_AntennaSelection_Ant1)
**/
void gpTest_SetTxAntenna(gpTest_AntennaSelection_t antenna);

/** @brief Returns the RXOnWhenIdle flag.
 *
 *  This function returns the current GetRXOnWhenIdle setting.
 *  Possible values:
 *  - true                      RXOnWhenIdle is activated and the radio is turned on.
 *  - false                     RXOnWhenIdle is deactivated.
 *  @return Bool
 */
Bool gpTest_GetRxState(void);

/** @brief Returns the ShortAddress.
 *
 *  This function returns the current short address setting.
 *  Possible values: 0x0000...0xffff.
 *
 *  @return UInt16
 */
UInt16 gpTest_GetShortAddress(gpTest_SourceIdentifier_t srcId);

/** @brief Returns the Extended Address.
 *
 *  This function returns the current extended address.
 *
 *  @param pointer where the MacAddress is returned
 */
void gpTest_GetExtendedAddress(MACAddress_t* pExtendedAddress);

/** @brief Returns the enable state of address recognition.
 *
 *  This function returns if the address recognition is turned on.
 *
 *  @return logical enable information
 */
Bool gpTest_GetAddressRecognition(void);

/** @brief Returns the antenna used to transmit packets.
 *
 *  This function returns the current antenna used for transmission.
 *  Possible values:
 *  - defined in enumeration gpTest_AntennaSelection
 *
 *  @return gpTest_AntennaSelection_t
 */
gpTest_AntennaSelection_t gpTest_GetTxAntenna(void);

/** @brief Returns the current transmit power setting.
 *
 *  This function returns the current transmit power setting.
 *  Possible values: -32...3 (in dBm).
 *
 *  @return Int8
 */
Int8 gpTest_GetTxPower(void);

/** @brief Sets the channel.
 *
 *  This function sets the channel.
 *
 *  @param channel (UInt8) the IEEE channel in the range 11..26.
 *
 *  @return gpTest_Result_t
 *  Possible results are:
 *  - gpTest_ResultSuccess
 *  - gpTest_ResultInvalidParameter (not in correct channel range)
*/
gpTest_Result_t gpTest_SetChannel(UInt8 channel);

/** @brief Sets the number of MAC retries.
 *
 *  The function sets the number of MAC retries.
 *
 *  @param numRetries (UInt8) The number of MAC retries.
 *
 *  @return void
 */
void gpTest_SetNumberOfRetries(UInt8 numRetries);


/** @brief Sets the PAN ID.
 *
 *  This function sets the PAN ID .
 *  Setting the PAN ID of your network enables the automatic filter of
 *  packets not intended for your device.
 *
 *  @param panID (UInt16) The PAN ID of the network.
 *
 *  @return void
 */
void gpTest_SetPanId(UInt16 panID, gpTest_SourceIdentifier_t srcId);

/** @brief Enables or disables the promiscuous mode.
 *
 *  In promiscuous mode, all packets will be received. In order to enable the receiver the
 *  RXOnWhenIdle flag must be set.
 *
 *  @param flag (Bool) Possible values are :
 *  - true                      Promiscuous mode is enabled and the filters disabled.
 *  - false                     Normal filtering is applied on incoming packets.
 *
 *  @return void
 */
void gpTest_SetPromiscuousMode(Bool flag);

/** @brief Enables or disables the sniffer mode.
 *
 *  In sniffer mode, all packets will be received and the device will participate normally in the
 *  network. In order to enable the receiver the RXOnWhenIdle flag must be set.
 *
 *  @param flag (Bool) Possible values are :
 *  - true                      Sniffer mode is enabled and the filters disabled.
 *  - false                     Return to normal behavior - filter packets.
 *
 *  @return void
 */
void gpTest_SetSnifferMode(Bool flag);

/** @brief Sets the maxBE parameter.
 *
 *  This function sets the maxBE parameter. This parameter can be set to a minimum of 1.
 *  To send packets with only one CCA the gpTest_CollisionAvoidanceModeCCA must
 *  be used in the gpTest_DataRequest.
 *
 *  @param maxBE (UInt8) The maximum backoff setting in CSMA-CA.
 *
 *  @return void
 */
void gpTest_SetMaxBE(UInt8 maxBE);

/** @brief Sets the minBE parameter.
 *
 *  This function sets the minBE parameter.
 *
 *  @param minBE (UInt8) The minimum backoff setting in CSMA-CA.
 *
 *  @return void
 */
void gpTest_SetMinBE(UInt8 minBE);


/** @brief Sets the maximum CSMA Backoffs.
 *
 *  This function sets the maximum CSMA backoffs.
 *
 *  @param maxBackoffs (UInt8) the maximum CSMA backoffs.
 *
 *  @return void
 */
void gpTest_SetMaxCSMABackoffs(UInt8 maxBackoffs);

/** @brief Enables or disables the Retransmits on CCA failures.
 * 
 * This function enables or disables the Retransmits on CCA failures. The actual amount of retransmits 
 * is set through gpTest_SetNumberOfRetries. As such this function only changes the 
 * default MAC 802.15.4-2015 behavior to also retransmit on a CCA fail.
 * 
 *  @param enable          Enables or disables the Retransmits.
 *
 *  @return void
 */
void gpTest_SetRetransmitOnCcaFail(Bool enable);

/** @brief returns if the feature "Retransmits on CCA fail" is enabled.
 * 
 * This function returns if the feature "Retransmits on CCA fail" is enabled.
 *
 *  @return Bool
 * 
 */
Bool gpTest_GetRetransmitOnCcaFail(void);

/** @brief Enables or disables the random delays between subsequent retransmits.
 * 
 * This function enables or disables the random delays between subsequent retransmits. 
 * The actual amount of retransmits is set through gpTest_SetNumberOfRetries. As such this 
 * function only changes the default MAC 802.15.4-2015 behavior to add random delays between retransmits.
 * 
 *  @param enable          Enables or disables the random delays between subsequent retransmits.
 *
 *  @return void
 */
void gpTest_SetRetransmitRandomBackoff(Bool enable);

/** @brief returns if the feature "random delays between subsequent retransmits" is enabled.
 * 
 * This function returns if the feature "random delays between subsequent retransmits" is enabled.
 * 
 *  @return Bool
 * 
 */
Bool gpTest_GetRetransmitRandomBackoff(void);

/** @brief This function configures the minimum backoff exponent for random delays between retransmits.
 * 
 * This function configures the minimum backoff exponent for random delays between retransmits.
 * Not to be confused with the backoff exponent for random delays between CCA failures!
 * 
 *  @param minBERetransmit The minimum backoff exponent.
 *
 *  @return void
 */
void gpTest_SetMinBeRetransmit(UInt8 minBERetransmit);

/** @brief This function returns the minimum backoff exponent for random delays between retransmits.
 * 
 * This function returns the minimum backoff exponent for random delays between retransmits.
 * 
 *  @return UInt8
 * 
 */
UInt8 gpTest_GetMinBeRetransmit(void);

/** @brief This function configures the maximum backoff exponent for random delays between retransmits.
 * 
 * This function configures the maximum backoff exponent for random delays between retransmits.
 * Not to be confused with the backoff exponent for random delays between CCA failures!
 * 
 *  @param maxBERetransmit The maximum backoff exponent.
 *
 *  @return void
 */
void gpTest_SetMaxBeRetransmit(UInt8 maxBERetransmit);

/** @brief This function returns the maximum backoff exponent for random delays between retransmits.
 * 
 * This function returns the maximum backoff exponent for random delays between retransmits.
 * 
 *  @return UInt8
 * 
 */
UInt8 gpTest_GetMaxBeRetransmit(void);

/** @brief Sets or clears the RXOnWhenIdle flag.
 *
 *  This function sets the RXOnWhenIdle flag.
 *  Turns on the receiver when the device is idle.  Switching between
 *  TX and RX is done automatically.
 *
 *  @param flag (Bool) Possible values are :
 *  - true                      RXOnWhenIdle is activated and the radio (receiver) is turned on.
 *  - false                     RXOnWhenIdle is deactivated.
 *
 *  @return void
 */

void gpTest_SetRxState(Bool flag);

/** @brief Sets the Short Address.
 *
 *  This functions sets the Short Address.
 *  Setting the Short Address of your device enables the automatic filter of
 *  packets not intended for your device.
 *
 *  @param shortAddress (UInt16) The Short Address of the device.
 *  @return void
 */
void gpTest_SetShortAddress(UInt16 shortAddress, gpTest_SourceIdentifier_t srcId);

/** @brief Sets the Extended Address.
 *
 *  This functions sets the Extended Address.
 *  Setting the Extended Address of your device enables the automatic filter of
 *  packets not intended for your device.
 *
 *  @param pointer where the MacAddress is stored
 *  @return void
 */

void gpTest_SetExtendedAddress(MACAddress_t* pExtendedAddress);
/** @brief Enables/Disables Address Recognition.
 *
 *  This function sets the address recognition options.
 *  @param enable           Possible values are :
 *                           - set to true : destination address of a packet will checked against the address (set by gpTest_SetExtendedAddress and gpTest_SetShortAddress) and destination PAN ID of incoming packets.
 *                           - set to false: address recognition disabled.
 *  @param panCoordinator   Possible values are :
 *                           - set to true : The device is a PAN coordinator.  He will accept messages without a destination address.
 *                           - set to false: Normal filtering will be applied according to recognition settings.
*/
void gpTest_SetAddressRecognition(Bool enable, Bool panCoordinator);

/** @brief Sets the transmit power.
 *
 *  This function sets the transmit power.
 *
 *  @param transmitPower (Int8) The transmitpower setting (in dBm -70..3dBm).
 *
 *  @return result (gpTest_Result_t)
 *  Possible results are:
 *  - gpTest_ResultSuccess
 *  - gpTest_ResultInvalidParameter (not in correct range)
 */
void gpTest_SetTxPower(Int8 transmitPower);

/* @brief Init Test component
 *
 * This function performs all needed SW initialization. The settings of the SW are intialized (Set to their defaults)
 */
void gpTest_Init(void);
/* @brief Startup Test component
 *
 * This function performs all needed HW initialization. The settings of the HW are intialized (Set to their defaults)
 */
void gpTest_Start(void);

void gpTest_Stop(void);

/** @brief This callback is called when a data transmission has been completed.
 *
 * This callback is called when a data transmission has been completed.  The transmission has been
 * initiated by calling the gpTest_DataRequest().
 *
 * This callback has to be implemented by the software layer that is using gpTest.
 *
 *  @param status               The status of the data transfer.
 *  @param packetsSentOK        The number of packets that were sent successfully. This counter
 *                              is reset at the start of the Transmit command.
 *  @param packetsSentError     The number of packets that was not sent. This counter is reset
 *                              at the start of the Transmit command.
 *
 *  @return void
 */
void gpTest_cbDataConfirm(UInt8 status, UInt16 packetsSentOK, UInt16 packetsSentError);

/** @brief This callback is called when a data packet has been received.
 *
 *  This callback is called when a data packet has been received.
 *
 *  This callback has to be implemented by the software layer that is using gpTest.
 *
 *  @param length               The length of the received data.
 *  @param startOfData          The address of the received data.
 *  @param handle               The indentifier that should be used to free the packet.
 *
 *  @return void
 */
void gpTest_cbDataIndication(UInt8 length, gpPd_Offset_t dataOffset, gpPd_Handle_t handle);

/** @brief This callback indicates the completion of an Energy detection (ED).
 *
 *  This callback is called when an energy detection (ED) has been completed.
 *
 *  This callback has to be implemented by the software layer that is using gpTest.
 *
 *  @param result               The result status of the energy detection.
 *  @param channelMask          Mask that indicates for which channels there is valid data.
 *  @param pData                Real energy level (converted by gpTest_CalculateED())
 *  @param Finished             Indication whether scan is completed or not.
 *
 *  @return void
 */
void gpTest_cbEDConfirm(UInt8 result, UInt16 channelMask, UInt8 *pData, Bool Finished);


/** @brief This function issues the wakeup signal of the chip from the MCU.
 *
 *  This function issues the wakeup signal of the chips from the MCU.
 *
 *  Since the chip is blind for Wakeup's immediately after having been put to sleep, a retry
 *  mechanism has been implemented. If we don't get a wake up response from the chip after
 *  5ms, a second wakeup command will be given to it.
 *
 *  This callback has to be implemented by the software layer that is using gpTest.
 *
 *  @return void
*/
void gpTest_WakeUpGP(void);


/** @brief This function returns the current continuous transmit mode.
 *
 *  This function returns the current continuous transmit mode.
 *
 *  @return Bool
 *  Possible values are:
 *  - true                  Continuous transmit is enabled
 *  - false                 Continuous transmit is disabled
 */
Bool gpTest_GetContTx(void);


/** @brief This function sets the collision avoidance mode to use
 *
 *  This function sets the collision avoidance mode to use
 *
 *  @param newMode
 *  Possible values are:
 *  - gpTest_CollisionAvoidanceModeNoCCA
 *  - gpTest_CollisionAvoidanceModeCCA
 *  - gpTest_CollisionAvoidanceModeCSMA
 *
 *  @return void
 */
void gpTest_SetCollisionAvoidanceModeToUse(gpTest_CollisionAvoidanceMode_t newMode);


/** @brief This function returns the collision avoidance mode currently in use.
 *
 *  This function returns the collision avoidance mode currently in use.
 *
 *  @return gpTest_CollisionAvoidanceMode_t
 *  Possible values are:
 *  - gpTest_CollisionAvoidanceModeNoCCA
 *  - gpTest_CollisionAvoidanceModeCCA
 *  - gpTest_CollisionAvoidanceModeCSMA
 */
gpTest_CollisionAvoidanceMode_t gpTest_GetCollisionAvoidanceModeInUse(void);


/** @brief This function enables or disables the Packet In Packet detection.
 *
 *  This function enables or disables the Packet In Packet detection.
 *
 *  @param newPIP
 *  Possible values:
 *  - true                      Packet In Packet detection is enabled
 *  - false                     Packet In Packet detection is disabled
 *
 *  @return void
 */
void gpTest_SetPacketInPacketMode(Bool newPIP);


/** @brief This function returns the currently programmed Packet In Packet detection mode.
 *
 *  This function returns the currently programmed Packet In Packet detection mode.
 *
 *  @return Bool
 *  Possible values:
 *  - true                      Packet In Packet detection is enabled
 *  - false                     Packet In Packet detection is disbaled
 */
Bool gpTest_GetPacketInPacketMode(void);


/** @brief This function returns the current statistics.
 *
 *  This function returns the current statistics.
 *
 *  @param Statistics           Pointer to a gpTest_Statistics_t structure.
 *
 *  @return void
 */
void gpTest_GetStatistics(gpTest_Statistics_t * Statistics);


/** @brief This function returns the current settings.
 *
 *  This function returns the current settings.
 *
 *  @param Settings             Pointer to a gpTest_Settings_t structure.
 *
 *  @return void
 */
void gpTest_GetSettings(gpTest_Settings_t * Settings);


/** @brief Sets the antenna used to transmit and receive packets.
 *
 *  This function selects the antenna used to transmit and receive a packet
 *
 *  @param antenna (gpTest_AntennaSelection_t) The antenna used to transmit and receive packets,
 *                  possible values defined in enumeration gpTest_AntennaSelection
 *
 *  @return void
 */
gpTest_Result_t gpTest_SetAntenna(gpTest_AntennaSelection_t antenna);


/** @brief This function enables or disables antenna diversity.
 *
 *  @param OnOff
 *  Possible values are:
 *  - true          Enable antenna diversity
 *  - false         Disable antenna diversity
 *
 *  @return void
 */
void gpTest_SetAntennaDiversity(Bool OnOff);


/** @brief This function returns the current antenna diversity setting.
 *
 *  @return (Bool)
 *  Possible values are:
 *  - true          Antenna diversity is enabled
 *  - false         Antenna diversity is disabled
 */
Bool gpTest_GetAntennaDiversity(void);

/** @brief This function enables or disables continuous wave operation.
 *
 *  When continuous wave is enabled, there are 2 possible flavors: modulated
 *  (set with CW_MODULATED) and unmodulated (set with CW_UNMODULATED).
 *
 *  @param newMode (UInt8) Possible values are:
 *  - CW_OFF                    Disables continuous wave mode
 *  - CW_MODULATED              Enables modulated continuous wave mode
 *  - CW_UNMODULATED            Enables unmodulated continuous wave mode
 *
 *  @return void
 */
void gpTest_SetContinuousWaveMode(UInt8 newMode);


/** @brief This function returns the current continuous wave mode
 *
 *  @return gpTest_ContinuousWaveMode_t
 *  Possible values:
 *  - CW_OFF                    Continuous mode is not active.
 *  - CW_MODULATED              Modulated continuous mode is active.
 *  - CW_UNMODULATED            Unmodulated continuous mode is active.
 */
gpTest_ContinuousWaveMode_t gpTest_GetContinuousWaveMode(void);


/** @brief This function puts the chip to sleep.
 *
 *  This function puts the chip to sleep.
 *
 *  @return void
 */
void gpTest_SetAsleep(void);


/** @brief This function returns whether or not the chip is asleep.
 *
 *  This function returns whether or not the chip is asleep.
 *
 *  @return (Bool)
 *  Possible values:
 *  - true                      chip is awake
 *  - false                     chip is alseep
 */
Bool gpTest_IsAwake(void);


/** @brief This function sets the sleepmode.
 *
 *  This function sets the sleepmode.
 *
 *  @param mode (gpTest_SleepMode_t)
 *  Possible values are:
 *  - gpTest_SleepModeEvent
 *  - gpTest_SleepMode32kHz
 *  - gpTest_SleepMode16MHz
 *
 *  @return void
 */
void gpTest_SetSleepMode(gpTest_SleepMode_t mode);


/** @brief This function returns the current sleepmode.
 *
 *  This function returns the current sleepmode.
 *
 *  @return (gpTest_SleepMode_t)
 *  Possible values are:
 *  - gpTest_SleepModeEvent
 *  - gpTest_SleepMode32kHz
 *  - gpTest_SleepMode16MHz
 */
gpTest_SleepMode_t gpTest_GetSleepMode(void);

/** @ brief This function returns whether the measurements for the requested sleep mode have been performed and what the
 * measured frequency is.
 *
 * @return gpTest_SleepClockMeasurementStatus_t: status that indicates if measurements where performed or not
 * @return frequencymHz: status that indicates if measurements where performed or not.
 */

gpTest_SleepClockMeasurementStatus_t gpTest_GetMeasuredSleepClockFrequency(gpTest_SleepMode_t mode, UInt32* frequencymHz);

/** @brief This function initiates the packet transmit function.
 *
 *  This function initiates the packet transmit function.
 *
 *  @param numberOfPackets      Number of packets to be transmitted
 *                              Note: the following 'special' values have been defined:
 *                              - 0xffff: start continuous transmit
 *                              - 0x0000: stop continuous transmit
 *  @param intervalInMs         Interval (in ms) between transmissions
 *  @param dataLength           Length of data to be transmitted. Maximum dataLength = GP_TEST_MAX_LENGTH_PACKET.
 *  @param pData                Pointer to data to be transmitted.
 *                              When this pointer is equal to zero, the default message will be transmitted.
 *                              where to properly define default message
 *                              IMPORTANT NOTE: the buffer this pointer points to, must be statically allocated !
 *  @param txOptions            Options as bitmasks
 *                              0x01    Send random data
 *                              0x02    Append incrementing value to data
 *                                      Start with the original value in last 2 bytes (Uint16) and increment per packet
 *
 *  @return void
 */
void gpTest_TxPacket(UInt16 numberOfPackets, UInt16 intervalInMs, UInt8 dataLength, UInt8* pData, UInt8 txOptions);
void gpTestMac_SetExpectedRx(UInt8 dataLength, UInt8* pData);

/** @brief This function initiates the packet transmit function with a missing CRC value
 *
 *  This function initiates the packet transmit function.
 *
 *  @param numberOfPackets      Number of packets to be transmitted
 *                              Note: the following 'special' values have been defined:
 *                              - 0xffff: start continuous transmit
 *                              - 0x0000: stop continuous transmit
 *  @param intervalInMs         Interval (in ms) between transmissions
 *  @param dataLength           Length of data to be transmitted. Maximum dataLength = GP_TEST_MAX_LENGTH_PACKET.
 *  @param pData                Pointer to data to be transmitted.
 *                              When this pointer is equal to zero, the default message will be transmitted.
 *                              where to properly define default message
 *                              IMPORTANT NOTE: the buffer this pointer points to, must be statically allocated !
 *  @param txOptions            Options as bitmasks
 *                              0x01    Send random data
 *                              0x02    Append incrementing value to data
 *                                      Start with the original value in last 2 bytes (Uint16) and increment per packet
 *                              0x04    First send a corrupted packet, then a correct packet
 *
 *  @return void
 */
void gpTest_TxCorruptedPacket(UInt16 numberOfPackets, UInt16 intervalInMs, UInt8 dataLength, UInt8* pData, UInt8 txOptions);

/** @brief This function initiates the packet transmit function with a response window after TX.
 *
 *  This function initiates the packet transmit function.
 *
 *  @param numberOfPackets      Number of packets to be transmitted
 *                              Note: the following 'special' values have been defined:
 *                              - 0xffff: start continuous transmit
 *                              - 0x0000: stop continuous transmit
 *  @param intervalInMs         Interval (in ms) between transmissions
 *  @param dataLength           Length of data to be transmitted. Maximum dataLength = GP_TEST_MAX_LENGTH_PACKET.
 *  @param pData                Pointer to data to be transmitted.
 *                              When this pointer is equal to zero, the default message will be transmitted.
 *                              where to properly define default message
 *                              IMPORTANT NOTE: the buffer this pointer points to, must be statically allocated !
 *  @param txOptions            Options as bitmasks
 *                              0x01    Send random data
 *                              0x02    Append incrementing value to data
 *                                      Start with the original value in last 2 bytes (Uint16) and increment per packet
 *
 *  @return void
 */
void gpTest_TxPollPacket(UInt16 numberOfPackets, UInt16 intervalInMs, UInt8 dataLength, UInt8* pData, UInt8 txOptions);

/** @brief This function set a reply to be sent after a dataindication. This tests a poll response.
 *
 *  This function initiates the packet transmit function.
 *
 *  @param delayUs              Time between receiving the indication and sending the Tx.
 *  @param dataLength           Length of data to be transmitted. Maximum dataLength = GP_TEST_MAX_LENGTH_PACKET.
 *  @param pData                Pointer to data to be transmitted.
 *                              When this pointer is equal to zero, the default message will be transmitted.
 *                              where to properly define default message
 *                              IMPORTANT NOTE: the buffer this pointer points to, must be statically allocated !
 *  @param txOptions            Options as bitmasks
 *                              0x01    Send random data
 *                              0x02    Append incrementing value to data
 *                                      Start with the original value in last 2 bytes (Uint16) and increment per packet
 *
 *  @return void
 */
void gpTest_SetRxResponsePacket(UInt32 delayUs, UInt8 dataLength, UInt8* pData, UInt8 txOptions);


/** @brief This function initiates the energy detect scan.
 *
 *  This function initiates an energy detect scan. Note that this function will enable the receiver.
 *
 *  @param numberOfScans        Number of Energy Detect scans to perform
 *  @param intervalInMs         Interval (in ms) between scans
 *  @param channelMask          Channels which must be scanned
 *
 *  @return void
 */
void gpTest_EDScan(UInt16 numberOfScans, UInt16 intervalInMs, UInt16 channelMask);


/** @brief This function initiates the energy detect scan.
 *
 *  This function initiates an energy detect scan. Note that this function will enable the receiver.
 *
 *  @param numberOfScans        Number of Energy Detect scans to perform
 *  @param intervalInMs         Interval (in ms) between scans
 *  @param channelMask          Channels which must be scanned
 *  @param duration_us          duration for which to scan
 *
 *  @return void
 */
void gpTest_ExtendedEDScan(UInt16 numberOfScans, UInt16 intervalInMs, UInt16 channelMask, UInt32 duration_us);


/** @brief This function resets the gpTest statistics.
 *
 *  @return void
 */
void gpTest_ResetStatistics(void);


/** @brief This function schedules a system reset.
 *
 *  After this function has been performed, both the hardware and software will be
 *  back in their default (startup) state.
 *
 *  @return void
 */
void gpTest_ResetRequest(void);


/** @brief This function returns the average Link Quality Indication.
 *
 *  When frames are received that match the data in the buffer pointed to by gpTest_SavedDataPointer,
 *  the LQI for that frame is obtained and added to a running total value.
 *  When queried, an average LQI is returned, which is calculated by dividing the running total by
 *  the number of received matching frames.
 *
 *  @return (UInt8)
 *  Possible values are:
 *  0 ... 0x7f
 *
 */
UInt8 gpTest_GetAverageLQI(void);


/** @brief This function returns the average Received Signal Strength Indication.
 *
 *  When frames are received that match the data in the buffer pointed to by gpTest_SavedDataPointer,
 *  the RSSI for that frame is obtained and added to a running total value.
 *  When queried, an average RSSI is returned, which is calculated by dividing the running total
 *  by the number of received matching frames.
 *
 *  @return (Int8)
 *  Possible values are:
 *  -
 *
 */
Int8 gpTest_GetAverageRSSI(void);

/** @brief This function gets version info.
 *
 *  This function gets version info of specified type
 *
 *  @param versionType          Type of the version info to get
 *  @param pVersion             Reference to the storage location for the version info
 *
 *  @return void
 */
void gpTest_GetVersionInfo(gpTest_VersioningIndex_t versionType, gpVersion_ReleaseInfo_t * pVersion);


/** @brief This function gets WB register byte.
 *
 *  This function gets byte from register located on WB interface.
 *
 *  @param address              address of the register to read
 *
 *  @return specified register value
 */
UInt8 gpTest_ReadReg(UInt32 address);

/** @brief This function swrites WB register byte.
 *
 *  This function writes byte to register located on WB interface.
 *
 *  @param address              address of the register to write
 *
 *  @param writeByte            value to be written to specified WB address
 *
 *  @return status of execution of the command (true is always returned).
 */
Bool gpTest_WriteReg(UInt32 address, UInt8 writeByte);

/** @brief This function updates enable status of gpLog_Printf function.
 *
 *  @param enable              enable status of the gpLog_Printf.
 */
void gpTest_PrintfEnable(Bool enable);


void gpTest_EnableExternalLna(Bool enable);

void gpTest_RegisterCallbacks(void);
void gpTest_UnregisterCallbacks(void);

void gpTest_StatisticsCountersGet(gpTest_StatisticsCounter_t* pStatisticsCounters);

void gpTest_StatisticsCountersClear(void);

/** @brief This function returns the last Tx power setting of the chip in dBm
 *
 *  @return Int8        Tx Power in dBm of the chip
 */
gpTest_TxPower_t gpTest_GetLastUsedTxPower(void);

/** @brief This function sets the Phy Mode i.e. BLE or 802.15.4 MAC
 *
 *  @param mode
 *  Possible values:
 *  - gpTest_MAC: 0
 *  - gpTest_BLE: 1
 *
 *  @return void
 */
void gpTest_SetPhyMode(gpTest_PhyMode_t mode);


/** @brief This function does Ipc_Stop/Ipc_Restart in the background
 *
 *  @param numberOfRestarts         Number of times to restart. Also the number of gpTest_cbIpcRestartConfirm indications that will be sent back.
 *  @param intervalInMs             Time between restart and next stop
 *  @param stopDurationInUs         Time between stop and restart
 *  @param delayUs                  Time between trigger and first stop
 *  @param trigger                  Trigger that starts the IPC:
 *                                      0x00 -- immediate
 *                                      0x01 -- after Tx (don't wait for confirm)
 *
 *  @return void
 */
gpTest_Result_t gpTest_IpcRestart(UInt16 numberOfRestarts, UInt32 intervalInMs, UInt32 stopDurationInUs, UInt32 delayUs, UInt8 trigger);

#if defined(GP_HAL_DIVERSITY_INCLUDE_IPC)
/** @brief This callback indicates the completion of an Energy detection (ED).
 *
 *  @param stopDurationUs               The time it to took for gpTest_IpcStop to return
 *  @param restartDurationUs            The time it to took for gpTest_IpcRestart to return
 *  @return void
 */
void gpTest_cbIpcRestartConfirm(UInt8 result, UInt32 stopDurationUs, UInt32 restartDurationUs);
#endif

extern void gpTest_BleSetNumberTxPacketsInTestMode(UInt16 numberOfPackets);

/** @brief This function gets the Phy Mode i.e. BLE or 802.15.4 MAC
 *
 *  @param mode
 *  Possible values:
 *  - gpTest_MAC: 0
 *  - gpTest_BLE: 1
 *
 *  @return void
 */
gpTest_PhyMode_t gpTest_GetPhyMode(void);

Bool gpTest_CheckDCDCEnable(void);

extern gpTest_Result_t gpTest_BleTransmitterTest(UInt8 length, UInt8 payloadtype);
extern gpTest_Result_t gpTest_BleReceiverTest(void);
extern UInt16 gpTest_BleTestEnd(void);
extern void gpTest_SetModulation(gpTest_BleTxPhy_t modulation);
extern gpTest_Result_t gpTest_BleSetRxPhyMask(UInt8 rxPhyMask);

gpTest_Result_t gpTest_SetMcuClockSpeed(UInt8 clockSpeed);

/** @brief Get the chip id information
*   @return chipid
*/
UInt8 gpTest_GetChipId(void);

/** @brief Get the chip version information
*   @return chipversion
*/
UInt8 gpTest_GetChipVersion(void);

void gpTest_SetPwrCtrlInByPassMode(Bool enable);
Bool gpTest_GetPwrCtrlInByPassMode(void);

void gpTest_ReadProductId(UInt8* productId);

void gpTest_SetRxLnaAttDuringTimeoutForRssiBasedAgcMode(Bool enable);

void gpTest_SetDpiZbBuffering(UInt8 packetsBuffered);
void gpTest_EnableDpiZb(Bool start);
void gpTest_EnableDtm(Bool enable);

/** @brief This function sets the channel for stacks 1 and 2 (but not for 0!) in case of multi-channel testing.
 *
 *  @param stack1_channel     The channel to be used for stack 1
 *  @param stack2_channel     The channel to be used for stack 2
 *
 *  @return gpTest_Result_t
 */
gpTest_Result_t gpTest_SetChannelForOtherStacks(UInt8 stack1_channel, UInt8 stack2_channel);

/** @brief This function enables the different RX modes. Only one can be set at a time
 *
 *  @param enableMultiStandard      Enable Multi standard RX mode
 *  @param enableMultiChannel       Enable Multi channel RX mode
 *  @param enableHighSensitivity    Enable High sensitivity RX mnode
 *
 *  @return gpTest_Result_t
 *  *  Possible values:
 *  - gpHal_ResultSuccess
 *  - gpHal_ResultInvalidParameter: An invalid combination of parameters was tried to be selected, and the function failed to set the new RX mode
 */
gpTest_Result_t gpTest_SetRxModeOptions(Bool enableMultiStandard, Bool enableMultiChannel, Bool enableHighSensitivity);

#ifdef __cplusplus
}
#endif

#endif //_GP_TEST_H_

