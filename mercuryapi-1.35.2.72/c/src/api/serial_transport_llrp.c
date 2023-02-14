/**
 *  @file serial_transport_llrp.c
 *  @brief Mercury API - serial transport over LLRP passthrough
 *  @author Nathan Williams
 *  @date 11/19/2009
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
#include "tm_config.h"
#ifdef TMR_ENABLE_LLRP_TRANSPORT
#ifdef TMR_ENABLE_LLRP_SERIAL_TRANSPORT

#include <errno.h>
#include <netdb.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "tm_reader.h"
#include "tmr_utils.h"

#define LLRP_TYPE_CLOSE_CONNECTION_RESPONSE    4
#define LLRP_TYPE_CLOSE_CONNECTION            14
#define LLRP_TYPE_READER_EVENT_NOTIFICATION   63
#define LLRP_TYPE_CUSTOM_MESSAGE            1023

#define LLRP_PARAM_READER_EVENT_NOTIFICATION_DATA  246
#define LLRP_PARAM_CONNECTION_ATTEMPT_EVENT        256
#define  LLRP_PARAM_CONNECTION_ATTEMPT_EVENT_SUCCESS 0
#define  LLRP_PARAM_CONNECTION_ATTEMPT_EVENT_ANOTHER 4
#define LLRP_PARAM_CONNECTION_CLOSE_EVENT          257
#define LLRP_PARAM_LLRP_STATUS                     287
#define  LLRP_STATUS_M_SUCCESS                       0

#define LLRP_TM_ID 0x67ba
#define LLRP_TM_TYPE_DATA_REQUEST     1
#define LLRP_TM_TYPE_DATA_RESPONSE    2
#define LLRP_TM_TYPE_CONTROL_REQUEST  3
#define LLRP_TM_TYPE_CONTROL_RESPONSE 4
#define LLRP_TM_CONTROL_POWER 1
#define LLRP_TM_CONTROL_BAUD  2

struct llrp_message
{
  uint32_t len; /* Length pointed to by value, not the conventional LLRP 
                   length of the entire object */
  uint32_t id;
  uint8_t *value, *rawbuf;
  uint8_t ver;
  uint16_t type;
};

struct llrp_param
{
  uint16_t type; 
  uint16_t len; /* Length pointed to by value, not the conventional LLRP 
                    length of the entire object */
  uint8_t *value;
};



static TMR_Status
llrp_receive_message(int sockfd, struct llrp_message *message, uint8_t *buf,
                     uint16_t buflen, uint32_t timeout)
{
  int ret;
  ssize_t msgsize, retlen;
  struct timeval timeo;

  timeo.tv_sec = timeout / 1000;
  timeo.tv_usec = 1000 * (timeout % 1000);

  ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo));
  if (-1 == ret)
  {
    return TMR_ERROR_COMM_ERRNO(errno);
  }

  msgsize = 0;
  do
  {
    retlen = recv(sockfd, buf + msgsize, buflen - msgsize, 0);
    if (-1 == retlen)
    {
        if (EAGAIN == errno)
        {
            return TMR_ERROR_TIMEOUT;
        }
        else
        {
            return TMR_ERROR_COMM_ERRNO(errno);
        }
    }
    msgsize += retlen;
  }
  while ((msgsize < 10 || msgsize < GETU32AT(buf, 2)) && msgsize < buflen);

  message->ver = (GETU8AT(buf, 0) >> 2) & 0x7;
  message->type = GETU16AT(buf, 0) & 0x3ff;
  message->len =  GETU32AT(buf,2) - 10;
  message->id =   GETU32AT(buf,6);
  message->rawbuf = buf;
  message->value = buf + 10;
  
  return TMR_SUCCESS;
}

static TMR_Status
llrp_send_message(int sockfd, struct llrp_message *message)
{
  uint8_t *buf;
  uint8_t i;
  uint16_t msgsize;
  ssize_t retlen;

  buf = message->rawbuf;
  i = 0;
  SETU16(buf, i, (message->ver << 10) | message->type);
  SETU32(buf, i, 10 + message->len);
  SETU32(buf, i, message->id);

  msgsize = 10 + message->len;

  while (msgsize > 0)
  {
    retlen = send(sockfd, buf, msgsize, 0);
    if (-1 == retlen)
    {
      return TMR_ERROR_COMM_ERRNO(errno);
    }
    buf += retlen;
    msgsize -= retlen;
  }
  return TMR_SUCCESS;
}

