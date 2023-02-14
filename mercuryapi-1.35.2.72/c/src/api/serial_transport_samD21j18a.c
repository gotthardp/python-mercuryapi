/**
 *  @file serial_transport_samD21j18a.c
 *  @brief Mercury API - Sample serial transport that does nothing
 *  @author Pallav Joshi
 *  @date 6/14/2016
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

#include "tm_reader.h"
#include <asf.h>
#include <stdio.h>

/*** MAX_RX_BUFFER_LENGTH Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/
#define MAX_RX_BUFFER_LENGTH   256
#define RBUFLEN ((uint32_t)((rbuf.head < rbuf.tail) ? ((MAX_RX_BUFFER_LENGTH- rbuf.tail)+rbuf.head):(rbuf.head - rbuf.tail)))

/* Circular Buffer Structure */
typedef struct buf_st
{
  volatile uint32_t head;                      /* Next In Index  */
  uint32_t tail;                               /* Next Out Index */
  volatile char buffer [MAX_RX_BUFFER_LENGTH]; /* Buffer         */
}circBuf_t;

struct usart_config config_usart;
static uint8_t s_open_flag = 1;
struct usart_module usart_instance;
circBuf_t rbuf;

void sercom4Handler(void);
void configure_usart(void);

/**
 * Handles receive interrupt as it occur.
 * Note:To use this interrupt handler you have to register it in 
 * usart_init() function as shown below.
 * _sercom_set_handler(instance_index, sercom4Handler);
 */
void sercom4Handler(void)
{
  /* Temporary variables */
  uint16_t interrupt_status;
  uint8_t receiveByte;
  uint8_t error_code;

  /* Get device instance from the look-up table */
  struct usart_module *module = &usart_instance;

  /* Pointer to the hardware module instance */
  SercomUsart *const usart_hw = &(usart_instance.hw->USART);

  /* Wait for the synchronization to complete */
  _usart_wait_for_sync(module);

  /* Read and mask interrupt flag register */
  interrupt_status = usart_hw->INTFLAG.reg;
  interrupt_status &= usart_hw->INTENSET.reg;

  /*Disable DATA READY interrupt if it is enabled */
  usart_hw->INTENCLR.reg = SERCOM_USART_INTFLAG_DRE;

  if(interrupt_status & SERCOM_USART_INTFLAG_RXC)
  {
    /* Read out the status code and mask away all but the 3 LSBs*/
    error_code = (uint8_t)(usart_hw->STATUS.reg & SERCOM_USART_STATUS_MASK);

    /* Check if an error has occurred during the receiving */
    if (error_code)
    {
      /* Check which error occurred */
      if (error_code & SERCOM_USART_STATUS_FERR) 
      {
        /* Clear flag by writing a 1 to it */
        usart_hw->STATUS.reg = SERCOM_USART_STATUS_FERR;
      } 
      if (error_code & SERCOM_USART_STATUS_BUFOVF)
      {
        /* Clear flag by writing a 1 to it */
        usart_hw->STATUS.reg = SERCOM_USART_STATUS_BUFOVF;
      }
      if (error_code & SERCOM_USART_STATUS_PERR)
      {
        /* Clear flag by writing a 1 to it */
        usart_hw->STATUS.reg = SERCOM_USART_STATUS_PERR;
      }
#ifdef FEATURE_USART_LIN_SLAVE
      if (error_code & SERCOM_USART_STATUS_ISF)
      {
        /* Clear flag by writing 1 to it */
        usart_hw->STATUS.reg |= SERCOM_USART_STATUS_ISF;
      }
#endif
#ifdef FEATURE_USART_COLLISION_DECTION
      if (error_code & SERCOM_USART_STATUS_COLL)
      {
        /* Clear flag by writing 1 to it */
        usart_hw->STATUS.reg |= SERCOM_USART_STATUS_COLL;
      }
#endif
    }
    else
    {
      uint32_t next;
      receiveByte = usart_hw->DATA.reg;
      next= rbuf.head + 1;
      next &= (MAX_RX_BUFFER_LENGTH-1);

      if (next != rbuf.tail)
      {
        rbuf.buffer[rbuf.head] = receiveByte;
        rbuf.head = next;
      }
    }
  }
  else
  {
    /* This should not happen. Disable Receive Complete interrupt. */
    usart_hw->INTENCLR.reg = SERCOM_USART_INTFLAG_RXC;
  }
}

void
configure_usart()
{
  usart_get_config_defaults(&config_usart);
  config_usart.baudrate    = 115200;
  config_usart.mux_setting = EXT1_UART_SERCOM_MUX_SETTING;
  config_usart.pinmux_pad0 = EXT1_UART_SERCOM_PINMUX_PAD0;
  config_usart.pinmux_pad1 = EXT1_UART_SERCOM_PINMUX_PAD1;
  config_usart.pinmux_pad2 = EXT1_UART_SERCOM_PINMUX_PAD2;
  config_usart.pinmux_pad3 = EXT1_UART_SERCOM_PINMUX_PAD3;

  /* Initialize the Rx circular buffer structure */
  rbuf.head = 0;
  rbuf.tail = 0;
  for(uint32_t i = 0; i< MAX_RX_BUFFER_LENGTH; i++)
  {
    rbuf.buffer[i] = 0;
  }
  while (usart_init(&usart_instance, EXT1_UART_MODULE, &config_usart) != STATUS_OK);
  usart_enable(&usart_instance);
}

