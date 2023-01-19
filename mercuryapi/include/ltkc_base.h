
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
 * Version is four each 8-bit numbers:
 * major, minor, maintenance, progress number
 */
#define LTKC_VERSION        0x01000005
#define LTKC_VERSION_STR    "1.0.0.5"

enum LLRP_ResultCode;
struct LLRP_SErrorDetails;
struct LLRP_SVendorDescriptor;
struct LLRP_SNamespaceDescriptor;
struct LLRP_STypeDescriptor;
enum LLRP_EFieldType;
enum LLRP_EFieldFormat;
struct LLRP_SFieldDescriptor;
struct LLRP_SEnumTableEntry;
struct LLRP_STypeRegistry;
struct LLRP_SElement;
struct LLRP_SMessage;
struct LLRP_SParameter;
struct LLRP_SDecoder;
struct LLRP_SDecoderOps;
struct LLRP_SDecoderStream;
struct LLRP_SDecoderStreamOps;
struct LLRP_SEncoder;
struct LLRP_SEncoderOps;
struct LLRP_SEncoderStream;
struct LLRP_SEncoderStreamOps;


typedef enum LLRP_ResultCode            LLRP_tResultCode;
typedef struct LLRP_SErrorDetails       LLRP_tSErrorDetails;
typedef struct LLRP_SVendorDescriptor   LLRP_tSVendorDescriptor;
typedef struct LLRP_SNamespaceDescriptor LLRP_tSNamespaceDescriptor;
typedef struct LLRP_STypeDescriptor     LLRP_tSTypeDescriptor;
typedef enum LLRP_EFieldType            LLRP_tEFieldType;
typedef enum LLRP_EFieldFormat          LLRP_tEFieldFormat;
typedef struct LLRP_SFieldDescriptor    LLRP_tSFieldDescriptor;
typedef struct LLRP_SEnumTableEntry     LLRP_tSEnumTableEntry;
typedef struct LLRP_STypeRegistry       LLRP_tSTypeRegistry;
typedef struct LLRP_SElement            LLRP_tSElement;
typedef struct LLRP_SMessage            LLRP_tSMessage;
typedef struct LLRP_SParameter          LLRP_tSParameter;
typedef struct LLRP_SDecoder            LLRP_tSDecoder;
typedef struct LLRP_SDecoderOps         LLRP_tSDecoderOps;
typedef struct LLRP_SDecoderStream      LLRP_tSDecoderStream;
typedef struct LLRP_SDecoderStreamOps   LLRP_tSDecoderStreamOps;
typedef struct LLRP_SEncoder            LLRP_tSEncoder;
typedef struct LLRP_SEncoderOps         LLRP_tSEncoderOps;
typedef struct LLRP_SEncoderStream      LLRP_tSEncoderStream;
typedef struct LLRP_SEncoderStreamOps   LLRP_tSEncoderStreamOps;


typedef struct
{
    llrp_u16_t                  nValue;
    llrp_u8_t *                 pValue;
} llrp_u8v_t;

extern llrp_u8v_t   LLRP_u8v_construct(llrp_u16_t nValue);
extern void         LLRP_u8v_clear(llrp_u8v_t *pDst);
extern void         LLRP_u8v_set(llrp_u8v_t *pDst, llrp_u8v_t  Value);
extern llrp_u8v_t   LLRP_u8v_copy(llrp_u8v_t Value);

typedef struct
{
    llrp_u16_t                  nValue;
    llrp_s8_t *                 pValue;
} llrp_s8v_t;

extern llrp_s8v_t   LLRP_s8v_construct(llrp_u16_t nValue);
extern void         LLRP_s8v_clear(llrp_s8v_t *pDst);
extern void         LLRP_s8v_set(llrp_s8v_t *pDst, llrp_s8v_t  Value);
extern llrp_s8v_t   LLRP_s8v_copy(llrp_s8v_t Value);

typedef struct
{
    llrp_u16_t                  nValue;
    llrp_u16_t *                pValue;
} llrp_u16v_t;

extern llrp_u16v_t  LLRP_u16v_construct(llrp_u16_t nValue);
extern void         LLRP_u16v_clear(llrp_u16v_t *pDst);
extern void         LLRP_u16v_set(llrp_u16v_t *pDst, llrp_u16v_t  Value);
extern llrp_u16v_t  LLRP_u16v_copy(llrp_u16v_t Value);

