#ifndef _SERIAL_READER_IMP_H
#define _SERIAL_READER_IMP_H

/**
 *  @file serial_reader_imp.h
 *  @brief Serial reader internal implementation header
 *  @author Nathan Williams
 *  @date 10/28/2009
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
#include "tm_reader.h"
#include "tmr_status.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum TMR_SR_OpCode
{
#ifdef TMR_ENABLE_UHF
  TMR_SR_OPCODE_WRITE_FLASH             = 0x01,
  TMR_SR_OPCODE_READ_FLASH              = 0x02,
#endif /* TMR_ENABLE_UHF */
  TMR_SR_OPCODE_VERSION                 = 0x03,
  TMR_SR_OPCODE_BOOT_FIRMWARE           = 0x04,
  TMR_SR_OPCODE_SET_BAUD_RATE           = 0x06,
  TMR_SR_OPCODE_ERASE_FLASH             = 0x07,
#ifdef TMR_ENABLE_UHF
  TMR_SR_OPCODE_VERIFY_IMAGE_CRC        = 0x08,
#endif  /* TMR_ENABLE_UHF */
  TMR_SR_OPCODE_BOOT_BOOTLOADER         = 0x09,
  TMR_SR_OPCODE_HW_VERSION              = 0x10,
#ifdef TMR_ENABLE_UHF
  TMR_SR_OPCODE_MODIFY_FLASH            = 0x0A,
  TMR_SR_OPCODE_GET_DSP_SILICON_ID      = 0x0B,
#endif /* TMR_ENABLE_UHF */
  TMR_SR_OPCODE_GET_CURRENT_PROGRAM     = 0x0C,
  TMR_SR_OPCODE_WRITE_FLASH_SECTOR      = 0x0D,
  TMR_SR_OPCODE_GET_SECTOR_SIZE         = 0x0E,
  TMR_SR_OPCODE_MODIFY_FLASH_SECTOR     = 0x0F,
  TMR_SR_OPCODE_READ_TAG_ID_SINGLE      = 0x21,
  TMR_SR_OPCODE_READ_TAG_ID_MULTIPLE    = 0x22,
#ifdef TMR_ENABLE_UHF
  TMR_SR_OPCODE_WRITE_TAG_ID            = 0x23,
#endif /* TMR_ENABLE_UHF */
  TMR_SR_OPCODE_WRITE_TAG_DATA          = 0x24,
#ifdef TMR_ENABLE_UHF
  TMR_SR_OPCODE_LOCK_TAG                = 0x25,
  TMR_SR_OPCODE_KILL_TAG                = 0x26,
#endif /* TMR_ENABLE_UHF */
  TMR_SR_OPCODE_READ_TAG_DATA           = 0x28,
  TMR_SR_OPCODE_PASS_THROUGH            = 0x27,
  TMR_SR_OPCODE_GET_TAG_ID_BUFFER       = 0x29,
  TMR_SR_OPCODE_CLEAR_TAG_ID_BUFFER     = 0x2A,
#ifdef TMR_ENABLE_UHF
  TMR_SR_OPCODE_WRITE_TAG_SPECIFIC      = 0x2D,
  TMR_SR_OPCODE_ERASE_BLOCK_TAG_SPECIFIC= 0x2E,
#endif /* TMR_ENABLE_UHF */
  TMR_SR_OPCODE_MULTI_PROTOCOL_TAG_OP   = 0x2F,
  TMR_SR_OPCODE_GET_ANTENNA_PORT        = 0x61,
  TMR_SR_OPCODE_GET_READ_TX_POWER       = 0x62,
  TMR_SR_OPCODE_GET_TAG_PROTOCOL        = 0x63,
  TMR_SR_OPCODE_GET_WRITE_TX_POWER      = 0x64,
#ifdef TMR_ENABLE_UHF
  TMR_SR_OPCODE_GET_FREQ_HOP_TABLE      = 0x65,
#endif /* TMR_ENABLE_UHF*/
  TMR_SR_OPCODE_GET_USER_GPIO_INPUTS    = 0x66,
  TMR_SR_OPCODE_GET_REGION              = 0x67,
  TMR_SR_OPCODE_GET_POWER_MODE          = 0x68,
#ifdef TMR_ENABLE_UHF
  TMR_SR_OPCODE_GET_USER_MODE           = 0x69,
#endif /* TMR_ENABLE_UHF */
  TMR_SR_OPCODE_GET_READER_OPTIONAL_PARAMS=0x6A,
  TMR_SR_OPCODE_GET_PROTOCOL_PARAM      = 0x6B,
  TMR_SR_OPCODE_GET_READER_STATS        = 0x6C,
#ifdef TMR_ENABLE_UHF
  TMR_SR_OPCODE_GET_USER_PROFILE        = 0x6D,
#endif /* TMR_ENABLE_UHF */
  TMR_SR_OPCODE_GET_AVAILABLE_PROTOCOLS = 0x70,
  TMR_SR_OPCODE_GET_AVAILABLE_REGIONS   = 0x71,
  TMR_SR_OPCODE_GET_TEMPERATURE         = 0x72,
  TMR_SR_OPCODE_SET_ANTENNA_PORT        = 0x91,
  TMR_SR_OPCODE_SET_READ_TX_POWER       = 0x92,
  TMR_SR_OPCODE_SET_TAG_PROTOCOL        = 0x93,
  TMR_SR_OPCODE_SET_WRITE_TX_POWER      = 0x94,
#ifdef TMR_ENABLE_UHF
  TMR_SR_OPCODE_SET_FREQ_HOP_TABLE      = 0x95,
#endif /* TMR_ENABLE_UHF */
  TMR_SR_OPCODE_SET_USER_GPIO_OUTPUTS   = 0x96,
  TMR_SR_OPCODE_SET_REGION              = 0x97,
  TMR_SR_OPCODE_SET_POWER_MODE          = 0x98,
#ifdef TMR_ENABLE_UHF
  TMR_SR_OPCODE_SET_USER_MODE           =  0x99,
#endif /* TMR_ENABLE_UHF */
  TMR_SR_OPCODE_SET_READER_OPTIONAL_PARAMS=0x9a,
  TMR_SR_OPCODE_SET_PROTOCOL_PARAM      = 0x9B,
  TMR_SR_OPCODE_SET_USER_PROFILE        = 0x9D,
  TMR_SR_OPCODE_SET_PROTOCOL_LICENSEKEY = 0x9E,
#ifdef TMR_ENABLE_UHF
  TMR_SR_OPCODE_SET_OPERATING_FREQ      = 0xC1,
  TMR_SR_OPCODE_TX_CW_SIGNAL            = 0xC3,
#endif /* TMR_ENABLE_UHF */
}TMR_SR_OpCode;

#ifdef TMR_ENABLE_HF_LF
typedef enum TMR_SR_SelectSingulationOptions
{
  TMR_SR_SINGULATION_OPTION_SELECT_ON_UID     = 0x01,
  TMR_SR_SINGULATION_OPTION_SELECT_ON_TAGTYPE = 0x02,
  TMR_SR_SINGULATION_OPTION_EXT_TAGOP_PARAMS  = 0x04,
}TMR_SR_SelectSingulationOptions;
#endif /* TMR_ENABLE_HF_LF */

typedef enum TMR_SR_Gen2
{
#ifdef TMR_ENABLE_UHF
  TMR_SR_GEN2_SINGULATION_OPTION_SELECT_DISABLED         = 0x00,
  TMR_SR_GEN2_SINGULATION_OPTION_SELECT_ON_EPC           = 0x01,
  TMR_SR_GEN2_SINGULATION_OPTION_SELECT_ON_TID           = 0x02,
  TMR_SR_GEN2_SINGULATION_OPTION_SELECT_ON_USER_MEM      = 0x03,
  TMR_SR_GEN2_SINGULATION_OPTION_SELECT_ON_ADDRESSED_EPC = 0x04,
  TMR_SR_GEN2_SINGULATION_OPTION_USE_PASSWORD            = 0x05,
  TMR_SR_GEN2_SINGULATION_OPTION_SELECT_ON_LENGTH_OF_EPC = 0x06,
  TMR_SR_GEN2_SINGULATION_OPTION_SELECT_GEN2TRUNCATE	 = 0x07,
  TMR_SR_GEN2_SINGULATION_OPTION_INVERSE_SELECT_BIT      = 0x08,
#endif /* TMR_ENABLE_UHF */
  TMR_SR_GEN2_SINGULATION_OPTION_FLAG_METADATA           = 0x10,
#ifdef TMR_ENABLE_UHF
  TMR_SR_GEN2_SINGULATION_OPTION_EXTENDED_DATA_LENGTH    = 0x20,
#endif /* TMR_ENABLE_UHF */
  TMR_SR_GEN2_SINGULATION_OPTION_SECURE_READ_DATA        = 0x40
}TMR_SR_Gen2SingulationOptions;

#ifdef TMR_ENABLE_UHF
typedef enum TMR_SR_TagidOption
{
  TMR_SR_TAG_ID_OPTION_NONE    = 0x00,
  TMR_SR_TAG_ID_OPTION_REWIND  = 0x01
}TMR_SR_TagidOption;
#endif /* TMR_ENABLE_UHF */

typedef enum TMR_SR_ModelHardwareID
{
  TMR_SR_MODEL_M6E         = 0x18,
  TMR_SR_MODEL_M6E_I	   = 0x19,
  TMR_SR_MODEL_MICRO       = 0x20,
  TMR_SR_MODEL_M6E_NANO    = 0x30,
  TMR_SR_MODEL_M7E_PICO    = 0x38,
  TMR_SR_MODEL_M3E         = 0x80,
  TMR_SR_MODEL_UNKNOWN     = 0xFF,
} TMR_SR_ModelHardwareID;

#ifdef TMR_ENABLE_HF_LF
typedef enum TMR_SR_ModelM3E
{
  TMR_SR_MODEL_M3E_REV1 = 0x01
}TMR_SR_ModelM3E;
#endif /* TMR_ENABLE_HF_LF */

