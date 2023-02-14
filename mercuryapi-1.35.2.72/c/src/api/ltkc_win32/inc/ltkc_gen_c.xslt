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
<xsl:call-template name='VendorDescriptors'/>
<xsl:call-template name='NamespaceDescriptors'/>
<xsl:call-template name='EnumerationStringTablesFields'/>
<xsl:call-template name='StructDefinitionsMessages'/>
<xsl:call-template name='StructDefinitionsParameters'/>
<xsl:call-template name='StructDefinitionsChoices'/>
<xsl:call-template name='GenerateEnrollIntoTypeRegistryFunction'/>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief FileHeader template
 -
 - Invoked by top level template.
 -
 - Current node
 -      n/a - does not matter for this template
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
 * C implementation. It is generated into a .inc file
 * that is included by a platform specific .c source file.
 * That .c file takes care of prerequisites needed by this file.
 */


</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief VendorDescriptors template
 -
 - Invoked by top level template.
 -
 - Current node
 -      <llrpdef>
 -
 -
 - Generates the vendor descriptor for each <vendorDefinition>.
 - Such are at the top level and referenced by custom messages
 - and parameters. The primary purpose of the <vendorDefinition>
 - is to establish a short, programming name and the PEN (private
 - enterprise number).
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='VendorDescriptors'>
/*
 * Vendor descriptors
 */

  <xsl:for-each select='LL:vendorDefinition'>
const LLRP_tSVendorDescriptor
LLRP_vdesc<xsl:value-of select='@name'/> =
{
  "<xsl:value-of select='@name'/>",	// .pName
  <xsl:value-of select='@vendorID'/>,	// .VendorID
};

</xsl:for-each>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief NamespaceDescriptors template
 -
 - Invoked by top level template.
 -
 - Current node
 -      <llrpdef>
 -
 -
 - Generates the namespace descriptor for each <namespaceDefinition>.
 - Such are at the top level and referenced by custom messages
 - and parameters. The primary purpose of the <namespaceDefinition>
 - is to establish the XML namespace used for custom elements.
 - Some LTK implementations use the namespace in a way consistent
 - with their methods. This C implementation does not.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='NamespaceDescriptors'>
/*
 * Namespace descriptors
 */

  <xsl:for-each select='LL:namespaceDefinition'>
const LLRP_tSNamespaceDescriptor
LLRP_nsdesc<xsl:value-of select='@prefix'/> =
{
  "<xsl:value-of select='@prefix'/>",	// .pPrefix
  "<xsl:value-of select='@URI'/>",	// .pURI
  "<xsl:value-of select='@schemaLocation'/>",	// .pSchemaLocation
};

</xsl:for-each>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief EnumerationStringTablesFields template
 -
 - Invoked by top level template.
 -
 - Current node
 -      <llrpdef>
 -
 - Generates the enumeration string tables for LLRP fields.
 - These tables are referenced by field descriptors. They are
 - used to facilitate pretty-printing of enumerated LLRP fields,
 - and for converting from textual representations like XML.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='EnumerationStringTablesFields'>
/*
 * Enumeration string tables
 */

  <xsl:for-each select='LL:enumerationDefinition|LL:customEnumerationDefinition'>
    <xsl:variable name='enumBaseName' select='@name'/>
const LLRP_tSEnumTableEntry
LLRP_est<xsl:value-of select='$enumBaseName'/>[] =
{
  <xsl:for-each select='LL:entry'>
    {
        "<xsl:value-of select='@name'/>",
        LLRP_<xsl:value-of select='$enumBaseName'/>_<xsl:value-of select='@name'/>,
    },</xsl:for-each>
    { 0, 0 }
};

</xsl:for-each>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDefinitionsMessages template
 -
 - Invoked by top level template.
 -
 - Current node
 -      <llrpdef>
 -
 - Generates definitions of the message elements: constructors,
 - destructors, accessors, functions, and reference data
 - including type descriptor and field descriptors.
 -
 - This loops through the message definitions, selects
 - important values, and invokes the StructDefinitionCommon
 - template with the right <xsl:with-param>s.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDefinitionsMessages'>

<xsl:for-each select='LL:messageDefinition'>


/*
*****************************************************************
**
** Message <xsl:value-of select='@name'/>
**
*****************************************************************
*/ 
  <xsl:call-template name='StructDefinitionCommon'>
    <xsl:with-param name='BaseType'>tSMessage</xsl:with-param>
    <xsl:with-param name='LLRPName'><xsl:value-of select='@name'/></xsl:with-param>
    <xsl:with-param name='IsMessage'>TRUE</xsl:with-param>
    <xsl:with-param name='TypeNum'><xsl:value-of select='@typeNum'/></xsl:with-param>
    <xsl:with-param name='pVendorDescriptor'>NULL</xsl:with-param>
    <xsl:with-param name='pNamespaceDescriptor'>&amp;LLRP_nsdescllrp</xsl:with-param>
    <xsl:with-param name='pResponseType'>
      <xsl:choose>
        <xsl:when test='@responseType'>&amp;LLRP_td<xsl:value-of select='@responseType'/></xsl:when>
        <xsl:otherwise>NULL</xsl:otherwise>
      </xsl:choose>
    </xsl:with-param>
    <xsl:with-param name='IsCustomParameter'>false</xsl:with-param>
  </xsl:call-template>
</xsl:for-each>


<xsl:for-each select='LL:customMessageDefinition'>

/*
*****************************************************************
**
** Custom message <xsl:value-of select='@name'/>
**
*****************************************************************
*/ 
  <xsl:call-template name='StructDefinitionCommon'>
    <xsl:with-param name='BaseType'>tSParameter</xsl:with-param>
    <xsl:with-param name='LLRPName'><xsl:value-of select='@name'/></xsl:with-param>
    <xsl:with-param name='IsMessage'>TRUE</xsl:with-param>
    <xsl:with-param name='TypeNum'><xsl:value-of select='@subtype'/></xsl:with-param>
    <xsl:with-param name='pVendorDescriptor'>&amp;LLRP_vdesc<xsl:value-of select='@vendor'/></xsl:with-param>
    <xsl:with-param name='pNamespaceDescriptor'>&amp;LLRP_nsdesc<xsl:value-of select='@namespace'/></xsl:with-param>
    <xsl:with-param name='pResponseType'>
      <xsl:choose>
        <xsl:when test='@responseType'>
          &amp;LLRP_td<xsl:value-of select='@responseType'/>
        </xsl:when>
        <xsl:otherwise>
          NULL
        </xsl:otherwise>
      </xsl:choose>
    </xsl:with-param>
    <xsl:with-param name='IsCustomParameter'>false</xsl:with-param>
  </xsl:call-template>
</xsl:for-each>

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDefinitionsParameters template
 -
 - Invoked by top level template.
 - Current node
 -      <llrpdef>
 -
 - Generates definitions of the parameter elements: constructors,
 - destructors, accessors, functions, and reference data
 - including type descriptor and field descriptors.
 -
 - This loops through the parameter definitions, selects
 - important values, and invokes the StructDefinitionCommon
 - template with the right <xsl:with-param>s.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDefinitionsParameters'>

<xsl:for-each select='LL:parameterDefinition'>


/*
*****************************************************************
**
** Parameter <xsl:value-of select='@name'/>
**
*****************************************************************
*/ 
  <xsl:call-template name='StructDefinitionCommon'>
    <xsl:with-param name='BaseType'>tSParameter</xsl:with-param>
    <xsl:with-param name='LLRPName'><xsl:value-of select='@name'/></xsl:with-param>
    <xsl:with-param name='IsMessage'>FALSE</xsl:with-param>
    <xsl:with-param name='TypeNum'><xsl:value-of select='@typeNum'/></xsl:with-param>
    <xsl:with-param name='pVendorDescriptor'>NULL</xsl:with-param>
    <xsl:with-param name='pNamespaceDescriptor'>&amp;LLRP_nsdescllrp</xsl:with-param>
    <xsl:with-param name='pResponseType'>NULL</xsl:with-param>
    <xsl:with-param name='IsCustomParameter'>false</xsl:with-param>
  </xsl:call-template>
</xsl:for-each>

<xsl:for-each select='LL:customParameterDefinition'>


