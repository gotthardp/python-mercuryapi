
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

#include "ltkc_platform.h"
#include "ltkc_base.h"
#include "ltkc_frame.h"


/*
 * BEGIN forward decls
 */

LLRP_tSFrameDecoder *
LLRP_FrameDecoder_construct (
  const LLRP_tSTypeRegistry *   pTypeRegistry,
  unsigned char *               pBuffer,
  unsigned int                  nBuffer);

static void
decoderDestruct (
  LLRP_tSDecoder *              pBaseDecoder);

static LLRP_tSMessage *
topDecodeMessage (
  LLRP_tSDecoder *              pBaseDecoder);

static llrp_u8_t
next_u8 (
  LLRP_tSFrameDecoder *         pDecoder);

static llrp_u16_t
next_u16 (
  LLRP_tSFrameDecoder *         pDecoder);

static llrp_u32_t
next_u32 (
  LLRP_tSFrameDecoder *         pDecoder);

static llrp_u64_t
next_u64 (
  LLRP_tSFrameDecoder *         pDecoder);

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

static llrp_u8v_t
get_e8v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static void
get_reserved (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  unsigned int                  nBit);

static void
streamConstruct_outermost (
  LLRP_tSFrameDecoderStream *   pDecoderStream,
  LLRP_tSFrameDecoder *         pDecoder);

static void
streamConstruct_nested (
  LLRP_tSFrameDecoderStream *   pDecoderStream,
  LLRP_tSFrameDecoderStream *   pEnclosingDecoderStream);

static LLRP_tSMessage *
decodeMessage (
  LLRP_tSFrameDecoderStream *   pDecoderStream);

static LLRP_tSParameter *
decodeParameter (
  LLRP_tSFrameDecoderStream *   pDecoderStream);

static unsigned int
getRemainingByteCount (
  LLRP_tSFrameDecoderStream *   pDecoderStream);

