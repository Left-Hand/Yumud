#include "flash.h"

void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite){
 
    u32 sect_pos;
    u16 secoff;
    u16 sect_remain;
    u16 i;
    u32 offset_addr;
 
    if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))
        return;
 
    FLASH_Unlock();
    offset_addr=WriteAddr-STM32_FLASH_BASE;
    sect_pos=offset_addr/STM_SECTOR_SIZE;
    secoff=(offset_addr%STM_SECTOR_SIZE)/2;
    sect_remain=STM_SECTOR_SIZE/2-secoff;
    if(NumToWrite<=sect_remain)sect_remain=NumToWrite;
 
    while(1){
 
        STMFLASH_Read(sect_pos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);
        for(i=0;i<sect_remain;i++){
            if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;
        }
 
        if(i<sect_remain){
            FLASH_ErasePage(sect_pos*STM_SECTOR_SIZE+STM32_FLASH_BASE);
 
            for(i=0;i<sect_remain;i++){
                STMFLASH_BUF[i+secoff]=pBuffer[i];
            }
 
            STMFLASH_Write_NoCheck(sect_pos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);
 
        }else STMFLASH_Write_NoCheck(WriteAddr,pBuffer,sect_remain);
 
        if(NumToWrite==sect_remain)
            break;
        else{
            sect_pos++;
            secoff=0;
            pBuffer+=sect_remain;
            WriteAddr+=(sect_remain*2);
            NumToWrite-=sect_remain;
 
            if(NumToWrite>(STM_SECTOR_SIZE/2))
                sect_remain=STM_SECTOR_SIZE/2;
 
            else sect_remain=NumToWrite;
        }
    };
    FLASH_Lock();
}