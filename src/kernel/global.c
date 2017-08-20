//
// Created by sulvto on 17-7-29.
//

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "fs.h"
#include "tty.h"
#include "console.h"
#include "proc.h"
#include "global.h"
#include "proto.h"

PUBLIC struct proc  proc_table[NR_TASKS + NR_PROCS];

PUBLIC char     task_stack[STACK_SIZE_TOTAL];

PUBLIC struct task      task_table[NR_TASKS] = {
        {task_tty, STACK_SIZE_TTY, "tty"},
        {task_sys, STACK_SIZE_SYS, "SYS"},
        {task_hd, STACK_SIZE_HD, "HD"},
        {task_fs, STACK_SIZE_FS, "FS"}};

PUBLIC struct task      user_proc_table[NR_PROCS] = {
        {TestA, STACK_SIZE_TESTA, "TaskA"},
        {TestB, STACK_SIZE_TESTB, "TaskB"},
        {TestC, STACK_SIZE_TESTC, "TaskC"}};

/**
 * For dd_map[k],
 *  'k' is the device nr.\ dd_map[k].driver_nr is the driver nr.
 *
 */
struct dev_drv_map dd_map[] = {
    // ddeiver nr.
    {INVALID_DRIVER},
    {INVALID_DRIVER},
    {INVALID_DRIVER},
    {TASK_HD},
    {TASK_TTY},
    {INVALID_DRIVER}
}


PUBLIC irq_handler irq_table[NR_IRQ];

PUBLIC system_call sys_call_table[NR_SYS_CALL] = {sys_printx, sys_sendrec};

PUBLIC TTY      tty_table[NR_CONSOLES];
PUBLIC CONSOLE  console_table[NR_CONSOLES];