/*
*****************************************************************
**
** Custom parameter <xsl:value-of select='@name'/>
**
*****************************************************************
*/ 
  <xsl:call-template name='StructDefinitionCommon'>
    <xsl:with-param name='BaseType'>tSParameter</xsl:with-param>
    <xsl:with-param name='LLRPName'><xsl:value-of select='@name'/></xsl:with-param>
    <xsl:with-param name='IsMessage'>FALSE</xsl:with-param>
    <xsl:with-param name='TypeNum'><xsl:value-of select='@subtype'/></xsl:with-param>
    <xsl:with-param name='pVendorDescriptor'>&amp;LLRP_vdesc<xsl:value-of select='@vendor'/></xsl:with-param>
    <xsl:with-param name='pNamespaceDescriptor'>&amp;LLRP_nsdesc<xsl:value-of select='@namespace'/></xsl:with-param>
    <xsl:with-param name='pResponseType'>NULL</xsl:with-param>
    <xsl:with-param name='IsCustomParameter'>true</xsl:with-param>
  </xsl:call-template>

llrp_bool_t
LLRP_<xsl:value-of select='@name'/>_isAllowedIn (
  const LLRP_tSTypeDescriptor *pEnclosingElementType)
{
  <xsl:for-each select='LL:allowedIn'>
    if(pEnclosingElementType == &amp;LLRP_td<xsl:value-of select='@type'/>)
    {
        return TRUE;
    }
  </xsl:for-each>
    return FALSE;
}
</xsl:for-each>

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDefinitionCommon template
 -
 - Invoked by templates
 -      StructDefinitionsMessages
 -      StructDefinitionsParameters
 -
 - Current node
 -      <llrpdef><messageDefinition>
 -      <llrpdef><parameterDefinition>
 -
 - @param   BaseType        Name of the base type, LLRP_tSMessage
 -                          or LLRP_tSParameter
 - @param   LLRPName        The original, LLRP name for the element
 - @param   IsMessage       Either TRUE or FALSE
 - @param   TypeNum         Either an enumerated symbol
 -                          (e.g. LLRP_PT_xxx) or an integer
 - @param   pVendorDescriptor The pointer expression for this parameters
 -                          vendor descriptor and, thereby, PEN for
 -                          custom elements (messages/parameters).
 -                          Usually NULL for standard LLRP elements.
 - @param   pNamespaceDescriptor The pointer expression for this parameters
 -                          namespace descriptor. Usually LLRP_nsdescllrp
 -                          for standard parameters and messages.
 - @param   pResponseType   The pointer expression for this messages
 -                          response counterpart.
 - @param   IsCustomParameter Either "true" or "false".
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDefinitionCommon'>
  <xsl:param name='BaseType'/>
  <xsl:param name='LLRPName'/>
  <xsl:param name='IsMessage'/>
  <xsl:param name='TypeNum'/>
  <xsl:param name='pVendorDescriptor'/>
  <xsl:param name='pNamespaceDescriptor'/>
  <xsl:param name='pResponseType'/>
  <xsl:param name='IsCustomParameter'/>

  <xsl:call-template name='StructDefnTypeDescriptor'>
    <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
    <xsl:with-param name='IsMessage'><xsl:value-of select='$IsMessage'/></xsl:with-param>
    <xsl:with-param name='TypeNum'><xsl:value-of select='$TypeNum'/></xsl:with-param>
    <xsl:with-param name='pVendorDescriptor'><xsl:value-of select='$pVendorDescriptor'/></xsl:with-param>
    <xsl:with-param name='pNamespaceDescriptor'><xsl:value-of select='$pNamespaceDescriptor'/></xsl:with-param>
    <xsl:with-param name='pResponseType'><xsl:value-of select='$pResponseType'/></xsl:with-param>
    <xsl:with-param name='IsCustomParameter'><xsl:value-of select='$IsCustomParameter'/></xsl:with-param>
  </xsl:call-template>

  <xsl:call-template name='StructDefnFieldDescriptorTable'>
    <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
  </xsl:call-template>

  <xsl:call-template name='StructDefnFieldDescriptors'>
    <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
  </xsl:call-template>

  <xsl:call-template name='StructConstructFunction'>
    <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
  </xsl:call-template>

  <xsl:call-template name='StructDestructFunction'>
    <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
  </xsl:call-template>

  <xsl:call-template name='StructDecodeFieldsFunction'>
    <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
  </xsl:call-template>

  <xsl:call-template name='AssimilateSubParametersFunction'>
    <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
  </xsl:call-template>

  <xsl:call-template name='FieldAccessorFunctions'>
    <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
  </xsl:call-template>

  <xsl:call-template name='SubParameterAccessorFunctions'>
    <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
  </xsl:call-template>

  <xsl:call-template name='EncodeFunction'>
    <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
  </xsl:call-template>


</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDefinitionsChoices template
 -
 - Invoked by top level template.
 - Current node
 -      <llrpdef>
 -
 - Generates definitions of the choice elements: isMember,
 - type descriptor, etc.
 -
 - This loops through the choice definitions, selects
 - important values, and invokes the StructDefinitionCommon
 - template with the right <xsl:with-param>s.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDefinitionsChoices'>

<xsl:for-each select='LL:choiceDefinition|LL:customChoiceDefinition'>


/*
*****************************************************************
**
** Choice <xsl:value-of select='@name'/>
**
*****************************************************************
*/ 
  <xsl:call-template name='StructDefinitionOneChoice'>
    <xsl:with-param name='LLRPName'><xsl:value-of select='@name'/></xsl:with-param>
    <xsl:with-param name='pVendorDescriptor'>NULL</xsl:with-param>
  </xsl:call-template>
</xsl:for-each>

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDefinitionOneChoice template
 -
 - Invoked by templates
 -      StructDefinitionsChoices
 -
 - Current node
 -      <llrpdef><choiceDefinition>
 -
 - @param   LLRPName        The original, LLRP name for the element
 - @param   pVendorDescriptor The pointer expression for this parameters
 -                          vendor descriptor and, thereby, PEN for
 -                          custom elements (messages/parameters).
 -                          Usually NULL for stanrd LLRP elements.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDefinitionOneChoice'>
  <xsl:param name='LLRPName'/>
  <xsl:param name='pVendorDescriptor'/>

const LLRP_tSTypeDescriptor
LLRP_td<xsl:value-of select='$LLRPName'/> =
{
    FALSE,      //.bIsMessage
    "<xsl:value-of select='$LLRPName'/>",       //.pName
    <xsl:value-of select='$pVendorDescriptor'/>,        //.pVendorDescriptor
    NULL,       //.pNamespaceDescriptor
    -1, //.TypeNum
    NULL,       //.ppFieldDescriptorTable
    0,  //.nSizeBytes
    NULL,       //.pfConstruct
    NULL,       //.pfDestruct
    NULL,       //.pfDecodeFields
    NULL,       //.pfAssimilateSubParameters
    NULL,       //.pfEncode
    NULL,       //.pfIsAllowedIn
};

llrp_bool_t
LLRP_<xsl:value-of select='$LLRPName'/>_isMember (
  LLRP_tSParameter *            pParameter)
{
    const LLRP_tSTypeDescriptor *pType;

    pType = pParameter-&gt;elementHdr.pType;

  <xsl:for-each select='LL:parameter'>
    <xsl:choose>
      <xsl:when test='@type = "Custom"'>
    if(LLRP_Parameter_isAllowedIn(pParameter, &amp;LLRP_td<xsl:value-of select='$LLRPName'/>))
    {
        return TRUE;
    }
      </xsl:when>
      <xsl:otherwise>
    if(&amp;LLRP_td<xsl:value-of select='@type'/> == pType)
    {
        return TRUE;
    }
      </xsl:otherwise>
    </xsl:choose>
</xsl:for-each>

    return FALSE;
}

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDefnTypeDescriptor template
 -
 - Invoked by templates
 -      StructDefinitionCommon
 -
 - Current node
 -      <llrpdef><messageDefinition>
 -      <llrpdef><parameterDefinition>
 -
 - @param   LLRPName        The original, LLRP name for the element
 - @param   IsMessage       Either TRUE or FALSE
 - @param   TypeNum         Either an enumerated symbol
 -                          (e.g. LLRP_PT_xxx) or an integer
 - @param   pVendorDescriptor The pointer expression for this parameters
 -                          vendor descriptor and, thereby, PEN for
 -                          custom elements (messages/parameters).
 -                          Usually NULL for standard LLRP elements.
 - @param   pNamespaceDescriptor The pointer expression for this parameters
 -                          namespace descriptor. Usually LLRP_nsdescllrp
 -                          for standard parameters and messages.
 - @param   pResponseType   The pointer expression for this messages
 -                          response counterpart.
 - @param   IsCustomParameter Either "true" or "false".
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDefnTypeDescriptor'>
  <xsl:param name='LLRPName'/>
  <xsl:param name='IsMessage'/>
  <xsl:param name='TypeNum'/>
  <xsl:param name='pVendorDescriptor'/>
  <xsl:param name='pNamespaceDescriptor'/>
  <xsl:param name='pResponseType'/>
  <xsl:param name='IsCustomParameter'/>

