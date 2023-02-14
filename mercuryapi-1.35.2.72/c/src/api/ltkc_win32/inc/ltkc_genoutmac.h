
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


#define SUBPARAM_SET(MEMBER,VALUE)			\
        LLRP_Element_setSubParameterPtr(		\
            (LLRP_tSElement *)pThis,			\
            (LLRP_tSParameter**)&pThis->MEMBER,		\
            (LLRP_tSParameter*)(VALUE))

#define SUBPARAM_ADD(MEMBER,VALUE)			\
        LLRP_Element_addToSubParameterList(		\
            (LLRP_tSElement *)pThis,			\
            (LLRP_tSParameter**)&pThis->MEMBER,		\
            (LLRP_tSParameter*)(VALUE))

#define SUBPARAM_ATTACH(MEMBER,VALUE)			\
        LLRP_Element_attachToSubParameterList(		\
            (LLRP_tSParameter**)&pThis->MEMBER,		\
            (LLRP_tSParameter*)(VALUE))

#define SUBPARAM_CLEAR(MEMBER)				\
        LLRP_Element_clearSubParameterList(		\
            (LLRP_tSElement *)pThis,			\
            (LLRP_tSParameter**)&pThis->MEMBER)
