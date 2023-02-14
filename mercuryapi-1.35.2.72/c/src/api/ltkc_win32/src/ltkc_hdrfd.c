
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
    LLRP_FT_U16,	// .eFieldType
    LLRP_FMT_NORMAL,	// .eFieldFormat
    "MessageHeader.Type",	// .pName
    NULL	// .pEnumTable
};

LLRP_tSFieldDescriptor
LLRP_g_fdMessageHeader_Length =
{
    LLRP_FT_U32,	// .eFieldType
    LLRP_FMT_NORMAL,	// .eFieldFormat
    "MessageHeader.Length",	// .pName
    NULL	// .pEnumTable
};

LLRP_tSFieldDescriptor
LLRP_g_fdMessageHeader_MessageID =
{
    LLRP_FT_U32,	// .eFieldType
    LLRP_FMT_NORMAL,	// .eFieldFormat
    "MessageHeader.MessageID",	// .pName
    NULL	// .pEnumTable
};

LLRP_tSFieldDescriptor
LLRP_g_fdMessageHeader_VendorPEN =
{
    LLRP_FT_U32,	// .eFieldType
    LLRP_FMT_NORMAL,	// .eFieldFormat
    "MessageHeader.CustomVendorPEN",	// .pName
    NULL	// .pEnumTable
};

LLRP_tSFieldDescriptor
LLRP_g_fdMessageHeader_Subtype =
{
    LLRP_FT_U8,	// .eFieldType
    LLRP_FMT_NORMAL,	// .eFieldFormat
    "MessageHeader.CustomSubtype",	// .pName
    NULL	// .pEnumTable
};

LLRP_tSFieldDescriptor
LLRP_g_fdParameterHeader_TVType =
{
    LLRP_FT_U8,	// .eFieldType
    LLRP_FMT_NORMAL,	// .eFieldFormat
    "ParameterHeader.TVType",	// .pName
    NULL	// .pEnumTable
};

LLRP_tSFieldDescriptor
LLRP_g_fdParameterHeader_TLVType =
{
    LLRP_FT_U16,	// .eFieldType
    LLRP_FMT_NORMAL,	// .eFieldFormat
    "ParameterHeader.TLVType",	// .pName
    NULL	// .pEnumTable
};

LLRP_tSFieldDescriptor
LLRP_g_fdParameterHeader_TLVLength =
{
    LLRP_FT_U16,	// .eFieldType
    LLRP_FMT_NORMAL,	// .eFieldFormat
    "ParameterHeader.TLVLength",	// .pName
    NULL	// .pEnumTable
};

LLRP_tSFieldDescriptor
LLRP_g_fdParameterHeader_VendorPEN =
{
    LLRP_FT_U32,	// .eFieldType
    LLRP_FMT_NORMAL,	// .eFieldFormat
    "ParameterHeader.CustomVendorPEN",	// .pName
    NULL	// .pEnumTable
};

LLRP_tSFieldDescriptor
LLRP_g_fdParameterHeader_Subtype =
{
    LLRP_FT_U32,	// .eFieldType
    LLRP_FMT_NORMAL,	// .eFieldFormat
    "ParameterHeader.CustomSubtype",	// .pName
    NULL	// .pEnumTable
};
