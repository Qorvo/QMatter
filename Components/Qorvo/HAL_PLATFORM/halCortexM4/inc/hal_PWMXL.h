/*
 * Copyright (c) 2022, Qorvo Inc
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
 *
 */

#ifndef _HAL_PWMXL_H_
#define _HAL_PWMXL_H_

#include "global.h"

#ifdef HAL_DIVERSITY_PWMXL

#define HAL_PWMXL_DEFAULT_FREQUENCY 1000UL

typedef enum {
    PWMXL_TM_PHASE_MATCH =  GP_WB_ENUM_PWMXL_PHASE_TRIGGER_MODE_PHASE_MATCH,
    PWMXL_TM_EXTERNAL = GP_WB_ENUM_PWMXL_PHASE_TRIGGER_MODE_EXTERNAL,
    PWMXL_TM_EXTERNAL_PHASE_MATCH = GP_WB_ENUM_PWMXL_PHASE_TRIGGER_MODE_EXTERNAL_AFTER_PHASE_MATCH
} pwmxl_trigger_mode_t;

typedef enum {
    HAL_PWMXL_VALUE_TYPE_PHASE = 0,
    HAL_PWMXL_VALUE_TYPE_WIDTH = 1,
    HAL_PWMXL_VALUE_TYPE_DITHER = 2,
    HAL_PWMXL_VALUE_TYPE_TOP_WRAP = 3
} pwmxl_dma_value_type_t;

typedef enum {
    HAL_PWMXL_DMA_16BIT = 0,
    HAL_PWMXL_DMA_32BITWAVEFORM = 1,
} pwmxl_dma_mode_t;

/** @brief Structure containing one entry used to generate arbitrary waveform.
 *         For more information how to use it, please refer to the chip databook,
 *         chapter 13.8.2 (PWMXL, Advanced features).
 */
typedef struct {
    UInt16 value;
    union {
        struct {
            pwmxl_dma_value_type_t value_type : 2;
            UInt16 _reserved1 : 6;
            UInt16 idx : 3;
            Bool validate_phase : 1;
            Bool validate_width : 1;
            Bool validate_dither : 1;
            UInt16 _reserved2 : 2;
        };
        UInt16 dma_u16;
    };
} pwmxl_dma_value_t;

/** @brief Structure used to map PWMXL_MAP_x_EMPTY event (where x is PHASE, WIDTH or DITHER)
 *         of a specific channel to NEXT_BUFFER_NOT_FULL interrupt.
 *
 *         This is needed for DMA triggering.
 */
typedef struct {
    UInt8 next_phase : 1;
    UInt8 next_width : 1;
    UInt8 next_dither : 1;
    UInt8 channel : 5;
} pwmxl_dma_buffermap_t;

/** @brief Callback invoked to fetch more samples during DMA transfer.
 *
 *  This function is required if the application use DMA with PWM_XL peripheral.
 *
 *  @param sizeToFill  Size in bytes of new sample data needed.
 *  @param buffer      Pointer to sample buffer.
 *  @param writeIndex  Offset in bytes in the buffer where new sample data must be written.
 *  @param mode        Defines current transfer mode and data structure - 16-bit PCM
 *                     or 32-bit structure for arbitrary waveform generation.
 *                     Application should cast buffer pointer to a proper type
 *                     and calculate number of elements basing on value of this parameter.
 *
 *  @return            True when buffer was filled, False if no more data is available
 *                     and playback should be halted.
 */
typedef Bool (*pwmxl_callback_t)(UInt16 sizeToFill,
                                 void* buffer,
                                 UInt16 writeIndex,
                                 pwmxl_dma_mode_t mode);

/** @brief Initialize the PWMXL peripheral.
 *         It will configure the PWM peripheral to run on a default frequency (1kHz).
*/
void halPwmxl_init();

/** @brief Enable or disable the PWMXL peripheral.
 *
 *  @param enable  True to enable, false to disable.
 */
GP_API void halPwmxl_enable(Bool enable);

/** @brief Set base period of the PWMXL peripheral to a specific number of cycles.
 *         In order to get a specific frequency in Hz, use halPwmxl_setFrequency() instead.
 *
 *  @param top        Counter top value.
 *  @param prescaler  Counter prescaler value in range (0..7).
 *
 *  Duty cycles of all PWM channels will be invalid after changing the base frequency.
 *  Subsequent calls to halPwmxl_setDutyCycle() are needed to set valid duty cycles.
 */
GP_API void halPwmxl_setPeriodRaw(UInt16 top, UInt8 prescaler);

/** @brief Set base frequency of the PWMXL peripheral.
 *
 *  @param frequency  Desired frequency in Hz. Must be > 0.
 *
 *  Duty cycles of all PWM channels will be invalid after changing the base frequency.
 *  Subsequent calls to halPwmxl_setDutyCycle() are needed to set valid duty cycles.
 */
