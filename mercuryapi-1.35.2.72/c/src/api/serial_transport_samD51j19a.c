/**
 *  @file serial_transport_samD51j19a.c
 *  @brief Mercury API - Serial transport functions for SAMD51J19A board
 *  @author Anjali Panchal
 *  @date 11/09/2021
 */


/*
 * Copyright (c) 2020 ThingMagic, Inc.
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
#include "driver_init.h"
#include "inventory.h"
#include <string.h>
#include <stdio.h>
#include <utils.h>
#include <hal_init.h>
#include <atmel_start.h>
#include <peripheral_clk_config.h>

#define DEBUG_LOGS 0

/*! The buffer size for USART */
#define MODULE_COM_BUFFER_SIZE 16
/*** MAX_RX_BUFFER_LENGTH Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/
#define MAX_RX_BUFFER_LENGTH   256

struct io_descriptor *moduleIo;
struct usart_async_descriptor MODULE_COM;
static uint8_t MODULE_COM_buffer[MODULE_COM_BUFFER_SIZE];

volatile static uint8_t dataSent  = 0;
volatile static uint8_t dataError = 0;

/* Circular Buffer Structure */
typedef struct moduleCircBuf_t
{
  volatile uint32_t head;                      /* Next In Index  */
  uint32_t tail;                               /* Next Out Index */
  volatile char buffer [MAX_RX_BUFFER_LENGTH]; /* Buffer         */
}moduleCircBuf_t;

moduleCircBuf_t moduleRxbuf;

uint32_t module_buffer_num_items()
{
  return moduleRxbuf.head - moduleRxbuf.tail;
}

static void rx_cb_MODULE_COM(const struct usart_async_descriptor *const io_descr)
{
  /* Receive completed */
  
  uint32_t next;
  uint8_t receiveByte;

  io_read( moduleIo, &receiveByte, 0x01 );
  next = moduleRxbuf.head + 1;
  next &= ( MAX_RX_BUFFER_LENGTH - 1 );

  if (next != moduleRxbuf.tail)
  {
    moduleRxbuf.buffer[moduleRxbuf.head] = receiveByte;
    moduleRxbuf.head = next;
  }
}

static void tx_cb_MODULE_COM(const struct usart_async_descriptor *const io_descr)
{
  /* Send completed */
  dataSent = 1;
}

static void err_cb_MODULE_COM(const struct usart_async_descriptor *const io_descr)
{
  /* error handle */
  dataError = 1;
}

/**
 * USART Clock initialization function
 * Enables register interface and peripheral clock
 */
void MODULE_COM_CLOCK_init()
{
  hri_gclk_write_PCHCTRL_reg(GCLK, SERCOM2_GCLK_ID_CORE, CONF_GCLK_SERCOM2_CORE_SRC | (1 << GCLK_PCHCTRL_CHEN_Pos));
  hri_gclk_write_PCHCTRL_reg(GCLK, SERCOM2_GCLK_ID_SLOW, CONF_GCLK_SERCOM2_SLOW_SRC | (1 << GCLK_PCHCTRL_CHEN_Pos));

  hri_mclk_set_APBBMASK_SERCOM2_bit(MCLK);
}

/**
 * USART pinmux initialization function
 * Set each required pin to USART functionality
 */
void MODULE_COM_PORT_init()
{
  gpio_set_pin_function(PA12, PINMUX_PA12C_SERCOM2_PAD0);
  gpio_set_pin_function(PA13, PINMUX_PA13C_SERCOM2_PAD1);
}

/**
 * brief USART initialization function
 * Enables USART peripheral, clocks and initializes USART driver
 */
void MODULE_COM_init(void)
{
  MODULE_COM_CLOCK_init();
  usart_async_init(&MODULE_COM, SERCOM2, MODULE_COM_buffer, MODULE_COM_BUFFER_SIZE, (void *)NULL);
  MODULE_COM_PORT_init();
}

