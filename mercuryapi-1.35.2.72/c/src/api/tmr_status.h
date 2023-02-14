#ifndef _TMR_STATUS_H
#define _TMR_STATUS_H

/**
 *  @file tmr_status.h
 *  @brief Mercury API - status codes
 *  @author Nathan Williams
 *  @date 11/24/2009
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
#ifdef WINCE
#include <stdint_win32.h>
#else
#include <stdint.h>
#endif

#ifdef  __cplusplus
extern "C" {
#endif

typedef uint32_t TMR_Status;

#define TMR_STATUS_MAKE(type, value) (((type) << 24) | (value))
#define TMR_STATUS_GET_TYPE(x)       (((x) >> 24) & 0xff)
#define TMR_STATUS_GET_VALUE(x)      ((x) & 0xffffff)

#define TMR_SUCCESS_TYPE    0L
#define TMR_ERROR_TYPE_COMM 1L
#define TMR_ERROR_TYPE_CODE 2L
#define TMR_ERROR_TYPE_MISC 3L
#define TMR_ERROR_TYPE_LLRP 4L

#define TMR_SUCCESS TMR_STATUS_MAKE(TMR_SUCCESS_TYPE, 0)

#define TMR_ERROR_CODE(x)                                 TMR_STATUS_MAKE(TMR_ERROR_TYPE_CODE, (x))
#define TMR_ERROR_IS_CODE(x)                              (TMR_ERROR_TYPE_CODE == TMR_STATUS_GET_TYPE(x))
#define TMR_ERROR_GET_CODE(x)                             TMR_STATUS_GET_VALUE(x)
/**Invalid number of arguments  */
#define TMR_ERROR_MSG_WRONG_NUMBER_OF_DATA                TMR_ERROR_CODE(0x100)
/**Command opcode not recognized.  */
#define TMR_ERROR_INVALID_OPCODE                          TMR_ERROR_CODE(0x101)
/**Command opcode recognized, but is not supported.  */
#define TMR_ERROR_UNIMPLEMENTED_OPCODE                    TMR_ERROR_CODE(0x102)
/**Requested power setting is above the allowed maximum.  */
#define TMR_ERROR_MSG_POWER_TOO_HIGH                      TMR_ERROR_CODE(0x103)
/**Requested frequency is outside the allowed range.  */
#define TMR_ERROR_MSG_INVALID_FREQ_RECEIVED               TMR_ERROR_CODE(0x104)
/**Parameter value is outside the allowed range.  */
#define TMR_ERROR_MSG_INVALID_PARAMETER_VALUE             TMR_ERROR_CODE(0x105)
/**Requested power setting is below the allowed minimum. */
#define TMR_ERROR_MSG_POWER_TOO_LOW                       TMR_ERROR_CODE(0x106)
/**Command not supported. */
#define TMR_ERROR_UNIMPLEMENTED_FEATURE                   TMR_ERROR_CODE(0x109)
/**Requested serial speed is not supported. */
#define TMR_ERROR_INVALID_BAUD_RATE                       TMR_ERROR_CODE(0x10a)
/**Region is not supported. */
#define TMR_ERROR_INVALID_REGION                          TMR_ERROR_CODE(0x10b)
/** License key code is invalid */
#define TMR_ERROR_INVALID_LICENSE_KEY                     TMR_ERROR_CODE(0x10c)
/**Firmware is corrupt: Checksum doesn't match content. */
#define TMR_ERROR_BL_INVALID_IMAGE_CRC                    TMR_ERROR_CODE(0x200)
/**Serial protocol status code for this exception. */
#define TMR_ERROR_BL_INVALID_APP_END_ADDR                 TMR_ERROR_CODE(0x201)
/**Internal reader error.  Contact support. */
#define TMR_ERROR_FLASH_BAD_ERASE_PASSWORD                TMR_ERROR_CODE(0x300)
/**Internal reader error.  Contact support. */
#define TMR_ERROR_FLASH_BAD_WRITE_PASSWORD                TMR_ERROR_CODE(0x301)
/**Internal reader error.  Contact support. */
#define TMR_ERROR_FLASH_UNDEFINED_SECTOR                  TMR_ERROR_CODE(0x302)
/**Internal reader error.  Contact support. */
#define TMR_ERROR_FLASH_ILLEGAL_SECTOR                    TMR_ERROR_CODE(0x303)
/**Internal reader error.  Contact support. */
#define TMR_ERROR_FLASH_WRITE_TO_NON_ERASED_AREA          TMR_ERROR_CODE(0x304)
/**Internal reader error.  Contact support. */
#define TMR_ERROR_FLASH_WRITE_TO_ILLEGAL_SECTOR           TMR_ERROR_CODE(0x305)
/**Internal reader error.  Contact support. */
#define TMR_ERROR_FLASH_VERIFY_FAILED                     TMR_ERROR_CODE(0x306)
/**Reader was asked to find tags, but none were detected. */
#define TMR_ERROR_NO_TAGS_FOUND                           TMR_ERROR_CODE(0x400)
/**RFID protocol has not been configured. */
#define TMR_ERROR_NO_PROTOCOL_DEFINED                     TMR_ERROR_CODE(0x401)
/**Requested RFID protocol is not recognized. */
#define TMR_ERROR_INVALID_PROTOCOL_SPECIFIED              TMR_ERROR_CODE(0x402)
/**For write-then-lock commands, tag was successfully written, but lock failed.*/
#define TMR_ERROR_WRITE_PASSED_LOCK_FAILED                TMR_ERROR_CODE(0x403)
/**Tag data was requested, but could not be read. */
#define TMR_ERROR_PROTOCOL_NO_DATA_READ                   TMR_ERROR_CODE(0x404)
/**Reader not fully initialized and hasn't yet turned on its radio.  Have you set region?*/
#define TMR_ERROR_AFE_NOT_ON                              TMR_ERROR_CODE(0x405)
/**Write to tag failed. */
#define TMR_ERROR_PROTOCOL_WRITE_FAILED                   TMR_ERROR_CODE(0x406)
/**Command is not supported in the current RFID protocol. */
#define TMR_ERROR_NOT_IMPLEMENTED_FOR_THIS_PROTOCOL       TMR_ERROR_CODE(0x407)
/**Data does not conform to protocol standards. */
#define TMR_ERROR_PROTOCOL_INVALID_WRITE_DATA             TMR_ERROR_CODE(0x408)
/**Requested data address is outside the valid range. */
#define TMR_ERROR_PROTOCOL_INVALID_ADDRESS                TMR_ERROR_CODE(0x409)
/**Unknown error during RFID operation. */
#define TMR_ERROR_GENERAL_TAG_ERROR                       TMR_ERROR_CODE(0x40a)
/**Read Tag Data was asked for more data than it supports. */
#define TMR_ERROR_DATA_TOO_LARGE                          TMR_ERROR_CODE(0x40b)
/**Incorrect password was provided to Kill Tag. */
#define TMR_ERROR_PROTOCOL_INVALID_KILL_PASSWORD          TMR_ERROR_CODE(0x40c)
/**Kill failed for unknown reason. */
#define TMR_ERROR_PROTOCOL_KILL_FAILED                    TMR_ERROR_CODE(0x40e)
/**Internal reader error.  Contact support. */
#define TMR_ERROR_PROTOCOL_BIT_DECODING_FAILED            TMR_ERROR_CODE(0x40f)
/**Internal reader error.  Contact support. */
#define TMR_ERROR_PROTOCOL_INVALID_EPC                    TMR_ERROR_CODE(0x410)
/**Internal reader error.  Contact support. */
#define TMR_ERROR_PROTOCOL_INVALID_NUM_DATA               TMR_ERROR_CODE(0x411)
/**Internal reader error.  Contact support. */
#define TMR_ERROR_PROTOCOL_INVALID_TAG_OP                 TMR_ERROR_CODE(0x413)
/**Internal reader error.  Contact support. */
#define TMR_ERROR_GEN2_PROTOCOL_OTHER_ERROR               TMR_ERROR_CODE(0x420)
/**Internal reader error.  Contact support. */
#define TMR_ERROR_GEN2_PROTOCOL_MEMORY_OVERRUN_BAD_PC     TMR_ERROR_CODE(0x423)
/**Internal reader error.  Contact support. */
#define TMR_ERROR_GEN2_PROTOCOL_MEMORY_LOCKED             TMR_ERROR_CODE(0x424)
/**Authentication failed with specified key. */
#define TMR_ERROR_GEN2_PROTOCOL_V2_AUTHEN_FAILED          TMR_ERROR_CODE(0x425)
/** Untrace operation failed. */
#define TMR_ERROR_GEN2_PROTOCOL_V2_UNTRACE_FAILED         TMR_ERROR_CODE(0x426)
/**Internal reader error.  Contact support. */
#define TMR_ERROR_GEN2_PROTOCOL_INSUFFICIENT_POWER        TMR_ERROR_CODE(0x42b)
/**Internal reader error.  Contact support. */
#define TMR_ERROR_GEN2_PROTOCOL_NON_SPECIFIC_ERROR        TMR_ERROR_CODE(0x42f)
/**Internal reader error.  Contact support. */ 
#define TMR_ERROR_GEN2_PROTOCOL_UNKNOWN_ERROR             TMR_ERROR_CODE(0x430)
/**Internal reader error.  Contact support. */ 
#define TMR_ERROR_PROTOCOL_AUTH_FAILED                    TMR_ERROR_CODE(0x4A7)
/**A command was received to set a frequency outside the specified range. */
#define TMR_ERROR_AHAL_INVALID_FREQ                       TMR_ERROR_CODE(0x500)
/**With LBT enabled an attempt was made to set the frequency to an occupied channel. */
#define TMR_ERROR_AHAL_CHANNEL_OCCUPIED                   TMR_ERROR_CODE(0x501)
/**Checking antenna status while CW is on is not allowed. */
#define TMR_ERROR_AHAL_TRANSMITTER_ON                     TMR_ERROR_CODE(0x502)
/** Antenna not detected during pre-transmit safety test. */
#define TMR_ERROR_ANTENNA_NOT_CONNECTED                   TMR_ERROR_CODE(0x503)
/**Reader temperature outside safe range. */
#define TMR_ERROR_TEMPERATURE_EXCEED_LIMITS               TMR_ERROR_CODE(0x504)
/** Excess power detected at transmitter port, usually due to antenna tuning mismatch. */
#define TMR_ERROR_HIGH_RETURN_LOSS                        TMR_ERROR_CODE(0x505)
#define TMR_ERROR_INVALID_ANTENNA_CONFIG                  TMR_ERROR_CODE(0x507)
/**Reader DC voltage outside safe range. */
#define TMR_ERROR_IPVOLTAGE_EXCEEDED_LIMITS               TMR_ERROR_CODE(0x509)
/**Asked for more tags than were available in the buffer. */
#define TMR_ERROR_TAG_ID_BUFFER_NOT_ENOUGH_TAGS_AVAILABLE TMR_ERROR_CODE(0x600)
/**Too many tags are in buffer.  Remove some with Get Tag ID Buffer or Clear Tag ID Buffer. */
#define TMR_ERROR_TAG_ID_BUFFER_FULL                      TMR_ERROR_CODE(0x601)
/**Internal error -- reader is trying to insert a duplicate tag record.  Contact support. */
#define TMR_ERROR_TAG_ID_BUFFER_REPEATED_TAG_ID           TMR_ERROR_CODE(0x602)
/**Asked for tags than a single transaction can handle. */
#define TMR_ERROR_TAG_ID_BUFFER_NUM_TAG_TOO_LARGE         TMR_ERROR_CODE(0x603)
/**Blocked response to get additional data from host. */
#define TMR_ERROR_TAG_ID_BUFFER_AUTH_REQUEST              TMR_ERROR_CODE(0x604)
/**Internal reader error.  Contact support. */
#define TMR_ERROR_SYSTEM_UNKNOWN_ERROR                    TMR_ERROR_CODE(0x7f00)
/**Internal reader error.  Contact support. */
#define TMR_ERROR_TM_ASSERT_FAILED                        TMR_ERROR_CODE(0x7f01)

