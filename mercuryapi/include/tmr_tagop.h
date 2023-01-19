#ifndef _TMR_TAGOP_H
#define _TMR_TAGOP_H
/** 
 *  @file tmr_tagop.h
 *  @brief Mercury API - Tag Operations Interface
 *  @author Nathan Williams
 *  @date 1/8/2010
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
#include "tmr_tag_auth.h"
#include "osdep.h"

#ifdef  __cplusplus
extern "C" {
#endif

/** The type of a tag operation structure */
typedef enum TMR_TagOpType
{
#ifdef TMR_ENABLE_UHF
  /** Gen2 EPC write */
  TMR_TAGOP_GEN2_WRITETAG,
  /** Gen2 memory read */
  TMR_TAGOP_GEN2_READDATA,
  /** Gen2 memory write */
  TMR_TAGOP_GEN2_WRITEDATA,
  /** Gen2 memory lock/unlock */
  TMR_TAGOP_GEN2_LOCK,
  /** Gen2 tag kill */
  TMR_TAGOP_GEN2_KILL,
  /** Gen2 tag block write */
  TMR_TAGOP_GEN2_BLOCKWRITE,
  /** Gen2 tag block permalock */
  TMR_TAGOP_GEN2_BLOCKPERMALOCK,
  /** Gen2 tag block erase */
  TMR_TAGOP_GEN2_BLOCKERASE,
  /** Higgs2 Partial Load Image */
  TMR_TAGOP_GEN2_ALIEN_HIGGS2_PARTIALLOADIMAGE,
  /** Higgs2 Full Load Image */
  TMR_TAGOP_GEN2_ALIEN_HIGGS2_FULLLOADIMAGE,
  /** Higgs3 Fast Load Image */
  TMR_TAGOP_GEN2_ALIEN_HIGGS3_FASTLOADIMAGE,
  /** Higgs3 Load Image */
  TMR_TAGOP_GEN2_ALIEN_HIGGS3_LOADIMAGE,
  /** Higgs3 Block Read Lock */
  TMR_TAGOP_GEN2_ALIEN_HIGGS3_BLOCKREADLOCK,

  /** NXP set read protect */
  TMR_TAGOP_GEN2_NXP_SETREADPROTECT,
  /** NXP reset read protect */
  TMR_TAGOP_GEN2_NXP_RESETREADPROTECT,
  /** NXP Change EAS */
  TMR_TAGOP_GEN2_NXP_CHANGEEAS,
  /** NXP EAS Alarm */
  TMR_TAGOP_GEN2_NXP_EASALARM,
  /** NXP Calibrate */
  TMR_TAGOP_GEN2_NXP_CALIBRATE,
  /** NXP ChangeConfig */
  TMR_TAGOP_GEN2_NXP_CHANGECONFIG,

/** NXP AES Gen2v2 untraceable operations */
  TMR_TAGOP_GEN2_NXP_UNTRACEABLE,
  /** NXP AES Gen2v2 Authenticate operations */
  TMR_TAGOP_GEN2_NXP_AUTHENTICATE,
  /** NXP AES Gen2v2 ReadBuffer operations */
  TMR_TAGOP_GEN2_NXP_READBUFFER,


  /** Monza4 QT Read/Write*/
  TMR_TAGOP_GEN2_IMPINJ_MONZA4_QTREADWRITE,

  /** ISO180006B memory read */
  TMR_TAGOP_ISO180006B_READDATA,
  /** ISO180006B memory write */
  TMR_TAGOP_ISO180006B_WRITEDATA,
  /** ISO180006B memory lock/unlock */
  TMR_TAGOP_ISO180006B_LOCK,
  /** ISO180006B tag kill */
  /** Gen2 Secure read */
  TMR_TAGOP_GEN2_SECURE_READDATA,

  /** Gen2 IAVDenatran ActivateSecureMode - PA Protocol */
  TMR_TAGOP_GEN2_DENATRAN_IAV_ACTIVATESECUREMODE,
  /** Gen2 IAVDenatran Authenticate OBU - PA Protocol */
  TMR_TAGOP_GEN2_DENATRAN_IAV_AUTHENTICATEOBU,
  /** Gen2 IAVDenatran GEN2_ACTIVATE_SINIAV_MODE - G0 Protocol */
  TMR_TAGOP_GEN2_ACTIVATE_SINIAV_MODE,
  /** Gen2 IAVDenatran GEN2_OBU_AUTH_ID - G0 Protocol */
  TMR_TAGOP_GEN2_OBU_AUTH_ID,
  /** Gen2 IAVDenatran GEN2_AUTHENTICATE_OBU_FULL_PASS1 - G0 Protocol */
  TMR_TAGOP_GEN2_AUTHENTICATE_OBU_FULL_PASS1,
  /** Gen2 IAVDenatran GEN2_AUTHENTICATE_OBU_FULL_PASS2 - G0 Protocol */
  TMR_TAGOP_GEN2_AUTHENTICATE_OBU_FULL_PASS2,
  /** Gen2 IAVDenatran GEN2_OBU_READ_FROM_MEM_MAP - G0 Protocol */
  TMR_TAGOP_GEN2_OBU_READ_FROM_MEM_MAP,
  /** Gen2 IAVDenatran GEN2_OBU_WRITE_TO_MEM_MAP - G0 Protocol */
  TMR_TAGOP_GEN2_OBU_WRITE_TO_MEM_MAP,
  /** Gen2 IAVDenatran GET_TOKEN_ID - G0 Protocol */
  TMR_TAGOP_GEN2_DENATRAN_IAV_GET_TOKEN_ID,
  /** Gen2 IAVDenatran AUTHENTICATE_OBU_FULL_PASS */
  TMR_TAGOP_GEN2_DENATRAN_IAV_AUTHENTICATE_OBU_FULL_PASS,
  /** Gen2 IAVDenatran Read Sec - IP63 protocol */
  TMR_TAGOP_GEN2_DENATRAN_IAV_READ_SEC,
  /** Gen2 IAVDenatran Write Sec - IP63 protocol */
  TMR_TAGOP_GEN2_DENATRAN_IAV_WRITE_SEC,
  /** Gen2 IAVDenatran GEN2_G0_PA_OBU_AUTHENTICATE_ID - ( PA + G0 ) Protocol */
  TMR_TAGOP_GEN2_DENATRAN_IAV_G0_PA_OBU_AUTHENTICATE_ID,
  
  /** Gen2 Ids get sensor type */
  TMR_TAGOP_GEN2_IDS_SL900A_GETSENSOR,
  /** Gen2 Ids get log state */
  TMR_TAGOP_GEN2_IDS_SL900A_GETLOGSTATE,
  /** Gen2 Ids set log mode */
  TMR_TAGOP_GEN2_IDS_SL900A_SETLOGMODE,
  /** Gen2 Ids end log */
  TMR_TAGOP_GEN2_IDS_SL900A_ENDLOG,
  /** Gen2 Ids Initialize */
  TMR_TAGOP_GEN2_IDS_SL900A_INITIALIZE,
  /** Gen2 Ids Fifo Status */
  TMR_TAGOP_GEN2_IDS_SL900A_ACCESSFIFOSTATUS,
  /** Gen2 Ids Fifo Read */
  TMR_TAGOP_GEN2_IDS_SL900A_ACCESSFIFOREAD,
  /** Gen2 Ids Fifo write */
  TMR_TAGOP_GEN2_IDS_SL900A_ACCESSFIFOWRITE,
  /** Gen2 Ids Starrt Log */
  TMR_TAGOP_GEN2_IDS_SL900A_STARTLOG,
  /** Gen2 Ids GetCalibrationData */
  TMR_TAGOP_GEN2_IDS_SL900A_GETCALIBRATIONDATA,
  /** Gen2 Ids SetCalibration */
  TMR_TAGOP_GEN2_IDS_SL900A_SETCALIBRATIONDATA,
  /** Gen2 Ids SetSfeParameters */
  TMR_TAGOP_GEN2_IDS_SL900A_SETSFEPARAMETERS,
  /** Gen2 Ids GetMeasurmentSetup */
  TMR_TAGOP_GEN2_IDS_SL900A_GETMEASUREMENTSETUP,
  /** Gen2 Ids GetBattery Level */
  TMR_TAGOP_GEN2_IDS_SL900A_GETBATTERYLEVEL,
  /** Gen2 Ids SetLogLimits */
  TMR_TAGOP_GEN2_IDS_SL900A_SETLOGLIMITS,
  /** Gen2 Ids SetShelfLife */
  TMR_TAGOP_GEN2_IDS_SL900A_SETSHELFLIFE,
  /** Set IDS Password */
  TMR_TAGOP_GEN2_IDS_SL900A_SETPASSWORD,
  /** Monza6 Margin Read */
  TMR_TAGOP_GEN2_IMPINJ_MONZA6_MARGINREAD,
  /** NXP UCODE 7 ChangeConfig */ 
  TMR_TAGOP_GEN2_NXP_UCODE7_CHANGECONFIG,
  /** Fudan Read reg */
  TMR_TagOP_GEN2_FUDAN_READREG,
  /** Fudan Write Reg */
  TMR_TagOP_GEN2_FUDAN_WRITEREG,
  /** Fudan Load reg */
  TMR_TagOP_GEN2_FUDAN_LOADREG,
  /** Fudan Start/Stop LOG */
  TMR_TagOP_GEN2_FUDAN_STARTSTOPLOG,
  /** Fudan Auth */
  TMR_TagOP_GEN2_FUDAN_AUTH,
  /** Fudan Read Mem */
  TMR_TagOP_GEN2_FUDAN_READMEM,
  /** Fudan Write Mem */
  TMR_TagOP_GEN2_FUDAN_WRITEMEM,
  /** Fudan State Check */
  TMR_TagOP_GEN2_FUDAN_STATECHECK,
  /** Fudan Measure */
  TMR_TagOP_GEN2_FUDAN_MEASURE,
  /** ILIAN tag select command */
  TMR_TagOP_GEN2_ILIAN_TAG_SELECT_COMMAND,
  /** EM-MICRO EM4325 get sensor data */
  TMR_TagOP_GEN2_EM4325_GET_SENSOR_DATA,
  /** EM-MICRO EM4325 reset alarms */
  TMR_TagOP_GEN2_EM4325_RESET_ALARMS,
  /** List of tag operations */
  TMR_TAGOP_LIST,
#endif /* TMR_ENABLE_UHF */

#ifdef TMR_ENABLE_HF_LF
  /* Block Write Operation */
  TMR_TAGOP_WRITE_MEMORY,
  /* Block Read Operation */
  TMR_TAGOP_READ_MEMORY,
  /* PassThrough Operation */
  TMR_TAGOP_PASS_THROUGH,
#endif /* TMR_ENABLE_HF_LF */
} TMR_TagOpType;

#ifdef TMR_ENABLE_UHF
/** The type of Gen2 Secure operation  */
typedef enum SecureTagType
{
  GEN2_EMBEDDED_SECURE_DEFAULT = 0x00,
  /* Alien Higgs 3 secure access */
  GEN2_EMBEDDED_SECURE_HIGGS3_ACCESS   = (1 << 1),
  /* Monza 4 secure access */
  GEN2_EMBEDDED_SECURE_MONZA4_ACCESS   = (1 << 2)
}SecureTagType;
 
/* The type of Gen2 IAVDenatranCmdEnum */
typedef enum IAVDenatranSecureTagOpType
{
  /* Activate Secure Mode - PA Protocol */
  GEN2_ACTIVATE_SECURE_MODE  = 0x00,
  /* Authenticate OBU - PA Protocol */
  GEN2_AUTHENTICATE_OBU  = 0x01,
  /* Activate Siniav Mode OBU - G0 Protocol */
  GEN2_ACTIVATE_SINIAV_MODE = 0x02,
  /* Authenticate OBU ID - G0 Protocol */
  GEN2_OBU_AUTH_ID = 0x03,
  /* Authenticate OBU FullPass1 - G0 Protocol */
  GEN2_AUTHENTICATE_OBU_FULL_PASS1 = 0x04,
  /* Authenticate OBU FullPass2 - G0 Protocol */
  GEN2_AUTHENTICATE_OBU_FULL_PASS2 = 0x05,
  /* Read From Mem - G0 Protocol */
  GEN2_OBU_READ_FROM_MEM_MAP = 0x06,
  /* Write to Mem - G0 Protocol */
  GEN2_OBU_WRITE_TO_MEM_MAP = 0x07,
  /* Authenticate OBU FullPass2 - G0 Protocol */
  GEN2_AUTHENTICATE_OBU_FULL_PASS = 0x08,
  /* Get token ID - Go Protocol */
  GEN2_GET_TOKEN_ID = 0x09,
  /* Read Sec - IP63 Protocol */
  GEN2_READ_SEC = 0x0A,
  /* Write Sec - IP63 Protocol */
  GEN2_WRITE_SEC = 0x0B,
  /* Pa G0 - (PA + G0) Protocol */
  GEN2_PA_G0_AUTHENTICATE = 0x0C,
}IAVDenatranSecureTagOpType;