typedef struct
{
    llrp_u16_t                  nValue;
    llrp_s16_t *                pValue;
} llrp_s16v_t;

extern llrp_s16v_t  LLRP_s16v_construct(llrp_u16_t nValue);
extern void         LLRP_s16v_clear(llrp_s16v_t *pDst);
extern void         LLRP_s16v_set(llrp_s16v_t *pDst, llrp_s16v_t  Value);
extern llrp_s16v_t  LLRP_s16v_copy(llrp_s16v_t Value);

typedef struct
{
    llrp_u16_t                  nValue;
    llrp_u32_t *                pValue;
} llrp_u32v_t;

extern llrp_u32v_t  LLRP_u32v_construct(llrp_u16_t nValue);
extern void         LLRP_u32v_clear(llrp_u32v_t *pDst);
extern void         LLRP_u32v_set(llrp_u32v_t *pDst, llrp_u32v_t  Value);
extern llrp_u32v_t  LLRP_u32v_copy(llrp_u32v_t Value);

typedef struct
{
    llrp_u16_t                  nValue;
    llrp_s32_t *                pValue;
} llrp_s32v_t;

extern llrp_s32v_t  LLRP_s32v_construct(llrp_u16_t nValue);
extern void         LLRP_s32v_clear(llrp_s32v_t *pDst);
extern void         LLRP_s32v_set(llrp_s32v_t *pDst, llrp_s32v_t  Value);
extern llrp_s32v_t  LLRP_s32v_copy(llrp_s32v_t Value);

typedef struct
{
    llrp_u16_t                  nValue;
    llrp_u64_t *                pValue;
} llrp_u64v_t;

extern llrp_u64v_t  LLRP_u64v_construct(llrp_u16_t nValue);
extern void         LLRP_u64v_clear(llrp_u64v_t *pDst);
extern void         LLRP_u64v_set(llrp_u64v_t *pDst, llrp_u64v_t  Value);
extern llrp_u64v_t  LLRP_u64v_copy(llrp_u64v_t Value);

typedef struct
{
    llrp_u16_t                  nValue;
    llrp_s64_t *                pValue;
} llrp_s64v_t;

extern llrp_s64v_t  LLRP_s64v_construct(llrp_u16_t nValue);
extern void         LLRP_s64v_clear(llrp_s64v_t *pDst);
extern void         LLRP_s64v_set(llrp_s64v_t *pDst, llrp_s64v_t  Value);
extern llrp_s64v_t  LLRP_s64v_copy(llrp_s64v_t Value);

typedef struct
{
    llrp_u16_t                  nBit;
    llrp_u8_t *                 pValue;
} llrp_u1v_t;

extern llrp_u1v_t   LLRP_u1v_construct(llrp_u16_t nBit);
extern void         LLRP_u1v_clear(llrp_u1v_t *pDst);
extern void         LLRP_u1v_set(llrp_u1v_t *pDst, llrp_u1v_t  Value);
extern llrp_u1v_t   LLRP_u1v_copy(llrp_u1v_t Value);

typedef struct
{
    llrp_u16_t                  nValue;
    llrp_utf8_t *               pValue;
} llrp_utf8v_t;

extern llrp_utf8v_t LLRP_utf8v_construct(llrp_u16_t nValue);
extern void         LLRP_utf8v_clear(llrp_utf8v_t *pDst);
extern void         LLRP_utf8v_set(llrp_utf8v_t *pDst, llrp_utf8v_t  Value);
extern llrp_utf8v_t LLRP_utf8v_copy(llrp_utf8v_t Value);

typedef struct
{
    llrp_u16_t                  nValue;
    llrp_byte_t *               pValue;
} llrp_bytesToEnd_t;

extern llrp_bytesToEnd_t LLRP_bytesToEnd_construct(llrp_u16_t nValue);
extern void         LLRP_bytesToEnd_clear(llrp_bytesToEnd_t *pDst);
extern void
LLRP_bytesToEnd_set(llrp_bytesToEnd_t *pDst, llrp_bytesToEnd_t  Value);
extern llrp_bytesToEnd_t LLRP_bytesToEnd_copy(llrp_bytesToEnd_t Value);

