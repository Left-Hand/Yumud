#pragma once

#include <cstdint>


#ifndef BIT_CAST
#define BIT_CAST(type, source) __builtin_bit_cast(type, (source))
#endif

namespace CH32L103{

#define DEF_LPTIM_FLAGS\
    uint32_t CMPM:1;\
    uint32_t ARRM:1;\
    uint32_t EXTTRIG:1;\
    uint32_t CMPOK:1;\
    uint32_t ARROK:1;\
    uint32_t UP:1;\
    uint32_t DOWN:1;\

struct R32_LPTIM_ISR{
    static constexpr uint32_t offset = 0x00;

    DEF_LPTIM_FLAGS;
    uint32_t DIR_SYNC:1;
    uint32_t :24;
};

struct R32_LPTIM_ICR{
    static constexpr uint32_t offset = 04;

    DEF_LPTIM_FLAGS;
    uint32_t :25;
};


struct R32_LPTIM_IER{
    static constexpr uint32_t offset = 0x08;

    DEF_LPTIM_FLAGS;
};

struct R32_LPTIM_CFGR{
    static constexpr uint32_t offset = 0x0C;

    uint32_t CKSEL:1;
    uint32_t CKPOL:2;
    uint32_t CKFLT:2;
    uint32_t :1;
    uint32_t TRGFLT:2;
    uint32_t :1;
    uint32_t PRESC:3;
    uint32_t :1;
    uint32_t TRIGSEL:2;
    uint32_t :2;
    uint32_t TRIGEN:2;
    uint32_t TIMOUT:1;
    uint32_t WAVE:1;
    uint32_t WAVPOL:1;
    uint32_t PRELOAD:1;
    uint32_t COUNTMODE:1;
    uint32_t ENC:1;
    uint32_t CLKMX_SEL:1;
    uint32_t FORCE_PWM:1;
    uint32_t :4;
};

struct R32_LPTIM_CR{
    static constexpr uint32_t offset = 0x10;

    uint32_t ENABLE:1;
    uint32_t SNGSTRT:1;
    uint32_t CNTSTRT:1;
    uint32_t OUTEN:1;
    uint32_t DIR_EXTEN:1;
    uint32_t :27;
};

struct R32_LPTIM_CMP{
    static constexpr uint32_t offset = 0x10;

    uint16_t CMP;
    uint16_t :16;
};

struct R32_LPTIM_ARR{
    static constexpr uint32_t offset = 0x10;

    uint16_t ARR;
    uint16_t :16;
};

struct R32_LPTIM_CNT{
    static constexpr uint32_t offset = 0x10;

    uint16_t CNT;
    uint16_t :16;
};


struct LPTIM_Def{
    volatile R32_LPTIM_ISR ISR;
    volatile R32_LPTIM_ICR ICR;
    volatile R32_LPTIM_IER IER;
    volatile R32_LPTIM_CFGR CFGR;
    volatile R32_LPTIM_CR CR;
    volatile R32_LPTIM_CMP CMP;
    volatile R32_LPTIM_ARR ARR;
    volatile R32_LPTIM_CNT CNT;

    struct Events{
        DEF_LPTIM_FLAGS;
    };

    bool is_cnt_matched(){
        return ISR.CMPM;
    }

    bool is_arr_matched(){
        return ISR.ARRM;
    }

    bool has_ext_trig(){
        return ISR.EXTTRIG;
    }

    bool is_cmp_update_ok(){
        return ISR.CMPOK;
    }

    bool is_arr_update_ok(){
        return ISR.ARROK;
    }

    void set_count_direction_is_up(const bool dir){
        ISR.UP = dir;
    }

    void clear_interrupts(const Events ev){
        const_cast<R32_LPTIM_ICR &>(ICR) = BIT_CAST(R32_LPTIM_ICR, ev);
    }

    void enable_intrrupts(const Events ev){
        const_cast<R32_LPTIM_IER &>(IER) = BIT_CAST(R32_LPTIM_IER, ev);
    }

    void enable_use_ext_clock(const Enable en){
        CFGR.CKSEL = en;
    }

    void set_valid_edge(const uint8_t edge){
        CFGR.CKPOL = edge;
    }

    void set_encoder_mode(const Enable en){
        CFGR.CKPOL = en;
    }

    void set_ext_clock_filter(const uint8_t flt){
        CFGR.CKFLT = flt;
    }

    void set_trigger_filter(const uint8_t flt){
        CFGR.TRGFLT = flt;
    }

    constexpr void set_prescale(const uint8_t prescale){
        CFGR.PRESC = [&]->uint8_t{
            switch(prescale){
                case 1:return 0;
                case 2:return 1;
                case 4:return 2;
                case 8:return 3;
                case 16:return 4;
                case 32:return 5;
                case 64:return 6;
                case 128:return 7;
            }
        }();
    }

    constexpr void set_trigger_source(const uint8_t src){
        CFGR.TRIGSEL = src;
    }

    constexpr void set_trigger_edge(const uint8_t mode){
        CFGR.TRIGEN = mode;
    }

    constexpr void set_timeout(const Enable en){
        CFGR.TIMOUT = en;
    }

    constexpr void enable_oneshot_wave(const Enable en){
        CFGR.WAVE = en;
    }

    constexpr void set_wave_polarity(const bool pol){
        CFGR.WAVPOL = pol;
    }

    constexpr void set_update_mode(const bool mode){
        CFGR.PRELOAD = mode;
    }

    constexpr void set_count_mode(const bool mode){
        CFGR.COUNTMODE = mode;
    }

    constexpr void enable_encoder_mode(const Enable en){
        CFGR.ENC = en;
    }

    constexpr void set_internal_clock_source(const Enable en){
        CFGR.CLKMX_SEL = en;
    }

    constexpr void enable_force_pwm(const Enable en){
        CFGR.FORCE_PWM = en;
    }

    constexpr void enable(const Enable en){
        CR.ENABLE = en;
    }

    constexpr void start_single_count(const Enable en){
        CR.SNGSTRT = en;
    }

    constexpr void start_continuous_count(const Enable en){
        CR.CNTSTRT = en;
    }

    constexpr void enable_output(const Enable en){
        CR.OUTEN = en;
    }

    constexpr void enable_ext_trig_count_direction(const Enable en){
        CR.DIR_EXTEN = en;
    }

    void set_cmp(const uint16_t cmp){
        CMP.CMP = cmp;
    }

    uint16_t get_cmp(){
        return CMP.CMP;
    }

    void set_arr(const uint16_t arr){
        ARR.ARR = arr;
    }

    uint16_t get_arr(){
        return ARR.ARR;
    }

    void set_cnt(const uint16_t cnt){
        CNT.CNT = cnt;
    }

    uint16_t get_cnt(){
        return CNT.CNT;
    }
};

static constexpr LPTIM_Def * LPTIM1 = reinterpret_cast<LPTIM_Def *>(0x40007C00);


#undef DEF_LPTIM_FLAGS
}