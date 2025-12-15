#pragma once

#include "core/constants/enums.hpp"


#ifndef ASSERT_REG_IS_32BIT
#define ASSERT_REG_IS_32BIT(name) static_assert((sizeof(name) == 4),  #name " is not 32bit"); 
#endif


namespace ymd::ral::ch32::ch32m030_adc{

struct [[nodiscard]] R32_ADC_STATR{
    uint32_t AWD:1;
    uint32_t EOC:1;
    uint32_t JEOC:1;
    uint32_t JSTRT:1;
    uint32_t STRT:1;
    uint32_t AWD_OVF:1;
    uint32_t AWD_DNF:1;
    uint32_t MULT_CMP1:1;
    uint32_t MULT_CMP2:1;
    uint32_t MULT_CMP3:1;
    uint32_t :6;
    uint32_t COVFIF:1;
    uint32_t CDNFIF:1;
    uint32_t :14;
};ASSERT_REG_IS_32BIT(R32_ADC_STATR)


struct [[nodiscard]] R32_ADC_CTLR1{
    uint32_t AWDCH:5;
    uint32_t EOCIE:1;
    uint32_t AWDIE:1;
    uint32_t JEOCIE:1;

    uint32_t SCAN:1;
    uint32_t AWDSGL:1;
    uint32_t JAUTO:1;
    uint32_t DISCEN:1;
    uint32_t JDISCEN:1;
    uint32_t DISCNUM:3;

    uint32_t __RESV1__:6;
    uint32_t JAWDEN:1;
    uint32_t AWDEN:1;
    uint32_t __RESV2__:2;
    uint32_t BUFEN:1;
    uint32_t __RESV3__:1;
    uint32_t OVFIE:1;
    uint32_t DNFIE:1;
    uint32_t MULT_CMPEN:1;
};ASSERT_REG_IS_32BIT(R32_ADC_CTLR1)


struct [[nodiscard]] R32_ADC_CTLR2{
    uint32_t ADON:1;
    uint32_t CONT:1;
    uint32_t CAL:1;
    uint32_t RSTCAL:1;
    uint32_t __RESV1__:4;

    uint32_t DMA:1;
    uint32_t __RESV2__:1;
    uint32_t ALIGN:1;
    uint32_t JEXTSEL:4;
    uint32_t JEXTTRIG:1;

    uint32_t EXTSEL:4;
    uint32_t EXTTRIG:1;
    uint32_t JSWSTART:1;
    uint32_t SWSTART:1;

