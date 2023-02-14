/**
 *  @file tmr_utils.c
 *  @brief Mercury API - utility functions
 *  @author Nathan Williams
 *  @date 12/10/2009
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

#include <stddef.h>

#include "osdep.h"
#include "tmr_types.h"
#include "tmr_utils.h"

#ifndef TMR_USE_HOST_C_LIBRARY

void *
tm_memcpy(void *dest, const void *src, size_t n)
{
  const uint8_t *u8src;
  uint8_t *u8dest;

  u8dest = dest;
  u8src = src;

  while (0 != n)
  {
    *u8dest = *u8src;
    u8dest++;
    u8src++;
    n--;
  }

  return dest;
}

char *
tm_strcpy(char *dest, const char *src)
{
  char *ptr, val;

  ptr = dest;

  do
  {
    val = *src;
    *ptr = val;
    ptr++;
    src++;
  } 
  while ('\0' != val);

  return dest;
}

char *
tm_strchr(const char *s, int c)
{
  const char *ptr;

  for (ptr = s; *ptr != '\0'; ptr++)
  {
    if (*ptr == c)
    {
      return (char *)ptr;
    }
  }

  return NULL;
}

#endif /* TMR_USE_HOST_C_LIBRARY */

int
tm_strcasecmp(const char *s1, const char *s2)
{
  char v1, v2;
  int diff;

  do
  {
    v1 = *s1;
    if (v1 >= 'a' && v1 <= 'z')
      v1 -= 'a' - 'A';
    v2 = *s2;
    if (v2 >= 'a' && v2 <= 'z')
      v2 -= 'a' - 'A';
    s1++;
    s2++;
  }
  while (v1 == v2 && '\0' != v1 && '\0' != v2);

  diff = v1 - v2;
  if (diff < 0)
    return -1;
  else if (diff > 0)
    return 1;
  else
    return 0;
}

/* Get a high/low timestamp pair, being careful not to get one value
 * before a wraparound and the other one after, which would lead to
 * 50-day time jumps forward or backwards.  (high<<32)|low is in milliseconds.
 */
void
tm_gettime_consistent(uint32_t *high, uint32_t *low)
{
  uint32_t tmpHigh;

  *high   = tmr_gettime_high();
  *low    = tmr_gettime_low();
  tmpHigh = tmr_gettime_high();

  if (tmpHigh != *high)
  {
    *high = tmpHigh;
    *low = tmr_gettime_low();
  }
}

/* Find the time difference from start to end, allowing for end having
 * wrapped around UINT32_MAX and back to zero.
 */
uint32_t
tm_time_subtract(uint32_t end, uint32_t start)
{
  if (end >= start)
    return end - start;
  else
    return (UINT32_MAX - start) + end;
}

/** Minimum number of bytes required to hold a given number of bits.
 *
 * @param bitCount  number of bits to hold
 * @return  Minimum length of bytes that can contain that many bits
 */
int tm_u8s_per_bits(int bitCount) 
{
	return ((0<bitCount) ?((((bitCount)-1)>>3)+1) :0);
}

void
TMR_stringCopy(TMR_String *dest, const char *src, int len)
{
  if (dest->max - 1 < len)
  {
    len = dest->max - 1;
  }
  if (dest->max > 0)
  {
    memcpy(dest->value, src, len);
    dest->value[len] = '\0';
  }
}

uint64_t
TMR_makeBitMask(int offset, int length)
{
  uint64_t mask;

  mask = 0;
  mask = ~mask;
  mask <<= length;
  mask = ~mask;

  mask <<= offset;

  return mask;
}

uint32_t
TMR_byteArrayToInt(uint8_t data[], int offset)
{
  uint32_t value = 0;
  int count;

  for( count = 0; count < 4; count++)
  {
    value <<= 8;
    value ^= (data[count + offset] & 0x000000FF);
  }
  return value;
}

uint16_t
TMR_byteArrayToShort(uint8_t data[], int offset)
{
  uint16_t value = 0;
  int hi, lo;
  hi = (uint16_t) (data[offset++]) << 8;
  lo = (uint16_t) (data[offset++]);
  value = (uint16_t)(hi | lo);

  return value;
}

uint64_t
TMR_byteArrayToLong(uint8_t data[], int offset)
{
  uint64_t value = 0;
  int i;
  for (i = 0; i < 8; i++)
  {
    value <<= 8;
    value ^= (uint64_t) (data[i + offset] & 0xff);
  }
  return value;
}

/**
 * This function will convert bytes into words format.
 *
 * @param count: the number of bytes to convert
 * @param data: the data to convert into words
 * @param data16: buffer to store the converted data
 */

void
TMR_bytesToWords(uint16_t count, const uint8_t data[], uint16_t data16[])
{
  uint16_t wordCount;
  uint16_t iWord;
  wordCount = count/2;
  for (iWord=0; iWord<wordCount; iWord++)
  {
    data16[iWord] = 0;
    data16[iWord] |= data[(2*iWord)+0];
    data16[iWord] <<= 8;
    data16[iWord] |= data[(2*iWord)+1];
  }
}

/**
 * This function will convert words into bytes format.
 *
 * @param count: the number of words to convert
 * @param data: the data to convert into bytes
 * @param buf: buffer to store the converted data
 */

void
TMR_wordsToBytes(uint16_t count, const uint16_t data[], uint8_t buf[])
{
  int i;

  for (i = 0 ; i < count ; i++)
  {
    buf[2*i    ] = data[i] >> 8;
    buf[2*i + 1] = data[i] & 0xff;
  }
}

