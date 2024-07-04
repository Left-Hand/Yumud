#ifndef __DVP_HPP__

#define __DVP_HPP__

#include "sys/platform.h"
#include "hal/gpio/gpio.hpp"
#include "hal/gpio/port.hpp"



class Dvp{
protected:
    void install();
public:
    void init(uint32_t *image0_addr, uint32_t *image1_addr, uint16_t col_len, uint16_t row_len);
};

#ifdef HAVE_DVP

extern "C"{
__interrupt void DVP_IRQHandler(void);
}

extern Dvp dvp;

#endif

#endif