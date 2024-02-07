#include "ST7789/st7789.h"
#include "stdio.h"
#include "../types/real.hpp"
#include "../types/string/String.hpp"
#include "../types/complex/complex_t.hpp"
#include "../types/vector2/vector2_t.hpp"
#include "../types/color/color_t.hpp"
#include "../types/matrix/matrix.hpp"
#include "MLX90640/MLX90640_API.h"
#include "SDcard/SPI2_Driver.h"
#include "HX711/HX711.h"
#include "TTP229/TTP229.h"

#include "bus/bus_inc.h"

#include "ST7789V2/st7789.hpp"
#include "SSD1306/ssd1306.hpp"
#include "MPU6050/mpu6050.hpp"
#include "SGM58031/sgm58031.hpp"
#include "TCS34725/tcs34725.hpp"
#include "VL53L0X/vl53l0x.hpp"
#include "PCF8574/pcf8574.hpp"
#include "AS5600/as5600.hpp"
#include "TM8211/tm8211.hpp"
#include "gpio/gpio.hpp"

using Complex = Complex_t<real_t>;
using Color = Color_t<real_t>;
using Vector2 = Vector2_t<real_t>;

#define I2C_SW_SCL GPIO_Pin_6
#define I2C_SW_SDA GPIO_Pin_7

#define I2S_SW_SCK GPIO_Pin_10
#define I2S_SW_SDA GPIO_Pin_11
#define I2S_SW_WS GPIO_Pin_1

Gpio i2cScl = Gpio(GPIOB, I2C_SW_SCL);
Gpio i2cSda = Gpio(GPIOB, I2C_SW_SDA);

Gpio i2sSck = Gpio(GPIOB, I2S_SW_SCK);
Gpio i2sSda = Gpio(GPIOB, I2S_SW_SDA);
Gpio i2sWs = Gpio(GPIOB, I2S_SW_WS);

I2cSw i2cSw(i2cScl, i2cSda);
I2sSw i2sSw(i2sSck, i2sSda, i2sWs);

SpiDrv SpiDrvLcd = SpiDrv(spi2_hs, 0);
SpiDrv spiDrvOled = SpiDrv(spi2, 0);
I2cDrv i2cDrvOled = I2cDrv(i2cSw,(uint8_t)0x78);
I2cDrv i2cDrvMpu = I2cDrv(i2cSw,(uint8_t)0xD0);
I2cDrv i2cDrvAdc = I2cDrv(i2cSw, 0x90);
I2cDrv i2cDrvTcs = I2cDrv(i2cSw, 0x52);
I2cDrv i2cDrvVlx = I2cDrv(i2cSw, 0x52);
I2cDrv i2cDrvPcf = I2cDrv(i2cSw, 0x4e);
I2cDrv i2cDrvAS = I2cDrv(i2cSw, 0x6c);
I2sDrv i2sDrvTm = I2sDrv(i2sSw);

ST7789 tftDisplayer(SpiDrvLcd);
SSD1306 oledDisPlayer(spiDrvOled);
MPU6050 mpu(i2cDrvMpu);
SGM58031 extadc(i2cDrvAdc);
TCS34725 tcs(i2cDrvTcs);
VL53L0X vlx(i2cDrvVlx);
PCF8574 pcf(i2cDrvPcf);
AS5600 mags(i2cDrvAS);
TM8211 extdac(i2sDrvTm);

Gpio PC13 = Gpio(GPIOC, GPIO_Pin_13);

void writePcfGpioImag(uint16_t index, bool value){
    pcf.writeBit(index, value);
}

bool readPcfGpioImag(uint16_t index){
    return pcf.readBit(index);
}

GpioImag PC13_2 = GpioImag(0, 
    [](uint16_t index, bool value){GPIO_WriteBit(GPIOC, GPIO_Pin_13, (BitAction)value);},
    [](uint16_t index) -> bool {return GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13);});

GpioImag tp = GpioImag(0, writePcfGpioImag, readPcfGpioImag);
GpioImag i2cSclIm = GpioImag(1, writePcfGpioImag, readPcfGpioImag);
GpioImag i2cSdaIm = GpioImag(2, writePcfGpioImag, readPcfGpioImag);

I2cSw i2cSwIm = I2cSw(i2cSclIm, i2cSdaIm, 0);

extern "C" void TIM2_IRQHandler(void) __interrupt;

void GPIO_PortC_Init( void ){
    CHECK_INIT

    GPIO_InitTypeDef  GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE );

    PWR_BackupAccessCmd( ENABLE );
    RCC_LSEConfig( RCC_LSE_OFF );
    BKP_TamperPinCmd(DISABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13| GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure );
    PWR_BackupAccessCmd(DISABLE);
}

void GPIO_SW_I2C_Init(void){
    GPIO_InitTypeDef  GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );

    GPIO_InitStructure.GPIO_Pin = I2C_SW_SCL | I2C_SW_SDA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_WriteBit(GPIOB, GPIO_InitStructure.GPIO_Pin, (BitAction)true);
    GPIO_Init( GPIOB, &GPIO_InitStructure );
}

