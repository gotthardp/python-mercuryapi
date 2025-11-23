APIZIP ?= _vendor/mercuryapi-$(APIVER).zip
APIVER ?= 1.37.4.8
PYTHON ?= $(shell { command -v python3 || command -v python; } 2>/dev/null)

.PHONY: all mercuryapi install

all: mercuryapi
	$(PYTHON) setup.py build

install: mercuryapi
	$(PYTHON) setup.py install

mercuryapi: mercuryapi-$(APIVER)/.done
	make -C mercuryapi-$(APIVER)/c/src/api SKIP_SAMPLES=1

	mkdir -p build/mercuryapi/include
	find mercuryapi-*/c/src/api -type f -name '*.h' ! -name '*_imp.h' ! -path '*ltkc_win32*' -exec cp {} build/mercuryapi/include/ \;

	mkdir -p build/mercuryapi/lib
	find mercuryapi-*/c/src/api -type f \( -name '*.a' -or -name '*.so.1' \) -exec cp {} build/mercuryapi/lib/ \;

mercuryapi-$(APIVER)/.done: $(APIZIP)
	unzip -o $(APIZIP) -x "*/cs/*" "*/java/*"
	patch -p0 -d mercuryapi-$(APIVER) < mercuryapi.patch
	touch mercuryapi-$(APIVER)/.done
