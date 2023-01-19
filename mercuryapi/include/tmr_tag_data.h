/* ex: set tabstop=2 shiftwidth=2 expandtab cindent: */
#ifndef _TMR_TAG_DATA_H
#define _TMR_TAG_DATA_H
/**
 *  @file tmr_tag_data.h  
 *  @brief Mercury API - Tag and tag read data structures
 *  @author Brian Fiegel
 *  @date 5/7/2009
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

#include "tmr_tag_protocol.h"
#include "tmr_gen2.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define TMR_MAX_EPC_BYTE_COUNT (62)

/**
 * A structure to represent RFID tags.
 * @ingroup filter
 */
typedef struct TMR_TagData
{
  /** Tag EPC */
  uint8_t epc[TMR_MAX_EPC_BYTE_COUNT];
  /** Protocol of the tag */
  TMR_TagProtocol protocol;
  /** Length of the tag's EPC in bytes */
  uint8_t epcByteCount;
#ifdef TMR_ENABLE_UHF
  /** Scrambled Brand Identifier of UCODE8 tag */
  uint8_t xepc[2];
  /** Tag CRC */
  uint16_t crc;
  /** Protocol-specific tag information */
  union
  {
    /** Gen2-specific tag information */
    TMR_GEN2_TagData gen2;
  }u;
#endif /* TMR_ENABLE_UHF */
} TMR_TagData;
  
#ifdef TMR_ENABLE_UHF
/**
 * Convert a hexadecimal string into an array of bytes. The string may
 * optionally include a "0x" prefix, which will be ignored. 
 *
 * @param hex The hexadecimal string to convert.
 * @param[out] bytes The array to store the converted bytes.
 * @param size The length of the byte array to store into.
 * @param[out] convertLen The number of bytes written to the array, or @c NULL.
 * 
 * @test If hex string is empty, *bytes should not be altered.
 * @test *convertLen is set to the correct value.
 * @test Function works when convertLen is @c NULL.
 * @test Specific values: @c "00", @c "0000", @c "ffff", @c "0xffff", @c "00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff00112233445566778899aabb".
 */
TMR_Status TMR_hexToBytes(const char *hex, uint8_t *bytes, uint32_t size, 
                          uint32_t *convertLen);
#endif /* TMR_ENABLE_UHF*/

/**
 * Convert an array of bytes, such as a tag EPC, into a hexadecimal string.
 *
 * @param bytes The byte array to convert.
 * @param size The length of the byte array to convert.
 * @param[out] hex The converted hexadecimal string, null-terminated.
 *
 * @test If size is 0, *hex should point to an empty string.
 */
void TMR_bytesToHex(const uint8_t *bytes, uint32_t size, char *hex);

/**
 * Defines the constants that may be combined to form the metadata
 * bitmask argument to the TMR_SR_cmdReadTagSingle,
 * TMR_SR_cmdReadTagData, and TMR_SR_cmdGetTagBuffer methods.
 */
typedef enum TMR_TRD_MetadataFlag
{
  TMR_TRD_METADATA_FLAG_NONE      = 0x0000,
  TMR_TRD_METADATA_FLAG_READCOUNT = 0x0001,
  TMR_TRD_METADATA_FLAG_RSSI      = 0x0002,
  TMR_TRD_METADATA_FLAG_ANTENNAID = 0x0004,
  TMR_TRD_METADATA_FLAG_FREQUENCY = 0x0008,
  TMR_TRD_METADATA_FLAG_TIMESTAMP = 0x0010,
  TMR_TRD_METADATA_FLAG_PHASE     = 0x0020,
  TMR_TRD_METADATA_FLAG_PROTOCOL  = 0x0040,
  TMR_TRD_METADATA_FLAG_DATA      = 0x0080,
  TMR_TRD_METADATA_FLAG_GPIO_STATUS = 0x0100,
  TMR_TRD_METADATA_FLAG_GEN2_Q    = 0x0200,
  TMR_TRD_METADATA_FLAG_GEN2_LF   = 0x0400,
  TMR_TRD_METADATA_FLAG_GEN2_TARGET = 0x0800,
  TMR_TRD_METADATA_FLAG_BRAND_IDENTIFIER = 0x1000,
#ifdef TMR_ENABLE_HF_LF
  TMR_TRD_METADATA_FLAG_TAGTYPE   = 0x2000,
#endif /* TMR_ENABLE_HF_LF */
  TMR_TRD_METADATA_FLAG_MAX = 0x2000,
  /* ALL */
  TMR_TRD_METADATA_FLAG_ALL       =    (
#ifdef TMR_ENABLE_UHF
                                        TMR_TRD_METADATA_FLAG_RSSI |
                                        TMR_TRD_METADATA_FLAG_FREQUENCY |
                                        TMR_TRD_METADATA_FLAG_PHASE |
                                        TMR_TRD_METADATA_FLAG_GPIO_STATUS |
                                        TMR_TRD_METADATA_FLAG_GEN2_Q |
                                        TMR_TRD_METADATA_FLAG_GEN2_LF |
                                        TMR_TRD_METADATA_FLAG_GEN2_TARGET |
#endif /* TMR_ENABLE_UHF */
                                        TMR_TRD_METADATA_FLAG_READCOUNT |
                                        TMR_TRD_METADATA_FLAG_ANTENNAID |
                                        TMR_TRD_METADATA_FLAG_TIMESTAMP |
                                        TMR_TRD_METADATA_FLAG_PROTOCOL |
										TMR_TRD_METADATA_FLAG_DATA
#ifdef TMR_ENABLE_HF_LF
                                        | TMR_TRD_METADATA_FLAG_TAGTYPE
#endif /* TMR_ENABLE_HF_LF */
							          )
} TMR_TRD_MetadataFlag;

