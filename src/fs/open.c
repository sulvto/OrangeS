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
#include "keyboard.h"
#include "proto.h"

PRIVATE struct inode* create_file(char * path, int flags);
PRIVATE int alloc_imap_bit(int dev);
PRIVATE int alloc_smap_bit(int dev, int nr_sects_to_alloc);
PRIVATE struct inode * new_inode(int dev, int inode_nr, int start_sect);
PRIVATE void new_dir_entry(struct inode *dir_inode, int inode_nr, char *filename);


/**
 * Open a file and return the file descriptor.
 *
 * @return File descriptor if successful, otherwise a negative error code.
 */
PUBLIC int do_open() {
    int fd = -1;                    // return value
    char pathname[MAX_PATH];
    
    // get parameters from the message
    int flags = fs_msg.FLAGS;       // access mode
    int name_len = fs_msg.NAME_LEN; // length of filename
    int src = fs_msg.source;        // caller proc nr.
    assert(name_len < MAX_PATH);
    phys_copy((void*)va2la(TASK_FS, pathname), (void*)va2la(src, fs_msg.PATHNAME), name_len);
    pathname[name_len] = 0;

    // find a free slot in PROCESS::filp[]
    int i;
    for (i = 0; i < NR_FILES; i++) {
        if (pcaller->filp[i] == 0) {
            fd = i;
            break;
        }
    }
    if ((fd < 0) || (fd >= NR_FILES)) {
        panic("filp[] is full (PID:%d)", proc2pid(pcaller));
    }
    
    // find a free solt in f_desc_table[]
    for (i = 0; i < NR_FILE_DESC; i++) {
        if (f_desc_table[i].fd_inode == 0) {
            break;
        }
    }
    if (i >= NR_FILE_DESC) {
        panic("f_desc_table[] is full (PID:%d)", proc2pid(pcaller));
    }
    int inode_nr = search_file(pathname);
    struct inode * pin = 0;
    if (flags & O_CREAT) {
        if (inode_nr) {
            printl("file exists.\n");
            return -1;
        } else {
            pin = create_file(pathname, flags);
        }
    } else {    
        assert(flags & O_RDWR);
        
        char filename[MAX_PATH];
        struct inode * dir_inode;
        if (strip_path(filename, pathname, &dir_inode) != 0) {
            return -1;
        }
        pin = get_inode(dir_inode->i_dev, inode_nr);
    }
    
    if (pin) {
        // connects proc with file_descriptor
        pcaller->filp[fd] = &f_desc_table[i];

        // connects file_descriptor with inode
        f_desc_table[i].fd_inode = pin;

        f_desc_table[i].fd_mode = flags;
        // f_desc_table[i].fd_cnt = 1;
        f_desc_table[i].fd_pos = 0;
        
        int imode = pin->i_mode & I_TYPE_MASK;

        if (imode == I_CHAR_SPECIAL) {
            MESSAGE driver_msg;
            
            driver_msg.type = DEV_OPEN;
            int dev = pin->i_start_sect;
            driver_msg.DEVICE = MINOR(dev);
            assert(MAJOR(dev) == 4);
            assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
            send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &driver_msg);
        } else if (imode == I_DIRECTORY) {
            assert(pin->i_num == ROOT_INODE);
        } else {
            assert(pin->i_mode == I_REGULAR);
        }
    } else {
        return -1;
    }

    return fd;
}

/**
 * Handle the message CLOSE.
 *
 *  @return Zero if success.
 */
PUBLIC int do_close() {
    int fd = fs_msg.FD;
    put_inode(pcaller->filp[fd]->fd_inode);
    pcaller->filp[fd]->fd_inode = 0;
    pcaller->filp[fd] = 0;

    return 0;
}


/**
 * Create a file and return it`s inode prt.
 *
 * @param[in] path The full path of the new file.
 * @param[in] flags Attribiutes of the new file
 *
 * @return Ptr to i-node of the new file if successful, otherwise 0.
 *
 * @see open()
 *
 * @see do_open()
 *
 */
PRIVATE struct inode* create_file(char * path, int flags) {
    char filename[MAX_PATH];
    struct inode * dir_inode;
    if (strip_path(filename, path, &dir_inode) !=0) {
        return 0;
    } 
    int inode_nr = alloc_imap_bit(dir_inode->i_dev);
    int free_sect_nr = alloc_smap_bit(dir_inode->i_dev, NR_DEFAULT_FILE_SECTS);

    struct inode *newinode = new_inode(dir_inode->i_dev, inode_nr, free_sect_nr);
    new_dir_entry(dir_inode, newinode->i_num, filename);
        
    return newinode;
}

/**
 * Allocate a bit in inode-map
 *
 * @param dev In which device the inode-map is located.
 * 
 * @return I-node nr.
 */
