
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


#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "ltkc_platform.h"
#include "ltkc_base.h"
#include "ltkc_xmltext.h"


/*
 * BEGIN forward decls
 */

LLRP_tSXMLTextDecoder *
LLRP_XMLTextDecoder_construct (
  const LLRP_tSTypeRegistry *   pTypeRegistry,
  unsigned char *               pBuffer,
  unsigned int                  nBuffer);

static void
decoderDestruct (
  LLRP_tSDecoder *              pBaseDecoder);

static LLRP_tSMessage *
topDecodeMessage (
  LLRP_tSDecoder *              pBaseDecoder);

static int
getTag (
  LLRP_tSXMLTextDecoder *       pDecoder);

static int
getTextChar (
  LLRP_tSXMLTextDecoder *       pDecoder);

static int
getText (
  LLRP_tSXMLTextDecoder *       pDecoder);

static int
skipInsignificantText (
  LLRP_tSXMLTextDecoder *       pDecoder);

static void
skipXMLComment (
  LLRP_tSXMLTextDecoder *       pDecoder);

static void
skipXMLProcessingInstruction (
  LLRP_tSXMLTextDecoder *       pDecoder);

static llrp_u8_t
get_u8 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_s8_t
get_s8 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_u8v_t
get_u8v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_s8v_t
get_s8v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_u16_t
get_u16 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_s16_t
get_s16 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_u16v_t
get_u16v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_s16v_t
get_s16v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_u32_t
get_u32 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_s32_t
get_s32 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_u32v_t
get_u32v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_s32v_t
get_s32v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_u64_t
get_u64 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_s64_t
get_s64 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_u64v_t
get_u64v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_s64v_t
get_s64v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_u1_t
get_u1 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_u1v_t
get_u1v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_u2_t
get_u2 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_u96_t
get_u96 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_utf8v_t
get_utf8v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_bytesToEnd_t
get_bytesToEnd (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static int
get_e1 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static int
get_e2 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static int
get_e8 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static int
get_e16 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static int
get_e32 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
get_reserved (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  unsigned int                  nBit);

static void
streamConstruct_outermost (
  LLRP_tSXMLTextDecoderStream *   pDecoderStream,
  LLRP_tSXMLTextDecoder *         pDecoder);

static void
streamConstruct_nested (
  LLRP_tSXMLTextDecoderStream *   pDecoderStream,
  LLRP_tSXMLTextDecoderStream *   pEnclosingDecoderStream);

static LLRP_tSElement *
decodeElement (
  LLRP_tSXMLTextDecoderStream * pDecoderStream,
  int                           bAllowMessage);

static llrp_bool_t
getFieldText (
  LLRP_tSXMLTextDecoderStream * pDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_s64_t
getFieldIntg (
  LLRP_tSXMLTextDecoderStream * pDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor,
  llrp_s64_t                    minValue,
  llrp_s64_t                    maxValue);

static llrp_bool_t
getFieldIntgVec (
  LLRP_tSXMLTextDecoderStream * pDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor,
  unsigned int                  nValueMax,
  llrp_s64_t                    minValue,
  llrp_s64_t                    maxValue);

static llrp_bool_t
getFieldHexBinary (
  LLRP_tSXMLTextDecoderStream * pDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static int
getFieldEnum (
  LLRP_tSXMLTextDecoderStream * pDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_bool_t
verifyVectorAllocation (
  LLRP_tSXMLTextDecoderStream *   pDecoderStream,
  const void *                  pValue,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

/*
 * END forward decls
 */


#define MAX_U8      ((1ull << 8u) - 1u)
#define MIN_U8      0ull
#define MAX_S8      ((1ull << 7u) - 1u)
#define MIN_S8      (-1ull - MAX_S8)

#define MAX_U16     ((1ull << 16u) - 1u)
#define MIN_U16     0ull
#define MAX_S16     ((1ull << 15u) - 1u)
#define MIN_S16     (-1ull - MAX_S16)

#define MAX_U32     ((1ull << 32u) - 1u)
#define MIN_U32     0ull
#define MAX_S32     ((1ull << 31u) - 1u)
#define MIN_S32     (-1ull - MAX_S32)

#define MAX_U64     ((1ull << 63u) - 1u)    // one bit lost
#define MIN_U64     0ull
#define MAX_S64     ((1ull << 63u) - 1u)
#define MIN_S64     (-1ull - MAX_S64)


#define ARRAY_COUNT(X)  (sizeof(X) / sizeof(X[0]))

#define ISQUOTE(C)      ((C) == '"' || (C) == '\'')



static LLRP_tSDecoderOps
s_XMLTextDecoderOps =
{
    .pfDestruct             = decoderDestruct,
    .pfDecodeMessage        = topDecodeMessage,
};

static LLRP_tSDecoderStreamOps
s_XMLTextDecoderStreamOps =
{
    .pfGet_u8               = get_u8,
    .pfGet_s8               = get_s8,
    .pfGet_u8v              = get_u8v,
    .pfGet_s8v              = get_s8v,

    .pfGet_u16              = get_u16,
    .pfGet_s16              = get_s16,
    .pfGet_u16v             = get_u16v,
    .pfGet_s16v             = get_s16v,

    .pfGet_u32              = get_u32,
    .pfGet_s32              = get_s32,
    .pfGet_u32v             = get_u32v,
    .pfGet_s32v             = get_s32v,

    .pfGet_u64              = get_u64,
    .pfGet_s64              = get_s64,
    .pfGet_u64v             = get_u64v,
    .pfGet_s64v             = get_s64v,

    .pfGet_u1               = get_u1,
    .pfGet_u1v              = get_u1v,
    .pfGet_u2               = get_u2,
    .pfGet_u96              = get_u96,
    .pfGet_utf8v            = get_utf8v,
    .pfGet_bytesToEnd       = get_bytesToEnd,

    .pfGet_e1               = get_e1,
    .pfGet_e2               = get_e2,
    .pfGet_e8               = get_e8,
    .pfGet_e16              = get_e16,
    .pfGet_e32              = get_e32,

    .pfGet_reserved         = get_reserved,
};

LLRP_tSXMLTextDecoder *
LLRP_XMLTextDecoder_construct (
  const LLRP_tSTypeRegistry *   pTypeRegistry,
  unsigned char *               pInput,
  unsigned int                  nInput)
{
    LLRP_tSXMLTextDecoder *     pDecoder;

    pDecoder = malloc(sizeof *pDecoder);
    if(NULL == pDecoder)
    {
        return pDecoder;
    }

    memset(pDecoder, 0, sizeof *pDecoder);

    pDecoder->decoderHdr.pDecoderOps = &s_XMLTextDecoderOps;
    pDecoder->decoderHdr.pRegistry = pTypeRegistry;

    pDecoder->pInput    = pInput;
    pDecoder->nInput    = nInput;

    pDecoder->iNext     = 0;

    return pDecoder;
}

static void
decoderDestruct (
  LLRP_tSDecoder *              pBaseDecoder)
{
    LLRP_tSXMLTextDecoder *     pDecoder =
                                    (LLRP_tSXMLTextDecoder*)pBaseDecoder;

    free(pDecoder);
}

static LLRP_tSMessage *
topDecodeMessage (
  LLRP_tSDecoder *              pBaseDecoder)
{
    LLRP_tSXMLTextDecoder *     pDecoder;
    LLRP_tSXMLTextDecoderStream DecoderStream;
    LLRP_tSMessage *            pMessage;

    pDecoder = (LLRP_tSXMLTextDecoder *) pBaseDecoder;

    streamConstruct_outermost(&DecoderStream, pDecoder);

    pMessage = (LLRP_tSMessage *) decodeElement(&DecoderStream, TRUE);

    return pMessage;
}

static int
getTag (
  LLRP_tSXMLTextDecoder *       pDecoder)
{
    LLRP_tSErrorDetails *       pError = &pDecoder->decoderHdr.ErrorDetails;
    unsigned char *             pText = pDecoder->aTagBuf;
    unsigned                    iTextEnd = ARRAY_COUNT(pDecoder->aTagBuf);
    unsigned char *             pTextEnd = &pText[iTextEnd - 1];
    unsigned int                iSave;
    unsigned char *             pScan;
    unsigned char *             pInEnd = &pDecoder->pInput[pDecoder->nInput];
    int                         bInWord = FALSE;
    int                         bInQuote = FALSE;
    int                         quoteChar = 0;
    int                         bEndTag = FALSE;
    int                         bEmptyTag = FALSE;

    pDecoder->nTagArg = 0;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return -1;
    }

    skipInsignificantText(pDecoder);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return -1;
    }

    iSave = pDecoder->iNext;

    pScan = &pDecoder->pInput[pDecoder->iNext];

    if(pScan < pInEnd && '<' != *pScan)
    {
        pDecoder->iNext = iSave;

        // TODO good error
        pError->eResultCode = LLRP_RC_MiscError;
        pError->pWhatStr    = "missing tag";
        pError->pRefType    = NULL;
        pError->pRefField   = NULL;
        pError->OtherDetail = pDecoder->iNext;

        return -1;
    }
    pScan++;

    if(pScan < pInEnd && '/' == *pScan)
    {
        pScan++;
        bEndTag = TRUE;
    }

    for(; pScan < pInEnd; )
    {
        int                     c = *pScan++;

        if(c == '>')
        {
            break;
        }

        if(c == '/' && pScan < pInEnd && '>' == *pScan)
        {
            /* empty tag */
            pScan++;
            bEmptyTag = TRUE;
            break;
        }

        if(pText >= pTextEnd)
        {
            /* text buffer overflow */
            break;
        }

        if(bInWord)
        {
            if(bInQuote)
            {
                if(c == quoteChar)
                {
                    bInQuote = FALSE;
                }
                else
                {
                    *pText++ = c;
                }
            }
            else if(isspace(c))
            {
                *pText++ = 0;
                bInWord = FALSE;
            }
            else if(ISQUOTE(c))
            {
                bInQuote = TRUE;
                quoteChar = c;
            }
            else
            {
                *pText++ = c;
            }
        }
        else if(!isspace(c))
        {
            /* start word */
            pDecoder->apTagArg[pDecoder->nTagArg++] = pText;
            bInWord = TRUE;
            if(ISQUOTE(c))
            {
                bInQuote = TRUE;
                quoteChar = c;
            }
            else
            {
                *pText++ = c;
            }
        }
        else
        {
            /* isspace() and not in word, just skip over it */
        }
    }

    if(bInWord)
    {
        *pText++ = 0;
        bInWord = FALSE;
    }

    if(bEndTag)
    {
        pDecoder->eTagType = XMLTEXT_END_TAG;
    }
    else if(bEmptyTag)
    {
        pDecoder->eTagType = XMLTEXT_EMPTY_TAG;
    }
    else
    {
        pDecoder->eTagType = XMLTEXT_START_TAG;
    }

    if(0 == pDecoder->nTagArg)
    {
        pDecoder->iNext = iSave;

        // TODO good error
        pError->eResultCode = LLRP_RC_MiscError;
        pError->pWhatStr    = "malformed tag";
        pError->pRefType    = NULL;
        pError->pRefField   = NULL;
        pError->OtherDetail = pDecoder->iNext;

        return -1;
    }

    pDecoder->iNext = pScan - pDecoder->pInput;

    return TRUE;
}

static int
getTextChar (
  LLRP_tSXMLTextDecoder *       pDecoder)
{
    LLRP_tSErrorDetails *       pError = &pDecoder->decoderHdr.ErrorDetails;
    int                         c;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return -1;
    }

    if(pDecoder->iNext >= pDecoder->nInput)
    {
        return -1;
    }

    pDecoder->iLastGetTextChar = pDecoder->iNext;

    c = 0xFF & pDecoder->pInput[pDecoder->iNext];

    if('<' == c)
    {
        unsigned char *     pScan = &pDecoder->pInput[pDecoder->iNext];
        unsigned char *     pInEnd = &pDecoder->pInput[pDecoder->nInput];

        if(pScan+4 <= pInEnd && 0 == memcmp(pScan, "<!--", 4))
        {
            pDecoder->iNext += 4;
            skipXMLComment(pDecoder);
            if(LLRP_RC_OK != pError->eResultCode)
            {
                return -1;
            }
            else
            {
                return ' ';
            }
        }

        if(pScan+2 <= pInEnd && 0 == memcmp(pScan, "<?", 2))
        {
            pDecoder->iNext += 2;
            skipXMLProcessingInstruction(pDecoder);
            if(LLRP_RC_OK != pError->eResultCode)
            {
                return -1;
            }
            else
            {
                return ' ';
            }
        }

        return -1;
    }

    if('&' == c)
    {
        /* TODO: entity call-out */
        // TODO good error
        LLRP_Error_resultCodeAndWhatStr(pError, LLRP_RC_MiscError,
            "unsupported entity reference");
        return -1;
    }
    else
    {
        pDecoder->iNext++;
    }

    return c;
}


static int
getText (
  LLRP_tSXMLTextDecoder *       pDecoder)
{
    LLRP_tSErrorDetails *       pError = &pDecoder->decoderHdr.ErrorDetails;
    unsigned char *             pText = pDecoder->aTextBuf;
    unsigned                    iTextEnd = ARRAY_COUNT(pDecoder->aTagBuf);
    unsigned char *             pTextEnd = &pText[iTextEnd - 1];

    pDecoder->iLastText = pDecoder->iNext;

    for(;;)
    {
        int                     c;

        if(LLRP_RC_OK != pError->eResultCode)
        {
            break;
        }

        c = getTextChar(pDecoder);
        if(0 > c)
        {
            break;
        }

        if(pText >= pTextEnd)
        {
            LLRP_Error_resultCodeAndWhatStr(pError, LLRP_RC_MiscError,
                "text overrun");
            continue;
        }

        if(pText == pDecoder->aTextBuf)
        {
            if(isspace(c))
            {
                /* trim leading space */
                continue;
            }
            pDecoder->iLastText = pDecoder->iNext;
        }

        *pText++ = c;
    }

    *pText = 0;
    pDecoder->nTextBuf = pText - pDecoder->aTextBuf;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

static int
skipInsignificantText (
  LLRP_tSXMLTextDecoder *       pDecoder)
{
    LLRP_tSErrorDetails *       pError = &pDecoder->decoderHdr.ErrorDetails;

    for(;;)
    {
        int                     c;

        if(LLRP_RC_OK != pError->eResultCode)
        {
            break;
        }

        c = getTextChar(pDecoder);
        if(0 > c)
        {
            break;
        }

        if(!isspace(c))
        {
            pDecoder->iNext = pDecoder->iLastGetTextChar;

            // TODO good error
            pError->eResultCode = LLRP_RC_MiscError;
            pError->pWhatStr    = "unexpected text";
            pError->pRefType    = NULL;
            pError->pRefField   = NULL;
            pError->OtherDetail = pDecoder->iNext;
            break;
        }
    }

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

static void
skipXMLComment (
  LLRP_tSXMLTextDecoder *       pDecoder)
{
    LLRP_tSErrorDetails *       pError = &pDecoder->decoderHdr.ErrorDetails;
    unsigned char *             pScan = &pDecoder->pInput[pDecoder->iNext];
    unsigned char *             pInEnd = &pDecoder->pInput[pDecoder->nInput];
    int                         bFound = 0;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return;
    }

    for(; pScan+3 < pInEnd; pScan++)
    {
        if(0 == memcmp(pScan, "-->", 3))
        {
            pScan += 3;
            bFound = 1;
            break;
        }

        if(0 == memcmp(pScan, "--", 2))
        {
            pDecoder->iNext = pScan - pDecoder->pInput;

            // TODO good error
            LLRP_Error_resultCodeAndWhatStr(pError, LLRP_RC_MiscError,
                "illegal in comment");
            pError->OtherDetail = pDecoder->iNext;
            return;
        }
    }

    if(!bFound)
    {
        /* unterminated comment */

        // TODO good error
        LLRP_Error_resultCodeAndWhatStr(pError, LLRP_RC_MiscError,
            "unterminated comment");

        return;
    }

    pDecoder->iNext = pScan - pDecoder->pInput;
}

static void
skipXMLProcessingInstruction (
  LLRP_tSXMLTextDecoder *       pDecoder)
{
    LLRP_tSErrorDetails *       pError = &pDecoder->decoderHdr.ErrorDetails;
    unsigned char *             pScan = &pDecoder->pInput[pDecoder->iNext];
    unsigned char *             pInEnd = &pDecoder->pInput[pDecoder->nInput];
    int                         bFound = 0;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return;
    }

    for(; pScan+2 < pInEnd; pScan++)
    {
        if(0 == memcmp(pScan, "?>", 2))
        {
            pScan += 2;
            bFound = 1;
            break;
        }
    }

    if(!bFound)
    {
        /* unterminated processing instruction */

        // TODO good error
        LLRP_Error_resultCodeAndWhatStr(pError, LLRP_RC_MiscError,
            "unterminated processing instruction");

        return;
    }

    pDecoder->iNext = pScan - pDecoder->pInput;
}

static llrp_bool_t
matchStartTag (
  LLRP_tSXMLTextDecoder *       pDecoder,
  const char *                  pPattern)
{
    LLRP_tSErrorDetails *       pError = &pDecoder->decoderHdr.ErrorDetails;
    unsigned int                iSave;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return FALSE;
    }

    skipInsignificantText(pDecoder);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return FALSE;
    }

    iSave = pDecoder->iNext;

    getTag(pDecoder);

    if(XMLTEXT_START_TAG == pDecoder->eTagType &&
       0 == strcmp(pPattern, pDecoder->apTagArg[0]))
    {
        return TRUE;
    }

    pDecoder->iNext = iSave;

    return FALSE;
}

static llrp_bool_t
matchEndTag (
  LLRP_tSXMLTextDecoder *       pDecoder,
  const char *                  pPattern)
{
    LLRP_tSErrorDetails *       pError = &pDecoder->decoderHdr.ErrorDetails;
    unsigned int                iSave;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return FALSE;
    }

    skipInsignificantText(pDecoder);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return FALSE;
    }

    iSave = pDecoder->iNext;

    if(pDecoder->iNext+2 > pDecoder->nInput ||
       '<' != pDecoder->pInput[pDecoder->iNext+0] ||
       '/' != pDecoder->pInput[pDecoder->iNext+1])
    {
        return FALSE;
    }

    getTag(pDecoder);

    if(XMLTEXT_END_TAG == pDecoder->eTagType &&
       0 == strcmp(pPattern, pDecoder->apTagArg[0]))
    {
        return TRUE;
    }

    pDecoder->iNext = iSave;

    return FALSE;
}

static llrp_u8_t
get_u8 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_u8_t                   Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;

    Value = (llrp_u8_t) getFieldIntg(pDecoderStream, pFieldDescriptor,
                            MIN_U8, MAX_U8);

    return Value;
}

static llrp_s8_t
get_s8 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_s8_t                   Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;

    Value = (llrp_s8_t) getFieldIntg(pDecoderStream, pFieldDescriptor,
                            MIN_S8, MAX_S8);

    return Value;
}

