#pragma once

#include "spi.hpp"

class SpiHw:public Spi{
protected:
    SPI_TypeDef * instance;
    bool hw_cs_enabled = false;

    Gpio & getMosiPin();
    Gpio & getMisoPin();
    Gpio & getSclkPin();
    Gpio & getCsPin();

    void enableRcc(const bool en = true);
    uint16_t calculatePrescaler(const uint32_t baudRate);
    void initGpios();

public:
    SpiHw(SPI_TypeDef * _instance):instance(_instance){;}

    void init(const uint32_t baudRate, const CommMethod tx_method = CommMethod::Blocking, const CommMethod rx_method = CommMethod::Blocking);
    void enableHwCs(const bool en = true);

    void enableRxIt(const bool en = true);

    Error write(const uint32_t & data) override;
    Error read(uint32_t & data, bool toAck = true) override;
    Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck = true) override;

    void configDataSize(const uint8_t & data_size) override{
        uint16_t tempreg =  instance->CTLR1;
        if(data_size == 16){
            if(tempreg & SPI_DataSize_16b) return;
            tempreg |= SPI_DataSize_16b;
        }else{
            tempreg &= ~SPI_DataSize_16b;
        }
        instance->CTLR1 = tempreg;
    }

    void configBaudRate(const uint32_t & baudRate) override{
        instance->CTLR1 &= ~SPI_BaudRatePrescaler_256;
        instance->CTLR1 |= calculatePrescaler(baudRate);
    }

    void configBitOrder(const bool & msb) override {
        instance->CTLR1 &= (!SPI_FirstBit_LSB);
        instance->CTLR1 |= msb ? SPI_FirstBit_MSB : SPI_FirstBit_LSB;
    }
};

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