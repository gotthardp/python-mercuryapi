/* ex: set tabstop=2 shiftwidth=2 expandtab cindent: */
#ifndef _TMR_ISO180006B_H
#define _TMR_ISO180006B_H
/**
 *  @file tmr_iso180006b.h  
 *  @brief Mercury API - ISO180006B tag information and interfaces
 *  @author Brian Fiegel
 *  @date 5/7/2009
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

/** ISO180006B Select operations */
typedef enum TMR_ISO180006B_SelectOp
{
  /** Select if tag data matches op data */
  TMR_ISO180006B_SELECT_OP_EQUALS       = 0,
  /** Select if tag data does not match op data */
  TMR_ISO180006B_SELECT_OP_NOT_EQUALS   = 1,
  /** Select if tag data is less than op data */
  TMR_ISO180006B_SELECT_OP_LESS_THAN    = 2,
  /** Select if tag data is greater than op data */
  TMR_ISO180006B_SELECT_OP_GREATER_THAN = 3
} TMR_ISO180006B_SelectOp;

/**
 * A single selection operation in the ISO18000-6B protocol.
 * @ingroup filter
 */ 
typedef struct TMR_ISO180006B_Select
{
  /** Whether to invert the selection (deselect tags that meet the comparison). */
  bool invert;
  /** The operation to use to compare the tag data to the provided data. */
  TMR_ISO180006B_SelectOp op;
  /** The address of the tag memory to compare to the provided data. */
  uint8_t address;
  /** Bitmask of which of the eight provided data bytes to compare to the tag memory. */
  uint8_t mask;
  /** The data to compare. Exactly eight bytes. */
  uint8_t data[8];
} TMR_ISO180006B_Select;

/**
 * The arguments to a TMR_lockTag() method for ISO18000-6B tags. 
 * Represents the byte to be locked.
 */
typedef struct TMR_ISO180006B_LockAction
{
  /** The memory address of the byte to lock */
  uint8_t address;
} TMR_IS0180006B_LockAction;

/** ISO180006B link frequencies */
typedef enum TMR_ISO180006B_LinkFrequency
{
  /** 40 kHz */
  TMR_ISO180006B_LINKFREQUENCY_40KHZ  = 40,
  /** 160 kHz */
  TMR_ISO180006B_LINKFREQUENCY_160KHZ = 160,
} TMR_ISO180006B_LinkFrequency;

/** ISO180006B modulation depth */
typedef enum TMR_ISO180006B_ModulationDepth
{
  /** 99 percent modulation */
  TMR_ISO180006B_Modulation99percent = 0x00,
  /** 11 percent modulation */
  TMR_ISO180006B_Modulation11percent = 0x01
}TMR_ISO180006B_ModulationDepth;

/** ISO180006B Delimiter */
typedef enum TMR_ISO180006B_Delimiter
{
  /** delimiter 1*/
  TMR_ISO180006B_Delimiter1 = 0x01,
  /** delimiter 4 */
  TMR_ISO180006B_Delimiter4 = 0x04
}TMR_ISO180006B_Delimiter;
#ifdef __cplusplus
}
#endif

#endif /*_TMR_ISO180006B_H*/
