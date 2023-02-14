/**
 *  @file tmr_loadsave_configuration.c
 *  @brief Mercury API - load save configuration implementation
 *  @author Lingaraj Bal
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
#include <stdio.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdlib.h>
#if !defined(WIN32) && !defined(WINCE)
#include <libgen.h>
#endif

#include "tm_reader.h"
#include "tmr_utils.h"
#include "serial_reader_imp.h"

#if defined(WIN32) || defined(WINCE)
#define strncasecmp _strnicmp
#endif /* WIN32 */


#define CONFIG_MAX_BUFFER_LEN 1500
#define CONFIG_MAX_PARAMETERS_LEN 100

typedef struct properties
{
  char keyConfig[CONFIG_MAX_BUFFER_LEN];
  char valueConfig[CONFIG_MAX_BUFFER_LEN];
}properties;

typedef enum paramOption
{
  PARAM_OPTION_GET = 0x00,
  PARAM_OPTION_SET = 0x01,
}paramOption;

/* Private: Should not be used by user level application. */
#ifdef TMR_ENABLE_UHF
static const char *regions[] = {"UNSPEC", "NA", "EU", "KR", "IN", "JP", "PRC",
                                "EU2", "EU3", "KR2", "PRC2", "AU", "NZ", "NA2", "NA3", "IS", "MY", "ID", "PH", "TW", "MO", "RU", "SG", "JP2", "JP3", "VN", "TH", "AR", "HK", "BD","EU4","UNIVERSAL","IS2", "NA4"};
static const char *powerModes[] = {"FULL", "MINSAVE", "MEDSAVE", "MAXSAVE", "SLEEP"};
static const char *hexChars = "0123456789abcdefABCDEF";
static const char *selectOptionNames[] = {"EQ", "NE", "GT", "LT"};
static const char *tagEncodingNames[] = {"FM0", "M2", "M4", "M8"};
static const char *sessionNames[] = {"S0", "S1", "S2", "S3"};
static const char *targetNames[] = {"A", "B", "AB", "BA"};
static const char *tariNames[] = {"TARI_25US", "TARI_12_5US", "TARI_6_25US"};
static const char *modeNames[] = {"CONTINUOUS", "TIMED"};
static const char *modulationNames[] = {NULL, "CW","PRBS"};
static const char *gen2LinkFrequencyNames[] = {"LINK250KHZ", "LINK300KHZ", "LINK320KHZ", "LINK40KHZ", "LINK640KHZ"};
static const char *bankNames[] = {"Reserved", "EPC", "TID", "User"};
static const char *tagopNames[] = {NULL, "ReadData"};
#endif /* TMR_ENABLE_UHF */
static const char *protocolNames[] = {NULL, NULL, NULL, "ISO180006B", NULL, "GEN2", "ISO180006B_UCODE", "IPX64", "IPX256", "ISO14443A", "ISO14443B", "ISO15693", "ISO18092",
                                      "FELICA", "ISO18000_3M3", NULL, NULL, NULL, NULL, NULL, "LF125KHZ", "LF134KHZ", NULL, NULL, NULL, NULL, NULL, NULL, NULL, "ATA"};
#ifdef TMR_ENABLE_HF_LF
static const char *ISO1443ATagtypeNames[] = {"AUTO_DETECT", "MIFARE_PLUS", "MIFARE_ULTRALIGHT", "MIFARE_CLASSIC", "NTAG", "MIFARE_DESFIRE", "MIFARE_MINI", "ULTRALIGHT_NTAG"};
static const char *ISO15693TagtypeNames[] = {"AUTO_DETECT", "HID_ICLASS_SE", "ICODE_SLI", "ICODE_SLI_L", "ICODE_SLI_S", "ICODE_DNA",
                                             "ICODE_SLIX", "ICODE_SLIX_L", "ICODE_SLIX_S", "ICODE_SLIX_2", "VIGO", "TAGIT", "PICOPASS"};
static const char *ISO1443BTagtypeNames[] = {"AUTO_DETECT", "CALYSO", "CALYPSO_INNOVATRON_PROTOCOL", "CTS",
                                             "MONEO", "PICO_PASS", "SRI4K", "SRIX4K", "SRI512", "SRT512"};
static const char *LF125KHZTagtypeNames[] = {"AUTO_DETECT", "HID_PROX", "AWID", "HITAG_1", "HITAG_2", "EM_4100", "KERI", "INDALA"};
static const char *LF134KHZTagtypeNames[] = {"AUTO_DETECT"};
#endif /* TMR_ENABLE_HF_LF */
static TMR_Status readLine(FILE *fp, char *buffer);
static TMR_Status isParamWritable(TMR_Param key);
static int parseU32List(const char *arg, TMR_uint32List *list, int *nchars);
static int parseU8List(const char *arg, TMR_uint8List *list, int *nchars);
static int parseReadPlan(struct TMR_Reader *reader, const char *arg, TMR_ReadPlan *plan, int *nchars);
static void printU8List(TMR_String *string, TMR_uint8List *list);
static void printU32List(TMR_String *string, TMR_uint32List *list);
static void printReadPlan(const char *string, TMR_ReadPlan *plan, int *nchars);
static bool strcasecmpcount(const char *a, const char *b, int *matches);
const char *listname(const char *list[], int listlen, unsigned int id);
static int listid(const char *list[], int listlen, const char *name);
static TMR_Status getSetOneParam(struct TMR_Reader *reader, const char *paramName, TMR_String *string, paramOption option);
static TMR_Status loadConfiguration(struct TMR_Reader *reader, char *filePath, properties *dict, uint8_t noOfEntries, bool isRollBack);
static TMR_Status rollBackConfigData(struct TMR_Reader *reader, char *filePath);
static int parseFilter(const char *arg, TMR_TagFilter **filterp, int *nchars);
static int parseTagop(const char *arg, TMR_TagOp **tagopp, int *nchars);
#ifdef TMR_ENABLE_UHF
static const char *regionName(TMR_Region region);
static TMR_Region regionID(const char *name);
static void logErrorMessage(struct TMR_Reader *reader, char *errorString);
static int parsePortValueList(const char *arg, TMR_PortValueList *list, int *nchars);
static int parseEpc(const char *arg, TMR_TagData *tag, int *nchars);
static void printPortValueList(TMR_String *string, TMR_PortValueList *list);
static void printFilter(TMR_String *string, TMR_TagFilter *filter);
static void printTagop(TMR_String *string, TMR_TagOp *tagop);
static int parseInitQ(const char *arg, TMR_GEN2_initQ *value);
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_HF_LF
static void printTagtype(TMR_String *string, TMR_TagType *tagType);
static int parseTagtype(const char *arg, TMR_TagType *tagType, int *nchars);
#endif /* TMR_ENABLE_HF_LF*/

static void
logErrorMessage(struct TMR_Reader *reader, char *errorString)
{
  if(TMR_READER_TYPE_SERIAL == reader->readerType)
  {
    sprintf(reader->u.serialReader.errMsg, "%s", errorString);
  }
#ifdef TMR_ENABLE_LLRP_READER
  else
  {
    sprintf(reader->u.llrpReader.errMsg,  "%s", errorString);
  }
#endif
}

static bool
strcasecmpcount(const char *a, const char *b, int *matches)
{
  int i = 0;

  while (*a && *b && 
    (tolower((unsigned char)*a) == tolower((unsigned char)*b)))
  {
    i++;
    a++;
    b++;
  }
  if (matches)
  {
    *matches = i;
  }
  return (*a == *b);
}

static 
int listid(const char *list[], int listlen, const char *name)
{
  int i, len, match, bestmatch, bestmatchindex;

  bestmatch = 0;
  bestmatchindex = -1;
  len = (int)strlen(name);
  for (i = 0; i < listlen; i++)
  {
    if (NULL != list[i])
    {
      if (strcasecmpcount(name, list[i], &match))
      {
        return i; /* Exact match - return immediately */
      }
      if (match == len)
      {
        if (bestmatch == 0)
        {
          /* Prefix match - return if nothing else conflicts */
          bestmatch = match;
          bestmatchindex = i;
        }
        else
        {
          /* More than one prefix match of the same length - ambiguous */
          bestmatchindex = -1;
        }
      }
    }
  }
  return bestmatchindex;
}

const char *listname(const char *list[], int listlen, unsigned int id)
{
  if ((id < (unsigned)listlen) && list[id] != NULL)
  {
    return (list[id]);
  }
  return ("Unknown");
}
#ifdef TMR_ENABLE_HF_LF
static void printTagtype(TMR_String *string, TMR_TagType *tagType)
{
  uint8_t listlen, i;
  char *end;
  uint64_t tempTagtType;

  end = string->value;
  end += sprintf(string->value, "%c", '[');
  if(tagType != NULL)
  {
    switch (tagType->tagProtocol)
    {
      case TMR_TAG_PROTOCOL_ISO14443A:
      {
        listlen = numberof(ISO1443ATagtypeNames);
        tempTagtType = tagType->u.iso14443a;

        for (i = 0; i < listlen; i++)
        {
          if ((1 << i) & tagType->u.iso14443a)
          {
            tempTagtType ^= (uint64_t)(1 << i);
            end += sprintf(end, "%s", listname(ISO1443ATagtypeNames, listlen, i));
            end += sprintf(end, "%s",(tempTagtType ? ", " : ""));
          }
        }
        break;
      }
      case TMR_TAG_PROTOCOL_ISO14443B:
      {
        listlen = numberof(ISO1443BTagtypeNames);
        tempTagtType = tagType->u.iso14443b;

        for (i = 0; i < listlen; i++)
        {
          if ((1 << i) & tagType->u.iso14443b)
          {
            tempTagtType ^= (uint64_t)(1 << i);
            end += sprintf(end, "%s", listname(ISO1443BTagtypeNames, listlen, i));
            end += sprintf(end, "%s",(tempTagtType ? ", " : ""));
          }
        }
        break;
      }
      case TMR_TAG_PROTOCOL_ISO15693:
      {
        listlen = numberof(ISO15693TagtypeNames);
        tempTagtType = tagType->u.iso15693;

        for (i = 0; i < listlen; i++)
        {
          if ((1 << i) & tagType->u.iso15693)
          {
            tempTagtType ^= (uint64_t)(1 << i);
            end += sprintf(end, "%s", listname(ISO15693TagtypeNames, listlen, i));
            end += sprintf(end, "%s",(tempTagtType ? ", " : ""));
          }
        }
        break;
      }
      case TMR_TAG_PROTOCOL_LF125KHZ:
      {
        listlen = numberof(LF125KHZTagtypeNames);
        tempTagtType = tagType->u.lf125khz;

        for (i = 0; i < listlen; i++)
        {
          if ((1 << i) & tagType->u.lf125khz)
          {
            tempTagtType ^= (uint64_t)(1 << i);
            end += sprintf(end, "%s", listname(LF125KHZTagtypeNames, listlen, i));
            end += sprintf(end, "%s",(tempTagtType ? ", " : ""));
          }
        }
        break;
      }
      case TMR_TAG_PROTOCOL_LF134KHZ:
      {
        listlen = numberof(LF134KHZTagtypeNames);
        tempTagtType = tagType->u.lf134khz;

        for (i = 0; i < listlen; i++)
        {
          if ((1 << i) & tagType->u.lf134khz)
          {
            tempTagtType ^= (uint64_t)(1 << i);
            end+= sprintf(end, "%s", listname(LF134KHZTagtypeNames, listlen, i));
            end += sprintf(end, "%s",(tempTagtType ? ", " : ""));
          }
        }
        break;
      }
      default:
        break;
    }
  }
  end += sprintf(end, "%c", ']');
}
#endif /* TMR_ENABLE_HF_LF */
#ifdef TMR_ENABLE_UHF
static const char *regionName(TMR_Region region)
{
  if (region == TMR_REGION_OPEN)
  {
    return "OPEN";
  }
  return listname(regions, numberof(regions), region);
}

