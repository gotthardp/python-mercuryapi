
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




LLRP_tSFieldDescriptor
LLRP_g_fdMessageHeader_Type =
{
    .eFieldType     = LLRP_FT_U16,
    .eFieldFormat   = LLRP_FMT_NORMAL,
    .pName          = "MessageHeader.Type",
    .pEnumTable     = NULL
};

LLRP_tSFieldDescriptor
LLRP_g_fdMessageHeader_Length =
{
    .eFieldType     = LLRP_FT_U32,
    .eFieldFormat   = LLRP_FMT_NORMAL,
    .pName          = "MessageHeader.Length",
    .pEnumTable     = NULL
};

LLRP_tSFieldDescriptor
LLRP_g_fdMessageHeader_MessageID =
{
    .eFieldType     = LLRP_FT_U32,
    .eFieldFormat   = LLRP_FMT_NORMAL,
    .pName          = "MessageHeader.MessageID",
    .pEnumTable     = NULL
};

LLRP_tSFieldDescriptor
LLRP_g_fdMessageHeader_VendorPEN =
{
    .eFieldType     = LLRP_FT_U32,
    .eFieldFormat   = LLRP_FMT_NORMAL,
    .pName          = "MessageHeader.CustomVendorPEN",
    .pEnumTable     = NULL
};

LLRP_tSFieldDescriptor
LLRP_g_fdMessageHeader_Subtype =
{
    .eFieldType     = LLRP_FT_U8,
    .eFieldFormat   = LLRP_FMT_NORMAL,
    .pName          = "MessageHeader.CustomSubtype",
    .pEnumTable     = NULL
};

LLRP_tSFieldDescriptor
LLRP_g_fdParameterHeader_TVType =
{
    .eFieldType     = LLRP_FT_U8,
    .eFieldFormat   = LLRP_FMT_NORMAL,
    .pName          = "ParameterHeader.TVType",
    .pEnumTable     = NULL
};

LLRP_tSFieldDescriptor
LLRP_g_fdParameterHeader_TLVType =
{
    .eFieldType     = LLRP_FT_U16,
    .eFieldFormat   = LLRP_FMT_NORMAL,
    .pName          = "ParameterHeader.TLVType",
    .pEnumTable     = NULL
};

LLRP_tSFieldDescriptor
LLRP_g_fdParameterHeader_TLVLength =
{
    .eFieldType     = LLRP_FT_U16,
    .eFieldFormat   = LLRP_FMT_NORMAL,
    .pName          = "ParameterHeader.TLVLength",
    .pEnumTable     = NULL
};

LLRP_tSFieldDescriptor
LLRP_g_fdParameterHeader_VendorPEN =
{
    .eFieldType     = LLRP_FT_U32,
    .eFieldFormat   = LLRP_FMT_NORMAL,
    .pName          = "ParameterHeader.CustomVendorPEN",
    .pEnumTable     = NULL
};

LLRP_tSFieldDescriptor
LLRP_g_fdParameterHeader_Subtype =
{
    .eFieldType     = LLRP_FT_U32,
    .eFieldFormat   = LLRP_FMT_NORMAL,
    .pName          = "ParameterHeader.CustomSubtype",
    .pEnumTable     = NULL
};
