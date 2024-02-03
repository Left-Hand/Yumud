#include "spi2.hpp"

uint16_t SPI2_Prescale_Caculate(uint32_t baudRate){
	RCC_ClocksTypeDef RCC_CLK;
    RCC_GetClocksFreq(&RCC_CLK);

	uint32_t busFreq = RCC_CLK.PCLK1_Frequency;
	uint32_t exp_div = busFreq / baudRate;
	
	uint32_t real_div = 2;
    uint8_t i = 0;
	while(real_div < exp_div){
        real_div <<= 1;
        i++;
    }

    return MIN(i * 8, SPI_BaudRatePrescaler_256);
}

void SPI2_GPIO_Init(void){
    CHECK_INIT

    RCC_APB2PeriphClockCmd(SPI2_CS_Periph, ENABLE);
    RCC_APB2PeriphClockCmd(SPI2_GPIO_Periph, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.GPIO_Pin = SPI2_MOSI_Pin | SPI2_SCK_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SPI2_Port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SPI2_CS_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(SPI2_CS_Port, &GPIO_InitStructure);

    SPI2_CS_Port->BSHR = SPI2_CS_Pin;

    GPIO_InitStructure.GPIO_Pin = SPI2_MISO_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(SPI2_Port, &GPIO_InitStructure);
}

void SPI2_Init(uint32_t baudRate){
    CHECK_INIT

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);	

    SPI_InitTypeDef SPI_InitStructure = {0};

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);
 
	SPI_Cmd(SPI2, ENABLE);
	
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
    SPI2->DATAR = 0;
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
    SPI2->DATAR;
}   


void Spi2::init(const uint32_t & baudRate){
    SPI2_GPIO_Init();
    SPI2_Init(baudRate);
}

void Spi2::configDataSize(const uint8_t & data_size){
    this->data_size = data_size;
    SPI_DataSizeConfig(SPI2, data_size == 16 ? SPI_DataSize_16b : SPI_DataSize_8b);
}

void Spi2::configBaudRate(const uint32_t & baudRate){
    SPI2->CTLR1 &= ~SPI_BaudRatePrescaler_256;
    SPI2->CTLR1 |= SPI2_Prescale_Caculate(baudRate);
}

void Spi2::configBitOrder(const bool & msb){
    SPI2->CTLR1 &= (!SPI_FirstBit_LSB);
    SPI2->CTLR1 |= msb ? SPI_FirstBit_MSB : SPI_FirstBit_LSB;
}

Bus::Error Spi2::write(void * _data_ptr, const size_t & len){
    if(data_size == 8){
        uint8_t * data_ptr = (uint8_t *)(_data_ptr);
        for(size_t i = 0; i < len; i++) write(data_ptr[i]);
    }else{
        uint16_t * data_ptr = (uint16_t *)(_data_ptr);
        for(size_t i = 0; i < len; i++) write(data_ptr[i]);
    }
    return Bus::ErrorType::OK;
}

Bus::Error Spi2::transfer(void * _data_rx_ptr, void * _data_tx_ptr, const size_t & len){
    if(data_size == 8){
        uint8_t * data_rx_ptr = (uint8_t *)(_data_rx_ptr);
        uint8_t * data_tx_ptr = (uint8_t *)(_data_tx_ptr);
        uint32_t data_rx = 0;
        for(size_t i = 0; i < len; i++){
            transfer(data_rx, data_tx_ptr[i]);
            data_rx_ptr[i] = data_rx;
        }
    }else{
        uint16_t * data_rx_ptr = (uint16_t *)(_data_rx_ptr);
        uint16_t * data_tx_ptr = (uint16_t *)(_data_tx_ptr);
        uint32_t data_rx = 0;
        for(size_t i = 0; i < len; i++){
            transfer(data_rx, data_tx_ptr[i]);
            data_rx_ptr[i] = data_rx;
        }
    }
    return Bus::ErrorType::OK;
}

Bus::Error Spi2::read(void * _data_ptr, const size_t & len){
    if(data_size == 8){
        uint8_t * data_ptr = (uint8_t *)(_data_ptr);
        uint32_t data_rx = 0;
        for(size_t i = 0; i < len; i++){
            transfer(data_rx, 0);
            data_ptr[i] = data_rx;
        }
    }else{
        uint16_t * data_ptr = (uint16_t *)(_data_ptr);
        uint32_t data_rx = 0;
        for(size_t i = 0; i < len; i++){
            transfer(data_rx, 0);
            data_ptr[i] = data_rx;
        }
    }
    return Bus::ErrorType::OK;
}

#ifndef HAVE_SPI2
#define HAVE_SPI2
Spi2 spi2;
#endif

int8_t Spi2::occupied = false; 