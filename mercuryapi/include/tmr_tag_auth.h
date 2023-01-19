/* ex: set tabstop=2 shiftwidth=2 expandtab cindent: */
#ifndef _TMR_TAG_AUTH_H
#define _TMR_TAG_AUTH_H
/**
 *  @file tmr_tag_auth.h
 *  @brief Mercury API - Tag Authentication Interface
 *  @author Brian Fiegel
 *  @date 4/18/2009
 */

/*
 * Copyright (c) 2009 ThingMagic, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifdef  __cplusplus
extern "C" {
#endif

#include "tmr_gen2.h"

#ifdef TMR_ENABLE_UHF
/* 
 * RFID Authentication structures
 */
typedef enum TMR_AuthType
{
  TMR_AUTH_TYPE_GEN2_PASSWORD = 0,
  TMR_AUTH_TYPE_GEN2_DENATRAN_IAV_WRITE_CREDENTIALS = 1,
  TMR_AUTH_TYPE_GEN2_DENATRAN_IAV_WRITE_SEC_CREDENTIALS = 2,
} TMR_AuthType;

/**
 * Authentication data for RFID tag protocols.
 */
typedef struct TMR_TagAuthentication
{
  /**
   * The type of authentication data.
   * Indicates which member of the union is valid.
   */
  TMR_AuthType type;
  union
  {
    /** Gen2 authentication data */
    TMR_GEN2_Password gen2Password;
    /** Gen2 Denatran IAV write credential */
    TMR_GEN2_DENATRAN_IAV_WriteCredentials writeCreds;
    TMR_GEN2_DENATRAN_IAV_WriteSecCredentials writeSecCreds;
  } u;
} TMR_TagAuthentication;

TMR_Status TMR_TA_init_gen2(TMR_TagAuthentication *auth,
                            TMR_GEN2_Password password);

TMR_Status TMR_TA_init_gen2_Denatran_IAV_writeCredentials(TMR_TagAuthentication *auth, uint8_t idLength, uint8_t* tafId, uint8_t len,
                                                          uint8_t  *data);
TMR_Status TMR_TA_init_gen2_Denatran_IAV_writeSecCredentials(TMR_TagAuthentication *auth, uint8_t dataLength, uint8_t* data, uint8_t len,
                                                          uint8_t  *credentials);
#endif /* TMR_ENABLE_UHF */
#ifdef  __cplusplus
}
#endif

#endif /* _TMR_TAG_AUTH_H_ */