#ifdef TMR_ENABLE_UHF
typedef enum TMR_SR_ModelMicro
{
  TMR_SR_MODEL_M6E_MICRO     = 0x01,
  TMR_SR_MODEL_M6E_MICRO_USB = 0x02,
  TMR_SR_MODEL_M6E_MICRO_USB_PRO = 0x03,
}TMR_SR_ModelMicro;

typedef enum TMR_SR_ModelM6E_I
{
  TMR_SR_MODEL_M6E_I_REV1 = 0x01,
  TMR_SR_MODEL_M6E_I_PRC  = 0x02,
  TMR_SR_MODEL_M6E_I_JIC  = 0x03,
}TMR_SR_ModelM6E_I;
#endif /* TMR_ENABLE_UHF */

typedef enum TMR_SR_ProductGroupID
{
  TMR_SR_PRODUCT_MODULE = 0,
  TMR_SR_PRODUCT_RUGGEDIZED_READER = 1,
  TMR_SR_PRODUCT_USB_READER = 2,
  TMR_SR_PRODUCT_INVALID = 0xFFFF,
}TMR_SR_ProductGroupID;

#ifdef TMR_ENABLE_UHF
typedef enum TMR_SR_TagOperationByte
{
  TMR_SR_TAGOP_BAP = 0x81,
  TMR_SR_TAGOP_READ_AFTER_WRITE = 0x84,
  TMR_SR_TAGOP_MULTI_SELECT = 0x88,
}TMR_SR_TagOperationByte;
#endif /* TMR_ENABLE_UHF */

#ifdef TMR_ENABLE_UHF
typedef enum TMR_SR_EndOfSelect
{
  TMR_SR_END_OF_SELECT_INDICATOR = 0x00,
}TMR_SR_EndOfSelect;
#endif /* TMR_ENABLE_UHF */

TMR_Status TMR_SR_sendTimeout(TMR_Reader *reader, uint8_t *data,
                              uint32_t timeoutMs);
TMR_Status TMR_SR_send(TMR_Reader *reader, uint8_t *data);
TMR_Status TMR_SR_sendCmd(TMR_Reader *reader, uint8_t *data, uint8_t i);
TMR_Status TMR_SR_sendMessage(TMR_Reader *reader, uint8_t *data,
                              uint8_t *opcode, uint32_t timeoutMs);
TMR_Status TMR_SR_receiveMessage(TMR_Reader *reader, uint8_t *data,
                                 uint8_t opcode, uint32_t timeoutMs);
TMR_Status TMR_SR_receiveAutonomousReading(struct TMR_Reader *reader, TMR_TagReadData *trd, TMR_Reader_StatsValues *stats);
void TMR_SR_parseMetadataFromMessage(TMR_Reader *reader, TMR_TagReadData *read, uint16_t flags,
                                     uint8_t *i, uint8_t msg[]);
#ifdef TMR_ENABLE_UHF
void extractGen2MemoryBankValues(TMR_TagReadData *read);
#endif /* TMR_ENABLE_UHF */

void
TMR_SR_parseMetadataOnly(TMR_Reader *reader, TMR_TagReadData *read, uint16_t flags,
                                uint8_t *i, uint8_t msg[]);
void TMR_SR_postprocessReaderSpecificMetadata(TMR_TagReadData *read,
                                              TMR_SR_SerialReader *sr);
bool isContinuousReadParamSupported(TMR_Reader *reader);

/**
 * This structure is returned from read tag multiple embedded commands.
 */
typedef struct TMR_SR_MultipleStatus
{
  /** The number of tags found during the read. */
  uint16_t tagsFound;
  /** The number of tags for which the embedded operation succeeded. */
  uint16_t successCount;
  /** The number of tags for which the embedded operation failed. */
  uint16_t failureCount;
}TMR_SR_MultipleStatus;

/**
 * This structure is returned from TMR_SR_cmdGetTxRxPorts and
 * TMR_SR_cmdGetAntennaSearchList, and passed as a parameter to
 * TMR_SR_cmdSetAntennaSearchList.
 */
typedef struct TMR_SR_PortPair
{
  /** The transmit port. */
  uint8_t txPort;
  /** The receive port. */
  uint8_t rxPort;
}TMR_SR_PortPair;

/**
 * This structure is returned from TMR_SR_cmdAntennaDetect.
 */
typedef struct TMR_SR_PortDetect
{
  /** The port number. */
  uint8_t port;
  /** Whether an antenna was detected on the port. */
  bool detected;
}TMR_SR_PortDetect;

/**
 * Reader statistics options enum
 */ 
typedef enum TMR_SR_ReaderStatsOption
{
  /* Get statistics specified by the statistics flag */
  TMR_SR_READER_STATS_OPTION_GET=   0x00,
  /* Reset the specified statistic */
  TMR_SR_READER_STATS_OPTION_RESET= 0x01,
  /* Get the per-port statistics specified by the statistics flag*/
  TMR_SR_READER_STATS_OPTION_GET_PER_PORT = 0x02,
}TMR_SR_ReaderStatsOption;
  
#ifdef TMR_ENABLE_UHF
/**
 *  Reader Statistics Flag Enum
 */
typedef enum TMR_SR_ReaderStatisticsFlag
{
  /* Total time the port has been transmitting, in milliseconds. Resettable */
  TMR_SR_READER_STATS_FLAG_RF_ON_TIME     = (1<<0),
  /* Detected noise floor with transmitter off. Recomputed when requested, not resettable.*/
  TMR_SR_READER_STATS_FLAG_NOISE_FLOOR    = (1<<1),
  /* Detected noise floor with transmitter on. Recomputed when requested, not resettable. */
  TMR_SR_READER_STATS_FLAG_NOISE_FLOOR_TX_ON = (1<<3),
  /* ALL */
  TMR_SR_READER_STATS_ALL = (TMR_SR_READER_STATS_FLAG_RF_ON_TIME | 
      TMR_SR_READER_STATS_FLAG_NOISE_FLOOR |
      TMR_SR_READER_STATS_FLAG_NOISE_FLOOR_TX_ON),
}TMR_SR_ReaderStatisticsFlag;
#endif /* TMR_ENABLE_UHF */
/**
 * Antenna Configuration. Returned by TMR_SR_cmdGetAntennaConfiguration.
 */
typedef struct TMR_SR_AntennaPort
{
  /** The number of physical antenna ports. */
  uint8_t numPorts;
  /** The current logical Transmit port. */
  uint8_t txPort;
  /** The current logical receive port. */
  uint8_t rxPort;
  /** List specifying what ports are terminated. */
  TMR_uint8List portTerminatedList;
}TMR_SR_AntennaPort;

#ifdef TMR_ENABLE_UHF
/** Per-port power levels. Used by TMR_SR_cmd{Get,Set}AntennaPortPowers */
typedef struct TMR_SR_PortPower
{
  /** The port number. */
  uint8_t port;
  /** The power level to use for read operations, in centidBm. */
  uint16_t readPower;
  /** The power level to use for write operations, in centidBm. */
  uint16_t writePower;
} TMR_SR_PortPower;

/**
 * Per-port power levels and settling times. Used by
 * TMR_SR_cmd{Get,Set}AntennaPortPowersAndSettlingTime.
 */
typedef struct TMR_SR_PortPowerAndSettlingTime
{
  /** The port number. */
  uint8_t port;
  /** The power level to use for read operations, in centidBm. */
  int16_t readPower;
  /** The power level to use for write operations, in centidBm. */
  int16_t writePower;
  /** The duration to wait after switching to this port, in microseconds. */
  uint16_t settlingTime;
} TMR_SR_PortPowerAndSettlingTime;
#endif /* TMR_ENABLE_UHF */

/** The current power level and the bounds of the power level range. */
typedef struct TMR_SR_PowerWithLimits
{
  /** The current power level, in centidBm. */
  uint16_t setPower;
  /** The maximum power level, in centidBm. */
  uint16_t maxPower;
  /** The minimum power level, in centidBm. */
  uint16_t minPower;
} TMR_SR_PowerWithLimits;


void TMR_hexDottedQuad(const uint8_t bytes[4], char buf[12]);
#ifdef TMR_ENABLE_UHF
TMR_Status TMR_hexDottedQuadToUint32(const char bytes[12], uint32_t *result);
#endif /* TMR_ENABLE_UHF */


/**
 * This type enumerates the serial reader configuration parameters for
 * TMR_SR_cmdSetReaderConfiguration and
 * TMR_SR_cmdGetReaderConfiguration.  Each value is
 * associated with a particular data type for the setting.
 */
