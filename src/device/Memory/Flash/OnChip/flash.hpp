#ifndef __ON_CHIP_FLASH_HPP__

#define __ON_CHIP_FLASH_HPP__

#include "types/range/range_t.hpp"


class OnChipFlash{
protected:


    using Page = uint32_t;
    using PageRange = Range_t<Page>;
    using Address = uint32_t;
    using AddressRange = Range_t<Address>;

    static constexpr Page page_size = 256;
    static constexpr Address base_address = 0x08000000;

    Page page_count;
    PageRange page_range;

    uint32_t pre_clock;

    void settleClock(){
        pre_clock = Sys::Clock::getAHBFreq();
        Sys::Clock::setAHBFreq(72000000);
    }

    void resetClock(){
        Sys::Clock::setAHBFreq(pre_clock);
    }

    Page pageWarp(const int & index){
        return index > 0 ? index : page_count + index;
    }

    AddressRange getAddressRange(){
        return (page_range * page_size).shift(base_address);
    }
public:
    Flash(int _page_begin):Flash(_page_begin, Sys::Chip::getFlashSize() / page_size){;}
    Flash(int _page_begin, int _page_end):
            page_count(Sys::Chip::getFlashSize() / page_size),
            page_range(PageRange(Page(0),Sys::Chip::getFlashSize() / page_size)
            .intersection(PageRange(pageWarp(_page_begin), pageWarp(_page_end)))){;}

    ~Flash(){exit();}

    void init(){
        settleClock();
        Systick_Init();
        delay(10);
    }

    void test(){

        auto NbrOfPage = page_range.get_length(); //计算要擦除多少页
        Address PAGE_WRITE_START_ADDR = getAddressRange().start;
        Address PAGE_WRITE_END_ADDR = getAddressRange().end;
        Address address = PAGE_WRITE_START_ADDR;

        FLASH_Status FLASHStatus =  FLASH_COMPLETE;

        __disable_irq();
        // uint16_t Data = 0x1234;
        // FLASH_Unlock(); //解锁
        //FLASH_FLAG_BSY:指示忙状态标志；FLASH_FLAG_EOP：指示操作结束标志；FLASH_FLAG_PGERR：闪存程序错误标志；FLASH_FLAG_WRPRTERR：指示写保护错误标志

        // FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP|FLASH_FLAG_WRPRTERR); //清空所有标志位

        // logger.println("begin erase", PAGE_WRITE_START_ADDR);
        // for(Page EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++){
        //     FLASHStatus = FLASH_ErasePage(PAGE_WRITE_START_ADDR + (page_size * EraseCounter));
        // }

        // address = PAGE_WRITE_START_ADDR; //向内部FLASH写入数据起始地址
        // while((address < PAGE_WRITE_END_ADDR) && (FLASHStatus == FLASH_COMPLETE)){
        //     FLASHStatus = FLASH_ProgramHalfWord(address, Data); //向指定地址写入半字
        //     address += 2;
        // }

        // FLASH_Lock(); //上锁

        FLASH_Unlock_Fast();
        uint32_t Data = 0x12345678;
        FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP|FLASH_FLAG_WRPRTERR);

        for(Page EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++){
            FLASH_ErasePage_Fast(PAGE_WRITE_START_ADDR + (page_size * EraseCounter));
        }

        address = PAGE_WRITE_START_ADDR; //向内部FLASH写入数据起始地址
        while((address < PAGE_WRITE_END_ADDR) && (FLASHStatus == FLASH_COMPLETE)){
            FLASH_ProgramWord_Fast(address, Data); //向指定地址写入半字
            address += 4;
        }

        FLASH_Lock_Fast();
        __enable_irq();

        address = PAGE_WRITE_START_ADDR;
        bool MemoryProgramStatus = true;

        while((address < PAGE_WRITE_END_ADDR) && (MemoryProgramStatus != false)){
            auto read_data = (*(__IO uint32_t*) address);
            if(read_data != Data){
                MemoryProgramStatus = false;
            }
            address += 4;
        }

    };
    void exit(){
        resetClock();
    }


};


class OnChipFlashSector:public Sector<256>{

};


#endif