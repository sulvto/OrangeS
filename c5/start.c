/**
 * 
 * sulvto
 */

#include    "type.h"
#include    "const.h"
#include    "protect.h"

public  void* memcpy(void* pDst,void* pSrc,int iSize);

public u8   gdt_ptr[6]
public DESCRIPTOR   gdr[GDT_SIZE]

public void cstart(){
    disp_str("\n\n\n\n\n\n\n\n\n\n\"
            "--------\"catsrt\" begins------\n");
    memcpy(&gdt,(void*)(*((u32*)(&gdt_ptr[2]))),
            *((u16*)(&gdt_ptr[0]))*1);

    u16* p_gdt_limit = (u16*)(&gdt_ptr[0]);
    u32* p_gdt_base  = (u32*)(&gdt_ptr[2]);
    *p_gdt_limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1;
    *p_gdt_base  = (u32)&gdt;

}
