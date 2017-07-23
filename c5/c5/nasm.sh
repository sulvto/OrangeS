#/bin/bash

nasm loader.asm -o loader.bin
dd if=loader.bin of=a.img bs=512 count=1 conv=notrunc
sudo mount -o loop a.img /mnt/floppy
sudo cp loader.bin  /mny/floppy
sudo umount /mnt/floppy

nasm boot.asm -o boot.bin
dd if=boot.bin of=a.img bs=512 count=1 conv=notrunc
sudo mount -o loop a.img /mnt/floppy
sudo cp boot.bin  /mny/floppy
sudo umount /mnt/floppy

nasm kernel.asm -o kernel.bin
dd if=kernel.bin of=a.img bs=512 count=1 conv=notrunc
sudo mount -o loop a.img /mnt/floppy
sudo cp kernel.bin  /mny/floppy
sudo umount /mnt/floppy

