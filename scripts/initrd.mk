ifndef __CABIN_INITRD_MK__
define __CABIN_INITRD_MK__
endef

INITRD_ROOT := $(OUTPUT_DIR)/initrd-build

$(INITRD_ROOT): $(OUTPUT_DIR) $(BINARIES)
	mkdir -p $@
	cp $(addprefix $(OUTPUT_DIR)/,$(BINARIES)) $@
	cp $(SOURCE_DIR)/scripts/init.sh $@
	cp $(SOURCE_DIR)/scripts/init-vga.sh $@
	cp $(SOURCE_DIR)/scripts/init-doom.sh $@
	cp $(ROOT_DIR)/initrd-extra/* $@

initrd: $(OUTPUT_DIR)/initrd
$(OUTPUT_DIR)/initrd: $(INITRD_ROOT) FORCE
	$(call qinfo, CPIO, $(call rel-dir, $@, $(OUTPUT_DIR)))
	ls $< | \
		cpio -o \
        --no-absolute-filenames \
		-D $< \
		-H bin \
		> $@

endif