static llrp_u8v_t
get_u8v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_u8v_t                  Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    if(getFieldIntgVec(pDecoderStream, pFieldDescriptor,
                            0, MIN_U8, MAX_U8))
    {
        LLRP_tSXMLTextDecoder * pDecoder = pDecoderStream->pDecoder;
        llrp_u16_t              nValue;

        nValue = pDecoder->nIntgValue;
        Value = LLRP_u8v_construct(nValue);
        if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                            pFieldDescriptor))
        {
            unsigned int    Ix;

            for(Ix = 0; Ix < nValue; Ix++)
            {
                Value.pValue[Ix] = (llrp_u8_t) pDecoder->aIntgValue[Ix];
            }
        }
    }

    return Value;
}

static llrp_s8v_t
get_s8v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_s8v_t                  Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    if(getFieldIntgVec(pDecoderStream, pFieldDescriptor,
                            0, MIN_S8, MAX_S8))
    {
        LLRP_tSXMLTextDecoder * pDecoder = pDecoderStream->pDecoder;
        llrp_u16_t              nValue;

        nValue = pDecoder->nIntgValue;
        Value = LLRP_s8v_construct(nValue);
        if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                            pFieldDescriptor))
        {
            unsigned int    Ix;

            for(Ix = 0; Ix < nValue; Ix++)
            {
                Value.pValue[Ix] = (llrp_s8_t) pDecoder->aIntgValue[Ix];
            }
        }
    }

    return Value;
}

