/* ex: set tabstop=2 shiftwidth=2 expandtab cindent: */
#ifndef _TM_CONFIG_H
#define _TM_CONFIG_H

/**
 *  @file tm_config.h
 *  @brief Mercury API - Build Configuration
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

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * API version number
 */
#define TMR_VERSION "1.35.2.72"

/**
 * Define this to enable support for devices that support the serial
 * reader command set (M5e, M6e, Vega, etc.)
 */
#define TMR_ENABLE_SERIAL_READER

/**
* Assign value as 1 to Enable required Param
* Assign value as 0 to Disable required Param
*/
 #define  ENABLE_TMR_PARAM_REGION_ID                                    1
 #define  ENABLE_TMR_PARAM_POWERMODE                                    1
 #define  ENABLE_TMR_PARAM_VERSION_MODEL                                1
 #define  ENABLE_TMR_PARAM_REGION_SUPPORTEDREGIONS                      1
 #define  ENABLE_TMR_PARAM_BAUDRATE                                     1
 #define  ENABLE_TMR_PARAM_RADIO_READPOWER                              1
 #define  ENABLE_TMR_PARAM_METADATAFLAG                                 1
 #define  ENABLE_TMR_PARAM_READ_PLAN                                    1
 #define  ENABLE_TMR_PARAM_VERSION_SOFTWARE                             1
 #define  ENABLE_TMR_PARAM_ANTENNA_CHECKPORT                            1
 #define  ENABLE_TMR_PARAM_PROBEBAUDRATES                               1
 #define  ENABLE_TMR_PARAM_COMMANDTIMEOUT                               1
 #define  ENABLE_TMR_PARAM_TRANSPORTTIMEOUT                             1
 #define  ENABLE_TMR_PARAM_USERMODE                                     1
 #define  ENABLE_TMR_PARAM_ANTENNA_PORTLIST                             1
 #define  ENABLE_TMR_PARAM_ANTENNA_CONNECTEDPORTLIST                    1
 #define  ENABLE_TMR_PARAM_ANTENNA_PORTSWITCHGPOS                       1
 #define  ENABLE_TMR_PARAM_ANTENNA_SETTLINGTIMELIST                     1
 #define  ENABLE_TMR_PARAM_ANTENNA_RETURNLOSS                           1
 #define  ENABLE_TMR_PARAM_ANTENNA_TXRXMAP                              1
 #define  ENABLE_TMR_PARAM_PER_ANTENNA_TIME                             1
 #define  ENABLE_TMR_PARAM_GPIO_INPUTLIST_OUTPUTLIST                    1
 #define  ENABLE_TMR_PARAM_GEN2_ACCESSPASSWORD                          1
 #define  ENABLE_TMR_PARAM_GEN2_Q                                       1
 #define  ENABLE_TMR_PARAM_GEN2_TAGENCODING                             1
 #define  ENABLE_TMR_PARAM_GEN2_SESSION                                 1
 #define  ENABLE_TMR_PARAM_GEN2_TARGET                                  1
 #define  ENABLE_TMR_PARAM_GEN2_BLF                                     1
 #define  ENABLE_TMR_PARAM_GEN2_TARI                                    1
 #define  ENABLE_TMR_PARAM_GEN2_WRITEMODE                               1
 #define  ENABLE_TMR_PARAM_GEN2_BAP                                     1
 #define  ENABLE_TMR_PARAM_GEN2_INITIAL_Q                               1
 #define  ENABLE_TMR_PARAM_GEN2_SEND_SELECT                             1
 #define  ENABLE_TMR_PARAM_GEN2_PROTOCOLEXTENSION                       1
 #define  ENABLE_TMR_PARAM_ISO180006B_BLF                               1
 #define  ENABLE_TMR_PARAM_ISO180006B_MODULATION_DEPTH                  1
 #define  ENABLE_TMR_PARAM_ISO180006B_DELIMITER                         1
 #define  ENABLE_TMR_PARAM_READ_ASYNCOFFTIME                            1
 #define  ENABLE_TMR_PARAM_READ_ASYNCONTIME                             1
 #define  ENABLE_TMR_PARAM_RADIO_ENABLEPOWERSAVE                        1
 #define  ENABLE_TMR_PARAM_RADIO_POWERMAX_POWERMIN                      1
 #define  ENABLE_TMR_PARAM_RADIO_WRITEPOWER                             1
 #define  ENABLE_TMR_PARAM_RADIO_PORTREADPOWERLIST                      1
 #define  ENABLE_TMR_PARAM_RADIO_PORTWRITEPOWERLIST                     1
 #define  ENABLE_TMR_PARAM_RADIO_TEMPERATURE                            1
 #define  ENABLE_TMR_PARAM_TAGREADDATA_RECORDHIGHESTRSSI                1
 #define  ENABLE_TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA                  1
 #define  ENABLE_TMR_PARAM_TAGREADDATA_UNIQUEBYDATA                     1
 #define  ENABLE_TMR_PARAM_TAGOP_ANTENNA                                1
 #define  ENABLE_TMR_PARAM_TAGOP_PROTOCOL                               1
 #define  ENABLE_TMR_PARAM_VERSION_HARDWARE                             1
 #define  ENABLE_TMR_PARAM_VERSION_SERIAL                               1
 #define  ENABLE_TMR_PARAM_VERSION_SUPPORTEDPROTOCOLS                   1
 #define  ENABLE_TMR_PARAM_REGION_HOPTABLE                              1
 #define  ENABLE_TMR_PARAM_REGION_HOPTIME                               1
 #define  ENABLE_TMR_PARAM_REGION_LBT_ENABLE                            1
 #define  ENABLE_TMR_PARAM_REGION_DWELL_TIME_ENABLE                     1
 #define  ENABLE_TMR_PARAM_REGION_LBT_THRESHOLD                         1
 #define  ENABLE_TMR_PARAM_REGION_DWELL_TIME                            1
 #define  ENABLE_TMR_PARAM_LICENSE_KEY                                  1
 #define  ENABLE_TMR_PARAM_USER_CONFIG                                  1
 #define  ENABLE_TMR_PARAM_RADIO_ENABLESJC                              1
 #define  ENABLE_TMR_PARAM_READER_STATISTICS                            1
 #define  ENABLE_TMR_PARAM_READER_STATS                                 1
 #define  ENABLE_TMR_PARAM_URI                                          1
 #define  ENABLE_TMR_PARAM_PRODUCT_GROUP_ID                             1
 #define  ENABLE_TMR_PARAM_PRODUCT_GROUP                                1
 #define  ENABLE_TMR_PARAM_PRODUCT_ID                                   1
 #define  ENABLE_TMR_PARAM_TAGREADATA_TAGOPSUCCESSCOUNT                 1
 #define  ENABLE_TMR_PARAM_TAGREADATA_TAGOPFAILURECOUNT                 1
 #define  ENABLE_TMR_PARAM_TAGREADDATA_ENABLEREADFILTER                 1
 #define  ENABLE_TMR_PARAM_STATUS_ENABLE_ANTENNAREPORT                  1
 #define  ENABLE_TMR_PARAM_STATUS_ENABLE_FREQUENCYREPORT                1
 #define  ENABLE_TMR_PARAM_STATUS_ENABLE_TEMPERATUREREPORT              1
 #define  ENABLE_TMR_PARAM_TAGREADDATA_READFILTERTIMEOUT                1
 #define  ENABLE_TMR_PARAM_TAGREADDATA_UNIQUEBYPROTOCOL                 1
 #define  ENABLE_TMR_PARAM_READER_DESCRIPTION                           1
 #define  ENABLE_TMR_PARAM_READER_HOSTNAME                              1
 #define  ENABLE_TMR_PARAM_CURRENTTIME                                  1
 #define  ENABLE_TMR_PARAM_READER_WRITE_REPLY_TIMEOUT_EARLY_EXIT        1
 #define  ENABLE_TMR_PARAM_READER_STATS_ENABLE                          1
 #define  ENABLE_TMR_PARAM_TRIGGER_READ_GPI                             1
 #define  ENABLE_TMR_PARAM_GEN2_T4                                      1
 #define  ENABLE_TMR_PARAM_LICENSED_FEATURES                            1
 #define  ENABLE_TMR_PARAM_SELECTED_PROTOCOLS                           1
 #define  ENABLE_TMR_PARAM_REGION_QUANTIZATION_STEP                     1
 #define  ENABLE_TMR_PARAM_REGION_MINIMUM_FREQUENCY                     1
 #define  ENABLE_TMR_PARAM_MANAGE_LICENSE_KEY                           1
 #define  ENABLE_TMR_PARAM_REGULATORY_MODE                              1
 #define  ENABLE_TMR_PARAM_REGULATORY_MODULATION                        1
 #define  ENABLE_TMR_PARAM_REGULATORY_ONTIME                            1
 #define  ENABLE_TMR_PARAM_REGULATORY_OFFTIME                           1
 #define  ENABLE_TMR_PARAM_REGULATORY_ENABLE                            1
 #define  ENABLE_TMR_PARAM_ISO14443A_TAGTYPE                            1
 #define  ENABLE_TMR_PARAM_ISO14443B_TAGTYPE                            1
 #define  ENABLE_TMR_PARAM_ISO15693_TAGTYPE                             1
 #define  ENABLE_TMR_PARAM_LF125KHZ_TAGTYPE                             1
 #define  ENABLE_TMR_PARAM_LF134KHZ_TAGTYPE                             1
 #define  ENABLE_TMR_PARAM_ISO14443A_SUPPORTED_TAGTYPES                 1
 #define  ENABLE_TMR_PARAM_ISO14443B_SUPPORTED_TAGTYPES                 1
 #define  ENABLE_TMR_PARAM_ISO15693_SUPPORTED_TAGTYPES                  1
 #define  ENABLE_TMR_PARAM_LF125KHZ_SUPPORTED_TAGTYPES                  1
 #define  ENABLE_TMR_PARAM_LF134KHZ_SUPPORTED_TAGTYPES                  1
 #define  ENABLE_TMR_PARAM_ISO14443A_SUPPORTED_TAG_FEATURES             1
 #define  ENABLE_TMR_PARAM_ISO15693_SUPPORTED_TAG_FEATURES              1
 #define  ENABLE_TMR_PARAM_LF125KHZ_SUPPORTED_TAG_FEATURES              1
 #define  ENABLE_TMR_PARAM_LF125KHZ_SECURE_RD_FORMAT                    1
 #define  ENABLE_TMR_PARAM_RADIO_KEEP_RF_ON                             1
 #define  ENABLE_TMR_PARAM_PROTOCOL_LIST                                1

