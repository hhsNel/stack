CC ?= cc

BUILDDIR := build

BOOTSTRAP ?= COMPILE

CFLAGS := -std=c99 -Wall -Wextra -Wpedantic -Wno-unused-parameter
LDFLAGS := 
LIBS := -lc

ifneq ($(PREFIX),)
  override PREFIX := $(PREFIX)-
endif

.PHONY: all stg0 clean

all: stg0

stg0:
	$(MAKE) -C stage0 OUTDIR=$(CURDIR)/$(BUILDDIR) BOOTSTRAP=$(BOOTSTRAP) PREFIX=$(PREFIX)stage0 SUFFIX=$(SUFFIX)

clean:
	$(MAKE) -C stage0 clean
	rm -rf build/

