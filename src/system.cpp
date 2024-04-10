#include "system.hpp"
#include "src/crc/crc.hpp"

real_t Sys::t;

void Sys::Reset(){
    NVIC_SystemReset();
}


void Sys::reCalculateTime(){
        #ifdef USE_IQ
        t.value = msTick * (int)(0.001 * (1 << GLOBAL_Q));
        #else
        t = msTick * (1 / 1000.0f);
        #endif
    }


real_t Sys::getCurrentSeconds(){
        reCalculateTime();
        return t;
    }

uint64_t Sys::getChipId(){
    static uint32_t chip_id[2] = {
        *(volatile uint32_t *)0x1FFFF7E8,
        *(volatile uint32_t *)0x1FFFF7EC
    };
    return ((uint64_t)chip_id[1] << 32) | chip_id[0];
}

uint32_t Sys::getChipIdCrc(){
    static uint32_t chip_id_crc = 0;
    if(!chip_id_crc){
        crc.init();
        crc.clear();
        uint64_t chip_id = getChipId();
        chip_id_crc = crc.update({(uint32_t)chip_id, (uint32_t)(chip_id >> 32)});
    }
    return chip_id_crc;
}
