

ifdef CONFIG_X64
QEMU := qemu-system-x86_64
ISO := $(OUTPUT_DIR)/cabin.iso
QEMU_DEPS += $(ISO)
QEMU_FLAGS += -cdrom $(ISO)

QEMU_FLAGS += -chardev stdio,id=chardev0,logfile=serial.log
QEMU_FLAGS += -chardev memory,id=chardev1,logfile=pciserial.log

QEMU_FLAGS += -serial chardev:chardev0
QEMU_FLAGS += -device VGA
QEMU_FLAGS += -M hpet=on
QEMU_FLAGS += -device virtio-serial-pci
QEMU_FLAGS += -device virtio-gpu-pci
QEMU_FLAGS += -device virtio-rng
QEMU_FLAGS += -device virtconsole,chardev=chardev1,name=console.0
QEMU_FLAGS += -accel tcg
QEMU_FLAGS += -device edu
QEMU_FLAGS += -device pci-testdev
QEMU_FLAGS += -device e1000e
QEMU_FLAGS += -m 1G
QEMU_FLAGS += -smp 2
endif

ifdef CONFIG_RISCV64
QEMU := qemu-system-riscv64
QEMU_FLAGS += -kernel $(KANAWHA_OUTPUT_DIR)/kanawha.bin
QEMU_FLAGS += -bios default
QEMU_FLAGS += -serial stdio
QEMU_FLAGS += -M virt
QEMU_FLAGS += -m 2G
QEMU_FLAGS += -smp 1

QEMU_DEPS += $(OUTPUT_DIR)/initrd
QEMU_FLAGS += -initrd $(OUTPUT_DIR)/initrd
#QEMU_FLAGS += -machine dumpdtb=virt.dtb
endif

ifdef QEMU
qemu: $(QEMU_DEPS)
	$(QEMU) $(QEMU_FLAGS)
qemu-gdb: $(QEMU_DEPS)
	$(QEMU) $(QEMU_FLAGS) -gdb tcp::1234 -S -no-reboot -no-shutdown
endif

