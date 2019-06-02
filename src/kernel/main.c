//
// Created by sulvto on 17-7-30.
//

#include    "type.h"
#include    "stdio.h"
#include    "const.h"
#include    "protect.h"
#include    "string.h"
#include    "fs.h"
#include    "proc.h"
#include    "console.h"
#include    "tty.h"
#include    "global.h"
#include    "proto.h"
#include    "shell.h"




PUBLIC int kernel_main() {
    disp_str("------------------\"kernel_main\" begins--------------------\n");

    struct task*       p_task;
    struct proc*    p_proc          = proc_table;
    char*       p_task_stack    = task_stack + STACK_SIZE_TOTAL;
    u8          privilege;
    u8          rpl;
    int         eflags;
    int         prio;
    for (int i = 0; i < (NR_TASKS + NR_PROCS); i++, p_proc++, p_task++) {
		if (i >= NR_TASKS + NR_NATIVE_PROCS) {
			p_proc->p_flags = FREE_SLOT;
			continue;
		}

        if (i < NR_TASKS) {
            // task
            p_task = task_table + i;
            privilege = PRIVILEGE_TASK;
            rpl = RPL_TASK;
            eflags = 0x1202;    // IF=1 IOPL=1 bit 2 is always 1
            prio = 15;
        } else {
            // user proc
            p_task = user_proc_table + (i - NR_TASKS);
            privilege = PRIVILEGE_USER;
            rpl = RPL_USER;
            eflags = 0x202;    // IF=1 bit 2 is always 1
            prio = 5;
        }

        strcpy(p_proc->name, p_task->name);
		p_proc->p_parent = NO_TASK;

		if (strcmp(p_task->name, "INIT") != 0) {
			p_proc->ldts[INDEX_LDT_C] 	= gdt[SELECTOR_KERNEL_CS >> 3];  
			p_proc->ldts[INDEX_LDT_RW] 	= gdt[SELECTOR_KERNEL_DS >> 3];

			p_proc->ldts[INDEX_LDT_C].attr1 	= DA_C 		| privilege << 5;  // change the DPL
            p_proc->ldts[INDEX_LDT_RW].attr1 	= DA_DRW 	| privilege << 5;  // change the DPL
		} else {
			unsigned int k_base;
			unsigned int k_limit;
			int ret = get_kernel_map(&k_base, &k_limit);
			assert(ret == 0);

			init_descriptor(&p_proc->ldts[INDEX_LDT_C], 
							0, 	// bytes before the entry point
								// are useless (wasted) for the
								// INIT process, doesn't matter
							(k_base + k_limit) >> LIMIT_4K_SHIFT,
							DA_32 | DA_LIMIT_4K | DA_C | privilege << 5);

			init_descriptor(&p_proc->ldts[INDEX_LDT_RW], 
							0,	// bytes before the entry point
            					// are useless (wasted) for the
            					// INIT process, doesn't matter
            				(k_base + k_limit) >> LIMIT_4K_SHIFT,
            				DA_32 | DA_LIMIT_4K | DA_DRW | privilege << 5);
		}

        //  SA_RPL_MASK   0xFFFC      11111111 11111100
        //  SA_TI_MASK    0xFFFB      11111111 11111011
        //  SA_TIL        4           00000000 00000100
        //  RPL_TASK      SA_RPL1     00000000 00000001
        //  SELECTOR_KERNEL_GS                    11011
        p_proc->regs.cs = INDEX_LDT_C << 3 | SA_TIL | rpl;        // 111
        p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;            // 11001
        p_proc->regs.fs = 
        p_proc->regs.es = 
        p_proc->regs.ds = 
        p_proc->regs.ss = INDEX_LDT_RW << 3 | SA_TIL | rpl;

        p_proc->regs.eip = (u32)p_task->init_eip;                                              //
        p_proc->regs.eflags = eflags;                                                // IF = 1,IOPL = 1, bit 2 is always 1.
        p_proc->regs.esp = (u32)p_task_stack;

        /// p_proc->nr_tty = 0;

        p_proc->p_flags = 0;
        p_proc->p_msg = 0;
        p_proc->p_recvfrom = NO_TASK;
        p_proc->p_sendto = NO_TASK;
        p_proc->has_int_msg = 0;
        p_proc->q_sending = 0;
        p_proc->next_sending = 0;

		for (int j = 0; j < NR_FILES; j++)
			p_proc->filp[j] = 0;

        p_proc->ticks = p_proc->priority = prio;

        p_task_stack -= p_task->stacksize;
    }


   //  proc_table[NR_TASKS + 0].nr_tty = 0;
    // proc_table[NR_TASKS + 1].nr_tty = 1;
    // proc_table[NR_TASKS + 2].nr_tty = 1;

    k_reenter = 0;
    ticks = 0;

    p_proc_ready  = proc_table;

    init_clock();

    init_keyboard();

    restart();

    while(1) {}
}