typedef struct
{
    llrp_u8_t                   aValue[12u];
} llrp_u96_t;

enum LLRP_ResultCode
{
    LLRP_RC_OK                          = 0,
    LLRP_RC_MiscError                   = 100,
    LLRP_RC_Botch,
    LLRP_RC_SendIOError,
    LLRP_RC_RecvIOError,
    LLRP_RC_RecvEOF,
    LLRP_RC_RecvTimeout,
    LLRP_RC_RecvFramingError,
    LLRP_RC_BadVersion,
    LLRP_RC_MissingResponseType,
    LLRP_RC_UnknownMessageType,
    LLRP_RC_UnknownParameterType,
    LLRP_RC_ExcessiveLength,
    LLRP_RC_InvalidLength,
    LLRP_RC_FieldUnderrun,
    LLRP_RC_ReservedBitsUnderrun,
    LLRP_RC_FieldOverrun,
    LLRP_RC_ReservedBitsOverrun,
    LLRP_RC_UnalignedBitField,
    LLRP_RC_UnalignedReservedBits,
    LLRP_RC_MessageAllocationFailed,
    LLRP_RC_ParameterAllocationFailed,
    LLRP_RC_FieldAllocationFailed,
    LLRP_RC_ExtraBytes,
    LLRP_RC_MissingParameter,
    LLRP_RC_UnexpectedParameter,
    LLRP_RC_InvalidChoiceMember,
    LLRP_RC_EnrollBadTypeNumber,
    LLRP_RC_NotAllowedAtExtensionPoint,
};

struct LLRP_SErrorDetails
{
    LLRP_tResultCode            eResultCode;
    const LLRP_tSTypeDescriptor *pRefType;
    const LLRP_tSFieldDescriptor *pRefField;
    const char *                pWhatStr;
    int                         OtherDetail;
    llrp_u8_t                   tcpErrno;
};

extern void
LLRP_Error_clear (
  LLRP_tSErrorDetails *         pError);

extern void
LLRP_Error_missingParameter (
  LLRP_tSErrorDetails *         pError,
  const LLRP_tSTypeDescriptor * pRefType);

extern void
LLRP_Error_unexpectedParameter (
  LLRP_tSErrorDetails *         pError,
  const LLRP_tSParameter *      pParameter);

extern void
LLRP_Error_resultCodeAndWhatStr (
  LLRP_tSErrorDetails *         pError,
  LLRP_tResultCode              eResultCode,
  const char *                  pWhatStr);




struct LLRP_SVendorDescriptor
{
    /* Short name for the vendor, e.g. "Acme" */
    char *                      pName;

    /* Vendor PEN of a custom message or parameter */
    llrp_u32_t                  VendorID;
};

struct LLRP_SNamespaceDescriptor
{
    /* Short name for the namespace, e.g. "acmeNS" */
    char *                      pPrefix;

    /* URI for the namespace, this is the true namespace name */
    char *                      pURI;

    /* URI for the XSD (schema) for custom parameters and messages
     * defined within the namespace */
    char *                      pSchemaLocation;
};


/*
 *
 *  +-----------------------+
 *  |                       |
 *  |  TypeDescriptor       --------+   ppFieldDescriptorTable
 *  |                       |       |
 *  +-----------------------+       |
 *                                  |
 *              +-------------------+
 *              V
 *  +-----------------------+
 *  |                       |           [field number]
 *  ~  FieldDescriptor *[]  --------+
 *  |                       |       |
 *  +-----------------------+       |
 *                                  |
 *              +-------------------+
 *              V
 *  +-----------------------+
 *  |                       |           Optional pEnumTable
 *  |  FieldDescriptor      --------+
 *  |                       |       |
 *  +-----------------------+       |
 *                                  |
 *              +-------------------+
 *              V
 *  +-----------------------+
 *  |                       |
 *  |  EnumTableEntry[]     |
 *  |                       |
 *  +-----------------------+
 *
 */



/*
 * STypeDescriptor
 *
 * Describes a message or parameter type.
 */

struct LLRP_STypeDescriptor
{
    /* TRUE for a message type, FALSE for a parameter type */
    llrp_bool_t                 bIsMessage;

