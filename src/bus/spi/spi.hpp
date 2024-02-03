#ifndef __SPI_HPP__

#define __SPI_HPP__

#include "../serbus.hpp"
#include "../../defines/comm_inc.h"
#include "../../clock/clock.h"

class Spi:public SerBus{
public:
    // Error write(void * _data_ptr, const size_t & len) override;
    // Error read(void * _data_ptr, const size_t & len) override;
    // Error transfer(void * _data_rx_ptr, void * _data_tx_ptr, const size_t & len) override;
};


#endif