#ifndef _LLRP_READER_IMP_H
#define _LLRP_READER_IMP_H

/**
 *  @file llrp_reader_imp.h
 *  @brief LLRP reader internal implementation header
 *  @author Somu
 *  @date 05/25/2011
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

#include <time.h>
#include "tm_reader.h"
#include "tmr_status.h"
#include "ltkc.h"

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * Magic Number to reboot the Thingmagic LLRP
 * fixed reader.
 **/
#define TMR_POWER_CYCLE_MAGIC_NUMBER 0x20000920  

/**
 * LLRP specific AccessCommandOpSpecResult parameter typenums
 **/
#define TMR_LLRP_LISTOPSPECRESULT 501
#define TMR_LLRP_C1G2READOPSPECRESULT 349
#define TMR_LLRP_C1G2WRITEOPSPECRESULT 350
#define TMR_LLRP_C1G2KILLOPSPECRESULT 351
#define TMR_LLRP_C1G2LOCKOPSPECRESULT 352
#define TMR_LLRP_C1G2BLOCKERASEOPSPECRESULT 353
#define TMR_LLRP_C1G2BLOCKWRITEOPSPECRESULT 354
#define TMR_LLRP_CUSTOM_BLOCKPERMALOCKOPSPECRESULT 22
#define TMR_LLRP_CUSTOM_HIGGS2PARTIALLOADIMAGEOPSPECRESULT 24
#define TMR_LLRP_CUSTOM_HIGGS2FULLLOADIMAGEOPSPECRESULT 26
#define TMR_LLRP_CUSTOM_HIGGS3FASTLOADIMAGEOPSPECRESULT 28
#define TMR_LLRP_CUSTOM_HIGGS3LOADIMAGEOPSPECRESULT 30
#define TMR_LLRP_CUSTOM_HIGGS3BLOCKREADLOCKOPSPECRESULT 32
#define TMR_LLRP_CUSTOM_G2ISETREADPROTECTOPSPECRESULT 34                               
#define TMR_LLRP_CUSTOM_G2XSETREADPROTECTOPSPECRESULT 36                               
#define TMR_LLRP_CUSTOM_G2IRESETREADPROTECTOPSPECRESULT 38                             
#define TMR_LLRP_CUSTOM_G2XRESETREADPROTECTOPSPECRESULT 40                             
#define TMR_LLRP_CUSTOM_G2ICHANGEEASOPSPECRESULT 42                                    
#define TMR_LLRP_CUSTOM_G2XCHANGEEASOPSPECRESULT 44                                    
#define TMR_LLRP_CUSTOM_G2IEASALARMOPSPECRESULT 46                                     
#define TMR_LLRP_CUSTOM_G2XEASALARMOPSPECRESULT 48                                     
#define TMR_LLRP_CUSTOM_G2ICALIBRATEOPSPECRESULT 50                                    
#define TMR_LLRP_CUSTOM_G2XCALIBRATEOPSPECRESULT 52                                    
#define TMR_LLRP_CUSTOM_G2ICHANGECONFIGOPSPECRESULT 54                                 
#define TMR_LLRP_CUSTOM_MONZA4QTREADWRITEOPSPECRESULT 57
#define TMR_LLRP_CUSTOM_IDS_SETSFEPARAMETERSOPSPECRESULT 99
#define TMR_LLRP_CUSTOM_IDS_GETMEASUREMENTSETUPOPSPECRESULT 93
#define TMR_LLRP_CUSTOM_IDS_GETBATTERYLEVELOPSPECRESULT 103
#define TMR_LLRP_CUSTOM_IDS_SETLOGLIMITSOPSPECRESULT 116
#define TMR_LLRP_CUSTOM_IDS_SETSHELFLIFEOPSPECRESULT 124
#define TMR_LLRP_CUSTOM_IDS_SETPASSWORDOPSPECRESULT 118
#define TMR_LLRP_CUSTOM_WRITETAGOPSPECRESULT  61
#define TMR_LLRP_CUSTOM_IDS_GETSENSORVALUEOPSPECRESULT 77
#define TMR_LLRP_CUSTOM_IDS_SETLOGMODEOPSPECRESULT 85
#define TMR_LLRP_CUSTOM_IDS_STARTLOGMODEOPSPECRESULT 87
#define TMR_LLRP_CUSTOM_IDS_GETLOGSTATEOPSPECRESULT 79
#define TMR_LLRP_CUSTOM_IDS_ENDLOGOPSPECRESULT 89
#define TMR_LLRP_CUSTOM_IDS_INITIALIZEOPSPECRESULT 91
#define TMR_LLRP_CUSTOM_IDS_ACCESSFIFOSTATUSOPSPECRESULT 101
#define TMR_LLRP_CUSTOM_IDS_ACCESSFIFOWRITEOPSPECRESULT 114
#define TMR_LLRP_CUSTOM_IDS_ACCESSFIFOREADOPSPECRESULT 112
#define TMR_LLRP_CUSTOM_IDS_GETCALIBRATIONDATAOPSPECRESULT 95
#define TMR_LLRP_CUSTOM_IDS_SETCALIBRATIONDATAOPSPECRESULT 97
#define TMR_LLRP_CUSTOM_DENATRAN_IAV_ACTIVATESECUREMODEOPSPECRESULT 127
#define TMR_LLRP_CUSTOM_DENATRAN_IAV_AUTHENTICATEOBUOPSPECRESULT 129
#define TMR_LLRP_CUSTOM_DENATRAN_IAV_ACTIVATESINIAVMODEOPSPECRESULT 131
#define TMR_LLRP_CUSTOM_DENATRAN_IAV_AUTHENTICATEIDOPSPECRESULT 133
#define TMR_LLRP_CUSTOM_DENATRAN_IAV_AUTHENTICATEFULLPASS1OPSPECRESULT 135
#define TMR_LLRP_CUSTOM_DENATRAN_IAV_AUTHENTICATEFULLPASS2OPSPECRESULT 137
#define TMR_LLRP_CUSTOM_DENATRAN_IAV_OBUREADFROMMEMMAPOPSPECRESULT 139
#define TMR_LLRP_CUSTOM_DENATRAN_IAV_OBUWRITETOMEMMAPOPSPECRESULT 141
#define TMR_LLRP_CUSTOM_DENATRAN_IAV_G0PAOBUAUTHOPSPECRESULT 185
#define TMR_LLRP_CUSTOM_DENATRAN_IAV_OBUAUTHFULLPASSOPSPECRESULT 187
#define TMR_LLRP_CUSTOM_DENATRAN_IAV_READSECTOROPSPECRESULT 189
#define TMR_LLRP_CUSTOM_DENATRAN_IAV_WRITESECTOROPSPECRESULT 191
#define TMR_LLRP_CUSTOM_DENATRAN_IAV_GETTOKENIDOPSPECRESULT 193
#define TMR_LLRP_CUSTOM_NXP_AUTHENTICATEOPSPECRESULT 151
#define TMR_LLRP_CUSTOM_NXP_READBUFFEROPSPECRESULT 155
#define TMR_LLRP_CUSTOM_NXP_UNTRACEABLEOPSPECRESULT 157
#ifdef TMR_ENABLE_ISO180006B
#define TMR_LLRP_CUSTOM_ISO_READDATAOPSPECRESULT  65
#define TMR_LLRP_CUSTOM_ISO_WRITEDATAOPSPECRESULT 68
#define TMR_LLRP_CUSTOM_ISO_LOCKOPSPECRESULT 70
#endif /* TMR_ENABLE_ISO180006B */

