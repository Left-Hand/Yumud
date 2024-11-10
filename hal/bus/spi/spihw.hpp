#pragma once

#include "spi.hpp"


extern"C"{
#ifdef ENABLE_SPI1
__interrupt void SPI1_IRQHandler(void);
#endif

#ifdef ENABLE_SPI2
__interrupt void SPI2_IRQHandler(void);
#endif
}

namespace yumud{

class SpiHw:public Spi{
protected:
    SPI_TypeDef * instance;
    bool hw_cs_enabled = false;

    Gpio & getMosiGpio();
    Gpio & getMisoGpio();
    Gpio & getSclkGpio();
    Gpio & getHwCsGpio();

    void enableRcc(const bool en = true);
    uint16_t calculatePrescaler(const uint32_t baudRate);
    void installGpios();

public:
    DELETE_COPY_AND_MOVE(SpiHw)
    SpiHw(SPI_TypeDef * _instance):instance(_instance){;}

    void init(const uint32_t baudRate, const CommMethod tx_method = CommMethod::Blocking, const CommMethod rx_method = CommMethod::Blocking) override;

    void enableHwCs(const bool en = true);

    void enableRxIt(const bool en = true);

    __fast_inline Error write(const uint32_t data) override;
    __fast_inline Error read(uint32_t & data, bool toAck = true) override;
    __fast_inline Error transfer(uint32_t & data_rx, const uint32_t data_tx, bool toAck = true) override;
    void setDataBits(const uint8_t len) override;
    void setBaudRate(const uint32_t baudRate) override;
    void setBitOrder(const Endian endian) override;

    #ifdef ENABLE_SPI1
    friend void ::SPI1_IRQHandler(void);
    #endif

    #ifdef ENABLE_SPI2
    friend void ::SPI2_IRQHandler(void);
    #endif
};

SpiHw::Error SpiHw::write(const uint32_t data){
    uint32_t dummy = 0;
    transfer(dummy, data);
    return ErrorType::OK;
}
SpiHw::Error SpiHw::read(uint32_t & data, bool toAck){
    transfer(data, 0);
    return ErrorType::OK;
}

SpiHw::Error SpiHw::transfer(uint32_t & data_rx, const uint32_t data_tx, bool toAck){
    if(txMethod != CommMethod::None){
        while ((instance->STATR & SPI_I2S_FLAG_TXE) == RESET);
        instance->DATAR = data_tx;
    }

    if(rxMethod != CommMethod::None){
        while ((instance->STATR & SPI_I2S_FLAG_RXNE) == RESET);
        data_rx = instance->DATAR;
    }

    return Bus::ErrorType::OK;
}

#ifdef ENABLE_SPI1
static inline yumud::SpiHw spi1{SPI1};
#endif

#ifdef ENABLE_SPI2
static inline yumud::SpiHw spi2{SPI2};
#endif

}