typedef enum TMR_SR_Configuration
{
#ifdef TMR_ENABLE_UHF
  /**
   *  Key tag buffer records off of antenna ID as well as EPC;
   *  i.e., keep separate records for the same EPC read on different antennas
   *  0: Disable -- Different antenna overwrites previous record.
   *  1: Enable -- Different Antenna creates a new record.
   */
  TMR_SR_CONFIGURATION_UNIQUE_BY_ANTENNA        = 0,
  /**
   *  Run transmitter in lower-performance, power-saving mode.
   *  0: Disable -- Higher transmitter bias for improved reader sensitivity
   *  1: Enable -- Lower transmitter bias sacrifices sensitivity for power consumption
   */
  TMR_SR_CONFIGURATION_TRANSMIT_POWER_SAVE      = 1,
#endif /* TMR_ENABLE_UHF */
  /**
   *  Configure GPOs to drive antenna switch.
   *  0: No switch
   *  1: Switch on GPO1
   *  2: Switch on GPO2
   *  3: Switch on GPO1,GPO2
   */
  TMR_SR_CONFIGURATION_ANTENNA_CONTROL_GPIO     = 3,
#ifdef TMR_ENABLE_UHF
  /**
   *  Refuse to transmit if antenna is not detected
   */
  TMR_SR_CONFIGURATION_SAFETY_ANTENNA_CHECK     = 4,
  /**
   *  Refuse to transmit if overtemperature condition detected
   */
  TMR_SR_CONFIGURATION_SAFETY_TEMPERATURE_CHECK = 5,
  /**
   *  If tag read duplicates an existing tag buffer record (key is the same),
   *  update the record's timestamp if incoming read has higher RSSI reading.
   *  0: Keep timestamp of record's first read
   *  1: Keep timestamp of read with highest RSSI
   */
  TMR_SR_CONFIGURATION_RECORD_HIGHEST_RSSI      = 6,
  /**
   *  Key tag buffer records off tag data as well as EPC;
   *  i.e., keep separate records for the same EPC read with different data
   *  0: Disable -- Different data overwrites previous record.
   *  1: Enable -- Different data creates new record.
   */
  TMR_SR_CONFIGURATION_UNIQUE_BY_DATA           = 8,
  /**
   *  Self jammer cancellation
   *  User can enable/disable through level2 API
   */
  TMR_SR_CONFIGURATION_SELF_JAMMER_CANCELLATION = 0x0A,
  /**
   *  Key tag buffer records off of protocol as well as EPC;
   *  i.e., keep separate records for the same EPC read on different protocols
   *  0: Disable -- Different protocol overwrites previous record.
   *  1: Enable -- Different protocol creates a new record.
   */
  TMR_SR_CONFIGURATION_UNIQUE_BY_PROTOCOL       = 0x0B,
#endif /* TMR_ENABLE_UHF */
  /**
   *  Enable read filtering
   */
  TMR_SR_CONFIGURATION_ENABLE_READ_FILTER         = 0x0C,
#ifdef TMR_ENABLE_UHF
  /**
   *  Tag buffer entry timeout
   */
  TMR_SR_CONFIGURATION_READ_FILTER_TIMEOUT        = 0x0D,
  /**
    * Transport (bus) type
    **/ 
  TMR_SR_CONFIGURATION_CURRENT_MSG_TRANSPORT      = 0x0E,
  /**
   * Enable the CRC calculation
   */
  TMR_SR_CONFIGURATION_SEND_CRC               = 0x1B,
#endif /* TMR_ENABLE_UHF */
  /**
   *  General category of finished reader into which module is integrated; e.g.,
   *  0: bare module
   *  1: In-vehicle Reader (e.g., Tool Link, Vega)
   *  2: USB Reader
   */
  TMR_SR_CONFIGURATION_PRODUCT_GROUP_ID         = 0x12,
  /**
   * Product ID (Group ID 0x0002 ) information
   * 0x0001 :M5e-C USB reader
   * 0x0002 :Backback NA antenna
   * 0x0003 :Backback EU antenna
   **/
  TMR_SR_CONFIGURATION_PRODUCT_ID              = 0x13,
  /**
   *  Configure GPIs to drive trigger read.
   *  0: No switch
   *  1: Switch on GPI1
   *  2: Switch on GPI2
   *  3: Switch on GPI3 (if supported)
   *  4: Switch on GOI4 (if supported)
   */
  TMR_SR_CONFIGURATION_TRIGGER_READ_GPIO                     = 0x1E,

#ifdef TMR_ENABLE_HF_LF
  TMR_SR_CONFIGURATION_KEEP_RF_ON  =  0x21,
#endif /* TMR_ENABLE_HF_LF */

} TMR_SR_Configuration;

#ifdef TMR_ENABLE_UHF
/**
 * This type enumerates the region configuration parameters for
 * the TMR_SR_cmdGetRegionConfiguration() command.
 */
typedef enum TMR_SR_RegionConfiguration
{
  TMR_SR_REGION_CONFIGURATION_LBT_ENABLED = 0x40,
  TMR_SR_REGION_CONFIGURATION_LBTTHRESHOLD = 0x41,
  TMR_SR_REGION_CONFIGURATION_DWELLTIME_ENABLED = 0x42,
  TMR_SR_REGION_CONFIGURATION_DWELLTIME = 0x43,
} TMR_SR_RegionConfiguration;

/**
 * This is the enumeration of Gen2-specific configuration values. Each
 * enumerated value is associated with a particular data type for the
 * setting value.
 */
typedef enum TMR_SR_Gen2Configuration
{
  TMR_SR_GEN2_CONFIGURATION_SESSION = 0x00,
  TMR_SR_GEN2_CONFIGURATION_TARGET  = 0x01,
  TMR_SR_GEN2_CONFIGURATION_TAGENCODING = 0x02,
  TMR_SR_GEN2_CONFIGURATION_LINKFREQUENCY = 0x10,
  TMR_SR_GEN2_CONFIGURATION_TARI    = 0x11,
  TMR_SR_GEN2_CONFIGURATION_Q       = 0x12,
  TMR_SR_GEN2_CONFIGURATION_BAP    = 0x13,
  TMR_SR_GEN2_CONFIGURATION_PROTCOLEXTENSION = 0x14,
  /* The Gen2 T4 is the minimum time between Select and Query command.
   * It is a 4-byte value specified in microseconds.
   * Type: uint32_t
  */
  TMR_SR_GEN2_CONFIGURATION_T4 = 0x15,
  /* option to set/get "Initial Q" */
  TMR_SR_GEN2_INITIAL_Q = 0x16,
  /* option to set/get Send-Select-With-Every-Query flag */
  TMR_SR_GEN2_SEND_SELECT = 0x17
} TMR_SR_Gen2Configuration;

/**
 * This struture is retuned from TMR_SR_Gen2ReaderResponseTimeOut
 */
typedef struct TMR_SR_Gen2ReaderWriteTimeOut
{
  /* Status of reader timeout */
  bool earlyexit;

  /* Timeout value used for write operation*/
  uint16_t writetimeout;
}TMR_SR_Gen2ReaderWriteTimeOut;
/**
 * This is the enumeration of ISO 18000-6B-specific configuration
 * values. Each enumerated value is associated with a particular data
 * type for the setting value.
 */
typedef enum TMR_SR_Iso180006bConfiguration
{
  TMR_SR_ISO180006B_CONFIGURATION_LINKFREQUENCY = 0x10,
  TMR_SR_ISO180006B_CONFIGURATION_MODULATION_DEPTH = 0x11,
  TMR_SR_ISO180006B_CONFIGURATION_DELIMITER = 0x12
} TMR_SR_Iso180006bConfiguration;

/**
 * This is the enumeration of iPx-specific configuration
 * values. Each enumerated value is associated with a particular data
 * type for the setting value.
 */
typedef enum TMR_SR_iPxConfiguration
{
  TMR_SR_IPX_CONFIGURATION_LINKFREQUENCY = 0x10
} TMR_SR_iPxConfiguration;

#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_HF_LF

typedef enum TMR_NHX_Type
{
  NHX_TYPE_NONE  = 0,
  NHX_TYPE_10022 = 1,
}TMR_NHX_Type;

//Tag features.
typedef enum TMR_SupportedTagFeatures
{
  TMR_HF_HID_ICLASS_SE_SECURE_RD = 0x00000001,
  TMR_LF_HID_PROX_SECURE_RD = 0x00000010,
} TMR_SupportedTagFeatures;

/**
 * This is the enumeration of ISO14443A-specific configuration values.
 * Each enumerated value is associated with a particular data
 * type for the setting value.
 */
typedef enum TMR_SR_Iso14443aConfiguration
{
  TMR_SR_ISO14443A_CONFIGURATION_SUPPORTED_TAGTYPES = 0x01,
  TMR_SR_ISO14443A_CONFIGURATION_TAGTYPE = 0x02,
  TMR_SR_ISO14443A_CONFIGURATION_SUPPORTED_FEATURES = 0x03,
} TMR_SR_Iso14443aConfiguration;

/**
 * This is the enumeration of ISO14443B-specific configuration values.
 * Each enumerated value is associated with a particular data
 * type for the setting value.
 */
typedef enum TMR_SR_Iso14443bConfiguration
{
  TMR_SR_ISO14443B_CONFIGURATION_SUPPORTED_TAGTYPES = 0x01,
  TMR_SR_ISO14443B_CONFIGURATION_TAGTYPE = 0x02
} TMR_SR_Iso14443bConfiguration;

/**
 * This is the enumeration of ISO15693-specific configuration values.
 * Each enumerated value is associated with a particular data
 * type for the setting value.
 */
typedef enum TMR_SR_Iso15693Configuration
{
  TMR_SR_ISO15693_CONFIGURATION_SUPPORTED_TAGTYPES = 0x01,
  TMR_SR_ISO15693_CONFIGURATION_TAGTYPE = 0x02,
  TMR_SR_ISO15693_CONFIGURATION_SUPPORTED_FEATURES = 0x03,
} TMR_SR_Iso15693Configuration;

/**
 * This is the enumeration of LF125KHZ-specific configuration values.
 * Each enumerated value is associated with a particular data
 * type for the setting value.
 */
typedef enum TMR_SR_LF125khzConfiguration
{
  TMR_SR_LF125KHZ_CONFIGURATION_SUPPORTED_TAGTYPES = 0x01,
  TMR_SR_LF125KHZ_CONFIGURATION_TAGTYPE            = 0x02,
  TMR_SR_LF125KHZ_CONFIGURATION_SUPPORTED_FEATURES = 0x03,
  TMR_SR_LF125KHZ_CONFIGURATION_SECURE_RD_FORMAT   = 0x04,
} TMR_SR_LF125khzConfiguration;

/**
 * This is the enumeration of LF134KHZ-specific configuration values.
 * Each enumerated value is associated with a particular data
 * type for the setting value.
 */
typedef enum TMR_SR_LF134khzConfiguration
{
  TMR_SR_LF134KHZ_CONFIGURATION_SUPPORTED_TAGTYPES = 0x01,
  TMR_SR_LF134KHZ_CONFIGURATION_TAGTYPE = 0x02
} TMR_SR_LF134khzConfiguration;
#endif /* TMR_ENABLE_HF_LF */

/**
 * This type selects the protocol configuration option for
 * TMR_SR_cmdSetProtocolConfiguration() and
 * TMR_SR_cmdGetProtocolConfiguration().  Each value is associated
 * with a particular data type for the setting.
 */