    /* String name of parameter/message type (e.g. "ROSpec") */
    char *                      pName;

    /* NULL=>standard LLRP, !NULL=>Vendor (PEN) of custom
     * message or parameter */
    const LLRP_tSVendorDescriptor *   pVendorDescriptor;

    /* Namespace of message or parameter, for XML */
    const LLRP_tSNamespaceDescriptor *pNamespaceDescriptor;

    /* Type number or, for custom, subtype number */
    llrp_u32_t                  TypeNum;

    /* For messages (bIsMessage==TRUE), this is the type descriptor for
     * the corresponding response. NULL for a request or notification. */
    const LLRP_tSTypeDescriptor *   pResponseType;

    /* Table of pointers to the field descriptors */
    const LLRP_tSFieldDescriptor * const * const ppFieldDescriptorTable;

    /* Size of an instance of this element type */
    unsigned int                nSizeBytes;

    /* Function to make an instance of the parameter or message */
    LLRP_tSElement *
    (*pfConstruct)(void);

    /* Destruct instance, dealloc memory */
    void
    (*pfDestruct) (
      LLRP_tSElement *          pElement);

    /* Decoder, sometimes used when we want to decode w/o an instance */
    /* Function provided by each specific element type
     * to decode fields (simple values). Leaves pDecoderStream
     * at first subparameter. */

    void
    (*pfDecodeFields) (
      LLRP_tSElement *          pElement,
      LLRP_tSDecoderStream *    pDecoderStream);

    /* After fields are decoded, the CDecoder itself takes care
     * of gathering the subparameters into m_listAllSubParameters.
     * Once the end of the enclosing TLV (or message) is reached
     * this assimilateSubParameters() function is called to
     * create parameter refrences from the primary member variables.
     */
    void
    (*pfAssimilateSubParameters) (
      LLRP_tSElement *          pElement,
      LLRP_tSErrorDetails *     pError);

    /* Encode fields and subparameters */
    void
    (*pfEncode) (
      const LLRP_tSElement *    pElement,
      LLRP_tSEncoderStream *    pEncoderStream);

    /* For extension parameters, ask if they are allowed in
     * an enclosing parameter or message */
    llrp_bool_t
    (*pfIsAllowedIn) (
      const LLRP_tSTypeDescriptor *pEnclosingElementType);
};

enum LLRP_EFieldType {
    LLRP_FT_U8,  LLRP_FT_S8,  LLRP_FT_U8V,  LLRP_FT_S8V,
    LLRP_FT_U16, LLRP_FT_S16, LLRP_FT_U16V, LLRP_FT_S16V,
    LLRP_FT_U32, LLRP_FT_S32, LLRP_FT_U32V, LLRP_FT_S32V,
    LLRP_FT_U64, LLRP_FT_S64, LLRP_FT_U64V, LLRP_FT_S64V,

    LLRP_FT_U1,  LLRP_FT_U1V, LLRP_FT_U2,   LLRP_FT_U96,
    LLRP_FT_UTF8V,

    LLRP_FT_E1,  LLRP_FT_E2,  LLRP_FT_E8,   LLRP_FT_E16,   LLRP_FT_E32,
    LLRP_FT_E8V,

    LLRP_FT_BYTESTOEND,
};


enum LLRP_EFieldFormat {
    LLRP_FMT_NORMAL,
    LLRP_FMT_DEC,
    LLRP_FMT_HEX,
    LLRP_FMT_UTF8,
    LLRP_FMT_DATETIME,
};


/*
 * SFieldDescriptor
 *
 * Describes a single field.
 */
struct LLRP_SFieldDescriptor
{
    /* A code for the field type */
    LLRP_tEFieldType            eFieldType;
    /* A code for how the field should be formatted */
    LLRP_tEFieldType            eFieldFormat;
    /* String name of field (e.g. "ROSpecID") */
    char *                      pName;
    /* NULL or ptr to table base for enumerated fields */
    const LLRP_tSEnumTableEntry * pEnumTable;
};


