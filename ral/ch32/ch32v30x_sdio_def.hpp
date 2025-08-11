#pragma once

#include <cstdint>

namespace ymd::ral::CH32V30x::SDIO_Regs{

struct POWER_Reg{
    uint32_t PWRCTRL:2;
    uint32_t :30;
};

struct CLKCR_Reg{
    uint32_t CLKDIV:8;
    uint32_t CLKEN:1;
    uint32_t PWRSAV:1;
    uint32_t BYPASS:1;
    uint32_t WIDBUS:2;
    uint32_t NEGEDGE:1;
    uint32_t HWFCEN:1;
    uint32_t :17;
};

struct ARG_Reg{
    uint32_t CMDARG;
}; 

struct CMD_Reg{
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

struct RESPCMD_Reg{
    uint32_t RESPCMD:6;
    uint32_t :26;
};


}