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


PRIVATE void init_fs();

/**
 * <Ring 1> The main loop of TASK FS
 */
PUBLIC void task_fs() {
    printl("Task FS begins.\n");
    init_fs();
    spin("FS");
}

/**
 * <Ring 1> Do some preparation.
 */
PRIVATE void init_fs() {
    // open the device:hard disk
    MESSAGE driver_msg;
    driver_msg.type = DEV_OPEN;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

    mkfs();
}

PRIVATE void mkfs() {
    // TODO
}
