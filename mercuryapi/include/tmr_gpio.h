/* ex: set tabstop=2 shiftwidth=2 expandtab cindent: */
#ifndef _TMR_GPIO_H
#define _TMR_GPIO_H
/**
 *  @file tmr_gpio.h
 *  @brief Mercury API - GPIO Definitons
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

/**
 * The identity and state of a single GPIO pin.
 */
typedef struct TMR_GpioPin
{
  /** The ID number of the pin. */
  uint8_t id;
  /** Whether the pin is in the high state. */
  bool high;
  /* The GPI pin status from higher nibble(4-7) */
  bool bGPIStsTagRdMeta;
  /** The direction of the pin */
  bool output;
}TMR_GpioPin;


#ifdef  __cplusplus
}
#endif

#endif /* _TMR_GPIO_H_ */
