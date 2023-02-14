
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
#ifdef __cplusplus
extern "C" {
#endif

struct LLRP_SFrameExtract;
struct LLRP_SFrameDecoder;
struct LLRP_SFrameDecoderStream;
struct LLRP_SFrameEncoder;
struct LLRP_SFrameEncoderStream;

typedef struct LLRP_SFrameExtract       LLRP_tSFrameExtract;
typedef struct LLRP_SFrameDecoder       LLRP_tSFrameDecoder;
typedef struct LLRP_SFrameDecoderStream LLRP_tSFrameDecoderStream;
typedef struct LLRP_SFrameEncoder       LLRP_tSFrameEncoder;
typedef struct LLRP_SFrameEncoderStream LLRP_tSFrameEncoderStream;


struct LLRP_SFrameExtract
{
    enum {
        LLRP_FRAME_UNKNOWN,
        LLRP_FRAME_READY,
        LLRP_FRAME_ERROR,
        LLRP_FRAME_NEED_MORE
    }                           eStatus;

    llrp_u32_t                  MessageLength;
    llrp_u16_t                  MessageType;
    llrp_u8_t                   ProtocolVersion;
    llrp_u32_t                  MessageID;

    unsigned int                nBytesNeeded;
};

struct LLRP_SFrameDecoder
{
    LLRP_tSDecoder              decoderHdr;

    unsigned char *             pBuffer;
    unsigned int                nBuffer;

    unsigned int                iNext;
    unsigned int                BitFieldBuffer;
    unsigned int                nBitFieldResid;
};

extern LLRP_tSFrameExtract
LLRP_FrameExtract (
  const unsigned char *         pBuffer,
  unsigned int                  nBuffer);

struct LLRP_SFrameDecoderStream
{
    LLRP_tSDecoderStream        decoderStreamHdr;

    LLRP_tSFrameDecoder *       pDecoder;
    LLRP_tSFrameDecoderStream * pEnclosingDecoderStream;
    const LLRP_tSTypeDescriptor *pRefType;
    unsigned int                iBegin;
    unsigned int                iLimit;
};

extern LLRP_tSFrameDecoder *
LLRP_FrameDecoder_construct (
  const LLRP_tSTypeRegistry *   pTypeRegistry,
  unsigned char *               pBuffer,
  unsigned int                  nBuffer);


struct LLRP_SFrameEncoder
{
    LLRP_tSEncoder              encoderHdr;

    unsigned char *             pBuffer;
    unsigned int                nBuffer;

    unsigned int                iNext;
    unsigned int                BitFieldBuffer;
    unsigned int                nBitFieldResid;
};

struct LLRP_SFrameEncoderStream
{
    LLRP_tSEncoderStream        encoderStreamHdr;

    LLRP_tSFrameEncoder *       pEncoder;
    LLRP_tSFrameEncoderStream * pEnclosingEncoderStream;
    const LLRP_tSTypeDescriptor *pRefType;
    unsigned int                iBegin;
};

extern LLRP_tSFrameEncoder *
LLRP_FrameEncoder_construct (
  unsigned char *               pBuffer,
  unsigned int                  nBuffer);

#ifdef __cplusplus
}
#endif