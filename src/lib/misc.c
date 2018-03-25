//
// Created by sulvto on 17-8-16.
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
#include "keyboard.h"
#include "proto.h"

/**
 * Compare memory areas.
 * @param s1 The 1st area.
 * @param s2 The 2nd area.
 * @param n The first n bytes will be compared.
 */
PUBLIC int memcmp(const void * s1, const void * s2, int n) {
    if ((s1 == 0) || (s2 == 0)) {
        return (s1 - s2);
    }
        const char * p1 = (const char *)s1;
    const char * p2 = (const char *)s2;

    for (int i = 0; i < n; i++, p1++, p2++) {
        if (*p1 != *p2){
            return (*p1 - *p2);
        }
    }
    return 0;
}


/**
 * Compare two strings.
 *
 * @param s1
 * @param s2
 *
 * @retrun 
 */
PUBLIC int strcmp(const char* s1, const char* s2) {
	if ((s1 == 0) || (s2 == 0)) {
		return (s1 - s2);
	}

	const char* p1 = s1;
	const char* p2 = s2;

	for (; *p1 && *p2; p1++,p2++) {
		if (*p1 != p2) {
			break;
		}
	}

	return (*p1 - *p2);
}

PUBLIC void spin(char * func_name) {
    printl("\nspinning in %s ...\n", func_name);
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
    // error MAG_CH_ASSERT -> '\003'
    printl("%c  assert(%s) failed: file: %s, base_file: %s, line: %d", MAG_CH_ASSERT, exp, file, base_file, line);

    spin("assertion_failure()");
    
    __asm__ __volatile__("ud2");
}