static llrp_bool_t
checkAvailable (
  LLRP_tSFrameDecoderStream *   pDecoderStream,
  unsigned int                  nByte,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static unsigned int
getBitField (
  LLRP_tSFrameDecoderStream *   pDecoderStream,
  unsigned int                  nBit,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_u16_t
getVarlenCount (
  LLRP_tSFrameDecoderStream *   pDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_bool_t
verifyVectorAllocation (
  LLRP_tSFrameDecoderStream *   pDecoderStream,
  const void *                  pValue,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

/*
 * END forward decls
 */



static LLRP_tSDecoderOps
s_FrameDecoderOps =
{
    decoderDestruct,	// .pfDestruct
    topDecodeMessage,	// .pfDecodeMessage
};

static LLRP_tSDecoderStreamOps
s_FrameDecoderStreamOps =
{
    get_u8,	// .pfGet_u8
    get_s8,	// .pfGet_s8
    get_u8v,	// .pfGet_u8v
    get_s8v,	// .pfGet_s8v

    get_u16,	// .pfGet_u16
    get_s16,	// .pfGet_s16
    get_u16v,	// .pfGet_u16v
    get_s16v,	// .pfGet_s16v

    get_u32,	// .pfGet_u32
    get_s32,	// .pfGet_s32
    get_u32v,	// .pfGet_u32v
    get_s32v,	// .pfGet_s32v

    get_u64,	// .pfGet_u64
    get_s64,	// .pfGet_s64
    get_u64v,	// .pfGet_u64v
    get_s64v,	// .pfGet_s64v

    get_u1,	// .pfGet_u1
    get_u1v,	// .pfGet_u1v
    get_u2,	// .pfGet_u2
    get_u96,	// .pfGet_u96
    get_utf8v,	// .pfGet_utf8v
    get_bytesToEnd,	// .pfGet_bytesToEnd

    get_e1,	// .pfGet_e1
    get_e2,	// .pfGet_e2
    get_e8,	// .pfGet_e8
    get_e16,	// .pfGet_e16
    get_e32,	// .pfGet_e32
    get_e8v,	// .pfGet_e8v

    get_reserved,	// .pfGet_reserved
};

LLRP_tSFrameDecoder *
LLRP_FrameDecoder_construct (
  const LLRP_tSTypeRegistry *   pTypeRegistry,
  unsigned char *               pBuffer,
  unsigned int                  nBuffer)
{
    LLRP_tSFrameDecoder *       pDecoder;

    pDecoder =(LLRP_tSFrameDecoder *)malloc(sizeof *pDecoder);  //changes
    if(NULL == pDecoder)
    {
        return pDecoder;
    }

    memset(pDecoder, 0, sizeof *pDecoder);

    pDecoder->decoderHdr.pDecoderOps = &s_FrameDecoderOps;
    pDecoder->decoderHdr.pRegistry = pTypeRegistry;

    pDecoder->pBuffer        = pBuffer;
    pDecoder->nBuffer        = nBuffer;

    pDecoder->iNext          = 0;
    pDecoder->BitFieldBuffer = 0;
    pDecoder->nBitFieldResid = 0;

    return pDecoder;
}

static void
decoderDestruct (
  LLRP_tSDecoder *              pBaseDecoder)
{
    LLRP_tSFrameDecoder *       pDecoder = (LLRP_tSFrameDecoder*)pBaseDecoder;

    free(pDecoder);
}

LLRP_tSMessage *
topDecodeMessage (
  LLRP_tSDecoder *              pBaseDecoder)
{
    LLRP_tSFrameDecoder *       pDecoder;
    LLRP_tSFrameDecoderStream   DecoderStream;
    LLRP_tSMessage *            pMessage;

    pDecoder = (LLRP_tSFrameDecoder *) pBaseDecoder;

    streamConstruct_outermost(&DecoderStream, pDecoder);

    pMessage = decodeMessage(&DecoderStream);

    return pMessage;
}

static llrp_u8_t
next_u8 (
  LLRP_tSFrameDecoder *         pDecoder)
{
    llrp_u8_t                   Value;

    assert(pDecoder->iNext + 1u <= pDecoder->nBuffer);

    Value = pDecoder->pBuffer[pDecoder->iNext++];

    return Value;
}

static llrp_u16_t
next_u16 (
  LLRP_tSFrameDecoder *         pDecoder)
{
    llrp_u16_t                  Value;

    assert(pDecoder->iNext + 2u <= pDecoder->nBuffer);

    Value = pDecoder->pBuffer[pDecoder->iNext++];
    Value <<= 8u;
    Value |= pDecoder->pBuffer[pDecoder->iNext++];

    return Value;
}

static llrp_u32_t
next_u32 (
  LLRP_tSFrameDecoder *         pDecoder)
{
    llrp_u32_t                  Value;

    assert(pDecoder->iNext + 4u <= pDecoder->nBuffer);

    Value = pDecoder->pBuffer[pDecoder->iNext++];
    Value <<= 8u;
    Value |= pDecoder->pBuffer[pDecoder->iNext++];
    Value <<= 8u;
    Value |= pDecoder->pBuffer[pDecoder->iNext++];
    Value <<= 8u;
    Value |= pDecoder->pBuffer[pDecoder->iNext++];

    return Value;
}

static llrp_u64_t
next_u64 (
  LLRP_tSFrameDecoder *         pDecoder)
{
    llrp_u64_t                  Value;

    assert(pDecoder->iNext + 8u <= pDecoder->nBuffer);

    Value = pDecoder->pBuffer[pDecoder->iNext++];
    Value <<= 8u;
    Value |= pDecoder->pBuffer[pDecoder->iNext++];
    Value <<= 8u;
    Value |= pDecoder->pBuffer[pDecoder->iNext++];
    Value <<= 8u;
    Value |= pDecoder->pBuffer[pDecoder->iNext++];

    Value <<= 8u;
    Value |= pDecoder->pBuffer[pDecoder->iNext++];
    Value <<= 8u;
    Value |= pDecoder->pBuffer[pDecoder->iNext++];
    Value <<= 8u;
    Value |= pDecoder->pBuffer[pDecoder->iNext++];
    Value <<= 8u;
    Value |= pDecoder->pBuffer[pDecoder->iNext++];

    return Value;
}


static llrp_u8_t
get_u8 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_u8_t                   Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;

    if(checkAvailable(pDecoderStream, 1u, pFieldDescriptor))
    {
        Value = next_u8(pDecoderStream->pDecoder);
    }
    else
    {
        Value = 0;
    }

    return Value;
}

static llrp_s8_t
get_s8 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_s8_t                   Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;

    if(checkAvailable(pDecoderStream, 1u, pFieldDescriptor))
    {
        Value = next_u8(pDecoderStream->pDecoder);
    }
    else
    {
        Value = 0;
    }

    return Value;
}

static llrp_u8v_t
get_u8v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_u16_t                  nValue;
    llrp_u8v_t                  Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    nValue = getVarlenCount(pDecoderStream, pFieldDescriptor);

    if(0 < nValue)
    {
        if(checkAvailable(pDecoderStream, 1u * nValue, pFieldDescriptor))
        {
            Value = LLRP_u8v_construct(nValue);
            if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                                pFieldDescriptor))
            {
                unsigned int    Ix;

                for(Ix = 0; Ix < nValue; Ix++)
                {
                    Value.pValue[Ix] = next_u8(pDecoderStream->pDecoder);
                }
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
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_u16_t                  nValue;
    llrp_s8v_t                  Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    nValue = getVarlenCount(pDecoderStream, pFieldDescriptor);

    if(0 < nValue)
    {
        if(checkAvailable(pDecoderStream, 1u * nValue, pFieldDescriptor))
        {
            Value = LLRP_s8v_construct(nValue);
            if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                                pFieldDescriptor))
            {
                unsigned int    Ix;

                for(Ix = 0; Ix < nValue; Ix++)
                {
                    Value.pValue[Ix] = next_u8(pDecoderStream->pDecoder);
                }
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
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_u16_t                  Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;

    if(checkAvailable(pDecoderStream, 2u, pFieldDescriptor))
    {
        Value = next_u16(pDecoderStream->pDecoder);
    }
    else
    {
        Value = 0;
    }

    return Value;
}

static llrp_s16_t
get_s16 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_s16_t                  Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;

    if(checkAvailable(pDecoderStream, 2u, pFieldDescriptor))
    {
        Value = next_u16(pDecoderStream->pDecoder);
    }
    else
    {
        Value = 0;
    }

    return Value;
}

static llrp_u16v_t
get_u16v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_u16_t                  nValue;
    llrp_u16v_t                 Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    nValue = getVarlenCount(pDecoderStream, pFieldDescriptor);

    if(0 < nValue)
    {
        if(checkAvailable(pDecoderStream, 2u * nValue, pFieldDescriptor))
        {
            Value = LLRP_u16v_construct(nValue);
            if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                                pFieldDescriptor))
            {
                unsigned int    Ix;

                for(Ix = 0; Ix < nValue; Ix++)
                {
                    Value.pValue[Ix] = next_u16(pDecoderStream->pDecoder);
                }
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
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_u16_t                  nValue;
    llrp_s16v_t                 Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    nValue = getVarlenCount(pDecoderStream, pFieldDescriptor);

    if(0 < nValue)
    {
        if(checkAvailable(pDecoderStream, 2u * nValue, pFieldDescriptor))
        {
            Value = LLRP_s16v_construct(nValue);
            if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                                pFieldDescriptor))
            {
                unsigned int    Ix;

                for(Ix = 0; Ix < nValue; Ix++)
                {
                    Value.pValue[Ix] = next_u16(pDecoderStream->pDecoder);
                }
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
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_u32_t                  Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;

    if(checkAvailable(pDecoderStream, 4u, pFieldDescriptor))
    {
        Value = next_u32(pDecoderStream->pDecoder);
    }
    else
    {
        Value = 0;
    }

    return Value;
}

static llrp_s32_t
get_s32 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_s32_t                  Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;

    if(checkAvailable(pDecoderStream, 4u, pFieldDescriptor))
    {
        Value = next_u32(pDecoderStream->pDecoder);
    }
    else
    {
        Value = 0;
    }

    return Value;
}

static llrp_u32v_t
get_u32v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_u16_t                  nValue;
    llrp_u32v_t                 Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    nValue = getVarlenCount(pDecoderStream, pFieldDescriptor);

    if(0 < nValue)
    {
        if(checkAvailable(pDecoderStream, 4u * nValue, pFieldDescriptor))
        {
            Value = LLRP_u32v_construct(nValue);
            if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                                pFieldDescriptor))
            {
                unsigned int    Ix;

                for(Ix = 0; Ix < nValue; Ix++)
                {
                    Value.pValue[Ix] = next_u32(pDecoderStream->pDecoder);
                }
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
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_u16_t                  nValue;
    llrp_s32v_t                 Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    nValue = getVarlenCount(pDecoderStream, pFieldDescriptor);

    if(0 < nValue)
    {
        if(checkAvailable(pDecoderStream, 4u * nValue, pFieldDescriptor))
        {
            Value = LLRP_s32v_construct(nValue);
            if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                                pFieldDescriptor))
            {
                unsigned int    Ix;

                for(Ix = 0; Ix < nValue; Ix++)
                {
                    Value.pValue[Ix] = next_u32(pDecoderStream->pDecoder);
                }
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
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_u64_t                  Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;

    if(checkAvailable(pDecoderStream, 8u, pFieldDescriptor))
    {
        Value = next_u64(pDecoderStream->pDecoder);
    }
    else
    {
        Value = 0;
    }

    return Value;
}

static llrp_s64_t
get_s64 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_s64_t                  Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;

    if(checkAvailable(pDecoderStream, 8u, pFieldDescriptor))
    {
        Value = next_u64(pDecoderStream->pDecoder);
    }
    else
    {
        Value = 0;
    }

    return Value;
}

static llrp_u64v_t
get_u64v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_u16_t                  nValue;
    llrp_u64v_t                 Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    nValue = getVarlenCount(pDecoderStream, pFieldDescriptor);

    if(0 < nValue)
    {
        if(checkAvailable(pDecoderStream, 8u * nValue, pFieldDescriptor))
        {
            Value = LLRP_u64v_construct(nValue);
            if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                                pFieldDescriptor))
            {
                unsigned int    Ix;

                for(Ix = 0; Ix < nValue; Ix++)
                {
                    Value.pValue[Ix] = next_u64(pDecoderStream->pDecoder);
                }
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
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_u16_t                  nValue;
    llrp_s64v_t                 Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    nValue = getVarlenCount(pDecoderStream, pFieldDescriptor);

    if(0 < nValue)
    {
        if(checkAvailable(pDecoderStream, 8u * nValue, pFieldDescriptor))
        {
            Value = LLRP_s64v_construct(nValue);
            if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                                pFieldDescriptor))
            {
                unsigned int    Ix;

                for(Ix = 0; Ix < nValue; Ix++)
                {
                    Value.pValue[Ix] = next_u64(pDecoderStream->pDecoder);
                }
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
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_u1_t                   Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;

    Value = getBitField(pDecoderStream, 1, pFieldDescriptor);

    return Value;
}

static llrp_u1v_t
get_u1v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_u16_t                  nBit;
    llrp_u1v_t                  Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    nBit = getVarlenCount(pDecoderStream, pFieldDescriptor);

    if(0 < nBit)
    {
        unsigned int            nByte = (nBit + 7u) / 8u;

        if(checkAvailable(pDecoderStream, nByte, pFieldDescriptor))
        {
            Value = LLRP_u1v_construct(nBit);
            if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                                pFieldDescriptor))
            {
                unsigned int    Ix;

                for(Ix = 0; Ix < nByte; Ix++)
                {
                    Value.pValue[Ix] = next_u8(pDecoderStream->pDecoder);
                }
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
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_u2_t                   Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;

    Value = getBitField(pDecoderStream, 2, pFieldDescriptor);

    return Value;
}

static llrp_u96_t
get_u96 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_u96_t                  Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;

    if(checkAvailable(pDecoderStream, 12u, pFieldDescriptor))
    {
        unsigned int            Ix;

        for(Ix = 0; Ix < 12u; Ix++)
        {
            Value.aValue[Ix] = next_u8(pDecoderStream->pDecoder);
        }
    }
    else
    {
        memset(&Value, 0, sizeof Value);
    }

    return Value;
}

static llrp_utf8v_t
get_utf8v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_u16_t                  nValue;
    llrp_utf8v_t                Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    nValue = getVarlenCount(pDecoderStream, pFieldDescriptor);

    if(0 < nValue)
    {
        if(checkAvailable(pDecoderStream, 1u * nValue, pFieldDescriptor))
        {
            Value = LLRP_utf8v_construct(nValue);
            if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                                pFieldDescriptor))
            {
                unsigned int    Ix;

                for(Ix = 0; Ix < nValue; Ix++)
                {
                    Value.pValue[Ix] = next_u8(pDecoderStream->pDecoder);
                }
            }
        }
    }

    return Value;
}