static TMR_Region regionID(const char *name)
{

  if (0 == strcasecmp(name, "OPEN"))
  {
    return TMR_REGION_OPEN;
  }

  return listid(regions, numberof(regions), name);
}
#endif /* TMR_ENABLE_UHF */

/* Helper function to read a new line from the file */
static TMR_Status 
readLine(FILE *fp, char *buffer)
{
  int c;
  TMR_Status ret = TMR_SUCCESS;  
  int i = 0x00;

  do
  {
    c = fgetc(fp);
    if (feof(fp))
    {
      /* encountered EOF */
      buffer[i] = (char)c;
      ret = TMR_ERROR_TRYAGAIN;
      break;
    }
    else
    {
      if ('\n' != c)
      {
        if (32 != c)
        {
          /* remove spaces if any and then copy */
          buffer[i] = (char) c;
          i++;
        }
        else
        {
          continue;
        }
      }
      else
      {
        /* one line read */
        ret = TMR_SUCCESS;
        break;
      }
    }
  }while(i < CONFIG_MAX_BUFFER_LEN);

  /* check for buffer over flow */
  if (CONFIG_MAX_BUFFER_LEN == i)
  {
    ret = TMR_ERROR_OUT_OF_MEMORY;
  }
return ret;
}

static TMR_Status
isParamWritable(TMR_Param key)
{
  TMR_Status ret;

  ret = TMR_SUCCESS;

  switch(key)
  {
  case TMR_PARAM_URI:
  case TMR_PARAM_PRODUCT_GROUP_ID:
  case TMR_PARAM_PRODUCT_GROUP:
  case TMR_PARAM_PRODUCT_ID:
  case TMR_PARAM_TAGREADATA_TAGOPSUCCESSCOUNT:
  case TMR_PARAM_TAGREADATA_TAGOPFAILURECOUNT:
  case TMR_PARAM_RADIO_POWERMAX:
  case TMR_PARAM_RADIO_POWERMIN:
  case TMR_PARAM_REGION_SUPPORTEDREGIONS:
  case TMR_PARAM_ANTENNA_PORTLIST:
  case TMR_PARAM_ANTENNA_CONNECTEDPORTLIST:
  case TMR_PARAM_VERSION_SUPPORTEDPROTOCOLS:
  case TMR_PARAM_RADIO_TEMPERATURE:
  case TMR_PARAM_VERSION_HARDWARE:
  case TMR_PARAM_VERSION_MODEL:
  case TMR_PARAM_VERSION_SOFTWARE:
  case TMR_PARAM_VERSION_SERIAL:
#ifdef TMR_ENABLE_UHF
  case TMR_PARAM_ANTENNA_RETURNLOSS:
  case TMR_PARAM_CURRENTTIME:
  case TMR_PARAM_GEN2_PROTOCOLEXTENSION:
    /* Following parameters are not read only in Mercury API, but we decided
     * not to support them as part for LoadSaveConfiguration. Hence adding them as
     * read only parameters. */
  case TMR_PARAM_LICENSE_KEY:
  case TMR_PARAM_READER_STATS:
  case TMR_PARAM_USER_CONFIG:
  case TMR_PARAM_READER_STATISTICS:
  case TMR_PARAM_REGION_LBT_ENABLE:
  case TMR_PARAM_REGION_LBT_THRESHOLD:
  case TMR_PARAM_REGION_DWELL_TIME:
  case TMR_PARAM_REGION_DWELL_TIME_ENABLE:
  case TMR_PARAM_RADIO_ENABLEPOWERSAVE:
  case TMR_PARAM_GEN2_BAP:
  case TMR_PARAM_ISO180006B_MODULATION_DEPTH:
  case TMR_PARAM_ISO180006B_DELIMITER:
  case TMR_PARAM_ISO180006B_BLF:
  case TMR_PARAM_RADIO_ENABLESJC:
  case TMR_PARAM_USERMODE:
  case TMR_PARAM_GEN2_WRITEMODE:
  case TMR_PARAM_READER_WRITE_EARLY_EXIT:
  case TMR_PARAM_READER_WRITE_REPLY_TIMEOUT:
  case TMR_PARAM_READER_DESCRIPTION:
  case TMR_PARAM_READER_HOSTNAME:
  case TMR_PARAM_MANAGE_LICENSE_KEY:
  case TMR_PARAM_LICENSED_FEATURES:
  case TMR_PARAM_SELECTED_PROTOCOLS:
  case TMR_PARAM_REGULATORY_ENABLE:
  case TMR_PARAM_REGION_MINIMUM_FREQUENCY:
  case TMR_PARAM_REGION_QUANTIZATION_STEP:
  case TMR_PARAM_READER_STATS_ENABLE:
#endif /* TMR_ENABLE_UHF */
  case TMR_PARAM_METADATAFLAG:
  case TMR_PARAM_TAGOP_ANTENNA:
#ifdef TMR_ENABLE_HF_LF
  case TMR_PARAM_ISO14443A_SUPPORTED_TAGTYPES:
  case TMR_PARAM_ISO14443B_SUPPORTED_TAGTYPES:
  case TMR_PARAM_ISO15693_SUPPORTED_TAGTYPES:
  case TMR_PARAM_LF125KHZ_SUPPORTED_TAGTYPES:
  case TMR_PARAM_LF134KHZ_SUPPORTED_TAGTYPES:
  case TMR_PARAM_ISO14443A_SUPPORTED_TAG_FEATURES:
  case TMR_PARAM_ISO15693_SUPPORTED_TAG_FEATURES:
  case TMR_PARAM_LF125KHZ_SUPPORTED_TAG_FEATURES:
  case TMR_PARAM_LF125KHZ_SECURE_RD_FORMAT:
  case TMR_PARAM_RADIO_KEEP_RF_ON:
  case TMR_PARAM_PROTOCOL_LIST:
#endif /* TMR_ENABLE_HF_LF */
    {
      ret = TMR_ERROR_READONLY;
      break;
    }
  default:
    ret = TMR_SUCCESS;
  }
  return ret;
}

/*
 * Tag operations (for embedding)
 *  Read:   gen2.read:EPC,2,2 (word address and count)
 */
static int
parseTagop(const char *arg, TMR_TagOp **tagopp, int *nchars)
{
#ifdef TMR_ENABLE_UHF
  TMR_TagOp *tagop;
  const char *orig;
  int id;
  char buf[100];
  int charcount;
#endif /* TMR_ENABLE_UHF */

  if (0 == strncasecmp(arg, "NULL", 4))
  {
    *tagopp = NULL;
    if (nchars)
    {
      *nchars = 4;
    }
    return 0;
  }
  else
  {
    return -1;
  }
#ifdef TMR_ENABLE_UHF
  orig = arg;
  if (1 != sscanf(arg, "%[^]:]%n", buf, &charcount))
  {
    return -1;
  }

  tagop = malloc(sizeof(*tagop));
  id = listid(tagopNames, numberof(tagopNames), buf);
  if (-1 == id)
  {
    free(tagop);
    return -1;
  }

  arg += charcount;
  tagop->type = id;

  switch (tagop->type)
  {
    case TMR_TAGOP_GEN2_READDATA:
      {
        TMR_GEN2_Bank bank;
        int address,  wordLen;

        bank = 0x00;
        address = 0x00;
        wordLen = 0x00;

        arg++;
        while (']' != *arg)
        {
          arg++;
          if (1 != sscanf(arg, "%[^]=]%n", buf, &charcount))
          {
            return -1;
          }

          if (0x00 == strcmp("Bank", buf))
          {
            arg += charcount;
            arg++;
            
            if (1 != sscanf(arg, "%[^],]%n", buf, &charcount))
            {
              return -1;
            }

            arg += charcount;
            id = listid(bankNames, numberof(bankNames), buf);
            if (-1 == id)
            {
              free(tagop);
              return -1;
            }
            bank = id;
          }
          else if (0x00 == strcmp("WordAddress", buf))
          {
            arg += charcount;
            arg++;
            if (1 != sscanf(arg, "%[^],]%n", buf, &charcount))
            {
              return -1;
            }
            arg += charcount;
            address = atol(buf);
          }
          else if (0x00 == strcmp("Len", buf))
          {
            arg += charcount;
            arg++;
            if (1 != sscanf(arg, "%[^]]%n", buf, &charcount))
            {
              return -1;
            }
            arg += charcount;
            wordLen = atol(buf);
          }
        }
        TMR_TagOp_init_GEN2_ReadData(tagop, bank, address, wordLen);
        break;
      }
    default:
      {
        /* Currently supporting only Gen2 Read Data */
        free(tagop);
        return -1;
      }
  }

  if (NULL != nchars)
  {
    *nchars = (int)(arg - orig);
  }

  *tagopp = tagop;
  return 0;
#endif /* TMR_ENABLE_UHF */
}

#ifdef TMR_ENABLE_HF_LF
static int 
parseTagtype(const char *arg, TMR_TagType *tagType, int *nchars)
{
  const char *orig;
  char buf[100];
  int charcount, id;
  uint32_t type = 0;

  orig = arg;

  while (']' != *arg)
  {
    arg++;
    if (1 != sscanf(arg, "%[^],]%n", buf, &charcount))
    {
      return -1;
    }
    switch(tagType->tagProtocol)
    {
      case TMR_TAG_PROTOCOL_ISO14443A:
      {
        id = listid(ISO1443ATagtypeNames, numberof(ISO1443ATagtypeNames), buf);
        if (-1 == id)
        {
          return -1;
        }
        type |= (1 << id);
        tagType->u.iso14443a = type;
        break;
      }
      case TMR_TAG_PROTOCOL_ISO14443B:
      {
        id = listid(ISO1443BTagtypeNames, numberof(ISO1443BTagtypeNames), buf);
        if (-1 == id)
        {
          return -1;
        }
        type |= (1 << id);
        tagType->u.iso14443b = type;
        break;
      }
      case TMR_TAG_PROTOCOL_ISO15693:
      {
        id = listid(ISO15693TagtypeNames, numberof(ISO15693TagtypeNames), buf);
        if (-1 == id)
        {
          return -1;
        }
        type |= (1 << id);
        tagType->u.iso15693 = type;
        break;
      }
      case TMR_TAG_PROTOCOL_LF125KHZ:
      {
        id = listid(LF125KHZTagtypeNames, numberof(LF125KHZTagtypeNames), buf);
        if (-1 == id)
        {
          return -1;
        }
        type |= (1 << id);
        tagType->u.lf125khz = type;
        break;
      }
      case TMR_TAG_PROTOCOL_LF134KHZ:
      {
        id = listid(LF134KHZTagtypeNames, numberof(LF134KHZTagtypeNames), buf);
        if (-1 == id)
        {
          return -1;
        }
        type |= (1 << id);
        tagType->u.lf134khz = type;
        break;
      }
      default:
        break;
    }
    arg += charcount;
  }

  if (NULL != nchars)
    *nchars = (int)(arg - orig);

  return type;
}
#endif /* TMR_ENABLE_HF_LF */

