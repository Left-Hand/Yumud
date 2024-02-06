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

#include "bus/uart/uart1.hpp"
#include "bus/uart/uart2.hpp"
#include "bus/spi/spi2.hpp"
#include "bus/spi/spi2_hs.hpp"
#include "bus/i2c/i2cSw.hpp"
#include "bus/i2s/i2sSw.hpp"

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

BusDrv SpiDrvLcd = BusDrv(spi2_hs, 0);
BusDrv spiDrvOled = BusDrv(spi2, 0);
BusDrv i2cDrvOled = BusDrv(i2cSw,(uint8_t)0x78);
BusDrv i2cDrvMpu = BusDrv(i2cSw,(uint8_t)0xD0);
BusDrv i2cDrvAdc = BusDrv(i2cSw, 0x90);
BusDrv i2cDrvTcs = BusDrv(i2cSw, 0x52);
BusDrv i2cDrvVlx = BusDrv(i2cSw, 0x52);
BusDrv i2cDrvPcf = BusDrv(i2cSw, 0x4e);
BusDrv i2cDrvAS = BusDrv(i2cSw, 0x6c);
BusDrv i2sDrvTm = BusDrv(i2sSw);

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
    mags.init();

    Color c1 = Color::from_hsv(0);
    c1 = 3 * Color::from_hsv(20);
    while(1){

        // LCD_Fill_Screen(RGB565::BLACK);


        c1 = Color::from_hsv(fmod(t*0.3, real_t(360)));
        color = c1;

        // renderTest4();
        if(use_tft){
            tftDisplayer.flush(color);
            // tftDisplayer.flush(RGB565::BLACK);
        }else{
            oledDisPlayer.flush(true);
            // delay(200);
            // oledDisPlayer.flush(false);
            // delay(200);
        }
        // delay(200);
        // tftDisplayer.flush(RGB565::BLACK);
        // LCD_Fill_Screen(RGB565::BLACK);
        // LCD_Fill_Screen(RGB565::BLUE);
        // LCD_Printf(0,0,white, String(c1).c_str());
        // LCD_Printf(0,8,white, String(Vector2(1,1)).c_str());
        // LCD_Printf(0,16,white, String(Complex(1,1)).c_str());

        uint64_t delta_u = (micros() - begin_u);
        begin_u = micros();
        delta = real_t(delta_u / 1000000.0f);

        if(delta){
            fps = real_t(1) / delta;
            if(!fps_filted){
                fps_filted = (fps_filted * 19 + fps) / 20;
            }else{
                fps_filted = fps;
            }
        }

        // LCD_Printf(170, 240 - 1 - 8, white, (String("fps: ") + String(fps)).c_str());

        // uart1.print(SpecToken::Comma, SpecToken::Eps4, 8.45723, 2467.08849, 79,SpecToken::CommaWithSpace,"hi",33, c1.get_h(), c1);
        // uart1.print(SpecToken::Comma, SpecToken::Eps4, 8.45723, 2467.08849, 79,SpecToken::CommaWithSpace,"hi",33);
        // uart1.print(SpecToken::Comma, SpecToken::Eps4, 8.45723, 2467.08849, 79,SpecToken::CommaWithSpace,"hi",33);
        // uart1.print(SpecToken::Comma, SpecToken::Eps3, 8.45723,SpecToken::Eps4, 2467.08849, 79,SpecToken::CommaWithSpace,"hi",33);
        // uart1.println(SpecToken::Comma, SpecToken::Eps4, 8.45723, 2467.08849, 79,SpecToken::CommaWithSpace,"hi",33);
        // uart1.println(SpecToken::Eps4, 
        //     8.5532234, 2.4687, "hi",33,
        //     8.5532234, 2.4687, "hi",33,
        //     8.5532234, 2.4687, "hi",33,
        //     8.5532234, 2.4687, "hi",33
        // );

        // static uint8_t cnt = 0;
        // static char chr = 'A';
        // cnt++;
        // chr = (chr == 'Z' ? 'A' : chr + 1);
        // for(uint8_t i = 0; i < 23; i++) uart2.print((char)(chr+i));

        // const char str[] = "Hello, a small fox jumps over a lazy dog!";
        // uint8_t cnt = sizeof(str);

        // uint8_t to_send[cnt] = {0};
        // memcpy((void *)to_send, str, cnt);

        // uint8_t to_recv[cnt] = {0};

        // spi2.begin();
        // spi2.transfer((void *)to_recv, (void *)to_send, cnt);
        // spi2.end();

        // uart1.println("Send:", (char *)to_send);
        // uart1.println("Recv:", String((char *)to_recv, cnt - 1));
        // uart2.print(str);
        // uint64_t startms = micros();
        // for(volatile uint32_t i = 0; i < 1000000; i++){
        //     // GPIOC->BSHR = GPIO_Pin_0;

        //     GPIOC -> BSHR = GPIO_Pin_0;
        //     GPIOC->BCR = GPIO_Pin_1;
        //     GPIOC -> BSHR = GPIO_Pin_0;
        //     GPIOC->BCR = GPIO_Pin_1;
        // }
        // while(uart1.available() < sizeof(str) - 1);    
        // uint64_t endms = micros();

        // String ret = uart1.readAll();
        // ret.trim();

        // // uart1.println(SpecToken::Space, "recv: ", ret, (endms - startms));
        // String stri = "test, a, ,a, b";
        // String strm = "a,";
        // String strr = "!!";

        // uart1.println(stri, stri.replace(strm, strr));


        // static bool calied = false;
        // if(!calied){
        //     HX711_Cali();
        //     calied = true;
        // }
        
        // uint32_t start_m = micros();
        // int32_t weight = HX711_Get_Weight();
        // uint32_t waste_m = micros() - start_m;

        // // uart1 << ;
        // if(HX711_Valid())
        //     uart1.println(SpecToken::CommaWithSpace,waste_m,weight);

        // uint32_t start_m = micros();
        // TTP229_Scan();


        // uint32_t waste_m = micros() - start_m;
        // int32_t key = TTP229_Get_Key();
        // delayNanoseconds(100);
        // uart1.println(SpecToken::CommaWithSpace,waste_m,key);
        // uint8_t cmd_token = 0x00;
        // uint8_t cmd = 0x89;
        // if(i2cDrv.isI2cBus()){
        //     uint8_t buf[2] = {cmd_token, cmd};
        //     i2cDrv.write(buf, 2);
        // }
            // busdrv.write(cmd_token);
            // busdrv.write(cmd);
        // uint8_t buf[] = "test";     
        // i2cDrv.write(&buf[0], (size_t)sizeof(buf));
            // i2cDrv.end();
        // }
        // uart1.println(typeid(i2cDrv) == typeid(int));
        // i2cDrv.write((uint8_t)0x55);



        // uint8_t data = 0;
        // i2cDrv.read(data);
        // uart1.println(data);
        // mpu.reflash();
        // extadc.getId();

        // mpu.getTemprature();
        // static uint8_t low_band_cnt = 0;
        // low_band_cnt++;
        // extadc.getId();
        // static bool started = false;
        // int16_t buf[960];
        // for(uint16_t i = 0; i < 960; i++){
        //     while(!extadc.isIdle());
        //     buf[i] = extadc.getConvData();
        //     extadc.startConv();
        // }
        // for(uint16_t i = 0; i < 960; i++){
        //     uart1.println(buf[i], i);
        // }
        // if (extadc.isIdle()){
        //     // if(!started){
        //         uart1.println(extadc.getConvData());

        //         extadc.startConv();
                // started = true;
            // }else{
                // started = false;
            // }
        // }
        // int test_v = (int)(sin(t)*360);
        // uart2.println(test_v);
        // uart1.println(i2cSw.occupied);
        // mpu.getAccel();
        // mpu.reflash();
        // uart1.println(i2cSw.occupied);
        // for(uint8_t i = 0; i<10;i++){
            // delayMicroseconds(1000);
        // if(tcs.isIdle()){
        //     uint16_t c, r, g, b;
        //     tcs.getCRGB(c,r,g,b);
        //     uart1.println(c,r,g,b);
        //     tcs.startConv();

        //     c1.r = r / 65525.0f;
        //     c1.g = g / 65535.0f;
        //     c1.b = b / 65535.0f;
        //     c1.a = 1.0f;
        // }

        // if(vlx.isIdle()){
        //     vlx.reflash();
        //     uart1.println(vlx.getDistanceMm(), vlx.getAmbientCount(), vlx.getSignalCount());
        //     // vlx.startConv();
        // }

        // tp = PC13_2;

        // i2cSwIm.begin(0x55);
        // i2cSwIm.end();

        // delay(100);
        // uint64_t micro_before = micros();
        // for(uint8_t i = 0; i< 1; i++)uart2.println(String(3 * sin(4 * t)));
        // uint64_t micro_after = micros();
        // uart1.println((uint32_t)(micro_after - micro_before));
        real_t play_v(100);
        real_t volt_out_l = (1.35 + 0.5 * sin(play_v * t));
        real_t volt_out_r = (1.35 + 0.5 * cos(play_v * t));
        extdac.setVoltage((float)volt_out_l, (float)volt_out_r);

        if(extadc.isIdle()){
            static uint8_t adc_prog = 0;
            static real_t volt_in_l(0);
            static real_t volt_in_r(0);
            switch(adc_prog){
            case 0:
                extadc.setMux(SGM58031::MUX::P1NG);
                volt_in_l = extadc.getConvData() * 2.048 / 0x8000;
                adc_prog = 1;
                break;
            case 1:
                extadc.setMux(SGM58031::MUX::P0NG);
                volt_in_r = extadc.getConvData() * 2.048 / 0x8000;
                uart1.println(volt_in_l, volt_in_r);
                adc_prog = 0;
            }
        }

        PC13_2 = !PC13_2;
        // delayMicroseconds
        t += delta;
    }
}

// GENERATE_STD