#define TMR_ERROR_COMM(x)                                 TMR_STATUS_MAKE(TMR_ERROR_TYPE_COMM, (x))
#define TMR_ERROR_IS_COMM(x)                              (TMR_ERROR_TYPE_COMM == TMR_STATUS_GET_TYPE(x))
#define TMR_ERROR_COMM_ERRNO(x)                           TMR_ERROR_COMM(0x8000 | (x))
#define TMR_ERROR_COMM_IS_ERRNO(x)                        (0x8000 == (TMR_STATUS_GET_VALUE(x) & 0x8000))
#define TMR_ERROR_COMM_GET_ERRNO(x)                       (TMR_STATUS_GET_VALUE(x) & 0x7fff)
#define TMR_ERROR_TIMEOUT                                 TMR_ERROR_COMM(1)
#define TMR_ERROR_NO_HOST                                 TMR_ERROR_COMM(2)
#define TMR_ERROR_LLRP                                    TMR_ERROR_COMM(3)
#define TMR_ERROR_PARSE                                   TMR_ERROR_COMM(4)
#define TMR_ERROR_DEVICE_RESET                            TMR_ERROR_COMM(5)
#define TMR_ERROR_CRC_ERROR                               TMR_ERROR_COMM(6)
#define TMR_ERROR_BOOT_RESPONSE                           TMR_ERROR_COMM(7)

