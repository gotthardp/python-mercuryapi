/**
 *  @file serial_reader.c
 *  @brief Mercury API - serial reader high level implementation
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tm_reader.h"
#include "serial_reader_imp.h"
#include "tmr_utils.h"
#include "osdep.h"

#ifdef TMR_ENABLE_SERIAL_READER
bool clearTagBuffer = false;
bool isClearTagBufferCmdSent = false;
#ifdef TMR_ENABLE_UHF
extern bool isMultiSelectEnabled;
extern bool isEmbeddedTagopEnabled;
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_HF_LF
extern bool isAddrByteExtended;
#endif /* TMR_ENABLE_HF_LF */
uint64_t PrevSysTime = 0;
int delta;
extern int LastDSPMicro;

#ifdef TMR_ENABLE_UHF
/* This is used to enable the Gen2 secure readdata option */
extern bool isSecureAccessEnabled;
#endif /* TMR_ENABLE_UHF */

void 
fillStaticTxRxMapData(TMR_Reader *reader, TMR_SR_PortDetect *ports, uint8_t numPorts)
{
  uint8_t i, value;
 
  for (i = 0; i < numPorts; i++)
  {
    value = ports[i].port;

    reader->u.serialReader.staticTxRxMapData[i].antenna = value;
    reader->u.serialReader.staticTxRxMapData[i].txPort = value;
    reader->u.serialReader.staticTxRxMapData[i].rxPort = value;

    if ((!reader->tagOpParams.antenna) && ports[i].detected)
    {
      reader->tagOpParams.antenna = value;
    }
  }
}

static TMR_Status
initTxRxMapFromPorts(TMR_Reader *reader)
{
  TMR_Status ret;
  TMR_SR_PortDetect ports[TMR_SR_MAX_ANTENNA_PORTS];
  uint8_t i, numPorts;
  TMR_SR_SerialReader *sr;

  numPorts = numberof(ports);
  sr = &reader->u.serialReader;

  /* Need number of ports to set up Tx-Rx map */
  ret = TMR_SR_cmdAntennaDetect(reader, &numPorts, ports);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Modify TxRxMap according to reader product */
  switch (sr->productId)
  {
#ifdef TMR_ENABLE_UHF
    case 0x0002:
      /*
       * USB Reader -- Default map is okay
       * M5e-C only has 1 antenna port, anyway
       *
       * However, M6eMicro USB has 2 ports and no auto antenna detection,
       * so manually limit to first port for now.
       * TODO: Revisit if M6eMicro USB starts supporting port 2 in future.
       * port for now.
       */
      if ((TMR_SR_MODEL_MICRO == sr->versionInfo.hardware[0]) &&
        (sr->versionInfo.hardware[3] == TMR_SR_MODEL_M6E_MICRO_USB))
      {
        fillStaticTxRxMapData(reader, ports, 1);
        break;
      }
#endif /*TMR_ENABLE_UHF */
    default:
    {
      fillStaticTxRxMapData(reader, ports, numPorts);
      break;
    }
  }

  sr->staticTxRxMap.max = TMR_SR_MAX_ANTENNA_PORTS;
  sr->staticTxRxMap.len = numPorts;
  sr->staticTxRxMap.list = sr->staticTxRxMapData;
  sr->txRxMap = &sr->staticTxRxMap;

  /* Store default TxRx map before loading custom TxRx map */
  sr->staticTxRxDefMap.list = sr->staticTxRxDefMapData;
   
  for (i = 0; i < numPorts; i++)
  {
    sr->staticTxRxDefMap.list[i] = sr->staticTxRxMap.list[i];
  }
  sr->staticTxRxDefMap.max = sr->staticTxRxMap.max;
  sr->staticTxRxDefMap.len = sr->staticTxRxMap.len;
  sr->defaultTxRxMap = &sr->staticTxRxDefMap;

  return TMR_SUCCESS;
}

TMR_Status
TMR_SR_boot(TMR_Reader *reader, uint32_t currentBaudRate)
{
  TMR_Status ret;
  uint8_t program;
  int i = 0;
  TMR_TagProtocolList protocolList;
  TMR_TagProtocol protocols[TMR_MAX_PROTOCOLS];
  TMR_SR_SerialReader *sr = &reader->u.serialReader;
  ret = TMR_SUCCESS;

#ifdef TMR_ENABLE_UHF
    /*
   * Once out of bootloader, configure for wakeup preambles.
   * Bootloader doesn't support preambles, and some versions
   * fail to respond to normal commands afterwards.
   *
   * It is safe to avoid preambles up to this point because
   * TMR_SR_connect has already been talking to the module
   * to keep it awake.
   */
  if ((TMR_SR_MODEL_M6E == sr->versionInfo.hardware[0]) ||
      (TMR_SR_MODEL_MICRO == sr->versionInfo.hardware[0]) ||
      (TMR_SR_MODEL_M6E_I == sr->versionInfo.hardware[0]))
  {
    sr->supportsPreamble = true;
  }
#endif /* TMR_ENABLE_UHF */

  /* Get current program */
  ret = TMR_SR_cmdGetCurrentProgram(reader, &program);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* If bootloader, enter app */
  if ((program & 0x3) == 1)
  {
    ret = TMR_SR_cmdBootFirmware(reader);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
  }

  /* Initialize cached power mode value */
  /* Should read power mode as soon as possible.
   * Default mode assumes module is in deep sleep and
   * adds a lengthy "wake-up preamble" to every command.
   */
  if (sr->powerMode == TMR_SR_POWER_MODE_INVALID)
  {
    ret = TMR_paramGet(reader, TMR_PARAM_POWERMODE, &sr->powerMode);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
  }
#if TMR_ENABLE_CRC
#ifdef TMR_ENABLE_UHF
  /**
   * In case for M6E and it's variant  check for CRC
   **/
  if (TMR_SR_MODEL_M3E != reader->u.serialReader.versionInfo.hardware[0])
  {
    /**
     * Get the transport/BUS type
     **/
    ret = TMR_SR_cmdGetReaderConfiguration(reader, TMR_SR_CONFIGURATION_CURRENT_MSG_TRANSPORT, &reader->u.serialReader.transportType);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
    /**
     * In case of USB port disable the CRC
     **/
    if (TMR_SR_MSG_SOURCE_USB == reader->u.serialReader.transportType)
    {
      bool value = false;
      ret = TMR_SR_cmdSetReaderConfiguration(reader, TMR_SR_CONFIGURATION_SEND_CRC, &value);
      if (TMR_SUCCESS == ret)
      {
        reader->u.serialReader.crcEnabled = false;
      }
      else
      {
        /**
         * Not Fatal, Going ahead with CRC enabled
         **/ 
      }
    }
  }
#endif /* TMR_ENABLE_UHF */
#endif /* TMR_ENABLE_CRC */
  
  if(sr->isUserBaudRateSet)
  {
    if (sr->baudRate != currentBaudRate)
    {
      ret = TMR_paramSet(reader, TMR_PARAM_BAUDRATE, &sr->baudRate);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
    }
  }
  else
  {
    sr->baudRate = currentBaudRate;
  }

  reader->_storeSupportsResetStats = reader->featureFlags & TMR_READER_FEATURES_FLAG_READER_STATS;
  reader->pSupportsResetStats = &(reader->_storeSupportsResetStats);

  /* Initialize the paramPresent and paramConfirmed bits. */
  /* This block is expected to be collapsed by the compiler into a
   * small number of constant-value writes into the sr->paramPresent
   * array.
   */

  memset(reader->u.serialReader.paramPresent,0,
         sizeof(reader->u.serialReader.paramPresent));

  for(i = 1 ; i <= TMR_PARAM_MAX; i++)
  {
    BITSET(sr->paramPresent, i);
  }

  for (i = 0 ; i < TMR_PARAMWORDS; i++)
  {
    sr->paramConfirmed[i] = sr->paramPresent[i];
  }

  /* Get productGroupID early, so other params (e.g., txRxMap) can use it */
  {
    ret = TMR_SR_cmdGetReaderConfiguration(reader, TMR_SR_CONFIGURATION_PRODUCT_GROUP_ID, &sr->productId);
    if (TMR_SUCCESS != ret)
    {
#ifdef TMR_ENABLE_UHF
      if (TMR_ERROR_MSG_INVALID_PARAMETER_VALUE == ret)
      {
        /*
         * Modules with firmware older than wilder will throw 0x105 error, as it was not 
         * implemented. Catch this error but do not return.
         */
		sr->productId = 0xFFFF;
      }
      else
#endif /* TMR_ENABLE_UHF */
      {
        return ret;
      }
    }
#ifdef TMR_ENABLE_UHF
    if(TMR_SR_MODEL_M3E != sr->versionInfo.hardware[0])
    {
      /* 
       * If product is ruggedized reader, 
       * set reader's GPO pin which is used for antenna port switching
       */
      if (1 == sr->productId)
      {
        uint8_t pin = 1;
        ret = TMR_SR_cmdSetReaderConfiguration(reader, TMR_SR_CONFIGURATION_ANTENNA_CONTROL_GPIO, &pin);
        if (TMR_SUCCESS != ret)
        {
          return ret;
        }
      }
    }
#endif /* TMR_ENABLE_UHF */
  }
  /* Set region if user set the param */
  if (TMR_REGION_NONE != sr->regionId)
  {
    ret = TMR_SR_cmdSetRegion(reader, sr->regionId);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
  }

  ret = TMR_SR_cmdGetCurrentProtocol(reader, &sr->currentProtocol);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  protocolList.list = &protocols[0];
  protocolList.max = TMR_MAX_PROTOCOLS;

  ret = TMR_SR_cmdGetAvailableProtocols(reader, &protocolList);
#ifdef TMR_ENABLE_UHF
  if (TMR_TAG_PROTOCOL_NONE == sr->currentProtocol)
  {
    for(i = 0; i < protocolList.len; i++)
    {
      /*
       * Search whether GEN2 Protocol is supported by reader in its protocol list.
       * If so set it as current protocol, else leave protocol selection to user.
       */
      if(TMR_TAG_PROTOCOL_GEN2 == protocolList.list[i])
      {
        ret = TMR_SR_cmdSetProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
        if (TMR_SUCCESS != ret)
        {
          return ret;
        }
        sr->currentProtocol = TMR_TAG_PROTOCOL_GEN2;
        break;
      }
    }
  }
#endif /* TMR_ENABLE_UHF */

  reader->tagOpParams.protocol = sr->currentProtocol;
  reader->tagOpParams.antenna = 0;
  ret = initTxRxMapFromPorts(reader);

  { /* Do this only in case of M6e */
    /* Get reader's enable read filter setting */
    ret = TMR_SR_cmdGetReaderConfiguration(reader, TMR_SR_CONFIGURATION_ENABLE_READ_FILTER, &reader->u.serialReader.enableReadFiltering);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }

#ifdef TMR_ENABLE_UHF
    if(TMR_SR_MODEL_M3E != sr->versionInfo.hardware[0])
    {
      int32_t timeout = 0;
      /* Get reader's read filter entry timeout */
      ret = TMR_SR_cmdGetReaderConfiguration(reader, TMR_SR_CONFIGURATION_READ_FILTER_TIMEOUT, &timeout);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
      reader->u.serialReader.readFilterTimeout = (0 == timeout) ? TMR_DEFAULT_READ_FILTER_TIMEOUT : timeout;
    }
#endif /* TMR_ENABLE_UHF */
  }

  
  return ret;
}

/**
 * Probing the correct baudrate to communicate with the serial reader.
 *
 * @param reader The reader
 * @param currentBaudRate the baud rate at which module is communicating.
 */
TMR_Status
TMR_SR_cmdProbeBaudRate(TMR_Reader *reader, uint32_t *currentBaudRate)
{
  TMR_Status ret;
  TMR_SR_SerialReader *sr;
  TMR_SR_SerialTransport *transport;
  uint32_t rate = 0x00;
  int i,count = 2;
  uint32_t tTimeout = 0;
  bool isVerCmdRspRcvd = false;

  ret = TMR_SUCCESS;
  sr = &reader->u.serialReader;
  transport = &reader->u.serialReader.transport;

  /**
   * If user not set any trasportTimeout value.
   * Use the default value.
   */
  tTimeout = reader->u.serialReader.transportTimeout;
  if (false == reader->u.serialReader.usrTimeoutEnable)
  {
    reader->u.serialReader.transportTimeout = 100;
  }

  for (i = 0; (uint32_t)i < sr->probeBaudRates.len; i++)
  {
    if (i <= 1 && count)
    { 
      rate = sr->baudRate; /* Try this first */
      /* Module might be in deep sleep mode, if there is no response for the
      * first attempt, Try the same baudrate again. i = 0 and i = 1
      */
      count--;

      /* Hold i=0 (undo increment) until count has expired */

      i--;
    }
    else
    {
      rate = sr->probeBaudRates.list[i];
      if (rate == sr->baudRate)
        continue; /* We already tried this one */
    }

    if (NULL != transport->setBaudRate)
    {
      /**
      * some transport layer does not support baud rate settings.
      * for ex: TCP transport. In that case skip the baud rate
      * settings.
      */ 
      ret = transport->setBaudRate(transport, rate);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
    }

    ret = transport->flush(transport);
    if ((TMR_SUCCESS != ret) && (TMR_ERROR_UNIMPLEMENTED != ret))
    {
      return ret;
    }

    while(false == isVerCmdRspRcvd)
    {
      ret = TMR_SR_cmdVersion(reader, &(reader->u.serialReader.versionInfo));
      if (TMR_SUCCESS == ret)
      {
        /* Got a reply?  Then this is the right baud rate! */
        reader->u.serialReader.transportTimeout = tTimeout;
        isVerCmdRspRcvd = true;
        break;
      }
      else if(TMR_ERROR_BOOT_RESPONSE == ret)
      {
        continue;
      }
      /* Timeouts are okay -- they usually mean "wrong baud rate",
       * so just try the next one.  All other errors are real
       * and should be forwarded immediately. 
       */
      else if (TMR_ERROR_TIMEOUT != ret)
      {
        if (TMR_ERROR_COMM(ret))
        {
          reader->u.serialReader.transportTimeout = tTimeout;
          ret = verifySearchStatus(reader);
          if (TMR_SUCCESS == ret)
          {
            continue;
          }
          else
          {
            return ret;
          }
        } 
        else
        {
          return ret;
        }
      }
      break;
    }
    if (isVerCmdRspRcvd)
    {
      break;
    }
  }
  if (i == sr->probeBaudRates.len)
  {
    return TMR_ERROR_TIMEOUT;
  }

  /* copy the baud rate */
  *currentBaudRate = rate;  
  return ret;
}

TMR_Status
TMR_SR_connect(TMR_Reader *reader)
{
  TMR_Status ret;
  uint32_t rate;
  TMR_SR_SerialReader *sr;
  TMR_SR_SerialTransport *transport;
  TMR_TagProtocol protocol;

  ret = TMR_SUCCESS;
  sr = &reader->u.serialReader;
  transport = &reader->u.serialReader.transport;

  ret = transport->open(transport);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }
  rate = sr->probeBaudRates.list[0]; //this fixes the compilation errors in some compilers
  ret = TMR_SR_cmdProbeBaudRate(reader, &rate);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  reader->connected = true;

  if(TMR_SR_MODEL_M3E != sr->versionInfo.hardware[0])
  {
#ifdef TMR_ENABLE_UHF
    protocol = TMR_TAG_PROTOCOL_GEN2;
    reader->userMetadataFlag = (TMR_TRD_METADATA_FLAG_RSSI |
                                TMR_TRD_METADATA_FLAG_FREQUENCY |
                                TMR_TRD_METADATA_FLAG_PHASE |
                                TMR_TRD_METADATA_FLAG_GPIO_STATUS |
                                TMR_TRD_METADATA_FLAG_GEN2_Q |
                                TMR_TRD_METADATA_FLAG_GEN2_LF |
                                TMR_TRD_METADATA_FLAG_GEN2_TARGET |
                                TMR_TRD_METADATA_FLAG_READCOUNT |
                                TMR_TRD_METADATA_FLAG_ANTENNAID |
                                TMR_TRD_METADATA_FLAG_TIMESTAMP |
                                TMR_TRD_METADATA_FLAG_PROTOCOL |
                                TMR_TRD_METADATA_FLAG_DATA);

    reader->userStatsFlag = (TMR_READER_STATS_FLAG_RF_ON_TIME |
                             TMR_READER_STATS_FLAG_NOISE_FLOOR_SEARCH_RX_TX_WITH_TX_ON |
                             TMR_READER_STATS_FLAG_PROTOCOL |
                             TMR_READER_STATS_FLAG_ANTENNA_PORTS |
                             TMR_READER_STATS_FLAG_FREQUENCY |
                             TMR_READER_STATS_FLAG_CONNECTED_ANTENNAS |
                             TMR_READER_STATS_FLAG_TEMPERATURE);
#endif /* TMR_ENABLE_UHF */
  }
  else
  {
#ifdef TMR_ENABLE_HF_LF
    protocol = TMR_TAG_PROTOCOL_ISO14443A;
    reader->userMetadataFlag = (TMR_TRD_METADATA_FLAG_READCOUNT |
                                TMR_TRD_METADATA_FLAG_ANTENNAID |
                                TMR_TRD_METADATA_FLAG_TIMESTAMP |
                                TMR_TRD_METADATA_FLAG_PROTOCOL  |
                                TMR_TRD_METADATA_FLAG_DATA|
                                TMR_TRD_METADATA_FLAG_TAGTYPE);

    reader->userStatsFlag = (TMR_READER_STATS_FLAG_TEMPERATURE |
                             TMR_READER_STATS_FLAG_DC_VOLTAGE);
#endif /* TMR_ENABLE_HF_LF */
  }

  TMR_RP_init_simple(&reader->readParams.defaultReadPlan, 0, NULL, protocol, 1);
  reader->allValidMetadataFlags = reader->userMetadataFlag;

  /* At this point we will have the firmware version.
   * Check for available features as per firmware version.
   */
  checkForAvailableFeatures(reader);

  /* Boot */
  ret = TMR_SR_boot(reader, rate);

  if(ret !=TMR_SUCCESS)
  {
    if(ret == TMR_ERROR_AUTOREAD_ENABLED)
    {
      ret = verifySearchStatus(reader);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
      ret = TMR_SR_boot(reader, rate);
    }
  }

  return ret;
}

TMR_Status
TMR_SR_destroy(TMR_Reader *reader)
{
  TMR_SR_SerialTransport *transport;
  reader->hasContinuousReadStarted = false;

  transport = &reader->u.serialReader.transport;
#if TMR_ENABLE_CRC
#ifdef TMR_ENABLE_UHF
  /**
   * Enable the CRC, in case it is disabled
   **/
  if (false == reader->u.serialReader.crcEnabled)
  {
    reader->u.serialReader.crcEnabled = true;
    TMR_SR_cmdSetReaderConfiguration(reader, TMR_SR_CONFIGURATION_SEND_CRC, &reader->u.serialReader.crcEnabled);
  }
#endif /* TMR_ENABLE_UHF */
#endif /* TMR_ENABLE_CRC */
  transport->shutdown(transport);
  reader->connected = false;

#ifdef TMR_ENABLE_BACKGROUND_READS
  /* Cleanup background threads */
  cleanup_background_threads(reader);
#endif

  return TMR_SUCCESS;
}

static TMR_Status
autoDetectAntennaList(struct TMR_Reader *reader)
{
  TMR_Status ret;
  TMR_SR_PortDetect ports[TMR_SR_MAX_ANTENNA_PORTS];
  TMR_SR_PortPair searchList[TMR_SR_MAX_ANTENNA_PORTS];
  uint8_t i, listLen, numPorts;
  uint16_t j;
  TMR_AntennaMapList *map;
    
  ret = TMR_SUCCESS;
  map = reader->u.serialReader.txRxMap;

  /* 1. Detect current set of antennas */
  numPorts = TMR_SR_MAX_ANTENNA_PORTS;
  ret = TMR_SR_cmdAntennaDetect(reader, &numPorts, ports);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* 2. Set antenna list based on detected antennas (Might be clever
   * to cache this and not bother sending the set-list command
   * again, but it's more code and data space).
   */
  for (i = 0, listLen = 0; i < numPorts; i++)
  {
    if (ports[i].detected)
    {
      /* Ensure that the port exists in the map */
      for (j = 0; j < map->len; j++)
        if (ports[i].port == map->list[j].txPort)
        {
          searchList[listLen].txPort = map->list[j].txPort;
          searchList[listLen].rxPort = map->list[j].rxPort;
          listLen++;
          break;
        }
    }
  }
  if (0 == listLen) /* No ports auto-detected */
  {
    return TMR_ERROR_NO_ANTENNA;
  }
  ret = TMR_SR_cmdSetAntennaSearchList(reader, listLen, searchList);
  
  return ret;
}

static TMR_Status
setAntennaList(struct TMR_Reader *reader, TMR_uint8List *antennas)
{
  TMR_SR_PortPair searchList[64];
  uint16_t i, j, listLen;
  TMR_AntennaMapList *map;

  map = reader->u.serialReader.txRxMap;

  /** @todo cache the set list and don't reset it if it hasn't changed */
  listLen = 0;
  for (i = 0; i < antennas->len ; i++)
  {
    for (j = 0; j < map->len; j++)
    {
      if (antennas->list[i] == map->list[j].antenna)
      {
        searchList[listLen].txPort = map->list[j].txPort;
        searchList[listLen].rxPort = map->list[j].rxPort;
        listLen++;
        break;
      }
    }
    if (j == map->len)
    {
      return TMR_ERROR_INVALID_ANTENNA_CONFIG;
    }
  }
  return TMR_SR_cmdSetAntennaSearchList(reader,(uint8_t)listLen, searchList);
}
#ifdef TMR_ENABLE_UHF
/**
 * Validation for “Parameters (protocol, filter, tagop) are identical in all subplans
 * It return true if params are same across the entire set of read plans.
 */
bool
validateParams(TMR_ReadPlan *plan)
{
  TMR_ReadPlan *plan1, *plan2;
  uint8_t matchingPlanCount = 0;
  uint8_t i;
  bool status = false;

  /* Take 1st readplan as a reference for comparision. */
  plan1 = plan->u.multi.plans[0];

  for (i = 1; i < plan->u.multi.planCount; i++)
  {
    plan2 = plan->u.multi.plans[i];


    if(plan1->u.simple.protocol == plan2->u.simple.protocol)
    {
      if(( NULL == plan1->u.simple.tagop) && ( NULL == plan2->u.simple.tagop))
      {
        if(( NULL == plan1->u.simple.filter) && (NULL == plan2->u.simple.filter))
        {
          matchingPlanCount ++;
        }
        else
        {
          if(( NULL != plan1->u.simple.filter) && ( NULL != plan2->u.simple.filter))
          {
            if((plan1->u.simple.filter->type == plan2->u.simple.filter->type))
            {
              matchingPlanCount ++;
            }
          }
        }
      }
      if((( NULL != plan1->u.simple.tagop) && ( NULL != plan2->u.simple.tagop)) && (plan1->u.simple.tagop->type == plan2->u.simple.tagop->type))
      {
        if(( NULL == plan1->u.simple.filter) && (NULL == plan2->u.simple.filter))
        {
          matchingPlanCount ++;
        }
        else
        {
          if(( NULL != plan1->u.simple.filter) && ( NULL != plan2->u.simple.filter))
          {
            if((plan1->u.simple.filter->type == plan2->u.simple.filter->type))
            {
              matchingPlanCount ++;
            }
          }
        }
      }
    }
    else
    {
      status = false;
      break;
    }
  }

  /* At this point matchingPlanCount should be equal to total plan counts 
  * which means all plans are having same param value
  */
  if(matchingPlanCount == (plan->u.multi.planCount - 1))
  {
    status = true;
  }

  return status;
}

/** Recursively assemble a setAntennaReadTime command
 * @arg reader  Reader object
 * @arg msg  Serial message buffer
 * @arg pI  Current write position in serial read buffer (pass by pointer, so we can modify its value)
 * @arg plan  Read plan (recursively descended)
 * @arg ontime  Number of milliseconds alloted to this read plan
 */
TMR_Status
setAntennaReadTimeHelper(struct TMR_Reader *reader, uint8_t* msg, uint8_t* pI, TMR_ReadPlan* plan, uint32_t ontime, uint32_t offtime)
{
  uint32_t subOntime = 0, subOfftime = 0;
  int j, antCount, k;
  TMR_Status ret;
  TMR_AntennaMapList *map;
  
  map = reader->u.serialReader.txRxMap;
  switch (plan->type)
  {
    case TMR_READ_PLAN_TYPE_SIMPLE:
      {
        //Find out the exact number of valid antennas from the provided antenna list.
        for (j=0, antCount = 0; j<plan->u.simple.antennas.len; j++)
        {
          if(plan->u.simple.antennas.list[j])
          {
            antCount++;
          }
        }

        //Throw an error in case of invalid antenna.
        if (antCount != plan->u.simple.antennas.len)
        {
          return TMR_ERROR_INVALID_ANTENNA_CONFIG;
        }

        if(antCount)
        {
          //Divide Global asyncOn time by number of antennas.
          subOntime = ontime / antCount;

          //Divide Global asyncOff time by number of antennas.
          subOfftime = offtime / antCount;
        }

        // Embedding ontime and offtime for the antenna list in "per antenna ontime"(91 07) command.
        for (j=0; j<plan->u.simple.antennas.len; j++)
        {
          for (k=0; k<map->len; k++)
          {
            if ((plan->u.simple.antennas.list[j] == map->list[k].antenna))
            {
              SETU8 (msg, *pI, map->list[k].txPort);
              SETU16(msg, *pI, subOntime);

              //If subOfftime is non-zero, follow it immediately after ontime with 0x00 as an antenna number.
#if defined(TMR_ENABLE_BACKGROUND_READS)|| defined(SINGLE_THREAD_ASYNC_READ)
              if((reader->continuousReading) && (reader->readParams.asyncOffTime))
              {
                SETU8 (msg, *pI, 0x00);
                SETU16(msg, *pI, subOfftime);
              }
#endif
            }
          }
        }
        
        ret = TMR_SUCCESS;
        break;
      }
    case TMR_READ_PLAN_TYPE_MULTI:
      {
        for (j=0; j<plan->u.multi.planCount; j++)
        {
          TMR_ReadPlan *subplan;
          subplan = plan->u.multi.plans[j];
		  if(plan->weight != 0)
		  {
		    subOntime = (subplan->weight * ontime)/ plan->weight;
            subOfftime = (subplan->weight * offtime)/ plan->weight;
		  }
		  else
		  {
		    subOntime = ontime/ plan->u.multi.planCount;
            subOfftime = offtime/ plan->u.multi.planCount;
		  }

          ret = setAntennaReadTimeHelper(reader, msg, pI, subplan, subOntime, subOfftime);
          if (TMR_SUCCESS != ret)
          {
            return ret;
          }
        }
        ret = TMR_SUCCESS;
        break;
      }
    default:
      {
        ret = TMR_ERROR_INVALID;
        break;
      }
  }
  return ret;
}

TMR_Status
setAntennaReadTimeList(struct TMR_Reader *reader)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t i;
  TMR_ReadPlan* plan;
  uint32_t onTime;
  TMR_Status ret;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_ANTENNA_PORT);
  SETU8(msg, i, 7); /* antenna read time option */

  plan = reader->readParams.readPlan;
  if(reader->continuousReading)
  {
#if defined(TMR_ENABLE_BACKGROUND_READS)|| defined(SINGLE_THREAD_ASYNC_READ)
    uint32_t asyncOnTime,asyncOffTime;
    asyncOnTime = reader->readParams.asyncOnTime;
    asyncOffTime = reader->readParams.asyncOffTime;
    ret = setAntennaReadTimeHelper(reader, msg, &i, plan, asyncOnTime, asyncOffTime);
#endif
  }
  else
  {
    onTime = reader->readParams.onTime;
    ret = setAntennaReadTimeHelper(reader, msg, &i, plan, onTime, 0);
  }
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  return TMR_SR_sendCmd(reader, msg, i);
}
#endif /* TMR_ENABLE_UHF */

TMR_Status
setEnableReadFilter(TMR_Reader * reader)
{
  bool value;
  TMR_Status ret = TMR_SUCCESS;

  value = !(reader->continuousReading);

  if ((reader->u.serialReader.enableReadFiltering != value) &&
      (reader->u.serialReader.userEnableReadFiltering == false))
  {
    ret = TMR_SR_cmdSetReaderConfiguration(reader, TMR_SR_CONFIGURATION_ENABLE_READ_FILTER, &value);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
    reader->u.serialReader.enableReadFiltering = value;
  }
  return ret;
}

static TMR_Status
setProtocol(struct TMR_Reader *reader, TMR_TagProtocol protocol)
{
  TMR_Status ret;

  if(reader->u.serialReader.currentProtocol != protocol)
  {
    ret = TMR_SR_cmdSetProtocol(reader, protocol);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
    reader->u.serialReader.currentProtocol = protocol;
  }
  return TMR_SUCCESS;
}

/**
 * Compare antenna list in readplans list, return true if antenna
 * list are consistent across the entire set of read plans.
 **/
bool
compareAntennas(TMR_MultiReadPlan *multi)
{
  TMR_ReadPlan *plan1, *plan2;

  uint8_t allAntennasNull = 0;
  uint8_t matchingPlanCount = 0;
  uint8_t i;
  bool status = false;

  for (i = 0; i < multi->planCount; i++)
  {
    plan1 = multi->plans[0];
    plan2 = multi->plans[i];

    if ((0 != plan1->u.simple.antennas.len) && (0 != plan2->u.simple.antennas.len))
    {
      uint8_t j;

      if (plan1->u.simple.antennas.len == plan2->u.simple.antennas.len)
      {
        for (j = 0; j < plan1->u.simple.antennas.len; j++)
        {
          if (plan1->u.simple.antennas.list[j] != plan2->u.simple.antennas.list[j])
          {
            status = false;
            break;
          }
        }
        if (j == plan1->u.simple.antennas.len)
        {
          matchingPlanCount++;
        }
      }
    }
    else if ((0 == plan1->u.simple.antennas.len) && (0 == plan2->u.simple.antennas.len))
    {
      allAntennasNull++;
    }
    else
    {
      status = false;
      break;
    }
  }

  if ((matchingPlanCount == multi->planCount) || (allAntennasNull == multi->planCount))
  {
    status = true;
  }

  return status;
}

TMR_Status
prepForSearch(TMR_Reader *reader, TMR_ReadPlan *rp)
{
  TMR_Status ret = TMR_SUCCESS;
  switch (rp->type)
  {
    case TMR_READ_PLAN_TYPE_SIMPLE:
    {
      TMR_uint8List *antennaList;
      antennaList = &(rp->u.simple.antennas);
      if (reader->isAntennaSet)
      {
        if (antennaList->len == 0)
        {
#ifdef TMR_ENABLE_UHF
          if(!reader->isPerAntTimeSet)
#endif /* TMR_ENABLE_UHF */
          {
            ret = autoDetectAntennaList(reader);
          }
        }
        else
        {
          ret = setAntennaList(reader, antennaList);
        }
	  }
      break;
    }
    case TMR_READ_PLAN_TYPE_MULTI:
    {
#ifdef TMR_ENABLE_UHF
      if((validateParams(rp)) && (reader->featureFlags & TMR_READER_FEATURES_FLAG_ANTENNA_READ_TIME))
      {
        if (reader->isAntennaSet)
        {
          ret = setAntennaReadTimeList(reader);
        }
      }
      else if(compareAntennas(&rp->u.multi))
#else
      if(compareAntennas(&rp->u.multi))
#endif /* TMR_ENABLE_UHF */
      {
        TMR_uint8List *antennaList;
        antennaList = &(rp->u.multi.plans[0]->u.simple.antennas);
        if (reader->isAntennaSet)
        {
          if (antennaList->len == 0)
          {
            ret = autoDetectAntennaList(reader);
          }
          else
          {
            ret = setAntennaList(reader, antennaList);
          }
        }
      }
      else
      {
        reader->dutyCycle = false;
        ret = TMR_ERROR_UNSUPPORTED;
      }
        break;
    }
    default:
      ret = TMR_ERROR_INVALID;
      break;
  }
  return ret;
}

static TMR_Status
prepEmbReadTagMultiple(TMR_Reader *reader, uint8_t *msg, uint8_t *i,
            uint16_t timeout, TMR_SR_SearchFlag searchFlag,
            const TMR_TagFilter *filter, TMR_TagProtocol protocol,
            TMR_GEN2_Password accessPassword, uint8_t *lenbyte)
{
  TMR_Status ret;

  ret = TMR_SR_msgSetupReadTagMultiple(reader,
        msg, i, (uint16_t)timeout, searchFlag, filter, protocol, accessPassword);
  
  /* Embedded command count (Currently supports only one command)*/
  SETU8(msg, *i, 1);  
  *lenbyte = (*i)++;
  return ret;
}

