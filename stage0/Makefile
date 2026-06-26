CC ?= cc
BASE64 ?= base64

B64DIR := b64
SRCDIR := src
BUILDDIR := build
OBJDIR := $(BUILDDIR)/obj
OUTDIR := $(BUILDDIR)/tool
TESTDIR := tests
S1ALIB ?= lib.s1a

BOOTSTRAP ?= COMPILE
PREFIX ?= 
SUFFIX ?= 
DEBUG ?= 0
TESTS_VERBOSE ?= 

COMPILE_CFLAGS := -std=c99 -Wall -Wextra -Wpedantic -Wno-unused-parameter
COMPILE_LDFLAGS := 

ifneq ($(PREFIX),)
  override PREFIX := $(PREFIX)-
endif
ifneq ($(SUFFIX),)
  override SUFFIX := -$(SUFFIX)
endif
ifeq ($(DEBUG),1)
  COMPILE_CFLAGS += -DDEBUG
endif

ASSEMBLER_B64 := $(B64DIR)/assembler.b64
EPACK_B64 := $(B64DIR)/epack.b64
ASSEMBLER_SRCS := $(SRCDIR)/assembler.c
EPACK_SRCS := $(SRCDIR)/epack.c
ASSEMBLER_OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(ASSEMBLER_SRCS))
EPACK_OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(EPACK_SRCS))
ASSEMBLER_NAME := $(PREFIX)assembler$(SUFFIX)
EPACK_NAME := $(PREFIX)epack$(SUFFIX)
ASSEMBLER = $(OUTDIR)/$(ASSEMBLER_NAME)
EPACK = $(OUTDIR)/$(EPACK_NAME)

.PHONY: all clean check create-b64

all: $(ASSEMBLER) $(EPACK)

ifeq ($(BOOTSTRAP),COMPILE)
$(ASSEMBLER): $(ASSEMBLER_OBJS) | $(OUTDIR)
	$(CC) $(COMPILE_LDFLAGS) -o $@ $<
$(EPACK): $(EPACK_OBJS) | $(OUTDIR)
	$(CC) $(COMPILE_LDFLAGS) -o $@ $<
else
ifeq ($(BOOTSTRAP),BINARY)
$(ASSEMBLER): $(OUTDIR)
	$(BASE64) -d $(ASSEMBLER_B64) > $(ASSEMBLER)
	chmod +x $(ASSEMBLER)
$(EPACK): $(OUTDIR)
	$(BASE64) -d $(EPACK_B64) > $(EPACK)
	chmod +x $(EPACK)
else
  $(error BOOTSTRAP should be either COMPILE or BINARY)
endif
endif

create-b64: $(ASSEMBLER_B64) $(EPACK_B64)

$(ASSEMBLER_B64): $(ASSEMBLER)
	$(BASE64) $(ASSEMBLER) > $(ASSEMBLER_B64)

$(EPACK_B64): $(EPACK)
	$(BASE64) $(EPACK) > $(EPACK_B64)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(COMPILE_CFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $@

$(OUTDIR):
	mkdir -p $@

check: all
	PASSED=0; \
	FAILED=0; \
	FAILED_NAMES=""; \
	for f in $(TESTDIR)/*/test.sh; do \
		if [[ -f $$f ]]; then \
			if (cd $$(dirname $$f) && verbose="$(TESTS_VERBOSE)" ./test.sh $(abspath $(ASSEMBLER)) $(abspath $(EPACK)) $(abspath $(S1ALIB))); then \
				PASSED=$$(($$PASSED + 1)); \
			else \
				FAILED=$$(($$FAILED + 1)); \
				FAILED_NAMES="$$FAILED_NAMES $$f"; \
			fi \
		fi \
	done; \
	echo "PASSED: $$PASSED"; \
	echo "FAILED: $$FAILED ($$FAILED_NAMES)"; \
	exit $$FAILED

clean:
	rm -rf $(BUILDDIR)
