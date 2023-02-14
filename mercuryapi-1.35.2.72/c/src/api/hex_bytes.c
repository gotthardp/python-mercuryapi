/**
 *  @file hex_bytes.c
 *  @brief Routines for converting between byte arrays and hex strings
 *  @author Nathan Williams
 *  @date 10/23/2009
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

#include <stdlib.h>

#include "tm_reader.h"
#include "serial_reader_imp.h"

static char hexchars[] = "0123456789ABCDEF";

#ifdef TMR_ENABLE_UHF
TMR_Status
TMR_hexToBytes(const char *hex, uint8_t *bytes, uint32_t size,
               uint32_t *convertLen)
{
  char c;
  int i, len;
  uint8_t val[2];

  if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X'))
  {
    hex += 2;
  }

  len = 0;
  while (size > 0 && *hex != '\0')
  {
    for (i = 0; i < 2; i++)
    {
      c = *hex++;
      if (c >= '0' && c <= '9')
        val[i] = c - '0';
      else if (c >= 'a' && c <= 'f')
        val[i] = 10 + c - 'a';
      else if (c >= 'A' && c <= 'F')
        val[i] = 10 + c - 'A';
      else
        return TMR_ERROR_INVALID;
    }
    *bytes = (val[0] << 4) | val[1];

    len++;
    bytes++;
    size--;
  }

  if (convertLen != NULL)
  {
    *convertLen = len;
  }

  return TMR_SUCCESS;
}
#endif /* TMR_ENABLE_UHF */

void
TMR_bytesToHex(const uint8_t *bytes, uint32_t size, char *hex)
{

  while (size--)
  {
    *hex++ = hexchars[*bytes >> 4];
    *hex++ = hexchars[*bytes & 15];
    bytes++;
  }
  *hex = '\0';
}

/**
 * Convert a four-byte array into a null-terminated string
 * with the format %02X.%02X.%02X.%02X (AA.BB.CC.DD).
 *
 * @param bytes The array of four bytes to convert
 * @param buf The string to write to. Must be at least 12 bytes.
 */
void
TMR_hexDottedQuad(const uint8_t bytes[4], char buf[12])
{
  int i;

  for (i = 0; i < 4 ; i++)
  {
    *buf++ = hexchars[*bytes >> 4];
    *buf++ = hexchars[*bytes & 15];
    *buf++ = '.';
    bytes++;
  }
  *--buf = '\0';
}
#ifdef TMR_ENABLE_UHF
/**
 * Convert a 12-byte null-terminated string hexDottedQuad into a uint32_t
 *
 * @param bytes The array of twelve bytes to convert
 * @param result The resulting uint32_t
 * @return uint32_t represented but the 12 bytes
 */
TMR_Status
TMR_hexDottedQuadToUint32(const char bytes[12], uint32_t *result)
{
  TMR_Status retVal;
  uint32_t tmpResult;
  uint8_t byteVal;
  int i,j;

  tmpResult=0;
  for (i = 0, j = 0; i < 12 ; i+=3,++j)
  {
    retVal = TMR_hexToBytes(&bytes[i], &byteVal, 1, NULL);
    if (TMR_SUCCESS == retVal)
    {
      tmpResult |= (byteVal << (8 * (3-j)));
    }
    else
    {
      return retVal;
    }
  }

  if (NULL != result)
  {
    *result = tmpResult;
  }
  return retVal;
}
#endif /* TMR_ENABLE_UHF */
