/**
 *  @file serial_transport_dummy.c
 *  @brief Mercury API - Sample serial transport that does nothing
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

#include "tm_reader.h"
#include <stdio.h>
#include "MercuryApi.h"

/* Stub implementation of serial transport layer routines. */

static TMR_Status
s_open(TMR_SR_SerialTransport *this)
{

    printf("s_open");
  /* This routine should open the serial connection */
    return RDRscMgrOpen(this);
}


static TMR_Status
s_sendBytes(TMR_SR_SerialTransport *this, uint32_t length, 
                uint8_t* message, const uint32_t timeoutMs)
{

  /* This routine should send length bytes, pointed to by message on
   * the serial connection. If the transmission does not complete in
   * timeoutMs milliseconds, it should return TMR_ERROR_TIMEOUT.
   */

  return RDRscMgrSendBytes(this, length, message, timeoutMs);
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

  return RDRscMgrReceiveBytes(this, length, messageLength, message, timeoutMs);
}


static TMR_Status
s_setBaudRate(TMR_SR_SerialTransport *this, uint32_t rate)
{

  /* This routine should change the baud rate of the serial connection
   * to the specified rate, or return TMR_ERROR_INVALID if the rate is
   * not supported.
   */

  return setRDRscMgrBaudRate(this, rate);
}


static TMR_Status
s_shutdown(TMR_SR_SerialTransport *this)
{

  /* This routine should close the serial connection and release any
   * acquired resources.
   */

  return RDRscMgrshutdown(this);
}

static TMR_Status
s_flush(TMR_SR_SerialTransport *this)
{

  /* This routine should empty any input or output buffers in the
   * communication channel. If there are no such buffers, it may do
   * nothing.
   */

  return RDRscMgrFlush(this);
}



/* This function is not part of the API as such. This is for
 * application code to call to fill in the transport object before
 * initializing the reader object itself, as in the following code:
 * 
 * TMR_Reader reader;
 *
 * TMR_SR_SerialTransportDummyInit(&reader.u.serialReader.transport, myArgs);
 * TMR_SR_SerialReader_init(&reader);
 *
 * The initialization should not actually open a communication channel
 * or acquire other communication resources at this time.
 */
TMR_Status
TMR_SR_SerialTransportNativeInit(TMR_SR_SerialTransport *transport,
								TMR_SR_SerialPortNativeContext *context, const char *device)
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

/**
 * Initialize a TMR_SR_SerialTransport structure with a given serial device.
 *
 * @param transport The TMR_SR_SerialTransport structure to initialize.
 * @param context A TMR_SR_SerialPortNativeContext structure for the callbacks to use.
 * @param device The path or name of the serial device (@c /dev/ttyS0, @c COM1)
 */
//TMR_Status
//TMR_SR_SerialTransportNativeInit(TMR_SR_SerialTransport *transport,
//                                 TMR_SR_SerialPortNativeContext *context,
//                                 const char *device)
//{
//  return TMR_SUCCESS;
//}