/* Parse strings of the form [1,2,3,4] into a TMR_uint32List */
static int
parseU32List(const char *arg, TMR_uint32List *list, int *nchars)
{
  int value;
  int chars;
  const char *origarg;

  origarg = arg;
  if ('[' != arg[0])
  {
    return -1;
  }

  arg++;
  list->len = 0;
  list->max = 4;
  list->list = malloc(sizeof(list->list[0]) * list->max);

  while (1 == sscanf(arg, "%i%n", &value, &chars))
  {
    if (list->len+1 > list->max)
    {
      list->list = realloc(list->list, 2 * list->max * sizeof(list->list[0]));
      list->max = 2 * list->max;
    }
    list->list[list->len] = value;
    list->len++;
    arg += chars;
    if (',' == *arg)
    {
      arg++;
    }
  }

  if (']' == *arg)
  {
    arg++;
  }
  else
  {
    free(list->list);
    return -1;
  }

  if (nchars)
  {
    *nchars = (int)(arg - origarg);
  }
  return 0;
}

#ifdef TMR_ENABLE_UHF
static int 
parseEpc(const char *arg, TMR_TagData *tag, int *nchars)
{
  static const char epcPrefix[] = "EPC=";
  static int epcPrefixLen = sizeof(epcPrefix) - 1;
  TMR_Status ret;
  int len;

  if (0 != strncasecmp(arg, "TagData:", 8))
  {
    return -1;
  }

  arg += 9;
  if (0 != strncasecmp(arg, epcPrefix, epcPrefixLen))
  {
    return -1;
  }

  arg += epcPrefixLen;
  len = (int)strspn(arg, hexChars) / 2;
  ret = TMR_hexToBytes(arg, tag->epc, len, NULL);
  if (TMR_SUCCESS != ret)
  {
    return -1;
  }
  tag->protocol = TMR_TAG_PROTOCOL_NONE;
  tag->epcByteCount = len;

  if (nchars)
  {
    *nchars = epcPrefixLen + 2 * len;
  }

  return 0;
}
#endif /* TMR_ENABLE_UHF */
/*
 * Tag filters:
 *  Tag ID:      EPC:000011112222333344445555
 *  Gen2 Select: Gen2.Select:~EPC,16,32,DEADBEEF
 *  ISO 18k6B:   Iso18k6b.Select:~EQ,1,ff,001122334455667788
 *               (EQ,NE,GT,LT),address,bytemask,data
 */
static int
parseFilter(const char *arg, TMR_TagFilter **filterp, int *nchars)
{
#ifdef TMR_ENABLE_UHF
  TMR_Status ret;
  TMR_TagFilter *filter;
  static const char gen2Prefix[] = "Gen2.Select:";
  static int gen2PrefixLen = sizeof(gen2Prefix) - 1;
  static const char iso18k6bPrefix[] = "Iso18k6b.Select:";
  static int iso18k6bPrefixLen = sizeof(iso18k6bPrefix) - 1;

  int len = 0x00;
#endif /* TMR_ENABLE_UHF */

  if (0 == strncasecmp(arg, "NULL", 4))
  {
    *filterp = NULL;
    if (nchars)
    {
      *nchars = 4;
    }
    return 0;
  }
  else
  {
    return -1;
  }
#ifdef TMR_ENABLE_UHF
  filter = malloc(sizeof(*filter));

  if (0 == parseEpc(arg, &filter->u.tagData, nchars))
  {
    filter->type = TMR_FILTER_TYPE_TAG_DATA;
  }
  else if (0 == strncasecmp(arg, gen2Prefix, gen2PrefixLen))
  {
    bool invert = false;
    int bank = 0x00, pointer = 0x00, bitlen = 0x00;
    uint8_t *mask = NULL;
    char numbuf[100];
    const char *orig;
    int charcount = 0x00;

    orig = arg;
    arg += (gen2PrefixLen);
    
    while(']' != *arg)
    {
      arg++;
      if (1 != sscanf(arg, "%[^]=]%n", numbuf, &charcount))
      {
        return -1;
      }
      if (0x00 == strcmp(numbuf, "Invert"))
      {
        arg += charcount;
        arg++;
        if (1 != sscanf(arg, "%[^],]%n", numbuf, &charcount))
        {
          return -1;
        }
        arg += charcount;
        invert = (0 == strcmp(numbuf, "true")) ? true : false;
      }
      else if (0x00 == strcmp(numbuf, "Bank"))
      {
        arg += charcount;
        arg++;
        if (1 != sscanf(arg, "%[^],]%n", numbuf, &charcount))
        {
          return -1;
        }
        arg += charcount;
        bank = listid(bankNames, numberof(bankNames), numbuf);
        if (-1 == bank)
        {
          free(filter);
          return -1;
        }
      }
      else if (0x00 == strcmp(numbuf, "BitPointer"))
      {
        arg += charcount;
        arg++;
        if (1 != sscanf(arg, "%[^],]%n", numbuf, &charcount))
        {
          return -1;
        }
        arg += charcount;
        pointer = atol(numbuf);
      }
      else if (0x00 == strcmp(numbuf, "BitLength"))
      {
        arg += charcount;
        arg++;
        if (1 != sscanf(arg, "%[^],]%n", numbuf, &charcount))
        {
          return -1;
        }
        arg += charcount;
        bitlen = atol(numbuf);
      }
      else if (0x00 == strcmp(numbuf, "Mask"))
      {
        arg += charcount;
        arg++;
        if (1 != sscanf(arg, "%[^]]%n", numbuf, &charcount))
        {
          return -1;
        }
        arg += charcount;
        len = (int)(strspn(numbuf, hexChars) / 2);
        mask = malloc(len);
        ret = TMR_hexToBytes(numbuf, mask, len, NULL);
        if (TMR_SUCCESS != ret)
        {
          free(mask);
          free(filter);
          return -1;
        }
      }
    }
    TMR_TF_init_gen2_select(filter, invert, bank, pointer, bitlen, mask);
    if (nchars)
    {
      *nchars = (int)((arg + 2 * len) - orig);
    }
  }
  else if (0 == strncasecmp(arg, iso18k6bPrefix, iso18k6bPrefixLen))
  {
    bool invert;
    int selectOp, address, mask;
    uint8_t bytes[8];
    char buf[16];
    const char *orig;
    char *next;

    orig = arg;
    arg += iso18k6bPrefixLen;

    if ('~' == *arg)
    {
      invert = true;
      arg++;
    }
  else
  {
      invert = false;
    }

    /* Parse group select option */
    next = strchr(arg, ',');
    if (NULL == next)
    {
      free(filter);
      return -1;
    }

    strncpy(buf, arg, next-arg);
    buf[next-arg] = '\0';
    selectOp = listid(selectOptionNames, numberof(selectOptionNames), buf);
    if (-1 == selectOp)
    {
      free(filter);
      return -1;
    }
    arg = next + 1;

    /* Parse byte address */
    next = strchr(arg, ',');
    if (NULL == next || next - arg > 16)
    {
      free(filter);
      return -1;
    }
    strncpy(buf, arg, next-arg);
    buf[next-arg] = '\0';
    address = strtol(buf, NULL, 16);
    arg = next + 1;

    /* Parse byte mask */
    next = strchr(arg, ',');
    if (NULL == next || next - arg > 16)
    {
      free(filter);
      return -1;
    }
    strncpy(buf, arg, next-arg);
    buf[next-arg] = '\0';
    mask = strtol(buf, NULL, 16);
    arg = next + 1;

    len = (int)strspn(arg, hexChars) / 2;
    if (8 != len)
    {
      free(filter);
      return -1;
    }
    ret = TMR_hexToBytes(arg, bytes, len, NULL);
    if (TMR_SUCCESS != ret)
    {
      free(filter);
      return -1;
    }
#ifdef TMR_ENABLE_ISO180006B    

    TMR_TF_init_ISO180006B_select(filter, invert, selectOp, address, mask,
                                  bytes);
#endif /* TMR_ENABLE_ISO180006B */

    if (nchars)
    {
      *nchars = (int)((arg + 2 * len) - orig);
    }
  }
  else
  {
   free(filter);
    return -1;
  }

  *filterp = filter;
  return 0;
#endif /* TMR_ENABLE_UHF */
}

/* Parse strings of the form [1,2,3,4] into a TMR_uint8List */
static int
parseU8List(const char *arg, TMR_uint8List *list, int *nchars)
{
  TMR_uint32List u32list;
  int i, ret;

  ret = 0;

  if (-1 == parseU32List(arg, &u32list, nchars))
  {
    return -1;
  }

  list->len = u32list.len;
  list->max = u32list.len;
  list->list = malloc(sizeof(list->list[0]) * list->max);
  for (i = 0; i < list->len; i++)
  {
    if (u32list.list[i] > UINT8_MAX)
    {
      ret = -1;
      break;
    }
    list->list[i] = (uint8_t)u32list.list[i];
  }

  free(u32list.list);
  return ret;
}

#ifdef TMR_ENABLE_UHF
/* Parse strings of the form [[1,100],[2,200]] into a
 * TMR_PortValueList structure.
 */
static int
parsePortValueList(const char *arg, TMR_PortValueList *list, int *nchars)
{
  int port, value;
  int chars;
  const char *origarg = arg;

  if ('[' != arg[0]
      || ']' != arg[strlen(arg)-1])
  {
    return -1;
  }

  arg++;

  list->len = 0;
  list->max = 4;
  list->list = malloc(sizeof(list->list[0]) * list->max);

  while (2 == sscanf(arg, "[%i,%i]%*[],]%n", &port, &value, &chars))
  {
    if (list->len+1 > list->max)
    {
      list->list = realloc(list->list, 2 * list->max * sizeof(list->list[0]));
      list->max = 2 * list->max;
    }
    if (port > UINT8_MAX || value > UINT16_MAX)
    {
      return -1;
    }
    list->list[list->len].port = port;
    list->list[list->len].value = value;
    list->len++;
    arg += chars;
  }

  if (nchars)
  {
    *nchars = (int)(arg - origarg);
  }

  return 0;
}
#endif /* TMR_ENABLE_UHF */

static void
printU8List(TMR_String *string, TMR_uint8List *list)
{
  int i;
  char *end;

  end = string->value;
  end += sprintf(string->value, "%c", '[');
  for (i = 0; i < list->len && i < list->max; i++)
  {
    end += sprintf(end, "%u%s", list->list[i], ((i + 1) == list->len) ? "" : ",");
  }
  if (list->len > list->max)
  {
    end += sprintf(end, "%s", "...");
  }
  end += sprintf(end, "%c", ']');
}

static void
printU32List(TMR_String *string, TMR_uint32List *list)
{
  int i;
  char *end;

  end = string->value;
  end += sprintf(string->value, "%c", '[');
  for (i = 0; i < list->len && i < list->max; i++)
  {
    end += sprintf(end, "%"PRIu32"%s", list->list[i], ((i + 1) == list->len) ? "" : ",");
  }
  if (list->len > list->max)
  {
    end += sprintf(end, "%s", "...");
  }
  end += sprintf(end, "%c", ']');
}