extern LLRP_tSFieldDescriptor   LLRP_g_fdMessageHeader_Type;
extern LLRP_tSFieldDescriptor   LLRP_g_fdMessageHeader_Length;
extern LLRP_tSFieldDescriptor   LLRP_g_fdMessageHeader_MessageID;
extern LLRP_tSFieldDescriptor   LLRP_g_fdMessageHeader_VendorPEN;
extern LLRP_tSFieldDescriptor   LLRP_g_fdMessageHeader_Subtype;
extern LLRP_tSFieldDescriptor   LLRP_g_fdParameterHeader_TVType;
extern LLRP_tSFieldDescriptor   LLRP_g_fdParameterHeader_TLVType;
extern LLRP_tSFieldDescriptor   LLRP_g_fdParameterHeader_TLVLength;
extern LLRP_tSFieldDescriptor   LLRP_g_fdParameterHeader_VendorPEN;
extern LLRP_tSFieldDescriptor   LLRP_g_fdParameterHeader_Subtype;


/*
 * SEnumTableEntry
 *
 * Simple table of enumerations. Table is terminated
 * by an entry with pName==NULL.
 */
struct LLRP_SEnumTableEntry
{
    /* String name, (e.g. "Immediate") */
    char *                      pName;
    int                         Value;
};

/*
 * STypeRegistry
 *
 * A collection of pointers to STypeDescriptors.
 * During decode operations types can be looked up
 * by code (vendor and typenum) or by name.
 */
#define LTKC_MAX_CUSTOM_MESSAGE     100u
#define LTKC_MAX_CUSTOM_PARAMETER   300u
struct LLRP_STypeRegistry
{
    /* Standard messages subscripted by type number */
    const LLRP_tSTypeDescriptor * apStdMessageTypeDescriptors[1024u];
    /* Standard parameters subscripted by type number */
    const LLRP_tSTypeDescriptor * apStdParameterTypeDescriptors[1024u];

    /* Custom messages, sequential search */
    const LLRP_tSTypeDescriptor *
                apCustMessageTypeDescriptors[LTKC_MAX_CUSTOM_MESSAGE];
    unsigned int                nCustMessageTypeDescriptors;
    /* Custom parameters, sequential search */
    const LLRP_tSTypeDescriptor *
                apCustParameterTypeDescriptors[LTKC_MAX_CUSTOM_PARAMETER];
    unsigned int                nCustParameterTypeDescriptors;
};

/* Create a new TypeRegistry */
extern LLRP_tSTypeRegistry *
LLRP_TypeRegistry_construct (void);

/* Destruct a TypeRegistry */
extern void
LLRP_TypeRegistry_destruct (
  LLRP_tSTypeRegistry *         pTypeRegistry);

/* Add a type descriptor to the registry */
extern LLRP_tResultCode
LLRP_TypeRegistry_enroll (
  LLRP_tSTypeRegistry *         pTypeRegistry,
  const LLRP_tSTypeDescriptor * pTypeDescriptor);

/* Lookup a standard message type descriptor. NULL=>not found */
const LLRP_tSTypeDescriptor *
LLRP_TypeRegistry_lookupMessage (
  const LLRP_tSTypeRegistry *   pTypeRegistry,
  unsigned int                  MessageTypeNum);

/* Lookup a standard parameter type descriptor. NULL=>not found */
const LLRP_tSTypeDescriptor *
LLRP_TypeRegistry_lookupParameter (
  const LLRP_tSTypeRegistry *   pTypeRegistry,
  unsigned int                  ParameterTypeNum);

/* Lookup a custom message type descriptor. NULL=>not found */
const LLRP_tSTypeDescriptor *
LLRP_TypeRegistry_lookupCustomMessage (
  const LLRP_tSTypeRegistry *   pTypeRegistry,
  unsigned int                  VendorID,
  unsigned int                  MessageSubTypeNum);

/* Lookup a custom parameter type descriptor. NULL=>not found */
const LLRP_tSTypeDescriptor *
LLRP_TypeRegistry_lookupCustomParameter (
  const LLRP_tSTypeRegistry *   pTypeRegistry,
  unsigned int                  VendorID,
  unsigned int                  ParameterSubTypeNum);


