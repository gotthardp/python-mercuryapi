/**
 *  @file tmr_param.c
 *  @brief Mercury API - parameter string implementation
 *  @author Nathan Williams
 *  @date 11/3/2009
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

#include <string.h>

#include "tm_reader.h"
#include "tmr_utils.h"

#ifdef TMR_ENABLE_PARAM_STRINGS

static const char *paramNames[1 + TMR_PARAM_MAX] = {
  "",  /* TMR_PARAM_NONE */
  "/reader/baudRate",  /* TMR_PARAM_BAUDRATE */
  "/reader/probeBaudRates",/* TMR_PARAM_PROBEBAUDRATES */
  "/reader/commandTimeout",  /* TMR_PARAM_COMMANDTIMEOUT */
  "/reader/transportTimeout",  /* TMR_PARAM_TRANSPORTTIMEOUT */
  "/reader/powerMode",  /* TMR_PARAM_POWERMODE */
#ifdef TMR_ENABLE_UHF
  "/reader/userMode",  /* TMR_PARAM_USERMODE */
  "/reader/antenna/checkPort",  /* TMR_PARAM_ANTENNA_CHECKPORT */
#endif /* TMR_ENABLE_UHF */
  "/reader/antenna/portList",  /* TMR_PARAM_ANTENNA_PORTLIST */
  "/reader/antenna/connectedPortList",  /* TMR_PARAM_ANTENNA_CONNECTEDPORTLIST */
  "/reader/antenna/portSwitchGpos",  /* TMR_PARAM_ANTENNA_PORTSWITCHGPOS */
#ifdef TMR_ENABLE_UHF
  "/reader/antenna/settlingTimeList",  /* TMR_PARAM_ANTENNA_SETTLINGTIMELIST */
  "/reader/antenna/returnLoss", /* TMR_PARAM_ANTENNA_RETURNLOSS */
#endif /* TMR_ENABLE_UHF */
  "/reader/antenna/txRxMap",  /* TMR_PARAM_ANTENNA_TXRXMAP */
#ifdef TMR_ENABLE_UHF
  "/reader/antenna/perAntennaTime", /* TMR_PARAM_PER_ANTENNA_TIME */
#endif /* TMR_ENABLE_UHF */
  "/reader/gpio/inputList",  /* TMR_PARAM_GPIO_INPUTLIST */
  "/reader/gpio/outputList",  /* TMR_PARAM_GPIO_OUTPUTLIST */
#ifdef TMR_ENABLE_UHF
  "/reader/gen2/accessPassword",  /* TMR_PARAM_GEN2_ACCESSPASSWORD */
  "/reader/gen2/q",  /* TMR_PARAM_GEN2_Q */
  "/reader/gen2/tagEncoding",  /* TMR_PARAM_GEN2_TAGENCODING*/
  "/reader/gen2/session",  /* TMR_PARAM_GEN2_SESSION */
  "/reader/gen2/target",  /* TMR_PARAM_GEN2_TARGET */
  "/reader/gen2/BLF",  /* TMR_PARAM_GEN2_BLF */
  "/reader/gen2/tari",  /* TMR_PARAM_GEN2_TARI */
  "/reader/gen2/writeMode",/*TMR_PARAM_GEN2_WRITEMODE*/
  "/reader/gen2/bap",  /* TMR_PARAM_GEN2_BAP */
  "/reader/gen2/initQ", /* TMR_PARAM_GEN2_INITIAL_Q */
  "/reader/gen2/sendSelect", /* TMR_PARAM_GEN2_SEND_SELECT */
  "/reader/gen2/protocolExtension",  /* TMR_PARAM_GEN2_PROTOCOLEXTENSION */
  "/reader/iso180006b/BLF",  /* TMR_PARAM_ISO18000_6B_LINKFREQUENCY */
  "/reader/iso180006b/modulationDepth", /* TMR_PARAM_ISO18000_6B_MODULATION_DEPTH */
  "/reader/iso180006b/delimiter", /* TMR_PARAM_ISO18000_6B_DELIMITER */
#endif /* TMR_ENABLE_UHF */
  "/reader/read/asyncOffTime",  /* TMR_PARAM_READ_ASYNCOFFTIME */
  "/reader/read/asyncOnTime",  /* TMR_PARAM_READ_ASYNCONTIME */
  "/reader/read/plan",  /* TMR_PARAM_READ_PLAN */
#ifdef TMR_ENABLE_UHF
  "/reader/radio/enablePowerSave", /* TMR_PARAM_RADIO_ENABLEPOWERSAVE */
#endif /* TMR_ENABLE_UHF */
  "/reader/radio/powerMax",  /* TMR_PARAM_RADIO_POWERMAX */
  "/reader/radio/powerMin",  /* TMR_PARAM_RADIO_POWERMIN */
  "/reader/radio/readPower",  /* TMR_PARAM_RADIO_READPOWER */
  "/reader/radio/writePower",  /* TMR_PARAM_RADIO_WRITEPOWER */