#ifdef TMR_ENABLE_UHF
static void 
printPortValueList(TMR_String *string, TMR_PortValueList *list)
{
  int i;
  char *end;

  end = string->value;
  end += sprintf(string->value, "%c", '[');
  for (i = 0; i < list->len && i < list->max; i++)
  {
    end += sprintf(end, "[%u,%u]%s", list->list[i].port, list->list[i].value,
      ((i + 1) == list->len) ? "" : ",");
  }
  if (list->len > list->max)
  {
    end += sprintf(end, "%s", "...");
  }
  end += sprintf(end, "%c", ']');
}

static void 
printFilter(TMR_String *string, TMR_TagFilter *filter)
{
  char tmpStr[128];
  char *end;

  end = string->value;
  if (NULL == filter)
  {
    end += sprintf(end, "%s", ",Filter=null");
  }
  else if (TMR_FILTER_TYPE_TAG_DATA == filter->type)
  {
    TMR_bytesToHex(filter->u.tagData.epc, filter->u.tagData.epcByteCount, tmpStr);
    end += sprintf(end, ",Filter=TagData:[EPC=%s]", tmpStr);
  }
  else if (TMR_FILTER_TYPE_GEN2_SELECT == filter->type)
  {
    TMR_bytesToHex(filter->u.gen2Select.mask, (filter->u.gen2Select.maskBitLength + 7) / 8, tmpStr);
    end += sprintf(end, ",Filter=Gen2.Select:[Invert=%s,Bank=%s,BitPointer=%"PRIu32",BitLength=%d,Mask=%s]", 
      filter->u.gen2Select.invert ? "true" : "false",
      listname(bankNames, numberof(bankNames), filter->u.gen2Select.bank),
      filter->u.gen2Select.bitPointer,
      filter->u.gen2Select.maskBitLength,
      tmpStr);
  }
  else
  {
    end += sprintf(end, "%s", "Unknown");
  }
}

static void 
printTagop(TMR_String *string, TMR_TagOp *tagop)
{
  char *end;

  end = string->value;
  if (NULL == tagop)
  {
    end += sprintf(end, "%s", ",Op=null");
  }
  else
  {
    if (TMR_TAGOP_GEN2_READDATA == tagop->type)
    {
      end += sprintf(end, ",Op=ReadData:[Bank=%s,WordAddress=%d,Len=%d]",
        listname(bankNames, numberof(bankNames),tagop->u.gen2.u.readData.bank),
        tagop->u.gen2.u.readData.wordAddress,
        tagop->u.gen2.u.readData.len);
    }
    else
    {
      /* Currently supporting only read data tag op */
      end += sprintf(end, "%s", ",Op=null");
    }
  }
}

static int
parseStats(char *string, TMR_Reader_StatsFlag *stats)
{
  char *arg = NULL;
  
  *stats = TMR_READER_STATS_FLAG_NONE;
  arg = string;

  if (']' == *(arg + 1))
  {
    /* Empty string, return */
    return 0;
  }

  while(']' != *arg)
  {
    /* skip the leading '[' */
    arg++;
    if (0 == strncmp(arg, "NONE", 0x04))
    {
      *stats |= TMR_READER_STATS_FLAG_NONE;
      arg += 0x04;
    }
    else if(0 == strncmp(arg, "ALL", 0x03))
    {
      *stats |= TMR_READER_STATS_FLAG_ALL;
      arg += 0x03;
    }
    else if(0 == strncmp(arg, "RFONTIME", 0x08))
    {
      *stats |= TMR_READER_STATS_FLAG_RF_ON_TIME;
      arg += 0x08;
    }
    else if(0 == strncmp(arg, "NOISEFLOOR", 0x0A))
    {
      *stats |= TMR_READER_STATS_FLAG_NOISE_FLOOR_SEARCH_RX_TX_WITH_TX_ON;
      arg += 0x0A;
    }
    else if(0 == strncmp(arg, "FREQUENCY", 0x09))
    {
      *stats |= TMR_READER_STATS_FLAG_FREQUENCY;
      arg += 0x09;
    }
    else if(0 == strncmp(arg, "TEMPERATURE", 0x0B))
    {
      *stats |= TMR_READER_STATS_FLAG_TEMPERATURE;
      arg += 0x0B;
    }
    else if(0 == strncmp(arg, "ANTENNA", 0x07))
    {
      *stats |= TMR_READER_STATS_FLAG_ANTENNA_PORTS;
      arg += 0x07;
    }
    else if(0 == strncmp(arg, "PROTOCOL", 0x08))
    {
      *stats |= TMR_READER_STATS_FLAG_PROTOCOL;
      arg += 0x08;
    }
    else if(0 == strncmp(arg, "CONNECTEDANTENNAPORTS", 0x15))
    {
      *stats |= TMR_READER_STATS_FLAG_CONNECTED_ANTENNAS;
      arg += 0x15;
    }
    else
    {
      return -1;
    }
  }
  return 0; 
}

