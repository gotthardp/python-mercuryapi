
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




struct LLRP_SXMLTextDecoder;
struct LLRP_SXMLTextDecoderStream;
struct LLRP_SXMLTextEncoder;
struct LLRP_SXMLTextEncoderStream;

typedef struct LLRP_SXMLTextDecoder         LLRP_tSXMLTextDecoder;
typedef struct LLRP_SXMLTextDecoderStream   LLRP_tSXMLTextDecoderStream;
typedef struct LLRP_SXMLTextEncoder         LLRP_tSXMLTextEncoder;
typedef struct LLRP_SXMLTextEncoderStream   LLRP_tSXMLTextEncoderStream;


struct LLRP_SXMLTextDecoder
{
    LLRP_tSDecoder              decoderHdr;

    unsigned char *             pInput;
    unsigned int                nInput;

    unsigned int                iNext;

    unsigned char               aTextBuf[4u*1024u];
    unsigned int                nTextBuf;

    unsigned int                iLastGetTextChar;
    unsigned int                iLastText;

    unsigned char               aTagBuf[1024u];
    unsigned char *             apTagArg[32u];
    unsigned int                nTagArg;
    enum {
        XMLTEXT_BAD_TAG,
        XMLTEXT_START_TAG,
        XMLTEXT_END_TAG,
        XMLTEXT_EMPTY_TAG
    }                           eTagType;

    llrp_s64_t                  aIntgValue[256u];
    unsigned int                nIntgValue;
};

struct LLRP_SXMLTextDecoderStream
{
    LLRP_tSDecoderStream        decoderStreamHdr;

    LLRP_tSXMLTextDecoder *     pDecoder;
    LLRP_tSXMLTextDecoderStream * pEnclosingDecoderStream;
    const LLRP_tSTypeDescriptor *pRefType;
};

extern LLRP_tSXMLTextDecoder *
LLRP_XMLTextDecoder_construct (
  const LLRP_tSTypeRegistry *   pTypeRegistry,
  unsigned char *               pBuffer,
  unsigned int                  nBuffer);


struct LLRP_SXMLTextEncoder
{
    LLRP_tSEncoder              encoderHdr;

    unsigned char *             pBuffer;
    unsigned int                nBuffer;

    unsigned int                iNext;

    int                         bOverflow;
};

struct LLRP_SXMLTextEncoderStream
{
    LLRP_tSEncoderStream        encoderStreamHdr;

    LLRP_tSXMLTextEncoder *       pEncoder;
    LLRP_tSXMLTextEncoderStream * pEnclosingEncoderStream;
    const LLRP_tSTypeDescriptor *pRefType;
    unsigned int                nDepth;
};

extern LLRP_tSXMLTextEncoder *
LLRP_XMLTextEncoder_construct (
  unsigned char *               pBuffer,
  unsigned int                  nBuffer);
