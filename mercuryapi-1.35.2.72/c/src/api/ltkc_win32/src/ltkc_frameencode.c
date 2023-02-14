
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
 * BEGIN forward declarations
 */

LLRP_tSFrameEncoder *
LLRP_FrameEncoder_construct (
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
next_u8 (
  LLRP_tSFrameEncoder *         pEncoder,
  llrp_u8_t                     Value);

static void
next_u16 (
  LLRP_tSFrameEncoder *         pEncoder,
  llrp_u16_t                    Value);

static void
next_u32 (
  LLRP_tSFrameEncoder *         pEncoder,
  llrp_u32_t                    Value);

static void
next_u64 (
  LLRP_tSFrameEncoder *         pEncoder,
  llrp_u64_t                    Value);

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
put_reserved (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  unsigned int                  nBits);

static void
streamConstruct_outermost (
  LLRP_tSFrameEncoderStream *   pEncoderStream,
  LLRP_tSFrameEncoder *         pEncoder);

static void
streamConstruct_nested (
  LLRP_tSFrameEncoderStream *   pEncoderStream,
  LLRP_tSFrameEncoderStream *   pEnclosingEncoderStream);

static void
putElement (
  LLRP_tSFrameEncoderStream *   pEncoderStream,
  const LLRP_tSElement *        pElement);

static void
nestSubParameter (
  LLRP_tSFrameEncoderStream *   pEncoderStream,
  const LLRP_tSParameter *      pParameter);

static llrp_bool_t
checkAvailable (
  LLRP_tSFrameEncoderStream *   pEncoderStream,
  unsigned int                  nByte,
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

static llrp_bool_t
putBitField (
  LLRP_tSFrameEncoderStream *   pEncoderStream,
  unsigned int                  nBit,
  unsigned int                  Value,  
  const LLRP_tSFieldDescriptor *pFieldDescriptor);

/*
 * END forward declarations
 */


static LLRP_tSEncoderOps
s_FrameEncoderOps =
{
    encoderDestruct,	// .pfDestruct
    encodeElement,	// .pfEncodeElement
};

static LLRP_tSEncoderStreamOps
s_FrameEncoderStreamOps =
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


LLRP_tSFrameEncoder *
LLRP_FrameEncoder_construct (
  unsigned char *               pBuffer,
  unsigned int                  nBuffer)
{
    LLRP_tSFrameEncoder *       pEncoder;

    pEncoder =(LLRP_tSFrameEncoder *)malloc(sizeof *pEncoder);  //changes
    if(NULL == pEncoder)
    {
        return pEncoder;
    }

    memset(pEncoder, 0, sizeof *pEncoder);

    pEncoder->encoderHdr.pEncoderOps = &s_FrameEncoderOps;

    pEncoder->pBuffer        = pBuffer;
    pEncoder->nBuffer        = nBuffer;

    pEncoder->iNext          = 0;
    pEncoder->BitFieldBuffer = 0;
    pEncoder->nBitFieldResid = 0;

    return pEncoder;
}

static void
encoderDestruct (
  LLRP_tSEncoder *              pBaseEncoder)
{
    LLRP_tSFrameEncoder *       pEncoder = (LLRP_tSFrameEncoder*)pBaseEncoder;

    free(pEncoder);
}

static void
encodeElement (
  LLRP_tSEncoder *              pBaseEncoder,
  const LLRP_tSElement *        pElement)
{
    LLRP_tSFrameEncoder *       pEncoder = (LLRP_tSFrameEncoder*)pBaseEncoder;
    LLRP_tSFrameEncoderStream   EncoderStream;

    if(NULL == pElement)
    {
        return;
    }

    streamConstruct_outermost(&EncoderStream, pEncoder);

    putElement(&EncoderStream, pElement);
}

static void
next_u8 (
  LLRP_tSFrameEncoder *         pEncoder,
  llrp_u8_t                     Value)
{
    assert(pEncoder->iNext + 1u <= pEncoder->nBuffer);

    pEncoder->pBuffer[pEncoder->iNext++] = Value;
}

static void
next_u16 (
  LLRP_tSFrameEncoder *         pEncoder,
  llrp_u16_t                    Value)
{
    assert(pEncoder->iNext + 2u <= pEncoder->nBuffer);

    pEncoder->pBuffer[pEncoder->iNext++] = Value >> 8u;
    pEncoder->pBuffer[pEncoder->iNext++] = Value >> 0u;
}

static void
next_u32 (
  LLRP_tSFrameEncoder *         pEncoder,
  llrp_u32_t                    Value)
{
    assert(pEncoder->iNext + 4u <= pEncoder->nBuffer);

    pEncoder->pBuffer[pEncoder->iNext++] = Value >> 24u;
    pEncoder->pBuffer[pEncoder->iNext++] = Value >> 16u;
    pEncoder->pBuffer[pEncoder->iNext++] = Value >> 8u;
    pEncoder->pBuffer[pEncoder->iNext++] = Value >> 0u;
}

static void
next_u64 (
  LLRP_tSFrameEncoder *         pEncoder,
  llrp_u64_t                    Value)
{
    assert(pEncoder->iNext + 8u <= pEncoder->nBuffer);

    pEncoder->pBuffer[pEncoder->iNext++] = Value >> 56u;
    pEncoder->pBuffer[pEncoder->iNext++] = Value >> 48u;
    pEncoder->pBuffer[pEncoder->iNext++] = Value >> 40u;
    pEncoder->pBuffer[pEncoder->iNext++] = Value >> 32u;

    pEncoder->pBuffer[pEncoder->iNext++] = Value >> 24u;
    pEncoder->pBuffer[pEncoder->iNext++] = Value >> 16u;
    pEncoder->pBuffer[pEncoder->iNext++] = Value >> 8u;
    pEncoder->pBuffer[pEncoder->iNext++] = Value >> 0u;
}


static void
putRequiredSubParameter (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const LLRP_tSParameter *      pParameter,
  const LLRP_tSTypeDescriptor * pRefType)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;

    if(NULL == pParameter)
    {
        LLRP_tSFrameEncoder *   pEncoder = pEncoderStream->pEncoder;
        LLRP_tSErrorDetails *   pError = &pEncoder->encoderHdr.ErrorDetails;

        LLRP_Error_missingParameter(pError, pRefType);
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
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;

    nestSubParameter(pEncoderStream, pParameter);
}

static void
putRequiredSubParameterList (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const LLRP_tSParameter *      pParameterList,
  const LLRP_tSTypeDescriptor * pRefType)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    const LLRP_tSParameter *    pParameter;

    if(NULL == pParameterList)
    {
        LLRP_tSFrameEncoder *   pEncoder = pEncoderStream->pEncoder;
        LLRP_tSErrorDetails *   pError = &pEncoder->encoderHdr.ErrorDetails;

        LLRP_Error_missingParameter(pError, pRefType);
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
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
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
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;

    if(checkAvailable(pEncoderStream, 1u, pFieldDescriptor))
    {
        next_u8(pEncoder, Value);
    }
}

static void
put_s8 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s8_t               Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;

    if(checkAvailable(pEncoderStream, 1u, pFieldDescriptor))
    {
        next_u8(pEncoder, Value);
    }
}

static void
put_u8v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u8v_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;
    unsigned int                nByte;

    nByte = 2u + Value.nValue * 1u;

    if(checkAvailable(pEncoderStream, nByte, pFieldDescriptor))
    {
        unsigned int            i;

        next_u16(pEncoder, Value.nValue);
        for(i = 0; i < Value.nValue; i++)
        {
            next_u8(pEncoder, Value.pValue[i]);
        }
    }
}

