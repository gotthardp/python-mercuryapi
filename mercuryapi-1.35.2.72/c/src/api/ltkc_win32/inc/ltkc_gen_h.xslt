<?xml version="1.0" encoding="UTF-8"?>
<!--
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -  Copyright 2007,2008 Impinj, Inc.
 -
 -  Licensed under the Apache License, Version 2.0 (the "License");
 -  you may not use this file except in compliance with the License.
 -  You may obtain a copy of the License at
 -
 -      http://www.apache.org/licenses/LICENSE-2.0
 -
 -  Unless required by applicable law or agreed to in writing, software
 -  distributed under the License is distributed on an "AS IS" BASIS,
 -  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 -  See the License for the specific language governing permissions and
 -  limitations under the License.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:stylesheet
        version='1.0'
        xmlns:LL="http://www.llrp.org/ltk/schema/core/encoding/binary/1.0"
        xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
<xsl:output omit-xml-declaration='yes' method='text' encoding='iso-8859-1'/>

<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief top level template
 -
 - Matches the <llrpdef> root, thereby traversing it, and invokes
 - the various templates that generate the sections.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template match='/LL:llrpdef'>
<xsl:call-template name='FileHeader'/>
<xsl:call-template name='ForwardDeclStructMessages'/>
<xsl:call-template name='ForwardDeclStructParameters'/>
<xsl:call-template name='VendorDescriptorDeclarations'/>
<xsl:call-template name='NamespaceDescriptorDeclarations'/>
<xsl:call-template name='EnumerationDefinitionsFields'/>
<xsl:call-template name='StructDeclarationsMessages'/>
<xsl:call-template name='StructDeclarationsParameters'/>
<xsl:call-template name='StructDeclarationsChoices'/>

void
LLRP_enroll<xsl:value-of select='$RegistryName'/>TypesIntoRegistry (
  LLRP_tSTypeRegistry *         pTypeRegistry);

<xsl:call-template name='FileFooter'/>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief FileHeader template
 -
 - Invoked by top level template.
 -
 - Generates the source file header that warns that the file is generated.
 - @todo    It would be nice if we could get a date and time stamp and
 -          maybe some details about what input descriptions were used.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='FileHeader' xml:space='preserve'>
/*
 * Generated file - DO NOT EDIT
 *
 * This is the header file for the LLRP Tool Kit (LTK)
 * C++ (aka cpp) implementation. It is generated into a .inc file
 * that is included by a platform specific .h header file.
 * That .h file takes care of prerequisites needed by this file.
 */

#ifdef __cplusplus
extern "C" {
#endif
</xsl:template>

<xsl:template name='FileFooter' xml:space='preserve'>
#ifdef __cplusplus
}
#endif
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief ForwardDeclStructMessages template
 -
 - Invoked by top level template.
 -
 - Generates the section containing forward declarations
 - of the structures for LLRP messages.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='ForwardDeclStructMessages'>

/*
 * Message classes - forward decls
 */
<xsl:for-each select='LL:messageDefinition'>
struct LLRP_S<xsl:value-of select='@name'/>;
typedef struct LLRP_S<xsl:value-of select='@name'/>
    LLRP_tS<xsl:value-of select='@name'/>;
</xsl:for-each>

/* Custom messages */
<xsl:for-each select='LL:customMessageDefinition'>
struct LLRP_S<xsl:value-of select='@name'/>;
typedef struct LLRP_S<xsl:value-of select='@name'/>
    LLRP_tS<xsl:value-of select='@name'/>;
</xsl:for-each>

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief ForwardDeclStructParameters template
 -
 - Invoked by top level template.
 -
 - Generates the section containing forward declarations
 - of the structures for LLRP parameters.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='ForwardDeclStructParameters'>

/*
 * Parameter classes - forward decls
 */
<xsl:for-each select='LL:parameterDefinition'>
struct LLRP_S<xsl:value-of select='@name'/>;
typedef struct LLRP_S<xsl:value-of select='@name'/>
    LLRP_tS<xsl:value-of select='@name'/>;
</xsl:for-each>

/* Custom parameters */
<xsl:for-each select='LL:customParameterDefinition'>
struct LLRP_S<xsl:value-of select='@name'/>;
typedef struct LLRP_S<xsl:value-of select='@name'/>
    LLRP_tS<xsl:value-of select='@name'/>;
