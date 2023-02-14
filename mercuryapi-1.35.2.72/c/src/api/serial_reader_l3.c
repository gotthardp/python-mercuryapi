/**
 *  @file serial_reader_l3.c
 *  @brief Mercury API - serial reader low level implementation
 *  @author Nathan Williams
 *  @date 11/2/2009
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

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tm_reader.h"
#include "serial_reader_imp.h"
#include "tmr_utils.h"

#ifdef TMR_ENABLE_UHF
#define NUMBER_OF_MULTISELECT_SUPPORTED 3u
#endif /* TMR_ENABLE_UHF */

#ifdef TMR_ENABLE_HF_LF
#define NUMBER_OF_HF_LF_MULTISELECT_SUPPORTED 2u
#endif /* TMR_ENABLE_HF_LF */

#ifdef TMR_ENABLE_UHF
bool isMultiSelectEnabled = false;
bool isEmbeddedTagopEnabled = false;
bool isSecureAccessEnabled = false;
bool LBTEnable = false;
int8_t LBTThreshold;
bool DwellTimeEnable = false;
uint16_t DwellTime;
bool regionConfigurationFlag = false;
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_HF_LF
bool isAddrByteExtended = false;
#endif /* TMR_ENABLE_HF_LF */
extern int delta;
uint16_t totalMsgIdx = 0;

/* This flag is used/enabled in "autonomousmode.c" codelet.
 * Used in streaming option.
 */
bool isStreamEnabled = false;
int LastDSPMicro = 0;

void
notify_read_listeners(TMR_Reader *reader, TMR_TagReadData *trd);
#ifdef TMR_ENABLE_SERIAL_READER
static TMR_Status filterbytes(TMR_TagProtocol protocol,
                              const TMR_TagFilter *filter, 
                              uint8_t *option, uint8_t *i, uint8_t *msg,
                              uint32_t accessPassword, bool usePassword);
TMR_Status
TMR_SR_sendBytes(TMR_Reader *reader, uint8_t len, uint8_t *data, uint32_t timeoutMs);

/*
 * ThingMagic-mutated CRC used for messages.
 * Notably, not a CCITT CRC-16, though it looks close.
 */
static uint16_t crctable[] = 
{
  0x0000, 0x1021, 0x2042, 0x3063,
  0x4084, 0x50a5, 0x60c6, 0x70e7,
  0x8108, 0x9129, 0xa14a, 0xb16b,
  0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
};

static uint16_t
tm_crc(uint8_t *u8Buf, uint8_t len)
{
  uint16_t crc;
  int i;

  crc = 0xffff;

  for (i = 0; i < len ; i++)
  {
    crc = ((crc << 4) | (u8Buf[i] >> 4))  ^ crctable[crc >> 12];
    crc = ((crc << 4) | (u8Buf[i] & 0xf)) ^ crctable[crc >> 12];
  }

  return crc;
}

uint8_t 
parseEBVdata(uint8_t* msg, uint8_t *ebvValue, uint8_t *idx)
{
  uint8_t len = 0;
  uint8_t i   = 0;
  ebvValue[len++] = GETU8(msg, *idx);

  /* If MSB of retrieved byte is set, then extract next byte
   * and add it to ebvValue. Repeat this process for 5 bytes.
   */
  for(i = 0; ((ebvValue[i] & 0x80) && (i < 7)); i++)
  {
    ebvValue[len++] = GETU8(msg, *idx);
  }
  return len;
}

/**
 * Send a byte string
 *
 * @param reader The reader
 * @param[in] len Number of bytes to send
 * @param[in] data Bytes to send, with length in byte 1. Byte 0 is reserved for the SOF character, and two characters at the end are reserved for the CRC.
 * @param timeoutMs Timeout value.
 */
TMR_Status
TMR_SR_sendBytes(TMR_Reader *reader, uint8_t len, uint8_t *data, uint32_t timeoutMs)
{
  TMR_SR_SerialTransport *transport;
  TMR_Status ret;

  transport = &reader->u.serialReader.transport;

  if (NULL != reader->transportListeners)
  {
    TMR__notifyTransportListeners(reader, true, len, data, timeoutMs);
  }

  ret = transport->sendBytes(transport, len, data, timeoutMs);
  return ret;
}

/**
 * TMR_flush function sends Flush bytes when Timeout error occurs 
 * to connect smoothly for the next command
 */
void 
TMR_flush(struct TMR_Reader* rp)
{
  uint32_t timeoutMs = 1000;
  uint8_t i = 0;
  uint8_t flushBytes[] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };
  for (i = 0; i < 30; i++)
  {
    TMR_SR_sendBytes(rp, sizeof(flushBytes) / sizeof(uint8_t), flushBytes, timeoutMs);
  }
}

/**
 * Send a message to the reader
 *
 * @param reader The reader
 * @param[in] data Message to send, with length in byte 1. Byte 0 is reserved for the SOF character, and two characters at the end are reserved for the CRC.
 * @param[out] opcode Opcode sent with message (pass this value to receiveMessage to match against response)
 * @param timeoutMs Timeout value.
 */
TMR_Status
TMR_SR_sendMessage(TMR_Reader *reader, uint8_t *data, uint8_t *opcode, uint32_t timeoutMs)
{
  TMR_SR_SerialReader *sr;
  TMR_Status ret;
  uint16_t crc;
  uint8_t len;
  uint8_t j;
  //uint8_t byteLen;

  sr = &reader->u.serialReader;
  timeoutMs += sr->transportTimeout;
/*  if (reader->u.serialReader.crcEnabled)
  {
    byteLen = 5;
  }
  else
  {
    byteLen = 3;
  }*/
#ifdef TMR_ENABLE_UHF
  /* Wake up processor from deep sleep.  Tickle the RS-232 line, then
   * wait a fixed delay while the processor spins up communications again. */
  if (sr->supportsPreamble && ((sr->powerMode == TMR_SR_POWER_MODE_INVALID) ||
                              (sr->powerMode == TMR_SR_POWER_MODE_SLEEP)) )
  {
    uint8_t flushBytes[] = {
      0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    };

    TMR_SR_sendBytes(reader, sizeof(flushBytes)/sizeof(uint8_t), flushBytes, timeoutMs);
    {
      //uint32_t bytesper100ms;
      uint32_t bytesSent;

      /* Calculate fixed delay in terms of byte-lengths at current speed */
      /* @todo Optimize delay length.  This value (100 bytes at 9600bps) is taken
       * directly from arbser, which was itself using a hastily-chosen value. */
      //bytesper100ms = sr->baudRate / 50;
      for (bytesSent=0; bytesSent < 24;
         bytesSent++)
      {
        TMR_SR_sendBytes(reader, sizeof(flushBytes)/sizeof(uint8_t), flushBytes, timeoutMs);
        /* Introducing 25 ms delay between each Iteration */
        tmr_sleep(9); 
      }
    }
  }    
#endif /* TMR_ENABLE_UHF */
  /* Layout of message in data array: 
   * [0] [1] [2] [3] [4]  ... [LEN+2] [LEN+3] [LEN+4]
   * FF  LEN OP  xx  xx   ... xx      CRCHI   CRCLO
   */
  data[0] = 0xff;
  len = data[1];
  *opcode = data[2];

  if (isContinuousReadParamSupported(reader))
  {
	  for (j = 0; j <= len; j++)
	  {
		  reader->paramMessage[j] = data[2 + j];
	  }
	  data[1] = 5 + len;
	  data[2] = 0x2f;
	  data[3] = 0x00;
	  data[4] = 0x00;
	  data[5] = 0x04;
	  data[6] = len;
	  for (j = 0; j <= len ; j++)
	  {
		  data[7 + j] = reader->paramMessage[j];
	  }
	  len = data[1];
	  reader->paramWait = true;
  }

 // if (reader->u.serialReader.crcEnabled)
  {
  crc = tm_crc(&data[1], len + 2);
  data[len + 3] = crc >> 8;
  data[len + 4] = crc & 0xff;
  }

  /* + 4 includes length of SOH + cmdLen + CRC */ 
  if((totalMsgIdx + 4) > 255)
  {
    totalMsgIdx = 0;
    return TMR_ERROR_CMDLEN_EXCEED_LIMIT;
  }
  totalMsgIdx = 0;

  ret = TMR_SR_sendBytes(reader, len+5, data, timeoutMs);
  return ret;
}


bool
isContinuousReadParamSupported(TMR_Reader *reader)
{
#ifdef TMR_ENABLE_UHF
	uint8_t *readerVersion = reader->u.serialReader.versionInfo.fwVersion ;
	uint8_t checkVersion[4];
#endif /* TMR_ENABLE_UHF */

	if (reader->hasContinuousReadStarted == false || reader->continuousReading == false)
		return false;

	switch (reader->u.serialReader.versionInfo.hardware[0])
	{
#ifdef TMR_ENABLE_UHF
		case TMR_SR_MODEL_M6E:
		case TMR_SR_MODEL_M6E_I:
			checkVersion[0] = 0x01; checkVersion[1] = 0x21; checkVersion[2] = 0x01; checkVersion[3] = 0x19;
			break;
		case TMR_SR_MODEL_MICRO:
			checkVersion[0] = 0x01; checkVersion[1] = 0x09; checkVersion[2] = 0x00; checkVersion[3] = 0x14;
			break;
		case TMR_SR_MODEL_M6E_NANO:
			checkVersion[0] = 0x01; checkVersion[1] = 0x07; checkVersion[2] = 0x00; checkVersion[3] = 0x0E;
			break;
        case TMR_SR_MODEL_M7E_PICO:
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_HF_LF
		case TMR_SR_MODEL_M3E:
#endif /* TMR_ENABLE_HF_LF */
            return true;
		default:
           return false;
	}
#ifdef TMR_ENABLE_UHF
    return versionCompare(readerVersion, checkVersion);
#endif /* TMR_ENABLE_UHF */
}

/**
 * Receive a response.
 *
 * @param reader The reader
 * @param[in] data Message to send, with length in byte 1. Byte 0 is reserved for the SOF character, and two characters at the end are reserved for the CRC.
 * @param[out] data Message received.
 * @param opcode Opcode that was sent with message that elicited this response, to be matched against incoming response opcode.
 * @param timeoutMs Timeout value.
 */
TMR_Status
TMR_SR_receiveMessage(TMR_Reader *reader, uint8_t *data, uint8_t opcode, uint32_t timeoutMs)
{
  TMR_Status ret;
  uint16_t crc, status;
  uint8_t len = 0;
  uint8_t receiveBytesLen;
  uint32_t inlen = 0;
  uint32_t rxcount = 0;
  uint8_t sohIndex = 0;
  TMR_SR_SerialTransport *transport;
  uint8_t retryCount = 0;
  bool sohFound = false;

  transport = &reader->u.serialReader.transport;
  timeoutMs += reader->u.serialReader.transportTimeout;
  if(opcode == 0x22)
  {
#if defined(TMR_ENABLE_BACKGROUND_READS)|| defined(SINGLE_THREAD_ASYNC_READ)
    timeoutMs += reader->readParams.asyncOffTime;
#endif
  }

  /**
   * Initialize the receive bytes length based on
   * sr->expectCRC option
   **/
#ifdef TMR_ENABLE_UHF
  if (reader->u.serialReader.crcEnabled)
#endif /* TMR_ENABLE_UHF */
  {
    receiveBytesLen = 7;
  }
#if TMR_ENABLE_CRC
#ifdef TMR_ENABLE_UHF
  else
  {
    receiveBytesLen = 5;
  }
#endif /* TMR_ENABLE_UHF */
#endif /* TMR_ENABLE_CRC */

  do
  {
    /* Pull at least receiveBytesLen bytes on first serial receive */
    ret = transport->receiveBytes(transport, (receiveBytesLen - inlen), &rxcount, (data + inlen), timeoutMs);
    if (TMR_SUCCESS != ret)
    {
      /* @todo Figure out how many bytes were actually obtained in a failed receive */
      return ret;
    }
    inlen += rxcount;

    if (inlen < receiveBytesLen)
    {
      /* SOH not found or data not received!! Retry */
      continue;
    }

    /* Search for SOH(0xFF) */
    for (sohIndex = 0; sohIndex < (receiveBytesLen - 2); sohIndex++)
    {
      /* <Valid SOH(0xFF)> + <Valid length(Less than 0xF8)> + <Valid OPCODE> */
      if ((data[sohIndex + 0] == 0xFF) && (data[sohIndex + 1] <= 0xF8))
      {
        if ((data[sohIndex + 2] == opcode)
        || (data[sohIndex + 2] == 0x22)
        || (data[sohIndex + 2] == 0x2F)
        || (data[sohIndex + 2] == 0x9D))
        {
          if (sohIndex == 0)
          {
            sohFound = true;
          }
          break; /* EXIT FOR */
        }
      }
    }
    if (sohFound)
      break;
    else
    {
      /* Update inlen with correct length after discarding invalid bytes. */
      inlen = receiveBytesLen - sohIndex;
      /* Now, copy the inlen number of bytes to data buffer at 0th index. */
      memmove(data, data+sohIndex, inlen);
    }
  }while (++retryCount < 20);

  if (retryCount >= 20)
  {
    return TMR_ERROR_TIMEOUT;
  }

  /* After this point, we have the the bare minimum (5 or 7)  of bytes in the buffer */

  /* Layout of response in data array: 
   * [0] [1] [2] [3]      [4]      [5] [6]  ... [LEN+4] [LEN+5] [LEN+6]
   * FF  LEN OP  STATUSHI STATUSLO xx  xx   ... xx      CRCHI   CRCLO
   */
  len = data[1];

  if (0 == len)
  {
    inlen = 0;
  }
  else if ((TMR_SR_MAX_PACKET_SIZE - receiveBytesLen) < len)
  {
    /**
     * packet data size is overflowing the buffer size. This could be a
     * corrupted packet. Discard it and move on. Return back with TMR_ERROR_TOO_BIG
     * error.
     **/
    return TMR_ERROR_TOO_BIG;
  }
  else
  {
    ret = transport->receiveBytes(transport, len, &inlen, data + receiveBytesLen, timeoutMs);
  }

  if (NULL != reader->transportListeners)
  {
    TMR__notifyTransportListeners(reader, false, inlen + receiveBytesLen, data, timeoutMs);
  }

  if (TMR_SUCCESS != ret)
  {
    /* before we can actually process the message, we have to properly receive the message */
    return ret;
  }

  /**
   * Calculate the CRC only if expectCRC option
   * is enabled
   **/
#ifdef TMR_ENABLE_UHF
  if (reader->u.serialReader.crcEnabled)
#endif /* TMR_ENABLE_UHF */
  {
  crc = tm_crc(&data[1], len + 4);
  if ((data[len + 5] != (crc >> 8)) ||
      (data[len + 6] != (crc & 0xff)))
  {
    return TMR_ERROR_CRC_ERROR;
  }
  }

#ifdef TMR_ENABLE_HF_LF
  if(TMR_SR_MODEL_M3E == reader->u.serialReader.versionInfo.hardware[0])
  {
    if (data[2] == 0x06)
    {
      /* Wait for 100ms */
      tmr_sleep(100);
    }
  }
#endif /* TMR_ENABLE_HF_LF */

  if ((data[2] != opcode) && ((data[2] != 0x2F) || (!reader->continuousReading)))
  {
    if(data[2] == 0x9D)
     {
       return TMR_ERROR_AUTOREAD_ENABLED;
     }

    /* We got a response for a different command than the one we
     * sent. This usually means we received the boot-time message from
     * a M6e, and thus that the device was rebooted somewhere between
     * the previous command and this one. Report this as a problem.
     */

    else if(data[2] == 0x04)      // Boot response received.
    {
      /* Need to flush the response of command which was sent earlier. */
      ret = transport->flush(transport); 
      if (ret == TMR_SUCCESS)
      {
        return TMR_ERROR_BOOT_RESPONSE;
      }
    }

    return TMR_ERROR_DEVICE_RESET;
 }

  status = GETU16AT(data, 3);
  if (status != 0)
  {
    ret = TMR_ERROR_CODE(status);
    if (ret == TMR_ERROR_TM_ASSERT_FAILED)
    {
	  uint32_t line;
	  uint8_t *assert = (uint8_t *) (data + 5);

	  memset(reader->u.serialReader.errMsg, 0 ,TMR_SR_MAX_PACKET_SIZE);
	  line = GETU32AT(assert, 0);
	  sprintf(reader->u.serialReader.errMsg, "Assertion failed at line %"PRId32" in file ", line);
	  memcpy(reader->u.serialReader.errMsg + strlen(reader->u.serialReader.errMsg), assert + 4, len - 4);
  }
  }  
  return ret;
}

/**
 * Send a message and receive a response.
 *
 * @param reader The reader
 * @param[in] data Message to send, with length in byte 1. Byte 0 is reserved for the SOF character, and two characters at the end are reserved for the CRC.
 * @param[out] data Message received.
 * @param timeoutMs Timeout value.
 */
TMR_Status
TMR_SR_sendTimeout(TMR_Reader *reader, uint8_t *data, uint32_t timeoutMs)
{
  TMR_Status ret;
  uint8_t opcode;
  ret = TMR_SR_sendMessage(reader, data, &opcode, timeoutMs);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  if (isContinuousReadParamSupported(reader))
  {
	  while(reader->paramWait)
	  {
#ifdef SINGLE_THREAD_ASYNC_READ
		TMR_TagReadData trd;
		ret = TMR_hasMoreTags(reader);
		if (TMR_SUCCESS == ret)
		{
			TMR_getNextTag(reader, &trd);
			notify_read_listeners(reader, &trd);
		}
#endif
	  }

	  reader->paramWait = false;	  
	  memcpy(data , &reader->paramMessage[5], reader->paramMessage[1] * sizeof(uint8_t));
	  data[0] = 0xFF;
	  ret = (TMR_Status) GETU16AT(&reader->paramMessage[0], 3);
	  if (TMR_SUCCESS != ret)
	  {
		  return TMR_ERROR_CODE(ret);
	  }
	  ret = (TMR_Status) GETU16AT(data, 3);
	  if (TMR_SUCCESS != ret)
	  {
		  return TMR_ERROR_CODE(ret);
	  }
  }
  else
  {
    ret = TMR_SR_receiveMessage(reader, data, opcode, timeoutMs);
    if (TMR_SUCCESS != ret)
    {
		return ret;
	}
  }
  return ret;
}

TMR_Status
TMR_SR_callSendCmd(TMR_Reader *reader, uint8_t *msg, uint8_t opcode)
{
  uint8_t i = 2;

  SETU8(msg, i, opcode);
  return TMR_SR_sendCmd(reader, msg, i);
}

TMR_Status
TMR_SR_sendCmd(TMR_Reader *reader, uint8_t *msg, uint8_t i)
{
  msg[1] = i - 3;  /* Install length */
  return TMR_SR_send(reader, msg);
}

TMR_Status
TMR_SR_send(TMR_Reader *reader, uint8_t *data)
{
  return TMR_SR_sendTimeout(reader, data,
                            reader->u.serialReader.commandTimeout);
}

void
TMR_SR_cmdFrameHeader(uint8_t *msg, uint8_t *idx, TMR_SR_OpCode opcode, uint16_t timeout, bool isChipType)
{
  SETU8(msg, *idx, opcode);
  SETU16(msg, *idx, timeout);
  
  if(isChipType)
  {
    SETU8(msg, *idx, 0x00);
  }
#ifdef TMR_ENABLE_UHF
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *idx, TMR_SR_TAGOP_MULTI_SELECT);
  }
#endif /* TMR_ENABLE_UHF */
}

/**
 * Set the operating frequency of the device.
 * Testing command.
 *
 * @param reader The reader
 * @param frequency the frequency to set, in kHz
 */ 

#ifdef TMR_ENABLE_UHF
TMR_Status 
TMR_SR_cmdTestSetFrequency(TMR_Reader *reader, uint32_t frequency)
{
  
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;
  SETU8(msg, i,TMR_SR_OPCODE_SET_OPERATING_FREQ);
  SETU32(msg, i, frequency);
  return TMR_SR_sendCmd(reader, msg, i);
}

/**
 * Turn CW transmission on or off.
 * Testing command.
 *
 * @param reader The reader
 * @param on whether to turn CW on or off
 */ 
TMR_Status 
TMR_SR_cmdTestSendCw(TMR_Reader *reader, bool on)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;
  SETU8(msg, i,TMR_SR_OPCODE_TX_CW_SIGNAL);
  if(on)
    SETU8(msg, i,1);
  else
    SETU8(msg, i,0);
  return TMR_SR_sendCmd(reader, msg, i);
}

/**
 * Turn on pseudo-random bit stream transmission for a particular
 * duration.  
 * Testing command.
 *
 * @param reader The reader
 * @param duration the duration to transmit the PRBS signal.
 */ 
TMR_Status 
TMR_SR_cmdTestSendPrbs(TMR_Reader *reader, uint16_t duration)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;
  SETU8(msg, i,TMR_SR_OPCODE_TX_CW_SIGNAL);
  SETU8(msg, i,2);
  SETU16(msg, i,duration);
  return TMR_SR_sendCmd(reader, msg, i);
}
#endif /* TMR_ENABLE_UHF */
/**
 * Setting user profile on the basis of operation, category and type parameter
 *
 * @param reader The reader
 * @param op operation to be performed on configuration (Save,restore,verify and reset)
 * @param category Which category of configuration to operate on -- only TMR_SR_ALL is currently supported
 * @param type Type of configuration value to use (default, custom...)
 */

TMR_Status 
TMR_SR_cmdSetUserProfile(TMR_Reader *reader,TMR_SR_UserConfigOperation op,TMR_SR_UserConfigCategory category, TMR_SR_UserConfigType type)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  TMR_Status ret;
  TMR_Status ret1;
  uint16_t readTime = 250;
  TMR_SR_SerialReader *sr;
  TMR_SR_SerialTransport *transport;

  sr = &reader->u.serialReader;
  transport = &reader->u.serialReader.transport;

  i = 2;
  SETU8(msg,i,TMR_SR_OPCODE_SET_USER_PROFILE);
  SETU8(msg,i,op);
  SETU8(msg,i,category);
  SETU8(msg,i,type);

#ifdef TMR_ENABLE_BACKGROUND_READS
  readTime = (uint16_t)reader->readParams.asyncOnTime; 
#endif

  if (op == TMR_USERCONFIG_CLEAR)
  {
    /* Resrt the Autonomous option if enabled */
    sr->enableAutonomousRead = false;
  }

  if (op == TMR_USERCONFIG_SAVE_WITH_READPLAN)
  {
    /** 
     * extract the read plan configuration form API read plan object, frame
     * the serial command for read by using existing parser methods in read
     * operation and append that to save configuration command.
     **/
    TMR_ReadPlan *rp;
    uint8_t tempMsg[TMR_SR_MAX_PACKET_SIZE];
    bool value = false;

    TMR_TagProtocolList p;
    TMR_TagProtocolList *protocolList = &p;
    TMR_TagFilter *filters[TMR_MAX_SERIAL_MULTIPROTOCOL_LENGTH];
    TMR_TagProtocol protocols[TMR_MAX_SERIAL_MULTIPROTOCOL_LENGTH];
    TMR_SR_SearchFlag antennas;

    rp = reader->readParams.readPlan;    

    /**
     * Currently Mercury API supports only true continuous read as a part of
     * save configuration with read plan.
     * TODO: Add support for other read types as well if needed in future.
     **/
     reader->continuousReading = true;

    /* Add the Autonomous read option */
    sr->enableAutonomousRead = rp->enableAutonomousRead;
    SETU8(msg,i,rp->enableAutonomousRead);

    ret = TMR_SR_cmdSetReaderConfiguration(reader, TMR_SR_CONFIGURATION_ENABLE_READ_FILTER, &value);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }

    antennas = (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST | TMR_SR_SEARCH_FLAG_TAG_STREAMING);

    if (TMR_READ_PLAN_TYPE_MULTI == rp->type)
    {
      /* The Multi read plan */
      uint8_t i; 

      if (TMR_MAX_SERIAL_MULTIPROTOCOL_LENGTH < rp->u.multi.planCount)
      {
        return TMR_ERROR_TOO_BIG ;
      }
      protocolList->len = rp->u.multi.planCount;
      protocolList->max = rp->u.multi.planCount;
      protocolList->list = protocols;

      for (i = 0; i < rp->u.multi.planCount; i++)
      {
        protocolList->list[i] = rp->u.multi.plans[i]->u.simple.protocol;
        filters[i]= rp->u.multi.plans[i]->u.simple.filter; 
      }

      if ((0 < rp->u.multi.planCount) &&
        (rp->u.multi.plans[0]->type == TMR_READ_PLAN_TYPE_SIMPLE) &&
        (compareAntennas(&rp->u.multi)))
      {        
        ret = prepForSearch(reader, rp);
        if (TMR_SUCCESS != ret)
        {
          return ret;
        }

        /* Call the helper function to frame the multi protocol read command */
        ret = TMR_SR_msgSetupMultipleProtocolSearch(reader, tempMsg, TMR_SR_OPCODE_READ_TAG_ID_MULTIPLE, protocolList,
          reader->userMetadataFlag, antennas, filters, (uint16_t)readTime);
        if (TMR_SUCCESS != ret)
        {
          return ret;
        }
      }
      else
      {
        /**
         * Coming here means the requested read plan is not for true continuous read.
         * Throw error back to user.
         * TODO:Remove this validation, if we need to support for other type of read
         * operation in future.
         */
        return TMR_ERROR_UNSUPPORTED;
      }
    }
    else
    {
      /* The Simple Read Plan */
#ifdef TMR_ENABLE_UHF
      reader->fastSearch = rp->u.simple.useFastSearch;
#endif /* TMR_ENABLE_UHF */
      reader->triggerRead = rp->u.simple.triggerRead.enable;
      protocolList->len = 1;
      protocolList->max = 1;
      protocolList->list = protocols;

      ret = prepForSearch(reader, rp);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      protocolList->list[0] = rp->u.simple.protocol;
      filters[0]= rp->u.simple.filter;

      ret = TMR_SR_msgSetupMultipleProtocolSearch(reader, tempMsg, TMR_SR_OPCODE_READ_TAG_ID_MULTIPLE, protocolList,
        reader->userMetadataFlag, antennas, filters, (uint16_t)readTime);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }      
    }

    /* Append the serial read command to save configuration command */
    memcpy((msg + i), tempMsg+1, tempMsg[1]+2);
    /* Add two bytes for Opcode and header */
    i = (i + tempMsg[1] + 2);    
    reader->continuousReading = false;
  }

  ret1 = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret1)
  {
    return ret1;
  }

  /* If Autonomous read is enabled skip this */
  if (!sr->enableAutonomousRead)
  {
    if ((op == TMR_USERCONFIG_RESTORE)||(op == TMR_USERCONFIG_CLEAR))  //reprobe the baudrate
    {
      uint32_t rate;
      if (reader->connected == false)
      {
        ret = transport->open(transport);
        if (TMR_SUCCESS != ret)
        {
          return ret;
        }
      }

      ret = TMR_SR_cmdProbeBaudRate(reader, &rate);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
      reader->connected = true;
      sr->baudRate = rate;
    }

    /* Restore the region and protocol*/
    if ((op == TMR_USERCONFIG_RESTORE) || (op == TMR_USERCONFIG_CLEAR))
    {
      ret = TMR_SR_cmdGetRegion(reader, &sr->regionId);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      ret = TMR_SR_cmdGetCurrentProtocol(reader, &reader->tagOpParams.protocol);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
      sr->currentProtocol = reader->tagOpParams.protocol;
    }

    if (op == TMR_USERCONFIG_CLEAR)
    {
      TMR_TagProtocol protocol;
      if(TMR_SR_MODEL_M3E != sr->versionInfo.hardware[0])
      {
        protocol = TMR_TAG_PROTOCOL_GEN2;
      }
      else
      {
        protocol = TMR_TAG_PROTOCOL_ISO14443A;
      }

      TMR_RP_init_simple(&reader->readParams.defaultReadPlan, 0, NULL, protocol, 1);

      /* Set read plan to default*/
      ret = TMR_paramSet(reader, TMR_PARAM_READ_PLAN, &reader->readParams.defaultReadPlan);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
    }
  }
  return ret1;
}
#ifdef TMR_ENABLE_UHF
/**
 * Get Save/Restore Configuration 
 *
 * @param reader The reader
 * @param byte array consists of a opcode option(s)
 * @param length Length of byte array
 * @param response Response of the operation
 * @param response_length Length of response array
 */

TMR_Status TMR_SR_cmdGetUserProfile(TMR_Reader *reader, uint8_t byte[], uint8_t length, uint8_t response[], uint8_t* response_length)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i,j;
  i = 2;
  SETU8(msg,i,TMR_SR_OPCODE_GET_USER_PROFILE);
  for(j=0;j<length;j++)
  {
    SETU8(msg,i,byte[j]);
  }

  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  for(j=0;j<msg[1];j++)
  {
    response[j]=msg[5+j];
  }
  *response_length=msg[1];
  return ret;
}

TMR_Status TMR_SR_cmdBlockWrite(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Bank bank, uint32_t wordPtr, 
                                 uint32_t wordCount, const uint16_t* data, uint32_t accessPassword, const TMR_TagFilter* target)
        {   
            TMR_Status ret;
            uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
            uint8_t i, option=0,rec;

            i = 2;
            TMR_SR_cmdFrameHeader(msg, &i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC, timeout, true);
            rec=i;
            SETU8(msg,i,0x40);//option
            SETU8(msg,i,0x00);
            SETU8(msg,i,0xC7);
            ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, &i, msg,accessPassword,true);
            if(TMR_SUCCESS != ret)
            {
              return ret;
            }
            msg[rec]=msg[rec]|option;
            SETU8(msg,i,0x00);
            SETU8(msg,i,bank);
            SETU32(msg,i,wordPtr);
            SETU8(msg,i,(uint8_t)wordCount);
            {
              uint32_t iWord;
              for (iWord = 0; iWord < wordCount; iWord++)
              {
                SETU8(msg, i, ((data[iWord]>>8)&0xFF));
                SETU8(msg, i, ((data[iWord]>>0)&0xFF));
              }
            }
            return TMR_SR_sendCmd(reader, msg, i);
         }

TMR_Status 
TMR_SR_cmdBlockErase(TMR_Reader *reader, uint16_t timeout,
                     TMR_GEN2_Bank bank, uint32_t wordPtr,
                     uint8_t wordCount, uint32_t accessPassword,
                     TMR_TagFilter *target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i = 2;

  ret = TMR_SR_msgAddGEN2BlockErase(msg, &i, timeout, wordPtr, bank, wordCount, accessPassword, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }

  return TMR_SR_sendCmd(reader, msg, i);
}

TMR_Status
TMR_SR_cmdBlockPermaLock(TMR_Reader *reader, uint16_t timeout,uint32_t readLock, TMR_GEN2_Bank bank, uint32_t blockPtr, uint32_t blockRange, uint16_t* mask, uint32_t accessPassword, TMR_TagFilter* target, TMR_uint8List* data)
        { 
            TMR_Status ret;
            uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
            uint8_t i, option=0,rec;
            unsigned int j;

            i = 2;
            TMR_SR_cmdFrameHeader(msg, &i, TMR_SR_OPCODE_ERASE_BLOCK_TAG_SPECIFIC, timeout, true);
            rec=i;
            SETU8(msg,i,0x40);
            SETU8(msg,i,0x01);
            ret = filterbytes(TMR_TAG_PROTOCOL_GEN2,target, &option, &i, msg,accessPassword,true);
            if(TMR_SUCCESS != ret)
            {
              return ret;
            }
            msg[rec]=msg[rec]|option;
            SETU8(msg,i,0x00);
            SETU8(msg,i,(uint8_t)readLock);
            
            SETU8(msg,i,bank);
            SETU32(msg,i,blockPtr);
            SETU8(msg,i,(uint8_t)blockRange);
            
            if (readLock==0x01)
            {
              for(j=0;j<blockRange;j++)
              {
               SETU8(msg,i,(mask[j]>>8)&(0xff));
               SETU8(msg,i,(mask[j]>>0) & (0xff));
              }
            }
            ret =  TMR_SR_sendCmd(reader, msg, i);
            if (TMR_SUCCESS != ret)
            {
              return ret;
            }
            if ((0 == readLock) && (NULL != data))
            {
              data->len = msg[1]-2;
              if (data->len > data->max)
              {
                data->len = data->max;
            }
              memcpy(data->list, msg+7, data->len);
            }
            return ret;
        }  
#endif /* TMR_ENABLE_UHF */
void fillVersionInfo(TMR_SR_VersionInfo *info, uint8_t *msg)
{
  uint8_t i, j;
  i = 5;

  for (j = 0; j < 4 ; j++)
  {
    info->bootloader[j] = GETU8(msg, i);
  }
  for (j = 0; j < 4 ; j++)
  {
    info->hardware[j] = GETU8(msg, i);
  }
  for (j = 0; j < 4 ; j++)
  {
    info->fwDate[j] = GETU8(msg, i);
  }
  for (j = 0; j < 4 ; j++)
  {
    info->fwVersion[j] = GETU8(msg, i);
  }
  info->protocols = GETU32(msg, i);
}

TMR_Status
TMR_SR_cmdVersion(TMR_Reader *reader, TMR_SR_VersionInfo *info)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i = 2;

  SETU8(msg, i, TMR_SR_OPCODE_VERSION);
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, 0);
  if (TMR_SUCCESS != ret)
  {
    return ret;    
  }
  if (NULL != info)
  {
    fillVersionInfo(info, msg);
  }
  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_cmdBootFirmware(TMR_Reader *reader)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_BOOT_FIRMWARE);
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, 1000);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  fillVersionInfo(&reader->u.serialReader.versionInfo, msg);
  return TMR_SUCCESS;
}