/* The type of Gen2 secure password  */
typedef enum SecurePasswordType
{
  /* Gen2 password type */
  TMR_SECURE_GEN2_PASSWORD = 0x01,
  /* Look up table password */
  TMR_SECURE_GEN2_LOOKUP_TABLE_PASSWORD = 0x02
}SecurePasswordType;

/* The fild type of calibration data that user can modify */
typedef enum CalibrationParameter
{
  /* AD1 lower voltage reference - coarse */
  TMR_GEN2_IDS_SL900A_CALIBRATION_COARSE1,
  /* AD2 lower voltage reference - coarse */
  TMR_GEN2_IDS_SL900A_CALIBRATION_COARSE2,
  /* Switches the lower AD voltage reference to ground */
  TMR_GEN2_IDS_SL900A_CALIBRATION_GNDSWITCH,
  /* POR voltage level for 1.5V system */
  TMR_GEN2_IDS_SL900A_CALIBRATION_SELP12,
  /* RTC oscillator calibration */
  TMR_GEN2_IDS_SL900A_CALIBRATION_DF,
  /* Controlled battery supply for external sensor - the battery voltage is connected to the EXC pin */
  TMR_GEN2_IDS_SL900A_CALIBRATION_SWEXTEN,
  /* POR voltage level for 3V system */
  TMR_GEN2_IDS_SL900A_CALIBRATION_SELP22,
  /* Voltage level interrupt level for external sensor -- ratiometric */
  TMR_GEN2_IDS_SL900A_CALIBRATION_IRLEV,
  /* Excitate for resistive sensors without DC */
  TMR_GEN2_IDS_SL900A_CALIBRATION_EXCRES,
}CalibrationParameter;

/* The field type of sfe parameters that user can modify */
typedef enum SfeParameter
{
  /* External sensor 2 range */
  TMR_GEN2_IDS_SL900A_SFE_RANG,
  /* External sensor 1 range */
  TMR_GEN2_IDS_SL900A_SFE_SETI,
  /* External sensor 1 type */
  TMR_GEN2_IDS_SL900A_SFE_EXT1,
  /* External sensor 2 type */
  TMR_GEN2_IDS_SL900A_SFE_EXT2,
  /* Use preset range */
  TMR_GEN2_IDS_SL900A_SFE_AUTORANGEDISABLE,
  /* Sensor used in limit check */
  TMR_GEN2_IDS_SL900A_SFE_VERIFYSENSORID,
}SfeParameter;

typedef struct TMR_TagOp_GEN2_NXP_Tam1Authentication
{
	uint8_t Authentication;
	uint8_t CSI;
	uint8_t keyID;
	uint8_t KeyLength;
        uint8_t keyData[32];
	TMR_uint8List  Key;
} TMR_TagOp_GEN2_NXP_Tam1Authentication;

typedef struct TMR_TagOp_GEN2_NXP_Tam2Authentication
{
	TMR_TagOp_GEN2_NXP_Tam1Authentication tam1Auth;
	uint16_t Offset;
	uint8_t ProtMode;
	uint8_t BlockCount;
	TMR_NXP_Profile profile;
} TMR_TagOp_GEN2_NXP_Tam2Authentication;

typedef enum TMR_GEN2_UNTRACEABLE_AuthType
{
	/* Tag exposes EPC memory */
	UNTRACEABLE_WITH_AUTHENTICATION = 0,
	/*Tag untraceably hides EPC memory above that*/
	UNTRACEABLE_WITH_ACCESS = 1

}TMR_GEN2_UNTRACEABLE_AuthType;
typedef struct TMR_TagOp_GEN2_NXP_UNTRACEABLE_Authentication
{   
	TMR_GEN2_UNTRACEABLE_AuthType authType;
	TMR_TagOp_GEN2_NXP_Tam1Authentication tam1Auth;
	TMR_GEN2_Password accessPassword;

}TMR_TagOp_GEN2_NXP_UNTRACEABLE_Authentication;

typedef struct TMR_TagOp_GEN2_NXP_Untraceable
{
	TMR_TagOp_GEN2_NXP_UNTRACEABLE_Authentication auth; 
	TMR_GEN2_UNTRACEABLE_Epc epc;
	int32_t epcLength;
	TMR_GEN2_UNTRACEABLE_Tid tid;
	TMR_GEN2_UNTRACEABLE_UserMemory userMemory;
	TMR_GEN2_UNTRACEABLE_Range range;
	uint8_t subCommand;

} TMR_TagOp_GEN2_NXP_Untraceable;

typedef enum TMR_GEN2_AUTHENTICATE_Type
{
	TAM1_AUTHENTICATION,
	TAM2_AUTHENTICATION
}TMR_GEN2_AUTHENTICATE_Type;

typedef struct TMR_TagOp_GEN2_NXP_Authenticate
{
	TMR_GEN2_AUTHENTICATE_Type type;
        TMR_NXP_KeyId keyType;
	TMR_TagOp_GEN2_NXP_Tam1Authentication tam1Auth;
	TMR_TagOp_GEN2_NXP_Tam2Authentication tam2Auth;
	uint8_t subCommand;
        bool sendRawData;
}TMR_TagOp_GEN2_NXP_Authenticate;

typedef struct TMR_TagOp_GEN2_NXP_Readbuffer
{
	uint16_t wordPointer;
	uint16_t bitCount;
	TMR_TagOp_GEN2_NXP_Authenticate authenticate;

}TMR_TagOp_GEN2_NXP_Readbuffer;

/** Parameters of a Gen2 EPC write operation */
typedef struct TMR_TagOp_GEN2_WriteTag
{
  /** Tag EPC */
  TMR_TagData* epcptr;
} TMR_TagOp_GEN2_WriteTag;

/** Parameters of a Gen2 memory read operation */
typedef struct TMR_TagOp_GEN2_ReadData
{
  /** Gen2 memory bank to read from */
  TMR_GEN2_Bank bank;
  /** Word address to start reading at */
  uint32_t wordAddress;
  /** Number of words to read */
  uint8_t len;
} TMR_TagOp_GEN2_ReadData;

/** Parameters of Gen2 Secure operation */

/* Parameters of SecurePassWordLookup Table */
typedef struct SecurePasswordLookup
{
  /* used in case Gen2 passWord */
  TMR_TagAuthentication gen2PassWord;

  /* used in case of password look table */
  /* Number of bits used to address the AP list (MSB byte - byte 4) */
  uint8_t secureAddressLength;
  /* EPC word offset (Next MSB byte - byte 3) */
  uint8_t secureAddressOffset;
  /*  User flash offset, starting from 0x0000 (LSB 2 bytes) */
   uint16_t secureFlashOffset;
} SecurePasswordLookup;

typedef struct TMR_TagOp_GEN2_SecureReadData
{
  /** Gen2 read operation*/
  TMR_TagOp_GEN2_ReadData readData;

  /** type of Gen2 Secure operation*/
  /* Options to select Alien Higgs 3 secure access and Monza 4 secure access */
  SecureTagType type;
  /* option to select Gen2 password or look up table password */
  SecurePasswordType passwordType;
  /* Options to specify the password */
  SecurePasswordLookup password;
 } TMR_TagOp_GEN2_SecureReadData;

/** Parameters of a Gen2 memory write operation */
typedef struct TMR_TagOp_GEN2_WriteData
{
  /** Gen2 memory bank to write to */
  TMR_GEN2_Bank bank;
  /** Word address to start writing at */
  uint32_t wordAddress;
  /** Data to write */
  TMR_uint16List data;
} TMR_TagOp_GEN2_WriteData;

/** Parameters of a Gen2 memory lock/unlock operation */
typedef struct TMR_TagOp_GEN2_Lock
{ 
  /** Bitmask indicating which lock bits to change */
  uint16_t mask;
  /** New values of each bit specified in the mask */
  uint16_t action;
  /** Access Password to use to lock the tag*/
  TMR_GEN2_Password accessPassword;
} TMR_TagOp_GEN2_Lock;

/** Parameters of a Gen2 tag kill operation */
typedef struct TMR_TagOp_GEN2_Kill
{
  /** Kill password to use to kill the tag */
  TMR_GEN2_Password password;
} TMR_TagOp_GEN2_Kill;

/** Parameters of a Gen2 tag Block Write operation */
typedef struct TMR_TagOp_GEN2_BlockWrite
{
  /** Gen2 memory bank to write to */
  TMR_GEN2_Bank bank; 
  /** The word address to start writing to */
  uint32_t wordPtr; 
  /** The data to write */
  TMR_uint16List data; 
}TMR_TagOp_GEN2_BlockWrite;

/** Parameters of a Gen2 tag Block PermaLock operation */
typedef struct TMR_TagOp_GEN2_BlockPermaLock
{
  /** Read lock status or write it? */
  uint8_t readLock; 
  /** Gen2 memory bank to lock */
  TMR_GEN2_Bank bank; 
  /** The starting word address to lock */
  uint32_t blockPtr;
  /** Mask: Which blocks to lock? */
  TMR_uint16List mask;
}TMR_TagOp_GEN2_BlockPermaLock;

/** Parameters of a Gen2 tag Block Erase operation */
typedef struct TMR_TagOp_GEN2_BlockErase
{
  /** Gen2 memory bank to erase */
  TMR_GEN2_Bank bank;
  /** The starting word address for block erase */
  uint32_t wordPtr;
  /** Number of words to erase */
  uint8_t wordCount;
} TMR_TagOp_GEN2_BlockErase;

/** Parameters of a Gen2 memory lock/unlock operation */
typedef struct TMR_TagOp_ISO180006B_Lock
{ 
  /** The memory address of the byte to lock */
  uint8_t address;
} TMR_TagOp_ISO180006B_Lock;

/** Parameters of an ISO180006B memory read operation */
typedef struct TMR_TagOp_ISO180006B_ReadData
{
  /** Byte address to start reading at */
  uint8_t byteAddress;
  /** Number of bytes to read */
  uint8_t len;
} TMR_TagOp_ISO180006B_ReadData;

/** Parameters of an ISO180006B memory write operation */
typedef struct TMR_TagOp_ISO180006B_WriteData
{
  /** Byte address to start writing at */
  uint8_t byteAddress;
  /** Data to write */
  TMR_uint8List data;
} TMR_TagOp_ISO180006B_WriteData;

/**
 *  Tagops for Gen2 custom commands
 **/

/** Parameters for Alien Higgs2, Partial Load Image command*/
typedef struct TMR_TagOp_GEN2_Alien_Higgs2_PartialLoadImage
{
  /** Kill password to write to the tag */
  TMR_GEN2_Password killPassword;
  /** Access password to write to the tag */
  TMR_GEN2_Password accessPassword;
  /** Tag EPC to write to the tag*/
  TMR_TagData* epcptr;
} TMR_TagOp_GEN2_Alien_Higgs2_PartialLoadImage;

/** Parameters for Alien Higgs2, Full Load Image command*/
typedef struct TMR_TagOp_GEN2_Alien_Higgs2_FullLoadImage
{
  /** Kill password to write to the tag */
  TMR_GEN2_Password killPassword;
  /** Access password to write to the tag */
  TMR_GEN2_Password accessPassword;
  /** Lock Bits */
  uint16_t lockBits;
  /** PC word */
  uint16_t pcWord;
  /** Tag EPC to write to the tag*/
  TMR_TagData* epcptr;
} TMR_TagOp_GEN2_Alien_Higgs2_FullLoadImage;

