//
// Created by sulvto on 17-7-29.
//

#ifdef  GLOBAL_VARIABLES_HERE
#undef  EXTERN
#define EXTERN
#endif

EXTERN int                  disp_pos;
EXTERN u8                   gdt_ptr[6];
EXTERN struct descriptor    gdt[GDT_SIZE];
EXTERN u8                   idt_ptr[6];
EXTERN struct gate          idt[IDT_SIZE];

EXTERN u32          k_reenter;

EXTERN u32          key_pressed;


EXTERN struct tss       tss;
EXTERN struct proc*     p_proc_ready;

EXTERN int          ticks;

EXTERN int nr_current_console;

extern struct proc  proc_table[];
extern char         task_stack[];
extern struct task  task_table[];
extern struct task  user_proc_table[];
extern irq_handler  irq_table[];
extern TTY          tty_table[];
extern CONSOLE      console_table[];

// FS
EXTERN struct file_desc     f_desc_table[NR_FILE_DESC];
EXTERN struct inode         inode_table[NR_INODE];
EXTERN struct super_block   super_block[NR_SUPER_BLOCK];
extern u8 *         fsbuf;
extern const int    FSBUF_SIZE;
EXTERN MESSAGE      fs_msg;
EXTERN struct proc *        pcaller;
EXTERN struct inode *       root_inode;
extern struct dev_drv_map   dd_map[];