/**
  Define TMR_ENABLE_CRC for USB interface.
*/
#define TMR_ENABLE_CRC                                                  0

/**
 * Define this to enable support for local serial port access via
 * native interfaces (COM1 on Windows, /dev/ttyS0, /dev/ttyUSB0 on
 * Linux, etc.).
 */
#define TMR_ENABLE_SERIAL_TRANSPORT_NATIVE

/**
 * The longest possible name for a reader.
 */
#define TMR_MAX_READER_NAME_LENGTH 64

/**
 * The maximum number of protocols supported in a multiprotocol search command
 */
#define TMR_MAX_SERIAL_MULTIPROTOCOL_LENGTH 32

/**
 * The maximum size of the Queue, used to share the streamed messages
 * between the do_background_reads thread and parse_tag_reads thread.
 */
#define TMR_MAX_QUEUE_SLOTS 20

/** 
 * Number of bytes to allocate for embedded data return
 * in each TagReadData.
 */
#define TMR_MAX_EMBEDDED_DATA_LENGTH 128 

/**
 * The maximum length of the probe baudrate list. This list specifies the  
 * baudrates which are used while connecting to the serial reader.
 */
#define TMR_MAX_PROBE_BAUDRATE_LENGTH 8

/* The minimum and maximum values of AsyncOn and AsyncOff time. */
#define TMR_MAX_VALUE 65535u
#define TMR_MIN_VALUE 0u

