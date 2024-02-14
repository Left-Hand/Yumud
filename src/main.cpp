#include "misc.h"

using Complex = Complex_t<real_t>;
using Color = Color_t<real_t>;


Gpio i2cScl = Gpio(GPIOB, I2C_SW_SCL);
Gpio i2cSda = Gpio(GPIOB, I2C_SW_SDA);

Gpio i2sSck = Gpio(GPIOB, I2S_SW_SCK);
Gpio i2sSda = Gpio(GPIOB, I2S_SW_SDA);
Gpio i2sWs = Gpio(GPIOB, I2S_SW_WS);

I2cSw i2cSw(i2cScl, i2cSda);
I2sSw i2sSw(i2sSck, i2sSda, i2sWs);

SpiDrv SpiDrvLcd = SpiDrv(spi2_hs, 0);
SpiDrv spiDrvOled = SpiDrv(spi2, 0);
SpiDrv spiDrvFlash = SpiDrv(spi1, 0);
SpiDrv spiDrvMagSensor = SpiDrv(spi1, 0);
SpiDrv spiDrvRadio = SpiDrv(spi1, 0);
// I2cDrv i2cDrvOled = I2cDrv(i2cSw,(uint8_t)0x78);
// I2cDrv i2cDrvMpu = I2cDrv(i2cSw,(uint8_t)0xD0);
I2cDrv i2cDrvAdc = I2cDrv(i2c1, 0x90);
// I2cDrv i2cDrvTcs = I2cDrv(i2cSw, 0x52);
// I2cDrv i2cDrvVlx = I2cDrv(i2cSw, 0x52);
// I2cDrv i2cDrvPcf = I2cDrv(i2cSw, 0x4e);
// I2cDrv i2cDrvAS = I2cDrv(i2cSw, 0x6c);
I2sDrv i2sDrvTm = I2sDrv(i2sSw);

ST7789 tftDisplayer(SpiDrvLcd);
SSD1306 oledDisPlayer(spiDrvOled);
// MPU6050 mpu(i2cDrvMpu);
SGM58031 ext_adc(i2cDrvAdc);
LT8920 radio(spiDrvRadio);
// TCS34725 tcs(i2cDrvTcs);
// VL53L0X vlx(i2cDrvVlx);
// PCF8574 pcf(i2cDrvPcf);
// AS5600 mags(i2cDrvAS);
TM8211 extern_dac(i2sDrvTm);
W25QXX extern_flash(spiDrvFlash);
MA730 mag_sensor(spiDrvMagSensor);

Gpio blueLed = Gpio(BUILTIN_LED_PORT, BUILTIN_RedLED_PIN);

extern "C" void TIM2_IRQHandler(void) __interrupt;



void TIM2_Init(){
    //PA0 CH1
    //PA1 CH2
    //PA2 CH3
    //PA3 CH4
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_1);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);

	TIM_ICInitTypeDef TIM_ICInitStruct;
    TIM_ICStructInit(&TIM_ICInitStruct);

	TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStruct.TIM_ICFilter = 0xF;


	TIM_ICInit(TIM2,&TIM_ICInitStruct);

	TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;

	TIM_ICInit(TIM2,&TIM_ICInitStruct);

	TIM_EncoderInterfaceConfig(TIM2,TIM_EncoderMode_TI12,TIM_ICPolarity_Rising,TIM_ICPolarity_Rising);

    TIM_Cmd(TIM2, ENABLE);
}


RGB565 color = 0xffff;
const RGB565 white = 0xffff;
const RGB565 black = 0;
const RGB565 red = RGB565(31,0,0);
const RGB565 green = RGB565(0,63,0);
const RGB565 blue = RGB565(0,0,31);

real_t delta = real_t(0);
real_t fps = real_t(0);
real_t t = real_t(0);

namespace Shaders{
__fast_inline RGB565 ShaderP(const Vector2i & pos){
    static int cnt = 0;
    cnt++;
    return pos.x + pos.y + cnt;
}

__fast_inline RGB565 ShaderUV(const Vector2 & UV){
    if((UV - Vector2(0.5, 0.5)).length_squared() <= real_t(0.25)){
        return RGB565::RED;
    }else{
        return RGB565::BLUE;
    }
}

__fast_inline RGB565 Mandelbrot(const Vector2 & UV){
    Complex c(lerp(UV.x, real_t(0.5), real_t(-1.5)), lerp(UV.y, real_t(-1), real_t(1)));
    Complex z;
    uint8_t count = 0;

    while ((z < real_t(4)) && (count < 23))
    {
        z = z*z + c;
        count = count + 1;
    }
    return count * 100;
}
};