/**
 * Maximum number of ROSpecs supported by reader.
 **/
#define TMR_LLRP_MAX_ROSPECS 32
/**
 * Maximum milliseconds required for reader to stop an ongoing search
 **/
#define TMR_LLRP_STOP_TIMEOUT 5000

/**
 * This structure is returned from TMR_LLRP_cmdAntennaDetect.
 **/
typedef struct TMR_LLRP_PortDetect
{
  /** The port number. */
  uint16_t port;
  /** Whether the antenna is connected? */
  bool connected;
  /** Antenna gain */
  int16_t gain;
}TMR_LLRP_PortDetect;

/**
 * This struture is returned from ThingMagicDeDuplication
 **/
typedef struct TMR_LLRP_TMDeDuplication
{
  /** Record Highest RSSI */
  llrp_u1_t highestRSSI;

  /** Unique By Antenna */
  llrp_u1_t uniquebyAntenna;

  /** Unique By Data */
  llrp_u1_t uniquebyData;
}TMR_LLRP_TMDeDuplication;
  
/**
 * This structure is returned from cmdGetThingmagicReaderConfiguration
 **/
typedef struct TMR_LLRP_TMReaderConfiguration
{
  /** Reader description */
  llrp_utf8v_t description;
  /** Reader Role */
  llrp_utf8v_t role;
  /** Reader host name */
  llrp_utf8v_t hostName;
} TMR_LLRP_TMReaderConfiguration;

