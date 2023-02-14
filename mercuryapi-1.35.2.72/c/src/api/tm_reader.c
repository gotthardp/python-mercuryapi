/**
 *  @file tm_reader.c
 *  @brief Mercury API - top level implementation
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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tm_reader.h"
#include "serial_reader_imp.h"
#include "tmr_utils.h"

#define EAPI_PREFIX "eapi://"
#define EAPI_PREFIX_LEN (sizeof(EAPI_PREFIX)-1)

#define TMR_PREFIX "tmr://"
#define TMR_PREFIX_LEN (sizeof(TMR_PREFIX)-1)


//#define LLRP_EAPI_PREFIX "llrp+eapi://"
//#define LLRP_EAPI_PREFIX_LEN (sizeof(LLRP_EAPI_PREFIX)-1)
#if WIN32
#define snprintf sprintf_s
#endif 

#ifdef TMR_ENABLE_UHF
#define LLRP_PREFIX "llrp://"
#define LLRP_PREFIX_LEN (sizeof(LLRP_PREFIX)-1)
#endif /* TMR_ENABLE_UHF */

TMR_Status restart_reading(struct TMR_Reader *reader);

/**
 * Private: Should not be used by user level application.
 * structure containing the transport scheme name  and the
 * pointer to the factory init function.
 **/
static void TMR_initSerialTransportTable(void);
void* do_background_receiveAutonomousReading(void * arg);

typedef struct TMR_SerialTransportDispatchTableData
{
  char transportScheme[TMR_MAX_TRANSPORT_SCHEME_NAME];
  TMR_TransportNativeInit transportInit;
}TMR_SerialTransportDispatchTableData;

typedef struct TMR_SerialTransportDispatchTable
{
  TMR_SerialTransportDispatchTableData *list;
  uint8_t max;
  uint8_t len;
}TMR_SerialTransportDispatchTable;

static bool transportTableInitialized = false;
static TMR_SerialTransportDispatchTableData tdTableData[10];
static TMR_SerialTransportDispatchTable tdTable;

static void TMR_initSerialTransportTable()
{
  if (false == transportTableInitialized)
  {
    /** 
     * We did not get a chance to initialize the transport table.
     * Initialize it now.
     */
    memset(tdTableData, 0, sizeof(tdTableData));
    tdTable.list = tdTableData;
    tdTable.max = sizeof(tdTableData)/sizeof(tdTableData[0]);
    tdTable.len = 0;

    /* Add default schemes to the table */
    {
      TMR_String str;

      str.max = TMR_MAX_TRANSPORT_SCHEME_NAME;
      str.value = tdTable.list[tdTable.len].transportScheme;
      TMR_stringCopy(&str, "eapi", (int)strlen("eapi"));
      tdTable.list[tdTable.len].transportInit = TMR_SR_SerialTransportNativeInit;
      tdTable.len++;

      str.value = tdTable.list[tdTable.len].transportScheme;
      TMR_stringCopy(&str, "tmr", (int)strlen("tmr"));
      tdTable.list[tdTable.len].transportInit = TMR_SR_SerialTransportNativeInit;
      tdTable.len++;
    }
    transportTableInitialized = true;
  }
}

TMR_Status
TMR_create_alloc(TMR_Reader **reader, const char* deviceUri)
{

  *reader = malloc(sizeof(**reader));
  if (NULL == *reader)
    return TMR_ERROR_OUT_OF_MEMORY;
  return TMR_create(*reader, deviceUri);
}


TMR_Status
TMR_create(TMR_Reader *reader, const char* deviceUri)
{
  TMR_Status ret;

  ret = TMR_ERROR_INVALID;

#ifdef TMR_ENABLE_BACKGROUND_READS
  /* Because destruction of the readers may rely on the locks, these must be
   * created before any possible error can cause an early exit
   */
  pthread_mutex_init(&reader->backgroundLock, NULL);
  pthread_mutex_init(&reader->parserLock, NULL);
  pthread_cond_init(&reader->backgroundCond, NULL);
  pthread_cond_init(&reader->parserCond, NULL);
  pthread_cond_init(&reader->readCond, NULL);
  pthread_mutex_init(&reader->listenerLock, NULL);
  pthread_mutex_init(&reader->queue_lock, NULL);
#ifdef TMR_ENABLE_UHF
  reader->authReqListeners = NULL;
  reader->statusListeners = NULL;
#endif /* TMR_ENABLE_UHF */
  reader->readState = TMR_READ_STATE_IDLE;
  reader->backgroundSetup = false;
  reader->parserSetup = false;
#endif
  reader->readExceptionListeners = NULL;
  reader->statsListeners = NULL;
  reader->readListeners = NULL;
  reader->dutyCycle = false;
  reader->paramWait = false;
  reader->hasContinuousReadStarted = false;
  reader->lastReportedException = TMR_SUCCESS;

#ifdef TMR_ENABLE_SERIAL_READER
#ifdef TMR_ENABLE_SERIAL_TRANSPORT_NATIVE
if (
     (strncmp(deviceUri, EAPI_PREFIX, EAPI_PREFIX_LEN) == 0) ||
     ((strncmp(deviceUri, TMR_PREFIX, TMR_PREFIX_LEN) == 0)
      && *(deviceUri + TMR_PREFIX_LEN) == '/'))
{
  uint8_t i;
  char  *scheme;
  char devname[TMR_MAX_READER_NAME_LENGTH];
  bool scheme_exists = false;

  TMR_initSerialTransportTable();

  strcpy(devname, deviceUri);
  scheme = strtok((char *)devname, ":");

  for (i = 0; i < tdTable.len; i++)
  {
    if (0 == strcmp(scheme, tdTable.list[i].transportScheme))
    {
      if ('e' == deviceUri[0])
      {
        scheme = (char *)deviceUri + EAPI_PREFIX_LEN;
      }
      else
      {
        scheme = (char *)deviceUri + TMR_PREFIX_LEN;
      }
      strncpy(reader->uri, deviceUri, TMR_MAX_READER_NAME_LENGTH);

        /* Entry exists */
        ret = tdTable.list[i].transportInit(&reader->u.serialReader.transport,
            &reader->u.serialReader.
            transportContext.nativeContext,
            scheme);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      scheme_exists = true;
      break;
    }
  }

  if (false == scheme_exists)
  {
    /* No entry found */
    return TMR_ERROR_INVALID;
  }
  return TMR_SR_SerialReader_init(reader);
}
else
#endif /* TMR_ENABLE_SERIAL_TRANSPORT_NATIVE */
#ifdef TMR_ENABLE_UHF
  if ((strncmp(deviceUri, LLRP_PREFIX, LLRP_PREFIX_LEN) == 0)
      || (strncmp(deviceUri, TMR_PREFIX, TMR_PREFIX_LEN) == 0))
{
#ifdef TMR_ENABLE_LLRP_READER
  const char *host, *port;
  char hostCopy[256];

  if ('t' == deviceUri[0])
  {
    host = deviceUri + TMR_PREFIX_LEN;
  }
  else
  {
    host = deviceUri + LLRP_PREFIX_LEN;
  }
  port = strchr(host, ':');
  strcpy(hostCopy, host);
  if (port == NULL)
  {
    char *slash;
    reader->u.llrpReader.portNum = TMR_LLRP_READER_DEFAULT_PORT;
    slash = strchr(hostCopy, '/');
    if (slash)
    {
      *slash = '\0';
    }
  }
  else
  {
    reader->u.llrpReader.portNum = atoi(port + 1);
    hostCopy[port - host] = '\0';
  }

  strncpy(reader->uri, hostCopy, TMR_MAX_READER_NAME_LENGTH); 
  return TMR_LLRP_LlrpReader_init(reader);

#else
  return TMR_ERROR_UNSUPPORTED_READER_TYPE;
#endif /* TMR_ENABLE_LLRP_READER */
}
else
#endif /* TMR_ENABLE_UHF */
{
#if !((!defined(TMR_ENABLE_UHF)) || defined(BARE_METAL))
  /**
   * we are here means user requested for custom transport
   * scheme.
   **/
  uint8_t i;
  char* scheme;
  bool scheme_exists = false;

  scheme = strtok((char *)deviceUri, ":");

  for (i = 0; i < tdTable.len; i++)
  {
    if (0 == strcmp(scheme, tdTable.list[i].transportScheme))
    {
      deviceUri += strlen(scheme) + 2;
      strncpy(reader->uri, deviceUri, TMR_MAX_READER_NAME_LENGTH);

      /* Entry exists */
      ret = tdTable.list[i].transportInit(&reader->u.serialReader.transport,
          &reader->u.serialReader.
          transportContext.nativeContext,
          reader->uri);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      scheme_exists = true;
      break;
    }
  }

  if (false == scheme_exists)
  {
    /* No entry found */
    return TMR_ERROR_INVALID;
  }
  return TMR_SR_SerialReader_init(reader);
#else
  return TMR_ERROR_UNSUPPORTED_READER_TYPE;
#endif /* TMR_ENABLE_HF_LF || BARE_METAL */
}


#else /* TMR_ENABLE_SERIAL_READER */
/* No readers supported! */
return TMR_ERROR_INVALID;
#endif
}

TMR_Status
TMR_reader_init_internal(struct TMR_Reader *reader)
{
  char errMsg[256];

#ifdef TMR_ENABLE_UHF
  TMR_RP_init_simple(&reader->readParams.defaultReadPlan, 0, NULL, 
                         TMR_TAG_PROTOCOL_GEN2, 1);
#endif /* TMR_ENABLE_UHF */
  reader->readParams.readPlan = &reader->readParams.defaultReadPlan;
  reader->connected = false;
  reader->pSupportsResetStats = NULL;
  reader->_storeSupportsResetStats = false;
  reader->transportListeners = NULL;
  reader->readParams.onTime = 0;
#ifdef SINGLE_THREAD_ASYNC_READ
  reader->readParams.asyncOnTime = 250;
  reader->readParams.asyncOffTime = 0;
#endif
#ifdef TMR_ENABLE_BACKGROUND_READS
  reader->readParams.asyncOnTime = 250;  
  reader->readParams.asyncOffTime = 0;
#if 0
  pthread_mutex_init(&reader->backgroundLock, NULL);
  pthread_mutex_init(&reader->parserLock, NULL);
  pthread_cond_init(&reader->backgroundCond, NULL);
  pthread_cond_init(&reader->parserCond, NULL);
  pthread_cond_init(&reader->readCond, NULL);
  pthread_mutex_init(&reader->listenerLock, NULL);
  pthread_mutex_init(&reader->queue_lock, NULL);
  reader->readListeners = NULL;
  reader->authReqListeners = NULL;
  reader->readExceptionListeners = NULL;
  reader->statsListeners = NULL;
  reader->statusListeners = NULL;
  reader->readState = TMR_READ_STATE_IDLE;
  reader->backgroundSetup = false;
  reader->parserSetup = false;
#endif
  reader->backgroundThreadCancel = false;
  reader->backgroundEnabled = false;
  reader->trueAsyncflag = false;
  reader->parserEnabled = false;
  reader->tagQueueHead = NULL;
  reader->tagQueueTail = NULL;
  reader->isStatusResponse = false;  
  reader->isOffTimeAdded = false;
  reader->fetchTagReads = false;
  reader->tagFetchTime = 0;
  reader->searchStatus = false;
  reader->finishedReading = true;
#endif
#ifdef TMR_ENABLE_UHF
  reader->streamStats = TMR_SR_STATUS_NONE;
#endif /* TMR_ENABLE_UHF */
  reader->statsFlag = TMR_READER_STATS_FLAG_NONE;
  reader->subOffTime = 0;
  reader->continuousReading = false;
  reader->numberOfTagsToRead = 0;
  reader->featureFlags = TMR_READER_FEATURES_FLAG_NONE;
  reader->triggerRead = false;
  reader->isStopNTags = false;
  reader->portmask = 0;
#ifdef TMR_ENABLE_UHF
  reader->fastSearch = false;
  reader->isReadAfterWrite = false;
  reader->extendedAntOption = 0;
  reader->isPerAntTimeSet = 0;
#endif /* TMR_ENABLE_UHF */
#if !((!defined(TMR_ENABLE_UHF)) || defined(BARE_METAL))
  reader->regulatoryParams.RegMode = TIMED;
  reader->regulatoryParams.RegModulation = CW;
  reader->regulatoryParams.regOnTime = 500;
  reader->regulatoryParams.regOffTime = 0;
#endif /* TMR_ENABLE_HF_LF || BARE_METAL */

  /* take backup of previous value of reader stats */
  reader->curStatsFlag = 0;
  reader->isAntennaSet = false;

#ifdef TMR_ENABLE_HF_LF
  reader->isProtocolDynamicSwitching = false;
#endif /* TMR_ENABLE_HF_LF */

  //Set default error message.
  sprintf(errMsg, "Unknown error");
  if(TMR_READER_TYPE_SERIAL == reader->readerType)
  {
    sprintf(reader->u.serialReader.errMsg, "%s", errMsg);
  }
#ifdef TMR_ENABLE_LLRP_READER
  else
  {
    sprintf(reader->u.llrpReader.errMsg,  "%s", errMsg);
  }
#endif /* TMR_ENABLE_LLRP_READER */

  return TMR_SUCCESS;
}

TMR_Status
TMR_setSerialTransport(char* scheme, TMR_TransportNativeInit nativeInit)
{
  bool scheme_exists = false;
  uint8_t i;

  TMR_initSerialTransportTable();

  if ((0 == strcmp("llrp", scheme))
      || (0 == strcmp("llrp", scheme)))
  {
    /**
     * Currently set serial transport is only supported for
     * serial readers only.
     **/
    return TMR_ERROR_UNSUPPORTED;
  }

  for (i = 0; i < tdTable.len; i++)
  {
    if (0 == strcmp(scheme, tdTable.list[i].transportScheme))
    {
      /* Entry exists already, just update it */
      if (NULL != nativeInit)
      {
        tdTable.list[i].transportInit = nativeInit;
      }
      scheme_exists = true;
      break;
    }
  }

  if (false == scheme_exists)
  {

    TMR_String str;
    /* Its a new entry, add it to table */

    str.max = TMR_MAX_TRANSPORT_SCHEME_NAME;
    str.value = tdTable.list[tdTable.len].transportScheme;
    TMR_stringCopy(&str, scheme, (int)strlen(scheme));
    tdTable.list[tdTable.len].transportInit = nativeInit;
    tdTable.len++;
  }
  return TMR_SUCCESS;
}

#ifdef TMR_ENABLE_UHF
#ifdef TMR_ENABLE_API_SIDE_DEDUPLICATION

static int
TMR_findDupTag(TMR_Reader *reader,
               TMR_TagReadData* newRead,
               TMR_TagReadData oldReads[], int32_t oldLength,
               bool uniqueByAntenna, bool uniqueByData, bool uniqueByProtocol)
{
  int i;

  for (i=0; i<oldLength; i++)
  {
    TMR_TagReadData* oldRead = &oldReads[i];
    TMR_TagData* oldTag = &oldRead->tag;
    TMR_TagData* newTag = &newRead->tag;

    if ((oldTag->epcByteCount != newTag->epcByteCount) ||
        (0 != memcmp(oldTag->epc, newTag->epc,
                     (oldTag->epcByteCount)*sizeof(uint8_t))))
    {
      continue;
    }
    if (uniqueByAntenna)
    {
      if (oldRead->antenna != newRead->antenna)
      {
        continue;
      }
    }
    if (uniqueByData)
    {
      if ((oldRead->data.len != newRead->data.len) ||
          (0 != memcmp(oldRead->data.list, newRead->data.list,
                       (oldRead->data.len)*sizeof(uint8_t))))
      {
        continue;
      }
    }
    if (uniqueByProtocol)
    {
      if (oldRead->tag.protocol != newRead->tag.protocol)
      {
        continue;
      }
    }
    /* No fields mismatched; this tag is a match */
    break;
  }

  return (i < oldLength) ? i : -1;
}

static void
TMR_updateDupTag(TMR_Reader* reader,
                 TMR_TagReadData* oldRead, TMR_TagReadData* newRead,
                 bool highestRssi)
{
  oldRead->readCount += newRead->readCount;

  if (highestRssi)
  {
    if (newRead->rssi > oldRead->rssi)
    {
      uint32_t saveCount = oldRead->readCount;

      memcpy(oldRead, newRead, sizeof(TMR_TagReadData));
      /* TODO: TagReadData.data field not yet supported, pending a
       * comprehensive strategy for dynamic memory allocation. */

      oldRead->readCount = saveCount;
    }
  }
}

#endif /* TMR_ENABLE_API_SIDE_DEDUPLICATION */

TMR_Status
TMR_readIntoArray(struct TMR_Reader *reader, uint32_t timeoutMs,
                  int32_t *tagCount, TMR_TagReadData *result[])
{
  int32_t tagsRead, count, alloc;
  uint32_t readTimeMs, elapsed = 0;
  uint32_t starttimeLow, starttimeHigh;
  TMR_TagReadData *results;
  TMR_Status ret;
  bool isOpFailed = false;
#ifdef TMR_ENABLE_API_SIDE_DEDUPLICATION
  bool uniqueByAntenna, uniqueByData, recordHighestRssi, uniqueByProtocol;
#endif /* TMR_ENABLE_API_SIDE_DEDUPLICATION */

  readTimeMs = timeoutMs;

#ifdef TMR_ENABLE_API_SIDE_DEDUPLICATION
  {
    bool bval;

    ret = TMR_paramGet(reader, TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA, &bval);
    if (TMR_ERROR_NOT_FOUND == ret) { bval = false; }
    else if (TMR_SUCCESS != ret) { return ret; }
    uniqueByAntenna = bval;

    ret = TMR_paramGet(reader, TMR_PARAM_TAGREADDATA_UNIQUEBYDATA, &bval);
    if (TMR_ERROR_NOT_FOUND == ret) { bval = false; }
    else if (TMR_SUCCESS != ret) { return ret; }
    uniqueByData = bval;

    ret = TMR_paramGet(reader, TMR_PARAM_TAGREADDATA_UNIQUEBYPROTOCOL, &bval);
    if (TMR_ERROR_NOT_FOUND == ret) { bval = false; }
    else if (TMR_SUCCESS != ret) { return ret; }
    uniqueByProtocol = bval;

    ret = TMR_paramGet(reader, TMR_PARAM_TAGREADDATA_RECORDHIGHESTRSSI, &bval);
    if (TMR_ERROR_NOT_FOUND == ret) { bval = false; }
    else if (TMR_SUCCESS != ret) { return ret; }
    recordHighestRssi = bval;
  }
#endif /* TMR_ENABLE_API_SIDE_DEDUPLICATION */

  tagsRead = 0;
  alloc = 0;
  results = NULL;

  /* Cache the read time so it can be put in tag read data later */
  tm_gettime_consistent(&starttimeHigh, &starttimeLow);

  /* Calculating the elapsed time */
  elapsed = tm_time_subtract(tmr_gettime_low(), starttimeLow);

  do 
  {
    readTimeMs = timeoutMs - elapsed;
    if (timeoutMs > 65535)
    {
      timeoutMs = 65535;
    }

    ret = TMR_read(reader, readTimeMs, &count);
    if ((TMR_SUCCESS != ret) && (TMR_ERROR_TAG_ID_BUFFER_FULL != ret))
    {
      break;
    }

    if (0 == count)
    {
      break;
    }
    else if (-1 == count) /* Unknown - streaming */
    {
      alloc += 4;
    }
    else
    {
      alloc += count;
    }

    {
      TMR_TagReadData *newResults;
      newResults = realloc(results, alloc * sizeof(*results));
      if (NULL == newResults)
      {
        ret = TMR_ERROR_OUT_OF_MEMORY;
        break;
      }
      results = newResults;
    }

    while (TMR_SUCCESS == TMR_hasMoreTags(reader))
    {
      if (tagsRead == alloc)
      {
        TMR_TagReadData *newResults;
        alloc *= 2;
        newResults = realloc(results, alloc * sizeof(*results));
        if (NULL == newResults)
        {
          ret = TMR_ERROR_OUT_OF_MEMORY;
          isOpFailed = true;
          break;
        }
        results = newResults;
      }

      TMR_TRD_init(&results[tagsRead]);
      ret = TMR_getNextTag(reader, &results[tagsRead]);
      if (TMR_SUCCESS != ret)
      {
        isOpFailed = true;
        break;
      }

#ifndef TMR_ENABLE_API_SIDE_DEDUPLICATION
      tagsRead++;
#else
      /* Search array for record duplicating the one just fetched.
       * If no dup found, commit fetched tag by incrementing tag count.
       * If dup found, copy last record to found position, don't advance count.
       */
      if (true == reader->u.serialReader.enableReadFiltering)
      {
        TMR_TagReadData* last = &results[tagsRead];
        int dupIndex = TMR_findDupTag(reader, last, results, tagsRead,
                        uniqueByAntenna, uniqueByData, uniqueByProtocol);
        if (-1 == dupIndex)
        {
          tagsRead++;
        }
        else
        {
          TMR_updateDupTag(reader, &results[dupIndex], last, recordHighestRssi);
        }
      }
      else
      {
        tagsRead++;
      }
#endif /* TMR_ENABLE_API_SIDE_DEDUPLICATION */
    }

    if(isOpFailed)
    {
      break;
    }

    elapsed = tm_time_subtract(tmr_gettime_low(), starttimeLow);

  }while (elapsed <= timeoutMs);

  if (NULL != tagCount)
  {
    *tagCount = tagsRead;
  }
  *result = results;

  return ret;
}

#endif /* TMR_ENABLE_UHF */

TMR_Status
validateReadPlan(TMR_Reader *reader, TMR_ReadPlan *plan,
                  TMR_AntennaMapList *txRxMap, uint32_t protocols)
{
  TMR_Status ret = TMR_SUCCESS;
  int i, j;

  if (TMR_READ_PLAN_TYPE_MULTI == plan->type)
  {
#ifdef TMR_ENABLE_HF_LF
    if (reader->isProtocolDynamicSwitching)
    {
      reader->isProtocolDynamicSwitching = false;
      return TMR_ERROR_UNSUPPORTED;
    }
#endif /* TMR_ENABLE_HF_LF */

    plan->u.multi.totalWeight = 0;
    for (i = 0; i < plan->u.multi.planCount; i++)
    {
      ret = validateReadPlan(reader, plan->u.multi.plans[i], txRxMap, protocols);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
      plan->u.multi.totalWeight += plan->u.multi.plans[i]->weight;
    }
  }
  else if (TMR_READ_PLAN_TYPE_SIMPLE == plan->type)
  {
    if (0 == ((1 << (plan->u.simple.protocol - 1)) & protocols))
    {
      return TMR_ERROR_INVALID_PROTOCOL_SPECIFIED;
    }
    for (i = 0 ; i < plan->u.simple.antennas.len; i++)
    { 
      /* Check for antenna number. It should be non-zero otherwise skip this validation*/
      if(plan->u.simple.antennas.list[i])
      {
        for (j = 0; j < txRxMap->len; j++)
        { 
          if (plan->u.simple.antennas.list[i] == txRxMap->list[j].antenna)
          {
            break;
          }
        }
        if (j == txRxMap->len)
        {
          return TMR_ERROR_INVALID_ANTENNA_CONFIG;
        }
      }
    }
  }
  return ret;
}

