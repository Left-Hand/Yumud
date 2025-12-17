#include "spihw.hpp"
#include "core/sdk.hpp"
#include "core/system.hpp"
#include "core/debug/debug.hpp"

#include "hal/gpio/gpio_port.hpp"

#include "spi_layout.hpp"

using namespace ymd;
using namespace ymd::hal;


static constexpr SpiPrescaler calculate_prescaler(
    const uint32_t aligned_bus_clk_freq, 
    const uint32_t baudrate
){
    uint32_t real_div = 2;
    uint8_t i = 0;

    while(real_div * baudrate < aligned_bus_clk_freq){
        real_div <<= 1;
        i++;
    }

    return SpiPrescaler(std::bit_cast<SpiPrescaler::Kind>(static_cast<uint8_t>(i & 0b111)));
}

[[maybe_unused]] static Nth _spi_to_nth(const void * inst){
    switch(reinterpret_cast<size_t>(inst)){
        #ifdef SPI1_PRESENT
        case SPI1_BASE:
            return Nth(1);
        #endif
        #ifdef SPI2_PRESENT
        case SPI2_BASE:
            return Nth(2);
        #endif
        #ifdef SPI3_PRESENT
        case SPI3_BASE:
            return Nth(3);
        #endif
    }
    __builtin_trap();
}

template<SpiRemap REMAP>
[[maybe_unused]] static Gpio _spi_to_miso_gpio(const void * inst){
    const auto nth = _spi_to_nth(inst);
    switch(nth.count()){
        #ifdef SPI1_PRESENT
        case 1:
            return pintag_to_pin<spi::miso_pin_t<1, REMAP>>();
        #endif
        #ifdef SPI2_PRESENT
        case 2:
            return pintag_to_pin<spi::miso_pin_t<2, REMAP>>();
        #endif
        #ifdef SPI3_PRESENT
        case 3:
            return pintag_to_pin<spi::miso_pin_t<3, REMAP>>();
        #endif
    }
    __builtin_trap();
}

template<SpiRemap REMAP>
[[maybe_unused]] static Gpio _spi_to_mosi_gpio(const void * inst){
    const auto nth = _spi_to_nth(inst);
    switch(nth.count()){
        #ifdef SPI1_PRESENT
        case 1:
            return pintag_to_pin<spi::mosi_pin_t<1, REMAP>>();
        #endif
        #ifdef SPI2_PRESENT
        case 2:
            return pintag_to_pin<spi::mosi_pin_t<2, REMAP>>();
        #endif
        #ifdef SPI3_PRESENT
        case 3:
            return pintag_to_pin<spi::mosi_pin_t<3, REMAP>>();
        #endif
    }
    __builtin_trap();
}

template<SpiRemap REMAP>
[[maybe_unused]] static Gpio _spi_to_sclk_gpio(const void * inst){
    const auto nth = _spi_to_nth(inst);
    switch(nth.count()){
        #ifdef SPI1_PRESENT
        case 1:
            return pintag_to_pin<spi::sclk_pin_t<1, REMAP>>();
        #endif
        #ifdef SPI2_PRESENT
        case 2:
            return pintag_to_pin<spi::sclk_pin_t<2, REMAP>>();
        #endif
        #ifdef SPI3_PRESENT
        case 3:
            return pintag_to_pin<spi::sclk_pin_t<3, REMAP>>();
        #endif
    }
    __builtin_trap();
}

template<SpiRemap REMAP>
[[maybe_unused]] static Gpio _spi_to_hwcs_gpio(const void * inst){
    const auto nth = _spi_to_nth(inst);
    switch(nth.count()){
        #ifdef SPI1_PRESENT
        case 1:
            return pintag_to_pin<spi::hwcs_pin_t<1, REMAP>>();
        #endif
        #ifdef SPI2_PRESENT
        case 2:
            return pintag_to_pin<spi::hwcs_pin_t<2, REMAP>>();
        #endif
        #ifdef SPI3_PRESENT
        case 3:
            return pintag_to_pin<spi::hwcs_pin_t<3, REMAP>>();
        #endif
    }
    __builtin_trap();
}


