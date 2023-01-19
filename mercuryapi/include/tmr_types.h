/* ex: set tabstop=2 shiftwidth=2 expandtab cindent: */
#ifndef _TMR_TYPES_H
#define _TMR_TYPES_H
/**
 *  @file tmr_types.h
 *  @brief Mercury API - General data type definitions
 *  @author Nathan Williams
 *  @date 10/20/2009
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

#ifdef WINCE
#include <stdint_win32.h>
#else
#include <stdint.h>
#endif
#include <stdbool.h>

#ifdef  __cplusplus
extern "C" {
#endif

/** Internal string representation with known storage size */
typedef struct TMR_String
{
  /** The null-terminated string value */
  char *value;
  /** The allocated number of bytes pointed to */
  uint16_t max;
} TMR_String;

/** List of uint8_t values */
typedef struct TMR_uint8List
{
  /** The array of values */
  uint8_t *list;
  /** The number of entries there is space for in the array */
  uint16_t max;
  /** The number of entries in the list - may be larger than max, indicating truncated data. */
  uint16_t len;
} TMR_uint8List;

/** List of uint16_t values */
typedef struct TMR_uint16List
{
  /** The array of values */
  uint16_t *list;
  /** The number of entries there is space for in the array */
  uint16_t max;
  /** The number of entries in the list - may be larger than max, indicating truncated data. */
  uint16_t len;
} TMR_uint16List;

/** List of uint32_t values */
typedef struct TMR_uint32List
{
  /** The array of values */
  uint32_t *list;
  /** The number of entries there is space for in the array */
  uint16_t max;
  /** The number of entries in the list - may be larger than max, indicating truncated data. */
  uint16_t len;
} TMR_uint32List;

/** List of int8_t values */
typedef struct TMR_int8List
{
  /** The array of values */
  int8_t *list;
  /** The number of entries there is space for the array */
  int8_t max;
  /** The number of entries in the list - may be larger than max, indicating truncated data. */
  int8_t len;
}TMR_int8List;

/**
 * Function callback used to obtain firmware data during a firmware
 * load operation.  TMR_firmwareLoad will call this repeatedly to
 * obtain data to send to the reader. A buffer of a specified size
 * will be passed in; the callback is free to place less than that
 * amount of data in the buffer, provided that size is modified
 * accordingly. When the end of the firmware image has been reached,
 * the callback should return false.  

 * @see TMR_memoryProvider and TMR_fileProvider for pre-existing
 * callbacks to load firmware from system memory or from a file.
 * 
 * @param cookie Arbitrary data passed to the callback.
 * @param[in] size Pointer to amount of space in the buffer.
 * @param[out] Points to number of bytes stored in the buffer.
 * @param[out] data Buffer of bytes of firmware image.
 * @return true if there is more data to load
 */
typedef bool (*TMR_FirmwareDataProvider)(void *cookie, uint16_t *size, uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif /* _TMR_TYPES_H */
