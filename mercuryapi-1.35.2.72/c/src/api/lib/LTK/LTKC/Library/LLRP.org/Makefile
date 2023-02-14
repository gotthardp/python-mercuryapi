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

#CFLAGS     = -O -Wall -I..
AR          = ar
CC          ?= gcc
CFLAGS      ?= -g -Wall
CFLAGS      += -I..

# Position-independent code required for shared libraries
CFLAGS          += -fpic
ifeq ($(LTKC_PLATFORM),EMBEDDED)
CFLAGS          += -mcpu=strongarm -mtune=xscale -mbig-endian
endif

TMDEF      = ../../../Definitions/LLRP.org/llrp-control-def.xml

CODE_GEN_H_XSLT = ../ltkc_gen_h.xslt
CODE_GEN_C_XSLT = ../ltkc_gen_c.xslt

LTKC_HDRS = \
	../ltkc.h		\
	../ltkc_base.h		\
	../ltkc_connection.h	\
	../ltkc_frame.h		\
	../ltkc_platform.h	\
	../out_ltkc.h


TM_LTKC_LIB = libltkctm.a
TM_LTKC_SHARED_LIB = libltkctm.so.1
TM_LTKC_OBJS = \
	tm_ltkc_genout.o

TM_LTKC_SONAME = $(TM_LTKC_LIB:.a=.so.1)
TM_LTKC_SHARED_LIB = $(TM_LTKC_SONAME)

all:  $(TM_LTKC_LIB) $(TM_LTKC_SHARED_LIB)

$(TM_LTKC_LIB) : $(TM_LTKC_OBJS)
	$(AR) crv $(TM_LTKC_LIB) $(TM_LTKC_OBJS)

$(TM_LTKC_SHARED_LIB) : $(TM_LTKC_OBJS)
	$(CC) $(CFLAGS) -shared -Wl,-soname,$(TM_LTKC_SHARED_LIB) -o $(TM_LTKC_SHARED_LIB) $(TM_LTKC_OBJS)
	$(STRIP) $@

$(TM_LTKC_OBJS) : $(LTKC_HDRS)

tm_ltkc_genout.o : out_tm_ltkc.inc
tm_ltkc_genout.o : out_tm_ltkc.h
tm_ltkc_genout.o : tm_ltkc_genout.c
	$(CC) -c $(CFLAGS) -Wno-unused tm_ltkc_genout.c \
		-o tm_ltkc_genout.o

out_tm_ltkc.h: $(CODE_GEN_H_XSLT) $(TMDEF)
	xsltproc --stringparam RegistryName Tm \
		-o out_tm_ltkc.h \
		$(CODE_GEN_H_XSLT) \
		$(TMDEF)

out_tm_ltkc.inc: $(CODE_GEN_C_XSLT) $(TMDEF)
	xsltproc --stringparam RegistryName Tm \
		-o out_tm_ltkc.inc \
		$(CODE_GEN_C_XSLT) \
		$(TMDEF)

clean:
	rm -f *.o *.core core.[0-9]*
	rm -f out_*.inc out_*.h
	rm -f *.a
