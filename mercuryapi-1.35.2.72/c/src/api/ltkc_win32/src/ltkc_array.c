
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




llrp_u8v_t
LLRP_u8v_construct (
  llrp_u16_t                    nValue)
{
    unsigned int                nByte;
    llrp_u8v_t                  Value;

    nByte = nValue * sizeof Value.pValue[0];

    Value.pValue =(llrp_u8_t *)malloc(nByte);  //changes
    if(NULL != Value.pValue)
    {
        Value.nValue = nValue;
    }
    else
    {
        Value.nValue = 0;
    }

    return Value;
}

void
LLRP_u8v_clear (
  llrp_u8v_t *                  pDst)
{
    if(NULL != pDst->pValue)
    {
        free(pDst->pValue);
    }
    pDst->nValue = 0;
    pDst->pValue = NULL;
}

void
LLRP_u8v_set (
  llrp_u8v_t *                  pDst,
  llrp_u8v_t                    Value)
{
    LLRP_u8v_clear(pDst);
    *pDst = Value;
}

llrp_u8v_t
LLRP_u8v_copy (
  llrp_u8v_t                    Value)
{
    unsigned int                nByte;
    llrp_u8v_t                  Ret;

    nByte = Value.nValue * sizeof Value.pValue[0];

    Ret.pValue =(llrp_u8_t *)malloc(nByte);  //changes
    if(NULL != Ret.pValue)
    {
        Ret.nValue = Value.nValue;
        memcpy(Ret.pValue, Value.pValue, nByte);
    }
    else
    {
        Ret.nValue = 0;
    }

    return Ret;
}

llrp_s8v_t
LLRP_s8v_construct (
  llrp_u16_t                    nValue)
{
    unsigned int                nByte;
    llrp_s8v_t                  Value;

    nByte = nValue * sizeof Value.pValue[0];

    Value.pValue =(llrp_s8_t *)malloc(nByte);   //changes
    if(NULL != Value.pValue)
    {
        Value.nValue = nValue;
    }
    else
    {
        Value.nValue = 0;
    }

    return Value;
}

void
LLRP_s8v_clear (
  llrp_s8v_t *                  pDst)
{
    if(NULL != pDst->pValue)
    {
        free(pDst->pValue);
    }
    pDst->nValue = 0;
    pDst->pValue = NULL;
}

void
LLRP_s8v_set (
  llrp_s8v_t *                  pDst,
  llrp_s8v_t                    Value)
{
    LLRP_s8v_clear(pDst);
    *pDst = Value;
}

llrp_s8v_t
LLRP_s8v_copy (
  llrp_s8v_t                    Value)
{
    unsigned int                nByte;
    llrp_s8v_t                  Ret;

    nByte = Value.nValue * sizeof Value.pValue[0];

    Ret.pValue =(llrp_s8_t *)malloc(nByte);  //changes
    if(NULL != Ret.pValue)
    {
        Ret.nValue = Value.nValue;
        memcpy(Ret.pValue, Value.pValue, nByte);
    }
    else
    {
        Ret.nValue = 0;
    }

    return Ret;
}

llrp_u16v_t
LLRP_u16v_construct (
  llrp_u16_t                    nValue)
{
    unsigned int                nByte;
    llrp_u16v_t                 Value;

    nByte = nValue * sizeof Value.pValue[0];

    Value.pValue = (llrp_u16_t *)malloc(nByte);  //changes
    if(NULL != Value.pValue)
    {
        Value.nValue = nValue;
    }
    else
    {
        Value.nValue = 0;
    }

    return Value;
}

void
LLRP_u16v_clear (
  llrp_u16v_t *                 pDst)
{
    if(NULL != pDst->pValue)
    {
        free(pDst->pValue);
    }
    pDst->nValue = 0;
    pDst->pValue = NULL;
}

void
LLRP_u16v_set (
  llrp_u16v_t *                 pDst,
  llrp_u16v_t                   Value)
{
    LLRP_u16v_clear(pDst);
    *pDst = Value;
}