static llrp_u16_t
get_u16 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_u16_t                  Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;

    Value = (llrp_u16_t) getFieldIntg(pDecoderStream, pFieldDescriptor,
                            MIN_U16, MAX_U16);

    return Value;
}

static llrp_s16_t
get_s16 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_s16_t                  Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;

    Value = (llrp_s16_t) getFieldIntg(pDecoderStream, pFieldDescriptor,
                            MIN_S16, MAX_S16);

    return Value;
}

static llrp_u16v_t
get_u16v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_u16v_t                 Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    if(getFieldIntgVec(pDecoderStream, pFieldDescriptor,
                            0, MIN_U16, MAX_U16))
    {
        LLRP_tSXMLTextDecoder * pDecoder = pDecoderStream->pDecoder;
        llrp_u16_t              nValue;

        nValue = pDecoder->nIntgValue;
        Value = LLRP_u16v_construct(nValue);
        if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                            pFieldDescriptor))
        {
            unsigned int    Ix;

            for(Ix = 0; Ix < nValue; Ix++)
            {
                Value.pValue[Ix] = (llrp_u16_t) pDecoder->aIntgValue[Ix];
            }
        }
    }

    return Value;
}

static llrp_s16v_t
get_s16v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_s16v_t                 Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    if(getFieldIntgVec(pDecoderStream, pFieldDescriptor,
                            0, MIN_S16, MAX_S16))
    {
        LLRP_tSXMLTextDecoder * pDecoder = pDecoderStream->pDecoder;
        llrp_u16_t              nValue;

        nValue = pDecoder->nIntgValue;
        Value = LLRP_s16v_construct(nValue);
        if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                            pFieldDescriptor))
        {
            unsigned int    Ix;

            for(Ix = 0; Ix < nValue; Ix++)
            {
                Value.pValue[Ix] = (llrp_u16_t) pDecoder->aIntgValue[Ix];
            }
        }
    }

    return Value;
}

static llrp_u32_t
get_u32 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_u32_t                  Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;

    Value = (llrp_u32_t) getFieldIntg(pDecoderStream, pFieldDescriptor,
                            MIN_U32, MAX_U32);

    return Value;
}

static llrp_s32_t
get_s32 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_s32_t                  Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;

    Value = (llrp_s32_t) getFieldIntg(pDecoderStream, pFieldDescriptor,
                            MIN_S32, MAX_S32);

    return Value;
}

static llrp_u32v_t
get_u32v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_u32v_t                 Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    if(getFieldIntgVec(pDecoderStream, pFieldDescriptor,
                            0, MIN_U32, MAX_U32))
    {
        LLRP_tSXMLTextDecoder * pDecoder = pDecoderStream->pDecoder;
        llrp_u16_t              nValue;

        nValue = pDecoder->nIntgValue;
        Value = LLRP_u32v_construct(nValue);
        if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                            pFieldDescriptor))
        {
            unsigned int    Ix;

            for(Ix = 0; Ix < nValue; Ix++)
            {
                Value.pValue[Ix] = (llrp_u32_t) pDecoder->aIntgValue[Ix];
            }
        }
    }

    return Value;
}

static llrp_s32v_t
get_s32v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_s32v_t                 Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    if(getFieldIntgVec(pDecoderStream, pFieldDescriptor,
                            0, MIN_S32, MAX_S32))
    {
        LLRP_tSXMLTextDecoder * pDecoder = pDecoderStream->pDecoder;
        llrp_u16_t              nValue;

        nValue = pDecoder->nIntgValue;
        Value = LLRP_s32v_construct(nValue);
        if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                            pFieldDescriptor))
        {
            unsigned int    Ix;

            for(Ix = 0; Ix < nValue; Ix++)
            {
                Value.pValue[Ix] = (llrp_s32_t) pDecoder->aIntgValue[Ix];
            }
        }
    }

    return Value;
}

static llrp_u64_t
get_u64 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_u64_t                  Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;

    Value = (llrp_u64_t) getFieldIntg(pDecoderStream, pFieldDescriptor,
                            MIN_U64, MAX_U64);

    return Value;
}

static llrp_s64_t
get_s64 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_s64_t                  Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;

    Value = (llrp_s64_t) getFieldIntg(pDecoderStream, pFieldDescriptor,
                            MIN_S64, MAX_S64);

    return Value;
}

static llrp_u64v_t
get_u64v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_u64v_t                 Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    if(getFieldIntgVec(pDecoderStream, pFieldDescriptor,
                            0, MIN_U64, MAX_U64))
    {
        LLRP_tSXMLTextDecoder * pDecoder = pDecoderStream->pDecoder;
        llrp_u16_t              nValue;

        nValue = pDecoder->nIntgValue;
        Value = LLRP_u64v_construct(nValue);
        if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                            pFieldDescriptor))
        {
            unsigned int    Ix;

            for(Ix = 0; Ix < nValue; Ix++)
            {
                Value.pValue[Ix] = (llrp_u64_t) pDecoder->aIntgValue[Ix];
            }
        }
    }

    return Value;
}

