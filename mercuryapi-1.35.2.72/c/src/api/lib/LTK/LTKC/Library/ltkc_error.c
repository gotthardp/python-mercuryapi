
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



void
LLRP_Error_clear (
  LLRP_tSErrorDetails *         pError)
{
    memset(pError, 0, sizeof *pError);
}

void
LLRP_Error_missingParameter (
  LLRP_tSErrorDetails *         pError,
  const LLRP_tSTypeDescriptor * pRefType)
{
    if(LLRP_RC_OK == pError->eResultCode)
    {
        pError->eResultCode = LLRP_RC_MissingParameter;
        pError->pRefType = pRefType;
        pError->pWhatStr = "missing parameter";
    }
}

void
LLRP_Error_unexpectedParameter (
  LLRP_tSErrorDetails *         pError,
  const LLRP_tSParameter *      pParameter)
{
    if(LLRP_RC_OK == pError->eResultCode)
    {
        pError->eResultCode = LLRP_RC_UnexpectedParameter;
        pError->pRefType = pParameter->elementHdr.pType;
        pError->pWhatStr = "unexpected parameter";
    }
}

void
LLRP_Error_resultCodeAndWhatStr (
  LLRP_tSErrorDetails *         pError,
  LLRP_tResultCode              eResultCode,
  const char *                  pWhatStr)
{
    if(LLRP_RC_OK == pError->eResultCode)
    {
        pError->eResultCode = eResultCode;
        pError->pWhatStr = pWhatStr;
    }
}

