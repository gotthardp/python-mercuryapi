ifeq ($(PLATFORM),EMBEDDED)
ENDIANNESS = BIG
include ../arch/ARM/ixp42x/module.mk
STRIP ?= $(TOOLS_PATH)/strip

else ifeq ($(PLATFORM), BB_LINUX)
include ../arch/ARM/bb/module.mk

else
STRIP ?= strip
endif

ifeq ($(TMR_ENABLE_SERIAL_READER_ONLY), 1)
SERIAL_READER_ONLY = 1
else ifeq ($(TMR_ENABLE_HF_LF), 1)
SERIAL_READER_ONLY = 1
endif 

ifneq ($(SERIAL_READER_ONLY), 1)
LTKC_LIB_DIR    = lib/LTK/LTKC/Library
LTK_INC_DIR     = $(LTKC_LIB_DIR)
LTK_TM_INC_DIR  = $(LTKC_LIB_DIR)/LLRP.org
#LLRP = $(LTKC_LIB_DIR)/*.o $(LTKC_LIB_DIR)/LLRP.org/*.o
endif

STATIC_LIB = libmercuryapi.a
SHARED_LIB = libmercuryapi.so.1

ifneq ($(SERIAL_READER_ONLY), 1)
LTKC_LIB = $(LTKC_LIB_DIR)/libltkc.a
LTKC_TM_LIB = $(LTKC_LIB_DIR)/LLRP.org/libltkctm.a
LTKC_LIBS = $(LTKC_LIB) $(LTKC_TM_LIB)
endif

OBJS += serial_transport_posix.o
OBJS += serial_transport_tcp_posix.o
#OBJS += serial_transport_llrp.o
OBJS += tmr_strerror.o
OBJS += tmr_param.o
OBJS += hex_bytes.o
OBJS += tm_reader.o
OBJS += tm_reader_async.o
OBJS += serial_reader.o
OBJS += tmr_loadsave_configuration.o
ifneq ($(SERIAL_READER_ONLY), 1)
OBJS += llrp_reader.o
OBJS += llrp_reader_l3.o
endif
OBJS += serial_reader_l3.o
OBJS += tmr_utils.o

OBJS += osdep_posix.o

HEADERS += serial_reader_imp.h
ifneq ($(SERIAL_READER_ONLY), 1)
HEADERS += llrp_reader_imp.h
HEADERS += tmr_llrp_reader.h
endif
HEADERS += tm_config.h
HEADERS += tm_reader.h
HEADERS += tmr_filter.h
HEADERS += tmr_gen2.h
HEADERS += tmr_gpio.h
HEADERS += tmr_ipx.h
HEADERS += tmr_iso180006b.h
HEADERS += tmr_iso14443a.h
HEADERS += tmr_iso14443b.h
HEADERS += tmr_iso15693.h
HEADERS += tmr_lf125khz.h
HEADERS += tmr_lf134khz.h
HEADERS += tmr_params.h
HEADERS += tmr_read_plan.h
HEADERS += tmr_region.h
HEADERS += tmr_serial_reader.h
HEADERS += tmr_serial_transport.h
HEADERS += tmr_status.h
HEADERS += tmr_tag_auth.h
HEADERS += tmr_tag_data.h
HEADERS += tmr_tag_lock_action.h
HEADERS += tmr_tag_protocol.h
HEADERS += tmr_tagop.h
HEADERS += tmr_types.h
HEADERS += tmr_utils.h

DBG ?= -g
CWARN = -Werror -Wall
# Add -Wextra to chase down warnings that might appear on build server, but not dev machines
#CWARN += -Wextra

ifneq ($(SERIAL_READER_ONLY), 1)
CFLAGS += -I$(LTK_INC_DIR) -I$(LTK_TM_INC_DIR)

ifeq ($(TMR_ENABLE_UHF), 1)
CFLAGS += -D TMR_ENABLE_UHF=1
endif

else

ifeq ($(TMR_ENABLE_HF_LF), 1)
CFLAGS += -D TMR_ENABLE_HF_LF=1
else
CFLAGS += -D TMR_ENABLE_SERIAL_READER_ONLY=1
endif
endif


CFLAGS += -I. $(DBG) $(CWARN)

# Position-independent code required for shared libraries
CFLAGS += -fPIC

