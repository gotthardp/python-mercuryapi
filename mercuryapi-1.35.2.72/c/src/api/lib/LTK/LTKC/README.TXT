############################################################################
#   Copyright 2007,2008 Impinj, Inc.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
############################################################################


OVERVIEW OF THE LLRP Tool Kit for C
Last updated 28 Jan 2008




INTRODUCTION
============

This is a high level overview of and orientation to
the LLRP Tool Kit for C (LTKC) on Linux.

The most recent versions of and news about the
LLRP Tool Kit is available on SourceForge.net

    http://sourceforge.net/projects/llrp-toolkit/

This document is organized:
    WHAT IT IS -- a summary for an apps programmer
    HOW IT WORKS -- orientation to the internals
    LIST OF FILES -- small description of each source file
    TODO LIST -- what still needs to be done and known problems




WHAT IT IS
==========

From the application C programmer's view:
    - There is a C struct definition for each LLRP
      message and parameter type
    - An application constructs messages as "element trees"
      of these structures.
    - Using a transaction layer, an element tree is encoded into
      a "binary frame" and sent to the LLRP compatible RFID reader.
    - Messages from the reader are decoded into element trees.
    - Applications traverse the element trees to retrieve
      results and tag data
    - Applications are responsible to be sure element trees
      are properly deallocated




HOW IT WORKS
============

The LTKC library code has two major parts.

The first part is the base types, structs, and subroutines.
These are written by hand.

The second part is generated code. The code generators
are written in XSLT and process ../Definitions/Core/llrp-1x0-def.xml.
For each LLRP message and parameter type, generated are:
    - A C struct
    - A type descriptor -- name, type number, constructor
      function pointer, etc.
    - A set of functions to encode/decode the structs.

The generated encode/decode functions act through
an abstract interface. For example, the generated
function that encodes an ROBoundarySpec does not
know whether the abstract encoder is being used to
generate a LLRP binary frame or XML text.

The model for decode is a little tricky to understand
at first:
    - Identify the message or parameter type.
    - Look in a type registry to find the type descriptor.
    - Call the generated decodeFields() function.
    - The remainder of the message or parameter is considered
      a sequence of subparameters. Each are decoded
      by the method described here.
    - As each subparameter is decoded it is appended to
      an all-list.
    - Once all subparameters have been decoded, a generated
      assimilateSubParameters() function is called to
      walk the all-list and update the individual
      struct members.

Errors during encode/decode are generally handled by
setting values in an error details structure and then
terminating. The error details include an error number,
a string, and sometimes a reference type descriptor and/or
field descriptor. With rare exception, this is enough
information for the application programmer to determine
what went wrong with the application program.




LIST OF FILES
=============

Documentation/
    Directory for LTKC-specific documentation.
    Empty at this time.

Examples/
    Directory of example applications.

Examples/example1.c
    Implements the "LLRP Hello World" program.
    It does a five second inventory five times.
    Requires a LLRP reader.
    Runs from the command line (not a GUI).

Examples/Example-Makefile
    The Makefile that is copied to the example directory
    of each binary distribution. The copy is named
    "Makefile" and gets include files and libraries
    from directory .. (dot-dot, the parent dir).

Examples/Makefile
    Makefile for the examples (during normal build).

INSTALL.TXT
    Terse instructions for obtaining and building LTKC

Library/
    The principal directory of LTKC.

Library/LLRP.org/
    Directory that contains the LLRP.org extension C++ files.
    It is combined with ../Definitions/LLRP.org/LLRPOrgExampleDef.xml
    These extensions don't really do anything, they are
    just used for testing and to demonstrate how the LTK
    implementations build extensions.

Library/LLRP.org/llrporg_ltkc_genout.c
Library/LLRP.org/llrporg_ltkc.h
    Wrappers that combine and condition the generated C++
    source and header files for LLRP.org extensions.

Library/LLRP.org/Makefile
    Makefile for LLRP.org extensions.

Library/ltkc_array.c
    Helper functions for array types, like utf8v "strings"

Library/ltkc_base.h
    Base classes of LTKC, hand coded. Base classes include:
        Elements -- common between messages and parameters
        Messages -- base class for LLRP messages
        Parameters -- base class for LLRP parameters
        TypeDescriptors -- info about element types, eg name, typenum
        FieldDescriptors -- info about fields within elements
        NamespaceDescriptors -- info about an XML namespace, eg prefix, URI
        VendorDescriptors -- info about a vendor, eg name, PEN
        Decoders -- abstract interface for decoders: external->elements
        Encoders -- abstract interface for decoders: elements->external
        TypeRegistry -- used to help look up the right TypeDescriptor
        ErrorDetails -- helps explain what went wrong

Library/ltkc_connection.c
Library/ltkc_connection.h
    Implements a simple transaction interface for conveying
    messages to/from a remote reader. It does not try to be
    all things to all people. Rather it is an important
    reference to aid developers of application-specific
    transaction layers.