/** Parameters for Alien Higgs3, Fast Load Image command*/
typedef struct TMR_TagOp_GEN2_Alien_Higgs3_FastLoadImage
{
  /** Access password used to write to the tag */
  TMR_GEN2_Password currentAccessPassword;
  /** Kill password to write to the tag */
  TMR_GEN2_Password killPassword;
  /** Access password to write to the tag */
  TMR_GEN2_Password accessPassword;
  /** PC word */
  uint16_t pcWord;
  /** Tag EPC to write to the tag*/
  TMR_TagData* epcptr;
} TMR_TagOp_GEN2_Alien_Higgs3_FastLoadImage;


/** Parameters for Alien Higgs3, Load Image command*/
typedef struct TMR_TagOp_GEN2_Alien_Higgs3_LoadImage
{
  /** Access password used to write to the tag */
  TMR_GEN2_Password currentAccessPassword;
  /** Kill password to write to the tag */
  TMR_GEN2_Password killPassword;
  /** Access password to write to the tag */
  TMR_GEN2_Password accessPassword;
  /** PC word */
  uint16_t pcWord;
  /** Tag EPC and user data to write to the tag (76 bytes)*/
  TMR_uint8List *epcAndUserData;
} TMR_TagOp_GEN2_Alien_Higgs3_LoadImage;

/** Parameters for Alien Higgs3, Block Read Lock command*/
typedef struct TMR_TagOp_GEN2_Alien_Higgs3_BlockReadLock
{
  /** Access password to use to write to the tag, in case if the tag is already locked */
  TMR_GEN2_Password accessPassword;
  /** A bitmask of bits to lock */
  uint8_t lockBits;
} TMR_TagOp_GEN2_Alien_Higgs3_BlockReadLock;

/** Parameters for NXP, Set Read Protect command */
typedef struct TMR_TagOp_GEN2_NXP_SetReadProtect
{
  /** Access password to use to write to the tag */
  TMR_GEN2_Password accessPassword;
}TMR_TagOp_GEN2_NXP_SetReadProtect;

/** Parameters for NXP, Reset Read Protect Command*/
typedef struct TMR_TagOp_GEN2_NXP_ResetReadProtect
{
  /** Access password to use to write to the tag */
  TMR_GEN2_Password accessPassword;
} TMR_TagOp_GEN2_NXP_ResetReadProtect;

/** Parameters for NXP, Change EAS Command*/
typedef struct TMR_TagOp_GEN2_NXP_ChangeEAS
{
  /** Access password to use to write to the tag */
  TMR_GEN2_Password accessPassword;
  /** Reset or set EAS bit */
  bool reset;
} TMR_TagOp_GEN2_NXP_ChangeEAS;

/** Parameters for NXP, EAS alarm command */
typedef struct TMR_TagOp_GEN2_NXP_EASAlarm
{
  /** Gen2 divide ratio to use */
  TMR_GEN2_DivideRatio dr;
  /** Gen2 M parameter to use */
  TMR_GEN2_TagEncoding m;
  /** Gen2 TrExt value to use */
  TMR_GEN2_TrExt trExt;  
} TMR_TagOp_GEN2_NXP_EASAlarm;

/** Parameters for NXP, Calibration command */
typedef struct TMR_TagOp_GEN2_NXP_Calibrate
{
  /** Access password to use to write to the tag */
  TMR_GEN2_Password accessPassword;
} TMR_TagOp_GEN2_NXP_Calibrate;

/** Parameters for NXP UCODE 7, Change Config command */
typedef struct TMR_TagOp_GEN2_NXP_UCODE7_ChangeConfig
{
  /** Access password to use to write to the tag */
  TMR_GEN2_Password accessPassword;
  /** ConfigWord to write to the tag*/
  TMR_NXP_UCODE7_ConfigWord configWord;
}TMR_TagOp_GEN2_NXP_UCODE7_ChangeConfig;

/** Parameters for NXP, Change Config command */
typedef struct TMR_TagOp_GEN2_NXP_ChangeConfig
{
  /** Access password to use to write to the tag */
  TMR_GEN2_Password accessPassword;
  /** ConfigWord to write to the tag*/
  TMR_NXP_ConfigWord configWord;
}TMR_TagOp_GEN2_NXP_ChangeConfig;

/** Parameters for Impinj, Monza4 QTReadWrite command */
typedef struct TMR_TagOp_GEN2_Impinj_Monza4_QTReadWrite
{
  /** Access password to use to write to the tag */
  TMR_GEN2_Password accessPassword;
  /** Control Byte to write to the tag*/
  TMR_Monza4_ControlByte controlByte;
  /** Payload */
  TMR_Monza4_Payload payload;    
}TMR_TagOp_GEN2_Impinj_Monza4_QTReadWrite;
 
/** Parameters for Impinj, Monza6 Margin Read command */
typedef struct TMR_TagOp_GEN2_Impinj_Monza6_MarginRead
{
  /** Gen2 memory bank to read from */
  TMR_GEN2_Bank bank;
  /** bit address to start reading at */
  uint32_t bitAddress;
  /** Number of mask bits */
  uint16_t maskBitLength;   
  /** Pointer to mask */
  uint8_t *mask;
}TMR_TagOp_GEN2_Impinj_Monza6_MarginRead;

/** Parameters for IAV, ActivateSecureMode command */
typedef struct TMR_TagOp_GEN2_Denatran_IAV_Activate_Secure_Mode
{
  /* Enum for IAV tagoperation */
  IAVDenatranSecureTagOpType mode;
  /* RFU field */
  uint8_t payload;
}TMR_TagOp_GEN2_Denatran_IAV_Activate_Secure_Mode;

/** Parameters for IAV, AuthenticateOBU command */
typedef struct TMR_TagOp_GEN2_Denatran_IAV_Authenticate_OBU
{
  /* Enum for IAV tagoperation */
  IAVDenatranSecureTagOpType mode;
  /* RFU field */
  uint8_t payload;
}TMR_TagOp_GEN2_Denatran_IAV_Authenticate_OBU;

/** Parameters for IAV, Activate_Siniav_Mode command - G0 Protocol */
typedef struct TMR_TagOp_GEN2_Denatran_IAV_Activate_Siniav_Mode
{
  /* Enum for IAV tagoperation */
  IAVDenatranSecureTagOpType mode;
  /* RFU field */
  uint8_t payload;
  /* token desc */
  bool isTokenDesc;
  /* 64 bits of token data */
  uint8_t token[8];
}TMR_TagOp_GEN2_Denatran_IAV_Activate_Siniav_Mode;

/** Parameters for IAV, OBU_Auth_ID command - G0 Protocol */
typedef struct TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_ID
{
  /* Enum for IAV tagoperation */
  IAVDenatranSecureTagOpType mode;
  /* RFU field */
  uint8_t payload;
}TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_ID;

/** Parameters for IAV, OBU_Auth_Full_Pass1 command - G0 Protocol */
typedef struct TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_Full_Pass1
{
  /* Enum for IAV tagoperation */
  IAVDenatranSecureTagOpType mode;
  /* RFU field */
  uint8_t payload;
}TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_Full_Pass1;

/** Parameters for IAV,  OBU_Auth_Full_Pass2 command - G0 Protocol */
typedef struct TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_Full_Pass2
{
  /* Enum for IAV tagoperation */
  IAVDenatranSecureTagOpType mode;
  /* RFU field */
  uint8_t payload;
}TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_Full_Pass2;

/** Parameters for IAV,  OBU_ReadFromMemMap command - G0 Protocol */
typedef struct TMR_TagOp_GEN2_Denatran_IAV_OBU_ReadFromMemMap
{
  /* Enum for IAV tagoperation */
  IAVDenatranSecureTagOpType mode;
  /* RFU field */
  uint8_t payload;
  /* address to be read from USER bank */
  uint16_t readPtr;
}TMR_TagOp_GEN2_Denatran_IAV_OBU_ReadFromMemMap;

/** Parameters for IAV, read sec  command - IP63 Protocol */
typedef struct TMR_TagOp_GEN2_Denatran_IAV_Read_Sec
{
  /* Enum for IAV tagoperation */
  IAVDenatranSecureTagOpType mode;
  /* RFU field */
  uint8_t payload;
  /* address to be read from USER bank */
  uint16_t readPtr;
}TMR_TagOp_GEN2_Denatran_IAV_Read_Sec;

/** Parameters for IAV,  OBU_WriteToMemMap command - G0 Protocol */
typedef struct TMR_TagOp_GEN2_Denatran_IAV_OBU_WriteToMemMap
{
  /* Enum for IAV tagoperation */
  IAVDenatranSecureTagOpType mode;
  /* RFU field */
  uint8_t payload;
  /* Pointer to the USER data */
  uint16_t writePtr;
  /* data to be written */
  uint16_t wordData;
  /* Tag ID */
  uint8_t* tagIdentification;
  /* Credentials written word */
  uint8_t* dataBuf;  
}TMR_TagOp_GEN2_Denatran_IAV_OBU_WriteToMemMap;

/** Parameters for IAV,  write sec command - IP63 Protocol */
typedef struct TMR_TagOp_GEN2_Denatran_IAV_Write_Sec
{
  /* Enum for IAV tagoperation */
  IAVDenatranSecureTagOpType mode;
  /* RFU field */
  uint8_t payload;
  /* data words */
  uint8_t* dataWords;
  /* Credentials written word */
  uint8_t* dataBuf;
}TMR_TagOp_GEN2_Denatran_IAV_Write_Sec;

/** Parameters for IAV, Get Token ID command */
typedef struct TMR_TagOp_GEN2_Denatran_IAV_Get_Token_Id
{
  /* Enum for IAV tagoperation */
  IAVDenatranSecureTagOpType mode;
}TMR_TagOp_GEN2_Denatran_IAV_Get_Token_Id;

/** Parameters for IAV, Authenticate OBU Full Pass */
typedef struct TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_Full_Pass
{
  /* Enum for IAV tagoperation */
  IAVDenatranSecureTagOpType mode;
  /* RFU field */
  uint8_t payload;
}TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_Full_Pass;

/** Parameters for IAV, G0_PA_OBU_Auth_ID */
typedef struct TMR_TagOp_GEN2_Denatran_IAV_G0_PA_OBU_Auth_ID
{
  /* Enum for IAV tagoperation */
  IAVDenatranSecureTagOpType mode;
  /* RFU field */
  uint8_t payload;
}TMR_TagOp_GEN2_Denatran_IAV_G0_PA_OBU_Auth_ID;
#endif /* TMR_ENABLE_UHF */

/* Forward declaration for the benefit of TMR_TagOp_List */
typedef struct TMR_TagOp TMR_TagOp;

#ifdef TMR_ENABLE_UHF
/** List of tag operations */
typedef struct TMR_TagOp_List
{
  /** Array of pointers to tag operations*/
  TMR_TagOp **list;
  /** Number of tag operations in list */
  uint16_t len;
} TMR_TagOp_List;

/** Sub-class for Gen2 Alien Higgs2 custom tag extensions */
typedef struct TMR_TagOp_GEN2_Alien_Higgs2
{
  union
  {
    TMR_TagOp_GEN2_Alien_Higgs2_PartialLoadImage partialLoadImage;
    TMR_TagOp_GEN2_Alien_Higgs2_FullLoadImage fullLoadImage;
  } u;
}TMR_TagOp_GEN2_Alien_Higgs2;

/** Sub-class for Gen2 Alien Higgs3 custom tag extensions */
typedef struct TMR_TagOp_GEN2_Alien_Higgs3
{
  union
  {
    TMR_TagOp_GEN2_Alien_Higgs3_FastLoadImage fastLoadImage;
    TMR_TagOp_GEN2_Alien_Higgs3_LoadImage loadImage;
    TMR_TagOp_GEN2_Alien_Higgs3_BlockReadLock blockReadLock;
  } u;
}TMR_TagOp_GEN2_Alien_Higgs3;

/** Sub-class for Gen2 Alien custom tag extensions */
typedef struct TMR_TagOp_GEN2_Alien
{
  union
  {
    TMR_TagOp_GEN2_Alien_Higgs2 higgs2;
    TMR_TagOp_GEN2_Alien_Higgs3 higgs3;
  } u;
}TMR_TagOp_GEN2_Alien;