TMR_Status
TMR_SR_cmdSetBaudRate(TMR_Reader *reader, uint32_t rate)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_BAUD_RATE);
  SETU32(msg, i, rate);

  return TMR_SR_sendCmd(reader, msg, i);
}


TMR_Status
TMR_SR_cmdEraseFlash(TMR_Reader *reader, uint8_t sector, uint32_t password)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_ERASE_FLASH);
  SETU32(msg, i, password);
  SETU8(msg, i, sector);
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, 30000);
}


TMR_Status
TMR_SR_cmdWriteFlashSector(TMR_Reader *reader, uint8_t sector, uint32_t address,
                           uint32_t password, uint8_t length, const uint8_t data[],
                           uint32_t offset)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_WRITE_FLASH_SECTOR);
  SETU32(msg, i, password);
  SETU32(msg, i, address);
  SETU8(msg, i, sector);
  if(NULL != data)
  {
    memcpy(&msg[i], data + offset, length);
    i += length;
  }
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, 3000);
}


#ifdef TMR_ENABLE_UHF
TMR_Status
TMR_SR_cmdModifyFlashSector(TMR_Reader *reader, uint8_t sector, uint32_t address,
                            uint32_t password, uint8_t length, const uint8_t data[],
                            uint32_t offset)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_MODIFY_FLASH_SECTOR);
  SETU32(msg, i, password);
  SETU32(msg, i, address);
  SETU8(msg, i, sector);
  memcpy(&msg[i], data + offset, length);
  i += length;
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, 3000);
}
#endif /* TMR_ENABLE_UHF */
TMR_Status
TMR_SR_cmdBootBootloader(TMR_Reader *reader)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];

#ifdef TMR_ENABLE_UHF
  reader->u.serialReader.crcEnabled = true;
#endif /* TMR_ENABLE_UHF */
  return TMR_SR_callSendCmd(reader, msg, TMR_SR_OPCODE_BOOT_BOOTLOADER);
}


TMR_Status
TMR_SR_cmdGetHardwareVersion(TMR_Reader *reader, uint8_t option, uint8_t flags,
                             uint8_t* count, uint8_t data[])
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_HW_VERSION);
  SETU8(msg, i, option);
  SETU8(msg, i, flags);

  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
  {
    *count = 0;
    return ret;
  }

  for (i = 0 ; i < msg[1] && i < *count; i++)
  {
    data[i] = msg[5 + i];
  }

  *count = msg[1];

  return TMR_SUCCESS;
}


TMR_Status
TMR_SR_cmdGetCurrentProgram(TMR_Reader *reader, uint8_t *program)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];

  ret = TMR_SR_callSendCmd(reader, msg, TMR_SR_OPCODE_GET_CURRENT_PROGRAM);
  *program = msg[5];

  return ret;
}
#if !((!defined(TMR_ENABLE_UHF)) || defined(BARE_METAL))
TMR_Status TMR_SR_msgSetupReadTagSingle(uint8_t *msg, uint8_t *i, TMR_TagProtocol protocol,TMR_TRD_MetadataFlag metadataFlags, const TMR_TagFilter *filter,uint16_t timeout)
{
  uint8_t optbyte;

  SETU8(msg, *i, TMR_SR_OPCODE_READ_TAG_ID_SINGLE);
  SETU16(msg, *i, timeout);
  optbyte = *i;
  SETU8(msg, *i, 0); /* Initialize option byte */
  msg[optbyte] |= TMR_SR_GEN2_SINGULATION_OPTION_FLAG_METADATA;
  SETU16(msg,*i, metadataFlags);
  filterbytes(protocol, filter, &msg[optbyte], i, msg,0, true);
  msg[optbyte] |= TMR_SR_GEN2_SINGULATION_OPTION_FLAG_METADATA;
  return TMR_SUCCESS;

}
#endif /* TMR_ENABLE_HF_LF || BARE_METAL */
TMR_Status
TMR_SR_msgSetupReadTagMultiple(TMR_Reader *reader, uint8_t *msg, uint8_t *i, uint16_t timeout,
                               TMR_SR_SearchFlag searchFlag,
                               const TMR_TagFilter *filter,
                               TMR_TagProtocol protocol,
                               TMR_GEN2_Password accessPassword)
{
  return TMR_SR_msgSetupReadTagMultipleWithMetadata(reader, msg, i, timeout,
    searchFlag, reader->userMetadataFlag, filter, protocol,accessPassword);
}


TMR_Status
TMR_SR_msgSetupReadTagMultipleWithMetadata(TMR_Reader *reader, uint8_t *msg, uint8_t *i, uint16_t timeout,
                               TMR_SR_SearchFlag searchFlag,
							                 TMR_TRD_MetadataFlag metadataFlag,
                               const TMR_TagFilter *filter,
                               TMR_TagProtocol protocol,
                               TMR_GEN2_Password accessPassword)
{
  TMR_Status ret;
  TMR_SR_SerialReader *sr;
  uint8_t optbyte;
#ifdef TMR_ENABLE_UHF
  uint8_t SingulationOption = 0;
  uint8_t planCount = 0;
#endif /* TMR_ENABLE_UHF */
  sr = &reader->u.serialReader;
  sr->opCode = TMR_SR_OPCODE_READ_TAG_ID_MULTIPLE;
  ret = TMR_SUCCESS;

#ifdef TMR_ENABLE_UHF
  if(reader->featureFlags & TMR_READER_FEATURES_FLAG_MULTI_SELECT)
  {
    isMultiSelectEnabled = ((filter) && (filter->type == TMR_FILTER_TYPE_TAG_DATA)) ? false : true;
  }
#endif /* TMR_ENABLE_UHF */

  SETU8(msg, *i, TMR_SR_OPCODE_READ_TAG_ID_MULTIPLE);

#ifdef TMR_ENABLE_UHF
  /**
   * we need to add the option for bap parameters if enabled
   * 
   * for adding the bap parameter option, and EBV technique is used
   * raising the lowest order bit of the high opder byte signals the
   * use of new Gen2 Bap support parameters.
   **/
  if (sr->isBapEnabled)
  {
    SingulationOption = TMR_SR_TAGOP_BAP;
  }

  /**
    * In Embedded ReadAfterWrite operation, option field should be preceded by 0x84.
    * Adding the field in msg.
    **/
  if (reader->isReadAfterWrite)
  {
    SingulationOption |= TMR_SR_TAGOP_READ_AFTER_WRITE;
  }

  /* To perform Multiple select operation, option byte should be preceded by 0x88.*/
  if(isMultiSelectEnabled)
  {
    SingulationOption |= TMR_SR_TAGOP_MULTI_SELECT;
    /* Enable isEmbeddedTagopEnabled flag to indicate that 0x88 option has already been added. */
    if((reader->readParams.readPlan->type == TMR_READ_PLAN_TYPE_SIMPLE) && (reader->readParams.readPlan->u.simple.tagop != NULL))
    {
      isEmbeddedTagopEnabled = true;
    }
    else if(reader->readParams.readPlan->type == TMR_READ_PLAN_TYPE_MULTI)
    {
      for(planCount = 0; planCount < reader->readParams.readPlan->u.multi.planCount; planCount++)
      {
        if(reader->readParams.readPlan->u.multi.plans[planCount]->u.simple.tagop != NULL)
        {
          isEmbeddedTagopEnabled = true;
        }
      }
    }
  }

  if(SingulationOption)
  {
    SETU8(msg, *i, SingulationOption);
  }
#endif /* TMR_ENABLE_UHF */
  optbyte = *i;
  SETU8(msg, *i, 0); /* Initialize option byte */

  /* add the large tag population support */
  searchFlag = (TMR_SR_SearchFlag)(searchFlag 
        | TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT);

  if (reader->continuousReading)
  {
    searchFlag = (TMR_SR_SearchFlag)(searchFlag
        | TMR_SR_SEARCH_FLAG_TAG_STREAMING);

    /**
     * We need to send a different flag for the new reader stats,
     * Both these options are mutually exclusive in nature,
     * Hence can not be raised at once.
     **/
#ifdef TMR_ENABLE_UHF
    if (TMR_SR_STATUS_NONE != reader->streamStats)
    {
      searchFlag |= TMR_SR_SEARCH_FLAG_STATUS_REPORT_STREAMING;
    }
    else
#endif /* TMR_ENABLE_UHF */
    {
      if (TMR_READER_STATS_FLAG_NONE != reader->statsFlag)
      {
        searchFlag |= TMR_SR_SEARCH_FLAG_STATS_REPORT_STREAMING;
      }
    }
  }

  /**
   * Add the fast search flag depending on the user choice
   */
#ifdef TMR_ENABLE_UHF
  if (reader->fastSearch)
  {
    searchFlag = (TMR_SR_SearchFlag)(searchFlag
      |TMR_SR_SEARCH_FLAG_READ_MULTIPLE_FAST_SEARCH);
    reader->fastSearch = false;
  }
#endif /* TMR_ENABLE_UHF */
  /**
   * Add the trigger read flag depending on the user choice
   */
  if (reader->triggerRead)
  {
    searchFlag = (TMR_SR_SearchFlag)(searchFlag
      |TMR_SR_SEARCH_FLAG_GPI_TRIGGER_READ);
    reader->triggerRead = false;
  }

  /**
   * Add the duty cycle flag depending on the user choice
   */
  if (reader->dutyCycle)
  {
    searchFlag = (TMR_SR_SearchFlag)(searchFlag
        |TMR_SR_SEARCH_FLAG_DUTY_CYCLE_CONTROL);
  }
  if (reader->isStopNTags)
  {
    searchFlag = (TMR_SR_SearchFlag)(searchFlag
        |TMR_SR_SEARCH_FLAG_RETURN_ON_N_TAGS);
  }
  SETU16(msg, *i, searchFlag);
  SETU16(msg, *i, timeout);

  if(reader->dutyCycle)
  {
    if(TMR_READ_PLAN_TYPE_MULTI == reader->readParams.readPlan->type)
    {
      SETU16(msg, *i, (uint16_t)reader->subOffTime);
    }
    else
    {
	  uint32_t offtime;
	  ret = TMR_paramGet(reader,TMR_PARAM_READ_ASYNCOFFTIME,&offtime);
	  SETU16(msg, *i, (uint16_t)offtime);
    }
  }
  SETU16(msg, *i, metadataFlag);

  /**
   * Add the no of tags to be read requested by user
   * in stop N tag reads.
   **/
  if (reader->isStopNTags)
  {
    SETU32(msg , *i, (uint32_t)reader->numberOfTagsToRead);
  }

  if (reader->continuousReading)
  {
    if (TMR_READER_STATS_FLAG_NONE != reader->statsFlag)
    {
      /**
       * To extend the flag byte, an EBV technique is to be used.
       * When the highest order bit of the flag byte is used,
       * it signals the reader’s parser, that another flag byte is to follow
       */
        SETU16(msg, *i, (uint16_t)reader->statsFlag);
    }
#ifdef TMR_ENABLE_UHF
    else
    {
      if (TMR_SR_STATUS_NONE != reader->streamStats)
      {
        /* Add status report flags, so that the status stream responses are received */
        SETU16(msg, *i, (uint16_t)reader->streamStats);
      }
    }
#endif /* TMR_ENABLE_UHF */
  }

#ifdef TMR_ENABLE_ISO180006B
  /*
   * Earlier, this filter bytes were skipped for a null filter and gen2 0 access password.
   * as the filterbytes it self has the checks internally, these were removed.
   * for some protocols (such as ISO180006B) the "null" filter is not zero-length, but we don't need to send
   * that with this command.
   */
  if (TMR_TAG_PROTOCOL_ISO180006B == protocol && NULL == filter)
  {
    /* ISO18000-6B should not include any filter arg bytes if null */
  }
  else
#endif /* TMR_ENABLE_ISO180006B */
  {
    ret = filterbytes(protocol, filter, &msg[optbyte], i, msg,
        accessPassword, true);
  }

  msg[optbyte] |= TMR_SR_GEN2_SINGULATION_OPTION_FLAG_METADATA;

#ifdef TMR_ENABLE_UHF
  if (isSecureAccessEnabled)
  {
    msg[optbyte] |= TMR_SR_GEN2_SINGULATION_OPTION_SECURE_READ_DATA;
  }
#endif /* TMR_ENABLE_UHF */
  return ret;
}

TMR_Status
TMR_SR_cmdReadTagMultiple(TMR_Reader *reader, uint16_t timeout,
                          TMR_SR_SearchFlag searchFlag,
                          const TMR_TagFilter *filter, TMR_TagProtocol protocol,
                          uint32_t *tagCount)
{
  TMR_Status ret;
  TMR_SR_SerialReader *sr;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  sr = &reader->u.serialReader;
  i = 2;

  ret = TMR_SR_msgSetupReadTagMultiple(reader, msg, &i, timeout, searchFlag,
                                       filter, protocol, 0);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  sr->opCode = TMR_SR_OPCODE_READ_TAG_ID_MULTIPLE;
  if (reader->continuousReading)
  {
    uint8_t opcode;
    ret = TMR_SR_sendMessage(reader, msg, &opcode, timeout);
    *tagCount = (uint32_t)0;
    return ret;
  }
  else
  {
    ret = TMR_SR_sendTimeout(reader, msg, timeout);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }

    if (NULL != tagCount)
    {
      if (4 == msg[1])
      {
        /* Plain 1-byte count: Reader without large-tag-population support */
        *tagCount = GETU8AT(msg, 8);
      }
      else if (5 == msg[1])
      {
        /* Later 1-byte count: ISO18k select option included in reply */
        *tagCount = GETU8AT(msg, 9);
      }
      else if (7 == msg[1])
      {
        /* Plain 4-byte count: Reader with large-tag-population support */
        *tagCount = GETU32AT(msg, 8);
      }
      else if (8 == msg[1])
      {
        /* Later 4-byte count: Large-tag-population support and ISO18k
         * select option included in reply.
         */
        *tagCount = GETU32AT(msg, 9);
      }
      else
      {
        return TMR_ERROR_PARSE;
      }
    }
    return TMR_SUCCESS;
  }
}

TMR_Status
TMR_SR_executeEmbeddedRead(TMR_Reader *reader, uint8_t *msg, uint16_t timeout,
                           TMR_SR_MultipleStatus *status)
{
  TMR_Status ret;
  uint8_t i, len, index;
  uint8_t newMsg[TMR_SR_MAX_PACKET_SIZE];
  uint16_t searchFlags = 0;
    
  if (reader->continuousReading)
  {
    uint8_t opcode = TMR_SR_OPCODE_MULTI_PROTOCOL_TAG_OP;
    i = 2;
    SETU8(newMsg, i, opcode);  /* Opcode */
    
    /* Timeout should be zero for true continuous reading */
    SETU16(newMsg, i, 0);
    SETU8(newMsg, i, (uint8_t)0x1); /* TM Option 1, for continuous reading */
    SETU8(newMsg, i, (uint8_t)TMR_SR_OPCODE_READ_TAG_ID_MULTIPLE); /* sub command opcode */

#ifdef TMR_ENABLE_HF_LF
    if (reader->isProtocolDynamicSwitching)
    {
      searchFlags |= (uint16_t)(TMR_SR_SEARCH_FLAG_DYNAMIC_PROTOCOL_SWITCHING);
    }
#endif /* TMR_ENABLE_HF_LF */

    if (reader->isStopNTags)
    {
      /**
      * True means atleast one sub read plan has the requested for stop N trigger.
      */
      searchFlags |= (uint16_t)(TMR_SR_SEARCH_FLAG_RETURN_ON_N_TAGS);
    }

    //Add search flags.
    SETU16(newMsg, i, (uint16_t)searchFlags);

    if(reader->isStopNTags)
    {
      //Add the total tag count for stop N trigger.
      SETU32(newMsg, i, reader->numberOfTagsToRead);
    }

    if (TMR_SR_MODEL_M3E != reader->u.serialReader.versionInfo.hardware[0])
	{
      SETU8(newMsg, i, (uint8_t)TMR_TAG_PROTOCOL_GEN2); /* protocol ID */
	}
	else
	{
	  SETU8(newMsg, i, (uint8_t)reader->u.serialReader.currentProtocol); /* protocol ID */
	}

    len = msg[1];
    index = i;
    SETU8(newMsg, i, 0); /* Protocol command length (initialize to 0)*/

    /* Copy the protocol command including the command opcode (len + 1)*/
    memcpy(&newMsg[i], &msg[2], (size_t)(len + 1));
    i += len + 1;

    /* Insert the exact protocol command length */
    newMsg[index] = i - index - 2;

    /* Install the total packet length*/
    newMsg[1]=i - 3;    

    ret = TMR_SR_sendMessage(reader, newMsg, &opcode, timeout);
    status->tagsFound = status->successCount = status->failureCount = 0;

    return ret;
  }
  else
  {
#ifdef TMR_ENABLE_UHF
    uint16_t searchFlags;

    if ((reader->isReadAfterWrite)|| (isMultiSelectEnabled))
    {
      searchFlags = GETU16AT(msg, 5);
    }
    else
    {
      searchFlags = GETU16AT(msg, 4);
    }
#endif /* TMR_ENABLE_UHF */

    ret = TMR_SR_sendTimeout(reader, msg, timeout);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }

    if (NULL != status)
    {
      int readIdx = 9;

#ifdef TMR_ENABLE_UHF
      (((reader -> isReadAfterWrite) || (isMultiSelectEnabled)) ? (readIdx = 9) : (readIdx = 8));

      if (TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT & searchFlags)
	    {
        /* Only in case of Large Tag Population, the tag length will be 4 bytes */
		    status->tagsFound = (uint16_t)GETU32AT(msg, readIdx);
		    readIdx += 4;
        }
	    else
#endif /* TMR_ENABLE_UHF */
	    {
		    status->tagsFound = GETU8AT(msg, readIdx);
		    readIdx += 1;
	    }
      readIdx += 2;
      status->successCount = GETU16AT(msg, readIdx);
      readIdx += 2;
      status->failureCount = GETU16AT(msg, readIdx);
      readIdx += 2;
    }
  }

  return TMR_SUCCESS;
}

#ifdef TMR_ENABLE_UHF
void
TMR_SR_msgAddGEN2WriteTagEPC(uint8_t *msg, uint8_t *i, uint16_t timeout, uint8_t *epc, uint8_t count)
{
  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_ID);
  SETU16(msg, *i, timeout); /* timeout */
  SETU16(msg, *i, 0); /* RFU 2 bytes */
  memcpy(&msg[*i], epc, count);
  *i += count;
}

void
TMR_SR_msgAddGEN2DataRead(uint8_t *msg, uint8_t *i, uint16_t timeout,
                      TMR_GEN2_Bank bank, uint32_t wordAddress, uint8_t len, uint8_t option, bool withMetaData)
{
  TMR_SR_cmdFrameHeader(msg, i, TMR_SR_OPCODE_READ_TAG_DATA, timeout, false);
 
  SETU8(msg, *i, option);  /* Options - initialize */
  if (withMetaData)
  {
    SETU16(msg, *i, 0x0000);  /* metadata flags - initialize */
  }
  SETU8(msg, *i, bank);
  SETU32(msg, *i, wordAddress);
  SETU8(msg, *i, len);
}


void
TMR_SR_msgAddGEN2DataWrite(uint8_t *msg, uint8_t *i, uint16_t timeout,
                       TMR_GEN2_Bank bank, uint32_t address, bool isReadAfterWrite)
{
  uint8_t SingulationOption = 0;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_DATA);
  SETU16(msg, *i, timeout);
  if(isReadAfterWrite)
  {
    SingulationOption = TMR_SR_TAGOP_READ_AFTER_WRITE; /* Option byte for Read after WriteData*/
  }
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SingulationOption |= TMR_SR_TAGOP_MULTI_SELECT;
  }
  if(SingulationOption)
  {
    SETU8(msg, *i, SingulationOption);
  }
  SETU8(msg, *i, 0); /* Option - initialize */
  SETU32(msg, *i, address);
  SETU8(msg, *i, bank);
}


void
TMR_SR_msgAddGEN2LockTag(uint8_t *msg, uint8_t *i, uint16_t timeout, uint16_t mask,
                         uint16_t action, TMR_GEN2_Password accessPassword)
{
  TMR_SR_cmdFrameHeader(msg, i, TMR_SR_OPCODE_LOCK_TAG, timeout, false);

  SETU8(msg, *i, 0);  /* Option - initialize */
  SETU32(msg, *i, accessPassword);
  SETU16(msg, *i, mask);
  SETU16(msg, *i, action);
}


void
TMR_SR_msgAddGEN2KillTag(uint8_t *msg, uint8_t *i, uint16_t timeout,
                         TMR_GEN2_Password password)
{
  TMR_SR_cmdFrameHeader(msg, i, TMR_SR_OPCODE_KILL_TAG, timeout, false);

  SETU8(msg, *i, 0);  /* Option - initialize */
  SETU32(msg, *i, password);
  SETU8(msg, *i, 0);  /* RFU */
}

TMR_Status
TMR_SR_msgAddGEN2BlockWrite(uint8_t *msg, uint8_t *i, uint16_t timeout,TMR_GEN2_Bank bank, uint32_t wordPtr, uint32_t wordCount, uint16_t* data, uint32_t accessPassword, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option=0,rec;      
  SETU8(msg,*i,TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i,timeout);
  SETU8(msg,*i,0x00);//chip type
  rec=*i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg,*i,0x00);
  SETU8(msg,*i,0xC7);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg,accessPassword,true);
  msg[rec]=msg[rec]|option;
  SETU8(msg,*i,0x00);
  SETU8(msg,*i,bank);
  SETU32(msg,*i,wordPtr);
  SETU8(msg,*i,(uint8_t)wordCount);
  {
    uint32_t iWord;
    for (iWord=0; iWord<wordCount; iWord++)
    {
      SETU8(msg, *i, ((data[iWord]>>8)&0xFF));
      SETU8(msg, *i, ((data[iWord]>>0)&0xFF));
    }
  }
  return ret;
}

TMR_Status
TMR_SR_msgAddGEN2BlockPermaLock(uint8_t *msg, uint8_t *i, uint16_t timeout, uint32_t readLock, TMR_GEN2_Bank bank, uint32_t blockPtr, uint32_t blockRange, uint16_t* mask, uint32_t accessPassword,TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option=0,rec;    
  SETU8(msg,*i,TMR_SR_OPCODE_ERASE_BLOCK_TAG_SPECIFIC);
  SETU16(msg,*i,timeout);
  SETU8(msg,*i,0x00);
  rec=*i;
  SETU8(msg,*i,0x40);
  SETU8(msg,*i,0x01);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2,target, &option, i, msg,accessPassword,true);
  msg[rec]=msg[rec]|option;
  SETU8(msg,*i,0x00);
  SETU8(msg,*i,(uint8_t)readLock);

  SETU8(msg,*i,bank);
  SETU32(msg,*i,blockPtr);
  SETU8(msg,*i,(uint8_t)blockRange);
  if (readLock==0x01)
  {
   SETU16(msg, *i, *mask);
  }
  return ret;
}

TMR_Status
TMR_SR_msgAddGEN2BlockErase(uint8_t *msg, uint8_t *i, uint16_t timeout,
                            uint32_t wordPtr, TMR_GEN2_Bank bank, uint8_t wordCount,
                            uint32_t accessPassword, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  TMR_SR_cmdFrameHeader(msg, i, TMR_SR_OPCODE_ERASE_BLOCK_TAG_SPECIFIC, timeout, true);

  rec = *i;
  SETU8(msg, *i, 0x40);
  SETU8(msg, *i, 0x00); /* Block erase */
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec] = msg[rec] | option;

  SETU32(msg, *i, wordPtr);
  SETU8(msg, *i, bank);
  SETU8(msg,*i, wordCount);

  return ret;
}

TMR_Status
TMR_SR_cmdWriteGen2TagEpc(TMR_Reader *reader, const TMR_TagFilter *filter, TMR_GEN2_Password accessPassword, 
					  uint16_t timeout, uint8_t count, const uint8_t *id, bool lock)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i, optbyte;

  i = 2;
  TMR_SR_cmdFrameHeader(msg, &i, TMR_SR_OPCODE_WRITE_TAG_ID, timeout, false);

  optbyte = i;
  SETU8(msg, i, 0);
  

  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, filter, &msg[optbyte], &i, msg,
                    accessPassword, true);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  if (0 == msg[optbyte])
  {
	  SETU8(msg, i, 0);  // Initialize second RFU byte to zero
  }

  if (i + count + 1 > TMR_SR_MAX_PACKET_SIZE)
  {
    return TMR_ERROR_TOO_BIG;
  }

  memcpy(&msg[i], id, count);
  i += count;
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}

/**
 * Read After Write data from a Gen2 tag.
 *
 * @param reader: the reader pointer which points to reader object
 * @param filter: a specification of the air protocol filtering to
 *                perform to find the tag
 * @param accessPassword: the password to use when writing the tag
 * @param timeout: the duration in milliseconds to search for
 *                 a tag to write to. Valid range is 0-65535
 * @param count: the number of words to write
 * @param :id: pointer to the data to write
 * @param :lock: lock flag
 * @param readBank: the Gen2 memory bank to read from
 * @param readAddress: the word address to start reading from
 * @param readLen: the number of words to read
 * @param read: pointes to TMR_TagReadData
 */
TMR_Status
TMR_SR_cmdReadAfterWriteGen2TagEpc(TMR_Reader *reader, const TMR_TagFilter *filter, TMR_GEN2_Password accessPassword, 
                                   uint16_t timeout, uint8_t count, const uint8_t *id, bool lock, uint32_t readBank,
                                   uint32_t readAddress, uint8_t readLen, TMR_TagReadData *read)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i, optbyte;
  uint8_t SingulationOption = 0;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_WRITE_TAG_ID);
  SETU16(msg, i, timeout);
  SingulationOption = TMR_SR_TAGOP_READ_AFTER_WRITE; /* Option byte for Read after WriteData*/
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SingulationOption |= TMR_SR_TAGOP_MULTI_SELECT;
  }
  if(SingulationOption)
  {
    SETU8(msg, i, SingulationOption);
  }
  optbyte = i;
  SETU8(msg, i, 0);

  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, filter, &msg[optbyte], &i, msg,
  accessPassword, true);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  if (i + count + 1 > TMR_SR_MAX_PACKET_SIZE)
  {
    return TMR_ERROR_TOO_BIG;
  }

  memcpy(&msg[i], id, count);
  i += count;

  SETU8(msg, i, readBank);
  SETU32(msg, i, readAddress);
  SETU8(msg, i, readLen);

  msg[1] = i - 3; /* Install length */
  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  if (NULL != read->data.list)
  {
    i = 7;

    /* Read Tag Data doesn't put actual tag data inside the metadata fields.
    * Read the actual data here (remainder of response.) */
    {
      uint16_t dataLength;
      uint16_t copyLength;

      copyLength = dataLength = msg[1] + 5 - i;
      if (copyLength >  
      read->data.max)
      {
        copyLength = read->data.max;
      }
      read->data.len = copyLength;
      memcpy(read->data.list, &msg[i], copyLength);
      i += dataLength;
    }
  }
  return TMR_SUCCESS;
}
#endif /* TMR_ENABLE_UHF */

TMR_Status
TMR_SR_cmdClearTagBuffer(TMR_Reader *reader)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];

  return TMR_SR_callSendCmd(reader, msg, TMR_SR_OPCODE_CLEAR_TAG_ID_BUFFER);
}

TMR_Status TMR_SR_cmdGetTagsRemaining(TMR_Reader *reader, uint16_t *tagCount)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  TMR_Status ret;

  ret = TMR_SR_callSendCmd(reader, msg, TMR_SR_OPCODE_GET_TAG_ID_BUFFER);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  
  *tagCount = GETU16AT(msg, 7) - GETU16AT(msg, 5);
  return TMR_SUCCESS;
}

