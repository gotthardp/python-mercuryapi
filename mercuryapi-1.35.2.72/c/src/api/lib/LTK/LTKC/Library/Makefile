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

#CFLAGS = -O -Wall
AR              = ar
CC              ?= gcc
CFLAGS          ?= -g -Wall

# Position-independent code required for shared libraries
CFLAGS          += -fpic
ifeq ($(LTKC_PLATFORM),EMBEDDED)
CFLAGS          += -mcpu=strongarm -mtune=xscale -mbig-endian
endif

LLRPDEF         = ../../Definitions/Core/llrp-1x0-def.xml

CODE_GEN_H_XSLT = ltkc_gen_h.xslt
CODE_GEN_C_XSLT = ltkc_gen_c.xslt

LTKC_HDRS = \
	ltkc.h			\
	ltkc_base.h		\
	ltkc_connection.h	\
	ltkc_frame.h		\
	ltkc_platform.h		\
	ltkc_xmltext.h		\
	out_ltkc.h

LTKC_LIB = libltkc.a
LTKC_SHARED_LIB = libltkc.so.1
LTKC_OBJS = \
	ltkc_array.o		\
	ltkc_connection.o	\
	ltkc_element.o		\
	ltkc_encdec.o		\
	ltkc_error.o		\
	ltkc_framedecode.o	\
	ltkc_frameencode.o	\
	ltkc_frameextract.o	\
	ltkc_hdrfd.o		\
	ltkc_xmltextencode.o	\
	ltkc_typeregistry.o	\
	ltkc_genout.o


LTKC_SONAME = $(LTKC_LIB:.a=.so.1)
LTKC_SHARED_LIB = libltkc.so.1

all:    $(LTKC_LIB) $(LTKC_SHARED_LIB)
	cd LLRP.org; make all

everything:
	make all

$(LTKC_LIB) : $(LTKC_OBJS)
	$(AR) crv $(LTKC_LIB) $(LTKC_OBJS)

$(LTKC_SHARED_LIB) : $(LTKC_OBJS)
	$(CC) $(CFLAGS) -shared -Wl,-soname,$(LTKC_SONAME) -o $(LTKC_SHARED_LIB) $(LTKC_OBJS)
	$(STRIP) $@

$(LTKC_OBJS) : $(LTKC_HDRS)

ltkc_array.o       : ltkc_array.c
	$(CC) -c $(CFLAGS) ltkc_array.c \
		-o ltkc_array.o

ltkc_connection.o  : ltkc_connection.c
	$(CC) -c $(CFLAGS) ltkc_connection.c \
		-o ltkc_connection.o

ltkc_element.o     : ltkc_element.c
	$(CC) -c $(CFLAGS) ltkc_element.c \
		-o ltkc_element.o

ltkc_encdec.o      : ltkc_encdec.c
	$(CC) -c $(CFLAGS) ltkc_encdec.c \
		-o ltkc_encdec.o

ltkc_error.o       : ltkc_error.c
	$(CC) -c $(CFLAGS) ltkc_error.c \
		-o ltkc_error.o

ltkc_framedecode.o : ltkc_framedecode.c
	$(CC) -c $(CFLAGS) ltkc_framedecode.c \
		-o ltkc_framedecode.o

ltkc_frameencode.o : ltkc_frameencode.c
	$(CC) -c $(CFLAGS) ltkc_frameencode.c \
		-o ltkc_frameencode.o

ltkc_frameextract.o : ltkc_frameextract.c
	$(CC) -c $(CFLAGS) ltkc_frameextract.c \
		-o ltkc_frameextract.o

ltkc_hdrfd.o       : ltkc_hdrfd.c
	$(CC) -c $(CFLAGS) ltkc_hdrfd.c \
		-o ltkc_hdrfd.o

ltkc_xmltextencode.o : ltkc_xmltextencode.c
	$(CC) -c $(CFLAGS) ltkc_xmltextencode.c \
		-o ltkc_xmltextencode.o

ltkc_typeregistry.o : ltkc_typeregistry.c
	$(CC) -c $(CFLAGS) ltkc_typeregistry.c \
		-o ltkc_typeregistry.o

ltkc_genout.o      : out_ltkc.inc out_ltkc.h
ltkc_genout.o      : ltkc_genout.c
	$(CC) -c $(CFLAGS) -Wno-unused ltkc_genout.c \
		-o ltkc_genout.o

out_ltkc.h: ltkc_gen_h.xslt $(LLRPDEF)
	xsltproc --stringparam RegistryName Core \
		-o out_ltkc.h ltkc_gen_h.xslt $(LLRPDEF)

out_ltkc.inc: ltkc_gen_c.xslt $(LLRPDEF)
	xsltproc --stringparam RegistryName Core \
		-o out_ltkc.inc ltkc_gen_c.xslt $(LLRPDEF)

clean:
	rm -f *.o *.core core.[0-9]*
	rm -f out_*.inc out_*.h
	rm -f *.a
	cd LLRP.org; make clean
