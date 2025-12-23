#pragma once

#include "core/platform.hpp"



namespace ymd::hal{
class Dvp{
public:
    struct Config{
        uint32_t image0_addr;
        uint32_t image1_addr; 
        size_t num_col; 
        size_t num_row;
    };

    void init(const Config & cfg);
private:
    void plant();

    void enable_rcc(const Enable en);
};

#ifdef ENABLE_DVP

extern "C"{
__interrupt void DVP_IRQHandler(void);
}

extern Dvp dvp;

#endif
}