void
TMR_SR_parseMetadataFromMessage(TMR_Reader *reader, TMR_TagReadData *read, uint16_t flags,
                                uint8_t *i, uint8_t msg[])
{
  int msgEpcLen;
  TMR_SR_parseMetadataOnly(reader, read, flags, i, msg);
  msgEpcLen = tm_u8s_per_bits(GETU16(msg, *i)); 

#ifdef TMR_ENABLE_UHF
  if (TMR_SR_MODEL_M3E != reader->u.serialReader.versionInfo.hardware[0])
  {
    if (TMR_TAG_PROTOCOL_ATA != read->tag.protocol)
    {
      /* ATA protocol does not have TAG CRC */
      if (msgEpcLen >= 2)
      {
        msgEpcLen -= 2; /* Remove 2 bytes CRC*/
      }
    }
    if (TMR_TAG_PROTOCOL_GEN2 == read->tag.protocol)
    {
      read->tag.u.gen2.pc[0] = GETU8(msg, *i);
      read->tag.u.gen2.pc[1] = GETU8(msg, *i);
      if (msgEpcLen >= 2)
      {
        msgEpcLen -= 2;
      }
      read->tag.u.gen2.pcByteCount = 2;

      /* Add support for XPC bits
       * XPC_W1 is present, when the 6th most significant bit of PC word is set
       */
      if ((read->tag.u.gen2.pc[0] & 0x02) == 0x02)
      {
        /* When this bit is set, the XPC_W1 word will follow the PC word
         * Our TMR_Gen2_TagData::pc has enough space, so copying to the same.
         */
        read->tag.u.gen2.pc[2] = GETU8(msg, *i);
        read->tag.u.gen2.pc[3] = GETU8(msg, *i);
        msgEpcLen -= 2;                           /* EPC length will be length - 4(PC + XPC_W1)*/
        read->tag.u.gen2.pcByteCount += 2;        /* PC bytes are now 4*/

        if ((read->tag.u.gen2.pc[2] & 0x80) == 0x80)
        {
          /*
           * If the most siginificant bit of XPC_W1 is set, then there exists
           * XPC_W2. A total of 6  (PC + XPC_W1 + XPC_W2 bytes)
           */
           read->tag.u.gen2.pc[4] = GETU8(msg, *i);
           read->tag.u.gen2.pc[5] = GETU8(msg, *i);
           msgEpcLen -= 2;                       /* EPC length will be length - 6 (PC + XPC_W1 + XPC_W2)*/
           read->tag.u.gen2.pcByteCount += 2;    /* PC bytes are now 6 */
        }
      }    
    }
  }
#endif /* TMR_ENABLE_UHF */

  read->tag.epcByteCount = msgEpcLen;

#ifdef TMR_ENABLE_UHF
  if (flags & TMR_TRD_METADATA_FLAG_BRAND_IDENTIFIER)
  {
    read->tag.epcByteCount -= 2;
  }
#endif /* TMR_ENABLE_UHF */

  if (read->tag.epcByteCount > TMR_MAX_EPC_BYTE_COUNT)
  {
    read->tag.epcByteCount = TMR_MAX_EPC_BYTE_COUNT;
  }

  memcpy(read->tag.epc, &msg[*i], read->tag.epcByteCount);
  *i += msgEpcLen;

#ifdef TMR_ENABLE_UHF
  if (flags & TMR_TRD_METADATA_FLAG_BRAND_IDENTIFIER)
  {
    memcpy(read->tag.xepc, &(msg[(*i)-2]), 2);
  }

  if (TMR_SR_MODEL_M3E != reader->u.serialReader.versionInfo.hardware[0])
  {
    if (TMR_TAG_PROTOCOL_ATA != read->tag.protocol)
    {
      read->tag.crc = GETU16(msg, *i);
    }
    else
    {
      read->tag.crc = 0xffff;
    }
  }
#endif /* TMR_ENABLE_UHF */
}
#ifdef TMR_ENABLE_UHF
void
extractGen2MemoryBankValues(TMR_TagReadData *read)
{
  uint16_t dataLength = read->data.len;
  uint8_t readOffSet = 0;
  uint8_t bank;
  uint16_t epcDataLength;
  TMR_uint8List *memData;

  while (dataLength != 0)
  {
    if (readOffSet >= dataLength)
    break;
    bank = ((read->data.list[readOffSet] >> 4) & 0x1F);
    epcDataLength = (read->data.list[readOffSet + 1] * 2);
    switch (bank)
    {
      case TMR_GEN2_BANK_EPC:
        {
          memData = &(read->epcMemData);
          break;
        }
      case TMR_GEN2_BANK_RESERVED:
        {
          memData = &(read->reservedMemData);
          break;
        }
      case TMR_GEN2_BANK_TID:
        {
          memData = &(read->tidMemData);
          break;
        }
      case TMR_GEN2_BANK_USER:
        {
          memData = &(read->userMemData);
          break;
        }
      default:
          break;
    }

    memData->len = epcDataLength;
    if (epcDataLength > memData->max)
    {
      epcDataLength = memData->max;
    }
    if (NULL != memData->list)
    {
      memcpy(memData->list, (read->data.list + readOffSet + 2), epcDataLength);
    }
    readOffSet += (epcDataLength + 2);
  }
}
#endif /* TMR_ENABLE_UHF */
void
TMR_SR_parseMetadataOnly(TMR_Reader *reader, TMR_TagReadData *read, uint16_t flags,
                                uint8_t *i, uint8_t msg[])
{
  read->metadataFlags = flags;
#ifdef TMR_ENABLE_UHF
  read->gpioCount = 0;
#endif /* TMR_ENABLE_UHF */
  read->antenna = 0;
  read->dspMicros = 0;

  /* Fill in tag data from response */
  if (flags & TMR_TRD_METADATA_FLAG_READCOUNT)
  {
    read->readCount = GETU8(msg, *i);
  }
#ifdef TMR_ENABLE_UHF
  if (flags & TMR_TRD_METADATA_FLAG_RSSI)
  {
    read->rssi = (int8_t)GETU8(msg, *i);
  }
#endif /* TMR_ENABLE_UHF */
  if (flags & TMR_TRD_METADATA_FLAG_ANTENNAID)
  {
    read->antenna = GETU8(msg, *i);
  }
#ifdef TMR_ENABLE_UHF
  if (flags & TMR_TRD_METADATA_FLAG_FREQUENCY)
  {
    read->frequency = GETU24(msg, *i);
  }
#endif /* TMR_ENABLE_UHF */
  if (flags & TMR_TRD_METADATA_FLAG_TIMESTAMP)
  {
    read->dspMicros = GETU32(msg, *i);
  }
#ifdef TMR_ENABLE_UHF
  if (flags & TMR_TRD_METADATA_FLAG_PHASE)
  {
    read->phase = GETU16(msg, *i);
  }
#endif /* TMR_ENABLE_UHF */
  if (flags & TMR_TRD_METADATA_FLAG_PROTOCOL)
  {
    read->tag.protocol = (TMR_TagProtocol)GETU8(msg, *i);
  }
  if (flags & TMR_TRD_METADATA_FLAG_DATA)
  {
    int msgDataLen;
    if (reader->continuousReading)
    {
      reader->u.serialReader.tagopSuccessCount = 1;
    }

    msgDataLen = GETU16(msg, *i);

    //Store data length in bits only.
    read->data.len = msgDataLen;

    //Convert data bit length into data byte length.
     msgDataLen = tm_u8s_per_bits(msgDataLen);

    if(TMR_SR_MODEL_M3E != reader->u.serialReader.versionInfo.hardware[0])
    {
      //In UHF data length is stored in bytes.
      read->data.len = msgDataLen;
    }

    /**
     * msgDataLen will be 0x1000(8000 / 8), when embedded tagOp fails.
     * 0x8000 indicates the error, it is not the data length.
     * Hence, no need to update the msgDataLen in this case!
     */
    if ((msgDataLen > read->data.max) && (msgDataLen != 0x1000))
    {
      msgDataLen = read->data.max;
    }

    if (NULL != read->data.list)
    {
#ifdef TMR_ENABLE_HF_LF
      if (msgDataLen & 0x1000)
      {
        /**
         * 0x8000 in msgDataLen field indicates an error when embedded tagOp fails.
         * msgDataLen field indicates data length when embedded tagOp doesn’t fail. It is usually indicated in bits.
         * So when embedded tagOp fails, it is equal to 0x1000 bytes (i.e., 0x8000 / 8) and it shouldn’t be
         * considered as valid data length!
         */
        read->data.len = 0x8000;

        /**
         * If embedded tagOp fails, the data will contain error code of 2 bytes.
         * Hence, making length 2.
         */
        msgDataLen = 2;
      }
#endif /* TMR_ENABLE_HF_LF */

      memcpy(read->data.list, &msg[*i], msgDataLen);
    }
#ifdef TMR_ENABLE_UHF
    /**
    * if the gen2AllMemoryBankEnabled is enabled,
    * extract the values
    **/
    if (reader->u.serialReader.gen2AllMemoryBankEnabled)
    {
      extractGen2MemoryBankValues(read);
      /**
      * Now, we extracted all the values,
      * Disable the gen2AllMemoryBankEnabled option.
      **/
    }
#endif /* TMR_ENABLE_UHF */
    *i += msgDataLen;
  }
#ifdef TMR_ENABLE_UHF
  if (flags & TMR_TRD_METADATA_FLAG_GPIO_STATUS)
  {
    int j;
    uint8_t gpioByte=GETU8(msg, *i);

    switch(reader->u.serialReader.versionInfo.hardware[0])
    {
#ifdef TMR_ENABLE_UHF
      case TMR_SR_MODEL_MICRO:
        read->gpioCount = 2;
        break;
      case TMR_SR_MODEL_M6E:
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_HF_LF
      case TMR_SR_MODEL_M3E:
#endif /* TMR_ENABLE_HF_LF */
      default:
        read->gpioCount = 4;
        break;
    }
    for (j=0;j<read->gpioCount ;j++) 
    {
      read->gpio[j].id = j+1;
      /* GPO status */
      read->gpio[j].high = (((gpioByte >> j) & 0x1)== 1);
      /* GPI status returned in tag read metadata (upper nibble of byte(4-7))*/
      read->gpio[j].bGPIStsTagRdMeta = (((gpioByte >> (j+4)) & 0x1) == 1);
    }
  }
  if (TMR_TAG_PROTOCOL_GEN2 == read->tag.protocol)
  {
    if(flags & TMR_TRD_METADATA_FLAG_GEN2_Q)
    {
      read->u.gen2.q.u.staticQ.initialQ = GETU8(msg, *i);
    }
    if(flags & TMR_TRD_METADATA_FLAG_GEN2_LF)
    {
      uint8_t value;
      value = GETU8(msg, *i);
      switch(value) 
      {
        case 0x00:
        read->u.gen2.lf = TMR_GEN2_LINKFREQUENCY_250KHZ;
        break;
        case 0x02:
        read->u.gen2.lf = TMR_GEN2_LINKFREQUENCY_320KHZ;
        break;
        case 0x04:
        read->u.gen2.lf = TMR_GEN2_LINKFREQUENCY_640KHZ;
        break;
      }
    }
    if(flags & TMR_TRD_METADATA_FLAG_GEN2_TARGET)
    {
      uint8_t value;
      value = GETU8(msg, *i);
      switch(value) 
      {
        case 0x00:
        read->u.gen2.target = TMR_GEN2_TARGET_A;
        break;
        case 0x01:
        read->u.gen2.target = TMR_GEN2_TARGET_B;
        break;
      }
    }
  }
  if (flags & TMR_TRD_METADATA_FLAG_BRAND_IDENTIFIER)
  {
    memcpy(read->brandIdentifier, &(msg[*i]), 2);
    *i += 2;
  }
#endif /* TMR_ENABLE_HF_LF */
#ifdef TMR_ENABLE_HF_LF
  if(flags & TMR_TRD_METADATA_FLAG_TAGTYPE)
  {
    uint8_t len = 1, tagtype[8];

    len = parseEBVdata(msg, tagtype, i);
    read->tagType = TMR_SR_convertFromEBV(tagtype, len);
  }
#endif /* TMR_ENABLE_HF_LF */
}

void
TMR_SR_postprocessReaderSpecificMetadata(TMR_TagReadData *read, TMR_SR_SerialReader *sr)
{
  uint16_t j;
  uint32_t timestampLow;
  uint64_t currTime64, lastSentTagTime64; /*for comparison*/
  int32_t tempDiff;
  uint32_t time = 0;

  timestampLow = sr->readTimeLow;
  read->timestampHigh = sr->readTimeHigh;

  if((int)read->dspMicros <= delta)
  {
    time = delta + 1;
  }
  else
  {
    time = read->dspMicros;
  }

  timestampLow = timestampLow + time;
  LastDSPMicro = time;

  currTime64 = ((uint64_t)read->timestampHigh << 32) | timestampLow;
  lastSentTagTime64 = ((uint64_t)sr->lastSentTagTimestampHigh << 32) | sr->lastSentTagTimestampLow;

  if (lastSentTagTime64 >= currTime64)
  {
    tempDiff = (int32_t)(currTime64 - lastSentTagTime64);
    timestampLow = timestampLow - tempDiff + 1;

    if (timestampLow < sr->lastSentTagTimestampLow) /*account for overflow*/
    {
      read->timestampHigh++;
    }
  }

  if (timestampLow < sr->readTimeLow) /* Overflow */
  {
    read->timestampHigh++;
  }

  read->timestampLow = timestampLow;
  sr->lastSentTagTimestampHigh = read->timestampHigh;
  sr->lastSentTagTimestampLow = read->timestampLow;

#ifdef WIN32
 {	 
   uint64_t unixms;
   FILETIME ft;

   unixms= ((uint64_t)(read->timestampHigh)<<32) | (read->timestampLow);

   tmr_unixms_to_filetime(unixms, &ft);

   read->timestampHigh =(uint32_t)ft.dwHighDateTime;
   read->timestampLow = (uint32_t)ft.dwLowDateTime;
 }
#endif

  if(!isStreamEnabled)
  {
    uint8_t tx = (read->antenna >> 4) & 0xF;
    uint8_t rx = (read->antenna >> 0) & 0xF;

    // Due to limited space, Antenna 16 wraps around to 0
    if (0 == tx) { tx = 16; }
    if (0 == rx) { rx = 16; }

    for (j = 0; j < sr->defaultTxRxMap->len; j++)
    {
      if (rx == sr->defaultTxRxMap->list[j].rxPort &&
          tx == sr->defaultTxRxMap->list[j].txPort)
      {
        bool supportedModel = false;
        read->antenna = sr->defaultTxRxMap->list[j].antenna;

        if ((sr->versionInfo.hardware[0] != TMR_SR_MODEL_M6E_NANO) &&
            (sr->versionInfo.hardware[0] != TMR_SR_MODEL_M7E_PICO))
        {
          supportedModel = true;
        }

#ifdef TMR_ENABLE_UHF
        if (read->gpioCount > 2)
        {
          if (supportedModel && ((read->gpio[2].high)&& (!read->gpio[3].high)))
          {
            read->antenna += 16;
          }
          else if (supportedModel && ((!read->gpio[2].high) && (read->gpio[3].high)))
          {
            read->antenna += 32;
          }
          else
          {
            if (supportedModel && ((read->gpio[2].high) && (read->gpio[3].high)))
            {
              read->antenna += 48;
            }
          }
        }
#endif /* TMR_ENABLE_UHF */
        break;
      }
    }
	
	/* Custom map set? */
    if(sr->isTxRxMapSet)
    {
      for (j = 0; j < sr->txRxMap->len; j++)
      {
        if (read->antenna == sr->txRxMap->list[j].rxPort &&
            read->antenna == sr->txRxMap->list[j].txPort)
        {
          read->antenna = sr->txRxMap->list[j].antenna;
          break;
        }
      }
    }
  }
}

#ifdef TMR_ENABLE_ISO180006B
TMR_Status
TMR_SR_cmdISO180006BReadTagData(TMR_Reader *reader,
                                uint16_t timeout, uint8_t address,
                                uint8_t length, const TMR_TagFilter *filter,
                                TMR_TagReadData *read)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t copylen, i;

  if (length > 8
      || filter == NULL
      || TMR_FILTER_TYPE_TAG_DATA != filter->type
      || filter->u.tagData.epcByteCount != 8)
  {
    return TMR_ERROR_INVALID;
  }

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_READ_TAG_DATA);
  SETU16(msg, i, timeout);
  SETU8(msg, i, 0x01); /* Standard read operations */
  SETU8(msg, i, TMR_SR_ISO180006B_COMMAND_READ);
  SETU8(msg, i, 0x00); /* RFU */
  SETU8(msg, i, length);
  SETU8(msg, i, address);
  memcpy(&msg[i], filter->u.tagData.epc, 8);
  i += 8;
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  read->metadataFlags = TMR_TRD_METADATA_FLAG_DATA;
  read->tag.protocol = TMR_TAG_PROTOCOL_ISO180006B;
  read->tag.epcByteCount = 0;

  read->data.len = msg[1];
  copylen = (uint8_t)read->data.len;
  if (copylen > read->data.max)
  {
    copylen = (uint8_t)read->data.max;
  }
  if (NULL != read->data.list)
  {
    memcpy(read->data.list, &msg[5], copylen);
  }

  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_cmdISO180006BWriteTagData(TMR_Reader *reader,
                                 uint16_t timeout, uint8_t address,
                                 uint8_t count, const uint8_t data[],
                                 const TMR_TagFilter *filter)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;

  SETU8(msg, i, TMR_SR_OPCODE_WRITE_TAG_DATA);
  SETU16(msg, i, timeout);
  if (NULL != filter
    && TMR_FILTER_TYPE_TAG_DATA == filter->type
    && filter->u.tagData.epcByteCount == 8)
  {
    SETU8(msg, i, 
          TMR_SR_ISO180006B_WRITE_OPTION_READ_VERIFY_AFTER
          | TMR_SR_ISO180006B_WRITE_OPTION_COUNT_PROVIDED); 
    SETU8(msg, i, TMR_SR_ISO180006B_COMMAND_WRITE4BYTE);
    SETU8(msg, i, TMR_SR_ISO180006B_WRITE_LOCK_NO);
    SETU8(msg, i, address);
    memcpy(&msg[i], filter->u.tagData.epc, 8);
    i += 8;
  }
  else
  {
    SETU8(msg, i, TMR_SR_ISO180006B_WRITE_OPTION_GROUP_SELECT 
                  | TMR_SR_ISO180006B_WRITE_OPTION_COUNT_PROVIDED);
    SETU8(msg, i, TMR_SR_ISO180006B_COMMAND_WRITE4BYTE_MULTIPLE);
    SETU8(msg, i, TMR_SR_ISO180006B_WRITE_LOCK_NO);
    SETU8(msg, i, address);
    /* 
     * Actually, we don't use password in case of iso.
     * passing 1, instead of '0' fixes the crash.
     */
    ret = filterbytes(TMR_TAG_PROTOCOL_ISO180006B, filter, NULL,
                      &i, msg, 1, false);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
  }
  SETU16(msg, i, count);
  memcpy(&msg[i], data, count);
  i += count;
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);

}


TMR_Status
TMR_SR_cmdISO180006BLockTag(TMR_Reader *reader, uint16_t timeout,
                            uint8_t address, const TMR_TagFilter *filter)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  if (filter == NULL
      || TMR_FILTER_TYPE_TAG_DATA != filter->type
      || filter->u.tagData.epcByteCount != 8)
  {
    return TMR_ERROR_INVALID;
  }

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_LOCK_TAG);
  SETU16(msg, i, timeout);
  SETU8(msg, i, TMR_SR_ISO180006B_LOCK_OPTION_TYPE_FOLLOWS);
  SETU8(msg, i, TMR_SR_ISO180006B_LOCK_TYPE_QUERYLOCK_THEN_LOCK);
  SETU8(msg, i, address);
  memcpy(&msg[i], filter->u.tagData.epc, 8);
  i += 8;
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}
#endif /* TMR_ENABLE_ISO180006B */

#ifdef TMR_ENABLE_UHF
TMR_Status
TMR_SR_cmdGEN2WriteTagData(TMR_Reader *reader,
                           uint16_t timeout, TMR_GEN2_Bank bank,
                           uint32_t address, uint8_t count,
                           const uint8_t data[],
                           TMR_GEN2_Password accessPassword,
                           const TMR_TagFilter *filter)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t optbyte, i;

  i = 2;
  TMR_SR_msgAddGEN2DataWrite(msg, &i, timeout, bank, address, false);
  optbyte = isMultiSelectEnabled ? 6 : 5;
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, filter, &msg[optbyte], &i, msg,
                    accessPassword, true);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  if (i + count + 1 > TMR_SR_MAX_PACKET_SIZE)
  {
    return TMR_ERROR_TOO_BIG;
  }
  memcpy(&msg[i], data, count);
  i += count;
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}

/**
 * Read After Write data from a Gen2 tag.
 *
 * @param reader: the reader pointer which points to reader object
 * @param timeout: the duration in milliseconds to search for
 *                 a tag to write to. Valid range is 0-65535
 * @param writeBank: the Gen2 memory bank to write to
 * @param writeAddress: the word address to start writing at
 * @param count: bytes count of the data to write
 * @param data: the data to write - must be an even number of bytes
 * @param accessPassword: the password to use when writing the tag
 * @param filter: a specification of the air protocol filtering to
 *                perform to find the tag
 * @param readBank: the Gen2 memory bank to read from
 * @param readAddress: the word address to start reading from
 * @param readLen: the number of words to read
 * @param read: pointes to TMR_TagReadData
 */
TMR_Status
TMR_SR_cmdGEN2ReadAfterWriteTagData(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Bank writeBank, uint32_t writeAddress,
                                    uint16_t count, const uint8_t data[], TMR_GEN2_Password accessPassword, const TMR_TagFilter *filter,
                                    TMR_GEN2_Bank readBank, uint32_t readAddress, uint8_t readLen, TMR_TagReadData *read)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t optbyte, i;

  i = 2;
  TMR_SR_msgAddGEN2DataWrite(msg, &i, timeout, writeBank, writeAddress, true);

  optbyte = 6;
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, filter, &msg[optbyte], &i, msg,
  accessPassword, true);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  if (i + count + 1 > TMR_SR_MAX_PACKET_SIZE)
  {
    return TMR_ERROR_TOO_BIG;
  }
  memcpy(&msg[i], data, count);
  i += count;

  SETU8(msg, i, readBank);
  SETU32(msg, i, readAddress);
  SETU8(msg, i, readLen);

  msg[1] = i - 3; /* Install length */
  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  if (NULL != read->data.list)
  {
    i = 7;

    /* Read Tag Data doesn't put actual tag data inside the metadata fields.
    * Read the actual data here (remainder of response.) */
    {
      uint16_t dataLength;
      uint16_t copyLength;

      copyLength = dataLength = msg[1] + 5 - i;
      if (copyLength > read->data.max)
      {
        copyLength = read->data.max;
      }
      read->data.len = copyLength;
      memcpy(read->data.list, &msg[i], copyLength);
      i += dataLength;
    }
  }
  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_cmdGEN2LockTag(TMR_Reader *reader, uint16_t timeout,
                      uint16_t mask, uint16_t action, 
                      TMR_GEN2_Password accessPassword, 
                      const TMR_TagFilter *filter)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t optbyte, i;

  i = 2;
  TMR_SR_msgAddGEN2LockTag(msg, &i, timeout, mask, action, accessPassword);
  optbyte = isMultiSelectEnabled ? 6 : 5;
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, filter, &msg[optbyte], &i, msg,
                    0, false);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
    }
#endif /* TMR_ENABLE_UHF */
TMR_Status
TMR_SR_cmdrebootReader(TMR_Reader *reader)
{
  TMR_Status ret;
  TMR_SR_SerialReader *sr;
  TMR_SR_SerialTransport *transport;

  sr = &reader->u.serialReader;
  transport = &sr->transport;

  if (TMR_SR_MODEL_M3E != sr->versionInfo.hardware[0])
  {
    /*
     * Drop baud to 9600 so we know for sure what it will be after going
     * back to the bootloader.  (Older firmwares always revert to 9600.
     * Newer ones keep the current baud rate.)
     */
    if (NULL != transport->setBaudRate)
    {
      /**
       * some transport layer does not support baud rate settings.
       * for ex: TCP transport. In that case skip the baud rate
       * settings.
       */ 

      ret = TMR_SR_cmdSetBaudRate(reader, 9600);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
      ret = transport->setBaudRate(transport, 9600);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
    }
  }

  ret = TMR_SR_cmdBootBootloader(reader);
  if ((TMR_SUCCESS != ret)
      /* Invalid Opcode okay -- means "already in bootloader" */
      && (TMR_ERROR_INVALID_OPCODE != ret))
  {
    return ret;
  }

  /*
   * Wait for the bootloader to be entered. 200ms is enough.
   */
  tmr_sleep(200);

  return ret;
}

#ifdef TMR_ENABLE_UHF
TMR_Status
TMR_SR_cmdKillTag(TMR_Reader *reader, uint16_t timeout,
                  TMR_GEN2_Password killPassword, 
                  const TMR_TagFilter *filter)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t optbyte, i;

  i = 2;
  TMR_SR_msgAddGEN2KillTag(msg, &i, timeout, killPassword);
  optbyte = isMultiSelectEnabled ? 6 : 5;
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, filter, &msg[optbyte], &i, msg, 
                    0, false);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}


TMR_Status
TMR_SR_cmdGEN2ReadTagData(TMR_Reader *reader,
                          uint16_t timeout, TMR_GEN2_Bank bank,
                          uint32_t address, uint8_t length,
                          uint32_t accessPassword, const TMR_TagFilter *filter,
                          TMR_TagReadData *read)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t optbyte, i, mdfbyte;
  uint32_t starttimeHigh, starttimeLow;

  i = 2;
  TMR_SR_msgAddGEN2DataRead(msg, &i, timeout, bank, address, length, 0x00, true);
  optbyte = isMultiSelectEnabled ? 6 : 5;
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, filter, &msg[optbyte], &i, msg, 
                    accessPassword, true);


  msg[optbyte] |= 0x10;
  mdfbyte = isMultiSelectEnabled ? 7 : 6;
  read->metadataFlags |= TMR_TRD_METADATA_FLAG_DATA | TMR_TRD_METADATA_FLAG_PROTOCOL;
  SETU16(msg, mdfbyte, read->metadataFlags);


  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  /* Cache the read time so it can be put in tag read data later */
  tm_gettime_consistent(&starttimeHigh, &starttimeLow);
  reader->u.serialReader.readTimeHigh = starttimeHigh;
  reader->u.serialReader.readTimeLow = starttimeLow;

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  if (NULL != read->data.list)
  {
  i = isMultiSelectEnabled ? 9 :8;
   
  TMR_SR_parseMetadataOnly(reader, read, read->metadataFlags , &i, msg);
  TMR_SR_postprocessReaderSpecificMetadata(read, &reader->u.serialReader);

  /* Read Tag Data doesn't put actual tag data inside the metadata fields.
   * Read the actual data here (remainder of response.) */
  {
    uint16_t dataLength;
    uint16_t copyLength;
    
    copyLength = dataLength = msg[1] + 5 - i;
    if (copyLength > read->data.max)
    {
      copyLength = read->data.max;
  }
    read->data.len = copyLength;
    memcpy(read->data.list, &msg[i], copyLength);
    i += dataLength;
  }
  }

  return TMR_SUCCESS;
}
#endif /* TMR_ENABLE_UHF */

TMR_Status
TMR_SR_cmdSetTxRxPorts(TMR_Reader *reader, uint8_t txPort, uint8_t rxPort)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_ANTENNA_PORT);
  SETU8(msg, i, txPort);
  SETU8(msg, i, rxPort);

  return TMR_SR_sendCmd(reader, msg, i);
}


TMR_Status
TMR_SR_cmdSetAntennaSearchList(TMR_Reader *reader, uint8_t count,
                               const TMR_SR_PortPair *ports)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  uint8_t j;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_ANTENNA_PORT);
  SETU8(msg, i, 2); /* logical antenna list option */
  for (j = 0; j < count ; j++)
  {
    SETU8(msg, i, ports[j].txPort);
    SETU8(msg, i, ports[j].rxPort);
  }

  return TMR_SR_sendCmd(reader, msg, i);
}

#ifdef TMR_ENABLE_UHF
TMR_Status
TMR_SR_cmdSetAntennaPortPowersAndSettlingTime(
  TMR_Reader *reader, uint8_t count, const TMR_SR_PortPowerAndSettlingTime *ports)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t j, i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_ANTENNA_PORT);
  SETU8(msg, i, 4); /* power and settling time option */

  if(reader->featureFlags & TMR_READER_FEATURES_FLAG_EXTENDED_LOGICAL_ANTENNA)
  {
    SETU8(msg, i, reader->extendedAntOption);
    for (j = 0; j < count; j++)
    {
      SETU8(msg, i, ports[j].port);
      switch (reader->extendedAntOption)
      {
        case 1:
          {
            SETS16(msg, i, (int16_t) ports[j].readPower);
          }
          break;
        case 2:
          {
            SETS16(msg, i, (int16_t)ports[j].writePower);
          }
          break;
        case 3:
          {
            SETU16(msg, i, ports[j].settlingTime);
          }
          break;
      }
    }
  }
  else
  {
    for (j = 0; j < count; j++)
    {
      SETU8(msg, i, ports[j].port);
      SETS16(msg, i, (int16_t) ports[j].readPower);
      SETS16(msg, i, (int16_t)ports[j].writePower);
      SETU16(msg, i, ports[j].settlingTime);
    }
  }

  return TMR_SR_sendCmd(reader, msg, i);
}
#endif /* TMR_ENABLE_UHF */

TMR_Status TMR_SR_cmdSetReadWriteTxPower(TMR_Reader *reader, int32_t power, uint8_t opcode)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  /** The read power should always be a 16 bit value */
  if (power > 32767 || power < -32768)
  {
    return TMR_ERROR_ILLEGAL_VALUE;
  }

  i = 2;
  SETU8(msg, i, opcode);

  SETS16(msg,i, (int16_t)power);

  return TMR_SR_sendCmd(reader, msg, i);
}

TMR_Status
TMR_SR_cmdSetReadTxPower(TMR_Reader *reader, int32_t power)
{
  return TMR_SR_cmdSetReadWriteTxPower(reader, power, TMR_SR_OPCODE_SET_READ_TX_POWER);
}

#ifdef TMR_ENABLE_HF_LF
TMR_Status
TMR_SR_cmdSetProtocolList(TMR_Reader *reader, TMR_TagProtocolList *protocols)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i, idx;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_TAG_PROTOCOL);
  SETU8(msg, i , 0x01); //Sub Option for protocol list

  for (idx = 0; idx < protocols->len; idx++)
  {
    if (0 == ((1 << (protocols->list[idx] - 1)) & reader->u.serialReader.versionInfo.protocols))
    {
      return TMR_ERROR_UNSUPPORTED;
    }
    SETU16(msg, i, protocols->list[idx]);
  }

  return TMR_SR_sendCmd(reader, msg, i);
}
#endif /* TMR_ENABLE_HF_LF */

TMR_Status
TMR_SR_cmdSetProtocol(TMR_Reader *reader, TMR_TagProtocol protocol)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_TAG_PROTOCOL);
  SETU16(msg, i, protocol);

  return TMR_SR_sendCmd(reader, msg, i);
}

TMR_Status
TMR_SR_cmdSetWriteTxPower(TMR_Reader *reader, int32_t power)
{
  return TMR_SR_cmdSetReadWriteTxPower(reader, power, TMR_SR_OPCODE_SET_WRITE_TX_POWER);
}

#ifdef TMR_ENABLE_UHF
TMR_Status
TMR_SR_cmdSetFrequencyHopTable(TMR_Reader *reader, uint8_t count,
                               const uint32_t *table)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i, j;

  i = 2;

  if (count > 62)
  {
    return TMR_ERROR_TOO_BIG;
  }

  SETU8(msg, i, TMR_SR_OPCODE_SET_FREQ_HOP_TABLE);
  for (j = 0; j < count; j++)
  {
    SETU32(msg, i, table[j]);
  }

  return TMR_SR_sendCmd(reader, msg, i);
}


TMR_Status
TMR_SR_cmdSetFrequencyHopTableOption(TMR_Reader *reader, uint32_t value, uint8_t subOption)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_FREQ_HOP_TABLE);
  SETU8(msg, i, subOption); /* hop time option */
  SETU32(msg, i, value);

  return TMR_SR_sendCmd(reader, msg, i);
}

/**
 * Set Minimum frequency. 
 *
 * @param reader: The reader object pointer
 * @param freq: Minimum frequency value to be set. 
*/
TMR_Status
TMR_SR_cmdSetMinimumFrequency(TMR_Reader *reader, uint32_t freq)
{
  return TMR_SR_cmdSetFrequencyHopTableOption(reader, freq, TMR_SR_OPCODE_FREQ_HOP_TABLE_OPTION_MINIMUM_FREQUENCY);
}
#endif /* TMR_ENABLE_UHF */

TMR_Status
TMR_SR_cmdSetGPIO(TMR_Reader *reader, uint8_t gpio, bool high)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_USER_GPIO_OUTPUTS);
  SETU8(msg, i, gpio);
  SETU8(msg, i, (high == true));

  return TMR_SR_sendCmd(reader, msg, i);
}
#ifdef TMR_ENABLE_UHF
TMR_Status 
TMR_SR_regionConfiguration(bool lbtEnable, int8_t lbtThreshold, bool dwellTimeEnable, uint32_t *dwellTime)
{
  regionConfigurationFlag = true;
  LBTEnable = lbtEnable;

  if((*(uint32_t *)dwellTime > TMR_MIN_VALUE) && (*(uint32_t *)dwellTime <= TMR_MAX_VALUE))
  {
    DwellTime = *(uint16_t *)dwellTime;
  }
  else
  {
    return TMR_ERROR_INVALID_VALUE;
  }
  LBTThreshold = lbtThreshold;
  DwellTimeEnable = dwellTimeEnable;

  return TMR_SUCCESS;
}
#endif /* TMR_ENABLE_UHF */

TMR_Status
TMR_SR_cmdSetRegion(TMR_Reader *reader, TMR_Region region)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

#ifdef TMR_ENABLE_UHF
  if(regionConfigurationFlag)
  {
    if(reader->featureFlags & TMR_READER_FEATURES_FLAG_REGION_CONFIGURATION)
    {
      SETU8(msg, i, TMR_SR_OPCODE_SET_REGION);
      SETU8(msg, i, 0x01);
      SETU8(msg, i, TMR_REGION_OPEN);
      SETU8(msg, i, 0x40);
      SETU8(msg, i, LBTEnable ? 1 : 0);
      SETU8(msg, i, 0x41);
      SETU8(msg, i, LBTThreshold);
      SETU8(msg, i, 0x42);
      SETU8(msg, i, DwellTimeEnable ? 1 : 0);
      SETU8(msg, i, 0x43);
      SETU16(msg, i, DwellTime);
      msg[1] = i - 3;
    }
    else
    {
      return TMR_ERROR_MSG_INVALID_PARAMETER_VALUE;
    }
  }
  else
#endif /* TMR_ENABLE_UHF */
  {
    SETU8(msg, i, TMR_SR_OPCODE_SET_REGION);
    SETU8(msg, i, region);
  }

#ifdef TMR_ENABLE_UHF
  regionConfigurationFlag = false;
  DwellTimeEnable = false;
  LBTEnable = false;
#endif /* TMR_ENABLE_UHF*/
  return TMR_SR_sendCmd(reader, msg, i);
}

#ifdef TMR_ENABLE_UHF
TMR_Status TMR_SR_cmdSetRegionDwellTimeLbtEnable(TMR_Reader *reader, TMR_Region region, bool enable, uint8_t subOption)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_REGION);
  SETU8(msg, i, 0x01);
  SETU8(msg, i, region);
  SETU8(msg, i, subOption);
  SETU8(msg, i, enable ? 1:0);

  return TMR_SR_sendCmd(reader, msg, i);
}

TMR_Status
TMR_SR_cmdSetRegionDwellTimeEnable(TMR_Reader *reader, TMR_Region region, bool dwellTimeEnable)
{
  return TMR_SR_cmdSetRegionDwellTimeLbtEnable(reader, region, dwellTimeEnable,
                                               TMR_SR_REGION_CONFIGURATION_DWELLTIME_ENABLED);
}

TMR_Status
TMR_SR_cmdSetRegionDwellTime(TMR_Reader *reader, TMR_Region region, uint16_t dwellTime)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_REGION);
  SETU8(msg, i, 0x01);
  SETU8(msg, i, region);
  SETU8(msg, i, 0x43);
  SETU16(msg, i, dwellTime);

  return TMR_SR_sendCmd(reader, msg, i);
}

TMR_Status
TMR_SR_cmdSetRegionLbtThreshold(TMR_Reader *reader, TMR_Region region, int8_t lbtThreshold)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_REGION);
  SETU8(msg, i, 0x01);
  SETU8(msg, i, region);
  SETU8(msg, i, 0x41);
  SETU8(msg, i, lbtThreshold);

  return TMR_SR_sendCmd(reader, msg, i);
}

TMR_Status
TMR_SR_cmdSetRegionLbt(TMR_Reader *reader, TMR_Region region, bool lbt)
{
  return TMR_SR_cmdSetRegionDwellTimeLbtEnable(reader, region, lbt,
                                               TMR_SR_REGION_CONFIGURATION_LBT_ENABLED);
}
#endif /* TMR_ENABLE_HF_LF */


TMR_Status
TMR_SR_cmdSetMode(TMR_Reader *reader, uint8_t mode, uint8_t opcode)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, opcode);
  SETU8(msg, i, mode);

  return TMR_SR_sendCmd(reader, msg, i);
}

TMR_Status
TMR_SR_cmdSetPowerMode(TMR_Reader *reader, TMR_SR_PowerMode mode)
{
  return TMR_SR_cmdSetMode(reader, mode, TMR_SR_OPCODE_SET_POWER_MODE);
}

#ifdef TMR_ENABLE_UHF
TMR_Status
TMR_SR_cmdSetUserMode(TMR_Reader *reader, TMR_SR_UserMode mode)
{
  return TMR_SR_cmdSetMode(reader, mode, TMR_SR_OPCODE_SET_USER_MODE);
}
#endif /* TMR_ENABLE_UHF */


