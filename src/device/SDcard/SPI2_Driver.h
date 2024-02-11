#ifndef __SPI2_DRIVER_H__

#define __SPI2_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "stdint.h"
#include "src/comm_inc.h"

#include "ch32v20x_spi.h"
#include "ch32v20x_gpio.h"

void SPI2_Init();

#ifdef __cplusplus
}
#endif 
#endif