/**
 *  @file llrp_reader.c
 *  @brief Mercury API - llrp reader high level implementation
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

#include "tm_reader.h"
#ifdef TMR_ENABLE_LLRP_READER

#include "tmr_utils.h"
#include "osdep.h"
#include "tmr_llrp_reader.h"
#include "llrp_reader_imp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(WIN32)|| defined(WINCE)  /* WIN32 or WINCE */
#include <ws2tcpip.h>
#else                                /* linux */
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif                               /* WIN32 or WINCE */

#define MAX_KEEP_ALIVE_ACK_MISSES 3
#define MAX_SUB_PLAN_COUNT 5    /*For TMreader build <5.3.2.93*/

extern uint8_t TMR_LLRP_gpiListSargas[];
extern uint8_t TMR_LLRP_gpoListSargas[];
extern uint8_t TMR_LLRP_gpiListIzar[];
extern uint8_t TMR_LLRP_gpoListIzar[];
extern uint8_t sizeGpiListSargas;
extern uint8_t sizeGpoListSargas;
extern uint8_t sizeGpiListIzar;
extern uint8_t sizeGpoListIzar;
extern uint8_t TMR_LLRP_gpiListM6Astra[];
extern uint8_t TMR_LLRP_gpoListM6Astra[];
extern uint8_t sizeGpiListM6Astra;
extern uint8_t sizeGpoListM6Astra;
extern bool isPerAntennaEnabled;
extern bool isStandaloneTagop;
extern uint32_t currentInventorySpecID;

static TMR_Status
TMR_LLRP_initTxRxMapFromPorts(TMR_Reader *reader)
{
  TMR_Status ret;
  TMR_LLRP_PortDetect ports[TMR_SR_MAX_ANTENNA_PORTS];
  uint8_t numPorts, i;
  TMR_LLRP_LlrpReader *lr;

  if (NULL == reader)
  {
    return TMR_ERROR_INVALID;
  }
  numPorts = numberof(ports);
  lr = &reader->u.llrpReader;

  /* Need number of ports to set up Tx-Rx map */
  ret = TMR_LLRP_cmdAntennaDetect(reader, &numPorts, ports);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  lr->portMask = 0;
  for (i = 0; i < numPorts; i ++)
  {
    lr->portMask |= 1 << (ports[i].port - 1);
    lr->staticTxRxMapData[i].antenna = (uint8_t)ports[i].port;
    lr->staticTxRxMapData[i].txPort  = (uint8_t)ports[i].port;
    lr->staticTxRxMapData[i].rxPort  = (uint8_t)ports[i].port;

    if (0 == reader->tagOpParams.antenna && ports[i].connected)
    {
      reader->tagOpParams.antenna = (uint8_t)ports[i].port;
    }
  }

  lr->staticTxRxMap.max = TMR_SR_MAX_ANTENNA_PORTS;
  lr->staticTxRxMap.len = numPorts;
  lr->staticTxRxMap.list = lr->staticTxRxMapData;
  lr->txRxMap = &lr->staticTxRxMap;

  return TMR_SUCCESS;
}


static TMR_Status
TMR_LLRP_boot(TMR_Reader *reader)
{
  TMR_Status ret;
  TMR_LLRP_LlrpReader *lr;

  int i;

  if (NULL == reader)
  {
    return TMR_ERROR_INVALID;
  }
  ret = TMR_SUCCESS;
  lr = &reader->u.llrpReader;

  if (TMR_LLRP_READER_DEFAULT_PORT == reader->u.llrpReader.portNum)
  {
  /**
     * Default port is used for data operations. If the connection
     * is being made on default data port only then reset the reader.
     **/
    
    /**
     * Set HoldEventsAndReports to true on connect.  
     **/
    ret = TMR_LLRP_setHoldEventsAndReportsStatus(reader, 1);
    if (TMR_SUCCESS != ret)
    {
      /**
       * Not Fatal, moving forward
       **/ 
    }

    /**
     * Stop any active ROSpecs running on reader
     **/
    ret = TMR_LLRP_stopActiveROSpecs(reader);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
  }

  /**
   * Get current region and cache it
   **/
  ret = TMR_LLRP_cmdGetRegion(reader, &reader->u.llrpReader.regionId);
  if (TMR_SUCCESS != ret)
  {
    /**
     * Not Fatal, moving forward
     * value might be changed, restore the dafult value.
     **/
    reader->u.llrpReader.regionId = TMR_REGION_NA;
  }

  /**
   * Get reader capabilities and cache it
   **/
  ret = TMR_LLRP_cmdGetReaderCapabilities(reader, &reader->u.llrpReader.capabilities);
  if (TMR_SUCCESS != ret)
  {
    uint8_t length = (uint8_t)strlen("NOT AVAILABLE");
    /**
     * Not Fatal, moving forward
     * Value might be changed, restore the default value.
     **/
    memcpy(reader->u.llrpReader.capabilities.softwareVersion, "NOT AVAILABLE", (size_t)length);
    reader->u.llrpReader.capabilities.softwareVersion[length + 1] = '\0';
    reader->u.llrpReader.capabilities.model = 0;
    reader->u.llrpReader.capabilities.powerTable.list = NULL;
    reader->u.llrpReader.capabilities.powerTable.len = 0;
    reader->u.llrpReader.capabilities.freqTable.list = NULL;
    reader->u.llrpReader.capabilities.freqTable.len = 0;
  }

  /* Initialize the paramPresent and paramConfirmed bits. */
  /* This block is expected to be collapsed by the compiler into a
   * small number of constant-value writes into the lr->paramPresent
   * array.
   */
  for (i = 0 ; i < TMR_PARAMWORDS; i++)
  {
    lr->paramPresent[i] = 0;
  }

  BITSET(lr->paramPresent, TMR_PARAM_REGION_ID);
  BITSET(lr->paramPresent, TMR_PARAM_ANTENNA_PORTLIST);
  BITSET(lr->paramPresent, TMR_PARAM_ANTENNA_CONNECTEDPORTLIST);
  BITSET(lr->paramPresent, TMR_PARAM_RADIO_POWERMAX);
  BITSET(lr->paramPresent, TMR_PARAM_RADIO_POWERMIN);
  BITSET(lr->paramPresent, TMR_PARAM_RADIO_PORTREADPOWERLIST);
  BITSET(lr->paramPresent, TMR_PARAM_RADIO_PORTWRITEPOWERLIST);
  BITSET(lr->paramPresent, TMR_PARAM_READ_PLAN);
  BITSET(lr->paramPresent, TMR_PARAM_URI);
  BITSET(lr->paramPresent, TMR_PARAM_TRANSPORTTIMEOUT);
  BITSET(lr->paramPresent, TMR_PARAM_COMMANDTIMEOUT);
  BITSET(lr->paramPresent, TMR_PARAM_GPIO_INPUTLIST);
  BITSET(lr->paramPresent, TMR_PARAM_GPIO_OUTPUTLIST);
  BITSET(lr->paramPresent, TMR_PARAM_GEN2_ACCESSPASSWORD);
  BITSET(lr->paramPresent, TMR_PARAM_VERSION_HARDWARE);
  BITSET(lr->paramPresent, TMR_PARAM_VERSION_MODEL);
  BITSET(lr->paramPresent, TMR_PARAM_VERSION_SERIAL);
  BITSET(lr->paramPresent, TMR_PARAM_VERSION_SOFTWARE);
  BITSET(lr->paramPresent, TMR_PARAM_TAGREADDATA_RECORDHIGHESTRSSI);
  BITSET(lr->paramPresent, TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA);
  BITSET(lr->paramPresent, TMR_PARAM_TAGREADDATA_UNIQUEBYDATA);
  BITSET(lr->paramPresent, TMR_PARAM_READER_DESCRIPTION);
  BITSET(lr->paramPresent, TMR_PARAM_READER_HOSTNAME);
  BITSET(lr->paramPresent, TMR_PARAM_CURRENTTIME);
  BITSET(lr->paramPresent, TMR_PARAM_RADIO_TEMPERATURE);
  BITSET(lr->paramPresent, TMR_PARAM_GEN2_BLF);
  BITSET(lr->paramPresent, TMR_PARAM_GEN2_TAGENCODING);
  BITSET(lr->paramPresent, TMR_PARAM_GEN2_TARI);
  BITSET(lr->paramPresent, TMR_PARAM_GEN2_Q);
  BITSET(lr->paramPresent, TMR_PARAM_GEN2_SEND_SELECT);
  BITSET(lr->paramPresent, TMR_PARAM_GEN2_INITIAL_Q);
  BITSET(lr->paramPresent, TMR_PARAM_RADIO_READPOWER);
  BITSET(lr->paramPresent, TMR_PARAM_RADIO_WRITEPOWER);
  BITSET(lr->paramPresent, TMR_PARAM_VERSION_SUPPORTEDPROTOCOLS);
  BITSET(lr->paramPresent, TMR_PARAM_ANTENNA_CHECKPORT);
  BITSET(lr->paramPresent, TMR_PARAM_GEN2_SESSION);
  BITSET(lr->paramPresent, TMR_PARAM_GEN2_TARGET);
  BITSET(lr->paramPresent, TMR_PARAM_LICENSE_KEY);
  BITSET(lr->paramPresent, TMR_PARAM_MANAGE_LICENSE_KEY);
  BITSET(lr->paramPresent, TMR_PARAM_TAGOP_ANTENNA);
  BITSET(lr->paramPresent, TMR_PARAM_TAGOP_PROTOCOL);
  BITSET(lr->paramPresent, TMR_PARAM_ISO180006B_DELIMITER);
  BITSET(lr->paramPresent, TMR_PARAM_ISO180006B_MODULATION_DEPTH);
  BITSET(lr->paramPresent, TMR_PARAM_ISO180006B_BLF);
  BITSET(lr->paramPresent, TMR_PARAM_REGION_SUPPORTEDREGIONS);
  BITSET(lr->paramPresent, TMR_PARAM_READ_ASYNCOFFTIME);
  BITSET(lr->paramPresent, TMR_PARAM_LICENSED_FEATURES);
  BITSET(lr->paramPresent, TMR_PARAM_SELECTED_PROTOCOLS);
  BITSET(lr->paramPresent, TMR_PARAM_GEN2_PROTOCOLEXTENSION);
  BITSET(lr->paramPresent, TMR_PARAM_GEN2_T4);
  BITSET(lr->paramPresent, TMR_PARAM_PRODUCT_ID);
  BITSET(lr->paramPresent, TMR_PARAM_PRODUCT_GROUP_ID);
  BITSET(lr->paramPresent, TMR_PARAM_PRODUCT_GROUP);
  BITSET(lr->paramPresent, TMR_PARAM_REGULATORY_MODE);
  BITSET(lr->paramPresent, TMR_PARAM_REGULATORY_MODULATION);
  BITSET(lr->paramPresent, TMR_PARAM_REGULATORY_ONTIME);
  BITSET(lr->paramPresent, TMR_PARAM_REGULATORY_OFFTIME);
  BITSET(lr->paramPresent, TMR_PARAM_REGULATORY_ENABLE);
  BITSET(lr->paramPresent, TMR_PARAM_REGION_HOPTABLE);
  BITSET(lr->paramPresent, TMR_PARAM_ANTENNA_PORTSWITCHGPOS);
  BITSET(lr->paramPresent, TMR_PARAM_READ_ASYNCONTIME);
  BITSET(lr->paramPresent, TMR_PARAM_ANTENNA_RETURNLOSS);
  BITSET(lr->paramPresent, TMR_PARAM_METADATAFLAG);
  BITSET(lr->paramPresent, TMR_PARAM_READER_STATS_ENABLE);
  BITSET(lr->paramPresent, TMR_PARAM_READER_STATS);
 
  for (i = 0; i < TMR_PARAMWORDS; i++)
  {
    lr->paramConfirmed[i] = lr->paramPresent[i];
  }

  /**
   * Right now, only Gen2 protocol is supported.
   * TODO: create a proper bitmask of supportedProtocols
   * by getting list of available protocols through /reader/supportedProtocols.
   **/
  {
  TMR_TagProtocol protocol[5];
   TMR_TagProtocolList protocolList;
   protocolList.list = protocol;
   ret = TMR_LLRP_cmdGetTMDeviceProtocolCapabilities(reader, &protocolList);
   if (TMR_SUCCESS != ret)
   {
  /**
      * Not Fatal, moving forward
      * value might be changed, restore the dafult value.
      **/
     lr->supportedProtocols = (1 << (TMR_TAG_PROTOCOL_GEN2 - 1));
   }
  }
  /**
   * Initialize txrxmap
   **/
  ret = TMR_LLRP_initTxRxMapFromPorts(reader);
  if (TMR_SUCCESS != ret)
  {
    /**
     * Not Fatal, moving forward
     **/
  }

  if (TMR_LLRP_READER_DEFAULT_PORT == reader->u.llrpReader.portNum)
  {
    /**
     * Keep alives are needed only if the connection is made on default
     * data port. Keep alives are not supported on other ports from reader end.
     * Set Keep alive messages to pulse reader heart beat
     **/
    ret = TMR_LLRP_setKeepAlive(reader);
    if (TMR_SUCCESS != ret)
    {
      /**
       * Not Fatal, moving forward
       **/
    }
  }

  return ret;
}

static void
TMR_LLRP_paramProbe(struct TMR_Reader *reader, TMR_Param key)
{
  TMR_LLRP_LlrpReader *lr;
 /* buf is at least as large as the largest parameter, with all values 0
  * (NULL pointers and 0 lengths).
  */
  uint32_t buf[] = {0, 0, 0, 0, 0, 0, 0, 0};
  TMR_Status ret;

  if (NULL == reader)
  {
    return;
  }
  lr = &reader->u.llrpReader;

  ret = TMR_paramGet(reader, key, &buf);
  if (TMR_SUCCESS == ret)
  {
    BITSET(lr->paramPresent, key);
  }
  BITSET(lr->paramConfirmed, key);
}

