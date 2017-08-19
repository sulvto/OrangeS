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

PRIVATE void init_hd();
PRIVATE void hd_cmd_out(struct hd_cmd* cmd); 
PRIVATE int waitfor(int mask, int val, int timeout);
PRIVATE void interrupt_wait(); 
PRIVATE void hd_identify(int drive);
PRIVATE void print_identify_info(u16* hdinfo); 

PRIVATE u8 hd_status;
PRIVATE u8 hdbuf[SECTOR_SIZE * 2];


/**
 * Main loop of HD driver.
 */ 
PUBLIC void task_hd() {
    MESSAGE msg;
    init_hd();

    while (1) {
        send_recv(RECEIVE, ANY, &msg);

        int src = msg.source;

        switch (msg.type) {
            case DEV_OPEN:
                hd_identify(0);
                break;

            default:
                // dump_msg("HD driver::unknown msg", &msg);
                spin("FS::main_loop (invalid meg.type)");
                break;
        }

        send_recv(SEND, src, &msg);
    }
}

/**
 * <Ring 1> Check hard drive,set IRQ handler,enable IRQ and initialize data structures.
 *
 */
PRIVATE void init_hd() {
    // Get the number of drivers grom thr BIOS data area.
    u8 * pNrDrivers = (u8*)(0x475);
    printl("NrDrivers:%d.\n", *pNrDrivers);
    assert(*pNrDrivers);
    put_irq_handler(AT_WINI_IRQ, hd_handler);
    enable_irq(CASCADE_IRQ);
    enable_irq(AT_WINI_IRQ);
}

/**
 *  <Ring 1> Get the disk information
 * @param drive Drive Nr
 */
PRIVATE void hd_identify(int drive) {
    struct hd_cmd cmd;
    cmd.device - MAKE_DEVICE_REG(0, drive, 0);
    cmd.command = ATA_IDENTIFY;
    hd_cmd_out(&cmd);
    interrupt_wait();
    port_read(REG_DATA, hdbuf, SECTOR_SIZE);
    
    print_identify_info((u16*)hdbuf);
}

PRIVATE void print_identify_info(u16* hdinfo) {
    char s[64];
    
    struct iden_info_ascii {
        int idx;
        int len;
        char * desc;
    } iinfo[] = {{10, 20, "HD SH"},       // Serial number in ASCII
                 {27, 40, "HD Model"}};   // Model number in ASCII
             
    int i;   
    for (int k = 0; k < sizeof(iinfo)/sizeof(iinfo[0]); k++) {
        char * p = (char*)&hdinfo[iinfo[k].idx];
        for (i = 0; i < iinfo[k].len/2; i++) {
            s[i*2+1] = *p++;
            s[i*2] = *p++;
        }
        s[i*2] = 0;
        printl("%s: %s\n", iinfo[k].desc, s);
    }

    int capabilities = hdinfo[49];
    printl("LBA supported: %s\n", (capabilities & 0x0200) ? "Yes" : "No");

    int cmd_set_supported = hdinfo[83];
    printl("LBA48 supported: %s\n", (cmd_set_supported & 0x0400) ? "Yes" : "No");

    int sectors = ((int)hdinfo[61] << 16) + hdinfo[60];                                             
    printl("HD size: %dMB\n", sectors * 512 / 1000000);
}

PRIVATE void hd_cmd_out(struct hd_cmd* cmd) {
    if (!waitfor(STATUS_BSY, 0, HD_TIMEOUT)) panic("hd error.");
    
    // Activate the Interrupt Enable (nIEN) bit
    out_byte(REG_DEV_CTRL, 0);
    // Load required parameters in the Command Block Registers
    out_byte(REG_FEATIRES, cmd->features);
    out_byte(REG_NSECTOR, cmd->count);
    out_byte(REG_LBA_LOW, cmd->lba_low);
    out_byte(REG_LBA_MID, cmd->lba_mid);
    out_byte(REG_LBA_HIGH, cmd->lba_high);
    out_byte(REG_DEVICE, cmd->device);
    // Write the command code to the Command Register
    out_byte(REG_CMD, cmd->command);
}

/**
 * <Ring 1> Wait until a disk interrupt occurs
 */
PRIVATE void interrupt_wait() {
    MESSAGE msg;
    send_recv(RECEIVE, INTERRUPT, &msg);
}


/**
 * <Ring 1> Wait for a certain status
 */
PRIVATE int waitfor(int mask, int val, int timeout) {
    int t = get_ticks();
    while (((get_ticks() - t) * 1000 / HZ) < timeout) {
        if ((in_byte(REG_STATUS) & mask) == val) {
            return 1;
        }
    }

    return 0;
}

/**
 * <Ring 0> Interrupt handler
 *
 * @param irq IRQ nr of the disk interrupt
 */
PUBLIC void hd_handler(int irq) {
    hd_status = in_byte(REG_STATUS);
//    inform_int(TASK_HD);
}
