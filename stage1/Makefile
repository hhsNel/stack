CC ?= cc
BASE64 ?= base64

SRCDIR := src
BUILDDIR := build
CONCATDIR := $(BUILDDIR)/concat
PASS1DIR := $(BUILDDIR)/pass1
OUTDIR := $(BUILDDIR)/tool
S1ALIB ?= $(SRCDIR)/common/lib.s1a
S0_ASSEMBLER ?= /dev/null
S0_EPACK ?= /dev/null

PREFIX ?= 
SUFFIX ?= 

ifneq ($(PREFIX),)
  override PREFIX := $(PREFIX)-
endif
ifneq ($(SUFFIX),)
  override SUFFIX := -$(SUFFIX)
endif

ifeq ($(shell test -x $(S0_ASSEMBLER) && echo 'ok'),)
  $(error expected S0_ASSEMBLER to be present and executable)
endif
ifeq ($(shell test -x $(S0_EPACK) && echo 'ok'),)
  $(error expected S0_EPACK to be present and executable)
endif

COMMON_SRCS := $(shell find $(SRCDIR)/common -name "*.s1a")
ASSEMBLER_SRCS := $(SRCDIR)/assembler/main.s1a $(filter-out $(SRCDIR)/assembler/main.s1a, $(shell find $(SRCDIR)/assembler -name "*.s1a")) $(COMMON_SRCS)
EPACK_SRCS     := $(SRCDIR)/epack/main.s1a     $(filter-out $(SRCDIR)/epack/main.s1a,     $(shell find $(SRCDIR)/epack -name "*.s1a")    ) $(COMMON_SRCS)
ASSEMBLER_NAME := $(PREFIX)assembler$(SUFFIX)
EPACK_NAME := $(PREFIX)epack$(SUFFIX)
ASSEMBLER_CONCAT := $(CONCATDIR)/$(ASSEMBLER_NAME)
EPACK_CONCAT := $(CONCATDIR)/$(EPACK_NAME)
ASSEMBLER = $(OUTDIR)/$(ASSEMBLER_NAME)
EPACK = $(OUTDIR)/$(EPACK_NAME)

.PHONY: all clean check

all: $(ASSEMBLER) $(EPACK)

$(OUTDIR)/assembler: $(CONCATDIR)/assembler.s1a $(PASS1DIR)/assembler $(PASS1DIR)/epack | $(OUTDIR)
	cat $< | $(PASS1DIR)/assembler | $(PASS1DIR)/epack > $@
	chmod +x $@

$(OUTDIR)/epack: $(CONCATDIR)/epack.s1a $(PASS1DIR)/assembler $(PASS1DIR)/epack | $(OUTDIR)
	cat $< | $(PASS1DIR)/assembler | $(PASS1DIR)/epack > $@
	chmod +x $@

$(PASS1DIR)/assembler: $(CONCATDIR)/assembler.s1a | $(PASS1DIR)
	cat $^ | $(S0_ASSEMBLER) | $(S0_EPACK) > $@
	chmod +x $@

$(PASS1DIR)/epack: $(CONCATDIR)/epack.s1a | $(PASS1DIR)
	cat $^ | $(S0_ASSEMBLER) | $(S0_EPACK) > $@
	chmod +x $@

$(CONCATDIR)/assembler.s1a: $(ASSEMBLER_SRCS) | $(CONCATDIR)
	rm -f $@
	for f in $^; do \
		cat $$f >> $@; \
	done

$(CONCATDIR)/epack.s1a: $(EPACK_SRCS)| $(CONCATDIR)
	rm -f $@
	for f in $^; do \
		cat $$f >> $@; \
	done

$(CONCATDIR):
	mkdir -p $@

$(OUTDIR):
	mkdir -p $@

$(PASS1DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILDDIR)