static TMR_Status
TMR_LLRP_paramSet(struct TMR_Reader *reader, TMR_Param key, const void *value)
{
  TMR_Status ret;
  TMR_LLRP_LlrpReader *lr;

  if (NULL == reader)
  {
    return TMR_ERROR_INVALID;
  }
  ret = TMR_SUCCESS;
  lr = &reader->u.llrpReader;

  if (0 == BITGET(lr->paramConfirmed, key))
  {
    TMR_LLRP_paramProbe(reader, key);
  }

  if (BITGET(lr->paramConfirmed, key) && (0 == BITGET(lr->paramPresent, key)))
  {
    return TMR_ERROR_NOT_FOUND;
  }

  if (NULL == value)
  {
    return TMR_ERROR_INVALID;
  }

  switch (key)
  {
    case TMR_PARAM_REGION_ID:
    case TMR_PARAM_REGION_SUPPORTEDREGIONS:
      {
        ret = TMR_ERROR_READONLY;
        break;
      }
   
    case TMR_PARAM_ANTENNA_PORTLIST:
    case TMR_PARAM_URI:
    case TMR_PARAM_ANTENNA_CONNECTEDPORTLIST:
    case TMR_PARAM_RADIO_POWERMAX:
    case TMR_PARAM_RADIO_POWERMIN:
    case TMR_PARAM_GPIO_INPUTLIST:
    case TMR_PARAM_GPIO_OUTPUTLIST:
    case TMR_PARAM_VERSION_MODEL:
    case TMR_PARAM_VERSION_SERIAL:
    case TMR_PARAM_VERSION_SOFTWARE:
    case TMR_PARAM_VERSION_HARDWARE:
    case TMR_PARAM_CURRENTTIME:
    case TMR_PARAM_RADIO_TEMPERATURE:
    case TMR_PARAM_VERSION_SUPPORTEDPROTOCOLS:
    case TMR_PARAM_ANTENNA_RETURNLOSS:
      {
        ret = TMR_ERROR_READONLY;
        break;
      }

    case TMR_PARAM_RADIO_PORTREADPOWERLIST:
      {
        TMR_PortValueList *list = (TMR_PortValueList *)value;

        if (NULL == list)
        {
          ret = TMR_ERROR_ILLEGAL_VALUE;
          break;
        }

        if (0 >= list->len)
        {
          ret = TMR_ERROR_ILLEGAL_VALUE;
          break;
        }

        ret = TMR_LLRP_cmdSetReadTransmitPowerList(reader, list);
        break;
      }

    case TMR_PARAM_RADIO_PORTWRITEPOWERLIST:
      {
        TMR_PortValueList *list = (TMR_PortValueList *)value;

        if (NULL == list)
        {
          ret = TMR_ERROR_ILLEGAL_VALUE;
          break;
        }

        if (0 >= list->len)
        {
          ret = TMR_ERROR_ILLEGAL_VALUE;
          break;
        }

        ret = TMR_LLRP_cmdSetWriteTransmitPowerList(reader, list);
        break;
      }

    case TMR_PARAM_READ_PLAN:
      {
        const TMR_ReadPlan *plan;
        TMR_ReadPlan tmpPlan;

        plan = value;
        tmpPlan = *plan;

        ret = validateReadPlan(reader, &tmpPlan, 
                        lr->txRxMap, lr->supportedProtocols);
        if (TMR_SUCCESS != ret)
        {
          return ret;
        }

        *reader->readParams.readPlan = tmpPlan;
        break;
      }

    case TMR_PARAM_COMMANDTIMEOUT:
      {
        uint32_t val = *(uint32_t *)value;
        if ((1<<31) & val)
        {
          ret = TMR_ERROR_ILLEGAL_VALUE;
        }
        else
        {
          lr->commandTimeout = *(uint32_t *)value;
        }
        break;
      }

    case TMR_PARAM_TRANSPORTTIMEOUT:
      {
        uint32_t val = *(uint32_t *)value;
        if ((1<<31) & val)
        {
          ret = TMR_ERROR_ILLEGAL_VALUE;
        }
        else
        {
          lr->transportTimeout = *(uint32_t *)value;
        }
        break;
      }

    case TMR_PARAM_GEN2_ACCESSPASSWORD:
      {
        lr->gen2AccessPassword = *(TMR_GEN2_Password *)value;
        break;
      }

    case TMR_PARAM_TAGREADDATA_RECORDHIGHESTRSSI:
    case TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA:
    case TMR_PARAM_TAGREADDATA_UNIQUEBYDATA:
      {
        TMR_LLRP_TMDeDuplication duplication;

        if (NULL == value)
        {
          return TMR_ERROR_ILLEGAL_VALUE;
        }

        /* Get ThingMagic DeDuplication before setting it */
        ret = TMR_LLRP_cmdGetThingMagicDeDuplication(reader, &duplication);

        if(TMR_SUCCESS != ret)
        {
          break;
        }

        /* Set the parameter asked by the user */
        switch(key)
        {
          case TMR_PARAM_TAGREADDATA_RECORDHIGHESTRSSI:
            {
              duplication.highestRSSI = *(bool*)value;
              break;
            }

          case TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA:
            {
              duplication.uniquebyAntenna = *(bool*)value;
              break;
            }

          case TMR_PARAM_TAGREADDATA_UNIQUEBYDATA:
            {
              duplication.uniquebyData = *(bool*)value;
              break;
            }

          default:
            ret = TMR_ERROR_NOT_FOUND;
        }
        /* Set ThingMagic DeDuplication */
        ret = TMR_LLRP_cmdSetThingMagicDeDuplication(reader, &duplication);

        break;
      }
    case TMR_PARAM_READER_DESCRIPTION:
      {
        TMR_String *desc = (TMR_String *)value;
        TMR_LLRP_TMReaderConfiguration config;

        if (NULL == value)
        {
          ret = TMR_ERROR_INVALID;
          break;
        }

        if (NULL == desc->value)
        {
          ret = TMR_ERROR_INVALID;
          break;
        }

        /* Get Reader Configuration before setting it */
        ret = TMR_LLRP_cmdGetThingmagicReaderConfiguration(reader, &config);
        if (TMR_SUCCESS != ret)
        {
          break;
        }

        /* Set the description asked by user */
        LLRP_utf8v_clear(&config.description);
        config.description = LLRP_utf8v_construct((llrp_u16_t)strlen(desc->value));
        strcpy((char *)config.description.pValue, desc->value);
        
        /* Set Reader Configuration */
        ret = TMR_LLRP_cmdSetThingmagicReaderConfiguration(reader, &config);

        TMR_LLRP_freeTMReaderConfiguration(&config);
        break;
      }

    case TMR_PARAM_READER_HOSTNAME:
      {
        TMR_String *hostname = (TMR_String *)value;
        TMR_LLRP_TMReaderConfiguration config;

        if (NULL == value)
        {
          ret = TMR_ERROR_INVALID;
          break;
        }

        if (NULL == hostname->value)
        {
          ret = TMR_ERROR_INVALID;
          break;
        }

        /* Get Reader Configuration before setting it */
        ret = TMR_LLRP_cmdGetThingmagicReaderConfiguration(reader, &config);
        if (TMR_SUCCESS != ret)
        {
          break;
        }

        /* Set the host name asked by user */
        LLRP_utf8v_clear(&config.hostName);
        config.hostName = LLRP_utf8v_construct((llrp_u16_t)strlen(hostname->value));
        strcpy((char *)config.hostName.pValue, hostname->value);

        /* Set Reader Configuration */
        ret = TMR_LLRP_cmdSetThingmagicReaderConfiguration(reader, &config);

        TMR_LLRP_freeTMReaderConfiguration(&config);
        break;
      }

    case TMR_PARAM_GEN2_BLF:
      {
        TMR_LLRP_RFControl rfControl;
        TMR_GEN2_LinkFrequency setBlf, activeBlf;
        TMR_GEN2_TagEncoding activeM;
        
        ret = TMR_LLRP_cmdGetActiveRFControl(reader, &rfControl);
        if (TMR_SUCCESS != ret)
        {
          break;
        }

        setBlf = *(TMR_GEN2_LinkFrequency *)value;
        activeBlf = lr->capabilities.u.gen2Modes[rfControl.index].blf;
        activeM = lr->capabilities.u.gen2Modes[rfControl.index].m;

        /* Set only if the active BLF value is not same */
        if (setBlf != activeBlf)
        {
          uint8_t i;

          /* RFMode table index starts from 1*/
          for (i = 1; i < TMR_LLRP_MAX_RFMODE_ENTRIES; i++)
          {
            TMR_LLRP_C1G2RFModeTable *entry = &lr->capabilities.u.gen2Modes[i];

            /* Find a suitable mode with the setM and active BLF */
            if ((setBlf == entry->blf) && (activeM == entry->m))
            {
              /* Set mode index */
              rfControl.index = i;

              /* Set this as active RFControl */
              ret = TMR_LLRP_cmdSetActiveRFControl(reader, &rfControl);
              return ret;
            }
          }

          /**
           * If the control comes here, then something went wrong.
           * Could be that the combination of currently active m value
           * and set BLF value doesnt match with any entry in RFMode table.
           * Eg: if m already set to M2, and if user sets BLF value other than
           * 250KHz, then throw error.
           **/
          return TMR_ERROR_LLRP_INVALID_RFMODE;
        }
 
        break;
      }

    case TMR_PARAM_GEN2_TAGENCODING:
      {
        TMR_LLRP_RFControl rfControl;
        TMR_GEN2_TagEncoding setM, activeM;
        TMR_GEN2_LinkFrequency activeBlf;

        ret = TMR_LLRP_cmdGetActiveRFControl(reader, &rfControl);
        if (TMR_SUCCESS != ret)
        {
          break;
        }

        setM = *(TMR_GEN2_TagEncoding *)value;
        activeBlf = lr->capabilities.u.gen2Modes[rfControl.index].blf;
        activeM = lr->capabilities.u.gen2Modes[rfControl.index].m;

        /* Set only if the active m value is not same */
        if (setM != activeM)
        {
          uint8_t i;

          /* RFMode table index starts from 1*/
          for (i = 1; i < TMR_LLRP_MAX_RFMODE_ENTRIES; i++)
          {
            TMR_LLRP_C1G2RFModeTable *entry = &lr->capabilities.u.gen2Modes[i];

            /* Find a suitable mode with the setM and active BLF */
            if ((setM == entry->m) && (activeBlf == entry->blf))
            {
              /* Set mode index */
              rfControl.index = i;

              /* Set this as active RFControl */
              ret = TMR_LLRP_cmdSetActiveRFControl(reader, &rfControl);
              return ret;
            }
          }

          /**
           * If the control comes here, then something went wrong.
           * Could be that the combination of currently active blf value
           * and set M value doesnt match with any entry in RFMode table.
           * Eg: if blf already set to 640, and if user sets M value other
           * than FM0, then throw error
           **/
          return TMR_ERROR_LLRP_INVALID_RFMODE;
        }
        break;
      }

    case TMR_PARAM_GEN2_TARI:
      {
        TMR_LLRP_RFControl rfControl;
        TMR_GEN2_Tari tari = *(TMR_GEN2_Tari *)value;

        /* Get active RFControl */
        ret = TMR_LLRP_cmdGetActiveRFControl(reader, &rfControl);
        if (TMR_SUCCESS != ret)
        {
          break;
        }
        if((tari < lr->capabilities.u.gen2Modes[rfControl.index].minTari) || (tari > lr->capabilities.u.gen2Modes[rfControl.index].maxTari))
        {
          return TMR_ERROR_ILLEGAL_VALUE;  
        }

        /* Set only if the active tari value is not same */
        if (tari != rfControl.tari)
        {
          rfControl.tari = tari;
          ret = TMR_LLRP_cmdSetActiveRFControl(reader, &rfControl);
        }

        break;
      }

    case TMR_PARAM_GEN2_Q:
      {
        TMR_GEN2_Q *q = (TMR_GEN2_Q *)value;

        /* Error check for q value, in case of static type */
        if (TMR_SR_GEN2_Q_STATIC == q->type)
        {
          int qval = (int) q->u.staticQ.initialQ;
          if ((qval < 0) || (qval > 15))
          {
            return TMR_ERROR_ILLEGAL_VALUE;
          }
        }

        ret = TMR_LLRP_cmdSetGen2Q(reader, q);
        break;
      }

    case TMR_PARAM_GEN2_INITIAL_Q:
      {
        TMR_GEN2_initQ *q = (TMR_GEN2_initQ *)value;

        /* Error check for q value, in case of static type */
       {          
          int qval = (int)q->qValue;
          if ((qval < 2) || (qval > 10))
          {
            return TMR_ERROR_ILLEGAL_VALUE;
          }
        }

        ret = TMR_LLRP_cmdSetInitQ(reader, q);
        break;
      }

    case TMR_PARAM_GEN2_SEND_SELECT:
      {
        bool Select;

        Select = *(uint8_t *)value;
        ret = TMR_LLRP_cmdSetsendSelect(reader,&Select);
        break;
      }

    case TMR_PARAM_GEN2_TARGET:
      {
        TMR_GEN2_Target target = *(TMR_GEN2_Target *)value;

        if (NULL == value)
        {
          ret = TMR_ERROR_INVALID;
          break;
        }
        ret = TMR_LLRP_cmdSetGen2Target(reader, &target);
        break;
      }

    case TMR_PARAM_GEN2_T4:
      {
        uint32_t paramValue;

        if (NULL == value)
        {
          ret = TMR_ERROR_INVALID;
          break;
        }
        paramValue = *(uint32_t *)value;
        ret = TMR_LLRP_cmdSetGen2T4Param(reader, paramValue);
        break;
      }

    case TMR_PARAM_GEN2_SESSION:
      {
        TMR_GEN2_Session session = *(TMR_GEN2_Session *)value;

        if (NULL == value)
        {
          ret = TMR_ERROR_INVALID;
          break;
        }
        ret = TMR_LLRP_cmdSetGen2Session(reader, &session);
        break;
      }
            
    case TMR_PARAM_RADIO_READPOWER:
    case TMR_PARAM_RADIO_WRITEPOWER:
      {
        TMR_PortValueList powerList;
        TMR_PortValue list[TMR_SR_MAX_ANTENNA_PORTS];
        int32_t power = *(int32_t *)value;
        uint8_t i;

        powerList.max = TMR_SR_MAX_ANTENNA_PORTS;
        powerList.list = list;
        powerList.len = 0;

        /* Set all antennas to same power */
        for (i = 0; i < lr->txRxMap->len; i ++)
        {
          powerList.list[i].port  = lr->txRxMap->list[i].antenna;
          powerList.list[i].value = power;
          powerList.len ++;
        }

        if (TMR_PARAM_RADIO_READPOWER == key)
        {
          /* Set read power */
          ret = TMR_LLRP_cmdSetReadTransmitPowerList(reader, &powerList);
        }
        else
        {
          /* Set write power */
          ret = TMR_LLRP_cmdSetWriteTransmitPowerList(reader, &powerList);
        }
     
        break;
      }
    case TMR_PARAM_ANTENNA_CHECKPORT:
      {
        if (NULL == value)
        {
          ret = TMR_ERROR_INVALID;
          break;
        }
        ret = TMR_LLRP_cmdSetThingMagicAntennaDetection(reader, (bool*)value);
        break;
      }

    case TMR_PARAM_LICENSE_KEY:
      {
        if (NULL == value)
        {
          ret = TMR_ERROR_INVALID;
          break;
        }
        ret = TMR_LLRP_cmdSetTMLicenseKey(reader, (TMR_uint8List *)value);
        break;
      }

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

        if(licenseOperation->option == TMR_SET_LICENSE_KEY)
          ret = TMR_LLRP_cmdSetTMLicenseKey(reader, license);
        else
          ret = TMR_ERROR_UNIMPLEMENTED_FEATURE;
        break;
      }  

    case TMR_PARAM_TAGOP_ANTENNA:
      {
        uint16_t i;
        TMR_AntennaMapList *map;
        uint8_t antenna, txPort, rxPort;

        map = lr->txRxMap;
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
        break;
      }

    case TMR_PARAM_TAGOP_PROTOCOL:
      {
        /**
         * Check if the protocol specified is in the supported
         * protocol list
         **/
        if (0 == ((1 << (*(TMR_TagProtocol *)value - 1)) &
                                lr->supportedProtocols))
        {
          /* If not present, return unsupported error */
          ret = TMR_ERROR_UNSUPPORTED;
        }
        else
        {
          reader->tagOpParams.protocol = *(TMR_TagProtocol *)value;
          reader->u.llrpReader.currentProtocol = reader->tagOpParams.protocol;
        }
        break;
      }

    case TMR_PARAM_READ_ASYNCOFFTIME:
      {
        uint32_t offtime = *(uint32_t *)value;
        ret = TMR_LLRP_cmdSetTMAsyncOffTime(reader, offtime);
        break;
      }

    case TMR_PARAM_READ_ASYNCONTIME:
      {
        uint32_t ontime = *(uint32_t *)value;
        ret = TMR_LLRP_cmdSetTMAsyncOnTime(reader, ontime);
        break;
      }

    case TMR_PARAM_METADATAFLAG:
      {
        uint16_t metadata = *(uint16_t *)value;
#ifdef TMR_ENABLE_HF_LF
        if (metadata != TMR_TRD_METADATA_FLAG_ALL)
        {
          if (metadata & TMR_TRD_METADATA_FLAG_TAGTYPE)
          {
            return TMR_ERROR_METADATA_INVALID;
          }
        }
#endif /* TMR_ENABLE_HF_LF */
        metadata &= 0xDFFF;
        ret = TMR_LLRP_cmdSetTMMetadataFlag(reader, metadata);
        break;
      }

     case TMR_PARAM_READER_STATS_ENABLE:
      {
        uint16_t statsEnable = *(uint16_t *)value;

#ifdef TMR_ENABLE_HF_LF
        if (statsEnable != TMR_READER_STATS_FLAG_ALL)
        {
          if(statsEnable & TMR_READER_STATS_FLAG_DC_VOLTAGE)
          {
            return TMR_ERROR_INVALID_READER_STATS;
          }
        }
#endif /* TMR_ENABLE_HF_LF */

        statsEnable &= 0xBFFF;
        ret = TMR_LLRP_cmdSetTMStatsEnable(reader, statsEnable);
        if( TMR_SUCCESS == ret)
        {
          reader->u.llrpReader.statsEnable = statsEnable;
        }
        break;
      }

    case TMR_PARAM_LICENSED_FEATURES:
    case TMR_PARAM_SELECTED_PROTOCOLS:
    case TMR_PARAM_GEN2_PROTOCOLEXTENSION:
    case TMR_PARAM_PRODUCT_ID:
    case TMR_PARAM_PRODUCT_GROUP_ID:
    case TMR_PARAM_PRODUCT_GROUP:
    case TMR_PARAM_READER_STATS:
      {
        ret = TMR_ERROR_READONLY;
        break;
      }

    case TMR_PARAM_REGULATORY_MODE:
      {
        uint16_t paramValue;

        paramValue = *(uint8_t *)value;
        ret = TMR_LLRP_cmdSetThingMagicRegulatoryMode(reader, paramValue);
        break;
      }

    case TMR_PARAM_REGULATORY_MODULATION:
      {
        uint16_t paramValue;

        paramValue = *(uint8_t *)value;
        ret = TMR_LLRP_cmdSetThingMagicRegulatoryModulation(reader, paramValue);
        break;
      }

    case TMR_PARAM_REGULATORY_ENABLE:
      {
        bool paramValue;

        paramValue = *(bool *)value;
        ret = TMR_LLRP_cmdSetThingMagicRegulatoryEnable(reader, paramValue);
        break;
      }

    case TMR_PARAM_REGULATORY_ONTIME:
      {
        uint32_t paramValue;

        paramValue = *(uint32_t *)value;
        ret = TMR_LLRP_cmdSetThingMagicRegulatoryOntime(reader, paramValue);
        break;
      }

    case TMR_PARAM_REGULATORY_OFFTIME:
      {
        uint32_t paramValue;

        paramValue = *(uint32_t *)value;
        ret = TMR_LLRP_cmdSetThingMagicRegulatoryOfftime(reader, paramValue);
        break;
      }

    case TMR_PARAM_REGION_HOPTABLE:
      {
        TMR_uint32List *u32List = (TMR_uint32List *)value;

        if (NULL == u32List)
        {
          ret = TMR_ERROR_INVALID;
          break;
        }
        if (0 >= u32List->len)
        {
          ret = TMR_ERROR_ILLEGAL_VALUE;
          break;
        }
        ret = TMR_LLRP_cmdSetThingMagicRegionHoptable(reader, u32List);
        break;
      }

    case TMR_PARAM_ANTENNA_PORTSWITCHGPOS:
      {
        printf("Setting PORTSWITCH GPO parameter\n");
        if (NULL == value)
        {
          ret = TMR_ERROR_INVALID;
          break;
        }
        ret = TMR_LLRP_cmdSetThingMagicPortSwitchGPOs(reader, (TMR_uint8List *)value);
        if(ret == TMR_SUCCESS)
        {
          TMR_LLRP_initTxRxMapFromPorts(reader);
        }
        break;
      }

#ifdef TMR_ENABLE_ISO180006B
    case TMR_PARAM_ISO180006B_DELIMITER:
      {
        TMR_ISO180006B_Delimiter delimiter = *(TMR_ISO180006B_Delimiter *)value;

        if (NULL == value)
        {
          ret = TMR_ERROR_INVALID;
          break;
        }
        ret = TMR_LLRP_cmdSetISO18K6BDelimiter(reader, &delimiter);
        break;
      }

    case TMR_PARAM_ISO180006B_MODULATION_DEPTH:
      {
        TMR_ISO180006B_ModulationDepth modDepth = *(TMR_ISO180006B_ModulationDepth *)value;

        if (NULL == value)
        {
          ret = TMR_ERROR_INVALID;
          break;
        }
        ret = TMR_LLRP_cmdSetISO18K6BModDepth(reader, &modDepth);
        break;
      }

    case TMR_PARAM_ISO180006B_BLF:
      {
        TMR_ISO180006B_LinkFrequency linkFreq = *(TMR_ISO180006B_LinkFrequency *)value;

        if (NULL == value)
        {
          ret = TMR_ERROR_INVALID;
          break;
        }
        ret = TMR_LLRP_cmdSetISO18K6BLinkFrequency(reader, &linkFreq);
        break;
      }
#endif /* TMR_ENABLE_ISO180006B */

    default:
      ret = TMR_ERROR_NOT_FOUND;
  }

  return ret;
}


