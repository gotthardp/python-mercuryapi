/**
 *  @file serial_transport_win32.c
 *  @brief Mercury API - Serial transport over local serial port on Win32
 *  @author Nathan Williams
 *  @date 10/20/2009
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

#include "tm_config.h"
#include "tmr_status.h"

#if defined(WIN32) || defined(WINCE)
#if !defined(_WINSOCK2API_) && !defined(_WINSOCKAPI_)
#include <winsock2.h>
#endif
#include <stdio.h>

#ifndef __GNUC__
#  if defined(WINCE)
#    define snprintf _snprintf
#  else
#    define snprintf sprintf_s
#  endif
#endif /* __GNUC__ */
#endif /* WIN32 */

#include "tm_reader.h"

__declspec(dllexport) TMR_Status
s_open(TMR_SR_SerialTransport *this)
{
  TMR_SR_SerialPortNativeContext *c;
  COMMTIMEOUTS timeOuts;
  DCB dcb;

  c = this->cookie;
  
  c->handle = CreateFile((TCHAR*)c->devicename,
                         GENERIC_READ | GENERIC_WRITE,
                         0,             /* not shared */
                         NULL,          /* default security */
                         OPEN_EXISTING, /* don't create a new file */
                         0,             /* flags and attributes */
                         NULL);         /* template file */
                         
  if (INVALID_HANDLE_VALUE == c->handle)
    return TMR_ERROR_COMM_ERRNO(GetLastError());

  timeOuts.ReadIntervalTimeout          = 0xFFFFFFFF;
  timeOuts.ReadTotalTimeoutConstant     = 0;
  timeOuts.ReadTotalTimeoutMultiplier   = 0;
  timeOuts.WriteTotalTimeoutConstant    = 5000;
  timeOuts.WriteTotalTimeoutMultiplier  = 0;
  
  SetCommTimeouts(c->handle, &timeOuts);

  dcb.DCBlength = sizeof(DCB);
  GetCommState(c->handle, &dcb);
  dcb.fOutxCtsFlow = 0;
  dcb.fOutxDsrFlow = 0;
  dcb.fDtrControl = DTR_CONTROL_DISABLE;
  dcb.fDsrSensitivity = 0;
  dcb.fOutX = 0;
  dcb.fInX = 0;
  dcb.fNull = 0;
  dcb.fRtsControl = RTS_CONTROL_DISABLE;
  dcb.ByteSize  = 8;
  dcb.fParity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;
  if ((0 == SetCommState(c->handle, &dcb)) ||
      (0 == SetupComm(c->handle, 10000, 10000)))
  {
    return TMR_ERROR_COMM_ERRNO(GetLastError());
  }
  
  return TMR_SUCCESS;
}

__declspec(dllexport) TMR_Status
s_sendBytes(TMR_SR_SerialTransport *this, uint32_t length,
            uint8_t* message, const uint32_t timeoutMs)
{
  TMR_SR_SerialPortNativeContext *c;
  BOOL writeStatus;
  COMMTIMEOUTS timeOuts;

  c = this->cookie;
  GetCommTimeouts(c->handle, &timeOuts);
  timeOuts.WriteTotalTimeoutConstant = timeoutMs;
  SetCommTimeouts(c->handle, &timeOuts);
  
  {
	long numberOfBytesWritten;
	writeStatus = WriteFile(c->handle, message, length, &numberOfBytesWritten, NULL);
  }
  if (0 == writeStatus)
  {
    return TMR_ERROR_COMM_ERRNO(GetLastError());
  }
  return TMR_SUCCESS;
}

__declspec(dllexport) TMR_Status
s_receiveBytes(TMR_SR_SerialTransport *this, uint32_t length,
              uint32_t* messageLength, uint8_t* message, const uint32_t
timeoutMs)
{
 DWORD readLength;
 DWORD errorFlags;
 COMSTAT comStat;
 TMR_SR_SerialPortNativeContext *c;
 BOOL    readStatus;
 COMMTIMEOUTS timeOuts;

 c = this->cookie;
 *messageLength=0;

 GetCommTimeouts(c->handle, &timeOuts);
 timeOuts.ReadTotalTimeoutConstant = timeoutMs;
 SetCommTimeouts(c->handle, &timeOuts);

 ClearCommError(c->handle, &errorFlags, &comStat);
 while (length > 0)
 {
   readLength=0;
   readStatus = ReadFile(c->handle, message, length, &readLength, NULL);
   *messageLength += readLength;
   if (0 == readStatus)
   {
     return TMR_ERROR_COMM_ERRNO(GetLastError());
   }
   if(readLength == 0)
   {
     return TMR_ERROR_TIMEOUT;
   }
   length -= readLength;
   message += readLength;

 }
 return TMR_SUCCESS;
}


__declspec(dllexport) TMR_Status
s_setBaudRate(TMR_SR_SerialTransport *this, uint32_t rate)
{
  TMR_SR_SerialPortNativeContext *c;
  DCB   dcb;

  c = this->cookie;

  dcb.DCBlength = sizeof(DCB);
  GetCommState(c->handle, &dcb);
  dcb.BaudRate  = rate;

  if (0 == SetCommState(c->handle, &dcb))
  {
    return TMR_ERROR_COMM_ERRNO(GetLastError());
  }

  return TMR_SUCCESS;
}

static TMR_Status
s_shutdown(TMR_SR_SerialTransport *this)
{
  TMR_SR_SerialPortNativeContext *c;

  c = this->cookie;

  CloseHandle(c->handle);
  /* What, exactly, would be the point of checking for an error here? */

  return TMR_SUCCESS;
}

static TMR_Status
s_flush(TMR_SR_SerialTransport *this)
{
  TMR_SR_SerialPortNativeContext *c;

  c = this->cookie;

  if (PurgeComm(c->handle, PURGE_RXCLEAR) == 0)
  {
	  return TMR_ERROR_COMM_ERRNO(errno);
  }
  return TMR_SUCCESS;
}

__declspec(dllexport)TMR_Status
TMR_SR_SerialTransportNativeInit(TMR_SR_SerialTransport *transport,
                                 TMR_SR_SerialPortNativeContext *context,
                                 const char *device)
{
  // Transform COM port name from "/COMnn" to "\\\\.\\COMnn"
  if (strlen(device)-1 + 4 + 1 > TMR_MAX_READER_NAME_LENGTH)
  {
    return TMR_ERROR_INVALID;
  }
  snprintf(context->devicename, sizeof(context->devicename),
    "\\\\.\\%s", device+1);

  transport->cookie = context;
  transport->open = s_open;
  transport->sendBytes = s_sendBytes;
  transport->receiveBytes = s_receiveBytes;
  transport->setBaudRate = s_setBaudRate;
  transport->shutdown = s_shutdown;
  transport->flush = s_flush;

  return TMR_SUCCESS;
}
