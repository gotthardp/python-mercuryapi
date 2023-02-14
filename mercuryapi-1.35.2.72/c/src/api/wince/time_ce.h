
///////////////////////////////////////////////////////////////////////////////
//
// Legalese:
// This file is part of the OpenNETCF time.h port for Windows CE.
// You are free to use, modify or distribute this code or any
// derivative work that you create.  This code is provided WITHOUT
// WARRANTY and OpenNETCF does NOT claim in any way that this code 
// is fit for any specific or general use and holds NO RESPONSIBILITY
// for consequences of using any of it.  It is simply provided as-is.
//
// About:
// This is part of a free and open project to provide a simply way to
// port that uses time.h functions to Windows CE.  For the latest
// code, or to submit fixes, feature additions, etc. visit:
//
// http://www.opennetcf.com
//
// Version 0.01 - March 22, 2007
//
///////////////////////////////////////////////////////////////////////////////


#ifndef __TIME_CE_H__
#define __TIME_CE_H__

//
// time_CE.h
//
 
#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

/*
// these pragmas help Studio to build for some CE platforms - use as necessary
#pragma comment(linker, "/nodefaultlib:libc.lib")
#pragma comment(linker, "/nodefaultlib:libcd.lib")
#pragma comment(lib, "secchk.lib")
*/

// Number of clock ticks per second. Used by the clock() function.
//
#define CLOCKS_PER_SEC		1000

// Macro for CLK_PER_SEC
//
#define CLK_TICK			CLOCKS_PER_SEC

// structure definitions
//
#ifndef _TM_DEFINED
struct tm {
    int tm_sec;			// seconds
    int tm_min;			// minutes
    int tm_hour;		// hours
    int tm_mday;		// day of the month
    int tm_mon;			// month
    int tm_year;		// years since 1900 (from 0)
    int tm_wday;		// days since Sunday (from 0)
    int tm_yday;		// days since Jan 1
    int tm_isdst;		// daylight savings (summertime) flag
    };
#define _TM_DEFINED
#endif

// typedefs
//
typedef DWORD clock_t_ce;

// since time_t is already defined in CE as a ULONG, we need a new definition
//
typedef LONGLONG time_t_ce;

static struct tm tmbuf;

// Convert tm to a string in the format "Www Mmm dd hh:mm:ss yyyy", 
// where Www is the weekday, Mmm the month in letters, dd the day 
// of the month, hh:mm:ss the time, and yyyy the year. The string 
// is followed by a newline and a terminating null character, 
// conforming a total of 26 characters. 
//
char *asctime_ce(const struct tm* tmptr);

// Return number of clock ticks since process start.
//
clock_t_ce clock_ce(void);

// Convert time_t value to string in the same format as asctime.
//
char* ctime_ce(const time_t_ce* timer);

// Returns the difference in seconds between the two times.
//
double difftime_ce(time_t_ce timer2, time_t_ce timer1);

// Convert a time_t value to a tm structure as UTC time. 
//
struct tm* gmtime_ce(const time_t_ce* timer);

// Convert a time_t value to a tm structure as local time. 
//
struct tm* localtime_ce(const time_t_ce* timer);

// Returns the Unix timestamp corresponding to the arguments given. 
// This timestamp is a long integer containing the number of seconds between the Unix Epoch 
// (January 1 1970 00:00:00 GMT) and the time specified. 
//
time_t_ce mktime_ce(struct tm *tptr);

// Get the current time from the system clock. Stores that value in timer.
// If timer is null, the value is not stored, but it is still returned by
// the function.
//
time_t_ce time_ce(time_t_ce* timer);

// Format tm into a date/time string
//
size_t strftime_ce(char *s, size_t maxs, const char *f, const struct tm *t);

#ifdef __cplusplus
}
#endif

#endif