TMR_Status
TMR_SR_cmdSetReaderConfiguration(TMR_Reader *reader, TMR_SR_Configuration key,
                                 const void *value)
{
#ifdef TMR_ENABLE_UHF
	TMR_SR_SerialReader *sr = &reader->u.serialReader;
#endif /* TMR_ENABLE_UHF */
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_READER_OPTIONAL_PARAMS);
  SETU8(msg, i, 1); /* key-value form of command */
  SETU8(msg, i, key);

  switch (key)
  {
#ifdef TMR_ENABLE_HF_LF
  case TMR_SR_CONFIGURATION_KEEP_RF_ON:
#endif /* TMR_ENABLE_HF_LF */
  case TMR_SR_CONFIGURATION_ANTENNA_CONTROL_GPIO:
  case TMR_SR_CONFIGURATION_TRIGGER_READ_GPIO:
    SETU8(msg, i, *(uint8_t *)value);
    break;
#ifdef TMR_ENABLE_UHF
  case TMR_SR_CONFIGURATION_UNIQUE_BY_ANTENNA:
  case TMR_SR_CONFIGURATION_UNIQUE_BY_DATA:
  case TMR_SR_CONFIGURATION_UNIQUE_BY_PROTOCOL:
    SETU8(msg, i, *(bool *)value ? 0 : 1);
    break;
  case TMR_SR_CONFIGURATION_TRANSMIT_POWER_SAVE:
		if(TMR_SR_MODEL_MICRO == sr->versionInfo.hardware[0])
		{
			// Open loop power calibration
			SETU8(msg, i, *(bool *)value ? 2 : 1);
		}
		else
		{
			// Closed loop power calibration
			SETU8(msg, i, *(bool *)value ? 1 : 0);
		}
		break;
  case TMR_SR_CONFIGURATION_SAFETY_ANTENNA_CHECK:
  case TMR_SR_CONFIGURATION_SAFETY_TEMPERATURE_CHECK:
  case TMR_SR_CONFIGURATION_RECORD_HIGHEST_RSSI:
  case TMR_SR_CONFIGURATION_SELF_JAMMER_CANCELLATION:
  case TMR_SR_CONFIGURATION_SEND_CRC:
#endif /* TMR_ENABLE_UHF */
  case TMR_SR_CONFIGURATION_ENABLE_READ_FILTER:
    SETU8(msg, i, *(bool *)value ? 1 : 0);
    break;
#ifdef TMR_ENABLE_UHF
  case TMR_SR_CONFIGURATION_READ_FILTER_TIMEOUT:
    SETS32(msg, i, *(int32_t *)value);
    break;
#endif /* TMR_ENABLE_UHF */
  default:
    return TMR_ERROR_NOT_FOUND;
  }

  return TMR_SR_sendCmd(reader, msg, i);
}

/**
 * Handles license key to enable protocol 
 *
 * @param reader The reader
 * @param option Option to set or erase the license key
 * @param key license key
 * @param key_len Length of license key or the key array
 * @param retData The response data
*/


TMR_Status TMR_SR_cmdSetProtocolLicenseKey(TMR_Reader *reader, 
                                           TMR_SR_SetProtocolLicenseOption option, 
                                           uint8_t key[], int key_len)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  TMR_Status ret;
  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_PROTOCOL_LICENSEKEY);
  SETU8(msg, i, option);

  if(TMR_SR_SET_LICENSE_KEY == option)
  {
  memcpy(msg+i, key, key_len);
  i+= key_len;
  }

  ret = TMR_SR_sendCmd(reader, msg, i);
  if (ret != TMR_SUCCESS)
  {
    return ret;
  }

  /* Parse the command response only if setting license key */ 
  if(TMR_SR_SET_LICENSE_KEY == option)
  {
    reader->u.serialReader.versionInfo.protocols = 0x00;
    for (i = 0; i < msg[1] - 2 ; i += 2)
    {
      reader->u.serialReader.versionInfo.protocols |= (1 << (GETU8AT(msg, 7 + i) - 1)); 
    }
  }

  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_cmdSetProtocolConfiguration(TMR_Reader *reader, TMR_TagProtocol protocol,
                                   TMR_SR_ProtocolConfiguration key,
                                   const void *value)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
#ifdef TMR_ENABLE_UHF
  uint8_t BLF = 0;
  int target = 0;
#endif /* TMR_ENABLE_UHF */ 
  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_PROTOCOL_PARAM);
  SETU8(msg, i, protocol);

#ifdef TMR_ENABLE_UHF
  if (TMR_TAG_PROTOCOL_GEN2 == key.protocol)
  {
    SETU8(msg, i, key.u.gen2);
    switch (key.u.gen2)
    {
      case TMR_SR_GEN2_CONFIGURATION_SESSION:
      case TMR_SR_GEN2_CONFIGURATION_TAGENCODING:
      case TMR_SR_GEN2_CONFIGURATION_TARI:
      case TMR_SR_GEN2_CONFIGURATION_PROTCOLEXTENSION:
      /* if sendSelect = 00, send Select with first Query and whenever antenna is switched(default case) 
       * if sendSelect = 01, send Select with every Query */
      case TMR_SR_GEN2_SEND_SELECT:
        {
          SETU8(msg, i, *(uint16_t *)value);
          break;
        }
      case TMR_SR_GEN2_CONFIGURATION_LINKFREQUENCY:
        {
          switch (*(TMR_GEN2_LinkFrequency *)value)
          {
            case 250:
              BLF = 0x00;
              break;
            case 320:
              BLF = 0x02;
              break;
            case 640:
              BLF = 0x04;
              break;
            default:
              return TMR_ERROR_INVALID;
          }
          SETU8(msg, i, BLF);
          break;
        }
      case TMR_SR_GEN2_CONFIGURATION_TARGET:
        {
          switch (*(TMR_GEN2_Target *)value)
          {
            case TMR_GEN2_TARGET_A:
              target = 0x0100;
              break;
            case TMR_GEN2_TARGET_B:
              target = 0x0101;
              break;
            case TMR_GEN2_TARGET_AB:
              target = 0x0000;
              break;
            case TMR_GEN2_TARGET_BA:
              target = 0x0001;
              break;
            default:
              return TMR_ERROR_INVALID;
          }
          SETU16(msg, i, target);
          break;
        }
      case TMR_SR_GEN2_CONFIGURATION_Q:
        {
          const TMR_SR_GEN2_Q *q = value;
          SETU8(msg, i, q->type);

          if (q->type == TMR_SR_GEN2_Q_STATIC)
          {
            SETU8(msg, i, q->u.staticQ.initialQ);
          }
          break;
        }
      case TMR_SR_GEN2_CONFIGURATION_BAP:
        {
          TMR_GEN2_Bap *bap;
          bap = (TMR_GEN2_Bap *)value;

          //version
          SETU8(msg, i, 0x01);
          // Enabling power-up delay and frequency hop offtime bits.
          SETU16(msg, i, 0x03);
          SETU32(msg, i, bap->powerUpDelayUs);
          SETU32(msg, i, bap->freqHopOfftimeUs);
      
          /* M value and FlexQueryPayload” are unsupported. */ 
          break;
        }
      case TMR_SR_GEN2_CONFIGURATION_T4:
        {
          SETU32(msg, i, *(uint32_t *)value);
          break;
        }
      case TMR_SR_GEN2_INITIAL_Q:
        {
          TMR_GEN2_initQ *Q;
          Q = (TMR_GEN2_initQ *)value;

          SETU8(msg, i, Q->qEnable);
          if(Q->qEnable)
          {
            /* "Initial Q" value(this field is absent if qEnable is false) */
            SETU8(msg, i, Q->qValue);
          }
          break;
        }
      default:
        return TMR_ERROR_NOT_FOUND;
    }
  }
#ifdef TMR_ENABLE_ISO180006B
  else if (TMR_TAG_PROTOCOL_ISO180006B == key.protocol 
           || TMR_TAG_PROTOCOL_ISO180006B_UCODE == key.protocol)
  {
    switch (key.u.iso180006b)
    {
      case TMR_SR_ISO180006B_CONFIGURATION_LINKFREQUENCY:
        {
          switch (*(int *)value)
          {
            case 40:
              BLF = 0x01;
              break;
            case 160:
              BLF = 0x00;
              break;
            default:
              return TMR_ERROR_INVALID;
          }
          break;
        }
      case TMR_SR_ISO180006B_CONFIGURATION_MODULATION_DEPTH:
        {
          switch (*(int *)value)
          {
            case 0:
              BLF = TMR_ISO180006B_Modulation99percent;
              break;
            case 1:
              BLF = TMR_ISO180006B_Modulation11percent;
              break;
            default:
              return TMR_ERROR_INVALID;
          }
          break;
        }
      case TMR_SR_ISO180006B_CONFIGURATION_DELIMITER:
        {
          switch (*(int *)value)
          {
            case 1:
              BLF = TMR_ISO180006B_Delimiter1;
              break;
            case 4:
              BLF = TMR_ISO180006B_Delimiter4;
              break;
            default:
              return  TMR_ERROR_INVALID;
          }
          break;
        }
      default:
        return TMR_ERROR_NOT_FOUND;
    }

    SETU8(msg, i, key.u.iso180006b);
    SETU8(msg, i, BLF);
  }
#endif /* TMR_ENABLE_ISO180006B */
#endif /* TMR_ENABLE_UHF */

#ifndef TMR_ENABLE_UHF
  if (TMR_TAG_PROTOCOL_ISO14443A == key.protocol)
#else
  else if (TMR_TAG_PROTOCOL_ISO14443A == key.protocol)
#endif /* TMR_ENABLE_UHF */
  {
#ifdef TMR_ENABLE_HF_LF
    TMR_Status ret = TMR_SUCCESS;
    switch (key.u.iso14443a)
    {
      case TMR_SR_ISO14443A_CONFIGURATION_TAGTYPE:
        {
          SETU8(msg, i, key.u.iso14443a);
          ret = TMR_SR_convertToEBV(msg, &i, *(TMR_ISO14443A_TagType *)value);
          if (TMR_SUCCESS != ret)
          {
            return ret;
          }
          break;
        }
      default:
        return TMR_ERROR_NOT_FOUND;
    }
  }
  else if (TMR_TAG_PROTOCOL_ISO14443B == key.protocol)
  {
    TMR_Status ret = TMR_SUCCESS;
    switch (key.u.iso14443b)
    {
      case TMR_SR_ISO14443B_CONFIGURATION_TAGTYPE:
        {
          SETU8(msg, i, key.u.iso14443b);
          ret = TMR_SR_convertToEBV(msg, &i, *(TMR_ISO14443B_TagType *)value);
          if (TMR_SUCCESS != ret)
          {
            return ret;
          }
          break;
        }
      default:
        return TMR_ERROR_NOT_FOUND;
    }
  }
  else if (TMR_TAG_PROTOCOL_ISO15693 == key.protocol)
  {
    TMR_Status ret = TMR_SUCCESS;
    switch (key.u.iso15693)
    {
      case TMR_SR_ISO15693_CONFIGURATION_TAGTYPE:
        {
          SETU8(msg, i, key.u.iso15693);
          ret = TMR_SR_convertToEBV(msg, &i, *(TMR_ISO15693_TagType *)value);
          if (TMR_SUCCESS != ret)
          {
            return ret;
          }
          break;
        }
      default:
        return TMR_ERROR_NOT_FOUND;
    }
  }
  else if (TMR_TAG_PROTOCOL_LF125KHZ == key.protocol)
  {
    TMR_Status ret = TMR_SUCCESS;
    switch (key.u.lf125khz)
    {
      case TMR_SR_LF125KHZ_CONFIGURATION_TAGTYPE:
        {
          SETU8(msg, i, key.u.lf125khz);
          ret = TMR_SR_convertToEBV(msg, &i, *(TMR_LF125KHZ_TagType *)value);
          if (TMR_SUCCESS != ret)
          {
            return ret;
          }
          break;
        }
      case TMR_SR_LF125KHZ_CONFIGURATION_SECURE_RD_FORMAT:
        {
          SETU8(msg, i, key.u.lf125khz);
          SETU8(msg, i, *(uint8_t *)value);
          break;
        }
      default:
        return TMR_ERROR_NOT_FOUND;
    }
  }
  else if (TMR_TAG_PROTOCOL_LF134KHZ == key.protocol)
  {
    TMR_Status ret = TMR_SUCCESS;
    switch (key.u.lf134khz)
    {
      case TMR_SR_LF134KHZ_CONFIGURATION_TAGTYPE:
        {
          SETU8(msg, i, key.u.lf134khz);
          ret = TMR_SR_convertToEBV(msg, &i, *(TMR_LF134KHZ_TagType *)value);
          if (TMR_SUCCESS != ret)
          {
            return ret;
          }
          break;
        }
      default:
        return TMR_ERROR_NOT_FOUND;
    }
#endif /* TMR_ENABLE_HF_LF */
  }
  else
  {
    return TMR_ERROR_INVALID;
  }

  return TMR_SR_sendCmd(reader, msg, i);
}


TMR_Status TMR_SR_cmdGetTxRxPorts(TMR_Reader *reader, TMR_SR_PortPair *ant)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;

  SETU8(msg, i, TMR_SR_OPCODE_GET_ANTENNA_PORT);
  SETU8(msg, i, 0); /* just configured ports */

  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  ant->txPort = msg[5];
  ant->rxPort = msg[6];
  
  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_cmdAntennaDetect(TMR_Reader *reader, uint8_t *count,
                        TMR_SR_PortDetect *ports)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i, j; 

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_GET_ANTENNA_PORT);
  SETU8(msg, i, 5); /* antenna detect option */

  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  for (i = 1, j = 0; i < msg[1] && j < *count; i += 2, j++)
  {
    ports[j].port = msg[i + 5];
    ports[j].detected = (msg[i + 6] == 1);
  }
  *count = j;

  return TMR_SUCCESS;
}


#ifdef TMR_ENABLE_UHF
TMR_Status
TMR_SR_cmdGetAntennaPortPowersAndSettlingTime(
  TMR_Reader *reader, uint8_t *count, TMR_SR_PortPowerAndSettlingTime *ports)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i, j;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_GET_ANTENNA_PORT);
  SETU8(msg, i, 4); /* power and settling time option */
  if(reader->featureFlags & TMR_READER_FEATURES_FLAG_EXTENDED_LOGICAL_ANTENNA)
  {
    SETU8(msg, i, reader->extendedAntOption);
  }

  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  if(reader->featureFlags & TMR_READER_FEATURES_FLAG_EXTENDED_LOGICAL_ANTENNA)
  {
    reader->extendedAntOption = GETU8AT(msg, 6);
    for (i = 1, j = 0; i < msg[1] && j < *count; i += 3, j++)
    {
      ports[j].port = GETU8AT(msg, i + 6);
      switch (reader->extendedAntOption)
      {
        case 1:
          {
            ports[j].readPower = (int16_t)GETS16AT(msg, i + 7);
            ports[j].writePower = 0x0000;
            ports[j].settlingTime = 0x0000;
          }
          break;
        case 2:
          {
            ports[j].readPower = 0x0000;
            ports[j].writePower = (int16_t)GETS16AT(msg, i + 7);
            ports[j].settlingTime = 0x0000;
          }
          break;
        case 3:
          {
            ports[j].readPower = 0x0000;
            ports[j].writePower = 0x0000;
            ports[j].settlingTime = GETU16AT(msg, i + 7);
          }
          break;
      }
    }
  }
  else
  {
    for (i = 1, j = 0; i < msg[1] && j < *count; i += 7, j++)
    {
      ports[j].port = GETU8AT(msg, i + 5);
      ports[j].readPower = (int16_t)GETS16AT(msg, i + 6);
      ports[j].writePower = (int16_t)GETS16AT(msg, i + 8);
      ports[j].settlingTime = GETU16AT(msg, i + 10);
    }
  }
  *count = j;

  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_cmdGetAntennaReturnLoss(TMR_Reader *reader, TMR_PortValueList *ports)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i, j, count;

  count = TMR_SR_MAX_ANTENNA_PORTS;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_GET_ANTENNA_PORT);
  SETU8(msg, i, 6); /* antenna return loss option */

  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  
  for (i = 1, j = 0; i < msg[1] && j < count; i += 2, j++)
  {
    if (j < ports->max)
    {
      ports->list[j].port = (uint8_t)GETU8AT(msg, i + 5);
      ports->list[j].value = (int16_t)GETU8AT(msg, i + 6);
    }
    else
    {
      /* no sufficent memory, break from here */
      break;
    }
  }
  ports->len = (uint8_t)j;

  return TMR_SUCCESS;
}
#endif /* TMR_ENABLE_UHF */

TMR_Status
TMR_SR_cmdGetReadWriteTxPower(TMR_Reader *reader, int32_t *power, uint8_t opcode)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, opcode);
  SETU8(msg, i, 0); /* just return power */

  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  *power = (int32_t)GETS16AT(msg, 6);

  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_cmdGetReadTxPower(TMR_Reader *reader, int32_t *power)
{
  return TMR_SR_cmdGetReadWriteTxPower(reader, power, TMR_SR_OPCODE_GET_READ_TX_POWER);
}


TMR_Status
TMR_SR_cmdGetReadTxPowerWithLimits(TMR_Reader *reader,
                                   TMR_SR_PowerWithLimits *power)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_GET_READ_TX_POWER);
  SETU8(msg, i, 1); /* return limits */

  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  power->setPower = GETU16AT(msg, 6);
  power->maxPower = GETU16AT(msg, 8);
  power->minPower = GETU16AT(msg, 10);

  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_cmdGetWriteTxPower(TMR_Reader *reader, int32_t *power)
{
  return TMR_SR_cmdGetReadWriteTxPower(reader, power, TMR_SR_OPCODE_GET_WRITE_TX_POWER);
}

#ifdef TMR_ENABLE_UHF
TMR_Status
TMR_SR_cmdGetFrequencyHopTable(TMR_Reader *reader, uint8_t *count,
                               uint32_t *hopTable)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i = 3, j, len;

  ret = TMR_SR_callSendCmd(reader, msg, TMR_SR_OPCODE_GET_FREQ_HOP_TABLE);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  len = msg[1] / 4;
  for (j = 0; i < *count && j < len ; j++)
  {
    hopTable[j] = GETU32AT(msg, 5 + 4*j);
  }
  *count = len;

  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_cmdGetFrequencyHopTableOption(TMR_Reader *reader, uint32_t *value, uint8_t subOption)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_GET_FREQ_HOP_TABLE);
  SETU8(msg, i, subOption);

  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  *value = GETU32AT(msg, 6);

  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_cmdGetFrequencyHopTime(TMR_Reader *reader, uint32_t *hopTime)
{
  return TMR_SR_cmdGetFrequencyHopTableOption(reader, hopTime, TMR_SR_OPCODE_FREQ_HOP_TABLE_OPTION_HOPTIME);
}

/**
 * Get Quantization step value. 
 *
 * @param reader: The reader object pointer
 * @param step: pointer to collect Quantization step value. 
*/
TMR_Status
TMR_SR_cmdGetQuantizationStep(TMR_Reader *reader, uint32_t *step)
{
  return TMR_SR_cmdGetFrequencyHopTableOption(reader, step, TMR_SR_OPCODE_FREQ_HOP_TABLE_OPTION_QUANTIZATION_STEP);
}

/**
 * Get Minimum frequency. 
 *
 * @param reader: The reader object pointer
 * @param freq: pointer to collect Minimum frequency value. 
*/
TMR_Status
TMR_SR_cmdGetMinimumFrequency(TMR_Reader *reader, uint32_t *freq)
{
  return TMR_SR_cmdGetFrequencyHopTableOption(reader, freq, TMR_SR_OPCODE_FREQ_HOP_TABLE_OPTION_MINIMUM_FREQUENCY);
}
#endif /* TMR_ENABLE_UHF */


TMR_Status
TMR_SR_cmdGetGPIO(TMR_Reader *reader, uint8_t *count, TMR_GpioPin *state)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE] = {0};
  uint8_t i, j, len, offset;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_GET_USER_GPIO_INPUTS);
  SETU8(msg, i, 0x01);  // option
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, reader->u.serialReader.commandTimeout);
  if (TMR_SUCCESS != ret)
    return ret;

  len = (msg[1] - 1)/3;
  if (len > *count)
  {
    len = *count;
  }

  offset = 6;
  for (j = 0; j < len ; j++)
  {
    state[j].id = msg[offset++];
    state[j].output = (1 == msg[offset++]) ? true : false;
    state[j].high = (1 == msg[offset++]) ? true : false;
  }
  *count = len;

  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_cmdGetGPIODirection(TMR_Reader *reader, uint8_t pin, bool *out)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_USER_GPIO_OUTPUTS);
  SETU8(msg, i, pin);

  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
    return ret;

  *out = (msg[6] == 1);

  return TMR_SUCCESS;
}


TMR_Status
TMR_SR_cmdSetGPIODirection(TMR_Reader *reader, uint8_t pin, bool out)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_USER_GPIO_OUTPUTS);
  SETU8(msg, i, 1); /* Option flag */
  SETU8(msg, i, pin);
  SETU8(msg, i, (out == true) ? 1 : 0);
  SETU8(msg, i, 0); /* New value if output */

  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
    return ret;

  return TMR_SUCCESS;
}

#if !((!defined(TMR_ENABLE_UHF)) || defined(BARE_METAL))
TMR_Status
TMR_SR_cmdSetRegulatoryTest(struct TMR_Reader *reader, bool enable)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i = 2;

  SETU8(msg, i, TMR_SR_OPCODE_TX_CW_SIGNAL);
  if(enable)
  {
    SETU8(msg, i, (uint8_t)reader->regulatoryParams.RegModulation);
    SETU8(msg, i, (uint8_t)0x00);
    SETU8(msg, i, (uint8_t)reader->regulatoryParams.RegMode); // no. of cycle will be 0 for CONTINUOUS mode and 1 for TIMED mode.
    SETU16(msg, i, (uint16_t)reader->regulatoryParams.regOnTime);
    if((uint8_t)reader->regulatoryParams.RegMode == CONTINUOUS)
    {
      SETU16(msg, i, (uint16_t)reader->regulatoryParams.regOffTime);
    }
    else
    {
      SETU16(msg, i, (uint16_t)0x0000);
    }
  }
  else
  {
    SETU8(msg, i, (uint8_t)0x00);
  }
  return TMR_SR_sendCmd(reader, msg, i);
}
#endif /* TMR_ENABLE_HF_LF || BARE_METAL */

#ifdef TMR_ENABLE_HF_LF
TMR_Status
TMR_SR_cmdGetProtocolList(TMR_Reader *reader, TMR_TagProtocolList *protocols)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_GET_TAG_PROTOCOL);
  SETU8(msg, i, 0x01); //Sub Option for protocol list
  
  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  protocols->len = 0;
  for (i = 0; i < msg[1] - 1 ; i += 2)
  {
    LISTAPPEND(protocols, (TMR_TagProtocol)GETU16AT(msg, 6 + i)); 
  }

  return TMR_SUCCESS;
 }
#endif /* TMR_ENABLE_HF_LF */

TMR_Status 
TMR_SR_cmdGetCurrentProtocol(TMR_Reader *reader, TMR_TagProtocol *protocol)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];

  ret = TMR_SR_callSendCmd(reader, msg, TMR_SR_OPCODE_GET_TAG_PROTOCOL);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  *protocol = (TMR_TagProtocol)GETU16AT(msg, 5);

  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_cmdGetRegion(TMR_Reader *reader, TMR_Region *region)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];

  ret = TMR_SR_callSendCmd(reader, msg, TMR_SR_OPCODE_GET_REGION);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  *region = (TMR_Region)GETU8AT(msg, 5);
  
  return TMR_SUCCESS;
}

#ifdef TMR_ENABLE_UHF
TMR_Status
TMR_SR_cmdGetRegionConfiguration(TMR_Reader *reader,
                                 TMR_SR_RegionConfiguration key,
                                 void *value)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i, getValue;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_GET_REGION);
  SETU8(msg, i, 1);
  SETU8(msg, i, key);

  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  getValue = GETU8AT(msg, 8);
  switch (key)
  {
  case TMR_SR_REGION_CONFIGURATION_LBT_ENABLED:
    *(bool *)value = (getValue == 1);
    break;
  case TMR_SR_REGION_CONFIGURATION_LBTTHRESHOLD:
    *(int8_t *)value = getValue;
    break;
  case TMR_SR_REGION_CONFIGURATION_DWELLTIME_ENABLED:
    *(bool *)value = (getValue == 1);
    break;
  case TMR_SR_REGION_CONFIGURATION_DWELLTIME:
    *(uint16_t *)value = GETU16AT(msg, 8);
    break;
  default:
    return TMR_ERROR_NOT_FOUND;
  }

  return TMR_SUCCESS;
}
#endif /* TMR_ENABLE_UHF */


TMR_Status
TMR_SR_cmdGetPowerMode(TMR_Reader *reader, TMR_SR_PowerMode *mode)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];

  ret = TMR_SR_callSendCmd(reader, msg, TMR_SR_OPCODE_GET_POWER_MODE);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  *mode = (TMR_SR_PowerMode)GETU8AT(msg, 5);

  return TMR_SUCCESS;
}

#ifdef TMR_ENABLE_UHF
TMR_Status
TMR_SR_cmdGetUserMode(TMR_Reader *reader, TMR_SR_UserMode *mode)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];

  ret = TMR_SR_callSendCmd(reader, msg, TMR_SR_OPCODE_GET_USER_MODE);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  *mode = (TMR_SR_UserMode)GETU8AT(msg, 5);

  return TMR_SUCCESS;
}
#endif /* TMR_ENABLE_UHF */


TMR_Status
TMR_SR_cmdGetReaderConfiguration(TMR_Reader *reader, TMR_SR_Configuration key,
                                 void *value)
{
#ifdef TMR_ENABLE_UHF
	TMR_SR_SerialReader *sr = &reader->u.serialReader;
#endif /* TMR_ENABLE_UHF */
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i, getValue;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_GET_READER_OPTIONAL_PARAMS);
  SETU8(msg, i, 1); /* key-value form of command */
  SETU8(msg, i, key);

  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  getValue = GETU8AT(msg, 7);
  switch (key)
  {
#ifdef TMR_ENABLE_HF_LF
  case TMR_SR_CONFIGURATION_KEEP_RF_ON:
#endif /* TMR_ENABLE_HF_LF */
#ifdef TMR_ENABLE_UHF
  case TMR_SR_CONFIGURATION_ANTENNA_CONTROL_GPIO:
#endif /* TMR_ENABLE_UHF */
  case TMR_SR_CONFIGURATION_TRIGGER_READ_GPIO:
    *(uint8_t *)value = getValue;
    break;
#ifdef TMR_ENABLE_UHF
  case TMR_SR_CONFIGURATION_UNIQUE_BY_ANTENNA:
  case TMR_SR_CONFIGURATION_UNIQUE_BY_DATA:
  case TMR_SR_CONFIGURATION_UNIQUE_BY_PROTOCOL:
    *(bool *)value = (getValue == 0);
    break;
  case TMR_SR_CONFIGURATION_TRANSMIT_POWER_SAVE:
		if(TMR_SR_MODEL_MICRO == sr->versionInfo.hardware[0])
		{
			if(getValue == 2)
			{
				//Open loop power calibration
				*(bool *)value = 1;
			}
			else
			{
				//Closed loop power calibration
				*(bool *)value = 0;
			}
		}
		else
		{
			*(bool *)value = (getValue == 1);
		}
		break;
  case TMR_SR_CONFIGURATION_SAFETY_ANTENNA_CHECK:
  case TMR_SR_CONFIGURATION_SAFETY_TEMPERATURE_CHECK:
  case TMR_SR_CONFIGURATION_RECORD_HIGHEST_RSSI:
  case TMR_SR_CONFIGURATION_SELF_JAMMER_CANCELLATION:
  case TMR_SR_CONFIGURATION_SEND_CRC:
#endif /* TMR_ENABLE_UHF */
  case TMR_SR_CONFIGURATION_ENABLE_READ_FILTER:
    *(bool *)value = (getValue == 1);
    break;
#ifdef TMR_ENABLE_UHF
  case TMR_SR_CONFIGURATION_CURRENT_MSG_TRANSPORT:
    *(uint8_t *)value = getValue;
    break;
  case TMR_SR_CONFIGURATION_READ_FILTER_TIMEOUT:
    *(uint32_t *)value = getValue;
    break;
#endif /* TMR_ENABLE_UHF */
  case TMR_SR_CONFIGURATION_PRODUCT_GROUP_ID:
    *(uint16_t *)value = GETU16AT(msg, 7);
    break;

  case TMR_SR_CONFIGURATION_PRODUCT_ID:
    *(uint16_t *)value = GETU16AT(msg, 7);
    break;

  default:
    return TMR_ERROR_NOT_FOUND;
  }

  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_cmdGetProtocolConfiguration(TMR_Reader *reader, TMR_TagProtocol protocol,
                                   TMR_SR_ProtocolConfiguration key,
                                   void *value)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i, protocolKey = 0;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_GET_PROTOCOL_PARAM);
  SETU8(msg, i, protocol);

  // Frame configuration key based on protocol
  switch (key.protocol)
  {
#ifdef TMR_ENABLE_UHF
    case TMR_TAG_PROTOCOL_GEN2:
      {
        protocolKey = key.u.gen2;
        break;
      }
#endif /* TMR_ENABLE_UHF*/
#ifdef TMR_ENABLE_ISO180006B
    case TMR_TAG_PROTOCOL_ISO180006B:
    case TMR_TAG_PROTOCOL_ISO180006B_UCODE:
      {
        protocolKey = key.u.iso180006b;
        break;
      }
#endif /* TMR_ENABLE_ISO180006B */
#ifdef TMR_ENABLE_HF_LF
    case TMR_TAG_PROTOCOL_ISO14443A:
      {
        protocolKey = key.u.iso14443a;
        break;
      }
    case TMR_TAG_PROTOCOL_ISO14443B:
      {
        protocolKey = key.u.iso14443b;
        break;
      }
    case TMR_TAG_PROTOCOL_ISO15693:
      {
        protocolKey = key.u.iso15693;
        break;
      }
    case TMR_TAG_PROTOCOL_LF125KHZ:
      {
        protocolKey = key.u.lf125khz;
        break;
      }
    case TMR_TAG_PROTOCOL_LF134KHZ:
      {
        protocolKey = key.u.lf134khz;
        break;
      }
#endif /* TMR_ENABLE_HF_LF */
    default:
        return TMR_ERROR_INVALID;
  }

  SETU8(msg, i, protocolKey);
  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

#ifdef TMR_ENABLE_UHF
  if (TMR_TAG_PROTOCOL_GEN2 == key.protocol)
  {
    uint8_t getValue = GETU8AT(msg, 7);
    switch (key.u.gen2)
    {
    case TMR_SR_GEN2_CONFIGURATION_SESSION:
        *(TMR_GEN2_Session*)value = (TMR_GEN2_Session)getValue;
        break;

    case TMR_SR_GEN2_CONFIGURATION_TAGENCODING:
        *(TMR_GEN2_TagEncoding*)value = (TMR_GEN2_TagEncoding)getValue;
        break;

    case TMR_SR_GEN2_CONFIGURATION_LINKFREQUENCY:
        *(TMR_GEN2_LinkFrequency*)value = (TMR_GEN2_LinkFrequency)getValue;
        break;

    case TMR_SR_GEN2_CONFIGURATION_TARI:
        *(TMR_GEN2_Tari*)value = (TMR_GEN2_Tari)getValue;
        break;

    case TMR_SR_GEN2_CONFIGURATION_PROTCOLEXTENSION:
        *(TMR_GEN2_ProtocolExtension*)value = (TMR_GEN2_ProtocolExtension)getValue;
        break;
    case TMR_SR_GEN2_SEND_SELECT:
    {
        *(uint8_t*)value = getValue;
        break;
    }
    case TMR_SR_GEN2_CONFIGURATION_BAP:
      {
        TMR_GEN2_Bap *b = value;

        i = 10;
        b->powerUpDelayUs = GETU32(msg,i);
        b->freqHopOfftimeUs = GETU32(msg,i);
        break;
      }

    case TMR_SR_GEN2_CONFIGURATION_TARGET:
      {
        uint16_t target;

        target = GETU16AT(msg, 7);
        switch (target)
        {
        case 0x0100:
          *(TMR_GEN2_Target *)value = TMR_GEN2_TARGET_A;
          break;
        case 0x0101:
          *(TMR_GEN2_Target *)value = TMR_GEN2_TARGET_B;
          break;
        case 0x0000:
          *(TMR_GEN2_Target *)value = TMR_GEN2_TARGET_AB;
          break;
        case 0x0001:
          *(TMR_GEN2_Target *)value = TMR_GEN2_TARGET_BA;
          break;
        default:
          *(TMR_GEN2_Target *)value = TMR_GEN2_TARGET_INVALID;
        }
        break;
      }

    case TMR_SR_GEN2_CONFIGURATION_Q:
      {
        TMR_SR_GEN2_Q *q = value;

        q->type = (TMR_SR_GEN2_QType)GETU8AT(msg, 7);
        if (q->type == TMR_SR_GEN2_Q_STATIC)
        {
          q->u.staticQ.initialQ = GETU8AT(msg, 8);
        }
        break;
      }

    case TMR_SR_GEN2_CONFIGURATION_T4:
      *(uint32_t *)value = GETU32AT(msg, 7); 
      break;

    case TMR_SR_GEN2_INITIAL_Q:
      {
        TMR_GEN2_initQ *Q;
        Q = (TMR_GEN2_initQ *)value;

        Q->qEnable = (bool)GETU8AT(msg, 7);
        Q->qValue = (uint8_t)GETU8AT(msg, 8);
        break;
      }
    default:
      return TMR_ERROR_NOT_FOUND;
    }
  }
