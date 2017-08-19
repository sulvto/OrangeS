//
// Created by sulvto on 17-8-19.
//
#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

#include "hd.h"

PUBLIC void task_fs() {
    printl("Task FS begins.\n");
    MESSAGE driver_msg;
    driver_msg.type = DEV_OPEN;
    send_recv(BOTH, TASK_HD, &driver_msg);

    spin("FS");
}