static llrp_bytesToEnd_t
get_bytesToEnd (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameDecoderStream * pDecoderStream;
    llrp_u16_t                  nValue;
    llrp_bytesToEnd_t           Value;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;
    memset(&Value, 0, sizeof Value);

    nValue = getRemainingByteCount(pDecoderStream);

    if(0 < nValue)
    {
        if(checkAvailable(pDecoderStream, 1u * nValue, pFieldDescriptor))
        {
            Value = LLRP_bytesToEnd_construct(nValue);
            if(verifyVectorAllocation(pDecoderStream, Value.pValue,
                                pFieldDescriptor))
            {
                unsigned int    Ix;

                for(Ix = 0; Ix < nValue; Ix++)
                {
                    Value.pValue[Ix] = next_u8(pDecoderStream->pDecoder);
                }
            }
        }
    }

    return Value;
}

static int
get_e1 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    int                         eValue;

    eValue = (int)get_u1(pBaseDecoderStream, pFieldDescriptor);

    return eValue;
}

static int
get_e2 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    int                         eValue;

    eValue = (int)get_u2(pBaseDecoderStream, pFieldDescriptor);

    return eValue;
}

static int
get_e8 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    int                         eValue;

    eValue = (int)get_u8(pBaseDecoderStream, pFieldDescriptor);

    return eValue;
}

