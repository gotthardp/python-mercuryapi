/**
 *  @file llrp_reader_l3.c
 *  @brief Mercury API - LLRP reader low level implementation
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

/**
 * Define this to convert string representation of time
 * to time structure
 */ 
#define _XOPEN_SOURCE


#include "tm_reader.h"
#include "osdep.h"
#ifdef TMR_ENABLE_LLRP_READER

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(WIN32) && !defined(WINCE)
#include <sys/select.h>
#endif

#include <time.h>	
#include "llrp_reader_imp.h"
#include "tmr_utils.h"

#define BACKGROUND_RECEIVER_LOOP_PERIOD 1
#define MAX_KEEP_ALIVE_ACK_MISSES 3
#define TMMP_CUSTOM_RFPHASE        143
#define TMMP_CUSTOM_PROTOCOL_ID    175
#define TMMP_CUSTOM_TAGOP_RESPONSE 216
#define TMMPD_CUSTOM_GPIO_STATUS   224
#define TMMPD_CUSTOM_GEN2          226

void process_async_response(TMR_Reader *reader);

uint8_t TMR_LLRP_gpiListSargas[] = {0,1};
uint8_t TMR_LLRP_gpoListSargas[] = {2,3};
uint8_t TMR_LLRP_gpiListIzar[] = {1,2,3,4};
uint8_t TMR_LLRP_gpoListIzar[] = {1,2,3,4};
uint8_t sizeGpiListSargas = sizeof(TMR_LLRP_gpiListSargas)/sizeof(TMR_LLRP_gpiListSargas[0]);
uint8_t sizeGpoListSargas = sizeof(TMR_LLRP_gpoListSargas)/sizeof(TMR_LLRP_gpoListSargas[0]);
uint8_t sizeGpiListIzar = sizeof(TMR_LLRP_gpiListIzar)/sizeof(TMR_LLRP_gpiListIzar[0]);
uint8_t sizeGpoListIzar = sizeof(TMR_LLRP_gpoListIzar)/sizeof(TMR_LLRP_gpoListIzar[0]);
uint8_t TMR_LLRP_gpiListM6Astra[] = {3,4,6,7};
uint8_t TMR_LLRP_gpoListM6Astra[] = {0,1,2,5};
uint8_t sizeGpiListM6Astra = sizeof(TMR_LLRP_gpiListM6Astra)/sizeof(TMR_LLRP_gpiListM6Astra[0]);
uint8_t sizeGpoListM6Astra = sizeof(TMR_LLRP_gpoListM6Astra)/sizeof(TMR_LLRP_gpoListM6Astra[0]);
static bool tx_mutex_lock_enabled = false;
bool isPerAntennaEnabled = false;
bool isStandaloneTagop = false;
bool isMultiFilterEnabled = false;
bool isStateAwareTargetMapped = false;
uint32_t currentInventorySpecID = 0;
TMR_uint32List tempHopTable;

uint32_t hoptableArr[64];
static TMR_Status
TMR_LLRP_llrpToTmGpi(uint32_t model, llrp_u16_t llrpNum, uint8_t* tmNum)
{
  int index = llrpNum - 1;
  int gpiCount;

  if ((TMR_LLRP_MODEL_IZAR == model) || (TMR_LLRP_MODEL_ASTRA200 == model))
  {
	  gpiCount = sizeGpiListIzar;
  }
  else if (TMR_LLRP_MODEL_SARGAS == model)
  {
	  gpiCount = sizeGpiListSargas;
  }
  else
  {
	  gpiCount = sizeGpiListM6Astra;
  }
  if ((index < 0) || (gpiCount <= index))
  {
    return TMR_ERROR_LLRP_UNDEFINED_VALUE;
  }

  if ((TMR_LLRP_MODEL_IZAR == model) || (TMR_LLRP_MODEL_ASTRA200 == model))
  {
	  *tmNum = TMR_LLRP_gpiListIzar[index];
  }
  else if (TMR_LLRP_MODEL_SARGAS == model)
  {
	  *tmNum = TMR_LLRP_gpiListSargas[index];
  }
  else
  {
	  *tmNum = TMR_LLRP_gpiListM6Astra[index];
  }
  return TMR_SUCCESS;
}

static TMR_Status
TMR_LLRP_tmToLlrpGpo(uint32_t model, uint8_t tmNum, llrp_u16_t *llrpNum)
{
  int gpoCount;
  int i;

  if ((TMR_LLRP_MODEL_IZAR == model) || (TMR_LLRP_MODEL_ASTRA200 == model))
  {
	  gpoCount = sizeGpoListIzar;
  }
  else if (TMR_LLRP_MODEL_SARGAS == model)
  {
	  gpoCount = sizeGpoListSargas;
  }
  else
  {
	  gpoCount = sizeGpoListM6Astra;
  }
  for (i = 0; i < gpoCount; i++)
  {
	  if ((TMR_LLRP_MODEL_IZAR == model) || (TMR_LLRP_MODEL_ASTRA200 == model))
	  {
	    if (TMR_LLRP_gpoListIzar[i] == tmNum) break;
	  }
	  else if (TMR_LLRP_MODEL_SARGAS == model)
	  {
	    if (TMR_LLRP_gpoListSargas[i] == tmNum) break;
	  }
	  else
	  {
	    if (TMR_LLRP_gpoListM6Astra[i] == tmNum) break;
	  }
  }
  if (i == gpoCount)
  {
	  return TMR_ERROR_INVALID;
  }
  else
  {
	  *llrpNum = i + 1;
	  return TMR_SUCCESS;
  }
}

/**
 * Notify transport listener,
 * Called from SendMessage and ReceiveMessage
 *
 * @param reader The reader
 * @param pMsg Pointer to Message to send (of type LLRP_tSMessage * for llrp reader)
 * @param tx True if called from SendMessage, false if called from ReceiveMessage
 * @param timeout Timeout value.
 */
TMR_Status
TMR_LLRP_notifyTransportListener(TMR_Reader *reader, LLRP_tSMessage *pMsg, bool tx, int timeout)
{
  char buf[150*1024];

  if (LLRP_RC_OK != LLRP_toXMLString(&pMsg->elementHdr, buf, sizeof(buf)))
  {
    TMR__notifyTransportListeners(reader, tx, 0, (uint8_t *)buf, timeout);
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR; 
  }

  TMR__notifyTransportListeners(reader, tx, 0, (uint8_t *)buf, timeout);

  return TMR_SUCCESS;
}

/**
 * Send a message to the reader
 *
 * @param reader The reader
 * @param[in] pMsg Pointer to Message to send.
 * @param timeoutMs Timeout value.
 */
TMR_Status
TMR_LLRP_sendMessage(TMR_Reader *reader, LLRP_tSMessage *pMsg, int timeoutMs)
{
  LLRP_tSConnection *pConn = reader->u.llrpReader.pConn;
  timeoutMs += reader->u.llrpReader.transportTimeout;

  if (NULL == pConn)
  {
    return TMR_ERROR_LLRP_SENDIO_ERROR;
  }

  pMsg->MessageID = reader->u.llrpReader.msgId ++;

  if (NULL != reader->transportListeners)
  {
    TMR_LLRP_notifyTransportListener(reader, pMsg, true, timeoutMs);
  }
  /*
   * If LLRP_Conn_sendMessage() returns other than LLRP_RC_OK
   * then there was an error.
   */
  if(true == reader->continuousReading)
  {
    pthread_mutex_lock(&reader->u.llrpReader.transmitterLock);
    tx_mutex_lock_enabled = true;
  }
  if (LLRP_RC_OK != LLRP_Conn_sendMessage(pConn, pMsg))
  {
    const LLRP_tSErrorDetails *pError = LLRP_Conn_getSendError(pConn);
    sprintf(reader->u.llrpReader.errMsg, "ERROR: %s sendMessage failed, %s",
                pMsg->elementHdr.pType->pName,
                pError->pWhatStr ? pError->pWhatStr : "no reason given");

    if(true == tx_mutex_lock_enabled)
    {
      pthread_mutex_unlock(&reader->u.llrpReader.transmitterLock);
      tx_mutex_lock_enabled = false;
    }
    return TMR_ERROR_LLRP_SENDIO_ERROR;
  }
  if(true == tx_mutex_lock_enabled)
  {
    pthread_mutex_unlock(&reader->u.llrpReader.transmitterLock);
    tx_mutex_lock_enabled = false;
  }
  return TMR_SUCCESS;
}

/**
 * Receive a response.
 *
 * @param reader The reader
 * @param[out] pMsg Message received.
 * @param timeoutMs Timeout value.
 */
TMR_Status
TMR_LLRP_receiveMessage(TMR_Reader *reader, LLRP_tSMessage **pMsg, int timeoutMs)
{
  LLRP_tSConnection *pConn = reader->u.llrpReader.pConn;
  timeoutMs += reader->u.llrpReader.transportTimeout;

  if (NULL == pConn)
  {
    return TMR_ERROR_LLRP_RECEIVEIO_ERROR;
  }

  /*
   * Receive the message subject to a time limit
   */
  *pMsg = LLRP_Conn_recvMessage(pConn, timeoutMs);
  /*
   * If LLRP_Conn_recvMessage() returns NULL then there was
   * an error.
   */
  if(NULL == *pMsg)
  {
    const LLRP_tSErrorDetails *pError = LLRP_Conn_getRecvError(pConn);
    
    sprintf(reader->u.llrpReader.errMsg,
            "ERROR: recvMessage failed, %s",
            pError->pWhatStr ? pError->pWhatStr : "no reason given");
    
    return TMR_ERROR_LLRP_RECEIVEIO_ERROR;
  }
#ifndef WINCE
  TMR_LLRP_notifyTransportListener(reader, *pMsg, false, timeoutMs);
#endif
  return TMR_SUCCESS;
}

/**
 * Send a message and receive a response with timeout.
 *
 * @param reader The reader
 * @param[in] pMsg Message to send
 * @param[out] pRsp Message received.
 * @param timeoutMs Timeout value.
 */
TMR_Status
TMR_LLRP_sendTimeout(TMR_Reader *reader, LLRP_tSMessage *pMsg, LLRP_tSMessage **pRsp, int timeoutMs)
{
  TMR_Status ret;
  bool rx_mutex_lock_enabled = false;
  bool backGroundReceiverDisabled = false;

  if (false == reader->continuousReading)
  {
    /**
     * If not in continuous reading, then disable background receiver
     **/
    TMR_LLRP_setBackgroundReceiverState(reader, false);
    backGroundReceiverDisabled = true;
  }

  ret = TMR_LLRP_sendMessage(reader, pMsg, timeoutMs);
  if (TMR_SUCCESS != ret)
  {
    goto out;
  }

repeat:
  if(true == reader->continuousReading)
  {
    pthread_mutex_lock(&reader->u.llrpReader.receiverLock);
    rx_mutex_lock_enabled = true;
  }
  ret = TMR_LLRP_receiveMessage(reader, pRsp, timeoutMs);
  if (TMR_SUCCESS != ret)
  {
    if(rx_mutex_lock_enabled == true)
    {
      pthread_mutex_unlock(&reader->u.llrpReader.receiverLock);
      rx_mutex_lock_enabled = false;
    }
    goto out;
  }

  if (pMsg->elementHdr.pType->pResponseType != (*pRsp)->elementHdr.pType)
  {
    /**
     * If the response message type and the expected response type
     * are not equal, (i.e., you can get any asynchronous response like events
     * which is not the correct response for the message sent)
     * Handle the other response, and go back to receive.
     * No need for error checking though.
     **/
    if(reader->u.llrpReader.isResponsePending == true)
    {
      if(pMsg->elementHdr.pType->pResponseType == reader->u.llrpReader.unhandledAsyncResponse.lMsg->elementHdr.pType)
      {
        *pRsp = reader->u.llrpReader.unhandledAsyncResponse.lMsg;
        reader->u.llrpReader.isResponsePending = false;
        if(rx_mutex_lock_enabled == true)
        {
          pthread_mutex_unlock(&reader->u.llrpReader.receiverLock);
          rx_mutex_lock_enabled = false;
        }
        goto out;
      }
    }
    TMR_LLRP_processReceivedMessage(reader, *pRsp);
    if(rx_mutex_lock_enabled == true)
    {
      pthread_mutex_unlock(&reader->u.llrpReader.receiverLock);
      rx_mutex_lock_enabled = false;
    }
    goto repeat;
  }
  if(rx_mutex_lock_enabled == true)
  {
    pthread_mutex_unlock(&reader->u.llrpReader.receiverLock);
    rx_mutex_lock_enabled = false;
  }

out:
  if (true == backGroundReceiverDisabled)
  {
    /**
     * Re-enable background reader when not in continuous reading.
     **/
    TMR_LLRP_setBackgroundReceiverState(reader, true);
    backGroundReceiverDisabled = false;
  }
  return ret;
}

/**
 * Send a message and receive a response.
 *
 * @param reader The reader
 * @param[in] pMsg Message to send
 * @param[out] pRsp Message received.
 */
TMR_Status
TMR_LLRP_send(TMR_Reader *reader, LLRP_tSMessage *pMsg, LLRP_tSMessage **pRsp)
{
  return TMR_LLRP_sendTimeout(reader, pMsg, pRsp,
                              reader->u.llrpReader.commandTimeout
                              + reader->u.llrpReader.transportTimeout);
}

/**
 * Free LLRP message
 *
 * @param pMsg Message to free
 */
void
TMR_LLRP_freeMessage(LLRP_tSMessage *pMsg)
{
  /* Free only if the message is not null */
  if (NULL != pMsg)
  {
  LLRP_Element_destruct(&pMsg->elementHdr);
    pMsg = NULL;
  }
}

/**
 * Check LLRP Message status
 * When a message is received, call this function
 * to check the LLRP message status
 *
 * @param pLLRPStatus Pointer to LLRP status parameter in LLRP message
 */
TMR_Status
TMR_LLRP_checkLLRPStatus(LLRP_tSLLRPStatus *pLLRPStatus)
{
  if (LLRP_StatusCode_M_Success != pLLRPStatus->eStatusCode)
  {
    return TMR_ERROR_LLRP;
  }
  else
  {
    return TMR_SUCCESS;
  }
}

/**
 * Command to get region id
 *
 * @param reader Reader pointer
 * @param[out] region Pointer to TMR_Region object to hold the region value
 */
TMR_Status
TMR_LLRP_cmdGetRegion(TMR_Reader *reader, TMR_Region *region)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration  *pTMRegionConfig;
  LLRP_tSParameter                      *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/region/id" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMRegionConfig = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMRegionConfig)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., region configuration)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMRegionConfig, LLRP_ThingMagicControlConfiguration_ThingMagicRegionConfiguration);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMRegionConfig->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMRegionConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract region id from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    *region = LLRP_ThingMagicRegionConfiguration_getRegionID((LLRP_tSThingMagicRegionConfiguration *) pCustParam);
  }
  else
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }
  
  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}
/**
 * Command to get the read async offtime
 *
 * @param reader Reader pointer
 * @param[out] offtime of uin32_t to hold the off time
 **/ 
TMR_Status
TMR_LLRP_cmdGetTMAsyncOffTime(TMR_Reader *reader, uint32_t *offtime)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration  *pTMAsyncOffTime;
  LLRP_tSParameter                      *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/asyncofftime" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMAsyncOffTime = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMAsyncOffTime)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., AsyncOffTime)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMAsyncOffTime, LLRP_ThingMagicControlConfiguration_ThingMagicAsyncOFFTime);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMAsyncOffTime->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMAsyncOffTime);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract async offtime from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    *offtime = LLRP_ThingMagicAsyncOFFTime_getAsyncOFFTime((LLRP_tSThingMagicAsyncOFFTime *) pCustParam);
  }
  else
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to get the read async ontime
 *
 * @param reader Reader pointer
 * @param[out] ontime of uin32_t to hold the on time
 **/ 
TMR_Status
TMR_LLRP_cmdGetTMAsyncOnTime(TMR_Reader *reader, uint32_t *ontime)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration  *pTMAsyncOnTime;
  LLRP_tSParameter                      *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/asyncontime" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMAsyncOnTime = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMAsyncOnTime)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., AsyncOnTime)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMAsyncOnTime, LLRP_ThingMagicControlConfiguration_ThingMagicAsyncONTime);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMAsyncOnTime->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMAsyncOnTime);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract async ontime from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    *ontime = LLRP_ThingMagicAsyncONTime_getAsyncONTime((LLRP_tSThingMagicAsyncONTime *) pCustParam);
  }
  else
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to get the metadata
 *
 * @param reader Reader pointer
 * @param[out] metadata of uin16_t to hold the metadata flag
 **/ 
TMR_Status
TMR_LLRP_cmdGetTMMetadataFlag(TMR_Reader *reader, uint16_t *metadata)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration  *pTMMetadataFlag;
  LLRP_tSParameter                      *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/metadata" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMMetadataFlag = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMMetadataFlag)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., MetadataFlag)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMMetadataFlag, LLRP_ThingMagicControlConfiguration_ThingMagicMetadata);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMMetadataFlag->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMMetadataFlag);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract metadata from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    *metadata = LLRP_ThingMagicMetadata_getMetadata((LLRP_tSThingMagicMetadata *) pCustParam);
  }
  else
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to get the StatsEnable
 *
 * @param reader Reader pointer
 * @param[out] StatsEnable of uint16_t to hold the StatsEnable flag
 **/ 
TMR_Status
TMR_LLRP_cmdGetTMStatsEnable(TMR_Reader *reader, uint16_t *statsEnable)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration  *pTMStatsEnableFlag;
  LLRP_tSParameter                      *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/stats/enable" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMStatsEnableFlag = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMStatsEnableFlag)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., StatsEnable)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMStatsEnableFlag, LLRP_ThingMagicControlConfiguration_ThingMagicStatsEnable);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMStatsEnableFlag->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMStatsEnableFlag);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract stats enable flag from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    *statsEnable = LLRP_ThingMagicStatsEnable_getStatsEnable((LLRP_tSThingMagicStatsEnable *) pCustParam);
  }
  else
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to get the StatsValue
 *
 * @param reader Reader pointer
 * @param[out] StatsValue of TMR_Reader_StatsValues to hold the StatsValue
 **/ 
TMR_Status
TMR_LLRP_cmdGetTMStatsValue(TMR_Reader *reader, TMR_Reader_StatsValues *statsValue)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration  *pTMStatsValue;
  LLRP_tSParameter                      *pCustParam;
  LLRP_tSCustomStatsValue               *customStats;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/stats/statsValue" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMStatsValue = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMStatsValue)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., StatsValue)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMStatsValue, LLRP_ThingMagicControlConfiguration_ThingMagicReaderStats);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMStatsValue->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMStatsValue);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**`
   * Response is success, extract stats value from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    customStats = LLRP_ThingMagicReaderStats_getCustomStatsValue((LLRP_tSThingMagicReaderStats *) pCustParam);
    if (NULL != customStats)
    {
      TMR_LLRP_parseCustomStatsValues(customStats, statsValue);
    }
    else
    {
      TMR_LLRP_freeMessage(pRspMsg);
      return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
    }
  }
  else
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

void
TMR_LLRP_parseCustomStatsValues(LLRP_tSCustomStatsValue *customStats, TMR_Reader_StatsValues *statsValue)
{
  llrp_u8v_t                            connectedAntennas;
  LLRP_tSperAntennaStatsList            *pPerAntennaParam;
  LLRP_tSConnectedAntennaList           *pConnectedAntennas;
  LLRP_tSTemperatureParam               *pTemperatureParam;
  LLRP_tSProtocolParam                  *pProtocolParam;
  LLRP_tSAntennaParam                   *pAntennaParam;
  LLRP_tSFrequencyParam                 *pFrequencyParam;
  LLRP_tSRFOntimeParam                  *pRFOntimeParam;
  LLRP_tSNoiseFloorParam                *pNoiseFloorParam;
  unsigned int                          i;
  statsValue->valid = (TMR_Reader_StatsFlag)LLRP_CustomStatsValue_getStatsEnable(customStats);
  if (statsValue->valid & TMR_READER_STATS_FLAG_TEMPERATURE)
  {
    pTemperatureParam = LLRP_CustomStatsValue_getTemperatureParam(customStats);
    statsValue->temperature = (int8_t)LLRP_TemperatureParam_getTemperature(pTemperatureParam);
  }
  if (statsValue->valid & TMR_READER_STATS_FLAG_PROTOCOL)
  {
    pProtocolParam = LLRP_CustomStatsValue_getProtocolParam(customStats);
    statsValue->protocol = (TMR_TagProtocol)LLRP_ProtocolParam_getProtocol(pProtocolParam);
  }
  if (statsValue->valid & TMR_READER_STATS_FLAG_ANTENNA_PORTS)
  {
    pAntennaParam = LLRP_CustomStatsValue_getAntennaParam(customStats);
    statsValue->antenna = (uint16_t)LLRP_AntennaParam_getAntenna(pAntennaParam);
  }
  if (statsValue->valid & TMR_READER_STATS_FLAG_FREQUENCY)
  {
    pFrequencyParam = LLRP_CustomStatsValue_getFrequencyParam(customStats);
    statsValue->frequency = (uint32_t)LLRP_FrequencyParam_getFrequency(pFrequencyParam);
  }
  if (statsValue->valid & TMR_READER_STATS_FLAG_CONNECTED_ANTENNAS)
  {
    pConnectedAntennas = LLRP_CustomStatsValue_getConnectedAntennaList(customStats);
    connectedAntennas = LLRP_ConnectedAntennaList_getconnectedAntennas(pConnectedAntennas);
    statsValue->connectedAntennas.len = connectedAntennas.nValue;
    if((connectedAntennas.nValue > 0) && (connectedAntennas.pValue != NULL))
    {
      memcpy(statsValue->connectedAntennas.list, connectedAntennas.pValue,
              connectedAntennas.nValue);
    }
  }
  for (pPerAntennaParam = LLRP_CustomStatsValue_beginperAntennaStatsList(customStats),
        i = 0;
        (pPerAntennaParam != NULL);
        pPerAntennaParam= LLRP_CustomStatsValue_nextperAntennaStatsList(pPerAntennaParam),
        i++)
  {
    statsValue->perAntenna.list[i].antenna    = (uint8_t)pPerAntennaParam->antenna;
    if (statsValue->valid & TMR_READER_STATS_FLAG_RF_ON_TIME)
    {
      pRFOntimeParam = LLRP_perAntennaStatsList_getRFOntimeParam(pPerAntennaParam);
      statsValue->perAntenna.list[i].rfOnTime   = (uint32_t)pRFOntimeParam->rfOntime;
    }
    if (statsValue->valid & TMR_READER_STATS_FLAG_NOISE_FLOOR_SEARCH_RX_TX_WITH_TX_ON)
    {
      pNoiseFloorParam = LLRP_perAntennaStatsList_getNoiseFloorParam(pPerAntennaParam);
      statsValue->perAntenna.list[i].noiseFloor = (int8_t)pNoiseFloorParam->noiseFloor;
    }
  }
  statsValue->perAntenna.len = i;
}

 /**
 * Command to get the antenna port returnloss
 *
 * @param reader Reader pointer
 * @param[out] returnloss of TMR_PortValueList to hold the returnloss list
 **/ 
TMR_Status
TMR_LLRP_cmdGetTMAntennaReturnloss(TMR_Reader *reader, TMR_PortValueList *returnloss)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG                     *pCmd;
  LLRP_tSMessage                               *pCmdMsg;
  LLRP_tSMessage                               *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE            *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration  *pTMAntennaReturnloss;
  LLRP_tSParameter                             *pCustParam;
  LLRP_tSReturnlossValue                       *pReturnlossParam;
  uint8_t                                      i;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/AntennaReturnloss" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMAntennaReturnloss = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMAntennaReturnloss)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., AntennaReturnloss)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMAntennaReturnloss,
          LLRP_ThingMagicControlConfiguration_ThingMagicAntennaReturnloss);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMAntennaReturnloss->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMAntennaReturnloss);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract return loss from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL == pCustParam)
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }
  returnloss->len = 0;

  for (pReturnlossParam = LLRP_ThingMagicAntennaReturnloss_beginReturnlossValue((LLRP_tSThingMagicAntennaReturnloss *) pCustParam),
         i = 0; pReturnlossParam != NULL; 
         pReturnlossParam = LLRP_ThingMagicAntennaReturnloss_nextReturnlossValue((LLRP_tSReturnlossValue *)pReturnlossParam), i++)
  {
    returnloss->list[returnloss->len].port = LLRP_ReturnlossValue_getPort((LLRP_tSReturnlossValue *)pReturnlossParam);
    returnloss->list[returnloss->len++].value = LLRP_ReturnlossValue_getValue((LLRP_tSReturnlossValue *)pReturnlossParam);
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to Detect antenna connection status
 *
 * @param reader Reader pointer
 * @param count[out] Number of antennas detected 
 * @param[out] ports Pointer to TMR_LLRP_PortDetect object
 */
TMR_Status
TMR_LLRP_cmdAntennaDetect(TMR_Reader *reader, uint8_t *count, TMR_LLRP_PortDetect *ports)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSAntennaProperties              *pAntProps;
  uint8_t                               i;

  ret = TMR_SUCCESS;
  i = 0;
  /**
   * /reader/antenna/connectedPortList parameter is available
   * as an LLRP standard parameter GET_READER_CONFIG_RESPONSE.AntennaProperties.AntennaConnected.
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_AntennaProperties);
  LLRP_GET_READER_CONFIG_setAntennaID(pCmd, 0);

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract connected port list from it
   **/
  for (pAntProps = LLRP_GET_READER_CONFIG_RESPONSE_beginAntennaProperties(pRsp),
      i = 0;
      (pAntProps != NULL) && (i < *count);
      pAntProps = LLRP_GET_READER_CONFIG_RESPONSE_nextAntennaProperties(pAntProps),
      i ++)
  {
    ports[i].port       = pAntProps->AntennaID;
    ports[i].connected  = pAntProps->AntennaConnected;
    ports[i].gain       = pAntProps->AntennaGain;
  }
  *count = i;

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to get thingmagic Device Information Capabilities
 *
 * @param reader Reader pointer
 * @param version Pointer to TMR_String to hold the version hardware value 
 */
TMR_Status
TMR_LLRP_cmdGetTMDeviceInformationCapabilities(TMR_Reader *reader, int param, TMR_String *version)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CAPABILITIES              *pCmd;
  LLRP_tSMessage                              *pCmdMsg;
  LLRP_tSMessage                              *pRspMsg;
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE     *pRsp;
  LLRP_tSThingMagicDeviceControlCapabilities  *pTMCaps;
  LLRP_tSParameter                            *pCustParam;
  llrp_utf8v_t                                 hardwareVersion;
  llrp_utf8v_t                                 serialNumber;
  llrp_utf8v_t                                 productGroup;
  LLRP_tSReaderProductGroup                   *pReaderProductGroup;

  ret = TMR_SUCCESS;

  /**
   * Initialize GET_READER_CAPABILITIES message
   **/
  pCmd = LLRP_GET_READER_CAPABILITIES_construct();
  LLRP_GET_READER_CAPABILITIES_setRequestedData(pCmd, LLRP_GetReaderCapabilitiesRequestedData_General_Device_Capabilities);

  /**
   * /reader/version/hardware is a custom parameter.And is available as part of
   * ThingMagicDeviceControlCapabilities.ThingMagicControlCapabilities.DeviceInformationCapabilities
   **/
  pTMCaps = LLRP_ThingMagicDeviceControlCapabilities_construct();
  if (NULL == pTMCaps)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * set the requested data
   * and add to GET_READER_CAPABILITIES message
   **/
  LLRP_ThingMagicDeviceControlCapabilities_setRequestedData(pTMCaps, LLRP_ThingMagicControlCapabilities_DeviceInformationCapabilities);
  if (LLRP_RC_OK != LLRP_GET_READER_CAPABILITIES_addCustom(pCmd, &pTMCaps->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMCaps);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }
  pCmdMsg       = &pCmd->hdr;

  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CAPABILITIES_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }
  /**
   * Response is success
   * Extract hardware version from it
   **/
  pCustParam = LLRP_GET_READER_CAPABILITIES_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    if(param == TMR_PARAM_VERSION_HARDWARE)
    {
      hardwareVersion = ((LLRP_tSDeviceInformationCapabilities *)pCustParam)->HardwareVersion;
      TMR_stringCopy(version, (char *)hardwareVersion.pValue, (int)hardwareVersion.nValue);
    }
    else if(param == TMR_PARAM_VERSION_SERIAL)
    {
      serialNumber = ((LLRP_tSDeviceInformationCapabilities *)pCustParam)->ReaderSerialNumber;
      TMR_stringCopy(version, (char *)serialNumber.pValue, (int)serialNumber.nValue);
    }
    else if(param == TMR_PARAM_PRODUCT_GROUP)
    {
      pReaderProductGroup = LLRP_DeviceInformationCapabilities_getReaderProductGroup(
          (LLRP_tSDeviceInformationCapabilities *)pCustParam);
      productGroup = LLRP_ReaderProductGroup_getProductGroup(pReaderProductGroup);
      TMR_stringCopy(version, (char *)productGroup.pValue, (int)productGroup.nValue);
    }
  }
  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;

}

/**
 * Command to get thingmagic Device Information IDs
 *
 * @param reader Reader pointer
 * @param id Pointer to uint16_t to hold the product ID or Product group ID 
 */
TMR_Status
TMR_LLRP_cmdGetTMDeviceInformationIDs(TMR_Reader *reader, int param, uint16_t *id)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CAPABILITIES              *pCmd;
  LLRP_tSMessage                              *pCmdMsg;
  LLRP_tSMessage                              *pRspMsg;
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE     *pRsp;
  LLRP_tSThingMagicDeviceControlCapabilities  *pTMCaps;
  LLRP_tSParameter                            *pCustParam;
  llrp_u16_t                                   productID;
  llrp_u16_t                                   productGroupID;
  LLRP_tSReaderProductID                      *pReaderProductID;
  LLRP_tSReaderProductGroupID                 *pReaderProductGroupID;

  ret = TMR_SUCCESS;

  /**
   * Initialize GET_READER_CAPABILITIES message
   **/
  pCmd = LLRP_GET_READER_CAPABILITIES_construct();
  LLRP_GET_READER_CAPABILITIES_setRequestedData(pCmd, LLRP_GetReaderCapabilitiesRequestedData_General_Device_Capabilities);

  /**
   * /reader/version/productID and /reader/version/productGroupID are custom parameters.And is available as part of
   * ThingMagicDeviceControlCapabilities.ThingMagicControlCapabilities.DeviceInformationCapabilities
   **/
  pTMCaps = LLRP_ThingMagicDeviceControlCapabilities_construct();
  if (NULL == pTMCaps)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * set the requested data
   * and add to GET_READER_CAPABILITIES message
   **/
  LLRP_ThingMagicDeviceControlCapabilities_setRequestedData(pTMCaps, LLRP_ThingMagicControlCapabilities_DeviceInformationCapabilities);
  if (LLRP_RC_OK != LLRP_GET_READER_CAPABILITIES_addCustom(pCmd, &pTMCaps->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMCaps);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }
  pCmdMsg       = &pCmd->hdr;

  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CAPABILITIES_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }
  /**
   * Response is success
   * Extract product IDs from it
   **/
  pCustParam = LLRP_GET_READER_CAPABILITIES_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    if(param == TMR_PARAM_PRODUCT_ID)
    {
      pReaderProductID = LLRP_DeviceInformationCapabilities_getReaderProductID(
          (LLRP_tSDeviceInformationCapabilities *)pCustParam);
      productID = LLRP_ReaderProductID_getProductID(pReaderProductID);
      *(uint16_t*)id = (uint16_t)productID;
    }
    else if(param == TMR_PARAM_PRODUCT_GROUP_ID)
    {
      pReaderProductGroupID = LLRP_DeviceInformationCapabilities_getReaderProductGroupID(
          (LLRP_tSDeviceInformationCapabilities *)pCustParam);
      productGroupID = LLRP_ReaderProductGroupID_getProductGroupID(pReaderProductGroupID);
      *(uint16_t*)id = (uint16_t)productGroupID;
    }
  }
  else
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }
  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;

}

TMR_GEN2_Tari
TMR_convertTari(uint16_t tari)
{
  switch(tari) {
  case 6250:
    return TMR_GEN2_TARI_6_25US;
    break;
  case 12500:
    return TMR_GEN2_TARI_12_5US;
    break;
  case 25000:
    return TMR_GEN2_TARI_25US;
    break;
  default:
    return TMR_GEN2_TARI_25US;
    break;
  }
}

/**
 * Command to get Reader capabilities
 *
 * @param reader Reader pointer
 * @param capabilities Pointer to TMR_LLRP_ReaderCapabilities
 */
TMR_Status
TMR_LLRP_cmdGetReaderCapabilities(TMR_Reader *reader, TMR_LLRP_ReaderCapabilities *capabilities)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CAPABILITIES          *pCmd;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSMessage                          *pRspMsg;
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE *pRsp;
  TMR_GEN2_Tari minTari, maxTari;

  ret = TMR_SUCCESS;

  /**
   * Retrieve all reader capabilities
   * Initialize GET_READER_CAPABILITIES message 
   **/
  pCmd = LLRP_GET_READER_CAPABILITIES_construct();
  LLRP_GET_READER_CAPABILITIES_setRequestedData(pCmd, LLRP_GetReaderCapabilitiesRequestedData_All);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CAPABILITIES_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract all required information
   **/

  /**
   * We can also cache, model and software version
   * This eliminates explicit functions to get model and software versions.
   * Extract General device capabilities
   **/
  {
    /**
     * Extract model
     **/
    LLRP_tSGeneralDeviceCapabilities        *pReaderCap;
    pReaderCap = LLRP_GET_READER_CAPABILITIES_RESPONSE_getGeneralDeviceCapabilities(pRsp);

    if (TM_MANUFACTURER_ID == pReaderCap->DeviceManufacturerName)
    {
      capabilities->model = pReaderCap->ModelName;
    }

    /**
     * Extract software version
     **/
    memcpy(capabilities->softwareVersion, pReaderCap->ReaderFirmwareVersion.pValue, 
                                                  pReaderCap->ReaderFirmwareVersion.nValue );
    capabilities->softwareVersion[pReaderCap->ReaderFirmwareVersion.nValue] = '\0';
  }

  /**
   * Extract Regulatory capabilities
   **/
  {
    LLRP_tSRegulatoryCapabilities *pRegCaps;
    pRegCaps = LLRP_GET_READER_CAPABILITIES_RESPONSE_getRegulatoryCapabilities(pRsp);

    {
      /**
       * Extract power leve table from response.
       **/
      LLRP_tSTransmitPowerLevelTableEntry     *pTableEntry;

      /*Initialize cached powerTable */
      TMR_uint16List *table = &capabilities->powerTable;
      table->max = 255;
      table->len = 0;
      table->list = capabilities->powerList;

      /* Before extracting the power level initialize the powerList table to zero*/
      memset(table->list, 0, table->max*(sizeof(uint16_t)));


      for (pTableEntry = LLRP_UHFBandCapabilities_beginTransmitPowerLevelTableEntry(
                                                              pRegCaps->pUHFBandCapabilities);
          (pTableEntry != NULL);
          pTableEntry = LLRP_UHFBandCapabilities_nextTransmitPowerLevelTableEntry(pTableEntry))
      {
        table->list[pTableEntry->Index] = pTableEntry->TransmitPowerValue;
        table->len ++;
      }
    }

    {
      /**
       * Extract Frequency information from response
       **/
      LLRP_tSFrequencyHopTable  *pFreqHopTable;
      llrp_u16_t count;
      uint8_t i;

      /*Initialize cached frequency table */
      TMR_uint32List *table = &capabilities->freqTable;
      table->max = 64;
      table->len = 0;
      table->list = capabilities->freqList;

      /* Before extracting the frequency  initialize the freqList table to zero*/
      memset(table->list, 0, table->max*(sizeof(uint32_t)));

      pFreqHopTable = LLRP_FrequencyInformation_beginFrequencyHopTable(
                                pRegCaps->pUHFBandCapabilities->pFrequencyInformation);

      for (count = 0; count < pFreqHopTable->Frequency.nValue; count ++)
      {
        table->list[count] = pFreqHopTable->Frequency.pValue[count];
        table->len ++;
      }
      tempHopTable.list = hoptableArr;
      for(i = 0; i < table->len; i++)
      {
        tempHopTable.list[i] = table->list[i];
      }
      tempHopTable.len = table->len;
    }
    {
      /**
       * Iterate through listAirProtocolUHFRFModeTable
       **/
      LLRP_tSParameter *pRFModeTable;

      for (pRFModeTable = LLRP_UHFBandCapabilities_beginAirProtocolUHFRFModeTable(pRegCaps->pUHFBandCapabilities);
          (pRFModeTable != NULL);
          pRFModeTable = LLRP_UHFBandCapabilities_nextAirProtocolUHFRFModeTable(pRFModeTable))
      {
        /* Check for Mode table protocol */
        if (&LLRP_tdC1G2UHFRFModeTable == pRFModeTable->elementHdr.pType)
        {
          /**
           * It is a Gen2 protocol RFMode table.
           * Currently we are using only BLF and tagEncoding values
           **/
          LLRP_tSC1G2UHFRFModeTableEntry *pModeEntry;

          /* Iterate through list of entries in table */
          for (pModeEntry = LLRP_C1G2UHFRFModeTable_beginC1G2UHFRFModeTableEntry(
                                                (LLRP_tSC1G2UHFRFModeTable *)pRFModeTable);
              (pModeEntry != NULL);
              pModeEntry = LLRP_C1G2UHFRFModeTable_nextC1G2UHFRFModeTableEntry(pModeEntry))
          {
            TMR_GEN2_LinkFrequency blf;
            llrp_u32_t index        = pModeEntry->ModeIdentifier;
            llrp_u32_t bdr          = pModeEntry->BDRValue;
            llrp_u32_t minTariValue = pModeEntry->MinTariValue;
            llrp_u32_t maxTariValue = pModeEntry->MaxTariValue;

            switch (bdr)
            {
              case 250000:
                blf = TMR_GEN2_LINKFREQUENCY_250KHZ;
                break;

              case 640000:
                blf = TMR_GEN2_LINKFREQUENCY_640KHZ;
                break;

              case 320000:
                blf = TMR_GEN2_LINKFREQUENCY_320KHZ;
                break;

              default:
                blf = TMR_GEN2_LINKFREQUENCY_250KHZ;
                break;
            }
            /* In the tari enum smaller enum values = higher
             * tari values. i.e. tari25us=enum 0 and tari6.25= enum 2.
             * Hence, swapping the values
             */ 
            maxTari = TMR_convertTari((uint16_t)minTariValue);
            minTari = TMR_convertTari((uint16_t)maxTariValue);

            /** Cache blf value */
            capabilities->u.gen2Modes[index].blf = blf;
            /** Cache m value */
            capabilities->u.gen2Modes[index].m = pModeEntry->eMValue;
            /** Cache minTari value */
            capabilities->u.gen2Modes[index].minTari = minTari;
            /** Cache maxTari value */
            capabilities->u.gen2Modes[index].maxTari = maxTari;
          }
        }
        else
        {
          /**
           * TODO: Need to implement for other protocols
           **/
          TMR_LLRP_freeMessage(pRspMsg);
          return TMR_ERROR_UNIMPLEMENTED_FEATURE;
        }
      }
    } /* End of iterating through listAirProtocolUHFRFModeTable */
  } /* End of extracting Regulatory capabilities */

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to get Regulatory capabilities
 *
 * @param reader Reader pointer
 * @param capabilities Pointer to TMR_LLRP_ReaderCapabilities
 */
TMR_Status
TMR_LLRP_cmdGetRegulatoryCapabilities(TMR_Reader *reader, TMR_uint32List *table)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CAPABILITIES          *pCmd;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSMessage                          *pRspMsg;
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE *pRsp;

  ret = TMR_SUCCESS;

  /**
   * Retrieve reader regulatory capabilities
   * Initialize GET_READER_CAPABILITIES message 
   **/
  pCmd = LLRP_GET_READER_CAPABILITIES_construct();
  LLRP_GET_READER_CAPABILITIES_setRequestedData(pCmd, LLRP_GetReaderCapabilitiesRequestedData_Regulatory_Capabilities);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CAPABILITIES_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, Extract Regulatory capabilities
   **/
  {
    LLRP_tSRegulatoryCapabilities *pRegCaps;
    pRegCaps = LLRP_GET_READER_CAPABILITIES_RESPONSE_getRegulatoryCapabilities(pRsp);

    {
      /**
       * Extract Frequency information from response
       **/
      LLRP_tSFrequencyHopTable  *pFreqHopTable;
      llrp_u16_t count;

      /* Before extracting the frequency  initialize the freqList table to zero*/
      memset(table->list, 0, table->max*(sizeof(uint32_t)));
      table->len = 0;

      pFreqHopTable = LLRP_FrequencyInformation_beginFrequencyHopTable(
                                pRegCaps->pUHFBandCapabilities->pFrequencyInformation);

      for (count = 0; count < pFreqHopTable->Frequency.nValue; count ++)
      {
        table->list[count] = pFreqHopTable->Frequency.pValue[count];
        table->len ++;
      }
    }
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to set GPO state
 *
 * @param reader Reader pointer
 * @param pins[in] Pointer to TMR_GpioPin array
 */
TMR_Status
TMR_LLRP_cmdSetGPOState(TMR_Reader *reader, uint8_t count,
                        const TMR_GpioPin state[])
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE     *pRsp;
  uint8_t i;

  ret = TMR_SUCCESS;

  /**
   * GPO state can be set  LLRP standard parameter
   * SET_READER_CONFIG.GPOWriteData
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Set GPO state as per the list supplied */
  for (i=0; i<count; i++)
  {
    uint8_t id;
    bool high;
    llrp_u16_t llrpId;
    LLRP_tSGPOWriteData *pParam;

    id = state[i].id;
    high = state[i].high;

    /* Construct LLRP parameter */
    pParam = LLRP_GPOWriteData_construct();
    ret = TMR_LLRP_tmToLlrpGpo(reader->u.llrpReader.capabilities.model, id, &llrpId);
    if (TMR_SUCCESS != ret) { return ret; }
    LLRP_GPOWriteData_setGPOPortNumber(pParam, llrpId);
    LLRP_GPOWriteData_setGPOData(pParam, high);

    /* Add param */
    LLRP_SET_READER_CONFIG_addGPOWriteData(pCmd, pParam);
  }

  /**
   * Now the message is framed completely and send the message
   **/
  pCmdMsg = &pCmd->hdr;

  if (reader->continuousReading)
  {
    ret = TMR_LLRP_sendMessage(reader, pCmdMsg, reader->u.llrpReader.transportTimeout);
    if (TMR_SUCCESS != ret)
    {
      TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
      return ret;
    }

    while (false == reader->u.llrpReader.isResponsePending)
    {
      tmr_sleep(10);
    }

    if (NULL != reader->u.llrpReader.unhandledAsyncResponse.lMsg)
    {
      if (pCmdMsg->elementHdr.pType->pResponseType == reader->u.llrpReader.unhandledAsyncResponse.lMsg->elementHdr.pType)
      {
        TMR_LLRP_freeMessage((LLRP_tSMessage *)reader->u.llrpReader.unhandledAsyncResponse.lMsg);
        reader->u.llrpReader.isResponsePending = false;
        TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);

        return TMR_SUCCESS;
      }
    }
    else
    {
      TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
      return TMR_ERROR_LLRP;
    }
  }
  else
  {
    ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
    /**
     * Done with the command, free the message
     * and check for message status
     **/ 
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }

    /**
     * Check response message status
     **/
    pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
    if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
    {
      TMR_LLRP_freeMessage(pRspMsg);
      return TMR_ERROR_LLRP; 
    }

    /**
     * Done with the response, free the message
     **/
    TMR_LLRP_freeMessage(pRspMsg);
  }

  return ret;
}


/**
 * Command to set Read Transmit power list
 *
 * @param reader Reader pointer
 * @param pPortValueList[in] Pointer to TMR_PortValueList
 */
TMR_Status
TMR_LLRP_cmdSetReadTransmitPowerList(TMR_Reader *reader, TMR_PortValueList *pPortValueList)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE     *pRsp;
  uint8_t                               i;

  ret = TMR_SUCCESS;
  i = 0;
  /**
   * Port Read Power list can be set
   * through  LLRP standard parameter SET_READER_CONFIG.AntennaConfiguration.RFTransmitter.TransmitPower
   * Initialize SET_READER_CONFIG message 
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Set antenna configuration as per the list supplied */
  for (i = 0; i < pPortValueList->len; i ++)
  {
    LLRP_tSAntennaConfiguration   *pAntConfig;
    LLRP_tSRFTransmitter          *pRfTransmitter;
    uint16_t                       index, power = 0;

    /* Construct RFTransmitter */
    pRfTransmitter = LLRP_RFTransmitter_construct();
    if (NULL == pRfTransmitter)
    {
      TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
      return TMR_ERROR_LLRP;
    }

    /* Get the index of corresponding power from powerTable  */
    for (index = 1; index <= reader->u.llrpReader.capabilities.powerTable.len; 
                                                                  index ++)
    {
      power = reader->u.llrpReader.capabilities.powerTable.list[index];
      if (pPortValueList->list[i].value == power)
      {
        break;
      }
      else
      {
        /*
         * we are cacheing the power level in 0.2db resolution.
         * if the value provided by the user is within the range
         * but not in the powertable always down grade the value
         * to the nearest available lower power value
         */
        if((pPortValueList->list[i].value > power)&&
            (pPortValueList->list[i].value < reader->u.llrpReader.capabilities.powerTable.list[index+1]))
        {
          power = reader->u.llrpReader.capabilities.powerTable.list[index];
          break;
        }
      }
    }

    if (index > reader->u.llrpReader.capabilities.powerTable.len)
    {
      /* power value specified is unknown or out of range */
      TMR_LLRP_freeMessage((LLRP_tSMessage *)pRfTransmitter);
      TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
      return TMR_ERROR_ILLEGAL_VALUE;
    }

    if ((TMR_LLRP_MODEL_ASTRA_EX == reader->u.llrpReader.capabilities.model)
        && (1 == pPortValueList->list[i].port)
        && (TMR_REGION_NA == reader->u.llrpReader.regionId))
    {
      if (3000 < power)
      {
        TMR_LLRP_freeMessage((LLRP_tSMessage *)pRfTransmitter);
        TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
        return TMR_ERROR_MSG_POWER_TOO_HIGH;
      }
    }

    LLRP_RFTransmitter_setTransmitPower(pRfTransmitter, index);

    /* Construct AntennaConfiguration */
    pAntConfig = LLRP_AntennaConfiguration_construct();
    LLRP_AntennaConfiguration_setAntennaID(pAntConfig, pPortValueList->list[i].port);
    LLRP_AntennaConfiguration_setRFTransmitter(pAntConfig, pRfTransmitter);

    /* Add AntennaConfiguration */
    LLRP_SET_READER_CONFIG_addAntennaConfiguration(pCmd, pAntConfig);
  }

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to get Selected protocols
 *
 * @param reader Reader pointer
 * @param features  Pointer to TMR_TagProtocolList to hold the Selected Protoocls value
 */
TMR_Status
TMR_LLRP_cmdGetSelectedProtocols(TMR_Reader *reader, TMR_TagProtocolList *protocolList)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration   *pTMConfig;
  LLRP_tSParameter                      *pCustParam;
  //LLRP_tSThingMagicSelectedProtocols    *pTMSelectedProtocols;
  LLRP_tSSelectedProtocol               *pSelectedProtocol;
  uint8_t i;
  
  ret = TMR_SUCCESS;

  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);
  
  /**
   * "/reader/selectedProtocols" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMConfig = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMConfig)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }
  
  /**
   * Set the requested data (i.e., Selected Protocols)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMConfig, 
      LLRP_ThingMagicControlConfiguration_ThingMagicSelectedProtocols);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMConfig->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract selected protocols from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    protocolList->len = 0;
    for (pSelectedProtocol = LLRP_ThingMagicSelectedProtocols_beginSelectedProtocol(
          (LLRP_tSThingMagicSelectedProtocols *) pCustParam), i = 0;
        (NULL != pSelectedProtocol);
        pSelectedProtocol = LLRP_ThingMagicSelectedProtocols_nextSelectedProtocol(
          pSelectedProtocol), i ++)
    {
      protocolList->list[i] = LLRP_SelectedProtocol_getProtocol(pSelectedProtocol);
      protocolList->len ++;
    }
  }
  
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}
/**
 * Command to get Licensed features
 *
 * @param reader Reader pointer
 * @param features  Pointer to TMR_uint8List to hold the Licensed features value
 */
TMR_Status
TMR_LLRP_cmdGetLicensedFeatures(TMR_Reader *reader, TMR_uint8List *features)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration  *pTMLicensedFeatures;
  LLRP_tSParameter                      *pCustParam;
  llrp_u8v_t                            licensedFeatures;

  ret = TMR_SUCCESS;

  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/licensedFeatures" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMLicensedFeatures = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMLicensedFeatures)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }
  
  
  /**
   * Set the requested data (i.e., Licensed Features)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMLicensedFeatures, LLRP_ThingMagicControlConfiguration_ThingMagicLicensedFeatures);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMLicensedFeatures->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMLicensedFeatures);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }
  
  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }
  /**
   * Response is success, extract licensed features from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    licensedFeatures = LLRP_ThingMagicLicensedFeatures_getLicensedFeatures((LLRP_tSThingMagicLicensedFeatures *) pCustParam);
  }
  else
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  memcpy(features->list, licensedFeatures.pValue, licensedFeatures.nValue);
  features->len = licensedFeatures.nValue;

  TMR_LLRP_freeMessage(pRspMsg);

  return ret;

}
/**
 * Command to get Version Serial
 *
 * @param reader Reader pointer
 * @param version  Pointer to TMR_String to hold the version serial value
 */
TMR_Status
TMR_LLRP_cmdGetVersionSerial(TMR_Reader *reader, TMR_String *version)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;

  ret = TMR_SUCCESS;

  /**
   * Version Serial  can be retrieved
   * through  LLRP standard parameter GET_READER_CONFIG_IDENTIFICATION_READERID
   * Initialize GET_READER_CONFIG message 
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }
  /**
   * Response is success, extract reader ID from it
   **/
  TMR_bytesToHex(pRsp->pIdentification->ReaderID.pValue, pRsp->pIdentification->ReaderID.nValue, version->value);
  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;

}

/**
 * Command to get GPI state
 *
 * @param reader Reader pointer
 * @param count Pointer to uint8_t to hold number of elements written into state
 * @param state Pointer to TMR_GpioPin array to hold the GPI state value
   */
TMR_Status
TMR_LLRP_cmdGetGPIState(TMR_Reader *reader, uint8_t *count, TMR_GpioPin state[])
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg = NULL;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;

  *count = 0;
  ret = TMR_SUCCESS;

  /**
   * GPI state can be retrieved
   * through  LLRP standard parameter GET_READER_CONFIG_RESPONSE GPIPortCurrentState
   * Initialize GET_READER_CONFIG message 
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_GPIPortCurrentState);

  /**
   * Now the message is framed completely and send the message
   **/
  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  if (reader->continuousReading)
  {
    ret = TMR_LLRP_sendMessage(reader, pCmdMsg, reader->u.llrpReader.transportTimeout);
    if (TMR_SUCCESS != ret)
    {
      TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
      return ret;
    }

    while (false == reader->u.llrpReader.isResponsePending)
    {
      tmr_sleep(10);
    }

    if (NULL != reader->u.llrpReader.unhandledAsyncResponse.lMsg)
    {
      if (pCmdMsg->elementHdr.pType->pResponseType == reader->u.llrpReader.unhandledAsyncResponse.lMsg->elementHdr.pType)
      {
        pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *)reader->u.llrpReader.unhandledAsyncResponse.lMsg;
        if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
        {
          TMR_LLRP_freeMessage((LLRP_tSMessage *)reader->u.llrpReader.unhandledAsyncResponse.lMsg);
          reader->u.llrpReader.isResponsePending = false;
          TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
          return TMR_ERROR_LLRP; 
        }
        /**
         * Response is success, extract GPI state from it
         **/
        {
          LLRP_tSGPIPortCurrentState* list = pRsp->listGPIPortCurrentState;
          while (NULL != list)
          {
            TMR_GpioPin* pin = &state[*count];
            ret = TMR_LLRP_llrpToTmGpi(reader->u.llrpReader.capabilities.model, list->GPIPortNum, &pin->id);
            if (TMR_SUCCESS != ret) { return ret; }
            pin->high = (LLRP_GPIPortState_High == list->eState);
            pin->output = false;
            (*count)++;
            list = (LLRP_tSGPIPortCurrentState*)list->hdr.pNextSubParameter;
          }
        }

        TMR_LLRP_freeMessage((LLRP_tSMessage *)reader->u.llrpReader.unhandledAsyncResponse.lMsg);
        reader->u.llrpReader.isResponsePending = false;
        TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);

        return TMR_SUCCESS;
      }
    }    
    else
    {
      TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
      return TMR_ERROR_LLRP;
    }
  }
  else
  {

    ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
    /**
     * Done with the command, free the message
     * and check for message status
     **/ 
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }

    /**
     * Check response message status
     **/
    pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
    if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
    {
      TMR_LLRP_freeMessage(pRspMsg);
      return TMR_ERROR_LLRP; 
    }
    /**
     * Response is success, extract GPI state from it
     **/
    {
      LLRP_tSGPIPortCurrentState* list = pRsp->listGPIPortCurrentState;
      while (NULL != list)
      {
        TMR_GpioPin* pin = &state[*count];
        ret = TMR_LLRP_llrpToTmGpi(reader->u.llrpReader.capabilities.model, list->GPIPortNum, &pin->id);
        if (TMR_SUCCESS != ret) { return ret; }
        pin->high = (LLRP_GPIPortState_High == list->eState);
        pin->output = false;
        (*count)++;
        list = (LLRP_tSGPIPortCurrentState*)list->hdr.pNextSubParameter;
      }
    }

    /**
     * Done with the response, free the message
     **/
    TMR_LLRP_freeMessage(pRspMsg);
  }
  return ret;

}

/**
 * Command to get Read transmit power list
 *
 * @param reader Reader pointer
 * @param pPortValueList[out] Pointer to TMR_PortValueList to hold the power value list
 */
TMR_Status 
TMR_LLRP_cmdGetReadTransmitPowerList(TMR_Reader *reader, TMR_PortValueList *pPortValueList)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSAntennaConfiguration           *pAntConfig;
  uint8_t                               i;

  ret = TMR_SUCCESS;
  i = 0;
  /**
   * Port Power list can be retrieved
   * through  LLRP standard parameter GET_READER_CONFIG_RESPONSE.AntennaConfiguration.RFTransmitter.TransmitPower
   * Initialize GET_READER_CONFIG message 
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_AntennaConfiguration);

  /* Get antenna configuration for all antennas*/
  LLRP_GET_READER_CONFIG_setAntennaID(pCmd, 0);

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract read power from it
   **/
  pPortValueList->len = 0;
  for (pAntConfig = LLRP_GET_READER_CONFIG_RESPONSE_beginAntennaConfiguration(pRsp),
          i = 0;
          (pAntConfig != NULL);
          pAntConfig = LLRP_GET_READER_CONFIG_RESPONSE_nextAntennaConfiguration(pAntConfig),
          i ++)
  {
    uint16_t index;

    if (i > pPortValueList->max)
    {
      break;
    }
    pPortValueList->list[i].port  = (uint8_t)pAntConfig->AntennaID;
    index = pAntConfig->pRFTransmitter->TransmitPower;
    if (NULL != reader->u.llrpReader.capabilities.powerTable.list)
    {
    pPortValueList->list[i].value = (int32_t)reader->u.llrpReader.capabilities.powerTable.list[index];
    pPortValueList->len ++;
  }
  }
  
  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to set write transmit power list
 *
 * @param reader Reader pointer
 * @param pPortValueList[in] Pointer to TMR_PortValueList
 */
TMR_Status
TMR_LLRP_cmdSetWriteTransmitPowerList(TMR_Reader *reader, TMR_PortValueList *pPortValueList)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE     *pRsp;
  uint8_t                               i;

  ret = TMR_SUCCESS;
  i = 0;
  /**
   * Port Write Power list is custom parameter and can be set
   * through  parameter SET_READER_CONFIG.AntennaConfiguration.RFTransmitter.TransmitPower
   * Initialize SET_READER_CONFIG message 
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Set antenna configuration as per the list supplied */
  for (i = 0; i < pPortValueList->len; i ++)
  {
    LLRP_tSThingMagicAntennaConfiguration   *pAntConfig;
    LLRP_tSWriteTransmitPower               *pWriteTransmitPower;
    uint16_t                                index, power = 0;

    /* Construct AntennaConfiguration */
    pAntConfig = LLRP_ThingMagicAntennaConfiguration_construct();
    if (NULL == pAntConfig)
    {
      TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
      return TMR_ERROR_LLRP;
    }
    LLRP_ThingMagicAntennaConfiguration_setAntennaID(pAntConfig, pPortValueList->list[i].port);
    pWriteTransmitPower = LLRP_WriteTransmitPower_construct();
    if (NULL == pWriteTransmitPower)
    {
      TMR_LLRP_freeMessage((LLRP_tSMessage *)pAntConfig);
      TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
      return TMR_ERROR_LLRP;
    }
 
    /* Get the index of corresponding power from powerTable  */
    for (index = 1; index <= reader->u.llrpReader.capabilities.powerTable.len; 
                                                                  index ++)
    {
      power = reader->u.llrpReader.capabilities.powerTable.list[index];
      if (pPortValueList->list[i].value == power)
      {
        break;
      }
      else
      {
        /*
         * we are cacheing the power level in 0.2db resolution.
         * if the value provided by the user is within the range
         * but not in the powertable always down grade the value
         * to the nearest available lower power value
         */
        if((pPortValueList->list[i].value > power)&&
            (pPortValueList->list[i].value < reader->u.llrpReader.capabilities.powerTable.list[index+1]))
        {
          power = reader->u.llrpReader.capabilities.powerTable.list[index];
          break;
        }
      }
    }

    if (index > reader->u.llrpReader.capabilities.powerTable.len)
    {
      /* power value specified is unknown or out of range */
      TMR_LLRP_freeMessage((LLRP_tSMessage *)pWriteTransmitPower);
      TMR_LLRP_freeMessage((LLRP_tSMessage *)pAntConfig);
      TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
      return TMR_ERROR_ILLEGAL_VALUE;
    }

    if ((TMR_LLRP_MODEL_ASTRA_EX == reader->u.llrpReader.capabilities.model)
        && (1 == pPortValueList->list[i].port)
        && (TMR_REGION_NA == reader->u.llrpReader.regionId))
    {
      if (3000 < power)
      {
        TMR_LLRP_freeMessage((LLRP_tSMessage *)pWriteTransmitPower);
        TMR_LLRP_freeMessage((LLRP_tSMessage *)pAntConfig);
        TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
        return TMR_ERROR_MSG_POWER_TOO_HIGH;
      }
    }

    LLRP_WriteTransmitPower_setWriteTransmitPower(pWriteTransmitPower, index);
    LLRP_ThingMagicAntennaConfiguration_setWriteTransmitPower(pAntConfig, pWriteTransmitPower);

    /* Add AntennaConfiguration as a custom parameter*/
    LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pAntConfig);
  }

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to get write transmit power list
 *
 * @param reader Reader pointer
 * @param pPortValueList[out] Pointer to TMR_PortValueList to hold the power list
 */
TMR_Status
TMR_LLRP_cmdGetWriteTransmitPowerList(TMR_Reader *reader, TMR_PortValueList *pPortValueList)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG                      *pCmd;
  LLRP_tSMessage                                *pCmdMsg;
  LLRP_tSMessage                                *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE             *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration   *pTMConfig;
  LLRP_tSWriteTransmitPower                     *pWriteTransmitPower;
  LLRP_tSParameter                              *pCustParam;
  uint8_t                                       i;

  ret = TMR_SUCCESS;
  i = 0;

  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/radio/portWritePowerList" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.ThingMagicAntennaConfiguration
   * Initialize the custom parameter
   **/
  pTMConfig = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMConfig)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., Antenna configuration)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMConfig, LLRP_ThingMagicControlConfiguration_ThingMagicAntennaConfiguration);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMConfig->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract port write power list from response
   **/
  pPortValueList->len = 0;
  for (pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp),
          i = 0;
          (pCustParam != NULL);
          pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_nextCustom(pCustParam),
          i ++)
  {
    uint16_t index;

    if (i > pPortValueList->max)
    {
      break;
    }
    pPortValueList->list[i].port
      = (uint8_t)((LLRP_tSThingMagicAntennaConfiguration *) pCustParam)->AntennaID;
    pWriteTransmitPower =
      LLRP_ThingMagicAntennaConfiguration_getWriteTransmitPower((LLRP_tSThingMagicAntennaConfiguration *) pCustParam);
    index = LLRP_WriteTransmitPower_getWriteTransmitPower(pWriteTransmitPower);
    if (NULL != reader->u.llrpReader.capabilities.powerTable.list)
    {
    pPortValueList->list[i].value = (int32_t)reader->u.llrpReader.capabilities.powerTable.list[index];
    pPortValueList->len ++;
  }
  }
 
  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to delete all ROSpecs on LLRP Reader
 *
 * @param reader Reader pointer
 */
TMR_Status
TMR_LLRP_cmdDeleteAllROSpecs(TMR_Reader *reader, bool receiveResponse)
{
  TMR_Status ret;
  LLRP_tSDELETE_ROSPEC          *pCmd;
  LLRP_tSMessage                *pCmdMsg;
  LLRP_tSMessage                *pRspMsg;
  LLRP_tSDELETE_ROSPEC_RESPONSE *pRsp;

  ret = TMR_SUCCESS;

  /**
   * Create delete rospec message
   **/
  pCmd = LLRP_DELETE_ROSPEC_construct();
  LLRP_DELETE_ROSPEC_setROSpecID(pCmd, 0);        /* All */

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  if (false == receiveResponse)
  {
    ret = TMR_LLRP_sendMessage(reader, pCmdMsg, reader->u.llrpReader.transportTimeout);
    /*
     * done with the command, free it
     */
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  }
  else
  {
    ret = TMR_LLRP_sendTimeout(reader, pCmdMsg, &pRspMsg,
                               TMR_LLRP_STOP_TIMEOUT
                               + reader->u.llrpReader.commandTimeout
                               + reader->u.llrpReader.transportTimeout);
    /**
     * Done with the command, free the message
     * and check for message status
     **/ 
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSDELETE_ROSPEC_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, Done with the response message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  }

  return ret;
}

uint8_t
TMR_LLRP_mapStateAwareTarget(uint8_t target)
{
  uint8_t stateAwareTarget;
  switch(target)
  {
    case SELECT:
      stateAwareTarget = LLRP_C1G2StateAwareTarget_SL;
      break;
    case INVENTORIED_S0:
      stateAwareTarget = LLRP_C1G2StateAwareTarget_Inventoried_State_For_Session_S0;
      break;
    case INVENTORIED_S1:
      stateAwareTarget = LLRP_C1G2StateAwareTarget_Inventoried_State_For_Session_S1;
      break;
    case INVENTORIED_S2:
      stateAwareTarget = LLRP_C1G2StateAwareTarget_Inventoried_State_For_Session_S2;
      break;
    case INVENTORIED_S3:
      stateAwareTarget = LLRP_C1G2StateAwareTarget_Inventoried_State_For_Session_S3;
      break;
    default:
      stateAwareTarget = target;
      break;
    }
    return stateAwareTarget;
}

/**
 * Convert API specific TMR_TagFilter into
 * LLRP specific filter element.
 **/
static TMR_Status
TMR_LLRP_prepareTagFilter(LLRP_tSAntennaConfiguration **pAntConfig, LLRP_tSC1G2InventoryCommand **pInvCommand,
	                 TMR_TagProtocol protocol, const TMR_TagFilter *filter, uint8_t antID, uint8_t count)
{
  TMR_Status ret;
  uint16_t i;
  ret = TMR_SUCCESS;

  /* Add C1G2Filter */
  if (TMR_TAG_PROTOCOL_GEN2 == protocol)
  {
    if ((TMR_FILTER_TYPE_MULTI == filter->type) && (isMultiFilterEnabled == true))
    {
      /* Set TagInventoryStateAwareAction to true */
      pInvCommand[count]->TagInventoryStateAware = true;

      for (i=0; i<filter->u.multiFilterList.len; i++)
      {
        /**
         * If Gen2 protocol select filter
         **/
        if (filter->u.multiFilterList.tagFilterList[i]->type == TMR_FILTER_TYPE_GEN2_SELECT)
        {
          LLRP_tSC1G2Filter             *pFilter;
          LLRP_tSC1G2TagInventoryMask   *pMask;
          LLRP_tSC1G2TagInventoryStateAwareFilterAction
                                        *pAwareAction;
          uint8_t                        stateAwareTarget;
          const TMR_GEN2_Select *fp;
          llrp_u1v_t  tmpMask;

          /* Initialize */
          pFilter        = LLRP_C1G2Filter_construct();
          pMask          = LLRP_C1G2TagInventoryMask_construct();
          pAwareAction = LLRP_C1G2TagInventoryStateAwareFilterAction_construct();

          /* Set antenna id to antConfig */
	  LLRP_AntennaConfiguration_setAntennaID(pAntConfig[count], antID);
          fp = &filter->u.multiFilterList.tagFilterList[i]->u.gen2Select;

          /**
           * Initialize Mask 
           **/
          /* Set Memory bank */
          if ((fp->bank == TMR_GEN2_BANK_RESERVED) || (fp->bank == TMR_GEN2_BANK_EPC) ||
              (fp->bank == TMR_GEN2_BANK_TID) || (fp->bank == TMR_GEN2_BANK_USER))
          {
            LLRP_C1G2TagInventoryMask_setMB(pMask, fp->bank);
          }
          else
          {
            return TMR_ERROR_INVALID; 
          }

          /* Set tag mask */
          tmpMask = LLRP_u1v_construct(fp->maskBitLength);
          memcpy(tmpMask.pValue, fp->mask, fp->maskBitLength / 8);
          LLRP_C1G2TagInventoryMask_setTagMask(pMask, tmpMask);

          /* Set bit Pointer */
          LLRP_C1G2TagInventoryMask_setPointer(pMask, (llrp_u16_t)fp->bitPointer);

          /* Set TagInventory StateAware Action */
          LLRP_C1G2TagInventoryStateAwareFilterAction_setAction(pAwareAction,
                                      fp->action);
          if(isStateAwareTargetMapped)
          {
            stateAwareTarget = TMR_LLRP_mapStateAwareTarget(fp->target);
          }
          else
          {
            stateAwareTarget = fp->target;
          }
          LLRP_C1G2TagInventoryStateAwareFilterAction_setTarget(pAwareAction,
                                      stateAwareTarget);

#if 0
          if(fp->invert)
          {
            /* If invert is set true, Unselect matched and select unmatched */
            LLRP_C1G2TagInventoryStateUnawareFilterAction_setAction(pUnawareAction, 
                                    LLRP_C1G2StateUnawareAction_Unselect_Select);
          }
#endif
          /* Set mask to filter */
          LLRP_C1G2Filter_setC1G2TagInventoryMask(pFilter, pMask);

          /* Set InventoryStateAwareFilter Action to filter */
          LLRP_C1G2Filter_setC1G2TagInventoryStateAwareFilterAction(pFilter, pAwareAction);

          /* Add filter to InventoryCommand */
	  LLRP_C1G2InventoryCommand_addC1G2Filter(pInvCommand[count], pFilter);

        }
      }
    }
    else if ((TMR_FILTER_TYPE_MULTI == filter->type) && (isMultiFilterEnabled == false))
    {
      return TMR_ERROR_UNSUPPORTED; 
    }
    else
    {
      LLRP_tSC1G2Filter             *pFilter;
      LLRP_tSC1G2TagInventoryMask   *pMask;

      /* Initialize */
      pFilter        = LLRP_C1G2Filter_construct();
      pMask          = LLRP_C1G2TagInventoryMask_construct();

      /* Set antenna id to antConfig */
      LLRP_AntennaConfiguration_setAntennaID(pAntConfig[count], antID);


      /**
       * If Gen2 protocol select filter
       **/
      if (TMR_FILTER_TYPE_GEN2_SELECT == filter->type)
      {
        const TMR_GEN2_Select *fp;
        llrp_u1v_t  tmpMask;
        uint8_t     stateAwareTarget;


        fp = &filter->u.gen2Select;
        if (isMultiFilterEnabled)
        {
          LLRP_tSC1G2TagInventoryStateAwareFilterAction
                                        *pAwareAction;
          /* Set TagInventoryStateAwareAction to true */
	  pInvCommand[count]->TagInventoryStateAware = true;
          pAwareAction = LLRP_C1G2TagInventoryStateAwareFilterAction_construct();
          /* Set TagInventory StateAware Action */
          LLRP_C1G2TagInventoryStateAwareFilterAction_setAction(pAwareAction, fp->action);
          if(isStateAwareTargetMapped)
          {
            stateAwareTarget = TMR_LLRP_mapStateAwareTarget(fp->target);
          }
          else
          {
            stateAwareTarget = fp->target;
          }
          LLRP_C1G2TagInventoryStateAwareFilterAction_setTarget(pAwareAction, stateAwareTarget);
          /* Set InventoryStateAwareFilter Action to filter */
          LLRP_C1G2Filter_setC1G2TagInventoryStateAwareFilterAction(pFilter, pAwareAction);
        }
        else
        {
          LLRP_tSC1G2TagInventoryStateUnawareFilterAction
                                        *pUnawareAction;
          /* Set TagInventoryStateUnawareAction to true */
	  pInvCommand[count]->TagInventoryStateAware = false;
          pUnawareAction = LLRP_C1G2TagInventoryStateUnawareFilterAction_construct();
          if(fp->invert)
          {
            /* If invert is set true, Unselect matched and select unmatched */
            LLRP_C1G2TagInventoryStateUnawareFilterAction_setAction(pUnawareAction, 
                                    LLRP_C1G2StateUnawareAction_Unselect_Select);
          }
          else
          {
            /* Set TagInventory StateUnaware Action */
            LLRP_C1G2TagInventoryStateUnawareFilterAction_setAction(pUnawareAction,
                                    LLRP_C1G2StateUnawareAction_Select_Unselect);
          }
          /* Set InventoryStateUnawareFilter Action to filter */
          LLRP_C1G2Filter_setC1G2TagInventoryStateUnawareFilterAction(pFilter, pUnawareAction);
        }

        /**
         * Initialize Mask 
         **/
        /* Set Memory bank */
        if ((fp->bank == TMR_GEN2_BANK_RESERVED) || (fp->bank == TMR_GEN2_BANK_EPC) ||
            (fp->bank == TMR_GEN2_BANK_TID) || (fp->bank == TMR_GEN2_BANK_USER))
        {
          LLRP_C1G2TagInventoryMask_setMB(pMask, fp->bank);
        }
        else
        {
          return TMR_ERROR_INVALID; 
        }

        /* Set tag mask */
        tmpMask = LLRP_u1v_construct(fp->maskBitLength);
        memcpy(tmpMask.pValue, fp->mask, fp->maskBitLength / 8);
        LLRP_C1G2TagInventoryMask_setTagMask(pMask, tmpMask);

        /* Set bit Pointer */
        LLRP_C1G2TagInventoryMask_setPointer(pMask, (llrp_u16_t)fp->bitPointer);
      }
      /**
       * If Gen2 tag data filter
       **/
      else if (TMR_FILTER_TYPE_TAG_DATA == filter->type)
      {
        const TMR_TagData *fp;
        llrp_u1v_t  tmpMask;
        LLRP_tSC1G2TagInventoryStateUnawareFilterAction
                                      *pUnawareAction;

        /* Set TagInventoryStateUnawareAction to false */
        pInvCommand[count]->TagInventoryStateAware = false;

        pUnawareAction = LLRP_C1G2TagInventoryStateUnawareFilterAction_construct();
        fp = &filter->u.tagData;

        /**
         * Initialize Mask 
         **/
        /* Set Memory bank to EPC always, in case of TagData */
        LLRP_C1G2TagInventoryMask_setMB(pMask, TMR_GEN2_BANK_EPC);

        /* Set tag mask */
        tmpMask = LLRP_u1v_construct(fp->epcByteCount * 8);
        memcpy(tmpMask.pValue, fp->epc, fp->epcByteCount);
        LLRP_C1G2TagInventoryMask_setTagMask(pMask, tmpMask);

        /**
         * Set bit Pointer
         * (EPC always starts from bit 32)
         **/
        LLRP_C1G2TagInventoryMask_setPointer(pMask, 32);

        /* Set TagInventory StateUnaware Action */
        LLRP_C1G2TagInventoryStateUnawareFilterAction_setAction(pUnawareAction,
                                LLRP_C1G2StateUnawareAction_Select_Unselect);
        /* Set InventoryStateUnawareFilter Action to filter */
        LLRP_C1G2Filter_setC1G2TagInventoryStateUnawareFilterAction(pFilter, pUnawareAction);
      }
      /**
       * Else, Unknown filter type. Return error
       **/
      else
      {
        return TMR_ERROR_INVALID;
      }

      /* Set mask to filter */
      LLRP_C1G2Filter_setC1G2TagInventoryMask(pFilter, pMask);

      /* Add filter to InventoryCommand */
      LLRP_C1G2InventoryCommand_addC1G2Filter(pInvCommand[count], pFilter);

    }
  }
  else
  {
    if (TMR_TAG_PROTOCOL_ISO180006B == protocol)
    {

#ifdef TMR_ENABLE_ISO180006B

      LLRP_tSThingMagicISO180006BInventoryCommand   *pTMISOInventory;
      LLRP_tSThingMagicISO180006BTagPattern         *pTMISOTagPattern;

      /* initialize  the TM inventory commmand */
      pTMISOInventory = LLRP_ThingMagicISO180006BInventoryCommand_construct();

      /* initialize  the  TM tagpattern */
      pTMISOTagPattern = LLRP_ThingMagicISO180006BTagPattern_construct();

    /**
       * If ISO180006B select filter
     **/
      if (TMR_FILTER_TYPE_ISO180006B_SELECT == filter->type)
      {
        const TMR_ISO180006B_Select *fp;
        llrp_u2_t   selectOp;
        llrp_u8v_t  tmpMask;

        fp = &filter->u.iso180006bSelect;

        /* Initialize the filter type */
        LLRP_ThingMagicISO180006BTagPattern_setFilterType(pTMISOTagPattern,
            LLRP_ThingMagicISO180006BFilterType_ISO180006BSelect);

        /* Initialize the select tag op */
        selectOp = fp->op;
        if ((selectOp == TMR_ISO180006B_SELECT_OP_EQUALS) || (selectOp == TMR_ISO180006B_SELECT_OP_NOT_EQUALS) ||
            (selectOp == TMR_ISO180006B_SELECT_OP_LESS_THAN) || (selectOp == TMR_ISO180006B_SELECT_OP_GREATER_THAN))
        {
          LLRP_ThingMagicISO180006BTagPattern_setSelectOp(pTMISOTagPattern, selectOp);
        }
        else
        {
          return TMR_ERROR_INVALID;
        }

        /* set address of tag memory */
        LLRP_ThingMagicISO180006BTagPattern_setAddress(pTMISOTagPattern, fp->address);

        /* set the tag mask */
        LLRP_ThingMagicISO180006BTagPattern_setMask(pTMISOTagPattern, fp->mask);

        /* set the data to compare */
        /* In case of iso the data length must be eight */ 
        tmpMask = LLRP_u8v_construct((llrp_u16_t)(sizeof(fp->data))/(sizeof(fp->data[0])));
        memcpy(tmpMask.pValue, fp->data, tmpMask.nValue);
        LLRP_ThingMagicISO180006BTagPattern_setTagData(pTMISOTagPattern, tmpMask);

        /* set the selct action */
        LLRP_ThingMagicISO180006BTagPattern_setInvert(pTMISOTagPattern, (llrp_u1_t)fp->invert);

        /* Add TagPattern to the TM ISOInventory */
        LLRP_ThingMagicISO180006BInventoryCommand_setThingMagicISO180006BTagPattern(pTMISOInventory, pTMISOTagPattern);

        /* Add InvCommand to antenna configuration */
        LLRP_AntennaConfiguration_addAirProtocolInventoryCommandSettings(pAntConfig[0],
            (LLRP_tSParameter *)pTMISOInventory);

      }
      /**
       * If Iso tag filter
       **/
      else if(TMR_FILTER_TYPE_TAG_DATA == filter->type)
      {
        const TMR_TagData *fp;
        llrp_u8v_t  tmpMask;

        /* Initialize the filter type */
        LLRP_ThingMagicISO180006BTagPattern_setFilterType(pTMISOTagPattern,
            LLRP_ThingMagicISO180006BFilterType_ISO180006BTagData);

        fp = &filter->u.tagData;
        /* Initialize the select tag op */
        /* Set the Select op as TMR_ISO180006B_SELECT_OP_EQUALS 
         * in case of TagData
         */
        LLRP_ThingMagicISO180006BTagPattern_setSelectOp(pTMISOTagPattern, (llrp_u2_t) TMR_ISO180006B_SELECT_OP_EQUALS);
        /* Set the tag memory address to Zero , in case of TagData */
        LLRP_ThingMagicISO180006BTagPattern_setAddress(pTMISOTagPattern, (llrp_u8_t)0);
        /* Set the mask to 0xff, in case TagData */
        LLRP_ThingMagicISO180006BTagPattern_setMask(pTMISOTagPattern, (llrp_u8_t)0xFF);

        /* set the data to compare */
        /* In case of iso the data length must be eight */
        tmpMask = LLRP_u8v_construct((llrp_u16_t)fp->epcByteCount);
        memcpy(tmpMask.pValue, fp->epc, tmpMask.nValue);
        LLRP_ThingMagicISO180006BTagPattern_setTagData(pTMISOTagPattern, tmpMask);

        /* Set the select action false, in case of TagData */
        LLRP_ThingMagicISO180006BTagPattern_setInvert(pTMISOTagPattern, (llrp_u1_t)false);

        /* Add TagPattern to the TM ISOInventory */
        LLRP_ThingMagicISO180006BInventoryCommand_setThingMagicISO180006BTagPattern(pTMISOInventory, pTMISOTagPattern);

        /* Add InvCommand to antenna configuration */
        LLRP_AntennaConfiguration_addAirProtocolInventoryCommandSettings(pAntConfig[0],
            (LLRP_tSParameter *)pTMISOInventory);

      }
      else
      {
    return TMR_ERROR_INVALID;
  }
#endif /* TMR_ENABLE_ISO180006B */
    }
    else
    {
      /* Unsuppoerd Protocol */
      return TMR_ERROR_INVALID;
    }
  }

  return ret;
}

/** 
 * Add Custom Antenna Configuration per individual antenna (session, target and filter), FastSearch option
 * to the read
 **/
TMR_Status
TMR_LLRP_addCustomAntennaConfigPerAntenna(TMR_CustomAntConfig *customAntConfig, LLRP_tSAntennaConfiguration **pAntConfig, 
		                         LLRP_tSC1G2InventoryCommand **pInvCommand, bool fastSearch)
{
  TMR_Status ret;
  LLRP_tSThingMagicFastSearchMode *pFastSearch;
  LLRP_tSC1G2SingulationControl   *pSingulationControl;
  LLRP_tSThingMagicTargetStrategy *pTarget;
  uint8_t i = 0;

  ret = TMR_SUCCESS;
  if(NULL != customAntConfig)
  {
  for(i=0; i < (customAntConfig->antennaCount); i++)
  {
    /* Initialize SingulationControl */
    pSingulationControl = LLRP_C1G2SingulationControl_construct();
    pTarget = LLRP_ThingMagicTargetStrategy_construct();
    LLRP_AntennaConfiguration_setAntennaID(pAntConfig[i], customAntConfig->customAntConfigPerAntenna[i]->antID);
    if(customAntConfig->customAntConfigPerAntenna[i]->session != NULL)
    {
    switch(*(customAntConfig->customAntConfigPerAntenna[i]->session))
    {
      case TMR_GEN2_SESSION_S0:
        {
          pSingulationControl->Session = 0;
          break;
        }
      case TMR_GEN2_SESSION_S1:
        {
          pSingulationControl->Session = 1;
          break;
        }
      case TMR_GEN2_SESSION_S2:
        {
          pSingulationControl->Session = 2;
          break;
        }
      case TMR_GEN2_SESSION_S3:
        {
          pSingulationControl->Session = 3;
           break;
        }
      default:
        {
          TMR_LLRP_freeMessage((LLRP_tSMessage *)pSingulationControl);
          return TMR_ERROR_INVALID;
        }
    }

    /* Set Singulation Control to inventoryCommand */
    LLRP_C1G2InventoryCommand_setC1G2SingulationControl(pInvCommand[i], pSingulationControl);
    }
    if(customAntConfig->customAntConfigPerAntenna[i]->target != NULL)
    {
    switch(*(customAntConfig->customAntConfigPerAntenna[i]->target))
    {
      case TMR_GEN2_TARGET_A:
        break;
      case TMR_GEN2_TARGET_B:
        break;
      case TMR_GEN2_TARGET_AB:
        break;
      case TMR_GEN2_TARGET_BA:
        break;
      default:
        TMR_LLRP_freeMessage((LLRP_tSMessage *)pTarget);
        return TMR_ERROR_INVALID;
    }

    LLRP_ThingMagicTargetStrategy_setThingMagicTargetStrategyValue(pTarget,(*(customAntConfig->customAntConfigPerAntenna[i]->target)));

    LLRP_C1G2InventoryCommand_addCustom (pInvCommand[i],
      (LLRP_tSParameter *)pTarget);
    }
    if(fastSearch)
    {
      pFastSearch = LLRP_ThingMagicFastSearchMode_construct();
      LLRP_ThingMagicFastSearchMode_setThingMagicFastSearch(pFastSearch,
      (LLRP_tEThingMagicFastSearchValue)fastSearch);

      /* Add FastSearch to InventoryCommand */
      LLRP_C1G2InventoryCommand_addCustom (pInvCommand[i],
      (LLRP_tSParameter *)pFastSearch); 
    }

  }
  }
  else
  {
      /* Set antenna id to antConfig (all antennas) */
      pAntConfig[0]->AntennaID = 0;
      pFastSearch = LLRP_ThingMagicFastSearchMode_construct();
      LLRP_ThingMagicFastSearchMode_setThingMagicFastSearch(pFastSearch,
      (LLRP_tEThingMagicFastSearchValue)fastSearch);

      /* Add FastSearch to InventoryCommand */
      LLRP_C1G2InventoryCommand_addCustom (pInvCommand[0],
      (LLRP_tSParameter *)pFastSearch); 

  }

  return ret;
}

/**
 * Command to Add an ROSpec
 *
 * @param reader Reader pointer
 * @param readDuration Duration Trigger (Duration of AISpec in milliseconds)
 * @param antennaList Pointer to TMR_uint8List containing the antennas on which to inventory
 * @param filter Pointer to Tag filter
 * @param protocol Protocol to be used
 */ 
TMR_Status
TMR_LLRP_cmdAddROSpec(TMR_Reader *reader, uint16_t readDuration, 
                                  TMR_uint8List *antennaList,
                                  TMR_TagFilter *filter,
                                  TMR_TagProtocol protocol)
{
  TMR_Status ret;
  LLRP_tSADD_ROSPEC               *pCmd;
  LLRP_tSMessage                  *pCmdMsg;
  LLRP_tSMessage                  *pRspMsg;
  LLRP_tSADD_ROSPEC_RESPONSE      *pRsp;

  LLRP_tSROSpec                   *pROSpec;
  LLRP_tSROSpecStartTrigger       *pROSpecStartTrigger;
  LLRP_tSROSpecStopTrigger        *pROSpecStopTrigger;
  LLRP_tSROBoundarySpec           *pROBoundarySpec;
  LLRP_tSAISpecStopTrigger        *pAISpecStopTrigger;
  LLRP_tSInventoryParameterSpec   *pInventoryParameterSpec;
  LLRP_tSAISpec                   *pAISpec;
  LLRP_tSTagReportContentSelector *pTagReportContentSelector;
  LLRP_tSROReportSpec             *pROReportSpec;
  LLRP_tSTagObservationTrigger    *pAISpecTagObservationTrigger;
  LLRP_tSRFSurveySpec             *pRFSurveySpec;
  LLRP_tSCustomRFSurveySpec       *pCustomRFSurveySpec;

  llrp_u16v_t                     AntennaIDs;
  int                             i;
  uint8_t                         planCount = 0;
  bool                            useFastSearch = false;
  uint8_t                         AISpecIterations = 1;
  uint32_t                        simpleReadPlanWeight = 0;
  uint32_t                        multiReadPlanWeight = 0;
  TMR_CustomAntConfig             *customAntConfig = NULL;
  LLRP_tSAntennaConfiguration     **pAntConfig = NULL;
  LLRP_tSC1G2InventoryCommand     **pInvCommand = NULL;
  uint8_t count = 0;
  ret = TMR_SUCCESS;

  /**
   * Initialize AddROSpec message
   **/
  pCmd = LLRP_ADD_ROSPEC_construct();
  /**
   *  Initialize ROSpec and
   *  1. Set rospec id
   *  2. Set priority
   *  3. Set current state
   *  4. Set ROBoundarySpec
   *     4.1 Set ROSpecStartTrigger
   *     4.2 Set ROSpecStopTrigger
   *  5. Set AISpec
   *     5.1 Set AISpecStopTrigger
   *     5.2 Set AntennaID
   *     5.3 Set InventoryParameterSpec
   *  6. Set ROReportSpec
   *  7. Set RFSurvaySpec
   **/

  /* Construct ROSpec message */
  pROSpec = LLRP_ROSpec_construct();
  {
    /* 1. Set ROSpec id */
    LLRP_ROSpec_setROSpecID(pROSpec, reader->u.llrpReader.roSpecId);

    /* 2. Set priority  */
    LLRP_ROSpec_setPriority(pROSpec, 0);

    /* 3. Set current State */
    LLRP_ROSpec_setCurrentState(pROSpec, LLRP_ROSpecState_Disabled);


    /** 
     * 4. Initialize and set ROBoundarySpec
     **/
    {
      pROBoundarySpec = LLRP_ROBoundarySpec_construct();

      /* Initialize ROSpec start trigger */
      pROSpecStartTrigger = LLRP_ROSpecStartTrigger_construct();

      if ((true == reader->continuousReading) &&
           (TMR_READ_PLAN_TYPE_MULTI == reader->readParams.readPlan->type) &&
           (false == isPerAntennaEnabled))
      {
#ifdef TMR_ENABLE_BACKGROUND_READS
        /**
         * When TMR_ENABLE_BACKGROUND_READS is not defined, control never
         * comes here.
         **/
        /**
         * In case of continuous reading and if multiple read plans exist,
         * then each simple read plan ROSpec's start trigger is set to
         * duration trigger with the duration set to total read duration.
         **/
        LLRP_tSPeriodicTriggerValue *pPeriod;

        LLRP_ROSpecStartTrigger_setROSpecStartTriggerType(pROSpecStartTrigger,
                                        LLRP_ROSpecStartTriggerType_Periodic);

        /**
         * Initialize Periodic trigger value.
         **/
        pPeriod = LLRP_PeriodicTriggerValue_construct();
        /**
         * Total read time is equal to asyncOnTime, which is sliced to
         * each simple read plan as per their wheitage. So the total
         * period after which each ROSpec has to re-run is asyncOnTime.
         **/
        pPeriod->Period = reader->readParams.asyncOnTime;
        LLRP_ROSpecStartTrigger_setPeriodicTriggerValue(pROSpecStartTrigger, pPeriod);
#endif
      }
      else
      { 
        /**
         * ROSpec start trigger is set to null in all cases when per antenna ontime support available.
         * Because sending complete multi read plan information in multiple AISpecs in single ROSPec
         * No need to periodically execute ROSpec incase of multi read async
         **/
        LLRP_ROSpecStartTrigger_setROSpecStartTriggerType(pROSpecStartTrigger, 
                                            LLRP_ROSpecStartTriggerType_Null);
      }

      /* Initialize ROSpec stop trigger */
      pROSpecStopTrigger = LLRP_ROSpecStopTrigger_construct();
      if ((true != reader->continuousReading) &&
           (TMR_READ_PLAN_TYPE_MULTI == reader->readParams.readPlan->type) &&
           (true == isPerAntennaEnabled))
      {
        /**
         * In case of non continuous reading and if multiple read plans exist,
         * then ROSpec's stop trigger is set to
         * duration trigger with the duration set to total read duration.
         **/
        LLRP_ROSpecStopTrigger_setROSpecStopTriggerType(pROSpecStopTrigger, LLRP_ROSpecStopTriggerType_Duration);
        LLRP_ROSpecStopTrigger_setDurationTriggerValue(pROSpecStopTrigger, readDuration);
      }
      else
      {
        /**
         * ROSpec stop trigger is set to null in all cases.
         **/
        LLRP_ROSpecStopTrigger_setROSpecStopTriggerType(pROSpecStopTrigger, LLRP_ROSpecStopTriggerType_Null);
      }

      /* Set start and stop triggers to BoundarySpec */
      LLRP_ROBoundarySpec_setROSpecStartTrigger(pROBoundarySpec, pROSpecStartTrigger);
      LLRP_ROBoundarySpec_setROSpecStopTrigger(pROBoundarySpec, pROSpecStopTrigger);

      /* Set ROBoundarySpec to ROSpec  */
      LLRP_ROSpec_setROBoundarySpec(pROSpec, pROBoundarySpec);
    }

    /**
     * 5. Initialize and set AISpec 
     **/
    if (isPerAntennaEnabled)
    {
      if (TMR_READ_PLAN_TYPE_SIMPLE == reader->readParams.readPlan->type)
      {
        AISpecIterations = 1;
      }
      else if (TMR_READ_PLAN_TYPE_MULTI == reader->readParams.readPlan->type)
      {
        AISpecIterations = reader->readParams.readPlan->u.multi.planCount;
      }
    }
    else
    {
        AISpecIterations = 1;
    }
    for (planCount = 0; planCount < AISpecIterations; planCount ++) 
    {
      pAISpec = LLRP_AISpec_construct();

      /* Initialize AISpec stop trigger  */
      pAISpecStopTrigger = LLRP_AISpecStopTrigger_construct();

      if (isPerAntennaEnabled)
      {
        if((true != reader->continuousReading) &&
            (TMR_READ_PLAN_TYPE_SIMPLE == reader->readParams.readPlan->type))
        {
          /**
           * In sync read and for 
           * AISpec stop trigger should be duration based.
           **/
          reader->isStopNTags = reader->readParams.readPlan->u.simple.stopOnCount.stopNTriggerStatus;
          reader->numberOfTagsToRead = reader->readParams.readPlan->u.simple.stopOnCount.noOfTags;
          if(reader->isStopNTags == true && reader->numberOfTagsToRead == 0)
          {
            return TMR_ERROR_INVALID;
          }
 
          if(reader->isStopNTags)
          {
            pAISpecTagObservationTrigger = LLRP_TagObservationTrigger_construct();
            LLRP_TagObservationTrigger_setTriggerType(pAISpecTagObservationTrigger, 
                       LLRP_TagObservationTriggerType_Upon_Seeing_N_Tags_Or_Timeout);
            LLRP_TagObservationTrigger_setNumberOfTags(pAISpecTagObservationTrigger, (llrp_u16_t)reader->numberOfTagsToRead);
            LLRP_TagObservationTrigger_setTimeout(pAISpecTagObservationTrigger, readDuration);
            LLRP_AISpecStopTrigger_setAISpecStopTriggerType(pAISpecStopTrigger, LLRP_AISpecStopTriggerType_Tag_Observation);
            LLRP_AISpecStopTrigger_setTagObservationTrigger(pAISpecStopTrigger, pAISpecTagObservationTrigger);

          }
          else
          {
            LLRP_AISpecStopTrigger_setAISpecStopTriggerType(pAISpecStopTrigger, LLRP_AISpecStopTriggerType_Duration);
            LLRP_AISpecStopTrigger_setDurationTrigger(pAISpecStopTrigger, readDuration);
          }
        }
        else
        {
          /**
           * In all other cases, i.e., for both sync and async read
           * AISpec stop trigger should be NULL.
           **/
          LLRP_AISpecStopTrigger_setAISpecStopTriggerType(pAISpecStopTrigger, LLRP_AISpecStopTriggerType_Null);
          LLRP_AISpecStopTrigger_setDurationTrigger(pAISpecStopTrigger, 0);
        }
      }
      else
      {
        if ((true == reader->continuousReading) &&
            (TMR_READ_PLAN_TYPE_SIMPLE == reader->readParams.readPlan->type))
        {
          /**
           * Only in case of continuous reading and if the readplan is a 
           * simple read plan, then set the duration trigger of AISpec
           * to NULL.
           **/
          LLRP_AISpecStopTrigger_setAISpecStopTriggerType(pAISpecStopTrigger, LLRP_AISpecStopTriggerType_Null);
          LLRP_AISpecStopTrigger_setDurationTrigger(pAISpecStopTrigger, 0);
        }
        else
        {
          /**
           * In all other cases, i.e., for both sync and async read
           * AISpec stop trigger should be duration based.
           **/
          if(reader->isStopNTags)
          {
            pAISpecTagObservationTrigger = LLRP_TagObservationTrigger_construct();
            LLRP_TagObservationTrigger_setTriggerType(pAISpecTagObservationTrigger, 
                       LLRP_TagObservationTriggerType_Upon_Seeing_N_Tags_Or_Timeout);
            LLRP_TagObservationTrigger_setNumberOfTags(pAISpecTagObservationTrigger, (llrp_u16_t)reader->numberOfTagsToRead);
            LLRP_TagObservationTrigger_setTimeout(pAISpecTagObservationTrigger, readDuration);
            LLRP_AISpecStopTrigger_setAISpecStopTriggerType(pAISpecStopTrigger, LLRP_AISpecStopTriggerType_Tag_Observation);
            LLRP_AISpecStopTrigger_setTagObservationTrigger(pAISpecStopTrigger, pAISpecTagObservationTrigger);

          }
          else
          {
            LLRP_AISpecStopTrigger_setAISpecStopTriggerType(pAISpecStopTrigger, LLRP_AISpecStopTriggerType_Duration);
            LLRP_AISpecStopTrigger_setDurationTrigger(pAISpecStopTrigger, readDuration);
          }
        }
      }

      /* Set AISpec stop trigger */
      LLRP_AISpec_setAISpecStopTrigger(pAISpec, pAISpecStopTrigger);

      /* set antenna ids to aispec */
      {
        AntennaIDs = LLRP_u16v_construct(TMR_SR_MAX_ANTENNA_PORTS);
        AntennaIDs.nValue = 0;

        if ((isPerAntennaEnabled) && (!isStandaloneTagop))
        {
          if (TMR_READ_PLAN_TYPE_SIMPLE == reader->readParams.readPlan->type)
          {
            antennaList = &(reader->readParams.readPlan->u.simple.antennas);
            protocol = reader->readParams.readPlan->u.simple.protocol;
            filter = reader->readParams.readPlan->u.simple.filter;
            useFastSearch = reader->readParams.readPlan->u.simple.useFastSearch; 
	    customAntConfig=reader->readParams.readPlan->u.simple.customAntConfig;
          }
          if (TMR_READ_PLAN_TYPE_MULTI == reader->readParams.readPlan->type)
          {
            antennaList = &(reader->readParams.readPlan->u.multi.plans[planCount]->u.simple.antennas);
            protocol = reader->readParams.readPlan->u.multi.plans[planCount]->u.simple.protocol;
          }
        }
        else
        {
          useFastSearch = reader->fastSearch;
        }
        if (NULL != antennaList->list)
        {
          /**
           * In case of user set readplan, use the antenna list
           * set in read plan
           **/
          if (TMR_SR_MAX_ANTENNA_PORTS < antennaList->len)
          {
            antennaList->len = TMR_SR_MAX_ANTENNA_PORTS;
          }

          for (i = 0; i < antennaList->len; i++)
          {
            AntennaIDs.pValue[i] = antennaList->list[i];
            AntennaIDs.nValue ++;
          }
        }
        else
        {
          /**
           * In case of default read plan, use "0" to read on
           * all antennas.
           **/
          AntennaIDs.pValue[0] = 0;
          AntennaIDs.nValue    = 1;
        }

        LLRP_AISpec_setAntennaIDs(pAISpec, AntennaIDs);
      }

      /**
       *  Initialize InventoryParameterSpec
       *  and set it to AISpec
       **/
      pInventoryParameterSpec = LLRP_InventoryParameterSpec_construct();

      /* Set protocol */
      if (TMR_TAG_PROTOCOL_GEN2 == protocol)
      {
        LLRP_InventoryParameterSpec_setProtocolID(pInventoryParameterSpec, LLRP_AirProtocols_EPCGlobalClass1Gen2);
      }
#ifdef TMR_ENABLE_ISO180006B
      else if (TMR_TAG_PROTOCOL_ISO180006B == protocol)
        {
          LLRP_tSThingMagicCustomAirProtocols *pInventoryParameterCustom;

		  /* For other protocol specify the Inventory Parameter protocol ID as unspecified */
          LLRP_InventoryParameterSpec_setProtocolID(pInventoryParameterSpec, LLRP_AirProtocols_Unspecified);
          pInventoryParameterCustom = LLRP_ThingMagicCustomAirProtocols_construct();
          LLRP_ThingMagicCustomAirProtocols_setcustomProtocolId(pInventoryParameterCustom,
              LLRP_ThingMagicCustomAirProtocolList_Iso180006b);

          /* add this as a custom parameter to InventoryParameterSpec */
          LLRP_InventoryParameterSpec_addCustom(pInventoryParameterSpec, (LLRP_tSParameter *)pInventoryParameterCustom);
        }
#endif /* TMR_ENABLE_ISO180006B */
      else if(TMR_TAG_PROTOCOL_ATA == protocol)
      {
        LLRP_tSThingMagicCustomAirProtocols *pInventoryParameterCustom;

        /* For other protocol specify the Inventory Parameter protocol ID as unspecified */
        LLRP_InventoryParameterSpec_setProtocolID(pInventoryParameterSpec, LLRP_AirProtocols_Unspecified);
        pInventoryParameterCustom = LLRP_ThingMagicCustomAirProtocols_construct();
        LLRP_ThingMagicCustomAirProtocols_setcustomProtocolId(pInventoryParameterCustom,
            LLRP_ThingMagicCustomAirProtocolList_Ata);

        /* add this as a custom parameter to InventoryParameterSpec */
        LLRP_InventoryParameterSpec_addCustom(pInventoryParameterSpec, (LLRP_tSParameter *)pInventoryParameterCustom);
      }
      else if(TMR_TAG_PROTOCOL_IPX64 == protocol)
      {
        LLRP_tSThingMagicCustomAirProtocols *pInventoryParameterCustom;

		/* For other protocol specify the Inventory Parameter protocol ID as unspecified */
        LLRP_InventoryParameterSpec_setProtocolID(pInventoryParameterSpec, LLRP_AirProtocols_Unspecified);
        pInventoryParameterCustom = LLRP_ThingMagicCustomAirProtocols_construct();
        LLRP_ThingMagicCustomAirProtocols_setcustomProtocolId(pInventoryParameterCustom,
            LLRP_ThingMagicCustomAirProtocolList_IPX64);

        /* add this as a custom parameter to InventoryParameterSpec */
        LLRP_InventoryParameterSpec_addCustom(pInventoryParameterSpec, (LLRP_tSParameter *)pInventoryParameterCustom);
      }
      else if(TMR_TAG_PROTOCOL_IPX256 == protocol)
      {
        LLRP_tSThingMagicCustomAirProtocols *pInventoryParameterCustom;

		/* For other protocol specify the Inventory Parameter protocol ID as unspecified */
        LLRP_InventoryParameterSpec_setProtocolID(pInventoryParameterSpec, LLRP_AirProtocols_Unspecified);
        pInventoryParameterCustom = LLRP_ThingMagicCustomAirProtocols_construct();
        LLRP_ThingMagicCustomAirProtocols_setcustomProtocolId(pInventoryParameterCustom,
            LLRP_ThingMagicCustomAirProtocolList_IPX256);

        /* add this as a custom parameter to InventoryParameterSpec */
        LLRP_InventoryParameterSpec_addCustom(pInventoryParameterSpec, (LLRP_tSParameter *)pInventoryParameterCustom);
      }
      else
      {
        return TMR_ERROR_UNIMPLEMENTED_FEATURE;
      }

      /* Set InventoryParameterSpec id */
      LLRP_InventoryParameterSpec_setInventoryParameterSpecID(pInventoryParameterSpec, (planCount+1));

      /**
       * Add filter to Inventory parameter
       **/
      {
        if(reader->u.llrpReader.featureFlags & TMMP_READER_FEATURES_FLAG_MULTI_SELECT)
        {
          isMultiFilterEnabled = true;
        }
        else
        {
          isMultiFilterEnabled = false;
        }
        if(reader->u.llrpReader.featureFlags & TMMP_READER_FEATURES_FLAG_STATEAWARE_TARGET_MAPPED)
        {
          isStateAwareTargetMapped = true;
        }
        else
        {
          isStateAwareTargetMapped = false;
        }
	if(NULL != customAntConfig)
	{
	      	pAntConfig     = malloc(customAntConfig->antennaCount*sizeof(struct LLRP_tSAntennaConfiguration *));
	      	pInvCommand    = malloc(customAntConfig->antennaCount*sizeof(struct LLRP_tSC1G2InventoryCommand *));

		for (count=0; count < customAntConfig->antennaCount; count++)
  		{
		      	filter=customAntConfig->customAntConfigPerAntenna[count]->filter;
			/* Initialize antenna configuration */
			pAntConfig[count]     = LLRP_AntennaConfiguration_construct();
                        pInvCommand[count]    = LLRP_C1G2InventoryCommand_construct();
	                if (NULL != filter)
			{
			  ret = TMR_LLRP_prepareTagFilter(pAntConfig, pInvCommand, protocol, filter,
					  customAntConfig->customAntConfigPerAntenna[count]->antID, count);
			}
			if (TMR_SUCCESS != ret)
			{
		      		return ret;
			}
		}
	}
	else if (NULL != filter)
       {
	      pAntConfig     = malloc(sizeof(struct LLRP_tSAntennaConfiguration *));
	      pAntConfig[0]     = LLRP_AntennaConfiguration_construct();
	      pInvCommand    = malloc(sizeof(struct LLRP_tSC1G2InventoryCommand *));
              pInvCommand[0]    = LLRP_C1G2InventoryCommand_construct();
	      ret = TMR_LLRP_prepareTagFilter(pAntConfig, pInvCommand, protocol, filter, 0U, 0U);
       } 

        if (TMR_SUCCESS != ret)
        {
          return ret;
        }
      }

      /**
       * Add FastSearch option as Custom
       * parameter to the Inventory parameter
       * Add Custom Antenna configuration (Session, Target and filter) per individual antenna to the Inventory parameter
       **/
      if(NULL != customAntConfig || useFastSearch)
      {
        if(pAntConfig == NULL)
	{
	  pAntConfig     = malloc(sizeof(struct LLRP_tSAntennaConfiguration *));
	  pAntConfig[0]     = LLRP_AntennaConfiguration_construct();
	}
	if(pInvCommand == NULL)
	{
	      pInvCommand    = malloc(sizeof(struct LLRP_tSC1G2InventoryCommand *));
              pInvCommand[0]    = LLRP_C1G2InventoryCommand_construct();
	}
        ret = TMR_LLRP_addCustomAntennaConfigPerAntenna(customAntConfig, pAntConfig, pInvCommand, useFastSearch);
        if (TMR_SUCCESS != ret)
        {
          return ret;
        }
      }
      if(NULL != customAntConfig)
      {
		for (count=0; count < customAntConfig->antennaCount; count++)
  		{ 
		  /* Add InvCommand to antenna configuration */
                  LLRP_AntennaConfiguration_addAirProtocolInventoryCommandSettings(pAntConfig[count],
                  (LLRP_tSParameter *)pInvCommand[count]);
                  /* Finally, Add antenna configuration to InventoryParameterSpec */
		  LLRP_InventoryParameterSpec_addAntennaConfiguration(pInventoryParameterSpec, pAntConfig[count]);

		}
      }
      else if (useFastSearch || NULL != filter )
      {
        /* Add InvCommand to antenna configuration */
        LLRP_AntennaConfiguration_addAirProtocolInventoryCommandSettings(pAntConfig[0],
          (LLRP_tSParameter *)pInvCommand[0]);
        /* Finally, Add antenna configuration to InventoryParameterSpec */
        LLRP_InventoryParameterSpec_addAntennaConfiguration(pInventoryParameterSpec, pAntConfig[0]);
      }
      if ((TMR_READ_PLAN_TYPE_MULTI == reader->readParams.readPlan->type) &&
          (true == isPerAntennaEnabled))
      {
        /**
         * Add readPlan weight option as Custom
         * parameter to the Inventory parameter
         **/
        LLRP_tSThingMagicCustomReadplanWeight *pReadplanWeight;

        simpleReadPlanWeight = reader->readParams.readPlan->u.multi.plans[planCount]->weight;
        multiReadPlanWeight = reader->readParams.readPlan->weight;
        pReadplanWeight = LLRP_ThingMagicCustomReadplanWeight_construct();
        LLRP_ThingMagicCustomReadplanWeight_setplanWeight(pReadplanWeight, simpleReadPlanWeight);
        LLRP_ThingMagicCustomReadplanWeight_setmultiPlanWeight(pReadplanWeight, multiReadPlanWeight);

        /* add this as a custom parameter to InventoryParameterSpec */
        LLRP_InventoryParameterSpec_addCustom(pInventoryParameterSpec, (LLRP_tSParameter *)pReadplanWeight);
      }

      if(NULL != customAntConfig)
      {
        /**
	 * Add ThingMagicCustomAntennaSwitching type
	 **/
        LLRP_tSThingMagicCustomAntennaSwitching *pAntSwitching;
	pAntSwitching = LLRP_ThingMagicCustomAntennaSwitching_construct();

        if (customAntConfig->antSwitchingType == LLRP_ThingMagicCustomAntennaSwitchingType_Dynamic)
        {
          LLRP_ThingMagicCustomAntennaSwitching_setAntSwitchingType(pAntSwitching, LLRP_ThingMagicCustomAntennaSwitchingType_Dynamic);
        }
        else
        {
          LLRP_ThingMagicCustomAntennaSwitching_setAntSwitchingType(pAntSwitching, LLRP_ThingMagicCustomAntennaSwitchingType_Equal);
        }
        LLRP_ThingMagicCustomAntennaSwitching_setTimeout(pAntSwitching, customAntConfig->tagReadTimeout);
        LLRP_InventoryParameterSpec_addCustom(pInventoryParameterSpec, (LLRP_tSParameter *)pAntSwitching);
      }

      /* Add InventoryParameterSpec to AISpec */
      LLRP_AISpec_addInventoryParameterSpec(pAISpec, pInventoryParameterSpec);

      /* Now AISpec is fully framed and add it to ROSpec Parameter list */
      LLRP_ROSpec_addSpecParameter(pROSpec, (LLRP_tSParameter *)pAISpec);
      filter = NULL;
    }

    /**
     * 6. Initialize and Set ROReportSpec
     **/
    {
      pROReportSpec = LLRP_ROReportSpec_construct();

      /* Set ROReportSpecTrigger type  */
      LLRP_ROReportSpec_setROReportTrigger(pROReportSpec, LLRP_ROReportTriggerType_Upon_N_Tags_Or_End_Of_ROSpec);
      
      if (reader->continuousReading)
      {
        /**
         * In case of continuous Reading, report is
         * requested for every 10 tags. (10 being the moderate value)
         * In case there is no report (when there are not enough tags),
         * we are supposed to GET_REPORT from reader.
         **/
        LLRP_ROReportSpec_setN(pROReportSpec, 1);
      }
      else
      {
        /**
         * For sync read, only one report is expected at the end of search
         * Setting N = 0, will send report only at the end of ROSpec.
         **/
        LLRP_ROReportSpec_setN(pROReportSpec, 0);
      }

      /**
       * Get Configured Metadata flag and cache it
       **/
      ret = TMR_LLRP_cmdGetTMMetadataFlag(reader, (uint16_t *)&reader->u.llrpReader.metadata);
      if (TMR_SUCCESS != ret)
      {
        /**
         * Not Fatal, moving forward
         * value might be changed, restore the dafult value.
         **/
        reader->u.llrpReader.metadata = TMR_TRD_METADATA_FLAG_ALL;
      }

      /* Initialize and Set ReportContent selection */
      pTagReportContentSelector = LLRP_TagReportContentSelector_construct();

      pTagReportContentSelector->EnableROSpecID                 = 1;
      pTagReportContentSelector->EnableSpecIndex                = 1;
      if (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_ANTENNAID)
      {
        pTagReportContentSelector->EnableAntennaID                = 1;
      }
      if (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_FREQUENCY)
      {
        pTagReportContentSelector->EnableChannelIndex             = 1;
      }
      if (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_RSSI)
      {
        pTagReportContentSelector->EnablePeakRSSI                 = 1;
      }
      if (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_TIMESTAMP)
      {
        pTagReportContentSelector->EnableFirstSeenTimestamp       = 1;
        pTagReportContentSelector->EnableLastSeenTimestamp        = 1;
      }
      if (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_READCOUNT)
      {
        pTagReportContentSelector->EnableTagSeenCount             = 1;
      }

      if (TMR_TAG_PROTOCOL_GEN2 == protocol)
      {
        /**
         * If Gen2 protocol, ask for CRC and PCBits as well.
         **/
        LLRP_tSC1G2EPCMemorySelector *pGen2MemSelector;

        pGen2MemSelector = LLRP_C1G2EPCMemorySelector_construct();
        LLRP_C1G2EPCMemorySelector_setEnableCRC(pGen2MemSelector, 1);
        LLRP_C1G2EPCMemorySelector_setEnablePCBits(pGen2MemSelector, 1);

        /* Add C1G2MemorySelector to ReportContentSelector  */
        LLRP_TagReportContentSelector_addAirProtocolEPCMemorySelector(
            pTagReportContentSelector, (LLRP_tSParameter *)pGen2MemSelector);

      }

      LLRP_ROReportSpec_setTagReportContentSelector(pROReportSpec, pTagReportContentSelector);
      /**
       * Add thingMagicTagReportContentSelector
       * For Backwared compatibility check the version
       **/
      if (((atoi(&reader->u.llrpReader.capabilities.softwareVersion[0]) == 4) 
            && (atoi(&reader->u.llrpReader.capabilities.softwareVersion[2]) >= 17))
          || (atoi(&reader->u.llrpReader.capabilities.softwareVersion[0]) > 4))
      {
        LLRP_tSThingMagicTagReportContentSelector *pTMTagReportContentSelector;
        LLRP_tSMetadataGPIOMode                   *pTMMetadataGPIOMode;
        LLRP_tSMetadataGen2QMode                  *pTMMetadataGen2QMode;
        LLRP_tSMetadataGen2LFMode                 *pTMMetadataGen2LFMode;
        LLRP_tSMetadataGen2TargetMode             *pTMMetadataGen2TargetMode;
        LLRP_tSMetadataDataMode                   *pTMMetadataDataMode;

        /* Initialize and Set the ThingMagicReportContentSelector */
        pTMTagReportContentSelector = LLRP_ThingMagicTagReportContentSelector_construct();

        if (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_PHASE)
        {
          /* Set the phase angel mode */
          LLRP_ThingMagicTagReportContentSelector_setPhaseMode(
              pTMTagReportContentSelector, (LLRP_tEThingMagicPhaseMode)
              LLRP_ThingMagicPhaseMode_Enabled);
        }
        else
        {
          /* Set the phase angel mode */
          LLRP_ThingMagicTagReportContentSelector_setPhaseMode(
              pTMTagReportContentSelector, (LLRP_tEThingMagicPhaseMode)
              LLRP_ThingMagicPhaseMode_Disabled);
        }

        if (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_GPIO_STATUS)
        {
          pTMMetadataGPIOMode = LLRP_MetadataGPIOMode_construct();
          LLRP_MetadataGPIOMode_setMode(pTMMetadataGPIOMode,
              LLRP_ThingMagicMetadataFlagStatus_Enabled);
          LLRP_ThingMagicTagReportContentSelector_setMetadataGPIOMode(
              pTMTagReportContentSelector, (LLRP_tSMetadataGPIOMode *)
              pTMMetadataGPIOMode); 
        }

        if (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_GEN2_Q)
        {
          pTMMetadataGen2QMode = LLRP_MetadataGen2QMode_construct();
          LLRP_MetadataGen2QMode_setMode(pTMMetadataGen2QMode,
              LLRP_ThingMagicMetadataFlagStatus_Enabled);
          LLRP_ThingMagicTagReportContentSelector_setMetadataGen2QMode(
              pTMTagReportContentSelector, (LLRP_tSMetadataGen2QMode *)
              pTMMetadataGen2QMode); 
        }

        if (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_GEN2_LF)
        {
          pTMMetadataGen2LFMode = LLRP_MetadataGen2LFMode_construct();
          LLRP_MetadataGen2LFMode_setMode(pTMMetadataGen2LFMode,
              LLRP_ThingMagicMetadataFlagStatus_Enabled);
          LLRP_ThingMagicTagReportContentSelector_setMetadataGen2LFMode(
              pTMTagReportContentSelector, (LLRP_tSMetadataGen2LFMode *)
              pTMMetadataGen2LFMode); 
        }

        if (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_GEN2_TARGET)
        {
          pTMMetadataGen2TargetMode = LLRP_MetadataGen2TargetMode_construct();
          LLRP_MetadataGen2TargetMode_setMode(pTMMetadataGen2TargetMode,
              LLRP_ThingMagicMetadataFlagStatus_Enabled);
          LLRP_ThingMagicTagReportContentSelector_setMetadataGen2TargetMode(
              pTMTagReportContentSelector, (LLRP_tSMetadataGen2TargetMode *)
              pTMMetadataGen2TargetMode); 
        }

        if (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_DATA)
        {
          pTMMetadataDataMode = LLRP_MetadataDataMode_construct();
          LLRP_MetadataDataMode_setMode(pTMMetadataDataMode,
              LLRP_ThingMagicMetadataFlagStatus_Enabled);
          LLRP_ThingMagicTagReportContentSelector_setMetadataDataMode(
              pTMTagReportContentSelector, (LLRP_tSMetadataDataMode *)
              pTMMetadataDataMode); 
        }

        /**
         * Set the TMTagReportContentSelector as custom
         * parameter to the ROReportSpec
         **/
        LLRP_ROReportSpec_addCustom(pROReportSpec, (LLRP_tSParameter *)pTMTagReportContentSelector);
      }

      /* Now ROReportSpec is fully framed and set it to ROSpec */
      LLRP_ROSpec_setROReportSpec(pROSpec, pROReportSpec);
    }

    /**
     * 7. Initialize and set RFSurveySpec
     **/
    if((true == reader->continuousReading) && (reader->u.llrpReader.featureFlags & TMMP_READER_FEATURES_FLAG_STATS_LISTENER))
    {
      LLRP_tSRFSurveySpecStopTrigger *pRFSurveyStopTrigger;
      pRFSurveySpec = LLRP_RFSurveySpec_construct();

      /* Set first antenna from the antenna list to AntennaID */
      LLRP_RFSurveySpec_setAntennaID(pRFSurveySpec, AntennaIDs.pValue[0]);

      /* Set setStartFrequency value */
      LLRP_RFSurveySpec_setStartFrequency(pRFSurveySpec, tempHopTable.list[0]);

      /* Set setEndFrequency value */
      LLRP_RFSurveySpec_setEndFrequency(pRFSurveySpec, tempHopTable.list[tempHopTable.len]);

      /* Set RFSurveySpecStopTrigger type */
      pRFSurveyStopTrigger = LLRP_RFSurveySpecStopTrigger_construct();

      LLRP_RFSurveySpecStopTrigger_setStopTriggerType(pRFSurveyStopTrigger, 0);
      LLRP_RFSurveySpec_setRFSurveySpecStopTrigger(pRFSurveySpec, pRFSurveyStopTrigger);
      /**
       * Get Configured stats flag and cache it
       **/
      ret = TMR_LLRP_cmdGetTMStatsEnable(reader, (uint16_t *)&reader->u.llrpReader.statsEnable);
      if (TMR_SUCCESS != ret)
      {
        /**
         * Not Fatal, moving forward
         * value might be changed, restore the dafult value.
         **/
        reader->u.llrpReader.statsEnable = TMR_READER_STATS_FLAG_ALL;
      }
      pCustomRFSurveySpec = LLRP_CustomRFSurveySpec_construct();
      LLRP_CustomRFSurveySpec_setStatsEnable(pCustomRFSurveySpec, reader->u.llrpReader.statsEnable);
      LLRP_RFSurveySpec_addCustom(pRFSurveySpec, (LLRP_tSParameter *)pCustomRFSurveySpec);

      LLRP_ROSpec_addSpecParameter(pROSpec, (LLRP_tSParameter *)pRFSurveySpec);
    }
  }

  /* Now ROSpec is fully framed, add to AddROSpec */
  LLRP_ADD_ROSPEC_setROSpec(pCmd, pROSpec);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSADD_ROSPEC_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to Enable the ROSpec
 *
 * @param reader Reader pointer
 */
TMR_Status
TMR_LLRP_cmdEnableROSpec(TMR_Reader *reader)
{
  TMR_Status ret;
  LLRP_tSENABLE_ROSPEC          *pCmd;
  LLRP_tSMessage                *pCmdMsg;
  LLRP_tSMessage                *pRspMsg;
  LLRP_tSENABLE_ROSPEC_RESPONSE *pRsp;
  
  ret = TMR_SUCCESS;

  /**
   * Initialize EnableROSpec message
   **/
  pCmd = LLRP_ENABLE_ROSPEC_construct();
  LLRP_ENABLE_ROSPEC_setROSpecID(pCmd, reader->u.llrpReader.roSpecId);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSENABLE_ROSPEC_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}
TMR_Status
TMR_LLRP_cmdDisableROSpec(TMR_Reader *reader)
{
  TMR_Status ret;
  LLRP_tSDISABLE_ROSPEC          *pCmd;
  LLRP_tSMessage                *pCmdMsg;
  LLRP_tSMessage                *pRspMsg;
  LLRP_tSDISABLE_ROSPEC_RESPONSE *pRsp;
  
  ret = TMR_SUCCESS;

  /**
   * Initialize EnableROSpec message
   **/
  pCmd = LLRP_DISABLE_ROSPEC_construct();
  LLRP_DISABLE_ROSPEC_setROSpecID(pCmd, reader->u.llrpReader.roSpecId);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSDISABLE_ROSPEC_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to Start the ROSpec
 *
 * @param reader Reader pointer
 */
TMR_Status
TMR_LLRP_cmdStartROSpec(TMR_Reader *reader, llrp_u32_t roSpecId)
{
  TMR_Status ret;
  LLRP_tSSTART_ROSPEC           *pCmd;
  LLRP_tSMessage                *pCmdMsg;
  LLRP_tSMessage                *pRspMsg;
  LLRP_tSSTART_ROSPEC_RESPONSE  *pRsp;
  
  ret = TMR_SUCCESS;

  /**
   * Initialize StartROSpec message
   **/
  pCmd = LLRP_START_ROSPEC_construct();
  LLRP_START_ROSPEC_setROSpecID(pCmd, roSpecId);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSTART_ROSPEC_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to Stop ROSpec
 *
 * @param reader Reader pointer
 * @param receiveResponse Boolean parameter, whether to receive response or not.
 *        true = receive response for STOP_ROSPEC message
 *        false = Do not receive response
 */
TMR_Status
TMR_LLRP_cmdStopROSpec(TMR_Reader *reader, bool receiveResponse)
{
  TMR_Status ret;
  LLRP_tSSTOP_ROSPEC            *pCmd;
  LLRP_tSMessage                *pCmdMsg;
  LLRP_tSMessage                *pRspMsg;
  LLRP_tSSTOP_ROSPEC_RESPONSE   *pRsp;

  ret = TMR_SUCCESS;

  /**
   * Initialize StartROSpec message
   **/
  pCmd = LLRP_STOP_ROSPEC_construct();
  LLRP_STOP_ROSPEC_setROSpecID(pCmd, reader->u.llrpReader.roSpecId);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  if (false == receiveResponse)
  {
    /* As the response for the previous message is not received & going to intiate stop read command, 
     * wait until previous sendMessage timeout first. This is modified to fix stop reading blockage issue. 
     */
    if(true == tx_mutex_lock_enabled)
    {
      tmr_sleep(reader->u.llrpReader.transportTimeout);
    }
    /* If still mutex lock is enabled after specified wait, then unlock mutex forcibly */
    if(true == tx_mutex_lock_enabled)
    {
      pthread_mutex_unlock(&reader->u.llrpReader.transmitterLock);
      tx_mutex_lock_enabled = false;
    }

    ret = TMR_LLRP_sendMessage(reader, pCmdMsg, reader->u.llrpReader.transportTimeout);
    /**
     * done with the command, free it
     **/
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  }
  else
  {
    ret = TMR_LLRP_sendTimeout(reader, pCmdMsg, &pRspMsg,
                               TMR_LLRP_STOP_TIMEOUT
                               + reader->u.llrpReader.commandTimeout
                               + reader->u.llrpReader.transportTimeout);

    /**
     * Done with the command, free the message
     * and check for message status
     **/ 
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSTOP_ROSPEC_RESPONSE *) pRspMsg;
    if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
      return TMR_ERROR_LLRP;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  }

  return ret;
}

/**
 * Prepare ROSpec
 * This method does the initial preparations inorder
 * to perform Reader operation.
 *
 * Note: This does not start the RO Operation.
 *
 * @param reader Reader pointer
 * @param timeout Read duration
 * @param antennaList Antenna list to be used
 * @param filter Tag Filter to be used
 * @param protocol Protocol to be used
 */
TMR_Status 
TMR_LLRP_cmdPrepareROSpec(TMR_Reader *reader, uint16_t timeout, 
                            TMR_uint8List *antennaList,
                            TMR_TagFilter *filter, 
                            TMR_TagProtocol protocol)
{
  TMR_Status ret;
  
  ret = TMR_SUCCESS;

  /**
   *  1. ADD_ROSPEC
   *  2. ENABLE_ROSPEC
   **/
 
  /**
   * 1. AddROSpec
   **/
  ret = TMR_LLRP_cmdAddROSpec(reader, timeout, antennaList, filter, protocol);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * 2. Enable ROSpec
   **/
  return TMR_LLRP_cmdEnableROSpec(reader);
}

TMR_Status
TMR_LLRP_verifyReadOperation(TMR_Reader *reader, int32_t *tagCount)
{
  LLRP_tSTagReportData    *pTagReportData;
  LLRP_tSRO_ACCESS_REPORT *pReport;
  TMR_LLRP_LlrpReader *lr;
  uint32_t count, i;

  lr = &reader->u.llrpReader;
  /**
   * Wait until the read is finished, meanwhile
   * the tag reads are buffered by the background receiver.
   **/
  while (true)
  {
    pthread_mutex_lock(&lr->receiverLock);
    while(lr->numOfROSpecEvents > 0 ) {
      pthread_cond_wait(&lr->receiverCond, &lr->receiverLock);
    }
    if (0 >= lr->numOfROSpecEvents)
  {
      /* We have received all ROSpec events */
      if (-1 == lr->numOfROSpecEvents)
    {
      pthread_mutex_unlock(&lr->receiverLock);
        return TMR_ERROR_LLRP_READER_CONNECTION_LOST;
    }
      pthread_mutex_unlock(&lr->receiverLock);
      break;
  }
    pthread_mutex_unlock(&lr->receiverLock);

    /* Wait until all rospec events arrive */
  }

  for (i = 0; i < lr->bufPointer; i ++)
  {
    pReport = (LLRP_tSRO_ACCESS_REPORT *)lr->bufResponse[i];
    if (NULL == pReport)
    {
    /**
       * If NULL, no tag reports have been received.
       * i.e., no tags found, continue operation
     **/
      continue;
    }

    count = 0;
      /**
       * Count the number of tag reports and fill tagCount
       **/
      for(pTagReportData = pReport->listTagReportData;
          NULL != pTagReportData;
          pTagReportData = (LLRP_tSTagReportData *)pTagReportData->hdr.pNextSubParameter)
      {
        count ++;
      }

    lr->tagsRemaining += count;
      if (NULL != tagCount)
      {
        *tagCount += count;
      }
    }

  return TMR_SUCCESS;
}

/**
 * Internal method to parse metadata from LLRP Report response
 * This method constructs a TMR_TagReadData object by extracting the 
 * information from LLRP RO and ACCESS Reports
 *
 * @param reader Reader pointer
 * @param data[out] Pointer to TMR_TagReadData
 * @param pTagReportData[in] Pointer to LLRP_tSTagReportData which needs to be parsed.
 */
TMR_Status
TMR_LLRP_parseMetadataFromMessage(TMR_Reader *reader, TMR_TagReadData *data, LLRP_tSTagReportData *pTagReportData)
{
  TMR_Status ret;
  LLRP_tSParameter *pEPC;
  llrp_u16_t        ChannelIndex;

  ret = TMR_SUCCESS;

  if (NULL != pTagReportData)
  {
    if(NULL != pTagReportData->pEPCParameter)
    {
      /**
       * If EPCParameter is present, get epc bytecount and copy 
       * the value of epc.
       **/
      pEPC = pTagReportData->pEPCParameter;
      if (&LLRP_tdEPCData == pEPC->elementHdr.pType)
      {
        /**
         * If EPCParameter is of type LLRP_tSEPCData
         **/
        LLRP_tSEPCData *pEPCData = (LLRP_tSEPCData *)pEPC;
        data->tag.epcByteCount = (pEPCData->EPC.nBit + 7u) / 8u;
        memcpy(data->tag.epc, pEPCData->EPC.pValue, data->tag.epcByteCount);
      }
      else
      {
        /**
         * Else it is a LLRP_tSEPC_96 EPC, hence the epcByteCount
         * is always 12 bytes
         **/
        LLRP_tSEPC_96 *pEPC_96 = (LLRP_tSEPC_96 *)pEPC;
        data->tag.epcByteCount = 12;
        memcpy(data->tag.epc, pEPC_96->EPC.aValue, data->tag.epcByteCount);
      }
    }
    else
    {
      return TMR_ERROR_LLRP;
    }

    /**
     * Copy the timestamp
     **/
    if (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_TIMESTAMP)
    {
      if (NULL != pTagReportData->pLastSeenTimestampUTC)
      {
        llrp_u64_t msSinceEpoch = (pTagReportData->pLastSeenTimestampUTC->Microseconds)/1000;
        data->dspMicros = (uint32_t)(msSinceEpoch % 1000);
        data->timestampHigh = (uint32_t)(msSinceEpoch>>32) & 0xFFFFFFFF;
        data->timestampLow  = (uint32_t)(msSinceEpoch>> 0) & 0xFFFFFFFF;
        data->metadataFlags |= TMR_TRD_METADATA_FLAG_TIMESTAMP;
      }
      else
      {
        return TMR_ERROR_LLRP;
      }
    }

    /**
     * Copy the rest of metadata
     * TODO: Add support for other metadata when the 
     * server side has implemenation.
     **/
    if (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_ANTENNAID)
    {
      if (NULL != pTagReportData->pAntennaID)
      {
        data->antenna = (uint8_t)pTagReportData->pAntennaID->AntennaID;
        data->metadataFlags |= TMR_TRD_METADATA_FLAG_ANTENNAID;
      }
      else
      {
        return TMR_ERROR_LLRP;
      }
    }

    if (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_READCOUNT)
    {
      if (NULL != pTagReportData->pTagSeenCount)
      {
        data->readCount = pTagReportData->pTagSeenCount->TagCount;
        data->metadataFlags |= TMR_TRD_METADATA_FLAG_READCOUNT;
      }
      else
      {
        return TMR_ERROR_LLRP;
      }
    }

    if (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_RSSI)
    {
      if (NULL != pTagReportData->pPeakRSSI)
      {
        data->rssi = pTagReportData->pPeakRSSI->PeakRSSI;
        data->metadataFlags |= TMR_TRD_METADATA_FLAG_RSSI;
      }
      else
      {
        return TMR_ERROR_LLRP;
      }
    }
    /**
     * Copy the RF Carrier frequency with which the tag was read
     * right now only one HopeTable is supported
     **/
    if (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_FREQUENCY)
    {
      if (NULL != pTagReportData->pChannelIndex)
      {
        ChannelIndex = pTagReportData->pChannelIndex->ChannelIndex;
      }
      else
      {
        return TMR_ERROR_LLRP;
      }
      /* as for llrp the indexing always starts with one
       * but in api indexing always starts with zero
       * so, decrementing the index
       */
      if ((NULL != reader->u.llrpReader.capabilities.freqTable.list) &&
          (ChannelIndex != 0))
      {
        data->frequency = reader->u.llrpReader.capabilities.freqTable.list[ChannelIndex - 1];
        data->metadataFlags |= TMR_TRD_METADATA_FLAG_FREQUENCY;
      }
    }

    {
      /**
       * TODO: Currently protocol is not available as part of the metadata,
       * Add support to that when server side has the implementation for it.
       * For now, hardcoding it to GEN2
       **/
      {
        /** 
         * Initialize the protocol form the readPlanProtocolList
         * depending upon the rospec id
         **/ 
        unsigned int protocolindex;
        if (NULL != pTagReportData->pROSpecID)
        {
          for (protocolindex = 0; protocolindex <= pTagReportData->pROSpecID->ROSpecID; protocolindex++)
          {
            if (protocolindex == pTagReportData->pROSpecID->ROSpecID)
            {
              data->tag.protocol = reader->u.llrpReader.readPlanProtocol[protocolindex].rospecProtocol;
            }
          }
        }
        else
        {
          return TMR_ERROR_LLRP;
        }
      }
      data->metadataFlags |= TMR_TRD_METADATA_FLAG_PROTOCOL;
    }
    if ((reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_PHASE)
        || (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_PROTOCOL)
        || (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_DATA)
        || (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_GPIO_STATUS)
        || (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_GEN2_Q)
        || (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_GEN2_LF)
        || (reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_GEN2_TARGET))
    {
      /**
       * Extract TMCustomParameters, if any
       * For backward compatibility check for version
       **/
      if (((atoi(&reader->u.llrpReader.capabilities.softwareVersion[0]) == 4) 
            && (atoi(&reader->u.llrpReader.capabilities.softwareVersion[2]) >= 17))
            || (atoi(&reader->u.llrpReader.capabilities.softwareVersion[0]) > 4))
      {
        LLRP_tSParameter *pParameter;
        llrp_u16_t        phase;
        llrp_u8_t         protocolID;
        llrp_u8_t         tagopResultType;
        LLRP_tSTagopByteStreamParam *pTagopResultData;
        llrp_u8v_t        tagopByteStream;

        if (isPerAntennaEnabled)
        {
          for (pParameter = pTagReportData->listCustom;
            NULL != pParameter;
            pParameter = (LLRP_tSParameter *)pParameter->pNextSubParameter)
          {
            switch(pParameter->elementHdr.pType->TypeNum)
            {
              case TMMP_CUSTOM_RFPHASE:
                /**
                 * Currently phase is coming as 1st custom parameter
                 **/
                if(reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_PHASE)
                {
                  phase = LLRP_ThingMagicRFPhase_getPhase(
                      (LLRP_tSThingMagicRFPhase *)pParameter);

                  /* Copy the value to tagReport */
                  data->phase = phase;
                  data->metadataFlags |= TMR_TRD_METADATA_FLAG_PHASE;
                }
                break;
              case TMMP_CUSTOM_PROTOCOL_ID:
                /**
                 * Currently protocol ID is coming as 2nd custom parameter
                 **/
                if(reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_PROTOCOL)
                {
                  protocolID = LLRP_ThingMagicCustomProtocolID_getProtocolId(
                                          (LLRP_tSThingMagicCustomProtocolID *)pParameter);
                  /* Copy the value to tagReport */
                  switch(protocolID)
                  {
                    case LLRP_ThingMagicCustomProtocol_Gen2:
                      data->tag.protocol = TMR_TAG_PROTOCOL_GEN2;
                      break;
                    case LLRP_ThingMagicCustomProtocol_Iso180006b:
                      data->tag.protocol = TMR_TAG_PROTOCOL_ISO180006B;
                      break;
                    case LLRP_ThingMagicCustomProtocol_IPX64:
                      data->tag.protocol = TMR_TAG_PROTOCOL_IPX64;
                      break;
                    case LLRP_ThingMagicCustomProtocol_IPX256:
                      data->tag.protocol = TMR_TAG_PROTOCOL_IPX256;
                      break;
                    case LLRP_ThingMagicCustomProtocol_Ata:
                      data->tag.protocol = TMR_TAG_PROTOCOL_ATA;
                      break;
                    default:
                      break;
                  }
                  data->metadataFlags |= TMR_TRD_METADATA_FLAG_PROTOCOL;
                }
                  break;
              case TMMP_CUSTOM_TAGOP_RESPONSE:
                /**
                 * Currently Tagop response is coming as 3rd custom parameter
                 * for embedded tagop with multi read plan case
                 **/ 
                if(reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_DATA)
                {
                  tagopResultType = LLRP_ThingMagicCustomTagopResponse_getResult(
                      (LLRP_tSThingMagicCustomTagopResponse *)pParameter);

                  if (tagopResultType == LLRP_ThingMagicCustomTagOpSpecResultType_Success)
                  {
                    pTagopResultData = LLRP_ThingMagicCustomTagopResponse_getTagopByteStreamParam(
                        (LLRP_tSThingMagicCustomTagopResponse *)pParameter);
                    tagopByteStream = LLRP_TagopByteStreamParam_getByteStream(pTagopResultData);
                
                    data->data.len = tagopByteStream.nValue;
                    if (data->data.len > data->data.max) {
                      data->data.len = data->data.max;
                    }

                    if (NULL != data->data.list) {
                    memcpy (data->data.list, tagopByteStream.pValue,
                        (size_t)(data->data.len));
                    }
                    data->metadataFlags |= TMR_TRD_METADATA_FLAG_DATA;
                  }
                }
                break;
              case TMMPD_CUSTOM_GPIO_STATUS:
                /**
                 * Currently GPIO status is coming as custom parameter
                 * in TagReportData
                 **/ 
                if(reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_GPIO_STATUS)
                {
                  uint8_t i;
                  LLRP_tSGPIOStatus *pGPIOStatus;
                  for (pGPIOStatus = LLRP_ThingMagicMetadataGPIO_beginGPIOStatus(
                         (LLRP_tSThingMagicMetadataGPIO *)pParameter), i = 0;
                    NULL != pGPIOStatus;
                    pGPIOStatus = LLRP_ThingMagicMetadataGPIO_nextGPIOStatus(
                         (LLRP_tSGPIOStatus *)pGPIOStatus), i++)
                  {
                    data->gpio[i].id = LLRP_GPIOStatus_getid(pGPIOStatus);
                    data->gpio[i].high = LLRP_GPIOStatus_getStatus(pGPIOStatus);
                    data->gpio[i].output = LLRP_GPIOStatus_getDirection(pGPIOStatus);
                  }
                  data->gpioCount = i;
                  data->metadataFlags |= TMR_TRD_METADATA_FLAG_GPIO_STATUS;
                }
                break;
              case TMMPD_CUSTOM_GEN2:
                /**
                 * Currently GEN2 Q, LF and TARGET is coming as custom parameter
                 * in TagReportData
                 **/ 
                if(reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_GEN2_Q)
                {
                  LLRP_tSGen2QResponse *pGen2QResponse;
                  pGen2QResponse = LLRP_ThingMagicMetadataGen2_getGen2QResponse(
                           (LLRP_tSThingMagicMetadataGen2 *)pParameter);
                  data->u.gen2.q.u.staticQ.initialQ = LLRP_Gen2QResponse_getQValue(
                           (LLRP_tSGen2QResponse *)pGen2QResponse);
                  data->metadataFlags |= TMR_TRD_METADATA_FLAG_GEN2_Q;
                }
                if(reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_GEN2_LF)
                {
                  LLRP_tSGen2LFResponse *pGen2LFResponse;
                  pGen2LFResponse = LLRP_ThingMagicMetadataGen2_getGen2LFResponse(
                           (LLRP_tSThingMagicMetadataGen2 *)pParameter);
                  data->u.gen2.lf = LLRP_Gen2LFResponse_getLFValue(
                           (LLRP_tSGen2LFResponse *)pGen2LFResponse);
                  data->metadataFlags |= TMR_TRD_METADATA_FLAG_GEN2_LF;
                }
                if(reader->u.llrpReader.metadata & TMR_TRD_METADATA_FLAG_GEN2_TARGET)
                {
                  LLRP_tSGen2TargetResponse *pGen2TargetResponse;
                  pGen2TargetResponse = LLRP_ThingMagicMetadataGen2_getGen2TargetResponse(
                           (LLRP_tSThingMagicMetadataGen2 *)pParameter);
                  data->u.gen2.target = LLRP_Gen2TargetResponse_getTargetValue(
                           (LLRP_tSGen2TargetResponse *)pGen2TargetResponse);
                  data->metadataFlags |= TMR_TRD_METADATA_FLAG_GEN2_TARGET;
                }
                break;
              default:
                break;
            }
          }
        }
        else
        {
          for (pParameter = pTagReportData->listCustom;
            NULL != pParameter;
            pParameter = (LLRP_tSParameter *)pParameter->pNextSubParameter)
          {
            /**
             * Currently phase is coming as custom parameter
             **/ 
            phase = LLRP_ThingMagicRFPhase_getPhase(
                (LLRP_tSThingMagicRFPhase *)pParameter);

            /* Copy the value to tagReport */
            data->phase = phase;
          }
        }
      }
    }

    if (TMR_TAG_PROTOCOL_GEN2 == data->tag.protocol)
    {
      LLRP_tSParameter *pParameter;

      /**
       * If protocol is Gen2, extract CRC and PCBits
       **/
      for(pParameter = pTagReportData->listAirProtocolTagData;
          NULL != pParameter;
          pParameter = (LLRP_tSParameter *)pParameter->pNextSubParameter)
      {
        if (&LLRP_tdC1G2_PC == pParameter->elementHdr.pType)
        {
          llrp_u16_t pc;

          pc = LLRP_C1G2_PC_getPC_Bits((LLRP_tSC1G2_PC *)pParameter);
          data->tag.u.gen2.pc[0] = pc & 0xFF;
          data->tag.u.gen2.pc[1] = (pc & 0xFF00) >> 8;

          data->tag.u.gen2.pcByteCount = 2;

          /** TODO: Add support for XPC bits */
        }
        else if (&LLRP_tdC1G2_CRC == pParameter->elementHdr.pType)
        {
          data->tag.crc = LLRP_C1G2_CRC_getCRC((LLRP_tSC1G2_CRC *)pParameter);
        }
      } /* End of for loop */
    }

    /**
     * Extract OpSpecResults if any. 
     **/
    {
      LLRP_tSParameter *pParameter;

      for (pParameter = pTagReportData->listAccessCommandOpSpecResult;
           NULL != pParameter;
           pParameter = (LLRP_tSParameter *)pParameter->pNextSubParameter)
      {
        /**
         * Each airProtocol's OpSpecResult is expressed as a different
         * LLRP parameter. Verify the OpSpecResult status, if not success,
         * then notify the exception to user.
         **/
        TMR_Status retVal;

        retVal = TMR_LLRP_verifyOpSpecResultStatus(reader, pParameter);
        if (TMR_SUCCESS != retVal)
        {
          /**
           * The OpSpec has failed to execute, notify the execption to 
           * user.
           * FIXME: Notifying the error through exception listener.
           * This is exact behavior in case of continuous reading, 
           * How do we handle this for sync reads? We can not return
           * Error though..
           **/
#ifdef TMR_ENABLE_BACKGROUND_READS
          notify_exception_listeners(reader, retVal);
#endif

          /**
           * Continue with next OpSpecResult parameter.
           **/
          continue;
        }

        /**
         * At this point, we have verified the OpSpecResult status,
         * and it is success.
         * But currently, only for ReadData (i.e., C1G2ReadOpSpecResult),
         * we extract the read data and place it in TMR_TagReadData->data
         * TODO: For other tag operations which return tagop data,
         * where do we store that?
         **/
        if (NULL != &data->data)
        {
          TMR_LLRP_parseTagOpSpecData(pParameter, &data->data);
          data->metadataFlags |= TMR_TRD_METADATA_FLAG_DATA;
        }
      } /* End of for loop */
    } /* End of parsing OpSpecResults */
  }
  else
  {
    return TMR_ERROR_LLRP;
  }

  return ret;
}

/**
 * Internal method to parse TagOp data from TagOpSpecResult parameter
 * This method extracts the data from TagOpSpecResult and 
 * fills the data variable as a uint8List.
 *
 * @param pParameter[in] Pointer to LLRP_tSParameter which contains the opspec result
 * @param data[out] Pointer to TMR_uint8List which contains the extracted data.
 */
void
TMR_LLRP_parseTagOpSpecData(LLRP_tSParameter *pParameter, TMR_uint8List *data)
{
  switch (pParameter->elementHdr.pType->TypeNum)
  {
    /**
     * C1G2ReadOpSpecResult
     **/
    case TMR_LLRP_C1G2READOPSPECRESULT:
      {
        LLRP_tSC1G2ReadOpSpecResult *pReadOpSpecResult;
        int copyLen;

        /**
         * If the parameter is C1G2ReadOpSpecResult, i.e., Gen2.ReadData
         **/
        pReadOpSpecResult = (LLRP_tSC1G2ReadOpSpecResult *)pParameter;

        /* nValue is the length in 16-bit words */
        data->len = pReadOpSpecResult->ReadData.nValue * 2;
        copyLen = data->len;

        /* couple of validations and error checks */
        if (copyLen > data->max)
        {
          copyLen = data->max;
        }

        if (NULL != data->list)
        {
          /* Copy the data  */
          int i, j;

          for (i = 0, j = 0; i < copyLen; i += 2, j ++)
          {
            /* Hi byte */
            data->list[i] = pReadOpSpecResult->ReadData.pValue[j] >> 8;
            /* Lo byte */
            data->list[i + 1] = pReadOpSpecResult->ReadData.pValue[j] & 0xff;
          }
        }
        break;
      }

     /**
     * BLOCKPERMALOCK
     * 2 bytes of PermaLock data will be returned, on successful operation
     **/
      case TMR_LLRP_CUSTOM_BLOCKPERMALOCKOPSPECRESULT:
      {
        LLRP_tSThingMagicBlockPermalockOpSpecResult *pOpSpecResult;
        int copyLen;

        pOpSpecResult = (LLRP_tSThingMagicBlockPermalockOpSpecResult *)pParameter;

        /* nValue is the length in 16-bit words */
        data->len = pOpSpecResult->PermalockStatus.nValue * 2;
        copyLen = data->len;

        /* couple of validations and error checks */
        if (copyLen > data->max)
        {
          copyLen = data->max;
        }

        if (NULL != data->list)
        {
          /* Copy the data  */
          int i, j;

          for (i = 0, j = 0; i < copyLen; i += 2, j ++)
          {
            /* Hi byte */
            data->list[i + 1] = pOpSpecResult->PermalockStatus.pValue[j] >> 8;
            /* Lo byte */
            data->list[i] = pOpSpecResult->PermalockStatus.pValue[j] & 0xff;
          }
        }
        break;
      }

    /**
     * EASAlarm
     * 8 bytes of EAS alarm data will be returned, on successful operation
     **/
    case TMR_LLRP_CUSTOM_G2IEASALARMOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPG2IEASAlarmOpSpecResult *pResult;
        int copyLen;

        pResult = (LLRP_tSThingMagicNXPG2IEASAlarmOpSpecResult *)pParameter;
        
        /**
         * EASAlarm code is returned as uint8 list, so copy it directly
         **/
        data->len = pResult->EASAlarmCode.nValue;
        copyLen = data->len;

        /* couple of validations and error checks */
        if (copyLen > data->max)
        {
          copyLen = data->max;
        }

        if (NULL != data->list)
        {
          memcpy(data->list, pResult->EASAlarmCode.pValue, copyLen);
        }
        break;
      }

    /**
     * EASAlarm
     * 8 bytes of EAS alarm data will be returned, on successful operation
     **/
    case TMR_LLRP_CUSTOM_G2XEASALARMOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPG2XEASAlarmOpSpecResult *pResult;
        int copyLen;

        pResult = (LLRP_tSThingMagicNXPG2XEASAlarmOpSpecResult *)pParameter;
        
        /**
         * EASAlarm code is returned as uint8 list, so copy it directly
         **/
        data->len = pResult->EASAlarmCode.nValue;
        copyLen = data->len;

        /* couple of validations and error checks */
        if (copyLen > data->max)
        {
          copyLen = data->max;
        }

        if (NULL != data->list)
        {
          memcpy(data->list, pResult->EASAlarmCode.pValue, copyLen);
        }
        break;
      }

    /**
     * Calibrate
     * 64 bytes of calibration data will be
     * returned on a successful operation
     **/
    case TMR_LLRP_CUSTOM_G2ICALIBRATEOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPG2ICalibrateOpSpecResult *pResult;
        int copyLen;

        pResult = (LLRP_tSThingMagicNXPG2ICalibrateOpSpecResult *)pParameter;

        /**
         * Calibrate data is returned as uint8 list, so copy it directly
         **/
        data->len = pResult->CalibrateData.nValue;
        copyLen = data->len;
        /* couple of validations and error checks */
        if (copyLen > data->max)
        {
          copyLen = data->max;
        }

        if (NULL != data->list)
        {
          memcpy(data->list, pResult->CalibrateData.pValue, copyLen);
        }
        break;
      }

    /**
     * Calibrate
     * 64 bytes of calibration data will be
     * returned on a successful operation
     **/
    case TMR_LLRP_CUSTOM_G2XCALIBRATEOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPG2XCalibrateOpSpecResult *pResult;
        int copyLen;

        pResult = (LLRP_tSThingMagicNXPG2XCalibrateOpSpecResult *)pParameter;

        /**
         * Calibrate data is returned as uint8 list, so copy it directly
         **/
        data->len = pResult->CalibrateData.nValue;
        copyLen = data->len;
        /* couple of validations and error checks */
        if (copyLen > data->max)
        {
          copyLen = data->max;
        }

        if (NULL != data->list)
        {
          memcpy(data->list, pResult->CalibrateData.pValue, copyLen);
        }
        break;
      }

    /**
     * ChangeConfig
     * Returns current configword setting on the tag
     * on a successful operation
     **/
    case TMR_LLRP_CUSTOM_G2ICHANGECONFIGOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPG2IChangeConfigOpSpecResult *pResult;

        pResult = (LLRP_tSThingMagicNXPG2IChangeConfigOpSpecResult *)pParameter;

        /**
         * Configword is returned as uint16. So
         * send it as an array of two uint8's
         **/
        data->len = 2;
        /* couple of validations and error checks */
        if ((data->len < data->max) &&
            (NULL != data->list))
        {
          /* Hi byte */
          data->list[0] = pResult->ConfigData >> 8;
          /* Lo byte */
          data->list[1] = pResult->ConfigData & 0xff;
        }
        break;
      }

    /**
       * IDS GetBatteryLevel Command
       * Returns following fields on successful operation
       */
    case TMR_LLRP_CUSTOM_IDS_GETBATTERYLEVELOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900AGetBatteryLevelOpSpecResult *pResult;

        pResult = (LLRP_tSThingMagicIDSSL900AGetBatteryLevelOpSpecResult *)pParameter;

        /**
         * IDS GetBatteryLevel Data comes as byte array
         * send it as an array of  uint8's
         */
        data->len = pResult->pThingMagicIDSBatteryLevel->batteryValueByteStream.nValue;
        /* couple of validations and error checks */
        if (data->len > data->max)
        {
          data->len = data->max;
        }
        if (NULL != data->list)
        {
          memcpy (data->list, pResult->pThingMagicIDSBatteryLevel->batteryValueByteStream.pValue,
              (size_t)(pResult->pThingMagicIDSBatteryLevel->batteryValueByteStream.nValue * sizeof(uint8_t)));
        }
        break;
      }

    /**
     * IDS GetMeasurementSetup Command
     * returns following fields on successful operation
     */
    case TMR_LLRP_CUSTOM_IDS_GETMEASUREMENTSETUPOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900AGetMeasurementSetupOpSpecResult *pResult;

        pResult = (LLRP_tSThingMagicIDSSL900AGetMeasurementSetupOpSpecResult *)pParameter;

        /**
         * IDS GetMeasurementsetup Data comes as byte array
         * send it as an array of  uint8's
         */
        data->len = pResult->measurementByteStream.nValue;
        /* couple of validations and error checks */
        if (data->len > data->max)
        {
          data->len = data->max;
        }
        if (NULL != data->list)
        {
          memcpy (data->list, pResult->measurementByteStream.pValue,
              (size_t)(pResult->measurementByteStream.nValue * sizeof(uint8_t)));
        }
        break;
      }

    /**
     * QT ReadWrite
     * Returns the payload on a successful operation
     **/
    case TMR_LLRP_CUSTOM_MONZA4QTREADWRITEOPSPECRESULT:
      {
        LLRP_tSThingMagicImpinjMonza4QTReadWriteOpSpecResult *pResult;

        pResult = (LLRP_tSThingMagicImpinjMonza4QTReadWriteOpSpecResult *)pParameter;

        /**
         * Payload is returned as uint16. So
         * send it as an array of two uint8's
         **/
        data->len = 2;
        /* couple of validations and error checks */
        if ((data->len < data->max) &&
            (NULL != data->list))
        {
          /* Hi byte */
          data->list[0] = pResult->Payload >> 8;
          /* Lo byte */
          data->list[1] = pResult->Payload & 0xff;
        }
        break;
      }

      /**
       * IDS GetSensor Command
       * Returns following fields on a successful operation
       */
    case TMR_LLRP_CUSTOM_IDS_GETSENSORVALUEOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900ASensorValueOpSpecResult  *pResult;

        pResult = (LLRP_tSThingMagicIDSSL900ASensorValueOpSpecResult *)pParameter;

        /**
         * IDSGetSensor return value as uint16. So
         * send it as an array of two uint8's
         **/
        data->len = 2;
        /* couple of validations and error checks */
        if (data->len > data->max)
        {
          data->len = data->max;
        }

        if (NULL != data->list)
        {
          memcpy (data->list, pResult->SensorValueByteStream.pValue, (llrp_u16_t)data->len);
        }
        break;
      }

      /**
       * IDS GetLogState Command
       * Returns following fields on successful operation
       */ 
    case TMR_LLRP_CUSTOM_IDS_GETLOGSTATEOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900ALogStateOpSpecResult  *pResult;

        pResult = (LLRP_tSThingMagicIDSSL900ALogStateOpSpecResult *)pParameter;

        /**
         * IDS GetLogState Data comes as byte array
         * send it as an array of  uint8's
         **/
        data->len = pResult->LogStateByteStream.nValue;
        /* couple of validations and error checks */
        if (data->len > data->max)
        {
          data->len = data->max;
        }

        if (NULL != data->list)
        {
          memcpy (data->list, pResult->LogStateByteStream.pValue, (llrp_u16_t)data->len); 
        }
        break;
      }

      /**
       * IDS AcccessFifoStatus Command
       * Returns following fields on successful operation
       */
    case TMR_LLRP_CUSTOM_IDS_ACCESSFIFOSTATUSOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900AAccessFIFOStatusOpSpecResult  *pResult;

        pResult = (LLRP_tSThingMagicIDSSL900AAccessFIFOStatusOpSpecResult *)pParameter;

        /**
         * IDS AccessFifoStatus response comes as uint8_t value
         * copy the response
         */
        data->len = 1;
        /* couple of validations and error checks */
        if (data->len > data->max)
        {
          data->len = data->max;
        }

        if (NULL != data->list)
        {
          memcpy (data->list, &(pResult->FIFOStatusRawByte), (size_t)((data->len) * sizeof(uint8_t)));
        }
        break;
      }

      /**
       * IDS AccessFifoRead Command
       * Returns following fields on a successful operation
       */ 
    case TMR_LLRP_CUSTOM_IDS_ACCESSFIFOREADOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900AAccessFIFOReadOpSpecResult  *pResult;

        pResult = (LLRP_tSThingMagicIDSSL900AAccessFIFOReadOpSpecResult *)pParameter;

        /**
         * IDS AccessFifoRead response comes as byte array
         * sent it as array of uin8_t values
         */
        data->len = pResult->readPayLoad.nValue;
        /* couple of validations and error checks */
        if (data->len > data->max)
        {
          data->len = data->max;
        }

        if (NULL != data->list)
        {
          memcpy (data->list, pResult->readPayLoad.pValue, (size_t)(pResult->readPayLoad.nValue * sizeof(uint8_t)));
        }

        break;
      }

      /**
       * IDS GetCalibrationData Command
       * Returns following fields on a successful operation
       */
    case TMR_LLRP_CUSTOM_IDS_GETCALIBRATIONDATAOPSPECRESULT:
    {
    	LLRP_tSThingMagicIDSSL900AGetCalibrationDataOpSpecResult  *pResult;

      pResult = (LLRP_tSThingMagicIDSSL900AGetCalibrationDataOpSpecResult *)pParameter;

      /**
       * IDS GetCalibrationData Command response comes as byte array
       * sent it as array of uin8_t values
       */
      data->len = pResult->pThingMagicIDSCalibrationData->calibrationValueByteStream.nValue;
      /* couple of validations and error checks */
      if (data->len > data->max)
      {
        data->len = data->max;
      }

      if (NULL != data->list)
      {
        memcpy (data->list, pResult->pThingMagicIDSCalibrationData->calibrationValueByteStream.pValue,
        		(size_t)(pResult->pThingMagicIDSCalibrationData->calibrationValueByteStream.nValue * sizeof(uint8_t)));
      }

      break;
    }

    /**
     * DenatranIAV ActivateSecureMode Command
     * Returns following fields on successful operation
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_ACTIVATESECUREMODEOPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVActivateSecureModeOpSpecResult *pResult;

      pResult = (LLRP_tSThingMagicDenatranIAVActivateSecureModeOpSpecResult *)pParameter;

      /**
       * DenatranIAV ActivateSecureMode Command response comes as byte array
       * sent it as array of uin8_t values
       */
      data->len = pResult->ActivateSecureModeByteStream.nValue;
      /* couple of validations and error checks */
      if (data->len > data->max)
      {
        data->len = data->max;
      }

      if (NULL != data->list)
      {
        memcpy (data->list, pResult->ActivateSecureModeByteStream.pValue,
            (size_t)(pResult->ActivateSecureModeByteStream.nValue * sizeof(uint8_t)));
      }

      break;
    }

    /**
     * DenatranIAV OuthenticateOBU Command
     * Returns following fields on successful operation
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_AUTHENTICATEOBUOPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVAuthenticateOBUOpSpecResult *pResult;

      pResult = (LLRP_tSThingMagicDenatranIAVAuthenticateOBUOpSpecResult *)pParameter;

      /**
       * DenatranIAV AuthenticateOBU Command response comes as byte array
       * sent it as array of uin8_t values
       */
      data->len = pResult->AuthenitcateOBUByteStream.nValue;
      /* couple of validations and error checks */
      if (data->len > data->max)
      {
        data->len = data->max;
      }

      if (NULL != data->list)
      {
        memcpy (data->list, pResult->AuthenitcateOBUByteStream.pValue,
            (size_t)(pResult->AuthenitcateOBUByteStream.nValue * sizeof(uint8_t)));
      }

      break;
    }

    /**
     * DenatranIAV ActivateSiniavMode Command
     * Returns following fields on successful operation
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_ACTIVATESINIAVMODEOPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVActivateSiniavModeOpSpecResult *pResult;

      pResult = (LLRP_tSThingMagicDenatranIAVActivateSiniavModeOpSpecResult *)pParameter;

      /**
       * DenatranIAV ActivateSiniavMode Command response comes as byte array
       * sent it as array of uin8_t values
       */
      data->len = pResult->ActivateSiniavModeByteStream.nValue;
      /* couple of validations and error checks */
      if (data->len > data->max)
      {
        data->len = data->max;
      }

      if (NULL != data->list)
      {
        memcpy (data->list, pResult->ActivateSiniavModeByteStream.pValue,
            (size_t)(pResult->ActivateSiniavModeByteStream.nValue * sizeof(uint8_t)));
      }

      break;
    }

    /**
     * DenatranIAV OBUAuthenticateID Command
     * Returns following fields on successful operation
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_AUTHENTICATEIDOPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVOBUAuthenticateIDOpSpecResult *pResult;

      pResult = (LLRP_tSThingMagicDenatranIAVOBUAuthenticateIDOpSpecResult *)pParameter;

      /**
       * DenatranIAV OBUAuthenticateID Command response comes as byte array
       * sent it as array of uin8_t values
       */
      data->len = pResult->OBUAuthenticateIDByteStream.nValue;
      /* couple of validations and error checks */
      if (data->len > data->max)
      {
        data->len = data->max;
      }

      if (NULL != data->list)
      {
        memcpy (data->list, pResult->OBUAuthenticateIDByteStream.pValue,
            (size_t)(pResult->OBUAuthenticateIDByteStream.nValue * sizeof(uint8_t)));
      }

      break;
    }

    /**
     * DenatranIAV AuthenticateOBU FullPass1 Command
     * Returns following fields on successful operation
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_AUTHENTICATEFULLPASS1OPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVOBUAuthenticateFullPass1OpSpecResult *pResult;

      pResult = (LLRP_tSThingMagicDenatranIAVOBUAuthenticateFullPass1OpSpecResult *)pParameter;

      /**
       * DenatranIAV AuthenticateOBU FullPass1 Command response comes as byte array
       * sent it as array of uin8_t values
       */
      data->len = pResult->OBUAuthenticateFullPass1ByteStream.nValue;
      /* couple of validations and error checks */
      if (data->len > data->max)
      {
        data->len = data->max;
      }

      if (NULL != data->list)
      {
        memcpy (data->list, pResult->OBUAuthenticateFullPass1ByteStream.pValue,
            (size_t)(pResult->OBUAuthenticateFullPass1ByteStream.nValue * sizeof(uint8_t)));
      }

      break;
    }

    /**
     * DenatranIAV AuthenticateOBU FullPass2 Command
     * Returns following fields on successful operation
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_AUTHENTICATEFULLPASS2OPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVOBUAuthenticateFullPass2OpSpecResult *pResult;

      pResult = (LLRP_tSThingMagicDenatranIAVOBUAuthenticateFullPass2OpSpecResult *)pParameter;

      /**
       * DenatranIAV AuthenticateOBU FullPass2 Command response comes as byte array
       * sent it as array of uin8_t values
       */
      data->len = pResult->OBUAuthenticateFullPass2ByteStream.nValue;
      /* couple of validations and error checks */
      if (data->len > data->max)
      {
        data->len = data->max;
      }

      if (NULL != data->list)
      {
        memcpy (data->list, pResult->OBUAuthenticateFullPass2ByteStream.pValue,
            (size_t)(pResult->OBUAuthenticateFullPass2ByteStream.nValue * sizeof(uint8_t)));
      }

      break;
    }

    /**
     * DenatranIAV OBU ReadFromMemMap Command
     * Returns following fields on successful operation
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_OBUREADFROMMEMMAPOPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVOBUReadFromMemMapOpSpecResult *pResult;

      pResult = (LLRP_tSThingMagicDenatranIAVOBUReadFromMemMapOpSpecResult *)pParameter;

      /**
       * DenatranIAV OBU ReadFromMemMap Command response comes as byte array
       * sent it as array of uin8_t values
       */
      data->len = pResult->OBUReadMemoryMapByteStream.nValue;
      /* couple of validations and error checks */
      if (data->len > data->max)
      {
        data->len = data->max;
      }

      if (NULL != data->list)
      {
        memcpy (data->list, pResult->OBUReadMemoryMapByteStream.pValue,
            (size_t)(pResult->OBUReadMemoryMapByteStream.nValue * sizeof(uint8_t)));
      }

      break;
    }

    /**
     * DenatranIAV OBU WriteMemoryMap Command
     * Returns following fields on successful operation
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_OBUWRITETOMEMMAPOPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVOBUWriteToMemMapOpSpecResult *pResult;

      pResult = (LLRP_tSThingMagicDenatranIAVOBUWriteToMemMapOpSpecResult *)pParameter;

      /**
       * DenatranIAV OBU WriteToMemMap Command response comes as byte array
       * sent it as array of uin8_t values
       */
      data->len = pResult->OBUWriteMemoryMapByteStream.nValue;
      /* couple of validations and error checks */
      if (data->len > data->max)
      {
        data->len = data->max;
      }

      if (NULL != data->list)
      {
        memcpy (data->list, pResult->OBUWriteMemoryMapByteStream.pValue,
            (size_t)(pResult->OBUWriteMemoryMapByteStream.nValue * sizeof(uint8_t)));
      }

      break;
    }

    /**
     * ThingMagic DenatranIAV GetTokenID result
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_GETTOKENIDOPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVGetTokenIDOpSpecResult *pResult;

      pResult = (LLRP_tSThingMagicDenatranIAVGetTokenIDOpSpecResult *)pParameter;

      /**
       * DenatranIAV GetTokenID Command response comes as byte array
       * sent it as array of uin8_t values
       */
      data->len = pResult->GetTokenIDByteStream.nValue;
      /* couple of validations and error checks */
      if (data->len > data->max)
      {
        data->len = data->max;
      }

      if (NULL != data->list)
      {
        memcpy (data->list, pResult->GetTokenIDByteStream.pValue,
            (size_t)(pResult->GetTokenIDByteStream.nValue * sizeof(uint8_t)));
      }

      break;
    }

    /**
     * ThingMagic DenatranIAV OBUAuthenticateFullPass result
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_OBUAUTHFULLPASSOPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVOBUAuthFullPassOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicDenatranIAVOBUAuthFullPassOpSpecResult *)pParameter;

      /**
       * DenatranIAV OBUAuthFullPass Command response comes as byte array
       * sent it as array of uin8_t values
       */
      data->len = pResult->OBUAuthFullPassByteStream.nValue;
      /* couple of validations and error checks */
      if (data->len > data->max)
      {
        data->len = data->max;
      }

      if (NULL != data->list)
      {
        memcpy (data->list, pResult->OBUAuthFullPassByteStream.pValue,
            (size_t)(pResult->OBUAuthFullPassByteStream.nValue * sizeof(uint8_t)));
      }

      break;
    }

    /**
     * ThingMagic DenatranIAV ReadSec result
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_READSECTOROPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVReadSecOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicDenatranIAVReadSecOpSpecResult *)pParameter;

      /**
       * DenatranIAV ReadSec Command response comes as byte array
       * sent it as array of uin8_t values
       */
      data->len = pResult->ReadSecByteStream.nValue;
      /* couple of validations and error checks */
      if (data->len > data->max)
      {
        data->len = data->max;
      }

      if (NULL != data->list)
      {
        memcpy (data->list, pResult->ReadSecByteStream.pValue,
            (size_t)(pResult->ReadSecByteStream.nValue * sizeof(uint8_t)));
      }

      break;
    }

    /**
     * ThingMagic DenatranIAV WriteSec result
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_WRITESECTOROPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVWriteSecOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicDenatranIAVWriteSecOpSpecResult *)pParameter;

      /**
       * DenatranIAV WriteSec Command response comes as byte array
       * sent it as array of uin8_t values
       */
      data->len = pResult->WriteSecByteStream.nValue;
      /* couple of validations and error checks */
      if (data->len > data->max)
      {
        data->len = data->max;
      }

      if (NULL != data->list)
      {
        memcpy (data->list, pResult->WriteSecByteStream.pValue,
            (size_t)(pResult->WriteSecByteStream.nValue * sizeof(uint8_t)));
      }

      break;
    }

    /**
     * ThingMagic DenatranIAV G0PAOBUAuth result
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_G0PAOBUAUTHOPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVG0PAOBUAuthOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicDenatranIAVG0PAOBUAuthOpSpecResult *)pParameter;

      /**
       * DenatranIAV G0PAOBUAuth Command response comes as byte array
       * sent it as array of uin8_t values
       */
      data->len = pResult->G0PAOBUAuthByteStream.nValue;
      /* couple of validations and error checks */
      if (data->len > data->max)
      {
        data->len = data->max;
      }

      if (NULL != data->list)
      {
        memcpy (data->list, pResult->G0PAOBUAuthByteStream.pValue,
            (size_t)(pResult->G0PAOBUAuthByteStream.nValue * sizeof(uint8_t)));
      }

      break;
    }

      /**
       * NXP Authentication Command
       * Returns following fields on a successful operation
       */
    case TMR_LLRP_CUSTOM_NXP_AUTHENTICATEOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPAuthenticationOpSpecResult *pResult;

        pResult = (LLRP_tSThingMagicNXPAuthenticationOpSpecResult *)pParameter;

        data->len = pResult->NXPAuthenticationByteStream.nValue;
        if (data->len > data->max) {
          data->len = data->max;
        }

        if (NULL != data->list) {
          memcpy (data->list, pResult->NXPAuthenticationByteStream.pValue,
              (size_t)(data->len));
        }
        break;
      }

      /**
       * NXP Readbuffer Command
       * Returns following fields on a successful operation
       */
    case TMR_LLRP_CUSTOM_NXP_READBUFFEROPSPECRESULT:
      {
        LLRP_tSThingMagicNXPReadbufferOpSpecResult *pResult;

        pResult = (LLRP_tSThingMagicNXPReadbufferOpSpecResult *)pParameter;

        data->len = pResult->NXPReadbufferByteStream.nValue;
        if (data->len > data->max) {
          data->len = data->max;
        }

        if (NULL != data->list) {
          memcpy (data->list, pResult->NXPReadbufferByteStream.pValue,
              (size_t)(data->len));
        }
        break;
      }

      /**
       * NXP Untraceable Command
       * Returns following fields on a successful operation
       */
    case TMR_LLRP_CUSTOM_NXP_UNTRACEABLEOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPUntraceableOpSpecResult *pResult;

        pResult = (LLRP_tSThingMagicNXPUntraceableOpSpecResult *)pParameter;

        data->len = pResult->NXPUntraceableByteStream.nValue;
        if (data->len > data->max) {
          data->len = data->max;
        }

        if (NULL != data->list) {
          memcpy (data->list, pResult->NXPUntraceableByteStream.pValue,
              (size_t)(data->len));
        }
        break;
      }

#ifdef TMR_ENABLE_ISO180006B
    case TMR_LLRP_CUSTOM_ISO_READDATAOPSPECRESULT:
      {
        LLRP_tSThingMagicISO180006BReadOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicISO180006BReadOpSpecResult *)pParameter;
        memcpy(data->list, pResult->ReadData.pValue, (pResult->ReadData.nValue * sizeof(uint8_t)));

        break; 
      }
#endif /* TMR_ENABLE_ISO180006B */

    default:
      {
        /**
         * Tag operations other than the above might not return
         * any data. Do nothing here.
         **/
      }
  }
}

/**
 * Command to get ThingMagic DeDuplication
 * @param reader Reader pointer
 * @param duplication pointer to TMR_LLRP_TMDeDuplication
 * to hold the value of ThingMagic DeDuplication parameter
 */
TMR_Status
TMR_LLRP_cmdGetThingMagicDeDuplication(TMR_Reader *reader, TMR_LLRP_TMDeDuplication *duplication)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration  *pTMDeDuplication;
  LLRP_tSParameter                      *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd,
      LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * Thingmagic DeDuplication  is available as a custom parameter under
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMDeDuplication = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMDeDuplication)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., reader DeDuplication)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMDeDuplication,
      LLRP_ThingMagicControlConfiguration_ThingMagicDeDuplication);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMDeDuplication->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMDeDuplication);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }

  /**
   * Response is success, extract reader configuration from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    /* Get the Record Highest RSSI */
    duplication->highestRSSI = LLRP_ThingMagicDeDuplication_getRecordHighestRSSI((LLRP_tSThingMagicDeDuplication*)pCustParam);

    /* Get the Unique By antenna */
    duplication->uniquebyAntenna = LLRP_ThingMagicDeDuplication_getUniqueByAntenna((LLRP_tSThingMagicDeDuplication*)pCustParam);

    /* Get the Unique By data */
    duplication->uniquebyData = LLRP_ThingMagicDeDuplication_getUniqueByData((LLRP_tSThingMagicDeDuplication*)pCustParam);
  }
  else
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to set ThingMagic DeDuplication
 * @param reader Reader pointer
 * @param duplication pointer to TMR_LLRP_TMDeDuplication
 * to hold the value of ThingMagic DeDuplication parameter
 */
TMR_Status
TMR_LLRP_cmdSetThingMagicDeDuplication(TMR_Reader *reader, TMR_LLRP_TMDeDuplication *duplication)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSThingMagicDeDuplication        *pTMDeDuplication;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic DeDuplication is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize DeDuplication */
  pTMDeDuplication = LLRP_ThingMagicDeDuplication_construct();

  /* set the highestRSSI to DeDuplication */
  pTMDeDuplication->RecordHighestRSSI = duplication->highestRSSI;

  /* set the unique by antenna to DeDuplication */
  pTMDeDuplication->UniqueByAntenna = duplication->uniquebyAntenna;

  /* set the unique by data to DeDuplication */
  pTMDeDuplication->UniqueByData = duplication->uniquebyData;

  /* Add DeDuplication as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pTMDeDuplication);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;

}

void 
TMR_LLRP_freeTMReaderConfiguration(TMR_LLRP_TMReaderConfiguration *config)
{
  if (NULL != config->description.pValue)
  {
    free(config->description.pValue);
  }

  if (NULL != config->role.pValue)
  {
    free(config->role.pValue);
  }
}


/**
 * Command to get Thingmagic Reader configuration
 * It is the user responsibility to free the "config" after 
 * a successful get (Call TMR_LLRP_freeTMReaderConfiguration to free)
 *
 * @param reader Reader pointer
 * @param[out] config Pointer to TMR_LLRP_TMReaderConfiguration which holds the configuration
 */
TMR_Status
TMR_LLRP_cmdGetThingmagicReaderConfiguration(TMR_Reader *reader, 
                       TMR_LLRP_TMReaderConfiguration *config)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration  *pTMReaderConfig;
  LLRP_tSParameter                      *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, 
                        LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * Thingmagic Reader configuration is available as a custom parameter under 
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMReaderConfig = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMReaderConfig)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., reader configuration)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMReaderConfig, 
            LLRP_ThingMagicControlConfiguration_ThingMagicReaderConfiguration);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMReaderConfig->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMReaderConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract reader configuration from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    /* Get description */
    {
      llrp_utf8v_t description;
      description = LLRP_ThingMagicReaderConfiguration_getReaderDescription((LLRP_tSThingMagicReaderConfiguration*) pCustParam);
      config->description = LLRP_utf8v_copy(description);

    }
    /* Get Role */
    {
      llrp_utf8v_t role;
      role = LLRP_ThingMagicReaderConfiguration_getReaderRole((LLRP_tSThingMagicReaderConfiguration*) pCustParam);
      config->role = LLRP_utf8v_copy(role);
    }
    /* Get host name */
    {
      llrp_utf8v_t hostname;
      hostname = LLRP_ThingMagicReaderConfiguration_getReaderHostName((LLRP_tSThingMagicReaderConfiguration*) pCustParam);
      config->hostName = LLRP_utf8v_copy(hostname);
    }
  }
  else
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }
  
  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to set Thingmagic reader configuration
 *
 * @param reader Reader pointer
 * @param config[in] Pointer to TMR_LLRP_TMReaderConfiguration to be set
 */
TMR_Status
TMR_LLRP_cmdSetThingmagicReaderConfiguration(TMR_Reader *reader, 
                                  TMR_LLRP_TMReaderConfiguration *config)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSThingMagicReaderConfiguration  *pReaderConfig;
  
  ret = TMR_SUCCESS;
  /**
   * Thingmagic Reader configuration is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize Reader configuration */
  pReaderConfig = LLRP_ThingMagicReaderConfiguration_construct();

  /* Set description to reader config */
  pReaderConfig->ReaderDescription = LLRP_utf8v_copy(config->description);
  /* Set Reader role */
  pReaderConfig->ReaderRole = LLRP_utf8v_copy(config->role);
  /* Set reader host name */
  pReaderConfig->ReaderHostName = LLRP_utf8v_copy(config->hostName);

  /* Add ThingMagic reader configuration as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pReaderConfig);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to get Thingmagic Current Time
 *
 * @param reader Reader pointer
 * @param curTime Pointer to TMR_String  to
 *  hold the value of Thingmagic Current Time
 */
TMR_Status
TMR_LLRP_cmdGetThingMagicCurrentTime(TMR_Reader *reader, struct tm *curTime)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration  *pTMCurrentTime;
  LLRP_tSParameter                      *pCustParam;
  
  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd,
      LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * Thingmagic Current Time  is available as a custom parameter under
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMCurrentTime = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMCurrentTime)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., Current Time)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMCurrentTime,
      LLRP_ThingMagicControlConfiguration_ThingMagicCurrentTime);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMCurrentTime->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMCurrentTime);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }

  /**
   * Response is success, extract current  time  from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    /* Get the reader current time */
    {
      llrp_utf8v_t readerCT;
      readerCT = LLRP_ThingMagicCurrentTime_getReaderCurrentTime ((LLRP_tSThingMagicCurrentTime *) pCustParam);
#ifndef WIN32
	  /* Until a satisfactory Windows replacement for strptime can be found, just stub out this call (and always fail).
	   * Release note that LLRP GetThingMagicCurrentTime is not supported on Windows. */
	  if (!strptime((const char *)readerCT.pValue, "%Y-%m-%dT%H:%M:%S", curTime))
      {
        return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
      }
#else
      {
        return TMR_ERROR_UNSUPPORTED;
      }
#endif
    }
  }
  else
  {
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}


/**
 * Command to get Thingmagic Reader Module Temperature
 *
 * @param reader Reader pointer
 * @param readerTemp Pointer to uint8_t  to
 *  hold the value of Thingmagic Reader Module Temperature
 */
TMR_Status
TMR_LLRP_cmdGetThingMagicReaderModuleTemperature(TMR_Reader *reader, int8_t *readerTemp)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration  *pTMReaderTemp;
  LLRP_tSParameter                      *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd,
      LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * Thingmagic Reader Module Temperature  is available
   * as a custom parameter under
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMReaderTemp = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMReaderTemp)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e. Reader Temperature,)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMReaderTemp,
      LLRP_ThingMagicControlConfiguration_ThingMagicReaderModuleTemperature);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMReaderTemp->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMReaderTemp);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }

  /**
   * Response is success, extract reader temperature  from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    /* get the reader module temperature */
    llrp_s8_t temp;
    temp = LLRP_ThingMagicReaderModuleTemperature_getReaderModuleTemperature ((
         LLRP_tSThingMagicReaderModuleTemperature *) pCustParam);

   *(int8_t *)readerTemp = (int8_t)temp;
  }
  else
  {
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to get Thingmagic Antenna Detection
 *
 * @param reader Reader pointer
 * @param antennaport Pointer to bool  to
 *  hold the value of Thingmagic Antenna  Detection
 */
TMR_Status
TMR_LLRP_cmdGetThingMagicAntennaDetection(TMR_Reader *reader, bool *antennaport)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration  *pTMAntennaDetection;
  LLRP_tSParameter                      *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd,
      LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * Thingmagic Antenna Detection   is available
   * as a custom parameter under
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMAntennaDetection = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMAntennaDetection)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e. Antenna Detection,)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMAntennaDetection,
      LLRP_ThingMagicControlConfiguration_ThingMagicAntennaDetection);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMAntennaDetection->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMAntennaDetection);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }

  /**
   * Response is success, extract antenna detection  from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    /* get the antenna checkport */
    llrp_u1_t temp;
    temp = LLRP_ThingMagicAntennaDetection_getAntennaDetection ((
          LLRP_tSThingMagicAntennaDetection *) pCustParam);

    *(bool*)antennaport = (bool)temp;
  }
  else
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;

}

/**
 * Command to set Thingmagic Antenna Detection 
 *
 * @param reader Reader pointer
 * @param antennaport Pointer to bool  to
 *  hold the value of Thingmagic Antenna  Detection 
 */
TMR_Status
TMR_LLRP_cmdSetThingMagicAntennaDetection(TMR_Reader *reader, bool *antennaport)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSThingMagicAntennaDetection        *pTMAntennaDetection;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic Antenna Detection is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize Antenna Detection */
  pTMAntennaDetection = LLRP_ThingMagicAntennaDetection_construct();

  /* set  Antenna Detction */
  pTMAntennaDetection->AntennaDetection = *(bool*)antennaport;

  /* Add Antenna Detection as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pTMAntennaDetection);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;

}

/**
 * Command to get Thingmagic Gen2 protocol Extension
 *
 * @param reader Reader pointer
 * @param protocolExt Pointer to bool  to
 *  hold the value of Thingmagic gen2 protocol extension
 */
TMR_Status
TMR_LLRP_cmdGetThingMagicGEN2ProtocolExtension(TMR_Reader *reader, uint8_t *protocolExt)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration  *pTMGEN2ProtocolExtension;
  LLRP_tSParameter                      *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd,
      LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * Thingmagic GEn2 protocol Extension is available
   * as a custom parameter under
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMGEN2ProtocolExtension = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMGEN2ProtocolExtension)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e. GEN2 protocol Extension,)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMGEN2ProtocolExtension,
      LLRP_ThingMagicControlConfiguration_ThingMagicGEN2ProtocolExtension);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMGEN2ProtocolExtension->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMGEN2ProtocolExtension);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }

  /**
   * Response is success, extract GEN2 protocol extension from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    /* get the GEN2 protocol extension */
    llrp_u8_t temp;
    temp = LLRP_ThingMagicGEN2ProtocolExtension_getGEN2ProtocolExtension ((
          LLRP_tSThingMagicGEN2ProtocolExtension *) pCustParam);
    switch(temp)
    {
      case 1:
        *(uint8_t*)protocolExt = TMR_GEN2_PROTOCOLEXTENSION_LICENSE_IAV_DENATRAN; 
        break;
      default:
        *(uint8_t*)protocolExt = TMR_GEN2_PROTOCOLEXTENSION_LICENSE_NONE; 
        break;
    }
  }
  else
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;

}

/**
 * Command to get Thingmagic Device Protocol Capabilities
 *
 * @param reader Reader pointer
 * @param protocol Pointer to TMR_TagProtocolList  to
 *  hold the value of Thingmagic Device protocol Capabilities
 */
TMR_Status
TMR_LLRP_cmdGetTMDeviceProtocolCapabilities(TMR_Reader *reader, TMR_TagProtocolList *protocolList)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CAPABILITIES              *pCmd;
  LLRP_tSMessage                              *pCmdMsg;
  LLRP_tSMessage                              *pRspMsg;
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE     *pRsp;
  LLRP_tSThingMagicDeviceControlCapabilities  *pTMCaps;
  LLRP_tSParameter                            *pCustParam;
  LLRP_tSSupportedProtocols                   *pSupportedProtocols;
  uint8_t                                     i;

  ret = TMR_SUCCESS;

  /**
   * Initialize GET_READER_CAPABILITIES message
   **/
  pCmd = LLRP_GET_READER_CAPABILITIES_construct();
  LLRP_GET_READER_CAPABILITIES_setRequestedData(pCmd, LLRP_GetReaderCapabilitiesRequestedData_General_Device_Capabilities);

  /**
   * /reader/version/supportedProtocols is a custom parameter.And is available as part of
   * ThingMagicDeviceControlCapabilities.ThingMagicControlCapabilities.DeviceProtocolCapabilities
   **/
  pTMCaps = LLRP_ThingMagicDeviceControlCapabilities_construct();
  if (NULL == pTMCaps)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * set the requested data
   * and add to GET_READER_CAPABILITIES message
   **/
  LLRP_ThingMagicDeviceControlCapabilities_setRequestedData(pTMCaps, LLRP_ThingMagicControlCapabilities_DeviceProtocolCapabilities);
  if (LLRP_RC_OK != LLRP_GET_READER_CAPABILITIES_addCustom(pCmd, &pTMCaps->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMCaps);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }
  pCmdMsg       = &pCmd->hdr;

  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CAPABILITIES_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }
  /**
   * Response is success
   * Extract supported protocol  from it
   **/
  pCustParam = LLRP_GET_READER_CAPABILITIES_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    protocolList->len = 0;

    for (pSupportedProtocols = LLRP_DeviceProtocolCapabilities_beginSupportedProtocols(
          (LLRP_tSDeviceProtocolCapabilities *) pCustParam), i = 0;
        (NULL != pSupportedProtocols);
        pSupportedProtocols = LLRP_DeviceProtocolCapabilities_nextSupportedProtocols(
          pSupportedProtocols), i ++)
    {
      protocolList->list[i] = LLRP_SupportedProtocols_getProtocol(pSupportedProtocols);
      /**
       *Adding support for ATA, IPX64 and IPX256 protocols 
       **/
      if (TMR_TAG_PROTOCOL_GEN2 == protocolList->list[i] || 
          TMR_TAG_PROTOCOL_ISO180006B == protocolList->list[i] || 
          TMR_TAG_PROTOCOL_ATA == protocolList->list[i] ||
          TMR_TAG_PROTOCOL_IPX64 == protocolList->list[i] ||
          TMR_TAG_PROTOCOL_IPX256 == protocolList->list[i]) 
        reader->u.llrpReader.supportedProtocols |= (1 << (protocolList->list[i] -1 ));
      protocolList->len ++;
    }

  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;

}

/**
 * Command to get active RFControl
 *
 * @param reader Reader pointer
 * @param rfControl[out] Pointer to TMR_LLRP_RFControl
 */
TMR_Status 
TMR_LLRP_cmdGetActiveRFControl(TMR_Reader *reader, TMR_LLRP_RFControl *rfControl)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSAntennaConfiguration           *pAntConfig;

  ret = TMR_SUCCESS;
  /**
   * RFControl can be retrieved
   * through  LLRP standard parameter GET_READER_CONFIG_RESPONSE.AntennaConfiguration.
   * airProtocolInventoryCommand.RFControl
   * Initialize GET_READER_CONFIG message 
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_AntennaConfiguration);

  /* Get antenna configuration for all antennas*/
  LLRP_GET_READER_CONFIG_setAntennaID(pCmd, 0);

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract mode index from it
   **/
  for (pAntConfig = LLRP_GET_READER_CONFIG_RESPONSE_beginAntennaConfiguration(pRsp);
      (pAntConfig != NULL);
      pAntConfig = LLRP_GET_READER_CONFIG_RESPONSE_nextAntennaConfiguration(pAntConfig))
  {
    LLRP_tSParameter *pInventoryCommand;

    for (pInventoryCommand = LLRP_AntennaConfiguration_beginAirProtocolInventoryCommandSettings(pAntConfig);
        (pInventoryCommand != NULL);
        pInventoryCommand = LLRP_AntennaConfiguration_nextAirProtocolInventoryCommandSettings(pInventoryCommand))
    {
      LLRP_tSC1G2RFControl *pRFControl;
      pRFControl = LLRP_C1G2InventoryCommand_getC1G2RFControl(
          (LLRP_tSC1G2InventoryCommand *)pInventoryCommand);
      rfControl->index = pRFControl->ModeIndex;

      /* Convert Tari value */
      switch (pRFControl->Tari)
      {
        case 25000:
          rfControl->tari = TMR_GEN2_TARI_25US;
          break;

        case 12500:
          rfControl->tari = TMR_GEN2_TARI_12_5US;
          break;

        case 6250:
          rfControl->tari = TMR_GEN2_TARI_6_25US;
          break;

        default:
          rfControl->tari = TMR_GEN2_TARI_INVALID;
      }
    }
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to set active RFControl
 *
 * @param reader Reader pointer
 * @param rfControl[in] Pointer to TMR_LLRP_RFControl
 */
TMR_Status 
TMR_LLRP_cmdSetActiveRFControl(TMR_Reader *reader, TMR_LLRP_RFControl *rfControl)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSAntennaConfiguration           *pAntConfig;
  LLRP_tSC1G2RFControl                  *pRFControl;
  LLRP_tSC1G2InventoryCommand           *pInventoryCommand;


  ret = TMR_SUCCESS;
  /**
   * RFControl can be set
   * through  LLRP standard parameter GET_READER_CONFIG_RESPONSE.AntennaConfiguration.
   * airProtocolInventoryCommand.RFControl
   * Initialize SET_READER_CONFIG message 
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize antenna configuration */
  pAntConfig = LLRP_AntennaConfiguration_construct();
  LLRP_AntennaConfiguration_setAntennaID(pAntConfig, 0); /* For all antennas */

  /* Initialize Inventory Command */
  pInventoryCommand = LLRP_C1G2InventoryCommand_construct();

  /* Initialize RFControl */
  pRFControl = LLRP_C1G2RFControl_construct();
  pRFControl->ModeIndex = rfControl->index;

  /* Convert Tari into nano seconds */
  switch (rfControl->tari)
  {
    case TMR_GEN2_TARI_25US:
      pRFControl->Tari = 25000;
      break;

    case TMR_GEN2_TARI_12_5US:
      pRFControl->Tari = 12500;
      break;

    case TMR_GEN2_TARI_6_25US:
      pRFControl->Tari = 6250;
      break;

    default:
      /**
       * FIXME: should we return error in case of default value
       * or send Tari as 0? Considering 0 as default value
       **/
      pRFControl->Tari = 0;
  }

  /* Set RFControl to inventoryCommand */
  LLRP_C1G2InventoryCommand_setC1G2RFControl(pInventoryCommand, pRFControl);

  /* Set InventoryCommand to antenna configuration */
  LLRP_AntennaConfiguration_addAirProtocolInventoryCommandSettings(
      pAntConfig, (LLRP_tSParameter *)pInventoryCommand);

  /* Set antenna configuration to SET_READER_CONFIG */
  LLRP_SET_READER_CONFIG_addAntennaConfiguration(pCmd, pAntConfig);

  pCmdMsg       = &pCmd->hdr;

  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to get Gen2 Q value
 *
 * @param reader Reader pointer
 * @param[out] q Pointer to TMR_GEN2_Q object to hold the region value
 */
TMR_Status
TMR_LLRP_cmdGetGen2Q(TMR_Reader *reader, TMR_GEN2_Q *q)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG                    *pCmd;
  LLRP_tSMessage                              *pCmdMsg;
  LLRP_tSMessage                              *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE           *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration *pTMProtoConfig;
  LLRP_tSParameter                            *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/gen2/q" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMProtoConfig = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMProtoConfig)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., protocol configuration)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMProtoConfig, 
        LLRP_ThingMagicControlConfiguration_ThingMagicProtocolConfiguration);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMProtoConfig->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMProtoConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract Gen2 Q from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    LLRP_tSGen2CustomParameters *gen2Custom;
    LLRP_tSGen2Q *gen2Q;

    gen2Custom = LLRP_ThingMagicProtocolConfiguration_getGen2CustomParameters(
                        (LLRP_tSThingMagicProtocolConfiguration *)pCustParam);
    gen2Q = LLRP_Gen2CustomParameters_getGen2Q(gen2Custom);

    if(gen2Q->eGen2QType)
    {
      /* If static Q, then get the q value */
      q->type = TMR_SR_GEN2_Q_STATIC;
      q->u.staticQ.initialQ = gen2Q->InitQValue;
    }
    else
    {
      q->type = TMR_SR_GEN2_Q_DYNAMIC;
    }
  }
  else
  {
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }
  
  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to Set Gen2 Q value
 *
 * @param reader Reader pointer
 * @param[in] q Pointer to TMR_GEN2_Q object which needs to be set
 */
TMR_Status
TMR_LLRP_cmdSetGen2Q(TMR_Reader *reader, TMR_GEN2_Q *q)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                *pCmd;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSMessage                          *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE       *pRsp;
  LLRP_tSThingMagicProtocolConfiguration  *pProtoConfig;
  LLRP_tSGen2CustomParameters             *pGen2Custom;
  LLRP_tSGen2Q                            *pGen2Q;
  
  ret = TMR_SUCCESS;
  /**
   * Thingmagic Protocol configuration is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize Protocol configuration */
  pProtoConfig = LLRP_ThingMagicProtocolConfiguration_construct();

  /* Initialize gen2 Q */
  pGen2Q = LLRP_Gen2Q_construct();
  pGen2Q->eGen2QType = q->type;

  if (TMR_SR_GEN2_Q_STATIC == q->type)
  {
    /* If static q, set the value */
    pGen2Q->InitQValue = q->u.staticQ.initialQ;
  }
  else
  {
    /* If dynamic, set initQ value to 0 */
    pGen2Q->InitQValue = 0;
  }

  /* Initialize gen2 custom parameter */
  pGen2Custom = LLRP_Gen2CustomParameters_construct();
  LLRP_Gen2CustomParameters_setGen2Q(pGen2Custom, pGen2Q);

  /* Set Gen2 Custom parameter to protocol configuration */
  LLRP_ThingMagicProtocolConfiguration_setGen2CustomParameters(pProtoConfig,
                                                                pGen2Custom);
    
  /* Add ThingMagic Protocol configuration as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pProtoConfig);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

 /**
 * Command to get sendSelect value
 *
 * @param reader Reader pointer
 * @param[out] select Pointer to hold the select value
 */
TMR_Status
TMR_LLRP_cmdGetsendSelect(TMR_Reader *reader, bool *select)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG                    *pCmd;
  LLRP_tSMessage                              *pCmdMsg;
  LLRP_tSMessage                              *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE           *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration *pTMProtoConfig;
  LLRP_tSParameter                            *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/gen2/sendSelect" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMProtoConfig = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMProtoConfig)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., protocol configuration)
   * And add to GET_READER_CONFIG message.
   **/
 LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMProtoConfig,
        LLRP_ThingMagicControlConfiguration_ThingMagicProtocolConfiguration);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMProtoConfig->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMProtoConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }
  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }

  /**
   * Response is success, extract sendSelect from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    LLRP_tSGen2CustomParameters *gen2Custom;
    LLRP_tSsendSelect *sendSelect;

    gen2Custom = LLRP_ThingMagicProtocolConfiguration_getGen2CustomParameters(
                        (LLRP_tSThingMagicProtocolConfiguration *)pCustParam);
    sendSelect = LLRP_Gen2CustomParameters_getsendSelect(gen2Custom);
    if(sendSelect != NULL)
    {    
      *select = sendSelect->selectValue;
    }
  }
  else
  {
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
 }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to Set sendSelect value
 *
 * @param reader Reader pointer
 * @param[in] Select Pointer to hold the value of select which needs to be set
 */
TMR_Status
TMR_LLRP_cmdSetsendSelect(TMR_Reader *reader, bool *select)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                *pCmd;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSMessage                          *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE       *pRsp;
  LLRP_tSThingMagicProtocolConfiguration  *pProtoConfig;
  LLRP_tSGen2CustomParameters             *pGen2Custom;
  LLRP_tSsendSelect                       *pSelect;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic Protocol configuration is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize Protocol configuration */
  pProtoConfig = LLRP_ThingMagicProtocolConfiguration_construct();

  /* Initialize sendSelect value*/
  pSelect = LLRP_sendSelect_construct();
  pSelect->selectValue = *select;

  /* Initialize gen2 custom parameter */
  pGen2Custom = LLRP_Gen2CustomParameters_construct();
  LLRP_Gen2CustomParameters_setsendSelect(pGen2Custom, pSelect);

  /* Set Gen2 Custom parameter to protocol configuration */
  LLRP_ThingMagicProtocolConfiguration_setGen2CustomParameters(pProtoConfig,
                                                                pGen2Custom);
  /* Add ThingMagic Protocol configuration as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pProtoConfig);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

 /**
 * Command to get initQ value
 *
 * @param reader Reader pointer
 * @param[out] q Pointer to TMR_GEN2_initQ object to hold the initQ value
 */
TMR_Status
TMR_LLRP_cmdGetInitQ(TMR_Reader *reader, TMR_GEN2_initQ *q)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG                    *pCmd;
  LLRP_tSMessage                              *pCmdMsg;
  LLRP_tSMessage                              *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE           *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration *pTMProtoConfig;
  LLRP_tSParameter                            *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/gen2/initQ" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMProtoConfig = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMProtoConfig)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., protocol configuration)
   * And add to GET_READER_CONFIG message.
   **/
 LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMProtoConfig,
        LLRP_ThingMagicControlConfiguration_ThingMagicProtocolConfiguration);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMProtoConfig->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMProtoConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }

  /**
   * Response is success, extract initQ from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    LLRP_tSGen2CustomParameters *gen2Custom;
    LLRP_tSInitQ *InitQ;
    LLRP_tSqValue *pQvalue;
    uint8_t Value;

    gen2Custom = LLRP_ThingMagicProtocolConfiguration_getGen2CustomParameters(
                        (LLRP_tSThingMagicProtocolConfiguration *)pCustParam);
    InitQ = LLRP_Gen2CustomParameters_getInitQ(gen2Custom);
    if(InitQ != NULL)
    {
      pQvalue = LLRP_InitQ_getqValue(InitQ);
      Value = LLRP_qValue_getvalue(pQvalue);
      q->qEnable = InitQ->qEnable;
      q->qValue = Value;
    }
  }
  else
  {
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
 }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to Set InitQ value
 *
 * @param reader Reader pointer
 * @param[in] q Pointer to TMR_GEN2_initQ object which needs to be set
 */
TMR_Status
TMR_LLRP_cmdSetInitQ(TMR_Reader *reader, TMR_GEN2_initQ *q)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                *pCmd;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSMessage                          *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE       *pRsp;
  LLRP_tSThingMagicProtocolConfiguration  *pProtoConfig;
  LLRP_tSGen2CustomParameters             *pGen2Custom;
  LLRP_tSInitQ                            *pInitQ;
  LLRP_tSqValue                           *pQvalue;
  ret = TMR_SUCCESS;
  /**
   * Thingmagic Protocol configuration is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize Protocol configuration */
  pProtoConfig = LLRP_ThingMagicProtocolConfiguration_construct();

  /* Initialize initQ */
  pInitQ = LLRP_InitQ_construct();
  LLRP_InitQ_setqEnable(pInitQ, q->qEnable);

  if (q->qEnable)
  {
    pQvalue = LLRP_qValue_construct();
    /* If qEnable is true, set the value */
    LLRP_qValue_setvalue(pQvalue, q->qValue);
    LLRP_InitQ_setqValue(pInitQ, pQvalue);
  }

  /* Initialize gen2 custom parameter */
  pGen2Custom = LLRP_Gen2CustomParameters_construct();
  LLRP_Gen2CustomParameters_setInitQ(pGen2Custom, pInitQ);

  /* Set Gen2 Custom parameter to protocol configuration */
  LLRP_ThingMagicProtocolConfiguration_setGen2CustomParameters(pProtoConfig,
                                                                pGen2Custom);

  /* Add ThingMagic Protocol configuration as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pProtoConfig);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Command to get Gen2 Session value
 *
 * @param reader Reader pointer
 * @param session Pointer to TMR_Gen2_session
 */
TMR_Status
TMR_LLRP_cmdGetGen2Session(TMR_Reader *reader, TMR_GEN2_Session *session)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSAntennaConfiguration           *pAntConfig;

  ret = TMR_SUCCESS;
  /**
   * Gen2 Session can be retrieved
   * through  LLRP standard parameter GET_READER_CONFIG_RESPONSE.AntennaConfiguration.
   * airProtocolInventoryCommand.SingulationControl
   * Initialize GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_AntennaConfiguration);

  /* Get antenna configuration for all antennas*/
  LLRP_GET_READER_CONFIG_setAntennaID(pCmd, 0);

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }

  /**
   * Response is success, extract Session value from response
   **/
  for (pAntConfig = LLRP_GET_READER_CONFIG_RESPONSE_beginAntennaConfiguration(pRsp);
      (pAntConfig != NULL);
      pAntConfig = LLRP_GET_READER_CONFIG_RESPONSE_nextAntennaConfiguration(pAntConfig))
  {
    LLRP_tSParameter *pInventoryCommand;
    llrp_u2_t        temp;

    for (pInventoryCommand = LLRP_AntennaConfiguration_beginAirProtocolInventoryCommandSettings(pAntConfig);
        (pInventoryCommand != NULL);
        pInventoryCommand = LLRP_AntennaConfiguration_nextAirProtocolInventoryCommandSettings(pInventoryCommand))
    {
      LLRP_tSC1G2SingulationControl *pSingulationControl;

      pSingulationControl = LLRP_C1G2InventoryCommand_getC1G2SingulationControl(
          (LLRP_tSC1G2InventoryCommand *)pInventoryCommand);

      /* get the session */
      temp = LLRP_C1G2SingulationControl_getSession(pSingulationControl);

      /* convert the session value */
      switch (temp)
      {
        case 0:
          {
            *session = TMR_GEN2_SESSION_S0;
            break;
          }
        case 1:
          {
            *session = TMR_GEN2_SESSION_S1;
            break;
          }
        case 2:
          {
            *session = TMR_GEN2_SESSION_S2;
            break;
          }
        case 3:
          {
            *session = TMR_GEN2_SESSION_S3;
            break;
          }
        default:
          {
            *session = TMR_GEN2_SESSION_INVALID;
            break;
          }
      }
    }
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to set Gen2 Session
 *
 * @param reader Reader pointer
 * @param session Pointer to TMR_GEN2_Session
 */
TMR_Status
TMR_LLRP_cmdSetGen2Session(TMR_Reader *reader, TMR_GEN2_Session *session)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG              *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE     *pRsp;
  LLRP_tSAntennaConfiguration           *pAntConfig;
  LLRP_tSC1G2SingulationControl         *pSingulationControl;
  LLRP_tSC1G2InventoryCommand           *pInventoryCommand;

  ret = TMR_SUCCESS;
  /**
   * Gen2 Session can be set
   * through  LLRP standard parameter GET_READER_CONFIG_RESPONSE.AntennaConfiguration.
   * airProtocolInventoryCommand.SingulationControl
   * Initialize SET_READER_CONFIG message
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize antenna configuration */
  pAntConfig = LLRP_AntennaConfiguration_construct();
  if (NULL == pAntConfig)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  LLRP_AntennaConfiguration_setAntennaID(pAntConfig, 0); /* For all antennas */

  /* Initialize Inventory Command */
  pInventoryCommand = LLRP_C1G2InventoryCommand_construct();
  if (NULL == pInventoryCommand)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pAntConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /* Initialize SingulationControl */
  pSingulationControl = LLRP_C1G2SingulationControl_construct();
  if (NULL == pSingulationControl)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pInventoryCommand);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pAntConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }


  /* convert the session value */
  switch (*session)
  {
    case TMR_GEN2_SESSION_S0:
      {
        pSingulationControl->Session = 0;
        break;
      }
    case TMR_GEN2_SESSION_S1:
      {
        pSingulationControl->Session = 1;
        break;
      }
    case TMR_GEN2_SESSION_S2:
      {
        pSingulationControl->Session = 2;
        break;
      }
    case TMR_GEN2_SESSION_S3:
      {
        pSingulationControl->Session = 3;
        break;
      }
    default:
      {
        TMR_LLRP_freeMessage((LLRP_tSMessage *)pSingulationControl);
        TMR_LLRP_freeMessage((LLRP_tSMessage *)pInventoryCommand);
        TMR_LLRP_freeMessage((LLRP_tSMessage *)pAntConfig);
        TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
        return TMR_ERROR_INVALID;
      }
  }

  /* Set Singulation Control to inventoryCommand */
  LLRP_C1G2InventoryCommand_setC1G2SingulationControl(pInventoryCommand, pSingulationControl);

  /* Set InventoryCommand to antenna configuration */
  LLRP_AntennaConfiguration_addAirProtocolInventoryCommandSettings(
      pAntConfig, (LLRP_tSParameter *)pInventoryCommand);

  /* Set antenna configuration to SET_READER_CONFIG */
  LLRP_SET_READER_CONFIG_addAntennaConfiguration(pCmd, pAntConfig);

  pCmdMsg       = &pCmd->hdr;

  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to get Gen2 Target value
 *
 * @param reader Reader pointer
 * @param target  Pointer to TMR_GEN2_Target object to hold the Gen2 target value
 */ 
TMR_Status
TMR_LLRP_cmdGetGen2Target(TMR_Reader *reader, TMR_GEN2_Target *target)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG                    *pCmd;
  LLRP_tSMessage                              *pCmdMsg;
  LLRP_tSMessage                              *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE           *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration *pTMProtoConfig;
  LLRP_tSParameter                            *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/gen2/target" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMProtoConfig = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMProtoConfig)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., protocol configuration)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMProtoConfig, 
      LLRP_ThingMagicControlConfiguration_ThingMagicProtocolConfiguration);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMProtoConfig->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMProtoConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract Gen2 target from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    LLRP_tSGen2CustomParameters *gen2Custom;
    LLRP_tSThingMagicTargetStrategy  *gen2target;

    gen2Custom = LLRP_ThingMagicProtocolConfiguration_getGen2CustomParameters(
        (LLRP_tSThingMagicProtocolConfiguration *)pCustParam);

    gen2target = LLRP_Gen2CustomParameters_getThingMagicTargetStrategy(gen2Custom);
    *target = (TMR_GEN2_Target)gen2target->eThingMagicTargetStrategyValue;
  }
  else
  {
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to Set Gen2 Target value
 *
 * @param reader Reader pointer
 * @param target Pointer to TMR_GEN2_Target which needs to be set
 */
TMR_Status
TMR_LLRP_cmdSetGen2Target(TMR_Reader *reader, TMR_GEN2_Target *target)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                *pCmd;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSMessage                          *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE       *pRsp;
  LLRP_tSThingMagicProtocolConfiguration  *pProtoConfig;
  LLRP_tSGen2CustomParameters             *pGen2Custom;
  LLRP_tSThingMagicTargetStrategy         *gen2target;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic Protocol configuration is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize Protocol configuration */
  pProtoConfig = LLRP_ThingMagicProtocolConfiguration_construct();

  /* Initialize gen2 Target */
  gen2target = LLRP_ThingMagicTargetStrategy_construct();
  gen2target->eThingMagicTargetStrategyValue = (LLRP_tEThingMagicC1G2TargetStrategy)*target;

  /* Initialize gen2 custom parameter */
  pGen2Custom = LLRP_Gen2CustomParameters_construct();
  LLRP_Gen2CustomParameters_setThingMagicTargetStrategy(pGen2Custom, gen2target);

  /* Set Gen2 Custom parameter to protocol configuration */
  LLRP_ThingMagicProtocolConfiguration_setGen2CustomParameters(pProtoConfig,
      pGen2Custom);

  /* Add ThingMagic Protocol configuration as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pProtoConfig);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to get Gen2 T4 Param value
 *
 * @param reader Reader pointer
 * @param pT4Param Pointer to hold the Gen2 T4 param value
 */ 
TMR_Status
TMR_LLRP_cmdGetGen2T4Param(TMR_Reader *reader, uint32_t *pT4Param)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG                    *pCmd;
  LLRP_tSMessage                              *pCmdMsg;
  LLRP_tSMessage                              *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE           *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration *pTMProtoConfig;
  LLRP_tSParameter                            *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/gen2/t4" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMProtoConfig = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMProtoConfig)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., protocol configuration)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMProtoConfig, 
      LLRP_ThingMagicControlConfiguration_ThingMagicProtocolConfiguration);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMProtoConfig->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMProtoConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract Gen2 t4 param from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    LLRP_tSGen2CustomParameters *gen2Custom;
    LLRP_tSGen2T4Param *gen2T4;

    gen2Custom = LLRP_ThingMagicProtocolConfiguration_getGen2CustomParameters(
        (LLRP_tSThingMagicProtocolConfiguration *)pCustParam);

    gen2T4 = LLRP_Gen2CustomParameters_getGen2T4Param(gen2Custom);
    *pT4Param = (uint32_t)LLRP_Gen2T4Param_getT4ParamValue(gen2T4);
  }
  else
  {
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to Set Gen2 T4 param value
 *
 * @param reader Reader pointer
 * @param t4Param  which needs to be set
 */
TMR_Status
TMR_LLRP_cmdSetGen2T4Param(TMR_Reader *reader, const uint32_t t4Param)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                *pCmd;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSMessage                          *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE       *pRsp;
  LLRP_tSThingMagicProtocolConfiguration  *pProtoConfig;
  LLRP_tSGen2CustomParameters             *pGen2Custom;
  LLRP_tSGen2T4Param                      *pGen2T4;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic Protocol configuration is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize Protocol configuration */
  pProtoConfig = LLRP_ThingMagicProtocolConfiguration_construct();

  /* Initialize gen2 T4 */
  pGen2T4 = LLRP_Gen2T4Param_construct();
  LLRP_Gen2T4Param_setT4ParamValue(pGen2T4, t4Param);

  /* Initialize gen2 custom parameter */
  pGen2Custom = LLRP_Gen2CustomParameters_construct();
  LLRP_Gen2CustomParameters_setGen2T4Param(pGen2Custom, pGen2T4);

  /* Set Gen2 Custom parameter to protocol configuration */
  LLRP_ThingMagicProtocolConfiguration_setGen2CustomParameters(pProtoConfig,
      pGen2Custom);

  /* Add ThingMagic Protocol configuration as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pProtoConfig);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to get Regulatory Mode Param value
 *
 * @param reader Reader pointer
 * @param pModeParam Pointer to hold the Regulatory mode param value
 */ 
TMR_Status
TMR_LLRP_cmdGetThingMagicRegulatoryMode(TMR_Reader *reader, uint16_t *pModeParam)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG                    *pCmd;
  LLRP_tSMessage                              *pCmdMsg;
  LLRP_tSMessage                              *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE           *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration *pTMRegulatoryConfig;
  LLRP_tSParameter                            *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/regulatory/mode" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMRegulatoryConfig = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMRegulatoryConfig)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., regulatory configuration)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMRegulatoryConfig, 
      LLRP_ThingMagicControlConfiguration_ThingMagicRegulatoryConfiguration);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMRegulatoryConfig->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMRegulatoryConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract regulatory mode param from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    LLRP_tSRegulatoryMode *regMode;
    llrp_u8_t modeParam;

    regMode = LLRP_ThingMagicRegulatoryConfiguration_getRegulatoryMode(
        (LLRP_tSThingMagicRegulatoryConfiguration *)pCustParam);
    modeParam = LLRP_RegulatoryMode_getModeParam(regMode);
    switch(modeParam)
    {
      case 1:
        *(uint8_t*)pModeParam = LLRP_ThingMagicRegulatoryMode_ONE_SHOT; 
        break;
      default:
        *(uint8_t*)pModeParam = LLRP_ThingMagicRegulatoryMode_CONTINUOUS; 
        break;
    }
  }
  else
  {
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to Set Regulatory Mode param value
 *
 * @param reader Reader pointer
 * @param modeParam  which needs to be set
 */
TMR_Status
TMR_LLRP_cmdSetThingMagicRegulatoryMode(TMR_Reader *reader, uint16_t modeParam)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                  *pCmd;
  LLRP_tSMessage                            *pCmdMsg;
  LLRP_tSMessage                            *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE         *pRsp;
  LLRP_tSThingMagicRegulatoryConfiguration  *pRegulatoryConfig;
  LLRP_tSRegulatoryMode                     *pMode;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic Regulatory configuration is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize Regulatory configuration */
  pRegulatoryConfig = LLRP_ThingMagicRegulatoryConfiguration_construct();

  /* Initialize Regulatory mode */
  pMode = LLRP_RegulatoryMode_construct();
  LLRP_RegulatoryMode_setModeParam(pMode, modeParam);

  /* Set regulatory mode parameter to regulatory configuration */
  LLRP_ThingMagicRegulatoryConfiguration_setRegulatoryMode(pRegulatoryConfig, pMode);

  /* Add ThingMagic Regulatory configuration as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pRegulatoryConfig);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to get Regulatory Modulation Param value
 *
 * @param reader Reader pointer
 * @param pModulationParam Pointer to hold the Regulatory modulation param value
 */ 
TMR_Status
TMR_LLRP_cmdGetThingMagicRegulatoryModulation(TMR_Reader *reader, uint16_t *pModulationParam)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG                    *pCmd;
  LLRP_tSMessage                              *pCmdMsg;
  LLRP_tSMessage                              *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE           *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration *pTMRegulatoryConfig;
  LLRP_tSParameter                            *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/regulatory/modulation" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMRegulatoryConfig = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMRegulatoryConfig)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., regulatory configuration)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMRegulatoryConfig, 
      LLRP_ThingMagicControlConfiguration_ThingMagicRegulatoryConfiguration);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMRegulatoryConfig->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMRegulatoryConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract regulatory mode param from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    LLRP_tSRegulatoryModulation *regModulation;
    llrp_u8_t modulationParam;

    regModulation = LLRP_ThingMagicRegulatoryConfiguration_getRegulatoryModulation(
        (LLRP_tSThingMagicRegulatoryConfiguration *)pCustParam);
    modulationParam = LLRP_RegulatoryModulation_getModulationParam(regModulation);

    switch(modulationParam)
    {
      case 1:
        *(uint8_t*)pModulationParam = LLRP_ThingMagicRegulatoryModulation_CW;
        break;
      case 2:
        *(uint8_t*)pModulationParam = LLRP_ThingMagicRegulatoryModulation_PRBS; 
        break;
      default:
        break;
    }
  }
  else
  {
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to Set Regulatory Modulation param value
 *
 * @param reader Reader pointer
 * @param modulationParam  which needs to be set
 */
TMR_Status
TMR_LLRP_cmdSetThingMagicRegulatoryModulation(TMR_Reader *reader, uint16_t modulationParam)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                  *pCmd;
  LLRP_tSMessage                            *pCmdMsg;
  LLRP_tSMessage                            *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE         *pRsp;
  LLRP_tSThingMagicRegulatoryConfiguration  *pRegulatoryConfig;
  LLRP_tSRegulatoryModulation               *pModulation;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic Regulatory configuration is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize Regulatory configuration */
  pRegulatoryConfig = LLRP_ThingMagicRegulatoryConfiguration_construct();

  /* Initialize Regulatory modulation */
  pModulation = LLRP_RegulatoryModulation_construct();
  LLRP_RegulatoryModulation_setModulationParam(pModulation, modulationParam);

  /* Set regulatory modulation parameter to regulatory configuration */
  LLRP_ThingMagicRegulatoryConfiguration_setRegulatoryModulation(pRegulatoryConfig, pModulation);

  /* Add ThingMagic Regulatory configuration as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pRegulatoryConfig);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}
/**
 * Command to get Regulatory Enable Param value
 *
 * @param reader Reader pointer
 * @param pEnableParam Pointer to hold the Regulatory mode param value
 */ 
TMR_Status
TMR_LLRP_cmdGetThingMagicRegulatoryEnable(TMR_Reader *reader, bool *pEnableParam)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG                    *pCmd;
  LLRP_tSMessage                              *pCmdMsg;
  LLRP_tSMessage                              *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE           *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration *pTMRegulatoryConfig;
  LLRP_tSParameter                            *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/regulatory/enable" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMRegulatoryConfig = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMRegulatoryConfig)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., regulatory configuration)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMRegulatoryConfig, 
      LLRP_ThingMagicControlConfiguration_ThingMagicRegulatoryConfiguration);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMRegulatoryConfig->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMRegulatoryConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract regulatory mode param from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    LLRP_tSRegulatoryEnable *regEnable;
    bool enableParam;

    regEnable = LLRP_ThingMagicRegulatoryConfiguration_getRegulatoryEnable(
        (LLRP_tSThingMagicRegulatoryConfiguration *)pCustParam);
    enableParam = LLRP_RegulatoryEnable_getEnableParam(regEnable);
    *(bool*)pEnableParam = enableParam;

  }
  else
  {
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to Set Regulatory Enable param value
 *
 * @param reader Reader pointer
 * @param enableParam  which needs to be set
 */
TMR_Status
TMR_LLRP_cmdSetThingMagicRegulatoryEnable(TMR_Reader *reader, bool enableParam)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                  *pCmd;
  LLRP_tSMessage                            *pCmdMsg;
  LLRP_tSMessage                            *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE         *pRsp;
  LLRP_tSThingMagicRegulatoryConfiguration  *pRegulatoryConfig;
  LLRP_tSRegulatoryEnable                   *pEnable;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic Regulatory configuration is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize Regulatory configuration */
  pRegulatoryConfig = LLRP_ThingMagicRegulatoryConfiguration_construct();

  /* Initialize Regulatory enable */
  pEnable = LLRP_RegulatoryEnable_construct();
  LLRP_RegulatoryEnable_setEnableParam(pEnable, enableParam);

  /* Set regulatory enable parameter to regulatory configuration */
  LLRP_ThingMagicRegulatoryConfiguration_setRegulatoryEnable(pRegulatoryConfig, pEnable);

  /* Add ThingMagic Regulatory configuration as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pRegulatoryConfig);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}
/**
 * Command to get Regulatory ontime Param value
 *
 * @param reader Reader pointer
 * @param pOntimeParam Pointer to hold the Regulatory mode param value
 */ 
TMR_Status
TMR_LLRP_cmdGetThingMagicRegulatoryOntime(TMR_Reader *reader, uint32_t *pOntimeParam)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG                    *pCmd;
  LLRP_tSMessage                              *pCmdMsg;
  LLRP_tSMessage                              *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE           *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration *pTMRegulatoryConfig;
  LLRP_tSParameter                            *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/regulatory/ontime" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMRegulatoryConfig = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMRegulatoryConfig)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., regulatory configuration)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMRegulatoryConfig, 
      LLRP_ThingMagicControlConfiguration_ThingMagicRegulatoryConfiguration);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMRegulatoryConfig->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMRegulatoryConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract regulatory mode param from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    LLRP_tSRegulatoryOntime *regOntime;
    llrp_u32_t ontimeparam;

    regOntime = LLRP_ThingMagicRegulatoryConfiguration_getRegulatoryOntime(
        (LLRP_tSThingMagicRegulatoryConfiguration *)pCustParam);
    ontimeparam = LLRP_RegulatoryOntime_getOntimeParam(regOntime);

    *pOntimeParam = ontimeparam;
  }
  else
  {
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to Set Regulatory Ontime param value
 *
 * @param reader Reader pointer
 * @param ontimeParam  which needs to be set
 */
TMR_Status
TMR_LLRP_cmdSetThingMagicRegulatoryOntime(TMR_Reader *reader, uint32_t ontimeParam)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                  *pCmd;
  LLRP_tSMessage                            *pCmdMsg;
  LLRP_tSMessage                            *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE         *pRsp;
  LLRP_tSThingMagicRegulatoryConfiguration  *pRegulatoryConfig;
  LLRP_tSRegulatoryOntime                   *pOntime;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic Regulatory configuration is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize Regulatory configuration */
  pRegulatoryConfig = LLRP_ThingMagicRegulatoryConfiguration_construct();

  /* Initialize Regulatory ontime */
  pOntime = LLRP_RegulatoryOntime_construct();
  LLRP_RegulatoryOntime_setOntimeParam(pOntime, ontimeParam);

  /* Set regulatory ontime parameter to regulatory configuration */
  LLRP_ThingMagicRegulatoryConfiguration_setRegulatoryOntime(pRegulatoryConfig, pOntime);

  /* Add ThingMagic Regulatory configuration as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pRegulatoryConfig);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}
/**
 * Command to get Regulatory Offtime Param value
 *
 * @param reader Reader pointer
 * @param pOfftimeParam Pointer to hold the Regulatory mode param value
 */ 
TMR_Status
TMR_LLRP_cmdGetThingMagicRegulatoryOfftime(TMR_Reader *reader, uint32_t *pOfftimeParam)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG                    *pCmd;
  LLRP_tSMessage                              *pCmdMsg;
  LLRP_tSMessage                              *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE           *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration *pTMRegulatoryConfig;
  LLRP_tSParameter                            *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/regulatory/offtime" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMRegulatoryConfig = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMRegulatoryConfig)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., regulatory configuration)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMRegulatoryConfig, 
      LLRP_ThingMagicControlConfiguration_ThingMagicRegulatoryConfiguration);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMRegulatoryConfig->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMRegulatoryConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract regulatory mode param from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    LLRP_tSRegulatoryOfftime *regOfftime;
    llrp_u32_t offtimeParam;

    regOfftime = LLRP_ThingMagicRegulatoryConfiguration_getRegulatoryOfftime(
        (LLRP_tSThingMagicRegulatoryConfiguration *)pCustParam);
    offtimeParam = LLRP_RegulatoryOfftime_getOfftimeParam(regOfftime);

    *pOfftimeParam = offtimeParam;
  }
  else
  {
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to Set Regulatory Offtime param value
 *
 * @param reader Reader pointer
 * @param offtimeParam  which needs to be set
 */
TMR_Status
TMR_LLRP_cmdSetThingMagicRegulatoryOfftime(TMR_Reader *reader, uint32_t offtimeParam)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                  *pCmd;
  LLRP_tSMessage                            *pCmdMsg;
  LLRP_tSMessage                            *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE         *pRsp;
  LLRP_tSThingMagicRegulatoryConfiguration  *pRegulatoryConfig;
  LLRP_tSRegulatoryOfftime                  *pOfftime;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic Regulatory configuration is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize Regulatory configuration */
  pRegulatoryConfig = LLRP_ThingMagicRegulatoryConfiguration_construct();

  /* Initialize Regulatory offtime */
  pOfftime = LLRP_RegulatoryOfftime_construct();
  LLRP_RegulatoryOfftime_setOfftimeParam(pOfftime, offtimeParam);

  /* Set regulatory offtime parameter to regulatory configuration */
  LLRP_ThingMagicRegulatoryConfiguration_setRegulatoryOfftime(pRegulatoryConfig, pOfftime);

  /* Add ThingMagic Regulatory configuration as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pRegulatoryConfig);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to Set Port Switch GPOs
 *
 * @param reader Reader pointer
 * @param GPOList  which needs to be set
 */
TMR_Status
TMR_LLRP_cmdSetThingMagicPortSwitchGPOs(TMR_Reader *reader, TMR_uint8List *GPOList)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                  *pCmd;
  LLRP_tSMessage                            *pCmdMsg;
  LLRP_tSMessage                            *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE         *pRsp;
  LLRP_tSThingMagicPortSwitchGPO            *pPortSwitchGPO;
  llrp_u8v_t LLRPGPOList;

  ret = TMR_SUCCESS;
  /**
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();


  /* Initialize ThingMagic PortSwitch GPO  */
  pPortSwitchGPO = LLRP_ThingMagicPortSwitchGPO_construct();
  LLRPGPOList = LLRP_u8v_construct(GPOList->len);
  LLRPGPOList.nValue = GPOList->len;
  memcpy(LLRPGPOList.pValue, GPOList->list, GPOList->len);

  /* Set the GPO List */

  LLRP_ThingMagicPortSwitchGPO_setportSwitchGPOList(pPortSwitchGPO, LLRPGPOList);

  /* Add ThingMagic Port Switch GPO configuration as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pPortSwitchGPO);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
 ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to Set Custom Hoptable param value
 *
 * @param reader Reader pointer
 * @param hopTableParam which needs to be set
 */
TMR_Status
TMR_LLRP_cmdSetThingMagicRegionHoptable(TMR_Reader *reader, const TMR_uint32List *hopTableParam)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                  *pCmd;
  LLRP_tSMessage                            *pCmdMsg;
  LLRP_tSMessage                            *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE         *pRsp;
  LLRP_tSThingMagicFrequencyConfiguration   *pFrequencyConfig;
  LLRP_tSCustomFrequencyHopTable            *pHopTable;
  llrp_u32v_t                                freqHopTable;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic Frequency configuration is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize Frequency configuration */
  pFrequencyConfig = LLRP_ThingMagicFrequencyConfiguration_construct();

  /*Set Hopping to 1 to set Frequncy Hoptable*/
  LLRP_ThingMagicFrequencyConfiguration_setHopping(pFrequencyConfig, 1);

  /* Initialize Frequency Hoptable */
  pHopTable = LLRP_CustomFrequencyHopTable_construct();
  LLRP_CustomFrequencyHopTable_setHopTableID(pHopTable, 1);
  
  freqHopTable = LLRP_u32v_construct(hopTableParam->len);
  if (NULL == freqHopTable.pValue)
  {
    return TMR_ERROR_LLRP;
  }
  memcpy(freqHopTable.pValue, hopTableParam->list,
        (hopTableParam->len
         * sizeof(hopTableParam->list[0])));
  LLRP_CustomFrequencyHopTable_setFrequency(pHopTable, freqHopTable);

  /* Set Frequency HopTable parameter to frequency configuration */
  LLRP_ThingMagicFrequencyConfiguration_addCustomFrequencyHopTable(pFrequencyConfig, pHopTable);

  /* Add ThingMagic Frequency configuration as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pFrequencyConfig);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to Set  TMLicenseKey value
 *
 * @param reader Reader pointer
 * @param license Pointer to TMR_uint8List which needs to be set
 */
TMR_Status
TMR_LLRP_cmdSetTMLicenseKey(TMR_Reader *reader, TMR_uint8List *license)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                *pCmd;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSMessage                          *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE       *pRsp;
  LLRP_tSThingMagicLicenseKey             *pTMLicense;
  llrp_u8v_t key;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic  LicenseKey is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize ThingMagic LicenseKey  */
  pTMLicense = LLRP_ThingMagicLicenseKey_construct();

  /* Set the License key */
  key = LLRP_u8v_construct(license->len);
  key.nValue = license->len;
  memcpy(key.pValue, license->list, license->len);

  LLRP_ThingMagicLicenseKey_setLicenseKey(pTMLicense, key);

  /* Add ThingMagic License key  as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pTMLicense);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to Set  TMAsyncOfftime value
 *
 * @param reader Reader pointer
 * @param license Pointer to TMR_uint8List which needs to be set
   */
TMR_Status
TMR_LLRP_cmdSetTMAsyncOffTime(TMR_Reader *reader, uint32_t offtime)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                *pCmd;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSMessage                          *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE       *pRsp;
  LLRP_tSThingMagicAsyncOFFTime           *pTMAsyncOffTime;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic  LicenseKey is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize ThingMagic LicenseKey  */
  pTMAsyncOffTime = LLRP_ThingMagicAsyncOFFTime_construct();

  /* Set the License key */

  LLRP_ThingMagicAsyncOFFTime_setAsyncOFFTime(pTMAsyncOffTime, (llrp_u32_t)offtime);

  /* Add ThingMagic License key  as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pTMAsyncOffTime);

  pCmdMsg = &pCmd->hdr;
 /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to Set  TMAsyncOntime value
 *
 * @param reader Reader pointer
 * @param ontime which needs to be set
   */
TMR_Status
TMR_LLRP_cmdSetTMAsyncOnTime(TMR_Reader *reader, uint32_t ontime)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                *pCmd;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSMessage                          *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE       *pRsp;
  LLRP_tSThingMagicAsyncONTime            *pTMAsyncOnTime;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic ontime is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize ThingMagic ontime  */
  pTMAsyncOnTime = LLRP_ThingMagicAsyncONTime_construct();

  /* Set the ontime */

  LLRP_ThingMagicAsyncONTime_setAsyncONTime(pTMAsyncOnTime, (llrp_u32_t)ontime);

  /* Add ThingMagic ontime  as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pTMAsyncOnTime);

  pCmdMsg = &pCmd->hdr;
 /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to Set  TMMetadata flag
 *
 * @param reader Reader pointer
 * @param metadata which needs to be set
   */
TMR_Status
TMR_LLRP_cmdSetTMMetadataFlag(TMR_Reader *reader, uint16_t metadata)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                *pCmd;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSMessage                          *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE       *pRsp;
  LLRP_tSThingMagicMetadata               *pTMMetadataFlag;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic metadata is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize ThingMagic metadata  */
  pTMMetadataFlag = LLRP_ThingMagicMetadata_construct();

  /* Set the metadata */

  LLRP_ThingMagicMetadata_setMetadata(pTMMetadataFlag, (llrp_u16_t)metadata);

  /* Add ThingMagic metadata  as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pTMMetadataFlag);

  pCmdMsg = &pCmd->hdr;
 /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}
/**
 * Command to Set  TMStatsEnable flag
 *
 * @param reader Reader pointer
 * @param statsEnable which needs to be set
   */
TMR_Status
TMR_LLRP_cmdSetTMStatsEnable(TMR_Reader *reader, uint16_t statsEnable)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                *pCmd;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSMessage                          *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE       *pRsp;
  LLRP_tSThingMagicStatsEnable               *pTMStatsEnableFlag;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic statsEnable is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize ThingMagic statsEnable  */
  pTMStatsEnableFlag = LLRP_ThingMagicStatsEnable_construct();

  /* Set the statsEnable */

  LLRP_ThingMagicStatsEnable_setStatsEnable(pTMStatsEnableFlag, (llrp_u16_t)statsEnable);

  /* Add ThingMagic statsEnable  as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pTMStatsEnableFlag);

  pCmdMsg = &pCmd->hdr;
 /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

#ifdef TMR_ENABLE_ISO180006B
/**
 * Command to get ISO 18K6B delimiter value
 *
 * @param reader Reader pointer
 * @param delimiter  Pointer to TMR_ISO180006B_Delimiter object to hold the ISO 18K6B delimiter value
 */ 
TMR_Status
TMR_LLRP_cmdGetISO18K6BDelimiter(TMR_Reader *reader, TMR_ISO180006B_Delimiter *delimiter)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG                    *pCmd;
  LLRP_tSMessage                              *pCmdMsg;
  LLRP_tSMessage                              *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE           *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration *pTMProtoConfig;
  LLRP_tSParameter                            *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/iso180006b/delimiter" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMProtoConfig = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMProtoConfig)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., protocol configuration)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMProtoConfig, 
      LLRP_ThingMagicControlConfiguration_ThingMagicProtocolConfiguration);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMProtoConfig->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMProtoConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract ISO 18K6B delimiter  from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    LLRP_tSISO18K6BCustomParameters *iso18k6bCustParam;
    LLRP_tSThingMagicISO180006BDelimiter  *iso18k6bDelimiter;

    iso18k6bCustParam = LLRP_ThingMagicProtocolConfiguration_getISO18K6BCustomParameters(
        (LLRP_tSThingMagicProtocolConfiguration *)pCustParam);

    if (NULL != iso18k6bCustParam) 
    {
      iso18k6bDelimiter = LLRP_ISO18K6BCustomParameters_getThingMagicISO180006BDelimiter(iso18k6bCustParam);
      *delimiter = (TMR_ISO180006B_Delimiter)iso18k6bDelimiter->eISO18K6BDelimiter;
    }
    else
    {
      return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
    }
  }
  else
  {
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to Set ISO18K6B delimiter value
 *
 * @param reader Reader pointer
 * @param delimiter Pointer to TMR_ISO180006B_Delimiter which needs to be set
 */
TMR_Status
TMR_LLRP_cmdSetISO18K6BDelimiter(TMR_Reader *reader, TMR_ISO180006B_Delimiter *delimiter)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                *pCmd;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSMessage                          *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE       *pRsp;
  LLRP_tSThingMagicProtocolConfiguration  *pProtoConfig;
  LLRP_tSISO18K6BCustomParameters         *pISO18k6bCustParam;
  LLRP_tSThingMagicISO180006BDelimiter    *pISO18k6bDelimiter;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic Protocol configuration is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize Protocol configuration */
  pProtoConfig = LLRP_ThingMagicProtocolConfiguration_construct();

  /* Initialize ISO 18K6B dedlimiter */
  pISO18k6bDelimiter = LLRP_ThingMagicISO180006BDelimiter_construct();
  pISO18k6bDelimiter->eISO18K6BDelimiter = (LLRP_tEThingMagicCustom18K6BDelimiter)*delimiter;

  /* Initialize ISO 18K6B custom parameter */
  pISO18k6bCustParam = LLRP_ISO18K6BCustomParameters_construct();
  LLRP_ISO18K6BCustomParameters_setThingMagicISO180006BDelimiter(pISO18k6bCustParam, pISO18k6bDelimiter);

  /* Set ISO 18K6B Custom parameter to protocol configuration */
  LLRP_ThingMagicProtocolConfiguration_setISO18K6BCustomParameters(pProtoConfig,
      pISO18k6bCustParam);

  /* Add ThingMagic Protocol configuration as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pProtoConfig);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to get ISO18K6B modulation Depth value
 *
 * @param reader Reader pointer
 * @param modDepth  Pointer to TMR_ISO180006B_ModulationDepth object to hold the ISO 18K6B modulation Depth value
 */ 
TMR_Status
TMR_LLRP_cmdGetISO18K6BModDepth(TMR_Reader *reader, TMR_ISO180006B_ModulationDepth *modDepth)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG                    *pCmd;
  LLRP_tSMessage                              *pCmdMsg;
  LLRP_tSMessage                              *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE           *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration *pTMProtoConfig;
  LLRP_tSParameter                            *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/iso180006b/modulationDepth" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMProtoConfig = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMProtoConfig)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., protocol configuration)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMProtoConfig, 
      LLRP_ThingMagicControlConfiguration_ThingMagicProtocolConfiguration);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMProtoConfig->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMProtoConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract ISO 18K6B modulation depth from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    LLRP_tSISO18K6BCustomParameters          *pISO18k6bCustParam;
    LLRP_tSThingMagicISO18K6BModulationDepth *pISO18k6bModDepth;

    pISO18k6bCustParam = LLRP_ThingMagicProtocolConfiguration_getISO18K6BCustomParameters(
        (LLRP_tSThingMagicProtocolConfiguration *)pCustParam);

    if (NULL != pISO18k6bCustParam) 
    {
      pISO18k6bModDepth = LLRP_ISO18K6BCustomParameters_getThingMagicISO18K6BModulationDepth(pISO18k6bCustParam);
      *modDepth = (TMR_ISO180006B_ModulationDepth)pISO18k6bModDepth->eISO18K6BModulationDepth;
    }
    else {
      return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
    }
  }
  else
  {
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to Set ISO18K6B Modulation Depth value
 *
 * @param reader Reader pointer
 * @param modDepth Pointer to TMR_ISO180006B_ModulationDepth which needs to be set
 */
TMR_Status
TMR_LLRP_cmdSetISO18K6BModDepth(TMR_Reader *reader, TMR_ISO180006B_ModulationDepth *modDepth)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                *pCmd;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSMessage                          *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE       *pRsp;
  LLRP_tSThingMagicProtocolConfiguration  *pProtoConfig;
  LLRP_tSISO18K6BCustomParameters         *pISO18k6bCustParam;
  LLRP_tSThingMagicISO18K6BModulationDepth *pISO18k6bModDepth;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic Protocol configuration is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize Protocol configuration */
  pProtoConfig = LLRP_ThingMagicProtocolConfiguration_construct();

  /* Initialize ISO 18K6B modulation Depth */
  pISO18k6bModDepth = LLRP_ThingMagicISO18K6BModulationDepth_construct();
  pISO18k6bModDepth->eISO18K6BModulationDepth = (LLRP_tEThingMagicCustom18K6BModulationDepth)*modDepth;

  /* Initialize ISO 18K6B custom parameter */
  pISO18k6bCustParam = LLRP_ISO18K6BCustomParameters_construct();
  LLRP_ISO18K6BCustomParameters_setThingMagicISO18K6BModulationDepth(pISO18k6bCustParam, pISO18k6bModDepth);

  /* Set ISO 18K6B Custom parameter to protocol configuration */
  LLRP_ThingMagicProtocolConfiguration_setISO18K6BCustomParameters(pProtoConfig,
      pISO18k6bCustParam);

  /* Add ThingMagic Protocol configuration as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pProtoConfig);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to get ISO18K6B link frequency value
 *
 * @param reader Reader pointer
 * @param linkFreq Pointer to TMR_ISO180006B_linkFrequency object to hold the ISO 18K6B linkfrequency value
 */ 
TMR_Status
TMR_LLRP_cmdGetISO18K6BLinkFrequency(TMR_Reader *reader, TMR_ISO180006B_LinkFrequency *linkFreq)
{
  TMR_Status ret;
  LLRP_tSGET_READER_CONFIG                    *pCmd;
  LLRP_tSMessage                              *pCmdMsg;
  LLRP_tSMessage                              *pRspMsg;
  LLRP_tSGET_READER_CONFIG_RESPONSE           *pRsp;
  LLRP_tSThingMagicDeviceControlConfiguration *pTMProtoConfig;
  LLRP_tSParameter                            *pCustParam;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_READER_CONFIG message
   **/
  pCmd = LLRP_GET_READER_CONFIG_construct();
  LLRP_GET_READER_CONFIG_setRequestedData(pCmd, LLRP_GetReaderConfigRequestedData_Identification);

  /**
   * "/reader/iso180006b/BLF" is a custom parameter. And is available as part of
   * ThingMagicDeviceControlConfiguration.
   * Initialize the custom parameter
   **/
  pTMProtoConfig = LLRP_ThingMagicDeviceControlConfiguration_construct();
  if (NULL == pTMProtoConfig)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  /**
   * Set the requested data (i.e., protocol configuration)
   * And add to GET_READER_CONFIG message.
   **/
  LLRP_ThingMagicDeviceControlConfiguration_setRequestedData(pTMProtoConfig, 
      LLRP_ThingMagicControlConfiguration_ThingMagicProtocolConfiguration);
  if (LLRP_RC_OK != LLRP_GET_READER_CONFIG_addCustom(pCmd, &pTMProtoConfig->hdr))
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pTMProtoConfig);
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  pCmdMsg       = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, extract Iso linkfreq from response
   **/
  pCustParam = LLRP_GET_READER_CONFIG_RESPONSE_beginCustom(pRsp);
  if (NULL != pCustParam)
  {
    LLRP_tSISO18K6BCustomParameters *iso18k6bCustParam;
    LLRP_tSThingMagicISO18K6BLinkFrequency  *iso18k6blinkFreq;

    iso18k6bCustParam = LLRP_ThingMagicProtocolConfiguration_getISO18K6BCustomParameters(
        (LLRP_tSThingMagicProtocolConfiguration *)pCustParam);

    if (NULL != iso18k6bCustParam) 
    {
      iso18k6blinkFreq =  LLRP_ISO18K6BCustomParameters_getThingMagicISO18K6BLinkFrequency(iso18k6bCustParam);
      *linkFreq = (TMR_ISO180006B_LinkFrequency)iso18k6blinkFreq->eISO18K6BLinkFrequency;
    }
    else
    {
      return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
    }
  }
  else
  {
    return TMR_ERROR_LLRP_MSG_PARSE_ERROR;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to Set ISO18K6B linkFrequency value
 *
 * @param reader Reader pointer
 * @param linkFreq Pointer to TMR_ISO180006B_linkFrequency which needs to be set
 */
TMR_Status
TMR_LLRP_cmdSetISO18K6BLinkFrequency(TMR_Reader *reader, TMR_ISO180006B_LinkFrequency *linkFreq)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                *pCmd;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSMessage                          *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE       *pRsp;
  LLRP_tSThingMagicProtocolConfiguration  *pProtoConfig;
  LLRP_tSISO18K6BCustomParameters         *pISO18k6bCustParam;
  LLRP_tSThingMagicISO18K6BLinkFrequency  *pISO18k6blinkFreq;

  ret = TMR_SUCCESS;
  /**
   * Thingmagic Protocol configuration is available as a custom parameter
   * and can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize Protocol configuration */
  pProtoConfig = LLRP_ThingMagicProtocolConfiguration_construct();

  /* Initialize ISO 18K6B linkFrequency */
  pISO18k6blinkFreq = LLRP_ThingMagicISO18K6BLinkFrequency_construct();
  pISO18k6blinkFreq->eISO18K6BLinkFrequency = (LLRP_tEThingMagicCustom18K6BLinkFrequency)*linkFreq;

  /* Initialize ISO 18K6B custom parameter */
  pISO18k6bCustParam = LLRP_ISO18K6BCustomParameters_construct();
  LLRP_ISO18K6BCustomParameters_setThingMagicISO18K6BLinkFrequency(pISO18k6bCustParam, pISO18k6blinkFreq);

  /* Set ISO 18K6B Custom parameter to protocol configuration */
  LLRP_ThingMagicProtocolConfiguration_setISO18K6BCustomParameters(pProtoConfig,
      pISO18k6bCustParam);

  /* Add ThingMagic Protocol configuration as a custom parameter*/
  LLRP_SET_READER_CONFIG_addCustom(pCmd, (LLRP_tSParameter *)pProtoConfig);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}
#endif /* TMR_ENABLE_ISO180006B */

TMR_Status
TMR_LLRP_stopActiveROSpecs(TMR_Reader *reader)
{
  TMR_Status ret;
  LLRP_tSGET_ROSPECS                    *pCmd;
  LLRP_tSMessage                        *pCmdMsg;
  LLRP_tSMessage                        *pRspMsg;
  LLRP_tSGET_ROSPECS_RESPONSE           *pRsp;
  LLRP_tSROSpec                         *pROSpec;

  ret = TMR_SUCCESS;
  /**
   * Initialize the GET_ROSPECS message
   **/
  pCmd = LLRP_GET_ROSPECS_construct();
  pCmdMsg = &pCmd->hdr;
  
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSGET_ROSPECS_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus)) 
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }

  /**
   * Response is success, extract list of ROSpecs from response
   **/
  for (pROSpec = LLRP_GET_ROSPECS_RESPONSE_beginROSpec(pRsp);
        NULL != pROSpec;
        pROSpec = LLRP_GET_ROSPECS_RESPONSE_nextROSpec(pROSpec))
  {
    /* If this ROSpec is actively running, then stop it */
    if (LLRP_ROSpecState_Active == pROSpec->eCurrentState)
    {
      reader->u.llrpReader.roSpecId = pROSpec->ROSpecID;
      ret = TMR_LLRP_cmdStopROSpec(reader, true);
    }
    if (LLRP_ROSpecStartTriggerType_Periodic ==
        pROSpec->pROBoundarySpec->pROSpecStartTrigger->eROSpecStartTriggerType)
    {
      reader->u.llrpReader.roSpecId = pROSpec->ROSpecID;
      ret = TMR_LLRP_cmdDisableROSpec(reader);
    }
  }

  /* Revert back to default value */
  reader->u.llrpReader.roSpecId = 0;
  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

TMR_Status
TMR_LLRP_enableEventsAndReports(TMR_Reader *reader)
{
  TMR_Status ret;
  LLRP_tSENABLE_EVENTS_AND_REPORTS *pCmd;
  LLRP_tSMessage                   *pCmdMsg;

  ret = TMR_SUCCESS;

  /**
   * Initialize ENABLE_EVENTS_AND_REPORTS message
   **/
  pCmd = LLRP_ENABLE_EVENTS_AND_REPORTS_construct();
  pCmdMsg = &pCmd->hdr;
 
  /**
   * For LLRP_ENABLE_EVENTS_AND_REPORTS message, there
   * will be no response. So just send the message
   **/
  ret = TMR_LLRP_sendMessage(reader, pCmdMsg, 
          reader->u.llrpReader.transportTimeout);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  return ret;
}

TMR_Status
TMR_LLRP_setHoldEventsAndReportsStatus(TMR_Reader *reader, llrp_u1_t status)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                *pCmd;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSMessage                          *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE       *pRsp;
  LLRP_tSEventsAndReports                 *pEvents;

  ret = TMR_SUCCESS;
  /**
   * EventsAndReports can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize EventsAndReports  */
  pEvents = LLRP_EventsAndReports_construct();
  LLRP_EventsAndReports_setHoldEventsAndReportsUponReconnect(pEvents, status); 


  /* Add EventsAndReports to SET_READER_CONFIG*/
  LLRP_SET_READER_CONFIG_setEventsAndReports(pCmd, pEvents);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus)) 
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}


TMR_Status
TMR_LLRP_setKeepAlive(TMR_Reader *reader)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                *pCmd;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSMessage                          *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE       *pRsp;
  LLRP_tSKeepaliveSpec                    *pKeepAlive;

  ret = TMR_SUCCESS;
  /**
   * Keep alive can be set through SET_READER_CONFIG
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize Keep Alive Spec  */
  pKeepAlive = LLRP_KeepaliveSpec_construct();
  LLRP_KeepaliveSpec_setKeepaliveTriggerType(pKeepAlive, LLRP_KeepaliveTriggerType_Periodic);
  LLRP_KeepaliveSpec_setPeriodicTriggerValue(pKeepAlive, TMR_LLRP_KEEP_ALIVE_TIMEOUT);


  /* Add KeepaliveSpec to SET_READER_CONFIG*/
  LLRP_SET_READER_CONFIG_setKeepaliveSpec(pCmd, pKeepAlive);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus)) 
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  /**
   * Start LLRP background receiver
   **/
  return TMR_LLRP_startBackgroundReceiver(reader);
}

TMR_Status
TMR_LLRP_handleReaderEvents(TMR_Reader *reader, LLRP_tSMessage *pMsg)
{
  LLRP_tSReaderEventNotificationData *pEventData;
  LLRP_tSReaderExceptionEvent *pReaderExceptionEvent;
  llrp_utf8v_t  exceptionMessage;
  TMR_Status ret;
  
  ret = TMR_SUCCESS;
  pEventData = ((LLRP_tSREADER_EVENT_NOTIFICATION *)pMsg)->pReaderEventNotificationData;

  /**
   * Currently we care only for ROSpec event
   **/
  if (NULL != pEventData->pROSpecEvent)
  {
    /**
     * Check for ROSpec event type.
     * We need End of ROSpec event only for sync reads.
     **/
    if ((LLRP_ROSpecEventType_End_Of_ROSpec ==
        pEventData->pROSpecEvent->eEventType) &&
        (false == reader->continuousReading))
    {
      /**
       * If end of rospec, then decrement
       * numROSpecEvents...
       **/
      pthread_mutex_lock(&reader->u.llrpReader.receiverLock);
      reader->u.llrpReader.numOfROSpecEvents --;
      if(reader->u.llrpReader.numOfROSpecEvents <= 0 ) {
        pthread_cond_broadcast(&reader->u.llrpReader.receiverCond);
      }
      pthread_mutex_unlock(&reader->u.llrpReader.receiverLock);
    }
  }

  /**
   * Place for handling other events.
   * For future use.
   **/

  pReaderExceptionEvent = LLRP_ReaderEventNotificationData_getReaderExceptionEvent(pEventData);

  if (NULL != pReaderExceptionEvent)
  {
    exceptionMessage = LLRP_ReaderExceptionEvent_getMessage(pReaderExceptionEvent);
    ret = atoi((const char*)exceptionMessage.pValue);
    notify_exception_listeners(reader, ret);
  }
  /**
   * Done with reader event notification message.
   * Free it.
   **/
  TMR_LLRP_freeMessage(pMsg);
  return ret;
}

TMR_Status
TMR_LLRP_handleKeepAlive(TMR_Reader *reader, LLRP_tSMessage *pMsg)
{
  TMR_Status ret;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSKEEPALIVE_ACK                    *pAck;

  ret = TMR_SUCCESS;

  /**
   * Free keepalive message, assuming the message type
   * is keepalivespec.
   **/
  TMR_LLRP_freeMessage(pMsg);

  /**
   * Send keep alive acknowledgement
   **/
  pAck = LLRP_KEEPALIVE_ACK_construct();
  pCmdMsg = &pAck->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_sendMessage(reader, pCmdMsg,
                reader->u.llrpReader.transportTimeout);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /*
   * Response is success, and done with the command
   */
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pAck);

  return TMR_SUCCESS;
}

static void *
llrp_receiver_thread(void *arg)
{
  TMR_Status ret;
  TMR_Reader *reader;
  TMR_LLRP_LlrpReader *lr;
  LLRP_tSMessage *pMsg;
  LLRP_tSConnection *pConn;
  struct timeval tv;
  fd_set set;
  bool ka_start_flag = true;
  bool receive_failed = false;
  static bool mutex_lock_enabled = false;

  ret = TMR_SUCCESS;
  reader = arg;
  lr = &reader->u.llrpReader;

  /**
   * You are going to kill me, if you set runInBackground to false.
   * Please be kind enough !!
   **/
  while (true)
  {
    pConn = reader->u.llrpReader.pConn;
    if (NULL != pConn)
    {
      pthread_mutex_lock(&lr->receiverLock);
      lr->receiverRunning = false;
      pthread_cond_broadcast(&lr->receiverCond);
      while (false == lr->receiverEnabled)
      {
        pthread_cond_wait(&lr->receiverCond, &lr->receiverLock);
      }
    
      lr->receiverRunning = true;
      pthread_mutex_unlock(&lr->receiverLock);

      FD_ZERO(&set);
      FD_SET(pConn->fd, &set);
      tv.tv_sec = 0;
      tv.tv_usec = BACKGROUND_RECEIVER_LOOP_PERIOD * 1000;
	  {
#if defined(WIN32)|| defined(WINCE)
        int nfds = -1;
#else
        int nfds = pConn->fd + 1;
#endif
        ret = select(nfds, &set, NULL, NULL, &tv);
	  }
      if (0 < ret)
      {
        if(true == reader->continuousReading)
        {
          pthread_mutex_lock(&reader->u.llrpReader.receiverLock);
          mutex_lock_enabled = true;
        }
        /* check for new message in Inbox */
        ret = TMR_LLRP_receiveMessage(reader, &pMsg, lr->transportTimeout);
        if (TMR_SUCCESS != ret)
        {
          /**
           * If not success, then could be that no message
           * has been arrived yet, lets wait for some more time.
           * Do nothing here
           **/
          receive_failed = true;
          if(true == mutex_lock_enabled)
          {
            pthread_mutex_unlock(&reader->u.llrpReader.receiverLock);
            mutex_lock_enabled = false;
          }
        }
        else
        {
          /**
           * Ahaa!! New message has arrived
           * process received Message.
           **/
          ret = TMR_LLRP_processReceivedMessage(reader, pMsg);
          ka_start_flag = true;
          receive_failed = false;
          if((true == reader->continuousReading) || (true == mutex_lock_enabled))
          {
            pthread_mutex_unlock(&reader->u.llrpReader.receiverLock);
            mutex_lock_enabled = false;
          }
        }
      }
      else
      {
        /**
         * The select call returns 0. Could be that there is no data
         * to read because of connection problem.
         **/
        receive_failed = true;
      }
    }
    else
    {
      receive_failed = true;
      /* We shouldn't be in here if pConn is NULL, but if it is sleep for a little bit and hopefully things will get better */
      tmr_sleep(1000);
    }

    if(true == receive_failed)
    {
      /**
       * The select call has failed. Could be that there is no data
       * to read because of connection problem. Wait to see if
       * the connection recovers back.
       **/
      uint64_t diffTime;

      if (ka_start_flag)
      {
        reader->u.llrpReader.ka_start = tmr_gettime();
        pthread_mutex_lock(&reader->u.llrpReader.receiverLock);
        reader->u.llrpReader.keepAliveAckMissCnt = 0;
        pthread_mutex_unlock(&reader->u.llrpReader.receiverLock);
        ka_start_flag = false;
      }
      reader->u.llrpReader.ka_now = tmr_gettime();
      diffTime = reader->u.llrpReader.ka_now - reader->u.llrpReader.ka_start;
      /* Count the number of KEEP_ALIVEs missed.*/
      if(((TMR_LLRP_KEEP_ALIVE_TIMEOUT * (reader->u.llrpReader.keepAliveAckMissCnt + 1)) < diffTime) && 
         (reader->u.llrpReader.keepAliveAckMissCnt < MAX_KEEP_ALIVE_ACK_MISSES))
      {
        pthread_mutex_lock(&reader->u.llrpReader.receiverLock);
        reader->u.llrpReader.keepAliveAckMissCnt++;
        pthread_mutex_unlock(&reader->u.llrpReader.receiverLock);
      }
      if ((TMR_LLRP_KEEP_ALIVE_TIMEOUT * 4) < diffTime)
      {
        if(reader->u.llrpReader.keepAliveAckMissCnt >= MAX_KEEP_ALIVE_ACK_MISSES)
        {
          pthread_mutex_lock(&reader->u.llrpReader.receiverLock);
          reader->u.llrpReader.keepAliveAckMissCnt = 0;
          pthread_mutex_unlock(&reader->u.llrpReader.receiverLock);
          /**
           * We have waited for enough time (4 times keep alive duration), 
           * and still there is no response from reader. 
           * Connection might be lost. Indicate an error so that the
           * continuous reading will be stopped.
           **/

          /**
           * Set numOfROSpec events to -1, indicating an unknown error
           * occured during the read process.
           **/
          pthread_mutex_lock(&lr->receiverLock);
          lr->numOfROSpecEvents = -1;
          pthread_cond_broadcast(&lr->receiverCond);
          pthread_mutex_unlock(&lr->receiverLock);
          ka_start_flag = true;
        }
      }
    }

    if (true == reader->u.llrpReader.threadCancel)
    {
      /** Time to exit */
      pthread_exit(NULL);
    }
  } /* End of while */
  return NULL;
}

TMR_Status
TMR_LLRP_processReceivedMessage(TMR_Reader *reader, LLRP_tSMessage *pMsg)
{
  TMR_Status ret;
  TMR_LLRP_LlrpReader *lr;

  ret = TMR_SUCCESS;
  lr = &reader->u.llrpReader;

  /* Check if it is a keepalive */
  if (&LLRP_tdKEEPALIVE == pMsg->elementHdr.pType)
  {
    /**
     * Handle keep alive messages, no need
     * for error checking.
     **/
    ret = TMR_LLRP_handleKeepAlive(reader, pMsg);
  }

  /**
   * Check if it is a RO_ACCESS_REPORT
   **/
  else if (&LLRP_tdRO_ACCESS_REPORT == pMsg->elementHdr.pType)
  {
    /**
     * Handle RO_ACCESS_REPORTS,
     * We receive RO_ACCESS_REPORTS here only incase of sync read.
     * Buffer the message pointer, so that it can be used later.
     **/
    if (NULL == lr->bufResponse)
    {
      /* We haven't had opportunity to allocate the buffer yet, so allocate the
       * first
       */
      lr->bufResponse = (LLRP_tSMessage **) malloc( 1 * sizeof(LLRP_tSMessage *));
      lr->bufPointer = 0;
    }
    lr->bufResponse[lr->bufPointer ] = pMsg;

    /**
     * Reallocate the memory for bufResponse, so that it can hold the
     * pointer to next RO_ACCESS_REPORT in case if it is received.
     **/
    {
      LLRP_tSMessage **newResponse;

      newResponse = realloc(lr->bufResponse, (lr->bufPointer + 2) * sizeof(lr->bufResponse[0]));
      if (NULL == newResponse)
      {
        /**
         * FIXME: How to handle this error. This would'nt happen
         * in general, but can expect when running on reader.
         * How to handle this.
         **/
      }
      lr->bufResponse = newResponse;
    }

    if(reader->continuousReading)
    {
        process_async_response(reader);
    }
    /**
     * Do not free pMsg here. We hold that memory for further
     * processing of tagReads, and will be freed later.
     **/
    else
    {
        lr->bufPointer += 1;
    }
  }

  /**
   * Check if it is a ReaderEventNotification
   **/
  else if (&LLRP_tdREADER_EVENT_NOTIFICATION  == pMsg->elementHdr.pType)
  {
    /**
     * Handle Reader event notifications.
     **/
    ret = TMR_LLRP_handleReaderEvents(reader, pMsg);
  }

  else
  {
    /**
     * What kind of message is this. I do not
     * know how to handle, ignoring for now
     * and free the message.
     **/
    TMR_LLRP_freeMessage(pMsg);
  }

  return ret;
}

void
TMR_LLRP_setBackgroundReceiverState(TMR_Reader *reader, bool state)
{
  bool receiverSetup;
  pthread_mutex_lock(&reader->u.llrpReader.receiverLock);
  receiverSetup = reader->u.llrpReader.receiverSetup;
  pthread_mutex_unlock(&reader->u.llrpReader.receiverLock);
  if (true == receiverSetup)
  {
    if (false == state)
    {
      /**
       * Disable background receiving
       **/
      pthread_mutex_lock(&reader->u.llrpReader.receiverLock);
      reader->u.llrpReader.receiverEnabled = false;
      while (true == reader->u.llrpReader.receiverRunning)
      {
        pthread_cond_wait(&reader->u.llrpReader.receiverCond, &reader->u.llrpReader.receiverLock);
      }
      pthread_mutex_unlock(&reader->u.llrpReader.receiverLock);
    }
    else
    {
      /**
       * Enable background receiving
       **/
      pthread_mutex_lock(&reader->u.llrpReader.receiverLock);
      reader->u.llrpReader.receiverEnabled = true;
      pthread_cond_broadcast(&reader->u.llrpReader.receiverCond);
      pthread_mutex_unlock(&reader->u.llrpReader.receiverLock);
    }
  }
}

TMR_Status
TMR_LLRP_startBackgroundReceiver(TMR_Reader *reader)
{
  TMR_Status ret;
  TMR_LLRP_LlrpReader *lr = &reader->u.llrpReader;

  ret = TMR_SUCCESS;

  /* Initialize background llrp receiver */
  pthread_mutex_lock(&lr->receiverLock);

  ret = pthread_create(&lr->llrpReceiver, NULL,
                      llrp_receiver_thread, reader);
  if (0 != ret)
  {
    pthread_mutex_unlock(&lr->receiverLock);
    return TMR_ERROR_NO_THREADS;
  }
  
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  
  lr->receiverSetup = true;
  lr->receiverEnabled = true;
  pthread_mutex_unlock(&lr->receiverLock);

  return TMR_SUCCESS;
}

TMR_Status
TMR_LLRP_cmdSetEventNotificationSpec(TMR_Reader *reader, bool state)
{
  TMR_Status ret;
  LLRP_tSSET_READER_CONFIG                *pCmd;
  LLRP_tSMessage                          *pCmdMsg;
  LLRP_tSMessage                          *pRspMsg;
  LLRP_tSSET_READER_CONFIG_RESPONSE       *pRsp;
  LLRP_tSReaderEventNotificationSpec      *pEventNotificationSpec;
  LLRP_tSEventNotificationState           *pEventNotificationState;

  ret = TMR_SUCCESS;
  /**
   * Reader event notifications can be set through
   * Initialize SET_READER_CONFIG
   **/
  pCmd = LLRP_SET_READER_CONFIG_construct();

  /* Initialize Reader Event Notification spec */
  pEventNotificationSpec = LLRP_ReaderEventNotificationSpec_construct();

  /**
   * Initialize events
   * Add ROSPEC event and Reader Exception events
   **/
  pEventNotificationState = LLRP_EventNotificationState_construct();
  LLRP_EventNotificationState_setEventType(pEventNotificationState,
                                   LLRP_NotificationEventType_ROSpec_Event);
  LLRP_EventNotificationState_setNotificationState(pEventNotificationState, (uint8_t)state);

  /* Set Event to EventNotificationSpec */
  LLRP_ReaderEventNotificationSpec_addEventNotificationState(pEventNotificationSpec,
                                                    pEventNotificationState);


  pEventNotificationState = LLRP_EventNotificationState_construct();
  LLRP_EventNotificationState_setEventType(pEventNotificationState,
                                   LLRP_NotificationEventType_Reader_Exception_Event);
  LLRP_EventNotificationState_setNotificationState(pEventNotificationState, (uint8_t)state);

  /* Set Event to EventNotificationSpec */
  LLRP_ReaderEventNotificationSpec_addEventNotificationState(pEventNotificationSpec,
                                                    pEventNotificationState);
  /* Add EventNotificationSpec to SET_READER_CONFIG*/
  LLRP_SET_READER_CONFIG_setReaderEventNotificationSpec(pCmd, pEventNotificationSpec);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP;
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);
  return ret;
}

/**
 * Command to Enable AccessSpec
 *
 * @param reader Reader pointer
 * @param accessSpecId AccessSpec Id
 */
TMR_Status
TMR_LLRP_cmdEnableAccessSpec(TMR_Reader *reader, llrp_u32_t accessSpecId)
{
  TMR_Status ret;
  LLRP_tSENABLE_ACCESSSPEC          *pCmd;
  LLRP_tSMessage                    *pCmdMsg;
  LLRP_tSMessage                    *pRspMsg;
  LLRP_tSENABLE_ACCESSSPEC_RESPONSE *pRsp;
  
  ret = TMR_SUCCESS;

  /**
   * Initialize EnableAccessSpec message
   **/
  pCmd = LLRP_ENABLE_ACCESSSPEC_construct();
  LLRP_ENABLE_ACCESSSPEC_setAccessSpecID(pCmd, accessSpecId);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSENABLE_ACCESSSPEC_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Prepare AccessCommand
 *
 * @param reader Reader pointer
 * @param pAccessCommand Pointer to AccessCommand parameter
 * @param filter Pointer to Tag filter
 * @param tagop Pointer to TMR_TagOp
 */
TMR_Status
TMR_LLRP_msgPrepareAccessCommand(TMR_Reader *reader,
                                  LLRP_tSAccessCommand *pAccessCommand, 
                                  TMR_TagFilter *filter, 
                                  TMR_TagOp *tagop)
{
  TMR_Status ret;

  ret = TMR_SUCCESS;

  /**
   * 1. Prepare and Add TagSpec
   **/
  {
    LLRP_tSC1G2TagSpec *pTagSpec;
    LLRP_tSC1G2TargetTag *pTargetTag;

    /* Initialize TagSpec */
    pTagSpec = LLRP_C1G2TagSpec_construct();
    pTargetTag = LLRP_C1G2TargetTag_construct();
  
    /**
     * TagFilter is not supported as part of AccessSpec's TagSpec.
     * So nothing to do with the filter passed in.
     * All we do is construct a dummy TagSpec and set as a 
     * TargetTag.
     **/
    
    /* Add TargetTag to TagSpec */
    LLRP_C1G2TagSpec_addC1G2TargetTag(pTagSpec, pTargetTag);

    /**
     * Add TagSpec to AccessSpec
     **/
    LLRP_AccessCommand_setAirProtocolTagSpec(pAccessCommand,
                                              (LLRP_tSParameter *)pTagSpec);
  }

  /**
   * 2. Prepare and Add OpSpec
   **/
  {
    reader->u.llrpReader.opSpecId ++;

    /**
     * Check for the tagop type and add appropriate
     * OpSpec.
     **/
PREPARE_COMMAND:
    switch (tagop->type)
    {
      case TMR_TAGOP_GEN2_READDATA:
        {
          TMR_TagOp_GEN2_ReadData *args;
          LLRP_tSC1G2Read         *pC1G2Read;

          args = &tagop->u.gen2.u.readData;
          
          /* Construct and initialize C1G2Read */
          pC1G2Read = LLRP_C1G2Read_construct();
          /* Set OpSpec Id */
          LLRP_C1G2Read_setOpSpecID(pC1G2Read, reader->u.llrpReader.opSpecId);
          /* Set access password */
          LLRP_C1G2Read_setAccessPassword(pC1G2Read, reader->u.llrpReader.gen2AccessPassword);
          /* Set Memory Bank */
          if ((args->bank == TMR_GEN2_BANK_RESERVED) || (args->bank == TMR_GEN2_BANK_EPC) ||
              (args->bank == TMR_GEN2_BANK_TID) || (args->bank == TMR_GEN2_BANK_USER))
          {
            LLRP_C1G2Read_setMB(pC1G2Read, (llrp_u2_t)args->bank);
          }
          else
          {
            return TMR_ERROR_INVALID; 
          }
          /* Set word pointer */
          LLRP_C1G2Read_setWordPointer(pC1G2Read, (llrp_u16_t)args->wordAddress);
          /* Set word length to read */
          LLRP_C1G2Read_setWordCount(pC1G2Read, args->len);

         /**
          * Set C1G2Read as OpSpec to accessSpec
          **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand, 
                                          (LLRP_tSParameter *)pC1G2Read); 
          break;
        }

      case TMR_TAGOP_GEN2_BLOCKERASE:
        {
          TMR_TagOp_GEN2_BlockErase *args;
          LLRP_tSC1G2BlockErase     *pC1G2BlockErase;

          args = &tagop->u.gen2.u.blockErase;

          /* Construct and initialize C1G2BlockErase */
          pC1G2BlockErase = LLRP_C1G2BlockErase_construct();
          /* Set OpSpec Id */
          LLRP_C1G2BlockErase_setOpSpecID(pC1G2BlockErase, reader->u.llrpReader.opSpecId);
          /* Set access password */
          LLRP_C1G2BlockErase_setAccessPassword(pC1G2BlockErase, reader->u.llrpReader.gen2AccessPassword);
          /* Set Memory Bank */
          if ((args->bank == TMR_GEN2_BANK_RESERVED) || (args->bank == TMR_GEN2_BANK_EPC) ||
              (args->bank == TMR_GEN2_BANK_TID) || (args->bank == TMR_GEN2_BANK_USER))
          {
            LLRP_C1G2BlockErase_setMB(pC1G2BlockErase, (llrp_u2_t)args->bank);
          }
          else
          {
            return TMR_ERROR_INVALID; 
          }
          /* Set word pointer */
          LLRP_C1G2BlockErase_setWordPointer(pC1G2BlockErase, (llrp_u16_t)args->wordPtr);
          /* Set word count to erase */
          LLRP_C1G2BlockErase_setWordCount(pC1G2BlockErase, (llrp_u16_t)args->wordCount);

          /**
           * Set C1G2BlockErase as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                                      (LLRP_tSParameter *)pC1G2BlockErase);
          break;
        }

      case TMR_TAGOP_GEN2_WRITETAG:
        {
          TMR_TagOp_GEN2_WriteTag     *args;
          LLRP_tSThingMagicWriteTag   *pC1G2WriteTag;
          llrp_u16v_t                  data;
          int                          i,j;

          args = &tagop->u.gen2.u.writeTag;

          /* Construct and initialize ThingMagicWriteTag */
          pC1G2WriteTag = LLRP_ThingMagicWriteTag_construct();
          /* Set OpSpec Id */
          LLRP_ThingMagicWriteTag_setOpSpecID(pC1G2WriteTag, reader->u.llrpReader.opSpecId);
          /* Set access password */
          LLRP_ThingMagicWriteTag_setAccessPassword(pC1G2WriteTag, reader->u.llrpReader.gen2AccessPassword);
          /* Set the data to be written */
          /* As API epc datatype is uint8_t but
           * llrp takes epc of type uint16_t
           * so, mapping the uint8_t data into uint16_t
           */
          data = LLRP_u16v_construct((llrp_u16_t) (args->epcptr->epcByteCount / 2));
          for(i = 0, j = 0;i < data.nValue; i++, j+=2)
          {
            data.pValue[i] = (args->epcptr->epc[j] << 8) | args->epcptr->epc[j + 1];
          }

          LLRP_ThingMagicWriteTag_setWriteData (pC1G2WriteTag, data);

          /**
           * Set ThingMagicWriteTag as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pC1G2WriteTag);
         
          break;
        }

      case TMR_TAGOP_GEN2_WRITEDATA:
        {
          TMR_TagOp_GEN2_WriteData *args;
          LLRP_tSC1G2Write         *pC1G2WriteData;
          llrp_u16v_t               data;

          args = &tagop->u.gen2.u.writeData;

          /* Construct and initialize C1GWrite */
          pC1G2WriteData = LLRP_C1G2Write_construct();
          /* Set OpSpec Id */
          LLRP_C1G2Write_setOpSpecID(pC1G2WriteData, reader->u.llrpReader.opSpecId);
          /* Set access password */
          LLRP_C1G2Write_setAccessPassword(pC1G2WriteData, reader->u.llrpReader.gen2AccessPassword);
          /* Set Memory Bank */
          if ((args->bank == TMR_GEN2_BANK_RESERVED) || (args->bank == TMR_GEN2_BANK_EPC) ||
              (args->bank == TMR_GEN2_BANK_TID) || (args->bank == TMR_GEN2_BANK_USER))
          {
            LLRP_C1G2Write_setMB(pC1G2WriteData, (llrp_u2_t)args->bank);
          }
          else
          {
            return TMR_ERROR_INVALID; 
          }
          /* Set word pointer */
          LLRP_C1G2Write_setWordPointer(pC1G2WriteData, (llrp_u16_t)args->wordAddress);
          /* Set the data to be written */
          data = LLRP_u16v_construct (args->data.len);
          memcpy (data.pValue, args->data.list, data.nValue * sizeof(uint16_t));
          LLRP_C1G2Write_setWriteData (pC1G2WriteData, data);

          /**
           * Set C1G2WriteData as OpSpec to accessSpec
           **/
          
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pC1G2WriteData);
         
          break;
        }

      case TMR_TAGOP_GEN2_BLOCKPERMALOCK:
        {
          TMR_TagOp_GEN2_BlockPermaLock      *args;
          LLRP_tSThingMagicBlockPermalock    *pC1G2BlockPermaLock;
          llrp_u16v_t                        mask;

          args = &tagop->u.gen2.u.blockPermaLock;

          /* Construct and initialize BlockPermaLock */
          pC1G2BlockPermaLock = LLRP_ThingMagicBlockPermalock_construct();
          /* Set OpSpec Id */
          LLRP_ThingMagicBlockPermalock_setOpSpecID(pC1G2BlockPermaLock, 
                                            reader->u.llrpReader.opSpecId);
          /* Set access password */
          LLRP_ThingMagicBlockPermalock_setAccessPassword(pC1G2BlockPermaLock,
                                      reader->u.llrpReader.gen2AccessPassword);
          /* Set ReadLock */
          LLRP_ThingMagicBlockPermalock_setReadLock(pC1G2BlockPermaLock, args->readLock);
          /* Set Memory Bank */
          if ((args->bank == TMR_GEN2_BANK_RESERVED) || (args->bank == TMR_GEN2_BANK_EPC) ||
              (args->bank == TMR_GEN2_BANK_TID) || (args->bank == TMR_GEN2_BANK_USER))
          {
            LLRP_ThingMagicBlockPermalock_setMB(pC1G2BlockPermaLock, (llrp_u2_t)args->bank);
          }
          else
          {
            return TMR_ERROR_INVALID; 
          }
          /* Set block pointer */
          LLRP_ThingMagicBlockPermalock_setBlockPointer(pC1G2BlockPermaLock, 
                                                            args->blockPtr);
          /* Set block range mask */
          mask = LLRP_u16v_construct(args->mask.len);
          memcpy(mask.pValue, args->mask.list, mask.nValue * sizeof(uint16_t));
          LLRP_ThingMagicBlockPermalock_setBlockMask(pC1G2BlockPermaLock, mask);

          /**
           * Set BlockPermaLock as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                                      (LLRP_tSParameter *)pC1G2BlockPermaLock);
 
          break;
        }

      case TMR_TAGOP_GEN2_ALIEN_HIGGS2_PARTIALLOADIMAGE:
        {
          TMR_TagOp_GEN2_Alien_Higgs2_PartialLoadImage *args;
          LLRP_tSThingMagicHiggs2PartialLoadImage      *pHiggs2PartialLoadImage;
          llrp_u8v_t                                    epc;

          args = &tagop->u.gen2.u.custom.u.alien.u.higgs2.u.partialLoadImage;

          /**
           * Error check
           **/
          if ((args->epcptr->epcByteCount > 12) || 
              (args->epcptr->epcByteCount <=0))
          { 
            /* Only 96 bit epc */
            return TMR_ERROR_PROTOCOL_INVALID_EPC;
          }    

          /* Construct and initialize Higgs2PartialLoadImage */
          pHiggs2PartialLoadImage = LLRP_ThingMagicHiggs2PartialLoadImage_construct();
          /* Set OpSpec Id */
          LLRP_ThingMagicHiggs2PartialLoadImage_setOpSpecID(pHiggs2PartialLoadImage, 
                                                      reader->u.llrpReader.opSpecId);
          /* Set access password to use to write on tag */
          LLRP_ThingMagicHiggs2PartialLoadImage_setCurrentAccessPassword(
              pHiggs2PartialLoadImage, reader->u.llrpReader.gen2AccessPassword);
          /* Set Kill Password to be written on tag */
          LLRP_ThingMagicHiggs2PartialLoadImage_setKillPassword(pHiggs2PartialLoadImage,
                                                                    args->killPassword);
          /* Set access password to be written on tag */
          LLRP_ThingMagicHiggs2PartialLoadImage_setAccessPassword(pHiggs2PartialLoadImage,
                                                                    args->accessPassword);
          /* Set EPC to be written */
          epc = LLRP_u8v_construct (args->epcptr->epcByteCount * sizeof(uint8_t));
          memcpy (epc.pValue, args->epcptr->epc, epc.nValue);
          LLRP_ThingMagicHiggs2PartialLoadImage_setEPCData(pHiggs2PartialLoadImage, epc);

          /**
           * Set Higgs2PartialLoadImage as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pHiggs2PartialLoadImage);
 
          break;
        }

      case TMR_TAGOP_GEN2_ALIEN_HIGGS2_FULLLOADIMAGE:
        {
          TMR_TagOp_GEN2_Alien_Higgs2_FullLoadImage *args;
          LLRP_tSThingMagicHiggs2FullLoadImage      *pOpSpec;
          llrp_u8v_t                                epc;

          args = &tagop->u.gen2.u.custom.u.alien.u.higgs2.u.fullLoadImage;

          /**
           * Error check
           **/
          if ((args->epcptr->epcByteCount > 12) || 
              (args->epcptr->epcByteCount <=0))
          { 
            /* Only 96 bit epc */
            return TMR_ERROR_PROTOCOL_INVALID_EPC;
          }    

          /* Construct and initialize Higgs2FullLoadImage */
          pOpSpec = LLRP_ThingMagicHiggs2FullLoadImage_construct();
          /* Set OpSpec Id */
          LLRP_ThingMagicHiggs2FullLoadImage_setOpSpecID(pOpSpec, 
                                  reader->u.llrpReader.opSpecId);
          /* Set access password to use to write on tag */
          LLRP_ThingMagicHiggs2FullLoadImage_setCurrentAccessPassword(
                      pOpSpec, reader->u.llrpReader.gen2AccessPassword);
          /* Set Kill Password to be written on tag */
          LLRP_ThingMagicHiggs2FullLoadImage_setKillPassword(pOpSpec, args->killPassword);
          /* Set access password to be written on tag */
          LLRP_ThingMagicHiggs2FullLoadImage_setAccessPassword(pOpSpec, args->accessPassword);
          /* Set LockBits */
          LLRP_ThingMagicHiggs2FullLoadImage_setLockBits(pOpSpec, args->lockBits);
          /* Set PCWord */
          LLRP_ThingMagicHiggs2FullLoadImage_setPCWord(pOpSpec, args->pcWord);
          /* Set EPC to be written */
          epc = LLRP_u8v_construct (args->epcptr->epcByteCount * sizeof(uint8_t));
          memcpy (epc.pValue, args->epcptr->epc, epc.nValue);
          LLRP_ThingMagicHiggs2FullLoadImage_setEPCData(pOpSpec, epc);

          /**
           * Set Higgs2FullLoadImage as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                                      (LLRP_tSParameter *)pOpSpec);
 
          break;
        }

      case TMR_TAGOP_GEN2_ALIEN_HIGGS3_FASTLOADIMAGE:
        {
          TMR_TagOp_GEN2_Alien_Higgs3_FastLoadImage *args;
          LLRP_tSThingMagicHiggs3FastLoadImage      *pOpSpec;
          llrp_u8v_t                                epc;

          args = &tagop->u.gen2.u.custom.u.alien.u.higgs3.u.fastLoadImage;

          /**
           * Error check
           **/
          if ((args->epcptr->epcByteCount > 12) || 
              (args->epcptr->epcByteCount <=0))
          { 
            /* Only 96 bit epc */
            return TMR_ERROR_PROTOCOL_INVALID_EPC;
          }    

          /* Construct and initialize Higgs3FastLoadImage */
          pOpSpec = LLRP_ThingMagicHiggs3FastLoadImage_construct();
          /* Set OpSpec Id */
          LLRP_ThingMagicHiggs3FastLoadImage_setOpSpecID(pOpSpec, 
                                  reader->u.llrpReader.opSpecId);
          /* Set access password to use to write on tag */
          LLRP_ThingMagicHiggs3FastLoadImage_setCurrentAccessPassword(
                                  pOpSpec, args->currentAccessPassword);
          /* Set Kill Password to be written on tag */
          LLRP_ThingMagicHiggs3FastLoadImage_setKillPassword(pOpSpec, args->killPassword);
          /* Set access password to be written on tag */
          LLRP_ThingMagicHiggs3FastLoadImage_setAccessPassword(pOpSpec, args->accessPassword);
          /* Set PCWord */
          LLRP_ThingMagicHiggs3FastLoadImage_setPCWord(pOpSpec, args->pcWord);
          /* Set EPC to be written */
          epc = LLRP_u8v_construct (args->epcptr->epcByteCount * sizeof(uint8_t));
          memcpy (epc.pValue, args->epcptr->epc, epc.nValue);
          LLRP_ThingMagicHiggs3FastLoadImage_setEPCData(pOpSpec, epc);

          /**
           * Set Higgs3FastLoadImage as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                                      (LLRP_tSParameter *)pOpSpec);
 
          break;
        }

      case TMR_TAGOP_GEN2_ALIEN_HIGGS3_LOADIMAGE:
        {
          TMR_TagOp_GEN2_Alien_Higgs3_LoadImage *args;
          LLRP_tSThingMagicHiggs3LoadImage      *pOpSpec;
          llrp_u8v_t                            epcAndUserData;

          args = &tagop->u.gen2.u.custom.u.alien.u.higgs3.u.loadImage;

          /**
           * Error Check
           **/
          if ((args->epcAndUserData->len > 76) || 
              (args->epcAndUserData->len <= 0))                                                                                                 
          { 
            /* Only 76 byte epcAndUserData */
            return TMR_ERROR_MSG_INVALID_PARAMETER_VALUE;
          }

          /* Construct and initialize Higgs3LoadImage */
          pOpSpec = LLRP_ThingMagicHiggs3LoadImage_construct();
          /* Set OpSpec Id */
          LLRP_ThingMagicHiggs3LoadImage_setOpSpecID(pOpSpec, 
                               reader->u.llrpReader.opSpecId);
          /* Set access password to use to write on tag */
          LLRP_ThingMagicHiggs3LoadImage_setCurrentAccessPassword(
                              pOpSpec, args->currentAccessPassword);
          /* Set Kill Password to be written on tag */
          LLRP_ThingMagicHiggs3LoadImage_setKillPassword(pOpSpec, args->killPassword);
          /* Set access password to be written on tag */
          LLRP_ThingMagicHiggs3LoadImage_setAccessPassword(pOpSpec, args->accessPassword);
          /* Set PCWord */
          LLRP_ThingMagicHiggs3LoadImage_setPCWord(pOpSpec, args->pcWord);
          /* Set EPC And User data to be written */
          epcAndUserData = LLRP_u8v_construct (args->epcAndUserData->len);
          memcpy (epcAndUserData.pValue, args->epcAndUserData->list,
                                 epcAndUserData.nValue);
          LLRP_ThingMagicHiggs3LoadImage_setEPCAndUserData(pOpSpec, epcAndUserData);

          /**
           * Set Higgs3LoadImage as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                                      (LLRP_tSParameter *)pOpSpec);
 
          break;
        }

      case TMR_TAGOP_GEN2_ALIEN_HIGGS3_BLOCKREADLOCK:
        {
          TMR_TagOp_GEN2_Alien_Higgs3_BlockReadLock *args;
          LLRP_tSThingMagicHiggs3BlockReadLock      *pOpSpec;

          args = &tagop->u.gen2.u.custom.u.alien.u.higgs3.u.blockReadLock;

          /* Construct and initialize Higgs3BlockReadLock */
          pOpSpec = LLRP_ThingMagicHiggs3BlockReadLock_construct();
          /* Set OpSpec Id */
          LLRP_ThingMagicHiggs3BlockReadLock_setOpSpecID(pOpSpec, 
                                  reader->u.llrpReader.opSpecId);
          /* Set AccessPassword */
          LLRP_ThingMagicHiggs3BlockReadLock_setAccessPassword(
                                  pOpSpec, args->accessPassword);
          /* Set LockBits */
          LLRP_ThingMagicHiggs3BlockReadLock_setLockBits(
                                  pOpSpec, args->lockBits);

          /**
           * Set Higgs3BlockReadLock as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                                      (LLRP_tSParameter *)pOpSpec);
 
          break;
        }

      case TMR_TAGOP_GEN2_LOCK:
        {
          TMR_TagOp_GEN2_Lock        *args;
          LLRP_tSC1G2Lock            *pC1G2Lock;
          LLRP_tSC1G2LockPayload     *pC1G2LockPayload;
          int                         index;

          args = &tagop->u.gen2.u.lock;
          /* Construct and initialize C1G2Lock */
          pC1G2Lock = LLRP_C1G2Lock_construct();
          /* Set OpSpec Id */
          LLRP_C1G2Lock_setOpSpecID(pC1G2Lock, reader->u.llrpReader.opSpecId);
          /* Set access password */
          LLRP_C1G2Lock_setAccessPassword(pC1G2Lock, args->accessPassword);
          /* Construct and initialize C1G2LockPayload */
          pC1G2LockPayload = LLRP_C1G2LockPayload_construct();
          /* Set the LockPrivilege  */
          switch (args->mask)
          {
            case TMR_GEN2_LOCK_BITS_USER_PERM:
              {
                index = 0;
                pC1G2LockPayload->eDataField = LLRP_C1G2LockDataField_User_Memory;
                if (1 == (args->action >> index))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Lock;
                }
                else
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Unlock;
                }
                break;
              }
            case TMR_GEN2_LOCK_BITS_USER:
              {
                index = 1;
                pC1G2LockPayload->eDataField = LLRP_C1G2LockDataField_User_Memory;
                if (1 == (args->action >> index))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Read_Write;
                }
                else
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Unlock;
                }
                break;
              }
            case TMR_GEN2_LOCK_BITS_MASK_USER:
              {
                pC1G2LockPayload->eDataField = LLRP_C1G2LockDataField_User_Memory;
                if( 0 == (args ->action))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Unlock;
                }
                else if( TMR_GEN2_LOCK_BITS_USER == (args->action))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Read_Write;
                }
                else if ( TMR_GEN2_LOCK_BITS_USER_PERM == (args -> action))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Unlock;
                }
                else if ( TMR_GEN2_LOCK_BITS_MASK_USER == (args -> action)) 
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Lock;
                }
                else
                {
                  return TMR_ERROR_INVALID;
                }
                break;
              }
            case TMR_GEN2_LOCK_BITS_TID_PERM:
              {
                index = 2;
                pC1G2LockPayload->eDataField = LLRP_C1G2LockDataField_TID_Memory;
                if (1 == (args->action >> index))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Lock;
                }
                else
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Unlock;
                }
                break;
              }
            case TMR_GEN2_LOCK_BITS_TID:
              {
                index = 3;
                pC1G2LockPayload->eDataField = LLRP_C1G2LockDataField_TID_Memory;
                if (1 == (args->action >> index))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Read_Write;
                }
                else
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Unlock;
                }
                break;
              }
            case TMR_GEN2_LOCK_BITS_MASK_TID:
              {
                pC1G2LockPayload->eDataField = LLRP_C1G2LockDataField_TID_Memory;
                if( 0 == (args ->action))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Unlock;
                }
                else if( TMR_GEN2_LOCK_BITS_TID == (args->action))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Read_Write;
                }
                else if ( TMR_GEN2_LOCK_BITS_TID_PERM == (args -> action))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Unlock;
                }
                else if ( TMR_GEN2_LOCK_BITS_MASK_TID == (args -> action)) 
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Lock;
                }
                else
                {
                  return TMR_ERROR_INVALID;
                }
                break;
              }
            case TMR_GEN2_LOCK_BITS_EPC_PERM:
              {
                index = 4;
                pC1G2LockPayload->eDataField = LLRP_C1G2LockDataField_EPC_Memory;
                if (1 == (args->action >> index))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Lock;
                }
                else
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Unlock;
                }
                break;
              }
            case TMR_GEN2_LOCK_BITS_EPC:
              {
                index = 5;
                pC1G2LockPayload->eDataField = LLRP_C1G2LockDataField_EPC_Memory;
                if (1 == (args->action >> index))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Read_Write;
                }
                else
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Unlock;
                }
                break;
              }
            case TMR_GEN2_LOCK_BITS_MASK_EPC:
              {
                pC1G2LockPayload->eDataField = LLRP_C1G2LockDataField_EPC_Memory;
                if( 0 == (args ->action))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Unlock;
                }
                else if( TMR_GEN2_LOCK_BITS_EPC == (args->action))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Read_Write;
                }
                else if ( TMR_GEN2_LOCK_BITS_EPC_PERM == (args -> action))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Unlock;
                }
                else if ( TMR_GEN2_LOCK_BITS_MASK_EPC == (args -> action)) 
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Lock;
                }
                else
                {
                  return TMR_ERROR_INVALID;
                }
                break;
              }
            case TMR_GEN2_LOCK_BITS_ACCESS_PERM:
              {
                index = 6;
                pC1G2LockPayload->eDataField = LLRP_C1G2LockDataField_Access_Password;
                if (1 == (args->action >> index))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Lock;
                }
                else
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Unlock;
                }
                break;
              }
            case TMR_GEN2_LOCK_BITS_ACCESS:
              {
                index = 7;
                pC1G2LockPayload->eDataField = LLRP_C1G2LockDataField_Access_Password;
                if (1 == (args->action >> index))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Read_Write;
                }
                else
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Unlock;
                }
                break;
              }
            case TMR_GEN2_LOCK_BITS_MASK_ACCESS:
              {
                pC1G2LockPayload->eDataField = LLRP_C1G2LockDataField_Access_Password;
                if( 0 == (args ->action))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Unlock;
                }
                else if( TMR_GEN2_LOCK_BITS_ACCESS == (args->action))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Read_Write;
                }
                else if ( TMR_GEN2_LOCK_BITS_ACCESS_PERM == (args -> action))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Unlock;
                }
                else if ( TMR_GEN2_LOCK_BITS_MASK_ACCESS == (args -> action)) 
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Lock;
                }
                else
                {
                  return TMR_ERROR_INVALID;
                }
                break;
              }
            case TMR_GEN2_LOCK_BITS_KILL_PERM:
              {
                index = 8;
                pC1G2LockPayload->eDataField = LLRP_C1G2LockDataField_Kill_Password;
                if (1 == (args->action >> index))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Lock;
                }
                else
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Unlock;
                }
                break;
              }
            case TMR_GEN2_LOCK_BITS_KILL:
              {
                index = 9;
                pC1G2LockPayload->eDataField = LLRP_C1G2LockDataField_Kill_Password;
                if (1 == (args->action >> index))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Read_Write;
                }
                else
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Unlock;
                }
                break;
              }
            case TMR_GEN2_LOCK_BITS_MASK_KILL:
              {
                pC1G2LockPayload->eDataField = LLRP_C1G2LockDataField_Kill_Password;
                if( 0 == (args ->action))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Unlock;
                }
                else if( TMR_GEN2_LOCK_BITS_KILL == (args->action))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Read_Write;
                }
                else if ( TMR_GEN2_LOCK_BITS_KILL_PERM == (args -> action))
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Unlock;
                }
                else if ( TMR_GEN2_LOCK_BITS_MASK_KILL == (args -> action)) 
                {
                  pC1G2LockPayload->ePrivilege = LLRP_C1G2LockPrivilege_Perma_Lock;
                }
                else
                {
                  return TMR_ERROR_INVALID;
                }
                break;
              }
            default:
              {
                /* Unknown lockaction  return invalid error */
                return TMR_ERROR_INVALID; 

              }
          }

          /**
           * Set C1G2LockPayload  to C1G2Lock
           **/
          LLRP_C1G2Lock_addC1G2LockPayload (pC1G2Lock, pC1G2LockPayload);
          /**
           * Set C1G2Lock as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pC1G2Lock);

          break;
        }

      case TMR_TAGOP_GEN2_KILL:
        {
          TMR_TagOp_GEN2_Kill   *args;
          LLRP_tSC1G2Kill       *pC1G2Kill;

          args = &tagop->u.gen2.u.kill;
          /* Construct and initialize C1G2Kill */
          pC1G2Kill = LLRP_C1G2Kill_construct();
          /* Set OpSpec Id */
          LLRP_C1G2Kill_setOpSpecID(pC1G2Kill, reader->u.llrpReader.opSpecId);
          /* Set the kill password */
          LLRP_C1G2Kill_setKillPassword(pC1G2Kill, args->password);

          /**
           * Set C1G2Kill as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                                      (LLRP_tSParameter *)pC1G2Kill);
          break;
        }

      case TMR_TAGOP_GEN2_BLOCKWRITE:
        {
          TMR_TagOp_GEN2_BlockWrite  *args;
          LLRP_tSC1G2BlockWrite      *pC1G2BlockWrite;
          llrp_u16v_t                 data;

          args = &tagop->u.gen2.u.blockWrite;
          /* Construct and initialize C1G2BlockWrite */
          pC1G2BlockWrite = LLRP_C1G2BlockWrite_construct();
          /* Set OpSpec Id */
          LLRP_C1G2BlockWrite_setOpSpecID(pC1G2BlockWrite, reader->u.llrpReader.opSpecId);
          /* Set access password */
          LLRP_C1G2BlockWrite_setAccessPassword(pC1G2BlockWrite, reader->u.llrpReader.gen2AccessPassword);
          /* Set Memory Bank */
          if ((args->bank == TMR_GEN2_BANK_RESERVED) || (args->bank == TMR_GEN2_BANK_EPC) ||
              (args->bank == TMR_GEN2_BANK_TID) || (args->bank == TMR_GEN2_BANK_USER))
          {
            LLRP_C1G2BlockWrite_setMB(pC1G2BlockWrite, (llrp_u2_t)args->bank);
          }
          else
          {
            return TMR_ERROR_INVALID; 
          }
          /* Set word pointer */
          LLRP_C1G2BlockWrite_setWordPointer(pC1G2BlockWrite, (llrp_u16_t)args->wordPtr);
          /* Set the data to be written */
          data = LLRP_u16v_construct (args->data.len);
          memcpy (data.pValue, args->data.list, data.nValue * sizeof(uint16_t));
          LLRP_C1G2BlockWrite_setWriteData(pC1G2BlockWrite, data);

          /**
           * Set C1G2BlockWrite as OpSpec to accessSpec
           **/

          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pC1G2BlockWrite);

          break;
        }

      case TMR_TAGOP_GEN2_NXP_SETREADPROTECT:
        {
          TMR_TagOp_GEN2_NXP_SetReadProtect *args;
          LLRP_tSThingMagicNXPG2ISetReadProtect *pG2I;
          LLRP_tSThingMagicNXPG2XSetReadProtect *pG2X;

          args = &tagop->u.gen2.u.custom.u.nxp.u.setReadProtect;

          if (TMR_SR_GEN2_NXP_G2I_SILICON == tagop->u.gen2.u.custom.chipType)
          {
            /* Construct and initialize G2I setReadProtect */
            pG2I = LLRP_ThingMagicNXPG2ISetReadProtect_construct();
            /* Set OpSpec Id */
            LLRP_ThingMagicNXPG2ISetReadProtect_setOpSpecID(
                    pG2I, reader->u.llrpReader.opSpecId);
            /* Set access password */
            LLRP_ThingMagicNXPG2ISetReadProtect_setAccessPassword(
                                      pG2I, args->accessPassword);
            /**
             * Set G2I SetReadProtect as OpSpec to accessSpec
             **/
            LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                                          (LLRP_tSParameter *)pG2I);
          }
          else
          {
            /* Construct and initialize G2X setReadProtect */
            pG2X = LLRP_ThingMagicNXPG2XSetReadProtect_construct();
            /* Set OpSpec Id */
            LLRP_ThingMagicNXPG2XSetReadProtect_setOpSpecID(
                        pG2X, reader->u.llrpReader.opSpecId);
            /* Set access password */
            LLRP_ThingMagicNXPG2XSetReadProtect_setAccessPassword(
                                      pG2X, args->accessPassword);
            /**
             * Set G2X SetReadProtect as OpSpec to accessSpec
             **/
            LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                                            (LLRP_tSParameter *)pG2X);
          }
          break;
        }

      case TMR_TAGOP_GEN2_NXP_RESETREADPROTECT:
        {
          TMR_TagOp_GEN2_NXP_ResetReadProtect   *args;
          LLRP_tSThingMagicNXPG2IResetReadProtect *pG2I;
          LLRP_tSThingMagicNXPG2XResetReadProtect *pG2X;

          args = &tagop->u.gen2.u.custom.u.nxp.u.resetReadProtect;

          if (TMR_SR_GEN2_NXP_G2I_SILICON == tagop->u.gen2.u.custom.chipType)
          {
            /* Construct and initialize G2I resetReadProtect */
            pG2I = LLRP_ThingMagicNXPG2IResetReadProtect_construct();
            /* Set OpSpec Id */
            LLRP_ThingMagicNXPG2IResetReadProtect_setOpSpecID(
                    pG2I, reader->u.llrpReader.opSpecId);
            /* Set access password */
            LLRP_ThingMagicNXPG2IResetReadProtect_setAccessPassword(
                                      pG2I, args->accessPassword);
            /**
             * Set G2I ResetReadProtect as OpSpec to accessSpec
             **/
            LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                                          (LLRP_tSParameter *)pG2I);
          }
          else
          {
            /* Construct and initialize G2X resetReadProtect */
            pG2X = LLRP_ThingMagicNXPG2XResetReadProtect_construct();
            /* Set OpSpec Id */
            LLRP_ThingMagicNXPG2XResetReadProtect_setOpSpecID(
                        pG2X, reader->u.llrpReader.opSpecId);
            /* Set access password */
            LLRP_ThingMagicNXPG2XResetReadProtect_setAccessPassword(
                                      pG2X, args->accessPassword);
            /**
             * Set G2X ResetReadProtect as OpSpec to accessSpec
             **/
            LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                                            (LLRP_tSParameter *)pG2X);
          }
          break;
        }

      case TMR_TAGOP_GEN2_NXP_CHANGEEAS:
        {
          TMR_TagOp_GEN2_NXP_ChangeEAS     *args;
          LLRP_tSThingMagicNXPG2IChangeEAS *pG2I;
          LLRP_tSThingMagicNXPG2XChangeEAS *pG2X;

          args = &tagop->u.gen2.u.custom.u.nxp.u.changeEAS;

          if (TMR_SR_GEN2_NXP_G2I_SILICON == tagop->u.gen2.u.custom.chipType)
          {
            /* Construct and initialize G2I changeEAS */
            pG2I = LLRP_ThingMagicNXPG2IChangeEAS_construct();
            /* Set OpSpec Id */
            LLRP_ThingMagicNXPG2IChangeEAS_setOpSpecID(
                    pG2I, reader->u.llrpReader.opSpecId);
            /* Set access password */
            LLRP_ThingMagicNXPG2IChangeEAS_setAccessPassword(
                                      pG2I, args->accessPassword);
            /* Set EASStatus */
            LLRP_ThingMagicNXPG2IChangeEAS_setReset(
                                      pG2I, args->reset);
            /**
             * Set G2I ChangeEAS as OpSpec to accessSpec
             **/
            LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                                          (LLRP_tSParameter *)pG2I);
          }
          else
          {
            /* Construct and initialize G2X ChangeEAS */
            pG2X = LLRP_ThingMagicNXPG2XChangeEAS_construct();
            /* Set OpSpec Id */
            LLRP_ThingMagicNXPG2XChangeEAS_setOpSpecID(
                        pG2X, reader->u.llrpReader.opSpecId);
            /* Set access password */
            LLRP_ThingMagicNXPG2XChangeEAS_setAccessPassword(
                                      pG2X, args->accessPassword);
            /* Set EASStatus */
            LLRP_ThingMagicNXPG2XChangeEAS_setReset(
                                      pG2X, args->reset);
            /**
             * Set G2X ChangeEAS as OpSpec to accessSpec
             **/
            LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                                            (LLRP_tSParameter *)pG2X);
          }
          break;
        }

      case TMR_TAGOP_GEN2_NXP_EASALARM:
        {
          TMR_TagOp_GEN2_NXP_EASAlarm     *args;
          LLRP_tSThingMagicNXPG2IEASAlarm *pG2I;
          LLRP_tSThingMagicNXPG2XEASAlarm *pG2X;

          args = &tagop->u.gen2.u.custom.u.nxp.u.EASAlarm;

          if (TMR_SR_GEN2_NXP_G2I_SILICON == tagop->u.gen2.u.custom.chipType)
          {
            /* Construct and initialize G2I EASAlarm */
            pG2I = LLRP_ThingMagicNXPG2IEASAlarm_construct();
            /* Set OpSpec Id */
            LLRP_ThingMagicNXPG2IEASAlarm_setOpSpecID(
                    pG2I, reader->u.llrpReader.opSpecId);
            /* Set access password */
            LLRP_ThingMagicNXPG2IEASAlarm_setAccessPassword(
                pG2I, reader->u.llrpReader.gen2AccessPassword);
            /* Set DivideRatio */
            LLRP_ThingMagicNXPG2IEASAlarm_setDivideRatio(pG2I, args->dr);
            /* Set TagEncoding */
            LLRP_ThingMagicNXPG2IEASAlarm_setTagEncoding(pG2I, args->m);
            /* Set TrExt */
            LLRP_ThingMagicNXPG2IEASAlarm_setPilotTone(pG2I, args->trExt);

            /**
             * Set G2I EASAlarm as OpSpec to accessSpec
             **/
            LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                                          (LLRP_tSParameter *)pG2I);
          }
          else
          {
            /* Construct and initialize G2X EASAlarm */
            pG2X = LLRP_ThingMagicNXPG2XEASAlarm_construct();
            /* Set OpSpec Id */
            LLRP_ThingMagicNXPG2XEASAlarm_setOpSpecID(
                    pG2X, reader->u.llrpReader.opSpecId);
            /* Set access password */
            LLRP_ThingMagicNXPG2XEASAlarm_setAccessPassword(
                pG2X, reader->u.llrpReader.gen2AccessPassword);
            /* Set DivideRatio */
            LLRP_ThingMagicNXPG2XEASAlarm_setDivideRatio(pG2X, args->dr);
            /* Set TagEncoding */
            LLRP_ThingMagicNXPG2XEASAlarm_setTagEncoding(pG2X, args->m);
            /* Set TrExt */
            LLRP_ThingMagicNXPG2XEASAlarm_setPilotTone(pG2X, args->trExt);

            /**
             * Set G2X EASAlarm as OpSpec to accessSpec
             **/
            LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                                            (LLRP_tSParameter *)pG2X);
          }
          break;
        }

      case TMR_TAGOP_GEN2_NXP_CALIBRATE:
        {
          TMR_TagOp_GEN2_NXP_Calibrate     *args;
          LLRP_tSThingMagicNXPG2ICalibrate *pG2I;
          LLRP_tSThingMagicNXPG2XCalibrate *pG2X;

          args = &tagop->u.gen2.u.custom.u.nxp.u.calibrate;

          if (TMR_SR_GEN2_NXP_G2I_SILICON == tagop->u.gen2.u.custom.chipType)
          {
            /* Construct and initialize G2I calibrate */
            pG2I = LLRP_ThingMagicNXPG2ICalibrate_construct();
            /* Set OpSpec Id */
            LLRP_ThingMagicNXPG2ICalibrate_setOpSpecID(
                    pG2I, reader->u.llrpReader.opSpecId);
            /* Set access password */
            LLRP_ThingMagicNXPG2ICalibrate_setAccessPassword(
                                      pG2I, args->accessPassword);
            /**
             * Set G2I Calibrate as OpSpec to accessSpec
             **/
            LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                                          (LLRP_tSParameter *)pG2I);
          }
          else
          {
            /* Construct and initialize G2X Calibrate */
            pG2X = LLRP_ThingMagicNXPG2XCalibrate_construct();
            /* Set OpSpec Id */
            LLRP_ThingMagicNXPG2XCalibrate_setOpSpecID(
                        pG2X, reader->u.llrpReader.opSpecId);
            /* Set access password */
            LLRP_ThingMagicNXPG2XCalibrate_setAccessPassword(
                                      pG2X, args->accessPassword);
            /**
             * Set G2X Calibrate as OpSpec to accessSpec
             **/
            LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                                            (LLRP_tSParameter *)pG2X);
          }
          break;
        }

      case TMR_TAGOP_GEN2_NXP_CHANGECONFIG:
        {
          TMR_TagOp_GEN2_NXP_ChangeConfig     *args;
          LLRP_tSThingMagicNXPG2IChangeConfig *pG2I;
          LLRP_tSThingMagicNXPConfigWord      *pConfigWord;

          args = &tagop->u.gen2.u.custom.u.nxp.u.changeConfig;

          if (TMR_SR_GEN2_NXP_G2I_SILICON == tagop->u.gen2.u.custom.chipType)
          {
            /* Construct and initialize G2I changeconfig */
            pG2I = LLRP_ThingMagicNXPG2IChangeConfig_construct();
            /* Set OpSpec Id */
            LLRP_ThingMagicNXPG2IChangeConfig_setOpSpecID(
                    pG2I, reader->u.llrpReader.opSpecId);
            /* Set access password */
            LLRP_ThingMagicNXPG2IChangeConfig_setAccessPassword(
                                      pG2I, args->accessPassword);

            /* Construct and initialize configword */
            pConfigWord = LLRP_ThingMagicNXPConfigWord_construct();
            
            pConfigWord->PSFAlarm = args->configWord.bits.psfAlarm;
            pConfigWord->ReadProtectTID = args->configWord.bits.readProtectTID;
            pConfigWord->ReadProtectEPC = args->configWord.bits.readProtectEPC;
            pConfigWord->ReadProtectUser = args->configWord.bits.readProtectUser;
            pConfigWord->PrivacyMode = args->configWord.bits.privacyMode;
            pConfigWord->DigitalOutput = args->configWord.bits.digitalOutput;
            pConfigWord->MaxBackscatterStrength = args->configWord.bits.maxBackscatterStrength;
            pConfigWord->ConditionalReadRangeReduction_OpenShort =
                  args->configWord.bits.conditionalReadRangeReduction_openShort;
            pConfigWord->ConditionalReadRangeReduction_OnOff =
                  args->configWord.bits.conditionalReadRangeReduction_onOff;
            pConfigWord->DataMode = args->configWord.bits.dataMode;
            pConfigWord->TransparentMode = args->configWord.bits.transparentMode;
            pConfigWord->InvertDigitalOutput = args->configWord.bits.invertDigitalOutput;
            pConfigWord->ExternalSupply = args->configWord.bits.externalSupply;
            pConfigWord->TamperAlarm = args->configWord.bits.tamperAlarm;

            /* Set configword to opSpec */
            LLRP_ThingMagicNXPG2IChangeConfig_setThingMagicNXPConfigWord(
                                                        pG2I, pConfigWord);
            /**
             * Set G2I Calibrate as OpSpec to accessSpec
             **/
            LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                                          (LLRP_tSParameter *)pG2I);
          }
          else
          {
            /**
             * ChangeConfig works only for G2I tags.
             **/
            return TMR_ERROR_UNSUPPORTED;
          }
          break;
        }

      case TMR_TAGOP_GEN2_IDS_SL900A_SETSFEPARAMETERS:
        {
          TMR_TagOp_GEN2_IDS_SL900A_SetSfeParameters *args;
          LLRP_tSThingMagicIDSSL900ASetSFEParams     *pTMIdsSetSfeParameters;
          LLRP_tSThingMagicIDSSFEParam               *pTMIdsSfeParam;
          LLRP_tSThingMagicIDSSL900ACommandRequest   *pTMIdsCommandRequest;

          args = &tagop->u.gen2.u.custom.u.ids.u.setSfeParameters;

          /* Construct and initialize TMIDS SetSfeParameters opspec */
          pTMIdsSetSfeParameters = LLRP_ThingMagicIDSSL900ASetSFEParams_construct();
          /* Construct and initialize TMIDS Command Request */
          pTMIdsCommandRequest = LLRP_ThingMagicIDSSL900ACommandRequest_construct();
          /* Set OpSpecId to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setOpSpecID(pTMIdsCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set access password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setAccessPassword(pTMIdsCommandRequest,
              (llrp_u32_t) args->AccessPassword);
          /* Set the IDS password level to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setPasswordLevel(pTMIdsCommandRequest,
              (LLRP_tEThingMagicCustomIDSPasswordLevel)args->sl900A.level);
          /* Set the Command code for IDS SetSfeParameters operation to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setCommandCode(pTMIdsCommandRequest,
              (llrp_u8_t)args->CommandCode);
          /* Set the IDS Password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setIDSPassword(pTMIdsCommandRequest,
              (llrp_u32_t)args->Password);
          /* Set the IDS Command Request to TMIDS SetSfeParameters opspec */
          LLRP_ThingMagicIDSSL900ASetSFEParams_setThingMagicIDSSL900ACommandRequest(pTMIdsSetSfeParameters,
              pTMIdsCommandRequest);
          /* Construct and initialize the TMIDS SetSfeParams */
          {
            pTMIdsSfeParam = LLRP_ThingMagicIDSSFEParam_construct();
            /* Raw 16-bit SFE parameters value */
            LLRP_ThingMagicIDSSFEParam_setraw(pTMIdsSfeParam, (llrp_u16_t)args->sfe->raw);
            /* External sensor 2 range */
            LLRP_ThingMagicIDSSFEParam_setrange(pTMIdsSfeParam, (llrp_u8_t)args->sfe->Rang);
            /* External sensor 1 range */
            LLRP_ThingMagicIDSSFEParam_setseti(pTMIdsSfeParam, (llrp_u8_t)args->sfe->Seti);
            /* External sensor 1 type */
            LLRP_ThingMagicIDSSFEParam_setExt1(pTMIdsSfeParam, (llrp_u8_t)args->sfe->Ext1);
            /* External sensor 2 type */
            LLRP_ThingMagicIDSSFEParam_setExt2(pTMIdsSfeParam, (llrp_u8_t)args->sfe->Ext2);
            /* Use preset range */
            LLRP_ThingMagicIDSSFEParam_setAutoRangeDisable(pTMIdsSfeParam, (llrp_u1_t)args->sfe->AutorangeDisable);
            /* Sensor used in limit check */
            LLRP_ThingMagicIDSSFEParam_setVerifySensorID(pTMIdsSfeParam, (llrp_u8_t)args->sfe->VerifySensorID);
            /* specifies the type of field user want to modify */
            LLRP_ThingMagicIDSSFEParam_setSFEType(pTMIdsSfeParam, (LLRP_tEThingMagicCustomIDSSFEType)args->sfe->type);

            LLRP_ThingMagicIDSSL900ASetSFEParams_setThingMagicIDSSFEParam(pTMIdsSetSfeParameters,
                (LLRP_tSThingMagicIDSSFEParam *)pTMIdsSfeParam);
          }

          /**
           * Set TMIDS SetSfeParameters as OpSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMIdsSetSfeParameters);
          break;
        }

      case TMR_TAGOP_GEN2_IDS_SL900A_GETMEASUREMENTSETUP:
        {
          TMR_TagOp_GEN2_IDS_SL900A_GetMeasurementSetup *args;
          LLRP_tSThingMagicIDSSL900AGetMeasurementSetup *pTMIdsGetMeasurementSetup;
          LLRP_tSThingMagicIDSSL900ACommandRequest      *pTMIdsCommandRequest;

          args = &tagop->u.gen2.u.custom.u.ids.u.measurementSetup;

          /* Construct and initialize TMIDS GetMeasurementSetUp opspec */
          pTMIdsGetMeasurementSetup = LLRP_ThingMagicIDSSL900AGetMeasurementSetup_construct();
          /* Construct and initialize TMIDS Command Request */
          pTMIdsCommandRequest = LLRP_ThingMagicIDSSL900ACommandRequest_construct();
          /* Set OpSpecId to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setOpSpecID(pTMIdsCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set access password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setAccessPassword(pTMIdsCommandRequest,
              (llrp_u32_t) args->AccessPassword);
          /* Set the IDS password level to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setPasswordLevel(pTMIdsCommandRequest,
              (LLRP_tEThingMagicCustomIDSPasswordLevel)args->sl900A.level);
          /* Set the Command code for IDS GetMeasurementSetup operation to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setCommandCode(pTMIdsCommandRequest,
              (llrp_u8_t)args->CommandCode);
          /* Set the IDS Password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setIDSPassword(pTMIdsCommandRequest,
              (llrp_u32_t)args->Password);
          /* Set the IDS Command Request to TMIDS GetMeasurementSetup opspec */
          LLRP_ThingMagicIDSSL900AGetMeasurementSetup_setThingMagicIDSSL900ACommandRequest(pTMIdsGetMeasurementSetup,
              pTMIdsCommandRequest);

          /**
           * Set TMIDS GetMeasurementSetup as OpSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMIdsGetMeasurementSetup);
          break;
        }

      case TMR_TAGOP_GEN2_IDS_SL900A_GETBATTERYLEVEL:
        {
          TMR_TagOp_GEN2_IDS_SL900A_GetBatteryLevel *args;
          LLRP_tSThingMagicIDSSL900AGetBatteryLevel *pTMIdsGetBatteryLevel;
          LLRP_tSThingMagicIDSSL900ACommandRequest  *pTMIdsCommandRequest;

          args = &tagop->u.gen2.u.custom.u.ids.u.batteryLevel;

          /* Construct and initialize TMIDS GetBatteryLevel opspec */
          pTMIdsGetBatteryLevel = LLRP_ThingMagicIDSSL900AGetBatteryLevel_construct();
          /* Construct and initialize TMIDS Command Request */
          pTMIdsCommandRequest = LLRP_ThingMagicIDSSL900ACommandRequest_construct();
          /* Set OpSpecId to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setOpSpecID(pTMIdsCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set access password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setAccessPassword(pTMIdsCommandRequest,
              (llrp_u32_t) args->AccessPassword);
          /* Set the IDS password level to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setPasswordLevel(pTMIdsCommandRequest,
              (LLRP_tEThingMagicCustomIDSPasswordLevel)args->sl900A.level);
          /* Set the Command code for IDS GetBatteryLevel operation to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setCommandCode(pTMIdsCommandRequest,
              (llrp_u8_t)args->CommandCode);
          /* Set the IDS Password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setIDSPassword(pTMIdsCommandRequest,
              (llrp_u32_t)args->Password);
          /* Set the IDS Command Request to TMIDS GetBatteryLevel opspec */
          LLRP_ThingMagicIDSSL900AGetBatteryLevel_setThingMagicIDSSL900ACommandRequest(pTMIdsGetBatteryLevel,
              pTMIdsCommandRequest);

          /**
           * Set TMIDS GetBatteryLevel as OpSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMIdsGetBatteryLevel);
          break;
        }

      case TMR_TAGOP_GEN2_IDS_SL900A_SETLOGLIMITS:
        {
          TMR_TagOp_GEN2_IDS_SL900A_SetLogLimits    *args;
          LLRP_tSThingMagicIDSSL900ASetLogLimits    *pTMIdsSetLogLimits;
          LLRP_tSThingMagicIDSLogLimits             *pTMIdsLogLimits;
          LLRP_tSThingMagicIDSSL900ACommandRequest  *pTMIdsCommandRequest;

          args = &tagop->u.gen2.u.custom.u.ids.u.setLogLimit;

          /* Construct and initialize TMIDS SetLogLimits opspec */
          pTMIdsSetLogLimits = LLRP_ThingMagicIDSSL900ASetLogLimits_construct();
          /* Construct and initialize TMIDS Command Request */
          pTMIdsCommandRequest = LLRP_ThingMagicIDSSL900ACommandRequest_construct();
          /* Set OpSpecId to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setOpSpecID(pTMIdsCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set access password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setAccessPassword(pTMIdsCommandRequest,
              (llrp_u32_t) args->AccessPassword);
          /* Set the IDS password level to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setPasswordLevel(pTMIdsCommandRequest,
              (LLRP_tEThingMagicCustomIDSPasswordLevel)args->sl900A.level);
          /* Set the Command code for IDS SetLogLimits operation to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setCommandCode(pTMIdsCommandRequest,
              (llrp_u8_t)args->CommandCode);
          /* Set the IDS Password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setIDSPassword(pTMIdsCommandRequest,
              (llrp_u32_t)args->Password);
          /* Set the IDS Command Request to TMIDS SetLogLimits opspec */
          LLRP_ThingMagicIDSSL900ASetLogLimits_setThingMagicIDSSL900ACommandRequest(pTMIdsSetLogLimits,
              pTMIdsCommandRequest);
          /* Construct and initialize the TMIDS LogLimits */
          {
            pTMIdsLogLimits = LLRP_ThingMagicIDSLogLimits_construct();
            /* Specifying the extreme lower limit */
            LLRP_ThingMagicIDSLogLimits_setextremeLower(pTMIdsLogLimits, (llrp_u16_t)args->limit.extremeLower);
            /* Specifying the lower limit */
            LLRP_ThingMagicIDSLogLimits_setlower(pTMIdsLogLimits, (llrp_u16_t)args->limit.lower);
            /* Specifying the upper limit */
            LLRP_ThingMagicIDSLogLimits_setupper(pTMIdsLogLimits, (llrp_u16_t)args->limit.upper);
            /* Specifying the extreme upper limit */
            LLRP_ThingMagicIDSLogLimits_setextremeUpper(pTMIdsLogLimits, (llrp_u16_t)args->limit.extremeUpper);

            LLRP_ThingMagicIDSSL900ASetLogLimits_setThingMagicIDSLogLimits(pTMIdsSetLogLimits,
                (LLRP_tSThingMagicIDSLogLimits *)pTMIdsLogLimits);
          }

          /**
           * Set TMIDS SetLogLimits as OpSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMIdsSetLogLimits);
          break;
        }

      case TMR_TAGOP_GEN2_IDS_SL900A_SETSHELFLIFE:
        {
          TMR_TagOp_GEN2_IDS_SL900A_SetShelfLife    *args;
          LLRP_tSThingMagicIDSSetShelfLife          *pTMIdsSetShelfLife;
          LLRP_tSThingMagicIDSSLBlock0              *pTMIdsBlock0;
          LLRP_tSThingMagicIDSSLBlock1              *pTMIdsBlock1;
          LLRP_tSThingMagicIDSSL900ACommandRequest  *pTMIdsCommandRequest;

          args = &tagop->u.gen2.u.custom.u.ids.u.setShelfLife;

          /* Construct and initialize TMIDS SetShelfLife opspec */
          pTMIdsSetShelfLife = LLRP_ThingMagicIDSSetShelfLife_construct();
          /* Construct and initialize TMIDS Command Request */
          pTMIdsCommandRequest = LLRP_ThingMagicIDSSL900ACommandRequest_construct();
          /* Set OpSpecId to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setOpSpecID(pTMIdsCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set access password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setAccessPassword(pTMIdsCommandRequest,
              (llrp_u32_t) args->AccessPassword);
          /* Set the IDS password level to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setPasswordLevel(pTMIdsCommandRequest,
              (LLRP_tEThingMagicCustomIDSPasswordLevel)args->sl900A.level);
          /* Set the Command code for IDS SetShelfLife operation to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setCommandCode(pTMIdsCommandRequest,
              (llrp_u8_t)args->CommandCode);
          /* Set the IDS Password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setIDSPassword(pTMIdsCommandRequest,
              (llrp_u32_t)args->Password);
          /* Set the IDS Command Request to TMIDS SetShelfLife opspec */
          LLRP_ThingMagicIDSSetShelfLife_setThingMagicIDSSL900ACommandRequest(pTMIdsSetShelfLife,
              pTMIdsCommandRequest);
          /* Construct and Initialize the TMIDS Block0 Data */
          {
            pTMIdsBlock0 = LLRP_ThingMagicIDSSLBlock0_construct();
            /* raw value of block0 */
            LLRP_ThingMagicIDSSLBlock0_setraw(pTMIdsBlock0, (llrp_u32_t)args->shelfLifeBlock0->raw);
            /* Tmax */
            LLRP_ThingMagicIDSSLBlock0_setTimeMax(pTMIdsBlock0, (llrp_u8_t)args->shelfLifeBlock0->Tmax);
            /* Tmin */
            LLRP_ThingMagicIDSSLBlock0_setTimeMin(pTMIdsBlock0, (llrp_u8_t)args->shelfLifeBlock0->Tmin);
            /* Tstd */
            LLRP_ThingMagicIDSSLBlock0_setTimeStd(pTMIdsBlock0, (llrp_u8_t)args->shelfLifeBlock0->Tstd);
            /* Ea */
            LLRP_ThingMagicIDSSLBlock0_setEa(pTMIdsBlock0, (llrp_u8_t)args->shelfLifeBlock0->Ea);

            LLRP_ThingMagicIDSSetShelfLife_setThingMagicIDSSLBlock0(pTMIdsSetShelfLife,
                (LLRP_tSThingMagicIDSSLBlock0 *)pTMIdsBlock0);
          }
          /* Construct and Initialize the TMIDS Block1 Data */
          {
            pTMIdsBlock1 = LLRP_ThingMagicIDSSLBlock1_construct();
            /* raw value for block 1 */
            LLRP_ThingMagicIDSSLBlock1_setraw(pTMIdsBlock1, (llrp_u32_t)args->shelfLifeBlock1->raw);
            /* SLinit */
            LLRP_ThingMagicIDSSLBlock1_setSLInit(pTMIdsBlock1, (llrp_u16_t)args->shelfLifeBlock1->SLinit);
            /* Tint */
            LLRP_ThingMagicIDSSLBlock1_setTInit(pTMIdsBlock1, (llrp_u16_t)args->shelfLifeBlock1->Tint);
            /* SensorID */
            LLRP_ThingMagicIDSSLBlock1_setSensorID(pTMIdsBlock1, (llrp_u8_t)args->shelfLifeBlock1->sensorID);
            /* Enable negative shelf life */
            LLRP_ThingMagicIDSSLBlock1_setenableNegative(pTMIdsBlock1, (llrp_u1_t)args->shelfLifeBlock1->enableNegative);
            /* Shelf life algorithem enable */
            LLRP_ThingMagicIDSSLBlock1_setalgorithmEnable(pTMIdsBlock1, (llrp_u1_t)args->shelfLifeBlock1->algorithmEnable);
            /* RFU Bytes */
            LLRP_ThingMagicIDSSLBlock1_setRFU(pTMIdsBlock1, (llrp_u8_t)args->shelfLifeBlock1->rfu);

            LLRP_ThingMagicIDSSetShelfLife_setThingMagicIDSSLBlock1(pTMIdsSetShelfLife,
                (LLRP_tSThingMagicIDSSLBlock1 *)pTMIdsBlock1);
          }

          /**
           * Set TMIDS SetShelfLife as OpSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMIdsSetShelfLife);
          break;
        }

      case TMR_TAGOP_GEN2_IDS_SL900A_SETPASSWORD:
        {
          TMR_TagOp_GEN2_IDS_SL900A_SetPassword     *args;
          LLRP_tSThingMagicIDSSL900ASetIDSPassword  *pTMIdsSetPassword;
          LLRP_tSThingMagicIDSSL900ACommandRequest  *pTMIdsCommandRequest;

          args = &tagop->u.gen2.u.custom.u.ids.u.setPassword;
          /* Construct and initialize TMIDS SetPassword opspec */
          pTMIdsSetPassword = LLRP_ThingMagicIDSSL900ASetIDSPassword_construct();
          /* Construct and initialize TMIDS Command Request */
          pTMIdsCommandRequest = LLRP_ThingMagicIDSSL900ACommandRequest_construct();
          /* Set OpSpecId to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setOpSpecID(pTMIdsCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set access password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setAccessPassword(pTMIdsCommandRequest,
              (llrp_u32_t) args->AccessPassword);
          /* Set the IDS password level to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setPasswordLevel(pTMIdsCommandRequest,
              (LLRP_tEThingMagicCustomIDSPasswordLevel)args->sl900A.level);
          /* Set the Command code for IDS SetPassword operation to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setCommandCode(pTMIdsCommandRequest,
              (llrp_u8_t)args->CommandCode);
          /* Set the IDS Password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setIDSPassword(pTMIdsCommandRequest,
              (llrp_u32_t)args->Password);
          /* Set the IDS Command Request to TMIDS SetPassword opspec */
          LLRP_ThingMagicIDSSL900ASetIDSPassword_setThingMagicIDSSL900ACommandRequest(pTMIdsSetPassword,
              pTMIdsCommandRequest);
          /* Set the IDS New PasswordLevel */
          LLRP_ThingMagicIDSSL900ASetIDSPassword_setNewPasswordLevel(pTMIdsSetPassword,
              (LLRP_tEThingMagicCustomIDSPasswordLevel)args->NewPasswordLevel);
          /* Set the IDS New Password */
          LLRP_ThingMagicIDSSL900ASetIDSPassword_setNewIDSPassword(pTMIdsSetPassword,
              (llrp_u32_t)args->NewPassword);

          /**
           * Set TMIDS SetPassword as OpSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMIdsSetPassword);
          break;
        }

      case TMR_TAGOP_GEN2_IMPINJ_MONZA4_QTREADWRITE:
        {
          TMR_TagOp_GEN2_Impinj_Monza4_QTReadWrite *args;
          LLRP_tSThingMagicImpinjMonza4QTReadWrite *pQTReadWrite;
          LLRP_tSThingMagicMonza4ControlByte       *pControlByte;
          LLRP_tSThingMagicMonza4Payload           *pPayload;

          args = &tagop->u.gen2.u.custom.u.impinj.u.monza4.u.qtReadWrite;

          /* Construct and initialize monza4 opspec */
          pQTReadWrite = LLRP_ThingMagicImpinjMonza4QTReadWrite_construct();
          /* Set OpSpecId */
          LLRP_ThingMagicImpinjMonza4QTReadWrite_setOpSpecID(
                    pQTReadWrite, reader->u.llrpReader.opSpecId);
          /* Set Access Password */
          LLRP_ThingMagicImpinjMonza4QTReadWrite_setAccessPassword(
                    pQTReadWrite, args->accessPassword);
          /* Initialize and set controlbyte */
          pControlByte = LLRP_ThingMagicMonza4ControlByte_construct();
          pControlByte->Persistance = args->controlByte.bits.persistence;
          pControlByte->ReadWrite = args->controlByte.bits.readWrite;

          LLRP_ThingMagicImpinjMonza4QTReadWrite_setThingMagicMonza4ControlByte(
              pQTReadWrite, pControlByte);

          /* Initialize and set payload */
          pPayload = LLRP_ThingMagicMonza4Payload_construct();
          pPayload->QT_MEM = args->payload.bits.QT_MEM;
          pPayload->QT_SR = args->payload.bits.QT_SR;

          LLRP_ThingMagicImpinjMonza4QTReadWrite_setThingMagicMonza4Payload(
              pQTReadWrite, pPayload);

          /**
           * Set Monza4 QTReadWrite as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pQTReadWrite);
          break;
        }

      case TMR_TAGOP_GEN2_IDS_SL900A_GETSENSOR:
        {
          TMR_TagOp_GEN2_IDS_SL900A_GetSensorValue  *args;
          LLRP_tSThingMagicIDSSL900ASensorValue     *pTMIdsGetSensor;
          LLRP_tSThingMagicIDSSL900ACommandRequest  *pTMIdsCommandRequest;

          args = &tagop->u.gen2.u.custom.u.ids.u.sensor;

          /* Construct and initialize TMIDS GetSensor opspec */
          pTMIdsGetSensor = LLRP_ThingMagicIDSSL900ASensorValue_construct();
          /* Construct and initialize TMIDS Command Request */
          pTMIdsCommandRequest = LLRP_ThingMagicIDSSL900ACommandRequest_construct();
          /* Set OpSpecId to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setOpSpecID(pTMIdsCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set access password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setAccessPassword(pTMIdsCommandRequest,
              (llrp_u32_t) args->AccessPassword);
          /* Set the IDS password level to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setPasswordLevel(pTMIdsCommandRequest,
              (LLRP_tEThingMagicCustomIDSPasswordLevel)args->sl900A.level);
          /* Set the Command code for IDS GetSensor operation to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setCommandCode(pTMIdsCommandRequest,
              (llrp_u8_t)args->CommandCode);
          /* Set the IDS Password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setIDSPassword(pTMIdsCommandRequest,
              (llrp_u32_t)args->Password);
          /* Set the IDS Command Request to TMIDS GetSensor opspec */
          LLRP_ThingMagicIDSSL900ASensorValue_setThingMagicIDSSL900ACommandRequest(pTMIdsGetSensor,
              pTMIdsCommandRequest);
          /* Set the Sensor type to TMIDS GetSensor opspec */
          LLRP_ThingMagicIDSSL900ASensorValue_setSensorType(pTMIdsGetSensor,
              (LLRP_tEThingMagicCustomIDSSensorType)args->sl900A.sensortype);

          /**
           * Set TMIDS GetSensor as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMIdsGetSensor);
          break;

        }

      case TMR_TAGOP_GEN2_IDS_SL900A_INITIALIZE:
        {
          TMR_TagOp_GEN2_IDS_SL900A_Initialize      *args;
          LLRP_tSThingMagicIDSSL900AInitialize      *pTMIdsInitialize;
          LLRP_tSThingMagicIDSDelayTime             *pTMIdsDelayTime;
          LLRP_tSThingMagicIDSApplicationData       *pTMIdsApplicationData;
          LLRP_tSThingMagicIDSSL900ACommandRequest  *pTMIdsCommandRequest;

          args = &tagop->u.gen2.u.custom.u.ids.u.initialize;

          /* Construct and initialize TMIDS Initialize opspec */
          pTMIdsInitialize = LLRP_ThingMagicIDSSL900AInitialize_construct();
          /* Construct and initialize TMIDS Command Request */
          pTMIdsCommandRequest = LLRP_ThingMagicIDSSL900ACommandRequest_construct();
          /* Set OpSpecId to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setOpSpecID(pTMIdsCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set access password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setAccessPassword(pTMIdsCommandRequest,
              (llrp_u32_t) args->AccessPassword);
          /* Set the IDS password level to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setPasswordLevel(pTMIdsCommandRequest,
              (LLRP_tEThingMagicCustomIDSPasswordLevel)args->sl900A.level);
          /* Set the Command code for IDS Initialize operation to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setCommandCode(pTMIdsCommandRequest,
              (llrp_u8_t)args->CommandCode);
          /* Set the IDS Password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setIDSPassword(pTMIdsCommandRequest,
              (llrp_u32_t)args->Password);
          /* Set the IDS Command Request to TMIDS Initialize opspec */
          LLRP_ThingMagicIDSSL900AInitialize_setThingMagicIDSSL900ACommandRequest(pTMIdsInitialize,
              pTMIdsCommandRequest);
          /* Set the Delay time parameters */
          pTMIdsDelayTime = LLRP_ThingMagicIDSDelayTime_construct();
          pTMIdsDelayTime->delayMode = (llrp_u8_t) args->delayTime.Mode;
          pTMIdsDelayTime->delayTime = (llrp_u16_t) args->delayTime.Time;
          pTMIdsDelayTime->timerEnable = (llrp_u1_t) args->delayTime.IrqTimerEnable;
          LLRP_ThingMagicIDSSL900AInitialize_setThingMagicIDSDelayTime(pTMIdsInitialize,
              pTMIdsDelayTime);
          /* Set the Application Data */
          pTMIdsApplicationData = LLRP_ThingMagicIDSApplicationData_construct();
          pTMIdsApplicationData->brokenWordPointer = (llrp_u8_t) args->applicationData.BrokenWordPointer;
          pTMIdsApplicationData->numberOfWords = (llrp_u16_t) args->applicationData.NumberOfWords;
          LLRP_ThingMagicIDSSL900AInitialize_setThingMagicIDSApplicationData(pTMIdsInitialize,
              pTMIdsApplicationData);

          /**
           * Set TMIDS Initialize as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMIdsInitialize);
          break;
        }

      case TMR_TAGOP_GEN2_IDS_SL900A_SETLOGMODE:
        {
          TMR_TagOp_GEN2_IDS_SL900A_SetLogMode      *args;
          LLRP_tSThingMagicIDSSL900ASetLogMode      *pTMIdsSetLogMode;
          LLRP_tSThingMagicIDSSL900ACommandRequest  *pTMIdsCommandRequest;

          args = &tagop->u.gen2.u.custom.u.ids.u.setLogMode;

          /* Construct and initialize TMIDS SetLogMode opspec */
          pTMIdsSetLogMode = LLRP_ThingMagicIDSSL900ASetLogMode_construct();
          /* Construct and initialize TMIDS Command Request */
          pTMIdsCommandRequest = LLRP_ThingMagicIDSSL900ACommandRequest_construct();
          /* Set OpSpecId to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setOpSpecID(pTMIdsCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set access password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setAccessPassword(pTMIdsCommandRequest,
              (llrp_u32_t) args->AccessPassword);
          /* Set the IDS password level to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setPasswordLevel(pTMIdsCommandRequest,
              (LLRP_tEThingMagicCustomIDSPasswordLevel)args->sl900A.level);
          /* Set the Command code for IDS SetLogMode operation to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setCommandCode(pTMIdsCommandRequest,
              (llrp_u8_t)args->CommandCode);
          /* Set the IDS Password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setIDSPassword(pTMIdsCommandRequest,
              (llrp_u32_t)args->Password);
          /* Set the IDS Command Request to TMIDS SetLogMode opspec */
          LLRP_ThingMagicIDSSL900ASetLogMode_setThingMagicIDSSL900ACommandRequest(pTMIdsSetLogMode,
              pTMIdsCommandRequest);
          /* Set the logging Form */
          LLRP_ThingMagicIDSSL900ASetLogMode_setLoggingForm(pTMIdsSetLogMode,
              (LLRP_tEThingMagicCustomIDSLoggingForm)args->sl900A.dataLog);
          /* Set the Storage rule */
          LLRP_ThingMagicIDSSL900ASetLogMode_setStorageRule(pTMIdsSetLogMode,
              (LLRP_tEThingMagicCustomIDSStorageRule)args->sl900A.rule);
          /* Enable log for EXT1 external sensor */
          LLRP_ThingMagicIDSSL900ASetLogMode_setExt1Enable(pTMIdsSetLogMode,
              (llrp_u1_t)args->Ext1Enable);
          /* Enable log for EXT2 external sensor */
          LLRP_ThingMagicIDSSL900ASetLogMode_setExt2Enable(pTMIdsSetLogMode,
              (llrp_u1_t)args->Ext2Enable);
          /* Enable log for Temperature sensor */
          LLRP_ThingMagicIDSSL900ASetLogMode_setTempEnable(pTMIdsSetLogMode,
              (llrp_u1_t)args->TempEnable);
          /* Enable log for BATT sensor */
          LLRP_ThingMagicIDSSL900ASetLogMode_setBattEnable(pTMIdsSetLogMode,
              (llrp_u1_t)args->BattEnable);
          /* Set the log interval */
          LLRP_ThingMagicIDSSL900ASetLogMode_setLogInterval(pTMIdsSetLogMode,
              (llrp_u16_t)args->LogInterval);

          /**
           * Set TMIDS SetLogMode as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMIdsSetLogMode);
          break;
        }

      case TMR_TAGOP_GEN2_IDS_SL900A_STARTLOG:
        {
          TMR_TagOp_GEN2_IDS_SL900A_StartLog        *args;
          LLRP_tSThingMagicIDSSL900AStartLog        *pTMIdsStartLog;
          LLRP_tSThingMagicIDSSL900ACommandRequest  *pTMIdsCommandRequest;

          args = &tagop->u.gen2.u.custom.u.ids.u.startLog;

          /* Construct and initialize TMIDS StartLog opspec */
          pTMIdsStartLog = LLRP_ThingMagicIDSSL900AStartLog_construct();
          /* Construct and initialize TMIDS Command Request */
          pTMIdsCommandRequest = LLRP_ThingMagicIDSSL900ACommandRequest_construct();
          /* Set OpSpecId to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setOpSpecID(pTMIdsCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set access password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setAccessPassword(pTMIdsCommandRequest,
              (llrp_u32_t) args->AccessPassword);
          /* Set the IDS password level to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setPasswordLevel(pTMIdsCommandRequest,
              (LLRP_tEThingMagicCustomIDSPasswordLevel)args->sl900A.level);
          /* Set the Command code for IDS StartLog operation to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setCommandCode(pTMIdsCommandRequest,
              (llrp_u8_t)args->CommandCode);
          /* Set the IDS Password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setIDSPassword(pTMIdsCommandRequest,
              (llrp_u32_t)args->Password);
          /* Set the IDS Command Request to TMIDS StartLog opspec */
          LLRP_ThingMagicIDSSL900AStartLog_setThingMagicIDSSL900ACommandRequest(pTMIdsStartLog,
              pTMIdsCommandRequest);
          /* Set the Time to initialize log timestamp counter with */
          LLRP_ThingMagicIDSSL900AStartLog_setStartTime(pTMIdsStartLog,
              (llrp_u32_t)args->startTime);

          /**
           * Set TMIDS StartLog as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMIdsStartLog);
          break;
        }

      case TMR_TAGOP_GEN2_IDS_SL900A_GETLOGSTATE:
        {
          TMR_TagOp_GEN2_IDS_SL900A_GetLogState     *args;
          LLRP_tSThingMagicIDSSL900AGetLogState     *pTMIdsGetLogState;
          LLRP_tSThingMagicIDSSL900ACommandRequest  *pTMIdsCommandRequest;

          args = &tagop->u.gen2.u.custom.u.ids.u.getLog;

          /* Construct and initialize TMIDS StartLog opspec */
          pTMIdsGetLogState = LLRP_ThingMagicIDSSL900AGetLogState_construct();
          /* Construct and initialize TMIDS Command Request */
          pTMIdsCommandRequest = LLRP_ThingMagicIDSSL900ACommandRequest_construct();
          /* Set OpSpecId to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setOpSpecID(pTMIdsCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set access password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setAccessPassword(pTMIdsCommandRequest,
              (llrp_u32_t) args->AccessPassword);
          /* Set the IDS password level to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setPasswordLevel(pTMIdsCommandRequest,
              (LLRP_tEThingMagicCustomIDSPasswordLevel)args->sl900A.level);
          /* Set the Command code for IDS GetLogState operation to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setCommandCode(pTMIdsCommandRequest,
              (llrp_u8_t)args->CommandCode);
          /* Set the IDS Password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setIDSPassword(pTMIdsCommandRequest,
              (llrp_u32_t)args->Password);
          /* Set the IDS Command Request to TMIDS GetLogState opspec */
          LLRP_ThingMagicIDSSL900AGetLogState_setThingMagicIDSSL900ACommandRequest(pTMIdsGetLogState,
              pTMIdsCommandRequest);

          /**
           * Set TMIDS GetLogState as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMIdsGetLogState);
          break;
        }

      case TMR_TAGOP_GEN2_IDS_SL900A_ENDLOG:
        {
          TMR_TagOp_GEN2_IDS_SL900A_EndLog          *args;
          LLRP_tSThingMagicIDSSL900AEndLog          *pTMIdsEndLog;
          LLRP_tSThingMagicIDSSL900ACommandRequest  *pTMIdsCommandRequest;

          args = &tagop->u.gen2.u.custom.u.ids.u.endLog;

          /* Construct and initialize TMIDS EndLOg opspec */
          pTMIdsEndLog = LLRP_ThingMagicIDSSL900AEndLog_construct();
          /* Construct and initialize TMIDS Command Request */
          pTMIdsCommandRequest = LLRP_ThingMagicIDSSL900ACommandRequest_construct();
          /* Set OpSpecId to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setOpSpecID(pTMIdsCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set access password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setAccessPassword(pTMIdsCommandRequest,
              (llrp_u32_t) args->AccessPassword);
          /* Set the IDS password level to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setPasswordLevel(pTMIdsCommandRequest,
              (LLRP_tEThingMagicCustomIDSPasswordLevel)args->sl900A.level);
          /* Set the Command code for IDS EndLog operation to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setCommandCode(pTMIdsCommandRequest,
              (llrp_u8_t)args->CommandCode);
          /* Set the IDS Password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setIDSPassword(pTMIdsCommandRequest,
              (llrp_u32_t)args->Password);
          /* Set the IDS Command Request to TMIDS EndLog opspec */
          LLRP_ThingMagicIDSSL900AEndLog_setThingMagicIDSSL900ACommandRequest(pTMIdsEndLog,
              pTMIdsCommandRequest);

          /**
           * Set TMIDS EndLog as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMIdsEndLog);
          break;
        }

      case TMR_TAGOP_GEN2_IDS_SL900A_ACCESSFIFOSTATUS:
        {
          TMR_TagOp_GEN2_IDS_SL900A_AccessFifoStatus  *args;
          LLRP_tSThingMagicIDSSL900AAccessFIFOStatus  *pTMIdsAccessFifoStatus;
          LLRP_tSThingMagicIDSSL900ACommandRequest    *pTMIdsCommandRequest;

          args = &tagop->u.gen2.u.custom.u.ids.u.accessFifoStatus;

          /* Construct and initialize TMIDS FifoStatus opspec */
          pTMIdsAccessFifoStatus = LLRP_ThingMagicIDSSL900AAccessFIFOStatus_construct();
          /* Construct and initialize TMIDS Command Request */
          pTMIdsCommandRequest = LLRP_ThingMagicIDSSL900ACommandRequest_construct();
          /* Set OpSpecId to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setOpSpecID(pTMIdsCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set access password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setAccessPassword(pTMIdsCommandRequest,
              (llrp_u32_t) args->status.AccessPassword);
          /* Set the IDS password level to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setPasswordLevel(pTMIdsCommandRequest,
              (LLRP_tEThingMagicCustomIDSPasswordLevel)args->status.sl900A.level);
          /* Set the Command code for IDS AccessFifoStatus operation to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setCommandCode(pTMIdsCommandRequest,
              (llrp_u8_t)args->status.CommandCode);
          /* Set the IDS Password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setIDSPassword(pTMIdsCommandRequest,
              (llrp_u32_t)args->status.Password);
          /* Set the IDS Command Request to TMIDS AccessFifoStatus opspec */
          LLRP_ThingMagicIDSSL900AAccessFIFOStatus_setThingMagicIDSSL900ACommandRequest(pTMIdsAccessFifoStatus,
              pTMIdsCommandRequest);

          /**
           * Set TMIDS AccessFifoStatus as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMIdsAccessFifoStatus);
          break;
        }

      case TMR_TAGOP_GEN2_IDS_SL900A_ACCESSFIFOWRITE:
        {
          TMR_TagOp_GEN2_IDS_SL900A_AccessFifoWrite  *args;
          LLRP_tSThingMagicIDSSL900AAccessFIFOWrite  *pTMIdsAccessFifoWrite;
          LLRP_tSThingMagicIDSSL900ACommandRequest   *pTMIdsCommandRequest;

          args = &tagop->u.gen2.u.custom.u.ids.u.accessFifoWrite;

          /* Construct and initialize TMIDS FifoWrite opspec */
          pTMIdsAccessFifoWrite = LLRP_ThingMagicIDSSL900AAccessFIFOWrite_construct();
          /* Construct and initialize TMIDS Command Request */
          pTMIdsCommandRequest = LLRP_ThingMagicIDSSL900ACommandRequest_construct();
          /* Set OpSpecId to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setOpSpecID(pTMIdsCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set access password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setAccessPassword(pTMIdsCommandRequest,
              (llrp_u32_t) args->write.AccessPassword);
          /* Set the IDS password level to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setPasswordLevel(pTMIdsCommandRequest,
              (LLRP_tEThingMagicCustomIDSPasswordLevel)args->write.sl900A.level);
          /* Set the Command code for IDS AccessFifoWrite operation to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setCommandCode(pTMIdsCommandRequest,
              (llrp_u8_t)args->write.CommandCode);
          /* Set the IDS Password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setIDSPassword(pTMIdsCommandRequest,
              (llrp_u32_t)args->write.Password);
          /* Set the IDS Command Request to TMIDS AccessFifoWrite opspec */
          LLRP_ThingMagicIDSSL900AAccessFIFOWrite_setThingMagicIDSSL900ACommandRequest(pTMIdsAccessFifoWrite,
              pTMIdsCommandRequest);
          /* Set the writePayLoad */
          {
            llrp_u8v_t  writePayLoad;
            /* construct and initialize the writePayLoad */
            writePayLoad = LLRP_u8v_construct((llrp_u16_t)args->payLoad->len);
            memcpy(writePayLoad.pValue, args->payLoad->list,
                (size_t) (args->payLoad->len * sizeof (args->payLoad->list[0])));
            LLRP_ThingMagicIDSSL900AAccessFIFOWrite_setwritePayLoad(pTMIdsAccessFifoWrite,
                writePayLoad);
          }

          /**
           * Set TMIDS AccessFifoWrite as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMIdsAccessFifoWrite);
          break;
        }

      case TMR_TAGOP_GEN2_IDS_SL900A_ACCESSFIFOREAD:
        {
          TMR_TagOp_GEN2_IDS_SL900A_AccessFifoRead  *args;
          LLRP_tSThingMagicIDSSL900AAccessFIFORead  *pTMIdsAccessFifoRead;
          LLRP_tSThingMagicIDSSL900ACommandRequest  *pTMIdsCommandRequest;

          args = &tagop->u.gen2.u.custom.u.ids.u.accessFifoRead;

          /* Construct and initialize TMIDS FifoRead opspec */
          pTMIdsAccessFifoRead = LLRP_ThingMagicIDSSL900AAccessFIFORead_construct();
          /* Construct and initialize TMIDS Command Request */
          pTMIdsCommandRequest = LLRP_ThingMagicIDSSL900ACommandRequest_construct();
          /* Set OpSpecId to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setOpSpecID(pTMIdsCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set access password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setAccessPassword(pTMIdsCommandRequest,
              (llrp_u32_t) args->read.AccessPassword);
          /* Set the IDS password level to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setPasswordLevel(pTMIdsCommandRequest,
              (LLRP_tEThingMagicCustomIDSPasswordLevel)args->read.sl900A.level);
          /* Set the Command code for IDS AccessFifoRead operation to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setCommandCode(pTMIdsCommandRequest,
              (llrp_u8_t)args->read.CommandCode);
          /* Set the IDS Password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setIDSPassword(pTMIdsCommandRequest,
              (llrp_u32_t)args->read.Password);
          /* Set the IDS Command Request to TMIDS AccessFifoRead opspec */
          LLRP_ThingMagicIDSSL900AAccessFIFORead_setThingMagicIDSSL900ACommandRequest(pTMIdsAccessFifoRead,
              pTMIdsCommandRequest);
          /* Specify the fifo read length */
          LLRP_ThingMagicIDSSL900AAccessFIFORead_setFIFOReadLength(pTMIdsAccessFifoRead,
              (llrp_u8_t)args->length);

          /**
           * Set TMIDS AccessFifoRead as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMIdsAccessFifoRead);
          break;
        }

      case TMR_TAGOP_GEN2_IDS_SL900A_GETCALIBRATIONDATA:
        {
          TMR_TagOp_GEN2_IDS_SL900A_GetCalibrationData *args;
          LLRP_tSThingMagicIDSSL900AGetCalibrationData *pTMIdsGetCalibrationData;
          LLRP_tSThingMagicIDSSL900ACommandRequest     *pTMIdsCommandRequest;

          args = &tagop->u.gen2.u.custom.u.ids.u.calibrationData;

          /* Construct and initialize TMIDS GetCalibration opspec */
          pTMIdsGetCalibrationData = LLRP_ThingMagicIDSSL900AGetCalibrationData_construct();
          /* Construct and initialize TMIDS Command Request */
          pTMIdsCommandRequest = LLRP_ThingMagicIDSSL900ACommandRequest_construct();
          /* Set OpSpecId to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setOpSpecID(pTMIdsCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set access password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setAccessPassword(pTMIdsCommandRequest,
              (llrp_u32_t) args->AccessPassword);
          /* Set the IDS password level to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setPasswordLevel(pTMIdsCommandRequest,
              (LLRP_tEThingMagicCustomIDSPasswordLevel)args->sl900A.level);
          /* Set the Command code for IDS GetCalibrationData operation to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setCommandCode(pTMIdsCommandRequest,
              (llrp_u8_t)args->CommandCode);
          /* Set the IDS Password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setIDSPassword(pTMIdsCommandRequest,
              (llrp_u32_t)args->Password);
          /* Set the IDS Command Request to TMIDS GetCalibrationData opspec */
          LLRP_ThingMagicIDSSL900AGetCalibrationData_setThingMagicIDSSL900ACommandRequest(pTMIdsGetCalibrationData,
              pTMIdsCommandRequest);

          /**
           * Set TMIDS GetCalibrationData as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMIdsGetCalibrationData);
          break;
        }

      case TMR_TAGOP_GEN2_IDS_SL900A_SETCALIBRATIONDATA:
        {
          TMR_TagOp_GEN2_IDS_SL900A_SetCalibrationData *args;
          LLRP_tSThingMagicIDSSL900ASetCalibrationData *pTMIdsSetCalibrationData;
          LLRP_tSThingMagicIDSSL900ACommandRequest     *pTMIdsCommandRequest;
          LLRP_tSThingMagicIDSCalibrationData          *pTMIdsCalibrationData;

          args = &tagop->u.gen2.u.custom.u.ids.u.setCalibration;

          /* Construct and initialize TMIDS SetCalibration opspec */
          pTMIdsSetCalibrationData = LLRP_ThingMagicIDSSL900ASetCalibrationData_construct();
          /* Construct and initialize TMIDS Command Request */
          pTMIdsCommandRequest = LLRP_ThingMagicIDSSL900ACommandRequest_construct();
          /* Set OpSpecId to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setOpSpecID(pTMIdsCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set access password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setAccessPassword(pTMIdsCommandRequest,
              (llrp_u32_t) args->AccessPassword);
          /* Set the IDS password level to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setPasswordLevel(pTMIdsCommandRequest,
              (LLRP_tEThingMagicCustomIDSPasswordLevel)args->sl900A.level);
          /* Set the Command code for IDS SetCalibration operation to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setCommandCode(pTMIdsCommandRequest,
              (llrp_u8_t)args->CommandCode);
          /* Set the IDS Password to TMIDS Command Request */
          LLRP_ThingMagicIDSSL900ACommandRequest_setIDSPassword(pTMIdsCommandRequest,
              (llrp_u32_t)args->Password);
          /* Set the IDS Command Request to TMIDS SetCalibrationData opspec */
          LLRP_ThingMagicIDSSL900ASetCalibrationData_setThingMagicIDSSL900ACommandRequest(pTMIdsSetCalibrationData,
              pTMIdsCommandRequest);
          /* Construct and initialize the TMIDS CalibrationData */
          {
            pTMIdsCalibrationData = LLRP_ThingMagicIDSCalibrationData_construct();
            /* Set the Raw Binary stream to CalibrationData */
            LLRP_ThingMagicIDSCalibrationData_setraw(pTMIdsCalibrationData, (llrp_u64_t)args->cal.raw);
            /* AD1 lower voltage reference - fine - DO NOT MODIFY */
            LLRP_ThingMagicIDSCalibrationData_setad1(pTMIdsCalibrationData, (llrp_u8_t)args->cal.Ad1);
            /* AD2 lower voltage reference - fine - DO NOT MODIFY */
            LLRP_ThingMagicIDSCalibrationData_setad2(pTMIdsCalibrationData, (llrp_u8_t)args->cal.Ad2);
            /* AD1 lower voltage reference - coarse */
            LLRP_ThingMagicIDSCalibrationData_setcoars1(pTMIdsCalibrationData, (llrp_u8_t)args->cal.Coarse1);
            /* AD2 lower voltage reference - coarse */
            LLRP_ThingMagicIDSCalibrationData_setcoars2(pTMIdsCalibrationData, (llrp_u8_t)args->cal.Coarse2);
            /* Switches the lower AD voltage reference to ground */
            LLRP_ThingMagicIDSCalibrationData_setgndSwitch(pTMIdsCalibrationData, (llrp_u1_t)args->cal.GndSwitch);
            /* POR voltage level for 1.5V system */
            LLRP_ThingMagicIDSCalibrationData_setselp12(pTMIdsCalibrationData, (llrp_u8_t)args->cal.Selp12);
            /* Main reference voltage calibration -- DO NOT MODIFY */
            LLRP_ThingMagicIDSCalibrationData_setadf(pTMIdsCalibrationData, (llrp_u8_t)args->cal.Adf);
            /* RTC oscillator calibration */
            LLRP_ThingMagicIDSCalibrationData_setdf(pTMIdsCalibrationData, (llrp_u8_t)args->cal.Df);
            /* Controlled battery supply for external sensor - the battery voltage is connected to the EXC pin */
            LLRP_ThingMagicIDSCalibrationData_setswExtEn(pTMIdsCalibrationData, (llrp_u1_t)args->cal.SwExtEn);
            /* POR voltage level for 3V system */
            LLRP_ThingMagicIDSCalibrationData_setselp22(pTMIdsCalibrationData, (llrp_u8_t)args->cal.Selp22);
            /* Voltage level interrupt level for external sensor -- ratiometric */
            LLRP_ThingMagicIDSCalibrationData_setirlev(pTMIdsCalibrationData, (llrp_u8_t)args->cal.Irlev);
            /* Main system clock oscillator calibration -- DO NOT MODIFY */
            LLRP_ThingMagicIDSCalibrationData_setringCal(pTMIdsCalibrationData, (llrp_u8_t)args->cal.RingCal);
            /* Temperature conversion offset calibration -- DO NOT MODIFY */
            LLRP_ThingMagicIDSCalibrationData_setoffInt(pTMIdsCalibrationData, (llrp_u8_t)args->cal.OffInt);
            /* Bandgap voltage temperature coefficient calibration -- DO NOT MODIFY */
            LLRP_ThingMagicIDSCalibrationData_setreftc(pTMIdsCalibrationData, (llrp_u8_t)args->cal.Reftc);
            /* Excitate for resistive sensors without DC */
            LLRP_ThingMagicIDSCalibrationData_setexcRes(pTMIdsCalibrationData, (llrp_u1_t)args->cal.ExcRes);
            /* Reserved for Future Use */
            LLRP_ThingMagicIDSCalibrationData_setRFU(pTMIdsCalibrationData, (llrp_u8_t)args->cal.RFU);

            LLRP_ThingMagicIDSSL900ASetCalibrationData_setThingMagicIDSCalibrationData(pTMIdsSetCalibrationData,
                (LLRP_tSThingMagicIDSCalibrationData *)pTMIdsCalibrationData);
          }

          /**
           * Set TMIDS SetCalibrationData as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMIdsSetCalibrationData);
          break;
        }

      case TMR_TAGOP_GEN2_DENATRAN_IAV_ACTIVATESECUREMODE:
        {
          TMR_TagOp_GEN2_Denatran_IAV_Activate_Secure_Mode *args;
          LLRP_tSThingMagicDenatranIAVActivateSecureMode   *pTMDenatranIAVActivateSecureMode;
          LLRP_tSThingMagicDenatranIAVCommandRequest       *pTMDenatranIAVCommandRequest;

          args = &tagop->u.gen2.u.custom.u.IavDenatran.u.secureMode;

          /* Construct and initialize the TMDenatranIAV ActivateSecureMode OpSpec */
          pTMDenatranIAVActivateSecureMode = LLRP_ThingMagicDenatranIAVActivateSecureMode_construct();
          /* Construct and initialize the TM DenatranIAV Command request */
          pTMDenatranIAVCommandRequest = LLRP_ThingMagicDenatranIAVCommandRequest_construct();
          /* Set the OpSpecId to the TM Denatran IAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setOpSpecID(pTMDenatranIAVCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set the DenatranIAV Payload value to the TM DenatranIAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setPayLoad(pTMDenatranIAVCommandRequest,
              args->payload);
          /* Set the DenatranIAV Command request to the TM DenatranIAV ActivateSecureMode */
          LLRP_ThingMagicDenatranIAVActivateSecureMode_setThingMagicDenatranIAVCommandRequest(pTMDenatranIAVActivateSecureMode,
              pTMDenatranIAVCommandRequest);

          /**
           * Set TMDenatranIAV ActivateecureMode as opSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMDenatranIAVActivateSecureMode);
          break;
        }
      case TMR_TAGOP_GEN2_DENATRAN_IAV_AUTHENTICATEOBU:
        {
          TMR_TagOp_GEN2_Denatran_IAV_Authenticate_OBU *args;
          LLRP_tSThingMagicDenatranIAVAuthenticateOBU  *pTMDenatranIAVAuthenticateOBU;
          LLRP_tSThingMagicDenatranIAVCommandRequest   *pTMDenatranIAVCommandRequest;

          args = &tagop->u.gen2.u.custom.u.IavDenatran.u.authenticateOBU;

          /* Construct and Initialize the TMDenatranIAV AuthenticateOBU OpSpec */
          pTMDenatranIAVAuthenticateOBU = LLRP_ThingMagicDenatranIAVAuthenticateOBU_construct();
          /* Construct and Initialize the TMDenatranIAV Command request */
          pTMDenatranIAVCommandRequest = LLRP_ThingMagicDenatranIAVCommandRequest_construct();
          /* Set the OpSpecId to the TM Denatran IAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setOpSpecID(pTMDenatranIAVCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set the DenatranIAV Payload value to the TM DenatranIAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setPayLoad(pTMDenatranIAVCommandRequest,
              args->payload);
          /* Set the DenatranIAV Command request to the TM DenatranIAV ActivateSecureMode */
          LLRP_ThingMagicDenatranIAVAuthenticateOBU_setThingMagicDenatranIAVCommandRequest(pTMDenatranIAVAuthenticateOBU,
              pTMDenatranIAVCommandRequest);

          /**
           * Set TMDenatranIAV ActivateecureMode as opSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMDenatranIAVAuthenticateOBU);
          break;
        }
      case TMR_TAGOP_GEN2_ACTIVATE_SINIAV_MODE:
        {
          TMR_TagOp_GEN2_Denatran_IAV_Activate_Siniav_Mode *args;
          LLRP_tSThingMagicDenatranIAVActivateSiniavMode   *pTMDenatranIAVSiniavMode;
          LLRP_tSThingMagicDenatranIAVCommandRequest       *pTMDenatranIAVCommandRequest;

          args = &tagop->u.gen2.u.custom.u.IavDenatran.u.activateSiniavMode;

          /* Construct and Initialize the TMDenatranIAV ActivateSiniavMode OpSpec */
          pTMDenatranIAVSiniavMode = LLRP_ThingMagicDenatranIAVActivateSiniavMode_construct();
          /* Construct and Initialize the TMDenatranIAV Command request */
          pTMDenatranIAVCommandRequest = LLRP_ThingMagicDenatranIAVCommandRequest_construct();
          /* Set the OpSpecId to the TM Denatran IAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setOpSpecID(pTMDenatranIAVCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set the DenatranIAV Payload value to the TM DenatranIAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setPayLoad(pTMDenatranIAVCommandRequest,
              args->payload);
          {
            /**
             * The length and the format of the token field depends on the
             * token description bits. Alocate memory accordingly.
             **/
            llrp_u8v_t temp;
            uint8_t len = 8;

            temp = LLRP_u8v_construct((llrp_u16_t)len);
            memcpy(temp.pValue, args->token, (size_t)temp.nValue);
            LLRP_ThingMagicDenatranIAVActivateSiniavMode_settokenData(pTMDenatranIAVSiniavMode,
                temp);
          }

          /* Set the DenatranIAV Command request to the TM DenatranIAV ActivateSecureMode */
          LLRP_ThingMagicDenatranIAVActivateSiniavMode_setThingMagicDenatranIAVCommandRequest(pTMDenatranIAVSiniavMode,
              pTMDenatranIAVCommandRequest);

          /**
           * Set TMDenatranIAV ActivateecureMode as opSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMDenatranIAVSiniavMode);
          break;
        }
      case TMR_TAGOP_GEN2_OBU_AUTH_ID:
        {
          TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_ID       *args;
          LLRP_tSThingMagicDenatranIAVOBUAuthenticateID *pTMDenatranIAVOBUAuthID;
          LLRP_tSThingMagicDenatranIAVCommandRequest    *pTMDenatranIAVCommandRequest;

          args = &tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthId;

          /* Construct and Initialize the TMDenatranIAV OBU AuthID OpSpec */
          pTMDenatranIAVOBUAuthID = LLRP_ThingMagicDenatranIAVOBUAuthenticateID_construct();
          /* Construct and Initialize the TMDenatranIAV Command request */
          pTMDenatranIAVCommandRequest = LLRP_ThingMagicDenatranIAVCommandRequest_construct();
          /* Set the OpSpecId to the TM Denatran IAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setOpSpecID(pTMDenatranIAVCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set the DenatranIAV Payload value to the TM DenatranIAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setPayLoad(pTMDenatranIAVCommandRequest,
              args->payload);
          /* Set the DenatranIAV Command request to the TM DenatranIAV ActivateSecureMode */
          LLRP_ThingMagicDenatranIAVOBUAuthenticateID_setThingMagicDenatranIAVCommandRequest(pTMDenatranIAVOBUAuthID,
              pTMDenatranIAVCommandRequest);

          /**
           * Set TMDenatranIAV ActivateecureMode as opSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMDenatranIAVOBUAuthID);
          break;
        }
      case TMR_TAGOP_GEN2_AUTHENTICATE_OBU_FULL_PASS1:
        {
          TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_Full_Pass1      *args;
          LLRP_tSThingMagicDenatranIAVOBUAuthenticateFullPass1 *pTMDenatranIAVOBUFullPass1;
          LLRP_tSThingMagicDenatranIAVCommandRequest           *pTMDenatranIAVCommandRequest;

          args = &tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthFullPass1;

          /* Construct and Initialize the TMDenatranIAV OBU Full Pass1 OpSpec */
          pTMDenatranIAVOBUFullPass1 = LLRP_ThingMagicDenatranIAVOBUAuthenticateFullPass1_construct();
          /* Construct and Initialize the TMDenatranIAV Command request */
          pTMDenatranIAVCommandRequest = LLRP_ThingMagicDenatranIAVCommandRequest_construct();
          /* Set the OpSpecId to the TM Denatran IAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setOpSpecID(pTMDenatranIAVCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set the DenatranIAV Payload value to the TM DenatranIAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setPayLoad(pTMDenatranIAVCommandRequest,
              args->payload);
          /* Set the DenatranIAV Command request to the TM DenatranIAV ActivateSecureMode */
          LLRP_ThingMagicDenatranIAVOBUAuthenticateFullPass1_setThingMagicDenatranIAVCommandRequest(pTMDenatranIAVOBUFullPass1,
              pTMDenatranIAVCommandRequest);

          /**
           * Set TMDenatranIAV ActivateecureMode as opSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMDenatranIAVOBUFullPass1);
          break;
        }
      case TMR_TAGOP_GEN2_AUTHENTICATE_OBU_FULL_PASS2:
        {
          TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_Full_Pass2      *args;
          LLRP_tSThingMagicDenatranIAVOBUAuthenticateFullPass2 *pTMDenatranIAVOBUFullPass2;
          LLRP_tSThingMagicDenatranIAVCommandRequest           *pTMDenatranIAVCommandRequest;

          args = &tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthFullPass2;

          /* Construct and Initialize the TMDenatranIAV OBU Full Pass2 OpSpec */
          pTMDenatranIAVOBUFullPass2 = LLRP_ThingMagicDenatranIAVOBUAuthenticateFullPass2_construct();
          /* Construct and Initialize the TMDenatranIAV Command request */
          pTMDenatranIAVCommandRequest = LLRP_ThingMagicDenatranIAVCommandRequest_construct();
          /* Set the OpSpecId to the TM Denatran IAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setOpSpecID(pTMDenatranIAVCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set the DenatranIAV Payload value to the TM DenatranIAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setPayLoad(pTMDenatranIAVCommandRequest,
              args->payload);
          /* Set the DenatranIAV Command request to the TM DenatranIAV ActivateSecureMode */
          LLRP_ThingMagicDenatranIAVOBUAuthenticateFullPass2_setThingMagicDenatranIAVCommandRequest(pTMDenatranIAVOBUFullPass2,
              pTMDenatranIAVCommandRequest);

          /**
           * Set TMDenatranIAV ActivateecureMode as opSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMDenatranIAVOBUFullPass2);
          break;
        }
      case TMR_TAGOP_GEN2_OBU_READ_FROM_MEM_MAP:
        {
          TMR_TagOp_GEN2_Denatran_IAV_OBU_ReadFromMemMap *args;
          LLRP_tSThingMagicDenatranIAVOBUReadFromMemMap  *pTMDenatranIAVOBUReadFromMemMap;
          LLRP_tSThingMagicDenatranIAVCommandRequest     *pTMDenatranIAVCommandRequest;

          args = &tagop->u.gen2.u.custom.u.IavDenatran.u.obuReadFromMemMap;

          /* Construct and Initialize the TMDenatranIAV OBU ReadFromMemMap OpSpec */
          pTMDenatranIAVOBUReadFromMemMap = LLRP_ThingMagicDenatranIAVOBUReadFromMemMap_construct();
          /* Construct and Initialize the TMDenatranIAV Command request */
          pTMDenatranIAVCommandRequest = LLRP_ThingMagicDenatranIAVCommandRequest_construct();
          /* Set the OpSpecId to the TM Denatran IAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setOpSpecID(pTMDenatranIAVCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set the DenatranIAV Payload value to the TM DenatranIAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setPayLoad(pTMDenatranIAVCommandRequest,
              args->payload);
          /* Set the read pointer to the TMDenatranIAV OBU ReadFromMemMap read pointer */
          LLRP_ThingMagicDenatranIAVOBUReadFromMemMap_setReadPtr(pTMDenatranIAVOBUReadFromMemMap,
              args->readPtr);
          /* Set the DenatranIAV Command request to the TM DenatranIAV ActivateSecureMode */
          LLRP_ThingMagicDenatranIAVOBUReadFromMemMap_setThingMagicDenatranIAVCommandRequest(pTMDenatranIAVOBUReadFromMemMap,
              pTMDenatranIAVCommandRequest);

          /**
           * Set TMDenatranIAV ActivateecureMode as opSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMDenatranIAVOBUReadFromMemMap);
          break;
        }
      case TMR_TAGOP_GEN2_OBU_WRITE_TO_MEM_MAP:
        {
          TMR_TagOp_GEN2_Denatran_IAV_OBU_WriteToMemMap *args;
          LLRP_tSThingMagicDenatranIAVOBUWriteToMemMap  *pTMDenatranIAVOBUWriteToMemMap;
          LLRP_tSThingMagicDenatranIAVCommandRequest     *pTMDenatranIAVCommandRequest;

          args = &tagop->u.gen2.u.custom.u.IavDenatran.u.obuWriteToMemMap;

          /* Construct and Initialize the TMDenatranIAV OBU WriteToMemMap OpSpec */
          pTMDenatranIAVOBUWriteToMemMap = LLRP_ThingMagicDenatranIAVOBUWriteToMemMap_construct();
          /* Construct and Initialize the TMDenatranIAV Command request */
          pTMDenatranIAVCommandRequest = LLRP_ThingMagicDenatranIAVCommandRequest_construct();
          /* Set the OpSpecId to the TM Denatran IAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setOpSpecID(pTMDenatranIAVCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set the DenatranIAV Payload value to the TM DenatranIAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setPayLoad(pTMDenatranIAVCommandRequest,
              args->payload);
          /* Set the write pointer to the TM DenatranIAV command write to mem map */
          LLRP_ThingMagicDenatranIAVOBUWriteToMemMap_setWritePtr(pTMDenatranIAVOBUWriteToMemMap,
              args->writePtr);
          /* Set the data to be written to the TM DenatranIAV Command writeToMemMap */
          LLRP_ThingMagicDenatranIAVOBUWriteToMemMap_setWordData(pTMDenatranIAVOBUWriteToMemMap,
              args->wordData);

          {
            /**
             * Here is length of the tag identification is predefined to 
             * be 8 bytes. allocate that much of memory only
             **/
            llrp_u8v_t temp; 
            uint8_t len = 8;

            temp = LLRP_u8v_construct((llrp_u16_t)len);
            memcpy(temp.pValue, args->dataBuf, (size_t)temp.nValue);
            LLRP_ThingMagicDenatranIAVOBUWriteToMemMap_setTagIdentification(pTMDenatranIAVOBUWriteToMemMap,
                temp);
          }

          {
            /** 
             * Here is length is predefined to be 16 bytes,
             * allocate that much of memory only
             **/
            llrp_u8v_t temp; 
            uint8_t len = 16;

            temp = LLRP_u8v_construct((llrp_u16_t)len);
            memcpy(temp.pValue, args->dataBuf, (size_t)temp.nValue);
            LLRP_ThingMagicDenatranIAVOBUWriteToMemMap_setDataBuf(pTMDenatranIAVOBUWriteToMemMap,
                temp);
          }

          /* Set the DenatranIAV Command request to the TM DenatranIAV ActivateSecureMode */
          LLRP_ThingMagicDenatranIAVOBUWriteToMemMap_setThingMagicDenatranIAVCommandRequest(pTMDenatranIAVOBUWriteToMemMap,
              pTMDenatranIAVCommandRequest);

          /**
           * Set TMDenatranIAV ActivateecureMode as opSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMDenatranIAVOBUWriteToMemMap);
          break;
        }

      case TMR_TAGOP_GEN2_DENATRAN_IAV_GET_TOKEN_ID:
        {
          LLRP_tSThingMagicDenatranIAVGetTokenID           *pTMDenatranIAVGetTokenID;

          /* Construct and initialize the TMDenatranIAV GetTokenID OpSpec */
          pTMDenatranIAVGetTokenID = LLRP_ThingMagicDenatranIAVGetTokenID_construct();
          /* Set the OpSpecId to the TM Denatran IAV Get Token ID */
          LLRP_ThingMagicDenatranIAVGetTokenID_setOpSpecID(pTMDenatranIAVGetTokenID,
              reader->u.llrpReader.opSpecId);

          /**
           * Set TMDenatranIAV GetTokenID as opSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMDenatranIAVGetTokenID);
          break;
        }

      case TMR_TAGOP_GEN2_DENATRAN_IAV_AUTHENTICATE_OBU_FULL_PASS:
        {
          TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_Full_Pass   *args;
          LLRP_tSThingMagicDenatranIAVOBUAuthFullPass      *pTMDenatranIAVOBUAuthFullPass;
          LLRP_tSThingMagicDenatranIAVCommandRequest       *pTMDenatranIAVCommandRequest;

          args = &tagop->u.gen2.u.custom.u.IavDenatran.u.obuAuthFullPass;

          /* Construct and initialize the TMDenatranIAV OBUAuthFullPass OpSpec */
          pTMDenatranIAVOBUAuthFullPass = LLRP_ThingMagicDenatranIAVOBUAuthFullPass_construct();
          /* Construct and initialize the TM DenatranIAV Command request */
          pTMDenatranIAVCommandRequest = LLRP_ThingMagicDenatranIAVCommandRequest_construct();
          /* Set the OpSpecId to the TM Denatran IAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setOpSpecID(pTMDenatranIAVCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set the DenatranIAV Payload value to the TM DenatranIAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setPayLoad(pTMDenatranIAVCommandRequest,
              args->payload);
          /* Set the DenatranIAV Command request to the TM DenatranIAV OBUAuthFullPass */
          LLRP_ThingMagicDenatranIAVOBUAuthFullPass_setThingMagicDenatranIAVCommandRequest(pTMDenatranIAVOBUAuthFullPass,
              pTMDenatranIAVCommandRequest);

          /**
           * Set TMDenatranIAV OBUAuthFullPass as opSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMDenatranIAVOBUAuthFullPass);
          break;
        }

      case TMR_TAGOP_GEN2_DENATRAN_IAV_READ_SEC:
        {
          TMR_TagOp_GEN2_Denatran_IAV_Read_Sec             *args;
          LLRP_tSThingMagicDenatranIAVReadSec              *pTMDenatranIAVReadSec;
          LLRP_tSThingMagicDenatranIAVCommandRequest       *pTMDenatranIAVCommandRequest;

          args = &tagop->u.gen2.u.custom.u.IavDenatran.u.readSec;

          /* Construct and initialize the TMDenatranIAV ReadSec OpSpec */
          pTMDenatranIAVReadSec = LLRP_ThingMagicDenatranIAVReadSec_construct();
          /* Construct and initialize the TM DenatranIAV Command request */
          pTMDenatranIAVCommandRequest = LLRP_ThingMagicDenatranIAVCommandRequest_construct();
          /* Set the OpSpecId to the TM Denatran IAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setOpSpecID(pTMDenatranIAVCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set the DenatranIAV Payload value to the TM DenatranIAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setPayLoad(pTMDenatranIAVCommandRequest,
              args->payload);
          /* Set the DenatranIAV Command request to the TM DenatranIAV ReadSec */
          LLRP_ThingMagicDenatranIAVReadSec_setThingMagicDenatranIAVCommandRequest(pTMDenatranIAVReadSec,
              pTMDenatranIAVCommandRequest);

          /* Set the DenatranIAV ReadSec read pointer to the TM DenatranIAV ReadSec */
          LLRP_ThingMagicDenatranIAVReadSec_setReadSec(pTMDenatranIAVReadSec,
              args->readPtr);
          /**
           * Set TMDenatranIAV ReadSec as opSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMDenatranIAVReadSec);
          break;
        }

      case TMR_TAGOP_GEN2_DENATRAN_IAV_WRITE_SEC:
        {
          TMR_TagOp_GEN2_Denatran_IAV_Write_Sec            *args;
          LLRP_tSThingMagicDenatranIAVWriteSec             *pTMDenatranIAVWriteSec;
          LLRP_tSThingMagicDenatranIAVCommandRequest       *pTMDenatranIAVCommandRequest;

          args = &tagop->u.gen2.u.custom.u.IavDenatran.u.writeSec;

          /* Construct and initialize the TMDenatranIAV WriteSec OpSpec */
          pTMDenatranIAVWriteSec = LLRP_ThingMagicDenatranIAVWriteSec_construct();
          /* Construct and initialize the TM DenatranIAV Command request */
          pTMDenatranIAVCommandRequest = LLRP_ThingMagicDenatranIAVCommandRequest_construct();
          /* Set the OpSpecId to the TM Denatran IAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setOpSpecID(pTMDenatranIAVCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set the DenatranIAV Payload value to the TM DenatranIAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setPayLoad(pTMDenatranIAVCommandRequest,
              args->payload);
          /* Set the DenatranIAV Command request to the TM DenatranIAV WriteSec */
          LLRP_ThingMagicDenatranIAVWriteSec_setThingMagicDenatranIAVCommandRequest(pTMDenatranIAVWriteSec,
              pTMDenatranIAVCommandRequest);

          /* Set the DenatranIAV writeSec credentials to the TM DenatranIAV WriteSec */
          {
            /**
             * Here is length is predefined to 
             * be 16 bytes. allocate that much of memory only
             **/
            llrp_u8v_t temp; 
            uint8_t len = 16;

            temp = LLRP_u8v_construct((llrp_u16_t)len);
            memcpy(temp.pValue, args->dataBuf, (size_t)temp.nValue);
            LLRP_ThingMagicDenatranIAVWriteSec_setDataBuf(pTMDenatranIAVWriteSec,
                temp);
          }

          /* Set the DenatranIAV writeSec data to the TM DenatranIAV WriteSec */
          {
            /** 
             * Here is length is predefined to be 6 bytes,
             * allocate that much of memory only
             **/
            llrp_u8v_t temp; 
            uint8_t len = 6;

            temp = LLRP_u8v_construct((llrp_u16_t)len);
            memcpy(temp.pValue, args->dataWords, (size_t)temp.nValue);
            LLRP_ThingMagicDenatranIAVWriteSec_setValueBuf(pTMDenatranIAVWriteSec,
                temp);
          }

          /**
           * Set TMDenatranIAV WriteSec as opSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMDenatranIAVWriteSec);
          break;
        }

      case TMR_TAGOP_GEN2_DENATRAN_IAV_G0_PA_OBU_AUTHENTICATE_ID:
        {
          TMR_TagOp_GEN2_Denatran_IAV_G0_PA_OBU_Auth_ID    *args;
          LLRP_tSThingMagicDenatranIAVG0PAOBUAuth          *pTMDenatranIAVG0PAOBUAuth;
          LLRP_tSThingMagicDenatranIAVCommandRequest       *pTMDenatranIAVCommandRequest;

          args = &tagop->u.gen2.u.custom.u.IavDenatran.u.g0paobuauthid;

          /* Construct and initialize the TMDenatranIAV G0PAOBUAuth OpSpec */
          pTMDenatranIAVG0PAOBUAuth = LLRP_ThingMagicDenatranIAVG0PAOBUAuth_construct();
          /* Construct and initialize the TM DenatranIAV Command request */
          pTMDenatranIAVCommandRequest = LLRP_ThingMagicDenatranIAVCommandRequest_construct();
          /* Set the OpSpecId to the TM Denatran IAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setOpSpecID(pTMDenatranIAVCommandRequest,
              reader->u.llrpReader.opSpecId);
          /* Set the DenatranIAV Payload value to the TM DenatranIAV Command request */
          LLRP_ThingMagicDenatranIAVCommandRequest_setPayLoad(pTMDenatranIAVCommandRequest,
              args->payload);
          /* Set the DenatranIAV Command request to the TM DenatranIAV G0PAOBUAuth */
          LLRP_ThingMagicDenatranIAVG0PAOBUAuth_setThingMagicDenatranIAVCommandRequest(pTMDenatranIAVG0PAOBUAuth,
              pTMDenatranIAVCommandRequest);

          /**
           * Set TMDenatranIAV G0PAOBUAuthenticate as opSpec to accessSpec
           */
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMDenatranIAVG0PAOBUAuth);
          break;
        }

#ifdef TMR_ENABLE_ISO180006B
      case TMR_TAGOP_ISO180006B_READDATA:
        {
          TMR_TagOp_ISO180006B_ReadData    *args;
          LLRP_tSThingMagicISO180006BRead  *pTMisoRead;
          llrp_u16_t                        data;

          args = &tagop->u.iso180006b.u.readData;

          /* Construct and initialize TMISO Read opspec */
          pTMisoRead = LLRP_ThingMagicISO180006BRead_construct();
          /* Set OpSpecId */
          LLRP_ThingMagicISO180006BRead_setOpSpecID(pTMisoRead, reader->u.llrpReader.opSpecId);
          /* Set the byte address */
          /* As API  datatype is uint8_t but
           * llrp takes  of type uint16_t
           * so, mapping the uint8_t data into uint16_t
           */
          data = args->byteAddress;
          LLRP_ThingMagicISO180006BRead_setByteAddress(pTMisoRead, data);

          /* Set the byte len */
          data = args->len;
          LLRP_ThingMagicISO180006BRead_setByteLen(pTMisoRead, data);

          /**
           * Set TMisoRead as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMisoRead);
          break;
        }
       
      case TMR_TAGOP_ISO180006B_WRITEDATA:
        {
          TMR_TagOp_ISO180006B_WriteData    *args;
          LLRP_tSThingMagicISO180006BWrite  *pTMisoWriteData;
          llrp_u8v_t                         writeData;
          llrp_u16_t                         data;

          args = &tagop->u.iso180006b.u.writeData;

          /* Construct and initialize the TM iso write data */
          pTMisoWriteData = LLRP_ThingMagicISO180006BWrite_construct();
          /*  Set the OpSpec id */
          LLRP_ThingMagicISO180006BWrite_setOpSpecID(pTMisoWriteData, reader->u.llrpReader.opSpecId);
          /* Set the byte addresss */
          /* As API datatype  is uint8_t but
           * llrp takes  of type uint16_t
           * so, mapping the uint8_t data into uint16_t
           **/
          data = args->byteAddress;
          LLRP_ThingMagicISO180006BWrite_setByteAddress(pTMisoWriteData, data);
          /* set the data to be written */
          writeData = LLRP_u8v_construct(args->data.len);
          memcpy(writeData.pValue, args->data.list, writeData.nValue);
          LLRP_ThingMagicISO180006BWrite_setWriteData(pTMisoWriteData,
              writeData);

          /**
           * Set TMisoRead as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMisoWriteData);

          break;
        }
        
      case TMR_TAGOP_ISO180006B_LOCK:
        {
          TMR_TagOp_ISO180006B_Lock          *args;
          LLRP_tSThingMagicISO180006BLock    *pTMisoLock;

          args = &tagop->u.iso180006b.u.lock;

          /* Construct and initialize the TM iso write data */
          pTMisoLock = LLRP_ThingMagicISO180006BLock_construct();
          /*  Set the OpSpec id */
          LLRP_ThingMagicISO180006BLock_setOpSpecID(pTMisoLock, reader->u.llrpReader.opSpecId);
          /* Set the address to lock */
          LLRP_ThingMagicISO180006BLock_setAddress(pTMisoLock, args->address);

          /**
           * Set TMisoLock as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMisoLock);

          break;
        }
#endif /* TMR_ENABLE_ISO180006B */

      case TMR_TAGOP_GEN2_NXP_AUTHENTICATE:
        {
          TMR_GEN2_AUTHENTICATE_Type  *pType;
          TMR_TagOp_GEN2_NXP_Tam1Authentication  *pTam1Data;
          TMR_TagOp_GEN2_NXP_Tam2Authentication  *pTam2Data;
          llrp_u8v_t key;

          LLRP_tSThingMagicNXPAuthentication *pTMNXPAuthentication;
          LLRP_tSThingMagicNXPCommandRequest *pTMNXPCommandRequest;
          LLRP_tSThingMagicNXPTAM1AuthenticationData  *pTMNXPTam1AuthData;
          LLRP_tSThingMagicNXPTAM2AuthenticationData  *pTMNXPTam2AuthData;          

          pType = &tagop->u.gen2.u.custom.u.nxp.u.authenticate.type;
          pTam1Data = &tagop->u.gen2.u.custom.u.nxp.u.authenticate.tam1Auth;
          pTam2Data = &tagop->u.gen2.u.custom.u.nxp.u.authenticate.tam2Auth;
          /* Construct and initialize TMNXP for authentications */
          pTMNXPAuthentication = LLRP_ThingMagicNXPAuthentication_construct();
          /* Construct and initialize TMNXP Command Request */
          pTMNXPCommandRequest = LLRP_ThingMagicNXPCommandRequest_construct();
          /* Set OpSpecId to TMNXP Command Request */
          LLRP_ThingMagicNXPCommandRequest_setOpSpecID(pTMNXPCommandRequest,
              reader->u.llrpReader.opSpecId);
          LLRP_ThingMagicNXPCommandRequest_setAccessPassword(pTMNXPCommandRequest,
              reader->u.llrpReader.gen2AccessPassword);
          LLRP_ThingMagicNXPAuthentication_setThingMagicNXPCommandRequest(pTMNXPAuthentication, pTMNXPCommandRequest);
          if (*pType == TAM1_AUTHENTICATION)
          {
            LLRP_ThingMagicNXPAuthentication_settype(pTMNXPAuthentication, TAM1_AUTHENTICATION);
            pTMNXPTam1AuthData = LLRP_ThingMagicNXPTAM1AuthenticationData_construct();
            LLRP_ThingMagicNXPTAM1AuthenticationData_setAuthentication(pTMNXPTam1AuthData, pTam1Data->Authentication);
            LLRP_ThingMagicNXPTAM1AuthenticationData_setCSI(pTMNXPTam1AuthData, pTam1Data->CSI);
            LLRP_ThingMagicNXPTAM1AuthenticationData_setkeyID(pTMNXPTam1AuthData, pTam1Data->keyID);
            LLRP_ThingMagicNXPTAM1AuthenticationData_setKeyLength(pTMNXPTam1AuthData, pTam1Data->KeyLength);
            key = LLRP_u8v_construct (pTam1Data->Key.len);
            memcpy(key.pValue, pTam1Data->Key.list, pTam1Data->Key.len);
       
            LLRP_ThingMagicNXPTAM1AuthenticationData_setKey(pTMNXPTam1AuthData, key);

            LLRP_ThingMagicNXPAuthentication_setThingMagicNXPTAM1AuthenticationData(pTMNXPAuthentication, pTMNXPTam1AuthData);
          }
          else if(*pType == TAM2_AUTHENTICATION)
          {
            LLRP_ThingMagicNXPAuthentication_settype(pTMNXPAuthentication, TAM2_AUTHENTICATION);
            pTMNXPTam1AuthData = LLRP_ThingMagicNXPTAM1AuthenticationData_construct();
            LLRP_ThingMagicNXPTAM1AuthenticationData_setAuthentication(pTMNXPTam1AuthData, pTam2Data->tam1Auth.Authentication);
            LLRP_ThingMagicNXPTAM1AuthenticationData_setCSI(pTMNXPTam1AuthData, pTam2Data->tam1Auth.CSI);
            LLRP_ThingMagicNXPTAM1AuthenticationData_setkeyID(pTMNXPTam1AuthData, pTam2Data->tam1Auth.keyID);
            LLRP_ThingMagicNXPTAM1AuthenticationData_setKeyLength(pTMNXPTam1AuthData, pTam2Data->tam1Auth.KeyLength);
            key = LLRP_u8v_construct (pTam2Data->tam1Auth.Key.len);
            memcpy(key.pValue, pTam2Data->tam1Auth.Key.list, pTam2Data->tam1Auth.Key.len);
            LLRP_ThingMagicNXPTAM1AuthenticationData_setKey(pTMNXPTam1AuthData, key);

            pTMNXPTam2AuthData = LLRP_ThingMagicNXPTAM2AuthenticationData_construct();
            LLRP_ThingMagicNXPTAM2AuthenticationData_setoffset(pTMNXPTam2AuthData, pTam2Data->Offset);
            LLRP_ThingMagicNXPTAM2AuthenticationData_setProtMode(pTMNXPTam2AuthData, pTam2Data->ProtMode );
            LLRP_ThingMagicNXPTAM2AuthenticationData_setBlockCount(pTMNXPTam2AuthData, pTam2Data->BlockCount);
            LLRP_ThingMagicNXPTAM2AuthenticationData_setprofile(pTMNXPTam2AuthData, pTam2Data->profile);
            LLRP_ThingMagicNXPTAM2AuthenticationData_setThingMagicNXPTAM1AuthenticationData(pTMNXPTam2AuthData, pTMNXPTam1AuthData);
            LLRP_ThingMagicNXPAuthentication_setThingMagicNXPTAM2AuthenticationData(pTMNXPAuthentication, pTMNXPTam2AuthData); 
          }
          LLRP_ThingMagicNXPAuthentication_setsubCommand(pTMNXPAuthentication, tagop->u.gen2.u.custom.u.nxp.u.authenticate.subCommand);
          /**
           * Set TMNXP authentication as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMNXPAuthentication);
          break;
        }

      case TMR_TAGOP_GEN2_NXP_READBUFFER:
        {
          LLRP_tSThingMagicNXPReadbuffer             *pTMNXPReadbuffer;
          LLRP_tSThingMagicNXPCommandRequest         *pTMNXPCommandRequest;
          LLRP_tSThingMagicNXPTAM1AuthenticationData *pTMNXPTam1AuthData;          
          LLRP_tSThingMagicNXPTAM2AuthenticationData *pTMNXPTam2AuthData;          
          TMR_TagOp_GEN2_NXP_Tam1Authentication      *pTam1Data;
          TMR_TagOp_GEN2_NXP_Tam2Authentication      *pTam2Data;
          llrp_u8v_t key;
          llrp_u8_t *pSubCommand;
          TMR_GEN2_AUTHENTICATE_Type *pType;
          llrp_u16_t *pWordPointer, *pBitCount;
          
          pWordPointer = &tagop->u.gen2.u.custom.u.nxp.u.readBuffer.wordPointer;
          pBitCount = &tagop->u.gen2.u.custom.u.nxp.u.readBuffer.bitCount;
          pType = &tagop->u.gen2.u.custom.u.nxp.u.readBuffer.authenticate.type;
          pSubCommand = &tagop->u.gen2.u.custom.u.nxp.u.readBuffer.authenticate.subCommand;

          /* Construct and initialize TMNXP for Readbuffer */
          pTMNXPReadbuffer = LLRP_ThingMagicNXPReadbuffer_construct();
          /* Construct and initialize TMNXP Command Request */
          pTMNXPCommandRequest = LLRP_ThingMagicNXPCommandRequest_construct();
          /* Set OpSpecId to TMNXP Command Request */
          LLRP_ThingMagicNXPCommandRequest_setOpSpecID(pTMNXPCommandRequest,
              reader->u.llrpReader.opSpecId);
          LLRP_ThingMagicNXPCommandRequest_setAccessPassword(pTMNXPCommandRequest,
              reader->u.llrpReader.gen2AccessPassword);
          LLRP_ThingMagicNXPReadbuffer_setThingMagicNXPCommandRequest(pTMNXPReadbuffer, pTMNXPCommandRequest);
          
          LLRP_ThingMagicNXPReadbuffer_setsubCommand(pTMNXPReadbuffer, *pSubCommand);
          LLRP_ThingMagicNXPReadbuffer_setAuthtype(pTMNXPReadbuffer, *pType);
          LLRP_ThingMagicNXPReadbuffer_setsubCommand(pTMNXPReadbuffer, tagop->u.gen2.u.custom.u.nxp.u.readBuffer.authenticate.subCommand);
          if (*pType == TAM1_AUTHENTICATION) {
          /* Construct and initialize TMNXP for authentications */
            pTam1Data = &tagop->u.gen2.u.custom.u.nxp.u.readBuffer.authenticate.tam1Auth;
            pTMNXPTam1AuthData = LLRP_ThingMagicNXPTAM1AuthenticationData_construct();
            LLRP_ThingMagicNXPTAM1AuthenticationData_setAuthentication(pTMNXPTam1AuthData, pTam1Data->Authentication);
            LLRP_ThingMagicNXPTAM1AuthenticationData_setCSI(pTMNXPTam1AuthData, pTam1Data->CSI);
            LLRP_ThingMagicNXPTAM1AuthenticationData_setkeyID(pTMNXPTam1AuthData, pTam1Data->keyID);
            LLRP_ThingMagicNXPTAM1AuthenticationData_setKeyLength(pTMNXPTam1AuthData, pTam1Data->KeyLength);
            key = LLRP_u8v_construct (pTam1Data->Key.len);
            memcpy(key.pValue, pTam1Data->Key.list, pTam1Data->Key.len);
       
            LLRP_ThingMagicNXPTAM1AuthenticationData_setKey(pTMNXPTam1AuthData, key);

            LLRP_ThingMagicNXPReadbuffer_setThingMagicNXPTAM1AuthenticationData(pTMNXPReadbuffer, pTMNXPTam1AuthData);
          }
          else if(*pType == TAM2_AUTHENTICATION) {
            pTam2Data = &tagop->u.gen2.u.custom.u.nxp.u.readBuffer.authenticate.tam2Auth;
            pTMNXPTam1AuthData = LLRP_ThingMagicNXPTAM1AuthenticationData_construct();
            LLRP_ThingMagicNXPTAM1AuthenticationData_setAuthentication(pTMNXPTam1AuthData, pTam2Data->tam1Auth.Authentication);
            LLRP_ThingMagicNXPTAM1AuthenticationData_setCSI(pTMNXPTam1AuthData, pTam2Data->tam1Auth.CSI);
            LLRP_ThingMagicNXPTAM1AuthenticationData_setkeyID(pTMNXPTam1AuthData, pTam2Data->tam1Auth.keyID);
            LLRP_ThingMagicNXPTAM1AuthenticationData_setKeyLength(pTMNXPTam1AuthData, pTam2Data->tam1Auth.KeyLength);
            key = LLRP_u8v_construct (pTam2Data->tam1Auth.Key.len);
            memcpy(key.pValue, pTam2Data->tam1Auth.Key.list, pTam2Data->tam1Auth.Key.len);
            LLRP_ThingMagicNXPTAM1AuthenticationData_setKey(pTMNXPTam1AuthData, key);

            pTMNXPTam2AuthData = LLRP_ThingMagicNXPTAM2AuthenticationData_construct();
            LLRP_ThingMagicNXPTAM2AuthenticationData_setoffset(pTMNXPTam2AuthData, pTam2Data->Offset);
            LLRP_ThingMagicNXPTAM2AuthenticationData_setProtMode(pTMNXPTam2AuthData, pTam2Data->ProtMode );
            LLRP_ThingMagicNXPTAM2AuthenticationData_setBlockCount(pTMNXPTam2AuthData, pTam2Data->BlockCount);
            LLRP_ThingMagicNXPTAM2AuthenticationData_setprofile(pTMNXPTam2AuthData, pTam2Data->profile);
            LLRP_ThingMagicNXPTAM2AuthenticationData_setThingMagicNXPTAM1AuthenticationData(pTMNXPTam2AuthData, pTMNXPTam1AuthData);
            LLRP_ThingMagicNXPReadbuffer_setThingMagicNXPTAM2AuthenticationData(pTMNXPReadbuffer, pTMNXPTam2AuthData); 
          }
          LLRP_ThingMagicNXPReadbuffer_setwordPointer(pTMNXPReadbuffer, *pWordPointer);
          LLRP_ThingMagicNXPReadbuffer_setbitCount(pTMNXPReadbuffer, *pBitCount);
          /**
           * Set TMNXP readbuffer as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMNXPReadbuffer);
          break;
        }
      case TMR_TAGOP_GEN2_NXP_UNTRACEABLE:
        {
          LLRP_tSThingMagicNXPUntraceable               *pTMNXPUntraceable;
          LLRP_tSThingMagicNXPUntraceableAuthentication *pTMNXPUntraceableAuth;
          LLRP_tSThingMagicNXPCommandRequest            *pTMNXPCommandRequest;
          LLRP_tSThingMagicNXPTAM1AuthenticationData    *pTMNXPTam1AuthData;          
          TMR_TagOp_GEN2_NXP_Tam1Authentication         *pTam1Data;
          TMR_GEN2_UNTRACEABLE_AuthType                 *pType;
          TMR_GEN2_UNTRACEABLE_Epc                      *pEpc;
          TMR_GEN2_UNTRACEABLE_Tid                      *pTid;
          TMR_GEN2_UNTRACEABLE_UserMemory               *pUserMemory;
          TMR_GEN2_UNTRACEABLE_Range                    *pRange;
          llrp_u8v_t key;
          int32_t *pEpcLength;
          uint8_t *pSubCommand;
          uint32_t accessPassword;
          
          pType = &tagop->u.gen2.u.custom.u.nxp.u.untraceable.auth.authType;
          pEpc = &tagop->u.gen2.u.custom.u.nxp.u.untraceable.epc;
          pTid = &tagop->u.gen2.u.custom.u.nxp.u.untraceable.tid;
          pUserMemory = &tagop->u.gen2.u.custom.u.nxp.u.untraceable.userMemory;
          pRange = &tagop->u.gen2.u.custom.u.nxp.u.untraceable.range;
          pEpcLength = &tagop->u.gen2.u.custom.u.nxp.u.untraceable.epcLength;
          pSubCommand = &tagop->u.gen2.u.custom.u.nxp.u.untraceable.subCommand;
          pTam1Data = &tagop->u.gen2.u.custom.u.nxp.u.untraceable.auth.tam1Auth;
          accessPassword = tagop->u.gen2.u.custom.u.nxp.u.untraceable.auth.accessPassword;

          /* Construct and initialize TMNXP for Untraceable */
          pTMNXPUntraceable = LLRP_ThingMagicNXPUntraceable_construct();
          /* Construct and initialize TMNXP Command Request */
          pTMNXPCommandRequest = LLRP_ThingMagicNXPCommandRequest_construct();
          /* Set OpSpecId to TMNXP Command Request */
          LLRP_ThingMagicNXPCommandRequest_setOpSpecID(pTMNXPCommandRequest,
              reader->u.llrpReader.opSpecId);
          LLRP_ThingMagicNXPCommandRequest_setAccessPassword(pTMNXPCommandRequest,
              reader->u.llrpReader.gen2AccessPassword);
          LLRP_ThingMagicNXPUntraceable_setThingMagicNXPCommandRequest(pTMNXPUntraceable, pTMNXPCommandRequest);

          /* Construct and initialize TMNXP for Untraceable authentication */
          pTMNXPUntraceableAuth = LLRP_ThingMagicNXPUntraceableAuthentication_construct();
          LLRP_ThingMagicNXPUntraceableAuthentication_setauthType(pTMNXPUntraceableAuth, *pType);
          LLRP_ThingMagicNXPUntraceable_setsubCommand(pTMNXPUntraceable, *pSubCommand);
          if (*pType == UNTRACEABLE_WITH_AUTHENTICATION) {
            /* Construct and initialize TMNXP for authentications */
            pTMNXPTam1AuthData = LLRP_ThingMagicNXPTAM1AuthenticationData_construct();
            LLRP_ThingMagicNXPTAM1AuthenticationData_setAuthentication(pTMNXPTam1AuthData, pTam1Data->Authentication);
            LLRP_ThingMagicNXPTAM1AuthenticationData_setCSI(pTMNXPTam1AuthData, pTam1Data->CSI);
            LLRP_ThingMagicNXPTAM1AuthenticationData_setkeyID(pTMNXPTam1AuthData, pTam1Data->keyID);
            LLRP_ThingMagicNXPTAM1AuthenticationData_setKeyLength(pTMNXPTam1AuthData, pTam1Data->KeyLength);
            key = LLRP_u8v_construct (pTam1Data->Key.len);
            memcpy(key.pValue, pTam1Data->Key.list, pTam1Data->Key.len);
       
            LLRP_ThingMagicNXPTAM1AuthenticationData_setKey(pTMNXPTam1AuthData, key);
            LLRP_ThingMagicNXPUntraceableAuthentication_setThingMagicNXPTAM1AuthenticationData(pTMNXPUntraceableAuth, pTMNXPTam1AuthData);
          }
          LLRP_ThingMagicNXPUntraceableAuthentication_setaccessPassword(pTMNXPUntraceableAuth, accessPassword);
          LLRP_ThingMagicNXPUntraceable_setThingMagicNXPUntraceableAuthentication(pTMNXPUntraceable, pTMNXPUntraceableAuth);

          LLRP_ThingMagicNXPUntraceable_setepc(pTMNXPUntraceable, *pEpc);
          LLRP_ThingMagicNXPUntraceable_setepcLength(pTMNXPUntraceable, *pEpcLength);
          LLRP_ThingMagicNXPUntraceable_settid(pTMNXPUntraceable, *pTid);
          LLRP_ThingMagicNXPUntraceable_setuserMemory(pTMNXPUntraceable, *pUserMemory);
          LLRP_ThingMagicNXPUntraceable_setrange(pTMNXPUntraceable, *pRange);
          LLRP_ThingMagicNXPUntraceable_setsubCommand(pTMNXPUntraceable, *pSubCommand);
          /**
           * Set TMNXP readbuffer as OpSpec to accessSpec
           **/
          LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
              (LLRP_tSParameter *)pTMNXPUntraceable);
          break;
        }

      case TMR_TAGOP_LIST:
        {
          TMR_TagOp_GEN2_WriteData    *pArgsWriteData;
          TMR_TagOp_GEN2_WriteTag     *pArgsWriteTag;
          TMR_TagOp_GEN2_ReadData     *pArgsReadData;
          LLRP_tSC1G2Write            *pC1G2WriteData;
          LLRP_tSThingMagicWriteTag   *pC1G2WriteTag;
          LLRP_tSC1G2Read             *pC1G2Read;
          llrp_u16v_t                 data;
          TMR_TagOp                   *pReadOp,*pWriteOp;
          int                         i,j;

          if (tagop->u.list.len == 1)
          {
            tagop = tagop->u.list.list[0];
            goto PREPARE_COMMAND;
          }
          if ((tagop->u.list.len <= 2) && (tagop->u.list.len != 0))
          {
            if ((tagop->u.list.list[0]->type == TMR_TAGOP_GEN2_WRITEDATA) && (tagop->u.list.list[1]->type == TMR_TAGOP_GEN2_READDATA))
            {
              pWriteOp = tagop->u.list.list[0];
              pArgsWriteData = &pWriteOp->u.gen2.u.writeData;

              /* Construct and initialize C1G2Write */
              pC1G2WriteData = LLRP_C1G2Write_construct();
              /* Set OpSpec Id */
              LLRP_C1G2Write_setOpSpecID(pC1G2WriteData, reader->u.llrpReader.opSpecId);
              /* Set access password */
              LLRP_C1G2Write_setAccessPassword(pC1G2WriteData, reader->u.llrpReader.gen2AccessPassword);
              /* Set Memory Bank */
              if ((pArgsWriteData->bank == TMR_GEN2_BANK_RESERVED) || (pArgsWriteData->bank == TMR_GEN2_BANK_EPC) ||
                  (pArgsWriteData->bank == TMR_GEN2_BANK_TID) || (pArgsWriteData->bank == TMR_GEN2_BANK_USER))
              {
                LLRP_C1G2Write_setMB(pC1G2WriteData, (llrp_u2_t)pArgsWriteData->bank);
              }
              else
              {
                return TMR_ERROR_INVALID; 
              }
              /* Set word pointer */
              LLRP_C1G2Write_setWordPointer(pC1G2WriteData, (llrp_u16_t)pArgsWriteData->wordAddress);
              /* Set the data to be written */
              data = LLRP_u16v_construct(pArgsWriteData->data.len);
              memcpy (data.pValue, pArgsWriteData->data.list, data.nValue * sizeof(uint16_t));
              LLRP_C1G2Write_setWriteData(pC1G2WriteData, data);

              /**
               * Set C1G2WriteData as OpSpec to accessSpec
               **/
          
              LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                  (LLRP_tSParameter *)pC1G2WriteData);
              
              reader->u.llrpReader.opSpecId ++;
            }
            else if ((tagop->u.list.list[0]->type == TMR_TAGOP_GEN2_WRITETAG) && (tagop->u.list.list[1]->type == TMR_TAGOP_GEN2_READDATA))
            {
              pWriteOp = tagop->u.list.list[0];
              pArgsWriteTag = &pWriteOp->u.gen2.u.writeTag;

              /* Construct and initialize ThingMagicWriteTag */
              pC1G2WriteTag = LLRP_ThingMagicWriteTag_construct();
              /* Set OpSpec Id */
              LLRP_ThingMagicWriteTag_setOpSpecID(pC1G2WriteTag, reader->u.llrpReader.opSpecId);
              /* Set access password */
              LLRP_ThingMagicWriteTag_setAccessPassword(pC1G2WriteTag, reader->u.llrpReader.gen2AccessPassword);
              /* Set the data to be written */
              /* As API epc datatype is uint8_t but
               * llrp takes epc of type uint16_t
               * so, mapping the uint8_t data into uint16_t
               */
              data = LLRP_u16v_construct((llrp_u16_t) (pArgsWriteTag->epcptr->epcByteCount / 2));
              for(i = 0, j = 0;i < data.nValue; i++, j+=2)
              {
                data.pValue[i] = (pArgsWriteTag->epcptr->epc[j] << 8) | pArgsWriteTag->epcptr->epc[j + 1];
              }

              LLRP_ThingMagicWriteTag_setWriteData(pC1G2WriteTag, data);

              /**
               * Set ThingMagicWriteTag as OpSpec to accessSpec
               **/
              LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand,
                  (LLRP_tSParameter *)pC1G2WriteTag);

              reader->u.llrpReader.opSpecId ++;
            }
            else
            {
              /**
               * We support maximum 2 OpSpecs per AccessSpec.
               * So return unsupported error.
               **/
              return TMR_ERROR_UNSUPPORTED;
            }

            {
              pReadOp = tagop->u.list.list[1];
              pArgsReadData = &pReadOp->u.gen2.u.readData;
          
              /* Construct and initialize C1G2Read */
              pC1G2Read = LLRP_C1G2Read_construct();
              /* Set OpSpec Id */
              LLRP_C1G2Read_setOpSpecID(pC1G2Read, reader->u.llrpReader.opSpecId);
              /* Set access password */
              LLRP_C1G2Read_setAccessPassword(pC1G2Read, reader->u.llrpReader.gen2AccessPassword);
              /* Set Memory Bank */
              if ((pArgsReadData->bank == TMR_GEN2_BANK_RESERVED) || (pArgsReadData->bank == TMR_GEN2_BANK_EPC) ||
                  (pArgsReadData->bank == TMR_GEN2_BANK_TID) || (pArgsReadData->bank == TMR_GEN2_BANK_USER))
              {
                LLRP_C1G2Read_setMB(pC1G2Read, (llrp_u2_t)pArgsReadData->bank);
              }
              else
              {
                return TMR_ERROR_INVALID; 
              }
              /* Set word pointer */
              LLRP_C1G2Read_setWordPointer(pC1G2Read, (llrp_u16_t)pArgsReadData->wordAddress);
              /* Set word length to read */
              LLRP_C1G2Read_setWordCount(pC1G2Read, pArgsReadData->len);

             /**
              * Set C1G2Read as OpSpec to accessSpec
              **/
              LLRP_AccessCommand_addAccessCommandOpSpec(pAccessCommand, 
                                              (LLRP_tSParameter *)pC1G2Read); 
            }
          }
          else
          {
            /**
             * We support maximum 2 OpSpecs per AccessSpec.
             * So return unsupported error.
             **/
            return TMR_ERROR_UNSUPPORTED;
          }
          break;
        }

      default:
        {
          /* Unknown tagop - return unsupported error */
          return TMR_ERROR_UNSUPPORTED; 
        }
    }
  }

  return ret;
}

/**
 * Command to Add an AccessSpec
 *
 * @param reader Reader pointer
 * @param protocol Protocol to be used
 * @param filter Pointer to Tag filter
 * @param roSpecId ROSpecID with which this AccessSpec need to be associated
 * @param tagop Pointer to TMR_TagOp
 * @param isStandalone Boolean variable to indicate whether a standalone
 *        or embedded operation.
 *        true = standalone operation, false = embedded operation.
 */ 
TMR_Status
TMR_LLRP_cmdAddAccessSpec(TMR_Reader *reader, 
                          TMR_TagProtocol protocol,
                          TMR_TagFilter *filter,
                          llrp_u32_t roSpecId,
                          TMR_TagOp *tagop,
                          bool isStandalone)
{
  TMR_Status ret;
  LLRP_tSADD_ACCESSSPEC               *pCmd;
  LLRP_tSMessage                      *pCmdMsg;
  LLRP_tSMessage                      *pRspMsg;
  LLRP_tSADD_ACCESSSPEC_RESPONSE      *pRsp;

  LLRP_tSAccessSpec                   *pAccessSpec;
  LLRP_tSAccessSpecStopTrigger        *pAccessSpecStopTrigger;
  LLRP_tSAccessCommand                *pAccessCommand;

  ret = TMR_SUCCESS;
  
  /**
   * Initialize AddAccessSpec message
   **/
  pCmd = LLRP_ADD_ACCESSSPEC_construct();

  /**
   *  Initialize AccessSpec and
   *  1. Set AccessSpec Id
   *  2. Set Antenna Id
   *  3. Set current state
   *  4. Set Protocol
   *  5. Set ROSpec Id
   *  6. Set AccessSpec Stop trigger
   *  7. Set Access Command operation
   *  8. Set AccessReportSpec
   **/

  /* Construct AccessSpec parameter */
  pAccessSpec = LLRP_AccessSpec_construct();
  if (NULL == pAccessSpec)
  {
    TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
    return TMR_ERROR_LLRP;
  }

  {
    /* 1. Set AccessSpec Id */
    LLRP_AccessSpec_setAccessSpecID(pAccessSpec, reader->u.llrpReader.accessSpecId);
    
    /* 2. Set Antenna Id */
    /**
     * In case of both embedded tag operation,
     * the antenna list is adjusted in
     * ROSpec's AISpec. And here set antenna Id to 0, so that this 
     * spec is operational on all antennas mentioned in AISpec.
     **/
    if (isStandalone)
    {
      /**
       * For standalone tag operation prepare antennaList
       * The operation has to be performed on the antenna specified
       * in the /reader/tagop/antenna parameter.
       **/ 
      uint8_t antenna = 0;

      antenna = reader->tagOpParams.antenna;
      LLRP_AccessSpec_setAntennaID(pAccessSpec, antenna);
    }
    else
    {
      LLRP_AccessSpec_setAntennaID(pAccessSpec, 0);
    }

    /* 3. Set Current State */
    LLRP_AccessSpec_setCurrentState(pAccessSpec, 
                                LLRP_AccessSpecState_Disabled);

    /* 4. Set Protocol */
    switch (protocol)
    {
      case TMR_TAG_PROTOCOL_GEN2:
        {
          LLRP_AccessSpec_setProtocolID(pAccessSpec,
              LLRP_AirProtocols_EPCGlobalClass1Gen2);

          break;
        }
      case TMR_TAG_PROTOCOL_ISO180006B:
        {
          LLRP_AccessSpec_setProtocolID(pAccessSpec,
              LLRP_AirProtocols_Unspecified);

          break;
        }
      default:
        {
          TMR_LLRP_freeMessage((LLRP_tSMessage *)pAccessSpec);
          TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
          return TMR_ERROR_UNIMPLEMENTED_FEATURE;

          break;
        }
    }


    /* 5. Set ROSpec ID */
    LLRP_AccessSpec_setROSpecID(pAccessSpec, roSpecId);

    if(reader->u.llrpReader.featureFlags & TMMP_READER_FEATURES_FLAG_INVENTORYSPEC_ID)
    {
      /*Set Inventory Spec ID as custom parameter to AccessSpec*/
      LLRP_tSThingMagicCustomInventorySpecID *pInventorySpecIDCustom;
      pInventorySpecIDCustom = LLRP_ThingMagicCustomInventorySpecID_construct();
      LLRP_ThingMagicCustomInventorySpecID_setInventorySpecId(pInventorySpecIDCustom,
          currentInventorySpecID);
      /* add this as a custom parameter to AccessSpec */
      LLRP_AccessSpec_addCustom(pAccessSpec, (LLRP_tSParameter *)pInventorySpecIDCustom); 
    }

    /* 6. Set AccessSpec Stop trigger */
    {
      /* Construct and initialize AccessSpec stop trigger */
      pAccessSpecStopTrigger = LLRP_AccessSpecStopTrigger_construct();

      /**
       * If stand alone operation, set operation count to 1.
       * else set it to 0 (i.e., No stop trigger defined)
       **/
      if (isStandalone)
      {
        /**
         * Set AccessSpec StopTrigger type to operation count
         **/
        LLRP_AccessSpecStopTrigger_setAccessSpecStopTrigger(pAccessSpecStopTrigger,
                              LLRP_AccessSpecStopTriggerType_Operation_Count);
        /**
         * In case of standalone operation, we want to execute this operation
         * only once on the first matching tag. So set operation count to 1
         **/
        LLRP_AccessSpecStopTrigger_setOperationCountValue(pAccessSpecStopTrigger, 1);
      }
      else
      {
        /**
         * Set AccessSpec StopTrigger type to NULL
         **/
        LLRP_AccessSpecStopTrigger_setAccessSpecStopTrigger(pAccessSpecStopTrigger,
                              LLRP_AccessSpecStopTriggerType_Null);
      }

      /* Set stoptrigger to accessspec */
      LLRP_AccessSpec_setAccessSpecStopTrigger(pAccessSpec, pAccessSpecStopTrigger);
    }

    /* 7. Set Access Command operation */
    {
      /* Initialize AccessCommand parameter */
      pAccessCommand = LLRP_AccessCommand_construct();

      /* prepare TagSpec and OpSpec and add them to AccessCommand */
      ret = TMR_LLRP_msgPrepareAccessCommand(reader, pAccessCommand, filter, tagop);
      if (TMR_SUCCESS != ret)
      {
        TMR_LLRP_freeMessage((LLRP_tSMessage *)pAccessCommand);
        TMR_LLRP_freeMessage((LLRP_tSMessage *)pAccessSpec);
        TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
        return ret;
      }

      /* Set AccessCommand to AccessSpec parameter */
      LLRP_AccessSpec_setAccessCommand(pAccessSpec, pAccessCommand);
    }

    /* 8. Set AccessReportSpec */
    /**
     * AccessReportSpec is optional, and the default value 
     * (i.e., Whenever ROReport is generated for the RO that
     * triggered the execution of this AccessSpec)
     * is what we want to set in any way. So nothing to do here.
     **/
  }

  /* Now AccessSpec is fully framed, add to ADD_ACCESSSPEC message */
  LLRP_ADD_ACCESSSPEC_setAccessSpec(pCmd, pAccessSpec);

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSADD_ACCESSSPEC_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

/**
 * Parse ThingMagic Custom TagOpSpec Result parameter status
 * @param status The Result type parameter
 **/
TMR_Status
TMR_LLRP_parseCustomTagOpSpecResultType(LLRP_tEThingMagicCustomTagOpSpecResultType status)
{
  switch (status)
  {
    case LLRP_ThingMagicCustomTagOpSpecResultType_Success:
      {
        return TMR_SUCCESS;
      }

    case LLRP_ThingMagicCustomTagOpSpecResultType_Nonspecific_Tag_Error:
      {
        return TMR_ERROR_GENERAL_TAG_ERROR;
      }

    case LLRP_ThingMagicCustomTagOpSpecResultType_No_Response_From_Tag:
      {
        return TMR_ERROR_GEN2_PROTOCOL_OTHER_ERROR;
      }

    case LLRP_ThingMagicCustomTagOpSpecResultType_Nonspecific_Reader_Error:
      {
        return TMR_ERROR_LLRP_READER_ERROR;
      }

    case LLRP_ThingMagicCustomTagOpSpecResultType_Tag_Memory_Overrun_Error:
      {
        return TMR_ERROR_GEN2_PROTOCOL_MEMORY_OVERRUN_BAD_PC;
      }

    case LLRP_ThingMagicCustomTagOpSpecResultType_Gen2V2_Authentication_Fail:
      {
        return TMR_ERROR_GEN2_PROTOCOL_V2_AUTHEN_FAILED;
      }

    case LLRP_ThingMagicCustomTagOpSpecResultType_Gen2V2_Untrace_Fail:
      {
        return TMR_ERROR_GEN2_PROTOCOL_V2_UNTRACE_FAILED;
      }

    default:
      return TMR_ERROR_LLRP;
  }
}

/**
 * Verify OpSpecResult status and return
 * appropriate API status code.
 *
 * @param reader Reader pointer
 * @param pParameter Pointer to LLRP_tSParameter which point to OpSpecResult
 **/
 
TMR_Status
TMR_LLRP_verifyOpSpecResultStatus(TMR_Reader *reader, 
                                  LLRP_tSParameter *pParameter)
{
  TMR_Status ret;

  ret = TMR_SUCCESS;

  /**
   * Compare the OpSpec Results based on the typenums
   **/
  switch (pParameter->elementHdr.pType->TypeNum)
  {

    /**
     * Read after writeOpSpecResult
     * Always 0 means success, if eResult is other than zero
     * then it is an error, translate the error into appropriate
     * API error.
     * Result will be same for both C1G2 read and read after write cases
     **/
    case TMR_LLRP_LISTOPSPECRESULT:
    /**
     * C1G2ReadOpSpecResult
     **/
    case TMR_LLRP_C1G2READOPSPECRESULT:
      {
        LLRP_tSC1G2ReadOpSpecResult *pC1G2ReadOpSpecResult;
        pC1G2ReadOpSpecResult = (LLRP_tSC1G2ReadOpSpecResult *)pParameter;

        switch (pC1G2ReadOpSpecResult->eResult)
        {
          case LLRP_C1G2ReadResultType_Success:
            {
              ret = TMR_SUCCESS;
              break;
            }

          case LLRP_C1G2ReadResultType_Nonspecific_Tag_Error:
            {
              ret = TMR_ERROR_GENERAL_TAG_ERROR;
              break;
            }

          case LLRP_C1G2ReadResultType_No_Response_From_Tag:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_OTHER_ERROR;
              break;
            }

          case LLRP_C1G2ReadResultType_Nonspecific_Reader_Error:
            {
              ret = TMR_ERROR_LLRP_READER_ERROR;
              break;
            }

          case LLRP_C1G2ReadResultType_Memory_Overrun_Error:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_MEMORY_OVERRUN_BAD_PC;
              break;
            }

          case LLRP_C1G2ReadResultType_Memory_Locked_Error:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_MEMORY_LOCKED;
              break;
            }

          default:
            ret = TMR_ERROR_LLRP;
        }
        break;
      }

    /**
     * C1G2WriteOpSpecResult
     **/
    case TMR_LLRP_C1G2WRITEOPSPECRESULT:
      {
        LLRP_tSC1G2WriteOpSpecResult *pC1G2WriteOpSpecResult;
        pC1G2WriteOpSpecResult = (LLRP_tSC1G2WriteOpSpecResult *)pParameter;

        switch (pC1G2WriteOpSpecResult->eResult)
        {
          case LLRP_C1G2WriteResultType_Success:
            {
              ret = TMR_SUCCESS;
              break;
            }

          case LLRP_C1G2WriteResultType_Tag_Memory_Overrun_Error:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_MEMORY_OVERRUN_BAD_PC;
              break;
            }

          case LLRP_C1G2WriteResultType_Tag_Memory_Locked_Error:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_MEMORY_LOCKED;
              break;
            }

          case LLRP_C1G2WriteResultType_Insufficient_Power:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_INSUFFICIENT_POWER;
              break;
            }

          case LLRP_C1G2WriteResultType_Nonspecific_Tag_Error:
            {
              ret = TMR_ERROR_GENERAL_TAG_ERROR;
              break;
            }

          case LLRP_C1G2WriteResultType_No_Response_From_Tag:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_OTHER_ERROR;
              break;
            }

          case LLRP_C1G2WriteResultType_Nonspecific_Reader_Error:
            {
              ret = TMR_ERROR_LLRP_READER_ERROR;
              break;
            }

          default:
            ret = TMR_ERROR_LLRP;
        }
        break;
      }

    /**
     * C1G2KillOpSpecResult
     **/
    case TMR_LLRP_C1G2KILLOPSPECRESULT:
      {
        LLRP_tSC1G2KillOpSpecResult *pC1G2KillOpSpecResult;
        pC1G2KillOpSpecResult = (LLRP_tSC1G2KillOpSpecResult *)pParameter;

        switch (pC1G2KillOpSpecResult->eResult)
        {
          case LLRP_C1G2KillResultType_Success:
            {
              ret = TMR_SUCCESS;
              break;
            }

          case LLRP_C1G2KillResultType_Zero_Kill_Password_Error:
            {
              ret = TMR_ERROR_PROTOCOL_INVALID_KILL_PASSWORD;
              break;
            }

          case LLRP_C1G2KillResultType_Insufficient_Power:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_INSUFFICIENT_POWER;
              break;
            }

          case LLRP_C1G2KillResultType_Nonspecific_Tag_Error:
            {
              ret = TMR_ERROR_GENERAL_TAG_ERROR;
              break;
            }

          case LLRP_C1G2KillResultType_No_Response_From_Tag:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_OTHER_ERROR;
              break;
            }

          case LLRP_C1G2KillResultType_Nonspecific_Reader_Error:
            {
              ret = TMR_ERROR_LLRP_READER_ERROR;
              break;
            }

          default:
            ret = TMR_ERROR_LLRP;
        }
        break;
      }

    /**
     * C1G2LockOpSpecResult
     **/
    case TMR_LLRP_C1G2LOCKOPSPECRESULT:
      {
        LLRP_tSC1G2LockOpSpecResult *pC1G2LockOpSpecResult;
        pC1G2LockOpSpecResult = (LLRP_tSC1G2LockOpSpecResult *)pParameter;

        switch (pC1G2LockOpSpecResult->eResult)
        {
          case LLRP_C1G2LockResultType_Success:
            {
              ret = TMR_SUCCESS;
              break;
            }

          case LLRP_C1G2LockResultType_Insufficient_Power:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_INSUFFICIENT_POWER;
              break;
            }

          case LLRP_C1G2LockResultType_Nonspecific_Tag_Error:
            {
              ret = TMR_ERROR_GENERAL_TAG_ERROR;
              break;
            }

          case LLRP_C1G2LockResultType_No_Response_From_Tag:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_OTHER_ERROR;
              break;
            }

          case LLRP_C1G2LockResultType_Nonspecific_Reader_Error:
            {
              ret = TMR_ERROR_LLRP_READER_ERROR;
              break;
            }

          default:
            ret = TMR_ERROR_LLRP;
        }
        break;
      }

    /**
     * C1G2BlockEraseOpSpecResult
     **/
    case TMR_LLRP_C1G2BLOCKERASEOPSPECRESULT:
      {
        LLRP_tSC1G2BlockEraseOpSpecResult *pC1G2BlockEraseOpSpecResult;
        pC1G2BlockEraseOpSpecResult = (LLRP_tSC1G2BlockEraseOpSpecResult *)pParameter;

        switch (pC1G2BlockEraseOpSpecResult->eResult)
        {
          case LLRP_C1G2BlockEraseResultType_Success:
            {
              ret = TMR_SUCCESS;
              break;
            }

          case LLRP_C1G2BlockEraseResultType_Tag_Memory_Overrun_Error:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_MEMORY_OVERRUN_BAD_PC;
              break;
            }

          case LLRP_C1G2BlockEraseResultType_Tag_Memory_Locked_Error:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_MEMORY_LOCKED;
              break;
            }

          case LLRP_C1G2BlockEraseResultType_Insufficient_Power:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_INSUFFICIENT_POWER;
              break;
            }

          case LLRP_C1G2BlockEraseResultType_Nonspecific_Tag_Error:
            {
              ret = TMR_ERROR_GENERAL_TAG_ERROR;
              break;
            }

          case LLRP_C1G2BlockEraseResultType_No_Response_From_Tag:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_OTHER_ERROR;
              break;
            }

          case LLRP_C1G2BlockEraseResultType_Nonspecific_Reader_Error:
            {
              ret = TMR_ERROR_LLRP_READER_ERROR;
              break;
            }

          default:
            ret = TMR_ERROR_LLRP;
        }
        break;
      }

    /**
     * C1G2BlockWriteOpSpecResult
     **/
    case TMR_LLRP_C1G2BLOCKWRITEOPSPECRESULT:
      {
        LLRP_tSC1G2BlockWriteOpSpecResult *pC1G2BlockWriteOpSpecResult;
        pC1G2BlockWriteOpSpecResult = (LLRP_tSC1G2BlockWriteOpSpecResult *)pParameter;

        switch (pC1G2BlockWriteOpSpecResult->eResult)
        {
          case LLRP_C1G2BlockWriteResultType_Success:
            {
              ret = TMR_SUCCESS;
              break;
            }

          case LLRP_C1G2BlockWriteResultType_Tag_Memory_Overrun_Error:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_MEMORY_OVERRUN_BAD_PC;
              break;
            }

          case LLRP_C1G2BlockWriteResultType_Tag_Memory_Locked_Error:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_MEMORY_LOCKED;
              break;
            }

          case LLRP_C1G2BlockWriteResultType_Insufficient_Power:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_INSUFFICIENT_POWER;
              break;
            }

          case LLRP_C1G2BlockWriteResultType_Nonspecific_Tag_Error:
            {
              ret = TMR_ERROR_GENERAL_TAG_ERROR;
              break;
            }

          case LLRP_C1G2BlockWriteResultType_No_Response_From_Tag:
            {
              ret = TMR_ERROR_GEN2_PROTOCOL_OTHER_ERROR;
              break;
            }

          case LLRP_C1G2BlockWriteResultType_Nonspecific_Reader_Error:
            {
              ret = TMR_ERROR_LLRP_READER_ERROR;
              break;
            }

          default:
            ret = TMR_ERROR_LLRP;
        }
        break;
      }

    /**
     * ThingMagicBlockPermalock result.
     **/
    case TMR_LLRP_CUSTOM_BLOCKPERMALOCKOPSPECRESULT:
      {
        LLRP_tSThingMagicBlockPermalockOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicBlockPermalockOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

    /**
     * Higgs2PartialLoadImage result.
     **/
    case TMR_LLRP_CUSTOM_HIGGS2PARTIALLOADIMAGEOPSPECRESULT:
      {
        LLRP_tSThingMagicHiggs2PartialLoadImageOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicHiggs2PartialLoadImageOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

       break;
      }

    /**
     * Higgs2FullLoadImage result
     **/
    case TMR_LLRP_CUSTOM_HIGGS2FULLLOADIMAGEOPSPECRESULT:
      {
        LLRP_tSThingMagicHiggs2FullLoadImageOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicHiggs2FullLoadImageOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

    /**
     * Higgs3FastLoadImage result
     **/
    case TMR_LLRP_CUSTOM_HIGGS3FASTLOADIMAGEOPSPECRESULT:
      {
        LLRP_tSThingMagicHiggs3FastLoadImageOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicHiggs3FastLoadImageOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

    /**
     * Higgs3LoadImage result
     **/
    case TMR_LLRP_CUSTOM_HIGGS3LOADIMAGEOPSPECRESULT:
      {
        LLRP_tSThingMagicHiggs3LoadImageOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicHiggs3LoadImageOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

    /**
     * Higgs3BlockReadLock result
     **/
    case TMR_LLRP_CUSTOM_HIGGS3BLOCKREADLOCKOPSPECRESULT:
      {
        LLRP_tSThingMagicHiggs3BlockReadLockOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicHiggs3BlockReadLockOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

    /**
     * G2ISetReadProtect result
     **/
    case TMR_LLRP_CUSTOM_G2ISETREADPROTECTOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPG2ISetReadProtectOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicNXPG2ISetReadProtectOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }
 
    /**
     * G2XSetReadProtect result
     **/
    case TMR_LLRP_CUSTOM_G2XSETREADPROTECTOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPG2XSetReadProtectOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicNXPG2XSetReadProtectOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }
 
    /**
     * G2IResetReadProtect result
     **/
    case TMR_LLRP_CUSTOM_G2IRESETREADPROTECTOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPG2IResetReadProtectOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicNXPG2IResetReadProtectOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }
 
    /**
     * G2XResetReadProtect result
     **/
    case TMR_LLRP_CUSTOM_G2XRESETREADPROTECTOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPG2XResetReadProtectOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicNXPG2XResetReadProtectOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }
 
    /**
     * G2IChangeEAS result
     **/
    case TMR_LLRP_CUSTOM_G2ICHANGEEASOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPG2IChangeEASOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicNXPG2IChangeEASOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }
 
    /**
     * G2XChangeEAS result
     **/
    case TMR_LLRP_CUSTOM_G2XCHANGEEASOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPG2XChangeEASOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicNXPG2XChangeEASOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }
 
    /**
     * G2IEASAlarm result
     **/
    case TMR_LLRP_CUSTOM_G2IEASALARMOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPG2IEASAlarmOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicNXPG2IEASAlarmOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }
 
    /**
     * G2XEASAlarm result
     **/
    case TMR_LLRP_CUSTOM_G2XEASALARMOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPG2XEASAlarmOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicNXPG2XEASAlarmOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }
 
    /**
     * G2ICalibrate result
     **/
    case TMR_LLRP_CUSTOM_G2ICALIBRATEOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPG2ICalibrateOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicNXPG2ICalibrateOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }
 
    /**
     * G2XCalibrate result
     **/
    case TMR_LLRP_CUSTOM_G2XCALIBRATEOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPG2XCalibrateOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicNXPG2XCalibrateOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

    /**
     * G2IChangeConfig result
     **/
    case TMR_LLRP_CUSTOM_G2ICHANGECONFIGOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPG2IChangeConfigOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicNXPG2IChangeConfigOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }
 
    /**
     * Monza4QTReadWrite result
     **/
    case TMR_LLRP_CUSTOM_MONZA4QTREADWRITEOPSPECRESULT:
      {
        LLRP_tSThingMagicImpinjMonza4QTReadWriteOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicImpinjMonza4QTReadWriteOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

     /**
      * ThingMagicSetSfeParameters Result
      */
    case TMR_LLRP_CUSTOM_IDS_SETSFEPARAMETERSOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900ASetSFEParamsOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicIDSSL900ASetSFEParamsOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

    /**
     * ThingMagicGetMeasurementSetup result
     */
    case TMR_LLRP_CUSTOM_IDS_GETMEASUREMENTSETUPOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900AGetMeasurementSetupOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicIDSSL900AGetMeasurementSetupOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

    /**
     * ThingMagicGetBatteryLevel result
     */
    case TMR_LLRP_CUSTOM_IDS_GETBATTERYLEVELOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900AGetBatteryLevelOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicIDSSL900AGetBatteryLevelOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

    /**
     * ThingMagicSetLogLimits result
     */
    case TMR_LLRP_CUSTOM_IDS_SETLOGLIMITSOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900ASetLogLimitsOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicIDSSL900ASetLogLimitsOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

    /**
     * ThingMagicSetShelfLife result
     */
    case TMR_LLRP_CUSTOM_IDS_SETSHELFLIFEOPSPECRESULT:
    {
    	LLRP_tSThingMagicIDSSetShelfLifeOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicIDSSetShelfLifeOpSpecResult *)pParameter;
      ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

      break;
    }

    /**
     * ThingMagicSetPassword result
     */
    case TMR_LLRP_CUSTOM_IDS_SETPASSWORDOPSPECRESULT:
    {
      LLRP_tSThingMagicIDSSL900ASetPasswordOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicIDSSL900ASetPasswordOpSpecResult *)pParameter;
      ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

      break;
    }

     /**
      * ThingMagicWriteTag result
      **/
    case TMR_LLRP_CUSTOM_WRITETAGOPSPECRESULT:
      {
        LLRP_tSThingMagicWriteTagOpSpecResult  *pResult;
        pResult = (LLRP_tSThingMagicWriteTagOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

      /**
       * ThingMagicIDSGetSensor result
       */
    case TMR_LLRP_CUSTOM_IDS_GETSENSORVALUEOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900ASensorValueOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicIDSSL900ASensorValueOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

      /**
       * ThingMagicIDSSetLogMode result
       */
    case TMR_LLRP_CUSTOM_IDS_SETLOGMODEOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900ASetLogModeOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicIDSSL900ASetLogModeOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

      /**
       * ThingMagicIDSStartLog result
       */
    case TMR_LLRP_CUSTOM_IDS_STARTLOGMODEOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900AStartLogOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicIDSSL900AStartLogOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

      /**
       * ThingMagicIDSGetLogState result
       */
    case TMR_LLRP_CUSTOM_IDS_GETLOGSTATEOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900ALogStateOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicIDSSL900ALogStateOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

      /**
       * ThingMagicIDSEndLog result
       */
    case TMR_LLRP_CUSTOM_IDS_ENDLOGOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900AEndLogOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicIDSSL900AEndLogOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

      /**
       * ThingMagicIDSInitialize result
       */
    case TMR_LLRP_CUSTOM_IDS_INITIALIZEOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900AInitializeOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicIDSSL900AInitializeOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

      /**
       * ThingMagicIDSAccessFifoStatus result
       */
    case TMR_LLRP_CUSTOM_IDS_ACCESSFIFOSTATUSOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900AAccessFIFOStatusOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicIDSSL900AAccessFIFOStatusOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

      /**
       * ThingMagicAccessFifoWrite result
       */
    case TMR_LLRP_CUSTOM_IDS_ACCESSFIFOWRITEOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900AAccessFIFOWriteOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicIDSSL900AAccessFIFOWriteOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

      /**
       * ThingMagicAccessFifoRead result
       */
    case TMR_LLRP_CUSTOM_IDS_ACCESSFIFOREADOPSPECRESULT:
      {
        LLRP_tSThingMagicIDSSL900AAccessFIFOReadOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicIDSSL900AAccessFIFOReadOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

      /**
       * ThingMagicGetCalibrationData result
       */
    case TMR_LLRP_CUSTOM_IDS_GETCALIBRATIONDATAOPSPECRESULT:
    {
    	LLRP_tSThingMagicIDSSL900AGetCalibrationDataOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicIDSSL900AGetCalibrationDataOpSpecResult *)pParameter;
      ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

      break;
    }

    /**
     * ThingMagicSetCalibrationData result
     */
    case TMR_LLRP_CUSTOM_IDS_SETCALIBRATIONDATAOPSPECRESULT:
    {
    	LLRP_tSThingMagicIDSSL900ASetCalibrationDataOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicIDSSL900ASetCalibrationDataOpSpecResult *)pParameter;
      ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

      break;
    }

    /**
     * ThingMagic DenatranIAV ActivateSecureMode result
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_ACTIVATESECUREMODEOPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVActivateSecureModeOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicDenatranIAVActivateSecureModeOpSpecResult *)pParameter;
      ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

      break;
    }

    /**
     * ThingMagic DenatranIAV AuthenticateOBU result
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_AUTHENTICATEOBUOPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVAuthenticateOBUOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicDenatranIAVAuthenticateOBUOpSpecResult *)pParameter;
      ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

      break;
    }

    /**
     * ThingMagic DenatranIAV ActivateSiniavMode result
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_ACTIVATESINIAVMODEOPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVActivateSiniavModeOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicDenatranIAVActivateSiniavModeOpSpecResult *)pParameter;
      ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

      break;
    }

    /**
     * ThingMagic DenatranIAV OBUAuthenticateID result
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_AUTHENTICATEIDOPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVOBUAuthenticateIDOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicDenatranIAVOBUAuthenticateIDOpSpecResult *)pParameter;
      ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

      break;
    }

    /**
     * ThingMagic DenatranIAV AuthenticateOBU FullPass1 result
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_AUTHENTICATEFULLPASS1OPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVOBUAuthenticateFullPass1OpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicDenatranIAVOBUAuthenticateFullPass1OpSpecResult *)pParameter;
      ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

      break;
    }

    /**
     * ThingMagic DenatranIAV AuthenticateOBU FullPass2 result
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_AUTHENTICATEFULLPASS2OPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVOBUAuthenticateFullPass2OpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicDenatranIAVOBUAuthenticateFullPass2OpSpecResult *)pParameter;
      ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

      break;
    }

    /**
     * ThingMagic DenatranIAV OBU ReadFromMEM result
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_OBUREADFROMMEMMAPOPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVOBUReadFromMemMapOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicDenatranIAVOBUReadFromMemMapOpSpecResult *)pParameter;
      ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

      break;
    }

    /**
     * ThingMagic DenatranIAV OBU WriteToMem result
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_OBUWRITETOMEMMAPOPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVOBUWriteToMemMapOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicDenatranIAVOBUWriteToMemMapOpSpecResult *)pParameter;
      ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

      break;
    }

    /**
     * ThingMagic DenatranIAV GetTokenID result
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_GETTOKENIDOPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVGetTokenIDOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicDenatranIAVGetTokenIDOpSpecResult *)pParameter;
      ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

      break;
    }

    /**
     * ThingMagic DenatranIAV OBUAuthenticateFullPass result
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_OBUAUTHFULLPASSOPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVOBUAuthFullPassOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicDenatranIAVOBUAuthFullPassOpSpecResult *)pParameter;
      ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

      break;
    }

    /**
     * ThingMagic DenatranIAV ReadSec result
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_READSECTOROPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVReadSecOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicDenatranIAVReadSecOpSpecResult *)pParameter;
      ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

      break;
    }

    /**
     * ThingMagic DenatranIAV WriteSec result
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_WRITESECTOROPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVWriteSecOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicDenatranIAVWriteSecOpSpecResult *)pParameter;
      ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

      break;
    }

    /**
     * ThingMagic DenatranIAV G0PAOBUAuth result
     */
    case TMR_LLRP_CUSTOM_DENATRAN_IAV_G0PAOBUAUTHOPSPECRESULT:
    {
      LLRP_tSThingMagicDenatranIAVG0PAOBUAuthOpSpecResult *pResult;
      pResult = (LLRP_tSThingMagicDenatranIAVG0PAOBUAuthOpSpecResult *)pParameter;
      ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

      break;
    }


#ifdef TMR_ENABLE_ISO180006B
    case TMR_LLRP_CUSTOM_ISO_READDATAOPSPECRESULT:
      {
        LLRP_tSThingMagicISO180006BReadOpSpecResult  *pResult;
        pResult = (LLRP_tSThingMagicISO180006BReadOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }

    case TMR_LLRP_CUSTOM_ISO_WRITEDATAOPSPECRESULT:
      {
        LLRP_tSThingMagicISO180006BWriteOpSpecResult  *pResult;
        pResult = (LLRP_tSThingMagicISO180006BWriteOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }
    case TMR_LLRP_CUSTOM_ISO_LOCKOPSPECRESULT:
      {
        LLRP_tSThingMagicISO180006BLockOpSpecResult  *pResult;
        pResult = (LLRP_tSThingMagicISO180006BLockOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);

        break;
      }
#endif /* TMR_ENABLE_ISO180006B */

      /**
       * ThingMagicNXPAuthentication result
       */
    case TMR_LLRP_CUSTOM_NXP_AUTHENTICATEOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPAuthenticationOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicNXPAuthenticationOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);
        break;
      }

      /**
       * ThingMagicNXPReadbuffer result
       */
    case TMR_LLRP_CUSTOM_NXP_READBUFFEROPSPECRESULT:
      {
        LLRP_tSThingMagicNXPReadbufferOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicNXPReadbufferOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);
        break;
      }

      /**
       * ThingMagicNXPUntraceable result
       */
    case TMR_LLRP_CUSTOM_NXP_UNTRACEABLEOPSPECRESULT:
      {
        LLRP_tSThingMagicNXPUntraceableOpSpecResult *pResult;
        pResult = (LLRP_tSThingMagicNXPUntraceableOpSpecResult *)pParameter;
        ret = TMR_LLRP_parseCustomTagOpSpecResultType(pResult->eResult);
        break;
      }

    default:
      {
        /**
         * I do not know what kind of OpSpecResult is this.
         * So returning msg parse error.
         **/
        ret = TMR_ERROR_LLRP_MSG_PARSE_ERROR;
      }
  }

  return ret;
}

/**
 * Command to delete all AccessSpecs on LLRP Reader
 *
 * @param reader Reader pointer
 */
TMR_Status
TMR_LLRP_cmdDeleteAllAccessSpecs(TMR_Reader *reader)
{
  TMR_Status ret;
  LLRP_tSDELETE_ACCESSSPEC          *pCmd;
  LLRP_tSMessage                    *pCmdMsg;
  LLRP_tSMessage                    *pRspMsg;
  LLRP_tSDELETE_ACCESSSPEC_RESPONSE *pRsp;

  ret = TMR_SUCCESS;

  /**
   * Create delete accessspec message
   **/
  pCmd = LLRP_DELETE_ACCESSSPEC_construct();
  LLRP_DELETE_ACCESSSPEC_setAccessSpecID(pCmd, 0);        /* All */

  pCmdMsg = &pCmd->hdr;
  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSDELETE_ACCESSSPEC_RESPONSE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Response is success, Done with the response message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}

TMR_Status
TMR_LLRP_cmdStopReading(struct TMR_Reader *reader)
{
  if (TMR_READ_PLAN_TYPE_SIMPLE == reader->readParams.readPlan->type)
  {
    /* receiveResponse = false, as we do not need here */
    return TMR_LLRP_cmdStopROSpec(reader, false);
  }
  else
  {
    /**
     * In case of multiple readplans, sending STOP_ROSPEC doesn't
     * actually stop the rospec execution, since their start trigger
     * was set to periodic based. So delete all rospecs.
     **/
    return TMR_LLRP_cmdDeleteAllROSpecs(reader, false);
  }
}

TMR_Status
TMR_LLRP_cmdGetReport(TMR_Reader *reader)
{
  TMR_Status ret;
  LLRP_tSGET_REPORT *pCmd;
  LLRP_tSMessage    *pCmdMsg;

  ret = TMR_SUCCESS;

  /**
   * Initialize GET_REPORT message
   **/
  pCmd = LLRP_GET_REPORT_construct();
  pCmdMsg = &pCmd->hdr;
 
  /**
   * Response to GET_REPORT message will be RO_ACCESS_REPORTs,
   * which needs to be processed in other place.
   * Here we just send the message.
   **/
  ret = TMR_LLRP_sendMessage(reader, pCmdMsg, 
          reader->u.llrpReader.transportTimeout);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  return ret;
}

TMR_Status
TMR_LLRP_cmdrebootReader(TMR_Reader *reader)
{
  TMR_Status ret;
  LLRP_tSTHINGMAGIC_CONTROL_REQUEST_POWER_CYCLE_READER  *pCmd;
  LLRP_tSMessage                                        *pCmdMsg;
  LLRP_tSMessage                                        *pRspMsg;
  LLRP_tSTHINGMAGIC_CONTROL_RESPONSE_POWER_CYCLE_READER *pRsp;

  ret = TMR_SUCCESS;

  /**
   * Initialize Custom message THINGMAGIC_CONTROL_REQUEST_POWER_CYCLE_READER
   **/
  pCmd = LLRP_THINGMAGIC_CONTROL_REQUEST_POWER_CYCLE_READER_construct();

  /* Add the thingmagic magic number to recycle the reader */
  LLRP_THINGMAGIC_CONTROL_REQUEST_POWER_CYCLE_READER_setMagicNumber(pCmd, TMR_POWER_CYCLE_MAGIC_NUMBER);

  /* Add the safe mode option */
  LLRP_THINGMAGIC_CONTROL_REQUEST_POWER_CYCLE_READER_setBootToSafeMode(pCmd, false);

  pCmdMsg = &pCmd->hdr;

  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSTHINGMAGIC_CONTROL_RESPONSE_POWER_CYCLE_READER *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  /**
   * Wait for the reader to  reboot. 90s is enough.
   */
  tmr_sleep(90000);

  return ret;
}

#if 0
TMR_Status
TMR_LLRP_cmdResetHoptable(TMR_Reader *reader)
{
  TMR_Status ret;
  LLRP_tSTHINGMAGIC_CONTROL_REQUEST_RESET_HOPTABLE  *pCmd;
  LLRP_tSMessage                                    *pCmdMsg;
  LLRP_tSMessage                                    *pRspMsg;
  LLRP_tSTHINGMAGIC_CONTROL_RESPONSE_RESET_HOPTABLE *pRsp;

  ret = TMR_SUCCESS;

  /**
   * Initialize Custom message THINGMAGIC_CONTROL_REQUEST_RESET_HOPTABLE
   **/
  pCmd = LLRP_THINGMAGIC_CONTROL_REQUEST_RESET_HOPTABLE_construct();

  pCmdMsg = &pCmd->hdr;

  /**
   * Now the message is framed completely and send the message
   **/
  ret = TMR_LLRP_send(reader, pCmdMsg, &pRspMsg);
  /**
   * Done with the command, free the message
   * and check for message status
   **/ 
  TMR_LLRP_freeMessage((LLRP_tSMessage *)pCmd);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Check response message status
   **/
  pRsp = (LLRP_tSTHINGMAGIC_CONTROL_RESPONSE_RESET_HOPTABLE *) pRspMsg;
  if (TMR_SUCCESS != TMR_LLRP_checkLLRPStatus(pRsp->pLLRPStatus))  
  {
    TMR_LLRP_freeMessage(pRspMsg);
    return TMR_ERROR_LLRP; 
  }

  /**
   * Done with the response, free the message
   **/
  TMR_LLRP_freeMessage(pRspMsg);

  return ret;
}
#endif
#endif  /* TMR_ENABLE_LLRP_READER */

