//
// Created by sulvto on 17-8-19.
//
#include "type.h"
#include "config.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

#include "hd.h"


PRIVATE void init_fs();
PRIVATE void mkfs();
PRIVATE void read_super_block(int dev);

/**
 * <Ring 1> The main loop of TASK FS
 */
PUBLIC void task_fs() {
    printl("Task FS begins.\n");
    init_fs();

    while (1) {
        send_recv(RECEIVE, ANY, &fs_msg);
        
        int src = fs_msg.source;
        pcaller = &proc_table[src];
        
        switch (fs_msg.type) {
            case OPEN:
                fs_msg.FD = do_open();
                break;
            case CLOSE:
                fs_msg.RETVAL = do_close();
                break;
            case READ:
            case WRITE:
                fs_msg.CNT = do_rdwt();
                break;
        }

        fs_msg.type = SYSCALL_RET;
        send_recv(SEND, src, &fs_msg);
    }
}

/**
 * <Ring 1> Do some preparation.
 */
PRIVATE void init_fs() {
    // f_desc_table[]
    for (int i = 0; i < NR_FILE_DESC; i++) {
        memset(&f_desc_table[i], 0, sizeof(struct file_desc));
    }

    // inode_table[]
    for (int i = 0; i < NR_INODE; i++) {
        memset(&inode_table[i], 0, sizeof(struct inode));
    }

    // super_block[]
    struct super_block * sb = super_block;
    for (; sb < &super_block[NR_SUPER_BLOCK]; sb++) {
        sb->sb_dev = NO_DEV;
    }

    // open the device:hard disk
    MESSAGE driver_msg;
    driver_msg.type = DEV_OPEN;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);
    
    // make FS
    mkfs();
    
    // load super block of ROOT
    read_super_block(ROOT_DEV);

    sb = get_super_block(ROOT_DEV);
    assert(sb->magic == MAGIC_V1);
    
    root_inode = get_inode(ROOT_DEV, ROOT_INODE);
}

PRIVATE void mkfs() {

    // open the device:hard disk
    MESSAGE driver_msg;
    int bits_per_sect = SECTOR_SIZE * 8;    // 8 bits per byte
    struct part_info geo;
    driver_msg.type     = DEV_OPEN;
    driver_msg.DEVICE   = MINOR(ROOT_DEV);
    driver_msg.REQUEST  = DIOCTL_GET_GEO;
    driver_msg.BUF      = &geo;
    driver_msg.PROC_NR  = TASK_FS;
    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

    printl("dev size: 0x%x sectors\n", geo.size);

    // super block
    struct super_block sb;
    sb.magic            =  MAGIC_V1;
    sb.nr_inodes        = bits_per_sect;
    sb.nr_inode_sects   = sb.nr_inodes;
    sb.nr_sects         = geo.size;
    sb.nr_imap_sects    = 1;
    sb.nr_smap_sects    = sb.nr_sects / bits_per_sect;
    sb.n_1st_sect       = 1 + 1 + sb.nr_imap_sects + sb.nr_smap_sects + sb.nr_inode_sects;
    sb.root_inode       = ROOT_INODE;
    sb.inode_size       = INODE_SIZE;
    struct inode x;
    sb.inode_isize_off  = (int)&x.i_size - (int)&x;
    sb.inode_start_off  = (int)&x.i_start_sect - (int)&x;
    sb.dir_ent_size     = DIR_ENTRY_SIZE;
    struct dir_entry de;
    sb.dir_ent_inode_off = (int)&de.inode_nr - (int)&de;
    sb.dir_ent_fname_off = (int)&de.name - (int)&de;

    memset(fsbuf, 0x90, SECTOR_SIZE);
    memcpy(fsbuf, &sb, SUPER_BLOCK_SIZE);

    // writer the super block
    WR_SECT(ROOT_DEV, 1);

    printl("devbase: 0x%x00, sb:0x%x00, imap:0x%x00, smap:0x%x00\n"
           "        inodes:0x%x00, lst_sector:0x%x00\n",
            geo.base * 2,
            (geo.base + 1) * 2,
            (geo.base + 1 + 1) * 2,
            (geo.base + 1 + 1 + sb.nr_imap_sects)  * 2,
            (geo.base + 1 + 1 + sb.nr_imap_sects + sb.nr_smap_sects)  * 2,
            (geo.base + sb.n_1st_sect) * 2);
    
    // inode map
    memset(fsbuf, 0, SECTOR_SIZE);
    for (int i=0; i < (NR_CONSOLES + 2); i++) {
        fsbuf[0] |= 1 << i;
    }

    assert(fsbuf[0] == 0x1F);   // 0001 1111

    WR_SECT(ROOT_DEV, 2);

    // sector map
    memset(fsbuf, 0, SECTOR_SIZE);
    int nr_sects = NR_DEFAULT_FILE_SECTS + 1;

    int i;
    for (i = 0; i < nr_sects / 8; i++) {
        fsbuf[i] = 0xFF;
    }
    for (int j = 0; j < nr_sects % 8; j++) {
        fsbuf[i] |= (1 << j);
    }

    WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects);

    memset(fsbuf, 0, SECTOR_SIZE);
    for (int i = 1; i < sb.nr_smap_sects; i++) {
        WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + i);
    }

    // inodes
    memset(fsbuf, 0, SECTOR_SIZE);
    struct inode * pi = (struct inodes*)fsbuf;
    pi->i_mode = I_DIRECTORY;
    pi->i_size = DIR_ENTRY_SIZE * 4;
    
    pi->i_start_sect = sb.n_1st_sect;
    pi->i_nr_sects = NR_DEFAULT_FILE_SECTS;
    // inode of  '/dev_tty0-2/'

    for (i = 0; i < NR_CONSOLES; i++) {
        pi = (struct inode*)(fsbuf + (INODE_SIZE * (i + 1)));
        pi->i_mode = I_CHAR_SPECIAL;
        pi->i_size = 0;
        pi->i_start_sect = MAKE_DEV(DEV_CHAR_TTY, i);
        pi->i_nr_sects = 0;
    }
    WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + sb.nr_smap_sects);

    // '/' root
    memset(fsbuf, 0, SECTOR_SIZE);
    struct dir_entry * pde = (struct dir_entry *)fsbuf;

    pde->inode_nr = 1;
    strcpy(pde->name, ".");

    // dir entries of '/dev_tty0-2'
    for (int i=0; i < NR_CONSOLES; i++) {
        pde++;
        pde->inode_nr = i + 2;
        sprintf(pde->name, "dev_tty%d", i);
    }
    WR_SECT(ROOT_DEV, sb.n_1st_sect);
}

