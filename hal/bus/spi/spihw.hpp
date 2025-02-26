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

namespace ymd::hal{

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

    void init(
        const uint32_t baudRate, 
        const CommMethod tx_method = CommMethod::Blocking, 
        const CommMethod rx_method = CommMethod::Blocking) override;

    void enableHwCs(const bool en = true);

    void enableRxIt(const bool en = true);

    BusError write(const uint32_t data) override;
    BusError read(uint32_t & data) override;
    BusError transfer(uint32_t & data_rx, const uint32_t data_tx) override;
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



#ifdef ENABLE_SPI1
extern SpiHw spi1;
#endif

#ifdef ENABLE_SPI2
extern SpiHw spi2;
#endif

}


