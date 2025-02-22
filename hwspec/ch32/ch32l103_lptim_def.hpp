#pragma once

#include <sys/io/regs.hpp>

namespace hwspec::CH32L103::USBPD_Defs{

struct R32_ISR{
    static constexpr uint32_t offset = 0x00;

    uint32_t CMPM:1;
    uint32_t ARRM:1;
    uint32_t EXTTRIG:1;
    uint32_t CMPOK:1;
    uint32_t ARROK:1;
    uint32_t UP:1;
    uint32_t DOWN:1;
    uint32_t DIR_SYNC:1;
    uint32_t :24;
};

struct R32_ICR{
    static constexpr uint32_t offset = 04;

    uint32_t CMPM_NCF:1;
    uint32_t ARRM_NCF:1;
    uint32_t EXTTRIG_NCF:1;
    uint32_t CMPOK_NCF:1;
    uint32_t ARROK_NCF:1;
    uint32_t UP_NCF:1;
    uint32_t DOWN_NCF:1;
    uint32_t :25;
};


struct R32_IER{
    static constexpr uint32_t offset = 0x08;

    uint32_t CMPM_IE:1;
    uint32_t ARRM_IE:1;
    uint32_t EXTTRIG_IE:1;
    uint32_t CMPOK_IE:1;
    uint32_t ARROK_IE:1;
    uint32_t UP_IE:1;
    uint32_t DOWN_IE:1;
};

struct R32_CFGR{
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

struct R32_CR{
    static constexpr uint32_t offset = 0x10;

    uint32_t ENABLE:1;
    uint32_t SNGSTRT:1;
    uint32_t CNTSTRT:1;
    uint32_t OUTEN:1;
    uint32_t DIR_EXTEN:1;
    uint32_t :27;
};

struct R32_CMP{
    static constexpr uint32_t offset = 0x10;

    uint16_t CMP;
    uint16_t :16;
};

struct R32_ARR{
    static constexpr uint32_t offset = 0x10;

    uint16_t ARR;
    uint16_t :16;
};

struct R32_CNT{
    static constexpr uint32_t offset = 0x10;

    uint16_t CNT;
    uint16_t :16;
};

struct LPTIM_Def{
    volatile R32_ISR ISR;
    volatile R32_ICR ICR;
    volatile R32_IER IER;
    volatile R32_CFGR CFGR;
    volatile R32_CR CR;
    volatile R32_CMP CMP;
    volatile R32_ARR ARR;
    volatile R32_CNT CNT;
};

static constexpr LPTIM_Def * LPTIM1 = reinterpret_cast<LPTIM_Def *>(0x40007C00);

}