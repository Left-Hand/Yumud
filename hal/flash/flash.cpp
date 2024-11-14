#include "flash.hpp"

using namespace ymd;


#define CLOCK_CONFIG_BY_AHB 0
#define CLOCK_CONFIG_BY_REG 1

#define CLOCK_CONFIG CLOCK_CONFIG_BY_REG

void Flash::storeBytes(const Address loc, const void * data, const Address len){

    unlock();
    uint8_t buf[page_size] = {0};

    AddressWindow store_window = AddressWindow{loc,loc + len};
    AddressWindow op_window = {0,0};
    do{
        op_window = store_window.grid_forward(op_window, page_size);
        if(op_window){
            auto page_window = AddressWindow::grid(op_window.from, page_size);

            switch(page_window.rooms(op_window)){
                case 0:
                    erasePage(page_window.from);  
                    programPage(page_window.from, &reinterpret_cast<const uint8_t*>(data)[page_window.from - loc]);
                    break;

                case 1:
                    if(page_window.room_right(op_window)){
                        // {----[--]}
                        auto padding_len = page_size - op_window.length();
                        loadBytes(page_window.from, buf, padding_len);
                        memcpy(buf + (op_window.from - page_window.from), data, op_window.length());

                        erasePage(page_window.from);  
                        programPage(page_window.from, buf);
                    }else{ 
                        // {[--]----}
                        erasePage(page_window.from);  
                        programPage(page_window.from, &reinterpret_cast<const uint8_t*>(data)[op_window.from - loc]);
                    }
                    break;

                case 2:
                    // {--[--]--}
                    loadBytes(page_window.from, buf, page_size);
                    memcpy(buf + (op_window.from - page_window.from), data, op_window.length());

                    erasePage(page_window.from);  
                    programPage(page_window.from, buf);
                    break;
            }
            // if(op_window.length() < page_size){
                
            //     erasePage(page_window.from);
            // }else{

            //     erasePage(page_window.from);
            // }
        }
    }while(op_window);


    lock();
};

void Flash::loadBytes(const Address loc, void * data, const Address len){
    FAULT_IF(!ISALIGNED(data))
    memcpy(data, (void *)(loc + base_address), len);
};

void Flash::erasePage(const Address vaddr){
    auto phyaddr = base_address + vaddr;
    FAULT_IF(!ISALIGNED(phyaddr));
    FLASH_ErasePage_Fast(phyaddr);
}

void Flash::programPage(const Address vaddr, const void * buf){
    auto phyaddr = base_address + vaddr;
    FAULT_IF(!ISALIGNED(phyaddr));
    FLASH_ProgramPage_Fast(phyaddr, reinterpret_cast<const uint32_t *>(buf));
}

void Flash::configClock(){
    #if (CLOCK_CONFIG == CLOCK_CONFIG_BY_AHB)

    orginal_clock = Sys::Clock::getAHBFreq();
    if(orginal_clock > max_clock) Sys::Clock::setAHBFreq(max_clock);

    #elif (CLOCK_CONFIG == CLOCK_CONFIG_BY_REG)

    FLASH_Access_Clock_Cfg(FLASH_Access_SYSTEM_HALF);

    #endif
}

void Flash::revertClock(){

    #if (CLOCK_CONFIG == CLOCK_CONFIG_BY_AHB)

    Sys::Clock::setAHBFreq(orginal_clock);
    
    #elif (CLOCK_CONFIG == CLOCK_CONFIG_BY_REG)

    FLASH_Access_Clock_Cfg(FLASH_Access_SYSTEM);

    #endif
}