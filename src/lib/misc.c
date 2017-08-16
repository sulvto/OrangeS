//
// Created by sulvto on 17-8-16.
//

PUBLIC void assertion_failure(char *exp, char *file, char *base_file, int line) {
    printl("%c  assert(%s) failed: file:%s,base_file: %s, in %d",
        MAG_CH_ASSERT, exp, file, base_file, line);

    spin("assertion_failure()");
    
    _asm_ _volatile_('ud2');
}