const LLRP_tSTypeDescriptor
LLRP_td<xsl:value-of select='$LLRPName'/> =
{
    <xsl:value-of select='$IsMessage'/>,	// .bIsMessage
    "<xsl:value-of select='$LLRPName'/>",	// .pName
    <xsl:value-of select='$pVendorDescriptor'/>,	// .pVendorDescriptor
    <xsl:value-of select='$pNamespaceDescriptor'/>,	// .pNamespaceDescriptor
    <xsl:value-of select='$TypeNum'/>,	// .TypeNum
    <xsl:value-of select='$pResponseType'/>,	// .pResponseType
    LLRP_apfd<xsl:value-of select='$LLRPName'/>,	// .ppFieldDescriptorTable
    sizeof(LLRP_tS<xsl:value-of select='$LLRPName'/>),	// .nSizeBytes
    (LLRP_tSElement *(*)(void)) LLRP_<xsl:value-of select='$LLRPName'/>_construct,	// .pfConstruct
    (void (*)(LLRP_tSElement *)) LLRP_<xsl:value-of select='$LLRPName'/>_destruct,	// .pfDestruct

    // .pfDecodeFields
        (void (*)(LLRP_tSElement *, LLRP_tSDecoderStream *))
            LLRP_<xsl:value-of select='$LLRPName'/>_decodeFields,

    // .pfAssimilateSubParameters
        (void (*)(LLRP_tSElement *, LLRP_tSErrorDetails *))
            LLRP_<xsl:value-of select='$LLRPName'/>_assimilateSubParameters,

    // .pfEncode
        (void (*)(const LLRP_tSElement *, LLRP_tSEncoderStream *))
            LLRP_<xsl:value-of select='$LLRPName'/>_encode,

  <xsl:choose>
    <xsl:when test='$IsCustomParameter = "true"'>
    LLRP_<xsl:value-of select='$LLRPName'/>_isAllowedIn,	// .pfIsAllowedIn
    </xsl:when>
    <xsl:otherwise>
    NULL,	// .pfIsAllowedIn
    </xsl:otherwise>
  </xsl:choose>
};

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDefnFieldDescriptorTable template
 -
 - Invoked by templates
 -      StructDefinitionCommon
 -
 - Current node
 -      <llrpdef><messageDefinition>
 -      <llrpdef><parameterDefinition>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDefnFieldDescriptorTable'>
  <xsl:param name='LLRPName'/>

const LLRP_tSFieldDescriptor *
LLRP_apfd<xsl:value-of select='$LLRPName'/>[] =
{
  <xsl:for-each select='LL:field'>
    &amp;LLRP_fd<xsl:value-of select='$LLRPName'/>_<xsl:value-of select='@name'/>,</xsl:for-each>
    NULL
};

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDefnFieldDescriptors template
 -
 - Invoked by templates
 -      StructDefinitionCommon
 -
 - Current node
 -      <llrpdef><messageDefinition>
 -      <llrpdef><parameterDefinition>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDefnFieldDescriptors'>
  <xsl:param name='LLRPName'/>
  <xsl:for-each select='LL:field'>
    <xsl:choose>
      <xsl:when test='@enumeration'>
        <xsl:call-template name='StructDefnFieldDescEnum'>
          <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
          <xsl:with-param name='FieldLLRPName'><xsl:value-of select='@name'/></xsl:with-param>
          <xsl:with-param name='FieldBaseType'><xsl:value-of select='@type'/></xsl:with-param>
          <xsl:with-param name='Enumeration'><xsl:value-of select='@enumeration'/></xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name='StructDefnFieldDescPlain'>
          <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
          <xsl:with-param name='FieldLLRPName'><xsl:value-of select='@name'/></xsl:with-param>
          <xsl:with-param name='FieldBaseType'><xsl:value-of select='@type'/></xsl:with-param>
          <xsl:with-param name='FieldFormat'>
            <xsl:choose>
              <xsl:when test='@format'>
                <xsl:value-of select='@format'/>
              </xsl:when>
              <xsl:otherwise>Normal</xsl:otherwise>
            </xsl:choose>
          </xsl:with-param>
        </xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:for-each>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDefnFieldDescEnum template
 -
 - Invoked by templates
 -      StructDefnFieldDescriptor
 -
 - Current node
 -      <llrpdef><messageDefinition><field enumeration=...>
 -      <llrpdef><parameterDefinition><field enumeration=...>
 -
 - This determines the field type (LLRP_FT_Exx) then invokes template
 - StructDefnFieldDescCommon with appropriate <xsl:with-param>'s
 -
 - In all cases:
 -      The derived MemberName is "e$FieldLLRPName", note the "e" for enum
 -      The derived EnumStrTable is "LLRP_est$Enumeration"
 -      $LLRPName and $FieldLLRPName are passed unchanged
 -
 - Each case determines the FieldEnumType that is mapped
 - u1=>LLRP_FT_E1, u2=>LLRP_FT_E2, u8=>LLRP_FT_E8,
 - u16=>LLRP_FT_E16, u32=>LLRP_FT_E32
 -
 - @param   LLRPName        The original, LLRP name for the element
 - @param   FieldLLRPName   The original, LLRP name for the field
 - @param   FieldBaseType   The type, for enums it is one of
 -                          u1, u2, u8, u16, or u32
 - @param   Enumeration     The enumeration base name
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDefnFieldDescEnum'>
  <xsl:param name='LLRPName'/>
  <xsl:param name='FieldLLRPName'/>
  <xsl:param name='FieldBaseType'/>
  <xsl:param name='Enumeration'/>
  <xsl:variable name='FieldEnumType'>
    <xsl:choose>
      <xsl:when test='$FieldBaseType="u1"'  >LLRP_FT_E1</xsl:when>
      <xsl:when test='$FieldBaseType="u2"'  >LLRP_FT_E2</xsl:when>
      <xsl:when test='$FieldBaseType="u8"'  >LLRP_FT_E8</xsl:when>
      <xsl:when test='$FieldBaseType="u16"' >LLRP_FT_E16</xsl:when>
      <xsl:when test='$FieldBaseType="u32"' >LLRP_FT_E32</xsl:when>
      <xsl:when test='$FieldBaseType="u8v"' >LLRP_FT_E8V</xsl:when>
      <xsl:otherwise>FT_EBOGOSITY</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:call-template name='StructDefnFieldDescCommon'>
    <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
    <xsl:with-param name='MemberName'>m_e<xsl:value-of select='$FieldLLRPName'/></xsl:with-param>
    <xsl:with-param name='FieldLLRPName'><xsl:value-of select='$FieldLLRPName'/></xsl:with-param>
    <xsl:with-param name='FieldEnumType'><xsl:value-of select='$FieldEnumType'/></xsl:with-param>
    <xsl:with-param name='FieldEnumFormat'>LLRP_FMT_NORMAL</xsl:with-param>
    <xsl:with-param name='EnumStrTable'>LLRP_est<xsl:value-of select='$Enumeration'/></xsl:with-param>
  </xsl:call-template>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDefnFieldDescPlain template
 -
 - Invoked by templates
 -      StructDefnFieldDescriptor
 -
 - Current node
 -      <llrpdef><messageDefinition><field>
 -      <llrpdef><parameterDefinition><field>
 -
 - This determines the field type (LLRP_FT_xx) then invokes template
 - StructDefnFieldDescCommon with appropriate <xsl:with-param>'s
 -
 - The derived MemberName is "$FieldLLRPName"
 - The EnumStrTable is NULL
 - $LLRPName and $FieldLLRPName are passed unchanged
 - The FieldEnumType is derived by mapping $FieldBaseType (@type)
 - The FieldEnumFormat is derived by mapping $FieldFormat (@format)
 -
 - @param   LLRPName        The original, LLRP name for the element
 - @param   FieldLLRPName   The original, LLRP name for the field
 - @param   FieldBaseType   The type, something like {u,s}{8,16,32,64}[v]
 - @param   FieldFormat     The format, something like Normal, Hex, Datetime
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDefnFieldDescPlain'>
  <xsl:param name='LLRPName'/>
  <xsl:param name='FieldLLRPName'/>
  <xsl:param name='FieldBaseType'/>
  <xsl:param name='FieldFormat'/>
  <xsl:variable name='FieldEnumType'>
    <xsl:choose>
      <xsl:when test='$FieldBaseType="u8"'  >LLRP_FT_U8</xsl:when>
      <xsl:when test='$FieldBaseType="s8"'  >LLRP_FT_S8</xsl:when>
      <xsl:when test='$FieldBaseType="u8v"' >LLRP_FT_U8V</xsl:when>
      <xsl:when test='$FieldBaseType="s8v"' >LLRP_FT_S8V</xsl:when>
      <xsl:when test='$FieldBaseType="u16"' >LLRP_FT_U16</xsl:when>
      <xsl:when test='$FieldBaseType="s16"' >LLRP_FT_S16</xsl:when>
      <xsl:when test='$FieldBaseType="u16v"'>LLRP_FT_U16V</xsl:when>
      <xsl:when test='$FieldBaseType="s16v"'>LLRP_FT_S16V</xsl:when>
      <xsl:when test='$FieldBaseType="u32"' >LLRP_FT_U32</xsl:when>
      <xsl:when test='$FieldBaseType="s32"' >LLRP_FT_S32</xsl:when>
      <xsl:when test='$FieldBaseType="u32v"'>LLRP_FT_U32V</xsl:when>
      <xsl:when test='$FieldBaseType="s32v"'>LLRP_FT_S32V</xsl:when>
      <xsl:when test='$FieldBaseType="u64"' >LLRP_FT_U64</xsl:when>
      <xsl:when test='$FieldBaseType="s64"' >LLRP_FT_S64</xsl:when>
      <xsl:when test='$FieldBaseType="u64v"'>LLRP_FT_U64V</xsl:when>
      <xsl:when test='$FieldBaseType="s64v"'>LLRP_FT_S64V</xsl:when>
      <xsl:when test='$FieldBaseType="u1"'  >LLRP_FT_U1</xsl:when>
      <xsl:when test='$FieldBaseType="u2"'  >LLRP_FT_U2</xsl:when>
      <xsl:when test='$FieldBaseType="u1v"' >LLRP_FT_U1V</xsl:when>
      <xsl:when test='$FieldBaseType="u96"' >LLRP_FT_U96</xsl:when>
      <xsl:when test='$FieldBaseType="utf8v"' >LLRP_FT_UTF8V</xsl:when>
      <xsl:when test='$FieldBaseType="bytesToEnd"' >LLRP_FT_BYTESTOEND</xsl:when>
      <xsl:otherwise>LLRP_FT_BOGOSITY</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name='FieldEnumFormat'>
    <xsl:choose>
      <xsl:when test='$FieldFormat="Normal"'  >LLRP_FMT_NORMAL</xsl:when>
      <xsl:when test='$FieldFormat="Dec"'     >LLRP_FMT_DEC</xsl:when>
      <xsl:when test='$FieldFormat="Hex"'     >LLRP_FMT_HEX</xsl:when>
      <xsl:when test='$FieldFormat="Datetime"'>LLRP_FMT_DATETIME</xsl:when>
      <xsl:when test='$FieldFormat="UTF8"'    >LLRP_FMT_UTF8</xsl:when>
      <xsl:otherwise>LLRP_FMT_BOGOSITY</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:call-template name='StructDefnFieldDescCommon'>
    <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
    <xsl:with-param name='MemberName'>m_<xsl:value-of select='$FieldLLRPName'/></xsl:with-param>
    <xsl:with-param name='FieldLLRPName'><xsl:value-of select='$FieldLLRPName'/></xsl:with-param>
    <xsl:with-param name='FieldEnumType'><xsl:value-of select='$FieldEnumType'/></xsl:with-param>
    <xsl:with-param name='FieldEnumFormat'><xsl:value-of select='$FieldEnumFormat'/></xsl:with-param>
    <xsl:with-param name='EnumStrTable'>NULL</xsl:with-param>
  </xsl:call-template>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDefnFieldDescCommon template
 -
 - Invoked by templates
 -      StructDefnFieldDescEnum
 -      StructDefnFieldDescPlain
 -
 - Current node
 -      <llrpdef><messageDefinition><field>
 -      <llrpdef><parameterDefinition><field>
 -
 - @param   LLRPName        The original, LLRP name for the element
 - @param   MemberName      The name of the structure variable,
 -                          Something or eSomething
 - @param   FieldLLRPName   The original, LLRP name for the field
 - @param   FieldEnumType   The enum symbol from LLRP_tEFieldType
 - @param   FieldEnumFormat The enum symbol from LLRP_tEFieldFormat
 - @param   EnumStrTable    The initializer for the enum string table
 -                          base pointer
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDefnFieldDescCommon'>
  <xsl:param name='LLRPName'/>
  <xsl:param name='MemberName'/>
  <xsl:param name='FieldLLRPName'/>
  <xsl:param name='FieldEnumType'/>
  <xsl:param name='FieldEnumFormat'/>
  <xsl:param name='EnumStrTable'/>

