/* ex: set tabstop=2 shiftwidth=2 expandtab cindent: */
#ifndef _TMR_GEN2_H
#define _TMR_GEN2_H
/** 
 *  @file tmr_gen2.h  
 *  @brief Mercury API - Gen2 tag information and interfaces
 *  @author Brian Fiegel
 *  @date 5/7/2009
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
#ifdef TMR_ENABLE_UHF
/** Memory lock bits */
typedef enum TMR_GEN2_LockBits
{
  /** User memory bank lock permalock bit */
  TMR_GEN2_LOCK_BITS_USER_PERM    = (1 << 0),
  /** User memory bank lock bit */
  TMR_GEN2_LOCK_BITS_USER         = (1 << 1),
  /** TID memory bank lock permalock bit */
  TMR_GEN2_LOCK_BITS_TID_PERM     = (1 << 2),
  /** TID memory bank lock bit */
  TMR_GEN2_LOCK_BITS_TID          = (1 << 3),
  /** EPC memory bank lock permalock bit */
  TMR_GEN2_LOCK_BITS_EPC_PERM     = (1 << 4),
  /** EPC memory bank lock bit */
  TMR_GEN2_LOCK_BITS_EPC          = (1 << 5),
  /** Access password lock permalock bit */
  TMR_GEN2_LOCK_BITS_ACCESS_PERM  = (1 << 6),
  /** Access password lock bit */
  TMR_GEN2_LOCK_BITS_ACCESS       = (1 << 7),
  /** Kill password lock permalock bit */
  TMR_GEN2_LOCK_BITS_KILL_PERM    = (1 << 8),
  /** Kill password lock bit */
  TMR_GEN2_LOCK_BITS_KILL         = (1 << 9)
} TMR_GEN2_LockBits;

typedef enum TMR_GEN2_LockBits_Mask
{
  TMR_GEN2_LOCK_BITS_MASK_USER      = TMR_GEN2_LOCK_BITS_USER_PERM | TMR_GEN2_LOCK_BITS_USER,
  TMR_GEN2_LOCK_BITS_MASK_TID       = TMR_GEN2_LOCK_BITS_TID_PERM | TMR_GEN2_LOCK_BITS_TID,
  TMR_GEN2_LOCK_BITS_MASK_EPC       = TMR_GEN2_LOCK_BITS_EPC_PERM | TMR_GEN2_LOCK_BITS_EPC,
  TMR_GEN2_LOCK_BITS_MASK_ACCESS    = TMR_GEN2_LOCK_BITS_ACCESS_PERM | TMR_GEN2_LOCK_BITS_ACCESS,
  TMR_GEN2_LOCK_BITS_MASK_KILL      = TMR_GEN2_LOCK_BITS_KILL_PERM | TMR_GEN2_LOCK_BITS_KILL
} TMR_GEN2_LockBits_Mask;

