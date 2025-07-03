#pragma once

#include "spi.hpp"
#include "core/sdk.hpp"
#include "core/utils/Option.hpp"

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



class SpiHw final:public Spi{
public:
    explicit SpiHw(chip::SPI_Def * instance):inst_(instance){;}

    SpiHw(const SpiHw & other) = delete;
    SpiHw(SpiHw && other) = delete;

    void init(const Config & cfg);

    void enable_hw_cs(const Enable en = EN);

    [[nodiscard]] __fast_inline hal::HalResult fast_write(const uint16_t data){
        while ((inst_->STATR.TXE) == RESET);
        inst_->DATAR.DR = data;

        return hal::HalResult::Ok();
    }

    [[nodiscard]] __fast_inline hal::HalResult write(const uint32_t data){
        uint32_t dummy;
        return transceive(dummy, data);
    }
    
    
    [[nodiscard]] __fast_inline hal::HalResult read(uint32_t & data){
        return transceive(data, 0);
    }
    
    [[nodiscard]] __fast_inline hal::HalResult transceive(uint32_t & data_rx, const uint32_t data_tx){
        if(bool(tx_strategy_)){
            while ((inst_->STATR.TXE) == RESET);
            inst_->DATAR.DR = data_tx;
        }
    
        if(bool(rx_strategy_)){
            while ((inst_->STATR.RXNE) == RESET);
            data_rx = inst_->DATAR.DR;
        }
    
        return hal::HalResult::Ok();
    }
    [[nodiscard]] hal::HalResult set_data_width(const uint8_t len);
    [[nodiscard]] hal::HalResult set_baudrate(const uint32_t baudrate);
    [[nodiscard]] hal::HalResult set_bitorder(const Endian endian);

    #ifdef ENABLE_SPI1
    friend void ::SPI1_IRQHandler(void);
    #endif

    #ifdef ENABLE_SPI2
    friend void ::SPI2_IRQHandler(void);
    #endif

private:
    chip::SPI_Def * inst_ = nullptr;
    bool hw_cs_enabled_ = false;

    Gpio & get_mosi_gpio();
    Gpio & get_miso_gpio();
    Gpio & get_sclk_gpio();
    Gpio & get_hw_cs_gpio();

    uint32_t get_bus_freq() const;

    void enable_rcc(const Enable en = EN);
    void install_gpios();
    
    void enable_rx_it(const Enable en = EN);

    static constexpr uint8_t calculate_prescaler(
            const uint32_t bus_freq, const uint32_t baudrate){
        uint32_t real_div = 2;
        uint8_t i = 0;

        while(real_div * baudrate < bus_freq){
            real_div <<= 1;
            i++;
        }

        return i & 0b111;

    }
};


#ifdef ENABLE_SPI1
extern SpiHw spi1;
#endif

#ifdef ENABLE_SPI2
extern SpiHw spi2;
#endif

}