static TMR_Status
TMR_LLRP_paramGet(struct TMR_Reader *reader, TMR_Param key, void *value)
{
  TMR_Status ret;
  TMR_LLRP_LlrpReader *lr;

  if (NULL == reader)
  {
    return TMR_ERROR_INVALID;
  }
  ret = TMR_SUCCESS;
  lr = &reader->u.llrpReader;

  if (NULL == value)
  {
    return TMR_ERROR_ILLEGAL_VALUE;
  }

  if (BITGET(lr->paramConfirmed, key) && (0 == BITGET(lr->paramPresent, key)))
  {
    return TMR_ERROR_NOT_FOUND;
  }

  switch (key)
  {
    case TMR_PARAM_REGION_ID:
      {
        *(TMR_Region *)value = lr->regionId;
        break;
      }

    case TMR_PARAM_REGION_SUPPORTEDREGIONS:
      {
        /**
         * Parameters /reader/region/id and /reader/region/supportedRegions
         * are same in case of fixed readers
         **/
        TMR_RegionList *regions = (TMR_RegionList *)value;
        regions->len = 0;
        regions->list[regions->len++] = lr->regionId;
        break;
      }

    case TMR_PARAM_REGION_HOPTABLE:
      {
        TMR_uint32List *u32List;
        u32List = (TMR_uint32List *)value;

        ret = TMR_LLRP_cmdGetRegulatoryCapabilities(reader, u32List);
        break;
      }

    case TMR_PARAM_URI:
      {
        TMR_stringCopy((TMR_String *) value, reader->uri, (int)strlen(reader->uri));
        break;
      }

    case TMR_PARAM_ANTENNA_PORTLIST:
      {
        uint8_t i;
        TMR_uint8List *u8List;

        u8List = value;
        u8List->len = 0;

        for (i = 0; i < reader->u.llrpReader.txRxMap->len; i++)
        {
          LISTAPPEND(u8List, reader->u.llrpReader.txRxMap->list[i].antenna);
        }
        break;
      }

    case TMR_PARAM_ANTENNA_CONNECTEDPORTLIST:
      {
        TMR_LLRP_PortDetect ports[TMR_SR_MAX_ANTENNA_PORTS];
        uint8_t numPorts, i;
        TMR_uint8List *u8List;

        u8List = (TMR_uint8List *)value;
        u8List->len = 0;

        numPorts = numberof(ports);
        ret = TMR_LLRP_cmdAntennaDetect(reader, &numPorts, ports);
        if (TMR_SUCCESS != ret)
        {
          break;
        }

        if (u8List->max < numPorts)
        {
          numPorts = (uint8_t)u8List->max;
        }

        for (i = 0; i < numPorts; i++)
        {
          if (ports[i].connected)
          {
            LISTAPPEND(u8List, (uint8_t)ports[i].port);
          }
        }
        break;
      }

    case TMR_PARAM_ANTENNA_RETURNLOSS:
      {
        TMR_PortValueList *returnloss;
        returnloss = (TMR_PortValueList *)value;

        ret = TMR_LLRP_cmdGetTMAntennaReturnloss(reader, returnloss);
        if (TMR_SUCCESS != ret)
        {
          break;
        }
        break;
      }

    case TMR_PARAM_GPIO_INPUTLIST:
      {
        if (TMR_LLRP_MODEL_M6 == lr->capabilities.model ||
        		TMR_LLRP_MODEL_ASTRA_EX == lr->capabilities.model ||
        		TMR_LLRP_MODEL_SARGAS == lr->capabilities.model ||
        		TMR_LLRP_MODEL_IZAR == lr->capabilities.model ||
        		TMR_LLRP_MODEL_ASTRA200 == lr->capabilities.model)
        {
          TMR_uint8List *u8List;
          uint8_t i, numPins;

          u8List = (TMR_uint8List *)value;

          u8List->len = 0;
          if ((TMR_LLRP_MODEL_IZAR == lr->capabilities.model) || (TMR_LLRP_MODEL_ASTRA200 == lr->capabilities.model))
          {
        	  numPins = sizeGpiListIzar;
          }
          else if (TMR_LLRP_MODEL_SARGAS == lr->capabilities.model)
          {
        	  numPins = sizeGpiListSargas;
          }
          else
          {
        	  numPins = sizeGpiListM6Astra;
          }
          if (u8List->max < numPins)
          {
            numPins = (uint8_t)u8List->max;
          }

          for(i = 0; i < numPins; i++)
          {
        	  if ((TMR_LLRP_MODEL_IZAR == lr->capabilities.model) || (TMR_LLRP_MODEL_ASTRA200 == lr->capabilities.model))
        	  {
        	    LISTAPPEND(u8List, TMR_LLRP_gpiListIzar[i]);
        	  }
        	  else if (TMR_LLRP_MODEL_SARGAS == lr->capabilities.model)
        	  {
        	    LISTAPPEND(u8List, TMR_LLRP_gpiListSargas[i]);
        	  }
        	  else
        	  {
        	    LISTAPPEND(u8List, TMR_LLRP_gpiListM6Astra[i]);
        	  }
          }
        }
        break;

      }
    case TMR_PARAM_GPIO_OUTPUTLIST:
      {
        if (TMR_LLRP_MODEL_M6 == lr->capabilities.model ||
        		TMR_LLRP_MODEL_ASTRA_EX == lr->capabilities.model ||
        		TMR_LLRP_MODEL_SARGAS == lr->capabilities.model ||
        		TMR_LLRP_MODEL_IZAR == lr->capabilities.model ||
        		TMR_LLRP_MODEL_ASTRA200 == lr->capabilities.model)
        {
          TMR_uint8List *u8List;
          uint8_t i, numPins;

          u8List = (TMR_uint8List *)value;

          u8List->len = 0;
          if ((TMR_LLRP_MODEL_IZAR == lr->capabilities.model) || (TMR_LLRP_MODEL_ASTRA200 == lr->capabilities.model))
          {
        	  numPins = sizeGpoListIzar;
          }
          else if (TMR_LLRP_MODEL_SARGAS == lr->capabilities.model)
          {
        	  numPins = sizeGpoListSargas;
          }
          else
          {
        	  numPins = sizeGpoListM6Astra;
          }
          if (u8List->max < numPins)
          {
            numPins = (uint8_t)u8List->max;
          }
          for(i = 0; i < numPins; i++)
          {
        	  if ((TMR_LLRP_MODEL_IZAR == lr->capabilities.model) || (TMR_LLRP_MODEL_ASTRA200 == lr->capabilities.model))
        	  {
        	    LISTAPPEND(u8List, TMR_LLRP_gpoListIzar[i]);
        	  }
        	  else if (TMR_LLRP_MODEL_SARGAS == lr->capabilities.model)
        	  {
        	    LISTAPPEND(u8List, TMR_LLRP_gpoListSargas[i]);
        	  }
        	  else
        	  {
        	    LISTAPPEND(u8List, TMR_LLRP_gpoListM6Astra[i]);
        	  }
          }
        }
        break;
      }
    case TMR_PARAM_RADIO_POWERMAX:
      {
        TMR_uint16List *table;
        int16_t i, max;

        table = &lr->capabilities.powerTable;

        if (0 == table->len)
        {
          ret = TMR_ERROR_INVALID;
          break;
        }

        /** Find the maximum value  */
        max = table->list[1];
        for(i = 1; i <= table->len; i ++)
        {
          if(table->list[i] > max)
          {
            max = table->list[i];
          }
        }

        *(int16_t*)value = max;
        break;
      }

    case TMR_PARAM_RADIO_POWERMIN:
      {
        TMR_uint16List *table;
        int16_t i, min;

        table = &lr->capabilities.powerTable;

        if (0 == table->len)
        {
          ret = TMR_ERROR_INVALID;
          break;
        }

        /** Find the minimum value */
        min = table->list[1];
        for(i = 1; i <= table->len; i ++)
        {
          if(table->list[i] < min)
          {
            min = table->list[i];
          }
        }

        *(int16_t*)value = min;
        break;
      }

    case TMR_PARAM_RADIO_PORTREADPOWERLIST:
      {
        TMR_PortValueList *list = (TMR_PortValueList *)value;

        if (NULL == list)
        {
          ret = TMR_ERROR_ILLEGAL_VALUE;
          break;
        }

        if (0 >= list->max)
        {
          ret = TMR_ERROR_ILLEGAL_VALUE;
          break;
        }

        ret = TMR_LLRP_cmdGetReadTransmitPowerList(reader, (TMR_PortValueList *)list);
        break;
      }

    case TMR_PARAM_RADIO_PORTWRITEPOWERLIST:
      {
        TMR_PortValueList *list = (TMR_PortValueList *)value;

        if (NULL == list)
        {
          ret = TMR_ERROR_ILLEGAL_VALUE;
          break;
        }

        if (0 >= list->max)
        {
          ret = TMR_ERROR_ILLEGAL_VALUE;
          break;
        }

        ret = TMR_LLRP_cmdGetWriteTransmitPowerList(reader, (TMR_PortValueList *)list);
        break;
      }

    case TMR_PARAM_VERSION_MODEL:
      {
        const char *model;

        switch (lr->capabilities.model)
        {
          case TMR_LLRP_MODEL_M6:
            model = "Mercury6";
            break;
          case TMR_LLRP_MODEL_ASTRA_EX:
            model = "Astra-EX" ;
            break;

          case TMR_LLRP_MODEL_SARGAS:
            model = "Sargas" ;
            break;

          case TMR_LLRP_MODEL_IZAR:
            model = "Izar" ;
            break;

          case TMR_LLRP_MODEL_ASTRA200:
            model = "Astra200";
            break;

          default:
            model = "Unknown";
        }

        TMR_stringCopy(value, model, (int)strlen(model));
        break;
      }
    case TMR_PARAM_VERSION_SERIAL:
      {
        //TMR_String *val = (TMR_String *) value;
        //ret = TMR_LLRP_cmdGetVersionSerial (reader, (TMR_String *)val);
        //break;
        TMR_String *val = (TMR_String *)value;
        ret = TMR_LLRP_cmdGetTMDeviceInformationCapabilities (reader, TMR_PARAM_VERSION_SERIAL, (TMR_String *)val);
        break;

      }

    case TMR_PARAM_VERSION_SOFTWARE:
      {
        char *software = lr->capabilities.softwareVersion;
        TMR_stringCopy(value, software, (int)strlen(software));
        break;
      }
      
    case TMR_PARAM_VERSION_HARDWARE:
      {
        TMR_String *val = (TMR_String *)value;
        ret = TMR_LLRP_cmdGetTMDeviceInformationCapabilities (reader, TMR_PARAM_VERSION_HARDWARE, (TMR_String *)val);

        break;
      }

    case TMR_PARAM_COMMANDTIMEOUT:
      {
        *(uint32_t *)value = lr->commandTimeout;
        break;
      }

    case TMR_PARAM_TRANSPORTTIMEOUT:
      {
        *(uint32_t *)value = lr->transportTimeout;
        break;
      }

    case TMR_PARAM_GEN2_ACCESSPASSWORD:
      {
        *(TMR_GEN2_Password *)value = lr->gen2AccessPassword;
        break;
      }

    case TMR_PARAM_TAGREADDATA_RECORDHIGHESTRSSI:
    case TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA:
    case TMR_PARAM_TAGREADDATA_UNIQUEBYDATA:
      {
        TMR_LLRP_TMDeDuplication duplication;
        ret = TMR_LLRP_cmdGetThingMagicDeDuplication(reader, &duplication);
        if (TMR_SUCCESS != ret)
        {
          break;
        }

        switch(key)
        {
          case TMR_PARAM_TAGREADDATA_RECORDHIGHESTRSSI:
            {
              *(bool*)value = (bool)duplication.highestRSSI;
              break;
            }
          case TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA:
            {
              *(bool*)value = (bool)duplication.uniquebyAntenna;
              break;
            }
          case TMR_PARAM_TAGREADDATA_UNIQUEBYDATA:
            {
              *(bool*)value = (bool)duplication.uniquebyData;
              break;
            }
          default:
            ret = TMR_ERROR_NOT_FOUND;
        }

        break;
      }

    case TMR_PARAM_READER_DESCRIPTION:
      {
        TMR_String *desc = (TMR_String *)value;
        TMR_LLRP_TMReaderConfiguration config;

        if (NULL != desc->value)
        {
          ret = TMR_LLRP_cmdGetThingmagicReaderConfiguration(reader, &config);
          if (TMR_SUCCESS == ret)
          {
            TMR_stringCopy(desc, (char *)config.description.pValue, config.description.nValue);
            TMR_LLRP_freeTMReaderConfiguration(&config);
          }
        }
        else
        {
          ret = TMR_ERROR_ILLEGAL_VALUE;
        }

        break;
      }

    case TMR_PARAM_READER_HOSTNAME:
      {
        TMR_String *hostname = (TMR_String *)value;
        TMR_LLRP_TMReaderConfiguration config;
        hostname->max = 25;

        if (NULL != hostname->value)
        {
          ret = TMR_LLRP_cmdGetThingmagicReaderConfiguration(reader, &config);
          if (TMR_SUCCESS == ret)
          {
            TMR_stringCopy(hostname, (char *)config.hostName.pValue, config.hostName.nValue);
            TMR_LLRP_freeTMReaderConfiguration(&config);
          }
        }
        else
        {
          ret = TMR_ERROR_ILLEGAL_VALUE;
        }

        break;
      }

    case TMR_PARAM_CURRENTTIME:
      {
        ret = TMR_LLRP_cmdGetThingMagicCurrentTime(reader, (struct tm *)value);
        break;
      }

    case TMR_PARAM_RADIO_TEMPERATURE:
    {
      ret = TMR_LLRP_cmdGetThingMagicReaderModuleTemperature(reader, (int8_t *)value);
      break;
    }
    case TMR_PARAM_ANTENNA_CHECKPORT:
    {
      ret = TMR_LLRP_cmdGetThingMagicAntennaDetection(reader, (bool *)value);
      break;
    }

    case TMR_PARAM_VERSION_SUPPORTEDPROTOCOLS:
    {
      TMR_TagProtocolList *protocol = (TMR_TagProtocolList *)value;
      if (NULL == protocol->list)
      {
        ret = TMR_ERROR_ILLEGAL_VALUE;
        break;
      }

      ret = TMR_LLRP_cmdGetTMDeviceProtocolCapabilities(reader, (TMR_TagProtocolList *)protocol);
      break;
    }

    case TMR_PARAM_GEN2_BLF:
      {
        TMR_LLRP_RFControl rfControl;
        TMR_GEN2_LinkFrequency *blf = (TMR_GEN2_LinkFrequency *)value;

        ret = TMR_LLRP_cmdGetActiveRFControl(reader, &rfControl);
        if (TMR_SUCCESS != ret)
        {
          break;
        }

        *blf = lr->capabilities.u.gen2Modes[rfControl.index].blf;
        break;
      }

    case TMR_PARAM_GEN2_TAGENCODING:
      {
        TMR_LLRP_RFControl rfControl;
        TMR_GEN2_TagEncoding *m = (TMR_GEN2_TagEncoding *)value;

        ret = TMR_LLRP_cmdGetActiveRFControl(reader, &rfControl);
        if (TMR_SUCCESS != ret)
        {
          break;
        }

        *m = lr->capabilities.u.gen2Modes[rfControl.index].m;
        break;
      }

    case TMR_PARAM_GEN2_TARI:
      {
        TMR_LLRP_RFControl rfControl;
        TMR_GEN2_Tari *tari = (TMR_GEN2_Tari *)value;

        ret = TMR_LLRP_cmdGetActiveRFControl(reader, &rfControl);
        if (TMR_SUCCESS != ret)
        {
          break;
        }

        *tari = rfControl.tari;
        break;
      }

    case TMR_PARAM_GEN2_Q:
      {
        TMR_GEN2_Q *q = (TMR_GEN2_Q *)value;

        ret = TMR_LLRP_cmdGetGen2Q(reader, q);
        break;
      }

    case TMR_PARAM_GEN2_SEND_SELECT:
      {
         bool *select;
         select = (bool *)value;
         ret = TMR_LLRP_cmdGetsendSelect(reader, (bool *)select);

         break;
      }

    case TMR_PARAM_GEN2_INITIAL_Q:
      {
        TMR_GEN2_initQ *q = (TMR_GEN2_initQ *)value;
        ret = TMR_LLRP_cmdGetInitQ(reader, q);

        break;
      }

    case TMR_PARAM_GEN2_TARGET:
      {
        TMR_GEN2_Target *target = (TMR_GEN2_Target *)value;

        ret = TMR_LLRP_cmdGetGen2Target(reader, target);
        break;
      }

    case TMR_PARAM_GEN2_T4:
      {
        uint32_t *paramValue = (uint32_t *)value;

        ret = TMR_LLRP_cmdGetGen2T4Param(reader, paramValue);
        break;
      }

    case TMR_PARAM_LICENSE_KEY:
      {
        ret = TMR_ERROR_UNSUPPORTED;
        break;
      }

    case TMR_PARAM_MANAGE_LICENSE_KEY:
      {
        ret = TMR_ERROR_UNSUPPORTED;
        break;
      }

    case TMR_PARAM_GEN2_SESSION:
      {
        TMR_GEN2_Session *session = (TMR_GEN2_Session *)value;

        ret = TMR_LLRP_cmdGetGen2Session(reader, session);
        break;
      }

    case TMR_PARAM_RADIO_READPOWER:
    case TMR_PARAM_RADIO_WRITEPOWER:
      {
        TMR_PortValueList powerList;
        TMR_PortValue list[TMR_SR_MAX_ANTENNA_PORTS];
        uint8_t i;
        int32_t power;


        powerList.max  = TMR_SR_MAX_ANTENNA_PORTS;
        powerList.list = list;
        powerList.len  = 0;

        if (TMR_PARAM_RADIO_READPOWER == key)
        {
          /* Get per port read power list */
          ret = TMR_LLRP_cmdGetReadTransmitPowerList(reader, &powerList);
        }
        else
        {
          /* Get per port write power list */
          ret = TMR_LLRP_cmdGetWriteTransmitPowerList(reader, &powerList);
        }
        if (TMR_SUCCESS != ret)
        {
          break;
        }
       
        power = powerList.list[0].value;
        for (i = 1; i < powerList.len; i ++)
        {
          if (power != powerList.list[i].value)
          {
            return TMR_ERROR_LLRP_UNDEFINED_VALUE;
          }
        }

        /** 
         * At this point, all antennas have same power
         * return that value 
         **/
        *(int32_t *)value = (int32_t)power;
        
        break;
      }

    case TMR_PARAM_TAGOP_ANTENNA:
      {
        *(uint8_t *)value = reader->tagOpParams.antenna;
        break;
      }

    case TMR_PARAM_TAGOP_PROTOCOL:
      {
        *(TMR_TagProtocol *)value = reader->tagOpParams.protocol;
        break;
      }
    case TMR_PARAM_READ_ASYNCOFFTIME:
      {
        uint32_t offtime;
        ret = TMR_LLRP_cmdGetTMAsyncOffTime(reader, &offtime);
        if (TMR_SUCCESS != ret)
        {
          break;
        }
        *(uint32_t *)value = offtime;
        break;
      }

    case TMR_PARAM_READ_ASYNCONTIME:
      {
        uint32_t ontime;
        ret = TMR_LLRP_cmdGetTMAsyncOnTime(reader, &ontime);
        if (TMR_SUCCESS != ret)
        {
          break;
        }
        *(uint32_t *)value = ontime;
        break;
      }

    case TMR_PARAM_METADATAFLAG:
      {
        uint16_t metadata;
        ret = TMR_LLRP_cmdGetTMMetadataFlag(reader, &metadata);
        if (TMR_SUCCESS != ret)
        {
          break;
        }
        *(uint16_t *)value = metadata;
        break;
      }

    case TMR_PARAM_READER_STATS_ENABLE:
      {
        uint16_t statsEnable;
        ret = TMR_LLRP_cmdGetTMStatsEnable(reader, &statsEnable);
        if (TMR_SUCCESS != ret)
        {
          break;
        }
        *(uint16_t *)value = statsEnable;
        break;
      }

    case TMR_PARAM_READER_STATS:
      {
        ret = TMR_LLRP_cmdGetTMStatsValue(reader, (TMR_Reader_StatsValues *)value);
        if (TMR_SUCCESS != ret)
        {
          break;
        }
        break;
      } 

    case TMR_PARAM_LICENSED_FEATURES:
      {
        if((TMR_LLRP_MODEL_SARGAS == lr->capabilities.model) || (TMR_LLRP_MODEL_IZAR== lr->capabilities.model) ||
           (TMR_LLRP_MODEL_ASTRA200== lr->capabilities.model))
        {
          TMR_uint8List *val = (TMR_uint8List *)value;
          ret = TMR_LLRP_cmdGetLicensedFeatures(reader, (TMR_uint8List *)val);
          if (TMR_SUCCESS != ret)
          {
            break;
          }
        }
        else
        {
          ret = TMR_ERROR_UNSUPPORTED;
        }
        break;
      }
    case TMR_PARAM_SELECTED_PROTOCOLS:
      {
        if((TMR_LLRP_MODEL_SARGAS == lr->capabilities.model) || (TMR_LLRP_MODEL_IZAR == lr->capabilities.model) ||
           (TMR_LLRP_MODEL_ASTRA200 == lr->capabilities.model))
        {
          TMR_TagProtocolList *protocol = (TMR_TagProtocolList *)value;
          if (NULL == protocol->list)
          {
            ret = TMR_ERROR_ILLEGAL_VALUE;
            break;
          }
          //ret = TMR_LLRP_cmdGetTMDeviceProtocolCapabilities(reader, (TMR_TagProtocolList *)protocol);
          ret = TMR_LLRP_cmdGetSelectedProtocols(reader, (TMR_TagProtocolList*) protocol);
          if (TMR_SUCCESS != ret)
          {
            break;
          }
        }
        else
        {
          ret = TMR_ERROR_UNSUPPORTED;
        }
        break;
      }
    case TMR_PARAM_GEN2_PROTOCOLEXTENSION:
    {
      ret = TMR_LLRP_cmdGetThingMagicGEN2ProtocolExtension(reader, (uint8_t *)value);
      break;
    }
    case TMR_PARAM_PRODUCT_ID:
    case TMR_PARAM_PRODUCT_GROUP_ID:
    {
      ret = TMR_LLRP_cmdGetTMDeviceInformationIDs(reader, key, (uint16_t *)value);
      break;
    }
    case TMR_PARAM_PRODUCT_GROUP:
    {
      TMR_String *group = (TMR_String *)value;
      ret = TMR_LLRP_cmdGetTMDeviceInformationCapabilities(reader, TMR_PARAM_PRODUCT_GROUP, (TMR_String *)group);

      break;
    }

    case TMR_PARAM_REGULATORY_MODE:
      {
        uint16_t *paramValue = (uint16_t *)value;

        ret = TMR_LLRP_cmdGetThingMagicRegulatoryMode(reader, paramValue);
        break;
      }

    case TMR_PARAM_REGULATORY_MODULATION:
      {
        uint16_t *paramValue = (uint16_t *)value;

        ret = TMR_LLRP_cmdGetThingMagicRegulatoryModulation(reader, paramValue);
        break;
      }

    case TMR_PARAM_REGULATORY_ENABLE:
      {
        bool *paramValue = (bool *)value;

        ret = TMR_LLRP_cmdGetThingMagicRegulatoryEnable(reader, paramValue);
        break;
      }

    case TMR_PARAM_REGULATORY_ONTIME:
      {
        uint32_t *paramValue = (uint32_t *)value;

        ret = TMR_LLRP_cmdGetThingMagicRegulatoryOntime(reader, paramValue);
        break;
      }

    case TMR_PARAM_REGULATORY_OFFTIME:
      {
        uint32_t *paramValue = (uint32_t *)value;

        ret = TMR_LLRP_cmdGetThingMagicRegulatoryOfftime(reader, paramValue);
        break;
      }

#ifdef TMR_ENABLE_ISO180006B
    case TMR_PARAM_ISO180006B_DELIMITER:
      {
        TMR_ISO180006B_Delimiter *delimiter = (TMR_ISO180006B_Delimiter *)value;

        ret = TMR_LLRP_cmdGetISO18K6BDelimiter(reader, delimiter);
        break;
      }

    case TMR_PARAM_ISO180006B_MODULATION_DEPTH:
      {
        TMR_ISO180006B_ModulationDepth *modDepth = (TMR_ISO180006B_ModulationDepth *)value;

        ret = TMR_LLRP_cmdGetISO18K6BModDepth(reader, modDepth);
        break;
      }

    case TMR_PARAM_ISO180006B_BLF:
      {
        TMR_ISO180006B_LinkFrequency *linkFreq = (TMR_ISO180006B_LinkFrequency *)value;
        
        ret = TMR_LLRP_cmdGetISO18K6BLinkFrequency(reader, linkFreq);
        break;
      }
#endif /* TMR_ENABLE_ISO180006B */

    default:
      ret = TMR_ERROR_NOT_FOUND;
  }

  if (0 == BITGET(lr->paramConfirmed, key))
  {
    if (TMR_SUCCESS == ret)
    {
      BITSET(lr->paramPresent, key);
    }
    BITSET(lr->paramConfirmed, key);
  }

  return ret;
}