/**
 * A class to represent a read of an RFID tag. Provides access to the
 TMR_TRD_METADATA_FLAG_FREQUENCY tag structure and the metadata of the read event, such as the time
 * of the read, the antenna that read the tag, and the number of times
 * the tag was seen by the air protocol.
 */
typedef struct TMR_TagReadData
{
  /** The tag that was read */
  TMR_TagData tag;
#ifdef TMR_ENABLE_UHF
  union
  {
    /** Gen2 specific data from tag response */
    TMR_GEN2_TagReadData gen2;
  }u;
#endif /* TMR_ENABLE_UHF */
  /** The set of metadata items below that are valid */
  uint16_t metadataFlags;
#ifdef TMR_ENABLE_UHF
  /** Tag response phase */
  uint16_t phase;
#endif /* TMR_ENABLE_UHF*/
  /** Antenna where the tag was read */
  uint8_t antenna;
#ifdef TMR_ENABLE_UHF
  /** State of GPIO pins at the moment of the tag read */ 
  TMR_GpioPin gpio[16];
   /** the actual number of available gpio pins on module*/
  uint8_t gpioCount;
#endif /* TMR_ENABLE_UHF */
  /** Number of times the tag was read */
  uint32_t readCount;
#ifdef TMR_ENABLE_UHF
  /** Strength of the signal received from the tag */
  int32_t rssi;
  /** RF carrier frequency the tag was read with */
  uint32_t frequency;
  /** Decoded Ucode8 brand identifier */
  uint8_t brandIdentifier[2];
#endif /* TMR_ENABLE_UHF */
  /** Absolute time of the read (32 least-significant bits), in milliseconds since 1/1/1970 UTC */
  uint32_t timestampLow;
  /** Absolute time of the read (32 most-significant bits), in milliseconds since 1/1/1970 UTC */
  uint32_t timestampHigh;
  /** Data read from the tag */
  TMR_uint8List data;
#ifdef TMR_ENABLE_UHF
  /** Read EPC bank data bytes */
  TMR_uint8List epcMemData;
  /** Read TID bank data bytes */
  TMR_uint8List tidMemData;
  /** Read USER bank data bytes */
  TMR_uint8List userMemData;
  /** Read RESERVED bank data bytes */
  TMR_uint8List reservedMemData;
  /** [PRIVATE] Relative time of the read within the read interval, in microseconds (for internal use only,
   * this value is used to calculate timestampLow and timestampHigh)
   **/
#endif /* TMR_ENABLE_UHF */
  uint32_t dspMicros;
#ifdef TMR_ENABLE_HF_LF
  /**
   * Holds the HF/LF protocol supported tagtype value 
   * returned as part of metadata.
   */
  uint64_t tagType;
#endif /* TMR_ENABLE_HF_LF */
#if TMR_MAX_EMBEDDED_DATA_LENGTH
  /** Preallocated storage for data */
  uint8_t _dataList[TMR_MAX_EMBEDDED_DATA_LENGTH];
#ifdef TMR_ENABLE_UHF
  uint8_t _epcMemDataList[TMR_MAX_EMBEDDED_DATA_LENGTH];
  uint8_t _tidMemDataList[TMR_MAX_EMBEDDED_DATA_LENGTH];
  uint8_t _userMemDataList[TMR_MAX_EMBEDDED_DATA_LENGTH];
  uint8_t _reservedMemDataList[TMR_MAX_EMBEDDED_DATA_LENGTH];
#endif /* TMR_ENABLE_UHF */
#endif 
  /** Reader instance, to keep track of tag reads */
  TMR_Reader *reader;
} TMR_TagReadData;

TMR_Status TMR_TRD_init(TMR_TagReadData *trd);
TMR_Status TMR_TRD_init_data(TMR_TagReadData *trd, uint16_t size, uint8_t *buf);
#ifdef TMR_ENABLE_UHF
TMR_Status TMR_TRD_MEMBANK_init_data(TMR_uint8List *data, uint16_t size, uint8_t *buf);
#endif /* TMR_ENABLE_UHF */

#ifdef  __cplusplus
}
#endif

#endif /* __TAG_DATA_H */