    uint32_t __RESV4__:9;
};ASSERT_REG_IS_32BIT(R32_ADC_CTLR2)

struct [[nodiscard]] R32_ADC_SAMPTR1{
    uint32_t SMP16:2;
    uint32_t SMP17:2;
    uint32_t SMP18:2;
    uint32_t SMP19:2;
    uint32_t :24;
};ASSERT_REG_IS_32BIT(R32_ADC_SAMPTR1)

struct [[nodiscard]] R32_ADC_SAMPTR2{
    uint32_t SMP0:2;
    uint32_t SMP1:2;
    uint32_t SMP2:2;
    uint32_t SMP3:2;
    uint32_t SMP4:2;
    uint32_t SMP5:2;
    uint32_t SMP6:2;
    uint32_t SMP7:2;
    uint32_t SMP8:2;
    uint32_t SMP9:2;
    uint32_t SMP10:2;
    uint32_t SMP11:2;
    uint32_t SMP12:2;
    uint32_t SMP13:2;
    uint32_t SMP14:2;
    uint32_t SMP15:2;
};ASSERT_REG_IS_32BIT(R32_ADC_SAMPTR2)

struct [[nodiscard]] R32_ADC_IOFR{
    uint32_t OFFSET:12;
    uint32_t :20;
};

struct [[nodiscard]] R32_ADC_WDHTR{
    uint32_t HT:12;
    uint32_t :20;
};

struct [[nodiscard]] R32_ADC_WDLTR{
    uint32_t LT:12;
    uint32_t :20;
};

struct [[nodiscard]] R32_ADC_RSQR1{
    uint32_t SQ13:5;
    uint32_t SQ14:5;
    uint32_t SQ15:5;
    uint32_t SQ16:5;
    uint32_t L:4;
    uint32_t :8;
};ASSERT_REG_IS_32BIT(R32_ADC_RSQR1)

struct [[nodiscard]] R32_ADC_RSQR2{
    uint32_t SQ7:5;
    uint32_t SQ8:5;
    uint32_t SQ9:5;
    uint32_t SQ10:5;
    uint32_t SQ11:5;
    uint32_t SQ12:5;
    uint32_t :2;
};ASSERT_REG_IS_32BIT(R32_ADC_RSQR2)

struct [[nodiscard]] R32_ADC_RSQR3{
    uint32_t SQ1:5;
    uint32_t SQ2:5;
    uint32_t SQ3:5;
    uint32_t SQ4:5;
    uint32_t SQ5:5;
    uint32_t SQ6:5;
    uint32_t :2;
};ASSERT_REG_IS_32BIT(R32_ADC_RSQR3)

struct [[nodiscard]] R32_ADC_ISQR{
    uint32_t JSQ1:5;
    uint32_t JSQ2:5;
    uint32_t JSQ3:5;
    uint32_t JSQ4:5;
    uint32_t JL:2;
    uint32_t :10;
};ASSERT_REG_IS_32BIT(R32_ADC_ISQR)

struct [[nodiscard]] R32_ADC_IDATAR{
    uint16_t DATA;
    uint16_t :16;
};

struct [[nodiscard]] R32_ADC_RDATAR{
    uint16_t DATA;
    uint16_t :16;
};


struct [[nodiscard]] R32_ADC_CTLR3{
    uint32_t SW_DIS:1;
    uint32_t :2;
    uint32_t AWD_SCAN:1;
    uint32_t AWD0_RST_SCAN:1;
    uint32_t AWD1_RST_SCAN:1;
    uint32_t AWD2_RST_SCAN:1;
    uint32_t CH_SAVE:1;


    uint32_t AWD0_RES:1;
    uint32_t AWD1_RES:1;
    uint32_t AWD3_RES:1;
    uint32_t :5;

    uint32_t COFIF:1;
    uint32_t DNFIF:1;
    uint32_t OVF:1;
    uint32_t DNF:1;
    uint32_t AVG_K:3;
    uint32_t AVG_EN:1;

    uint32_t AVG_OVFIE:1;
    uint32_t AVG_DNFIE:1;
    uint32_t CMP_XOR_MODE:1;
    uint32_t RDR_POLL:1;
    uint32_t __RESV__:4;
};

struct [[nodiscard]] R32_ADC_WDTR1{
    uint32_t LTR1:12;
    uint32_t __RESV1__:4;
    uint32_t HTR1:12;
    uint32_t __RESV2__:4;
};

struct [[nodiscard]] R32_ADC_WDTR2{
    uint32_t LTR2:12;
    uint32_t __RESV1__:4;
    uint32_t HTR2:12;
    uint32_t __RESV2__:4;
};

struct [[nodiscard]] R32_ADC_WDTR3{
    uint32_t LTR3:12;
    uint32_t __RESV1__:4;
    uint32_t HTR3:12;
    uint32_t __RESV2__:4;
};

struct [[nodiscard]] R32_ADC_DLYR{

};

struct [[nodiscard]] R32_ADC_ADR{

};

struct [[nodiscard]] ADC_Def{
    volatile R32_ADC_STATR STATR;
    volatile R32_ADC_CTLR1 CTLR1;
    volatile R32_ADC_CTLR2 CTLR2;
    volatile R32_ADC_SAMPTR1 SAMPTR1;
    volatile R32_ADC_SAMPTR2 SAMPTR2;
    volatile R32_ADC_IOFR IOFR[4];
    volatile R32_ADC_WDHTR WDHTR;
    volatile R32_ADC_WDLTR WDLTR;
    volatile R32_ADC_RSQR1 RSQR1;
    volatile R32_ADC_RSQR2 RSQR2;
    volatile R32_ADC_RSQR3 RSQR3;
    volatile R32_ADC_ISQR ISQR;
    volatile R32_ADC_IDATAR IDATAR[4];
    volatile R32_ADC_RDATAR RDATAR;

