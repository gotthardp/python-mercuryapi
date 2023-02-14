
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


#include "ltkc_platform.h"
#include "ltkc_base.h"
#include "ltkc_frame.h"


/*
 * <0  => Error
 *  0  => Done, length is *pMessageLength
 * >0  => Need more, return value is number of required bytes
 */

LLRP_tSFrameExtract
LLRP_FrameExtract (
  const unsigned char *         pBuffer,
  unsigned int                  nBuffer)
{
    LLRP_tSFrameExtract         frameExtract;

    memset(&frameExtract, 0, sizeof frameExtract);

    if(10 > nBuffer)
    {
        frameExtract.MessageLength = 10u;
        frameExtract.nBytesNeeded = frameExtract.MessageLength - nBuffer;
#ifndef __cplusplus
        frameExtract.eStatus = LLRP_FRAME_NEED_MORE;
#else
		frameExtract.eStatus = frameExtract.LLRP_FRAME_NEED_MORE;
#endif
    }
    else
    {
        llrp_u16_t                  VersType;

        VersType = pBuffer[0];
        VersType <<= 8u;
        VersType |= pBuffer[1];

        frameExtract.MessageLength = pBuffer[2];
        frameExtract.MessageLength <<= 8u;
        frameExtract.MessageLength |= pBuffer[3];
        frameExtract.MessageLength <<= 8u;
        frameExtract.MessageLength |= pBuffer[4];
        frameExtract.MessageLength <<= 8u;
        frameExtract.MessageLength |= pBuffer[5];

        /*
         * Should we be picky about reserved bits?
         */

        frameExtract.MessageType = VersType & 0x3FFu;
        frameExtract.ProtocolVersion = (VersType >> 10u) & 0x7u;

        frameExtract.MessageID = pBuffer[6];
        frameExtract.MessageID <<= 8u;
        frameExtract.MessageID |= pBuffer[7];
        frameExtract.MessageID <<= 8u;
        frameExtract.MessageID |= pBuffer[8];
        frameExtract.MessageID <<= 8u;
        frameExtract.MessageID |= pBuffer[9];

        if(10u > frameExtract.MessageLength)
        {
            frameExtract.nBytesNeeded = frameExtract.MessageLength - nBuffer;
#ifndef __cplusplus
			 frameExtract.eStatus = LLRP_FRAME_ERROR; 
#else
            frameExtract.eStatus = frameExtract.LLRP_FRAME_ERROR; 
#endif//changes
        }
        else if(nBuffer >= frameExtract.MessageLength)
        {
            frameExtract.nBytesNeeded = 0;
#ifndef __cplusplus
			 frameExtract.eStatus = LLRP_FRAME_READY;
#else
            frameExtract.eStatus = frameExtract.LLRP_FRAME_READY;                          //changes
#endif        
		}
        else
        {
            frameExtract.nBytesNeeded = frameExtract.MessageLength - nBuffer;
#ifndef __cplusplus
			frameExtract.eStatus = LLRP_FRAME_NEED_MORE; 
#else
            frameExtract.eStatus = frameExtract.LLRP_FRAME_NEED_MORE;                          //changes
#endif     
		}
    }

    return frameExtract;
}
