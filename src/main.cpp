#include "misc.h"
#include "apps.h"

using Complex = Complex_t<real_t>;
using Color = Color_t<real_t>;

#include "dsp/pll.hpp"
#include "src/opa/opa.hpp"
#include "src/device/Adc/INA226/ina226.hpp"

// constexpr uint32_t SPI1_BaudRate = (144000000/32);
// constexpr uint32_t SPI2_BaudRate = (144000000/8);

#define I2C_BaudRate 400000


#include "src/device/Encoder/Estimmator.hpp"
#include "src/device/Memory/EEPROM/AT24CXX/at24c02.hpp"

// I2cSw i2cSw(i2cScl, i2cSda);


// SpiDrv SpiDrvLcd = SpiDrv(spi2_hs, 0);
// SpiDrv spiDrvOled = SpiDrv(spi2, 0);
// SpiDrv spiDrvFlash = SpiDrv(spi1, 0);

// SpiDrv spiDrvRadio = SpiDrv(spi1, 0);
// I2cDrv i2cDrvOled = I2cDrv(i2cSw,(uint8_t)0x78);
// I2cDrv i2cDrvMpu = I2cDrv(i2cSw,(uint8_t)0xD0);
// I2cDrv i2cDrvAdc = I2cDrv(i2c1, 0x90);
// I2cDrv i2cDrvTcs = I2cDrv(i2cSw, 0x52);
// I2cDrv i2cDrvVlx = I2cDrv(i2cSw, 0x52);
// I2cDrv i2cDrvPcf = I2cDrv(i2cSw, 0x4e);
// I2cDrv i2cDrvAS = I2cDrv(i2cSw, 0x6c);
// 
// I2cDrv i2cDrvAS = I2cDrv(i2cSw, 0x36 << 1);
// I2cDrv i2cDrvQm = I2cDrv(i2cSw, 0x1a);
// I2cDrv i2cDrvBm = I2cDrv(i2cSw, 0xec);
// I2cDrv i2cDrvMt = I2cDrv(i2cSw, 0x0C);
// ST7789 tftDisplayer(SpiDrvLcd);
// SSD13XX oledDisPlayer(spiDrvOled);
// MPU6050 mpu(i2cDrvMpu);
// SGM58031 ext_adc(i2cDrvAdc);
// LT8920 radio(spiDrvRadio);
// TCS34725 tcs(i2cDrvTcs);
// VL53L0X vlx(i2cDrvVlx);
// PCF8574 pcf(i2cDrvPcf);
// AS5600 mags(i2cDrvAS);

// W25QXX extern_flash(spiDrvFlash);

// AS5600 mag_sensor(i2cDrvAS);
// QMC5883L earth_sensor(i2cDrvQm);
// BMP280 prs_sensor(i2cDrvBm);
// MT6701 mt_sensor(i2cDrvMt);



using Sys::t;

// static Printer & logger = uart2;
//     // flash.load(temp);
//     while(true) __WFI;
//     // while(true){
//         // logger.println("Hi");
//         // delay(100);
//     // }

// }


// class Pmdc{
// protected:
//     PwmChannel & pwm_f;
//     PwmChannel & pwm_b;
// public:
//     Pmdc(PwmChannel & _pwm_f, PwmChannel & _pwm_b):pwm_f(_pwm_f), pwm_b(_pwm_b){;}


// }

#include "src/device/Encoder/ABEncoder.hpp"
#include "src/device/Encoder/OdometerLines.hpp"
#include "src/adc/adcs/adc1.hpp"
#include "dsp/oscillator.hpp"
#include "src/device/Dac/TM8211/tm8211.hpp"

#include "json.hpp"

#include <bits/stl_numeric.h>
#include <optional>

#include "src/bus/bus_inc.h"
#include "robots/buck.hpp"
#include "robots/stepper.hpp"


template<typename T>
struct targAndMeasurePair_t{
    T target;
    T measure;
};

using targAndMeasurePair = targAndMeasurePair_t<real_t>;


real_t warp_to_halfpi(const real_t & x){
    real_t ret = x;
    while(ret > PI / 2) ret -= PI;
    while(ret < -PI / 2) ret += PI;
    return ret;
}


struct buckRuntimeValues{
    targAndMeasurePair curr;
    targAndMeasurePair volt;
};



static constexpr size_t mem_size = 8;
char mem_buf[mem_size] = "12345\r\n";

void DMA_INIT(void){
    dma1Ch4.init(DmaChannel::Mode::toPeriph);

    dma1Ch4.begin((void *)(&USART1->DATAR), (void *)mem_buf, mem_size);

    // DMA_InitTypeDef   DMA_InitStructure;

    // NVIC_InitTypeDef  NVIC_InitStructure;



    // RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);                       //使能开启DMA时钟



    // DMA_DeInit(DMA1_Channel4);

    // DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DATAR);        /*设置DMA源：串口3数据寄存器地址*/

    // DMA_InitStructure.DMA_MemoryBaseAddr = (u32)mem_buf;                   /*内存地址(要传输的变量的指针)*/

    // DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                       /*方向：从内存到外设*/

    // DMA_InitStructure.DMA_BufferSize = mem_size;                              /*传输大小DMA_BufferSize=TxSize1*/

    // DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;         /*外设地址不增*/

    // DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                  /*内存地址自增*/

    // DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  /*外设数据单位 字节*/

    // DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;          /*内存数据单位 字节*/

    // DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                            /*DMA模式：正常模式，只传输一次。注意：在AD采集之类时要配置成循环模式*/

    // DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                  /*优先级：非常高*/

    // DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                             /*禁止内存到内存的传输*/

    // DMA_Init(DMA1_Channel4, &DMA_InitStructure);                             /*配置DMA1的2通道*/



    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    // NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;

    // NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;

    // NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;

    // NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    // NVIC_Init(&NVIC_InitStructure);



    // DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);  //使能DMA传输完成中断


    dma1Ch4.enableIt({1,1});
    dma1Ch4.enableDoneIt();

    DMA_Cmd (DMA1_Channel4,ENABLE);  //使能DMA
}


int main(){

    Sys::Misc::prework();

    UartHw & uart = uart2;
    uart.init(115200 * 8, CommMethod::Dma, CommMethod::Blocking);
    // uart.init(115200 * 8);
    // DEBUG_PRINT(dma1Ch4.dma_index, dma1Ch4.channel_index);
    // DMA_INIT();
    // USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    // delay(100);
    // mem_buf[5] = '0';
    // DMA_INIT();
    // USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    // delay(100);
    // while(true);
    // uart1.init(115200 * 8, CommMethod::Interrupt, CommMethod::Dma);
    // test();
    // uint8_t cnt = 0;

    // while(true){
    //     if(uart.available()){
    //         delay(10);
    //         uart.println(uart.readStringAll());
    //     }
    //     delay(500);
    //     uart.println("??");
    // }
    while(true){
        delay(100);
        // for(uint8_t i = 0; i < 8; i++)
        uart << millis() << "\r\n";
    }

    // Stepper stp;
    // stp.init();
    // while(true){
    //     stp.run();
    // }
    // pedestrian_app();
    // modem_app();
    // test_app();
    // pmdc_test();
    // SpreadCycle::chopper_test();
    // buck_test();
    // osc_test();
}