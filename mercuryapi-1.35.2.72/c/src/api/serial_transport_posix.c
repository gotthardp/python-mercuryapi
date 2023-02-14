/**
 *  @file serial_transport_posix.c
 *  @brief Mercury API - POSIX serial transport implementation
 *  @author Nathan Williams
 *  @date 10/20/2009
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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include "tm_reader.h"

#ifdef __APPLE__
#include <sys/ioctl.h>
#include <IOKit/serial/ioss.h>
#endif

#ifdef TMR_ENABLE_SERIAL_TRANSPORT_NATIVE

static TMR_Status
s_open(TMR_SR_SerialTransport *this)
{
  int ret;
  struct termios t;
#if TMR_MAX_READER_NAME_LENGTH > 0
  TMR_SR_SerialPortNativeContext *c;

  c = this->cookie;

  c->handle = open(c->devicename, O_RDWR);
  if (c->handle == -1)
    return TMR_ERROR_COMM_ERRNO(errno);
#endif

  /*
   * Set 8N1, disable high-bit stripping, soft flow control, and hard
   * flow control (modem lines).
   */
  ret = tcgetattr(c->handle, &t);
  if (-1 == ret)
    return TMR_ERROR_COMM_ERRNO(errno);
  t.c_iflag &= ~(ICRNL | IGNCR | INLCR | INPCK | ISTRIP | IXANY 
                 | IXON | IXOFF | PARMRK);
  t.c_oflag &= ~OPOST;
  t.c_cflag &= ~(CRTSCTS | CSIZE | CSTOPB | PARENB);
  t.c_cflag |= CS8 | CLOCAL | CREAD | HUPCL;
  t.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  t.c_cc[VMIN] = 0;
  t.c_cc[VTIME] = 1;
  ret = tcsetattr(c->handle, TCSANOW, &t);
  if (-1 == ret)
    return TMR_ERROR_COMM_ERRNO(errno);

  ret = flock(c->handle, LOCK_EX | LOCK_NB);
  if ( -1 == ret)
    return TMR_ERROR_COMM_ERRNO(errno);

  return TMR_SUCCESS;
}

static TMR_Status
s_sendBytes(TMR_SR_SerialTransport *this, uint32_t length, 
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
      if (ENXIO == errno)
      {
        return TMR_ERROR_TIMEOUT; 
      }
      else
      {
        return TMR_ERROR_COMM_ERRNO(errno);
      }
    }
    length -= ret;
    message += ret;
  }
  while (length > 0);

  return TMR_SUCCESS;
}

static TMR_Status
s_receiveBytes(TMR_SR_SerialTransport *this, uint32_t length, 
               uint32_t *messageLength, uint8_t* message, const uint32_t timeoutMs)
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
    tv.tv_sec = timeoutMs / 1000;
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
      if (ENXIO == errno)
      {
        return TMR_ERROR_TIMEOUT; 
      }
      else
      {
        return TMR_ERROR_COMM_ERRNO(errno);
      }
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

static TMR_Status
s_setBaudRate(TMR_SR_SerialTransport *this, uint32_t rate)
{
  TMR_SR_SerialPortNativeContext *c;
  
  c = this->cookie;

#if defined(__APPLE__)
  {
    speed_t speed = rate;

    if (ioctl(c->handle, IOSSIOSPEED, &speed) == -1)
      return TMR_ERROR_COMM_ERRNO(errno);
  }
#else
  {
    struct termios t;

    tcgetattr(c->handle, &t);

#define BCASE(t,n) case n: cfsetispeed((t),B##n); cfsetospeed((t),B##n); break;
    switch (rate)
    {
      BCASE(&t, 9600);
      BCASE(&t, 19200);
      BCASE(&t, 38400);
      // Believe it or not, speeds beyond 38400 aren't required by POSIX.
#ifdef B57600
      BCASE(&t, 57600);
#endif
#ifdef B115200
      BCASE(&t, 115200);
#endif
#ifdef B230400
      BCASE(&t, 230400);
#endif
#ifdef B460800
      BCASE(&t, 460800);
#endif
#ifdef B921600
      BCASE(&t, 921600);
#endif
    default:
      return TMR_ERROR_INVALID;
    }
#undef BCASE
    if (tcsetattr(c->handle, TCSANOW, &t) != 0)
    {
      return TMR_ERROR_COMM_ERRNO(errno);
    }
  }
#endif /* __APPLE__ */

  return TMR_SUCCESS;
}

static TMR_Status
s_shutdown(TMR_SR_SerialTransport *this)
{
  TMR_SR_SerialPortNativeContext *c;

  c = this->cookie;

  close(c->handle);
  /* What, exactly, would be the point of checking for an error here? */

  return TMR_SUCCESS;
}

static TMR_Status
s_flush(TMR_SR_SerialTransport *this)
{
  TMR_SR_SerialPortNativeContext *c;

  c = this->cookie;

  if (tcflush(c->handle, TCOFLUSH) == -1)
  {
    return TMR_ERROR_COMM_ERRNO(errno);
  }

  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_SerialTransportNativeInit(TMR_SR_SerialTransport *transport,
                                 TMR_SR_SerialPortNativeContext *context,
                                 const char *device)
{

#if TMR_MAX_READER_NAME_LENGTH > 0
  if (strlen(device) + 1 > TMR_MAX_READER_NAME_LENGTH)
  {
    return TMR_ERROR_INVALID;
  }
  strcpy(context->devicename, device);
#else
  /* No space to store the device name, so open it now */
  context->handle = open(device, O_RDWR);
  if (context->handle == -1)
  {
    return TMR_ERROR_COMM_ERRNO(errno);
  }
#endif

  transport->cookie = context;
  transport->open = s_open;
  transport->sendBytes = s_sendBytes;
  transport->receiveBytes = s_receiveBytes;
  transport->setBaudRate = s_setBaudRate;
  transport->shutdown = s_shutdown;
  transport->flush = s_flush;

  return TMR_SUCCESS;
}

#endif
