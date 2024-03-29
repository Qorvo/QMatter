/*!
 *  \file   wdxs_stream.c
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

#include <string.h>
#include "wsf_types.h"
#include "util/wstr.h"
#include "wsf_trace.h"
#include "wsf_assert.h"
#include "wsf_efs.h"
#include "util/bstream.h"
#include "svc_wdxs.h"
#include "wdxs_api.h"
#include "wdxs_main.h"
#include "wdxs_stream.h"
#include "dm_api.h"
#include "att_api.h"
#include "app_api.h"


/*! Type of waveform to output from the Example Stream */
static uint8_t wdxsStreamWaveform = WDXS_STREAM_WAVEFORM_SAWTOOTH;

/*************************************************************************************************/
/*
 * Note: This file contains an example of creating Wireless Data Exchange
 * (WDXS) Streams.  WDXS Streams are implemented as virtual Physical Media in
 * the Embedded File System (EFS).  A Stream can be created in three steps:
 *
 * Step 1: Create a FileMedia_t (EFS Media Control structure) for the stream
 *         containing the read function created in step 2.
 *
 * Step 2: Implement a read function for the stream.  The WDXS and EFS will
 *         call the read function to get data from the stream.
 *
 * Step 3: Register the media with the EFS, and add a file to the embedded
 *         file system that uses the media created in step 2.
 */
/*************************************************************************************************/

/*************************************************************************************************/
/* Step 1: Media Control Block */
/*************************************************************************************************/

/* Prototype of stream read function */
static uint8_t wdxsStreamRead(uint8_t *pBuf, uint32_t address, uint32_t len);

/* Example media control structure for a stream */
static const wsfEfsMedia_t WDXS_StreamMedia =
{
  0,
  0,
  0,
  NULL,
  NULL,
  wdxsStreamRead,
  NULL,
  NULL
};

/*************************************************************************************************/
/* Step 2: Read Function */
/*************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \fn     wdxsSineRead
 *
 *  \brief  Example of a media read function that generates a Sine Wave.
 *
 *  \param  pBuf     buffer to hold stream data.
 *  \param  address  unused in streams.
 *  \param  len      size of pBuf in bytes.
 *
 *  \return None.
 */
/*************************************************************************************************/
static uint8_t wdxsSineRead(uint8_t *pBuf, uint32_t address, uint32_t len)
{
  static int8_t incr = 1;
  static uint8_t dataVal = 0;

  /* Build data in sine waveform */
  memset(pBuf, dataVal, len);

  if (dataVal <= 127)
  {
    incr++;
  }
  else
  {
    incr--;
  }

  dataVal += (incr / 2);

  return TRUE;
}

/*************************************************************************************************/
/*!
 *  \fn     wdxsStepRead
 *
 *  \brief  Example of a media read function  that generates a Step Wave.
 *
 *  \param  pBuf     buffer to hold stream data.
 *  \param  address  unused in streams.
 *  \param  len      size of pBuf in bytes.
 *
 *  \return None.
 */
/*************************************************************************************************/
static uint8_t wdxsStepRead(uint8_t *pBuf, uint32_t address, uint32_t len)
{
  static int8_t count = 0;
  static int8_t incr = 25;
  static uint8_t dataVal = 0;

  /* Build data in step waveform */
  memset(pBuf, dataVal, len);

  if (count++ == 5)
  {
    count = 0;
    dataVal += incr;

    if (dataVal == 0)
      incr = 25;

    if (dataVal == 250)
      incr = -25;
  }

  return TRUE;
}

/*************************************************************************************************/
/*!
 *  \fn     wdxsSawtoothRead
 *
 *  \brief  Example of a media read function that generates a Sawtooth Wave.
 *
 *  \param  pBuf     buffer to hold stream data.
 *  \param  address  unused in streams.
 *  \param  len      size of pBuf in bytes.
 *
 *  \return None.
 */
/*************************************************************************************************/
static uint8_t wdxsSawtoothRead(uint8_t *pBuf, uint32_t address, uint32_t len)
{
  static int8_t incr = 1;
  static uint8_t dataVal = 0;
  uint16_t i;

  /* Build data in sawtooth waveform */
  for (i=0; i<len; i++)
  {
    *pBuf++ = dataVal;
    dataVal += incr;

    if (dataVal == 0)
      incr = 1;
    else if (dataVal == 255)
      incr = -1;
  }

  return TRUE;
}

/*************************************************************************************************/
/*!
 *  \fn     wdxsStreamRead
 *
 *  \brief  Example of a media read function.
 *
 *  \param  pBuf     buffer to hold stream data.
 *  \param  address  unused in streams.
 *  \param  len      size of pBuf in bytes.
 *
 *  \return None.
 */
/*************************************************************************************************/
static uint8_t wdxsStreamRead(uint8_t *pBuf, uint32_t address, uint32_t len)
{
  //APP_TRACE_INFO1("WDXS: wdxsStreamRead addr %u, len %u", address, len);
  wdxsCb.streamingBytesSent += len;
  switch(wdxsStreamWaveform)
  {
  case WDXS_STREAM_WAVEFORM_SINE:
    wdxsSineRead(pBuf, address, len);
    break;

  case WDXS_STREAM_WAVEFORM_STEP:
    wdxsStepRead(pBuf, address, len);
    break;

  case WDXS_STREAM_WAVEFORM_SAWTOOTH:
    wdxsSawtoothRead(pBuf, address, len);
    break;

  default:
    break;
  }

  return TRUE;
}

/*************************************************************************************************/
/* Step 3: Register the stream media and adding the stream file. */
/*************************************************************************************************/

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
void wdxsStreamInit(void)
{
  wsfEsfAttributes_t attr;


  /* Register the media for the stream */
  WsfEfsRegisterMedia(&WDXS_StreamMedia, WDX_STREAM_MEDIA);

  /* Set the attributes for the stream */
  attr.permissions = WSF_EFS_REMOTE_VISIBLE | WSF_EFS_REMOTE_GET_PERMITTED;
  attr.type = WSF_EFS_FILE_TYPE_STREAM;
  WstrnCpy(attr.name, "Qorvotest", WSF_EFS_NAME_LEN);
  WstrnCpy(attr.version, "1.0", WSF_EFS_VERSION_LEN);

  /* Add a file for the stream */
  WsfEfsAddFile(0, WDX_STREAM_MEDIA, &attr, 0);
}

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
void wdxsSetStreamWaveform(uint8_t type)
{
  if (type <= WDXS_STREAM_WAVEFORM_SAWTOOTH)
  {
    wdxsStreamWaveform = type;
  }
}
