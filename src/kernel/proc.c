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

PRIVATE void block(struct proc* p);
PRIVATE void unblock(struct proc* p);
PRIVATE int msg_send(struct proc* current, int dest, MESSAGE* m);
PRIVATE int msg_receive(struct proc* current, int src, MESSAGE* m);
PRIVATE int deadlock(int src, int dest);

/**
 * sys_get_ticks
 */
PUBLIC int sys_get_ticks() {
    return ticks;
}

PUBLIC void schedule() {

    struct proc *p;
    int greatest_ticks = 0;
    while (!greatest_ticks) {
        for (p = &FIRST_PROC; p <= &LAST_PROC; p++) {
            if (0 == p->p_flags) {
                if (greatest_ticks < p->ticks) {
                    greatest_ticks = p->ticks;
                    p_proc_ready = p;
                }
            }
        }



        if (!greatest_ticks) {
            for (p = &FIRST_PROC; p <= &LAST_PROC; p++) { 
                if (p->p_flags == 0){
                    p->ticks = p->priority;;
                }
            }
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
    mla->source = caller;
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


PUBLIC int send_recv(int function, int src_dest, MESSAGE* msg) {
    int ret = 0;
    
    if (function == RECEIVE) {
        memset(msg, 0, sizeof(MESSAGE));
    }

    switch (function) {
        case BOTH:
            ret = sendrec(SEND, src_dest, msg);
            if (ret == 0) {
                ret = sendrec(RECEIVE, src_dest, msg);
            }
            break;
        case SEND:
        case RECEIVE:
            ret = sendrec(function, src_dest, msg);
            break;
        default:
            assert((function == BOTH) || (function == SEND) || (function == RECEIVE));
            break;
    }

    return ret;
}


/**
 * <Ring 0-1>
 * @param p Whose (the proc ptr).
 * @param idx Which (one proc has more than one segments)
 * @return The required linear address
 */
PUBLIC int ldt_seg_linear(struct proc* p, int idx) {
    struct descriptor * d = &p->ldts[idx];

    return d->base_high << 24 | d->base_mid << 16 | d->base_low;
}

/**
 * <Ring 0-1> Virtual addr --> Linear addr.
 *
 * @param pid PID of the proc whose address id to be calculated
 * @param va Virtual address.
 * @return The linear address for the given virtual address.
 */
PUBLIC void* va2la(int pid, void* va) {
    struct proc* p = &proc_table[pid];

    u32 seg_base = ldt_seg_linear(p, INDEX_LDT_RW);
    u32 la = seg_base + (u32)va;

    if (pid < NR_TASKS + NR_PROCS) {
        assert(la == (u32)va);
    }

    return (void*)la;
}

/**
 * <Ring 0-3> Clear a MESSAGE by setting each byte to 0.
 *
 * @param p The message to be cleared.
 */
PUBLIC void reset_msg(MESSAGE* p) {
    memset(p, 0, sizeof(MESSAGE));
}

/**
 *
 * @param p The proc to be blocked.
 */
PRIVATE void block(struct proc* p) {
    assert(p->p_flags);
    schedule();
}

/**
 * @param The unblocked proc.
 */
PRIVATE void unblock(struct proc* p) {
    assert(p->p_flags == 0);
}

/**
 * <Ring 0>
 *
 * @param src Who wants to send message.
 * @param dest To whom te message is sent.
 * @return Zreo if success.
 */
PRIVATE int deadlock(int src, int dest) {
    struct proc* p = proc_table + dest;
    while (1) {
        if (p->p_flags & SENDING) {
            if (p->p_sendto == src) {
                // print the chain
                p = proc_table + dest;
                printl("=_=%s", p->name);
                do {
                    assert(p->p_msg);
                    p = proc_table + p->p_sendto;
                    printl("->%s",p->name);
                } while (p != proc_table + src);
                printl("=_=");
                return 1;
            }
            p = proc_table + p->p_sendto;
        } else {
            break;
        }
    }
    return 0;
}

PRIVATE int msg_send(struct proc* current, int dest,MESSAGE* m) {

    struct proc* sender = current;
    struct proc* p_dest = proc_table + dest;
    
    assert(proc2pid(sender) != dest);

    // check for deadlock here
    if (deadlock(proc2pid(sender), dest)) {
        panic(">>DEADLOCK<< %s->%s", sender->name, p_dest->name);
    }

    if ((p_dest->p_flags & RECEIVING) && 
        (p_dest->p_recvfrom == proc2pid(sender) ||
         p_dest->p_recvfrom ==ANY)) {
        assert(p_dest->p_msg);
        assert(m);
        
        phys_copy(va2la(dest, p_dest->p_msg),
                  va2la(proc2pid(sender), m),
                  sizeof(MESSAGE));
        p_dest->p_msg = 0;
        p_dest->p_flags &= ~RECEIVING;
        p_dest->p_recvfrom = NO_TASK;
        unblock(p_dest);

        assert(p_dest->p_flags == 0);
        assert(p_dest->p_msg == 0);
        assert(p_dest->p_recvfrom == NO_TASK);
        assert(p_dest->p_sendto == NO_TASK);
        assert(sender->p_flags == 0);
        assert(sender->p_msg == 0);
        assert(sender->p_recvfrom == NO_TASK);
        assert(sender->p_sendto == NO_TASK);
    } else {
        sender->p_flags |= SENDING;
        assert(sender->p_flags == SENDING);
        sender->p_sendto = dest;
        sender->p_msg = m;
        
        if (p_dest->q_sending) {
            // append to the sending queue
            struct proc * p;
            p = p_dest->q_sending;
            while (p->next_sending) {
                p = p->next_sending;
            }
            p->next_sending = sender;
        } else {
            p_dest->q_sending = sender;
        }
        sender->next_sending = 0;

        block(sender);
        
        assert(sender->p_flags == SENDING);
        assert(sender->p_msg != 0);
        assert(sender->p_recvfrom == NO_TASK);
        assert(sender->p_sendto == dest);
    }
        
    return 0;
}


PRIVATE int msg_receive(struct proc* current, int src, MESSAGE* m) {

    struct proc* p_who_wanna_recv = current;
    struct proc* p_from = 0;
    struct proc* prev = 0;
    int copyok = 0;
    
    assert(proc2pid(p_who_wanna_recv) != src);
    
    if ((p_who_wanna_recv->has_int_msg) && ((src == ANY) || (src == INTERRUPT))) {
        MESSAGE msg;
        reset_msg(&msg);
        msg.source = INTERRUPT;
        msg.type = HARD_INT;
        assert(m);
        phys_copy(va2la(proc2pid(p_who_wanna_recv), m), &msg, sizeof(MESSAGE));
        
        p_who_wanna_recv->has_int_msg = 0;
        
        assert(p_who_wanna_recv->p_flags == 0);
        assert(p_who_wanna_recv->p_msg == 0);
        assert(p_who_wanna_recv->p_sendto == NO_TASK);
        assert(p_who_wanna_recv->has_int_msg == 0);
        
        return 0;
    }

 
    if (src == ANY) {
        if (p_who_wanna_recv->q_sending) {
            p_from = p_who_wanna_recv->q_sending;
            copyok = 1;
            assert(p_who_wanna_recv->p_flags == 0);
            assert(p_who_wanna_recv->p_msg == 0);
            assert(p_who_wanna_recv->p_recvfrom == NO_TASK);
            assert(p_who_wanna_recv->p_sendto == NO_TASK);
            assert(p_who_wanna_recv->q_sending != 0);
            assert(p_from->p_flags == SENDING);
            assert(p_from->p_msg != 0);
            assert(p_from->p_recvfrom == NO_TASK);
            assert(p_from->p_sendto == proc2pid(p_who_wanna_recv));
        }
    } else {
        p_from = &proc_table[src];

        if ((p_from->p_flags & SENDING) && 
            (p_from->p_sendto == proc2pid(p_who_wanna_recv))) {
            copyok = 1;
    
            struct proc* p = p_who_wanna_recv->q_sending;
            assert(p);

            while (p) {
                assert(p_from->p_flags & SENDING);
                if (proc2pid(p) == src) {
                    p_from = p;
                    break;
                }
                prev = p;
                p = p->next_sending;
            }

            assert(p_who_wanna_recv->p_flags == 0);
            assert(p_who_wanna_recv->p_msg == 0);
            assert(p_who_wanna_recv->p_recvfrom == NO_TASK);
            assert(p_who_wanna_recv->p_sendto== NO_TASK);
            assert(p_who_wanna_recv->q_sending != 0);
            assert(p_from->p_flags == SENDING);
            assert(p_from->p_msg != 0);
            assert(p_from->p_recvfrom == NO_TASK);
            assert(p_from->p_sendto == proc2pid(p_who_wanna_recv));
        }
    }

    if (copyok) {
        if (p_from == p_who_wanna_recv->q_sending) {
            assert(prev == 0);
            p_who_wanna_recv->q_sending = p_from->next_sending;
            p_from->next_sending = 0;
        } else {
            assert(prev);                                            
            prev->next_sending = p_from->next_sending;
            p_from->next_sending = 0;
        }

        assert(m);
        assert(p_from->p_msg);
        phys_copy(va2la(proc2pid(p_who_wanna_recv), m),
                  va2la(proc2pid(p_from), p_from->p_msg),
                  sizeof(MESSAGE));
        
        p_from->p_msg = 0;
        p_from->p_sendto = NO_TASK;
        p_from->p_flags &= ~SENDING;
        unblock(p_from);
    } else {
        p_who_wanna_recv->p_flags |= RECEIVING;
    
        p_who_wanna_recv->p_msg = m;
        if (src == ANY) {
            p_who_wanna_recv->p_recvfrom = ANY;
        } else {
            p_who_wanna_recv->p_recvfrom = proc2pid(p_from);
        }

        block(p_who_wanna_recv);

        assert(p_who_wanna_recv->p_flags == RECEIVING);
        assert(p_who_wanna_recv->p_msg != 0);
        assert(p_who_wanna_recv->p_recvfrom != NO_TASK);
        assert(p_who_wanna_recv->p_sendto == NO_TASK);
        assert(p_who_wanna_recv->has_int_msg == 0);
    }
        
    return 0;
}
