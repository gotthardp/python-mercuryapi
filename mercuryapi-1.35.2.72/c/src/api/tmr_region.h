/* ex: set tabstop=2 shiftwidth=2 expandtab cindent: */
#ifndef _TMR_REGION_H
#define _TMR_REGION_H
/**
 *  @file tmr_region.h
 *  @brief Mercury API - Region Definitions
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

/**
  * RFID regulatory regions
 */
typedef enum TMR_Region
{
  /** Unspecified region */         TMR_REGION_NONE = 0,
  /** North America */              TMR_REGION_NA   = 1,
  /** European Union */             TMR_REGION_EU   = 2,
  /** Korea */                      TMR_REGION_KR   = 3,
  /** India */                      TMR_REGION_IN   = 4,
  /** Japan */                      TMR_REGION_JP   = 5,
  /** People's Republic of China */ TMR_REGION_PRC  = 6,
  /** European Union 2 */           TMR_REGION_EU2  = 7,
  /** European Union 3 */           TMR_REGION_EU3  = 8,
  /** Korea 2*/                     TMR_REGION_KR2  = 9,
  /** People's Republic of China(840MHZ)*/TMR_REGION_PRC2 = 10,
  /** Australia */                  TMR_REGION_AU   = 11,
  /** New Zealand !!EXPERIMENTAL!! */ TMR_REGION_NZ   = 12,
  /** Reduced FCC region */         TMR_REGION_NA2 = 13,
  /** 5MHZ FCC band */              TMR_REGION_NA3 = 14,
  /** Israel  **/                   TMR_REGION_IS = 15,
  /** Malaysia  **/                 TMR_REGION_MY = 16,
  /** Indonesia  **/                TMR_REGION_ID = 17,
  /** Philippines  **/              TMR_REGION_PH = 18,
  /** Taiwan  **/                   TMR_REGION_TW = 19,
  /** Macau  **/                    TMR_REGION_MO = 20,
  /** Russia  **/                   TMR_REGION_RU = 21,
  /** Singapore  **/                TMR_REGION_SG = 22,
  /** Japan2 (24dBm with 13 channels) */ TMR_REGION_JP2 = 23,
  /** Japan3 (24dBm with 6 channels) */ TMR_REGION_JP3 = 24,
  /** Vietnam **/                   TMR_REGION_VN = 25,
  /** Thailand **/                  TMR_REGION_TH = 26,
  /** Argentina **/                 TMR_REGION_AR = 27,
  /** Hong Kong **/                 TMR_REGION_HK = 28,
  /** Bangladesh **/                TMR_REGION_BD = 29,
  /** European Union 4 
      4 channels (916.3MHz, 
      917.5MHz,
      918.7MHz)**/        TMR_REGION_EU4 = 30,
  /** Universal region 
      applicable for M3e product */ TMR_REGION_UNIVERSAL = 31,
  /** Israel2(IS2) region 
  applicable for Micro and Nano */ TMR_REGION_IS2 = 32,
  /** NA4 region 
  applicable for Micro and M6e */ TMR_REGION_NA4 = 33,
  /** OPEN region with extended 
      frequency range 840-960MHz 
      for M6ePlus module**/         TMR_REGION_OPEN_EXTENDED = 254,

  /** Open */                       TMR_REGION_OPEN = 0xFF
} TMR_Region;

/** A list of TMR_Region values */
typedef struct TMR_RegionList
{
  /** Array of TMR_Region values */
  TMR_Region *list;
  /** Allocated size of the array */
  uint8_t max;
  /** Length of the list - may be larger than max, indicating truncated data */
  uint8_t len;
} TMR_RegionList;

#ifdef  __cplusplus
}
#endif

#endif /* _TMR_REGION_H_ */
