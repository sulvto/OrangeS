//
// Created by sulvto on 17-7-29.
//

#ifdef  GLOBAL_VARIABLES_HERE
#undef  EXTERN
#define EXTERN
#endif

EXTERN int          disp_pos;
EXTERN u8           gdt_ptr[6];
EXTERN DESCRIPTOR   gdt[GDT_SIZE];
EXTERN u8           idt_ptr[6];
EXTERN GATE         idt[IDT_SIZE];

EXTERN u32          k_reenter; 

EXTERN TSS          tss;
EXTERN PROCESS*     p_proc_ready;

EXTERN int          ticks;

EXTERN int nr_current_console;

extern PROCESS      proc_table[];
extern char         task_stack[];
extern TASK         task_table[];
extern TASK         user_proc_table[];
extern irq_handler  irq_table[];
extern TTY          tty_table[];
extern CONSOLE      console_table[];

