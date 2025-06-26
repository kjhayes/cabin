
mount /blk blkdev -t sys -s
mount /root /blk/virtio-blk-0 -t ext2

mkdir /root/sys

mkdir /root/sys/ramfile
mount /root/sys/ramfile ramfile -t sys -s

mkdir /root/sys/initrd
mount /root/sys/initrd /root/sys/ramfile/initrd -t cpio

chroot /root

setenv PATH /sys/initrd/;/usr/bin/

mkdir /sys/chr
mount /sys/chr chardev -t sys -s
mkdir /sys/kbd
mount /sys/kbd kbd -t sys -s
mkdir /sys/log
mount /sys/log log -t sys -s
mkdir /sys/fb
mount /sys/fb fbdev -t sys -s
mkdir /sys/rand
mount /sys/rand randdev -t sys -s
mkdir /sys/blk
mount /sys/blk blkdev -t sys -s
mkdir /sys/pci
mount /sys/pci pci -t sys -s
mkdir /sys/acpi
mount /sys/acpi acpi -t sys -s
mkdir /sys/proc
mount /sys/proc proc -t sys -s

setenv TERM ansi
setenv TERMINFO /usr/share/terminfo

xlatekbd /sys/kbd/ps2-kbd-0 | sh | fbterm -m 0 -l 0 -f /sys/fb/virtio-gpu-0 -t /sys/initrd/standard.psf -d /fbterm.log
#xlatekbd /sys/kbd/ps2-kbd-0 | sh | fbterm -m 2 -l 0 -f /sys/fb/vga -t /sys/initrd/standard.psf
#xlatekbd /sys/kbd/ps2-kbd-0 | sh

