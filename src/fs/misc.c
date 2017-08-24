//
// Created by sulvto on 17-8-25.
//


/**
 * Get the basename from the fullpath.
 *
 * @param[out]  filename    The string for the result.
 * @param[in]   pathname    The full pathname.
 * @param[out]  ppinode     The ptr of the dir`s inode will be stored here.
 * @return Zero if success, otherwise the pathname is not valid.
 */ 
PUBLIC int strip_path(char * filename, const char * pathname, struct inode** ppinode) {
    const char * s = pathname;
    char * t = filename;
    
    if (s == 0) {
        return -1;
    }
    if (*s == '/') {
        s++;
    }
    while (*s) {
        if (*s == '/') {
            return -1;
        }
        *t++ = *s++;
        // if filename is too long, just truncate it
        if (t - filename >=MAX_FILENAME_LEN) {
            break;
        }
    }
    *t = 0;
    *ppinode = root_inode;
    return 0;
}


/**
 * Search the file and return the inode_nr.
 * @param[in] path The full path of the file to search.
 * @return Prt to the i-node of the file if successful, otherwise zero.
 */
PUBLIC int search_file(char * path) {
    // TODO
}