static int
get_e16 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    int                         eValue;

    eValue = (int)get_u16(pBaseDecoderStream, pFieldDescriptor);

    return eValue;
}

static int
get_e32 (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    int                         eValue;

    eValue = (int)get_u32(pBaseDecoderStream, pFieldDescriptor);

    return eValue;
}

static llrp_u8v_t
get_e8v (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    return get_u8v(pBaseDecoderStream, pFieldDescriptor);
}

static void
get_reserved (
  LLRP_tSDecoderStream *        pBaseDecoderStream,
  unsigned int                  nBit)
{
    LLRP_tSFrameDecoderStream * pDecoderStream;
    LLRP_tSFrameDecoder *       pDecoder;
    LLRP_tSErrorDetails *       pError;

    pDecoderStream = (LLRP_tSFrameDecoderStream *) pBaseDecoderStream;
    pDecoder = pDecoderStream->pDecoder;
    pError = &pDecoder->decoderHdr.ErrorDetails;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return;
    }

    while(0 < nBit)
    {
        unsigned int            Step = 7u & nBit;

        if(0 != pDecoder->nBitFieldResid)
        {
            if(Step != pDecoder->nBitFieldResid)
            {
                pError->eResultCode = LLRP_RC_UnalignedReservedBits;
                pError->pWhatStr    = "unaligned reserved bits";
                pError->pRefType    = pDecoderStream->pRefType;
                pError->pRefField   = NULL;
                pError->OtherDetail = pDecoder->iNext;
                return;
            }

            nBit -= Step;
            pDecoder->nBitFieldResid = 0;
        }
        else
        {
            if(0 != Step)
            {
                pError->eResultCode = LLRP_RC_UnalignedReservedBits;
                pError->pWhatStr    = "unaligned reserved bits";
                pError->pRefType    = pDecoderStream->pRefType;
                pError->pRefField   = NULL;
                pError->OtherDetail = pDecoder->iNext;
                return;
            }

            if(pDecoder->iNext >= pDecoderStream->iLimit)
            {
                pError->eResultCode = LLRP_RC_ReservedBitsUnderrun;
                pError->pWhatStr    = "underrun at reserved bits";
                pError->pRefType    = pDecoderStream->pRefType;
                pError->pRefField   = NULL;
                pError->OtherDetail = pDecoder->iNext;
                return;
            }

            next_u8(pDecoder);
            nBit -= 8;
        }
    }
}