#ifdef TMR_ENABLE_ISO180006B
  else if (TMR_TAG_PROTOCOL_ISO180006B == key.protocol 
    || TMR_TAG_PROTOCOL_ISO180006B_UCODE == key.protocol)
  {
    uint8_t getValue = GETU8AT(msg, 7);

    switch (key.u.iso180006b)
    {
    case TMR_SR_ISO180006B_CONFIGURATION_LINKFREQUENCY:
      {
        TMR_iso18000BBLFValToInt(getValue, value);
        break;
      }
    case TMR_SR_ISO180006B_CONFIGURATION_MODULATION_DEPTH:
      {
        switch (getValue)
        {
        case 0:
          *(TMR_ISO180006B_ModulationDepth *)value = TMR_ISO180006B_Modulation99percent;
          break;
        case 1:
          *(TMR_ISO180006B_ModulationDepth *)value = TMR_ISO180006B_Modulation11percent;
          break;
        default:
          return TMR_ERROR_NOT_FOUND;
        }
        break;
      }
    case TMR_SR_ISO180006B_CONFIGURATION_DELIMITER:
      {
        switch (getValue)
        {
        case 1:
          *(TMR_ISO180006B_Delimiter *)value = TMR_ISO180006B_Delimiter1;
          break;
        case 4:
          *(TMR_ISO180006B_Delimiter *)value = TMR_ISO180006B_Delimiter4;
          break;
        default:
          return TMR_ERROR_NOT_FOUND;
        }
        break;
      }
    default:
      return TMR_ERROR_NOT_FOUND;
      }
    }  
#endif /* TMR_ENABLE_ISO180006B */
#endif /* TMR_ENABLE_UHF */

#ifndef TMR_ENABLE_UHF
  if (TMR_TAG_PROTOCOL_ISO14443A == key.protocol)
#else
  else if (TMR_TAG_PROTOCOL_ISO14443A == key.protocol)
#endif /* TMR_ENABLE_UHF */
  {
#ifdef TMR_ENABLE_HF_LF
    uint8_t arr[8];
    switch (key.u.iso14443a)
    {
      case TMR_SR_ISO14443A_CONFIGURATION_SUPPORTED_TAGTYPES:
      case TMR_SR_ISO14443A_CONFIGURATION_TAGTYPE:
        {
          memcpy(arr, msg + 7, msg[1] - 2);
          *(TMR_ISO14443A_TagType *) value =  (TMR_ISO14443A_TagType) TMR_SR_convertFromEBV(arr, (msg[1] - 2));
          break;
        }
      case TMR_SR_ISO14443A_CONFIGURATION_SUPPORTED_FEATURES:
        {
          memcpy(arr, msg + 7, msg[1] - 2);
          *(TMR_SupportedTagFeatures *) value =  (TMR_SupportedTagFeatures) TMR_SR_convertFromEBV(arr, (msg[1] - 2));
          break;
        }
      default:
        return TMR_ERROR_NOT_FOUND;
    }
  }
  else if (TMR_TAG_PROTOCOL_ISO14443B == key.protocol)
  {
    uint8_t arr[8];
    switch (key.u.iso14443b)
    {
      case TMR_SR_ISO14443B_CONFIGURATION_SUPPORTED_TAGTYPES:
      case TMR_SR_ISO14443B_CONFIGURATION_TAGTYPE:
        {
          memcpy(arr, msg + 7, msg[1] - 2);
          *(TMR_ISO14443B_TagType *) value =  (TMR_ISO14443B_TagType) TMR_SR_convertFromEBV(arr, (msg[1] - 2));
          break;
        }
      default:
        return TMR_ERROR_NOT_FOUND;
    }
  }
  else if (TMR_TAG_PROTOCOL_ISO15693 == key.protocol)
  {
    uint8_t arr[8];
    switch (key.u.iso15693)
    {
      case TMR_SR_ISO15693_CONFIGURATION_SUPPORTED_TAGTYPES:
      case TMR_SR_ISO15693_CONFIGURATION_TAGTYPE:
        {
          memcpy(arr, msg + 7, msg[1] - 2);
          *(TMR_ISO15693_TagType *) value = (TMR_ISO15693_TagType) TMR_SR_convertFromEBV(arr, (msg[1] - 2));
          break;
        }
      case TMR_SR_ISO15693_CONFIGURATION_SUPPORTED_FEATURES:
        {
          memcpy(arr, msg + 7, msg[1] - 2);
          *(TMR_SupportedTagFeatures *) value =  (TMR_SupportedTagFeatures) TMR_SR_convertFromEBV(arr, (msg[1] - 2));
          break;
        }
      default:
        return TMR_ERROR_NOT_FOUND;
    }
  }
  else if (TMR_TAG_PROTOCOL_LF125KHZ == key.protocol)
  {
    uint8_t arr[8];
    switch (key.u.lf125khz)
    {
      case TMR_SR_LF125KHZ_CONFIGURATION_SUPPORTED_TAGTYPES:
      case TMR_SR_LF125KHZ_CONFIGURATION_TAGTYPE:
        {
          memcpy(arr, msg + 7, msg[1] - 2);
          *(TMR_LF125KHZ_TagType *) value = (TMR_LF125KHZ_TagType) TMR_SR_convertFromEBV(arr, (msg[1] - 2));
          break;
        }
      case TMR_SR_LF125KHZ_CONFIGURATION_SUPPORTED_FEATURES:
        {
          memcpy(arr, msg + 7, msg[1] - 2);
          *(TMR_SupportedTagFeatures *) value =  (TMR_SupportedTagFeatures) TMR_SR_convertFromEBV(arr, (msg[1] - 2));
          break;
        }
      case TMR_SR_LF125KHZ_CONFIGURATION_SECURE_RD_FORMAT:
        {
          *(uint8_t *)value = (uint8_t)GETU8AT(msg, 7);
          break;
        }
      default:
        return TMR_ERROR_NOT_FOUND;
    }
  }
  else if (TMR_TAG_PROTOCOL_LF134KHZ == key.protocol)
  {
    uint8_t arr[8];
    switch (key.u.lf134khz)
    {
      case TMR_SR_LF134KHZ_CONFIGURATION_SUPPORTED_TAGTYPES:
      case TMR_SR_LF134KHZ_CONFIGURATION_TAGTYPE:
        {
          memcpy(arr, msg + 7, msg[1] - 2);
          *(TMR_LF134KHZ_TagType *) value = (TMR_LF134KHZ_TagType) TMR_SR_convertFromEBV(arr, (msg[1] - 2));
          break;
        }
      default:
        return TMR_ERROR_NOT_FOUND;
    }
#endif /* TMR_ENABLE_HF_LF */
  }
  else
  {
    return TMR_ERROR_INVALID;
  }
  return TMR_SUCCESS;
}

#ifdef TMR_ENABLE_UHF
TMR_Status TMR_iso18000BBLFValToInt(int val, void *lf)
{
  switch (val)
  {
    case 1:
	  *(TMR_ISO180006B_LinkFrequency *)lf = TMR_ISO180006B_LINKFREQUENCY_40KHZ;
	  break;
	case 0:
	  *(TMR_ISO180006B_LinkFrequency *)lf = TMR_ISO180006B_LINKFREQUENCY_160KHZ;
	  break;
	default:
      return TMR_ERROR_NOT_FOUND;
  }
  return TMR_SUCCESS;
}
#endif /* TMR_ENABLE_UHF */

/* Helper function to frame the multiple protocol search command */
TMR_Status 
TMR_SR_msgSetupMultipleProtocolSearch(TMR_Reader *reader, uint8_t *msg, TMR_SR_OpCode op, TMR_TagProtocolList *protocols, TMR_TRD_MetadataFlag metadataFlags, TMR_SR_SearchFlag antennas, TMR_TagFilter **filter, uint16_t timeout)
{
  TMR_Status ret;
  uint8_t i = 0, j = 0;
  uint16_t subTimeout = 0;
  uint32_t asyncOffTime = 0;
  uint16_t searchFlags = 0;

  ret = TMR_SUCCESS;
  i=2;

#if defined(TMR_ENABLE_BACKGROUND_READS)|| defined(SINGLE_THREAD_ASYNC_READ)
  asyncOffTime = reader->readParams.asyncOffTime;
#endif

  SETU8(msg, i, TMR_SR_OPCODE_MULTI_PROTOCOL_TAG_OP);  //Opcode
  if(reader->continuousReading)
  {
    /* Timeout should be zero for true continuous reading */
    SETU16(msg, i, 0);
    SETU8(msg, i, (uint8_t)0x1);//TM Option 1, for continuous reading
  }
  else
  {
    SETU16(msg, i, timeout); //command timeout
    SETU8(msg, i, (uint8_t)0x11);//TM Option, turns on metadata
    SETU16(msg, i, (uint16_t)metadataFlags);
  }

  SETU8(msg, i, (uint8_t)op);//sub command opcode

  if (TMR_READ_PLAN_TYPE_MULTI == reader->readParams.readPlan->type)
  {
    reader->isStopNTags = false;
    /**
    * in case of multi read plan look for the stop N trigger option
    **/
    for (j = 0; j < reader->readParams.readPlan->u.multi.planCount; j++)
    {
      if (reader->readParams.readPlan->u.multi.plans[j]->u.simple.stopOnCount.stopNTriggerStatus)
      {
        reader->numberOfTagsToRead += reader->readParams.readPlan->u.multi.plans[j]->u.simple.stopOnCount.noOfTags;
        reader->isStopNTags = true;
      }
    }
  }
  else
  {
    reader->numberOfTagsToRead = reader->readParams.readPlan->u.simple.stopOnCount.noOfTags;
  }

#ifdef TMR_ENABLE_HF_LF
  if (reader->isProtocolDynamicSwitching)
  {
    searchFlags |= (uint16_t)(TMR_SR_SEARCH_FLAG_DYNAMIC_PROTOCOL_SWITCHING);
  }
#endif /* TMR_ENABLE_HF_LF */

  if (reader->isStopNTags)
  {
    /**
    * True means atleast one sub read plan has the requested for stop N trigger.
    */
    searchFlags |= (uint16_t)(TMR_SR_SEARCH_FLAG_RETURN_ON_N_TAGS);
  }

  //Add search flags.
  SETU16(msg, i, (uint16_t)searchFlags);

  if (reader->isStopNTags)
  {
    //Add the total tag count for stop N trigger.
    SETU32(msg, i, reader->numberOfTagsToRead);
  }

  /**
  * TODO:add the timeout as requested by the user
  **/
  subTimeout =(uint16_t)(timeout/(protocols->len));

  for (j=0;j<protocols->len;j++) // iterate through the protocol search list
  {
    int PLenIdx;

    TMR_TagProtocol subProtocol=protocols->list[j];
    SETU8(msg, i, (uint8_t)(subProtocol)); //protocol ID
    PLenIdx = i;
    SETU8(msg, i, 0); //PLEN

    /**
    * in case of multi readplan and the total weight is not zero,
    * we should use the weight as provided by the user.
    **/
    if (TMR_READ_PLAN_TYPE_MULTI == reader->readParams.readPlan->type)
    {
      if (0 != reader->readParams.readPlan->u.multi.totalWeight)
      {
        subTimeout = (uint16_t)(reader->readParams.readPlan->u.multi.plans[j]->weight * timeout
          / reader->readParams.readPlan->u.multi.totalWeight); 
        reader->subOffTime =  (uint16_t)(reader->readParams.readPlan->u.multi.plans[j]->weight * asyncOffTime
          / reader->readParams.readPlan->u.multi.totalWeight); 
      }
      else
      {
        reader->subOffTime =(uint16_t)(asyncOffTime /(protocols->len));
      }

      /**
      * In case of Multireadplan, check each simple read plan FastSearch option and
      * stop N trigger option.
      */
      reader->triggerRead = reader->readParams.readPlan->u.multi.plans[j]->u.simple.triggerRead.enable;
#ifdef TMR_ENABLE_UHF
      reader->fastSearch = reader->readParams.readPlan->u.multi.plans[j]->u.simple.useFastSearch;
#endif /* TMR_ENABLE_UHF */
      reader->isStopNTags = reader->readParams.readPlan->u.multi.plans[j]->u.simple.stopOnCount.stopNTriggerStatus;
      reader->numberOfTagsToRead = reader->readParams.readPlan->u.multi.plans[j]->u.simple.stopOnCount.noOfTags;
    }

    switch(op)
    {
#if !((!defined(TMR_ENABLE_UHF)) || defined(BARE_METAL))
    case TMR_SR_OPCODE_READ_TAG_ID_SINGLE :
      {
        ret = TMR_SR_msgSetupReadTagSingle(msg, &i, subProtocol,metadataFlags, filter[j], subTimeout);
        break;
      }
#endif /* TMR_ENABLE_HF_LF || BARE_METAL */
    case TMR_SR_OPCODE_READ_TAG_ID_MULTIPLE:
      {
        TMR_ReadPlan *plan;
        /**
        * simple read plan uses this function, only when tagop is NULL,
        * s0, need to check for simple read plan tagop.
        **/
        if (TMR_READ_PLAN_TYPE_MULTI == reader->readParams.readPlan->type)
        {
          plan = reader->readParams.readPlan->u.multi.plans[j];
        }
        else
        {
          plan = reader->readParams.readPlan;
        }
        {
          /* check for the tagop */
          if (NULL != plan->u.simple.tagop)
          {
#ifdef TMR_ENABLE_UHF
            uint32_t readTimeMs = (uint32_t)subTimeout;
            uint8_t lenbyte = 0;
            //add the tagoperation here
            ret = TMR_SR_addTagOp(reader,plan->u.simple.tagop, plan, msg, &i, readTimeMs, &lenbyte);
#endif /* TMR_ENABLE_HF_LF */
          }
          else
          {
            ret = TMR_SR_msgSetupReadTagMultipleWithMetadata(reader, msg, &i, subTimeout, antennas, metadataFlags ,filter[j], subProtocol, 0);
          }
        }
        break;
      }
    default :
      {
        return TMR_ERROR_INVALID_OPCODE;          
      }
    }

    msg[PLenIdx]= i - PLenIdx - 2; //PLEN
    msg[1]=i - 3;
  }
  return ret;
}

TMR_Status TMR_SR_cmdMultipleProtocolSearch(TMR_Reader *reader,TMR_SR_OpCode op,TMR_TagProtocolList *protocols, TMR_TRD_MetadataFlag metadataFlags,TMR_SR_SearchFlag antennas, TMR_TagFilter **filter, uint16_t timeout, uint32_t *tagsFound)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  TMR_SR_SerialReader *sr;

  sr = &reader->u.serialReader;
  *tagsFound = 0 ;

  ret = TMR_SR_msgSetupMultipleProtocolSearch(reader, msg, op, protocols, metadataFlags, antennas, filter, timeout);
  if (ret != TMR_SUCCESS)
  {
    return ret;
  }
#ifndef BARE_METAL
  if (op == TMR_SR_OPCODE_READ_TAG_ID_SINGLE)
  {
    uint8_t opcode;

    sr->opCode = op;
    ret = TMR_SR_sendMessage(reader, msg, &opcode, timeout);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
    sr->tagsRemaining = 1;
  }
#endif /* BARE_METAL */
  if (op == TMR_SR_OPCODE_READ_TAG_ID_MULTIPLE)
  {
    sr->opCode = op;
    if(reader->continuousReading)
    {
      uint8_t opcode;
      ret = TMR_SR_sendMessage(reader, msg, &opcode, timeout);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
	  reader->hasContinuousReadStarted = true;
      sr->tagsRemaining=1;
    }
    else
    {
      ret = TMR_SR_sendTimeout(reader, msg, timeout);
      if (TMR_ERROR_NO_TAGS_FOUND == ret)
      {
        sr->tagsRemaining = *tagsFound = 0;
        return ret;
      }
      else if ((TMR_ERROR_TM_ASSERT_FAILED == ret) || (TMR_ERROR_TIMEOUT == ret))
      {
        return ret;
      }
      else if (TMR_SUCCESS != ret)
      {
        uint16_t remainingTagsCount;
        TMR_Status ret1;

        /* Check for the tag count (in case of module error)*/
        ret1 = TMR_SR_cmdGetTagsRemaining(reader, &remainingTagsCount);
        if (TMR_SUCCESS != ret1)
        {
          return ret1;
        }

        *tagsFound = remainingTagsCount;
        sr->tagsRemaining = *tagsFound;
        return ret;
      }

      *tagsFound = GETU32AT(msg , 9);
      sr->tagsRemaining = *tagsFound;
    }
  }

  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_cmdGetAvailableProtocols(TMR_Reader *reader,
                                TMR_TagProtocolList *protocols)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  ret = TMR_SR_callSendCmd(reader, msg, TMR_SR_OPCODE_GET_AVAILABLE_PROTOCOLS);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  protocols->len = 0;
  for (i = 0; i < msg[1] ; i += 2)
  {
    LISTAPPEND(protocols, (TMR_TagProtocol)GETU16AT(msg, 5 + i)); 
  }

  reader->u.serialReader.versionInfo.protocols = 0x0000;
  for (i = 0 ; i < protocols->len; i ++)
  {
    reader->u.serialReader.versionInfo.protocols |= (1 << (protocols->list[i] - 1)); 
  }
  
  return TMR_SUCCESS;
}


TMR_Status
TMR_SR_cmdGetAvailableRegions(TMR_Reader *reader, TMR_RegionList *regions)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  ret = TMR_SR_callSendCmd(reader, msg, TMR_SR_OPCODE_GET_AVAILABLE_REGIONS);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  regions->len = 0;
  for (i = 0; i < msg[1] ; i++)
  {
    LISTAPPEND(regions, (TMR_Region)GETU8AT(msg, 5 + i)); 
  }

  return TMR_SUCCESS;
}



TMR_Status
TMR_SR_cmdGetTemperature(TMR_Reader *reader, int8_t *temp)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];

  ret = TMR_SR_callSendCmd(reader, msg, TMR_SR_OPCODE_GET_TEMPERATURE);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  *temp = msg[5];
  return TMR_SUCCESS;
}

static TMR_Status
filterbytes(TMR_TagProtocol protocol, const TMR_TagFilter *filter, 
            uint8_t *option, uint8_t *i, uint8_t *msg,
            uint32_t accessPassword, bool usePassword)
{
#ifdef TMR_ENABLE_UHF
  int j;
  if (isSecureAccessEnabled)
  {
    *option = TMR_SR_GEN2_SINGULATION_OPTION_SECURE_READ_DATA;
  }
#endif /* TMR_ENABLE_UHF */

  if (NULL == filter && 0 == accessPassword)
  {
    *option |= 0x00;
    return TMR_SUCCESS;
  }

#ifdef TMR_ENABLE_UHF
  if (TMR_TAG_PROTOCOL_GEN2 == protocol)
  {
    if (usePassword)
    {
      if ((NULL != filter) &&
          (TMR_FILTER_TYPE_GEN2_SELECT == filter->type) &&
          (TMR_GEN2_EPC_LENGTH_FILTER == filter->u.gen2Select.bank))
      {
        /* Ignore access password */
      }
      else
      {
        SETU32(msg, *i, accessPassword);
      }
    }

    if (NULL == filter)
    {
      *option |= TMR_SR_GEN2_SINGULATION_OPTION_USE_PASSWORD;
      if(isMultiSelectEnabled)
      {
        SETU8(msg, *i, SELECT);
        SETU8(msg, *i, ON_N_OFF);
        SETU8(msg, *i, TMR_SR_END_OF_SELECT_INDICATOR);
      }
    }
    else if (TMR_FILTER_TYPE_GEN2_SELECT == filter->type)
    {
      const TMR_GEN2_Select *fp;

      fp = &filter->u.gen2Select;

      if (1 == fp->bank)
      {
        *option |= TMR_SR_GEN2_SINGULATION_OPTION_SELECT_ON_ADDRESSED_EPC;
      }
      else /* select based on the bank */
      {
        *option |= fp->bank;
      }

	  if (fp->bank == TMR_GEN2_EPC_LENGTH_FILTER)
	  {
		  SETU16(msg, *i, fp->maskBitLength);
	  }
	  else
	  {
		  if(true == fp->invert)
		  {
			*option |= TMR_SR_GEN2_SINGULATION_OPTION_INVERSE_SELECT_BIT;
		  }

		  if (fp->maskBitLength > 255)
		  {
			*option |= TMR_SR_GEN2_SINGULATION_OPTION_EXTENDED_DATA_LENGTH;
		  }

		  SETU32(msg, *i, fp->bitPointer);

		  if (fp->maskBitLength > 255)
		  {
			SETU8(msg, *i, (fp->maskBitLength >> 8) & 0xFF);
		  }
		  SETU8(msg, *i, fp->maskBitLength & 0xFF);

		  for(j = 0; j < tm_u8s_per_bits(fp->maskBitLength) ; j++)
		  {
			SETU8(msg, *i, fp->mask[j]);
		  }
      }
      if(isMultiSelectEnabled)
      {
        SETU8(msg, *i, filter->u.gen2Select.target);
        SETU8(msg, *i, filter->u.gen2Select.action);
        SETU8(msg, *i, TMR_SR_END_OF_SELECT_INDICATOR);
      }
    }
    else if (TMR_FILTER_TYPE_TAG_DATA == filter->type)
    {
      const TMR_TagData *fp;
      int bitCount;

      fp = &filter->u.tagData;
      bitCount = fp->epcByteCount * 8;

      /* select on the EPC */
      *option |= 1;
      if (bitCount > 255)
      {
        *option |= TMR_SR_GEN2_SINGULATION_OPTION_EXTENDED_DATA_LENGTH;
        SETU8(msg, *i, (bitCount>>8) & 0xFF);
      }
      SETU8(msg, *i, (bitCount & 0xFF));

      for(j = 0 ; j < fp->epcByteCount ; j++)
      {
        SETU8(msg, *i, fp->epc[j]);
      }
    }
    else if(TMR_FILTER_TYPE_MULTI == filter->type)
    {
      int j;
      uint16_t filterLength, temp;
      filterLength = filter->u.multiFilterList.len;

      if(!isMultiSelectEnabled)
      {
        return TMR_ERROR_UNSUPPORTED;
      }

      for(j = 0; j < filterLength; j++)
      {
        if(filter->u.multiFilterList.tagFilterList[j]->type == TMR_FILTER_TYPE_TAG_DATA)
        {
          return TMR_ERROR_UNSUPPORTED;
        }
      }

      if(filterLength > NUMBER_OF_MULTISELECT_SUPPORTED)
      {
        return TMR_ERROR_UNSUPPORTED;
      }
      else
      {
        temp = 0;
        filterbytes(protocol, filter->u.multiFilterList.tagFilterList[temp++], option, i, msg, accessPassword, false);

        while (temp < filterLength)
        {
          option = &msg[(*i) - 1];
          filterbytes(protocol, filter->u.multiFilterList.tagFilterList[temp++], option, i, msg, accessPassword, false);
        }
      }
    }
    else
    {
      return TMR_ERROR_INVALID;
    }
  }
#ifdef TMR_ENABLE_ISO180006B
  else if (TMR_TAG_PROTOCOL_ISO180006B == protocol)
  {
    if (option)
    {
      *option = 1;
    }
    
    if (NULL == filter)
    {
       /* Set up a match-anything filter, since it isn't the default */
      SETU8(msg, *i, TMR_ISO180006B_SELECT_OP_EQUALS);
      SETU8(msg, *i, 0); /* address */
      SETU8(msg, *i, 0); /* mask - don't compare anything */
      SETU32(msg, *i, 0); /* dummy tag ID bytes 0-3, not compared */
      SETU32(msg, *i, 0); /* dummy tag ID bytes 4-7, not compared */
      return TMR_SUCCESS;
    }
    else if (TMR_FILTER_TYPE_ISO180006B_SELECT == filter->type)
    {
      const TMR_ISO180006B_Select *fp;
      
      fp = &filter->u.iso180006bSelect;
      if (false == fp->invert)
      {
        SETU8(msg, *i, fp->op);
      }
      else
      {
        SETU8(msg, *i, fp->op | 4);
      }
      SETU8(msg, *i, fp->address);
      SETU8(msg, *i, fp->mask);
      for (j = 0 ; j < 8 ; j++)
      {
        SETU8(msg, *i, fp->data[j]);
      }
    }
    else if (TMR_FILTER_TYPE_TAG_DATA == filter->type)
    {
      const TMR_TagData *fp;
      uint8_t mask;

      fp = &filter->u.tagData;

      if (fp->epcByteCount > 8)
      {
        return TMR_ERROR_INVALID;
      }

      /* Convert the byte count to a MSB-based bit mask */
      mask = (0xff00 >> fp->epcByteCount) & 0xff;

      SETU8(msg, *i, TMR_ISO180006B_SELECT_OP_EQUALS);
      SETU8(msg, *i, 0); /* Address - EPC is at the start of memory */
      SETU8(msg, *i, mask);
      for (j = 0 ; j < fp->epcByteCount; j++)
      {
        SETU8(msg, *i, fp->epc[j]);
      }
      for ( ; j < 8 ; j++)
      {
        SETU8(msg, *i, 0); /* EPC data must be 8 bytes */
      }
    }
    else
    {
      return TMR_ERROR_INVALID;
    }
    
  }
#endif /* TMR_ENABLE_ISO180006B */
#endif /* TMR_ENABLE_UHF */

#ifndef TMR_ENABLE_UHF
  if (TMR_FILTER_TYPE_UID_SELECT == filter->type)
#else
  else if (TMR_FILTER_TYPE_UID_SELECT == filter->type)
#endif /* TMR_ENABLE_UHF */
  {
#ifdef TMR_ENABLE_HF_LF
    uint8_t len;
    const TMR_UID_Select *fp;
    fp = &filter->u.uidSelect;

    *option |= TMR_SR_SINGULATION_OPTION_SELECT_ON_UID;

    SETU8(msg, *i, fp->UIDMaskBitLen);
    for (len = 0; len < tm_u8s_per_bits(fp->UIDMaskBitLen); len++)
    {
      SETU8(msg, *i, fp->UIDMask[len]);
    }
    SETU8(msg, *i, 0x00); //End Of Filter
  }
  else if (TMR_FILTER_TYPE_TAGTYPE_SELECT == filter->type)
  {
    TMR_Status ret;
    const TMR_TAGTYPE_Select *fp;
    fp = &filter->u.tagtypeSelect;

    *option |= TMR_SR_SINGULATION_OPTION_SELECT_ON_TAGTYPE;

    ret = TMR_SR_convertToEBV(msg, i, fp->tagType);
    if(TMR_SUCCESS != ret)
    {
      return ret;
    }
    SETU8(msg, *i, 0x00); //End Of Filter
  }
  else if (TMR_FILTER_TYPE_MULTI == filter->type)
  {
    uint16_t filterLength, temp;
    filterLength = filter->u.multiFilterList.len;

    if(filterLength > NUMBER_OF_HF_LF_MULTISELECT_SUPPORTED)
    {
      return TMR_ERROR_UNSUPPORTED;
    }
    else
    {
      temp = 0;
      filterbytes(protocol, filter->u.multiFilterList.tagFilterList[temp++], option, i, msg, accessPassword, false);

      while (temp < filterLength)
      {
        option = &msg[(*i) - 1];
        filterbytes(protocol, filter->u.multiFilterList.tagFilterList[temp++], option, i, msg, accessPassword, false);
      }
    }
#endif /* TMR_ENABLE_HF_LF */
  }
  else
  {
    return TMR_ERROR_INVALID;
  }

  return TMR_SUCCESS;
}
#ifdef TMR_ENABLE_UHF
/** 
 *  Alien Higgs2 and Higgs3 Specific Commands
 **/

/** Helper routine to form the Higgs2 Partial Load Image command **/
void TMR_SR_msgAddHiggs2PartialLoadImage(uint8_t *msg, uint8_t *i, uint16_t timeout,
      TMR_GEN2_Password accessPassword, TMR_GEN2_Password killPassword, uint8_t len, const uint8_t *epc, TMR_TagFilter* target)
{
  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_ALIEN_HIGGS_SILICON);  /* Chip - type*/
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  SETU8(msg, *i, (uint8_t)0x01);  /* Sub command, partial load image */
  SETU32(msg, *i, killPassword);
  SETU32(msg, *i, accessPassword);
  memcpy(&msg[*i], epc, len);
  *i += len;
}
/**
 * Partial Load Image (only 96-bit EPC with no user memory versions)
 * Chip type = 0x01
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to write on the tag
 * @param killPassword The kill password to write on the tag
 * @param len Length of the EPC
 * @param epc The EPC to write to the tag. Maximum of 12 bytes (96 bits)
 * @param target Filter to be applied.
 */
TMR_Status TMR_SR_cmdHiggs2PartialLoadImage(TMR_Reader *reader, uint16_t timeout,
            TMR_GEN2_Password accessPassword, TMR_GEN2_Password killPassword, 
            uint8_t len, const uint8_t epc[], TMR_TagFilter* target)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  if(NULL != target)
  {
    return TMR_ERROR_UNSUPPORTED;
  }
  TMR_SR_msgAddHiggs2PartialLoadImage(msg, &i, timeout, accessPassword, killPassword, len, epc, target);
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}


/** Helper routine to form the Higgs2 Full Load Image command **/
void TMR_SR_msgAddHiggs2FullLoadImage(uint8_t *msg, uint8_t *i, uint16_t timeout,
      TMR_GEN2_Password accessPassword, TMR_GEN2_Password killPassword, uint16_t lockBits, uint16_t pcWord, uint8_t len, const uint8_t *epc, TMR_TagFilter* target)
{
  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_ALIEN_HIGGS_SILICON);  /* Chip - type*/
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  SETU8(msg, *i, (uint8_t)0x03);  /* Sub command, full load image */
  SETU32(msg, *i, killPassword);
  SETU32(msg, *i, accessPassword);
  SETU16(msg, *i, lockBits);
  SETU16(msg, *i, pcWord);
  memcpy(&msg[*i], epc, len);
  *i += len;
}
/**
 * Full Load Image (only 96-bit EPC with no user memory versions)
 * Chip type = 0x01
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to write on the tag
 * @param killPassword The kill password to write on the tag
 * @param lockBits locking the tag according to the Alien Higgs LockBits
 * @param pcWord PC Word in the Tag EPC memBank defined in the Gen2 Specification
 * @param len Length of the EPC
 * @param epc The EPC to write to the tag. Maximum of 12 bytes (96 bits)
 * @param target Filter to be applied.
 */
TMR_Status TMR_SR_cmdHiggs2FullLoadImage(TMR_Reader *reader, uint16_t timeout,
            TMR_GEN2_Password accessPassword, TMR_GEN2_Password killPassword,
            uint16_t lockBits, uint16_t pcWord, uint8_t len,
            const uint8_t epc[], TMR_TagFilter* target)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  if(NULL != target)
  {
    return TMR_ERROR_UNSUPPORTED;
  }
  TMR_SR_msgAddHiggs2FullLoadImage(msg, &i, timeout, accessPassword, killPassword, lockBits, pcWord, len, epc, target);
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}

/** Helper routine to form the Higgs3 Fast Load Image command **/
TMR_Status TMR_SR_msgAddHiggs3FastLoadImage(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password currentAccessPassword,
      TMR_GEN2_Password accessPassword, TMR_GEN2_Password killPassword, uint16_t pcWord, uint8_t len, const uint8_t *epc, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option=0,rec; 

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_ALIEN_HIGGS3_SILICON);  /* Chip - type*/
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }

  rec=*i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)0x01);  /* Sub command, fast load image */
  
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, 0, false); 
  msg[rec]=msg[rec]|option;

  SETU32(msg, *i, currentAccessPassword);
  SETU32(msg, *i, killPassword);
  SETU32(msg, *i, accessPassword);
  SETU16(msg, *i, pcWord);
  memcpy(&msg[*i], epc, len);
  *i += len;

  return ret;
}
/**
 * Higgs3 Fast Load Image (only 96-bit EPC)
 * Chip type = 0x05
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param currentAccessPassword The access password to use to write to the tag
 * @param accessPassword The access password to write on the tag
 * @param killPassword The kill password to write on the tag
 * @param pcWord PC Word in the Tag EPC memBank defined in the Gen2 Specification
 * @param len Length of the EPC
 * @param epc The EPC to write to the tag. Maximum of 12 bytes (96 bits)
 * @param target Filter to be applied.
 */
TMR_Status 
TMR_SR_cmdHiggs3FastLoadImage(TMR_Reader *reader, uint16_t timeout,
            TMR_GEN2_Password currentAccessPassword, TMR_GEN2_Password accessPassword, 
            TMR_GEN2_Password killPassword, uint16_t pcWord, uint8_t len, const uint8_t epc[], TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddHiggs3FastLoadImage(msg, &i, timeout, currentAccessPassword, accessPassword, killPassword, pcWord, len, epc, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}

/** Helper routine to form the Higgs3 Load Image command **/
TMR_Status TMR_SR_msgAddHiggs3LoadImage(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password currentAccessPassword,
      TMR_GEN2_Password accessPassword, TMR_GEN2_Password killPassword, uint16_t pcWord, uint8_t len, const uint8_t *epcAndUserData, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option=0,rec; 

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_ALIEN_HIGGS3_SILICON);  /* Chip - type*/
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }

  rec=*i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)0x03);  /* Sub command, Load image */

  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, 0, false); 
  msg[rec]=msg[rec]|option;

  SETU32(msg, *i, currentAccessPassword);
  SETU32(msg, *i, killPassword);
  SETU32(msg, *i, accessPassword);
  SETU16(msg, *i, pcWord);
  memcpy(&msg[*i], epcAndUserData, len);
  *i += len;

  return ret;
}

/**
 * Higgs3 Load Image 
 * Chip type = 0x05
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param currentAccessPassword The access password to use to write to the tag
 * @param accessPassword The access password to write on the tag
 * @param killPassword The kill password to write on the tag
 * @param pcWord PC Word in the Tag EPC memBank defined in the Gen2 Specification
 * @param len Length of epcAndUserData
 * @param epcAndUserData The EPC and user data to write to the tag. Must be exactly 76 bytes. 
 *                       The pcWord specifies which of this is EPC and which is user data.
 * @param target Filter to be applied.
 */
