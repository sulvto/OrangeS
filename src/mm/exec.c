//
// Created by sulvto on 18-11-25.
//

#include    "type.h"
#include    "stdio.h"
#include    "const.h"
#include    "protect.h"
#include    "string.h"
#include    "fs.h"
#include    "proc.h"
#include    "tty.h"
#include    "console.h"
#include    "global.h"
#include    "proto.h"



/**
 * Perform the exec() syscall.
 *
 * @return Zero if success, otherwise -1.
 */
PUBLIC int do_exec() {
    printl("[MM] do_exec\n");

    // get parameters from the message.
    int name_len = mm_msg.NAME_LEN;
    int src = mm_msg.source;
    assert(name_len < MAX_PATH);

    char pathname[MAX_PATH];
    phy_copy((void*)va2la(TASK_MM, pathname), (void*)va2la(src, mm_msg.PATHNAME), name_len);
    pathname[name_len] = 0;

    // get the file size.
    // why get file??
    return 0;
}