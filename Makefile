CC ?= cc
BASE64 ?= base64

BUILDDIR := build

BOOTSTRAP ?= COMPILE
TESTS_VERBOSE ?= 

CFLAGS := -std=c99 -Wall -Wextra -Wpedantic -Wno-unused-parameter
LDFLAGS := 
LIBS := -lc

STAGES := stg0
STAGE_NAMES := stage0

ifneq ($(PREFIX),)
  override PREFIX := $(PREFIX)-
endif

.PHONY: all stg0 clean check

all: stg0

$(BUILDDIR)/$(PREFIX)stage0-assembler$(SUFFIX): stg0
$(BUILDDIR)/$(PREFIX)stage0-epack$(SUFFIX): stg0
stg0:
	$(MAKE) -C stage0 CC=$(CC) BASE64=$(BASE64) OUTDIR=$(abspath $(BUILDDIR)) BOOTSTRAP=$(BOOTSTRAP) PREFIX=$(PREFIX)stage0 SUFFIX=$(SUFFIX)

check: $(STAGES)
	PASSED=0; \
	FAILED=0; \
	for stg in $(STAGE_NAMES); do \
		if [[ -f $$stg/Makefile ]]; then \
			$(MAKE) -C $$stg OUTDIR=$(abspath $(BUILDDIR)) PREFIX=$(PREFIX)$$stg SUFFIX=$(SUFFIX) TESTS_VERBOSE=$(TESTS_VERBOSE) check; \
		fi \
	done;

clean:
	$(MAKE) -C stage0 clean
	rm -rf build/

