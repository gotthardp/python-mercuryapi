/**
 *  @file serial_transport_tcp_win32.c
 *  @brief Mercury API - TCP serial transport implementation for Win32
 *  @author Lingaraj
 *  @date 04/16/2014
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
#include <stdio.h>
#if !defined(_WINSOCK2API_) && !defined(_WINSOCKAPI_)
#include <winsock2.h>
#endif
#pragma comment(lib, "Ws2_32.lib")

#  if defined(WINCE)
#    define snprintf _snprintf
#  else
#    define snprintf sprintf_s
#  endif

#endif /* WIN32 */

#include "tm_reader.h"

__declspec(dllexport) TMR_Status
tcp_open(TMR_SR_SerialTransport *this)
{
  TMR_SR_SerialPortNativeContext *c;
  struct hostent *he;
  struct in_addr **addr_list;
  struct sockaddr_in server;
  char ip[100];
  char hostCopy[256];
  SOCKET s;
  WSADATA wsa;
  uint8_t i;
  int portNum = 0;
  const char* port;
  TMR_Status ret = TMR_SUCCESS;

  c = this->cookie;
  port = strchr(c->devicename, ':');
  strcpy(hostCopy, c->devicename + 1);
  portNum = atoi(port + 1);
  hostCopy[port - c->devicename -1 ] = '\0';

  /* Initialising Winsock... */
  if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
  {
	  ret = WSAGetLastError();
	  WSACleanup();
	  return ret;
  }

  if ( (he = gethostbyname( hostCopy ) ) == NULL)
  {
	  ret = WSAGetLastError();
	  WSACleanup();
	  return ret;
  }
  addr_list = (struct in_addr **) he->h_addr_list;
  for(i = 0; addr_list[i] != NULL; i++)
  {
	  //Return the first one;
	  strcpy(ip , inet_ntoa(*addr_list[i]) );
  }

  /* create a socket */
  if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
  {
	  /* Could not create socket */
	  ret = WSAGetLastError();
	  WSACleanup();
	  return ret;
  }

  server.sin_addr.s_addr = inet_addr(ip);
  server.sin_family = AF_INET;
  server.sin_port = htons(portNum);

  /* Connect the socket to the reader. This can stall. */
  if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
  {
	  /* Connect error */
	  closesocket(s);
	  WSACleanup();
	  return TMR_ERROR_INVALID;
  }

  /* Record the socket in the connection instance */
  c->handle = (PLATFORM_HANDLE)s;
  
  return TMR_SUCCESS;
}

__declspec(dllexport) TMR_Status
tcp_sendBytes(TMR_SR_SerialTransport *this, uint32_t length,
            uint8_t* message, const uint32_t timeoutMs)
{
  TMR_SR_SerialPortNativeContext *c;
  uint32_t sendLen = 0;
  TMR_Status ret = TMR_SUCCESS;
  
  c = this->cookie;
    
  sendLen = send( (SOCKET)c->handle, message, (int)length, 0 );
  if (sendLen == SOCKET_ERROR)
  {
	  /* send failed */
	  closesocket((SOCKET)c->handle);
	  WSACleanup();
	  return ret;
  }
  return ret;
}

__declspec(dllexport) TMR_Status
tcp_receiveBytes(TMR_SR_SerialTransport *this, uint32_t length,
              uint32_t* messageLength, uint8_t* message, const uint32_t
timeoutMs)
{
 int32_t readLength;
 TMR_Status ret = TMR_SUCCESS;
 TMR_SR_SerialPortNativeContext *c;
 DWORD dwTime = timeoutMs;

 c = this->cookie;
 *messageLength = 0;

 setsockopt((SOCKET)c->handle, SOL_SOCKET, SO_RCVTIMEO, (const char*)&dwTime, sizeof(dwTime));

 while (length > 0)
 {
   readLength=0;
   readLength = recv((SOCKET)c->handle, message, length, 0);   
   if (readLength > 0)
   {
	   /* bytes received */
	   *messageLength += readLength;
   }
   else if(readLength == 0)
   {
	   /* send failed */
        closesocket((SOCKET)c->handle);
        WSACleanup();
		return TMR_ERROR_TIMEOUT;
   }
   else
   {
	   ret = WSAGetLastError();
     if (WSAETIMEDOUT == ret)
     {
       return TMR_ERROR_TIMEOUT;
     }
	   return ret;
   }
   length -= readLength;
   message += readLength;
 }
 return ret;
}

#if 0
__declspec(dllexport) TMR_Status
tcp_setBaudRate(TMR_SR_SerialTransport *this, uint32_t rate)
{
  TMR_SR_SerialPortNativeContext *c;
  DCB   dcb;

  c = this->cookie;

  dcb.DCBlength = sizeof(DCB);
  GetCommState(c->handle, &dcb);
  dcb.BaudRate  = rate;

  if (0 == SetCommState(c->handle, &dcb)) /* @todo use GetLastError() */
  {
    return TMR_ERROR_INVALID;
  }

  return TMR_SUCCESS;
}
#endif

static TMR_Status
tcp_shutdown(TMR_SR_SerialTransport *this)
{
  TMR_SR_SerialPortNativeContext *c;
  TMR_Status ret = TMR_SUCCESS;

  c = this->cookie;
  
  closesocket((SOCKET)c->handle);
  WSACleanup();

  return ret;
}

static TMR_Status
tcp_flush(TMR_SR_SerialTransport *this)
{

  return TMR_SUCCESS;
}

__declspec(dllexport)TMR_Status
TMR_SR_SerialTransportTcpNativeInit(TMR_SR_SerialTransport *transport,
                                 TMR_SR_SerialPortNativeContext *context,
                                 const char *device)
{
  if (strlen(device) + 1 > TMR_MAX_READER_NAME_LENGTH)
  {
	  return TMR_ERROR_INVALID;
  }
  strcpy(context->devicename, device);

  transport->cookie = context;
  transport->open = tcp_open;
  transport->sendBytes = tcp_sendBytes;
  transport->receiveBytes = tcp_receiveBytes;
  transport->setBaudRate = NULL;
  transport->shutdown = tcp_shutdown;
  transport->flush = tcp_flush;

  return TMR_SUCCESS;
}
