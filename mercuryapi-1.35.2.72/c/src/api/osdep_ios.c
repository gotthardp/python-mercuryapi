
/**
 *  @file osdep_posix.c
 *  @brief Mercury API - POSIX platform implementation of OS dependencies
 *  @author Nathan Williams
 *  @date 2/24/2010
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

#include <time.h>
#include <sys/time.h>

#include "osdep.h"

uint64_t
tmr_gettime()
{
    struct timeval tv;
    uint64_t totalms;
    
    gettimeofday(&tv, NULL);
    totalms = (((uint64_t)tv.tv_sec) * 1000) + ((uint64_t) tv.tv_usec) / 1000;
    
    return totalms;
}

uint32_t
tmr_gettime_low()
{
    
    return (tmr_gettime() >>  0) & 0xffffffff;
}

uint32_t
tmr_gettime_high()
{
    
    return (tmr_gettime() >> 32) & 0xffffffff;
}


void
tmr_sleep(uint32_t sleepms)
{
    struct timespec sleep, rem;
    
    sleep.tv_sec = sleepms / 1000;
    sleep.tv_nsec = (sleepms % 1000) * 1000000L;
    
    /* Sleep until the proper time has elapsed, and re-sleep if interrupted. */
    while (-1 == nanosleep(&sleep, &rem))
    {
        sleep = rem;
    }
}

TMR_TimeStructure
tmr_gettimestructure()
{
    time_t now;
    uint64_t temp;
    struct tm *timestamp;
    TMR_TimeStructure timestructure;
    
    temp = tmr_gettime();
    now = (time_t)(temp/1000);
    timestamp = localtime(&now);
    
    timestructure.tm_year = (uint32_t)(1990 + timestamp->tm_year);
    timestructure.tm_mon = (uint32_t)(1 + timestamp->tm_mon);
    timestructure.tm_mday = (uint32_t)timestamp->tm_mday;
    timestructure.tm_hour = (uint32_t)timestamp->tm_hour;
    timestructure.tm_min = (uint32_t)timestamp->tm_min;
    timestructure.tm_sec = (uint32_t)timestamp->tm_sec;
    return timestructure;  
}