static void
put_s8v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s8v_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;
    unsigned int                nByte;

    nByte = 2u + Value.nValue * 1u;

    if(checkAvailable(pEncoderStream, nByte, pFieldDescriptor))
    {
        unsigned int            i;

        next_u16(pEncoder, Value.nValue);
        for(i = 0; i < Value.nValue; i++)
        {
            next_u8(pEncoder, Value.pValue[i]);
        }
    }
}

static void
put_u16 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u16_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;

    if(checkAvailable(pEncoderStream, 2u, pFieldDescriptor))
    {
        next_u16(pEncoder, Value);
    }
}

static void
put_s16 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s16_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;

    if(checkAvailable(pEncoderStream, 2u, pFieldDescriptor))
    {
        next_u16(pEncoder, Value);
    }
}

static void
put_u16v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u16v_t             Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;
    unsigned int                nByte;

    nByte = 2u + Value.nValue * 2u;

    if(checkAvailable(pEncoderStream, nByte, pFieldDescriptor))
    {
        unsigned int            i;

        next_u16(pEncoder, Value.nValue);
        for(i = 0; i < Value.nValue; i++)
        {
            next_u16(pEncoder, Value.pValue[i]);
        }
    }
}

static void
put_s16v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s16v_t             Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;
    unsigned int                nByte;

    nByte = 2u + Value.nValue * 2u;

    if(checkAvailable(pEncoderStream, nByte, pFieldDescriptor))
    {
        unsigned int            i;

        next_u16(pEncoder, Value.nValue);
        for(i = 0; i < Value.nValue; i++)
        {
            next_u16(pEncoder, Value.pValue[i]);
        }
    }
}

