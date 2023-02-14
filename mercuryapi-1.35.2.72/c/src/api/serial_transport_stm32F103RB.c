/**
 *  @file serial_transport_stm32F103RB.c
 *  @brief Mercury API - Serial transport functions for STM32F103RB board
 *  @author Pallav Joshi
 *  @date 5/16/2016
 */


/*
 * Copyright (c) 2016 ThingMagic, Inc.
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
#include "STM32F10x.h"

/*** MAX_TX_BUFFER_LENGTH Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/
#define MAX_TX_BUFFER_LENGTH   256
#define TBUFLEN ((uint32_t)((tbuf.head < tbuf.tail) ? ((MAX_TX_BUFFER_LENGTH- tbuf.tail)+tbuf.head):(tbuf.head - tbuf.tail)))

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

circBuf_t rbuf,tbuf;

/************************Circular Buffer length validation*********************************************/

#if MAX_TX_BUFFER_LENGTH < 2
#error MAX_TX_BUFFER_LENGTH is too small. It must be larger than 1.
#elif ((MAX_TX_BUFFER_LENGTH & (MAX_TX_BUFFER_LENGTH-1)) != 0)
#error MAX_TX_BUFFER_LENGTH must be a power of 2.
#endif

#if MAX_RX_BUFFER_LENGTH < 2
#error MAX_RX_BUFFER_LENGTH is too small. It must be larger than 1.
#elif ((MAX_RX_BUFFER_LENGTH & (MAX_RX_BUFFER_LENGTH-1)) != 0)
#error MAX_RX_BUFFER_LENGTH must be a power of 2.
#endif
/*********************************************************************/

/* NZ if TX restart is required */
static unsigned int tx_restart = 1;

/********** USART1_IRQHandler Handles USART1 global interrupt request. ***********/

void USART1_IRQHandler (void)
{
  volatile unsigned int IIR;
  uint32_t next;
  uint8_t receiveByte;
  uint32_t sendByte;

  IIR = USART1->SR;
  if (IIR & USART_SR_RXNE)                      /* read interrupt */
  {
    USART1->SR &= ~USART_SR_RXNE;               /* clear interrupt */

    receiveByte = (USART1->DR & 0x1FF);
    next= rbuf.head + 1;
    next &= (MAX_RX_BUFFER_LENGTH-1);

    if (next != rbuf.tail)
    {
      rbuf.buffer[rbuf.head] = receiveByte;
      rbuf.head = next;
    }
  }

  if (IIR & USART_SR_TXE)                       /* read interrupt  */
  {
    USART1->SR &= ~USART_SR_TXE;                /* clear interrupt */
    if (tbuf.head != tbuf.tail)
    {
      sendByte =(tbuf.buffer[tbuf.tail] & 0x1FF);
      USART1->DR  =sendByte ;
      next = tbuf.tail + 1;
      next &= (MAX_TX_BUFFER_LENGTH-1);
      tbuf.tail = next;
    }
    else 
    {
      tx_restart = 1;
      USART1->CR1 &= ~USART_SR_TXE;            /* disable TX IRQ if nothing to send */
    }
  }
}

/****************** SendChar:Sends a character ****************/
int SendChar (int c) 
{
  if (TBUFLEN >= MAX_TX_BUFFER_LENGTH)          /* If the buffer is full            */
  {
    return (-1);                                  /* return an error value            */
  }

  tbuf.buffer[tbuf.head] = c;
  tbuf.head += 1;

  if(tbuf.head == MAX_TX_BUFFER_LENGTH)
  {
    tbuf.head = 0;
  }

  if(tx_restart)                                /* If TX interrupt is disabled   */
  {                          
    tx_restart = 0;                             /* enable it                     */
    USART1->CR1 |= USART_SR_TXE;                /* enable TX interrupt           */
  }

  return (0);
}


/****************** Stub implementation of serial transport layer routines. ****************/
static TMR_Status
s_open(TMR_SR_SerialTransport *this)
{
  int i;

  /* Initialize the Tx circular buffer structure */
  tbuf.head = 0;
  tbuf.tail = 0;
  for(i = 0; i< MAX_RX_BUFFER_LENGTH; i++)
  {
    tbuf.buffer[i] = 0;
  }

  /* Initialize the Rx circular buffer structure */
  rbuf.head = 0;
  rbuf.tail = 0;
  for(i = 0; i< MAX_RX_BUFFER_LENGTH; i++)
  {
    rbuf.buffer[i] = 0;
  }

  RCC->APB2ENR |=  (   1UL <<  0);              /* enable clock Alternate Function  */
  AFIO->MAPR   &= ~(   1UL <<  2);              /* clear USART1 remap               */

  RCC->APB2ENR |=  (   1UL <<  2);              /* enable GPIOA clock               */
  GPIOA->CRH   &= ~(0xFFUL <<  4);              /* clear PA9, PA10                  */
  GPIOA->CRH   |=  (0x0BUL <<  4);              /* USART1 Tx (PA9) output push-pull */
  GPIOA->CRH   |=  (0x04UL <<  8);              /* USART1 Rx (PA10) input floating  */

  RCC->APB2ENR |=  (   1UL << 14);              /* enable USART#1 clock             */

  USART1->BRR   = 0x0271;                       /* 115200 baud @ PCLK2 72MHz        */
  USART1->CR1   = ((   1UL <<  2) |             /* enable RX                        */
                   (   1UL <<  3) |             /* enable TX                        */
                   (   1UL <<  5) |             /* enable RXNE Interrupt            */
                   (   1UL <<  7) |             /* enable TXE Interrupt             */
                   (   0UL << 12) );            /* 1 start bit, 8 data bits         */
  USART1->CR2   = 0x0000;                       /* 1 stop bit                       */
  USART1->CR3   = 0x0000;                       /* no flow control                  */
  for (i = 0; i < 0x1000; i++) __NOP();         /* avoid unwanted output            */

  NVIC_EnableIRQ(USART1_IRQn);
  USART1->CR1  |= ((   1UL << 13) );            /* enable USART                     */

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

  tbuf.head = 0;                                /* clear com buffer indexes */
  tbuf.tail = 0;
  for (i = 0; i<length; i++)
  {
    SendChar(message[i]);
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
  *messageLength = 0;

  while (index < length )
  {
    uint32_t next;

    while (rbuf.head == rbuf.tail);             /*If no data in circular buffer then wait for data*/
    if (rbuf.head != rbuf.tail)
    {
      message[index] = rbuf.buffer[rbuf.tail];
      next = rbuf.tail + 1;
      next &= (MAX_RX_BUFFER_LENGTH-1);
      rbuf.tail = next;
    }
    index++;
  }
  *messageLength = index;

  return TMR_SUCCESS;
}

static TMR_Status
s_setBaudRate(TMR_SR_SerialTransport *this, uint32_t rate)
{
  /* This routine should change the baud rate of the serial connection
  * to the specified rate, or return TMR_ERROR_INVALID if the rate is
  * not supported.
  */
  USART1->BRR   = 72000000 / (long) rate;       /* 115200 baud @ PCLK2 72MHz */
  return TMR_SUCCESS;
}

static TMR_Status
s_shutdown(TMR_SR_SerialTransport *this)
{
  /* This routine should close the serial connection and release any
  * acquired resources.
  */
  USART1->CR1 &= ~USART_SR_TXE;     /* disable TX IRQ if nothing to send  */
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
