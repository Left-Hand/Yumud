#pragma once

#include <cstdint>
#include "core/io/regs.hpp"


namespace ymd::ral::CH32V203{

struct R32_OPA_CTLR{
    uint32_t EN1:1;
    uint32_t MODE1:1;
    uint32_t NSEL1:1;
    uint32_t PSEL1:1;

    uint32_t EN2:1;
    uint32_t MODE2:1;
    uint32_t NSEL2:1;
    uint32_t PSEL2:1;

    uint32_t EN3:1;
    uint32_t MODE3:1;
    uint32_t NSEL3:1;
    uint32_t PSEL3:1;

    uint32_t EN4:1;
    uint32_t MODE4:1;
    uint32_t NSEL4:1;
    uint32_t PSEL4:1;

    uint32_t :16;
};

struct OPA_Def{
    volatile R32_OPA_CTLR CTLR;

    constexpr void enable(const uint8_t nth, const Enable en){
        switch(nth){
            case 1: this->CTLR.EN1 = en == EN; return;
            case 2: this->CTLR.EN2 = en == EN; return;
            case 3: this->CTLR.EN3 = en == EN; return;
            case 4: this->CTLR.EN4 = en == EN; return;
        }
    }
    constexpr void select_out(const uint8_t nth, const uint8_t sel){
        switch(nth){
            case 1: this->CTLR.MODE1 = sel; return;
            case 2: this->CTLR.MODE2 = sel; return;
            case 3: this->CTLR.MODE3 = sel; return;
            case 4: this->CTLR.MODE4 = sel; return;
        }
    }

    constexpr void select_pos(const uint8_t nth, const uint8_t sel){
        switch(nth){
            case 1: this->CTLR.PSEL1 = sel; return;
            case 2: this->CTLR.PSEL2 = sel; return;
            case 3: this->CTLR.PSEL3 = sel; return;
            case 4: this->CTLR.PSEL4 = sel; return;
        }
    }

    constexpr void select_neg(const uint8_t nth, const uint8_t sel){
        switch(nth){
            case 1: this->CTLR.NSEL1 = sel; return;
            case 2: this->CTLR.NSEL2 = sel; return;
            case 3: this->CTLR.NSEL3 = sel; return;
            case 4: this->CTLR.NSEL4 = sel; return;
        }
    }
};

[[maybe_unused]] static inline OPA_Def * OPA_Inst = reinterpret_cast<OPA_Def *>(0x40023804);

};

namespace ymd::chip{
    using OPA_Def = ral::CH32V203::OPA_Def;
    using ral::CH32V203::OPA_Inst;
}