/*
 * Array containing the length of each TV-type parameter
 * or -1 if there is no such parameter. TV-types outside of
 * this array are equivalent to -1.
 */
static int llrp_tv_param_len[] =
{
/* 0   1   2   3   4   5   6   7   8   9 */
  -1,  3,  9,  9,  9,  9,  2,  3,  3,  5,
   3,  3,  3, 13,  3,  3,  5,  3,  5
};

static TMR_Status
llrp_get_param(uint8_t *buf, uint16_t buflen, uint16_t paramtype,
               struct llrp_param *param)
{
  uint16_t offset, type, headersize, len;

  offset = 0;
  while (offset < buflen)
  {
    type = GETU16AT(buf, offset);
    if (type & 0x8000)
    {
      type = (type >> 8) & 0x7f;
      if (type > numberof(llrp_tv_param_len) ||
          -1 == llrp_tv_param_len[type])
      {
        return TMR_ERROR_LLRP;
      }
      headersize = 1;
      len = llrp_tv_param_len[type];
    }
    else
    {
      type = type & 0x3ff;
      headersize = 4;
      len = GETU16AT(buf, offset+2);
    }
    if (type == paramtype)
    {
      param->type = type;
      param->len = len - headersize;
      param->value = buf + offset + headersize;
      return TMR_SUCCESS;
    }
    offset += len;
  }

  return TMR_ERROR_NOT_FOUND;
}

static TMR_Status
llrp_check_success(uint8_t *msgptr, uint16_t msglen)
{
  struct llrp_param param;
  TMR_Status ret;

  ret = llrp_get_param(msgptr, msglen,
                       LLRP_PARAM_LLRP_STATUS, &param);
  if (TMR_SUCCESS != ret)
    return TMR_ERROR_LLRP;

  if (LLRP_STATUS_M_SUCCESS != GETU16AT(param.value, 0))
    return TMR_ERROR_LLRP; /** @todo do something with the LLRP error value? */

  return TMR_SUCCESS;
}

/* Wrap llrp_receive_message() in a loop that filters out
 * connection-attempt notifications, which the server can send us at
 * any time.
 */
static TMR_Status
llrp_receive_message_filtered(int sockfd, struct llrp_message *message,
                              uint8_t *buf, uint16_t buflen, uint32_t timeout)
{
  struct llrp_param param;
  TMR_Status ret;
  uint16_t status;

  while (1)
  {
    ret = llrp_receive_message(sockfd, message, buf, buflen, timeout);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }

    if (LLRP_TYPE_READER_EVENT_NOTIFICATION != message->type)
      break; /* Got the message we were looking for */

    ret = llrp_get_param(message->value, message->len,
                         LLRP_PARAM_READER_EVENT_NOTIFICATION_DATA, &param);
    if (TMR_ERROR_NOT_FOUND == ret)
    {
      /* Not the reader event notification type we're filtering */
      break;
    }
    else if (TMR_SUCCESS != ret)
    {
      return ret;
    }

    ret = llrp_get_param(param.value, param.len,
                         LLRP_PARAM_CONNECTION_ATTEMPT_EVENT, &param);
    if (TMR_ERROR_NOT_FOUND == ret)
    {
      /* Not the reader event notification type we're filtering */
      break;
    }
    else if (TMR_SUCCESS != ret)
    {
      return ret;
    }

    status = GETU16AT(param.value, 0);
    if (LLRP_PARAM_CONNECTION_ATTEMPT_EVENT_ANOTHER != status)
    {
      break;
    }
  }

  return TMR_SUCCESS;
}


static TMR_Status
s_open(TMR_SR_SerialTransport *this)
{
  TMR_Status ret;
  TMR_SR_LlrpEapiTransportContext *context;
  struct llrp_message message;
  struct llrp_param param;
  uint8_t buf[270];
  uint16_t status;

  context = this->cookie;

  if (-1 == connect(context->socket, (struct sockaddr *)&context->addr,
                    context->addrlen))
  {
    return TMR_ERROR_COMM_ERRNO(errno);
  }
  
  ret = llrp_receive_message(context->socket, &message, buf, sizeof(buf), 0);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  if (LLRP_TYPE_READER_EVENT_NOTIFICATION != message.type)
  {
    return TMR_ERROR_LLRP;
  }

  ret = llrp_get_param(message.value, message.len,
                       LLRP_PARAM_READER_EVENT_NOTIFICATION_DATA, &param);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  ret = llrp_get_param(param.value, param.len,
                       LLRP_PARAM_CONNECTION_ATTEMPT_EVENT, &param);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  status = GETU16AT(param.value,0);
  if (LLRP_PARAM_CONNECTION_ATTEMPT_EVENT_SUCCESS != status)
  {
    return TMR_ERROR_LLRP;
  }

  if (true == context->powerCycleAtOpen)
  {
    ret = TMR_LlrpEapiPowerCycle(this);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
  }

  return TMR_SUCCESS;
}