bool versionCompare(uint8_t *readerVersion, uint8_t *checkVersion)
{
 uint8_t i = 0;
 
 if(readerVersion[i] < checkVersion[i])
 {
  return false;
 }
 else if(readerVersion[i] == checkVersion[i])
 {
  if(readerVersion[i+1] < checkVersion[i+1])
  {
   return false;
  }
  else if(readerVersion[i+1] == checkVersion[i+1])
  {
   if(readerVersion[i+2] < checkVersion[i+2])
   {
    return false;
   }
   else if(readerVersion[i+2] == checkVersion[i+2])
   {
    if(readerVersion[i+3] < checkVersion[i+3])
    {
     return false;
    }
   }
  }
 }
 return true;
}

#ifdef TMR_ENABLE_UHF
void getExtendedLogicalAntennaVersion(struct TMR_Reader *reader, uint8_t *checkVersion)
{
  switch (reader->u.serialReader.versionInfo.hardware[0])
  {
    case TMR_SR_MODEL_M6E:
    case TMR_SR_MODEL_M6E_I:
      checkVersion[0] = 0x01; checkVersion[1] = 0x23; checkVersion[2] = 0x01; checkVersion[3] = 0x06;
      break;
    default:
      checkVersion[0] = 0xFF; checkVersion[1] = 0xFF; checkVersion[2] = 0xFF; checkVersion[3] = 0xFF;
  }
}

void getMultiSelectVersion(struct TMR_Reader *reader, uint8_t *checkVersion)
{
  switch (reader->u.serialReader.versionInfo.hardware[0])
  {
    case TMR_SR_MODEL_M6E:
    case TMR_SR_MODEL_M6E_I:
      checkVersion[0] = 0x01; checkVersion[1] = 0x23; checkVersion[2] = 0x00; checkVersion[3] = 0x15;
      break;
    case TMR_SR_MODEL_MICRO:
      checkVersion[0] = 0x01; checkVersion[1] = 0x0b; checkVersion[2] = 0x01; checkVersion[3] = 0x2c;
      break;
    case TMR_SR_MODEL_M6E_NANO:
      checkVersion[0] = 0x01; checkVersion[1] = 0x09; checkVersion[2] = 0x00; checkVersion[3] = 0x0d;
      break;
    case TMR_SR_MODEL_M7E_PICO:
      checkVersion[0] = 0x00; checkVersion[1] = 0x00; checkVersion[2] = 0x00; checkVersion[3] = 0x00;
      break;
    default:
      checkVersion[0] = 0xFF; checkVersion[1] = 0xFF; checkVersion[2] = 0xFF; checkVersion[3] = 0xFF;
  }
}

void getDutyCycleVersion(struct TMR_Reader *reader, uint8_t *checkVersion)
{
  switch (reader->u.serialReader.versionInfo.hardware[0])
  {
    case TMR_SR_MODEL_M6E:
    case TMR_SR_MODEL_M6E_I:
      checkVersion[0] = 0x01; checkVersion[1] = 0x21; checkVersion[2] = 0x01; checkVersion[3] = 0x07;
      break;
    case TMR_SR_MODEL_MICRO:
      checkVersion[0] = 0x01; checkVersion[1] = 0x09; checkVersion[2] = 0x00; checkVersion[3] = 0x02;
      break;
    case TMR_SR_MODEL_M6E_NANO:
      checkVersion[0] = 0x01; checkVersion[1] = 0x07; checkVersion[2] = 0x00; checkVersion[3] = 0x02;
      break;
    case TMR_SR_MODEL_M7E_PICO:
      checkVersion[0] = 0x00; checkVersion[1] = 0x00; checkVersion[2] = 0x00; checkVersion[3] = 0x00;
      break;
    default:
      checkVersion[0] = 0xFF; checkVersion[1] = 0xFF; checkVersion[2] = 0xFF; checkVersion[3] = 0xFF;
  }
}

void getAntennaReadTimeVersion(struct TMR_Reader *reader, uint8_t *checkVersion)
{
  switch (reader->u.serialReader.versionInfo.hardware[0])
  {
    case TMR_SR_MODEL_M6E:
    case TMR_SR_MODEL_M6E_I:
      checkVersion[0] = 0x01; checkVersion[1] = 0x23; checkVersion[2] = 0x00; checkVersion[3] = 0x05;
      break;
    case TMR_SR_MODEL_MICRO:
      checkVersion[0] = 0x01; checkVersion[1] = 0x0B; checkVersion[2] = 0x00; checkVersion[3] = 0x14;
      break;
    case TMR_SR_MODEL_M6E_NANO:
      checkVersion[0] = 0x01; checkVersion[1] = 0x09; checkVersion[2] = 0x00; checkVersion[3] = 0x05;
      break;
    case TMR_SR_MODEL_M7E_PICO:
      checkVersion[0] = 0x00; checkVersion[1] = 0x00; checkVersion[2] = 0x00; checkVersion[3] = 0x00;
      break;
    default:
      checkVersion[0] = 0xFF; checkVersion[1] = 0xFF; checkVersion[2] = 0xFF; checkVersion[3] = 0xFF;
  }
}

void getRegionConfigurationVersion(struct TMR_Reader *reader, uint8_t *checkVersion)
{
  switch (reader->u.serialReader.versionInfo.hardware[0])
  {
    case TMR_SR_MODEL_M6E:
    case TMR_SR_MODEL_M6E_I:
      checkVersion[0] = 0x01; checkVersion[1] = 0x23; checkVersion[2] = 0x01; checkVersion[3] = 0x0A;
      break;
    case TMR_SR_MODEL_MICRO:
      checkVersion[0] = 0x01; checkVersion[1] = 0x0B; checkVersion[2] = 0x03; checkVersion[3] = 0x0F;
      break;
    case TMR_SR_MODEL_M6E_NANO:
      checkVersion[0] = 0x01; checkVersion[1] = 0x09; checkVersion[2] = 0x00; checkVersion[3] = 0x05;
      break;
    default:
      checkVersion[0] = 0x00; checkVersion[1] = 0x00; checkVersion[2] = 0x00; checkVersion[3] = 0x00;
  }
}
#endif /* TMR_ENABLE_UHF */

void getReaderStatsVersion(struct TMR_Reader *reader, uint8_t *checkVersion)
{
  switch (reader->u.serialReader.versionInfo.hardware[0])
  {
    case TMR_SR_MODEL_M6E:
    case TMR_SR_MODEL_M6E_I:
      checkVersion[0] = 0x01; checkVersion[1] = 0x21; checkVersion[2] = 0x01; checkVersion[3] = 0x02;
      break;
    case TMR_SR_MODEL_MICRO:
      checkVersion[0] = 0x01; checkVersion[1] = 0x03; checkVersion[2] = 0x00; checkVersion[3] = 0x20;
      break;
    case TMR_SR_MODEL_M6E_NANO:
      checkVersion[0] = 0x01; checkVersion[1] = 0x03; checkVersion[2] = 0x02; checkVersion[3] = 0x74;
      break;
    case TMR_SR_MODEL_M3E:
    case TMR_SR_MODEL_M7E_PICO:
      checkVersion[0] = 0x00; checkVersion[1] = 0x00; checkVersion[2] = 0x00; checkVersion[3] = 0x00;
      break;
    default:
      checkVersion[0] = 0xFF; checkVersion[1] = 0xFF; checkVersion[2] = 0xFF; checkVersion[3] = 0xFF;
  }
}

#ifdef TMR_ENABLE_HF_LF
void getAddrByteExtEnabledVersion(struct TMR_Reader *reader, uint8_t *checkVersion)
{
  switch (reader->u.serialReader.versionInfo.hardware[0])
  {
    case TMR_SR_MODEL_M3E:
      checkVersion[0] = 0x01; checkVersion[1] = 0x01; checkVersion[2] = 0x02; checkVersion[3] = 0x16;
      break;
    default:
      checkVersion[0] = 0xFF; checkVersion[1] = 0xFF; checkVersion[2] = 0xFF; checkVersion[3] = 0xFF;
  }
}
#endif  /* TMR_ENABLE_HF_LF */

#ifdef TMR_ENABLE_LLRP_READER
bool readerVersionCheck(struct TMR_Reader *reader, uint8_t *checkVersion)
{
  uint8_t readerVersion[4];
  char temp[128], *token = NULL;
  int i = 0;

  strcpy(temp, reader->u.llrpReader.capabilities.softwareVersion);
  token = strtok(temp, ".");
  if (NULL != token)
  {
    readerVersion[i++] = (uint8_t)atoi(token);
    while(NULL != token)
    {
      token = strtok(NULL, ".");
      if (token)
      {
        readerVersion[i++] = (uint8_t)atoi(token);
      }
    }
  }
  return versionCompare(readerVersion,checkVersion);
}

bool isTmreaderStateAwaterFilterTargetMapped(struct TMR_Reader *reader)
{
  uint8_t checkVersion[4];

  checkVersion[0] = 0x05; checkVersion[1] = 0x03; checkVersion[2] = 0x04; checkVersion[3] = 59;
  return readerVersionCheck(reader, checkVersion);
}

bool isTmreaderStatsListenerSupported(struct TMR_Reader *reader)
{
  uint8_t checkVersion[4];

  checkVersion[0] = 0x05; checkVersion[1] = 0x03; checkVersion[2] = 0x04; checkVersion[3] = 50;
  return readerVersionCheck(reader, checkVersion);
}

bool isTMReaderInventorySpecIDSupported(struct TMR_Reader *reader)
{
  uint8_t checkVersion[4];

  checkVersion[0] = 0x05; checkVersion[1] = 0x03; checkVersion[2] = 0x02; checkVersion[3] = 97;
  return readerVersionCheck(reader, checkVersion);
}

bool isTMReaderMultiSelectSupported(struct TMR_Reader *reader)
{
  uint8_t checkVersion[4];

  checkVersion[0] = 0x05; checkVersion[1] = 0x03; checkVersion[2] = 0x02; checkVersion[3] = 12;
  return readerVersionCheck(reader, checkVersion);
}

bool isTMReaderPerAntennaOntimeSupported(struct TMR_Reader *reader)
{
  uint8_t checkVersion[4];

  checkVersion[0] = 0x05; checkVersion[1] = 0x03; checkVersion[2] = 0x02; checkVersion[3] = 38;
  return readerVersionCheck(reader, checkVersion);
}
#endif

/* This function will check all available Reader features with respect to software version. */
void checkForAvailableReaderFeatures(struct TMR_Reader *reader)
{
#ifdef TMR_ENABLE_LLRP_READER
  reader->u.llrpReader.featureFlags = 0;
  if(isTMReaderPerAntennaOntimeSupported(reader))
  {
    reader->u.llrpReader.featureFlags |= TMMP_READER_FEATURES_FLAG_PERANTENNA_ONTIME;
  }
  if(isTMReaderMultiSelectSupported(reader))
  {
    reader->u.llrpReader.featureFlags |= TMMP_READER_FEATURES_FLAG_MULTI_SELECT;
  }
  if(isTMReaderInventorySpecIDSupported(reader))
  {
    reader->u.llrpReader.featureFlags |= TMMP_READER_FEATURES_FLAG_INVENTORYSPEC_ID;
  }
  if (isTmreaderStatsListenerSupported(reader))
  {
    reader->u.llrpReader.featureFlags |= TMMP_READER_FEATURES_FLAG_STATS_LISTENER;
  }
  if (isTmreaderStateAwaterFilterTargetMapped(reader))
  {
    reader->u.llrpReader.featureFlags |= TMMP_READER_FEATURES_FLAG_STATEAWARE_TARGET_MAPPED;
  }
#endif
}

/* This function will check all available features with respect to firmware version. */
void checkForAvailableFeatures(struct TMR_Reader *reader)
{ 
  uint8_t i = 0;
  uint8_t checkVersion[4] = {0};
  uint8_t *readerVersion = reader->u.serialReader.versionInfo.fwVersion;

  while(i < TMR_READER_TOTAL_FEATURES)
  {
    switch (1 << i)
    {
#ifdef TMR_ENABLE_UHF
      case TMR_READER_FEATURES_FLAG_DUTY_CYCLE:
      {
        getDutyCycleVersion(reader, checkVersion);
        break;
      }
      case TMR_READER_FEATURES_FLAG_MULTI_SELECT:
      {
        getMultiSelectVersion(reader, checkVersion);
        break;
      }
      case TMR_READER_FEATURES_FLAG_ANTENNA_READ_TIME:
      {
        getAntennaReadTimeVersion(reader, checkVersion);
        break;
      }
      case TMR_READER_FEATURES_FLAG_EXTENDED_LOGICAL_ANTENNA:
      {
        getExtendedLogicalAntennaVersion(reader, checkVersion);
        break;
      }
      case TMR_READER_FEATURES_FLAG_REGION_CONFIGURATION:
      {
        getRegionConfigurationVersion(reader, checkVersion);
        break;
      }
#endif /* TMR_ENABLE_UHF */
      case TMR_READER_FEATURES_FLAG_READER_STATS:
      {
        getReaderStatsVersion(reader, checkVersion);
        break;
      }
#ifdef TMR_ENABLE_HF_LF
      case TMR_READER_FEATURES_FLAG_ADDR_BYTE_EXTENSION:
      {
        getAddrByteExtEnabledVersion(reader, checkVersion);
        break;
      }
#endif /* TMR_ENABLE_HF_LF */
      default:
        break;
    }

    if (versionCompare(readerVersion, checkVersion))
    {
      reader->featureFlags |= (1 << i);
    }

    i++;
  }
}

TMR_Status
TMR_paramSet(struct TMR_Reader *reader, TMR_Param key, const void *value)
{
  TMR_Status ret;
  ret = TMR_SUCCESS;

  switch (key)
  {
#if defined(TMR_ENABLE_BACKGROUND_READS)|| defined(SINGLE_THREAD_ASYNC_READ)
    case TMR_PARAM_READ_ASYNCOFFTIME:
      {
        if (TMR_READER_TYPE_LLRP != reader->readerType)
        {
          if((*(uint32_t *)value) > TMR_MAX_VALUE)
          {
            return TMR_ERROR_INVALID_VALUE;
          }

          if (reader->readParams.asyncOffTime != *(uint32_t *)value)
          {
            uint32_t asyncOffTime = reader->readParams.asyncOffTime;
            reader->readParams.asyncOffTime = *(uint32_t *)value;
            
            if (reader->continuousReading)
            {
              ret = restart_reading(reader);
              if(ret != TMR_SUCCESS)
              {
                reader->readParams.asyncOffTime = asyncOffTime;
              }
            }
          }
          break;
        }
      }
    case TMR_PARAM_READ_ASYNCONTIME:
      {
        if ((TMR_READER_TYPE_LLRP != reader->readerType)
#ifdef TMR_ENABLE_LLRP_READER
           || (!(reader->u.llrpReader.featureFlags & TMMP_READER_FEATURES_FLAG_PERANTENNA_ONTIME))
#endif
           )
        {
          if((*(uint32_t *)value) > TMR_MAX_VALUE)
          {
            return TMR_ERROR_INVALID_VALUE;
          }

          if (reader->readParams.asyncOnTime != *(uint32_t *)value)
          {
            uint32_t asyncOnTime = reader->readParams.asyncOnTime;
            reader->readParams.asyncOnTime = *(uint32_t *)value;

            if (reader->continuousReading)
            {
              ret = restart_reading(reader);
              if(ret != TMR_SUCCESS)
              {
                reader->readParams.asyncOnTime = asyncOnTime;
              }
            }
          }
          break;
        }
      }
#endif
    default:
    {
      ret = reader->paramSet(reader, key, value);
    }
  }

  return ret;
}


TMR_Status
TMR_paramGet(struct TMR_Reader *reader, TMR_Param key, void *value)
{
  TMR_Status ret;
  ret = TMR_SUCCESS;

  switch (key)
  {
    case TMR_PARAM_READ_PLAN:
      {
        TMR_ReadPlan *plan = value;
        *plan = *reader->readParams.readPlan;
        
        break;
      }
#if defined(TMR_ENABLE_BACKGROUND_READS)|| defined(SINGLE_THREAD_ASYNC_READ) 
    case TMR_PARAM_READ_ASYNCOFFTIME:
      {
        if (TMR_READER_TYPE_LLRP != reader->readerType)
        {
          *(uint32_t *)value = reader->readParams.asyncOffTime;

          break;
        }
      }
    case TMR_PARAM_READ_ASYNCONTIME:
    {
      if ((TMR_READER_TYPE_LLRP != reader->readerType)
#ifdef TMR_ENABLE_LLRP_READER
         || (!(reader->u.llrpReader.featureFlags & TMMP_READER_FEATURES_FLAG_PERANTENNA_ONTIME))
#endif
          )
      {
        *(uint32_t *)value = reader->readParams.asyncOnTime;
        break;
      }
    }
#endif
    default:
    {
      ret = reader->paramGet(reader, key, value);
    }
  }
  return ret;
}


TMR_Status
TMR_addTransportListener(TMR_Reader *reader, TMR_TransportListenerBlock *b)
{

  b->next = reader->transportListeners;
  reader->transportListeners = b;

  return TMR_SUCCESS;
}


TMR_Status
TMR_removeTransportListener(TMR_Reader *reader, TMR_TransportListenerBlock *b)
{
  TMR_TransportListenerBlock *block, **prev;

  prev = &reader->transportListeners;
  block = reader->transportListeners;
  while (NULL != block)
  {
    if (block == b)
    {
      *prev = block->next;
      break;
    }
    prev = &block->next;
    block = block->next;
  }
  if (block == NULL)
  {
    return TMR_ERROR_INVALID;
  }

  return TMR_SUCCESS;
}


void
TMR__notifyTransportListeners(TMR_Reader *reader, bool tx, 
                              uint32_t dataLen, uint8_t *data,
                              int timeout)
{
  TMR_TransportListenerBlock *block;

  block = reader->transportListeners;
  while (NULL != block)
  {
    block->listener(tx, dataLen, data, timeout, block->cookie);
    block = block->next;
  }
}

bool
TMR_memoryProvider(void *cookie, uint16_t *size, uint8_t *data)
{
  TMR_memoryCookie *mc;
  int len;

  mc = cookie;

  if (0 == mc->firmwareSize)
  {
    return false;
  }

  if (*size > mc->firmwareSize)
  {
    *size =(uint16_t) mc->firmwareSize;
  }

  len = *size;

  memcpy(data, mc->firmwareStart, len);
  
  mc->firmwareSize -= len;
  mc->firmwareStart += len;

  return true;
}

#ifdef TMR_ENABLE_STDIO
bool
TMR_fileProvider(void *cookie, uint16_t *size, uint8_t *data)
{
  FILE *fp;
  size_t len;

  fp = cookie;

  len = fread(data, 1, *size, fp);
  if (0 == len)
  {
    return false;
  }
  
  *size = (uint16_t) len;
  return true;
}
#endif

/**
 * Initialize TMR_TagReadData with default values.
 * The tagData buffer will be initialized to TMR_MAX_EMBEDDED_DATA_LENGTH
 * which can be found in tm_config.h.  
 * If this value is zero, then the buffer is pointed to NULL.
 * @param trd Pointer to the TMR_TagReadData structure to initialize
 */