static void
streamConstruct_outermost (
  LLRP_tSFrameDecoderStream *   pDecoderStream,
  LLRP_tSFrameDecoder *         pDecoder)
{
    memset(pDecoderStream, 0, sizeof *pDecoderStream);
    pDecoderStream->decoderStreamHdr.pDecoderStreamOps =
                                &s_FrameDecoderStreamOps;

    pDecoderStream->pDecoder                = pDecoder;
    pDecoderStream->pEnclosingDecoderStream = NULL;
    pDecoderStream->pRefType                = NULL;
    pDecoderStream->iBegin                  = pDecoder->iNext;
    pDecoderStream->iLimit                  = pDecoder->nBuffer;
}

static void
streamConstruct_nested (
  LLRP_tSFrameDecoderStream *   pDecoderStream,
  LLRP_tSFrameDecoderStream *   pEnclosingDecoderStream)
{
    LLRP_tSFrameDecoder *       pDecoder;

    pDecoder = pEnclosingDecoderStream->pDecoder;

    memset(pDecoderStream, 0, sizeof *pDecoderStream);
    pDecoderStream->decoderStreamHdr.pDecoderStreamOps =
                                &s_FrameDecoderStreamOps;

    pDecoderStream->pDecoder                = pDecoder;
    pDecoderStream->pEnclosingDecoderStream = pEnclosingDecoderStream;
    pDecoderStream->pRefType                = NULL;
    pDecoderStream->iBegin                  = pDecoder->iNext;
    pDecoderStream->iLimit                  = pEnclosingDecoderStream->iLimit;
}