/**
 * This value is returned from TMR_LLRP_cmdVersionModel.
 **/
# define  TMR_LLRP_MODEL_M6  0x06
# define  TMR_LLRP_MODEL_ASTRA_EX 0x30
# define  TMR_LLRP_MODEL_SARGAS 0x3430
# define  TMR_LLRP_MODEL_IZAR 0x3530
# define  TMR_LLRP_MODEL_ASTRA200 0x3630

TMR_Status TMR_LLRP_notifyTransportListener(TMR_Reader *reader, LLRP_tSMessage *pMsg, bool tx, int timeout);
TMR_Status TMR_LLRP_sendMessage(TMR_Reader *reader, LLRP_tSMessage *pMsg, int timeoutMs);
TMR_Status TMR_LLRP_receiveMessage(TMR_Reader *reader, LLRP_tSMessage **pMsg, int timeoutMs);
TMR_Status TMR_LLRP_sendTimeout(TMR_Reader *reader, LLRP_tSMessage *pMsg, LLRP_tSMessage **pRsp, int timeoutMs);
TMR_Status TMR_LLRP_send(TMR_Reader *reader, LLRP_tSMessage *pMsg, LLRP_tSMessage **pRsp);
void TMR_LLRP_freeMessage(LLRP_tSMessage *pMsg);
TMR_Status TMR_LLRP_checkLLRPStatus(LLRP_tSLLRPStatus *pLLRPStatus);

TMR_Status TMR_LLRP_cmdGetRegion(TMR_Reader *reader, TMR_Region *region);
TMR_Status TMR_LLRP_cmdAntennaDetect(TMR_Reader *reader, uint8_t *count, TMR_LLRP_PortDetect *ports);
TMR_Status TMR_LLRP_cmdGetTMAsyncOffTime(TMR_Reader *reader, uint32_t *offtime);
TMR_Status TMR_LLRP_cmdGetTMAsyncOnTime(TMR_Reader *reader, uint32_t *ontime);
TMR_Status TMR_LLRP_cmdGetTMMetadataFlag(TMR_Reader *reader, uint16_t *metadata);
TMR_Status TMR_LLRP_cmdGetTMStatsEnable(TMR_Reader *reader, uint16_t *statsEnable);
TMR_Status TMR_LLRP_cmdGetTMStatsValue(TMR_Reader *reader, TMR_Reader_StatsValues *statsValue);

TMR_Status TMR_LLRP_cmdGetGPIState(TMR_Reader *reader, uint8_t *count, TMR_GpioPin state[]);
TMR_Status TMR_LLRP_cmdSetGPOState(TMR_Reader *reader, uint8_t count, const TMR_GpioPin state[]);

TMR_Status TMR_LLRP_cmdGetReaderCapabilities(TMR_Reader *reader, TMR_LLRP_ReaderCapabilities *capabilities);
TMR_Status TMR_LLRP_cmdGetRegulatoryCapabilities(TMR_Reader *reader,  TMR_uint32List *table);

TMR_Status TMR_LLRP_cmdGetReadTransmitPowerList(TMR_Reader *reader, TMR_PortValueList *pPortValueList);
TMR_Status TMR_LLRP_cmdSetReadTransmitPowerList(TMR_Reader *reader, TMR_PortValueList *pPortValueList);