/** Sub-class for Gen2 NXP custom tag extensions */
typedef struct TMR_TagOp_GEN2_NXP
{
  union
  {
    TMR_TagOp_GEN2_NXP_SetReadProtect setReadProtect;
    TMR_TagOp_GEN2_NXP_ResetReadProtect resetReadProtect;
    TMR_TagOp_GEN2_NXP_ChangeEAS changeEAS;
    TMR_TagOp_GEN2_NXP_EASAlarm EASAlarm;
    TMR_TagOp_GEN2_NXP_Calibrate calibrate;
    TMR_TagOp_GEN2_NXP_ChangeConfig changeConfig;
    TMR_TagOp_GEN2_NXP_Untraceable untraceable;
    TMR_TagOp_GEN2_NXP_Authenticate authenticate;
    TMR_TagOp_GEN2_NXP_Readbuffer readBuffer;
    TMR_TagOp_GEN2_NXP_UCODE7_ChangeConfig ucode7ChangeConfig;
  }u;
}TMR_TagOp_GEN2_NXP;

/** Sub-class for Gen2 Impinj Monza4 custom tag extensions */
typedef struct TMR_TagOp_GEN2_Impinj_Monza4
{
  union
  {
    TMR_TagOp_GEN2_Impinj_Monza4_QTReadWrite qtReadWrite;
  } u;
}TMR_TagOp_GEN2_Impinj_Monza4;

/** Sub-class for Gen2 Impinj Monza6 custom tag extensions */
typedef struct TMR_TagOp_GEN2_Impinj_Monza6
{
  union
  {
    TMR_TagOp_GEN2_Impinj_Monza6_MarginRead marginRead;
  } u;
}TMR_TagOp_GEN2_Impinj_Monza6;

/** Sub-class for Gen2 Impinj custom tag extensions */
typedef struct TMR_TagOp_GEN2_Impinj
{
  union
  {
    TMR_TagOp_GEN2_Impinj_Monza4 monza4;
    TMR_TagOp_GEN2_Impinj_Monza6 monza6;
  } u;
}TMR_TagOp_GEN2_Impinj;

/** Sub-class for Gen2 IavDenatran custom tag extensions */
typedef struct TMR_TagOp_GEN2_Denatran
{
  union
  {
    TMR_TagOp_GEN2_Denatran_IAV_Activate_Secure_Mode secureMode;
    TMR_TagOp_GEN2_Denatran_IAV_Authenticate_OBU authenticateOBU;
    TMR_TagOp_GEN2_Denatran_IAV_Activate_Siniav_Mode activateSiniavMode;
    TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_ID obuAuthId;
    TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_Full_Pass1 obuAuthFullPass1;
    TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_Full_Pass2 obuAuthFullPass2;
    TMR_TagOp_GEN2_Denatran_IAV_OBU_ReadFromMemMap obuReadFromMemMap;
    TMR_TagOp_GEN2_Denatran_IAV_OBU_WriteToMemMap obuWriteToMemMap;
    TMR_TagOp_GEN2_Denatran_IAV_Get_Token_Id getTokenId;
    TMR_TagOp_GEN2_Denatran_IAV_Read_Sec readSec;
    TMR_TagOp_GEN2_Denatran_IAV_Write_Sec writeSec;
    TMR_TagOp_GEN2_Denatran_IAV_OBU_Auth_Full_Pass obuAuthFullPass;
    TMR_TagOp_GEN2_Denatran_IAV_G0_PA_OBU_Auth_ID g0paobuauthid;
  } u;
}TMR_TagOp_GEN2_Denatran;

/** Sensor type for IDS Sl900A chip */
typedef enum Sensor
{
  /* SL900A sensor type values */
  TMR_GEN2_IDS_SL900A_SENSOR_TEMP  = 0x00,
  TMR_GEN2_IDS_SL900A_SENSOR_EXT1  = 0x01,
  TMR_GEN2_IDS_SL900A_SENSOR_EXT2  = 0x02,
  TMR_GEN2_IDS_SL900A_SENSOR_BATTV = 0x03,
}Sensor;

/** SL900A password access level values */
typedef enum PasswordLevel
{
  /* No access to any protected area */
  TMR_GEN2_IDS_SL900A_PASSWORD_NOT_ALLOWED = 0x00,
  /* System area access */
  TMR_GEN2_IDS_SL900A_PASSWORD_SYSTEM = 0x01,
  /* Application area access */
  TMR_GEN2_IDS_SL900A_PASSWORD_APPLICATION = 0x02,
  /* Measurement area access */
  TMR_GEN2_IDS_SL900A_PASSWORD_MEASUREMENT = 0x03,
}PasswordLevel;

/** SL900A Data log format selection */
typedef enum LoggingForm
{
  /* Dense Logging */
  TMR_GEN2_IDS_SL900A_LOGGINGFORM_DENSE = 0x00,
  /* Log values outside specified limits */
  TMR_GEN2_IDS_SL900A_LOGGINGFORM_OUTOFLIMITS = 0x01,
  /* Log values at limit crossing points */
  TMR_GEN2_IDS_SL900A_LOGGINGFORM_LIMITSCROSSING = 0x03,
  /* Trigger log on EXT1 input */
  TMR_GEN2_IDS_SL900A_LOGGINGFORM_IRQ1 = 0x05,
  /* Trigger log on EXT2 input */
  TMR_GEN2_IDS_SL900A_LOGGINGFORM_IRQ2 = 0x06,
  /* Trigger log on EXT1 and EXT2 input */
  TMR_GEN2_IDS_SL900A_LOGGINGFORM_IRQ1IRQ2 = 0x07,
}LoggingForm;

/** SL900A Data log memory-full behavior */
typedef enum DelayMode
{
  /* Start logging after delay time */
  TMR_GEN2_IDS_SL900A_DELAYMODE_TIMER = 0x00,
  /* Start logging on external input */
  TMR_GEN2_IDS_SL900A_DELAYMODE_EXTSWITCH = 0x01,
}DelayMode;

/** SL900A Logging memory-full behavior */
typedef enum StorageRule
{
  /* Stop logging when memory fills */
  TMR_GEN2_IDS_SL900A_STORAGERULE_NORMAL = 0x00,
  /*  Roll around (circular buffer) when memory fills */
  TMR_GEN2_IDS_SL900A_STORAGERULE_ROLLING = 0x01,
}StorageRule;

/** Sub-Class for BatterType, re-check or default */
typedef enum BatteryType
{
  /* Default */
  TMR_GEN2_IDS_SL900A_BATTERYTYPE_CHECK = 0x00,
  /* Application requested for the re-check of battery type */
  TMR_GEN2_IDS_SL900A_BATTERYTYPE_RECHECK = 0x01,
}BatteryType;

/** Sub-Class for Raw 9 or 21-bit Get Log State reply */
typedef struct LimitCounter
{
  /* Number of times selected sensor has gone beyond extreme lower limit */
  uint8_t extremeLower;
  /* Number of times selected sensor has gone beyond lower limit */
  uint8_t lower;
  /* Number of times selected sensor has gone beyond upper limit */
  uint8_t upper;
  /* Number of times selected sensor has gone beyond extreme upper limit */
  uint8_t extremeUpper;
}LimitCounter;

/** Sub-Class for  specifying 40 bit LogLimits Value */
typedef struct LogLimits
{
  /* Specifying the extreme lower limit */
  uint16_t extremeLower;
  /* Specifying the lower limit */
  uint16_t lower;
  /* Specifying the upper limit */
  uint16_t upper;
  /* Specifying the extreme upper limit */
  uint16_t extremeUpper;
}LogLimits;

/** Sub-class for SystemStatus reply object */
typedef struct SystemStatus
{
  /* Measurement Address Pointer */
  uint16_t MeasurementAddressPointer;
  /* Number of memory replacements */
  uint8_t NumMemReplacements;
  /* Number of measurements */
  uint16_t NumMeasurements;
  /* Active */
  bool Active;
}SystemStatus;

/** Sub-Class for Log Status Flags */
typedef struct StatusFlags
{
  /* Logging active */
  bool Active;
  /* Measurement area full */
  bool Full;
  /* Measurement overwritten */
  bool Overwritten;
  /* A/D error occurred */
  bool ADError;
  /* Low battery */
  bool LowBattery;
  /* Shelf life low error */
  bool ShelfLifeLow;
  /* Shelf life high error */
  bool ShelfLifeHigh;
  /* ShelfLifeExpired */
  bool ShelfLifeExpired;
}StatusFlags;
/** Sub-class for get log state value reply */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_LogState
{
  /* Number of excursions beyond set limits */
  LimitCounter limitCount;
  /* Logging system status */
  SystemStatus statStatus;
  /* Logging status flags */
  StatusFlags  statFlag;
}TMR_TagOp_GEN2_IDS_SL900A_LogState;

/** Sub-Class for Get Sensor Value reply */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_SensorReading
{
  /* Raw 16-bit response from GetSensorValue command */
  uint16_t reply;
  /* Raw sensor reply */
  uint16_t Raw;
  /* Did A/D conversion error occur? */
  bool ADError;
  /* 5-bit Range/Limit value */
  uint8_t RangeLimit;
  /* 10-bit Sensor value */
  uint16_t Value;
}TMR_TagOp_GEN2_IDS_SL900A_SensorReading;

/** Sub-Class for Get Battery Level Reply */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_BatteryLevelReading
{
  /* Raw 16-bit response from GetBatteryLevel command */
  uint16_t reply;
  /* Did A/D conversion error occur? */
  bool ADError;
  /* 1-bit Battery Type */
  uint8_t BatteryType;
  /* 10-bit Battery Level value */
  uint16_t Value;
}TMR_TagOp_GEN2_IDS_SL900A_BatteryLevelReading;

/** Sub-class for  SL900A  Log Mode Data */
typedef struct LogModeData
{
  /* Raw 8-bit response from Measurement Setup Data command */
  uint8_t Raw;
  /* Logging Format */
  LoggingForm Form;
  /* Log Memory-Full Behavior */
  StorageRule Storage;
  /* Enable log for EXT1 external sensor */
  bool Ext1Enable;
  /* Enable log for EXT2 external sensor */
  bool Ext2Enable;
  /* Enable log for temperature sensor */
  bool TempEnable;
  /* Enable log for battery sensor */
  bool BattEnable;
}LogModeData;

/** Sub-class for Calibration Data */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_CalibrationData
{
  /* Raw 56-bit Calibration data value */
  uint64_t raw;
  /* AD1 lower voltage reference - fine - DO NOT MODIFY */
  uint8_t Ad1;
  /* AD1 lower voltage reference - coarse */
  uint8_t Coarse1;
  /* AD2 lower voltage reference - fine - DO NOT MODIFY */
  uint8_t Ad2;
  /* AD2 lower voltage reference - coarse */
  uint8_t Coarse2;
  /* Switches the lower AD voltage reference to ground */
  bool GndSwitch;
  /* POR voltage level for 1.5V system */
  uint8_t Selp12;
  /* Main reference voltage calibration -- DO NOT MODIFY */
  uint8_t Adf;
  /* RTC oscillator calibration */
  uint8_t Df;
  /* Controlled battery supply for external sensor - the battery voltage is connected to the EXC pin */
  bool SwExtEn;
  /* POR voltage level for 3V system */
  uint8_t Selp22;
  /* Voltage level interrupt level for external sensor -- ratiometric */
  uint8_t Irlev;
  /* Main system clock oscillator calibration -- DO NOT MODIFY */
  uint8_t RingCal;
  /* Temperature conversion offset calibration -- DO NOT MODIFY */
  uint8_t  OffInt;
  /* Bandgap voltage temperature coefficient calibration -- DO NOT MODIFY */
  uint8_t Reftc;
  /* Excitate for resistive sensors without DC */
  bool ExcRes;
  /*  Reserved for Future Use */
  uint8_t RFU;
  /* Specifyes the field user can modify */
  CalibrationParameter type;
}TMR_TagOp_GEN2_IDS_SL900A_CalibrationData;

