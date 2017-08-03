//
// Created by sulvto on 17-7-29.
//

#include    "type.h"
#include    "const.h"
#include    "protect.h"
#include    "proto.h"
#include    "string.h"
#include    "proc.h"
#include    "global.h"

/**
 * 数字前面的 0 不被显示出来, 0000B800 --> B800
 * 
 */
PUBLIC char * itoa(char *str, int num) {
    char *p = str;
    int flag = 0;
    *p++ = '0';
    *p++ = 'x';
    if (num == 0) {
        *p++ = '0';
    } else {
        for (int i = 28; i >= 0; i -= 4) {
            char ch = (num >> i) & 0xF;
            if (flag || (ch > 0)) {
                flag = 1;
                ch += '0';
                if (ch > '9') {
                    ch += 7;
                }
                *p++ = ch;
            }
        }
    }
    *p = 0;
    return str;
}


/**
 * disp int
 */
PUBLIC void disp_int(int input) {
    char output[16];
    itoa(output, input);
    disp_str(output);
}


PUBLIC void delay(int time) {
    int i,j,k;
    for(k=0;k < time; k++) {
        for(j=0;j < 100; j++) {
            for(i=0;i < 10000; i++) { }
        }   
    }
}
