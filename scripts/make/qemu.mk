
QEMU_FLAGS += -device virtio-gpu-pci
#QEMU_FLAGS += -device virtio-gpu-pci
QEMU_FLAGS += -drive file=$(ROOT_DIR)/disk.img,if=none,id=virtio-disk0,format=raw \
			  -device virtio-blk-pci,drive=virtio-disk0,id=disk0
#QEMU_FLAGS += -device virtio-rng

#QEMU_FLAGS += -drive id=disk,file=ahci.img,if=none \
              -device ahci,id=ahci \
              -device ide-hd,drive=disk,bus=ahci.0

#QEMU_FLAGS += -device virtio-serial-pci,id=virtio-serial0
#QEMU_FLAGS += -chardev memory,id=charconsole0,logfile=serial.log
#QEMU_FLAGS += -device virtconsole,chardev=charconsole0,id=console0

QEMU_FLAGS += -device edu
QEMU_FLAGS += -device pci-testdev
# QEMU_FLAGS += -nic user,model=virtio-net-pci-non-transitional
#QEMU_FLAGS += -device e1000e

QEMU_FLAGS += -device e1000e,netdev=net0 -netdev user,id=net0

ifdef CONFIG_X64
QEMU := qemu-system-x86_64
ISO := $(OUTPUT_DIR)/cabin.iso
QEMU_DEPS += $(ISO)
QEMU_FLAGS += -cdrom $(ISO)

QEMU_FLAGS += -serial stdio
QEMU_FLAGS += -smp 4

#QEMU_FLAGS += -device VGA
QEMU_FLAGS += -accel tcg
QEMU_FLAGS += -machine q35
QEMU_FLAGS += -m 8G
endif

ifdef CONFIG_RISCV64
QEMU := qemu-system-riscv64
QEMU_FLAGS += -kernel $(KANAWHA_OUTPUT_DIR)/kanawha.bin
QEMU_FLAGS += -bios default
QEMU_FLAGS += -serial stdio
QEMU_FLAGS += -M virt
QEMU_FLAGS += -m 8G

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

