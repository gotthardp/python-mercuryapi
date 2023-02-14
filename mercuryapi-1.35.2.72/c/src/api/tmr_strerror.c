/**
 *  @file tmr_strerror.c
 *  @brief Mercury API - status code to string conversion
 *  @author Nathan Williams
 *  @date 10/23/2009
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

#include <stdio.h>
#include <string.h>

#include "tm_reader.h"

#if defined(TMR_ENABLE_ERROR_STRINGS)

const char *
TMR_strerror(TMR_Status status)
{
  return TMR_strerr(NULL, status);
}

const char *
TMR_strerr(TMR_Reader *reader, TMR_Status status)
{

  if (TMR_ERROR_IS_COMM(status) && TMR_ERROR_COMM_IS_ERRNO(status))
  {
#ifdef TMR_USE_STRERROR
    return strerror(TMR_ERROR_COMM_GET_ERRNO(status));
#else
    return "Error";
#endif    
  }

#ifdef TMR_ENABLE_LLRP_READER
  if (TMR_ERROR_IS_LLRP_SPECIFIC(status))
  {
    switch (status)
    {
      case TMR_ERROR_LLRP_MSG_PARSE_ERROR:
        return "Error parsing LLRP message";
      case TMR_ERROR_LLRP_ALREADY_CONNECTED:
        return "Already connected to reader";
      case TMR_ERROR_LLRP_INVALID_RFMODE:
        return "Specified RF Mode operation is not supported";
      case TMR_ERROR_LLRP_UNDEFINED_VALUE:
        return "Undefined Value";
      case TMR_ERROR_LLRP_READER_ERROR:
        return "LLRP reader unknown error";
      case TMR_ERROR_LLRP_READER_CONNECTION_LOST:
        return "LLRP reader connection lost";
      case TMR_ERROR_LLRP_GETTYPEREGISTRY:
        return "LLRP Reader GetTypeRegistry Failed";
      case TMR_ERROR_LLRP_CONNECTIONFAILED:
        return "LLRP Reader Connection attempt is failed";
      case TMR_ERROR_LLRP_CLIENT_CONNECTION_EXISTS:
        return "LLRP Reader Connection attempt is failed, A Client initiated connection already exists";
      case TMR_ERROR_LLRP_SENDIO_ERROR:
        return "LLRP Reader Send Messages failed";
      case TMR_ERROR_LLRP_RECEIVEIO_ERROR:
        return "LLRP Reader Receive Messages failed";
      case TMR_ERROR_LLRP_RECEIVE_TIMEOUT:
        return "LLRP Reader Receive Messages Timeout";
      default:
        {
          if (NULL == reader)
            return "Unknown error: Reader reference lost";
          else
        return reader->u.llrpReader.errMsg;
    }
  }
  }
#endif

  switch (status)
  {
  case TMR_ERROR_MSG_WRONG_NUMBER_OF_DATA:
    return "Message command length is incorrect";
  case TMR_ERROR_INVALID_OPCODE:
    return "Invalid command opcode";
  case TMR_ERROR_UNIMPLEMENTED_OPCODE:
    return "Unimplemented opcode";
  case TMR_ERROR_MSG_POWER_TOO_HIGH:
    return "Command attempted to set power above maximum";
  case TMR_ERROR_MSG_INVALID_FREQ_RECEIVED:
    return "Command attempted to set an unsupported frequency";
  case TMR_ERROR_MSG_INVALID_PARAMETER_VALUE:
    return "Parameter to command is invalid";
  case TMR_ERROR_MSG_POWER_TOO_LOW:
    return "Command attempted to set power below minimum";
  case TMR_ERROR_UNIMPLEMENTED_FEATURE:
    return "Unimplemented feature";
  case TMR_ERROR_INVALID_BAUD_RATE:
    return "Invalid baud rate";
  case TMR_ERROR_INVALID_REGION:
    return "Invalid region";
  case TMR_ERROR_INVALID_LICENSE_KEY:
    return "The license key code received is invalid";
  case TMR_ERROR_BL_INVALID_IMAGE_CRC:
    return "Application image failed CRC check";
  case TMR_ERROR_BL_INVALID_APP_END_ADDR:
    return "Application image failed data check";
  case TMR_ERROR_FLASH_BAD_ERASE_PASSWORD:
    return "Incorrect password to erase flash sector";
  case TMR_ERROR_FLASH_BAD_WRITE_PASSWORD:
    return "Incorrect password to write to flash sector";
  case TMR_ERROR_FLASH_UNDEFINED_SECTOR:
    return "Internal error in flash";
  case TMR_ERROR_FLASH_ILLEGAL_SECTOR:
    return "Incorrect password to erase or write to flash sector";
  case TMR_ERROR_FLASH_WRITE_TO_NON_ERASED_AREA:
    return "Area of flash to write to is not erased";
  case TMR_ERROR_FLASH_WRITE_TO_ILLEGAL_SECTOR:
    return "Flash write attempted to cross sector boundary";
  case TMR_ERROR_FLASH_VERIFY_FAILED:
    return "Flash verify failed";
  case TMR_ERROR_NO_TAGS_FOUND:
    return "No tags found";
  case TMR_ERROR_NO_PROTOCOL_DEFINED:
    return "Protocol not set";
  case TMR_ERROR_INVALID_PROTOCOL_SPECIFIED:
    return "Specified protocol not supported";
  case TMR_ERROR_WRITE_PASSED_LOCK_FAILED:
    return "Lock failed after write operation";
  case TMR_ERROR_PROTOCOL_NO_DATA_READ:
    return "No data could be read from a tag";
  case TMR_ERROR_AFE_NOT_ON:
    return "AFE not on - reader not sufficiently configured";
  case TMR_ERROR_PROTOCOL_WRITE_FAILED:
    return "Tag write operation failed";
  case TMR_ERROR_NOT_IMPLEMENTED_FOR_THIS_PROTOCOL:
    return "Operation not supported for this protocol";
  case TMR_ERROR_PROTOCOL_INVALID_WRITE_DATA:
    return "Tag ID supplied in write operation is incorrect";
  case TMR_ERROR_PROTOCOL_INVALID_ADDRESS:
    return "Invalid address in tag address space";
  case TMR_ERROR_GENERAL_TAG_ERROR:
    return "General tag error";
  case TMR_ERROR_DATA_TOO_LARGE:
    return "Size specified in read tag data command is too large";
  case TMR_ERROR_PROTOCOL_INVALID_KILL_PASSWORD:
    return "Kill password is not correct";
  case TMR_ERROR_PROTOCOL_KILL_FAILED:
    return "Kill failed";
  case TMR_ERROR_PROTOCOL_BIT_DECODING_FAILED:
    return "Bit decoding failed";
  case TMR_ERROR_PROTOCOL_INVALID_EPC:
    return "Invalid EPC provided";
  case TMR_ERROR_PROTOCOL_INVALID_NUM_DATA:
    return "Invalid amount of data provided";
  case TMR_ERROR_GEN2_PROTOCOL_OTHER_ERROR:
    return "Other Gen2 error";
  case TMR_ERROR_GEN2_PROTOCOL_MEMORY_OVERRUN_BAD_PC:
    return "Gen2 memory overrun - bad PC";
  case TMR_ERROR_GEN2_PROTOCOL_MEMORY_LOCKED:
    return "Gen2 memory locked";
  case TMR_ERROR_GEN2_PROTOCOL_INSUFFICIENT_POWER:
    return "Gen2 tag has insufficient power for operation";
  case TMR_ERROR_GEN2_PROTOCOL_NON_SPECIFIC_ERROR:
    return "Gen2 nonspecific error";
  case TMR_ERROR_GEN2_PROTOCOL_UNKNOWN_ERROR:
    return "Gen2 unknown error";
	case TMR_ERROR_GEN2_PROTOCOL_V2_AUTHEN_FAILED:
		return "Authentication failed with specified key.";
	case TMR_ERROR_GEN2_PROTOCOL_V2_UNTRACE_FAILED:
		return "Untrace operation failed.";
  case TMR_ERROR_AHAL_INVALID_FREQ:
    return "Invalid frequency";
  case TMR_ERROR_AHAL_CHANNEL_OCCUPIED:
    return "Channel occupied";
  case TMR_ERROR_AHAL_TRANSMITTER_ON:
    return "Transmitter already on";
  case TMR_ERROR_ANTENNA_NOT_CONNECTED:
    return "Antenna not connected";
  case TMR_ERROR_TEMPERATURE_EXCEED_LIMITS:
    return "Reader temperature too high";
  case TMR_ERROR_HIGH_RETURN_LOSS:
    return "High return loss detected, RF ended to avoid damage";
  case TMR_ERROR_INVALID_ANTENNA_CONFIG:
    return "Invalid antenna configuration";
  case TMR_ERROR_TAG_ID_BUFFER_NOT_ENOUGH_TAGS_AVAILABLE:
    return "Not enough tag IDs in buffer";
  case TMR_ERROR_TAG_ID_BUFFER_FULL:
    return "Tag ID buffer full";
  case TMR_ERROR_TAG_ID_BUFFER_REPEATED_TAG_ID:
    return "Tag ID buffer repeated tag ID";
  case TMR_ERROR_TAG_ID_BUFFER_NUM_TAG_TOO_LARGE:
    return "Number of tags too large";
  case TMR_ERROR_IPVOLTAGE_EXCEEDED_LIMITS:
    return "Input voltage is too high";
  case TMR_ERROR_PROTOCOL_AUTH_FAILED:
    return "Tag Authentication Failed";
  case TMR_ERROR_PROTOCOL_INVALID_TAG_OP:
    return "Invalid tag operation";
  case TMR_ERROR_SYSTEM_UNKNOWN_ERROR:
    return "Unknown system error";
  case TMR_ERROR_TM_ASSERT_FAILED:
	{
	  if (NULL == reader)
	  {
    return "Assertion failed";
	  }
	  else
	  {
	    return reader->u.serialReader.errMsg;
	  }
	}
  case TMR_ERROR_BUFFER_OVERFLOW:
  return "Buffer overflow";
  case TMR_ERROR_TIMEOUT:
    return "Timeout";
  case TMR_ERROR_NO_HOST:
    return "No matching host found";
  case TMR_ERROR_LLRP:
    return "LLRP error";
  case TMR_ERROR_PARSE:
    return "Error parsing device response";
  case TMR_ERROR_DEVICE_RESET:
    return "Device was reset externally";
  case TMR_ERROR_CRC_ERROR:
    return "CRC Error";
  case TMR_ERROR_INVALID:
    return "Invalid argument";
  case TMR_ERROR_INVALID_VALUE:
    return "Input value is out of bound";
  case TMR_ERROR_UNIMPLEMENTED:
    return "Unimplemented operation";
  case TMR_ERROR_NO_ANTENNA:
    return "No antenna or invalid antenna";
  case TMR_ERROR_READONLY:
    return "Value is read-only";
  case TMR_ERROR_TOO_BIG:
    return "Value too big";
  case TMR_ERROR_NO_THREADS:
    return "Thread initialization failed";
  case TMR_ERROR_NO_TAGS:
    return "No tags to be retrieved";
  case TMR_ERROR_NOT_FOUND:
    return "Key not found";
  case TMR_ERROR_FIRMWARE_FORMAT:
    return "Size or format of firmware image is incorrect";
  case TMR_ERROR_TRYAGAIN:
    return "Temporary error, try again";
  case TMR_ERROR_OUT_OF_MEMORY:
    return "Out of memory";
  case TMR_ERROR_UNSUPPORTED:
    return "Unsupported operation";
  case TMR_ERROR_INVALID_WRITE_MODE:
	return "Invalid write mode";
  case TMR_ERROR_ILLEGAL_VALUE:
    return "Illegal value";
  case TMR_ERROR_UNSUPPORTED_READER_TYPE:
    return "Unsupported reader type";
  case TMR_ERROR_AUTOREAD_ENABLED:
    return "Autonomous mode is enabled on reader. Please disable it";
  case TMR_ERROR_FIRMWARE_UPDATE_ON_AUTOREAD:
    return "Firmware update is successful. Autonomous mode is already enabled on reader";
  case TMR_ERROR_TIMESTAMP_NULL:
    return "Timestamp cannot be null";
  case TMR_ERROR_METADATA_PROTOCOLMISSING:
    return "Invalid argument in /reader/metadata. TMR_TRD_METADATA_FLAG_PROTOCOL is a mandatory parameter.";
  case TMR_ERROR_METADATA_INVALID:
    return "Selected metadata are invalid";
#ifdef TMR_ENABLE_HF_LF
  case TMR_ERROR_METADATA_TAGTYPEMISSING:
    return "Invalid argument in /reader/metadata. TMR_TRD_METADATA_FLAG_TAGTYPE is mandatory for M3e.";
  case TMR_ERROR_UNSUPPORTED_SECUREREAD_TAGTYPE:
    return "Unsupported tagtype for secure read operation.";
#endif /* TMR_ENABLE_HF_LF */
  case TMR_ERROR_INVALID_READER_STATS:
    return "Reader stats selected are invalid.";
  case TMR_ERROR_NULL_POINTER:
    return "Null pointer exception.";
  case TMR_ERROR_CMDLEN_EXCEED_LIMIT:
    return "Command out of index range";

  default:
    {
      if (NULL == reader)
        return "Unknown error: Reader reference lost";
      else
      {
#ifdef TMR_ENABLE_LLRP_READER
        if (TMR_READER_TYPE_LLRP == reader->readerType)
        {
          return reader->u.llrpReader.errMsg;
        }
        else
#endif
        {
          return reader->u.serialReader.errMsg;
        }
      }
    }
  }
}

/**
 * This function will convert error code(uint16_t) into status code(TMR_Status).
 *
 * @param errorCode : 2 bytes of error code received from reader. 
 */
TMR_Status TMR_translateErrorCode(uint16_t errCode)
{
  return TMR_ERROR_CODE(errCode);
}
#endif /* defined(TMR_ENABLE_ERROR_STRINGS) */