typedef enum TMR_GEN2_LockBits_Action
{
  TMR_GEN2_LOCK_BITS_ACTION_USER_LOCK = TMR_GEN2_LOCK_BITS_USER,
  TMR_GEN2_LOCK_BITS_ACTION_USER_UNLOCK = 0,
  TMR_GEN2_LOCK_BITS_ACTION_USER_PERMALOCK = TMR_GEN2_LOCK_BITS_USER_PERM | TMR_GEN2_LOCK_BITS_USER,
  TMR_GEN2_LOCK_BITS_ACTION_USER_PERMAUNLOCK = TMR_GEN2_LOCK_BITS_USER_PERM,
  TMR_GEN2_LOCK_BITS_ACTION_TID_LOCK = TMR_GEN2_LOCK_BITS_TID,
  TMR_GEN2_LOCK_BITS_ACTION_TID_UNLOCK = 0,  
  TMR_GEN2_LOCK_BITS_ACTION_TID_PERMALOCK = TMR_GEN2_LOCK_BITS_TID_PERM | TMR_GEN2_LOCK_BITS_TID,
  TMR_GEN2_LOCK_BITS_ACTION_TID_PERMAUNLOCK = TMR_GEN2_LOCK_BITS_TID_PERM,
  TMR_GEN2_LOCK_BITS_ACTION_EPC_LOCK = TMR_GEN2_LOCK_BITS_EPC,
  TMR_GEN2_LOCK_BITS_ACTION_EPC_UNLOCK = 0,   
  TMR_GEN2_LOCK_BITS_ACTION_EPC_PERMALOCK = TMR_GEN2_LOCK_BITS_EPC_PERM | TMR_GEN2_LOCK_BITS_EPC,
  TMR_GEN2_LOCK_BITS_ACTION_EPC_PERMAUNLOCK = TMR_GEN2_LOCK_BITS_EPC_PERM,
  TMR_GEN2_LOCK_BITS_ACTION_ACCESS_LOCK = TMR_GEN2_LOCK_BITS_ACCESS,
  TMR_GEN2_LOCK_BITS_ACTION_ACCESS_UNLOCK = 0,
  TMR_GEN2_LOCK_BITS_ACTION_ACCESS_PERMALOCK = TMR_GEN2_LOCK_BITS_ACCESS_PERM | TMR_GEN2_LOCK_BITS_ACCESS,
  TMR_GEN2_LOCK_BITS_ACTION_ACCESS_PERMAUNLOCK = TMR_GEN2_LOCK_BITS_ACCESS_PERM,
  TMR_GEN2_LOCK_BITS_ACTION_KILL_LOCK = TMR_GEN2_LOCK_BITS_KILL,
  TMR_GEN2_LOCK_BITS_ACTION_KILL_UNLOCK = 0,
  TMR_GEN2_LOCK_BITS_ACTION_KILL_PERMALOCK = TMR_GEN2_LOCK_BITS_KILL_PERM | TMR_GEN2_LOCK_BITS_KILL,
  TMR_GEN2_LOCK_BITS_ACTION_KILL_PERMAUNLOCK = TMR_GEN2_LOCK_BITS_KILL_PERM
} TMR_GEN2_LockBits_Action;

/**
 * The arguments to a TMR_lockTag() method for Gen2 tags. 
 */
typedef struct TMR_GEN2_LockAction
{
  /** The gen2 lock mask bits */
  uint16_t mask;
  /** The gen2 lock action bits */
  uint16_t action;
}TMR_GEN2_LockAction;
#endif /* TMR_ENABLE_UHF */
/**
 * @ingroup tagauth
 * A 32-bit password (access or kill) in the Gen2 protocol.
 */
typedef uint32_t TMR_GEN2_Password;
#ifdef TMR_ENABLE_UHF
/**
 * @ingroup tagauth
 * a 16 byte(128 bit) Gen2 Denatran IAV write credentials
 */
typedef struct TMR_GEN2_DENATRAN_IAV_WriteCredentials
{
  uint8_t value[16];
  uint8_t tagId[8];
  uint8_t credentialLength;
  uint8_t tagIdLength;
}TMR_GEN2_DENATRAN_IAV_WriteCredentials;

/**
 * @ingroup tagauth
 * a 16 byte(128 bit) Gen2 Denatran IAV write sec credentials
 */
typedef struct TMR_GEN2_DENATRAN_IAV_WriteSecCredentials
{
  uint8_t value[16];
  uint8_t data[6];
  uint8_t credentialLength;
  uint8_t dataLength;
}TMR_GEN2_DENATRAN_IAV_WriteSecCredentials;

/** Gen2 memory banks */
typedef enum TMR_GEN2_Bank
{
  /** Reserved bank (kill and access passwords) */
  TMR_GEN2_BANK_RESERVED  = 0x0,
  /** EPC memory bank */
  TMR_GEN2_BANK_EPC       = 0x1,
  /** TID memory bank */
  TMR_GEN2_BANK_TID       = 0x2,
  /** User memory bank */
  TMR_GEN2_BANK_USER      = 0x3,
  /** Used to enable the read of additional membanks - reserved mem bank */
  TMR_GEN2_BANK_RESERVED_ENABLED = 0x4,
  /** Used to Filter Gen2 Tag with specified EPC length */
  TMR_GEN2_EPC_LENGTH_FILTER = 0x6,
  /** Gen2 Truncate Option */
  TMR_GEN2_EPC_TRUNCATE = 0x7,
  /** Used to enable the read of additional membanks - epc mem bank */
  TMR_GEN2_BANK_EPC_ENABLED = 0x8,
  /** Used to enable the read of additional membanks - tid mem bank */
  TMR_GEN2_BANK_TID_ENABLED = 0x10,
  /** Used to enable the read of additional membanks - user mem bank */
  TMR_GEN2_BANK_USER_ENABLED = 0x20
} TMR_GEN2_Bank;

