#
# Makefile of Orange`S
#

ENTRYPOINT	= 0x30400
ENTRYOFFSET = 0x400

# Programs,flags,etc.
ASM			= nasm	
DASM		= ndisasm
CC			= gcc
LD			= ld
ASMBFLAGS	= -I boot/include/
ASMKFLAGS	= -I include/ -f elf
CFLAGS		= -I include/ -c -fno-builtin
LDFLAGS		= -m elf_i386 -s -Ttext $(ENTRYPOINT)
DASMFLAGS	= -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)

# This Program
ORANGESBOOT	  = boot/boot.bin boot/loader.bin
ORANGESKERNEL = kernel.bin
OBJS		  = kernel/kernel.o kernel/start.o lib/kliba.o lib/string.o
DASMOUTPUT	  = kernel.bin.asm

.PHONY : everyting final image clean realclean disasm all buildimg

everyting : $(ORANGESBOOT) $(ORANGESKERNEL) 
all : realclean everyting
final : all clean
image : final buildimg
clean :
		rm -f $(OBJS)
realclean :
		rm -f $(OBJS) $(ORANGESBOOT) $(ORANGESKERNEL)
disasm :
		$(DASM) $(DASMFLAGS) $(ORANGESKERNEL) > $(DASMOUTPUT)

buildimg:
		dd if=boot/boot.bin of=../a.img bs=512 count=1 conv=notrunc
		sudo mount -o loop ../a.img /mnt/floppy/
		sudo cp -fv boot/loader.bin /mnt/floppy/
		sudo cp -fv kernel.bin /mnt/floppy
		sudo umount /mnt/floppy

boot/boot.bin : boot/boot.asm boot/include/load.inc boot/include/fat12hdr.inc
		$(ASM) $(ASMBFLAGS) -o $@ $<

boot/loader.bin : boot/loader.asm boot/include/load.inc boot/include/fat12hdr.inc boot/include/pm.inc
		$(ASM) $(ASMBFLAGS) -o $@ $<

$(ORANGESKERNEL) : $(OBJS)
		$(LD) $(LDFLAGS) -o $(ORANGESKERNEL) $(OBJS)

kernel/kernel.o : kernel/kernel.asm
		$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/start.o : kernel/start.c include/type.h include/const.h include/protect.h
		$(CC) -m32 $(CFLAGS) -o $@ $<

lib/kliba.o : lib/kliba.asm
		$(ASM) $(ASMKFLAGS) -o $@ $<

lib/string.o : lib/string.asm
		$(ASM) $(ASMKFLAGS) -o $@ $<
