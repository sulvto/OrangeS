#/bin/bash

nasm boot.asm -o boot.bin
dd if=boot.bin of=../a.img bs=512 count=1 conv=notrunc

nasm loader.asm -o loader.bin
sudo mount -o loop ../a.img /mnt/floppy
sudo cp loader.bin  /mnt/floppy
sudo umount /mnt/floppy

nasm -f elf -o kernel.o kernel.asm
nasm -f elf -o string.o string.asm
nasm -f elf -o kliba.o kliba.asm
gcc -c -fno-builtin -o start.o start.c
ld -m elf_i386 -s -Ttext 0x30400 -o kernel.bin kernel.o string.o start.o kernel.o

sudo mount -o loop ../a.img /mnt/floppy
sudo cp kernel.bin  /mnt/floppy
sudo umount /mnt/floppy

bochs -q -f ../.bochsrc

