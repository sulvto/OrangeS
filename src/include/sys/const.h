//
// Created by sulvto on 17-7-28.
//

#ifndef _ORANGES_CONST_H_
#define _ORANGES_CONST_H_

// min(); max();
#define max(a,b) ((a) > (b) ? (a) : (b));
#define min(a,b) ((a) < (b) ? (a) : (b));

/**
 * Color
 */
#define BLACK   0x0     // 0000
#define WHITE   0x7     // 0111
#define RED     0x4     // 0100
#define GREEN   0x2     // 0010
#define BLUE    0x1     // 0001
#define FLASH   0x80    // 1000 0000
#define BRIGHT  0x08    // 0000 1000
#define MAKE_COLOR(x,y) ((x<<4) | y)


/*
 * GDT 和 IDT 中描述符的个数
 */
#define GDT_SIZE    128
#define IDT_SIZE    256

// 权限
#define PRIVILEGE_KRNL  0
#define PRIVILEGE_TASK  1
#define PRIVILEGE_USER  3

// RPL
#define RPL_KRNL    SA_RPL0
#define RPL_TASK    SA_RPL1
#define RPL_USER    SA_RPL3

// 8259A
#define  INT_M_CTL      0x20
#define  INT_M_CTLMASK  0x21
#define  INT_S_CTL      0xA0
#define  INT_S_CTLMASK  0xA1

// 8253/8254 PIT
#define  TIMER0         0x40
#define  TIMER_MODE     0x43
#define  RATE_GENERATOR 0x34

#define  TIMER_FREQ     1193182L
#define  HZ             100

// Process
#define SENDING         0x02
#define RECEIVING       0x04

// TTY
#define NR_CONSOLES     3

// AT keyboard
// 8042 ports
#define  KB_DATA    0x60
#define  KB_CMD     0x64

#define  NR_IRQ     16
#define  CLOCK_IRQ  0
#define  KEYBOARD_IRQ   1
#define  CASCADE_IRQ    2

#define  AT_WINI_IRQ    14   

// VGA
#define CRTC_ADDR_REG   0x3D4       // CRT Controller Registers - Addr Registers
#define CRTC_DATA_REG   0x3D5       // CRT Controller Registers - Data Registers
#define START_ADDR_H    0xC         // reg index of video mem start addr (MSB)
#define START_ADDR_L    0xD         // reg index of video mem start addr (LSB)
#define CURSOR_H        0xE         // reg index of cursor position (MSB)
#define CURSOR_L        0xF         // reg index of cursor position (LSB)
#define V_MEM_BASE      0xB8000     // base of color video memory
#define V_MEM_SIZE      0x8000      // 32K: B8000H -> BFFFFH

#define LED_CODE        0xED
#define KB_ACK          0xFA

// task
#define INVALID_DRIVER  -20
#define INTERRUPT       -10
#define TASK_TTY        0
#define TASK_SYS        1
#define TASK_HD         2
#define TASK_FS         3
#define ANY             (NR_TASKS + NR_PROCS + 10)
#define NO_TASK         (NR_TASKS + NR_PROCS + 20)


#define NR_SYS_CALL     3

// ipc
#define SEND            1
#define RECEIVE         2
#define BOTH            3       // BOTH = (SEND | RECEIVE)

// magic chars used by 'printx'
#define MAG_CH_PANIC    '\002'
#define MAG_CH_ASSERT   '\003'

enum msgtype {
    // 
    HARD_INT = 1,
    // SYS task
    GET_TICKS,
    // FS
    OPEN, CLOSE, READ, WRITE,
    // TTY, SYS, FS, MM, etc
    SYSCALL_RET,

    // message type for  drivers
    DEV_OPEN = 1001,
    DEV_CLOSE,
    DEV_READ,
    DEV_WRITE,
    DEV_IOCTL
};

// macros for messages
#define FD          u.m3.m3i2
#define PATHNAME    u.m3.m3i2
#define FLAGS       u.m3.m3i2
#define NAME_LEN    u.m3.m3i2
#define CNT         u.m3.m3i2
#define REQUEST     u.m3.m3i2
#define PROC_NR     u.m3.m3i3
#define DEVICE      u.m3.m3i4
#define POSITION    u.m3.m3l1
#define BUF         u.m3.m3p2
#define RETVAL      u.m3.m3i1







#define DIOCTL_GET_GEO 1

// Hard Drive
#define SECTOR_SIZE     512
#define SECTOR_BITS     (SECTOR_SIZE * 8)
#define SECTOR_SIZE_SHIFT   9

// major device numbers
#define NO_DEV          0
#define DEV_FLOPPY      1
#define DEV_CDROM       2
#define DEV_HD          3
#define DEV_CHAR_TTY    4
#define DEV_SCSI        5

// make device number from major and minor nu,bers
#define MAJOR_SHIFT     8
#define MAKE_DEV(a,b)   ((a << MAJOR_SHIFT) | b)

// separate major and minor numbers from device number
#define MAJOR(x)        ((x >> MAJOR_SHIFT) & 0xFF)
#define MINOR(x)        (x & 0xFF)

// device numbers of hard disk
#define MINOR_hd1a          0x10
#define MINOR_hd2a          (MINOR_hd1a + NR_SUB_PER_PART)

#define ROOT_DEV            MAKE_DEV(DEV_HD, MINOR_BOOT)

#define INVALID_INODE       0
#define ROOT_INODE          1

#define MAX_DRIVES      2
#define NR_PART_PER_DRIVE   4
#define NR_SUB_PER_PART     16
#define NR_SUB_PER_DRIVE    (NR_SUB_PER_PART * NR_PART_PER_DRIVE)
#define NR_PRIM_PER_DRIVE   (NR_PART_PER_DRIVE + 1)

#define MAX_PRIM            (MAX_DRIVES * NR_PRIM_PER_DRIVE - 1)
#define MAX_SUBPARTITIONS   (NR_SUB_PER_DRIVE * MAX_DRIVES)

#define P_PRIMARY       0
#define P_EXTENDED      1

#define ORANGES_PART    0x99
#define NO_PART         0x00
#define EXT_PART        0x05

#define NR_FILES        64
#define NR_FILE_DESC    64
#define NR_INODE        64
#define NR_SUPER_BLOCK  8

// INODE::i_mode(sctal, lower 32 bits reserved)
#define I_TYPE_MASK     0170000
#define I_REGULAR       0100000
#define I_BLOCK_SPECIAL 0060000
#define I_DIRECTORY     0040000
#define I_CHAR_SPECIAL  0020000
#define I_NAMED_PIPE    0010000

#define NR_DEFAULT_FILE_SECTS   2048

#endif

