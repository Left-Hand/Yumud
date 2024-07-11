#ifndef __FLASH_HPP__

#define __FLASH_HPP__

#include "sys/platform.h"
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

    void entry_store() override{
        settleClock();
        Systick_Init();
        delay(10);
    }

    void exit_store() override{
        resetClock();
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
        __disable_irq();
        __nopn(4);
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
                auto phy_window = AddressWindow::grid(op_window.shift(base_address).start, page_size);
                // if(op_window.length() < page_size) load(buf, page_size, phy_window.start);

                // memcpy(buf, (uint8_t *)data + op_window.start, op_window.length());
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

#endif