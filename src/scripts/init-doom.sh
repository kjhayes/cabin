
mount /testimg.ext2 / root ext2

mkdir /root/sys

mkdir /root/sys/chr
mount chardev /root/sys/ chr sys y

setstdout /root/sys/chr/COM0
setstderr /root/sys/chr/COM0
setstdin /root/sys/chr/COM0

mkdir /root/sys/ramfile
mount ramfile /root/sys/ ramfile sys y
mkdir /root/sys/kbd
mount kbd /root/sys/ kbd sys y
mkdir /root/sys/log
mount log /root/sys/ log sys y
mkdir /root/sys/initrd

mount /root/sys/ramfile/initrd /root/sys/ initrd cpio

chroot /root

setenv PATH /sys/initrd/

cd /sys/initrd/
doomgeneric /sys/ramfile/vga-fb /sys/kbd/ps2-kbd-0