Library/ltkc_element.c
    Subroutines for elements (parameters and messages)

Library/ltkc_encdec.c
    Subroutines for encoders and decoders

Library/ltkc_error.c
    Subroutines for ErrorDetails

Library/ltkc_framedecode.c
    Frame decoder, translates "LLRP Binary" into element trees.
    Conforms, of course, the Decoder abstract definition (see ltkc_base.h)

Library/ltkc_frameencode.c
    Frame encoder, translates  element trees into "LLRP Binary".
    Conforms, of course, the Encoder abstract definition (see ltkc_base.h)

Library/ltkc_frameextract.c
    Used to identify and extract a single "LLRP Binary" frame
    from a byte buffer. LLRP frames can be coallesced in a
    network socket. This does the first level interpretation
    of the bytes so that the frames can be individually processed.

Library/ltkc_frame.h
    Declarations for ltkc_frame*.c

Library/ltkc_gen_c.xslt
    XSLT code generator for C definitions.
    This is where the generated encode/decode/etc functions are.
    Used to process ../../Definition/Core/llrp-1x0-def.xml
    into a transient file named out_ltkc.inc.
    Similarly used to process extension definitions.
    Requires Linux xsltproc(1).

Library/ltkc_gen_h.xslt
    XSLT code generator for C declarations (type definitions).
    This is where the structs are defined for each message and parameter.
    Used to process ../../Definition/Core/llrp-1x0-def.xml
    into a transient file named out_ltkc.h.
    Similarly used to process extension definitions.
    Requires Linux xsltproc(1).

Library/ltkc_genout.c
    Wrapper that combines platform headers, LTKC base headers,
    and the generated out_ltkc.inc. After the code generation
    this file is ready to compile.

Library/ltkc_genoutmac.h
    A file that defines macros needed by generated code.
    It is inlcuded by ltkc_genout.c and the extension
    counterparts.

Library/ltkc.h
    API C header file. This is the one applications #include.
    It #includes out_ltkc.h along with platform and
    LTKC header files.

Library/ltkc_hdrfd.c
    Contains FieldDescriptors (see ltkc_base.h) for
    message and parameter header fields.

Library/ltkc_platform.h
    Header file that #includes the right files
    for the current platform. It uses #ifdef linux, etc.
    This is where basic typedefs, like llrp_u32_t, are done.

Library/ltkc_typeregistry.c
    A type registry is a collection of pointers to
    type descriptors. During decode, a decoder might
    ask a type registry to find the type descriptor
    for message type number 123.

Library/ltkc_xmltextencode.c
    XML text encoder, translates element trees into XML text.
    It is essentially a pretty-printer.
    Conforms, of course, the Encoder abstract definition (see ltkc_base.h)
    The output is a string saved to a caller supplied buffer.
    It is mostly a trouble-shooting aid.

Library/ltkc_xmltext.h
    Declarations for ltkc_xmltext*.c

Library/Makefile
    Makefile for the library.

Makefile
    Top-level makefile. Builds the library, tests, and examples.

README.TXT
    This file that you are reading.

Release/
    Directory where binary distributions are built

Release/Makefile
    Makefile to build binary distributions

Release/std/        generated
    This directory is created and populated with the
    standard LLRP library and headers.

Tests/
    Directory of test programs.

Tests/dx10?.c
    All dx10?.c (dx101, dx102, etc) are stand-alone tests
    of the library. No reader is required.

Tests/dx101.c
    Reads a file containing LLRP binary frames. For each frame:
        - Decode it into an element tree
        - Encode it into XML text
        - Encode it into an LLRP frame
        - Compare the original input frame to the one produced by the encoder
        - Destruct the element tree.
    It is used as a regression to make sure things work consistently
    and to verify there are no memory leaks.
    Runs from the command line (not a GUI).

Tests/dx20?.c
    All dx20?.c (dx201, dx202, etc) are tests of the library
    that require a reader to talk to.

Tests/dx201.c
    Does a simple inventory operation with some bells
    and whistles. It also does extra queries.
    Along with the basic library this exercises the
    ltkc_connection.[ch] transaction layer.
    Runs from the command line (not a GUI).

Tests/Makefile
    Makefile for the test programs.

Tests/RUN101
    A shell script that runs dx101 with certain
    inputs and validation tools. Reports PASS/FAIL.

Tests/RUN201
    A shell script that runs dx201 with certain
    validation and measurement tools. Used to
    characterize CPU and memory utilization.




TODO LIST
=========

- Need to settle question of formatting whether u1 fields are
  implictly boolean or not
- Need to finish comments, especially ltkc_base.h
- Need a decoder for XML text (XML text -> element tree)
- MAYBE: Need encoder and decoder for XML DOM trees
- Need a test for accessors (dx102)
- Need a test for XML encode/decode, needs XML encode/decode first,
  should be integrated into dx101.c
