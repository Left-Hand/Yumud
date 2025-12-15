#pragma once

#include "spi.hpp"
#include "core/utils/Option.hpp"

#include "ral/spi.hpp"
#include "spi_layout.hpp"


extern"C"{
#ifdef SPI1_PRESENT
__interrupt void SPI1_IRQHandler(void);
#endif

#ifdef SPI2_PRESENT
__interrupt void SPI2_IRQHandler(void);
#endif

#ifdef SPI3_PRESENT
__interrupt void SPI3_IRQHandler(void);
#endif
}

namespace ymd::hal{

class Gpio;




class SpiHw final:public Spi{
public:
    using Callback = std::function<void(SpiEvent)>;

    explicit SpiHw(ral::SPI_Def * inst):
        inst_(inst){;}

    SpiHw(const SpiHw & other) = delete;
    SpiHw(SpiHw && other) = delete;

    HalResult init(const SpiConfig & cfg);
    void deinit();

    void enable_hw_cs(const Enable en);

    [[nodiscard]] __fast_inline HalResult fast_blocking_write(const uint16_t data){
        while ((inst_->STATR.TXE) == false);
        inst_->DATAR.DR = data;

        return HalResult::Ok();
    }

    [[nodiscard]] __fast_inline HalResult blocking_write(const uint32_t data){
        uint32_t dummy;
        return blocking_transceive(dummy, data);
    }
    
    [[nodiscard]] __fast_inline HalResult blocking_read(uint32_t & data){
        return blocking_transceive(data, 0);
    }
    
    [[nodiscard]] __fast_inline HalResult blocking_transceive(uint32_t & data_rx, const uint32_t data_tx){
        while ((inst_->STATR.TXE) == false);
        inst_->DATAR.DR = data_tx;

        while ((inst_->STATR.RXNE) == false);
        data_rx = inst_->DATAR.DR;

    
        return HalResult::Ok();
    }

    [[nodiscard]] HalResult set_word_width(const uint8_t len);
    [[nodiscard]] HalResult set_baudrate(const SpiBaudrate baud);
    [[nodiscard]] HalResult set_prescaler(const SpiPrescaler prescaler);
    [[nodiscard]] HalResult set_bitorder(const BitOrder bitorder);

    template<typename Fn>
    void set_event_handler(Fn && fn){
        callback_ = std::forward<Fn>(fn);
    }

    [[nodiscard]] bool is_busy(){
        return inst_->STATR.BSY;
    }

private:
    ral::SPI_Def * inst_ = nullptr;
    Callback callback_ = nullptr;
    bool hw_cs_enabled_ = false;

    uint32_t get_periph_clk_freq() const;

    void enable_rcc(const Enable en);
    void set_remap(const SpiRemap remap);
    void alter_to_pins(const SpiRemap remap);
    
    void enable_rx_it(const Enable en);
    void enable_tx_it(const Enable en);

    void accept_interrupt(const SpiI2sIT it);

    #ifdef SPI1_PRESENT
    friend void ::SPI1_IRQHandler(void);
    #endif
    #ifdef SPI2_PRESENT
    friend void ::SPI2_IRQHandler(void);
    #endif
    #ifdef SPI3_PRESENT
    friend void ::SPI3_IRQHandler(void);
    #endif
};


#ifdef SPI1_PRESENT
extern SpiHw spi1;
#endif

#ifdef SPI2_PRESENT
extern SpiHw spi2;
#endif

#ifdef SPI3_PRESENT
extern SpiHw spi3;
#endif

}


