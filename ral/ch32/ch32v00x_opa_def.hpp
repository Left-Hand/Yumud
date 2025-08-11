#pragma once

#include <cstdint>

namespace ymd::ral::CH32X035::OPA{

struct CFGR1_Reg{
    uint32_t POLL_EN:1;
    uint32_t :1;
    uint32_t POLL1_NUM:2;
    uint32_t RST_EN1:1;
    uint32_t SETUP_CFG:1;
    uint32_t AUDO_ADC_CFG:1;
    uint32_t IE_OUT1:1;
    uint32_t :1;
    uint32_t NMI_EN:1;
    uint32_t :1;
    uint32_t IF_OUT_POLL_CH1:1;
    uint32_t IF_OUT_POLL_CH2:1;
    uint32_t IF_OUT_POLL_CH3:1;
    uint32_t :1;
    uint32_t POLL_CH1:2;
    uint32_t POLL_CH2:2;
    uint32_t POLL_CH3:2;
    uint32_t :2;
    uint32_t POLL_SWSTRT:1;
    uint32_t POLL_SEL:3;
    uint32_t :3;
    uint32_t POLL_LOCK:1;
};

struct CFGR2_Reg{
    uint32_t OPA_EN1:1;
    uint32_t MODE1:2;
    uint32_t :1;
    uint32_t PSEL1:2;
    uint32_t :2;
    uint32_t NSEL1:2;
    uint32_t FB_EN1:1;
    uint32_t PGADIF:1;
    uint32_t :3;
    uint32_t VBEN:1;
    uint32_t VBSEL:1;
    uint32_t VBCMPSEL:2;
    uint32_t OPA_HS1:1;
    uint32_t :10;
    uint32_t OPA_LOCK:1;
};

struct CTLR1_Reg{
    uint32_t POLL_EN1:1;
    uint32_t :1;
    uint32_t POLL_NUM:1;
    uint32_t RST_EN1:1;
    uint32_t RST_EN2:1;
    uint32_t :2;
    uint32_t IE_OUT1:1;
    uint32_t IE_CNT:1;
    uint32_t :2;

    uint32_t IF_OUT_POLL_CH1:1;
    uint32_t IF_OUT_POLL_CH2:1;
    uint32_t IF_OUT_POLL_CH3:1;
    uint32_t IF_CNT:1;
    uint32_t POLL_VLU:9;

    uint32_t POLL_CH1:2;
    uint32_t POLL_CH2:2;
    uint32_t POLL_CH3:2;
    uint32_t :1;
};

struct CTLR2_Reg{
    uint32_t CMP_EN1:1;
    uint32_t MODE1:2;
    uint32_t NSEL1:2;
    uint32_t PSEL1:2;
    uint32_t HYEN1:2;
    uint32_t RMID1:2;
    uint32_t :6;

    uint32_t CMP_EN2:1;
    uint32_t :7;
    uint32_t FILT_EN:1;
    uint32_t FILT_SEL:1;
    uint32_t BKIN_CFG:2;
    uint32_t BKIN_CFG:3;
    uint32_t CMP_LOCK:1;
};


struct KEY_Reg{
    uint32_t KEY:32;
};

struct CMPKEY_Reg{
    uint32_t KEY:32;
};

struct POLLKEY_Reg{
    uint32_t KEY:32;
};

struct OPA_Def{
    volatile CFGR1_Reg CFGR1;
    volatile CFGR2_Reg CFGR2;
    volatile CTLR1_Reg CTLR1;
    volatile CTLR2_Reg CTLR2;

    volatile KEY_Reg KEY;
    volatile CMPKEY_Reg CMPKEY;
    volatile POLLKEY_Reg POLLKEY;
};

}