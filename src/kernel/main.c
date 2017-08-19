//
// Created by sulvto on 17-7-30.
//

#include    "type.h"
#include    "const.h"
#include    "protect.h"
#include    "string.h"
#include    "proc.h"
#include    "console.h"
#include    "tty.h"
#include    "global.h"
#include    "proto.h"




PUBLIC int kernel_main() {
    disp_str("------------------\"kernel_main\" begins--------------------\n");

    struct task*       p_task;
    struct proc*    p_proc          = proc_table;
    char*       p_task_stack    = task_stack + STACK_SIZE_TOTAL;
    u16         selector_ldt    = SELECTOR_LDT_FIRST;
    u8          privilege;
    u8          rpl;
    int         eflags;
    for (int i = 0; i < (NR_TASKS + NR_PROCS); i++) {
        if (i < NR_TASKS) {
            p_task = task_table + i;
            privilege = PRIVILEGE_TASK;
            rpl = RPL_TASK;
            eflags = 0x1202;    // IF=1 IOPL=1 bit 2 is always 1
        }else {
            p_task = user_proc_table + (i - NR_TASKS);                 
            privilege = PRIVILEGE_USER;
            rpl = RPL_USER;
            eflags = 0x202;    // IF=1 bit 2 is always 1
        }

        strcpy(p_proc->name,p_task->name);
        p_proc->pid = i;
        p_proc->ldt_sel = selector_ldt;
        p_proc->ldt_sel = SELECTOR_LDT_FIRST;

        memcpy(&p_proc->ldts[0],&gdt[SELECTOR_KERNEL_CS >> 3],sizeof(struct descriptor));
        p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;  // change the DPL
        memcpy(&p_proc->ldts[1],&gdt[SELECTOR_KERNEL_DS >> 3],sizeof(struct descriptor));
        p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;  // change the DPL

        //  SA_RPL_MASK   0xFFFC      11111111 11111100
        //  SA_TI_MASK    0xFFFB      11111111 11111011
        //  SA_TIL        4           00000000 00000100
        //  RPL_TASK      SA_RPL1     00000000 00000001
        //  SELECTOR_KERNEL_GS                    11011
        p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | RPL_TASK;            // 11001 
        p_proc->regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;       // 1101   
        p_proc->regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;       // 1101 
        p_proc->regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
        p_proc->regs.eip = (u32)p_task->init_eip;                                              //
        p_proc->regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;        // 111 
        p_proc->regs.eflags = 0x1202;                                                // IF = 1,IOPL = 1, bit 2 is always 1.
        p_proc->regs.esp = (u32)p_task_stack;
        p_proc->regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
        
        p_proc->nr_tty = 0;
        
        p_task_stack -= p_task->stacksize;
        p_proc++;
        p_task++;
        selector_ldt += 1<< 3;
    }



    proc_table[0].ticks = proc_table[0].priority = 15;
    proc_table[1].ticks = proc_table[1].priority = 5;
    proc_table[2].ticks = proc_table[2].priority = 3;

    proc_table[1].nr_tty = 0;
    proc_table[2].nr_tty = 1;
    proc_table[3].nr_tty = 1;

    k_reenter = 0;
    ticks = 0;

    p_proc_ready  = proc_table;

    init_clock();

    init_keyboard();

    restart();

    while(1){}
}

PUBLIC void panic(const char *fmt, ...) {
    int i;
    char buf[256];

    va_list arg = (va_list)((char*)&fmt + 4);
    
    i = vsprintf(buf, fmt, arg);

    printl("%c !!painc!! %s", MAG_CH_PANIC, buf);

    __asm__ __volatile__("ud2");
}

PUBLIC int get_ticks() {
    MESSAGE msg;
    reset_msg(&msg);
    msg.type = GET_TICKS;
    send_recv(BOTH, TASK_SYS, &msg);
    return msg.RETVAL;
}


void TestA() {
    while(1) {

        printf("<Ticks:%x>",get_ticks());
        milli_delay(10);
    }
}

void TestB() {
    int i=0X1000;
    while(1) {
        printf("B.");
        milli_delay(10);
    }
}

void TestC() {
    int i=0X2000;
    while(1) {
        printf("C.");
        milli_delay(10);
    }
}