const LLRP_tSFieldDescriptor
LLRP_fd<xsl:value-of select='$LLRPName'/>_<xsl:value-of select='$FieldLLRPName'/> =
{
    <xsl:value-of select='$FieldEnumType'/>,	// .eFieldType
    <xsl:value-of select='$FieldEnumFormat'/>,	// .eFieldFormat
    "<xsl:value-of select='$FieldLLRPName'/>",	// .pName
    <xsl:value-of select='$EnumStrTable'/>,	// .pEnumTable
};
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructConstructFunction template
 -
 - Invoked by templates
 -      StructDefinitionCommon
 -
 - Current node
 -      <llrpdef><messageDefinition>
 -      <llrpdef><parameterDefinition>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructConstructFunction'>
  <xsl:param name='LLRPName'/>
LLRP_tS<xsl:value-of select='$LLRPName'/> *
LLRP_<xsl:value-of select='$LLRPName'/>_construct (void)
{
    LLRP_tS<xsl:value-of select='$LLRPName'/> *pNew;

    pNew = (LLRP_tS<xsl:value-of select='$LLRPName'/> *)
        LLRP_Element_construct(&amp;LLRP_td<xsl:value-of select='$LLRPName'/>);

    return pNew;
}
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDestructFunction template
 -
 - Invoked by templates
 -      StructDefinitionCommon
 -
 - Current node
 -      <llrpdef><messageDefinition>
 -      <llrpdef><parameterDefinition>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDestructFunction'>
  <xsl:param name='LLRPName'/>
void
LLRP_<xsl:value-of select='$LLRPName'/>_destruct (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis)
{
  <xsl:for-each select='LL:field'>
    <xsl:choose>
      <xsl:when test='@type = "u8v"  or @type = "s8v"  or
                      @type = "u16v" or @type = "s16v" or
                      @type = "u32v" or @type = "s32v" or
                      @type = "u64v" or @type = "s64v" or
                      @type = "u1v"  or @type = "utf8v" or
                      @type = "bytesToEnd"'>
    LLRP_<xsl:value-of select='@type'/>_clear(&amp;pThis-&gt;<xsl:value-of select='@name'/>);
      </xsl:when>
    </xsl:choose>
  </xsl:for-each>

    LLRP_Element_finalDestruct((LLRP_tSElement *) pThis);
}
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief StructDecodeFieldsFunction template
 -
 - Invoked by templates
 -      StructDefinitionCommon
 -
 - Current node
 -      <llrpdef><messageDefinition>
 -      <llrpdef><parameterDefinition>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='StructDecodeFieldsFunction'>
  <xsl:param name='LLRPName'/>
