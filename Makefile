
export

default:
	@

KANAWHA_OUTPUT_DIR ?= $(ROOT_DIR)/../kanawha/build

CROSS_COMPILE_PREFIX ?= x86_64-kanawha-
CC := $(CROSS_COMPILE_PREFIX)gcc

# Root directory of Cabin
ROOT_DIR := $(shell pwd)

SOURCE_DIR := $(ROOT_DIR)/src
SCRIPTS_DIR := $(ROOT_DIR)/scripts
SETUPS_DIR := $(ROOT_DIR)/setups

# All output files should end up in this directory
# IMPORTANT SAFETY NOTE: "make clean" simply deletes this directory
OUTPUT_DIR := $(ROOT_DIR)/build
$(shell mkdir -p $(OUTPUT_DIR))

COMMON_FLAGS += \
				-g \
				-fno-pie \
				-fno-pic \
				-std=c99

CFLAGS += -O0
AFLAGS += -D__ASSEMBLER__

BINARIES := \
	cat \
	more \
	sh \
	cp \
	ls \
	mount \
	mkdir \
	cowsay \
	write \
	set \
	sleep \
	hexdump \
	insmod \
	rmmod \
	xlatekbd \
	fbinfo \
	fbterm \
	doomgeneric \
	fault \
	clear

define binary_build_rules =

-include $$(SOURCE_DIR)/$(1)/Makefile

$$(shell mkdir -p $$(OUTPUT_DIR)/$(1)-obj)

$$(OUTPUT_DIR)/$(1)-obj/%.o: $$(SOURCE_DIR)/$(1)/%.c
	$$(CC) $$(CFLAGS) $$(COMMON_FLAGS) -c $$< -o $$@

$(1): $$(OUTPUT_DIR)/$(1)
$$(OUTPUT_DIR)/$(1): $$(addprefix $$(OUTPUT_DIR)/$(1)-obj/, $$($(1)-obj))
	$$(CC) $$^ -o $$@ -lkfb

endef

$(foreach BINARY,$(BINARIES),$(eval $(call binary_build_rules,$(BINARY))))

default: $(BINARIES)

clean: FORCE
	rm -rf $(OUTPUT_DIR)

include $(SCRIPTS_DIR)/qemu.mk
include $(SCRIPTS_DIR)/initrd.mk
include $(SCRIPTS_DIR)/isoimage.mk

FORCE:

