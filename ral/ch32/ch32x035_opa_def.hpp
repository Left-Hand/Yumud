#pragma once

#include <cstdint>

namespace ymd::ral::CH32X035::OPA{

struct [[nodiscard]] R32_CFGR1{
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

struct [[nodiscard]] R32_CFGR2{
    uint16_t POLL_VLU:9;
    uint16_t POLL1_NUM:1;
    uint16_t POLL2_NUM:1;
    uint16_t :3;
};

struct [[nodiscard]] R32_CTLR1{
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

struct [[nodiscard]] R32_CTLR2{
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


struct [[nodiscard]] R32_KEY{
    uint32_t KEY:32;
};

struct [[nodiscard]] R32_CMPKEY{
    uint32_t KEY:32;
};

struct [[nodiscard]] R32_POLLKEY{
    uint32_t KEY:32;
};

struct [[nodiscard]] OPA_Def{
    volatile R32_CFGR1 CFGR1;
    volatile R32_CFGR2 CFGR2;
    volatile R32_CTLR1 CTLR1;
    volatile R32_CTLR2 CTLR2;

    volatile R32_KEY KEY;
    volatile R32_CMPKEY CMPKEY;
    volatile R32_POLLKEY POLLKEY;
};

}