#ifdef TMR_ENABLE_UHF
  "/reader/radio/portReadPowerList",  /* TMR_PARAM_RADIO_PORTREADPOWERLIST */
  "/reader/radio/portWritePowerList",  /* TMR_PARAM_RADIO_PORTWRITEPOWERLIST */
#endif /* TMR_ENABLE_UHF */
  "/reader/radio/temperature",  /* TMR_PARAM_RADIO_TEMPERATURE */
#ifdef TMR_ENABLE_UHF
  "/reader/tagReadData/recordHighestRssi",  /* TMR_PARAM_TAGREADDATA_RECORDHIGHESTRSSI */
  "/reader/tagReadData/uniqueByAntenna",  /* TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA */
  "/reader/tagReadData/uniqueByData",  /* TMR_PARAM_TAGREADDATA_UNIQUEBYDATA */
#endif /* TMR_ENABLE_UHF */
  "/reader/tagop/antenna",  /* TMR_PARAM_TAGOP_ANTENNA */
  "/reader/tagop/protocol",  /* TMR_PARAM_TAGOP_PROTOCOL */
  "/reader/version/hardware",  /* TMR_PARAM_VERSION_HARDWARE */
  "/reader/version/serial", /* TMR_PARAM_VERSION_SERIAL */
  "/reader/version/model",  /* TMR_PARAM_VERSION_MODEL */
  "/reader/version/software",  /* TMR_PARAM_VERSION_SOFTWARE */
  "/reader/version/supportedProtocols",  /* TMR_PARAM_VERSION_SUPPORTEDPROTOCOLS */
#ifdef TMR_ENABLE_UHF
  "/reader/region/hopTable",  /* TMR_PARAM_REGION_HOPTABLE */
  "/reader/region/hopTime",  /* TMR_PARAM_REGION_HOPTIME */
#endif /* TMR_ENABLE_UHF */
  "/reader/region/id",  /* TMR_PARAM_REGION_ID */
  "/reader/region/supportedRegions",  /* TMR_PARAM_REGION_SUPPORTEDREGIONS */
#ifdef TMR_ENABLE_UHF
  "/reader/region/lbt/enable",  /* TMR_PARAM_REGION_LBT_ENABLE */
  "/reader/region/dwellTime/enable",  /* TMR_PARAM_REGION_DWELL_TIME_ENABLE */
  "/reader/region/lbtThreshold",  /* TMR_PARAM_REGION_LBT_THRESHOLD */
  "/reader/region/dwellTime",  /* TMR_PARAM_REGION_DWELL_TIME */
#endif /* TMR_ENABLE_UHF */
  "/reader/licenseKey",  /* TMR_PARAM_LICENSE_KEY */
  "/reader/userConfig",  /* TMR_PARAM_USER_CONFIG */
#ifdef TMR_ENABLE_UHF
  "/reader/radio/enableSJC", /* TMR_PARAM_RADIO_ENABLESJC */
  "/reader/statistics", /* TMR_PARAM_READER_STATISTICS */
#endif /* TMR_ENABLE_UHF */
  "/reader/stats", /* TMR_PARAM_READER_STATS */
  "/reader/uri", /* TMR_PARAM_URI */
  "/reader/version/productGroupID", /* TMR_PARAM_PRODUCT_GROUP_ID */
  "/reader/version/productGroup", /* TMR_PARAM_PRODUCT_GROUP */
  "/reader/version/productID", /* TMR_PARAM_PRODUCT_ID */
  "/reader/tagReadData/tagopSuccesses",  /* TMR_PARAM_TAGREADATA_TAGOPSUCCESSCOUNT */
  "/reader/tagReadData/tagopFailures", /* TMR_PARAM_TAGREADATA_TAGOPFAILURECOUNT */
  "/reader/tagReadData/enableReadFilter", /* TMR_PARAM_TAGREADDATA_ENABLEREADFILTER */
#ifdef TMR_ENABLE_UHF
  "/reader/status/antennaEnable", /* TMR_PARAM_STATUS_ENABLE_ANTENNAREPORT */
  "/reader/status/frequencyEnable", /* TMR_PARAM_STATUS_ENABLE_FREQUENCYREPORT */
  "/reader/status/temperatureEnable", /* TMR_PARAM_STATUS_ENABLE_TEMPERATUREREPORT */
  "/reader/tagReadData/readFilterTimeout", /* TMR_PARAM_TAGREADDATA_READFILTERTIMEOUT */
  "/reader/tagReadData/uniqueByProtocol", /* TMR_PARAM_TAGREADDATA_UNIQUEBYPROTOCOL */
  "/reader/description", /* TMR_PARAM_READER_DESCRIPTION */
  "/reader/hostname", /* TMR_PARAM_READER_HOSTNAME */
  "/reader/currentTime", /* TMR_PARAM_CURRENTTIME */
  "/reader/gen2/writeReplyTimeout", /* TMR_PARAM_READER_WRITE_REPLY_TIMEOUT */
  "/reader/gen2/writeEarlyExit", /* /reader/gen2/writeEarlyExit */