static TMR_Status
TMR_SR_read_internal(struct TMR_Reader *reader, uint32_t timeoutMs,
                     int32_t *tagCount, TMR_ReadPlan *rp)
{
  TMR_Status ret = TMR_SUCCESS;
  TMR_SR_SerialReader *sr;
  TMR_SR_MultipleStatus multipleStatus = {0};
  uint32_t elapsed_tagop;
  uint32_t count, elapsed;
  uint32_t readTimeMs, starttimeLow, starttimeHigh;
  uint32_t prevElapsed = 0;
#ifdef TMR_ENABLE_BACKGROUND_READS
  uint32_t sleepTime;
  reader->tagFetchTime = 0;
#endif /* TMR_ENABLE_BACKGROUND_READS */
  sr = &reader->u.serialReader;

  /* The ontime passed in TMR_read() function through codelet is being collected in 'timeoutMs' variable.
   * Here assigning this value to onTime variable of readParams structure. Which will be ussed in setAntennaReadTimeList() function.*/
  reader->readParams.onTime = timeoutMs;
  if (TMR_READ_PLAN_TYPE_MULTI == rp->type)
  {
    int i;
    TMR_TagProtocolList p;
    TMR_TagProtocolList *protocolList = &p;
    TMR_TagFilter *filters[TMR_MAX_SERIAL_MULTIPROTOCOL_LENGTH];
    TMR_TagProtocol protocols[TMR_MAX_SERIAL_MULTIPROTOCOL_LENGTH];


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
    
    if (((0 < rp->u.multi.planCount) &&
         (rp->u.multi.plans[0]->type == TMR_READ_PLAN_TYPE_SIMPLE))
         || (true == reader->continuousReading))
    {
      ret = prepForSearch(reader, rp);
      reader->isAntennaSet = false;

      if (TMR_SUCCESS == ret)
      {
        /* Cache search timeout for later call to streaming receive */
        sr->searchTimeoutMs = timeoutMs;

        /**
         * take the time stamp only in case of sync read,
         * async read does not depend on this
         */
        if (!reader->continuousReading)
        {
          /* Cache the read time so it can be put in tag read data later */
          tm_gettime_consistent(&starttimeHigh, &starttimeLow);
          sr->readTimeHigh = starttimeHigh;
          sr->readTimeLow = starttimeLow;
          sr->lastSentTagTimestampHigh = starttimeHigh;
          sr->lastSentTagTimestampLow = starttimeLow;
        }

        ret = setEnableReadFilter(reader);
        if (TMR_SUCCESS != ret)
        {
          return ret;
        }

        ret = TMR_SR_cmdMultipleProtocolSearch(reader, TMR_SR_OPCODE_READ_TAG_ID_MULTIPLE,
                                                 protocolList, reader->userMetadataFlag,
                                                 TMR_SR_SEARCH_FLAG_CONFIGURED_LIST,
                                                 filters, (uint16_t)timeoutMs, &count);

        if (NULL != tagCount)
        {
          *tagCount += count;
        }
        return ret;
      }
      if (TMR_ERROR_UNSUPPORTED == ret)
      {
        /* If this read plan is not simple enough to pass to the module,
         * then drop down to the next clause, which will break it down
         * recursively.
         */
        reader->continuousReading = false;
      }
      /* But if this is an actual error, just throw it. */
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
    }
  }

  if (TMR_READ_PLAN_TYPE_SIMPLE == rp->type)
  {
    reader->triggerRead = rp->u.simple.triggerRead.enable;
#ifdef TMR_ENABLE_UHF
    reader->fastSearch = rp->u.simple.useFastSearch;
#endif /* TMR_ENABLE_UHF */

    //Update stopNTrigger status and number of tag count to read.
    reader->isStopNTags = rp->u.simple.stopOnCount.stopNTriggerStatus;
    reader->numberOfTagsToRead = rp->u.simple.stopOnCount.noOfTags;
  }
  else if (TMR_READ_PLAN_TYPE_MULTI == rp->type)
  {
    uint32_t subTimeout = 0, asyncOffTime = 0;
    uint8_t i;
#if defined(TMR_ENABLE_BACKGROUND_READS)|| defined(SINGLE_THREAD_ASYNC_READ)
    asyncOffTime = reader->readParams.asyncOffTime;
#endif

    if ((0 == rp->u.multi.totalWeight) && rp->u.multi.planCount)
    {
      subTimeout = timeoutMs / rp->u.multi.planCount;
      if(asyncOffTime)
      {
        reader->subOffTime = asyncOffTime / rp->u.multi.planCount;
     }
    }

    for (i = 0; i < rp->u.multi.planCount; i++)
    {
      if (rp->u.multi.totalWeight)
      {
        subTimeout = rp->u.multi.plans[i]->weight * timeoutMs 
          / rp->u.multi.totalWeight;
        if(asyncOffTime)
        {
          reader->subOffTime = rp->u.multi.plans[i]->weight * asyncOffTime 
          / rp->u.multi.totalWeight;
        }
      }

      reader->isAntennaSet = true;
      ret = TMR_SR_read_internal(reader, subTimeout, tagCount, 
        rp->u.multi.plans[i]);
      if (TMR_SUCCESS != ret && TMR_ERROR_NO_TAGS_FOUND != ret)
      {
        return ret;
      }
    }
    return ret;
  }
  else
  {
    return TMR_ERROR_INVALID;
  }

  {
    /* At this point we're guaranteed to have a simple read plan */
    ret = prepForSearch(reader, rp);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
  }
  reader->isAntennaSet = false;

#ifdef TMR_ENABLE_HF_LF
  if (!reader->isProtocolDynamicSwitching)
#endif /* TMR_ENABLE_HF_LF */
  {
    /* Set protocol to that specified by the read plan. */
    ret = setProtocol(reader, rp->u.simple.protocol);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
  }
#ifdef TMR_ENABLE_HF_LF
  else
  {
    if (!reader->continuousReading)
    {
      reader->isProtocolDynamicSwitching = false;
      return TMR_ERROR_UNSUPPORTED;
    }
  }
#endif /* TMR_ENABLE_HF_LF */

  ret = setEnableReadFilter(reader);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* Cache the read time so it can be put in tag read data later */
  tm_gettime_consistent(&starttimeHigh, &starttimeLow);
  sr->readTimeHigh = starttimeHigh;
  sr->readTimeLow = starttimeLow;
  sr->lastSentTagTimestampHigh = starttimeHigh;
  sr->lastSentTagTimestampLow = starttimeLow;

  /* Cache search timeout for later call to streaming receive */
  sr->searchTimeoutMs = timeoutMs;

  elapsed = tm_time_subtract(tmr_gettime_low(), starttimeLow);
  elapsed_tagop = elapsed;
  
  /**
   * Ignoring the elapsed time calculation in case of true
   * continuous reading,
   */
  if ((reader->continuousReading) && (0 != timeoutMs))
  {
    elapsed = timeoutMs - 1;
  }

  while (elapsed <= timeoutMs)
  {
    prevElapsed = elapsed;    /* Update previous read(22h) elapsed time value */
    readTimeMs = timeoutMs - elapsed;
    if (readTimeMs > 65535)
    {
      readTimeMs = 65535;
    }

    if (NULL == rp->u.simple.tagop)
    {
      if(reader->continuousReading)
      {
        TMR_TagProtocolList p;
        TMR_TagProtocolList *protocolList = &p;
        TMR_TagFilter *filters[TMR_MAX_SERIAL_MULTIPROTOCOL_LENGTH];
        TMR_TagProtocol protocols[TMR_MAX_SERIAL_MULTIPROTOCOL_LENGTH];
        
        protocolList->len = 1;
        protocolList->max = 1;
        protocolList->list = protocols;

        protocolList->list[0] = rp->u.simple.protocol;
        filters[0]= rp->u.simple.filter;

        ret = TMR_SR_cmdMultipleProtocolSearch(reader,
                            TMR_SR_OPCODE_READ_TAG_ID_MULTIPLE,
                            protocolList, reader->userMetadataFlag,
                            TMR_SR_SEARCH_FLAG_CONFIGURED_LIST,
                            filters,
                            (uint16_t)timeoutMs, &count);
      }
      else
      {
        ret = TMR_SR_cmdReadTagMultiple(reader,(uint16_t)readTimeMs,
                            TMR_SR_SEARCH_FLAG_CONFIGURED_LIST, 
                            rp->u.simple.filter, rp->u.simple.protocol, &count);
      }
    }
    else
    {
      uint8_t msg[256];
      uint8_t i, lenbyte;

      /* Since this is embedded tagop, removing elapsed time based on
       * continuous reading should not be done 
       */
      readTimeMs = timeoutMs - elapsed_tagop;
      if (readTimeMs > 65535)
      {
        readTimeMs = 65535;
      }

      i = 2;
     
      /**
       * add the tagoperation
       **/
      ret = TMR_SR_addTagOp(reader, rp->u.simple.tagop, rp, msg, &i, readTimeMs, &lenbyte);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      msg[lenbyte] = i - (lenbyte + 2); /* Install length of subcommand */
      msg[1] = i - 3; /* Install length */
      ret = TMR_SR_executeEmbeddedRead(reader, msg, (uint16_t)timeoutMs, &multipleStatus);
      count = multipleStatus.tagsFound;


      /* Update embedded tagop success/failure count */
      reader->u.serialReader.tagopSuccessCount += multipleStatus.successCount;
      reader->u.serialReader.tagopFailureCount += multipleStatus.failureCount;
    }
    if (TMR_ERROR_NO_TAGS_FOUND == ret)
    {
      count = 0;
      ret = TMR_SUCCESS;
    }
    else if ((TMR_ERROR_TM_ASSERT_FAILED == ret) ||
             (TMR_ERROR_TIMEOUT == ret))
	{
	  return ret;
	}
#ifdef TMR_ENABLE_BACKGROUND_READS
    else if((reader->fetchTagReads) && (TMR_ERROR_TAG_ID_BUFFER_FULL == ret))
    {
      TMR_Status ret1;
      notify_exception_listeners(reader, ret);

      /* Check for the tag count */
      ret1 = TMR_SR_cmdGetTagsRemaining(reader, (uint16_t *)&count);
      if (TMR_SUCCESS != ret1)
      {
        return ret1;
      }
    }
#endif /* TMR_ENABLE_BACKGROUND_READS */
    else if (TMR_SUCCESS != ret)
    {
      uint16_t remainingTagsCount;
      TMR_Status ret1;
      reader->isStopNTags = false;

      /* Check for the tag count (in case of module error)*/
      ret1 = TMR_SR_cmdGetTagsRemaining(reader, &remainingTagsCount);
      if (TMR_SUCCESS != ret1)
      {
        return ret1;
      }
      sr->tagsRemaining += remainingTagsCount;
      if (NULL != tagCount)
      {
        *tagCount += remainingTagsCount;
      }
      return ret;
    }

    sr->tagsRemaining += count;
    if (NULL != tagCount)
    {
      *tagCount += count;
    }

    if (reader->continuousReading)
    {
      sr->tagsRemaining = 1;
      break;
    }
    else if (reader->isStopNTags)
    {
      /** 
       * No need to loop back for stop N tags
       **/
#ifdef TMR_ENABLE_UHF
      isMultiSelectEnabled = false;
      isEmbeddedTagopEnabled = false;
#endif /* TMR_ENABLE_UHF */
      break;
    }
    else
    {  
      elapsed = tm_time_subtract(tmr_gettime_low(), starttimeLow);
      elapsed += prevElapsed;
    }
#ifdef TMR_ENABLE_BACKGROUND_READS
    if(reader->fetchTagReads)
    {
      uint32_t onTime, offTime;
      uint64_t end, now, difftime;

      TMR_paramGet(reader, TMR_PARAM_READ_ASYNCONTIME, &onTime);
      TMR_paramGet(reader, TMR_PARAM_READ_ASYNCOFFTIME, &offTime);

      /**
       * Set an indication that the reading is started
       **/
      reader->readState = TMR_READ_STATE_ACTIVE;
      pthread_cond_broadcast(&reader->readCond);

      reader->backgroundRunning = true;
      pthread_mutex_unlock(&reader->backgroundLock);

      /** 
       * On M5e and its variants, streaming is not supported
       * So still, retain the pseudo-async mechanism
       * Also, when asyncOffTime is non-zero the API should fallback to 
       * pseudo async mode.
       */

      end = tmr_gettime();

      while (TMR_SUCCESS == TMR_SR_hasMoreTags(reader))
      {
        TMR_TagReadData trd;
        TMR_ReadListenerBlock *rlb;

        TMR_TRD_init(&trd);

        ret = TMR_SR_getNextTag(reader, &trd);
        if (TMR_SUCCESS != ret)
        {
          pthread_mutex_lock(&reader->backgroundLock);
          reader->backgroundEnabled = false;
          pthread_mutex_unlock(&reader->backgroundLock);
          notify_exception_listeners(reader, ret);
          break;
        }

        pthread_mutex_lock(&reader->listenerLock);
        rlb = reader->readListeners;
        while (rlb)
        { 
          rlb->listener(reader, &trd, rlb->cookie);
          rlb = rlb->next;
        }
        pthread_mutex_unlock(&reader->listenerLock);
      }

      /* Calculate and accumulate time spent in fetching tags */
      now = tmr_gettime();
      difftime = now - end;
      reader->tagFetchTime += difftime;
      clearTagBuffer = true;
    }

    if(clearTagBuffer)
    {
      /* Clear Tag buffer */ 
      ret = TMR_SR_cmdClearTagBuffer(reader);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
      clearTagBuffer = false;
      isClearTagBufferCmdSent = true;
      tm_gettime_consistent(&starttimeHigh, &starttimeLow);
    }
#endif
#ifdef TMR_ENABLE_UHF
    isMultiSelectEnabled = false;
    isEmbeddedTagopEnabled = false;
#endif /* TMR_ENABLE_UHF */
  }
#ifdef TMR_ENABLE_BACKGROUND_READS
  if((reader->fetchTagReads) && (reader->subOffTime))
  {
    reader->isOffTimeAdded = true;
    /* Credit tag-fetching overhead towards total offTime */
    sleepTime = (reader->subOffTime) - (uint32_t)reader->tagFetchTime;

    /* Wait for the asyncOffTime duration to pass */
    if(sleepTime > 0)
    {
      tmr_sleep(sleepTime);
    }
  }
#endif /* TMR_ENABLE_BACKGROUND_READS */
  return ret;
}

TMR_Status
TMR_SR_read(struct TMR_Reader *reader, uint32_t timeoutMs, int32_t *tagCount)
{
  TMR_Status ret;
  TMR_ReadPlan *rp;

  if(timeoutMs > TMR_MAX_VALUE)
  {
    return TMR_ERROR_INVALID_VALUE;
  }

  if ((!reader->continuousReading) && (!isClearTagBufferCmdSent))
  {
    /**
    * In case of sync read only
    * clear tag buffer
    */
    ret = TMR_SR_cmdClearTagBuffer(reader);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
  }

  reader->u.serialReader.tagsRemaining = 0;

#ifdef TMR_ENABLE_BACKGROUND_READS
  if (false == reader->backgroundEnabled)
  /**
   * if TMR_ENABLE_BACKGROUND_READS is not defined, then
   * only sync read is possible. (Continuous and pseudo async reads
   * are not available)
   **/
#endif
  {
    /* If sync read, then reset tagop result count here */
    reader->u.serialReader.tagopSuccessCount = 0;
    reader->u.serialReader.tagopFailureCount = 0;
  }
  rp = reader->readParams.readPlan;

  if (tagCount)
  {
    *tagCount = 0;
  }
  
  ret = TMR_SR_read_internal(reader, timeoutMs, tagCount, rp);
  if (ret != TMR_SUCCESS)
  {
	  return ret;
  }
  if (reader->continuousReading)
	reader->hasContinuousReadStarted = true;
  return ret;
}

TMR_Status
verifySearchStatus(TMR_Reader *reader)
{
  TMR_SR_SerialReader *sr;
  TMR_Status ret;
  uint8_t *msg;
  uint32_t timeoutMs;

  sr = &reader->u.serialReader;
  msg = sr->bufResponse;

  timeoutMs = sr->searchTimeoutMs ? sr->searchTimeoutMs : reader->u.serialReader.commandTimeout;

#ifdef TMR_ENABLE_UHF
  reader->u.serialReader.crcEnabled = true;
#endif /* TMR_ENABLE_UHF */
  TMR_SR_cmdStopReading(reader);
  
  while(true)
  {
    ret = TMR_SR_receiveMessage(reader, msg, TMR_SR_OPCODE_READ_TAG_ID_MULTIPLE, timeoutMs);

    if(TMR_SUCCESS == ret || TMR_ERROR_DEVICE_RESET == ret)
    {
      if ((0x2F == msg[2]) && (0x02 == msg[5]))
      {
#if TMR_ENABLE_CRC
#ifdef TMR_ENABLE_UHF
        bool crcEnable;
        /**
         * 0x2F with type 0x02 means stop continuous reading.
         * Module has already pushed all the tagreads before
         * sending this response. i.e., reading is finished
         **/
		if (TMR_SR_MSG_SOURCE_USB == reader->u.serialReader.transportType)
		{
		  crcEnable = false;
		  ret = TMR_SR_cmdSetReaderConfiguration(reader, TMR_SR_CONFIGURATION_SEND_CRC, &crcEnable);
		  if (TMR_SUCCESS == ret)
			reader->u.serialReader.crcEnabled = false;
		  else
			reader->u.serialReader.crcEnabled = true;
		}
		else
		{
			reader->u.serialReader.crcEnabled = true;
		}
#endif /* TMR_ENABLE_UHF */
#endif /* TMR_ENABLE_CRC */
        return TMR_SUCCESS;
      }
      else if ((0x2F == msg[2]) && (0x01 == msg[3] && (0x00 == msg[4])))
      {
        /**
         * 0x2F with error status can also be treated as response for
         * stop continuous reading.
         **/
        return TMR_SUCCESS;
      }
    }
    else
    {
      /**
       * We might get comm errors here, either CRC or TIMEOUT errors
       * What should we do here??
       **/
      if (TMR_ERROR_CRC_ERROR == ret)
      {
        /**
         * When the module is pushing all tags out, just don't bother about
         * CRC and keep waiting for response for stopReading
         **/
        continue;
      }
      if (TMR_ERROR_TIMEOUT == ret)
      {
        return ret;
      }
    }
  }
}

TMR_Status
TMR_SR_hasMoreTags(struct TMR_Reader *reader)
{
  TMR_SR_SerialReader* sr;
  TMR_Status ret;

  sr = &reader->u.serialReader;

//#ifdef TMR_ENABLE_BACKGROUND_READS
  if ((reader->continuousReading) && (0 == sr->tagsRemainingInBuffer))
  {
    uint8_t *msg;
    uint32_t timeoutMs;
    uint8_t response_type_pos;
    
    msg = sr->bufResponse;
    timeoutMs = sr->searchTimeoutMs;

    ret = TMR_SR_receiveMessage(reader, msg, TMR_SR_OPCODE_READ_TAG_ID_MULTIPLE, timeoutMs);
#ifdef TMR_ENABLE_UHF
    if (TMR_ERROR_TAG_ID_BUFFER_AUTH_REQUEST == ret)
    {
      /* Tag password needed to complete tagop.
       * Parse TagReadData and pass to password-generating callback,
       * which will return the appropriate authentication. */
      uint16_t flags = 0;
      uint8_t bufptr;
      TMR_TagReadData trd;
      TMR_TagAuthentication tauth;

      TMR_TRD_init(&trd);

      /* In case of streaming, the metadata flag starts at position 9 only if 0x88 option field is present, otherwise it starts at position 8 */
      bufptr = isMultiSelectEnabled ? 9 : 8;
      flags = GETU16AT(msg, bufptr);
      bufptr += 2;
      bufptr++;  /* Skip tag count (always = 1) */
      TMR_SR_parseMetadataFromMessage(reader, &trd, flags, &bufptr, msg);
      TMR_SR_postprocessReaderSpecificMetadata(&trd, sr);
      trd.reader = reader;

#if !defined(BARE_METAL) && defined(TMR_ENABLE_BACKGROUND_READS)
      // printf("Parsed 0604 TagReadData: readCount=%d rssi=%d ant=%d, freq=%d, t_hi=%X, t_lo=%X\n", 
      //trd.readCount, trd.rssi, trd.antenna, trd.frequency, trd.timestampHigh, trd.timestampLow, trd.phase);
      {
        notify_authreq_listeners(reader, &trd, &tauth);		
      }
#endif

      /* TODO: Factor out password generation into callback */
	  reader->hasContinuousReadStarted = false;
      ret = TMR_SR_cmdAuthReqResponse(reader, &tauth);
      reader->hasContinuousReadStarted = true;
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
      else
      {
        return TMR_ERROR_TAG_ID_BUFFER_AUTH_REQUEST;
      }
    }
#endif /* TMR_ENABLE_UHF */

    if ((TMR_SUCCESS != ret) && (TMR_ERROR_TAG_ID_BUFFER_FULL != ret))
    {
      if((TMR_ERROR_NO_TAGS_FOUND != ret) || ((TMR_ERROR_NO_TAGS_FOUND == ret) && ((0 == msg[1]) || (0x88 == (msg[5] & 0x88))))
#ifdef TMR_ENABLE_HF_LF
      || (TMR_SR_MODEL_M3E == reader->u.serialReader.versionInfo.hardware[0])
#endif /* TMR_ENABLE_HF_LF */
        )
      {
        if (msg[5] != 0x04 || msg[2] != 0x2f)
        {
          reader->u.serialReader.isBasetimeUpdated = false;

          /** If multi select option is enabled, timestamp is also sent in the response
           *  In case of streaming after every async ON cycle, module sends the tag not found response as mentioned below
           *  ff 0b 22 04 00 88 10 00 1b 00 10 01 00 00 00 fd. 
           *  <FF> <Length> <Opcode> <Status(2 bytes)> <Multi select option (1 byte)> <Metadata flags enable option(1 byte)> <Search flags(2 bytes)>
           *  <Metadata flags (2 bytes - Timestamp)> <Response type(1 byte)> <Timestamp metadata (4 bytes)>
           *  In this case fetch the 4 bytes of Timestamp metadata flag and update the 
           *  basetimestamp.
           */
          if(0x88 == (msg[5] & 0x88))
          {
            sr->elapsedTime = GETU32AT(msg, 12);
          }
#ifdef TMR_ENABLE_HF_LF
          else if (TMR_SR_MODEL_M3E == reader->u.serialReader.versionInfo.hardware[0])
          {
            sr->elapsedTime = GETU32AT(msg, 11);
          }
#endif /* TMR_ENABLE_HF_LF */
          return ret;
        }
      }
    }

    if ((TMR_SUCCESS == ret) && (0 == msg[1]))
    {
      /**
       * In case of streaming and ISO protocol after every search cycle
       * module sends the response for embedded operation status as
       * FF 00 22 00 00.
       * In this case return back with 0x400 error, because success response will deceive
       * the read thread to process it. For GEN2 case we got the response with 0x400 status.
       **/
      return TMR_ERROR_NO_TAGS_FOUND;
    }
    if (((0x2F == msg[2]) || (0x22 == msg[2])) && (TMR_ERROR_TAG_ID_BUFFER_FULL == ret))
    {
      return ret;
    }

    if (0x2F == msg[2])
    {
      if (0x02 == msg[5])
      {
        /**
         * 0x2F with type 0x02 means stop continuous reading.
         * Module has already pushed all the tagreads before
         * sending this response. i.e., reading is finished
         **/
        reader->finishedReading = true;
        reader->u.serialReader.isBasetimeUpdated = false;
        return TMR_ERROR_END_OF_READING;
      }
	  else if (0x03 == msg[5])
      {
      /**
         * 0x2F with type 0x03 means left over client Auth message/response, ignore it.
         **/		
        return TMR_ERROR_TAG_ID_BUFFER_AUTH_REQUEST;		
      }
	  else if (0x04 == msg[5])
	  {
		  memcpy(&reader->paramMessage[0], msg, (msg[1] + 5) * sizeof(uint8_t));
		  reader->paramWait = false;
	  }
      /**
       * Control comes here in case of the response received
       * for start continuous reading command. (0x2F with type 0x01)
       **/
      return TMR_ERROR_NO_TAGS;
    }
    else if (0x22 == msg[2])
    {
      if((0x07 == msg[1]) &&                   //Length is 7 byte.
         (0x00 == msg[3]) && (0x00 == msg[4])  //Success response.
        )
      {
        uint8_t option      = GETU8AT(msg, 5);
        uint16_t searchFlag = GETU16AT(msg, 6);

        //Check if Stop N tag feature is enabled.
        if((0x01 & option) &&  //TM Option 1, for continuous reading.
          (reader->isStopNTags && (searchFlag & TMR_SR_SEARCH_FLAG_RETURN_ON_N_TAGS))
          )
        {
          //Retreive total tag count read.
          uint32_t tagCount = GETU32AT(msg, 8);

          //Check if total requested tag count is matching with read tag count.
          if(tagCount >= reader->numberOfTagsToRead)
          {
            /* Can't use "reader->finishedReading" flag here to check as it will be 
             * updated only after receiving stop read response.
             *
             * Checking "hasContinuousReadStarted" & "reader->searchStatus" flags will 
             * work here as these flags are getting turned-on for read and turned-off when TMR_StopReading() is called.
             *
             * Do not send stop read again if sent once for a read command.
             */
#ifdef TMR_ENABLE_BACKGROUND_READS
            if(reader->hasContinuousReadStarted && reader->searchStatus)
#endif  /* TMR_ENABLE_BACKGROUND_READS */
            {
              //Total requested tags are read. Send stop read command to the module.
              TMR_SR_cmdStopReading(reader);

#ifdef TMR_ENABLE_BACKGROUND_READS
              //Disable flags.
              reader->hasContinuousReadStarted = false;
              reader->searchStatus             = false;
#endif  /* TMR_ENABLE_BACKGROUND_READS */
            }
          }
        }

        //Signal that we are done with tag reading.
        return TMR_ERROR_NO_TAGS;
      }
    }
    else if (msg[1] < 6)
    { /* Need at least enough bytes to get to Response Type field */
      return TMR_ERROR_PARSE;
    }

    {
      uint8_t idx = 0;
#ifdef TMR_ENABLE_UHF
      if ((isMultiSelectEnabled) || (reader->isReadAfterWrite))
      {
        idx += 1;
      }
#endif /* TMR_ENABLE_UHF */
      response_type_pos = (0x10 == (msg[5 + idx] & 0x10)) ? (10 + idx) : (8 + idx);
    }

    switch (msg[response_type_pos])
    {
    case 0x02:
      /* Handle status stream responses */
      reader->isStatusResponse = true;
      sr->bufPointer = 9;
      return TMR_SUCCESS;
    case 0x01:
      /* Stream continues after this message */
      reader->isStatusResponse = false;
      sr->tagsRemainingInBuffer = 1;
      sr->bufPointer = 11;
      return TMR_SUCCESS;
    case 0x00:
      /* while fixing bug#4190, Missed updating base timestamp for embedded read. 
      Because of this time stamps are falling back.To fix this issue updated base time stamp. */
      reader->u.serialReader.isBasetimeUpdated = false;
      /* Stream ends with this message */
      sr->tagsRemaining = 0;
#ifdef TMR_ENABLE_UHF
      if (sr->oldQ.type != TMR_SR_GEN2_Q_INVALID)
      {
        ret = TMR_paramSet(reader, TMR_PARAM_GEN2_Q, &(sr->oldQ));
        if (TMR_SUCCESS != ret)
        {
          return ret;
        }
        sr->oldQ.type = TMR_SR_GEN2_Q_INVALID;
      }
#endif /* TMR_ENABLE_UHF */
      if (NULL != reader->readParams.readPlan->u.simple.tagop)
      {
        response_type_pos += 7;
        sr->tagopSuccessCount += GETU16(msg, response_type_pos);
        sr->tagopFailureCount += GETU16(msg, response_type_pos);
      }
      if (TMR_SUCCESS == ret)
      { /* If things look good so far, signal that we are done with tags */
        return TMR_ERROR_NO_TAGS;
      }
      /* otherwise feed the error back (should only be TMR_ERROR_TAG_ID_BUFFER_FULL) */
      return ret;
    default:
      /* Unknown response type */
      return TMR_ERROR_PARSE;
    }
  }
  else
//#endif
  {
    /**
     * TMR_SR_hasMoreTags control comes here only in case of sync reading
     * And in case of pseudo async reading when TMR_ENABLE_BACKGROUND_READS
     * is defined.
     **/
    ret = (sr->tagsRemaining > 0) ? TMR_SUCCESS : TMR_ERROR_NO_TAGS;
    return ret;
  }
}

TMR_Status
TMR_SR_getNextTag(struct TMR_Reader *reader, TMR_TagReadData *read)
{
  TMR_SR_SerialReader *sr;
  TMR_Status ret;
  uint8_t *msg;
  uint8_t i;
  uint16_t flags = 0;
#if !((!defined(TMR_ENABLE_UHF)) || defined(BARE_METAL))
  uint32_t timeoutMs;
  uint8_t subResponseLen = 0;
  uint8_t crclen = 2 ;
  uint8_t epclen = 0;
#endif /* TMR_ENABLE_HF_LF || BARE_METAL */

  sr = &reader->u.serialReader;
#if !((!defined(TMR_ENABLE_UHF)) || defined(BARE_METAL))
  timeoutMs = sr->searchTimeoutMs;
#endif /* TMR_ENABLE_HF_LF || BARE_METAL */

  {
    msg = sr->bufResponse;

    if (sr->tagsRemaining == 0)
    {
      return TMR_ERROR_NO_TAGS;
    }

    if (sr->tagsRemainingInBuffer == 0)
    {
      /* Fetch the next set of tags from the reader */
      if (reader->continuousReading)
      {
        ret = TMR_SR_hasMoreTags(reader);
        if (TMR_SUCCESS != ret)
        {
          return ret;
        }
      }
      else
      {
        if (reader->u.serialReader.opCode == TMR_SR_OPCODE_READ_TAG_ID_MULTIPLE)
        {
          i = 2;
          SETU8(msg, i, TMR_SR_OPCODE_GET_TAG_ID_BUFFER);
          SETU16(msg, i, reader->userMetadataFlag);
          SETU8(msg, i, 0); /* read options */

          ret = TMR_SR_sendCmd(reader, msg, i);
          if (TMR_SUCCESS != ret)
          {
            return ret;
          }
          sr->tagsRemainingInBuffer = msg[8];
          sr->bufPointer = 9;
        }
#if !((!defined(TMR_ENABLE_UHF)) || defined(BARE_METAL))
        else if (reader->u.serialReader.opCode == TMR_SR_OPCODE_READ_TAG_ID_SINGLE)
        {
          TMR_SR_receiveMessage(reader, msg, reader->u.serialReader.opCode, timeoutMs);
          sr->tagsRemainingInBuffer = (uint8_t)GETU32AT(msg , 9);
          sr->tagsRemaining = sr->tagsRemainingInBuffer;
          sr->bufPointer = 13 ;
        }
#endif /* TMR_ENABLE_HF_LF || BARE_METAL */
        else
        {
           return TMR_ERROR_INVALID_OPCODE; 
        }
      }
    }

    i = sr->bufPointer;
    if (reader->u.serialReader.opCode == TMR_SR_OPCODE_READ_TAG_ID_MULTIPLE)
    {
      uint8_t idx = 0;

#ifdef SINGLE_THREAD_ASYNC_READ
#ifdef TMR_ENABLE_UHF
      if(isMultiSelectEnabled)
      {
        idx += 1;
        i++;
      }
#endif /* TMR_ENABLE_UHF */
#endif /* SINGLE_THREAD_ASYNC_READ */
      flags = GETU16AT(msg, reader->continuousReading ? (8 + idx) : (5 + idx));

      TMR_SR_parseMetadataFromMessage(reader, read, flags, &i, msg);
    }
#if !((!defined(TMR_ENABLE_UHF)) || defined(BARE_METAL))
    if (reader->u.serialReader.opCode == TMR_SR_OPCODE_READ_TAG_ID_SINGLE)
    {
      flags = GETU16AT(msg, i + 6);
      subResponseLen = msg[i+1];
      i += 7;
      TMR_SR_parseMetadataOnly(reader, read, flags, &i, msg);
      epclen = subResponseLen + 4 - (i - sr->bufPointer) - crclen;
      read->tag.epcByteCount=epclen;
      memcpy(&(read->tag.epc), &msg[i], read->tag.epcByteCount);
      i+=epclen;
      read->tag.crc = GETU16(msg, i);
    }
#endif /* TMR_ENABLE_HF_LF || BARE_METAL */
    sr->bufPointer = i;
    
    
    TMR_SR_postprocessReaderSpecificMetadata(read, sr);

    sr->tagsRemainingInBuffer--;

    if (false == reader->continuousReading)
    {
      sr->tagsRemaining--;
#ifdef TMR_ENABLE_UHF
      if(sr->tagsRemaining == 0)
      {
        sr->gen2AllMemoryBankEnabled = false;
      }
#endif /* TMR_ENABLE_UHF */
    }
    read->reader = reader;

    return TMR_SUCCESS;
  }
}

#ifdef TMR_ENABLE_UHF
TMR_Status 
TMR_SR_writeTag(struct TMR_Reader *reader, const TMR_TagFilter *filter,
                const TMR_TagData *data)
{
  TMR_Status ret;
  TMR_SR_SerialReader *sr;

  sr = &reader->u.serialReader;

  ret = setProtocol(reader, reader->tagOpParams.protocol);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  if (TMR_TAG_PROTOCOL_GEN2 == reader->tagOpParams.protocol)
  {  
    return TMR_SR_cmdWriteGen2TagEpc(reader, filter, sr->gen2AccessPassword, (uint16_t)(sr->commandTimeout), 
                                 data->epcByteCount, data->epc, 0);
  }
  else
  {
    return TMR_ERROR_UNIMPLEMENTED;
  }
}


TMR_Status
TMR_SR_readTagMemWords(TMR_Reader *reader, const TMR_TagFilter *target, 
                       uint32_t bank, uint32_t wordAddress, 
                       uint16_t wordCount, uint16_t data[])
{
  TMR_Status ret;

  ret = TMR_SR_readTagMemBytes(reader, target, bank, wordAddress * 2,
                               wordCount * 2, (uint8_t *)data);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

#ifndef TMR_BIG_ENDIAN_HOST
  {
    uint16_t i;
    uint8_t *data8;
    /* We used the uint16_t data as temporary storage for the values read,
       but we need to adjust for possible endianness differences.
       This is technically correct on all platforms, though it's a no-op
       on big-endian ones. */
    data8 = (uint8_t *)data;
    for (i = 0; i < wordCount; i++)
    {
      data[i] = (data8[2*i] << 8) | data8[2*i + 1];
    }
  }
#endif

  return TMR_SUCCESS;
}