#define TMR_ERROR_MISC(x)                                 TMR_STATUS_MAKE(TMR_ERROR_TYPE_MISC, (x))
#define TMR_ERROR_INVALID                                 TMR_ERROR_MISC(1)
#define TMR_ERROR_UNIMPLEMENTED                           TMR_ERROR_MISC(2)
#define TMR_ERROR_UNSUPPORTED                             TMR_ERROR_MISC(3)
#define TMR_ERROR_NO_ANTENNA                              TMR_ERROR_MISC(4)
#define TMR_ERROR_READONLY                                TMR_ERROR_MISC(5)
#define TMR_ERROR_TOO_BIG                                 TMR_ERROR_MISC(6)
#define TMR_ERROR_NO_THREADS                              TMR_ERROR_MISC(7)
#define TMR_ERROR_NO_TAGS                                 TMR_ERROR_MISC(8)
#define TMR_ERROR_NOT_FOUND                               TMR_ERROR_MISC(9)
#define TMR_ERROR_FIRMWARE_FORMAT                         TMR_ERROR_MISC(10)
#define TMR_ERROR_TRYAGAIN                                TMR_ERROR_MISC(11)
#define TMR_ERROR_OUT_OF_MEMORY                           TMR_ERROR_MISC(12)
#define TMR_ERROR_INVALID_WRITE_MODE                      TMR_ERROR_MISC(13)
#define TMR_ERROR_ILLEGAL_VALUE	                          TMR_ERROR_MISC(14)
#define TMR_ERROR_END_OF_READING	                      TMR_ERROR_MISC(15)
#define TMR_ERROR_UNSUPPORTED_READER_TYPE                 TMR_ERROR_MISC(16)
#define TMR_ERROR_BUFFER_OVERFLOW                         TMR_ERROR_MISC(17)
#define TMR_ERROR_LOADSAVE_CONFIG                         TMR_ERROR_MISC(18)
#define TMR_ERROR_AUTOREAD_ENABLED                        TMR_ERROR_MISC(19)
#define TMR_ERROR_FIRMWARE_UPDATE_ON_AUTOREAD             TMR_ERROR_MISC(20)
#define TMR_ERROR_TIMESTAMP_NULL                          TMR_ERROR_MISC(21)
#define TMR_ERROR_METADATA_PROTOCOLMISSING                TMR_ERROR_MISC(22)
#define TMR_ERROR_INVALID_VALUE                           TMR_ERROR_MISC(23)
#define TMR_ERROR_METADATA_INVALID                        TMR_ERROR_MISC(24)
#ifdef TMR_ENABLE_HF_LF
#define TMR_ERROR_METADATA_TAGTYPEMISSING                 TMR_ERROR_MISC(25)
#define TMR_ERROR_UNSUPPORTED_SECUREREAD_TAGTYPE          TMR_ERROR_MISC(26)
#endif /* TMR_ENABLE_HF_LF */
#define TMR_ERROR_INVALID_READER_STATS                    TMR_ERROR_MISC(27)
#define TMR_ERROR_NULL_POINTER                            TMR_ERROR_MISC(28)
#define TMR_ERROR_CMDLEN_EXCEED_LIMIT                     TMR_ERROR_MISC(29) //command length

