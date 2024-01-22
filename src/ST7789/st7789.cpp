
#include "st7789.h"

SPI_InitTypeDef  SPI_InitStructure;

// static volatile uint16_t buf[H][W] = {0};
static volatile uint16_t ConstData;

void LCD_GPIO_Init(void){
    CHECK_INIT

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure = {0};

    //PB3 SCLK
    //PB5 MOSI
    //PA15 DC

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = LCD_DC_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LCD_DC_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = LCD_RES_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LCD_RES_PORT, &GPIO_InitStructure);

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE);
}
void SPI1_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE );	

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRate;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
 
	SPI_Cmd(SPI1, ENABLE);
	
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, 0);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    // SPI_I2S_ReceiveData(SPI1);
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

void LCD_Write_Const_16b(uint16_t data, uint32_t length){

    while ((SPI1->STATR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);	
    SPI_DataSizeConfig(SPI1, SPI_DataSize_16b);

    #ifdef USE_DMA

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
    SPI_DataSizeConfig(SPI1, SPI_DataSize_8b);
}

void LCD_Write_Pool_16b(uint16_t * dataTxPool, uint32_t length)
{
    while ((SPI1->STATR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);	

    SPI_DataSizeConfig(SPI1, SPI_DataSize_16b);
    // delayMicroseconds(200);

    #ifdef USE_DMA

    DMA1_CH3_DataSizeConfig(DMA_MemoryDataSize_HalfWord);
    DMA1_CH3_DataLengthConfig(length);
    DMA1_CH3_SourceConfig((void *)dataTxPool);
    DMA1_CH3_IncConfig(ENABLE);

    SPI1_DMA_Start();
    while(DMA_GetFlagStatus(DMA1_FLAG_TC3) != SET);
    DMA_ClearFlag(DMA1_FLAG_TC3);
    // SPI1_DMA_Stop();

    #else

    for(uint32_t i = 0; i < length;i++){
        SPI1_Write_16b(dataTxPool[i]);
    }
    
    #endif

    SPI_DataSizeConfig(SPI1, SPI_DataSize_8b);
}


void LCD_Init(){

    LCD_GPIO_Init();					 
    SPI1_Init();

    #ifdef USE_DMA
    DMA1_CH3_Init((void *)(&ConstData), (void *)(&SPI1->DATAR));	
    #endif

    LCD_RESET_RES
    delay(120);
    LCD_SET_RES

    LCD_Write_Command(0x01);
    delayMicroseconds(150);
	LCD_Write_Command(0x11);
	delayMicroseconds(120);
	LCD_Write_Command(0x3A);
	LCD_Write_Data_8b(0x55);
	LCD_Write_Command(0x36);
	LCD_Write_Data_8b(0x00);
	LCD_Write_Command(0x21);
	LCD_Write_Command(0x13);
	LCD_Write_Command(0x29);
	
}

void LCD_Write_Data_8b(uint8_t data){
   LCD_ON_DATA
   SPI1_Write_8b(data);
}

void LCD_Write_Data_16b(uint16_t data){
   LCD_ON_DATA
   SPI1_Write_8b((uint8_t)(data >> 8));
   SPI1_Write_8b((uint8_t)data);
}

void LCD_Write_Command(uint8_t cmd){
    LCD_ON_COMMAND
    SPI1_Write_8b(cmd);
}

void LCD_Set_Postion(uint16_t x0, uint16_t y0){
	LCD_Write_Command(0x2a);
    LCD_Write_Data_16b(x0);
	
	LCD_Write_Command(0x2b);
    LCD_Write_Data_16b(y0);

    LCD_Write_Command(0x2c);
}

void LCD_Set_Window(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h){
	LCD_Write_Command(0x2a);
    LCD_Write_Data_16b(x0);
    LCD_Write_Data_16b(x0 + w);
	
	LCD_Write_Command(0x2b);
    LCD_Write_Data_16b(y0);
    LCD_Write_Data_16b(y0 + h);

    LCD_Write_Command(0x2c);
}

void LCD_Draw_Pixel(int16_t x0, int16_t y0, uint16_t color){
    if(x0 < 0 || y0 < 0 || x0 >= W || y0 >= H) return;

	LCD_Set_Postion(x0, y0);
    LCD_Write_Data_16b(color);
}


void LCD_Draw_Hrizon_Line(int16_t x0, int16_t y0, int16_t l, uint16_t color){
    int16_t xa = MAX(MIN(x0, x0 + l), 0);
    int16_t xb = MIN(MAX(x0, x0 + l), W);
    if(xb <= xa || y0 < 0 || y0 >= H) return;

    uint16_t dx = xb - xa;
    LCD_Set_Postion(xa, y0);

    LCD_ON_DATA
    LCD_Write_Const_16b(color, dx);
}

void LCD_Draw_Vertical_Line(int16_t x0, int16_t y0, int16_t l, uint16_t color){
    int16_t ya = MAX(MIN(y0, y0 + l), 0);
    int16_t yb = MIN(MAX(y0, y0 + l), H);
    if(x0 < 0 || x0 >= W || ya >= yb) return;

    
    for(uint16_t y = ya; y < yb; y++){
        LCD_Set_Postion(x0, y);
        LCD_Write_Data_16b(color);
    }
}

void LCD_Draw_Filled_Rect(int16_t x0, int16_t y0, int16_t w, int16_t h, uint16_t color){
    int16_t xa = MAX(MIN(x0, x0 + w), 0);
    int16_t ya = MAX(MIN(y0, y0 + h), 0);
    int16_t xb = MIN(MAX(x0, x0 + w), W);
    int16_t yb = MIN(MAX(y0, y0 + h), H);

    if(xb < 0 || xa >= W || xa == xb ||
        yb < 0 || ya >= H || ya == yb) return;

    uint16_t dx = xb - xa;

    for(uint16_t y = ya; y < yb; y++){
        LCD_Draw_Hrizon_Line(xa, y, dx, color);
    }
}

void LCD_Draw_Hollow_Rect(int16_t x0, int16_t y0, int16_t w, int16_t h, uint16_t color){
    int16_t xa = MAX(MIN(x0, x0 + w), 0);
    int16_t ya = MAX(MIN(y0, y0 + h), 0);
    int16_t xb = MIN(MAX(x0, x0 + w), W);
    int16_t yb = MIN(MAX(y0, y0 + h), H);

    uint16_t dx = xb - xa;
    uint16_t dy = yb - ya;

    if(xb < 0 || xa >= W || xa == xb ||
        yb < 0 || ya >= H || ya == yb) return;

    if(dy >= 2){
        LCD_Draw_Hrizon_Line(xa, ya, dx, color);
        LCD_Draw_Hrizon_Line(xa, yb, dx, color);
        LCD_Draw_Vertical_Line(xa, ya + 1, dy - 2, color);
        LCD_Draw_Vertical_Line(xb, ya + 1, dy - 2, color);
    }else{
        LCD_Draw_Hrizon_Line(xa, ya, dx, color);
    }
}

void LCD_Draw_Line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color){
    if(y0 == y1){
        LCD_Draw_Hrizon_Line(x0, y0, x1 - x0, color);
        return;
    }else if (x0 == x1){
        LCD_Draw_Vertical_Line(x0, y0, y1 - y0, color);
        return;
    }
    
    int16_t dx=ABS(x1-x0);
    int8_t sx=(x1 > x0)? 1: -1;
    int16_t dy=ABS(y1-y0);
    int8_t sy=(y1 > y0)? 1 : -1;

    int16_t err=(dx>dy ? dx : -dy)/2;
    int16_t e2;
    
    int16_t x = x0;
    int16_t y = y0;

    while(1){
        LCD_Draw_Pixel(x, y, color);
        if (x==x1 && y==y1) 
            break;

        e2=err;
        if (e2>-dx) { 
            err-=dy; 
            x+=sx; 
        }
        if (e2<dy) { 
            err+=dx; 
            y+=sy; 
        }
    }
}

void LCD_Fill_Screen(uint16_t color){
    for(uint16_t y = 0; y < H; y++)
        LCD_Draw_Hrizon_Line(0,y,W,color);
}

void LCD_Draw_Textured_Line(int16_t x0, int16_t y0, int16_t l, uint16_t * buf){
    if(y0 < 0 || l == 0) return;
    if((x0 < 0 && l < 0) || (x0 >= W && l > 0)) return;

    int16_t xa = MAX(MIN(x0, x0 + l), 0);
    int16_t xb = MIN(MAX(x0, x0 + l), W);


    uint16_t times;
    uint16_t x;

    if(x0 < 0){
        x = 0;
        times = xb - x;
    }else if(x0 >= W){
        x = xa;
        times = W - 1 - x;
    }else{
        x = xa;
        times = xb - x;
    }

    LCD_Set_Postion(x, y0);
    LCD_ON_DATA;
    LCD_Write_Pool_16b(&buf[l-times], times);
}

void LCD_Draw_Image(int16_t x0, int16_t y0, int16_t w, int16_t h, uint16_t * buf){
    uint16_t xa = MIN(x0, x0 + w);
    int16_t xb = MIN(MAX(x0, x0 + w), W);
    uint16_t ya = MIN(y0, y0 + h);
    int16_t yb = MIN(MAX(y0, y0 + h), H);

    if( xb >= W || xa == xb || yb >= H || ya == yb) return;

    // uint16_t dx = xb - xa;
    // uint16_t dy = yb - ya;

    LCD_Set_Postion(xa, ya);

    LCD_ON_DATA
    LCD_Write_Pool_16b(buf, w*h);
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

void SWAP(int16_t * x, int16_t * y){
    int16_t temp = *y;
    *y = *x;
    *x = temp;
}