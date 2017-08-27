//
// Created by sulvto on 17-7-29.
//

// klib.asm
PUBLIC void out_byte(u16 port,u8 value);
PUBLIC u8 in_byte(u16 info);
PUBLIC void disp_str(char *info);
PUBLIC void disp_color_str(char *info, int color);
PUBLIC void disable_irq(int irq);
PUBLIC void enable_irq(int irq);
PUBLIC void disable_int();
PUBLIC void enable_int();

// string.asm
PUBLIC char* strcpy(char* dst, const char* src);

// protect.c
PUBLIC void init_prot();
PUBLIC u32 seg2phys(u16 seg);

// klib.c
PUBLIC void delay(int time);
PUBLIC void delay_ini(int input);
PUBLIC char * itoa(char * str, int num);

// kernel.asm
PUBLIC void restart();

// main.c
PUBLIC int get_ticks();
PUBLIC void TestA();
PUBLIC void TestB();
PUBLIC void TestC();
PUBLIC void panic(const char *fmt,...); 

// i8259.c
PUBLIC void init_8259A();
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

// clock.c
PUBLIC void clock_handler(int irq);
PUBLIC void init_clock();
PUBLIC void milli_delay(int milli_sec);

// keyboard.c
PUBLIC void init_keyboard();
PUBLIC void keyboard_read(TTY* p_tty);

// kernel/hd.c
PUBLIC void task_hd();
PUBLIC void hd_handler(int irq);

// tty.c
PUBLIC void task_tty();
PUBLIC void in_process(TTY* p_tty, u32 key);

// systask.c
PUBLIC void task_sys();

// fs/main.c
PUBLIC void task_fs();
PUBLIC int rw_sector(int io_type, int dev, u64 pos, int bytes, int proc_nr, void* buf);
PUBLIC struct super_block *	get_super_block(int dev);

// fs/open.c
PUBLIC int do_open();
PUBLIC int do_close();

// fs/read_write.c
PUBLIC int do_rdwt();

// fs/misc.c
PUBLIC int strip_path(char * filename, const char * pathname, struct inode** ppinode);
PUBLIC int search_file(char * path);
PUBLIC struct inode * get_inode(int dev, int num);
PUBLIC void put_inode(struct inode * pinode);
PUBLIC void sync_inode(struct inode * p);

/* console.c */
PUBLIC void out_char(CONSOLE* p_con, char ch);
PUBLIC int is_current_console(CONSOLE* p_console); 
PUBLIC void select_console(int nr_console);
PUBLIC void scroll_screen(CONSOLE* p_console, int direction);
PUBLIC void init_screen(TTY* p_tty);

// printf.c
PUBLIC int printf(const char *fmt, ...);
#define printl printf

// vsprintf.c
PUBLIC int vsprintf(char *buf, const char *fmt, va_list args);
PUBLIC int sprintf(char *buf, const char *fmt, ...);


// proc.c
PUBLIC void schedule();
PUBLIC void* va2la(int pid, void* va);
PUBLIC int ldt_seg_linear(struct proc* p, int idx);
PUBLIC void reset_msg(MESSAGE* m);
PUBLIC int send_recv(int function,int src_dest, MESSAGE* msg);
PUBLIC void inform_int(int task_nr);

// lib/misc.c
PUBLIC void spin(char * func_name);

// 系统调用相关

// 系统调用 - 系统级
// proc.c
PUBLIC int sys_sendrec(int function, int src_dest, MESSAGE* m, struct proc* p);
PUBLIC int sys_printx(int _unused1, int _unused2, char* s, struct proc* p_proc);
// system.asm
PUBLIC void sys_call();

// 系统调用 - 用户级
PUBLIC int  sendrec(int function, int src_dest, MESSAGE* p_msg);
PUBLIC void printx(char* str);