#endif /* TMR_ENABLE_UHF */
  "/reader/stats/enable", /* TMR_PARAM_READER_STATS_ENABLE */
  "/reader/trigger/read/Gpi", /* TMR_PARAM_TRIGGER_READ_GPI */
  "/reader/metadataflags", /* TMR_PARAM_METADATAFLAG */
#ifdef TMR_ENABLE_UHF
  "/reader/gen2/t4",  /* TMR_PARAM_GEN2_T4 */
  "/reader/licensedFeatures",  /* TMR_PARAM_LICENSED_FEATURES */
  "/reader/selectedProtocols",  /* TMR_PARAM_SELECTED_PROTOCOLS */
  "/reader/region/quantizationStep",  /* TMR_PARAM_REGION_QUANTIZATION_STEP */
  "/reader/region/minimumFrequency",  /* TMR_PARAM_REGION_MINIMUM_FREQUENCY */
#endif /* TMR_ENABLE_UHF */
  "/reader/manageLicenseKey",  /* TMR_PARAM_MANAGE_LICENSE_KEY */
#ifdef TMR_ENABLE_UHF
  "/reader/regulatory/mode", /* TMR_PARAM_REGULATORY_MODE */
  "/reader/regulatory/modulation", /* TMR_PARAM_REGULATORY_MODULATION */
  "/reader/regulatory/onTime", /* TMR_PARAM_REGULATORY_ONTIME */
  "/reader/regulatory/offTime", /* TMR_PARAM_REGULATORY_OFFTIME, */
  "/reader/regulatory/enable", /* TMR_PARAM_REGULATORY_ENABLE */
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_HF_LF
  "/reader/iso14443a/tagType", /* TMR_PARAM_ISO14443A_TAGTYPE */
  "/reader/iso14443b/tagType", /* TMR_PARAM_ISO14443B_TAGTYPE */
  "/reader/iso15693/tagType", /* TMR_PARAM_ISO15693_TAGTYPE */
  "/reader/lf125khz/tagType", /* TMR_PARAM_LF125KHZ_TAGTYPE */
  "/reader/lf134khz/tagType", /* TMR_PARAM_LF134KHZ_TAGTYPE */
  "/reader/iso14443a/supportedTagTypes", /* TMR_PARAM_ISO14443A_SUPPORTED_TAGTYPES */
  "/reader/iso14443b/supportedTagTypes", /* TMR_PARAM_ISO14443B_SUPPORTED_TAGTYPES */
  "/reader/iso15693/supportedTagTypes", /* TMR_PARAM_ISO15693_SUPPORTED_TAGTYPES */
  "/reader/lf125khz/supportedTagTypes", /* TMR_PARAM_LF125KHZ_SUPPORTED_TAGTYPES */
  "/reader/lf134khz/supportedTagTypes", /* TMR_PARAM_LF134KHZ_SUPPORTED_TAGTYPES */
  "/reader/iso14443a/supportedTagFeatures", /* TMR_PARAM_ISO14443A_SUPPORTED_TAG_FEATURES */
  "/reader/iso15693/supportedTagFeatures", /* TMR_PARAM_ISO15693_SUPPORTED_TAG_FEATURES */
  "/reader/lf125khz/supportedTagFeatures", /* TMR_PARAM_LF125KHZ_SUPPORTED_TAG_FEATURES */
  "/reader/lf125khz/secureRdFormat", /* TMR_PARAM_LF125KHZ_SECURE_RD_FORMAT */
  "/reader/radio/KeepRFOn", /* TMR_PARAM_RADIO_KEEP_RF_ON */
  "/reader/protocolList", /* TMR_PARAM_PROTOCOL_LIST */
#endif /* TMR_ENABLE_HF_LF */
};


TMR_Param
TMR_paramID(const char *name)
{
  int i;

  for (i = 1 ; i <= TMR_PARAM_MAX ; i++)
  {
    if (0 == strcasecmp(name, paramNames[i]))
    {
      return (TMR_Param)i;
    }
  }

  return TMR_PARAM_NONE;
}

const char *
TMR_paramName(TMR_Param key)
{

  if (key <= TMR_PARAM_MAX)
  {
    return paramNames[key];
  }
  return NULL;
}

#endif /* TMR_ENABLE_PARAM_STRINGS */
