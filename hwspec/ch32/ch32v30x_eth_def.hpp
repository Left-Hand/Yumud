#pragma once

#include <cstdint>

namespace hwspec::CH32V30x::ETH_Regs{

struct MACCR_Reg{
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

struct MACFFR_Reg{
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

struct MACMIIAR_Reg{
    uint32_t MB:1;
    uint32_t MW:1;
    uint32_t CR:3;
    uint32_t :3;
    uint32_t MR:5;
    uint32_t PA:5;
    uint32_t :16;
};

struct MACMIIDR_Reg{
    uint32_t MD:16;
    uint32_t :16;
};

struct MACFCR_Reg{
    uint32_t FCB:1;
    uint32_t TFCE:1;
    uint32_t RFCE:1;
    uint32_t UPFD:1;
    uint32_t :12;
    uint32_t PT:16;
};

struct MACVLAN_Reg{
    uint32_t VLANT:17;
    uint32_t :15;
};

struct MACPMTCSR_Reg{
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

struct MACSR_Reg{
    uint32_t :3;
    uint32_t PMTS:1;
    uint32_t MMCS:1;
    uint32_t MMCRS:1;
    uint32_t MMCTS:1;
    uint32_t :2;
    uint32_t TSTS:1;
    uint32_t :22;
};


struct MACIMR_Reg{
    uint32_t :3;
    uint32_t PMTIM:1;
    uint32_t :5;
    uint32_t TSTIM:1;
    uint32_t :22;
};

struct MACA0HR_Reg{
    uint32_t MACAOH:16;
    uint32_t :15;
    uint32_t M0:1;
};

};