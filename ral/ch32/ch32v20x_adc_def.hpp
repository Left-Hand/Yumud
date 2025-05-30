#pragma once

#include <cstdint>



#ifndef ASSERT_REG_IS_32BIT
#define ASSERT_REG_IS_32BIT(name) static_assert((sizeof(name) == 4),  #name " is not 32bit"); 
#endif


namespace CH32V20x{

struct R32_ADC_STATR{
    uint32_t AWD:1;
    uint32_t EOC:1;
    uint32_t JEOC:1;
    uint32_t JSTRT:1;
    uint32_t STRT:1;
    uint32_t :27;
};


struct R32_ADC_CTLR1{
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

    uint32_t DUALMOD:4;
    uint32_t __RESV1__:2;
    uint32_t JAWDEN:1;
    uint32_t AWDEN:1;

    uint32_t TKENABLE:1;
    uint32_t TKITUNE:1;
    uint32_t BUFEN:1;
    uint32_t PGA:2;
    uint32_t __RESV2__:3;
};ASSERT_REG_IS_32BIT(R32_ADC_CTLR1)


struct R32_ADC_CTLR2{
    uint32_t ADON:1;
    uint32_t CONT:1;
    uint32_t CAL:1;
    uint32_t RSTCAL:1;
    uint32_t __RESV1__:4;

    uint32_t DMA:1;
    uint32_t __RESV2__:2;
    uint32_t ALIGN:1;
    uint32_t JEXTSEL:3;
    uint32_t JEXTTRIG:1;

    uint32_t __RESV3__:1;
    uint32_t EXTSEL:3;
    uint32_t EXTTRIG:1;
    uint32_t JSWSTART:1;
    uint32_t SWSTART:1;
    uint32_t TSVREFE:1;

    uint32_t __RESV4__:8;
};ASSERT_REG_IS_32BIT(R32_ADC_CTLR2)

struct R32_ADC_SAMPTR1{
    uint32_t SMP10:3;
    uint32_t SMP11:3;
    uint32_t SMP12:3;
    uint32_t SMP13:3;
    uint32_t SMP14:3;
    uint32_t SMP15:3;
    uint32_t SMP16:3;
    uint32_t SMP17:3;
    uint32_t :8;
};ASSERT_REG_IS_32BIT(R32_ADC_SAMPTR1)

struct R32_ADC_SAMPTR2{
    uint32_t SMP0:3;
    uint32_t SMP1:3;
    uint32_t SMP2:3;
    uint32_t SMP3:3;
    uint32_t SMP4:3;
    uint32_t SMP5:3;
    uint32_t SMP6:3;
    uint32_t SMP7:3;
    uint32_t SMP8:3;
    uint32_t SMP9:3;
};ASSERT_REG_IS_32BIT(R32_ADC_SAMPTR2)

struct R32_ADC_IOFR{
    uint32_t OFFSET:12;
    uint32_t :20;
};

struct R32_ADC_WDHTR{
    uint32_t HT:12;
    uint32_t :20;
};

struct R32_ADC_WDLTR{
    uint32_t LT:12;
    uint32_t :20;
};

struct R32_ADC_RSQR1{
    uint32_t SQ13:5;
    uint32_t SQ14:5;
    uint32_t SQ15:5;
    uint32_t SQ16:5;
    uint32_t L:4;
    uint32_t :8;
};ASSERT_REG_IS_32BIT(R32_ADC_RSQR1)

struct R32_ADC_RSQR2{
    uint32_t SQ7:5;
    uint32_t SQ8:5;
    uint32_t SQ9:5;
    uint32_t SQ10:5;
    uint32_t SQ11:5;
    uint32_t SQ12:5;
    uint32_t :2;
};ASSERT_REG_IS_32BIT(R32_ADC_RSQR2)

struct R32_ADC_RSQR3{
    uint32_t SQ1:5;
    uint32_t SQ2:5;
    uint32_t SQ3:5;
    uint32_t SQ4:5;
    uint32_t SQ5:5;
    uint32_t SQ6:5;
    uint32_t :2;
};ASSERT_REG_IS_32BIT(R32_ADC_RSQR3)

struct R32_ADC_ISQR{
    uint32_t JSQ1:5;
    uint32_t JSQ2:5;
    uint32_t JSQ3:5;
    uint32_t JSQ4:5;
    uint32_t JL:2;
    uint32_t :10;
};ASSERT_REG_IS_32BIT(R32_ADC_ISQR)

struct R32_ADC_IDATAR{
    uint16_t DATA;
    uint16_t :16;
};

struct R32_ADC_RDATAR{
    uint16_t DATA;
    uint16_t :16;
};


struct R32_TKEY_CHARGE2{
    uint32_t TKCG0:3;
    uint32_t TKCG1:3;
    uint32_t TKCG2:3;
    uint32_t TKCG3:3;
    uint32_t TKCG4:3;
    uint32_t TKCG5:3;
    uint32_t TKCG6:3;
    uint32_t TKCG7:3;
    uint32_t TKCG8:3;
    uint32_t TKCG9:3;
    uint32_t :2;

};

struct R32_TKEY_CHARGE1{
 
    uint32_t TKCG10:3;
    uint32_t TKCG11:3;
    uint32_t TKCG12:3;
    uint32_t TKCG13:3;
    uint32_t TKCG14:3;
    uint32_t TKCG15:3;
    uint32_t TKCG16:3;
    uint32_t TKCG17:3;
    uint32_t :8;
};

struct R32_TKEY_CHGOFFSET{
    uint32_t TKCGOFFSET:8;
    uint32_t :24;
};

struct R32_TKEY_ACT_DCG{
    uint32_t TKACT_DCG:8;
    uint32_t :124;
};

struct R32_TKEY_DR{
    uint32_t DATA:16;
    uint32_t :116;
};

struct ADC_Def{
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
        CTLR1.EOCIE = en;
    }

