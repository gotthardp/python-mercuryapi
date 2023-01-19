#ifndef _TMR_LLRP_READER_H
#define _TMR_LLRP_READER_H
/**
 *  @file tmr_llrp_reader.h
 *  @brief Mercury API - LLRP Reader interface
 *  @author Somu
 *  @date 05/23/2011
 */

/*
 * Copyright (c) 2011 ThingMagic, Inc.
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

#include <pthread.h>
#include <semaphore.h>
#include "tmr_region.h"
#include "tmr_tag_protocol.h"
#include "tmr_serial_transport.h"
#include "tmr_params.h"
#include "ltkc.h"
#include "tm_ltkc.h"

#ifdef __cplusplus
extern "C" {
#endif
 
#ifdef TMR_ENABLE_UHF
#define TM_MANUFACTURER_ID  26554
#define TMR_LLRP_SYNC_MAX_ROSPECS 256  
#define TMR_LLRP_MAX_RFMODE_ENTRIES 7
#define TMR_LLRP_READER_DEFAULT_PORT 5084

/**
 * This structure is returned from cmdGetRFControl
 **/
typedef struct TMR_LLRP_RFControl
{
  /* RFMode index */
  llrp_u16_t index;

  /* Tari */
  TMR_GEN2_Tari tari;

} TMR_LLRP_RFControl;

/**
 * Gen2 RF Mode Table structure.
 * Currently we use only BLF and Encoding from 
 * RF Mode table. 
 **/
typedef struct TMR_LLRP_C1G2RFModeTable
{
  /* Gen2 BLF */
  TMR_GEN2_LinkFrequency blf;

  /* Gen2 Tag encoding */
  TMR_GEN2_TagEncoding m;

  /* Min Tari Value*/
  TMR_GEN2_Tari minTari;

  /* Max Tari Value*/
  TMR_GEN2_Tari maxTari;

} TMR_LLRP_C1G2RFModeTable;

/**
 * This structure is returned from cmdGetReaderCapabilities.
 **/
typedef struct TMR_LLRP_ReaderCapabilities
{
  /** Country code */
  llrp_u16_t countryCode;

  /** Reader model */
  uint32_t model;

  /** Software version */
  char softwareVersion[128];

  /** Power level table */
  TMR_uint16List powerTable;
  uint16_t powerList[255];

  /** Frequency table */
  TMR_uint32List freqTable;
  uint32_t freqList[64];

  union
  {
    /** 
     * Gen2 RF Mode tables
     * FIXME: Max RFModes available for now is 5.
     * and the index start from 1-5
     **/
    TMR_LLRP_C1G2RFModeTable gen2Modes[TMR_LLRP_MAX_RFMODE_ENTRIES];
    /**
     * Can have RF Mode table specific to 
     * other protocols. 
     **/
  } u;

} TMR_LLRP_ReaderCapabilities;

/** 
 * This struture used to
 * moniter the protocol
 * in read plan
 **/
typedef struct ROSpecProtocolTable
{
  /* rospecid */
  uint8_t rospecID;
  /*protocol */
  TMR_TagProtocol rospecProtocol;
}ROSpecProtocolTable;

/**
 * This is the structure used to
 * store the unhandled async response
 **/
typedef struct TMR_LLRP_UnhandledAsyncResponse
{
  /** Buffer to hold the LLRP message */
  LLRP_tSMessage *lMsg;
}TMR_LLRP_UnhandledAsyncResponse;

/**
 *  Reader features Flag Enum
 */
typedef enum TMMP_Reader_FeaturesFlag
{
  TMMP_READER_FEATURES_FLAG_NONE = 0x00,
  /** Multipe select feature support flag */
  TMMP_READER_FEATURES_FLAG_MULTI_SELECT = (1 << 0),
  /** PerAntenna ontime feature support flag */
  TMMP_READER_FEATURES_FLAG_PERANTENNA_ONTIME = (1 << 1),
  /** Custom Inventory Spec ID support flag */
  TMMP_READER_FEATURES_FLAG_INVENTORYSPEC_ID = (1 << 2),
  /** stats listener in async read support flag */
  TMMP_READER_FEATURES_FLAG_STATS_LISTENER = (1 << 3),
  TMMP_READER_FEATURES_FLAG_STATEAWARE_TARGET_MAPPED = (1 << 4),

  /* ALL */
  TMMP_READER_FEATURES_FLAG_ALL = (TMMP_READER_FEATURES_FLAG_MULTI_SELECT | TMMP_READER_FEATURES_FLAG_PERANTENNA_ONTIME
                                    | TMMP_READER_FEATURES_FLAG_INVENTORYSPEC_ID |  TMMP_READER_FEATURES_FLAG_STATS_LISTENER
                                    | TMMP_READER_FEATURES_FLAG_STATEAWARE_TARGET_MAPPED)
}TMMP_Reader_FeaturesFlag;

/**
 * LLRP reader structure
 */
