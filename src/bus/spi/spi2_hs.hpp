#ifndef __SPI2_HS_HPP__

#define __SPI2_HS_HPP__

#include "../bus.hpp"
#include "../../defines/comm_inc.h"
#include "../../clock/clock.h"
#include "spi2.hpp"

#define CS_LOW (SPI2_CS_Port->BCR = SPI2_CS_Pin)
#define CS_HIGH (SPI2_CS_Port->BSHR = SPI2_CS_Pin)

class Spi2_hs:public Spi2{
public:
    Spi2_hs():Spi2(){;}

    __fast_inline void begin() override {
        __nopn(6);
        CS_LOW;
        __nopn(6);
    }

    __fast_inline void end() override {
        __nopn(6);
        CS_HIGH;
        __nopn(6);
    }
    __fast_inline bool busy() override {return (SPI2->STATR & SPI_I2S_FLAG_BSY);}

    __fast_inline Error write(const uint32_t & data) override;

    __fast_inline Error read(uint32_t & data, bool toAck = true) override{return ErrorType::OK;}
    Error read(void * _data_ptr, const size_t & len) override {return ErrorType::OK;}

    __fast_inline Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck = true) override {return ErrorType::OK;}
    Error transfer(void * _data_rx_ptr, void * _data_tx_ptr, const size_t & len) override {return ErrorType::OK;}
};

__fast_inline Bus::Error Spi2_hs::write(const uint32_t & data){
    while ((SPI2->STATR & SPI_I2S_FLAG_TXE) == RESET);
    SPI2->DATAR = data;
    // __nopn(2); 
    return Bus::ErrorType::OK;
}

uint16_t SPI2_Prescale_Caculate(uint32_t baudRate);

#undef CS_LOW
#undef CS_HIGH

extern Spi2_hs spi2_hs;

#endif