static TMR_Status
TMR_SR_readTagMemBytesUnaligned(TMR_Reader *reader,
                                const TMR_TagFilter *target, 
                                uint32_t bank, uint32_t byteAddress, 
                                uint16_t byteCount, uint8_t data[])
{
  TMR_Status ret;
  TMR_TagReadData read;
  uint16_t wordCount;
  uint8_t buf[254];
  TMR_SR_SerialReader *sr;

  sr = &reader->u.serialReader;

  wordCount = (uint16_t)((byteCount + 1 + (byteAddress & 1) ) / 2);
  read.data.max = 254;
  read.data.list = buf;
  read.metadataFlags = 0;

  ret = TMR_SR_cmdGEN2ReadTagData(reader, (uint16_t)(sr->commandTimeout),
                                  (TMR_GEN2_Bank)bank, byteAddress / 2, (uint8_t)wordCount,
                                  sr->gen2AccessPassword, target, &read);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  memcpy(data, buf + (byteAddress & 1), byteCount);

  return TMR_SUCCESS;
}


TMR_Status
TMR_SR_readTagMemBytes(TMR_Reader *reader, const TMR_TagFilter *target, 
                       uint32_t bank, uint32_t byteAddress, 
                       uint16_t byteCount, uint8_t data[])
{
  TMR_Status ret;
  TMR_TagReadData read;
  TMR_SR_SerialReader *sr;

  sr = &reader->u.serialReader;

  ret = setProtocol(reader, reader->tagOpParams.protocol);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  read.data.max = byteCount;
  read.data.list = (uint8_t *)data;
  read.metadataFlags = 0;

  if (TMR_TAG_PROTOCOL_GEN2 == reader->tagOpParams.protocol)
  {
    /*
     * Handling unaligned reads takes spare memory; avoid allocating that
     * (on that stack) if not necessary.
     */
    if ((byteAddress & 1) || (byteCount & 1))
    {
      return TMR_SR_readTagMemBytesUnaligned(reader, target, bank, byteAddress,
                                             byteCount, data);
    }

    return TMR_SR_cmdGEN2ReadTagData(reader, (uint16_t)(sr->commandTimeout),
                                     (TMR_GEN2_Bank)bank, byteAddress / 2, byteCount / 2,
                                     sr->gen2AccessPassword, target, &read);
  }
#ifdef TMR_ENABLE_ISO180006B
  else if (TMR_TAG_PROTOCOL_ISO180006B == reader->tagOpParams.protocol)
  {
    return TMR_SR_cmdISO180006BReadTagData(reader,(uint16_t)(sr->commandTimeout),
                                           (uint8_t)byteAddress, (uint8_t)byteCount, target,
                                           &read);
  }
#endif /* TMR_ENABLE_ISO180006B */
  else
  {
    return TMR_ERROR_UNIMPLEMENTED;
  }
}

TMR_Status
TMR_SR_modifyFlash(TMR_Reader *reader, uint8_t sector, uint32_t address,
                   uint32_t password, uint8_t length, const uint8_t data[],
                   uint32_t offset)
{
  /**
   * As only M6e reader and it's variant supports modifying the flash in 
   * application mode.Hence, this check is required.
   */
  return TMR_SR_cmdModifyFlashSector(reader, sector, address, password, length,
                                       data, offset);
}

TMR_Status
TMR_SR_writeTagMemWords(struct TMR_Reader *reader, const TMR_TagFilter *filter,
                        uint32_t bank, uint32_t address,
                        uint16_t count, const uint16_t data[])
{
  const uint8_t *dataPtr;
#ifndef TMR_BIG_ENDIAN_HOST
  uint8_t buf[254];
  TMR_wordsToBytes(count, data, buf);
  dataPtr = buf;
#else
    dataPtr = (const uint8_t *)data;
#endif

    return TMR_SR_writeTagMemBytes(reader, filter, bank, address * 2, count * 2,
                                   dataPtr);
}

TMR_Status
TMR_SR_writeTagMemBytes(struct TMR_Reader *reader, const TMR_TagFilter *filter,
                        uint32_t bank, uint32_t address,
                        uint16_t count, const uint8_t data[])
{
  TMR_Status ret;
  TMR_SR_SerialReader *sr;
  TMR_GEN2_WriteMode mode;
  TMR_GEN2_WriteMode *value = &mode;
 
  sr = &reader->u.serialReader;
  TMR_paramGet(reader, TMR_PARAM_GEN2_WRITEMODE, value);

  ret = setProtocol(reader, reader->tagOpParams.protocol);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  if (TMR_TAG_PROTOCOL_GEN2 == reader->tagOpParams.protocol)
  {
    /* Buffer for converting to Gen2 native word size.
     * A waste of space, but the only way to do byte/word conversions without
     * munging the original data input.  ReadTagMemBytes is deprecated, anyway,
     * along with all the other protocol-independent methods, so avoid using it,
     * if possible. */

    /* Misaligned writes are not permitted */
    if ((address & 1) || (count & 1))
    {
      return TMR_ERROR_INVALID;
    }

    switch (mode)
    {
      case TMR_GEN2_BLOCK_ONLY:
      case TMR_GEN2_BLOCK_FALLBACK:
        {
          uint16_t data16[TMR_SR_MAX_PACKET_SIZE/2];
          TMR_bytesToWords(count, data, data16);
          ret = TMR_SR_cmdBlockWrite(reader,(uint16_t)sr->commandTimeout, (TMR_GEN2_Bank)bank, (uint32_t)(address/2), 
                                      (uint8_t)(count/2), data16, sr->gen2AccessPassword, filter);

          switch(mode)
          {
            case TMR_GEN2_BLOCK_ONLY:
              return ret;
            case TMR_GEN2_BLOCK_FALLBACK:
              {
                /* If ret != TMR_SUCCESS, go to the next case and execute 
                 * "TMR_SR_cmdGEN2WriteTagData()" funxtion 
                 */
                if (TMR_SUCCESS == ret)
                {
                  return ret;
                }
              }
            default:
              break;
          }
        }
      case TMR_GEN2_WORD_ONLY:
        return TMR_SR_cmdGEN2WriteTagData(reader, (uint16_t)(sr->commandTimeout), (TMR_GEN2_Bank)bank, (uint32_t)(address / 2),
                                           (uint8_t)count, data, sr->gen2AccessPassword, filter);
      default: 
        return TMR_ERROR_INVALID_WRITE_MODE;
    }
  }
#ifdef TMR_ENABLE_ISO180006B
  else if (TMR_TAG_PROTOCOL_ISO180006B == reader->tagOpParams.protocol)
  {
    if (count != 1)
    {
      return TMR_ERROR_INVALID;
    }
    return TMR_SR_cmdISO180006BWriteTagData(reader, (uint16_t)(sr->commandTimeout),
                                            (uint8_t)address, 1, data, filter);
  }
#endif
  else
  {
    return TMR_ERROR_INVALID;
  }
}

/**
 * Read After Write data from a Gen2 tag.
 *
 * @param reader: the reader pointer which points to reader object
 * @param writeBank: the Gen2 memory bank to write to
 * @param writeAddress: the word address to start writing at
 * @param data: the data to write - must be an even number of bytes
 * @param count: the number of words to write
 * @param filter: a specification of the air protocol filtering to
 *                perform to find the tag
 * @param readBank: the Gen2 memory bank to read from
 * @param readAddress: the word address to start reading from
 * @param readLen: the number of words to read
 * @param read: pointes to TMR_TagReadData
 */
static TMR_Status
TMR_SR_readAfterWriteTagMemWords(struct TMR_Reader *reader, uint32_t writeBank,
                                 uint32_t writeAddress, const uint16_t data[], uint16_t count,
                                 const TMR_TagFilter *filter, uint32_t readBank,
                                 uint32_t readAddress, uint8_t readLen, TMR_TagReadData *read)
{
  TMR_Status ret;
  TMR_SR_SerialReader *sr;
  TMR_GEN2_WriteMode mode;
  const uint8_t *dataPtr;
  uint8_t buf[254];

  sr = &reader->u.serialReader;
#ifndef TMR_BIG_ENDIAN_HOST
  TMR_wordsToBytes(count, data, buf);
  dataPtr = buf;
#else
  dataPtr = (const uint8_t *)data;
#endif

  TMR_paramGet(reader, TMR_PARAM_GEN2_WRITEMODE, &mode);

  ret = setProtocol(reader, reader->tagOpParams.protocol);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  if (TMR_TAG_PROTOCOL_GEN2 == reader->tagOpParams.protocol)
  {
    /* Misaligned writes are not permitted */
    if ((writeAddress & 1) || (count & 1))
    {
      return TMR_ERROR_INVALID;
    }

    switch (mode)
    {
      case TMR_GEN2_WORD_ONLY:
        return TMR_SR_cmdGEN2ReadAfterWriteTagData(reader, (uint16_t)(sr->commandTimeout), (TMR_GEN2_Bank)writeBank,
                                                   writeAddress / 2, (uint8_t)count, dataPtr, sr->gen2AccessPassword,
                                                   filter, (TMR_GEN2_Bank)readBank, readAddress, readLen, read);
      default: 
        return TMR_ERROR_UNIMPLEMENTED_FEATURE;
    }
  }
  else
  {
    return TMR_ERROR_INVALID;
  }
}

TMR_Status
TMR_SR_lockTag(struct TMR_Reader *reader, const TMR_TagFilter *filter,
               TMR_TagLockAction *action)
{
  TMR_Status ret;
  TMR_SR_SerialReader *sr;

  sr = &reader->u.serialReader;

  ret = setProtocol(reader, reader->tagOpParams.protocol);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  if (TMR_TAG_PROTOCOL_GEN2 == reader->tagOpParams.protocol)
  {
    if (TMR_LOCK_ACTION_TYPE_GEN2_LOCK_ACTION != action->type)
    {
      /* Lock type doesn't match tagop protocol */
      return TMR_ERROR_INVALID;
    }
    return TMR_SR_cmdGEN2LockTag(reader, (uint16_t)sr->commandTimeout,
                                 action->u.gen2LockAction.mask, 
                                 action->u.gen2LockAction.action, 
                                 sr->gen2AccessPassword,
                                 filter);
  }
#ifdef TMR_ENABLE_ISO180006B
  else if (TMR_TAG_PROTOCOL_ISO180006B == reader->tagOpParams.protocol)
  {
    if (TMR_LOCK_ACTION_TYPE_ISO180006B_LOCK_ACTION != action->type)
    {
      /* Lock type doesn't match tagop protocol */
      return TMR_ERROR_INVALID;
    }
    return TMR_SR_cmdISO180006BLockTag(reader, (uint16_t)(sr->commandTimeout),
                                       action->u.iso180006bLockAction.address, 
                                       filter);
  }
#endif
  else
  {
    return TMR_ERROR_UNIMPLEMENTED;
  }
}
#endif /* TMR_ENABLE_UHF */

TMR_Status
TMR_SR_reboot(TMR_Reader *reader)
{
  TMR_Status ret;

  ret = TMR_SR_cmdrebootReader(reader);

  return ret;
}

#ifdef TMR_ENABLE_UHF
TMR_Status
TMR_SR_killTag(struct TMR_Reader *reader, const TMR_TagFilter *filter,
               const TMR_TagAuthentication *auth)
{
  TMR_Status ret;
  TMR_SR_SerialReader *sr;

  sr = &reader->u.serialReader;

  ret = setProtocol(reader, reader->tagOpParams.protocol);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  if (TMR_TAG_PROTOCOL_GEN2 == reader->tagOpParams.protocol)
  {
    if (TMR_AUTH_TYPE_GEN2_PASSWORD != auth->type)
    {
      /* Auth type doesn't match tagop protocol */
      return TMR_ERROR_INVALID;
    }

    return TMR_SR_cmdKillTag(reader,
                             (uint16_t)(sr->commandTimeout),
                             auth->u.gen2Password,
                             filter);
  }
  else
  {
    return TMR_ERROR_UNIMPLEMENTED;
  }
}
#endif /* TMR_ENABLE_UHF */
TMR_Status
TMR_SR_gpoSet(struct TMR_Reader *reader, uint8_t count,
              const TMR_GpioPin state[])
{
  TMR_Status ret;
  int i;
  
  for (i = 0; i < count; i++)
  {
    ret = TMR_SR_cmdSetGPIO(reader, state[i].id, state[i].high);
    if (TMR_SUCCESS != ret)
      return ret;
  }

  return TMR_SUCCESS;
}

TMR_Status TMR_SR_gpiGet(struct TMR_Reader *reader, uint8_t *count,
                         TMR_GpioPin state[])
{
  TMR_Status ret;
  TMR_GpioPin pinStates[4];
  uint8_t i,j, numPins;

  numPins = numberof(pinStates);
  ret = TMR_SR_cmdGetGPIO(reader, &numPins, pinStates);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  if (numPins > *count)
  {
    numPins = *count;
  }

  *count = 0;
  j = 0;
  for (i = 0 ; i < numPins ; i++)
  {
    if (!pinStates[i].output)
    {
      /* If pin is input, only then copy to output */
      state[j].id = pinStates[i].id;
      state[j].high = pinStates[i].high;
      state[j].output = pinStates[i].output;
      (*count)++;
      j ++;
    }
  }

  return TMR_SUCCESS;
}

#ifdef TMR_ENABLE_STDIO
TMR_Status
TMR_SR_firmwareLoad(struct TMR_Reader *reader, void *cookie,
                    TMR_FirmwareDataProvider provider)
{
  static const uint8_t magic[] = { 0x54, 0x4D, 0x2D, 0x53, 0x50, 0x61, 0x69, 0x6B};
  TMR_Status ret;
  uint8_t buf[256], FLASH_APP_SECTOR = 0x02,fwUpdateCnt=0;
  uint16_t packetLen, packetRemaining, size, offset;
  uint32_t len, rate, address, remaining, sector, pwd_writeFlash;
  TMR_SR_SerialReader *sr = &reader->u.serialReader;
  TMR_SR_SerialTransport *transport;
  transport = &sr->transport;

  while (1)
  {
    offset = 0;
    remaining = numberof(magic) + 8;
    pwd_writeFlash = 0;

    if (remaining > 0)
    {
      size = (uint16_t)remaining;
      if (false == provider(cookie, &size, buf + offset))
      {
        // Terminate the loop when end of file is reached.
        if (fwUpdateCnt >= 1)
          break;
        return TMR_ERROR_FIRMWARE_FORMAT;
      }
      remaining -= size;
      offset += size;
    }

    if (0 != memcmp(buf, magic, numberof(magic)))
    {
      return TMR_ERROR_FIRMWARE_FORMAT;
    }

    sector = GETU32AT(buf, 8);
    len = GETU32AT(buf, 12);

    /* @todo get any params we want to reset */
    /*
    * Drop baud to 9600 so we know for sure what it will be after going
    * back to the bootloader.  (Older firmwares always revert to 9600.
    * Newer ones keep the current baud rate.)
    */
    if (sector == FLASH_APP_SECTOR)
    {
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

#ifdef TMR_ENABLE_UHF
      /* Bootloader doesn't support wakeup preambles */
      sr->supportsPreamble = false;
#endif /* TMR_ENABLE_UHF */

      /* Bootloader doesn't support high speed operation */
      rate = sr->baudRate;
      if (rate > 115200)
      {
        rate = 115200;
      }

      if (NULL != transport->setBaudRate)
      {
        /**
        * some transport layer does not support baud rate settings.
        * for ex: TCP transport. In that case skip the baud rate
        * settings.
        */

        ret = TMR_SR_cmdSetBaudRate(reader, rate);
        if (TMR_SUCCESS != ret)
        {
          return ret;
        }
        ret = transport->setBaudRate(transport, rate);
        if (TMR_SUCCESS != ret)
        {
          return ret;
        }
      }
      ret = TMR_SR_cmdEraseFlash(reader, sector, 0x08959121);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
      pwd_writeFlash = 0x02254410;
    }

    address = 0;
    remaining = len;
    while (remaining > 0)
    {
      packetLen = 240;
      if (packetLen > remaining)
      {
        packetLen = (uint16_t)remaining;
      }
      offset = 0;
      packetRemaining = packetLen;
      while (packetRemaining > 0)
      {
        size = packetRemaining;
        if (false == provider(cookie, &size, buf + offset))
        {
          return TMR_ERROR_FIRMWARE_FORMAT;
        }
        packetRemaining -= size;
        offset += size;
      }
      ret = TMR_SR_cmdWriteFlashSector(reader, sector, address, pwd_writeFlash, (uint8_t)packetLen,buf, 0);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
      address += packetLen;
      remaining -= packetLen;
    }

    // If sector is not Application and last packet length is 240 bytes, send an empty buffer writeflash cmd 
    // to ensure peripheral fw upgrade is successful.
    if ((sector != FLASH_APP_SECTOR) && (packetLen == 240))
    {
      uint8_t *buf = NULL;
      ret = TMR_SR_cmdWriteFlashSector(reader, FLASH_APP_SECTOR, 0, pwd_writeFlash, (uint8_t)packetLen, buf, 0);
    }

    if (sector == FLASH_APP_SECTOR)
    {
      ret = TMR_SR_boot(reader, rate);
      if (ret != TMR_SUCCESS)
      {
        if (ret == TMR_ERROR_AUTOREAD_ENABLED)
        {
          ret = verifySearchStatus(reader);
          if (TMR_SUCCESS != ret)
          {
            return ret;
          }
          ret = TMR_SR_boot(reader, rate);
          }
      }
    }
    fwUpdateCnt ++;
  }
  return ret;
}
#endif /* TMR_ENABLE_STDIO */

static TMR_Status
getHardwareInfo(struct TMR_Reader *reader, void *value)
{
  //TMR_Status ret;
  //uint8_t buf[127];
  char tmp[255];
  //uint8_t count;
  TMR_SR_VersionInfo *info;

  info = &reader->u.serialReader.versionInfo;

  TMR_hexDottedQuad(info->hardware, tmp);
  /**
   * Commented below code to fix the Bug#2117
   */
  /*
     tmp[11] = '-';
     count = 127;
     ret = TMR_SR_cmdGetHardwareVersion(reader, 0, 0, &count, buf);
     if (TMR_SUCCESS != ret)
     {
     count = 0;
     tmp[11] = '\0';
     }

     TMR_bytesToHex(buf, count, tmp + 12);
     TMR_stringCopy(value, tmp, 12 + 2*count);*/
  TMR_stringCopy(value, tmp, (int)strlen(tmp));

  return TMR_SUCCESS;
}

static TMR_Status
getSerialNumber(struct TMR_Reader *reader, void *value)
{
  /* See http://trac/swtree/changeset/6498 for previous implementation */
  TMR_Status ret;
  uint8_t buf[127];
  uint8_t count, tmplen = 0;
  char tmp[127];

  count = 127;
  ret = TMR_SR_cmdGetHardwareVersion(reader, 0, 0x40, &count, buf);
  if (count == 0)
  {
    TMR_String *serial = (TMR_String *)value;
    /**
     * Command failure: 
     * Serial number not implemented on this reader
     * Leave value at default "no value" 
     */
    serial->value[0] = '\0';
    return TMR_SUCCESS;
  }
  else
  {
    int idx;
    uint8_t len;

    idx = 3;
    len = buf[idx++];
    if (len > (count-3))
    {
      ret = TMR_ERROR_UNIMPLEMENTED;
    }
    else
    {
      for (tmplen=0; tmplen<len; tmplen++)
      {
        tmp[tmplen] = (char)buf[idx++];
      }
    }
  }

  TMR_stringCopy(value, tmp, tmplen);
  return ret;
}

/* Abuse the structure layout of TMR_SR_PortPowerAndSettlingTime a bit
 * so that this can be one function instead of three only slightly
 * different pieces of code. Since all three per-port values are
 * represented as uint16_t, take an additional 'offset' parameter that
 * gives the pointer distance from the first element to the element
 * (readPower, writePower, settlingTime) that we actually want to set.
 */

#ifdef TMR_ENABLE_UHF
static TMR_Status
TMR_SR_cmdSetPerAntennaTime(struct TMR_Reader *reader, const TMR_PortValueList *list)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t j, i;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_SET_ANTENNA_PORT);
  SETU8(msg, i, 7);

  for (j = 0; j < list->len; j++)
  {
    SETU8(msg, i, list->list[j].port);
    if(list->list[j].value <= 0xFFFF)
    {
      SETS16(msg, i, (int16_t) list->list[j].value);
    }
  }

  return TMR_SR_sendCmd(reader, msg, i);
}

static TMR_Status
TMR_SR_cmdGetPerAntennaTime(struct TMR_Reader *reader, TMR_PortValueList *list)
{
  uint8_t msg[TMR_SR_MAX_PACKET_SIZE];
  uint8_t j, i;
  TMR_Status ret;

  i = 2;
  SETU8(msg, i, TMR_SR_OPCODE_GET_ANTENNA_PORT);
  SETU8(msg, i, 7);

  ret = TMR_SR_sendCmd(reader, msg, i);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  for (i = 1, j = 0; i < msg[1] && j < list->max ; i += 3, j++)
  {
    list->list[j].port = GETU8AT(msg, i + 5);
    list->list[j].value = (uint16_t)GETS16AT(msg, i + 6);
  }

  list->len = j;

  return ret;
}

static TMR_Status
setPortValues(struct TMR_Reader *reader, const TMR_PortValueList *list,
              int offset)
{
  TMR_Status ret;
  TMR_SR_PortPowerAndSettlingTime ports[TMR_SR_MAX_ANTENNA_PORTS];
  uint8_t count;
  uint16_t i, j, k;
  TMR_AntennaMapList *map;
  map = reader->u.serialReader.txRxMap;

  count = numberof(ports);
  reader->extendedAntOption = offset + 1;
  ret = TMR_SR_cmdGetAntennaPortPowersAndSettlingTime(reader, &count, ports);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /* If a value is left out, 0 is assumed */
  /*for (j = 0; j < count; j++)
  {
    *(&ports[j].readPower + offset) = 0;
  }*/

  /* 
   * For each settling time in the user's list, try to find an
   * existing entry in the list returned from the reader.
   */
  for (i = 0; i < list->len; i++)
  {
    for (k = 0 ; k < count ; k++)
    {
      if (map->list[k].antenna == list->list[i].port)
      {
        break;
      }
    }
    for (j = 0 ; j < count ; j++)
    {
      if ( map->list[k].txPort == ports[j].port)
      {
        break;
      }
    }
    if (j == count)
    {
      if (count == TMR_SR_MAX_ANTENNA_PORTS)
      {
        return TMR_ERROR_TOO_BIG;
      }
      ports[j].port =  map->list[k].txPort;
      ports[j].readPower = 0;
      ports[j].writePower = 0;
      ports[j].settlingTime = 0;
      count++;
    }

    if (list->list[i].value > 32767 || list->list[i].value < -32768)
    {
      return TMR_ERROR_ILLEGAL_VALUE;
    }
    *(&ports[j].readPower + offset) = (int16_t)(list->list[i].value);
  }
  return TMR_SR_cmdSetAntennaPortPowersAndSettlingTime(reader, count, ports);
}

/* See comment before setPortValues() for the meaning of offset */
static TMR_Status
getPortValues(struct TMR_Reader *reader, TMR_PortValueList *list, int offset)
{
  TMR_Status ret;
  TMR_SR_PortPowerAndSettlingTime ports[TMR_SR_MAX_ANTENNA_PORTS];
  uint8_t count;
  uint16_t i, j, k;
  TMR_AntennaMapList *map;
 
  map = reader->u.serialReader.txRxMap;

  count = numberof(ports);
  reader->extendedAntOption = offset + 1;
  ret = TMR_SR_cmdGetAntennaPortPowersAndSettlingTime(reader, &count, ports);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  for (i = 0, j = 0; i < count; i++)
  {
    if (
        (0 == *(&ports[i].readPower + offset)  && 
        ((TMR_SR_MODEL_MICRO != reader->u.serialReader.versionInfo.hardware[0]) ||
         (TMR_SR_MODEL_M6E_NANO != reader->u.serialReader.versionInfo.hardware[0]) ||
         (TMR_SR_MODEL_M7E_PICO != reader->u.serialReader.versionInfo.hardware[0]))) ||
         (((TMR_SR_MODEL_MICRO == reader->u.serialReader.versionInfo.hardware[0]) ||
           (TMR_SR_MODEL_M6E_NANO == reader->u.serialReader.versionInfo.hardware[0]) ||
           (TMR_SR_MODEL_M7E_PICO == reader->u.serialReader.versionInfo.hardware[0])) &&
           (-32768 ==  *(&ports[i].readPower + offset))))
    {
      continue;
    }
    if (j < list->max)
    {
      for (k = 0 ; k < count ; k++)
      {
        if ( map->list[k].txPort == ports[i].port)
        {
          break;
        }
      }
      list->list[j].port = map->list[k].antenna;
      list->list[j].value = (int32_t)*(&ports[i].readPower + offset);
    }
    j++;
  }
  list->len = (uint8_t)j;

  return TMR_SUCCESS;
}