static llrp_s64v_t
get_s64v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_s64v_t                 Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    if(getFieldIntgVec(pDecoderStream, pFieldDescriptor,
                            0, MIN_S64, MAX_S64))
    {
        LLRP_tSXMLTextDecoder * pDecoder = pDecoderStream->pDecoder;
        llrp_u16_t              nValue;

        nValue = pDecoder->nIntgValue;
        Value = LLRP_s64v_construct(nValue);
        if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                            pFieldDescriptor))
        {
            unsigned int    Ix;

            for(Ix = 0; Ix < nValue; Ix++)
            {
                Value.pValue[Ix] = (llrp_s64_t) pDecoder->aIntgValue[Ix];
            }
        }
    }

    return Value;
}

static llrp_u1_t
get_u1 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_u1_t                   Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;

    Value = (llrp_u1_t) getFieldIntg(pDecoderStream, pFieldDescriptor,
                            0, 1);

    return Value;
}

static llrp_u1v_t
get_u1v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_u1v_t                  Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    if(getFieldHexBinary(pDecoderStream, pFieldDescriptor))
    {
        LLRP_tSXMLTextDecoder * pDecoder = pDecoderStream->pDecoder;
        llrp_u16_t              nByte;
        llrp_u16_t              nBit;
        unsigned int            Ix;

        nByte = pDecoder->nTextBuf;
        nBit = nByte * 8u;

        for(Ix = 1; Ix < pDecoder->nTagArg; Ix++)
        {
            if(0 == strncmp(pDecoder->apTagArg[Ix], "Count=", 6))
            {
                nBit = atoi(pDecoder->apTagArg[Ix]+6);
            }
        }

        nByte = (nBit + 7u) / 8u;

        if(nByte != pDecoder->nTextBuf)
        {
            LLRP_tSErrorDetails *pError = &pDecoder->decoderHdr.ErrorDetails;

            pDecoder->iNext = pDecoder->iLastText;

            pError->eResultCode = LLRP_RC_MiscError;
            pError->pWhatStr    = "incorrect number of bytes";
            pError->pRefType    = pDecoderStream->pRefType;
            pError->pRefField   = pFieldDescriptor;
            pError->OtherDetail = pDecoder->iNext;
        }
        else
        {
            Value = LLRP_u1v_construct(nBit);
            if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                            pFieldDescriptor))
            {
                for(Ix = 0; Ix < nByte; Ix++)
                {
                    Value.pValue[Ix] = pDecoder->aTextBuf[Ix];
                }
                Value.pValue[Ix] = 0;
            }
        }
    }

    return Value;
}

static llrp_u2_t
get_u2 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_u2_t                   Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;

    Value = (llrp_u2_t) getFieldIntg(pDecoderStream, pFieldDescriptor,
                            0, 3);

    return Value;
}

static llrp_u96_t
get_u96 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_u96_t                  Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    if(getFieldHexBinary(pDecoderStream, pFieldDescriptor))
    {
        LLRP_tSXMLTextDecoder * pDecoder = pDecoderStream->pDecoder;
        llrp_u16_t              nValue = pDecoder->nTextBuf;
        unsigned int            Ix;

        if(12 > nValue)
        {
            nValue = 12;
        }

        for(Ix = 0; Ix < nValue; Ix++)
        {
            Value.aValue[Ix] = pDecoder->aTextBuf[Ix];
        }

        /* TODO: check it */
        if(12 != pDecoder->nTextBuf)
        {
            LLRP_tSErrorDetails *pError = &pDecoder->decoderHdr.ErrorDetails;

            pDecoder->iNext = pDecoder->iLastText;

            pError->eResultCode = LLRP_RC_MiscError;
            pError->pWhatStr    = "incorrect number of bytes";
            pError->pRefType    = pDecoderStream->pRefType;
            pError->pRefField   = pFieldDescriptor;
            pError->OtherDetail = pDecoder->iNext;
        }
    }

    return Value;
}

static llrp_utf8v_t
get_utf8v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_utf8v_t                Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    if(getFieldText(pDecoderStream, pFieldDescriptor))
    {
        LLRP_tSXMLTextDecoder * pDecoder = pDecoderStream->pDecoder;
        llrp_u16_t              nValue;

        nValue = pDecoder->nTextBuf;
        Value = LLRP_utf8v_construct(nValue + 1);
        if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                            pFieldDescriptor))
        {
            unsigned int    Ix;

            for(Ix = 0; Ix < nValue; Ix++)
            {
                Value.pValue[Ix] = pDecoder->aTextBuf[Ix];
            }
            Value.pValue[Ix] = 0;
        }
    }

    return Value;
}

static llrp_bytesToEnd_t
get_bytesToEnd (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    llrp_bytesToEnd_t           Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    if(getFieldText(pDecoderStream, pFieldDescriptor))
    {
        LLRP_tSXMLTextDecoder * pDecoder = pDecoderStream->pDecoder;
        llrp_u16_t              nValue;

        nValue = pDecoder->nTextBuf;
        Value = LLRP_bytesToEnd_construct(nValue);
        if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                            pFieldDescriptor))
        {
            unsigned int    Ix;

            for(Ix = 0; Ix < nValue; Ix++)
            {
                Value.pValue[Ix] = pDecoder->aTextBuf[Ix];
            }
            Value.pValue[Ix] = 0;
        }
    }

    return Value;
}

static int
get_e1 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    int                         Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;
    Value = getFieldEnum(pDecoderStream, pFieldDescriptor);
    return Value;
}

static int
get_e2 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    int                         Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;
    Value = getFieldEnum(pDecoderStream, pFieldDescriptor);
    return Value;
}

static int
get_e8 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    int                         Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;
    Value = getFieldEnum(pDecoderStream, pFieldDescriptor);
    return Value;
}

static int
get_e16 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    int                         Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;
    Value = getFieldEnum(pDecoderStream, pFieldDescriptor);
    return Value;
}

static int
get_e32 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoderStream *pDecoderStream;
    int                         Value;

    pDecoderStream = (LLRP_tSXMLTextDecoderStream *) pBaseDecoderStream;
    Value = getFieldEnum(pDecoderStream, pFieldDescriptor);
    return Value;
}

static void
get_reserved (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  unsigned int                  nBit)
{
    return;
}

