#pragma once

#include "core/platform.hpp"



namespace ymd::hal{
class Dvp{
protected:
    void install();
public:
    void init(uint32_t *image0_addr, uint32_t *image1_addr, uint16_t col_len, uint16_t row_len);
};

#ifdef ENABLE_DVP

extern "C"{
__interrupt void DVP_IRQHandler(void);
}

extern Dvp dvp;

#endif
}