static int
parseMetadata(char *string, TMR_TRD_MetadataFlag *flags)
{
  char *arg = NULL;

  *flags = TMR_TRD_METADATA_FLAG_NONE;
  arg = string;

  if (']' == *(arg + 1))
  {
    /* Empty string, return */
    return 0;
  }

  while(']' != *arg)
  {
    /* skip the leading '[' */
    arg++;
    if (0 == strncmp(arg, "NONE", 0x04))
    {
      *flags |= TMR_TRD_METADATA_FLAG_NONE;
      arg += 0x04;
    }
    else if(0 == strncmp(arg, "ALL", 0x03))
    {
      *flags |= TMR_TRD_METADATA_FLAG_ALL;
      arg += 0x03;
    }
    else if(0 == strncmp(arg, "READCOUNT", 0x09))
    {
      *flags |= TMR_TRD_METADATA_FLAG_READCOUNT;
      arg += 0x09;
    }
    else if(0 == strncmp(arg, "RSSI", 0x04))
    {
      *flags |= TMR_TRD_METADATA_FLAG_RSSI;
      arg += 0x04;
    }
    else if (0 == strncmp(arg, "ANTENNAID", 0x09))
    {
      *flags |= TMR_TRD_METADATA_FLAG_ANTENNAID;
      arg += 0x09;
    }
    else if(0 == strncmp(arg, "FREQUENCY", 0x09))
    {
      *flags |= TMR_TRD_METADATA_FLAG_FREQUENCY;
      arg += 0x09;
    }
    else if (0 == strncmp(arg, "TIMESTAMP", 0x09))
    {
      *flags |= TMR_TRD_METADATA_FLAG_TIMESTAMP;
      arg += 0x09;
    }
    else if(0 == strncmp(arg, "PHASE", 0x05))
    {
      *flags |= TMR_TRD_METADATA_FLAG_PHASE;
      arg += 0x05;
    }
    else if (0 == strncmp(arg, "PROTOCOL", 0x08))
    {
      *flags |= TMR_TRD_METADATA_FLAG_PROTOCOL;
      arg += 0x08;
    }
    else if(0 == strncmp(arg, "GPIO_STATUS", 0x0B))
    {
      *flags |= TMR_TRD_METADATA_FLAG_GPIO_STATUS;
      arg += 0x0B;
    }
    else if (0 == strncmp(arg, "GEN2_Q", 0x06))
    {
      *flags |= TMR_TRD_METADATA_FLAG_GEN2_Q;
      arg += 0x06;
    }
    else if(0 == strncmp(arg, "GEN2_LF", 0x07))
    {
      *flags |= TMR_TRD_METADATA_FLAG_GEN2_LF;
      arg += 0x07;
    }
    else if (0 == strncmp(arg, "GEN2_TARGET", 0x0B))
    {
      *flags |= TMR_TRD_METADATA_FLAG_GEN2_TARGET;
      arg += 0x0B;
    }
    else if (0 == strncmp(arg, "BRAND_IDENTIFIER", 0x10))
    {
      *flags |= TMR_TRD_METADATA_FLAG_BRAND_IDENTIFIER;
      arg += 0x10;
    }
    else
    {
      return -1;
    }
  }
  return 0; 
}
#endif /* TMR_ENABLE_UHF */
static int
parseReadPlan(struct TMR_Reader *reader, const char *arg, TMR_ReadPlan *plan, int *nchars)
{
  TMR_Status ret;
  int charcount;

  if ((0 != strncmp(arg, "MultiReadPlan:", 0x0E)) &&
    (0 != strncmp(arg, "SimpleReadPlan:", 0x0F)))
  {
    return -1;
  }

  ret = TMR_SUCCESS;
  plan->weight = 1; /* Default value */

  if (0 == strncmp(arg, "MultiReadPlan:", 0x0E))
  {
    /* MultiReadPlan */
    const char *subString = NULL;
    TMR_ReadPlan *plans;
    int i, nplans;
    TMR_ReadPlan **planlist;

    subString = (arg + 0x0F);
    nplans = 2;
    plans = malloc(nplans * sizeof(plans[0]));
    i = 0x00;
    while (0 == strncmp(subString, "SimpleReadPlan:", 0x0F))
    {
      if (i == nplans)
      {
        nplans *= 2;
        plans = realloc(plans, nplans * sizeof(plans[0]));        
      }
      ret = parseReadPlan(reader, subString, &plans[i], &charcount);
      subString++;
      if (ret == -1 || (',' != *(subString + charcount) && ']' != *(subString + charcount)))
      {
        free(plans);
        return -1;
      }
      subString += charcount;      
      subString++;
      i++;
    }
    nplans = i;
    planlist = malloc(nplans * sizeof(planlist[0]));
    for (i = 0; i < nplans ; i++)
    {
      planlist[i] = &plans[i];;
    }
    subString--;
    plan->type = TMR_READ_PLAN_TYPE_MULTI;
    plan->u.multi.planCount = nplans;
    plan->u.multi.plans = planlist;
  }
  else
  {
    /* SimpleReadPlan */
    const char *srpSubString = NULL;
    char protocolName[100];
    int tmp;
    srpSubString = arg + 0x0F;
    plan->type = TMR_READ_PLAN_TYPE_SIMPLE;

    /* Initialize the simple read plan with default read pln */
    if (TMR_SR_MODEL_M3E != reader->u.serialReader.versionInfo.hardware[0])
	{
      TMR_RP_init_simple(plan, 0, NULL, TMR_TAG_PROTOCOL_GEN2, 1);
	}
	else
	{
      TMR_RP_init_simple(plan, 0, NULL, TMR_TAG_PROTOCOL_ISO14443A, 1);
	}

    /* skip the leading '[' */
    while (']' != *srpSubString)
    {
      srpSubString++;
      if (1 != sscanf(srpSubString, "%[^]=]%n", protocolName, &charcount))
      {
        return -1;
      }

      if (0x00 == strcmp("Antennas", protocolName))
      {
        srpSubString += charcount;
        /* skip the '=' symbol */
        srpSubString++;
        if (1 != sscanf(srpSubString, "%[^]]]%n", protocolName, &charcount))
        {
          /* there is no antenna value */
        }
        srpSubString += charcount;
        strcat(protocolName, "]");
        if (-1 == parseU8List(protocolName, &plan->u.simple.antennas, &charcount))
        {
          return -1;
        }
      }
      else if(0x00 == strcmp("Protocol", protocolName))
      {
        srpSubString += charcount;
        /* skip the '=' symbol */
        srpSubString++;
        if (1 != sscanf(srpSubString, "%[^],]%n", protocolName, &charcount))
        {
          return -1;
        }
        tmp = listid(protocolNames, numberof(protocolNames), protocolName);
        if (-1 == tmp)
        {
          return -1;
        }
        plan->u.simple.protocol = tmp;
        srpSubString += charcount;
      }
      else if (0x00 == strcmp("Filter", protocolName))
      {
        srpSubString += charcount;
        //*nchars += charcount;
        /* skip the '=' symbol */
        srpSubString++;
        if (TMR_SR_MODEL_M3E != reader->u.serialReader.versionInfo.hardware[0])
		{
		  if (1 != sscanf(srpSubString, "%[^]]]%n", protocolName, &charcount))
          {
            return -1;
          }
		}
		else
		{
          if (1 != sscanf(srpSubString, "%[^],]%n", protocolName, &charcount))
          {
            return -1;
          }
		}

        if (0 != strncasecmp(srpSubString, "NULL", 4))
        {
          srpSubString += charcount;
        }
#ifdef TMR_ENABLE_UHF
        strcat(protocolName, "]");
#endif /* TMR_ENABTMR_ENABLE_UHFLE_M3E_ONLY */
        if (-1 == parseFilter(protocolName, &plan->u.simple.filter, &charcount))
        {
          return -1;
        }
         srpSubString += charcount;
      }
      else if (0x00 == strcmp("Op", protocolName))
      {
        srpSubString += charcount;
        /* skip the '=' symbol */
        srpSubString++;
#ifdef TMR_ENABLE_HF_LF
        if (1 != sscanf(srpSubString, "%[^],]%n", protocolName, &charcount))
#else
        if (1 != sscanf(srpSubString, "%[^]]]%n", protocolName, &charcount))
#endif /* TMR_ENABLE_HF_LF */
        {
          return -1;
        }

        if (0 != strncasecmp(srpSubString, "NULL", 4))
        {
          srpSubString += charcount;
        }
#ifdef TMR_ENABLE_UHF
        strcat(protocolName, "]");
#endif /* TMR_ENABLE_UHF */
        if (-1 == parseTagop(protocolName, &plan->u.simple.tagop, &charcount))
        {
          return -1;
        }
         srpSubString += charcount;
      }
      else if (0x00 == strcmp("UseFastSearch", protocolName))
      {
#ifdef TMR_ENABLE_UHF
        if (TMR_SR_MODEL_M3E != reader->u.serialReader.versionInfo.hardware[0])
		{
          srpSubString += charcount;
          /* skip the '=' symbol */
          srpSubString++;
          if (1 != sscanf(srpSubString, "%[^],]%n", protocolName, &charcount))
          {
            return -1;
          }
          srpSubString += charcount;
          if (0 == strcmp(protocolName, "true"))
          {
            plan->u.simple.useFastSearch = true;
          }
          else
          {
            plan->u.simple.useFastSearch = false;
          }
		}
		else
#endif /* TMR_ENABLE_UHF */
		{
          return -1;
	    }
      }
      else if(0x00 == strcmp("Weight", protocolName))
      {
        srpSubString += charcount;
        /* skip the '=' symbol */
        srpSubString++;
        if (1 != sscanf(srpSubString, "%[^]]%n", protocolName, &charcount))
        {
          return -1;
        }
        srpSubString += charcount;
        sscanf(protocolName, "%"SCNi32"%n", &plan->weight, &charcount);
      }
    }
    if (nchars)
    {
      *nchars = (int)(srpSubString - arg);
    }    
  }
 
  return 0;
}
#ifdef TMR_ENABLE_UHF
static int	
parseInitQ(const char *end, TMR_GEN2_initQ *value)
{
  char name[100];
  int charcount;
  const char *arg;

  arg = end;
  if ('[' != arg[0] || ']' != arg[strlen(arg)-1])
  {
    return -1;
  }

  arg++;
  if(sscanf(arg, "%[^:]%n", name, &charcount))
  {
    if (0x00 == strcmp("qEnable", name))
    {
      arg += charcount + 1;
      sscanf(arg, "%[^,]%n", name, &charcount);
      if(strcmp(name,"false") == 0)
      {
        value->qEnable = 0;
      }
      if(strcmp(name,"true") == 0)
      {
        value->qEnable = 1;
      }
    }
    else
    {
      return -1;
    }
  }
  else
  {
    return -1;
  }
  arg += charcount + 1;
  if(sscanf(arg, "%[^:]%n", name, &charcount))
  {
    if (0x00 == strcmp("initialQ", name))
    {
      arg += charcount + 1;
      sscanf(arg, "%[^]]%n", name, &charcount);
      value->qValue = (uint8_t)atoi(name);
    }
    else
    {
      return -1;
    }
  }
  else
  {
    return -1;
  }
  return 0;
}
#endif /* TMR_ENABLE_UHF */
static void
printReadPlan(const char *string, TMR_ReadPlan *plan, int *nchars)
{
  TMR_String temp;
  char str[100];
  char *end;
  int charRead;

  end = (char *)string;
  temp.max = numberof(str);
  temp.value = str;
  if (TMR_READ_PLAN_TYPE_MULTI == plan->type)
  {
    int i;
    end += sprintf(end, "%s", "MultiReadPlan:[");
    for (i = 0 ; i < plan->u.multi.planCount; i++)
    {
      printReadPlan(end, plan->u.multi.plans[i], &charRead);
      end += charRead;
      if (i + 1 !=  plan->u.multi.planCount)
      {
        end += sprintf(end, "%c", ',');
      }
    }
    end += sprintf(end, "%c", ']');
  }
  else
  {
    end += sprintf(end, "%s", "SimpleReadPlan:[");
    end += sprintf(end, "%s", "Antennas=");
    printU8List(&temp, &plan->u.simple.antennas);
    end += sprintf(end, "%s", temp.value);
    end += sprintf(end, "%s%s", ",Protocol=", listname(protocolNames, numberof(protocolNames), plan->u.simple.protocol));
#ifdef TMR_ENABLE_UHF
    printFilter(&temp, plan->u.simple.filter);
    end += sprintf(end, "%s", temp.value);
    printTagop(&temp, plan->u.simple.tagop);
    end += sprintf(end, "%s", temp.value);
    end += sprintf(end, "%s%s", ",UseFastSearch=", plan->u.simple.useFastSearch ? "true" : "false");
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_HF_LF
    end += sprintf(end, "%s", ",Filter=null");
    end += sprintf(end, "%s", ",Op=null");
#endif /* TMR_ENABLE_HF_LF */
    end += sprintf(end, "%s%"PRIu32"]", ",Weight=", plan->weight);
    if (nchars)
    {
      *nchars = (int)(end - string);
    }
  }
}

#ifdef TMR_ENABLE_UHF
static void
printReaderStats(const char *string, TMR_Reader_StatsFlag *stats)
{
  char *end;

  end = (char *)string;

  end += sprintf(end, "%s", "[");
  switch (*stats)
  {
  case TMR_READER_STATS_FLAG_NONE:
    end += sprintf(end, "%s", "NONE,");
    break;
  
  case TMR_READER_STATS_FLAG_ALL:
    end += sprintf(end, "%s", "ALL,");
    break;
  
  default:
    {
      if (TMR_READER_STATS_FLAG_ANTENNA_PORTS & (*stats))
      {
        end += sprintf(end, "%s", "ANTENNA,");
      }
      if(TMR_READER_STATS_FLAG_CONNECTED_ANTENNAS & (*stats))
      {
        end += sprintf(end, "%s", "CONNECTEDANTENNAPORTS,");
      }
      if(TMR_READER_STATS_FLAG_FREQUENCY & (*stats))
      {
        end += sprintf(end, "%s", "FREQUENCY,");
      }
      if(TMR_READER_STATS_FLAG_NOISE_FLOOR_SEARCH_RX_TX_WITH_TX_ON & (*stats))
      {
        end += sprintf(end, "%s", "NOISEFLOOR,");
      }
      if(TMR_READER_STATS_FLAG_PROTOCOL & (*stats))
      {
        end += sprintf(end, "%s", "PROTOCOL,");
      }
      if(TMR_READER_STATS_FLAG_RF_ON_TIME & (*stats))
      {
        end += sprintf(end, "%s", "RFONTIME,");
      }
      if(TMR_READER_STATS_FLAG_TEMPERATURE & (*stats))
      {
        end += sprintf(end, "%s", "TEMPERATURE,");
      }
      break;
    }
  }
  end += sprintf((end -1), "%s", "]");
}

