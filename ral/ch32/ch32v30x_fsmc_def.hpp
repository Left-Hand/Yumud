#pragma once

#include <cstdint>

namespace ymd::ral::ch32::ch32v30x_fsmc{

struct [[nodiscard]] R32_BCR1 {
    uint32_t MBKEN:1;
    uint32_t MUXEN:1;
    uint32_t MTYP:2;
    uint32_t MWID:2;
    uint32_t FACCEN:1;
    uint32_t :1;
    uint32_t BURSTEN:1;
    uint32_t WAITPOL:1;
    uint32_t WAITCFG:1;
    uint32_t WREN:1;
    uint32_t WAIT_EN:1;
    uint32_t EXTMOD:1;
    uint32_t ASYNCWAIT:1;
    uint32_t :3;
    uint32_t CBURSTRW:1;
    uint32_t :12;
};

struct [[nodiscard]] R32_BTR1 {
    uint32_t ADDSET:4;
    uint32_t ADDHLD:4;
    uint32_t DATAST:8;

    uint32_t BUSTURN:4;
    uint32_t CLKDIV:4;
    uint32_t DATLAT:4;
    uint32_t ACCMOD:2;
    uint32_t:2;
};

struct [[nodiscard]] R32_PCR2{
    uint32_t :1;
    uint32_t PWAITEN:1;
    uint32_t PBKEN:1;
    uint32_t PTYP:1;
    uint32_t PWID:2;
    uint32_t ECCEN:1;
    uint32_t :2;
    uint32_t TCLR:4;
    uint32_t TAR:4;
    uint32_t ECCPS:3;
    uint32_t :13;
};

struct [[nodiscard]] R32_SR2{
    uint32_t :6;
    uint32_t FEMPT:1;
    uint32_t :25;
};

struct [[nodiscard]] R32_PMEM2{
    uint32_t MEMSETx:8;
    uint32_t MEMWAITx:8;
    uint32_t MEMHOLDx:8;
    uint32_t MEMHIZx:8;
};


struct [[nodiscard]] R32_PATT2{
    uint32_t ATTSETx:8;
    uint32_t ATTWAITx:8;
    uint32_t ATTHOLDx:8;
    uint32_t ATTHIZx:8;
};

struct [[nodiscard]] R32_ECCR2{
    uint32_t ECC;
};

struct [[nodiscard]] R32_BWTR1{
    uint32_t ADDSET:4;
    uint32_t ADDHLD:4;
    uint32_t DATAST:8;

    uint32_t BUSTYRN:4;
    uint32_t :2;
    uint32_t ACCMOD:2;
    uint32_t :2;
};

}