static void
streamConstruct_outermost (
  LLRP_tSXMLTextDecoderStream * pDecoderStream,
  LLRP_tSXMLTextDecoder *       pDecoder)
{
    memset(pDecoderStream, 0, sizeof *pDecoderStream);
    pDecoderStream->decoderStreamHdr.pDecoderStreamOps =
                                &s_XMLTextDecoderStreamOps;

    pDecoderStream->pDecoder                = pDecoder;
    pDecoderStream->pEnclosingDecoderStream = NULL;
    pDecoderStream->pRefType                = NULL;
}

static void
streamConstruct_nested (
  LLRP_tSXMLTextDecoderStream * pDecoderStream,
  LLRP_tSXMLTextDecoderStream * pEnclosingDecoderStream)
{
    LLRP_tSXMLTextDecoder *     pDecoder;

    pDecoder = pEnclosingDecoderStream->pDecoder;

    memset(pDecoderStream, 0, sizeof *pDecoderStream);
    pDecoderStream->decoderStreamHdr.pDecoderStreamOps =
                                &s_XMLTextDecoderStreamOps;

    pDecoderStream->pDecoder                = pDecoder;
    pDecoderStream->pEnclosingDecoderStream = pEnclosingDecoderStream;
    pDecoderStream->pRefType                = NULL;
}

static LLRP_tSElement *
decodeElement (
  LLRP_tSXMLTextDecoderStream * pDecoderStream,
  int                           bAllowMessage)
{
    LLRP_tSXMLTextDecoder *     pDecoder  = pDecoderStream->pDecoder;
    LLRP_tSErrorDetails *       pError    = &pDecoder->decoderHdr.ErrorDetails;
    const LLRP_tSTypeRegistry * pRegistry = pDecoder->decoderHdr.pRegistry;
    LLRP_tSDecoderStream *      pBaseDecoderStream =
                                        &pDecoderStream->decoderStreamHdr;
    const LLRP_tSTypeDescriptor *pTypeDescriptor;
    llrp_u32_t                  MessageID = 0;
    LLRP_tSElement *            pElement;
    unsigned int                iSave;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return NULL;
    }

    skipInsignificantText(pDecoder);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return NULL;
    }

    iSave = pDecoder->iNext;

    getTag(pDecoder);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return NULL;
    }

    if(XMLTEXT_START_TAG != pDecoder->eTagType)
    {
        pDecoder->iNext = iSave;

        // TODO good error
        pError->eResultCode = LLRP_RC_MiscError;
        pError->pWhatStr    = "missing start tag";
        pError->pRefType    = NULL;
        pError->pRefField   = NULL;
        pError->OtherDetail = pDecoder->iNext;
        return NULL;
    }

    pTypeDescriptor = LLRP_TypeRegistry_lookupByName(pRegistry,
                        pDecoder->apTagArg[0]);

    if(NULL == pTypeDescriptor)
    {
        pDecoder->iNext = iSave;

        // TODO good error
        pError->eResultCode = LLRP_RC_UnknownParameterType;
        pError->pWhatStr    = "unknown element type";
        pError->pRefType    = NULL;
        pError->pRefField   = NULL;
        pError->OtherDetail = pDecoder->iNext;
        return NULL;
    }

    pDecoderStream->pRefType = pTypeDescriptor;

    if(pTypeDescriptor->bIsMessage)
    {
        int                     iTagArg;

        if(!bAllowMessage)
        {
            pDecoder->iNext = iSave;

            pError->eResultCode = LLRP_RC_MiscError;
            pError->pWhatStr    = "message as subparameter";
            pError->pRefType    = pTypeDescriptor;
            pError->pRefField   = NULL;
            pError->OtherDetail = pDecoder->iNext;
            return NULL;
        }

        for(iTagArg = 1; iTagArg < pDecoder->nTagArg; iTagArg++)
        {
            char *              pArg = pDecoder->apTagArg[iTagArg];

            if(0 == strncmp("MessageID=", pArg, 10))
            {
                char *          pTail;

                MessageID = strtoul(pArg+10, &pTail, 0);
                if(*pTail != 0)
                {
                    /* malformed */
                    pDecoder->iNext = iSave;

                    pError->eResultCode = LLRP_RC_MiscError;
                    pError->pWhatStr    = "malformed MessageID";
                    pError->pRefType    = pTypeDescriptor;
                    pError->pRefField   = NULL;
                    pError->OtherDetail = pDecoder->iNext;
                    return NULL;
                }
                break;
            }
        }
    }

    pElement = LLRP_Element_construct(pTypeDescriptor);

    if(NULL == pElement)
    {
        pDecoder->iNext = iSave;

        pError->eResultCode = LLRP_RC_MessageAllocationFailed;
        pError->pWhatStr    = "element allocation failed";
        pError->pRefType    = pTypeDescriptor;
        pError->pRefField   = NULL;
        pError->OtherDetail = pDecoder->iNext;
        return NULL;
    }

    ((LLRP_tSMessage *) pElement)->MessageID = MessageID;

    pTypeDescriptor->pfDecodeFields(pElement, pBaseDecoderStream);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        LLRP_Element_destruct(pElement);
        return NULL;
    }

    /*
     * Subparameters
     */
    for(;;)
    {
        LLRP_tSXMLTextDecoderStream     NestStream;
        LLRP_tSElement *                pSubElement;
        LLRP_tSParameter *              pParameter;

        skipInsignificantText(pDecoder);

        if(LLRP_RC_OK != pError->eResultCode)
        {
            break;
        }

        iSave = pDecoder->iNext;

        getTag(pDecoder);

        if(LLRP_RC_OK != pError->eResultCode)
        {
            break;
        }

        if(XMLTEXT_END_TAG == pDecoder->eTagType)
        {
            if(0 != strcmp(pDecoder->apTagArg[0], pTypeDescriptor->pName))
            {
                pDecoder->iNext = iSave;

                // TODO good error
                pError->eResultCode = LLRP_RC_MiscError;
                pError->pWhatStr    = "missing end tag";
                pError->pRefType    = pTypeDescriptor;
                pError->pRefField   = NULL;
                pError->OtherDetail = pDecoder->iNext;
            }
            break;
        }

        pDecoder->iNext = iSave;

        streamConstruct_nested(&NestStream, pDecoderStream);

        pSubElement = decodeElement(&NestStream, FALSE);

        if(NULL == pSubElement)
        {
            if(LLRP_RC_OK == pError->eResultCode)
            {
                pDecoder->iNext = iSave;

                pError->eResultCode = LLRP_RC_Botch;
                pError->pWhatStr    = "botch -- no param and no error";
                pError->pRefType    = pTypeDescriptor;
                pError->pRefField   = NULL;
                pError->OtherDetail = pDecoder->iNext;
            }
            break;
        }

        pParameter = (LLRP_tSParameter *) pSubElement;

        pParameter->elementHdr.pParent = pElement;
        LLRP_Element_addSubParameterToAllList(pElement, pParameter);
    }

    if(LLRP_RC_OK != pError->eResultCode)
    {
        LLRP_Element_destruct(pElement);
        return NULL;
    }

    pTypeDescriptor->pfAssimilateSubParameters(pElement, pError);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        LLRP_Element_destruct(pElement);
        return NULL;
    }

    return pElement;
}