/** TMR_GEN2_Select_target indicates flag to Act on */
typedef enum TMR_GEN2_Select_target
{
  /*Inventoried flag for session S0*/
  INVENTORIED_S0 = 0x00,
  /*Inventoried flag for session S1*/
  INVENTORIED_S1 = 0x01,
  /*Inventoried flag for session S2*/
  INVENTORIED_S2 = 0x02,
  /*Inventoried flag for session S3*/
  INVENTORIED_S3 = 0x03,
  /* Select flag*/
  SELECT         = 0x04,
  /* Reserved for future use */
  RFU1           = 0x05,
  /* Reserved for future use */
  RFU2           = 0x06,
  /* Reserved for future use */
  RFU3           = 0x07,
}TMR_GEN2_Select_target;

/*********************************************************************************
 TMR_GEN2_Select_action indicates which Select action to take
 (See Gen2 spec /Select commands / Tag response to Action parameter)
 |-----------------------------------------------------------------------------|
 |  Action  |        Tag Matching            |        Tag Not-Matching         |
 |----------|--------------------------------|---------------------------------|
 |   0x00   |  Assert SL or Inventoried->A   |  Deassert SL or Inventoried->B  |
 |   0x01   |  Assert SL or Inventoried->A   |  Do nothing                     |
 |   0x02   |  Do nothing                    |  Deassert SL or Inventoried->B  |
 |   0x03   |  Negate SL or (A->B,B->A)      |  Do nothing                     |
 |   0x04   |  Deassert SL or Inventoried->B |  Assert SL or Inventoried->A    |
 |   0x05   |  Deassert SL or Inventoried->B |  Do nothing                     |
 |   0x06   |  Do nothing                    |  Assert SL or Inventoried->A    |
 |   0x07   |  Do nothing                    |  Negate SL or (A->B,B->A)       |
**********************************************************************************
 *
 *  To improve readability and ease of typing, these names abbreviate the official terminology of the Gen2 spec.
 *  <A>_N_<B>: The "_N_" stands for "Non-Matching".
 *  The <A> clause before the N describes what happens to Matching tags.
 *  The <B> clause after the N describes what happens to Non-Matching tags.
 *  (Alternately, you can pronounce "_N_" as "and", or "&"; i.e.,
 *  the pair of Matching / Non-Matching actions is known as "<A> and <B>".)
 *
 *  ON: assert SL or inventoried -> A
 *  OFF: deassert SL or inventoried -> B
 *  NEG: negate SL or (A->B, B->A)
 *  NOP: do nothing
 *
 *  The enum is simply a transliteration of the Gen2 spec's table: "Tag response to Action parameter" 
 */
typedef enum TMR_GEN2_Select_action
{
  ON_N_OFF  = 0x00,
  ON_N_NOP  = 0x01,
  NOP_N_OFF = 0x02,
  NEG_N_NOP = 0x03,
  OFF_N_ON  = 0x04,
  OFF_N_NOP = 0x05,
  NOP_N_ON  = 0x06,
  NOP_N_NEG = 0x07,
}TMR_GEN2_Select_action;

/**
 * A single selection operation in the Gen2 protocol.
 * @ingroup filter
 */
typedef struct TMR_GEN2_Select
{
  /** Whether to invert the selection (deselect tags that meet the comparison) */
  bool invert;
  /** The memory bank in which to compare the mask */
  TMR_GEN2_Bank bank;
  /** The location (in bits) at which to begin comparing the mask */
  uint32_t bitPointer;
  /** The length (in bits) of the mask */
  uint16_t maskBitLength;
  /** The mask value to compare with the specified region of tag memory, MSB first */ 
  uint8_t *mask;
  /** Target byte*/
  TMR_GEN2_Select_target target;
  /** Action byte*/
  TMR_GEN2_Select_action action;
} TMR_GEN2_Select;

