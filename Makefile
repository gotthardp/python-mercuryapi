APIVER ?= 1.35.2.72
PYTHON ?= $(shell { command -v python3 || command -v python; } 2>/dev/null)

.PHONY: all mercuryapi install

all: mercuryapi
	$(PYTHON) setup.py build

install: mercuryapi
	$(PYTHON) setup.py install

mercuryapi: 
	patch -p0 -d mercuryapi-$(APIVER) < mercuryapi.patch

	make -C mercuryapi-$(APIVER)/c/src/api all clean

	mkdir -p mercuryapi/include
	find mercuryapi-*/c/src/api -type f -name '*.h' ! -name '*_imp.h' ! -path '*ltkc_win32*' -exec cp {} mercuryapi/include/ \;

	mkdir -p mercuryapi/lib
	find mercuryapi-*/c/src/api -type f \( -name '*.a' -or -name '*.so.1' \) -exec cp {} mercuryapi/lib/ \;
