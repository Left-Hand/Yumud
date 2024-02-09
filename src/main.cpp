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
#include "memory/flash.hpp"

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
SGM58031 ext_adc(i2cDrvAdc);
TCS34725 tcs(i2cDrvTcs);
VL53L0X vlx(i2cDrvVlx);
PCF8574 pcf(i2cDrvPcf);
AS5600 mags(i2cDrvAS);
TM8211 extern_dac(i2sDrvTm);

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

void BKP_Init(){
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
}

void BKP_WriteData(uint8_t index, uint16_t data){
    if(!index || index > 10) return;
    PWR_BackupAccessCmd(ENABLE);
    BKP_WriteBackupRegister(index << 2, data);
    PWR_BackupAccessCmd(DISABLE);
}

uint16_t BKP_ReadData(uint8_t index){
    if(!index || index > 10) return 0;
    return BKP_ReadBackupRegister(index << 2);
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

typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;
#define PAGE_WRITE_START_ADDR  ((uint32_t)0x0800F000) /* Start from 60K */
#define PAGE_WRITE_END_ADDR    ((uint32_t)0x08010000) /* End at 64K */
#define FLASH_PAGE_SIZE                   4096

/* Global Variable */
uint32_t EraseCounter = 0x0, Address = 0x0;
uint16_t Data = 0xAAAA;
uint32_t WRPR_Value = 0xFFFFFFFF, ProtectedPages = 0x0;
uint32_t NbrOfPage;
volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
volatile TestStatus MemoryProgramStatus = PASSED;
volatile TestStatus MemoryEraseStatus = PASSED;
u32 buf[64];

/*********************************************************************
 * @fn      Flash_Test
 *
 * @brief   Flash Program Test.
 *
 * @return  none
 */
void Flash_Test(void)
{
    uart1.print("FLASH Test\r\n");

    __disable_irq();

    RCC_ClocksTypeDef RCC_CLK;
	RCC_GetClocksFreq(&RCC_CLK);

    uint32_t hclkFrequency = RCC_CLK.HCLK_Frequency;
    if(hclkFrequency > 120000000U){
        RCC_HCLKConfig(RCC_SYSCLK_Div4);
    }else if(hclkFrequency > 60000000U){
        RCC_HCLKConfig(RCC_SYSCLK_Div2);
    }

    uart1.setBaudRate(UART1_Baudrate);
    FLASH_Unlock();

    NbrOfPage = (PAGE_WRITE_END_ADDR - PAGE_WRITE_START_ADDR) / FLASH_PAGE_SIZE;

    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP |FLASH_FLAG_WRPRTERR);

    for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
    {
      FLASHStatus = FLASH_ErasePage(PAGE_WRITE_START_ADDR + (FLASH_PAGE_SIZE * EraseCounter));  //Erase 4KB

    if(FLASHStatus != FLASH_COMPLETE){
        uart1.print("FLASH Erase Fail\r\n");
    }
    uart1.print("FLASH Erase Suc\r\n");
    }

    Address = PAGE_WRITE_START_ADDR;
    uart1.print("Programming...\r\n");
    while((Address < PAGE_WRITE_END_ADDR) && (FLASHStatus == FLASH_COMPLETE)){
        FLASHStatus = FLASH_ProgramHalfWord(Address, Data);
        Address = Address + 2;
    }

    Address = PAGE_WRITE_START_ADDR;

    uart1.print("Program Checking...\r\n");
    while((Address < PAGE_WRITE_END_ADDR) && (MemoryProgramStatus != FAILED)){
        uint16_t result =*(volatile uint16_t *)Address;
        if(result != Data){
            MemoryProgramStatus = FAILED;
        }
        if(Address < 10) uart1.println(result);
        Address += 2;
    }

    if(MemoryProgramStatus == FAILED)
    {
        uart1.print("Memory Program FAIL!\r\n");
    }
    else{
        uart1.print("Memory Program PASS!\r\n");
    }

    FLASH_Lock();

    RCC_HCLKConfig(RCC_HCLK_Div1);
    uart1.setBaudRate(UART1_Baudrate);
    __enable_irq();
}

uint16_t FLash_Read(uint32_t Address){
    return (*(__IO uint16_t*) (Address & 0xFFFFFFFE));
}

RGB565 color = 0xffff;
const RGB565 white = 0xffff;
const RGB565 black = 0;
const RGB565 red = RGB565(31,0,0);
const RGB565 green = RGB565(0,63,0);
const RGB565 blue = RGB565(0,0,31);

uint64_t begin_u = 0;
uint64_t begin_m = 0;

