
/*
 ***************************************************************************
 *  Copyright 2007,2008 Impinj, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ***************************************************************************
 */


#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "ltkc_platform.h"
#include "ltkc_base.h"
#include "ltkc_xmltext.h"


/*
 * BEGIN forward declarations
 */

LLRP_tSXMLTextEncoder *
LLRP_XMLTextEncoder_construct (
  unsigned char *               pBuffer,
  unsigned int                  nBuffer);

static void
encoderDestruct (
  LLRP_tSEncoder *              pBaseEncoder);

static void
encodeElement (
  LLRP_tSEncoder *              pBaseEncoder,
  const LLRP_tSElement *        pElement);

static void
putRequiredSubParameter (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const LLRP_tSParameter *      pParameter,
  const LLRP_tSTypeDescriptor * pRefType);

static void
putOptionalSubParameter (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const LLRP_tSParameter *      pParameter,
  const LLRP_tSTypeDescriptor * pRefType);

static void
putRequiredSubParameterList (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const LLRP_tSParameter *      pParameterList,
  const LLRP_tSTypeDescriptor * pRefType);

static void
putOptionalSubParameterList (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const LLRP_tSParameter *      pParameterList,
  const LLRP_tSTypeDescriptor * pRefType);

static void
put_u8 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u8_t               Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_s8 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s8_t               Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_u8v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u8v_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_s8v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s8v_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_u16 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u16_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_s16 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s16_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_u16v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u16v_t             Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_s16v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s16v_t             Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_u32 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u32_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_s32 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s32_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_u32v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u32v_t             Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_s32v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s32v_t             Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_u64 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u64_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_s64 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s64_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_u64v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u64v_t             Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_s64v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s64v_t             Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_u1 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u1_t               Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_u1v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u1v_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_u2 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u2_t               Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_u96 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u96_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_utf8v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_utf8v_t            Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_bytesToEnd (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_bytesToEnd_t       Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_e1 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const int                     Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_e2 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const int                     Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_e8 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const int                     Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_e16 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const int                     Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_e32 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const int                     Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_e8v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u8v_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_enum (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  int                           eValue,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
put_reserved (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  unsigned int                  nBits);

static void
streamConstruct_outermost (
  LLRP_tSXMLTextEncoderStream * pEncoderStream,
  LLRP_tSXMLTextEncoder *       pEncoder);

static void
streamConstruct_nested (
  LLRP_tSXMLTextEncoderStream * pEncoderStream,
  LLRP_tSXMLTextEncoderStream * pEnclosingEncoderStream);

static void
putElement (
  LLRP_tSXMLTextEncoderStream * pEncoderStream,
  const LLRP_tSElement *        pElement);

static int
discoverNamespaces (
  const LLRP_tSElement *        pElement,
  void *                        pArg);

static void
nestSubParameter (
  LLRP_tSXMLTextEncoderStream * pEncoderStream,
  const LLRP_tSParameter *      pParameter);

static void
indent (
  LLRP_tSXMLTextEncoderStream * pEncoderStream,
  int                           adjust);

static void
appendOpenTag (
  LLRP_tSXMLTextEncoderStream * pEncoderStream,
  const char *                  pName);

static void
appendCloseTag (
  LLRP_tSXMLTextEncoderStream * pEncoderStream,
  const char *                  pName);

static void
appendPrefixedTagName (
  LLRP_tSXMLTextEncoderStream * pEncoderStream,
  const char *                  pName);

static void
appendFormat (
  LLRP_tSXMLTextEncoderStream * pEncoderStream,
  char *                        pFmtStr,
                                ...);
/*
 * END forward declarations
 */

/*
 * Structures used by discoverNamespaces() and putElement().
 */
#define MAX_NS      10u
struct NamespaceList
{
    const LLRP_tSNamespaceDescriptor *apNamespaceDescriptor[MAX_NS];
    unsigned int                nNamespaceDescriptor;
};
typedef struct NamespaceList    tNamespaceList;




static LLRP_tSEncoderOps
s_XMLTextEncoderOps =
{
    encoderDestruct,	// .pfDestruct
    encodeElement,	// .pfEncodeElement
};

static LLRP_tSEncoderStreamOps
s_XMLTextEncoderStreamOps =
{
    putRequiredSubParameter,	// .pfPutRequiredSubParameter
    putOptionalSubParameter,	// .pfPutOptionalSubParameter
    putRequiredSubParameterList,	// .pfPutRequiredSubParameterList
    putOptionalSubParameterList,	// .pfPutOptionalSubParameterList

    put_u8,	// .pfPut_u8
    put_s8,	// .pfPut_s8
    put_u8v,	// .pfPut_u8v
    put_s8v,	// .pfPut_s8v

    put_u16,	// .pfPut_u16
    put_s16,	// .pfPut_s16
    put_u16v,	// .pfPut_u16v
    put_s16v,	// .pfPut_s16v

    put_u32,	// .pfPut_u32
    put_s32,	// .pfPut_s32
    put_u32v,	// .pfPut_u32v
    put_s32v,	// .pfPut_s32v

    put_u64,	// .pfPut_u64
    put_s64,	// .pfPut_s64
    put_u64v,	// .pfPut_u64v
    put_s64v,	// .pfPut_s64v

    put_u1,	// .pfPut_u1
    put_u1v,	// .pfPut_u1v
    put_u2,	// .pfPut_u2
    put_u96,	// .pfPut_u96
    put_utf8v,	// .pfPut_utf8v
    put_bytesToEnd,	// .pfPut_bytesToEnd

    put_e1,	// .pfPut_e1
    put_e2,	// .pfPut_e2
    put_e8,	// .pfPut_e8
    put_e16,	// .pfPut_e16
    put_e32,	// .pfPut_e32
    put_e8v,	// .pfPut_e8v

    put_reserved,	// .pfPut_reserved
};


LLRP_tSXMLTextEncoder *
LLRP_XMLTextEncoder_construct (
  unsigned char *               pBuffer,
  unsigned int                  nBuffer)
{
    LLRP_tSXMLTextEncoder *     pEncoder;

    pEncoder = (LLRP_tSXMLTextEncoder *)malloc(sizeof *pEncoder);  //changes
    if(NULL == pEncoder)
    {
        return pEncoder;
    }

    memset(pEncoder, 0, sizeof *pEncoder);

    pEncoder->encoderHdr.pEncoderOps = &s_XMLTextEncoderOps;
    pEncoder->pBuffer = pBuffer;
    pEncoder->nBuffer = nBuffer;
    pEncoder->iNext   = 0;
    pEncoder->bOverflow = 0;

    return pEncoder;
}

static void
encoderDestruct (
  LLRP_tSEncoder *              pBaseEncoder)
{
    LLRP_tSXMLTextEncoder *     pEncoder =
                                    (LLRP_tSXMLTextEncoder *) pBaseEncoder;

    free(pEncoder);
}

static void
encodeElement (
  LLRP_tSEncoder *              pBaseEncoder,
  const LLRP_tSElement *        pElement)
{
    LLRP_tSXMLTextEncoder *     pEncoder =
                                    (LLRP_tSXMLTextEncoder *) pBaseEncoder;
    LLRP_tSXMLTextEncoderStream EncoderStream;

    if(NULL == pElement)
    {
        return;
    }

    streamConstruct_outermost(&EncoderStream, pEncoder);

    putElement(&EncoderStream, pElement);
}

static void
putRequiredSubParameter (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const LLRP_tSParameter *      pParameter,
  const LLRP_tSTypeDescriptor * pRefType)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;

    if(NULL == pParameter)
    {
        appendFormat(pEncoderStream, "warning: missing %s\n",
            (NULL == pRefType) ? "<something>" : pRefType->pName);
        return;
    }

    nestSubParameter(pEncoderStream, pParameter);
}

static void
putOptionalSubParameter (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const LLRP_tSParameter *      pParameter,
  const LLRP_tSTypeDescriptor * pRefType)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;

    nestSubParameter(pEncoderStream, pParameter);
}