</xsl:for-each>

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief VendorDescriptorDeclarations template
 -
 - Invoked by top level template.
 -
 - Generates declarations of the vendor descriptors.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='VendorDescriptorDeclarations'>

/*
 * Vendor descriptor declarations.
 */
<xsl:for-each select='LL:vendorDefinition'>
extern const LLRP_tSVendorDescriptor
LLRP_vdesc<xsl:value-of select='@name'/>;
</xsl:for-each>

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief NamespaceDescriptorDeclarations template
 -
 - Invoked by top level template.
 -
 - Generates declarations of the namespace descriptors.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='NamespaceDescriptorDeclarations'>

/*
 * Namespace descriptor declarations.
 */
<xsl:for-each select='LL:namespaceDefinition'>
extern const LLRP_tSNamespaceDescriptor
LLRP_nsdesc<xsl:value-of select='@prefix'/>;
</xsl:for-each>

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief EnumerationDefinitionsFields template
 -
 - Invoked by top level template.
 -
 - Generates definitions of the enumerations and declares the
 - enumeration string tables.
 -
 - The enumeration entry names must be unique in the LLRP namespace.
 - The values must be per the spec.
 -
 - The enumeration string tables are referenced by field descriptors.
 -
 - NB: The LLRP specification does not define enumeration names.
 -     The names in the input description were derived from text
 -     of the LLRP specification.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='EnumerationDefinitionsFields'>

/*
 * Enumeration definitions and declarations of
 * enumeration string tables.
 */

<xsl:for-each select='LL:enumerationDefinition|LL:customEnumerationDefinition'>
  <xsl:variable name='enumBaseName' select='@name'/>
enum LLRP_E<xsl:value-of select='@name'/>
{
<xsl:for-each select='LL:entry' xml:space='preserve'>
    LLRP_<xsl:value-of select='$enumBaseName'/>_<xsl:value-of select='@name'/> = <xsl:value-of select='@value'/>,</xsl:for-each>
};

typedef enum LLRP_E<xsl:value-of select='@name'/>
    LLRP_tE<xsl:value-of select='@name'/>;

extern const LLRP_tSEnumTableEntry
LLRP_est<xsl:value-of select='$enumBaseName'/>[];

</xsl:for-each>

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclarationsMessages template
 -
 - Invoked by top level template.
 -
 - Generates declaration of the message classes.
 - A message class is derived from the CMessage base class
 - which is derived from the CElement base class.
 - This loops through the message definitions, selects
 - important values, and invokes the StructDeclarationCommon
 - template with the right <xsl:with-param>s.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclarationsMessages'>

<xsl:for-each select='LL:messageDefinition|LL:customMessageDefinition'>
  <xsl:call-template name='StructDeclarationCommon'>
    <xsl:with-param name='StructBase'>LLRP_tSMessage</xsl:with-param>
    <xsl:with-param name='StructName'>LLRP_S<xsl:value-of select='@name'/></xsl:with-param>
    <xsl:with-param name='BaseName'><xsl:value-of select='@name'/></xsl:with-param>
    <xsl:with-param name='IsCustomParameter'>false</xsl:with-param>
  </xsl:call-template>
</xsl:for-each>

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclarationsParameters template
 -
 - Invoked by top level template.
 -
 - Generates declaration of the parameter classes.
 - A parameter class is derived from the CParameter base class
 - which is derived from the CElement base class.
 - This loops through the parameter definitions, selects
 - important values, and invokes the StructDeclarationCommon
 - template with the right <xsl:with-param>s.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclarationsParameters'>

<xsl:for-each select='LL:parameterDefinition|LL:customParameterDefinition'>
  <xsl:call-template name='StructDeclarationCommon'>
    <xsl:with-param name='StructBase'>LLRP_tSParameter</xsl:with-param>
    <xsl:with-param name='StructName'>LLRP_S<xsl:value-of select='@name'/></xsl:with-param>
    <xsl:with-param name='BaseName'><xsl:value-of select='@name'/></xsl:with-param>
    <xsl:with-param name='IsCustomParameter'>
      <xsl:choose>
        <xsl:when test='self::LL:customParameterDefinition'>true</xsl:when>
        <xsl:otherwise>false</xsl:otherwise>
      </xsl:choose>
    </xsl:with-param>
  </xsl:call-template>