void
LLRP_<xsl:value-of select='$LLRPName'/>_decodeFields (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis,
  LLRP_tSDecoderStream *        pDecoderStream)
{
    LLRP_tSDecoderStreamOps *   pOps;

    pOps = pDecoderStream-&gt;pDecoderStreamOps;

  <xsl:for-each select='LL:field|LL:reserved'>
    <xsl:choose>
      <xsl:when test='self::LL:field'>
        <xsl:call-template name='DecodeOneField'>
          <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test='self::LL:reserved'>
        <xsl:call-template name='DecodeOneReserved'>
          <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
        </xsl:call-template>
      </xsl:when>
    </xsl:choose>
  </xsl:for-each>
}
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief DecodeOneField template
 -
 - Invoked by templates
 -      StructDecodeFieldsFunction
 -
 - Current node
 -      <llrpdef><messageDefinition><field>
 -      <llrpdef><parameterDefinition><field>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='DecodeOneField'>
  <xsl:param name='LLRPName'/>
  <xsl:variable name='FieldDesc'>&amp;LLRP_fd<xsl:value-of select='$LLRPName'/>_<xsl:value-of select='@name'/></xsl:variable>
  <xsl:choose>
    <xsl:when test='@enumeration and @type = "u8v"'>
    if(NULL != pThis)
    {
        pThis-&gt;<xsl:value-of select='@name'/> =
                pOps-&gt;pfGet_e8v(pDecoderStream,
                        <xsl:value-of select='$FieldDesc'/>);
    }
    else
    {
        pOps-&gt;pfGet_e8v(pDecoderStream,
                <xsl:value-of select='$FieldDesc'/>);
    }
    </xsl:when>
    <xsl:when test='@enumeration'>
      <xsl:call-template name='DecodeOneFieldEnum'>
        <xsl:with-param name='LLRPName'>
          <xsl:value-of select='$LLRPName'/>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
    if(NULL != pThis)
    {
        pThis-&gt;<xsl:value-of select='@name'/> =
                pOps-&gt;pfGet_<xsl:value-of select='@type'/>(pDecoderStream,
                        <xsl:value-of select='$FieldDesc'/>);
    }
    else
    {
        pOps-&gt;pfGet_<xsl:value-of select='@type'/>(pDecoderStream,
                <xsl:value-of select='$FieldDesc'/>);
    }
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief DecodeOneFieldEnum template
 -
 - Invoked by templates
 -      DecodeOneField
 -
 - Current node
 -      <llrpdef><messageDefinition><field enumeration=...>
 -      <llrpdef><parameterDefinition><field enumeration=...>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='DecodeOneFieldEnum'>
  <xsl:param name='LLRPName'/>
  <xsl:variable name='FieldDesc'>&amp;LLRP_fd<xsl:value-of select='$LLRPName'/>_<xsl:value-of select='@name'/></xsl:variable>
  <xsl:variable name='eType'>
    <xsl:choose>
      <xsl:when test='@type="u1"'>e1</xsl:when>
      <xsl:when test='@type="u2"'>e2</xsl:when>
      <xsl:when test='@type="u8"'>e8</xsl:when>
      <xsl:when test='@type="u16"'>e16</xsl:when>
      <xsl:when test='@type="u32"'>e32</xsl:when>
      <xsl:when test='@type="u8v"'>e8v</xsl:when>
      <xsl:otherwise>ebogus</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
    if(NULL != pThis)
    {
        pThis-&gt;e<xsl:value-of select='@name'/> =
                (LLRP_tE<xsl:value-of select='@enumeration'/>) pOps-&gt;pfGet_<xsl:value-of select='$eType'/>(pDecoderStream,
                        <xsl:value-of select='$FieldDesc'/>);
    }
    else
    {
        pOps-&gt;pfGet_<xsl:value-of select='$eType'/>(pDecoderStream,
                <xsl:value-of select='$FieldDesc'/>);
    }
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief DecodeOneReserved template
 -
 - Invoked by templates
 -      StructDecodeFieldsFunction
 -
 - Current node
 -      <llrpdef><messageDefinition><reserved>
 -      <llrpdef><parameterDefinition><reserved>
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='DecodeOneReserved'>
    pOps-&gt;pfGet_reserved(pDecoderStream, <xsl:value-of select='@bitCount'/>);
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief AssimilateSubParametersFunction template
 -
 - Invoked by templates
 -      StructDefinitionCommon
 -
 - Current node
 -      <llrpdef><messageDefinition>
 -      <llrpdef><parameterDefinition>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='AssimilateSubParametersFunction'>
  <xsl:param name='LLRPName'/>
void
LLRP_<xsl:value-of select='$LLRPName'/>_assimilateSubParameters (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis,
  LLRP_tSErrorDetails *         pError)
{
    LLRP_tSParameter *              pCur;
    const LLRP_tSTypeDescriptor *   pType;

    pCur = pThis-&gt;hdr.elementHdr.listAllSubParameters;

  <xsl:for-each select='LL:parameter|LL:choice'>
    <xsl:choose>
      <xsl:when test='self::LL:parameter and @type = "Custom"'>
        <xsl:call-template name='AssimSubParamExtension'/>
      </xsl:when>
      <xsl:when test='self::LL:parameter'>
        <xsl:call-template name='AssimSubParamParameter'/>
      </xsl:when>
      <xsl:when test='self::LL:choice'>
        <xsl:call-template name='AssimSubParamChoice'/>
      </xsl:when>
      <xsl:otherwise>
    HELP -- parameter/choice <xsl:value-of select='@type'/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:for-each>

    if(NULL != pCur)
    {
        LLRP_Error_unexpectedParameter(pError, pCur);
    }

    return;

  missing:
    LLRP_Error_missingParameter(pError, pType);
}
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief AssimSubParamParameter template
 -
 - Invoked by templates
 -      AssimilateSubParametersFunction
 -
 - Current node
 -      <llrpdef><messageDefinition><parameter>
 -      <llrpdef><parameterDefinition><parameter>
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='AssimSubParamParameter'>
  <xsl:variable name='MemberBaseName'>
    <xsl:choose>
      <xsl:when test='@name'><xsl:value-of select='@name'/></xsl:when>
      <xsl:otherwise><xsl:value-of select='@type'/></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name='ParamType'><xsl:value-of select='@type'/></xsl:variable>

    // <xsl:value-of select='@repeat'/> of <xsl:value-of select='$MemberBaseName'/>
    pType = &amp;LLRP_td<xsl:value-of select='$ParamType'/>;
  <xsl:choose>
    <xsl:when test='@repeat="1"'>
    if(NULL == pCur || pCur-&gt;elementHdr.pType != pType)
    {
        goto missing;
    }
    pThis-&gt;p<xsl:value-of select='$MemberBaseName'/> = (LLRP_tS<xsl:value-of select='$ParamType'/> *) pCur;
    pCur = pCur-&gt;pNextAllSubParameters;
    </xsl:when>
    <xsl:when test='@repeat="0-1"'>
    if(NULL != pCur &amp;&amp; pCur-&gt;elementHdr.pType == pType)
    {
        pThis-&gt;p<xsl:value-of select='$MemberBaseName'/> = (LLRP_tS<xsl:value-of select='$ParamType'/> *) pCur;
        pCur = pCur-&gt;pNextAllSubParameters;
    }
    </xsl:when>
    <xsl:when test='@repeat="0-N"'>
    while(NULL != pCur &amp;&amp; pCur-&gt;elementHdr.pType == pType)
    {
        SUBPARAM_ATTACH(list<xsl:value-of select='$MemberBaseName'/>, pCur);
        pCur = pCur-&gt;pNextAllSubParameters;
    }
    </xsl:when>
    <xsl:when test='@repeat="1-N"'>
    if(NULL == pCur || pCur-&gt;elementHdr.pType != pType)
    {
        goto missing;
    }
    while(NULL != pCur &amp;&amp; pCur-&gt;elementHdr.pType == pType)
    {
        SUBPARAM_ATTACH(list<xsl:value-of select='$MemberBaseName'/>, pCur);
        pCur = pCur-&gt;pNextAllSubParameters;
    }
    </xsl:when>
    <xsl:otherwise>
    HELP -- parameter/choice <xsl:value-of select='@type'/> <xsl:value-of select='@repeat'/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief AssimSubParamChoice template
 -
 - Invoked by templates
 -      AssimilateSubParametersFunction
 -
 - Current node
 -      <llrpdef><messageDefinition><choice>
 -      <llrpdef><parameterDefinition><choice>
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='AssimSubParamChoice'>
  <xsl:variable name='MemberBaseName'>
    <xsl:choose>
      <xsl:when test='@name'><xsl:value-of select='@name'/></xsl:when>
      <xsl:otherwise><xsl:value-of select='@type'/></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name='isMember'>LLRP_<xsl:value-of select='@type'/>_isMember(pCur)</xsl:variable>

    // <xsl:value-of select='@repeat'/> of choice <xsl:value-of select='$MemberBaseName'/>
    pType = NULL;
  <xsl:choose>
    <xsl:when test='@repeat="1"'>
    if(NULL == pCur || !<xsl:value-of select='$isMember'/>)
    {
        goto missing;
    }
    pThis-&gt;p<xsl:value-of select='$MemberBaseName'/> = pCur;
    pCur = pCur-&gt;pNextAllSubParameters;
    </xsl:when>
    <xsl:when test='@repeat="0-1"'>
    if(NULL != pCur &amp;&amp; <xsl:value-of select='$isMember'/>)
    {
        pThis-&gt;p<xsl:value-of select='$MemberBaseName'/> = pCur;
        pCur = pCur-&gt;pNextAllSubParameters;
    }
    </xsl:when>
    <xsl:when test='@repeat="0-N"'>
    while(NULL != pCur &amp;&amp; <xsl:value-of select='$isMember'/>)
    {
        SUBPARAM_ATTACH(list<xsl:value-of select='$MemberBaseName'/>, pCur);
        pCur = pCur-&gt;pNextAllSubParameters;
    }
    </xsl:when>
    <xsl:when test='@repeat="1-N"'>
    if(NULL == pCur || !<xsl:value-of select='$isMember'/>)
    {
        goto missing;
    }
    while(NULL != pCur &amp;&amp; <xsl:value-of select='$isMember'/>)
    {
        SUBPARAM_ATTACH(list<xsl:value-of select='$MemberBaseName'/>, pCur);
        pCur = pCur-&gt;pNextAllSubParameters;
    }
    </xsl:when>
    <xsl:otherwise>
    HELP -- parameter/choice <xsl:value-of select='@type'/> <xsl:value-of select='@repeat'/>
    </xsl:otherwise>
  </xsl:choose>

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief AssimSubParamExtension template
 -
 - Invoked by templates
 -      AssimilateSubParametersFunction
 -
 - Current node
 -      <llrpdef><messageDefinition><parameter type='Custom'>
 -      <llrpdef><parameterDefinition><parameter type='Custom'>
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='AssimSubParamExtension'>
  <xsl:variable name='MemberBaseName'>
    <xsl:choose>
      <xsl:when test='@name'><xsl:value-of select='@name'/></xsl:when>
      <xsl:otherwise><xsl:value-of select='@type'/></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name='isAllowed'>LLRP_Parameter_isAllowedExtension(pCur, &amp;LLRP_td<xsl:value-of select='../@name'/>)</xsl:variable>

    // <xsl:value-of select='@repeat'/> of choice <xsl:value-of select='$MemberBaseName'/>
    pType = NULL;
  <xsl:choose>
    <xsl:when test='@repeat="1"'>
    if(NULL == pCur || !<xsl:value-of select='$isAllowed'/>)
    {
        goto missing;
    }
    pThis-&gt;p<xsl:value-of select='$MemberBaseName'/> = pCur;
    pCur = pCur-&gt;pNextAllSubParameters;
    </xsl:when>
    <xsl:when test='@repeat="0-1"'>
    if(NULL != pCur &amp;&amp; <xsl:value-of select='$isAllowed'/>)
    {
        pThis-&gt;p<xsl:value-of select='$MemberBaseName'/> = pCur;
        pCur = pCur-&gt;pNextAllSubParameters;
    }
    </xsl:when>
    <xsl:when test='@repeat="0-N"'>
    while(NULL != pCur &amp;&amp; <xsl:value-of select='$isAllowed'/>)
    {
        SUBPARAM_ATTACH(list<xsl:value-of select='$MemberBaseName'/>, pCur);
        pCur = pCur-&gt;pNextAllSubParameters;
    }
    </xsl:when>
    <xsl:when test='@repeat="1-N"'>
    if(NULL == pCur || !<xsl:value-of select='$isAllowed'/>)
    {
        goto missing;
    }
    while(NULL != pCur &amp;&amp; <xsl:value-of select='$isAllowed'/>)
    {
        SUBPARAM_ATTACH(list<xsl:value-of select='$MemberBaseName'/>, pCur);
        pCur = pCur-&gt;pNextAllSubParameters;
    }
    </xsl:when>
    <xsl:otherwise>
    HELP -- parameter/choice <xsl:value-of select='@type'/> <xsl:value-of select='@repeat'/>
    </xsl:otherwise>
  </xsl:choose>

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief EncodeFunction template
 -
 - Invoked by templates
 -      StructDefinitionCommon
 -
 - Current node
 -      <llrpdef><messageDefinition>
 -      <llrpdef><parameterDefinition>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='EncodeFunction'>
  <xsl:param name='LLRPName'/>