static TMR_Status
s_sendBytes(TMR_SR_SerialTransport *this, uint32_t length, 
            uint8_t* bytes, const uint32_t timeoutMs)
{
  TMR_SR_LlrpEapiTransportContext *context;
  struct llrp_message message;
  uint8_t buf[270];
  int i;

  context = this->cookie;
  i = 0;

  message.ver = 1;
  message.type = LLRP_TYPE_CUSTOM_MESSAGE;
  message.id = context->sequenceId++;
  message.rawbuf = buf;
  message.value = buf + 10;

  SETU32(message.value, i, LLRP_TM_ID);
  SETU8(message.value, i, LLRP_TM_TYPE_DATA_REQUEST);
  memcpy(message.value + i, bytes, length);
  message.len = length + i;

  return llrp_send_message(context->socket, &message);
}

static TMR_Status
s_receiveBytes(TMR_SR_SerialTransport *this, uint32_t length, 
               uint32_t* messageLength, uint8_t* bytes, const uint32_t timeoutMs)
{
  TMR_SR_LlrpEapiTransportContext *context;
  struct llrp_message message;
  TMR_Status ret;
  uint16_t copylen;
  uint8_t buf[270];

  *messageLength = 0;
  context = this->cookie;

  if (length > 255)
  {
    return TMR_ERROR_INVALID;
  }
  
  if (context->buflen > 0)
  {
    copylen = context->buflen;
    if (copylen > length)
    {
      copylen = length;
    }
    memcpy(bytes, context->buf + context->bufstart, copylen);
    context->buflen -= copylen;
    context->bufstart += copylen;
    bytes += copylen;
    *messageLength += copylen;
    length -= copylen;
  }

  if (length == 0)
  {
    return TMR_SUCCESS;
  }

  ret = llrp_receive_message_filtered(context->socket, &message, buf,
                                      sizeof(buf), timeoutMs);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  if (LLRP_TM_ID != GETU32AT(message.value, 0))
  {
    return TMR_ERROR_LLRP; /* Not for us */
  }
  if (LLRP_TM_TYPE_DATA_RESPONSE != GETU8AT(message.value, 4))
  {
    return TMR_ERROR_LLRP; /* Not the data we expected */
  }

  copylen = message.len - 5;
  if (copylen > length)
  {
    /* Stash the rest of the data for next time */
    memcpy(context->buf, message.value + 5 + length, copylen - length);
    context->bufstart = 0;
    context->buflen = copylen - length;
    copylen = length;
  }
  memcpy(bytes, message.value + 5, copylen);

  return TMR_SUCCESS;
}

static TMR_Status
s_setBaudRate(TMR_SR_SerialTransport *this, uint32_t rate)
{
  TMR_Status ret;
  TMR_SR_LlrpEapiTransportContext *context;
  struct llrp_message message;
  uint8_t buf[270];
  int i;

  context = this->cookie;
  i = 0;

  message.ver = 1;
  message.type = LLRP_TYPE_CUSTOM_MESSAGE;
  message.id = context->sequenceId++;
  message.rawbuf = buf;
  message.value = buf + 10;

  SETU32(message.value, i, LLRP_TM_ID);
  SETU8(message.value, i, LLRP_TM_TYPE_CONTROL_REQUEST);
  SETU16(message.value, i, LLRP_TM_CONTROL_BAUD);
  SETU32(message.value, i, rate);

  message.len = i;

  ret = llrp_send_message(context->socket, &message);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  ret = llrp_receive_message_filtered(context->socket, &message, buf,
                                      sizeof(buf), 0);

  if ((LLRP_TYPE_CUSTOM_MESSAGE != message.type) ||
      (LLRP_TM_ID != GETU32AT(message.value, 0)))
  {
    return TMR_ERROR_LLRP; /* Not for us */
  }

  if (LLRP_TM_TYPE_CONTROL_RESPONSE != GETU8AT(message.value, 4))
  {
    return TMR_ERROR_LLRP; /* Not the data we expected */
  }

  return llrp_check_success(message.value + 5, message.len - 5);
}