</xsl:for-each>

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclarationCommon template
 -
 - Invoked by templates
 -      StructDeclarationsMessages
 -      StructDeclarationsParameters
 -
 - @param   StructBase      Name of the base class, CMessage
 -                          or CParameter usually
 - @param   StructName      Name of generated class. This already has
 -                          "LLRP_tS" prefixed to the LLRP name.
 - @param   IsCustomParameter Either "true" or "false".
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclarationCommon' xml:space='preserve'>
  <xsl:param name='StructBase'/>
  <xsl:param name='StructName'/>
  <xsl:param name='BaseName'/>
  <xsl:param name='IsCustomParameter'/>
  <xsl:variable name='TypeName'>LLRP_tS<xsl:value-of select='$BaseName'/></xsl:variable>
struct <xsl:value-of select='$StructName'/>
{
    <xsl:value-of select='$StructBase'/> hdr;
  <xsl:call-template name='StructDeclFields'/>
  <xsl:call-template name='StructDeclSubParameters'/>
};

extern const LLRP_tSTypeDescriptor
LLRP_td<xsl:value-of select='@name'/>;

extern const LLRP_tSFieldDescriptor *
LLRP_apfd<xsl:value-of select='@name'/>[];

extern <xsl:value-of select='$TypeName'/> *
LLRP_<xsl:value-of select='$BaseName'/>_construct (void);

extern void
LLRP_<xsl:value-of select='$BaseName'/>_destruct (
 <xsl:value-of select='$TypeName'/> * pThis);

