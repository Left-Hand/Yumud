#ifndef __BITBAND_H__

#define __BITBAND_H__

#include "sys/platform.h"

typedef struct {
	uint32_t bit0 :1;
	uint32_t bit1 :1;
	uint32_t bit2 :1;
	uint32_t bit3 :1;
	uint32_t bit4 :1;
	uint32_t bit5 :1;
	uint32_t bit6 :1;
	uint32_t bit7 :1;
	uint32_t bit8 :1;
	uint32_t bit9 :1;
	uint32_t bit10 :1;
	uint32_t bit11 :1;
	uint32_t bit12 :1;
	uint32_t bit13 :1;
	uint32_t bit14 :1;
	uint32_t bit15 :1;
	uint32_t bit16 :1;
	uint32_t bit17 :1;
	uint32_t bit18 :1;
	uint32_t bit19 :1;
	uint32_t bit20 :1;
	uint32_t bit21 :1;
	uint32_t bit22 :1;
	uint32_t bit23 :1;
	uint32_t bit24 :1;
	uint32_t bit25 :1;
	uint32_t bit26 :1;
	uint32_t bit27 :1;
	uint32_t bit28 :1;
	uint32_t bit29 :1;
	uint32_t bit30 :1;
	uint32_t bit31 :1;
} GPIO_REG;

#define PAout(n)	(((GPIO_REG *)(&(GPIOA->OUTDR)))->bit##n)
#define PAin(n)		(((GPIO_REG *)(&(GPIOA->INDR)))->bit##n)
#define PBout(n)	(((GPIO_REG *)(&(GPIOB->OUTDR)))->bit##n)
#define PBin(n)		(((GPIO_REG *)(&(GPIOB->INDR)))->bit##n)
#define PCout(n)	(((GPIO_REG *)(&(GPIOC->OUTDR)))->bit##n)
#define PCin(n)		(((GPIO_REG *)(&(GPIOC->INDR)))->bit##n)

#endif