#define DEF_SPI_BIND_PIN_LAYOUTER(name)\
[[maybe_unused]] static Gpio spi_to_##name##_gpio(const void * inst, const SpiRemap remap){\
    switch(remap){\
        case SpiRemap::_0: return _spi_to_##name##_gpio<SpiRemap::_0>(inst);\
        case SpiRemap::_1: return _spi_to_##name##_gpio<SpiRemap::_1>(inst);\
        case SpiRemap::_2: return _spi_to_##name##_gpio<SpiRemap::_2>(inst);\
        case SpiRemap::_3: return _spi_to_##name##_gpio<SpiRemap::_3>(inst);\
    }\
    __builtin_trap();\
}\

DEF_SPI_BIND_PIN_LAYOUTER(miso)
DEF_SPI_BIND_PIN_LAYOUTER(mosi)
DEF_SPI_BIND_PIN_LAYOUTER(sclk)
DEF_SPI_BIND_PIN_LAYOUTER(hwcs)

void SpiHw::enable_rcc(const Enable en){
    switch(reinterpret_cast<size_t>(inst_)){
        #ifdef SPI1_PRESENT
        case SPI1_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, en == EN);
            return;
        #endif
        #ifdef SPI2_PRESENT
        case SPI2_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, en == EN);
            return;
        #endif
        #ifdef SPI3_PRESENT
        case SPI3_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, en == EN);
            return;
        #endif
    }
    __builtin_trap();
}

void SpiHw::set_remap(const SpiRemap remap){
    switch(reinterpret_cast<size_t>(inst_)){
        #ifdef SPI1_PRESENT
        case SPI1_BASE:
            switch(remap){
                case SpiRemap::_0: return GPIO_PinRemapConfig(GPIO_Remap_SPI1, DISABLE);
                case SpiRemap::_1: return GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE);
                default: break;
            }
            break;
        #endif
        #ifdef SPI2_PRESENT
        case SPI2_BASE:
            switch(remap){
                case SpiRemap::_0: return; //SPI2 NO REMAP
                default: break;
            }
            break;
        #endif
        #ifdef SPI3_PRESENT
        case SPI3_BASE:
            //TODO
            switch(remap){
                default: break;
            }
            break;
        #endif
    }
    __builtin_trap();
}




void SpiHw::alter_to_pins(const SpiRemap remap){
    spi_to_mosi_gpio(inst_, remap).afpp();

    spi_to_miso_gpio(inst_, remap).inflt();

    spi_to_sclk_gpio(inst_, remap).afpp();
    
    if(hw_cs_enabled_){
        spi_to_hwcs_gpio(inst_, remap).afpp();
    }
}

void SpiHw::enable_hw_cs(const Enable en){
    #if 0
    auto && cs_gpio = spi_to_hwcs_gpio(inst_, remap);
    cs_gpio.set_high();

    if(en == EN){
        cs_gpio.afpp();
    }else{
        cs_gpio.outpp();
    }

    inst_->enable_soft_cs(!en);
    #else
    __builtin_trap();
    #endif
}

uint32_t SpiHw::get_periph_clk_freq() const {
    switch(reinterpret_cast<size_t>(inst_)) {
        #ifdef SPI1_PRESENT
        case SPI1_BASE:
            return sys::clock::get_apb1_clk_freq();
            break;
        #endif

        #ifdef SPI2_PRESENT
        case SPI2_BASE:
            return sys::clock::get_apb2_clk_freq();
            break;
        #endif

        #ifdef SPI3_PRESENT
        case SPI3_BASE:
            return sys::clock::get_apb2_freq();
            break;
        #endif
    }
    __builtin_trap();
}