void
LLRP_<xsl:value-of select='$LLRPName'/>_encode (
  const LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis,
  LLRP_tSEncoderStream *        pEncoderStream)
{
    const LLRP_tSEncoderStreamOps *pOps = pEncoderStream-&gt;pEncoderStreamOps;
    const LLRP_tSTypeDescriptor *pType;

  <xsl:call-template name='EncodeAllFields'>
    <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
  </xsl:call-template>
  <xsl:call-template name='EncodeAllSubParameters'>
    <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
  </xsl:call-template>
}
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief EncodeAllFields template
 -
 - Invoked by templates
 -      EncodeFunction
 -
 - Current node
 -      <llrpdef><messageDefinition>
 -      <llrpdef><parameterDefinition>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='EncodeAllFields'>
  <xsl:param name='LLRPName'/>
  <xsl:for-each select='LL:field|LL:reserved'>
    <xsl:choose>
      <xsl:when test='self::LL:field'>
        <xsl:call-template name='EncodeOneField'>
          <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test='self::LL:reserved'>
        <xsl:call-template name='EncodeOneReserved'>
          <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
    HELP -- EncodeAllFields
      </xsl:otherwise>
    </xsl:choose>
  </xsl:for-each>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief EncodeOneField template
 -
 - Invoked by templates
 -      EncodeAllFields
 -
 - Current node
 -      <llrpdef><messageDefinition><field>
 -      <llrpdef><parameterDefinition><field>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='EncodeOneField'>
  <xsl:param name='LLRPName'/>
  <xsl:choose>
    <xsl:when test='@enumeration and @type = "u8v"'>
    pOps-&gt;pfPut_e8v(pEncoderStream,
        pThis-&gt;<xsl:value-of select='@name'/>,
        &amp;LLRP_fd<xsl:value-of select='$LLRPName'/>_<xsl:value-of select='@name'/>);
    </xsl:when>
    <xsl:when test='@enumeration'>
      <xsl:call-template name='EncodeOneFieldEnum'>
        <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name='EncodeOneFieldPlain'>
        <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
      </xsl:call-template>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief EncodeOneFieldPlain template
 -
 - Invoked by templates
 -      EncodeOneField
 -
 - Current node
 -      <llrpdef><messageDefinition><field>
 -      <llrpdef><parameterDefinition><field>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='EncodeOneFieldPlain'>
  <xsl:param name='LLRPName'/>
    pOps-&gt;pfPut_<xsl:value-of select='@type'/>(pEncoderStream,
        pThis-&gt;<xsl:value-of select='@name'/>,
        &amp;LLRP_fd<xsl:value-of select='$LLRPName'/>_<xsl:value-of select='@name'/>);
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief EncodeOneFieldEnum template
 -
 - Invoked by templates
 -      EncodeOneField
 -
 - Current node
 -      <llrpdef><messageDefinition><field enumeration=...>
 -      <llrpdef><parameterDefinition><field enumeration=...>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='EncodeOneFieldEnum'>
  <xsl:param name='LLRPName'/>
  <xsl:variable name='eType'>
    <xsl:choose>
      <xsl:when test='@type="u1"' >e1</xsl:when>
      <xsl:when test='@type="u2"' >e2</xsl:when>
      <xsl:when test='@type="u8"' >e8</xsl:when>
      <xsl:when test='@type="u16"'>e16</xsl:when>
      <xsl:when test='@type="u32"'>e32</xsl:when>
      <xsl:when test='@type="u8v"'>e8v</xsl:when>
      <xsl:otherwise>ebogus</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
    pOps-&gt;pfPut_<xsl:value-of select='$eType'/>(pEncoderStream,
        (int)pThis-&gt;e<xsl:value-of select='@name'/>,
        &amp;LLRP_fd<xsl:value-of select='$LLRPName'/>_<xsl:value-of select='@name'/>);
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief EncodeOneReserved template
 -
 - Invoked by templates
 -      EncodeAllFields
 -
 - Current node
 -      <llrpdef><messageDefinition><reserved>
 -      <llrpdef><parameterDefinition><reserved>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='EncodeOneReserved'>
  <xsl:param name='LLRPName'/>
    pOps-&gt;pfPut_reserved(pEncoderStream, <xsl:value-of select='@bitCount'/>);
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief EncodeAllSubParameters template
 -
 - Invoked by templates
 -      EncodeFunction
 -
 - Current node
 -      <llrpdef><messageDefinition>
 -      <llrpdef><parameterDefinition>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='EncodeAllSubParameters'>
  <xsl:param name='LLRPName'/>
  <xsl:for-each select='LL:parameter|LL:choice'>
    <xsl:variable name='MemberBaseName'>
      <xsl:choose>
        <xsl:when test='@name'><xsl:value-of select='@name'/></xsl:when>
        <xsl:otherwise><xsl:value-of select='@type'/></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test='self::LL:parameter'>
    pType = &amp;LLRP_td<xsl:value-of select='@type'/>;
      </xsl:when>
      <xsl:otherwise>
    pType = NULL;
      </xsl:otherwise>
    </xsl:choose>
    <xsl:choose>
      <xsl:when test='@repeat="1"'>
    pOps-&gt;pfPutRequiredSubParameter(pEncoderStream,
        (LLRP_tSParameter *)pThis-&gt;p<xsl:value-of select='$MemberBaseName'/>,
        pType);
      </xsl:when>
      <xsl:when test='@repeat="0-1"'>
    pOps-&gt;pfPutOptionalSubParameter(pEncoderStream,
        (LLRP_tSParameter *)pThis-&gt;p<xsl:value-of select='$MemberBaseName'/>,
        pType);
      </xsl:when>
      <xsl:when test='@repeat="0-N"'>
    pOps-&gt;pfPutOptionalSubParameterList(pEncoderStream,
        (LLRP_tSParameter *)pThis-&gt;list<xsl:value-of select='$MemberBaseName'/>,
        pType);
      </xsl:when>
      <xsl:when test='@repeat="1-N"'>
    pOps-&gt;pfPutRequiredSubParameterList(pEncoderStream,
        (LLRP_tSParameter *)pThis-&gt;list<xsl:value-of select='$MemberBaseName'/>,
        pType);
      </xsl:when>
      <xsl:otherwise>
    HELP -- EncodeAllSubParameters <xsl:value-of select='@type'/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:for-each>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief FieldAccessorFunctions template
 -
 - Invoked by templates
 -      StructDefinitionCommon
 -
 - Current node
 -      <llrpdef><messageDefinition>
 -      <llrpdef><parameterDefinition>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='FieldAccessorFunctions'>
  <xsl:param name='LLRPName'/>
  <xsl:for-each select='LL:field'>
    <xsl:choose>
      <xsl:when test='@enumeration and @type != "u8v"'>
        <xsl:call-template name='FieldAccessorsEnum'>
          <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test='@type = "u8v"  or @type = "s8v"  or
                      @type = "u16v" or @type = "s16v" or
                      @type = "u32v" or @type = "s32v" or
                      @type = "u64v" or @type = "s64v" or
                      @type = "u1v"  or @type = "utf8v" or
                      @type = "bytesToEnd"'>
        <xsl:call-template name='FieldAccessorsVector'>
          <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name='FieldAccessorsPlain'>
          <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
        </xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:for-each>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief FieldAccessorsEnum template
 -
 - Invoked by templates
 -      FieldAccessorFunctions
 -
 - Current node
 -      <llrpdef><messageDefinition><field enumeration=...>
 -      <llrpdef><parameterDefinition><field enumeration=...>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='FieldAccessorsEnum'>
  <xsl:param name='LLRPName'/>
