//
// Created by sulvto on 17-8-19.
//
#include "type.h"
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

/**
 * <Ring 1> The main loop of TASK FS
 */
PUBLIC void task_fs() {
    printl("Task FS begins.\n");
    init_fs();
    spin("FS");
}

/**
 * <Ring 1> Do some preparation.
 */
PRIVATE void init_fs() {
    // open the device:hard disk
    MESSAGE driver_msg;
    driver_msg.type = DEV_OPEN;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

    mkfs();
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
    sb.nr_smap_sects    = sb.nr_sects / bits_per_sect
    sb.n_lst_sect       = 1 + 1 + sb.nr_imap_sects + sb.nr_smap_sects + sb.nr_inodes;
    sb.root_sect        = ROOT_INODE;
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
            (geo.base + sb.n_lst_sect) * 2);
    
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
    for (int i = 0; i < nr_sects / 8; i++) {
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
    
    pi->i_start_sect = sb.n_lst_sect;
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

    ped->inode_nr = 1;
    strcpy(pde->name, ".");

    // dir entries of '/dev_tty0-2'
    for (int i=0; i < NR_CONSOLES; i++) {
        ped++;
        ped->inode_nr = i + 2;
        sprintf(ped->name, "dev_tty%d", i);
    }
    WR_SECT(ROOT_DEV, sb.n_lst_sect);
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
