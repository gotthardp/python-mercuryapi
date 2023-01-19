/* ex: set tabstop=2 shiftwidth=2 expandtab cindent: */
#ifndef _TMR_LF125KHZ_H
#define _TMR_LF125KHZ_H
/** 
 *  @file tmr_lf125khz.h  
 *  @brief Mercury API - LF125Khz tag information and interfaces
 *  @author Anjali Panchal
 *  @date 2/24/2020
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
#ifdef TMR_ENABLE_HF_LF
/** Defines LF125KHZ protocol supported tagTypes */
typedef enum TMR_LF125KHZ_TagType
{
  TMR_LF125KHZ_TAGTYPE_AUTO_DETECT          = 0x00000001,
  TMR_LF125KHZ_TAGTYPE_HID_PROX             = 0x01000000,
  TMR_LF125KHZ_TAGTYPE_AWID                 = 0x02000000,
  TMR_LF125KHZ_TAGTYPE_KERI                 = 0x04000000,
  TMR_LF125KHZ_TAGTYPE_INDALA               = 0x08000000,
  TMR_LF125KHZ_TAGTYPE_HITAG_2              = 0x10000000,
  TMR_LF125KHZ_TAGTYPE_HITAG_1              = 0x20000000,
  TMR_LF125KHZ_TAGTYPE_EM_4100              = 0x40000000,
}TMR_LF125KHZ_TagType;
#endif /* TMR_ENABLE_HF_LF */
#ifdef __cplusplus
}
#endif

#endif /*_TMR_LF125KHZ_H*/
