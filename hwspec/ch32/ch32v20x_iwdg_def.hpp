#pragma once

#include <cstdint>


#ifndef ASSERT_REG_IS_32BIT
#define ASSERT_REG_IS_32BIT(name) static_assert((sizeof(name) == 4),  #name " is not 4 bytes"); 
#endif

#ifndef ASSERT_REG_IS_16BIT
#define ASSERT_REG_IS_16BIT(name) static_assert((sizeof(name) == 2),  #name " is not 2 bytes"); 
#endif

namespace CH32V20x{

struct R16_IWDG_CTLR {
    uint16_t KEY:16;
};

struct R16_IWDG_PSCR{
    uint16_t PR:3;
    uint16_t :13;
};

struct R16_IWDG_RLDR{
    uint16_t RL:12;
    uint16_t :4;
};

struct R16_IWDG_STATR{
    uint16_t PVU:1;
    uint16_t RVU:1;
    uint16_t :14;
};

struct IWDG_Def {
    volatile R16_IWDG_CTLR CTLR;
    uint16_t :16;
    volatile R16_IWDG_PSCR PSCR;
    uint16_t :16;
    volatile R16_IWDG_RLDR RLDR;
    uint16_t :16;
    volatile R16_IWDG_STATR STATR;
    uint16_t :16;

    constexpr void feed(){
        CTLR.KEY = 0xAAAA;
    }

    constexpr void enable_modify(){
        CTLR.KEY = 0x5555;
    }

    constexpr void start_iwdg(){
        CTLR.KEY = 0xcccc;
    }

    // 000：4 分频； 001：8 分频；
    // 010：16 分频； 011：32 分频；
    // 100：64 分频； 101：128 分频；
    // 110：256 分频； 111：256 分频。
    constexpr void set_prescale(const int prescale){
        PSCR.PR = [&]{
            switch(prescale){
                default:
                case 4:     return 0b000; 
                case 8:     return 0b001; 
                case 16:    return 0b010; 
                case 32:    return 0b011; 
                case 64:    return 0b100; 
                case 128:   return 0b101; 
                case 256:   return 0b110; 
            }
        }();
    }

    constexpr void set_reload(const uint16_t period){
        RLDR.RL = period;
    }

    constexpr bool prescale_is_updaing(){
        return STATR.PVU;
    }

    constexpr bool reload_is_updaing(){
        return STATR.RVU;
    }
};

static_assert(sizeof(IWDG_Def) == 4*4, "IWDG_Def size is not 16 bytes");
}