TMR_Status
TMR_TRD_init(TMR_TagReadData *trd)
{
  trd->tag.protocol = TMR_TAG_PROTOCOL_NONE;
  trd->tag.epcByteCount = 0;
#ifdef TMR_ENABLE_UHF
  trd->tag.crc = 0;
#endif /* TMR_ENABLE_UHF */
  trd->metadataFlags = 0;
#ifdef TMR_ENABLE_UHF
  trd->phase = 0;
  memset(trd->gpio, 0, sizeof(trd->gpio));
  trd->gpioCount = 0;
  trd->rssi = 0;
  trd->frequency = 0;
  trd->u.gen2.lf = (TMR_GEN2_LinkFrequency)0;
  trd->u.gen2.target = TMR_GEN2_TARGET_MIN;
  trd->u.gen2.q.u.staticQ.initialQ = 0;
  trd->gpioCount = 0;
#endif /* TMR_ENABLE_UHF*/
  trd->antenna = 0;
  trd->readCount = 0;
  trd->dspMicros = 0;
  trd->timestampLow = 0;
  trd->timestampHigh = 0;
#if TMR_MAX_EMBEDDED_DATA_LENGTH
  trd->data.list = trd->_dataList;
#ifdef TMR_ENABLE_UHF
  trd->epcMemData.list = trd->_epcMemDataList;
  trd->tidMemData.list = trd->_tidMemDataList;
  trd->userMemData.list = trd->_userMemDataList;
  trd->reservedMemData.list = trd->_reservedMemDataList;
#endif /* TMR_ENABLE_UHF */

  trd->data.max = TMR_MAX_EMBEDDED_DATA_LENGTH;
#ifdef TMR_ENABLE_UHF
  trd->epcMemData.max = TMR_MAX_EMBEDDED_DATA_LENGTH;
  trd->userMemData.max = TMR_MAX_EMBEDDED_DATA_LENGTH;
  trd->reservedMemData.max = TMR_MAX_EMBEDDED_DATA_LENGTH;
  trd->tidMemData.max = TMR_MAX_EMBEDDED_DATA_LENGTH;
#endif /* TMR_ENABLE_UHF */
#else
  trd->data.list = NULL;
#ifdef TMR_ENABLE_UHF
  trd->epcMemData.list = NULL;
  trd->userMemData.list = NULL;
  trd->tidMemData.list = NULL;
  trd->reservedMemData.list = NULL;
#endif /* TMR_ENABLE_UHF */

  trd->data.max = 0;
#ifdef TMR_ENABLE_UHF
  trd->epcMemData.max = 0;
  trd->userMemData.max = 0;
  trd->reservedMemData.max = 0;
  trd->tidMemData.max = 0;
#endif /* TMR_ENABLE_UHF */
#endif /* TMR_MAX_EMBEDDED_DATA_LENGTH */
  trd->data.len = 0;
#ifdef TMR_ENABLE_UHF
  trd->epcMemData.len = 0;
  trd->userMemData.len = 0;
  trd->tidMemData.len = 0;
  trd->reservedMemData.len = 0;
#endif /* TMR_ENABLE_UHF */
  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagReadData with the provided data storage area.
 *
 * @param trd Pointer to the TMR_TagReadData structure to initialize
 * @param size The number of bytes pointed to
 * @param buf Pointer to the uint8_t storage area
 */
TMR_Status
TMR_TRD_init_data(TMR_TagReadData *trd, uint16_t size, uint8_t *buf)
{
  trd->data.max = size;
  trd->data.len = 0;
  trd->data.list = buf;

  return TMR_SUCCESS;
}

#ifdef TMR_ENABLE_UHF
/**
 * Initialize a TMR_TagReadDataMemBank with provided data storage area
 *
 * @param data to the TMR_uint8List structure to initialize
 * @param size The nuber of bytes pointed to
 * @param buf pointer to the uint8_t storage area
 **/
TMR_Status
TMR_TRD_MEMBANK_init_data(TMR_uint8List *data, uint16_t size, uint8_t *buf)
{
  data->max = size;
  data->len = 0;
  data->list = buf;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_GEN2_Bap with the provided parameters
 *
 * @param bapVal pointer to the TMR_GEN2_Bap structure
 * @param powerUpDelayUs the power up delay time
 * @param freqHopOfftimeUs the offtime for frequencyHop
 **/
TMR_Status
TMR_GEN2_init_BapParams(TMR_GEN2_Bap *bapVal, int32_t powerUpDelayUs, int32_t freqHopOfftimeUs)
{

  bapVal->powerUpDelayUs = powerUpDelayUs;
  bapVal->freqHopOfftimeUs=freqHopOfftimeUs;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_Filter structure as tag data (EPC) filter with the
 * provided tag (which is copied).
 * 
 * @param filter Pointer to the filter structure to initialize
 * @param tag TMR_TagData to use as the filter value
 */
TMR_Status
TMR_TF_init_tag(TMR_TagFilter *filter, TMR_TagData *tag)
{
  
  filter->type = TMR_FILTER_TYPE_TAG_DATA;
  filter->u.tagData = *tag;
  
  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_Filter structure as a Gen2 select filter with the
 * provided parameters.
 * 
 * @param filter Pointer to the filter structure to initialize
 * @param invert Whether to invert the result of the select
 * @param bank The memory bank on the tag to compare with the data
 * @param bitPointer The bit address of the tag data to compare
 * @param maskBitLength The length of the data to compare
 * @param mask The data to compare
 */
TMR_Status
TMR_TF_init_gen2_select(TMR_TagFilter *filter, bool invert, TMR_GEN2_Bank bank,
                        uint32_t bitPointer, uint16_t maskBitLength,
                        uint8_t *mask)
{
  
  filter->type = TMR_FILTER_TYPE_GEN2_SELECT;
  filter->u.gen2Select.invert = invert;
  filter->u.gen2Select.bank = bank;
  filter->u.gen2Select.bitPointer = bitPointer;
  filter->u.gen2Select.maskBitLength = maskBitLength;
  filter->u.gen2Select.mask = mask;
  filter->u.gen2Select.target = SELECT; //default value of target
  filter->u.gen2Select.action = ON_N_OFF; //default value of action
  
  return TMR_SUCCESS;
}

#ifdef TMR_ENABLE_ISO180006B
/**
 * Initialize a TMR_Filter structure as an ISO180006B select filter with the
 * provided parameters.
 * 
 * @param filter Pointer to the filter structure to initialize
 * @param invert Whether to invert the result of the select
 * @param op The operation to use to compare the provided data with the tag data
 * @param address The address of the 8 bytes of data on the tag to compare 
 * @param mask 8-bit mask of the bytes to compare
 * @param wordData The data to compare to the tag data
 */
TMR_Status TMR_TF_init_ISO180006B_select(TMR_TagFilter *filter, bool invert,
                                         TMR_ISO180006B_SelectOp op,
                                         uint8_t address, uint8_t mask,
                                         uint8_t wordData[8])
{

  filter->type = TMR_FILTER_TYPE_ISO180006B_SELECT;
  filter->u.iso180006bSelect.invert = invert;
  filter->u.iso180006bSelect.op = op;
  filter->u.iso180006bSelect.address = address;
  filter->u.iso180006bSelect.mask = mask;
  memcpy(filter->u.iso180006bSelect.data, wordData, 8);
  
  return TMR_SUCCESS;
}
#endif /* TMR_ENABLE_ISO180006B */


bool
TMR_TF_match(TMR_TagFilter *filter, TMR_TagData *tag)
{
  int32_t i, bitAddr;
  bool match;
  TMR_GEN2_Select *sel;

  if (TMR_FILTER_TYPE_GEN2_SELECT != filter->type)
  {
    return false;
  }

  if (TMR_TAG_PROTOCOL_GEN2 != tag->protocol)
  {
    return false;
  }

  sel = &filter->u.gen2Select;

  if (TMR_GEN2_BANK_EPC != sel->bank)
  {
    /*
     * Can't perform non-EPC matches, since we don't have the rest of
     * the tag data.
     */
    return false;
  }

  i = 0;
  bitAddr = sel->bitPointer;
  /*
   * Matching against the CRC and PC does not have defined
   * behavior; see section 6.3.2.11.1.1 of Gen2 version 1.2.0.
   * We choose to let it match, because that's simple.
   */
  bitAddr -= 32;
  if (bitAddr < 0)
  {
    i -= bitAddr;
    bitAddr = 0;
  }

  match = true;
  for (; i < sel->maskBitLength; i++, bitAddr++)
  {
    if (bitAddr >(int32_t) (tag->epcByteCount * 8))
    {
      match = false;
      break;
    }
    /* Extract the relevant bit from both the EPC and the mask. */
    if (((tag->epc[bitAddr / 8] >> (7 - (bitAddr & 7))) & 1) !=
        ((sel->mask[i / 8] >> (7 - (i & 7))) & 1))
    {
      match = false;
      break;
    }
  }
  if (sel->invert)
    match = match ? false : true;

  return match;
}


/**
 * Initialize a TMR_TagAuthentication structure as a Gen2 password.
 *
 * @param auth Pointer to the structure to initialize.
 * @param password The password 32-bit Gen2 password value.
 */
TMR_Status
TMR_TA_init_gen2(TMR_TagAuthentication *auth, TMR_GEN2_Password password)
{

  auth->type = TMR_AUTH_TYPE_GEN2_PASSWORD;
  auth->u.gen2Password = password;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagAuthentication structure as Denatran IAV write credential
 *
 * @param auth Pointer to the structure to initialize.
 * @param data Gen2 Denatran IAV write credential
 */
TMR_Status
TMR_TA_init_gen2_Denatran_IAV_writeCredentials(TMR_TagAuthentication *auth, uint8_t idLength, uint8_t* tagId, uint8_t len, uint8_t *data)
{
  uint8_t i;
  auth->type = TMR_AUTH_TYPE_GEN2_DENATRAN_IAV_WRITE_CREDENTIALS;

  /* The length should be 16 byte */
  auth->u.writeCreds.credentialLength = len;
  if (16 != auth->u.writeCreds.credentialLength)
  {
    return TMR_ERROR_INVALID;
  }

  /* The length for tag ID should be 8 bytes */
  auth->u.writeCreds.tagIdLength = idLength;
  if (8 != auth->u.writeCreds.tagIdLength)
  {
    return TMR_ERROR_INVALID;
  }

  /* Copy the data */
  for (i = 0; i < auth->u.writeCreds.tagIdLength; i++)
  {
    memcpy(auth->u.writeCreds.tagId, tagId, auth->u.writeCreds.tagIdLength);
  }

  /* Copy the data */
  for (i = 0; i < auth->u.writeCreds.credentialLength; i++)
  {
    memcpy(auth->u.writeCreds.value, data, auth->u.writeCreds.credentialLength);
  }

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagAuthentication structure as Denatran IAV write credential
 *
 * @param auth Pointer to the structure to initialize.
 * @param data Gen2 Denatran IAV write credential
 */
TMR_Status
TMR_TA_init_gen2_Denatran_IAV_writeSecCredentials(TMR_TagAuthentication *auth, uint8_t length, uint8_t* data, uint8_t len, uint8_t* credentials)
{
  uint8_t i;
  auth->type = TMR_AUTH_TYPE_GEN2_DENATRAN_IAV_WRITE_SEC_CREDENTIALS;

  /* The length should be 16 byte */
  auth->u.writeSecCreds.credentialLength = len;
  if (16 != auth->u.writeSecCreds.credentialLength)
  {
    return TMR_ERROR_INVALID;
  }

  /* The length for data words should be 6 bytes */
  auth->u.writeSecCreds.dataLength = length;
  if (6 != auth->u.writeSecCreds.dataLength)
  {
    return TMR_ERROR_INVALID;
  }

  /* Copy the data */
  for (i = 0; i < auth->u.writeSecCreds.dataLength; i++)
  {
    memcpy(auth->u.writeSecCreds.data, data, auth->u.writeSecCreds.dataLength);
  }

  /* Copy the data */
  for (i = 0; i < auth->u.writeSecCreds.credentialLength; i++)
  {
    memcpy(auth->u.writeSecCreds.value, credentials, auth->u.writeSecCreds.credentialLength);
  }

  return TMR_SUCCESS;
}

/** Initialize a TMR_StatValues structure with the default vales
 *
 * @param stats Pointer to the TMR_StatValues structure to initialize.
 */
TMR_Status
TMR_STATS_init(TMR_Reader_StatsValues *stats)
{
  if (NULL != stats)
  {
    stats->valid = TMR_READER_STATS_FLAG_NONE;
    /* Allocate space for the parameters */
    stats->connectedAntennas.len = 0;
    stats->connectedAntennas.max = TMR_SR_MAX_ANTENNA_PORTS;
    stats->connectedAntennas.list = stats->_connectedAntennasStorage;
    stats->perAntenna.len = 0;
    stats->perAntenna.max = TMR_SR_MAX_ANTENNA_PORTS;
    stats->perAntenna.list = stats->_perAntStorage;
  }
  return TMR_SUCCESS;
}

/** Copy a TMR_StatValues strcutre pointed by src, to TMR_StatValues structure pointed by dst
 *
 * @param dst Pointer to the TMR_StatValues structure to copy to.
 * @param src Pointer to the TMR_StatValues structure to copy from.
*/
TMR_Status
TMR_STATS_copy(TMR_Reader_StatsValues *dst, TMR_Reader_StatsValues *src)
{
  if (NULL != src && NULL != dst)
  {
    *dst = *src;
     dst->perAntenna.list = dst->_perAntStorage;
  }
  return TMR_SUCCESS;
}

#endif /* TMR_ENABLE_UHF */
/**
 * Initialize a TMR_GPITriggerRead structure with default values.
 *
 * @param triggerRead Pointer to the read plan to initialize.
 * @param enable Option to enable trigger read.
 */
TMR_Status
TMR_GPITR_init_enable(TMR_GPITriggerRead *triggerRead, bool enable)
{
  triggerRead->enable = enable;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_ReadPlan structure as a simple read plan with the
 * provided parameters.  
 *
 * Only the mandatory elements are parameters to
 * this function. The optional elements, filters and tag operations,
 * can be set with TMR_RP_set_filter() and TMR_RP_set_tagop().
 *
 * @param plan Pointer to the read plan to initialize.
 * @param antennaCount Number of antennas in antenna list. A
 * zero-length list requests the reader to use all antennas known to
 * be connected at the time of the read operation.
 * @param antennaList Pointer to antenna numbers.
 * @param protocol Protocol
 * @param weight Weight.
 */
TMR_Status
TMR_RP_init_simple(TMR_ReadPlan *plan, uint8_t antennaCount,
                   uint8_t *antennaList, TMR_TagProtocol protocol,
                   uint32_t weight) 
{
  
  plan->type = TMR_READ_PLAN_TYPE_SIMPLE;
  plan->u.simple.antennas.max = antennaCount;
  plan->u.simple.antennas.len = antennaCount;
  plan->u.simple.antennas.list = antennaList;
  plan->u.simple.protocol = protocol;
  plan->u.simple.filter = NULL;
  plan->u.simple.tagop = NULL;
  plan->weight = weight;
#ifdef TMR_ENABLE_LLRP_READER
  plan->u.simple.customAntConfig = NULL;
#endif /* TMR_ENABLE_LLRP_READER */
#ifdef TMR_ENABLE_UHF
  plan->u.simple.useFastSearch = false;
#endif /* TMR_ENABLE_UHF */
  plan->u.simple.stopOnCount.stopNTriggerStatus = false;
  plan->u.simple.stopOnCount.noOfTags = 0;
  plan->u.simple.triggerRead.enable = false;
  plan->u.simple.triggerRead.gpiList.len = 0;
  plan->u.simple.triggerRead.gpiList.list = NULL;
  plan->u.simple.triggerRead.gpiList.max = 0;
  plan->enableAutonomousRead = false;
  return TMR_SUCCESS;
}

/**
 * Set the stop on N tags  option of a read plan
 *
 * @param plan Pointer to the read plan
 * @param nCount the number of tags user requested to search.
 */
TMR_Status
TMR_RP_set_stopTrigger(TMR_ReadPlan *plan, uint32_t nCount)
{

  if (TMR_READ_PLAN_TYPE_SIMPLE != plan->type)
    return TMR_ERROR_INVALID;

  plan->u.simple.stopOnCount.stopNTriggerStatus = true;
  plan->u.simple.stopOnCount.noOfTags = nCount;

  return TMR_SUCCESS;
}

/**
 * Set the filter of a simple read plan.
 *
 * @param plan Pointer to the read plan
 * @param filter Pointer to the filter 
 */
TMR_Status
TMR_RP_set_filter(TMR_ReadPlan *plan, TMR_TagFilter *filter)
{

  if (TMR_READ_PLAN_TYPE_SIMPLE != plan->type)
    return TMR_ERROR_INVALID;

  plan->u.simple.filter = filter;

  return TMR_SUCCESS;
}

#ifdef TMR_ENABLE_UHF
#ifdef TMR_ENABLE_LLRP_READER
TMR_Status
TMR_RP_set_customAntConfig(TMR_ReadPlan *plan,TMR_CustomAntConfig *customAntConfig)
{

  if (TMR_READ_PLAN_TYPE_SIMPLE != plan->type)
    return TMR_ERROR_INVALID;
  
  plan->u.simple.customAntConfig = customAntConfig;

  return TMR_SUCCESS;
}
#endif /* TMR_ENABLE_LLRP_READER */
/**
 * Set the fast search option of a read plan
 *
 * @param plan Pointer to the read plan
 * @param useFastSearch Option for FastSearch
 */ 
TMR_Status
TMR_RP_set_useFastSearch(TMR_ReadPlan *plan, bool useFastSearch)
{

  if (TMR_READ_PLAN_TYPE_SIMPLE != plan->type)
    return TMR_ERROR_INVALID;

  plan->u.simple.useFastSearch = useFastSearch;

  return TMR_SUCCESS;
}
#endif /* TMR_ENABLE_UHF */
/**
 * Set the autonomous read option of a read plan
 *
 * @param plan Pointer to the read plan
 * @param autonomousRead Option for autonomous read.
 */ 
TMR_Status
TMR_RP_set_enableAutonomousRead(TMR_ReadPlan *plan, bool autonomousRead)
{
  plan->enableAutonomousRead = autonomousRead;
  return TMR_SUCCESS;
}

/**
 * Set the trigger read option of a read plan
 *
 * @param plan Pointer to the read plan
 * @param triggerRead Pointer for trigger read
 */
TMR_Status
TMR_RP_set_enableTriggerRead(TMR_ReadPlan *plan, TMR_GPITriggerRead *triggerRead)
{
  if (TMR_READ_PLAN_TYPE_SIMPLE != plan->type)
    return TMR_ERROR_INVALID;

  plan->u.simple.triggerRead.enable = triggerRead->enable;
  /*
   * TODO: extent the set_enableTriggerRead() method to accept the GPI list directly
           as a part of read plan. Current the GPI list can be set through TMR_paramSet().
   */
  plan->u.simple.triggerRead.gpiList.list = NULL;
  plan->u.simple.triggerRead.gpiList.len = 0;
  plan->u.simple.triggerRead.gpiList.max = 0;  

  return TMR_SUCCESS;
}

/**
 * Set the tagop of a simple read plan.
 *
 * @param plan Pointer to the read plan
 * @param tagop Pointer to the tagop 
 */
TMR_Status
TMR_RP_set_tagop(TMR_ReadPlan *plan, TMR_TagOp *tagop)
{

  if (TMR_READ_PLAN_TYPE_SIMPLE != plan->type)
    return TMR_ERROR_INVALID;

  plan->u.simple.tagop = tagop;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_ReadPlan structure as a multi-read plan with the
 * provided parameters.
 *
 * @param plan Pointer to the read plan to initialize.
 * @param plans Array of pointers to read plans to include
 * @param planCount Number of elements in array
 * @param weight Weight.
 */
TMR_Status
TMR_RP_init_multi(TMR_ReadPlan *plan, TMR_ReadPlan **plans, uint8_t planCount,
                  uint32_t weight)
{
  plan->type = TMR_READ_PLAN_TYPE_MULTI;
  plan->u.multi.plans = plans;
  plan->u.multi.planCount = planCount;
  plan->u.multi.totalWeight = 0;
  plan->weight = weight;

  return TMR_SUCCESS;
}


#ifdef TMR_ENABLE_UHF
/**
 * Initialize a TMR_TagLockAction as a Gen2 lock action with the
 * provided parameters.
 *
 * @param lockAction Pointer to the structure to initialize.
 * @param mask mask
 * @param action action
 */
TMR_Status
TMR_TLA_init_gen2(TMR_TagLockAction *lockAction, uint16_t mask, uint16_t action)
{

  lockAction->type = TMR_LOCK_ACTION_TYPE_GEN2_LOCK_ACTION;
  lockAction->u.gen2LockAction.mask = mask;
  lockAction->u.gen2LockAction.action = action;

  return TMR_SUCCESS;
}


#ifdef TMR_ENABLE_ISO180006B
/**
 * Initialize a TMR_TagLockAction as an ISO180006B lock action with the
 * provided parameters.
 *
 * @param lockAction Pointer to the structure to initialize.
 * @param address The byte to lock
 */
TMR_Status
TMR_TLA_init_ISO180006B(TMR_TagLockAction *lockAction, uint8_t address)
{

  lockAction->type = TMR_LOCK_ACTION_TYPE_ISO180006B_LOCK_ACTION;
  lockAction->u.iso180006bLockAction.address = address;

  return TMR_SUCCESS;
}

#endif /* TMR_ENABLE_ISO180006B */


/**
 * Initialize a TMR_TagOp as a GEN2 EPC write operation with the
 * provided parameters.
 * 
 * @param tagop Pointer to the tagop structure to initialize.
 * @param epc EPC to write
 */
TMR_Status
TMR_TagOp_init_GEN2_WriteTag(TMR_TagOp *tagop, TMR_TagData* epc)
{
  tagop->type = TMR_TAGOP_GEN2_WRITETAG;
  tagop->u.gen2.u.writeTag.epcptr = epc;  /* Takes pointer to EPC; doesn't make an actual copy */

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a GEN2 data read operation with the
 * provided parameters.
 * 
 * @param tagop Pointer to the tagop structure to initialize.
 * @param bank Memory bank from which to read
 * @param wordAddress Word address of location in bank from which to read
 * @param len Number of words to read
 */
TMR_Status
TMR_TagOp_init_GEN2_ReadData(TMR_TagOp *tagop, TMR_GEN2_Bank bank,
                             uint32_t wordAddress, uint8_t len)
{

  tagop->type = TMR_TAGOP_GEN2_READDATA;
  tagop->u.gen2.u.readData.bank = bank;
  tagop->u.gen2.u.readData.wordAddress = wordAddress;
  tagop->u.gen2.u.readData.len = len;

  return TMR_SUCCESS;
}

/** 
 * Initialize a TMR_TagOp as a GEN2 Secure data read operation with the
 * provided parameters.
 *
 * @param tagop Pointer to the tagop structure to initialize.
 * @param bank Memory bank from which to read
 * @param wordAddress Word address of location in bank from which to read
 * @param len Number of words to read
 * @param tagtype to select Alien Higgs 3 secure access and Monza 4 secure access
 * @param passwordType specifying the mode of password 
 */
TMR_Status
TMR_TagOp_init_GEN2_SecureReadData(TMR_TagOp *tagop, TMR_GEN2_Bank bank, uint32_t wordAddress,
                                   uint8_t len, uint8_t tagtype, uint8_t passwordType)
{
  tagop->type = TMR_TAGOP_GEN2_SECURE_READDATA;
  tagop->u.gen2.u.secureReadData.passwordType = (SecurePasswordType)passwordType;
  tagop->u.gen2.u.secureReadData.readData.bank = bank;
  tagop->u.gen2.u.secureReadData.readData.wordAddress = wordAddress;
  tagop->u.gen2.u.secureReadData.readData.len = len;
  tagop->u.gen2.u.secureReadData.type = (SecureTagType)tagtype;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 Secure Password with the 
 * provided parameters.
 *
 * @param tagop Pointer to the tagop structure to initialize
 * @param passwordType specifying the mode of password 
 * @param gen2PassWord Gen2 access password
 * @param addressLength Number of bits used to address the AP list
 * @param addressOffset EPC word offset
 * @param flashOffset User flash offset
 */
TMR_Status
TMR_TagOp_init_GEN2_SecurePassWord(TMR_TagOp *tagop, uint8_t passwordType, uint32_t gen2PassWord,
                                   uint8_t addressLength, uint8_t addressOffset, uint16_t flashOffset)
{
  
  tagop->u.gen2.u.secureReadData.passwordType = (SecurePasswordType)passwordType;
  tagop->u.gen2.u.secureReadData.password.gen2PassWord.u.gen2Password = gen2PassWord;
  tagop->u.gen2.u.secureReadData.password.secureAddressLength = addressLength;
  tagop->u.gen2.u.secureReadData.password.secureAddressOffset = addressOffset;
  tagop->u.gen2.u.secureReadData.password.secureFlashOffset = flashOffset;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a ISO18000B data read operation with the
 * provided parameters.
 * 
 * @param tagop Pointer to the tagop structure to initialize.
 * @param byteAddress Address of location in bank from which to read
 * @param len Number of bytes to read
 */
TMR_Status
TMR_TagOp_init_ISO180006B_ReadData(TMR_TagOp *tagop, uint8_t byteAddress, uint8_t len)
{

  tagop->type = TMR_TAGOP_ISO180006B_READDATA;
  tagop->u.iso180006b.u.readData.byteAddress = byteAddress;
  tagop->u.iso180006b.u.readData.len = len;

  return TMR_SUCCESS;
}


/**
 * Initialize a TMR_TagOp as a GEN2 data write operation with the
 * provided parameters.
 * 
 * @param tagop Pointer to the tagop structure to initialize.
 * @param bank Memory bank to write into
 * @param wordAddress Word address of location to begin write
 * @param data Data to write
 */
TMR_Status
TMR_TagOp_init_GEN2_WriteData(TMR_TagOp *tagop, TMR_GEN2_Bank bank,
                              uint32_t wordAddress, TMR_uint16List *data)
{
  tagop->type = TMR_TAGOP_GEN2_WRITEDATA;
  tagop->u.gen2.u.writeData.bank = bank;
  tagop->u.gen2.u.writeData.wordAddress = wordAddress;
  tagop->u.gen2.u.writeData.data = *data; /* Copies pointer to the words adata but not data */

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a ISO180006B data write operation with the
 * provided parameters.
 * 
 * @param tagop Pointer to the tagop structure to initialize.
 * @param byteAddress  address of location to begin write
 * @param data Data to write
 */
TMR_Status
TMR_TagOp_init_ISO180006B_WriteData(TMR_TagOp *tagop, uint8_t byteAddress, TMR_uint8List *data)
{
  tagop->type = TMR_TAGOP_ISO180006B_WRITEDATA;
  tagop->u.iso180006b.u.writeData.byteAddress = byteAddress;
  tagop->u.iso180006b.u.writeData.data = *data;
  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a GEN2 lock operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param mask mask
 * @param action action
 * @param accessPassword The password to use to lock the tag.
 */
TMR_Status
TMR_TagOp_init_GEN2_Lock(TMR_TagOp *tagop, uint16_t mask, uint16_t action, TMR_GEN2_Password accessPassword)
{

  tagop->type = TMR_TAGOP_GEN2_LOCK;
  tagop->u.gen2.u.lock.mask = mask;
  tagop->u.gen2.u.lock.action = action;
  tagop->u.gen2.u.lock.accessPassword = accessPassword;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a ISO180006B lock operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param address The memory address of the byte to lock.
 */
TMR_Status
TMR_TagOp_init_ISO180006B_Lock(TMR_TagOp *tagop,  uint8_t address)
{

  tagop->type = TMR_TAGOP_ISO180006B_LOCK;
  tagop->u.iso180006b.u.lock.address = address;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a GEN2 kill operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param killPassword tag kill password
 */
TMR_Status
TMR_TagOp_init_GEN2_Kill(TMR_TagOp *tagop, TMR_GEN2_Password killPassword)
{

  tagop->type = TMR_TAGOP_GEN2_KILL;
  tagop->u.gen2.u.kill.password = killPassword;

  return TMR_SUCCESS;
}


/**
 * Initialize a TMR_TagOp as a GEN2 BlockWrite operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param bank bank
 * @param wordPtr word pointer
 * @param data data (The length of the data specifies the word count)
 */
TMR_Status
TMR_TagOp_init_GEN2_BlockWrite(TMR_TagOp *tagop, TMR_GEN2_Bank bank, uint32_t wordPtr, TMR_uint16List *data)
{
  tagop->type = TMR_TAGOP_GEN2_BLOCKWRITE;
  tagop->u.gen2.u.blockWrite.bank = bank;
  tagop->u.gen2.u.blockWrite.wordPtr = wordPtr;
  tagop->u.gen2.u.blockWrite.data.len = data->len;
  tagop->u.gen2.u.blockWrite.data.list = data->list;
  tagop->u.gen2.u.blockWrite.data.max = data->max;
  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a GEN2 BlockPermaLock operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param readLock readLock
 * @param bank bank
 * @param blockPtr block pointer
 * @param mask mask (The length of the mask specifies the block range)
 */
TMR_Status
TMR_TagOp_init_GEN2_BlockPermaLock(TMR_TagOp *tagop, uint8_t readLock, TMR_GEN2_Bank bank, uint32_t blockPtr, TMR_uint16List *mask)
{
  tagop->type = TMR_TAGOP_GEN2_BLOCKPERMALOCK;
  tagop->u.gen2.u.blockPermaLock.readLock = readLock;
  tagop->u.gen2.u.blockPermaLock.bank = bank;
  tagop->u.gen2.u.blockPermaLock.blockPtr = blockPtr;
  tagop->u.gen2.u.blockPermaLock.mask.len = mask->len;
  tagop->u.gen2.u.blockPermaLock.mask.list = mask->list;
  tagop->u.gen2.u.blockPermaLock.mask.max = mask->max;
  return TMR_SUCCESS;

}

/**
 * Initialize a TMR_TagOp as a GEN2 BlockErase operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param bank bank
 * @param wordPtr The starting word address to erase
 * @param wordCount Number of words to erase
 */
TMR_Status 
TMR_TagOp_init_GEN2_BlockErase(TMR_TagOp *tagop, TMR_GEN2_Bank bank, uint32_t wordPtr, uint8_t wordCount)
{
  tagop->type = TMR_TAGOP_GEN2_BLOCKERASE;
  tagop->u.gen2.u.blockErase.bank = bank;
  tagop->u.gen2.u.blockErase.wordCount = wordCount;
  tagop->u.gen2.u.blockErase.wordPtr = wordPtr;
  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Higgs2 Partial Load Image operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param killPassword kill password
 * @param accessPassword access password
 * @param epc EPC to write
 */

#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_GEN2_CUSTOM_TAGOPS

TMR_Status 
TMR_TagOp_init_GEN2_Alien_Higgs2_PartialLoadImage(TMR_TagOp *tagop, TMR_GEN2_Password killPassword,
                                       TMR_GEN2_Password accessPassword, TMR_TagData *epc)
{
  tagop->type = TMR_TAGOP_GEN2_ALIEN_HIGGS2_PARTIALLOADIMAGE;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_ALIEN_HIGGS_SILICON;
  tagop->u.gen2.u.custom.u.alien.u.higgs2.u.partialLoadImage.accessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.alien.u.higgs2.u.partialLoadImage.killPassword = killPassword;
  tagop->u.gen2.u.custom.u.alien.u.higgs2.u.partialLoadImage.epcptr = epc; /* Takes pointer to EPC; doesn't make an actual copy */

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Higgs2 Full Load Image operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param killPassword kill password
 * @param accessPassword access password
 * @param lockBits Lock bits to write to the tag
 * @param pcWord PC word to write to the tag
 * @param epc EPC to write
 */
TMR_Status
TMR_TagOp_init_GEN2_Alien_Higgs2_FullLoadImage(TMR_TagOp *tagop, TMR_GEN2_Password killPassword,
                                    TMR_GEN2_Password accessPassword, uint16_t lockBits, 
                                    uint16_t pcWord, TMR_TagData *epc)
{
  tagop->type = TMR_TAGOP_GEN2_ALIEN_HIGGS2_FULLLOADIMAGE;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_ALIEN_HIGGS_SILICON;
  tagop->u.gen2.u.custom.u.alien.u.higgs2.u.fullLoadImage.accessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.alien.u.higgs2.u.fullLoadImage.killPassword = killPassword;
  tagop->u.gen2.u.custom.u.alien.u.higgs2.u.fullLoadImage.lockBits = lockBits;
  tagop->u.gen2.u.custom.u.alien.u.higgs2.u.fullLoadImage.pcWord = pcWord;
  tagop->u.gen2.u.custom.u.alien.u.higgs2.u.fullLoadImage.epcptr = epc; /* Takes pointer to EPC; doesn't make an actual copy */

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Higgs3 Fast Load Image operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param currentAccessPassword The access password used to write to the tag
 * @param accessPassword access password
 * @param killPassword kill password
 * @param pcWord PC word to write to the tag
 * @param epc EPC to write
 */
TMR_Status TMR_TagOp_init_GEN2_Alien_Higgs3_FastLoadImage(TMR_TagOp *tagop, TMR_GEN2_Password currentAccessPassword,
                                    TMR_GEN2_Password accessPassword, TMR_GEN2_Password killPassword, 
                                    uint16_t pcWord, TMR_TagData *epc)
{
  tagop->type = TMR_TAGOP_GEN2_ALIEN_HIGGS3_FASTLOADIMAGE;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_ALIEN_HIGGS3_SILICON;
  tagop->u.gen2.u.custom.u.alien.u.higgs3.u.fastLoadImage.currentAccessPassword = currentAccessPassword;
  tagop->u.gen2.u.custom.u.alien.u.higgs3.u.fastLoadImage.killPassword = killPassword;
  tagop->u.gen2.u.custom.u.alien.u.higgs3.u.fastLoadImage.accessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.alien.u.higgs3.u.fastLoadImage.pcWord = pcWord;
  tagop->u.gen2.u.custom.u.alien.u.higgs3.u.fastLoadImage.epcptr = epc; /* Takes pointer to EPC; doesn't make an actual copy */

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Higgs3 Load Image operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param currentAccessPassword The access password used to write to the tag
 * @param accessPassword access password
 * @param killPassword kill password
 * @param pcWord PC word to write to the tag
 * @param epcAndUserData Tag EPC and user data to write to the tag (76 bytes)
 */
TMR_Status TMR_TagOp_init_GEN2_Alien_Higgs3_LoadImage(TMR_TagOp *tagop, TMR_GEN2_Password currentAccessPassword,
                                    TMR_GEN2_Password accessPassword, TMR_GEN2_Password killPassword, 
                                    uint16_t pcWord, TMR_uint8List *epcAndUserData)
{
  tagop->type = TMR_TAGOP_GEN2_ALIEN_HIGGS3_LOADIMAGE;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_ALIEN_HIGGS3_SILICON;
  tagop->u.gen2.u.custom.u.alien.u.higgs3.u.loadImage.currentAccessPassword = currentAccessPassword;
  tagop->u.gen2.u.custom.u.alien.u.higgs3.u.loadImage.killPassword = killPassword;
  tagop->u.gen2.u.custom.u.alien.u.higgs3.u.loadImage.accessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.alien.u.higgs3.u.loadImage.pcWord = pcWord;
  tagop->u.gen2.u.custom.u.alien.u.higgs3.u.loadImage.epcAndUserData = epcAndUserData; /* Takes pointer epcAndUserData; doesn't make an actual copy */

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Higgs3 Block Read Lock operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword access password
 * @param lockBits A bitmask of bits to lock
 */
TMR_Status TMR_TagOp_init_GEN2_Alien_Higgs3_BlockReadLock(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, uint8_t lockBits)
{
  tagop->type = TMR_TAGOP_GEN2_ALIEN_HIGGS3_BLOCKREADLOCK;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_ALIEN_HIGGS3_SILICON;
  tagop->u.gen2.u.custom.u.alien.u.higgs3.u.blockReadLock.accessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.alien.u.higgs3.u.blockReadLock.lockBits = lockBits;
  
  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a NXP G2I set read protect operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword access password
 */
TMR_Status TMR_TagOp_init_GEN2_NXP_G2I_SetReadProtect(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword)
{
  tagop->type = TMR_TAGOP_GEN2_NXP_SETREADPROTECT;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_NXP_G2I_SILICON;
  tagop->u.gen2.u.custom.u.nxp.u.setReadProtect.accessPassword = accessPassword;
  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a NXP G2X set read protect operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword access password
 */
TMR_Status TMR_TagOp_init_GEN2_NXP_G2X_SetReadProtect(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword)
{
  tagop->type = TMR_TAGOP_GEN2_NXP_SETREADPROTECT;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_NXP_G2X_SILICON;
  tagop->u.gen2.u.custom.u.nxp.u.setReadProtect.accessPassword = accessPassword;
  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a NXP G2I reset read protect operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword access password
 */
TMR_Status TMR_TagOp_init_GEN2_NXP_G2I_ResetReadProtect(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword)
{
  tagop->type = TMR_TAGOP_GEN2_NXP_RESETREADPROTECT;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_NXP_G2I_SILICON;
  tagop->u.gen2.u.custom.u.nxp.u.resetReadProtect.accessPassword = accessPassword;
  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a NXP G2X reset read protect operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword access password
 */
TMR_Status TMR_TagOp_init_GEN2_NXP_G2X_ResetReadProtect(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword)
{
  tagop->type = TMR_TAGOP_GEN2_NXP_RESETREADPROTECT;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_NXP_G2X_SILICON;
  tagop->u.gen2.u.custom.u.nxp.u.resetReadProtect.accessPassword = accessPassword;
  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a NXP G2I Change EAS operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword access password
 * @param resetEAS   -  if true, Reset EAS
 *                      if false, Set EAS
 */
TMR_Status TMR_TagOp_init_GEN2_NXP_G2I_ChangeEAS(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, bool resetEAS)
{
  tagop->type = TMR_TAGOP_GEN2_NXP_CHANGEEAS;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_NXP_G2I_SILICON;
  tagop->u.gen2.u.custom.u.nxp.u.changeEAS.accessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.nxp.u.changeEAS.reset = resetEAS;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a NXP G2X Change EAS operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword access password
 * @param resetEAS   -  if true, Reset EAS
 *                      if false, Set EAS
 */
TMR_Status TMR_TagOp_init_GEN2_NXP_G2X_ChangeEAS(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, bool resetEAS)
{
  tagop->type = TMR_TAGOP_GEN2_NXP_CHANGEEAS;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_NXP_G2X_SILICON;
  tagop->u.gen2.u.custom.u.nxp.u.changeEAS.accessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.nxp.u.changeEAS.reset = resetEAS;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a NXP G2I EAS alarm operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param dr Gen2 divide ratio to use
 * @param m Gen2 M(tag encoding) parameter to use
 * @param trExt txExt Gen2 TrExt value to use
 */
TMR_Status TMR_TagOp_init_GEN2_NXP_G2I_EASAlarm(TMR_TagOp *tagop, TMR_GEN2_DivideRatio dr, TMR_GEN2_TagEncoding m, TMR_GEN2_TrExt trExt)
{
  tagop->type = TMR_TAGOP_GEN2_NXP_EASALARM;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_NXP_G2I_SILICON;
  tagop->u.gen2.u.custom.u.nxp.u.EASAlarm.dr = dr;
  tagop->u.gen2.u.custom.u.nxp.u.EASAlarm.m = m;
  tagop->u.gen2.u.custom.u.nxp.u.EASAlarm.trExt = trExt;
  return TMR_SUCCESS;
}

TMR_Status TMR_TagOp_init_GEN2_NXP_AES_Untraceable(TMR_TagOp *tagop,TMR_GEN2_UNTRACEABLE_Epc epc, int epclen , TMR_GEN2_UNTRACEABLE_Tid tid, TMR_GEN2_UNTRACEABLE_UserMemory user,
																									 TMR_GEN2_UNTRACEABLE_Range range, TMR_TagOp_GEN2_NXP_Untraceable *untraceable)
{
	tagop->type = TMR_TAGOP_GEN2_NXP_UNTRACEABLE;
	tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_NXP_AES_UCODE;
	tagop->u.gen2.u.custom.u.nxp.u.untraceable.epc = epc;
	tagop->u.gen2.u.custom.u.nxp.u.untraceable.epcLength = epclen;
	tagop->u.gen2.u.custom.u.nxp.u.untraceable.tid = tid;
	tagop->u.gen2.u.custom.u.nxp.u.untraceable.userMemory = user;
	tagop->u.gen2.u.custom.u.nxp.u.untraceable.range = range;
        tagop->u.gen2.u.custom.u.nxp.u.untraceable.auth.authType = untraceable->auth.authType;
	if(untraceable->auth.authType == UNTRACEABLE_WITH_AUTHENTICATION)
	{
		tagop->u.gen2.u.custom.u.nxp.u.untraceable.subCommand = 0x02;
		tagop->u.gen2.u.custom.u.nxp.u.untraceable.auth.tam1Auth = untraceable->auth.tam1Auth;
		tagop->u.gen2.u.custom.u.nxp.u.untraceable.auth.tam1Auth.Authentication |= 0x03;
	}
	else
	{
		tagop->u.gen2.u.custom.u.nxp.u.untraceable.subCommand = 0x03;
		tagop->u.gen2.u.custom.u.nxp.u.untraceable.auth.accessPassword = untraceable->auth.accessPassword;
	}
	return TMR_SUCCESS;
}

TMR_Status TMR_TagOp_init_GEN2_NXP_AES_Authenticate(TMR_TagOp *tagop, TMR_TagOp_GEN2_NXP_Authenticate *authenticate)
{
	tagop->type = TMR_TAGOP_GEN2_NXP_AUTHENTICATE;
	tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_NXP_AES_UCODE;
	if(authenticate->type == TAM1_AUTHENTICATION)
	{
		tagop->u.gen2.u.custom.u.nxp.u.authenticate.type = authenticate->type;
		tagop->u.gen2.u.custom.u.nxp.u.authenticate.subCommand = 0x01;
		tagop->u.gen2.u.custom.u.nxp.u.authenticate.tam1Auth = authenticate->tam1Auth;
		tagop->u.gen2.u.custom.u.nxp.u.authenticate.tam1Auth.Authentication |= 0x03;
	}
	else
	{
		tagop->u.gen2.u.custom.u.nxp.u.authenticate.tam1Auth = authenticate->tam2Auth.tam1Auth;
		tagop->u.gen2.u.custom.u.nxp.u.authenticate.type = authenticate->type;
		tagop->u.gen2.u.custom.u.nxp.u.authenticate.subCommand = 0x01;
		tagop->u.gen2.u.custom.u.nxp.u.authenticate.tam2Auth = authenticate->tam2Auth;
		tagop->u.gen2.u.custom.u.nxp.u.authenticate.tam1Auth.Authentication |= 0x07;
	}
	return TMR_SUCCESS;
}

TMR_Status TMR_TagOp_init_GEN2_NXP_AES_Tam2authentication(TMR_TagOp_GEN2_NXP_Tam2Authentication *auth, TMR_NXP_KeyId keyid, TMR_uint8List *key, 
																		TMR_NXP_Profile profile, uint16_t Offset,uint8_t blockCount, int protMode, bool sendRawData)
{
	auth->tam1Auth.Authentication = sendRawData ? 0x80 : 0x00;
	auth->tam1Auth.CSI = 0x00;
	auth->tam1Auth.KeyLength = 0x10;
	auth->tam1Auth.Key.len = key->len;
	auth->tam1Auth.Key.list = key->list;
	auth->tam1Auth.Key.max = key->max;
	auth->tam1Auth.keyID = keyid;
	auth->Offset = Offset;
	auth->BlockCount = blockCount;
	auth->ProtMode = protMode;
	auth->profile = profile;
	return TMR_SUCCESS;
}
TMR_Status TMR_TagOp_init_GEN2_NXP_AES_ReadBuffer(TMR_TagOp *tagop, uint16_t wordPointer, uint16_t bitCount, TMR_TagOp_GEN2_NXP_Readbuffer *readbuffer)
{
	tagop->type = TMR_TAGOP_GEN2_NXP_READBUFFER;
	tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_NXP_AES_UCODE;
	tagop->u.gen2.u.custom.u.nxp.u.readBuffer.wordPointer = wordPointer;
	tagop->u.gen2.u.custom.u.nxp.u.readBuffer.bitCount = bitCount;
	if(readbuffer->authenticate.type == TAM1_AUTHENTICATION)
	{
		tagop->u.gen2.u.custom.u.nxp.u.readBuffer.authenticate.type = readbuffer->authenticate.type;
		tagop->u.gen2.u.custom.u.nxp.u.readBuffer.authenticate.subCommand = 0x04;
		tagop->u.gen2.u.custom.u.nxp.u.readBuffer.authenticate.tam1Auth = readbuffer->authenticate.tam1Auth;
		tagop->u.gen2.u.custom.u.nxp.u.readBuffer.authenticate.tam1Auth.Authentication |= 0x00;
	}
	else
	{
		tagop->u.gen2.u.custom.u.nxp.u.readBuffer.authenticate.tam1Auth = readbuffer->authenticate.tam2Auth.tam1Auth;
		tagop->u.gen2.u.custom.u.nxp.u.readBuffer.authenticate.type = readbuffer->authenticate.type;
		tagop->u.gen2.u.custom.u.nxp.u.readBuffer.authenticate.subCommand = 0x04;
		tagop->u.gen2.u.custom.u.nxp.u.readBuffer.authenticate.tam2Auth = readbuffer->authenticate.tam2Auth;
		tagop->u.gen2.u.custom.u.nxp.u.readBuffer.authenticate.tam1Auth.Authentication |= 0x04;
	}
	return TMR_SUCCESS;
}

TMR_Status TMR_TagOp_init_GEN2_NXP_AES_Tam1authentication(TMR_TagOp_GEN2_NXP_Tam1Authentication *auth, TMR_NXP_KeyId keyid, TMR_uint8List *key, bool sendRawData )
{
	auth->Authentication = sendRawData ? 0x80 : 0x00;
	auth->CSI = 0x00;
	auth->KeyLength = 0x10;
	auth->Key.len = key->len;
	auth->Key.list = key->list;
	auth->Key.max = key->max;
	auth->keyID = keyid;
	return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a NXP G2X EAS alarm operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param dr Gen2 divide ratio to use
 * @param m Gen2 M(tag encoding) parameter to use
 * @param trExt txExt Gen2 TrExt value to use
 */
TMR_Status TMR_TagOp_init_GEN2_NXP_G2X_EASAlarm(TMR_TagOp *tagop, TMR_GEN2_DivideRatio dr, TMR_GEN2_TagEncoding m, TMR_GEN2_TrExt trExt)
{
  tagop->type = TMR_TAGOP_GEN2_NXP_EASALARM;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_NXP_G2X_SILICON;
  tagop->u.gen2.u.custom.u.nxp.u.EASAlarm.dr = dr;
  tagop->u.gen2.u.custom.u.nxp.u.EASAlarm.m = m;
  tagop->u.gen2.u.custom.u.nxp.u.EASAlarm.trExt = trExt;
  
  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a NXP G2i Calibrate operation with the provided parameters.
 *
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword access password
 */
TMR_Status TMR_TagOp_init_GEN2_NXP_G2I_Calibrate(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword)
{
  tagop->type = TMR_TAGOP_GEN2_NXP_CALIBRATE;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_NXP_G2I_SILICON;
  tagop->u.gen2.u.custom.u.nxp.u.calibrate.accessPassword = accessPassword;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a NXP G2x Calibrate operation with the provided parameters.
 *
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword access password
 */
TMR_Status TMR_TagOp_init_GEN2_NXP_G2X_Calibrate(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword)
{
  tagop->type = TMR_TAGOP_GEN2_NXP_CALIBRATE;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_NXP_G2X_SILICON;
  tagop->u.gen2.u.custom.u.nxp.u.calibrate.accessPassword = accessPassword;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a NXP G2i Change Config operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword access password
 * @param configWord ConfigWord to write to the tag.
 */
TMR_Status TMR_TagOp_init_GEN2_NXP_G2I_ChangeConfig(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, TMR_NXP_ConfigWord configWord)
{
  tagop->type = TMR_TAGOP_GEN2_NXP_CHANGECONFIG;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_NXP_G2I_SILICON;
  tagop->u.gen2.u.custom.u.nxp.u.changeConfig.accessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.nxp.u.changeConfig.configWord = configWord;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a NXP G2x Change Config operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword access password
 * @param configWord ConfigWord to write to the tag.
 */
TMR_Status TMR_TagOp_init_GEN2_NXP_G2X_ChangeConfig(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, TMR_NXP_ConfigWord configWord)
{
  tagop->type = TMR_TAGOP_GEN2_NXP_CHANGECONFIG;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_NXP_G2X_SILICON;
  tagop->u.gen2.u.custom.u.nxp.u.changeConfig.accessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.nxp.u.changeConfig.configWord = configWord;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a NXP UCODE7 Change Config operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword access password
 * @param configWord ConfigWord to write to the tag.
 */
TMR_Status TMR_TagOp_init_GEN2_NXP_UCODE7_ChangeConfig(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, TMR_NXP_UCODE7_ConfigWord configWord)
{
  tagop->type = TMR_TAGOP_GEN2_NXP_UCODE7_CHANGECONFIG;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_NXP_UCODE7_SILICON;
  tagop->u.gen2.u.custom.u.nxp.u.ucode7ChangeConfig.accessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.nxp.u.ucode7ChangeConfig.configWord = configWord;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Monza4 QT Read/Write operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword access password
 * @param controlByte The QT control Byte 
 * @param payload The QT payload
 */
TMR_Status 
TMR_TagOp_init_GEN2_Impinj_Monza4_QTReadWrite(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                  TMR_Monza4_ControlByte controlByte, TMR_Monza4_Payload payload)
{
  tagop->type = TMR_TAGOP_GEN2_IMPINJ_MONZA4_QTREADWRITE;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IMPINJ_MONZA4_SILICON;
  tagop->u.gen2.u.custom.u.impinj.u.monza4.u.qtReadWrite.accessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.impinj.u.monza4.u.qtReadWrite.controlByte = controlByte;
  tagop->u.gen2.u.custom.u.impinj.u.monza4.u.qtReadWrite.payload = payload;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Monza6 Margin read operation with the provided parameters.
 * @param tagop: Pointer to the tagop structure to initialize.
 * @param bank: The Gen2 memory bank to read from
 * @param bitAddress: The bit address to start reading from 
 * @param maskBitLength: The number of mask bits
 * @param mask: Pointer to mask data
 */
TMR_Status
TMR_TagOp_init_GEN2_Impinj_Monza6_MarginRead(TMR_TagOp *tagop, TMR_GEN2_Bank bank,
                                   uint32_t bitAddress, uint16_t maskBitLength, uint8_t *mask)
{

  tagop->type = TMR_TAGOP_GEN2_IMPINJ_MONZA6_MARGINREAD;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IMPINJ_MONZA6_SILICON;
  tagop->u.gen2.u.custom.u.impinj.u.monza6.u.marginRead.bank = bank;
  tagop->u.gen2.u.custom.u.impinj.u.monza6.u.marginRead.bitAddress = bitAddress;
  tagop->u.gen2.u.custom.u.impinj.u.monza6.u.marginRead.maskBitLength = maskBitLength;
  tagop->u.gen2.u.custom.u.impinj.u.monza6.u.marginRead.mask = mask;

  return TMR_SUCCESS;
}

 /**
 * Initialize a TMR_TagOp as a Gen2 IAVDenatran ActivateSecureMode operation with the provided parameter
 * @param tagop Pointer to the tagop structure to initialize
 * @param payload 1byte->[TC(Transmission Count) 1bit + RFFU(Reserved For Furture Use) 7bits]
 */
TMR_Status
TMR_TagOp_init_GEN2_Denatran_IAV_Activate_Secure_Mode(TMR_TagOp *tagop, uint8_t payload)
{
  tagop->type = TMR_TAGOP_GEN2_DENATRAN_IAV_ACTIVATESECUREMODE;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_DENATRAN_IAV_SILICON; 
  tagop->u.gen2.u.custom.u.IavDenatran.u.secureMode.mode = GEN2_ACTIVATE_SECURE_MODE;
  tagop->u.gen2.u.custom.u.IavDenatran.u.secureMode.payload = payload;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 IAVDenatran AuthenticateOBU operation with the provided parameter
 * @param tagop Pointer to the tagop structure to initialize 
 * @param payload 1byte->[TC(Transmission Count) 1bit + RFFU(Reserved For Furture Use) 7bits]
 */
TMR_Status
TMR_TagOp_init_GEN2_Denatran_IAV_Authenticate_OBU(TMR_TagOp *tagop, uint8_t payload)
{
  tagop->type = TMR_TAGOP_GEN2_DENATRAN_IAV_AUTHENTICATEOBU;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_DENATRAN_IAV_SILICON; 
  tagop->u.gen2.u.custom.u.IavDenatran.u.authenticateOBU.mode = GEN2_AUTHENTICATE_OBU;
  tagop->u.gen2.u.custom.u.IavDenatran.u.authenticateOBU.payload = payload;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 IAVDenatran ACTIVATE_SINIAV_MODE operation with the provided parameter
 * @param tagop Pointer to the tagop structure to initialize 
 * @param payload 1byte->[TC(Transmission Count) 1bit + RFFU(Reserved For Furture Use) 7bits]
 * @param data 64 bits of token number to activate the tag
 */
TMR_Status
TMR_TagOp_init_GEN2_Denatran_IAV_Activate_Siniav_Mode(TMR_TagOp *tagop, uint8_t payload, TMR_uint8List *token)
{
  uint8_t tokenDesc;

  tagop->type = TMR_TAGOP_GEN2_ACTIVATE_SINIAV_MODE;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_DENATRAN_IAV_SILICON; 
  tagop->u.gen2.u.custom.u.IavDenatran.u.activateSiniavMode.mode = GEN2_ACTIVATE_SINIAV_MODE;
  tagop->u.gen2.u.custom.u.IavDenatran.u.activateSiniavMode.payload = payload;

  /**
   * Currently last two bits of the payload is used as TokenDesc
   * (Token Descriptor): 2 bits parameter indicating the presence and format of Token
   * 00 : No Token.
   * 01 : Token of 64 bits.
   */
  tokenDesc = 0x03 & payload;
  if (0x01 == tokenDesc)
  { 
    /* the token field is always 64 bits in this case */    
    if (0x08 != token->len)
    {
      return TMR_ERROR_INVALID;
    }
    memcpy(tagop->u.gen2.u.custom.u.IavDenatran.u.activateSiniavMode.token, token->list, token->len);
    tagop->u.gen2.u.custom.u.IavDenatran.u.activateSiniavMode.isTokenDesc = true;  
  }
  else
  {
    tagop->u.gen2.u.custom.u.IavDenatran.u.activateSiniavMode.isTokenDesc = false;
  }

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 IAVDenatran OBU_Auth_ID operation with the provided parameter
 * @param tagop Pointer to the tagop structure to initialize 
 * @param payload 1byte->[TC(Transmission Count) 1bit + RFFU(Reserved For Furture Use) 7bits] 
 */
TMR_Status
TMR_TagOp_init_GEN2_Denatran_IAV_OBU_Auth_ID(TMR_TagOp *tagop, uint8_t payload)
{
  tagop->type = TMR_TAGOP_GEN2_OBU_AUTH_ID;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_DENATRAN_IAV_SILICON; 
  tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthId.mode = GEN2_OBU_AUTH_ID;
  tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthId.payload = payload;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 IAVDenatran OBU_Auth_Full_Pass1 operation with the provided parameter
 * @param tagop Pointer to the tagop structure to initialize 
 * @param payload 1byte->[TC(Transmission Count) 1bit + RFFU(Reserved For Furture Use) 7bits] 
 */
TMR_Status
TMR_TagOp_init_GEN2_Denatran_IAV_OBU_Auth_Full_Pass1(TMR_TagOp *tagop, uint8_t payload)
{
  tagop->type = TMR_TAGOP_GEN2_AUTHENTICATE_OBU_FULL_PASS1;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_DENATRAN_IAV_SILICON; 
  tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthFullPass1.mode = GEN2_AUTHENTICATE_OBU_FULL_PASS1;
  tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthFullPass1.payload = payload;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 IAVDenatran OBU_Auth_Full_Pass2 operation with the provided parameter
 * @param tagop Pointer to the tagop structure to initialize 
 * @param payload 1byte->[TC(Transmission Count) 1bit + RFFU(Reserved For Furture Use) 7bits] 
 */
TMR_Status
TMR_TagOp_init_GEN2_Denatran_IAV_OBU_Auth_Full_Pass2(TMR_TagOp *tagop, uint8_t payload)
{
  tagop->type = TMR_TAGOP_GEN2_AUTHENTICATE_OBU_FULL_PASS2;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_DENATRAN_IAV_SILICON; 
  tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthFullPass2.mode = GEN2_AUTHENTICATE_OBU_FULL_PASS2;
  tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthFullPass2.payload = payload;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 IAVDenatran OBU_ReadFromMemMap operation with the provided parameter
 * @param tagop Pointer to the tagop structure to initialize 
 * @param payload The OBU_ReadFromMemMap Payload
 * @param wordAddress pointer indicating the address to be read from USER memory bank
 */
TMR_Status
TMR_TagOp_init_GEN2_Denatran_IAV_OBU_ReadFromMemMap(TMR_TagOp *tagop, uint8_t payload, uint16_t wordAddress)
{
  tagop->type = TMR_TAGOP_GEN2_OBU_READ_FROM_MEM_MAP;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_DENATRAN_IAV_SILICON;
  tagop->u.gen2.u.custom.u.IavDenatran.u.obuReadFromMemMap.mode = GEN2_OBU_READ_FROM_MEM_MAP;
  tagop->u.gen2.u.custom.u.IavDenatran.u.obuReadFromMemMap.payload = payload;
  tagop->u.gen2.u.custom.u.IavDenatran.u.obuReadFromMemMap.readPtr = wordAddress;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 IAVDenatran OBU_WriteToMemMap operation with the provided parameter
 * @param tagop Pointer to the tagop structure to initialize 
 * @param payload The OBU_WriteToMemMap Payload
 * @param wordAddress pointer to the USER data
 * @param word data to be written
 * @param data credentials written word
 */
TMR_Status
TMR_TagOp_init_GEN2_Denatran_IAV_OBU_WriteToMemMap(TMR_TagOp *tagop, uint8_t payload, uint16_t wordAddress, uint16_t word, uint8_t* tagId, uint8_t* data)
{
  tagop->type = TMR_TAGOP_GEN2_OBU_WRITE_TO_MEM_MAP;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_DENATRAN_IAV_SILICON;
  tagop->u.gen2.u.custom.u.IavDenatran.u.obuWriteToMemMap.mode = GEN2_OBU_WRITE_TO_MEM_MAP;
  tagop->u.gen2.u.custom.u.IavDenatran.u.obuWriteToMemMap.payload = payload;
  tagop->u.gen2.u.custom.u.IavDenatran.u.obuWriteToMemMap.writePtr = wordAddress;
  tagop->u.gen2.u.custom.u.IavDenatran.u.obuWriteToMemMap.wordData = word;
  tagop->u.gen2.u.custom.u.IavDenatran.u.obuWriteToMemMap.tagIdentification = tagId;
  tagop->u.gen2.u.custom.u.IavDenatran.u.obuWriteToMemMap.dataBuf = data;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 IAVDenatran Write Sec operation with the provided parameter
 * @param tagop Pointer to the tagop structure to initialize 
 * @param payload The OBU_WriteToMemMap Payload
 * @param wordAddress pointer to the USER data
 * @param dataWords 4 data words
 * @param data credentials written word
 */
TMR_Status
TMR_TagOp_init_GEN2_Denatran_IAV_WriteSec(TMR_TagOp *tagop, uint8_t payload, uint8_t* data, uint8_t* credentials)
{
  tagop->type = TMR_TAGOP_GEN2_DENATRAN_IAV_WRITE_SEC;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_DENATRAN_IAV_SILICON;
  tagop->u.gen2.u.custom.u.IavDenatran.u.writeSec.mode = GEN2_WRITE_SEC;
  tagop->u.gen2.u.custom.u.IavDenatran.u.writeSec.payload = payload;
  tagop->u.gen2.u.custom.u.IavDenatran.u.writeSec.dataWords = data;
  tagop->u.gen2.u.custom.u.IavDenatran.u.writeSec.dataBuf = credentials;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 IAVDenatran OBU_Auth_Full_Pass operation with the provided parameter
 * @param tagop Pointer to the tagop structure to initialize 
 * @param payload 1byte->[TC(Transmission Count) 1bit + RFFU(Reserved For Furture Use) 7bits] 
 */
TMR_Status
TMR_TagOp_init_GEN2_Denatran_IAV_OBU_Auth_Full_Pass(TMR_TagOp *tagop, uint8_t payload)
{
  tagop->type = TMR_TAGOP_GEN2_DENATRAN_IAV_AUTHENTICATE_OBU_FULL_PASS;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_DENATRAN_IAV_SILICON; 
  tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthFullPass.mode = GEN2_AUTHENTICATE_OBU_FULL_PASS;
  tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthFullPass.payload = payload;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 IAVDenatran G0_PA_OBU_Auth_ID operation with the provided parameter
 * @param tagop Pointer to the tagop structure to initialize 
 * @param payload 1byte->[TC(Transmission Count) 1bit + RFFU(Reserved For Furture Use) 7bits] 
 */
TMR_Status
TMR_TagOp_init_GEN2_Denatran_IAV_G0_PA_OBU_Auth(TMR_TagOp *tagop, uint8_t payload)
{
  tagop->type = TMR_TAGOP_GEN2_DENATRAN_IAV_G0_PA_OBU_AUTHENTICATE_ID;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_DENATRAN_IAV_SILICON; 
  tagop->u.gen2.u.custom.u.IavDenatran.u.g0paobuauthid.mode = GEN2_PA_G0_AUTHENTICATE;
  tagop->u.gen2.u.custom.u.IavDenatran.u.g0paobuauthid.payload = payload;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 IAVDenatran Get Token Id operation with the provided parameter
 * @param tagop Pointer to the tagop structure to initialize 
 */
TMR_Status
TMR_TagOp_init_GEN2_Denatran_IAV_GetTokenId(TMR_TagOp *tagop)
{
  tagop->type = TMR_TAGOP_GEN2_DENATRAN_IAV_GET_TOKEN_ID;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_DENATRAN_IAV_SILICON;
  tagop->u.gen2.u.custom.u.IavDenatran.u.getTokenId.mode = GEN2_GET_TOKEN_ID;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 IAVDenatran read sec operation with the provided parameter
 * @param tagop Pointer to the tagop structure to initialize
 * @param payLoad 8 bits for future use
 * @param wordAddress pointer indicating the sector to be read from USER memory bank
 */
TMR_Status
TMR_TagOp_init_GEN2_Denatran_IAV_ReadSec(TMR_TagOp *tagop, uint8_t payload, uint16_t wordAddress)
{
  tagop->type = TMR_TAGOP_GEN2_DENATRAN_IAV_READ_SEC;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_DENATRAN_IAV_SILICON;
  tagop->u.gen2.u.custom.u.IavDenatran.u.readSec.mode = GEN2_READ_SEC;
  tagop->u.gen2.u.custom.u.IavDenatran.u.readSec.payload = payload;
  tagop->u.gen2.u.custom.u.IavDenatran.u.readSec.readPtr = wordAddress;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 Ids SL900A GetCalibrationData value operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword Gen2 accessPassword
 * @param password IDS password
 * @param level IDS passwordlevel
 */
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_GetCalibrationData(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                                  PasswordLevel level, uint32_t password)
{
  tagop->type = TMR_TAGOP_GEN2_IDS_SL900A_GETCALIBRATIONDATA;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IDS_SL900A_SILICON;
  tagop->u.gen2.u.custom.u.ids.u.calibrationData.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.ids.u.calibrationData.CommandCode = 0xA9;
  tagop->u.gen2.u.custom.u.ids.u.calibrationData.Password = password;
  tagop->u.gen2.u.custom.u.ids.u.calibrationData.sl900A.level = level;

  return TMR_SUCCESS;
}

/**
 * Initialize  a TMR_TagOp as a GEn2 IDs SL900A SetPassword value with
 * @param tagop pointer to the tagop struture to SetPassword
 * @param accessPassword Gen2 accessPassword
 * @param level IDS passwordLevel
 * @param password IDS password
 * @param newPasswordLevel IDS NewPasswordLevel
 * @param newPassword IDS NewPassword
 */ 
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_SetPassword(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, PasswordLevel level,
                                           uint32_t password, PasswordLevel newPasswordLevel, uint32_t newPassword)
{
  tagop->type = TMR_TAGOP_GEN2_IDS_SL900A_SETPASSWORD;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IDS_SL900A_SILICON;
  tagop->u.gen2.u.custom.u.ids.u.setPassword.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.ids.u.setPassword.CommandCode = 0xA0;
  tagop->u.gen2.u.custom.u.ids.u.setPassword.sl900A.level = level;
  tagop->u.gen2.u.custom.u.ids.u.setPassword.Password = password;
  tagop->u.gen2.u.custom.u.ids.u.setPassword.NewPasswordLevel = newPasswordLevel;
  tagop->u.gen2.u.custom.u.ids.u.setPassword.NewPassword = newPassword;
  
  return TMR_SUCCESS;
}
/**
 * Initialize a TMR_TagOp as a Gen2 Ids SL900A SetCalibrationData value operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword Gen2 accessPassword
 * @param password IDS password
 * @param level IDS passwordlevel
 * @param data IDS calibration data
 */
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_SetCalibrationData(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, PasswordLevel level, 
                                                  uint32_t password, TMR_TagOp_GEN2_IDS_SL900A_CalibrationData *data)
{
  tagop->type = TMR_TAGOP_GEN2_IDS_SL900A_SETCALIBRATIONDATA;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IDS_SL900A_SILICON;
  tagop->u.gen2.u.custom.u.ids.u.setCalibration.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.ids.u.setCalibration.CommandCode = 0xA5;
  tagop->u.gen2.u.custom.u.ids.u.setCalibration.Password = password;
  tagop->u.gen2.u.custom.u.ids.u.setCalibration.sl900A.level = level;
  tagop->u.gen2.u.custom.u.ids.u.setCalibration.cal.raw = data->raw;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 Ids SL900A SetSfeParameters value operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword Gen2 accessPassword
 * @param password IDS password
 * @param level IDS passwordlevel
 * @param data IDS sfe parameters
 */
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_SetSfeParameters(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, PasswordLevel level,
                                                uint32_t password, TMR_TagOp_GEN2_IDS_SL900A_SfeParameters *data)
{
  tagop->type = TMR_TAGOP_GEN2_IDS_SL900A_SETSFEPARAMETERS;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IDS_SL900A_SILICON;
  tagop->u.gen2.u.custom.u.ids.u.setSfeParameters.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.ids.u.setSfeParameters.CommandCode = 0xA4;
  tagop->u.gen2.u.custom.u.ids.u.setSfeParameters.Password = password;
  tagop->u.gen2.u.custom.u.ids.u.setSfeParameters.sl900A.level = level;
  tagop->u.gen2.u.custom.u.ids.u.setSfeParameters.sfe = data;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 Ids SL900A GetSensor value operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword Gen2 accessPassword
 * @param password IDS password
 * @param level IDS passwordlevel
 * @param type IDS sensor type
 */
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_GetSensorValue(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                              PasswordLevel level, uint32_t password, Sensor type)
{
  tagop->type = TMR_TAGOP_GEN2_IDS_SL900A_GETSENSOR;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IDS_SL900A_SILICON;
  tagop->u.gen2.u.custom.u.ids.u.sensor.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.ids.u.sensor.CommandCode = 0xAD;
  tagop->u.gen2.u.custom.u.ids.u.sensor.Password = password;
  tagop->u.gen2.u.custom.u.ids.u.sensor.sl900A.level = level;
  tagop->u.gen2.u.custom.u.ids.u.sensor.sl900A.sensortype = type;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 Ids SL900A GetMeasurementSetup operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword Gen2 accessPassword
 * @param password IDS password
 * @param level IDS passwordlevel
 */
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_GetMeasurementSetup(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                                   PasswordLevel level, uint32_t password)
{
  tagop->type =TMR_TAGOP_GEN2_IDS_SL900A_GETMEASUREMENTSETUP;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IDS_SL900A_SILICON;
  tagop->u.gen2.u.custom.u.ids.u.measurementSetup.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.ids.u.measurementSetup.CommandCode = 0xA3;
  tagop->u.gen2.u.custom.u.ids.u.measurementSetup.Password = password;
  tagop->u.gen2.u.custom.u.ids.u.measurementSetup.sl900A.level = level;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 Ids SL900A GetLogState operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword Gen2 accessPassword
 * @param password IDS password
 * @param level IDS passwordlevel
 */
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_GetLogState(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                           PasswordLevel level, uint32_t password)
{
  tagop->type = TMR_TAGOP_GEN2_IDS_SL900A_GETLOGSTATE;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IDS_SL900A_SILICON;
  tagop->u.gen2.u.custom.u.ids.u.getLog.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.ids.u.getLog.CommandCode = 0xA8;
  tagop->u.gen2.u.custom.u.ids.u.getLog.Password = password;
  tagop->u.gen2.u.custom.u.ids.u.getLog.sl900A.level = level;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 Ids SL900A Set Log Mode operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword Gen2 accessPassword
 * @param password IDS password
 * @param level IDS passwordlevel
 * @param form IDS logging form
 * @param rule IDS storage rule
 * @param Ext1Enable to Enable log for EXT1 external sensor
 * @param Ext2Enable to Enable log for EXT2 external sensor
 * @param TempEnable to Enable log for temperature sensor
 * @param BattEnable to Enable log for battery sensor
 * @param LogInterval to Time (seconds) between log readings
 */
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_SetLogMode(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, PasswordLevel level,
                                          uint32_t password, LoggingForm form, StorageRule rule, bool Ext1Enable,
                                          bool Ext2Enable,  bool TempEnable, bool BattEnable, uint16_t LogInterval)
{
  tagop->type = TMR_TAGOP_GEN2_IDS_SL900A_SETLOGMODE;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IDS_SL900A_SILICON;
  tagop->u.gen2.u.custom.u.ids.u.setLogMode.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.ids.u.setLogMode.CommandCode = 0xA1;
  tagop->u.gen2.u.custom.u.ids.u.setLogMode.Password = password;
  tagop->u.gen2.u.custom.u.ids.u.setLogMode.sl900A.level = level;
  tagop->u.gen2.u.custom.u.ids.u.setLogMode.sl900A.dataLog = form;
  tagop->u.gen2.u.custom.u.ids.u.setLogMode.sl900A.rule = rule;
  tagop->u.gen2.u.custom.u.ids.u.setLogMode.Ext1Enable = Ext1Enable;
  tagop->u.gen2.u.custom.u.ids.u.setLogMode.Ext2Enable = Ext2Enable;
  tagop->u.gen2.u.custom.u.ids.u.setLogMode.TempEnable = TempEnable;
  tagop->u.gen2.u.custom.u.ids.u.setLogMode.BattEnable = BattEnable;
  tagop->u.gen2.u.custom.u.ids.u.setLogMode.LogInterval = LogInterval;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 Ids SL900A EndLog operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword Gen2 accessPassword
 * @param password IDS password
 * @param level IDS passwordlevel
 */
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_EndLog(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                      PasswordLevel level, uint32_t password)
{
  tagop->type = TMR_TAGOP_GEN2_IDS_SL900A_ENDLOG;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IDS_SL900A_SILICON;
  tagop->u.gen2.u.custom.u.ids.u.endLog.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.ids.u.endLog.CommandCode = 0xA6;
  tagop->u.gen2.u.custom.u.ids.u.endLog.Password = password;
  tagop->u.gen2.u.custom.u.ids.u.endLog.sl900A.level = level;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 Ids SL900A initialize operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword Gen2 accessPassword
 * @param level IDS passwordlevel
 * @param password IDS password
 * @param delayMode IDS delaymode
 * @param delayTime specifying IDS Delay time
 * @param timeEnable IDS timeenable
 * @param numberOfWords specifying Number of user bank memory words to reserve
 * @param BrokenWordPointer IDS broken word pointer
 */
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_Initialize(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                          PasswordLevel level, uint32_t password, uint8_t delayMode, 
                                          uint16_t delayTime, bool timeEnable, uint16_t numberOfWords, 
                                          uint8_t BrokenWordPointer)
{
  uint16_t mask = 0x1 << 1;

  tagop->type = TMR_TAGOP_GEN2_IDS_SL900A_INITIALIZE;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IDS_SL900A_SILICON;
  tagop->u.gen2.u.custom.u.ids.u.initialize.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.ids.u.initialize.CommandCode = 0xAC;
  tagop->u.gen2.u.custom.u.ids.u.initialize.Password = password;
  tagop->u.gen2.u.custom.u.ids.u.initialize.sl900A.level = level;

  if (TMR_GEN2_IDS_SL900A_DELAYMODE_TIMER == delayMode)
  {   
   uint16_t raw = 0;
   raw &= (uint16_t)~mask;
   tagop->u.gen2.u.custom.u.ids.u.initialize.delayTime.raw = raw;
   tagop->u.gen2.u.custom.u.ids.u.initialize.delayTime.Mode = (DelayMode)delayMode;
  }
  else
  {
    tagop->u.gen2.u.custom.u.ids.u.initialize.delayTime.raw |= mask;
    tagop->u.gen2.u.custom.u.ids.u.initialize.delayTime.Mode = (DelayMode)delayMode;
  }

  if (delayTime)
  {
    uint16_t maskDelayTime = 0xFFF << 4;
    tagop->u.gen2.u.custom.u.ids.u.initialize.delayTime.raw &= (uint16_t)~maskDelayTime;
    tagop->u.gen2.u.custom.u.ids.u.initialize.delayTime.raw |= (uint16_t)(delayTime << 4);
    tagop->u.gen2.u.custom.u.ids.u.initialize.delayTime.Time = delayTime;
  }
  mask = 0x1;
  if(timeEnable)
  {
    tagop->u.gen2.u.custom.u.ids.u.initialize.delayTime.raw |= mask;
    tagop->u.gen2.u.custom.u.ids.u.initialize.delayTime.IrqTimerEnable = timeEnable;
  }
  else
  {
    tagop->u.gen2.u.custom.u.ids.u.initialize.delayTime.raw &= (uint16_t)~mask;
    tagop->u.gen2.u.custom.u.ids.u.initialize.delayTime.IrqTimerEnable = timeEnable;
  }

  mask = 0x1FF;
  if (numberOfWords)
  {
    tagop->u.gen2.u.custom.u.ids.u.initialize.applicationData.raw &= (uint16_t)~mask;
    tagop->u.gen2.u.custom.u.ids.u.initialize.applicationData.raw |= (uint16_t)(numberOfWords << 7);
    tagop->u.gen2.u.custom.u.ids.u.initialize.applicationData.NumberOfWords = numberOfWords;
  }

  mask = 0x7;
  if (BrokenWordPointer)
  {
    tagop->u.gen2.u.custom.u.ids.u.initialize.applicationData.raw &= (uint16_t)~mask;
    tagop->u.gen2.u.custom.u.ids.u.initialize.applicationData.raw |= (uint16_t)(BrokenWordPointer);
    tagop->u.gen2.u.custom.u.ids.u.initialize.applicationData.BrokenWordPointer = BrokenWordPointer;
  }

  return TMR_SUCCESS;
}


/**
 * Initialize a TMR_TagOp as a Gen2 Ids SL900A AcessFifo Status operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword Gen2 accessPassword
 * @param password IDS password
 * @param level IDS passwordlevel
 */
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_AccessFifoStatus(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                                 PasswordLevel level, uint32_t password)
{
  tagop->type = TMR_TAGOP_GEN2_IDS_SL900A_ACCESSFIFOSTATUS;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IDS_SL900A_SILICON;
  tagop->u.gen2.u.custom.u.ids.u.accessFifoStatus.status.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.ids.u.accessFifoStatus.status.CommandCode = 0xAF;
  tagop->u.gen2.u.custom.u.ids.u.accessFifoStatus.status.Password = password;
  tagop->u.gen2.u.custom.u.ids.u.accessFifoStatus.status.sl900A.level = level;
  tagop->u.gen2.u.custom.u.ids.u.accessFifoStatus.status.operation = TMR_GEN2_IDS_SL900A_ACCESSFIFO_STATUS;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 Ids SL900A AcessFifo Read operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword Gen2 accessPassword
 * @param password IDS password
 * @param level IDS passwordlevel
 * @param readLength specify no of data to be read from fifo
 */
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_AccessFifoRead(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                               PasswordLevel level, uint32_t password, uint8_t readLength)
{
  tagop->type = TMR_TAGOP_GEN2_IDS_SL900A_ACCESSFIFOREAD;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IDS_SL900A_SILICON;
  tagop->u.gen2.u.custom.u.ids.u.accessFifoRead.read.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.ids.u.accessFifoRead.read.CommandCode = 0xAF;
  tagop->u.gen2.u.custom.u.ids.u.accessFifoRead.read.Password = password;
  tagop->u.gen2.u.custom.u.ids.u.accessFifoRead.read.sl900A.level = level;
  tagop->u.gen2.u.custom.u.ids.u.accessFifoRead.read.operation = TMR_GEN2_IDS_SL900A_ACCESSFIFO_READ;
  
  if (readLength != (readLength & 0xF))
  {
    return TMR_ERROR_MSG_INVALID_PARAMETER_VALUE;
  }
  tagop->u.gen2.u.custom.u.ids.u.accessFifoRead.length = readLength;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 Ids SL900A AcessFifo Write operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword Gen2 accessPassword
 * @param password IDS password
 * @param level IDS passwordlevel
 * @param payLoad specify  data to be written into fifo
 */
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_AccessFifoWrite(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                               PasswordLevel level, uint32_t password, TMR_uint8List *payLoad)
{
  tagop->type = TMR_TAGOP_GEN2_IDS_SL900A_ACCESSFIFOWRITE;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IDS_SL900A_SILICON;
  tagop->u.gen2.u.custom.u.ids.u.accessFifoWrite.write.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.ids.u.accessFifoWrite.write.CommandCode = 0xAF;
  tagop->u.gen2.u.custom.u.ids.u.accessFifoWrite.write.Password = password;
  tagop->u.gen2.u.custom.u.ids.u.accessFifoWrite.write.sl900A.level = level;
  tagop->u.gen2.u.custom.u.ids.u.accessFifoWrite.write.operation = TMR_GEN2_IDS_SL900A_ACCESSFIFO_WRITE;
  
  if (payLoad->len != (payLoad->len & 0xF))
  {
    return TMR_ERROR_MSG_INVALID_PARAMETER_VALUE;
  }
  tagop->u.gen2.u.custom.u.ids.u.accessFifoWrite.payLoad = payLoad;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 Ids SL900A  StartLog operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword Gen2 accessPassword
 * @param level IDS passwordlevel
 * @param password IDS password
 * @param timestamp pointer to TMR_TimeStructure (timestamp structure) 
 */
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_StartLog(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                        PasswordLevel level, uint32_t password, TMR_TimeStructure *timestamp)
{
  tagop->type = TMR_TAGOP_GEN2_IDS_SL900A_STARTLOG;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IDS_SL900A_SILICON;
  tagop->u.gen2.u.custom.u.ids.u.startLog.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.ids.u.startLog.CommandCode = 0xA7;
  tagop->u.gen2.u.custom.u.ids.u.startLog.Password = password;
  tagop->u.gen2.u.custom.u.ids.u.startLog.sl900A.level = level;
  if (NULL == timestamp)
  {
    /** in case user not providing the time stamp
    * use current system time
    */
    return TMR_ERROR_TIMESTAMP_NULL;
  }
  
  tagop->u.gen2.u.custom.u.ids.u.startLog.startTime = 0;
  tagop->u.gen2.u.custom.u.ids.u.startLog.startTime |= (uint32_t)((timestamp->tm_year - 2010) << 26);
  tagop->u.gen2.u.custom.u.ids.u.startLog.startTime |= (uint32_t)((timestamp->tm_mon) << 22);
  tagop->u.gen2.u.custom.u.ids.u.startLog.startTime |= (uint32_t)(timestamp->tm_mday << 17);
  tagop->u.gen2.u.custom.u.ids.u.startLog.startTime |= (uint32_t)(timestamp->tm_hour << 12);
  tagop->u.gen2.u.custom.u.ids.u.startLog.startTime |= (uint32_t)(timestamp->tm_min << 6);
  tagop->u.gen2.u.custom.u.ids.u.startLog.startTime |= (uint32_t)(timestamp->tm_sec);

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 Ids SL900A GetBatteryLevel operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param accessPassword Gen2 accessPassword
 * @param level IDS passwordlevel
 * @param password IDS password
 * @param type IDS batterytype
 */
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_GetBatteryLevel(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                               PasswordLevel level, uint32_t password, BatteryType type)
{
  tagop->type = TMR_TAGOP_GEN2_IDS_SL900A_GETBATTERYLEVEL;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IDS_SL900A_SILICON;
  tagop->u.gen2.u.custom.u.ids.u.batteryLevel.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.ids.u.batteryLevel.CommandCode = 0xAA;
  tagop->u.gen2.u.custom.u.ids.u.batteryLevel.Password = password;
  tagop->u.gen2.u.custom.u.ids.u.batteryLevel.sl900A.level = level;
  tagop->u.gen2.u.custom.u.ids.u.batteryLevel.batteryType = type;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 Ids SL900A Set Log Limits operation with
 * @param tagop Pointer to the tagop struture to initialize.
 * @param accessPassword Gen2 accessPassword
 * @param level IDS apsswordlevel
 * @param passowrd IDS password
 * @param logLimits pointer to IDS LogLimits
 */
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_SetLogLimit(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, PasswordLevel level,
                                           uint32_t password, LogLimits *logLimits)
{
  tagop->type = TMR_TAGOP_GEN2_IDS_SL900A_SETLOGLIMITS;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IDS_SL900A_SILICON;
  tagop->u.gen2.u.custom.u.ids.u.setLogLimit.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.ids.u.setLogLimit.CommandCode = 0xA2;
  tagop->u.gen2.u.custom.u.ids.u.setLogLimit.Password = password;
  tagop->u.gen2.u.custom.u.ids.u.setLogLimit.sl900A.level = level;

  /**
   * LogLimit values are only 10 bit long,
   */
  if (0x03FF < logLimits->extremeLower)
  {
    return TMR_ERROR_MSG_INVALID_PARAMETER_VALUE; 
  }

  tagop->u.gen2.u.custom.u.ids.u.setLogLimit.limit.extremeLower = logLimits->extremeLower;

  if (0x03FF < logLimits->lower)
  {
    return TMR_ERROR_MSG_INVALID_PARAMETER_VALUE;
  }
  tagop->u.gen2.u.custom.u.ids.u.setLogLimit.limit.lower = logLimits->lower;

  if (0x03FF < logLimits->upper)
  {
    return TMR_ERROR_MSG_INVALID_PARAMETER_VALUE;
  }
  tagop->u.gen2.u.custom.u.ids.u.setLogLimit.limit.upper = logLimits->upper;

  if (0x03FF < logLimits->extremeUpper)
  {
    return TMR_ERROR_MSG_INVALID_PARAMETER_VALUE;
  }
  tagop->u.gen2.u.custom.u.ids.u.setLogLimit.limit.extremeUpper = logLimits->extremeUpper;

  return TMR_SUCCESS;

}

/**
 * Initialize the Gen2 IDS SL900A SetShelfLifeBlock0 with
 * @param block0 pointer to the TMR_TagOp_GEN2_IDS_SL900A_ShelfLifeBlock0,
 * @param tmax SetShelfLife Tmax
 * @param tmin SetShelfLife Tmin
 * @param tstd SetshelfLIfe Tstd;
 * @param ea   SetshelfLife Ea;
 */ 
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_ShelfLifeBlock0(TMR_TagOp_GEN2_IDS_SL900A_ShelfLifeBlock0 *block0,
                                               uint8_t tmax, uint8_t tmin, uint8_t tstd, uint8_t ea)
{
  uint64_t mask;
  block0->raw = 0;

  /* Copying the tmax to  block0 raw value */ 
  {
    block0->Tmax = tmax;
    mask = TMR_makeBitMask(24, 8);
    block0->raw &= ~mask;
    block0->raw |= (uint32_t)((uint32_t)tmax << 24);
  }

  /* Copying the tmin to  block0 raw value */ 
  {
    block0->Tmin = tmin;
    mask = TMR_makeBitMask(16, 8);
    block0->raw &= ~mask;
    block0->raw |= (uint32_t)((uint32_t)tmin << 16);
  }

  /* Copying the tstd to  block0 raw value */ 
  {
    block0->Tstd = tstd;
    mask = TMR_makeBitMask(8, 8);
    block0->raw &= ~mask;
    block0->raw |= (uint32_t)((uint32_t)tstd << 8);
  }

  /* Copying the ea to  block0 raw value */ 
  {
    block0->Ea = ea;
    mask = TMR_makeBitMask(0, 8);
    block0->raw &= ~mask;
    block0->raw |= (uint32_t)((uint32_t)ea << 0);
  }

  return TMR_SUCCESS;
}


/**
 * Initialize the Gen2 IDS SL900A SetShelfLifeBlock1 with
 * @param block1 pointer to the TMR_TagOp_GEN2_IDS_SL900A_ShelfLifeBlock1,
 * @param slinit SetShelfLife SLinit
 * @param tint SetShelfLife Tint
 * @param sensorid SetshelfLIfe sensorID
 * @param negative SetshelfLife enableNegative
 * @param algorithm SetShelfLife algorithmEnable
 */ 
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_ShelfLifeBlock1(TMR_TagOp_GEN2_IDS_SL900A_ShelfLifeBlock1 *block1,
                                               uint16_t slinit, uint16_t tint, uint8_t sensorid,
                                               bool negative, bool algorithm)
{
  uint64_t mask;
  block1->raw = 0;

  /* Copying the SLinit value to block1 raw value */
  {
    block1->SLinit = slinit;
    mask = TMR_makeBitMask(16, 16);
    block1->raw &= ~mask;
    block1->raw |= (uint32_t)((uint32_t)slinit << 16);
  }

  /* Copying the tint value to block1 raw value */
  {
    block1->Tint = tint;
    mask = TMR_makeBitMask(6, 10);
    block1->raw &= ~mask;
    block1->raw |= (uint32_t)((uint32_t)tint << 6);
  }

  /* Copying the sensorID to block1 raw value */
  {
    block1->sensorID = sensorid;
    mask = TMR_makeBitMask(4, 2);
    block1->raw &= ~mask;
    block1->raw |= (uint32_t)((uint32_t)sensorid << 4);
  }

  /* Copying the enableNegative value to block1 raw value */
  {
    block1->enableNegative = negative;
    mask = TMR_makeBitMask(3, 1);
    block1->raw &= ~mask;
    block1->raw |= (uint32_t)(negative << 3);
  }

  /* Copying the algorithemEnable value to block1 raw value */
  {
    block1->algorithmEnable = algorithm;
    mask = TMR_makeBitMask(2, 1);
    block1->raw &= ~mask;
    block1->raw |= (uint32_t)(negative << 2);
  }

  /* Setting the RFU bytes to zero */
  block1->rfu = 0;
  mask = TMR_makeBitMask(0, 2);
  block1->raw &= ~mask;
  block1->raw |= (uint32_t)(0 << 0);

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a Gen2 Ids SL900A Set Shelf Life operation with
 * @param tagop pointer to the tagop struture to setShelfLife
 * @param accessPassword Gen2 accessPassword
 * @param level IDS passwordlevel
 * @param password IDS password
 * @param block0 pointer to the TMR_TagOp_GEN2_IDS_SL900A_ShelfLifeBlock0,
 * @param block1 pointer to the TMR_TagOp_GEN2_IDS_SL900A_ShelfLifeBlock1,
 */
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_SetShelfLife(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, PasswordLevel level,
                                            uint32_t password,TMR_TagOp_GEN2_IDS_SL900A_ShelfLifeBlock0 *block0,
                                            TMR_TagOp_GEN2_IDS_SL900A_ShelfLifeBlock1 *block1)
{
  tagop->type = TMR_TAGOP_GEN2_IDS_SL900A_SETSHELFLIFE;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_IDS_SL900A_SILICON;
  tagop->u.gen2.u.custom.u.ids.u.setShelfLife.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.ids.u.setShelfLife.CommandCode = 0xAB;
  tagop->u.gen2.u.custom.u.ids.u.setShelfLife.Password = password;
  tagop->u.gen2.u.custom.u.ids.u.setShelfLife.sl900A.level = level;
  tagop->u.gen2.u.custom.u.ids.u.setShelfLife.shelfLifeBlock0 = block0;
  tagop->u.gen2.u.custom.u.ids.u.setShelfLife.shelfLifeBlock1 = block1;

  return TMR_SUCCESS;
}
#endif /* TMR_ENABLE_GEN2_CUSTOM_TAGOPS */
/**
 * Initialize TMR_UserConfigOp to set the default category.
 * @param config pointer to the TMR_UserConfigOp structure to initialize
 * @param op user configuration opeartion (save, restore or clear)
 */
TMR_Status
TMR_init_UserConfigOp(TMR_SR_UserConfigOp *config, TMR_SR_UserConfigOperation op)
{
  config->category = TMR_SR_ALL;
  config->op       = op;

  return TMR_SUCCESS;
}
#ifdef TMR_ENABLE_GEN2_CUSTOM_TAGOPS
/**
 *  Initialize TMR_NXP_ConfigWord to set the default value.
 *  If the instance of the above structure is created as 'static',
 *  then no need to call this constructor.
 *  @param configWord Instance of TMR_NXP_ConfigWord
 */
TMR_Status 
TMR_init_GEN2_NXP_G2I_ConfigWord(TMR_NXP_ConfigWord *configWord)
{
  configWord->data = 0x0000;
  return TMR_SUCCESS;
}

/**
 *  Initialize TMR_NXP_UCODE7_ConfigWord to set the default value.
 *  If the instance of the above structure is created as 'static',
 *  then no need to call this constructor.
 *  @param configWord Instance of TMR_NXP_UCODE7_ConfigWord
 */
TMR_Status 
TMR_init_GEN2_NXP_UCODE7_ConfigWord(TMR_NXP_UCODE7_ConfigWord *configWord)
{
  configWord->data = 0x0000;
  return TMR_SUCCESS;
}

/**
 *  Initialize TMR_Monza4_ControlByte to set the default value to 0.
 *  If the instance of the above structure is created as 'static' or
 *  if it is already initialized to 0, then no need to call this constructor.
 *  @param controlByte Instance of TMR_Monza4_ControlByte
 */
TMR_Status
TMR_init_GEN2_Impinj_Monza4_ControlByte(TMR_Monza4_ControlByte *controlByte)
{
  controlByte->data = 0x00;
  return TMR_SUCCESS;
}

/**
 *  Initialize TMR_Monza4_Payload to set the default value to 0.
 *  If the instance of the above structure is created as 'static' or
 *  if it is already initialized to 0, then no need to call this constructor.
 *  @param payload Instance of TMR_Monza4_Payload
 */
TMR_Status
TMR_init_GEN2_Impinj_Monza4_Payload(TMR_Monza4_Payload *payload)
{
  payload->data = 0x0000;
  return TMR_SUCCESS;
}

/**
 * Helper function to update or modifying the sfe parameters
 */
TMR_Status
TMR_update_GEN2_IDS_SL900A_SfeParameters(TMR_TagOp_GEN2_IDS_SL900A_SfeParameters *sfe)
{
  uint64_t mask;
  /* Update the raw value as per the  field set by the user */
  switch (sfe->type)
  {
    case TMR_GEN2_IDS_SL900A_SFE_RANG:
      {
        mask = TMR_makeBitMask(11, 5);
        sfe->raw &= ~mask;
        sfe->raw |= ((uint64_t)(((uint64_t)sfe->Rang) << 11));

        break;
      }
    case TMR_GEN2_IDS_SL900A_SFE_SETI:
      {
        mask = TMR_makeBitMask(6, 5);
        sfe->raw &= ~mask;
        sfe->raw |= ((uint64_t)(((uint64_t)sfe->Seti) << 6));

        break;
      }
    case TMR_GEN2_IDS_SL900A_SFE_EXT1:
      {
        mask = TMR_makeBitMask(4, 2);
        sfe->raw &= ~mask;
        sfe->raw |= ((uint64_t)(((uint64_t)sfe->Ext1) << 4));

        break;
      }
    case TMR_GEN2_IDS_SL900A_SFE_EXT2:
      {
        mask = TMR_makeBitMask(3, 1);
        sfe->raw &= ~mask;
        sfe->raw |= ((uint64_t)(((uint64_t)sfe->Ext2) << 3));

        break;
      }
    case TMR_GEN2_IDS_SL900A_SFE_AUTORANGEDISABLE:
      {
        mask = TMR_makeBitMask(2, 1);
        sfe->raw &= ~mask;
        sfe->raw |= ((uint64_t)(((uint64_t)sfe->AutorangeDisable) << 2));

        break;
      }
    case TMR_GEN2_IDS_SL900A_SFE_VERIFYSENSORID:
      {
        mask = TMR_makeBitMask(0, 2);
        sfe->raw &= ~mask;
        sfe->raw |= ((uint64_t)(((uint64_t)sfe->VerifySensorID) << 0));

        break;
      }
    default: 
      break;
  }
  return TMR_SUCCESS;
}

/**
 * Helper function to update or modifying the calibration Data
 */
TMR_Status
TMR_update_GEN2_IDS_SL900A_CalibrationData(TMR_TagOp_GEN2_IDS_SL900A_CalibrationData *cal)
{
  uint64_t mask;
  /* Update the raw value as per the  field set by the user */
  switch (cal->type)
  {
    case TMR_GEN2_IDS_SL900A_CALIBRATION_COARSE1:
      {
        mask = TMR_makeBitMask(48, 3);
        cal->raw &= ~mask;
        cal->raw |= ((uint64_t)(((uint64_t)cal->Coarse1) << 48));
        cal->raw = cal->raw;

        break;
      }
    case TMR_GEN2_IDS_SL900A_CALIBRATION_COARSE2:
      {
        mask = TMR_makeBitMask(40, 3);
        cal->raw &= ~mask;
        cal->raw |= ((uint64_t)(((uint64_t)cal->Coarse2) << 40));
        cal->raw = cal->raw;

        break;
      }
    case TMR_GEN2_IDS_SL900A_CALIBRATION_GNDSWITCH:
      {
        mask = TMR_makeBitMask(39, 1);
        cal->raw &= ~mask;
        cal->raw |= ((uint64_t)(((uint64_t)cal->GndSwitch) << 39));
        cal->raw = cal->raw;

        break;
      }
    case TMR_GEN2_IDS_SL900A_CALIBRATION_SELP12:
      {
        mask = TMR_makeBitMask(37, 2);
        cal->raw &= ~mask;
        cal->raw |= ((uint64_t)(((uint64_t)cal->Selp12) << 37));
        cal->raw = cal->raw;

        break;
      }
    case TMR_GEN2_IDS_SL900A_CALIBRATION_DF:
      {
        mask = TMR_makeBitMask(24, 8);
        cal->raw &= ~mask;
        cal->raw |= ((uint64_t)(((uint64_t)cal->Df) << 24));
        cal->raw = cal->raw;

        break;
      }
    case TMR_GEN2_IDS_SL900A_CALIBRATION_SWEXTEN:
      {
        mask = TMR_makeBitMask(23, 1);
        cal->raw &= ~mask;
        cal->raw |= ((uint64_t)(((uint64_t)cal->SwExtEn) << 23));
        cal->raw = cal->raw;

        break;
      }
    case TMR_GEN2_IDS_SL900A_CALIBRATION_SELP22:
      {
        mask = TMR_makeBitMask(21, 2);
        cal->raw &= ~mask;
        cal->raw |= ((uint64_t)(((uint64_t)cal->Selp22) << 21));
        cal->raw = cal->raw;

        break;
      }
    case TMR_GEN2_IDS_SL900A_CALIBRATION_IRLEV:
      {
        mask = TMR_makeBitMask(19, 2);
        cal->raw &= ~mask;
        cal->raw |= ((uint64_t)(((uint64_t)cal->Irlev) << 19));
        cal->raw = cal->raw;

        break;
      }
    case TMR_GEN2_IDS_SL900A_CALIBRATION_EXCRES:
      {
        mask = TMR_makeBitMask(2, 1);
        cal->raw &= ~mask;
        cal->raw |= ((uint64_t)(((uint64_t)cal->ExcRes) << 2));
        cal->raw = cal->raw;

        break;
      }
    default: 
      break;
  }
  return TMR_SUCCESS;
}

/**
 * Helper function to set the calibration Data
 */
TMR_Status
TMR_init_GEN2_IDS_SL900A_CalibrationData(uint8_t byte[7], TMR_TagOp_GEN2_IDS_SL900A_CalibrationData *cal)
{
  if (NULL != byte)
  {
    /**
     * ToU64 requires 8 bytes of input, but CalibrationData is only 7 bytes long
     * Create a temporary array to provide the necessary padding.
     */
    uint8_t tmp[] = {0,0,0,0,0,0,0,0,};
    memcpy(tmp+1, byte, 7);
    cal->raw = (uint64_t)(0
        | ((uint64_t)(tmp[ 0]) << 56)
        | ((uint64_t)(tmp[ 1]) << 48)
        | ((uint64_t)(tmp[ 2]) << 40)
        | ((uint64_t)(tmp[3]) << 32)
        | ((uint64_t)(tmp[4]) << 24)
        | ((uint64_t)(tmp[5]) << 16)
        | ((uint64_t)(tmp[ 6]) << 8)
        | ((uint64_t)(tmp[7]) << 0));

    cal->raw &= (uint64_t)0x00FFFFFFFFFFFFFFLL;
  }

  return TMR_SUCCESS;
}

/**
 * Helper function to set the sfe parameters
 */
TMR_Status
TMR_init_GEN2_IDS_SL900A_SfeParameters(uint8_t byte[2], TMR_TagOp_GEN2_IDS_SL900A_SfeParameters *sfe)
{
  /* Create SFEParameters object from raw 2-byte reply */
  int hi, lo;
  hi = (uint16_t)(byte[0]) << 8;
  lo = (uint16_t)(byte[1]);
  sfe->raw = (hi | lo);

  return TMR_SUCCESS;
}

/**
 * Initialize TMR_GEN2_SL900A_SensorReading to Get Sensor Value response.
 * @param reply instance of TMR_uint8List
 * @param sensor instance of TMR_TagOp_GEN2_IDS_SL900A_SensorReading,
 * response for sl900A getSensorvalue command.
 */
TMR_Status
TMR_init_GEN2_IDS_SL900A_SensorReading(TMR_uint8List *reply, TMR_TagOp_GEN2_IDS_SL900A_SensorReading *sensor)
{  

  if (2 != reply->len)
  {
    /* Sensor Reading value must be exactly 2 bytes long */
    return TMR_ERROR_MSG_INVALID_PARAMETER_VALUE;
  }
  sensor->reply = reply->list[0] << 8 | reply->list[1];
  sensor->Raw = sensor->reply;
  sensor->ADError = (((sensor->reply >> 15) & 0x1) != 0);
  sensor->RangeLimit = (uint8_t) ((sensor->reply >> 10) & 0x1F);
  sensor->Value = ((sensor->reply >> 0 ) & 0x3FF);

  return TMR_SUCCESS;
}

/**
 * Initialize TMR_GEN2_SL900A_CalSfe to Get getcalibration response.
 * @param reply instance of TMR_uint8List
 * @param calSfe instance of TMR_TagOp_GEN2_IDS_SL900A_CalSfe,
 * response for CalibrationData and SfeParameters.
 */
TMR_Status
TMR_init_GEN2_IDS_SL900A_CalSfe(TMR_uint8List *reply, TMR_TagOp_GEN2_IDS_SL900A_CalSfe *calSfe)
{
  uint64_t mask;
  TMR_init_GEN2_IDS_SL900A_CalibrationData(reply->list, &calSfe->Cal);
  TMR_init_GEN2_IDS_SL900A_SfeParameters(reply->list + 7, &calSfe->Sfe);

  mask = TMR_makeBitMask(51, 5);
  calSfe->Cal.Ad1 = (uint8_t)((calSfe->Cal.raw & mask) >> 51);

  mask = TMR_makeBitMask(48, 3);
  calSfe->Cal.Coarse1 = (uint8_t)((calSfe->Cal.raw & mask) >> 48);

  mask = TMR_makeBitMask(43, 5);
  calSfe->Cal.Ad2 = (uint8_t)((calSfe->Cal.raw & mask) >> 43);

  mask = TMR_makeBitMask(40, 3);
  calSfe->Cal.Coarse2 = (uint8_t)((calSfe->Cal.raw & mask) >> 40);

  mask = TMR_makeBitMask(39, 1);
  calSfe->Cal.GndSwitch = (uint8_t)((calSfe->Cal.raw & mask) >> 39);

  mask = TMR_makeBitMask(37, 2);
  calSfe->Cal.Selp12 = (uint8_t)((calSfe->Cal.raw & mask) >> 37);

  mask = TMR_makeBitMask(32, 5);
  calSfe->Cal.Adf = (uint8_t)((calSfe->Cal.raw & mask) >> 32);

  mask = TMR_makeBitMask(24, 8);
  calSfe->Cal.Df = (uint8_t)((calSfe->Cal.raw & mask) >> 24);

  mask = TMR_makeBitMask(23, 1);
  calSfe->Cal.SwExtEn = (uint8_t)((calSfe->Cal.raw & mask) >> 23);

  mask = TMR_makeBitMask(21, 2);
  calSfe->Cal.Selp22 = (uint8_t)((calSfe->Cal.raw & mask) >> 21);

  mask = TMR_makeBitMask(19, 2);
  calSfe->Cal.Irlev = (uint8_t)((calSfe->Cal.raw & mask) >> 19);

  mask = TMR_makeBitMask(14, 5);
  calSfe->Cal.RingCal = (uint8_t)((calSfe->Cal.raw & mask) >> 14);

  mask = TMR_makeBitMask(7, 7);
  calSfe->Cal.OffInt = (uint8_t)((calSfe->Cal.raw & mask) >> 7);

  mask = TMR_makeBitMask(3, 4);
  calSfe->Cal.Reftc = (uint8_t)((calSfe->Cal.raw & mask) >> 3);

  mask = TMR_makeBitMask(2, 1);
  calSfe->Cal.ExcRes = (uint8_t)((calSfe->Cal.raw & mask) >> 2);

  mask = TMR_makeBitMask(0, 2);
  calSfe->Cal.RFU = (uint8_t)((calSfe->Cal.raw & mask) >> 0);

  mask = TMR_makeBitMask(11, 5);
  calSfe->Sfe.Rang = (uint8_t)((calSfe->Sfe.raw & mask) >> 11);

  mask = TMR_makeBitMask(6, 5);
  calSfe->Sfe.Seti = (uint8_t)((calSfe->Sfe.raw & mask) >> 6);

  mask = TMR_makeBitMask(4, 2);
  calSfe->Sfe.Ext1 = (uint8_t)((calSfe->Sfe.raw & mask) >> 4);

  mask = TMR_makeBitMask(3, 1);
  calSfe->Sfe.Ext2 = (uint8_t)((calSfe->Sfe.raw & mask) >> 3);

  mask = TMR_makeBitMask(2, 1);
  calSfe->Sfe.AutorangeDisable = (uint8_t)((calSfe->Sfe.raw & mask) >> 2);

  mask = TMR_makeBitMask(0, 2);
  calSfe->Sfe.VerifySensorID = (uint8_t)((calSfe->Sfe.raw & mask) >> 0);

  return TMR_SUCCESS;
}

/**
 * Initialize TMR_GEN2_SL900A_BatteryLevelReading to get the getBatteryLevel response.
 * @param reply instance of TMR_uint8List
 * @param battery instance of TMR_TagOp_GEN2_IDS_SL900A_BatteryLevelReading,
 * the reply structure of getBatteryLevel
 */ 

TMR_Status
TMR_init_GEN2_IDS_SL900A_BatteryLevelReading(TMR_uint8List *reply, TMR_TagOp_GEN2_IDS_SL900A_BatteryLevelReading *battery)
{  

  if (2 != reply->len)
  {
    /* Sensor Reading value must be exactly 2 bytes long */
    return TMR_ERROR_MSG_INVALID_PARAMETER_VALUE;
  }
  battery->reply = (uint16_t)(reply->list[0] << 8 | reply->list[1]);
  battery->ADError = (bool)(((battery->reply >> 15) & 0x1) != 0);
  battery->BatteryType = (uint8_t) ((battery->reply >> 14) & 0x1);
  battery->Value = (uint16_t)((battery->reply >> 0 ) & 0x3FF);

  return TMR_SUCCESS;
}
/**
 * Initialize TMR_GEN2_SL900A_FifoStatus to Get status of fifo reply
 * @param statusReply instance of TMR_uint8List
 * @param status instance of TMR_TagOp_GEN2_IDS_SL900A_FifoStatus,
 * the reply structure of sl900A fifistatus command. 
 */
TMR_Status
TMR_init_GEN2_IDS_SL900A_FifoStatus(TMR_uint8List *statusReply, TMR_TagOp_GEN2_IDS_SL900A_FifoStatus *status)
{

  uint8_t reply;
  if (1 != statusReply->len)
  {
    /* Fifo Status value must be exactly 1 byte long */
    return TMR_ERROR_MSG_INVALID_PARAMETER_VALUE;
  }
  status->raw = statusReply->list[0];
  reply = statusReply->list[0];
  status->fifoBusy = (0 != ((reply >> 7) & 1));
  status->dataReady = (0 != ((reply >> 6) & 1));
  status->noData = (0 != ((reply >> 5) & 1));
  status->numValidBytes = (uint8_t)(reply & 0xF);

  return TMR_SUCCESS;
}

/**
 * Initialize TMR_GEN2_SL900A_LogState to Get get log state Value response.
 * @param reply instance of TMR_uint8List
 * @param log instance of TMR_TagOp_GEN2_IDS_SL900A_LogState,
 * sl900A logging parameters.
 */
TMR_Status
TMR_init_GEN2_IDS_SL900A_LogState(TMR_uint8List *reply, TMR_TagOp_GEN2_IDS_SL900A_LogState *log)
{  

  uint32_t offset = 0;
  uint32_t raw;  
  if ((9 != reply->len) && (20 != reply->len))
  {
    /* GetLogState replies must be 9 or 21 bytes in length */
    return TMR_ERROR_MSG_INVALID_PARAMETER_VALUE;
  }

  if (9 == reply->len)
  {
    log->limitCount.extremeLower = reply->list[offset + 0];    
    log->limitCount.lower = reply->list[offset + 1];
    log->limitCount.upper = reply->list[offset + 2];
    log->limitCount.extremeUpper = reply->list[offset + 3];

    offset += 4;
    raw = TMR_byteArrayToInt(reply->list, offset);
    log->statStatus.MeasurementAddressPointer = (uint16_t)((raw >> 22) & 0x1FF);
    log->statStatus.NumMemReplacements = (uint8_t) ((raw >> 16) & 0x3F);
    log->statStatus.NumMeasurements = (uint16_t) ((raw >> 1) & 0x7FFF);
    log->statStatus.Active = (bool)(0 != (raw & 0x1));
    offset += 4;
  }
  if (20 == reply->len)
  {
    /* @todo Fully support shelf life arguments.  For now, just skip over them */
    offset += 8;
    offset += 4;
  }
  raw = TMR_byteArrayToInt(reply->list, offset);
  log->statFlag.Active = (bool)(0 != ((raw >> 7) & 1));
  log->statFlag.Full = (bool)(0 != ((raw >> 6) & 1));
  log->statFlag.Overwritten = (bool)(0 != ((raw >> 5) & 1));
  log->statFlag.ADError = (bool) (0 != ((raw >> 4) & 1));
  log->statFlag.LowBattery = (bool) (0 != ((raw >> 3) & 1));
  log->statFlag.ShelfLifeLow = (bool) (0 != ((raw >> 2) & 1));
  log->statFlag.ShelfLifeHigh = (bool)(0 != ((raw >> 1) & 1));
  log->statFlag.ShelfLifeExpired = (bool)(0 != ((raw >> 0) & 1));
  
  return TMR_SUCCESS;
}

/**
 * Initialize TMR_TagOp_GEN2_IDS_SL900A_MeasurementSetupData to Get measurment setup data 
 * Value response.
 * @param reply instance of TMR_uint8List
 * @param measurement instance of TMR_TagOp_GEN2_IDS_SL900A_MeasurementSetupData,
 * response structure for sl900A getMeasurementSetupData command.
 */
TMR_Status
TMR_init_GEN2_IDS_SL900A_MeasurementSetupData(TMR_uint8List *reply, TMR_TagOp_GEN2_IDS_SL900A_MeasurementSetupData *measurement)
{
  if(16 != reply->len)
  {
    /* MeasurementSetupData value must be exactly 16 byte long */
    return TMR_ERROR_MSG_INVALID_PARAMETER_VALUE;
  }

  memcpy(measurement->Raw, reply->list, reply->len);

  //get the start time
  {
    uint32_t temp;
    temp = TMR_byteArrayToInt(reply->list, 0);
    measurement->startTime.tm_year = (int)(2010+((temp >> 26) & 0x3F));
    measurement->startTime.tm_mon = (int)((temp >> 22) & 0xF);
    measurement->startTime.tm_mday = (int)((temp >> 17) & 0x1F);
    measurement->startTime.tm_hour = (int)((temp >> 12) & 0x1F);
    measurement->startTime.tm_min = (int)((temp >> 6) & 0x3F);
    measurement->startTime.tm_sec = (int)((temp >> 0) & 0x3F);
  }
  //get the log limits
  {
    int offset = 4;
    uint64_t value = 0;
    /* LogLimits value is of 40 bits
     * extract that from the response
     */
    value = TMR_byteArrayToLong(reply->list, offset);
    /* 
     * Indivisual field in LogLimits is of 10 bits
     * masking each of them to a 16 bit value for convinence
     */
    measurement->loglimit.extremeLower = (uint16_t) ((value >> 54) & 0x03FF);
    measurement->loglimit.lower = (uint16_t) ((value >> 44) & 0x03FF);
    measurement->loglimit.upper = (uint16_t) ((value >> 34) & 0x03FF);
    measurement->loglimit.extremeUpper = (uint16_t) ((value >> 24) & 0x03FF);
  }
  //get log mode
  {
    uint8_t temp = (uint8_t)reply->list[9];
    measurement->logModeData.Raw = temp;
    measurement->logModeData.Form = (LoggingForm)((temp >> 5) & 7);
    measurement->logModeData.Storage = (StorageRule)(((temp >> 4) & 1));
    measurement->logModeData.Ext1Enable = (0 != ((temp >> 3) & 1));
    measurement->logModeData.Ext2Enable = (0 != ((temp >> 2) & 1));
    measurement->logModeData.TempEnable = (0 != ((temp >> 1) & 1));
    measurement->logModeData.BattEnable = (0 != ((temp>> 0) & 1));
  }
  //log interval
  {
    uint16_t temp;
    temp = TMR_byteArrayToShort(reply->list, 10);
    temp = ((temp >> 1) & 0x7FFF);
    measurement->logInterval = temp;
  }
  //delay time
  {
    uint16_t temp;
    temp = TMR_byteArrayToShort(reply->list, 12);
    measurement->delyTime.raw = temp;
    measurement->delyTime.Mode = (0 == ((temp >> 1) & 0x1)) ? TMR_GEN2_IDS_SL900A_DELAYMODE_TIMER
                                                           : TMR_GEN2_IDS_SL900A_DELAYMODE_EXTSWITCH;
    measurement->delyTime.Time = (uint16_t)((temp >> 4) & 0xFFF);
    measurement->delyTime.IrqTimerEnable = (0 != (temp & 0x1));
  }
  //application data
  {
    uint16_t temp;
    temp = TMR_byteArrayToShort(reply->list, 14);
    measurement->addData.raw = temp;
    measurement->addData.NumberOfWords = (uint16_t)((temp >> 7) & 0x1FF);
    measurement->addData.BrokenWordPointer = (uint8_t)(temp & 0x7);
  }

  return TMR_SUCCESS;
}

#endif /* TMR_ENABLE_GEN2_CUSTOM_TAGOPS  */
void
TMR_paramProbe(struct TMR_Reader *reader, TMR_Param key)
{

  /* buf is at least as large as the largest parameter, with all values 0
   * (NULL pointers and 0 lengths).
   */
  uint32_t buf[] = {0, 0, 0, 0, 0, 0, 0, 0};
  TMR_Status ret;

  ret = TMR_paramGet(reader, key, &buf);
  if (TMR_SUCCESS == ret)
  {
    if (TMR_READER_TYPE_SERIAL == reader->readerType)
    {
      BITSET(reader->u.serialReader.paramPresent, key);
      BITSET(reader->u.serialReader.paramConfirmed, key);
    }
#ifdef TMR_ENABLE_LLRP_READER
    if (TMR_READER_TYPE_LLRP == reader->readerType)
    {
      BITSET(reader->u.llrpReader.paramPresent, key);
      BITSET(reader->u.llrpReader.paramConfirmed, key);
    }
#endif
  }
}


/**
 * Get a list of the parameters available
 * @reader pointer of reader object
 * @key pointer of TMR_Param
 * @len pointer to uint32_t
 */

TMR_Status
TMR_paramList(struct TMR_Reader *reader, TMR_Param *keys, uint32_t *len)
{
  int i, count, max;

  max = *len;
  count = 0;
  for (i = TMR_PARAM_MIN; i <= TMR_PARAM_MAX ; i++)
  {
    if (TMR_READER_TYPE_SERIAL == reader->readerType)
    {
      if (0 == BITGET(reader->u.serialReader.paramConfirmed, i))
      {
        /* Fix me */
        TMR_paramProbe(reader, (TMR_Param)i);
      }

      if (BITGET(reader->u.serialReader.paramPresent, i))
      {
        if (count < max)
          keys[count] = (TMR_Param)i;
        count++;
      }
    }
#ifdef TMR_ENABLE_LLRP_READER
    if (TMR_READER_TYPE_LLRP == reader->readerType)
    {
      if (0 == BITGET(reader->u.llrpReader.paramConfirmed, i))
      {
        TMR_paramProbe(reader, i);
      }

      if (BITGET(reader->u.llrpReader.paramPresent, i))
      {
        if (count < max)
          keys[count] = i;
        count++;
      }
    }
#endif
  }

  *len = count;

  return TMR_SUCCESS;
}

TMR_Status
TMR_receiveAutonomousReading(struct TMR_Reader *reader, TMR_TagReadData *trd, TMR_Reader_StatsValues *stats)
{
  TMR_Status ret;

  ret = TMR_SUCCESS;

  if (TMR_READER_TYPE_SERIAL != reader->readerType)
  {
    /* Currently supporting only serial reader */
    return TMR_ERROR_UNSUPPORTED;
  }

#ifdef TMR_ENABLE_BACKGROUND_READS
  {
    /**
     *      *     * create the thread
     *           *         */
    ret = pthread_create(&reader->autonomousBackgroundReader, NULL,
        do_background_receiveAutonomousReading, reader);
    if (0 != ret)
    {
      return TMR_ERROR_NO_THREADS;
    }
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_detach(reader->autonomousBackgroundReader);
  }
#else
  {
    /* This will add support for non thread platform */
    ret =  TMR_SR_receiveAutonomousReading(reader, trd, stats);
  }
#endif

  return ret;
}
#ifdef TMR_ENABLE_UHF
TMR_Status isAntDetectEnabled(struct TMR_Reader *reader, uint8_t *antennaList)
{
  TMR_Status ret = TMR_SUCCESS;
  bool antDetectionFlag = false;

  /* paramGet of antenna detection flag */
  ret = TMR_paramGet(reader, TMR_PARAM_ANTENNA_CHECKPORT, &antDetectionFlag);
  if(ret != TMR_SUCCESS)
  {
    if((ret == TMR_ERROR_UNIMPLEMENTED_FEATURE) && (NULL == antennaList))
    { return TMR_ERROR_UNSUPPORTED; }
    else
    { return TMR_SUCCESS; }
  }
  else
  {
    TMR_String model;
    TMR_String swVersion;
    char softwareVersion[100];
    char str[64];

    swVersion.value = softwareVersion;
    swVersion.max = sizeof(softwareVersion)/sizeof(softwareVersion[0]);
    model.value = str;
    model.max = 64;

    /* paramGet of software version */
    TMR_paramGet(reader, TMR_PARAM_VERSION_SOFTWARE, &swVersion);
    TMR_paramGet(reader, TMR_PARAM_VERSION_MODEL, &model);

    if (((0 == strcmp("M6e Micro", model.value)) || (0 == strcmp("M6e Nano", model.value)) ||
        ((0 == strcmp("Sargas", model.value)) && ((0 == strncmp(swVersion.value,"5.0",3)) || (swVersion.value[0] < 5))) ||
        (((0 == strcmp("Sargas", model.value)) || (0 == strcmp("Izar", model.value)) || (0 == strcmp("Astra200", model.value))) &&
         (((swVersion.value[0] == 5) && (swVersion.value[2] >= 1)) || (swVersion.value[0] > 6)) && (false == antDetectionFlag))) &&
        (NULL == antennaList))
    {
      return TMR_ERROR_UNSUPPORTED;
    }
  }
  return ret;
}

TMR_Status
TMR_TagOp_init_GEN2_fdn_ReadREG(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, uint16_t RegAddress)
{
  tagop->type = TMR_TagOP_GEN2_FUDAN_READREG;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_FUDAN_SILICON;
  tagop->u.gen2.u.custom.u.fdn.u.ReadREG.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.fdn.u.ReadREG.CommandCode = 0x0009;
  tagop->u.gen2.u.custom.u.fdn.u.ReadREG.RegAddress = RegAddress;
  
  return TMR_SUCCESS;
}

TMR_Status
TMR_TagOp_init_GEN2_fdn_WriteREG(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, uint16_t RegAddress, uint16_t RegWriteData)
{
  tagop->type = TMR_TagOP_GEN2_FUDAN_WRITEREG;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_FUDAN_SILICON;
  tagop->u.gen2.u.custom.u.fdn.u.WriteREG.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.fdn.u.WriteREG.CommandCode = 0x0008;
  tagop->u.gen2.u.custom.u.fdn.u.WriteREG.RegAddress = RegAddress;
  tagop->u.gen2.u.custom.u.fdn.u.WriteREG.RegWriteData = RegWriteData;
  
  return TMR_SUCCESS;
}

TMR_Status
TMR_TagOp_init_GEN2_fdn_LoadREG(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, uint8_t CmdCfg)
{
  tagop->type = TMR_TagOP_GEN2_FUDAN_LOADREG;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_FUDAN_SILICON;
  tagop->u.gen2.u.custom.u.fdn.u.LoadREG.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.fdn.u.LoadREG.CommandCode = 0x0011;
  tagop->u.gen2.u.custom.u.fdn.u.LoadREG.CmdCfg = CmdCfg;
  
  return TMR_SUCCESS;
}

TMR_Status
TMR_TagOp_init_GEN2_fdn_StartStopLOG(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, uint8_t CmdCfg, uint32_t FlagResetPassword)
{
  tagop->type = TMR_TagOP_GEN2_FUDAN_STARTSTOPLOG;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_FUDAN_SILICON;
  tagop->u.gen2.u.custom.u.fdn.u.StartStopLOG.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.fdn.u.StartStopLOG.CommandCode = 0x0006;
  tagop->u.gen2.u.custom.u.fdn.u.StartStopLOG.CmdCfg = CmdCfg;
  tagop->u.gen2.u.custom.u.fdn.u.StartStopLOG.FlagResetPassword = FlagResetPassword;
  
  return TMR_SUCCESS;
}

TMR_Status
TMR_TagOp_init_GEN2_fdn_Auth(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, uint8_t CmdCfg, uint32_t AuthPassword)
{
  tagop->type = TMR_TagOP_GEN2_FUDAN_AUTH;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_FUDAN_SILICON;
  tagop->u.gen2.u.custom.u.fdn.u.Auth.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.fdn.u.Auth.CommandCode = 0x0002;
  tagop->u.gen2.u.custom.u.fdn.u.Auth.CmdCfg = CmdCfg;
  tagop->u.gen2.u.custom.u.fdn.u.Auth.AuthPassword = AuthPassword;
  
  return TMR_SUCCESS;
}

TMR_Status
TMR_TagOp_init_GEN2_fdn_ReadMEM(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, uint16_t StartAddress, uint16_t len)
{
  tagop->type = TMR_TagOP_GEN2_FUDAN_READMEM;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_FUDAN_SILICON;
  tagop->u.gen2.u.custom.u.fdn.u.ReadMEM.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.fdn.u.ReadMEM.CommandCode = 0x0000;
  tagop->u.gen2.u.custom.u.fdn.u.ReadMEM.StartAddress = StartAddress;
  tagop->u.gen2.u.custom.u.fdn.u.ReadMEM.len = len;
  
  return TMR_SUCCESS;
}

TMR_Status
TMR_TagOp_init_GEN2_fdn_WriteMEM(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, uint16_t StartAddress, TMR_uint8List *Writedata)
{
  tagop->type = TMR_TagOP_GEN2_FUDAN_WRITEMEM;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_FUDAN_SILICON;
  tagop->u.gen2.u.custom.u.fdn.u.WriteMEM.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.fdn.u.WriteMEM.CommandCode = 0x0001;
  tagop->u.gen2.u.custom.u.fdn.u.WriteMEM.StartAddress = StartAddress;
  tagop->u.gen2.u.custom.u.fdn.u.WriteMEM.data = Writedata;
  
  return TMR_SUCCESS;
}

TMR_Status
TMR_TagOp_init_GEN2_fdn_StateCheck(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, TMR_uint8List *data)
{
  tagop->type = TMR_TagOP_GEN2_FUDAN_STATECHECK;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_FUDAN_SILICON;
  tagop->u.gen2.u.custom.u.fdn.u.StateCheck.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.fdn.u.StateCheck.CommandCode = 0x000C;
  tagop->u.gen2.u.custom.u.fdn.u.StateCheck.data = data;

  return TMR_SUCCESS;
}

TMR_Status
TMR_TagOp_init_GEN2_ILN_TagSelectCommand(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword)
{
  tagop->type = TMR_TagOP_GEN2_ILIAN_TAG_SELECT_COMMAND;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_ILIAN_LED_SILICON;
  tagop->u.gen2.u.custom.u.ilian.u.IlianTagSelect.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.ilian.u.IlianTagSelect.CommandCode = 0x0001;
  
  return TMR_SUCCESS;
}
TMR_Status
TMR_TagOp_init_GEN2_fdn_Measure(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, uint8_t CmdCfg, uint8_t StoreBlockAddress)
{
  tagop->type = TMR_TagOP_GEN2_FUDAN_MEASURE;
  tagop->u.gen2.u.custom.chipType = TMR_SR_GEN2_FUDAN_SILICON;
  tagop->u.gen2.u.custom.u.fdn.u.Measure.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.fdn.u.Measure.CommandCode = 0x0003;
  tagop->u.gen2.u.custom.u.fdn.u.Measure.CmdCfg = CmdCfg;
  tagop->u.gen2.u.custom.u.fdn.u.Measure.StoreBlockAddress = StoreBlockAddress;

  return TMR_SUCCESS;
}
TMR_Status
TMR_TagOp_init_GEN2_EM4325_GetSensorData(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, bool sendUID, bool sendNewSample)
{
  uint8_t bitsToSet = 0;

  tagop->type                                                              = TMR_TagOP_GEN2_EM4325_GET_SENSOR_DATA;
  tagop->u.gen2.u.custom.chipType                                          = TMR_SR_GEN2_EMMICRO_EM4325_SILICON;
  tagop->u.gen2.u.custom.u.emmicro.u.em4325.u.getSensorData.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.emmicro.u.em4325.u.getSensorData.CommandCode    = 0x0001;

  /* Set appropriate bits based on sendUid and sendNewSample flags.
   * a) sendUid flag.
   * If true  : sends UID in response.
   * If false : response doesn't include UID.
   *
   * b) sendNewSample flag.
   * If true  : gets the new sample.
   * If false : gets the last sample.
   */
  bitsToSet  = (uint8_t)(sendUID ? 0x80 : 0);
  bitsToSet |= (uint8_t)(sendNewSample ? 0x40 : 0);
  tagop->u.gen2.u.custom.u.emmicro.u.em4325.u.getSensorData.bitsToSet      = bitsToSet;

  return TMR_SUCCESS;
}
TMR_Status
TMR_TagOp_init_GEN2_EM4325_ResetAlarms(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword)
{
  tagop->type                                                            = TMR_TagOP_GEN2_EM4325_RESET_ALARMS;
  tagop->u.gen2.u.custom.chipType                                        = TMR_SR_GEN2_EMMICRO_EM4325_SILICON;
  tagop->u.gen2.u.custom.u.emmicro.u.em4325.u.resetAlarms.AccessPassword = accessPassword;
  tagop->u.gen2.u.custom.u.emmicro.u.em4325.u.resetAlarms.CommandCode    = 0x0004;
  tagop->u.gen2.u.custom.u.emmicro.u.em4325.u.resetAlarms.fillValue      = 0x50;

  return TMR_SUCCESS;
}
#endif /* TMR_ENABLE_UHF */

#ifdef TMR_ENABLE_HF_LF
/**
 * Initialize an access password in TMR_TagOp with the provided parameters.
 *
 * @param tagop Pointer to the tagop structure to initialize 
 * @param accessPassword access password.
 */
TMR_Status
TMR_set_accessPassword(TMR_TagOp *tagop, TMR_uint8List *accessPassword)
{
  tagop->u.extTagOp.accessPassword = accessPassword;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a write memory operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param memType types of different memory operations
 * @param address address
 * @param data data to be written
 */
TMR_Status
TMR_TagOp_init_WriteMemory(TMR_TagOp *tagop, TMR_Memory_Type memType, uint32_t address,
                                     TMR_uint8List *data)
{
  tagop->type                          = TMR_TAGOP_WRITE_MEMORY;
  tagop->u.extTagOp.accessPassword     = NULL;
  tagop->u.extTagOp.writeMem.memType   = memType;
  tagop->u.extTagOp.writeMem.address   = address;
  tagop->u.extTagOp.writeMem.data.len  = data->len;
  tagop->u.extTagOp.writeMem.data.list = data->list;
  tagop->u.extTagOp.writeMem.data.max  = data->max;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a read memory operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param memType types of different memory operations
 * @param address address
 * @param len number of memory units to read
 */
TMR_Status
TMR_TagOp_init_ReadMemory(TMR_TagOp *tagop, TMR_Memory_Type memType, uint32_t address, uint8_t len)
{
  tagop->type                         = TMR_TAGOP_READ_MEMORY;
  tagop->u.extTagOp.accessPassword    = NULL;
  tagop->u.extTagOp.readMem.memType   = memType;
  tagop->u.extTagOp.readMem.address   = address;
  tagop->u.extTagOp.readMem.len       = len;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_TagOp as a passthrough operation with the provided parameters.
 * @param tagop Pointer to the tagop structure to initialize.
 * @param timeout time to perform the operation
 * @param responseLen expected length of the response
 * @param configFlags configuration flags
 * @param buffer command buffer
 */
TMR_Status
TMR_TagOp_init_PassThrough(TMR_TagOp *tagop, uint32_t timeout,
                             uint32_t configFlags, TMR_uint8List *buffer)
{
  tagop->type = TMR_TAGOP_PASS_THROUGH;
  tagop->u.passThrough.timeout = timeout;
  tagop->u.passThrough.configFlags = configFlags;
  tagop->u.passThrough.buffer.len = buffer->len;
  tagop->u.passThrough.buffer.max = buffer->max;
  tagop->u.passThrough.buffer.list = buffer->list;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_Filter structure as an UID select filter with the
 * provided parameters.
 * 
 * @param filter Pointer to the filter structure to initialize
 * @param UIDMask the mask of the bytes to compare
 * @param byteLen length of UIDMask
 */
TMR_Status TMR_TF_init_uid_select(TMR_TagFilter *filter, uint8_t UIDMaskBitLen, uint8_t *UIDMask)
{
  filter->type = TMR_FILTER_TYPE_UID_SELECT;
  memcpy(filter->u.uidSelect.UIDMask, UIDMask, tm_u8s_per_bits(UIDMaskBitLen));
  filter->u.uidSelect.UIDMaskBitLen = UIDMaskBitLen;

  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_Filter structure as a TAGTYPE select filter with the
 * provided parameters.
 * 
 * @param filter Pointer to the filter structure to initialize
 * @param tagType the protocol specific tagtype to be matched
 */
TMR_Status TMR_TF_init_tagtype_select(TMR_TagFilter *filter, uint64_t tagType)
{
  filter->type = TMR_FILTER_TYPE_TAGTYPE_SELECT;
  filter->u.tagtypeSelect.tagType = tagType;

  return TMR_SUCCESS;
}
#endif /* TMR_ENABLE_HF_LF */

/** Convert value to Extensible bit vectors(EBV) format 
  * Set value in 1/2/3/4/5 bytes, dynamically based on value 
  */
TMR_Status
TMR_SR_convertToEBV(uint8_t *msg, uint8_t *i, uint64_t value)
{
  uint64_t temp = 0;

  /* 1 byte is sufficient */
  if ((0x80) > value)
  {
    SETU8(msg, *i, (uint8_t)value);
  }
  /* 2 bytes are sufficient */
  else if((0x4000) > value)
  {
    temp = (value & 0x7F);
    value &= 0xFF80;
    value = ((value << 1) | temp);

    SETU16(msg, *i, (uint16_t)(0x8000 | value));
  }
  /* 3 bytes are sufficient */
  else if((0x200000) > value)
  {
    temp = (value & 0x7F);
    value = (value << 1);
    temp |= (value & 0x7f00);
    value = (((value << 1) & 0xff0000) | temp);

    SETU24(msg, *i, (uint32_t)(0x808000 | value));
  }
  /* 4 bytes are sufficient */
  else if((0x10000000) > value)
  {
    temp = (value & 0x7f);
    value = (value << 1);
    temp |= (value & 0x7f00);
    value = (value << 1);
    temp |= (value & 0x7f0000);
    value = (((value << 1) & 0xff000000) | temp);
    
    SETU32(msg, *i, (uint32_t)(0x80808000 | value));
  }
  /* 5 bytes are sufficient */
  else if((0x800000000ULL) > value)
  {
    temp = (value & 0x7f);
    value = (value << 1);
    temp |= (value & 0x7f00);
    value = (value << 1);
    temp |= (value & 0x7f0000);
    value = (value << 1);
    temp |= (value & 0x7f000000);
    value = (((value << 1) & 0xff00000000ULL) | temp);
    
    SETU40(msg, *i, (uint64_t)(0x8080808000ULL | value));
  }
  else
  {
    return TMR_ERROR_INVALID;
  }
  return TMR_SUCCESS;
}

/** Convert from 1/2/3/4/5 bytes of EBV formated value */
uint64_t
TMR_SR_convertFromEBV(uint8_t *msg, uint8_t length)
{
  uint64_t value = 0, temp = 0;

  switch (length)
  {
    case 0x01:
    {
      value = msg[0];
      break;
    }
    case 0x02:
    {
      value = GETU16AT(msg, 0);

      temp = (value & 0x7f);
      value &= 0x7fff;
      value = (((value >> 1) & 0xff80) | temp);
      break;
    }
    case 0x03:
    {
      value = GETU24AT(msg, 0);

      value &= 0x7f7f7f;
      temp = (value & 0x7f);
      value = ((value >> 1) | temp);
      temp |= (value & 0x3f80);
      value = (((value >> 1) & 0xffc000) | temp);
      break;
    }
    case 0x04:
    {
      value = GETU32AT(msg, 0);
      value &= 0x7f7f7f7f;
      temp = (value & 0x7f);
      value = ((value >> 1) | temp);
      temp |= (value & 0x3f80);
      value = (((value >> 1) & 0xffffc000) | temp);
      temp |= (value & 0x1fc000);
      value = (((value >> 1) & 0xffe00000) | temp);
      break;
    }
    case 0x05:
    {
      value = GETU40AT(msg, 0);
      value &= 0x7f7f7f7f7fULL;

      temp = (value & 0x7f);
      value = ((value >> 1) | temp);
      temp |= (value & 0x3f80);
      value = (((value >> 1) & 0xffffffc000ULL) | temp);
      temp |= (value & 0x1fc000);
      value = (((value >> 1) & 0xffffe00000ULL) | temp);
      temp |= (value & 0x1fe00000);
      value = (((value >> 1) & 0xfff0000000ULL) | temp);
      break;
    }
    case 0x08:
    {
      while(length)
      {
        temp = msg[8-length];
        length--;
        value |= (temp << (length*8));
      }
    }
    default:
      break;
  }
  return value;
}

#ifndef BARE_METAL
/**
 * Convert the tag timestamp to readable format
 * @param rp pointer to reader structure
 * @param read pointer to tag read data structure
 * @param timeStr string to contain converted timestamp
 */
void TMR_getTimeStamp(struct TMR_Reader *rp, const struct TMR_TagReadData *read, char *timeString)
{
  char* timeEnd;
  char* end;
  char timeStr[128];

#ifdef WIN32
  if (rp->readerType == TMR_READER_TYPE_SERIAL)
  {
    FILETIME ft, utc;
    SYSTEMTIME st;

    utc.dwHighDateTime = read->timestampHigh;
    utc.dwLowDateTime = read->timestampLow;

    FileTimeToLocalFileTime( &utc, &ft );
    FileTimeToSystemTime( &ft, &st );
    timeEnd = timeStr + sizeof(timeStr)/sizeof(timeStr[0]);
    end = timeStr;
    end += sprintf(end, "%d-%d-%d", st.wYear,st.wMonth,st.wDay);
    end += sprintf(end, "T%d:%d:%d.%03d", st.wHour,st.wMinute,st.wSecond, st.wMilliseconds);
  }
  else
  {
    uint8_t shift;
    uint64_t timestamp;
    time_t seconds;
    int micros;

    shift = 32;
    timestamp = ((uint64_t)read->timestampHigh<<shift) | read->timestampLow;
    seconds = timestamp / 1000;
    micros = (int) ((timestamp % 1000) * 1000);

    /*
     * Timestamp already includes millisecond part of dspMicros,
     * so subtract this out before adding in dspMicros again
     */
    micros -= read->dspMicros / 1000;
    micros += read->dspMicros;

    timeEnd = timeStr + sizeof(timeStr)/sizeof(timeStr[0]);
    end = timeStr;
    end += strftime(end, timeEnd-end, "%Y-%m-%dT%H:%M:%S", localtime(&seconds));
    end += snprintf(end, timeEnd-end, ".%06d", micros);
    //end += strftime(end, timeEnd-end, "%z", localtime(&seconds));
  }
  #else
  {
    uint8_t shift;
    uint64_t timestamp;
    time_t seconds;
    int micros;

    shift = 32;
    timestamp = ((uint64_t)read->timestampHigh<<shift) | read->timestampLow;
    seconds = timestamp / 1000;
    micros = (timestamp % 1000) * 1000;

    /*
     * Timestamp already includes millisecond part of dspMicros,
     * so subtract this out before adding in dspMicros again
     */
    micros -= read->dspMicros / 1000;
    micros += read->dspMicros;

    timeEnd = timeStr + (sizeof(timeStr)/sizeof(timeStr[0]));
    end = timeStr;
    end += strftime(end, timeEnd-end, "%Y-%m-%dT%H:%M:%S", localtime(&seconds));
    end += snprintf(end, timeEnd-end, ".%06d", micros);
    end += strftime(end, timeEnd-end, "%z", localtime(&seconds));
  }
#endif
  memcpy(timeString, timeStr, (sizeof(timeStr)/sizeof(timeStr[0])));
}
#endif /* BARE_METAL */
