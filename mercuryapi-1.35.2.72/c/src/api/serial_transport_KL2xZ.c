/**
 *  @file serial_transport_KL2xZ.c
 *  @brief Mercury API - Serial transport functions for FRDM-KL25Z and FRDM-KL27Z boards
 *  @author Anjali Panchal
 *  @date 9/8/2020
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

#include "board.h"
#include "fsl_uart.h"
#include "pin_mux.h"

#define USE_KL27Z 1 //Disable this for KL25Z board

/* UART instance and clock */

#if USE_KL27Z
/* UART2: PTD2,PTD3 used which is J1-5(Tx), J1-7(Rx) on FRDM-KL27Z */
#define UART            UART2
#define UART_IRQn       UART2_FLEXIO_IRQn
#define UART_IRQHandler UART2_FLEXIO_IRQHandler
#else
/* UART1: PTE0,PTE1 used which is J2-20(TX), J2-18(RX) on FRDM-KL25Z */
#define UART            UART1
#define UART_IRQn       UART1_IRQn
#define UART_IRQHandler UART1_IRQHandler
#endif /* USE_KL27Z */

#define UART_BAUDRATE BOARD_DEBUG_UART_BAUDRATE
#define UART_CLKSRC     BUS_CLK
#define UART_CLK_FREQ   CLOCK_GetFreq(BUS_CLK)

/* MAX_RX_BUFFER_LENGTH Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) */
#define MAX_RX_BUFFER_LENGTH   256

/* Circular Buffer Structure */
typedef struct buf_st
{
  volatile uint32_t head;                       /* Next In Index  */
  uint32_t tail;                                /* Next Out Index */
  volatile char buffer [MAX_RX_BUFFER_LENGTH];  /* Buffer         */
}circBuf_t;

circBuf_t rbuf;
uart_config_t config;

/* Interrupt based UART handler which pulls data from the RX buffer */
void
UART_IRQHandler(void)
{
  uint8_t receiveByte = 0;
  uint32_t next = 0;

  /* If new data arrived. */
  if ((kUART_RxDataRegFullFlag | kUART_RxOverrunFlag) & UART_GetStatusFlags(UART))
  {
    receiveByte = UART_ReadByte(UART);

    next = rbuf.head + 1;
    next &= (MAX_RX_BUFFER_LENGTH - 1);

    /* If ring buffer is not full, add data to ring buffer. */
    if (next != rbuf.tail)
    {
      rbuf.buffer[rbuf.head] = receiveByte;
      rbuf.head = next;
    }
  }
}

void
configure_uart(void)
{
  /*
   * config.baudRate_Bps = 115200U;
   * config.parityMode = kUART_ParityDisabled;
   * config.stopBitCount = kUART_OneStopBit;
   * config.txFifoWatermark = 0;
   * config.rxFifoWatermark = 1;
   * config.enableTx = false;
   * config.enableRx = false;
   */
  UART_GetDefaultConfig(&config);
  config.baudRate_Bps = UART_BAUDRATE;
  config.enableTx = true;
  config.enableRx = true;

  /* Initialize the Rx circular buffer structure */
  rbuf.head = 0;
  rbuf.tail = 0;

  for (uint32_t i = 0; i < MAX_RX_BUFFER_LENGTH; i++)
  {
    rbuf.buffer[i] = 0;
  }

  UART_Init(UART, &config, UART_CLK_FREQ);
}

void
sendChar(uint8_t ch)
{
  bool txEmpty = true;

  while (txEmpty)
  {
    if (kUART_TxDataRegEmptyFlag & UART_GetStatusFlags(UART))
    {
      UART_WriteByte(UART, ch);
      txEmpty = false;
    }
  }
}

/****************************************************************/

static TMR_Status
s_open(TMR_SR_SerialTransport *this)
{
  BOARD_InitPins();
  BOARD_BootClockRUN();
  configure_uart();

  /* Enable RX interrupt. */
  UART_EnableInterrupts(UART,
       kUART_RxDataRegFullInterruptEnable | kUART_RxOverrunInterruptEnable);

  EnableIRQ(UART_IRQn);

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

  for (i = 0; i < length; i++)
  {
    sendChar(message[i]);
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

  while (index < length)
  {
    uint32_t next;

    /*If no data in circular buffer then wait for data*/
    while ((rbuf.head == rbuf.tail));

    if (rbuf.head != rbuf.tail)
    {
      message[index] = rbuf.buffer[rbuf.tail];
      next = rbuf.tail + 1;
      next &= (MAX_RX_BUFFER_LENGTH - 1);
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
  config.baudRate_Bps = rate;
  UART_Init(UART, &config, UART_CLK_FREQ);

  return TMR_SUCCESS;
}

static TMR_Status
s_shutdown(TMR_SR_SerialTransport *this)
{
  /* This routine should close the serial connection and release any
   * acquired resources.
   */
  UART_EnableTx(UART, false);
  UART_EnableRx(UART, false);

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