#endif /* TMR_ENABLE_UHF */
static TMR_Status
TMR_SR_paramSet(struct TMR_Reader *reader, TMR_Param key, const void *value)
{
  TMR_Status ret;
  TMR_SR_ProtocolConfiguration protokey;
  TMR_SR_SerialReader *sr;
#if ENABLE_TMR_PARAM_BAUDRATE
  TMR_SR_SerialTransport *transport;
#endif /* ENABLE_TMR_PARAM_BAUDRATE */
  TMR_SR_Configuration readerkey;
#ifdef TMR_ENABLE_UHF
  readerkey = TMR_SR_CONFIGURATION_TRANSMIT_POWER_SAVE;
  protokey.protocol = TMR_TAG_PROTOCOL_GEN2;
#endif /* TMR_ENABLE_UHF */
  ret = TMR_SUCCESS;
  sr = &reader->u.serialReader;

  if (0 == BITGET(sr->paramConfirmed, key))
  {
    TMR_paramProbe(reader, key);
  }

  if (BITGET(sr->paramConfirmed, key) && (0 == BITGET(sr->paramPresent, key)))
  {
    return TMR_ERROR_NOT_FOUND;
  }

  switch (key)
  {
    case TMR_PARAM_URI:
    case TMR_PARAM_PRODUCT_GROUP_ID:
    case TMR_PARAM_PRODUCT_GROUP:
    case TMR_PARAM_PRODUCT_ID:
    case TMR_PARAM_VERSION_HARDWARE:
    case TMR_PARAM_VERSION_MODEL:
    case TMR_PARAM_VERSION_SOFTWARE:
    case TMR_PARAM_RADIO_POWERMAX:
    case TMR_PARAM_RADIO_POWERMIN:
    case TMR_PARAM_ANTENNA_PORTLIST:
    case TMR_PARAM_RADIO_TEMPERATURE:
    case TMR_PARAM_REGION_SUPPORTEDREGIONS:
    case TMR_PARAM_ANTENNA_CONNECTEDPORTLIST:
    case TMR_PARAM_VERSION_SUPPORTEDPROTOCOLS:
    case TMR_PARAM_TAGREADATA_TAGOPSUCCESSCOUNT:
    case TMR_PARAM_TAGREADATA_TAGOPFAILURECOUNT:
#ifdef TMR_ENABLE_UHF
    case TMR_PARAM_ANTENNA_RETURNLOSS:
    case TMR_PARAM_GEN2_PROTOCOLEXTENSION:
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_HF_LF
    case TMR_PARAM_ISO14443A_SUPPORTED_TAGTYPES:
    case TMR_PARAM_ISO14443B_SUPPORTED_TAGTYPES:
    case TMR_PARAM_ISO15693_SUPPORTED_TAGTYPES:
    case TMR_PARAM_LF125KHZ_SUPPORTED_TAGTYPES:
    case TMR_PARAM_LF134KHZ_SUPPORTED_TAGTYPES:
    case TMR_PARAM_ISO14443A_SUPPORTED_TAG_FEATURES:
    case TMR_PARAM_ISO15693_SUPPORTED_TAG_FEATURES:
    case TMR_PARAM_LF125KHZ_SUPPORTED_TAG_FEATURES:
#endif /* TMR_ENABLE_HF_LF */
    {
      ret = TMR_ERROR_READONLY;
      break;
    }
#if ENABLE_TMR_PARAM_REGION_ID
    case TMR_PARAM_REGION_ID:
      {
        ret = TMR_SR_cmdSetRegion(reader, *(TMR_Region*)value);
        if(TMR_SUCCESS == ret)
        {
          sr->regionId = *(TMR_Region*)value;
        }
        break;
      }
#endif /*ENABLE_TMR_PARAM_REGION_ID*/ 
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_STATUS_ENABLE_ANTENNAREPORT
  case TMR_PARAM_STATUS_ENABLE_ANTENNAREPORT:
    {
      *(bool *)value ? (reader->streamStats |= TMR_SR_STATUS_ANTENNA) :
        (reader->streamStats &= ~TMR_SR_STATUS_ANTENNA) ;
      break;
    }
#endif /* ENABLE_TMR_PARAM_STATUS_ENABLE_ANTENNAREPORT */
#if ENABLE_TMR_PARAM_STATUS_ENABLE_FREQUENCYREPORT
  case TMR_PARAM_STATUS_ENABLE_FREQUENCYREPORT:
    {
      *(bool *)value ? (reader->streamStats |= TMR_SR_STATUS_FREQUENCY) :
        (reader->streamStats &= ~TMR_SR_STATUS_FREQUENCY) ;
      break;
    }
#endif /*ENABLE_TMR_PARAM_STATUS_ENABLE_FREQUENCYREPORT*/
#if ENABLE_TMR_PARAM_STATUS_ENABLE_TEMPERATUREREPORT
  case TMR_PARAM_STATUS_ENABLE_TEMPERATUREREPORT:
    {
      *(bool *)value ? (reader->streamStats |= TMR_SR_STATUS_TEMPERATURE) :
        (reader->streamStats &= ~TMR_SR_STATUS_TEMPERATURE) ;
      break;
    }
#endif /* TMR_PARAM_STATUS_ENABLE_TEMPERATUREREPORT */
#endif /* TMR_ENABLE_UHF */
#if ENABLE_TMR_PARAM_BAUDRATE
  case TMR_PARAM_BAUDRATE:
  {
    uint32_t rate;

    transport = &sr->transport;
    rate = *(uint32_t *)value;

    if (reader->connected)
    {
      if (NULL != transport->setBaudRate)
      {
        /**
         * some transport layer does not support baud rate settings.
         * for ex: TCP transport. In that case skip the baud rate
         * settings.
         */ 
        ret = TMR_SR_cmdSetBaudRate(reader, rate);
        if (TMR_SUCCESS != ret)
        {
          break;
        }
        sr->baudRate = rate;
        transport->setBaudRate(transport, sr->baudRate);
      }
    }
    else
    {
      sr->baudRate = rate;
    }
    sr->isUserBaudRateSet = true;
    break;
  }
#endif /* ENABLE_TMR_PARAM_BAUDRATE */ 
#if ENABLE_TMR_PARAM_PROBEBAUDRATES
  case TMR_PARAM_PROBEBAUDRATES:
  {
    const TMR_uint32List *u32List;
    uint8_t i;
    u32List = value;

    if (u32List->len > sr->probeBaudRates.max)
    {
      ret = TMR_ERROR_TOO_BIG;
      break;
    }

    for(i = 0; i < u32List->len; i++)
    {
      sr->probeBaudRates.list[i] = u32List->list[i];
    }

    sr->probeBaudRates.len = u32List->len;
    break;
  }
#endif /*ENABLE_TMR_PARAM_PROBEBAUDRATES*/
#if ENABLE_TMR_PARAM_COMMANDTIMEOUT
  case TMR_PARAM_COMMANDTIMEOUT:
	{
      /* Typecast to int32 to check negative value */
      int32_t timeout = *(int32_t*)value;
      if (timeout < 0)
	  {
	    ret = TMR_ERROR_ILLEGAL_VALUE; 
	  }
	  else
	  {
        sr->commandTimeout = *(uint32_t *)value;
	  }
	}
    break;
#endif /* ENABLE_TMR_PARAM_COMMANDTIMEOUT */
#if ENABLE_TMR_PARAM_TRANSPORTTIMEOUT
  case TMR_PARAM_TRANSPORTTIMEOUT:
    {
      /* Typecast to int32 to check negative value */
      int32_t timeout = *(int32_t*)value;
      if (timeout < 0)
      {
        ret = TMR_ERROR_ILLEGAL_VALUE; 
      }
      else
      {
        /**
         * In case user specified the timeout value for connect
         * Enable the usrTimoutEnable option
         */
        sr->transportTimeout = *(uint32_t *)value;
        sr->usrTimeoutEnable = true;
      }
    }
	break;
#endif /* ENABLE_TMR_PARAM_TRANSPORTTIMEOUT */
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_RADIO_ENABLEPOWERSAVE
  case TMR_PARAM_RADIO_ENABLEPOWERSAVE:
	readerkey = TMR_SR_CONFIGURATION_TRANSMIT_POWER_SAVE;
	break;
#endif /* ENABLE_TMR_PARAM_RADIO_ENABLEPOWERSAVE */
#if ENABLE_TMR_PARAM_RADIO_ENABLESJC
  case TMR_PARAM_RADIO_ENABLESJC:
	readerkey = TMR_SR_CONFIGURATION_SELF_JAMMER_CANCELLATION;
	break;
#endif /*ENABLE_TMR_PARAM_RADIO_ENABLESJC*/
#if ENABLE_TMR_PARAM_TAGREADDATA_UNIQUEBYPROTOCOL
  case TMR_PARAM_TAGREADDATA_UNIQUEBYPROTOCOL:
    {
      ret = TMR_SR_cmdSetReaderConfiguration(reader, TMR_SR_CONFIGURATION_UNIQUE_BY_PROTOCOL, value);
    }
    break;
#endif /* ENABLE_TMR_PARAM_TAGREADDATA_UNIQUEBYPROTOCOL */
#if ENABLE_TMR_PARAM_TAGREADDATA_READFILTERTIMEOUT
  case TMR_PARAM_TAGREADDATA_READFILTERTIMEOUT:
    {
      int32_t timeout = (TMR_DEFAULT_READ_FILTER_TIMEOUT == *(int32_t *)value) ? 0 : *(int32_t *)value;
      ret = TMR_SR_cmdSetReaderConfiguration(reader, TMR_SR_CONFIGURATION_READ_FILTER_TIMEOUT, &timeout);
      if (TMR_SUCCESS == ret)
      {
        reader->u.serialReader.readFilterTimeout = timeout;
      }
    }
    break;
#endif /* ENABLE_TMR_PARAM_TAGREADDATA_READFILTERTIMEOUT */
#endif /* TMR_ENABLE_UHF */
#if ENABLE_TMR_PARAM_TAGREADDATA_ENABLEREADFILTER
  case TMR_PARAM_TAGREADDATA_ENABLEREADFILTER:
    {
      ret = TMR_SR_cmdSetReaderConfiguration(reader, TMR_SR_CONFIGURATION_ENABLE_READ_FILTER, value);
      if (TMR_SUCCESS == ret)
      {
        reader->u.serialReader.enableReadFiltering = *(bool *)value;
        reader->u.serialReader.userEnableReadFiltering = true;
      }
    }
    break;
#endif /* ENABLE_TMR_PARAM_TAGREADDATA_ENABLEREADFILTER */
#if ENABLE_TMR_PARAM_RADIO_READPOWER
  case TMR_PARAM_RADIO_READPOWER:
	ret = TMR_SR_cmdSetReadTxPower(reader, *(int32_t *)value);
	break;
#endif /* ENABLE_TMR_PARAM_RADIO_READPOWER */
#if ENABLE_TMR_PARAM_RADIO_WRITEPOWER
  case TMR_PARAM_RADIO_WRITEPOWER:
    ret = TMR_SR_cmdSetWriteTxPower(reader, *(int32_t *)value);
    break;
#endif /* ENABLE_TMR_PARAM_RADIO_WRITEPOWER */ 
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_RADIO_PORTREADPOWERLIST
  case TMR_PARAM_RADIO_PORTREADPOWERLIST:
    ret = setPortValues(reader, value, 0);
    break;
#endif /* ENABLE_TMR_PARAM_RADIO_PORTREADPOWERLIST */
#if ENABLE_TMR_PARAM_RADIO_PORTWRITEPOWERLIST
  case TMR_PARAM_RADIO_PORTWRITEPOWERLIST:
    ret = setPortValues(reader, value, 1);
    break;
#endif /*ENABLE_TMR_PARAM_RADIO_PORTWRITEPOWERLIST*/
#if ENABLE_TMR_PARAM_ANTENNA_SETTLINGTIMELIST
  case TMR_PARAM_ANTENNA_SETTLINGTIMELIST:
    ret = setPortValues(reader, value, 2);
    break;
#endif /* TMR_PARAM_ANTENNA_SETTLINGTIMELIST */
#if ENABLE_TMR_PARAM_ANTENNA_CHECKPORT
  case TMR_PARAM_ANTENNA_CHECKPORT:
    readerkey = TMR_SR_CONFIGURATION_SAFETY_ANTENNA_CHECK;
    break;
#endif /* ENABLE_TMR_PARAM_ANTENNA_CHECKPORT */
#if ENABLE_TMR_PARAM_TAGREADDATA_RECORDHIGHESTRSSI
  case TMR_PARAM_TAGREADDATA_RECORDHIGHESTRSSI:
    readerkey = TMR_SR_CONFIGURATION_RECORD_HIGHEST_RSSI;
    break;
#endif /* ENABLE_TMR_PARAM_TAGREADDATA_RECORDHIGHESTRSSI */
#if ENABLE_TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA
  case TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA:
    readerkey = TMR_SR_CONFIGURATION_UNIQUE_BY_ANTENNA;
    break;
#endif /* ENABLE_TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA */
#if ENABLE_TMR_PARAM_TAGREADDATA_UNIQUEBYDATA
  case TMR_PARAM_TAGREADDATA_UNIQUEBYDATA:
    readerkey = TMR_SR_CONFIGURATION_UNIQUE_BY_DATA;
    break;
#endif /*ENABLE_TMR_PARAM_TAGREADDATA_UNIQUEBYDATA*/
#endif /* TMR_ENABLE_UHF */
#if ENABLE_TMR_PARAM_ANTENNA_PORTSWITCHGPOS
  case TMR_PARAM_ANTENNA_PORTSWITCHGPOS:
  {
    const TMR_uint8List *u8list;
    uint16_t i;

    u8list = value;
    reader->portmask = 0;
    for (i = 0 ; i < u8list->len && i < u8list->max ; i++)
    {
      reader->portmask |= 1 << (u8list->list[i] - 1);
    }
    
    ret = TMR_SR_cmdSetReaderConfiguration(
    reader, TMR_SR_CONFIGURATION_ANTENNA_CONTROL_GPIO, &reader->portmask);

    if (TMR_SUCCESS != ret)
    {
      break;
    }
	sr->isTxRxMapSet = true;
    ret = initTxRxMapFromPorts(reader);
    
    break;
  }
#endif /* ENABLE_TMR_PARAM_ANTENNA_PORTSWITCHGPOS */
#if ENABLE_TMR_PARAM_TRIGGER_READ_GPI
  case TMR_PARAM_TRIGGER_READ_GPI:
    {
      uint8_t portmask;
      const TMR_uint8List *u8list;
      uint16_t i;

      u8list = value;
      portmask = 0;
      for (i = 0 ; i < u8list->len && i < u8list->max ; i++)
      {
        portmask |= 1 << (u8list->list[i] - 1);
      }

      ret = TMR_SR_cmdSetReaderConfiguration(
        reader, TMR_SR_CONFIGURATION_TRIGGER_READ_GPIO, &portmask);
      break;
    }
#endif /* ENABLE_TMR_PARAM_TRIGGER_READ_GPI */
#if ENABLE_TMR_PARAM_ANTENNA_TXRXMAP
  case TMR_PARAM_ANTENNA_TXRXMAP:
  {
    const TMR_AntennaMapList *map;
    TMR_AntennaMapList *mymap;
    uint8_t len;
    uint16_t i, j;

    map = value;
    mymap = sr->txRxMap;

    if (map->len > mymap->len)
    {
      ret = TMR_ERROR_TOO_BIG;
      break;
    }

    len = 0;

    for (i = 0 ; i < map->len ; i++)
    {
      /* Check whether the user provided mapping lists elements are valid or not.
       * Throw an error in case of the logical antenna or tx port which is not set/supported on the module.
       * Also, The txport and rxport should be same.
       */
      for (j = 0; j < mymap->len; j++)
      {
        if ((map->list[i].txPort == mymap->list[j].antenna) && (map->list[i].txPort == map->list[i].rxPort))
        {
          break;
        }
      }
      if (j == mymap->len)
      {
        return TMR_ERROR_NO_ANTENNA;
      }

      /* Error check for txrxmap.
       * Antenna or tx/rx port should not be repeated in the mapping list.
       */
      for (j = i+1; j < map->len; j++)
      {
        if ((map->list[i].antenna == map->list[j].antenna) || (map->list[i].txPort == map->list[j].txPort))
        {
          return TMR_ERROR_INVALID_ANTENNA_CONFIG;
        }
      }

      len = i+1;
    }

    for (i = 0; i < len; i ++)
    {
      mymap->list[i] = map->list[i];
    }
    mymap->len = len;
    sr->isTxRxMapSet = true;
    break;
  }
#endif /* ENABLE_TMR_PARAM_ANTENNA_TXRXMAP */

#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_REGION_HOPTABLE
  case TMR_PARAM_REGION_HOPTABLE:
  {
    const TMR_uint32List *u32list;

    u32list = value;

    ret = TMR_SR_cmdSetFrequencyHopTable(reader, (uint8_t)u32list->len, u32list->list);
    break;
  }
#endif /* ENABLE_TMR_PARAM_REGION_HOPTABLE */
#if ENABLE_TMR_PARAM_REGION_HOPTIME
  case TMR_PARAM_REGION_HOPTIME:
    ret = TMR_SR_cmdSetFrequencyHopTableOption(reader, *(uint32_t *)value,
                                                 TMR_SR_OPCODE_FREQ_HOP_TABLE_OPTION_HOPTIME);
    break;
#endif /*ENABLE_TMR_PARAM_REGION_HOPTIME */
#if ENABLE_TMR_PARAM_REGION_DWELL_TIME
  case  TMR_PARAM_REGION_DWELL_TIME:
    ret = TMR_SR_cmdSetRegionDwellTime(reader, sr->regionId, *(uint16_t *)value);
    break;
#endif /*ENABLE_TMR_PARAM_REGION_DWELL_TIME*/
#if ENABLE_TMR_PARAM_REGION_DWELL_TIME_ENABLE
  case  TMR_PARAM_REGION_DWELL_TIME_ENABLE:
    ret = TMR_SR_cmdSetRegionDwellTimeEnable(reader, sr->regionId, *(bool *)value);
    break;
#endif /*ENABLE_TMR_PARAM_REGION_DWELL_TIME_ENABLE*/
#if ENABLE_TMR_PARAM_REGION_LBT_THRESHOLD
  case  TMR_PARAM_REGION_LBT_THRESHOLD:
    ret = TMR_SR_cmdSetRegionLbtThreshold(reader, sr->regionId, *(int8_t *)value);
    break;
#endif /*ENABLE_TMR_PARAM_REGION_LBT_THRESHOLD*/
#if ENABLE_TMR_PARAM_REGION_LBT_ENABLE
  case TMR_PARAM_REGION_LBT_ENABLE:
  {
    ret = TMR_SR_cmdSetRegionLbt(reader, sr->regionId, *(bool *)value);
    break;
  }
#endif /* ENABLE_TMR_PARAM_REGION_LBT_ENABLE */
#endif /* TMR_ENABLE_UHF */
#if ENABLE_TMR_PARAM_TAGOP_ANTENNA
  case TMR_PARAM_TAGOP_ANTENNA:
  {
    uint16_t i;
    TMR_AntennaMapList *map;
    uint8_t antenna;
    uint8_t txPort, rxPort;

    map = sr->txRxMap;
    antenna = *(uint8_t *)value;

    txPort = rxPort = 0;
    for (i = 0; i < map->len && i < map->max; i++)
    {
      if (map->list[i].antenna == antenna)
      {
        txPort = map->list[i].txPort;
        rxPort = map->list[i].rxPort;
        reader->tagOpParams.antenna = antenna;
        break;
      }
    }
    if (txPort == 0)
    {
      ret = TMR_ERROR_NO_ANTENNA;
    }
    else
    {
      ret = TMR_SR_cmdSetTxRxPorts(reader, txPort, rxPort);
    }
    break;
  }
#endif /* ENABLE_TMR_PARAM_TAGOP_ANTENNA */
#if ENABLE_TMR_PARAM_TAGOP_PROTOCOL
  case TMR_PARAM_TAGOP_PROTOCOL:
    {
      ret = setProtocol(reader, *(TMR_TagProtocol *)value);
      if (TMR_SUCCESS == ret)
      {
        reader->tagOpParams.protocol = *(TMR_TagProtocol *)value;
        reader->u.serialReader.currentProtocol = reader->tagOpParams.protocol;
#ifdef TMR_ENABLE_HF_LF
        reader->isProtocolDynamicSwitching = false;
#endif /* TMR_ENABLE_HF_LF */
      }
      break;
    }
#endif /* ENABLE_TMR_PARAM_TAGOP_PROTOCOL */
#if ENABLE_TMR_PARAM_READ_PLAN
  case TMR_PARAM_READ_PLAN:
  {
    const TMR_ReadPlan *plan;
    TMR_ReadPlan tmpPlan;

    plan = value;
    tmpPlan = *plan;

    ret = validateReadPlan(reader, &tmpPlan, 
                sr->txRxMap, sr->versionInfo.protocols);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
    
    *reader->readParams.readPlan = tmpPlan;
    reader->isAntennaSet = true;
    break;
  }
#endif /* ENABLE_TMR_PARAM_READ_PLAN */
#if ENABLE_TMR_PARAM_GPIO_INPUTLIST_OUTPUTLIST
  case TMR_PARAM_GPIO_INPUTLIST:
  case TMR_PARAM_GPIO_OUTPUTLIST:
    {
      const TMR_uint8List *u8list;
      int bit, i, newDirections, pin;

      u8list = value;

      if (key == TMR_PARAM_GPIO_OUTPUTLIST)
      {
        newDirections = 0;
      }
      else
      {
        newDirections = 0x1e;
      }

      for (i = 0 ; i < u8list->len && i < u8list->max ; i++)
      {
        if (key == TMR_PARAM_GPIO_OUTPUTLIST)
        {
          newDirections |= (1 << u8list->list[i]);
        }
        else
        {
          newDirections &= ~(1 << u8list->list[i]);
        }
      }

      for (pin = 0 ; pin < u8list->len ; pin++)
      {
        bit = 1 << (u8list->list[pin]);

        ret = TMR_SR_cmdSetGPIODirection(reader, u8list->list[pin],
          (newDirections & bit) != 0);
        if (TMR_SUCCESS != ret)
        {
          return ret;
        }

      }
      break;
    }
#endif /* ENABLE_TMR_PARAM_GPIO_INPUTLIST */
#if ENABLE_TMR_PARAM_POWERMODE
  case TMR_PARAM_POWERMODE:
    {
      ret = TMR_SR_cmdSetPowerMode(reader, *(TMR_SR_PowerMode *)value);
      if (TMR_SUCCESS == ret)
      {
        sr->powerMode = *(TMR_SR_PowerMode *)value;
      }
    }
    break;
#endif /* ENABLE_TMR_PARAM_POWERMODE */
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_USERMODE
  case TMR_PARAM_USERMODE:
    ret = TMR_SR_cmdSetUserMode(reader, *(TMR_SR_UserMode *)value);
    break;
#endif /* ENABLE_TMR_PARAM_USERMODE */
#if ENABLE_TMR_PARAM_GEN2_Q
  case TMR_PARAM_GEN2_Q:
    protokey.u.gen2 = TMR_SR_GEN2_CONFIGURATION_Q;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_Q */
#if ENABLE_TMR_PARAM_GEN2_TAGENCODING
  case TMR_PARAM_GEN2_TAGENCODING:
    protokey.u.gen2 = TMR_SR_GEN2_CONFIGURATION_TAGENCODING;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_TAGENCODING */
#if ENABLE_TMR_PARAM_GEN2_SESSION
  case TMR_PARAM_GEN2_SESSION:
    protokey.u.gen2 = TMR_SR_GEN2_CONFIGURATION_SESSION;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_SESSION */
#if ENABLE_TMR_PARAM_GEN2_TARGET
  case TMR_PARAM_GEN2_TARGET:
    protokey.u.gen2 = TMR_SR_GEN2_CONFIGURATION_TARGET;
    break;
#endif /*ENABLE_TMR_PARAM_GEN2_TARGET*/
#if ENABLE_TMR_PARAM_GEN2_BLF
  case TMR_PARAM_GEN2_BLF:
    protokey.u.gen2 = TMR_SR_GEN2_CONFIGURATION_LINKFREQUENCY;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_BLF */
#if ENABLE_TMR_PARAM_GEN2_TARI
  case TMR_PARAM_GEN2_TARI:
    protokey.u.gen2 = TMR_SR_GEN2_CONFIGURATION_TARI;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_TARI */
#if ENABLE_TMR_PARAM_GEN2_WRITEMODE
  case TMR_PARAM_GEN2_WRITEMODE:
    sr->writeMode = *(TMR_GEN2_WriteMode *)value;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_WRITEMODE */
#if ENABLE_TMR_PARAM_GEN2_BAP
  case TMR_PARAM_GEN2_BAP:
    protokey.u.gen2 = TMR_SR_GEN2_CONFIGURATION_BAP;
    break;
#endif /*ENABLE_TMR_PARAM_GEN2_BAP*/
#if ENABLE_TMR_PARAM_GEN2_T4
  case TMR_PARAM_GEN2_T4:
    protokey.u.gen2 = TMR_SR_GEN2_CONFIGURATION_T4;
    break;
#endif /*ENABLE_TMR_PARAM_GEN2_T4*/
#if ENABLE_TMR_PARAM_GEN2_INITIAL_Q
  case TMR_PARAM_GEN2_INITIAL_Q:
    protokey.u.gen2 = TMR_SR_GEN2_INITIAL_Q;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_INITIAL_Q */
#if ENABLE_TMR_PARAM_GEN2_SEND_SELECT
  case TMR_PARAM_GEN2_SEND_SELECT:
    protokey.u.gen2 = TMR_SR_GEN2_SEND_SELECT;
    break;
#endif /*ENABLE_TMR_PARAM_GEN2_SEND_SELECT*/
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_ISO180006B
#if ENABLE_TMR_PARAM_ISO180006B_BLF
  case TMR_PARAM_ISO180006B_BLF:
    protokey.protocol = TMR_TAG_PROTOCOL_ISO180006B;
    protokey.u.iso180006b = TMR_SR_ISO180006B_CONFIGURATION_LINKFREQUENCY;
    break;
#endif /* ENABLE_TMR_PARAM_ISO180006B_BLF */
#if ENABLE_TMR_PARAM_ISO180006B_MODULATION_DEPTH
  case TMR_PARAM_ISO180006B_MODULATION_DEPTH:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_ISO180006B;
      protokey.u.iso180006b = TMR_SR_ISO180006B_CONFIGURATION_MODULATION_DEPTH;
      break;
    }
#endif /* ENABLE_TMR_PARAM_ISO180006B_MODULATION_DEPTH */
#if ENABLE_TMR_PARAM_ISO180006B_DELIMITER
  case TMR_PARAM_ISO180006B_DELIMITER:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_ISO180006B;
      protokey.u.iso180006b = TMR_SR_ISO180006B_CONFIGURATION_DELIMITER;
      break;
    }
#endif /*ENABLE_TMR_PARAM_ISO180006B_DELIMITER*/
#endif /* TMR_ENABLE_ISO180006B */
#ifdef TMR_ENABLE_HF_LF
#if ENABLE_TMR_PARAM_PROTOCOL_LIST
  case TMR_PARAM_PROTOCOL_LIST:
    {
      TMR_TagProtocolList *protocols;
      protocols = (TMR_TagProtocolList *)value;

      ret = TMR_SR_cmdSetProtocolList(reader, protocols);
      if (TMR_SUCCESS == ret)
      {
        reader->isProtocolDynamicSwitching = true;
      }
      break;
    }
#endif /*ENABLE_TMR_PARAM_PROTOCOL_LIST*/
#if ENABLE_TMR_PARAM_ISO14443A_TAGTYPE
  case TMR_PARAM_ISO14443A_TAGTYPE:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_ISO14443A;
      protokey.u.iso14443a = TMR_SR_ISO14443A_CONFIGURATION_TAGTYPE;
      break;
    }
#endif /* ENABLE_TMR_PARAM_ISO14443A_TAGTYPE */
#if ENABLE_TMR_PARAM_ISO14443B_TAGTYPE
  case TMR_PARAM_ISO14443B_TAGTYPE:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_ISO14443B;
      protokey.u.iso14443b = TMR_SR_ISO14443B_CONFIGURATION_TAGTYPE;
      break;
    }
#endif /*ENABLE_TMR_PARAM_ISO14443B_TAGTYPE*/
#if ENABLE_TMR_PARAM_ISO15693_TAGTYPE
  case TMR_PARAM_ISO15693_TAGTYPE:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_ISO15693;
      protokey.u.iso15693 = TMR_SR_ISO15693_CONFIGURATION_TAGTYPE;
      break;
    }
#endif /* ENABLE_TMR_PARAM_ISO15693_TAGTYPE */
#if ENABLE_TMR_PARAM_LF125KHZ_TAGTYPE
  case TMR_PARAM_LF125KHZ_TAGTYPE:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_LF125KHZ;
      protokey.u.lf125khz = TMR_SR_LF125KHZ_CONFIGURATION_TAGTYPE;
      break;
    }
#endif /* ENABLE_TMR_PARAM_LF125KHZ_TAGTYPE */
#if ENABLE_TMR_PARAM_LF134KHZ_TAGTYPE
  case TMR_PARAM_LF134KHZ_TAGTYPE:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_LF134KHZ;
      protokey.u.lf134khz = TMR_SR_LF134KHZ_CONFIGURATION_TAGTYPE;
      break;
    }
#endif /* ENABLE_TMR_PARAM_LF134KHZ_TAGTYPE */
#if ENABLE_TMR_PARAM_LF125KHZ_SECURE_RD_FORMAT
  case TMR_PARAM_LF125KHZ_SECURE_RD_FORMAT:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_LF125KHZ;
      protokey.u.lf125khz = TMR_SR_LF125KHZ_CONFIGURATION_SECURE_RD_FORMAT;
      break;
    }
#endif /* ENABLE_TMR_PARAM_LF125KHZ_SECURE_RD_FORMAT */
#if ENABLE_TMR_PARAM_RADIO_KEEP_RF_ON
  case TMR_PARAM_RADIO_KEEP_RF_ON:
    {
      readerkey = TMR_SR_CONFIGURATION_KEEP_RF_ON;
      break;
    }
#endif /* ENABLE_TMR_PARAM_RADIO_KEEP_RF_ON */
#endif /* TMR_ENABLE_HF_LF */
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_GEN2_ACCESSPASSWORD
  case TMR_PARAM_GEN2_ACCESSPASSWORD:
    sr->gen2AccessPassword = *(TMR_GEN2_Password *)value;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_ACCESSPASSWORD */
#endif /* TMR_ENABLE_UHF */
#if ENABLE_TMR_PARAM_LICENSE_KEY
  case TMR_PARAM_LICENSE_KEY:
    {
      TMR_uint8List *license = (TMR_uint8List *)value;

      ret = TMR_SR_cmdSetProtocolLicenseKey(reader, TMR_SR_SET_LICENSE_KEY, license->list, license->len);
	}
	break;
#endif /* ENABLE_TMR_PARAM_LICENSE_KEY */
#if ENABLE_TMR_PARAM_MANAGE_LICENSE_KEY
  case TMR_PARAM_MANAGE_LICENSE_KEY:
    {
      TMR_LicenseOperation *licenseOperation;
      TMR_uint8List *license;

      if (NULL == value)
      {
        ret = TMR_ERROR_INVALID;
        break;
      }

      licenseOperation = (TMR_LicenseOperation *)value ;
      license = licenseOperation->license;

      ret = TMR_SR_cmdSetProtocolLicenseKey(reader, (TMR_SR_SetProtocolLicenseOption)licenseOperation->option, license->list, license->len);
    }
    break;
#endif /* ENABLE_TMR_PARAM_MANAGE_LICENSE_KEY */
#if ENABLE_TMR_PARAM_USER_CONFIG
  case TMR_PARAM_USER_CONFIG:
    {
	  TMR_SR_UserConfigOp *config = (TMR_SR_UserConfigOp *)value;
   
      ret = TMR_SR_cmdSetUserProfile(reader, (TMR_SR_UserConfigOperation)config->op, config->category, 
                                       TMR_SR_CUSTOM_CONFIGURATION);
      break;
	}
#endif /*ENABLE_TMR_PARAM_USER_CONFIG*/
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_READER_STATISTICS
  case TMR_PARAM_READER_STATISTICS:
  /* Only RF On time statistic can be reset to 0 */
  ret = TMR_SR_cmdResetReaderStatistics(reader, TMR_SR_READER_STATS_ALL);
  break;
#endif /* ENABLE_TMR_PARAM_READER_STATISTICS */
#endif /* TMR_ENABLE_UHF */
#if ENABLE_TMR_PARAM_READER_STATS
  case TMR_PARAM_READER_STATS:
  {
    if ((NULL != reader->pSupportsResetStats) && (false == *(reader->pSupportsResetStats)))
    {
      /* Command not supported, pop up the error */
      return TMR_ERROR_UNSUPPORTED;
    }

    /* Only RF On time statistic can be reset to 0 */
    ret = TMR_SR_cmdResetReaderStats(reader, reader->userStatsFlag);
    break;
  }
#endif /* ENABLE_TMR_PARAM_READER_STATS */
#if ENABLE_TMR_PARAM_READER_STATS_ENABLE
  case TMR_PARAM_READER_STATS_ENABLE:
  {
    TMR_Reader_StatsFlag flags;
    flags = *(TMR_Reader_StatsFlag *)value;

    if ((NULL != reader->pSupportsResetStats) && (false == *(reader->pSupportsResetStats)))
    {
      /* Command not supported, pop up the error */
      return TMR_ERROR_UNSUPPORTED;
    }

    if (flags == TMR_READER_STATS_FLAG_ALL)
    {
      reader->statsFlag = reader->userStatsFlag;
    }
    else
    {
      if ((flags & reader->userStatsFlag) == flags)
      {
        /* Store the statitics flags requested by the user */
        reader->statsFlag = flags;  
      }
      else
      {
        return TMR_ERROR_INVALID_READER_STATS;
      }
    }
    break;
  }
#endif /* ENABLE_TMR_PARAM_READER_STATS_ENABLE */
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_READER_WRITE_REPLY_TIMEOUT_EARLY_EXIT
  case TMR_PARAM_READER_WRITE_REPLY_TIMEOUT:
  case TMR_PARAM_READER_WRITE_EARLY_EXIT:
    {
      TMR_SR_Gen2ReaderWriteTimeOut timeout;

      /* Get the values before setting it */
      ret = TMR_SR_cmdGetReaderWriteTimeOut(reader, protokey.protocol, &timeout);
      if (TMR_SUCCESS != ret)
      {
        break;
      }

      /* set the parameter asked by the user */
      switch (key)
      {
      case TMR_PARAM_READER_WRITE_REPLY_TIMEOUT:
        {
          timeout.writetimeout = *(uint16_t *)value;
          break;
        }
      case TMR_PARAM_READER_WRITE_EARLY_EXIT:
        {
          timeout.earlyexit = !(*(bool *)value);
          break;
        }
      default:
        ret = TMR_ERROR_NOT_FOUND;
      }

      /* set the vakue */
      ret = TMR_SR_cmdSetReaderWriteTimeOut (reader,protokey.protocol, &timeout);
      break;
    }
#endif /*ENABLE_TMR_PARAM_READER_WRITE_REPLY_TIMEOUT*/
#endif /* TMR_ENABLE_UHF */
#if ENABLE_TMR_PARAM_METADATAFLAG
  case TMR_PARAM_METADATAFLAG:
	  {
        TMR_TRD_MetadataFlag metadataFlag;
        metadataFlag = *(TMR_TRD_MetadataFlag *)value;

        if (metadataFlag & TMR_TRD_METADATA_FLAG_PROTOCOL)
        {
#ifdef TMR_ENABLE_HF_LF
          if(TMR_SR_MODEL_M3E == sr->versionInfo.hardware[0])
          {
            if (!(metadataFlag & TMR_TRD_METADATA_FLAG_TAGTYPE))
			{
              return TMR_ERROR_METADATA_TAGTYPEMISSING;
			}
          }
#endif /* TMR_ENABLE_HF_LF */

          if (metadataFlag != TMR_TRD_METADATA_FLAG_ALL)
		  {
            if ((metadataFlag & TMR_TRD_METADATA_FLAG_BRAND_IDENTIFIER) && !(TMR_SR_MODEL_M3E == sr->versionInfo.hardware[0]))
            {
               metadataFlag &= reader->userMetadataFlag | TMR_TRD_METADATA_FLAG_BRAND_IDENTIFIER;
               reader->allValidMetadataFlags = reader->userMetadataFlag | TMR_TRD_METADATA_FLAG_BRAND_IDENTIFIER;
            }
            if ((metadataFlag & reader->allValidMetadataFlags) ==  metadataFlag)
            {
              reader->userMetadataFlag = metadataFlag;
            }
            else
            {
              ret = TMR_ERROR_METADATA_INVALID;
            }
          }
          else
          {
            reader->userMetadataFlag = reader->allValidMetadataFlags;
          }
        }
        else
        {
          ret = TMR_ERROR_METADATA_PROTOCOLMISSING;
        }
		break;
	  }
#endif /* ENABLE_TMR_PARAM_METADATAFLAG */
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_REGION_QUANTIZATION_STEP
  case TMR_PARAM_REGION_QUANTIZATION_STEP:
    ret = TMR_SR_cmdSetFrequencyHopTableOption(reader, *(uint32_t *)value,
                                                 TMR_SR_OPCODE_FREQ_HOP_TABLE_OPTION_QUANTIZATION_STEP);
    break;
#endif /*ENABLE_TMR_PARAM_REGION_QUANTIZATION_STEP*/
#if ENABLE_TMR_PARAM_REGION_MINIMUM_FREQUENCY
  case TMR_PARAM_REGION_MINIMUM_FREQUENCY:
    ret = TMR_SR_cmdSetFrequencyHopTableOption(reader, *(uint32_t *)value, 
                                                 TMR_SR_OPCODE_FREQ_HOP_TABLE_OPTION_MINIMUM_FREQUENCY);  
    break;
#endif /* ENABLE_TMR_PARAM_REGION_MINIMUM_FREQUENCY */
#endif /* TMR_ENABLE_UHF */
#if !((!defined(TMR_ENABLE_UHF)) || defined(BARE_METAL))
#if ENABLE_TMR_PARAM_REGULATORY_MODE
  case TMR_PARAM_REGULATORY_MODE:
    {
      reader->regulatoryParams.RegMode = *(TMR_SR_RegulatoryMode *)value;
      break;
    }
#endif /*ENABLE_TMR_PARAM_REGULATORY_MODE*/
#if ENABLE_TMR_PARAM_REGULATORY_MODULATION
  case TMR_PARAM_REGULATORY_MODULATION:
    {
      reader->regulatoryParams.RegModulation = *(TMR_SR_RegulatoryModulation *)value;
      break;
    }
#endif /*ENABLE_TMR_PARAM_REGULATORY_MODULATION*/
#if ENABLE_TMR_PARAM_REGULATORY_ONTIME
  case TMR_PARAM_REGULATORY_ONTIME:
    {
      if(((*(uint32_t *)value) < TMR_MIN_VALUE) || ((*(uint32_t *)value) > TMR_MAX_VALUE))
      {
        ret = TMR_ERROR_INVALID_VALUE;
      }
      reader->regulatoryParams.regOnTime = *(uint32_t *)value;
      break;
    }
#endif /*ENABLE_TMR_PARAM_REGULATORY_ONTIME*/
#if ENABLE_TMR_PARAM_REGULATORY_OFFTIME
  case TMR_PARAM_REGULATORY_OFFTIME:
    {
      if(((*(uint32_t *)value) < TMR_MIN_VALUE) || ((*(uint32_t *)value) > TMR_MAX_VALUE))
      {
        ret = TMR_ERROR_INVALID_VALUE;
      }
      reader->regulatoryParams.regOffTime = *(uint32_t *)value;
      break;
    }
#endif /* ENABLE_TMR_PARAM_REGULATORY_OFFTIME */
#if ENABLE_TMR_PARAM_REGULATORY_ENABLE
  case TMR_PARAM_REGULATORY_ENABLE:
    {
      ret = TMR_SR_cmdSetRegulatoryTest(reader, *(uint8_t *)value);
      break;
    }
#endif /* ENABLE_TMR_PARAM_REGULATORY_ENABLE */
#endif /* TMR_ENABLE_HF_LF || BARE_METAL */
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_PER_ANTENNA_TIME
  case TMR_PARAM_PER_ANTENNA_TIME:
  {
    ret = TMR_SR_cmdSetPerAntennaTime(reader, (TMR_PortValueList *)value);
    if(ret == TMR_SUCCESS)
    {
      reader->isPerAntTimeSet = true;
    }
    break;
  }