TMR_Status
TMR_LLRP_LlrpReader_init(TMR_Reader *reader)
{
  if (NULL == reader)
  {
    return TMR_ERROR_INVALID;
  }
  reader->readerType = TMR_READER_TYPE_LLRP;
  
  /*
   * Allocate the type registry. This is needed
   * by the connection to decode.
   */
  reader->u.llrpReader.pTypeRegistry = LLRP_getTheTypeRegistry();
  if(NULL == reader->u.llrpReader.pTypeRegistry)
  {
    return TMR_ERROR_LLRP_GETTYPEREGISTRY;
  }
  /**
   * Enroll ThingMagic custom entities to the registry
   **/
  LLRP_enrollTmTypesIntoRegistry(reader->u.llrpReader.pTypeRegistry);


  /**
   * Configure the reader methods to point to LLRP methods
   **/
  reader->connect     = TMR_LLRP_connect;
  reader->destroy     = TMR_LLRP_destroy;
  reader->paramSet    = TMR_LLRP_paramSet;
  reader->paramGet    = TMR_LLRP_paramGet;
  reader->read        = TMR_LLRP_read;
  reader->hasMoreTags = TMR_LLRP_hasMoreTags;
  reader->getNextTag  = TMR_LLRP_getNextTag;
  reader->executeTagOp = TMR_LLRP_executeTagOp;
  reader->gpiGet = TMR_LLRP_gpiGet;
  reader->gpoSet = TMR_LLRP_gpoSet;
  reader->readTagMemBytes = TMR_LLRP_readTagMemBytes;
  reader->readTagMemWords = TMR_LLRP_readTagMemWords;
  reader->writeTagMemBytes = TMR_LLRP_writeTagMemBytes;
  reader->writeTagMemWords = TMR_LLRP_writeTagMemWords;
  reader->firmwareLoad = TMR_LLRP_firmwareLoad;
  reader->reboot = TMR_LLRP_reboot;
  reader->writeTag = TMR_LLRP_writeTag;
  reader->killTag = TMR_LLRP_killTag;
  reader->lockTag = TMR_LLRP_lockTag;
//  reader->resetHoptable = TMR_LLRP_resetHoptable;
#ifdef TMR_ENABLE_BACKGROUND_READS
  reader->cmdStopReading = TMR_LLRP_cmdStopReading;
#endif
  
  /**
   * Initialize llrpReader members to default values
   **/
  reader->u.llrpReader.msgId = 1;
  reader->u.llrpReader.roSpecId = 0;
  reader->u.llrpReader.opSpecId = 0;
  reader->u.llrpReader.accessSpecId = 0;
  reader->u.llrpReader.gen2AccessPassword = 0;
  memset(reader->u.llrpReader.paramConfirmed,0,
         sizeof(reader->u.llrpReader.paramConfirmed));
  memset(reader->u.llrpReader.paramPresent,0,
         sizeof(reader->u.llrpReader.paramPresent));
  
  reader->u.llrpReader.transportTimeout = 5000;
  reader->u.llrpReader.commandTimeout = 1000;
  reader->u.llrpReader.currentProtocol = TMR_TAG_PROTOCOL_NONE;
  reader->u.llrpReader.supportedProtocols = 0;
  reader->u.llrpReader.regionId = TMR_REGION_NONE;
  reader->u.llrpReader.tagsRemaining = 0;
  reader->continuousReading = false;
  reader->u.llrpReader.capabilities.model = 0;
  reader->u.llrpReader.metadata = 0;

  /* Initialize tagOpParams */
  reader->tagOpParams.antenna = 1;
  reader->tagOpParams.protocol = TMR_TAG_PROTOCOL_GEN2;

  /* Initialize llrp receiver thread params */
  pthread_mutex_init(&reader->u.llrpReader.receiverLock, NULL);
  pthread_cond_init(&reader->u.llrpReader.receiverCond, NULL);
  reader->u.llrpReader.receiverSetup = false;
  reader->u.llrpReader.receiverRunning = false;
  reader->u.llrpReader.receiverEnabled = false;
  reader->u.llrpReader.numOfROSpecEvents = 0;
  reader->u.llrpReader.bufResponse = NULL;

  /* Initialize llrp transmitter thread params */
  pthread_mutex_init(&reader->u.llrpReader.transmitterLock, NULL);

  /* Initialize keep alive params */
  reader->u.llrpReader.ka_start = 0;
  reader->u.llrpReader.ka_now   = 0;
  reader->u.llrpReader.keepAliveAckMissCnt = 0;
  reader->u.llrpReader.get_report = false;
  reader->u.llrpReader.reportReceived = false;
  reader->u.llrpReader.isResponsePending = false;
  reader->u.llrpReader.threadCancel = false;
  return TMR_reader_init_internal(reader);
}

