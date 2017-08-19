//
// Created by sulvto on 17-7-27.
//

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

PUBLIC void cstart() {
    disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                     "--------\"cstart\" begins------\n");
    memcpy(&gdt,
           (void *) (*((u32 * )(&gdt_ptr[2]))),
           *((u16 * )(&gdt_ptr[0])) + 1);

    // gdt_ptr[6] 共6个字节：0～15：Limit  16～47:Base.
    u16 *p_gdt_limit = (u16 * )(&gdt_ptr[0]);
    u32 *p_gdt_base = (u32 * )(&gdt_ptr[2]);
    *p_gdt_limit = GDT_SIZE * sizeof(struct descriptor) - 1;
    *p_gdt_base = (u32) & gdt;

    // idt_ptr[6] 共6个字节：0～15：Limit  16~47:Base.
    u16 *p_idt_limit = (u16* )(&idt_ptr[0]);
    u32 *p_idt_base = (u32* )(&idt_ptr[2]);
    *p_idt_limit = IDT_SIZE * sizeof(struct gate) - 1;
    *p_idt_base = (u32) &idt;

    init_prot();

    disp_str("--------\"cstart\" ends------\n");
}