/** Sub-class to specify Sensor Front End Parameters */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_SfeParameters
{
  /*  Raw 16-bit SFE parameters value */
  uint16_t raw;
  /* External sensor 2 range */
  uint8_t Rang;
  /* External sensor 1 range */
  uint8_t Seti;
  /** 
   * External sensor 1 type
   * 00 -- linear resistive sensor
   * 01 -- high impedance input (voltage follower), bridge
   * 10 -- capacitive sensor with DC
   * 11 -- capacitive or resistive sensor without DC
   */
  uint8_t Ext1;
  /**
   * External sensor 2 type
   * 00 -- linear conductive sensor
   * 01 -- high impedance input (voltage follower), bridge
   */
  uint8_t Ext2;
  /* Use preset range */
  bool AutorangeDisable;
  /**
   * Sensor used in limit check
   * 00 - first selected sensor
   * 01 -- second selected sensor
   * 10 -- third selected sensor
   * 11 -- fourth selected sensor
   */
  uint8_t VerifySensorID;
  /* specifies the type of field user want to modufy */
  SfeParameter type;
}TMR_TagOp_GEN2_IDS_SL900A_SfeParameters;

/** Sub-Class to Combination Calibration Data / SFE Parameters object */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_CalSfe
{
  /* Calibration Data */
  TMR_TagOp_GEN2_IDS_SL900A_CalibrationData Cal;
  /* Sensor Front End Parameters */
  TMR_TagOp_GEN2_IDS_SL900A_SfeParameters Sfe;
}TMR_TagOp_GEN2_IDS_SL900A_CalSfe;

/**Sub-Class to spcify fifo source */
typedef enum FifoSource
{
  /* Data from SPI */
  TMR_GEN2_IDS_SL900A_FIFOSOURCE_SPI = 0x00,
  /* Data from RFID */
  TMR_GEN2_IDS_SL900A_FIFOSOURCE_RFID = 0x01,
}FifoSource;
/** Sub-Class for Get status fifo reply */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_FifoStatus
{
  /* Raw 8-bit response from AccessFifo Status command */
  uint8_t raw;
  /* FIFO Busy bit */
  bool fifoBusy;
  /* Data Ready bit */
  bool dataReady;
  /* No Data bit */
  bool noData;
  /* Data Source bit (SPI, RFID) */
  FifoSource source;
  /* Number of valid bytes in FIFO register */
  uint8_t numValidBytes;
}TMR_TagOp_GEN2_IDS_SL900A_FifoStatus;

/** Sub-Class for specifying SL900A application data */
typedef struct ApplicationData
{
  /* Raw 16-bit protocol value */
  uint16_t raw;
  /* Number of user bank memory words to reserve for non-logging purposes */
  uint16_t NumberOfWords;
  /* Broken word pointer */
  uint8_t BrokenWordPointer;
}ApplicationData;

/** Sub-Class for Gen2 Ids Sl900A coustom tag extension */
typedef struct TMR_TagOP_GEN2_IDS_SL900A
{
  /* SL900A sensor type values */
  Sensor sensortype;
  /* SL900A password access level values */
  PasswordLevel level;
  /* Data log format selection */
  LoggingForm dataLog;
  /* Data log memory-full behavior */
  DelayMode mode;
  /* Logging memory-full behavior */
  StorageRule rule;
}TMR_TagOP_GEN2_IDS_SL900A;

/** Sub- class for Ids Sl900A sensor */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_GetSensorValue
{
  TMR_TagOP_GEN2_IDS_SL900A sl900A;
  /* Custom Command Code */
  uint8_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* IDS SL900A Password */
  uint32_t Password;
}TMR_TagOp_GEN2_IDS_SL900A_GetSensorValue;

/** Sub-class for IDs SL900A get log state */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_GetLogState
{
  TMR_TagOP_GEN2_IDS_SL900A sl900A;
  /* Custom Command Code */
  uint8_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* IDS SL900A Password */
  uint32_t Password;
}TMR_TagOp_GEN2_IDS_SL900A_GetLogState;

/** Sub-Class for IDS SL900A set log mode */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_SetLogMode
{
  TMR_TagOP_GEN2_IDS_SL900A sl900A;
  /* Custom Command Code */
  uint8_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* IDS SL900A Password */
  uint32_t Password;
  /*Enable log for EXT1 external sensor */
  bool Ext1Enable;
  /* Enable log for EXT2 external sensor */
  bool Ext2Enable;
  /* Enable log for temperature sensor */
  bool TempEnable;
  /* Enable log for battery sensor */
  bool BattEnable;
  /* Time (seconds) between log readings */
  uint16_t LogInterval;
}TMR_TagOp_GEN2_IDS_SL900A_SetLogMode;

/** Sub-Class for Gen2 end log command*/
typedef struct TMR_TagOp_GEN2_IDS_SL900A_EndLog
{
  TMR_TagOP_GEN2_IDS_SL900A sl900A;
  /* Custom Command Code */
  uint8_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* IDS SL900A Password */
  uint32_t Password;
}TMR_TagOp_GEN2_IDS_SL900A_EndLog;

/** Sub-Class for DelayTime */
typedef struct Delay
{
  /* Raw 16-bit protocol value */
  uint16_t raw;
  /* Logging start mode */
  DelayMode Mode;
  /* Logging timer delay value (units of 512 seconds) */
  uint16_t Time;
  /* Trigger log on both timer and external interrupts */
  bool IrqTimerEnable;
}Delay;

/** Sub-Classs for ShelfLife Block 0 */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_ShelfLifeBlock0
{
  /* raw value of block0 */
  uint32_t raw;
  /* Tmax */
  uint8_t Tmax;
  /* Tmin */
  uint8_t Tmin;
  /* Tstd */
  uint8_t Tstd;
  /* Ea */
  uint8_t Ea;
}TMR_TagOp_GEN2_IDS_SL900A_ShelfLifeBlock0;

/** Sub-Class for ShelfLife Block 1 */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_ShelfLifeBlock1
{
  /* raw value for block 1 */
  uint32_t raw;
  /* SLinit */
  uint16_t SLinit;
  /* Tint */
  uint16_t Tint;
  /* SensorID */
  uint8_t sensorID;
  /* Enable negative shelf life */
  bool enableNegative;
  /* Shelf life algorithem enable */
  bool algorithmEnable;
  /* RFU bytes */
  uint8_t rfu;
}TMR_TagOp_GEN2_IDS_SL900A_ShelfLifeBlock1;

/** Sub-class for GetMeasurmentSetUp command */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_MeasurementSetupData
{
  /* Raw 16 byte response from Measurement Setup Data command */
  uint8_t Raw[16];
  /* LogLimits Data */
  LogLimits loglimit;
  /* Log Mode Data */
  LogModeData logModeData;
  /* Delay Data */
  Delay delyTime;
  /* Application Data */
  ApplicationData addData;
  /* Time (seconds) between log readings */
  uint16_t logInterval;
  /* Start Time */
  TMR_TimeStructure startTime;
}TMR_TagOp_GEN2_IDS_SL900A_MeasurementSetupData;

/** Sub-Class for GEN2 initialize command */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_Initialize
{
  TMR_TagOP_GEN2_IDS_SL900A sl900A;
  /* Logging memory configuration */
  ApplicationData applicationData;
  /* Custom Command Code */
  uint8_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* IDS SL900A Password */
  uint32_t Password;
  /* Delay time */
  Delay delayTime;
}TMR_TagOp_GEN2_IDS_SL900A_Initialize;

/** Sub-Class for specifying AccessFifo operation*/
typedef enum AccessFifoOperation
{
  /* Read from FIFO */
  TMR_GEN2_IDS_SL900A_ACCESSFIFO_READ = 0x80,
  /* Write to FIFO */
  TMR_GEN2_IDS_SL900A_ACCESSFIFO_WRITE = 0xA0,
  /* Get FIFO status */
  TMR_GEN2_IDS_SL900A_ACCESSFIFO_STATUS = 0xC0,
}AccessFifoOperation;

/** Sub-Class for Gen2 Ids AccessFifo command */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_AccessFifo
{
  TMR_TagOP_GEN2_IDS_SL900A sl900A;
  /* Custom Command Code */
  uint8_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* IDS SL900A Password */
  uint32_t Password;
  /* specify the operation do be done on fifo */
  AccessFifoOperation operation;
}TMR_TagOp_GEN2_IDS_SL900A_AccessFifo;

/** Sub-Class for Gen2 Ids accessFifo write command */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_AccessFifoWrite
{
  TMR_TagOp_GEN2_IDS_SL900A_AccessFifo write;
  /* Bytes to write to FIFO */
  TMR_uint8List *payLoad;
}TMR_TagOp_GEN2_IDS_SL900A_AccessFifoWrite;

/** Sub-Class for Gen2 Ids accessFifo read command */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_AccessFifoRead
{
  TMR_TagOp_GEN2_IDS_SL900A_AccessFifo read;
  /* Number of bytes to read from FIFO */
  uint8_t length;
}TMR_TagOp_GEN2_IDS_SL900A_AccessFifoRead;

/** Sub-Class for Gen2 Ids accessFifo status command */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_AccessFifoStatus
{
  TMR_TagOp_GEN2_IDS_SL900A_AccessFifo status;
}TMR_TagOp_GEN2_IDS_SL900A_AccessFifoStatus;

/** Sub-Class for Gen2 StartLog command */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_StartLog
{
  TMR_TagOP_GEN2_IDS_SL900A sl900A;
  /* Custom Command Code */
  uint8_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* IDS SL900A Password */
  uint32_t Password;
  /* Time to initialize log timestamp counter with */
  uint32_t startTime;  
}TMR_TagOp_GEN2_IDS_SL900A_StartLog;

/** Sub-Class for Gen2 IDS GetCalibrationData command */
typedef struct  TMR_TagOp_GEN2_IDS_SL900A_GetCalibrationData
{
  TMR_TagOP_GEN2_IDS_SL900A sl900A;
  /* Custom Command Code */
  uint8_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* IDS SL900A Password */
  uint32_t Password;
}TMR_TagOp_GEN2_IDS_SL900A_GetCalibrationData;

/** Sub-Class for Gen2 IDS SetCalibrationData command */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_SetCalibrationData
{
  TMR_TagOP_GEN2_IDS_SL900A sl900A;
  /* Custom Command Code */
  uint8_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* IDS SL900A Password */
  uint32_t Password;
  /* Calibration parameters */
  TMR_TagOp_GEN2_IDS_SL900A_CalibrationData cal; 
}TMR_TagOp_GEN2_IDS_SL900A_SetCalibrationData;

/** Sub-class for Gen2 Ids SetSfeParameters command */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_SetSfeParameters
{
  TMR_TagOP_GEN2_IDS_SL900A sl900A;
  /* Custom Command Code */
  uint8_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* IDS SL900A Password */
  uint32_t Password;
  /* Sfe Parameters */
  TMR_TagOp_GEN2_IDS_SL900A_SfeParameters *sfe;
}TMR_TagOp_GEN2_IDS_SL900A_SetSfeParameters;

/** Sub-class for Gen2 Ids GetMeasurementSetup command */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_GetMeasurementSetup
{
  TMR_TagOP_GEN2_IDS_SL900A sl900A;
  /* Custom Command Code */
  uint8_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* IDS SL900A Password */
  uint32_t Password;
}TMR_TagOp_GEN2_IDS_SL900A_GetMeasurementSetup;

/** Sub-class for Gen2 Ids GetBatteryLevel command */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_GetBatteryLevel
{
  TMR_TagOP_GEN2_IDS_SL900A sl900A;
  /* Custom Command Code */
  uint8_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* IDS SL900A Password */
  uint32_t Password;
  /* enum BatterType, re-check or default */
  BatteryType batteryType;
}TMR_TagOp_GEN2_IDS_SL900A_GetBatteryLevel;

/** Sub-Class for Gen2 Ids SetLogLimits Command */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_SetLogLimits
{
  TMR_TagOP_GEN2_IDS_SL900A sl900A;
  /* Custom Command Code */
  uint8_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* IDS SL900A Password */
  uint32_t Password;
  /* Specify the Log Limits */
  LogLimits limit;
}TMR_TagOp_GEN2_IDS_SL900A_SetLogLimits;

/** Sub-Class for Gen2 Ids SetShelfLife Command */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_SetShelfLife
{
  TMR_TagOP_GEN2_IDS_SL900A sl900A;
  /* Custom Command Code */
  uint8_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* IDS SL900A Password */
  uint32_t Password;
  /* Specify the ShelfLifeBlock0 */
  TMR_TagOp_GEN2_IDS_SL900A_ShelfLifeBlock0 *shelfLifeBlock0;
  /* Specify the ShelfLifeBlock1 */
  TMR_TagOp_GEN2_IDS_SL900A_ShelfLifeBlock1 *shelfLifeBlock1;
}TMR_TagOp_GEN2_IDS_SL900A_SetShelfLife;