TMR_Status TMR_SR_cmdHiggs3LoadImage(TMR_Reader *reader, uint16_t timeout,
            TMR_GEN2_Password currentAccessPassword,
            TMR_GEN2_Password accessPassword, TMR_GEN2_Password killPassword,
            uint16_t pcWord, uint8_t len, const uint8_t epcAndUserData[], TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddHiggs3LoadImage(msg, &i, timeout, currentAccessPassword, accessPassword, 
                                killPassword, pcWord, len, epcAndUserData, target);  /* Length of epcAndUserData must be 76 */
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}

/** Helper routine to form the Higgs3 Block Read Lock command**/
TMR_Status TMR_SR_msgAddHiggs3BlockReadLock(uint8_t *msg, uint8_t *i, uint16_t timeout, 
          TMR_GEN2_Password accessPassword, uint8_t lockBits, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option=0,rec; 

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_ALIEN_HIGGS3_SILICON);  /* Chip - type*/
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }

  rec=*i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)0x09);  /* Sub command, Block Read Lock */
  
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, 0, false); 
  msg[rec]=msg[rec]|option;

  SETU32(msg, *i, accessPassword);
  SETU8(msg, *i, lockBits);

  return ret;
}

/**
 * Higgs3 Block Read Lock 
 * Chip type = 0x05
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param lockBits A bitmask of bits to lock. Valid range 0-255
 * @param target Filter to be applied.
 */
TMR_Status TMR_SR_cmdHiggs3BlockReadLock(TMR_Reader *reader, uint16_t timeout,
            TMR_GEN2_Password accessPassword, uint8_t lockBits, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddHiggs3BlockReadLock(msg, &i, timeout, accessPassword, lockBits, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}

/** 
 *  NXP Silicon (G2xL and G2iL) Specific Commands
 *  Chip Type = 0x02 and 0x07
 **/

/** Helper routine to form the NXP Set Read Protect command**/
TMR_Status TMR_SR_msgAddNXPSetReadProtect(uint8_t *msg, uint8_t *i, uint16_t timeout,
                TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option=0,rec; 

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)chip);  /* Chip - type*/
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec=*i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)0x01);  /* Sub command, Set Read Protect */
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, 0, false);
  msg[rec]=msg[rec]|option;
  
  SETU32(msg, *i, accessPassword);
  return ret;
}

/** Helper routine to form the NXP Set Read Protect command**/
TMR_Status TMR_SR_msgAddNXPResetReadProtect(uint8_t *msg, uint8_t *i, uint16_t timeout,
                TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option=0,rec;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)chip);  /* Chip - type*/
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec=*i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)0x02);  /* Sub command, Reset Read Protect */
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, 0, false); 
  msg[rec]=msg[rec]|option;
  
  SETU32(msg, *i, accessPassword);
  return ret;
}

/**
 * NXP Set Read Protect
 * Chip type = 0x02 or 0x07, Command = 0x01
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param chip The NXP Chip type (G2iL or G2xL)
 * @param accessPassword The access password to use to write on the tag
 * @param target Filter to be applied.
 */
TMR_Status TMR_SR_cmdNxpSetReadProtect(TMR_Reader *reader, uint16_t timeout,
            TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;
  
  ret = TMR_SR_msgAddNXPSetReadProtect(msg, &i, timeout, chip, accessPassword, target);  /* set read protect */
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}

/**
 * NXP Reset Read Protect
 * Chip type = 0x02 or 0x07, Command = 0x02
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param chip The NXP Chip type (G2iL or G2xL)
 * @param accessPassword The access password to use to write on the tag
 * @param target Filter to be applied.
 */
TMR_Status TMR_SR_cmdNxpResetReadProtect(TMR_Reader *reader, uint16_t timeout,
            TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddNXPResetReadProtect(msg, &i, timeout, chip, accessPassword, target);  /* Reset read protect */
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}

/** Helper routine to form the NXP Change EAS command**/
TMR_Status TMR_SR_msgAddNXPChangeEAS(uint8_t *msg, uint8_t *i, uint16_t timeout, 
                TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, bool reset, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option=0,rec; 

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)chip);  /* Chip - type*/
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec=*i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)0x03);  /* Sub command, Change EAS */
  
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, 0, false); 
  msg[rec]=msg[rec]|option;
    
  SETU32(msg, *i, accessPassword);  
  if(reset)
  {
    SETU8(msg, *i, (uint8_t)0x02);  /* reset EAS */
  }
  else
  {
    SETU8(msg, *i, (uint8_t)0x01);  /* set EAS */
  }
  return ret;
}

/**
 * NXP Change EAS
 * Chip type = 0x02 or 0x07, Command = 0x03
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param chip The NXP Chip type (G2iL or G2xL)
 * @param accessPassword The access password to use to write on the tag
 * @param reset Reset or set EAS bit
 * @param target Filter to be applied.
 */
TMR_Status TMR_SR_cmdNxpChangeEas(TMR_Reader *reader, uint16_t timeout,
            TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, bool reset, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddNXPChangeEAS(msg, &i, timeout, chip, accessPassword, reset, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}

/** Helper routine to form the NXP EAS Alarm command **/
void TMR_SR_msgAddNXPEASAlarm(uint8_t *msg, uint8_t *i, uint16_t timeout, 
                TMR_SR_GEN2_SiliconType chip, TMR_GEN2_DivideRatio dr, TMR_GEN2_TagEncoding m, TMR_GEN2_TrExt trExt, TMR_TagFilter* target)
{
  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)chip);  /* Chip - type*/
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);  
  SETU8(msg, *i, (uint8_t)0x04);  /* Sub command, EAS Alarm */

  SETU8(msg, *i, (uint8_t)dr);    
  SETU8(msg, *i, (uint8_t)m);
  SETU8(msg, *i, (uint8_t)trExt);
}

/**
 * NXP EAS Alarm
 * Chip type = 0x02 or 0x07, Command = 0x04
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param chip The NXP Chip type (G2iL or G2xL)
 * @param dr Gen2 divide ratio to use
 * @param m Gen2 M(tag encoding) parameter to use
 * @param trExt Gen2 TrExt value to use
 * @param data 8 bytes of EAS alarm data will be returned, on successful operation
 * @param target Filter to be applied.
 */
TMR_Status TMR_SR_cmdNxpEasAlarm(TMR_Reader *reader, uint16_t timeout,
            TMR_SR_GEN2_SiliconType chip, TMR_GEN2_DivideRatio dr, TMR_GEN2_TagEncoding m, TMR_GEN2_TrExt trExt,
            TMR_uint8List *data, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  if (NULL != target)
  {  /* EAS Alarm command is sent without any singulation of the tag*/
    return TMR_ERROR_UNSUPPORTED;
  }
  TMR_SR_msgAddNXPEASAlarm(msg, &i, timeout, chip, dr, m, trExt, target);
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  
  
  /* Alarm data always starts at position 9 */
  /*  FF    0A    2D   00 00    xx        40         00 04            [8 bytes]       ?? ??
   *  SOH Length OpCode Status ChipType  option     SubCommand      [EAS AlarmData]      CRC
   */
  i = 9;
  if (NULL != data)
  {
    uint16_t copyLength;
    
    copyLength = msg[1] + 5 - i;
    memcpy(data->list, &msg[i], copyLength);    
    data->len = copyLength;    
  }

  return TMR_SUCCESS;
}

/** Helper routine to form the NXP Calibrate command **/
TMR_Status TMR_SR_msgAddNXPCalibrate(uint8_t *msg, uint8_t *i, uint16_t timeout, 
                               TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option=0,rec; 

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)chip);  /* Chip - type*/
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }

  rec=*i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)0x05);  /* Sub command, Calibrate */

  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, 0, false); 
  msg[rec]=msg[rec]|option;
  
  SETU32(msg, *i, accessPassword);
  return ret;
}
/**
 * NXP Calibrate (only for G2xL)
 * Chip type = 0x02, Command = 0x05
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param chip The NXP Chip type (G2iL or G2xL)
 * @param accessPassword The access password to use to write on the tag
 * @param data  64 bytes of calibration data will be returned on a successful operation
 * @param target Filter to be applied.
 */
TMR_Status TMR_SR_cmdNxpCalibrate(TMR_Reader *reader, uint16_t timeout,
                                  TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_uint8List *data, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddNXPCalibrate(msg, &i, timeout, chip, accessPassword, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  
  
  /* Calibration data always starts at position 9 */
  /* FF    42     2D    00 00     xx     40       00 05         [64 bytes]     ?? ??
   * SOH Length OpCode Status  ChipType  option  SubCommand   [CalibrateData]     CRC
   */
  i = isMultiSelectEnabled ? 10 : 9;
  if (NULL != data)
  {    
    uint16_t copyLength;
    
    copyLength = msg[1] + 5 - i;
    memcpy(data->list, &msg[i], copyLength);    
    data->len = copyLength;
  }

  return TMR_SUCCESS;
}

/** Helper routine to form the NXP ChangeConfig command **/
TMR_Status TMR_SR_msgAddNXPChangeConfig(uint8_t *msg, uint8_t *i, uint16_t timeout,
        TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_NXP_ConfigWord configWord, TMR_TagFilter* target)
{
  TMR_Status ret;
  //uint16_t configData;
  uint8_t option=0,rec; 

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)chip);  /* Chip - type*/
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec=*i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)0x07);  /* Sub command, ChangeConfig */
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true); 
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)0x00); //RFU  
  SETU16(msg, *i, configWord.data);

  return ret;
}

/**
 * NXP ChangeConfig (only for G2iL)
 * Chip type = 0x07, Command = 0x07
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param chip The NXP Chip type (G2iL or G2xL)
 * @param accessPassword The access password to use to write on the tag
 * @param configWord (I/O)The config word to write on the tag. 
 * @param data If the operation is success, this data contains the current configword setting on the tag.
 * @param target Filter to be applied.
 */
TMR_Status TMR_SR_cmdNxpChangeConfig(TMR_Reader *reader, uint16_t timeout,
            TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_NXP_ConfigWord configWord,
            TMR_uint8List *data, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  if (chip == TMR_SR_GEN2_NXP_G2X_SILICON)
  {
    /* ChangeConfig works only for G2iL tags*/
    return TMR_ERROR_UNSUPPORTED;  
  }
  
  ret = TMR_SR_msgAddNXPChangeConfig(msg, &i, timeout, chip, accessPassword, configWord, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the config data from response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    06     2d    00 00      07     40     00 07         80 46        59 2f
   * SOH Length OpCode Status  ChipType  option  SubCommand  [ConfigData]     CRC
   */
  {
    uint16_t copyLength;
    
    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/** Helper routine to form the NXP UCODE 7 ChangeConfig command **/
TMR_Status TMR_SR_msgAddNXPUCODE7ChangeConfig(uint8_t *msg, uint8_t *i, uint16_t timeout,
        TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_NXP_UCODE7_ConfigWord configWord, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option=0,rec; 

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)chip);  /* Chip - type*/
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec=*i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, TMR_SR_GEN2_UCODE7_CHANGE_CONFIG_COMMAND);  /* Sub command, ChangeConfig */
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true); 
  msg[rec]=msg[rec]|option; 
  SETU16(msg, *i, configWord.data);

  return ret;
}

/**
 * NXP ChangeConfig (only for UCODE 7)
 * Chip type = 0x0E, Command = 0x08
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param chip The NXP Chip type (UCODE7)
 * @param accessPassword The access password to use to write on the tag
 * @param configWord (I/O)The config word to write on the tag. 
 * @param data If the operation is success, this data contains the current configword setting on the tag.
 * @param target Filter to be applied.
 */
TMR_Status TMR_SR_cmdNxpUcode7ChangeConfig(TMR_Reader *reader, uint16_t timeout,
            TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_NXP_UCODE7_ConfigWord configWord,
            TMR_uint8List *data, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;
  
  ret = TMR_SR_msgAddNXPUCODE7ChangeConfig(msg, &i, timeout, chip, accessPassword, configWord, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the config data from response back to the user */
  i = 9;  
  /* FF    06     2d    00 00      0e     40     00 08         00 40        59 2f
   * SOH Length OpCode Status  ChipType  option  SubCommand  [ConfigData]     CRC
   */
  {
    uint16_t copyLength;
    
    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

TMR_Status TMR_SR_msgAddGen2v2NxpUntraceable(uint8_t *msg, uint8_t *i, uint16_t timeout,
									   TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, uint16_t configWord, 
									   TMR_TagOp_GEN2_NXP_Untraceable op ,TMR_TagFilter* target)
{
    TMR_Status ret;
	uint8_t option=0,rec; 
	SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
	SETU16(msg, *i, timeout);
	SETU8(msg, *i, (uint8_t)chip);  /* Chip - type*/
    if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
    {
      SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
    }
	rec=*i;
	SETU8(msg,*i,0x00);//option
	SETU8(msg, *i, (uint8_t)op.subCommand);  /* Sub command, Untraceable */
	ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true); 
	msg[rec]=msg[rec]|option;
	SETU16(msg, *i, configWord);
	if(op.subCommand == 0x02)// Untraceable with Authentication
	{
		SETU8(msg,*i, op.auth.tam1Auth.Authentication);
		SETU8(msg,*i,op.auth.tam1Auth.CSI);
		SETU8(msg,*i,op.auth.tam1Auth.keyID);
		SETU8(msg,*i,op.auth.tam1Auth.KeyLength);	
		{
			uint32_t iWord;
			for (iWord = 0; iWord < op.auth.tam1Auth.Key.len; iWord++)
			{
				SETU8(msg, *i, (op.auth.tam1Auth.Key.list[iWord]));
				// SETU8(msg, *i, ((op.auth.tam1Auth.Key.list[iWord]>>0)&0xFF));
			}
		}
	}
	else //Untraceable with Access 
	{
		SETU32(msg, *i,op.auth.accessPassword);
	}
    return ret;
}

TMR_Status TMR_SR_cmdGen2v2NXPUntraceable(TMR_Reader *reader, uint16_t timeout,
										  TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, uint16_t configWord,
										  TMR_TagOp_GEN2_NXP_Untraceable op,TMR_uint8List *data, TMR_TagFilter* target)
{
	TMR_Status ret;
	uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
	uint8_t i;
	i = 2;

    ret = TMR_SR_msgAddGen2v2NxpUntraceable(msg, &i, timeout, chip, accessPassword, configWord, op ,target);
    if(TMR_SUCCESS != ret)
    {
      return ret;
    }
	msg[1] = i - 3; /* Install length */

	ret = TMR_SR_sendTimeout(reader, msg, timeout);
	if (TMR_SUCCESS != ret)
	{
		return ret;
	}
	return TMR_SUCCESS;
}
uint8_t getCmdProtModeBlockCount(uint8_t protMode, uint8_t blockCount)
{
	/*The UCODE AES Tag only supports ProtMode = "0001b" (CBC encryption) whereas NMV2D Tag supported 
	ProtModes are "0000b"(Plain text encryption),"0001b" (CBC encryption), "0010b" (CMAC encryption),
	"0011b" ((CBC + CMAC) encryption).*/
	int protModeBlockCount = 0x00; 
	uint16_t mask = 0x0F;
	int pos = 4;
	protModeBlockCount |= ((blockCount & mask) << pos);

	pos = 0;
	protModeBlockCount |= (protMode & mask) << pos;
	return protModeBlockCount;
}
uint16_t getCmdProfileOffset(uint16_t profile, uint16_t offset)
{
	uint16_t profileOffset = 0x0000;
	uint16_t mask;
	int pos;

	mask = 0xF;
	pos = 12;
	profileOffset |= ((profile & mask) << pos);

	mask = 0xFFF;
	pos = 0;
	profileOffset |= ((offset & mask) << pos);

	return profileOffset;
}
TMR_Status TMR_SR_msgAddGen2v2NxpAuthenticate(uint8_t *msg, uint8_t *i, uint16_t timeout,TMR_SR_GEN2_SiliconType chip,
										TMR_GEN2_Password accessPassword,TMR_TagOp_GEN2_NXP_Authenticate op ,TMR_TagFilter* target)
{
    TMR_Status ret;
	uint8_t option=0,rec; 
	SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
	SETU16(msg, *i, timeout);
	SETU8(msg, *i, (uint8_t)chip);  /* Chip - type*/
    if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
    {
      SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
    }
	rec=*i;
	SETU8(msg,*i,0x00);//option
	SETU8(msg, *i, (uint8_t)op.subCommand);  /* Sub command, Untraceable */
    ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true); 
	msg[rec]=msg[rec]|option;
	SETU8(msg,*i, op.tam1Auth.Authentication);
	SETU8(msg,*i,op.tam1Auth.CSI);
	SETU8(msg,*i,op.tam1Auth.keyID);
	SETU8(msg,*i,op.tam1Auth.KeyLength);	
	{
		uint8_t iWord;
		for (iWord = 0; iWord < op.tam1Auth.Key.len; iWord++)
		{
			SETU8(msg, *i, (op.tam1Auth.Key.list[iWord]));
		}
	}
	if(op.type == TAM2_AUTHENTICATION)
	{
		SETU16(msg, *i, getCmdProfileOffset(op.tam2Auth.profile,op.tam2Auth.Offset));
		SETU8(msg, *i, getCmdProtModeBlockCount(op.tam2Auth.ProtMode,op.tam2Auth.BlockCount));
	}
    return ret;
}

TMR_Status TMR_SR_cmdGen2v2NXPAuthenticate(TMR_Reader *reader, uint16_t timeout,TMR_SR_GEN2_SiliconType chip,
										   TMR_GEN2_Password accessPassword, TMR_TagOp_GEN2_NXP_Authenticate op, TMR_uint8List *data, TMR_TagFilter* target)
{
	TMR_Status ret;
	uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
	uint8_t i;
	i = 2;

    ret = TMR_SR_msgAddGen2v2NxpAuthenticate(msg, &i, timeout, chip, accessPassword, op ,target);
    if(TMR_SUCCESS != ret)
    {
      return ret;
    }
	msg[1] = i - 3; /* Install length */

	ret = TMR_SR_sendTimeout(reader, msg, timeout);

	if (TMR_SUCCESS != ret)
	{
		return ret;
	}

	if (NULL != data->list)
	{
		i = 8;
        if(isMultiSelectEnabled)
        {
          i++;
        }
        {
			uint16_t dataLength;
			uint16_t copyLength;

			copyLength = dataLength = msg[1] + 5 - i;
			if (copyLength > data->max)
			{
				copyLength = data->max;
			}
			data->len = copyLength;
			memcpy(data->list, &msg[i], copyLength);
			i += dataLength;
		}
	}
	return TMR_SUCCESS;
}
uint16_t getCmdWordPointer(uint16_t wordPointer)
{
	uint16_t rdBufferWordPointer = 0x0000;
	if (wordPointer > 0)
	{
		uint16_t mask = 0x0FFF;
		int pos = 0;
		rdBufferWordPointer |= ((wordPointer & mask) << pos);
	}
	return rdBufferWordPointer;
}
uint16_t getCmdBitCount(uint16_t bitCount)
{
	uint16_t rdBufferBitCount = 0x0000;
	if (bitCount > 0)
	{
		uint16_t mask = 0x0FFF;
		int pos = 0;
		rdBufferBitCount |= ((bitCount & mask) << pos);
	}
	return rdBufferBitCount;
}
TMR_Status TMR_SR_msgAddGen2v2NxpReadBuffer(uint8_t *msg, uint8_t *i, uint16_t timeout,TMR_SR_GEN2_SiliconType chip,
									  TMR_GEN2_Password accessPassword,TMR_TagOp_GEN2_NXP_Readbuffer op ,TMR_TagFilter* target)
{
    TMR_Status ret;
	uint8_t option=0,rec; 
	SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
	SETU16(msg, *i, timeout);
	SETU8(msg, *i, (uint8_t)chip);  /* Chip - type*/
    if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
    {
      SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
    }
	rec=*i;
	SETU8(msg,*i,0x00);//option
	SETU8(msg, *i, (uint8_t)op.authenticate.subCommand);  /* Sub command, Untraceable */
    ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true); 
	msg[rec]=msg[rec]|option;
	SETU16(msg, *i, getCmdWordPointer(op.wordPointer));
	SETU16(msg, *i, getCmdBitCount(op.bitCount));
	SETU8(msg,*i, op.authenticate.tam1Auth.Authentication);
	SETU8(msg,*i,op.authenticate.tam1Auth.CSI);
	SETU8(msg,*i,op.authenticate.tam1Auth.keyID);
	SETU8(msg,*i,op.authenticate.tam1Auth.KeyLength);	
	{
		uint8_t iWord;
		for (iWord = 0; iWord < op.authenticate.tam1Auth.Key.len; iWord++)
		{
			SETU8(msg, *i, (op.authenticate.tam1Auth.Key.list[iWord]));
		}
	}
	if(op.authenticate.type == TAM2_AUTHENTICATION)
	{
		SETU16(msg, *i, getCmdProfileOffset(op.authenticate.tam2Auth.profile,op.authenticate.tam2Auth.Offset));
		SETU8(msg, *i, getCmdProtModeBlockCount(op.authenticate.tam2Auth.ProtMode,op.authenticate.tam2Auth.BlockCount));
	}
    return ret;
}

TMR_Status TMR_SR_cmdGen2v2NXPReadBuffer(TMR_Reader *reader, uint16_t timeout,TMR_SR_GEN2_SiliconType chip,
										 TMR_GEN2_Password accessPassword, TMR_TagOp_GEN2_NXP_Readbuffer op, TMR_uint8List *data, TMR_TagFilter* target)
{
	TMR_Status ret;
	uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
	uint8_t i;
	i = 2;

    ret = TMR_SR_msgAddGen2v2NxpReadBuffer(msg, &i, timeout, chip, accessPassword, op ,target);
    if(TMR_SUCCESS != ret)
    {
      return ret;
    }
	msg[1] = i - 3; /* Install length */

	ret = TMR_SR_sendTimeout(reader, msg, timeout);
	if (TMR_SUCCESS != ret)
	{
		return ret;
	}
	if (NULL != data->list)
	{
		i = 8;
        if(isMultiSelectEnabled)
        {
          i++;
        }
        {
			uint16_t dataLength;
			uint16_t copyLength;

			copyLength = dataLength = msg[1] + 5 - i;
			if (copyLength > data->max)
			{
				copyLength = data->max;
			}
			data->len = copyLength;
			memcpy(data->list, &msg[i], copyLength);
			i += dataLength;
		}
	}
	return TMR_SUCCESS;
}
/** 
 *  Monza4 Silicon Specific Commands
 *  Chip Type = 0x08
 **/

/** Helper routine to form the Monza4 QT Read/Write command **/
TMR_Status TMR_SR_msgAddMonza4QTReadWrite(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                TMR_Monza4_ControlByte controlByte, TMR_Monza4_Payload payload, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option=0,rec; 

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_IMPINJ_MONZA4_SILICON);  /* Chip - type*/
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }

  rec=*i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)0x00);  /* Sub command, QT Read/Write */
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true); 
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, controlByte.data);
  SETU16(msg, *i, payload.data);

  return ret;
}

/**
 * Impinj Monza4 QT Read/Write
 * Chip type = 0x08, command = 0x00
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param controlByte The control byte to write on the tag
 * @param payload The payload 
 * @param data If the operation is success, this data contains the payload
 *             When Read/Write (bit) = 0, then the payload is the value read,
 *             When Read/Write (bit) = 1, then the payload is the value written.
 * @param target Filter to be applied.
 */
TMR_Status TMR_SR_cmdMonza4QTReadWrite(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, 
                                       TMR_Monza4_ControlByte controlByte, TMR_Monza4_Payload payload, TMR_uint8List *data, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddMonza4QTReadWrite(msg, &i, timeout, accessPassword, controlByte, payload, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  
  /* Parse the payload data from response back to the user */
  i = 9;  
  /* FF    06     2d    00 00      08     40     00 00         xx xx        xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand  [payload]      CRC
   */
  {
    uint16_t copyLength;
    
    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;

}

/** Helper routine to form the Monza6 Margin Read command **/
TMR_Status TMR_SR_msgAddMonza6MarginRead(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                   TMR_GEN2_Bank bank, uint32_t bitAddress, uint16_t maskBitLength, uint8_t *mask, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option=0,rec; 
  int j;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, TMR_SR_GEN2_IMPINJ_MONZA6_SILICON);  /* Chip - type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec=*i;
  SETU8(msg, *i, 0x40);  /* option */
  SETU16(msg, *i, TMR_SR_GEN2_MARGIN_READ_COMMAND);  /* Sub command, MarginRead */
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true); 
  msg[rec]=msg[rec]|option; 
  SETU8(msg, *i, bank);
  SETU32(msg, *i, bitAddress);
  SETU8(msg, *i, maskBitLength);
  for(j = 0; j < tm_u8s_per_bits(maskBitLength) ; j++)
  {
    SETU8(msg, *i, mask[j]);
  }
  return ret;
}

/**
 * Impinj Monza6 Margin Read
 * Chip type = 0x0D, command = 0x00
 *  
 * @param reader: The reader
 * @param timeout: The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param chip: The silicon chip type
 * @param accessPassword: The access password to use to write on the tag
 * @param bank: The Gen2 memory bank to read from
 * @param bitAddress: The bit address to start reading from 
 * @param maskBitLength: The number of mask bits
 * @param mask: Pointer to mask data
 * @param target Filter to be applied.
 */
TMR_Status TMR_SR_cmdMonza6MarginRead(TMR_Reader *reader, uint16_t timeout, TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword,
                                      TMR_GEN2_Bank bank, uint32_t bitAddress, uint16_t maskBitLength, uint8_t *mask, TMR_TagFilter *filter)
{
  
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;
  
  ret = TMR_SR_msgAddMonza6MarginRead(msg, &i, timeout, accessPassword, bank, bitAddress, maskBitLength, mask, filter);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout); 
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  return TMR_SUCCESS;
}

/**
 * IDS SL900A specific commands
 * chip type 0A
 */

/**
 * Helper routine to form the IDS SL900A get battery level command
 */
TMR_Status TMR_SR_msgAddIdsSL900aGetBatteryLevel(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                           uint8_t CommandCode, uint32_t password, PasswordLevel level, BatteryType batteryType,
                                           TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_IDS_SL900A_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)level);
  SETU32(msg, *i, password);
  SETU8(msg, *i, batteryType);

  return ret;
}

/**
 * IDS SL900A  Get Battery Level
 * Chip type = 0x0A, command = 0x00 AA
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param password to specify SL900A password access level values
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdSL900aGetBatteryLevel(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                uint8_t CommandCode, uint32_t password, PasswordLevel level,BatteryType type,
                                TMR_uint8List *data, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIdsSL900aGetBatteryLevel(msg, &i, timeout, accessPassword, CommandCode, password, level, type, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the battery level data from response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    06     2d    00 00      0A     42     00 AD         xx xx        xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand    [level]       CRC
   */
  {
    uint16_t copyLength;
    
    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the IDS SL900A get sensor value command
 */
TMR_Status TMR_SR_msgAddIdsSL900aGetSensorValue(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                          uint8_t CommandCode, uint32_t password, PasswordLevel level, Sensor sensortype,
                                          TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_IDS_SL900A_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)level);
  SETU32(msg, *i, password);
  SETU8(msg, *i, sensortype);

  return ret;
}

/**
 * IDS SL900A  Get Sensor value
 * Chip type = 0x0A, command = 0x00 AD
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param password to specify SL900A password access level values
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdSL900aGetSensorValue(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                               uint8_t CommandCode, uint32_t password, PasswordLevel level,Sensor sensortype,
                                TMR_uint8List *data, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIdsSL900aGetSensorValue(msg, &i, timeout, accessPassword, CommandCode, password, level, sensortype, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the sensor type data from response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    06     2d    00 00      0A     42     00 AD         xx xx        xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand    [type]       CRC
   */
  {
    uint16_t copyLength;
    
    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the IDS SL900A get measurment setup command
 */
TMR_Status TMR_SR_msgAddIdsSL900aGetMeasurementSetup(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                          uint8_t CommandCode, uint32_t password, PasswordLevel level, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_IDS_SL900A_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)level);
  SETU32(msg, *i, password);

  return ret;
}

/**
 * IDS SL900A  Get Sensor value
 * Chip type = 0x0A, command = 0x00 A3
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param password to specify SL900A password access level values
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdSL900aGetMeasurementSetup(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                               uint8_t CommandCode, uint32_t password, PasswordLevel level,
                                TMR_uint8List *data, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIdsSL900aGetMeasurementSetup(msg, &i, timeout, accessPassword, CommandCode, password, level, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the measurment data from response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    06     2d    00 00      0A     42     00 AD         xx xx    xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand    [data]   CRC
   */
  {
    uint16_t copyLength;
    
    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the IDS SL900A get log state command
 */
TMR_Status TMR_SR_msgAddIdsSL900aGetLogState(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                       uint8_t CommandCode, uint32_t password, PasswordLevel level, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_IDS_SL900A_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)level);
  SETU32(msg, *i, password);

  return ret;
}

/**
 * IDS SL900A  Get log State
 * Chip type = 0x0A, command = 0x00 AD
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param password to specify SL900A password access level values
 * @param to specify SL900A sensor type
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdSL900aGetLogState(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                            uint8_t CommandCode, uint32_t password, PasswordLevel level,TMR_uint8List *data,
                            TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIdsSL900aGetLogState(msg, &i, timeout, accessPassword, CommandCode, password, level, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the sensor type data from response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    06     2d    00 00      0A     42     00 AD         xx xx        xx xx
  * SOH Length OpCode Status  ChipType  option  SubCommand    [type]       CRC
  */
  {
    uint16_t copyLength;

    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the IDS SL900A set log mode command
 */
TMR_Status TMR_SR_msgAddIdsSL900aSetLogMode(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                      uint8_t CommandCode, uint32_t password, PasswordLevel level, LoggingForm form,
                                      StorageRule rule, bool Ext1Enable, bool Ext2Enable, bool TempEnable, bool BattEnable,
                                      uint16_t LogInterval, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;
  uint32_t logmode = 0;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_IDS_SL900A_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)level);
  SETU32(msg, *i, password);

  logmode |= (uint32_t)form << 21;
  logmode |= (uint32_t)rule << 20;
  logmode |= (uint32_t)(Ext1Enable ? 1 : 0) << 19;
  logmode |= (uint32_t)(Ext2Enable ? 1 : 0) << 18;
  logmode |= (uint32_t)(TempEnable ? 1 : 0) << 17;
  logmode |= (uint32_t)(BattEnable ? 1 : 0) << 16;
  logmode |= (uint32_t)LogInterval << 1;
  SETU8(msg, *i, (uint8_t)((logmode >> 16) & 0xFF));
  SETU8(msg, *i, (uint8_t)((logmode >> 8) & 0xFF));
  SETU8(msg, *i, (uint8_t)((logmode >> 0) & 0xFF));

  return ret;
}

/**
 * IDS SL900A  Get log State
 * Chip type = 0x0A, command = 0x00 AD
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param password to specify SL900A password access level values
 * @param level IDS password level
 * @param form IDS logging form
 * @param rule IDS storage rule
 * @param Ext1Enable to Enable log for EXT1 external sensor
 * @param Ext2Enable to Enable log for EXT2 external sensor
 * @param TempEnable to Enable log for temperature sensor
 * @param BattEnable to Enable log for battery sensor
 * @param LogInterval to Time (seconds) between log readings
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdSL900aSetLogMode(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                           uint8_t CommandCode, uint32_t password, PasswordLevel level, LoggingForm form,
                           StorageRule rule, bool Ext1Enable, bool Ext2Enable, bool TempEnable, bool BattEnable,
                           uint16_t LogInterval, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIdsSL900aSetLogMode(msg, &i, timeout, accessPassword, CommandCode, password, level, form,
    rule, Ext1Enable, Ext2Enable, TempEnable, BattEnable, LogInterval, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}

/* Helper routine to form the IDS SL900A initialize command */
TMR_Status TMR_SR_msgAddIdsSL900aInitialize(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                      uint8_t CommandCode, uint32_t password, PasswordLevel level, uint16_t delayTime,
                                      uint16_t applicatioData, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_IDS_SL900A_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)level);
  SETU32(msg, *i, password);
  SETU16(msg, *i, delayTime);
  SETU16(msg, *i, applicatioData);

  return ret;
}

/**
 * IDS SL900A  initialize command
 * Chip type = 0x0A, command = 0x00 AC
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param password to specify SL900A password access level values
 * @param delayTime to secify delayTime
 * @param applicatioData to specify applicationData
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdSL900aInitialize(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                           uint8_t CommandCode, uint32_t password, PasswordLevel level, uint16_t delayTime,
                           uint16_t applicationData, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIdsSL900aInitialize(msg, &i, timeout, accessPassword, CommandCode, password, level, delayTime,
    applicationData, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}

/**
 *  Helper routine to form the IDS SL900A end log command
 */
TMR_Status TMR_SR_msgAddIdsSL900aEndLog(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                  uint8_t CommandCode, uint32_t password, PasswordLevel level, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_IDS_SL900A_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)level);
  SETU32(msg, *i, password);

  return ret;
}

/**
 * IDS SL900A Initialize command
 * Chip type = 0x0A, command = 0x00 A6
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param password to specify SL900A password access level values
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdSL900aEndLog(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                               uint8_t CommandCode, uint32_t password, PasswordLevel level, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIdsSL900aEndLog(msg, &i, timeout, accessPassword, CommandCode, password, level, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}

/**
 * Helper function to form the IDS SL900A SetPassword command
 */ 