static LLRP_tSMessage *
decodeMessage (
  LLRP_tSFrameDecoderStream *   pDecoderStream)
{
    LLRP_tSFrameDecoder *       pDecoder  = pDecoderStream->pDecoder;
    LLRP_tSErrorDetails *       pError    = &pDecoder->decoderHdr.ErrorDetails;
    const LLRP_tSTypeRegistry * pRegistry = pDecoder->decoderHdr.pRegistry;
    LLRP_tSDecoderStream *      pBaseDecoderStream =
                                        &pDecoderStream->decoderStreamHdr;
    const LLRP_tSTypeDescriptor *pTypeDescriptor;
    llrp_u16_t                  Type;
    llrp_u16_t                  Vers;
    llrp_u32_t                  nLength;
    unsigned int                iLimit;
    llrp_u32_t                  MessageID;
    LLRP_tSElement *            pElement;
    LLRP_tSMessage *            pMessage;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return NULL;
    }

    Type = get_u16(pBaseDecoderStream, &LLRP_g_fdMessageHeader_Type);
    Vers = (Type >> 10u) & 3;
    Type &= 0x3FF;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return NULL;
    }

    if(1u != Vers)
    {
        pError->eResultCode = LLRP_RC_BadVersion;
        pError->pWhatStr    = "unsupported version";
        pError->pRefType    = NULL;
        pError->pRefField   = &LLRP_g_fdMessageHeader_Type;
        pError->OtherDetail = pDecoder->iNext;
        return NULL;
    }

    nLength = get_u32(pBaseDecoderStream, &LLRP_g_fdMessageHeader_Length);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return NULL;
    }

    if(10u > nLength)
    {
        pError->eResultCode = LLRP_RC_InvalidLength;
        pError->pWhatStr    = "message length too small";
        pError->pRefType    = NULL;
        pError->pRefField   = &LLRP_g_fdMessageHeader_Length;
        pError->OtherDetail = pDecoder->iNext;
        return NULL;
    }

    iLimit = pDecoderStream->iBegin + nLength;

    if(iLimit > pDecoderStream->iLimit)
    {
        pError->eResultCode = LLRP_RC_ExcessiveLength;
        pError->pWhatStr    = "message length exceeds enclosing length";
        pError->pRefType    = NULL;
        pError->pRefField   = &LLRP_g_fdMessageHeader_Length;
        pError->OtherDetail = pDecoder->iNext;
        return NULL;
    }

    pDecoderStream->iLimit = iLimit;

    MessageID = get_u32(pBaseDecoderStream, &LLRP_g_fdMessageHeader_MessageID);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return NULL;
    }

    /* Custom? */
    if(1023u == Type)
    {
        llrp_u32_t              VendorPEN;
        llrp_u8_t               Subtype;

        VendorPEN = get_u32(pBaseDecoderStream,
                            &LLRP_g_fdMessageHeader_VendorPEN);
        Subtype   = get_u8(pBaseDecoderStream,
                            &LLRP_g_fdMessageHeader_Subtype);

        if(LLRP_RC_OK != pError->eResultCode)
        {
            return NULL;
        }

        pTypeDescriptor = LLRP_TypeRegistry_lookupCustomMessage(pRegistry,
            VendorPEN, Subtype);
        if(NULL == pTypeDescriptor)
        {
            /*
             * If we don't have a definition for a particular
             * CUSTOM message, just use the generic one.
             */
            pDecoder->iNext -= 5;       /* back up to VendorPEN and SubType */
            pTypeDescriptor = LLRP_TypeRegistry_lookupMessage(pRegistry, Type);
        }
    }
    else
    {
        pTypeDescriptor = LLRP_TypeRegistry_lookupMessage(pRegistry, Type);
    }

    if(NULL == pTypeDescriptor)
    {
        pError->eResultCode = LLRP_RC_UnknownMessageType;
        pError->pWhatStr    = "unknown message type";
        pError->pRefType    = NULL;
        pError->pRefField   = &LLRP_g_fdMessageHeader_Type;
        pError->OtherDetail = 0;
        return NULL;
    }

    pDecoderStream->pRefType = pTypeDescriptor;

    pElement = LLRP_Element_construct(pTypeDescriptor);

    if(NULL == pElement)
    {
        pError->eResultCode = LLRP_RC_MessageAllocationFailed;
        pError->pWhatStr    = "message allocation failed";
        pError->pRefType    = pTypeDescriptor;
        pError->pRefField   = NULL;
        pError->OtherDetail = pDecoder->iNext;
        return NULL;
    }

    pMessage = (LLRP_tSMessage *) pElement;
    pMessage->MessageID = MessageID;

    pTypeDescriptor->pfDecodeFields(pElement, pBaseDecoderStream);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        LLRP_Element_destruct(pElement);
        return NULL;
    }

    /*
     * Subparameters
     */
    while(0 < getRemainingByteCount(pDecoderStream) &&
          LLRP_RC_OK == pError->eResultCode)
    {
        LLRP_tSFrameDecoderStream       NestStream;
        LLRP_tSParameter *              pParameter;

        streamConstruct_nested(&NestStream, pDecoderStream);

        pParameter = decodeParameter(&NestStream);

        if(NULL == pParameter)
        {
            if(LLRP_RC_OK == pError->eResultCode)
            {
                pError->eResultCode = LLRP_RC_Botch;
                pError->pWhatStr    = "botch -- no param and no error";
                pError->pRefType    = pTypeDescriptor;
                pError->pRefField   = NULL;
                pError->OtherDetail = pDecoder->iNext;
            }
            break;
        }

        pParameter->elementHdr.pParent = pElement;
        LLRP_Element_addSubParameterToAllList(pElement, pParameter);
    }

    if(LLRP_RC_OK == pError->eResultCode)
    {
        if(pDecoder->iNext != pDecoderStream->iLimit)
        {
            pError->eResultCode = LLRP_RC_ExtraBytes;
            pError->pWhatStr    = "extra bytes at end of message";
            pError->pRefType    = pTypeDescriptor;
            pError->pRefField   = NULL;
            pError->OtherDetail = pDecoder->iNext;
        }
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

    return pMessage;
}