static void
put_u32 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u32_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;

    if(checkAvailable(pEncoderStream, 4u, pFieldDescriptor))
    {
        next_u32(pEncoder, Value);
    }
}

static void
put_s32 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s32_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;

    if(checkAvailable(pEncoderStream, 4u, pFieldDescriptor))
    {
        next_u32(pEncoder, Value);
    }
}

static void
put_u32v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u32v_t             Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;
    unsigned int                nByte;

    nByte = 2u + Value.nValue * 4u;

    if(checkAvailable(pEncoderStream, nByte, pFieldDescriptor))
    {
        unsigned int            i;

        next_u16(pEncoder, Value.nValue);
        for(i = 0; i < Value.nValue; i++)
        {
            next_u32(pEncoder, Value.pValue[i]);
        }
    }
}

static void
put_s32v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s32v_t             Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;
    unsigned int                nByte;

    nByte = 2u + Value.nValue * 4u;

    if(checkAvailable(pEncoderStream, nByte, pFieldDescriptor))
    {
        unsigned int            i;

        next_u16(pEncoder, Value.nValue);
        for(i = 0; i < Value.nValue; i++)
        {
            next_u32(pEncoder, Value.pValue[i]);
        }
    }
}

static void
put_u64 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u64_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;

    if(checkAvailable(pEncoderStream, 8u, pFieldDescriptor))
    {
        next_u64(pEncoder, Value);
    }
}

static void
put_s64 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s64_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;

    if(checkAvailable(pEncoderStream, 8u, pFieldDescriptor))
    {
        next_u64(pEncoder, Value);
    }
}

static void
put_u64v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u64v_t             Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;
    unsigned int                nByte;

    nByte = 2u + Value.nValue * 8u;

    if(checkAvailable(pEncoderStream, nByte, pFieldDescriptor))
    {
        unsigned int            i;

        next_u16(pEncoder, Value.nValue);
        for(i = 0; i < Value.nValue; i++)
        {
            next_u64(pEncoder, Value.pValue[i]);
        }
    }
}

static void
put_s64v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_s64v_t             Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;
    unsigned int                nByte;

    nByte = 2u + Value.nValue * 8u;

    if(checkAvailable(pEncoderStream, nByte, pFieldDescriptor))
    {
        unsigned int            i;

        next_u16(pEncoder, Value.nValue);
        for(i = 0; i < Value.nValue; i++)
        {
            next_u64(pEncoder, Value.pValue[i]);
        }
    }
}

static void
put_u1 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u1_t               Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;

    putBitField(pEncoderStream, 1u, Value, pFieldDescriptor);
}

static void
put_u1v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u1v_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;
    unsigned int                nByte = (Value.nBit + 7u) / 8u;

    if(checkAvailable(pEncoderStream, 2u + nByte, pFieldDescriptor))
    {
        unsigned int            i;

        next_u16(pEncoder, Value.nBit);
        for(i = 0; i < nByte; i++)
        {
            next_u8(pEncoder, Value.pValue[i]);
        }
    }
}

static void
put_u2 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u2_t               Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;

    putBitField(pEncoderStream, 2u, Value, pFieldDescriptor);

}

static void
put_u96 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u96_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;

    if(checkAvailable(pEncoderStream, 12u, pFieldDescriptor))
    {
        unsigned int            i;

        for(i = 0; i < 12; i++)
        {
            next_u8(pEncoder, Value.aValue[i]);
        }
    }
}

