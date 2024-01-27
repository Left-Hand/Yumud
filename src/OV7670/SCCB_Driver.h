// #ifndef __SCCB_H
// #define __SCCB_H

// #ifdef __cplusplus
// extern "C" {
// #endif 

// #include "stdint.h"
// #include "../src/defines/comm_inc.h"

// #include "ch32v20x_gpio.h"

// #define BITBAND(addr, bitnum) ((addr & 0xF0000000) + 0x20000000 + \
//                               ((addr & 0xFFFFF) << 5) + (bitnum << 2))

// #define MEM_ADDR(addr) *((volatile unsigned long *)(addr))
// #define BIT_ADDR(addr, bitnum) MEM_ADDR(BITBAND(addr, bitnum))

// #define GPIOA_ODR_Addr (&(GPIOA->OUTDR))
// #define GPIOB_ODR_Addr (&(GPIOB->OUTDR))

// #define PAout(n) BIT_ADDR(GPIOA_ODR_Addr, n)
// #define PBout(n) BIT_ADDR(GPIOB_ODR_Addr, n)
// #define GPIOout(port, n) BIT_ADDR((&(port->OUTDR)), n)

// #define SCCB_SCL_PORT GPIOB
// #define SCCB_SDA_PORT GPIOB

// #define SCCB_SCL_PIN GPIO_Pin_0
// #define SCCB_SDA_PIN GPIO_Pin_1

// #define SCCB_SCL GPIOout(SCCB_SCL_PORT, SCCB_SCL_PIN)
// #define SCCB_SDA GPIOout(SCCB_SDA_PORT, SCCB_SDA_PIN)

// // #define SCCB_SDA_IN()  {GPIOG->CRH&=0XFF0FFFFF;GPIOG->CRH|=0X00800000;}
// // #define SCCB_SDA_OUT() {GPIOG->CRH&=0XFF0FFFFF;GPIOG->CRH|=0X00300000;}



// #define SCCB_READ_SDA    	PGin(13)  		//输入SDA    
// #define SCCB_ID   			0X42  			//OV7670的ID


// void SCCB_Init(void);
// void SCCB_Start(void);
// void SCCB_Stop(void);
// void SCCB_No_Ack(void);
// uint8_t SCCB_WR_Byte(uint8_t dat);
// uint8_t SCCB_RD_Byte(void);
// uint8_t SCCB_WR_Reg(uint8_t reg,uint8_t data);
// uint8_t SCCB_RD_Reg(uint8_t reg);

// #ifdef __cplusplus
// }
// #endif 

// #endif