LLRP_tE<xsl:value-of select='@enumeration'/>
LLRP_<xsl:value-of select='$LLRPName'/>_get<xsl:value-of select='@name'/> (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis)
{
    return pThis-&gt;e<xsl:value-of select='@name'/>;
}

LLRP_tResultCode
LLRP_<xsl:value-of select='$LLRPName'/>_set<xsl:value-of select='@name'/> (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis,
  LLRP_tE<xsl:value-of select='@enumeration'/> eValue)
{
    pThis-&gt;e<xsl:value-of select='@name'/> = eValue;
    return LLRP_RC_OK;
}

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief FieldAccessorsVector template
 -
 - Invoked by templates
 -      FieldAccessorFunctions
 -
 - Current node
 -      <llrpdef><messageDefinition><field enumeration=...>
 -      <llrpdef><parameterDefinition><field enumeration=...>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='FieldAccessorsVector'>
  <xsl:param name='LLRPName'/>
llrp_<xsl:value-of select='@type'/>_t
LLRP_<xsl:value-of select='$LLRPName'/>_get<xsl:value-of select='@name'/> (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis)
{
    return pThis-&gt;<xsl:value-of select='@name'/>;
}

LLRP_tResultCode
LLRP_<xsl:value-of select='$LLRPName'/>_set<xsl:value-of select='@name'/> (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis,
  llrp_<xsl:value-of select='@type'/>_t Value)
{
    LLRP_<xsl:value-of select='@type'/>_clear(&amp;pThis-&gt;<xsl:value-of select='@name'/>);

    pThis-&gt;<xsl:value-of select='@name'/> = Value;
    return LLRP_RC_OK;
}

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief FieldAccessorsPlain template
 -
 - Invoked by templates
 -      FieldAccessorFunctions
 -
 - Current node
 -      <llrpdef><messageDefinition><field enumeration=...>
 -      <llrpdef><parameterDefinition><field enumeration=...>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='FieldAccessorsPlain'>
  <xsl:param name='LLRPName'/>
llrp_<xsl:value-of select='@type'/>_t
LLRP_<xsl:value-of select='$LLRPName'/>_get<xsl:value-of select='@name'/> (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis)
{
    return pThis-&gt;<xsl:value-of select='@name'/>;
}

LLRP_tResultCode
LLRP_<xsl:value-of select='$LLRPName'/>_set<xsl:value-of select='@name'/> (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis,
  llrp_<xsl:value-of select='@type'/>_t Value)
{
    pThis-&gt;<xsl:value-of select='@name'/> = Value;
    return LLRP_RC_OK;
}

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief SubParameterAccessorFunctions template
 -
 - Invoked by templates
 -      StructDefinitionCommon
 -
 - Current node
 -      <llrpdef><messageDefinition>
 -      <llrpdef><parameterDefinition>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='SubParameterAccessorFunctions'>
  <xsl:param name='LLRPName'/>
  <xsl:for-each select='LL:parameter|LL:choice'>
    <xsl:choose>
      <xsl:when test='self::LL:parameter and @type = "Custom"'>
        <xsl:call-template name='SubParamAccessorsExtensionPoint'>
          <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test='self::LL:parameter'>
        <xsl:call-template name='SubParamAccessorsParam'>
          <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test='self::LL:choice'>
        <xsl:call-template name='SubParamAccessorsChoice'>
          <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
    HELP -- SubParameterAccessorFunctions <xsl:value-of select='@type'/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:for-each>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief SubParamAccessorsParam template
 -
 - Invoked by templates
 -      SubParameterAccessorFunctions
 -
 - Current node
 -      <llrpdef><messageDefinition><parameter>
 -      <llrpdef><parameterDefinition><parameter>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='SubParamAccessorsParam'>
  <xsl:param name='LLRPName'/>
  <xsl:choose>
    <xsl:when test='@repeat = "1" or @repeat = "0-1"'>
      <xsl:call-template name='SubParamAccessorsParamPtr'>
        <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test='@repeat = "0-N" or @repeat = "1-N"'>
      <xsl:call-template name='SubParamAccessorsParamList'>
        <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
    HELP -- SubParamAccessorsParam <xsl:value-of select='@repeat'/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief SubParamAccessorsParamPtr template
 -
 - Invoked by templates
 -      SubParamAccessorsParam
 -
 - Current node
 -      <llrpdef><messageDefinition><parameter repeat="1">
 -      <llrpdef><messageDefinition><parameter repeat="0-1">
 -      <llrpdef><parameterDefinition><parameter repeat="1">
 -      <llrpdef><parameterDefinition><parameter repeat="0-1">
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='SubParamAccessorsParamPtr'>
  <xsl:param name='LLRPName'/>
  <xsl:variable name='MemberBaseName'>
    <xsl:choose>
      <xsl:when test='@name'><xsl:value-of select='@name'/></xsl:when>
      <xsl:otherwise><xsl:value-of select='@type'/></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
LLRP_tS<xsl:value-of select='@type'/> *
LLRP_<xsl:value-of select='$LLRPName'/>_get<xsl:value-of select='$MemberBaseName'/> (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis)
{
    return pThis-&gt;p<xsl:value-of select='$MemberBaseName'/>;
}

LLRP_tResultCode
LLRP_<xsl:value-of select='$LLRPName'/>_set<xsl:value-of select='$MemberBaseName'/> (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis,
  LLRP_tS<xsl:value-of select='@type'/> * pValue)
{
    SUBPARAM_SET(p<xsl:value-of select='$MemberBaseName'/>, pValue);

    return LLRP_RC_OK;
}

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief SubParamAccessorsParamList template
 -
 - Invoked by templates
 -      SubParamAccessorsParam
 -
 - Current node
 -      <llrpdef><messageDefinition><parameter repeat="0-N">
 -      <llrpdef><messageDefinition><parameter repeat="1-N">
 -      <llrpdef><parameterDefinition><parameter repeat="0-N">
 -      <llrpdef><parameterDefinition><parameter repeat="1-N">
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='SubParamAccessorsParamList'>
  <xsl:param name='LLRPName'/>
  <xsl:variable name='MemberBaseName'>
    <xsl:choose>
      <xsl:when test='@name'><xsl:value-of select='@name'/></xsl:when>
      <xsl:otherwise><xsl:value-of select='@type'/></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
LLRP_tS<xsl:value-of select='@type'/> *
LLRP_<xsl:value-of select='$LLRPName'/>_begin<xsl:value-of select='$MemberBaseName'/> (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis)
{
    return pThis-&gt;list<xsl:value-of select='$MemberBaseName'/>;
}

