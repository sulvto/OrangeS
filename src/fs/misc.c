//
// Created by sulvto on 17-8-25.
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
#include "hd.h"
#include "fs.h"

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
    int i,j;
    char filename[MAX_PATH];
    memset(filename, 0, MAX_FILENAME_LEN);
    struct inode * dir_inode;
    if (strip_path(filename, path, &dir_inode) != 0) {
        return 0;
    }
    // path: '/'
    if (filename[0] == 0) {
        return dir_inode->i_num;
    }

    /**
     * Search the dir for the file.
     */
    int dir_blk0_nr = dir_inode->i_start_sect;  
    int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    int nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE;
    
    int m = 0;
    struct dir_entry * pde;
    for (i = 0; i < nr_dir_blks; i++) {
        RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
        pde = (struct dir_entry *)fsbuf;
        for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++) {
            if (memcmp(filename, pde->name, MAX_FILENAME_LEN) == 0) {
                return pde->inode_nr;
            }
            if (++m > nr_dir_entries) { 
                break;
            }
        }
        if (m > nr_dir_entries) {
            break;
        }
    }
        
    // file not found
    return 0;
}

/**
 * <Ring 1>Get the inode ptr of given inode nr. A cache -- inode_table[] -- is 
 * maintained to make things faster. If the inode requested is already there,
 * just return it. Otherwise the inode will be read  from the disk.
 *
 *  @param dev Device nr.
 *  @param num I-node nr.
 *
 *  @return The inode ptr requested.
 */
PUBLIC struct inode * get_inode(int dev, int num) {
    if (num == 0) return 0;
    
    struct inode * p;
    struct inode * q = 0;
    for (p = &inode_table[0]; p < &inode_table[NR_INODE]; p++) {
        if (p->i_cnt) {         // not a free slot
            if ((p->i_dev == dev) && (p->i_num == num)) {
                // this is the inode we want
                p->i_cnt++;
                return p;
            }
        } else {                // a free slot
            if (!q) {
                q = p;          // q <- the lst free slot;
            }
        }
    }
        
    if (!q) {
        panic("the inode table is full");
    }

    q->i_dev = dev;
    q->i_num = num;
    q->i_cnt = 1;

    struct super_block * sb = get_super_block(dev);
    int blk_nr = 1 + 1 + sb->nr_imap_sects + sb->nr_smap_sects + ((num - 1) / (SECTOR_SIZE / INODE_SIZE));
    RD_SECT(dev, blk_nr);
    struct inode * pinode = (struct inode*)((u8*)fsbuf + ((num - 1) % (SECTOR_SIZE / INODE_SIZE)) * INODE_SIZE);
    
    q->i_mode = pinode->i_mode;
    q->i_size = pinode->i_size;
    q->i_start_sect = pinode->i_start_sect;
    q->i_nr_sects = pinode->i_nr_sects;
    return q;
}

PUBLIC void put_inode(struct inode * pinode) {
    assert(pinode->i_cnt > 0);
    pinode->i_cnt--;
}

/**
 * <Ring 1> Write the inode back to the disk.Commonly invoked as soon as the inode is changed.
 *
 * @param p I-node ptr.
 */
PUBLIC void sync_inode(struct inode * p) {
    struct super_block * sb = get_super_block(p->i_dev);
    int blk_nr = 1 + 1 + sb->nr_imap_sects + sb->nr_smap_sects + ((p->i_num -1) / (SECTOR_SIZE / INODE_SIZE));
    RD_SECT(p->i_dev, blk_nr);
    struct inode * pinode = (struct inode*)((u8*)fsbuf + ((p->i_num - 1) % (SECTOR_SIZE / INODE_SIZE)) * INODE_SIZE);
    
    pinode->i_mode = p->i_mode;
    pinode->i_size = p->i_size;
    pinode->i_start_sect = p->i_start_sect;
    pinode->i_nr_sects = p->i_nr_sects;
    
    WR_SECT(p->i_dev, blk_nr);
}