void TIM2_Init(){
    
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
    TIM_TimeBaseStructure.TIM_Period=144000000/20000 - 1;
    TIM_TimeBaseStructure.TIM_Prescaler=1;
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);

    TIM_ARRPreloadConfig(TIM2, DISABLE);
    
    TIM_Cmd(TIM2, ENABLE);

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 
}


void GPIO_SW_I2S_Init(void){
    GPIO_InitTypeDef  GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );

    GPIO_InitStructure.GPIO_Pin = I2S_SW_SDA | I2S_SW_SCK | I2S_SW_WS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_WriteBit(GPIOB, GPIO_InitStructure.GPIO_Pin, (BitAction)true);
    GPIO_Init( GPIOB, &GPIO_InitStructure );
}

RGB565 color = 0xffff;
const RGB565 white = 0xffff;
const RGB565 black = 0;
const RGB565 red = RGB565(31,0,0);
const RGB565 green = RGB565(0,63,0);
const RGB565 blue = RGB565(0,0,31);

uint64_t begin_u = 0;
uint64_t begin_m = 0;

struct DebugMeausres{
    uint32_t t_base;
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;
}debugMeausres;

real_t delta = real_t(0);
real_t fps = real_t(0);
real_t fps_filted = real_t(0);
real_t t = real_t(0);


int main(){
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    Systick_Init();
    GPIO_PortC_Init();
    TIM2_Init();
    // HX711_GPIO_Init();
    GPIO_SW_I2C_Init();
    GPIO_SW_I2S_Init();
    // TTP229_GPIO_Init();
    // delayMicroseconds(20);
 

    uart1.init(115200 * 4);
    uart2.init(115200);

    spi2.init(144000000);
    spi2.configDataSize(8);
    spi2.configBaudRate(144000000 / 2);
    // spi2.configBitOrder(false);

    LCD_Init();
    
    bool use_tft = true;
    bool use_mini = false;
    if(use_tft){
    if(use_mini){
        tftDisplayer.init();
        tftDisplayer.setDisplayArea(160, 80, 1, 26);
        tftDisplayer.setFlipX(true);
        tftDisplayer.setFlipY(false);
        tftDisplayer.setSwapXY(true);
        tftDisplayer.setFormatRGB(false);
        tftDisplayer.setReflashDirH(false);
        tftDisplayer.setReflashDirV(false);
        tftDisplayer.setInversion(true);
    }else{
        tftDisplayer.init();
        tftDisplayer.setDisplayArea(240, 240, 0, 0);

        tftDisplayer.setFlipX(false);
        tftDisplayer.setFlipY(false);
        tftDisplayer.setSwapXY(false);
        tftDisplayer.setFormatRGB(true);
        tftDisplayer.setReflashDirH(false);
        tftDisplayer.setReflashDirV(false);
        tftDisplayer.setInversion(true);
    }}else{
        oledDisPlayer.init();

        oledDisPlayer.setOffsetY(6);
        oledDisPlayer.setFlipX(false);
        oledDisPlayer.setFlipY(false);
        oledDisPlayer.setInversion(false);
    }

    // mpu.init();
    extadc.init();
    extadc.setContMode(true);
    extadc.setFS(SGM58031::FS::FS2_048);
    extadc.setMux(SGM58031::MUX::P0NG);
    extadc.setDataRate(SGM58031::DataRate::DR960);
    extadc.startConv();
    // tcs.init();
    // tcs.setIntegration(48);
    // tcs.setGain(TCS34725::Gain::X60);
    // tcs.startConv();
    // vlx.init();
    // vlx.setContinuous(true);
    // vlx.setHighPrecision(false);
    // vlx.startConv();
    // mags.init();
    extdac.setDistort(5);
    extdac.setRail(real_t(1), real_t(4));

    Color c1 = Color::from_hsv(0);
    c1 = 3 * Color::from_hsv(20);
    while(1){

        c1 = Color::from_hsv(frac(t));
        color = c1;

        if(use_tft){
            tftDisplayer.flush(color);
        }else{
            oledDisPlayer.flush(true);
        }

        uint64_t delta_u = (micros() - begin_u);
        begin_u = micros();
        delta = real_t(delta_u / 1000000.0f);



        // if(extadc.isIdle()){
        //     uart1.println(extadc.getConvData(), extdac.ldata);
        //     extadc.startConv();
        // }
        PC13_2 = !PC13_2;
        // t += delta;

    }
}

real_t wave(const real_t & x){
    if(x > 0){
        const real_t play_v(50);
        return  exp(-10 * x) * sin(TAU * frac(play_v * x));
    }else{
        return real_t(0);
    }
}
extern "C"{
void TIM2_IRQHandler(void) 
{ 	    	  	
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
        t = real_t(micros() / 1000000.0f);
        real_t t_frac = frac(frac(t) * 3);
        real_t volt_out_l = (2.5 + 0.4 * (wave(t_frac) + wave(t_frac - 0.25) + wave(t_frac - 0.5)));
        extdac.setVoltage(volt_out_l, volt_out_l);

		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }     
}
}

