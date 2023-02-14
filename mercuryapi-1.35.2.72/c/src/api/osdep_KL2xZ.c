/**
*  @file osdep_KL2xZ.c
*  @brief Mercury API - Baremetal platform implementation of OS dependencies
*  @author Anjali Panchal
*  @date 9/8/2020
*/

#include "osdep.h"
#include "clock_config.h"

uint32_t msec=0;
#if 0
void 
SysTick_Handler(void)
{
  msec++;
}
#endif

uint64_t 
tmr_gettime(void)
{
  uint32_t ret;
  ret=SysTick_Config(SystemCoreClock/1000); /* Configure SysTick to generate an interrupt every millisecond */

  if(ret==0)
  {
    return msec;
  }
  return 0;
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

void 
tmr_sleep(uint32_t sleepms)
{
  /* The function returns after at least 'sleepms' milliseconds
   * have elapsed.
   */
  while (sleepms--)
  {
    SysTick_Config(SystemCoreClock/1000);
  }
}

TMR_TimeStructure
tmr_gettimestructure()
{
  TMR_TimeStructure timestructure;
  timestructure.tm_year = (uint32_t)0;
  timestructure.tm_mon = (uint32_t)0;
  timestructure.tm_mday = (uint32_t)0;
  timestructure.tm_hour = (uint32_t)0;
  timestructure.tm_min = (uint32_t)0;
  timestructure.tm_sec = (uint32_t)0;

  return timestructure;
}