/** Gen2 session values */
typedef enum TMR_GEN2_Session
{
  TMR_GEN2_SESSION_MIN= 0x00,
  /** Session 0 */
  TMR_GEN2_SESSION_S0 = 0x00,
  /** Session 1 */
  TMR_GEN2_SESSION_S1 = 0x01,
  /** Session 2 */ 
  TMR_GEN2_SESSION_S2 = 0x02,
  /** Session 3 */ 
  TMR_GEN2_SESSION_S3 = 0x03,
  TMR_GEN2_SESSION_MAX     = TMR_GEN2_SESSION_S3,
  TMR_GEN2_SESSION_INVALID = TMR_GEN2_SESSION_MAX + 1
} TMR_GEN2_Session;

/** Gen2 divide ratio values */
typedef enum TMR_GEN2_DivideRatio
{
  /** DR of 8 */
  TMR_GEN2_DIVIDE_RATIO_8    = 0,
  /** DR of 64/3 */
  TMR_GEN2_DIVIDE_RATIO_64_3 = 1
} TMR_GEN2_DivideRatio;

/** Gen2 TrExt bit */
typedef enum TMR_GEN2_TrExt
{
  /** No pilot tone in tag response */
  TMR_GEN2_TR_EXT_NO_PILOT_TONE = 0,
  /** Pilot tone in tag response */
  TMR_GEN2_TR_EXT_PILOT_TONE    = 1
} TMR_GEN2_TrExt;

/** Gen2 target search algorithms */
typedef enum TMR_GEN2_Target
{
  TMR_GEN2_TARGET_MIN= 0,
  /** Search target A */
  TMR_GEN2_TARGET_A  = 0,
  /** Search target B */
  TMR_GEN2_TARGET_B  = 1,
  /** Search target A until exhausted, then search target B */
  TMR_GEN2_TARGET_AB = 2,
  /** Search target B until exhausted, then search target A */
  TMR_GEN2_TARGET_BA = 3,
  TMR_GEN2_TARGET_MAX     = TMR_GEN2_TARGET_BA,
  TMR_GEN2_TARGET_INVALID = TMR_GEN2_TARGET_MAX+1
}TMR_GEN2_Target;

/** Gen2 tag encoding modulation values */
typedef enum TMR_GEN2_TagEncoding
{
  /** FM0 **/
  TMR_GEN2_FM0 = 0,

  TMR_GEN2_MILLER_MIN = 1,
  /** M = 2 */
  TMR_GEN2_MILLER_M_2 = 1,
  /** M = 4 */
  TMR_GEN2_MILLER_M_4 = 2,
  /** M = 8 */
  TMR_GEN2_MILLER_M_8 = 3,
  TMR_GEN2_MILLER_MAX     = TMR_GEN2_MILLER_M_8,
  TMR_GEN2_MILLER_INVALID = TMR_GEN2_MILLER_MAX+1
}TMR_GEN2_TagEncoding;

/** Gen2 link frequencies */
typedef enum TMR_GEN2_LinkFrequency
{
  /** 250 kHz */
  TMR_GEN2_LINKFREQUENCY_250KHZ  = 250,
  /** 320 KHZ */
  TMR_GEN2_LINKFREQUENCY_320KHZ  = 320,
  /**640 kHz*/
  TMR_GEN2_LINKFREQUENCY_640KHZ   = 640,
  TMR_GEN2_LINKFREQUENCY_MAX     = 640,
  TMR_GEN2_LINKFREQUENCY_INVALID = TMR_GEN2_LINKFREQUENCY_MAX + 1,
} TMR_GEN2_LinkFrequency;

/** Gen2 Protocol Extension parameters */
typedef enum TMR_GEN2_ProtocolExtension
{
  TMR_GEN2_PROTOCOLEXTENSION_LICENSE_NONE			= 0,
  TMR_GEN2_PROTOCOLEXTENSION_LICENSE_IAV_DENATRAN	= 1
}TMR_GEN2_ProtocolExtension;