PUBLIC int rw_sector(int io_type, int dev, u64 pos, int bytes, int proc_nr, void* buf) {
    MESSAGE driver_msg;
    
    driver_msg.type = io_type;
    driver_msg.DEVICE = MINOR(dev);
    driver_msg.POSITION = pos;
    driver_msg.BUF = buf;
    driver_msg.CNT = bytes;
    driver_msg.PROC_NR = proc_nr;
    assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &driver_msg);
    
    return 0;
}


/**
 * <Ring 1> Read super block from the given device then writer it info 
 *  a free super_block[] solt.
 *
 * @param dev From which device the super block comes.
 */
PRIVATE void read_super_block(int dev) {
    int i;
    MESSAGE driver_msg;
    
    driver_msg.type = DEV_READ;
    driver_msg.DEVICE = MINOR(dev);
    driver_msg.POSITION = SECTOR_SIZE * 1;
    driver_msg.BUF = fsbuf;
    driver_msg.CNT = SECTOR_SIZE;
    driver_msg.PROC_NR = TASK_FS;
    assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &driver_msg);

    // find a free slot in super_block[]
    for (i = 0; i < NR_SUPER_BLOCK; i++) {
        if (super_block[i].sb_dev == NO_DEV) {
            break;
        }
    }
    if (i == NR_SUPER_BLOCK) {
        panic("super_block slots used up");
    }
    // currently we use only the lst slot.
    assert(i == 0);
    
    struct super_block * psb = (struct super_block *)fsbuf;
    super_block[i] = *psb;
    super_block[i].sb_dev = dev;
}

/**
 * <Ring 1> Get the super block from super_block[]
 * @param dev Device nr.
 */
PUBLIC struct super_block * get_super_block(int dev) {
    struct super_block * sb = super_block;
    for (; sb < &super_block[NR_SUPER_BLOCK]; sb++) {
        if (sb->sb_dev == dev) {
            return sb;
        }
    }
    panic("super block of devie %d not found.\n",dev);
    return 0;
}


