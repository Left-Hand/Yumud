#pragma once

#include <cstdint>

namespace ymd::ral::ch32::ch32v30x_sdio{

struct R32_POWER{
    uint32_t PWRCTRL:2;
    uint32_t :30;
};

struct R32_CLKCR{
    uint32_t CLKDIV:8;
    uint32_t CLKEN:1;
    uint32_t PWRSAV:1;
    uint32_t BYPASS:1;
    uint32_t WIDBUS:2;
    uint32_t NEGEDGE:1;
    uint32_t HWFCEN:1;
    uint32_t :17;
};

struct R32_ARG{
    uint32_t CMDARG;
}; 

struct R32_CMD{
    uint32_t CMDINDEX:6;
    uint32_t WAITRESP:2;
    uint32_t WAITINT:1;
    uint32_t WAITPEND:1;
    uint32_t CPSMEN:1;
    uint32_t SDIOSuspend:1;
    uint32_t ENCMDComplete:1;
    uint32_t NIEN:1;
    uint32_t ARACMD:1;
    uint32_t :17;
};

struct R32_RESPCMD{
    uint32_t RESPCMD:6;
    uint32_t :26;
};


}