/** Gen2 bap parameters */
typedef struct TMR_GEN2_Bap
{
  /* Default to 3000 us */
  int32_t powerUpDelayUs;
  /* Default to 20000 us */
  int32_t freqHopOfftimeUs;
}TMR_GEN2_Bap;

/** Gen2v2 untraceably hides part of EPC memory or not */
typedef enum TMR_GEN2_UNTRACEABLE_Epc
{
	/* Tag exposes EPC memory */
	EPC_SHOW = 0,
	/*Tag untraceably hides EPC memory above that*/
	EPC_HIDE = 1
}TMR_GEN2_UNTRACEABLE_Epc;

/** Gen2v2 untraceably hides part of TID memory or not */
typedef enum TMR_GEN2_UNTRACEABLE_Tid
{
	/* Tag exposes TID memory. */
	HIDE_NONE = 0,
	/* Tag’s allocation class identifier is E0h then the 
	Tag untraceably hides TID memory above 10h,inclusive;
	if the Tag’s allocation class identifier is E2h then the
	Tag untraceably hides TID memory above 20h, inclusive. */
	HIDE_SOME = 1,
	/*Tag untraceably hides all of TID memory.*/
	HIDE_ALL = 2,
	/* RFU */
	TID_RFU = 3
}TMR_GEN2_UNTRACEABLE_Tid;

/** Gen2v2 untraceably hides part of UserMemory memory or not */
typedef enum TMR_GEN2_UNTRACEABLE_UserMemory
{
	/* Tag exposes User memory */
	SHOW = 0,
	/*Tag untraceably hides User memory*/
	HIDE = 1
}TMR_GEN2_UNTRACEABLE_UserMemory;

/** specifies Gen2v2 Tag’s operating range */
typedef enum TMR_GEN2_UNTRACEABLE_Range
{
	/* Tag persistently enables normal operating range */
	NORMAL = 0,
	/* Tag temporarily toggles its operating range*/
	TOGGLE_TEMPORARILY = 1,
	/*Tag persistently enables reduced operating range.*/
	REDUCED = 2,
	/* RFU */
	RFU = 3
}TMR_GEN2_UNTRACEABLE_Range;

/** Authentication KeyId */
typedef enum TMR_NXP_KeyId
{
	/* key0*/
	KEY0 = 0,
	/*Key1*/
	KEY1 = 1
}TMR_NXP_KeyId;

/** Memory profile for the addition of custom data */
typedef enum TMR_NXP_Profile
{
	/* EPC memory bank */
	EPC = 0,
	/* TID memory bank */
	TID = 1,
	/* User memory bank */
	USER = 2
}TMR_NXP_Profile;

/** Gen2 Tari values */
typedef enum TMR_GEN2_Tari
{
  /** Tari of 25 microseconds */
  TMR_GEN2_TARI_25US    = 0,
  /** Tari of 12.5 microseconds */
  TMR_GEN2_TARI_12_5US  = 1,
  /** Tari of 6.25 microseconds */
  TMR_GEN2_TARI_6_25US  = 2,
  TMR_GEN2_TARI_MAX     = 2,
  TMR_GEN2_TARI_INVALID = TMR_GEN2_TARI_MAX + 1,
} TMR_GEN2_Tari;

/** Gen2 WriteMode */
typedef enum TMR_GEN2_WriteMode
{
  /** WORD ONLY */
  TMR_GEN2_WORD_ONLY    = 0,
  /** BLOCK ONLY */
  TMR_GEN2_BLOCK_ONLY    = 1,
  /** BLOCK FALLBACK */
  TMR_GEN2_BLOCK_FALLBACK    = 2,

} TMR_GEN2_WriteMode;

/**
 * Defines the values for different gen2 silicon types.
 */