    constexpr bool analog_watchdog_happend(){
        return STATR.AWD;
    }

    constexpr bool is_conv_done(){
        return STATR.EOC;
    }

    constexpr bool is_injected_conv_done(){
        return STATR.JEOC;
    }
    
    constexpr bool is_injected_conv_started(){
        return STATR.JSTRT;
    }

    constexpr bool is_regular_conv_started(){
        return STATR.STRT;
    }

    constexpr void set_analog_watchdog_threshold(const int low,const int high){
        WDLTR.LT = low;
        WDHTR.HT = high;
    }

    constexpr void set_analog_watchdog_channel(const uint8_t channel){
        CTLR1.AWDCH = channel;
    }

    constexpr void enable_eoc_it(const Enable en){
        CTLR1.EOCIE = en == EN;
    }

    constexpr void enable_awd_it(const Enable en){
        CTLR1.AWDIE = en == EN;
    }

    constexpr void enable_jeoc_it(const Enable en){
        CTLR1.JEOCIE = en == EN;
    }

    constexpr void enable_scan_mode(const Enable en){
        CTLR1.SCAN = en == EN;
    }

    constexpr void enable_single_channel_watchdog_when_scan(const Enable en){
        CTLR1.AWDSGL = en == EN;
    }

    constexpr void enable_auto_inject(const Enable en){
        CTLR1.JAUTO = en == EN;
    }

    constexpr void enable_regular_disc_mode(const Enable en){
        CTLR1.DISCEN = en == EN;
    }

    constexpr void enable_injected_disc_mode(const Enable en){
        CTLR1.JDISCEN = en == EN;
    }

    constexpr void set_regular_disc_num(const uint8_t num){
        CTLR1.DISCNUM = num;
    }


    constexpr void enable_injected_watchdog(const Enable en){
        CTLR1.JAWDEN = en == EN;
    }

    constexpr void enable_regular_watchdog(const Enable en){
        CTLR1.AWDEN = en == EN;
    }
    constexpr void enable_buffer(const Enable en){
        CTLR1.BUFEN = en == EN;
    }

    constexpr void enable(const Enable en){
        CTLR2.ADON = en == EN;
    }

    constexpr void enable_cont_mode(const Enable en){
        CTLR2.CONT = en == EN;
    }

    constexpr void enable_calibrate(const Enable en){
        CTLR2.CAL = en == EN;
    }

    constexpr void reset_calibrate(const Enable en){
        CTLR2.RSTCAL = en == EN;
    }

    constexpr void enable_dma(const Enable en){
        CTLR2.DMA = en == EN;
    }

    constexpr void enable_left_align(const Enable en){
        CTLR2.ALIGN = en == EN;
    }

    // 000：定时器 1 的 TRGO 事件；
    // 001：定时器 1 的 CC4 事件；
    // 010：定时器 2 的 TRGO 事件；
    // 011：定时器 2 的 CC1 事件；
    // 100：定时器 3 的 CC4 事件；
    // 101：定时器 4 的 TRGO 事件；
    // 110：EXTI 线 15/定时器 8 的 CC4 事件；
    // 111：JSWSTART 软件触发。
    constexpr void select_injected_trigger(const uint8_t trigger){
        CTLR2.JEXTSEL = trigger;
    }

    constexpr void enable_injected_trigger(const Enable en){
        CTLR2.JEXTTRIG = en == EN;
    }

    constexpr void select_regular_trigger(const uint8_t trigger){
        CTLR2.EXTSEL = trigger;
    }

    constexpr void enable_regular_trigger(const Enable en){
        CTLR2.EXTTRIG = en == EN;
    }