extern void
LLRP_<xsl:value-of select='$BaseName'/>_decodeFields (
 <xsl:value-of select='$TypeName'/> * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_<xsl:value-of select='$BaseName'/>_assimilateSubParameters (
 <xsl:value-of select='$TypeName'/> * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_<xsl:value-of select='$BaseName'/>_encode (
  const LLRP_tS<xsl:value-of select='$BaseName'/> *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  <xsl:if test='$IsCustomParameter = "true"'>
extern llrp_bool_t
LLRP_<xsl:value-of select='$BaseName'/>_isAllowedIn (
  const LLRP_tSTypeDescriptor *pEnclosingElementType);
  </xsl:if>

  <xsl:call-template name='StructDeclFieldAccessors'/>
  <xsl:call-template name='StructDeclSubParameterAccessors'/>

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclFields template
 -
 - Invoked by template
 -      StructDeclarationCommon
 -
 - Current node
 -      <llrpdef><messageDefinition>
 -      <llrpdef><parameterDefinition>
 -
 - Generate for each field the member variable.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclFields'>
  <xsl:variable name='StructName' select='@name'/>
  <xsl:for-each select='LL:field'>
    <xsl:call-template name='StructDeclOneField'>
      <xsl:with-param name='StructName' select='$StructName'/>
      <xsl:with-param name='FieldType'>
        <xsl:choose>
          <xsl:when test='@enumeration and @type = "u8v"'>llrp_u8v_t </xsl:when>
          <xsl:when test='@enumeration'>LLRP_tE<xsl:value-of select='@enumeration'/> </xsl:when>
          <xsl:otherwise>llrp_<xsl:value-of select='@type'/>_t </xsl:otherwise>
        </xsl:choose>
      </xsl:with-param>
      <xsl:with-param name='MemberName'>
        <xsl:choose>
          <xsl:when test='@enumeration and @type = "u8v"'><xsl:value-of select='@name'/></xsl:when>
          <xsl:when test='@enumeration'> e<xsl:value-of select='@name'/></xsl:when>
          <xsl:otherwise><xsl:value-of select='@name'/></xsl:otherwise>
       </xsl:choose>
      </xsl:with-param>
      <xsl:with-param name='BaseName'><xsl:value-of select='@name'/></xsl:with-param>
    </xsl:call-template>
  </xsl:for-each>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclOneField template
 -
 - Invoked by templates
 -      StructDeclFields
 -
 - Current node
 -      <llrpdef><messageDefinition><field>
 -      <llrpdef><parameterDefinition><field>
 -
 - Generate the field member and accessor (get/set) functions
 - for a field. The accessors are inline. The tricky part
 - is determining whether the field is enumerated or not
 - and adjust the native type and member name accordingly.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclOneField'
              xml:space='preserve'>
  <xsl:param name='StructName'/><xsl:param name='FieldType'/><xsl:param name='MemberName'/><xsl:param name='BaseName'/>
    <xsl:value-of select='$FieldType'/> <xsl:value-of select='$MemberName'/>;
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclFieldAccessors template
 -
 - Invoked by template
 -      StructDeclarationCommon
 -
 - Current node
 -      <llrpdef><messageDefinition>
 -      <llrpdef><parameterDefinition>
 -
 - Generate for each field the accessor functions.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclFieldAccessors'>
  <xsl:variable name='StructName' select='@name'/>
  <xsl:for-each select='LL:field'>
    <xsl:call-template name='StructDeclOneFieldAccessor'>
      <xsl:with-param name='StructName' select='$StructName'/>
      <xsl:with-param name='FieldType'>
        <xsl:choose>
          <xsl:when test='@enumeration and @type = "u8v"'>llrp_u8v_t</xsl:when>
          <xsl:when test='@enumeration'>LLRP_tE<xsl:value-of select='@enumeration'/></xsl:when>
          <xsl:otherwise>llrp_<xsl:value-of select='@type'/>_t</xsl:otherwise>
        </xsl:choose>
      </xsl:with-param>
      <xsl:with-param name='MemberName'>
        <xsl:choose>
          <xsl:when test='@enumeration and @type = "u8v"'><xsl:value-of select='@name'/></xsl:when>
          <xsl:when test='@enumeration'>e<xsl:value-of select='@name'/></xsl:when>
          <xsl:otherwise><xsl:value-of select='@name'/></xsl:otherwise>
       </xsl:choose>
      </xsl:with-param>
      <xsl:with-param name='BaseName'><xsl:value-of select='@name'/></xsl:with-param>
    </xsl:call-template>
  </xsl:for-each>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclOneFieldAccessor template
 -
 - Invoked by templates
 -      StructDeclFieldAccessors
 -
 - Current node
 -      <llrpdef><messageDefinition><field>
 -      <llrpdef><parameterDefinition><field>
 -
 - Generate the field accessor (get/set) functions for a field.
 - The accessors are inline. The tricky part is determining whether
 - the field is enumerated or not and adjust the native type and
 - member name accordingly.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclOneFieldAccessor'>
  <xsl:param name='StructName'/>
  <xsl:param name='FieldType'/>
  <xsl:param name='MemberName'/>
  <xsl:param name='BaseName'/>
extern const LLRP_tSFieldDescriptor
LLRP_fd<xsl:value-of select='$StructName'/>_<xsl:value-of select='$BaseName'/>;

extern <xsl:value-of select='$FieldType'/>
LLRP_<xsl:value-of select='$StructName'/>_get<xsl:value-of select='$BaseName'/> (
  LLRP_tS<xsl:value-of select='$StructName'/> *pThis);

extern LLRP_tResultCode
LLRP_<xsl:value-of select='$StructName'/>_set<xsl:value-of select='$BaseName'/> (
  LLRP_tS<xsl:value-of select='$StructName'/> *pThis,
  <xsl:value-of select='$FieldType'/> Value);
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclSubParameters template
 -
 - Invoked by template
 -      StructDeclarationCommon
 -
 - Generate for each parameter (or choice) the member variable
 - and declare accessor functions. This gets tricky:
 -      * Determine if it is a parameter or choice
 -      * Determine the name, either explicit name= or derived from type=
 -      * Determine the kind of repeat (1, 0-1, 0-N, 1-N)
 -      * Make the right kind of member variable (* or std::list)
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclSubParameters'>
  <xsl:for-each select='LL:parameter|LL:choice'>
    <xsl:choose>
      <xsl:when test='self::LL:parameter and @type = "Custom"'>
        <xsl:call-template name='StructDeclSubExtensionPoint'/>
      </xsl:when>
      <xsl:when test='self::LL:parameter'>
        <xsl:call-template name='StructDeclSubParam'/>
      </xsl:when>
      <xsl:when test='self::LL:choice'>
        <xsl:call-template name='StructDeclSubChoice'/>
      </xsl:when>
      <xsl:otherwise>
        HELP: StructDeclSubParameters <xsl:value-of select='@type'/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:for-each>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclSubParam template
 -
 - Invoked by template
 -      StructDeclSubParameters
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclSubParam'>
  <xsl:choose>
    <xsl:when test='@name'>
      <xsl:call-template name='StructDeclSubXXXWithNameAndType'>
        <xsl:with-param name='Name'><xsl:value-of select='@name'/></xsl:with-param>
        <xsl:with-param name='NativeType'>LLRP_tS<xsl:value-of select='@type'/></xsl:with-param>
        <xsl:with-param name='Repeat'><xsl:value-of select='@repeat'/></xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name='StructDeclSubXXXWithNameAndType'>
        <xsl:with-param name='Name'><xsl:value-of select='@type'/></xsl:with-param>
        <xsl:with-param name='NativeType'>LLRP_tS<xsl:value-of select='@type'/></xsl:with-param>
        <xsl:with-param name='Repeat'><xsl:value-of select='@repeat'/></xsl:with-param>
      </xsl:call-template>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclSubChoice template
 -
 - Invoked by template
 -      StructDeclSubParameters
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclSubChoice'>
  <xsl:choose>
    <xsl:when test='@name'>
      <xsl:call-template name='StructDeclSubXXXWithNameAndType'>
        <xsl:with-param name='Name'><xsl:value-of select='@name'/></xsl:with-param>
        <xsl:with-param name='NativeType'>LLRP_tSParameter</xsl:with-param>
        <xsl:with-param name='Repeat'><xsl:value-of select='@repeat'/></xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name='StructDeclSubXXXWithNameAndType'>
        <xsl:with-param name='Name'><xsl:value-of select='@type'/></xsl:with-param>
        <xsl:with-param name='NativeType'>LLRP_tSParameter</xsl:with-param>
        <xsl:with-param name='Repeat'><xsl:value-of select='@repeat'/></xsl:with-param>
      </xsl:call-template>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclSubExtensionPoint template
 -
 - Invoked by template
 -      StructDeclSubParameters
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclSubExtensionPoint'>
  <xsl:call-template name='StructDeclSubXXXWithNameAndType'>
    <xsl:with-param name='Name'><xsl:value-of select='@type'/></xsl:with-param>
    <xsl:with-param name='NativeType'>LLRP_tSParameter</xsl:with-param>
    <xsl:with-param name='Repeat'><xsl:value-of select='@repeat'/></xsl:with-param>
  </xsl:call-template>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclSubXXXWithNameAndType template
 -
 - Invoked by template
 -      StructDeclSubParam
 -      StructDeclSubChoice
 -
 - Common for parameters and choices.
 -
 - @param   Name
 - @param   NativeType
 - @param   Repeat
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclSubXXXWithNameAndType'>
  <xsl:param name='Name'/>
  <xsl:param name='NativeType'/>
  <xsl:param name='Repeat'/>
  <xsl:choose>
    <xsl:when test='$Repeat="1" or $Repeat="0-1"'>
      <xsl:call-template name='StructDeclSubXXXWithNameAndType_Ptr'>
        <xsl:with-param name='Name'><xsl:value-of select='$Name'/></xsl:with-param>
        <xsl:with-param name='NativeType'><xsl:value-of select='$NativeType'/></xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test='$Repeat="0-N" or $Repeat="1-N"'>
      <xsl:call-template name='StructDeclSubXXXWithNameAndType_List'>
        <xsl:with-param name='Name'><xsl:value-of select='$Name'/></xsl:with-param>
        <xsl:with-param name='NativeType'><xsl:value-of select='$NativeType'/></xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise xml:space='preserve'>
    HELP: StructDeclSubXXXWithNameAndType <xsl:value-of select='$Name'/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclSubXXXWithNameAndType_Ptr template
 -
 - Invoked by template
 -      StructDeclSubXXXWithNameAndType
 -
 - Common for parameters and choices.
 -
 - @param   Name
 - @param   NativeType
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclSubXXXWithNameAndType_Ptr'
              xml:space='preserve'>
  <xsl:param name='Name'/><xsl:param name='NativeType'/>
    <xsl:value-of select='$NativeType'/> * p<xsl:value-of select='$Name'/>;
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclSubXXXWithNameAndType_List template
 -
 - Invoked by template
 -      StructDeclSubXXXWithNameAndType
 -
 - Common for parameters and choices.
 -
 - @param   Name
 - @param   NativeType
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclSubXXXWithNameAndType_List'
              xml:space='preserve'>
  <xsl:param name='Name'/><xsl:param name='NativeType'/>
    <xsl:value-of select='$NativeType'/> * list<xsl:value-of select='$Name'/>;
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclSubParameterAccessors template
 -
 - Invoked by template
 -      StructDeclarationCommon
 -
 - Current node
 -      <llrpdef><messageDefinition>
 -      <llrpdef><parameterDefinition>
 -
 - Generate for each parameter (or choice) the member variable
 - and declare accessor functions. This gets tricky:
 -      * Determine if it is a parameter or choice
 -      * Determine the name, either explicit name= or derived from type=
 -      * Determine the kind of repeat (1, 0-1, 0-N, 1-N)
 -      * Make the right kind of member variable (* or std::list)
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclSubParameterAccessors'>
  <xsl:variable name='StructName' select='@name'/>
  <xsl:for-each select='LL:parameter|LL:choice'>
    <xsl:choose>
      <xsl:when test='self::LL:parameter and @type = "Custom"'>
        <xsl:call-template name='StructDeclSubExtensionPointAccessor'>
           <xsl:with-param name='StructName' select='$StructName'/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test='self::LL:parameter'>
        <xsl:call-template name='StructDeclSubParamAccessor'>
           <xsl:with-param name='StructName' select='$StructName'/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test='self::LL:choice'>
        <xsl:call-template name='StructDeclSubChoiceAccessor'>
           <xsl:with-param name='StructName' select='$StructName'/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        HELP: StructDeclSubParameterAccessors <xsl:value-of select='@type'/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:for-each>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclSubParamAccessor template
 -
 - Invoked by template
 -      StructDeclSubParameterAccessors
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclSubParamAccessor'>
  <xsl:param name='StructName'/>
  <xsl:choose>
    <xsl:when test='@name'>
      <xsl:call-template name='StructDeclSubXXXAccessorWithNameAndType'>
        <xsl:with-param name='StructName' select='$StructName'/>
        <xsl:with-param name='Name'><xsl:value-of select='@name'/></xsl:with-param>
        <xsl:with-param name='NativeType'>LLRP_tS<xsl:value-of select='@type'/></xsl:with-param>
        <xsl:with-param name='Repeat'><xsl:value-of select='@repeat'/></xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name='StructDeclSubXXXAccessorWithNameAndType'>
        <xsl:with-param name='StructName' select='$StructName'/>
        <xsl:with-param name='Name'><xsl:value-of select='@type'/></xsl:with-param>
        <xsl:with-param name='NativeType'>LLRP_tS<xsl:value-of select='@type'/></xsl:with-param>
        <xsl:with-param name='Repeat'><xsl:value-of select='@repeat'/></xsl:with-param>
      </xsl:call-template>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclSubChoiceAccessor template
 -
 - Invoked by template
 -      StructDeclSubParameterAccessors
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclSubChoiceAccessor'>
  <xsl:param name='StructName'/>
  <xsl:choose>
    <xsl:when test='@name'>
      <xsl:call-template name='StructDeclSubXXXAccessorWithNameAndType'>
        <xsl:with-param name='StructName' select='$StructName'/>
        <xsl:with-param name='Name'><xsl:value-of select='@name'/></xsl:with-param>
        <xsl:with-param name='NativeType'>LLRP_tSParameter</xsl:with-param>
        <xsl:with-param name='Repeat'><xsl:value-of select='@repeat'/></xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name='StructDeclSubXXXAccessorWithNameAndType'>
        <xsl:with-param name='StructName' select='$StructName'/>
        <xsl:with-param name='Name'><xsl:value-of select='@type'/></xsl:with-param>
        <xsl:with-param name='NativeType'>LLRP_tSParameter</xsl:with-param>
        <xsl:with-param name='Repeat'><xsl:value-of select='@repeat'/></xsl:with-param>
      </xsl:call-template>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclSubExtensionPointAccessor template
 -
 - Invoked by template
 -      StructDeclSubParameterAccessors
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclSubExtensionPointAccessor'>
  <xsl:param name='StructName'/>
  <xsl:call-template name='StructDeclSubXXXAccessorWithNameAndType'>
    <xsl:with-param name='StructName' select='$StructName'/>
    <xsl:with-param name='Name'><xsl:value-of select='@type'/></xsl:with-param>
    <xsl:with-param name='NativeType'>LLRP_tSParameter</xsl:with-param>
    <xsl:with-param name='Repeat'><xsl:value-of select='@repeat'/></xsl:with-param>
  </xsl:call-template>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclSubXXXAccessorWithNameAndType template
 -
 - Invoked by template
 -      StructDeclSubParamAccessor
 -      StructDeclSubChoiceAccessor
 -
 - Common for parameters and choices.
 -
 - @param   Name
 - @param   NativeType
 - @param   Repeat
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclSubXXXAccessorWithNameAndType'>
  <xsl:param name='StructName'/>
  <xsl:param name='Name'/>
  <xsl:param name='NativeType'/>
  <xsl:param name='Repeat'/>
  <xsl:choose>
    <xsl:when test='$Repeat="1" or $Repeat="0-1"'>
      <xsl:call-template name='StructDeclSubXXXAccessorWithNameAndType_Ptr'>
        <xsl:with-param name='StructName' select='$StructName'/>
        <xsl:with-param name='Name'><xsl:value-of select='$Name'/></xsl:with-param>
        <xsl:with-param name='NativeType'><xsl:value-of select='$NativeType'/></xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test='$Repeat="0-N" or $Repeat="1-N"'>
      <xsl:call-template name='StructDeclSubXXXAccessorWithNameAndType_List'>
        <xsl:with-param name='StructName' select='$StructName'/>
        <xsl:with-param name='Name'><xsl:value-of select='$Name'/></xsl:with-param>
        <xsl:with-param name='NativeType'><xsl:value-of select='$NativeType'/></xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise xml:space='preserve'>
    HELP: StructDeclSubXXXAccessorWithNameAndType <xsl:value-of select='$Name'/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclSubXXXAccessorWithNameAndType_Ptr template
 -
 - Invoked by template
 -      StructDeclSubXXXAccessorWithNameAndType
 -
 - Common for parameters and choices.
 -
 - @param   StructName
 - @param   Name
 - @param   NativeType
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclSubXXXAccessorWithNameAndType_Ptr'
              xml:space='preserve'>
  <xsl:param name='StructName'/><xsl:param name='Name'/><xsl:param name='NativeType'/>
extern <xsl:value-of select='$NativeType'/> *
LLRP_<xsl:value-of select='$StructName'/>_get<xsl:value-of select='$Name'/> (
  LLRP_tS<xsl:value-of select='$StructName'/> *pThis);

extern LLRP_tResultCode
LLRP_<xsl:value-of select='$StructName'/>_set<xsl:value-of select='$Name'/> (
  LLRP_tS<xsl:value-of select='$StructName'/> *pThis,
  <xsl:value-of select='$NativeType'/> *pValue);
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclSubXXXAccessorWithNameAndType_List template
 -
 - Invoked by template
 -      StructDeclSubXXXAccessorWithNameAndType
 -
 - Common for parameters and choices.
 -
 - @param   StructName
 - @param   Name
 - @param   NativeType
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclSubXXXAccessorWithNameAndType_List'
              xml:space='preserve'>
  <xsl:param name='StructName'/><xsl:param name='Name'/><xsl:param name='NativeType'/>
extern <xsl:value-of select='$NativeType'/> *
LLRP_<xsl:value-of select='$StructName'/>_begin<xsl:value-of select='$Name'/> (
  LLRP_tS<xsl:value-of select='$StructName'/> *pThis);

extern <xsl:value-of select='$NativeType'/> *
LLRP_<xsl:value-of select='$StructName'/>_next<xsl:value-of select='$Name'/> (
  <xsl:value-of select='$NativeType'/> *pCurrent);

extern void
LLRP_<xsl:value-of select='$StructName'/>_clear<xsl:value-of select='$Name'/> (
  LLRP_tS<xsl:value-of select='$StructName'/> *pThis);

extern LLRP_tResultCode
LLRP_<xsl:value-of select='$StructName'/>_add<xsl:value-of select='$Name'/> (
  LLRP_tS<xsl:value-of select='$StructName'/> *pThis,
  <xsl:value-of select='$NativeType'/> *pValue);

</xsl:template>

<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDeclarationsChoices template
 -
 - Invoked by top level template.
 -
 - Generates declaration of the choice classes.
 - A choice class is really a set of functions and type descriptors.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDeclarationsChoices'>

<xsl:for-each select='LL:choiceDefinition|LL:customChoiceDefinition'>

extern llrp_bool_t
LLRP_<xsl:value-of select='@name'/>_isMember (
  LLRP_tSParameter *            pParameter);

extern const LLRP_tSTypeDescriptor
LLRP_td<xsl:value-of select='@name'/>;
</xsl:for-each>

</xsl:template>


</xsl:stylesheet>