typedef enum TMR_SR_GEN2_SiliconType
{
  TMR_SR_GEN2_SILICON_ANY             = 0x00,
  TMR_SR_GEN2_ALIEN_HIGGS_SILICON     = 0x01,
  TMR_SR_GEN2_NXP_G2X_SILICON         = 0x02,
  TMR_SR_GEN2_ALIEN_HIGGS3_SILICON    = 0x05,
  TMR_SR_GEN2_NXP_G2I_SILICON         = 0x07,
  TMR_SR_GEN2_IMPINJ_MONZA4_SILICON   = 0x08,
  TMR_SR_GEN2_EMMICRO_EM4325_SILICON  = 0x09,
  TMR_SR_GEN2_IDS_SL900A_SILICON      = 0x0A,
  TMR_SR_GEN2_DENATRAN_IAV_SILICON    = 0x0B,
  TMR_SR_GEN2_NXP_AES_UCODE           = 0x0C,
  TMR_SR_GEN2_IMPINJ_MONZA6_SILICON   = 0x0D,
  TMR_SR_GEN2_NXP_UCODE7_SILICON      = 0x0E,
  TMR_SR_GEN2_FUDAN_SILICON           = 0x0F,
  TMR_SR_GEN2_ILIAN_LED_SILICON       = 0x10,
}TMR_SR_GEN2_SiliconType;

/**
 * The argument to a ChangeConfig command for NXP G2iL tags
 * User can set the configWord directly using the 16 bit 'data' field.
 * Otherwise, provides an option to individually set each flag.
 * Instance of TMR_NXP_ConfigWord must be initialized before using either
 * by declaring it as 'static' or by using the constructor routine
 * TMR_init_NXP_ConfigWord() method.
 */
typedef union TMR_NXP_ConfigWord
{
  uint16_t data;
  /*
   * Since the bit-fields are used, The bits in configWord are shown from
   * Least significant bit to Most significant bit.
   */
  struct
  {
    /** Least significant bit */
    /** PSF alarm flag (Permanently stored in tag memory)*/
    unsigned psfAlarm:1;
    /** Read protect TID bank (permanently stored in tag memory)*/
    unsigned readProtectTID:1;
    /** Read protect EPC bank (Permanently stored in tag memory)*/
    unsigned readProtectEPC:1;
     /** Read protect User memory bank (permanently stored in tag memory)*/
    unsigned readProtectUser:1;
    /** Read range reduction on/off (permanently stored in tag memory)*/
    unsigned privacyMode:1;
    /** Digital output (permanently stored in tag memory)*/
    unsigned digitalOutput:1;
    /** Maximum backscatter strength (permanently stored in tag memory)*/
    unsigned maxBackscatterStrength:1;
    /** Conditional Read Range Reduction open/short (permanently stored in tag memory)*/
    unsigned conditionalReadRangeReduction_openShort:1;
    /** Conditional Read Range Reduction on/off (permanently stored in tag memory)*/
    unsigned conditionalReadRangeReduction_onOff:1;
    /** Transparent mode data/raw (reset at power up)*/
    unsigned dataMode:1;
     /** Transparent mode on/off (reset at power up)*/
    unsigned transparentMode:1;
    /** Invert digital output (reset at power up)*/
    unsigned invertDigitalOutput:1;
    /** RFU 3*/
    unsigned RFU3:1;
    /** RFU 2*/
    unsigned RFU2:1;
    /** External supply flag digital input (read only)*/
    unsigned externalSupply:1;
    /** Tamper alarm flag (Read only)*/
    unsigned tamperAlarm:1;    
    /** Most significant bit */
  }bits;
}TMR_NXP_ConfigWord;

/**
 * The argument to a ChangeConfig command for NXP UCODE 7 tags
 * User can set the configWord directly using the 16 bit 'data' field.
 * Otherwise, provides an option to individually set each flag.
 * Instance of TMR_NXP_UCODE7_ConfigWord must be initialized before using either
 * by declaring it as 'static' or by using the constructor routine
 * TMR_init_NXP_UCODE7_ConfigWord() method.
 */
