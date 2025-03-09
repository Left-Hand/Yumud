#pragma once

#include <cstdint>

namespace hwspec::CH32X035::OPA{

struct CFGR1_Reg{
    uint16_t POLL_EN1:1;
    uint16_t POLL_EN2:1;
    uint16_t BKIN_EN1:1;
    uint16_t BKIN_EN2:1;

    uint16_t RST_EN1:1;
    uint16_t RST_EN2:1;

    uint16_t BKIN_SEL:1;
    uint16_t POLL_LOCK:1;
    uint16_t IE_OUT1:1;
    uint16_t IE_OUT2:1;
    uint16_t IE_CNT:1;
    uint16_t NMI_EN:1;
    uint16_t IF_OUT1:1;
    uint16_t IF_OUT2:1;
    uint16_t IF_CNT:1;
    uint16_t :1;
};

struct CFGR2_Reg{
    uint16_t POLL_VLU:9;
    uint16_t POLL1_NUM:1;
    uint16_t POLL2_NUM:1;
    uint16_t :3;
};

struct CTLR1_Reg{
    uint32_t  EN1:1;
    uint32_t  MODE1:1;
    uint32_t  :1;
    uint32_t  PSEL1:2;
    uint32_t  FB_EN1:1;
    uint32_t  NSEL1:2;
    uint32_t  :7;

    uint32_t  EN2:1;
    uint32_t  MODE2:1;
    uint32_t  :1;
    uint32_t  PSEL2:2;
    uint32_t  FB_EN2:1;
    uint32_t  NSEL2:2;

    uint32_t :6;
    uint32_t OPA_LOCK:1;
};

struct CTLR2_Reg{
    uint32_t EN1:1;
    uint32_t MODE1:1;
    uint32_t NSEL1:1;
    uint32_t PSEL1:1;
    uint32_t HYEN1:1;

    uint32_t EN2:1;
    uint32_t MODE2:1;
    uint32_t NSEL2:1;
    uint32_t PSEL2:1;
    uint32_t HYEN2:1;

    uint32_t EN3:1;
    uint32_t MODE3:1;
    uint32_t NSEL3:1;
    uint32_t PSEL3:1;
    uint32_t HYEN3:1;
    
    uint32_t :16;
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