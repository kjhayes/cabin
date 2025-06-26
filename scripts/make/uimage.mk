
uImage: FORCE
	$(OBJCOPY) -O binary $(KANAWHA_OUTPUT_DIR)/kanawha.o Image
	mkimage -A riscv -O linux -T kernel -C none \
		-a $(LINK_ADDR) -e $(LINK_ADDR) \
		-n "kanawha" \
		-d Image uImage
	$(Q)rm Image