struct DebugMeasures{
    uint32_t t_base;
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;
}debugMeasures;

real_t delta = real_t(0);
real_t fps = real_t(0);
real_t fps_filtered = real_t(0);
real_t t = real_t(0);

__fast_inline void reCalculateTime(){
    #ifdef USE_IQ
    t.value = msTick * (int)(0.001 * (1 << GLOBAL_Q));
    #else
    t = msTick * (1 / 1000.0f);
    #endif
}

void SysInfo_ShowUp(){
    RCC_ClocksTypeDef RCC_CLK;
	RCC_GetClocksFreq(&RCC_CLK);//Get chip frequencies

    uart1.setSpace(" ");

    uart1.println("\r\n\r\n------------------------");
	uart1.println("System Clock Source : ", (int)RCC_GetSYSCLKSource());
	uart1.println("APB1/PCLK1 : ", (int)RCC_CLK.PCLK1_Frequency, "Hz");
	uart1.println("APB2/PCLK2 : ", (int)RCC_CLK.PCLK2_Frequency, "Hz");
	uart1.println("SYSCLK     : ", (int)RCC_CLK.SYSCLK_Frequency, "Hz");
	uart1.println("HCLK       : ", (int)RCC_CLK.HCLK_Frequency, "Hz");

    uint16_t flash_size = *(volatile uint16_t *)0x1FFFF7E0;
    uint32_t chip_id[2];
    chip_id[0] = *(volatile uint32_t *)0x1FFFF7E8;
    chip_id[1] = *(volatile uint32_t *)0x1FFFF7EC;

	uart1.println("FlashSize       : ", (int)flash_size, "KB");
    uart1 << SpecToken::Hex;
    uart1.println("ChipID          : ", (uint64_t)chip_id[0], chip_id[1]);

    RCC_AHBPeriphClockCmd(RCC_CRCEN, ENABLE);
    CRC_ResetDR();
    uint32_t crc_code = CRC_CalcBlockCRC(chip_id, 3);
    uart1.println("CRC code:", crc_code);
    uart1 << SpecToken::Dec;
}
int main(){
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    Systick_Init();
    GPIO_PortC_Init();
    // TIM2_Init();
    // HX711_GPIO_Init();
    GPIO_SW_I2C_Init();
    GPIO_SW_I2S_Init();

    uart1.init(UART1_Baudrate);
    uart2.init(UART2_Baudrate);

    SysInfo_ShowUp();

    for(uint8_t i = 0; i <10; i++){
        uart1 << SpecToken::Hex;
        uart1.println(FLash_Read(PAGE_WRITE_START_ADDR + i));
    }
 	// auto ok1 = EEPROM_Init(nullptr);
    BKP_Init();
	uint16_t boot_count = BKP_ReadData(1);
	// auto ok2 = Config_Read_Buf(0, &boot_count, sizeof(boot_count));
    boot_count++;
    BKP_WriteData(1, boot_count);
    // TODO :fix flash
	// if(boot_count % 2)
        // Flash_Test();

    uart1.println("System boot times: ", boot_count);
	// auto ok3 =Config_Write_Buf(0, &boot_count, sizeof(boot_count));
	// uart1.println("System boot times: ", boot_count, ok2, ok3, EE_ReadWord(0), EE_ReadWord(1));



    spi2.init(144000000);
    spi2.configDataSize(8);
    spi2.configBaudRate(144000000 / 2);

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
        tftDisplayer.setFlushDirH(false);
        tftDisplayer.setFlushDirV(false);
        tftDisplayer.setInversion(true);
    }else{
        tftDisplayer.init();
        tftDisplayer.setDisplayArea(240, 240, 0, 0);

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
    ext_adc.init();
    ext_adc.setContMode(true);
    ext_adc.setFS(SGM58031::FS::FS2_048);
    ext_adc.setMux(SGM58031::MUX::P0NG);
    ext_adc.setDataRate(SGM58031::DataRate::DR960);
    ext_adc.startConv();
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



        reCalculateTime();
        PC13_2 = !PC13_2;
        // uart1.println("a small fox jumps over a lazy dog!!", "a small fox jumps over a lazy dog!!", "a small fox jumps over a lazy dog!!"
        // ,"a small fox jumps over a lazy dog!!", "a small fox jumps over a lazy dog!!", "a small fox jumps over a lazy dog!!");
        // PCout(13) = !PCin(13);
        // uint64_t ns_before = nanos();
        // for(uint8_t i = 0; i<128; i++) reCalculateTime();
        // uint64_t ns_after = nanos();
        // uart1.println((uint32_t)(ns_after - ns_before) / 128);
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