#endif /*ENABLE_TMR_PARAM_PER_ANTENNA_TIME*/
#endif /* TMR_ENABLE_UHF */
  default:
    ret = TMR_ERROR_NOT_FOUND;
  }

  switch (key)
  {
#ifdef TMR_ENABLE_UHF
  case TMR_PARAM_ANTENNA_CHECKPORT:
  case TMR_PARAM_RADIO_ENABLEPOWERSAVE:
  case TMR_PARAM_RADIO_ENABLESJC:
  case TMR_PARAM_TAGREADDATA_RECORDHIGHESTRSSI:
  case TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA:
  case TMR_PARAM_TAGREADDATA_UNIQUEBYDATA:
#else
  case TMR_PARAM_RADIO_KEEP_RF_ON:
#endif /* TMR_ENABLE_UHF */
    {
      ret = TMR_SR_cmdSetReaderConfiguration(reader, readerkey, value);
      break;
   }

#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_GEN2_BAP
  case TMR_PARAM_GEN2_BAP:
    {
      TMR_GEN2_Bap *bap;
      bap = (TMR_GEN2_Bap *)value;

      if (NULL == (TMR_GEN2_Bap *)value)
      {
        /** It means user is disabling the bap support, make the flag down 
         * and skip the command sending
         **/ 
        sr->isBapEnabled = false;
        break;
      }
      else if ((-1 > bap->powerUpDelayUs) || (-1 > bap->freqHopOfftimeUs))
      {
        /*
         * Invalid values for BAP parameters,
         * Accepts only positive values or -1 for NULL
         */ 
        return TMR_ERROR_ILLEGAL_VALUE;
      }
      else if ((-1 == bap->powerUpDelayUs) && (-1 == bap->freqHopOfftimeUs))
      {
        /**
         * Here -1 signifies NULL. API should skips the parameter and does not sent the matching
         * command to the reader. Let reader use its own default
         **/
        sr->isBapEnabled = true;
        break;
      }
      else
      {
        /*
         * do a paramGet of BAP parameters. This serves two purposes.
         * 1. API knows wheather module supports the BAP options or not.
         * 2. API can assign default values to the fields, are not set by the user.
         */
        TMR_GEN2_Bap getBapParams;

        ret = TMR_SR_cmdGetProtocolConfiguration(reader, protokey.protocol, 
            protokey, &getBapParams);

        if (TMR_SUCCESS != ret)
        {
          /* throw the error and come out from the loop */
          break;
        }
        else
        {
          /*
           * Modify the get BAP structure if either of the set BAP params are -1
           */
          sr->isBapEnabled = true;

          if (-1 == bap->powerUpDelayUs)
          {
            bap->powerUpDelayUs = getBapParams.powerUpDelayUs;
          }
          if (-1 == bap->freqHopOfftimeUs)
          {
            bap->freqHopOfftimeUs = getBapParams.freqHopOfftimeUs;
          }
        }
      }
      // No break -- fall through to regular Gen2 param handler
    }
#endif /* ENABLE_TMR_PARAM_GEN2_BAP */

  case TMR_PARAM_GEN2_Q:
  case TMR_PARAM_GEN2_TAGENCODING:
  case TMR_PARAM_GEN2_SESSION:
  case TMR_PARAM_GEN2_TARGET:
  case TMR_PARAM_GEN2_BLF:
  case TMR_PARAM_GEN2_TARI:
  case TMR_PARAM_GEN2_T4:
  case TMR_PARAM_GEN2_INITIAL_Q:
  case TMR_PARAM_GEN2_SEND_SELECT:
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_ISO180006B
  case TMR_PARAM_ISO180006B_BLF:
  case TMR_PARAM_ISO180006B_MODULATION_DEPTH:
  case TMR_PARAM_ISO180006B_DELIMITER:
#endif /* TMR_ENABLE_ISO180006B */
#ifdef TMR_ENABLE_HF_LF
  case TMR_PARAM_ISO14443A_TAGTYPE:
  case TMR_PARAM_ISO14443B_TAGTYPE:
  case TMR_PARAM_ISO15693_TAGTYPE:
  case TMR_PARAM_LF125KHZ_TAGTYPE:
  case TMR_PARAM_LF134KHZ_TAGTYPE:
  case TMR_PARAM_LF125KHZ_SECURE_RD_FORMAT:
#endif /* TMR_ENABLE_HF_LF */

    ret = TMR_SR_cmdSetProtocolConfiguration(reader, protokey.protocol, 
                                             protokey, value);
    break;

  default:
    ;
  }
  return ret;
}

static TMR_Status
TMR_SR_paramGet(struct TMR_Reader *reader, TMR_Param key, void *value)
{
  TMR_Status ret;
  TMR_SR_Configuration readerkey;
  TMR_SR_ProtocolConfiguration protokey;
  TMR_SR_SerialReader *sr;

  ret = TMR_SUCCESS;
  sr = &reader->u.serialReader;
#ifdef TMR_ENABLE_UHF
  readerkey = TMR_SR_CONFIGURATION_TRANSMIT_POWER_SAVE;
  protokey.protocol = TMR_TAG_PROTOCOL_GEN2;
#endif /* TMR_ENABLE_UHF */
  if (BITGET(sr->paramConfirmed, key) && 0 == BITGET(sr->paramPresent, key))
  {
    return TMR_ERROR_NOT_FOUND;
  }

  switch (key)
  {
#if ENABLE_TMR_PARAM_BAUDRATE
  case TMR_PARAM_BAUDRATE:
    *(uint32_t *)value = sr->baudRate;
    break;
#endif /* ENABLE_TMR_PARAM_BAUDRATE */
#if ENABLE_TMR_PARAM_PROBEBAUDRATES
  case TMR_PARAM_PROBEBAUDRATES:
    {
      TMR_uint32List *uint32List;
      uint8_t i;

      uint32List = value;

      if (sr->probeBaudRates.len > uint32List->max)
      {
        sr->probeBaudRates.len = uint32List->max;
      }

      if (sr->probeBaudRates.len)
      {
        for(i = 0; i < sr->probeBaudRates.len; i++)
        {
          uint32List->list[i] = sr->probeBaudRates.list[i];
        }
        uint32List->len = sr->probeBaudRates.len;
      }
      else
      {
        //list is empty
      }
      break;
    }
#endif /* ENABLE_TMR_PARAM_PROBEBAUDRATES */
#if ENABLE_TMR_PARAM_URI
  case TMR_PARAM_URI:
    TMR_stringCopy((TMR_String *)value, reader->uri, (int)strlen(reader->uri));
    break;
#endif /*ENABLE_TMR_PARAM_URI*/
#if ENABLE_TMR_PARAM_COMMANDTIMEOUT
  case TMR_PARAM_COMMANDTIMEOUT:
    *(uint32_t *)value = sr->commandTimeout;
    break;
#endif  /* ENABLE_TMR_PARAM_COMMANDTIMEOUT */
#if ENABLE_TMR_PARAM_TRANSPORTTIMEOUT
  case TMR_PARAM_TRANSPORTTIMEOUT:
    *(uint32_t *)value = sr->transportTimeout;
    break;
#endif /*ENABLE_TMR_PARAM_TRANSPORTTIMEOUT*/
#if ENABLE_TMR_PARAM_REGION_ID
  case TMR_PARAM_REGION_ID:
    {
      if ((TMR_REGION_NONE == sr->regionId) && (reader->connected))
      {
        ret = TMR_SR_cmdGetRegion(reader, &sr->regionId);
      }
      *(TMR_Region *)value = sr->regionId;
    }
	break;
#endif /*ENABLE_TMR_PARAM_REGION_ID*/
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_RADIO_ENABLEPOWERSAVE
  case TMR_PARAM_RADIO_ENABLEPOWERSAVE:
	readerkey = TMR_SR_CONFIGURATION_TRANSMIT_POWER_SAVE;
	break;
#endif /*ENABLE_TMR_PARAM_RADIO_ENABLEPOWERSAVE*/
#if ENABLE_TMR_PARAM_RADIO_ENABLESJC
  case TMR_PARAM_RADIO_ENABLESJC:
    readerkey = TMR_SR_CONFIGURATION_SELF_JAMMER_CANCELLATION;
	break;
#endif /*ENABLE_TMR_PARAM_RADIO_ENABLESJC*/
#if ENABLE_TMR_PARAM_TAGREADDATA_READFILTERTIMEOUT
  case TMR_PARAM_TAGREADDATA_READFILTERTIMEOUT:
    {
      *(int32_t *)value = sr->readFilterTimeout;
    }
    break;
#endif /* ENABLE_TMR_PARAM_TAGREADDATA_READFILTERTIMEOUT */
#endif /* TMR_ENABLE_UHF */
#if ENABLE_TMR_PARAM_TAGREADDATA_ENABLEREADFILTER
  case TMR_PARAM_TAGREADDATA_ENABLEREADFILTER:
    {
#ifdef TMR_ENABLE_HF_LF
      ret = TMR_SR_cmdGetReaderConfiguration(reader, TMR_SR_CONFIGURATION_ENABLE_READ_FILTER, &(sr->enableReadFiltering));
#endif /* TMR_ENABLE_HF_LF */
      *(bool *)value = sr->enableReadFiltering;
    }
    break;
#endif /* ENABLE_TMR_PARAM_TAGREADDATA_ENABLEREADFILTER */
#if ENABLE_TMR_PARAM_RADIO_POWERMAX_POWERMIN
  case TMR_PARAM_RADIO_POWERMAX:
  case TMR_PARAM_RADIO_POWERMIN:
  {
    TMR_SR_PowerWithLimits power;

    ret = TMR_SR_cmdGetReadTxPowerWithLimits(reader, &power);
    if (TMR_SUCCESS != ret)
    {
      break;
    }

    switch(key)
    {
      case TMR_PARAM_RADIO_POWERMAX:
      {
        *(int16_t *)value = power.maxPower;
        break;
      }
      case TMR_PARAM_RADIO_POWERMIN:
      {
        *(int16_t *)value = power.minPower;
        break;
      }
      default:
        ret = TMR_ERROR_NOT_FOUND;
    }

    break;
  }
#endif /* ENABLE_TMR_PARAM_RADIO_POWERMAX */
#if ENABLE_TMR_PARAM_RADIO_READPOWER
  case TMR_PARAM_RADIO_READPOWER:
    ret = TMR_SR_cmdGetReadTxPower(reader, (int32_t *)value);
    break;
#endif /* ENABLE_TMR_PARAM_RADIO_READPOWER */
#if ENABLE_TMR_PARAM_RADIO_WRITEPOWER
  case TMR_PARAM_RADIO_WRITEPOWER:
    ret = TMR_SR_cmdGetWriteTxPower(reader, (int32_t *)value);
    break;
#endif /* ENABLE_TMR_PARAM_RADIO_WRITEPOWER */
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_ANTENNA_CHECKPORT
  case TMR_PARAM_ANTENNA_CHECKPORT:
    readerkey = TMR_SR_CONFIGURATION_SAFETY_ANTENNA_CHECK;
    break;
#endif /* ENABLE_TMR_PARAM_ANTENNA_CHECKPORT */
#if ENABLE_TMR_PARAM_TAGREADDATA_RECORDHIGHESTRSSI
  case TMR_PARAM_TAGREADDATA_RECORDHIGHESTRSSI:
    readerkey = TMR_SR_CONFIGURATION_RECORD_HIGHEST_RSSI;
    break;
#endif /* ENABLE_TMR_PARAM_TAGREADDATA_RECORDHIGHESTRSSI */
#if ENABLE_TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA
  case TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA:
    readerkey = TMR_SR_CONFIGURATION_UNIQUE_BY_ANTENNA;
    break;
#endif /* ENABLE_TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA */
#if ENABLE_TMR_PARAM_TAGREADDATA_UNIQUEBYDATA
  case TMR_PARAM_TAGREADDATA_UNIQUEBYDATA:
    readerkey = TMR_SR_CONFIGURATION_UNIQUE_BY_DATA;
    break;
#endif /* ENABLE_TMR_PARAM_TAGREADDATA_UNIQUEBYDATA */
#if ENABLE_TMR_PARAM_TAGREADDATA_UNIQUEBYPROTOCOL
  case TMR_PARAM_TAGREADDATA_UNIQUEBYPROTOCOL:
    ret = TMR_SR_cmdGetReaderConfiguration(reader, TMR_SR_CONFIGURATION_UNIQUE_BY_PROTOCOL, value);
    break;
#endif /* ENABLE_TMR_PARAM_TAGREADDATA_UNIQUEBYPROTOCOL */
#endif /* TMR_ENABLE_UHF */
#if ENABLE_TMR_PARAM_PRODUCT_GROUP_ID
  case TMR_PARAM_PRODUCT_GROUP_ID:
    readerkey = TMR_SR_CONFIGURATION_PRODUCT_GROUP_ID;
    break;
#endif /* ENABLE_TMR_PARAM_PRODUCT_GROUP_ID */
#if ENABLE_TMR_PARAM_PRODUCT_ID
  case TMR_PARAM_PRODUCT_ID:
    readerkey = TMR_SR_CONFIGURATION_PRODUCT_ID;
    break;
#endif /* ENABLE_TMR_PARAM_PRODUCT_ID */
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_STATUS_ENABLE_ANTENNAREPORT
  case TMR_PARAM_STATUS_ENABLE_ANTENNAREPORT:
    *(bool *)value = (reader->streamStats & TMR_SR_STATUS_ANTENNA) ? true : false;
    break;
#endif /* ENABLE_TMR_PARAM_STATUS_ENABLE_ANTENNAREPORT */
#if ENABLE_TMR_PARAM_STATUS_ENABLE_FREQUENCYREPORT
  case TMR_PARAM_STATUS_ENABLE_FREQUENCYREPORT:
    *(bool *)value = (reader->streamStats & TMR_SR_STATUS_FREQUENCY) ? true : false;
    break;
#endif /* ENABLE_TMR_PARAM_STATUS_ENABLE_FREQUENCYREPORT */
#if ENABLE_TMR_PARAM_STATUS_ENABLE_TEMPERATUREREPORT
  case TMR_PARAM_STATUS_ENABLE_TEMPERATUREREPORT:
    *(bool *)value = (reader->streamStats & TMR_SR_STATUS_TEMPERATURE) ? true : false;
    break;
#endif /* ENABLE_TMR_PARAM_STATUS_ENABLE_TEMPERATUREREPORT */
#endif /* TMR_ENABLE_UHF */
#if ENABLE_TMR_PARAM_ANTENNA_PORTSWITCHGPOS
  case TMR_PARAM_ANTENNA_PORTSWITCHGPOS:
  {
    uint8_t i = 0;
    TMR_uint8List *u8list;
    u8list = value;

    ret = TMR_SR_cmdGetReaderConfiguration(
	reader, TMR_SR_CONFIGURATION_ANTENNA_CONTROL_GPIO, &reader->portmask);
    if (TMR_SUCCESS != ret)
    {
      break;
    }
    u8list->len = 0;
    if(reader->portmask == 0)
    {
      LISTAPPEND(u8list, 0);
    }

    for(i = 0; i < 4; i++)
    {
      if ((reader->portmask >> i) & 1)
      {
        LISTAPPEND(u8list, i + 1);
      }
    }
    break;
  }
#endif /* ENABLE_TMR_PARAM_ANTENNA_PORTSWITCHGPOS */
#if ENABLE_TMR_PARAM_TRIGGER_READ_GPI
  case TMR_PARAM_TRIGGER_READ_GPI:
    {
      uint8_t portmask, i;
      TMR_uint8List *u8list;

      u8list = value;

      ret = TMR_SR_cmdGetReaderConfiguration(
        reader, TMR_SR_CONFIGURATION_TRIGGER_READ_GPIO, &portmask);
      if (TMR_SUCCESS != ret)
      {
        break;
      }
      u8list->len = 0;

      for (i = 0; i < 4; i++)
      {
        if (portmask & (1 << i))
        {
          LISTAPPEND(u8list, (i + 1));
        }
      }
      break;
    }
#endif /* ENABLE_TMR_PARAM_TRIGGER_READ_GPI */
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_ANTENNA_SETTLINGTIMELIST
  case TMR_PARAM_ANTENNA_SETTLINGTIMELIST:
    ret = getPortValues(reader, value, 2);
    break;
#endif /* ENABLE_TMR_PARAM_ANTENNA_SETTLINGTIMELIST */
#if ENABLE_TMR_PARAM_RADIO_PORTREADPOWERLIST
  case TMR_PARAM_RADIO_PORTREADPOWERLIST:
    ret = getPortValues(reader, value, 0);
    break;
#endif /* ENABLE_TMR_PARAM_RADIO_PORTREADPOWERLIST */
#if ENABLE_TMR_PARAM_RADIO_PORTWRITEPOWERLIST
  case TMR_PARAM_RADIO_PORTWRITEPOWERLIST:
    ret = getPortValues(reader, value, 1);
    break;
#endif /* ENABLE_TMR_PARAM_RADIO_PORTWRITEPOWERLIST */
#if ENABLE_TMR_PARAM_ANTENNA_RETURNLOSS
  case TMR_PARAM_ANTENNA_RETURNLOSS:
    ret = TMR_SR_cmdGetAntennaReturnLoss(reader, value);
    break;
#endif /* ENABLE_TMR_PARAM_ANTENNA_RETURNLOSS */
#endif /* TMR_ENABLE_UHF */
#if ENABLE_TMR_PARAM_GPIO_INPUTLIST_OUTPUTLIST
  case TMR_PARAM_GPIO_INPUTLIST:
  case TMR_PARAM_GPIO_OUTPUTLIST:
  {
    TMR_uint8List *u8list;
    u8list = value;
    u8list->len = 0;

    {
      int pin, wantout;
      bool out;
      TMR_GpioPin pinStates[16];
      uint8_t numPins;

      //sr->gpioDirections = -1;
      numPins = numberof(pinStates);
      ret = TMR_SR_cmdGetGPIO(reader, &numPins, pinStates);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
 
      wantout = (key == TMR_PARAM_GPIO_OUTPUTLIST) ? 1 : 0;
      //if (-1 == sr->gpioDirections)
      {
        /* Cache the current state */
        sr->gpioDirections = 0;
        for (pin = 1; pin <= numPins ; pin++)
        {
          ret = TMR_SR_cmdGetGPIODirection(reader, pin, &out);
          if (TMR_SUCCESS != ret)
          {
            return ret;
          }
          if (out)
          {
            sr->gpioDirections |= 1 << pin;
          }
        }
      }
      for (pin = 1; pin <= numPins ; pin++)
      {
        if (wantout == ((sr->gpioDirections >> pin) & 1))
        {
          LISTAPPEND(u8list, pin);
        }
      }
    }
    break;
  }
#endif /* ENABLE_TMR_PARAM_GPIO_INPUTLIST */
#if ENABLE_TMR_PARAM_ANTENNA_PORTLIST
  case TMR_PARAM_ANTENNA_PORTLIST:
  {
    uint8_t i;
    TMR_uint8List *u8list;

    u8list = value;

    u8list->len = 0;
    for (i = 0; i < reader->u.serialReader.txRxMap->len; i++)
    {
      LISTAPPEND(u8list, reader->u.serialReader.txRxMap->list[i].antenna);
    }
    break;
  }
#endif /* ENABLE_TMR_PARAM_ANTENNA_PORTLIST */
#if ENABLE_TMR_PARAM_ANTENNA_CONNECTEDPORTLIST
  case TMR_PARAM_ANTENNA_CONNECTEDPORTLIST:
  {
    // Store detected ports in array for quick lookup
    bool detected[TMR_SR_MAX_ANTENNA_PORTS+1];
    uint8_t i;
    TMR_uint8List *u8list;

    for (i=0; i<TMR_SR_MAX_ANTENNA_PORTS; i++)
    {
      detected[i] = false;
    }
    u8list = value;

    {
      TMR_SR_PortDetect ports[TMR_SR_MAX_ANTENNA_PORTS];
      uint8_t numPorts;

      numPorts = numberof(ports);
      ret = TMR_SR_cmdAntennaDetect(reader, &numPorts, ports);
      if (TMR_SUCCESS != ret)
      {
        break;
      }
      for (i=0; i<numPorts; i++)
      {
        detected[ports[i].port] = ports[i].detected;
      }
    }
    
    u8list->len = 0;
    for (i=0; i<reader->u.serialReader.txRxMap->len; i++)
    {
      int ant = reader->u.serialReader.txRxMap->list[i].antenna;
      int tx  = reader->u.serialReader.txRxMap->list[i].txPort;
      int rx  = reader->u.serialReader.txRxMap->list[i].rxPort;

      if (detected[tx] && detected[rx])
      {
        LISTAPPEND(u8list, ant);
      }
    }
    break;
  }
#endif /* ENABLE_TMR_PARAM_ANTENNA_CONNECTEDPORTLIST */
#if ENABLE_TMR_PARAM_ANTENNA_TXRXMAP
  case TMR_PARAM_ANTENNA_TXRXMAP:
  {
    TMR_AntennaMapList *map, *mymap;
    uint16_t i;

    map = value;
    mymap = sr->txRxMap;

    for (i = 0 ; i < mymap->len && i < map->max ; i++)
      map->list[i] = mymap->list[i];
    map->len = mymap->len;
    break;
  }
#endif /* ENABLE_TMR_PARAM_ANTENNA_TXRXMAP */
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_REGION_HOPTABLE
  case TMR_PARAM_REGION_HOPTABLE:
  {
    TMR_uint32List *u32List;
    uint8_t count;

    u32List = value;

    count = (uint8_t)u32List->max;
    ret = TMR_SR_cmdGetFrequencyHopTable(reader,&count, u32List->list);
    if (TMR_SUCCESS != ret)
    {
      break;
    }
    u32List->len = count;
    break;
  }
#endif /* ENABLE_TMR_PARAM_REGION_HOPTABLE */
#if ENABLE_TMR_PARAM_REGION_HOPTIME
  case TMR_PARAM_REGION_HOPTIME:
    ret = TMR_SR_cmdGetFrequencyHopTime(reader, value);
    break;
#endif /* ENABLE_TMR_PARAM_REGION_HOPTIME */
#if ENABLE_TMR_PARAM_REGION_DWELL_TIME
  case  TMR_PARAM_REGION_DWELL_TIME:
    ret = TMR_SR_cmdGetRegionConfiguration(reader, TMR_SR_REGION_CONFIGURATION_DWELLTIME, value);
    break;
#endif /* ENABLE_TMR_PARAM_REGION_DWELL_TIME */
#if ENABLE_TMR_PARAM_REGION_DWELL_TIME_ENABLE
  case  TMR_PARAM_REGION_DWELL_TIME_ENABLE:
    ret = TMR_SR_cmdGetRegionConfiguration(reader, TMR_SR_REGION_CONFIGURATION_DWELLTIME_ENABLED, value);
    break;
#endif /* ENABLE_TMR_PARAM_REGION_DWELL_TIME_ENABLE */
#if ENABLE_TMR_PARAM_REGION_LBT_THRESHOLD
  case  TMR_PARAM_REGION_LBT_THRESHOLD:
    ret = TMR_SR_cmdGetRegionConfiguration(reader, TMR_SR_REGION_CONFIGURATION_LBTTHRESHOLD, value);
    break;
#endif /* ENABLE_TMR_PARAM_REGION_LBT_THRESHOLD */
#if ENABLE_TMR_PARAM_REGION_LBT_ENABLE
  case TMR_PARAM_REGION_LBT_ENABLE:
    ret = TMR_SR_cmdGetRegionConfiguration(reader,
                                           TMR_SR_REGION_CONFIGURATION_LBT_ENABLED, value);
    if (TMR_SUCCESS != ret && TMR_ERROR_IS_CODE(ret))
    {
      *(bool *)value = false;
      ret = TMR_SUCCESS;
    }
    break;
#endif /* ENABLE_TMR_PARAM_REGION_LBT_ENABLE */
#endif /* TMR_ENABLE_UHF */
#if ENABLE_TMR_PARAM_TAGOP_ANTENNA
  case TMR_PARAM_TAGOP_ANTENNA:
    *(uint8_t *)value = reader->tagOpParams.antenna;
    break;
#endif /* ENABLE_TMR_PARAM_TAGOP_ANTENNA */
#if ENABLE_TMR_PARAM_TAGOP_PROTOCOL
  case TMR_PARAM_TAGOP_PROTOCOL:
    *(TMR_TagProtocol *)value = reader->tagOpParams.protocol;
    break;
#endif /* ENABLE_TMR_PARAM_TAGOP_PROTOCOL*/
#if ENABLE_TMR_PARAM_POWERMODE
  case TMR_PARAM_POWERMODE:
    {
      TMR_SR_PowerMode pm;
      ret = TMR_SR_cmdGetPowerMode(reader, &pm);
      if (TMR_SUCCESS == ret)
      {
        sr->powerMode = pm;
      }
    }
    *(TMR_SR_PowerMode*)value = sr->powerMode;
    break;
#endif /* ENABLE_TMR_PARAM_POWERMODE */
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_USERMODE
  case TMR_PARAM_USERMODE:
    ret = TMR_SR_cmdGetUserMode(reader, value);
    break;
#endif /* ENABLE_TMR_PARAM_USERMODE*/
#if ENABLE_TMR_PARAM_GEN2_Q
  case TMR_PARAM_GEN2_Q:
    protokey.u.gen2 = TMR_SR_GEN2_CONFIGURATION_Q;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_Q */
#if ENABLE_TMR_PARAM_GEN2_TAGENCODING
  case TMR_PARAM_GEN2_TAGENCODING:
    protokey.u.gen2 = TMR_SR_GEN2_CONFIGURATION_TAGENCODING;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_TAGENCODING */
#if ENABLE_TMR_PARAM_GEN2_SESSION
  case TMR_PARAM_GEN2_SESSION:
    protokey.u.gen2 = TMR_SR_GEN2_CONFIGURATION_SESSION;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_SESSION */
#if ENABLE_TMR_PARAM_GEN2_TARGET
  case TMR_PARAM_GEN2_TARGET:
    protokey.u.gen2 = TMR_SR_GEN2_CONFIGURATION_TARGET;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_TARGET */
#if ENABLE_TMR_PARAM_GEN2_BLF
  case TMR_PARAM_GEN2_BLF:
    protokey.u.gen2 = TMR_SR_GEN2_CONFIGURATION_LINKFREQUENCY;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_BLF */
#if ENABLE_TMR_PARAM_GEN2_TARI
  case TMR_PARAM_GEN2_TARI:
    protokey.u.gen2 = TMR_SR_GEN2_CONFIGURATION_TARI;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_TARI */
#if ENABLE_TMR_PARAM_GEN2_BAP
  case TMR_PARAM_GEN2_BAP:
    protokey.u.gen2 = TMR_SR_GEN2_CONFIGURATION_BAP;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_BAP */
#if ENABLE_TMR_PARAM_GEN2_WRITEMODE
  case TMR_PARAM_GEN2_WRITEMODE:
    *(TMR_GEN2_WriteMode *)value = sr->writeMode;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_WRITEMODE */
#if ENABLE_TMR_PARAM_GEN2_PROTOCOLEXTENSION
  case TMR_PARAM_GEN2_PROTOCOLEXTENSION:
    protokey.u.gen2 = TMR_SR_GEN2_CONFIGURATION_PROTCOLEXTENSION;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_PROTOCOLEXTENSION */
#if ENABLE_TMR_PARAM_GEN2_T4
  case TMR_PARAM_GEN2_T4:
    protokey.u.gen2 = TMR_SR_GEN2_CONFIGURATION_T4;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_T4 */ 
#if ENABLE_TMR_PARAM_GEN2_INITIAL_Q
  case TMR_PARAM_GEN2_INITIAL_Q:
    protokey.u.gen2 = TMR_SR_GEN2_INITIAL_Q;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_INITIAL_Q */
#if ENABLE_TMR_PARAM_GEN2_SEND_SELECT
  case TMR_PARAM_GEN2_SEND_SELECT:
    protokey.u.gen2 = TMR_SR_GEN2_SEND_SELECT;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_SEND_SELECT */
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_ISO180006B
#if ENABLE_TMR_PARAM_ISO180006B_BLF
  case TMR_PARAM_ISO180006B_BLF:
    protokey.protocol = TMR_TAG_PROTOCOL_ISO180006B;
    protokey.u.iso180006b = TMR_SR_ISO180006B_CONFIGURATION_LINKFREQUENCY;
    break;
#endif /* ENABLE_TMR_PARAM_ISO180006B_BLF */
#if ENABLE_TMR_PARAM_ISO180006B_MODULATION_DEPTH
  case TMR_PARAM_ISO180006B_MODULATION_DEPTH:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_ISO180006B;
      protokey.u.iso180006b = TMR_SR_ISO180006B_CONFIGURATION_MODULATION_DEPTH;
      break;
    }
#endif /* ENABLE_TMR_PARAM_ISO180006B_MODULATION_DEPTH */
#if ENABLE_TMR_PARAM_ISO180006B_DELIMITER
  case TMR_PARAM_ISO180006B_DELIMITER:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_ISO180006B;
      protokey.u.iso180006b = TMR_SR_ISO180006B_CONFIGURATION_DELIMITER;
      break;
    }
#endif /* ENABLE_TMR_PARAM_ISO180006B_DELIMITER */
#endif /* TMR_ENABLE_ISO180006B */
#ifdef TMR_ENABLE_HF_LF
#if ENABLE_TMR_PARAM_PROTOCOL_LIST
  case TMR_PARAM_PROTOCOL_LIST:
    {
      ret = TMR_SR_cmdGetProtocolList(reader, value);
      break;
    }
#endif /* ENABLE_TMR_PARAM_PROTOCOL_LIST */
#if ENABLE_TMR_PARAM_ISO14443A_TAGTYPE
  case TMR_PARAM_ISO14443A_TAGTYPE:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_ISO14443A;
      protokey.u.iso14443a = TMR_SR_ISO14443A_CONFIGURATION_TAGTYPE;
      break;
    }
#endif /* ENABLE_TMR_PARAM_ISO14443A_TAGTYPE */
#if ENABLE_TMR_PARAM_ISO14443B_TAGTYPE
  case TMR_PARAM_ISO14443B_TAGTYPE:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_ISO14443B;
      protokey.u.iso14443b = TMR_SR_ISO14443B_CONFIGURATION_TAGTYPE;
      break;
    }
#endif /* ENABLE_TMR_PARAM_ISO14443B_TAGTYPE */
#if ENABLE_TMR_PARAM_ISO15693_TAGTYPE
  case TMR_PARAM_ISO15693_TAGTYPE:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_ISO15693;
      protokey.u.iso15693 = TMR_SR_ISO15693_CONFIGURATION_TAGTYPE;
      break;
    }
#endif /* ENABLE_TMR_PARAM_ISO15693_TAGTYPE */
#if ENABLE_TMR_PARAM_LF125KHZ_TAGTYPE
  case TMR_PARAM_LF125KHZ_TAGTYPE:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_LF125KHZ;
      protokey.u.lf125khz = TMR_SR_LF125KHZ_CONFIGURATION_TAGTYPE;
      break;
    }
#endif /* ENABLE_TMR_PARAM_LF125KHZ_TAGTYPE */
#if ENABLE_TMR_PARAM_LF134KHZ_TAGTYPE
  case TMR_PARAM_LF134KHZ_TAGTYPE:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_LF134KHZ;
      protokey.u.lf134khz = TMR_SR_LF134KHZ_CONFIGURATION_TAGTYPE;
      break;
    }
#endif /* ENABLE_TMR_PARAM_LF134KHZ_TAGTYPE */
#if ENABLE_TMR_PARAM_ISO14443A_SUPPORTED_TAGTYPES
  case TMR_PARAM_ISO14443A_SUPPORTED_TAGTYPES:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_ISO14443A;
      protokey.u.iso14443a = TMR_SR_ISO14443A_CONFIGURATION_SUPPORTED_TAGTYPES;
      break;
    }
#endif /* ENABLE_TMR_PARAM_ISO14443A_SUPPORTED_TAGTYPES */
#if ENABLE_TMR_PARAM_ISO14443B_SUPPORTED_TAGTYPES
  case TMR_PARAM_ISO14443B_SUPPORTED_TAGTYPES:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_ISO14443B;
      protokey.u.iso14443b = TMR_SR_ISO14443B_CONFIGURATION_SUPPORTED_TAGTYPES;
      break;
    }
#endif /* ENABLE_TMR_PARAM_ISO14443B_SUPPORTED_TAGTYPES */
#if ENABLE_TMR_PARAM_ISO15693_SUPPORTED_TAGTYPES
  case TMR_PARAM_ISO15693_SUPPORTED_TAGTYPES:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_ISO15693;
      protokey.u.iso15693 = TMR_SR_ISO15693_CONFIGURATION_SUPPORTED_TAGTYPES;
      break;
    }
#endif /* ENABLE_TMR_PARAM_ISO15693_SUPPORTED_TAGTYPES */
#if ENABLE_TMR_PARAM_LF125KHZ_SUPPORTED_TAGTYPES
  case TMR_PARAM_LF125KHZ_SUPPORTED_TAGTYPES:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_LF125KHZ;
      protokey.u.lf125khz = TMR_SR_LF125KHZ_CONFIGURATION_SUPPORTED_TAGTYPES;
      break;
    }
#endif /* ENABLE_TMR_PARAM_LF125KHZ_SUPPORTED_TAGTYPES */
#if ENABLE_TMR_PARAM_LF134KHZ_SUPPORTED_TAGTYPES
  case TMR_PARAM_LF134KHZ_SUPPORTED_TAGTYPES:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_LF134KHZ;
      protokey.u.lf134khz = TMR_SR_LF134KHZ_CONFIGURATION_SUPPORTED_TAGTYPES;
      break;
    }
#endif /* ENABLE_TMR_PARAM_LF134KHZ_SUPPORTED_TAGTYPES */
#if ENABLE_TMR_PARAM_ISO14443A_SUPPORTED_TAG_FEATURES
  case TMR_PARAM_ISO14443A_SUPPORTED_TAG_FEATURES:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_ISO14443A;
      protokey.u.iso14443a = TMR_SR_ISO14443A_CONFIGURATION_SUPPORTED_FEATURES;
      break;
    }
#endif /* ENABLE_TMR_PARAM_ISO14443A_SUPPORTED_TAG_FEATURES */
#if ENABLE_TMR_PARAM_ISO15693_SUPPORTED_TAG_FEATURES
  case TMR_PARAM_ISO15693_SUPPORTED_TAG_FEATURES:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_ISO15693;
      protokey.u.iso15693 = TMR_SR_ISO15693_CONFIGURATION_SUPPORTED_FEATURES;
      break;
    }
#endif /* ENABLE_TMR_PARAM_ISO15693_SUPPORTED_TAG_FEATURES */
#if ENABLE_TMR_PARAM_LF125KHZ_SUPPORTED_TAG_FEATURES
  case TMR_PARAM_LF125KHZ_SUPPORTED_TAG_FEATURES:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_LF125KHZ;
      protokey.u.lf125khz = TMR_SR_LF125KHZ_CONFIGURATION_SUPPORTED_FEATURES;
      break;
    }
#endif /* ENABLE_TMR_PARAM_LF125KHZ_SUPPORTED_TAG_FEATURES */
#if ENABLE_TMR_PARAM_LF125KHZ_SECURE_RD_FORMAT
  case TMR_PARAM_LF125KHZ_SECURE_RD_FORMAT:
    {
      protokey.protocol = TMR_TAG_PROTOCOL_LF125KHZ;
      protokey.u.lf125khz = TMR_SR_LF125KHZ_CONFIGURATION_SECURE_RD_FORMAT;
      break;
    }
#endif /* ENABLE_TMR_PARAM_LF125KHZ_SECURE_RD_FORMAT */
#if ENABLE_TMR_PARAM_RADIO_KEEP_RF_ON
  case TMR_PARAM_RADIO_KEEP_RF_ON:
    {
      readerkey = TMR_SR_CONFIGURATION_KEEP_RF_ON;
      break;
    }
