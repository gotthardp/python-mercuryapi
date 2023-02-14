/**
*  @file osdep_samD51j19a.c
*  @brief Mercury API - Baremetal platform implementation of OS dependencies
*  @author Anjali Panchal
*  @date 11/09/2021
*/

#include "osdep.h"
#include "samd51.h"

volatile static uint32_t msec = 0;

//ISR implementation
void SysTick_Handler(void)
{
  msec++;
}

uint64_t 
tmr_gettime(void)
{
  return msec;
}

uint32_t 
tmr_gettime_low()
{
  /* Returns the low 32 bits of a millisecond counter.
   * The API will not otherwise interpret the counter value.
   */
  return (tmr_gettime() >>  0) & 0xffffffff;
}

uint32_t 
tmr_gettime_high()
{
  /* Returns the high 32 bits of a millisecond counter.
   * The API will not otherwise interpret the counter value.
   */
  return (tmr_gettime() >> 32) & 0xffffffff;
}

void tmr_sleep(uint32_t sleepMs)
{
  /* The function returns after 'sleepMs' milliseconds
   * have elapsed.
   */
  uint32_t startTime = tmr_gettime();
  while((tmr_gettime() - startTime) < sleepMs)
  {
    //Do Nothing
  }
}

TMR_TimeStructure
tmr_gettimestructure()
{
  TMR_TimeStructure timestructure;
  timestructure.tm_year = (uint32_t)0;
  timestructure.tm_mon  = (uint32_t)0;
  timestructure.tm_mday = (uint32_t)0;
  timestructure.tm_hour = (uint32_t)0;
  timestructure.tm_min  = (uint32_t)0;
  timestructure.tm_sec  = (uint32_t)0;

  return timestructure;
}
