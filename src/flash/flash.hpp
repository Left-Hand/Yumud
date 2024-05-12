#ifndef __FLASH_HPP__

#define __FLASH_HPP__

#include "src/platform.h"

class Flash{
protected:


    using Page = uint32_t;
    using PageRange = Range_t<Page>;
    using Address = uint32_t;
    using AddressWindow = Range_t<Address>;

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

    AddressWindow getAddressRange(){
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

    template<typename T>
    volatile bool store(const T & data){

        auto NbrOfPage = page_range.get_length(); //计算要擦除多少页
        Address PAGE_WRITE_START_ADDR = getAddressRange().start;
        Address PAGE_WRITE_END_ADDR = getAddressRange().end;

        logger.println(PAGE_WRITE_START_ADDR);
        FLASH_Status FLASHStatus =  FLASH_COMPLETE;

        __disable_irq();
        FLASH_Unlock_Fast();

        FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP|FLASH_FLAG_WRPRTERR);

        // logger.println("begin erase", PAGE_WRITE_START_ADDR);

        // uint32_t len = sizeof(data);
        // for(Page EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++){
            // Address op_address = PAGE_WRITE_START_ADDR + (page_size * EraseCounter)
        uint32_t buf[page_size / sizeof(uint32_t)] = {0};
        // memcpy(&buf, &data, sizeof(T));
        for(size_t i = 0; i < sizeof(T); i++){
            ((uint8_t *)buf)[i] = ((uint8_t *)&data)[i];
            // logger.println(i,((uint8_t *)&data)[i] );
        }

        FLASH_ErasePage_Fast(PAGE_WRITE_START_ADDR);
            // if()
            // FLASH_ProgramPage_Fast(PAGE_WRITE_START_ADDR, (uint32_t *)((uint8_t *)&data + page_size * EraseCounter));
        FLASH_ProgramPage_Fast(PAGE_WRITE_START_ADDR, (uint32_t *)&buf);
        // }

        FLASH_Lock_Fast();
        __enable_irq();

        Address address = PAGE_WRITE_START_ADDR;
        bool MemoryProgramStatus = true;

        uint32_t i = 0;
        while((address < PAGE_WRITE_END_ADDR) && (MemoryProgramStatus != false)){
            auto read_data = (*(__IO uint32_t*) address);
            auto examine_data = buf[i];
            if(read_data != examine_data){
                MemoryProgramStatus = false;
            }
            // logger.println("vait", address, read_data, examine_data);
            address += 4;
            i++;
        }
        // logger.println("vait", MemoryProgramStatus);
        return MemoryProgramStatus;
    };

    template<typename T>
    volatile void load(T & data){
        Address PAGE_WRITE_START_ADDR = getAddressRange().start;
        Address PAGE_WRITE_END_ADDR = getAddressRange().end;
        Address address = PAGE_WRITE_START_ADDR;
        // uint32_t i = 0;
        for(size_t i = 0; i<sizeof(T);i++){
            auto read_data = (*(__IO uint8_t*)(PAGE_WRITE_START_ADDR + i));
            *((volatile uint8_t*)&data + i) = read_data;
            // logger.println("read", address, read_data);
            // address += 1;
            // i++;
        }
    };

    void exit(){
        resetClock();
    }
};

// void testFlash(){


//     Flash flash(-1);
//     flash.init();
//     Sys::Misc::prework();

//     uart2.init(115200);

//     logger.setSpace(" ");
//     logger.setRadix(16);

//     // logger.println("Flash Size:", Sys::Chip::getFlashSize());
//     // logger.println("Sys Clock:", Sys::Clock::getSystemFreq());
//     // logger.println("AHB Clock:", Sys::Clock::getAHBFreq());
//     // logger.println("APB1 Clock:", Sys::Clock::getAPB1Freq());
//     // logger.println("APB2 Clock:", Sys::Clock::getAPB2Freq());
//     struct Temp{
//         uint8_t data[4] = {0,1,2,3};
//         char name[8] = "Rstr1aN";
//         real_t value = real_t(0.1);
//         uint8_t crc = 0x08;
//     };
//     Temp temp;

//     logger.println("单纯的妇女而会计法v你rfwevdnoln");
//     // bkp.init();
//     // logger.println("bkp_data", bkp.readData(1));

//     // if(bkp.readData(1) % 2){
//     //     // flash.load(temp);
//     //     logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
//     //     logger.println(temp.name);
//     //     logger.println(temp.value);
//     //     logger.println(temp.crc);

//     // }
//     flash.load(temp);
//     if(temp.data[0] == 0x39){
//         logger.println("need to store new");
//         logger.println("new data is");
//         Temp new_temp = Temp();
//         logger.println(new_temp.data[0], new_temp.data[1], new_temp.data[2], new_temp.data[3]);
//         flash.store(new_temp);
//         flash.load(temp);
//         logger.println("new store done");
//     }

//     logger.println("data loaded");
//     logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
//     logger.println(temp.name);
//     logger.println(temp.value);
//     logger.println(temp.crc);
//     bkp.init();
//     bkp.writeData(1, bkp.readData(1) + 1);
//     temp.crc = bkp.readData(1);
//     flash.store(temp);
//     // flash.load(temp);
//     // logger.println();
//     // if(temp.data[0] == 0xE3 || temp.data[0] == 0x39 || (bkp.readData(1) & 0b11) == 0){
//     //     logger.println(temp.data[0]);

//     //     temp.data[3] = bkp.readData(1);
//     //     temp.name[2] = 'k';
//     //     // temp.value = real_t(0.2);
//     //     temp.crc = 10;

//     //     logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
//     //     logger.println(temp.name);
//     //     logger.println(temp.value);
//     //     logger.println(temp.crc);
//     //     flash.store(temp);
//     //     flash.load(temp);
//     // }else{
//     //     logger.println("suss");
//     //         logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
//     // logger.println(temp.name);
//     // logger.println(temp.value);
//     // logger.println(temp.crc);
//     // }


//     // }
//     // logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
//     // logger.println(temp.name);
//     // logger.println(temp.value);
//     // logger.println(temp.crc);

//     // 

#endif