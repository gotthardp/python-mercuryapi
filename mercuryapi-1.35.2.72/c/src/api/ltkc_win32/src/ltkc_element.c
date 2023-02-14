
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




LLRP_tSElement *
LLRP_Element_construct (
  const LLRP_tSTypeDescriptor *  pTypeDescriptor)
{
    LLRP_tSElement *            pElement;

    pElement =(LLRP_tSElement *)malloc(pTypeDescriptor->nSizeBytes);  //changes
    if(NULL != pElement)
    {
        memset(pElement, 0, pTypeDescriptor->nSizeBytes);

        pElement->pType = pTypeDescriptor;
    }

    return pElement;
}

void
LLRP_Element_destruct (
  LLRP_tSElement *              pElement)
{
    if(NULL != pElement)
    {
        pElement->pType->pfDestruct(pElement);
    }
}

void
LLRP_Element_finalDestruct (
  LLRP_tSElement *              pElement)
{
    LLRP_Element_clearSubParameterAllList(pElement);
    memset(pElement, 0xAA, pElement->pType->nSizeBytes);
    free(pElement);
}

void
LLRP_Element_addSubParameterToAllList (
  LLRP_tSElement *              pElement,
  LLRP_tSParameter *            pParameter)
{
    LLRP_tSParameter **         ppParameter;
    LLRP_tSParameter *          pNextParameter;

    for(
        ppParameter = &pElement->listAllSubParameters;
        NULL != *ppParameter;
        ppParameter = &pNextParameter->pNextAllSubParameters)
    {
        pNextParameter = *ppParameter;
    }

    pParameter->pNextAllSubParameters = NULL;
    *ppParameter = pParameter;
}

void
LLRP_Element_removeSubParameterFromAllList (
  LLRP_tSElement *              pElement,
  LLRP_tSParameter *            pParameter)
{
    LLRP_tSParameter **         ppParameter;
    LLRP_tSParameter *          pNextParameter;

    for(
        ppParameter = &pElement->listAllSubParameters;
        NULL != *ppParameter;
        )
    {
        pNextParameter = *ppParameter;
        if(pNextParameter == pParameter)
        {
            *ppParameter = pNextParameter->pNextAllSubParameters;
        }
        else
        {
            ppParameter = &pNextParameter->pNextAllSubParameters;
        }
    }

    pParameter->pNextAllSubParameters = NULL;
}

void
LLRP_Element_clearSubParameterAllList (
  LLRP_tSElement *              pElement)
{
    LLRP_tSParameter *          pParameter;

    while(NULL != (pParameter = pElement->listAllSubParameters))
    {
        pElement->listAllSubParameters =
                pParameter->pNextAllSubParameters;
        LLRP_Element_destruct(&pParameter->elementHdr);
    }
}

void
LLRP_Element_setSubParameterPtr (
  LLRP_tSElement *              pElement,
  LLRP_tSParameter **           ppPtr,
  LLRP_tSParameter *            pValue)
{
    if(NULL != *ppPtr)
    {
        LLRP_Element_removeSubParameterFromAllList(pElement, *ppPtr);
        LLRP_Element_destruct((LLRP_tSElement *) *ppPtr);
    }
    *ppPtr = pValue;
    if(NULL != *ppPtr)
    {
        LLRP_Element_addSubParameterToAllList(pElement, *ppPtr);
    }
}

void
LLRP_Element_addToSubParameterList (
  LLRP_tSElement *              pElement,
  LLRP_tSParameter **           ppListHead,
  LLRP_tSParameter *            pValue)
{
    LLRP_tSParameter **         ppCur = ppListHead;

    if(NULL != pValue)
    {
        while(NULL != *ppCur)
        {
            ppCur = &(*ppCur)->pNextSubParameter;
        }
        pValue->pNextSubParameter = NULL;
        *ppCur = pValue;

        LLRP_Element_addSubParameterToAllList(pElement, pValue);
    }
}

void
LLRP_Element_attachToSubParameterList (
  LLRP_tSParameter **           ppListHead,
  LLRP_tSParameter *            pValue)
{
    LLRP_tSParameter **         ppCur = ppListHead;

    if(NULL != pValue)
    {
        while(NULL != *ppCur)
        {
            ppCur = &(*ppCur)->pNextSubParameter;
        }
        pValue->pNextSubParameter = NULL;
        *ppCur = pValue;
    }
}

void
LLRP_Element_clearSubParameterList (
  LLRP_tSElement *              pElement,
  LLRP_tSParameter **           ppListHead)
{
    LLRP_tSParameter **         ppCur = ppListHead;
    LLRP_tSParameter *          pValue;

    while (NULL != (pValue = *ppCur))
    {
        *ppCur = pValue->pNextSubParameter;

        LLRP_Element_removeSubParameterFromAllList(pElement, pValue);
        LLRP_Element_destruct((LLRP_tSElement *) pValue);
    }
}

int
LLRP_Element_walk (
  const LLRP_tSElement *          pElement,
  int                           (*pFunc)(
                                  const LLRP_tSElement *    pElement,
                                  void *                    pArg),
  void *                        pArg,
  int                           iDepth,
  int                           nMaxDepth)
 {
    LLRP_tSParameter *          pParameter;
    int                         rc;

    rc = (*pFunc)(pElement, pArg);
    if(0 != rc)
    {
        return rc;
    }

    if(iDepth >= nMaxDepth)
    {
        return 0;
    }

    for(pParameter = pElement->listAllSubParameters;
        NULL != pParameter;
        pParameter = pParameter->pNextAllSubParameters)
    {
        rc = LLRP_Element_walk(&pParameter->elementHdr,
                pFunc, pArg, iDepth+1, nMaxDepth);
        if(0 != rc)
        {
            return rc;
        }
    }

    return 0;
}

void
LLRP_Message_setMessageID (
  LLRP_tSMessage *              pMessage,
  llrp_u32_t                    MessageID)
{
    pMessage->MessageID = MessageID;
}

llrp_bool_t
LLRP_Parameter_isAllowedIn (
  LLRP_tSParameter *            pParameter,
  const LLRP_tSTypeDescriptor * pEnclosingTypeDescriptor)
{
    const LLRP_tSTypeDescriptor *pType;

    pType = pParameter->elementHdr.pType;

    if(NULL == pType->pfIsAllowedIn)
    {
        return FALSE;
    }

    return (*pType->pfIsAllowedIn)(pEnclosingTypeDescriptor);
}

llrp_bool_t
LLRP_Parameter_isAllowedExtension (
  LLRP_tSParameter *            pParameter,
  const LLRP_tSTypeDescriptor * pEnclosingTypeDescriptor)
{
    const LLRP_tSTypeDescriptor *pType;

    pType = pParameter->elementHdr.pType;

    /*
     * If it is a generic Custom parameter allow it.
     */
    if(!pType->bIsMessage && NULL == pType->pVendorDescriptor &&
       1023u == pType->TypeNum)
    {
        return TRUE;
    }

    /*
     * If it is some kind of custom parameter allow it.
     */
    if(!pType->bIsMessage && NULL != pType->pVendorDescriptor)
    {
        return TRUE;
    }

    /*
     * At this point checking specifically if it is allowed
     * is perfunctory.
     */
    return LLRP_Parameter_isAllowedIn(pParameter, pEnclosingTypeDescriptor);
}