typedef struct TMR_SR_ProtocolConfiguration
{
  /** The protocol to configure. Determines which union member is valid. */
  TMR_TagProtocol protocol;
  union
  {
#ifdef TMR_ENABLE_UHF
    /** The configuration key for a Gen2 option. */
    TMR_SR_Gen2Configuration gen2;
    /** The configuration key for an ISO18000-6B option. */
    TMR_SR_Iso180006bConfiguration iso180006b;
    TMR_SR_iPxConfiguration ipx;
#endif /* TMR_ENABLE_UHF */

#ifdef TMR_ENABLE_HF_LF
    /** The configuration key for an ISO14443A option. */
    TMR_SR_Iso14443aConfiguration iso14443a;
    /** The configuration key for an ISO14443B option. */
    TMR_SR_Iso14443bConfiguration iso14443b;
    /** The configuration key for an ISO15693 option. */
    TMR_SR_Iso15693Configuration iso15693;
    /** The configuration key for an LF125KHZ option. */
    TMR_SR_LF125khzConfiguration lf125khz;
    /** The configuration key for an LF134KHZ option. */
    TMR_SR_LF134khzConfiguration lf134khz;
#endif /* TMR_ENABLE_HF_LF */
  }u;
} TMR_SR_ProtocolConfiguration;


/**
 * Defines the values for search flags for TMR_SR_cmdReadTagMultiple.
 */
typedef enum TMR_SR_SearchFlag
{
  TMR_SR_SEARCH_FLAG_CONFIGURED_ANTENNA = 0,
  TMR_SR_SEARCH_FLAG_ANTENNA_1_THEN_2   = 1,
  TMR_SR_SEARCH_FLAG_ANTENNA_2_THEN_1   = 2,
  TMR_SR_SEARCH_FLAG_CONFIGURED_LIST    = 3,
  TMR_SR_SEARCH_FLAG_ANTENNA_MASK       = 3,
  TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND   = 4,
  TMR_SR_SEARCH_FLAG_TAG_STREAMING      = 8,
  TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT = 16,
  TMR_SR_SEARCH_FLAG_STATUS_REPORT_STREAMING = 32,
  TMR_SR_SEARCH_FLAG_RETURN_ON_N_TAGS = 64,
  TMR_SR_SEARCH_FLAG_READ_MULTIPLE_FAST_SEARCH = 128,
  TMR_SR_SEARCH_FLAG_STATS_REPORT_STREAMING = 256,
  TMR_SR_SEARCH_FLAG_GPI_TRIGGER_READ = 512,
  TMR_SR_SEARCH_FLAG_DUTY_CYCLE_CONTROL = 1024,

#ifdef TMR_ENABLE_HF_LF
  TMR_SR_SEARCH_FLAG_DYNAMIC_PROTOCOL_SWITCHING = 2048, //0x0800
#endif /* TMR_ENABLE_HF_LF */
}TMR_SR_SearchFlag;

#ifdef TMR_ENABLE_UHF
typedef enum TMR_SR_ISO180006BCommands
{
  TMR_SR_ISO180006B_COMMAND_DATA_READ           = 0x0B,
  TMR_SR_ISO180006B_COMMAND_READ                = 0x0C,
  TMR_SR_ISO180006B_COMMAND_WRITE               = 0x0D,
  TMR_SR_ISO180006B_COMMAND_WRITE_MULTIPLE      = 0x0E,
  TMR_SR_ISO180006B_COMMAND_WRITE4BYTE          = 0x1B,
  TMR_SR_ISO180006B_COMMAND_WRITE4BYTE_MULTIPLE = 0x1C,
} TMR_SR_ISO180006BCommands;

typedef enum TMR_SR_ISO180006BCommandOptions
{
  TMR_SR_ISO180006B_WRITE_OPTION_READ_AFTER           = 0x00,
  TMR_SR_ISO180006B_WRITE_OPTION_NO_VERIFY            = 0x01,
  TMR_SR_ISO180006B_WRITE_OPTION_READ_VERIFY_AFTER    = 0x02,
  TMR_SR_ISO180006B_WRITE_OPTION_GROUP_SELECT         = 0x03,
  TMR_SR_ISO180006B_WRITE_OPTION_COUNT_PROVIDED       = 0x08,
  TMR_SR_ISO180006B_WRITE_LOCK_NO                     = 0x00,
  TMR_SR_ISO180006B_WRITE_LOCK_YES                    = 0x01,
  TMR_SR_ISO180006B_LOCK_OPTION_TYPE_FOLLOWS          = 0x01,
  TMR_SR_ISO180006B_LOCK_TYPE_QUERYLOCK_THEN_LOCK     = 0x01,
} TMR_SR_ISO180006BCommandOptions;

TMR_Status TMR_SR_cmdRaw(TMR_Reader *reader, uint32_t timeout, uint8_t msgLen,
            uint8_t msg[]);
#endif /* TMR_ENABLE_UHF */

TMR_Status
TMR_SR_boot(TMR_Reader *reader, uint32_t currentBaudRate);

TMR_Status TMR_SR_setSerialBaudRate(TMR_Reader *reader, uint32_t rate);
TMR_Status TMR_SR_cmdVersion(TMR_Reader *reader, TMR_SR_VersionInfo *info);
TMR_Status TMR_SR_cmdBootFirmware(TMR_Reader *reader);
TMR_Status TMR_SR_cmdSetBaudRate(TMR_Reader *reader, uint32_t rate);
TMR_Status TMR_SR_cmdEraseFlash(TMR_Reader *reader, uint8_t sector, 
            uint32_t password);
TMR_Status TMR_SR_cmdWriteFlashSector(TMR_Reader *reader, uint8_t sector, 
            uint32_t address, uint32_t password, uint8_t length,
            const uint8_t data[], uint32_t offset);
#ifdef TMR_ENABLE_UHF
TMR_Status TMR_SR_cmdVerifyImage(TMR_Reader *reader, bool *status);
TMR_Status TMR_SR_cmdModifyFlashSector(TMR_Reader *reader, uint8_t sector, 
            uint32_t address, uint32_t password, uint8_t length,
            const uint8_t data[], uint32_t offset);
#endif /* TMR_ENABLE_UHF */
TMR_Status TMR_SR_cmdBootBootloader(TMR_Reader *reader);
TMR_Status TMR_SR_cmdGetHardwareVersion(TMR_Reader *reader, uint8_t option,
            uint8_t flags, uint8_t* count, uint8_t data[]);
TMR_Status TMR_SR_cmdGetCurrentProgram(TMR_Reader *reader, uint8_t *program);

#ifdef TMR_ENABLE_UHF
TMR_Status TMR_SR_cmdReadTagSingle(TMR_Reader *reader, uint16_t timeout, 
            uint16_t metadataFlags, const TMR_TagFilter *filter, 
            TMR_TagProtocol protocol, TMR_TagReadData *tagData);
#endif /* TMR_ENABLE_UHF */
TMR_Status TMR_SR_cmdReadTagMultiple(TMR_Reader *reader, uint16_t timeout,
            TMR_SR_SearchFlag flags, const TMR_TagFilter *filter,
            TMR_TagProtocol protocol, uint32_t *tagCount);

#ifdef TMR_ENABLE_UHF
TMR_Status TMR_SR_cmdWriteGen2TagEpc(TMR_Reader *reader, const TMR_TagFilter *filter, TMR_GEN2_Password accessPassword,
			uint16_t timeout, uint8_t count, const uint8_t *id, bool lock);
TMR_Status TMR_SR_cmdReadAfterWriteGen2TagEpc(TMR_Reader *reader, const TMR_TagFilter *filter, TMR_GEN2_Password accessPassword, 
            uint16_t timeout, uint8_t count, const uint8_t *id, bool lock, uint32_t readBank,
            uint32_t readAddress, uint8_t readLen, TMR_TagReadData *read);
TMR_Status TMR_SR_cmdGEN2WriteTagData(TMR_Reader *reader,
            uint16_t timeout, TMR_GEN2_Bank bank, uint32_t address,
            uint8_t count, const uint8_t data[],
            TMR_GEN2_Password accessPassword, const TMR_TagFilter *filter);
TMR_Status TMR_SR_cmdGEN2ReadAfterWriteTagData(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Bank writeBank, uint32_t writeAddress,
            uint16_t count, const uint8_t data[], TMR_GEN2_Password accessPassword, const TMR_TagFilter *filter,
            TMR_GEN2_Bank readBank, uint32_t readAddress, uint8_t readLen, TMR_TagReadData *read);
TMR_Status TMR_SR_cmdGEN2LockTag(TMR_Reader *reader, uint16_t timeout, 
            uint16_t mask, uint16_t action, TMR_GEN2_Password accessPassword, 
            const TMR_TagFilter *filter);
TMR_Status TMR_SR_cmdKillTag(TMR_Reader *reader, uint16_t timeout,
            TMR_GEN2_Password killPassword, const TMR_TagFilter *filter);
TMR_Status TMR_SR_cmdGEN2ReadTagData(TMR_Reader *reader,
            uint16_t timeout, TMR_GEN2_Bank bank,
            uint32_t address, uint8_t length, uint32_t accessPassword,
            const TMR_TagFilter *filter, TMR_TagReadData *data);
#endif /* TMR_ENABLE_UHF */

TMR_Status TMR_SR_cmdGetTagsRemaining(TMR_Reader *reader, uint16_t *remaining);
TMR_Status TMR_SR_cmdGetTagBuffer(TMR_Reader *reader, uint16_t count, bool epc496,
            TMR_TagProtocol protocol, TMR_TagData tagData[]);
TMR_Status TMR_SR_cmdClearTagBuffer(TMR_Reader *reader);

#ifdef TMR_ENABLE_UHF
TMR_Status TMR_SR_cmdHiggs2PartialLoadImage(TMR_Reader *reader, uint16_t timeout,
            TMR_GEN2_Password accessPassword, TMR_GEN2_Password killPassword, 
            uint8_t len, const uint8_t epc[], TMR_TagFilter* target);
TMR_Status TMR_SR_cmdHiggs2FullLoadImage(TMR_Reader *reader, uint16_t timeout,
            TMR_GEN2_Password accessPassword, TMR_GEN2_Password killPassword,
            uint16_t lockBits, uint16_t pcWord, uint8_t count,
            const uint8_t epc[], TMR_TagFilter* target);