/** Sub-Class for Gen2 Ids SetPassword Command */
typedef struct TMR_TagOp_GEN2_IDS_SL900A_SetPassword
{
  TMR_TagOP_GEN2_IDS_SL900A sl900A;
  /* Custom Command Code */
  uint8_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* IDS SL900A Password */
  uint32_t Password;
  /* New Passwordlevel */
  PasswordLevel NewPasswordLevel;
  /* New password */
  uint32_t NewPassword;
}TMR_TagOp_GEN2_IDS_SL900A_SetPassword;

/** Sub-class for Gen2 Ids parameters */
typedef struct TMR_TagOP_GEN2_IDS
{
  union
  {
    TMR_TagOp_GEN2_IDS_SL900A_GetSensorValue sensor;
    TMR_TagOp_GEN2_IDS_SL900A_GetLogState  getLog;
    TMR_TagOp_GEN2_IDS_SL900A_SetLogMode   setLogMode;
    TMR_TagOp_GEN2_IDS_SL900A_EndLog endLog;
    TMR_TagOp_GEN2_IDS_SL900A_Initialize initialize;
    TMR_TagOp_GEN2_IDS_SL900A_AccessFifoRead accessFifoRead;
    TMR_TagOp_GEN2_IDS_SL900A_AccessFifoWrite accessFifoWrite;
    TMR_TagOp_GEN2_IDS_SL900A_AccessFifoStatus accessFifoStatus;
    TMR_TagOp_GEN2_IDS_SL900A_StartLog startLog;
    TMR_TagOp_GEN2_IDS_SL900A_GetCalibrationData calibrationData;
    TMR_TagOp_GEN2_IDS_SL900A_SetCalibrationData setCalibration;
    TMR_TagOp_GEN2_IDS_SL900A_SetSfeParameters setSfeParameters;
    TMR_TagOp_GEN2_IDS_SL900A_GetMeasurementSetup measurementSetup;
    TMR_TagOp_GEN2_IDS_SL900A_GetBatteryLevel batteryLevel;
    TMR_TagOp_GEN2_IDS_SL900A_SetLogLimits setLogLimit;
    TMR_TagOp_GEN2_IDS_SL900A_SetShelfLife setShelfLife;
    TMR_TagOp_GEN2_IDS_SL900A_SetPassword setPassword;
    
  }u;
}TMR_TagOP_GEN2_IDS;

/** Sub-Class for Gen2 Fudan Read REG Command */
typedef struct TMR_TagOP_GEN2_FUDAN_ReadREG
{
  /* Custom Command Code */
  uint16_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* Reg address */
  uint16_t RegAddress;
}TMR_TagOP_GEN2_FUDAN_ReadREG;

/** Sub-Class for Gen2 Fudan Writed REG Command */
typedef struct TMR_TagOP_GEN2_FUDAN_WriteREG
{
  /* Custom Command Code */
  uint16_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* Reg address */
  uint16_t RegAddress;
  /* Reg Write Data */
  uint16_t RegWriteData;
}TMR_TagOP_GEN2_FUDAN_WriteREG;

/** Sub-Class for Gen2 Fudan Load REG Command */
typedef struct TMR_TagOP_GEN2_FUDAN_LoadREG
{
  /* Custom Command Code */
  uint16_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* Cmd Cfg */
  uint8_t CmdCfg;
}TMR_TagOP_GEN2_FUDAN_LoadREG;

/** Sub-Class for Gen2 Fudan Start/Stop Log Command */
typedef struct TMR_TagOP_GEN2_FUDAN_StartStopLOG
{
  /* Custom Command Code */
  uint16_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* Cmd Cfg */
  uint8_t CmdCfg;
  /* Flow Flag Reset Password */
  uint32_t FlagResetPassword;
}TMR_TagOP_GEN2_FUDAN_StartStopLOG;

/** Sub-Class for Gen2 Fudan Auth Command */
typedef struct TMR_TagOP_GEN2_FUDAN_Auth
{
  /* Custom Command Code */
  uint16_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* Cmd Cfg */
  uint8_t CmdCfg;
  /* Auth Password */
  uint32_t AuthPassword;
}TMR_TagOP_GEN2_FUDAN_Auth;

/** Sub-Class for Gen2 Fudan Read MEM Command */
typedef struct TMR_TagOP_GEN2_FUDAN_ReadMEM
{
  /* Custom Command Code */
  uint16_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* Start address to start reading at */
  uint16_t StartAddress;
  /* Length of data to be read */
  uint16_t len;
}TMR_TagOP_GEN2_FUDAN_ReadMEM;

/** Sub-Class for Gen2 Fudan Write MEM Command */
typedef struct TMR_TagOP_GEN2_FUDAN_WriteMEM
{
  /* Custom Command Code */
  uint16_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* Word address to start writing at */
  uint16_t StartAddress;
  /* Data to write */
  TMR_uint8List *data;
}TMR_TagOP_GEN2_FUDAN_WriteMEM;

/** Sub-Class for Gen2 Fudan State Check Command */
typedef struct TMR_TagOP_GEN2_FUDAN_StateCheck
{
  /* Custom Command Code */
  uint16_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* Data to write */
  TMR_uint8List *data;
}TMR_TagOP_GEN2_FUDAN_StateCheck;

/** Sub-Class for Gen2 Fudan Measure Command */
typedef struct TMR_TagOP_GEN2_FUDAN_Measure
{
  /* Custom Command Code */
  uint16_t CommandCode;
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* Cmd Cfg */
  uint8_t CmdCfg;
  /* Store Block Address */
  uint8_t StoreBlockAddress;
}TMR_TagOP_GEN2_FUDAN_Measure;

/** Sub-class for Gen2 Fudan parameters */
typedef struct TMR_TagOP_GEN2_FUDAN
{
  union
  {
    TMR_TagOP_GEN2_FUDAN_ReadREG ReadREG;
    TMR_TagOP_GEN2_FUDAN_LoadREG LoadREG;
    TMR_TagOP_GEN2_FUDAN_StartStopLOG StartStopLOG;
    TMR_TagOP_GEN2_FUDAN_Auth Auth;
    TMR_TagOP_GEN2_FUDAN_ReadMEM ReadMEM;
    TMR_TagOP_GEN2_FUDAN_WriteMEM WriteMEM;
    TMR_TagOP_GEN2_FUDAN_WriteREG WriteREG;
    TMR_TagOP_GEN2_FUDAN_StateCheck StateCheck;
    TMR_TagOP_GEN2_FUDAN_Measure Measure;
  }u;
}TMR_TagOP_GEN2_FUDAN;

/** Sub-Class for Gen2 Ilian tag select Command */
typedef struct TMR_TagOP_GEN2_ILIAN_Tag_Select_Command
{
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* Custom Command Code */
  uint16_t CommandCode;
}TMR_TagOP_GEN2_ILIAN_Tag_Select_Command;

/** Sub-class for Gen2 Ilian parameters */
typedef struct TMR_TagOP_GEN2_ILIAN
{
  union
  {
    TMR_TagOP_GEN2_ILIAN_Tag_Select_Command IlianTagSelect;
  }u;
}TMR_TagOP_GEN2_ILIAN;

/** Sub-Class for Gen2 EMMICRO EM4325 get sensor data. */
typedef struct TMR_TagOP_GEN2_EM4325_Get_Sensor_Data
{
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* Custom Command Code */
  uint16_t CommandCode;
  /* bitsToSet holds the byte value when sendUid or sendNewSample or both is enabled*/
  uint8_t bitsToSet;
}TMR_TagOP_GEN2_EM4325_Get_Sensor_Data;

/** Sub-Class for Gen2 EMMICRO EM4325 RESET ALARMS. */
typedef struct TMR_TagOP_GEN2_EM4325_Reset_Alarms
{
  /* Gen2 access password */
  uint32_t AccessPassword;
  /* Custom Command Code */
  uint16_t CommandCode;
  /* Fill value */
  uint8_t fillValue;
}TMR_TagOP_GEN2_EM4325_Reset_Alarms;

/** Sub-class for Gen2 EMMICRO parameters */
typedef struct TMR_TagOP_GEN2_EM4325
{
  union
  {
    TMR_TagOP_GEN2_EM4325_Get_Sensor_Data getSensorData;
    TMR_TagOP_GEN2_EM4325_Reset_Alarms resetAlarms;
  }u;
}TMR_TagOP_GEN2_EM4325;

/** Sub-class for Gen2 EMMICRO parameters */
typedef struct TMR_TagOP_GEN2_EMMICRO
{
  union
  {
    TMR_TagOP_GEN2_EM4325 em4325;
  }u;
}TMR_TagOP_GEN2_EMMICRO;

/** Sub-class for Gen2 custom tagops */
typedef struct TMR_TagOp_GEN2_Custom
{
  TMR_SR_GEN2_SiliconType chipType;
  union
  {
    TMR_TagOp_GEN2_Alien alien;
    TMR_TagOp_GEN2_NXP nxp;
    TMR_TagOp_GEN2_Impinj impinj;
    TMR_TagOP_GEN2_IDS ids;
    TMR_TagOp_GEN2_Denatran IavDenatran;
    TMR_TagOP_GEN2_FUDAN fdn;
    TMR_TagOP_GEN2_ILIAN ilian;
    TMR_TagOP_GEN2_EMMICRO emmicro;

  } u;
}TMR_TagOp_GEN2_Custom;

/** Sub-class for ISO180006B tagops */
typedef struct TMR_TagOp_ISO180006B
{
  union
  {
    TMR_TagOp_ISO180006B_Lock lock;
    TMR_TagOp_ISO180006B_WriteData writeData;
    TMR_TagOp_ISO180006B_ReadData readData;
  } u;
}TMR_TagOp_ISO180006B;

/** Sub-class for Gen2 standard tagops */
typedef struct TMR_TagOp_GEN2
{
  union
  {
    TMR_TagOp_GEN2_WriteTag writeTag;
    TMR_TagOp_GEN2_WriteData writeData;
    TMR_TagOp_GEN2_ReadData readData;
    TMR_TagOp_GEN2_SecureReadData secureReadData;
    TMR_TagOp_GEN2_Lock lock;
    TMR_TagOp_GEN2_Kill kill;
    TMR_TagOp_GEN2_BlockWrite blockWrite;
    TMR_TagOp_GEN2_BlockPermaLock blockPermaLock;
    TMR_TagOp_GEN2_BlockErase blockErase;
    TMR_TagOp_GEN2_Custom custom;
  } u;
}TMR_TagOp_GEN2;
#endif /* TMR_ENABLE_UHF */

#ifdef TMR_ENABLE_HF_LF

/** Types of memory operations */
typedef enum TMR_Memory_Type
{
  TMR_TAGOP_TAG_MEMORY                 = 0x21, // Both read and write are supported.
  TMR_TAGOP_TAG_INFO                   = 0x22, // Only read is supported.
  TMR_TAGOP_PROTECTION_SECURITY_STATUS = 0x23, // Only read is supported.
  TMR_TAGOP_SECURE_ID                  = 0x24, // Only read is supported.
  TMR_TAGOP_EXT_TAG_MEMORY             = 0x25, // API enum used for extended tag Operations.
}TMR_Memory_Type;

#if TMR_ENABLE_DESFIRE_TAGOPS
/** DesFire commands for read/write operations */
typedef enum TMR_DesFire_Cmd
{
  TMR_TAGOP_DESFIRE_CMD_NONE          = 0x00,
  TMR_TAGOP_DESFIRE_CMD_READ          = 0x01,
  TMR_TAGOP_DESFIRE_CMD_WRITE         = 0x01,
  TMR_TAGOP_DESFIRE_CMD_APP_ID        = 0x02,
  TMR_TAGOP_DESFIRE_CMD_FILE_ID       = 0x03,
  TMR_TAGOP_DESFIRE_CMD_AUTHENTICATE  = 0x04,
  TMR_TAGOP_DESFIRE_CMD_DELETE_APP    = 0x05,
  TMR_TAGOP_DESFIRE_CMD_FORMAT        = 0x06,
  TMR_TAGOP_DESFIRE_CMD_CHANGE_KEY    = 0x07,
  TMR_TAGOP_DESFIRE_CMD_KEY_SETTINGS  = 0x08,
  TMR_TAGOP_DESFIRE_CMD_FILE_SETTINGS = 0x09,
  TMR_TAGOP_DESFIRE_CMD_DELETE_FILE   = 0x0A,
  TMR_TAGOP_DESFIRE_CMD_KEY_VERSION   = 0x0B,

  TMR_TAGOP_DESFIRE_CMD_MAX    = TMR_TAGOP_DESFIRE_CMD_KEY_VERSION + 1,
}TMR_DesFire_Cmd;

