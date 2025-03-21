
ISO_BOOT_FILES += $(KANAWHA_OUTPUT_DIR)/kanawha.o
ISO_BOOT_FILES += $(OUTPUT_DIR)/initrd

isofs: $(ISO_BOOT_FILES) $(SETUPS_DIR)/x64/grub.cfg FORCE
	mkdir -p $(OUTPUT_DIR)/iso
	mkdir -p $(OUTPUT_DIR)/iso/boot
	mkdir -p $(OUTPUT_DIR)/iso/boot/grub
	cp $(SETUPS_DIR)/x64/grub.cfg $(OUTPUT_DIR)/iso/boot/grub/
	cp $(ISO_BOOT_FILES) $(OUTPUT_DIR)/iso/boot/

isoimage: cabin.iso FORCE
cabin.iso: $(OUTPUT_DIR)/cabin.iso FORCE
$(OUTPUT_DIR)/cabin.iso: isofs FORCE
	grub-mkrescue -o $@ $(OUTPUT_DIR)/iso