/****************************************************************/

/* Stub implementation of serial transport layer routines. */
static TMR_Status
s_open(TMR_SR_SerialTransport *this)
{
  /* This routine should open the serial connection */
  if(s_open_flag)
  {
    system_init();
    configure_usart();

    /* Enable the RX Complete Interrupt */
    usart_instance.hw->USART.INTENSET.reg = SERCOM_USART_INTFLAG_RXC;
    system_interrupt_enable_global();
  }

  s_open_flag=0;
  return TMR_SUCCESS;
}

static TMR_Status
s_sendBytes(TMR_SR_SerialTransport *this, uint32_t length,
uint8_t* message, const uint32_t timeoutMs)
{
  /* This routine should send length bytes, pointed to by message on
   * the serial connection. If the transmission does not complete in
   * timeoutMs milliseconds, it should return TMR_ERROR_TIMEOUT.
   */
  uint32_t i = 0;
  for (i = 0; i<length; i++)
  {
    usart_serial_putchar(&usart_instance, message[i]);
  }
  return TMR_SUCCESS;
}

static TMR_Status
s_receiveBytes(TMR_SR_SerialTransport *this, uint32_t length,
uint32_t* messageLength, uint8_t* message, const uint32_t timeoutMs)
{
  /* This routine should receive exactly length bytes on the serial
   * connection and store them into the memory pointed to by
   * message. If the required number of bytes are note received in
   * timeoutMs milliseconds, it should return TMR_ERROR_TIMEOUT.
   */
  uint32_t index = 0;

  while (index < length )
  {
    uint32_t next;
    /*If no data in circular buffer then wait for data*/
    while ((rbuf.head == rbuf.tail));
    if (rbuf.head != rbuf.tail)
    {
      message[index] = rbuf.buffer[rbuf.tail];
      next = rbuf.tail + 1;
      next &= (MAX_RX_BUFFER_LENGTH-1);
      rbuf.tail = next;
    }
    index++;
  }
  return TMR_SUCCESS;
}

static TMR_Status
s_setBaudRate(TMR_SR_SerialTransport *this, uint32_t rate)
{
  /* This routine should change the baud rate of the serial connection
   * to the specified rate, or return TMR_ERROR_INVALID if the rate is
   * not supported.
   */
  usart_disable(&usart_instance);
  usart_get_config_defaults(&config_usart);
  config_usart.baudrate    = rate;
  config_usart.mux_setting = EXT1_UART_SERCOM_MUX_SETTING;
  config_usart.pinmux_pad0 = EXT1_UART_SERCOM_PINMUX_PAD0;
  config_usart.pinmux_pad1 = EXT1_UART_SERCOM_PINMUX_PAD1;
  config_usart.pinmux_pad2 = EXT1_UART_SERCOM_PINMUX_PAD2;
  config_usart.pinmux_pad3 = EXT1_UART_SERCOM_PINMUX_PAD3;
  
  while (usart_init(&usart_instance, EXT1_UART_MODULE, &config_usart) != STATUS_OK);
  usart_enable(&usart_instance);

  return TMR_SUCCESS;
}

static TMR_Status
s_shutdown(TMR_SR_SerialTransport *this)
{
  /* This routine should close the serial connection and release any
   * acquired resources.
   */
  usart_disable(&usart_instance);
  return TMR_SUCCESS;
}

static TMR_Status
s_flush(TMR_SR_SerialTransport *this)
{
  /* This routine should empty any input or output buffers in the
   * communication channel. If there are no such buffers, it may do
   * nothing.
   */
  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_SR_SerialTransport structure with a given serial device.
 *
 * @param transport The TMR_SR_SerialTransport structure to initialize.
 * @param context A TMR_SR_SerialPortNativeContext structure for the callbacks to use.
 * @param device The path or name of the serial device (@c /dev/ttyS0, @c COM1)
 */
TMR_Status
TMR_SR_SerialTransportNativeInit(TMR_SR_SerialTransport *transport,
TMR_SR_SerialPortNativeContext *context,
const char *device)
{
  /* Each of the callback functions will be passed the transport
   * pointer, and they can use the "cookie" member of the transport
   * structure to store the information specific to the transport,
   * such as a file handle or the memory address of the FIFO.
   */
  transport->cookie = context;
  transport->open = s_open;
  transport->sendBytes = s_sendBytes;
  transport->receiveBytes = s_receiveBytes;
  transport->setBaudRate = s_setBaudRate;
  transport->shutdown = s_shutdown;
  transport->flush = s_flush;

#if TMR_MAX_SERIAL_DEVICE_NAME_LENGTH > 0
  if (strlen(device) + 1 > TMR_MAX_SERIAL_DEVICE_NAME_LENGTH)
  {
    return TMR_ERROR_INVALID;
  }
  strcpy(context->devicename, device);
  return TMR_SUCCESS;
#else
  /* No space to store the device name, so open it now */
  return s_open(transport);
#endif
}