llrp_u16v_t
LLRP_u16v_copy (
  llrp_u16v_t                   Value)
{
    unsigned int                nByte;
    llrp_u16v_t                 Ret;

    nByte = Value.nValue * sizeof Value.pValue[0];

    Ret.pValue = (llrp_u16_t *)malloc(nByte);  //changes
    if(NULL != Ret.pValue)
    {
        Ret.nValue = Value.nValue;
        memcpy(Ret.pValue, Value.pValue, nByte);
    }
    else
    {
        Ret.nValue = 0;
    }

    return Ret;
}

llrp_s16v_t
LLRP_s16v_construct (
  llrp_u16_t                    nValue)
{
    unsigned int                nByte;
    llrp_s16v_t                 Value;

    nByte = nValue * sizeof Value.pValue[0];

    Value.pValue =(llrp_s16_t *) malloc(nByte);  //changes
    if(NULL != Value.pValue)
    {
        Value.nValue = nValue;
    }
    else
    {
        Value.nValue = 0;
    }

    return Value;
}

void
LLRP_s16v_clear (
  llrp_s16v_t *                 pDst)
{
    if(NULL != pDst->pValue)
    {
        free(pDst->pValue);
    }
    pDst->nValue = 0;
    pDst->pValue = NULL;
}

void
LLRP_s16v_set (
  llrp_s16v_t *                 pDst,
  llrp_s16v_t                   Value)
{
    LLRP_s16v_clear(pDst);
    *pDst = Value;
}

llrp_s16v_t
LLRP_s16v_copy (
  llrp_s16v_t                   Value)
{
    unsigned int                nByte;
    llrp_s16v_t                 Ret;

    nByte = Value.nValue * sizeof Value.pValue[0];

    Ret.pValue = (llrp_s16_t *)malloc(nByte);  //changes
    if(NULL != Ret.pValue)
    {
        Ret.nValue = Value.nValue;
        memcpy(Ret.pValue, Value.pValue, nByte);
    }
    else
    {
        Ret.nValue = 0;
    }

    return Ret;
}

llrp_u32v_t
LLRP_u32v_construct (
  llrp_u16_t                    nValue)
{
    unsigned int                nByte;
    llrp_u32v_t                 Value;

    nByte = nValue * sizeof Value.pValue[0];

    Value.pValue =(llrp_u32_t *) malloc(nByte);  //changes
    if(NULL != Value.pValue)
    {
        Value.nValue = nValue;
    }
    else
    {
        Value.nValue = 0;
    }

    return Value;
}

void
LLRP_u32v_clear (
  llrp_u32v_t *                 pDst)
{
    if(NULL != pDst->pValue)
    {
        free(pDst->pValue);
    }
    pDst->nValue = 0;
    pDst->pValue = NULL;
}

void
LLRP_u32v_set (
  llrp_u32v_t *                 pDst,
  llrp_u32v_t                   Value)
{
    LLRP_u32v_clear(pDst);
    *pDst = Value;
}

llrp_u32v_t
LLRP_u32v_copy (
  llrp_u32v_t                   Value)
{
    unsigned int                nByte;
    llrp_u32v_t                 Ret;

    nByte = Value.nValue * sizeof Value.pValue[0];

    Ret.pValue =(llrp_u32_t *) malloc(nByte);  //changes
    if(NULL != Ret.pValue)
    {
        Ret.nValue = Value.nValue;
        memcpy(Ret.pValue, Value.pValue, nByte);
    }
    else
    {
        Ret.nValue = 0;
    }

    return Ret;
}

llrp_s32v_t
LLRP_s32v_construct (
  llrp_u16_t                    nValue)
{
    unsigned int                nByte;
    llrp_s32v_t                 Value;

    nByte = nValue * sizeof Value.pValue[0];

    Value.pValue =(llrp_s32_t *) malloc(nByte);  //changes
    if(NULL != Value.pValue)
    {
        Value.nValue = nValue;
    }
    else
    {
        Value.nValue = 0;
    }

    return Value;
}

void
LLRP_s32v_clear (
  llrp_s32v_t *                 pDst)
{
    if(NULL != pDst->pValue)
    {
        free(pDst->pValue);
    }
    pDst->nValue = 0;
    pDst->pValue = NULL;
}