GP_API void halPwmxl_setFrequency(UInt32 frequency);

/** @brief Initialize single PWM channel.
 *
 *  @param channel  PWM channel number.
 *  @param enable   GPIO number assigned to channel output.
 *                  The selected pin must have a PWMx alternate function, where x is the
 *                  specific channel number. Function will fail if invalid pin was specified.
 *
 *  @return True on success, otherwise false.
 */
GP_API Bool halPwmxl_initChannel(UInt8 channel, UInt8 pin);

/** @brief Enable or disable the PWM function of the GPIO pin for the specified PWM channel.
 *
 *  @param channel  PWM channel number
 *  @param enable   True to enable the PWM function of the associated GPIO,
 *                  false to disconnect GPIO from PWM.
 *
 *  @return True on success, false if invalid pin was specified.
 */
GP_API Bool halPwmxl_enableChannel(UInt8 channel, Bool enable);

/** @brief Returns if the @p channel is mapped to a GPIO pin.
 *
 *  @return True when the channel is mapped, false otherwise.
 */
GP_API Bool halPwmxl_isChannelEnabled(UInt8 channel);

/** @brief Set output active time of a PWM channel in clock cycles.
 *
 *  @param channel    PWM channel number.
 *  @param dutyCycle  Cycle count.
 */
GP_API void halPwmxl_setDutyCycleRaw(UInt8 channel, UInt16 dutyCycle);

/** @brief Set duty cycle (output active ratio) of a PWM channel.
 *
 *  dutyCyclePercent is set in 0.01 percent increments
 *
 *  @param channel           PWM channel number.
 *  @param dutyCyclePercent  Active output time ratio in range 0 .. 10000 (100%).
 */
GP_API void halPwmxl_setDutyCycle(UInt8 channel, UInt16 dutyCyclePercent);

/** @brief Set phase shift of an output active state in clock cycles.
 *
 *  @param channel  PWM channel number.
 *  @param phase    Cycle count.
 */
GP_API void halPwmxl_setPhaseRaw(UInt8 channel, UInt16 phase);

/** @brief Set phase shift of an output active state in microseconds.
 *
 *  @param channel  PWM channel number.
 *  @param phaseUs  Phase shift in microseconds.
 */
GP_API void halPwmxl_setPhaseUs(UInt8 channel, UInt32 phaseUs);

/** @brief Set phase shift of an output active state in percentage of PWM period.
 *
 *  @param channel       PWM channel number.
 *  @param phasePercent  Phase shift time ratio in range 0 .. 10000 (100%).
 */
GP_API void halPwmxl_setPhase(UInt8 channel, UInt16 phasePercent);

/** @brief Set the dithering configuration for the specified PWM channel.
 *
 *  @param channel  PWM channel number.
 *  @param dither   Dithering value (0-31). For more information, please
 *                  refer to the chip databook, chapter 13.8.2 (PWMXL, Advanced features).
 */
GP_API void halPwmxl_setDither(UInt8 channel, UInt8 dither);

/** @brief Enable or disable inverted output for the specified PWM channel.
 *
 *  @param channel  PWM channel number.
 *  @param enable   True to enable inverted output (output low for duration of duty cycle).
 */
GP_API void halPwmxl_setInverted(UInt8 channel, Bool invert);

/** @brief Configure the output pin drive of a PWM channel.
 *
 *  @param channel       PWM channel number.
 *  @param openDrain     True for open draing mode; false for push-pull.
 */
GP_API void halPwmxl_setDrive(UInt8 channel, Bool openDrain);

/** @brief Start the DMA transfer of 16-bit samples.
 *
 *  @param channel      PWM channel number.
 *  @param buffer       Sample buffer.
 *  @param len          Length of the buffer in bytes.
 *  @param dmaCallback  Callback function, which is called everytime when
 *                      new data is requested.
 */
GP_API Bool halPwmxl_startDMA(UInt8 channel, UInt16* buffer, UInt16 len,
                              pwmxl_callback_t dmaCallback);

/** @brief Generate the arbitrary waveform using DMA tranfer.
 *
 *  @param bufferMapping Maps a specific channel's buffer empty event to
 *                       NEXT_BUFFER_NOT_FULL interrupt in order to trigger/pause DMA action.
 *                       For more information how to use it, please refer to the chip
 *                       databook, chapter 13.8.2 (PWMXL, Advanced features).
 *  @param buffer        Buffer containing structures used to generate the waveform.
 *  @param len           Length of the buffer in bytes.
 *  @param dmaCallback   Callback function, which is called everytime when
 *                       new data is requested.
 */
GP_API Bool halPwmxl_startDMAWaveform(pwmxl_dma_buffermap_t bufferMapping, pwmxl_dma_value_t* buffer,
                                      UInt16 len, pwmxl_callback_t dmaCallback);

#endif // HAL_DIVERSITY_PWMXL

#endif // _HAL_PWMXL_H