TMR_Status TMR_SR_cmdHiggs3FastLoadImage(TMR_Reader *reader, uint16_t timeout,
            TMR_GEN2_Password currentAccessPassword, 
            TMR_GEN2_Password accessPassword, TMR_GEN2_Password killPassword,
            uint16_t pcWord, uint8_t count, const uint8_t epc[], TMR_TagFilter* target);
TMR_Status TMR_SR_cmdHiggs3LoadImage(TMR_Reader *reader, uint16_t timeout,
            TMR_GEN2_Password currentAccessPassword,
            TMR_GEN2_Password accessPassword, TMR_GEN2_Password killPassword,
            uint16_t pcWord, uint8_t len, const uint8_t epcAndUserData[], TMR_TagFilter* target);
TMR_Status TMR_SR_cmdHiggs3BlockReadLock(TMR_Reader *reader, uint16_t timeout,
            TMR_GEN2_Password accessPassword, uint8_t lockBits, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdNxpSetReadProtect(TMR_Reader *reader, uint16_t timeout,
            TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdNxpResetReadProtect(TMR_Reader *reader, uint16_t timeout,
            TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdNxpChangeEas(TMR_Reader *reader, uint16_t timeout,
            TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, bool reset, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdNxpEasAlarm(TMR_Reader *reader, uint16_t timeout,
            TMR_SR_GEN2_SiliconType chip, TMR_GEN2_DivideRatio dr, TMR_GEN2_TagEncoding m, TMR_GEN2_TrExt trExt,
            TMR_uint8List *data, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdNxpCalibrate(TMR_Reader *reader, uint16_t timeout,
            TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_uint8List *data, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdNxpChangeConfig(TMR_Reader *reader, uint16_t timeout,
            TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_NXP_ConfigWord configWord, TMR_uint8List *data, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdNxpUcode7ChangeConfig(TMR_Reader *reader, uint16_t timeout,
            TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_NXP_UCODE7_ConfigWord configWord, TMR_uint8List *data, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdGen2v2NXPUntraceable(TMR_Reader *reader, uint16_t timeout,TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, uint16_t configWord,
			TMR_TagOp_GEN2_NXP_Untraceable op,TMR_uint8List *data, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdGen2v2NXPAuthenticate(TMR_Reader *reader, uint16_t timeout,TMR_SR_GEN2_SiliconType chip,
	         TMR_GEN2_Password accessPassword, TMR_TagOp_GEN2_NXP_Authenticate op, TMR_uint8List *data, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdGen2v2NXPReadBuffer(TMR_Reader *reader, uint16_t timeout,TMR_SR_GEN2_SiliconType chip,
					TMR_GEN2_Password accessPassword, TMR_TagOp_GEN2_NXP_Readbuffer op, TMR_uint8List *data, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdMonza4QTReadWrite(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
            TMR_Monza4_ControlByte controlByte, TMR_Monza4_Payload payload, TMR_uint8List *data, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdMonza6MarginRead(TMR_Reader *reader, uint16_t timeout, TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword,
            TMR_GEN2_Bank bank, uint32_t bitAddress, uint16_t maskBitLength, uint8_t *mask, TMR_TagFilter *filter);
TMR_Status TMR_SR_cmdSL900aGetSensorValue(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
            uint8_t CommandCode, uint32_t password, PasswordLevel level, Sensor sensortype, TMR_uint8List *data, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdSL900aGetMeasurementSetup(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
            uint8_t CommandCode, uint32_t password, PasswordLevel level,TMR_uint8List *data, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdSL900aGetCalibrationData(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
            uint8_t CommandCode, uint32_t password, PasswordLevel level, TMR_uint8List *data, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdSL900aSetCalibrationData(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
            uint8_t CommandCode, uint32_t password, PasswordLevel level, uint64_t calibration, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdSL900aSetSfeParameters(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
            uint8_t CommandCode, uint32_t password, PasswordLevel level, uint16_t sfe, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdSL900aGetLogState(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
            uint8_t CommandCode, uint32_t password, PasswordLevel level, TMR_uint8List *data, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdSL900aSetLogMode(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
           uint8_t CommandCode, uint32_t password, PasswordLevel level, LoggingForm form, StorageRule rule, bool Ext1Enable,
           bool Ext2Enable, bool TempEnable, bool BattEnable, uint16_t LogInterval, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdSL900aSetLogLimit(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
           uint8_t CommandCode, uint32_t password, PasswordLevel level, uint16_t exLower,
           uint16_t lower, uint16_t upper, uint16_t exUpper, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdSL900aSetShelfLife(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                             uint8_t CommandCode, uint32_t password, PasswordLevel level, uint32_t block0, uint32_t block1,
                             TMR_TagFilter* target);
TMR_Status TMR_SR_cmdSL900aInitialize(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                      uint8_t CommandCode, uint32_t password, PasswordLevel level, uint16_t delayTime,
                                      uint16_t applicatioData, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdSL900aStartLog(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t CommandCode,
                                    uint32_t password, PasswordLevel level, uint32_t time, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdSL900aEndLog(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
           uint8_t CommandCode, uint32_t password, PasswordLevel level, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdSL900aSetPassword(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
           uint8_t CommandCode, uint32_t password, PasswordLevel level, uint32_t newPassword,
           PasswordLevel newPasswordLevel, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdSL900aGetBatteryLevel(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                           uint8_t CommandCode, uint32_t password, PasswordLevel level,BatteryType type,
                                           TMR_uint8List *data, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdSL900aAccessFifoStatus(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t CommandCode,
           uint32_t password, PasswordLevel level, AccessFifoOperation operation, TMR_uint8List * data, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdSL900aAccessFifoRead(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t CommandCode,
            uint32_t password, PasswordLevel level, AccessFifoOperation operation, uint8_t length,TMR_uint8List * data, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdSL900aAccessFifoWrite(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t CommandCode,
           uint32_t password, PasswordLevel level, AccessFifoOperation operation, TMR_uint8List *payLoad, TMR_uint8List * data, TMR_TagFilter* target);
TMR_Status 
TMR_SR_cmdFDNReadREG(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint16_t CommandCode, uint16_t RegAddress, TMR_TagFilter* target, TMR_uint8List *data);
TMR_Status
TMR_SR_cmdFDNWriteREG(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint16_t CommandCode, uint16_t RegAddress,uint16_t RegWriteData, TMR_TagFilter* target, TMR_uint8List *data);
TMR_Status
TMR_SR_cmdFDNLoadREG(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint16_t CommandCode, uint8_t CmdCfg, TMR_TagFilter* target, TMR_uint8List *data);
TMR_Status
TMR_SR_cmdFDNStartStopLOG(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint16_t CommandCode, uint8_t CmdCfg, uint32_t FlagResetPassword, TMR_TagFilter* target, TMR_uint8List *data);
TMR_Status
TMR_SR_cmdFDNAuth(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint16_t CommandCode, uint8_t CmdCfg, uint32_t AuthPassword, TMR_TagFilter* target, TMR_uint8List *data);
TMR_Status 
TMR_SR_cmdFDNReadMEM(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint16_t CommandCode, uint16_t StartAddress, uint16_t len, TMR_TagFilter* target, TMR_uint8List *data);
TMR_Status 
TMR_SR_cmdFDNWriteMEM(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint16_t CommandCode, uint16_t StartAddress, 
                      uint16_t len, const uint8_t* Writedatadata, TMR_TagFilter* target, TMR_uint8List *data);
TMR_Status
TMR_SR_cmdFDNStateCheck(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint16_t CommandCode,
                        uint16_t len, const uint8_t* Data, TMR_TagFilter* target, TMR_uint8List *data);
TMR_Status 
TMR_SR_cmdFDNMeasure(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint16_t CommandCode,
                      uint8_t CmdCfg, uint8_t StoreBlockAddress, TMR_TagFilter* target, TMR_uint8List *data);
TMR_Status
TMR_SR_cmdILIANTagSelect(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                         uint16_t CommandCode, TMR_TagFilter* target);
TMR_Status
TMR_SR_cmdEM4325GetSensorData(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                         uint16_t CommandCode, TMR_TagFilter* target, uint8_t bitToSet, TMR_uint8List *data);
TMR_Status
TMR_SR_cmdEM4325ResetAlarms(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword,
                         uint16_t CommandCode, TMR_TagFilter* target, uint8_t fillValue);
TMR_Status TMR_SR_cmdHibikiReadLock(TMR_Reader *reader, uint16_t timeout,
            TMR_GEN2_Password accessPassword, uint16_t mask, uint16_t action);
TMR_Status TMR_SR_cmdHibikiGetSystemInformation(TMR_Reader *reader, uint16_t timeout,
            TMR_GEN2_Password accessPassword,
            TMR_GEN2_HibikiSystemInformation *info);
TMR_Status TMR_SR_cmdHibikiSetAttenuate(TMR_Reader *reader, uint16_t timeout,
            TMR_GEN2_Password accessPassword, uint8_t level, bool lock);
TMR_Status TMR_SR_cmdHibikiBlockLock(TMR_Reader *reader, uint16_t timeout,
            TMR_GEN2_Password accessPassword, uint8_t block,
            TMR_GEN2_Password blockPassword, uint8_t mask, uint8_t action);
TMR_Status TMR_SR_cmdHibikiBlockReadLock(TMR_Reader *reader, uint16_t timeout,
            TMR_GEN2_Password accessPassword, uint8_t block,
            TMR_GEN2_Password blockPassword, uint8_t mask, uint8_t action);
TMR_Status TMR_SR_cmdHibikiWriteMultipleWords(TMR_Reader *reader,
            uint16_t timeout, TMR_GEN2_Password accessPassword,
            TMR_GEN2_Bank bank, uint32_t wordOffset, uint8_t count,
            const uint8_t data[]);
TMR_Status TMR_SR_cmdEraseBlockTagSpecific(TMR_Reader *reader, uint16_t timeout,
            TMR_GEN2_Bank bank, uint32_t address, uint8_t count);
TMR_Status TMR_SR_cmdGetTxRxPorts(TMR_Reader *reader, TMR_SR_PortPair *ant);
TMR_Status TMR_SR_cmdGetAntennaConfiguration(TMR_Reader *reader,
            TMR_SR_AntennaPort *config);
TMR_Status TMR_SR_cmdGetAntennaSearchList(TMR_Reader *reader, uint8_t *count,
            TMR_SR_PortPair *ants);
TMR_Status TMR_SR_cmdGetAntennaPortPowers(TMR_Reader *reader, uint8_t *count,
            TMR_SR_PortPower *ports);
TMR_Status TMR_SR_cmdGetAntennaPortPowersAndSettlingTime(TMR_Reader *reader,
            uint8_t *count, TMR_SR_PortPowerAndSettlingTime *ports);
TMR_Status TMR_SR_cmdGetAntennaReturnLoss(TMR_Reader *reader, TMR_PortValueList *ports);
#endif /* TMR_ENABLE_UHF */
TMR_Status TMR_SR_cmdAntennaDetect(TMR_Reader *reader, uint8_t *count,
            TMR_SR_PortDetect *ports);
TMR_Status TMR_SR_cmdGetReadTxPower(TMR_Reader *reader, int32_t *power);
TMR_Status TMR_SR_cmdGetReadTxPowerWithLimits(TMR_Reader *reader,
            TMR_SR_PowerWithLimits *power);
TMR_Status TMR_SR_cmdGetWriteTxPower(TMR_Reader *reader, int32_t *power);
#ifdef TMR_ENABLE_UHF
TMR_Status TMR_SR_cmdGetWriteTxPowerWithLimits(TMR_Reader *reader,
            TMR_SR_PowerWithLimits *power);
#endif /* TMR_ENABLE_UHF */

TMR_Status TMR_SR_cmdGetCurrentProtocol(TMR_Reader *reader,
            TMR_TagProtocol *protocol);
TMR_Status TMR_SR_cmdMultipleProtocolSearch(TMR_Reader *reader,TMR_SR_OpCode op,TMR_TagProtocolList *protocols, TMR_TRD_MetadataFlag metadataFlags,TMR_SR_SearchFlag antennas, TMR_TagFilter **filter, uint16_t timeout, uint32_t *tagsFound);

#ifdef TMR_ENABLE_UHF
TMR_Status TMR_SR_cmdGetFrequencyHopTable(TMR_Reader *reader, uint8_t *count,
            uint32_t *hopTable);
TMR_Status TMR_SR_cmdGetFrequencyHopTime(TMR_Reader *reader, uint32_t *hopTime);
TMR_Status TMR_SR_cmdGetQuantizationStep(TMR_Reader *reader, uint32_t *step);
TMR_Status TMR_SR_cmdGetMinimumFrequency(TMR_Reader *reader, uint32_t *freq);
#endif /* TMR_ENABLE_UHF */

TMR_Status TMR_SR_cmdGetGPIO(TMR_Reader *reader, uint8_t *count, TMR_GpioPin *state);
TMR_Status TMR_SR_cmdGetGPIODirection(TMR_Reader *reader, uint8_t pin,
            bool *out);
TMR_Status TMR_SR_cmdGetRegion(TMR_Reader *reader, TMR_Region *region);
#ifdef TMR_ENABLE_UHF
TMR_Status TMR_SR_cmdGetRegionConfiguration(TMR_Reader *reader,
            TMR_SR_RegionConfiguration key, void *value);
#endif /* TMR_ENABLE_UHF */
TMR_Status TMR_SR_cmdGetPowerMode(TMR_Reader *reader, TMR_SR_PowerMode *mode);
#ifdef TMR_ENABLE_UHF 
TMR_Status TMR_SR_cmdGetUserMode(TMR_Reader *reader, TMR_SR_UserMode *mode);
#endif /* TMR_ENABLE_UHF */
TMR_Status TMR_SR_cmdGetReaderConfiguration(TMR_Reader *reader,
            TMR_SR_Configuration key, void *value);
#ifdef TMR_ENABLE_UHF
TMR_Status TMR_SR_cmdIAVDenatranCustomOp(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t mode, uint8_t rfu,
           TMR_uint8List *data, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdIAVDenatranCustomActivateSiniavMode(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t mode, uint8_t payload,
           TMR_uint8List *data, TMR_TagFilter* target, bool tokenDesc, uint8_t *token);
TMR_Status TMR_SR_cmdIAVDenatranCustomReadFromMemMap(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t mode, uint8_t payload,
            TMR_uint8List *data, TMR_TagFilter* target, uint16_t wordAddress);
TMR_Status TMR_SR_cmdIAVDenatranCustomWriteToMemMap(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t mode, uint8_t payload,
            TMR_uint8List *data, TMR_TagFilter* target, uint16_t wordPtr, uint16_t wordData, uint8_t* tagId, uint8_t* dataBuf);
TMR_Status TMR_SR_cmdIAVDenatranCustomWriteSec(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t mode, uint8_t payload,
            TMR_uint8List *data, TMR_TagFilter* target, uint8_t* dataWords, uint8_t* dataBuf);
TMR_Status TMR_SR_cmdIAVDenatranCustomGetTokenId(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t mode,
           TMR_uint8List *data, TMR_TagFilter* target);
TMR_Status TMR_SR_cmdIAVDenatranCustomReadSec(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t mode, uint8_t payload,
            TMR_uint8List *data, TMR_TagFilter* target, uint16_t wordAddress);
#endif /* TMR_ENABLE_UHF */

TMR_Status TMR_SR_cmdGetProtocolConfiguration(TMR_Reader *reader, TMR_TagProtocol protocol,
            TMR_SR_ProtocolConfiguration key, void *value);
TMR_Status TMR_SR_cmdGetReaderStats(TMR_Reader *reader,
           TMR_Reader_StatsFlag statFlags, TMR_Reader_StatsValues *stats);
#ifdef TMR_ENABLE_UHF
TMR_Status TMR_SR_cmdGetReaderStatistics(TMR_Reader *reader,
            TMR_SR_ReaderStatisticsFlag statFlags,
            TMR_SR_ReaderStatistics *stats);
#endif /* TMR_ENABLE_UHF */

TMR_Status TMR_SR_cmdGetAvailableProtocols(TMR_Reader *reader,
            TMR_TagProtocolList *protocols);
TMR_Status TMR_SR_cmdGetAvailableRegions(TMR_Reader *reader,
            TMR_RegionList *regions);
TMR_Status TMR_SR_cmdGetTemperature(TMR_Reader *reader, int8_t *temp);
TMR_Status TMR_SR_cmdSetTxRxPorts(TMR_Reader *reader, uint8_t txPrt,
            uint8_t rxPort);
TMR_Status TMR_SR_cmdSetAntennaSearchList(TMR_Reader *reader,
            uint8_t count, const TMR_SR_PortPair *ports);

#ifdef TMR_ENABLE_UHF
TMR_Status TMR_SR_cmdSetAntennaPortPowers(TMR_Reader *reader,
            uint8_t count, const TMR_SR_PortPower *ports);
TMR_Status TMR_SR_cmdSetAntennaPortPowersAndSettlingTime(TMR_Reader *reader,
            uint8_t count, const TMR_SR_PortPowerAndSettlingTime *ports);
#endif /* TMR_ENABLE_UHF */
TMR_Status TMR_SR_cmdSetReadTxPower(TMR_Reader *reader, int32_t power);
TMR_Status TMR_SR_cmdSetWriteTxPower(TMR_Reader *reader, int32_t power);

TMR_Status TMR_SR_cmdSetProtocol(TMR_Reader *reader, TMR_TagProtocol protocol);

#ifdef TMR_ENABLE_UHF
TMR_Status TMR_SR_cmdSetFrequencyHopTable(TMR_Reader *reader, uint8_t count,
            const uint32_t *table);
TMR_Status
TMR_SR_cmdSetFrequencyHopTableOption(TMR_Reader *reader, uint32_t value, uint8_t subOption);
#endif /* TMR_ENABLE_UHF */
TMR_Status TMR_SR_cmdSetGPIO(TMR_Reader *reader, uint8_t gpio, bool high);
TMR_Status TMR_SR_cmdSetGPIODirection(TMR_Reader *reader, uint8_t pin,
            bool out);
TMR_Status TMR_SR_cmdSetRegion(TMR_Reader *reader, TMR_Region region);

#ifdef TMR_ENABLE_UHF
TMR_Status TMR_SR_regionConfiguration(bool lbtEenable, int8_t lbtThreshold, bool dwellTimeEnable, uint32_t *dwellTime);
TMR_Status TMR_SR_cmdSetRegionLbt(TMR_Reader *reader, TMR_Region region, bool lbt);
TMR_Status TMR_SR_cmdSetRegionDwellTime(TMR_Reader *reader, TMR_Region region, uint16_t dwellTime);
TMR_Status TMR_SR_cmdSetRegionDwellTimeEnable(TMR_Reader *reader, TMR_Region region, bool dwellTimeEnable);
TMR_Status TMR_SR_cmdSetRegionLbtThreshold(TMR_Reader *reader, TMR_Region region, int8_t lbtThreshold);
#endif /* TMR_ENABLE_UHF */
TMR_Status TMR_SR_cmdSetPowerMode(TMR_Reader *reader, TMR_SR_PowerMode mode);
#ifdef TMR_ENABLE_UHF
TMR_Status TMR_SR_cmdSetUserMode(TMR_Reader *reader, TMR_SR_UserMode mode);
#endif /* TMR_ENABLE_UHF */

TMR_Status TMR_SR_cmdSetReaderConfiguration(TMR_Reader *reader, 
            TMR_SR_Configuration key, const void *value);
TMR_Status TMR_SR_cmdSetProtocolLicenseKey(TMR_Reader *reader, 
            TMR_SR_SetProtocolLicenseOption option, uint8_t key[], int key_len);
TMR_Status TMR_SR_cmdSetProtocolConfiguration(TMR_Reader *reader,
            TMR_TagProtocol protocol, TMR_SR_ProtocolConfiguration key,
            const void *value);
TMR_Status TMR_SR_cmdResetReaderStats(TMR_Reader *reader,
           TMR_Reader_StatsFlag statFlags);
#ifdef TMR_ENABLE_UHF
TMR_Status TMR_SR_cmdResetReaderStatistics(TMR_Reader *reader,
            TMR_SR_ReaderStatisticsFlag statFlags);
TMR_Status TMR_SR_cmdTestSetFrequency(TMR_Reader *reader, uint32_t frequency);
TMR_Status TMR_SR_cmdTestSendCw(TMR_Reader *reader, bool on);
TMR_Status TMR_SR_cmdTestSendPrbs(TMR_Reader *reader, uint16_t duration);
#endif /* TMR_ENABLE_UHF */
TMR_Status TMR_SR_cmdSetUserProfile(TMR_Reader *reader,
                                    TMR_SR_UserConfigOperation op,TMR_SR_UserConfigCategory category, TMR_SR_UserConfigType type);
#ifdef TMR_ENABLE_UHF
TMR_Status TMR_SR_cmdGetUserProfile(TMR_Reader *reader, 
                                    uint8_t byte[], uint8_t length, uint8_t response[], uint8_t* response_length);
TMR_Status TMR_SR_cmdBlockWrite(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Bank bank, uint32_t wordPtr, 
                                 uint32_t wordCount, const uint16_t* data, uint32_t accessPassword, const TMR_TagFilter* target);
TMR_Status TMR_SR_cmdBlockErase(TMR_Reader *reader, uint16_t timeout, TMR_GEN2_Bank bank, uint32_t wordPtr,
                                 uint8_t wordCount, uint32_t accessPassword, TMR_TagFilter *target);
TMR_Status TMR_SR_cmdBlockPermaLock(TMR_Reader *reader, uint16_t timeout,uint32_t readLock, TMR_GEN2_Bank bank, 
                         uint32_t blockPtr, uint32_t blockRange, uint16_t* mask, uint32_t accessPassword, TMR_TagFilter* target, TMR_uint8List *data);
#endif /* TMR_ENABLE_UHF */

TMR_Status TMR_SR_msgSetupReadTagMultiple(TMR_Reader *reader, uint8_t *msg, uint8_t *i,
            uint16_t timeout, TMR_SR_SearchFlag searchFlag,
            const TMR_TagFilter *filter, TMR_TagProtocol protocol,
            TMR_GEN2_Password accessPassword);

TMR_Status
TMR_SR_msgSetupReadTagMultipleWithMetadata(TMR_Reader *reader, uint8_t *msg, uint8_t *i, uint16_t timeout,
                               TMR_SR_SearchFlag searchFlag,
							                 TMR_TRD_MetadataFlag metadataFlag,
                               const TMR_TagFilter *filter,
                               TMR_TagProtocol protocol,
                               TMR_GEN2_Password accessPassword);

#ifdef TMR_ENABLE_UHF
TMR_Status TMR_SR_msgSetupReadTagSingle(uint8_t *msg, uint8_t *i, TMR_TagProtocol protocol,TMR_TRD_MetadataFlag metadataFlags, const TMR_TagFilter *filter,uint16_t timeout);
void TMR_SR_msgAddGEN2WriteTagEPC(uint8_t *msg, uint8_t *i, uint16_t timeout, uint8_t *epc, uint8_t count);
void TMR_SR_msgAddGEN2DataRead(uint8_t *msg, uint8_t *i, uint16_t timeout,
      TMR_GEN2_Bank bank, uint32_t wordAddress, uint8_t len, uint8_t option, bool withMetaData);
void TMR_SR_msgAddGEN2DataWrite(uint8_t *msg, uint8_t *i, uint16_t timeout,
      TMR_GEN2_Bank bank, uint32_t address, bool isReadAfterWrite);
void TMR_SR_msgAddGEN2LockTag(uint8_t *msg, uint8_t *i, uint16_t timeout,
      uint16_t mask, uint16_t action, TMR_GEN2_Password password);
void TMR_SR_msgAddGEN2KillTag(uint8_t *msg, uint8_t *i, uint16_t timeout,
      TMR_GEN2_Password password);
TMR_Status
TMR_SR_msgAddGEN2BlockWrite(uint8_t *msg, uint8_t *i, uint16_t timeout,TMR_GEN2_Bank bank, uint32_t wordPtr, uint32_t wordCount, uint16_t* data, uint32_t accessPassword,TMR_TagFilter* target);

TMR_Status
TMR_SR_msgAddGEN2BlockPermaLock(uint8_t *msg, uint8_t *i, uint16_t timeout, uint32_t readLock, TMR_GEN2_Bank bank, uint32_t blockPtr, uint32_t blockRange, uint16_t* mask, uint32_t accessPassword,TMR_TagFilter* target);

TMR_Status
TMR_SR_msgAddGEN2BlockErase(uint8_t *msg, uint8_t *i, uint16_t timeout, uint32_t wordPtr, TMR_GEN2_Bank bank,
                            uint8_t wordCount, uint32_t accessPassword, TMR_TagFilter* target);

void 
TMR_SR_msgAddHiggs2PartialLoadImage(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, 
                                    TMR_GEN2_Password killPassword, uint8_t len, const uint8_t *epc, TMR_TagFilter* target);
void 
TMR_SR_msgAddHiggs2FullLoadImage(uint8_t *msg, uint8_t *i, uint16_t timeout,
      TMR_GEN2_Password accessPassword, TMR_GEN2_Password killPassword, uint16_t lockBits, uint16_t pcWord, uint8_t len, const uint8_t *epc, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddHiggs3FastLoadImage(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password currentAccessPassword,
      TMR_GEN2_Password accessPassword, TMR_GEN2_Password killPassword, uint16_t pcWord, uint8_t len, const uint8_t *epc, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddHiggs3LoadImage(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password currentAccessPassword,
      TMR_GEN2_Password accessPassword, TMR_GEN2_Password killPassword, uint16_t pcWord, uint8_t len, const uint8_t *epcAndUserData, TMR_TagFilter* target);

TMR_Status 
TMR_SR_msgAddHiggs3BlockReadLock(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, uint8_t lockBits, TMR_TagFilter* target);

TMR_Status 
TMR_SR_msgAddNXPSetReadProtect(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_SR_GEN2_SiliconType chip,
                            TMR_GEN2_Password accessPassword, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddNXPResetReadProtect(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_SR_GEN2_SiliconType chip,
                            TMR_GEN2_Password accessPassword, TMR_TagFilter* target);
TMR_Status
TMR_SR_msgAddNXPChangeEAS(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_SR_GEN2_SiliconType chip,
                          TMR_GEN2_Password accessPassword, bool reset, TMR_TagFilter* target);
void 
TMR_SR_msgAddNXPEASAlarm(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_SR_GEN2_SiliconType chip,
                         TMR_GEN2_DivideRatio dr, TMR_GEN2_TagEncoding m, TMR_GEN2_TrExt trExt, TMR_TagFilter* target);
TMR_Status
TMR_SR_msgAddIAVDenatranCustomOp(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                         uint8_t mode, uint8_t rfu, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddIAVDenatranCustomActivateSiniavMode(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                         uint8_t mode, uint8_t payload, TMR_TagFilter* target, bool tokenDesc, uint8_t *token);
TMR_Status 
TMR_SR_msgAddIAVDenatranCustomReadFromMemMap(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                        uint8_t mode, uint8_t payload, TMR_TagFilter* target, uint16_t wordAddress);
TMR_Status 
TMR_SR_msgAddIAVDenatranCustomWriteToMemMap(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                        uint8_t mode, uint8_t payload, TMR_TagFilter* target, uint16_t wordPtr, uint16_t wordData, uint8_t* tagId, uint8_t* dataBuf);
TMR_Status
TMR_SR_msgAddIAVDenatranCustomWriteSec(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                        uint8_t mode, uint8_t payload, TMR_TagFilter* target, uint8_t* data, uint8_t* dataBuf);
TMR_Status 
TMR_SR_msgAddIAVDenatranCustomGetTokenId(uint8_t *msg, uint8_t *i, uint16_t timeout,
                        TMR_GEN2_Password accessPassword, uint8_t mode, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddIAVDenatranCustomReadSec(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                        uint8_t mode, uint8_t payload, TMR_TagFilter* target, uint16_t wordAddress);
TMR_Status 
TMR_SR_msgAddNXPCalibrate(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_SR_GEN2_SiliconType chip,
                         TMR_GEN2_Password accessPassword, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddNXPChangeConfig(uint8_t *msg, uint8_t *i, uint16_t timeout,
                         TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_NXP_ConfigWord configword, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddGen2v2NxpUntraceable(uint8_t *msg, uint8_t *i, uint16_t timeout, 
                              TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, uint16_t configWord, TMR_TagOp_GEN2_NXP_Untraceable op , TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddGen2v2NxpAuthenticate(uint8_t *msg, uint8_t *i, uint16_t timeout,
                               TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_TagOp_GEN2_NXP_Authenticate op , TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddGen2v2NxpReadBuffer(uint8_t *msg, uint8_t *i, uint16_t timeout,
                             TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_TagOp_GEN2_NXP_Readbuffer op , TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddNXPUCODE7ChangeConfig(uint8_t *msg, uint8_t *i, uint16_t timeout,
                         TMR_SR_GEN2_SiliconType chip, TMR_GEN2_Password accessPassword, TMR_NXP_UCODE7_ConfigWord configword, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddMonza4QTReadWrite(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                               TMR_Monza4_ControlByte controlByte, TMR_Monza4_Payload payload, TMR_TagFilter* target);

TMR_Status 
TMR_SR_msgAddMonza6MarginRead(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                               TMR_GEN2_Bank bank, uint32_t bitAddress, uint16_t maskBitLength, uint8_t * mask, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddIdsSL900aGetSensorValue(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                     uint8_t CommandCode, uint32_t password, PasswordLevel level, Sensor sensortype,
                                     TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddIdsSL900aGetMeasurementSetup(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                          uint8_t CommandCode, uint32_t password, PasswordLevel level, TMR_TagFilter* target);

TMR_Status 
TMR_SR_msgAddIdsSL900aGetCalibrationData(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                     uint8_t CommandCode, uint32_t password, PasswordLevel level, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddIdsSL900aSetCalibrationData(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                          uint8_t CommandCode, uint32_t password, PasswordLevel level, uint64_t calibration, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddIdsSL900aSetSfeParameters(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                          uint8_t CommandCode, uint32_t password, PasswordLevel level, uint16_t sfe,
                                          TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddIdsSL900aGetLogState(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                  uint8_t CommandCode, uint32_t password, PasswordLevel level, TMR_TagFilter* target);
TMR_Status
TMR_SR_msgAddIdsSL900aSetLogMode(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                 uint8_t CommandCode, uint32_t password, PasswordLevel level, LoggingForm form,
                                 StorageRule rule, bool Ext1Enable, bool Ext2Enable, bool TempEnable, bool BattEnable,
                                 uint16_t LogInterval, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddIdsSL900aSetLogLimit(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                  uint8_t CommandCode, uint32_t password, PasswordLevel level, uint16_t exLower,
                                  uint16_t lower, uint16_t upper, uint16_t exUpper, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddIdsSL900aSetShelfLife(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                   uint8_t CommandCode, uint32_t password, PasswordLevel level, uint32_t block0, uint32_t block1,
                                   TMR_TagFilter* target);
TMR_Status
TMR_SR_msgAddIdsSL900aInitialize(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                 uint8_t CommandCode, uint32_t password, PasswordLevel level, uint16_t delayTime,
                                 uint16_t applicatioData, TMR_TagFilter* target);
TMR_Status
TMR_SR_msgAddIdsSL900aEndLog(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                             uint8_t CommandCode, uint32_t password, PasswordLevel level, TMR_TagFilter* target);
TMR_Status
TMR_SR_msgAddIdsSL900aSetPassword(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                  uint8_t CommandCode, uint32_t password, PasswordLevel level, uint32_t newPassword,
                                  PasswordLevel newPasswordLevel, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddIdsSL900aAccessFifoStatus(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                  uint8_t CommandCode, uint32_t password, PasswordLevel level, AccessFifoOperation operation,
                                  TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddIdsSL900aGetBatteryLevel(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                      uint8_t CommandCode, uint32_t password, PasswordLevel level, BatteryType batteryType,
                                      TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddIdsSL900aAccessFifoRead(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                  uint8_t CommandCode, uint32_t password, PasswordLevel level, AccessFifoOperation operation,
                                  uint8_t length, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddIdsSL900aAccessFifoWrite(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                                  uint8_t CommandCode, uint32_t password, PasswordLevel level, AccessFifoOperation operation,
                                  TMR_uint8List *payLoad, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddIdsSL900aStartLog(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                               uint8_t CommandCode, uint32_t password, PasswordLevel level, uint32_t time, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddFDNReadREG(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, uint16_t CommandCode, uint16_t RegAddress, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddFDNWriteREG(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, uint16_t CommandCode, 
                         uint16_t RegAddress, uint16_t  RegWriteData, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddFDNLoadREG(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, uint16_t CommandCode, uint8_t CmdCfg, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddFDNStartStopLOG(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, uint16_t CommandCode, 
                             uint8_t CmdCfg, uint32_t FlagResetPassword, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddFDNAuth(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, uint16_t CommandCode, 
                             uint8_t CmdCfg, uint32_t AuthPassword, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddFDNReadMEM(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, 
                        uint16_t CommandCode, uint16_t StartAddress, uint16_t len, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddFDNStateCheck(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, uint16_t CommandCode,
                           uint16_t len, const uint8_t* data, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddFDNWriteMEM(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, uint16_t CommandCode,
                         uint16_t StartAddress, uint16_t len, const uint8_t* data, TMR_TagFilter* target);
TMR_Status 
TMR_SR_msgAddFDNMeasure(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, uint16_t CommandCode,
                         uint8_t CmdCfg, uint8_t StoreBlockAddress, TMR_TagFilter* target);
TMR_Status
TMR_SR_msgAddILIANTagSelect(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword, 
                         uint16_t CommandCode, TMR_TagFilter* target);
TMR_Status
TMR_SR_msgAddEM4325GetSensorData(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                         uint16_t CommandCode, TMR_TagFilter* target, uint8_t bitToSet);
TMR_Status
TMR_SR_msgAddEM4325ResetAlarms(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_GEN2_Password accessPassword,
                         uint16_t CommandCode, TMR_TagFilter* target, uint8_t fillValue);
TMR_Status TMR_SR_cmdISO180006BWriteTagData(TMR_Reader *reader,
      uint16_t timeout, uint8_t address, uint8_t count, const uint8_t data[],
      const TMR_TagFilter *filter);
TMR_Status TMR_SR_cmdISO180006BReadTagData(TMR_Reader *reader,
      uint16_t timeout, uint8_t address, uint8_t length,
      const TMR_TagFilter *filter, TMR_TagReadData *read);
TMR_Status TMR_SR_cmdISO180006BLockTag(TMR_Reader *reader, uint16_t timeout,
      uint8_t address, const TMR_TagFilter *filter);
TMR_Status TMR_iso18000BBLFValToInt(int val, void *lf);
#endif /* TMR_ENABLE_UHF */
TMR_Status TMR_SR_executeEmbeddedRead(TMR_Reader *reader, uint8_t *msg,
            uint16_t timeout, TMR_SR_MultipleStatus *status);
TMR_Status TMR_SR_cmdStopReading(struct TMR_Reader *reader);
#ifdef TMR_ENABLE_UHF
TMR_Status TMR_SR_cmdGetReaderWriteTimeOut (struct TMR_Reader *reader, TMR_TagProtocol protocol,
																						TMR_SR_Gen2ReaderWriteTimeOut *value);
TMR_Status TMR_SR_cmdSetReaderWriteTimeOut (struct TMR_Reader *reader, TMR_TagProtocol protocol,
																						TMR_SR_Gen2ReaderWriteTimeOut *value);
TMR_Status TMR_SR_cmdAuthReqResponse(struct TMR_Reader *reader, TMR_TagAuthentication *auth);
TMR_Status TMR_SR_cmdSetRegulatoryTest(struct TMR_Reader *reader, bool enable);
#endif /* TMR_ENABLE_UHF */
TMR_Status TMR_SR_addTagOp(struct TMR_Reader *reader, TMR_TagOp *tagop,TMR_ReadPlan *rp, 
                             uint8_t *msg, uint8_t *i, uint32_t readTimeMs, uint8_t *lenbyte);
void
TMR_parseTagStats(TMR_Reader* reader, TMR_Reader_StatsValues* stats,
                    uint8_t* msg, uint8_t offset);
TMR_Status
TMR_fillReaderStats(TMR_Reader *reader, TMR_Reader_StatsValues* stats, uint16_t flag,
                      uint8_t* msg, uint8_t offset);
bool validateParams(TMR_ReadPlan *plan);
bool compareAntennas(TMR_MultiReadPlan *multi);

TMR_Status
TMR_SR_cmdrebootReader(TMR_Reader *reader);
TMR_Status
prepForSearch(TMR_Reader *reader, TMR_ReadPlan *rp);
TMR_Status 
TMR_SR_msgSetupMultipleProtocolSearch(TMR_Reader *reader, uint8_t *msg, TMR_SR_OpCode op, TMR_TagProtocolList *protocols, TMR_TRD_MetadataFlag metadataFlags, TMR_SR_SearchFlag antennas, TMR_TagFilter **filter, uint16_t timeout);
TMR_Status TMR_SR_cmdProbeBaudRate(TMR_Reader *reader, uint32_t *currentBaudRate);
void TMR_SR_updateBaseTimeStamp(TMR_Reader *reader);
TMR_Status verifySearchStatus(TMR_Reader *reader);
#ifdef TMR_ENABLE_UHF
#ifdef TMR_ENABLE_BACKGROUND_READS
void notify_authreq_listeners(TMR_Reader *reader, TMR_TagReadData *trd, TMR_TagAuthentication *auth);
#endif
#endif /* TMR_ENABLE_UHF */

#ifdef TMR_ENABLE_HF_LF
TMR_Status
TMR_SR_cmdSetProtocolList(TMR_Reader *reader, TMR_TagProtocolList *protocols);
TMR_Status
TMR_SR_cmdGetProtocolList(TMR_Reader *reader, TMR_TagProtocolList *protocols);
TMR_Status
TMR_SR_cmdWriteMemory(TMR_Reader *reader, TMR_ExtTagOp *tagOp, 
                        const TMR_TagFilter *filter, TMR_uint8List tagOpExtParams);
TMR_Status
TMR_SR_cmdReadMemory(TMR_Reader *reader, TMR_ExtTagOp *tagOp, const TMR_TagFilter *filter,
                       TMR_uint8List *data, TMR_uint8List tagOpExtParams);
void
TMR_SR_msgAddReadMemory(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_Memory_Type memType,
                          uint32_t address, uint8_t len, bool withMetaData);
void
TMR_SR_msgAddWriteMemory(uint8_t *msg, uint8_t *i, uint16_t timeout, TMR_Memory_Type memType,
                           uint32_t address);
TMR_Status
TMR_SR_cmdPassThrough(TMR_Reader *reader, uint32_t timeout, uint32_t configFlags,
                                   uint16_t cmdLen, const uint8_t *cmd , TMR_uint8List *data);
#if TMR_ENABLE_EXTENDED_TAGOPS
TMR_Status
TMR_SR_msgAddExtendedParams(uint8_t *msg, uint8_t *i, uint8_t *optbyte, const TMR_uint8List *extParams);
#endif /* TMR_ENABLE_EXTENDED_TAGOPS */
#endif /* TMR_ENABLE_HF_LF */
#ifdef __cplusplus
}
#endif

#endif /* _SERIAL_READER_IMP_H */