void
LLRP_s32v_set (
  llrp_s32v_t *                 pDst,
  llrp_s32v_t                   Value)
{
    LLRP_s32v_clear(pDst);
    *pDst = Value;
}

llrp_s32v_t
LLRP_s32v_copy (
  llrp_s32v_t                   Value)
{
    unsigned int                nByte;
    llrp_s32v_t                 Ret;

    nByte = Value.nValue * sizeof Value.pValue[0];

    Ret.pValue =(llrp_s32_t *) malloc(nByte);  //changes
    if(NULL != Ret.pValue)
    {
        Ret.nValue = Value.nValue;
        memcpy(Ret.pValue, Value.pValue, nByte);
    }
    else
    {
        Ret.nValue = 0;
    }

    return Ret;
}

llrp_u64v_t
LLRP_u64v_construct (
  llrp_u16_t                    nValue)
{
    unsigned int                nByte;
    llrp_u64v_t                 Value;

    nByte = nValue * sizeof Value.pValue[0];

    Value.pValue =(llrp_u64_t *) malloc(nByte);  //changes
    if(NULL != Value.pValue)
    {
        Value.nValue = nValue;
    }
    else
    {
        Value.nValue = 0;
    }

    return Value;
}

void
LLRP_u64v_clear (
  llrp_u64v_t *                 pDst)
{
    if(NULL != pDst->pValue)
    {
        free(pDst->pValue);
    }
    pDst->nValue = 0;
    pDst->pValue = NULL;
}

void
LLRP_u64v_set (
  llrp_u64v_t *                 pDst,
  llrp_u64v_t                   Value)
{
    LLRP_u64v_clear(pDst);
    *pDst = Value;
}

llrp_u64v_t
LLRP_u64v_copy (
  llrp_u64v_t                   Value)
{
    unsigned int                nByte;
    llrp_u64v_t                 Ret;

    nByte = Value.nValue * sizeof Value.pValue[0];

    Ret.pValue =(llrp_u64_t *) malloc(nByte);  //changes
    if(NULL != Ret.pValue)
    {
        Ret.nValue = Value.nValue;
        memcpy(Ret.pValue, Value.pValue, nByte);
    }
    else
    {
        Ret.nValue = 0;
    }

    return Ret;
}

llrp_s64v_t
LLRP_s64v_construct (
  llrp_u16_t                    nValue)
{
    unsigned int                nByte;
    llrp_s64v_t                 Value;

    nByte = nValue * sizeof Value.pValue[0];

    Value.pValue = (llrp_s64_t *)malloc(nByte);  //changes
    if(NULL != Value.pValue)
    {
        Value.nValue = nValue;
    }
    else
    {
        Value.nValue = 0;
    }

    return Value;
}

void
LLRP_s64v_clear (
  llrp_s64v_t *                 pDst)
{
    if(NULL != pDst->pValue)
    {
        free(pDst->pValue);
    }
    pDst->nValue = 0;
    pDst->pValue = NULL;
}

void
LLRP_s64v_set (
  llrp_s64v_t *                 pDst,
  llrp_s64v_t                   Value)
{
    LLRP_s64v_clear(pDst);
    *pDst = Value;
}

llrp_s64v_t
LLRP_s64v_copy (
  llrp_s64v_t                   Value)
{
    unsigned int                nByte;
    llrp_s64v_t                 Ret;

    nByte = Value.nValue * sizeof Value.pValue[0];

    Ret.pValue =(llrp_s64_t *) malloc(nByte);  //changes
    if(NULL != Ret.pValue)
    {
        Ret.nValue = Value.nValue;
        memcpy(Ret.pValue, Value.pValue, nByte);
    }
    else
    {
        Ret.nValue = 0;
    }

    return Ret;
}

llrp_u1v_t
LLRP_u1v_construct (
  llrp_u16_t                    nBit)
{
    unsigned int                nByte;
    llrp_u1v_t                  Value;

    nByte = (nBit + 7u) / 8u;

	Value.pValue =(llrp_u8_t *)malloc(nByte);  //changes
    if(NULL != Value.pValue)
    {
        Value.nBit = nBit;
    }
    else
    {
        Value.nBit = 0;
    }

    return Value;
}

