#include "spihw.hpp"
#include "sys/core/system.hpp"

using namespace ymd;
using namespace ymd::hal;

void SpiHw::enableRcc(const bool en){
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

Gpio & SpiHw::getMosiGpio(){
    switch((uint32_t)instance){
        default:
            return GpioNull;
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

Gpio & SpiHw::getMisoGpio(){
    switch((uint32_t)instance){
        default:
            return GpioNull;
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

Gpio & SpiHw::getSclkGpio(){
    switch((uint32_t)instance){
        default:
            return GpioNull;
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

Gpio & SpiHw::getHwCsGpio(){
    switch((uint32_t)instance){
        default:
            return GpioNull;
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

uint16_t SpiHw::calculatePrescaler(const uint32_t baudrate){

	uint32_t busFreq = 0;

    switch((uint32_t)instance){
        #ifdef ENABLE_SPI1
        case SPI1_BASE:
            busFreq = sys::Clock::getAPB1Freq();
            break;
        #endif

        #ifdef ENABLE_SPI2
        case SPI2_BASE:
            busFreq = sys::Clock::getAPB2Freq();
            break;
        #endif

        #ifdef ENABLE_SPI3
        case SPI3_BASE:
            busFreq = Sys::Clock::getAPB2Freq();
            break;
        #endif
    
        default:
            return SPI_BaudRatePrescaler_256;
    }

	uint32_t real_div = 2;
    uint8_t i = 0;

	while(real_div * baudrate < busFreq){
        real_div <<= 1;
        i++;
    }

    return MIN(i << 3, SPI_BaudRatePrescaler_256);
}

void SpiHw::installGpios(){
    if(txMethod != CommMethod::None){
        Gpio & mosi_pin = getMosiGpio();
        mosi_pin.afpp();
    }

    if(rxMethod != CommMethod::None){
        Gpio & miso_pin = getMisoGpio();
        miso_pin.inflt();
    }

    {
        Gpio & sclk_pin = getSclkGpio();
        sclk_pin.afpp();
    }


    if(false == cs_port.isIndexValid(0)){
        Gpio & cs_pin = getHwCsGpio();
        cs_pin.set();
        if(hw_cs_enabled){
            cs_pin.afpp();
        }else{
            cs_pin.outpp();
        }
        bindCsPin(cs_pin, 0);
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

void SpiHw::enableHwCs(const bool en){
    Gpio & _cs_pin = getHwCsGpio();
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
void SpiHw::init(const uint32_t baudrate, const CommMethod tx_method, const CommMethod rx_method){

    txMethod = tx_method;
    rxMethod = rx_method;
	enableRcc();
    installGpios();

    SPI_InitTypeDef SPI_InitStructure = {
        .SPI_Direction = SPI_Direction_2Lines_FullDuplex,
        .SPI_Mode = SPI_Mode_Master,
        .SPI_DataSize = SPI_DataSize_8b,
        .SPI_CPOL = SPI_CPOL_High,
        .SPI_CPHA = SPI_CPHA_2Edge,
        .SPI_NSS = SPI_NSS_Soft,
        .SPI_BaudRatePrescaler = calculatePrescaler(baudrate),
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



void SpiHw::setDataBits(const uint8_t bits){
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

void SpiHw::setBaudRate(const uint32_t baudRate){
    uint32_t tempreg = instance -> CTLR1;
    tempreg &= ~SPI_BaudRatePrescaler_256;
    tempreg |= calculatePrescaler(baudRate);
    instance -> CTLR1 = tempreg;
}

void SpiHw::setBitOrder(const Endian endian){
    uint32_t tempreg = instance -> CTLR1;
    tempreg &= ~SPI_FirstBit_LSB;
    tempreg |= (endian == MSB) ? SPI_FirstBit_MSB : SPI_FirstBit_LSB;
    instance -> CTLR1 = tempreg;
}


SpiHw::Error SpiHw::write(const uint32_t data){
    // if(txMethod != CommMethod::None){
    //     while ((instance->STATR & SPI_I2S_FLAG_TXE) == RESET);
    //     instance->DATAR = data;
    // }

    // if(rxMethod != CommMethod::None){
    //     while ((instance->STATR & SPI_I2S_FLAG_RXNE) == RESET);
    //     instance->DATAR;
    // }
    // return ErrorType::OK;
    uint32_t dummy;
    return transfer(dummy, data);
}


SpiHw::Error SpiHw::read(uint32_t & data, bool toAck){
    // if(txMethod != CommMethod::None){
    //     while ((instance->STATR & SPI_I2S_FLAG_TXE) == RESET);
    //     instance->DATAR = 0;
    // }

    // if(rxMethod != CommMethod::None){
    //     while ((instance->STATR & SPI_I2S_FLAG_RXNE) == RESET);
    //     data = instance->DATAR;
    // }
    // return ErrorType::OK;
    return transfer(data, 0, toAck);
}


SpiHw::Error SpiHw::transfer(uint32_t & data_rx, const uint32_t data_tx, bool toAck){
    if(txMethod != CommMethod::None){
        while ((instance->STATR & SPI_I2S_FLAG_TXE) == RESET);
        instance->DATAR = data_tx;
    }

    if(rxMethod != CommMethod::None){
        while ((instance->STATR & SPI_I2S_FLAG_RXNE) == RESET);
        data_rx = instance->DATAR;
    }

    return Error::OK;
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