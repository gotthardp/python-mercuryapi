
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


/*
 * Include file to establish context
 * for the LLRP Tool Kit (LTK) C++ platform.
 */
#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) || defined(WINCE)
#include <stdint_win32.h>
#else
#include <stdint.h>
#endif

#include <stdlib.h>         /* malloc() */
#include <string.h>         /* memcpy() */

#define FALSE       0
#define TRUE        1

/*
 * Typedefs of simple types.
 * The LTK/C++ uses these types extensively.
 * To retarget to another C++ platform change
 * these typedefs. Everything else should be
 * good to go.
 */

#ifdef _STDINT_H

typedef uint8_t                 llrp_u8_t;
typedef int8_t                  llrp_s8_t;
typedef uint16_t                llrp_u16_t;
typedef int16_t                 llrp_s16_t;
typedef uint32_t                llrp_u32_t;
typedef int32_t                 llrp_s32_t;
typedef uint64_t                llrp_u64_t;
typedef int64_t                 llrp_s64_t;
typedef uint8_t                 llrp_u1_t;
typedef uint8_t                 llrp_u2_t;
typedef uint8_t                 llrp_utf8_t;
typedef int                     llrp_bool_t;
typedef uint8_t                 llrp_byte_t;

#endif /* _STDINT_H */

#ifdef __cplusplus
}
#endif