void
LLRP_u1v_clear (
  llrp_u1v_t *                  pDst)
{
    if(NULL != pDst->pValue)
    {
        free(pDst->pValue);
    }
    pDst->nBit = 0;
    pDst->pValue = NULL;
}

void
LLRP_u1v_set (
  llrp_u1v_t *                  pDst,
  llrp_u1v_t                    Value)
{
    LLRP_u1v_clear(pDst);
    *pDst = Value;
}

llrp_u1v_t
LLRP_u1v_copy (
  llrp_u1v_t                    Value)
{
    unsigned int                nByte;
    llrp_u1v_t                  Ret;

    nByte = (Value.nBit + 7u) / 8u;

    Ret.pValue =(llrp_u8_t *) malloc(nByte);  //changes
    if(NULL != Ret.pValue)
    {
        Ret.nBit = Value.nBit;
        memcpy(Ret.pValue, Value.pValue, nByte);
    }
    else
    {
        Ret.nBit = 0;
    }

    return Ret;
}

llrp_utf8v_t
LLRP_utf8v_construct (
  llrp_u16_t                    nValue)
{
    unsigned int                nByte;
    llrp_utf8v_t                Value;

    nByte = nValue * sizeof Value.pValue[0];

    Value.pValue =(llrp_utf8_t *) malloc(nByte);
    if(NULL != Value.pValue)
    {
        Value.nValue = nValue;
    }
    else
    {
        Value.nValue = 0;
    }

    return Value;
}

void
LLRP_utf8v_clear (
  llrp_utf8v_t *                pDst)
{
    if(NULL != pDst->pValue)
    {
        free(pDst->pValue);
    }
    pDst->nValue = 0;
    pDst->pValue = NULL;
}

void
LLRP_utf8v_set (
  llrp_utf8v_t *                pDst,
  llrp_utf8v_t                  Value)
{
    LLRP_utf8v_clear(pDst);
    *pDst = Value;
}

llrp_utf8v_t
LLRP_utf8v_copy (
  llrp_utf8v_t                  Value)
{
    unsigned int                nByte;
    llrp_utf8v_t                Ret;

    nByte = Value.nValue * sizeof Value.pValue[0];

    Ret.pValue =(llrp_utf8_t *) malloc(nByte);  //changes
    if(NULL != Ret.pValue)
    {
        Ret.nValue = Value.nValue;
        memcpy(Ret.pValue, Value.pValue, nByte);
    }
    else
    {
        Ret.nValue = 0;
    }

    return Ret;
}

llrp_bytesToEnd_t
LLRP_bytesToEnd_construct (
  llrp_u16_t                    nValue)
{
    unsigned int                nByte;
    llrp_bytesToEnd_t           Value;

    nByte = nValue * sizeof Value.pValue[0];

    Value.pValue = (llrp_byte_t *)malloc(nByte);  //changes
    if(NULL != Value.pValue)
    {
        Value.nValue = nValue;
    }
    else
    {
        Value.nValue = 0;
    }

    return Value;
}

void
LLRP_bytesToEnd_clear (
  llrp_bytesToEnd_t *           pDst)
{
    if(NULL != pDst->pValue)
    {
        free(pDst->pValue);
    }
    pDst->nValue = 0;
    pDst->pValue = NULL;
}

void
LLRP_bytesToEnd_set (
  llrp_bytesToEnd_t *           pDst,
  llrp_bytesToEnd_t             Value)
{
    LLRP_bytesToEnd_clear(pDst);
    *pDst = Value;
}

llrp_bytesToEnd_t
LLRP_bytesToEnd_copy (
  llrp_bytesToEnd_t             Value)
{
    unsigned int                nByte;
    llrp_bytesToEnd_t           Ret;

    nByte = Value.nValue * sizeof Value.pValue[0];

    Ret.pValue =(llrp_byte_t *) malloc(nByte);  //changes
    if(NULL != Ret.pValue)
    {
        Ret.nValue = Value.nValue;
        memcpy(Ret.pValue, Value.pValue, nByte);
    }
    else
    {
        Ret.nValue = 0;
    }

    return Ret;
}
