#ifndef __FLASH_HPP__

#define __FLASH_HPP__

#include "src/platform.h"
#include "drivers/Memory/storage.hpp"
#include "drivers/Memory/memory.hpp"

#ifdef FLASH_DEBUG
#undef FLASH_DEBUG
#define FLASH_DEBUG(...) DEBUG_LOG(SpecToken::Space, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define FLASH_DEBUG(...)
#endif

class Flash:public Storage{
protected:

    using Page = uint32_t;
    using PageRange = Range_t<Page>;
    using Address = uint32_t;
    using AddressWindow = Range_t<Address>;

    static constexpr Page page_size = 256;
    static constexpr Address base_address = 0x08000000;

    Page page_count;
    // PageRange page_range;

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

    void entry_store() override{
        settleClock();
        Systick_Init();
        delay(10);
    }

    void exit_store() override{

    }

    void entry_load() override{

    }

    void exit_load() override{
        resetClock();
    }


    void lock(){
        FLASH_Lock_Fast();
        __enable_irq();

    }

    void unlock(){
        __disable_irq();
        FLASH_Unlock_Fast();

        FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP|FLASH_FLAG_WRPRTERR);
    }

    void _store(const void * data, const Address data_size, const Address loc) override{

        unlock();
        uint32_t buf[page_size / sizeof(uint32_t)] = {0};

        AddressWindow store_window = AddressWindow{loc,loc + data_size};
        AddressWindow op_window = {0,0};
        do{
            op_window = store_window.grid_forward(op_window, page_size);
            if(op_window){
                auto phy_window = decltype(op_window)::grid(op_window.shift(base_address).start, page_size);
                if(op_window.length() < page_size) load(buf, page_size, phy_window.start);

                memcpy(buf, (uint8_t *)data + op_window.start, op_window.length());
                FLASH_ErasePage_Fast(phy_window.start);
                FLASH_ProgramPage_Fast(phy_window.start, (uint32_t *)&buf);
            }

        }while(op_window);
    

        lock();

    };

    void _load(void * data, const Address data_size, const Address loc) override{
        AddressWindow phy_window = AddressWindow{0, data_size}.shift(loc + base_address);
        memcpy(data, (void *)phy_window.start, data_size);
    };

public:
    Flash(Address _page_begin):Flash(_page_begin, Sys::Chip::getFlashSize() / page_size){;}
    Flash(Address _page_begin, Address _page_end):Storage((_page_end - _page_begin) * page_size),
            page_count(Sys::Chip::getFlashSize() / page_size){;}

    ~Flash(){}

    void init() override{

    }

    bool busy() override{
        return false;
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