/*
 * SElement
 *
 * This is the base class for all parameter and message types.
 *
 * During decode, all subparameters found are entered
 * on m_listAllSubParameters. Then the element's
 * assimilateSubParameters() member function is called
 * to iterate through the list and attach the parameters
 * to specific fields.
 *
 * The m_listAllSubParameters is a secondary reference to
 * all the subparameters. When the element is destructed
 * all parameters referenced by m_listAllSubParameters
 * are deleted. The C++ intrinsic destructors take care
 * of deleting the list itself.
 *
 * During destruct the specific fields are not processed.
 * The fields that are lists are automatically desctructed.
 * So are the fields that are array types (i.e. utf8v) are
 * also automatically destructed. The fields that are simple
 * pointers are simply ignored.
 *
 * This works because every parameter referenced by specific
 * fields is also referenced by m_listAllSubParameters.
 */

struct LLRP_SElement
{
    /* The type descriptor desribing this element */
    const LLRP_tSTypeDescriptor * pType;

    /* Element that encloses this one, NULL if this is top-level element */
    LLRP_tSElement *            pParent;

    /* List of all sub elements */
    LLRP_tSParameter *          listAllSubParameters;
};

struct LLRP_SMessage
{
    LLRP_tSElement              elementHdr;

    llrp_u32_t                  MessageID;

    LLRP_tSMessage *            pQueueNext;
};

struct LLRP_SParameter
{
    LLRP_tSElement              elementHdr;

    /* Next pointer for list of all sub elements */
    LLRP_tSParameter *          pNextAllSubParameters;

    /* Next pointer for element headed by specific member */
    LLRP_tSParameter *          pNextSubParameter;
};


/*
 * ltkc_element.c
 */
LLRP_tSElement *
LLRP_Element_construct (
  const LLRP_tSTypeDescriptor *  pTypeDescriptor);

extern void
LLRP_Element_destruct (
  LLRP_tSElement *              pElement);

extern void
LLRP_Element_finalDestruct (
  LLRP_tSElement *              pElement);

extern void
LLRP_Element_addSubParameterToAllList (
  LLRP_tSElement *              pElement,
  LLRP_tSParameter *            pParameter);

extern void
LLRP_Element_removeSubParameterFromAllList (
  LLRP_tSElement *              pElement,
  LLRP_tSParameter *            pParameter);

extern void
LLRP_Element_clearSubParameterAllList (
  LLRP_tSElement *              pElement);

extern void
LLRP_Element_setSubParameterPtr (
  LLRP_tSElement *              pElement,
  LLRP_tSParameter **           ppPtr,
  LLRP_tSParameter *            pValue);

extern void
LLRP_Element_addToSubParameterList (
  LLRP_tSElement *              pElement,
  LLRP_tSParameter **           ppListHead,
  LLRP_tSParameter *            pValue);

extern void
LLRP_Element_attachToSubParameterList (
  LLRP_tSParameter **           ppListHead,
  LLRP_tSParameter *            pValue);

extern void
LLRP_Element_clearSubParameterList (
  LLRP_tSElement *              pElement,
  LLRP_tSParameter **           ppListHead);

extern int
LLRP_Element_walk (
  const LLRP_tSElement *        pElement,
  int                           (*pFunc)(
                                  const LLRP_tSElement *    pElement,
                                  void *                    pArg),
  void *                        pArg,
  int                           iDepth,
  int                           nMaxDepth);

extern void
LLRP_Message_setMessageID (
  LLRP_tSMessage *              pMessage,
  llrp_u32_t                    MessageID);

extern llrp_bool_t
LLRP_Parameter_isAllowedIn (
  LLRP_tSParameter *            pParameter,
  const LLRP_tSTypeDescriptor * pEnclosingTypeDescriptor);

extern llrp_bool_t
LLRP_Parameter_isAllowedExtension (
  LLRP_tSParameter *            pParameter,
  const LLRP_tSTypeDescriptor * pEnclosingTypeDescriptor);


