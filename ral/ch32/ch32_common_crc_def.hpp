#pragma once

#include <cstdint>

namespace CH32L103{
struct CRC_Def{
    volatile uint32_t DATAR;
    volatile uint8_t IDATAR;
    uint8_t :8;
    uint8_t :8;
    uint8_t :8;

    volatile uint32_t CTLR;

    void clear(){
        CTLR = 1;
    }

    void push(uint32_t data){
        DATAR = data;
    }

    uint32_t get(){
        return DATAR;
    }
};
}