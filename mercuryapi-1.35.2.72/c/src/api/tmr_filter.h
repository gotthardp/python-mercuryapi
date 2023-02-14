/* ex: set tabstop=2 shiftwidth=2 expandtab cindent: */
#ifndef _TMR_FILTER_H
#define _TMR_FILTER_H
/**
 *  @file tmr_filter.h
 *  @brief Mercury API - Tag Filter Interface
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

#include "tmr_tag_data.h"
#include "tmr_gen2.h"
#include "tmr_iso180006b.h"

/**
 * @defgroup filter Tag filters
 *
 * Tag filters cause the reader to limit an inventory to a subset of
 * tags (in a TMR_ReadPlan) or a tag operation to an individual tag
 * (as a parameter to a tag operation function).
 */

#ifdef  __cplusplus
extern "C" {
#endif

/** Type of a TMR_TagFilter structure */
typedef enum TMR_FilterType
{
  /** Tag data filter - non-protocol-specific */
  TMR_FILTER_TYPE_TAG_DATA          = 0,
  /** Gen2 Select filter */
  TMR_FILTER_TYPE_GEN2_SELECT       = 1, 
  /** ISO180006B Select filter */
  TMR_FILTER_TYPE_ISO180006B_SELECT = 2,
  /** UID based filter **/
  TMR_FILTER_TYPE_UID_SELECT = 1,
  /** TagType based filter **/
  TMR_FILTER_TYPE_TAGTYPE_SELECT = 2,
  /** Multi select filter */
  TMR_FILTER_TYPE_MULTI = 3
} TMR_FilterType;

typedef struct TMR_TagFilter TMR_TagFilter;

/** List of tag filters*/
typedef struct TMR_MultiFilter
{
  /* The array of values.
  * These values are pointers to structure. We don't want to make lots of     
  copies. */
  TMR_TagFilter **tagFilterList;
  /** The number of entriess there is space for in the array. */
  uint16_t max;
  /** The number of entries in the lst may be larger than max, indicating 
  truncated data. */
  uint16_t len;
}TMR_MultiFilter;

#ifdef TMR_ENABLE_HF_LF
typedef struct TMR_UID_Select
{
  /** UID to use as a mask */
  uint8_t UIDMask[TMR_MAX_UID_BYTE_COUNT];
  /** Tag UID length in bits*/
  uint8_t UIDMaskBitLen;
}TMR_UID_Select;

typedef struct TMR_TAGTYPE_Select
{
  /** Protocol specific tagtype */
  uint64_t tagType;
}TMR_TAGTYPE_Select;
#endif /* TMR_ENABLE_HF_LF */

/**
 * Structure representing a filter or subset of available tags.
 * @ingroup filter
 */
struct TMR_TagFilter
{
  TMR_FilterType type;
  union
  {
#ifdef TMR_ENABLE_UHF
    /** A particular tag EPC */
    TMR_TagData     tagData;
    /** A Gen2 select operation */
    TMR_GEN2_Select gen2Select;
    /** An ISO180006B select operation */
    TMR_ISO180006B_Select iso180006bSelect;
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_HF_LF
    /** An uid select operation */
    TMR_UID_Select uidSelect;
    /** An tagtype select operation */
    TMR_TAGTYPE_Select tagtypeSelect;
#endif /* TMR_ENABLE_HF_LF */
    /** A list of filters */
    TMR_MultiFilter multiFilterList;
  } u;
};

#ifdef TMR_ENABLE_UHF
/**
 * Test if a tag matches this filter. Only applies to selects based
 * on the EPC.
 * 
 * @param filter The filter to test with.
 * @param tag The tag to test.
 * @return true if the tag matches the filter.
 */
bool TMR_TF_match(TMR_TagFilter *filter, TMR_TagData *tag);

TMR_Status TMR_TF_init_tag(TMR_TagFilter *filter, TMR_TagData *tag);

TMR_Status TMR_TF_init_gen2_select(TMR_TagFilter *filter, bool invert,
                                   TMR_GEN2_Bank bank, uint32_t bitPointer,
                                   uint16_t maskBitLength, uint8_t *mask);

TMR_Status TMR_TF_init_ISO180006B_select(TMR_TagFilter *filter, bool invert,
                                         TMR_ISO180006B_SelectOp op,
                                         uint8_t address, uint8_t mask,
                                         uint8_t wordData[8]);
#endif /* TMR_ENABLE_UHF */

#ifdef TMR_ENABLE_HF_LF
TMR_Status TMR_TF_init_uid_select(TMR_TagFilter *filter,  uint8_t UIDMaskBitLen, uint8_t *UIDMask);
TMR_Status TMR_TF_init_tagtype_select(TMR_TagFilter *filter, uint64_t tagType);
#endif /* TMR_ENABLE_HF_LF */
#ifdef  __cplusplus
}
#endif

#endif /* _TMR_FILTER_H_ */