static TMR_Status
TMR_LLRP_openConnectionToReader(TMR_Reader *reader, LLRP_tSConnection *pConn)
{
#if defined(WIN32)|| defined(WINCE)  /* WIN32 or WINCE */
    SOCKET sock;
#else                                /* linux */
    int sock;
#endif                               /* WIN32 or WINCE */
  static const struct addrinfo addrInfoMask =
  {    
    0,   
    AF_INET,
    SOCK_STREAM,
    0,                                                                                                                                                             
    0,   
    NULL,
    NULL,
    NULL 
  };   
  struct addrinfo *           hostAddress;
  int                         flag;
  struct sockaddr_in          sin; 
  int                         rc;

  if (NULL == reader)
  {
    return TMR_ERROR_INVALID;
  }
  if (NULL == pConn)
  {
    return TMR_ERROR_LLRP_CONNECTIONFAILED;
  }

  /*
   * Clear the connect error string
   */
  pConn->pConnectErrorStr = NULL;

  /*
   * Make sure there isn't already a connection.
   */
#if defined(WIN32)|| defined(WINCE)  /* WIN32 or WINCE */
    if(INVALID_SOCKET != pConn->fd) 
#else                                /* linux */                     
    if(0 <= pConn->fd)
#endif                               /* WIN32 or WINCE */
  {
    pConn->pConnectErrorStr = "already connected";
    return TMR_ERROR_LLRP_ALREADY_CONNECTED;
  }
#if defined(WIN32)|| defined(WINCE)  /* WIN32 or WINCE */
    /*
     * On Windows have to enable the WinSock library
     */
    {
        WSADATA SocketLibraryInitData;
        WSAStartup(0xFFFF, &SocketLibraryInitData);
    }
#endif  /* WIN32 or WINCE */
  /*
   * Look up host using getaddrinfo().
   * Gethostbyname() could be configured a lot of
   * different ways. There is /etc/hosts, DNS, NIS, etc, etc.
   * Suffice to say it is big, bulky, and susceptible to stall.
   */
  if(0 != getaddrinfo(reader->uri, NULL, &addrInfoMask, &hostAddress))
  {
    pConn->pConnectErrorStr = "host lookup failed";
    return -1;
  }

  /*
   * Convert the address to sockaddr_in format
   */
  memset(&sin, 0, sizeof sin);
  sin.sin_family = AF_INET;
  sin.sin_addr = ((struct sockaddr_in *)(hostAddress->ai_addr))->sin_addr;
  sin.sin_port = htons(reader->u.llrpReader.portNum);

  /*
   * Done withe the host addrinfo
   */
  freeaddrinfo(hostAddress);

  /*
   * Create the socket.
   */
  sock = socket(AF_INET, SOCK_STREAM, 0);
#if defined(WIN32)|| defined(WINCE) /* WIN32 or WINCE */
    if(INVALID_SOCKET == sock)
#else                        /* linux */
    if(0 > sock)
#endif                              /* WIN32 or WINCE*/
  {
    pConn->pConnectErrorStr = "socket() failed";
    return TMR_ERROR_LLRP_CONNECTIONFAILED;
  }

  /*
   * Connect the socket to reader. This can stall.
   */
  rc = connect(sock, (struct sockaddr *)&sin, sizeof sin);
  if(0 > rc)
  {
    /* Connect failed */
    /* check for the error no */
#if defined(WIN32)|| defined(WINCE) /* WIN32 or WINCE */
    if (WSAECONNREFUSED ==  errno)
#else                               /* linux */
    if (ECONNREFUSED ==  errno)
#endif                              /* WIN32 or WINCE */
	  {
      /** 
       * Server is not accepting any connection, lets wait some time and try
       * again.
       */
      tmr_sleep(1000);
    }
    pConn->pConnectErrorStr = "connect() failed";
#if defined(WIN32)|| defined(WINCE)  /* WIN32 or WINCE */
    closesocket(sock);	
#else                                /* linux */
    close(sock);
#endif                               /* WIN32 or WINCE */
    return TMR_ERROR_LLRP_CONNECTIONFAILED;
	}

  /*
   * Best effort to set no delay. If this doesn't work
   * (no reason it shouldn't) we do not declare defeat.
   */
  flag = 1;

#if defined(WIN32)|| defined(WINCE)  /* WIN32 or WINCE */
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof flag);
#else                                /* linux */
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void*)&flag, sizeof flag);
#endif                               /* WIN32 or WINCE */
  /*
   * Record the socket in the connection instance
   */
  pConn->fd = sock;

  return TMR_SUCCESS;
}

