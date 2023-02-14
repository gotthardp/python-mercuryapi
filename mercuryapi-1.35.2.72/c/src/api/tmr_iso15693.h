/* ex: set tabstop=2 shiftwidth=2 expandtab cindent: */
#ifndef _TMR_ISO15693_H
#define _TMR_ISO15693_H
/** 
 *  @file tmr_iso15693.h  
 *  @brief Mercury API - ISO15693 tag information and interfaces
 *  @author Anjali Panchal
 *  @date 1/30/2020
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
/** Defines ISO15693 protocol supported tagTypes */
typedef enum TMR_ISO15693_TagType
{
  TMR_ISO15693_TAGTYPE_AUTO_DETECT             = 0x00000001,
  TMR_ISO15693_TAGTYPE_HID_ICLASS_SE           = 0x00000100,
  TMR_ISO15693_TAGTYPE_ICODE_SLI               = 0x00000200,
  TMR_ISO15693_TAGTYPE_ICODE_SLI_L             = 0x00000400,
  TMR_ISO15693_TAGTYPE_ICODE_SLI_S             = 0x00000800,
  TMR_ISO15693_TAGTYPE_ICODE_DNA               = 0x00001000,
  TMR_ISO15693_TAGTYPE_ICODE_SLIX              = 0x00002000,
  TMR_ISO15693_TAGTYPE_ICODE_SLIX_L            = 0x00004000,
  TMR_ISO15693_TAGTYPE_ICODE_SLIX_S            = 0x00008000,
  TMR_ISO15693_TAGTYPE_ICODE_SLIX_2            = 0x00010000,
  TMR_ISO15693_TAGTYPE_VIGO                    = 0x00020000,
  TMR_ISO15693_TAGTYPE_TAGIT                   = 0x00040000,
  TMR_ISO15693_TAGTYPE_PICOPASS                = 0x00080000,
}TMR_ISO15693_TagType;

#endif /* TMR_ENABLE_HF_LF */

#ifdef __cplusplus
}
#endif

#endif /*_TMR_ISO15693_H*/