static void
put_utf8v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_utf8v_t            Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;
    unsigned int                nByte;

    nByte = 2u + Value.nValue * 1u;

    if(checkAvailable(pEncoderStream, nByte, pFieldDescriptor))
    {
        unsigned int            i;

        next_u16(pEncoder, Value.nValue);
        for(i = 0; i < Value.nValue; i++)
        {
            next_u8(pEncoder, Value.pValue[i]);
        }
    }
}

static void
put_bytesToEnd (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_bytesToEnd_t       Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;
    unsigned int                nByte;

    nByte = Value.nValue * 1u;

    if(checkAvailable(pEncoderStream, nByte, pFieldDescriptor))
    {
        unsigned int            i;

        for(i = 0; i < Value.nValue; i++)
        {
            next_u8(pEncoder, Value.pValue[i]);
        }
    }
}


static void
put_e1 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const int                     Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    put_u1(pBaseEncoderStream, (llrp_u1_t)Value, pFieldDescriptor);
}

static void
put_e2 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const int                     Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    put_u2(pBaseEncoderStream, (llrp_u2_t)Value, pFieldDescriptor);
}

static void
put_e8 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const int                     Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    put_u8(pBaseEncoderStream, (llrp_u8_t)Value, pFieldDescriptor);
}

static void
put_e16 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const int                     Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    put_u16(pBaseEncoderStream, (llrp_u16_t)Value, pFieldDescriptor);
}

static void
put_e32 (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const int                     Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    put_u32(pBaseEncoderStream, (llrp_u32_t)Value, pFieldDescriptor);
}

static void
put_e8v (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  const llrp_u8v_t              Value,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    put_u8v(pBaseEncoderStream, Value, pFieldDescriptor);
}

static void
put_reserved (
  LLRP_tSEncoderStream *        pBaseEncoderStream,
  unsigned int                  nBits)
{
    LLRP_tSFrameEncoderStream * pEncoderStream =
                            (LLRP_tSFrameEncoderStream *) pBaseEncoderStream;
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;
    LLRP_tSErrorDetails *       pError = &pEncoder->encoderHdr.ErrorDetails;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return;
    }

    while(nBits > 0)
    {
        unsigned int            Step = 7u & nBits;

        if(0 != pEncoder->nBitFieldResid)
        {
            if(Step != pEncoder->nBitFieldResid)
            {
                pError->eResultCode = LLRP_RC_UnalignedReservedBits;
                pError->pWhatStr    = "unaligned reserved bits";
                pError->pRefType    = pEncoderStream->pRefType;
                pError->pRefField   = NULL;
                pError->OtherDetail = pEncoder->iNext;
                return;
            }

            next_u8(pEncoder, pEncoder->BitFieldBuffer);
            nBits -= Step;
            pEncoder->BitFieldBuffer = 0;
            pEncoder->nBitFieldResid = 0;
        }
        else
        {
            if(0 != Step)
            {
                pError->eResultCode = LLRP_RC_UnalignedReservedBits;
                pError->pWhatStr    = "unaligned reserved bits";
                pError->pRefType    = pEncoderStream->pRefType;
                pError->pRefField   = NULL;
                pError->OtherDetail = pEncoder->iNext;
                return;
            }

            if(pEncoder->iNext >= pEncoder->nBuffer)
            {
                pError->eResultCode = LLRP_RC_ReservedBitsOverrun;
                pError->pWhatStr    = "overrun at reserved bits";
                pError->pRefType    = pEncoderStream->pRefType;
                pError->pRefField   = NULL;
                pError->OtherDetail = pEncoder->iNext;
                return;
            }

            next_u8(pEncoder, 0);
            nBits -= 8;
        }
    }
}

static void
streamConstruct_outermost (
  LLRP_tSFrameEncoderStream *   pEncoderStream,
  LLRP_tSFrameEncoder *         pEncoder)
{
    memset(pEncoderStream, 0, sizeof *pEncoderStream);
    pEncoderStream->encoderStreamHdr.pEncoderStreamOps =
                                &s_FrameEncoderStreamOps;

    pEncoderStream->pEncoder                = pEncoder;
    pEncoderStream->pEnclosingEncoderStream = NULL;
    pEncoderStream->pRefType                = NULL;
    pEncoderStream->iBegin                  = pEncoder->iNext;
}