TMR_Status TMR_SR_msgAddIdsSL900aSetPassword(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                       uint8_t CommandCode, uint32_t password, PasswordLevel level, uint32_t newPassword,
                                       PasswordLevel newPasswordLevel, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_IDS_SL900A_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)level);
  SETU32(msg, *i, password);
  SETU8(msg, *i, (uint8_t)newPasswordLevel);
  SETU32(msg, *i, newPassword);

  return ret;
}

/**
 * IDS SL900A SetPassword command
 * Chip type = 0x0A, command = 0x00 A0
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param password to specify SL900A password access level values
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdSL900aSetPassword(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                            uint8_t CommandCode, uint32_t password, PasswordLevel level, uint32_t newPassword,
                            PasswordLevel newPasswordLevel, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIdsSL900aSetPassword(msg, &i, timeout, accessPassword, CommandCode, password, level, newPassword, newPasswordLevel, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}

/**
 *  Helper routine to form the IDS SL900A AccessFifo Status command
 */
TMR_Status TMR_SR_msgAddIdsSL900aAccessFifoStatus(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                  uint8_t CommandCode, uint32_t password, PasswordLevel level, AccessFifoOperation operation,
                                  TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_IDS_SL900A_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)level);
  SETU32(msg, *i, password);
  SETU8(msg, *i, (uint8_t)operation);

  return ret;
}

/**
 * IDS SL900A AccessFifo status command
 * Chip type = 0x0A, command = 0x00 AF
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param password to specify SL900A password access level values
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdSL900aAccessFifoStatus(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t CommandCode,
                                 uint32_t password, PasswordLevel level, AccessFifoOperation operation,TMR_uint8List *data, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIdsSL900aAccessFifoStatus(msg, &i, timeout, accessPassword, CommandCode, password, level, operation, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the sensor type data from response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    06     2d    00 00      0A     42     00 AF         xx xx        xx xx
  * SOH Length OpCode Status  ChipType  option  SubCommand    [type]       CRC
  */
  {
    uint16_t copyLength;

    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the IDS SL900A AccessFifo Read command
 */
TMR_Status TMR_SR_msgAddIdsSL900aAccessFifoRead(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                  uint8_t CommandCode, uint32_t password, PasswordLevel level, AccessFifoOperation operation,
                                  uint8_t length, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_IDS_SL900A_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)level);
  SETU32(msg, *i, password);
  SETU8(msg, *i, (uint8_t)(operation | length));

  return ret;
}

/**
 * IDS SL900A AccessFifo read command
 * Chip type = 0x0A, command = 0x00 AF
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param password to specify SL900A password access level values
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdSL900aAccessFifoRead(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t CommandCode,
                               uint32_t password, PasswordLevel level, AccessFifoOperation operation, uint8_t length, TMR_uint8List *data,
                               TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIdsSL900aAccessFifoRead(msg, &i, timeout, accessPassword, CommandCode, password, level, operation, length, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the sensor type data from response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    06     2d    00 00      0A     42     00 AF         xx xx        xx xx
  * SOH Length OpCode Status  ChipType  option  SubCommand    [type]       CRC
  */
  {
    uint16_t copyLength;

    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the IDS SL900A AccessFifo Write command
 */
TMR_Status TMR_SR_msgAddIdsSL900aAccessFifoWrite(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                  uint8_t CommandCode, uint32_t password, PasswordLevel level, AccessFifoOperation operation,
                                  TMR_uint8List *payLoad, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_IDS_SL900A_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)level);
  SETU32(msg, *i, password);
  SETU8(msg, *i, (uint8_t)(operation | payLoad->len));
  memcpy(&msg[*i], payLoad->list, payLoad->len);
  *i += payLoad->len;

  return ret;
}

/**
 * IDS SL900A AccessFifo Write command
 * Chip type = 0x0A, command = 0x00 AF
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param password to specify SL900A password access level values
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdSL900aAccessFifoWrite(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t CommandCode,
                                uint32_t password, PasswordLevel level, AccessFifoOperation operation, TMR_uint8List *payLoad, TMR_uint8List *data,
                                TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIdsSL900aAccessFifoWrite(msg, &i, timeout, accessPassword, CommandCode, password, level, operation, payLoad, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the sensor type data from response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    06     2d    00 00      0A     42     00 AF         xx xx        xx xx
  * SOH Length OpCode Status  ChipType  option  SubCommand    [type]       CRC
  */
  {
    uint16_t copyLength;

    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the IDS SL900A Start Log command
 */
TMR_Status TMR_SR_msgAddIdsSL900aStartLog(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                    uint8_t CommandCode, uint32_t password, PasswordLevel level, uint32_t time, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_IDS_SL900A_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)level);
  SETU32(msg, *i, password);
  SETU32(msg, *i, time);

  return ret;
}

/**
 * IDS SL900A StartLog command
 * Chip type = 0x0A, command = 0x00 A7
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param password to specify SL900A password access level values
 * @param startTime to specify count start time
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdSL900aStartLog(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t CommandCode,
                         uint32_t password, PasswordLevel level, uint32_t time, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIdsSL900aStartLog(msg, &i, timeout, accessPassword, CommandCode, password, level, time, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}

/**
 *  Helper routine to form the IDS SL900A get calibration data  command
 */
TMR_Status TMR_SR_msgAddIdsSL900aGetCalibrationData(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                          uint8_t CommandCode, uint32_t password, PasswordLevel level, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_IDS_SL900A_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)level);
  SETU32(msg, *i, password);

  return ret;
}

/**
 * IDS SL900A  Get Calibration Data
 * Chip type = 0x0A, command = 0x00 A9
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param password to specify SL900A password access level values
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdSL900aGetCalibrationData(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                               uint8_t CommandCode, uint32_t password, PasswordLevel level,
                                TMR_uint8List *data, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIdsSL900aGetCalibrationData(msg, &i, timeout, accessPassword, CommandCode, password, level, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the  calibration  data from response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    06     2d    00 00      0A     42     00 A9         xx xx        xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand    [data]       CRC
   */
  {
    uint16_t copyLength;

    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the IDS SL900A set calibration data  command
 */
TMR_Status TMR_SR_msgAddIdsSL900aSetCalibrationData(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                          uint8_t CommandCode, uint32_t password, PasswordLevel level, uint64_t calibration, 
                                          TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;
  uint8_t calBytes[8];

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_IDS_SL900A_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)level);
  SETU32(msg, *i, password);

  calBytes[0] = (uint8_t) (calibration >> 56);
  calBytes[1] = (uint8_t) (calibration >> 48);
  calBytes[2] = (uint8_t) (calibration >> 40);
  calBytes[3] = (uint8_t) (calibration >> 32);
  calBytes[4] = (uint8_t) (calibration >> 24);
  calBytes[5] = (uint8_t) (calibration >> 16);
  calBytes[6] = (uint8_t) (calibration >> 8);
  calBytes[7] = (uint8_t) (calibration >> 0);

  memcpy(&msg[*i], calBytes + 1, 7);
  *i += 7;

  return ret;
}

/**
 * IDS SL900A  Set Calibration Data
 * Chip type = 0x0A, command = 0x00 A5
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param password to specify SL900A password access level values
 * @param calibration to spcify the calibration data
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdSL900aSetCalibrationData(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, 
    uint8_t CommandCode, uint32_t password, PasswordLevel level, uint64_t calibration,
    TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIdsSL900aSetCalibrationData(msg, &i, timeout, accessPassword, CommandCode, password, level, calibration, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}

/**
 *  Helper routine to form the IDS SL900A set sfe parameters command
 */
TMR_Status TMR_SR_msgAddIdsSL900aSetSfeParameters(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                          uint8_t CommandCode, uint32_t password, PasswordLevel level, uint16_t sfe,
                                          TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_IDS_SL900A_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)level);
  SETU32(msg, *i, password);
  SETU16(msg, *i, sfe);

  return ret;
}

/**
 * IDS SL900A  Set Calibration Data
 * Chip type = 0x0A, command = 0x00 A4
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param password to specify SL900A password access level values
 * @param sfe to spcify the sfe parameters
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdSL900aSetSfeParameters(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, 
    uint8_t CommandCode, uint32_t password, PasswordLevel level, uint16_t sfe,
    TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIdsSL900aSetSfeParameters(msg, &i, timeout, accessPassword, CommandCode, password, level, sfe, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}

/**
 * Helper routine to form the IDS Sl900A set log mode command
 */
TMR_Status TMR_SR_msgAddIdsSL900aSetLogLimit(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                       uint8_t CommandCode, uint32_t password, PasswordLevel level, uint16_t exLower,
                                       uint16_t lower, uint16_t upper, uint16_t exUpper, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;
  uint64_t temp = 0;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_IDS_SL900A_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)level);
  SETU32(msg, *i, password);

  temp |= (uint64_t)exLower << 30;
  temp |= (uint64_t)lower << 20;
  temp |= (uint64_t)upper << 10;
  temp |= (uint64_t)exUpper << 0;

  SETU8(msg, *i, (uint8_t)((temp >> 32) & 0xFF));
  SETU8(msg, *i, (uint8_t)((temp >> 24) & 0xFF));
  SETU8(msg, *i, (uint8_t)((temp >> 16) & 0xFF));
  SETU8(msg, *i, (uint8_t)((temp >> 8) & 0xFF));
  SETU8(msg, *i, (uint8_t)((temp >> 0) & 0xFF));

  return ret;
}

/**
 * IDS SL90A SetLogLimit
 * Chip type = 0x0A, command = 0x00 A2
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in millisecondds. Valid range 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param password to specify SL900A password access level values
 * @param level IDS password level
 * @param exLoweer IDS extreamLower limit
 * @param lower IDS lower limit
 * @param upper IDS upper limit
 * @param exUpper IDS extreamUpper limit
 */
TMR_Status
TMR_SR_cmdSL900aSetLogLimit(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                            uint8_t CommandCode, uint32_t password, PasswordLevel level, uint16_t exLower,
                            uint16_t lower, uint16_t upper, uint16_t exUpper, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIdsSL900aSetLogLimit(msg, &i, timeout, accessPassword, CommandCode, password, level, exLower,
      lower, upper, exUpper, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}

/**
 *  Helper routine to form the IDS SL900A SetShelfLife command
 */
TMR_Status TMR_SR_msgAddIdsSL900aSetShelfLife(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                        uint8_t CommandCode, uint32_t password, PasswordLevel level, uint32_t block0, uint32_t block1,
                                        TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_IDS_SL900A_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)level);
  SETU32(msg, *i, password);
  SETU32(msg, *i, block0);
  SETU32(msg, *i, block1);

  return ret;
}

/**
 * IDS SL900A  SetShelfLife
 * Chip type = 0x0A, command = 0x00 AB
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param password to specify SL900A password access level values
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdSL900aSetShelfLife(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                             uint8_t CommandCode, uint32_t password, PasswordLevel level, uint32_t block0, uint32_t block1,
                             TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIdsSL900aSetShelfLife(msg, &i, timeout, accessPassword, CommandCode, password, level, block0, block1, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  return TMR_SR_sendTimeout(reader, msg, timeout);
}

/** 
 *  Gen2 IAVDenatran Custom Commands
 *  Chip Type = 0x0B
 **/
 
/** Helper routine to form the Gen2 IAVDenatran Custom commands **/
TMR_Status TMR_SR_msgAddIAVDenatranCustomOp(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                              uint8_t mode, uint8_t payload, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option=0,rec; 

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_DENATRAN_IAV_SILICON);  /* Chip - type*/
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec=*i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)mode);  /* Sub command */
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true); 
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, (uint8_t)payload);

  return ret;
}

 /**
 * IAV Denatran Custom Commands
 * Chip type = 0x0B, command = 0x00
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdIAVDenatranCustomOp(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t mode, uint8_t payload,
                      TMR_uint8List *data, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIAVDenatranCustomOp(msg, &i, timeout, accessPassword, mode, payload, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the  response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    06     2d    00 00      08     40     00 00         xx xx        xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand  [payload]      CRC
   */
  {
    uint16_t copyLength;

    copyLength = msg[1] + 5 - i;

    if (copyLength > data->max)
    {
      copyLength = data->max;
    }
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/** Helper routine to form the Gen2 IAVDenatran Custom commad readFromMemMap */
TMR_Status TMR_SR_msgAddIAVDenatranCustomReadFromMemMap(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
    uint8_t mode, uint8_t payload, TMR_TagFilter* target, uint16_t wordAddress)
{
  TMR_Status ret;

  ret = TMR_SR_msgAddIAVDenatranCustomOp(msg, i, timeout, accessPassword, mode, payload, target);
  /* Add the address to be read from user bank */
  SETU16(msg, *i, wordAddress);

  return ret;
}

/** * IAV Denatran Custom Command Read From MemMap
 * Chip type = 0x0B, command = 0x06
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param target Filter to be applied.
 * @param wordAddress address to be read from USER bank
 */
TMR_Status
TMR_SR_cmdIAVDenatranCustomReadFromMemMap(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t mode, uint8_t payload,
                      TMR_uint8List *data, TMR_TagFilter* target, uint16_t wordAddress)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIAVDenatranCustomReadFromMemMap(msg, &i, timeout, accessPassword, mode, payload, target, wordAddress);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the  response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    06     2d    00 00      08     40     00 00         xx xx        xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand  [payload]      CRC
   */
  {
    uint16_t copyLength;

    copyLength = msg[1] + 5 - i;

    if (copyLength > data->max)
    {
      copyLength = data->max;
    }
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/** Helper routine to form the Gen2 IAVDenatran Custom commad read sec */
TMR_Status TMR_SR_msgAddIAVDenatranCustomReadSec(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
    uint8_t mode, uint8_t payload, TMR_TagFilter* target, uint16_t wordAddress)
{
  TMR_Status ret;

  ret = TMR_SR_msgAddIAVDenatranCustomOp(msg, i, timeout, accessPassword, mode, payload, target);
  /* Add the address to be read from user bank */
  SETU16(msg, *i, wordAddress);
  return ret;
}

/** * IAV Denatran Custom Command Read Sec
 * Chip type = 0x0B, command = 0x0A
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param target Filter to be applied.
 * @param wordAddress address to be read from USER bank
 */
TMR_Status
TMR_SR_cmdIAVDenatranCustomReadSec(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t mode, uint8_t payload,
                      TMR_uint8List *data, TMR_TagFilter* target, uint16_t wordAddress)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIAVDenatranCustomReadSec(msg, &i, timeout, accessPassword, mode, payload, target, wordAddress);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the  response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    06     2d    00 00      08     40     00 00         xx xx        xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand  [payload]      CRC
   */
  {
    uint16_t copyLength;

    copyLength = msg[1] + 5 - i;

    if (copyLength > data->max)
    {
      copyLength = data->max;
    }
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/** Helper routine to form the Gen2 IAVDenatran Custom commad Activate Siniav Mode */
TMR_Status TMR_SR_msgAddIAVDenatranCustomActivateSiniavMode(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                                      uint8_t mode, uint8_t payload, TMR_TagFilter* target, bool tokenDesc, uint8_t *token)
{
  TMR_Status ret;
  uint8_t j;

  ret = TMR_SR_msgAddIAVDenatranCustomOp(msg, i, timeout, accessPassword, mode, payload, target);
  /* add the token field */
  if (tokenDesc)
  {
    for (j = 0; j < 8; j++)
    {
      SETU8(msg, *i, token[j]);
    }
  }
  return ret;
}

 /**
 * IAV Denatran Custom Commands Activate Siniav Mode
 * Chip type = 0x0B, command = 0x02
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param target Filter to be applied.
 * @param token 64bit value to active the tag
 */
TMR_Status
TMR_SR_cmdIAVDenatranCustomActivateSiniavMode(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t mode, uint8_t payload,
                              TMR_uint8List *data, TMR_TagFilter* target, bool tokenDesc, uint8_t *token)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIAVDenatranCustomActivateSiniavMode(msg, &i, timeout, accessPassword, mode, payload, target, tokenDesc, token);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the  response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    06     2d    00 00      08     40     00 00         xx xx        xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand  [payload]      CRC
   */
  {
    uint16_t copyLength;

    copyLength = msg[1] + 5 - i;

    if (copyLength > data->max)
    {
      copyLength = data->max;
    }
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/** Helper routine to form the Gen2 IAVDenatran Custom commad writeToMemMap */
TMR_Status TMR_SR_msgAddIAVDenatranCustomWriteToMemMap(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
    uint8_t mode, uint8_t payload, TMR_TagFilter* target, uint16_t wordPtr, uint16_t wordData, uint8_t* tagId, uint8_t* dataBuf)
{
  TMR_Status ret;
  uint8_t j;

  ret = TMR_SR_msgAddIAVDenatranCustomOp(msg, i, timeout, accessPassword, mode, payload, target);
  /* Add the address to be read from user bank */
  SETU16(msg, *i, wordPtr);
  SETU16(msg, *i, wordData);
  for (j = 0; j < 8; j++)
  {
    SETU8(msg, *i, tagId[j]);
  }
  for (j = 0; j < 16; j++)
  {
    SETU8(msg, *i, dataBuf[j]);
  }
  return ret;
}

/** Helper routine to form the Gne2 IAVDenatran Custom command get token Id */
TMR_Status TMR_SR_msgAddIAVDenatranCustomGetTokenId(uint8_t *msg, uint8_t *i, uint16_t timeout, 
    TMR_GEN2_Password accessPassword, uint8_t mode, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0,rec; 

  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_DENATRAN_IAV_SILICON);  /* Chip - type*/
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec=*i;
  SETU8(msg,*i,0x40);//option
  SETU8(msg, *i, (uint8_t)0x00);
  SETU8(msg, *i, (uint8_t)mode);  /* Sub command */
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true); 
  msg[rec]=msg[rec]|option;

  return ret;
}

/**
 * IAV Denatran Custom Command write to MemMap
 * Chip type = 0x0B, command = 0x06
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param target Filter to be applied.
 * @param wordPtr pointer to the USER data
 * @param wordData data to be written
 * @param dataBuf credentials written word
 */
TMR_Status
TMR_SR_cmdIAVDenatranCustomWriteToMemMap(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t mode, uint8_t payload,
                      TMR_uint8List *data, TMR_TagFilter* target, uint16_t wordPtr, uint16_t wordData, uint8_t* tagId, uint8_t* dataBuf)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIAVDenatranCustomWriteToMemMap(msg, &i, timeout, accessPassword, mode, payload, target, wordPtr, wordData, tagId, dataBuf);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the  response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    06     2d    00 00      08     40     00 00         xx xx        xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand  [payload]      CRC
   */
  {
    uint16_t copyLength;

    copyLength = msg[1] + 5 - i;

    if (copyLength > data->max)
    {
      copyLength = data->max;
    }
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/** Helper routine to form the Gen2 IAVDenatran Custom commad write Sec */
TMR_Status TMR_SR_msgAddIAVDenatranCustomWriteSec(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
    uint8_t mode, uint8_t payload, TMR_TagFilter* target, uint8_t* data, uint8_t* dataBuf)
{
  TMR_Status ret;
  uint8_t j;

  ret = TMR_SR_msgAddIAVDenatranCustomOp(msg, i, timeout, accessPassword, mode, payload, target);

  for (j = 0; j < 6; j++)
  {
    SETU8(msg, *i, data[j]);
  }
  for (j = 0; j < 16; j++)
  {
    SETU8(msg, *i, dataBuf[j]);
  }
  return ret;
}

/**
 * IAV Denatran Custom Command write sec
 * Chip type = 0x0B, command = 0x0B
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param target Filter to be applied.
 * @param wordPtr pointer to the USER data
 * @param data data words
 * @param dataBuf credentials written word
 */
TMR_Status
TMR_SR_cmdIAVDenatranCustomWriteSec(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t mode, uint8_t payload,
                      TMR_uint8List *data, TMR_TagFilter* target, uint8_t* dataWords, uint8_t* dataBuf)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIAVDenatranCustomWriteSec(msg, &i, timeout, accessPassword, mode, payload, target, dataWords, dataBuf);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the  response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    06     2d    00 00      08     40     00 00         xx xx        xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand  [payload]      CRC
   */
  {
    uint16_t copyLength;

    copyLength = msg[1] + 5 - i;

    if (copyLength > data->max)
    {
      copyLength = data->max;
    }
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 * IAV Denatran Custom Command Get Token Id
 * Chip type = 0x0B, command = 0x09
 *  
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag 
 * @param target Filter to be applied.
 */ 
TMR_Status
TMR_SR_cmdIAVDenatranCustomGetTokenId(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t mode, 
                      TMR_uint8List *data, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddIAVDenatranCustomGetTokenId(msg, &i, timeout, accessPassword, mode, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the  response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    06     2d    00 00      08     40     00 00         xx xx        xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand  [payload]      CRC
   */
  {
    uint16_t copyLength;

    copyLength = msg[1] + 5 - i;

    if (copyLength > data->max)
    {
      copyLength = data->max;
    }
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 * TMR_SR_cmdGetReaderStatistics
 * Get the current per-port statistics.
 *
 * @param reader [in]The reader
 * @param statFlags [in]The set of statistics together
 * @param stats [out]The ReaderStatistics structure populated with requested per-port values
 */
TMR_Status
TMR_SR_cmdGetReaderStatistics(TMR_Reader *reader, TMR_SR_ReaderStatisticsFlag statFlags,
                                         TMR_SR_ReaderStatistics *stats)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i, length, offset;
  i = 2;

  SETU8(msg, i, TMR_SR_OPCODE_GET_READER_STATS);
  SETU8(msg, i, (uint8_t)TMR_SR_READER_STATS_OPTION_GET_PER_PORT); /* Option byte */
  SETU8(msg, i, (uint8_t)statFlags);

  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, reader->u.serialReader.commandTimeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the statistics from response */
  if (NULL != stats)
  {
    uint8_t  port = 0;
    offset = 7;

    length = reader->u.serialReader.txRxMap->len;

    while (offset < (msg[1] + 2))
    {
      if (0 != (msg[offset] & TMR_SR_READER_STATS_FLAG_RF_ON_TIME))
      {
        offset += 2;

        for (i = 0; i < length; i++)
        {
          port = msg[offset];
          if (i == (port-1))
          {
            offset ++;
            stats->rfOnTime[i] = GETU32(msg, offset);
          }
          else
          {
            stats->rfOnTime[i] = 0;
          }
        }
        stats->numPorts = length;
      }
      else if (0 != (msg[offset] & TMR_SR_READER_STATS_FLAG_NOISE_FLOOR))
      {
        offset += 2;

        for (i = 0; i < length; i++)
        {
          port = msg[offset];
          if (i == (port-1))
          {
            offset ++;
            stats->noiseFloor[i] = msg[offset++];
          }
          else
          {
            stats->noiseFloor[i] = 0;
          }
        }
        stats->numPorts = length;
      }
      else if (0 != (msg[offset] & TMR_SR_READER_STATS_FLAG_NOISE_FLOOR_TX_ON))
      {
        offset += 2;

        for (i = 0; i < length; i++)
        {
          port = msg[offset];
          if (i == (port-1))
          {
            offset ++;
            stats->noiseFloorTxOn[i] = msg[offset++];
          }
          else
          {
            stats->noiseFloorTxOn[i] = 0;
          }
        }
        stats->numPorts = length;
      }
    }
  }
  return TMR_SUCCESS;
}

/**
 * TMR_SR_cmdResetReaderStatistics
 * Reset the per-port statistics.
 *
 * @param reader [in]The reader
 * @param statFlags [in]The set of statistics to reset. Only the RF on time statistic may be reset.
 */
TMR_Status
TMR_SR_cmdResetReaderStatistics(TMR_Reader *reader, TMR_SR_ReaderStatisticsFlag statFlags)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  SETU8(msg, i, TMR_SR_OPCODE_GET_READER_STATS);
  SETU8(msg, i, (uint8_t)TMR_SR_READER_STATS_OPTION_RESET); /* Option byte */
  SETU8(msg, i, (uint8_t)statFlags);

  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, reader->u.serialReader.commandTimeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  return TMR_SUCCESS;
}
#endif /* TMR_ENABLE_UHF */

/**
 *  Helper function to Parse the received reader stats
 */
void
TMR_parseTagStats(TMR_Reader* reader, TMR_Reader_StatsValues* stats, 
                    uint8_t* msg, uint8_t offset)
{
  uint16_t flags = 0;
#ifdef TMR_ENABLE_UHF 
  uint8_t i, j;

  if (isMultiSelectEnabled)
  {
    offset += 1;
  }
#endif /* TMR_ENABLE_UHF  */

  flags = reader->statsFlag;
 
  /* Get stats content flags */
  if ((0x80) > reader->statsFlag)
  {
    offset += 1;
  }
  else if ((0x4000) > reader->statsFlag)
  {
    offset += 2;
  }
  else
  {
    offset += 3;
  }

#ifdef TMR_ENABLE_UHF
  /**
   * preinitialize the rf ontime and the noise floor value to zero
   * berfore getting the reader stats
   */
  for (i = 0; i < stats->perAntenna.max; i++)
  {
    stats->perAntenna.list[i].antenna = 0;
    stats->perAntenna.list[i].rfOnTime = 0;
    stats->perAntenna.list[i].noiseFloor = 0;
  }
#endif /* TMR_ENABLE_UHF */

  TMR_fillReaderStats(reader, stats, flags, msg, offset);
#ifdef TMR_ENABLE_UHF
  /**
   * iterate through the per antenna values,
   * If found  any 0-antenna rows, copy the
   * later rows down to compact out the empty space.
   */
  for (i = 0; i < reader->u.serialReader.txRxMap->len; i++)
  {
    if (!stats->perAntenna.list[i].antenna)
    {
      for (j = i + 1; j < reader->u.serialReader.txRxMap->len; j++)
      {
        if (stats->perAntenna.list[j].antenna)
        {
          stats->perAntenna.list[i].antenna = stats->perAntenna.list[j].antenna;
          stats->perAntenna.list[i].rfOnTime = stats->perAntenna.list[j].rfOnTime;
          stats->perAntenna.list[i].noiseFloor = stats->perAntenna.list[j].noiseFloor;
          stats->perAntenna.list[j].antenna = 0;
          stats->perAntenna.list[j].rfOnTime = 0;
          stats->perAntenna.list[j].noiseFloor = 0;

          stats->perAntenna.len++;
          break;
        }
      }
    }
    else
    {
      /* Increment the length */
      stats->perAntenna.len++;
    }
  }
#endif /* TMR_ENABLE_UHF */
  /* store the requested flags for future use */
  stats->valid = reader->statsFlag;
}

/**
 * Helper function to be used in GetReaderStats
 */ 
TMR_Status
TMR_fillReaderStats(TMR_Reader *reader, TMR_Reader_StatsValues* stats, uint16_t flag, uint8_t* msg, uint8_t offset)
{
  uint8_t idx = 0;
#if ENABLE_TMR_DEBUG != 0
#define TMR_FRS_DEBUG_MSG(label, offset, len) {\
    int i;\
    fprintf(stderr, "%s:%d: msg[%d] =", __FILE__, __LINE__, offset);\
    for (i=0; i<len; i++) {\
      fprintf(stderr, " %02X", msg[offset+i]); \
    }\
    fprintf(stderr, " (%s)", label);\
    fprintf(stderr, "\n");\
  }
#else
#define TMR_FRS_DEBUG_MSG(label, offset, len) {}
#endif

#ifdef TMR_ENABLE_UHF
  uint8_t i;
#endif /* TMR_ENABLE_UHF */

#if ENABLE_TMR_DEBUG != 0
  {
    int i;
    fprintf(stderr, "statsMessage:");
    for (i=0; i<msg[1] + 2 + 2; i++)
    {
      fprintf(stderr, " %02X", msg[i]);
    }
    fprintf(stderr, "\n");
  }
#endif

  while (offset < (msg[1] + 2))
  {
    uint8_t len = 1, ebvValue[8];

    TMR_DEBUG("[offset]=[%d]", offset);

    /* Stats flag are in EBV format. Hence the number of bytes are not fixed 
     *  Number of bytes to extract depends on the MSB of the retrieved byte.
     */
    idx = offset;
    len = parseEBVdata(msg, ebvValue, &idx);
    flag = (uint16_t)TMR_SR_convertFromEBV(ebvValue, len);

    TMR_DEBUG("flag=%04X", flag);
#ifdef TMR_ENABLE_UHF
    if (flag & TMR_READER_STATS_FLAG_RF_ON_TIME)
    {
      uint8_t len;
      uint8_t end;
      uint8_t tx;
      uint32_t value;
      
      offset += 1; // skip flag field (TODO: Refactor into flag parser)
      len = GETU8(msg, offset);
      end = offset+len;

      while (offset < end)
      {
	TMR_FRS_DEBUG_MSG("rfOnTime.tx", offset, 1);
	tx = GETU8(msg, offset);
	TMR_FRS_DEBUG_MSG("rfOnTime.value", offset, 4);
	value = GETU32(msg, offset);
	for (i = 0; i < reader->u.serialReader.txRxMap->len; i++)
	{
	  if (tx == reader->u.serialReader.txRxMap->list[i].txPort)
	  {
	    stats->perAntenna.list[i].antenna = reader->u.serialReader.txRxMap->list[i].antenna;
	    TMR_DEBUG("rfOnTime.antenna=%d", stats->perAntenna.list[i].antenna);
	    stats->perAntenna.list[i].rfOnTime = value;
	  }
	}
      }
    }

    else if (flag & TMR_READER_STATS_FLAG_NOISE_FLOOR_SEARCH_RX_TX_WITH_TX_ON)
    {
      uint8_t tx,rx, len;

      offset++;
      len = GETU8(msg, offset);
      TMR_FRS_DEBUG_MSG("tx,rx,noisefloor", offset, len*3);
      while (len)
      {
        tx = GETU8(msg, offset);
        rx = GETU8(msg, offset);
        for (i = 0; i < reader->u.serialReader.txRxMap->len; i++)
        {
          if ((rx == reader->u.serialReader.txRxMap->list[i].rxPort) && (tx == reader->u.serialReader.txRxMap->list[i].txPort))
          {
            stats->perAntenna.list[i].antenna = reader->u.serialReader.txRxMap->list[i].antenna;
            stats->perAntenna.list[i].noiseFloor = GETU8(msg, offset);
            len -= 3;
          }
        }
      }
    }

    else if (flag & TMR_READER_STATS_FLAG_FREQUENCY)
    {
      offset += 3;
      TMR_FRS_DEBUG_MSG("frequency", offset, 3);
      stats->frequency = GETU24(msg, offset);
    }

    else if (flag & TMR_READER_STATS_FLAG_TEMPERATURE)
#else
    if (flag & TMR_READER_STATS_FLAG_TEMPERATURE)
#endif /* TMR_ENABLE_UHF */
    {
      offset += 3;
      TMR_FRS_DEBUG_MSG("temperature", offset, 1);
      stats->temperature = GETU8(msg, offset);
    }
#ifdef TMR_ENABLE_UHF
    else if (flag & TMR_READER_STATS_FLAG_PROTOCOL)
    {
      offset += 3;
      TMR_FRS_DEBUG_MSG("protocol", offset, 1);
      stats->protocol = (TMR_TagProtocol)GETU8(msg, offset);
    }

    else if (flag & TMR_READER_STATS_FLAG_ANTENNA_PORTS)
    {
      uint8_t tx, rx;

      offset += 3;      
      TMR_FRS_DEBUG_MSG("antenna(tx,rx)", offset, 2);
      tx = GETU8(msg, offset);
      rx = GETU8(msg, offset);

      TMR_DEBUG("pre-value: stats->antenna = %d", stats->antenna);
      for (i = 0; i < reader->u.serialReader.txRxMap->len; i++)
      {
        if ((rx == reader->u.serialReader.txRxMap->list[i].rxPort) && (tx == reader->u.serialReader.txRxMap->list[i].txPort))
        {
          stats->antenna = reader->u.serialReader.txRxMap->list[i].antenna;
	  TMR_DEBUG("Mapped module port (tx=%d,rx=%d) to API antenna %d", tx, rx, stats->antenna);
          break;
        }
      }
      TMR_DEBUG("post-value: stats->antenna = %d", stats->antenna);
    }

    else if (flag & TMR_READER_STATS_FLAG_CONNECTED_ANTENNAS)
    {
      uint8_t k;
      offset += 3;
      stats->connectedAntennas.len = msg[offset - 1];
      TMR_FRS_DEBUG_MSG("connectedAntennnas", offset, stats->connectedAntennas.len);
      for (i = 0; i < stats->connectedAntennas.len; i++)
      {
        stats->connectedAntennas.list[i] = msg[offset++];
        if (((i % 2) == 0) || (i == 0))
        {
          for (k = 0; k < reader->u.serialReader.txRxMap->len; k++)
          {
            if (reader->u.serialReader.txRxMap->list[k].txPort == stats->connectedAntennas.list[i])
            {
              stats->connectedAntennas.list[i] = reader->u.serialReader.txRxMap->list[k].antenna;
              k = reader->u.serialReader.txRxMap->len;
            }
          }
        }
      }
    }
#endif /* TMR_ENABLE_UHF */

#ifdef TMR_ENABLE_HF_LF
    else if (flag & TMR_READER_STATS_FLAG_DC_VOLTAGE)
    {
      offset += 4;
      stats->dcVoltage = (int16_t)GETS16AT(msg, offset);
    }
#endif /* TMR_ENABLE_HF_LF */

    else
    {
      return TMR_ERROR_INVALID;
    }
  }/* End of while loop */
  return TMR_SUCCESS;
}
 
/**
 * TMR_SR_cmdGetReaderStats
 * Get the current per-port statistics.
 *   
 * @param reader [in]The reader
 * @param statFlags [in]The set of statistics together
 * @param stats [out]The ReaderStatistics structure populated with requested per-port values
 */
TMR_Status 
TMR_SR_cmdGetReaderStats(TMR_Reader *reader, TMR_Reader_StatsFlag statFlags,
                                         TMR_Reader_StatsValues *stats)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i, offset; 
  uint16_t flag = 0;
  i = 2;

  SETU8(msg, i, TMR_SR_OPCODE_GET_READER_STATS);
  SETU8(msg, i, (uint8_t)TMR_SR_READER_STATS_OPTION_GET_PER_PORT); /* Option byte */

  /**
   * To extend the flag byte, an EBV technique is to be used.
   * When the highest order bit of the flag byte is used,
   * it signals the reader’s parser, that another flag byte is to follow
   */
  TMR_SR_convertToEBV(msg, &i, statFlags);

  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, reader->u.serialReader.commandTimeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the statistics from response */
  if (NULL != stats)
  {
#ifdef TMR_ENABLE_UHF
    uint8_t  j;

    /**
     * preinitialize the rf ontime and the noise floor value to zero
     * berfore getting the reader stats
     */
    for (i = 0; i < stats->perAntenna.max; i++)
    {
      stats->perAntenna.list[i].antenna = 0;
      stats->perAntenna.list[i].rfOnTime = 0;
      stats->perAntenna.list[i].noiseFloor = 0;
    }
#endif /* TMR_ENABLE_UHF */

    if ((0x80) > statFlags)
    {
      offset = 7;
    }
    else if ((0x4000) > statFlags)
    {
      offset = 8;
    }
    else
    {
      offset = 9;
    }

    ret = TMR_fillReaderStats(reader, stats, flag, msg, offset);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
#ifdef TMR_ENABLE_UHF
    /**
     * iterate through the per antenna values,
     * If found  any 0-antenna rows, copy the
     * later rows down to compact out the empty space.
     */
    for (i = 0; i < reader->u.serialReader.txRxMap->len; i++)
    {
      if (!stats->perAntenna.list[i].antenna )
      {
        for (j = i + 1; j < reader->u.serialReader.txRxMap->len; j++)
        {
          if (stats->perAntenna.list[j].antenna) 
          {
            stats->perAntenna.list[i].antenna = stats->perAntenna.list[j].antenna;
            stats->perAntenna.list[i].rfOnTime = stats->perAntenna.list[j].rfOnTime;
            stats->perAntenna.list[i].noiseFloor = stats->perAntenna.list[j].noiseFloor;
            stats->perAntenna.list[j].antenna = 0;
            stats->perAntenna.list[j].rfOnTime = 0;
            stats->perAntenna.list[j].noiseFloor = 0;

            stats->perAntenna.len++;
            break;
          }
        }
      }
      else
      {
        /* Increment the length */
        stats->perAntenna.len++;
      }
    }
#endif /* TMR_ENABLE_UHF */
  }/* end of outermost if block */

  /* Store the requested flags for future validation */
  stats->valid = reader->statsFlag;

  return TMR_SUCCESS;
}

/**
 * TMR_SR_cmdResetReaderStats
 * Reset the per-port statistics.
 *   
 * @param reader [in]The reader
 * @param statFlags [in]The set of statistics to reset. Only the RF on time statistic may be reset.
 */
TMR_Status 
TMR_SR_cmdResetReaderStats(TMR_Reader *reader, TMR_Reader_StatsFlag statFlags)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  uint16_t temp = 0;
  i = 2;

  SETU8(msg, i, TMR_SR_OPCODE_GET_READER_STATS);
  SETU8(msg, i, (uint8_t)TMR_SR_READER_STATS_OPTION_RESET); /* Option byte */

 /**
   * To extend the flag byte, an EBV technique is to be used.
   * When the highest order bit of the flag byte is used,
   * it signals the reader’s parser, that another flag byte is to follow
   */
  if ((0x80) > statFlags)
  {
    SETU8(msg, i, (uint8_t)statFlags);
  }
  else
  {
    temp = statFlags & 0x7f;
    statFlags &= 0xFF80;
    statFlags = (TMR_Reader_StatsFlag)((statFlags << 1) | temp);
    statFlags &= 0xFF7F;
    SETU16(msg, i, ((uint16_t)0x8000 | statFlags));
  }

  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, reader->u.serialReader.commandTimeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_receiveAutonomousReading(struct TMR_Reader *reader, TMR_TagReadData *trd, TMR_Reader_StatsValues *stats)
{
  TMR_Status ret;

  ret = TMR_SUCCESS;
  reader->continuousReading = true;

  ret = TMR_SR_hasMoreTags(reader);
  if (TMR_SUCCESS == ret)
  {        
    uint16_t flags;
#ifdef TMR_ENABLE_UHF
    if(isMultiSelectEnabled)
    {
      reader->u.serialReader.bufPointer++;
    }
#endif /* TMR_ENABLE_UHF */
    if (false == reader->isStatusResponse)
    {
#ifdef TMR_ENABLE_UHF
      /* Ignore the fail cases and pass only valid messages */
      if(isMultiSelectEnabled)
      {
        flags = GETU16AT(reader->u.serialReader.bufResponse, 9);
      }
      else
#endif /* TMR_ENABLE_UHF */
      {
        flags = GETU16AT(reader->u.serialReader.bufResponse, 8);
      }
      TMR_SR_parseMetadataFromMessage(reader, trd, flags, &reader->u.serialReader.bufPointer, reader->u.serialReader.bufResponse);
      TMR_SR_postprocessReaderSpecificMetadata(trd, &reader->u.serialReader);
      trd->reader = reader;
      reader->u.serialReader.tagsRemainingInBuffer--;
    }
    else
    {
      /* A status stream response */
      uint8_t offset;
      uint16_t flags = 0; 
#ifdef TMR_ENABLE_UHF
      uint8_t i,j;
      TMR_STATS_init(stats);
#endif /* TMR_ENABLE_UHF */

      offset = reader->u.serialReader.bufPointer;

      /* Get status content flags */
      if ((0x80) > reader->statsFlag)
      {
        offset += 1;
      }
      else if ((0x4000) > reader->statsFlag)
      {
        offset += 2;
      }
      else
      {
        offset += 3;
      }

#ifdef TMR_ENABLE_UHF
      /**
      * preinitialize the rf ontime and the noise floor value to zero
      * berfore getting the reader stats
      */
      for (i = 0; i < stats->perAntenna.max; i++)
      {
        stats->perAntenna.list[i].antenna = 0;
        stats->perAntenna.list[i].rfOnTime = 0;
        stats->perAntenna.list[i].noiseFloor = 0;
      }
#endif /* TMR_ENABLE_UHF */
      TMR_fillReaderStats(reader, stats, flags, reader->u.serialReader.bufResponse, offset);
#ifdef TMR_ENABLE_UHF
      /**
      * iterate through the per antenna values,
      * If found  any 0-antenna rows, copy the
      * later rows down to compact out the empty space.
      */
    if(!isStreamEnabled)
    {
      for (i = 0; i < reader->u.serialReader.txRxMap->len; i++)
      {
        if (!stats->perAntenna.list[i].antenna)
        {
          for (j = i + 1; j < reader->u.serialReader.txRxMap->len; j++)
          {
            if (stats->perAntenna.list[j].antenna)
            {
              stats->perAntenna.list[i].antenna = stats->perAntenna.list[j].antenna;
              stats->perAntenna.list[i].rfOnTime = stats->perAntenna.list[j].rfOnTime;
              stats->perAntenna.list[i].noiseFloor = stats->perAntenna.list[j].noiseFloor;
              stats->perAntenna.list[j].antenna = 0;
              stats->perAntenna.list[j].rfOnTime = 0;
              stats->perAntenna.list[j].noiseFloor = 0;

              stats->perAntenna.len++;
              break;
            }
          }
        }
        else
        {
          /* Increment the length */
          stats->perAntenna.len++;
        }
      }
    }
#endif /* TMR_ENABLE_UHF */
      /* store the requested flags for future use */
      stats->valid = reader->statsFlag;
    }
  }
  
  reader->continuousReading = false;
  return ret;  
}

TMR_Status
TMR_SR_cmdStopReading(struct TMR_Reader *reader)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i, op;
  reader->hasContinuousReadStarted = false;
  i = 2;
  op = TMR_SR_OPCODE_MULTI_PROTOCOL_TAG_OP;
  SETU8(msg, i, op);
  SETU16(msg, i, 0); /* Timeout, Currently ignored */
  SETU8(msg, i, (uint8_t)0x02); /* option - stop continuous reading */

  msg[1]=i - 3;  /* Install length */

  /* No need to capture the response */
  return TMR_SR_sendMessage(reader, msg, &op, reader->u.serialReader.commandTimeout);
}
#ifdef TMR_ENABLE_UHF
TMR_Status
TMR_SR_cmdGetReaderWriteTimeOut (struct TMR_Reader *reader, TMR_TagProtocol protocol,
                                 TMR_SR_Gen2ReaderWriteTimeOut *value)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i, op;

  i = 2;
  op = TMR_SR_OPCODE_GET_PROTOCOL_PARAM;
  SETU8(msg, i, op);
  SETU8(msg, i, protocol); 
  SETU8(msg, i, (uint8_t)0x3f); /* option - reader write timeout */

  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Get the values */
  value->earlyexit =  GETU8AT(msg, 7);
  value->writetimeout = GETU16AT(msg, 8);

  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_cmdSetReaderWriteTimeOut (struct TMR_Reader *reader, TMR_TagProtocol protocol,
                                 TMR_SR_Gen2ReaderWriteTimeOut *value)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_PROTOCOL_PARAM);
  SETU8(msg, i, protocol);
  SETU8(msg, i, (uint8_t)0x3f); /* option - reader write timeout */
  SETU8(msg, i, (uint8_t)value->earlyexit);
  SETU16(msg,i, value->writetimeout);

  return TMR_SR_sendCmd(reader, msg, i);
}

