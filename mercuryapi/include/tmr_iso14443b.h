/* ex: set tabstop=2 shiftwidth=2 expandtab cindent: */
#ifndef _TMR_ISO14443B_H
#define _TMR_ISO14443B_H
/** 
 *  @file tmr_iso14443b.h  
 *  @brief Mercury API - ISO14443B tag information and interfaces
 *  @author Anjali Panchal
 *  @date 3/30/2020
 */

/*
 * Copyright (c) 2020 ThingMagic, Inc.
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

/** Defines ISO14443B protocol supported tagTypes */
typedef enum TMR_ISO14443B_TagType
{
  TMR_ISO14443B_TAGTYPE_AUTO_DETECT                          = 0x00000001,
  TMR_ISO14443B_TAGTYPE_CALYSO                               = 0x00000002,
  TMR_ISO14443B_TAGTYPE_CALYPSO_INNOVATRON_PROTOCOL          = 0x00000004,
  TMR_ISO14443B_TAGTYPE_CTS                                  = 0x00000008,	
  TMR_ISO14443B_TAGTYPE_MONEO                                = 0x00000010,
  TMR_ISO14443B_TAGTYPE_PICO_PASS                            = 0x00000020,
  TMR_ISO14443B_TAGTYPE_SRI4K                                = 0x00000040,
  TMR_ISO14443B_TAGTYPE_SRIX4K                               = 0x00000080,
  TMR_ISO14443B_TAGTYPE_SRI512                               = 0x00000100,
  TMR_ISO14443B_TAGTYPE_SRT512                               = 0x00000200,
}TMR_ISO14443B_TagType;

#ifdef __cplusplus
}
#endif

#endif /*_TMR_ISO14443B_H*/
