#pragma once

#include "spi.hpp"
#include "core/utils/Option.hpp"

#include "ral/chip.hpp"


extern"C"{
#ifdef ENABLE_SPI1
__interrupt void SPI1_IRQHandler(void);
#endif

#ifdef ENABLE_SPI2
__interrupt void SPI2_IRQHandler(void);
#endif

#ifdef ENABLE_SPI3
__interrupt void SPI3_IRQHandler(void);
#endif
}

namespace ymd::hal{

class Gpio;



struct SpiPrescaler{
    enum class Kind:uint8_t{
        _2 = 0b000,
        _4 = 0b001,
        _8 = 0b010,
        _16 = 0b011,
        _32 = 0b100,
        _64 = 0b101,
        _128 = 0b110,
        _256 = 0b111
    };

    using enum Kind;

    #ifdef __YMD_HAL_CH32_FEATURE_HSSPI
    enum class HsKind{
        // _2 = 0b000,
        _3 = 0b001,
        // _4 = 0b010,
        _5 = 0b011,
        _6 = 0b100,
        _7 = 0b101,
        // _8 = 0b110,
        _9 = 0b111
    };

    using enum HsKind;
    #endif
    constexpr SpiPrescaler(const Kind kind):kind_(kind){;}
    [[nodiscard]] constexpr Kind kind() const {return kind_;}
private:
    Kind kind_;
};

class SpiHw final:public Spi{
public:
    using Callback = std::function<void(SpiEvent)>;

    explicit SpiHw(chip::SPI_Def * inst):inst_(inst){;}

    SpiHw(const SpiHw & other) = delete;
    SpiHw(SpiHw && other) = delete;

    HalResult init(const Config & cfg);

    void enable_hw_cs(const Enable en);

    [[nodiscard]] __fast_inline HalResult fast_write(const uint16_t data){
        while ((inst_->STATR.TXE) == false);
        inst_->DATAR.DR = data;

        return HalResult::Ok();
    }

    [[nodiscard]] __fast_inline HalResult write(const uint32_t data){
        uint32_t dummy;
        return transceive(dummy, data);
    }
    
    
    [[nodiscard]] __fast_inline HalResult read(uint32_t & data){
        return transceive(data, 0);
    }
    
    [[nodiscard]] __fast_inline HalResult transceive(uint32_t & data_rx, const uint32_t data_tx){
        if(tx_strategy_ != CommStrategy::Nil){
            while ((inst_->STATR.TXE) == false);
            inst_->DATAR.DR = data_tx;
        }
    
        if(rx_strategy_ != CommStrategy::Nil){
            while ((inst_->STATR.RXNE) == false);
            data_rx = inst_->DATAR.DR;
        }
    
        return HalResult::Ok();
    }
    [[nodiscard]] HalResult set_data_width(const uint8_t len);
    [[nodiscard]] HalResult set_baudrate(const uint32_t baudrate);
    [[nodiscard]] HalResult set_prescaler(const SpiPrescaler prescaler);
    [[nodiscard]] HalResult set_bitorder(const Endian endian);

    template<typename Fn>
    void set_event_callback(Fn && fn){
        callback_ = std::forward<Fn>(fn);
    }

    [[nodiscard]] bool is_busy(){
        return inst_->STATR.BSY;
    }

    #ifdef ENABLE_SPI1
    friend void ::SPI1_IRQHandler(void);
    #endif

    #ifdef ENABLE_SPI2
    friend void ::SPI2_IRQHandler(void);
    #endif

private:
    chip::SPI_Def * inst_ = nullptr;
    Callback callback_ = nullptr;
    bool hw_cs_enabled_ = false;

    Gpio get_mosi_gpio();
    Gpio get_miso_gpio();
    Gpio get_sclk_gpio();
    Gpio get_hw_cs_gpio();

    uint32_t get_bus_freq() const;

    void enable_rcc(const Enable en);
    void set_remap(const uint8_t remap);
    void plant_gpio();
    
    void enable_rx_it(const Enable en);
    void enable_tx_it(const Enable en);

    void accept_interrupt(const SpiI2sIT);

    #ifdef ENABLE_SPI1
    friend void ::SPI1_IRQHandler(void);
    #endif
    #ifdef ENABLE_SPI2
    friend void ::SPI2_IRQHandler(void);
    #endif
    #ifdef ENABLE_SPI3
    friend void ::SPI3_IRQHandler(void);
    #endif
};


#ifdef ENABLE_SPI1
extern SpiHw spi1;
#endif

#ifdef ENABLE_SPI2
extern SpiHw spi2;
#endif

#ifdef ENABLE_SPI3
extern SpiHw spi3;
#endif

}