/* LLRP related errors */
#define TMR_ERROR_LLRP_SPECIFIC(x)                        TMR_STATUS_MAKE(TMR_ERROR_TYPE_LLRP, (x))
#define TMR_ERROR_IS_LLRP_SPECIFIC(x)                     (TMR_ERROR_TYPE_LLRP == TMR_STATUS_GET_TYPE(x))
#define TMR_ERROR_LLRP_GETTYPEREGISTRY                    TMR_ERROR_LLRP_SPECIFIC(1)
#define TMR_ERROR_LLRP_CONNECTIONFAILED                   TMR_ERROR_LLRP_SPECIFIC(2)
#define TMR_ERROR_LLRP_SENDIO_ERROR                       TMR_ERROR_LLRP_SPECIFIC(3)
#define TMR_ERROR_LLRP_RECEIVEIO_ERROR                    TMR_ERROR_LLRP_SPECIFIC(4)
#define TMR_ERROR_LLRP_RECEIVE_TIMEOUT                    TMR_ERROR_LLRP_SPECIFIC(5)
#define TMR_ERROR_LLRP_MSG_PARSE_ERROR                    TMR_ERROR_LLRP_SPECIFIC(6)
#define TMR_ERROR_LLRP_ALREADY_CONNECTED                  TMR_ERROR_LLRP_SPECIFIC(7)
#define TMR_ERROR_LLRP_INVALID_RFMODE                     TMR_ERROR_LLRP_SPECIFIC(8)
#define TMR_ERROR_LLRP_UNDEFINED_VALUE                    TMR_ERROR_LLRP_SPECIFIC(9)
#define TMR_ERROR_LLRP_READER_ERROR                       TMR_ERROR_LLRP_SPECIFIC(10)
#define TMR_ERROR_LLRP_READER_CONNECTION_LOST             TMR_ERROR_LLRP_SPECIFIC(11)
#define TMR_ERROR_LLRP_CLIENT_CONNECTION_EXISTS           TMR_ERROR_LLRP_SPECIFIC(12)

#ifdef TMR_ENABLE_ERROR_STRINGS
const char *TMR_strerror(TMR_Status status);
TMR_Status TMR_translateErrorCode(uint16_t errCode);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _TMR_STATUS_H */
