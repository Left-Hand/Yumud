#include "SPI1_Driver.h"

static volatile uint16_t ConstData;

void SPI1_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    //PB3 SCLK
    //PB5 MOSI
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE );	

    SPI_InitTypeDef SPI_InitStructure = {0};

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI1_Prescaler;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
 
	SPI_Cmd(SPI1, ENABLE);
	
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, 0);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    // SPI_I2S_ReceiveData(SPI1);
}   

void DMA1_CH3_Init(void * data, void * reg){
    CHECK_INIT

 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	DMA_InitTypeDef DMA_InitStructure = {0};
    DMA_DeInit(DMA1_Channel3);
	
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)reg;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)data;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);

    DMA_ClearFlag(DMA1_FLAG_TC3);
}

void DMA1_CH3_DataSizeConfig(uint16_t size){
    uint16_t tempreg = DMA1_Channel3->CFGR;
    tempreg &= !(DMA_MemoryDataSize_HalfWord | DMA_MemoryDataSize_Word);
    tempreg |= size;
    DMA1_Channel3->CFGR = tempreg;
}

void DMA1_CH3_DataLengthConfig(uint32_t length){
    DMA1_Channel3->CNTR = length;
}

void DMA1_CH3_SourceConfig(void * source){
    DMA1_Channel3->MADDR = (uint32_t)source;
}

void DMA1_CH3_IncConfig(FunctionalState inc){
    if(inc) 
        DMA1_Channel3->CFGR |= DMA_MemoryInc_Enable;
    else 
        DMA1_Channel3->CFGR &= (!DMA_MemoryInc_Enable);
}

void SPI1_DMA_Start(){
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
    DMA_Cmd(DMA1_Channel3, ENABLE);
}

void SPI1_DMA_Stop(){
    DMA_Cmd(DMA1_Channel3, DISABLE);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);
}

void SPI1_Write_8b(uint8_t dataTx)
{		
    while ((SPI1->STATR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);
    SPI1->DATAR = dataTx;	    
}

void SPI1_Write_16b(uint16_t dataTx)
{		
    while ((SPI1->STATR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);
    SPI1->DATAR = dataTx;	    
}

void SPI1_Write_Const_16b(uint16_t data, uint32_t length){

    while ((SPI1->STATR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);	
    __nopn(8);
    SPI_DataSizeConfig(SPI1, SPI_DataSize_16b);

    #ifdef SPI1_USE_DMA

    ConstData = data;

    DMA1_CH3_DataSizeConfig(DMA_MemoryDataSize_HalfWord);
    DMA1_CH3_DataLengthConfig(length);
    DMA1_CH3_SourceConfig((void *)(&ConstData));
    DMA1_CH3_IncConfig(DISABLE);

    SPI1_DMA_Start();
    while(DMA_GetFlagStatus(DMA1_FLAG_TC3) != SET);
    DMA_ClearFlag(DMA1_FLAG_TC3);
    SPI1_DMA_Stop();

    #else

    for(uint32_t i = 0; i < length;i++){
        SPI1_Write_16b(data);
    }
    
    #endif

    while ((SPI1->STATR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);
    __nopn(8);
    SPI_DataSizeConfig(SPI1, SPI_DataSize_8b);
}

void SPI1_Write_Pool_16b(uint16_t * data, uint32_t length)
{
    while ((SPI1->STATR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);	
    __nopn(8);
    SPI_DataSizeConfig(SPI1, SPI_DataSize_16b);

    #ifdef SPI1_USE_DMA

    DMA1_CH3_DataSizeConfig(DMA_MemoryDataSize_HalfWord);
    DMA1_CH3_DataLengthConfig(length);
    DMA1_CH3_SourceConfig((void *)data);
    DMA1_CH3_IncConfig(ENABLE);

    SPI1_DMA_Start();
    while(DMA_GetFlagStatus(DMA1_FLAG_TC3) != SET);
    DMA_ClearFlag(DMA1_FLAG_TC3);
    // SPI1_DMA_Stop();

    #else

    for(uint32_t i = 0; i < length;i++){
        SPI1_Write_16b(data[i]);
    }
    
    #endif

    while ((SPI1->STATR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);
    __nopn(8);
    SPI_DataSizeConfig(SPI1, SPI_DataSize_8b);
}