TMR_Status TMR_LLRP_cmdGetWriteTransmitPowerList(TMR_Reader *reader, TMR_PortValueList *pPortValueList);
TMR_Status TMR_LLRP_cmdSetWriteTransmitPowerList(TMR_Reader *reader, TMR_PortValueList *pPortValueList);
TMR_Status TMR_LLRP_cmdGetTMDeviceInformationCapabilities(TMR_Reader *reader, int param, TMR_String  *version);
TMR_Status TMR_LLRP_cmdGetTMDeviceInformationIDs(TMR_Reader *reader, int param, uint16_t *id);
TMR_Status TMR_LLRP_cmdGetActiveRFControl(TMR_Reader *reader, TMR_LLRP_RFControl *rfControl);
TMR_Status TMR_LLRP_cmdSetActiveRFControl(TMR_Reader *reader, TMR_LLRP_RFControl *rfControl);
TMR_Status TMR_LLRP_cmdGetGen2Session(TMR_Reader *reader, TMR_GEN2_Session  *session);
TMR_Status TMR_LLRP_cmdSetGen2Session(TMR_Reader *reader, TMR_GEN2_Session  *session);
TMR_Status TMR_LLRP_cmdSetTMAsyncOffTime(TMR_Reader *reader, uint32_t offtime);
TMR_Status TMR_LLRP_cmdSetTMAsyncOnTime(TMR_Reader *reader, uint32_t ontime);
TMR_Status TMR_LLRP_cmdSetTMMetadataFlag(TMR_Reader *reader, uint16_t metadata);
TMR_Status TMR_LLRP_cmdSetTMStatsEnable(TMR_Reader *reader, uint16_t statsEnable);
TMR_Status TMR_LLRP_cmdSetThingMagicPortSwitchGPOs(TMR_Reader *reader, TMR_uint8List *GPOList);

/*ThingMagic Licensed Features*/
TMR_Status TMR_LLRP_cmdGetLicensedFeatures(TMR_Reader *reader, TMR_uint8List  *features);

/*ThingMagic Selected Protocols*/
TMR_Status TMR_LLRP_cmdGetSelectedProtocols(TMR_Reader *reader, TMR_TagProtocolList *protocolList);

/* ThingMagic DeDuplication */
TMR_Status TMR_LLRP_cmdGetThingMagicDeDuplication(TMR_Reader *reader, TMR_LLRP_TMDeDuplication *duplication);
TMR_Status TMR_LLRP_cmdSetThingMagicDeDuplication(TMR_Reader *reader, TMR_LLRP_TMDeDuplication *duplication);

/* Thingmagic Reader configuration */
TMR_Status TMR_LLRP_cmdGetThingmagicReaderConfiguration(TMR_Reader *reader, TMR_LLRP_TMReaderConfiguration *config);
TMR_Status TMR_LLRP_cmdSetThingmagicReaderConfiguration(TMR_Reader *reader, TMR_LLRP_TMReaderConfiguration *config);
void TMR_LLRP_freeTMReaderConfiguration(TMR_LLRP_TMReaderConfiguration *config);

/* Thingmagic Current Time */
TMR_Status TMR_LLRP_cmdGetThingMagicCurrentTime(TMR_Reader *reader, struct tm *curTime);

/* Thingmagic Reader Module Temperature */
TMR_Status TMR_LLRP_cmdGetThingMagicReaderModuleTemperature(TMR_Reader *reader, int8_t *temp);

/* Thingmagic protocol configuration: Gen2 Q */
TMR_Status TMR_LLRP_cmdSetGen2Q(TMR_Reader *reader, TMR_GEN2_Q *q);
TMR_Status TMR_LLRP_cmdGetGen2Q(TMR_Reader *reader, TMR_GEN2_Q *q);

/* Thingmagic protocol configuration: InitQ */
TMR_Status TMR_LLRP_cmdSetInitQ(TMR_Reader *reader, TMR_GEN2_initQ *q);
TMR_Status TMR_LLRP_cmdGetInitQ(TMR_Reader *reader, TMR_GEN2_initQ *q);

/* Thingmagic protocol configuration: sendSelect */
TMR_Status TMR_LLRP_cmdSetsendSelect(TMR_Reader *reader, bool *Select);
TMR_Status TMR_LLRP_cmdGetsendSelect(TMR_Reader *reader, bool *Select);

/* Thingmagic Protocol configuration:Gen2 Target */
TMR_Status TMR_LLRP_cmdSetGen2Target(TMR_Reader *reader, TMR_GEN2_Target *target);
TMR_Status TMR_LLRP_cmdGetGen2Target(TMR_Reader *reader, TMR_GEN2_Target *target);

