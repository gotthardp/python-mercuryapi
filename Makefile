APIVER ?= 1.27.3.16
PYTHON ?= $(shell { command -v python3 || command -v python; } 2>/dev/null)

.PHONY: all mercuryapi install

all: mercuryapi
	$(PYTHON) setup.py build

install: mercuryapi
	$(PYTHON) setup.py install

mercuryapi: mercuryapi-$(APIVER)/.done
	make -C mercuryapi-$(APIVER)/c/src/api

	mkdir -p build/mercuryapi/include
	find mercuryapi-*/c/src/api -type f -name '*.h' ! -name '*_imp.h' | xargs cp -t build/mercuryapi/include

	mkdir -p build/mercuryapi/lib
	find mercuryapi-*/c/src/api -type f -name '*.a' -or -name '*.so.1' | xargs cp -t build/mercuryapi/lib

mercuryapi-$(APIVER)/.done: mercuryapi-$(APIVER).zip
	unzip mercuryapi-$(APIVER).zip
	patch -p0 -d mercuryapi-$(APIVER) < mercuryapi.patch
	touch mercuryapi-$(APIVER)/.done

mercuryapi-$(APIVER).zip:
	wget http://www.thingmagic.com/images/Downloads/software/mercuryapi-$(APIVER).zip
