#include "spihw.hpp"

#include "core/system.hpp"
#include "core/debug/debug.hpp"

#include "hal/gpio/gpio_port.hpp"

using namespace ymd;
using namespace ymd::hal;

static Gpio map_inst_to_mosi_gpio(const void * inst){
    switch(reinterpret_cast<size_t>(inst)){
        default:
            __builtin_unreachable();
        #ifdef ENABLE_SPI1
        case SPI1_BASE:
            return SPI1_MOSI_GPIO;
        #endif

        #ifdef ENABLE_SPI2
        case SPI2_BASE:
            return SPI2_MOSI_GPIO;
        #endif

        #ifdef ENABLE_SPI3
        case SPI3_BASE:
            return SPI3_MOSI_GPIO;
        #endif
    }
}

static Gpio map_inst_to_miso_gpio(const void * inst){
    switch(reinterpret_cast<size_t>(inst)){
        default:
            __builtin_unreachable();
        #ifdef ENABLE_SPI1
        case SPI1_BASE:
            return SPI1_MISO_GPIO;
        #endif

        #ifdef ENABLE_SPI2
        case SPI2_BASE:
            return SPI2_MISO_GPIO;
        #endif

        #ifdef ENABLE_SPI3
        case SPI3_BASE:
            return SPI3_MISO_GPIO;
        #endif
    }
}

static Gpio map_inst_to_sclk_gpio(const void * inst){
    switch(reinterpret_cast<size_t>(inst)){
        default:
            __builtin_unreachable();
        #ifdef ENABLE_SPI1
        case SPI1_BASE:
            return SPI1_SCLK_GPIO;
        #endif

        #ifdef ENABLE_SPI2
        case SPI2_BASE:
            return SPI2_SCLK_GPIO;
        #endif

        #ifdef ENABLE_SPI3
        case SPI3_BASE:
            return SPI3_SCLK_GPIO;
        #endif
    }
}

static Gpio map_inst_to_hw_cs_gpio(const void * inst){
    switch(reinterpret_cast<size_t>(inst)){
        default:
            __builtin_unreachable();
        #ifdef ENABLE_SPI1
        case SPI1_BASE:
            return SPI1_CS_GPIO;
        #endif

        #ifdef ENABLE_SPI2
        case SPI2_BASE:
            return SPI2_CS_GPIO;
        #endif

        #ifdef ENABLE_SPI3
        case SPI3_BASE:
            return SPI3_CS_GPIO;
        #endif
    }
}

void SpiHw::enable_rcc(const Enable en){
    switch(reinterpret_cast<size_t>(inst_)){
        default:
            __builtin_unreachable();
        #ifdef ENABLE_SPI1
        case SPI1_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, en == EN);
            break;
        #endif
        #ifdef ENABLE_SPI2
        case SPI2_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, en == EN);
            break;
        #endif
    }
}

void SpiHw::set_remap(const uint8_t remap){
    switch(reinterpret_cast<size_t>(inst_)){
        default:
            __builtin_unreachable();
        #ifdef ENABLE_SPI1
        case SPI1_BASE:
            if(SPI1_REMAP){
                GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE);
            }
            break;
        #endif
        #ifdef ENABLE_SPI2
        case SPI2_BASE:
            //SPI2 NO REMAP
            break;
        #endif
    }
}


static constexpr uint8_t get_default_remap(const void * inst_){
    switch(reinterpret_cast<size_t>(inst_)){
        default:
            __builtin_unreachable();
        #ifdef ENABLE_SPI1
        case SPI1_BASE:
            return SPI1_REMAP;
        #endif
        #ifdef ENABLE_SPI2
        case SPI2_BASE:
            //SPI2 NO REMAP
            return 0;
        #endif
    }
}


Gpio SpiHw::get_mosi_gpio(){
    return map_inst_to_mosi_gpio(inst_);
}
Gpio SpiHw::get_miso_gpio(){
    return map_inst_to_miso_gpio(inst_);
}
Gpio SpiHw::get_sclk_gpio(){
    return map_inst_to_sclk_gpio(inst_);
}
Gpio SpiHw::get_hw_cs_gpio(){
    return map_inst_to_hw_cs_gpio(inst_);
}