static llrp_bool_t
getFieldText (
  LLRP_tSXMLTextDecoderStream * pDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoder *     pDecoder = pDecoderStream->pDecoder;
    LLRP_tSErrorDetails *       pError = &pDecoder->decoderHdr.ErrorDetails;
    const char *                pFieldName = pFieldDescriptor->pName;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return FALSE;
    }

    skipInsignificantText(pDecoder);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return FALSE;
    }

    if(!matchStartTag(pDecoder, pFieldName))
    {
        if(LLRP_RC_OK == pError->eResultCode)
        {
            pError->eResultCode = LLRP_RC_MiscError;
            pError->pWhatStr    = "missing field start tag";
            pError->pRefType    = pDecoderStream->pRefType;
            pError->pRefField   = pFieldDescriptor;
            pError->OtherDetail = pDecoder->iNext;
        }

        return FALSE;
    }

    getText(pDecoder);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return FALSE;
    }

    if(!matchEndTag(pDecoder, pFieldName))
    {
        if(LLRP_RC_OK == pError->eResultCode)
        {
            pError->eResultCode = LLRP_RC_MiscError;
            pError->pWhatStr    = "missing field end tag";
            pError->pRefType    = pDecoderStream->pRefType;
            pError->pRefField   = pFieldDescriptor;
            pError->OtherDetail = pDecoder->iNext;
        }

        return FALSE;
    }

    return TRUE;
}

static llrp_s64_t
getFieldIntg (
  LLRP_tSXMLTextDecoderStream * pDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor,
  llrp_s64_t                    minValue,
  llrp_s64_t                    maxValue)
{
    LLRP_tSXMLTextDecoder *     pDecoder = pDecoderStream->pDecoder;

    if(getFieldIntgVec(pDecoderStream, pFieldDescriptor,
                                    1, minValue, maxValue))
    {
        if(1 != pDecoder->nIntgValue)
        {
            LLRP_tSErrorDetails *pError = &pDecoder->decoderHdr.ErrorDetails;

            pDecoder->iNext = pDecoder->iLastText;

            pError->eResultCode = LLRP_RC_MiscError;
            pError->pWhatStr    = "missing value";
            pError->pRefType    = pDecoderStream->pRefType;
            pError->pRefField   = pFieldDescriptor;
            pError->OtherDetail = pDecoder->iNext;
        }

        return pDecoder->aIntgValue[0];
    }
    else
    {
        return 0;
    }
}

static llrp_bool_t
getFieldIntgVec (
  LLRP_tSXMLTextDecoderStream * pDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor,
  unsigned int                  nValueMax,
  llrp_s64_t                    minValue,
  llrp_s64_t                    maxValue)
{
    LLRP_tSXMLTextDecoder *     pDecoder = pDecoderStream->pDecoder;
    LLRP_tSErrorDetails *       pError = &pDecoder->decoderHdr.ErrorDetails;
    const char *                pFieldName = pFieldDescriptor->pName;
    unsigned int                iSave;

    pDecoder->nIntgValue = 0;

    if(0 == nValueMax)
    {
        nValueMax = 200;
    }

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return FALSE;
    }

    skipInsignificantText(pDecoder);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return FALSE;
    }

    if(!matchStartTag(pDecoder, pFieldName))
    {
        if(LLRP_RC_OK == pError->eResultCode)
        {
            pError->eResultCode = LLRP_RC_MiscError;
            pError->pWhatStr    = "missing field start tag";
            pError->pRefType    = pDecoderStream->pRefType;
            pError->pRefField   = pFieldDescriptor;
            pError->OtherDetail = pDecoder->iNext;
        }

        return FALSE;
    }

    pDecoder->iLastText = pDecoder->iNext;

    for(;;)
    {
        int                     c = getTextChar(pDecoder);
        int                     bNeg = FALSE;
        unsigned int            nDigit = 0;
        llrp_s64_t              intgValue = 0;

        if(LLRP_RC_OK != pError->eResultCode)
        {
            return FALSE;
        }

        if(0 > c)
        {
            break;
        }

        if(isspace(c))
        {
            continue;
        }

        iSave = pDecoder->iLastGetTextChar;

        if('-' == c)
        {
            bNeg = TRUE;
            c = getTextChar(pDecoder);
        }

        while(0 <= c && isdigit(c))
        {
            intgValue *= 10u;
            intgValue += c - '0';
            nDigit++;
            c = getTextChar(pDecoder);
        }

        if(LLRP_RC_OK != pError->eResultCode)
        {
            return FALSE;
        }

        if(0 == nDigit)
        {
            pDecoder->iNext = iSave;

            pError->eResultCode = LLRP_RC_MiscError;
            pError->pWhatStr    = "conversion error";
            pError->pRefType    = pDecoderStream->pRefType;
            pError->pRefField   = pFieldDescriptor;
            pError->OtherDetail = pDecoder->iNext;

            return FALSE;
        }

        if(bNeg)
        {
            intgValue = -intgValue;
        }

        if(intgValue < minValue || maxValue < intgValue)
        {
            pDecoder->iNext = iSave;

            pError->eResultCode = LLRP_RC_MiscError;
            pError->pWhatStr    = "out of range";
            pError->pRefType    = pDecoderStream->pRefType;
            pError->pRefField   = pFieldDescriptor;
            pError->OtherDetail = pDecoder->iNext;

            return FALSE;
        }

        if(256u <= pDecoder->nIntgValue || nValueMax <= pDecoder->nIntgValue)
        {
            pDecoder->iNext = iSave;

            pError->eResultCode = LLRP_RC_MiscError;
            pError->pWhatStr    = "too many values";
            pError->pRefType    = pDecoderStream->pRefType;
            pError->pRefField   = pFieldDescriptor;
            pError->OtherDetail = pDecoder->iNext;

            return FALSE;
        }

        pDecoder->aIntgValue[pDecoder->nIntgValue++] = intgValue;
    }