static void
printMetadataFlags(const char *string, TMR_TRD_MetadataFlag *flags)
{
  char *end;

  end = (char *)string;

  end += sprintf(end, "%s", "[");
  switch (*flags)
  {
  case TMR_TRD_METADATA_FLAG_NONE:
    end += sprintf(end, "%s", "NONE,");
    break;
  
  case TMR_TRD_METADATA_FLAG_ALL:
    end += sprintf(end, "%s", "ALL,");
    break;
  
  default:
    {
      if (TMR_TRD_METADATA_FLAG_READCOUNT & (*flags))
      {
        end += sprintf(end, "%s", "READCOUNT,");
      }
      if(TMR_TRD_METADATA_FLAG_RSSI & (*flags))
      {
        end += sprintf(end, "%s", "RSSI,");
      }
      if(TMR_TRD_METADATA_FLAG_ANTENNAID & (*flags))
      {
        end += sprintf(end, "%s", "ANTENNAID,");
      }
      if(TMR_TRD_METADATA_FLAG_FREQUENCY & (*flags))
      {
        end += sprintf(end, "%s", "FREQUENCY,");
      }
      if(TMR_TRD_METADATA_FLAG_TIMESTAMP & (*flags))
      {
        end += sprintf(end, "%s", "TIMESTAMP,");
      }
      if(TMR_TRD_METADATA_FLAG_PHASE & (*flags))
      {
        end += sprintf(end, "%s", "PHASE,");
      }
      if(TMR_TRD_METADATA_FLAG_PROTOCOL & (*flags))
      {
        end += sprintf(end, "%s", "PROTOCOL,");
      }
      if(TMR_TRD_METADATA_FLAG_GPIO_STATUS & (*flags))
      {
        end += sprintf(end, "%s", "GPIO_STATUS,");
      }
      if(TMR_TRD_METADATA_FLAG_GEN2_Q & (*flags))
      {
        end += sprintf(end, "%s", "GEN2_Q,");
      }
      if(TMR_TRD_METADATA_FLAG_GEN2_LF & (*flags))
      {
        end += sprintf(end, "%s", "GEN2_LF,");
      }
      if(TMR_TRD_METADATA_FLAG_GEN2_TARGET & (*flags))
      {
        end += sprintf(end, "%s", "GEN2_TARGET,");
      }
      if(TMR_TRD_METADATA_FLAG_BRAND_IDENTIFIER & (*flags))
      {
        end += sprintf(end, "%s", "BRAND_IDENTIFIER,");
      }
      break;
    }
  }
  end += sprintf((end -1), "%s", "]");
}
#endif /* TMR_ENABLE_UHF */
static TMR_Status
getSetOneParam(struct TMR_Reader *reader, const char *paramName, TMR_String *string, paramOption option)
{
  TMR_Status ret;
  TMR_Param param;
  TMR_String temp;
  char str[CONFIG_MAX_BUFFER_LEN];
  char *end;
#ifdef TMR_ENABLE_UHF
  TMR_SR_SerialReader *sr;
  sr = &reader->u.serialReader;
#endif /* TMT_ENABLE_UHF */
 
  ret = TMR_SUCCESS;
  end = string->value;
  temp.max = CONFIG_MAX_BUFFER_LEN;
  temp.value = str;

  param = TMR_paramID(paramName);
  if (PARAM_OPTION_GET == option)
  {
    end += sprintf(end, "%s", paramName);
    end += sprintf(end, "%c", '=');
  }
  switch(param)
  {
#ifdef TMR_ENABLE_UHF
  case TMR_PARAM_ANTENNA_PORTSWITCHGPOS:
  case TMR_PARAM_TRIGGER_READ_GPI:
  case TMR_PARAM_GPIO_INPUTLIST:
  case TMR_PARAM_GPIO_OUTPUTLIST:
    {
      if(TMR_READER_TYPE_LLRP != reader->readerType)
      {
        if (PARAM_OPTION_GET == option)
        {
          /* The param get */
          TMR_uint8List value;
          uint8_t valueList[64];

          value.max = numberof(valueList);
          value.list = valueList;

          ret = TMR_paramGet(reader, param, &value);
          if (TMR_SUCCESS == ret)
          {
            printU8List(&temp, &value);
            end += sprintf(end, "%s", temp.value);
          }
        }
        else
        {
          if(TMR_SR_MODEL_M3E == sr->versionInfo.hardware[0])           
          {
            ret = TMR_ERROR_READONLY;
          }
          else
          {
            /* The param set */
            TMR_uint8List value;

            if (-1 == parseU8List(end, &value, NULL))
            {
              char errMsg[256];
              sprintf(errMsg, "Can't parse '%s' as list of 8-bit values", end);
              logErrorMessage(reader, errMsg);
              notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
              return TMR_ERROR_INVALID;
            }
            ret = TMR_paramSet(reader, param, &value);
            free(value.list);
          }
        }
      }
      else
      {
         ret = TMR_ERROR_READONLY;
      }
      break;
    }
#endif /* TMR_ENABLE_UHF */
  case TMR_PARAM_BAUDRATE:
  case TMR_PARAM_COMMANDTIMEOUT:
  case TMR_PARAM_TRANSPORTTIMEOUT:
  case TMR_PARAM_READ_ASYNCOFFTIME:
  case TMR_PARAM_READ_ASYNCONTIME:
#ifdef TMR_ENABLE_UHF
  case TMR_PARAM_REGION_HOPTIME:
  case TMR_PARAM_GEN2_T4:
  case TMR_PARAM_REGULATORY_ONTIME:
  case TMR_PARAM_REGULATORY_OFFTIME:
#endif /* TMR_ENABLE_UHF */
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        uint32_t value;
        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          end += sprintf(end, "%u", value);
        }
      }
      else
      {
        /* The param set */
        uint32_t value;
        int scans;
        scans = sscanf(end, "%"SCNi32, &value);
        if (1 != scans)
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as a 32-bit hex value", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        ret = TMR_paramSet(reader, param, &value);
      }
      break;
    }
#ifdef TMR_ENABLE_UHF
  case TMR_PARAM_ANTENNA_CHECKPORT:
  case TMR_PARAM_TAGREADDATA_RECORDHIGHESTRSSI:
  case TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA:
  case TMR_PARAM_TAGREADDATA_UNIQUEBYDATA:
  case TMR_PARAM_STATUS_ENABLE_ANTENNAREPORT:
  case TMR_PARAM_STATUS_ENABLE_FREQUENCYREPORT:
  case TMR_PARAM_STATUS_ENABLE_TEMPERATUREREPORT:
  case TMR_PARAM_TAGREADDATA_UNIQUEBYPROTOCOL:
  case TMR_PARAM_GEN2_SEND_SELECT:
#endif /* TMR_ENABLE_UHF */
  case TMR_PARAM_TAGREADDATA_ENABLEREADFILTER:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        bool value;
        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          end += sprintf(end, "%s", value ? "true" : "false");
        }
      }
      else
      {
        /* The param set */
        bool value;
        if (0 == strcasecmp(end, "true"))
        {
          value = true;
        }
        else if (0 == strcasecmp(end, "false"))
        {
          value = false;
        }
        else
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as boolean value", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        ret = TMR_paramSet(reader, param, &value);
      }
      break;
    }
#ifdef TMR_ENABLE_UHF
  case TMR_PARAM_GEN2_ACCESSPASSWORD:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        TMR_GEN2_Password value;
        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          end += sprintf(end, "%x", value);
        }        
      }
      else
      {
        /* The param set */
        TMR_GEN2_Password value;
        int scans;

        scans = sscanf(end, "%"SCNx32, &value);
        if (1 != scans)
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as a 32-bit hex value", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        ret = TMR_paramSet(reader, param, &value);
      }
      break;
    }
  case TMR_PARAM_GEN2_TARI:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        TMR_GEN2_Tari value;
        
        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          end += sprintf(end, "%s", listname(tariNames, numberof(tariNames), value));
        }
      }
      else
      {
        TMR_GEN2_Tari value;
        int i;

        i = listid(tariNames, numberof(tariNames), end);
        if (i == -1)
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as a Gen2 Tari value", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        value = i;
        ret = TMR_paramSet(reader, param, &value);
      }
      break;
    }
  case TMR_PARAM_GEN2_TAGENCODING:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        TMR_GEN2_TagEncoding value;

        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          end += sprintf(end, "%s", listname(tagEncodingNames, numberof(tagEncodingNames), value));
        }
      }
      else
      {
        /* The param set */
        TMR_GEN2_TagEncoding value;
        int i;

        i = listid(tagEncodingNames, numberof(tagEncodingNames), end);
        if (i == -1)
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as a Gen2 Miller M value", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        value = i;
        ret = TMR_paramSet(reader, param, &value);
      }
      break;
    }
  case TMR_PARAM_GEN2_SESSION:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        TMR_GEN2_Session value;

        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          end += sprintf(end, "%s", listname(sessionNames, numberof(sessionNames), value));
        }
      }
      else
      {
        /* The param set */
        TMR_GEN2_Session value;
        int i;

        i = listid(sessionNames, numberof(sessionNames), end);
        if (i == -1)
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as a Gen2 session name", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        value = i;
        ret = TMR_paramSet(reader, param, &value);
      }
      break;
    }
  case TMR_PARAM_GEN2_TARGET:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        TMR_GEN2_Target value;

        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          end += sprintf(end, "%s", listname(targetNames, numberof(targetNames), value));
        }
      }
      else
      {
        /* The param set */
        TMR_GEN2_Target value;
        int i;

        i = listid(targetNames, numberof(targetNames), end);
        if (i == -1)
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as a Gen2 target name", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        value = i;
        ret = TMR_paramSet(reader, param, &value);
      }
      break;
    }
  case TMR_PARAM_GEN2_Q:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        TMR_SR_GEN2_Q value;

        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          if (value.type == TMR_SR_GEN2_Q_DYNAMIC)
          {
            end += sprintf(end, "%s", "DynamicQ");
          }
          else if (value.type == TMR_SR_GEN2_Q_STATIC)
          {
            end += sprintf(end, "StaticQ(%d)", value.u.staticQ.initialQ);
          }
        }
      }
      else
      {
        /* The param set */
        TMR_SR_GEN2_Q value;
        int initialQ;

        if (0 == strcasecmp("DynamicQ", end))
        {
          value.type = TMR_SR_GEN2_Q_DYNAMIC;
        }
        else if (0 == strncasecmp("StaticQ(", end, 8) 
          && 1 == sscanf(end+8, "%i", &initialQ)
          && initialQ >= 0
          && initialQ <= UINT8_MAX)
        {
          value.type = TMR_SR_GEN2_Q_STATIC;
          value.u.staticQ.initialQ = initialQ;
        }
        else  
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as a Q algorithm", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        ret = TMR_paramSet(reader, param, &value);
      }
      break;
    }
  case TMR_PARAM_GEN2_BLF:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        TMR_GEN2_LinkFrequency value;

        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          int temp = value;
          switch(temp)
          {
            case 250:
              value = 0;
              break;
            case 640:
              value = 4;
              break;
            case 320:
              value = 2;
              break;
            default:;
          }
          end += sprintf(end, "%s", listname(gen2LinkFrequencyNames, numberof(gen2LinkFrequencyNames), value));
        }
      }
      else
      {
        /* The param set */
        TMR_GEN2_LinkFrequency lf;
        TMR_String model;
        char str[64];
        model.value = str;
        model.max = 64;

        lf = listid(gen2LinkFrequencyNames, 
          numberof(gen2LinkFrequencyNames),
          end);
        TMR_paramGet(reader, TMR_PARAM_VERSION_MODEL, &model);
        {
          int temp = lf;
          switch (temp)
          {
          case 0: lf = 250;
            break;
          case 2: lf = 320;
            break;
          case 4: lf = 640;
            break;
          default:;
          }
        }

        if (lf == -1)
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as a Gen2 link frequency", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        ret = TMR_paramSet(reader, param, &lf);
      }
      break;
    }
  case TMR_PARAM_RADIO_READPOWER:
  case TMR_PARAM_RADIO_WRITEPOWER:
  case TMR_PARAM_TAGREADDATA_READFILTERTIMEOUT:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        int32_t value;
        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          end += sprintf(end, "%d", value);
        }        
      }
      else
      {
        /* The param set */
        int32_t value;
        int scans;

        scans = sscanf(end, "%"SCNi32, &value);
        if (1 != scans)
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as a 32bit-bit signed integer value", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        ret = TMR_paramSet(reader, param, &value);
      }
      break;
    }
#endif /* TMR_ENABLE_UHF */
  case TMR_PARAM_PROBEBAUDRATES:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        TMR_uint32List value;
        uint32_t valueList[64];

        value.max = numberof(valueList);
        value.list = valueList;

        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          printU32List(&temp, &value);
          end += sprintf(end, "%s", temp.value);
        }
      }
      else
      {
        /* The param set */
        TMR_uint32List value;

        if (-1 == parseU32List(end, &value, NULL))
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as list of 32-bit values", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        ret = TMR_paramSet(reader, param, &value);
        free(value.list);
      }
      break;
    }