static TMR_Status
s_shutdown(TMR_SR_SerialTransport *this)
{
  TMR_Status ret;
  TMR_SR_LlrpEapiTransportContext *context;
  struct llrp_message message;
  uint8_t buf[270];

  context = this->cookie;

  message.ver = 1;
  message.type = LLRP_TYPE_CLOSE_CONNECTION;
  message.id = context->sequenceId++;
  message.rawbuf = buf;
  message.value = buf + 10;
  message.len = 0;

  ret = llrp_send_message(context->socket, &message);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  
  ret = llrp_receive_message_filtered(context->socket, &message, buf,
                                      sizeof(buf), 0);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  if (LLRP_TYPE_CLOSE_CONNECTION_RESPONSE != message.type)
  {
    return TMR_ERROR_LLRP; /* Not what we expected */
  }

  return llrp_check_success(message.value, message.len);
}

static TMR_Status
s_flush(TMR_SR_SerialTransport *this)
{

  return TMR_SUCCESS;
}


TMR_Status
TMR_LlrpEapiPowerCycle(TMR_SR_SerialTransport *this)
{
  TMR_Status ret;
  TMR_SR_LlrpEapiTransportContext *context;
  struct llrp_message message;
  uint8_t buf[270];
  int i;

  context = this->cookie;
  i = 0;

  message.ver = 1;
  message.type = LLRP_TYPE_CUSTOM_MESSAGE;
  message.id = context->sequenceId++;
  message.rawbuf = buf;
  message.value = buf + 10;

  SETU32(message.value, i, LLRP_TM_ID);
  SETU8(message.value, i, LLRP_TM_TYPE_CONTROL_REQUEST);
  SETU16(message.value, i, LLRP_TM_CONTROL_POWER);

  message.len = i;

  ret = llrp_send_message(context->socket, &message);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  ret = llrp_receive_message(context->socket, &message, buf, sizeof(buf), 0);

  if ((LLRP_TYPE_CUSTOM_MESSAGE != message.type) ||
      (LLRP_TM_ID != GETU32AT(message.value, 0)))
  {
    return TMR_ERROR_LLRP; /* Not for us */
  }
  if (LLRP_TM_TYPE_CONTROL_RESPONSE != GETU8AT(message.value, 4))
  {
    return TMR_ERROR_LLRP; /* Not the data we expected */
  }

  return llrp_check_success(message.value + 5, message.len - 5);
}

TMR_Status
TMR_SR_LlrpEapiTransportInit(TMR_SR_SerialTransport *transport,
                             TMR_SR_LlrpEapiTransportContext *context,
                             const char *host, int port, bool powerCycleAtOpen)
{
  int ret, sfd;
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  char portnum[12];

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
#ifdef AI_ADDRCONFIG
  hints.ai_flags = AI_ADDRCONFIG;
#else
  hints.ai_flags = 0;
#endif

  sprintf(portnum, "%d", port);
  ret = getaddrinfo(host, portnum, &hints, &result);
  if (ret != 0)
  {
    return TMR_ERROR_NO_HOST;
  }
  
  for (rp = result; rp != NULL; rp = rp->ai_next)
  {
    sfd = socket(rp->ai_family, rp->ai_socktype,
                 rp->ai_protocol);
    if (-1 != sfd)
    {
      break;
    }
  }

  if (NULL == rp)
  {
    ret = TMR_ERROR_COMM_ERRNO(errno);
    goto out;
  }

  context->socket = sfd;
  memcpy(&context->addr, rp->ai_addr, rp->ai_addrlen);
  context->addrlen = rp->ai_addrlen;
  context->powerCycleAtOpen = powerCycleAtOpen;
  context->sequenceId = 1;
  context->bufstart = 0;
  context->buflen = 0;

  transport->cookie = context;
  transport->open = s_open;
  transport->sendBytes = s_sendBytes;
  transport->receiveBytes = s_receiveBytes;
  transport->setBaudRate = s_setBaudRate;
  transport->shutdown = s_shutdown;
  transport->flush = s_flush;
  ret = TMR_SUCCESS;

out:
  freeaddrinfo(result);
  return ret;
}

#endif /* TMR_ENABLE_LLRP_SERIAL_TRANSPORT */
#endif /* TMR_ENABLE_LLRP_TRANSPORT */
