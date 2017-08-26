//
// Created by sulvto on 17-8-23.
//

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

/**
 * open/create a file
 * @param pathname  The full path of the file to be opened/created.
 * @param flags     O_CREAT, O_RDWR, ect.
 */
PUBLIC int open(const char *pathname, int flags) {
    MESSAGE msg;
    msg.type = OPEN;
    
    msg.PATHNAME = (void*)pathname;
    msg.FLAGS    = flags;
    msg.NAME_LEN = strilen(pathname);
    
    send_recv(BOTH, TASK_FS, &msg);
    assert(msg.type == SYSCALL_RET);

    return msg.FD;
}