#ifdef TMR_ENABLE_UHF
  case TMR_PARAM_REGION_HOPTABLE:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        TMR_uint32List value;
        uint32_t valueList[64];

        value.max = numberof(valueList);
        value.list = valueList;

        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          printU32List(&temp, &value);
          end += sprintf(end, "%s", temp.value);
        }
      }
      else
      {
        /* The param set */
        TMR_uint32List value;

        if (-1 == parseU32List(end, &value, NULL))
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as list of 32-bit values", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        ret = TMR_paramSet(reader, param, &value);
        free(value.list);
      }
      break;
    }
  case TMR_PARAM_ANTENNA_TXRXMAP:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        TMR_AntennaMapList value;
        TMR_AntennaMap valueList[64];
        int i;

        value.max = numberof(valueList);
        value.list = valueList;

        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          end += sprintf(end, "%c", '[');
          for (i = 0; i < value.len && i < value.max; i++)
          {
            end += sprintf(end, "[%d,%d,%d]%s", value.list[i].antenna, value.list[i].txPort,
              value.list[i].rxPort,((i + 1) == value.len) ? "" : ",");
          }
          if (value.len > value.max)
          {
            end += sprintf(end, "%s", "...");
          }
          end += sprintf(end, "%c", ']');
        }
      }
      else
      {
        /* The param set */
        TMR_AntennaMapList list;
        char *arg;
        int chars, v1, v2, v3;

        arg = end;
        if ('[' != arg[0]
        || ']' != arg[strlen(arg)-1])
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as list of antenna map entries", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        arg++;
        list.len = 0;
        list.max = 4;
        list.list = malloc(sizeof(list.list[0]) * list.max);

        while (3 == sscanf(arg, "[%i,%i,%i]%*[],]%n", &v1, &v2, &v3,
          &chars))
        {
          if (list.len + 1 > list.max)
          {
            list.list = realloc(list.list, 2 * list.max * sizeof(list.list[0]));
            list.max = 2 * list.max;
          }
          list.list[list.len].antenna = v1;
          list.list[list.len].txPort = v2;
          list.list[list.len].rxPort = v3;

          list.len++;
          arg += chars;
        }

        ret = TMR_paramSet(reader, param, &list);
        free(list.list);
      }
      break;
    }
  case TMR_PARAM_ANTENNA_SETTLINGTIMELIST:
  case TMR_PARAM_RADIO_PORTREADPOWERLIST:
  case TMR_PARAM_RADIO_PORTWRITEPOWERLIST:
  case TMR_PARAM_PER_ANTENNA_TIME:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        TMR_PortValueList value;
        TMR_PortValue valueList[64];

        value.max = numberof(valueList);
        value.list = valueList;

        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          printPortValueList(&temp, &value);
          end += sprintf(end, "%s", temp.value);
        }
      }
      else
      {
        /* The param set */
        TMR_PortValueList value;

        if (-1 == parsePortValueList(end, &value, NULL))
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as a list of ports and values", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }

        if (value.len)
        {
          ret = TMR_paramSet(reader, param, &value);
        }
        free(value.list);
      }
      break;
    }

  case TMR_PARAM_REGION_ID:
    {
      if(TMR_READER_TYPE_LLRP != reader->readerType)
      {
        if (PARAM_OPTION_GET == option)
        {
          /* The param get */
          TMR_Region region;

          ret = TMR_paramGet(reader, param, &region);
          if (TMR_SUCCESS == ret)
          {
            end += sprintf(end, "%s", regionName(region));
          }
        }
        else
        {
          if(TMR_SR_MODEL_M3E == sr->versionInfo.hardware[0])
          {
            ret = TMR_ERROR_READONLY;
          }
          else
          {
            /* The param set */
            TMR_Region region;

            region = regionID(end);
            if (TMR_REGION_NONE == region)
            {
              char errMsg[256];
              sprintf(errMsg, "Can't parse '%s' as a region name", end);
              logErrorMessage(reader, errMsg);
              notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
              return TMR_ERROR_INVALID;
            }
            ret = TMR_paramSet(reader, param, &region);
          }
        }
      }
      else
      {
        ret = TMR_ERROR_READONLY; 
      }
      break;
    }
  case TMR_PARAM_POWERMODE:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        TMR_SR_PowerMode mode;

        ret = TMR_paramGet(reader, param, &mode);
        if (TMR_SUCCESS == ret)
        {
          end += sprintf(end, "%s", listname(powerModes, numberof(powerModes), mode));
        }        
      }
      else
      {
        /* The param set */
        TMR_SR_PowerMode mode;
        int i;

        i = listid(powerModes, numberof(powerModes), end);
        if (i == -1)
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as a power mode", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        mode = i;
        ret = TMR_paramSet(reader, param, &mode);
      }
      break;
    }
#endif /* TMR_ENABLE_UHF */
  case TMR_PARAM_TAGOP_PROTOCOL:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        TMR_TagProtocol value;
        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          end += sprintf(end, "%s", listname(protocolNames, numberof(protocolNames), value));
        }
      }
      else
      {
        /* The param set */
        TMR_TagProtocol value;
        int i;

        i = listid(protocolNames, numberof(protocolNames), end);
        if (i == -1)
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as a protocol name", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        value = i;
        ret = TMR_paramSet(reader, param, &value);
      }
      break;
    }
  case TMR_PARAM_READ_PLAN:
    {
      if (PARAM_OPTION_GET == option)
      {        
        /* The param get */
        TMR_ReadPlan value;
        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          printReadPlan(temp.value, &value, NULL);
          end += sprintf(end, "%s", temp.value);
        }
      }
      else
      {
        /* The param set */
        TMR_ReadPlan value;

        if (-1 == parseReadPlan(reader, end, &value, NULL))
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as a read plan", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        ret = TMR_paramSet(reader, param, &value);
      }
      break;
    }
#ifdef TMR_ENABLE_UHF
  case TMR_PARAM_READER_STATS_ENABLE:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        TMR_Reader_StatsFlag value;
        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          printReaderStats(temp.value, &value);
          end += sprintf(end, "%s", temp.value);
        }
      }
      else
      {
        /* The param set */
        TMR_Reader_StatsFlag value;
        if (-1 == parseStats(end, &value))
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as stats flag", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        ret = TMR_paramSet(reader, param, &value);
      }
      break;
    }

  case TMR_PARAM_METADATAFLAG:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        TMR_TRD_MetadataFlag value;
        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          printMetadataFlags(temp.value, &value);
          end += sprintf(end, "%s", temp.value);
        }
      }
      else
      {
        /* The param set */
        TMR_TRD_MetadataFlag value;
        if (-1 == parseMetadata(end, &value))
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as stats flag", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        ret = TMR_paramSet(reader, param, &value);
      }
      break;
  }

  case TMR_PARAM_GEN2_INITIAL_Q:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        TMR_GEN2_initQ value;
        ret = TMR_paramGet(reader, param, &value);
        if (TMR_SUCCESS == ret)
        {
          end += sprintf(end, "[qEnable:%s,initialQ:%d]",(value.qEnable) ? "true" : "false", value.qValue);
        }
      }
      else
      {
        /* The param set */
        TMR_GEN2_initQ value;

        if (-1 == parseInitQ(end, &value))
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as initQ value", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        ret = TMR_paramSet(reader, param, &value);
      }
      break;
    }
  case TMR_PARAM_REGULATORY_MODE:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        TMR_SR_RegulatoryMode value = 0;

        ret = TMR_paramGet(reader, param, &value);
        end += sprintf(end, "%s", listname(modeNames, numberof(modeNames), value));
      }
      else
      {
        /* The param set */
        TMR_SR_RegulatoryMode value;
        int i;

        i = listid(modeNames, numberof(modeNames), end);
        if (i == -1)
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as a regulatory mode value", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        value = i;
        ret = TMR_paramSet(reader, param, &value);
      }
      break;
    }
    case TMR_PARAM_REGULATORY_MODULATION:
    {
      if (PARAM_OPTION_GET == option)
      {
        /* The param get */
        TMR_SR_RegulatoryModulation value = 0;

        ret = TMR_paramGet(reader, param, &value);
        end += sprintf(end, "%s", listname(modulationNames, numberof(modulationNames), value));
      }
      else
      {
        /* The param set */
        TMR_SR_RegulatoryModulation value;
        int i;

        i = listid(modulationNames, numberof(modulationNames), end);
        if (i == -1)
        {
          char errMsg[256];
          sprintf(errMsg, "Can't parse '%s' as a regulatory mode value", end);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          return TMR_ERROR_INVALID;
        }
        value = i;
        ret = TMR_paramSet(reader, param, &value);
      }
      break;
    }
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_HF_LF
  case TMR_PARAM_ISO14443A_TAGTYPE:
  case TMR_PARAM_ISO14443B_TAGTYPE:
  case TMR_PARAM_ISO15693_TAGTYPE:
  case TMR_PARAM_LF125KHZ_TAGTYPE:
  case TMR_PARAM_LF134KHZ_TAGTYPE:
    {
      switch (reader->tagOpParams.protocol)
      {
        case TMR_TAG_PROTOCOL_ISO14443A:
          {
            if(strcmp(paramName, "/reader/iso14443a/tagType"))
            {
              return -1;
            }
            break;
          }
        case TMR_TAG_PROTOCOL_ISO14443B:
          {
            if(strcmp(paramName, "/reader/iso14443b/tagType"))
            {
              return -1;
            }
            break;
          }
        case TMR_TAG_PROTOCOL_ISO15693:
          {
            if(strcmp(paramName, "/reader/iso15693/tagType"))
            {
              return -1;
            }
            break;
          }
        case TMR_TAG_PROTOCOL_LF125KHZ:
          {
            if(strcmp(paramName, "/reader/lf125khz/tagType"))
            {
              return -1;
            }
            break;
          }
        case TMR_TAG_PROTOCOL_LF134KHZ:
          {
            if(strcmp(paramName, "/reader/lf134khz/tagType"))
            {
              return -1;
            }
            break;
          }
        default:
          {
            return -1;
            break;
          }
      }
      {
        TMR_TagType *tagtype = NULL;
        tagtype = malloc(sizeof(* tagtype));
        tagtype->tagProtocol = reader->tagOpParams.protocol;

        if (PARAM_OPTION_GET == option)
        {
          /* The param get */
          uint32_t value;

          ret = TMR_paramGet(reader, param, &value);
          if (TMR_SUCCESS == ret)
          {
            TMR_String temp;
            char str[100];

            temp.max = numberof(str);
            temp.value = str;

            switch (reader->tagOpParams.protocol)
            {
              case TMR_TAG_PROTOCOL_ISO14443A:
                {
                  tagtype->u.iso14443a = (TMR_ISO14443A_TagType)value;
                  break;
                }
              case TMR_TAG_PROTOCOL_ISO14443B:
                {
                  tagtype->u.iso14443b = (TMR_ISO14443B_TagType)value;
                  break;
                }
              case TMR_TAG_PROTOCOL_ISO15693:
                {
                  tagtype->u.iso15693 = (TMR_ISO15693_TagType)value;
                  break;
                }
              case TMR_TAG_PROTOCOL_LF125KHZ:
                {
                  tagtype->u.lf125khz = (TMR_LF125KHZ_TagType)value;
                  break;
                }
              case TMR_TAG_PROTOCOL_LF134KHZ:
                {
                  tagtype->u.lf134khz = (TMR_LF134KHZ_TagType)value;
                  break;
                }
              default:
                  break;
            }

            printTagtype(&temp, tagtype);
            end += sprintf(end, "%s", temp.value);
          }
        }
        else
        {
          /* The param set */
          uint64_t value;
          int charcount;

          value = parseTagtype(end, tagtype, &charcount);
          if (-1 == value)
          {
            char errMsg[256];
            sprintf(errMsg, "Can't parse '%s' as a 32-bit value", end);
            logErrorMessage(reader, errMsg);
            notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
            return TMR_ERROR_INVALID;
          }
          ret = TMR_paramSet(reader, param, &value);
        }
        free(tagtype);
      }
      break;
    }