/*
 *
 * By way of example, this is how the CDecoder and CDecoderStream
 * classes work. This example is for decoding a binary frame.
 *
 *      +-------------------+               +---------------+
 *      |                   |               |               |
 *      |   CDecoder        --------------->| CTypeRegistry |
 *      |                   |               |               |
 *      +--|----------------+               +---------------+
 *         |    ^
 * pointer |    |
 * to next |    |   +-------------------+
 * byte    |    |   |                   |           pointer to msg end
 *         |    ^----  CDecoderStream   ----------------+
 *         |    |   |                   |               |
 *         |    |   +-------------------+               |
 *         |    |             ^                         |
 *         |    |             |                         |
 *         |    |   +-------------------+   ptr to      |
 *         |    |   |                   |   TLV end     |
 *         |    ^----  CDecoderStream   ------------+   |
 *         |    |   |                   |           |   |
 *         |    |   +-------------------+           |   |
 *         |    |             ^                     |   |
 *         |    |             |                     |   |
 *         |    |   +-------------------+           |   |
 *         |    |   |                   |           |   |
 *         |    ^----  CDecoderStream   --------+   |   |
 *         |        |                   |       |   |   |
 *         |        +-------------------+       |   |   |
 *         |                                    |   |   |
 *         +-------------------+                |   |   |
 *                             |                |   |   |
 *                             v                v   v   v
 *  +---------------------------------------------------------------+
 *  |                   Binary Frame Buffer                         |
 *  +---------------------------------------------------------------+
 *
 *                            \_________________/          Nestec TLVs
 *        \________________/\___________________________/  Nested TLVs
 *    \_________________________________________________/  Message
 *
 *
 * In the case of binary frame the references are to
 * bytes within the buffer. Lookups are by type number.
 *
 * In the case of an XML DOM tree, the references are
 * to nodes in the DOM tre. Lookups are by string name.
 */

struct LLRP_SDecoder
{
    const LLRP_tSDecoderOps *   pDecoderOps;

    const LLRP_tSTypeRegistry * pRegistry;

    LLRP_tSElement *            pRootElement;

    LLRP_tSErrorDetails         ErrorDetails;
};

struct LLRP_SDecoderOps
{
    void
    (*pfDestruct) (
      LLRP_tSDecoder *          pDecoder);

    LLRP_tSMessage *
    (*pfDecodeMessage) (
      LLRP_tSDecoder *          pDecoder);
};

struct LLRP_SDecoderStream
{
    LLRP_tSDecoderStreamOps *   pDecoderStreamOps;
};

struct LLRP_SDecoderStreamOps
{
    /*
     * 8-bit types
     */

    llrp_u8_t
    (*pfGet_u8) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    llrp_s8_t
    (*pfGet_s8) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    llrp_u8v_t
    (*pfGet_u8v) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    llrp_s8v_t
    (*pfGet_s8v) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    /*
     * 16-bit types
     */

    llrp_u16_t
    (*pfGet_u16) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    llrp_s16_t
    (*pfGet_s16) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    llrp_u16v_t
    (*pfGet_u16v) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    llrp_s16v_t
    (*pfGet_s16v) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    /*
     * 32-bit types
     */

    llrp_u32_t
    (*pfGet_u32) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    llrp_s32_t
    (*pfGet_s32) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    llrp_u32v_t
    (*pfGet_u32v) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    llrp_s32v_t
    (*pfGet_s32v) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    /*
     * 64-bit types
     */

    llrp_u64_t
    (*pfGet_u64) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    llrp_s64_t
    (*pfGet_s64) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    llrp_u64v_t
    (*pfGet_u64v) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    llrp_s64v_t
    (*pfGet_s64v) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    /*
     * Special types
     */

    llrp_u1_t
    (*pfGet_u1) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    llrp_u1v_t
    (*pfGet_u1v) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    llrp_u2_t
    (*pfGet_u2) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    llrp_u96_t
    (*pfGet_u96) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    llrp_utf8v_t
    (*pfGet_utf8v) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    llrp_bytesToEnd_t
    (*pfGet_bytesToEnd) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);


    /*
     * Enumerated types of various sizes
     */

    int
    (*pfGet_e1) (
       LLRP_tSDecoderStream *   pDecoderStream,
       const LLRP_tSFieldDescriptor *pFieldDescriptor);

    int
    (*pfGet_e2) (
       LLRP_tSDecoderStream *   pDecoderStream,
       const LLRP_tSFieldDescriptor *pFieldDescriptor);

    int
    (*pfGet_e8) (
       LLRP_tSDecoderStream *   pDecoderStream,
       const LLRP_tSFieldDescriptor *pFieldDescriptor);

    int
    (*pfGet_e16) (
       LLRP_tSDecoderStream *   pDecoderStream,
       const LLRP_tSFieldDescriptor *pFieldDescriptor);

    int
    (*pfGet_e32) (
       LLRP_tSDecoderStream *   pDecoderStream,
       const LLRP_tSFieldDescriptor *pFieldDescriptor);

    llrp_u8v_t
    (*pfGet_e8v) (
      LLRP_tSDecoderStream *    pDecoderStream,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    /*
     * Reserved means some number of bits
     */

    void
    (*pfGet_reserved) (
       LLRP_tSDecoderStream *   pDecoderStream,
       unsigned int             nBits);
};