PRIVATE int alloc_imap_bit(int dev) {
    int inode_nr = 0;
    int i,j,k;
    
    int imap_blko_nr = 2; // 1 boot sector & 1 super block
    struct super_block * sb = get_super_block(dev);

    for (i = 0; i < sb->nr_imap_sects; i++) {
        RD_SECT(dev, imap_blko_nr + i);
        
        for (j = 0; j < SECTOR_SIZE; j++) {
            // skip '11111111' bytes
            if (fsbuf[j] == 0xFF) {
                continue;
            }
            // skip '1' bits
            for (k = 0; ((fsbuf[j] >> k) & 1) != 0; k++) {}
            
            // i: sector index; j:byte index; k:bit index
            inode_nr = (1 * SECTOR_SIZE + j) * 8 + k;
            fsbuf[j] |= (1 << k);
            
            // write the bit to imap
            WR_SECT(dev, imap_blko_nr + i);
            break;
        }
        
        return inode_nr;
    }

    // no free bit in imap
    panic("inode-map is probably full.\n");
        
    return 0;
}

/**
 * Allocate a bit in sector-map
 * @param dev In which device the sector-map is located.
 * @param nr_sects_to_alloc How many sectors are allocated.
 * @return The 1st sector nr allocated
 */
PRIVATE int alloc_smap_bit(int dev, int nr_sects_to_alloc) {
    // int nr_sects_to_alloc = NR_DEFAULT_FILE_SECTS;
    int i; // sector index
    int j; // byte index
    int k; // bit index
    
    struct super_block *sb = get_super_block(dev);

    int smap_blko_nr = 2 + sb->nr_imap_sects;
    int free_sect_nr = 0;
    
    for ( i=0; i < sb->nr_smap_sects; i++) {
        RD_SECT(dev, smap_blko_nr + i);

        // bite offset in current sect
        for (j = 0; j < SECTOR_SIZE && nr_sects_to_alloc > 0; j++) {
            k = 0;
            if (!free_sect_nr) {
                // loop until a free bit is found
                if (fsbuf[j] == 0xFF) continue;
                for (; ((fsbuf[j] >> k) & 1) != 0; k++) {}
                free_sect_nr = (i * SECTOR_SIZE + j) * 8 + k -1 + sb->n_lst_sect;
            }
            for (; k < 8; k++) {
                assert(((fsbuf[j] >> k) & 1) == 0);
                fsbuf[j] |= (1 << k);
                if (--nr_sects_to_alloc == 0) break;
            }
        }
        
        // free bit found, write the bits to smap
        if (free_sect_nr) {
            WR_SECT(dev, smap_blko_nr + i);
        }

        if (nr_sects_to_alloc == 0) break;
    }

    assert(nr_sects_to_alloc == 0);
    
    return free_sect_nr;
}


/**
 * Generate a new i-node and write it to disk.
 *
 * @param dev Home device of the i-node.
 * @param inode_nr I-node nr.
 * @param start_sect Start sector of the file pointed by the new i-node.
 *
 * @return Prt of the new i-node.
 */
PRIVATE struct inode * new_inode(int dev, int inode_nr, int start_sect) {
    struct inode * new_inode = get_inode(dev, inode_nr);
    
    new_inode->i_mode = I_REGULAR;
    new_inode->i_size = 0;
    new_inode->i_start_sect = start_sect;
    new_inode->i_nr_sects = NR_DEFAULT_FILE_SECTS;

    new_inode->i_dev = dev;
    new_inode->i_cnt = 1;
    new_inode->i_num = inode_nr;

    // write to the inode array
    sync_inode(new_inode);

    return new_inode;
}

/**
 * Write a new entry into the directory.
 *
 * @param dir_inode I-node of the directory.
 * @param inode_nr I-node nr of the new file.
 * @param filename Filename of the new file.
 */
PRIVATE void new_dir_entry(struct inode *dir_inode, int inode_nr, char *filename) {
    // write the dir_entry
    int dir_blk0_nr = dir_inode->i_start_sect;
    int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE) / SECTOR_SIZE;
    int nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE; 
    int m = 0;
    struct dir_entry * pde;
    struct dir_entry * new_de = 0;
    
    int i,j;
    for (i = 0; i < nr_dir_blks; i++) {
        RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
        
        pde = (struct dir_entry *)fsbuf;
        for (j = 00; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++) {
            if (++m > nr_dir_entries) {
                break;
            }
            if (pde->inode_nr == 0) {
                new_de = pde;
                break;
            }
        }

        // all entries hava been iterated or free slot id found
        if (m > nr_dir_entries || new_de) {
            break;
        }
    }
    if (!new_de) {
        new_de = pde;
        dir_inode->i_size += DIR_ENTRY_SIZE;
    }
    new_de->inode_nr = inode_nr;
    strcpy(new_de->name, filename);
    
    // write dir block -- ROOT dir block
    WR_SECT(dir_inode->i_dev, dir_blk0_nr + i);

    // update dir inode
    sync_inode(dir_inode);
}