static void
streamConstruct_nested (
  LLRP_tSFrameEncoderStream *   pEncoderStream,
  LLRP_tSFrameEncoderStream *   pEnclosingEncoderStream)
{
    LLRP_tSFrameEncoder *       pEncoder;

    pEncoder = pEnclosingEncoderStream->pEncoder;

    memset(pEncoderStream, 0, sizeof *pEncoderStream);
    pEncoderStream->encoderStreamHdr.pEncoderStreamOps =
                                &s_FrameEncoderStreamOps;

    pEncoderStream->pEncoder                = pEncoder;
    pEncoderStream->pEnclosingEncoderStream = pEnclosingEncoderStream;
    pEncoderStream->pRefType                = NULL;
    pEncoderStream->iBegin                  = pEncoder->iNext;
}

static void
putElement (
  LLRP_tSFrameEncoderStream *   pEncoderStream,
  const LLRP_tSElement *        pElement)
{
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;
    LLRP_tSErrorDetails *       pError = &pEncoder->encoderHdr.ErrorDetails;
    LLRP_tSEncoderStream *      pBaseEncoderStream =
                                        &pEncoderStream->encoderStreamHdr;
    const LLRP_tSTypeDescriptor *pRefType = pElement->pType;
    enum { MSG, TLV, TV, CUST_MSG, CUST_TLV } eFormat;
    unsigned int                nLength;
    unsigned char *             pLen;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return;
    }

    pEncoderStream->pRefType = pRefType;

    if(pRefType->bIsMessage)
    {
        eFormat = (NULL == pRefType->pVendorDescriptor) ? MSG : CUST_MSG;
    }
    else if(NULL == pRefType->pVendorDescriptor && 128 > pRefType->TypeNum)
    {
        /* TV parameter, never custom, no length */
        eFormat = TV;
    }
    else
    {
        /* TLV parameter */
        eFormat = (NULL == pRefType->pVendorDescriptor) ? TLV : CUST_TLV;
    }

    /*
     * Format the element header. The length part, if one,
     * is a place holder and back-patched later.
     */
    switch(eFormat)
    {
    default:
        assert(0);
        break;

    case MSG:
        {
            llrp_u16_t          VersType;

            VersType = (1u << 10u) | pRefType->TypeNum;
            put_u16(pBaseEncoderStream, VersType,
                &LLRP_g_fdMessageHeader_Type);
            put_u32(pBaseEncoderStream, 0,
                &LLRP_g_fdMessageHeader_Length);
            put_u32(pBaseEncoderStream,
                ((const LLRP_tSMessage *)pElement)->MessageID,
                &LLRP_g_fdMessageHeader_MessageID);
        }
        break;

    case CUST_MSG:
        {
            llrp_u16_t          VersType;

            /* Custom message */
            VersType = (1u << 10u) | 1023u;
            put_u16(pBaseEncoderStream, VersType,
                &LLRP_g_fdMessageHeader_Type);
            /* length is a placeholder */
            put_u32(pBaseEncoderStream, 0,
                &LLRP_g_fdMessageHeader_Length);
            put_u32(pBaseEncoderStream,
                ((const LLRP_tSMessage *)pElement)->MessageID,
                &LLRP_g_fdMessageHeader_MessageID);
            put_u32(pBaseEncoderStream,
                pRefType->pVendorDescriptor->VendorID,
                &LLRP_g_fdMessageHeader_VendorPEN);
            put_u8(pBaseEncoderStream, pRefType->TypeNum,
                &LLRP_g_fdMessageHeader_Subtype);
        }
        break;

    case TV:
        put_u8(pBaseEncoderStream, pRefType->TypeNum | 0x80u,
            &LLRP_g_fdParameterHeader_TVType);
        break;

    case TLV:
        /* Standard parameter */
        put_u16(pBaseEncoderStream, pRefType->TypeNum,
            &LLRP_g_fdParameterHeader_TLVType);
        put_u16(pBaseEncoderStream, 0,
            &LLRP_g_fdParameterHeader_TLVLength);
        break;

    case CUST_TLV:
        /* Custom parameter */
        put_u16(pBaseEncoderStream, 1023u,
            &LLRP_g_fdParameterHeader_TLVType);
        put_u16(pBaseEncoderStream, 0,
            &LLRP_g_fdParameterHeader_TLVLength);
        put_u32(pBaseEncoderStream,
                pRefType->pVendorDescriptor->VendorID,
            &LLRP_g_fdParameterHeader_VendorPEN);
        put_u32(pBaseEncoderStream, pRefType->TypeNum,
            &LLRP_g_fdParameterHeader_Subtype);
        break;
    }

    /*
     * If something went wrong preparing the element header,
     * just give up now.
     */
    if(LLRP_RC_OK != pError->eResultCode)
    {
        return;
    }

    pRefType->pfEncode(pElement, pBaseEncoderStream);

    nLength = pEncoder->iNext - pEncoderStream->iBegin;
    pLen = &pEncoder->pBuffer[pEncoderStream->iBegin];

    switch(eFormat)
    {
    default:
        assert(0);
        break;

    case MSG:
    case CUST_MSG:
        assert(nLength >= 10);
        pLen += 2;
        pLen[0] = nLength >> 24u;
        pLen[1] = nLength >> 16u;
        pLen[2] = nLength >> 8u;
        pLen[3] = nLength >> 0u;
        break;

    case TV:
        break;

    case TLV:
    case CUST_TLV:
        assert(nLength >= 4);
        pLen += 2;
        pLen[0] = nLength >> 8u;
        pLen[1] = nLength >> 0u;
        break;
    }
}

