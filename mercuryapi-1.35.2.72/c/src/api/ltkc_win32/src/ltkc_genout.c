
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


#include "ltkc.h"
#include "ltkc_genoutmac.h"

#include "out_ltkc.inc"


static char     ident[] = {
    "$" "Id: libltkc "
    LTKC_VERSION_STR
    "  Built: "
    __DATE__
    " "
    __TIME__
    " $"
};

#ifdef __cplusplus
extern "C" {
#endif

LLRP_tSTypeRegistry *
LLRP_getTheTypeRegistry (void)
{
    LLRP_tSTypeRegistry *       pTypeRegistry;

    pTypeRegistry = LLRP_TypeRegistry_construct();
    if(NULL == pTypeRegistry)
    {
        return pTypeRegistry;
    }

    LLRP_enrollCoreTypesIntoRegistry (pTypeRegistry);

    return pTypeRegistry;
}

#ifdef __cplusplus
}
#endif