typedef struct TMR_LLRP_LlrpReader
{
  /* @privatesection */
  LLRP_tSTypeRegistry *       pTypeRegistry;
  LLRP_tSConnection *         pConn;
  int                         portNum;
  llrp_u32_t                  msgId;
  llrp_u32_t                  roSpecId, accessSpecId;
  llrp_u16_t                  opSpecId;

  TMR_AntennaMapList *txRxMap;
  uint32_t transportTimeout;
  uint32_t commandTimeout;
  TMR_Region  regionId;
  TMR_GEN2_Password gen2AccessPassword;

  /* Static storage for the default map */
  TMR_AntennaMap staticTxRxMapData[TMR_SR_MAX_ANTENNA_PORTS];
  TMR_AntennaMapList staticTxRxMap;

  uint32_t portMask;
  /* Bit mask of supported protocol list */
  uint32_t supportedProtocols;

  TMR_TagProtocol currentProtocol;

  /* Buffer to store additional error message */
  /* TODO: Change TMR_SR_MAX_PACKET_SIZE to appropriate value */
  char errMsg[TMR_SR_MAX_PACKET_SIZE];

  /* Large bitmask that stores whether each parameter's presence
   * is known or not.
   */
  uint32_t paramConfirmed[TMR_PARAMWORDS];

  /* Large bitmask that, if the corresponding bit in paramConfirmed is set,
   * stores whether each parameter is present or not.
   */
  uint32_t paramPresent[TMR_PARAMWORDS];
  
  /* Number of tags reported by reader */
  int tagsRemaining;

  /* Array of LLRP_tSMessage pointers holding the tag read responses */
  LLRP_tSMessage **bufResponse;

  /* bufResponse read index */
  uint8_t bufPointer;
  uint8_t bufIndex;

  /* Pointer to buffer holding the tag read data */
  LLRP_tSTagReportData *pTagReportData;

  int searchTimeoutMs;

  /* Cache LLRP Reader Capabilities */
  TMR_LLRP_ReaderCapabilities capabilities;

  /**
   * LLRP Asynchronous receiver to handle
   * keep alive and events.
   **/
  pthread_t llrpReceiver;
  bool receiverSetup, receiverRunning, receiverEnabled;
  pthread_cond_t receiverCond;
  int numOfROSpecEvents;
  /** The above variables must be protected by this lock */
  pthread_mutex_t receiverLock;

  /**
   * For monitoring keepalives:
   * Used only in case of async reading
   **/
  uint64_t ka_start, ka_now;
  bool get_report, reportReceived;
  /**
   * For monitoring protocol type
   * in case of multiprotocol read
   **/
  ROSpecProtocolTable readPlanProtocol[TMR_SR_MAX_PACKET_SIZE];

  /**
   * For monitoring the unhandeled responses
   * during the async read
   **/
  TMR_LLRP_UnhandledAsyncResponse unhandledAsyncResponse;
  /** To check the status of async unhandled responses */
  bool isResponsePending;
  /** To cancel the receiver thread */
  bool threadCancel;
  /* To hold the No.of Keep alives missed count*/
  uint8_t keepAliveAckMissCnt;
  TMMP_Reader_FeaturesFlag featureFlags;
  pthread_mutex_t transmitterLock;
  /* Cache metadata flag status */
  TMR_TRD_MetadataFlag metadata;
  uint16_t statsEnable;
}TMR_LLRP_LlrpReader;


TMR_Status TMR_LLRP_connect(TMR_Reader *reader);
TMR_Status TMR_LLRP_destroy(TMR_Reader *reader);
TMR_Status TMR_LLRP_hasMoreTags(TMR_Reader *reader); 
TMR_Status TMR_LLRP_getNextTag(TMR_Reader *reader, TMR_TagReadData *trd);
TMR_Status TMR_LLRP_executeTagOp(TMR_Reader *reader, TMR_TagOp *tagop, TMR_TagFilter *filter, TMR_uint8List *data);
TMR_Status TMR_LLRP_gpiGet(struct TMR_Reader *reader,uint8_t *count, TMR_GpioPin state[]);
TMR_Status TMR_LLRP_gpoSet(struct TMR_Reader *reader,uint8_t count, const TMR_GpioPin state[]);
TMR_Status TMR_LLRP_readTagMemBytes(struct TMR_Reader *reader, const TMR_TagFilter *filter,
                                    uint32_t bank, uint32_t addreass,
                                    uint16_t count,uint8_t data[]);
TMR_Status TMR_LLRP_readTagMemWords(struct TMR_Reader *reader, const TMR_TagFilter *filter,
                                    uint32_t bank, uint32_t address,
                                    uint16_t count, uint16_t *data);
TMR_Status TMR_LLRP_writeTagMemBytes(struct TMR_Reader *reader, const TMR_TagFilter *filter,
                                    uint32_t bank, uint32_t addreass,
                                    uint16_t count,const uint8_t data[]);
TMR_Status TMR_LLRP_writeTagMemWords(struct TMR_Reader *reader, const TMR_TagFilter *filter,
                                    uint32_t bank, uint32_t address,
                                    uint16_t count, const uint16_t *data);
TMR_Status TMR_LLRP_firmwareLoad( TMR_Reader *reader, void *cookie, TMR_FirmwareDataProvider provider);
TMR_Status TMR_LLRP_writeTag(TMR_Reader *reader, const TMR_TagFilter *filter, const TMR_TagData *data);
TMR_Status TMR_LLRP_killTag(struct TMR_Reader *reader, const TMR_TagFilter *filter,
                             const TMR_TagAuthentication *auth);
TMR_Status TMR_LLRP_lockTag(struct TMR_Reader *reader,const TMR_TagFilter *filter, TMR_TagLockAction *action);
TMR_Status TMR_LLRP_reboot(struct TMR_Reader *reader);
//TMR_Status TMR_LLRP_resetHoptable(struct TMR_Reader *reader);
/**
 * Initialize LLRP reader.
 */
TMR_Status TMR_LLRP_LlrpReader_init(TMR_Reader *reader);

TMR_Status TMR_LLRP_read(struct TMR_Reader *reader, uint32_t timeoutMs, int32_t *tagCount);
#endif /* TMR_ENABLE_UHF */
#ifdef __cplusplus
}
#endif

#endif /* _TMR_LLRP_READER_H */
