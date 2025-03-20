#include "spihw.hpp"
#include "core/system.hpp"
#include "hal/gpio/port.hpp"

using namespace ymd;
using namespace ymd::hal;

void SpiHw::enable_rcc(const bool en){
    switch((uint32_t)instance){
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
    switch((uint32_t)instance){
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
    switch((uint32_t)instance){
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
    switch((uint32_t)instance){
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
    switch((uint32_t)instance){
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
                return Sys::clock::get_apb2_freq();
                break;
            #endif
        
            default:
                __builtin_unreachable();
        }
    }(uint32_t(instance));
	uint32_t real_div = 2;
    uint8_t i = 0;

	while(real_div * baudrate < busFreq){
        real_div <<= 1;
        i++;
    }

    return MIN(i << 3, SPI_BaudRatePrescaler_256);
}

void SpiHw::install_gpios(){
    if(tx_strategy_ != CommStrategy::None){
        Gpio & mosi_pin = get_mosi_gpio();
        mosi_pin.afpp();
    }

    if(rx_strategy_ != CommStrategy::None){
        Gpio & miso_pin = get_miso_gpio();
        miso_pin.inflt();
    }

    {
        Gpio & sclk_pin = get_sclk_gpio();
        sclk_pin.afpp();
    }


    if(false == cs_port.is_index_valid(0)){
        Gpio & cs_pin = get_hw_cs_gpio();
        cs_pin.set();
        if(hw_cs_enabled){
            cs_pin.afpp();
        }else{
            cs_pin.outpp();
        }
        bind_cs_pin(cs_pin, 0);
    }

    // for(auto & cs_gpio : cs_port){
        // if(cs_gpio.isValid()){
        //     cs_gpio.outpp(1);
        // }
    // }

    for(size_t i = 0; i < cs_port.size(); i++){
        cs_port[i].outpp();
    }
}

void SpiHw::enable_hw_cs(const bool en){
    Gpio & _cs_pin = get_hw_cs_gpio();
    _cs_pin.set();

    if(en){
        _cs_pin.afpp();
    }else{
        _cs_pin.outpp();
    }

    hw_cs_enabled = en;
}

void SpiHw::enableRxIt(const bool en){

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
        .SPI_BaudRatePrescaler = calculate_prescaler(baudrate),
        .SPI_FirstBit = SPI_FirstBit_MSB,
        .SPI_CRCPolynomial = 7
    };


	SPI_Init(instance, &SPI_InitStructure);

	SPI_Cmd(instance, ENABLE);

    while (SPI_I2S_GetFlagStatus(instance, SPI_I2S_FLAG_TXE) == RESET);
    instance->DATAR = 0;
    while (SPI_I2S_GetFlagStatus(instance, SPI_I2S_FLAG_RXNE) == RESET);
    instance->DATAR;
}



void SpiHw::set_data_width(const uint8_t bits){
    uint16_t tempreg =  instance->CTLR1;

    switch(bits){
        default:
        case 16:
            if(tempreg & SPI_DataSize_16b) return;
            tempreg |= SPI_DataSize_16b;
            break;
        case 8:
            tempreg &= ~SPI_DataSize_16b;
            break;
    }

    instance->CTLR1 = tempreg;
}

void SpiHw::set_baudrate(const uint32_t baudrate){
    uint32_t tempreg = instance -> CTLR1;
    tempreg &= ~SPI_BaudRatePrescaler_256;
    tempreg |= calculate_prescaler(baudrate);
    instance -> CTLR1 = tempreg;
}

void SpiHw::set_bitorder(const Endian endian){
    uint32_t tempreg = instance -> CTLR1;
    tempreg &= ~SPI_FirstBit_LSB;
    tempreg |= (endian == MSB) ? SPI_FirstBit_MSB : SPI_FirstBit_LSB;
    instance -> CTLR1 = tempreg;
}


BusError SpiHw::write(const uint32_t data){
    // if(tx_strategy_ != CommStrategy::None){
    //     while ((instance->STATR & SPI_I2S_FLAG_TXE) == RESET);
    //     instance->DATAR = data;
    // }

    // if(rx_strategy_ != CommStrategy::None){
    //     while ((instance->STATR & SPI_I2S_FLAG_RXNE) == RESET);
    //     instance->DATAR;
    // }
    // return Error::OK;
    uint32_t dummy;
    return transfer(dummy, data);
}


BusError SpiHw::read(uint32_t & data){
    // if(tx_strategy_ != CommStrategy::None){
    //     while ((instance->STATR & SPI_I2S_FLAG_TXE) == RESET);
    //     instance->DATAR = 0;
    // }

    // if(rx_strategy_ != CommStrategy::None){
    //     while ((instance->STATR & SPI_I2S_FLAG_RXNE) == RESET);
    //     data = instance->DATAR;
    // }
    // return Error::OK;
    return transfer(data, 0);
}


BusError SpiHw::transfer(uint32_t & data_rx, const uint32_t data_tx){
    if(tx_strategy_ != CommStrategy::None){
        while ((instance->STATR & SPI_I2S_FLAG_TXE) == RESET);
        instance->DATAR = data_tx;
    }

    if(rx_strategy_ != CommStrategy::None){
        while ((instance->STATR & SPI_I2S_FLAG_RXNE) == RESET);
        data_rx = instance->DATAR;
    }

    return BusError::OK;
}

namespace ymd::hal{
#ifdef ENABLE_SPI1
SpiHw spi1{SPI1};
#endif

#ifdef ENABLE_SPI2
SpiHw spi2{SPI2};
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