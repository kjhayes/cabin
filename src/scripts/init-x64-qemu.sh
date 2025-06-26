
#mount blkdev / blk sys y
#mount /blk/virtio-blk-0 / root ext2

mount ramfile / ramfile sys y
mount /ramfile/initrd / initrd cpio
mount /initrd/disk.img / root ext2

mkdir /root/sys

mkdir /root/sys/ramfile
mount ramfile /root/sys/ ramfile sys y

mkdir /root/sys/initrd
mount /root/sys/ramfile/initrd /root/sys/ initrd cpio

chroot /root

setenv PATH /sys/initrd/

mkdir /sys/chr
mount chardev /sys/ chr sys y
mkdir /sys/kbd
mount kbd /sys/ kbd sys y
mkdir /sys/log
mount log /sys/ log sys y
mkdir /sys/fb
mount fbdev /sys/ fb sys y
mkdir /sys/rand
mount randdev /sys/ rand sys y
mkdir /sys/blk
mount blkdev /sys/ blk sys y
mkdir /sys/pci
mount pci /sys/ pci sys y

setstdin /sys/chr/COM0

#setstdout /sys/chr/vga-serial
#setstderr /sys/chr/vga-serial
setstdout /sys/chr/COM0
setstderr /sys/chr/COM0

#write -of /sys/fb/vga/mode 1
#doomgeneric /sys/fb/vga /sys/kbd/ps2-kbd-0

xlatekbd /sys/kbd/ps2-kbd-0 | sh | fbterm -m 2 -l 0 -f /sys/fb/vga -t /sys/initrd/standard.psf
#xlatekbd /sys/kbd/ps2-kbd-0 | sh | fbterm -m 0 -l 0 -f /sys/fb/virtio-gpu-0 -t /sys/initrd/standard.psf

# cd /sys/initrd