typedef union TMR_NXP_UCODE7_ConfigWord
{
  uint16_t data;
  /*
   * Since the bit-fields are used, The bits in configWord are shown from
   * Least significant bit to Most significant bit.
   */
  struct
  {
    /** Least significant bit */
    /** PSF alarm flag (Permanently stored in tag memory)*/
    unsigned psfAlarm:1;
    /** RFU 14*/
    unsigned RFU14:1;
    /** RFU 13*/
    unsigned RFU13:1;
     /** RFU 12*/
    unsigned RFU12:1;
    /** RFU 11*/
    unsigned RFU11:1;
    /** RFU 10*/
    unsigned RFU10:1;
    /** Maximum backscatter strength (permanently stored in tag memory)*/
    unsigned maxBackscatterStrength:1;
    /** RFU 8*/
    unsigned RFU8:1;
    /** RFU 7*/
    unsigned RFU7:1;
    /** RFU 6*/
    unsigned RFU6:1;
     /** RFU 5*/
    unsigned RFU5:1;
    /** Tag Power flag*/
    unsigned tagPower:1;
    /** RFU 3*/
    unsigned RFU3:1;
    /** Parallel Encoding flag*/
    unsigned parallelEncoding:1;
    /** RFU 1*/
    unsigned RFU1:1;
    /** RFU 0*/
    unsigned RFU0:1;    
    /** Most significant bit */
  }bits;
}TMR_NXP_UCODE7_ConfigWord;

/**
 * TMR_Monza4_ControlByte is an argument to Monza4 QTReadWrite custom command
 * User can set the control byte directly using the 8 bit 'data' field.
 * Otherwise, provides an option to individually set each flag.
 * Instance of TMR_Monza4_ControlByte must be initialized before using either
 * by declaring it as 'static' (or initializing it to 0) or by using the constructor routine
 * TMR_init_Monza4_ControlByte() method.
 */
typedef union TMR_Monza4_ControlByte
{
  /* 8-bit control byte */
  uint8_t data;
  /*
   * Since bit-fields are used, The bits in control Byte are shown from
   * Least significant bit to Most significant bit.
   */
  struct 
  {
    /**
     * The following bits are Reserved for Future Use. And will be ignored.
     * RFU_TM's are ThingMagic specific RFU bits, and RFU_Impinj bits are 
     * as per the Monza4 specification.
     */
    unsigned RFU_TM0:1;
    unsigned RFU_TM1:1;
    unsigned RFU_TM2:1;
    unsigned RFU_TM3:1;
    unsigned RFU_Impinj4:1;
    unsigned RFU_Impinj5:1;
    /**
     * If Read/Write=1, the Persistence field indicates whether the QT control is
     * written to nonvolatile (NVM) or volatile memory.
     * Persistence=0 means write to volatile memory. 
     * Persistence=1 means write to NVM memory
     */
    unsigned persistence:1;
    /**
     * The Read/Write bit indicates whether the tag reads or writes QT control data.
     * Read/Write=0 means read the QT control bits in cache.
     * Read/Write=1 means write the QT control bits
     */
    unsigned readWrite:1;    
  }bits;
}TMR_Monza4_ControlByte;

/**
 * TMR_Monza4_Payload is an argument to Monza4 QTReadWrite custom command
 * User can set the payload directly using the 16 bit 'data' field.
 * Otherwise, provides an option to individually set each flag.
 * Instance of TMR_Monza4_Payload must be initialized before using either
 * by declaring it as 'static' (or initializing it to 0) or by using the constructor routine
 * TMR_init_Monza4_Payload() method.
 * This field controls the QT functionality. These bits are ignored when the
 * Read/Write field equals 0 in control byte.
 */
typedef union TMR_Monza4_Payload
{
  /** 16-bit payload */
  uint16_t data;
  /*
   * Since bit-fields are used, The bits in payload are shown from
   * Least significant bit to Most significant bit.
   */
  struct
  {
    /**
     * The following bits are Reserved for Future Use. And will be ignored.
     * Tag will always return these bits as zero.
     * All these RFU bits are as per the monza4 spec.
     */
    unsigned RFU0:1;
    unsigned RFU1:1;
    unsigned RFU2:1;
    unsigned RFU3:1;
    unsigned RFU4:1;
    unsigned RFU5:1;
    unsigned RFU6:1;
    unsigned RFU7:1;
    unsigned RFU8:1;
    unsigned RFU9:1;
    unsigned RFU10:1;
    unsigned RFU11:1;
    unsigned RFU12:1;
    unsigned RFU13:1;
    /**
     *  Bit 14 
     *  1: Tag uses Public Memory Map 
     *  0: Tag uses Private Memory Map
     */
    unsigned QT_MEM:1;
    /**
     *  Bit 15 (MSB) is first transmitted bit of the payload field.
     *  Bit # Name Description
     *  1: Tag reduces range if in or about to be in OPEN or SECURED state
     *  0: Tag does not reduce range
     */
    unsigned QT_SR:1;
  }bits;
}TMR_Monza4_Payload;

