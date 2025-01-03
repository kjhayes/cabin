
mount /testimg.ext2 / root ext2

mkdir /root/sys

mkdir /root/sys/chr
mount chardev /root/sys/ chr sys y
mkdir /root/sys/ramfile
mount ramfile /root/sys/ ramfile sys y
mkdir /root/sys/kbd
mount kbd /root/sys/ kbd sys y
mkdir /root/sys/log
mount log /root/sys/ log sys y
mkdir /root/sys/pci
mount pci /root/sys/ pci sys y
mkdir /root/sys/initrd

mount /root/sys/ramfile/initrd /root/sys/ initrd cpio

chroot /root

setstdout /sys/chr/vga-serial
setstderr /sys/chr/vga-serial

setenv PATH /sys/initrd/

exec xlatekbd /sys/kbd/ps2-kbd-0 | sh