static LLRP_tSParameter *
decodeParameter (
  LLRP_tSFrameDecoderStream *   pDecoderStream)
{
    LLRP_tSFrameDecoder *       pDecoder  = pDecoderStream->pDecoder;
    LLRP_tSErrorDetails *       pError    = &pDecoder->decoderHdr.ErrorDetails;
    const LLRP_tSTypeRegistry * pRegistry = pDecoder->decoderHdr.pRegistry;
    LLRP_tSDecoderStream *      pBaseDecoderStream =
                                        &pDecoderStream->decoderStreamHdr;
    const LLRP_tSTypeDescriptor *pTypeDescriptor;
    llrp_u16_t                  Type;
    llrp_bool_t                 bIsTV;
    LLRP_tSElement *            pElement;
    LLRP_tSParameter *          pParameter;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return NULL;
    }

    Type = get_u8(pBaseDecoderStream, &LLRP_g_fdParameterHeader_TVType);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return NULL;
    }

    if(0 != (Type&0x80))
    {
        /*
         * Type-Value (TV).
         * All we can do is inherit the length from
         * the enclosing element.
         */
        Type &= 0x7F;
        bIsTV = TRUE;
    }
    else
    {
        llrp_u16_t              nLength;
        unsigned int            iLimit;

        /*
         * Type-Length-Value (TLV).
         * Back up and get the real type number,
         * then get the length.
         */
        pDecoder->iNext--;
        Type = get_u16(pBaseDecoderStream,
                    &LLRP_g_fdParameterHeader_TLVType);
        Type &= 0x3FF;

        if(LLRP_RC_OK != pError->eResultCode)
        {
            return NULL;
        }

        nLength = get_u16(pBaseDecoderStream,
                    &LLRP_g_fdParameterHeader_TLVLength);

        if(LLRP_RC_OK != pError->eResultCode)
        {
            return NULL;
        }

        if(4u > nLength)
        {
            pError->eResultCode = LLRP_RC_InvalidLength;
            pError->pWhatStr    = "TLV parameter length too small";
            pError->pRefType    = NULL;
            pError->pRefField   = &LLRP_g_fdParameterHeader_TLVLength;
            pError->OtherDetail = pDecoder->iNext;
            return NULL;
        }

        iLimit = pDecoderStream->iBegin + nLength;

        if(iLimit > pDecoderStream->iLimit)
        {
            pError->eResultCode = LLRP_RC_ExcessiveLength;
            pError->pWhatStr    =
                            "TLV parameter length exceeds enclosing length";
            pError->pRefType    = NULL;
            pError->pRefField   = &LLRP_g_fdParameterHeader_TLVLength;
            pError->OtherDetail = pDecoder->iNext;
            return NULL;
        }

        pDecoderStream->iLimit = iLimit;

        bIsTV = FALSE;
    }

    /* Custom? */
    if(1023u == Type)
    {
        llrp_u32_t              VendorPEN;
        llrp_u32_t              Subtype;

        VendorPEN = get_u32(pBaseDecoderStream,
                        &LLRP_g_fdParameterHeader_VendorPEN);
        Subtype   = get_u32(pBaseDecoderStream,
                        &LLRP_g_fdParameterHeader_Subtype);

        if(LLRP_RC_OK != pError->eResultCode)
        {
            return NULL;
        }

        pTypeDescriptor = LLRP_TypeRegistry_lookupCustomParameter(pRegistry,
            VendorPEN, Subtype);
        if(NULL == pTypeDescriptor)
        {
            /*
             * If we don't have a definition for a particular
             * CUSTOM parameter, just use the generic one.
             */
            pDecoder->iNext -= 8;       /* back up to VendorPEN and SubType */
            pTypeDescriptor =
                LLRP_TypeRegistry_lookupParameter(pRegistry, Type);
        }
    }
    else
    {
        pTypeDescriptor = LLRP_TypeRegistry_lookupParameter(pRegistry, Type);
    }

    if(NULL == pTypeDescriptor)
    {
        pError->eResultCode = LLRP_RC_UnknownParameterType;
        pError->pWhatStr    = "unknown parameter type";
        pError->pRefType    = NULL;
        if(bIsTV)
        {
            pError->pRefField = &LLRP_g_fdParameterHeader_TVType;
        }
        else
        {
            pError->pRefField = &LLRP_g_fdParameterHeader_TLVType;
        }
        pError->OtherDetail = pDecoder->iNext;
        return NULL;
    }

    pDecoderStream->pRefType = pTypeDescriptor;

    pElement = LLRP_Element_construct(pTypeDescriptor);

    if(NULL == pElement)
    {
        pError->eResultCode = LLRP_RC_ParameterAllocationFailed;
        pError->pWhatStr    = "parameter allocation failed";
        pError->pRefType    = pTypeDescriptor;
        pError->pRefField   = NULL;
        pError->OtherDetail = pDecoder->iNext;
        return NULL;
    }

    pParameter = (LLRP_tSParameter *) pElement;

    pTypeDescriptor->pfDecodeFields(pElement, pBaseDecoderStream);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        LLRP_Element_destruct(pElement);
        return NULL;
    }

    if(!bIsTV)
    {
        /*
         * Subparameters
         */
        while(0 < getRemainingByteCount(pDecoderStream) &&
              LLRP_RC_OK == pError->eResultCode)
        {
            LLRP_tSFrameDecoderStream       NestStream;
            LLRP_tSParameter *              pSubParameter;

            streamConstruct_nested(&NestStream, pDecoderStream);

            pSubParameter = decodeParameter(&NestStream);

            if(NULL == pSubParameter)
            {
                if(LLRP_RC_OK == pError->eResultCode)
                {
                    pError->eResultCode = LLRP_RC_Botch;
                    pError->pWhatStr    = "botch -- no param and no error";
                    pError->pRefType    = pTypeDescriptor;
                    pError->pRefField   = NULL;
                    pError->OtherDetail = pDecoder->iNext;
                }
                break;
            }

            pSubParameter->elementHdr.pParent = pElement;
            LLRP_Element_addSubParameterToAllList(pElement, pSubParameter);
        }

        if(LLRP_RC_OK == pError->eResultCode)
        {
            if(pDecoder->iNext != pDecoderStream->iLimit)
            {
                pError->eResultCode = LLRP_RC_ExtraBytes;
                pError->pWhatStr    = "extra bytes at end of TLV parameter";
                pError->pRefType    = pTypeDescriptor;
                pError->pRefField   = NULL;
                pError->OtherDetail = pDecoder->iNext;
            }
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
    }

    return pParameter;
}

