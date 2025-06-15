#include "spihw.hpp"

#include "core/system.hpp"
#include "core/debug/debug.hpp"

#include "hal/gpio/gpio_port.hpp"

using namespace ymd;
using namespace ymd::hal;

void SpiHw::enable_rcc(const Enable en){
    switch(reinterpret_cast<uint32_t>(inst_)){
        #ifdef ENABLE_SPI1
        case SPI1_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, en == EN);
            if(SPI1_REMAP){
                GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE);
            }
            break;
        #endif
        #ifdef ENABLE_SPI2
        case SPI2_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, en == EN);
            break;
        #endif
        default:
            __builtin_unreachable();
    }
}

Gpio & SpiHw::get_mosi_gpio(){
    switch(reinterpret_cast<uint32_t>(inst_)){
        default:
            return NullGpio;
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

Gpio & SpiHw::get_miso_gpio(){
    switch(reinterpret_cast<uint32_t>(inst_)){
        default:
            return NullGpio;
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

Gpio & SpiHw::get_sclk_gpio(){
    switch(reinterpret_cast<uint32_t>(inst_)){
        default:
            return NullGpio;
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

Gpio & SpiHw::get_hw_cs_gpio(){
    switch(reinterpret_cast<uint32_t>(inst_)){
        default:
            return NullGpio;
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


void SpiHw::install_gpios(){
    if(tx_strategy_ != CommStrategy::Nil){
        Gpio & mosi_pin = get_mosi_gpio();
        mosi_pin.afpp();
    }

    if(rx_strategy_ != CommStrategy::Nil){
        Gpio & miso_pin = get_miso_gpio();
        miso_pin.inflt();
    }

    {
        Gpio & sclk_pin = get_sclk_gpio();
        sclk_pin.afpp();
    }


    if(false == cs_port_.is_index_valid(0)){
        Gpio & cs_pin = get_hw_cs_gpio();
        cs_pin.set();
        if(hw_cs_enabled_){
            cs_pin.afpp();
        }else{
            cs_pin.outpp();
        }
        bind_cs_pin(cs_pin, 0);
    }

    for(size_t i = 0; i < cs_port_.size(); i++){
        if(cs_port_.is_index_valid(i)){
            cs_port_[i].outpp();
        }
    }
}

void SpiHw::enable_hw_cs(const Enable en){
    Gpio & cs_gpio = get_hw_cs_gpio();
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
    switch(reinterpret_cast<uint32_t>(inst_)) {
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

void SpiHw::init(const uint32_t baudrate, const CommStrategy tx_strategy, const CommStrategy rx_strategy){

    tx_strategy_ = tx_strategy;
    rx_strategy_ = rx_strategy;
	enable_rcc();
    install_gpios();



    const SPI_InitTypeDef SPI_InitStructure = {
        .SPI_Direction = SPI_Direction_2Lines_FullDuplex,
        .SPI_Mode = SPI_Mode_Master,
        .SPI_DataSize = SPI_DataSize_8b,
        .SPI_CPOL = SPI_CPOL_High,
        .SPI_CPHA = SPI_CPHA_2Edge,
        .SPI_NSS = SPI_NSS_Soft,
        .SPI_BaudRatePrescaler = uint16_t(calculate_prescaler(
            get_bus_freq(), baudrate) << 3),
        .SPI_FirstBit = SPI_FirstBit_MSB,
        .SPI_CRCPolynomial = 7
    };

	SPI_Init((SPI_TypeDef *)inst_, &SPI_InitStructure);

    inst_->enable_spi(EN);

    while ((inst_->STATR.TXE) == RESET);
    inst_->DATAR.DR = 0;

    while ((inst_->STATR.RXNE) == RESET);
    inst_->DATAR.DR;
}



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