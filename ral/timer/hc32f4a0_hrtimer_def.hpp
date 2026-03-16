#pragma once

#include <cstdint>

namespace ymd::ral::hc32::hc32f4a0{

struct HRPWM_CRn{
    uint32_t NSEL:8;
    uint32_t PSEL:8;
    uint32_t :13;
    uint32_t NE:1;
    uint32_t PE:1;
    uint32_t EN:1;
};

struct HRPWM_CALCRn{
    uint32_t CALCODE:8;
    uint32_t :4;
    uint32_t ENDF:1;
    uint32_t :2;
    uint32_t CALEN:1;
    uint32_t :16;
};

}