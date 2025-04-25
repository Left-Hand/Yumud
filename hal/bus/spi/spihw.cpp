#include "spihw.hpp"
#include "core/system.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;
using namespace ymd::hal;

void SpiHw::enable_rcc(const bool en){
    switch((uint32_t)instance_){
        #ifdef ENABLE_SPI1
        case SPI1_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, en);
            if(SPI1_REMAP){
                GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE);
            }
            break;
        #endif
        #ifdef ENABLE_SPI2
        case SPI2_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, en);
            break;
        #endif
        default:
            break;
    }
}

Gpio & SpiHw::get_mosi_gpio(){
    switch((uint32_t)instance_){
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
    switch((uint32_t)instance_){
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
    switch((uint32_t)instance_){
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
    switch((uint32_t)instance_){
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

uint16_t SpiHw::calculate_prescaler(const uint32_t baudrate){

	uint32_t busFreq = [](const uint32_t base){
        switch(base) {
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
    }(uint32_t(instance_));
	uint32_t real_div = 2;
    uint8_t i = 0;

	while(real_div * baudrate < busFreq){
        real_div <<= 1;
        i++;
    }

    return i & 0b111;
}

void SpiHw::install_gpios(){
    if(bool(tx_strategy_)){
        Gpio & mosi_pin = get_mosi_gpio();
        mosi_pin.afpp();
    }

    if(bool(rx_strategy_)){
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

void SpiHw::enable_hw_cs(const bool en){
    Gpio & cs_gpio = get_hw_cs_gpio();
    cs_gpio.set();

    if(en){
        cs_gpio.afpp();
    }else{
        cs_gpio.outpp();
    }

    instance_->enable_soft_cs(!en);
}

void SpiHw::enable_rx_it(const bool en){

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
        .SPI_BaudRatePrescaler = uint16_t(calculate_prescaler(baudrate) << 3),
        .SPI_FirstBit = SPI_FirstBit_MSB,
        .SPI_CRCPolynomial = 7
    };


	SPI_Init((SPI_TypeDef *)instance_, &SPI_InitStructure);

	// SPI_Cmd((SPI_TypeDef *)instance_, ENABLE);
    instance_->enable_spi(true);

    while ((instance_->STATR.TXE) == RESET);
    instance_->DATAR.DR = 0;

    while ((instance_->STATR.RXNE) == RESET);
    instance_->DATAR.DR;
}



void SpiHw::set_data_width(const uint8_t bits){
    instance_->enable_dualbyte(bits == 16);

}

void SpiHw::set_baudrate(const uint32_t baudrate){
    instance_ -> CTLR1.BR = calculate_prescaler(baudrate);

}

void SpiHw::set_bitorder(const Endian endian){
    instance_ -> CTLR1.LSB = (endian == MSB) ? 0 : 1;
}


hal::BusError SpiHw::write(const uint32_t data){
    uint32_t dummy;
    return transfer(dummy, data);
}


hal::BusError SpiHw::read(uint32_t & data){
    return transfer(data, 0);
}


hal::BusError SpiHw::transfer(uint32_t & data_rx, const uint32_t data_tx){
    if(bool(tx_strategy_)){
        while ((instance_->STATR.TXE) == RESET);
        instance_->DATAR.DR = data_tx;
    }

    if(bool(rx_strategy_)){
        while ((instance_->STATR.RXNE) == RESET);
        data_rx = instance_->DATAR.DR;
    }

    return hal::BusError::Ok();
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