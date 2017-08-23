//
// Created by sulvto on 17-7-30.
//

struct stackframe {
        u32 gs;
        u32 fs;
        u32 es;
        u32 ds;
        u32 edi;
        u32 esi;
        u32 ebp;
        u32 kernel_esp;
        u32 ebx;
        u32 edx;
        u32 ecx;
        u32 eax;
        u32 retaddr;
        u32 eip;
        u32 cs;
        u32 eflags;
        u32 esp;
        u32 ss;
};

struct proc {
    struct stackframe regs;             // process registers saved in stack frame

    u16 ldt_sel;                        // local descriptors for code and data */
    struct descriptor  ldts[LDT_SIZE];  // process id passed in from MM

    int ticks;
    int priority;

    u32 pid;
    char name[16];                      // name of the process

    int p_flags;                        // process flags. A proc runnable if p_flags == 0.
    MESSAGE * p_msg;
    int p_recvfrom;
    int p_sendto;

    int has_int_msg;
    
    struct proc * q_sending;

    struct proc * next_sending;
    
    int nr_tty;

    struct file_desc * filp[NR_FILES];
};

struct task {
    task_f  init_eip;
    int     stacksize;
    char    name[32];
}TASK;

#define proc2pid(x) (x - proc_table)


// Number of tasks & procs
#define NR_TASKS    4
#define NR_PROCS    3
#define FIRST_PROC  proc_table[0]
#define LAST_PROC   proc_table[NR_TASKS + NR_PROCS -1]

#define STACK_SIZE_TTY          0x8000
#define STACK_SIZE_SYS          0x8000
#define STACK_SIZE_HD           0x8000
#define STACK_SIZE_FS           0x8000
#define STACK_SIZE_TESTA        0x8000
#define STACK_SIZE_TESTB        0x8000
#define STACK_SIZE_TESTC        0x8000

#define STACK_SIZE_TOTAL        (STACK_SIZE_TTY + STACK_SIZE_SYS + STACK_SIZE_HD + STACK_SIZE_FS + STACK_SIZE_TESTA + STACK_SIZE_TESTB + STACK_SIZE_TESTC)