#endif /* TMR_ENABLE_HF_LF */
  default:
    break;
  }
  return ret;
}

static TMR_Status
rollBackConfigData(struct TMR_Reader *reader, char *filePath)
{
  TMR_Status ret;
  FILE* fp = NULL;
  char temp[CONFIG_MAX_BUFFER_LEN];
  properties dict[CONFIG_MAX_PARAMETERS_LEN];
  uint8_t noOfEntries = 0x00;

  ret = TMR_SUCCESS;
  if (NULL == filePath)
  {
    sprintf(reader->u.serialReader.errMsg, "File path is Empty");
    return TMR_ERROR_TRYAGAIN;
  }

  fp = fopen(filePath, "r");
  if (NULL == fp)
  {
    sprintf(reader->u.serialReader.errMsg, "Error: Unable to open the configuration properties file");
    return TMR_ERROR_TRYAGAIN;
  }
  
  /* read the config parameters from the file and store in key value pair */
  do
  {
    memset(temp, 0, sizeof(temp)/sizeof(temp[0]));
    ret = readLine(fp, temp);
    if (TMR_SUCCESS == ret)
    {
      if (
        (0x00 != strlen(temp)) && (';' != temp[0]) && ('#' != temp[0]) && ('*' != temp[0])
        && (('/' == temp[0]) && ('/' != temp[1]))
        )
      {
        char *token;
        token = strtok(temp, "=");
        if (NULL != token)
        {
          /* copy the key field */
          strcpy(dict[noOfEntries].keyConfig, token);
          /* copy the value field */
          memset(dict[noOfEntries].valueConfig, 0, sizeof(dict[noOfEntries].valueConfig)/sizeof(dict[noOfEntries].valueConfig[0]));
          token = strtok(NULL, "=");
          while (NULL != token)
          {
            /* walk through other tokens */
            strcat(dict[noOfEntries].valueConfig, token);
            token = strtok(NULL, "=");
            if (token)
            {
              strcat(dict[noOfEntries].valueConfig, "=");
            }
          }          
          noOfEntries++;
        }
        else
        {
          /* there is no '=' in the line, skip it and move on to next line */
          continue;
        }
      }
    }
  }while((TMR_SUCCESS == ret) && (CONFIG_MAX_PARAMETERS_LEN > noOfEntries));

  if (!noOfEntries)
  {
    sprintf(reader->u.serialReader.errMsg, "No valid parameter line found");
    ret = TMR_ERROR_INVALID;
  }
  else if(CONFIG_MAX_PARAMETERS_LEN == noOfEntries)
  {
    sprintf(reader->u.serialReader.errMsg, "No having sufficent memory to hold all the parameters");
    ret = TMR_ERROR_OUT_OF_MEMORY;
  }
  else
  {
    /* at least we have some parameters to process, process them now */
    ret = TMR_SUCCESS;
  }

  if (TMR_SUCCESS == ret)
  {
    /* Load the configurations to the module */
    ret = loadConfiguration(reader, filePath, dict, noOfEntries, false);
  }
  return ret;
}


static TMR_Status 
loadConfiguration(struct TMR_Reader *reader, char *filePath, properties *dict, uint8_t noOfEntries, bool isRollBack)
{
  TMR_Status ret;
  FILE *fp = NULL;
  char tempFilePath[200];
  char timeStr[100];
  char *end;
  TMR_TimeStructure time;
  int i;

  ret = TMR_SUCCESS;

  /* Currently creating the back up config file in the current directory for windows */
#ifndef WIN32
  {
    char *dirc = NULL, *dname = NULL;

    dirc = strdup(filePath);
    dname = dirname(dirc);
    if (NULL != dname)
    {
      strcat(dname, "/DeviceConfig");
      strcpy(tempFilePath, dname);
    }
  }
#else
  strcpy(tempFilePath, "DeviceConfig.");
#endif
  end = timeStr;
  time = tmr_gettimestructure();
  end += sprintf(end, "%d%d%d", time.tm_year, time.tm_mon, time.tm_mday);
  end += sprintf(end, "_%d%d%d", time.tm_hour, time.tm_min, time.tm_sec);
  strcat(tempFilePath, timeStr);

  fp = fopen(tempFilePath, "w");
  if (NULL == fp)
  {
    logErrorMessage(reader, "Unable to open the back up file");
    notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
    return TMR_ERROR_TRYAGAIN;
  }
  else
  {
    /* file is created */
    fclose(fp);
  }

  /* Save the reader current configuration for back up */
  ret = TMR_saveConfig(reader, tempFilePath);
  if (TMR_SUCCESS != ret)
  {
    logErrorMessage(reader, "Unable to take back up of parameters");
    notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
    return TMR_ERROR_TRYAGAIN;
  }

  for (i = 0; i < noOfEntries; i++)
  {
    /* load each parameter to module */
    if (0 != strncmp(dict[i].keyConfig, "/reader", 0x07))
    {
      /* Application specific parameter, skip them */
      continue;
    }
    else    
    {
      TMR_String temp;

      /* reader parameter, load it */
      temp.max = CONFIG_MAX_BUFFER_LEN;
      temp.value = dict[i].valueConfig;
      ret = getSetOneParam(reader, dict[i].keyConfig, &temp, PARAM_OPTION_SET);
      if (TMR_SUCCESS != ret)
      {
        if ((TMR_ERROR_READONLY == ret) || (TMR_ERROR_NOT_FOUND == ret) || 
          (TMR_ERROR_UNIMPLEMENTED_FEATURE == ret) || (TMR_ERROR_UNSUPPORTED == ret) || (TMR_ERROR_MSG_INVALID_PARAMETER_VALUE == ret))
        {
          char errMsg[CONFIG_MAX_BUFFER_LEN + 256];
          if(TMR_ERROR_MSG_INVALID_PARAMETER_VALUE != ret)
            sprintf(errMsg, "either '%s'is read only or not supported by reader. Skipping this param", dict[i].keyConfig);
          else
            sprintf(errMsg, "Reader received a valid command with an unsupported or invalid parameter value for '%s'. Skipping this param", dict[i].keyConfig);
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          ret = TMR_SUCCESS;
          continue;
        }
        else
        {
          char errMsg[256];
          sprintf(errMsg, "Rolling back the configurations");
          logErrorMessage(reader, errMsg);
          notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
          if (isRollBack)
          {
            ret = rollBackConfigData(reader, tempFilePath);
            if (TMR_SUCCESS != ret)
            {
              /* Unable to rollback the parameter, skip that */
              continue;
            }
            break;
          }
        }
      }
    }

  }
/* remove the generated file */
remove(tempFilePath);  
return ret;
}

TMR_Status
TMR_saveConfig(struct TMR_Reader *reader, char *filePath)
{
  TMR_Status ret;
  TMR_Param keys[TMR_PARAM_MAX];
  TMR_String paramString;
  char str[CONFIG_MAX_BUFFER_LEN];
  uint32_t i, len;
  FILE* fp = NULL;

  ret = TMR_SUCCESS;
  paramString.max = CONFIG_MAX_BUFFER_LEN;
  paramString.value = str;
  len = numberof(keys);

  /* Open the file requested by user in write mode */
  fp = fopen(filePath, "w");
  if (NULL == fp)
  {
    logErrorMessage(reader, "SaveConfig:Can not open the file");
    notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
    return TMR_ERROR_INVALID;
  }

  /* Get the list of supported parameters by the connected module */
  TMR_paramList(reader, keys, &len);
  for (i = 0; i < len ; i++)
  {
    /* Save the parameters those are writeable */
    if (TMR_SUCCESS == isParamWritable(keys[i]))
    {
      /* Get the current value of the parameter */
      ret = getSetOneParam(reader, TMR_paramName(keys[i]), &paramString, PARAM_OPTION_GET);
      if (TMR_SUCCESS == ret)
      {
        /* Store the data into the file */
        fprintf(fp, "%s\n", paramString.value);
      }
      else
      {
        /* Ignore any error while saving the configuration */
        ret = TMR_SUCCESS;
      }
    }
    else
    {
       /* Parameter is read only, skip it */
      continue;
    }
  }
  fclose(fp);
  return ret;
}

TMR_Status
TMR_loadConfig(struct TMR_Reader *reader, char *filePath)
{
  TMR_Status ret = TMR_SUCCESS;
  FILE* fp = NULL;
  char temp[CONFIG_MAX_BUFFER_LEN];
  properties dict[CONFIG_MAX_PARAMETERS_LEN];
  uint8_t noOfEntries = 0x00;

  if (NULL == filePath)
  {
    logErrorMessage(reader, "File path is Empty");
    notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
    return TMR_ERROR_TRYAGAIN;
  }

  fp = fopen(filePath, "r");
  if (NULL == fp)
  {
    logErrorMessage(reader, "Error: Unable to open the configuration properties file");
    notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
    return TMR_ERROR_TRYAGAIN;
  }
  
  /* read the config parameters from the file and store in key value pair */
  do
  {
    memset(temp, 0, sizeof(temp)/sizeof(temp[0]));
    ret = readLine(fp, temp);
    if (TMR_SUCCESS == ret)
    {
      if (
        (0x00 != strlen(temp)) && (';' != temp[0]) && ('#' != temp[0]) && ('*' != temp[0])
        && (('/' == temp[0]) && ('/' != temp[1]))
        )
      {
        char *token;

        token = strtok(temp, "=");
        if (NULL != token)
        {
          /* copy the key field */
          strcpy(dict[noOfEntries].keyConfig, token);

          /* copy the value field */
          memset(dict[noOfEntries].valueConfig, 0, sizeof(dict[noOfEntries].valueConfig)/sizeof(dict[noOfEntries].valueConfig[0]));
          token = strtok(NULL, "=");
          while (NULL != token)
          {
            /* walk through other tokens */
            strcat(dict[noOfEntries].valueConfig, token);
            token = strtok(NULL, "=");
            if (token)
            {
              strcat(dict[noOfEntries].valueConfig, "=");
            }
          }
          noOfEntries++;
        }
        else
        {
          /* there is no '=' in the line, skip it and move on to next line */
          continue;
        }
      }
    }
  }while((TMR_SUCCESS == ret) && (CONFIG_MAX_PARAMETERS_LEN > noOfEntries));

  if (!noOfEntries)
  {
    logErrorMessage(reader, "No valid parameter line found");
    notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
    ret = TMR_ERROR_INVALID;
  }
  else if(CONFIG_MAX_PARAMETERS_LEN == noOfEntries)
  {
    logErrorMessage(reader, "No having sufficent memory to hold all the parameters");
    notify_exception_listeners(reader, TMR_ERROR_LOADSAVE_CONFIG);
    ret = TMR_ERROR_OUT_OF_MEMORY;
  }
  else
  {
    /* at least we have some parameters to process, process them now */
    ret = TMR_SUCCESS;
  }

  if (TMR_SUCCESS == ret)
  {
    /* Load the configurations to the module */
    ret = loadConfiguration(reader, filePath, dict, noOfEntries, true);
  }
  return ret;
}