/**
 * Define this to enable support for LLRP readers.
 * (Not yet available for Windows)
 */
#define TMR_ENABLE_LLRP_TRANSPORT
#ifndef WIN32
#define TMR_ENABLE_LLRP_SERIAL_TRANSPORT
#endif

/**
 * LLRP reader keep alive timeout in milli seconds
 */
#define TMR_LLRP_KEEP_ALIVE_TIMEOUT 5000

/**
 * Define this to enable async read using single thread

Note:To run readsync_baremetal codelet you have to 
     define it.
	 Otherwise keep it undefined.
 */
//#define SINGLE_THREAD_ASYNC_READ
/**
 * Define this to enable for BARE_METAL API 
 */
#ifndef BARE_METAL
/**
 * Define this to enable support for the GEN2 Custom Tag Operations parameters
 * and access commands
 */
#define TMR_ENABLE_GEN2_CUSTOM_TAGOPS

/**
 * Define this to enable support for the ISO180006B protocol parameters
 * and access commands
 */
#define TMR_ENABLE_ISO180006B

/**
 * Define this to enable support for small microcontrollers.
 * Enabling this option will reduce the code which is not relevant to 
 * serial readers in order to minimize the program footprint
 * To compile for Serial Reader only.
 *
 * To compile for Serial Reader only, use one of the following options:
 ** Uncomment the following define
 ** Run make with TMR_ENABLE_SERIAL_READER_ONLY=1 defined
 ** Add -DTMR_ENABLE_SERIAL_READER_ONLY=1 to your compiler flags
 */ 