TMR_Status
TMR_SR_cmdAuthReqResponse(struct TMR_Reader *reader, TMR_TagAuthentication *auth)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  uint8_t opcode;

  i = 2;
  SETU8(msg, i, (uint8_t)TMR_SR_OPCODE_MULTI_PROTOCOL_TAG_OP);
  SETU8(msg, i, (uint8_t)0x00);
  SETU8(msg, i, (uint8_t)0x00);
  SETU8(msg, i, (uint8_t)0x03);
  SETU8(msg, i, (uint8_t)0x00);
  SETU8(msg, i, (uint8_t)0x01);
  SETU8(msg, i, (uint8_t)0x00);

  switch (auth->type)
  {
  default:
	return TMR_ERROR_UNSUPPORTED;

  case TMR_AUTH_TYPE_GEN2_PASSWORD:
	{
	  TMR_GEN2_Password password = auth->u.gen2Password;
	  SETU8(msg, i, (uint8_t)0x20);
	  SETU8(msg, i, (uint8_t)(password>>24));
	  SETU8(msg, i, (uint8_t)(password>>16));
	  SETU8(msg, i, (uint8_t)(password>> 8));
	  SETU8(msg, i, (uint8_t)(password>> 0));
	}
	break;
  }

  msg[1] = i - 3; /* Install length */
  return TMR_SR_sendMessage(reader, msg, &opcode, 0);
}

/**
 *  Helper routine to form the Fudan Read REG command
 */
TMR_Status 
TMR_SR_msgAddFDNReadREG(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, 
                        uint16_t CommandCode, uint16_t RegAddress, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, (uint8_t)TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, (uint16_t)timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_FUDAN_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40); //option
  SETU16(msg, *i, CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU16(msg, *i, RegAddress);

  return ret;
}

/**
 * Fudan Read REG
 * Chip type = 0x0F, command = 0x00 09
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param regaddress to specify the Reg address from the data need to be read
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdFDNReadREG(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                     uint16_t CommandCode, uint16_t RegAddress, TMR_TagFilter* target, TMR_uint8List *data)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddFDNReadREG(msg, &i, timeout, accessPassword, CommandCode, RegAddress, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the Read REG data response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    08     2d    00 00      0F     40     00 09          xx xx      xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand    [data]       CRC
   */
  {
    uint16_t copyLength;
    
    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the Fudan Write REG command
 */
TMR_Status 
TMR_SR_msgAddFDNWriteREG(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, 
                         uint16_t CommandCode, uint16_t RegAddress, uint16_t RegWriteData, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, (uint8_t)TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, (uint16_t)timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_FUDAN_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40); //option
  SETU16(msg, *i, CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU16(msg, *i, RegAddress);
  SETU16(msg, *i, RegWriteData);

  return ret;
}

/**
 * Fudan Write REG
 * Chip type = 0x0F, command = 0x00 08
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param regaddress to specify the Reg address from the data need to be read
 * @param regwritedata to specify the data need to be written
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdFDNWriteREG(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                     uint16_t CommandCode, uint16_t RegAddress, uint16_t RegWriteData, TMR_TagFilter* target, TMR_uint8List *data)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddFDNWriteREG(msg, &i, timeout, accessPassword, CommandCode, RegAddress, RegWriteData, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }

  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the Write REG data response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    08     2d    00 00      0F     40     00 08        xx.xx  xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand   [data]  CRC
   */
  {
    uint16_t copyLength;
    
    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the Fudan Load REG command
 */
TMR_Status 
TMR_SR_msgAddFDNLoadREG(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, 
                         uint16_t CommandCode, uint8_t CmdCfg, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, (uint8_t)TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, (uint16_t)timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_FUDAN_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40); //option
  SETU16(msg, *i, CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, CmdCfg);

  return ret;
}

/**
 * Fudan Load REG
 * Chip type = 0x0F, command = 0x00 11
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param CmdCfg is Reserved for furture use
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdFDNLoadREG(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                     uint16_t CommandCode, uint8_t CmdCfg, TMR_TagFilter* target, TMR_uint8List *data)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;
  ret = TMR_SR_msgAddFDNLoadREG(msg, &i, timeout, accessPassword, CommandCode, CmdCfg, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the Load REG data response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    08     2d    00 00      0F     40       00 11          00 00     
   * SOH Length OpCode Status  ChipType  option  SubCommand    [Result]       
   */
  {
    uint16_t copyLength;
    
    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the Fudan Start/Stop LOG command
 */
TMR_Status 
TMR_SR_msgAddFDNStartStopLOG(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, 
                         uint16_t CommandCode, uint8_t CmdCfg, uint32_t FlagResetPassword, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, (uint8_t)TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, (uint16_t)timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_FUDAN_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40); //option
  SETU16(msg, *i, CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, CmdCfg);
  SETU32(msg, *i, FlagResetPassword);

  return ret;
}

/**
 * Fudan Start/Stop LOG
 * Chip type = 0x0F, command = 0x00 06
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param CmdCfg is to start or stop logging
 * @param Flow Flag Reset password is the password used to start or stop the logging
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdFDNStartStopLOG(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                          uint16_t CommandCode, uint8_t CmdCfg, uint32_t FlagResetPassword, TMR_TagFilter* target, TMR_uint8List *data)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddFDNStartStopLOG(msg, &i, timeout, accessPassword, CommandCode, CmdCfg, FlagResetPassword, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }

  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the Start/Stop log data response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    08     2d    00 00      0F     40     00   06         00 00     
   * SOH Length OpCode Status  ChipType  option  SubCommand    [Result]       
   */
  {
    uint16_t copyLength;
    
    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the Fudan Auth command
 */
TMR_Status 
TMR_SR_msgAddFDNAuth(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, 
                         uint16_t CommandCode, uint8_t CmdCfg, uint32_t AuthPassword, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, (uint8_t)TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, (uint16_t)timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_FUDAN_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40); //option
  SETU16(msg, *i, CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, CmdCfg);
  SETU32(msg, *i, AuthPassword);

  return ret;
}

/**
 * Fudan Auth
 * Chip type = 0x0F, command = 0x00 02
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param CmdCfg is to configure command
 * @param Auth password is the password used to authenticate
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdFDNAuth(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                  uint16_t CommandCode, uint8_t CmdCfg, uint32_t AuthPassword, TMR_TagFilter* target, TMR_uint8List *data)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddFDNAuth(msg, &i, timeout, accessPassword, CommandCode, CmdCfg, AuthPassword, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }

  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the Read REG data response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    08     2d    00 00      0F     40     00 02          00 00     
   * SOH Length OpCode Status  ChipType  option  SubCommand    [Result]       
   */
  {
    uint16_t copyLength;
    
    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the Fudan Read MEM command
 */
TMR_Status 
TMR_SR_msgAddFDNReadMEM(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, 
                         uint16_t CommandCode, uint16_t StartAddress, uint16_t len, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, (uint8_t)TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, (uint16_t)timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_FUDAN_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40); //option
  SETU16(msg, *i, CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU16(msg, *i, StartAddress);
  SETU16(msg, *i, len);

  return ret;
}

/**
 * Fudan Read MEM
 * Chip type = 0x0F, command = 0x00 00
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param Startaddress to specify the Start address from which the data need to be read
 * @param len to specify the length of data to be read
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdFDNReadMEM(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                     uint16_t CommandCode, uint16_t StartAddress, uint16_t len, TMR_TagFilter* target, TMR_uint8List *data)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddFDNReadMEM(msg, &i, timeout, accessPassword, CommandCode, StartAddress, len, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  

  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the Read MEM data response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    08     2d    00 00      0F     40     00 00          xx xx      xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand    [data]       CRC
   */
  {
    uint16_t copyLength;
    
    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the Fudan Write Mem command
 */
TMR_Status 
TMR_SR_msgAddFDNWriteMEM(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, 
                         uint16_t CommandCode, uint16_t StartAddress, uint16_t len, const uint8_t* data, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, (uint8_t)TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, (uint16_t)timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_FUDAN_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40); //option
  SETU16(msg, *i, CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU16(msg, *i, StartAddress);
  SETU8(msg, *i, (uint8_t)len);
  if ((*i) + len + 1 > TMR_SR_MAX_PACKET_SIZE)
  {
    return TMR_ERROR_TOO_BIG;
  }
  memcpy(&msg[*i], data, len);
  (*i) += len;

  return ret;
}

/**
 * Fudan Write MEM
 * Chip type = 0x0F, command = 0x00 01
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param Startaddress to specify the Start address from which the data need to be written
 * @param len to specify the length of data to be read
 * @param data specify the data to be written
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdFDNWriteMEM(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                     uint16_t CommandCode, uint16_t StartAddress, uint16_t len, const uint8_t* Writedata, TMR_TagFilter* target, TMR_uint8List *data)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddFDNWriteMEM(msg, &i, timeout, accessPassword, CommandCode, StartAddress, len, Writedata, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }

  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the Read MEM data response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    08     2d    00 00      0F     40     00 01          xx xx      xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand    [data]       CRC
   */
  {
    uint16_t copyLength;
    
    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the Fudan State Check command
 */
TMR_Status 
TMR_SR_msgAddFDNStateCheck(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, 
                         uint16_t CommandCode, uint16_t len, const uint8_t* data, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, (uint8_t)TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, (uint16_t)timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_FUDAN_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40); //option
  SETU16(msg, *i, CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  if ((*i) + len + 1 > TMR_SR_MAX_PACKET_SIZE)
  {
    return TMR_ERROR_TOO_BIG;
  }
  memcpy(&msg[*i], data, len);
  (*i) += len;

  return ret;
}

/**
 * Fudan State Check
 * Chip type = 0x0F, command = 0x00 0C
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param data specify the Cmd Cfg
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdFDNStateCheck(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                     uint16_t CommandCode, uint16_t len, const uint8_t* Data, TMR_TagFilter* target, TMR_uint8List *data)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddFDNStateCheck(msg, &i, timeout, accessPassword, CommandCode, len, Data, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }

  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the Read MEM data response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    08     2d    00 00      0F     40     00 0c          xx xx      xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand    [data]       CRC
   */
  {
    uint16_t copyLength;
    
    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the Fudan Measure command
 */
TMR_Status 
TMR_SR_msgAddFDNMeasure(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, 
                        uint16_t CommandCode, uint8_t CmdCfg, uint8_t StoreBlockAddress, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, (uint8_t)TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, (uint16_t)timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_FUDAN_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40); //option
  SETU16(msg, *i, CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg, *i, CmdCfg);
  SETU8(msg, *i, StoreBlockAddress);

  return ret;
}

/**
 * Fudan Measure
 * Chip type = 0x0F, command = 0x00 03
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode to specify the operation
 * @param CmdCfg to specify the Command Configuration
 * @param StoreBlockAddress to specify the Block address in which the temperat
 * @param data If the operation is success, this data contains the requested value
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdFDNMeasure(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                     uint16_t CommandCode, uint8_t CmdCfg, uint8_t StoreBlockAddress, TMR_TagFilter* target, TMR_uint8List *data)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;

  ret = TMR_SR_msgAddFDNMeasure(msg, &i, timeout, accessPassword, CommandCode, CmdCfg, StoreBlockAddress, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the Read REG data response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    08     2d    00 00      0F     40     00 09          xx xx      xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand    [data]       CRC
   */
  {
    uint16_t copyLength;
    
    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the ILIAN Tag Select command
 */
TMR_Status 
TMR_SR_msgAddILIANTagSelect(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, 
                         uint16_t CommandCode, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, (uint8_t)TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, (uint16_t)timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_ILIAN_LED_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40); //option
  SETU16(msg, *i, CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;

  return ret;
}

/**
 * ILIAN Tag Select
 * Chip type = 0x10, command = 0x00 01
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode To specify the operation
 * @param target Filter to be applied.
 */
TMR_Status
TMR_SR_cmdILIANTagSelect(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                         uint16_t CommandCode, TMR_TagFilter* target)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;
  ret = TMR_SR_msgAddILIANTagSelect(msg, &i, timeout, accessPassword, CommandCode, target);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the EM4325 Get Sensor Data command
 */
TMR_Status 
TMR_SR_msgAddEM4325GetSensorData(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                         uint16_t CommandCode, TMR_TagFilter* target,  uint8_t bitToSet)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, (uint8_t)TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, (uint16_t)timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_EMMICRO_EM4325_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40); //option
  SETU16(msg, *i, CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg,*i, bitToSet);

  return ret;
}

/**
 * EM4325 Get Sensor Data.
 * Chip type = 0x09, command = 0x00 01
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode To specify the operation
 * @param target Filter to be applied.
 * @param bitsToSet response cfg flags to be applied.
 */
TMR_Status
TMR_SR_cmdEM4325GetSensorData(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                         uint16_t CommandCode,  TMR_TagFilter* target, uint8_t bitToSet, TMR_uint8List *data)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;
  ret = TMR_SR_msgAddEM4325GetSensorData(msg, &i, timeout, accessPassword, CommandCode, target, bitToSet);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Parse the Read REG data response back to the user */
  i = isMultiSelectEnabled ? 10 : 9;
  /* FF    08     2d    00 00      09     40     00 01          xx xx      xx xx
   * SOH Length OpCode Status  ChipType  option  SubCommand    [data]       CRC
   */
  {
    uint16_t copyLength;

    copyLength = msg[1] + 5 - i;
    if (NULL != data)
    {
      memcpy(data->list, &msg[i], copyLength);
      data->len = copyLength;
    }
  }
  return TMR_SUCCESS;
}

/**
 *  Helper routine to form the EM4325 Reset Alarms.
 */
TMR_Status 
TMR_SR_msgAddEM4325ResetAlarms(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                         uint16_t CommandCode, TMR_TagFilter* target, uint8_t fillValue)
{
  TMR_Status ret;
  uint8_t option = 0, rec;

  SETU8(msg, *i, (uint8_t)TMR_SR_OPCODE_WRITE_TAG_SPECIFIC);
  SETU16(msg, *i, (uint16_t)timeout);
  SETU8(msg, *i, (uint8_t)TMR_SR_GEN2_EMMICRO_EM4325_SILICON); /* chip type */
  if((isMultiSelectEnabled) && (!isEmbeddedTagopEnabled))
  {
    SETU8(msg, *i, TMR_SR_TAGOP_MULTI_SELECT);
  }
  rec = *i;
  SETU8(msg,*i,0x40); //option
  SETU16(msg, *i, CommandCode);
  ret = filterbytes(TMR_TAG_PROTOCOL_GEN2, target, &option, i, msg, accessPassword, true);
  msg[rec]=msg[rec]|option;
  SETU8(msg,*i, fillValue);

  return ret;
}

/**
 * EM4325 Reset Alarms.
 * Chip type = 0x09, command = 0x00 04
 *
 * @param reader The reader
 * @param timeout The timeout of the operation, in milliseconds. Valid range is 0-65535.
 * @param accessPassword The access password to use to write on the tag
 * @param commandcode To specify the operation
 * @param target Filter to be applied.
 * @param fillValue cfg to be applied.
 */
TMR_Status
TMR_SR_cmdEM4325ResetAlarms(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                         uint16_t CommandCode,  TMR_TagFilter* target, uint8_t fillValue)
{
  TMR_Status ret;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  i = 2;
  ret = TMR_SR_msgAddEM4325ResetAlarms(msg, &i, timeout, accessPassword, CommandCode, target, fillValue);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }
  msg[1] = i - 3; /* Install length */

  ret = TMR_SR_sendTimeout(reader, msg, timeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  return TMR_SUCCESS;
}
#endif /* TMR_ENABLE_UHF */

#ifdef TMR_ENABLE_HF_LF
void
TMR_SR_msgAddAccessPassword(uint8_t* msg, uint8_t* optbyte, uint8_t* i, const TMR_uint8List *accessPassword)
{
  if (NULL != accessPassword)
  {
    //Append access PW flag in option byte.
    msg[*optbyte] |= TMR_SR_GEN2_SINGULATION_OPTION_SECURE_READ_DATA;

    //Append access PW length.
    SETU8(msg, *i, accessPassword->len);

    //Append access password.
    memcpy(&msg[*i], accessPassword->list, accessPassword->len);
    *i += accessPassword->len;
  }
}

#if TMR_ENABLE_EXTENDED_TAGOPS
TMR_Status
TMR_SR_msgAddExtendedParams(uint8_t* msg, uint8_t* i, uint8_t* optbyte, const TMR_uint8List* extParams)
{
  TMR_Status ret = TMR_SUCCESS;

  //Append extended params for memory read and write.
  if (extParams->len && extParams->list[0])
  {
    //Check if extended params are exceeding allowed cmd length.
    if ((*i + extParams->len + 1) > TMR_SR_MAX_PACKET_SIZE)
    {
      return TMR_ERROR_TOO_BIG;
    }

    //Append extended params flag in option byte.
    msg[*optbyte] |= TMR_SR_SINGULATION_OPTION_EXT_TAGOP_PARAMS;

    //Append extended params length.
    SETU8(msg, *i, extParams->len);

    //Append extended params.
    memcpy(&msg[*i], extParams->list, extParams->len);
    *i += extParams->len;
  }

  return ret;
}
#endif /* TMR_ENABLE_EXTENDED_TAGOPS */

void
TMR_SR_msgAddWriteMemory(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_Memory_Type memType,
                         uint32_t address)
{
  SETU8(msg, *i, TMR_SR_OPCODE_WRITE_TAG_DATA); //0x24 command
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, 0x00); //option
  SETU8(msg, *i, memType);
  if(isAddrByteExtended)
  {
    SETU32(msg, *i, address);
  }
  else
  {
    SETU8(msg, *i, address);
  }
}

TMR_Status
TMR_SR_cmdWriteMemory(TMR_Reader *reader, TMR_ExtTagOp *tagOp, const TMR_TagFilter *filter, 
                        TMR_uint8List tagOpExtParams)
{
  TMR_Status ret;
  uint16_t timeout;
  uint16_t dataLength;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i = 2, optbyte = 0;
  TMR_SR_SerialReader *sr = &reader->u.serialReader;
  TMR_Memory_Type writeMemType = tagOp->writeMem.memType;

  timeout = (uint16_t)sr->commandTimeout;
  dataLength = tagOp->writeMem.data.len;

  if (TMR_TAGOP_EXT_TAG_MEMORY == tagOp->writeMem.memType)
  {
    writeMemType = TMR_TAGOP_TAG_MEMORY;
  }

  TMR_SR_msgAddWriteMemory(msg, &i, timeout, writeMemType, tagOp->writeMem.address);
  optbyte = 5;

  //Assemble access password.
  TMR_SR_msgAddAccessPassword(msg, &optbyte, &i, tagOp->accessPassword);

#if TMR_ENABLE_EXTENDED_TAGOPS
  //Assemble extended params.
  ret = TMR_SR_msgAddExtendedParams(msg, &i, &optbyte, &tagOpExtParams);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
#endif /* TMR_ENABLE_EXTENDED_TAGOPS */

  //Assemble filter data.
  ret = filterbytes(reader->u.serialReader.currentProtocol, filter, &msg[optbyte], &i, msg, 0, false);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }

  //Return error if the serial command has already reached the limit.
  if (i + dataLength + 1 > TMR_SR_MAX_PACKET_SIZE)
  {
    return TMR_ERROR_TOO_BIG;
  }

  //Assemble data to be written.
  memcpy(&msg[i], tagOp->writeMem.data.list, dataLength);
  i += dataLength;

  return TMR_SR_sendCmd(reader, msg, i);
}

void
TMR_SR_msgAddReadMemory(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_Memory_Type memType, uint32_t address,
                        uint8_t len, bool withMetaData)
{
  SETU8(msg, *i, TMR_SR_OPCODE_READ_TAG_DATA); //0x28 command
  SETU16(msg, *i, timeout);
  SETU8(msg, *i, 0x00); //option
  if (withMetaData)
  {
    SETU16(msg, *i, 0); // metadata flags - initialize
  }
  SETU8(msg, *i, memType);
  if(isAddrByteExtended)
  {
    SETU32(msg, *i, address);
  }
  else
  {
    SETU8(msg, *i, address);
  }
  SETU8(msg, *i, len);
}

TMR_Status
TMR_SR_cmdReadMemory(TMR_Reader *reader, TMR_ExtTagOp *tagOp, const TMR_TagFilter *filter, 
                       TMR_uint8List *data, TMR_uint8List tagOpExtParams)
{
  TMR_Status ret;
  uint16_t timeout;
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i = 2, optbyte = 0;
  TMR_SR_SerialReader *sr = &reader->u.serialReader;
  bool withMetaData = true;
  TMR_Memory_Type readMemType = tagOp->readMem.memType;

  timeout = (uint16_t)sr->commandTimeout;

  if (TMR_TAGOP_EXT_TAG_MEMORY == tagOp->readMem.memType)
  {
    withMetaData = false;
    readMemType = TMR_TAGOP_TAG_MEMORY;
  }

  TMR_SR_msgAddReadMemory(msg, &i, timeout, readMemType, tagOp->readMem.address,
                            tagOp->readMem.len, withMetaData);
  optbyte = 5;

  //Assemble access password.
  TMR_SR_msgAddAccessPassword(msg, &optbyte, &i, tagOp->accessPassword);

#if TMR_ENABLE_EXTENDED_TAGOPS
  //Assemble extended params.
  ret = TMR_SR_msgAddExtendedParams(msg, &i, &optbyte, &tagOpExtParams);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
#endif /* TMR_ENABLE_EXTENDED_TAGOPS */

  //Assemble filter data.
  ret = filterbytes(reader->u.serialReader.currentProtocol, filter, &msg[optbyte], &i, msg, 0, false);
  if(TMR_SUCCESS != ret)
  {
    return ret;
  }

  if (withMetaData)
  {
    msg[optbyte] |= 0x10;
    optbyte++;
    SETU16(msg, optbyte, 0);
  }

  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  if (NULL != data->list)
  {
    i = 8;

    /* Read Tag Data doesn't put actual tag data inside the metadata fields.
     * Read the actual data here (remainder of response.) */
    {
      uint16_t dataLength;
    
      dataLength = msg[1] + 5 - i;
      if (dataLength > data->max)
      {
        dataLength = data->max;
      }
      data->len = dataLength;

      memcpy(data->list, &msg[i], dataLength);
      i += dataLength;
    }
  }

  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_cmdPassThrough(TMR_Reader *reader, uint32_t timeout, uint32_t configFlags,
                                   uint16_t cmdLen, const uint8_t *cmd , TMR_uint8List *data)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  TMR_Status ret;
  uint8_t i = 2;

  SETU8(msg, i, TMR_SR_OPCODE_PASS_THROUGH); //0x27 Opcode
  SETU8(msg, i, 0x00); //Sub-Option-RFU
  SETU16(msg, i, (uint16_t)timeout);

  ret = TMR_SR_convertToEBV(msg, &i, configFlags);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  if (i + cmdLen + 1 > TMR_SR_MAX_PACKET_SIZE)
  {
    return TMR_ERROR_TOO_BIG;
  }

  memcpy(&msg[i], cmd, cmdLen);
  i += cmdLen;

  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  if (NULL != data->list)
  {
    i = 5;

    /* Read the passthrough command response. */
    {
      uint16_t dataLength;

      dataLength = msg[1] + 5 - i;
      if (dataLength > data->max)
      {
        dataLength = data->max;
      }
      data->len = dataLength;

      memcpy(data->list, &msg[i], dataLength);
      i += dataLength;
    }
  }
  return TMR_SUCCESS;
}
#endif /* TMR_ENABLE_HF_LF */

#endif /* TMR_ENABLE_SERIAL_READER */
