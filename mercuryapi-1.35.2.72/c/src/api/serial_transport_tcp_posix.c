/**
 *  @file serial_transport_tcp_posix.c
 *  @brief Mercury API - TCP serial transport implementation for POSIX
 *  @author Lingaraj
 *  @date 04/16/2014
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#include "tm_reader.h"

#ifdef TMR_ENABLE_SERIAL_TRANSPORT_NATIVE
static TMR_Status
tcp_open(TMR_SR_SerialTransport *this)
{
  int sock;
  const char *port;
  char hostCopy[256];
  TMR_Status ret;
  int portNum = 0;
  TMR_SR_SerialPortNativeContext *c; 

  c = this->cookie;
  port = strchr(c->devicename, ':');
  strcpy(hostCopy, c->devicename + 1);
  portNum = atoi(port + 1);
  hostCopy[port - c->devicename -1 ] = '\0';

  static const struct addrinfo addrInfoMask =
  {    
    0,   
    AF_INET,
    SOCK_STREAM,
    0,                                                                                                                                                             
    0,   
    NULL,
    NULL,
    NULL 
  };   
  struct addrinfo *           hostAddress;
  int                         flag;
  struct sockaddr_in          sin; 
  int                         rc;

  /*
   * Look up host using getaddrinfo().
   * Gethostbyname() could be configured a lot of
   * different ways. There is /etc/hosts, DNS, NIS, etc, etc.
   * Suffice to say it is big, bulky, and susceptible to stall.
   */
  if(0 != getaddrinfo(hostCopy, NULL, &addrInfoMask, &hostAddress))
  {
    return TMR_ERROR_INVALID;
  }

  /*
   * Convert the address to sockaddr_in format
   */
  memset(&sin, 0, sizeof sin);
  sin.sin_family = AF_INET;
  sin.sin_addr = ((struct sockaddr_in *)(hostAddress->ai_addr))->sin_addr;
  sin.sin_port = htons(portNum);

  /*
   * Done withe the host addrinfo
   */
  freeaddrinfo(hostAddress);

  /*
   * Create the socket.
   */
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(0 > sock)
  {
    return TMR_ERROR_INVALID;
  }

  /*
   * Connect the socket to reader. This can stall.
   */
  rc = connect(sock, (struct sockaddr *)&sin, sizeof sin);
  if(0 > rc)
  {
    /* Connect failed */
    close(sock);
    return TMR_ERROR_INVALID;
  }

  /*
   * Best effort to set no delay. If this doesn't work
   * (no reason it shouldn't) we do not declare defeat.
   */
  flag = 1;
  setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void*)&flag, sizeof flag);

  /*
   * Record the socket in the connection instance
   */
  c->handle = sock;

  ret = TMR_SUCCESS;
 
  return ret;
}


static TMR_Status
tcp_sendBytes(TMR_SR_SerialTransport *this, uint32_t length, 
                uint8_t* message, const uint32_t timeoutMs)
{
  TMR_SR_SerialPortNativeContext *c;
  int ret;

  c = this->cookie;
  do 
  {
    ret = write(c->handle, message, length);
    if (ret == -1)
    {
      return TMR_ERROR_COMM_ERRNO(errno);
    }
    length -= ret;
    message += ret;
  }
  while (length > 0);

  return TMR_SUCCESS;
}


static TMR_Status
tcp_receiveBytes(TMR_SR_SerialTransport *this, uint32_t length, 
                   uint32_t* messageLength, uint8_t* message, const uint32_t timeoutMs)
{
  TMR_SR_SerialPortNativeContext *c;
  int ret;
  struct timeval tv;
  fd_set set;
  int status = 0;

  *messageLength = 0;
  c = this->cookie;

  do
  {
    FD_ZERO(&set);
    FD_SET(c->handle, &set);
    tv.tv_sec = timeoutMs/1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    /* Ideally should reset this timeout value every time through */
    ret = select(c->handle + 1, &set, NULL, NULL, &tv);
    if (ret < 1)
    {
      return TMR_ERROR_TIMEOUT;
    }
    ret = read(c->handle, message, length);
    if (ret == -1)
    {
      return TMR_ERROR_COMM_ERRNO(errno);
    }

    if (0 == ret)
    {
      /**
       * We should not be here, coming here means the select()
       * is success , but we are not able to read the data.
       * check the serial port connection status.
       **/
      ret = ioctl(c->handle, TIOCMGET, &status);
      if (-1 == ret)
      {
        /* not success. check for errno */
        if (EIO == errno)
        {
          /**
           * EIO means I/O error, may serial port got disconnected,
           * throw the error.
           **/
          return TMR_ERROR_TIMEOUT;
        }
      }
    }

    length -= ret;
    *messageLength += ret;
    message += ret;
  }
  while (length > 0);

  return TMR_SUCCESS;
}

#if 0
static TMR_Status
tcp_setBaudRate(TMR_SR_SerialTransport *this, uint32_t rate)
{

  /* This routine should change the baud rate of the serial connection
   * to the specified rate, or return TMR_ERROR_INVALID if the rate is
   * not supported.
   */

  return TMR_ERROR_UNIMPLEMENTED;
}
#endif

static TMR_Status
tcp_shutdown(TMR_SR_SerialTransport *this)
{
	TMR_Status ret = TMR_SUCCESS;
	TMR_SR_SerialPortNativeContext *c;

	c = this->cookie;

	if (0 > c->handle)
	{
		ret = TMR_ERROR_INVALID;
	}

	close(c->handle);

	c->handle = -1;

	return ret;

}

static TMR_Status
tcp_flush(TMR_SR_SerialTransport *this)
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
#endif