//#define TMR_ENABLE_SERIAL_READER_ONLY

/**
 * Define this to enable support for background reads using native threads.
 * This feature is also available for windows (using pthreads-win32)
 */

#define TMR_ENABLE_BACKGROUND_READS


/**
 * Define this to include TMR_strerror().
 */
#define TMR_ENABLE_ERROR_STRINGS

/**
 * Define this to include TMR_paramName() and TMR_paramID().
 */
#define TMR_ENABLE_PARAM_STRINGS

/**
 * Enabling  this option will enable the support for the parameters defined 
 * in stdio.h header file like FILE *. This check is required as stdio.h does not
 * exist in some of the embedded  architectures.
 */
#define  TMR_ENABLE_STDIO
  

/**
 * Enabling  this option will enable the support for the parameters defined 
 * in string.h header file like strerror(). This check is required as string.h does not
 * exist in some of the embedded  architectures.
 */
#define  TMR_USE_STRERROR
#if defined(WINCE)
#undef TMR_USE_STRERROR
#endif

#else
/**
 * Define this to enable async read using single thread
 */
#define SINGLE_THREAD_ASYNC_READ

/**
 * Define this to enable support for the GEN2 Custom Tag Operations parameters
 * and access commands
 */
#undef TMR_ENABLE_GEN2_CUSTOM_TAGOPS

/**
 * Define this to enable support for the ISO180006B protocol parameters
 * and access commands
 */
#undef TMR_ENABLE_ISO180006B

/**
 * Define this to enable support for small microcontrollers.
 * Enabling this option will reduce the code which is not relevant to 
 * serial readers in order to minimize the program footprint
 * To compile for Serial Reader only.
 *
 * To compile for Serial Reader only, use one of the following options:
 ** Uncomment the following define
 ** Run make with TMR_ENABLE_SERIAL_READER_ONLY=1 defined
 ** Add -DTMR_ENABLE_SERIAL_READER_ONLY=1 to your compiler flags
 */ 
#define TMR_ENABLE_SERIAL_READER_ONLY

/**
 * Define this to enable support for background reads using native threads.
 * This feature is also available for windows (using pthreads-win32)
 */

