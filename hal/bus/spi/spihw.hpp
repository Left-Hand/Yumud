#pragma once

#include "spi.hpp"
#include "core/sdk.hpp"


#include "ral/chip.hpp"


extern"C"{
#ifdef ENABLE_SPI1
__interrupt void SPI1_IRQHandler(void);
#endif

#ifdef ENABLE_SPI2
__interrupt void SPI2_IRQHandler(void);
#endif
}

namespace ymd::hal{

class Gpio;

enum class SpiMode:uint8_t{
    _0,
    _1,
    _2,
    _3
};

class SpiHw final:public Spi{
protected:
    // SPI_TypeDef * instance_ = nullptr;
    chip::SPI_Def * instance_ = nullptr;
    bool hw_cs_enabled_ = false;

    Gpio & get_mosi_gpio();
    Gpio & get_miso_gpio();
    Gpio & get_sclk_gpio();
    Gpio & get_hw_cs_gpio();

    void enable_rcc(const bool en = true);
    uint16_t calculate_prescaler(const uint32_t baudrate);
    void install_gpios();

    void enable_rx_it(const bool en = true);
public:
    SpiHw(const SpiHw & other) = delete;
    SpiHw(SpiHw && other) = delete;
    SpiHw(chip::SPI_Def * instance):instance_(instance){;}

    void init(
        const uint32_t baudrate, 
        const CommStrategy tx_strategy = CommStrategy::Blocking, 
        const CommStrategy rx_strategy = CommStrategy::Blocking);

    void enable_hw_cs(const bool en = true);

    [[nodiscard]] hal::BusError write(const uint32_t data);
    [[nodiscard]] hal::BusError read(uint32_t & data);
    [[nodiscard]] hal::BusError transfer(uint32_t & data_rx, const uint32_t data_tx);
    void set_data_width(const uint8_t len);
    void set_baudrate(const uint32_t baudrate);
    void set_bitorder(const Endian endian);

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


