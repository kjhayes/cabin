
export

default:
	@

ROOT_DIR := $(shell pwd)

OUTPUT_DIR := $(ROOT_DIR)/build
$(shell mkdir -p $(OUTPUT_DIR))

ELK_ROOT_DIR := ../elk/
LIBC_INCLUDE_DIR := $(ELK_ROOT_DIR)/include/libc
POSIX_INCLUDE_DIR := $(ELK_ROOT_DIR)/include/posix
KLIB_INCLUDE_DIR := $(ELK_ROOT_DIR)/include/

KANAWHA_ROOT_DIR := ../kanawha/
KANAWHA_INCLUDE_DIR := $(KANAWHA_ROOT_DIR)/include

ELK_LIB_DIR := $(ELK_ROOT_DIR)/build
LIBC_PATH := $(ELK_LIB_DIR)/libc.o
KLIB_PATH := $(ELK_LIB_DIR)/klib.o
CRT0_PATH := $(ELK_LIB_DIR)/crt0.o
CRTI_PATH := $(ELK_LIB_DIR)/crti.o
CRTN_PATH := $(ELK_LIB_DIR)/crtn.o

COMMON_FLAGS += \
				-I $(LIBC_INCLUDE_DIR) \
				-I $(POSIX_INCLUDE_DIR) \
				-I $(KLIB_INCLUDE_DIR) \
				-I $(KANAWHA_INCLUDE_DIR) \
				-fno-pie \
				-fno-pic \
				-nostdlib \
				-ffreestanding

AFLAGS += -D__ASSEMBLER__

PRELINK := $(CRT0_PATH) $(CRTI_PATH)
POSTLINK := $(LIBC_PATH) $(KLIB_PATH) $(CRTN_PATH)

BINARIES := \
	init \
	cat \
	sh \
	ls \
	mount \
	mkdir \
	cowsay \
	test

define binary_build_rules =

-include $$(ROOT_DIR)/$(1)/Makefile

$$(shell mkdir -p $$(OUTPUT_DIR)/$(1)-obj)

$$(OUTPUT_DIR)/$(1)-obj/%.o: $$(ROOT_DIR)/$(1)/%.c
	$$(CC) $$(CFLAGS) $$(COMMON_FLAGS) -c $$< -o $$@

$(1): $$(OUTPUT_DIR)/$(1)
$$(OUTPUT_DIR)/$(1): $$(addprefix $$(OUTPUT_DIR)/$(1)-obj/, $$($(1)-obj))
	$$(LD) $$(LDFLAGS) $$(PRELINK) $$^ $$(POSTLINK) -o $$@

endef

$(foreach BINARY,$(BINARIES),$(eval $(call binary_build_rules,$(BINARY))))

default: $(BINARIES)

clean: FORCE
	rm -rf $(OUTPUT_DIR)

FORCE:

