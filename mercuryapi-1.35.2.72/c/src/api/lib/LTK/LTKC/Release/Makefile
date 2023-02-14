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

LIBDIR=../Library

LTKC_HDRS = \
	$(LIBDIR)/ltkc.h		\
	$(LIBDIR)/ltkc_base.h		\
	$(LIBDIR)/ltkc_connection.h	\
	$(LIBDIR)/ltkc_frame.h		\
	$(LIBDIR)/ltkc_platform.h	\
	$(LIBDIR)/ltkc_xmltext.h	\
	$(LIBDIR)/out_ltkc.h		\
	$(LIBDIR)/tm_ltkc.h		\
	$(LIBDIR)/out_tm_ltkc.h

LTKC_LIB_A          = \
	$(LIBDIR)/libltkc.a		\
	$(LIBDIR)/libltkctm.a
LTKC_LIBS           = $(LTKC_LIB_A)
LTKC_FILES          = $(LTKC_HDRS) $(LTKC_LIBS)

EXAMPLE_FILES       = ../Examples/example1.c ../Examples/Example-Makefile

all: std

std: $(LTKC_FILES) $(EXAMPLE_FILES)
	rm -rf std
	mkdir std
	chmod 777 std
	mkdir std/libltkc
	chmod 777 std/libltkc
	cp $(LTKC_FILES) std/libltkc/.
	chmod 444 std/libltkc/*
	mkdir std/libltkc/example
	cp ../Examples/example1.c std/libltkc/example/example1.c
	cp ../Examples/Example-Makefile std/libltkc/example/Makefile
	chmod 644 std/libltkc/example/*
	chmod 755 std/libltkc/example
	chmod 755 std/libltkc
	chmod 755 std
	cd std; tar czf libltkc_std.tgz libltkc

clean:
	rm -rf *.tgz
	rm -rf std