static void
nestSubParameter (
  LLRP_tSFrameEncoderStream *   pEncoderStream,
  const LLRP_tSParameter *      pParameter)
{
    LLRP_tSFrameEncoderStream   NestEncoderStream;

    if(NULL == pParameter)
    {
        return;
    }

    streamConstruct_nested(&NestEncoderStream, pEncoderStream);

    putElement(&NestEncoderStream, (LLRP_tSElement *)pParameter);
}

static llrp_bool_t
checkAvailable (
  LLRP_tSFrameEncoderStream *   pEncoderStream,
  unsigned int                  nByte,
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;
    LLRP_tSErrorDetails *       pError = &pEncoder->encoderHdr.ErrorDetails;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return FALSE;
    }

    if(pEncoder->iNext + nByte > pEncoder->nBuffer)
    {
        pError->eResultCode = LLRP_RC_FieldOverrun;
        pError->pWhatStr    = "overrun at field";
        pError->pRefType    = pEncoderStream->pRefType;
        pError->pRefField   = pFieldDescriptor;
        pError->OtherDetail = pEncoder->iNext;
        return FALSE;
    }

    if(0 != pEncoder->nBitFieldResid)
    {
        pError->eResultCode = LLRP_RC_UnalignedBitField;
        pError->pWhatStr    = "unalign/incomplete bit field";
        pError->pRefType    = pEncoderStream->pRefType;
        pError->pRefField   = pFieldDescriptor;
        pError->OtherDetail = pEncoder->iNext;
        return FALSE;
    }

    return TRUE;
}

static llrp_bool_t
putBitField (
  LLRP_tSFrameEncoderStream *   pEncoderStream,
  unsigned int                  nBit,
  unsigned int                  Value,  
  const LLRP_tSFieldDescriptor *pFieldDescriptor)
{
    LLRP_tSFrameEncoder *       pEncoder = pEncoderStream->pEncoder;
    LLRP_tSErrorDetails *       pError = &pEncoder->encoderHdr.ErrorDetails;

    if(0 == pEncoder->nBitFieldResid)
    {
        if(!checkAvailable(pEncoderStream, 1u, pFieldDescriptor))
        {
            return FALSE;
        }
        pEncoder->BitFieldBuffer = 0;
        pEncoder->nBitFieldResid = 8u;
    }

    if(pEncoder->nBitFieldResid < nBit)
    {
        pError->eResultCode = LLRP_RC_UnalignedBitField;
        pError->pWhatStr    = "unalign/incomplete bit field";
        pError->pRefType    = pEncoderStream->pRefType;
        pError->pRefField   = pFieldDescriptor;
        pError->OtherDetail = pEncoder->iNext;
        return FALSE;
    }

    pEncoder->nBitFieldResid -= nBit;

    Value &= (1u << nBit) - 1u;

    pEncoder->BitFieldBuffer |= Value << pEncoder->nBitFieldResid;

    if(0 == pEncoder->nBitFieldResid)
    {
        next_u8(pEncoder, pEncoder->BitFieldBuffer);
        pEncoder->BitFieldBuffer = 0;
        pEncoder->nBitFieldResid = 0;

    }

    return TRUE;
}