TMR_Status
TMR_LLRP_connect(TMR_Reader *reader)
{
  TMR_Status ret;
  LLRP_tSMessage *pMsg = NULL;
  LLRP_tSREADER_EVENT_NOTIFICATION *pNtf;
  LLRP_tSReaderEventNotificationData *pNtfData;
  LLRP_tSConnectionAttemptEvent *pEvent;
  LLRP_tSConnection *pConn;

  if (NULL == reader)
  {
    return TMR_ERROR_INVALID;
  }
  ret = TMR_SUCCESS;
  /*
   * Construct a connection (LLRP_tSConnection).
   * Using a 32kb max frame size for send/recv.
   * The connection object is ready for business
   * but not actually connected to the reader yet.
   */
  reader->u.llrpReader.pConn = LLRP_Conn_construct(reader->u.llrpReader.pTypeRegistry, 32u*1024u);
  if(NULL == reader->u.llrpReader.pConn)
  {
    sprintf(reader->u.llrpReader.errMsg, "Error: Connection initialization failed");
    return TMR_ERROR_LLRP_CONNECTIONFAILED;
  }

  /*
   * Open the connection to the reader
   */
  pConn = reader->u.llrpReader.pConn;
  ret = TMR_LLRP_openConnectionToReader(reader, pConn);
  if(TMR_SUCCESS != ret)
  {
    sprintf(reader->u.llrpReader.errMsg,
            "Error: Connect - %s", pConn->pConnectErrorStr);
		LLRP_Conn_destruct(pConn);
		reader->u.llrpReader.pConn = NULL;
    return TMR_ERROR_LLRP_CONNECTIONFAILED;
  }

  pthread_mutex_lock(&reader->u.llrpReader.receiverLock);
  /* Receive READER_EVENT_NOTIFICATION.ConnectionAttemptEvent */
  ret = TMR_LLRP_receiveMessage(reader, &pMsg, reader->u.llrpReader.commandTimeout + reader->u.llrpReader.transportTimeout);
  pthread_mutex_unlock(&reader->u.llrpReader.receiverLock);
  if (TMR_SUCCESS != ret)
  {
    goto fail;
  }

  /*
   * Check to make sure the message is of the right type.
   * The type label (pointer) in the message should be
   * the type descriptor for READER_EVENT_NOTIFICATION.
   */
  if (&LLRP_tdREADER_EVENT_NOTIFICATION != pMsg->elementHdr.pType)
  {
    goto fail;
  }

  /*
   * Now that we are sure it is a READER_EVENT_NOTIFICATION,
   * traverse to the ReaderEventNotificationData parameter.
   */
  pNtf = (LLRP_tSREADER_EVENT_NOTIFICATION *) pMsg;
  pNtfData = pNtf->pReaderEventNotificationData;
  if(NULL == pNtfData)
  {
    goto fail;
  }

  /*
   * The ConnectionAttemptEvent parameter must be present.
   */
  pEvent = pNtfData->pConnectionAttemptEvent;
  if(NULL == pEvent)
  {
    goto fail;
  }

  /*
   * The status in the ConnectionAttemptEvent parameter
   * must indicate connection success.
   */
  if(LLRP_ConnectionAttemptStatusType_Success != pEvent->eStatus)
  {
    if(LLRP_ConnectionAttemptStatusType_Failed_A_Client_Initiated_Connection_Already_Exists == pEvent->eStatus)
    {
      TMR_LLRP_freeMessage(pMsg);
      sprintf(reader->u.llrpReader.errMsg,
          "Error: Connection attempt failed - Client connection already exists");
      return TMR_ERROR_LLRP_CLIENT_CONNECTION_EXISTS;
    }
    goto fail;
  }

  /*
   * At this point, We are sure that the connection is success
   * Free the message
   */
  TMR_LLRP_freeMessage(pMsg);
  reader->connected = true;
  ret = TMR_LLRP_boot(reader);

  /* At this point we will have the software version.
   * Check for available features as per software version.
  */
  checkForAvailableReaderFeatures(reader);

  return ret;

fail:
  /*
   * Connection attempt is not successful, Return Error
   */
  TMR_LLRP_freeMessage(pMsg);
  sprintf(reader->u.llrpReader.errMsg,
      "Error: Connection attempt failed");
  return TMR_ERROR_LLRP_CONNECTIONFAILED;
}

TMR_Status
TMR_LLRP_destroy(TMR_Reader *reader)
{
  LLRP_tSMessage *pRspMsg = NULL;
  uint8_t i;

  if (NULL == reader)
  {
    return TMR_ERROR_INVALID;
  }
  {
#if defined(WIN32)|| defined(WINCE)  /* WIN32 or WINCE */
  LLRP_tSCLOSE_CONNECTION CloseConn = {0};
  CloseConn.hdr.elementHdr.pType = &LLRP_tdCLOSE_CONNECTION;
  CloseConn.hdr.MessageID        = reader->u.llrpReader.msgId ++;
#else
  LLRP_tSCLOSE_CONNECTION CloseConn = {
    .hdr.elementHdr.pType = &LLRP_tdCLOSE_CONNECTION,
    .hdr.MessageID        = reader->u.llrpReader.msgId ++,
  };
#endif  

  /**
   * Terminate llrp receiver thread
   **/

  /** enable this to send a signal for receiver thread to exit */
  reader->u.llrpReader.threadCancel = true;

  /* Before Waiting for receiver thread to cancel, 
   * make sure conditional wait is unblocked by broadcasting the receiverCond.
   * Some times it may get blocked waiting for the condition to get released.
   */
  pthread_mutex_lock(&reader->u.llrpReader.receiverLock);
  reader->u.llrpReader.receiverEnabled = true;
  pthread_cond_broadcast(&reader->u.llrpReader.receiverCond);
  pthread_mutex_unlock(&reader->u.llrpReader.receiverLock);

  /** wait for the thread to exit */
  pthread_join(reader->u.llrpReader.llrpReceiver, NULL);
  reader->u.llrpReader.threadCancel = false;

  pthread_mutex_lock(&reader->u.llrpReader.receiverLock);
  if (true == reader->u.llrpReader.receiverSetup)
  {
#ifdef WIN32
	Sleep(1);
#else
    usleep(10);
#endif
    reader->u.llrpReader.receiverSetup = false;
  }
  pthread_mutex_unlock(&reader->u.llrpReader.receiverLock);

#ifdef TMR_ENABLE_BACKGROUND_READS
  /* Cleanup background threads */
  cleanup_background_threads(reader);
#endif

  /** May not freeed before, free it now */
  if (NULL != reader->u.llrpReader.bufResponse)
  {
    /* free this before leaking the memory */
    for (i = 0; i < reader->u.llrpReader.bufPointer; i++)
    {
      TMR_LLRP_freeMessage(reader->u.llrpReader.bufResponse[i]);
    }
    free(reader->u.llrpReader.bufResponse);
    reader->u.llrpReader.bufResponse=NULL;
  }

  if (true == reader->connected)
  {
    /**
     * Send CLOSE_CONNECTION message
     */
    TMR_LLRP_send(reader, &CloseConn.hdr, &pRspMsg);
    /**
     * If the response to CLOSE_CONNECTION is not M_Success, close connection anyway
     * There's nothing the API can do about erros at this point of time,
     * Since it just want to stop talking to the reader.
     * Do not check for the ret value here
     **/
    if (NULL != pRspMsg)
    {
      TMR_LLRP_checkLLRPStatus(((LLRP_tSCLOSE_CONNECTION_RESPONSE *) pRspMsg)->pLLRPStatus);

      /* Free the response */
      TMR_LLRP_freeMessage(pRspMsg);
    }
    /*
     * Close the connection and release its resources
     */
  }
  if (NULL != reader->u.llrpReader.pConn)
  {
    LLRP_Conn_destruct(reader->u.llrpReader.pConn);
    reader->u.llrpReader.pConn=NULL;
  }
  if (NULL != reader->u.llrpReader.pTypeRegistry)
  {
    LLRP_TypeRegistry_destruct(reader->u.llrpReader.pTypeRegistry);
    reader->u.llrpReader.pTypeRegistry=NULL;
  }
  reader->connected = false;
  }

  return TMR_SUCCESS;
}

static TMR_Status
TMR_LLRP_read_internal(TMR_Reader *reader,
                        uint32_t timeoutMs,
                        TMR_ReadPlan *rp)
{
  TMR_Status ret;
  TMR_LLRP_LlrpReader *lr;
  TMR_uint8List *antennaList = NULL;
  uint8_t i;
  bool tagopPresent = false;
  TMR_TagFilter *filterList = NULL;

  if (NULL == reader)
  {
    return TMR_ERROR_INVALID;
  }
  if(reader->u.llrpReader.featureFlags & TMMP_READER_FEATURES_FLAG_PERANTENNA_ONTIME)
  {
    isPerAntennaEnabled = true;
  }
  else
  {
    isPerAntennaEnabled = false;
  }
  ret = TMR_SUCCESS;
  lr  = &reader->u.llrpReader;
  isStandaloneTagop = false;

  if (!isPerAntennaEnabled)
  {
    lr->searchTimeoutMs = timeoutMs;
    if (TMR_READ_PLAN_TYPE_SIMPLE == rp->type)
    {
      antennaList = &rp->u.simple.antennas;
      reader->fastSearch = rp->u.simple.useFastSearch;
      filterList = rp->u.simple.filter;

      if (NULL != rp->u.simple.tagop)
      {
        tagopPresent = true;
      }
      if(!reader->continuousReading)
      {
        /* Currently only supported for sync read case */
        reader->isStopNTags = rp->u.simple.stopOnCount.stopNTriggerStatus;
        reader->numberOfTagsToRead = rp->u.simple.stopOnCount.noOfTags;
        if(reader->isStopNTags == true && reader->numberOfTagsToRead == 0)
          return TMR_ERROR_INVALID;
      }
    }
    else if (TMR_READ_PLAN_TYPE_MULTI == rp->type)
    {
      /**
       * Loop through each simple read plan of multireadplan
       **/
      uint32_t subTimeout;
      int i;

      subTimeout = 0;

      /**
       * In case of multi read plan, set number of ROSpec events
       * to plan count.
       **/
      reader->u.llrpReader.numOfROSpecEvents = rp->u.multi.planCount;
      /**
       * If total weight is zero, divide the read duration equally
       * to all sub plans.
       **/
      if (0 == rp->u.multi.totalWeight)
      {
        subTimeout = timeoutMs / rp->u.multi.planCount;
      }

      for (i = 0; i < rp->u.multi.planCount; i ++)
      {
        if (rp->u.multi.totalWeight)
        {
          subTimeout = (rp->u.multi.plans[i]->weight * timeoutMs) 
                                    / rp->u.multi.totalWeight;
        }

        /* Iterate each simple read plan */
        ret = TMR_LLRP_read_internal(reader, subTimeout, rp->u.multi.plans[i]);
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
  }
  else
  {
    reader->u.llrpReader.numOfROSpecEvents = 1;
    antennaList = NULL;
    filterList = NULL;

    if (TMR_READ_PLAN_TYPE_MULTI == rp->type)
    {
      /**
       *Added this validation to limit number of read plans to 5 if build is <5.3.2.97
       *Fixed this limitation issue in build 5.3.2.97
       **/
      if ((!(reader->u.llrpReader.featureFlags & TMMP_READER_FEATURES_FLAG_INVENTORYSPEC_ID)) &&
          (rp->u.multi.planCount > MAX_SUB_PLAN_COUNT))
      {
        return TMR_ERROR_UNSUPPORTED;
      }
      for (i = 0; i < rp->u.multi.planCount; i ++)
      {
        if (NULL != rp->u.multi.plans[i]->u.simple.tagop)
        {
          tagopPresent = true;
        }
      }
    }
    else if (TMR_READ_PLAN_TYPE_SIMPLE == rp->type)
    {
      if (NULL != rp->u.simple.tagop)
      {
        tagopPresent = true;
      }
    }
  }

  /**
   * Simple/Multi read plan with no embedded tag operations
   **/
  if (false == tagopPresent)
  {
    /**
     * Prepare the reader to perform Read operation
     **/
    lr->roSpecId++;
    if (TMR_LLRP_SYNC_MAX_ROSPECS <= lr->roSpecId)
    {
      lr->roSpecId = 1;
    }
    lr->readPlanProtocol[lr->roSpecId].rospecProtocol = rp->u.simple.protocol;
    lr->readPlanProtocol[lr->roSpecId].rospecID = (uint8_t)lr->roSpecId;
    ret = TMR_LLRP_cmdPrepareROSpec(reader, (uint16_t)timeoutMs, antennaList,
                                  filterList, rp->u.simple.protocol);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
  }

  /**
   *  Read plan with embedded tag operation
   **/
  else
  {
    /**
     * Prepare the reader to perform embedded read operation.
     * PrepareROSpec first and then add and enable corresponding AccessSpec
     * specified by the user.
     **/
    lr->roSpecId ++;
    if (TMR_LLRP_SYNC_MAX_ROSPECS <= lr->roSpecId)
    {
      lr->roSpecId = 1;
    }
    lr->readPlanProtocol[lr->roSpecId].rospecProtocol = rp->u.simple.protocol;
    lr->readPlanProtocol[lr->roSpecId].rospecID = (uint8_t)lr->roSpecId;
    ret = TMR_LLRP_cmdPrepareROSpec(reader, (uint16_t)timeoutMs, antennaList,
                                  filterList, rp->u.simple.protocol);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }

    /**
     * Prepare / Add AccessSpec
     * In case of embedded operation, the TagSpec (filter to accessSpec)
     * is set to NULL, as the filter is already mentioned as part of ROSpec
     **/
    if (((TMR_READ_PLAN_TYPE_SIMPLE == rp->type) || (false == isPerAntennaEnabled)) && (NULL != rp->u.simple.tagop))
    {
      lr->accessSpecId ++;
      ret = TMR_LLRP_cmdAddAccessSpec(reader, rp->u.simple.protocol,
                                    NULL, lr->roSpecId, rp->u.simple.tagop, false);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }

      /**
       * Enable access spec first, so as to make this access spec active
       * before starting ROSpec.
       **/
      ret = TMR_LLRP_cmdEnableAccessSpec(reader, lr->accessSpecId);
      if (TMR_SUCCESS != ret)
      {
        return ret;
      }
    }
    else if ((TMR_READ_PLAN_TYPE_MULTI == rp->type) && (true == isPerAntennaEnabled))
    {
      /**
       * Prepare / Add AccessSpec
       * In case of embedded operation, the TagSpec (filter to accessSpec)
       * is set to NULL, as the filter is already mentioned as part of ROSpec
       **/
      for (i = 0; i < rp->u.multi.planCount; i ++)
      {
        if (NULL != rp->u.multi.plans[i]->u.simple.tagop)
        {
        lr->accessSpecId ++;
        currentInventorySpecID = i+1;
        ret = TMR_LLRP_cmdAddAccessSpec(reader, rp->u.multi.plans[i]->u.simple.protocol,
                                      NULL, lr->roSpecId, rp->u.multi.plans[i]->u.simple.tagop, false);
        if (TMR_SUCCESS != ret)
        {
          return ret;
        }

        /**
         * Enable access spec first, so as to make this access spec active
         * before starting ROSpec.
         **/
        ret = TMR_LLRP_cmdEnableAccessSpec(reader, lr->accessSpecId);
        if (TMR_SUCCESS != ret)
        {
          return ret;
        }
        }
      }
    }
  }

  if ((true == reader->continuousReading) &&
      (TMR_READ_PLAN_TYPE_MULTI == reader->readParams.readPlan->type) &&
      (false == isPerAntennaEnabled))
  {
    /**
     * Incase of multiread plan with async read and per antenna support not available, ROSpec's start trigger
     * is set to periodic, So no need to send START_ROSPEC message.
     **/
    return ret;
  }
  else
  {
    /**
     * ROSpec's start trigger type will not be set to periodic when per antenna ontime support available
     * So need to send START_ROSPEC message.
     **/
    return TMR_LLRP_cmdStartROSpec(reader, lr->roSpecId);
  }
}