#endif /* ENABLE_TMR_PARAM_RADIO_KEEP_RF_ON */
#endif /* TMR_ENABLE_HF_LF */
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_GEN2_ACCESSPASSWORD
  case TMR_PARAM_GEN2_ACCESSPASSWORD:
    *(TMR_GEN2_Password *)value = sr->gen2AccessPassword;
    break;
#endif /* ENABLE_TMR_PARAM_GEN2_ACCESSPASSWORD */
#endif /* TMR_ENABLE_UHF */
#if ENABLE_TMR_PARAM_REGION_SUPPORTEDREGIONS
  case TMR_PARAM_REGION_SUPPORTEDREGIONS:
    ret = TMR_SR_cmdGetAvailableRegions(reader, value);
    break;
#endif /* ENABLE_TMR_PARAM_REGION_SUPPORTEDREGIONS */
#if ENABLE_TMR_PARAM_VERSION_SUPPORTEDPROTOCOLS
  case TMR_PARAM_VERSION_SUPPORTEDPROTOCOLS:
    ret = TMR_SR_cmdGetAvailableProtocols(reader, value);
    break;
#endif /* ENABLE_TMR_PARAM_VERSION_SUPPORTEDPROTOCOLS */
#if ENABLE_TMR_PARAM_RADIO_TEMPERATURE
  case TMR_PARAM_RADIO_TEMPERATURE:
    ret = TMR_SR_cmdGetTemperature(reader, value);
    break;
#endif /* ENABLE_TMR_PARAM_RADIO_TEMPERATURE */
#if ENABLE_TMR_PARAM_VERSION_HARDWARE
  case TMR_PARAM_VERSION_HARDWARE:
    ret = getHardwareInfo(reader, value);
    break;
#endif /* ENABLE_TMR_PARAM_VERSION_HARDWARE */
#if ENABLE_TMR_PARAM_VERSION_SERIAL
  case TMR_PARAM_VERSION_SERIAL:
    ret = getSerialNumber(reader, value);
    break;
#endif /* ENABLE_TMR_PARAM_VERSION_SERIAL */
#if ENABLE_TMR_PARAM_VERSION_MODEL
  case TMR_PARAM_VERSION_MODEL:
  {
    const char *model = NULL;

    switch (sr->versionInfo.hardware[0])
    {
#ifdef TMR_ENABLE_UHF
    case TMR_SR_MODEL_M6E:
      model = "M6e";
      break;
    case TMR_SR_MODEL_M6E_I:
      {
        /**
        * M6e I has following variants
        **/
        switch (sr->versionInfo.hardware[3])
        {
          case TMR_SR_MODEL_M6E_I_PRC:
            model = "M6e PRC";
            break;
          case TMR_SR_MODEL_M6E_I_JIC:
            model = "M6e JIC";
            break;
          default:
            model = "M6e";// If there is no model recognized, it should fallback to base model.
            break;
        }
      }
      break;
    case TMR_SR_MODEL_MICRO:
      {
        /**
         * Micro has following variants.
         **/
        switch (sr->versionInfo.hardware[3])
        {
          case TMR_SR_MODEL_M6E_MICRO_USB:
            model = "M6e Micro USB";
            break;
          case TMR_SR_MODEL_M6E_MICRO_USB_PRO:
            model = "M6e Micro USBPro";
            break;
          case TMR_SR_MODEL_M6E_MICRO:
          default:
            model = "M6e Micro";// If there is no model recognized, it should fallback to base model.
            break;
        }
      }
      break;
    case TMR_SR_MODEL_M6E_NANO:
      model = "M6e Nano";
      break;
    case TMR_SR_MODEL_M7E_PICO:
    {
      model = "M7e Pico";
      break;
    }
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_HF_LF
    case TMR_SR_MODEL_M3E:
      {
        /**
         * M3e has following variants
         **/
        switch (sr->versionInfo.hardware[3])
        {
          case TMR_SR_MODEL_M3E_REV1:
          default:
            model = "M3e";// If there is no model recognized, it should fallback to base model.
            break;
        }
        break;
      }
#endif /* TMR_ENABLE_HF_LF */
    default:
      model = "Unknown";
    }
    TMR_stringCopy(value, model, (int)strlen(model));
    break;
  }    
#endif /* ENABLE_TMR_PARAM_VERSION_MODEL */
#if ENABLE_TMR_PARAM_VERSION_SOFTWARE
  case TMR_PARAM_VERSION_SOFTWARE:
  {
    char tmp[38];
    TMR_SR_VersionInfo *info;

    info = &sr->versionInfo;

    TMR_hexDottedQuad(info->fwVersion, tmp);
    tmp[11] = '-';
    TMR_hexDottedQuad(info->fwDate, tmp + 12);
    tmp[23] = '-';
    tmp[24] = 'B';
    tmp[25] = 'L';
    TMR_hexDottedQuad(info->bootloader, tmp+26);
    TMR_stringCopy(value, tmp, 37);
    break;
  }
#endif /* ENABLE_TMR_PARAM_VERSION_SOFTWARE */
#if ENABLE_TMR_PARAM_LICENSE_KEY
  case TMR_PARAM_LICENSE_KEY:
    ret = TMR_ERROR_UNSUPPORTED;
	break;
#endif /* ENABLE_TMR_PARAM_LICENSE_KEY */
#if ENABLE_TMR_PARAM_MANAGE_LICENSE_KEY
  case TMR_PARAM_MANAGE_LICENSE_KEY:
    ret = TMR_ERROR_UNSUPPORTED;
    break;
#endif /* ENABLE_TMR_PARAM_MANAGE_LICENSE_KEY */
#if ENABLE_TMR_PARAM_USER_CONFIG
  case TMR_PARAM_USER_CONFIG:
    ret = TMR_ERROR_UNSUPPORTED;
	break;
#endif /* ENABLE_TMR_PARAM_USER_CONFIG */
#if ENABLE_TMR_PARAM_READER_STATS
  case TMR_PARAM_READER_STATS:
  {
    if ((NULL != reader->pSupportsResetStats) && (false == *(reader->pSupportsResetStats)))
    {
      /* Command not supported, pop up the error */
      return TMR_ERROR_UNSUPPORTED;
    }
    
    /**
     * We should ask for the fields which are requested by the user,
     * if no fields are requested by the user, then fetch all fields.
     */

    if (TMR_READER_STATS_FLAG_NONE == reader->statsFlag)
    {
      reader->statsFlag = reader->userStatsFlag;
    }
    ret = TMR_SR_cmdGetReaderStats(reader, reader->statsFlag, value);
    break;
  }
#endif /* ENABLE_TMR_PARAM_READER_STATS */
#if ENABLE_TMR_PARAM_READER_STATS_ENABLE
  case TMR_PARAM_READER_STATS_ENABLE:
  {
    if ((NULL != reader->pSupportsResetStats) && (false == *(reader->pSupportsResetStats)))
    {
      /* Command not supported, pop up the error */
      return TMR_ERROR_UNSUPPORTED;
    }

    *(TMR_Reader_StatsFlag *)value = reader->statsFlag;
    break;
  }
#endif /* ENABLE_TMR_PARAM_READER_STATS_ENABLE */
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_READER_STATISTICS
  case TMR_PARAM_READER_STATISTICS:
  ret = TMR_SR_cmdGetReaderStatistics(reader, TMR_SR_READER_STATS_ALL, value);
  break;
#endif /* ENABLE_TMR_PARAM_READER_STATISTICS */
#endif /* TMR_ENABLE_UHF */
#if ENABLE_TMR_PARAM_PRODUCT_GROUP
  case TMR_PARAM_PRODUCT_GROUP:
    {
      const char *group;
      TMR_SR_ProductGroupID id = (TMR_SR_ProductGroupID)sr->productId;
      switch (id)
      {
        case TMR_SR_PRODUCT_MODULE:
        case TMR_SR_PRODUCT_INVALID:
          group = "Embedded Reader";
          break;
#ifdef TMR_ENABLE_UHF
        case TMR_SR_PRODUCT_RUGGEDIZED_READER:
          group = "Ruggedized Reader";
          break;
        case TMR_SR_PRODUCT_USB_READER:
          group = "USB Reader";
          break;
#endif /* TMR_ENABLE_UHF */
        default:
          group = "Unknown";
          break;
      }
      if (NULL != value)
      {
        TMR_stringCopy(value, group, (int)strlen(group));
      }
      else
      {
        ret = TMR_ERROR_ILLEGAL_VALUE;
      }
      break;
    }
#endif /* ENABLE_TMR_PARAM_PRODUCT_GROUP */
#if ENABLE_TMR_PARAM_TAGREADATA_TAGOPSUCCESSCOUNT
  case TMR_PARAM_TAGREADATA_TAGOPSUCCESSCOUNT:
    {
      *(uint16_t *)value = sr->tagopSuccessCount;
      break;
    }
#endif /* ENABLE_TMR_PARAM_TAGREADATA_TAGOPSUCCESSCOUNT */
#if ENABLE_TMR_PARAM_TAGREADATA_TAGOPFAILURECOUNT
  case TMR_PARAM_TAGREADATA_TAGOPFAILURECOUNT:
    {
      *(uint16_t *)value = sr->tagopFailureCount;
      break;
    }
#endif /* TMR_PARAM_TAGREADATA_TAGOPFAILURECOUNT */
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_READER_WRITE_REPLY_TIMEOUT_EARLY_EXIT
  case TMR_PARAM_READER_WRITE_REPLY_TIMEOUT:
  case TMR_PARAM_READER_WRITE_EARLY_EXIT:
    {
      TMR_SR_Gen2ReaderWriteTimeOut timeout;
      ret = TMR_SR_cmdGetReaderWriteTimeOut(reader, protokey.protocol, &timeout);
      if (TMR_SUCCESS != ret)
      {
        break;
      }
      switch (key)
      {
      case TMR_PARAM_READER_WRITE_REPLY_TIMEOUT:
        {
          *(uint16_t *)value = (uint16_t)timeout.writetimeout;
          break;
        }
      case TMR_PARAM_READER_WRITE_EARLY_EXIT:
        {
          *(bool *)value = !((bool)timeout.earlyexit);
          break;
        }
      default:
        ret = TMR_ERROR_NOT_FOUND;
      }
    }
    break;
#endif /* ENABLE_TMR_PARAM_READER_WRITE_REPLY_TIMEOUT */
#endif /* TMR_ENABLE_UHF */
#if ENABLE_TMR_PARAM_METADATAFLAG
  case TMR_PARAM_METADATAFLAG:
      {
        *(TMR_TRD_MetadataFlag *)value = reader->userMetadataFlag;
        break;
      }
#endif /* ENABLE_TMR_PARAM_METADATAFLAG */
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_REGION_QUANTIZATION_STEP
  case TMR_PARAM_REGION_QUANTIZATION_STEP:
    ret = TMR_SR_cmdGetQuantizationStep(reader, value);
    break;
#endif /* ENABLE_TMR_PARAM_REGION_QUANTIZATION_STEP*/
#if ENABLE_TMR_PARAM_REGION_MINIMUM_FREQUENCY
  case TMR_PARAM_REGION_MINIMUM_FREQUENCY:
    ret = TMR_SR_cmdGetMinimumFrequency(reader, value);
    break;
#endif /* ENABLE_TMR_PARAM_REGION_MINIMUM_FREQUENCY */
#endif /* TMR_ENABLE_UHF */
#if !((!defined(TMR_ENABLE_UHF)) || defined(BARE_METAL))
#if ENABLE_TMR_PARAM_REGULATORY_MODE
  case TMR_PARAM_REGULATORY_MODE:
    {
      *(TMR_SR_RegulatoryMode *)value = reader->regulatoryParams.RegMode;
      break;
    }
#endif /* ENABLE_TMR_PARAM_REGULATORY_MODE */
#if ENABLE_TMR_PARAM_REGULATORY_MODULATION
  case TMR_PARAM_REGULATORY_MODULATION:
    {
      *(TMR_SR_RegulatoryModulation *)value = reader->regulatoryParams.RegModulation;
      break;
    }
#endif /* ENABLE_TMR_PARAM_REGULATORY_MODULATION */
#if ENABLE_TMR_PARAM_REGULATORY_ONTIME
  case TMR_PARAM_REGULATORY_ONTIME:
    {
      *(uint32_t *)value = reader->regulatoryParams.regOnTime;
      break;
    }
#endif /* ENABLE_TMR_PARAM_REGULATORY_ONTIME */
#if ENABLE_TMR_PARAM_REGULATORY_OFFTIME
  case TMR_PARAM_REGULATORY_OFFTIME:
    {
      *(uint32_t *)value = reader->regulatoryParams.regOffTime;
      break;
    }
#endif /* ENABLE_TMR_PARAM_REGULATORY_OFFTIME */
#if ENABLE_TMR_PARAM_REGULATORY_ENABLE
  case TMR_PARAM_REGULATORY_ENABLE:
    {
      ret = TMR_ERROR_UNSUPPORTED;
      break;
    }
#endif /* ENABLE_TMR_PARAM_REGULATORY_ENABLE */
#endif /* TMR_ENABLE_HF_LF || BARE_METAL */
#ifdef TMR_ENABLE_UHF
#if ENABLE_TMR_PARAM_PER_ANTENNA_TIME
  case TMR_PARAM_PER_ANTENNA_TIME:
  {
    ret = TMR_SR_cmdGetPerAntennaTime(reader, (TMR_PortValueList *)value);
    break;
  }
#endif /* ENABLE_TMR_PARAM_PER_ANTENNA_TIME */
#endif /* TMR_ENABLE_UHF */
  default:
    ret = TMR_ERROR_NOT_FOUND;
  }

  switch (key)
  {
#ifdef TMR_ENABLE_UHF
  case TMR_PARAM_ANTENNA_CHECKPORT:
  case TMR_PARAM_RADIO_ENABLEPOWERSAVE:
  case TMR_PARAM_RADIO_ENABLESJC:
  case TMR_PARAM_TAGREADDATA_RECORDHIGHESTRSSI:
  case TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA:
  case TMR_PARAM_TAGREADDATA_UNIQUEBYDATA:
#else
  case TMR_PARAM_RADIO_KEEP_RF_ON:
#endif /* TMR_ENABLE_UHF */ 
  case TMR_PARAM_PRODUCT_GROUP_ID:
  case TMR_PARAM_PRODUCT_ID:
	  ret = TMR_SR_cmdGetReaderConfiguration(reader, readerkey, value);
	  break;
#ifdef TMR_ENABLE_UHF
  case TMR_PARAM_GEN2_Q:
  case TMR_PARAM_GEN2_TAGENCODING:
  case TMR_PARAM_GEN2_SESSION:
  case TMR_PARAM_GEN2_TARGET:
  case TMR_PARAM_GEN2_BLF:
  case TMR_PARAM_GEN2_TARI:
  case TMR_PARAM_GEN2_BAP:
  case TMR_PARAM_GEN2_PROTOCOLEXTENSION:
  case TMR_PARAM_GEN2_T4:
  case TMR_PARAM_GEN2_INITIAL_Q:
  case TMR_PARAM_GEN2_SEND_SELECT:
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_ISO180006B
  case TMR_PARAM_ISO180006B_BLF:
  case TMR_PARAM_ISO180006B_MODULATION_DEPTH:
  case TMR_PARAM_ISO180006B_DELIMITER:
#endif /* TMR_ENABLE_ISO180006B */
#ifdef TMR_ENABLE_HF_LF
  case TMR_PARAM_ISO14443A_TAGTYPE:
  case TMR_PARAM_ISO14443B_TAGTYPE:
  case TMR_PARAM_ISO15693_TAGTYPE:
  case TMR_PARAM_LF125KHZ_TAGTYPE:
  case TMR_PARAM_LF134KHZ_TAGTYPE:
  case TMR_PARAM_ISO14443A_SUPPORTED_TAGTYPES:
  case TMR_PARAM_ISO14443B_SUPPORTED_TAGTYPES:
  case TMR_PARAM_ISO15693_SUPPORTED_TAGTYPES:
  case TMR_PARAM_LF125KHZ_SUPPORTED_TAGTYPES:
  case TMR_PARAM_LF134KHZ_SUPPORTED_TAGTYPES:
  case TMR_PARAM_ISO14443A_SUPPORTED_TAG_FEATURES:
  case TMR_PARAM_ISO15693_SUPPORTED_TAG_FEATURES:
  case TMR_PARAM_LF125KHZ_SUPPORTED_TAG_FEATURES:
  case TMR_PARAM_LF125KHZ_SECURE_RD_FORMAT:
#endif /* TMR_ENABLE_HF_LF */
	  ret = TMR_SR_cmdGetProtocolConfiguration(reader, protokey.protocol, 
		  protokey, value);
	  break;

  default:
	  ;
  }

  if (0 == BITGET(sr->paramConfirmed, key))
  {
    if ((TMR_SUCCESS == ret) || ret == TMR_ERROR_AUTOREAD_ENABLED )
    {
      BITSET(sr->paramPresent, key);
    }
    BITSET(sr->paramConfirmed, key);
  }

  return ret;
}

TMR_Status
TMR_SR_SerialReader_init(TMR_Reader *reader)
{
  reader->readerType = TMR_READER_TYPE_SERIAL;

#ifndef TMR_ENABLE_SERIAL_READER_ONLY
  reader->connect = TMR_SR_connect;
  reader->destroy = TMR_SR_destroy;
  reader->read = TMR_SR_read;
  reader->hasMoreTags = TMR_SR_hasMoreTags;
  reader->getNextTag = TMR_SR_getNextTag;
  reader->executeTagOp = TMR_SR_executeTagOp;
  reader->readTagMemWords = TMR_SR_readTagMemWords;
  reader->readTagMemBytes = TMR_SR_readTagMemBytes;
  reader->writeTagMemBytes = TMR_SR_writeTagMemBytes;
  reader->writeTagMemWords = TMR_SR_writeTagMemWords;
  reader->writeTag = TMR_SR_writeTag;
  reader->killTag = TMR_SR_killTag;
  reader->lockTag = TMR_SR_lockTag;
  reader->gpiGet = TMR_SR_gpiGet;
  reader->gpoSet = TMR_SR_gpoSet;
#ifdef TMR_ENABLE_STDIO
  reader->firmwareLoad = TMR_SR_firmwareLoad;
#endif
  reader->modifyFlash = TMR_SR_modifyFlash;
  reader->reboot = TMR_SR_reboot;
#endif

  reader->cmdStopReading = TMR_SR_cmdStopReading;
#ifdef TMR_ENABLE_BACKGROUND_READS
  reader->cmdAutonomousReading = TMR_SR_receiveAutonomousReading;
#endif
  reader->paramSet = TMR_SR_paramSet;
  reader->paramGet = TMR_SR_paramGet;
 
  memset(reader->u.serialReader.paramConfirmed,0,
         sizeof(reader->u.serialReader.paramConfirmed));

  reader->u.serialReader.baudRate = 115200;
  reader->u.serialReader.isUserBaudRateSet = false;
  reader->u.serialReader.currentProtocol = TMR_TAG_PROTOCOL_NONE;
  reader->u.serialReader.versionInfo.hardware[0] = TMR_SR_MODEL_UNKNOWN;
  reader->u.serialReader.powerMode = TMR_SR_POWER_MODE_INVALID;
  reader->u.serialReader.transportTimeout = 5000;
  reader->u.serialReader.commandTimeout = 1000;
  reader->u.serialReader.regionId = TMR_REGION_NONE;
  reader->u.serialReader.tagsRemaining = 0;
  reader->u.serialReader.tagsRemainingInBuffer = 0;
  reader->u.serialReader.searchTimeoutMs = 0;
#ifdef TMR_ENABLE_UHF
  reader->u.serialReader.supportsPreamble = false;
  reader->u.serialReader.extendedEPC = false;
  reader->u.serialReader.gen2AccessPassword = 0;
  reader->u.serialReader.oldQ.type = TMR_SR_GEN2_Q_INVALID;
  reader->u.serialReader.writeMode = TMR_GEN2_WORD_ONLY;
  reader->u.serialReader.readFilterTimeout = 0;
  reader->u.serialReader.gen2AllMemoryBankEnabled = false;
  reader->u.serialReader.isBapEnabled = false;
#if TMR_ENABLE_CRC
  reader->u.serialReader.transportType = TMR_SR_MSG_SOURCE_UNKNOWN;
#endif /* TMR_ENABLE_CRC */
  reader->u.serialReader.crcEnabled = true;
#endif /* TMR_ENABLE_UHF */
  reader->u.serialReader.tagopSuccessCount = 0;
  reader->u.serialReader.tagopFailureCount = 0;
  reader->u.serialReader.enableReadFiltering = true;
  reader->u.serialReader.userEnableReadFiltering = false;
  reader->u.serialReader.usrTimeoutEnable = false;
  reader->u.serialReader.probeBaudRates.list = reader->u.serialReader.baudRates;
  reader->u.serialReader.probeBaudRates.max = TMR_MAX_PROBE_BAUDRATE_LENGTH;
  reader->u.serialReader.probeBaudRates.len = 0;
  reader->u.serialReader.enableAutonomousRead = false;
  reader->u.serialReader.isBasetimeUpdated = false;
  reader->u.serialReader.elapsedTime = 0;
  {
    int i = 0;

    //initialize the probe baud rate list with the supported  baud rate values
    uint32_t rates[TMR_MAX_PROBE_BAUDRATE_LENGTH] = {9600, 115200, 921600, 19200, 38400, 57600,230400, 460800};

    reader->u.serialReader.probeBaudRates.len = 8;
    for(i = 0; i < 8; i++)
    {
      reader->u.serialReader.probeBaudRates.list[i] = rates[i];
    }
  }

  return TMR_reader_init_internal(reader);
}

#ifdef TMR_ENABLE_UHF
uint16_t TMR_SR_getConfigWord(TMR_Reader *reader, TMR_TagOp_GEN2_NXP_Untraceable op)
{
	uint16_t configWord = 0x0000;
	uint16_t Mask;
	int Pos;

	if(op.epc == 1)
	{
		configWord |= 0x0001 << 14;
	}
	Mask = 0x1F;
	Pos = 9;
	configWord |= (uint16_t)((op.epcLength & Mask) << Pos);

	Mask = 0x03;
	Pos = 7;
	configWord |= (uint16_t)((op.tid & Mask) << Pos);

	if (op.userMemory == 1)
	{
		configWord |= 0x0001 << 6;
	}

	Mask = 0x03;
	Pos = 4;
	configWord |= (uint16_t)((op.range & Mask) << Pos);

	return configWord;
}
#endif /* TMR_ENABLE_UHF */

#if TMR_ENABLE_EXTENDED_TAGOPS
TMR_Status
getReadExtParams(TMR_ExtTagOp *op, TMR_uint8List *extParams)
{
  TMR_Status ret = TMR_SUCCESS;

  switch(op->tagType)
  {
#if TMR_ENABLE_DESFIRE_TAGOPS
    case TMR_ISO14443A_TAGTYPE_MIFARE_DESFIRE:
    {
      TMR_TagOp_DesFire_ReadData desFireReadOp;
      desFireReadOp = op->u.desfire.u.readData;

      //Append supOption.
      SETU8(extParams->list, extParams->len, desFireReadOp.subCmd);

      //Append Application ID.
      SETU24(extParams->list, extParams->len, desFireReadOp.appId);

      switch(desFireReadOp.subCmd)
      {
        case TMR_TAGOP_DESFIRE_CMD_READ:
        {
          //Append File ID.
          SETU8(extParams->list,  extParams->len, desFireReadOp.fileId);

          //Append File Type.
          SETU8(extParams->list,  extParams->len, desFireReadOp.fileType);

          break;
        }
        case TMR_TAGOP_DESFIRE_CMD_FILE_SETTINGS:
        {
          //Append File ID.
          SETU8(extParams->list, extParams->len, desFireReadOp.fileId);

          break;
        }
        case TMR_TAGOP_DESFIRE_CMD_AUTHENTICATE:
        case TMR_TAGOP_DESFIRE_CMD_KEY_VERSION:
        {
          //Append Key Number.
          SETU8(extParams->list, extParams->len, desFireReadOp.keyNo);

          break;
        }
        case TMR_TAGOP_DESFIRE_CMD_APP_ID:
        {
          //APP ID field is not required.
          extParams->len -= APPID_LEN;

          break;
        }
        case TMR_TAGOP_DESFIRE_CMD_FILE_ID:
        case TMR_TAGOP_DESFIRE_CMD_KEY_SETTINGS:
        {
          break;
        }
        default:
          //Invalid sub option.
          ret = TMR_ERROR_INVALID;
          break;
      }
      break;
    }
#endif /* TMR_ENABLE_DESFIRE_TAGOPS */

#if TMR_ENABLE_UL_NTAG_TAGOPS
    case TMR_ISO14443A_TAGTYPE_ULTRALIGHT_NTAG:
    case TMR_ISO14443A_TAGTYPE_NTAG:
    {
      TMR_TagOp_UL_Ntag_ReadData ulNtagReadOp;
      ulNtagReadOp = op->u.ulNtag.u.readData;

      //Append supOption.
      SETU8(extParams->list, extParams->len, ulNtagReadOp.subCmd);
      break;
    }
#endif /* TMR_ENABLE_UL_NTAG_TAGOPS */
    default:
      //Invalid tag-type.
      ret = TMR_ERROR_INVALID;
      break;
  }

  return ret;
}

TMR_Status
getWriteExtParams(TMR_ExtTagOp *op, TMR_uint8List *extParams)
{
  TMR_Status ret = TMR_SUCCESS;

  switch(op->tagType)
  {
#if TMR_ENABLE_DESFIRE_TAGOPS
    case TMR_ISO14443A_TAGTYPE_MIFARE_DESFIRE:
    {
      TMR_TagOp_DesFire_WriteData desFireWriteOp;
      desFireWriteOp = op->u.desfire.u.writeData; 

      //Append supOption.
      SETU8(extParams->list, extParams->len, desFireWriteOp.subCmd);

      //Append Application ID.
      SETU24(extParams->list, extParams->len, desFireWriteOp.appId);

      switch(desFireWriteOp.subCmd)
      {
        case TMR_TAGOP_DESFIRE_CMD_FILE_ID:
        case TMR_TAGOP_DESFIRE_CMD_WRITE:
        {
          //Append File ID.
          SETU8(extParams->list, extParams->len, desFireWriteOp.fileId);

          //Append File Type.
          SETU8(extParams->list, extParams->len, desFireWriteOp.fileType);

          if (TMR_TAGOP_DESFIRE_CMD_FILE_ID == desFireWriteOp.subCmd)
          {
            //Append File Size.
            SETU16(extParams->list, extParams->len, desFireWriteOp.fileSize);

            //Append Access Rights.
            SETU16(extParams->list, extParams->len, desFireWriteOp.accessRights);
          }

          break;
        }
        case TMR_TAGOP_DESFIRE_CMD_APP_ID:
        {
          //Append Key Settings.
          SETU8(extParams->list, extParams->len, desFireWriteOp.keySettings);

          //Append Number of Keys.
          SETU8(extParams->list, extParams->len, desFireWriteOp.numKeys);

          break;
        }
        case TMR_TAGOP_DESFIRE_CMD_FILE_SETTINGS:
        case TMR_TAGOP_DESFIRE_CMD_DELETE_FILE:
        {
          //Append File ID.
          SETU8(extParams->list, extParams->len, desFireWriteOp.fileId);
          break;
        }
        case TMR_TAGOP_DESFIRE_CMD_CHANGE_KEY:
        {
          //Append Old Key Number..
          SETU8(extParams->list, extParams->len, desFireWriteOp.keyNo);

          //Append New Key Number.
          SETU8(extParams->list, extParams->len, desFireWriteOp.newKeyNo);
          break;
        }
        case TMR_TAGOP_DESFIRE_CMD_FORMAT:
        {
          //APP ID field is not required.
          extParams->len -= APPID_LEN;
          break;
        }
        case TMR_TAGOP_DESFIRE_CMD_DELETE_APP:
        case TMR_TAGOP_DESFIRE_CMD_KEY_SETTINGS:
        {
          break;
        }
        default:
          //Invalid sub option.
          ret = TMR_ERROR_INVALID;
          break;
      }
      break;
    }
#endif /* TMR_ENABLE_DESFIRE_TAGOPS */
    default:
      //Invalid tag-type.
      ret = TMR_ERROR_INVALID;
      break;
  }

  return ret;
}

bool
isTagTypeFilterAvail(TMR_TagFilter *filter)
{
  if(filter != NULL)
  {
    if(filter->type != TMR_FILTER_TYPE_MULTI)
    {
      if(filter->type == TMR_FILTER_TYPE_TAGTYPE_SELECT)
      {
        return true;
      }
    }
    else
    {
      uint8_t i = 0;

      for(i = 0; i < filter->u.multiFilterList.len ; i++)
      {
        if(filter->u.multiFilterList.tagFilterList[i]->type == TMR_FILTER_TYPE_TAGTYPE_SELECT)
        {
          return true;
        }
      }
    }
  }

  return false;
}
#endif /* TMR_ENABLE_EXTENDED_TAGOPS */