int main(){
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    Systick_Init();
    GPIO_PortC_Init();
    GPIO_SW_I2S_Init();

    uart1.init(UART1_Baudrate);
    uart2.init(UART2_Baudrate);

    spi2.init(72000000);
    spi1.init(18000000);

    GLobal_Reset();
    SysInfo_ShowUp();

    TIM2_Init();
    bool use_tft = true;
    bool use_mini = false;
    if(use_tft){
    if(use_mini){
        tftDisplayer.init();
        tftDisplayer.setDisplayArea(Rect2i(0, 0, 160, 80));
        tftDisplayer.setDisplayOffset(Vector2i(1, 26));
        tftDisplayer.setFlipX(true);
        tftDisplayer.setFlipY(false);
        tftDisplayer.setSwapXY(true);
        tftDisplayer.setFormatRGB(false);
        tftDisplayer.setFlushDirH(false);
        tftDisplayer.setFlushDirV(false);
        tftDisplayer.setInversion(true);
    }else{
        tftDisplayer.init();
        tftDisplayer.setDisplayArea(Rect2i(0, 0, 240, 240));

        tftDisplayer.setFlipX(false);
        tftDisplayer.setFlipY(false);
        tftDisplayer.setSwapXY(false);
        tftDisplayer.setFormatRGB(true);
        tftDisplayer.setFlushDirH(false);
        tftDisplayer.setFlushDirV(false);
        tftDisplayer.setInversion(true);
    }}else{
        oledDisPlayer.init();

        oledDisPlayer.setOffsetY(6);
        oledDisPlayer.setFlipX(false);
        oledDisPlayer.setFlipY(false);
        oledDisPlayer.setInversion(false);
    }

    // mpu.init();
    // ext_adc.init();
    // // print("ext_adc ini")
    // ext_adc.setContMode(true);
    // ext_adc.setFS(SGM58031::FS::FS4_096);
    // ext_adc.setMux(SGM58031::MUX::P0NG);
    // ext_adc.setDataRate(SGM58031::DataRate::DR960);
    // ext_adc.startConv();
    // radio.init();
    uart1.println("flashCapacity: ", extern_flash.getDeviceCapacity());
    // tcs.init();
    // tcs.setIntegration(48);
    // tcs.setGain(TCS34725::Gain::X60);
    // tcs.startConv();
    // vlx.init();
    // vlx.setContinuous(true);
    // vlx.setHighPrecision(false);
    // vlx.startConv();
    // mags.init();
    extern_dac.setDistort(5);
    extern_dac.setRail(real_t(1), real_t(4));

    // mag_sensor.setPulsePerTurn(30);
    Font6x8 font6x8;
    Painter<RGB565> painter(&tftDisplayer, &font6x8);
    uart1.setSpace(",");
    while(1){
        painter.setColor(RGB565::BLACK);
        painter.flush();
        painter.setColor(RGB565::WHITE);
        painter.drawString(Vector2i(0,0), String(TIM_GetCounter(TIM2)));
        uart1.println(TIM_GetCounter(TIM2));
        reCalculateTime();
        blueLed = !blueLed;
        delay(16);
    }
}

// real_t wave(const real_t & x){
//     if(x > 0){
//         const real_t play_v(50);
//         return  exp(-10 * x) * sin(TAU * frac(play_v * x));
//     }else{
//         return real_t(0);
//     }
// }
// extern "C"{
// void TIM2_IRQHandler(void) 
// { 	    	  	
// 	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
// 	{
//         
//         real_t t_frac = frac(frac(t) * 3);
//         real_t volt_out_l = (2.5 + 0.4 * (wave(t_frac) + wave(t_frac - 0.25) + wave(t_frac - 0.5)));
//         ext_dac.setVoltage(volt_out_l, volt_out_l);

// 		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
//     }     
// }
// }

