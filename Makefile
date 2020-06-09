APIZIP ?= mercuryapi-1.31.1.36-2.zip
APIVER ?= 1.31.1.36
PYTHON ?= $(shell { command -v python3 || command -v python; } 2>/dev/null)

.PHONY: all mercuryapi install

all: mercuryapi
	$(PYTHON) setup.py build

install: mercuryapi
	$(PYTHON) setup.py install

mercuryapi: mercuryapi-$(APIVER)/.done
	make -C mercuryapi-$(APIVER)/c/src/api

	mkdir -p build/mercuryapi/include
	find mercuryapi-*/c/src/api -type f -name '*.h' ! -name '*_imp.h' ! -path '*ltkc_win32*' -exec cp {} build/mercuryapi/include/ \;

	mkdir -p build/mercuryapi/lib
	find mercuryapi-*/c/src/api -type f \( -name '*.a' -or -name '*.so.1' \) -exec cp {} build/mercuryapi/lib/ \;

mercuryapi-$(APIVER)/.done: $(APIZIP)
	unzip $(APIZIP)
	patch -p0 -d mercuryapi-$(APIVER) < mercuryapi.patch
	touch mercuryapi-$(APIVER)/.done

$(APIZIP):
	curl https://www.jadaktech.com/wp-content/uploads/2019/10/$(APIZIP) -o $(APIZIP)
