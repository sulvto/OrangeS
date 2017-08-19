//
// Created by sulvto on 17-8-16.
//

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PUBLIC void spin(char * func_name) {
    printl("\nspining in %s ...\n", func_name);
    while(1){}
}

/**
 * Invoked by assert()
 * @param exp       The failure expression itself
 * @param file      __FILE__
 * @param base_file __BASE_FILE__
 * @param line      __LINE__
 */
PUBLIC void assertion_failure(char *exp, char *file, char *base_file, int line) {
    printl("%c  assert(%s) failed: file:%s,base_file: %s, in %d",
        MAG_CH_ASSERT, exp, file, base_file, line);

    spin("assertion_failure()");
    
    __asm__ __volatile__("ud2");
}