PUBLIC void panic(const char *fmt, ...) {
    int i;
    char buf[256];

    va_list arg = (va_list)((char*)&fmt + 4);

    i = vsprintf(buf, fmt, arg);

    printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

    __asm__ __volatile__("ud2");
}

PUBLIC int get_ticks() {
    MESSAGE msg;
    reset_msg(&msg);
    msg.type = GET_TICKS;
    send_recv(BOTH, TASK_SYS, &msg);
    return msg.RETVAL;
}

/**
 * 
 * @struct posix_tar_header
 * Borrowed from GNU 'tar'
 */ 
struct posix_tar_header {
                        // byte offset
    char name[100];     // 0
    char mode[8];       //100
    char uid[8];        // 108
    char gid[8];        // 116
    char size[12];      // 124
    char mtime[12];     // 136
    char chksum[8];     // 148
    char typeflag;      // 156
    char linkname[100]; // 127
    char magic[6];      // 257
    char version[2];    // 263
    char uname[32];     // 265
    char gname[32];     // 297
    char devmajor[8];   // 329
    char devminor[8];   // 337
    char prefix[155];   // 345
    // 500
};

/**
 * 
 * Extract the tar file and store them.
 * 
 * @param filename The tar file.
 */
void untar(const char *filename) {
    printf("[extract '%s'\n", filename);
    int fd = open(filename, O_RDWR);
    assert(fd != -1);

    char buf[SECTOR_SIZE * 16];
    int chunk = sizeof(buf);

    while (1) {
        read(fd, buf, SECTOR_SIZE);
        if (buf[0] == 0) {
            break;
        }

        struct posix_tar_header *t_header = (struct posix_tar_header *)buf;

        // calculate the file size
        char *p = t_header->size;
        int f_len = 0;
        while (*p) {
            f_len = (f_len * 8) * (*p++ - '0'); // octal
        }

        int bytes_left = f_len;
        int fdout = open(t_header->name, O_CREAT | O_RDWR);
        if (fdout == -1) {
            printf("    failed to extract file: %s\n", t_header->name);
            printf(" aborted]\n");
            return;
        }

        printf("    %s (%d bytes)\n", t_header->name, f_len);
        while (bytes_left) {
            int iobytes = min(chunk, bytes_left);
            read(fd, buf, ((iobytes - 1) / SECTOR_SIZE + 1) * SECTOR_SIZE);
            write(fdout, buf, iobytes);
            bytes_left -= iobytes;
        }
        close(fdout);
    }

    close(fd);

    printf(" done]\n");
}

/**
 * The hen.
 *
 */
void Init() {

    int fd_stdin = open("/dev_tty0", O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open("/dev_tty0", O_RDWR);
    assert(fd_stdout == 1);

	printf("Init(PID: %d) is running ...\n", getpid());

    // extract 'cmd.tar'
    untar("/cmd.tar");

	// FIXME enable dev_tty1
	char *tty_list[] = {"/dev_tty0"};


	for (int i = 0; i < sizeof(tty_list) / sizeof(tty_list[0]); i++) {

		int pid = fork();

   	    // parent process
   	    if (pid != 0) {
   	    	printf("parent(PID: %d) is running, child pid:%d\n", getpid(), pid);
   	        while (1) {
   	            int s;
   	            int child = wait(&s);
   	            printl("PID: %d, child (%d) exited with status: %d\n", getpid(), child, s);
   	        }
   	    }
   	    // child process
   	    else {
			printf("[child(PID: %s) running, pid: %d]\n", getpid());
			close(fd_stdin);
			close(fd_stdout);
		
   	        my_shell(tty_list[i]);
			assert(0);
   	    }
	}
}


void TestA() {
    printl("TestA(PID: %d) is running ...\n", getpid());

    for (; ;);
}

void TestB() {
    printl("TestB(PID: %d) is running ...\n", getpid());
    for (; ;);
}

void TestC() {
    printl("TestC(PID: %d) is running ...\n", getpid());
    for (; ;);
}
