#pragma once

#include <cstdint>

namespace ymd::ral::ch32::ch32x035_wwdg {

struct [[nodiscard]] R32_CTLR {
    uint32_t T:7;
    uint32_t WDGA:1;
    uint32_t :24;
};

struct [[nodiscard]] R32_CFGR{
    uint32_t W:7;
    uint32_t WDGTB:2;
    uint32_t EWI:1;
    uint32_t :22;
};


struct [[nodiscard]] R32_STATR{
    uint32_t EWIF:1;
};

struct [[nodiscard]] IWDG_Def {
    volatile R32_CTLR CTLR;
    volatile R32_CFGR CFGR;
    volatile R32_STATR STATR;
};

}