static unsigned int
getRemainingByteCount (
  LLRP_tSFrameDecoderStream *   pDecoderStream)
{
    LLRP_tSFrameDecoder *       pDecoder = pDecoderStream->pDecoder;

    if(pDecoder->iNext < pDecoderStream->iLimit)
    {
        return pDecoderStream->iLimit - pDecoder->iNext;
    }
    else
    {
        return 0;
    }
}

static llrp_bool_t
checkAvailable (
  LLRP_tSFrameDecoderStream *   pDecoderStream,
  unsigned int                  nByte,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameDecoder *       pDecoder = pDecoderStream->pDecoder;
    LLRP_tSErrorDetails *       pError = &pDecoder->decoderHdr.ErrorDetails;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return FALSE;
    }

    if(pDecoder->iNext + nByte > pDecoderStream->iLimit)
    {
        pError->eResultCode = LLRP_RC_FieldUnderrun;
        pError->pWhatStr    = "underrun at field";
        pError->pRefType    = pDecoderStream->pRefType;
        pError->pRefField   = pFieldDescriptor;
        pError->OtherDetail = pDecoder->iNext;
        return FALSE;
    }

    if(0 != pDecoder->nBitFieldResid)
    {
        pError->eResultCode = LLRP_RC_UnalignedBitField;
        pError->pWhatStr    = "unaligned/incomplete bit field";
        pError->pRefType    = pDecoderStream->pRefType;
        pError->pRefField   = pFieldDescriptor;
        pError->OtherDetail = pDecoder->iNext;
        return FALSE;
    }

    return TRUE;
}

static unsigned int
getBitField (
  LLRP_tSFrameDecoderStream *   pDecoderStream,
  unsigned int                  nBit,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameDecoder *       pDecoder = pDecoderStream->pDecoder;
    LLRP_tSErrorDetails *       pError = &pDecoder->decoderHdr.ErrorDetails;
    unsigned int                Value;

    if(0 == pDecoder->nBitFieldResid)
    {
        if(checkAvailable(pDecoderStream, 1u, pFieldDescriptor))
        {
            pDecoder->BitFieldBuffer = next_u8(pDecoder);
            pDecoder->nBitFieldResid = 8u;
        }
        else
        {
            return 0;
        }
    }

    if(pDecoder->nBitFieldResid < nBit)
    {
        pError->eResultCode = LLRP_RC_UnalignedBitField;
        pError->pWhatStr    = "unaligned/incomplete bit field";
        pError->pRefType    = pDecoderStream->pRefType;
        pError->pRefField   = pFieldDescriptor;
        pError->OtherDetail = pDecoder->iNext;
        return 0;
    }

    pDecoder->nBitFieldResid -= nBit;

    Value = pDecoder->BitFieldBuffer >> pDecoder->nBitFieldResid;
    Value &= (1u << nBit) - 1u;

    return Value;
}

static llrp_u16_t
getVarlenCount (
  LLRP_tSFrameDecoderStream *   pDecoderStream,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    llrp_u16_t                  nValue;

    if(checkAvailable(pDecoderStream, 2u, pFieldDescriptor))
    {
        nValue = next_u16(pDecoderStream->pDecoder);
    }
    else
    {
        nValue = 0;
    }

    return nValue;
}

static llrp_bool_t
verifyVectorAllocation (
  LLRP_tSFrameDecoderStream *   pDecoderStream,
  const void *                  pValue,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    if(NULL == pValue)
    {
        LLRP_tSFrameDecoder *   pDecoder = pDecoderStream->pDecoder;
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

