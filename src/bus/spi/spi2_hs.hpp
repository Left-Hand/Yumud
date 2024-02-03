#ifndef __SPI2_HS_HPP__

#define __SPI2_HS_HPP__

#include "../bus.hpp"
#include "../../defines/comm_inc.h"
#include "../../clock/clock.h"
#include "spi2.hpp"

class Spi2_hs:public Spi2{
public:
    Spi2_hs():Spi2(){;}

    __fast_inline Error write(const uint32_t & data) override;

    __fast_inline Error read(uint32_t & data, bool toAck = true) override{return ErrorType::OK;}
    // Error read(void * _data_ptr, const size_t & len) override {return ErrorType::OK;}

    __fast_inline Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck = true) override {return ErrorType::OK;}
    // Error transfer(void * _data_rx_ptr, void * _data_tx_ptr, const size_t & len) override {return ErrorType::OK;}
};

__fast_inline Bus::Error Spi2_hs::write(const uint32_t & data){
    while ((SPI2->STATR & SPI_I2S_FLAG_TXE) == RESET);
    SPI2->DATAR = data;
    return Bus::ErrorType::OK;
}

extern Spi2_hs spi2_hs;

#endif