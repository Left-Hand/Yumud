#pragma once

#include "sys/core/system.hpp"
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

    using Page = size_t;
    using PageRange = Range_t<Page>;
    using Address = size_t;
    using AddressWindow = Range_t<Address>;

    scexpr Page page_size = 256;
    scexpr Address base_address = FLASH_WRProt_Sectors31to127;
    scexpr uint max_clock = 72000000;

    Page page_count;
    uint32_t orginal_clock;

    void configClock();

    void revertClock();

    void entry_store() override{
        configClock();
        // Systick_Init();
        // delay(1);
    }

    void exit_store() override{
        revertClock();
    }

    void entry_load() override{

    }

    void exit_load() override{
        revertClock();
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

    void storeBytes(const void * data, const Address len, const Address loc) override;

    void loadBytes(void * data, const Address len, const Address loc) override;

    void erasePage(const Address vaddr);
    void programPage(const Address vaddr, const void * buf);
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