    constexpr void soft_start_injected(){
        CTLR2.JSWSTART = 1;
    }

    constexpr void soft_start_regular(){
        CTLR2.SWSTART = 1;
    }

    constexpr void set_sample_cycles(const uint8_t channel,const uint8_t cycles){
        switch(channel){
            case 0: SAMPTR2.SMP0 = cycles; return;
            case 1: SAMPTR2.SMP1 = cycles; return;
            case 2: SAMPTR2.SMP2 = cycles; return;
            case 3: SAMPTR2.SMP3 = cycles; return;
            case 4: SAMPTR2.SMP4 = cycles; return;
            case 5: SAMPTR2.SMP5 = cycles; return;
            case 6: SAMPTR2.SMP6 = cycles; return;
            case 7: SAMPTR2.SMP7 = cycles; return;
            case 8: SAMPTR2.SMP8 = cycles; return;
            case 9: SAMPTR2.SMP9 = cycles; return;
            case 10: SAMPTR2.SMP10 = cycles; return;
            case 11: SAMPTR2.SMP11 = cycles; return;
            case 12: SAMPTR2.SMP12 = cycles; return;
            case 13: SAMPTR2.SMP13 = cycles; return;
            case 14: SAMPTR2.SMP14 = cycles; return;
            case 15: SAMPTR2.SMP15 = cycles; return;
            case 16: SAMPTR1.SMP16 = cycles; return;
            case 17: SAMPTR1.SMP17 = cycles; return;
            case 18: SAMPTR1.SMP18 = cycles; return;
            case 19: SAMPTR1.SMP19 = cycles; return;
            default: return;
        }
    }

    constexpr void set_injected_offset(const uint8_t rank,const uint16_t offset){
        IOFR[rank & 0b11].OFFSET = offset;
    }

    constexpr void set_regular_mapping(const uint8_t rank, const uint8_t channel){
        switch (rank) {
            case 1: RSQR3.SQ1 = channel; return;
            case 2: RSQR3.SQ2 = channel; return;
            case 3: RSQR3.SQ3 = channel; return;
            case 4: RSQR3.SQ4 = channel; return;
            case 5: RSQR3.SQ5 = channel; return;
            case 6: RSQR3.SQ6 = channel; return;
            case 7: RSQR2.SQ7 = channel; return;
            case 8: RSQR2.SQ8 = channel; return;
            case 9: RSQR2.SQ9 = channel; return;
            case 10: RSQR2.SQ10 = channel; return;
            case 11: RSQR2.SQ11 = channel; return;
            case 12: RSQR2.SQ12 = channel; return;
            case 13: RSQR1.SQ13 = channel; return;
            case 14: RSQR1.SQ14 = channel; return;
            case 15: RSQR1.SQ15 = channel; return;
            case 16: RSQR1.SQ16 = channel; return;
        }
    }

    constexpr void set_regular_length(const uint8_t len){
        RSQR1.L = len;
    }

    constexpr void set_injected_mapping(const uint8_t rank, const uint8_t channel){
        switch (rank) {
            case 1: ISQR.JSQ1 = channel; return;
            case 2: ISQR.JSQ2 = channel; return;
            case 3: ISQR.JSQ3 = channel; return;
            case 4: ISQR.JSQ4 = channel; return;
        }
    }

    constexpr void set_injected_length(const uint8_t len){
        ISQR.JL = len;
    }

    constexpr uint16_t get_injected_data(const uint8_t rank){
        return IDATAR[(rank-1) & 0b11].DATA;
    }

    constexpr uint16_t get_regular_data(){
        return RDATAR.DATA;
    }

};
static_assert(sizeof(ADC_Def) == (0x40012450 - 0x40012400), "ADC_Def size mismatch");
static_assert(sizeof(ADC_Def) == (0x40012850 - 0x40012800), "ADC_Def size mismatch");



static inline ADC_Def * ADC1_Inst = (ADC_Def *)(0x40012400);
static inline ADC_Def * ADC2_Inst = (ADC_Def *)(0x40012400);
}