#undef TMR_ENABLE_BACKGROUND_READS


/**
 * Define this to include TMR_strerror().
 */
#undef TMR_ENABLE_ERROR_STRINGS

/**
 * Define this to include TMR_paramName() and TMR_paramID().
 */
#undef TMR_ENABLE_PARAM_STRINGS

/**
 * Enabling  this option will enable the support for the parameters defined 
 * in stdio.h header file like FILE *. This check is required as stdio.h does not
 * exist in some of the embedded  architectures.
 */
#undef  TMR_ENABLE_STDIO
  
/**
 * Enabling  this option will enable the support for the parameters defined 
 * in string.h header file like strerror(). This check is required as string.h does not
 * exist in some of the embedded  architectures.
 */
#undef  TMR_USE_STRERROR

#endif    /*Bare_metal*/

/**
 * Define when compiling on a big-endian host platform to enable some
 * endian optimizations. Without this, no endianness will be assumed.
 */ 
#undef TMR_BIG_ENDIAN_HOST

/**
 * Define this to enable API-side tag read deduplication.  Under
 * certain conditions, the module runs out of buffer space to detect
 * reads of a previously-seen EPC.  
 */
#define TMR_ENABLE_API_SIDE_DEDUPLICATION

/**
 * Default read filter timeout.
 */
#define TMR_DEFAULT_READ_FILTER_TIMEOUT -1

/**
 * The max length of the custom transport scheme name
 */
#define TMR_MAX_TRANSPORT_SCHEME_NAME 50

#define TMR_ENABLE_ALL

#if (defined(TMR_ENABLE_HF_LF) || (defined(TMR_ENABLE_UHF))) 
/** Undefine the macro, because the API should get build either for UHF or HF/LF modules based 
 *  on the macro defined in project settings
 */
#undef TMR_ENABLE_ALL
#endif

/** Build the API for both UHF and HF/LF modules only if 'TMR_ENABLE_ALL' is defined */
#ifdef TMR_ENABLE_ALL
/** Too build API for HF and LF modules */
#define TMR_ENABLE_HF_LF

/** To build API for UHF modules */
#define TMR_ENABLE_UHF
#endif

#if (defined(TMR_ENABLE_HF_LF) && (!defined(TMR_ENABLE_UHF)))

  /**
   * To compile for Serial Reader only
   */
  #define TMR_ENABLE_SERIAL_READER_ONLY
  /**
   * Define this to enable support for the ISO180006B protocol parameters
   * and access commands
   */
  #undef TMR_ENABLE_ISO180006B
  /**
   * Define this to enable support for the GEN2 Custom Tag Operations parameters
   * and access commands
   */
  #undef TMR_ENABLE_GEN2_CUSTOM_TAGOPS
#endif

  /**
   * Maximum length of tag UID
   */
  #define TMR_MAX_UID_BYTE_COUNT 10

/**
 * Define the largest number serial reader antenna ports that will be supported
 */
#ifdef TMR_ENABLE_UHF
#define TMR_SR_MAX_ANTENNA_PORTS (64)
#else
/* M3e does not support antenna multiplexing, so antenna count will always be 2. */
#define TMR_SR_MAX_ANTENNA_PORTS (2)
#endif

#if (defined(TMR_ENABLE_ALL) || (defined(TMR_ENABLE_HF_LF)))

#define TMR_ENABLE_EXTENDED_TAGOPS 1

#if TMR_ENABLE_EXTENDED_TAGOPS
/**
 * Define this to enable support for the DesFire Tag Operations.
 */
#define TMR_ENABLE_DESFIRE_TAGOPS 1
/**
 * Define this to enable support for the Ultralight NTag Operations.
 */
#define TMR_ENABLE_UL_NTAG_TAGOPS 1
#endif /* TMR_ENABLE_EXTENDED_TAGOPS */

#endif /* TMR_ENABLE_ALL | TMR_ENABLE_HF_LF */

#ifdef __cplusplus
}
#endif

#endif /* _TM_CONFIG_H */