TMR_Status
TMR_LLRP_read(TMR_Reader *reader, uint32_t timeoutMs, int32_t *tagCount)
{
  TMR_Status ret;
  TMR_ReadPlan *rp;
  uint8_t i;

  if (NULL == reader)
  {
    return TMR_ERROR_INVALID;
  }
  rp = reader->readParams.readPlan;
  ret = TMR_SUCCESS;

  if (tagCount)
  {
    *tagCount = 0;
  }

  /**
   * Allocate memory for bufResponse, to atleast hold one RO_ACCESS_REPORT,
   * Depending on number of reports received, can be reallocated later.
   **/
  if (NULL != reader->u.llrpReader.bufResponse)
  {
    /* free this before leaking the memory */
    for (i = 0; i < reader->u.llrpReader.bufPointer; i++)
    {
      TMR_LLRP_freeMessage(reader->u.llrpReader.bufResponse[i]);
    }
    free(reader->u.llrpReader.bufResponse);
    reader->u.llrpReader.bufResponse=NULL;
  }

  reader->u.llrpReader.tagsRemaining = 0;
  reader->u.llrpReader.bufPointer = 0;
  reader->u.llrpReader.bufIndex = 0;
 
  reader->u.llrpReader.bufResponse = (LLRP_tSMessage **) malloc( 1 * sizeof(LLRP_tSMessage *));
  reader->u.llrpReader.pTagReportData = NULL;

  ret = TMR_LLRP_cmdGetTMMetadataFlag(reader, (uint16_t *)&reader->u.llrpReader.metadata);
  if (TMR_SUCCESS != ret)
  {
    reader->u.llrpReader.metadata = TMR_TRD_METADATA_FLAG_ALL;
  }
  if(reader->u.llrpReader.featureFlags & TMMP_READER_FEATURES_FLAG_PERANTENNA_ONTIME)
  {
    isPerAntennaEnabled = true;
  }
   
  /**
   * Enable EventsAndReports on the reader, so that the
   * reader flushes RO_ACCESS_REPORTS that are being hold at
   * the reader, (either due to connection loss or momentary pause)
   **/
  ret = TMR_LLRP_enableEventsAndReports(reader);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * DELETE_ROSPECs
   * Delete all ROSpecs, so we don't have to worry about the reader's
   * prior configuration
   **/
  ret = TMR_LLRP_cmdDeleteAllROSpecs(reader, true);
  /*FIXME:
   * If there are no rospecs on reader, it will throw an exception
   * Do we really need to care about the error here?
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }*/
  /**
   * DELETE_ACCESSSPECs
   * Delete all AccessSpecs, so we don't have to worry about reader's
   * prior configuration
   **/
  ret = TMR_LLRP_cmdDeleteAllAccessSpecs(reader);
  /**
   * FIXME: do we really need to care about the error here?
   **/

  if (!reader->continuousReading)
  {
    /**
     * Only for sync reads we depend on
     * Reader event notifications.
     **/
    /* Enable Reader Event Notifications */
    ret = TMR_LLRP_cmdSetEventNotificationSpec(reader, true);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
  }

  /**
   * Minimum number of ROSpec events to wait for
   **/
  reader->u.llrpReader.numOfROSpecEvents = 1;

  ret = TMR_LLRP_read_internal(reader, timeoutMs, rp);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  if (!reader->continuousReading)
  {
    /**
     * In case of sync read, verify read operation
     * i.e., wait until all tags received.
     **/
    ret = TMR_LLRP_verifyReadOperation(reader, tagCount);
    if (TMR_SUCCESS != ret)
    {
#ifdef TMR_ENABLE_BACKGROUND_READS      
      notify_exception_listeners(reader, ret);
#endif
      return ret;
    }

    /**
     * Only for sync reads we depend on
     * Reader event notifications.
     **/
    /* Disable Reader Event Notifications */
    ret = TMR_LLRP_cmdSetEventNotificationSpec(reader, false);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
  }

  return ret;
}

TMR_Status
TMR_LLRP_hasMoreTags(TMR_Reader *reader)
{
  TMR_LLRP_LlrpReader *lr;
  TMR_Status ret;

  if (NULL == reader)
  {
    return TMR_ERROR_INVALID;
  }
  lr = &reader->u.llrpReader;
  ret = TMR_SUCCESS;

#ifdef TMR_ENABLE_BACKGROUND_READS
  if (reader->continuousReading)
  {
    const LLRP_tSTypeDescriptor   *pType;
    int timeout;

    timeout = lr->searchTimeoutMs;

    if (true == reader->u.llrpReader.get_report)
    {
    /**
       * Force request the reader to send report.
       **/
      ret = TMR_LLRP_cmdGetReport(reader);
      if (TMR_SUCCESS == ret)
      {
        reader->u.llrpReader.get_report = false;
      }
    }

    /**
     * In case of continuous reading, user calling hasMoreTags
     * should free the lr->bufResponse which contains the response.
     **/
    pthread_mutex_lock(&reader->u.llrpReader.receiverLock);
    ret = TMR_LLRP_receiveMessage(reader, &lr->bufResponse[0], timeout);
    pthread_mutex_unlock(&reader->u.llrpReader.receiverLock);
    if (TMR_SUCCESS != ret)
    {
      TMR_LLRP_freeMessage(lr->bufResponse[0]);
      
      if (TMR_ERROR_LLRP_RECEIVEIO_ERROR == ret)
      {
        uint64_t diffTime;

        reader->u.llrpReader.ka_now = tmr_gettime();
        diffTime = reader->u.llrpReader.ka_now - reader->u.llrpReader.ka_start;
        /* Count the number of KEEP_ALIVEs missed */
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
            return TMR_ERROR_LLRP_READER_CONNECTION_LOST;
          }
        }
      }
      return ret;
    }

    if (NULL != lr->bufResponse[0])
    {
      pType = lr->bufResponse[0]->elementHdr.pType;
      
      /* Now we are treating every message received from the host as a Keep Alive and 
       * resetting the "ka_start" timestamp to avoid Mercury API 
       * client closing the connection as it is missing Keep Alives
       */
      reader->u.llrpReader.ka_start = tmr_gettime();

      /**
       * If the response is a tag report
       **/ 
      if (&LLRP_tdRO_ACCESS_REPORT == pType)
      {
        reader->isStatusResponse = false;
        reader->u.llrpReader.reportReceived = true;

        if(NULL != lr->bufResponse[0]->elementHdr.listAllSubParameters)
        {
          if (&LLRP_tdRFSurveyReportData == (lr->bufResponse[0]->elementHdr.listAllSubParameters->elementHdr.pType))
          {
            reader->isStatusResponse = true;
          }
        }
        return TMR_SUCCESS;
      }
      /**
       * There can also be a reader event, 
       * if the response is a reader event, handle it
       **/
      else if(&LLRP_tdREADER_EVENT_NOTIFICATION == pType)
      {
        /**
         * We generally do not expect events while in continuous
         * reading. But this is a place to handle reader exceptions
         * when they are supported.
         *
         * TODO: Events like reader expection and buffer full errors
         * need to be informed to the user.
         * Handle reader event notifications.
         **/
        TMR_LLRP_handleReaderEvents(reader, lr->bufResponse[0]);

        /**
         * No need for error checking and return no tags error
         * as this is event notification message, not a tag report.
         **/
        return TMR_ERROR_NO_TAGS;
      }
      /**
       * Handle keep alives
       **/
      else if (&LLRP_tdKEEPALIVE == pType)
      {
        /**
         * Keep alive is received. i.e., reader is still alive
         * Reset the ka_start time.
         **/
        pthread_mutex_lock(&reader->u.llrpReader.receiverLock);
        reader->u.llrpReader.keepAliveAckMissCnt = 0;
        pthread_mutex_unlock(&reader->u.llrpReader.receiverLock);

        /* handle keepalive messages. */
        TMR_LLRP_handleKeepAlive(reader, lr->bufResponse[0]);

        /**
         * If TagReport is not received between two keepalives,
         * There might not be enough tags to send. Demand
         * for tags by sending GET_REPORT message
         **/
        /* if (false == reader->u.llrpReader.reportReceived)
        {
          reader->u.llrpReader.get_report = true;
        }*/
        reader->u.llrpReader.reportReceived = false;

        return TMR_ERROR_NO_TAGS;
      }
      /**
       * If STOP_ROSPEC_RESPONSE is received,
       * then indicate end_of_search
       **/
      else if ((&LLRP_tdSTOP_ROSPEC_RESPONSE == pType) ||
               (&LLRP_tdDELETE_ROSPEC_RESPONSE == pType))
      {
        reader->finishedReading = true;
        TMR_LLRP_freeMessage(lr->bufResponse[0]);
        free(lr->bufResponse);
        lr->bufResponse = NULL;
        return TMR_ERROR_END_OF_READING;
      }
      else
      {
        /**
         * Unknown response type, store the ,message
         * it may be intent by some other event
         **/
        reader->u.llrpReader.unhandledAsyncResponse.lMsg = reader->u.llrpReader.bufResponse[0];
        reader->u.llrpReader.isResponsePending = true;

        return TMR_ERROR_NO_TAGS;
      }
    }
  }
  else
#endif
  {
    /**
     * TMR_LLRP_hasMoreTags control comes here only in case of sync reads.
     * LLRP reader will not have pseudo-async mode.
     **/
    ret = (lr->tagsRemaining > 0) ? TMR_SUCCESS : TMR_ERROR_NO_TAGS;

    if (TMR_SUCCESS == ret)
    {
      /* If TagReportData is not null, loop through the list */
      if (NULL != lr->pTagReportData)
      {
        lr->pTagReportData = (LLRP_tSTagReportData *)lr->pTagReportData->hdr.pNextSubParameter;
      }

      /**
       * pTagReportData->hdr.pNextSubParameter can be null when there
       * are no more tags. i.e., we are done with this RO_ACCESS_REPORT.
       * Move to next one.
       **/
      if (NULL == lr->pTagReportData)
      {
        /**
       * For the very first time, and in the above case
         * when TagReportData is null, Iterate through
       * next message in bufResponse.
       **/
        LLRP_tSRO_ACCESS_REPORT *pReport;
    
        pReport = (LLRP_tSRO_ACCESS_REPORT *)lr->bufResponse[lr->bufIndex];
        lr->pTagReportData = pReport->listTagReportData;
          lr->bufIndex ++;
        }
    }
    else
    {
      /**
       * At this point it is assured that all tags in all RO_ACCESS_REPORTS
       * are processed. Free RO_ACCESS_REPORTS that were buffered.
       **/
      uint8_t i;

      for (i = 0; i < lr->bufIndex; i ++)
      {
        TMR_LLRP_freeMessage(lr->bufResponse[i]);
      }
      
      /**
       * No more RO_ACCESS_REPORTS left, safe to free the bufResponse array
       * which was allocated in TMR_LLRP_read().
       **/
      free(lr->bufResponse);
      lr->bufResponse = NULL;
    }
  }
    return ret;
}