#if 0
    if(0 == pDecoder->nIntgValue)
    {
        pError->eResultCode = LLRP_RC_MiscError;
        pError->pWhatStr    = "missing value(s)";
        pError->pRefType    = pDecoderStream->pRefType;
        pError->pRefField   = pFieldDescriptor;
        pError->OtherDetail = pDecoder->iNext;

        return FALSE;
    }
#endif

    if(!matchEndTag(pDecoder, pFieldName))
    {
        pError->eResultCode = LLRP_RC_FieldUnderrun;
        pError->pWhatStr    = "missing field end tag";
        pError->pRefType    = pDecoderStream->pRefType;
        pError->pRefField   = pFieldDescriptor;
        pError->OtherDetail = pDecoder->iNext;

        return FALSE;
    }

    return TRUE;
}

static llrp_bool_t
getFieldHexBinary (
  LLRP_tSXMLTextDecoderStream * pDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoder *     pDecoder = pDecoderStream->pDecoder;
    LLRP_tSErrorDetails *       pError = &pDecoder->decoderHdr.ErrorDetails;
    const char *                pFieldName = pFieldDescriptor->pName;
    unsigned int                iSave;
    unsigned int                nDigit = 0;
    llrp_u8_t                   intgValue = 0;

    pDecoder->nTextBuf = 0;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return FALSE;
    }

    skipInsignificantText(pDecoder);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return FALSE;
    }

    if(!matchStartTag(pDecoder, pFieldName))
    {
        if(LLRP_RC_OK == pError->eResultCode)
        {
            pError->eResultCode = LLRP_RC_MiscError;
            pError->pWhatStr    = "missing field start tag";
            pError->pRefType    = pDecoderStream->pRefType;
            pError->pRefField   = pFieldDescriptor;
            pError->OtherDetail = pDecoder->iNext;
        }

        return FALSE;
    }

    pDecoder->iLastText = pDecoder->iNext;

    for(;;)
    {
        int                     c = getTextChar(pDecoder);

        if(LLRP_RC_OK != pError->eResultCode)
        {
            return FALSE;
        }

        if(0 > c)
        {
            break;
        }

        if(isspace(c))
        {
            continue;
        }

        iSave = pDecoder->iLastGetTextChar;

        if('0' <= c && c <= '9')
        {
            c -= '0';
        }
        else if('A' <= c && c <= 'F')
        {
            c -= 'A';
            c += 10;
        }
        else if('a' <= c && c <= 'f')
        {
            c -= 'a';
            c += 10;
        }
        else
        {
            pDecoder->iNext = iSave;

            pError->eResultCode = LLRP_RC_MiscError;
            pError->pWhatStr    = "conversion error";
            pError->pRefType    = pDecoderStream->pRefType;
            pError->pRefField   = pFieldDescriptor;
            pError->OtherDetail = pDecoder->iNext;

            return FALSE;
        }

        intgValue <<= 4u;
        intgValue += c;
        nDigit++;

        if(2 == nDigit)
        {
            pDecoder->aTextBuf[pDecoder->nTextBuf++] = intgValue;
            intgValue = 0;
            nDigit = 0;
        }
    }

    if(nDigit != 0)
    {
        intgValue <<= 4u;
        pDecoder->aTextBuf[pDecoder->nTextBuf++] = intgValue;
    }

    if(!matchEndTag(pDecoder, pFieldName))
    {
        pError->eResultCode = LLRP_RC_FieldUnderrun;
        pError->pWhatStr    = "missing field end tag";
        pError->pRefType    = pDecoderStream->pRefType;
        pError->pRefField   = pFieldDescriptor;
        pError->OtherDetail = pDecoder->iNext;

        return FALSE;
    }

    return TRUE;
}

static int
getFieldEnum (
  LLRP_tSXMLTextDecoderStream * pDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSXMLTextDecoder *     pDecoder = pDecoderStream->pDecoder;
    LLRP_tSErrorDetails *       pError = &pDecoder->decoderHdr.ErrorDetails;
    const LLRP_tSEnumTableEntry *pEntry;
    unsigned char *             pScan;

    if(!getFieldText(pDecoderStream, pFieldDescriptor))
    {
        return 0;
    }

    /*
     * Trim trailing whitespace
     */
    pScan = strchr(pDecoder->aTextBuf, 0);
    while(pScan > pDecoder->aTextBuf && isspace(pScan[-1]))
    {
        *--pScan = 0;
    }

    /*
     * Skip over leading white space
     */
    pScan = pDecoder->aTextBuf;
    while(isspace(*pScan))
    {
        pScan++;
    }

    for(
        pEntry = pFieldDescriptor->pEnumTable;
        NULL != pEntry->pName;
        pEntry++)
    {
        if(0 == strcmp(pEntry->pName, pScan))
        {
            break;
        }
    }

    if(NULL == pEntry->pName)
    {
        /*
         * TODO: check for intg value
         */

        if(LLRP_RC_OK == pError->eResultCode)
        {
            pDecoder->iNext = pDecoder->iLastText;

            pError->eResultCode = LLRP_RC_MiscError;
            pError->pWhatStr    = "invalid value";
            pError->pRefType    = pDecoderStream->pRefType;
            pError->pRefField   = pFieldDescriptor;
            pError->OtherDetail = pDecoder->iNext;
        }

        return 0;
    }

    return pEntry->Value;
}

static llrp_bool_t
verifyVectorAllocation (
  LLRP_tSXMLTextDecoderStream * pDecoderStream,
  const void *                  pValue,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    if(NULL == pValue)
    {
        LLRP_tSXMLTextDecoder * pDecoder = pDecoderStream->pDecoder;
        LLRP_tSErrorDetails *   pError = &pDecoder->decoderHdr.ErrorDetails;

        pError->eResultCode = LLRP_RC_FieldAllocationFailed;
        pError->pWhatStr    = "field allocation failed";
        pError->pRefType    = pDecoderStream->pRefType;
        pError->pRefField   = pFieldDescriptor;
        pError->OtherDetail = pDecoder->iNext;

        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
