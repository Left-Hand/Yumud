#pragma once

#include "spi.hpp"

class SpiHw:public Spi{
protected:
    SPI_TypeDef * instance;
    bool hw_cs_enabled = false;

    Gpio & getMosiGpio();
    Gpio & getMisoGpio();
    Gpio & getSclkGpio();
    Gpio & getCsGpio();

    void enableRcc(const bool en = true);
    uint16_t calculatePrescaler(const uint32_t baudRate);
    void installGpios();

public:
    SpiHw(SPI_TypeDef * _instance):instance(_instance){;}

    void init(const uint32_t baudRate, const CommMethod tx_method = CommMethod::Blocking, const CommMethod rx_method = CommMethod::Blocking) override;

    void enableHwCs(const bool en = true);

    void enableRxIt(const bool en = true);

    __fast_inline Error write(const uint32_t data) override;
    __fast_inline Error read(uint32_t & data, bool toAck = true) override;
    __fast_inline Error transfer(uint32_t & data_rx, const uint32_t data_tx, bool toAck = true) override;
    void setDataBits(const uint8_t data_size) override;
    void setBaudRate(const uint32_t baudRate) override;
    void setBitOrder(const Endian endian) override ;
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


#ifdef HAVE_SPI1
extern SpiHw spi1;
#endif

#ifdef HAVE_SPI2
extern SpiHw spi2;
#endif

extern"C"{
#ifdef HAVE_SPI1
__interrupt void SPI1_IRQHandler(void);
#endif

#ifdef HAVE_SPI2
__interrupt void SPI2_IRQHandler(void);
#endif

}