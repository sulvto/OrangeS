#/bin/bash

nasm boot.asm -o boot.bin
dd if=boot.bin of=../a.img bs=512 count=1 conv=notrunc

nasm loader.asm -o loader.bin
# dd if=loader.bin of=../a.img bs=512 count=1 conv=notrunc

sudo mount -o loop ../a.img /mnt/floppy
sudo cp loader.bin  /mnt/floppy
sudo umount /mnt/floppy

# run bochs
bochs -q -f .bochsrc