TMR_Status
TMR_LLRP_getNextTag(TMR_Reader *reader, TMR_TagReadData *data)
{
  TMR_LLRP_LlrpReader *lr;
  TMR_Status ret;
  LLRP_tSMessage *pMsg;

  if (NULL == reader)
  {
    return TMR_ERROR_INVALID;
  }
  lr = &reader->u.llrpReader;
  ret = TMR_SUCCESS;

  pMsg = lr->bufResponse[lr->bufIndex];

  if (0 == lr->tagsRemaining)
  {
    return TMR_ERROR_NO_TAGS;
  }

  if (reader->continuousReading)
  {
    ret = TMR_LLRP_hasMoreTags(reader);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
  }
  
  /**
   * Initialize TagReadData structure
   **/
  TMR_TRD_init(data);

  /* Parse the response message */
  ret = TMR_LLRP_parseMetadataFromMessage(reader, data, lr->pTagReportData);
  if (TMR_SUCCESS != ret)
  {
    TMR_LLRP_freeMessage(pMsg);
    return ret;
  }

  if (reader->continuousReading)
  {
    /**
     * In case of continuous reading, we get a tag report
     * for every tag read, so free every message received
     **/
    TMR_LLRP_freeMessage(pMsg);
  }
  else
  {
    lr->tagsRemaining --;
  }

  data->reader = reader;
  return ret;
}

/**
 * Execute Individual tag operation
 * 
 * This method provides the direct execution of TagOp commands.
 * * Reader operates on the first tag found, with applicable tag filtering.
 * * Reader stops and the call returns immediately after finding one tag
 *   and operating on it, unless the command timeout expires first
 * * The operation is performed on the antenna specified in 
 *   /reader/tagop/antenna parameter.
 * * /reader/tagop/protocol specifies the protocol to be used.
 *
 * @param reader Reader pointer
 * @param tagop Pointer to the TMR_TagOp which needs to be executed
 * @param filter Tag Filter to be used
 * @param data Data returned as a result of tag operation
 */
TMR_Status
TMR_LLRP_executeTagOp(TMR_Reader *reader, TMR_TagOp *tagop, 
                TMR_TagFilter *filter, TMR_uint8List *data)
{
  TMR_Status ret;
  TMR_LLRP_LlrpReader *lr;
  TMR_TagProtocol protocol;
  int timeout;
  uint64_t start, end, difftime;

  if (NULL == reader)
  {
    return TMR_ERROR_INVALID;
  }
  if(reader->u.llrpReader.featureFlags & TMMP_READER_FEATURES_FLAG_PERANTENNA_ONTIME)
  {
    isPerAntennaEnabled = true;
  }
  else
  {
    isPerAntennaEnabled = false;
  }
  lr = &reader->u.llrpReader;
  ret = TMR_SUCCESS;
  isStandaloneTagop = true;
 
  reader->u.llrpReader.bufPointer = 0;
  /**
   * Allocate memory for bufResponse, to atleast hold one RO_ACCESS_REPORT,
   * We only expect one RO_ACCESS_REPORT in case of standalone tag operation.
   **/
  if (NULL != reader->u.llrpReader.bufResponse)
  {
    /* free this before leaking the memory */
    free(reader->u.llrpReader.bufResponse);
    reader->u.llrpReader.bufResponse=NULL;
  }
  reader->u.llrpReader.bufResponse = (LLRP_tSMessage **) malloc(1 * sizeof(LLRP_tSMessage *));


  /**
   * Though its a standalone tag operation, From server point of view
   * we need to submit requests in the following order.
   *
   * 1. Reset reader
   * 2. Add ROSpec with the filter specified
   * 3. Enable ROSpec
   * 4. Add AccessSpec
   * 5. Enable AccessSpec
   * 6. Start ROSpec
   * 7. Wait for response and verify the result
   **/

  /**
   * 1. Reset Reader
   * Delete all ROSpec and AccessSpecs on the reader, so that
   * we don't have to worry about the prior configuration
   * No need to verify the error status.
   **/
  TMR_LLRP_cmdDeleteAllROSpecs(reader, true);
  TMR_LLRP_cmdDeleteAllAccessSpecs(reader);

  /**
   * 2. Add ROSpec
   * 3. Enable ROSpec
   * These two are performed by TMR_LLRP_cmdPrepareROSpec method
   **/
  {
    uint8_t antenna[1];
    TMR_uint8List antennaList;

    /**
     * prepare antennaList
     * The operation has to be performed on the antenna specified
     * in the /reader/tagop/antenna parameter
     **/
    antenna[0] = reader->tagOpParams.antenna;
    antennaList.len = 1;
    antennaList.max = 1;
    antennaList.list = antenna;
    /**
     * Protocol to use is specified in /reader/tagop/protocol
     **/
    if ((TMR_TAGOP_ISO180006B_READDATA == tagop->type) || (TMR_TAGOP_ISO180006B_WRITEDATA == tagop->type)
                                          ||(TMR_TAGOP_ISO180006B_LOCK == tagop->type))
    {
      protocol = TMR_TAG_PROTOCOL_ISO180006B;
    }
    else
    {
      protocol = reader->tagOpParams.protocol;
    }

    /**
     * Prepare ROSpec
     **/
    lr->roSpecId ++;
    /* timeout = 0, as it has no significance in this case */
    ret = TMR_LLRP_cmdPrepareROSpec(reader, 0, &antennaList, filter, protocol);
    if (TMR_SUCCESS != ret)
    {
      return ret;
    }
  }

  /**
   * 4. Add AccessSpec
   **/
  lr->accessSpecId ++;
  /**
   * Filter is still sent as NULL, as there is no support for it
   * and the filter sent as part of ROSpec is considered.
   *
   * isStandalone is set to true, since it is standalone tag operation
   **/
  ret = TMR_LLRP_cmdAddAccessSpec(reader, protocol, NULL,
                                lr->roSpecId, tagop, true);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * 5. Enable AccessSpec
   **/
  ret = TMR_LLRP_cmdEnableAccessSpec(reader, lr->accessSpecId);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * 6. Start ROSpec
   **/
  ret = TMR_LLRP_cmdStartROSpec(reader, lr->roSpecId);
  if (TMR_SUCCESS != ret)
  {
    return ret;
  }

  /**
   * Wait for the response.
   * In case of standalone tag operation, we expect only one
   * RO_ACCESS_REPORT with tagop result.
   **/
  timeout = lr->commandTimeout + lr->transportTimeout;
  start = tmr_gettime();
  while (true)
  {
    /**
     * LLRP background receiver will be receiving the messages.
     * If RO_ACCESS_REPORT is received lr->bufPointer is incremented.
     * Wait until lr->bufPointer is not zero
     **/
    if (lr->bufPointer)
    {
      break;
    }
    end = tmr_gettime();
    difftime = end - start;
    if (difftime > timeout)
    {
      /**
       * We have waited for enough time, but still the message
       * isn't received. There could be some problem with the network.
       * We can't wait forever, throw timeout error to the user
       **/
      return TMR_ERROR_TIMEOUT;
    }
  }

  /**
   * Now, we might have received RO_ACCESS_REPORT
   **/
  {
    LLRP_tSRO_ACCESS_REPORT *pReport;
    LLRP_tSTagReportData *pTagReportData;

    pReport = (LLRP_tSRO_ACCESS_REPORT *)lr->bufResponse[0];
    pTagReportData = pReport->listTagReportData;

    if (NULL != pTagReportData)
    {
      LLRP_tSParameter *pOpSpec;

      /**
       * We don't need to process the rest of the data in 
       * TagReportData. We only care about OpSpecResult parameter.
       **/
      pOpSpec = pTagReportData->listAccessCommandOpSpecResult;
      /* Verify the OpSpecResult status */
      ret = TMR_LLRP_verifyOpSpecResultStatus(reader, pOpSpec);
      if (TMR_SUCCESS != ret)
      {
        /**
         * Tag operation is failed to execute.
         * Return the error.
         **/
        return ret;
      }
      /**
       * Tag operation is success. Extract tagop data.
       **/
      if (NULL != data)
      {
        TMR_LLRP_parseTagOpSpecData(pOpSpec, data);
      }
    }

    /**
     * Done with the response. Free allocated memory
     **/
    {
      uint8_t i;

      for (i = 0; i < lr->bufPointer; i ++)
      {
        TMR_LLRP_freeMessage(lr->bufResponse[i]);
      }
      free(lr->bufResponse);
      lr->bufResponse = NULL;
    }
  }
  return ret;
}

TMR_Status
TMR_LLRP_gpiGet(struct TMR_Reader *reader, uint8_t *count,
                         TMR_GpioPin state[])
{
  TMR_Status ret;
  if (NULL == reader)
  {
    return TMR_ERROR_INVALID;
  }
  ret = TMR_LLRP_cmdGetGPIState(reader, count, state);
  return ret;
}

TMR_Status
TMR_LLRP_gpoSet(struct TMR_Reader *reader, uint8_t count,
                      const TMR_GpioPin state[])
{
  TMR_Status ret;
  if (NULL == reader)
  {
    return TMR_ERROR_INVALID;
  }
  ret = TMR_LLRP_cmdSetGPOState(reader, count, state);
  return ret;
}

TMR_Status
TMR_LLRP_readTagMemBytes(struct TMR_Reader *reader, const TMR_TagFilter *filter,
                         uint32_t bank, uint32_t address,
                         uint16_t count,uint8_t data[])
{
  TMR_Status ret;
  /*
   * currently in fixed reader this feature is
   * not supported
   */
  ret = TMR_ERROR_UNSUPPORTED;
  return ret;
}

TMR_Status
TMR_LLRP_readTagMemWords(struct TMR_Reader *reader, const TMR_TagFilter *filter,
                         uint32_t bank, uint32_t address,
                         uint16_t count, uint16_t *data)
{
  TMR_Status ret;
  /*
   * currently in fixed reader this feature is
   * not supported
   */
  ret = TMR_ERROR_UNSUPPORTED;
  return ret;
}

TMR_Status
TMR_LLRP_writeTagMemBytes(struct TMR_Reader *reader, const TMR_TagFilter *filter,
                         uint32_t bank, uint32_t address,
                         uint16_t count,const uint8_t data[])
{
  TMR_Status ret;
  /*
   * currently in fixed reader this feature is
   * not supported
   */
  ret = TMR_ERROR_UNSUPPORTED;
  return ret;
}

TMR_Status
TMR_LLRP_writeTagMemWords(struct TMR_Reader *reader, const TMR_TagFilter *filter,
                         uint32_t bank, uint32_t address,
                         uint16_t count, const uint16_t *data)
{
  TMR_Status ret;
  /*
   * currently in fixed reader this feature is
   * not supported
   */
  ret = TMR_ERROR_UNSUPPORTED;
  return ret;
}

TMR_Status
TMR_LLRP_firmwareLoad( TMR_Reader *reader, void *cookie,
                      TMR_FirmwareDataProvider provider)
{
  TMR_Status ret;
  /*
   * currently in fixed reader this feature is
   * not supported
   */
  ret = TMR_ERROR_UNSUPPORTED;
  return ret;
}

TMR_Status
TMR_LLRP_reboot(TMR_Reader *reader)
{
  TMR_Status ret;

  if (NULL == reader)
  {
    return TMR_ERROR_INVALID;
  }
  ret = TMR_LLRP_cmdrebootReader(reader);

  return ret;
}

#if 0
TMR_Status
TMR_LLRP_resetHoptable(TMR_Reader *reader)
{
  TMR_Status ret;

  if (NULL == reader)
  {
    return TMR_ERROR_INVALID;
  }
  ret = TMR_LLRP_cmdResetHoptable(reader);

  return ret;
}
#endif

TMR_Status
TMR_LLRP_writeTag( TMR_Reader *reader, const TMR_TagFilter *filter,
                  const TMR_TagData *data)
{
  TMR_Status ret;
  TMR_TagOp tagop;
  ret = TMR_TagOp_init_GEN2_WriteTag(&tagop, (TMR_TagData *)data);
  ret = TMR_executeTagOp(reader, &tagop, (TMR_TagFilter *)filter, NULL);
  if (TMR_SUCCESS != ret)
  {
    /**
     * Tag operation is failed to execute.
     * Return the error.
     **/
    return ret;
  }
  return ret;
}

TMR_Status
TMR_LLRP_killTag(struct TMR_Reader *reader, const TMR_TagFilter *filter,
                 const TMR_TagAuthentication *auth)
{
  TMR_Status ret;
  TMR_TagOp tagop;
  TMR_TagOp_init_GEN2_Kill(&tagop, auth->u.gen2Password);
  ret= TMR_executeTagOp(reader,&tagop, (TMR_TagFilter *)filter, NULL);
  if (TMR_SUCCESS != ret)
  {
    /**
     * Tag operation is failed to execute.
     * Return the error.
     **/
    return ret;
  }
  return ret;

}

TMR_Status
TMR_LLRP_lockTag(struct TMR_Reader *reader,const TMR_TagFilter *filter,
                 TMR_TagLockAction *action)
{
  TMR_Status ret;
  TMR_TagOp tagop;
  TMR_TagOp_init_GEN2_Lock(&tagop, action->u.gen2LockAction.mask, action->u.gen2LockAction.action, 
      reader->u.llrpReader.gen2AccessPassword);
  ret= TMR_executeTagOp(reader,&tagop, (TMR_TagFilter *)filter, NULL);
  if (TMR_SUCCESS != ret)
  {
    /**
     * Tag operation is failed to execute.
     * Return the error.
     **/
    return ret;
  }
  return ret;
}
#endif /* TMR_ENABLE_LLRP_TRANSPORT  */

