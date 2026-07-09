CC ?= cc
BASE64 ?= base64

BUILDDIR := build

BOOTSTRAP ?= COMPILE
TESTS_VERBOSE ?= 

CFLAGS := -std=c99 -Wall -Wextra -Wpedantic -Wno-unused-parameter
LDFLAGS := 
LIBS := -lc

STAGES := stg0 stg1

ifneq ($(PREFIX),)
  override PREFIX := $(PREFIX)-
endif

S0_ASM_NAME = $(abspath $(BUILDDIR)/$(PREFIX)stage0-assembler$(SUFFIX))
S0_EPACK_NAME = $(abspath $(BUILDDIR)/$(PREFIX)stage0-epack$(SUFFIX))
S1_ASM_NAME = $(abspath $(BUILDDIR)/$(PREFIX)stage1-assembler$(SUFFIX))
S1_EPACK_NAME = $(abspath $(BUILDDIR)/$(PREFIX)stage1-epack$(SUFFIX))
S0_ARGS = CC=$(CC) BASE64=$(BASE64) OUTDIR=$(abspath $(BUILDDIR)) BOOTSTRAP=$(BOOTSTRAP) PREFIX=$(PREFIX)stage0 SUFFIX=$(SUFFIX)
S1_ARGS = S0_ASSEMBLER=$(S0_ASM_NAME) S0_EPACK=$(S0_EPACK_NAME) OUTDIR=$(abspath $(BUILDDIR)) PREFIX=$(PREFIX)stage1 SUFFIX=$(SUFFIX)

.PHONY: all stg0 check-stg0 stg1 check-stg1 clean check

all: stg1

stg0: $(S0_ASM_NAME) $(S0_EPACK_NAME)
$(S0_ASM_NAME):
	$(MAKE) -C stage0 $(S0_ARGS) $@
$(S0_EPACK_NAME):
	$(MAKE) -C stage0 $(S0_ARGS) $@
check-stg0:
	$(MAKE) -C stage0 $(S0_ARGS) check

stg1: $(S1_ASM_NAME) $(S1_EPACK_NAME)
$(S1_ASM_NAME): $(S0_ASM_NAME) $(S0_EPACK_NAME)
		$(MAKE) -C stage1 $(S1_ARGS) $@
$(S1_EPACK_NAME): $(BUILDDIR)/$(PREFIX)stage0-assembler$(SUFFIX) $(BUILDDIR)/$(PREFIX)stage0-epack$(SUFFIX)
		$(MAKE) -C stage1 $(S1_ARGS) $@
check-stg1: $(BUILDDIR)/$(PREFIX)stage0-assembler$(SUFFIX) $(BUILDDIR)/$(PREFIX)stage0-epack$(SUFFIX)
		$(MAKE) -C stage1 $(S1_ARGS) check

check: $(foreach STAGE,$(STAGES),$(STAGE) check-$(STAGE))

clean:
	for stg in $(foreach STAGE,$(STAGES),$(subst stg,stage,$(STAGE))); do $(MAKE) -C $$stg clean; done
	rm -rf build/

