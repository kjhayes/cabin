
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
mkdir /root/sys/fb
mount fbdev /root/sys/ fb sys y
mkdir /root/sys/rand
mount randdev /root/sys/ rand sys y

mkdir /root/sys/initrd
mount /root/sys/ramfile/dt-initrd /root/sys/ initrd cpio

chroot /root

setstdin  /sys/chr/serial
setstdout /sys/chr/serial
setstderr /sys/chr/serial

cd /sys/
exec sh

