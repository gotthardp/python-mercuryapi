
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




/* Create a new TypeRegistry */
LLRP_tSTypeRegistry *
LLRP_TypeRegistry_construct (void)
{
    LLRP_tSTypeRegistry *       pTypeRegistry;

    pTypeRegistry = (LLRP_tSTypeRegistry *) malloc(sizeof *pTypeRegistry);
    if(NULL == pTypeRegistry)
    {
        return pTypeRegistry;
    }

    memset(pTypeRegistry, 0, sizeof *pTypeRegistry);

    return pTypeRegistry;
}

/* Destruct a TypeRegistry */
void
LLRP_TypeRegistry_destruct (
  LLRP_tSTypeRegistry *         pTypeRegistry)
{
    memset(pTypeRegistry, 0, sizeof *pTypeRegistry);
    free(pTypeRegistry);
}

/* Add a type descriptor to the registry */
LLRP_tResultCode
LLRP_TypeRegistry_enroll (
  LLRP_tSTypeRegistry *         pTypeRegistry,
  const LLRP_tSTypeDescriptor * pTypeDescriptor)
{
    if(NULL == pTypeDescriptor->pVendorDescriptor)
    {
        if(1023u < pTypeDescriptor->TypeNum)
        {
            return LLRP_RC_EnrollBadTypeNumber;
        }

        /*
         * Standard message or parameter
         */
        if(pTypeDescriptor->bIsMessage)
        {
            pTypeRegistry->apStdMessageTypeDescriptors[
                    pTypeDescriptor->TypeNum] = pTypeDescriptor;
        }
        else
        {
            pTypeRegistry->apStdParameterTypeDescriptors[
                    pTypeDescriptor->TypeNum] = pTypeDescriptor;
        }
    }
    else
    {
        /*
         * Custom message or parameter
         */
        if(pTypeDescriptor->bIsMessage)
        {
            unsigned int        ix;

            ix = pTypeRegistry->nCustMessageTypeDescriptors++;
            pTypeRegistry->apCustMessageTypeDescriptors[ix] =
                        pTypeDescriptor;
        }
        else
        {
            unsigned int        ix;

            ix = pTypeRegistry->nCustParameterTypeDescriptors++;
            pTypeRegistry->apCustParameterTypeDescriptors[ix] =
                        pTypeDescriptor;
        }
    }
    
    return LLRP_RC_OK;
}

/* Lookup a standard message type descriptor. NULL=>not found */
const LLRP_tSTypeDescriptor *
LLRP_TypeRegistry_lookupMessage (
  const LLRP_tSTypeRegistry *   pTypeRegistry,
  unsigned int                  MessageTypeNum)
{
    if(1023u < MessageTypeNum)
    {
        return NULL;
    }

    return pTypeRegistry->apStdMessageTypeDescriptors[MessageTypeNum];
}

/* Lookup a standard parameter type descriptor. NULL=>not found */
const LLRP_tSTypeDescriptor *
LLRP_TypeRegistry_lookupParameter (
  const LLRP_tSTypeRegistry *   pTypeRegistry,
  unsigned int                  ParameterTypeNum)
{
    if(1023u < ParameterTypeNum)
    {
        return NULL;
    }

    return pTypeRegistry->apStdParameterTypeDescriptors[ParameterTypeNum];
}

/* Lookup a custom message type descriptor. NULL=>not found */
const LLRP_tSTypeDescriptor *
LLRP_TypeRegistry_lookupCustomMessage (
  const LLRP_tSTypeRegistry *   pTypeRegistry,
  unsigned int                  VendorID,
  unsigned int                  MessageSubTypeNum)
{
    const LLRP_tSTypeDescriptor *pTypeDescriptor;
    unsigned int                ix;

    for(ix = 0; ix < pTypeRegistry->nCustMessageTypeDescriptors; ix++)
    {
        pTypeDescriptor = pTypeRegistry->apCustMessageTypeDescriptors[ix];
        if(VendorID == pTypeDescriptor->pVendorDescriptor->VendorID &&
           MessageSubTypeNum == pTypeDescriptor->TypeNum)
        {
            return pTypeDescriptor;
        }
    }

    return NULL;
}

/* Lookup a custom parameter type descriptor. NULL=>not found */
const LLRP_tSTypeDescriptor *
LLRP_TypeRegistry_lookupCustomParameter (
  const LLRP_tSTypeRegistry *   pTypeRegistry,
  unsigned int                  VendorID,
  unsigned int                  ParameterSubTypeNum)
{
    const LLRP_tSTypeDescriptor *pTypeDescriptor;
    unsigned int                ix;

    for(ix = 0; ix < pTypeRegistry->nCustParameterTypeDescriptors; ix++)
    {
        pTypeDescriptor = pTypeRegistry->apCustParameterTypeDescriptors[ix];
        if(VendorID == pTypeDescriptor->pVendorDescriptor->VendorID &&
           ParameterSubTypeNum == pTypeDescriptor->TypeNum)
        {
            return pTypeDescriptor;
        }
    }

    return NULL;
}

/* Lookup a type descriptor by name. NULL=>not found */
const LLRP_tSTypeDescriptor *
LLRP_TypeRegistry_lookupByName (
  const LLRP_tSTypeRegistry *   pTypeRegistry,
  const char *                  pElementName)
{
    unsigned int                i;
    const LLRP_tSTypeDescriptor *pTypeDescriptor;

    for(i = 0; i < 1024u; i++)
    {
        pTypeDescriptor = pTypeRegistry->apStdMessageTypeDescriptors[i];
        if(NULL == pTypeDescriptor)
        {
            continue;
        }

        if(0 == strcmp(pTypeDescriptor->pName, pElementName))
        {
            return pTypeDescriptor;
        }
    }

    for(i = 0; i < 1024u; i++)
    {
        pTypeDescriptor = pTypeRegistry->apStdParameterTypeDescriptors[i];
        if(NULL == pTypeDescriptor)
        {
            continue;
        }

        if(0 == strcmp(pTypeDescriptor->pName, pElementName))
        {
            return pTypeDescriptor;
        }
    }

    /* TODO: custom messages */
    /* TODO: custom parameters */

    return NULL;
}

