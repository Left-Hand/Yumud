#pragma once

#include <cstdint>
#include "core/io/regs.hpp"


#ifndef ASSERT_REG_IS_32BIT
#define ASSERT_REG_IS_32BIT(name) static_assert((sizeof(name) == 4),  #name " is not 4 bytes"); 
#endif

namespace ymd::ral::CH32{


struct [[nodiscard]] R32_GPHA_CTLR{
    uint32_t START:1;
    uint32_t SUSP:1;
    uint32_t ABORT:1;
    uint32_t :6;
    uint32_t TCIE:1;
    uint32_t TWIE:1;
    uint32_t CAEIE:1;
    uint32_t CTCIE:1;
    uint32_t CEIE:1;
    uint32_t :2;
    uint32_t MODE:3;
    uint32_t :13;
};

struct [[nodiscard]] R32_GPHA_ISR{
    uint32_t :1;
    uint32_t TCIF:1;
    uint32_t TWIF:1;
    uint32_t CAEIF:1;
    uint32_t CTCIF:1;
    uint32_t CEIF:1;
    uint32_t :26;
};


struct [[nodiscard]] R32_GPHA_IFCR{
    uint32_t :1;
    uint32_t CTCIF:1;
    uint32_t CTWIF:1;
    uint32_t CAECIF:1;
    uint32_t CCTCIF:1;
    uint32_t CCEIF:1;
    uint32_t :26;
};

struct [[nodiscard]] R32_GPHA_FGMAR{
    uint32_t MA;
};

struct [[nodiscard]] R32_GPHA_FGOR{
    uint32_t LO:14;
    uint32_t :18;
};

struct [[nodiscard]] R32_GPHA_BGMAR{
    uint32_t MA;
};

struct [[nodiscard]] R32_GPHA_BGOR{
    uint32_t LO:14;
    uint32_t :18;
};


struct [[nodiscard]] R32_GPHA_FGPFCCR{
    uint32_t :4;
    uint32_t CCM:1;
    uint32_t START:1;
    uint32_t :2;
    uint32_t CS:8;
    uint32_t AM:2;
    uint32_t CSS:2;
    uint32_t AI:1;
    uint32_t RBS:1;
    uint32_t :2;
    uint32_t ALPHA:8;
};


struct [[nodiscard]] R32_GPHA_FGCOLR{
    uint32_t BLUE:8;
    uint32_t GREEN:8;
    uint32_t RED:8;
    uint32_t :8;
};

struct [[nodiscard]] R32_GPHA_BGPFCCR{
    uint32_t :4;
    uint32_t CCM:1;
    uint32_t START:1;
    uint32_t :2;
    uint32_t CS:8;
    uint32_t AM:2;
    uint32_t :2;
    uint32_t AI:1;
    uint32_t RBS:1;
    uint32_t :2;
    uint32_t ALPHA:8;
};

struct [[nodiscard]] R32_GPHA_BGCOLR{
    uint32_t BLUE:8;
    uint32_t GREEN:8;
    uint32_t RED:8;
    uint32_t :8;
};

struct [[nodiscard]] R32_GPHA_FGCMAR{
    uint32_t MA;
};

struct [[nodiscard]] R32_GPHA_BGCMAR{
    uint32_t MA;
};

struct [[nodiscard]] R32_GPHA_OPFCCR{
    uint32_t :3;
    uint32_t :17;
    uint32_t AI:1;
    uint32_t RBS:1;
    uint32_t :10;
};


struct [[nodiscard]] R32_GPHA_OCOLR{
    uint32_t BLUE:8;
    uint32_t GREEN:8;
    uint32_t RED:8;
    uint32_t ALPHA:8;
};

struct [[nodiscard]] R32_GPHA_OMAR{
    uint32_t MA;
};

struct [[nodiscard]] R32_GPHA_OOR{
    uint32_t LO:14;
    uint32_t :18;
};

struct [[nodiscard]] R32_GPHA_NLR{
    uint32_t NL:16;
    uint32_t PL:14;
    uint32_t :2;
};

struct [[nodiscard]] R32_GPHA_LWR{
    uint32_t LW:16;
    uint32_t :16;
};


struct [[nodiscard]] R32_GPHA_AMTCR{
    uint32_t EN:1;
    uint32_t :7;
    uint32_t DT:8;
    uint32_t :16;
};

struct [[nodiscard]] R32_GPHA_FGCWRS{
    uint32_t FG_CLUT_INDEX:8;
    uint32_t :1;
    uint32_t :23;
};

struct [[nodiscard]] R32_GPHA_BGCWRS{
    uint32_t BG_CLUT_INDEX:8;
    uint32_t BG_CLUE_EN:1;
    uint32_t :23;
};


struct [[nodiscard]] R32_GPHA_BGCDAT{
    uint32_t DATA;
};


}