/** DesFire File-types for read/write operations */
typedef enum TMR_DesFire_FileType
{
  TMR_TAGOP_DESFIRE_FILE_TYPE_STDDATA        = 0x00,
  TMR_TAGOP_DESFIRE_FILE_TYPE_BACKUPDATA     = 0x01,
  TMR_TAGOP_DESFIRE_FILE_TYPE_VALUE          = 0x02,
  TMR_TAGOP_DESFIRE_FILE_TYPE_LINEAR_RECORD  = 0x03,
  TMR_TAGOP_DESFIRE_FILE_TYPE_CYCLIC_RECORD  = 0x04

}TMR_DesFire_FileType;
#endif /* TMR_ENABLE_DESFIRE_TAGOPS */

/** Read memory structure */
typedef struct TMR_TagOp_ReadMemory
{
  /** Types of memory operations */
  TMR_Memory_Type memType;
  /** Address to start reading at */
  uint32_t address;
  /** Number of memory units to read */
  uint8_t len;
} TMR_TagOp_ReadMemory;

/** Write memory structure */
typedef struct TMR_TagOp_WriteMemory
{
  /** Types of memory operations */
  TMR_Memory_Type memType;
  /** Address to start writing at */
  uint32_t address;
  /** Data to write */
  TMR_uint8List data;
} TMR_TagOp_WriteMemory;

typedef struct TMR_TagOp_PassThrough
{
  /** Timeout in msec */
  uint32_t timeout;
  /** Configuration flags */
  uint32_t configFlags;
  /** Command buffer */
  TMR_uint8List buffer; 
} TMR_TagOp_PassThrough;

#define MAX_PASSWORD_LEN  0x18

#if TMR_ENABLE_DESFIRE_TAGOPS
#define APPID_LEN         0x03

/** Read memory structure */
typedef struct TMR_TagOp_DesFire_ReadData
{
  /** Application ID to read from */
  uint32_t appId;
  /** Sub option to read */
  uint8_t subCmd;
  /** File ID to read from*/
  uint8_t fileId;
  /** File Type to be read */
  uint8_t fileType;
  /** Key Number */
  uint8_t keyNo;
} TMR_TagOp_DesFire_ReadData;

/** Write memory structure */
typedef struct TMR_TagOp_DesFire_WriteData
{
  /** Application ID to write into */
  uint32_t appId;
  /** Size of a file */
  uint16_t fileSize;
  /** Access rights of a file */
  uint16_t accessRights;
  /** Sub cmd to write */
  uint8_t subCmd;
  /** File ID to write into*/
  uint8_t fileId;
  /** File Type to be written */
  uint8_t fileType;
  /** Key settings */
  uint8_t keySettings;
  /** Number Of Keys */
  uint8_t numKeys;
  /** Key number */
  uint8_t keyNo;
  /** New key number */
  uint8_t newKeyNo;
} TMR_TagOp_DesFire_WriteData;

/** Sub-class for DesFire standard tagops */
typedef struct TMR_TagOp_DesFire
{
  union
  {
    TMR_TagOp_DesFire_ReadData readData;
    TMR_TagOp_DesFire_WriteData writeData;
  } u;
}TMR_TagOp_DesFire;
#endif /* TMR_ENABLE_DESFIRE_TAGOPS */

#if TMR_ENABLE_UL_NTAG_TAGOPS

/** DesFire commands for read/write operations */
typedef enum TMR_UL_Ntag_Cmd
{
  TMR_TAGOP_UL_NTAG_CMD_NONE      = 0x00,
  TMR_TAGOP_UL_NTAG_CMD_FAST_READ = 0x01,

  TMR_TAGOP_UL_NTAG_CMD_MAX       = TMR_TAGOP_UL_NTAG_CMD_FAST_READ + 1,
}TMR_UL_Ntag_Cmd;

/** Read memory structure */
typedef struct TMR_TagOp_UL_Ntag_ReadData
{
  /** Sub option to read */
  uint8_t subCmd;
} TMR_TagOp_UL_Ntag_ReadData;

/** Sub-class for Ultralight Ntag standard tagops */
typedef struct TMR_TagOp_UL_Ntag
{
  union
  {
    TMR_TagOp_UL_Ntag_ReadData readData;
  } u;
}TMR_TagOp_UL_Ntag;
#endif /* TMR_ENABLE_UL_NTAG_TAGOPS */

typedef struct TMR_ExtTagOp
{
  uint64_t tagType;
  TMR_uint8List *accessPassword;

  /* Standard tag operations */
  TMR_TagOp_ReadMemory readMem;
  TMR_TagOp_WriteMemory writeMem;

#if TMR_ENABLE_EXTENDED_TAGOPS
  /* Extended tag operations */
  union
  {
#if TMR_ENABLE_DESFIRE_TAGOPS
    TMR_TagOp_DesFire desfire;
#endif /* TMR_ENABLE_DESFIRE_TAGOPS */

#if TMR_ENABLE_UL_NTAG_TAGOPS
    TMR_TagOp_UL_Ntag ulNtag;
#endif /* TMR_ENABLE_UL_NTAG_TAGOPS */
  } u;
#endif /* TMR_ENABLE_EXTENDED_TAGOPS */
}TMR_ExtTagOp;
#endif /* TMR_ENABLE_HF_LF */

/** Tag operation data structure */
struct TMR_TagOp
{
  TMR_TagOpType type;
  union
  {
#ifdef TMR_ENABLE_UHF
    TMR_TagOp_GEN2 gen2;
    TMR_TagOp_ISO180006B iso180006b;
    TMR_TagOp_List list;
#endif /* TMR_ENABLE_UHF */
#ifdef TMR_ENABLE_HF_LF
    /* HF protocol supported tag operations */
    TMR_ExtTagOp extTagOp;
    TMR_TagOp_PassThrough passThrough;
#endif /* TMR_ENABLE_HF_LF */
  } u;
};
#ifdef TMR_ENABLE_UHF
TMR_Status TMR_TagOp_init_GEN2_WriteTag(TMR_TagOp *tagop, TMR_TagData* epc);
TMR_Status TMR_TagOp_init_GEN2_ReadData(TMR_TagOp *tagop, TMR_GEN2_Bank bank,
                                        uint32_t wordAddress, uint8_t len);
TMR_Status TMR_TagOp_init_GEN2_SecureReadData(TMR_TagOp *tagop, TMR_GEN2_Bank bank, uint32_t wordAddress,
                                               uint8_t len, uint8_t type, uint8_t passwordType);
TMR_Status TMR_TagOp_init_GEN2_SecurePassWord(TMR_TagOp *tagop, uint8_t passwordType, uint32_t gen2PassWord,
                                              uint8_t addressLength, uint8_t addressOffset, uint16_t flashOffset);
TMR_Status TMR_TagOp_init_GEN2_WriteData(TMR_TagOp *tagop, TMR_GEN2_Bank bank,
                                         uint32_t wordAddress,
                                         TMR_uint16List *data);
TMR_Status TMR_TagOp_init_GEN2_Lock(TMR_TagOp *tagop, uint16_t mask,
                                    uint16_t action,  TMR_GEN2_Password accessPassword);
TMR_Status TMR_TagOp_init_GEN2_Kill(TMR_TagOp *tagop,
                                    TMR_GEN2_Password killPassword);
TMR_Status TMR_TagOp_init_GEN2_BlockWrite(TMR_TagOp *tagop, TMR_GEN2_Bank bank, uint32_t wordPtr, TMR_uint16List *data);
TMR_Status TMR_TagOp_init_GEN2_BlockPermaLock(TMR_TagOp *tagop, uint8_t readLock, TMR_GEN2_Bank bank, uint32_t blockPtr, TMR_uint16List* mask);
TMR_Status TMR_TagOp_init_GEN2_BlockErase(TMR_TagOp *tagop, TMR_GEN2_Bank bank, uint32_t wordPtr, uint8_t wordCount);

TMR_Status TMR_TagOp_init_ISO180006B_ReadData(TMR_TagOp *tagop, uint8_t byteAddress, uint8_t len);
TMR_Status TMR_TagOp_init_ISO180006B_WriteData(TMR_TagOp *tagop, uint8_t byteAddress, TMR_uint8List *data);
TMR_Status TMR_TagOp_init_ISO180006B_Lock(TMR_TagOp *tagop, uint8_t address);
TMR_Status TMR_TagOp_init_GEN2_Alien_Higgs2_PartialLoadImage(TMR_TagOp *tagop, TMR_GEN2_Password killPassword,
                                                  TMR_GEN2_Password accessPassword, TMR_TagData *epc);
TMR_Status TMR_TagOp_init_GEN2_Alien_Higgs2_FullLoadImage(TMR_TagOp *tagop, TMR_GEN2_Password killPassword,
                                    TMR_GEN2_Password accessPassword, uint16_t lockBits, 
                                    uint16_t pcWord, TMR_TagData *epc);
TMR_Status TMR_TagOp_init_GEN2_Alien_Higgs3_FastLoadImage(TMR_TagOp *tagop, TMR_GEN2_Password currentAccessPassword,
                                    TMR_GEN2_Password accessPassword, TMR_GEN2_Password killPassword, 
                                    uint16_t pcWord, TMR_TagData *epc);
TMR_Status TMR_TagOp_init_GEN2_Alien_Higgs3_LoadImage(TMR_TagOp *tagop, TMR_GEN2_Password currentAccessPassword,
                                    TMR_GEN2_Password accessPassword, TMR_GEN2_Password killPassword, 
                                    uint16_t pcWord, TMR_uint8List *epcAndUserData);
TMR_Status TMR_TagOp_init_GEN2_Alien_Higgs3_BlockReadLock(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, uint8_t lockBits);

TMR_Status TMR_TagOp_init_GEN2_NXP_G2I_SetReadProtect(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword);
TMR_Status TMR_TagOp_init_GEN2_NXP_G2X_SetReadProtect(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword);

TMR_Status TMR_TagOp_init_GEN2_NXP_G2I_ResetReadProtect(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword);
TMR_Status TMR_TagOp_init_GEN2_NXP_G2X_ResetReadProtect(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword);

TMR_Status TMR_TagOp_init_GEN2_NXP_G2I_ChangeEAS(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, bool resetEAS);
TMR_Status TMR_TagOp_init_GEN2_NXP_G2X_ChangeEAS(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, bool resetEAS);

TMR_Status TMR_TagOp_init_GEN2_NXP_G2I_EASAlarm(TMR_TagOp *tagop, TMR_GEN2_DivideRatio dr, TMR_GEN2_TagEncoding m, TMR_GEN2_TrExt trExt);
TMR_Status TMR_TagOp_init_GEN2_NXP_G2X_EASAlarm(TMR_TagOp *tagop, TMR_GEN2_DivideRatio dr, TMR_GEN2_TagEncoding m, TMR_GEN2_TrExt trExt);

TMR_Status TMR_TagOp_init_GEN2_NXP_G2I_Calibrate(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword);
TMR_Status TMR_TagOp_init_GEN2_NXP_G2X_Calibrate(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword);

TMR_Status TMR_TagOp_init_GEN2_NXP_G2I_ChangeConfig(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, TMR_NXP_ConfigWord configWord);
TMR_Status TMR_TagOp_init_GEN2_NXP_G2X_ChangeConfig(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, TMR_NXP_ConfigWord configWord);
TMR_Status TMR_TagOp_init_GEN2_NXP_UCODE7_ChangeConfig(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, TMR_NXP_UCODE7_ConfigWord configWord);
TMR_Status TMR_TagOp_init_GEN2_NXP_AES_Untraceable(TMR_TagOp *tagop,TMR_GEN2_UNTRACEABLE_Epc epc, int epclen , TMR_GEN2_UNTRACEABLE_Tid tid,
				    TMR_GEN2_UNTRACEABLE_UserMemory user, TMR_GEN2_UNTRACEABLE_Range range, TMR_TagOp_GEN2_NXP_Untraceable *auth);
