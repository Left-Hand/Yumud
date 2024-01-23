#ifndef __ST7789_H__

#define __ST7789_H__

#include "stdint.h"
#include "clock/clock.h"
#include "defines/comm_defs.h"
// #ifdef __cplusplus
// extern "C" {
// #endif

#define LCD_DC_PORT GPIOA
#define LCD_DC_PIN GPIO_Pin_15

#define LCD_RES_PORT GPIOA
#define LCD_RES_PIN GPIO_Pin_12

#define LCD_ON_DATA LCD_DC_PORT -> BSHR = LCD_DC_PIN;
#define LCD_ON_COMMAND LCD_DC_PORT -> BCR = LCD_DC_PIN;

#define LCD_SET_RES LCD_DC_PORT -> BSHR = LCD_RES_PIN;
#define LCD_RESET_RES LCD_DC_PORT -> BCR = LCD_RES_PIN;

// #define USE_DMA


void LCD_Init();
void LCD_Write_Data_8b(uint8_t data);
void LCD_Write_Data_16b(uint16_t data);
void LCD_Write_Command(uint8_t data);
void LCD_Fill_Screen(uint16_t color);

void LCD_Set_Postion(uint16_t x0, uint16_t y0);
void LCD_Draw_Pixel(int16_t x0, int16_t y0, uint16_t color);
void LCD_Draw_Hrizon_Line(int16_t x0, int16_t y0, int16_t l, uint16_t color);
void LCD_Draw_Vertical_Line(int16_t x0, int16_t y0, int16_t l, uint16_t color);
void LCD_Draw_Filled_Rect(int16_t x0, int16_t y0, int16_t w, int16_t h, uint16_t color);
void LCD_Draw_Hollow_Rect(int16_t x0, int16_t y0, int16_t w, int16_t h, uint16_t color);
void LCD_Draw_Line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void LCD_Draw_Image(int16_t x0, int16_t y0, int16_t w, int16_t h, uint16_t * buf);
void LCD_Draw_Textured_Line(int16_t x0, int16_t y0, int16_t l, uint16_t * buf);
void LCD_Draw_Char();

void SPI1_Init(void);
void SPI1_Write_8b(uint8_t dataTx);
void SPI1_Write_16b(uint16_t dataTx);

void LCD_Write_Const_16b(uint16_t data, uint32_t length);
void LCD_Write_Pool_16b(uint16_t * dataTxPool, uint32_t length);

void DMA1_CH3_Init(void * data, void * reg);
void DMA1_CH3_DataSizeConfig(uint16_t size);
void DMA1_CH3_DataLengthConfig(uint32_t length);
void DMA1_CH3_IncConfig(FunctionalState inc);
void DMA1_CH3_SourceConfig(void * source);

void SPI1_DMA_Start();

void SPI1_DMA_Stop();

// #ifdef __cplusplus
// }
// #endif
#endif
