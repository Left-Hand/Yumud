#pragma once

#include <cstdint>


namespace ymd::ral::ch32::ch32v30x_eth{ 
struct [[nodiscard]] R32_ETH_MACCR{
    uint32_t :1;
    uint32_t TCF:1;
    uint32_t RE:1;
    uint32_t TE:1;
    uint32_t :3;
    uint32_t APCS:1;
    uint32_t :2;
    uint32_t IPCO:1;
    uint32_t DM:1;
    uint32_t LM:1;
    uint32_t :1;
    uint32_t FES:2;
    uint32_t :1;
    uint32_t IFG:3;
    uint32_t PR:1;
    uint32_t PI:1;
    uint32_t JD:1;
    uint32_t WD:1;
    uint32_t :5;
    uint32_t TCD:3;
};

struct [[nodiscard]] R32_ETH_MACFFR{
    uint32_t PM:1;
    uint32_t HU:1;
    uint32_t HM:1;
    uint32_t DAIF:1;
    uint32_t PAM:1;
    uint32_t BFD:1;
    uint32_t PCF:1;
    uint32_t SAIF:1;
    uint32_t SAIF:1;
    uint32_t SAF:1;
    uint32_t HPF:1;
    uint32_t :20;
    uint32_t RA:1;
};

using R32_ETH_MACHTHR = uint32_t;
using R32_ETH_MACHTLR = uint32_t;

struct [[nodiscard]] R32_ETH_MACMIIAR{
    uint32_t MB:1;
    uint32_t MW:1;
    uint32_t CR:3;
    uint32_t :3;
    uint32_t MR:5;
    uint32_t PA:5;
    uint32_t :16;
};

struct [[nodiscard]] R32_ETH_MACMIIDR{
    uint32_t MD:16;
    uint32_t :16;
};

struct [[nodiscard]] R32_ETH_MACFCR{
    uint32_t FCB:1;
    uint32_t TFCE:1;
    uint32_t RFCE:1;
    uint32_t UPFD:1;
    uint32_t :12;
    uint32_t PT:16;
};

struct [[nodiscard]] R32_ETH_MACVLAN{
    uint32_t VLANT:17;
    uint32_t :15;
};

struct [[nodiscard]] R32_ETH_MACPMTCSR{
    uint32_t PD:1;
    uint32_t MPE:1;
    uint32_t WFE:1;
    uint32_t :2;
    uint32_t MPR:1;
    uint32_t WFR:1;
    uint32_t :2;
    uint32_t GU:1;
    uint32_t :21;
    uint32_t WFFRP:1;
};

struct [[nodiscard]] R32_ETH_MACSR{
    uint32_t :3;
    uint32_t PMTS:1;
    uint32_t MMCS:1;
    uint32_t MMCRS:1;
    uint32_t MMCTS:1;
    uint32_t :2;
    uint32_t TSTS:1;
    uint32_t :22;
};


struct [[nodiscard]] R32_ETH_MACIMR{
    uint32_t :3;
    uint32_t PMTIM:1;
    uint32_t :5;
    uint32_t TSTIM:1;
    uint32_t :22;
};

struct [[nodiscard]] R32_ETH_MACA0HR{
    uint32_t MACAOH:16;
    uint32_t :15;
    uint32_t M0:1;
};

struct [[nodiscard]] ETH_Def{
    volatile R32_ETH_MACCR MACCR;
    volatile R32_ETH_MACFFR MACFFR;
    volatile R32_ETH_MACHTHR MACHTHR;
    volatile R32_ETH_MACHTLR MACHTLR;
};

}