LLRP_tResultCode
LLRP_<xsl:value-of select='$LLRPName'/>_add<xsl:value-of select='$MemberBaseName'/> (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis,
  LLRP_tS<xsl:value-of select='@type'/> *pValue)
{
    SUBPARAM_ADD(list<xsl:value-of select='$MemberBaseName'/>, pValue);

    return LLRP_RC_OK;
}

LLRP_tS<xsl:value-of select='@type'/> *
LLRP_<xsl:value-of select='$LLRPName'/>_next<xsl:value-of select='$MemberBaseName'/> (
  LLRP_tS<xsl:value-of select='@type'/> *pCurrent)
{
    return (LLRP_tS<xsl:value-of select='@type'/> *)
                pCurrent-&gt;hdr.pNextSubParameter;
}

void
LLRP_<xsl:value-of select='$LLRPName'/>_clear<xsl:value-of select='$MemberBaseName'/> (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis)
{
    SUBPARAM_CLEAR(list<xsl:value-of select='$MemberBaseName'/>);
}

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief SubParamAccessorsChoice template
 -
 - Invoked by templates
 -      SubParameterAccessorFunctions
 -
 - Current node
 -      <llrpdef><messageDefinition><choice>
 -      <llrpdef><parameterDefinition><choice>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='SubParamAccessorsChoice'>
  <xsl:param name='LLRPName'/>
  <xsl:choose>
    <xsl:when test='@repeat = "1" or @repeat = "0-1"'>
      <xsl:call-template name='SubParamAccessorsChoicePtr'>
        <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test='@repeat = "0-N" or @repeat = "1-N"'>
      <xsl:call-template name='SubParamAccessorsChoiceList'>
        <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
    HELP -- SubParamAccessorsChoice <xsl:value-of select='@repeat'/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief SubParamAccessorsChoicePtr template
 -
 - Invoked by templates
 -      SubParamAccessorsChoice
 -
 - Current node
 -      <llrpdef><messageDefinition><choice repeat="1">
 -      <llrpdef><messageDefinition><choice repeat="0-1">
 -      <llrpdef><parameterDefinition><choice repeat="1">
 -      <llrpdef><parameterDefinition><choice repeat="0-1">
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='SubParamAccessorsChoicePtr'>
  <xsl:param name='LLRPName'/>
  <xsl:variable name='MemberBaseName'>
    <xsl:choose>
      <xsl:when test='@name'><xsl:value-of select='@name'/></xsl:when>
      <xsl:otherwise><xsl:value-of select='@type'/></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
LLRP_tSParameter *
LLRP_<xsl:value-of select='$LLRPName'/>_get<xsl:value-of select='$MemberBaseName'/> (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis)
{
    return pThis-&gt;p<xsl:value-of select='$MemberBaseName'/>;
}

LLRP_tResultCode
LLRP_<xsl:value-of select='$LLRPName'/>_set<xsl:value-of select='$MemberBaseName'/> (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis,
  LLRP_tSParameter * pValue)
{
    if(!LLRP_<xsl:value-of select='@type'/>_isMember(pValue))
    {
        return LLRP_RC_InvalidChoiceMember;
    }

    SUBPARAM_SET(p<xsl:value-of select='$MemberBaseName'/>, pValue);

    return LLRP_RC_OK;
}

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief SubParamAccessorsChoiceList template
 -
 - Invoked by templates
 -      SubParamAccessorsChoice
 -
 - Current node
 -      <llrpdef><messageDefinition><choice repeat="0-N">
 -      <llrpdef><messageDefinition><choice repeat="1-N">
 -      <llrpdef><parameterDefinition><choice repeat="0-N">
 -      <llrpdef><parameterDefinition><choice repeat="1-N">
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='SubParamAccessorsChoiceList'>
  <xsl:param name='LLRPName'/>
  <xsl:variable name='MemberBaseName'>
    <xsl:choose>
      <xsl:when test='@name'><xsl:value-of select='@name'/></xsl:when>
      <xsl:otherwise><xsl:value-of select='@type'/></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
LLRP_tSParameter *
LLRP_<xsl:value-of select='$LLRPName'/>_begin<xsl:value-of select='$MemberBaseName'/> (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis)
{
    return pThis-&gt;list<xsl:value-of select='$MemberBaseName'/>;
}

LLRP_tResultCode
LLRP_<xsl:value-of select='$LLRPName'/>_add<xsl:value-of select='$MemberBaseName'/> (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis,
  LLRP_tSParameter *pValue)
{
    if(!LLRP_<xsl:value-of select='@type'/>_isMember(pValue))
    {
        return LLRP_RC_InvalidChoiceMember;
    }

    SUBPARAM_ADD(list<xsl:value-of select='$MemberBaseName'/>, pValue);

    return LLRP_RC_OK;
}

LLRP_tSParameter *
LLRP_<xsl:value-of select='$LLRPName'/>_next<xsl:value-of select='$MemberBaseName'/> (
  LLRP_tSParameter *pCurrent)
{
    return pCurrent-&gt;pNextSubParameter;
}

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief SubParamAccessorsExtensionPoint template
 -
 - Invoked by templates
 -      SubParameterAccessorFunctions
 -
 - Current node
 -      <llrpdef><messageDefinition><parameter type='Custom'>
 -      <llrpdef><parameterDefinition><parameter type='Custom'>
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='SubParamAccessorsExtensionPoint'>
  <xsl:param name='LLRPName'/>
  <xsl:choose>
    <xsl:when test='@repeat = "0-N" or @repeat = "1-N"'>
      <xsl:call-template name='SubParamAccessorsExtensionPointList'>
        <xsl:with-param name='LLRPName'><xsl:value-of select='$LLRPName'/></xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
    HELP -- SubParamAccessorsExtensionPoint <xsl:value-of select='@repeat'/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief SubParamAccessorsExtensionPointList template
 -
 - Invoked by templates
 -      SubParamAccessorsChoice
 -
 - Current node
 -      <llrpdef><messageDefinition><parameter type='Custom' repeat="0-N">
 -      <llrpdef><messageDefinition><parameter type='Custom' repeat="1-N">
 -      <llrpdef><parameterDefinition><parameter type='Custom' repeat="0-N">
 -      <llrpdef><parameterDefinition><parameter type='Custom' repeat="1-N">
 -
 - @param   LLRPName        The original, LLRP name for the element
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='SubParamAccessorsExtensionPointList'>
  <xsl:param name='LLRPName'/>
  <xsl:variable name='MemberBaseName'>
    <xsl:choose>
      <xsl:when test='@name'><xsl:value-of select='@name'/></xsl:when>
      <xsl:otherwise><xsl:value-of select='@type'/></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
LLRP_tSParameter *
LLRP_<xsl:value-of select='$LLRPName'/>_begin<xsl:value-of select='$MemberBaseName'/> (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis)
{
    return pThis-&gt;list<xsl:value-of select='$MemberBaseName'/>;
}

LLRP_tResultCode
LLRP_<xsl:value-of select='$LLRPName'/>_add<xsl:value-of select='$MemberBaseName'/> (
  LLRP_tS<xsl:value-of select='$LLRPName'/> *pThis,
  LLRP_tSParameter *pValue)
{
    if(!LLRP_Parameter_isAllowedIn(pValue, &amp;LLRP_td<xsl:value-of select='$LLRPName'/>))
    {
        return LLRP_RC_NotAllowedAtExtensionPoint;
    }

    SUBPARAM_ADD(list<xsl:value-of select='$MemberBaseName'/>, pValue);

    return LLRP_RC_OK;
}

LLRP_tSParameter *
LLRP_<xsl:value-of select='$LLRPName'/>_next<xsl:value-of select='$MemberBaseName'/> (
  LLRP_tSParameter *pCurrent)
{
    return pCurrent-&gt;pNextSubParameter;
}

</xsl:template>


<!--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -
 - @brief GenerateEnrollIntoTypeRegistryFunction template
 -
 - Invoked by top level template.
 -
 - Current node
 -      <llrpdef>
 -
 - Generates a function that constructs a CTypeRegistry
 - with all the known parameter and message types.
 -
 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 -->

<xsl:template name='GenerateEnrollIntoTypeRegistryFunction'>
void
LLRP_enroll<xsl:value-of select='$RegistryName'/>TypesIntoRegistry (
  LLRP_tSTypeRegistry *         pTypeRegistry)
{
  <xsl:for-each select='LL:parameterDefinition|LL:messageDefinition|LL:customParameterDefinition|LL:customMessageDefinition'>
    LLRP_TypeRegistry_enroll(pTypeRegistry,
        &amp;LLRP_td<xsl:value-of select='@name'/>);
  </xsl:for-each>
}
</xsl:template>

</xsl:stylesheet>
