#/bin/bash

nasm boot.asm -o boot.bin
dd if=boot.bin of=../a.img bs=512 count=1 conv=notrunc

nasm loader.asm -o loader.bin
sudo mount -o loop ../a.img /mnt/floppy
sudo cp loader.bin  /mnt/floppy
sudo umount /mnt/floppy

nasm -f elf kernel.asm -o kernel.o
ld -m elf_i386 -s kernel.o -o kernel.bin

sudo mount -o loop ../a.img /mnt/floppy
sudo cp kernel.bin  /mnt/floppy
sudo umount /mnt/floppy

bochs -q -f ../.bochsrc