/* Thingmagic Protocol configuration:Gen2 T4 */
TMR_Status TMR_LLRP_cmdSetGen2T4Param(TMR_Reader *reader, const uint32_t t4Param);
TMR_Status TMR_LLRP_cmdGetGen2T4Param(TMR_Reader *reader, uint32_t *t4Param);

/* Thingmagic LicenseKey */
TMR_Status TMR_LLRP_cmdSetTMLicenseKey(TMR_Reader *reader, TMR_uint8List *license);

/* Thingmagic ISO 18K6B Delimiter */
TMR_Status TMR_LLRP_cmdSetISO18K6BDelimiter(TMR_Reader *reader, TMR_ISO180006B_Delimiter *delimiter);
TMR_Status TMR_LLRP_cmdGetISO18K6BDelimiter(TMR_Reader *reader, TMR_ISO180006B_Delimiter *delimiter);

/* Thingmagic ISO 18K6B Modulation Depth */
TMR_Status TMR_LLRP_cmdSetISO18K6BModDepth(TMR_Reader *reader, TMR_ISO180006B_ModulationDepth *modDepth);
TMR_Status TMR_LLRP_cmdGetISO18K6BModDepth(TMR_Reader *reader, TMR_ISO180006B_ModulationDepth *modDepth);

/* Thingmagic ISO 1806B Link Frequency */
TMR_Status TMR_LLRP_cmdSetISO18K6BLinkFrequency(TMR_Reader *reader, TMR_ISO180006B_LinkFrequency *linkFreq);
TMR_Status TMR_LLRP_cmdGetISO18K6BLinkFrequency(TMR_Reader *reader, TMR_ISO180006B_LinkFrequency *linkFreq);

/* Thingmagic Device Protocol Capabilities */
TMR_Status TMR_LLRP_cmdGetTMDeviceProtocolCapabilities(TMR_Reader *reader, TMR_TagProtocolList * protocol);

/* Thingmagic Device Antenna Detection */
TMR_Status TMR_LLRP_cmdGetThingMagicAntennaDetection(TMR_Reader *reader, bool *antennaport);
TMR_Status TMR_LLRP_cmdSetThingMagicAntennaDetection(TMR_Reader *reader, bool *antennaport);

/* Thingmagic Device GEN2 Protocol Extensions */
TMR_Status TMR_LLRP_cmdGetThingMagicGEN2ProtocolExtension(TMR_Reader *reader, uint8_t *protocolExt);

/* Thingmagic Regulatory configuration */
TMR_Status TMR_LLRP_cmdSetThingMagicRegulatoryMode(TMR_Reader *reader, uint16_t modeParam);
TMR_Status TMR_LLRP_cmdSetThingMagicRegulatoryModulation(TMR_Reader *reader, uint16_t modulationParam);
TMR_Status TMR_LLRP_cmdSetThingMagicRegulatoryEnable(TMR_Reader *reader, bool enableParam);
TMR_Status TMR_LLRP_cmdSetThingMagicRegulatoryOntime(TMR_Reader *reader, uint32_t ontimeParam);
TMR_Status TMR_LLRP_cmdSetThingMagicRegulatoryOfftime(TMR_Reader *reader, uint32_t offtimeParam);
TMR_Status TMR_LLRP_cmdGetThingMagicRegulatoryMode(TMR_Reader *reader, uint16_t *modeParam);
TMR_Status TMR_LLRP_cmdGetThingMagicRegulatoryModulation(TMR_Reader *reader, uint16_t *modulationParam);
TMR_Status TMR_LLRP_cmdGetThingMagicRegulatoryEnable(TMR_Reader *reader, bool *enableParam);
TMR_Status TMR_LLRP_cmdGetThingMagicRegulatoryOntime(TMR_Reader *reader, uint32_t *ontimeParam);
TMR_Status TMR_LLRP_cmdGetThingMagicRegulatoryOfftime(TMR_Reader *reader, uint32_t *offtimeParam);
TMR_Status TMR_LLRP_cmdGetTMAntennaReturnloss(TMR_Reader *reader, TMR_PortValueList *returnloss);

TMR_Status TMR_LLRP_cmdSetThingMagicRegionHoptable(TMR_Reader *reader, const TMR_uint32List *hopTableParam);

/* Read  */
TMR_Status TMR_LLRP_cmdPrepareROSpec(TMR_Reader *reader, uint16_t timeout, TMR_uint8List *antennaList,
            TMR_TagFilter *filter, TMR_TagProtocol protocol);
