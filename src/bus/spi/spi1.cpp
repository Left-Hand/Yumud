#include "spi1.hpp"

uint16_t SPI1_Prescaler_Calculate(uint32_t baudRate){
	RCC_ClocksTypeDef RCC_CLK;
    RCC_GetClocksFreq(&RCC_CLK);

	uint32_t busFreq = RCC_CLK.PCLK2_Frequency;
	uint32_t exp_div = busFreq / baudRate;

	uint32_t real_div = 2;
    uint8_t i = 0;
	while(real_div < exp_div){
        real_div <<= 1;
        i++;
    }

    return MIN(i * 8, SPI_BaudRatePrescaler_256);
}

void SPI1_GPIO_Init(void){
    CHECK_INIT

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure = {0};

	GPIO_InitStructure.GPIO_Pin = SPI1_SCLK_Pin | SPI1_MOSI_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI1_Port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = SPI1_MISO_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI1_Port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SPI1_CS_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SPI1_CS_Port, &GPIO_InitStructure);

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    GPIO_PinRemapConfig(SPI1_REMAP, SPI1_REMAP_ENABLE);
}

void SPI1_Init(uint32_t baudRate){
    CHECK_INIT


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE );

    SPI_InitTypeDef SPI_InitStructure = {0};

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI1_Prescaler_Calculate(baudRate);
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);

	SPI_Cmd(SPI1, ENABLE);

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI1->DATAR = 0;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    SPI1->DATAR;
}


void Spi1::init(const uint32_t & baudRate){
    SPI1_GPIO_Init();
    SPI1_Init(baudRate);
}

void Spi1::configDataSize(const uint8_t & data_size){
    SPI_DataSizeConfig(SPI1, data_size == 16 ? SPI_DataSize_16b : SPI_DataSize_8b);
}

void Spi1::configBaudRate(const uint32_t & baudRate){
    SPI1->CTLR1 &= ~SPI_BaudRatePrescaler_256;
    SPI1->CTLR1 |= SPI1_Prescaler_Calculate(baudRate);
}

void Spi1::configBitOrder(const bool & msb){
    SPI1->CTLR1 &= (!SPI_FirstBit_LSB);
    SPI1->CTLR1 |= msb ? SPI_FirstBit_MSB : SPI_FirstBit_LSB;
}

#ifndef HAVE_SPI1
#define HAVE_SPI1
Spi1 spi1;
#endif

int8_t Spi1::occupied = -1;