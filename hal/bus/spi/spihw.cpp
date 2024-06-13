#include "spihw.hpp"

void SpiHw::enableRcc(const bool en){
    switch((uint32_t)instance){
        #ifdef HAVE_SPI1
        case SPI1_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, en);
            GPIO_PinRemapConfig(SPI1_REMAP, SPI1_REMAP_ENABLE);
            break;
        #endif
        #ifdef HAVE_SPI2
        case SPI2_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, en);
            break;
        #endif
        default:
            break;
    }
}


#define SPI_HW_GET_PIN_TEMPLATE(name, upper)\
Gpio & SpiHw::get##name##Pin(){\
\
    Port * gpio_port = nullptr;\
    uint16_t gpio_pin = 0;\
\
    switch((uint32_t)instance){\
        default:\
        case SPI1_BASE:\
            gpio_port = &SPI1_##upper##_Port;\
            gpio_pin = SPI1_##upper##_Pin;\
            break;\
        case SPI2_BASE:\
            gpio_port = &SPI2_##upper##_Port;\
            gpio_pin = SPI2_##upper##_Pin;\
            break;\
    }\
    return (*gpio_port)[(Pin)gpio_pin];\
}\

SPI_HW_GET_PIN_TEMPLATE(Mosi, MOSI)
SPI_HW_GET_PIN_TEMPLATE(Miso, MISO)
SPI_HW_GET_PIN_TEMPLATE(Sclk, SCLK)
SPI_HW_GET_PIN_TEMPLATE(Cs, CS)

#undef SPI_HW_GET_PIN_TEMPLATE

uint16_t SpiHw::calculatePrescaler(const uint32_t baudRate){
	RCC_ClocksTypeDef RCC_CLK;
    RCC_GetClocksFreq(&RCC_CLK);

	uint32_t busFreq = 0;

    switch((uint32_t)instance){
        #ifdef HAVE_SPI1
        case SPI1_BASE:
            busFreq = RCC_CLK.PCLK1_Frequency;
            break;
        #endif

        #ifdef HAVE_SPI2
        case SPI2_BASE:
            busFreq = RCC_CLK.PCLK2_Frequency;
            break;
        #endif
        default:
            return SPI_BaudRatePrescaler_256;
    }

	uint32_t exp_div = busFreq / baudRate;

	uint32_t real_div = 2;
    uint8_t i = 0;
	while(real_div < exp_div){
        real_div <<= 1;
        i++;
    }

    return MIN(i * 8, SPI_BaudRatePrescaler_256);
}

void SpiHw::initGpios(){
    if(((uint8_t)mode & (uint8_t)Mode::TxOnly)){
        Gpio & mosi_pin = getMosiPin();
        mosi_pin.OutAfPP();
    }

    if(((uint8_t)mode & (uint8_t)Mode::RxOnly)){
        Gpio & miso_pin = getMisoPin();
        miso_pin.InFloating();
    }

    Gpio & sclk_pin = getSclkPin();
    sclk_pin.OutAfPP();

    if(!cs_pins.isIndexValid(0)){
        Gpio & cs_pin = getCsPin();
        cs_pin = true;
        if(hw_cs_enabled){
            cs_pin.OutAfPP();
        }else{
            cs_pin.OutPP();
        }
        bindCsPin(cs_pin, 0);
    }

    for(uint8_t i = 0; i < cs_pins.getSize(); i++){
        cs_pins.setModeByIndex(i, PinMode::OutPP);
    }

    switch((uint32_t)instance){
        #ifdef HAVE_SPI1
        case SPI1_BASE:
            GPIO_PinRemapConfig(SPI1_REMAP, SPI1_REMAP_ENABLE);
            break;
        #endif
        default:
            break;
    }
}

void SpiHw::enableHwCs(const bool en){
    Gpio & _cs_pin = getCsPin();
    _cs_pin = true;

    if(en){
        _cs_pin.OutAfPP();
    }else{
        _cs_pin.OutPP();
    }

    hw_cs_enabled = en;
}

void SpiHw::enableRxIt(const bool en){

}
void SpiHw::init(const uint32_t baudrate, const Mode & mode){
    preinit();

	enableRcc();
    initGpios();

    SPI_InitTypeDef SPI_InitStructure = {0};

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = calculatePrescaler(baudrate);
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(instance, &SPI_InitStructure);

	SPI_Cmd(instance, ENABLE);

    while (SPI_I2S_GetFlagStatus(instance, SPI_I2S_FLAG_TXE) == RESET);
    instance->DATAR = 0;
    while (SPI_I2S_GetFlagStatus(instance, SPI_I2S_FLAG_RXNE) == RESET);
    instance->DATAR;
}

SpiHw::Error SpiHw::write(const uint32_t & data){
    uint32_t dummy = 0;
    transfer(dummy, data);
    return ErrorType::OK;
}
SpiHw::Error SpiHw::read(uint32_t & data, bool toAck){
    transfer(data, 0);
    return ErrorType::OK;
}
SpiHw::Error SpiHw::transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck){
    if(mode != Mode::RxOnly){
        while ((instance->STATR & SPI_I2S_FLAG_TXE) == RESET);
        instance->DATAR = data_tx;
    }
    if(mode != Mode::TxOnly){
        while ((instance->STATR & SPI_I2S_FLAG_RXNE) == RESET);
        data_rx = instance->DATAR;
    }

    return Bus::ErrorType::OK;
}

#ifdef HAVE_SPI1
SpiHw spi1{SPI1};
#endif

#ifdef HAVE_SPI2
SpiHw spi2{SPI2};
#endif
