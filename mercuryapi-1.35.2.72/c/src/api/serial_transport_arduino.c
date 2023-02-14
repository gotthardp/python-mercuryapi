/**
 *  @file serial_transport_arduino.c
 *  @brief Mercury API - Sample serial transport that does nothing
 *  @author Brian Fiegel
 *  @date 11/16/2010
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

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "tm_reader.h"

static TMR_Status
s_open(TMR_SR_SerialTransport *transport, const char *deviceName)
{
  /* This routine should open the serial connection */
  TMR_SR_SerialPortNativeContext *context;
  HardwareSerial *dev;

  dev = &Serial1;
  if (0==strcmp(deviceName,"/Serial"))
  {
    dev = &Serial;
  }
  else if (0==strcmp(deviceName,"/Serial1"))
  {
    dev = &Serial1;
  }
  else if (0==strcmp(deviceName,"/Serial2"))
  {
    dev = &Serial2;
  }
  else if (0==strcmp(deviceName,"/Serial3"))
  {
    dev = &Serial3;
  }

  context = (TMR_SR_SerialPortNativeContext*)transport->cookie;
  context->handle = dev;
  dev->begin(9600);
  return TMR_SUCCESS;
}

static TMR_Status
s_open(TMR_SR_SerialTransport *transport)
{
  /* This routine should open the serial connection */
  TMR_SR_SerialPortNativeContext *context;

  context = (TMR_SR_SerialPortNativeContext*)transport->cookie,
          s_open(transport,context->devicename);
  return TMR_SUCCESS;
}

static TMR_Status
s_sendBytes(TMR_SR_SerialTransport *transport, uint32_t length, 
                uint8_t* message, const uint32_t timeoutMs)
{
  /* This routine should send length bytes, pointed to by message on
   * the serial connection. If the transmission does not complete in
   * timeoutMs milliseconds, it should return TMR_ERROR_TIMEOUT.
   */
  TMR_SR_SerialPortNativeContext *context;
  HardwareSerial *dev;

  context = (TMR_SR_SerialPortNativeContext*)transport->cookie,
          dev = (HardwareSerial*)context->handle;
  dev->write(message, length);
  return TMR_SUCCESS;
}

static TMR_Status
s_receiveBytes(TMR_SR_SerialTransport *transport, uint32_t length, 
                   uint32_t* messageLength, uint8_t* message, const uint32_t timeoutMs)
{
  /* This routine should receive exactly length bytes on the serial
   * connection and store them into the memory pointed to by
   * message. If the required number of bytes are note received in
   * timeoutMs milliseconds, it should return TMR_ERROR_TIMEOUT.
   */
  unsigned long currentMillis,initialMillis;
  TMR_SR_SerialPortNativeContext *context;
  HardwareSerial *dev;

  context = (TMR_SR_SerialPortNativeContext*)transport->cookie,
          dev = (HardwareSerial*)context->handle;
  initialMillis = millis();
  currentMillis = initialMillis;
  *messageLength = 0;

  while((length > 0)
      && ((currentMillis - initialMillis) < timeoutMs)
      )
  {
    if (dev->available() > 0)
    {
      int inByte;
      // read the incoming byte:
      inByte = dev->read();
      if (inByte != -1)
      {
        message[*messageLength] = inByte;
        ++(*messageLength);
        --length;
      }
    }
    currentMillis  = millis();
  }

  if (0 != length)
  {
    return TMR_ERROR_TIMEOUT;
  }
  return TMR_SUCCESS;
}

static TMR_Status
s_setBaudRate(TMR_SR_SerialTransport *transport, uint32_t rate)
{
  /* This routine should change the baud rate of the serial connection
   * to the specified rate, or return TMR_ERROR_INVALID if the rate is
   * not supported.
   */
  TMR_SR_SerialPortNativeContext *context;
  HardwareSerial *dev;

  context = (TMR_SR_SerialPortNativeContext*)transport->cookie,
          dev = (HardwareSerial*)context->handle;
  dev->end();
  dev->begin(rate);
  return TMR_SUCCESS;
}

static TMR_Status
s_shutdown(TMR_SR_SerialTransport *transport)
{
  /* This routine should close the serial connection and release any
   * acquired resources.
   */
  TMR_SR_SerialPortNativeContext *context;
  HardwareSerial *dev;

  context = (TMR_SR_SerialPortNativeContext*)transport->cookie,
          dev = (HardwareSerial*)context->handle;
  dev->end();
  return TMR_SUCCESS;
}

static TMR_Status
s_flush(TMR_SR_SerialTransport *transport)
{
  /* This routine should empty any input or output buffers in the
   * communication channel. If there are no such buffers, it may do
   * nothing.
   */
  TMR_SR_SerialPortNativeContext *context;
  HardwareSerial *dev;

  context = (TMR_SR_SerialPortNativeContext*)transport->cookie,
          dev = (HardwareSerial*)context->handle;

  dev->flush();
  return TMR_SUCCESS;
}

/* This function is not part of the API as such. This is for
 * application code to call to fill in the transport object before
 * initializing the reader object itself, as in the following code:
 * 
 * TMR_Reader reader;
 *
 * TMR_SerialTransportDummyInit(&reader.u.serialReader.transport, myArgs);
 * TMR_SR_SerialReader_init(&reader);
 *
 * The initialization should not actually open a communication channel
 * or acquire other communication resources at this time.
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
  return s_open(transport,device);
#endif
}