ifndef SKIP_SAMPLES
ifneq ($(PLATFORM), BB_LINUX)
ifneq ($(PLATFORM),EMBEDDED)
PROGS += demo
endif
endif
PROGS += customantennaconfig
PROGS += filter
PROGS += firmwareload
PROGS += locktag
PROGS += multireadasync
PROGS += read
PROGS += readasync
PROGS += readintoarray
PROGS += blockpermalock
PROGS += blockwrite
PROGS += embeddedreadtid
PROGS += licensekey
PROGS += multiprotocolread
PROGS += savedconfig
PROGS += writetag
PROGS += readasynctrack
PROGS += readasyncfilter
PROGS += readasyncfilter-ISO18k-6b
PROGS += serialtime
PROGS += tagdir
PROGS += fastid
PROGS += denatranIAVcustomtagoperations
PROGS += readerstats
PROGS += readerInfo
PROGS += antennalist
PROGS += securereaddata
PROGS += readstopTrigger
PROGS += rebootReader
PROGS += readasyncGPIOControl
PROGS += readcustomtransport
PROGS += savedreadplanconfig
PROGS += loadsaveconfiguration
PROGS += bap
PROGS += readallmembanks-GEN2
PROGS += gpiocommands
PROGS += SL900Agetcalibrationdata
PROGS += SL900Agetsensorvalue
PROGS += SL900Asetcalibrationdata
PROGS += SL900Asetsfeparameters
PROGS += authenticate
PROGS += readbuffer
PROGS += untraceable
PROGS += autonomousmode
PROGS += RegulatoryTesting
PROGS += returnloss
PROGS += RegionConfiguration
PROGS += deviceDetection
PROGS += passThrough
endif

ifneq ($(SERIAL_READER_ONLY), 1)
all: $(LTKC_LIB) $(STATIC_LIB) $(SHARED_LIB) $(PROGS)

$(OBJS): $(LTKC_LIB) $(LTKC_TM_IB)

$(LTKC_LIB) $(LTKC_TM_LIB): lib/LTK

lib/LTK: lib/install_LTKC.sh
	SOURCE_DIR=lib PATCH_DIR=lib XML_DIR=lib sh $< NULL lib
	cd $(LTKC_LIB_DIR); make CC="$(CC) $(CFLAGS)" STRIP="$(STRIP)"

# Propagate patched LTKC files to Windows build files
update_ltkc_win32: lib/LTK
	find lib/LTK/LTKC/Library -name '*.h' -exec cp -p {} ltkc_win32/inc/ \;
	find lib/LTK/LTKC/Library -name '*.c' -exec cp -p {} ltkc_win32/src/ \;
else
all: $(STATIC_LIB) $(SHARED_LIB) $(PROGS)

$(OBJS):
endif

$(SHARED_LIB): $(OBJS)
	$(CC) $(CFLAGS) -shared -Wl,-rpath,/tm/lib,-soname,libmercuryapi.so.1 -o $@ $^ $(OPTOBJS) -lpthread
$(STATIC_LIB): $(OBJS)
	ar -rc $@ $^ $(OPTOBJS)

LIB = $(STATIC_LIB)
ifdef SHARED
  ifneq (0,$(SHARED))
    LIB = $(SHARED_LIB)
  endif
endif

ifndef SKIP_SAMPLES
include samples.mk
endif

.PHONY: clean
clean:
	rm -f $(STATIC_LIB) $(SHARED_LIB) $(PROGS) *.o ../samples/*.o core tests/*.output
	rm -fr lib/LTK

.PHONY: test
TESTSCRIPTS ?=
test: demo
	tests/runtests.sh $(TESTSCRIPTS)

longtest: demo
	while [ 1 ]; do echo Iteration: `date`; make test; done

test-sleeprecovery: demo
	./demo -v $(URI) <tests/test-sleeprecovery.prelim-script
# @todo Turn sleeprecovery into a real script when we know what to put in the key file

# What serial ports exist?
list-linux-ports:
	echo /dev/tty{ACM,USB,S}*

## Measure library size
libsize: $(LIB) $(LIB).stripped
	svn di tm_config.h
	echo DBG: $(DBG)
	ls -l $<*
	echo

%.stripped: %
	cp -p $< $@
	strip $@

## Test of minimum library size
empty.c:
	echo >$@

libempty.a: empty.o
	ar -rc $@ $^

libemptysize: libempty.a libempty.a.stripped
	ls -l $<*
	echo

# For internal use only.  Distribution should already include the following target files.
lib/install_LTKC.sh:
	make -f externals.mk

ifdef EXTRAMKS
include $(EXTRAMKS)
endif
