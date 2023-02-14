APIVER ?= 1.35.2.72
PYTHON ?= $(shell { command -v python3 || command -v python; } 2>/dev/null)
PLATFORM ?= linux

.PHONY: all mercuryapi install

all: mercuryapi
	$(PYTHON) setup.py build

install: mercuryapi
	$(PYTHON) setup.py install

mercuryapi: patchfiles
	make -C temp all SKIP_SAMPLES=yes
	mkdir -p mercuryapi/include
	mkdir -p mercuryapi/lib
	find temp -type f -name '*.h' ! -name '*_imp.h' ! -path '*ltkc_win32*' -exec cp {} mercuryapi/include/ \;
	find temp -type f \( -name '*.a' -or -name '*.so.1' \) -exec cp {} mercuryapi/lib/ \;
	rm -r temp

patchfiles: temporalfolder
ifeq ($(PLATFORM), linux)
	@echo "Applying Linux Patch"
	patch -p0 -d temp < mercuryapi.patch
else
	@echo "Applying Darwin Patch"
	patch -p0 -d temp < mercuryapi_osx.patch
endif

temporalfolder:
	mkdir -p temp
	cp -r mercuryapi-$(APIVER)/c/src/api/* temp/