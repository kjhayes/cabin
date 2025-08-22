
mount /root ramfs -t ramfs -s
mount /term termdev -t sys -s
setstdin /term/COM0
setstdout /term/COM0
setstderr /term/COM0

mkdir /root/sys

mkdir /root/sys/ramfile
mount /root/sys/ramfile ramfile -t sys -s
mkdir /root/sys/initrd
mount /root/sys/initrd /root/sys/ramfile/initrd -t cpio

chroot /root

setenv PATH /bin/;/usr/bin/;/sys/initrd/;/sys/initrd/bin/;/sys/rootdisk/usr/bin/

mkdir /dev
mkdir /dev/term
mount /dev/term termdev -t sys -s
mkdir /dev/kbd
mount /dev/kbd kbd -t sys -s
mkdir /dev/fb
mount /dev/fb fbdev -t sys -s
mkdir /dev/rand
mount /dev/rand randdev -t sys -s
mkdir /dev/blk
mount /dev/blk blkdev -t sys -s
mkdir /dev/eth
mount /dev/eth ethdev -t sys -s
mkdir /dev/ipv4
mount /dev/ipv4 ipv4 -t sys -s
mkdir /dev/shm
mount /dev/shm ramfs -t ramfs -s

mkdir /sys/info
mount /sys/info info -t sys -s
mkdir /sys/pci
mount /sys/pci pci -t sys -s
mkdir /sys/acpi
mount /sys/acpi acpi -t sys -s
mkdir /sys/proc
mount /sys/proc proc -t sys -s
mkdir /sys/udrv
mount /sys/udrv udrv -t sys -s


mkdir /sys/rootdisk
mount /sys/rootdisk /dev/blk/virtio-blk-0 -t ext2

setenv SHELL /sys/initrd/sh

setstdout /dev/term/COM0
setstderr /dev/term/COM0
setstdin  /dev/term/COM0

mkudrv term console
xlatekbd /dev/kbd/ps2-kbd-0 | udrv_term -i /sys/udrv/term/console &
udrv_term -o /sys/udrv/term/console | fbterm -m 1 -l 0 -f /dev/fb/vga -t /sys/initrd/standard.psf -d /dev/term/COM0 &

setstdin /dev/term/console
setstdout /dev/term/console
setstderr /dev/term/console

exec sh

cd /sys/initrd
doomgeneric /dev/fb/vga /dev/kbd/ps2-kbd-0

# udrv_rand &
# cat /dev/rand/udrv

#lua /sys/rootdisk/init/init.lua

#write -of /dev/fb/vga/mode 1
# cd /sys/initrd
# badapple /dev/fb/vga /dev/kbd/ps2-kbd-0

# xlatekbd /dev/kbd/ps2-kbd-0 | sh | fbterm -m 1 -l 0 -f /dev/fb/vga -t /sys/initrd/standard.psf -d /dev/term/COM0

