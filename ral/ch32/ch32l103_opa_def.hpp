#pragma once

#include <cstdint>

namespace CH32L103{

struct R16_OPA_CFGR1{
    uint16_t POLL_EN:1;
    uint16_t :1;
    uint16_t BKIN_EN:1;
    uint16_t :1;
    uint16_t RST_EN:1;
    uint16_t :2;
    uint16_t OPCM_LOCK:1;
    uint16_t IE_OUT:1;
    uint16_t :1;
    uint16_t IE_CNT:1;
    uint16_t NMI_EN:1;
    uint16_t IF_OUT:1;
    uint16_t :1;
    uint16_t IF_CNT:1;
    uint16_t :1;
};

struct R16_OPA_CFGR2{
    uint16_t POLL_VLU:8;
    uint16_t POLL1_NUM:3;
    uint16_t POLL1_CNT:3;
    uint16_t :1;
};

struct R32_OPA_CTLR1{
    uint32_t EN1:1;
    uint32_t MODE1:3;
    uint32_t PSEL1:3;
    uint32_t FB_EN:1;
    uint32_t NSEL1:1;
    uint32_t LP1:1;
    uint32_t :3;

    uint32_t ITRIMP:5;
    uint32_t :2;
    uint32_t ITRIMN:6;
    uint32_t :2;
};

struct R32_OPA_CTLR2{
    uint32_t EN1:1;
    uint32_t MODE1:2;
    uint32_t NSEL1:1;
    uint32_t PSEL1:1;
    uint32_t HYEN1:1;
    uint32_t LP1:1;
    uint32_t :1;

    uint32_t EN2:1;
    uint32_t MODE2:2;
    uint32_t NSEL2:1;
    uint32_t PSEL2:1;
    uint32_t HYEN2:1;
    uint32_t LP2:1;
    uint32_t :1;

    uint32_t EN3:1;
    uint32_t MODE3:2;
    uint32_t NSEL3:1;
    uint32_t PSEL3:1;
    uint32_t HYEN3:1;
    uint32_t LP3:1;
    uint32_t :1;

    uint32_t WKUP_MD:2;
    uint32_t :6;
};

using R32_OPCM_KEY = uint32_t;

struct OPA_Def{
    volatile R16_OPA_CFGR1 CFGR1;
    volatile R16_OPA_CFGR2 CFGR2;
    volatile R32_OPA_CTLR1 CTLR1;
    volatile R32_OPA_CTLR2 CTLR2;
    volatile R32_OPCM_KEY CMPKEY;


    constexpr void enable_polling(){
        CFGR1.POLL_EN = 1;
    }

    constexpr void enable_tim1_bkin_from_opa(bool en){
        CFGR1.BKIN_EN = en;
    }

    constexpr void enable_reset_chip_from_opa(bool en){
        CFGR1.RST_EN = en;
    }

    constexpr void lock_opa_cmp(){
        CFGR1.OPCM_LOCK = 1;
    }

    constexpr void enable_opa_interrupt(const bool en){
        CFGR1.IE_CNT = en;
    }

    constexpr void enable_opa_polling_interrupt(const bool en){
        CFGR1.IE_CNT = en;
    }

    constexpr void enable_nmi_from_opa(const bool en){
        CFGR1.NMI_EN = en;
    }

    constexpr void clear_opa_high_it_flag(){
        CFGR1.IF_OUT = 0;
    }

    bool get_opa_high_it_flag(){
        return CFGR1.IF_OUT;
    }

    constexpr void clear_polling_done_it_flag(){
        CFGR1.IF_CNT = 0;
    }
    bool get_polling_done_it_flag(){
        return CFGR1.IF_CNT;
    }

    constexpr void set_polling_duration_us(const uint16_t us){
        CFGR2.POLL_VLU = us - 1;
    }

    constexpr void set_polling_count(const uint8_t cnt){
        CFGR2.POLL1_NUM = cnt - 1;
    }

    uint8_t get_polling_index(){
        return CFGR2.POLL1_CNT;
    }

    constexpr void enable(const bool en){
        CTLR1.EN1 = en;
    }

    constexpr void set_opa_out_channel(const uint8_t ch){
        CTLR1.MODE1 = ch;
    }

    constexpr void set_opa_pos_channel(const uint8_t ch){
        CTLR1.PSEL1 = ch;
    }

    constexpr void enable_opa_fb_res(const bool en){
        CTLR1.FB_EN = en;
    }

    constexpr void set_opa_neg_channel(const uint8_t ch){
        CTLR1.NSEL1 = ch;
    }

    constexpr void enable_opa_lowpower_mode(const bool en){
        CTLR1.LP1 = en;
    }

    constexpr void set_opa_high_trim(const uint8_t trim){
        CTLR1.ITRIMP = trim;
    }

    constexpr void set_opa_low_trim(const uint8_t trim){
        CTLR1.ITRIMN = trim;
    }

    constexpr void enable_cmp(const uint8_t index, const bool en){
        switch(index){
            case 1: CTLR2.EN1 = en; break;
            case 2: CTLR2.EN2 = en; break;
            case 3: CTLR2.EN3 = en; break;
        }
    }

    constexpr void set_cmp_out_channel(const uint8_t index, const uint8_t ch){
        switch(index){
            case 1: CTLR2.MODE1 = ch; break;
            case 2: CTLR2.MODE2 = ch; break;
            case 3: CTLR2.MODE3 = ch; break;
        }
    }

    constexpr void set_cmp_neg_channel(const uint8_t index, const uint8_t ch){
        switch(index){
            case 1: CTLR2.NSEL1 = ch; break;
            case 2: CTLR2.NSEL2 = ch; break;
            case 3: CTLR2.NSEL3 = ch; break;
        }
    }

    constexpr void set_cmp_pos_channel(const uint8_t index, const uint8_t ch){
        switch(index){
            case 1: CTLR2.PSEL1 = ch; break;
            case 2: CTLR2.PSEL2 = ch; break;
            case 3: CTLR2.PSEL3 = ch; break;
        }
    }

    constexpr void enable_cmp_hysteresis(const uint8_t index,const bool en){
        switch(index){
            case 1: CTLR2.HYEN1 = en; break;
            case 2: CTLR2.HYEN2 = en; break;
            case 3: CTLR2.HYEN3 = en; break;
        }
    }

    constexpr void enable_cmp_lowpower_mode(const uint8_t index,const bool en){
        switch(index){
            case 1: CTLR2.LP1 = en; break;
            case 2: CTLR2.LP2 = en; break;
            case 3: CTLR2.LP3 = en; break;
        }
    }

    constexpr void unlock_opa(){
        constexpr uint32_t KEY = 0x45670123;
        CMPKEY = KEY;
    }

    constexpr void unlock_cmp(){
        constexpr uint32_t KEY = 0xCDEF89AB;
        CMPKEY = KEY;
    }
};

}