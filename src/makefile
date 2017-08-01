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
OBJS		  = kernel/kernel.o kernel/start.o kernel/main.o kernel/i8259.o kernel/global.o kernel/protect.o lib/klib.o lib/kliba.o lib/string.o 
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

kernel/kernel.o : kernel/kernel.asm include/sconst.inc
		$(ASM) $(ASMKFLAGS) -o $@ $<

# gcc -M kernel/start.c -I include // 自动生成依赖关系
kernel/start.o :kernel/start.c include/type.h include/const.h include/protect.h include/proto.h include/string.h \
 include/global.h 
		$(CC) -m32 $(CFLAGS) -o $@ $<

kernel/main.o : kernel/main.c include/type.h include/const.h include/protect.h include/string.h include/proc.h include/proto.h include/global.h
		$(CC) -m32 $(CFLAGS) -o $@ $<

kernel/i8259.o : kernel/i8259.c include/type.h include/const.h include/protect.h include/proto.h
		$(CC) -m32 $(CFLAGS) -o $@ $<

kernel/global.o : kernel/global.c include/type.h include/const.h include/protect.h include/proc.h include/global.h include/proto.h
		$(CC) -m32 $(CFLAGS) -o $@ $<

kernel/protect.o : kernel/protect.c include/type.h include/const.h include/protect.h include/proc.h include/proto.h include/global.h
		$(CC) -m32 $(CFLAGS) -o $@ $<

lib/klib.o : lib/klib.c include/type.h include/const.h include/protect.h include/string.h include/proc.h include/proto.h include/global.h
		$(CC) -m32 $(CFLAGS) -o $@ $<

lib/kliba.o : lib/kliba.asm
		$(ASM) $(ASMKFLAGS) -o $@ $<

lib/string.o : lib/string.asm
		$(ASM) $(ASMKFLAGS) -o $@ $<
