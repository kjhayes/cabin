
mount /ramfile ramfile -t sys -s
mount /initrd /ramfile/initrd -t cpio
mount /root /initrd/disk.img -t ext2

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
mkdir /sys/info
mount /sys/info info -t sys -s
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
mkdir /sys/eth
mount /sys/eth ethdev -t sys -s
mkdir /sys/ipv4
mount /sys/ipv4 ipv4 -t sys -s

setstdout /sys/chr/COM0
setstderr /sys/chr/COM0
setstdin /sys/chr/COM0

mkdir /ramfs
mount /ramfs ramfs -t ramfs -s

mkdir fat
mount /fat /sys/initrd/fatdisk.img -t fat

write -of /sys/fb/vga/mode 3
xlatekbd /sys/kbd/ps2-kbd-0 | sh | fbterm -m 3 -l 0 -f /sys/fb/vga -t /sys/initrd/standard.psf -d /sys/chr/COM0

#whiscash /sys/fb/vga /sys/kbd/ps2-kbd-0
#doomgeneric /sys/fb/vga /sys/kbd/ps2-kbd-0


#setstdout /sys/chr/vga-serial
#setstderr /sys/chr/vga-serial
#xlatekbd /sys/kbd/ps2-kbd-0 | sh

#xlatekbd /sys/kbd/ps2-kbd-0 | sh | fbterm -m 2 -l 0 -f /sys/fb/vga -t /sys/initrd/standard.psf -d /fbterm.log