TMR_Status
s_open(TMR_SR_SerialTransport *this)
{
  // Uncomment below line
  //init_mcu();
  MODULE_COM_init();

  SystemCoreClockUpdate();
  SysTick_Config(SystemCoreClock / 1000);

  usart_async_register_callback(&MODULE_COM, USART_ASYNC_TXC_CB, tx_cb_MODULE_COM);
  usart_async_register_callback(&MODULE_COM, USART_ASYNC_RXC_CB, rx_cb_MODULE_COM);
  usart_async_register_callback(&MODULE_COM, USART_ASYNC_ERROR_CB, err_cb_MODULE_COM);
  usart_async_get_io_descriptor(&MODULE_COM, &moduleIo);
  usart_async_enable(&MODULE_COM);

  return TMR_SUCCESS;
}

TMR_Status
s_sendBytes(TMR_SR_SerialTransport *this, uint32_t length, 
                uint8_t* message, const uint32_t timeoutMs)
{
  /* This routine should send length bytes, pointed to by message on
   * the serial connection. If the transmission does not complete in
   * timeoutMs milliseconds, it should return TMR_ERROR_TIMEOUT.
   */
  uint64_t preTime;
  preTime = tmr_gettime();

  io_write(moduleIo, message, length);

  // Wait until all bytes sent
  while(dataSent == 0)
  {
    if((tmr_gettime() - preTime) > timeoutMs)
    {
      return TMR_ERROR_TIMEOUT;
    }
  }

  dataSent = 0;
  
#if DEBUG_LOGS
  {
    char idStr[256];
    char header[256] = "Send : ";
    TMR_bytesToHex(message, length, idStr);

    strcat(header, idStr);
    sendHostBytes((uint8_t *)header, strlen(header));
  }
#endif /* DEBUG_LOGS */

  return TMR_SUCCESS;
}

TMR_Status
s_receiveBytes(TMR_SR_SerialTransport *this, uint32_t length, 
                   uint32_t* messageLength, uint8_t* message, const uint32_t timeoutMs)
{
  /* This routine should receive exactly length bytes on the serial
   * connection and store them into the memory pointed to by
   * message. If the required number of bytes are note received in
   * timeoutMs milliseconds, it should return TMR_ERROR_TIMEOUT.
   */
  uint32_t index = 0;
  uint64_t preTime = tmr_gettime();
  *messageLength = 0;

  while (index < length )
  {
    uint32_t next;

    if((tmr_gettime() - preTime) > timeoutMs)
    {
      return TMR_ERROR_TIMEOUT;
    }

    if (moduleRxbuf.head != moduleRxbuf.tail)
    {
      message[index] = moduleRxbuf.buffer[moduleRxbuf.tail];
      next = moduleRxbuf.tail + 1;
      next &= (MAX_RX_BUFFER_LENGTH-1);
      moduleRxbuf.tail = next;
      index++;
    }
  }

  *messageLength = index;
 
#if DEBUG_LOGS
  {
    char idStr[1024];
    char header[1024] = "Receive : ";
    TMR_bytesToHex(message, length, idStr);

    strcat(header, idStr);
    sendHostBytes((uint8_t *)header, strlen(header));
  }
#endif /* DEBUG_LOGS */

  return TMR_SUCCESS;
}

static TMR_Status
s_setBaudRate(TMR_SR_SerialTransport *this, uint32_t rate)
{
  /* This routine should change the baud rate of the serial connection
   * to the specified rate, or return TMR_ERROR_INVALID if the rate is
   * not supported.
   */
  return TMR_SUCCESS;
}

static TMR_Status
s_shutdown(TMR_SR_SerialTransport *this)
{
  /* This routine should close the serial connection and release any
   * acquired resources.
   */
  ASSERT(&MODULE_COM);
  _usart_async_deinit(&MODULE_COM.device);
  MODULE_COM.io.read  = NULL;
  MODULE_COM.io.write = NULL;

  return TMR_SUCCESS;
}

static TMR_Status
s_flush(TMR_SR_SerialTransport *this)
{
  /* This routine should empty any input or output buffers in the
   * communication channel. If there are no such buffers, it may do
   * nothing.
   */

  usart_async_flush_rx_buffer(&MODULE_COM);
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

  return TMR_SUCCESS;
}
