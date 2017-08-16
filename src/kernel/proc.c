//
// Created by sulvto on 17-8-9.
//

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"


/**
 * sys_get_ticks
 */
PUBLIC int sys_get_ticks() {
    return ticks;
}

PUBLIC void schedule() {
    PROCESS *p;
    int greatest_ticks = 0;
    while (!greatest_ticks) {
        for (p = proc_table; p < proc_table + NR_TASKS + NR_PROCS; p++) {
            if (greatest_ticks < p->ticks) {
                greatest_ticks = p->ticks;
                p_proc_ready = p;
            }
        }



        if (!greatest_ticks) { ;
            for (p = proc_table; p < proc_table + NR_TASKS + NR_PROCS; p++) { ;
                p->ticks = p->priority;;
            };
        }
    }
}

/**
 * <Ring 0> The core routine of system call 'sendrec()'.
 *
 * @param function  SEND or RECEIVE
 * @param src_dest  To/From whom the message is transferred.
 * @param m         Ptr to the MESSAGE body.
 * @param p         The caller proc.
 *
 * @return Zero if success
 *  
 */
PUBLIC int sys_sendrec(int function, int src_dest, MESSAGE* m, struct proc* p) {
    assert(k_reenter == 0); // make sure we are not in ring0
    assert((src_dest >= 0 && src_dest < NR_TASKS + NR_PROCS) || src_dest == ANY || src_dest == INTERRUPT);
    
    int ret = 0;
    int caller = proc2pid(p);
    MESSAGE* mla = (MESSAGE*)va2la(caller, m);

    assert(mla->source != src_dest);

    if (function == SEND) {
        ret = msg_send(p, src_dest, m);
        if (ret != 0) return ret;
    }else if (function == RECEIVE) {
        ret = msg_receive(p, src_dest, m);
        if (ret != 0) return ret;
    }else {
        panic("{sys_sendrec} invalid function: %d (SEND:%d, RECEIVE:%d).", function, SEND, RECEIVE);
    }
    return 0;
}
