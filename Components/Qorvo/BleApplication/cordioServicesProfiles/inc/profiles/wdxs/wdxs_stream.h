/*!
 *  \file   wdxs_stream.h
 *
 *  \brief  Wireless Data Exchange profile implementation - Stream Example.
 *
 *  Copyright (c) 2013-2017 ARM Ltd. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
/*************************************************************************************************/

#ifndef WDXS_STREAM_H
#define WDXS_STREAM_H

/**************************************************************************************************
  Constant Definitions
**************************************************************************************************/

/*! Type of waveform to output from the Example Stream */
#define WDXS_STREAM_WAVEFORM_SINE       0
#define WDXS_STREAM_WAVEFORM_STEP       1
#define WDXS_STREAM_WAVEFORM_SAWTOOTH   2

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \fn     wdxsStreamInit
 *
 *  \brief  Example of creating a WDXS stream.
 *
 *  \param  none
 *
 *  \return None.
 */
/*************************************************************************************************/
void wdxsStreamInit(void);

/*************************************************************************************************/
/*!
 *  \fn     wdxsSetStreamWaveform
 *
 *  \brief  Changes the type of waveform transmitted by the stream.
 *
 *  \param  type - Identifier of the waveform
 *
 *  \return None.
 */
/*************************************************************************************************/
void wdxsSetStreamWaveform(uint8_t type);

#endif /* WDXS_STREAM_H */

