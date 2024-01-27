#ifndef __SPI1_DRIVER_H__

#define __SPI1_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "stdint.h"
#include "../src/defines/comm_inc.h"

#include "ch32v20x_spi.h"
#include "ch32v20x_gpio.h"

void SPI1_Init(void);
void SPI1_Write_8b(uint8_t dataTx);
void SPI1_Write_16b(uint16_t dataTx);

void DMA1_CH3_Init(void * data, void * reg);
void DMA1_CH3_DataSizeConfig(uint16_t size);
void DMA1_CH3_DataLengthConfig(uint32_t length);
void DMA1_CH3_IncConfig(FunctionalState inc);
void DMA1_CH3_SourceConfig(void * source);

void SPI1_DMA_Start();
void SPI1_DMA_Stop();

void SPI1_Write_Const_16b(uint16_t data, uint32_t length);
void SPI1_Write_Pool_16b(uint16_t * data, uint32_t length);

#ifdef __cplusplus
}
#endif 

#endif