void SpiHw::plant_gpios(){
    if(tx_strategy_ != CommStrategy::Nil){
        get_mosi_gpio().afpp();
    }

    if(rx_strategy_ != CommStrategy::Nil){
        get_miso_gpio().inflt();
    }

    {
        get_sclk_gpio().afpp();
    }
}

void SpiHw::enable_hw_cs(const Enable en){
    auto cs_gpio = get_hw_cs_gpio();
    cs_gpio.set();

    if(en == EN){
        cs_gpio.afpp();
    }else{
        cs_gpio.outpp();
    }

    inst_->enable_soft_cs(!en);
}

void SpiHw::enable_rx_it(const Enable en){

}

uint32_t SpiHw::get_bus_freq() const {
    switch(reinterpret_cast<size_t>(inst_)) {
        #ifdef ENABLE_SPI1
        case SPI1_BASE:
            return sys::clock::get_apb1_freq();
            break;
        #endif

        #ifdef ENABLE_SPI2
        case SPI2_BASE:
            return sys::clock::get_apb2_freq();
            break;
        #endif

        #ifdef ENABLE_SPI3
        case SPI3_BASE:
            return sys::clock::get_apb2_freq();
            break;
        #endif
    
        default:
            __builtin_unreachable();
    }
}

void SpiHw::init(const Config & cfg){

    tx_strategy_ = cfg.tx_strategy;
    rx_strategy_ = cfg.rx_strategy;
	enable_rcc(EN);
    set_remap(get_default_remap(inst_));
    plant_gpios();

    const SPI_InitTypeDef SPI_InitStructure = {
        .SPI_Direction = SPI_Direction_2Lines_FullDuplex,
        .SPI_Mode = SPI_Mode_Master,
        .SPI_DataSize = SPI_DataSize_8b,
        .SPI_CPOL = SPI_CPOL_High,
        .SPI_CPHA = SPI_CPHA_2Edge,
        .SPI_NSS = SPI_NSS_Soft,
        .SPI_BaudRatePrescaler = uint16_t(calculate_prescaler(
            get_bus_freq(), cfg.baudrate) << 3),
        .SPI_FirstBit = SPI_FirstBit_MSB,
        .SPI_CRCPolynomial = 7
    };

	SPI_Init(reinterpret_cast<SPI_TypeDef *>(inst_), &SPI_InitStructure);
    // inst_->set_pre
    inst_->enable_spi(EN);


    while ((inst_->STATR.TXE) == RESET);
    inst_->DATAR.DR = 0;

    while ((inst_->STATR.RXNE) == RESET);
    inst_->DATAR.DR;
}





// static constexpr auto BIDIMODE_MASK = DEF_REG_BF_MASK(chip::R32_SPI_CTLR1, BIDIMODE);
// constexpr uint32_t mask = []{
//     // auto field_reg = std::bit_cast<chip::R32_SPI_CTLR1>(uint32_t(0));
//     auto field_reg = chip::R32_SPI_CTLR1{0};
//     field_reg.BIDIMODE = 1u;
//     return std::bit_cast<uint32_t>(field_reg);
// }();
hal::HalResult SpiHw::set_data_width(const uint8_t bits){
    inst_->enable_dualbyte((bits == 16) ? EN : DISEN);
    return hal::HalResult::Ok();
}

hal::HalResult SpiHw::set_baudrate(const uint32_t baudrate){
    inst_ -> CTLR1.BR = calculate_prescaler(get_bus_freq(), baudrate);
    return hal::HalResult::Ok();
}

hal::HalResult SpiHw::set_bitorder(const Endian endian){
    inst_ -> CTLR1.LSB = (endian == MSB) ? 0 : 1;
    return hal::HalResult::Ok();
}

namespace ymd::hal{
#ifdef ENABLE_SPI1
SpiHw spi1{chip::SPI1_Inst};
#endif

#ifdef ENABLE_SPI2
SpiHw spi2{chip::SPI2_Inst};
#endif
}

#ifdef ENABLE_SPI1
void SPI1_IRQHandler(void){

}

#endif

#ifdef ENABLE_SPI2
void SPI2_IRQHandler(void){
    
}
#endif