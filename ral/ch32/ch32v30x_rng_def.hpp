#pragma once

#include <cstdint>

namespace CH32V30X{

struct R32_RNG_CR{
    uint32_t :1;
    uint32_t RNGEN:1;
    uint32_t IE:1;
    uint32_t :28;
};

struct R32_RNG_SR{
    uint32_t DRDY:1;
    uint32_t CECS:1;
    uint32_t SECS:1;
    uint32_t :2;
    uint32_t CEIS:1;
    uint32_t SEIS:1;
    uint32_t :25;
};

struct R32_RNG_DR{
    uint32_t DATA;   
};

struct RNG_Def{
    volatile R32_RNG_CR CR;
    volatile R32_RNG_SR SR;
    volatile R32_RNG_DR DR;

    void enable(const Enable en){
        CR.RNGEN = en;
    }

    void enable_interrupt(const Enable en){
        CR.IE = en;
    }

    bool is_data_ready(){
        return SR.DRDY;
    }

    bool is_clock_error(){
        return SR.CEIS;
    }

    bool is_seed_error(){
        return SR.SEIS;
    }

    uint32_t get_data(){
        return DR.DATA;
    }
};

}