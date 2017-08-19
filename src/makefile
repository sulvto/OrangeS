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
CFLAGS		= -I include/ -c -fno-builtin -Wall  -m32
LDFLAGS		= -m elf_i386 -s -Ttext $(ENTRYPOINT) -Map krnl.map
DASMFLAGS	= -D

# This Program
ORANGESBOOT	  = boot/boot.bin boot/loader.bin
ORANGESKERNEL = kernel.bin
OBJS		  = kernel/kernel.o kernel/syscall.o kernel/start.o \
				kernel/main.o kernel/clock.o kernel/keyboard.o \
				kernel/tty.o kernel/console.o kernel/i8259.o \
				kernel/global.o kernel/protect.o kernel/proc.o kernel/systask.o \
				kernel/printf.o kernel/vsprintf.o lib/kliba.o \
				lib/klib.o lib/string.o lib/misc.o
DASMOUTPUT	  = kernel.bin.asm

.PHONY : everything final image clean realclean disasm all buildimg

everything : $(ORANGESBOOT) $(ORANGESKERNEL)
all : realclean everything
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

kernel/syscall.o : kernel/syscall.asm include/sconst.inc
		$(ASM) $(ASMKFLAGS) -o $@ $<

# gcc -M kernel/start.c -I include // 自动生成依赖关系
kernel/start.o :kernel/start.c include/type.h include/const.h include/protect.h include/proto.h include/string.h \
 include/global.h 
		$(CC) $(CFLAGS) -o $@ $<

kernel/main.o : kernel/main.c include/type.h include/const.h include/protect.h include/string.h include/proc.h include/proto.h include/global.h
		$(CC) $(CFLAGS) -o $@ $<

kernel/clock.o: kernel/clock.c
		$(CC) $(CFLAGS) -o $@ $<

kernel/proc.o: kernel/proc.c
		$(CC) $(CFLAGS) -o $@ $<

kernel/keyboard.o: kernel/keyboard.c
		$(CC) $(CFLAGS) -o $@ $<

kernel/tty.o: kernel/tty.c
		$(CC) $(CFLAGS) -o $@ $<

kernel/console.o: kernel/console.c
		$(CC) $(CFLAGS) -o $@ $<

kernel/i8259.o : kernel/i8259.c include/type.h include/const.h include/protect.h include/proto.h
		$(CC) $(CFLAGS) -o $@ $<

kernel/global.o : kernel/global.c include/type.h include/const.h include/protect.h include/proc.h include/global.h include/proto.h
		$(CC) $(CFLAGS) -o $@ $<

kernel/protect.o : kernel/protect.c include/type.h include/const.h include/protect.h include/proc.h include/proto.h include/global.h
		$(CC) $(CFLAGS) -o $@ $<

kernel/printf.o: kernel/printf.c
		$(CC) $(CFLAGS) -o $@ $<

kernel/vsprintf.o: kernel/vsprintf.c
		$(CC) $(CFLAGS) -o $@ $<

kernel/systask.o : kernel/systask.c
		$(CC) $(CFLAGS) -o $@ $<

lib/klib.o : lib/klib.c include/type.h include/const.h include/protect.h include/string.h include/proc.h include/proto.h include/global.h
		$(CC) $(CFLAGS) -o $@ $<

lib/misc.o: lib/misc.c
		$(CC) $(CFLAGS) -o $@ $<

lib/kliba.o : lib/kliba.asm
		$(ASM) $(ASMKFLAGS) -o $@ $<

lib/string.o : lib/string.asm
		$(ASM) $(ASMKFLAGS) -o $@ $<
