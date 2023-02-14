

OVERVIEW OF THE LLRP Tool Kit Definitions Files
Last updated 9 Jan 2008




INTRODUCTION
============

This LTK/Definitions/ directory contains definition files.
The definition files are XML and XML Schema files that
describe the LLRP Core Protocol -- as defined by the
LLRP specification -- and also contain definitions
for the "extensions" defined by other organizations.

The term "extension" refers to CUSTOM_MESSAGE and
CustomParameter definitions.

These definitions are used for automatated code generation.

The most recent versions of and news about the
LLRP Tool Kit is available on SourceForge.net

    http://sourceforge.net/projects/llrp-toolkit/




LIST OF FILES
=============

Core/
    Directory containing core protocol definitions, i.e.
    those defined by the LLRP Specification proper.

Core/llrp-1x0-def.xml
    Describes the binary protocol.
    This file MUST and DOES conform to ../llrpdef.xsd

Core/llrp-1x0.xsd
    An XML Schema description of the LLRP core protocol.
    It is used by XML editors and validators to be
    sure XML text files make sense.

llrpdef.xsd
    An XML Schema that describes the valid contents
    of the */*-def.xml files.

LLRP.org/LLRPOrgExampleDef.xml
    Describes example extensions. It is used by
    implementors of LTK code generators as an example.
    This file MUST and DOES conform to ../llrpdef.xsd

README.TXT
    This file that you are reading.

*.{com,edu,org}/
*.{com,edu,org}/*-def.xml
*.{com,edu,org}/*.xsd
    As organizations define and publish their extensions,
    they may add definition files.
    These *-def.xml files MUST conform to ../llrpdef.xsd.
