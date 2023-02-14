/* ex: set tabstop=2 shiftwidth=2 expandtab cindent: */
#ifndef _TMR_TAG_LOCK_ACTION_H
#define _TMR_TAG_LOCK_ACTION_H
/**
 *  @file tmr_tag_lock_action.h
 *  @brief Mercury API - Tag Lock Action Interface
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
#ifdef TMR_ENABLE_UHF
#include "tmr_gen2.h"
#include "tmr_iso180006b.h"

/**
 * Types of RFID Tag Locking Structures
 */
typedef enum TMR_TagLockActionType
{
  TMR_LOCK_ACTION_TYPE_GEN2_LOCK_ACTION       = 0,
  TMR_LOCK_ACTION_TYPE_ISO180006B_LOCK_ACTION = 1
} TMR_TagLockActionType;

/**
 * Tag locking structure
 */
typedef struct TMR_TagLockAction
{
  /** 
   * Type of the included locking structure. 
   * Indicates which member of the union is valid.
   */
  TMR_TagLockActionType type;
  union
  {
    /** Gen2 locking data */
    TMR_GEN2_LockAction gen2LockAction;
    /** ISO180006B locking data */
    TMR_IS0180006B_LockAction iso180006bLockAction;
  } u;
}TMR_TagLockAction;

TMR_Status TMR_TLA_init_gen2(TMR_TagLockAction *lockAction, uint16_t mask,
                             uint16_t action);

TMR_Status TMR_TLA_init_ISO180006B(TMR_TagLockAction *lockAction,
                                   uint8_t address);
#endif /* TMR_ENABLE_UHF */

#ifdef  __cplusplus
}
#endif

#endif /* _TMR_TAG_LOCK_ACTION_H_ */
