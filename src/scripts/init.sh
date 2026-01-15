
mount /root ramfs -t ramfs -s
mount /term termdev -t sys -s

setstdin /term/COM1
setstdout /term/COM1
setstderr /term/COM1

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

# mkdir /ide
# mount /ide /dev/blk/ide-0-primary -t ext2

setenv SHELL /sys/initrd/sh

setstdout /dev/term/COM1
setstderr /dev/term/COM1
setstdin  /dev/term/COM1

seat /dev/kbd/ps2-kbd-0 /dev/fb/vga 1 3 &
#seat /dev/kbd/ps2-kbd-0 /dev/fb/virtio-gpu-0 0 0 &

sleep 3000

mkudrv term console
xlatekbd /dev/kbd/seat-0 | udrv_term -i /sys/udrv/term/console &
udrv_term -o /sys/udrv/term/console | fbterm -m 0 -l 0 -f /dev/fb/seat-0 -t /sys/initrd/arm8.psf -d /dev/term/COM1 &

#cd /sys/initrd
#doomgeneric /dev/fb/seat-1 /dev/kbd/seat-1 &
#cd /

setstdin /dev/term/console
setstdout /dev/term/console
setstderr /dev/term/console
exec sh

# udrv_rand &
# cat /dev/rand/udrv

#write -of /dev/fb/vga/mode 1
# cd /sys/initrd
# badapple /dev/fb/vga /dev/kbd/ps2-kbd-0

# xlatekbd /dev/kbd/ps2-kbd-0 | sh | fbterm -m 1 -l 0 -f /dev/fb/vga -t /sys/initrd/standard.psf -d /dev/term/COM1