typedef struct TMR_GEN2_HibikiSystemInformation
{
  uint16_t infoFlags;     /* Indicates whether the banks are present and Custom Commands are implemented*/ 
  uint8_t reservedMemory; /* Indicates the size of this memory bank in words*/
  uint8_t epcMemory;      /* Indicates the size of this memory bank in words*/
  uint8_t tidMemory;      /* Indicates the size of this memory bank in words*/
  uint8_t userMemory;     /* Indicates the size of this memory bank in words*/
  uint8_t setAttenuate;   /**/
  uint16_t bankLock;      /* Indicates Lock state for this type of lock*/
  uint16_t blockReadLock; /* Indicates Lock state for this type of lock*/
  uint16_t blockRwLock;   /* Indicates Lock state for this type of lock*/
  uint16_t blockWriteLock;/* Indicates Lock state for this type of lock*/
} TMR_GEN2_HibikiSystemInformation;

/** Size allocated for storing PC data in TMR_GEN2_TagData */
#define TMR_GEN2_MAX_PC_BYTE_COUNT (6)

/**
 * This represents the types of Q algorithms avaliable on the reader.
 */
typedef enum TMR_SR_GEN2_QType
{
  TMR_SR_GEN2_Q_MIN     = 0,
  TMR_SR_GEN2_Q_DYNAMIC = 0,
  TMR_SR_GEN2_Q_STATIC  = 1,
  TMR_SR_GEN2_Q_MAX     = TMR_SR_GEN2_Q_STATIC,
  TMR_SR_GEN2_Q_INVALID = TMR_SR_GEN2_Q_MAX + 1,
} TMR_SR_GEN2_QType;

/** Configuration of the static-Q algorithm. */
typedef struct TMR_SR_GEN2_QStatic
{
  /** The initial Q value to use. */
  uint8_t initialQ;
} TMR_SR_GEN2_QStatic;

/**
 * This represents a Q algorithm on the reader.
 */
typedef struct TMR_SR_GEN2_Q
{
  /** The type of Q algorithm. */
  TMR_SR_GEN2_QType type;
  union
  {
    /** Configuration of the static-Q algorithm. */
    TMR_SR_GEN2_QStatic staticQ;
  }u;
} TMR_SR_GEN2_Q;

/* This object contains the information related to the initialQ 
   that can be configured to start the read cycle with. */
typedef struct TMR_GEN2_initQ
{
  /* "Initial Q" enable flag(true or false) */ 
  bool qEnable;
  /* "Initial Q" value */
  uint8_t qValue;
}TMR_GEN2_initQ;

/**
 * Gen2-specific per-tag data
 */
typedef struct TMR_GEN2_TagData
{ 
  /** Length of the tag PC */
  uint8_t pcByteCount;
  /** Tag PC */
  uint8_t pc[TMR_GEN2_MAX_PC_BYTE_COUNT];
} TMR_GEN2_TagData;

/**
 * Gen2-specific parameters per-tag data
 */
typedef struct TMR_GEN2_TagReadData
{ 
  /** Read gen2 Q value */
  TMR_SR_GEN2_Q q;
  /** Read gen2 link frequency */
  TMR_GEN2_LinkFrequency lf;
  /** Read gen2 target*/
  TMR_GEN2_Target target;
} TMR_GEN2_TagReadData;

typedef enum TMR_SR_GEN2_ImpinjMonza6CmdEnum
{
  TMR_SR_GEN2_MARGIN_READ_COMMAND = 0x00
}GEN2_ImpinjMonza6_Cmd;

typedef enum TMR_SR_GEN2_NxpUcode7CmdEnum
{
  TMR_SR_GEN2_UCODE7_CHANGE_CONFIG_COMMAND = 0x08
}TMR_SR_GEN2_NxpUcode7Cmd;
#endif /* TMR_ENABLE_UHF */
#ifdef __cplusplus
}
#endif

#endif /*_TMR_GEN2_H*/