HalResult SpiHw::init(const SpiConfig & cfg){
	enable_rcc(EN);
    const auto remap = cfg.remap;

    set_remap(remap);
    alter_to_pins(remap);


    #if 1
    const SPI_InitTypeDef SPI_InitStructure = {
        .SPI_Direction = SPI_Direction_2Lines_FullDuplex,
        .SPI_Mode = SPI_Mode_Master,
        .SPI_DataSize = SPI_DataSize_8b,
        .SPI_CPOL = SPI_CPOL_High,
        .SPI_CPHA = SPI_CPHA_2Edge,
        .SPI_NSS = SPI_NSS_Soft,
        .SPI_BaudRatePrescaler = 0,
        .SPI_FirstBit = SPI_FirstBit_MSB,
        .SPI_CRCPolynomial = 7
    };
	SPI_Init(reinterpret_cast<SPI_TypeDef *>(inst_), &SPI_InitStructure);

    #endif


    #if 0
    inst_->enable_dualbyte(DISEN);
    inst_ -> set_bitorder(MSB);
    inst_->enable_soft_cs(EN);
    inst_->set_cpol(true);
    inst_->set_cpha(true);
    inst_->enable_bidi(DISEN);
    inst_->enable_i2s(DISEN);
    set_baudrate(cfg.baudrate);
    inst_->CRCR.CRCPOLY = 7;
    reinterpret_cast<SPI_TypeDef *>(inst_)->I2SCFGR &= ((uint16_t)0xF7FF);
    #endif

    if(const auto res = set_baudrate(cfg.baudrate);
        res.is_err()) return res;

    inst_->enable_spi(EN);
    while ((inst_->STATR.TXE) == RESET);
    inst_->DATAR.DR = 0;

    while ((inst_->STATR.RXNE) == RESET);
    inst_->DATAR.DR;

    return HalResult::Ok();
}


HalResult SpiHw::set_wordsize(const SpiWordSize wordsize){
    switch(wordsize){
        case SpiWordSize::OneByte:
            inst_->enable_dualbyte(DISEN);
            return HalResult::Ok();
            break;
        case SpiWordSize::TwoBytes:
            inst_->enable_dualbyte(DISEN);
            return HalResult::Ok();
            break;
    }
    __builtin_trap();
}



HalResult SpiHw::set_baudrate(const SpiBaudrate baud){
    if(baud.is<SpiPrescaler>()){
        return set_prescaler(baud.unwrap_as<SpiPrescaler>());
    }else if(baud.is<LeastFreq>()){
        return set_prescaler(
            calculate_prescaler(get_periph_clk_freq(), 
            baud.unwrap_as<LeastFreq>().count
        ));
    }else if(baud.is<NearestFreq>()){
        return set_prescaler(
            calculate_prescaler(get_periph_clk_freq(), 
            baud.unwrap_as<NearestFreq>().count
        ));
    }

    //should not reach here
    __builtin_trap();
}

HalResult SpiHw::set_prescaler(const SpiPrescaler prescaler){
    inst_ -> CTLR1.BR = std::bit_cast<uint8_t>(prescaler.kind());
    return HalResult::Ok();
}


HalResult SpiHw::set_bitorder(const BitOrder bitorder){
    inst_->set_bitorder(bitorder);
    return HalResult::Ok();
}

void SpiHw::accept_interrupt(const SpiI2sIT it){

}

void SpiHw::deinit(){
	enable_rcc(DISEN);
}

namespace ymd::hal{
#ifdef SPI1_PRESENT
SpiHw spi1{ral::SPI1_Inst};
#endif

#ifdef SPI2_PRESENT
SpiHw spi2{ral::SPI2_Inst};
#endif

#ifdef SPI3_PRESENT
SpiHw spi3{ral::SPI3_Inst};
#endif
}

#ifdef SPI1_PRESENT
void SPI1_IRQHandler(void){
    
}

#endif

#ifdef SPI2_PRESENT
void SPI2_IRQHandler(void){
    
}
#endif

#ifdef SPI3_PRESENT
void SPI3_IRQHandler(void){
    
}
#endif