TMR_Status TMR_TagOp_init_GEN2_NXP_AES_Authenticate(TMR_TagOp *tagop, TMR_TagOp_GEN2_NXP_Authenticate *authenticate);
TMR_Status TMR_TagOp_init_GEN2_NXP_AES_ReadBuffer(TMR_TagOp *tagop, uint16_t wordPointer, uint16_t bitCount, TMR_TagOp_GEN2_NXP_Readbuffer *readbuffer);
TMR_Status TMR_TagOp_init_GEN2_NXP_AES_Tam1authentication(TMR_TagOp_GEN2_NXP_Tam1Authentication *auth, TMR_NXP_KeyId keyid, TMR_uint8List *key, 
					bool sendRawData );
TMR_Status TMR_TagOp_init_GEN2_NXP_AES_Tam2authentication(TMR_TagOp_GEN2_NXP_Tam2Authentication *auth, TMR_NXP_KeyId keyid, TMR_uint8List *key, 
					TMR_NXP_Profile profile, uint16_t Offset,uint8_t blockCount,int protMode, bool sendRawData);

TMR_Status TMR_TagOp_init_GEN2_Impinj_Monza4_QTReadWrite(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                             TMR_Monza4_ControlByte controlByte, TMR_Monza4_Payload payload);

TMR_Status TMR_TagOp_init_GEN2_Impinj_Monza6_MarginRead(TMR_TagOp *tagop, TMR_GEN2_Bank bank,
                                             uint32_t bitAddress, uint16_t maskBitLength, uint8_t *mask);
TMR_Status TMR_TagOp_init_GEN2_Denatran_IAV_Activate_Secure_Mode(TMR_TagOp *tagop, uint8_t rfuByte);
TMR_Status TMR_TagOp_init_GEN2_Denatran_IAV_Authenticate_OBU(TMR_TagOp *tagop, uint8_t rfuByte);
TMR_Status TMR_TagOp_init_GEN2_Denatran_IAV_Activate_Siniav_Mode(TMR_TagOp *tagop, uint8_t rfuByte, TMR_uint8List *data);
TMR_Status TMR_TagOp_init_GEN2_Denatran_IAV_OBU_Auth_ID(TMR_TagOp *tagop, uint8_t rfuByte);
TMR_Status TMR_TagOp_init_GEN2_Denatran_IAV_OBU_Auth_Full_Pass1(TMR_TagOp *tagop, uint8_t rfuByte);
TMR_Status TMR_TagOp_init_GEN2_Denatran_IAV_OBU_Auth_Full_Pass2(TMR_TagOp *tagop, uint8_t rfuByte);
TMR_Status TMR_TagOp_init_GEN2_Denatran_IAV_OBU_ReadFromMemMap(TMR_TagOp *tagop, uint8_t rfuByte, uint16_t wordAddress);
TMR_Status TMR_TagOp_init_GEN2_Denatran_IAV_OBU_WriteToMemMap(TMR_TagOp *tagop, uint8_t rfuByte, uint16_t wordAddress, uint16_t word, uint8_t* tagID, uint8_t* data);
TMR_Status TMR_TagOp_init_GEN2_Denatran_IAV_GetTokenId(TMR_TagOp *tagop);
TMR_Status TMR_TagOp_init_GEN2_Denatran_IAV_OBU_Auth_Full_Pass(TMR_TagOp *tagop, uint8_t payload);
TMR_Status TMR_TagOp_init_GEN2_Denatran_IAV_ReadSec(TMR_TagOp *tagop, uint8_t payload, uint16_t wordAddress);
TMR_Status TMR_TagOp_init_GEN2_Denatran_IAV_WriteSec(TMR_TagOp *tagop, uint8_t payload, uint8_t* data, uint8_t* credentials);
TMR_Status TMR_TagOp_init_GEN2_Denatran_IAV_G0_PA_OBU_Auth(TMR_TagOp *tagop, uint8_t rfuByte);

TMR_Status 
TMR_init_GEN2_NXP_G2I_ConfigWord(TMR_NXP_ConfigWord *configWord);

TMR_Status
TMR_init_GEN2_NXP_UCODE7_ConfigWord(TMR_NXP_UCODE7_ConfigWord *configWord);

TMR_Status
TMR_init_GEN2_Impinj_Monza4_ControlByte(TMR_Monza4_ControlByte *controlByte);

TMR_Status
TMR_init_GEN2_Impinj_Monza4_Payload(TMR_Monza4_Payload *payload);

TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_GetSensorValue(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                              PasswordLevel level, uint32_t password, Sensor type);
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_GetCalibrationData(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                                  PasswordLevel leveli, uint32_t password);
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_SetLogLimit(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, 
                                           PasswordLevel level, uint32_t password, LogLimits *logLimits);
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_SetCalibrationData(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, PasswordLevel level, 
                                                  uint32_t password, TMR_TagOp_GEN2_IDS_SL900A_CalibrationData *data);
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_SetSfeParameters(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, PasswordLevel level, 
                                                uint32_t password, TMR_TagOp_GEN2_IDS_SL900A_SfeParameters *data);
TMR_Status
TMR_init_GEN2_IDS_SL900A_SensorReading(TMR_uint8List *reply, TMR_TagOp_GEN2_IDS_SL900A_SensorReading *sensor);

TMR_Status
TMR_init_GEN2_IDS_SL900A_BatteryLevelReading(TMR_uint8List *reply, TMR_TagOp_GEN2_IDS_SL900A_BatteryLevelReading *battery);

TMR_Status
TMR_init_GEN2_IDS_SL900A_MeasurementSetupData(TMR_uint8List *reply, TMR_TagOp_GEN2_IDS_SL900A_MeasurementSetupData *measurment);

TMR_Status
TMR_init_GEN2_IDS_SL900A_CalSfe(TMR_uint8List *reply, TMR_TagOp_GEN2_IDS_SL900A_CalSfe *calSfe);

TMR_Status
TMR_init_GEN2_IDS_SL900A_CalibrationData(uint8_t byte[7], TMR_TagOp_GEN2_IDS_SL900A_CalibrationData *cal);

TMR_Status
TMR_update_GEN2_IDS_SL900A_CalibrationData(TMR_TagOp_GEN2_IDS_SL900A_CalibrationData *cal);

TMR_Status
TMR_update_GEN2_IDS_SL900A_SfeParameters(TMR_TagOp_GEN2_IDS_SL900A_SfeParameters *sfe);

TMR_Status
TMR_init_GEN2_IDS_SL900A_SfeParameters(uint8_t byte[2], TMR_TagOp_GEN2_IDS_SL900A_SfeParameters *sfe);

TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_GetLogState(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                           PasswordLevel level, uint32_t password);

TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_SetLogMode(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, PasswordLevel level,
                                          uint32_t password, LoggingForm form, StorageRule rule, bool Ext1Enable,
                                          bool Ext2Enable, bool TempEnable, bool BattEnable, uint16_t LogInterval);

TMR_Status
TMR_init_GEN2_IDS_SL900A_LogState(TMR_uint8List *reply, TMR_TagOp_GEN2_IDS_SL900A_LogState *log);

TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_Initialize(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, PasswordLevel level,
                                          uint32_t password, uint8_t delayMode, uint16_t delayTime, bool timeEnable,
                                          uint16_t numberOfWords, uint8_t BrokenWordPointer);

TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_EndLog(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                      PasswordLevel level, uint32_t password);
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_AccessFifoStatus(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                                PasswordLevel level ,uint32_t password);
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_AccessFifoRead(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, PasswordLevel level,
                                              uint32_t password,  uint8_t readLength);
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_AccessFifoWrite(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                               PasswordLevel level, uint32_t password, TMR_uint8List *payLoad);
TMR_Status
TMR_init_GEN2_IDS_SL900A_FifoStatus(TMR_uint8List *reply, TMR_TagOp_GEN2_IDS_SL900A_FifoStatus *status);

TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_StartLog(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                        PasswordLevel level, uint32_t password, TMR_TimeStructure *timestamp);
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_GetMeasurementSetup(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                                   PasswordLevel level, uint32_t password);

TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_SetPassword(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, PasswordLevel level,
                                           uint32_t password, PasswordLevel newPasswordLevel, uint32_t newPassword);
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_GetBatteryLevel(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword,
                                               PasswordLevel level, uint32_t password, BatteryType type);
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_ShelfLifeBlock0(TMR_TagOp_GEN2_IDS_SL900A_ShelfLifeBlock0 *block0,
                                               uint8_t tmax, uint8_t Tmin, uint8_t tstd, uint8_t Ea);
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_ShelfLifeBlock1(TMR_TagOp_GEN2_IDS_SL900A_ShelfLifeBlock1 *block1,
                                               uint16_t slinit, uint16_t tint, uint8_t sensorid,
                                               bool negative, bool algorithm);
TMR_Status
TMR_TagOp_init_GEN2_IDS_SL900A_SetShelfLife(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, PasswordLevel level,
                                            uint32_t password, TMR_TagOp_GEN2_IDS_SL900A_ShelfLifeBlock0 *block0,
                                            TMR_TagOp_GEN2_IDS_SL900A_ShelfLifeBlock1 *block1);
TMR_Status
TMR_TagOp_init_GEN2_fdn_ReadREG(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, uint16_t wordAddress);
TMR_Status
TMR_TagOp_init_GEN2_fdn_WriteREG(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, uint16_t RegAddress, uint16_t RegWriteData);
TMR_Status
TMR_TagOp_init_GEN2_fdn_LoadREG(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, uint8_t CmdCfg);
TMR_Status
TMR_TagOp_init_GEN2_fdn_StartStopLOG(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, uint8_t CmdCfg, uint32_t FlagResetPassword);
TMR_Status
TMR_TagOp_init_GEN2_fdn_Auth(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, uint8_t CmdCfg, uint32_t AuthPassword);
TMR_Status
TMR_TagOp_init_GEN2_fdn_ReadMEM(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, uint16_t startAddress, uint16_t len);
TMR_Status
TMR_TagOp_init_GEN2_fdn_WriteMEM(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, uint16_t startAddress, TMR_uint8List *data);
TMR_Status
TMR_TagOp_init_GEN2_fdn_StateCheck(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, TMR_uint8List *data);
TMR_Status
TMR_TagOp_init_GEN2_fdn_Measure(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, uint8_t CmdCfg, uint8_t StoreBlockAddress);
 TMR_Status
 TMR_TagOp_init_GEN2_ILN_TagSelectCommand(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword);

TMR_Status
TMR_TagOp_init_GEN2_ILN_TagSelectCommand(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword);
TMR_Status
TMR_TagOp_init_GEN2_EM4325_GetSensorData(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword, bool sendUID, bool sendNewSample);
TMR_Status
TMR_TagOp_init_GEN2_EM4325_ResetAlarms(TMR_TagOp *tagop, TMR_GEN2_Password accessPassword);
TMR_Status
TMR_GEN2_init_BapParams(TMR_GEN2_Bap *bapVal, int32_t powerUpDelayUs, int32_t freqHopOfftimeUs);
#endif /* TMR_ENABLE_UHF */

#ifdef TMR_ENABLE_HF_LF
TMR_Status
TMR_TagOp_init_ReadMemory(TMR_TagOp *tagop, TMR_Memory_Type memType, uint32_t address, uint8_t len);
TMR_Status
TMR_TagOp_init_WriteMemory(TMR_TagOp *tagop, TMR_Memory_Type memType, uint32_t address,
                             TMR_uint8List *data);
TMR_Status
TMR_TagOp_init_PassThrough(TMR_TagOp *tagop, uint32_t timeout,
                             uint32_t configFlags, TMR_uint8List *buffer);
TMR_Status
TMR_set_accessPassword(TMR_TagOp *tagop, TMR_uint8List *accessPW);
#endif /* TMR_ENABLE_HF_LF */

#ifdef __cplusplus
}
#endif

#endif /* _TMR_TAGOP_H */
