
export

ROOT_DIR := $(shell pwd)
SCRIPTS_DIR := $(ROOT_DIR)/scripts
MK_SCRIPTS_DIR := $(SCRIPTS_DIR)/make
SOURCE_REL_DIR := src
SOURCE_DIR := $(ROOT_DIR)/$(SOURCE_REL_DIR)

KANAWHA_OUTPUT_DIR := ../kanawha/build/

OUTPUT_DIR := $(ROOT_DIR)/build/

SETUPS_DIR := $(ROOT_DIR)/setups

PYTHON := python3

CFLAGS += -g

default:
	@

include $(MK_SCRIPTS_DIR)/include.mk
include $(MK_SCRIPTS_DIR)/config.mk

ifeq ($(findstring config,$(MAKECMDGOALS)),config)
# Don't try to do anything if the goal includes the substring "config"
else
ifndef CONFIG_CABIN

default: missing_config_message

missing_config_message: FORCE
	@echo Could Not Find .config File! Run "make menuconfig" or "make defconfig"!

else

ifdef CONFIG_X64
	ARCH := x64
endif
ifdef CONFIG_RISCV64
	ARCH := riscv64
endif

ifdef ARCH
-include $(SCRIPTS_DIR)/arch/$(ARCH)/arch.mk
else
	$(error "No Architecture Specified!")
endif

ifdef CONFIG_CLANG
	TOOLCHAIN := clang
endif
ifdef CONFIG_GCC
	TOOLCHAIN := gcc
endif

ifdef TOOLCHAIN
-include $(SCRIPTS_DIR)/toolchain/$(TOOLCHAIN)/toolchain.mk
else
	$(error "No Toolchain Specified!")
endif

$(OUTPUT_DIR): FORCE
	$(Q)mkdir -p $@

COMMON_FLAGS += \
				-g \
				-include $(AUTOCONF) \
				$(subst ",,$(CONFIG_OPT_FLAGS)) \
				-fno-pie \
				-fno-pic \

EXTRA_LIBS += \
			-lkfb \
			-lncurses_g \

COMMON_DEPS += $(AUTOCONF)
AFLAGS += -D__ASSEMBLER__
CFLAGS += -std=c99

ifdef CONFIG_DEBUG_SYMBOLS
COMMON_FLAGS += -g
endif

BINARIES := \
	more \
	sh \
	ksh \
	cp \
	rm \
	mount \
	mkdir \
	cowsay \
	write \
	set \
	insmod \
	rmmod \
	xlatekbd \
	fbinfo \
	fbterm \
	doomgeneric \
	fault \
	lspci \
	lsacpi \
	dumpenv \
	testdir \
	disptga \

define binary_build_rules =

# Final Link Rule
$(1): $$(OUTPUT_DIR)/$(1)
$$(OUTPUT_DIR)/$(1): $$(OUTPUT_DIR)/$$(SOURCE_REL_DIR)/$(1)/obj.o
	$(Q)$(CC) $(CFLAGS) $(COMMON_FLAGS) $$< -o $$@ $(EXTRA_LIBS)

# Object Compile Rule
$$(OUTPUT_DIR)/$$(SOURCE_REL_DIR)/$(1)/obj.o: $$(AUTOCONF) FORCE
	$(Q)$(MAKE) -C $$(SOURCE_DIR)/$(1) -f $$(MK_SCRIPTS_DIR)/build.mk obj

endef

$(foreach BINARY,$(BINARIES),$(eval $(call binary_build_rules,$(BINARY))))

default: $(BINARIES)

-include $(MK_SCRIPTS_DIR)/qemu.mk
-include $(MK_SCRIPTS_DIR)/initrd.mk
-include $(MK_SCRIPTS_DIR)/isoimage.mk
-include $(MK_SCRIPTS_DIR)/uimage.mk

clean: FORCE
	$(Q)find $(OUTPUT_DIR) -name "*.o" -delete $(QPIPE) $(QIGNORE)
	$(Q)find $(OUTPUT_DIR) -name "*.d" -delete $(QPIPE) $(QIGNORE)
	$(Q)rm $(AUTOCONF) $(QPIPE) $(QIGNORE)
	$(Q)rm -r $(OUTPUT_DIR) $(QPIPE) $(QIGNORE)

endif
endif