struct LLRP_SEncoder
{
    const LLRP_tSEncoderOps *   pEncoderOps;

    LLRP_tSErrorDetails         ErrorDetails;
};

struct LLRP_SEncoderOps
{
    void
    (*pfDestruct) (
      LLRP_tSEncoder *          pEncoder);

    void
    (*pfEncodeElement) (
      LLRP_tSEncoder *          pEncoder,
      const LLRP_tSElement *    pElement);
};

struct LLRP_SEncoderStream
{
    LLRP_tSEncoderStreamOps *   pEncoderStreamOps;
};

struct LLRP_SEncoderStreamOps
{
    void
    (*pfPutRequiredSubParameter) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const LLRP_tSParameter *  pParameter,
      const LLRP_tSTypeDescriptor *pRefType);

    void
    (*pfPutOptionalSubParameter) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const LLRP_tSParameter *  pParameter,
      const LLRP_tSTypeDescriptor *pRefType);

    void
    (*pfPutRequiredSubParameterList) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const LLRP_tSParameter *  pParameterList,
      const LLRP_tSTypeDescriptor *pRefType);

    void
    (*pfPutOptionalSubParameterList) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const LLRP_tSParameter *  pParameterList,
      const LLRP_tSTypeDescriptor *pRefType);

    /*
     * 8-bit types
     */

    void
    (*pfPut_u8) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_u8_t           Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_s8) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_s8_t           Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_u8v) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_u8v_t          Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_s8v) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_s8v_t          Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    /*
     * 16-bit types
     */

    void
    (*pfPut_u16) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_u16_t          Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_s16) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_s16_t          Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_u16v) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_u16v_t         Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_s16v) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_s16v_t         Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    /*
     * 32-bit types
     */

    void
    (*pfPut_u32) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_u32_t          Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_s32) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_s32_t          Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_u32v) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_u32v_t         Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_s32v) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_s32v_t         Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    /*
     * 64-bit types
     */

    void
    (*pfPut_u64) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_u64_t          Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_s64) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_s64_t          Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_u64v) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_u64v_t         Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_s64v) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_s64v_t         Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    /*
     * Special types
     */

    void
    (*pfPut_u1) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_u1_t           Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_u1v) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_u1v_t          Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_u2) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_u2_t           Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_u96) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_u96_t          Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_utf8v) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_utf8v_t        Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_bytesToEnd) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_bytesToEnd_t   Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    /*
     * Enumerated types of various sizes
     */

    void
    (*pfPut_e1) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const int                 Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_e2) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const int                 Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_e8) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const int                 Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_e16) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const int                 Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_e32) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const int                 Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);

    void
    (*pfPut_e8v) (
      LLRP_tSEncoderStream *    pEncoderStream,
      const llrp_u8v_t          Value,
      const LLRP_tSFieldDescriptor *pFieldDescriptor);


    /*
     * Reserved means some number of bits
     */

    void
    (*pfPut_reserved) (
      LLRP_tSEncoderStream *    pEncoderStream,
      unsigned int              nBits);
};

/*
 * ltkc_encdec.c
 */
extern void
LLRP_Decoder_destruct (
  LLRP_tSDecoder *              pDecoder);

LLRP_tSMessage *
LLRP_Decoder_decodeMessage (
  LLRP_tSDecoder *              pDecoder);

extern void
LLRP_Encoder_destruct (
  LLRP_tSEncoder *              pEncoder);

extern void
LLRP_Encoder_encodeElement (
  LLRP_tSEncoder *              pEncoder,
  const LLRP_tSElement *        pElement);


/*
 * ltkc_xmltextencode.c
 */

extern LLRP_tResultCode
LLRP_toXMLString (
  const LLRP_tSElement *        pElement,
  char *                        pBuffer,
  int                           nBuffer);
