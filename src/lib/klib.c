//
// Created by sulvto on 17-7-29.
//

#include    "type.h"
#include    "config.h"
#include    "stdio.h"
#include    "const.h"
#include    "protect.h"
#include    "tty.h"
#include    "console.h"
#include    "string.h"
#include    "fs.h"
#include    "proc.h"
#include    "global.h"
#include    "proto.h"

#include "elf.h"

/**
 * 数字前面的 0 不被显示出来, 0000B800 --> B800
 * 
 */
PUBLIC char * itoa(char *str, int num) {
    char *p = str;
    int flag = 0;
    *p++ = '0';
    *p++ = 'x';
    if (num == 0) {
        *p++ = '0';
    } else {
        for (int i = 28; i >= 0; i -= 4) {
            char ch = (num >> i) & 0xF;
            if (flag || (ch > 0)) {
                flag = 1;
                ch += '0';
                if (ch > '9') {
                    ch += 7;
                }
                *p++ = ch;
            }
        }
    }
    *p = 0;
    return str;
}


/**
 * disp int
 */
PUBLIC void disp_int(int input) {
    char output[16];
    itoa(output, input);
    disp_str(output);
}


PUBLIC void delay(int time) {
    int i,j,k;
    for(k=0;k < time; k++) {
        for(j=0;j < 100; j++) {
            for(i=0;i < 10000; i++) { }
        }   
    }
}


/**
 * <Ring 0-1> The boot parameters have been saved by LOADER.
 *			we just read them out.
 *
 * @param pbp Ptr to the boot params structure
 */
PUBLIC void get_boot_params(struct boot_params * pbp) {
	int * p = (int*)BOOT_PARAM_ADDR;
	assert(p[BI_MAG] == BOOT_PARAM_MAGIC);
	
	pbp->mem_size = p[BI_MEM_SIZE];
	pbp->kernel_file = (unsigned char*) (p[BI_KERNEL_FILE]);

	assert(memcmp(pbp->kernel_file, ELFMAG, SELFMAG) == 0);
}


/**
 * <Ring 0-2> Parse the kernel file, get the memory range of the kernel image.
 *
 * - The meaning of 'base':	base => first_vaild_byte
 * - The meaning of 'limit': base + limit => last_vaild_byte
 *
 *
 * @param base 		Memory base of kernel.
 * @param limit 	Memory limit of kernel.
 *
 */
PUBLIC int get_kernel_map(unsigned int *base, unsigned int * limit) {
	struct boot_params bp;
	get_boot_params(&bp);
	
	Elf32_Ehdr* elf_header = (Elf32_Ehdr*)(bp.kernel_file);

	if (memcmp(elf_header->e_ident, ELFMAG, SELFMAG) != 0) {
		return -1;
	}

	*base = ~0;
	unsigned int t = 0;
	for (int i = 0; i < elf_header->e_shnum; i++) {
		Elf32_Shdr* section_hander = 
				(Elf32_Shdr*)(bp.kernel_file + elf_header->e_shoff + i * elf_header->e_shentsize);

		if (section_hander->sh_flags & SHF_ALLOC) {
			int bottom = section_hander->sh_addr;
			int top = section_hander->sh_addr + section_hander->sh_size;

			if (*base > bottom) {
				*base = bottom;
			} 
			if (t < top){
				t = top;
			}
		}
	}

	assert(*base < t);
	*limit = t - *base -1;

	return 0;
}