static void
putRequiredSubParameterList (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const LLRP_tSParameter *      pParameterList,
  const LLRP_tSTypeDescriptor * pRefType)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const LLRP_tSParameter *    pParameter;

    if(NULL == pParameterList)
    {
        appendFormat(pEncoderStream, "warning: missing list of %s\n",
            (NULL == pRefType) ? "<something>" : pRefType->pName);
        return;
    }

    for(
        pParameter = pParameterList;
        NULL != pParameter;
        pParameter = pParameter->pNextSubParameter)
    {
        nestSubParameter(pEncoderStream, pParameter);
    }
}

static void
putOptionalSubParameterList (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const LLRP_tSParameter *      pParameterList,
  const LLRP_tSTypeDescriptor * pRefType)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const LLRP_tSParameter *    pParameter;

    for(
        pParameter = pParameterList;
        NULL != pParameter;
        pParameter = pParameter->pNextSubParameter)
    {
        nestSubParameter(pEncoderStream, pParameter);
    }
}

static void
put_u8 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u8_t               Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;

    appendOpenTag(pEncoderStream, pFieldName);
    switch(pFieldDescriptor->eFieldFormat)
    {
    case LLRP_FMT_NORMAL:
    case LLRP_FMT_DEC:
    default:
        appendFormat(pEncoderStream, "%u", Value);
        break;

    case LLRP_FMT_HEX:
        appendFormat(pEncoderStream, "%02X", Value);
        break;
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_s8 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s8_t               Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;

    appendOpenTag(pEncoderStream, pFieldName);
    switch(pFieldDescriptor->eFieldFormat)
    {
    case LLRP_FMT_NORMAL:
    case LLRP_FMT_DEC:
    default:
        appendFormat(pEncoderStream, "%d", Value);
        break;

    case LLRP_FMT_HEX:
        appendFormat(pEncoderStream, "%02X", 0xFF & Value);
        break;
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_u8v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u8v_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;
    int                         i;

    appendOpenTag(pEncoderStream, pFieldName);
    for(i = 0; i < Value.nValue; i++)
    {
        switch(pFieldDescriptor->eFieldFormat)
        {
        case LLRP_FMT_DEC:
        case LLRP_FMT_NORMAL:
        default:
            if(0 < i)
            {
                appendFormat(pEncoderStream, " ");
            }
            appendFormat(pEncoderStream, "%u", Value.pValue[i]);
            break;

        case LLRP_FMT_HEX:
            appendFormat(pEncoderStream, "%02X", Value.pValue[i]);
            break;
        }
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_s8v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s8v_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;
    int                         i;

    appendOpenTag(pEncoderStream, pFieldName);
    for(i = 0; i < Value.nValue; i++)
    {
        switch(pFieldDescriptor->eFieldFormat)
        {
        case LLRP_FMT_DEC:
        case LLRP_FMT_NORMAL:
        default:
            if(0 < i)
            {
                appendFormat(pEncoderStream, " ");
            }
            appendFormat(pEncoderStream, "%d", Value.pValue[i]);
            break;

        case LLRP_FMT_HEX:
            appendFormat(pEncoderStream, "%02X", 0xFF & Value.pValue[i]);
            break;
        }
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_u16 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u16_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;

    appendOpenTag(pEncoderStream, pFieldName);
    switch(pFieldDescriptor->eFieldFormat)
    {
    case LLRP_FMT_NORMAL:
    case LLRP_FMT_DEC:
    default:
        appendFormat(pEncoderStream, "%u", Value);
        break;

    case LLRP_FMT_HEX:
        appendFormat(pEncoderStream, "%04X", Value);
        break;
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_s16 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s16_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;

    appendOpenTag(pEncoderStream, pFieldName);
    switch(pFieldDescriptor->eFieldFormat)
    {
    case LLRP_FMT_NORMAL:
    case LLRP_FMT_DEC:
    default:
        appendFormat(pEncoderStream, "%d", Value);
        break;

    case LLRP_FMT_HEX:
        appendFormat(pEncoderStream, "%04X", 0xFFFF & Value);
        break;
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_u16v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u16v_t             Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;
    int                         i;

    appendOpenTag(pEncoderStream, pFieldName);
    for(i = 0; i < Value.nValue; i++)
    {
        if(0 < i)
        {
            appendFormat(pEncoderStream, " ");
        }
        switch(pFieldDescriptor->eFieldFormat)
        {
        case LLRP_FMT_DEC:
        case LLRP_FMT_NORMAL:
        default:
            appendFormat(pEncoderStream, "%u", Value.pValue[i]);
            break;

        case LLRP_FMT_HEX:
            appendFormat(pEncoderStream, "%04X", Value.pValue[i]);
            break;
        }
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_s16v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s16v_t             Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;
    int                         i;

    appendOpenTag(pEncoderStream, pFieldName);
    for(i = 0; i < Value.nValue; i++)
    {
        if(0 < i)
        {
            appendFormat(pEncoderStream, " ");
        }
        switch(pFieldDescriptor->eFieldFormat)
        {
        case LLRP_FMT_DEC:
        case LLRP_FMT_NORMAL:
        default:
            appendFormat(pEncoderStream, "%d", Value.pValue[i]);
            break;

        case LLRP_FMT_HEX:
            appendFormat(pEncoderStream, "%04X", 0xFFFF & Value.pValue[i]);
            break;
        }
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_u32 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u32_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;

    appendOpenTag(pEncoderStream, pFieldName);
    switch(pFieldDescriptor->eFieldFormat)
    {
    case LLRP_FMT_NORMAL:
    case LLRP_FMT_DEC:
    default:
        appendFormat(pEncoderStream, "%u", Value);
        break;

    case LLRP_FMT_HEX:
        appendFormat(pEncoderStream, "%08X", Value);
        break;
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_s32 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s32_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;

    appendOpenTag(pEncoderStream, pFieldName);
    switch(pFieldDescriptor->eFieldFormat)
    {
    case LLRP_FMT_NORMAL:
    case LLRP_FMT_DEC:
    default:
        appendFormat(pEncoderStream, "%d", Value);
        break;

    case LLRP_FMT_HEX:
        appendFormat(pEncoderStream, "%08X", Value);
        break;
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_u32v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u32v_t             Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;
    int                         i;

    appendOpenTag(pEncoderStream, pFieldName);
    for(i = 0; i < Value.nValue; i++)
    {
        if(0 < i)
        {
            appendFormat(pEncoderStream, " ");
        }
        switch(pFieldDescriptor->eFieldFormat)
        {
        case LLRP_FMT_DEC:
        case LLRP_FMT_NORMAL:
        default:
            appendFormat(pEncoderStream, "%u", Value.pValue[i]);
            break;

        case LLRP_FMT_HEX:
            appendFormat(pEncoderStream, "%08X", Value.pValue[i]);
            break;
        }
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_s32v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s32v_t             Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;
    int                         i;

    appendOpenTag(pEncoderStream, pFieldName);
    for(i = 0; i < Value.nValue; i++)
    {
        if(0 < i)
        {
            appendFormat(pEncoderStream, " ");
        }
        switch(pFieldDescriptor->eFieldFormat)
        {
        case LLRP_FMT_DEC:
        case LLRP_FMT_NORMAL:
        default:
            appendFormat(pEncoderStream, "%d", Value.pValue[i]);
            break;

        case LLRP_FMT_HEX:
            appendFormat(pEncoderStream, "%08X", Value.pValue[i]);
            break;
        }
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_u64 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u64_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;

    appendOpenTag(pEncoderStream, pFieldName);
    switch(pFieldDescriptor->eFieldFormat)
    {
    case LLRP_FMT_NORMAL:
    case LLRP_FMT_DEC:
    default:
#if defined(WIN32)|| defined(WINCE)
        appendFormat(pEncoderStream, "%I64u", Value);
#else
        appendFormat(pEncoderStream, "%llu", Value);
#endif
        break;

    case LLRP_FMT_HEX:
#if defined(WIN32)|| defined(WINCE)
        appendFormat(pEncoderStream, "%016I64X", Value);
#else
        appendFormat(pEncoderStream, "%016llX", Value);
#endif
        break;

    case LLRP_FMT_DATETIME:
        {
            char                aBuf[64];
            time_t              CurSec  = Value / 1000000u;
            llrp_u32_t          CurUSec = Value % 1000000u;
            struct tm *         pLclTime;
#ifndef WINCE
            pLclTime = localtime(&CurSec);
            strftime(aBuf, sizeof aBuf, "%Y-%m-%dT%H:%M:%S", pLclTime);
#endif
            appendFormat(pEncoderStream, "%s.%06d", aBuf, CurUSec);
        }
        break;
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_s64 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s64_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;

    appendOpenTag(pEncoderStream, pFieldName);
    switch(pFieldDescriptor->eFieldFormat)
    {
    case LLRP_FMT_NORMAL:
    case LLRP_FMT_DEC:
    default:
#if defined(WIN32)|| defined(WINCE)
        appendFormat(pEncoderStream, "%I64d", Value);
#else
        appendFormat(pEncoderStream, "%lld", Value);
#endif
        break;

    case LLRP_FMT_HEX:
#if defined(WIN32)|| defined(WINCE)
        appendFormat(pEncoderStream, "%016I64X", Value);
#else
        appendFormat(pEncoderStream, "%016llX", Value);
#endif
        break;
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_u64v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u64v_t             Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;
    int                         i;

    appendOpenTag(pEncoderStream, pFieldName);
    for(i = 0; i < Value.nValue; i++)
    {
        if(0 < i)
        {
            appendFormat(pEncoderStream, " ");
        }
        switch(pFieldDescriptor->eFieldFormat)
        {
        case LLRP_FMT_DEC:
        case LLRP_FMT_NORMAL:
        default:
#if defined(WIN32)|| defined(WINCE)
            appendFormat(pEncoderStream, "%I64u", Value.pValue[i]);
#else
            appendFormat(pEncoderStream, "%llu", Value.pValue[i]);
#endif
            break;

        case LLRP_FMT_HEX:
#if defined(WIN32)|| defined(WINCE)
            appendFormat(pEncoderStream, "%016I64X", Value.pValue[i]);
#else
			appendFormat(pEncoderStream, "%016X", Value.pValue[i]);
#endif
            break;
        }
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_s64v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s64v_t             Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;
    int                         i;

    appendOpenTag(pEncoderStream, pFieldName);
    for(i = 0; i < Value.nValue; i++)
    {
        if(0 < i)
        {
            appendFormat(pEncoderStream, " ");
        }
        switch(pFieldDescriptor->eFieldFormat)
        {
        case LLRP_FMT_DEC:
        case LLRP_FMT_NORMAL:
        default:
#if defined(WIN32)|| defined(WINCE)
            appendFormat(pEncoderStream, "%I64d", Value.pValue[i]);
#else
            appendFormat(pEncoderStream, "%lld", Value.pValue[i]);
#endif
            break;

        case LLRP_FMT_HEX:
#if defined(WIN32)|| defined(WINCE)
            appendFormat(pEncoderStream, "%016I64X", Value.pValue[i]);
#else
            appendFormat(pEncoderStream, "%016X", Value.pValue[i]);
#endif
            break;
        }
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_u1 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u1_t               Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;

    appendOpenTag(pEncoderStream, pFieldName);
    switch(pFieldDescriptor->eFieldFormat)
    {
    case LLRP_FMT_NORMAL:
    default:
        appendFormat(pEncoderStream, "%s", (Value & 1) ? "true" : "false");
        break;

    case LLRP_FMT_DEC:
    case LLRP_FMT_HEX:
        appendFormat(pEncoderStream, "%u", Value & 1);
        break;
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_u1v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u1v_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;
    int                         nByte = (Value.nBit + 7u) / 8u;
    int                         i;

    indent(pEncoderStream, 0);
    appendFormat(pEncoderStream, "<");
    appendPrefixedTagName(pEncoderStream, pFieldName);
    appendFormat(pEncoderStream, " Count='%d'>", Value.nBit);

    for(i = 0; i < nByte; i++)
    {
        appendFormat(pEncoderStream, "%02X", Value.pValue[i]);
    }

    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_u2 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u2_t               Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;

    appendOpenTag(pEncoderStream, pFieldName);
    appendFormat(pEncoderStream, "%u", Value & 3);
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_u96 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u96_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;
    int                         i;

    appendOpenTag(pEncoderStream, pFieldName);
    for(i = 0; i < 12; i++)
    {
        appendFormat(pEncoderStream, "%02X", Value.aValue[i]);
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_utf8v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_utf8v_t            Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;
    int                         i;

    appendOpenTag(pEncoderStream, pFieldName);
    for(i = 0; i < Value.nValue; i++)
    {
        int                     c = 0xFF & Value.pValue[i];

        if(0 == c && i+1 == Value.nValue)
        {
            continue;
        }
        if(' ' <= c && c < 0x7F)
        {
            appendFormat(pEncoderStream, "%c", c);
        }
        else
        {
            appendFormat(pEncoderStream, "&#%02x;", c);
        }
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_bytesToEnd (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_bytesToEnd_t       Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;
    int                         i;

    appendOpenTag(pEncoderStream, pFieldName);
    for(i = 0; i < Value.nValue; i++)
    {
        appendFormat(pEncoderStream, "%02X", Value.pValue[i]);
    }
    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_e1 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const int                     Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    put_enum(pBaseEncoderStream, Value, pFieldDescriptor);
}

static void
put_e2 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const int                     Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    put_enum(pBaseEncoderStream, Value, pFieldDescriptor);
}

static void
put_e8 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const int                     Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    put_enum(pBaseEncoderStream, Value, pFieldDescriptor);
}

static void
put_e16 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const int                     Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    put_enum(pBaseEncoderStream, Value, pFieldDescriptor);
}

static void
put_e32 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const int                     Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    put_enum(pBaseEncoderStream, Value, pFieldDescriptor);
}

static void
put_e8v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u8v_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;
    int                         i;

    appendOpenTag(pEncoderStream, pFieldName);

    for(i = 0; i < Value.nValue; i++)
    {
        int                     eValue = Value.pValue[i];
        const LLRP_tSEnumTableEntry *pEntry;

        for(
            pEntry = pFieldDescriptor->pEnumTable;
            NULL != pEntry->pName;
            pEntry++)
        {
            if(pEntry->Value == eValue)
            {
                break;
            }
        }

        if(0 < i)
        {
            appendFormat(pEncoderStream, " ");
        }

        if(NULL != pEntry->pName)
        {
            appendFormat(pEncoderStream, "%s", pEntry->pName);
        }
        else
        {
            appendFormat(pEncoderStream, "%d", eValue);
        }
    }

    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_enum (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  int                           eValue,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;
    const char *                pFieldName = pFieldDescriptor->pName;
    const LLRP_tSEnumTableEntry *pEntry;

    appendOpenTag(pEncoderStream, pFieldName);

    for(
        pEntry = pFieldDescriptor->pEnumTable;
        NULL != pEntry->pName;
        pEntry++)
    {
        if(pEntry->Value == eValue)
        {
            break;
        }
    }

    if(NULL != pEntry->pName)
    {
        appendFormat(pEncoderStream, "%s", pEntry->pName);
    }
    else
    {
        appendFormat(pEncoderStream, "%d", eValue);
    }

    appendCloseTag(pEncoderStream, pFieldName);
}

static void
put_reserved (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  unsigned int                  nBits)
{
    LLRP_tSXMLTextEncoderStream * pEncoderStream =
                            (LLRP_tSXMLTextEncoderStream *) pBaseEncoderStream;

    indent(pEncoderStream, 0);
    appendFormat(pEncoderStream, "<!-- reserved %d bits -->\n", nBits);
}

static void
streamConstruct_outermost (
  LLRP_tSXMLTextEncoderStream *   pEncoderStream,
  LLRP_tSXMLTextEncoder *         pEncoder)
{
    memset(pEncoderStream, 0, sizeof *pEncoderStream);
    pEncoderStream->encoderStreamHdr.pEncoderStreamOps =
                                &s_XMLTextEncoderStreamOps;

    pEncoderStream->pEncoder                = pEncoder;
    pEncoderStream->pEnclosingEncoderStream = NULL;
    pEncoderStream->pRefType                = NULL;
    pEncoderStream->nDepth                  = 1;
}


static void
streamConstruct_nested (
  LLRP_tSXMLTextEncoderStream *   pEncoderStream,
  LLRP_tSXMLTextEncoderStream *   pEnclosingEncoderStream)
{
    LLRP_tSXMLTextEncoder *       pEncoder;

    pEncoder = pEnclosingEncoderStream->pEncoder;

    memset(pEncoderStream, 0, sizeof *pEncoderStream);
    pEncoderStream->encoderStreamHdr.pEncoderStreamOps =
                                &s_XMLTextEncoderStreamOps;

    pEncoderStream->pEncoder                = pEncoder;
    pEncoderStream->pEnclosingEncoderStream = pEnclosingEncoderStream;
    pEncoderStream->pRefType                = NULL;
    pEncoderStream->nDepth                  =
                            pEnclosingEncoderStream->nDepth+1;
}


static void
putElement (
  LLRP_tSXMLTextEncoderStream * pEncoderStream,
  const LLRP_tSElement *        pElement)
{
    const LLRP_tSTypeDescriptor *pRefType = pElement->pType;

    pEncoderStream->pRefType = pRefType;

    indent(pEncoderStream, -1);
    appendFormat(pEncoderStream, "<");
    appendPrefixedTagName(pEncoderStream, pRefType->pName);
    if(pRefType->bIsMessage)
    {
        appendFormat(pEncoderStream, " MessageID='%u'",
            ((LLRP_tSMessage *)pElement)->MessageID);
    }

    if(NULL == pEncoderStream->pEnclosingEncoderStream)
    {
        tNamespaceList          NamespaceList;
        const LLRP_tSNamespaceDescriptor *pNamespaceDescriptor;
        int                     iNSD;

        memset(&NamespaceList, 0, sizeof NamespaceList);

        LLRP_Element_walk(pElement,
            discoverNamespaces, (void*)&NamespaceList,
            0, 12);

        /* Emit the namespace cookie for each */
        for(iNSD = 0; iNSD < NamespaceList.nNamespaceDescriptor; iNSD++)
        {
            pNamespaceDescriptor = NamespaceList.apNamespaceDescriptor[iNSD];

            appendFormat(pEncoderStream, "\n");
            indent(pEncoderStream, 0);
            appendFormat(pEncoderStream, "xmlns:%s='%s'",
                pNamespaceDescriptor->pPrefix,
                pNamespaceDescriptor->pURI);
            /*
             * If this is the default namespace then emit the assigment.
             */
            if(0 == strcmp(pNamespaceDescriptor->pPrefix, "llrp"))
            {
                appendFormat(pEncoderStream, "\n");
                indent(pEncoderStream, 0);
                appendFormat(pEncoderStream, "xmlns='%s'",
                    pNamespaceDescriptor->pURI);
            }
        }
    }
    appendFormat(pEncoderStream, ">\n");

    pRefType->pfEncode(pElement, &pEncoderStream->encoderStreamHdr);

    indent(pEncoderStream, -1);
    appendCloseTag(pEncoderStream, pRefType->pName);
}


static int
discoverNamespaces (
  const LLRP_tSElement *        pElement,
  void *                        pArg)
{
    tNamespaceList *            pNSL = (tNamespaceList *) pArg;
    const LLRP_tSNamespaceDescriptor *pNamespaceDescriptor;
    int                         iNSD;

    pNamespaceDescriptor = pElement->pType->pNamespaceDescriptor;

    for(iNSD = 0; iNSD < pNSL->nNamespaceDescriptor; iNSD++)
    {
        if(pNSL->apNamespaceDescriptor[iNSD] == pNamespaceDescriptor)
        {
            /* Already have it */
            return 0;
        }
    }

    /* if we get here this namespace isn't already in the list */
    if(MAX_NS > pNSL->nNamespaceDescriptor)
    {
        iNSD = pNSL->nNamespaceDescriptor++;
        pNSL->apNamespaceDescriptor[iNSD] = pNamespaceDescriptor;
    }

    return 0;
}


static void
nestSubParameter (
  LLRP_tSXMLTextEncoderStream * pEncoderStream,
  const LLRP_tSParameter *      pParameter)
{
    const LLRP_tSElement *      pElement = (const LLRP_tSElement *)pParameter;
    LLRP_tSXMLTextEncoderStream NestEncoderStream;

    if(NULL == pParameter)
    {
        return;
    }

    streamConstruct_nested(&NestEncoderStream, pEncoderStream);

    putElement(&NestEncoderStream, pElement);
}

static void
indent (
  LLRP_tSXMLTextEncoderStream * pEncoderStream,
  int                           adjust)
{
    int                         n = pEncoderStream->nDepth + adjust;
    int                         i;

    for(i = 0; i < n; i++)
    {
        appendFormat(pEncoderStream, "  ");
    }
}

static void
appendOpenTag (
  LLRP_tSXMLTextEncoderStream * pEncoderStream,
  const char *                  pName)
{
    indent(pEncoderStream, 0);
    appendFormat(pEncoderStream, "<");
    appendPrefixedTagName(pEncoderStream, pName);
    appendFormat(pEncoderStream, ">");
}

static void
appendCloseTag (
  LLRP_tSXMLTextEncoderStream * pEncoderStream,
  const char *                  pName)
{
    appendFormat(pEncoderStream, "</");
    appendPrefixedTagName(pEncoderStream, pName);
    appendFormat(pEncoderStream, ">\n");
}

static void
appendPrefixedTagName (
  LLRP_tSXMLTextEncoderStream * pEncoderStream,
  const char *                  pName)
{
    const LLRP_tSTypeDescriptor *pRefType = pEncoderStream->pRefType;
    const char *                pPrefix =
                                    pRefType->pNamespaceDescriptor->pPrefix;

    if(0 != strcmp("llrp", pPrefix))
    {
        appendFormat(pEncoderStream, "%s:%s", pPrefix, pName);
    }
    else
    {
        appendFormat(pEncoderStream, "%s", pName);
    }
}

static void
appendFormat (
  LLRP_tSXMLTextEncoderStream * pEncoderStream,
  char *                        pFmtStr,
                                ...)
{
    LLRP_tSXMLTextEncoder *     pEncoder = pEncoderStream->pEncoder;
    char                        aHoldBuf[256u];
    int                         nHoldBuf;
    va_list                     ap;

    /* If overflow already detected, bail */
    if(pEncoder->bOverflow)
    {
        return;
    }

    va_start(ap, pFmtStr);
#if defined(WIN32)|| defined(WINCE)
    _vsnprintf(aHoldBuf, sizeof aHoldBuf, pFmtStr, ap);
#else
    vsnprintf(aHoldBuf, sizeof aHoldBuf, pFmtStr, ap);
#endif
    va_end(ap);

    nHoldBuf = strlen(aHoldBuf);

    if(pEncoder->iNext + nHoldBuf >= pEncoder->nBuffer)
    {
        pEncoder->bOverflow = 1;
        return;
    }

    strcpy((char *)&pEncoder->pBuffer[pEncoder->iNext], aHoldBuf);

    pEncoder->iNext += nHoldBuf;
}


/**
 *****************************************************************************
 **
 ** @brief  Format an element as XML text
 **
 ** Basic steps
 **     - Construct an XML encoder that fills a buffer
 **     - Encode the message through the XML encoder
 **     - Destruct the XML encoder
 **
 ** @param[in]  pElement        Pointer to message/parameter to format
 **
 ** @return     LLRP_tResultCode
 **
 *****************************************************************************/

LLRP_tResultCode
LLRP_toXMLString (
  const LLRP_tSElement *        pElement,
  char *                        pBuffer,
  int                           nBuffer)
{
    LLRP_tSXMLTextEncoder *     pXMLEncoder;
    LLRP_tSEncoder *            pEncoder;
    const LLRP_tSErrorDetails * pError;

    /*
     * Make sure the element is not NULL.
     */
    if(NULL == pElement)
    {
        strcpy(pBuffer, "ERROR: NULL pMessage to printXMLMessage\n");
        return LLRP_RC_MiscError;
    }

    /*
     * Construct an XML encoder
     */
    pXMLEncoder = LLRP_XMLTextEncoder_construct((unsigned char *)pBuffer, nBuffer);
    if(NULL == pXMLEncoder)
    {
        strcpy(pBuffer, "ERROR: XMLTextEncoder_construct failed\n");
        return LLRP_RC_MiscError;
    }

    /*
     * Essentially cast the XMLEncoder as a generic encoder.
     */
    pEncoder = &pXMLEncoder->encoderHdr;

    /*
     * Now let the encoding mechanism do its thing.
     */
    LLRP_Encoder_encodeElement(pEncoder, pElement);

    /*
     * Check the outcome in the error details.
     * If there is a problem, return the error rather
     * than the assumed to be useless string.
     */
    pError = &pEncoder->ErrorDetails;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        sprintf(pBuffer, "ERROR: %s XML text failed, %s\n",
            pElement->pType->pName,
            pError->pWhatStr ? pError->pWhatStr : "no reason given");

        LLRP_Encoder_destruct(pEncoder);

        return pError->eResultCode;
    }

    /*
     * Check if the XML fit in the buffer.
     */
    if(pXMLEncoder->bOverflow)
    {
        strcpy(pBuffer, "ERROR: Buffer overflow\n");
        LLRP_Encoder_destruct(pEncoder);
        return LLRP_RC_MiscError;
    }

    /*
     * Done with the XML encoder.
     */
    LLRP_Encoder_destruct(pEncoder);

    return LLRP_RC_OK;
}

