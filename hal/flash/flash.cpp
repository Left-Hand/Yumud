#include "flash.hpp"
#include "sys/core/sdk.h"

using namespace ymd;

#define CLOCK_CONFIG_BY_AHB 0
#define CLOCK_CONFIG_BY_REG 1

#define CLOCK_CONFIG CLOCK_CONFIG_BY_REG
// #define CLOCK_CONFIG CLOCK_CONFIG_BY_AHB

void Flash::storeBytes(const Address vaddr, const void * data, const Address len){

    unlock();
    // DEBUG_PRINTLN("store");
    erasePage(vaddr);  
    programPage(vaddr, reinterpret_cast<const uint8_t*>(data));

    // uint8_t buf[page_size] = {0};

    // AddressWindow store_window = AddressWindow{loc,loc + len};
    // AddressWindow op_window = {0,0};
    // do{
    //     op_window = store_window.grid_forward(op_window, page_size);
    //     if(op_window){
    //         auto page_window = AddressWindow::grid(op_window.from, page_size);

    //         switch(page_window.rooms(op_window)){
    //             case 0:
    //                 erasePage(page_window.from);  
    //                 programPage(page_window.from, &reinterpret_cast<const uint8_t*>(data)[page_window.from - loc]);
    //                 break;

    //             case 1:
    //                 if(page_window.room_right(op_window)){
    //                     // {----[--]}
    //                     auto padding_len = page_size - op_window.length();
    //                     loadBytes(page_window.from, buf, padding_len);
    //                     memcpy(buf + (op_window.from - page_window.from), data, op_window.length());

    //                     erasePage(page_window.from);  
    //                     programPage(page_window.from, buf);
    //                 }else{ 
    //                     // {[--]----}
    //                     erasePage(page_window.from);  
    //                     programPage(page_window.from, &reinterpret_cast<const uint8_t*>(data)[op_window.from - loc]);
    //                 }
    //                 break;

    //             case 2:
    //                 // {--[--]--}
    //                 loadBytes(page_window.from, buf, page_size);
    //                 memcpy(buf + (op_window.from - page_window.from), data, op_window.length());

    //                 erasePage(page_window.from);  
    //                 programPage(page_window.from, buf);
    //                 break;
    //         }
    //         // if(op_window.length() < page_size){
                
    //         //     erasePage(page_window.from);
    //         // }else{

    //         //     erasePage(page_window.from);
    //         // }
    //     }
    // }while(op_window);


    lock();
};

void Flash::loadBytes(const Address vaddr, void * data, const Address len){
    FAULT_IF(!ISALIGNED(data))
    const auto phyaddr = base_address + vaddr;
    // DEBUG_VALUE(phyaddr, "load")
    memcpy(data, (void *)(phyaddr), len);
};

void Flash::erasePage(const Address vaddr){
    const auto phyaddr = base_address + vaddr;
    FAULT_IF(!ISALIGNED(phyaddr));
    BREAKPOINT
    // delay(2);
    // DEBUG_VALUE(phyaddr, "erase")
    // FLASH_ErasePage_Fast(phyaddr);
    FLASH_ErasePage(phyaddr);
}

void Flash::lock(){
    FLASH_Lock_Fast();
    FLASH_Lock();
    __enable_irq();
}

void Flash::unlock(){
    __disable_irq();
    __disable_irq();
    __nopn(4);
    FLASH_Unlock();
    FLASH_Unlock_Fast();

    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP|FLASH_FLAG_WRPRTERR);
}

void Flash::programPage(const Address vaddr, const void * buf){
    auto phyaddr = base_address + vaddr;
    FAULT_IF(!ISALIGNED(phyaddr));
    // DEBUG_VALUE(std::hex, phyaddr, "prog")
    if(false){
    // if(true){
        FLASH_ProgramPage_Fast(phyaddr, reinterpret_cast<const uint32_t *>(buf));
    }else{
        // const size_t len = 10;
        const auto * p = reinterpret_cast<const uint16_t *>(buf);
        // for(size_t i = 0; i < 128; i++){
        for(size_t i = 0; i < 2; i++){
            FLASH_ProgramHalfWord(phyaddr + 2 * i, p[i]);
        }
    }
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

void Flash::entry_store(){
    configClock();
    // Systick_Init();
    // delay(1);
}

void Flash::exit_store(){
    revertClock();
}

void Flash::entry_load(){

}

void Flash::exit_load(){
    revertClock();
}