TMR_Status
TMR_SR_executeTagOp(struct TMR_Reader *reader, TMR_TagOp *tagop, TMR_TagFilter *filter, TMR_uint8List *data)
{ 
  TMR_Status ret;
#ifdef TMR_ENABLE_UHF
  TMR_SR_SerialReader *sr;

  if(reader->featureFlags & TMR_READER_FEATURES_FLAG_MULTI_SELECT)
  {
    isMultiSelectEnabled = ((filter) && (filter->type == TMR_FILTER_TYPE_TAG_DATA)) ? false : true;
  }

  sr = &reader->u.serialReader;
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_HF_LF
  //Check for address byte extension flag.
  isAddrByteExtended = reader->featureFlags & TMR_READER_FEATURES_FLAG_ADDR_BYTE_EXTENSION;
#endif /* TMR_ENABLE_HF_LF */

  switch (tagop->type)
  {
#ifdef TMR_ENABLE_UHF
  case (TMR_TAGOP_GEN2_WRITETAG):
    {
      TMR_TagOp_GEN2_WriteTag op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      op = tagop->u.gen2.u.writeTag;

      ret = TMR_SR_cmdWriteGen2TagEpc(reader, filter, sr->gen2AccessPassword, 
        (uint16_t)(sr->commandTimeout),
        op.epcptr->epcByteCount,
        op.epcptr->epc,
        false);
    }
    break;
  case (TMR_TAGOP_GEN2_KILL):
    {
	  TMR_TagOp_GEN2_Kill op;
           
	  ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
	  if (TMR_SUCCESS != ret)
	  {
		return ret;
	  }

      op = tagop->u.gen2.u.kill;

      ret = TMR_SR_cmdKillTag(reader,
        (uint16_t)(sr->commandTimeout),
        op.password,
        filter);
    }
    break;
  case (TMR_TAGOP_GEN2_LOCK):
    {
	  TMR_TagOp_GEN2_Lock op;
           
	  ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
	  if (TMR_SUCCESS != ret)
	  {
		return ret;
	  }

      op = tagop->u.gen2.u.lock;

      ret = TMR_SR_cmdGEN2LockTag(reader, (uint16_t)sr->commandTimeout,
        op.mask, op.action, op.accessPassword, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_WRITEDATA):
    {
      TMR_TagOp_GEN2_WriteData op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      op = tagop->u.gen2.u.writeData;
      ret = TMR_SR_writeTagMemWords(reader, filter, op.bank, op.wordAddress, op.data.len, op.data.list);
    }
    break;
  case (TMR_TAGOP_GEN2_READDATA):
    {
      TMR_TagOp_GEN2_ReadData op;
      TMR_TagReadData read;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      if (NULL != data)
      {
        read.data.len = 0;
        read.data.list = data->list;
        read.data.max = data->max;
      }
      else
      {
        read.data.list = NULL;
      }
      read.metadataFlags = 0;
      op = tagop->u.gen2.u.readData;
      ret = TMR_SR_cmdGEN2ReadTagData(reader, (uint16_t)(sr->commandTimeout), op.bank,
                                      op.wordAddress, op.len, sr->gen2AccessPassword, filter, &read);
      if (NULL != data)
      {
        data->len = read.data.len;
      }
    }
    break;
  case (TMR_TAGOP_GEN2_BLOCKWRITE):
    {
      TMR_TagOp_GEN2_BlockWrite op;
           
	  ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
	  if (TMR_SUCCESS != ret)
	  {
		return ret;
      }

    op = tagop->u.gen2.u.blockWrite;
    ret = TMR_SR_cmdBlockWrite(reader,(uint16_t)sr->commandTimeout, op.bank,
            op.wordPtr, op.data.len, op.data.list, sr->gen2AccessPassword, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_BLOCKPERMALOCK):
    {
      TMR_TagOp_GEN2_BlockPermaLock op;
           
	  ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
	  if (TMR_SUCCESS != ret)
	  {
		return ret;
      }

      op = tagop->u.gen2.u.blockPermaLock;

      ret = TMR_SR_cmdBlockPermaLock(reader, (uint16_t)sr->commandTimeout, op.readLock, op.bank,
        op.blockPtr, op.mask.len, op.mask.list, sr->gen2AccessPassword, filter, data);
    }
    break;
  case (TMR_TAGOP_GEN2_BLOCKERASE):
    {
      TMR_TagOp_GEN2_BlockErase op;
      
      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      op = tagop->u.gen2.u.blockErase;
      ret = TMR_SR_cmdBlockErase(reader, (uint16_t)sr->commandTimeout, op.bank, op.wordPtr,
                                                op.wordCount, sr->gen2AccessPassword, filter);
    }
    break;
#ifdef TMR_ENABLE_GEN2_CUSTOM_TAGOPS
  case (TMR_TAGOP_GEN2_ALIEN_HIGGS2_PARTIALLOADIMAGE):
    {
      TMR_TagOp_GEN2_Alien_Higgs2_PartialLoadImage op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
      op = tagop->u.gen2.u.custom.u.alien.u.higgs2.u.partialLoadImage;
      if (op.epcptr->epcByteCount > 12 || op.epcptr->epcByteCount <=0 )
      { /* Only 96 bit epc */
        return TMR_ERROR_PROTOCOL_INVALID_EPC;
      }
      ret = TMR_SR_cmdHiggs2PartialLoadImage(reader, (uint16_t)sr->commandTimeout, 
        op.accessPassword, op.killPassword, op.epcptr->epcByteCount, op.epcptr->epc, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_ALIEN_HIGGS2_FULLLOADIMAGE):
    {
      TMR_TagOp_GEN2_Alien_Higgs2_FullLoadImage op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      op = tagop->u.gen2.u.custom.u.alien.u.higgs2.u.fullLoadImage;
      if (op.epcptr->epcByteCount > 12 || op.epcptr->epcByteCount <= 0 )
      { /* Only 96 bit epc */
        return TMR_ERROR_PROTOCOL_INVALID_EPC;
      }
      ret = TMR_SR_cmdHiggs2FullLoadImage(reader, (uint16_t)sr->commandTimeout, op.accessPassword,
        op.killPassword, op.lockBits, op.pcWord, op.epcptr->epcByteCount, op.epcptr->epc, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_ALIEN_HIGGS3_FASTLOADIMAGE):
    {
      /* The FastLoadImage command automatically erases the content of all
       * User Memory. If this is undesirable then use the LoadImage or multiple Write
       * Tag Data commands.
       */
      TMR_TagOp_GEN2_Alien_Higgs3_FastLoadImage op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      op = tagop->u.gen2.u.custom.u.alien.u.higgs3.u.fastLoadImage;
      if (op.epcptr->epcByteCount > 12 || op.epcptr->epcByteCount <= 0 )
      { /* Only 96 bit epc */
        return TMR_ERROR_PROTOCOL_INVALID_EPC;
      }
      ret = TMR_SR_cmdHiggs3FastLoadImage(reader, (uint16_t)sr->commandTimeout, op.currentAccessPassword, op.accessPassword,
        op.killPassword, op.pcWord, op.epcptr->epcByteCount, op.epcptr->epc, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_ALIEN_HIGGS3_LOADIMAGE):
    {
      TMR_TagOp_GEN2_Alien_Higgs3_LoadImage op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      op = tagop->u.gen2.u.custom.u.alien.u.higgs3.u.loadImage;
      if (op.epcAndUserData->len > 76 || op.epcAndUserData->len <= 0 )
      { /* Only 76 byte epcAndUserData */
        return TMR_ERROR_MSG_INVALID_PARAMETER_VALUE;
      }
      ret = TMR_SR_cmdHiggs3LoadImage(reader, (uint16_t)sr->commandTimeout, op.currentAccessPassword,
        op.accessPassword, op.killPassword, op.pcWord, (uint8_t)op.epcAndUserData->len, op.epcAndUserData->list, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_ALIEN_HIGGS3_BLOCKREADLOCK):
    {
      TMR_TagOp_GEN2_Alien_Higgs3_BlockReadLock op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
      op = tagop->u.gen2.u.custom.u.alien.u.higgs3.u.blockReadLock;

      ret = TMR_SR_cmdHiggs3BlockReadLock(reader, (uint16_t)sr->commandTimeout, op.accessPassword, op.lockBits, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_NXP_SETREADPROTECT):
    {
      TMR_TagOp_GEN2_NXP_SetReadProtect op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
      
      op = tagop->u.gen2.u.custom.u.nxp.u.setReadProtect;

      ret = TMR_SR_cmdNxpSetReadProtect(reader, (uint16_t)sr->commandTimeout,
        tagop->u.gen2.u.custom.chipType, op.accessPassword, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_NXP_RESETREADPROTECT):
    {
      TMR_TagOp_GEN2_NXP_ResetReadProtect op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
      
      op = tagop->u.gen2.u.custom.u.nxp.u.resetReadProtect;
      ret = TMR_SR_cmdNxpResetReadProtect(reader, (uint16_t)sr->commandTimeout,
          tagop->u.gen2.u.custom.chipType, op.accessPassword, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_NXP_CHANGEEAS):
    {
      TMR_TagOp_GEN2_NXP_ChangeEAS op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      op = tagop->u.gen2.u.custom.u.nxp.u.changeEAS;
      ret = TMR_SR_cmdNxpChangeEas(reader, (uint16_t)sr->commandTimeout, tagop->u.gen2.u.custom.chipType,
        op.accessPassword, op.reset, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_NXP_EASALARM):
    {
      TMR_TagOp_GEN2_NXP_EASAlarm op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      op = tagop->u.gen2.u.custom.u.nxp.u.EASAlarm;
      ret = TMR_SR_cmdNxpEasAlarm(reader, (uint16_t)sr->commandTimeout, tagop->u.gen2.u.custom.chipType,
        op.dr, op.m, op.trExt, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_NXP_CALIBRATE):
    {
      TMR_TagOp_GEN2_NXP_Calibrate op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
      
      op = tagop->u.gen2.u.custom.u.nxp.u.calibrate;
      ret = TMR_SR_cmdNxpCalibrate(reader, (uint16_t)sr->commandTimeout, tagop->u.gen2.u.custom.chipType,
        op.accessPassword, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_NXP_CHANGECONFIG):
    {
      TMR_TagOp_GEN2_NXP_ChangeConfig op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
       
      if(NULL != data)
      {
        data->len = 0;      
      }

      op = tagop->u.gen2.u.custom.u.nxp.u.changeConfig;
      ret = TMR_SR_cmdNxpChangeConfig(reader, (uint16_t)sr->commandTimeout, tagop->u.gen2.u.custom.chipType,
        op.accessPassword, op.configWord, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_NXP_UNTRACEABLE):
	{
	  TMR_TagOp_GEN2_NXP_Untraceable op;
	  uint16_t configWord;
	  ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
	  op = tagop->u.gen2.u.custom.u.nxp.u.untraceable;
	  configWord = TMR_SR_getConfigWord(reader,op);
	  ret = TMR_SR_cmdGen2v2NXPUntraceable(reader,(uint16_t)sr->commandTimeout, tagop->u.gen2.u.custom.chipType,
		 sr->gen2AccessPassword, configWord , op, data, filter);
	}
    break;
  case (TMR_TAGOP_GEN2_NXP_AUTHENTICATE):
	{
	  TMR_TagOp_GEN2_NXP_Authenticate op;

	  ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
	  op = tagop->u.gen2.u.custom.u.nxp.u.authenticate;
	  ret = TMR_SR_cmdGen2v2NXPAuthenticate(reader,(uint16_t)sr->commandTimeout, tagop->u.gen2.u.custom.chipType,
		 sr->gen2AccessPassword, op, data, filter);
	}
    break;
  case (TMR_TAGOP_GEN2_NXP_READBUFFER):
	{
	  TMR_TagOp_GEN2_NXP_Readbuffer op;

	  ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
	  op = tagop->u.gen2.u.custom.u.nxp.u.readBuffer;
	  ret = TMR_SR_cmdGen2v2NXPReadBuffer(reader,(uint16_t)sr->commandTimeout, tagop->u.gen2.u.custom.chipType,
		 sr->gen2AccessPassword, op, data, filter);
	}
    break;
  case (TMR_TAGOP_GEN2_IMPINJ_MONZA4_QTREADWRITE):
    {
      TMR_TagOp_GEN2_Impinj_Monza4_QTReadWrite op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.impinj.u.monza4.u.qtReadWrite;
      ret = TMR_SR_cmdMonza4QTReadWrite(reader, (uint16_t)sr->commandTimeout, op.accessPassword,
        op.controlByte, op.payload, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_IDS_SL900A_GETSENSOR):
    {
      TMR_TagOp_GEN2_IDS_SL900A_GetSensorValue op;

      /* Set the protocol for tag operation*/
      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.ids.u.sensor;
      ret = TMR_SR_cmdSL900aGetSensorValue(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode,
        op.Password,op.sl900A.level, op.sl900A.sensortype, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_IDS_SL900A_GETMEASUREMENTSETUP):
    {
      TMR_TagOp_GEN2_IDS_SL900A_GetMeasurementSetup op;

      /* Set the protocol for tag operation*/
      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.ids.u.measurementSetup;
      ret = TMR_SR_cmdSL900aGetMeasurementSetup(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode,
        op.Password,op.sl900A.level, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_IDS_SL900A_GETCALIBRATIONDATA):
    {
      TMR_TagOp_GEN2_IDS_SL900A_GetCalibrationData op;

      /* Set the protocol for tag operation*/
      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.ids.u.calibrationData;
      ret = TMR_SR_cmdSL900aGetCalibrationData(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode,
          op.Password, op.sl900A.level, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_IDS_SL900A_SETCALIBRATIONDATA):
    {
      TMR_TagOp_GEN2_IDS_SL900A_SetCalibrationData op;

      /* Set the protocol for tag operation*/
      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.ids.u.setCalibration;
      ret = TMR_SR_cmdSL900aSetCalibrationData(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode,
          op.Password, op.sl900A.level, op.cal.raw, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_IDS_SL900A_SETSFEPARAMETERS):
    {
      TMR_TagOp_GEN2_IDS_SL900A_SetSfeParameters op;

      /* Set the protocol for tag operation*/
      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.ids.u.setSfeParameters;
      ret = TMR_SR_cmdSL900aSetSfeParameters(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode,
          op.Password, op.sl900A.level, op.sfe->raw, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_IDS_SL900A_GETLOGSTATE):

    {
      TMR_TagOp_GEN2_IDS_SL900A_GetLogState op;

      /* Set the protocol for tag operation*/
      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.ids.u.getLog;
      ret = TMR_SR_cmdSL900aGetLogState(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode,
        op.Password, op.sl900A.level, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_IDS_SL900A_SETLOGMODE):
    {
      TMR_TagOp_GEN2_IDS_SL900A_SetLogMode op;

      /* Set the protocol for tag operation*/
      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.ids.u.setLogMode;
      ret = TMR_SR_cmdSL900aSetLogMode(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode,
        op.Password, op.sl900A.level, op.sl900A.dataLog, op.sl900A.rule, op.Ext1Enable, op.Ext2Enable, op.TempEnable,
        op.BattEnable, op.LogInterval, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_IDS_SL900A_INITIALIZE):
    {
      TMR_TagOp_GEN2_IDS_SL900A_Initialize op;
      
      /* Set the protocol for tag operation*/
      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.ids.u.initialize;

      ret = TMR_SR_cmdSL900aInitialize(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode,
        op.Password, op.sl900A.level, op.delayTime.raw, op.applicationData.raw, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_IDS_SL900A_ENDLOG):
    {
      TMR_TagOp_GEN2_IDS_SL900A_EndLog op;

      /* Set the protocol for tag operation*/
      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.ids.u.endLog;
      ret = TMR_SR_cmdSL900aEndLog(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode,
        op.Password,op.sl900A.level, filter);
    }
    break;   
  case (TMR_TAGOP_GEN2_IDS_SL900A_SETPASSWORD):
    {
      TMR_TagOp_GEN2_IDS_SL900A_SetPassword op;

      /* Set the protocol for tag operation*/
      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.ids.u.setPassword;
      ret = TMR_SR_cmdSL900aSetPassword(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode,
          op.Password,op.sl900A.level, op.NewPassword, op.NewPasswordLevel, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_IDS_SL900A_ACCESSFIFOSTATUS):
    {
      TMR_TagOp_GEN2_IDS_SL900A_AccessFifoStatus op;
      
      /* Set the protocol for tag operation*/
      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.ids.u.accessFifoStatus;
      ret = TMR_SR_cmdSL900aAccessFifoStatus(reader, (uint16_t)sr->commandTimeout, op.status.AccessPassword, op.status.CommandCode,
        op.status.Password, op.status.sl900A.level, op.status.operation, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_IDS_SL900A_ACCESSFIFOREAD):
    {
      TMR_TagOp_GEN2_IDS_SL900A_AccessFifoRead op;

      /* Set the protocol for tag operation*/
      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.ids.u.accessFifoRead;
      ret = TMR_SR_cmdSL900aAccessFifoRead(reader, (uint16_t)sr->commandTimeout, op.read.AccessPassword, op.read.CommandCode,
        op.read.Password, op.read.sl900A.level, op.read.operation,op.length, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_IDS_SL900A_ACCESSFIFOWRITE):
    {
      TMR_TagOp_GEN2_IDS_SL900A_AccessFifoWrite op;

      /* Set the protocol for tag operation*/
      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.ids.u.accessFifoWrite;
      ret = TMR_SR_cmdSL900aAccessFifoWrite(reader, (uint16_t)sr->commandTimeout, op.write.AccessPassword, op.write.CommandCode,
        op.write.Password, op.write.sl900A.level, op.write.operation, op.payLoad, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_IDS_SL900A_STARTLOG):
    {
      TMR_TagOp_GEN2_IDS_SL900A_StartLog op;
      
      /* Set the protocol for tag operation*/
      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }
      
      op = tagop->u.gen2.u.custom.u.ids.u.startLog;

      ret = TMR_SR_cmdSL900aStartLog(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode,
        op.Password, op.sl900A.level, op.startTime, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_IDS_SL900A_GETBATTERYLEVEL):
    {
      TMR_TagOp_GEN2_IDS_SL900A_GetBatteryLevel op;

      /* Set the protocol for tag operation*/
      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.ids.u.batteryLevel;

      ret = TMR_SR_cmdSL900aGetBatteryLevel(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode,
          op.Password, op.sl900A.level, op.batteryType, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_IDS_SL900A_SETLOGLIMITS):
    {
      TMR_TagOp_GEN2_IDS_SL900A_SetLogLimits op;

      /* Set the protocol for tag operation*/
      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.ids.u.setLogLimit;

      ret = TMR_SR_cmdSL900aSetLogLimit(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode,
          op.Password, op.sl900A.level, op.limit.extremeLower, op.limit.lower, op.limit.upper, op.limit.extremeUpper, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_IDS_SL900A_SETSHELFLIFE):
    {
      TMR_TagOp_GEN2_IDS_SL900A_SetShelfLife op;

      /* Set the protocol for tag operation*/
      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.ids.u.setShelfLife;

      ret = TMR_SR_cmdSL900aSetShelfLife(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode,
          op.Password, op.sl900A.level, op.shelfLifeBlock0->raw, op.shelfLifeBlock1->raw, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_DENATRAN_IAV_ACTIVATESECUREMODE):
    {
      TMR_TagOp_GEN2_Denatran_IAV_Activate_Secure_Mode op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.IavDenatran.u.secureMode;
      ret = TMR_SR_cmdIAVDenatranCustomOp(reader, (uint16_t)sr->commandTimeout, sr->gen2AccessPassword,
          op.mode, op.payload, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_DENATRAN_IAV_AUTHENTICATEOBU):
    {
      TMR_TagOp_GEN2_Denatran_IAV_Authenticate_OBU op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.IavDenatran.u.authenticateOBU;
      ret = TMR_SR_cmdIAVDenatranCustomOp(reader, (uint16_t)sr->commandTimeout, sr->gen2AccessPassword,
          op.mode, op.payload, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_ACTIVATE_SINIAV_MODE):
    {
      TMR_TagOp_GEN2_Denatran_IAV_Activate_Siniav_Mode op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.IavDenatran.u.activateSiniavMode;
      ret = TMR_SR_cmdIAVDenatranCustomActivateSiniavMode(reader, (uint16_t)sr->commandTimeout, sr->gen2AccessPassword,
        op.mode, op.payload, data, filter, op.isTokenDesc, op.token);
    }
    break;
  case (TMR_TAGOP_GEN2_OBU_AUTH_ID):
    {
      TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_ID op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthId;
      ret = TMR_SR_cmdIAVDenatranCustomOp(reader, (uint16_t)sr->commandTimeout, sr->gen2AccessPassword,
          op.mode, op.payload, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_AUTHENTICATE_OBU_FULL_PASS1):
    {
      TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_Full_Pass1 op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthFullPass1;
      ret = TMR_SR_cmdIAVDenatranCustomOp(reader, (uint16_t)sr->commandTimeout, sr->gen2AccessPassword,
          op.mode, op.payload, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_AUTHENTICATE_OBU_FULL_PASS2):
    {
      TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_Full_Pass2 op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthFullPass2;
      ret = TMR_SR_cmdIAVDenatranCustomOp(reader, (uint16_t)sr->commandTimeout, sr->gen2AccessPassword,
          op.mode, op.payload, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_OBU_READ_FROM_MEM_MAP):
    {
      TMR_TagOp_GEN2_Denatran_IAV_OBU_ReadFromMemMap op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.IavDenatran.u.obuReadFromMemMap;
      ret = TMR_SR_cmdIAVDenatranCustomReadFromMemMap(reader, (uint16_t)sr->commandTimeout, sr->gen2AccessPassword,
          op.mode, op.payload, data, filter, op.readPtr);
    }
    break;
  case (TMR_TAGOP_GEN2_DENATRAN_IAV_READ_SEC):
    {
      TMR_TagOp_GEN2_Denatran_IAV_Read_Sec op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.IavDenatran.u.readSec;
      ret = TMR_SR_cmdIAVDenatranCustomReadSec(reader, (uint16_t)sr->commandTimeout, sr->gen2AccessPassword,
          op.mode, op.payload, data, filter, op.readPtr);
    }
    break;
  case (TMR_TAGOP_GEN2_OBU_WRITE_TO_MEM_MAP):
    {

      TMR_TagOp_GEN2_Denatran_IAV_OBU_WriteToMemMap op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.IavDenatran.u.obuWriteToMemMap;
      ret = TMR_SR_cmdIAVDenatranCustomWriteToMemMap(reader, (uint16_t)sr->commandTimeout, sr->gen2AccessPassword,
        op.mode, op.payload, data, filter, op.writePtr, op.wordData,op.tagIdentification, op.dataBuf);
    }
    break;
  case (TMR_TAGOP_GEN2_DENATRAN_IAV_WRITE_SEC):
    {

      TMR_TagOp_GEN2_Denatran_IAV_Write_Sec op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.IavDenatran.u.writeSec;
      ret = TMR_SR_cmdIAVDenatranCustomWriteSec(reader, (uint16_t)sr->commandTimeout, sr->gen2AccessPassword,
          op.mode, op.payload, data, filter, op.dataWords, op.dataBuf);
    }
    break;
  case (TMR_TAGOP_GEN2_DENATRAN_IAV_GET_TOKEN_ID):
    {
      TMR_TagOp_GEN2_Denatran_IAV_Get_Token_Id op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if ( TMR_SUCCESS != ret)
      {
        return ret;
      }

      if (NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.IavDenatran.u.getTokenId;
      ret = TMR_SR_cmdIAVDenatranCustomGetTokenId(reader, (uint16_t)sr->commandTimeout, sr->gen2AccessPassword,
          op.mode, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_DENATRAN_IAV_AUTHENTICATE_OBU_FULL_PASS):
    {
      TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_Full_Pass op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthFullPass;
      ret = TMR_SR_cmdIAVDenatranCustomOp(reader, (uint16_t)sr->commandTimeout,
          sr->gen2AccessPassword, op.mode, op.payload, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_DENATRAN_IAV_G0_PA_OBU_AUTHENTICATE_ID):
    {
      TMR_TagOp_GEN2_Denatran_IAV_G0_PA_OBU_Auth_ID op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.IavDenatran.u.g0paobuauthid;
      ret = TMR_SR_cmdIAVDenatranCustomOp(reader, (uint16_t)sr->commandTimeout,
          sr->gen2AccessPassword, op.mode, op.payload, data, filter);
    }
    break;
  case (TMR_TAGOP_GEN2_IMPINJ_MONZA6_MARGINREAD):
    {
      TMR_TagOp_GEN2_Impinj_Monza6_MarginRead op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      op = tagop->u.gen2.u.custom.u.impinj.u.monza6.u.marginRead;
      ret = TMR_SR_cmdMonza6MarginRead(reader, (uint16_t)sr->commandTimeout, tagop->u.gen2.u.custom.chipType, 
          sr->gen2AccessPassword, op.bank, op.bitAddress, op.maskBitLength, op.mask , filter);
    }
    break;
    case (TMR_TAGOP_GEN2_NXP_UCODE7_CHANGECONFIG):
    {
      TMR_TagOp_GEN2_NXP_UCODE7_ChangeConfig op;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
       
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.nxp.u.ucode7ChangeConfig;
      ret = TMR_SR_cmdNxpUcode7ChangeConfig(reader, (uint16_t)sr->commandTimeout, tagop->u.gen2.u.custom.chipType,
        op.accessPassword, op.configWord, data, filter);
    }
    break;
    case (TMR_TagOP_GEN2_FUDAN_READREG):
    {
      TMR_TagOP_GEN2_FUDAN_ReadREG op;

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.fdn.u.ReadREG;
      ret = TMR_SR_cmdFDNReadREG(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode, op.RegAddress, filter, data);
    }
    break;
    case (TMR_TagOP_GEN2_FUDAN_WRITEREG) :
    {
      TMR_TagOP_GEN2_FUDAN_WriteREG op;
      
     /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.fdn.u.WriteREG;
      ret = TMR_SR_cmdFDNWriteREG(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode, op.RegAddress, op.RegWriteData, filter, data);
    }
    break;
    case (TMR_TagOP_GEN2_FUDAN_LOADREG):
    {
      TMR_TagOP_GEN2_FUDAN_LoadREG op;

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.fdn.u.LoadREG;
      ret = TMR_SR_cmdFDNLoadREG(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode, op.CmdCfg, filter, data);
    }
    break;
    case (TMR_TagOP_GEN2_FUDAN_STARTSTOPLOG):
    {
      TMR_TagOP_GEN2_FUDAN_StartStopLOG op;

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.fdn.u.StartStopLOG;
      ret = TMR_SR_cmdFDNStartStopLOG(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode, op.CmdCfg, op.FlagResetPassword, filter, data);
    }
    break;
    case (TMR_TagOP_GEN2_FUDAN_AUTH):
    {
      TMR_TagOP_GEN2_FUDAN_Auth op;

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.fdn.u.Auth;
      ret = TMR_SR_cmdFDNAuth(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode, op.CmdCfg, op.AuthPassword, filter, data);
    }
    break;
    case (TMR_TagOP_GEN2_FUDAN_READMEM):
    {
      TMR_TagOP_GEN2_FUDAN_ReadMEM op;

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.fdn.u.ReadMEM;
      ret = TMR_SR_cmdFDNReadMEM(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode, op.StartAddress, op.len, filter, data);
    }
    break;
    case (TMR_TagOP_GEN2_FUDAN_WRITEMEM):
    {
    TMR_TagOP_GEN2_FUDAN_WriteMEM op;

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.fdn.u.WriteMEM;
      ret = TMR_SR_cmdFDNWriteMEM(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode, op.StartAddress, op.data->len, op.data->list, filter, data);
    }
    break;
    case (TMR_TagOP_GEN2_FUDAN_STATECHECK):
    {
    TMR_TagOP_GEN2_FUDAN_StateCheck op;

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.fdn.u.StateCheck;
      ret = TMR_SR_cmdFDNStateCheck(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode, op.data->len, op.data->list, filter, data);
    }
    break;
    case (TMR_TagOP_GEN2_FUDAN_MEASURE):
    {
    TMR_TagOP_GEN2_FUDAN_Measure op;

      /* Do some error checking */
      if(NULL != data)
      {
        data->len = 0;
      }

      op = tagop->u.gen2.u.custom.u.fdn.u.Measure;
      ret = TMR_SR_cmdFDNMeasure(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode, op.CmdCfg, op.StoreBlockAddress, filter, data);
    }
    break;
    case (TMR_TagOP_GEN2_ILIAN_TAG_SELECT_COMMAND):
    {
      TMR_TagOP_GEN2_ILIAN_Tag_Select_Command op;

      op = tagop->u.gen2.u.custom.u.ilian.u.IlianTagSelect;
      ret = TMR_SR_cmdILIANTagSelect(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode, filter);
    }
    break;
    case (TMR_TagOP_GEN2_EM4325_GET_SENSOR_DATA):
    {
      TMR_TagOP_GEN2_EM4325_Get_Sensor_Data op;

      op = tagop->u.gen2.u.custom.u.emmicro.u.em4325.u.getSensorData;
      ret = TMR_SR_cmdEM4325GetSensorData(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode, filter, op.bitsToSet, data);
    }
    break;
    case (TMR_TagOP_GEN2_EM4325_RESET_ALARMS):
    {
      TMR_TagOP_GEN2_EM4325_Reset_Alarms op;

      op = tagop->u.gen2.u.custom.u.emmicro.u.em4325.u.resetAlarms;
      ret = TMR_SR_cmdEM4325ResetAlarms(reader, (uint16_t)sr->commandTimeout, op.AccessPassword, op.CommandCode, filter, op.fillValue);
    }
    break;
#endif /* TMR_ENABLE_GEN2_CUSTOM_TAGOPS */

#ifdef TMR_ENABLE_ISO180006B
  case (TMR_TAGOP_ISO180006B_READDATA):
    {
      TMR_TagOp_ISO180006B_ReadData op;
      TMR_TagReadData read;

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_ISO180006B);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      if (NULL != data)
      {
        read.data.max = data->max;
        read.data.len = 0;
        read.data.list = data->list;
      }
      else
      {
        read.data.list = NULL;
      }
      read.metadataFlags = 0;

      op = tagop->u.iso180006b.u.readData;
      ret = TMR_SR_cmdISO180006BReadTagData(reader, (uint16_t)sr->commandTimeout,
                op.byteAddress, op.len, filter, &read);
      if (NULL != data)
      {
        data->len = read.data.len;
      }
    }
    break;
  case (TMR_TAGOP_ISO180006B_WRITEDATA):
    {
      TMR_TagOp_ISO180006B_WriteData op;
           
	  ret = setProtocol(reader, TMR_TAG_PROTOCOL_ISO180006B);
	  if (TMR_SUCCESS != ret)
	  {
		return ret;
      }
      op = tagop->u.iso180006b.u.writeData;
      ret = TMR_SR_cmdISO180006BWriteTagData(reader, (uint16_t)(sr->commandTimeout), op.byteAddress,
        (uint8_t)op.data.len, op.data.list, filter);
    }
    break;
  case (TMR_TAGOP_ISO180006B_LOCK):
    {
      TMR_TagOp_ISO180006B_Lock op;
  
	  ret = setProtocol(reader, TMR_TAG_PROTOCOL_ISO180006B);
	  if (TMR_SUCCESS != ret)
	  {
	    return ret;
	  }

      op = tagop->u.iso180006b.u.lock;
      ret = TMR_SR_cmdISO180006BLockTag(reader, (uint16_t)(sr->commandTimeout), op.address, filter);
    }
    break;
#endif /* TMR_ENABLE_ISO180006B */

  case(TMR_TAGOP_LIST):
  {
    if(tagop->u.list.len ==1)
    {
      TMR_TagOp *op;     
      op = tagop->u.list.list[0];

      ret = TMR_SR_executeTagOp(reader, op, filter, data);	 
    }
    else if ((tagop->u.list.len == 2) && (TMR_TAGOP_GEN2_WRITEDATA == tagop->u.list.list[0]->type) && ( TMR_TAGOP_GEN2_READDATA == tagop->u.list.list[1]->type))
    {
      TMR_TagOp_GEN2_WriteData writeData;
      TMR_TagOp_GEN2_ReadData readData;
      TMR_TagOp *readOp,*writeOp;
      TMR_TagReadData read;

      writeOp = tagop->u.list.list[0];
      readOp = tagop->u.list.list[1];

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
      if (NULL != data)
      {
        read.data.len = 0;
        read.data.list = data->list;
        read.data.max = data->max;
      }
      else
      {
        read.data.list = NULL;
      }
      read.metadataFlags = 0;

      writeData = writeOp->u.gen2.u.writeData ;
      readData = readOp->u.gen2.u.readData;
      ret = TMR_SR_readAfterWriteTagMemWords(reader, writeData.bank, writeData.wordAddress * 2, writeData.data.list,
                                          	 writeData.data.len * 2, filter, readData.bank, readData.wordAddress, readData.len, &read);

      if (NULL != data)
      {
        data->len = read.data.len;
      }
    }
    else if ((tagop->u.list.len == 2) && (TMR_TAGOP_GEN2_WRITETAG == tagop->u.list.list[0]->type) && ( TMR_TAGOP_GEN2_READDATA == tagop->u.list.list[1]->type))
    {
      TMR_TagOp_GEN2_WriteTag writeTag;
      TMR_TagOp_GEN2_ReadData readData;
      TMR_TagOp *readOp,*writeOp;
      TMR_TagReadData read;

      writeOp = tagop->u.list.list[0];
      readOp = tagop->u.list.list[1];

      ret = setProtocol(reader, TMR_TAG_PROTOCOL_GEN2);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
      if (NULL != data)
      {
        read.data.len = 0;
        read.data.list = data->list;
        read.data.max = data->max;
      }
      else
      {
        read.data.list = NULL;
      }
      read.metadataFlags = 0;

      writeTag = writeOp->u.gen2.u.writeTag ;
      readData = readOp->u.gen2.u.readData;
      ret = TMR_SR_cmdReadAfterWriteGen2TagEpc(reader, filter, sr->gen2AccessPassword, (uint16_t)(sr->commandTimeout), writeTag.epcptr->epcByteCount,
                                               writeTag.epcptr->epc, false, readData.bank, readData.wordAddress, readData.len, &read);
      if (NULL != data)
      {
        data->len = read.data.len;
      }
    }
    else
    {
      ret = TMR_ERROR_UNIMPLEMENTED_FEATURE;
    }
  }
  break;
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_HF_LF
  case TMR_TAGOP_PASS_THROUGH:
    {
      TMR_TagOp_PassThrough op;
      op = tagop->u.passThrough;

      ret = TMR_SR_cmdPassThrough(reader, op.timeout, op.configFlags,
                                    op.buffer.len, op.buffer.list, data);
    }
    break;
  case TMR_TAGOP_READ_MEMORY:
    {
      TMR_ExtTagOp op;
#if TMR_ENABLE_EXTENDED_TAGOPS
      uint8_t extParamsData[255];
#endif /* TMR_ENABLE_EXTENDED_TAGOPS */
      TMR_uint8List tagOpExtParams;

      op = tagop->u.extTagOp;

      //Reset extended params buffer length.
      tagOpExtParams.len = 0;
#if TMR_ENABLE_EXTENDED_TAGOPS
      if (TMR_TAGOP_EXT_TAG_MEMORY == op.readMem.memType)
      {
        //Initialize extended params buffer.
        tagOpExtParams.list = extParamsData;
        tagOpExtParams.max = sizeof(extParamsData) / sizeof(extParamsData[0]);

        //Get the extended params for memory read.
        ret = getReadExtParams(&op, &tagOpExtParams);
        if (TMR_SUCCESS != ret)
        {
          return ret;
        }

        //Validate filter. Tag-Type filter is mandatory.
        if (!isTagTypeFilterAvail(filter))
        {
          return TMR_ERROR_INVALID;
        }
      }
#endif /* TMR_ENABLE_EXTENDED_TAGOPS */
      ret = TMR_SR_cmdReadMemory(reader, &op, filter, data, tagOpExtParams);
    }
    break;
  case TMR_TAGOP_WRITE_MEMORY:
    {
      TMR_ExtTagOp op;
#if TMR_ENABLE_EXTENDED_TAGOPS
#if TMR_ENABLE_DESFIRE_TAGOPS
      uint8_t extParamsData[255];
#endif /* TMR_ENABLE_DESFIRE_TAGOPS */
#endif /* TMR_ENABLE_EXTENDED_TAGOPS */
      TMR_uint8List tagOpExtParams;

      op = tagop->u.extTagOp;

      //Reset extended params buffer length.
      tagOpExtParams.len = 0;
#if TMR_ENABLE_EXTENDED_TAGOPS
#if TMR_ENABLE_DESFIRE_TAGOPS
      if (TMR_TAGOP_EXT_TAG_MEMORY == op.writeMem.memType)
      {
        //Initialize extended params buffer.
        tagOpExtParams.list = extParamsData;
        tagOpExtParams.max = sizeof(extParamsData) / sizeof(extParamsData[0]);

        //Get the extended params for memory Write.
        ret = getWriteExtParams(&op, &tagOpExtParams);
        if (TMR_SUCCESS != ret)
        {
          return ret;
        }

        //Validate filter. Tag-Type filter is mandatory.
        if (!isTagTypeFilterAvail(filter))
        {
          return TMR_ERROR_INVALID;
        }
      }
#endif /* TMR_ENABLE_DESFIRE_TAGOPS */
#endif /* TMR_ENABLE_EXTENDED_TAGOPS */
      ret = TMR_SR_cmdWriteMemory(reader, &op, filter, tagOpExtParams);
    }
    break;
#endif /* TMR_ENABLE_HF_LF */
  default:
    {
      ret = TMR_ERROR_UNIMPLEMENTED_FEATURE; 
    }
    break;
  }
#ifdef TMR_ENABLE_UHF
  isMultiSelectEnabled = false;
  isEmbeddedTagopEnabled = false;
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_HF_LF
  isAddrByteExtended = false;
#endif /* TMR_ENABLE_HF_LF */

  return ret;
}

/**
 * Internal method used for adding the tagop
 **/ 
TMR_Status
TMR_SR_addTagOp(struct TMR_Reader *reader, TMR_TagOp *tagop,TMR_ReadPlan *rp, uint8_t *msg, uint8_t *j, uint32_t readTimeMs, uint8_t *byte )
{
 
  TMR_Status ret = TMR_SUCCESS;
  uint8_t i, lenbyte;
#ifdef TMR_ENABLE_UHF
  bool customTagOp = false;
  TMR_SR_SerialReader *sr;
  sr = &reader->u.serialReader;
#endif /* TMR_ENABLE_UHF */

  i = *j; lenbyte = *byte;

#ifdef TMR_ENABLE_UHF
  if(TMR_SR_MODEL_M3E != sr->versionInfo.hardware[0])
  {
  switch(tagop->type)
  {
    case TMR_TAGOP_GEN2_WRITETAG:
    case TMR_TAGOP_GEN2_READDATA:
    case TMR_TAGOP_GEN2_WRITEDATA:
    case TMR_TAGOP_GEN2_LOCK:
    case TMR_TAGOP_GEN2_KILL:
    case TMR_TAGOP_GEN2_BLOCKWRITE:
    case TMR_TAGOP_GEN2_BLOCKPERMALOCK:
    case TMR_TAGOP_GEN2_BLOCKERASE:
    {
      ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
                                   | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
                                   rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);
      switch (tagop->type)
      {
        case (TMR_TAGOP_GEN2_WRITETAG):
        {
          TMR_TagOp_GEN2_WriteTag *args;
          args = &rp->u.simple.tagop->u.gen2.u.writeTag;

          TMR_SR_msgAddGEN2WriteTagEPC(msg,&i, 0, args->epcptr->epc, args->epcptr->epcByteCount);
          break;
        }
        case TMR_TAGOP_GEN2_READDATA:
        {
          TMR_TagOp_GEN2_ReadData *args;
          args = &rp->u.simple.tagop->u.gen2.u.readData;

          /**
           * If user wants to read all the memory bank data,
           * In that case args.bank value should be greater than 3
           **/
          if ((uint8_t)args->bank > 3)
          {
            /* enable the gen2AllMemoryBankEnabled option */
            sr->gen2AllMemoryBankEnabled = true;
          }

          TMR_SR_msgAddGEN2DataRead(msg, &i, 2000, args->bank, args->wordAddress, args->len, 0x00, false);
          break;
        }
        case TMR_TAGOP_GEN2_WRITEDATA:
        {
          TMR_TagOp_GEN2_WriteData *args;
          int idx ;

          args = &rp->u.simple.tagop->u.gen2.u.writeData;

          TMR_SR_msgAddGEN2DataWrite(msg, &i, 0, args->bank, args->wordAddress, false);

          for(idx = 0 ; idx< args->data.len; idx++)
          {
            msg[i++]= (args->data.list[idx]>>8) & 0xFF;
            msg[i++]= (args->data.list[idx]>>0) & 0xFF;
          }
          break;
        }
        case TMR_TAGOP_GEN2_LOCK:
        {
          TMR_TagOp_GEN2_Lock *args;
          args = &rp->u.simple.tagop->u.gen2.u.lock;

          TMR_SR_msgAddGEN2LockTag(msg, &i, 0, args->mask, args->action, args->accessPassword);
          break;
        }
        case TMR_TAGOP_GEN2_KILL:
        {
          TMR_TagOp_GEN2_Kill *args;
          args = &rp->u.simple.tagop->u.gen2.u.kill;

          TMR_SR_msgAddGEN2KillTag(msg, &i, 0, args->password);
          break;
        }
        case TMR_TAGOP_GEN2_BLOCKWRITE:
        {
          TMR_TagOp_GEN2_BlockWrite *args;
          args = &rp->u.simple.tagop->u.gen2.u.blockWrite;

         TMR_SR_msgAddGEN2BlockWrite(msg, &i, 0, args->bank, args->wordPtr, args->data.len, args->data.list, 0, NULL);
         break;
        }
        case TMR_TAGOP_GEN2_BLOCKPERMALOCK:
        {
          TMR_TagOp_GEN2_BlockPermaLock *args;
          args = &rp->u.simple.tagop->u.gen2.u.blockPermaLock;

          TMR_SR_msgAddGEN2BlockPermaLock(msg, &i, 0,args->readLock, args->bank, args->blockPtr, args->mask.len, 
            args->mask.list, 0, NULL);
           break;
        }
        case TMR_TAGOP_GEN2_BLOCKERASE:
        {
          TMR_TagOp_GEN2_BlockErase *args;
          args = &rp->u.simple.tagop->u.gen2.u.blockErase;

          TMR_SR_msgAddGEN2BlockErase(msg, &i, 0, args->wordPtr, args->bank, args->wordCount, 0, NULL);
          break;
        }
        default:
          break;
      }
      break;
    }
    default:
      customTagOp = true;
      break;
  }

  if(customTagOp)
  {
    switch (tagop->type)
    {
    case TMR_TAGOP_GEN2_SECURE_READDATA:
      {
        TMR_TagOp_GEN2_SecureReadData *args;
        uint8_t accessPassword[4];
        int index = 0;

        /* Enable the Secure ReadData option */
        isSecureAccessEnabled = true;

        args = &rp->u.simple.tagop->u.gen2.u.secureReadData;
        if (args->passwordType == TMR_SECURE_GEN2_LOOKUP_TABLE_PASSWORD)
        {
          /* Do this in case of look up table */
          accessPassword[index++] = args->password.secureAddressLength;
          accessPassword[index++] = args->password.secureAddressOffset;
          accessPassword[index++] = (args->password.secureFlashOffset >> 8);
          accessPassword[index++] = (args->password.secureFlashOffset & 0xFF);

          sr->gen2AccessPassword = (uint32_t)accessPassword[0] << 24 |
            (uint32_t)accessPassword[1] << 16 |
            (uint32_t)accessPassword[2] << 8  |
            (uint32_t)accessPassword[3];
        }
        else
        {
          /* Do this in case of Gen2 password */
          sr->gen2AccessPassword = args->password.gen2PassWord.u.gen2Password;
        }

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND | TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddGEN2DataRead(msg, &i, 2000, args->readData.bank, args->readData.wordAddress,
            args->readData.len,args->type, false);

				isSecureAccessEnabled = false;
        break;
      }
#ifdef TMR_ENABLE_GEN2_CUSTOM_TAGOPS
    case TMR_TAGOP_GEN2_ALIEN_HIGGS2_PARTIALLOADIMAGE:
      {
        TMR_TagOp_GEN2_Alien_Higgs2_PartialLoadImage *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.alien.u.higgs2.u.partialLoadImage;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddHiggs2PartialLoadImage(msg, &i, 0, args->accessPassword, args->killPassword, 
            args->epcptr->epcByteCount, args->epcptr->epc, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_ALIEN_HIGGS2_FULLLOADIMAGE:
      {
        TMR_TagOp_GEN2_Alien_Higgs2_FullLoadImage *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.alien.u.higgs2.u.fullLoadImage;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddHiggs2FullLoadImage(msg, &i, 0, args->accessPassword, args->killPassword, args->lockBits, 
            args->pcWord, args->epcptr->epcByteCount, args->epcptr->epc, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_ALIEN_HIGGS3_FASTLOADIMAGE:
      {
        TMR_TagOp_GEN2_Alien_Higgs3_FastLoadImage *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.alien.u.higgs3.u.fastLoadImage;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->currentAccessPassword, &lenbyte);

        TMR_SR_msgAddHiggs3FastLoadImage(msg, &i, 0, args->currentAccessPassword, args->accessPassword, 
            args->killPassword, args->pcWord, args->epcptr->epcByteCount, args->epcptr->epc, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_ALIEN_HIGGS3_LOADIMAGE:
      {
        TMR_TagOp_GEN2_Alien_Higgs3_LoadImage *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.alien.u.higgs3.u.loadImage;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->currentAccessPassword, &lenbyte);

        TMR_SR_msgAddHiggs3LoadImage(msg, &i, 0, args->currentAccessPassword, args->accessPassword, 
            args->killPassword, args->pcWord, (uint8_t)args->epcAndUserData->len, args->epcAndUserData->list, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_ALIEN_HIGGS3_BLOCKREADLOCK:
      {
        TMR_TagOp_GEN2_Alien_Higgs3_BlockReadLock *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.alien.u.higgs3.u.blockReadLock;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->accessPassword, &lenbyte);

        TMR_SR_msgAddHiggs3BlockReadLock(msg, &i, 0, args->accessPassword, args->lockBits, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_NXP_SETREADPROTECT:
      {
        TMR_TagOp_GEN2_NXP_SetReadProtect *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.nxp.u.setReadProtect;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->accessPassword, &lenbyte);

        TMR_SR_msgAddNXPSetReadProtect(msg, &i, 0, rp->u.simple.tagop->u.gen2.u.custom.chipType, args->accessPassword, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_NXP_RESETREADPROTECT:
      {
        TMR_TagOp_GEN2_NXP_ResetReadProtect *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.nxp.u.resetReadProtect;

        if (rp->u.simple.tagop->u.gen2.u.custom.chipType == TMR_SR_GEN2_NXP_G2X_SILICON)
        {
          /* NXP_G2XL_ResetReadProtect can not be embedded.
           * Throw un supported exception to the user
           */
          return TMR_ERROR_UNSUPPORTED;
        }

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->accessPassword, &lenbyte);

        TMR_SR_msgAddNXPResetReadProtect(msg, &i, 0, rp->u.simple.tagop->u.gen2.u.custom.chipType, args->accessPassword, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_NXP_CHANGEEAS:
      {
        TMR_TagOp_GEN2_NXP_ChangeEAS *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.nxp.u.changeEAS;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->accessPassword, &lenbyte);

        TMR_SR_msgAddNXPChangeEAS(msg, &i, 0, rp->u.simple.tagop->u.gen2.u.custom.chipType, args->accessPassword, args->reset, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_NXP_EASALARM:
      {
        return TMR_ERROR_UNSUPPORTED;
      }
    case TMR_TAGOP_GEN2_NXP_CALIBRATE:
      {
        TMR_TagOp_GEN2_NXP_Calibrate *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.nxp.u.calibrate;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->accessPassword, &lenbyte);

        TMR_SR_msgAddNXPCalibrate(msg, &i, 0, rp->u.simple.tagop->u.gen2.u.custom.chipType, args->accessPassword, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_NXP_CHANGECONFIG:
      {
        TMR_TagOp_GEN2_NXP_ChangeConfig *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.nxp.u.changeConfig;

        if (rp->u.simple.tagop->u.gen2.u.custom.chipType == TMR_SR_GEN2_NXP_G2X_SILICON)
        {
          /* Change Config is not supported for G2xL silicon*/
          return TMR_ERROR_UNSUPPORTED;
        }
        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->accessPassword, &lenbyte);

        TMR_SR_msgAddNXPChangeConfig(msg, &i, 0, rp->u.simple.tagop->u.gen2.u.custom.chipType, 0, args->configWord, NULL);
        break;
      }
    case (TMR_TAGOP_GEN2_NXP_UNTRACEABLE):
      {
        TMR_TagOp_GEN2_NXP_Untraceable *args;
        uint16_t configWord;
        
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.nxp.u.untraceable;
        configWord = TMR_SR_getConfigWord(reader, *args);

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddGen2v2NxpUntraceable(msg, &i, 0, rp->u.simple.tagop->u.gen2.u.custom.chipType, 0, configWord, *args, NULL);
        break;
      }
    case (TMR_TAGOP_GEN2_NXP_AUTHENTICATE):
      {
        TMR_TagOp_GEN2_NXP_Authenticate *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.nxp.u.authenticate;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddGen2v2NxpAuthenticate(msg, &i, 0, rp->u.simple.tagop->u.gen2.u.custom.chipType, 0, *args, NULL);
        break;
      }
    case (TMR_TAGOP_GEN2_NXP_READBUFFER):
      {
        TMR_TagOp_GEN2_NXP_Readbuffer *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.nxp.u.readBuffer;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);
 
        TMR_SR_msgAddGen2v2NxpReadBuffer(msg, &i, 0, rp->u.simple.tagop->u.gen2.u.custom.chipType, 0, *args, NULL);
        break;
     }
    case TMR_TAGOP_GEN2_IMPINJ_MONZA4_QTREADWRITE:
      {
        TMR_TagOp_GEN2_Impinj_Monza4_QTReadWrite *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.impinj.u.monza4.u.qtReadWrite;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->accessPassword, &lenbyte);

        TMR_SR_msgAddMonza4QTReadWrite(msg, &i, 0, 0, args->controlByte, args->payload, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_IDS_SL900A_GETSENSOR:
      {
        TMR_TagOp_GEN2_IDS_SL900A_GetSensorValue *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.ids.u.sensor;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);

        TMR_SR_msgAddIdsSL900aGetSensorValue(msg, &i, 0, 0, args->CommandCode, args->Password, args->sl900A.level,
            args->sl900A.sensortype, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_IDS_SL900A_GETMEASUREMENTSETUP:
      {
        TMR_TagOp_GEN2_IDS_SL900A_GetMeasurementSetup *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.ids.u.measurementSetup;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);

        TMR_SR_msgAddIdsSL900aGetMeasurementSetup(msg, &i, 0, 0, args->CommandCode, args->Password, args->sl900A.level, NULL);
        break;
      }

    case TMR_TAGOP_GEN2_IDS_SL900A_GETLOGSTATE:
      {
        TMR_TagOp_GEN2_IDS_SL900A_GetLogState *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.ids.u.getLog;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);

        TMR_SR_msgAddIdsSL900aGetLogState(msg, &i, 0, 0, args->CommandCode, args->Password, args->sl900A.level,NULL);

        break;
      }
    case TMR_TAGOP_GEN2_IDS_SL900A_SETLOGMODE:
      {
        TMR_TagOp_GEN2_IDS_SL900A_SetLogMode *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.ids.u.setLogMode;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);

        TMR_SR_msgAddIdsSL900aSetLogMode(msg, &i, 0, 0, args->CommandCode, args->Password, args->sl900A.level,
            args->sl900A.dataLog, args->sl900A.rule, args->Ext1Enable, args->Ext2Enable, args->TempEnable,
            args->BattEnable, args->LogInterval, NULL);

        break;
      }

    case TMR_TAGOP_GEN2_IDS_SL900A_INITIALIZE:
      {
        TMR_TagOp_GEN2_IDS_SL900A_Initialize *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.ids.u.initialize;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);

        TMR_SR_msgAddIdsSL900aInitialize(msg, &i, 0, 0, args->CommandCode, args->Password, args->sl900A.level,
            args->delayTime.raw, args->applicationData.raw, NULL);

        break;
      }
    case TMR_TAGOP_GEN2_IDS_SL900A_ENDLOG:
      {
        TMR_TagOp_GEN2_IDS_SL900A_EndLog *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.ids.u.endLog;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);

        TMR_SR_msgAddIdsSL900aEndLog(msg, &i, 0, 0, args->CommandCode, args->Password, args->sl900A.level, NULL);
        break;

      }
    case TMR_TAGOP_GEN2_IDS_SL900A_SETPASSWORD:
      {
        TMR_TagOp_GEN2_IDS_SL900A_SetPassword *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.ids.u.setPassword;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);

        TMR_SR_msgAddIdsSL900aSetPassword(msg, &i, 0, 0, args->CommandCode, args->Password, args->sl900A.level,
            args->NewPassword, args->NewPasswordLevel,  NULL);
        break;
      }
    case TMR_TAGOP_GEN2_IDS_SL900A_STARTLOG:
      {
        TMR_TagOp_GEN2_IDS_SL900A_StartLog *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.ids.u.startLog;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);

        TMR_SR_msgAddIdsSL900aStartLog(msg, &i, 0, 0, args->CommandCode, args->Password, args->sl900A.level, args->startTime, NULL);
        break;

      }
    case TMR_TAGOP_GEN2_IDS_SL900A_ACCESSFIFOSTATUS:
      {
        TMR_TagOp_GEN2_IDS_SL900A_AccessFifoStatus *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.ids.u.accessFifoStatus;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->status.AccessPassword, &lenbyte);

        TMR_SR_msgAddIdsSL900aAccessFifoStatus(msg, &i, 0, 0, args->status.CommandCode, args->status.Password, args->status.sl900A.level, args->status.operation, NULL);
        break;

      }
    case TMR_TAGOP_GEN2_IDS_SL900A_ACCESSFIFOREAD:
      {
        TMR_TagOp_GEN2_IDS_SL900A_AccessFifoRead *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.ids.u.accessFifoRead;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->read.AccessPassword, &lenbyte);

        TMR_SR_msgAddIdsSL900aAccessFifoRead(msg, &i, 0, 0, args->read.CommandCode, args->read.Password, args->read.sl900A.level, args->read.operation, args->length, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_IDS_SL900A_ACCESSFIFOWRITE:
      {
        TMR_TagOp_GEN2_IDS_SL900A_AccessFifoWrite *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.ids.u.accessFifoWrite;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->write.AccessPassword, &lenbyte);

        TMR_SR_msgAddIdsSL900aAccessFifoWrite(msg, &i, 0, 0, args->write.CommandCode, args->write.Password, args->write.sl900A.level, args->write.operation, args->payLoad, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_IDS_SL900A_GETCALIBRATIONDATA:
      {
        TMR_TagOp_GEN2_IDS_SL900A_GetCalibrationData *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.ids.u.calibrationData;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);

        TMR_SR_msgAddIdsSL900aGetCalibrationData(msg, &i, 0, 0, args->CommandCode, args->Password, args->sl900A.level, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_IDS_SL900A_SETCALIBRATIONDATA:
      {
        TMR_TagOp_GEN2_IDS_SL900A_SetCalibrationData *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.ids.u.setCalibration;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);

        TMR_SR_msgAddIdsSL900aSetCalibrationData(msg, &i, 0, 0, args->CommandCode, args->Password, args->sl900A.level, args->cal.raw, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_IDS_SL900A_SETSFEPARAMETERS:
      {
        TMR_TagOp_GEN2_IDS_SL900A_SetSfeParameters *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.ids.u.setSfeParameters;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);

        TMR_SR_msgAddIdsSL900aSetSfeParameters(msg, &i, 0, 0, args->CommandCode, args->Password, args->sl900A.level, args->sfe->raw, NULL);
        break;

      }
    case TMR_TAGOP_GEN2_IDS_SL900A_GETBATTERYLEVEL:
      {
        TMR_TagOp_GEN2_IDS_SL900A_GetBatteryLevel *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.ids.u.batteryLevel;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);

        TMR_SR_msgAddIdsSL900aGetBatteryLevel(msg, &i, 0, 0, args->CommandCode, args->Password, args->sl900A.level, args->batteryType, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_IDS_SL900A_SETLOGLIMITS:
      {
        TMR_TagOp_GEN2_IDS_SL900A_SetLogLimits *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.ids.u.setLogLimit;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);

        TMR_SR_msgAddIdsSL900aSetLogLimit(msg, &i, 0, 0, args->CommandCode, args->Password, args->sl900A.level, args->limit.extremeLower,
            args->limit.lower, args->limit.upper, args->limit.extremeUpper, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_DENATRAN_IAV_ACTIVATESECUREMODE:
      {
        TMR_TagOp_GEN2_Denatran_IAV_Activate_Secure_Mode *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.IavDenatran.u.secureMode;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddIAVDenatranCustomOp(msg, &i, 0, 0, args->mode, args->payload, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_DENATRAN_IAV_AUTHENTICATEOBU:
      {
        TMR_TagOp_GEN2_Denatran_IAV_Authenticate_OBU *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.IavDenatran.u.authenticateOBU;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddIAVDenatranCustomOp(msg, &i, 0, 0, args->mode, args->payload, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_ACTIVATE_SINIAV_MODE:
      {
        TMR_TagOp_GEN2_Denatran_IAV_Activate_Siniav_Mode *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.IavDenatran.u.activateSiniavMode;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddIAVDenatranCustomActivateSiniavMode(msg, &i, 0, 0, args->mode, args->payload, NULL, args->isTokenDesc, args->token);
        break;
      }
    case TMR_TAGOP_GEN2_OBU_AUTH_ID:
      {
        TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_ID *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthId;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddIAVDenatranCustomOp(msg, &i, 0, 0, args->mode, args->payload, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_AUTHENTICATE_OBU_FULL_PASS1:
      {
        TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_Full_Pass1 *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthFullPass1;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddIAVDenatranCustomOp(msg, &i, 0, 0, args->mode, args->payload, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_AUTHENTICATE_OBU_FULL_PASS2:
      {
        TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_Full_Pass2 *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthFullPass2;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddIAVDenatranCustomOp(msg, &i, 0, 0, args->mode, args->payload, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_OBU_READ_FROM_MEM_MAP:
      {
        TMR_TagOp_GEN2_Denatran_IAV_OBU_ReadFromMemMap *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.IavDenatran.u.obuReadFromMemMap;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddIAVDenatranCustomReadFromMemMap(msg, &i, 0, 0, args->mode, args->payload, NULL, args->readPtr);
        break;
      }
    case TMR_TAGOP_GEN2_DENATRAN_IAV_READ_SEC:
      {
        TMR_TagOp_GEN2_Denatran_IAV_Read_Sec *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.IavDenatran.u.readSec;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
              | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddIAVDenatranCustomReadSec(msg, &i, 0, 0, args->mode, args->payload, NULL, args->readPtr);
        break;
      }
 
    case TMR_TAGOP_GEN2_OBU_WRITE_TO_MEM_MAP:
      {
        TMR_TagOp_GEN2_Denatran_IAV_OBU_WriteToMemMap *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.IavDenatran.u.obuWriteToMemMap;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddIAVDenatranCustomWriteToMemMap(msg, &i, 0, 0, args->mode, args->payload, NULL, args->writePtr, args->wordData, args->tagIdentification, args->dataBuf);
        break;
      }

    case TMR_TAGOP_GEN2_DENATRAN_IAV_WRITE_SEC:
      {
        TMR_TagOp_GEN2_Denatran_IAV_Write_Sec *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.IavDenatran.u.writeSec;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
              | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddIAVDenatranCustomWriteSec(msg, &i, 0, 0, args->mode, args->payload, NULL, args->dataWords, args->dataBuf);
        break;
      }
 
    case TMR_TAGOP_GEN2_DENATRAN_IAV_GET_TOKEN_ID:
      {
        TMR_TagOp_GEN2_Denatran_IAV_Get_Token_Id *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.IavDenatran.u.getTokenId;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
              | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddIAVDenatranCustomGetTokenId(msg, &i, 0, 0, args->mode, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_DENATRAN_IAV_AUTHENTICATE_OBU_FULL_PASS:
      {
        TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_Full_Pass *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthFullPass;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
              | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddIAVDenatranCustomOp(msg, &i, 0, 0, args->mode, args->payload, NULL);
        break;

      }
    case TMR_TAGOP_GEN2_DENATRAN_IAV_G0_PA_OBU_AUTHENTICATE_ID:
      {
        TMR_TagOp_GEN2_Denatran_IAV_G0_PA_OBU_Auth_ID *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.IavDenatran.u.g0paobuauthid;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
              | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddIAVDenatranCustomOp(msg, &i, 0, 0, args->mode, args->payload, NULL);
        break;
      }
    case (TMR_TAGOP_GEN2_IMPINJ_MONZA6_MARGINREAD):
      {
        TMR_TagOp_GEN2_Impinj_Monza6_MarginRead *args;

        args = &rp->u.simple.tagop->u.gen2.u.custom.u.impinj.u.monza6.u.marginRead;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
               | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
             rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

        TMR_SR_msgAddMonza6MarginRead(msg, &i, 0, 0, args->bank, args->bitAddress, args->maskBitLength, args->mask, NULL);
        break;
      }
    case TMR_TAGOP_GEN2_NXP_UCODE7_CHANGECONFIG:
      {
        TMR_TagOp_GEN2_NXP_UCODE7_ChangeConfig *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.nxp.u.ucode7ChangeConfig;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->accessPassword, &lenbyte);

        TMR_SR_msgAddNXPUCODE7ChangeConfig(msg, &i, 0, rp->u.simple.tagop->u.gen2.u.custom.chipType, 0, args->configWord, NULL);
        break;
      }
    case TMR_TagOP_GEN2_FUDAN_READREG:
      {
        TMR_TagOP_GEN2_FUDAN_ReadREG *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.fdn.u.ReadREG;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);
        TMR_SR_msgAddFDNReadREG(msg, &i, 0, 0, args->CommandCode, args->RegAddress, NULL);

        break;
      }
    case TMR_TagOP_GEN2_FUDAN_WRITEREG:
      {
        TMR_TagOP_GEN2_FUDAN_WriteREG *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.fdn.u.WriteREG;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);
        TMR_SR_msgAddFDNWriteREG(msg, &i, 0, 0, args->CommandCode, args->RegAddress, args->RegWriteData, NULL);

        break;
      }
    case TMR_TagOP_GEN2_FUDAN_LOADREG:
      {
        TMR_TagOP_GEN2_FUDAN_LoadREG *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.fdn.u.LoadREG;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);
        TMR_SR_msgAddFDNLoadREG(msg, &i, 0, 0, args->CommandCode, args->CmdCfg, NULL);

        break;
      }
    case TMR_TagOP_GEN2_FUDAN_STARTSTOPLOG:
      {
        TMR_TagOP_GEN2_FUDAN_StartStopLOG *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.fdn.u.StartStopLOG;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);
        TMR_SR_msgAddFDNStartStopLOG(msg, &i, 0, 0, args->CommandCode, args->CmdCfg, args->FlagResetPassword, NULL);

        break;
      }
    case TMR_TagOP_GEN2_FUDAN_AUTH:
      {
        TMR_TagOP_GEN2_FUDAN_Auth *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.fdn.u.Auth;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);
        TMR_SR_msgAddFDNAuth(msg, &i, 0, 0, args->CommandCode, args->CmdCfg, args->AuthPassword, NULL);

        break;
      }
    case TMR_TagOP_GEN2_FUDAN_READMEM:
      {
        TMR_TagOP_GEN2_FUDAN_ReadMEM *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.fdn.u.ReadMEM;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);
        TMR_SR_msgAddFDNReadMEM(msg, &i, 0, 0, args->CommandCode, args->StartAddress, args->len, NULL);

        break;
      }
    case TMR_TagOP_GEN2_FUDAN_WRITEMEM:
      {
        TMR_TagOP_GEN2_FUDAN_WriteMEM *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.fdn.u.WriteMEM;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);
        TMR_SR_msgAddFDNWriteMEM(msg, &i, 0, 0, args->CommandCode, args->StartAddress, args->data->len, args->data->list, NULL);

        break;
      }
    case TMR_TagOP_GEN2_FUDAN_STATECHECK:
      {
        TMR_TagOP_GEN2_FUDAN_StateCheck *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.fdn.u.StateCheck;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);
        TMR_SR_msgAddFDNStateCheck(msg, &i, 0, 0, args->CommandCode, args->data->len, args->data->list, NULL);

        break;
      }
    case TMR_TagOP_GEN2_FUDAN_MEASURE:
      {
        TMR_TagOP_GEN2_FUDAN_Measure *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.fdn.u.Measure;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);
        TMR_SR_msgAddFDNMeasure(msg, &i, 0, 0, args->CommandCode, args->CmdCfg, args->StoreBlockAddress, NULL);

        break;
      }
      case TMR_TagOP_GEN2_ILIAN_TAG_SELECT_COMMAND:
      {
        TMR_TagOP_GEN2_ILIAN_Tag_Select_Command *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.ilian.u.IlianTagSelect;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);
        TMR_SR_msgAddILIANTagSelect(msg, &i, 0, 0, args->CommandCode, NULL);

        break;
      }
      case TMR_TagOP_GEN2_EM4325_GET_SENSOR_DATA:
      {
        TMR_TagOP_GEN2_EM4325_Get_Sensor_Data *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.emmicro.u.em4325.u.getSensorData;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);
        TMR_SR_msgAddEM4325GetSensorData(msg, &i, 0, 0, args->CommandCode, NULL, args->bitsToSet);

        break;
      }
      case TMR_TagOP_GEN2_EM4325_RESET_ALARMS:
      {
        TMR_TagOP_GEN2_EM4325_Reset_Alarms *args;
        args = &rp->u.simple.tagop->u.gen2.u.custom.u.emmicro.u.em4325.u.resetAlarms;

        ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
            | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
            rp->u.simple.filter, rp->u.simple.protocol, args->AccessPassword, &lenbyte);
        TMR_SR_msgAddEM4325ResetAlarms(msg, &i, 0, 0, args->CommandCode, NULL, args->fillValue);

        break;
      }
#endif /* TMR_ENABLE_GEN2_CUSTOM_TAGOPS */
    case TMR_TAGOP_LIST:
      {
        if(tagop->u.list.len == 1)
        {
          TMR_TagOp *op;
          op = tagop->u.list.list[0];
          return TMR_SR_addTagOp(reader, op, rp, msg, j, readTimeMs, byte);
        }
        else if (tagop->u.list.len == 2)
        {
          if (TMR_TAGOP_GEN2_READDATA == tagop->u.list.list[1]->type)
          {
            if((TMR_TAGOP_GEN2_WRITEDATA == tagop->u.list.list[0]->type) || (TMR_TAGOP_GEN2_WRITETAG == tagop->u.list.list[0]->type))
            {
              TMR_TagOp_GEN2_ReadData readData;
              TMR_TagOp *readOp, *writeOp;
              
              reader->isReadAfterWrite = true;
              writeOp = tagop->u.list.list[0];
              readOp = tagop->u.list.list[1];
              readData = readOp->u.gen2.u.readData;

              prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_CONFIGURED_LIST
                                      | TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND|TMR_SR_SEARCH_FLAG_LARGE_TAG_POPULATION_SUPPORT),
                                      rp->u.simple.filter, rp->u.simple.protocol, sr->gen2AccessPassword, &lenbyte);

              switch (tagop->u.list.list[0]->type)
              {
                case TMR_TAGOP_GEN2_WRITEDATA:
                {
                  TMR_TagOp_GEN2_WriteData writeData;
                  int idx;

                  writeData = writeOp->u.gen2.u.writeData;
                  TMR_SR_msgAddGEN2DataWrite(msg, &i, (uint16_t)readTimeMs, writeData.bank, writeData.wordAddress, false);

                  for(idx = 0 ; idx< writeData.data.len; idx++)
                  {
                    msg[i++]= (writeData.data.list[idx]>>8) & 0xFF;
                    msg[i++]= (writeData.data.list[idx]>>0) & 0xFF;
                  }
                }
                break;
                case TMR_TAGOP_GEN2_WRITETAG:
                {
                  TMR_TagOp_GEN2_WriteTag writeTag;
                  writeOp = tagop->u.list.list[0];

                  writeTag = writeOp->u.gen2.u.writeTag;
                  TMR_SR_msgAddGEN2WriteTagEPC(msg, &i, (uint16_t)readTimeMs, writeTag.epcptr->epc, writeTag.epcptr->epcByteCount);
                }
                break;
                default:
                  break;
              }

              SETU8(msg, i, readData.bank);
              SETU32(msg, i, readData.wordAddress);
              SETU8(msg, i, readData.len);
            }
            else
            {
              return TMR_ERROR_UNIMPLEMENTED_FEATURE;
            }
          }
          else
          {
            return TMR_ERROR_UNIMPLEMENTED_FEATURE;
          }
          break;
        }
        else
        {
        return TMR_ERROR_UNIMPLEMENTED_FEATURE;
        }
      }
    default:
      return TMR_ERROR_INVALID; /* Unknown tagop - internal error */
    }
  }
  }
  else
#endif /* TMR_ENABLE_UHF */
  {
#ifdef TMR_ENABLE_HF_LF
    ret = prepEmbReadTagMultiple(reader, msg, &i, (uint16_t)readTimeMs, (TMR_SR_SearchFlag)(TMR_SR_SEARCH_FLAG_EMBEDDED_COMMAND),
                                   rp->u.simple.filter, rp->u.simple.protocol, 0, &lenbyte);

    //Check for address byte extension flag.
    isAddrByteExtended = reader->featureFlags & TMR_READER_FEATURES_FLAG_ADDR_BYTE_EXTENSION;

    switch(tagop->type)
    {
      case TMR_TAGOP_READ_MEMORY:
        {
          TMR_TagOp_ReadMemory *args;
          args = &rp->u.simple.tagop->u.extTagOp.readMem;

          TMR_SR_msgAddReadMemory(msg, &i, (uint16_t)readTimeMs, args->memType, args->address, args->len, false);
          break;
        }
      case TMR_TAGOP_WRITE_MEMORY:
        {
          uint16_t len;
          TMR_TagOp_WriteMemory *args;
          args = &rp->u.simple.tagop->u.extTagOp.writeMem;

          TMR_SR_msgAddWriteMemory(msg, &i, (uint16_t)readTimeMs, args->memType, args->address);

          for (len = 0; len < args->data.len; len++)
          {
            SETU8(msg, i, args->data.list[len]);
          }
          break;
        }
      default:
        return TMR_ERROR_INVALID; /* Unknown tagop - internal error */
    }
    isAddrByteExtended = false;
#endif /* TMR_ENABLE_HF_LF */
  }

  *j = i;
  *byte = lenbyte;
  return ret;

}

/* Internal method used to update the base times stamp */
void TMR_SR_updateBaseTimeStamp(TMR_Reader *reader)
{
  /* update the base time stamp to current host time */
  uint32_t starttimeLow = 0, starttimeHigh = 0;
  TMR_SR_SerialReader *sr = &reader->u.serialReader;
  uint64_t CurrentSysTime;
  int64_t ElapsedTime = 0;
  delta = 0;

  tm_gettime_consistent(&starttimeHigh, &starttimeLow);
  CurrentSysTime = ((uint64_t)starttimeHigh << 32) | starttimeLow;

  if(!(sr->elapsedTime))
  {
    PrevSysTime = CurrentSysTime; //At start of read, Previous and Current System time will be same
  }
  ElapsedTime = (int64_t)CurrentSysTime-PrevSysTime;
  if(LastDSPMicro > ElapsedTime)
  {
    delta = LastDSPMicro - (int)ElapsedTime;
  }
  PrevSysTime = CurrentSysTime;

  sr->readTimeHigh = sr->lastSentTagTimestampHigh = starttimeHigh;
  sr->readTimeLow = sr->lastSentTagTimestampLow = starttimeLow;
}
#endif /* TMR_ENABLE_SERIAL_READER */