    constexpr void enable_awd_it(const Enable en){
        CTLR1.AWDIE = en;
    }

    constexpr void enable_jeoc_it(const Enable en){
        CTLR1.JEOCIE = en;
    }

    constexpr void enable_scan_mode(const Enable en){
        CTLR1.SCAN = en;
    }

    constexpr void enable_single_channel_watchdog_when_scan(const Enable en){
        CTLR1.AWDSGL = en;
    }

    constexpr void enable_auto_inject(const Enable en){
        CTLR1.JAUTO = en;
    }

    constexpr void enable_regular_disc_mode(const Enable en){
        CTLR1.DISCEN = en;
    }

    constexpr void enable_injected_disc_mode(const Enable en){
        CTLR1.JDISCEN = en;
    }

    constexpr void set_regular_disc_num(const uint8_t num){
        CTLR1.DISCNUM = num;
    }

    constexpr void set_dual_mode(const uint8_t mode){
        CTLR1.DUALMOD = mode;
    }

    constexpr void enable_injected_watchdog(const Enable en){
        CTLR1.JAWDEN = en;
    }

    constexpr void enable_regular_watchdog(const Enable en){
        CTLR1.AWDEN = en;
    }

    constexpr void enable_touchkey(const Enable en){
        CTLR1.TKENABLE = en;
    }

    constexpr void enable_touchkey_low_charge(const Enable en){
        CTLR1.TKITUNE = en;
    }

    constexpr void enable_buffer(const Enable en){
        CTLR1.BUFEN = en;
    }

    constexpr void set_gain(const uint8_t gain){
        CTLR1.PGA = [&]{
            switch(gain){
                default:
                case 1:   return 0b00;
                case 4:   return 0b01; 
                case 16:  return 0b10;
                case 64:  return 0b11;
            }
        }();
    }

    constexpr void enable(const Enable en){
        CTLR2.ADON = en;
    }

    constexpr void enable_cont_mode(const Enable en){
        CTLR2.CONT = en;
    }

    constexpr void enable_calibrate(const Enable en){
        CTLR2.CAL = en;
    }

    constexpr void reset_calibrate(const Enable en){
        CTLR2.RSTCAL = en;
    }

    constexpr void enable_dma(const Enable en){
        CTLR2.DMA = en;
    }

    constexpr void enable_left_align(const Enable en){
        CTLR2.ALIGN = en;
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
        CTLR2.JEXTTRIG = en;
    }

    constexpr void select_regular_trigger(const uint8_t trigger){
        CTLR2.EXTSEL = trigger;
    }

    constexpr void enable_regular_trigger(const Enable en){
        CTLR2.EXTTRIG = en;
    }

    constexpr void soft_start_injected(){
        CTLR2.JSWSTART = 1;
    }

    constexpr void soft_start_regular(){
        CTLR2.SWSTART = 1;
    }

    constexpr void enable_temp_and_vref(const Enable en){
        CTLR2.TSVREFE = en;
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
            case 10: SAMPTR1.SMP10 = cycles; return;
            case 11: SAMPTR1.SMP11 = cycles; return;
            case 12: SAMPTR1.SMP12 = cycles; return;
            case 13: SAMPTR1.SMP13 = cycles; return;
            case 14: SAMPTR1.SMP14 = cycles; return;
            case 15: SAMPTR1.SMP15 = cycles; return;
            case 16: SAMPTR1.SMP16 = cycles; return;
            case 17: SAMPTR1.SMP17 = cycles; return;
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



struct TKEY_De1f{
    volatile R32_TKEY_CHARGE1 CHARGE1;
    volatile R32_TKEY_CHARGE2 CHARGE2;
    volatile R32_TKEY_CHGOFFSET CHGOFFSET;
    volatile R32_TKEY_ACT_DCG ACT_DCG;
    volatile R32_TKEY_DR DR;

    void set_charge_time(const uint8_t channel, const uint8_t time){
        switch(channel){
            case 0: CHARGE2.TKCG0 = time; break;
            case 1: CHARGE2.TKCG1 = time; break;
            case 2: CHARGE2.TKCG2 = time; break;
            case 3: CHARGE2.TKCG3 = time; break;
            case 4: CHARGE2.TKCG4 = time; break;
            case 5: CHARGE2.TKCG5 = time; break;
            case 6: CHARGE2.TKCG6 = time; break;
            case 7: CHARGE2.TKCG7 = time; break;
            case 8: CHARGE2.TKCG8 = time; break;
            case 9: CHARGE2.TKCG9 = time; break;
            case 10: CHARGE1.TKCG10 = time; break;
            case 11: CHARGE1.TKCG11 = time; break;
            case 12: CHARGE1.TKCG12 = time; break;
            case 13: CHARGE1.TKCG13 = time; break;
            case 14: CHARGE1.TKCG14 = time; break;
            case 15: CHARGE1.TKCG15 = time; break;
            case 16: CHARGE1.TKCG16 = time; break;
            case 17: CHARGE1.TKCG17 = time; break;
        }

        return;
    }

    void set_charge_offset(const uint8_t offset){
        CHGOFFSET.TKCGOFFSET = offset;
    }

    void start_conv(const uint8_t discharge_time){
        ACT_DCG.TKACT_DCG = discharge_time;
    }

    uint16_t get_data(){
        return DR.DATA;
    }
};

static inline ADC_Def * ADC1_Inst = (ADC_Def *)(0x40012400);
static inline ADC_Def * ADC2_Inst = (ADC_Def *)(0x40012400);
}