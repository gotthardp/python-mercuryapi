/**
 *  @file osdep_arduino.c
 *  @brief Mercury API - Arduino platform implementation of OS dependencies
 *  @author Brian Fiegel
 *  @date 11/16/2010
 */

/*
 * Copyright (c) 2010 ThingMagic, Inc.
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

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "osdep.h"
#include "Time.h"

uint64_t
tmr_gettime(void)
{
  return millis();
}

uint32_t
tmr_gettime_low(void)
{
  return (tmr_gettime() >>  0) & 0xffffffff;
}

uint32_t
tmr_gettime_high(void)
{
  return (tmr_gettime() >> 32) & 0xffffffff;
}

void
tmr_sleep(uint32_t sleepms)
{
  delay(sleepms);
}

TMR_TimeStructure
tmr_gettimestructure()
{
  uint64_t temp;
  time_t now;
  TMR_TimeStructure timestructure;
  static tmElements_t elements;

  temp = tmr_gettime();
  now = temp/1000;

  breakTime(now, elements);  // break time_t into elements
  timestructure.tm_year = (uint32_t)(1990 + elements.Year);
  timestructure.tm_mon = (uint32_t)(1 + elements.Month);
  timestructure.tm_mday = (uint32_t)elements.Day;
  timestructure.tm_hour = (uint32_t)elements.Hour;
  timestructure.tm_min = (uint32_t)elements.Minute;
  timestructure.tm_sec = (uint32_t)elements.Second;
  return timestructure;
}