TMR_Status TMR_LLRP_cmdAddROSpec(TMR_Reader *reader, uint16_t readDuration, TMR_uint8List *antennaList,
                                   TMR_TagFilter *filter, TMR_TagProtocol protocol);
TMR_Status TMR_LLRP_cmdEnableROSpec(TMR_Reader *reader);
TMR_Status TMR_LLRP_cmdDisableROSpec(TMR_Reader *reader);
TMR_Status TMR_LLRP_cmdStartROSpec(TMR_Reader *reader, llrp_u32_t roSpecId);
TMR_Status TMR_LLRP_cmdStopROSpec(TMR_Reader *reader, bool receiveResponse);
TMR_Status TMR_LLRP_cmdDeleteAllROSpecs(TMR_Reader *reader, bool receiveResponse);
TMR_Status TMR_LLRP_parseMetadataFromMessage(TMR_Reader *reader, TMR_TagReadData *data, LLRP_tSTagReportData *msg);
TMR_Status TMR_LLRP_verifyReadOperation(TMR_Reader *reader, int32_t *tagCount);
TMR_Status TMR_LLRP_cmdStopReading(struct TMR_Reader *reader);
void TMR_LLRP_parseCustomStatsValues(LLRP_tSCustomStatsValue *customStats, TMR_Reader_StatsValues *statsValue);

TMR_Status TMR_LLRP_cmdGetVersionSerial(TMR_Reader *reader, TMR_String *version);
TMR_Status TMR_LLRP_cmdrebootReader(TMR_Reader *reader);
//TMR_Status TMR_LLRP_cmdResetHoptable(TMR_Reader *reader);

/* Reset Reader */
TMR_Status TMR_LLRP_stopActiveROSpecs(TMR_Reader *reader);

/* Reports, Notifications and Keepalives */
TMR_Status TMR_LLRP_setKeepAlive(TMR_Reader *reader);
TMR_Status TMR_LLRP_enableEventsAndReports(TMR_Reader *reader);
TMR_Status TMR_LLRP_setHoldEventsAndReportsStatus(TMR_Reader *reader, llrp_u1_t status);
TMR_Status TMR_LLRP_handleKeepAlive(TMR_Reader *reader, LLRP_tSMessage *pMsg);
TMR_Status TMR_LLRP_startBackgroundReceiver(TMR_Reader *reader);
TMR_Status TMR_LLRP_cmdSetEventNotificationSpec(TMR_Reader *reader, bool state);
TMR_Status TMR_LLRP_handleReaderEvents(TMR_Reader *reader, LLRP_tSMessage *pMsg);
TMR_Status TMR_LLRP_processReceivedMessage(TMR_Reader *reader, LLRP_tSMessage *pMsg);
void TMR_LLRP_setBackgroundReceiverState(TMR_Reader *reader, bool state);

/* Access Spec */
TMR_Status TMR_LLRP_cmdEnableAccessSpec(TMR_Reader *reader, llrp_u32_t accessSpecId);
TMR_Status TMR_LLRP_msgPrepareAccessCommand(TMR_Reader *reader, LLRP_tSAccessCommand *pAccessCommand,
                                                              TMR_TagFilter *filter, TMR_TagOp *tagop);
TMR_Status TMR_LLRP_cmdAddAccessSpec(TMR_Reader *reader, TMR_TagProtocol protocol, TMR_TagFilter *filter,
                                            llrp_u32_t roSpecId, TMR_TagOp *tagop, bool isStandalone);
TMR_Status TMR_LLRP_verifyOpSpecResultStatus(TMR_Reader *reader, LLRP_tSParameter *pParameter);
TMR_Status TMR_LLRP_cmdDeleteAllAccessSpecs(TMR_Reader *reader);

TMR_Status TMR_LLRP_parseCustomTagOpSpecResultType(LLRP_tEThingMagicCustomTagOpSpecResultType status);
void TMR_LLRP_parseTagOpSpecData(LLRP_tSParameter *pParameter, TMR_uint8List *data);

TMR_Status TMR_LLRP_cmdGetReport(TMR_Reader *reader);
#ifdef __cplusplus
}
#endif

#endif /* _LLRP_READER_IMP_H */
