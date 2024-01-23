#ifndef __I2C_DRIVER_H__

#define __I2C_DRIVER_H__

#include "../src/defines/comm_inc.h"
#include "ch32v20x_i2c.h"
#include "clock/clock.h"

#ifdef __cplusplus
extern"C"{
#endif

#define I2C_MEMADD_SIZE_8BIT 1
#define I2C_MEMADD_SIZE_16BIT 2

void I2C2_Init(uint8_t address, uint16_t bound);
int I2C_Mem_Read(I2C_TypeDef * I2Cx, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
int I2C_Mem_Write(I2C_TypeDef* I2Cx, uint8_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t* pData, uint16_t Size, uint32_t Timeout);

#ifdef __cplusplus
}
#endif

#endif