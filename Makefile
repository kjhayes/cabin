
export

default:
	@

# Set these to the root directories of a compatible version of
# the Kanawha Kernel and Elk Library Projects
ELK_ROOT_DIR := ../elk/
KANAWHA_ROOT_DIR := ../kanawha/

# Root directory of Cabin
ROOT_DIR := $(shell pwd)

SOURCE_DIR := $(ROOT_DIR)/src
SCRIPTS_DIR := $(ROOT_DIR)/scripts
SETUPS_DIR := $(ROOT_DIR)/setups

# All output files should end up in this directory
# IMPORTANT SAFETY NOTE: "make clean" simply deletes this directory
OUTPUT_DIR := $(ROOT_DIR)/build
$(shell mkdir -p $(OUTPUT_DIR))

LIBC_INCLUDE_DIR ?= $(ELK_ROOT_DIR)/include/libc
POSIX_INCLUDE_DIR ?= $(ELK_ROOT_DIR)/include/posix
KLIB_INCLUDE_DIR ?= $(ELK_ROOT_DIR)/include/

KANAWHA_INCLUDE_DIR ?= $(KANAWHA_ROOT_DIR)/include
KANAWHA_OUTPUT_DIR ?= $(KANAWHA_ROOT_DIR)/build

ELK_LIB_DIR := $(ELK_ROOT_DIR)/build
ELK_LINK_DIR := $(ELK_ROOT_DIR)/link
LIBC_PATH := $(ELK_LIB_DIR)/libc.o
KLIB_PATH := $(ELK_LIB_DIR)/klib.o
CRT0_PATH := $(ELK_LIB_DIR)/crt0.o
CRTI_PATH := $(ELK_LIB_DIR)/crti.o
CRTN_PATH := $(ELK_LIB_DIR)/crtn.o

COMMON_FLAGS += \
				-g \
				-I $(LIBC_INCLUDE_DIR) \
				-I $(POSIX_INCLUDE_DIR) \
				-I $(KLIB_INCLUDE_DIR) \
				-I $(KANAWHA_INCLUDE_DIR) \
				-fno-pie \
				-fno-pic \
				-nostdlib \
				-ffreestanding

CFLAGS += -O0
AFLAGS += -D__ASSEMBLER__

PRELINK := $(CRT0_PATH) $(CRTI_PATH)
POSTLINK := $(LIBC_PATH) $(KLIB_PATH) $(CRTN_PATH)

LDFLAGS += -T $(ELK_LINK_DIR)/link.x64.ld

BINARIES := \
	init \
	cat \
	more \
	sh \
	cp \
	ls \
	mount \
	mkdir \
	cowsay \
	write \
	hexdump \
	insmod \
	rmmod \
	xlatekbd \
	vga-fb-term \
	vga-splash \
	doomgeneric \
	fault

define binary_build_rules =

-include $$(SOURCE_DIR)/$(1)/Makefile

$$(shell mkdir -p $$(OUTPUT_DIR)/$(1)-obj)

$$(OUTPUT_DIR)/$(1)-obj/%.o: $$(SOURCE_DIR)/$(1)/%.c
	$$(CC) $$(CFLAGS) $$(COMMON_FLAGS) -c $$< -o $$@

$(1): $$(OUTPUT_DIR)/$(1)
$$(OUTPUT_DIR)/$(1): $$(addprefix $$(OUTPUT_DIR)/$(1)-obj/, $$($(1)-obj))
	$$(LD) $$(LDFLAGS) $$(PRELINK) $$^ $$(POSTLINK) -o $$@

endef

$(foreach BINARY,$(BINARIES),$(eval $(call binary_build_rules,$(BINARY))))

default: $(BINARIES)

clean: FORCE
	rm -rf $(OUTPUT_DIR)

include $(SCRIPTS_DIR)/qemu.mk
include $(SCRIPTS_DIR)/initrd.mk
include $(SCRIPTS_DIR)/isoimage.mk

FORCE:

