/* ex: set tabstop=2 shiftwidth=2 expandtab cindent: */
#ifndef _TMR_TAG_PROTOCOL_H
#define _TMR_TAG_PROTOCOL_H
/**
 *  @file tmr_tag_protocol.h
 *  @brief Mercury API - Tag Protocols
 *  @author Brian Fiegel
 *  @date 4/18/2009
 */

/*
 * Copyright (c) 2009 ThingMagic, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifdef  __cplusplus
extern "C" {
#endif

#include "tmr_gen2.h"
#include "tmr_iso180006b.h"
#include "tmr_ipx.h"
#include "tmr_iso14443a.h"
#include "tmr_iso14443b.h"
#include "tmr_iso15693.h"
#include "tmr_lf125khz.h"
#include "tmr_lf134khz.h"

/**
 * RFID Tag Protocols
 */
typedef enum TMR_TagProtocol 
{
  //UHF Protocols
  TMR_TAG_PROTOCOL_NONE              = 0x00,
  TMR_TAG_PROTOCOL_ISO180006B        = 0x03,
  TMR_TAG_PROTOCOL_GEN2              = 0x05,
  TMR_TAG_PROTOCOL_ISO180006B_UCODE  = 0x06, 
  TMR_TAG_PROTOCOL_IPX64             = 0x07,
  TMR_TAG_PROTOCOL_IPX256            = 0x08,
  TMR_TAG_PROTOCOL_ATA               = 0x1D,

  // HF Protocols
  TMR_TAG_PROTOCOL_ISO14443A         = 0x09,
  TMR_TAG_PROTOCOL_ISO14443B         = 0x0A,
  TMR_TAG_PROTOCOL_ISO15693          = 0x0B,
  TMR_TAG_PROTOCOL_ISO18092          = 0x0C,
  TMR_TAG_PROTOCOL_FELICA            = 0x0D,
  TMR_TAG_PROTOCOL_ISO18000_3M3      = 0x0E,

  // LF Protocols
  TMR_TAG_PROTOCOL_LF125KHZ          = 0x14,
  TMR_TAG_PROTOCOL_LF134KHZ          = 0x15,
} TMR_TagProtocol;

/** A list of TMR_Protocol values */
typedef struct TMR_TagProtocolList
{
  /** The array of values */
  TMR_TagProtocol *list;
  /** The number of entries there is space for in the array */
  uint8_t max;
  /** The number of entries in the list - may be larger than max, indicating truncated data. */
  uint8_t len;
} TMR_TagProtocolList;

#ifdef  __cplusplus
}
#endif


#endif /* _TMR_TAG_PROTOCOL_H_ */
